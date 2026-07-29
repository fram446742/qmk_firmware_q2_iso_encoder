/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "quantum_keycodes.h"

// ── Custom keycodes for combo-triggered features ────────────────────────────
// These are handled in process_record_user() in keymap.c.
// QK_KB_0 is the first custom keycode slot (SAFE_RANGE equivalent).

// keycodes_custom.h uses QK_KB_2 → NEW_SAFE_RANGE (0x5F0C).
// Our feature keycodes start at NEW_SAFE_RANGE to avoid collisions.
enum feature_keycodes {
    KC_AUTOSHIFT_TOGGLE = NEW_SAFE_RANGE,
    KC_FEAT_OVERVIEW,
};

// ── Combo event enum ────────────────────────────────────────────────────────
// The actual key_combos[] array is defined in combos.c (included from keymap.c).

enum combo_events {
    // Navigation shortcuts
    CB_ESC,            // A + S  → Escape  (left home row)
    CB_BSPC,           // J + K  → Backspace  (right home row)
    CB_DEL,            // K + L  → Delete  (right home row)

    // Feature toggles
    CB_TOG_AUTOSHIFT,  // Z + X  → toggle auto-shift

    // Feature overview
    CB_FEAT_OVERVIEW,  // O + P  → show feature status overview
};
