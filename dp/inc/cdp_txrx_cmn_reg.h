/*
 * Copyright (c) 2011-2019 The Linux Foundation. All rights reserved.
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

#include "hif_main.h"

#define MOB_DRV_LEGACY_DP	0xdeed/*FIXME Add MCL device IDs */
#define LITHIUM_DP		0xfffd/*FIXME Add Litium device ID */
/* Use these device IDs for attach in future */

#if defined(DP_TXRX_SOC_ATTACH)
static inline ol_txrx_soc_handle
ol_txrx_soc_attach(void *scn_handle, struct ol_if_ops *dp_ol_if_ops)
{
	return NULL;
}
#else
ol_txrx_soc_handle
ol_txrx_soc_attach(void *scn_handle, struct ol_if_ops *dp_ol_if_ops);
#endif

/**
 * dp_soc_attach_wifi3() - Attach txrx SOC
 * @ctrl_psoc:	Opaque SOC handle from Ctrl plane
 * @htc_handle:	Opaque HTC handle
 * @hif_handle:	Opaque HIF handle
 * @qdf_osdev:	QDF device
 * @ol_ops:	Offload Operations
 * @device_id:	Device ID
 *
 * Return: DP SOC handle on success, NULL on failure
 */

/**
 * dp_soc_init_wifi3() - Initialize txrx SOC
 * @soc: Opaque DP SOC handle
 * @ctrl_psoc: Opaque SOC handle from control plane
 * @hif_handle: Opaque HIF handle
 * @htc_handle: Opaque HTC handle
 * @qdf_osdev: QDF device
 * @ol_ops: Offload Operations
 * @device_id: Device ID
 *
 * Return: DP SOC handle on success, NULL on failure
 */
#if defined(QCA_WIFI_QCA8074) || defined(QCA_WIFI_QCA6018)
struct cdp_soc_t *
dp_soc_attach_wifi3(struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
		    struct hif_opaque_softc *hif_handle,
		    HTC_HANDLE htc_handle, qdf_device_t qdf_osdev,
		    struct ol_if_ops *ol_ops, uint16_t device_id);
void *dp_soc_init_wifi3(struct cdp_soc_t *soc,
			struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
			struct hif_opaque_softc *hif_handle,
			HTC_HANDLE htc_handle, qdf_device_t qdf_osdev,
			struct ol_if_ops *ol_ops, uint16_t device_id);
#else
static inline
struct cdp_soc_t *dp_soc_attach_wifi3(struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
				      struct hif_opaque_softc *hif_handle,
				      HTC_HANDLE htc_handle,
				      qdf_device_t qdf_osdev,
				      struct ol_if_ops *ol_ops,
				      uint16_t device_id)
{
	return NULL;
}

static inline
void *dp_soc_init_wifi3(struct cdp_soc_t *soc,
			struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
			struct hif_opaque_softc *hif_handle,
			HTC_HANDLE htc_handle, qdf_device_t qdf_osdev,
			struct ol_if_ops *ol_ops, uint16_t device_id)
{
	return NULL;
}
#endif /* QCA_WIFI_QCA8074 */

static inline ol_txrx_soc_handle cdp_soc_attach(
		u_int16_t devid, void *hif_handle, void *psoc,
		HTC_HANDLE htc_handle, qdf_device_t qdf_dev,
		struct ol_if_ops *dp_ol_if_ops)
{
	switch (devid) {
	case LITHIUM_DP: /*FIXME Add lithium devide IDs */
	case QCA8074_DEVICE_ID: /* Hawekeye */
	case QCA8074V2_DEVICE_ID: /* Hawekeye V2*/
	case QCA6290_DEVICE_ID:
	case QCN9000_DEVICE_ID:
	case QCA6390_DEVICE_ID:
	case QCA6490_DEVICE_ID:
	case QCA6390_EMULATION_DEVICE_ID:
	case RUMIM2M_DEVICE_ID_NODE0: /*lithium emulation */
	case RUMIM2M_DEVICE_ID_NODE1: /*lithium emulation */
	case RUMIM2M_DEVICE_ID_NODE2: /*lithium emulation */
	case RUMIM2M_DEVICE_ID_NODE3: /*lithium emulation */
	case RUMIM2M_DEVICE_ID_NODE4: /*lithium emulation */
	case RUMIM2M_DEVICE_ID_NODE5: /*lithium emulation */
		return dp_soc_attach_wifi3(psoc, hif_handle, htc_handle,
			qdf_dev, dp_ol_if_ops, devid);
	break;
	default:
		return ol_txrx_soc_attach(psoc, dp_ol_if_ops);
	}
	return NULL;
}

#endif /*_CDP_TXRX_CMN_REG_H_ */
