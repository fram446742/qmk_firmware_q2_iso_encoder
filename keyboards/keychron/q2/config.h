/* Copyright 2023 ~ 2025 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

/* Encoder Configuration */
#define ENCODER_DEFAULT_POS 0x3

#ifdef KEYCHRON_ENABLE
#    include "eeconfig_kb.h"
#endif

#ifdef RGB_MATRIX_ENABLE
/* RGB Matrix Driver Configuration */
#define SNLED27351_I2C_ADDRESS_1 SNLED27351_I2C_ADDRESS_VDDIO
#define SNLED27351_I2C_ADDRESS_2 SNLED27351_I2C_ADDRESS_GND

#ifdef KEYCHRON_ENABLE
/* Increase I2C speed to 1000 KHz */
#define I2C1_TIMINGR_PRESC 0U
#define I2C1_TIMINGR_SCLDEL 3U
#define I2C1_TIMINGR_SDADEL 0U
#define I2C1_TIMINGR_SCLH 15U
#define I2C1_TIMINGR_SCLL 51U

/* Set LED Current */
#define SNLED27351_PHASE_CHANNEL SNLED27351_SCAN_PHASE_9_CHANNEL
#define SNLED27351_CURRENT_TUNE  \
    { 0xC0, 0xC0, 0x5D, 0xC0, 0xC0, 0x5D, 0xC0, 0xC0, 0x5D, 0xC0, 0xC0, 0x5D }
#endif
#endif

#ifdef KEYCHRON_ENABLE
#    define CUSTOM_KEYCODES_ENABLE
#    define FACTORY_TEST_ENABLE
#    define APDAPTIVE_NKRO_ENABLE
#    define STATE_NOTIFY_ENABLE
#    define VIA_INSECURE

#    define MATRIX_UNSELECT_DRIVE_HIGH

/* Factory test keys */
#    define FN_KEY_1 MO(2)
#    define FN_KEY_2 MO(3)
#    define FN_KEY_3 MO(4)
#endif