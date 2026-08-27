/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// ═════════════════════════════════════════════════════════════════════════════
// Feature overview  —  interactive mode toggled by O+P combo
// ═════════════════════════════════════════════════════════════════════════════
// LED indices and timeouts are defined in keymap_config.h.

void feature_overview_trigger(void);
bool feature_overview_is_active(void);
void feature_overview_cancel(void);
void feature_overview_reset_timer(void);

/// Dispatch one key press while overview is open (feature toggles, layer
/// jumps, or exit).  Call from process_record_user — the key press is
/// always consumed.
void feature_overview_handle_key(keyrecord_t *record);

/// Map a layer number to a physical LED index.
/// Layer 0 → LED 10, layers 1-9 → LED N.  Returns 255 for invalid layers.
uint8_t layer_to_led(uint8_t layer);

/// Compute the current display layer and map to LED.
/// Returns the LED index for the active (non-default) layer indicator.
uint8_t indicator_led_for_layer(void);

void indicator_draw(uint8_t led_min, uint8_t led_max);
void indicator_task(void);
