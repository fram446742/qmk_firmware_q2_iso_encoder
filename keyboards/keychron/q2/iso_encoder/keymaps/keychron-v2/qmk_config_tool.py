#!/usr/bin/env python3
"""
QMK Config Export/Import Tool
───────────────────────────────
Reads/writes the full keyboard configuration over USB Raw HID.

  python3 qmk_config_tool.py export config.json
  python3 qmk_config_tool.py import config.json
  python3 qmk_config_tool.py dump
  python3 qmk_config_tool.py --mock export config.json
"""

import json, struct, sys, argparse
from dataclasses import dataclass, field
from typing import Optional

# ═══════════════════════════════════════════════════════════════════════════
# HID Protocol (matches firmware via_custom_value_command_kb)
# ═══════════════════════════════════════════════════════════════════════════
# Every transaction is a 32-byte report:
#   [0] = command: 0x07=set, 0x08=get, 0x09=save
#   [1] = channel: 0x00 (our custom channel)
#   [2] = value_id
#   [3] = index or count
#   [4..31] = payload

VIA_PROTOCOL_GET     = 0x08
VIA_PROTOCOL_SET     = 0x07
VIA_PROTOCOL_SAVE    = 0x09
VIA_CHANNEL          = 0x00

VALUE_FLAGS          = 0x01   # feature flags (1 byte)
VALUE_TAP_COUNT      = 0x02   # tap override count (1 byte)
VALUE_TAP_ENTRY      = 0x03   # tap override entry by index (10 bytes)
VALUE_COMBO_COUNT    = 0x04   # combo count (1 byte)
VALUE_COMBO_ENTRY    = 0x05   # combo entry by index (10 bytes)
VALUE_LEADER_COUNT   = 0x06   # leader count (1 byte)
VALUE_LEADER_ENTRY   = 0x07   # leader entry by index (6 bytes)

# Feature bit positions
FEATURE_MAP = {0: "tapDance", 1: "autoShift", 2: "capsWord",
               3: "repeatKey", 4: "dynMacro", 5: "leader", 6: "autocorrect"}

DBL_KEYCODE     = 0
DBL_UNICODE_STR = 1
DBL_UNICODE_CP  = 2

# ═══════════════════════════════════════════════════════════════════════════
# Keycode helpers
# ═══════════════════════════════════════════════════════════════════════════

KC = {  # subset of common QMK keycodes
    0x29:"KC_ESC",0x1E:"KC_1",0x1F:"KC_2",0x20:"KC_3",0x21:"KC_4",
    0x22:"KC_5",0x23:"KC_6",0x24:"KC_7",0x25:"KC_8",0x26:"KC_9",0x27:"KC_0",
    0x2D:"KC_MINS",0x2E:"KC_EQL",0x2A:"KC_BSPC",0x39:"KC_SPC",
    0x2B:"KC_TAB",0x14:"KC_Q",0x1A:"KC_W",0x08:"KC_E",0x15:"KC_R",
    0x17:"KC_T",0x1C:"KC_Y",0x18:"KC_U",0x0C:"KC_I",0x12:"KC_O",
    0x13:"KC_P",0x2F:"KC_LBRC",0x30:"KC_RBRC",0x28:"KC_ENT",
    0x33:"KC_SCLN",0x34:"KC_QUOT",0x35:"KC_GRV",0x31:"KC_NUBS",
    0x3A:"KC_CAPS",0x04:"KC_A",0x16:"KC_S",0x07:"KC_D",0x09:"KC_F",
    0x0A:"KC_G",0x0B:"KC_H",0x0D:"KC_J",0x0E:"KC_K",0x0F:"KC_L",
    0x32:"KC_NUHS",0x2C:"KC_Z",0x1D:"KC_X",0x06:"KC_C",0x19:"KC_V",
    0x05:"KC_B",0x11:"KC_N",0x10:"KC_M",0x36:"KC_COMM",0x37:"KC_DOT",
    0x38:"KC_SLSH",0x7C:"KC_DEL",0x4A:"KC_HOME",
    0xE1:"KC_LSFT",0xE5:"KC_RSFT",0xE0:"KC_LCTL",0xE4:"KC_RCTL",
    0xE2:"KC_LALT",0xE6:"KC_RALT",0xE3:"KC_LGUI",0xE7:"KC_RGUI",
    0x36:"KC_BSLS",0x52:"KC_UP",0x50:"KC_LEFT",0x51:"KC_DOWN",
    0x4F:"KC_RGHT",0xB0:"KC_MUTE",
    0x7C73:"CW_TOGG",0x5C01:"KC_FEAT_OVERVIEW",
}
KC_NAMES = {v:k for k,v in KC.items()}

