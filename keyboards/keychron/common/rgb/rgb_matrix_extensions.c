/* Copyright 2024 ~ 2025 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Keychron RGB matrix extensions: per-LED region table and the
 * region-filtered color setters. Compiled in via keychron_common.mk
 * when KEYCHRON_RGB_ENABLE = yes.
 */
#include "rgb_matrix.h"
#include "rgb_matrix_extensions.h"

#ifdef RGB_MATRIX_ENABLE

uint8_t rgb_regions[RGB_MATRIX_LED_COUNT];

void rgb_matrix_region_set_color(uint8_t region, int index, uint8_t red, uint8_t green, uint8_t blue) {
    if ((g_led_config.flags[index] & 0xF0) >> 4 == region) {
        rgb_matrix_driver.set_color(index, red, green, blue);
    }
}

void rgb_matrix_region_set_color_all(uint8_t region, uint8_t red, uint8_t green, uint8_t blue) {
    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++)
        if (((g_led_config.flags[i] & 0xF0) >> 4) == region)
            rgb_matrix_set_color(i, red, green, blue);
}

#endif // RGB_MATRIX_ENABLE
