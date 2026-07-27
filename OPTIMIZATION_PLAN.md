# Keychron Q2 Firmware — Optimization Analysis & Plan

**Date:** 2026-07-27
**Baseline build:** `keychron/q2/iso_encoder:keychron` (56,964 bytes, working)

---

## 1. Current Baseline

| Metric | Value | Headroom |
|--------|-------|----------|
| Flash (code) | 55 KB | 201 KB free (78%) |
| RAM (BSS + data) | 7.7 KB | 56 KB free (88%) |
| Core files modified (`quantum/`) | 13 files | High upstream-update friction |

STM32L432KC: 256 KB flash, 64 KB RAM (SRAM1 48 KB + SRAM2 16 KB).

---

## 2. Dead Code — Can Remove Safely

These modules are compiled unconditionally but serve no purpose on a wired mechanical Q2:

| Module | Lines | Enabled by | Why Q2 doesn't need it |
|--------|-------|------------|------------------------|
| `factory_test.c` / `.h` | 564 | `FACTORY_TEST_ENABLE` in `keychron_common.mk` | Manufacturing test mode. Launcher command 0xAB never used on production units. |
| `snap_click/` (4 files) | 228 | `SNAP_CLICK_ENABLE` via `snap_click.mk` | Bistable/magnetic-switch support. Q2 is standard mechanical. |
| `retail_demo.c` / `.h` | 188 | `RETAIL_DEMO_ENABLE` via `rgb/rgb.mk` | Retail store demo mode. Not a demo unit. |
| `language/` (3 files) | 67 | `LANGUAGE_ENABLE` via `language.mk` | Multi-language switching. Single-layout user. |
| `usb_report_rate.c` | 202 | `USB_REPORT_INTERVAL_ENABLE` key | Adjustable polling rate (1K/4K/8K Hz). 1K default is sufficient. |
| `debounce/*.c` (7 unused variants) | ~600 | `DEBOUNCE_TYPE=custom` via `debounce.mk` | Only 1 algorithm runs at runtime; 7 others are dead weight. |
| `rgb_matrix/animations/*` (unused effects) | varies | `ENABLE_RGB_MATRIX_*` flags | ~20+ effects compiled; user may only use a handful. |

**Estimated flash savings:** 6–8 KB after removing all above.

---

## 3. Core File Modifications — Update Friction

Files outside `keyboards/keychron/` that were modified. These must be re-applied after every upstream `git pull`.

| File | Change | Why needed | Could eliminate? |
|------|--------|------------|------------------|
| `quantum/rgb_matrix/rgb_matrix_types.h` | +`uint8_t region` field to `effect_params_t` | Required by `mixed_rgb.c` for multi-layer region rendering | No |
| `quantum/rgb_matrix/rgb_matrix.h` | +`rgb_matrix_region_set_color()` + `rgb_matrix_region_set_color_all()` | Declarations for the new region API | No |
| `quantum/rgb_matrix/rgb_matrix.c` | ~30 lines: region functions, `rgb_regions[]`, `rgb_matrix_none_indicators_kb/user()`, `kc_effect_*` wrappers, updated `rgb_effect_params` init | Region support, extern effect wrappers for mixed_rgb | Partially — wrappers could be avoided if mixed_rgb used function pointers |
| `quantum/debounce.h` | signature: `debounce(raw, cooked, changed)` → `debounce(raw, cooked, num_rows, changed)` + `debounce_init(num_rows)` + `debounce_free()` | Keychron custom debounce uses 4-param signature | No — required for DEBOUNCE_TYPE=custom |
| `quantum/matrix.c` | `debounce()` + `debounce_init()` calls updated with `MATRIX_ROWS` | Consequence of debounce.h change | No |
| `quantum/matrix_common.c` | Same as matrix.c | Consequence of debounce.h change | No |
| `quantum/debounce/*.c` (8 files) | All standard debounce functions updated to 4-param signature + stub `debounce_free()` | Required so standard debounce still compiles when DEBOUNCE_TYPE is not custom | No — but only applies when default debounce is used |

**Total: 13 core files modified.** Each upstream `git pull` risks conflicts in all of them.

---

## 4. RAM Usage Breakdown

| Consumer | Bytes | Notes |
|----------|-------|-------|
| `wear_leveling` buffer | 3,080 | Set by `logical_size: 3072` in info.json. Largest single allocation. |
| ChibiOS idle thread | 480 | OS overhead |
| ChibiOS driver buffers | 436 | I2C, DMA, etc. |
| `per_key_led[]` (68 × HSV) | 204 | Keychron per-key RGB LED colors |
| `USBD1` | 136 | USB device driver struct |
| `DMA` buffers | 120 | DMA transfer descriptors |
| ChibiOS thread `ch0` | 96 | Main thread |
| `effect_list[2][5]` | 80 | Custom RGB effect list (8 bytes × 2 layers × 5 effects) |
| `rgb_regions[68]` | 68 | Per-LED region assignment |
| `regions[68]` | 68 | Per-LED region (mixed_rgb) |
| `snap_click_pair` | 60 | Snap-click config (removable) |
| `I2CD1` | 52 | I2C driver for SNLED27351 RGB controller |
| Everything else | ~800 | Small counters, timers, states |

