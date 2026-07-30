/* Auto-generated from keyboard.json + keymap.c MAC_BASE layer
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>

// ── Packed matrix position: (row << 8) | col ──────────────────────
#define PACK_MTX(r, c)  (((r) << 8) | (c))

// ── Per-key macros ────────────────────────────────────────────────
//  POS_xxx    = packed matrix position (used for tap-override base_id)
//  POS_IDX_xxx = LED index in g_snled27351_leds[] (used for IND_* macros)

#define POS_QK_GESC     PACK_MTX(0, 0)  // QK_GESC
#define POS_IDX_QK_GESC 0                    // QK_GESC

#define POS_KC_1     PACK_MTX(0, 1)  // KC_1
#define POS_IDX_KC_1 1                    // KC_1

#define POS_KC_2     PACK_MTX(0, 2)  // KC_2
#define POS_IDX_KC_2 2                    // KC_2

#define POS_KC_3     PACK_MTX(0, 3)  // KC_3
#define POS_IDX_KC_3 3                    // KC_3

#define POS_KC_4     PACK_MTX(0, 4)  // KC_4
#define POS_IDX_KC_4 4                    // KC_4

#define POS_KC_5     PACK_MTX(0, 5)  // KC_5
#define POS_IDX_KC_5 5                    // KC_5

#define POS_KC_6     PACK_MTX(0, 6)  // KC_6
#define POS_IDX_KC_6 6                    // KC_6

#define POS_KC_7     PACK_MTX(0, 7)  // KC_7
#define POS_IDX_KC_7 7                    // KC_7

#define POS_KC_8     PACK_MTX(0, 8)  // KC_8
#define POS_IDX_KC_8 8                    // KC_8

#define POS_KC_9     PACK_MTX(0, 9)  // KC_9
#define POS_IDX_KC_9 9                    // KC_9

#define POS_KC_0     PACK_MTX(0, 10)  // KC_0
#define POS_IDX_KC_0 10                    // KC_0

#define POS_KC_MINS     PACK_MTX(0, 11)  // KC_MINS
#define POS_IDX_KC_MINS 11                    // KC_MINS

#define POS_KC_EQL     PACK_MTX(0, 12)  // KC_EQL
#define POS_IDX_KC_EQL 12                    // KC_EQL

#define POS_KC_BSPC     PACK_MTX(0, 13)  // KC_BSPC
#define POS_IDX_KC_BSPC 13                    // KC_BSPC

#define POS_KC_MUTE     PACK_MTX(0, 14)  // KC_MUTE
#define POS_IDX_KC_MUTE 14                    // KC_MUTE

#define POS_KC_TAB     PACK_MTX(1, 0)  // KC_TAB
#define POS_IDX_KC_TAB 15                    // KC_TAB

#define POS_KC_Q     PACK_MTX(1, 1)  // KC_Q
#define POS_IDX_KC_Q 16                    // KC_Q

#define POS_KC_W     PACK_MTX(1, 2)  // KC_W
#define POS_IDX_KC_W 17                    // KC_W

#define POS_KC_E     PACK_MTX(1, 3)  // KC_E
#define POS_IDX_KC_E 18                    // KC_E

#define POS_KC_R     PACK_MTX(1, 4)  // KC_R
#define POS_IDX_KC_R 19                    // KC_R

#define POS_KC_T     PACK_MTX(1, 5)  // KC_T
#define POS_IDX_KC_T 20                    // KC_T

#define POS_KC_Y     PACK_MTX(1, 6)  // KC_Y
#define POS_IDX_KC_Y 21                    // KC_Y

#define POS_KC_U     PACK_MTX(1, 7)  // KC_U
#define POS_IDX_KC_U 22                    // KC_U

#define POS_KC_I     PACK_MTX(1, 8)  // KC_I
#define POS_IDX_KC_I 23                    // KC_I

#define POS_KC_O     PACK_MTX(1, 9)  // KC_O
#define POS_IDX_KC_O 24                    // KC_O

#define POS_KC_P     PACK_MTX(1, 10)  // KC_P
#define POS_IDX_KC_P 25                    // KC_P

#define POS_KC_LBRC     PACK_MTX(1, 11)  // KC_LBRC
#define POS_IDX_KC_LBRC 26                    // KC_LBRC

#define POS_KC_RBRC     PACK_MTX(1, 12)  // KC_RBRC
#define POS_IDX_KC_RBRC 27                    // KC_RBRC

#define POS_KC_DEL     PACK_MTX(1, 14)  // KC_DEL
#define POS_IDX_KC_DEL 28                    // KC_DEL

#define POS_KC_CAPS     PACK_MTX(2, 0)  // KC_CAPS
#define POS_IDX_KC_CAPS 29                    // KC_CAPS

#define POS_KC_A     PACK_MTX(2, 1)  // KC_A
#define POS_IDX_KC_A 30                    // KC_A

#define POS_KC_S     PACK_MTX(2, 2)  // KC_S
#define POS_IDX_KC_S 31                    // KC_S

#define POS_KC_D     PACK_MTX(2, 3)  // KC_D
#define POS_IDX_KC_D 32                    // KC_D

#define POS_KC_F     PACK_MTX(2, 4)  // KC_F
#define POS_IDX_KC_F 33                    // KC_F

#define POS_KC_G     PACK_MTX(2, 5)  // KC_G
#define POS_IDX_KC_G 34                    // KC_G

#define POS_KC_H     PACK_MTX(2, 6)  // KC_H
#define POS_IDX_KC_H 35                    // KC_H

#define POS_KC_J     PACK_MTX(2, 7)  // KC_J
#define POS_IDX_KC_J 36                    // KC_J

#define POS_KC_K     PACK_MTX(2, 8)  // KC_K
#define POS_IDX_KC_K 37                    // KC_K

#define POS_KC_L     PACK_MTX(2, 9)  // KC_L
#define POS_IDX_KC_L 38                    // KC_L

#define POS_KC_SCLN     PACK_MTX(2, 10)  // KC_SCLN
#define POS_IDX_KC_SCLN 39                    // KC_SCLN

#define POS_KC_QUOT     PACK_MTX(2, 11)  // KC_QUOT
#define POS_IDX_KC_QUOT 40                    // KC_QUOT

#define POS_KC_NUHS     PACK_MTX(2, 13)  // KC_NUHS
#define POS_IDX_KC_NUHS 41                    // KC_NUHS

#define POS_KC_ENT     PACK_MTX(1, 13)  // KC_ENT
#define POS_IDX_KC_ENT 42                    // KC_ENT

#define POS_KC_HOME     PACK_MTX(2, 14)  // KC_HOME
#define POS_IDX_KC_HOME 43                    // KC_HOME

#define POS_KC_LSFT     PACK_MTX(3, 0)  // KC_LSFT
#define POS_IDX_KC_LSFT 44                    // KC_LSFT

#define POS_KC_NUBS     PACK_MTX(3, 1)  // KC_NUBS
#define POS_IDX_KC_NUBS 45                    // KC_NUBS

#define POS_KC_Z     PACK_MTX(3, 2)  // KC_Z
#define POS_IDX_KC_Z 46                    // KC_Z

#define POS_KC_X     PACK_MTX(3, 3)  // KC_X
#define POS_IDX_KC_X 47                    // KC_X

#define POS_KC_C     PACK_MTX(3, 4)  // KC_C
#define POS_IDX_KC_C 48                    // KC_C

#define POS_KC_V     PACK_MTX(3, 5)  // KC_V
#define POS_IDX_KC_V 49                    // KC_V

#define POS_KC_B     PACK_MTX(3, 6)  // KC_B
#define POS_IDX_KC_B 50                    // KC_B

#define POS_KC_N     PACK_MTX(3, 7)  // KC_N
#define POS_IDX_KC_N 51                    // KC_N

#define POS_KC_M     PACK_MTX(3, 8)  // KC_M
#define POS_IDX_KC_M 52                    // KC_M

#define POS_KC_COMM     PACK_MTX(3, 9)  // KC_COMM
#define POS_IDX_KC_COMM 53                    // KC_COMM

#define POS_KC_DOT     PACK_MTX(3, 10)  // KC_DOT
#define POS_IDX_KC_DOT 54                    // KC_DOT

#define POS_KC_SLSH     PACK_MTX(3, 11)  // KC_SLSH
#define POS_IDX_KC_SLSH 55                    // KC_SLSH

#define POS_KC_RSFT     PACK_MTX(3, 13)  // KC_RSFT
#define POS_IDX_KC_RSFT 56                    // KC_RSFT

#define POS_KC_UP     PACK_MTX(3, 14)  // KC_UP
#define POS_IDX_KC_UP 57                    // KC_UP

#define POS_KC_LCTL     PACK_MTX(4, 0)  // KC_LCTL
#define POS_IDX_KC_LCTL 58                    // KC_LCTL

#define POS_KC_LOPTN     PACK_MTX(4, 1)  // KC_LOPTN
#define POS_IDX_KC_LOPTN 59                    // KC_LOPTN

#define POS_KC_LCMMD     PACK_MTX(4, 2)  // KC_LCMMD
#define POS_IDX_KC_LCMMD 60                    // KC_LCMMD

#define POS_KC_SPC     PACK_MTX(4, 6)  // KC_SPC
#define POS_IDX_KC_SPC 61                    // KC_SPC

#define POS_KC_RCMMD     PACK_MTX(4, 10)  // KC_RCMMD
#define POS_IDX_KC_RCMMD 62                    // KC_RCMMD

#define POS_KC_LEFT     PACK_MTX(4, 13)  // KC_LEFT
#define POS_IDX_KC_LEFT 65                    // KC_LEFT

#define POS_KC_DOWN     PACK_MTX(2, 12)  // KC_DOWN
#define POS_IDX_KC_DOWN 66                    // KC_DOWN

#define POS_KC_RGHT     PACK_MTX(4, 14)  // KC_RGHT
#define POS_IDX_KC_RGHT 67                    // KC_RGHT

// Total: 66 keys

// ── LED-index → matrix-position lookup ──────────────────────────
// Keys without an RGB LED (e.g. rotary encoder) are excluded.
static const uint16_t PROGMEM led_to_mtx[67] = {
    PACK_MTX(0, 0),
    PACK_MTX(0, 1),
    PACK_MTX(0, 2),
    PACK_MTX(0, 3),
    PACK_MTX(0, 4),
    PACK_MTX(0, 5),
    PACK_MTX(0, 6),
    PACK_MTX(0, 7),
    PACK_MTX(0, 8),
    PACK_MTX(0, 9),
    PACK_MTX(0, 10),
    PACK_MTX(0, 11),
    PACK_MTX(0, 12),
    PACK_MTX(0, 13),
    PACK_MTX(1, 0),
    PACK_MTX(1, 1),
    PACK_MTX(1, 2),
    PACK_MTX(1, 3),
    PACK_MTX(1, 4),
    PACK_MTX(1, 5),
    PACK_MTX(1, 6),
    PACK_MTX(1, 7),
    PACK_MTX(1, 8),
    PACK_MTX(1, 9),
    PACK_MTX(1, 10),
    PACK_MTX(1, 11),
    PACK_MTX(1, 12),
    PACK_MTX(1, 14),
    PACK_MTX(2, 0),
    PACK_MTX(2, 1),
    PACK_MTX(2, 2),
    PACK_MTX(2, 3),
    PACK_MTX(2, 4),
    PACK_MTX(2, 5),
    PACK_MTX(2, 6),
    PACK_MTX(2, 7),
    PACK_MTX(2, 8),
    PACK_MTX(2, 9),
    PACK_MTX(2, 10),
    PACK_MTX(2, 11),
    PACK_MTX(2, 13),
    PACK_MTX(1, 13),
    PACK_MTX(2, 14),
    PACK_MTX(3, 0),
    PACK_MTX(3, 1),
    PACK_MTX(3, 2),
    PACK_MTX(3, 3),
    PACK_MTX(3, 4),
    PACK_MTX(3, 5),
    PACK_MTX(3, 6),
    PACK_MTX(3, 7),
    PACK_MTX(3, 8),
    PACK_MTX(3, 9),
    PACK_MTX(3, 10),
    PACK_MTX(3, 11),
    PACK_MTX(3, 13),
    PACK_MTX(3, 14),
    PACK_MTX(4, 0),
    PACK_MTX(4, 1),
    PACK_MTX(4, 2),
    PACK_MTX(4, 6),
    PACK_MTX(4, 10),
    PACK_MTX(4, 11),
    PACK_MTX(4, 12),
    PACK_MTX(4, 13),
    PACK_MTX(2, 12),
    PACK_MTX(4, 14),
};

