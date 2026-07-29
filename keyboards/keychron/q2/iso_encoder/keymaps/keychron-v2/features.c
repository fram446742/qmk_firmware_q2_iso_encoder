/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "features.h"
#include "eeprom.h"
#include "action_layer.h"

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
    // Load persisted flags
    g_feature_flags = eeprom_read_byte((const uint8_t *)FEATURES_EEPROM_ADDR);

    // If EEPROM was erased (all 0xFF), treat as default config
    if (g_feature_flags == 0xFF) {
        g_feature_flags = 0;  // all features off by default
        features_save();
    }

    // Apply to hardware
    feature_apply_all();
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
