# Existing
ENCODER_MAP_ENABLE = yes
VIA_ENABLE = yes

# Layers & macros
LAYER_LOCK_ENABLE = yes
CAPS_WORD_ENABLE = yes

# On-the-fly macro recording (separate from VIA macros)
DYNAMIC_MACRO_ENABLE = yes

# Key overrides (e.g. shift+[ = {)
KEY_OVERRIDE_ENABLE = yes

# Repeat / Alt-Repeat keys
REPEAT_KEY_ENABLE = yes

# Tap-dance: multi-tap on a single key
TAP_DANCE_ENABLE = yes

# Combos: multiple simultaneous keys = action
COMBO_ENABLE = yes

# Long-press auto-shift
AUTO_SHIFT_ENABLE = yes

# Leader key sequences
LEADER_ENABLE = yes

# Unicode input
UNICODE_ENABLE = yes

# Link-time optimization — reduces flash usage
LTO_ENABLE = yes

# Custom modules for this keymap (feature toggles, indicators)
SRC += features.c
SRC += indicators.c
