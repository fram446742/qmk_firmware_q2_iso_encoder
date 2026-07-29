/* Copyright 2023 ~ 2025 @ Keychron (https://www.keychron.com)
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ═════════════════════════════════════════════════════════════════════════════
 * Keychron Q2 ISO Encoder — VIA-enabled keymap with extended features
 * ═════════════════════════════════════════════════════════════════════════════
 *
 */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "send_string.h"
#include "features.h"
#include "indicators.h"
#include "combos.h"
#include "action_layer.h"    // layer_invert

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
    _FN6,
};

#define FN1_MAC MO(MAC_FN1)
#define FN1_WIN MO(WIN_FN1)
#define FN2     MO(_FN2)

// =============================================================================
// Tap Dance — custom callbacks (CUSTOM_TAP_DANCE_DOUBLE)
// =============================================================================
// Instead of the built-in ACTION_TAP_DANCE_DOUBLE, we define our own
// callback so any keycode (including CW_TOGG, UC() etc.) works reliably
// and custom behaviours can be added without touching QMK core.
//
// To add a new tap dance:
// ═════════════════════════════════════════════════════════════════════════════
// Transparent tap-dance override — loads from EEPROM
// ═════════════════════════════════════════════════════════════════════════════
// Override definitions now live in EEPROM (features_load_config).
// The compile-time table below is preserved as reference but NOT compiled.
// To change defaults, edit features_load_defaults() in features.c.
// The type td_dbl_type_t and eeprom_tap_t are defined in features.h.
//
// ── Old compile-time table (preserved as reference in features.c defaults)



// ── State ───────────────────────────────────────────────────────────────

static int8_t   tap_pending_idx = -1;   // index into tap_overrides[], -1 = none
static uint16_t tap_timer       = 0;

#define TAP_TERM 200  // ms — same as QMK's default tapping term

// ── Helpers ─────────────────────────────────────────────────────────────

// ── Helper: fire double-tap from eeprom_tap entry ───────────────────────

static void tap_fire_override(const eeprom_tap_t *ov) {
    switch (ov->dbl_type) {
        case TD_DBL_KEYCODE: {
            uint16_t kc = ov->dbl_val;
            if (kc >= QK_UNICODE && kc <= QK_UNICODE_MAX) {
                register_unicode(kc & 0x7FFF);
            } else {
                tap_code16(kc);
            }
            break;
        }
        case TD_DBL_UNICODE_STR: {
            // Reconstruct up to 4 UTF-8 bytes from dbl_val/dbl_extra
            uint8_t buf[5];
            buf[0] =  ov->dbl_val       & 0xFF;
            buf[1] = (ov->dbl_val >> 8) & 0xFF;
            buf[2] =  ov->dbl_extra     & 0xFF;
            buf[3] = (ov->dbl_extra >> 8) & 0xFF;
            buf[4] = 0;
            send_unicode_string((const char *)buf);
            break;
        }
        case TD_DBL_UNICODE_CP:
            register_unicode(ov->dbl_val | ((uint32_t)ov->dbl_extra << 16));
            break;
    }
}

// ── Main processing ─────────────────────────────────────────────────────

static bool process_tap_override(uint16_t keycode, keyrecord_t *record) {
    for (int i = 0; i < eeprom_tap_count; i++) {
        if (keycode == eeprom_tap[i].base_kc) {
            if (record->event.pressed) {
                uint16_t now = timer_read();

                if (tap_pending_idx == i && timer_elapsed(tap_timer) <= TAP_TERM) {
                    tap_pending_idx = -1;
                    tap_fire_override(&eeprom_tap[i]);
                    return false;
                }

                if (tap_pending_idx >= 0) {
                    tap_code16(eeprom_tap[tap_pending_idx].tap_kc);
                    tap_pending_idx = -1;
                }

                tap_pending_idx = i;
                tap_timer       = now;
                return false;
            } else {
                if (tap_pending_idx == i) return false;
                return true;
            }
        }
    }
    if (tap_pending_idx >= 0) {
        tap_code16(eeprom_tap[tap_pending_idx].tap_kc);
        tap_pending_idx = -1;
    }
    return true;
}

// ── Periodic task ────────────────────────────────────────────────────────

