# Indicator LEDs — Q2 ISO Encoder

All indicator LEDs use Keychron's proprietary `os_state_indicate()` path, so
they appear as toggles in the Keychron Launcher's indicator settings.

| Indicator | Physical key | LED index | Launcher toggle |
|---|---|---|---|
| Caps Lock | Caps | 28 (`POS_IDX_KC_CAPS`) | Caps Lock |
| Num Lock | Del | 27 (`POS_IDX_KC_DEL`) | Num Lock |
| Win Lock | Left Option | 58 (`POS_IDX_KC_LOPTN`) | Win Lock (`no_gui`) |

## How it works

Each indicator is enabled by a single `#define` in
`keyboards/keychron/q2/iso_encoder/config.h`:

```c
#define CAPS_LOCK_INDEX 28      // physical Caps
#define NUM_LOCK_INDEX  27      // physical Del
#define WINLOCK_LED_LIST { 58 } // physical Left Option (Win key)
```

`os_state_indicate()` in `keyboards/keychron/common/rgb/keychron_rgb.c` draws
them, and the Launcher discovers them via `GET_INDICATORS_CONFIG` (the
availability mask in `keychron_rgb.c:get_indicators_config()`). The disable
state and shared color live in the EEPROM `os_indicator_config_t`
(`os_ind_cfg.disable.*` + `os_ind_cfg.hsv`).

Notes:

- `WINLOCK_LED_LIST` lights red only while the Win key is locked
  (`keymap_config.no_gui`), not while Windows is simply the active OS mode —
  so it is off until the user toggles Win Lock. This is the same LED the old
  custom `IND_WIN_LOCK_HOST` used.
- The proprietary names differ from the upstream c2_pro convention
  (`NUM_LED_INDEX`, `CAPS_LED_INDEX`, `MAC_LED_INDEX`, `WIN_LED_INDEX`).
  Those belong to `led_matrix_indicators_kb` and do nothing here.

## Known vendor bug

`keychron_rgb.c:os_state_indicate()` checks
`host_keyboard_led_state().compose` for `SCROLL_LOCK_INDEX` instead of
`.scroll_lock`. Not fixed here (vendor code, and Scroll Lock is unused on this
keymap); any future port adding `SCROLL_LOCK_INDEX` must fix that line first.
