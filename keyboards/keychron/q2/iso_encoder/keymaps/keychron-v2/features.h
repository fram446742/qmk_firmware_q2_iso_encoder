/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// ═════════════════════════════════════════════════════════════════════════════
// Feature bit flags — stored as uint8_t in EEPROM at address 8100.
// ═════════════════════════════════════════════════════════════════════════════

#define FEATURE_TAP_DANCE   (1 << 0)  // Tap-dance keycode processing
#define FEATURE_AUTO_SHIFT  (1 << 1)  // Auto-shift on/off
#define FEATURE_CAPS_WORD   (1 << 2)  // Caps Word keycode processing
#define FEATURE_REPEAT_KEY  (1 << 3)  // Repeat / Alt-Repeat keycode processing
#define FEATURE_DYN_MACRO   (1 << 4)  // Dynamic Macro keycode processing
#define FEATURE_LEADER      (1 << 5)  // Leader key processing
// bit 6-7 reserved

// Runtime flags
extern uint8_t g_feature_flags;

// Feature flag API
void    features_init(void);               // Load from EEPROM, apply
void    features_save(void);               // Write flags to EEPROM
bool    feature_has(uint8_t flag);
void    feature_toggle(uint8_t flag);
void    feature_set(uint8_t flag, bool on);

// Convenience wrappers (used by keymap.c and indicators.c)
#define feature_tap_dance()      feature_has(FEATURE_TAP_DANCE)
#define feature_auto_shift()     feature_has(FEATURE_AUTO_SHIFT)
#define feature_caps_word()      feature_has(FEATURE_CAPS_WORD)
#define feature_repeat_key()     feature_has(FEATURE_REPEAT_KEY)
#define feature_dyn_macro()      feature_has(FEATURE_DYN_MACRO)
#define feature_leader()         feature_has(FEATURE_LEADER)

#define feature_toggle_tap_dance()      feature_toggle(FEATURE_TAP_DANCE)
#define feature_toggle_auto_shift()     feature_toggle(FEATURE_AUTO_SHIFT)
#define feature_toggle_caps_word()      feature_toggle(FEATURE_CAPS_WORD)
#define feature_toggle_repeat_key()     feature_toggle(FEATURE_REPEAT_KEY)
#define feature_toggle_dyn_macro()      feature_toggle(FEATURE_DYN_MACRO)
#define feature_toggle_leader()         feature_toggle(FEATURE_LEADER)

// ═════════════════════════════════════════════════════════════════════════════
// Tap-dance transparent override
// ═════════════════════════════════════════════════════════════════════════════
// Intercepts base keycodes and provides tap/double-tap behavior when
// FEATURE_TAP_DANCE is enabled.  No TD() keycodes in the keymap.

struct keyrecord_t;
typedef struct keyrecord_t keyrecord_t;

// Returns false if the keypress was consumed by tap overrides.
// Call from process_record_user when feature_tap_dance() is true.
bool features_tap_process(uint16_t keycode, keyrecord_t *record);

// Periodic timeout check — fires single-tap when pending + TAP_TERM expired.
// Call from matrix_scan_user.
void features_tap_task(void);

// ═════════════════════════════════════════════════════════════════════════════
// EEPROM-backed config (tap overrides, combos, leader sequences)
// ═════════════════════════════════════════════════════════════════════════════
//
// These constants are shared between C (features.c, keymap.c) and the Python
// tool (qmk_config_tool.py).  The Python tool reads this header at import time
// to stay in sync.

// HID protocol value IDs for via_custom_value_command_kb
// (hardware/firmware → tool communication)
#define VALUE_FLAGS        0x01  // get/set g_feature_flags
#define VALUE_TAP_COUNT    0x02  // get/set tap override count
#define VALUE_TAP_ENTRY    0x03  // get/set one tap override entry
#define VALUE_COMBO_COUNT  0x04  // get/set combo count
#define VALUE_COMBO_ENTRY  0x05  // get/set one combo entry
#define VALUE_LEADER_COUNT 0x06  // get/set leader count
#define VALUE_LEADER_ENTRY 0x07  // get/set one leader entry

// EEPROM addresses (logical byte offsets past VIA macro area)
#define EEP_FEATURES       8100  // feature flags (1 B)
#define EEP_TAP_BASE       8101  // tap: count(1B) + entries (200B = 20×10B)
#define EEP_COMBO_BASE     8302  // combos: count(1B) + entries (64B = 8×8B)
#define EEP_LEADER_BASE    8367  // leaders: count(1B) + entries (96B = 16×6B)

// Tap dance double-tap action types
typedef enum {
    TD_DBL_KEYCODE      = 0,  // single keycode (or modded, e.g. S(KC_2))
    TD_DBL_UNICODE_STR  = 1,  // Unicode string via send_unicode_string()
    TD_DBL_UNICODE_CP   = 2,  // Unicode codepoint via register_unicode()
    TD_DBL_SEND_STRING  = 3,  // ASCII string (≤4 chars) via send_string()
} td_dbl_type_t;

typedef struct __attribute__((packed)) {
    uint16_t base_kc;
    uint16_t tap_kc;
    uint8_t  dbl_type;
    uint8_t  pad;
    uint16_t dbl_val;
    uint16_t dbl_extra;
} eeprom_tap_t;
#define MAX_TAP_OVERRIDES 20

typedef struct __attribute__((packed)) {
    uint16_t keys[4];
    uint16_t output;
} eeprom_combo_t;
#define MAX_COMBOS 8

typedef struct __attribute__((packed)) {
    uint8_t  seq[3];
    uint8_t  mod;
    uint16_t key;
} eeprom_leader_t;
#define MAX_LEADERS 16

// Runtime arrays (loaded from EEPROM at boot, writable via HID)
extern eeprom_tap_t    eeprom_tap[MAX_TAP_OVERRIDES];
extern uint8_t         eeprom_tap_count;
extern eeprom_combo_t  eeprom_combos[MAX_COMBOS];
extern uint8_t         eeprom_combo_count;
extern eeprom_leader_t eeprom_leaders[MAX_LEADERS];
extern uint8_t         eeprom_leader_count;

void features_load_config(void);    // boot: EEPROM → RAM
void features_save_config(void);    // HID save: RAM → EEPROM
void features_load_defaults(void);  // first boot: defaults → RAM
