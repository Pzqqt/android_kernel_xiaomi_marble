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

#ifndef _CDP_TXRX_OCB_H_
#define _CDP_TXRX_OCB_H_
#include <cdp_txrx_mob_def.h>
#include "cdp_txrx_handle.h"
/**
 * cdp_set_ocb_chan_info() - set OCB channel info to vdev.
 * @soc - data path soc handle
 * @vdev: vdev handle
 * @ocb_set_chan: OCB channel information to be set in vdev.
 *
 * Return: NONE
 */
static inline void
cdp_set_ocb_chan_info(ol_txrx_soc_handle soc, struct cdp_vdev *vdev,
		struct ol_txrx_ocb_set_chan ocb_set_chan)
{
	if (!soc || !soc->ops || !soc->ops->ocb_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->ocb_ops->set_ocb_chan_info)
		soc->ops->ocb_ops->set_ocb_chan_info(vdev,
			ocb_set_chan);

}
/**
 * cdp_get_ocb_chan_info() - return handle to vdev ocb_channel_info
 * @soc - data path soc handle
 * @vdev: vdev handle
 *
 * Return: handle to struct ol_txrx_ocb_chan_info
 */
static inline struct ol_txrx_ocb_chan_info *
cdp_get_ocb_chan_info(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops || !soc->ops->ocb_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return NULL;
	}

	if (soc->ops->ocb_ops->get_ocb_chan_info)
		return soc->ops->ocb_ops->get_ocb_chan_info(vdev);

	return NULL;
}
#endif /* _CDP_TXRX_OCB_H_ */
