/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// ═════════════════════════════════════════════════════════════════════════════
// LED index → physical key map (Q2 ISO encoder)
// ═════════════════════════════════════════════════════════════════════════════
// Indices follow the order in iso_encoder.c's g_snled27351_leds[].
// The encoder (Mute) at position k0O has no RGB LED → 67 LEDs for 68 keys.
//
//  Index  Key              Notes
//  ─────  ──────────────── ──────────────────────────────
//  0      Esc
//  1      1
//  ...    (standard row)
//  10     0
//  11     '
//  12     ¡
//  13     Backspace
//  14     Mute              Encoder key, may not have LED
//  15     Tab
//  16     Q
//  17     W
//  18     E
//  19     R
//  20     T
//  21     Y
//  22     U
//  23     O
//  24     P
//  25     [
//  26     ]
//  27     Delete
//  28     Caps Lock         (also CAPS_LOCK_INDEX in config.h)
//  29     A
//  30     S
//  31     D
//  32     F
//  33     G
//  34     H
//  35     J
//  36     K
//  37     L
//  38     ;
//  39     '
//  40     NuHS
//  41     Enter             (ANSI Enter / ISO big Enter)
//  42     Home
//  43     Left Shift
//  44     NuBS              ISO key left of Z
//  45     Z
//  46     X
//  47     C
//  48     V
//  49     B
//  50     N                ← NKRO indicator
//  51     M
//  52     ,
//  53     .
//  54     /
//  55     Right Shift
//  56     Up
//  57     Left Control
//  58     Left Option (Mac) / Win (Win)
//  59     Left Cmd (Mac) / Alt (Win)
//  60     Space
//  61     Right Cmd (Mac) / Alt (Win)
//  62     FN1 (MAC_FN1 / WIN_FN1)
//  63     FN2 (_FN2)
//  64     Left
//  65     Down
//  66     Right

// ── Feature indicator LED indices ───────────────────────────────────────────
// During normal operation these LEDs are OFF (only Caps Lock at index 28
// lights).  They illuminate only during feature overview (O+P combo)
// to show which features are active.

// ── Existing hardware indicator (always lit) ─────────────────────────────
#define IND_CAPS_LOCK   28   // Caps Lock — white when caps active

// ── Feature indicators (lit only during O+P overview) ─────────────────────
#define IND_AUTO_SHIFT  29   // A          Auto-Shift ON
#define IND_TAP_DANCE   19   // T          Tap Dance ON
#define IND_CAPS_WORD   47   // C          Caps Word processing ON
#define IND_REPEAT_KEY  18   // R          Repeat Key ON
#define IND_DYN_MACRO   31   // D          Dynamic Macro ON
#define IND_LEADER      37   // L          Leader Key ON
#define IND_AUTOCORRECT 30   // S          Auto-correct ON
#define IND_NKRO        50   // N          NKRO ON (from keymap_config)

// ── Layer indicator (lit during O+P overview) ─────────────────────────────
// The currently active layer lights the matching number key.
// Layer 0 → key 0 (LED 10), Layer 1 → key 1 (LED 1), … Layer 8 → key 8 (LED 8)
#define IND_LAYER_BASE  1    // offset: layer N → LED 1+N, layer 0 → LED 10

// ── API ─────────────────────────────────────────────────────────────────────
// Trigger the feature overview — blacks out LEDs, lights indicators white
// for active features, dim gray for inactive. Auto-cancels after 2 seconds
// or on any keypress.

void feature_overview_trigger(void);
bool feature_overview_is_active(void);
void feature_overview_cancel(void);
void feature_overview_reset_timer(void);  // extend by OVERVIEW_TIMEOUT_MS from now

// Called from rgb_matrix_indicators_user() each frame.
void indicator_draw(void);

// Called from matrix_scan_user() for timeout handling.
void indicator_task(void);
