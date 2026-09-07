# Port divergences & memory ledger — Keychron Q2 ISO Encoder

This is the master map of **what this firmware is, where every piece lives, and how
it diverges from the three source trees it draws from**. Read this before touching
anything; it tells you whether a file is safe to overwrite from upstream/vendor or
is intentionally custom.

---

## 1. What this repository is

`/home/franc/qmk_firmware` (branch `keychron-port-v2`) is a **drop-in port** of the
Keychron vendor fork onto upstream QMK, plus a custom keymap `keychron-v2`.

The design goal: keep the vendor code byte-identical wherever possible, isolate every
extension into `keyboards/keychron/common/` (so upstream rebases stay near-zero), and
put **all** keymap-specific behavior under `keymaps/keychron-v2/` so it survives
keyboard/vendor updates untouched.

## 2. Source trees

| Tree | Path | Branch / HEAD | Role |
|---|---|---|---|
| **This port** | `/home/franc/qmk_firmware` | `keychron-port-v2` @ `479f4ddfb4` | Upstream QMK + Keychron port + custom keymap |
| **Vendor fork** | `/home/franc/qmk_firmware_keychron` | `2025q3` @ `ee7390c3bb` | Original Keychron firmware (source of truth for `keyboards/keychron/common/`) |
| **Upstream QMK** | `/home/franc/qmk_firmware_original` | `master` @ `3f26a9232a` | Stock QMK (source of truth for `quantum/`, `drivers/`, etc.) |

`origin` = `qmk/qmk_firmware` (upstream), `fork` = `fram446742/qmk_firmware_q2_iso_encoder`.
The port tracks `origin/master` with 0 divergence on core; the Keychron additions sit on top.

## 3. Where each thing lives

```
keyboards/keychron/q2/iso_encoder/keymaps/keychron-v2/   ← ALL keymap behavior (custom)
keyboards/keychron/q2/iso_encoder/                       ← keyboard data (mostly vendor)
keyboards/keychron/q2/                                   ← shared Q2 config (1 file diverges)
keyboards/keychron/common/                               ← Keychron vendor code (port additions + 14 edits)
drivers/led/snled27351.c/.h                              ← LED driver (modified — core divergence)
quantum/rgb_matrix/rgb_matrix.c                          ← RGB core (modified — core divergence)
```

## 4. Divergence inventory

### 4.1 Custom keymap — `keymaps/keychron-v2/` (all custom)

| File | Role |
|---|---|
| `keymap.c` | Layers, encoder map, and the wiring file: `pre_process_record_user` / `matrix_scan_user` / `rgb_matrix_indicators_advanced_user` dispatch to the per-feature modules |
| `keymap_config.h` | **Single config**: timings, layers, LED indices, overlay role colors, EEPROM layout, feature defaults |
| `config.h` | VIA layer/macro limits, `WINLOCK_LED_LIST` (Num Lock indicator disabled) — kept here so keyboard `config.h` stays vendor-identical |
| `rules.mk` | Feature enables + `SRC +=` + build-time generators |
| `features.h` / `features.c` | Feature-flag API, EEPROM config (tap/combos/leaders), tap-dance state machine, position combos, Raw-HID config protocol |
| `indicators.h` / `indicators.c` | Normal-state caps-lock LED (pwm path) + shared layer↔LED map (`layer_to_led`) |
| `feature_overview.h` / `feature_overview.c` | Feature-overview screen: state, `O + [` entry chord by physical position, modal dispatch, grid draw, idle timeout |
| `layer_picker.h` / `layer_picker.c` | "Layer mode" — knob long-press picker (layers-only modal), own dispatch + draw |
| `layer_visualizer.h` / `layer_visualizer.c` | Key-category overlay (timer + momentary MO modes, lock) + overlay flush manager (`layer_visualizer_frame`) |
| `combos.c` | QMK-native keycode combos (`key_combos[]`, `COMBO_ENABLE`) — reserved-key check vs the overview chord |
| `autocorrect_data.h` | Auto-generated typo trie (from `typos.txt`) |
| `key_positions.h` | Auto-generated `POS_KC_*`/`POS_IDX_*` + `led_to_mtx[]` |
| `typos.txt` | Autocorrect dictionary (source for the trie) |
| `gen_key_positions.py` | Generator for `key_positions.h` |
| `qmk_config_tool.py` | Export/import/dump feature config over USB Raw HID |

### 4.2 Keyboard-level — `keyboards/keychron/q2/`

| File | Status vs vendor | What differs |
|---|---|---|
| `q2.c` | **identical** | — |
| `iso_encoder/config.h` | **identical** | — (has `CAPS_LOCK_INDEX 28`, `ENCODER_DEFAULT_POS 0x3`) |
| `iso_encoder/iso_encoder.c` | **identical** | — (LED matrix `g_snled27351_leds[]`) |
| `config.h` | **diverges** | Keychron-specific defines wrapped in `#ifdef KEYCHRON_ENABLE` (so the keyboard builds without the Keychron common layer); adds `ENCODER_DEFAULT_POS 0x3` |
| `iso_encoder/keyboard.json` | **diverges** | `device_version` 1.0.0 vs vendor 1.1.0; adds data-driven `"enabled"`/`"flags"` RGB fields |
| `iso_encoder/rules.mk` | **diverges (cosmetic)** | trailing newline only — content identical |

### 4.3 Keychron common — `keyboards/keychron/common/`

Everything not listed here is **byte-identical to the vendor fork** — safe to re-sync.

**Intentionally modified (14 files — do NOT overwrite from vendor):**

