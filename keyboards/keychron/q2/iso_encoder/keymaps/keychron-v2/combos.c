/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

// NOTE: #included from keymap.c (not compiled separately) so keymap_introspection
// sees key_combos[].  keymap_config.h must be included before this file.

// ═════════════════════════════════════════════════════════════════════════════
// QMK-native combos  (keycode / "software-key" matching, process_combo)
// ═════════════════════════════════════════════════════════════════════════════
//
// Independent from BOTH the feature-overview chord AND the custom position
// combos (features.c).  Native combos match the resolved keycode on the active
// layer, so they only fire on layers where those keycodes exist.
//
// ⚠️  The feature-overview chord owns matrix (1,9)=O and (1,11)=[ by physical
//     position (indicators.c pre_process).  KC_O and KC_LBRC are RESERVED: a
//     native combo here that includes either fails to compile (duplicate enum
//     member) so the two systems can never collide.
//
// To add a native combo:
//   1. add its key array below (const uint16_t PROGMEM cb_<name>[] = {...}),
//   2. register every key in the COMBO_RESERVED_CHECK enum below,
//   3. add COMBO(cb_<name>, KC_<action>) to key_combos[].

#define CK(kc) COMBO_KEYCHECK_##kc
enum combo_key_reserved_check {
    // Reserved by the feature-overview chord — DO NOT reuse in a combo:
    CK(KC_O),
    CK(KC_LBRC),
    // Register every key of each new combo below, e.g.:
    // CK(KC_J), CK(KC_K),
    COMBO_KEYCHECK_END,
};
#undef CK

// (No native combos defined yet — this array intentionally empty.)
combo_t key_combos[] = {
    // [CB_EXAMPLE] = COMBO(cb_example, KC_X),
};
