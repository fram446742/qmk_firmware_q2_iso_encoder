/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H
#include "keychron_common.h"
#include "features.h"
#include "eeprom.h"
#include "send_string.h"
#include "unicode.h"
#include "dynamic_keymap.h"

// ═════════════════════════════════════════════════════════════════════════════
// All configuration constants (TAP_TERM, MAX_*, EEP_*, defaults, etc.) are
// in keymap_config.h, included from features.h.
// ═════════════════════════════════════════════════════════════════════════════

#include "keymap_config.h"

#ifdef AUTO_SHIFT_ENABLE
#    include "process_auto_shift.h"
#endif

#ifdef CAPS_WORD_ENABLE
#    include "caps_word.h"
#endif


// ═════════════════════════════════════════════════════════════════════════════
// Runtime state
// ═════════════════════════════════════════════════════════════════════════════

uint8_t g_feature_flags = 0;

eeprom_tap_t    eeprom_tap[MAX_TAP_OVERRIDES];
uint8_t         eeprom_tap_count   = 0;
eeprom_combo_t  eeprom_combos[MAX_COMBOS];
uint8_t         eeprom_combo_count = 0;
eeprom_leader_t eeprom_leaders[MAX_LEADERS];
uint8_t         eeprom_leader_count = 0;


// ═════════════════════════════════════════════════════════════════════════════
// Tap-dance transparent override  (timer-based, no TAP_DANCE_ENABLE)
// ═════════════════════════════════════════════════════════════════════════════

static int8_t   tap_pending_idx = -1;
static uint16_t tap_timer       = 0;

// ── Dispatch a double-tap action from the EEPROM entry ─────────────────────

static void tap_fire_override(const eeprom_tap_t *ov) {
    switch (ov->dbl_type) {
        case TD_DBL_KEYCODE: {
            uint16_t kc = ov->dbl_val;
            switch (kc) {
#ifdef CAPS_WORD_ENABLE
                case CW_TOGG:
                    caps_word_toggle();
                    break;
#endif
                default:
                    if (kc >= QK_UNICODE && kc <= QK_UNICODE_MAX) {
                        register_unicode(kc & 0x7FFF);
                    } else {
                        tap_code16(kc);
                    }
                    break;
            }
            break;
        }
        case TD_DBL_UNICODE_STR: {
            uint8_t buf[5];
            buf[0] =  ov->dbl_val       & 0xFF;
            buf[1] = (ov->dbl_val >> 8) & 0xFF;
            buf[2] =  ov->dbl_extra     & 0xFF;
            buf[3] = (ov->dbl_extra >> 8) & 0xFF;
            buf[4] = 0;
            send_unicode_string((const char *)buf);
            break;
        }
        case TD_DBL_UNICODE_CP:
            register_unicode(ov->dbl_val | ((uint32_t)ov->dbl_extra << 16));
            break;
        case TD_DBL_SEND_STRING: {
            uint8_t buf[5];
            buf[0] =  ov->dbl_val       & 0xFF;
            buf[1] = (ov->dbl_val >> 8) & 0xFF;
            buf[2] =  ov->dbl_extra     & 0xFF;
            buf[3] = (ov->dbl_extra >> 8) & 0xFF;
            buf[4] = 0;
            send_string((const char *)buf);
            break;
        }
    }
}

// ── Main key interception ──────────────────────────────────────────────────
// Matches by keycode (base_type=0) or matrix position (base_type=1).
// Position-based entries follow the physical key regardless of layer/layout.

static bool tap_matches(const eeprom_tap_t *ov, uint16_t keycode, keyrecord_t *record) {
    if (ov->base_type == BASE_IS_MATRIX) {
        uint8_t r = record->event.key.row;
        uint8_t c = record->event.key.col;
        return ((uint16_t)(r << 8) | c) == ov->base_id;
    }
    return keycode == ov->base_id;  // BASE_IS_KEYCODE
}

