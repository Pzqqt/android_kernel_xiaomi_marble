/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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
 /**
 * @file cdp_txrx_cmn.h
 * @brief Define the host data path converged API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_CMN_H_
#define _CDP_TXRX_CMN_H_

#include "qdf_types.h"
#include "qdf_nbuf.h"
#include "cdp_txrx_ops.h"
/******************************************************************************
 *
 * Common Data Path Header File
 *
 *****************************************************************************/

static inline int
cdp_soc_attach_target(ol_txrx_soc_handle soc)
{
	if (soc->ops->cmn_drv_ops->txrx_soc_attach_target)
		return soc->ops->cmn_drv_ops->txrx_soc_attach_target(soc);
	return 0;
}

static inline void *
cdp_vdev_attach(ol_txrx_soc_handle soc, void *pdev,
	uint8_t *vdev_mac_addr, uint8_t vdev_id, enum wlan_op_mode op_mode)
{
	if (soc->ops->cmn_drv_ops->txrx_vdev_attach)
		return soc->ops->cmn_drv_ops->txrx_vdev_attach(pdev,
			vdev_mac_addr, vdev_id, op_mode);
	return NULL;
}

static inline void
cdp_vdev_detach(ol_txrx_soc_handle soc, void *vdev,
	 ol_txrx_vdev_delete_cb callback, void *cb_context)
{
	if (soc->ops->cmn_drv_ops->txrx_vdev_detach)
		return soc->ops->cmn_drv_ops->txrx_vdev_detach(vdev,
			callback, cb_context);
	return;
}

static inline int
cdp_pdev_attach_target(ol_txrx_soc_handle soc, void *pdev)
{
	if (soc->ops->cmn_drv_ops->txrx_pdev_attach_target)
		return soc->ops->cmn_drv_ops->txrx_pdev_attach_target(pdev);
	return 0;
}

static inline void *cdp_pdev_attach
	(ol_txrx_soc_handle soc, void *ctrl_pdev,
	HTC_HANDLE htc_pdev, qdf_device_t osdev, uint8_t pdev_id)
{
	if (soc->ops->cmn_drv_ops->txrx_pdev_attach)
		return soc->ops->cmn_drv_ops->txrx_pdev_attach(soc, ctrl_pdev,
			htc_pdev, osdev, pdev_id);
	return NULL;
}

static inline int cdp_pdev_post_attach(ol_txrx_soc_handle soc, void *pdev)
{
	if (soc->ops->cmn_drv_ops->txrx_pdev_post_attach)
		return soc->ops->cmn_drv_ops->txrx_pdev_post_attach(pdev);
	return 0;
}

static inline void
cdp_pdev_detach(ol_txrx_soc_handle soc, void *pdev, int force)
{
	if (soc->ops->cmn_drv_ops->txrx_pdev_detach)
		return soc->ops->cmn_drv_ops->txrx_pdev_detach(pdev, force);
	return;
}

static inline void *cdp_peer_create
	(ol_txrx_soc_handle soc, void *vdev,
	uint8_t *peer_mac_addr)
{
	if (soc->ops->cmn_drv_ops->txrx_peer_create)
		return soc->ops->cmn_drv_ops->txrx_peer_create(vdev,
			peer_mac_addr);
	return NULL;
}

static inline void cdp_peer_setup
	(ol_txrx_soc_handle soc, void *vdev, void *peer)
{
	if (soc->ops->cmn_drv_ops->txrx_peer_setup)
		return soc->ops->cmn_drv_ops->txrx_peer_setup(vdev,
			peer);
	return;
}

static inline void cdp_peer_teardown
	(ol_txrx_soc_handle soc, void *vdev, void *peer)
{
	if (soc->ops->cmn_drv_ops->txrx_peer_teardown)
		return soc->ops->cmn_drv_ops->txrx_peer_teardown(vdev,
			peer);
	return;
}

static inline void
cdp_peer_delete(ol_txrx_soc_handle soc, void *peer)
{
	if (soc->ops->cmn_drv_ops->txrx_peer_delete)
		return soc->ops->cmn_drv_ops->txrx_peer_delete(peer);
	return;
}

static inline int
cdp_set_monitor_mode(ol_txrx_soc_handle soc, void *vdev)
{
	if (soc->ops->cmn_drv_ops->txrx_set_monitor_mode)
		return soc->ops->cmn_drv_ops->txrx_set_monitor_mode(vdev);
	return 0;
}

