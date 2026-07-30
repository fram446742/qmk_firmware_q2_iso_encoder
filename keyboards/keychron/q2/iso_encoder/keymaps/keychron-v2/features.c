/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "features.h"
#include "eeprom.h"
#include "send_string.h"
#include "unicode.h"

// ═════════════════════════════════════════════════════════════════════════════
// All configuration constants (TAP_TERM, MAX_*, EEP_*, defaults, etc.) are
// in keymap_config.h, included from features.h.
// ═════════════════════════════════════════════════════════════════════════════

#include "keymap_config.h"
#include "key_positions.h"

#ifdef AUTO_SHIFT_ENABLE
#    include "process_auto_shift.h"
#endif

#ifdef CAPS_WORD_ENABLE
#    include "caps_word.h"
#endif


// ═════════════════════════════════════════════════════════════════════════════
// Runtime state
// ═════════════════════════════════════════════════════════════════════════════

uint8_t g_feature_flags = 0;

eeprom_tap_t    eeprom_tap[MAX_TAP_OVERRIDES];
uint8_t         eeprom_tap_count   = 0;
eeprom_combo_t  eeprom_combos[MAX_COMBOS];
uint8_t         eeprom_combo_count = 0;
eeprom_leader_t eeprom_leaders[MAX_LEADERS];
uint8_t         eeprom_leader_count = 0;


// ═════════════════════════════════════════════════════════════════════════════
// Tap-dance transparent override  (timer-based, no TAP_DANCE_ENABLE)
// ═════════════════════════════════════════════════════════════════════════════

static int8_t   tap_pending_idx = -1;
static uint16_t tap_timer       = 0;

// ── Dispatch a double-tap action from the EEPROM entry ─────────────────────

