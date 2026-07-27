# Keychron Q2 Firmware Port — Complete Documentation

**Date:** 2026-07-27  
**Branch:** `keychron-port`  
**Baseline:** Upstream QMK Firmware (`qmk/qmk_firmware`, master)  
**Source:** Keychron QMK Fork + Vial QMK Fork  

---

## 1. Goal

Add Keychron Launcher (proprietary config software) and Vial (open-source configurator) support to the upstream QMK firmware for the **Keychron Q2 ISO encoder** keyboard, with minimal core changes so upstream updates remain easy.

---

## 2. Architecture

All changes stay under `keyboards/keychron/` except 13 small patches to `quantum/` core files.

```
keyboards/keychron/
├── common/                          # Shared across all Keychron keyboards
│   ├── keychron.h                   # Master include header
│   ├── keychron_common.h/.c         # Custom keycodes + processing logic
│   ├── keychron_common.mk           # Build system for all common files
│   ├── keychron_raw_hid.h/.c        # Keychron Launcher protocol (0xA0–0xAB)
│   ├── keychron_task.h/.c           # Process/super-loop hooks
│   ├── backlit_indicator.h/.c       # RGB/LED mode indicators (OS toggle, Win lock)
│   ├── state_notify.h/.c            # Layer change notifications to Launcher
│   ├── eeconfig_kb.h/.c             # EEPROM layout for Keychron features
│   ├── dfu_info.c                   # Firmware version for Launcher
│   ├── nkro.c                       # NKRO toggle via Launcher
│   ├── factory_test.h/.c            # Factory test mode
│   ├── common.mk                    # Custom matrix scanning (not used by Q2)
│   ├── language/                    # Language switching for Launcher
│   ├── snap_click/                  # Snap-click (magnetic switch) support
│   ├── debounce/                    # Custom debounce algorithms (8 variants)
│   └── rgb/                         # Custom RGB matrix
│       ├── keychron_rgb.c           # Custom RGB driver (KC_KEYCHRON_RGB 0xA8)
│       ├── per_key_rgb.c            # Per-key RGB effect
│       ├── mixed_rgb.c              # Multi-layer mixed RGB effect
│       ├── retail_demo.c            # Retail store demo mode
│       └── ...
└── q2/
    ├── info.json                    # Updated: EEPROM 6144, debounce_type custom
    ├── config.h                     # CUSTOM_KEYCODES_ENABLE, I2C speed, LED tune
    ├── rules.mk                     # Includes keychron_common.mk
    ├── keycodes_custom.h            # Per-keyboard custom keycode definitions
    ├── q2.c                         # keyboard_post_init → keychron_common_init()
    ├── iso_encoder/iso_encoder.c    # SNLED27351 layout + default_per_key_led + default_region
    ├── iso_encoder/rules.mk         # KEYCHRON_RGB_ENABLE = yes
    ├── iso/rules.mk                 # KEYCHRON_RGB_ENABLE = yes
    ├── ansi/rules.mk                # KEYCHRON_RGB_ENABLE = yes
    ├── ansi_encoder/rules.mk        # KEYCHRON_RGB_ENABLE = yes
    ├── via_json/                    # VIA keyboard definition JSON files
    └── iso_encoder/keymaps/
        ├── via/                     # VIA keymap (upstream original, unchanged)
        ├── keychron/                # Keychron Launcher keymap (MAC/WIN layers, encoder)
        └── vial/                    # Vial keymap (our addition)
```

## 3. Files Ported from Keychron Fork

### 3.1 Common (`keyboards/keychron/common/`)

| File | Purpose | Adapted? |
|------|---------|----------|
| `keychron_common.h` | Custom keycodes (KC_LOPTN, KC_LCMMD, KC_SIRI, KC_TASK, KC_FILE, etc.) | Yes — uses upstream header paths |
| `keychron_common.c` | Key processing: Siri, Mac/Win key translation, OS toggle, GUI lock | Yes — `timer_read32()` instead of `sync_timer_read32()` |
| `keychron_common.mk` | Build system — pulls in all common sources | Yes — conditional includes for non-existent subdirs |
| `common.mk` | Matrix include (unused by Q2) | Direct copy |
| `keychron_raw_hid.h` | Launcher protocol definitions (0xA0–0xAB commands) | Yes — no `src` parameter |
| `keychron_raw_hid.c` | Launcher protocol handler | **Major adaptation** — removed `src` parameter, all calls use `RAW_HID_SRC_USB` (wired-only) |
| `keychron_task.h` / `.c` | Process/super loop hooks | Yes — removed ANANLOG_MATRIX references |
| `keychron.h` | Master include for all modules | Yes — simplified for Q2's needs |
| `state_notify.h` / `.c` | Layer change notifications to Launcher | Direct copy |
| `eeconfig_kb.h` / `.c` | EEPROM layout for features | Direct copy |
| `backlit_indicator.h` / `.c` | RGB mode indicators | Yes — removed wireless/ChibiOS-specific USB suspend checks |
| `dfu_info.c` | Firmware version info | Direct copy |
| `nkro.c` | NKRO on/off via Launcher | Direct copy |
| `factory_test.h` / `.c` | Factory test mode | Direct copy |
| `language/` | Language switching | Created minimal stub |
| `snap_click/` | Snap-click support | Direct copy |
| `debounce/` (12 files) | Custom debounce algorithms (8 variants + config) | Direct copy |
| `rgb/` (10 files) | Custom RGB: per_key, mixed, retail_demo | Yes — `rgb_matrix_set_color()` instead of `rgb_matrix_region_set_color()` with `params->region` |

