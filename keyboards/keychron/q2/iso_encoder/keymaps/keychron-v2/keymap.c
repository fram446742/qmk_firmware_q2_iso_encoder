/* Copyright 2023 ~ 2025 @ Keychron (https://www.keychron.com)
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ═════════════════════════════════════════════════════════════════════════════
 * Keychron Q2 ISO Encoder — VIA-enabled keymap with extended features
 * ═════════════════════════════════════════════════════════════════════════════
 */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "features.h"
#include "indicators.h"
#include "feature_overview.h"
#include "keymap_config.h"
#include "layer_visualizer.h"
#include "layer_picker.h"


// =============================================================================
// Feature-overview chord (O + [) — opened by PHYSICAL position in
// pre_process_record_user (indicators.c: feature_overview_pre_process).
// QMK-native combos (combos.c) are separate, keycode-matched; the chord keys
// are reserved and can't be reused by a combo (compile-time check in combos.c).
// =============================================================================

#ifdef COMBO_ENABLE
#    include "combos.c"
#endif

// =============================================================================
// Leader Key  — multi-key shortcut sequences
// =============================================================================
// Press QK_LEAD (on _FN2 at Q) then one key within ~300 ms.
// Modifier auto-selects Cmd on Mac layers, Ctrl on Windows layers.

#ifdef LEADER_ENABLE

void leader_end_user(void) {
    bool on_mac = (layer_state_is(MAC_BASE) || layer_state_is(MAC_FN1));

    if (leader_sequence_one_key(KC_W)) {
        tap_code16(LEADER_MOD(on_mac, KC_W));
    } else if (leader_sequence_one_key(KC_Q)) {
        tap_code16(LEADER_MOD(on_mac, KC_Q));
    } else if (leader_sequence_one_key(KC_S)) {
        tap_code16(LEADER_MOD(on_mac, KC_S));
    } else if (leader_sequence_one_key(KC_F)) {
        tap_code16(LEADER_MOD(on_mac, KC_F));
    } else if (leader_sequence_one_key(KC_A)) {
        tap_code16(LEADER_MOD(on_mac, KC_A));
    } else if (leader_sequence_one_key(KC_C)) {
        tap_code16(LEADER_MOD(on_mac, KC_C));
    } else if (leader_sequence_one_key(KC_V)) {
        tap_code16(LEADER_MOD(on_mac, KC_V));
    } else if (leader_sequence_one_key(KC_X)) {
        tap_code16(LEADER_MOD(on_mac, KC_X));
    } else if (leader_sequence_one_key(KC_Z)) {
        tap_code16(LEADER_MOD(on_mac, KC_Z));
    } else if (leader_sequence_one_key(KC_T)) {
        tap_code16(LEADER_MOD(on_mac, KC_T));
    } else if (leader_sequence_one_key(KC_R)) {
        tap_code16(LEADER_MOD(on_mac, KC_R));
    } else if (leader_sequence_one_key(KC_B)) {
        tap_code16(LEADER_MOD(on_mac, LSFT(KC_B)));
    } else if (leader_sequence_one_key(KC_N)) {
        tap_code16(LEADER_MOD(on_mac, KC_N));
    } else if (leader_sequence_one_key(KC_G)) {
        tap_code16(LEADER_MOD(on_mac, KC_G));
    } else if (leader_sequence_one_key(KC_H)) {
        tap_code16(LEADER_MOD(on_mac, KC_H));
    } else if (leader_sequence_one_key(KC_D)) {
        tap_code16(LEADER_MOD(on_mac, KC_D));
    } else if (leader_sequence_one_key(KC_P)) {
        tap_code16(LEADER_MOD(on_mac, KC_P));
    }
}

#endif // LEADER_ENABLE


