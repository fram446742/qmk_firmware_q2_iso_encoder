/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ═════════════════════════════════════════════════════════════════════════════
 * KEYMAP CONFIGURATION — single file for all tunable constants and defaults
 * ═════════════════════════════════════════════════════════════════════════════
 *
 * Change any value here and rebuild.  All feature code reads from this file.
 *
 * Include AFTER QMK_KEYBOARD_H + keychron_common.h (needs KC_* keycodes,
 * COMBO_END, NEW_SAFE_RANGE, etc.)  Do NOT include from config.h — that
 * file is processed before QMK headers are available.
 *
 * The Python tool (qmk_config_tool.py) reads features.h for HID protocol
 * constants (VALUE_*, EEP_*).  Keep features.h in sync or point the
 * Python tool at this file instead.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>


// ═════════════════════════════════════════════════════════════════════════════
// TIMINGS (milliseconds)
// ═════════════════════════════════════════════════════════════════════════════

#define TAP_TERM               200     ///< Tap-dance double-tap timeout
#define OVERVIEW_TIMEOUT_MS  10000     ///< Feature overview auto-exit (0 = no timeout)


// ═════════════════════════════════════════════════════════════════════════════
// FEATURE BIT FLAGS  (stored as uint8_t in EEPROM at address EEP_FEATURES)
// ═════════════════════════════════════════════════════════════════════════════

#define FEATURE_TAP_DANCE   (1 << 0)  ///< Tap-dance keycode override
#define FEATURE_AUTO_SHIFT  (1 << 1)  ///< Auto-shift on/off
#define FEATURE_CAPS_WORD   (1 << 2)  ///< Caps Word processing
#define FEATURE_REPEAT_KEY  (1 << 3)  ///< Repeat / Alt-Repeat processing
#define FEATURE_DYN_MACRO   (1 << 4)  ///< Dynamic Macro processing
#define FEATURE_LEADER      (1 << 5)  ///< Leader key sequences
// bits 6-7 reserved

/// Default feature flags at first boot  (Caps Word + Repeat ON, others OFF)
#define DEFAULT_FEATURE_FLAGS  (FEATURE_CAPS_WORD | FEATURE_REPEAT_KEY)


// ═════════════════════════════════════════════════════════════════════════════
// MAX COUNTS
// ═════════════════════════════════════════════════════════════════════════════

#define MAX_TAP_OVERRIDES  20
#define MAX_COMBOS          8
#define MAX_LEADERS        16


// ═════════════════════════════════════════════════════════════════════════════
// TAP-DANCE DOUBLE-TAP ACTION TYPES  (eeprom_tap_t.dbl_type)
// ═════════════════════════════════════════════════════════════════════════════

typedef enum {
    TD_DBL_KEYCODE      = 0,  ///< Single keycode (or modded, e.g. S(KC_2))
    TD_DBL_UNICODE_STR  = 1,  ///< Unicode string via send_unicode_string()
    TD_DBL_UNICODE_CP   = 2,  ///< Unicode codepoint via register_unicode()
    TD_DBL_SEND_STRING  = 3,  ///< ASCII string (≤4 chars) via send_string()
} td_dbl_type_t;


// ═════════════════════════════════════════════════════════════════════════════
// EEPROM STRUCTS  (packed — binary layout matches Python tool)
// ═════════════════════════════════════════════════════════════════════════════

// How base_id is interpreted in eeprom_tap_t
typedef enum {
    BASE_IS_KEYCODE = 0,   ///< base_id is a QMK keycode (e.g. KC_BSPC) — current behavior
    BASE_IS_MATRIX  = 1,   ///< base_id is a packed matrix position (row << 8 | col)
} tap_base_type_t;

typedef struct __attribute__((packed)) {
    uint16_t base_id;       ///< keycode (type=0) or packed matrix position (type=1)
    uint16_t tap_kc;        ///< keycode to fire for single tap
    uint8_t  dbl_type;      ///< td_dbl_type_t
    uint8_t  base_type;     ///< tap_base_type_t — how to interpret base_id
    uint16_t dbl_val;       ///< depends on dbl_type
    uint16_t dbl_extra;     ///< depends on dbl_type
} eeprom_tap_t;

typedef struct __attribute__((packed)) {
    uint16_t keys[4];     ///< 0 = terminator; up to 4 keys per combo
    uint16_t output;      ///< keycode to fire
} eeprom_combo_t;

typedef struct __attribute__((packed)) {
    uint8_t  seq[3];      ///< keycodes in sequence; 0 = terminator
    uint8_t  mod;         ///< QMK MOD_* value, not a keycode (e.g. MOD_LGUI = 0x08)
    uint16_t key;         ///< final keycode to fire
} eeprom_leader_t;


