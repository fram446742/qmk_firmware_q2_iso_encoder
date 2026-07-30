/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

/// Permanent trigger: timer-based layer change (TO, physical switch, etc.)
void layer_visualizer_trigger(void);

/// Momentary start: MO key held — shows target_layer while held (no timer).
void layer_visualizer_momentary_start(uint8_t target_layer);

/// Momentary stop: MO key released — reverts to brief permanent display.
void layer_visualizer_momentary_stop(void);

bool layer_visualizer_is_active(void);
void layer_visualizer_task(void);
void layer_visualizer_draw(void);
void layer_vis_toggle(void);
