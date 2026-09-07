/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H
#include "keychron_common.h"  // NEW_SAFE_RANGE — must precede keymap_config.h
#include "keymap_config.h"
#include "layer_picker.h"
#include "feature_overview.h" // feature_overview_is_active()/encoder()
#include "features.h"         // features_combo_clear()
#include "indicators.h"       // layer_to_led()
#include "layer_visualizer.h" // overlay_clear_all()/overlay_set_color()

// ═════════════════════════════════════════════════════════════════════════════
// State
// ═════════════════════════════════════════════════════════════════════════════

static bool     picker_active   = false;
static uint32_t picker_start    = 0;
static uint8_t  saved_rgb_mode  = 0;
static bool     saved_rgb_on    = false;

// Knob button (0,14) long-press detection.  The press is held back so the
// mapped key (e.g. KC_MUTE) never fires during a long hold: release early →
// replayed as a tap; held past LAYER_PICKER_HOLD_MS → layer mode.
static bool        knob_pending   = false;  ///< press held back (not registered)
static bool        knob_live      = false;  ///< press replayed; release must pass
static keyrecord_t knob_press_rec;
static uint16_t    knob_press_kc  = KC_NO;  ///< mapped keycode captured at press
static uint32_t    knob_press_time = 0;

#define KNOB_POS POS_KC_MUTE  // (0,14)

// ── Role colors (configured in keymap_config.h) ───────────────────────────
static const uint8_t COL_ACTIVE[3] = IND_LAYER_ACTIVE; // current layer
static const uint8_t COL_CHOICE[3] = IND_LAYER_CHOICE; // other layers

// ═════════════════════════════════════════════════════════════════════════════
// Enter / exit
// ═════════════════════════════════════════════════════════════════════════════

static void picker_enter(void) {
    if (picker_active) return;
    // A fresh long-press must always require a fresh 3 s hold: drop any stale
    // hold-detection state from a previous session.
    knob_pending    = false;
    knob_live       = false;
    knob_press_time = 0;

    saved_rgb_mode  = rgb_matrix_config.mode;
    saved_rgb_on    = rgb_matrix_config.enable;
    layer_visualizer_cancel();            // pause any layer-viz overlay
    features_combo_clear();               // modal will eat combo-key releases
    rgb_matrix_config.enable = 1;         // force the overlay render path
    picker_active = true;
    picker_start  = timer_read32();
}

static void picker_restore_rgb(void) {
    rgb_matrix_config.mode   = saved_rgb_mode;
    rgb_matrix_config.enable = saved_rgb_on;
}

/// Exit keeping the layer currently selected.
static void picker_exit_keep(void) {
    if (!picker_active) return;
    picker_active = false;
    knob_pending  = false;
    knob_live     = false;
    picker_restore_rgb();
}

/// Exit and reset to the default (Mac/Win base) layer.
static void picker_exit_default(void) {
    if (!picker_active) return;
    layer_move(get_highest_layer(default_layer_state));
    picker_exit_keep();
}

bool layer_picker_is_active(void) {
    return picker_active;
}

// ═════════════════════════════════════════════════════════════════════════════
// Dispatch (only while the picker is open)
// ═════════════════════════════════════════════════════════════════════════════

static void picker_dispatch(uint16_t keycode, keyrecord_t *record) {
    uint16_t pos = PACK_MTX(record->event.key.row, record->event.key.col);

    switch (pos) {
        case PACK_MTX(0, 0):    // ESC — default layer and exit
            picker_exit_default();
            return;
        case PACK_MTX(0, 10):   // 0
            LAYER_MOVE_OR_DEFAULT(0);
            break;
        case PACK_MTX(0, 1):    // 1 … 8
        case PACK_MTX(0, 2):
        case PACK_MTX(0, 3):
        case PACK_MTX(0, 4):
        case PACK_MTX(0, 5):
        case PACK_MTX(0, 6):
        case PACK_MTX(0, 7):
        case PACK_MTX(0, 8):
            LAYER_MOVE_OR_DEFAULT(pos & 0xFF);
            break;
        case KNOB_POS:          // knob press — exit keeping the layer
            picker_exit_keep();
            return;
        default:                // any other key — exit keeping the layer
            picker_exit_keep();
            return;
    }
    picker_start = timer_read32();  // keep-open on a layer choice
}

