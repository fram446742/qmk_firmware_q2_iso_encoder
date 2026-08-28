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
//     The feature-overview combo (O+[) is defined in BOTH systems:
//     - QMK-native: matches O+[ by keycode (KC_O + KC_LBRC)
//     - Position:   matches O+[ by physical key (POS_KC_O + POS_KC_LBRC)
//     QMK's native process_combo() runs first (in pre_process_record_quantum,
//     before process_record_user) and consumes O+[ wherever the keys resolve
//     to KC_O/KC_LBRC — including transparent FN layers.  The position-based
//     path is the fallback that fires when the keys are remapped in
//     VIA/Launcher so the native combo no longer matches them by keycode.
// ═════════════════════════════════════════════════════════════════════════════

const uint16_t PROGMEM cb_feat_overview[] = COMBO_FEAT_OVERVIEW_KEYS;

combo_t key_combos[] = {
    [CB_FEAT_OVERVIEW] = COMBO(cb_feat_overview, COMBO_FEAT_OVERVIEW_ACTION),
};
