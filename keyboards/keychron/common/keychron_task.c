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

#include <stdlib.h>
#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "keychron_task.h"
#include "keychron_raw_hid.h"
#include "backlit_indicator.h"
#ifdef FACTORY_TEST_ENABLE
#    include "factory_test.h"
#endif
#ifdef RETAIL_DEMO_ENABLE
#    include "retail_demo.h"
#endif
#ifdef ANANLOG_MATRIX
#    include "profile.h"
#endif

__attribute__((weak)) bool process_record_keychron_kb(uint16_t keycode, keyrecord_t *record) {
    return true;
}

bool process_record_keychron(uint16_t keycode, keyrecord_t *record) {
    return process_record_keychron_kb(keycode, record) && process_record_keychron_common(keycode, record);
}

#if defined(LED_MATRIX_ENABLE)
__attribute__((weak)) bool led_matrix_indicators_keychron(void) {
    return false;
}
#endif

#if defined(RGB_MATRIX_ENABLE)
__attribute__((weak)) bool rgb_matrix_indicators_keychron(void) {
    return false;
}
#endif

__attribute__((weak)) void keychron_task_kb(void) {}

void keychron_task(void) {
    keychron_task_kb();
    keychron_common_task();
}

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    if (!process_record_keychron(keycode, record)) {
        return false;
    }
    return process_record_user(keycode, record);
}

#ifdef RGB_MATRIX_ENABLE
bool rgb_matrix_indicators_kb(void) {
    if (!rgb_matrix_indicators_keychron()) {
        return false;
    }
    return rgb_matrix_indicators_user();
}
#endif

#ifdef LED_MATRIX_ENABLE
bool led_matrix_indicators_kb(void) {
    if (!led_matrix_indicators_keychron()) {
        return false;
    }
    return led_matrix_indicators_user();
}
#endif

void housekeeping_task_kb(void) {
    keychron_task();
}