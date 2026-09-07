/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

// NOTE: #included from keymap.c (not compiled separately) so keymap_introspection
// sees key_combos[].  keymap_config.h must be included before this file.

// ═════════════════════════════════════════════════════════════════════════════
// QMK-native combos  (keycode / "software-key" matching, process_combo)
// ═════════════════════════════════════════════════════════════════════════════
//
// NOTE ON THE FEATURE-OVERVIEW CHORD: the overview is NOT opened by a native
// combo.  It is opened by a POSITION combo (features.c features_combo_process,
// POS_COMBOS_DEFS in keymap_config.h) whose keys are matched by PHYSICAL matrix
// position (POS_KC_O / POS_KC_LBRC), so it fires on any layer — including blank
// ones.  QMK-native combos here can only match the resolved keycode, so KC_O /
// KC_LBRC are reserved for the overview and must not appear in a native combo
// (compile-time check below).  See DIVERGENCES.md §4.x.
//
// To add a native combo:
//   1. add its key array below (const uint16_t PROGMEM cb_<name>[] = {...}),
//   2. register every key in the COMBO_RESERVED_CHECK enum below,
//   3. add COMBO(cb_<name>, KC_<action>) to key_combos[].

#define CK(kc) COMBO_KEYCHECK_##kc
enum combo_key_reserved_check {
    // Used by the feature-overview position combo — do NOT reuse in a native
    // combo:
    CK(KC_O),
    CK(KC_LBRC),
    // Register every key of each new combo below, e.g.:
    // CK(KC_J), CK(KC_K),
    COMBO_KEYCHECK_END,
};
#undef CK

// (No native combos defined yet — add them per the instructions above.)
combo_t key_combos[] = {
    // [CB_X] = COMBO(cb_x, KC_Y),
};
