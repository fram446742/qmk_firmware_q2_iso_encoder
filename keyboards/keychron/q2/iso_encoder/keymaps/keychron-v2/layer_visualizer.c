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
    [CAT_BYPASS]    = LV_COLOR_BYPASS,    [CAT_BLANK]     = LV_COLOR_BLANK,
    [CAT_MODIFIER]  = LV_COLOR_MODIFIER,  [CAT_MAC_EXTRA] = LV_COLOR_MAC_EXTRA,
    [CAT_FUNCTION]  = LV_COLOR_FUNCTION,  [CAT_BASIC]     = LV_COLOR_BASIC,
    [CAT_MEDIA]     = LV_COLOR_MEDIA,     [CAT_MACRO]     = LV_COLOR_MACRO,
    [CAT_SPECIAL]   = LV_COLOR_SPECIAL,   [CAT_LIGHT]     = LV_COLOR_LIGHT,
    [CAT_CUSTOM]    = LV_COLOR_CUSTOM,    [CAT_LAYER]     = LV_COLOR_LAYER,
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
//
// Held MO keys are tracked by MATRIX POSITION (PACK_MTX(row, col)), not by
// counting layer-bit removals:
//   - PRESS:   process_record_user detects IS_QK_MOMENTARY reliably because
//              the MO layer hasn't been added when the keycode resolves.
//   - RELEASE: process_record_user matches the key's position against the
//              held set on EVERY key release — no keycode resolution, so it
//              works even if another MO altered the layer stack.
//
// Layer bits can be added/removed without any MO press/release (VIA/Launcher
// layer commands, the default-layer sync's layer_move(), TO/TG/DF) — a
// bitmask-diff counter would desync on those, so the position set is the only
// bookkeeping.  During moment mode the draw reads the LIVE layer_state every
// frame — the same value state_notify.c reports to the Keychron Launcher — so
// the overlay always matches the layer the keyboard is actually on.

#define MAX_HELD_MO 8

static bool     perm_active   = false;
static uint32_t perm_start    = 0;
static bool     moment_active = false;
static uint8_t  vis_layer     = 0;  ///< cached layer for timer mode only
static uint16_t mo_positions[MAX_HELD_MO];  ///< held MO keys, by matrix position
static uint8_t  mo_count      = 0;  ///< how many MO keys held
static bool     vis_locked    = false;  ///< lock: no auto-hide until next layer change

// Suppress visualization during init.
static bool boot_done = false;

// Suppress timer restart on last-MO-release cleanup.
static bool mo_release_pending = false;

// Set on the first key press after boot.  Layer changes that arrive
// before the user touches the keyboard (initial default-layer sync, USB
// enumeration, Launcher/VIA connect commands) never start a display.
static bool user_activity = false;


// ═════════════════════════════════════════════════════════════════════════════
// Public API
// ═════════════════════════════════════════════════════════════════════════════

void layer_visualizer_momentary_start(uint16_t mtx_pos) {
    if (!feature_layer_vis()) return;
    if (feature_overview_is_active()) return;

    // Dedupe: a position can only be held once
    for (uint8_t i = 0; i < mo_count; i++) {
        if (mo_positions[i] == mtx_pos) return;
    }

    if (mo_count == 0) {
        // First MO press — enter moment mode
        moment_active = true;
        perm_active   = false;
        mo_release_pending = false;
        vis_cache_valid = false;
    }
    if (mo_count < MAX_HELD_MO) {
        mo_positions[mo_count++] = mtx_pos;
    }
}

void layer_visualizer_momentary_release(uint16_t mtx_pos) {
    if (!feature_layer_vis()) return;

    // Find this position — no-op for non-MO keys.
    for (uint8_t i = 0; i < mo_count; i++) {
        if (mo_positions[i] != mtx_pos) continue;

        // Remove the entry (shift the rest down)
        for (uint8_t j = i; j + 1 < mo_count; j++) {
            mo_positions[j] = mo_positions[j + 1];
        }
        mo_count--;

        if (mo_count == 0) {
            // Last MO released — exit moment mode.  The upcoming
            // layer_state_set_user runs trigger() which sees
            // mo_release_pending and skips starting the timer.
            moment_active = false;
            vis_cache_valid = false;
            mo_release_pending = true;
        } else {
            // Other MO(s) still held — the layer stack changed; draw() reads
            // the live layer anyway.  Invalidate the cache so the live
            // layer's colors are recomputed.
            vis_cache_valid = false;
        }
        return;
    }
}

/// Start the timer-based (permanent) overlay for the given layer.
static void start_perm_display(uint8_t layer) {
    vis_layer       = layer;
    vis_cache_valid = false;
    perm_active     = true;
    perm_start      = timer_read32();
}

