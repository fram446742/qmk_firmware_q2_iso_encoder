/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ═════════════════════════════════════════════════════════════════════════════
 *  KEYMAP CONFIGURATION  —  the ONE place to tune everything
 * ═════════════════════════════════════════════════════════════════════════════
 *
 *  Every user-facing constant for this keymap lives here, grouped by feature.
 *  Change a value, rebuild, flash.  Feature code only ever reads from this file.
 *
 *  ══  TABLE OF CONTENTS  ═══════════════════════════════════════════════════
 *    § 1  Layers                       (names, count, jump helpers)
 *    § 2  Feature overview             (O + [ screen: entry, timeout)
 *    § 3  Layer mode / picker          (knob-hold screen: hold/timeout)
 *    § 4  Layer visualization          (key-category overlay colors + timing)
 *    § 5  Tap overrides                (double-tap: types, timing, defaults)
 *    § 6  Leader key                   (modifier auto-select)
 *    § 7  Combos                       (position combos + native-combo guard)
 *    § 8  Runtime feature flags        (the 7 EEPROM on/off bits)
 *    § 9  EEPROM layout                (structs + addresses)
 *    §10  HID protocol VALUE ids       (shared with qmk_config_tool.py)
 *    §11  Indicator LED indices        (which physical LED lights what)
 *    §12  Boot-time defines (config.h) (VIA limits, lock LEDs — include order)
 *
 *  ══  INCLUDE ORDER  ══════════════════════════════════════════════════════
 *  Include this AFTER QMK_KEYBOARD_H + keychron_common.h (needs KC_* keycodes,
 *  COMBO_END, NEW_SAFE_RANGE, …).  Do NOT include it from config.h — that file
 *  is processed before the QMK headers are available (see §12).
 *
 *  The Python tool (qmk_config_tool.py) parses this file for the HID protocol
 *  constants (VALUE_*, EEP_*, the EEPROM structs, and the *_DEFAULTS lists),
 *  so do not rename or renumber anything in §9–§10.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

// Auto-generated — matrix positions (POS_KC_xxx) and LED indices (POS_IDX_xxx)
// from keyboard.json + the MAC_BASE layer.  Regenerated every build.
#include "key_positions.h"


/* ═══════════════════════════════════════════════════════════════════════════
 * §1  LAYERS
 * ═══════════════════════════════════════════════════════════════════════════ */

// Adding a layer: insert it above _FN6 and bump KEYMAP_LAYER_COUNT.
// Removing a layer: delete it and adjust KEYMAP_LAYER_COUNT.
#define KEYMAP_LAYER_COUNT 9

enum layers {
    MAC_BASE,   // 0  macOS default layer
    WIN_BASE,   // 1  Windows default layer
    MAC_FN1,    // 2  macOS Fn layer (hold FN1_MAC)
    WIN_FN1,    // 3  Windows Fn layer (hold FN1_WIN)
    _FN2,       // 4
    _FN3,       // 5
    _FN4,       // 6
    _FN5,       // 7
    _FN6,       // 8
};

// Convenience aliases for the momentary Fn keys used in the keymap.
#define FN1_MAC  MO(MAC_FN1)
#define FN1_WIN  MO(WIN_FN1)
#define FN2      MO(_FN2)

// Number-row layer jump used by the overview / layer-mode screens:
// if already on layer N, jump back to the default layer; otherwise go to N.
#define LAYER_MOVE_OR_DEFAULT(N) do {                                      \
    if (get_highest_layer(layer_state) == (N))                             \
        layer_move(get_highest_layer(default_layer_state));                \
    else                                                                    \
        layer_move(N);                                                      \
} while (0)


/* ═══════════════════════════════════════════════════════════════════════════
 * §2  FEATURE OVERVIEW  —  the O + [ config screen
 * ═══════════════════════════════════════════════════════════════════════════ */

// Auto-exit when the overview is left idle.  0 = never auto-exit.
#define OVERVIEW_TIMEOUT_MS 10000

