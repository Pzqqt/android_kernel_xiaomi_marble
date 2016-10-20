/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_cmn.h
 * @brief Define the host data path converged API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_CMN_REG_H_
#define _CDP_TXRX_CMN_REG_H_

#define IPQ4019_DEVICE_ID	0x12ef
#define AR900B_DEVICE_ID	0x0040
#define QCA9984_DEVICE_ID	0x0046
#define QCA9888_DEVICE_ID	0x0056
#define MOB_DRV_LEGACY_DP	0xffff/*FIXME Add MCL device IDs */
#define LITHIUM_DP		0xfffe/*FIXME Add Litium device ID */
/* Use these device IDs for attach in future */

ol_txrx_soc_handle ol_txrx_soc_attach(struct ol_if_ops *dp_ol_if_ops);

static inline ol_txrx_soc_handle cdp_soc_attach(u_int16_t devid,
	void *hif_handle, void *scn, void *htc_handle, qdf_device_t *qdf_dev,
	struct ol_if_ops *dp_ol_if_ops)
{
	switch (devid) {
	case LITHIUM_DP: /*FIXME Add lithium devide IDs */
		return NULL;
	break;
	default:
		return ol_txrx_soc_attach(dp_ol_if_ops);
	}
	return NULL;
}

#endif /*_CDP_TXRX_CMN_REG_H_ */
