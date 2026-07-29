# Keychron Q2 ISO Encoder — keychron-v2 features

## Quick reference

| What | How |
|---|---|
| **Toggle Auto-Shift** | `Z + X` |
| **Toggle Tap Dance** | `← + →` (Left + Right arrows) |
| **Toggle Caps Word** | `C + V` |
| **Toggle Repeat Key** | `R + T` |
| **Toggle Dyn. Macro** | `D + F` |
| **Toggle Leader Key** | `L + ;` |
| **Toggle NKRO** | `Space + Right Shift` |
| **Feature overview** | `O + P` — shows status LEDs for 2s |
| **Leader sequences** | `FN2 + Q` then one key (e.g. `W` = close tab) |
| **Configure layers** | VIA app — layers 0-8 |
| **Mac/Win base** | FN1 key adapts per base layer |

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

**FN key behaviour:** `FN1` (right of Space, index 62) activates `MAC_FN1` when `MAC_BASE` is active, or `WIN_FN1` when `WIN_BASE` is active. `FN2` (next to FN1, index 63) activates `_FN2`.

---

## Tap Dance (custom callbacks)

Tap dance lets one key do different things based on single tap vs double tap.
Instead of QMK's built-in `ACTION_TAP_DANCE_DOUBLE`, this keymap uses a
custom `CUSTOM_TD_DOUBLE` macro with its own `td_double_finished` callback.
This ensures any keycode (including `CW_TOGG`, `UC()`, etc.) works reliably.

### Defined tap dances

| Index | Keycode | Name | Single tap | Double tap |
|---|---|---|---|---|
| 0 | `0x5700` | `TD_BSPC_DEL` | Backspace | Delete |
| 1 | `0x5701` | `TD_ESC_CAPS` | Escape | Caps Word toggle |
| 2 | `0x5702` | `TD_E_EURO` | `e` | `€` (U+20AC) |

The first two replace the base layer's `KC_BSPC` and `KC_ESC`. The third
replaces `KC_E` on both Mac and Win base layers — single tap types `e`,
double tap inputs the Euro sign.

Plain `KC_BSPC` and `KC_ESC` are available on layer `_FN3`.

### Adding a new tap dance

Open `keymap.c` and find the `TAP_DANCE_ENABLE` section:

```c
enum {
    TD_BSPC_DEL,
    TD_ESC_CAPS,
    TD_E_EURO,
    // add yours here (no trailing comma needed before ])
};

tap_dance_action_t tap_dance_actions[] = {
    [TD_BSPC_DEL] = CUSTOM_TD_DOUBLE(KC_BSPC,   KC_DEL),
    [TD_ESC_CAPS] = CUSTOM_TD_DOUBLE(KC_ESC,    CW_TOGG),
    [TD_E_EURO]   = CUSTOM_TD_DOUBLE(KC_E,     UC(0x20AC)),
    // add yours here
};
```

Then place `TD(YOUR_ENUM)` at any position in the keymaps.

### Tapping term delay

The single-tap has a ~200ms delay (the `TAPPING_TERM`) because the firmware waits to see if you'll tap again. To reduce it:

```c
// In config.h — lower from default 200ms
#define TAPPING_TERM 150
```

Too low and double-taps may be registered as two single taps.

### Disabling tap dance at runtime

Press `← + →` (Left + Right arrows) simultaneously to toggle tap dance on/off. When off, the TD keycodes act as plain Backspace and Escape. The feature flag persists in EEPROM across power cycles.

Use the feature overview (`O + P`) to check current state:
- T key cyan = tap dance ON
- T key dim = tap dance OFF

---

## Combos

Combos fire when two keys are pressed **simultaneously** within the same 50ms window.

### Active combos

| Combo | Keys | Action |
|---|---|---|
| `CB_TOG_AUTOSHIFT` | `Z + X` | Toggle Auto-Shift on/off |
| `CB_TOG_TAP_DANCE` | `← + →` | Toggle Tap Dance on/off |
| `CB_TOG_NKRO` | `Space + Right Shift` | Toggle NKRO on/off |
| `CB_FEAT_OVERVIEW` | `O + P` | Show feature status overview |

### Commented-out combos (in combos.c)

Navigation combos `A+S=ESC`, `J+K=BSPC`, `K+L=DEL` are in the source but commented. They're redundant with the base layer keys and consume flash. Uncomment the PROGMEM arrays and the table entries in `combos.c` to restore them.

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

## Feature Overview

Press `O + P` to see which features are active:

| LED | Key | Color when ON | Color when OFF | Meaning |
|---|---|---|---|---|
| 10/1-8 | Number row | White | Off/red | Active layer number |
| 28 | Caps Lock | White | Red | Caps Lock hardware state |
| 19 | T | White | Red | Tap Dance ON |
| 29 | A | White | Red | Auto-Shift ON |
| 47 | C | White | Red | Caps Word processing ON |
| 18 | R | White | Red | Repeat Key ON |
| 31 | D | White | Red | Dynamic Macro ON |
| 37 | L | White | Red | Leader Key ON |
| 50 | N | White | Red | NKRO ON |

All other LEDs go dark. The display lasts 2 seconds or until the next keypress, then restores the normal RGB effect.

---

## Key Overrides

Key overrides are enabled in firmware but **no overrides are active by default**. A commented example is in `keymap.c` for `Shift + KC_NUBS → Tilde` (the ISO key left of Z).

To activate:
1. Uncomment the override instance in the `KEY_OVERRIDE_ENABLE` section of `keymap.c`
2. Uncomment the pointer in the `key_overrides[]` array
3. Rebuild and flash

---

## Caps Word

Caps Word acts like Caps Lock but **auto-disables after a non-alpha key**
(space, enter, punctuation, etc.). Great for typing `SOME_VAR` or `CONSTANT`
without accidentally leaving caps on.

| Method | How |
|---|---|
| Toggle on/off | `C + V` combo or assign `KC_CAPS_WORD_TOGGLE` in VIA |
| Activate caps word | `CW_TOGG` keycode (or double-tap Esc via tap dance) |

When Caps Word processing is OFF, the `CW_TOGG` keycode does nothing.
The C key in the feature overview shows white when ON, red when OFF.

## Repeat Key

After tapping any key, `QK_REP` repeats that key. `QK_ALT_REP` repeats with a
modified behaviour (e.g. after left-arrow, Alt-Repeat sends right-arrow).

| Method | How |
|---|---|
| Toggle on/off | `R + T` combo or assign `KC_REPEAT_KEY_TOGGLE` in VIA |
| Repeat last key | Assign `QK_REP` in VIA |
| Alt-repeat | Assign `QK_ALT_REP` in VIA |

The R key in the feature overview shows white when ON, red when OFF.

## Dynamic Macro

Record and playback keystrokes on the fly — no software needed.

| Method | How |
|---|---|
| Toggle on/off | `D + F` combo or assign `KC_DYN_MACRO_TOGGLE` in VIA |
| Record/play slot 1 | Assign `QK_DYNAMIC_MACRO_1` in VIA |
| Record/play slot 2 | Assign `QK_DYNAMIC_MACRO_2` in VIA |

**Usage:** press `QK_DYNAMIC_MACRO_1` → LEDs flash → type your macro →
press the same key again → recording stops. Press it again to play back.
The D key in the feature overview shows white when ON, red when OFF.

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
| 6-7 | Reserved | OFF |

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
