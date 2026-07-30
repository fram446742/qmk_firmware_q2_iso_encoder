# Existing
ENCODER_MAP_ENABLE = yes
VIA_ENABLE = yes

# Layers & macros
LAYER_LOCK_ENABLE = yes
CAPS_WORD_ENABLE = yes

# On-the-fly macro recording (separate from VIA macros)
DYNAMIC_MACRO_ENABLE = yes

# Key overrides (e.g. shift+[ = {)
# Repeat / Alt-Repeat keys
REPEAT_KEY_ENABLE = yes

# Combos: multiple simultaneous keys = action
COMBO_ENABLE = yes

# Long-press auto-shift
AUTO_SHIFT_ENABLE = yes

# Auto-correct (typo correction via compile-time trie dictionary)
AUTOCORRECT_ENABLE = yes

# Leader key sequences
LEADER_ENABLE = yes

# Unicode input
UNICODE_ENABLE = yes

# Link-time optimization — reduces flash usage
LTO_ENABLE = yes

# Custom modules for this keymap (feature toggles, indicators)
SRC += features.c
SRC += indicators.c

# Auto-generate autocorrect_data.h + key_positions.h at build time
DUMMY := $(shell $(QMK_BIN) generate-autocorrect-data $(KEYMAP_PATH)/typos.txt -o $(KEYMAP_PATH)/autocorrect_data.h 2>/dev/null)
DUMMY := $(shell python3 $(KEYMAP_PATH)/gen_key_positions.py 2>/dev/null)

