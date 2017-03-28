/*
 * Copyright (c) 2013-2014, 2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef __SYS_GLOBAL_H__
#define __SYS_GLOBAL_H__

typedef struct sAniSirSys {

	uint32_t gSysFrameCount[4][16];
	uint32_t gSysBbtReceived;
	uint32_t sys_bbt_pending_mgmt_count;
	uint32_t gSysBbtPostedToLim;
	uint32_t gSysBbtPostedToSch;
	uint32_t gSysBbtPostedToPmm;
	uint32_t gSysBbtPostedToHal;
	uint32_t gSysBbtDropped;
	uint32_t gSysBbtNonLearnFrameInv;
	uint32_t gSysBbtLearnFrameInv;
	uint32_t gSysBbtCrcFail;
	uint32_t gSysBbtDuplicates;
	uint32_t gSysReleaseCount;
	uint32_t probeError, probeBadSsid, probeIgnore, probeRespond;

	uint32_t gSysEnableLearnMode;
	uint32_t gSysEnableScanMode;
	uint32_t gSysEnableLinkMonitorMode;
	qdf_spinlock_t bbt_mgmt_lock;
} tAniSirSys, *tpAniSirSys;

#endif
