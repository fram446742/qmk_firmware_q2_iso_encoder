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
// Transparent tap-dance override  (no QMK TAP_DANCE_ENABLE needed)
// ═════════════════════════════════════════════════════════════════════════════
// Intercepts base-layer keycodes and provides tap/double-tap behavior
// WITHOUT modifying the keymap or using TD() codes.
// The keymap keeps plain KC_BSPC, KC_ESC, KC_E — this layer sits on top.
//
// When the feature flag is OFF, the keys work normally.
// When ON, the timer-based state machine detects double taps.
//
// To add a new override, add an entry to tap_overrides[] below.
// Each base_kc can appear at most once.  Duplicate base_kc entries
// (e.g. KC_LBRC for both "{" and "[") must be resolved by commenting one.

// ── Three mutually exclusive double-tap action types ─────────────────────
typedef enum {
    TD_DBL_KEYCODE,     // .dbl_kc: regular QMK keycode (e.g. KC_DEL, CW_TOGG)
    TD_DBL_UNICODE_STR, // .str:   Unicode string (e.g. "€")
    TD_DBL_UNICODE_CDP,  // .cdp:    Raw codepoint (e.g. 0x20AC for €)
} td_dbl_type_t;

typedef struct {
    uint16_t      base_kc;     // the key to intercept
    uint16_t      tap_kc;      // sent on single tap
    td_dbl_type_t dbl_type;    // selects which union member is active
    union {
        uint16_t    dbl_kc;   // for TD_DBL_KEYCODE
        const char *str;      // for TD_DBL_UNICODE_STR
        uint32_t    cdp;       // for TD_DBL_UNICODE_CDP
    }; // anonymous union — access directly as ov->dbl_kc / .str / .cdp
} tap_override_t;

// ── Duplicate check ────────────────────────────────────────────────────────
#define TAPDUP(kc) TAPDUPCHECK_##kc
enum {
    TAPDUP(KC_BSPC) = 0, TAPDUP(KC_ESC)  = 0,
    TAPDUP(KC_E)    = 0, TAPDUP(KC_2)    = 0,
    TAPDUP(KC_3)    = 0, TAPDUP(KC_5)    = 0,
    TAPDUP(KC_6)    = 0, TAPDUP(KC_GRV)  = 0,
    TAPDUP(KC_BSLS) = 0, TAPDUP(KC_LBRC) = 0,
    TAPDUP(KC_RBRC) = 0, TAPDUP(KC_NUBS) = 0,
};

// ── Override table ───────────────────────────────────────────────────────────
//  TD idx   type         base        tap         action
//  ───────  ───────────  ──────────  ──────────  ────────────────────────────
static const tap_override_t PROGMEM tap_overrides[] = {
    // TD(0x5700)  keycode
    {.base_kc = KC_BSPC, .tap_kc = KC_BSPC, .dbl_type = TD_DBL_KEYCODE,    .dbl_kc = KC_DEL},
    // TD(0x5701)  keycode
    {.base_kc = KC_ESC,  .tap_kc = KC_ESC,  .dbl_type = TD_DBL_KEYCODE,    .dbl_kc = CW_TOGG},
    // TD(0x5702)  unicode string
    {.base_kc = KC_E,    .tap_kc = KC_E,    .dbl_type = TD_DBL_UNICODE_STR, .str   = "€"},
    // TD(0x5703)  unicode string
    {.base_kc = KC_2,    .tap_kc = KC_2,    .dbl_type = TD_DBL_UNICODE_STR, .str   = "@"},
    // TD(0x5704)  unicode string
    {.base_kc = KC_3,    .tap_kc = KC_3,    .dbl_type = TD_DBL_UNICODE_STR, .str   = "#"},
    // TD(0x5705)  unicode string
    {.base_kc = KC_5,    .tap_kc = KC_5,    .dbl_type = TD_DBL_UNICODE_STR, .str   = "½"},
    // TD(0x5706)  unicode string
    {.base_kc = KC_6,    .tap_kc = KC_6,    .dbl_type = TD_DBL_UNICODE_STR, .str   = "¬"},
    // TD(0x5707)  unicode string
    {.base_kc = KC_GRV,  .tap_kc = KC_GRV,  .dbl_type = TD_DBL_UNICODE_STR, .str   = "~"},
    // TD(0x5708)  unicode string
    {.base_kc = KC_BSLS, .tap_kc = KC_BSLS, .dbl_type = TD_DBL_UNICODE_STR, .str   = "|"},
    // TD(0x5709)  unicode string
    {.base_kc = KC_LBRC, .tap_kc = KC_LBRC, .dbl_type = TD_DBL_UNICODE_STR, .str   = "{"},
    // TD(0x570A)  unicode string
    {.base_kc = KC_RBRC, .tap_kc = KC_RBRC, .dbl_type = TD_DBL_UNICODE_STR, .str   = "}"},
    // TD(0x570B)  unicode string
    {.base_kc = KC_NUBS, .tap_kc = KC_NUBS, .dbl_type = TD_DBL_UNICODE_STR, .str   = "\\"},
};
// An example of each possible double-tap action type.
//    {KC_ESC,  KC_ESC,  .dbl_type = TD_DBL_KEYCODE,    .dbl_kc = CW_TOGG},
//    {KC_E,    KC_E,    .dbl_type = TD_DBL_UNICODE_STR, .str   = "€"},
//    {KC_GRV,  KC_GRV,  .dbl_type = TD_DBL_UNICODE_CDP, .cdp    = 0x007E},