bool features_tap_process(uint16_t keycode, keyrecord_t *record) {
    // Modifier bypass: if any modifier is held (Ctrl, Alt, Shift, GUI),
    // skip the tap override and let the key pass through immediately.
    // This preserves modifier+key combinations like Ctrl+Backspace.
    uint8_t mods = get_mods();
    if (mods != 0) {
        return true;  // Let the key pass through normally
    }

    for (int i = 0; i < eeprom_tap_count; i++) {
        if (!tap_matches(&eeprom_tap[i], keycode, record)) continue;

        if (record->event.pressed) {
            uint16_t now = timer_read();

            if (tap_pending_idx == i && timer_elapsed(tap_timer) <= TAP_TERM) {
                tap_pending_idx = -1;
                tap_fire_override(&eeprom_tap[i]);
                return false;
            }

            if (tap_pending_idx >= 0) {
                tap_code16(eeprom_tap[tap_pending_idx].tap_kc);
                tap_pending_idx = -1;
            }

            tap_pending_idx = i;
            tap_timer       = now;
            return false;
        } else {
            // Always consume the release of a matched key, even if not
            // currently pending (the press was already consumed above).
            // Letting the release through could unregister a keycode that
            // was never sent to the host.
            return false;
        }
    }

    if (tap_pending_idx >= 0) {
        tap_code16(eeprom_tap[tap_pending_idx].tap_kc);
        tap_pending_idx = -1;
    }
    return true;
}

// ── Periodic timeout check ─────────────────────────────────────────────────

void features_tap_task(void) {
    if (tap_pending_idx >= 0 && timer_elapsed(tap_timer) > TAP_TERM) {
        int8_t idx = tap_pending_idx;
        tap_pending_idx = -1;
        tap_code16(eeprom_tap[idx].tap_kc);
    }
}


// ═════════════════════════════════════════════════════════════════════════════
// Esc key — modifier handling  (GESC_ALTGR_MODS / GESC_STRIP_MODS, §14)
// ═════════════════════════════════════════════════════════════════════════════
// QK_GESC is US-centric: it sends KC_ESCAPE unless Shift/GUI is held, and when
// it does send KC_GRAVE it sends it *together with* the held modifiers.  Both
// modifiers hold the character back on an es-ES layout:
//
//   * AltGr (RAlt) — QMK sends KC_ESCAPE, so the host saw RAlt+Esc; on Windows
//     Alt+Esc is the window-switch hotkey and nothing is typed.
//   * GUI (Win/Cmd) — the host saw GUI+KC_GRAVE and Windows consumes Win+<key>
//     as a shell shortcut (same reason Win+E types no 'e'), so no character
//     appears.  This is QMK issue #3769, never fixed upstream.
//
// Both cases become a plain KC_GRV (HID 0x35) and the OS layout supplies the
// character: AltGr + 0x35 = `\` on es-ES (level 3), plain 0x35 = `º`
// (level 1).  The modifier handling differs, though:
//   * AltGr must STAY in the report — the layout needs it to reach level 3.
//   * GUI must be HIDDEN from the report (del_mods without sending, so only the
//     keystroke's own report is affected) — otherwise Windows still sees it as
//     a Win shortcut.  It is not restored; see the caveat below.
//
// The release is consumed too, matched via the flag rather than the live
// modifier state, so letting a modifier go first can't leak a stray Escape.

typedef enum { GESC_NONE = 0, GESC_ALTGR, GESC_GUI } gesc_mode_t;

static gesc_mode_t gesc_mode = GESC_NONE;

bool features_gesc_process(uint16_t keycode, keyrecord_t *record) {
    if (keycode != QK_GESC) return true;

    if (record->event.pressed) {
        uint8_t mods = get_mods();

        if (mods & GESC_ALTGR_MODS) {
            gesc_mode = GESC_ALTGR;
            register_code(KC_GRV);  // report keeps AltGr → layout emits level 3
            return false;
        }
        if (mods & GESC_STRIP_MODS) {
            gesc_mode = GESC_GUI;
            del_mods(GESC_STRIP_MODS);  // no report sent: hides GUI from the next one only
            register_code(KC_GRV);      // report: no GUI + 0x35 → layout emits level 1
            return false;
        }
        return true;
    }

    if (gesc_mode == GESC_NONE) return true;
    gesc_mode = GESC_NONE;
    unregister_code(KC_GRV);
    return false;
}


// ═════════════════════════════════════════════════════════════════════════════
// Position combo processor  (custom — matrix-position and keycode combos)
// ═════════════════════════════════════════════════════════════════════════════
// Independent of QMK-native combos (key_combos[] in combos.c) and of the
// feature-overview chord (handled by position in pre_process_record_user).
// Runs from process_record_user; matches keys by matrix position (BASE_IS_MATRIX)
// or resolved keycode (BASE_IS_KEYCODE).  POS_COMBOS_DEFS is empty by default;
// the chord keys (1,9)/(1,11) are reserved (compile-time check in
// keymap_config.h).

