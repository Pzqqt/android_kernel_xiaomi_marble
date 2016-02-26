/*
 * Copyright (c) 2011-2015 The Linux Foundation. All rights reserved.
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

/*
 *
 * Author:      Sandesh Goel
 * Date:        02/09/03
 * History:-
 * 04/09/02        Created.
 * --------------------------------------------------------------------
 *
 */

#ifndef __CFGGLOBAL_H
#define __CFGGLOBAL_H

#include "sir_common.h"
#include "sir_types.h"
#include "wni_cfg.h"

#define CFG_MAX_NUM_STA      SIR_MAX_NUM_STA_IN_IBSS

#define CFG_MAX_STATIC_STRING   70
/* as the number of channels grows, 128 is not big enough */
#define CFG_MAX_STR_LEN       256

/*--------------------------------------------------------------------*/
/* Configuration Control Structure                                    */
/*--------------------------------------------------------------------*/
typedef struct {
	uint32_t control;
} tCfgCtl;


typedef struct sAniSirCfgStaticString {
	uint16_t cfgId;
	uint8_t  maxLen;
	uint8_t  length;
	uint8_t  data[255];
} cfgstatic_string;

typedef struct sAniSirCfgStatic {
	uint16_t cfgId;
	uint32_t control;
	uint32_t cfgIMin;
	uint32_t cfgIMax;
	uint32_t cfgIVal;
	void     *pStrData;
} cgstatic;

typedef struct sAniSirCfg {
	/* CFG module status */
	uint8_t gCfgStatus;

	tCfgCtl *gCfgEntry;

	uint8_t *gCfgSBuf;
	uint32_t *gCfgIBuf;
	uint32_t *gCfgIBufMin;
	uint32_t *gCfgIBufMax;

	uint16_t gCfgMaxIBufSize;
	uint16_t gCfgMaxSBufSize;

	/* Static buffer for string parameter (must be word-aligned) */
	uint8_t *gSBuffer;

	/* Message param list buffer (enough for largest possible response) */
	uint32_t *gParamList;
} tAniSirCfg, *tpAniSirCfg;

#endif