**Total: ~7.7 KB** — 56 KB free (88%). Wear-leveling buffer dominates.

### Wear-leveling sizing

`logical_size: 3072` = EEPROM emulation buffer for VIA + Keychron data. Could try 2048 if actual usage fits:

| Subsystem | Estimated bytes |
|-----------|----------------|
| VIA dynamic keymaps (5 layers × 68 keys × 2 bytes) | 680 |
| VIA layout options | 4 |
| VIA macros | ~256 |
| Keychron language | 1 |
| Keychron debounce | 2 |
| Keychron snap_click | variable |
| Keychron custom RGB (per_key + mixed + effects + os_indicator) | ~1200+ |

Total estimated: ~2,100+ bytes. Could try reducing to 2048 but risks overflow.

---

## 5. CPU / Runtime

- **Matrix scanning**: Standard QMK GPIO matrix, no bottleneck. ~1–5 ms per scan.
- **RGB rendering**: 68 LEDs at 30 FPS via I2C. Main CPU load, but imperceptible on wired.
- **Keychron task**: `keychron_common_task()` runs every `housekeeping_task_kb()`. Checks Siri timeout, OS toggle combo, winlock timer. Negligible cost.
- **State notifications**: `state_notify.c` sends raw HID packets every layer change. ~32 bytes per change, USB full-speed is 64 bytes/ms — no bottleneck.
- **Launcher protocol**: Only processes commands when Launcher sends them. Idle otherwise.

**No meaningful CPU optimization targets on a wired keyboard.** All cycle waste is in the microsecond range.

---

## 6. Proposed Optimization Plan

### Phase A — Dead Code Removal (no functional risk)

1. Remove `FACTORY_TEST_ENABLE` from `keychron_common.mk`
2. Remove `SNAP_CLICK_ENABLE` — delete or guard snap_click includes
3. Remove `RETAIL_DEMO_ENABLE` — delete or guard in `rgb/rgb.mk`
4. Remove `LANGUAGE_ENABLE` — delete or guard `language/` includes
5. Keep `USB_REPORT_INTERVAL_ENABLE` **off** for Q2
6. Trim `debounce/` to 1 algorithm (`sym_eager_pk`), delete unused
7. Trim RGB effects in `info.json` to only those used via Launcher

**Expected flash:** ~48–50 KB (back to near-upstream size)

### Phase B — Maintainability

1. Move feature defines from `keychron_common.mk` into per-keyboard `config.h`
   - Example: `-DFACTORY_TEST_ENABLE` → `#define FACTORY_TEST_ENABLE` in Q2's `config.h`
   - This makes it obvious which features each keyboard enables
2. Reduce `logical_size` from 3072 to 2048 if EEPROM data fits (saves 1 KB RAM)
3. Document all 13 core-file changes as a single rebase-able patch

### Phase C — Verification

1. Build `keychron/q2/iso_encoder:keychron`
2. Build `keychron/q2/iso_encoder:via`
3. Build `keychron/q2/iso_encoder:default`
4. Flash and verify: typing works, Launcher connects, VIA connects

---

## 7. Future Concerns

- **VIA protocol version mismatch**: Upstream `via.h` reports `VIA_PROTOCOL_VERSION 0x000D`, Keychron fork uses `0x000C`. If the Launcher checks this, it might reject the firmware. Not observed yet.
- **Keycode range overlap**: Q2's `keycodes_custom.h` uses `QK_KB_2`–`QK_KB_10`. The common `keychron_common.h` (when `CUSTOM_KEYCODES_ENABLE` is not defined) uses `QK_KB_0`–`QK_KB_31`. If any layout accidentally leaves `CUSTOM_KEYCODES_ENABLE` undefined, keycodes collide with VIA's range (VIA uses `QK_KB_0`–`QK_KB_1`).
- **EEPROM wear**: `eeconfig_init_custom_rgb()` writes to EEPROM on every boot. STM32L432 flash is rated for ~100K erase cycles. At 10 boots/day, that's ~27 years. Acceptable.

---

## 8. Quick Reference — Build Commands

```bash
cd /home/franc/qmk_firmware

# Keychron Launcher firmware
make keychron/q2/iso_encoder:keychron

# VIA firmware
make keychron/q2/iso_encoder:via

# Default (no VIA/Launcher)
make keychron/q2/iso_encoder:default

# Flash
make keychron/q2/iso_encoder:keychron:flash

# Update from upstream
git checkout keychron-port
git merge master
# Fix conflicts in quantum/ files (13 files)
make keychron/q2/iso_encoder:keychron
```