static const pos_combo_def_t pos_combos[] = {
    POS_COMBOS_DEFS
};
#define POS_COMBO_COUNT ((uint8_t)(sizeof(pos_combos) / sizeof(pos_combos[0])))
#define POS_COMBO_MAX_KEYS 4

static struct {
    uint8_t  down;         ///< bitmask of keys currently held (being tracked)
    uint16_t timer;        ///< time the first key went down
    bool     fired;        ///< output already sent
    uint8_t  live;         ///< bitmask of members re-pressed as a held key
    uint16_t live_kc[POS_COMBO_MAX_KEYS];  ///< keycode each live member was registered with
} pos_cb_state[POS_COMBO_COUNT];

/// Resolve the keycode a combo member should (re)produce, on the current layer.
static uint16_t combo_member_kc(const pos_combo_def_t *cb, uint8_t ki) {
    if (cb->base_type == BASE_IS_MATRIX) {
        uint16_t mtx = cb->keys[ki];
        uint16_t kc  = dynamic_keymap_get_keycode(get_highest_layer(layer_state), (mtx >> 8) & 0xFF, mtx & 0xFF);
        return (kc == KC_TRNS) ? KC_NO : kc;
    }
    return cb->keys[ki];
}

/// Register a held-back member as a live held key (down), now that its chord
/// can no longer form.  Output must go out immediately — if it waited for the
/// member's release, a following key pressed in the meantime would type first
/// and fast rolls would transpose ("on" → "no", "om" → "mo").  The member's
/// real release lifts it via the `live` bookkeeping.
static void combo_commit_member(uint8_t ci, uint8_t ki, uint8_t bit) {
    uint16_t kc = combo_member_kc(&pos_combos[ci], ki);
    if (kc == KC_NO || kc == KC_TRNS) return;
    register_code16(kc);
    pos_cb_state[ci].live |= bit;
    pos_cb_state[ci].live_kc[ki] = kc;
    pos_cb_state[ci].down &= ~bit;
}

bool features_combo_process(uint16_t keycode, keyrecord_t *record) {
    uint16_t mtx_pos = PACK_MTX(record->event.key.row, record->event.key.col);

    for (uint8_t ci = 0; ci < POS_COMBO_COUNT; ci++) {
        const pos_combo_def_t *cb = &pos_combos[ci];
        uint8_t ki;
        for (ki = 0; ki < cb->key_count; ki++) {
            bool match = (cb->base_type == BASE_IS_MATRIX) ? (cb->keys[ki] == mtx_pos)
                                                           : (cb->keys[ki] == keycode);
            if (match) break;
        }
        if (ki >= cb->key_count) continue; // not part of this combo

        uint8_t bit = (1 << ki);
        if (record->event.pressed) {
            // (Re-)arm after a previous full chord.
            if (pos_cb_state[ci].fired && pos_cb_state[ci].down == 0) pos_cb_state[ci].fired = false;

            pos_cb_state[ci].down |= bit;
            if (pos_cb_state[ci].down == (uint8_t)((1 << cb->key_count) - 1)) {
                // All keys held → fire.  Route through process_record so a
                // custom keycode reaches process_record_user().
                pos_cb_state[ci].fired = true;
                pos_cb_state[ci].down  = 0;
                pos_cb_state[ci].timer = 0;
                keyrecord_t combo_record = {.event = MAKE_COMBOEVENT(true), .keycode = cb->output};
                process_record(&combo_record);
                return false;
            }
            if (pos_cb_state[ci].timer == 0) pos_cb_state[ci].timer = timer_read();
            return false; // consumed — held back until the chord resolves
        } else {
            // ── Release ──
            if (pos_cb_state[ci].fired) {
                // Combo already fired; its opens are consumed by the modal.
                pos_cb_state[ci].down = pos_cb_state[ci].timer = 0;
                return false;
            }
            if (pos_cb_state[ci].live & bit) {
                // Member was re-pressed as a held key (timeout) — lift it now.
                unregister_code16(pos_cb_state[ci].live_kc[ki]);
                pos_cb_state[ci].live &= ~bit;
                return false; // consumed; we owned this member
            }
            // Never fired and not live → a single quick key.  We consumed its
            // press, so replay it now (down+up) or the tap would be lost.
            bool was_only = (pos_cb_state[ci].down == bit);
            pos_cb_state[ci].down &= ~bit;
            if (pos_cb_state[ci].down == 0) pos_cb_state[ci].timer = 0;
            if (was_only) {
                uint16_t kc = combo_member_kc(cb, ki);
                if (kc != KC_NO && kc != KC_TRNS) tap_code16(kc);
            }
            return false; // consumed; we owned this member
        }
    }
    // Not a member of any combo.  If a member of a pending chord is still
    // held back (down, not yet fired), that chord can no longer form — an
    // unrelated key means the partner was never the next key.  Commit the
    // held member(s) NOW, before this key types, so a fast roll keeps its
    // order ("on" stays "on", never "no").  Mirrors QMK-native combos, which
    // dump buffered single keys when an unrelated key breaks the chord.
    if (record->event.pressed) {
        for (uint8_t ci = 0; ci < POS_COMBO_COUNT; ci++) {
            if (pos_cb_state[ci].fired || pos_cb_state[ci].down == 0) continue;
            for (uint8_t ki = 0; ki < pos_combos[ci].key_count; ki++) {
                uint8_t bit = (1 << ki);
                if (pos_cb_state[ci].down & bit) combo_commit_member(ci, ki, bit);
            }
            if (pos_cb_state[ci].down == 0) pos_cb_state[ci].timer = 0;
        }
    }
    return true; // not handled
}

