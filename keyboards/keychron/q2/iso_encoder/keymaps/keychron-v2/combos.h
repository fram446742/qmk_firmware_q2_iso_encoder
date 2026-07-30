/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "quantum_keycodes.h"

// ═════════════════════════════════════════════════════════════════════════════
// Custom keycodes for combo-triggered features
// ═════════════════════════════════════════════════════════════════════════════
// QK_KB_0 through NEW_SAFE_RANGE-1 are used by Keychron's custom_keycodes.
// Our codes start at NEW_SAFE_RANGE (defined in q2/keycodes_custom.h).

enum feature_keycodes {
    KC_AUTOSHIFT_TOGGLE = NEW_SAFE_RANGE,
    KC_TAP_DANCE_TOGGLE,
    KC_CAPS_WORD_TOGGLE,
    KC_REPEAT_KEY_TOGGLE,
    KC_DYN_MACRO_TOGGLE,
    KC_LEADER_TOGGLE,
    KC_NKRO_TOGGLE,
    KC_FEAT_OVERVIEW,
};

enum combo_events {
    CB_FEAT_OVERVIEW,
};
