/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// ── Feature bit flags ──────────────────────────────────────────────────────
// Stored as a single byte in EEPROM. Bitwise ops for explicit control.
// Add new features here as (1 << N), N = next free bit.

#define FEATURE_TAP_DANCE   (1 << 0)  // Tap-dance (TD) keycode processing
#define FEATURE_AUTO_SHIFT  (1 << 1)  // Auto-shift on/off

// ── Current feature state (loaded from EEPROM at init) ─────────────────────
extern uint8_t g_feature_flags;

// ── API ─────────────────────────────────────────────────────────────────────
void    features_init(void);               // Load from EEPROM, apply to HW
void    features_save(void);               // Write to EEPROM

bool    feature_has(uint8_t flag);          // Test if feature is enabled
void    feature_toggle(uint8_t flag);       // Toggle and save
void    feature_set(uint8_t flag, bool on); // Set to specific state, save

// Convenience wrappers — always delegate to feature_toggle/set so the
// hardware apply (feature_apply_flag) and EEPROM save happen together.
static inline bool feature_tap_dance(void)          { return feature_has(FEATURE_TAP_DANCE); }
static inline void feature_toggle_tap_dance(void)   { feature_toggle(FEATURE_TAP_DANCE); }
static inline bool feature_auto_shift(void)         { return feature_has(FEATURE_AUTO_SHIFT); }
static inline void feature_toggle_auto_shift(void)  { feature_toggle(FEATURE_AUTO_SHIFT); }
