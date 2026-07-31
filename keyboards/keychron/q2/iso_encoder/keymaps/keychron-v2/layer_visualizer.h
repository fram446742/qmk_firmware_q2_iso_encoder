/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "action_layer.h"

/// Permanent trigger: timer-based layer change (TO, physical switch,
/// VIA command, etc.).  `state` is the NEW layer state — inside
/// layer_state_set_user the global `layer_state` is still the pre-change
/// value, so callers must pass it explicitly.
void layer_visualizer_trigger(layer_state_t state);

/// Call this from process_record_user when an MO() key is PRESSED, with
/// its packed matrix position (PACK_MTX(row, col)).  Reliable here because
/// the MO layer hasn't been added yet, so IS_QK_MOMENTARY(keycode)
/// resolves correctly.
void layer_visualizer_momentary_start(uint16_t mtx_pos);

/// Call this from process_record_user for EVERY key release, with the
/// key's packed matrix position.  No-op unless that position is tracked
/// as a held MO — matched by physical position, so it works even when
/// another MO on the stack changed the resolved keycode at this position.
void layer_visualizer_momentary_release(uint16_t mtx_pos);

bool layer_visualizer_is_active(void);
void layer_visualizer_task(void);
void layer_visualizer_draw(void);

/// Call from matrix_scan_user after the initial default-layer sync.
/// Enables triggers (the initial sync itself is suppressed).
void layer_visualizer_sync_complete(void);

/// Mark that the user has touched the keyboard.  Call from
/// process_record_user on every key press.  Layer changes before the
/// first press (boot sync, USB enumeration, Launcher/VIA connect)
/// never start a display.
void layer_visualizer_mark_user_activity(void);

void layer_vis_toggle(void);

/// Toggle the layer-visualization lock.  When locked, the color overlay
/// stays on indefinitely (no timer expiry) and updates with every layer
/// change, until the lock is toggled off again.  The lock survives overview
/// mode: entering overview pauses the overlay, exiting resumes it.
/// Locking is ignored while the layer-visualization feature is off.
void layer_visualizer_lock_toggle(void);

/// Returns true if the layer-visualization lock is currently active
/// (false when the feature is off).
bool layer_visualizer_is_locked(void);

/// Pause any ongoing visualization (moment mode, timer mode).  Preserves
/// the lock latch — layer_visualizer_resume() restores a locked overlay
/// when overview exits.  Called when overview mode is entered.
void layer_visualizer_cancel(void);

/// Resume a locked overlay after overview exits.  No-op unless the lock
/// is active and the feature is on.
void layer_visualizer_resume(void);
