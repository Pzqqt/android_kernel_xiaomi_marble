/*
 * Copyright (c) 2013,2016 The Linux Foundation. All rights reserved.
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

#if defined(AR6004_HEADERS_DEF)
#define AR6004 1

#define WLAN_HEADERS 1
#include "common_drv.h"
#include "AR6004/hw/apb_map.h"
#include "AR6004/hw/gpio_reg.h"
#include "AR6004/hw/rtc_reg.h"
#include "AR6004/hw/si_reg.h"
#include "AR6004/hw/mbox_reg.h"
#include "AR6004/hw/mbox_wlan_host_reg.h"

#define SYSTEM_SLEEP_OFFSET     SOC_SYSTEM_SLEEP_OFFSET
#define SCRATCH_BASE_ADDRESS    MBOX_BASE_ADDRESS

#define MY_TARGET_DEF AR6004_TARGETdef
#define MY_HOST_DEF AR6004_HOSTdef
#define MY_CEREG_DEF AR6004_CE_TARGETdef
#define MY_TARGET_BOARD_DATA_SZ AR6004_BOARD_DATA_SZ
#define MY_TARGET_BOARD_EXT_DATA_SZ AR6004_BOARD_EXT_DATA_SZ
#include "targetdef.h"
#include "hostdef.h"
#else
#include "common_drv.h"
#include "targetdef.h"
#include "hostdef.h"
struct targetdef_s *AR6004_TARGETdef;
struct hostdef_s *AR6004_HOSTdef;
#endif /*AR6004_HEADERS_DEF */