// ═════════════════════════════════════════════════════════════════════════════
// EEPROM ADDRESSES  (dynamically calculated — edit MAX_* or structs above)
// ═════════════════════════════════════════════════════════════════════════════
//
//  8100         Feature flags (1 B)
//  8101         Tap override count (1 B)
//  8101+1 …     Tap override entries (MAX_TAP_OVERRIDES × sizeof(eeprom_tap_t))
//  NEXT         Combo count (1 B)
//  NEXT+1 …     Combo entries (MAX_COMBOS × sizeof(eeprom_combo_t))
//  NEXT         Leader count (1 B)
//  NEXT+1 …     Leader entries (MAX_LEADERS × sizeof(eeprom_leader_t))
//
//  Change MAX_TAP_OVERRIDES, MAX_COMBOS, MAX_LEADERS, or any struct
//  and everything adjusts automatically (verified by static_assert below).

#define EEP_FEATURES        8100
#define EEP_TAP_BASE        (EEP_FEATURES + 1)
#define EEP_TAP_SIZE        (MAX_TAP_OVERRIDES * sizeof(eeprom_tap_t))

#define EEP_COMBO_BASE      (EEP_TAP_BASE + 1 + EEP_TAP_SIZE)
#define EEP_COMBO_SIZE      (MAX_COMBOS * sizeof(eeprom_combo_t))

#define EEP_LEADER_BASE     (EEP_COMBO_BASE + 1 + EEP_COMBO_SIZE)
#define EEP_LEADER_SIZE     (MAX_LEADERS * sizeof(eeprom_leader_t))

// Total EEPROM usage = EEP_LEADER_BASE + 1 + EEP_LEADER_SIZE - EEP_FEATURES
// ≈ 380 bytes — well within the 2 KB user-data area past VIA's buffer.


// ═════════════════════════════════════════════════════════════════════════════
// HID PROTOCOL VALUE IDs  (via_custom_value_command_kb — shared with Python tool)
// ═════════════════════════════════════════════════════════════════════════════

#define VALUE_FLAGS        0x01
#define VALUE_TAP_COUNT    0x02
#define VALUE_TAP_ENTRY    0x03
#define VALUE_COMBO_COUNT  0x04
#define VALUE_COMBO_ENTRY  0x05
#define VALUE_LEADER_COUNT 0x06
#define VALUE_LEADER_ENTRY 0x07


// ═════════════════════════════════════════════════════════════════════════════
// DEFAULT TAP-OVERRIDE ENTRIES  (loaded at first boot or when EEPROM is blank)
// ═════════════════════════════════════════════════════════════════════════════
//
// Entries use eeprom_tap_t field initializers.  Add, remove, or reorder
// freely — the count is derived at compile time via sizeof.
// A {0} sentinel marks the end of the list; keep it as the last entry.
//
//  base_type=1 → base_id = POS_KC_xxx (follows physical key, survives layout change)
//  base_type=0 → base_id = KC_xxx keycode (follows the keycode label)

#define TAP_DEFAULTS \
    {.base_id=KC_BSPC, .tap_kc=KC_BSPC, .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=KC_DEL},      \
    {.base_id=KC_GESC, .tap_kc=QK_GESC, .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(QK_GESC)}, \
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
    {.base_id=KC_BSPC, .tap_kc=KC_BSPC, .dbl_type=TD_DBL_KEYCODE, .base_type=0, .dbl_val=RALT(KC_BSPC)}, \
    {0}  /* sentinel — all fields zero, keep last */

// Combo and leader defaults start empty (sentinel-only entries → zero count).
// {{0}} uses nested braces for the first-member array (keys[]/seq[]) to
// satisfy -Werror=missing-braces.  Add real entries above the sentinel
// to ship with default combos/leaders.
#define COMBO_DEFAULTS   {{0}}  // sentinel-only = empty list
#define LEADER_DEFAULTS  {{0}}  // sentinel-only = empty list


// ═════════════════════════════════════════════════════════════════════════════
// LAYERS
// ═════════════════════════════════════════════════════════════════════════════
//
// Adding a layer: insert it above _FN6, bump KEYMAP_LAYER_COUNT.
// Removing a layer: delete it, adjust KEYMAP_LAYER_COUNT.

#define KEYMAP_LAYER_COUNT  9

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

// Convenience aliases
#define FN1_MAC  MO(MAC_FN1)
#define FN1_WIN  MO(WIN_FN1)
#define FN2      MO(_FN2)

