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
