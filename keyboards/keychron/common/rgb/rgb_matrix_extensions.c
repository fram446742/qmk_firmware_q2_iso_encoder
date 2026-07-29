/* Copyright 2025 @ Keychron (https://www.keychron.com)
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

#include "rgb_matrix_extensions.h"
#include "rgb_matrix_drivers.h"

// Per-LED region assignment for mix-RGB zone separation.
// BSS-initialised to zero so that standard (non-mix-RGB) effects
// paint every LED when going through rgb_matrix_region_set_color(0, …).
uint8_t rgb_regions[RGB_MATRIX_LED_COUNT];

void rgb_matrix_region_set_color(uint8_t region, int index, uint8_t red, uint8_t green, uint8_t blue) {
    if (rgb_regions[index] == region) {
        rgb_matrix_driver.set_color(index, red, green, blue);
    }
}

void rgb_matrix_region_set_color_all(uint8_t region, uint8_t red, uint8_t green, uint8_t blue) {
    for (uint8_t i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
        if (rgb_regions[i] == region) {
            rgb_matrix_set_color(i, red, green, blue);
        }
    }
}
