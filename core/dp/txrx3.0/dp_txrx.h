/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

#ifndef _DP_TXRX_H
#define _DP_TXRX_H

#include <wlan_objmgr_psoc_obj.h>
#include <dp_rx_thread.h>
#include <qdf_trace.h>
#include <cdp_txrx_cmn_struct.h>
#include <cdp_txrx_cmn.h>

/**
 * struct dp_txrx_config - dp txrx configuration passed to dp txrx modules
 * @enable_dp_rx_threads: enable DP rx threads or not
 */
struct dp_txrx_config {
	bool enable_rx_threads;
};

struct dp_txrx_handle_cmn;
/**
 * struct dp_txrx_handle - main dp txrx container handle
 * @soc: ol_txrx_soc_handle soc handle
 * @rx_tm_hdl: rx thread infrastructure handle
 */
struct dp_txrx_handle {
	ol_txrx_soc_handle soc;
	struct cdp_pdev *pdev;
	struct dp_rx_tm_handle rx_tm_hdl;
	struct dp_txrx_config config;
};

#ifdef FEATURE_WLAN_DP_RX_THREADS
/**
 * dp_txrx_get_cmn_hdl_frm_ext_hdl() - conversion func ext_hdl->txrx_handle_cmn
 * @dp_ext_hdl: pointer to dp_txrx_handle structure
 *
 * Return: typecasted pointer of type - struct dp_txrx_handle_cmn
 */
static inline struct dp_txrx_handle_cmn *
dp_txrx_get_cmn_hdl_frm_ext_hdl(struct dp_txrx_handle *dp_ext_hdl)
{
	return (struct dp_txrx_handle_cmn *)dp_ext_hdl;
}

/**
 * dp_txrx_get_ext_hdl_frm_cmn_hdl() - conversion func txrx_handle_cmn->ext_hdl
 * @txrx_cmn_hdl: pointer to dp_txrx_handle_cmn structure
 *
 * Return: typecasted pointer of type - struct dp_txrx_handle
 */
static inline struct dp_txrx_handle *
dp_txrx_get_ext_hdl_frm_cmn_hdl(struct dp_txrx_handle_cmn *txrx_cmn_hdl)
{
	return (struct dp_txrx_handle *)txrx_cmn_hdl;
}

static inline ol_txrx_soc_handle
dp_txrx_get_soc_from_ext_handle(struct dp_txrx_handle_cmn *txrx_cmn_hdl)
{
	struct dp_txrx_handle *dp_ext_hdl;

	dp_ext_hdl = dp_txrx_get_ext_hdl_frm_cmn_hdl(txrx_cmn_hdl);

	return dp_ext_hdl->soc;
}

static inline struct cdp_pdev*
dp_txrx_get_pdev_from_ext_handle(struct dp_txrx_handle_cmn *txrx_cmn_hdl)
{
	struct dp_txrx_handle *dp_ext_hdl;

	dp_ext_hdl = dp_txrx_get_ext_hdl_frm_cmn_hdl(txrx_cmn_hdl);

	return dp_ext_hdl->pdev;
}

/**
 * dp_txrx_init() - initialize DP TXRX module
 * @soc: ol_txrx_soc_handle
 * @pdev_id: id of dp pdev handle
 * @config: configuration for DP TXRX modules
 *
 * Return: QDF_STATUS_SUCCESS on success, error qdf status on failure
 */
QDF_STATUS dp_txrx_init(ol_txrx_soc_handle soc, uint8_t pdev_id,
			struct dp_txrx_config *config);

/**
 * dp_txrx_deinit() - de-initialize DP TXRX module
 * @soc: ol_txrx_soc_handle
 *
 * Return: QDF_STATUS_SUCCESS on success, error qdf status on failure
 */
QDF_STATUS dp_txrx_deinit(ol_txrx_soc_handle soc);

/**
 * dp_txrx_flush_pkts_by_vdev_id() - flush rx packets for a vdev_id
 * @soc: ol_txrx_soc_handle object
 * @vdev_id: vdev_id for which rx packets are to be flushed
 *
 * Return: QDF_STATUS_SUCCESS on success, error qdf status on failure
 */
