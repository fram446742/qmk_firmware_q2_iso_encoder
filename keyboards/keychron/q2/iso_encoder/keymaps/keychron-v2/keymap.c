/* Copyright 2023 ~ 2025 @ Keychron (https://www.keychron.com)
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ═════════════════════════════════════════════════════════════════════════════
 * Keychron Q2 ISO Encoder — VIA-enabled keymap with extended features
 * ═════════════════════════════════════════════════════════════════════════════
 *
 * LAYERS
 * ──────                                                                    
 * 8 layers: MAC_BASE / WIN_BASE / MAC_FN1 / WIN_FN1 / _FN2 / _FN3 / _FN4 / _FN5
 * - First 5 have default key assignments; _FN3–_FN5 are blank (configurable
 *   via VIA).  FN1 triggers MAC_FN1 (Mac) or WIN_FN1 (Win) depending on base.
 * - Base layers have tap-dance versions of Esc and Backspace (see below).
 *
 * TAP DANCE  (TAP_DANCE_ENABLE)
 * ─────────
 *  TD_BSPC_DEL (TD index 0, keycode 0x5700)
 *    Single tap  → Backspace
 *    Double tap  → Delete
 *    └─ Note: the single-tap feels slightly delayed (tapping-term wait to
 *       distinguish tap vs double-tap).  You can shorten it by adding
 *       `#define TAPPING_TERM 150` in config.h (default is 200ms).
 *
 *  TD_ESC_CAPS (TD index 1, keycode 0x5701)
 *    Single tap  → Escape
 *    Double tap  → Caps-Word toggle (auto-disables after a non-alpha key)
 *
 *  VIA/Launcher shows these as 0x5700 / 0x5701 (QK_TAP_DANCE base).
 *  This is a VIA protocol limitation — the app can't name dynamic TD codes.
 *
 * COMBOS  (COMBO_ENABLE)
 * ──────
 *  Z  +  X          → Toggle Auto-Shift on/off
 *  SPC + Right Shift → Toggle NKRO on/off
 *  O  +  P          → Show feature overview (LED status display)
 *
 *  (A+S=Esc, J+K=Bspc, K+L=Del are in combos.c but commented out — they're
 *   redundant with the base layer keys and cost flash space.)
 *
 * AUTO-SHIFT  (AUTO_SHIFT_ENABLE)
 * ──────────
 *  Long-press any alpha/key → shifted variant (e.g. hold 'a' → 'A').
 *  Toggle with Z+X combo or assign KC_AUTOSHIFT_TOGGLE in VIA.
 *  State is saved to EEPROM and restored on power-on.
 *
 * NKRO  (built-in, toggled via keymap_config.nkro)
 * ────
 *  Toggle with SPC+RSFT combo or assign KC_NKRO_TOGGLE in VIA.
 *  NKRO state is persisted by QMK core via eeconfig.
 *
 * KEY OVERRIDES  (KEY_OVERRIDE_ENABLE)
 * ──────────────
 *  Not active by default.  See combos.c for a commented example.
 *  Uncomment to map Shift + [ISO key left of Z] → Tilde.
 *
 * LEADER KEY  (LEADER_ENABLE)
 * ──────────
 *  Press FN2 + Q to start a leader sequence, then one of:
 *    W/Q/S/F/A/C/V/X/Z/T → platform-aware Cmd/Ctrl+{key}
 *  Mac layers send Cmd, Windows layers send Ctrl.
 *
 * OTHER ENABLED FEATURES (assign keycodes in VIA)
 * ─────────────────────
 *  Caps Word      CW_TOGG (or double-tap Esc via tap dance)
 *  Layer Lock     QK_LAYER_LOCK
 *  Repeat Key     QK_REP / QK_ALT_REP
 *  Dynamic Macro  QK_DYNAMIC_MACRO_1 / _2  (record live, no VIA needed)
 *  Unicode        UC(0xNNNN)
 *
 * FEATURE OVERVIEW
 * ────────────────
 *  Press O+P → all LEDs go black, then shows:
 *    A key green  = Auto-Shift ON   |  A key dim = Auto-Shift OFF
 *    N key white  = NKRO ON          |  N key dim = NKRO OFF
 *  Overview auto-cancels after 2 seconds or on next keypress.
 *  Normal RGB effect is restored afterward.
 *
 * EEPROM LAYOUT
 * ─────────────
 *  Byte 8100 (1 byte): feature toggle bitmask
 *    bit 0 = Auto-Shift enable
 *    bits 1-7 = reserved
 *  First boot (erased EEPROM = 0xFF) defaults all features OFF.
 *  Changing EEPROM layout requires a reflash + EEPROM clear.
 *
 * KEYCODE NAMES IN VIA / KEYCHRON LAUNCHER
 * ────────────────────────────────────────
 *  Tap dance codes (0x57xx) and custom keycodes (0x5Fxx) appear as raw
 *  hex values. This is a VIA protocol limitation — dynamic keycodes can't
 *  be named in the keyboard definition.  They still work correctly.
 *
 * TROUBLESHOOTING
 * ───────────────
 *  - Backspace feels slow?  That's the tap-dance tapping term.  Lower
 *    TAPPING_TERM (e.g. 150) in config.h or replace TD(TD_BSPC_DEL) with
 *    plain KC_BSPC on the base layers.
 *  - Combos not firing?  Make sure all combo keys are pressed within the
 *    50ms COMBO_TERM window — press them simultaneously, not sequentially.
 *  - Feature overview not showing?  Make sure RGB matrix is enabled
 *    (VIA lighting tab → any effect selected).
 *  - Build errors after editing combos.c?  The file is #included from
 *    keymap.c, so it's in the same translation unit as keymap_introspection.
 *    Don't add combos.c to SRC in rules.mk (it's already handled).
 *
 * BITWISE vs BITFIELD
 * ────────────────────
 *  Feature flags use explicit `uint8_t` bitwise ops rather than a struct of
 *  bitfields.  Both compile to identical machine code.  The explicit style
 *  wins here because:
 *    1. Direct byte read/write to EEPROM (no union cast needed)
 *    2. Atomic toggle with a single XOR:  flags ^= (1 << N)
 *    3. No padding/endianness surprises across compiler versions
 *    4. Bitwise AND/OR for batch operations:  flags & (A|B|C)
 *  So yes, explicit bitwise IS worth it for this use case.
 */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "send_string.h"
#include "features.h"
#include "indicators.h"
#include "combos.h"

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

// =============================================================================
// User callbacks — feature toggles, indicators, animation overview
// =============================================================================

#if defined(COMBO_ENABLE) || defined(KEY_OVERRIDE_ENABLE)
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        switch (keycode) {
#ifdef COMBO_ENABLE
            case KC_AUTOSHIFT_TOGGLE:
                feature_toggle_auto_shift();
                return false;
            case KC_NKRO_TOGGLE:
                clear_keyboard();
                keymap_config.nkro = !keymap_config.nkro;
                return false;
            case KC_FEAT_OVERVIEW:
                feature_overview_trigger();
                return false;
#endif
        }
    }
    return true;
}
#endif

void keyboard_post_init_user(void) {
    features_init();
}

void matrix_scan_user(void) {
    indicator_task();
}

#if defined(RGB_MATRIX_ENABLE)
bool rgb_matrix_indicators_user(void) {
    indicator_draw();
    return false;
}
#endif
