/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

/// Permanent trigger: timer-based layer change (TO, physical switch, etc.)
void layer_visualizer_trigger(void);

/// Call this from process_record_user when an MO key is PRESSED.
/// Reliable here because the MO layer hasn't been added yet, so
/// IS_QK_MOMENTARY(keycode) resolves correctly.
void layer_visualizer_momentary_start(void);

/// Call this from layer_state_set_user when a layer bit was REMOVED.
/// This is the authoritative MO-release detection path — QMK has already
/// removed the layer from the stack, so the state is correct regardless
/// of what keycode the physical position resolves to.
void layer_visualizer_mo_released(void);

bool layer_visualizer_is_active(void);
void layer_visualizer_task(void);
void layer_visualizer_draw(void);

/// Call once after keyboard init (after features_init). Records default layer.
void layer_visualizer_init(void);

/// Call from matrix_scan_user after the initial default-layer sync.
/// Enables triggers (the initial sync itself is suppressed).
void layer_visualizer_sync_complete(void);

void layer_vis_toggle(void);

/// Toggle the layer-visualization lock.  When locked, the color overlay
/// stays on indefinitely (no timer expiry) until a layer change occurs.
void layer_visualizer_lock_toggle(void);

/// Returns true if the layer-visualization lock is currently active.
bool layer_visualizer_is_locked(void);

/// Cancel any ongoing visualization (moment mode, timer mode, lock).
/// Called when overview mode is entered.
void layer_visualizer_cancel(void);