static inline QDF_STATUS dp_txrx_flush_pkts_by_vdev_id(ol_txrx_soc_handle soc,
						       uint8_t vdev_id)
{
	struct dp_txrx_handle *dp_ext_hdl;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	if (!soc) {
		qdf_status = QDF_STATUS_E_INVAL;
		goto ret;
	}

	dp_ext_hdl = cdp_soc_get_dp_txrx_handle(soc);
	if (!dp_ext_hdl) {
		qdf_status = QDF_STATUS_E_FAULT;
		goto ret;
	}

	qdf_status = dp_rx_tm_flush_by_vdev_id(&dp_ext_hdl->rx_tm_hdl, vdev_id);
ret:
	return qdf_status;
}

/**
 * dp_txrx_resume() - resume all threads
 * @soc: ol_txrx_soc_handle object
 *
 * Return: QDF_STATUS_SUCCESS on success, error qdf status on failure
 */
static inline QDF_STATUS dp_txrx_resume(ol_txrx_soc_handle soc)
{
	struct dp_txrx_handle *dp_ext_hdl;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	if (!soc) {
		qdf_status = QDF_STATUS_E_INVAL;
		goto ret;
	}

	dp_ext_hdl = cdp_soc_get_dp_txrx_handle(soc);
	if (!dp_ext_hdl) {
		qdf_status = QDF_STATUS_E_FAULT;
		goto ret;
	}

	qdf_status = dp_rx_tm_resume(&dp_ext_hdl->rx_tm_hdl);
ret:
	return qdf_status;
}

/**
 * dp_txrx_suspend() - suspend all threads
 * @soc: ol_txrx_soc_handle object
 *
 * Return: QDF_STATUS_SUCCESS on success, error qdf status on failure
 */
static inline QDF_STATUS dp_txrx_suspend(ol_txrx_soc_handle soc)
{
	struct dp_txrx_handle *dp_ext_hdl;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	if (!soc) {
		qdf_status = QDF_STATUS_E_INVAL;
		goto ret;
	}

	dp_ext_hdl = cdp_soc_get_dp_txrx_handle(soc);
	if (!dp_ext_hdl) {
		qdf_status = QDF_STATUS_E_FAULT;
		goto ret;
	}

	qdf_status = dp_rx_tm_suspend(&dp_ext_hdl->rx_tm_hdl);

ret:
	return qdf_status;
}

/**
 * dp_rx_enqueue_pkt() - enqueue packet(s) into the thread
 * @soc: ol_txrx_soc_handle object
 * @nbuf_list: list of packets to be queued into the rx_thread
 *
 * The function accepts a list of skbs connected by the skb->next pointer and
 * queues them into a RX thread to be sent to the stack.
 *
 * Return: QDF_STATUS_SUCCESS on success, error qdf status on failure
 */
static inline
QDF_STATUS dp_rx_enqueue_pkt(ol_txrx_soc_handle soc, qdf_nbuf_t nbuf_list)
{
	struct dp_txrx_handle *dp_ext_hdl;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	if (!soc || !nbuf_list) {
		qdf_status = QDF_STATUS_E_INVAL;
		dp_err("invalid input params soc %pK nbuf %pK"
		       , soc, nbuf_list);
		goto ret;
	}

	dp_ext_hdl = cdp_soc_get_dp_txrx_handle(soc);
	if (!dp_ext_hdl) {
		qdf_status = QDF_STATUS_E_FAULT;
		goto ret;
	}

	qdf_status = dp_rx_tm_enqueue_pkt(&dp_ext_hdl->rx_tm_hdl, nbuf_list);
ret:
	return qdf_status;
}

/**
 * dp_rx_gro_flush_ind() - Flush GRO packets for a given RX CTX Id
 * @soc: ol_txrx_soc_handle object
 * @rx_ctx_id: Context Id (Thread for which GRO packets need to be flushed)
 *
 * Return: QDF_STATUS_SUCCESS on success, error qdf status on failure
 */
static inline
QDF_STATUS dp_rx_gro_flush_ind(ol_txrx_soc_handle soc, int rx_ctx_id)
{
	struct dp_txrx_handle *dp_ext_hdl;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	if (!soc) {
		qdf_status = QDF_STATUS_E_INVAL;
		dp_err("invalid input param soc %pK", soc);
		goto ret;
	}

	dp_ext_hdl = cdp_soc_get_dp_txrx_handle(soc);
	if (!dp_ext_hdl) {
		qdf_status = QDF_STATUS_E_FAULT;
		goto ret;
	}

	qdf_status = dp_rx_tm_gro_flush_ind(&dp_ext_hdl->rx_tm_hdl, rx_ctx_id);
ret:
	return qdf_status;
}

