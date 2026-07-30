# Keychron Q2 ISO Encoder — keychron-v2 features

## Quick reference

| What | How |
|---|---|
| **Feature overview** | `O + P` — enter overview mode (10s timeout) |
| **Toggle features** | Inside overview: tap key (A=AutoShift, T=TapDance, S=AutoCorrect, C=CapsWord, R=RepeatKey, D=DynMacro, L=Leader, N=NKRO) |
| **Toggle layers** | Inside overview: tap number key (0-9) = `layer_move(N)`; same layer returns to default |
| **Exit overview** | Tap any non-indicator key or wait 10s |
| **Config export/import** | `uv run qmk_config_tool.py export <file>` / `import <file>` |
| **Leader sequences** | `FN2 + Q` then one key (`W`=close tab, `Q`=quit, etc.) |
| **Auto-correct** | Edit `typos.txt`, rebuild — trie auto-generated |
| **Build** | `PATH=...:$PATH make keychron/q2/iso_encoder:keychron-v2` |

## Module organization

| File | Role | Compiled |
|---|---|---|
| `keymap.c` | Layers, encoder map, callbacks (`process_record_user`, `matrix_scan_user`), HID handler | main entry |
| `features.h/c` | Feature flag API, EEPROM config (tap/combos/leaders), tap-dance state machine | `SRC +=` |
| `indicators.h/c` | RGB indicator drawing, feature overview trigger/state | `SRC +=` |
| `combos.h/c` | Custom keycodes, combo definitions (included from keymap.c) | `#include` |

## Layers

| Index | Name | Purpose |
|---|---|---|
| 0 | `MAC_BASE` | macOS base layout |
| 1 | `WIN_BASE` | Windows base layout |
| 2 | `MAC_FN1` | FN layer for Mac (media, lighting controls) |
| 3 | `WIN_FN1` | FN layer for Windows |
| 4 | `_FN2` | F-keys, lighting, Leader key |
| 5-8 | `_FN3` — `_FN6` | Blank — configure in VIA |

**FN key behaviour:** `FN1` (right of Space, index 62) activates `MAC_FN1` or `WIN_FN1` depending on the active base layer. `FN2` (next to FN1, index 63) activates `_FN2`. Physical Mac/Win switch overrides any overview-set layer.

## Tap Dance (transparent override)

Custom timer-based state machine in `features.c` — no QMK `TAP_DANCE_ENABLE`.
Intercepts base keycodes before QMK processes them. All keycodes stay plain
(e.g. `KC_BSPC`, `KC_ESC`, `KC_E`) — VIA shows proper names.

| Key | Single tap | Double tap | Type |
|---|---|---|---|
| `KC_BSPC` | Backspace | `KC_DEL` | keycode |
| `KC_ESC` | Escape | `CW_TOGG` (Caps Word) | keycode |
| `KC_E` | `e` | `€` (U+20AC) | Unicode CP |
| `KC_2` | `2` | `@` | Unicode string |
| `KC_4` | `4` | `~` | Unicode string |

**Defaults are in EEPROM** — loaded from `features_load_defaults()` in
`features.c`. Editable at runtime via `qmk_config_tool.py`. The toggle history
button (T in overview) enables/disables the entire feature.

### OS Unicode requirement

€, @, and ~ double-taps use `register_unicode()`. This requires OS-level
Unicode input to be configured:
- **Windows**: Set `HKCU\Control Panel\Input Method\EnableHexNumpad` = 1, reboot, then Alt+`+`+20AC
- **Linux**: IBus must be running (Ctrl+Shift+U then 20AC)
- **macOS**: Enable Unicode Hex Input in keyboard settings, hold Option+20AC

Without this, `register_unicode()` sends Alt/Ctrl chords interpreted as
shortcuts (Alt+Tab, etc.).

## Combos

| Combo | Keys | Action |
|---|---|---|
| `CB_FEAT_OVERVIEW` | `O + P` | Enter overview |

Combos are defined in `combos.c` (included from `keymap.c`).

## Feature Overview (interactive mode)

Press `O + P` to enter. LEDs go dark, indicator keys light up white=ON,
red=OFF. Uses **physical key positions** (matrix row/col) — works on any layer.

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
| **0-9** | `layer_move(N)` — same layer returns to default |
| **any other key** | Exit overview |

