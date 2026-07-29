# Dead Code Removal Scope — Keychron Q2

This document catalogs modules that are currently compiled into the firmware
but serve no runtime purpose on a **wired mechanical Keychron Q2 (ISO encoder,
STM32L432)**. Removing them saves flash, reduces compile time, and eliminates
maintenance surface.

**Current baseline:** `keychron/q2/iso_encoder:keychron` — 59.7 KB / 256 KB
flash (76% free), 7.7 KB / 64 KB RAM (88% free).

There is no capacity pressure.  All removals are purely for
maintainability — less code to audit when rebasing onto upstream.

---

## 1. Factory Test  (`factory_test.c` / `.h`)

| Metric | Value |
|--------|-------|
| Lines | ~564 |
| Enabled by | `FACTORY_TEST_ENABLE` in `keychron_common.mk` |
| Runtime trigger | Launcher command 0xAB |
| Why removable | Q2 is a production unit.  The factory-test command is never sent by the Launcher app after manufacturing. |

**Effect of removing:**
- Compile-time: `#define FACTORY_TEST_ENABLE` → out
- No functional change at runtime; the raw HID dispatch returns an error code
  for unknown command 0xAB instead of entering test mode.
- Stub call `factory_test_indicator()` in `keychron_task.c` goes away.

---

## 2. Snap Click  (`snap_click/` — 4 files)

| Metric | Value |
|--------|-------|
| Lines | ~228 |
| Enabled by | `SNAP_CLICK_ENABLE` via `snap_click.mk` (included unconditionally from `keychron_common.mk`) |
| Purpose | Bistable / magnetic-switch debounce (Hall-effect, Aikenso/Omron B3G) |
| Why removable | Q2 uses standard mechanical Cherry/Gateron switches. No magnetic sensors. |

**Effect of removing:**
- `#include` path in `keychron_common.mk` commented out or guarded by a
  per-keyboard `#define`.
- Saves ~228 lines + EEPROM struct `snap_click_pair` (60 bytes RAM).
- `snap_click.mk` file tree is safe to delete.

---

## 3. Retail Demo  (`retail_demo.c` / `.h`)

| Metric | Value |
|--------|-------|
| Lines | ~188 |
| Enabled by | `RETAIL_DEMO_ENABLE` via `rgb/rgb.mk` |
| Purpose | Showroom demo: cycles RGB effects automatically |
| Why removable | Not a retail demo unit. No customer ever triggers it. |

**Effect of removing:**
- Remove `-DRETAIL_DEMO_ENABLE` from `rgb/rgb.mk`.
- Guards in `keychron_task.c` (`#ifdef RETAIL_DEMO_ENABLE`) become dead code.
- Saves ~188 lines + retail EEConfig struct.

---

## 4. Language Switching  (`language/` — 3 files)

| Metric | Value |
|--------|-------|
| Lines | ~67 |
| Enabled by | `language.mk` (included unconditionally from `keychron_common.mk`) |
| Purpose | Multi-language keyboard layout switching (Launcher control) |
| Why removable | Q2 has a single physical layout (ISO). One user, one language. |

**Effect of removing:**
- Remove include line from `keychron_common.mk`.
- Saves ~67 lines + EEConfig byte.

---

## 5. USB Report Rate  (`usb_report_rate.c`)

| Metric | Value |
|--------|-------|
| Lines | ~202 |
| Enabled by | `USB_REPORT_INTERVAL_ENABLE` (not currently set for Q2) |
| Purpose | Dynamic switch between 1K/4K/8K Hz polling via FN+combo |
| Why removable | **STM32L432 is USB 2.0 Full-Speed only** — max 1 kHz is a hardware
  ceiling.  The 4K/8K options in the code cannot function on this MCU.
  The Q2 does not enable this feature already; the file is dead weight in
  the source tree but not compiled. |

**Effect of removing:**
- The file is not compiled for Q2 (no `USB_REPORT_INTERVAL_ENABLE`).
- Already dead in the build.  Listed here only because it's in the common
  directory and exists as a trap for future readers.

---

## 6. Unused Debounce Variants  (6 of 7 algorithm files)

| Metric | Value |
|--------|-------|
| Lines | ~600 |
| Enabled by | `DEBOUNCE_TYPE=custom` triggers `debounce.mk` which compiles all 7 |
| Default | `DEBOUNCE_SYM_EAGER_PER_KEY` (1 algorithm at runtime) |
| Why removable | Only one `debounce_t` dispatch slot runs at a time.  The other 6 `.c`
  files are compiled and linked but never called. |

**List of variants in `keyboards/keychron/common/debounce/`:**

| File | Preserved? | Reason |
|------|-----------|--------|
| `keychron_debounce.c` | Keep | Dispatcher + EEPROM config |
| `keychron_debounce.h` | Keep | Header |
| `eeconfig_debounce.h` | Keep | EEPROM offset |
| `debounce.mk` | Keep | Build wire-in |
| `sym_eager_pk.c` | **Keep** | Default algorithm for Q2 |
| `sym_eager_pr.c` | Remove | |
| `sym_defer_g.c` | Remove | |
| `sym_defer_pr.c` | Remove | |
| `sym_defer_pk.c` | Remove | |
| `asym_eager_defer_pk.c` | Remove | |
| `none.c` / `none.h` | Remove | |

**Effect of removing:**
- Saves ~600 lines of source + ~200 bytes of dead code from the .text section
  (linker strips most unused functions, but not all — the extern declarations
  in `keychron_debounce.c` force linkage).

---

## 7. Unused RGB Animation Effects

| Metric | Value |
|--------|-------|
| Enabled | 21 effects in `info.json` |
| Proposed | Keep only the ~6 used by Launcher (cycle_left_right, cycle_all,
  breathing, solid_reactive_simple, solid_splash, splash) |
| Why optional | Each disabled effect saves ~200–400 bytes of flash from the enum
  table + effect function.  Low priority — less than 5 KB total. |

**Effect of removing:**
- Check each `"ENABLE_RGB_MATRIX_*": true` in `info.json` against what the
  Launcher actually exposes.  Disable the rest.

---

## Estimated Savings

| Module | Flash | RAM | Source lines |
|--------|-------|-----|-------------|
| Factory test | ~2 KB | — | 564 |
| Snap click | ~1 KB | 60 B | 228 |
| Retail demo | ~1 KB | — | 188 |
| Language | ~200 B | — | 67 |
| USB report rate | 0 (not compiled) | — | 202 |
| Unused debounce | ~500 B | — | 600 |
| Unused RGB effects | ~3 KB | — | variable |
| **Total** | **~8 KB** | **60 B** | **~1,850** |

Flash would drop from 59.7 KB → ~52 KB.  RAM impact negligible.

---

## How to Implement

```makefile
# In keyboards/keychron/q2/config.h (per-keyboard, not common):
#define KEYCHRON_DISABLE_FACTORY_TEST
#define KEYCHRON_DISABLE_SNAP_CLICK
#define KEYCHRON_DISABLE_RETAIL_DEMO
#define KEYCHRON_DISABLE_LANGUAGE
```

Then guard the includes in `keychron_common.mk` and `rgb/rgb.mk` with
`ifndef KEYCHRON_DISABLE_<FEATURE>`.

For debounce trimming, modify `debounce.mk` to compile only the selected
algorithm instead of all 7.
