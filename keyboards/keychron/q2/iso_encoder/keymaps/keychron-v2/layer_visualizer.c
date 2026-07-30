/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "layer_visualizer.h"
#include "keymap_config.h"
#include "features.h"
#include "indicators.h"
#include "keycodes.h"
#include "keymap_introspection.h"


// ═════════════════════════════════════════════════════════════════════════════
// Key categories  (mirrors Keychron Launcher classification)
// ═════════════════════════════════════════════════════════════════════════════

typedef enum {
    CAT_BYPASS,      // KC_TRNS
    CAT_BLANK,       // KC_NO
    CAT_MODIFIER,    // LCTL, LSFT, LALT, LGUI, RCTL, RSFT, RALT, RGUI (0xE0-0xE7)
    CAT_MAC_EXTRA,   // KC_LOPTN, KC_LCMMD, KC_ROPTN, KC_RCMMD (Mac-specific QK_KB)
    CAT_FUNCTION,    // F1-F24
    CAT_BASIC,       // standard HID keycodes (alpha, numbers, punct, nav, etc.)
    CAT_MEDIA,       // system / consumer keys
    CAT_MACRO,       // QK_MACRO / tap dance
    CAT_SPECIAL,     // QK_KB keys (KC_TASK, KC_FILE, KC_SIRI, KC_SNAP, etc.)
    CAT_LIGHT,       // underglow, RGB matrix, backlight
    CAT_CUSTOM,      // QK_USER range
    CAT_LAYER,       // MO, TO, TG, DF, LT, MT, etc.
} key_category_t;

// ── Mac-specific QK_KB range ───────────────────────────────────────────────
// KC_LOPTN (QK_KB_2 = 0x7E02) through KC_MAC_SIRI (QK_KB_6 = 0x7E06).
#define IS_MAC_EXTRA(kc)  ((kc) >= KC_LOPTN && (kc) <= KC_MAC_SIRI)

// ── Function keys ──────────────────────────────────────────────────────────
#define IS_F_KEY(kc)      (((kc) >= KC_F1 && (kc) <= KC_F12) || ((kc) >= KC_F13 && (kc) <= KC_F24))


static key_category_t categorize(uint16_t kc) {
    if (kc == KC_TRNS)              return CAT_BYPASS;
    if (kc == KC_NO)                return CAT_BLANK;

    // Layer management — check before everything else
    if (IS_QK_TO(kc)             || IS_QK_MOMENTARY(kc)   ||
        IS_QK_DEF_LAYER(kc)      || IS_QK_TOGGLE_LAYER(kc) ||
        IS_QK_ONE_SHOT_LAYER(kc) || IS_QK_LAYER_TAP_TOGGLE(kc) ||
        IS_QK_PERSISTENT_DEF_LAYER(kc) ||
        IS_QK_LAYER_TAP(kc)      || IS_QK_LAYER_MOD(kc)   ||
        IS_QK_MOD_TAP(kc)        || IS_QK_SWAP_HANDS(kc))
        return CAT_LAYER;

    // Standard modifiers (Ctrl, Shift, Alt, GUI)
    if (IS_MODIFIER_KEYCODE(kc))    return CAT_MODIFIER;

    // Mac-specific keyboard keys (Option, Cmd — in QK_KB range)
    if (IS_MAC_EXTRA(kc))           return CAT_MAC_EXTRA;

    // Macros
    if (IS_MACRO_KEYCODE(kc) || IS_QK_TAP_DANCE(kc))
        return CAT_MACRO;

    // Media (system + consumer)
    if (IS_SYSTEM_KEYCODE(kc) || IS_CONSUMER_KEYCODE(kc))
        return CAT_MEDIA;

    // Lighting
    if (IS_UNDERGLOW_KEYCODE(kc) || IS_RGB_MATRIX_KEYCODE(kc) ||
        IS_BACKLIGHT_KEYCODE(kc) || IS_LED_MATRIX_KEYCODE(kc))
        return CAT_LIGHT;

    // QK_KB special keys (KC_TASK, KC_FILE, KC_SIRI, KC_SNAP, KC_CTANA)
    // Must come AFTER IS_MAC_EXTRA since those are also in QK_KB.
    if (IS_KB_KEYCODE(kc))          return CAT_SPECIAL;

    // User-defined range
    if (IS_USER_KEYCODE(kc))        return CAT_CUSTOM;

    // Function keys (in basic HID range, but we give them their own color)
    if (IS_F_KEY(kc))               return CAT_FUNCTION;

    // Basic HID — everything else
    return CAT_BASIC;
}


// ═════════════════════════════════════════════════════════════════════════════
// Color lookup table (PROGMEM for flash savings)
// ═════════════════════════════════════════════════════════════════════════════

static const uint8_t PROGMEM cat_colors[12][3] = {
    [CAT_BYPASS]    = LV_COLOR_BYPASS,
    [CAT_BLANK]     = LV_COLOR_BLANK,
    [CAT_MODIFIER]  = LV_COLOR_MODIFIER,
    [CAT_MAC_EXTRA] = LV_COLOR_MAC_EXTRA,
    [CAT_FUNCTION]  = LV_COLOR_FUNCTION,
    [CAT_BASIC]     = LV_COLOR_BASIC,
    [CAT_MEDIA]     = LV_COLOR_MEDIA,
    [CAT_MACRO]     = LV_COLOR_MACRO,
    [CAT_SPECIAL]   = LV_COLOR_SPECIAL,
    [CAT_LIGHT]     = LV_COLOR_LIGHT,
    [CAT_CUSTOM]    = LV_COLOR_CUSTOM,
    [CAT_LAYER]     = LV_COLOR_LAYER,
};