/// Drop all combo tracking (call when a modal screen opens — it will consume
/// the releases, so stale "down/fired/live" must not survive).
void features_combo_clear(void) {
    for (uint8_t ci = 0; ci < POS_COMBO_COUNT; ci++) {
        for (uint8_t ki = 0; ki < pos_combos[ci].key_count; ki++) {
            if (pos_cb_state[ci].live & (1 << ki)) unregister_code16(pos_cb_state[ci].live_kc[ki]);
        }
        pos_cb_state[ci].down  = 0;
        pos_cb_state[ci].timer = 0;
        pos_cb_state[ci].fired = false;
        pos_cb_state[ci].live  = 0;
    }
}

void features_combo_task(void) {
    // A member still held past COMBO_TERM with no partner → re-press it as a
    // held key (register down) so holding/auto-repeat behaves like typing it.
    for (uint8_t ci = 0; ci < POS_COMBO_COUNT; ci++) {
        if (!pos_cb_state[ci].down || pos_cb_state[ci].fired) continue;
        if (timer_elapsed(pos_cb_state[ci].timer) <= COMBO_TERM) continue;
        for (uint8_t ki = 0; ki < pos_combos[ci].key_count; ki++) {
            uint8_t bit = (1 << ki);
            if (pos_cb_state[ci].down & bit) combo_commit_member(ci, ki, bit);
        }
        if (pos_cb_state[ci].down == 0) pos_cb_state[ci].timer = 0;
    }
}

// ═════════════════════════════════════════════════════════════════════════════

static void feature_apply_flag(uint8_t flag) {
    // Most feature flags gate runtime behavior in process_record_user()
    // via the feature_*() convenience wrappers. Only auto-shift has a
    // direct enable/disable API that needs immediate application.
    (void)flag;

#ifdef AUTO_SHIFT_ENABLE
    if (flag == FEATURE_AUTO_SHIFT) {
        if (feature_auto_shift()) autoshift_enable();
        else                      autoshift_disable();
    }
#endif
}

void feature_apply_all(void) {
    feature_apply_flag(FEATURE_TAP_DANCE);
    feature_apply_flag(FEATURE_AUTO_SHIFT);
    feature_apply_flag(FEATURE_CAPS_WORD);
    feature_apply_flag(FEATURE_REPEAT_KEY);
    feature_apply_flag(FEATURE_DYN_MACRO);
    feature_apply_flag(FEATURE_LEADER);
    feature_apply_flag(FEATURE_LAYER_VIS);
}

void features_init(void) {
    g_feature_flags = eeprom_read_byte((const uint8_t *)EEP_FEATURES);

    // First boot: EEPROM is erased (0xFF) or zeroed — set defaults
    if (g_feature_flags == 0xFF || g_feature_flags == 0) {
        g_feature_flags = DEFAULT_FEATURE_FLAGS;
        features_save();
        features_load_defaults();
        features_save_config();
    }

    features_load_config();
    feature_apply_all();
}

void features_save(void) {
    eeprom_write_byte((uint8_t *)EEP_FEATURES, g_feature_flags);
}

bool feature_has(uint8_t flag) {
    return (g_feature_flags & flag) != 0;
}

