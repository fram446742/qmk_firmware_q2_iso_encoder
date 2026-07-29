# Keychron Q2 ISO Encoder — keychron-v2 features

## Quick reference

| What | How |
|---|---|
| **Feature overview** | `O + P` — enter overview mode (10s timeout) |
| **Toggle features** | Inside overview: tap indicator key (A=AutoShift, T=TapDance, etc.) |
| **Toggle layers** | Inside overview: tap number key (0-9) = `TG(N)` |
| **Exit overview** | Tap any non-indicator key or wait 10s |
| **Leader sequences** | `FN2 + Q` then one key (e.g. `W` = close tab) |
| **Configure layers** | VIA app — layers 0-8 |
| **Mac/Win base** | FN1 key adapts per base layer |
| **Auto-correct** | `S` key in overview to toggle; built-in 66-entry dictionary |

---

## Layers

| Index | Name | Purpose |
|---|---|---|
| 0 | `MAC_BASE` | macOS base layout |
| 1 | `WIN_BASE` | Windows base layout |
| 2 | `MAC_FN1` | FN layer for Mac (media, lighting controls) |
| 3 | `WIN_FN1` | FN layer for Windows |
| 4 | `_FN2` | F-keys, lighting, Leader key |
| 5 | `_FN3` | Blank — configure in VIA. Has plain Esc/Backspace fallbacks. |
| 6 | `_FN4` | Blank — configure in VIA |
| 7 | `_FN5` | Blank — configure in VIA |
| 8 | `_FN6` | Blank — configure in VIA |

The layer indicator in overview mode uses `layer_state` (not `default_layer_state`).
This means `TG(N)` toggles in overview immediately light the matching number key.

**FN key behaviour:** `FN1` (right of Space, index 62) activates `MAC_FN1` when `MAC_BASE` is active, or `WIN_FN1` when `WIN_FN1` is active. `FN2` (next to FN1, index 63) activates `_FN2`.

---

## Tap Dance (transparent override)

Tap dance behavior is provided by a **custom override system** that intercepts
base-layer keycodes before QMK processes them. No `TD()` codes are needed in
the keymap — all keycodes stay plain (e.g. `KC_BSPC`, `KC_ESC`, `KC_E`).

The override table is in `keymap.c` under the `tap_overrides[]` array.

### Current overrides

