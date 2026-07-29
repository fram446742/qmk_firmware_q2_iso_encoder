/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "indicators.h"
#include "features.h"
#include "quantum.h"     // keymap_config_t, rgb_matrix_*, timer_*

// ── Overview state ──────────────────────────────────────────────────────────
static bool     overview_active   = false;
static uint32_t overview_start    = 0;
static uint8_t  saved_rgb_mode    = 0;
static bool     saved_rgb_enabled = false;

// keymap_config (for keymap_config.nkro) is provided by quantum.h

#define OVERVIEW_TIMEOUT_MS 2000

// ── Public API ──────────────────────────────────────────────────────────────

void feature_overview_trigger(void) {
    if (overview_active) return;  // already showing

    // Save current state
    saved_rgb_mode    = rgb_matrix_config.mode;
    saved_rgb_enabled = rgb_matrix_config.enable;

    // Force static mode during overview (so the effect loop doesn't fight us)
    // We use 0xFF as a sentinel — indicator_draw() will handle rendering
    overview_active = true;
    overview_start  = timer_read32();

    // Ensure RGB is enabled
    rgb_matrix_config.enable = 1;
}

bool feature_overview_is_active(void) {
    return overview_active;
}

void feature_overview_cancel(void) {
    if (!overview_active) return;

    overview_active = false;

    // Restore saved state
    rgb_matrix_config.mode   = saved_rgb_mode;
    rgb_matrix_config.enable = saved_rgb_enabled;
}

// ── Per-frame drawing ───────────────────────────────────────────────────────

void indicator_draw(void) {
    if (!overview_active) return;

    // Black out all LEDs — this runs after the main effect so it overrides it
    rgb_matrix_set_color_all(0, 0, 0);

    // Draw feature status indicators
    // Active features → white (255,255,255)
    // Inactive features → very dim gray (5,5,5) so position is visible

    // Caps Lock (on Caps Lock key)
    if (host_keyboard_led_state().caps_lock) {
        rgb_matrix_set_color(IND_CAPS_LOCK, 255, 255, 255);
    } else {
        rgb_matrix_set_color(IND_CAPS_LOCK, 5, 5, 5);
    }

    // Auto-shift (on A key)
    if (feature_auto_shift()) {
        rgb_matrix_set_color(IND_AUTO_SHIFT, 0, 255, 0);
    } else {
        rgb_matrix_set_color(IND_AUTO_SHIFT, 5, 5, 5);
    }

    // NKRO (on N key)
    if (keymap_config.nkro) {
        rgb_matrix_set_color(IND_NKRO, 255, 255, 255);
    } else {
        rgb_matrix_set_color(IND_NKRO, 5, 5, 5);
    }
}

// ── Per-loop timeout check ──────────────────────────────────────────────────

void indicator_task(void) {
    if (overview_active && timer_elapsed32(overview_start) > OVERVIEW_TIMEOUT_MS) {
        feature_overview_cancel();
    }
}