static void tap_override_task(void) {
    if (tap_pending_idx >= 0 && timer_elapsed(tap_timer) > TAP_TERM) {
        int8_t idx = tap_pending_idx;
        tap_pending_idx = -1;
        tap_code16(eeprom_tap[idx].tap_kc);
    }
}

// =============================================================================
// Combos — defined in combos.c, #included here so keymap_introspection sees them
// =============================================================================

#ifdef COMBO_ENABLE
#    include "combos.c"
#endif

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
        KC_ESC,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,            KC_MUTE,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      KC_DEL,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,  KC_ENT,             KC_HOME,
        KC_LSFT,  KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,  KC_UP,
        KC_LCTL,  KC_LOPTN, KC_LCMMD,                               KC_SPC,                                 KC_RCMMD, FN1_MAC,  FN2,      KC_LEFT,  KC_DOWN,  KC_RGHT),

    [WIN_BASE] = LAYOUT_iso_68(
        KC_ESC,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,            KC_MUTE,
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

    [_FN6] = LAYOUT_iso_68(
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
    [_FN6]     = { ENCODER_CCW_CW(_______, _______) },
};
#endif

// =============================================================================
// User callbacks — feature toggles, indicators, animation overview
// =============================================================================

#if defined(COMBO_ENABLE) || defined(KEY_OVERRIDE_ENABLE)
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // ── Tap-dance override ─────────────────────────────────────────────
    // Intercepts base keys (KC_BSPC, KC_ESC, KC_E, …) and provides
    // tap/double-tap behavior when the feature flag is ON.
    if (feature_tap_dance()) {
        if (!process_tap_override(keycode, record)) return false;
    }

    // ── Interactive overview mode ───────────────────────────────────────
    // Matches on MATRIX POSITION (row, col) — no keycodes involved.
    // Works from any layer, even if the keymap is completely remapped.
    // Timer resets on each interactive keypress (10s from last action).
    if (record->event.pressed && feature_overview_is_active()) {
        uint8_t r = record->event.key.row;
        uint8_t c = record->event.key.col;
        switch ((r << 4) | c) {  // pack row+col into one value
            // Feature toggles — positions from the base layer layout
            case (2 << 4) | 1:  // A
                feature_toggle_auto_shift();    feature_overview_reset_timer(); return false;
            case (2 << 4) | 2:  // S — auto-correct
                autocorrect_toggle();
                feature_overview_reset_timer(); return false;
            case (1 << 4) | 5:  // T
                feature_toggle_tap_dance();     feature_overview_reset_timer(); return false;
            case (3 << 4) | 4:  // C
                feature_toggle_caps_word();     feature_overview_reset_timer(); return false;
            case (1 << 4) | 4:  // R
                feature_toggle_repeat_key();    feature_overview_reset_timer(); return false;
            case (2 << 4) | 3:  // D
                feature_toggle_dyn_macro();     feature_overview_reset_timer(); return false;
            case (2 << 4) | 9:  // L
                feature_toggle_leader();        feature_overview_reset_timer(); return false;
            case (3 << 4) | 7:  // N
                clear_keyboard();
                keymap_config.nkro = !keymap_config.nkro;
                feature_overview_reset_timer(); return false;
            // Layer toggles via number row — TG(N)
            case (0 << 4) | 10:  // 0
                layer_invert(0); feature_overview_reset_timer(); return false;
            case (0 << 4) | 1:   // 1
                layer_invert(1); feature_overview_reset_timer(); return false;
            case (0 << 4) | 2:   // 2
                layer_invert(2); feature_overview_reset_timer(); return false;
            case (0 << 4) | 3:   // 3
                layer_invert(3); feature_overview_reset_timer(); return false;
            case (0 << 4) | 4:   // 4
                layer_invert(4); feature_overview_reset_timer(); return false;
            case (0 << 4) | 5:   // 5
                layer_invert(5); feature_overview_reset_timer(); return false;
            case (0 << 4) | 6:   // 6
                layer_invert(6); feature_overview_reset_timer(); return false;
            case (0 << 4) | 7:   // 7
                layer_invert(7); feature_overview_reset_timer(); return false;
            case (0 << 4) | 8:   // 8
                layer_invert(8); feature_overview_reset_timer(); return false;
            case (0 << 4) | 9:   // 9
                layer_invert(9); feature_overview_reset_timer(); return false;

            // Any other key → exit overview
            default:
                feature_overview_cancel();
                return false;
        }
    }

    // ── Normal processing ───────────────────────────────────────────────
    // If a tap is pending and a different key is pressed, fire the pending tap
    if (feature_tap_dance() && tap_pending_idx >= 0) {
        uint16_t base = eeprom_tap[tap_pending_idx].tap_kc;
        tap_code16(base);
        tap_pending_idx = -1;
    }

    if (record->event.pressed) {
        switch (keycode) {
#ifdef COMBO_ENABLE
            case KC_FEAT_OVERVIEW:
                feature_overview_trigger();
                return false;
#endif
        }
    }
    return true;
}
#endif

