/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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

/* Include Files */
#include "wlan_ipa_core.h"
#include "wlan_ipa_main.h"
#include <ol_txrx.h>
#include "cdp_txrx_ipa.h"
#include "wal_rx_desc.h"

static struct wlan_ipa_priv *gp_ipa;

static struct wlan_ipa_iface_2_client {
	qdf_ipa_client_type_t cons_client;
	qdf_ipa_client_type_t prod_client;
} wlan_ipa_iface_2_client[WLAN_IPA_MAX_IFACE] = {
	{
		QDF_IPA_CLIENT_WLAN2_CONS, QDF_IPA_CLIENT_WLAN1_PROD
	}, {
		QDF_IPA_CLIENT_WLAN3_CONS, QDF_IPA_CLIENT_WLAN1_PROD
	}, {
		QDF_IPA_CLIENT_WLAN4_CONS, QDF_IPA_CLIENT_WLAN1_PROD
	}
};

/**
 * wlan_ipa_uc_sta_is_enabled() - Is STA mode IPA uC offload enabled?
 * @ipa_cfg: IPA config
 *
 * Return: true if STA mode IPA uC offload is enabled, false otherwise
 */
static inline bool wlan_ipa_uc_sta_is_enabled(struct wlan_ipa_config *ipa_cfg)
{
	return WLAN_IPA_IS_CONFIG_ENABLED(ipa_cfg, WLAN_IPA_UC_STA_ENABLE_MASK);
}

/**
 * wlan_ipa_is_pre_filter_enabled() - Is IPA pre-filter enabled?
 * @ipa_cfg: IPA config
 *
 * Return: true if pre-filter is enabled, otherwise false
 */
static inline
bool wlan_ipa_is_pre_filter_enabled(struct wlan_ipa_config *ipa_cfg)
{
	return WLAN_IPA_IS_CONFIG_ENABLED(ipa_cfg,
					 WLAN_IPA_PRE_FILTER_ENABLE_MASK);
}

/**
 * wlan_ipa_is_ipv6_enabled() - Is IPA IPv6 enabled?
 * @ipa_cfg: IPA config
 *
 * Return: true if IPv6 is enabled, otherwise false
 */
static inline bool wlan_ipa_is_ipv6_enabled(struct wlan_ipa_config *ipa_cfg)
{
	return WLAN_IPA_IS_CONFIG_ENABLED(ipa_cfg, WLAN_IPA_IPV6_ENABLE_MASK);
}

/**
 * wlan_ipa_msg_free_fn() - Free an IPA message
 * @buff: pointer to the IPA message
 * @len: length of the IPA message
 * @type: type of IPA message
 *
 * Return: None
 */
static void wlan_ipa_msg_free_fn(void *buff, uint32_t len, uint32_t type)
{
	ipa_debug("msg type:%d, len:%d", type, len);
	qdf_mem_free(buff);
}

/**
 * wlan_ipa_uc_loaded_uc_cb() - IPA UC loaded event callback
 * @priv_ctxt: IPA context
 *
 * Will be called by IPA context.
 * It's atomic context, then should be scheduled to kworker thread
 *
 * Return: None
 */
static void wlan_ipa_uc_loaded_uc_cb(void *priv_ctxt)
{
	struct wlan_ipa_priv *ipa_ctx;
	struct op_msg_type *msg;
	struct uc_op_work_struct *uc_op_work;

	if (!priv_ctxt) {
		ipa_err("Invalid IPA context");
		return;
	}

	ipa_ctx = priv_ctxt;

	msg = qdf_mem_malloc(sizeof(*msg));
	if (!msg) {
		ipa_err("op_msg allocation fails");
		return;
	}

	msg->op_code = WLAN_IPA_UC_OPCODE_UC_READY;

	uc_op_work = &ipa_ctx->uc_op_work[msg->op_code];

	/* When the same uC OPCODE is already pended, just return */
	if (uc_op_work->msg)
		goto done;

	uc_op_work->msg = msg;
	qdf_sched_work(0, &uc_op_work->work);
	/* work handler will free the msg buffer */
	return;

done:
	qdf_mem_free(msg);
}