static inline void
cdp_set_curchan(ol_txrx_soc_handle soc,
	void *pdev,
	uint32_t chan_mhz)
{
	if (soc->ops->cmn_drv_ops->txrx_set_curchan)
		return soc->ops->cmn_drv_ops->txrx_set_curchan(pdev, chan_mhz);
	return;
}

static inline void
cdp_set_privacy_filters(ol_txrx_soc_handle soc, void *vdev,
			 void *filter, uint32_t num)
{
	if (soc->ops->cmn_drv_ops->txrx_set_privacy_filters)
		return soc->ops->cmn_drv_ops->txrx_set_privacy_filters(vdev,
			filter, num);
	return;
}

/******************************************************************************
 * Data Interface (B Interface)
 *****************************************************************************/
static inline void
cdp_vdev_register(ol_txrx_soc_handle soc, void *vdev,
	 void *osif_vdev, struct ol_txrx_ops *txrx_ops)
{
	if (soc->ops->cmn_drv_ops->txrx_vdev_register)
		return soc->ops->cmn_drv_ops->txrx_vdev_register(vdev,
			osif_vdev, txrx_ops);
	return;
}

static inline int
cdp_mgmt_send(ol_txrx_soc_handle soc, void *vdev,
	qdf_nbuf_t tx_mgmt_frm,	uint8_t type)
{
	if (soc->ops->cmn_drv_ops->txrx_mgmt_send)
		return soc->ops->cmn_drv_ops->txrx_mgmt_send(vdev,
			tx_mgmt_frm, type);
	return 0;
}

static inline int
cdp_mgmt_send_ext(ol_txrx_soc_handle soc, void *vdev,
	 qdf_nbuf_t tx_mgmt_frm, uint8_t type,
	 uint8_t use_6mbps, uint16_t chanfreq)
{
	if (soc->ops->cmn_drv_ops->txrx_mgmt_send_ext)
		return soc->ops->cmn_drv_ops->txrx_mgmt_send_ext
			(vdev, tx_mgmt_frm, type, use_6mbps, chanfreq);
	return 0;
}


static inline void
cdp_mgmt_tx_cb_set(ol_txrx_soc_handle soc, void *pdev,
			 uint8_t type,
			 ol_txrx_mgmt_tx_cb download_cb,
			 ol_txrx_mgmt_tx_cb ota_ack_cb, void *ctxt)
{
	if (soc->ops->cmn_drv_ops->txrx_mgmt_tx_cb_set)
		return soc->ops->cmn_drv_ops->txrx_mgmt_tx_cb_set
			(pdev, type, download_cb, ota_ack_cb, ctxt);
	return;
}

static inline int cdp_get_tx_pending(ol_txrx_soc_handle soc,
void *pdev)
{
	if (soc->ops->cmn_drv_ops->txrx_get_tx_pending)
		return soc->ops->cmn_drv_ops->txrx_get_tx_pending(pdev);
	return 0;
}

static inline void
cdp_data_tx_cb_set(ol_txrx_soc_handle soc, void *data_vdev,
		 ol_txrx_data_tx_cb callback, void *ctxt)
{
	if (soc->ops->cmn_drv_ops->txrx_data_tx_cb_set)
		return soc->ops->cmn_drv_ops->txrx_data_tx_cb_set(data_vdev,
			callback, ctxt);
	return;
}

/******************************************************************************
 * Statistics and Debugging Interface (C Inteface)
 *****************************************************************************/

static inline int
cdp_aggr_cfg(ol_txrx_soc_handle soc, void *vdev,
			 int max_subfrms_ampdu,
			 int max_subfrms_amsdu)
{
	if (soc->ops->cmn_drv_ops->txrx_aggr_cfg)
		return soc->ops->cmn_drv_ops->txrx_aggr_cfg(vdev,
			max_subfrms_ampdu, max_subfrms_amsdu);
	return 0;
}

static inline int
cdp_fw_stats_get(ol_txrx_soc_handle soc, void *vdev,
	struct ol_txrx_stats_req *req, bool per_vdev,
	bool response_expected)
{
	if (soc->ops->cmn_drv_ops->txrx_fw_stats_get)
		return soc->ops->cmn_drv_ops->txrx_fw_stats_get(vdev, req,
			per_vdev, response_expected);
	return 0;
}

static inline int
cdp_debug(ol_txrx_soc_handle soc, void *vdev, int debug_specs)
{
	if (soc->ops->cmn_drv_ops->txrx_debug)
		return soc->ops->cmn_drv_ops->txrx_debug(vdev, debug_specs);
	return 0;
}