void feature_toggle(uint8_t flag) {
    g_feature_flags ^= flag;
    feature_apply_flag(flag);
    features_save();
}

void feature_set(uint8_t flag, bool on) {
    if (on) {
        g_feature_flags |= flag;
    } else {
        g_feature_flags &= ~flag;
    }
    feature_apply_flag(flag);
    features_save();
}


// ═════════════════════════════════════════════════════════════════════════════
// EEPROM config load / save / defaults
// ═════════════════════════════════════════════════════════════════════════════

void features_load_config(void) {
    eeprom_tap_count = eeprom_read_byte((const uint8_t *)EEP_TAP_BASE);
    if (eeprom_tap_count == 0xFF || eeprom_tap_count == 0 || eeprom_tap_count > MAX_TAP_OVERRIDES) {
        features_load_defaults();
        features_save_config();
        return;
    }
    // Read only the entries that are actually in use (count-sized),
    // not the full reserved region — fewer EEPROM reads at boot.
    eeprom_read_block(eeprom_tap, (void *)(EEP_TAP_BASE + 1), eeprom_tap_count * sizeof(eeprom_tap_t));

    eeprom_combo_count = eeprom_read_byte((const uint8_t *)EEP_COMBO_BASE);
    if (eeprom_combo_count == 0xFF || eeprom_combo_count > MAX_COMBOS) {
        eeprom_combo_count = 0;
    } else {
        eeprom_read_block(eeprom_combos, (void *)(EEP_COMBO_BASE + 1), eeprom_combo_count * sizeof(eeprom_combo_t));
    }

    eeprom_leader_count = eeprom_read_byte((const uint8_t *)EEP_LEADER_BASE);
    if (eeprom_leader_count == 0xFF || eeprom_leader_count > MAX_LEADERS) {
        eeprom_leader_count = 0;
    } else {
        eeprom_read_block(eeprom_leaders, (void *)(EEP_LEADER_BASE + 1), eeprom_leader_count * sizeof(eeprom_leader_t));
    }
}

void features_save_config(void) {
    // Write only the in-use entries — less EEPROM wear than writing the
    // full reserved region on every import.
    eeprom_write_byte((uint8_t *)EEP_TAP_BASE, eeprom_tap_count);
    eeprom_write_block(eeprom_tap, (void *)(EEP_TAP_BASE + 1), eeprom_tap_count * sizeof(eeprom_tap_t));
    eeprom_write_byte((uint8_t *)EEP_COMBO_BASE, eeprom_combo_count);
    eeprom_write_block(eeprom_combos, (void *)(EEP_COMBO_BASE + 1), eeprom_combo_count * sizeof(eeprom_combo_t));
    eeprom_write_byte((uint8_t *)EEP_LEADER_BASE, eeprom_leader_count);
    eeprom_write_block(eeprom_leaders, (void *)(EEP_LEADER_BASE + 1), eeprom_leader_count * sizeof(eeprom_leader_t));
}

// ── Sentinel-terminated default arrays (count derived at compile time) ─────
// The sentinel {0} entry is excluded from the count.

static const eeprom_tap_t    tap_defaults_all[]    = { TAP_DEFAULTS };
static const eeprom_combo_t  combo_defaults_all[]  = { COMBO_DEFAULTS };
static const eeprom_leader_t leader_defaults_all[] = { LEADER_DEFAULTS };

#define TAP_DEFAULTS_COUNT    ((sizeof(tap_defaults_all)    / sizeof(tap_defaults_all[0]))    - 1)
#define COMBO_DEFAULTS_COUNT  ((sizeof(combo_defaults_all)  / sizeof(combo_defaults_all[0]))  - 1)
#define LEADER_DEFAULTS_COUNT ((sizeof(leader_defaults_all) / sizeof(leader_defaults_all[0])) - 1)

void features_load_defaults(void) {
    memset(eeprom_tap,    0, sizeof(eeprom_tap));
    memset(eeprom_combos, 0, sizeof(eeprom_combos));
    memset(eeprom_leaders,0, sizeof(eeprom_leaders));

    eeprom_tap_count = TAP_DEFAULTS_COUNT;
    for (int i = 0; i < TAP_DEFAULTS_COUNT && i < MAX_TAP_OVERRIDES; i++) {
        eeprom_tap[i] = tap_defaults_all[i];
    }

    eeprom_combo_count = COMBO_DEFAULTS_COUNT;
    for (int i = 0; i < COMBO_DEFAULTS_COUNT && i < MAX_COMBOS; i++) {
        eeprom_combos[i] = combo_defaults_all[i];
    }

    eeprom_leader_count = LEADER_DEFAULTS_COUNT;
    for (int i = 0; i < LEADER_DEFAULTS_COUNT && i < MAX_LEADERS; i++) {
        eeprom_leaders[i] = leader_defaults_all[i];
    }
}


