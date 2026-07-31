#!/usr/bin/env python3
"""
Generate key_positions.h — maps keycode names to matrix positions AND LED indices.

Auto-detects which QMK variant is being used (ISO/ANSI/JIS × encoder/no-encoder)
by reading keyboard.json and keymap.c from the keymap directory.

Produces:
  #define POS_xxx   PACK_MTX(row, col)  — matrix position for tap-override base_id
  #define POS_IDX_xxx  <led_index>      — LED index for indicator macros
  static const uint16_t PROGMEM led_to_mtx[67]  — LED→matrix lookup for layer vis
"""

import re, json, os, sys

KEYMAP_DIR = os.path.dirname(os.path.abspath(__file__))

# ── Resolve paths ───────────────────────────────────────────────────────────
keyboard_json = os.path.normpath(os.path.join(KEYMAP_DIR, '../../keyboard.json'))
keymap_c = os.path.join(KEYMAP_DIR, 'keymap.c')

if not os.path.exists(keyboard_json):
    print(f"ERROR: keyboard.json not found at {keyboard_json}", file=sys.stderr)
    sys.exit(1)
if not os.path.exists(keymap_c):
    print(f"ERROR: keymap.c not found at {keymap_c}", file=sys.stderr)
    sys.exit(1)


# ── Parse keyboard.json ─────────────────────────────────────────────────────
with open(keyboard_json) as f:
    kb = json.load(f)

# Detect encoder: if there's an encoder section, one key has no RGB LED
has_encoder = kb.get('encoder', {}).get('enabled', False)

# Find the layout definition — there should be exactly one LAYOUT_* entry
layouts = kb.get('layouts', {})
if len(layouts) != 1:
    print(f"WARNING: expected 1 layout, found {len(layouts)}", file=sys.stderr)
layout_name, layout_data = next(iter(layouts.items()))
layout = layout_data['layout']
layout_count = len(layout)


# ── Extract MAC_BASE keycodes from keymap.c ────────────────────────────────
with open(keymap_c) as f:
    text = f.read()

# Match the first LAYOUT_*(...) call — should be MAC_BASE
m = re.search(r'LAYOUT_\w+\s*\((.*?)\)\s*,', text, re.DOTALL)
if not m:
    print("ERROR: Could not find LAYOUT_* call in keymap.c", file=sys.stderr)
    sys.exit(1)

args_raw = m.group(1)
args_raw = re.sub(r'//.*', '', args_raw)
args_raw = re.sub(r'\s+', ' ', args_raw).strip()
args = [a.strip() for a in args_raw.split(',') if a.strip()]

if len(args) != layout_count:
    print(f"WARNING: {len(args)} keycodes vs {layout_count} layout entries", file=sys.stderr)


# ── Determine which layout entries have no RGB LED ──────────────────────────
# On encoder variants, the encoder key (typically the rightmost key in
# the top row) lacks an LED.  Non-encoder variants have all keys lit.
no_led_positions = set()
if has_encoder:
    # The encoder is the rightmost key in the first row (largest x coordinate
    # among entries with y == minimum y, excluding big keys like space).
    rows = {}
    for entry in layout:
        y_key = round(entry['y'], 2)
        rows.setdefault(y_key, []).append(entry)

    # First row = smallest y
    first_row_y = min(rows.keys())
    first_row = rows[first_row_y]

    # Pick the entry with the maximum x — that's the encoder.
    encoder_entry = max(first_row, key=lambda e: e['x'])
    no_led_positions.add(tuple(encoder_entry['matrix']))
    print(f"  Encoder at matrix {encoder_entry['matrix']} — no RGB LED", file=sys.stderr)


# ── Per-key macro suffix ────────────────────────────────────────────────────
def kc_to_macro_suffix(raw):
    raw = raw.strip()
    if raw in ('_______', 'KC_TRNS', 'FN1_MAC', 'FN1_WIN', 'FN2'):
        return None
    if raw.startswith('MO(') or raw.startswith('FN'):
        return None
    return raw


# ── Generate output ─────────────────────────────────────────────────────────
lines = []
lines.append('/* Auto-generated from keyboard.json + keymap.c MAC_BASE layer')
lines.append(f' * Layout: {layout_name}, {layout_count} entries, encoder={has_encoder}')
lines.append(' * SPDX-License-Identifier: GPL-2.0-or-later */')
lines.append('#pragma once')
lines.append('')
lines.append('#include <stdint.h>')
lines.append('')
lines.append('// ── Packed matrix position: (row << 8) | col ──────────────────────')
lines.append('#define PACK_MTX(r, c)  (((r) << 8) | (c))')
lines.append('')
lines.append('// ── Per-key macros ────────────────────────────────────────────────')
lines.append('//  POS_xxx    = packed matrix position (used for tap-override base_id)')
lines.append('//  POS_IDX_xxx = LED index in g_snled27351_leds[] (used for IND_* macros)')
lines.append('')

# ── LED → matrix-position lookup array ─────────────────────────────────────
# Keys without an RGB LED (e.g. rotary encoder) are excluded.
led_layout = [e for e in layout if tuple(e['matrix']) not in no_led_positions]

# Hardware wiring correction: on the Q2 ISO(-Encoder), the ISO Enter's
# bottom cell (2, 12) is driven by the LAST channel of the bottom-row
# driver group, so it is the LAST lit LED in g_snled27351_leds[] — even
# though the layout array lists it before the Right arrow (4, 14).
# Verified against keyboards/keychron/q2/iso/iso.c (the full 68-LED
# array) vs iso_encoder.c (identical, minus the encoder entry at (0, 14)).
ENTER_BOTTOM = (2, 12)
led_layout = [e for e in led_layout if tuple(e['matrix']) != ENTER_BOTTOM] \
           + [e for e in led_layout if tuple(e['matrix']) == ENTER_BOTTOM]

# Real LED index per matrix position (lit keys only)
led_index = {tuple(e['matrix']): i for i, e in enumerate(led_layout)}
led_count = len(led_layout)

seen = {}
for arg, entry in zip(args, layout):
    suffix = kc_to_macro_suffix(arg)
    if suffix is None:
        continue
    row, col = entry['matrix']
    if suffix in seen:
        continue
    seen[suffix] = (row << 8) | col
    lines.append(f'#define POS_{suffix}     PACK_MTX({row}, {col})  // {arg}')
    led = led_index.get((row, col))
    if led is not None:
        lines.append(f'#define POS_IDX_{suffix} {led}                    // {arg}')
    lines.append('')

lines.append(f'// Total: {len(seen)} keys, {led_count} with LEDs')

lines.append('')
lines.append('// ── LED-index → matrix-position lookup (for layer visualization) ──')
lines.append(f'// {layout_count} layout entries, {led_count} with LEDs ({layout_count - led_count} skipped)')
lines.append(f'static const uint16_t PROGMEM led_to_mtx[{led_count}] = {{')
for entry in led_layout:
    row, col = entry['matrix']
    lines.append(f'    PACK_MTX({row}, {col}),')
lines.append('};')
lines.append('')

# ── Write output ───────────────────────────────────────────────────────────
out_path = os.path.join(KEYMAP_DIR, 'key_positions.h')
with open(out_path, 'w') as f:
    f.write('\n'.join(lines) + '\n')

print(f"Generated {out_path} — {len(seen)} keys, {led_count} LEDs ({layout_count} layout entries)")
