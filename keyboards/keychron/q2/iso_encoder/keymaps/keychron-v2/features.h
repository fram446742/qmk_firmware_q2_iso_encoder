/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// ═════════════════════════════════════════════════════════════════════════════
// Feature flags — runtime API
// ═════════════════════════════════════════════════════════════════════════════
// Bit flag constants are in keymap_config.h.  This header exposes only the
// runtime API (extern globals, function prototypes, convenience wrappers).

extern uint8_t g_feature_flags;

void    features_init(void);
void    features_save(void);
void    feature_apply_all(void);
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
#define feature_toggle_layer_vis()      feature_toggle(FEATURE_LAYER_VIS)

#define feature_layer_vis()             feature_has(FEATURE_LAYER_VIS)

// ═════════════════════════════════════════════════════════════════════════════
// Tap-dance transparent override  (timer-based, no TAP_DANCE_ENABLE)
// ═════════════════════════════════════════════════════════════════════════════

struct keyrecord_t;
typedef struct keyrecord_t keyrecord_t;

bool features_tap_process(uint16_t keycode, keyrecord_t *record);
void features_tap_task(void);

// ── Position combos (custom, features.c) — independent of QMK-native combos ──
bool features_combo_process(uint16_t keycode, keyrecord_t *record);
void features_combo_task(void);

// ═════════════════════════════════════════════════════════════════════════════
// EEPROM-backed config arrays  (extern declarations)
// ═════════════════════════════════════════════════════════════════════════════
// Struct types, EEPROM addresses, and HID protocol constants live in
// keymap_config.h.  The Python tool (qmk_config_tool.py) parses
// keymap_config.h at import time to stay in sync with the firmware.

#include "keymap_config.h"

extern eeprom_tap_t    eeprom_tap[MAX_TAP_OVERRIDES];
extern uint8_t         eeprom_tap_count;
extern eeprom_combo_t  eeprom_combos[MAX_COMBOS];
extern uint8_t         eeprom_combo_count;
extern eeprom_leader_t eeprom_leaders[MAX_LEADERS];
extern uint8_t         eeprom_leader_count;

void features_load_config(void);
void features_save_config(void);
void features_load_defaults(void);
