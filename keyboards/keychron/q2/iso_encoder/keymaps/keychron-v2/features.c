/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "features.h"
#include "quantum.h"
#include "eeprom.h"
#include "send_string.h"
#include "unicode.h"

#ifdef AUTO_SHIFT_ENABLE
#    include "process_auto_shift.h"  // autoshift_enable/disable
#endif

// For dispatching QMK-internal keycodes that can't be sent via tap_code16()
// (which sends raw HID usages).  CW_TOGG is an internal action, not a HID key.
#ifdef CAPS_WORD_ENABLE
#    include "caps_word.h"           // caps_word_toggle()
#endif

// ═════════════════════════════════════════════════════════════════════════════
// EEPROM layout — constants shared with Python tool via features.h
// ═════════════════════════════════════════════════════════════════════════════
#define EEP_TAP_SIZE          (MAX_TAP_OVERRIDES * sizeof(eeprom_tap_t))
#define EEP_COMBO_SIZE        (MAX_COMBOS * sizeof(eeprom_combo_t))
#define EEP_LEADER_SIZE       (MAX_LEADERS * sizeof(eeprom_leader_t))

// Default feature flags at first boot (all OFF except Caps Word + Repeat)
#define DEFAULT_FEATURE_FLAGS (FEATURE_CAPS_WORD | FEATURE_REPEAT_KEY)

// ═════════════════════════════════════════════════════════════════════════════
// Runtime state
// ═════════════════════════════════════════════════════════════════════════════

uint8_t g_feature_flags = 0;

// EEPROM-backed config arrays
eeprom_tap_t    eeprom_tap[MAX_TAP_OVERRIDES];
uint8_t         eeprom_tap_count   = 0;
eeprom_combo_t  eeprom_combos[MAX_COMBOS];
uint8_t         eeprom_combo_count = 0;
eeprom_leader_t eeprom_leaders[MAX_LEADERS];
uint8_t         eeprom_leader_count = 0;

// ═════════════════════════════════════════════════════════════════════════════
// Tap-dance transparent override
// ═════════════════════════════════════════════════════════════════════════════
// Intercepts base keycodes (KC_BSPC, KC_ESC, KC_E, …) from process_record_user
// and provides tap/double-tap behavior when FEATURE_TAP_DANCE is enabled.
// No TD() keycodes needed — the keymap stays clean for VIA compatibility.
//
// Adding or removing tap overrides at runtime: features_load_defaults() has
// the defaults; the HID config tool (qmk_config_tool.py) can read/write them.
//
// ═════════════════════════════════════════════════════════════════════════════

#define TAP_TERM 200  // ms — same as QMK default tapping term

static int8_t   tap_pending_idx = -1;  // index into eeprom_tap[], -1 = none
static uint16_t tap_timer       = 0;

// ── Fire the double-tap action for an override entry ────────────────────────

// ── Dispatch a double-tap action from the EEPROM override entry ────────────
// tap_code16() only works for real HID keycodes.  QMK-internal keycodes
// (CW_TOGG, QK_LEAD, etc.) are not HID usages and need direct dispatch.

static void tap_fire_override(const eeprom_tap_t *ov) {
    switch (ov->dbl_type) {
        case TD_DBL_KEYCODE: {
            uint16_t kc = ov->dbl_val;
            // QMK internal keycodes that need direct functions, not tap_code16
            // (which sends raw HID usages and will emit garbage for non-HID codes).
            // Add new cases here as needed — the EEPROM tool can write any value.
            switch (kc) {
#ifdef CAPS_WORD_ENABLE
                case CW_TOGG:
                    caps_word_toggle();
                    break;
#endif
                default:
                    // QK_UNICODE (≥0x8000) → register_unicode
                    // All other 16-bit values → tap_code16 (handles modded keycodes)
                    if (kc >= QK_UNICODE && kc <= QK_UNICODE_MAX) {
                        register_unicode(kc & 0x7FFF);
                    } else {
                        tap_code16(kc);
                    }
                    break;
            }
            break;
        }
        case TD_DBL_UNICODE_STR: {
            // Reconstruct up to 4 UTF-8 bytes from dbl_val/dbl_extra
            uint8_t buf[5];
            buf[0] =  ov->dbl_val       & 0xFF;
            buf[1] = (ov->dbl_val >> 8) & 0xFF;
            buf[2] =  ov->dbl_extra     & 0xFF;
            buf[3] = (ov->dbl_extra >> 8) & 0xFF;
            buf[4] = 0;
            send_unicode_string((const char *)buf);
            break;
        }
        case TD_DBL_UNICODE_CP:
            register_unicode(ov->dbl_val | ((uint32_t)ov->dbl_extra << 16));
            break;
        case TD_DBL_SEND_STRING: {
            // Up to 4 ASCII bytes via send_string() — works on any OS
            // because send_string() uses the firmware's HID keycode mapping.
            uint8_t buf[5];
            buf[0] =  ov->dbl_val       & 0xFF;
            buf[1] = (ov->dbl_val >> 8) & 0xFF;
            buf[2] =  ov->dbl_extra     & 0xFF;
            buf[3] = (ov->dbl_extra >> 8) & 0xFF;
            buf[4] = 0;
            send_string((const char *)buf);
            break;
        }
    }
}

