/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

// NOTE: This file is #included from keymap.c, NOT compiled separately.
// It needs QMK_KEYBOARD_H and quantum includes already visible via keymap.c.

#include "combos.h"
#include "features.h"
#include "indicators.h"

// ── Combo key arrays ────────────────────────────────────────────────────────

// Navigation shortcuts
const uint16_t PROGMEM cb_esc_combo[]       = {KC_A, KC_S, COMBO_END};  // ESC
const uint16_t PROGMEM cb_bspc_combo[]      = {KC_J, KC_K, COMBO_END};  // Backspace
const uint16_t PROGMEM cb_del_combo[]       = {KC_K, KC_L, COMBO_END};  // Delete

// Feature toggles
const uint16_t PROGMEM cb_tog_autoshift[]   = {KC_Z, KC_X, COMBO_END};  // Toggle AutoShift

// Feature overview
const uint16_t PROGMEM cb_feat_overview[]   = {KC_O, KC_P, COMBO_END};  // Feature overview

// ── Combo table ─────────────────────────────────────────────────────────────
// Fires custom keycodes — handled in process_record_user() in keymap.c.

combo_t key_combos[] = {
    [CB_ESC]           = COMBO(cb_esc_combo,       KC_ESC),
    [CB_BSPC]          = COMBO(cb_bspc_combo,      KC_BSPC),
    [CB_DEL]           = COMBO(cb_del_combo,       KC_DEL),
    [CB_TOG_AUTOSHIFT] = COMBO(cb_tog_autoshift,   KC_AUTOSHIFT_TOGGLE),
    [CB_FEAT_OVERVIEW] = COMBO(cb_feat_overview,   KC_FEAT_OVERVIEW),
};