// If already on layer N, jump back to default; otherwise go to N.
#define LAYER_MOVE_OR_DEFAULT(N) do {                                      \
    if (get_highest_layer(layer_state) == (N))                             \
        layer_move(get_highest_layer(default_layer_state));                \
    else                                                                    \
        layer_move(N);                                                      \
} while(0)


// ═════════════════════════════════════════════════════════════════════════════
// CUSTOM KEYCODES  (must not collide with Keychron's NEW_SAFE_RANGE block)
// ═════════════════════════════════════════════════════════════════════════════

enum feature_keycodes {
    KC_AUTOSHIFT_TOGGLE = NEW_SAFE_RANGE,
    KC_TAP_DANCE_TOGGLE,
    KC_CAPS_WORD_TOGGLE,
    KC_REPEAT_KEY_TOGGLE,
    KC_DYN_MACRO_TOGGLE,
    KC_LEADER_TOGGLE,
    KC_NKRO_TOGGLE,
    KC_FEAT_OVERVIEW,
};

enum combo_events {
    CB_FEAT_OVERVIEW,
};


// ═════════════════════════════════════════════════════════════════════════════
// COMBO DEFINITIONS
// ═════════════════════════════════════════════════════════════════════════════

#define COMBO_FEAT_OVERVIEW_KEYS    {POS_KC_O, POS_KC_LBRC, COMBO_END}
#define COMBO_FEAT_OVERVIEW_ACTION   KC_FEAT_OVERVIEW


// ═════════════════════════════════════════════════════════════════════════════
// FEATURE OVERVIEW — MATRIX POSITIONS  (row, col → dispatch)
// ═════════════════════════════════════════════════════════════════════════════
//
// When overview is active, tapping these physical keys toggles the
// corresponding feature.  Keycodes below the number row switch layers
// via LAYER_MOVE_OR_DEFAULT.  Anything else exits overview.
//
// Positions are for the Q2 ISO-Encoder matrix layout:
//
//  Position   Key   Toggles
//  ────────   ───   ──────────────────────
//  (2, 1)     A     Auto-Shift
//  (2, 2)     S     Auto-Correct
//  (1, 5)     T     Tap Dance
//  (3, 4)     C     Caps Word
//  (1, 4)     R     Repeat Key
//  (2, 3)     D     Dynamic Macro
//  (2, 9)     L     Leader Key
//  (3, 7)     N     NKRO toggle
//  (0, 1-10)  1-0   Layer N / return to default


// ═════════════════════════════════════════════════════════════════════════════
// LED INDICATOR INDICES  (Q2 ISO-Encoder physical LED order)
// ═════════════════════════════════════════════════════════════════════════════
//
// These match g_snled27351_leds[] order in iso_encoder.c.  During normal
// operation only CAPS_LOCK lights.  The others illuminate in overview mode.
//
//  Index  Key   Purpose
//  ─────  ───   ──────────────────────────
//   28    Caps  Hardware Caps Lock state
//   29    A     Auto-Shift ON
//   30    S     Auto-Correct ON
//   19    T     Tap Dance ON
//   47    C     Caps Word processing ON
//   18    R     Repeat Key ON
//   31    D     Dynamic Macro ON
//   37    L     Leader Key ON
//   50    N     NKRO ON

#define IND_CAPS_LOCK   POS_KC_CAPS
#define IND_AUTO_SHIFT  POS_KC_A
#define IND_TAP_DANCE   POS_KC_T
#define IND_CAPS_WORD   POS_KC_C
#define IND_REPEAT_KEY  POS_KC_R
#define IND_DYN_MACRO   POS_KC_D
#define IND_LEADER      POS_KC_L
#define IND_AUTOCORRECT POS_KC_S
#define IND_NKRO        POS_KC_N

// Layer indicator: layer N → key N+1 (LED 1-9), layer 0 → key 0 (LED 10)
#define IND_LAYER_BASE  1


// ═════════════════════════════════════════════════════════════════════════════
// LEADER KEY — modifier auto-selects Cmd on Mac layers, Ctrl on Windows
// ═════════════════════════════════════════════════════════════════════════════
// The QK_LEAD key is placed on _FN2 at the Q position in the keymap.
// The leader timer (LEADER_TIMEOUT in quantum/leader.h) defaults to 300ms.

#define LEADER_MOD(on_mac, base_kc)  ((on_mac) ? LGUI(base_kc) : LCTL(base_kc))


// NOTE: VIA overrides (DYNAMIC_KEYMAP_LAYER_COUNT, DYNAMIC_KEYMAP_MACRO_COUNT)
// are in config.h — they must be visible before QMK_KEYBOARD_H is included,
// so they can't live in this file.  Keep config.h in sync with KEYMAP_LAYER_COUNT
// above.
