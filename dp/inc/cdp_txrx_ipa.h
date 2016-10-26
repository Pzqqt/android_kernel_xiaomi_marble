/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_ipa.h
 * @brief Define the host data path IP Acceleraor API functions
 */
#ifndef _CDP_TXRX_IPA_H_
#define _CDP_TXRX_IPA_H_
#include <cdp_txrx_mob_def.h>
#include "cdp_txrx_handle.h"
/**
 * cdp_ipa_get_resource() - Get allocated wlan resources for ipa data path
 * @soc - data path soc handle
 * @pdev - device instance pointer
 * @ipa_res - ipa resources pointer
 *
 * Get allocated wlan resources for ipa data path
 *
 * return none
 */
static inline void
cdp_ipa_get_resource(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		 struct ol_txrx_ipa_resources *ipa_res)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->ipa_ops->ipa_get_resource)
		return soc->ops->ipa_ops->ipa_get_resource(pdev, ipa_res);

	return;
}

/**
 * cdp_ipa_set_doorbell_paddr() - give IPA db paddr to fw
 * @soc - data path soc handle
 * @pdev - device instance pointer
 * @ipa_tx_uc_doorbell_paddr - tx db paddr
 * @ipa_rx_uc_doorbell_paddr - rx db paddr
 *
 * give IPA db paddr to fw
 *
 * return none
 */
static inline void
cdp_ipa_set_doorbell_paddr(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		 qdf_dma_addr_t ipa_tx_uc_doorbell_paddr,
		 qdf_dma_addr_t ipa_rx_uc_doorbell_paddr)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->ipa_ops->ipa_set_doorbell_paddr)
		return soc->ops->ipa_ops->ipa_set_doorbell_paddr(pdev,
			ipa_tx_uc_doorbell_paddr, ipa_rx_uc_doorbell_paddr);

	return;
}

/**
 * cdp_ipa_set_active() - activate/de-ctivate wlan fw ipa data path
 * @soc - data path soc handle
 * @pdev - device instance pointer
 * @uc_active - activate or de-activate
 * @is_tx - toggle tx or rx data path
 *
 * activate/de-ctivate wlan fw ipa data path
 *
 * return none
 */
static inline void
cdp_ipa_set_active(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		 bool uc_active, bool is_tx)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->ipa_ops->ipa_set_active)
		return soc->ops->ipa_ops->ipa_set_active(pdev, uc_active,
				is_tx);

	return;
}

/**
 * cdp_ipa_op_response() - event handler from fw
 * @soc - data path soc handle
 * @pdev - device instance pointer
 * @op_msg - event contents from firmware
 *
 * event handler from fw
 *
 * return none
 */
static inline void
cdp_ipa_op_response(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		uint8_t *op_msg)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->ipa_ops->ipa_op_response)
		return soc->ops->ipa_ops->ipa_op_response(pdev, op_msg);

	return;
}

/**
 * cdp_ipa_register_op_cb() - register event handler function pointer
 * @soc - data path soc handle
 * @pdev - device instance pointer
 * @op_cb - event handler callback function pointer
 * @osif_dev -  osif instance pointer
 *
 * register event handler function pointer
 *
 * return none
 */
static inline void
cdp_ipa_register_op_cb(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		 ipa_op_cb_type op_cb, void *osif_dev)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->ipa_ops->ipa_register_op_cb)
		return soc->ops->ipa_ops->ipa_register_op_cb(pdev, op_cb,
			osif_dev);

	return;
}

/**
 * cdp_ipa_get_stat() - get ipa data path stats from fw
 * @soc - data path soc handle
 * @pdev - device instance pointer
 *
 * get ipa data path stats from fw async
 *
 * return none
 */
static inline void
cdp_ipa_get_stat(ol_txrx_soc_handle soc, struct cdp_pdev *pdev)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->ipa_ops->ipa_get_stat)
		return soc->ops->ipa_ops->ipa_get_stat(pdev);

	return;
}

/**
 * cdp_tx_send_ipa_data_frame() - send IPA data frame
 * @vdev: vdev
 * @skb: skb
 *
 * Return: skb/ NULL is for success
 */
static inline qdf_nbuf_t cdp_ipa_tx_send_data_frame(ol_txrx_soc_handle soc,
				struct cdp_vdev *vdev, qdf_nbuf_t skb)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return skb;
	}

	if (soc->ops->ipa_ops->ipa_tx_data_frame)
		return soc->ops->ipa_ops->ipa_tx_data_frame(vdev, skb);

	return skb;
}

/**
 * cdp_ipa_set_uc_tx_partition_base() - set tx packet partition base
 * @pdev: physical device instance
 * @value: partition base value
 *
 * Return: none
 */
static inline void cdp_ipa_set_uc_tx_partition_base(ol_txrx_soc_handle soc,
				struct cdp_cfg *cfg_pdev, uint32_t value)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->ipa_ops->ipa_set_uc_tx_partition_base)
		return soc->ops->ipa_ops->ipa_set_uc_tx_partition_base(cfg_pdev,
								       value);

	return;
}

/**
 * cdp_ipa_uc_get_share_stats() - get Tx/Rx byte stats from FW
 * @pdev: physical device instance
 * @value: reset stats
 *
 * Return: none
 */
static inline void cdp_ipa_uc_get_share_stats(ol_txrx_soc_handle soc,
				struct cdp_pdev *pdev, uint8_t value)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->ipa_ops->ipa_uc_get_share_stats)
		return soc->ops->ipa_ops->ipa_uc_get_share_stats(pdev,
								 value);

	return;
}

/**
 * cdp_ipa_uc_set_quota() - set quota limit to FW
 * @pdev: physical device instance
 * @value: quota limit bytes
 *
 * Return: none
 */
static inline void cdp_ipa_uc_set_quota(ol_txrx_soc_handle soc,
				struct cdp_pdev *pdev, uint64_t value)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->ipa_ops->ipa_uc_get_share_stats)
		return soc->ops->ipa_ops->ipa_uc_set_quota(pdev,
							   value);

	return;
}
#endif /* _CDP_TXRX_IPA_H_ */

