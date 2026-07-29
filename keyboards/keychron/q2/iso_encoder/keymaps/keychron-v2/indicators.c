/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include "indicators.h"
#include "features.h"
#include "quantum.h"  // keymap_config_t, rgb_matrix_*, timer_*, layer_state

// ── Overview state ──────────────────────────────────────────────────────────
static bool     overview_active   = false;
static uint32_t overview_start    = 0;
static uint8_t  saved_rgb_mode    = 0;
static bool     saved_rgb_enabled = false;

// Configurable overview duration (ms).  Set to 0 for no timeout.
#define OVERVIEW_TIMEOUT_MS 10000

// ── Helpers ─────────────────────────────────────────────────────────────────

// Show the default base layer.  default_layer_state tracks Mac (0) vs Win (1).
// Layer 0 → key "0" (LED 10), layer 1 → key "1" (LED 1), layer N→key N (LED N).
static uint8_t layer_to_led(uint8_t layer) {
    if (layer == 0) return 10;
    if (layer >= 1 && layer <= 8) return layer;
    return 255;
}

// ── Public API ──────────────────────────────────────────────────────────────

void feature_overview_trigger(void) {
    if (overview_active) return;

    saved_rgb_mode    = rgb_matrix_config.mode;
    saved_rgb_enabled = rgb_matrix_config.enable;

    overview_active = true;
    overview_start  = timer_read32();

    rgb_matrix_config.enable = 1;
}

bool feature_overview_is_active(void) {
    return overview_active;
}

void feature_overview_cancel(void) {
    if (!overview_active) return;
    overview_active = false;
    rgb_matrix_config.mode   = saved_rgb_mode;
    rgb_matrix_config.enable = saved_rgb_enabled;
}

void feature_overview_reset_timer(void) {
    if (overview_active) {
        overview_start = timer_read32();
    }
}

// ── Per-frame drawing ───────────────────────────────────────────────────────

void indicator_draw(void) {
    if (!overview_active) return;

    // Black out all LEDs
    rgb_matrix_set_color_all(0, 0, 0);

    // ── Active-layer indicator ──────────────────────────────────────────
    // Light the number key matching the highest active layer.
    // Uses layer_state so TG(N) toggles are reflected immediately.
    uint8_t layer = get_highest_layer(layer_state);
    uint8_t led   = layer_to_led(layer);
    if (led < RGB_MATRIX_LED_COUNT) {
        rgb_matrix_set_color(led, 255, 255, 255);  // white
    }

    // ── Feature indicators ──────────────────────────────────────────────
    // Active  → white (255,255,255)
    // Inactive → red   (255,0,0)

    // Caps Lock (always shown, hardware state)
    if (host_keyboard_led_state().caps_lock) {
        rgb_matrix_set_color(IND_CAPS_LOCK, 255, 255, 255);
    } else {
        rgb_matrix_set_color(IND_CAPS_LOCK, 255, 0, 0);
    }

    // Auto-Shift
    if (feature_auto_shift()) {
        rgb_matrix_set_color(IND_AUTO_SHIFT, 255, 255, 255);
    } else {
        rgb_matrix_set_color(IND_AUTO_SHIFT, 255, 0, 0);
    }

    // Tap Dance
    if (feature_tap_dance()) {
        rgb_matrix_set_color(IND_TAP_DANCE, 255, 255, 255);
    } else {
        rgb_matrix_set_color(IND_TAP_DANCE, 255, 0, 0);
    }

    // Caps Word
    if (feature_caps_word()) {
        rgb_matrix_set_color(IND_CAPS_WORD, 255, 255, 255);
    } else {
        rgb_matrix_set_color(IND_CAPS_WORD, 255, 0, 0);
    }

    // Repeat Key
    if (feature_repeat_key()) {
        rgb_matrix_set_color(IND_REPEAT_KEY, 255, 255, 255);
    } else {
        rgb_matrix_set_color(IND_REPEAT_KEY, 255, 0, 0);
    }

    // Dynamic Macro
    if (feature_dyn_macro()) {
        rgb_matrix_set_color(IND_DYN_MACRO, 255, 255, 255);
    } else {
        rgb_matrix_set_color(IND_DYN_MACRO, 255, 0, 0);
    }

    // Leader Key
    if (feature_leader()) {
        rgb_matrix_set_color(IND_LEADER, 255, 255, 255);
    } else {
        rgb_matrix_set_color(IND_LEADER, 255, 0, 0);
    }

    // NKRO (from QMK core — not a feature flag)
    if (keymap_config.nkro) {
        rgb_matrix_set_color(IND_NKRO, 255, 255, 255);
    } else {
        rgb_matrix_set_color(IND_NKRO, 255, 0, 0);
    }
}

// ── Per-loop timeout check ──────────────────────────────────────────────────

void indicator_task(void) {
    if (overview_active && timer_elapsed32(overview_start) > OVERVIEW_TIMEOUT_MS) {
        feature_overview_cancel();
    }
}