static inline void cdp_fw_stats_cfg(ol_txrx_soc_handle soc,
	 void *vdev, uint8_t cfg_stats_type, uint32_t cfg_val)
{
	if (soc->ops->cmn_drv_ops->txrx_fw_stats_cfg)
		return soc->ops->cmn_drv_ops->txrx_fw_stats_cfg(vdev,
			cfg_stats_type, cfg_val);
	return;
}

static inline void cdp_print_level_set(ol_txrx_soc_handle soc, unsigned level)
{
	if (soc->ops->cmn_drv_ops->txrx_print_level_set)
		return soc->ops->cmn_drv_ops->txrx_print_level_set(level);
	return;
}

static inline uint8_t *
cdp_get_vdev_mac_addr(ol_txrx_soc_handle soc, void *vdev)
{
	if (soc->ops->cmn_drv_ops->txrx_get_vdev_mac_addr)
		return soc->ops->cmn_drv_ops->txrx_get_vdev_mac_addr(vdev);
	return NULL;
}

/**
 * cdp_get_vdev_struct_mac_addr() - Return handle to struct qdf_mac_addr of
 * vdev
 * @vdev: vdev handle
 *
 * Return: Handle to struct qdf_mac_addr
 */
static inline struct qdf_mac_addr *cdp_get_vdev_struct_mac_addr
	(ol_txrx_soc_handle soc, void *vdev)
{
	if (soc->ops->cmn_drv_ops->txrx_get_vdev_struct_mac_addr)
		return soc->ops->cmn_drv_ops->txrx_get_vdev_struct_mac_addr
			(vdev);
	return NULL;
}

/**
 * cdp_get_pdev_from_vdev() - Return handle to pdev of vdev
 * @vdev: vdev handle
 *
 * Return: Handle to pdev
 */
static inline void *cdp_get_pdev_from_vdev
	(ol_txrx_soc_handle soc, void *vdev)
{
	if (soc->ops->cmn_drv_ops->txrx_get_pdev_from_vdev)
		return soc->ops->cmn_drv_ops->txrx_get_pdev_from_vdev(vdev);
	return NULL;
}

/**
 * cdp_get_ctrl_pdev_from_vdev() - Return control pdev of vdev
 * @vdev: vdev handle
 *
 * Return: Handle to control pdev
 */
static inline void *
cdp_get_ctrl_pdev_from_vdev(ol_txrx_soc_handle soc, void *vdev)
{
	if (soc->ops->cmn_drv_ops->txrx_get_ctrl_pdev_from_vdev)
		return soc->ops->cmn_drv_ops->txrx_get_ctrl_pdev_from_vdev
			(vdev);
	return NULL;
}

static inline void *
cdp_get_vdev_from_vdev_id(ol_txrx_soc_handle soc, void *pdev,
		uint8_t vdev_id)
{
	if (soc->ops->cmn_drv_ops->txrx_get_vdev_from_vdev_id)
		return soc->ops->cmn_drv_ops->txrx_get_vdev_from_vdev_id
			(pdev, vdev_id);
	return NULL;
}

static inline void
cdp_soc_detach(ol_txrx_soc_handle soc)
{
	if (soc->ops->cmn_drv_ops->txrx_soc_detach)
		return soc->ops->cmn_drv_ops->txrx_soc_detach((void *)soc);
	return;
}

static inline int cdp_addba_requestprocess(ol_txrx_soc_handle soc,
	void *peer_handle, uint8_t dialogtoken, uint16_t tid,
	uint16_t batimeout, uint16_t buffersize, uint16_t startseqnum)
{
	if (soc->ops->cmn_drv_ops->addba_requestprocess)
		return soc->ops->cmn_drv_ops->addba_requestprocess(peer_handle,
			dialogtoken, tid, batimeout, buffersize, startseqnum);
	return 0;
}

static inline void cdp_addba_responsesetup(ol_txrx_soc_handle soc,
	void *peer_handle, uint8_t tid, uint8_t *dialogtoken,
	uint16_t *statuscode, uint16_t *buffersize, uint16_t *batimeout)
{
	if (soc->ops->cmn_drv_ops->addba_responsesetup)
		soc->ops->cmn_drv_ops->addba_responsesetup(peer_handle, tid,
			dialogtoken, statuscode, buffersize, batimeout);
}

static inline int cdp_delba_process(ol_txrx_soc_handle soc,
	void *peer_handle, int tid, uint16_t reasoncode)
{
	if (soc->ops->cmn_drv_ops->delba_process)
		return soc->ops->cmn_drv_ops->delba_process(peer_handle,
			tid, reasoncode);
	return 0;
}

#endif /* _CDP_TXRX_CMN_H_ */
