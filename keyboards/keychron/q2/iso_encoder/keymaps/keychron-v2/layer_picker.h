/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdbool.h>
#include <stdint.h>

struct keyrecord_t;
typedef struct keyrecord_t keyrecord_t;

// ═════════════════════════════════════════════════════════════════════════════
// LAYER-PICKER / "layer mode"  —  hold the knob button to switch only layers
// ═════════════════════════════════════════════════════════════════════════════
//
// Separate module from the feature overview (indicators.c).  Activated by
// holding the knob button (matrix position (0,14)) for LAYER_PICKER_HOLD_MS;
// while active the board goes dark and only the layer LEDs light.  Numbers and
// knob rotation switch layers; knob press / any other key exit keeping the
// layer; ESC exits to the default layer; idle timeout auto-exits.
//
// The knob button press is held back until released (→ its mapped key, e.g.
// KC_MUTE, is tapped) or held past the threshold (→ enter layer mode), so the
// assigned key is never fired spuriously by the long-press.

/// Modal handling + knob long-press detection.  Call from pre_process_record_user
/// BEFORE feature_overview_pre_process.  Returns false when the event was
/// consumed (layer mode open, or knob press held back).
bool layer_picker_pre_process(uint16_t keycode, keyrecord_t *record);

/// Poll (matrix_scan): knob-hold → enter; idle timeout → exit.
void layer_picker_task(void);

bool layer_picker_is_active(void);

/// Draw the layer LEDs into the overlay buffer.  Call from
/// rgb_matrix_indicators_advanced_user while the picker is active.
void layer_picker_draw(void);
