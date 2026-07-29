/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

// NOTE: This file is #included from keymap.c, NOT compiled separately.
// It needs QMK_KEYBOARD_H and quantum includes already visible via keymap.c.

#include "combos.h"
#include "features.h"
#include "indicators.h"

// ═════════════════════════════════════════════════════════════════════════════
// Navigation shortcuts (COMMENTED OUT — uncomment if desired)
// ═════════════════════════════════════════════════════════════════════════════
// These combos fire when the listed keys are pressed simultaneously.
// They're commented because the base layer already has dedicated keys.
//
// const uint16_t PROGMEM cb_esc_combo[]  = {KC_A, KC_S, COMBO_END};  // A+S→ESC
// const uint16_t PROGMEM cb_bspc_combo[] = {KC_J, KC_K, COMBO_END};  // J+K→BSPC
// const uint16_t PROGMEM cb_del_combo[]  = {KC_K, KC_L, COMBO_END};  // K+L→DEL

// ═════════════════════════════════════════════════════════════════════════════
// Feature toggles
// ═════════════════════════════════════════════════════════════════════════════

const uint16_t PROGMEM cb_tog_autoshift[]   = {KC_Z, KC_X, COMBO_END};   // Z+X: AutoShift
const uint16_t PROGMEM cb_tog_nkro[]        = {KC_SPC, KC_RSFT, COMBO_END};  // SPC+RSFT: NKRO

// ═════════════════════════════════════════════════════════════════════════════
// Display
// ═════════════════════════════════════════════════════════════════════════════

const uint16_t PROGMEM cb_feat_overview[]   = {KC_O, KC_P, COMBO_END};  // O+P: Overview

// ═════════════════════════════════════════════════════════════════════════════
// Combo table — fires custom keycodes, handled in process_record_user()
// ═════════════════════════════════════════════════════════════════════════════

combo_t key_combos[] = {
    // Navigation (commented out)
    // [CB_ESC]           = COMBO(cb_esc_combo,       KC_ESC),
    // [CB_BSPC]          = COMBO(cb_bspc_combo,      KC_BSPC),
    // [CB_DEL]           = COMBO(cb_del_combo,       KC_DEL),

    // Feature toggles
    [CB_TOG_AUTOSHIFT]   = COMBO(cb_tog_autoshift,   KC_AUTOSHIFT_TOGGLE),
    [CB_TOG_NKRO]        = COMBO(cb_tog_nkro,        KC_NKRO_TOGGLE),

    // Overview
    [CB_FEAT_OVERVIEW]   = COMBO(cb_feat_overview,   KC_FEAT_OVERVIEW),
};
