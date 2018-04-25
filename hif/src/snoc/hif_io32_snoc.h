/*
 * Copyright (c) 2015-2018 The Linux Foundation. All rights reserved.
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
 * DOC: hif_io32_snoc.h
 *
 * snoc specific implementations and configurations
 */

#ifndef __HIF_IO32_SNOC_H__
#define __HIF_IO32_SNOC_H__

#include "hif.h"
#include "regtable.h"
#include "ce_reg.h"
#include "qdf_atomic.h"
#include "hif_main.h"
#include "hif_debug.h"

static inline void ce_enable_irq_in_individual_register(struct hif_softc *scn,
		int ce_id)
{
	uint32_t offset;

	offset = HOST_IE_ADDRESS + CE_BASE_ADDRESS(ce_id);
	if (!TARGET_REGISTER_ACCESS_ALLOWED(scn)) {
		hif_err_rl("%s: target access is not allowed", __func__);
		return;
	}
	hif_write32_mb(scn->mem + offset, 1);
}

static inline void ce_disable_irq_in_individual_register(struct hif_softc *scn,
		int ce_id)
{
	uint32_t offset;

	offset = HOST_IE_ADDRESS + CE_BASE_ADDRESS(ce_id);
	if (!TARGET_REGISTER_ACCESS_ALLOWED(scn)) {
		hif_err_rl("%s: target access is not allowed", __func__);
		return;
	}
	hif_write32_mb(scn->mem + offset, 0);

	if (!TARGET_REGISTER_ACCESS_ALLOWED(scn)) {
		hif_err_rl("%s: target access is not allowed", __func__);
		return;
	}
	hif_read32_mb(scn->mem + offset);
}
#endif
