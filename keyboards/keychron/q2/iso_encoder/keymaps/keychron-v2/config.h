/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

// VIA overrides — must be visible early in the build (before QMK_KEYBOARD_H).
// All other feature config lives in keymap_config.h (included from .c files).

// This is the maximum allowed by QMK's dynamic keymap system (DYNAMIC_KEYMAP_LAYER_COUNT is a uint8_t).  The actual limit is the number of MO keys in the keymap, which is 8. The default is 5.
#undef  DYNAMIC_KEYMAP_LAYER_COUNT
#define DYNAMIC_KEYMAP_LAYER_COUNT 9

// We limit by actual macro keycode range (0xF0-0xFF = 16).  This is the default.
#undef  DYNAMIC_KEYMAP_MACRO_COUNT
#define DYNAMIC_KEYMAP_MACRO_COUNT 16

// ── Indicator LEDs (Keychron proprietary os_state_indicate path) ──────────
// Physical positions: Num Lock = Del key (POS_IDX_KC_DEL = 27),
// Win Lock = Left Option / Win key (POS_IDX_KC_LOPTN = 58).
// Caps Lock (CAPS_LOCK_INDEX 28) stays in the keyboard config.h, matching the
// vendor.  These two are custom to this keymap, so they live here to keep the
// keyboard config.h byte-identical to the vendor.
#define NUM_LOCK_INDEX 27
#define WINLOCK_LED_LIST \
    { 58 }