// The overview opens by pressing O and [ TOGETHER.  It is a POSITION combo
// (defined in §8 POS_COMBOS_DEFS): the keys are matched by PHYSICAL matrix
// position (POS_KC_O = (1,9), POS_KC_LBRC = (1,11)) — never by keycode — so
// it fires on ANY layer, including blank ones where the keys resolve to
// nothing.  This is a deliberate divergence from QMK-native combos (which can
// only match the resolved keycode); documented in DIVERGENCES.md §4.x.
//
// The combo fires KC_FEAT_OVERVIEW, which the keymap's process_record_user()
// maps to feature_overview_trigger().  COMBO_TERM (QMK default 50 ms) governs
// the chord window; an incomplete single key is re-pressed so typing O / [
// alone still works.

// Custom keycode fired by the overview position combo (§8).  The keymap maps
// it to feature_overview_trigger().
enum { KC_FEAT_OVERVIEW = NEW_SAFE_RANGE };


/* ═══════════════════════════════════════════════════════════════════════════
 * §3  LAYER MODE / LAYER PICKER  —  hold the knob to pick a layer
 * ═══════════════════════════════════════════════════════════════════════════ */

// Knob button held this long (ms) → enter layer mode (only layer LEDs light).
#define LAYER_PICKER_HOLD_MS     3000
// Auto-exit when the picker is left idle.  0 = never auto-exit.
#define LAYER_PICKER_TIMEOUT_MS 10000
// Gap (ms) between the replayed press and release of a short knob tap, so the
// host sees a distinct media-key press (e.g. mute) instead of a collapsed one.
#define KNOB_TAP_RELEASE_DELAY_MS 6

// (The knob button's physical key is held back too, so the key mapped to it —
// e.g. KC_MUTE — only fires on a short press; a long press never leaks it.)


/* ═══════════════════════════════════════════════════════════════════════════
 * §4  LAYER VISUALIZATION  —  color every key by category on layer change
 * ═══════════════════════════════════════════════════════════════════════════ */

// How long the color overlay stays after a layer change / on the vis-lock.
#define LAYER_VIS_TIMEOUT_MS 1500

// How many momentary Fn (MO) keys the visualizer tracks at once (held together).
#define MAX_HELD_MO 8

// While an RGB/underglow key is pressed, show the raw effect for this long so
// you can see the change; the layer overlay resumes afterwards.
#define RGB_FEEDBACK_DURATION_MS 1000

// ── Colors per key category  {R, G, B} ─────────────────────────────────────
#define LV_COLOR_BYPASS    {5, 5, 5}       ///< transparent keys (_______) — dim grey
#define LV_COLOR_BLANK     {0, 0, 0}       ///< KC_NO — off
#define LV_COLOR_MODIFIER  {255, 140, 0}   ///< Ctrl, Shift, Alt, Win — amber
#define LV_COLOR_MAC_EXTRA {0, 190, 255}   ///< Mac Option/Cmd — sky blue
#define LV_COLOR_FUNCTION  {100, 220, 80}  ///< F1-F24 — lime green
#define LV_COLOR_BASIC     {0, 170, 0}     ///< alpha, numbers, navigation — green
#define LV_COLOR_MEDIA     {0, 90, 230}    ///< media keys — medium blue
#define LV_COLOR_MACRO     {255, 0, 200}   ///< macro keys — magenta
#define LV_COLOR_SPECIAL   {200, 100, 0}   ///< Keychron custom (KC_TASK, …) — dark orange
#define LV_COLOR_LIGHT     {200, 200, 0}   ///< lighting/RGB keys — yellow
#define LV_COLOR_CUSTOM    {120, 0, 255}   ///< user custom keycodes — purple
#define LV_COLOR_LAYER     {255, 0, 0}     ///< layer management — red


/* ═══════════════════════════════════════════════════════════════════════════
 * §5  OVERLAY ROLE COLORS  —  indicator LEDs shared by the screens
 * ═══════════════════════════════════════════════════════════════════════════ */

// RGB triplets used by the feature-overview and layer-mode screens (and the
// caps-lock overlay).  Add roles here as you add screens; keep every draw site
// reading from these macros, never hardcoded.
#define IND_LAYER_ACTIVE   {255, 255, 255}  ///< the layer you're on (white)
#define IND_LAYER_CHOICE   {255, 0, 0}      ///< another layer you can jump to (red)
#define IND_FEATURE_ON     {255, 255, 255}  ///< a feature toggle that is ON (white)
#define IND_FEATURE_OFF    {255, 0, 0}      ///< a feature toggle that is OFF (red)
#define IND_CAPS_LOCK_ON   {255, 255, 255}  ///< caps-lock drawn over the overlay


