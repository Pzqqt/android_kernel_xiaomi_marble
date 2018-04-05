/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: This file contains centralized definitions of SCAN component
 */
#ifndef __CONFIG_SCAN_H
#define __CONFIG_SCAN_H

#include "cfg_define.h"

#define CFG_ACTIVE_MAX_CHANNEL_TIME CFG_INI_UINT(\
		"gActiveMaxChannelTime",\
		0, 10000, 40,\
		CFG_VALUE_OR_DEFAULT, "active dwell time")

#define CFG_PASSIVE_MAX_CHANNEL_TIME CFG_INI_UINT(\
		"gPassiveMaxChannelTime",\
		0, 10000, 100,\
		CFG_VALUE_OR_DEFAULT, "passive dwell time")

#define CFG_SCAN_ALL \
	CFG(CFG_ACTIVE_MAX_CHANNEL_TIME) \
	CFG(CFG_PASSIVE_MAX_CHANNEL_TIME)

#endif
