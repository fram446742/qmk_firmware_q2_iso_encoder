/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "indicators.h"
#include "keymap_config.h"
#include "keychron_rgb_type.h"
#include "layer_visualizer.h"  // overlay_set_color()

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

#if defined(RGB_MATRIX_ENABLE) && defined(WINLOCK_LED_LIST)
// Win Lock (keymap_config.no_gui, toggled by FN2+Win → QK_MAGIC_TOGGLE_GUI).
// Two draw variants, deliberately asymmetric so the indicator never fights a
// running RGB effect:
//   * effect buffer (normal state) — silent unless the lock is ON: the Win key
//     LED then turns red; while released it keeps whatever colour the effect
//     gives it.
//   * overlay buffer (visualization/overview/layer mode) — the screens own the
//     whole board, so the key is always drawn: red while locked, green while
//     released.
// The vendor's os_state_indicate() cannot light it here: its WINLOCK block sits
// inside `#ifdef WIN_BASE_LAYER` (never defined by this port) and it only draws
// when no RGB effect is running.
static const uint8_t COL_WIN_LOCK_ON[3]  = IND_WIN_LOCK_ON;
static const uint8_t COL_WIN_LOCK_OFF[3] = IND_WIN_LOCK_OFF;

// ── pwm/effect-buffer variant — only the locked state overrides the effect ──
static void win_lock_indicate(uint8_t led_min, uint8_t led_max) {
    if (!keymap_config.no_gui) return;

    uint8_t idx_list[] = WINLOCK_LED_LIST;
    for (uint8_t i = 0; i < sizeof(idx_list); i++) {
        RGB_MATRIX_INDICATOR_SET_COLOR(idx_list[i], COL_WIN_LOCK_ON[0], COL_WIN_LOCK_ON[1], COL_WIN_LOCK_ON[2]);
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
#if defined(RGB_MATRIX_ENABLE) && defined(WINLOCK_LED_LIST)
    win_lock_indicate(led_min, led_max);
#endif
}

void indicator_draw_overlay(void) {
#if defined(RGB_MATRIX_ENABLE) && defined(WINLOCK_LED_LIST)
    // Same Win Lock state, but into the overlay buffer: the overlay screens
    // (visualization, overview, layer mode) repaint every LED, so this has to
    // be drawn after them or the lock state disappears while they're up.
    // Here the released state is drawn green too — the overlay owns the board,
    // so there is no effect colour to preserve.
    const uint8_t *c          = keymap_config.no_gui ? COL_WIN_LOCK_ON : COL_WIN_LOCK_OFF;
    uint8_t        idx_list[] = WINLOCK_LED_LIST;
    for (uint8_t i = 0; i < sizeof(idx_list); i++) {
        overlay_set_color(idx_list[i], c[0], c[1], c[2]);
    }
#endif
}
