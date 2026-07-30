# Keychron Q2 ISO Encoder — keychron-v2 features

## Quick reference

| What | How |
|---|---|
| **Feature overview** | `O + [` — enter overview mode (10s timeout). Position-based: works on any layer. |
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
| `keymap_config.h` | **Single configuration file** — timings, layers, keycodes, combos, LED indices, EEPROM layout, defaults | included from .c files |
| `key_positions.h` | **Auto-generated** — `POS_KC_xxx` matrix-position macros from MAC_BASE layer | `$(shell)` in rules.mk |
| `features.h/c` | Feature flag API, EEPROM config (tap/combos/leaders), tap-dance state machine | `SRC +=` |
| `indicators.h/c` | RGB indicator drawing, feature overview trigger/state | `SRC +=` |
| `combos.c` | Combo array (keys + actions from `keymap_config.h`) | `#include` from keymap.c |

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
Intercepts keys before QMK processes them. Uses **matrix position** matching
by default (`base_type=1`), so overrides follow the physical key regardless
of what keycode it sends on the current layer.

Default entries are defined in `keymap_config.h` via `TAP_DEFAULTS`. Each
entry targets a physical position using the `POS_KC_xxx` macros from
auto-generated `key_positions.h`:

```c
#define TAP_DEFAULTS \
    {.base_id=POS_KC_BSPC, .tap_kc=KC_BSPC, .base_type=1, ..., .dbl_val=KC_DEL}, \
    {.base_id=POS_KC_E,    .tap_kc=KC_E,    .base_type=1, ..., .dbl_val=RALT(KC_E)},
    {0}  /* sentinel */
```

To override by **keycode** instead (follows the keycode label across layout
changes), set `base_type=0` and use a `KC_xxx` value for `base_id`.

Editable at runtime via `qmk_config_tool.py`. The T key in overview mode
toggles the entire feature.

### OS Unicode requirement

€, @, and ~ double-taps use `register_unicode()`. This requires OS-level
Unicode input to be configured:
- **Windows**: Set `HKCU\Control Panel\Input Method\EnableHexNumpad` = 1, reboot, then Alt+`+`+20AC
- **Linux**: IBus must be running (Ctrl+Shift+U then 20AC)
- **macOS**: Enable Unicode Hex Input in keyboard settings, hold Option+20AC

Without this, `register_unicode()` sends Alt/Ctrl chords interpreted as
shortcuts (Alt+Tab, etc.).

## Combos

| Combo | Keys | Action | System |
|---|---|---|---|
| `CB_FEAT_OVERVIEW` | `O + [` (keycode) | Enter overview | QMK native (key_combos[]) |
| `CB_FEAT_OVERVIEW` | `O + [` (position) | Enter overview | Custom processor (pos_combos[]) |

### Two combo systems

The keymap runs **two independent combo processors**:

#### 1. QMK-native (`key_combos[]` in `combos.c`)
Processed by QMK's built-in `process_combo()`.  Keys in the `keys[]` array
must be **`KC_xxx` keycodes** — QMK compares against the keycode value on
the current layer.  If the keycode doesn't exist on the active layer (e.g.
`KC_O` on a layer where that position is `KC_F10`), the combo won't fire.

```c
// combos.c — uses keycodes (KC_O, KC_LBRC, not POS_KC_O/POS_KC_LBRC)
const uint16_t PROGMEM cb_feat_overview[] = {KC_O, KC_LBRC, COMBO_END};
combo_t key_combos[] = {
    [CB_FEAT_OVERVIEW] = COMBO(cb_feat_overview, KC_FEAT_OVERVIEW),
};
```

#### 2. Position-based (`pos_combos[]` in `features.c`)
Custom processor that matches by **physical matrix position** using the
`POS_KC_xxx` macros from `key_positions.h`.  Works on **any layer** — it
checks which physical key was pressed, not which keycode it sends.

```c
// keymap_config.h — uses matrix positions (POS_KC_O, POS_KC_LBRC)
#define POS_COMBOS_DEFS \
    POS_COMBO(2, BASE_IS_MATRIX, KC_FEAT_OVERVIEW, POS_KC_O, POS_KC_LBRC),
```

