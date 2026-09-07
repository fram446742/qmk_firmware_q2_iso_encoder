# Added functionality — keychron-v2 vs stock Keychron Q2

What this keymap adds on top of the **original Keychron proprietary firmware**.
For *where* each of these lives in the code and *what they cost*, see
[`DIVERGENCES.md`](DIVERGENCES.md).

---

## What stock firmware already does

The vendor Q2 firmware ships with: RGB matrix + the Keychron effect set, VIA/Launcher
dynamic keymap, Mac/Win base layers + FN layers, a caps-lock indicator, and the
proprietary Raw-HID channel. **All of that is preserved.** Everything below is new.

## Added features at a glance

| Feature | Entry point | Default |
|---|---|---|
| **Feature overview** (interactive config screen) | `O + [` combo | — |
| **Layer visualization** (color-coded key categories) | any layer change / hold `MO` | ON |
| **Runtime feature flags** | toggled inside overview | mixed (see flags) |
| **Tap dance** (double-tap = action, transparent override) | config in `keymap_config.h` | OFF |
| **Position-based combos** (works on any layer) | `O + [` fallback | — |
| **Auto-correct** | compile-time trie from `typos.txt` | OFF |
| **Leader key** (platform-aware shortcuts) | `FN2 + Q`, then a key | OFF |
| **Caps Word / Repeat Key / Dynamic Macro / Auto-Shift** | QMK features wired into the flag system | CW+REP ON, rest OFF |
| **NKRO toggle** | overview `N` | — |
| **Caps/Win-Lock indicators** (Launcher-visible) | `WINLOCK_LED_LIST` (Num Lock disabled) | — |
| **Config export/import over Raw HID** | `qmk_config_tool.py` | — |

---

## Feature overview (interactive mode)

`O + [` together enters a dark "config screen" where each feature has an indicator
LED (white = ON, red = OFF). The chord is matched by the native combo (keycode) with
a position-based fallback, and the key *dispatch* inside overview is physical-position
based — so it works, and you can get back out, from any layer, including blank ones.
Rendered into the same overlay buffer as layer visualization, so it never clears the
effect's per-key state.

| Key | Action |
|---|---|
| `A` `S` `T` `C` `R` `D` `L` | Toggle Auto-Shift / Auto-Correct / Tap Dance / Caps Word / Repeat Key / Dynamic Macro / Leader |
| `N` | Toggle NKRO |
| `0–8` | `layer_move(N)` (same layer again → default) |
| `9` | Toggle layer-visualization **lock** |
| Knob rotation | Cycle layers 0–8 (clockwise = next, CCW = previous) |
| Knob press | Exit overview, keep current layer |
| `Esc` | Back to default layer and exit overview |


| `L` | Leader sequences (W=close tab, Q=quit, S=save, F=find, A=select all, C=copy, V=paste, X=cut, Z=undo, R=reload, B=bookmarks, N=new window, G=go to line, H=history, D=duplicate, P=print) |


Shows every key's **category** as a color (modifier, function, media, macro, layer,
etc.) so you can see a layer at a glance. Two modes:

- **Momentary** — hold any `MO` key → overlay follows the live layer; release → it disappears.
- **Timer/permanent** — a non-MO layer change (TO/TG/VIA) shows the overlay for a
  timeout, or indefinitely when **locked** (`9` in overview).

Colors and the key→category map live in `layer_visualizer.c`. Implemented as a
double-buffer flush override (see `DIVERGENCES.md` §4.4) so the RGB effect's own
per-key state is never clobbered — this is what makes it work on lazy effects like
jellybean raindrops without freezing LEDs.

## Runtime feature flags

One EEPROM byte (`EEP_FEATURES` = 8100) holds 7 on/off bits. Default `0x4C`
(Caps Word + Repeat Key + Layer Viz ON). When a feature is OFF, its keycodes are
fenced in `process_record_user` (no-op); ON = normal QMK behavior.

| Bit | Feature |
|---|---|
| 0 | Tap dance |
| 1 | Auto-shift |
| 2 | Caps Word |
| 3 | Repeat Key |
| 4 | Dynamic Macro |
| 5 | Leader |
| 6 | Layer visualization |

## Tap dance (transparent override)

Custom timer-based state machine in `features.c` — **not** QMK `TAP_DANCE_ENABLE`.
Intercepts keys before QMK. Entries match by **keycode** or **matrix position**
(`base_type` 0/1), defined in `keymap_config.h` (`TAP_DEFAULTS`). Shipped default:
`Esc×2` → `RAlt+Esc`, `E×2` → `RAlt+E` (accented vowels).

**Unicode:** `€`/`@`/`~` double-taps use `register_unicode()`, which needs OS Unicode
input enabled (Windows `EnableHexNumpad`, Linux IBus, macOS Unicode Hex Input).
Without it the chords are read as Alt/Ctrl shortcuts.

## Combos (two systems, one chord)

The feature-overview chord `O + [` is wired twice, deliberately separate:

1. **QMK-native** (`combos.c` `key_combos[]`, `COMBO_ENABLE`) — keycode-matched
   (`KC_O` + `KC_LBRC`), processed in `pre_process_record_quantum`. Always on, so
   the overview entry can't be broken by the user combo feature.