// ── Main key interception ──────────────────────────────────────────────────

bool features_tap_process(uint16_t keycode, keyrecord_t *record) {
    for (int i = 0; i < eeprom_tap_count; i++) {
        if (keycode != eeprom_tap[i].base_kc) continue;

        if (record->event.pressed) {
            uint16_t now = timer_read();

            // Double-tap within TAP_TERM?
            if (tap_pending_idx == i && timer_elapsed(tap_timer) <= TAP_TERM) {
                tap_pending_idx = -1;
                tap_fire_override(&eeprom_tap[i]);
                return false;  // consumed
            }

            // A different key's tap was pending — fire it first
            if (tap_pending_idx >= 0) {
                tap_code16(eeprom_tap[tap_pending_idx].tap_kc);
                tap_pending_idx = -1;
            }

            // Start pending single-tap
            tap_pending_idx = i;
            tap_timer       = now;
            return false;  // consumed
        } else {
            // Release: suppress key-up for the pending key so the
            // single-tap (fired from task on timeout) works cleanly.
            if (tap_pending_idx == i) return false;
            return true;
        }
    }

    // Key is not a tap-override — fire any pending tap
    if (tap_pending_idx >= 0) {
        tap_code16(eeprom_tap[tap_pending_idx].tap_kc);
        tap_pending_idx = -1;
    }
    return true;  // not consumed
}

// ── Periodic timeout check ─────────────────────────────────────────────────

