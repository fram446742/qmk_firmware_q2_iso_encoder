/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "indicators.h"
#include "keymap_config.h"
#include "keychron_rgb_type.h"

// Launcher indicator config (disable flags + HSV).  Defined in the vendor's
// keychron_rgb.c; referenced here for the caps-lock toggle.
extern os_indicator_config_t os_ind_cfg;

// ═════════════════════════════════════════════════════════════════════════════
// Layer ↔ LED mapping  (used by the overview + layer-picker screens)
// ═════════════════════════════════════════════════════════════════════════════

// Layer 0 → key "0" (LED 10), layers 1-8 → keys 1-8
// LED 9 is reserved for the layer-visualization lock indicator.
uint8_t layer_to_led(uint8_t layer) {
    if (layer == 0) return 10;
    if (layer >= 1 && layer <= 8) return layer;
    return 255;
}

uint8_t indicator_led_for_layer(void) {
    uint8_t base    = get_highest_layer(default_layer_state);
    uint8_t highest = get_highest_layer(layer_state);
    uint8_t display = (highest != base) ? highest : base;
    return layer_to_led(display);
}

// ═════════════════════════════════════════════════════════════════════════════
// Normal-state caps-lock LED (into the effect's pwm_buffer)
// ═════════════════════════════════════════════════════════════════════════════

#if defined(RGB_MATRIX_ENABLE) && defined(CAPS_LOCK_INDEX)
// The vendor's os_state_indicate() only draws the lock LEDs when no RGB
// effect is running; this runs on every frame so the Launcher's caps-lock
// enable/disable toggle is honored while an effect is active.  Moved here
// from q2.c so the keyboard file stays byte-identical to the vendor.
static void caps_lock_indicate(uint8_t led_min, uint8_t led_max) {
    if (os_ind_cfg.disable.caps_lock) return;

    if (host_keyboard_led_state().caps_lock) {
        RGB_MATRIX_INDICATOR_SET_COLOR(CAPS_LOCK_INDEX, 255, 255, 255);
    } else if (!rgb_matrix_get_flags()) {
        RGB_MATRIX_INDICATOR_SET_COLOR(CAPS_LOCK_INDEX, 0, 0, 0);
    }
}
#endif

void indicator_draw(uint8_t led_min, uint8_t led_max) {
    // Caps/Num/Win Lock are drawn by os_state_indicate() in keychron_rgb.c;
    // this callback only draws the persistent caps-lock override.
    if (!rgb_matrix_is_enabled()) return;

#if defined(RGB_MATRIX_ENABLE) && defined(CAPS_LOCK_INDEX)
    caps_lock_indicate(led_min, led_max);
#endif
}
