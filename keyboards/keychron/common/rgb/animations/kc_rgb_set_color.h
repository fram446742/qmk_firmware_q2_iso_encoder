/* Copyright 2024 ~ 2025 @ Keychron (https://www.keychron.com)
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
 * Macros that always delegate to rgb_matrix_region_set_color / _all.
 *
 * Each effect receives params->region set by the caller (mixed_rgb or
 * standalone).  The region-aware functions filter by rgb_regions[i],
 * so a mix-RGB zone 0 paints only LED indices where rgb_regions[i]==0,
 * zone 1 paints only where rgb_regions[i]==1, etc.  There is no branch
 * on region == 0 — using the plain rgb_matrix_set_color would paint
 * *every* LED from zone 0, clobbering whatever higher zones painted.
 *
 * On the first boot (or after EEPROM reset), rgb_regions[] is seeded
 * from the keyboard's default_region[] array so that mix-RGB works
 * out of the box without any app-side configuration.
 */

#pragma once

#ifndef KC_RGB_SET_COLOR
#    define KC_RGB_SET_COLOR(params, i, r, g, b) \
        rgb_matrix_region_set_color((params)->region, (i), (r), (g), (b))
#endif

#ifndef KC_RGB_SET_COLOR_ALL
#    define KC_RGB_SET_COLOR_ALL(params, r, g, b) \
        rgb_matrix_region_set_color_all((params)->region, (r), (g), (b))
#endif
