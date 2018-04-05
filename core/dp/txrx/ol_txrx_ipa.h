/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
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

#ifndef _OL_TXRX_IPA_H_
#define _OL_TXRX_IPA_H_

#ifdef IPA_OFFLOAD

#include <cdp_txrx_cmn.h>       /* ol_txrx_vdev_t, etc. */
#include <ol_txrx_types.h>

/**
 * struct frag_header - fragment header type registered to IPA hardware
 * @length:    fragment length
 * @reserved1: Reserved not used
 * @reserved2: Reserved not used
 *
 */
#ifdef QCA_WIFI_3_0
struct frag_header {
	uint16_t length;
	uint32_t reserved1;
	uint32_t reserved2;
} __packed;
#else
struct frag_header {
	uint32_t
		length:16,
		reserved16:16;
	uint32_t reserved2;
} __packed;
#endif

/**
 * struct ipa_header - ipa header type registered to IPA hardware
 * @vdev_id:  vdev id
 * @reserved: Reserved not used
 *
 */
struct ipa_header {
	uint32_t
		vdev_id:8,      /* vdev_id field is LSB of IPA DESC */
		reserved:24;
} __packed;

/**
 * struct ol_txrx_ipa_uc_tx_hdr - full tx header registered to IPA hardware
 * @frag_hd: fragment header
 * @ipa_hd:  ipa header
 * @eth:     ether II header
 *
 */
struct ol_txrx_ipa_uc_tx_hdr {
	struct frag_header frag_hd;
	struct ipa_header ipa_hd;
	struct ethhdr eth;
} __packed;

/**
 * struct ol_txrx_ipa_uc_rx_hdr - full rx header registered to IPA hardware
 * @eth:     ether II header
 *
 */
struct ol_txrx_ipa_uc_rx_hdr {
	struct ethhdr eth;
} __packed;

#define OL_TXRX_IPA_UC_WLAN_8023_HDR_SIZE      14

#define OL_TXRX_IPA_IPV4_NAME_EXT              "_ipv4"
#define OL_TXRX_IPA_IPV6_NAME_EXT              "_ipv6"

#define OL_TXRX_IPA_MAX_IFACE                  3

#define OL_TXRX_IPA_WLAN_FRAG_HEADER        sizeof(struct frag_header)
#define OL_TXRX_IPA_WLAN_IPA_HEADER         sizeof(struct ipa_header)
#define OL_TXRX_IPA_UC_WLAN_TX_HDR_LEN      sizeof(struct ol_txrx_ipa_uc_tx_hdr)
#define OL_TXRX_IPA_UC_WLAN_RX_HDR_LEN      sizeof(struct ol_txrx_ipa_uc_rx_hdr)
#define OL_TXRX_IPA_UC_WLAN_HDR_DES_MAC_OFFSET \
	(OL_TXRX_IPA_WLAN_FRAG_HEADER + OL_TXRX_IPA_WLAN_IPA_HEADER)

#if defined(QCA_WIFI_3_0) && defined(CONFIG_IPA3)
#define OL_TXRX_IPA_WDI2_SET(pipe_in, ipa_res, osdev) \
	do { \
		QDF_IPA_PIPE_IN_UL_RDY_RING_RP_VA(pipe_in) = \
			ipa_res->rx_proc_done_idx->vaddr; \
		QDF_IPA_PIPE_IN_UL_RDY_COMP_RING(pipe_in) = \
			qdf_mem_get_dma_addr(osdev, \
				&ipa_res->rx2_rdy_ring->mem_info);\
		QDF_IPA_PIPE_IN_UL_RDY_COMP_RING_SIZE(pipe_in) = \
			ipa_res->rx2_rdy_ring->mem_info.size; \
		QDF_IPA_PIPE_IN_UL_RDY_COMP_RING_WP_PA(pipe_in) = \
			qdf_mem_get_dma_addr(osdev, \
				&ipa_res->rx2_proc_done_idx->mem_info); \
		QDF_IPA_PIPE_IN_UL_RDY_COMP_RING_WP_VA(pipe_in) = \
			ipa_res->rx2_proc_done_idx->vaddr; \
	} while (0)
#else
/* Do nothing */
#define OL_TXRX_IPA_WDI2_SET(pipe_in, ipa_res, osdev)
#endif /* IPA3 */

QDF_STATUS ol_txrx_ipa_uc_get_resource(struct cdp_pdev *pdev);
QDF_STATUS ol_txrx_ipa_uc_set_doorbell_paddr(struct cdp_pdev *pdev);
QDF_STATUS ol_txrx_ipa_uc_set_active(struct cdp_pdev *pdev, bool uc_active,
		bool is_tx);
QDF_STATUS ol_txrx_ipa_uc_op_response(struct cdp_pdev *pdev, uint8_t *op_msg);
QDF_STATUS ol_txrx_ipa_uc_register_op_cb(struct cdp_pdev *pdev,
		ipa_uc_op_cb_type op_cb, void *usr_ctxt);
QDF_STATUS ol_txrx_ipa_uc_get_stat(struct cdp_pdev *pdev);
QDF_STATUS ol_txrx_ipa_enable_autonomy(struct cdp_pdev *pdev);
QDF_STATUS ol_txrx_ipa_disable_autonomy(struct cdp_pdev *pdev);
#ifdef CONFIG_IPA_WDI_UNIFIED_API
QDF_STATUS ol_txrx_ipa_setup(struct cdp_pdev *pdev, void *ipa_i2w_cb,
		void *ipa_w2i_cb, void *ipa_wdi_meter_notifier_cb,
		uint32_t ipa_desc_size, void *ipa_priv, bool is_rm_enabled,
		uint32_t *tx_pipe_handle, uint32_t *rx_pipe_handle,
		bool is_smmu_enabled, qdf_ipa_sys_connect_params_t *sys_in);
#else /* CONFIG_IPA_WDI_UNIFIED_API */
QDF_STATUS ol_txrx_ipa_setup(struct cdp_pdev *pdev, void *ipa_i2w_cb,
		void *ipa_w2i_cb, void *ipa_wdi_meter_notifier_cb,
		uint32_t ipa_desc_size, void *ipa_priv, bool is_rm_enabled,
		uint32_t *tx_pipe_handle, uint32_t *rx_pipe_handle);
#endif /* CONFIG_IPA_WDI_UNIFIED_API */
QDF_STATUS ol_txrx_ipa_cleanup(uint32_t tx_pipe_handle,
		uint32_t rx_pipe_handle);
QDF_STATUS ol_txrx_ipa_setup_iface(char *ifname, uint8_t *mac_addr,
		qdf_ipa_client_type_t prod_client,
		qdf_ipa_client_type_t cons_client,
		uint8_t session_id, bool is_ipv6_enabled);
QDF_STATUS ol_txrx_ipa_cleanup_iface(char *ifname, bool is_ipv6_enabled);
QDF_STATUS ol_txrx_ipa_enable_pipes(struct cdp_pdev *pdev);
QDF_STATUS ol_txrx_ipa_disable_pipes(struct cdp_pdev *pdev);
QDF_STATUS ol_txrx_ipa_set_perf_level(int client,
		uint32_t max_supported_bw_mbps);
#ifdef FEATURE_METERING
QDF_STATUS ol_txrx_ipa_uc_get_share_stats(struct cdp_pdev *pdev,
		uint8_t reset_stats);
QDF_STATUS ol_txrx_ipa_uc_set_quota(struct cdp_pdev *pdev,
		uint64_t quota_bytes);
#endif
#endif
#endif /* _OL_TXRX_IPA_H_*/
