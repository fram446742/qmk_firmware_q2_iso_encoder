/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

// NOTE: This file is #included from keymap.c, NOT compiled separately.
// It needs QMK_KEYBOARD_H and quantum includes already visible via keymap.c.

#include "combos.h"
#include "features.h"
#include "indicators.h"

// ═════════════════════════════════════════════════════════════════════════════
// Navigation shortcuts (COMMENTED OUT)
// ═════════════════════════════════════════════════════════════════════════════
//
// const uint16_t PROGMEM cb_esc_combo[]  = {KC_A, KC_S, COMBO_END};
// const uint16_t PROGMEM cb_bspc_combo[] = {KC_J, KC_K, COMBO_END};
// const uint16_t PROGMEM cb_del_combo[]  = {KC_K, KC_L, COMBO_END};

// ═════════════════════════════════════════════════════════════════════════════
// Feature toggles (COMMENTED OUT — done via overview now)
// ═════════════════════════════════════════════════════════════════════════════
//
// const uint16_t PROGMEM cb_tog_autoshift[]   = {KC_Z, KC_X, COMBO_END};
// const uint16_t PROGMEM cb_tog_tap_dance[]   = {KC_LEFT, KC_RGHT, COMBO_END};
// const uint16_t PROGMEM cb_tog_caps_word[]   = {KC_C, KC_V, COMBO_END};
// const uint16_t PROGMEM cb_tog_repeat_key[]  = {KC_R, KC_T, COMBO_END};
// const uint16_t PROGMEM cb_tog_dyn_macro[]   = {KC_D, KC_F, COMBO_END};
// const uint16_t PROGMEM cb_tog_leader[]      = {KC_L, KC_SCLN, COMBO_END};
// const uint16_t PROGMEM cb_tog_nkro[]        = {KC_SPC, KC_RSFT, COMBO_END};

// ═════════════════════════════════════════════════════════════════════════════
// Display (the only active combo)
// ═════════════════════════════════════════════════════════════════════════════

const uint16_t PROGMEM cb_feat_overview[]   = {KC_O, KC_LBRC, COMBO_END};

// ═════════════════════════════════════════════════════════════════════════════
// Combo table
// ═════════════════════════════════════════════════════════════════════════════

combo_t key_combos[] = {
    // Navigation (commented out)
    // [CB_ESC]           = COMBO(cb_esc_combo,       KC_ESC),
    // [CB_BSPC]          = COMBO(cb_bspc_combo,      KC_BSPC),
    // [CB_DEL]           = COMBO(cb_del_combo,       KC_DEL),

    // Overview (the only active entry)
    [CB_FEAT_OVERVIEW]   = COMBO(cb_feat_overview,   KC_FEAT_OVERVIEW),
};
