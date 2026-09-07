/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// ═════════════════════════════════════════════════════════════════════════════
// Feature overview  —  interactive mode toggled by O+[ combo
// ═════════════════════════════════════════════════════════════════════════════
// LED indices and timeouts are defined in keymap_config.h.

void feature_overview_trigger(void);
bool feature_overview_is_active(void);
void feature_overview_cancel(void);
void feature_overview_reset_timer(void);

/// Overview modal + O+[ entry chord, by PHYSICAL position.  Call from
/// pre_process_record_user (before any keycode-based handler).  Returns
/// false when the event was consumed (overview open, or chord completed).
bool feature_overview_pre_process(uint16_t keycode, keyrecord_t *record);

/// Poll (matrix_scan): resolve a chord key held past OV_CHORD_TERM_MS alone.
void feature_overview_chord_task(void);

/// Dispatch one key press while overview is open (feature toggles, layer
/// jumps, or exit).  Called from feature_overview_pre_process — the key
/// press is always consumed.
void feature_overview_handle_key(keyrecord_t *record);

/// Knob rotation during overview: cycle layers 0-8 (9 is the vis lock, skipped).
void feature_overview_encoder(bool clockwise);

/// Map a layer number to a physical LED index.
/// Layer 0 → LED 10, layers 1-9 → LED N.  Returns 255 for invalid layers.
uint8_t layer_to_led(uint8_t layer);

/// Compute the current display layer and map to LED.
/// Returns the LED index for the active (non-default) layer indicator.
uint8_t indicator_led_for_layer(void);

void indicator_draw(uint8_t led_min, uint8_t led_max);
void indicator_task(void);