2. **Position-based** (`features.c` `pos_combos[]`, `features_combo_process`) —
   matrix-position-matched (`POS_KC_*`); the fallback that fires when `O`/`[` are
   remapped and no longer resolve to `KC_O`/`KC_LBRC`.

## Auto-correct

66-entry trie generated from `typos.txt` at build time (`autocorrect_data.h`).
Toggle with `S` in overview; edit `typos.txt` and rebuild to change.

## Leader key

`FN2 + Q` then one key within 300 ms. Platform-aware: Mac → `Cmd+key`, Win → `Ctrl+key`.
`W`=close, `Q`=quit, `S`=save, `F`=find, `A`=select all, `C`/`V`/`X`/`Z`=copy/paste/cut/undo, `T`=new tab.

## Indicators (Caps / Num / Win Lock)

Three status LEDs drawn through Keychron's proprietary `os_state_indicate()` path, so
they appear as toggles in the **Keychron Launcher**:

| Indicator | Physical key | LED | Launcher toggle |
|---|---|---|---|
| Caps Lock | Caps | 28 | Caps Lock |
| Win Lock | Left Option (Win) | 58 | Win Lock (`no_gui`) |

Enabled by `#define`s in the keymap `config.h` (`WINLOCK_LED_LIST`); caps lock stays
in the keyboard `config.h` matching the vendor. The Num Lock indicator is disabled
(`NUM_LOCK_INDEX` commented out — no numpad on the Q2). Win Lock lights red
only while the Win key is **locked**, not merely while Windows is the active OS mode.

Known vendor bug (not fixed here): `keychron_rgb.c:os_state_indicate()` checks
`.compose` instead of `.scroll_lock` for `SCROLL_LOCK_INDEX` — any future
`SCROLL_LOCK_INDEX` addition must fix that line first.

## Config export/import

`qmk_config_tool.py` reads/writes the EEPROM feature config over USB Raw HID
(`via_custom_value_command_kb`), and parses `keymap_config.h` at import so it can't
drift from the firmware.

```sh
uv run qmk_config_tool.py export config.json   # save
uv run qmk_config_tool.py import config.json   # restore
uv run qmk_config_tool.py dump                 # full dump
uv run qmk_config_tool.py --mock ...           # test without hardware
```

## Layers

| # | Name | Purpose |
|---|---|---|
| 0 | `MAC_BASE` | macOS base |
| 1 | `WIN_BASE` | Windows base |
| 2 | `MAC_FN1` | Mac FN layer |
| 3 | `WIN_FN1` | Win FN layer |
| 4 | `_FN2` | F-keys, lighting, Leader |
| 5–8 | `_FN3`–`_FN6` | blank, VIA-configured |

`FN1` (right of Space) → `MAC_FN1`/`WIN_FN1` per active base; `FN2` → `_FN2`. The
physical Mac/Win switch overrides any overview-set layer.

## EEPROM layout (user area)

All addresses derive from `EEP_FEATURES` (8100); the tap/combo/leader arrays are
sized from `MAX_*` constants so changing a limit re-lays everything out automatically.

| Address | Size | Content |
|---|---|---|
| 8100 | 1 B | Feature flags |
| 8101 | 1 B | Tap count |
| 8102 | `20 × eeprom_tap_t` | Tap entries |
| … | 1 B + `8 × eeprom_combo_t` | Combos |
| … | 1 B + `16 × eeprom_leader_t` | Leaders |

First-boot init: byte 8100 == `0xFF` **or** `0x00` → load defaults (wear-leveling
EEPROM reads erased state as `0x00`, not `0xFF`). Layout changes require an EEPROM wipe.

## Troubleshooting

| Symptom | Fix |
|---|---|
| Tap dance dead | Feature flag OFF — toggle `T` in overview |
| `€` sends Alt+Tab | OS Unicode input not configured |
| Combo won't fire | press both keys simultaneously; QMK-native combos need `KC_*`, position combos use `POS_KC_*` |
| Overview blank | RGB matrix off — enable an effect in VIA |
| Flags won't save | EEPROM collision — verify 8100 is past VIA macro buffer |

### Completed: Encoder-for-layers in overview (2026-09-01)
- Knob rotation cycles layers 0–8; knob press exits overview keeping the current layer; `Esc` resets to the default layer and exits overview
- Number keys 0–8 jump layers directly (`LAYER_MOVE_OR_DEFAULT` — same layer again → default)
- Fixed: layer visualization and feature overview regressions — `layer_visualizer_mark_user_activity()` and the tap-dance fence re-instated in `process_record_user`
### Completed Features (2026-09-01)
- Leader sequences: W=close tab, Q=quit, S=save, F=find, A=select all, C=copy, V=paste, X=cut, Z=undo, T=new tab, R=reload, B=bookmarks, N=new window, G=go to line, H=history, D=duplicate, P=print
- Encoder knob: rotation cycles layers 0-8 during overview; press exits overview keeping the current layer
- Per-layer RGB effects: IN PROGRESS — layer-based RGB mode switching (reverted from this turn; will be re-added after design review)
