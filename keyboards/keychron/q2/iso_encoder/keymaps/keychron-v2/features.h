/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// ═════════════════════════════════════════════════════════════════════════════
// Feature bit flags — stored as uint8_t in EEPROM at address 8100.
// Bitwise operations only: AND for test, XOR for toggle, OR/AND-NOT for set.
// ═════════════════════════════════════════════════════════════════════════════

#define FEATURE_TAP_DANCE   (1 << 0)  // Tap-dance (TD) keycode processing
#define FEATURE_AUTO_SHIFT  (1 << 1)  // Auto-shift on/off
#define FEATURE_CAPS_WORD   (1 << 2)  // Caps Word keycode processing
#define FEATURE_REPEAT_KEY  (1 << 3)  // Repeat / Alt-Repeat keycode processing
#define FEATURE_DYN_MACRO   (1 << 4)  // Dynamic Macro keycode processing
#define FEATURE_LEADER      (1 << 5)  // Leader key processing
// bit 6-7 reserved

// ── Current runtime state (from EEPROM) ────────────────────────────────────
extern uint8_t g_feature_flags;

// ── API ─────────────────────────────────────────────────────────────────────
void    features_init(void);               // Load from EEPROM, apply to HW
void    features_save(void);               // Write to EEPROM

// Test, toggle, set — all explicit bitwise ops on g_feature_flags
bool    feature_has(uint8_t flag);
void    feature_toggle(uint8_t flag);
void    feature_set(uint8_t flag, bool on);

// ── Convenience wrappers ────────────────────────────────────────────────────
static inline bool    feature_tap_dance(void)           { return feature_has(FEATURE_TAP_DANCE); }
static inline void    feature_toggle_tap_dance(void)    { feature_toggle(FEATURE_TAP_DANCE); }
static inline bool    feature_auto_shift(void)          { return feature_has(FEATURE_AUTO_SHIFT); }
static inline void    feature_toggle_auto_shift(void)   { feature_toggle(FEATURE_AUTO_SHIFT); }
static inline bool    feature_caps_word(void)           { return feature_has(FEATURE_CAPS_WORD); }
static inline void    feature_toggle_caps_word(void)    { feature_toggle(FEATURE_CAPS_WORD); }
static inline bool    feature_repeat_key(void)          { return feature_has(FEATURE_REPEAT_KEY); }
static inline void    feature_toggle_repeat_key(void)   { feature_toggle(FEATURE_REPEAT_KEY); }
static inline bool    feature_dyn_macro(void)           { return feature_has(FEATURE_DYN_MACRO); }
static inline void    feature_toggle_dyn_macro(void)    { feature_toggle(FEATURE_DYN_MACRO); }
static inline bool    feature_leader(void)              { return feature_has(FEATURE_LEADER); }
static inline void    feature_toggle_leader(void)       { feature_toggle(FEATURE_LEADER); }

// ═════════════════════════════════════════════════════════════════════════════
// EEPROM-backed config (tap overrides, combos, leader sequences)
// ═════════════════════════════════════════════════════════════════════════════
// Layout: 8100 flags(1B), 8101 count(1B), 8102+ entries, see features.c

typedef enum {
    TD_DBL_KEYCODE     = 0,
    TD_DBL_UNICODE_STR = 1,
    TD_DBL_UNICODE_CP  = 2,
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
