/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdbool.h>
#include <stdint.h>

struct keyrecord_t;
typedef struct keyrecord_t keyrecord_t;

// ═════════════════════════════════════════════════════════════════════════════
// FEATURE OVERVIEW  —  the interactive config screen (O + [ by physical key)
// ═════════════════════════════════════════════════════════════════════════════
//
// Own module (feature_overview.c).  Opened by pressing O (1,9) and [ (1,11)
// TOGETHER — matched by matrix position in pre_process_record_user, never by
// keycode, so it works from any layer.  While open it is a true modal: every
// key/encoder event is consumed before any keycode-based handler.  The screen
// draws feature-toggle + layer LEDs into the shared overlay buffer (colors from
// keymap_config.h OVERLAY ROLE COLORS).
//
// Hook pattern (shared by layer_picker / layer_visualizer):
//   pre_process (modal + position-combo entry) → feature_overview_pre_process()
//   matrix_scan poll                          → feature_overview_task()
//   rgb frame draw                            → feature_overview_draw()

void feature_overview_trigger(void);
bool feature_overview_is_active(void);
void feature_overview_cancel(void);
void feature_overview_reset_timer(void);

/// Overview modal + position-combo entry (O + [ by matrix position).  Call
/// from pre_process_record_user (before any keycode-based handler).  Returns
/// false when the event was consumed (overview open, or a position-combo key
/// held back / completed).
bool feature_overview_pre_process(uint16_t keycode, keyrecord_t *record);

/// Poll (matrix_scan): overview idle timeout (0 / vis-lock = stay open).
void feature_overview_task(void);

/// Dispatch one key press while overview is open (feature toggles, layer
/// jumps, or exit).  Called from feature_overview_pre_process — the key
/// press is always consumed.
void feature_overview_handle_key(keyrecord_t *record);

/// Knob rotation during overview: cycle layers 0-8 (9 is the vis lock, skipped).
void feature_overview_encoder(bool clockwise);

/// Draw the overview screen into the overlay buffer (active layer + feature
/// toggles).  Call from rgb_matrix_indicators_advanced_user while active.
void feature_overview_draw(void);
