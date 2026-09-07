/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "indicators.h"
#include "features.h"
#include "keymap_config.h"
#include "layer_visualizer.h"
#include "rgb_matrix_drivers.h"
#include "keychron_rgb_type.h"

// Launcher indicator config (disable flags + HSV).  Defined in the vendor's
// keychron_rgb.c; referenced here for the caps-lock toggle.
extern os_indicator_config_t os_ind_cfg;

// ═════════════════════════════════════════════════════════════════════════════
// Overview state
// ═════════════════════════════════════════════════════════════════════════════

static bool     overview_active   = false;
static uint32_t overview_start    = 0;
static uint8_t  saved_rgb_mode    = 0;
static bool     saved_rgb_enabled = false;

// ═════════════════════════════════════════════════════════════════════════════
// Layer ↔ LED mapping
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
// Public API
// ═════════════════════════════════════════════════════════════════════════════

void feature_overview_trigger(void) {
    if (overview_active) return;

    saved_rgb_mode    = rgb_matrix_config.mode;
    saved_rgb_enabled = rgb_matrix_config.enable;

    // Cancel any ongoing layer visualization (moment or timer mode)
    // so it doesn't leak into overview mode.
    layer_visualizer_cancel();

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
    // Restore a locked layer-visualization overlay that overview paused.
    layer_visualizer_resume();
}

void feature_overview_reset_timer(void) {
    if (overview_active) {
        overview_start = timer_read32();
    }
}

// ── Overview key dispatch ────────────────────────────────────────────────
// Called from process_record_user for every key press while overview is
// open.  Positions use PACK_MTX (same packing as key_positions.h).
// Feature keys and the number row keep the overview open (timer reset);
// any other key exits it.

void feature_overview_handle_key(keyrecord_t *record) {
    uint16_t pos = PACK_MTX(record->event.key.row, record->event.key.col);

    switch (pos) {
        case PACK_MTX(2, 1):   // A — Auto-Shift
            feature_toggle_auto_shift();
            break;
        case PACK_MTX(2, 2):   // S — Auto-Correct
            autocorrect_toggle();
            break;
        case PACK_MTX(1, 5):   // T — Tap Dance
            feature_toggle_tap_dance();
            break;
        case PACK_MTX(3, 4):   // C — Caps Word
            feature_toggle_caps_word();
            break;
        case PACK_MTX(1, 4):   // R — Repeat Key
            feature_toggle_repeat_key();
            break;
        case PACK_MTX(2, 3):   // D — Dynamic Macro
            feature_toggle_dyn_macro();
            break;
        case PACK_MTX(2, 9):   // L — Leader Key
            feature_toggle_leader();
            break;
        case PACK_MTX(3, 7):   // N — NKRO
            clear_keyboard();
            keymap_config.nkro = !keymap_config.nkro;
            break;
        case PACK_MTX(0, 10):  // 0 — layer 0 (or back to default)
            LAYER_MOVE_OR_DEFAULT(0);
            break;
        case PACK_MTX(0, 1):   // 1 … 8 — layers 1-8
        case PACK_MTX(0, 2):
        case PACK_MTX(0, 3):
        case PACK_MTX(0, 4):
        case PACK_MTX(0, 5):
        case PACK_MTX(0, 6):
        case PACK_MTX(0, 7):
        case PACK_MTX(0, 8):
            LAYER_MOVE_OR_DEFAULT(pos & 0xFF);
            break;
        case PACK_MTX(0, 9):   // 9 — layer-visualization lock
            layer_visualizer_lock_toggle();
            break;
        case PACK_MTX(0, 0):   // ESC — reset to default layer and exit overview
            layer_move(get_highest_layer(default_layer_state));
            feature_overview_cancel();
            return;
        case POS_KC_MUTE:      // knob button — exit overview, keep current layer
            feature_overview_cancel();
            break;
        default:               // any other key — exit overview
            feature_overview_cancel();
            return;
    }
    feature_overview_reset_timer();
}

