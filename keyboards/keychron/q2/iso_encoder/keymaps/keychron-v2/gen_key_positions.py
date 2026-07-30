#!/usr/bin/env python3
"""
Generate key_positions.h — maps keycode names to matrix positions AND LED indices.

Reads keyboard.json (layout positions) and keymap.c (MAC_BASE key order) to produce:
  #define POS_KC_xxx   PACK_MTX(row, col)   — matrix position for tap-override base_id
  #define POS_IDX_xxx  <led_index>           — LED index for indicator macros

Both use the same xxx suffix (KC_xxx or QK_xxx), so you can switch between
numeric and symbolic forms by editing one #define in keymap_config.h.
"""

import re, json, os, sys

KEYMAP_DIR = os.path.dirname(os.path.abspath(__file__))

# ── Resolve paths ───────────────────────────────────────────────────────────
keyboard_json = os.path.normpath(os.path.join(KEYMAP_DIR, '../../keyboard.json'))
keymap_c = os.path.join(KEYMAP_DIR, 'keymap.c')

# ── Parse keyboard.json layout ──────────────────────────────────────────────
with open(keyboard_json) as f:
    kb = json.load(f)

layout = kb['layouts']['LAYOUT_iso_68']['layout']

# ── Extract MAC_BASE keycodes from keymap.c ────────────────────────────────
with open(keymap_c) as f:
    text = f.read()

m = re.search(r'LAYOUT_iso_68\s*\((.*?)\)\s*,', text, re.DOTALL)
if not m:
    print("ERROR: Could not find LAYOUT_iso_68 call in keymap.c", file=sys.stderr)
    sys.exit(1)

args_raw = m.group(1)
args_raw = re.sub(r'//.*', '', args_raw)
args_raw = re.sub(r'\s+', ' ', args_raw).strip()
args = [a.strip() for a in args_raw.split(',') if a.strip()]


def kc_to_macro_suffix(raw):
    """Convert a C token like KC_ESC or QK_GESC to the suffix for POS_/POS_IDX_."""
    raw = raw.strip()
    if raw in ('_______', 'KC_TRNS', 'FN1_MAC', 'FN1_WIN', 'FN2'):
        return None
    if raw.startswith('MO(') or raw.startswith('FN'):
        return None
    # Keep the full prefix so POS_ matches the original keycode name
    return raw


# ── Generate output ────────────────────────────────────────────────────────
lines = []
lines.append('/* Auto-generated from keyboard.json + keymap.c MAC_BASE layer')
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

seen = {}  # suffix → (matrix_pos, led_index)
for i, (arg, entry) in enumerate(zip(args, layout)):
    suffix = kc_to_macro_suffix(arg)
    if suffix is None:
        continue
    row, col = entry['matrix']
    pos = (row << 8) | col
    if suffix in seen:
        continue  # skip duplicates (same key on multiple LAYOUT params)
    seen[suffix] = (pos, i)
    lines.append(f'#define POS_{suffix}     PACK_MTX({row}, {col})  // {arg}')
    lines.append(f'#define POS_IDX_{suffix} {i}                    // {arg}')
    lines.append('')

lines.append(f'// Total: {len(seen)} keys')

# ── LED → matrix-position lookup array (for layer visualization) ─────────
# Index by LED index from g_snled27351_leds[], get PACK_MTX(row,col).
# Keys without an RGB LED (e.g. rotary encoder) are excluded from the array.
lines.append('')
lines.append('// ── LED-index → matrix-position lookup ──────────────────────────')
lines.append('// Keys without an RGB LED (e.g. rotary encoder) are excluded.')

# Identify layout entries that have no RGB LED (encoder key).
no_led = {(0, 14)}  # (row,col) sets with no RGB LED on this keyboard
led_layout = [e for e in layout if tuple(e['matrix']) not in no_led]

lines.append(f'static const uint16_t PROGMEM led_to_mtx[{len(led_layout)}] = {{')
for entry in led_layout:
    row, col = entry['matrix']
    lines.append(f'    PACK_MTX({row}, {col}),')
lines.append('};')
lines.append('')

# ── Write output ───────────────────────────────────────────────────────────
out_path = os.path.join(KEYMAP_DIR, 'key_positions.h')
with open(out_path, 'w') as f:
    f.write('\n'.join(lines) + '\n')

print(f"Generated {out_path} — {len(seen)} keys ({len(args)} layout entries)")