### Indicator LED map

| LED | Key | Meaning |
|---|---|---|
| 10/1-8 | Number row | Active layer (highest in `layer_state`) |
| 28 | Caps Lock | Hardware Caps Lock state |
| 29 | A | Auto-Shift ON |
| 30 | S | Auto-Correct ON |
| 19 | T | Tap Dance ON |
| 47 | C | Caps Word processing ON |
| 18 | R | Repeat Key ON |
| 31 | D | Dynamic Macro ON |
| 37 | L | Leader Key ON |
| 50 | N | NKRO ON |

All indicator LEDs: white=active, red=inactive. Caps Lock uses hardware state
(`host_keyboard_led_state().caps_lock`), not a feature flag.

## Feature flags

EEPROM byte at 8100. Default: `0x0C` (CapsWord + RepeatKey ON, rest OFF).

| Bit | Feature | Default |
|---|---|---|
| 0 | Tap Dance | OFF |
| 1 | Auto-Shift | OFF |
| 2 | Caps Word | **ON** |
| 3 | Repeat Key | **ON** |
| 4 | Dynamic Macro | OFF |
| 5 | Leader Key | OFF |

Runtime `process_record_user` fences: when a feature flag is OFF, its
keycodes (`CW_TOGG`, `QK_REP`, `QK_DYNAMIC_MACRO_*`, `QK_LEAD`) pass
through as no-ops. When ON, they behave normally. Auto-Shift is the only
feature with a direct hardware enable/disable call.

## Leader Key

`FN2 + Q` starts a sequence, then one key within 300ms. Platform-aware:
Mac layers → `Cmd+key`, Windows layers → `Ctrl+key`.

| Seq | Action |
|---|---|
| `W` | Close tab/window |
| `Q` | Quit app |
| `S` | Save |
| `F` | Find |
| `A` | Select all |
| `C` | Copy |
| `V` | Paste |
| `X` | Cut |
| `Z` | Undo |
| `T` | New tab |

## Auto-Correct

66-entry trie compiled from `typos.txt`. Toggle with S in overview (uses
`keymap_config.autocorrect_enable`). Edit `typos.txt`, rebuild.

## EEPROM layout (user data area)

Beyond VIA's dynamic keymap and macro buffer:

| Offset | Size | Content |
|---|---|---|
| **8100** | 1B | Feature flags (bitmask, see above) |
| 8101 | 1B | Tap override count |
| 8102-8301 | 200B | Tap override entries (20 × 10B `eeprom_tap_t`) |
| 8302 | 1B | Combo count |
| 8303-8366 | 64B | Combo entries (8 × 8B `eeprom_combo_t`) |
| 8367 | 1B | Leader count |
| 8368-8463 | 96B | Leader entries (16 × 6B `eeprom_leader_t`) |

First boot detection: if byte 8100 is 0xFF or 0x00, all entries are
initialized with defaults and written to EEPROM.

Changing layout (e.g. layer count) requires wiping EEPROM after flash:
hold reset + power cycle, or use QMK Toolbox's "Clear EEPROM".

## Python config tool

`qmk_config_tool.py` in the keymap directory reads/writes EEPROM-backed
config over USB Raw HID (via `via_custom_value_command_kb`).

```sh
uv run qmk_config_tool.py export config.json   # save config to file
uv run qmk_config_tool.py import config.json   # restore from file
uv run qmk_config_tool.py dump                 # full dump
uv run qmk_config_tool.py debug                # firmware state check
uv run qmk_config_tool.py --mock ...           # test without hardware
```

The `mock` flag tests serialization/deserialization without a keyboard
connected.

## Troubleshooting

| Symptom | Likely cause | Fix |
|---|---|---|
| Tap dance doesn't respond | Feature flag OFF | Toggle T key in overview (O+P) |
| € key sends Alt+Tab | OS Unicode input not configured | See "OS Unicode requirement" above |
| Combos not firing | Keys pressed sequentially | Press O and P at the exact same time |
| Overview blank | RGB matrix mode disabled | Enable any effect in VIA's lighting tab |
| Debug shows all zeros | EEPROM not initialized | Reflash, first-boot init runs when byte 8100 is 0xFF/0x00 |
| Feature flag won't save | EEPROM address collision | Default 8100 is past VIA macro buffer; verify no other code uses it |
