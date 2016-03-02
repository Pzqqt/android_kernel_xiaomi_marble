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

#include <ol_cfg.h>
#include <ol_if_athvar.h>
#include <cdp_txrx_cfg.h>

unsigned int vow_config = 0;

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
/**
 * ol_tx_set_flow_control_parameters() - set flow control parameters
 * @cfg_ctx: cfg context
 * @cfg_param: cfg parameters
 *
 * Return: none
 */
static
void ol_tx_set_flow_control_parameters(struct txrx_pdev_cfg_t *cfg_ctx,
	struct txrx_pdev_cfg_param_t cfg_param)
{
	cfg_ctx->tx_flow_start_queue_offset =
					cfg_param.tx_flow_start_queue_offset;
	cfg_ctx->tx_flow_stop_queue_th =
					cfg_param.tx_flow_stop_queue_th;
}
#else
static
void ol_tx_set_flow_control_parameters(struct txrx_pdev_cfg_t *cfg_ctx,
	struct txrx_pdev_cfg_param_t cfg_param)
{
	return;
}
#endif

#if CFG_TGT_DEFAULT_RX_SKIP_DEFRAG_TIMEOUT_DUP_DETECTION_CHECK
static inline
uint8_t ol_defrag_timeout_check(void)
{
	return 1;
}
#else
static inline
uint8_t ol_defrag_timeout_check(void)
{
	return 0;
}
#endif

/* FIX THIS -
 * For now, all these configuration parameters are hardcoded.
 * Many of these should actually be determined dynamically instead.
 */

/**
 * ol_pdev_cfg_attach - setup configuration parameters
 *
 *@osdev - OS handle needed as an argument for some OS primitives
 *@cfg_param - configuration parameters
 *
 * Allocation configuration context that will be used across data path
 *
 * Return: the control device object
 */
ol_pdev_handle ol_pdev_cfg_attach(qdf_device_t osdev,
				  struct txrx_pdev_cfg_param_t cfg_param)
{
	struct txrx_pdev_cfg_t *cfg_ctx;

	cfg_ctx = qdf_mem_malloc(sizeof(*cfg_ctx));
	if (!cfg_ctx) {
		printk(KERN_ERR "cfg ctx allocation failed\n");
		return NULL;
	}

	/*
	 * Need to change HTT_LL_TX_HDR_SIZE_IP accordingly.
	 * Include payload, up to the end of UDP header for IPv4 case
	 */
	cfg_ctx->tx_download_size = 16;
	/* temporarily diabled PN check for Riva/Pronto */
	cfg_ctx->rx_pn_check = 1;
	cfg_ctx->defrag_timeout_check = ol_defrag_timeout_check();
	cfg_ctx->max_peer_id = 511;
	cfg_ctx->max_vdev = CFG_TGT_NUM_VDEV;
	cfg_ctx->pn_rx_fwd_check = 1;
	cfg_ctx->frame_type = wlan_frm_fmt_802_3;
	cfg_ctx->max_thruput_mbps = 800;
	cfg_ctx->max_nbuf_frags = 1;
	cfg_ctx->vow_config = vow_config;
	cfg_ctx->target_tx_credit = CFG_TGT_NUM_MSDU_DESC;
	cfg_ctx->throttle_period_ms = 40;
	cfg_ctx->rx_fwd_disabled = 0;
	cfg_ctx->is_packet_log_enabled = 0;
	cfg_ctx->is_full_reorder_offload = cfg_param.is_full_reorder_offload;
	cfg_ctx->ipa_uc_rsc.uc_offload_enabled =
		cfg_param.is_uc_offload_enabled;
	cfg_ctx->ipa_uc_rsc.tx_max_buf_cnt = cfg_param.uc_tx_buffer_count;
	cfg_ctx->ipa_uc_rsc.tx_buf_size = cfg_param.uc_tx_buffer_size;
	cfg_ctx->ipa_uc_rsc.rx_ind_ring_size =
		cfg_param.uc_rx_indication_ring_count;
	cfg_ctx->ipa_uc_rsc.tx_partition_base = cfg_param.uc_tx_partition_base;
	cfg_ctx->enable_rxthread = cfg_param.enable_rxthread;
	cfg_ctx->ip_tcp_udp_checksum_offload =
		cfg_param.ip_tcp_udp_checksum_offload;
	cfg_ctx->ce_classify_enabled = cfg_param.ce_classify_enabled;

	ol_tx_set_flow_control_parameters(cfg_ctx, cfg_param);
	return (ol_pdev_handle) cfg_ctx;
}

int ol_cfg_is_high_latency(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->is_high_latency;
}

int ol_cfg_max_peer_id(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	/*
	 * TBDXXX - this value must match the peer table
	 * size allocated in FW
	 */
	return cfg->max_peer_id;
}

int ol_cfg_max_vdevs(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->max_vdev;
}

