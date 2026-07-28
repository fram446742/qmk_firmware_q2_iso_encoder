/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Minimal stub for Vial keycode resolution — avoids static_assert failures */

#pragma once

#include "quantum_keycodes.h"

static inline uint16_t vial_resolve_keycode(uint16_t keycode) {
    return keycode;
}

static inline uint16_t vial_resolve_keycode_bypass(uint16_t keycode) {
    return keycode;
}