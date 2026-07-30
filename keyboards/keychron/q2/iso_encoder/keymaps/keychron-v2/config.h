/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

// VIA overrides — must be visible early in the build (before QMK_KEYBOARD_H).
// All other feature config lives in keymap_config.h (included from .c files).

// Override info.json's 5-layer limit — 9 layers for this keymap
#undef  DYNAMIC_KEYMAP_LAYER_COUNT
#define DYNAMIC_KEYMAP_LAYER_COUNT 9

// We limit by actual macro keycode range (0xF0-0xFF = 16).  This is the default.
#undef  DYNAMIC_KEYMAP_MACRO_COUNT
#define DYNAMIC_KEYMAP_MACRO_COUNT 16