### 3.2 Q2 Keyboard (`keyboards/keychron/q2/`)

| File | Status | Change |
|------|--------|--------|
| `info.json` | **Modified** | Added `build.debounce_type: custom`, `debounce: 50`, `eeprom.wear_leveling.driver/backing_size/logical_size`, `maintainer: Keychron` |
| `config.h` | **Modified** | Added `CUSTOM_KEYCODES_ENABLE`, I2C timing, SNLED27351 current tune, factory test key defs |
| `q2.c` | **Replaced** | Uses `#include "keychron.h"`, calls `keychron_common_init()` |
| `rules.mk` | **New** | Includes `keychron_common.mk` |
| `keycodes_custom.h` | **New** | Custom keycodes at QK_KB_2 range |
| `iso_encoder/iso_encoder.c` | **Replaced** | Added `default_per_key_led[]` and `default_region[]` arrays for custom RGB |
| `iso_encoder/rules.mk` | **New** | `KEYCHRON_RGB_ENABLE = yes` |
| `iso/rules.mk` | **New** | `KEYCHRON_RGB_ENABLE = yes` |
| `ansi/rules.mk` | **New** | `KEYCHRON_RGB_ENABLE = yes` |
| `ansi_encoder/rules.mk` | **New** | `KEYCHRON_RGB_ENABLE = yes` |
| `via_json/` (4 JSON files) | **New** | VIA keyboard definitions for all 4 layout variants |

### 3.3 Keymaps

| Keymap | Location | Purpose |
|--------|----------|---------|
| `via/` | `iso_encoder/keymaps/via/` | **Unchanged from upstream.** VIA configurator. |
| `keychron/` | `iso_encoder/keymaps/keychron/` (NEW) | Keychron Launcher firmware. Uses custom keycodes. |
| `keychron/` | `iso/keymaps/keychron/` (NEW) | Same for non-encoder variant. |
| `keychron/` | `ansi/keymaps/keychron/` (NEW) | Same for ANSI layout. |
| `vial/` | `iso_encoder/keymaps/vial/` (NEW) | Vial configurator. Uses custom keycodes + VialRGB. |

---

## 4. Core QMK Changes (`quantum/`)

These 13 files were modified. Each change is small but creates update friction.

### 4.1 RGB Matrix Region Support (3 files)

Required by `mixed_rgb.c` which runs multiple effects in different keyboard regions.

**`quantum/rgb_matrix/rgb_matrix_types.h`** — +1 field:
```c
typedef struct PACKED {
    uint8_t     iter;
    led_flags_t flags;
    bool        init;
    uint8_t     region;          // <-- ADDED
} effect_params_t;
```

**`quantum/rgb_matrix/rgb_matrix.h`** — +2 declarations:
```c
void rgb_matrix_region_set_color(uint8_t region, int index, uint8_t red, uint8_t green, uint8_t blue);
void rgb_matrix_region_set_color_all(uint8_t region, uint8_t red, uint8_t green, uint8_t blue);
```

**`quantum/rgb_matrix/rgb_matrix.c`** — +30 lines:
- `rgb_matrix_region_set_color()` — sets LED color only if LED's region matches
- `rgb_matrix_region_set_color_all()` — sets all LEDs in a region
- `uint8_t rgb_regions[RGB_MATRIX_LED_COUNT]` — global per-LED region assignment
- `rgb_matrix_none_indicators_kb()` / `rgb_matrix_none_indicators_user()` — weak override hooks
- `rgb_matrix_none_indicators()` — default implementation (calls kb + user hooks)
- `kc_effect_*` non-static wrapper functions (allows mixed_rgb.c to call static effect functions)
- Updated `rgb_effect_params` init to include `region: 0`

### 4.2 Debounce Signature Change (10 files)