| File | Why it diverges |
|---|---|
| `keychron_common.mk` | Gates feature modules; `KEYCHRON_ENABLE` wiring |
| `keychron_raw_hid.c` / `.h` | Port's 2-arg `src`-less wired-only variant (vendor's is 3-arg wireless-aware) |
| `rgb/keychron_rgb.c` | RGB-region fixes: `rgb_regions` zeroing, `regions[]` default detection, `os_state_indicate` caps-lock draw |
| `rgb/mixed_rgb.c` | Region seeding into `rgb_regions[]` |
| `rgb/per_key_rgb.c` | Per-key RGB region handling |
| `rgb/rgb.mk` | Effect/animation include set |
| `state_notify.c` / `.h` | Layer/RGB state reporting to Launcher |
| `wireless/wireless.c` / `.h` | Wired-only stubs (vendor has real wireless) |
| `debounce/keychron_debounce.c` / `.h` | Debounce dispatch + EEPROM config |
| `factory_test.c` | Factory-test stub wiring |

**Port additions (exist here, absent from vendor):**

| Path | What |
|---|---|
| `rgb/animations/*.h` | The RGB effect set (Keychron-modified upstream effects, swapped in via `rgb_matrix_effects.inc`) |
| `rgb/rgb_matrix_extensions.c` / `.h` | `rgb_matrix_region_set_color()` and region helpers |
| `vial/*` | Vial (not used by this keymap — see "Ideas") |

### 4.4 Core — upstream QMK files modified (rebase-visible)

| File | Change | Why |
|---|---|---|
| `quantum/rgb_matrix/rgb_matrix.c` | "ghost traces" clear gated to `init && iter == 0`; `memset(rgb_regions,0)` on mode change | Framebuffer effects erased by per-frame clear; stale RGB-region zones |
| `drivers/led/snled27351.c` / `.h` | `snled27351_flush_override` + `snled27351_overlay_dirty` + `snled27351_force_flush` | overlay (layer-viz + overview) renders to a side buffer with dirty tracking; driver flushes it without clobbering the effect's per-key state |

These are the only two core divergences. Track them on every `git rebase upstream/master`.

---

## 5. Memory & flash ledger

MCU: **STM32L432KC** — **256 KB flash** (ROM), **64 KB SRAM** (RAM).

**Totals (measured from `.elf`):**

| Resource | Used | Total | Headroom |
|---|---|---|---|
| Flash (ROM) | **63.1 KB** | 256 KB | ~75% free |
| RAM — static globals (`.data`+`.bss`) | **15.6 KB** | 64 KB | ~48 KB |
| RAM — ChibiOS heap (`.heap`, reserved) | **45.4 KB** | — | mostly free at runtime |
| RAM — stacks (`.mstack`+`.pstack`) | **3 KB** | — | — |

> **Neither is at capacity.** Static globals use only 15.6 KB of the 64 KB SRAM;
> the remaining ~45 KB is the ChibiOS heap, which is reserved but largely unused at
> runtime. Flash has ~75% free. The lever for *more* static RAM is shrinking the heap
> (`CH_CFG_MEMCORE_SIZE`), not the keymap — the keymap itself adds under 1 KB.

### Per-feature RAM (SRAM) — exact, measured from symbols

| Feature / owner | RAM | Notes |
|---|---|---|
| EEPROM emulation (`wear_leveling`) | **8200 B** | QMK platform cost — the embedded-flash EEPROM cache, not a keymap feature |
| VIA macro buffer | **1280 B** | `DYNAMIC_KEYMAP_MACRO_COUNT 16` |
| snled27351 `driver_buffers` | **436 B** | 2 drivers × pwm(192)+led_control(24)+2 flags |
| USB endpoint buffers | **432 B** | platform |
| Layer visualization — `overlay_pwm` | **384 B** | `[2][192]` side buffer (the double-buffer fix) |
| Tap dance — `eeprom_tap[20]` | **200 B** | `MAX_TAP_OVERRIDES 20` |
| Leader — `eeprom_leaders[16]` | **96 B** | `MAX_LEADERS 16` |
| Combos — `eeprom_combos[8]` | **80 B** | `MAX_COMBOS 8` |
| Layer viz — `mo_positions[8]` | **16 B** | held-MO tracking |
| Feature flags + EEPROM counts | **4 B** | `g_feature_flags` + 3 count bytes |
| **Keymap custom total** | **≈ 780 B** | everything above minus platform rows |

The keymap's *own* footprint is under 1 KB of RAM. The 8.2 KB EEPROM cache and the VIA
macro buffer dominate; reducing RAM means attacking those, not the keymap.

### Per-feature flash (ROM) — exact for these symbols (LTO merges the rest)

| Item | Flash |
|---|---|
| `keymaps[]` (9 layers) | **1350 B** |
| `autocorrect_data` (66-entry trie) | **1227 B** |
| `process_record_kb` (Q2 hook) | **2556 B** |
| `mixed_rgb` (custom) | **840 B** |
| `per_key_rgb` (custom) | **668 B** |
| `DIGITAL_RAIN` | **468 B** |
| `TYPING_HEATMAP` | **396 B** |
| `draw_layer` (layer viz) | **332 B** |
| `led_to_mtx[]` (layer viz) | **134 B** |
| `cat_colors[]` (layer viz) | **36 B** |
| `pos_combos[]` | **12 B** |

> With `LTO_ENABLE = yes`, individual `.c` files compile to GIMPLE, so per-module
> `.text` can't be measured directly — the figures above are post-link function
> sizes. The per-feature *RAM* figures are exact; per-feature *flash* is
> representative. If you need a true per-module flash breakdown, build once with
> `LTO_ENABLE = no`.