/* ═══════════════════════════════════════════════════════════════════════════
 * §6  TAP OVERRIDES  —  double-tap a key to fire an action (no QMK tap dance)
 * ═══════════════════════════════════════════════════════════════════════════ */

// Double-tap window (ms).
#define TAP_TERM 200

// What a double-tap does:
typedef enum {
    TD_DBL_KEYCODE     = 0,  ///< a single keycode (or modded, e.g. S(KC_2))
    TD_DBL_UNICODE_STR = 1,  ///< Unicode string via send_unicode_string()
    TD_DBL_UNICODE_CP  = 2,  ///< Unicode codepoint via register_unicode()
    TD_DBL_SEND_STRING = 3,  ///< ASCII string (≤4 chars) via send_string()
} td_dbl_type_t;

// Default double-tap overrides, loaded at first boot (or when EEPROM is blank).
// Entries use eeprom_tap_t field initializers; the count is derived at compile
// time via sizeof, so add/remove/reorder freely.  The {0} sentinel stays last.
//
//   base_type=0 → base_id is a KC_xxx keycode (follows the key's label)
//   base_type=1 → base_id is a POS_KC_xxx matrix position (follows the key)
#define TAP_DEFAULTS \
    {.base_id=QK_GESC, .tap_kc=QK_GESC, .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(QK_GESC)}, \
    {.base_id=KC_E,    .tap_kc=KC_E,    .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_E)},    \
    {.base_id=KC_1,    .tap_kc=KC_1,    .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_1)},    \
    {.base_id=KC_2,    .tap_kc=KC_2,    .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_2)},    \
    {.base_id=KC_3,    .tap_kc=KC_3,    .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_3)},    \
    {.base_id=KC_4,    .tap_kc=KC_4,    .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_4)},    \
    {.base_id=KC_5,    .tap_kc=KC_5,    .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_5)},    \
    {.base_id=KC_6,    .tap_kc=KC_6,    .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_6)},    \
    {.base_id=KC_7,    .tap_kc=KC_7,    .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_7)},    \
    {.base_id=KC_8,    .tap_kc=KC_8,    .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_8)},    \
    {.base_id=KC_9,    .tap_kc=KC_9,    .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_9)},    \
    {.base_id=KC_0,    .tap_kc=KC_0,    .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_0)},    \
    {.base_id=KC_MINS, .tap_kc=KC_MINS, .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_MINS)}, \
    {.base_id=KC_EQL,  .tap_kc=KC_EQL,  .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_EQL)},   \
    {.base_id=KC_BSPC, .tap_kc=KC_BSPC, .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=KC_DEL}, \
    {0}  /* sentinel — all fields zero, keep last */

// The optional combo/leader default lists start empty (sentinel only → count 0).
// {{0}} = nested braces for the first-member array (keys[]/seq[]) to satisfy
// -Werror=missing-braces.  Add real entries above the sentinel to ship with
// default combos/leaders.
#define COMBO_DEFAULTS   {{0}}  // sentinel-only = empty list
#define LEADER_DEFAULTS  {{0}}  // sentinel-only = empty list


/* ═══════════════════════════════════════════════════════════════════════════
 * §7  LEADER KEY  —  FN2+Q then a key
 * ═══════════════════════════════════════════════════════════════════════════ */

// Modifier auto-selects Cmd on the Mac layers, Ctrl on the Windows layers.
#define LEADER_MOD(on_mac, base_kc)  ((on_mac) ? LGUI(base_kc) : LCTL(base_kc))

// QK_LEAD is placed on _FN2 at the Q position (see keymap.c).  The leader
// timeout (LEADER_TIMEOUT in quantum/leader.h) defaults to 300 ms.


/* ═══════════════════════════════════════════════════════════════════════════
 * §8  COMBOS  —  position combos + the native-combo reserved-key guard
 * ═══════════════════════════════════════════════════════════════════════════ */

// ── Custom position combos (features.c) ─────────────────────────────────────
// Match by matrix position (BASE_IS_MATRIX, POS_KC_*) or keycode
// (BASE_IS_KEYCODE).  Independent of QMK-native combos (combos.c).  These are
// the ONLY combos that can match a physical key on any layer.