# Modifier bit values (used in leader entries, NOT keycodes)
# These are QMK's MOD_* constants: 1=LCtl,2=LSft,4=LAlt,8=LGui
MOD_NAMES = {1:"MOD_LCTL",2:"MOD_LSFT",4:"MOD_LALT",8:"MOD_LGUI",
             0x11:"MOD_RCTL",0x12:"MOD_RSFT",0x14:"MOD_RALT",0x18:"MOD_RGUI"}
MOD_VALS = {v:k for k,v in MOD_NAMES.items()}

def kc_name(v):
    if v == 0: return "KC_NO"
    if v in MOD_NAMES: return MOD_NAMES[v]
    if v in KC: return KC[v]
    if 0x5700 <= v <= 0x57FF: return f"TD({v-0x5700})"
    if 0x5F00 <= v <= 0x5FFF: return f"CUSTOM({v-0x5F00})"
    if v >= 0x8000: return f"UC(0x{v-0x8000:04X})"
    return f"0x{v:04X}"

def kc_val(s):
    if isinstance(s, int): return s
    if s.startswith("0x"): return int(s,16)
    if s.startswith("TD("): return 0x5700+int(s[3:-1])
    if s.startswith("CUSTOM("): return 0x5F00+int(s[7:-1])
    if s.startswith("UC(0x"): return 0x8000+int(s[5:-1],16)
    if s in KC_NAMES: return KC_NAMES[s]
    if s in MOD_VALS: return MOD_VALS[s]
    try: return int(s)
    except: return 0

# ═══════════════════════════════════════════════════════════════════════════
# Data structures
# ═══════════════════════════════════════════════════════════════════════════

def pack_tap_entry(base, tap, dtype, dval, dextra=0):
    return struct.pack('<HHBBHH', kc_val(base), kc_val(tap), dtype, 0, dval, dextra)

def unpack_tap_entry(data):
    b,t,d,_,v,x = struct.unpack_from('<HHBBHH', data)
    return b,t,d,v,x

def pack_combo(keys, out):
    ks = [kc_val(k) for k in keys] + [0]*(4-len(keys))
    return struct.pack('<HHHHH', *ks, kc_val(out))

def unpack_combo(data):
    k0,k1,k2,k3,out = struct.unpack_from('<HHHHH', data)
    return [k for k in [k0,k1,k2,k3] if k], out

def pack_leader(seq, mod, key):
    s = [kc_val(k) for k in seq]+[0]*(3-len(seq))
    mod_val = MOD_VALS.get(mod, 0)
    if mod_val == 0 and mod.startswith("KC_L"):
        # Map KC_LGUI (keycode 0xE3) to MOD_LGUI (0x08)
        mod_val = {0xE3:8,0xE7:0x18,0xE2:4,0xE6:0x14,0xE0:1,0xE4:0x11,0xE1:2,0xE5:0x12}.get(kc_val(mod), 0)
    return struct.pack('<BBBBH', s[0], s[1], s[2], mod_val, kc_val(key))

def unpack_leader(data):
    s0,s1,s2,m,k = struct.unpack_from('<BBBBH', data)
    mod_name = MOD_NAMES.get(m, kc_name(m))
    return [kc_name(s) for s in [s0,s1,s2] if s], mod_name, kc_name(k)

# ═══════════════════════════════════════════════════════════════════════════
# Mock device (fully functional for testing)
# ═══════════════════════════════════════════════════════════════════════════