Position combos are processed **in `process_record_user()` before** QMK's
native `process_combo()` runs.  If the positions match, the keys are
consumed and don't reach QMK's combo system.  This means:
- On layers where O+[ have the same physical keys (all base/FN layers):
  position combo fires, QMK combo is irrelevant (keys already consumed)
- On layers where those positions send different keycodes: position combo
  still fires, QMK combo would also fire if it matched those keycodes
  (but it won't, since the keys were already consumed)

### Adding a new combo

Both systems support up to 4 keys per combo.  To add a new position-based
combo in `keymap_config.h`:

```c
// 1. Define combo via inline keys in the macro
#define POS_COMBOS_DEFS \
    POS_COMBO(2, BASE_IS_MATRIX, KC_FEAT_OVERVIEW, POS_KC_O, POS_KC_LBRC),    \
    POS_COMBO(2, BASE_IS_MATRIX, KC_MYACTION,       POS_KC_A, POS_KC_S),
```

For keycode-based position combos (follows the keycode label, not the
physical key), use `BASE_IS_KEYCODE` instead:

```c
#define POS_COMBOS_DEFS \
    POS_COMBO(2, BASE_IS_KEYCODE, KC_FEAT_OVERVIEW, KC_O, KC_LBRC),  \
    POS_COMBO(2, BASE_IS_MATRIX,  KC_MYACTION,      POS_KC_A, POS_KC_S),
```

## Feature Overview (interactive mode)

Press `O + [` simultaneously to enter.  Uses the **position-based combo**
(see above) — works on any layer.  LEDs go dark, indicator keys light up
white=ON, red=OFF.

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
| **0-8** | `layer_move(N)` — same layer returns to default |
| **9** | Toggle **layer-visualization lock** (shows key categories permanently until next layer change) |
| **any other key** | Exit overview |

### Indicator LED map

| LED | Key | Meaning |
|---|---|---|
| 10/1-8 | Number row | Active layer (highest in `layer_state`) |
| 9 | 9 key | Layer-visualization lock ON |
| 29 | A | Auto-Shift ON |
| 30 | S | Auto-Correct ON |
| 19 | T | Tap Dance ON |
| 47 | C | Caps Word processing ON |
| 18 | R | Repeat Key ON |
| 31 | D | Dynamic Macro ON |
| 37 | L | Leader Key ON |
| 50 | N | NKRO ON |

All indicator LEDs: white=active, red=inactive.  The layer-vis lock indicator
shows white when the overview is shown in locked mode.  When locked, the
color overlay stays on indefinitely until the next layer change — the timer
never expires.

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

| Address | Size | Content |
|---|---|---|
| `EEP_FEATURES` (8100) | 1B | Feature flags (bitmask, see above) |
| `EEP_TAP_BASE` | 1B | Tap override count |
| `EEP_TAP_BASE + 1` | `EEP_TAP_SIZE` | Tap override entries |
| `EEP_COMBO_BASE` | 1B | Combo count |
| `EEP_COMBO_BASE + 1` | `EEP_COMBO_SIZE` | Combo entries |
| `EEP_LEADER_BASE` | 1B | Leader count |
| `EEP_LEADER_BASE + 1` | `EEP_LEADER_SIZE` | Leader entries |

All addresses are derived from `EEP_FEATURES` (8100). Change `MAX_TAP_OVERRIDES`,
`MAX_COMBOS`, `MAX_LEADERS`, or any `eeprom_*` struct and everything adjusts
automatically. See `keymap_config.h` for the exact calculations.

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
| Combos not firing | Keys pressed sequentially, or using POS_KC_xxx in QMK-native combo | Press O and [ at the exact same time. QMK-native combos require KC_xxx keycodes (not POS_KC_xxx). The keymap's position-based combo (features_combo_process) uses POS_KC_xxx and works on any layer — that's the recommended path for layer-independent combos. |
| Overview blank | RGB matrix mode disabled | Enable any effect in VIA's lighting tab |
| Debug shows all zeros | EEPROM not initialized | Reflash, first-boot init runs when byte 8100 is 0xFF/0x00 |
| Feature flag won't save | EEPROM address collision | Default 8100 is past VIA macro buffer; verify no other code uses it |
