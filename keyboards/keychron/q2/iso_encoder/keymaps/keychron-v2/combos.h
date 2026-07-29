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
    // Toggle keycodes — handled in process_record_user()
    // Only KC_FEAT_OVERVIEW is active by default.  The others are reserved for
    // future custom-keycode assignments (combos commented out in combos.c).
    KC_AUTOSHIFT_TOGGLE = NEW_SAFE_RANGE,
    KC_TAP_DANCE_TOGGLE,
    KC_CAPS_WORD_TOGGLE,
    KC_REPEAT_KEY_TOGGLE,
    KC_DYN_MACRO_TOGGLE,
    KC_LEADER_TOGGLE,
    KC_NKRO_TOGGLE,
    KC_FEAT_OVERVIEW,
};

// ═════════════════════════════════════════════════════════════════════════════
// Combo definitions
// ═════════════════════════════════════════════════════════════════════════════
// key_combos[] is defined in combos.c (included from keymap.c).
//
// Navigational combos (A+S=ESC, J+K=BSPC, K+L=DEL) are in the source but
// commented out — the base layer already has these keys.
//
// Feature-toggle combos (Z+X for AutoShift, etc.) are also commented out.
// Toggles are now done inside the feature overview: press O+P, then tap
// the indicator key to toggle (e.g. A toggles Auto-Shift).

enum combo_events {
    // Navigational (commented out — see combos.c)
    CB_ESC,
    CB_BSPC,
    CB_DEL,

    // Overview (the only active combo)
    CB_FEAT_OVERVIEW,
};