// ═════════════════════════════════════════════════════════════════════════════
// Knob long-press → enter layer mode
// ═════════════════════════════════════════════════════════════════════════════

static void knob_replay_down(void) {
    // Register the mapped key (e.g. KC_MUTE) directly, bypassing layer/mod
    // re-resolution; it stays down until the physical release lifts it.
    if (knob_press_kc != KC_NO && knob_press_kc != KC_TRNS) register_code16(knob_press_kc);
    knob_live = true;
}

bool layer_picker_pre_process(uint16_t keycode, keyrecord_t *record) {
    // Feature overview owns the modal while it's open — the knob press there
    // is the overview's exit, not a long-press candidate.
    if (feature_overview_is_active()) return true;

    // ── Layer mode open → true modal: consume everything, dispatch layers ──
    if (picker_active) {
        if (record->event.pressed) {
            if (IS_ENCODEREVENT(record->event)) {
                feature_overview_encoder(record->event.type == ENCODER_CW_EVENT);
                picker_start = timer_read32();
            } else {
                picker_dispatch(keycode, record);
            }
        }
        return false;
    }

    uint16_t pos = PACK_MTX(record->event.key.row, record->event.key.col);

    if (pos == KNOB_POS) {
        if (record->event.pressed) {
            // Always re-arm: even if a stale pending state leaked, a fresh
            // press must need a fresh LAYER_PICKER_HOLD_MS hold.
            knob_pending     = true;
            knob_live        = false;
            knob_press_kc    = keycode;   // mapped key, e.g. KC_MUTE
            knob_press_rec   = *record;
            knob_press_time  = timer_read();
            return false;  // hold back — nothing registered yet
        } else {
            if (knob_pending) {
                // Short press → tap the mapped key directly.  register + wait
                // + unregister guarantees a distinct media-key press reaches
                // the host (down+up via the event chain was being dropped).
                knob_pending = false;
                if (knob_press_kc != KC_NO && knob_press_kc != KC_TRNS) {
                    register_code16(knob_press_kc);
                    wait_ms(KNOB_TAP_RELEASE_DELAY_MS);
                    unregister_code16(knob_press_kc);
                }
                return false; // owned it; consume
            }
            knob_live = false; // was replayed; let the real release pass
            return true;
        }
    }
    // replay the knob key (rollover), then process this key normally.
    if (knob_pending && record->event.pressed) {
        knob_pending = false;
        knob_replay_down();
    }
    return true;
}

void layer_picker_task(void) {
    if (picker_active) {
        if (LAYER_PICKER_TIMEOUT_MS && timer_elapsed32(picker_start) > LAYER_PICKER_TIMEOUT_MS) {
            picker_exit_keep();
        }
        return;
    }
    if (!knob_pending) return;

    // The knob must STILL be physically held when the timer fires — never enter
    // layer mode from a released/tap press, even if stale state leaked in.
    uint8_t kr = (KNOB_POS >> 8) & 0xFF;
    uint8_t kc = KNOB_POS & 0xFF;
    if (!matrix_is_on(kr, kc)) {
        knob_pending = false;   // released before the hold time — nothing to do
        return;
    }
    if (timer_elapsed32(knob_press_time) > LAYER_PICKER_HOLD_MS) {
        knob_pending = false;
        picker_enter();
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// Drawing — only the layer LEDs, into the shared overlay buffer
// ═════════════════════════════════════════════════════════════════════════════

void layer_picker_draw(void) {
    if (!picker_active) return;
    overlay_clear_all();

    uint8_t cur = get_highest_layer(layer_state);
    for (uint8_t layer = 0; layer <= 8; layer++) {
        uint8_t led = layer_to_led(layer);
        if (led >= RGB_MATRIX_LED_COUNT) continue;
        const uint8_t *c = (layer == cur) ? COL_ACTIVE : COL_CHOICE;
        overlay_set_color(led, c[0], c[1], c[2]);
    }
}