static void tap_fire_override(const eeprom_tap_t *ov) {
    switch (ov->dbl_type) {
        case TD_DBL_KEYCODE: {
            uint16_t kc = ov->dbl_val;
            switch (kc) {
#ifdef CAPS_WORD_ENABLE
                case CW_TOGG:
                    caps_word_toggle();
                    break;
#endif
                default:
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
// Matches by keycode (base_type=0) or matrix position (base_type=1).
// Position-based entries follow the physical key regardless of layer/layout.

static bool tap_matches(const eeprom_tap_t *ov, uint16_t keycode, keyrecord_t *record) {
    if (ov->base_type == BASE_IS_MATRIX) {
        uint8_t r = record->event.key.row;
        uint8_t c = record->event.key.col;
        return ((uint16_t)(r << 8) | c) == ov->base_id;
    }
    return keycode == ov->base_id;  // BASE_IS_KEYCODE
}

bool features_tap_process(uint16_t keycode, keyrecord_t *record) {
    for (int i = 0; i < eeprom_tap_count; i++) {
        if (!tap_matches(&eeprom_tap[i], keycode, record)) continue;

        if (record->event.pressed) {
            uint16_t now = timer_read();

            if (tap_pending_idx == i && timer_elapsed(tap_timer) <= TAP_TERM) {
                tap_pending_idx = -1;
                tap_fire_override(&eeprom_tap[i]);
                return false;
            }

            if (tap_pending_idx >= 0) {
                tap_code16(eeprom_tap[tap_pending_idx].tap_kc);
                tap_pending_idx = -1;
            }

            tap_pending_idx = i;
            tap_timer       = now;
            return false;
        } else {
            // Always consume the release of a matched key, even if not
            // currently pending (the press was already consumed above).
            // Letting the release through could unregister a keycode that
            // was never sent to the host.
            return false;
        }
    }

    if (tap_pending_idx >= 0) {
        tap_code16(eeprom_tap[tap_pending_idx].tap_kc);
        tap_pending_idx = -1;
    }
    return true;
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
    // Most feature flags gate runtime behavior in process_record_user()
    // via the feature_*() convenience wrappers. Only auto-shift has a
    // direct enable/disable API that needs immediate application.
    (void)flag;

#ifdef AUTO_SHIFT_ENABLE
    if (flag == FEATURE_AUTO_SHIFT) {
        if (feature_auto_shift()) autoshift_enable();
        else                      autoshift_disable();
    }
#endif
}

void feature_apply_all(void) {
    feature_apply_flag(FEATURE_TAP_DANCE);
    feature_apply_flag(FEATURE_AUTO_SHIFT);
    feature_apply_flag(FEATURE_CAPS_WORD);
    feature_apply_flag(FEATURE_REPEAT_KEY);
    feature_apply_flag(FEATURE_DYN_MACRO);
    feature_apply_flag(FEATURE_LEADER);
    feature_apply_flag(FEATURE_LAYER_VIS);
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
    if (eeprom_tap_count == 0xFF || eeprom_tap_count > MAX_TAP_OVERRIDES) {
        features_load_defaults();
        features_save_config();
        return;
    }
    eeprom_read_block(eeprom_tap,    (void *)(EEP_TAP_BASE + 1),     EEP_TAP_SIZE);

    eeprom_combo_count = eeprom_read_byte((const uint8_t *)EEP_COMBO_BASE);
    if (eeprom_combo_count == 0xFF || eeprom_combo_count > MAX_COMBOS) {
        eeprom_combo_count = 0;
    } else {
        eeprom_read_block(eeprom_combos, (void *)(EEP_COMBO_BASE + 1), EEP_COMBO_SIZE);
    }

    eeprom_leader_count = eeprom_read_byte((const uint8_t *)EEP_LEADER_BASE);
    if (eeprom_leader_count == 0xFF || eeprom_leader_count > MAX_LEADERS) {
        eeprom_leader_count = 0;
    } else {
        eeprom_read_block(eeprom_leaders, (void *)(EEP_LEADER_BASE + 1), EEP_LEADER_SIZE);
    }
}

void features_save_config(void) {
    eeprom_write_byte((uint8_t *)EEP_TAP_BASE,      eeprom_tap_count);
    eeprom_write_block(eeprom_tap,    (void *)(EEP_TAP_BASE + 1),     EEP_TAP_SIZE);
    eeprom_write_byte((uint8_t *)EEP_COMBO_BASE,    eeprom_combo_count);
    eeprom_write_block(eeprom_combos, (void *)(EEP_COMBO_BASE + 1),   EEP_COMBO_SIZE);
    eeprom_write_byte((uint8_t *)EEP_LEADER_BASE,   eeprom_leader_count);
    eeprom_write_block(eeprom_leaders,(void *)(EEP_LEADER_BASE + 1),  EEP_LEADER_SIZE);
}

// ── Sentinel-terminated default arrays (count derived at compile time) ─────
// The sentinel {0} entry is excluded from the count.

static const eeprom_tap_t    tap_defaults_all[]    = { TAP_DEFAULTS };
static const eeprom_combo_t  combo_defaults_all[]  = { COMBO_DEFAULTS };
static const eeprom_leader_t leader_defaults_all[] = { LEADER_DEFAULTS };

#define TAP_DEFAULTS_COUNT    ((sizeof(tap_defaults_all)    / sizeof(tap_defaults_all[0]))    - 1)
#define COMBO_DEFAULTS_COUNT  ((sizeof(combo_defaults_all)  / sizeof(combo_defaults_all[0]))  - 1)
#define LEADER_DEFAULTS_COUNT ((sizeof(leader_defaults_all) / sizeof(leader_defaults_all[0])) - 1)

void features_load_defaults(void) {
    memset(eeprom_tap,    0, sizeof(eeprom_tap));
    memset(eeprom_combos, 0, sizeof(eeprom_combos));
    memset(eeprom_leaders,0, sizeof(eeprom_leaders));

    eeprom_tap_count = TAP_DEFAULTS_COUNT;
    for (int i = 0; i < TAP_DEFAULTS_COUNT && i < MAX_TAP_OVERRIDES; i++) {
        eeprom_tap[i] = tap_defaults_all[i];
    }

    eeprom_combo_count = COMBO_DEFAULTS_COUNT;
    for (int i = 0; i < COMBO_DEFAULTS_COUNT && i < MAX_COMBOS; i++) {
        eeprom_combos[i] = combo_defaults_all[i];
    }

    eeprom_leader_count = LEADER_DEFAULTS_COUNT;
    for (int i = 0; i < LEADER_DEFAULTS_COUNT && i < MAX_LEADERS; i++) {
        eeprom_leaders[i] = leader_defaults_all[i];
    }
}
