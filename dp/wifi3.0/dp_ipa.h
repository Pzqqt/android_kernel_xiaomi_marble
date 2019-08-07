/*
 * Copyright (c) 2017-2019, The Linux Foundation. All rights reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _DP_IPA_H_
#define _DP_IPA_H_

#ifdef IPA_OFFLOAD

#define DP_IPA_MAX_IFACE	3
#define IPA_TCL_DATA_RING_IDX	2
#define IPA_TX_COMP_RING_IDX	2
#define IPA_REO_DEST_RING_IDX	3
#define IPA_RX_REFILL_BUF_RING_IDX	2

/**
 * struct dp_ipa_uc_tx_hdr - full tx header registered to IPA hardware
 * @eth:     ether II header
 */
struct dp_ipa_uc_tx_hdr {
	struct ethhdr eth;
} __packed;

/**
 * struct dp_ipa_uc_rx_hdr - full rx header registered to IPA hardware
 * @eth:     ether II header
 */
struct dp_ipa_uc_rx_hdr {
	struct ethhdr eth;
} __packed;

#define DP_IPA_UC_WLAN_TX_HDR_LEN      sizeof(struct dp_ipa_uc_tx_hdr)
#define DP_IPA_UC_WLAN_RX_HDR_LEN      sizeof(struct dp_ipa_uc_rx_hdr)
#define DP_IPA_UC_WLAN_HDR_DES_MAC_OFFSET	0

QDF_STATUS dp_ipa_get_resource(struct cdp_pdev *pdev);
QDF_STATUS dp_ipa_set_doorbell_paddr(struct cdp_pdev *pdev);
QDF_STATUS dp_ipa_uc_set_active(struct cdp_pdev *pdev, bool uc_active,
		bool is_tx);
QDF_STATUS dp_ipa_op_response(struct cdp_pdev *pdev, uint8_t *op_msg);
QDF_STATUS dp_ipa_register_op_cb(struct cdp_pdev *pdev, ipa_uc_op_cb_type op_cb,
		void *usr_ctxt);
QDF_STATUS dp_ipa_get_stat(struct cdp_pdev *pdev);
qdf_nbuf_t dp_tx_send_ipa_data_frame(struct cdp_vdev *vdev, qdf_nbuf_t skb);
QDF_STATUS dp_ipa_enable_autonomy(struct cdp_pdev *pdev);
QDF_STATUS dp_ipa_disable_autonomy(struct cdp_pdev *pdev);
#ifdef CONFIG_IPA_WDI_UNIFIED_API
QDF_STATUS dp_ipa_setup(struct cdp_pdev *pdev, void *ipa_i2w_cb,
			void *ipa_w2i_cb, void *ipa_wdi_meter_notifier_cb,
			uint32_t ipa_desc_size, void *ipa_priv,
			bool is_rm_enabled, uint32_t *tx_pipe_handle,
			uint32_t *rx_pipe_handle,
			bool is_smmu_enabled,
			qdf_ipa_sys_connect_params_t *sys_in, bool over_gsi);
#else /* CONFIG_IPA_WDI_UNIFIED_API */
QDF_STATUS dp_ipa_setup(struct cdp_pdev *pdev, void *ipa_i2w_cb,
			void *ipa_w2i_cb, void *ipa_wdi_meter_notifier_cb,
			uint32_t ipa_desc_size, void *ipa_priv,
			bool is_rm_enabled, uint32_t *tx_pipe_handle,
			uint32_t *rx_pipe_handle);
#endif /* CONFIG_IPA_WDI_UNIFIED_API */
QDF_STATUS dp_ipa_cleanup(uint32_t tx_pipe_handle,
		uint32_t rx_pipe_handle);
QDF_STATUS dp_ipa_remove_header(char *name);
int dp_ipa_add_header_info(char *ifname, uint8_t *mac_addr,
		uint8_t session_id, bool is_ipv6_enabled);
int dp_ipa_register_interface(char *ifname, bool is_ipv6_enabled);
QDF_STATUS dp_ipa_setup_iface(char *ifname, uint8_t *mac_addr,
		qdf_ipa_client_type_t prod_client,
		qdf_ipa_client_type_t cons_client,
		uint8_t session_id, bool is_ipv6_enabled);
QDF_STATUS dp_ipa_cleanup_iface(char *ifname, bool is_ipv6_enabled);
QDF_STATUS dp_ipa_enable_pipes(struct cdp_pdev *pdev);
QDF_STATUS dp_ipa_disable_pipes(struct cdp_pdev *pdev);
QDF_STATUS dp_ipa_set_perf_level(int client,
		uint32_t max_supported_bw_mbps);

/**
 * dp_ipa_rx_intrabss_fwd() - Perform intra-bss fwd for IPA RX path
 *
 * @pvdev: pointer to dp_vdev structure
 * @nbuf: pointer to skb of ethernet packet received from IPA RX path
 * @fwd_success: pointer to indicate if skb succeeded in intra-bss TX
 *
 * This function performs intra-bss forwarding for WDI 3.0 IPA RX path.
 *
 * Return: true if packet is intra-bss fwd-ed and no need to pass to
 *	   network stack. false if packet needs to be passed to network stack.
 */
bool dp_ipa_rx_intrabss_fwd(struct cdp_vdev *pvdev, qdf_nbuf_t nbuf,
			    bool *fwd_success);
int dp_ipa_uc_detach(struct dp_soc *soc, struct dp_pdev *pdev);
int dp_ipa_uc_attach(struct dp_soc *soc, struct dp_pdev *pdev);
int dp_ipa_ring_resource_setup(struct dp_soc *soc,
		struct dp_pdev *pdev);
QDF_STATUS dp_ipa_handle_rx_buf_smmu_mapping(struct dp_soc *soc,
					     qdf_nbuf_t nbuf,
					     bool create);

bool dp_reo_remap_config(struct dp_soc *soc, uint32_t *remap1,
			 uint32_t *remap2);
bool dp_ipa_is_mdm_platform(void);

qdf_nbuf_t dp_ipa_handle_rx_reo_reinject(struct dp_soc *soc, qdf_nbuf_t nbuf);

#else
static inline int dp_ipa_uc_detach(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline int dp_ipa_uc_attach(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline int dp_ipa_ring_resource_setup(struct dp_soc *soc,
					     struct dp_pdev *pdev)
{
	return 0;
}

static inline QDF_STATUS dp_ipa_handle_rx_buf_smmu_mapping(struct dp_soc *soc,
							   qdf_nbuf_t nbuf,
							   bool create)
{
	return QDF_STATUS_SUCCESS;
}

static inline qdf_nbuf_t dp_ipa_handle_rx_reo_reinject(struct dp_soc *soc,
						       qdf_nbuf_t nbuf)
{
	return nbuf;
}

#endif
#endif /* _DP_IPA_H_ */
