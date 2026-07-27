KEYCHRON_ENABLE = yes

ifeq ($(strip $(KEYCHRON_ENABLE)), yes)
include keyboards/keychron/common/keychron_common.mk
endif