/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// ═════════════════════════════════════════════════════════════════════════════
// INDICATORS  —  normal-state lock-LED drawing + layer↔LED mapping
// ═════════════════════════════════════════════════════════════════════════════
// The modal screens (feature_overview.c, layer_picker.c) each own their own
// overlay drawing; this module owns the non-modal caps-lock LED (pwm path) and
// the shared layer→LED mapping both screens use.

/// Map a layer number to a physical LED index.
/// Layer 0 → LED 10, layers 1-9 → LED N.  Returns 255 for invalid layers.
uint8_t layer_to_led(uint8_t layer);

/// Compute the current display layer and map to LED.
/// Returns the LED index for the active (non-default) layer indicator.
uint8_t indicator_led_for_layer(void);

/// Draw the normal-state caps-lock LED into the effect's pwm_buffer.
/// Call from rgb_matrix_indicators_advanced_user when no modal screen is open.
void indicator_draw(uint8_t led_min, uint8_t led_max);
