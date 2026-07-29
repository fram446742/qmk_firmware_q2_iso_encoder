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

#include "keychron.h"
#include "keychron_rgb_type.h"

extern os_indicator_config_t os_ind_cfg;

#ifdef DIP_SWITCH_ENABLE
bool dip_switch_update_kb(uint8_t index, bool active) {
    if (!dip_switch_update_user(index, active)) {
         return false;
        }
    if (index == 0) {
        default_layer_set(1UL << (active ? 1 : 0));
    }
    return true;
}
#endif

#ifdef KEYCHRON_ENABLE
void keyboard_post_init_kb(void) {
    keychron_common_init();
    keyboard_post_init_user();
}
#else
void keyboard_post_init_kb(void) {
    keyboard_post_init_user();
}
#endif

#if defined(RGB_MATRIX_ENABLE) && defined(CAPS_LOCK_INDEX)

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {
    if (!rgb_matrix_indicators_advanced_user(led_min, led_max)) { return false; }

    /* Honor the Launcher's caps-lock LED enable/disable toggle.
     * The strong os_state_indicate() in keychron_rgb.c (called when no RGB
     * effect is active) already gates on os_ind_cfg.disable.caps_lock, but
     * this code path runs whenever an effect IS active, so the toggle was
     * silently ignored while any effect was rendering. */
    if (!os_ind_cfg.disable.caps_lock) {
        if (host_keyboard_led_state().caps_lock) {
            RGB_MATRIX_INDICATOR_SET_COLOR(CAPS_LOCK_INDEX, 255, 255, 255);
        } else {
            if (!rgb_matrix_get_flags()) {
                RGB_MATRIX_INDICATOR_SET_COLOR(CAPS_LOCK_INDEX, 0, 0, 0);
            }
        }
    }
    return true;
}

#endif // CAPS_LOCK_INDEX
