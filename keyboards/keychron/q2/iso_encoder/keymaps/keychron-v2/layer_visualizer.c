/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "layer_visualizer.h"
#include "rgb_matrix_drivers.h"
#include "keymap_config.h"
#include "features.h"
#include "feature_overview.h"
#include "keycodes.h"
#include "keymap_introspection.h"
#include "dynamic_keymap.h"
#include "layer_picker.h"


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

// ── Mac modifier keys (QK_KB range) ────────────────────────────────────────
// KC_LOPTN (QK_KB_0) through KC_RCMMD (QK_KB_3) — the four Option/Cmd keys.
// Keychron's other QK_KB keys (KC_TASK, KC_FILE, KC_SNAP, KC_CTANA, KC_SIRI,
// KC_MAC_MISSION_CONTROL, KC_MAC_LAUCHPAD, …) fall through to CAT_SPECIAL.
#define IS_MAC_EXTRA(kc)  ((kc) >= KC_LOPTN && (kc) <= KC_RCMMD)

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

static bool     perm_active   = false;
static uint32_t perm_start    = 0;
static bool     moment_active = false;
static uint8_t  vis_layer     = 0;  ///< cached layer for timer mode only
static uint16_t mo_positions[MAX_HELD_MO];  ///< held MO keys, by matrix position (MAX_HELD_MO in keymap_config.h)
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

// RGB feedback: temporarily disable layer visualization for 1 second
// so the user can see the actual RGB effect changes.  (Duration is in
// keymap_config.h — RGB_FEEDBACK_DURATION_MS.)
static bool     rgb_feedback_active = false;
static uint32_t rgb_feedback_timer  = 0;


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
            mo_release_pending = true;
        }
        return;
    }
}

/// Start the timer-based (permanent) overlay for the given layer.
static void start_perm_display(uint8_t layer) {
    vis_layer       = layer;
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
        rgb_feedback_active = false;
        return;
    }
    
    // Cancel RGB feedback if layer visualization is no longer active
    // (e.g., FN key was released during RGB feedback)
    if (rgb_feedback_active && !moment_active && !perm_active) {
        rgb_feedback_active = false;
    }
    
    // RGB feedback timeout
    if (rgb_feedback_active) {
        if (timer_elapsed32(rgb_feedback_timer) > RGB_FEEDBACK_DURATION_MS) {
            rgb_feedback_active = false;
            // No restart here: any perm/moment show that was active when the
            // feedback started simply resumes with its original remaining time
            // (the overlay is repainted on the activation transition).  The
            // timer below then expires it as usual.
        }
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

void rgb_feedback_trigger(void) {
    if (!feature_layer_vis()) return;

    // A layer-visualization show is in progress (timer or moment): keep it.
    // Without this, an RGB/underglow key on the newly shown layer (e.g. the
    // FN layers' UG_* keys and UG knob mappings) would cut the display off
    // for the 1 s feedback window and then restart it — the flicker seen on
    // some layers.
    if (perm_active || moment_active) return;

    // Just activate RGB feedback mode - don't modify perm_active or moment_active
    // This allows normal state tracking (e.g., FN key release) to work correctly
    rgb_feedback_active = true;
    rgb_feedback_timer  = timer_read32();
}

// ═════════════════════════════════════════════════════════════════════════════
// Drawing
// ═════════════════════════════════════════════════════════════════════════════
//
// Reads keycodes from the LIVE dynamic keymap (EEPROM-backed) on every
// frame, so remapping a key in the Keychron Launcher / VIA immediately
// updates the overlay colors.  The EEPROM reads are fast (flash-backed
// NVM on STM32, ~100 ns each) — 68 LEDs × 2 bytes ≈ 14 µs per frame.

// Overlay display buffer (one pwm_buffer-sized block per driver).  The overlay
// (layer visualization OR feature overview) renders here instead of into the
// driver's pwm_buffer, so lazy effects (jellybean raindrops, pixel rain) keep
// their per-key state.  snled27351_flush_override points at this buffer while
// the overlay is showing, so the driver flushes it instead of the pwm_buffer.
static uint8_t overlay_pwm[SNLED27351_DRIVER_COUNT][SNLED27351_LED_PWM_LENGTH] = {0};

// Writes one LED into the overlay buffer.  Skips unchanged pixels and only
// marks the overlay dirty when a pixel actually changes, so the driver's flush
// (which keys off snled27351_overlay_dirty) is skipped while the overlay is
// static.  Shared with the feature overview (indicators.c).
void overlay_set_color(uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
    snled27351_led_t snled;
    memcpy_P(&snled, &g_snled27351_leds[led], sizeof(snled));

    uint8_t *px = overlay_pwm[snled.driver];
    if (px[snled.r] == r && px[snled.g] == g && px[snled.b] == b) {
        return;  // unchanged — leave the dirty flag alone
    }
    px[snled.r] = r;
    px[snled.g] = g;
    px[snled.b] = b;
    snled27351_overlay_dirty = true;
}

void overlay_clear_all(void) {
    for (uint8_t led = 0; led < RGB_MATRIX_LED_COUNT; led++) {
        overlay_set_color(led, 0, 0, 0);
    }
}

static void draw_layer(uint8_t layer) {
    for (uint8_t led = 0; led < RGB_MATRIX_LED_COUNT; led++) {
        uint16_t mtx  = led_to_mtx[led];
        uint8_t  row  = (mtx >> 8) & 0xFF;
        uint8_t  col  = mtx & 0xFF;
        uint16_t kc   = dynamic_keymap_get_keycode(layer, row, col);
        key_category_t cat = categorize(kc);
        overlay_set_color(led,
                          pgm_read_byte(&cat_colors[cat][0]),
                          pgm_read_byte(&cat_colors[cat][1]),
                          pgm_read_byte(&cat_colors[cat][2]));
    }
}

void layer_visualizer_draw(void) {
    // RGB feedback takes priority: suppress the overlay so the effect is
    // visible while the feedback window is active.
    if (rgb_feedback_active) {
        return;
    }

    if (moment_active) {
        // During MO holds, always read QMK's live layer state.
        uint8_t live = get_highest_layer(layer_state);
        draw_layer(live);
    } else {
        draw_layer(vis_layer);
    }
}

/// Called every RGB frame (from rgb_matrix_indicators_advanced_user) before
/// the indicators are drawn.  Toggles the driver's flush override so the
/// overlay's pixels (layer visualization OR feature overview, both in
/// overlay_pwm) are displayed instead of the effect's pwm_buffer — without
/// ever clobbering the effect's per-key state, which lazy effects (jellybean
/// raindrops, pixel rain) store in the buffer.
void layer_visualizer_frame(void) {
    static bool prev_showing = false;
    bool        showing      = feature_overview_is_active()
                            || layer_picker_is_active()
                            || ((perm_active || moment_active) && !rgb_feedback_active);

    if (showing && !prev_showing) {
        snled27351_flush_override = &overlay_pwm[0][0];
        // The overlay buffer may already hold the exact colors from a previous
        // activation (same layer/keymap), so overlay_set_color would early-return
        // and leave it "clean".  Force a flush so the display actually switches
        // from the effect's pwm_buffer to the overlay.
        snled27351_overlay_dirty = true;
    } else if (!showing && prev_showing) {
        snled27351_flush_override = NULL;
        // Force the effect's pwm_buffer to be flushed once — it may not be
        // dirty (static effects early-return), yet the hardware still shows the
        // overlay and must be repainted with the effect's colors.
        snled27351_force_flush = true;
    }
    prev_showing = showing;
}