void features_tap_task(void) {
    if (tap_pending_idx >= 0 && timer_elapsed(tap_timer) > TAP_TERM) {
        int8_t idx = tap_pending_idx;
        tap_pending_idx = -1;
        tap_code16(eeprom_tap[idx].tap_kc);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// Feature flag management
// ═════════════════════════════════════════════════════════════════════════════

static void feature_apply_flag(uint8_t flag) {
    bool enabled = feature_has(flag);

    switch (flag) {
        case FEATURE_TAP_DANCE:
        case FEATURE_CAPS_WORD:
        case FEATURE_REPEAT_KEY:
        case FEATURE_DYN_MACRO:
        case FEATURE_LEADER:
            // No runtime action — process_record_user fences these
            break;
#ifdef AUTO_SHIFT_ENABLE
        case FEATURE_AUTO_SHIFT:
            if (enabled) autoshift_enable();
            else         autoshift_disable();
            break;
#endif
        default:
            break;
    }
}

static void feature_apply_all(void) {
    feature_apply_flag(FEATURE_TAP_DANCE);
    feature_apply_flag(FEATURE_AUTO_SHIFT);
    feature_apply_flag(FEATURE_CAPS_WORD);
    feature_apply_flag(FEATURE_REPEAT_KEY);
    feature_apply_flag(FEATURE_DYN_MACRO);
    feature_apply_flag(FEATURE_LEADER);
}

void features_init(void) {
    g_feature_flags = eeprom_read_byte((const uint8_t *)EEP_FEATURES);

    // First boot: EEPROM is erased (0xFF) or zeroed — set defaults
    if (g_feature_flags == 0xFF || g_feature_flags == 0) {
        g_feature_flags = DEFAULT_FEATURE_FLAGS;
        features_save();
        features_load_defaults();
        features_save_config();
    }

    features_load_config();
    feature_apply_all();
}

void features_save(void) {
    eeprom_write_byte((uint8_t *)EEP_FEATURES, g_feature_flags);
}

bool feature_has(uint8_t flag) {
    return (g_feature_flags & flag) != 0;
}

void feature_toggle(uint8_t flag) {
    g_feature_flags ^= flag;
    feature_apply_flag(flag);
    features_save();
}

void feature_set(uint8_t flag, bool on) {
    if (on) {
        g_feature_flags |= flag;
    } else {
        g_feature_flags &= ~flag;
    }
    feature_apply_flag(flag);
    features_save();
}

// ═════════════════════════════════════════════════════════════════════════════
// EEPROM config load / save / defaults
// ═════════════════════════════════════════════════════════════════════════════

void features_load_config(void) {
    eeprom_tap_count = eeprom_read_byte((const uint8_t *)EEP_TAP_BASE);
    if (eeprom_tap_count == 0xFF || eeprom_tap_count == 0
        || eeprom_tap_count > MAX_TAP_OVERRIDES) {
        features_load_defaults();
        features_save_config();
        return;
    }
    eeprom_read_block(eeprom_tap,    (void *)(EEP_TAP_BASE + 1),     EEP_TAP_SIZE);
    eeprom_combo_count = eeprom_read_byte((const uint8_t *)EEP_COMBO_BASE);
    eeprom_read_block(eeprom_combos, (void *)(EEP_COMBO_BASE + 1),   EEP_COMBO_SIZE);
    eeprom_leader_count = eeprom_read_byte((const uint8_t *)EEP_LEADER_BASE);
    eeprom_read_block(eeprom_leaders,(void *)(EEP_LEADER_BASE + 1),  EEP_LEADER_SIZE);
}

void features_save_config(void) {
    eeprom_write_byte((uint8_t *)EEP_TAP_BASE,      eeprom_tap_count);
    eeprom_write_block(eeprom_tap,    (void *)(EEP_TAP_BASE + 1),     EEP_TAP_SIZE);
    eeprom_write_byte((uint8_t *)EEP_COMBO_BASE,    eeprom_combo_count);
    eeprom_write_block(eeprom_combos, (void *)(EEP_COMBO_BASE + 1),   EEP_COMBO_SIZE);
    eeprom_write_byte((uint8_t *)EEP_LEADER_BASE,   eeprom_leader_count);
    eeprom_write_block(eeprom_leaders,(void *)(EEP_LEADER_BASE + 1),  EEP_LEADER_SIZE);
}

void features_load_defaults(void) {
    memset(eeprom_tap,    0, sizeof(eeprom_tap));
    memset(eeprom_combos, 0, sizeof(eeprom_combos));
    memset(eeprom_leaders,0, sizeof(eeprom_leaders));

    eeprom_tap_count = 5;

    // BSPC → DEL (double-tap)
    eeprom_tap[0] = (eeprom_tap_t){
        .base_kc=KC_BSPC, .tap_kc=KC_BSPC,
        .dbl_type=TD_DBL_KEYCODE, .dbl_val=KC_DEL
    };
    // ESC → CW_TOGG (double-tap)
    eeprom_tap[1] = (eeprom_tap_t){
        .base_kc=KC_ESC, .tap_kc=KC_ESC,
        .dbl_type=TD_DBL_KEYCODE, .dbl_val=CW_TOGG
    };
    // E → € — send AltGr+E (works on DE/FR/IT/ES layouts).
    // On US/UK layouts, change LCtrl+RCmd+E via the Python tool.
    eeprom_tap[2] = (eeprom_tap_t){
        .base_kc=KC_E, .tap_kc=KC_E,
        .dbl_type=TD_DBL_KEYCODE, .dbl_val=RALT(KC_E)
    };
    // 2 → @ — send Shift+2 (works on US layout).  On UK layout
    // (Shift+' = @), change to:
    //   .dbl_type=TD_DBL_KEYCODE, .dbl_val=RALT(KC_2)
    eeprom_tap[3] = (eeprom_tap_t){
        .base_kc=KC_2, .tap_kc=KC_2,
        .dbl_type=TD_DBL_KEYCODE, .dbl_val=S(KC_2)
    };
    // 4 → ~ — send Shift+` (works on US/DE layouts).  On ISO UK
    // change to S(KC_NUBS) or use Unicode.
    eeprom_tap[4] = (eeprom_tap_t){
        .base_kc=KC_4, .tap_kc=KC_4,
        .dbl_type=TD_DBL_KEYCODE, .dbl_val=S(KC_GRV)
    };

    // Combos and leaders: built-in compile-time (combos.c + keymap.c).
    // EEPROM combo/leader storage is for user customizations via the Python
    // tool only — no defaults here.  They start empty.
}
