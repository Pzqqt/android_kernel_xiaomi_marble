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
#include "ar9888def.h"
#include "ar6320def.h"
#include "ar6320v2def.h"
#include "hif_main.h"
#include "adrastea_reg_def.h"

void target_register_tbl_attach(struct hif_softc *scn, u32 target_type)
{
	switch (target_type) {
	case TARGET_TYPE_AR9888:
		scn->targetdef = &ar9888_targetdef;
		scn->target_ce_def = &ar9888_ce_targetdef;
		break;
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
	default:
		break;
	}
}

void hif_register_tbl_attach(struct hif_softc *scn, u32 hif_type)
{
	switch (hif_type) {
	case HIF_TYPE_AR9888:
		scn->hostdef = &ar9888_hostdef;
		break;
	case HIF_TYPE_AR6320:
		scn->hostdef = &ar6320_hostdef;
		break;
	case HIF_TYPE_AR6320V2:
		scn->hostdef = &ar6320v2_hostdef;
		break;
	case HIF_TYPE_ADRASTEA:
		scn->hostdef = &adrastea_hostdef;
		scn->host_shadow_regs = &adrastea_host_shadow_regs;
		break;
	default:
		break;
	}
}