// ── Per-LED color cache ─────────────────────────────────────────────────
// Computed once per vis transition (layer change or MO hold/release),
// then applied every RGB frame without recomputing keycode lookups.
static uint8_t  vis_cache[RGB_MATRIX_LED_COUNT][3];
static bool     vis_cache_valid = false;

static void build_cache(uint8_t layer) {
    for (uint8_t led = 0; led < RGB_MATRIX_LED_COUNT; led++) {
        uint16_t mtx  = led_to_mtx[led];
        uint8_t  row  = (mtx >> 8) & 0xFF;
        uint8_t  col  = mtx & 0xFF;
        uint16_t kc   = keycode_at_keymap_location_raw(layer, row, col);
        key_category_t cat = categorize(kc);
        vis_cache[led][0] = pgm_read_byte(&cat_colors[cat][0]);
        vis_cache[led][1] = pgm_read_byte(&cat_colors[cat][1]);
        vis_cache[led][2] = pgm_read_byte(&cat_colors[cat][2]);
    }
    vis_cache_valid = true;
}

static void apply_cache(void) {
    for (uint8_t led = 0; led < RGB_MATRIX_LED_COUNT; led++) {
        rgb_matrix_set_color(led,
                             vis_cache[led][0],
                             vis_cache[led][1],
                             vis_cache[led][2]);
    }
}

// ═════════════════════════════════════════════════════════════════════════════
// State
// ═════════════════════════════════════════════════════════════════════════════
// ═════════════════════════════════════════════════════════════════════════════

static bool   perm_active   = false;
static uint32_t perm_start  = 0;
static bool   moment_active = false;
static uint8_t vis_layer    = 0;

// Suppress visualization during init.  boot_done is set by init(), but
// the initial layer_move() in matrix_scan_user's default-layer sync also
// fires a trigger.  boot_done stays false until AFTER that sync completes.
// init() records the initial default layer so we can detect the sync.
static bool   boot_done = false;
static uint8_t boot_default_layer = 0;

// When an MO key is released, QMK processes the layer change AFTER
// process_record_user returns.  This flag tells the next trigger() call
// that it's caused by an MO release, so it should NOT start the timer.
static bool mo_release_pending = false;


// ═════════════════════════════════════════════════════════════════════════════
// Public API
// ═════════════════════════════════════════════════════════════════════════════

void layer_visualizer_trigger(void) {
    if (!feature_layer_vis()) return;

    // Suppress triggers during boot.  The first trigger is the initial
    // default-layer sync which we skip (boot_done still false here).
    if (!boot_done) {
        // Pop the guard: if this layer change is NOT the initial sync
        // (i.e. someone changes layer before matrix_scan runs), allow it.
        if (get_highest_layer(layer_state) != boot_default_layer) {
            boot_done = true;
        }
        return;
    }

    // Suppress trigger if it's the result of an MO key release.
    // The flag is set by momentary_stop() and consumed here.
    if (mo_release_pending) {
        mo_release_pending = false;
        vis_layer = get_highest_layer(layer_state);  // still update in case
        return;  // but don't restart the timer
    }

    if (moment_active) return;

    vis_layer   = get_highest_layer(layer_state);
    vis_cache_valid = false;
    perm_active = true;
    perm_start  = timer_read32();
}

void layer_visualizer_momentary_start(uint8_t target_layer) {
    if (!feature_layer_vis()) return;
    if (feature_overview_is_active()) return;  // don't override overview

    moment_active = true;
    vis_layer     = target_layer;
    vis_cache_valid = false;
    perm_active   = false;
}

void layer_visualizer_momentary_stop(void) {
    moment_active = false;
    vis_cache_valid = false;  // next draw will rebuild for the permanent layer
    // Mark the upcoming layer-state change (triggered by QMK processing
    // the MO release) as something to ignore.
    mo_release_pending = true;
}

bool layer_visualizer_is_active(void) {
    return perm_active || moment_active;
}

// Called once after keyboard init + default-layer sync.
// Before this, all layer_state_set_user calls are ignored.
// We detect the initial sync by remembering the default layer.
void layer_visualizer_init(void) {
    boot_default_layer = get_highest_layer(default_layer_state);
}

// Called by matrix_scan_user after the initial default-layer sync.
// This allows the FIRST layer trigger (which happens during that sync)
// to be suppressed — we don't show visualization for boot setup.
void layer_visualizer_sync_complete(void) {
    boot_done = true;
}

void layer_visualizer_task(void) {
    if (moment_active)          return;
    if (!perm_active)           return;
    if (timer_elapsed32(perm_start) > LAYER_VIS_TIMEOUT_MS) {
        perm_active = false;
    }
}

void layer_vis_toggle(void) {
    feature_toggle_layer_vis();
    if (feature_layer_vis()) {
        layer_visualizer_trigger();
    }
}


// ═════════════════════════════════════════════════════════════════════════════
// Drawing  (uses cached per-LED colors)
// ═════════════════════════════════════════════════════════════════════════════

static void draw_layer(uint8_t layer) {
    if (!vis_cache_valid) build_cache(layer);
    apply_cache();
}

void layer_visualizer_draw(void) {
    draw_layer(vis_layer);
}
