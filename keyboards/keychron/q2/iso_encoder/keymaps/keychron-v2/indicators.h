/* Copyright 2025 Franc
 * SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdint.h>
#include <stdbool.h>

// ═════════════════════════════════════════════════════════════════════════════
// Feature overview  —  interactive mode toggled by O+P combo
// ═════════════════════════════════════════════════════════════════════════════
// LED indices and timeouts are defined in keymap_config.h.

void feature_overview_trigger(void);
bool feature_overview_is_active(void);
void feature_overview_cancel(void);
void feature_overview_reset_timer(void);

void indicator_draw(void);
void indicator_task(void);
