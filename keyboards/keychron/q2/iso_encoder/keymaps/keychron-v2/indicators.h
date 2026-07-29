/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// ── LED index → physical key map (Q2 ISO encoder) ──────────────────────────
// These map the per-key RGB LED index to the physical keycap.
// The indices correspond to the order in iso_encoder.c's g_snled27351_leds[].
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
//  50     N
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
// These LEDs show feature status during overview mode only.
// During normal operation only the Caps Lock LED (index 28) is active.

#define IND_AUTO_SHIFT         29   // A key — green when auto-shift on

// ── API ─────────────────────────────────────────────────────────────────────

// Start feature overview: saves current RGB mode, blacks out all LEDs,
// lights feature indicators white (active) or dim (inactive).
// Automatically cancels after OVERVIEW_TIMEOUT_MS or on next keypress.
void feature_overview_trigger(void);
bool feature_overview_is_active(void);
void feature_overview_cancel(void);

// Called from rgb_matrix_indicators_user() each frame.
void indicator_draw(void);

// Called from matrix_scan_user() for timeout handling.
void indicator_task(void);