/**
 * wlan_ipa_uc_send_wdi_control_msg() - Set WDI control message
 * @ctrl: WDI control value
 *
 * Send WLAN_WDI_ENABLE for ctrl = true and WLAN_WDI_DISABLE otherwise.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wlan_ipa_uc_send_wdi_control_msg(bool ctrl)
{
	qdf_ipa_msg_meta_t meta;
	qdf_ipa_wlan_msg_t *ipa_msg;
	int ret = 0;

	/* WDI enable message to IPA */
	QDF_IPA_MSG_META_MSG_LEN(&meta) = sizeof(*ipa_msg);
	ipa_msg = qdf_mem_malloc(QDF_IPA_MSG_META_MSG_LEN(&meta));
	if (!ipa_msg) {
		ipa_err("msg allocation failed");
		return QDF_STATUS_E_NOMEM;
	}

	if (ctrl)
		QDF_IPA_SET_META_MSG_TYPE(&meta, QDF_WDI_ENABLE);
	else
		QDF_IPA_SET_META_MSG_TYPE(&meta, QDF_WDI_DISABLE);

	ipa_debug("ipa_send_msg(Evt:%d)", QDF_IPA_MSG_META_MSG_TYPE(&meta));
	ret = qdf_ipa_send_msg(&meta, ipa_msg, wlan_ipa_msg_free_fn);
	if (ret) {
		ipa_err("ipa_send_msg(Evt:%d)-fail=%d",
			QDF_IPA_MSG_META_MSG_TYPE(&meta), ret);
		qdf_mem_free(ipa_msg);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#ifdef CONFIG_IPA_WDI_UNIFIED_API

/*
 * TODO: Get WDI version through FW capabilities
 */
#ifdef CONFIG_LITHIUM
static inline void wlan_ipa_wdi_get_wdi_version(struct wlan_ipa_priv *ipa_ctx)
{
	ipa_ctx->wdi_version = IPA_WDI_3;
}
#elif defined(QCA_WIFI_3_0)
static inline void wlan_ipa_wdi_get_wdi_version(struct wlan_ipa_priv *ipa_ctx)
{
	ipa_ctx->wdi_version = IPA_WDI_2;
}
#else
static inline void wlan_ipa_wdi_get_wdi_version(struct wlan_ipa_priv *ipa_ctx)
{
	ipa_ctx->wdi_version = IPA_WDI_1;
}
#endif

static inline QDF_STATUS wlan_ipa_wdi_init(struct wlan_ipa_priv *ipa_ctx)
{
	qdf_ipa_wdi_init_in_params_t in;
	qdf_ipa_wdi_init_out_params_t out;
	int ret;

	ipa_ctx->uc_loaded = false;

	QDF_IPA_WDI_INIT_IN_PARAMS_WDI_VERSION(&in) = ipa_ctx->wdi_version;
	QDF_IPA_WDI_INIT_IN_PARAMS_NOTIFY(&in) = wlan_ipa_uc_loaded_uc_cb;
	QDF_IPA_WDI_INIT_IN_PARAMS_PRIV(&in) = (void *)ipa_ctx;

	ret = qdf_ipa_wdi_init(&in, &out);
	if (ret) {
		ipa_err("ipa_wdi_init failed with ret=%d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	if (QDF_IPA_WDI_INIT_OUT_PARAMS_IS_UC_READY(&out)) {
		ipa_info("IPA uC READY");
		ipa_ctx->uc_loaded = true;
		ipa_ctx->is_smmu_enabled =
			QDF_IPA_WDI_INIT_OUT_PARAMS_IS_SMMU_ENABLED(&out);
		ipa_info("is_smmu_enabled=%d", ipa_ctx->is_smmu_enabled);
	} else {
		return QDF_STATUS_E_BUSY;
	}

	return QDF_STATUS_SUCCESS;
}

static inline int wlan_ipa_wdi_cleanup(void)
{
	int ret;

	ret = qdf_ipa_wdi_cleanup();
	if (ret)
		ipa_info("ipa_wdi_cleanup failed ret=%d", ret);
	return ret;
}

static inline int wlan_ipa_wdi_setup_sys_pipe(struct wlan_ipa_priv *ipa_ctx,
					     struct ipa_sys_connect_params *sys,
					     uint32_t *handle)
{
	return 0;
}

static inline int wlan_ipa_wdi_teardown_sys_pipe(struct wlan_ipa_priv *ipa_ctx,
						uint32_t handle)
{
	return 0;
}

#else /* CONFIG_IPA_WDI_UNIFIED_API */

static inline void wlan_ipa_wdi_get_wdi_version(struct wlan_ipa_priv *ipa_ctx)
{
}

static inline QDF_STATUS wlan_ipa_wdi_init(struct wlan_ipa_priv *ipa_ctx)
{
	struct ipa_wdi_uc_ready_params uc_ready_param;

	ipa_ctx->uc_loaded = false;
	uc_ready_param.priv = (void *)ipa_ctx;
	uc_ready_param.notify = wlan_ipa_uc_loaded_uc_cb;
	if (qdf_ipa_uc_reg_rdyCB(&uc_ready_param)) {
		ipa_info("UC Ready CB register fail");
		return QDF_STATUS_E_FAILURE;
	}

	if (true == uc_ready_param.is_uC_ready) {
		ipa_info("UC Ready");
		ipa_ctx->uc_loaded = true;
	} else {
		return QDF_STATUS_E_BUSY;
	}

	return QDF_STATUS_SUCCESS;
}

static inline int wlan_ipa_wdi_cleanup(void)
{
	int ret;

	ret = qdf_ipa_uc_dereg_rdyCB();
	if (ret)
		ipa_info("UC Ready CB deregister fail");
	return ret;
}

static inline int wlan_ipa_wdi_setup_sys_pipe(
		struct wlan_ipa_priv *ipa_ctx,
		struct ipa_sys_connect_params *sys, uint32_t *handle)
{
	return qdf_ipa_setup_sys_pipe(sys, handle);
}

static inline int wlan_ipa_wdi_teardown_sys_pipe(
		struct wlan_ipa_priv *ipa_ctx,
		uint32_t handle)
{
	return qdf_ipa_teardown_sys_pipe(handle);
}

#endif /* CONFIG_IPA_WDI_UNIFIED_API */

/**
 * wlan_ipa_alloc_tx_desc_list() - Allocate IPA Tx desc list
 * @ipa_ctx: IPA context
 *
 * Return: QDF_STATUS
 */
static int wlan_ipa_alloc_tx_desc_list(struct wlan_ipa_priv *ipa_ctx)
{
	int i;
	uint32_t max_desc_cnt;
	struct wlan_ipa_tx_desc *tmp_desc;

	max_desc_cnt = ipa_ctx->config->txbuf_count;

	qdf_list_create(&ipa_ctx->tx_desc_list, max_desc_cnt);

	qdf_spin_lock_bh(&ipa_ctx->q_lock);
	for (i = 0; i < max_desc_cnt; i++) {
		tmp_desc = qdf_mem_malloc(sizeof(*tmp_desc));
		tmp_desc->id = i;
		tmp_desc->ipa_tx_desc_ptr = NULL;
		qdf_list_insert_back(&ipa_ctx->tx_desc_list,
				     &tmp_desc->node);
		tmp_desc++;
	}

	ipa_ctx->stats.num_tx_desc_q_cnt = 0;
	ipa_ctx->stats.num_tx_desc_error = 0;

	qdf_spin_unlock_bh(&ipa_ctx->q_lock);

	return QDF_STATUS_SUCCESS;
}

#ifndef QCA_LL_TX_FLOW_CONTROL_V2
/**
 * wlan_ipa_setup_tx_sys_pipe() - Setup IPA Tx system pipes
 * @ipa_ctx: Global IPA IPA context
 * @desc_fifo_sz: Number of descriptors
 *
 * Return: 0 on success, negative errno on error
 */
static int wlan_ipa_setup_tx_sys_pipe(struct wlan_ipa_priv *ipa_ctx,
				     int32_t desc_fifo_sz)
{
	int i, ret = 0;
	qdf_ipa_sys_connect_params_t *ipa;

	/*setup TX pipes */
	for (i = 0; i < WLAN_IPA_MAX_IFACE; i++) {
		ipa = &ipa_ctx->sys_pipe[i].ipa_sys_params;

		ipa->client = wlan_ipa_iface_2_client[i].cons_client;
		ipa->desc_fifo_sz = desc_fifo_sz;
		ipa->priv = &ipa_ctx->iface_context[i];
		ipa->notify = NULL;

		if (wlan_ipa_uc_sta_is_enabled(ipa_ctx->config)) {
			ipa->ipa_ep_cfg.hdr.hdr_len =
				WLAN_IPA_UC_WLAN_TX_HDR_LEN;
			ipa->ipa_ep_cfg.nat.nat_en = IPA_BYPASS_NAT;
			ipa->ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid = 1;
			ipa->ipa_ep_cfg.hdr.hdr_ofst_pkt_size = 0;
			ipa->ipa_ep_cfg.hdr.hdr_additional_const_len =
				WLAN_IPA_UC_WLAN_8023_HDR_SIZE;
			ipa->ipa_ep_cfg.hdr_ext.hdr_little_endian = true;
		} else {
			ipa->ipa_ep_cfg.hdr.hdr_len = WLAN_IPA_WLAN_TX_HDR_LEN;
		}
		ipa->ipa_ep_cfg.mode.mode = IPA_BASIC;

		ret = wlan_ipa_wdi_setup_sys_pipe(ipa_ctx, ipa,
				&ipa_ctx->sys_pipe[i].conn_hdl);
		if (ret) {
			ipa_err("Failed for pipe %d ret: %d", i, ret);
			return ret;
		}
		ipa_ctx->sys_pipe[i].conn_hdl_valid = 1;
	}

	return ret;
}
#else
/**
 * wlan_ipa_setup_tx_sys_pipe() - Setup IPA Tx system pipes
 * @ipa_ctx: Global IPA IPA context
 * @desc_fifo_sz: Number of descriptors
 *
 * Return: 0 on success, negative errno on error
 */
static int wlan_ipa_setup_tx_sys_pipe(struct wlan_ipa_priv *ipa_ctx,
				     int32_t desc_fifo_sz)
{
	/*
	 * The Tx system pipes are not needed for MCC when TX_FLOW_CONTROL_V2
	 * is enabled, where per vdev descriptors are supported in firmware.
	 */
	return 0;
}
#endif

/**
 * wlan_ipa_setup_rx_sys_pipe() - Setup IPA Rx system pipes
 * @ipa_ctx: Global IPA IPA context
 * @desc_fifo_sz: Number of descriptors
 *
 * Return: 0 on success, negative errno on error
 */
static int wlan_ipa_setup_rx_sys_pipe(struct wlan_ipa_priv *ipa_ctx,
				     int32_t desc_fifo_sz)
{
	int ret = 0;
	qdf_ipa_sys_connect_params_t *ipa;

	/*
	 * Hard code it here, this can be extended if in case
	 * PROD pipe is also per interface.
	 * Right now there is no advantage of doing this.
	 */
	ipa = &ipa_ctx->sys_pipe[WLAN_IPA_RX_PIPE].ipa_sys_params;

	ipa->client = IPA_CLIENT_WLAN1_PROD;

	ipa->desc_fifo_sz = desc_fifo_sz;
	ipa->priv = ipa_ctx;
	ipa->notify = NULL;

	ipa->ipa_ep_cfg.nat.nat_en = IPA_BYPASS_NAT;
	ipa->ipa_ep_cfg.hdr.hdr_len = WLAN_IPA_WLAN_RX_HDR_LEN;
	ipa->ipa_ep_cfg.hdr.hdr_ofst_metadata_valid = 1;
	ipa->ipa_ep_cfg.mode.mode = IPA_BASIC;

	ret = qdf_ipa_setup_sys_pipe(ipa,
			&ipa_ctx->sys_pipe[WLAN_IPA_RX_PIPE].conn_hdl);
	if (ret) {
		ipa_err("Failed for RX pipe: %d", ret);
		return ret;
	}
	ipa_ctx->sys_pipe[WLAN_IPA_RX_PIPE].conn_hdl_valid = 1;

	return ret;
}

/**
 * wlan_ipa_setup_sys_pipe() - Setup all IPA system pipes
 * @ipa_ctx: Global IPA IPA context
 *
 * Return: 0 on success, negative errno on error
 */
static int wlan_ipa_setup_sys_pipe(struct wlan_ipa_priv *ipa_ctx)
{
	int i = WLAN_IPA_MAX_IFACE, ret = 0;
	uint32_t desc_fifo_sz;

	/* The maximum number of descriptors that can be provided to a BAM at
	 * once is one less than the total number of descriptors that the buffer
	 * can contain.
	 * If max_num_of_descriptors = (BAM_PIPE_DESCRIPTOR_FIFO_SIZE / sizeof
	 * (SPS_DESCRIPTOR)), then (max_num_of_descriptors - 1) descriptors can
	 * be provided at once.
	 * Because of above requirement, one extra descriptor will be added to
	 * make sure hardware always has one descriptor.
	 */
	desc_fifo_sz = ipa_ctx->config->desc_size
		       + SPS_DESC_SIZE;

	ret = wlan_ipa_setup_tx_sys_pipe(ipa_ctx, desc_fifo_sz);
	if (ret) {
		ipa_err("Failed for TX pipe: %d", ret);
		goto setup_sys_pipe_fail;
	}

	if (!wlan_ipa_uc_sta_is_enabled(ipa_ctx->config)) {
		ret = wlan_ipa_setup_rx_sys_pipe(ipa_ctx, desc_fifo_sz);
		if (ret) {
			ipa_err("Failed for RX pipe: %d", ret);
			goto setup_sys_pipe_fail;
		}
	}

       /* Allocate free Tx desc list */
	ret = wlan_ipa_alloc_tx_desc_list(ipa_ctx);
	if (ret)
		goto setup_sys_pipe_fail;

	return ret;

setup_sys_pipe_fail:

	for (i = 0; i < WLAN_IPA_MAX_SYSBAM_PIPE; i++) {
		if (ipa_ctx->sys_pipe[i].conn_hdl_valid)
			qdf_ipa_teardown_sys_pipe(
				ipa_ctx->sys_pipe[i].conn_hdl);
		qdf_mem_zero(&ipa_ctx->sys_pipe[i],
			     sizeof(struct wlan_ipa_sys_pipe));
	}

	return ret;
}

/**
 * wlan_ipa_teardown_sys_pipe() - Tear down all IPA Sys pipes
 * @ipa_ctx: Global IPA IPA context
 *
 * Return: None
 */
static void wlan_ipa_teardown_sys_pipe(struct wlan_ipa_priv *ipa_ctx)
{
	int ret = 0, i;
	struct wlan_ipa_tx_desc *tmp_desc;
	qdf_ipa_rx_data_t *ipa_tx_desc;
	qdf_list_node_t *node;

	for (i = 0; i < WLAN_IPA_MAX_SYSBAM_PIPE; i++) {
		if (ipa_ctx->sys_pipe[i].conn_hdl_valid) {
			ret = wlan_ipa_wdi_teardown_sys_pipe(ipa_ctx,
					ipa_ctx->sys_pipe[i].conn_hdl);
			if (ret)
				ipa_err("Failed:%d", ret);

			ipa_ctx->sys_pipe[i].conn_hdl_valid = 0;
		}
	}

	while (qdf_list_remove_front(&ipa_ctx->tx_desc_list, &node) ==
	       QDF_STATUS_SUCCESS) {
		tmp_desc = qdf_container_of(node, struct wlan_ipa_tx_desc,
					    node);
		ipa_tx_desc = tmp_desc->ipa_tx_desc_ptr;
		if (ipa_tx_desc)
			qdf_ipa_free_skb(ipa_tx_desc);

		qdf_mem_free(tmp_desc);
	}
}

/**
 * wlan_ipa_setup() - IPA initialization function
 * @ipa_ctx: IPA context
 * @ipa_cfg: IPA config
 *
 * Allocate ipa_ctx resources, ipa pipe resource and register
 * wlan interface with IPA module.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS wlan_ipa_setup(struct wlan_ipa_priv *ipa_ctx,
			  struct wlan_ipa_config *ipa_cfg)
{
	int ret, i;
	struct wlan_ipa_iface_context *iface_context = NULL;
	QDF_STATUS status;

	ipa_debug("enter");

	gp_ipa = ipa_ctx;
	ipa_ctx->num_iface = 0;
	ipa_ctx->config = ipa_cfg;

	wlan_ipa_wdi_get_wdi_version(ipa_ctx);

	/* Create the interface context */
	for (i = 0; i < WLAN_IPA_MAX_IFACE; i++) {
		iface_context = &ipa_ctx->iface_context[i];
		iface_context->ipa_ctx = ipa_ctx;
		iface_context->cons_client =
			wlan_ipa_iface_2_client[i].cons_client;
		iface_context->prod_client =
			wlan_ipa_iface_2_client[i].prod_client;
		iface_context->iface_id = i;
		iface_context->dev = NULL;
		iface_context->device_mode = QDF_MAX_NO_OF_MODE;
		iface_context->tl_context = NULL;
		qdf_spinlock_create(&iface_context->interface_lock);
	}

	qdf_spinlock_create(&ipa_ctx->pm_lock);
	qdf_spinlock_create(&ipa_ctx->q_lock);
	qdf_nbuf_queue_init(&ipa_ctx->pm_queue_head);
	qdf_list_create(&ipa_ctx->pending_event, 1000);
	qdf_mutex_create(&ipa_ctx->event_lock);
	qdf_mutex_create(&ipa_ctx->ipa_lock);

	for (i = 0; i < WLAN_IPA_MAX_SYSBAM_PIPE; i++)
		qdf_mem_zero(&ipa_ctx->sys_pipe[i],
			     sizeof(struct wlan_ipa_sys_pipe));

	if (wlan_ipa_uc_is_enabled(ipa_ctx->config)) {
		qdf_mem_zero(&ipa_ctx->stats, sizeof(ipa_ctx->stats));
		ipa_ctx->sap_num_connected_sta = 0;
		ipa_ctx->ipa_tx_packets_diff = 0;
		ipa_ctx->ipa_rx_packets_diff = 0;
		ipa_ctx->ipa_p_tx_packets = 0;
		ipa_ctx->ipa_p_rx_packets = 0;
		ipa_ctx->resource_loading = false;
		ipa_ctx->resource_unloading = false;
		ipa_ctx->sta_connected = 0;
		ipa_ctx->ipa_pipes_down = true;
		ipa_ctx->wdi_enabled = false;
		/* Setup IPA system pipes */
		if (wlan_ipa_uc_sta_is_enabled(ipa_ctx->config)) {
			ret = wlan_ipa_setup_sys_pipe(ipa_ctx);
			if (ret)
				goto fail_create_sys_pipe;
		}

		status = wlan_ipa_wdi_init(ipa_ctx);
		if (status == QDF_STATUS_E_BUSY)
			status = wlan_ipa_uc_send_wdi_control_msg(false);
		if (status != QDF_STATUS_SUCCESS) {
			ipa_err("IPA WDI init failed: ret=%d", ret);
			goto fail_create_sys_pipe;
		}
	} else {
		ret = wlan_ipa_setup_sys_pipe(ipa_ctx);
		if (ret)
			goto fail_create_sys_pipe;
	}

	qdf_event_create(&ipa_ctx->ipa_resource_comp);

	ipa_debug("exit: success");

	return QDF_STATUS_SUCCESS;

fail_create_sys_pipe:
	qdf_spinlock_destroy(&ipa_ctx->pm_lock);
	gp_ipa = NULL;
	ipa_debug("exit: fail");

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_ipa_cleanup(struct wlan_ipa_priv *ipa_ctx)
{
	struct wlan_ipa_iface_context *iface_context = NULL;
	int i;

	if (!wlan_ipa_uc_is_enabled(ipa_ctx->config))
		wlan_ipa_teardown_sys_pipe(ipa_ctx);

	/* Teardown IPA sys_pipe for MCC */
	if (wlan_ipa_uc_sta_is_enabled(ipa_ctx->config))
		wlan_ipa_teardown_sys_pipe(ipa_ctx);

	qdf_spinlock_destroy(&ipa_ctx->pm_lock);
	qdf_spinlock_destroy(&ipa_ctx->q_lock);

	/* destroy the interface lock */
	for (i = 0; i < WLAN_IPA_MAX_IFACE; i++) {
		iface_context = &ipa_ctx->iface_context[i];
		qdf_spinlock_destroy(&iface_context->interface_lock);
	}

	if (wlan_ipa_uc_is_enabled(ipa_ctx->config)) {
		wlan_ipa_wdi_cleanup();
		qdf_mutex_destroy(&ipa_ctx->event_lock);
		qdf_mutex_destroy(&ipa_ctx->ipa_lock);
		qdf_list_destroy(&ipa_ctx->pending_event);

	}

	gp_ipa = NULL;

	return QDF_STATUS_SUCCESS;
}