int ol_cfg_rx_pn_check(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->rx_pn_check;
}

int ol_cfg_rx_fwd_check(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->pn_rx_fwd_check;
}

/**
 * ol_set_cfg_rx_fwd_disabled - set rx fwd disable/enable
 *
 * @pdev - handle to the physical device
 * @disable_rx_fwd 1 -> no rx->tx forward -> rx->tx forward
 *
 * Choose whether to forward rx frames to tx (where applicable) within the
 * WLAN driver, or to leave all forwarding up to the operating system.
 * Currently only intra-bss fwd is supported.
 *
 */
void ol_set_cfg_rx_fwd_disabled(ol_pdev_handle pdev, uint8_t disable_rx_fwd)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	cfg->rx_fwd_disabled = disable_rx_fwd;
}

/**
 * ol_set_cfg_packet_log_enabled - Set packet log config in HTT
 * config based on CFG ini configuration
 *
 * @pdev - handle to the physical device
 * @val - 0 - disable, 1 - enable
 */
void ol_set_cfg_packet_log_enabled(ol_pdev_handle pdev, uint8_t val)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	cfg->is_packet_log_enabled = val;
}

uint8_t ol_cfg_is_packet_log_enabled(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->is_packet_log_enabled;
}

int ol_cfg_rx_fwd_disabled(ol_pdev_handle pdev)
{
#if defined(ATHR_WIN_NWF)
	/* for Windows, let the OS handle the forwarding */
	return 1;
#else
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->rx_fwd_disabled;
#endif
}

int ol_cfg_rx_fwd_inter_bss(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->rx_fwd_inter_bss;
}

enum wlan_frm_fmt ol_cfg_frame_type(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->frame_type;
}

int ol_cfg_max_thruput_mbps(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->max_thruput_mbps;
}

int ol_cfg_netbuf_frags_max(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->max_nbuf_frags;
}

int ol_cfg_tx_free_at_download(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->tx_free_at_download;
}

uint16_t ol_cfg_target_tx_credit(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	uint16_t rc;
	uint16_t vow_max_sta = (cfg->vow_config & 0xffff0000) >> 16;
	uint16_t vow_max_desc_persta = cfg->vow_config & 0x0000ffff;

	rc =  (cfg->target_tx_credit + (vow_max_sta * vow_max_desc_persta));

	return rc;
}

int ol_cfg_tx_download_size(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->tx_download_size;
}

int ol_cfg_rx_host_defrag_timeout_duplicate_check(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->defrag_timeout_check;
}

int ol_cfg_throttle_period_ms(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->throttle_period_ms;
}

int ol_cfg_is_full_reorder_offload(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->is_full_reorder_offload;
}

/**
 * ol_cfg_is_rx_thread_enabled() - return rx_thread is enable/disable
 * @pdev : handle to the physical device
 *
 * Return: 1 - enable, 0 - disable
 */
int ol_cfg_is_rx_thread_enabled(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->enable_rxthread;
}

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
/**
 * ol_cfg_get_tx_flow_stop_queue_th() - return stop queue threshold
 * @pdev : handle to the physical device
 *
 * Return: stop queue threshold
 */
int ol_cfg_get_tx_flow_stop_queue_th(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->tx_flow_stop_queue_th;
}

/**
 * ol_cfg_get_tx_flow_start_queue_offset() - return start queue offset
 * @pdev : handle to the physical device
 *
 * Return: start queue offset
 */
int ol_cfg_get_tx_flow_start_queue_offset(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->tx_flow_start_queue_offset;
}
#endif

#ifdef IPA_OFFLOAD
unsigned int ol_cfg_ipa_uc_offload_enabled(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return (unsigned int)cfg->ipa_uc_rsc.uc_offload_enabled;
}

unsigned int ol_cfg_ipa_uc_tx_buf_size(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->ipa_uc_rsc.tx_buf_size;
}

unsigned int ol_cfg_ipa_uc_tx_max_buf_cnt(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->ipa_uc_rsc.tx_max_buf_cnt;
}

unsigned int ol_cfg_ipa_uc_rx_ind_ring_size(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->ipa_uc_rsc.rx_ind_ring_size;
}

unsigned int ol_cfg_ipa_uc_tx_partition_base(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->ipa_uc_rsc.tx_partition_base;
}
#endif /* IPA_OFFLOAD */

/**
 * ol_cfg_is_ce_classify_enabled() - Return if CE classification is enabled
 *				     or disabled
 * @pdev : handle to the physical device
 *
 * Return: 1 - enabled, 0 - disabled
 */
bool ol_cfg_is_ce_classify_enabled(ol_pdev_handle pdev)
{
	struct txrx_pdev_cfg_t *cfg = (struct txrx_pdev_cfg_t *)pdev;
	return cfg->ce_classify_enabled;
}