// ═════════════════════════════════════════════════════════════════════════════
// HID config handler — used by qmk_config_tool.py for export/import
// ═════════════════════════════════════════════════════════════════════════════
// data[0]=cmd(0x07=set/0x08=get/0x09=save), data[1]=channel(0x00),
// data[2]=value_id, data[3]=index/count, data[4+]=payload

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    uint8_t cmd = data[0];
    uint8_t vid = data[2];
    uint8_t idx = data[3];
    uint8_t *pay = data + 4;
    uint8_t  pay_len = length - 4;

    if (data[1] != 0x00) return;  // not our channel

    if (cmd == 0x08) {  // get
        switch (vid) {
            case 0x01:  // feature flags
                data[3] = g_feature_flags;
                break;
            case 0x02:  // tap override count
                data[3] = eeprom_tap_count;
                break;
            case 0x03:  // tap override entry by index
                if (idx < eeprom_tap_count && pay_len >= sizeof(eeprom_tap_t)) {
                    memcpy(pay, &eeprom_tap[idx], sizeof(eeprom_tap_t));
                }
                break;
            case 0x04:  // combo count
                data[3] = eeprom_combo_count;
                break;
            case 0x05:  // combo entry by index
                if (idx < eeprom_combo_count && pay_len >= sizeof(eeprom_combo_t)) {
                    memcpy(pay, &eeprom_combos[idx], sizeof(eeprom_combo_t));
                }
                break;
            case 0x06:  // leader count
                data[3] = eeprom_leader_count;
                break;
            case 0x07:  // leader entry by index
                if (idx < eeprom_leader_count && pay_len >= sizeof(eeprom_leader_t)) {
                    memcpy(pay, &eeprom_leaders[idx], sizeof(eeprom_leader_t));
                }
                break;
        }
    } else if (cmd == 0x07) {  // set
        switch (vid) {
            case 0x01:
                g_feature_flags = idx;
                features_save();
                break;
            case 0x02:
                eeprom_tap_count = (idx < MAX_TAP_OVERRIDES) ? idx : MAX_TAP_OVERRIDES;
                break;
            case 0x03:
                if (idx < eeprom_tap_count && pay_len >= sizeof(eeprom_tap_t)) {
                    memcpy(&eeprom_tap[idx], pay, sizeof(eeprom_tap_t));
                }
                break;
            case 0x04:
                eeprom_combo_count = (idx < MAX_COMBOS) ? idx : MAX_COMBOS;
                break;
            case 0x05:
                if (idx < eeprom_combo_count && pay_len >= sizeof(eeprom_combo_t)) {
                    memcpy(&eeprom_combos[idx], pay, sizeof(eeprom_combo_t));
                }
                break;
            case 0x06:
                eeprom_leader_count = (idx < MAX_LEADERS) ? idx : MAX_LEADERS;
                break;
            case 0x07:
                if (idx < eeprom_leader_count && pay_len >= sizeof(eeprom_leader_t)) {
                    memcpy(&eeprom_leaders[idx], pay, sizeof(eeprom_leader_t));
                }
                break;
        }
    } else if (cmd == 0x09) {  // save
        features_save_config();
    }
}

void keyboard_post_init_user(void) {
    features_init();
}

void matrix_scan_user(void) {
    indicator_task();
    tap_override_task();
}

#if defined(RGB_MATRIX_ENABLE)
bool rgb_matrix_indicators_user(void) {
    indicator_draw();
    return false;
}
#endif
