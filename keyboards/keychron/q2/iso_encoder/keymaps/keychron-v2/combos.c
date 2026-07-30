/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

// NOTE: #included from keymap.c, not compiled separately.
// keymap_config.h must be included before this file.

// ═════════════════════════════════════════════════════════════════════════════
// Combo definitions  —  keys and actions from keymap_config.h
// ═════════════════════════════════════════════════════════════════════════════

const uint16_t PROGMEM cb_feat_overview[] = COMBO_FEAT_OVERVIEW_KEYS;

combo_t key_combos[] = {
    [CB_FEAT_OVERVIEW] = COMBO(cb_feat_overview, COMBO_FEAT_OVERVIEW_ACTION),
};