// =============================================================================
// Keymaps
// =============================================================================

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [MAC_BASE] = LAYOUT_iso_68(
        QK_GESC,  KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,            KC_MUTE,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      KC_DEL,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,  KC_ENT,             KC_HOME,
        KC_LSFT,  KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,  KC_UP,
        KC_LCTL,  KC_LOPTN, KC_LCMMD,                               KC_SPC,                                 KC_RCMMD, FN1_MAC,  FN2,      KC_LEFT,  KC_DOWN,  KC_RGHT),

    [WIN_BASE] = LAYOUT_iso_68(
        QK_GESC,  KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,            KC_MUTE,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      KC_DEL,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,  KC_ENT,             KC_HOME,
        KC_LSFT,  KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,  KC_UP,
        KC_LCTL,  KC_LWIN,  KC_LALT,                                KC_SPC,                                 KC_RALT,  FN1_WIN,  FN2,      KC_LEFT,  KC_DOWN,  KC_RGHT),

    [MAC_FN1] = LAYOUT_iso_68(
        KC_GRV,   KC_BRID,  KC_BRIU,  KC_MCTL,  KC_LPAD,  UG_VALD,  UG_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  _______,            KC_F20,
        UG_TOGG,  UG_NEXT,  UG_VALU,  UG_HUEU,  UG_SATU,  UG_SPDU,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______,  UG_PREV,  UG_VALD,  UG_HUED,  UG_SATD,  UG_SPDD,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______,  _______,  _______),

    [WIN_FN1] = LAYOUT_iso_68(
        KC_GRV,   KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  UG_VALD,  UG_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  _______,            KC_F20,
        UG_TOGG,  UG_NEXT,  _______,  UG_VALU,  _______,  _______,  UG_HUEU,  _______,  _______,  UG_SATU,  _______,  UG_SPDU,  _______,                      _______,
        _______,  UG_PREV,  _______,  UG_VALD,  _______,  _______,  UG_HUED,  _______,  _______,  UG_SATD,  _______,  UG_SPDD,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
        _______,  _______,  _______,                                UG_TOGG,                                _______,  _______,  _______,  _______,  _______,  _______),

    [_FN2] = LAYOUT_iso_68(
        KC_TILD,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   _______,            _______,
        QK_LEAD,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      KC_INS,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            KC_PSCR,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  KC_PGUP,
        _______,  QK_MAGIC_TOGGLE_GUI,  _______,                                _______,                                _______,  _______,  _______,  KC_HOME,  KC_PGDN,  KC_END),

    [_FN3] = LAYOUT_iso_68(
        QK_GESC,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______,  _______,  _______),

    [_FN4] = LAYOUT_iso_68(
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______,  _______,  _______),

    [_FN5] = LAYOUT_iso_68(
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______,  _______,  _______),

    [_FN6] = LAYOUT_iso_68(
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______,  _______,  _______),
};
// clang-format on


// =============================================================================
// Encoder Map
// =============================================================================

#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [MAC_BASE] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [WIN_BASE] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [MAC_FN1]  = { ENCODER_CCW_CW(UG_VALD, UG_VALU) },
    [WIN_FN1]  = { ENCODER_CCW_CW(UG_VALD, UG_VALU) },
    [_FN2]     = { ENCODER_CCW_CW(UG_VALD, UG_VALU) },
    [_FN3]     = { ENCODER_CCW_CW(_______, _______) },
    [_FN4]     = { ENCODER_CCW_CW(_______, _______) },
    [_FN5]     = { ENCODER_CCW_CW(_______, _______) },
    [_FN6]     = { ENCODER_CCW_CW(_______, _______) },
};
#endif


// =============================================================================
// User callbacks  —  feature toggles, indicators, overview, HID handler
// =============================================================================

bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    // First key press after boot arms layer visualization (layer changes from
    // boot sync / USB enumeration / Launcher-VIA connect never start a show).
    if (record->event.pressed) {
        layer_visualizer_mark_user_activity();
    }

    // ── Feature overview & layer-picker are true modals, handled here by
    // physical position.  pre_process_record_user runs before every keycode-
    // based handler in the quantum chain (native combos, auto-shift, tap-dance,
    // leader, unicode…), so while one is open NOTHING can swallow its keys, and
    // the O+[ entry chord opens from any layer — blank layers included —
    // regardless of what those positions resolve to.  Layer-picker runs first
    // (knob long-press), then the feature-overview chord/modal.
    if (!layer_picker_pre_process(keycode, record)) return false;
    return feature_overview_pre_process(keycode, record);
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // ── Tap-dance override ──────────────────────────────────────────────
    if (feature_tap_dance()) {
        if (!features_tap_process(keycode, record)) return false;
    }

    // ── Position combos (custom) — separate from QMK-native (combos.c) and
    // from the overview chord (pre_process).  Runs here, overview not open.
    if (!features_combo_process(keycode, record)) return false;

    // ── RGB feedback: show RGB state for 1 second after RGB key press ──
    // Detects underglow, RGB matrix, backlight, and LED matrix keycodes.
    if (record->event.pressed) {
        if (IS_UNDERGLOW_KEYCODE(keycode) || IS_RGB_MATRIX_KEYCODE(keycode) ||
            IS_BACKLIGHT_KEYCODE(keycode) || IS_LED_MATRIX_KEYCODE(keycode)) {
            rgb_feedback_trigger();
        }
    }

    // ── Layer visualization: MO key tracking by matrix position ──────
    // PRESS: IS_QK_MOMENTARY(keycode) is reliable here because the MO
    // layer hasn't been added yet, so the resolved keycode is correct.
    // RELEASE: matched against the held-MO positions on EVERY key release.
    // No keycode resolution is involved, so it works even when another MO
    // on the stack changed the resolved keycode at this position.
    uint16_t mtx_pos = PACK_MTX(record->event.key.row, record->event.key.col);
    if (record->event.pressed) {
        if (IS_QK_MOMENTARY(keycode)) {
            layer_visualizer_momentary_start(mtx_pos);
        }
    } else {
        layer_visualizer_momentary_release(mtx_pos);
    }

    return true;
}


// ═════════════════════════════════════════════════════════════════════════════
// Layer change detection  —  triggers visualization overlay
// ═════════════════════════════════════════════════════════════════════════════

layer_state_t layer_state_set_user(layer_state_t state) {
    // Trigger vis on every layer change.  `state` is the NEW layer state —
    // the global layer_state is still the pre-change value inside this
    // hook, so it must be passed explicitly.  This is the same source
    // state_notify.c reports to the Keychron Launcher.
    layer_visualizer_trigger(state);
    return state;
}

// ═════════════════════════════════════════════════════════════════════════════
// Init / scan / indicators
// ═════════════════════════════════════════════════════════════════════════════
//
// NOTE: the HID config protocol (via_custom_value_command_kb) used by
// qmk_config_tool.py lives in features.c next to the EEPROM data it serves.

void keyboard_post_init_user(void) {
    features_init();
}

static layer_state_t last_default_layer = 0;

void matrix_scan_user(void) {
    // Per-module polls.  Hook pattern: every modal screen owns a *_task()
    // (idle/timeout) and a *_pre_process() / draw used by the hooks below.
    feature_overview_task();
    layer_picker_task();
    layer_visualizer_task();
    feature_overview_chord_task();
    features_tap_task();
    features_combo_task();

    if (last_default_layer != default_layer_state) {
        last_default_layer = default_layer_state;
        layer_move(get_highest_layer(default_layer_state));
    }

    // First scan completion: allows layer visualization triggers.
    // Must run after the initial default-layer sync above.
    // Use a static flag so this only runs once.
    static bool first_scan = true;
    if (first_scan) {
        first_scan = false;
        layer_visualizer_sync_complete();
    }
}

#if defined(RGB_MATRIX_ENABLE)
bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    // Flips the driver's flush override while an overlay screen (feature
    // overview, layer mode, or layer visualization) is showing, so its pixels
    // — not the effect's pwm_buffer — are displayed.  Runs first so the
    // per-screen draws below go into the overlay buffer and survive the
    // overlay deactivation clear.
    layer_visualizer_frame();

    // ── Modal screens own the whole board ──────────────────────────────
    if (feature_overview_is_active()) {
        feature_overview_draw();
        return true;
    }
    if (layer_picker_is_active()) {
        layer_picker_draw();
        return true;
    }

    // ── Normal state ───────────────────────────────────────────────────
    indicator_draw(led_min, led_max);  // caps-lock into the effect buffer

    if (layer_visualizer_is_active()) {
        layer_visualizer_draw();
        return true;
    }

    return true;
}
#endif
