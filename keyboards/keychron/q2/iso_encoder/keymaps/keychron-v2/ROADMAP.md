# Roadmap — planned features & improvements

Feature ideas with concrete implementation sketches. Nothing here is built yet.
For where things live today and the memory/flash budget, see `DIVERGENCES.md`.

**Status legend:** 📋 planned · ⏸️ paused (needs design) · ✅ done

---

## 📋 Per-layer RGB effects

**Goal:** the RGB effect (or its palette) changes when you switch layers, so e.g. the
FN layer glows a different way than the base layer.

**Why it fits:** the port already has the pieces — `layer_state_set_user` fires on every
layer change, and `rgb_matrix_config.mode`/`hsv` are settable at runtime.

**Sketch:**
1. Add a per-layer effect/hue table (9 layers × small struct) in `keymap_config.h`.
2. In `layer_state_set_user`, when the highest layer changes, save the current effect
   and load the target layer's effect via `rgb_matrix_mode_eeprom_helper()` / a direct
   `rgb_matrix_config` write + `eeconfig_update_rgb_matrix()`.
3. Config surface: add entries to the feature-overview screen or `qmk_config_tool.py`.

**Risks/decisions:**
- Interacts with the Mac/Win base switch (layer 0 vs 1) and the momentary FN layers —
  decide whether `MO` holds also swap the effect (probably not; only *base* layer changes).
- Keep it opt-in (a feature flag bit) so stock "one effect everywhere" behavior is preserved.
- No RAM cost (a PROGMEM table); ~200–400 B flash.

---

## 📋 Idle dimming

**Goal:** after N minutes without a keypress, fade to a dim idle animation; wake on the
first keypress.

**Sketch:**
1. Track last-activity time in `matrix_scan_user` (reset on any key event via
   `process_record_user`).
2. When `timer_elapsed(last_activity) > IDLE_MS`, dim the effect — simplest is
   `rgb_matrix_sethsv()` to a low value, or switch `rgb_matrix_config.mode` to a dim
   "breathing" effect; restore on wake.
3. Wake: any key press restores the saved mode/value.

**Risks/decisions:**
- Use the **host** `last_activity` / suspend path or a keymap timer? Keymap timer is
  simpler and self-contained; QMK also has `SUSPEND`/`rgb_matrix_indicators` hooks.
- Should the overlay/overview suppress idle dimming? Yes — gate it out while the overlay
  is showing (same as `rgb_feedback_active`).
- Constant I2C writes only when the value actually changes (reuse the dirty-tracked
  write pattern from the overlay).

---

## 📋 More leader sequences

**Goal:** expand `leader_end_user()` beyond the current 10 shortcuts.

**Sketch:** the dispatch in `keymap.c:leader_end_user()` is a flat `if/else if` chain over
`leader_sequence_one_key()`. Two options:

1. **Table-driven** (cleaner): a `PROGMEM` table `{uint8_t key; uint16_t action;}` and a
   loop. `action` uses `LEADER_MOD(on_mac, base)` so platform-awareness stays.
2. **Keep the chain** and just add `else if` branches.

**Ideas for sequences:**
- Window management: next/prev tab, close tab, reopen tab (`Cmd/Ctrl+Shift+T`).
- App launch: the port already has Keychron `KC_TASK`/`KC_FILE`/`KC_SNAP`/`KC_CTANA`
  (Task View, File Explorer, Snip, Cortana) — bind a few as leader sequences.
- Text snippets via `send_string()` (email, signature).
- Media: play/pause, prev/next, mute.

**Risks/decisions:** each new sequence is ~20 B flash (table) or ~40 B (branch). The
table-driven version is the better long-term shape if the list grows past ~15.

---

## ⏸️ Paused (need further adjustment)

| Feature | Why paused |
|---|---|
| **GUI config** | `qmk_config_tool.py` is CLI-only. A GUI (web/desktop) needs a decision on runtime (localhost web server on the MCU vs host-side desktop app) and how to keep the wire protocol (`via_custom_value_command_kb`) in sync with a second client. |
| **Accent expansion** | The tap-dance already handles `€`/`@`/`~`. Full accented-vowel set needs a decision on *which* keys to overload (they'd collide with the existing number-row `RAlt(N)` defaults) and how to present them in the overview/Launcher. |

---

## Ordering

1. **More leader sequences** — smallest, zero risk, pure addition.
2. **Idle dimming** — self-contained, one feature-flag gate.
3. **Per-layer RGB effects** — highest value but touches `layer_state_set_user` + effect
   persistence; do last, after the other two are settled.
