/* Copyright 2023 ~ 2025 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "send_string.h"

// =============================================================================
// Layers
// =============================================================================

enum layers {
    MAC_BASE,
    WIN_BASE,
    MAC_FN1,
    WIN_FN1,
    _FN2,
    _FN3,
    _FN4,
    _FN5,
};

#define FN1_MAC MO(MAC_FN1)
#define FN1_WIN MO(WIN_FN1)
#define FN2     MO(_FN2)

// =============================================================================
// Tap Dance — tap vs double-tap
// =============================================================================
// Replace KC_ESC / KC_BSPC on base layers to add double-tap actions.
// Fallback plain keys are on _FN3.

#ifdef TAP_DANCE_ENABLE

enum {
    TD_BSPC_DEL,       // tap = Backspace,  double-tap = Delete
    TD_ESC_CAPS,       // tap = Escape,    double-tap = Caps Word toggle
};

tap_dance_action_t tap_dance_actions[] = {
    [TD_BSPC_DEL] = ACTION_TAP_DANCE_DOUBLE(KC_BSPC, KC_DEL),
    [TD_ESC_CAPS] = ACTION_TAP_DANCE_DOUBLE(KC_ESC, CW_TOGG),
};

#endif // TAP_DANCE_ENABLE

// =============================================================================
// Combos — simultaneous key chords
// =============================================================================
// Combo keys don't need to be placed in the keymap — they fire automatically
// when the listed keys are pressed together within COMBO_TERM (default 50ms).

#ifdef COMBO_ENABLE

enum combo_events {
    CB_ESC,            // A + S  → Escape  (left home row)
    CB_BSPC,           // J + K  → Backspace  (right home row)
    CB_DEL,            // K + L  → Delete  (right home row)
};

const uint16_t PROGMEM cb_esc_combo[]  = {KC_A, KC_S, COMBO_END};
const uint16_t PROGMEM cb_bspc_combo[] = {KC_J, KC_K, COMBO_END};
const uint16_t PROGMEM cb_del_combo[]  = {KC_K, KC_L, COMBO_END};

combo_t key_combos[] = {
    [CB_ESC]  = COMBO(cb_esc_combo,  KC_ESC),
    [CB_BSPC] = COMBO(cb_bspc_combo, KC_BSPC),
    [CB_DEL]  = COMBO(cb_del_combo,  KC_DEL),
};

#endif // COMBO_ENABLE

// =============================================================================
// Key Overrides — modifier + key → different output
// =============================================================================
// Uncomment the example below to map Shift + [ISO key left of Z] → Tilde.
// Add more with ko_make_basic(trigger_mods, trigger_key, replacement).

#ifdef KEY_OVERRIDE_ENABLE

// Example: Shift + KC_NUBS (ISO key between left-shift and Z) → KC_TILD
// const key_override_t nubs_tilde_override = ko_make_basic(
//     MOD_MASK_SHIFT, KC_NUBS, KC_TILD
// );

// Point this array at your override instances. NULL means no overrides active.
const key_override_t *key_overrides[] = {
    // &nubs_tilde_override,   // uncomment when you add the override above
};

#endif // KEY_OVERRIDE_ENABLE

// =============================================================================
// Leader Key — multi-key shortcut sequences
// =============================================================================
// Press the QK_LEAD key (placed on _FN2 at the Q position) then a sequence.
// The modifier auto-selects Cmd on Mac layers, Ctrl on Windows layers.

#ifdef LEADER_ENABLE

void leader_end_user(void) {
    // Pick the right modifier based on active base layer
    bool on_mac = (layer_state_is(MAC_BASE) || layer_state_is(MAC_FN1));

    if (leader_sequence_one_key(KC_W)) {
        tap_code16(on_mac ? LGUI(KC_W) : LCTL(KC_W));  // Close tab/window
    } else if (leader_sequence_one_key(KC_Q)) {
        tap_code16(on_mac ? LGUI(KC_Q) : LCTL(KC_Q));  // Quit
    } else if (leader_sequence_one_key(KC_S)) {
        tap_code16(on_mac ? LGUI(KC_S) : LCTL(KC_S));  // Save
    } else if (leader_sequence_one_key(KC_F)) {
        tap_code16(on_mac ? LGUI(KC_F) : LCTL(KC_F));  // Find
    } else if (leader_sequence_one_key(KC_A)) {
        tap_code16(on_mac ? LGUI(KC_A) : LCTL(KC_A));  // Select all
    } else if (leader_sequence_one_key(KC_C)) {
        tap_code16(on_mac ? LGUI(KC_C) : LCTL(KC_C));  // Copy
    } else if (leader_sequence_one_key(KC_V)) {
        tap_code16(on_mac ? LGUI(KC_V) : LCTL(KC_V));  // Paste
    } else if (leader_sequence_one_key(KC_X)) {
        tap_code16(on_mac ? LGUI(KC_X) : LCTL(KC_X));  // Cut
    } else if (leader_sequence_one_key(KC_Z)) {
        tap_code16(on_mac ? LGUI(KC_Z) : LCTL(KC_Z));  // Undo
    } else if (leader_sequence_one_key(KC_T)) {
        tap_code16(on_mac ? LGUI(KC_T) : LCTL(KC_T));  // New tab
    }
}

#endif // LEADER_ENABLE

// =============================================================================
// Keymaps
// =============================================================================

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [MAC_BASE] = LAYOUT_iso_68(
        TD(TD_ESC_CAPS), KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   TD(TD_BSPC_DEL),    KC_MUTE,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      KC_DEL,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,  KC_ENT,             KC_HOME,
        KC_LSFT,  KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,  KC_UP,
        KC_LCTL,  KC_LOPTN, KC_LCMMD,                               KC_SPC,                                 KC_RCMMD, FN1_MAC,  FN2,      KC_LEFT,  KC_DOWN,  KC_RGHT),

    [WIN_BASE] = LAYOUT_iso_68(
        TD(TD_ESC_CAPS), KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   TD(TD_BSPC_DEL),    KC_MUTE,
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
        KC_ESC,   _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  KC_BSPC,            _______,
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
};

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
};
#endif