// ── State ───────────────────────────────────────────────────────────────

static int8_t   tap_pending_idx = -1;   // index into tap_overrides[], -1 = none
static uint16_t tap_timer       = 0;

#define TAP_TERM 200  // ms — same as QMK's default tapping term

// ── Helpers ─────────────────────────────────────────────────────────────

static void tap_fire_override(const tap_override_t *ov) {
    switch (ov->dbl_type) {
        case TD_DBL_KEYCODE: {
            uint16_t kc = ov->dbl_kc;
            if (kc >= QK_UNICODE && kc <= QK_UNICODE_MAX) {
                register_unicode(kc & 0x7FFF);
            } else {
                tap_code16(kc);
            }
            break;
        }
        case TD_DBL_UNICODE_STR:
            send_unicode_string(ov->str);
            break;
        case TD_DBL_UNICODE_CDP:
            register_unicode(ov->cdp);
            break;
    }
}

// ── Main processing (called from process_record_user) ────────────────────

static bool process_tap_override(uint16_t keycode, keyrecord_t *record) {
    for (int i = 0; i < ARRAY_SIZE(tap_overrides); i++) {
        if (keycode == pgm_read_word(&tap_overrides[i].base_kc)) {
            if (record->event.pressed) {
                uint16_t now = timer_read();

                // Double tap?
                if (tap_pending_idx == i && timer_elapsed(tap_timer) <= TAP_TERM) {
                    tap_pending_idx = -1;
                    tap_fire_override(&tap_overrides[i]);
                    return false;
                }

                // Different key or timeout — fire pending single if any
                if (tap_pending_idx >= 0) {
                    uint16_t base = pgm_read_word(&tap_overrides[tap_pending_idx].tap_kc);
                    tap_code16(base);
                    tap_pending_idx = -1;
                }

                // Start new pending tap
                tap_pending_idx = i;
                tap_timer       = now;
                return false;  // consume press
            } else {
                // Release: consume if still this key's pending release
                if (tap_pending_idx == i) {
                    return false;  // timer will fire the tap
                }
                return true;
            }
        }
    }
    // Not an overridden key — fire any pending tap then let it through
    if (tap_pending_idx >= 0) {
        uint16_t base = pgm_read_word(&tap_overrides[tap_pending_idx].tap_kc);
        tap_code16(base);
        tap_pending_idx = -1;
    }
    return true;  // let QMK process normally
}

// ── Periodic task (called from matrix_scan_user) ─────────────────────────

static void tap_override_task(void) {
    if (tap_pending_idx >= 0 && timer_elapsed(tap_timer) > TAP_TERM) {
        int8_t idx = tap_pending_idx;
        tap_pending_idx = -1;
        uint16_t kc = pgm_read_word(&tap_overrides[idx].tap_kc);
        tap_code16(kc);
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
        uint16_t base = pgm_read_word(&tap_overrides[tap_pending_idx].tap_kc);
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
