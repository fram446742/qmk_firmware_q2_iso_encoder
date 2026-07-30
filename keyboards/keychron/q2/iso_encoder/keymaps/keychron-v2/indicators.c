/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "indicators.h"
#include "features.h"
#include "keymap_config.h"

// ═════════════════════════════════════════════════════════════════════════════
// Overview state
// ═════════════════════════════════════════════════════════════════════════════

static bool     overview_active   = false;
static uint32_t overview_start    = 0;
static uint8_t  saved_rgb_mode    = 0;
static bool     saved_rgb_enabled = false;

// ═════════════════════════════════════════════════════════════════════════════
// Helpers
// ═════════════════════════════════════════════════════════════════════════════

// Layer 0 → key "0" (LED 10), layer N → key N (LED N, 1-9)
static uint8_t layer_to_led(uint8_t layer) {
    if (layer == 0) return 10;
    if (layer >= 1 && layer <= 9) return layer;
    return 255;
}

// ═════════════════════════════════════════════════════════════════════════════
// Public API
// ═════════════════════════════════════════════════════════════════════════════

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

// ═════════════════════════════════════════════════════════════════════════════
// Per-frame drawing
// ═════════════════════════════════════════════════════════════════════════════

void indicator_draw(void) {
    if (!overview_active) return;

    rgb_matrix_set_color_all(0, 0, 0);

    // ── Active-layer indicator ──────────────────────────────────────────
    uint8_t base    = get_highest_layer(default_layer_state);
    uint8_t highest = get_highest_layer(layer_state);
    uint8_t display = (highest != base) ? highest : base;
    uint8_t led     = layer_to_led(display);
    if (led < RGB_MATRIX_LED_COUNT)
        rgb_matrix_set_color(led, 255, 255, 255);

    // ── Feature indicators (active=white, inactive=red) ────────────────
    typedef struct { uint8_t led; bool active; } ind_t;
    ind_t list[] = {
        { IND_CAPS_LOCK,   host_keyboard_led_state().caps_lock         },
        { IND_AUTO_SHIFT,  feature_auto_shift()                       },
        { IND_TAP_DANCE,   feature_tap_dance()                        },
        { IND_CAPS_WORD,   feature_caps_word()                        },
        { IND_REPEAT_KEY,  feature_repeat_key()                       },
        { IND_DYN_MACRO,   feature_dyn_macro()                        },
        { IND_LEADER,      feature_leader()                           },
        { IND_AUTOCORRECT, keymap_config.autocorrect_enable           },
        { IND_NKRO,        keymap_config.nkro                         },
    };
    for (int i = 0; i < (int)(sizeof(list)/sizeof(list[0])); i++) {
        rgb_matrix_set_color(list[i].led, 255, list[i].active ? 255 : 0, list[i].active ? 255 : 0);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// Per-loop timeout check
// ═════════════════════════════════════════════════════════════════════════════

void indicator_task(void) {
    if (overview_active && timer_elapsed32(overview_start) > OVERVIEW_TIMEOUT_MS) {
        feature_overview_cancel();
    }
}
