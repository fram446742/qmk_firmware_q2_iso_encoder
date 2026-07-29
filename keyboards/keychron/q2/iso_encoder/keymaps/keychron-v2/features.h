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