Required because `keychron_debounce.c` uses a 4-parameter `debounce()` signature while upstream uses 3-parameter. Mismatch causes register corruption in ARM calling convention.

| File | Change |
|------|--------|
| `quantum/debounce.h` | `debounce(raw, cooked, changed)` → `debounce(raw, cooked, num_rows, changed)`. `debounce_init(void)` → `debounce_init(num_rows)`. Added `debounce_free()`. |
| `quantum/matrix.c` | Updated `debounce()` call to pass `MATRIX_ROWS`. Updated `debounce_init()` call. |
| `quantum/matrix_common.c` | Same as matrix.c |
| `quantum/debounce/sym_defer_g.c` | Updated to 4-param signature, added empty `debounce_free()` |
| `quantum/debounce/sym_defer_pr.c` | Same |
| `quantum/debounce/sym_defer_pk.c` | Same |
| `quantum/debounce/sym_eager_pr.c` | Same |
| `quantum/debounce/sym_eager_pk.c` | Same |
| `quantum/debounce/asym_eager_defer_pk.c` | Same |
| `quantum/debounce/none.c` | Same |

---

## 5. How Updating Works

```bash
cd /home/franc/qmk_firmware
git checkout keychron-port
git merge master
# Resolve conflicts in 13 quantum/ files (usually identical — just accept ours)
make keychron/q2/iso_encoder:keychron
```

The 13 `quantum/` core changes are the only conflict risk. Everything under `keyboards/keychron/` is new files, no conflicts.

---

## 6. Build Targets

```bash
# Keychron Launcher (recommended for full feature set)
make keychron/q2/iso_encoder:keychron

# Vial configurator
make keychron/q2/iso_encoder:vial

# VIA configurator (upstream original)
make keychron/q2/iso_encoder:via

# Default (no configurator)
make keychron/q2/iso_encoder:default

# Flash
make keychron/q2/iso_encoder:keychron:flash
```

---

## 7. Firmware Sizes

| Build | Size | Features |
|-------|------|----------|
| `default` | 50,416 B | Matrix, RGB, basic QMK |
| `via` | 56,964 B | VIA + Keychron common infra |
| `keychron` | 56,964 B | Keychron Launcher + VIA + custom RGB |
| `vial` | TBD | Vial + VialRGB + Keychron common infra |

---

## 8. Key Design Decisions

### 8.1 Wired-only (`src` parameter removed)
The Keychron fork uses a `src` parameter (USB vs wireless) in all raw HID functions. Since Q2 is wired-only, we removed `src` and always use `RAW_HID_SRC_USB` (0). This avoids modifying `quantum/raw_hid.h` and `quantum/via.c` — the two core files that would otherwise change.

### 8.2 No core via.c changes
The fork modifies `quantum/via.c` to use `via_raw_hid_send(src, ...)` instead of `raw_hid_send(...)`. We didn't port this. Instead, the Keychron protocol hooks into `via_command_kb()` which the upstream already exposes as a weak override. Response sending is handled inside the hook.

### 8.3 Keycode range
The fork's common `keychron_common.h` uses `QK_KB_0` for custom keycodes. The Q2's `keycodes_custom.h` (with `CUSTOM_KEYCODES_ENABLE`) uses `QK_KB_2` to avoid conflicting with VIA's use of `QK_KB_0`–`QK_KB_1`. This is documented in the upstream's original code comment as "TECH DEBT: see #19884".

### 8.4 RGB effect wrappers
Upstream QMK declares all RGB effect functions as `static` (internal linkage). The Keychron `mixed_rgb.c` needs to call them by name. We solved this by generating non-static `kc_effect_*()` wrapper functions in `rgb_matrix.c` that forward to the static originals.

---

## 9. Known Limitations

| Issue | Impact | Status |
|-------|--------|--------|
| VIA protocol version mismatch | Vial/VIA report 0x000D, Keychron fork uses 0x000C | Not observed to cause issues |
| `debounce_type: custom` needed for Launcher debounce controls | Without it, Launcher can't adjust debounce | Working with debounce signature fix |
| EEPROM `backing_size: 6144` required | Uses 3 flash pages instead of 2 | Verified stable |
| Keychron RGB + VialRGB may conflict | Not yet tested | Untested — Vial keymap is new |
| `per_key_led` arrays in `iso_encoder.c` are layout-specific | Each variant needs its own array | Only created for iso_encoder |

---

## 10. Files Summary

```
67 files changed, 5453 insertions(+), 110 deletions(-)
```

- **New files:** 64 (under `keyboards/keychron/`)
- **Modified files:** 3 in `keyboards/keychron/` (info.json, config.h, q2.c)
- **Core modified files:** 10 in `quantum/`