/// Called from layer_state_set_user with the NEW layer state.
void layer_visualizer_trigger(layer_state_t state) {
    if (!feature_layer_vis()) return;

    // Suppress everything until the initial default-layer sync completes
    // (first matrix scan).
    if (!boot_done) return;

    // Suppress layer changes that happen before the user touches the
    // keyboard — boot-sync remnants, USB enumeration, Launcher/VIA
    // connect commands.  The first key press arms the display.
    if (!user_activity) return;

    // No-op layer event — layer_on/layer_off of an already-set/cleared bit
    // still calls layer_state_set with an unchanged state.  Inside this hook
    // the global `layer_state` is still the pre-change value, so `state`
    // equals it exactly when nothing actually changed.
    if (state == layer_state) return;

    // Last MO release cleanup — skip timer start.
    if (mo_release_pending) {
        mo_release_pending = false;
        if (vis_locked) {
            // Locked: keep showing indefinitely
            start_perm_display(get_highest_layer(state));
        }
        return;
    }

    if (moment_active) {
        vis_cache_valid = false;
        return;
    }

    // Non-MO layer change (TO, TG, DF, layer_set, VIA command) — update display.
    start_perm_display(get_highest_layer(state));
}

// Called by matrix_scan_user after the initial default-layer sync.
// This allows the FIRST layer trigger (which happens during that sync)
// to be suppressed — we don't show visualization for boot setup.
void layer_visualizer_sync_complete(void) {
    boot_done    = true;
    moment_active = false;
    perm_active   = false;
    mo_count      = 0;
    mo_release_pending = false;
}

/// Mark that the user has interacted with the keyboard.  Call from
/// process_record_user on every key press.  This is the gate that keeps
/// boot-time layer changes (sync, Launcher/VIA connect) from starting
/// a display.
void layer_visualizer_mark_user_activity(void) {
    user_activity = true;
}

void layer_visualizer_task(void) {
    if (!feature_layer_vis()) {
        // Feature switched off (config tool): drop the lock latch and any
        // active overlay so re-enabling starts fresh.
        vis_locked = false;
        perm_active = false;
        moment_active = false;
        mo_count = 0;
        mo_release_pending = false;
        vis_cache_valid = false;
        return;
    }
    if (moment_active)          return;
    if (!perm_active)           return;
    if (vis_locked)             return;  // locked: no auto-hide
    if (timer_elapsed32(perm_start) > LAYER_VIS_TIMEOUT_MS) {
        perm_active = false;
    }
}

void layer_vis_toggle(void) {
    feature_toggle_layer_vis();
    if (feature_layer_vis()) {
        // Show the current layer immediately (bypasses trigger()'s
        // no-op guard — the state didn't change, we just enabled the feature).
        start_perm_display(get_highest_layer(layer_state));
    }
    // Disabling leaves cleanup to layer_visualizer_task() (same scan).
}

void layer_visualizer_cancel(void) {
    moment_active = false;
    perm_active   = false;
    mo_count      = 0;
    mo_release_pending = false;
    vis_cache_valid = false;
    // vis_locked is intentionally preserved — the lock is a latch that
    // survives overview; layer_visualizer_resume() restores the overlay
    // when overview exits.
}

void layer_visualizer_resume(void) {
    if (!feature_layer_vis()) return;
    if (!vis_locked)           return;
    if (feature_overview_is_active()) return;
    start_perm_display(get_highest_layer(layer_state));
}

void layer_visualizer_lock_toggle(void) {
    if (!feature_layer_vis()) return;  // master switch off — lock is inert

    vis_locked = !vis_locked;
    if (vis_locked) {
        // Enter locked mode.  The overlay only appears after overview
        // exits (layer_visualizer_resume()) — inside overview the toggle
        // state is shown by the IND_VIS_LOCK LED, like every other
        // feature toggle.  If an MO is held, moment mode already shows
        // the live layer and the release path starts the locked display.
        if (!moment_active && !feature_overview_is_active()) {
            start_perm_display(get_highest_layer(layer_state));
        }
    } else {
        // Unlock: drop the overlay.  During an MO hold the live moment
        // display continues; the release path now skips the permanent
        // display since the lock is off.
        if (!moment_active) {
            perm_active = false;
        }
    }
}

bool layer_visualizer_is_active(void) {
    if (!feature_layer_vis()) return false;
    return perm_active || moment_active;
}

bool layer_visualizer_is_locked(void) {
    return vis_locked && feature_layer_vis();
}


// ═════════════════════════════════════════════════════════════════════════════
// Drawing
// ═════════════════════════════════════════════════════════════════════════════

static void draw_layer(uint8_t layer) {
    if (!vis_cache_valid) build_cache(layer);
    apply_cache();
}

void layer_visualizer_draw(void) {
    if (moment_active) {
        // During MO holds, always read QMK's live layer state.
        uint8_t live = get_highest_layer(layer_state);
        draw_layer(live);
    } else {
        draw_layer(vis_layer);
    }
}