class MockHIDDevice:
    def __init__(self):
        self.features = 0x7F  # all bits on initially
        self.tap = []
        self.combos = []
        self.leaders = []
        self._init_defaults()

    def _init_defaults(self):
        self.tap = [
            pack_tap_entry("KC_BSPC","KC_BSPC",DBL_KEYCODE,kc_val("KC_DEL")),
            pack_tap_entry("KC_ESC","KC_ESC",DBL_KEYCODE,kc_val("CW_TOGG")),
            pack_tap_entry("KC_E","KC_E",DBL_UNICODE_STR,0x20AC),
            pack_tap_entry("KC_2","KC_2",DBL_UNICODE_STR,ord('@')),
            pack_tap_entry("KC_GRV","KC_GRV",DBL_UNICODE_STR,ord('~')),
        ]
        self.combos = [
            pack_combo(["KC_O","KC_P"],"CUSTOM(12)"),
        ]
        self.leaders = [
            pack_leader(["KC_W"],"KC_LGUI","KC_W"),
            pack_leader(["KC_Q"],"KC_LGUI","KC_Q"),
        ]

    def connect(self): pass
    def close(self): pass

    def send(self, data: bytes) -> bytes:
        cmd = data[0] if data else 0
        chan = data[1] if len(data)>1 else 0
        vid = data[2] if len(data)>2 else 0
        idx = data[3] if len(data)>3 else 0
        payload = data[4:]

        if cmd == VIA_PROTOCOL_GET:   # read
            if vid == VALUE_FLAGS:
                return bytes([cmd, chan, vid, self.features] + [0]*28)
            elif vid == VALUE_TAP_COUNT:
                return bytes([cmd, chan, vid, len(self.tap)] + [0]*28)
            elif vid == VALUE_TAP_ENTRY and idx < len(self.tap):
                return bytes([cmd, chan, vid, idx]) + self.tap[idx] + bytes([0]*18)
            elif vid == VALUE_COMBO_COUNT:
                return bytes([cmd, chan, vid, len(self.combos)] + [0]*28)
            elif vid == VALUE_COMBO_ENTRY and idx < len(self.combos):
                return bytes([cmd, chan, vid, idx]) + self.combos[idx] + bytes([0]*18)
            elif vid == VALUE_LEADER_COUNT:
                return bytes([cmd, chan, vid, len(self.leaders)] + [0]*28)
            elif vid == VALUE_LEADER_ENTRY and idx < len(self.leaders):
                return bytes([cmd, chan, vid, idx]) + self.leaders[idx] + bytes([0]*22)

        elif cmd == VIA_PROTOCOL_SET:  # write
            # For count-type values, idx holds the count, payload is empty
            # For entry-type values, payload holds the entry data
            if vid == VALUE_FLAGS:
                self.features = idx
            elif vid == VALUE_TAP_COUNT:
                self.tap = [b'\x00'*10]*idx
            elif vid == VALUE_TAP_ENTRY and idx < len(self.tap) and len(payload)>=10:
                self.tap[idx] = bytes(payload[:10])
            elif vid == VALUE_COMBO_COUNT:
                self.combos = [b'\x00'*10]*idx
            elif vid == VALUE_COMBO_ENTRY and idx < len(self.combos) and len(payload)>=10:
                self.combos[idx] = bytes(payload[:10])
            elif vid == VALUE_LEADER_COUNT:
                self.leaders = [b'\x00'*6]*idx
            elif vid == VALUE_LEADER_ENTRY and idx < len(self.leaders) and len(payload)>=6:
                self.leaders[idx] = bytes(payload[:6])

        # save is a no-op on mock
        return bytes([cmd, chan, vid, idx] + [0]*28)


# ═══════════════════════════════════════════════════════════════════════════
# Real HID device
# ═══════════════════════════════════════════════════════════════════════════

