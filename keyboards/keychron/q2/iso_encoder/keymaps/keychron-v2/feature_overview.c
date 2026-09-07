/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "feature_overview.h"
#include "indicators.h"        // layer_to_led(), indicator_led_for_layer()
#include "features.h"          // feature toggles / autocorrect / flags
#include "keymap_config.h"
#include "layer_visualizer.h"  // overlay buffer + cancel/resume/lock
#include "keychron_rgb_type.h" // os_indicator_config_t (caps-lock draw)

// Launcher indicator config (disable flags + HSV).  Defined in the vendor's
// keychron_rgb.c; referenced here for the caps-lock toggle.
extern os_indicator_config_t os_ind_cfg;

// ═════════════════════════════════════════════════════════════════════════════
// State
// ═════════════════════════════════════════════════════════════════════════════

static bool     overview_active   = false;
static uint32_t overview_start    = 0;
static uint8_t  saved_rgb_mode    = 0;
static bool     saved_rgb_enabled = false;

// The overview ENTRY is a position combo (POS_COMBOS_DEFS in keymap_config.h,
// handled by features.c features_combo_process): O + [ by physical matrix
// position → KC_FEAT_OVERVIEW.  This module only owns the open modal itself.

// ── Indicator role colors (configured in keymap_config.h) ────────────────
static const uint8_t COL_LAYER_ACTIVE[3] = IND_LAYER_ACTIVE;  // current layer
static const uint8_t COL_FEATURE_ON[3]   = IND_FEATURE_ON;    // toggle ON
static const uint8_t COL_FEATURE_OFF[3]  = IND_FEATURE_OFF;   // toggle OFF
static const uint8_t COL_CAPS_ON[3]      = IND_CAPS_LOCK_ON;  // caps over overlay

// ═════════════════════════════════════════════════════════════════════════════
// Public API — open / close
// ═════════════════════════════════════════════════════════════════════════════

void feature_overview_trigger(void) {
    if (overview_active) return;

    saved_rgb_mode    = rgb_matrix_config.mode;
    saved_rgb_enabled = rgb_matrix_config.enable;

    // Cancel any ongoing layer visualization (moment or timer mode)
    // so it doesn't leak into overview mode.
    layer_visualizer_cancel();

    // The modal will consume the chord keys' releases, so the position combo
    // (features_combo_process) must not keep stale down/fired/live state.
    features_combo_clear();

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

// ═════════════════════════════════════════════════════════════════════════════
// Overview key dispatch — by physical position (see below)
// ═════════════════════════════════════════════════════════════════════════════

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
// Overview modal + entry — keymap.c pre_process_record_user
// ═════════════════════════════════════════════════════════════════════════════
//
// Runs from pre_process_record_user (keymap.c), BEFORE every keycode-based
// handler in the quantum chain (native combos, auto-shift, tap-dance, leader,
// unicode…).  Two jobs:
//
//  1. While the overview is open it is a true modal: every key/encoder event
//     is consumed here, so no feature can swallow overview keys — the number
//     row switches layers regardless of what those keys mean on the current
//     layer or which runtime features are enabled.
//  2. When it is NOT open, every key is handed to features_combo_process()
//     (the position combo processor).  Its first entry is the O+[ overview
//     chord, matched by PHYSICAL matrix position (POS_KC_O / POS_KC_LBRC) —
//     never by keycode — so it opens from any layer.  On completion it fires
//     KC_FEAT_OVERVIEW, which process_record_user() maps to
//     feature_overview_trigger().  Single key presses are re-pressed so typing
//     O and [ alone still works (native-combo semantics, position-based).
//
// This is the documented divergence: a position-keyed combo instead of a
// QMK-native keycode combo (see DIVERGENCES.md).

bool feature_overview_pre_process(uint16_t keycode, keyrecord_t *record) {
    // ── Overview open → consume everything (press and release) ──────────
    if (overview_active) {
        if (record->event.pressed) {
            if (IS_ENCODEREVENT(record->event)) {
                feature_overview_encoder(record->event.type == ENCODER_CW_EVENT);
            } else {
                feature_overview_handle_key(record);
            }
        }
        return false;
    }

    // ── Not open: position combos own the entry chord (and any others). ──
    return features_combo_process(keycode, record);
}

// ═════════════════════════════════════════════════════════════════════════════
// Drawing — the overview grid into the shared overlay buffer
// ═════════════════════════════════════════════════════════════════════════════

void feature_overview_draw(void) {
    if (!overview_active) return;
    if (!rgb_matrix_is_enabled()) return;

    // Dark screen + indicator grid, into the OVERLAY buffer — never the
    // effect's pwm_buffer, so lazy effects keep their per-key state.
    // layer_visualizer_frame() flips the driver's flush override to display
    // this buffer while the overview is open.
    overlay_clear_all();

    // Active-layer indicator (white)
    uint8_t led = indicator_led_for_layer();
    if (led < RGB_MATRIX_LED_COUNT)
        overlay_set_color(led, COL_LAYER_ACTIVE[0], COL_LAYER_ACTIVE[1], COL_LAYER_ACTIVE[2]);

    // Feature indicators (ON = white, OFF = red)
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
        const uint8_t *c = list[i].active ? COL_FEATURE_ON : COL_FEATURE_OFF;
        overlay_set_color(list[i].led, c[0], c[1], c[2]);
    }

#if defined(RGB_MATRIX_ENABLE) && defined(CAPS_LOCK_INDEX)
    // Caps Lock stays visible over the dark screen.
    if (!os_ind_cfg.disable.caps_lock && host_keyboard_led_state().caps_lock) {
        overlay_set_color(CAPS_LOCK_INDEX, COL_CAPS_ON[0], COL_CAPS_ON[1], COL_CAPS_ON[2]);
    }
#endif
}

// ═════════════════════════════════════════════════════════════════════════════
// Idle timeout
// ═════════════════════════════════════════════════════════════════════════════

void feature_overview_task(void) {
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
