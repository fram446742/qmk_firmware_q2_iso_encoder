/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

// NOTE: This file is #included from keymap.c, NOT compiled separately.
// It needs QMK_KEYBOARD_H and quantum includes already visible via keymap.c.

#include "combos.h"

// ═════════════════════════════════════════════════════════════════════════════
// Combo definitions
// ═════════════════════════════════════════════════════════════════════════════

// Feature overview: O + P simultaneously
const uint16_t PROGMEM cb_feat_overview[]   = {KC_O, KC_P, COMBO_END};

combo_t key_combos[] = {
    [CB_FEAT_OVERVIEW]   = COMBO(cb_feat_overview,   KC_FEAT_OVERVIEW),
};
