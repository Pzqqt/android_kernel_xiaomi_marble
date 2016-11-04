/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

#include "targaddrs.h"
#include "cepci.h"
#include "regtable.h"
#include "ar6320def.h"
#include "ar6320v2def.h"
#include "hif_main.h"
#include "adrastea_reg_def.h"

#include "targetdef.h"
#include "hostdef.h"

void hif_target_register_tbl_attach(struct hif_softc *scn, u32 target_type)
{
	switch (target_type) {
	case TARGET_TYPE_AR6320:
		scn->targetdef = &ar6320_targetdef;
		scn->target_ce_def = &ar6320_ce_targetdef;
		break;
	case TARGET_TYPE_AR6320V2:
		scn->targetdef = &ar6320v2_targetdef;
		scn->target_ce_def = &ar6320v2_ce_targetdef;
		break;
	case TARGET_TYPE_ADRASTEA:
		scn->targetdef = &adrastea_targetdef;
		scn->target_ce_def = &adrastea_ce_targetdef;
		break;
#if defined(AR6002_HEADERS_DEF)
	case TARGET_TYPE_AR6002:
		scn->targetdef = AR6002_TARGETdef;
		break;
#endif
#if defined(AR6003_HEADERS_DEF)
	case TARGET_TYPE_AR6003:
		scn->targetdef = AR6003_TARGETdef;
		break;
#endif
#if defined(AR6004_HEADERS_DEF)
	case TARGET_TYPE_AR6004:
		scn->targetdef = AR6004_TARGETdef;
		break;
#endif
#if defined(AR9888_HEADERS_DEF)
	case TARGET_TYPE_AR9888:
		scn->targetdef = AR9888_TARGETdef;
		scn->target_ce_def = AR9888_CE_TARGETdef;
		break;
#endif
#if defined(AR9888V2_HEADERS_DEF)
	case TARGET_TYPE_AR9888V2:
		scn->targetdef = AR9888V2_TARGETdef;
		scn->target_ce_def = AR9888_CE_TARGETdef;
		break;
#endif
#if defined(AR900B_HEADERS_DEF)
	case TARGET_TYPE_AR900B:
		scn->targetdef = AR900B_TARGETdef;
		scn->target_ce_def = AR900B_CE_TARGETdef;
		break;
#endif
#if defined(QCA9984_HEADERS_DEF)
	case TARGET_TYPE_QCA9984:
		scn->targetdef = QCA9984_TARGETdef;
		scn->target_ce_def = QCA9984_CE_TARGETdef;
		break;
#endif
#if defined(QCA9888_HEADERS_DEF)
	case TARGET_TYPE_QCA9888:
		scn->targetdef = QCA9888_TARGETdef;
		scn->target_ce_def = QCA9888_CE_TARGETdef;
		break;
#endif
#ifdef ATH_AHB
#if defined(IPQ4019_HEADERS_DEF)
	case TARGET_TYPE_IPQ4019:
		scn->targetdef = IPQ4019_TARGETdef;
		scn->target_ce_def = IPQ4019_CE_TARGETdef;
		break;
#endif
#endif
#if defined(QCA8074_HEADERS_DEF)
	case TARGET_TYPE_QCA8074:
		scn->targetdef = QCA8074_TARGETdef;
		scn->target_ce_def = QCA8074_CE_TARGETdef;
		break;
#endif

	default:
		break;
	}
}

void hif_register_tbl_attach(struct hif_softc *scn, u32 hif_type)
{
	switch (hif_type) {
	case HIF_TYPE_AR6320V2:
		scn->hostdef = &ar6320v2_hostdef;
		break;
	case HIF_TYPE_ADRASTEA:
		scn->hostdef = &adrastea_hostdef;
		scn->host_shadow_regs = &adrastea_host_shadow_regs;
		break;
#if defined(AR6002_HEADERS_DEF)
	case HIF_TYPE_AR6002:
		scn->hostdef = AR6002_HOSTdef;
		break;
#endif
#if defined(AR6003_HEADERS_DEF)
	case HIF_TYPE_AR6003:
		scn->hostdef = AR6003_HOSTdef;
		break;
#endif
#if defined(AR6004_HEADERS_DEF)
	case HIF_TYPE_AR6004:
		scn->hostdef = AR6004_HOSTdef;
		break;
#endif
#if defined(AR9888_HEADERS_DEF)
	case HIF_TYPE_AR9888:
		scn->hostdef = AR9888_HOSTdef;
		break;
#endif
#if defined(AR9888V2_HEADERS_DEF)
	case HIF_TYPE_AR9888V2:
		scn->hostdef = AR9888V2_HOSTdef;
		break;
#endif
#if defined(AR900B_HEADERS_DEF)
	case HIF_TYPE_AR900B:
		scn->hostdef = AR900B_HOSTdef;
		break;
#endif
#if defined(QCA9984_HEADERS_DEF)
	case HIF_TYPE_QCA9984:
		scn->hostdef = QCA9984_HOSTdef;
		break;
#endif
#if defined(QCA9888_HEADERS_DEF)
	case HIF_TYPE_QCA9888:
		scn->hostdef = QCA9888_HOSTdef;
		break;
#endif

#ifdef ATH_AHB
#if defined(IPQ4019_HEADERS_DEF)
	case HIF_TYPE_IPQ4019:
		scn->hostdef = IPQ4019_HOSTdef;
		break;
#endif
#endif
#if defined(QCA8074_HEADERS_DEF)
	case HIF_TYPE_QCA8074:
		scn->hostdef = QCA8074_HOSTdef;
		break;
#endif

	default:
		break;
	}
}