class HIDDevice:
    def __init__(self):
        self._dev = None
    def connect(self):
        import hid
        for d in hid.enumerate(0,0):
            if d.get('usage_page')==0xFF60 and d.get('usage')==0x61:
                self._dev = hid.Device(path=d['path'])
                return
        raise ConnectionError("No QMK keyboard found")
    def send(self, data):
        # Use blocking reads with timeout - one transaction at a time
        report = data + b'\x00'*(32-len(data))
        self._dev.write(report)
        resp = self._dev.read(32, timeout=500)
        if resp is None:
            return b''
        return bytes(resp)
    def close(self):
        if self._dev: self._dev.close()


# ═══════════════════════════════════════════════════════════════════════════
# Export / Import
# ═══════════════════════════════════════════════════════════════════════════

def _read_flag(dev):
    r = dev.send(bytes([VIA_PROTOCOL_GET, VIA_CHANNEL, VALUE_FLAGS, 0]))
    return r[3] if len(r)>3 else 0

def _read_count(dev, vid):
    r = dev.send(bytes([VIA_PROTOCOL_GET, VIA_CHANNEL, vid, 0]))
    return r[3] if len(r)>3 else 0

def _read_entry(dev, vid, idx, size):
    r = dev.send(bytes([VIA_PROTOCOL_GET, VIA_CHANNEL, vid, idx]))
    return bytes(r[4:4+size]) if len(r)>=4+size else b'\x00'*size

def _write_flag(dev, val):
    dev.send(bytes([VIA_PROTOCOL_SET, VIA_CHANNEL, VALUE_FLAGS, val]))

def _write_count(dev, vid, count):
    dev.send(bytes([VIA_PROTOCOL_SET, VIA_CHANNEL, vid, count]))

def _write_entry(dev, vid, idx, data):
    dev.send(bytes([VIA_PROTOCOL_SET, VIA_CHANNEL, vid, idx]) + data)

def _save(dev):
    dev.send(bytes([VIA_PROTOCOL_SAVE, VIA_CHANNEL, 0, 0]))


def tap_to_json(data):
    b,t,d,v,x = unpack_tap_entry(data)
    if d == DBL_KEYCODE:  dt = "keycode"; dv = kc_name(v)
    elif d == DBL_UNICODE_STR:
        dt = "unicode"
        dv = (v.to_bytes(2,'little')+x.to_bytes(2,'little')).rstrip(b'\x00').decode('utf-8',errors='replace')
    else:  dt = "codepoint"; dv = f"U+{(v|x<<16):05X}"
    return {"base": kc_name(b), "tap": kc_name(t), "double": {"type": dt, "value": dv}}


def json_to_tap(j) -> bytes:
    b, t = kc_val(j["base"]), kc_val(j["tap"])
    d = j["double"]
    if d["type"] == "keycode":
        return pack_tap_entry(j["base"],j["tap"],DBL_KEYCODE,kc_val(d["value"]))
    elif d["type"] == "unicode":
        s = d["value"].encode('utf-8')[:4]
        v = int.from_bytes(s[:2].ljust(2,b'\x00'),'little')
        x = int.from_bytes(s[2:].ljust(2,b'\x00'),'little') if len(s)>2 else 0
        return pack_tap_entry(j["base"],j["tap"],DBL_UNICODE_STR,v,x)
    else:
        cp_str = d["value"].replace("U+","")
        cp = int(cp_str,16)
        return pack_tap_entry(j["base"],j["tap"],DBL_UNICODE_CP,cp&0xFFFF,(cp>>16)&0xFFFF)


