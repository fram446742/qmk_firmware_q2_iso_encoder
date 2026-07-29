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
    KC_NKRO_TOGGLE,
    KC_FEAT_OVERVIEW,
};

// ═════════════════════════════════════════════════════════════════════════════
// Combo definitions
// ═════════════════════════════════════════════════════════════════════════════
// key_combos[] is defined in combos.c (included from keymap.c).
//
// Navigational combos (A+S=ESC, J+K=BSPC, K+L=DEL) are in the source but
// commented out — the base layer already has these keys.  Uncomment in
// combos.c if you want simultaneous-press shortcuts.

enum combo_events {
    // (navigational combos commented out — see combos.c)

    // Toggles — fires custom keycode, handled in process_record_user()
    CB_TOG_AUTOSHIFT,  // Z + X     → toggle auto-shift
    CB_TOG_TAP_DANCE,  // LEFT + R.Arrow  → toggle tap-dance
    CB_TOG_NKRO,       // Space + R.Shift → toggle NKRO

    // Display
    CB_FEAT_OVERVIEW,  // O + P     → show feature status overview
};
