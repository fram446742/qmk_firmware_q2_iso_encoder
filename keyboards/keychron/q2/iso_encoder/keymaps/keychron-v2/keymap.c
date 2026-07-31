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
#include "keymap_config.h"
#include "layer_visualizer.h"


// =============================================================================
// Combos  — #included so keymap_introspection sees the array
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
        KC_GRV,   KC_BRID,  KC_BRIU,  KC_MCTL,  KC_LPAD,  UG_VALD,  UG_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  _______,            _______,
        UG_TOGG,  UG_NEXT,  UG_VALU,  UG_HUEU,  UG_SATU,  UG_SPDU,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______,  UG_PREV,  UG_VALD,  UG_HUED,  UG_SATD,  UG_SPDD,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______,  _______,  _______),

    [WIN_FN1] = LAYOUT_iso_68(
        KC_GRV,   KC_BRID,  KC_BRIU,  KC_TASK,  KC_FILE,  UG_VALD,  UG_VALU,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  _______,            _______,
        UG_TOGG,  UG_NEXT,  UG_VALU,  UG_HUEU,  UG_SATU,  UG_SPDU,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______,  UG_PREV,  UG_VALD,  UG_HUED,  UG_SATD,  UG_SPDD,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______,  _______,  _______),

    [_FN2] = LAYOUT_iso_68(
        KC_TILD,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   _______,            _______,
        QK_LEAD,  UG_NEXT,  UG_VALU,  UG_HUEU,  UG_SATU,  UG_SPDU,  _______,  _______,  _______,  _______,  _______,  _______,  _______,                      _______,
        _______,  UG_PREV,  UG_VALD,  UG_HUED,  UG_SATD,  UG_SPDD,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______,  _______,  _______),

    [_FN3] = LAYOUT_iso_68(
        QK_GESC,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  KC_BSPC,            _______,
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

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // ── Layer visualization: first key press after boot arms the overlay ─
    // Layer changes before this (boot sync, USB enumeration, Launcher/VIA
    // connect commands) never start a display.
    if (record->event.pressed) {
        layer_visualizer_mark_user_activity();
    }

    // ── Tap-dance override ──────────────────────────────────────────────
    if (feature_tap_dance()) {
        if (!features_tap_process(keycode, record)) return false;
    }

    // ── Position-based combo processor (before overview, before QMK) ───
    // Catches chords like O+P by matrix position so they work on any layer.
    if (!features_combo_process(keycode, record)) return false;

    // ── Interactive overview mode ───────────────────────────────────────
    if (record->event.pressed && feature_overview_is_active()) {
        uint8_t r = record->event.key.row;
        uint8_t c = record->event.key.col;
        switch ((r << 4) | c) {
            case (2 << 4) | 1: feature_toggle_auto_shift();    feature_overview_reset_timer(); return false;
            case (2 << 4) | 2: autocorrect_toggle();          feature_overview_reset_timer(); return false;
            case (1 << 4) | 5: feature_toggle_tap_dance();    feature_overview_reset_timer(); return false;
            case (3 << 4) | 4: feature_toggle_caps_word();    feature_overview_reset_timer(); return false;
            case (1 << 4) | 4: feature_toggle_repeat_key();   feature_overview_reset_timer(); return false;
            case (2 << 4) | 3: feature_toggle_dyn_macro();    feature_overview_reset_timer(); return false;
            case (2 << 4) | 9: feature_toggle_leader();       feature_overview_reset_timer(); return false;
            case (3 << 4) | 7:
                clear_keyboard();
                keymap_config.nkro = !keymap_config.nkro;
                feature_overview_reset_timer();
                return false;
            case (0 << 4) | 10: LAYER_MOVE_OR_DEFAULT(0);  feature_overview_reset_timer(); return false;
            case (0 << 4) | 1:  LAYER_MOVE_OR_DEFAULT(1);  feature_overview_reset_timer(); return false;
            case (0 << 4) | 2:  LAYER_MOVE_OR_DEFAULT(2);  feature_overview_reset_timer(); return false;
            case (0 << 4) | 3:  LAYER_MOVE_OR_DEFAULT(3);  feature_overview_reset_timer(); return false;
            case (0 << 4) | 4:  LAYER_MOVE_OR_DEFAULT(4);  feature_overview_reset_timer(); return false;
            case (0 << 4) | 5:  LAYER_MOVE_OR_DEFAULT(5);  feature_overview_reset_timer(); return false;
            case (0 << 4) | 6:  LAYER_MOVE_OR_DEFAULT(6);  feature_overview_reset_timer(); return false;
            case (0 << 4) | 7:  LAYER_MOVE_OR_DEFAULT(7);  feature_overview_reset_timer(); return false;
            case (0 << 4) | 8:  LAYER_MOVE_OR_DEFAULT(8);  feature_overview_reset_timer(); return false;
            case (0 << 4) | 9:  layer_visualizer_lock_toggle(); feature_overview_reset_timer(); return false;
            default:            feature_overview_cancel();  return false;
        }
    }

    // ── Combo-only actions ─────────────────────────────────────────────
    if (record->event.pressed) {
        switch (keycode) {
            case KC_FEAT_OVERVIEW:
                feature_overview_trigger();
                return false;
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
// HID config handler  — used by qmk_config_tool.py for export/import
// ═════════════════════════════════════════════════════════════════════════════

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    uint8_t cmd  = data[0];
    uint8_t vid  = data[2];
    uint8_t idx  = data[3];
    uint8_t *pay = data + 4;
    uint8_t pay_len = length - 4;

    if (data[1] != 0x00) return;

    if (cmd == 0x08) {
        switch (vid) {
            case VALUE_FLAGS:        data[3] = g_feature_flags;                              break;
            case VALUE_TAP_COUNT:    data[3] = eeprom_tap_count;                             break;
            case VALUE_TAP_ENTRY:
                if (idx < eeprom_tap_count && pay_len >= sizeof(eeprom_tap_t))
                    memcpy(pay, &eeprom_tap[idx], sizeof(eeprom_tap_t));
                break;
            case VALUE_COMBO_COUNT:  data[3] = eeprom_combo_count;                           break;
            case VALUE_COMBO_ENTRY:
                if (idx < eeprom_combo_count && pay_len >= sizeof(eeprom_combo_t))
                    memcpy(pay, &eeprom_combos[idx], sizeof(eeprom_combo_t));
                break;
            case VALUE_LEADER_COUNT: data[3] = eeprom_leader_count;                          break;
            case VALUE_LEADER_ENTRY:
                if (idx < eeprom_leader_count && pay_len >= sizeof(eeprom_leader_t))
                    memcpy(pay, &eeprom_leaders[idx], sizeof(eeprom_leader_t));
                break;
        }
    } else if (cmd == 0x07) {
        switch (vid) {
            case VALUE_FLAGS:
                g_feature_flags = idx;
                feature_apply_all();
                features_save();
                break;
            case VALUE_TAP_COUNT:
                eeprom_tap_count = (idx < MAX_TAP_OVERRIDES) ? idx : MAX_TAP_OVERRIDES;
                break;
            case VALUE_TAP_ENTRY:
                if (idx < eeprom_tap_count && pay_len >= sizeof(eeprom_tap_t))
                    memcpy(&eeprom_tap[idx], pay, sizeof(eeprom_tap_t));
                break;
            case VALUE_COMBO_COUNT:
                eeprom_combo_count = (idx < MAX_COMBOS) ? idx : MAX_COMBOS;
                break;
            case VALUE_COMBO_ENTRY:
                if (idx < eeprom_combo_count && pay_len >= sizeof(eeprom_combo_t))
                    memcpy(&eeprom_combos[idx], pay, sizeof(eeprom_combo_t));
                break;
            case VALUE_LEADER_COUNT:
                eeprom_leader_count = (idx < MAX_LEADERS) ? idx : MAX_LEADERS;
                break;
            case VALUE_LEADER_ENTRY:
                if (idx < eeprom_leader_count && pay_len >= sizeof(eeprom_leader_t))
                    memcpy(&eeprom_leaders[idx], pay, sizeof(eeprom_leader_t));
                break;
        }
    } else if (cmd == 0x09) {
        features_save_config();
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// Init / scan / indicators
// ═════════════════════════════════════════════════════════════════════════════

void keyboard_post_init_user(void) {
    features_init();
}

static layer_state_t last_default_layer = 0;

void matrix_scan_user(void) {
    indicator_task();
    layer_visualizer_task();
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
bool rgb_matrix_indicators_user(void) {
    indicator_draw();

    if (feature_overview_is_active()) {
        // Overview screen: indicator LEDs only (drawn by indicator_draw).
        // The layer-visualization overlay — including the lock — is
        // suspended while overview is open; a locked overlay resumes on
        // exit via layer_visualizer_resume().
        return false;
    }

    if (layer_visualizer_is_active()) {
        layer_visualizer_draw();
        return false;
    }

    return true;
}
#endif
