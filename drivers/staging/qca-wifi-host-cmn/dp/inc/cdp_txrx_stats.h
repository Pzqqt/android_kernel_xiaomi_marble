/*
 * Copyright (c) 2016-2017,2019,2021 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_stats.h
 * @brief Define the host data path statistics API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_STATS_H_
#define _CDP_TXRX_STATS_H_
#include <cdp_txrx_ops.h>
#include <cdp_txrx_cmn.h>

static inline QDF_STATUS
cdp_clear_stats(ol_txrx_soc_handle soc, uint8_t pdev_id, uint8_t bitmap)
{

	if (!soc || !soc->ops) {
		dp_cdp_debug("Invalid Instance");
		QDF_BUG(0);
		return QDF_STATUS_E_INVAL;
	}

	if (!soc->ops->mob_stats_ops ||
	    !soc->ops->mob_stats_ops->clear_stats)
		return QDF_STATUS_E_INVAL;

	return soc->ops->mob_stats_ops->clear_stats(soc, pdev_id, bitmap);
}

static inline int
cdp_stats(ol_txrx_soc_handle soc, uint8_t vdev_id, char *buffer,
		unsigned int buf_len)
{
	if (!soc || !soc->ops) {
		dp_cdp_debug("Invalid Instance");
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->mob_stats_ops ||
	    !soc->ops->mob_stats_ops->stats)
		return 0;

	return soc->ops->mob_stats_ops->stats(vdev_id, buffer, buf_len);
}

#endif /* _CDP_TXRX_STATS_H_ */