typedef struct {
    uint8_t  key_count;   ///< number of keys in this combo (1-4)
    uint8_t  base_type;   ///< tap_base_type_t — keycode vs matrix position
    uint16_t keys[4];     ///< values to match; unused = 0
    uint16_t output;      ///< keycode to fire when all keys held
} pos_combo_def_t;

#define POS_COMBO(count, type, out, ...) \
    { .key_count = (count), .base_type = (type), .keys = {__VA_ARGS__}, .output = (out) }

// Compile-time guard: these keys belong to the feature-overview entry below,
// so no OTHER position combo may reuse them (duplicate enum member → error).
#define CKPOS(p) POSCOMBO_KEYCHECK_##p
enum pos_combo_reserved_check {
    CKPOS(POS_KC_O),     // feature-overview entry (below) — reserved
    CKPOS(POS_KC_LBRC),  // feature-overview entry (below) — reserved
    // Register every key of each new combo you add here too, e.g.:
    // CKPOS(POS_KC_Q), CKPOS(POS_KC_W),
    POSCOMBO_KEYCHECK_END,
};
#undef CKPOS

// ── Runtime position-combo definitions ──────────────────────────────────────
// The FIRST entry is the feature-overview chord (O + [ by physical position,
// §2).  Add more combos after it; also list their keys in the reserve enum
// above.
#define POS_COMBOS_DEFS \
    POS_COMBO(2, BASE_IS_MATRIX, KC_FEAT_OVERVIEW, POS_KC_O, POS_KC_LBRC),


// (QMK-native keycode combos live in combos.c `key_combos[]`; their own guard
//  there rejects KC_O / KC_LBRC the same way.)


/* ═══════════════════════════════════════════════════════════════════════════
 * §9  RUNTIME FEATURE FLAGS  —  one EEPROM byte, toggled in the overview
 * ═══════════════════════════════════════════════════════════════════════════ */

#define FEATURE_TAP_DANCE   (1 << 0)  ///< Tap-dance keycode override
#define FEATURE_AUTO_SHIFT  (1 << 1)  ///< Auto-shift on/off
#define FEATURE_CAPS_WORD   (1 << 2)  ///< Caps Word processing
#define FEATURE_REPEAT_KEY  (1 << 3)  ///< Repeat / Alt-Repeat processing
#define FEATURE_DYN_MACRO   (1 << 4)  ///< Dynamic Macro processing
#define FEATURE_LEADER      (1 << 5)  ///< Leader key sequences
#define FEATURE_LAYER_VIS   (1 << 6)  ///< Layer visualization overlay
// bit 7 reserved

/// Flags enabled at first boot  (Caps Word + Repeat + Layer Vis ON, rest OFF)
#define DEFAULT_FEATURE_FLAGS (FEATURE_CAPS_WORD | FEATURE_REPEAT_KEY | FEATURE_LAYER_VIS)


/* ═══════════════════════════════════════════════════════════════════════════
 * §10  EEPROM LAYOUT  —  per-feature counts + packed on-disk structs
 * ═══════════════════════════════════════════════════════════════════════════ */

// How many entries each feature can hold (upper bounds).
#define MAX_TAP_OVERRIDES 20
#define MAX_COMBOS         8
#define MAX_LEADERS       16

// How base_id is interpreted (used by tap entries AND position combos).
typedef enum {
    BASE_IS_KEYCODE = 0,  ///< a QMK keycode (e.g. KC_BSPC)
    BASE_IS_MATRIX  = 1,  ///< a packed matrix position (row << 8 | col)
} tap_base_type_t;

// ── Packed EEPROM structs (binary layout matches qmk_config_tool.py) ──────
typedef struct __attribute__((packed)) {
    uint16_t base_id;       ///< keycode (type=0) or matrix position (type=1)
    uint16_t tap_kc;        ///< keycode to fire for a single tap
    uint8_t  dbl_type;      ///< td_dbl_type_t
    uint8_t  base_type;     ///< tap_base_type_t
    uint16_t dbl_val;       ///< depends on dbl_type
    uint16_t dbl_extra;     ///< depends on dbl_type
} eeprom_tap_t;

typedef struct __attribute__((packed)) {
    uint16_t keys[4];     ///< keycodes/positions; 0 = terminator
    uint16_t output;      ///< keycode to fire
} eeprom_combo_t;

