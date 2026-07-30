/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

// NOTE: #included from keymap.c, not compiled separately.
// keymap_config.h must be included before this file.

// ═════════════════════════════════════════════════════════════════════════════
// Combo definitions  —  keys and actions from keymap_config.h
// ═════════════════════════════════════════════════════════════════════════════
//
// TWO SEPARATE SYSTEMS:
//
// 1.  QMK-native key_combos[]  (this file)
//     Processed by QMK's built-in process_combo().  Keys must be KC_xxx
//     keycodes — only matches layers where those keycodes exist.
//     ⚠️  Do NOT use POS_KC_xxx or PACK_MTX here — QMK compares against
//        keycodes, not matrix positions.
//
// 2.  pos_combos[]  (features.c via features_combo_process)
//     Custom processor that accepts both KC_xxx keycodes AND matrix
//     positions (POS_KC_xxx / PACK_MTX), controlled by base_type.
//     Use for position-based combos that work on any layer.
//
//     The feature-overview combo (O+P) is defined in BOTH systems:
//     - QMK-native: matches O+P by keycode (KC_O + KC_LBRC)
//     - Position:   matches O+P by physical key (POS_KC_O + POS_KC_LBRC)
//     The position-based path runs first in process_record_user and
//     consumes the keys if the positions match, so it wins on any layer.
//     On layers where O and LBRC don't exist, the QMK-native path won't
//     fire (no false positive — no side-effect from the fallthrough).
// ═════════════════════════════════════════════════════════════════════════════

const uint16_t PROGMEM cb_feat_overview[] = COMBO_FEAT_OVERVIEW_KEYS;

combo_t key_combos[] = {
    [CB_FEAT_OVERVIEW] = COMBO(cb_feat_overview, COMBO_FEAT_OVERVIEW_ACTION),
};
