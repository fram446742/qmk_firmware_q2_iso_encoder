#!/usr/bin/env python3
"""
Generate key_positions.h — maps KC_* keycodes to matrix positions for MAC_BASE layer.

Reads keyboard.json (layout positions) and keymap.c (MAC_BASE key order) to produce
#define POS_KC_xxx ((row << 8) | col) macros keyed by the standard keycode name.
"""

import re, json, os, sys

KEYMAP_DIR = os.path.dirname(os.path.abspath(__file__))

# ── Resolve paths ───────────────────────────────────────────────────────────
keyboard_json = os.path.normpath(os.path.join(KEYMAP_DIR, '../../keyboard.json'))
# The actual keymap.c is the one in this keymap directory
keymap_c = os.path.join(KEYMAP_DIR, 'keymap.c')

# ── Parse keyboard.json layout ──────────────────────────────────────────────
with open(keyboard_json) as f:
    kb = json.load(f)

layout = kb['layouts']['LAYOUT_iso_68']['layout']
# Each entry: {"matrix": [row, col], "x": ..., "y": ...}

# ── Extract MAC_BASE keycodes from keymap.c ────────────────────────────────
# We look for the first LAYOUT_iso_68(...) call which is always MAC_BASE.
with open(keymap_c) as f:
    text = f.read()

# Find the MAC_BASE LAYOUT_iso_68( ... ) block — it's the first one, ends with ),
m = re.search(r'LAYOUT_iso_68\s*\((.*?)\)\s*,', text, re.DOTALL)
if not m:
    print("ERROR: Could not find LAYOUT_iso_68 call in keymap.c", file=sys.stderr)
    sys.exit(1)

args_raw = m.group(1)
# Split by commas, trimming whitespace and handling line continuations
# Remove line comments and newlines within the block
args_raw = re.sub(r'//.*', '', args_raw)
args_raw = re.sub(r'\s+', ' ', args_raw).strip()
# Split by comma — but be careful: KC_ macros with commas inside won't exist
args = [a.strip() for a in args_raw.split(',') if a.strip()]

# The encoder key (k0O / KC_MUTE) is at index 14 but shares the same matrix [0,14]
# Some entries in the layout have no corresponding keycode due to ISO big-enter shape
# Layout has 68 entries; keycodes should also have 68 after accounting for gaps.

if len(args) != len(layout):
    print(f"WARNING: {len(args)} keycodes vs {len(layout)} layout entries", file=sys.stderr)

# ── Build QMK keycode name lookup ──────────────────────────────────────────
# Standard HID usage → keycode name mapping (subset of quantum/keycodes.h)
HID_TO_NAME = {
    0x29: "ESC", 0x1E: "1", 0x1F: "2", 0x20: "3", 0x21: "4",
    0x22: "5", 0x23: "6", 0x24: "7", 0x25: "8", 0x26: "9", 0x27: "0",
    0x2D: "MINS", 0x2E: "EQL", 0x2A: "BSPC", 0x2B: "TAB",
    0x14: "Q", 0x1A: "W", 0x08: "E", 0x15: "R", 0x17: "T",
    0x1C: "Y", 0x18: "U", 0x0C: "I", 0x12: "O", 0x13: "P",
    0x2F: "LBRC", 0x30: "RBRC", 0x28: "ENT",
    0x33: "SCLN", 0x34: "QUOT", 0x35: "GRV", 0x31: "NUBS",
    0x3A: "CAPS", 0x04: "A", 0x16: "S", 0x07: "D", 0x09: "F",
    0x0A: "G", 0x0B: "H", 0x0D: "J", 0x0E: "K", 0x0F: "L",
    0x32: "NUHS", 0x2C: "Z", 0x1D: "X", 0x06: "C", 0x19: "V",
    0x05: "B", 0x11: "N", 0x10: "M",
    0x36: "COMM", 0x37: "DOT", 0x38: "SLSH",
    0x2C: "Z", 0x1D: "X",
    0xE1: "LSFT", 0xE5: "RSFT", 0xE0: "LCTL", 0xE4: "RCTL",
    0xE2: "LALT", 0xE6: "RALT", 0xE3: "LGUI", 0xE7: "RGUI",
    0x52: "LEFT", 0x51: "DOWN", 0x4F: "RIGHT", 0x50: "UP",
    0x2C: "Z",
    0x4C: "DEL", 0x4A: "HOME",
    0x39: "SPC",
    0x7E: "MUTE",
    0x65: "APP",
    0x46: "MCTL", 0x47: "MUTE", 0x48: "MPRV",
    0x49: "MUTE", 0x3F: "MUTE",
}

# Custom Keychron keycodes (from keycodes_custom.h)
CUSTOM_NAMES = {
    # KC_LOPTN, KC_ROPTN, KC_LCMMD, KC_RCMMD
    # These are QK_KB_2 + N, hard to predict. We'll handle KC_LCMMD etc.
}

# Manual name override for known custom keycodes not in HID
NAME_OVERRIDE = {
    'QK_GESC': 'GESC',
}

def kc_to_name(raw):
    """Convert a C token like KC_ESC or QK_GESC to the name part for POS_KC_xxx."""
    raw = raw.strip()
    if raw == '_______' or raw == 'KC_TRNS':
        return None  # transparent, no fixed position mapping
    if raw.startswith('MO(') or raw.startswith('FN'):
        return None  # layer keys, skip
    # Remove KC_ or QK_ prefix
    if raw.startswith('KC_'):
        return raw[3:]
    if raw.startswith('QK_'):
        return raw[3:]
    # Custom macros like FN1_MAC
    if raw in ('FN1_MAC', 'FN1_WIN', 'FN2'):
        return None
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

# Keep track of seen positions to detect duplicates
seen_positions = {}

for i, (arg, entry) in enumerate(zip(args, layout)):
    name = kc_to_name(arg)
    if name is None:
        continue
    row, col = entry['matrix']
    pos = (row << 8) | col
    pos_key = f"POS_KC_{name}"
    if pos_key in seen_positions:
        # Duplicate — probably a same-position key on different rows of the macro
        if seen_positions[pos_key] != pos:
            print(f"WARNING: {pos_key} redefined: {seen_positions[pos_key]} vs {pos}", file=sys.stderr)
        continue
    seen_positions[pos_key] = pos
    lines.append(f'#define {pos_key}  PACK_MTX({row}, {col})  // {arg}')

lines.append('')
lines.append(f'// Total: {len(seen_positions)} keys mapped')

# ── Write output ───────────────────────────────────────────────────────────
out_path = os.path.join(KEYMAP_DIR, 'key_positions.h')
with open(out_path, 'w') as f:
    f.write('\n'.join(lines) + '\n')

print(f"Generated {out_path} — {len(seen_positions)} positions")
