/* Copyright 2024 ~ 2025 @ Keychron (https://www.keychron.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * Declarations for Keychron RGB matrix extensions (region_set_color* and
 * the per-LED region table). These are referenced by Keychron's custom
 * RGB drivers (mixed_rgb, per_key_rgb, retail_demo) and exist outside
 * the upstream QMK rgb_matrix API.
 */
#pragma once

#include <stdint.h>

#ifdef RGB_MATRIX_ENABLE

/**
 * Per-LED region table. Indexed by LED index, holds the region (0-15)
 * that the LED belongs to. Bit-packed into the top 4 bits of the led
 * flag byte; this array is the unpacked view used by Keychron code.
 */
extern uint8_t rgb_regions[RGB_MATRIX_LED_COUNT];

/**
 * Set the color of a single LED only if it belongs to the given region.
 * Mirrors the API of rgb_matrix_set_color() with an extra region filter.
 */
void rgb_matrix_region_set_color(uint8_t region, int index, uint8_t red, uint8_t green, uint8_t blue);

/**
 * Set the color of all LEDs that belong to the given region.
 * Mirrors the API of rgb_matrix_set_color_all() with an extra region filter.
 */
void rgb_matrix_region_set_color_all(uint8_t region, uint8_t red, uint8_t green, uint8_t blue);

#endif // RGB_MATRIX_ENABLE
