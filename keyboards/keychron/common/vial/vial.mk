# Vial support — included from a keymap's rules.mk AFTER VIAL_ENABLE is set.
#
# Pulls in vial.c, exposes vial.h headers, and wires the include path so
# `#include "vial.h"` / `"qmk_settings.h"` / `"vial_generated_keyboard_definition.h"`
# resolve from keyboards/keychron/common/vial/.

VIAL_KEYCHRON_DIR := $(KEYCHRON_COMMON_DIR)/vial

OPT_DEFS += -DVIAL_ENABLE
SRC += $(VIAL_KEYCHRON_DIR)/vial.c
VPATH += $(VIAL_KEYCHRON_DIR)