// ── Encoder (knob) handling during overview ──────────────────────────────
// Rotate the knob to cycle layers 0-8 (like the number keys); press the knob
// to close the overview and keep the current layer.  (ESC is the exit that
// resets to the default layer.)

void feature_overview_encoder(bool clockwise) {
    // Cycle 0..8 (9 is the layer-visualization lock, not a layer).
    uint8_t current = get_highest_layer(layer_state);
    if (current > 8) current = 0;  // safety: never cycle into the vis-lock slot

    uint8_t next = clockwise ? ((current + 1) % 9) : (current == 0 ? 8 : current - 1);
    layer_move(next);
    feature_overview_reset_timer();
}

// ═════════════════════════════════════════════════════════════════════════════
// Per-frame drawing
// ═════════════════════════════════════════════════════════════════════════════

#if defined(RGB_MATRIX_ENABLE) && defined(CAPS_LOCK_INDEX)
// ── Caps Lock (persistent) ─────────────────────────────────────────────────
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
    // this callback draws the caps-lock override + the interactive overview
    // grid (overview only).
    if (!rgb_matrix_is_enabled()) return;

    if (overview_active) {
        // ── Overview mode: dark screen + indicator grid, into the OVERLAY
        //    buffer — never the effect's pwm_buffer, so lazy effects keep
        //    their per-key state.  layer_visualizer_frame() flips the driver's
        //    flush override to display this buffer while overview is open.
        overlay_clear_all();

        // Active-layer indicator (white)
        uint8_t led = indicator_led_for_layer();
        if (led < RGB_MATRIX_LED_COUNT)
            overlay_set_color(led, 255, 255, 255);

        // Feature indicators (active=white, inactive=red)
        typedef struct { uint8_t led; bool active; } ind_t;
        ind_t list[] = {
            { IND_AUTO_SHIFT,  feature_auto_shift()                       },
            { IND_TAP_DANCE,   feature_tap_dance()                        },
            { IND_CAPS_WORD,   feature_caps_word()                        },
            { IND_REPEAT_KEY,  feature_repeat_key()                       },
            { IND_DYN_MACRO,   feature_dyn_macro()                        },
            { IND_LEADER,      feature_leader()                           },
            { IND_AUTOCORRECT, keymap_config.autocorrect_enable           },
            { IND_NKRO,        keymap_config.nkro                         },
            { IND_VIS_LOCK,    layer_visualizer_is_locked()               },
        };
        for (int i = 0; i < (int)(sizeof(list)/sizeof(list[0])); i++) {
            overlay_set_color(list[i].led, 255, list[i].active ? 255 : 0, list[i].active ? 255 : 0);
        }

#if defined(RGB_MATRIX_ENABLE) && defined(CAPS_LOCK_INDEX)
        // Caps Lock stays visible over the dark screen.
        if (!os_ind_cfg.disable.caps_lock && host_keyboard_led_state().caps_lock) {
            overlay_set_color(CAPS_LOCK_INDEX, 255, 255, 255);
        }
#endif
        return;
    }

    // Normal path: caps lock into the pwm_buffer (as before).
#if defined(RGB_MATRIX_ENABLE) && defined(CAPS_LOCK_INDEX)
    caps_lock_indicate(led_min, led_max);
#endif
}
// Per-loop timeout check
// ═════════════════════════════════════════════════════════════════════════════


void indicator_task(void) {
    if (!overview_active) return;
    // When layer-visualization is locked (9 key / 10th indicator), the
    // overview is considered "permanent" — don't auto-exit.  This matches
    // the user's expectation that the permanent mode stays until explicitly
    // toggled off.  While locked, the timer is paused.
    if (layer_visualizer_is_locked()) return;
    if (timer_elapsed32(overview_start) > OVERVIEW_TIMEOUT_MS) {
        feature_overview_cancel();
    }
}
