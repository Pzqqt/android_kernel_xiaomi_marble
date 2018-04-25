/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_raw.h
 * @brief Define the host data path raw mode API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_RAW_H_
#define _CDP_TXRX_RAW_H_

#include "cdp_txrx_handle.h"
#include "cdp_txrx_ops.h"
/* TODO: adf need to be replaced with qdf */
static inline int cdp_get_nwifi_mode(ol_txrx_soc_handle soc,
	struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->raw_ops ||
	    !soc->ops->raw_ops->txrx_get_nwifi_mode)
		return 0;

	return soc->ops->raw_ops->txrx_get_nwifi_mode(vdev);
}

/**
 * @brief finds the ast entry for the packet
 * @details: Finds the ast entry i.e 4th address for the packet based on the
 *               details in the netbuf.
 *
 * @param vdev - the data virtual device object
 * @param pnbuf - pointer to nbuf
 * @param raw_ast - pointer to fill ast information
 *
 * @return - 0 on success, -1 on error, 1 if more nbufs need to be consumed.
 */

static inline void
cdp_rawsim_get_astentry (ol_txrx_soc_handle soc, struct cdp_vdev *vdev,
			qdf_nbuf_t *pnbuf, struct cdp_raw_ast *raw_ast)
{

	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_CDP, QDF_TRACE_LEVEL_DEBUG,
				"%s: Invalid Instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->raw_ops ||
	    !soc->ops->raw_ops->rsim_get_astentry)
		return;

	soc->ops->raw_ops->rsim_get_astentry(vdev, pnbuf, raw_ast);
}

#endif