// ═════════════════════════════════════════════════════════════════════════════
// HID config protocol  (via_custom_value_command_kb — qmk_config_tool.py)
// ═════════════════════════════════════════════════════════════════════════════
//
// Strong override of the weak handler in quantum/via.c.  Speaks QMK's VIA
// custom-value protocol over Raw HID so the host tool (qmk_config_tool.py
// in this keymap directory) can export/import the EEPROM-backed feature
// config.  Value IDs (VALUE_*) and wire layout are defined in
// keymap_config.h — the Python tool parses that header to stay in sync.
//
// Wire format (32-byte VIA custom value payload):
//   data[0] = command  0x08 read (get) | 0x07 write (set) | 0x09 save
//   data[1] = channel  (0)
//   data[2] = value ID (VALUE_*)
//   data[3] = index    (entry index, or value byte for single-byte values)
//   data[4..] = payload (entry data on write; response on read)

void via_custom_value_command_kb(uint8_t *data, uint8_t length) {
    uint8_t cmd      = data[0];
    uint8_t vid      = data[2];
    uint8_t idx      = data[3];
    uint8_t *pay     = data + 4;
    uint8_t pay_len  = length - 4;

    if (data[1] != 0x00) return;

    if (cmd == 0x08) {  // READ (get)
        switch (vid) {
            case VALUE_FLAGS:        data[3] = g_feature_flags;                              break;
            case VALUE_TAP_COUNT:    data[3] = eeprom_tap_count;                             break;
            case VALUE_TAP_ENTRY:
                if (idx < eeprom_tap_count && pay_len >= sizeof(eeprom_tap_t))
                    memcpy(pay, &eeprom_tap[idx], sizeof(eeprom_tap_t));
                break;
            case VALUE_COMBO_COUNT:  data[3] = eeprom_combo_count;                           break;
            case VALUE_COMBO_ENTRY:
                if (idx < eeprom_combo_count && pay_len >= sizeof(eeprom_combo_t))
                    memcpy(pay, &eeprom_combos[idx], sizeof(eeprom_combo_t));
                break;
            case VALUE_LEADER_COUNT: data[3] = eeprom_leader_count;                          break;
            case VALUE_LEADER_ENTRY:
                if (idx < eeprom_leader_count && pay_len >= sizeof(eeprom_leader_t))
                    memcpy(pay, &eeprom_leaders[idx], sizeof(eeprom_leader_t));
                break;
        }
    } else if (cmd == 0x07) {  // WRITE (set)
        switch (vid) {
            case VALUE_FLAGS:
                g_feature_flags = idx;
                feature_apply_all();
                features_save();
                break;
            case VALUE_TAP_COUNT:
                eeprom_tap_count = (idx < MAX_TAP_OVERRIDES) ? idx : MAX_TAP_OVERRIDES;
                break;
            case VALUE_TAP_ENTRY:
                if (idx < eeprom_tap_count && pay_len >= sizeof(eeprom_tap_t))
                    memcpy(&eeprom_tap[idx], pay, sizeof(eeprom_tap_t));
                break;
            case VALUE_COMBO_COUNT:
                eeprom_combo_count = (idx < MAX_COMBOS) ? idx : MAX_COMBOS;
                break;
            case VALUE_COMBO_ENTRY:
                if (idx < eeprom_combo_count && pay_len >= sizeof(eeprom_combo_t))
                    memcpy(&eeprom_combos[idx], pay, sizeof(eeprom_combo_t));
                break;
            case VALUE_LEADER_COUNT:
                eeprom_leader_count = (idx < MAX_LEADERS) ? idx : MAX_LEADERS;
                break;
            case VALUE_LEADER_ENTRY:
                if (idx < eeprom_leader_count && pay_len >= sizeof(eeprom_leader_t))
                    memcpy(&eeprom_leaders[idx], pay, sizeof(eeprom_leader_t));
                break;
        }
    } else if (cmd == 0x09) {  // SAVE (persist to EEPROM)
        features_save_config();
    }
}