typedef struct __attribute__((packed)) {
    uint8_t  seq[3];      ///< keycodes in sequence; 0 = terminator
    uint8_t  mod;         ///< QMK MOD_* value, not a keycode (e.g. MOD_LGUI = 0x08)
    uint16_t key;         ///< final keycode to fire
} eeprom_leader_t;

// ── Addresses (derived from the counts/structs above — edit those only) ────
//  8100         Feature flags (1 B)
//  8101         Tap count (1 B) + MAX_TAP_OVERRIDES × eeprom_tap_t
//  NEXT         Combo count (1 B) + MAX_COMBOS × eeprom_combo_t
//  NEXT         Leader count (1 B) + MAX_LEADERS × eeprom_leader_t
//
//  Total ≈ 380 B — comfortably inside the 2 KB user-data area past VIA's buffer.
#define EEP_FEATURES        8100
#define EEP_TAP_BASE        (EEP_FEATURES + 1)
#define EEP_TAP_SIZE        (MAX_TAP_OVERRIDES * sizeof(eeprom_tap_t))

#define EEP_COMBO_BASE      (EEP_TAP_BASE + 1 + EEP_TAP_SIZE)
#define EEP_COMBO_SIZE      (MAX_COMBOS * sizeof(eeprom_combo_t))

#define EEP_LEADER_BASE     (EEP_COMBO_BASE + 1 + EEP_COMBO_SIZE)
#define EEP_LEADER_SIZE     (MAX_LEADERS * sizeof(eeprom_leader_t))


/* ═══════════════════════════════════════════════════════════════════════════
 * §11  HID PROTOCOL VALUE IDs  —  via_custom_value_command_kb + Python tool
 * ═══════════════════════════════════════════════════════════════════════════ */

#define VALUE_FLAGS        0x01
#define VALUE_TAP_COUNT    0x02
#define VALUE_TAP_ENTRY    0x03
#define VALUE_COMBO_COUNT  0x04
#define VALUE_COMBO_ENTRY  0x05
#define VALUE_LEADER_COUNT 0x06
#define VALUE_LEADER_ENTRY 0x07


/* ═══════════════════════════════════════════════════════════════════════════
 * §12  INDICATOR LED INDICES  —  which physical LED lights what
 * ═══════════════════════════════════════════════════════════════════════════ */

// Derived from the auto-generated POS_IDX_xxx macros (physical LED of each key,
// so they follow the key even if its keycode is remapped).  Only CAPS_LOCK is
// lit in normal operation; the rest illuminate in the overview screen.
//
//  Key   Purpose
//  ───   ──────────────────────────
//  A     Auto-Shift ON
//  S     Auto-Correct ON
//  T     Tap Dance ON
//  C     Caps Word processing ON
//  R     Repeat Key ON
//  D     Dynamic Macro ON
//  L     Leader Key ON
//  N     NKRO ON
//  9     Layer-visualization lock ON (overview mode)
#define IND_AUTO_SHIFT  POS_IDX_KC_A
#define IND_TAP_DANCE   POS_IDX_KC_T
#define IND_CAPS_WORD   POS_IDX_KC_C
#define IND_REPEAT_KEY  POS_IDX_KC_R
#define IND_DYN_MACRO   POS_IDX_KC_D
#define IND_LEADER      POS_IDX_KC_L
#define IND_AUTOCORRECT POS_IDX_KC_S
#define IND_NKRO        POS_IDX_KC_N
#define IND_VIS_LOCK    POS_IDX_KC_9


/* ═══════════════════════════════════════════════════════════════════════════
 * §13  BOOT-TIME DEFINES  —  these MUST stay in the keymap config.h
 * ═══════════════════════════════════════════════════════════════════════════ */

// VIA/dynamic-keymap limits (DYNAMIC_KEYMAP_LAYER_COUNT, _MACRO_COUNT) and the
// lock-LED indices (WINLOCK_LED_LIST; NUM_LOCK_INDEX disabled) live in
// keyboards/keychron/q2/iso_encoder/keymaps/keychron-v2/config.h because that
// file is compiled BEFORE the QMK headers exist (they must override VIA's
// defaults before QMK_KEYBOARD_H is seen).  Everything else is here.
