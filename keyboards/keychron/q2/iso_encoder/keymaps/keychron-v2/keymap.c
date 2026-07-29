/* Copyright 2023 ~ 2025 @ Keychron (https://www.keychron.com)
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ═════════════════════════════════════════════════════════════════════════════
 * Keychron Q2 ISO Encoder — VIA-enabled keymap with extended features
 * ═════════════════════════════════════════════════════════════════════════════
 *
 * LAYERS
 * ──────
 * 9 layers: from MAC_BASE (0) to _FN6 (8).  Feature overview (O+P) lights
 *   the matching number key for the active layer (layer 0 → key 0, etc.).
 * - First 5 have default key assignments.
 * - _FN3–_FN6 are blank — configure via VIA.
 * - FN1 triggers MAC_FN1 (Mac) or WIN_FN1 (Win) depending on base.
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
//   1. Add an enum entry
//   2. Add a user_data array with {tap_kc, double_tap_kc}
//   3. Add the array address to tap_dance_actions[] with CUSTOM_TD_DOUBLE
//   4. Use TD(MY_NEW) in the keymaps
//
// Example already added: TD_E_EURO — tap=E, double=€

#ifdef TAP_DANCE_ENABLE

// ── Types ─────────────────────────────────────────────────────────────────

// Data for CUSTOM_TD_DOUBLE_UNICODE: single-tap keycode + Unicode string.
typedef struct {
    uint16_t     tap_kc;
    const char  *unicode_str;
} td_unicode_pair_t;

// ── Custom pair callbacks ───────────────────────────────────────────────

static void td_double_finished(tap_dance_state_t *state, void *user_data) {
    tap_dance_pair_t *pair = (tap_dance_pair_t *)user_data;
    uint16_t kc = (state->count == 1) ? pair->kc1 : pair->kc2;

    // Unicode codepoints (0x8000-0xBFFF range from UC() macro) need
    // register_unicode() to go through the OS input method.  tap_code16()
    // bypasses the Unicode processing pipeline and sends raw HID codes.
    if (kc >= QK_UNICODE && kc <= QK_UNICODE_MAX) {
        register_unicode(kc & 0x7FFF);
    } else {
        tap_code16(kc);
    }

    reset_tap_dance(state);
}

static void td_double_reset(tap_dance_state_t *state, void *user_data) {
    // No cleanup needed — td_double_finished / td_double_str_finished
    // already do press+release and call reset_tap_dance(state).
    // Calling reset_tap_dance again here would recurse forever.
}

// ── Unicode-string callback ─────────────────────────────────────────────

static void td_double_str_finished(tap_dance_state_t *state, void *user_data) {
    td_unicode_pair_t *pair = (td_unicode_pair_t *)user_data;
    if (state->count == 1) {
        tap_code16(pair->tap_kc);
    } else {
        send_unicode_string(pair->unicode_str);
    }
    reset_tap_dance(state);
}

// ── Helper macros ───────────────────────────────────────────────────────

// For regular keycode pairs:  CUSTOM_TD_DOUBLE(KC_BSPC, KC_DEL)
#define CUSTOM_TD_DOUBLE(kc1, kc2)                                          \
    { .fn = {NULL, td_double_finished, td_double_reset, NULL},               \
      .user_data = (void *)&((tap_dance_pair_t){kc1, kc2}) }

// For Unicode string double-actions:  CUSTOM_TD_DOUBLE_UNICODE(KC_E, "€")
// Works on any OS (Linux IBus, Win Alt-code, Mac Hex Input).
#define CUSTOM_TD_DOUBLE_UNICODE(kc1, str)                                  \
    { .fn = {NULL, td_double_str_finished, td_double_reset, NULL},           \
      .user_data = (void *)&((td_unicode_pair_t){kc1, str}) }

// (We use the built-in td_double_reset from QMK — no custom reset needed.)

// ── Enum ────────────────────────────────────────────────────────────────

enum {
    TD_BSPC_DEL,       // tap = Backspace,  double-tap = Delete
    TD_ESC_CAPS,       // tap = Escape,    double-tap = Caps Word toggle
    TD_E_EURO,         // tap = e,         double-tap = € (U+20AC)
    TD_ESC,
    TD_PIPE,
    TD_AT,
    TD_HASH,
    TD_TILDE,
    TD_HALF,
    TD_NOT,
    TD_LBRACE,
    TD_LBRACKET,
    TD_RBRACKET,
    TD_RBRACE,
    TD_BACKSLASH,
    TD_CEDILLA,
};

// ── Static pair-data arrays (PROGMEM-safe via compound literal in macro) ─

// Three macro patterns available (all shown for reference):
//
//   CUSTOM_TD_DOUBLE(KC_BSPC, KC_DEL)           — two regular keycodes
//   CUSTOM_TD_DOUBLE(KC_ESC,  CW_TOGG)           — keycode + special keycode
//   CUSTOM_TD_DOUBLE(KC_E,    UC(0x20AC))         — keycode + UC() codepoint
//   CUSTOM_TD_DOUBLE_UNICODE(KC_E, "€")          — keycode + Unicode string (OS-agnostic)
//
// The string variant uses send_unicode_string() which works on all OSes.
// The UC() variant is OS-dependent (needs correct Unicode input method).

tap_dance_action_t tap_dance_actions[] = {
    [TD_BSPC_DEL] = CUSTOM_TD_DOUBLE(KC_BSPC,             KC_DEL),
    [TD_ESC_CAPS] = CUSTOM_TD_DOUBLE(KC_ESC,               CW_TOGG),
    //[TD_E_EURO] = CUSTOM_TD_DOUBLE(KC_E,              UC(0x20AC)),   // UC() approach
    [TD_E_EURO]   = CUSTOM_TD_DOUBLE_UNICODE(KC_E,          "€"),       // string approach
    [TD_ESC] = CUSTOM_TD_DOUBLE_UNICODE(KC_ESC, "ª"),
    [TD_PIPE] = CUSTOM_TD_DOUBLE_UNICODE(KC_BSLS, "|"),
    [TD_AT] = CUSTOM_TD_DOUBLE_UNICODE(KC_2, "@"),
    [TD_HASH] = CUSTOM_TD_DOUBLE_UNICODE(KC_3, "#"),
    [TD_TILDE] = CUSTOM_TD_DOUBLE_UNICODE(KC_GRV, "~"),
    [TD_HALF] = CUSTOM_TD_DOUBLE_UNICODE(KC_5, "½"),
    [TD_NOT] = CUSTOM_TD_DOUBLE_UNICODE(KC_6, "¬"),
    [TD_LBRACE] = CUSTOM_TD_DOUBLE_UNICODE(KC_7, "{"),
    [TD_LBRACKET] = CUSTOM_TD_DOUBLE_UNICODE(KC_8, "["),
    [TD_RBRACKET] = CUSTOM_TD_DOUBLE_UNICODE(KC_9, "]"),
    [TD_RBRACE] = CUSTOM_TD_DOUBLE_UNICODE(KC_0, "}"),
    [TD_BACKSLASH] = CUSTOM_TD_DOUBLE_UNICODE(KC_MINS, "\\"),
    [TD_CEDILLA] = CUSTOM_TD_DOUBLE_UNICODE(KC_EQL, "¸"),
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
        TD(TD_ESC_CAPS), KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,    KC_MUTE,
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,                      KC_DEL,
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,  KC_NUHS,  KC_ENT,             KC_HOME,
        KC_LSFT,  KC_NUBS,  KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,  KC_UP,
        KC_LCTL,  KC_LOPTN, KC_LCMMD,                               KC_SPC,                                 KC_RCMMD, FN1_MAC,  FN2,      KC_LEFT,  KC_DOWN,  KC_RGHT),

    [WIN_BASE] = LAYOUT_iso_68(
        TD(TD_ESC_CAPS), KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,    KC_MUTE,
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

// ── Tap dance runtime fence ────────────────────────────────────────────────
// When tap dance is DISABLED, intercept ALL TD keycodes before
// process_tap_dance() sees them and send the plain single-tap keycode.
// The fence runs in preprocess_record_user which fires BEFORE tap dance.
//
// The plain-key fallback table must stay in sync with the enum order.

#if defined(TAP_DANCE_ENABLE) && defined(COMBO_ENABLE)

// Plain-key fallback for each TD index (used when tap dance is disabled).
// The order/indices must match the TD_ enum in this file.
static const uint16_t PROGMEM td_plain_fallback[] = {
    [TD_BSPC_DEL] = KC_BSPC,
    [TD_ESC_CAPS] = KC_ESC,
    [TD_E_EURO]   = KC_E,
};

bool preprocess_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!feature_tap_dance() && IS_QK_TAP_DANCE(keycode)) {
        uint8_t idx = QK_TAP_DANCE_GET_INDEX(keycode);
        if (idx < ARRAY_SIZE(td_plain_fallback)) {
            uint16_t plain = pgm_read_word(&td_plain_fallback[idx]);
            if (record->event.pressed) {
                tap_code16(plain);
            }
        }
        return false;  // block all further processing
    }
    return true;
}
#endif

#if defined(COMBO_ENABLE) || defined(KEY_OVERRIDE_ENABLE)
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        switch (keycode) {
#ifdef COMBO_ENABLE
            case KC_AUTOSHIFT_TOGGLE:
                feature_toggle_auto_shift();
                return false;
            case KC_TAP_DANCE_TOGGLE:
                feature_toggle_tap_dance();
                return false;
            case KC_NKRO_TOGGLE:
                clear_keyboard();
                keymap_config.nkro = !keymap_config.nkro;
                return false;
            case KC_CAPS_WORD_TOGGLE:
                feature_toggle_caps_word();
                return false;
            case KC_REPEAT_KEY_TOGGLE:
                feature_toggle_repeat_key();
                return false;
            case KC_DYN_MACRO_TOGGLE:
                feature_toggle_dyn_macro();
                return false;
            case KC_LEADER_TOGGLE:
                feature_toggle_leader();
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