| TD idx | Base key | Single tap | Double tap | Type |
|---|---|---|---|---|
| 0x5700 | `KC_BSPC` | Backspace | Delete | keycode |
| 0x5701 | `KC_ESC` | Escape | Caps Word toggle | keycode |
| 0x5702 | `KC_E` | `e` | `€` | Unicode string |
| 0x5703 | `KC_2` | `2` | `@` | Unicode string |
| 0x5704 | `KC_3` | `3` | `#` | Unicode string |
| 0x5705 | `KC_5` | `5` | `½` | Unicode string |
| 0x5706 | `KC_6` | `6` | `¬` | Unicode string |
| 0x5707 | `` ` `` | `` ` `` | `~` | Unicode string |
| 0x5708 | `KC_BSLS` | `\` | `|` | Unicode string |
| 0x5709 | `[` | `[` | `{` | Unicode string |
| 0x570A | `]` | `]` | `}` | Unicode string |
| 0x570B | `KC_NUBS` | `\`(ISO) | `\` | Unicode string |

### How it works

Instead of QMK's TAP_DANCE_ENABLE (which is disabled), a custom timer-based
state machine in `process_record_user()` intercepts the base keycodes:
1. First tap: starts a 200ms timer, consumes the keypress
2. Second tap within timeout: fires the double-tap action, resets
3. Timeout expires: fires the single-tap action
4. Different key pressed while pending: fires the single-tap immediately

### Adding/changing overrides

Edit the `tap_overrides[]` array in `keymap.c`. Each entry has a type:
- `TD_DBL_KEYCODE` — double-tap sends a QMK keycode (e.g. `KC_DEL`)
- `TD_DBL_UNICODE_STR` — double-tap sends a Unicode string (e.g. `"€"`)
- `TD_DBL_UNICODE_CP` — double-tap sends a raw codepoint (e.g. `0x20AC`)

A compile-time duplicate-check enum prevents adding two overrides for the
same base key.

### Disabling tap dance at runtime

Press **T** in the overview (`O + P`) to toggle. When OFF, all tapped keys
behave as their base keycode (no double-tap). The feature flag persists in
EEPROM.

### Tapping term

The 200ms window (`TAP_TERM` in keymap.c) can be adjusted. Lower values
feel more responsive but make double-taps harder to trigger.

---

## Combos

Combos fire when two keys are pressed **simultaneously** within the same 50ms window.

### Active combo

| Combo | Keys | Action |
|---|---|---|
| `CB_FEAT_OVERVIEW` | `O + P` | Enter feature overview mode |

### Commented-out combos (in combos.c)

All feature-toggle combos (`Z+X` for AutoShift, `←+→` for Tap Dance, etc.) and
navigation combos (`A+S=ESC`, `J+K=BSPC`, `K+L=DEL`) are commented out. Toggles
are now done inside the overview mode (see below). Source preserved for reference.

---

## Auto-Shift

Long-press any alpha or number key to get its shifted variant. For example, hold `A` → `a` initially, then `A` after the auto-shift timeout.

| Toggle | Result |
|---|---|
| `Z + X` | Enable/disable |
| A key in overview | Green = ON, dim = OFF |

State is saved to EEPROM. Default: OFF on first boot.

The auto-shift timeout is controlled by `AUTO_SHIFT_TIMEOUT` (default 150ms).

---

## NKRO (N-Key Rollover)

NKRO lets you press any number of keys simultaneously (vs 6KRO which is limited to 6).

| Toggle | Result |
|---|---|
| `Space + Right Shift` | Enable/disable |
| N key in overview | White = ON, dim = OFF |

State is saved by QMK core via `keymap_config.nkro`.

---

## Leader Key

Press `FN2 + Q` to start a leader sequence. Then press a single key within the LEADER_TIMEOUT (default 300ms) to trigger an action.

All sequences are **platform-aware**: Mac layers (`MAC_BASE`/`MAC_FN1`) send `Cmd+{key}`, Windows layers send `Ctrl+{key}`.

| Sequence | Action |
|---|---|
| `LEAD + W` | Close tab/window |
| `LEAD + Q` | Quit app |
| `LEAD + S` | Save |
| `LEAD + F` | Find |
| `LEAD + A` | Select all |
| `LEAD + C` | Copy |
| `LEAD + V` | Paste |
| `LEAD + X` | Cut |
| `LEAD + Z` | Undo |
| `LEAD + T` | New tab |

The Leader key (`QK_LEAD`) is at `_FN2` position row 1 col 0 (was `UG_TOGG` in the original keymap). `UG_TOGG` is still available on `MAC_FN1` and `WIN_FN1`.

---

## Feature Overview (interactive mode)

Press `O + P` to enter overview mode. All LEDs go dark, then indicator keys
light up: **white** = feature ON, **red** = feature OFF. Number keys show the
active layer.

While in overview mode (10s timeout, **resets on each keypress**), the keyboard
becomes a control panel using **physical key positions** (not keycodes):

| Press | Action |
|---|---|
| **A** | Toggle Auto-Shift |
| **S** | Toggle Auto-Correct |
| **T** | Toggle Tap Dance |
| **C** | Toggle Caps Word processing |
| **R** | Toggle Repeat Key processing |
| **D** | Toggle Dynamic Macro processing |
| **L** | Toggle Leader Key processing |
| **N** | Toggle NKRO |
| **0-9** | `TG(N)` — toggle layer N on/off |
| **any other key** | Exit overview |

Since matching is by physical position (matrix row/col), number keys work even
on function layers that don't have a number row in their keymap.

### Indicator LED map

| LED | Key | ON color | OFF color | Means |
|---|---|---|---|---|
| 10/1-8 | Number row | White | Red | Active layer (highest in layer_state) |
| 28 | Caps Lock | White | Red | Caps Lock hardware state |
| 19 | T | White | Red | Tap Dance processing ON |
| 29 | A | White | Red | Auto-Shift ON |
| 30 | **S** | White | Red | Auto-Correct ON |
| 47 | C | White | Red | Caps Word processing ON |
| 18 | R | White | Red | Repeat Key ON |
| 31 | D | White | Red | Dynamic Macro ON |
| 37 | L | White | Red | Leader Key ON |
| 50 | N | White | Red | NKRO ON |

---

## Key Overrides

Key overrides are enabled in firmware but **no overrides are active by default**. A commented example is in `keymap.c` for `Shift + KC_NUBS → Tilde` (the ISO key left of Z).

To activate:
1. Uncomment the override instance in the `KEY_OVERRIDE_ENABLE` section of `keymap.c`
2. Uncomment the pointer in the `key_overrides[]` array
3. Rebuild and flash

---

## Auto-Correct

Auto-correct detects common typos and replaces them as you type. The dictionary
is a 66-entry trie compiled into flash (not editable at runtime).

| Method | How |
|---|---|
| Toggle on/off | Press **S** in overview |
| State | EEPROM-backed via `keymap_config.autocorrect_enable` |
| Add corrections | Edit `typos.txt`, run `qmk generate-autocorrect-data`, reflash |

The S key in the feature overview shows white when ON, red when OFF.

### Customizing the dictionary

Edit `typos.txt` in the keymap directory, then:
```sh
qmk generate-autocorrect-data keyboards/keychron/q2/iso_encoder/keymaps/keychron-v2/typos.txt -o keyboards/keychron/q2/iso_encoder/keymaps/keychron-v2/autocorrect_data.h
make keychron/q2/iso_encoder:keychron-v2:flash
```

Format: `:typo -> correction` (one per line, colon marks word boundary).

## Caps Word

Caps Word acts like Caps Lock but **auto-disables after a non-alpha key**
(space, enter, punctuation, etc.).

| Method | How |
|---|---|
| Toggle processing | Press **C** in overview |
| Activate caps word | `CW_TOGG` keycode (or double-tap Esc via tap dance) |

When Caps Word processing is OFF, `CW_TOGG` does nothing.
The C key in the feature overview shows white when ON, red when OFF.

## Repeat Key

After tapping any key, `QK_REP` repeats it. `QK_ALT_REP` repeats with a
modified behaviour (e.g. after left-arrow, Alt-Repeat sends right-arrow).

| Method | How |
|---|---|
| Toggle processing | Press **R** in overview |
| Repeat last key | Assign `QK_REP` in VIA |
| Alt-repeat | Assign `QK_ALT_REP` in VIA |

The R key in the feature overview shows white when ON, red when OFF.

## Dynamic Macro

Record and playback keystrokes on the fly — no software needed.

| Method | How |
|---|---|
| Toggle processing | Press **D** in overview |
| Record/play slot 1 | Assign `QK_DYNAMIC_MACRO_1` in VIA |
| Record/play slot 2 | Assign `QK_DYNAMIC_MACRO_2` in VIA |

Press `QK_DYNAMIC_MACRO_1` → LEDs flash → type your macro →
press again → stops. Press to play back.
The D key shows white when ON, red when OFF in overview.

## Layer Lock

Assign `QK_LAYER_LOCK` in VIA to lock the current layer on/off. Press again
to unlock. Useful for locking into a function layer without holding FN.
No runtime toggle — always available.

## Unicode

Assign `UC(0xNNNN)` in VIA to input any Unicode character:
- `UC(0x20AC)` → €
- `UC(0x03A9)` → Ω
- `UC(0x00E9)` → é
- `UC(0x1F600)` → 😀

The input method depends on your OS (Linux IBus, Windows Alt-code, Mac
Unicode Hex Input). Unicode processing is always enabled (no toggle).

---

## EEPROM layout

With 8KB logical EEPROM:

| Offset | Size | Content |
|---|---|---|
| 0-35 | 36B | QMK core config |
| 36-39 | 4B | VIA layout options |
| 40-1389 | 1350B | Dynamic keymaps (9 layers × 5×15 × 2B) |
| 1390-1425 | 36B | Encoder map (9 layers × 1 × 2 × 2B) |
| 1426-8099 | ~6674B | VIA macro buffer |
| **8100** | **1B** | **Feature flags (our custom storage)** |
| 8101-8191 | 90B | Unused |

**Feature flags byte** (address 8100):
| Bit | Feature | Default |
|---|---|---|
| 0 | Tap Dance | ON (1) |
| 1 | Auto-Shift | OFF (0) |
| 2 | Caps Word | ON (1) |
| 3 | Repeat Key | ON (1) |
| 4 | Dynamic Macro | ON (1) |
| 5 | Leader Key | ON (1) |
| 6 | Auto-correct | ON (1) |
| 7 | Reserved | OFF |

First boot (EEPROM = 0xFF) writes `0x00` — all features OFF initially.
Tap-dance / auto-shift / NKRO functions are still available but the
processing flags are OFF until toggled on.

**Changing the EEPROM layout** (e.g. layer count, logical size) requires
clearing EEPROM after flashing — hold reset + power cycle, or use
QMK Toolbox's "Clear EEPROM" button.

---

## VIA keycode names

Tap dance codes appear as `0x5700` / `0x5701` and custom feature codes as `0x5F0C` etc. in VIA / Keychron Launcher. This is a VIA protocol limitation — the app has no way to name dynamic keycodes. The keys work correctly.

---

## Bitwise vs bitfield

Feature flags use explicit `uint8_t` with bitwise operators, not a C bitfield struct. Both compile to identical machine code. The explicit style is preferred because:

| Concern | Bitfield struct | `uint8_t` bitwise |
|---|---|---|
| EEPROM read/write | Need union cast | `eeprom_read_byte()` direct |
| Toggle a bit | `flags.a ^= 1` (RMW) | `flags ^= FEATURE_X` (one XOR) |
| Set/clear | `flags.a = 1` / `0` | `flags \|= FEATURE_X` / `&= ~FEATURE_X` |
| Batch check | Awkward (per-field) | `flags & (A\|B)` clean |
| Padding / layout | Implementation-defined | None |
| Compiler output | Identical | Identical |

For this use case (single EEPROM byte with bit-per-feature), explicit bitwise is simpler and more portable.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Backspace feels slow | Tap dance tapping term delay | Lower `TAPPING_TERM` (e.g. 150) in config.h |
| Combos not firing | Keys pressed sequentially, not simultaneously | Press both keys at the exact same time |
| Feature overview blank | RGB matrix not enabled | Enable any effect in VIA's lighting tab |
| `kc_effect_*` duplicate errors | Modifying quantum/rgb_matrix | Don't — those are in the Keychron port layer |
| Tap dance disable hangs | In-progress tap not reset | Toggle again to re-enable, let it complete, then disable |
| Build error: `keymap_introspection` | Missing `tap_dance_actions[]` or `key_combos[]` | Check `#include "combos.c"` is present in keymap.c |