/**
 * dp_txrx_ext_dump_stats() - dump txrx external module stats
 * @soc: ol_txrx_soc_handle object
 * @stats_id: id  for the module whose stats are needed
 *
 * Return: QDF_STATUS_SUCCESS on success, error qdf status on failure
 */
static inline QDF_STATUS dp_txrx_ext_dump_stats(ol_txrx_soc_handle soc,
						uint8_t stats_id)
{
	struct dp_txrx_handle *dp_ext_hdl;
	QDF_STATUS qdf_status;

	if (!soc) {
		dp_err("invalid input params soc %pK", soc);
		return QDF_STATUS_E_INVAL;
	}

	dp_ext_hdl = cdp_soc_get_dp_txrx_handle(soc);
	if (!dp_ext_hdl) {
		return QDF_STATUS_E_FAULT;
	}

	if (stats_id == CDP_DP_RX_THREAD_STATS)
		qdf_status = dp_rx_tm_dump_stats(&dp_ext_hdl->rx_tm_hdl);
	else
		qdf_status = QDF_STATUS_E_INVAL;

	return qdf_status;
}

/**
 * dp_rx_get_napi_context() - get NAPI context for a RX CTX ID
 * @soc: ol_txrx_soc_handle object
 * @rx_ctx_id: RX context ID (RX thread ID) corresponding to which NAPI is
 *             needed
 *
 * Return: NULL on failure, else pointer to NAPI corresponding to rx_ctx_id
 */
static inline
struct napi_struct *dp_rx_get_napi_context(ol_txrx_soc_handle soc,
					   uint8_t rx_ctx_id)
{
	struct dp_txrx_handle *dp_ext_hdl;

	if (!soc) {
		dp_err("soc in NULL!");
		return NULL;
	}

	dp_ext_hdl = cdp_soc_get_dp_txrx_handle(soc);
	if (!dp_ext_hdl) {
		dp_err("dp_ext_hdl in NULL!");
		return NULL;
	}

	return dp_rx_tm_get_napi_context(&dp_ext_hdl->rx_tm_hdl, rx_ctx_id);
}

/**
 * dp_txrx_set_cpu_mask() - set CPU mask for RX threads
 * @soc: ol_txrx_soc_handle object
 * @new_mask: New CPU mask pointer
 *
 * Return: QDF_STATUS_SUCCESS on success, error qdf status on failure
 */
static inline
QDF_STATUS dp_txrx_set_cpu_mask(ol_txrx_soc_handle soc, qdf_cpu_mask *new_mask)
{
	struct dp_txrx_handle *dp_ext_hdl;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	if (!soc) {
		qdf_status = QDF_STATUS_E_INVAL;
		goto ret;
	}

	dp_ext_hdl = cdp_soc_get_dp_txrx_handle(soc);
	if (!dp_ext_hdl) {
		qdf_status = QDF_STATUS_E_FAULT;
		goto ret;
	}

	qdf_status = dp_rx_tm_set_cpu_mask(&dp_ext_hdl->rx_tm_hdl, new_mask);

ret:
	return qdf_status;
}

#else

static inline
QDF_STATUS dp_txrx_init(ol_txrx_soc_handle soc, uint8_t pdev_id,
			struct dp_txrx_config *config)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_txrx_deinit(ol_txrx_soc_handle soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_txrx_flush_pkts_by_vdev_id(ol_txrx_soc_handle soc,
						       uint8_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_txrx_resume(ol_txrx_soc_handle soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_txrx_suspend(ol_txrx_soc_handle soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS dp_rx_enqueue_pkt(ol_txrx_soc_handle soc, qdf_nbuf_t nbuf_list)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS dp_rx_gro_flush_ind(ol_txrx_soc_handle soc, int rx_ctx_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_txrx_ext_dump_stats(ol_txrx_soc_handle soc,
						uint8_t stats_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline
struct napi_struct *dp_rx_get_napi_context(ol_txrx_soc_handle soc,
					   uint8_t rx_ctx_id)
{
	return NULL;
}

static inline
QDF_STATUS dp_txrx_set_cpu_mask(ol_txrx_soc_handle soc, qdf_cpu_mask *new_mask)
{
	return QDF_STATUS_SUCCESS;
}

#endif /* FEATURE_WLAN_DP_RX_THREADS */
#endif /* _DP_TXRX_H */
