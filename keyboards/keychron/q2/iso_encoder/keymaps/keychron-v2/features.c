/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "features.h"
#include "eeprom.h"
#include "action_layer.h"

// For default combo keycodes (KC_FEAT_OVERVIEW = 0x5F11, etc.)
// KC_FEAT_OVERVIEW is defined in combos.h but needs keycodes_custom.h
#define KC_FEAT_OVERVIEW_VAL 0x5F11

#ifdef TAP_DANCE_ENABLE
#    include "process_tap_dance.h"  // reset_tap_dance, tap_dance_get_state
#endif
#ifdef AUTO_SHIFT_ENABLE
#    include "process_auto_shift.h"
#endif

// ── EEPROM layout ───────────────────────────────────────────────────────────
// We store a single byte deep in the macro area, past all VIA/dynamic-keymap
// data. 8100 is safely past the expected macro buffer (≈1272–7200) on 8K EEPROM.
#define FEATURES_EEPROM_ADDR 8100

// ── Current runtime state ───────────────────────────────────────────────────
uint8_t g_feature_flags = 0;

// ── Forward declarations of static helpers ──────────────────────────────────
static void feature_apply_flag(uint8_t flag);
static void feature_apply_all(void);

// ── Hardware application ────────────────────────────────────────────────────

static void feature_apply_flag(uint8_t flag) {
    bool enabled = feature_has(flag);

    switch (flag) {
        case FEATURE_TAP_DANCE:
            // No runtime action needed — the preprocess_record_user fence
            // in keymap.c intercepts TD keycodes when this flag is OFF.
            // Any in-progress tap dance will time out naturally.
            break;
#ifdef AUTO_SHIFT_ENABLE
        case FEATURE_AUTO_SHIFT:
            if (enabled) autoshift_enable();
            else          autoshift_disable();
            break;
#endif
        // Remaining features have no HW state — process_record_user fence
        // blocks their keycodes when the feature flag is disabled.
        case FEATURE_CAPS_WORD:
        case FEATURE_REPEAT_KEY:
        case FEATURE_DYN_MACRO:
        case FEATURE_LEADER:
            break;
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

// ── Public API ──────────────────────────────────────────────────────────────

void features_init(void) {
    // TEMP FORCE-TEST: skip EEPROM, use hardcoded values
    // Remove the following block and uncomment the normal init below
    g_feature_flags = 0x05;  // TapDance + CapsWord ON
    eeprom_tap_count = 3;
    eeprom_tap[0] = (eeprom_tap_t){.base_kc=KC_BSPC,.tap_kc=KC_BSPC,.dbl_type=TD_DBL_KEYCODE,.dbl_val=KC_DEL};
    eeprom_tap[1] = (eeprom_tap_t){.base_kc=KC_ESC, .tap_kc=KC_ESC, .dbl_type=TD_DBL_KEYCODE,.dbl_val=CW_TOGG};
    eeprom_tap[2] = (eeprom_tap_t){.base_kc=KC_E,   .tap_kc=KC_E,   .dbl_type=TD_DBL_UNICODE_STR,.dbl_val=0x82E2,.dbl_extra=0x00AC};
    eeprom_combo_count = 1;
    eeprom_combos[0].keys[0] = KC_O;
    eeprom_combos[0].keys[1] = KC_P;
    eeprom_combos[0].output  = 0x5F11;
    eeprom_leader_count = 2;
    eeprom_leaders[0] = (eeprom_leader_t){.seq={KC_W}, .mod=MOD_LGUI, .key=KC_W};
    eeprom_leaders[1] = (eeprom_leader_t){.seq={KC_Q}, .mod=MOD_LGUI, .key=KC_Q};
    feature_apply_all();
#if 0
    // ── Normal init (restore when force-test above is removed) ──────────
    g_feature_flags = eeprom_read_byte((const uint8_t *)FEATURES_EEPROM_ADDR);

    // If EEPROM was erased (all 0xFF), treat as default config
    if (g_feature_flags == 0xFF) {
        g_feature_flags = 0;  // all features off by default
        features_save();
        features_load_defaults();
        features_save_config();
    }

    // Load EEPROM config into RAM arrays (tap overrides, combos, leaders)
    features_load_config();

    // Apply to hardware
    feature_apply_all();
#endif
}

void features_save(void) {
    eeprom_write_byte((uint8_t *)FEATURES_EEPROM_ADDR, g_feature_flags);
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
// EEPROM-backed config (tap overrides, combos, leader sequences)
// ═════════════════════════════════════════════════════════════════════════════

#define EEP_TAP_BASE    8101
#define EEP_TAP_SIZE    (MAX_TAP_OVERRIDES * sizeof(eeprom_tap_t))
#define EEP_COMBO_BASE  (EEP_TAP_BASE + 1 + EEP_TAP_SIZE)
#define EEP_COMBO_SIZE  (MAX_COMBOS * sizeof(eeprom_combo_t))
#define EEP_LEADER_BASE (EEP_COMBO_BASE + 1 + EEP_COMBO_SIZE)
#define EEP_LEADER_SIZE (MAX_LEADERS * sizeof(eeprom_leader_t))

// Runtime RAM arrays
eeprom_tap_t    eeprom_tap[MAX_TAP_OVERRIDES];
uint8_t         eeprom_tap_count   = 0;
eeprom_combo_t  eeprom_combos[MAX_COMBOS];
uint8_t         eeprom_combo_count = 0;
eeprom_leader_t eeprom_leaders[MAX_LEADERS];
uint8_t         eeprom_leader_count = 0;

void features_load_config(void) {
    eeprom_tap_count   = eeprom_read_byte((const uint8_t *)EEP_TAP_BASE);
    if (eeprom_tap_count == 0xFF || eeprom_tap_count > MAX_TAP_OVERRIDES) {
        features_load_defaults();
        features_save_config();
        return;
    }
    eeprom_read_block(eeprom_tap,    (void *)EEP_TAP_BASE + 1,    EEP_TAP_SIZE);
    eeprom_combo_count = eeprom_read_byte((const uint8_t *)EEP_COMBO_BASE);
    eeprom_read_block(eeprom_combos, (void *)EEP_COMBO_BASE + 1,  EEP_COMBO_SIZE);
    eeprom_leader_count = eeprom_read_byte((const uint8_t *)EEP_LEADER_BASE);
    eeprom_read_block(eeprom_leaders,(void *)EEP_LEADER_BASE + 1, EEP_LEADER_SIZE);
}

void features_save_config(void) {
    eeprom_write_byte((uint8_t *)EEP_TAP_BASE,      eeprom_tap_count);
    eeprom_write_block(eeprom_tap,    (void *)EEP_TAP_BASE + 1,     EEP_TAP_SIZE);
    eeprom_write_byte((uint8_t *)EEP_COMBO_BASE,    eeprom_combo_count);
    eeprom_write_block(eeprom_combos, (void *)EEP_COMBO_BASE + 1,   EEP_COMBO_SIZE);
    eeprom_write_byte((uint8_t *)EEP_LEADER_BASE,   eeprom_leader_count);
    eeprom_write_block(eeprom_leaders,(void *)EEP_LEADER_BASE + 1,  EEP_LEADER_SIZE);
}

void features_load_defaults(void) {
    // Clear all
    memset(eeprom_tap, 0, sizeof(eeprom_tap));
    memset(eeprom_combos, 0, sizeof(eeprom_combos));
    memset(eeprom_leaders, 0, sizeof(eeprom_leaders));

    // Default tap overrides (match compile-time defaults in keymap.c)
    eeprom_tap_count = 5;
    eeprom_tap[0] = (eeprom_tap_t){.base_kc=KC_BSPC,.tap_kc=KC_BSPC,.dbl_type=TD_DBL_KEYCODE,.dbl_val=KC_DEL};
    eeprom_tap[1] = (eeprom_tap_t){.base_kc=KC_ESC, .tap_kc=KC_ESC, .dbl_type=TD_DBL_KEYCODE,.dbl_val=CW_TOGG};
    eeprom_tap[2] = (eeprom_tap_t){.base_kc=KC_E,   .tap_kc=KC_E,   .dbl_type=TD_DBL_UNICODE_STR};
    // Copy UTF-8 bytes of "\u20AC" into dbl_val/dbl_extra
    eeprom_tap[2].dbl_val    = 0x82E2;  // UTF-8 bytes of €: E2 82
    eeprom_tap[2].dbl_extra  = 0x00AC;  // AC 00 (3rd byte + null)
    eeprom_tap[3] = (eeprom_tap_t){.base_kc=KC_2,   .tap_kc=KC_2,   .dbl_type=TD_DBL_UNICODE_STR,.dbl_val='@'};
    eeprom_tap[4] = (eeprom_tap_t){.base_kc=KC_GRV, .tap_kc=KC_GRV, .dbl_type=TD_DBL_UNICODE_STR,.dbl_val='~'};

    // Default combos (only overview)
    eeprom_combo_count = 1;
    eeprom_combos[0].keys[0] = KC_O;
    eeprom_combos[0].keys[1] = KC_P;
    eeprom_combos[0].output  = KC_FEAT_OVERVIEW_VAL;

    // Default leader sequences
    eeprom_leader_count = 2;
    eeprom_leaders[0] = (eeprom_leader_t){.seq={KC_W}, .mod=MOD_LGUI, .key=KC_W};
    eeprom_leaders[1] = (eeprom_leader_t){.seq={KC_Q}, .mod=MOD_LGUI, .key=KC_Q};
}
