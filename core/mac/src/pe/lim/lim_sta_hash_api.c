/*
 * Copyright (c) 2011-2012, 2017-2018 The Linux Foundation. All rights reserved.
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
 *
 * lim_sta_hash_api.c: Provides access functions to get/set values of station hash entry fields.
 * Author:    Sunit Bhatia
 * Date:       09/19/2006
 * History:-
 * Date        Modified by            Modification Information
 *
 * --------------------------------------------------------------------------
 *
 */

#include "lim_sta_hash_api.h"

/**
 * lim_get_sta_hash_bssidx()
 *
 ***FUNCTION:
 * This function is called to Get the Bss Index of the currently associated Station.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param pMac  pointer to Global Mac structure.
 * @param assocId AssocID of the Station.
 * @param bssidx pointer to the bss index, which will be returned by the function.
 *
 * @return success if GET operation is ok, else Failure.
 */

QDF_STATUS lim_get_sta_hash_bssidx(tpAniSirGlobal pMac, uint16_t assocId,
				      uint8_t *bssidx, tpPESession psessionEntry)
{
	tpDphHashNode pSta =
		dph_get_hash_entry(pMac, assocId, &psessionEntry->dph.dphHashTable);

	if (pSta == NULL) {
		pe_err("invalid STA: %d", assocId);
		return QDF_STATUS_E_NOENT;
	}

	*bssidx = (uint8_t) pSta->bssId;
	return QDF_STATUS_SUCCESS;
}