def export_config(dev) -> dict:
    flags = _read_flag(dev)
    features = {name: bool(flags&(1<<b)) for b,name in FEATURE_MAP.items()}

    tap_count = _read_count(dev, VALUE_TAP_COUNT)
    tap = []
    for i in range(tap_count):
        data = _read_entry(dev, VALUE_TAP_ENTRY, i, 10)
        if any(data):  # non-empty
            tap.append(tap_to_json(data))

    combo_count = _read_count(dev, VALUE_COMBO_COUNT)
    combos = []
    for i in range(combo_count):
        data = _read_entry(dev, VALUE_COMBO_ENTRY, i, 10)
        ks, out = unpack_combo(data)
        if ks:
            combos.append({"keys":[kc_name(k) for k in ks], "output":kc_name(out)})

    leader_count = _read_count(dev, VALUE_LEADER_COUNT)
    leaders = []
    for i in range(leader_count):
        data = _read_entry(dev, VALUE_LEADER_ENTRY, i, 6)
        seq, mod, key = unpack_leader(data)
        if seq:
            leaders.append({"sequence":seq, "mod":mod, "key":key})

    return {"formatVersion":1, "featureFlags":features,
            "tapOverrides":tap, "combos":combos, "leaderSequences":leaders}


def import_config(dev, cfg: dict):
    flags = 0
    for b,name in FEATURE_MAP.items():
        if cfg.get("featureFlags",{}).get(name, False):
            flags |= (1<<b)
    _write_flag(dev, flags)

    tap = cfg.get("tapOverrides",[])
    _write_count(dev, VALUE_TAP_COUNT, len(tap))
    for i,t in enumerate(tap):
        _write_entry(dev, VALUE_TAP_ENTRY, i, json_to_tap(t))

    combos = cfg.get("combos",[])
    _write_count(dev, VALUE_COMBO_COUNT, len(combos))
    for i,c in enumerate(combos):
        ks = [kc_val(k) for k in c["keys"]]+[0]*(4-len(c["keys"]))
        _write_entry(dev, VALUE_COMBO_ENTRY, i, pack_combo(c["keys"],c["output"]))

    leaders = cfg.get("leaderSequences",[])
    _write_count(dev, VALUE_LEADER_COUNT, len(leaders))
    for i,l in enumerate(leaders):
        _write_entry(dev, VALUE_LEADER_ENTRY, i, pack_leader(l["sequence"],l["mod"],l["key"]))

    _save(dev)


# ═══════════════════════════════════════════════════════════════════════════
# CLI
# ═══════════════════════════════════════════════════════════════════════════

def main():
    ap = argparse.ArgumentParser(description="QMK Config Export/Import Tool")
    ap.add_argument("action", choices=["export","import","dump","debug"])
    ap.add_argument("file", nargs="?")
    ap.add_argument("--mock", action="store_true")
    args = ap.parse_args()

    dev = MockHIDDevice() if args.mock else HIDDevice()
    if not args.mock:
        try: dev.connect(); print("Connected.", file=sys.stderr)
        except Exception as e: print(f"Error: {e}", file=sys.stderr); sys.exit(1)

    try:
        if args.action == "export":
            cfg = export_config(dev)
            if args.file:
                with open(args.file,"w") as f: json.dump(cfg,f,indent=2)
                print(f"Wrote {args.file}", file=sys.stderr)
            else: print(json.dumps(cfg,indent=2))
        elif args.action == "import":
            if not args.file: print("Need file"); sys.exit(1)
            with open(args.file) as f: cfg = json.load(f)
            import_config(dev, cfg)
            print(f"Imported {args.file}", file=sys.stderr)
        elif args.action == "dump":
            print(json.dumps(export_config(dev),indent=2))
        elif args.action == "debug":
            # Send each HID command and show raw response
            for vid, name in [(1,"flags"),(2,"tap_count"),(4,"combo_count"),(6,"leader_count")]:
                r = dev.send(bytes([0x08, 0x00, vid, 0]))
                print(f"{name}: cmd={r[0]:02X} chan={r[1]:02X} vid={r[2]:02X} val={r[3]}")
            for vid, name in [(3,"tap_entry"),(5,"combo_entry"),(7,"leader_entry")]:
                for i in range(3):
                    r = dev.send(bytes([0x08, 0x00, vid, i]))
                    if len(r) >= 14:
                        pay = r[4:14].hex()
                        print(f"  {name}[{i}]: {pay}")
                    else:
                        print(f"  {name}[{i}]: short response ({len(r)} bytes): {r.hex()}")
    finally:
        dev.close()


if __name__ == "__main__":
    main()
