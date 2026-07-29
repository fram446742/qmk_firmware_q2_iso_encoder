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
 *
 * Keychron RGB matrix extension API.
 *
 * These functions and globals extend upstream QMK's rgb_matrix with
 * region-aware per-LED colour control, used by mixed_rgb.c and the
 * region-aware Keychron animation effects.
 */

#pragma once

#include <stdint.h>
#include "rgb_matrix.h"

extern uint8_t rgb_regions[RGB_MATRIX_LED_COUNT];

void rgb_matrix_region_set_color(uint8_t region, int index, uint8_t red, uint8_t green, uint8_t blue);
void rgb_matrix_region_set_color_all(uint8_t region, uint8_t red, uint8_t green, uint8_t blue);
