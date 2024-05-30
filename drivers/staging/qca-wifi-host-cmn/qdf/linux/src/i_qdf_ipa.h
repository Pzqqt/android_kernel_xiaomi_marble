/*
 * Copyright (c) 2017-2021, The Linux Foundation. All rights reserved.
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

#ifndef _I_QDF_IPA_H
#define _I_QDF_IPA_H

#ifdef IPA_OFFLOAD

#include <linux/ipa.h>
#include <linux/version.h>

/**
 * __qdf_ipa_wdi_meter_evt_type_t - type of event client callback is
 * for AP+STA mode metering
 * @IPA_GET_WDI_SAP_STATS: get IPA_stats betwen SAP and STA -
 *			use ipa_get_wdi_sap_stats structure
 * @IPA_SET_WIFI_QUOTA: set quota limit on STA -
 *			use ipa_set_wifi_quota structure
 */
typedef enum ipa_wdi_meter_evt_type __qdf_ipa_wdi_meter_evt_type_t;

typedef struct ipa_get_wdi_sap_stats __qdf_ipa_get_wdi_sap_stats_t;

#define QDF_IPA_GET_WDI_SAP_STATS_RESET_STATS(wdi_sap_stats)	\
	(((struct ipa_get_wdi_sap_stats *)(wdi_sap_stats))->reset_stats)
#define QDF_IPA_GET_WDI_SAP_STATS_STATS_VALID(wdi_sap_stats)	\
	(((struct ipa_get_wdi_sap_stats *)(wdi_sap_stats))->stats_valid)
#define QDF_IPA_GET_WDI_SAP_STATS_IPV4_TX_PACKETS(wdi_sap_stats)	\
	(((struct ipa_get_wdi_sap_stats *)(wdi_sap_stats))->ipv4_tx_packets)
#define QDF_IPA_GET_WDI_SAP_STATS_IPV4_TX_BYTES(wdi_sap_stats)	\
	(((struct ipa_get_wdi_sap_stats *)(wdi_sap_stats))->ipv4_tx_bytes)
#define QDF_IPA_GET_WDI_SAP_STATS_IPV4_RX_PACKETS(wdi_sap_stats)	\
	(((struct ipa_get_wdi_sap_stats *)(wdi_sap_stats))->ipv4_rx_packets)
#define QDF_IPA_GET_WDI_SAP_STATS_IPV4_RX_BYTES(wdi_sap_stats)	\
	(((struct ipa_get_wdi_sap_stats *)(wdi_sap_stats))->ipv4_rx_bytes)
#define QDF_IPA_GET_WDI_SAP_STATS_IPV6_TX_PACKETS(wdi_sap_stats)	\
	(((struct ipa_get_wdi_sap_stats *)(wdi_sap_stats))->ipv6_tx_packets)
#define QDF_IPA_GET_WDI_SAP_STATS_IPV6_TX_BYTES(wdi_sap_stats)	\
	(((struct ipa_get_wdi_sap_stats *)(wdi_sap_stats))->ipv6_tx_bytes)
#define QDF_IPA_GET_WDI_SAP_STATS_IPV6_RX_PACKETS(wdi_sap_stats)	\
	(((struct ipa_get_wdi_sap_stats *)(wdi_sap_stats))->ipv6_rx_packets)
#define QDF_IPA_GET_WDI_SAP_STATS_IPV6_RX_BYTES(wdi_sap_stats)	\
	(((struct ipa_get_wdi_sap_stats *)(wdi_sap_stats))->ipv6_rx_bytes)

/**
 * __qdf_ipa_set_wifi_quota_t - structure used for
 *                                   IPA_SET_WIFI_QUOTA.
 */
typedef struct ipa_set_wifi_quota __qdf_ipa_set_wifi_quota_t;

#define QDF_IPA_SET_WIFI_QUOTA_BYTES(ipa_set_quota)	\
	(((struct ipa_set_wifi_quota *)(ipa_set_quota))->quota_bytes)
#define QDF_IPA_SET_WIFI_QUOTA_SET_QUOTA(ipa_set_quota)	\
	(((struct ipa_set_wifi_quota *)(ipa_set_quota))->set_quota)
#define QDF_IPA_SET_WIFI_QUOTA_SET_VALID(ipa_set_quota)	\
	(((struct ipa_set_wifi_quota *)(ipa_set_quota))->set_valid)

/**
 * __qdf_ipa_connect_params_t - low-level client connect input parameters. Either
 * client allocates the data and desc FIFO and specifies that in data+desc OR
 * specifies sizes and pipe_mem pref and IPA does the allocation.
 */
typedef struct ipa_connect_params __qdf_ipa_connect_params_t;

/**
 * __qdf_ipa_tx_meta_t - meta-data for the TX packet
 */
typedef struct ipa_tx_meta __qdf_ipa_tx_meta_t;

/**
 * __qdf_ipa_msg_free_fn_t - callback function
 *
 * Message callback registered by kernel client with IPA driver to
 * free message payload after IPA driver processing is complete
 */
typedef void (*__qdf_ipa_msg_free_fn_t)(void *buff, u32 len, u32 type);

/**
 *  __qdf_ipa_sps_params_t - SPS related output parameters resulting from
 */
typedef struct ipa_sps_params __qdf_ipa_sps_params_t;

/**
 * __qdf_ipa_tx_intf_t - interface tx properties
 */
typedef struct ipa_tx_intf __qdf_ipa_tx_intf_t;

#define QDF_IPA_TX_INTF_PROP(tx_intf)	\
	(((struct ipa_tx_intf *)(tx_intf))->prop)

/**
 * __qdf_ipa_rx_intf_t - interface rx properties
 */
typedef struct ipa_rx_intf __qdf_ipa_rx_intf_t;

#define QDF_IPA_RX_INTF_PROP(rx_intf)	\
	(((struct ipa_rx_intf *)(rx_intf))->prop)

/**
 * __qdf_ipa_ext_intf_t - interface ext properties
 */
typedef struct ipa_ext_intf __qdf_ipa_ext_intf_t;

/**
 * __qdf_ipa_sys_connect_params_t - information needed to setup an IPA end-point
 * in system-BAM mode
 */
typedef struct ipa_sys_connect_params __qdf_ipa_sys_connect_params_t;

#define QDF_IPA_SYS_PARAMS_NAT_EN(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->ipa_ep_cfg.nat.nat_en)
#define QDF_IPA_SYS_PARAMS_HDR_LEN(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->ipa_ep_cfg.hdr.hdr_len)
#define QDF_IPA_SYS_PARAMS_HDR_ADDITIONAL_CONST_LEN(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->ipa_ep_cfg.hdr.hdr_additional_const_len)
#define QDF_IPA_SYS_PARAMS_HDR_OFST_PKT_SIZE_VALID(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid)
#define QDF_IPA_SYS_PARAMS_HDR_OFST_PKT_SIZE(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->ipa_ep_cfg.hdr.hdr_ofst_pkt_size)
#define QDF_IPA_SYS_PARAMS_HDR_LITTLE_ENDIAN(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->ipa_ep_cfg.hdr_ext.hdr_little_endian)
#define QDF_IPA_SYS_PARAMS_MODE(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->ipa_ep_cfg.mode.mode)
#define QDF_IPA_SYS_PARAMS_CLIENT(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->client)
#define QDF_IPA_SYS_PARAMS_DESC_FIFO_SZ(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->desc_fifo_sz)
#define QDF_IPA_SYS_PARAMS_PRIV(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->priv)
#define QDF_IPA_SYS_PARAMS_NOTIFY(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->notify)
#define QDF_IPA_SYS_PARAMS_SKIP_EP_CFG(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->skip_ep_cfg)
#define QDF_IPA_SYS_PARAMS_KEEP_IPA_AWAKE(ipa_sys_params)	\
	(((struct ipa_sys_connect_params *)(ipa_sys_params))->keep_ipa_awake)

/**
 * __qdf_pa_rm_event_t - IPA RM events
 *
 * Indicate the resource state change
 */
typedef enum ipa_rm_event __qdf_ipa_rm_event_t;

/**
 * struct __qdf_ipa_rm_register_params_t - information needed to
 *      register IPA RM client with IPA RM
 */
typedef struct ipa_rm_register_params __qdf_ipa_rm_register_params_t;

/**
 * struct __qdf_ipa_rm_create_params_t - information needed to initialize
 *				the resource
 *
 * IPA RM client is expected to perform non blocking operations only
 * in request_resource and release_resource functions and
 * release notification context as soon as possible.
 */
typedef struct ipa_rm_create_params __qdf_ipa_rm_create_params_t;

#define QDF_IPA_RM_CREATE_PARAMS_NAME(create_params)	\
	(((struct ipa_rm_create_params *)(create_params))->name)
#define QDF_IPA_RM_CREATE_PARAMS_USER_DATA(create_params)	\
	(((struct ipa_rm_create_params *)(create_params))->reg_params.user_data)
#define QDF_IPA_RM_CREATE_PARAMS_NOTIFY_CB(create_params)	\
	(((struct ipa_rm_create_params *)(create_params))->reg_params.notify_cb)
#define QDF_IPA_RM_CREATE_PARAMS_REQUEST_RESOURCE(create_params)	\
	(((struct ipa_rm_create_params *)(create_params))->request_resource)
#define QDF_IPA_RM_CREATE_PARAMS_RELEASE_RESOURCE(create_params)	\
	(((struct ipa_rm_create_params *)(create_params))->release_resource)
#define QDF_IPA_RM_CREATE_PARAMS_FLOOR_VOLTAGE(create_params)	\
	(((struct ipa_rm_create_params *)(create_params))->floor_voltage)

/**
 * __qdf_ipa_rm_perf_profile_t - information regarding IPA RM client performance
 * profile
 */
typedef struct ipa_rm_perf_profile __qdf_ipa_rm_perf_profile_t;

#define QDF_IPA_RM_PERF_PROFILE_MAX_SUPPORTED_BANDWIDTH_MBPS(profile)	\
	(((struct ipa_rm_perf_profile *)(profile))->max_supported_bandwidth_mbps)

/**
 * __qdf_ipa_tx_data_desc_t - information needed
 * to send data packet to HW link: link to data descriptors
 * priv: client specific private data
 */
typedef struct ipa_tx_data_desc __qdf_ipa_tx_data_desc_t;

/**
 * __qdf_ipa_rx_data_t - information needed
 * to send to wlan driver on receiving data from ipa hw
 */
typedef struct ipa_rx_data __qdf_ipa_rx_data_t;

#define QDF_IPA_RX_DATA_SKB(desc)	\
	(((struct ipa_rx_data *)(desc))->skb)
#define QDF_IPA_RX_DATA_SKB_LEN(desc)	\
	(((struct ipa_rx_data *)(desc))->skb->len)
#define QDF_IPA_RX_DATA_DMA_ADDR(desc)	\
	(((struct ipa_rx_data *)(desc))->dma_addr)

/**
 * __qdf_ipa_wdi_ul_params_t - WDI_RX configuration
 */
typedef struct ipa_wdi_ul_params __qdf_ipa_wdi_ul_params_t;

/**
 * __qdf_ipa_wdi_ul_params_smmu_t - WDI_RX configuration (with WLAN SMMU)
 */
typedef struct ipa_wdi_ul_params_smmu __qdf_ipa_wdi_ul_params_smmu_t;

/**
 * __qdf_ipa_wdi_dl_params_t - WDI_TX configuration
 */
typedef struct ipa_wdi_dl_params __qdf_ipa_wdi_dl_params_t;

/**
 * __qdf_ipa_wdi_dl_params_smmu_t - WDI_TX configuration (with WLAN SMMU)
 */
typedef struct ipa_wdi_dl_params_smmu __qdf_ipa_wdi_dl_params_smmu_t;

/**
 * __qdf_ipa_wdi_in_params_t - information provided by WDI client
 */
typedef struct ipa_wdi_in_params __qdf_ipa_wdi_in_params_t;

#define QDF_IPA_PIPE_IN_NAT_EN(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.ipa_ep_cfg.nat.nat_en)
#define QDF_IPA_PIPE_IN_HDR_LEN(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.ipa_ep_cfg.hdr.hdr_len)
#define QDF_IPA_PIPE_IN_HDR_OFST_METADATA_VALID(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.ipa_ep_cfg.hdr.hdr_ofst_metadata_valid)
#define QDF_IPA_PIPE_IN_HDR_METADATA_REG_VALID(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.ipa_ep_cfg.hdr.hdr_metadata_reg_valid)
#define QDF_IPA_PIPE_IN_HDR_OFST_PKT_SIZE_VALID(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid)
#define QDF_IPA_PIPE_IN_HDR_OFST_PKT_SIZE(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.ipa_ep_cfg.hdr.hdr_ofst_pkt_size)
#define QDF_IPA_PIPE_IN_HDR_ADDITIONAL_CONST_LEN(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.ipa_ep_cfg.hdr.hdr_additional_const_len)
#define QDF_IPA_PIPE_IN_MODE(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.ipa_ep_cfg.mode.mode)
#define QDF_IPA_PIPE_IN_CLIENT(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.client)
#define QDF_IPA_PIPE_IN_DESC_FIFO_SZ(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.desc_fifo_sz)
#define QDF_IPA_PIPE_IN_PRIV(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.priv)
#define QDF_IPA_PIPE_IN_HDR_LITTLE_ENDIAN(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.ipa_ep_cfg.hdr_ext.hdr_little_endian)
#define QDF_IPA_PIPE_IN_NOTIFY(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.notify)
#define QDF_IPA_PIPE_IN_KEEP_IPA_AWAKE(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.keep_ipa_awake)
#define QDF_IPA_PIPE_IN_KEEP_IPA_AWAKE(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->sys.keep_ipa_awake)
#ifdef FEATURE_METERING
#define QDF_IPA_PIPE_IN_WDI_NOTIFY(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->wdi_notify)
#endif

#ifdef ENABLE_SMMU_S1_TRANSLATION
#define QDF_IPA_PIPE_IN_SMMU_ENABLED(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->smmu_enabled)

#define QDF_IPA_PIPE_IN_DL_SMMU_COMP_RING(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.dl_smmu.comp_ring)
#define QDF_IPA_PIPE_IN_DL_SMMU_CE_RING(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.dl_smmu.ce_ring)
#define QDF_IPA_PIPE_IN_DL_SMMU_COMP_RING_SIZE(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.dl_smmu.comp_ring_size)
#define QDF_IPA_PIPE_IN_DL_SMMU_CE_RING_SIZE(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.dl_smmu.ce_ring_size)
#define QDF_IPA_PIPE_IN_DL_SMMU_CE_DOOR_BELL_PA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.dl_smmu.ce_door_bell_pa)
#define QDF_IPA_PIPE_IN_DL_SMMU_NUM_TX_BUFFERS(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.dl_smmu.num_tx_buffers)

#define QDF_IPA_PIPE_IN_UL_SMMU_RDY_RING(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul_smmu.rdy_ring)
#define QDF_IPA_PIPE_IN_UL_SMMU_RDY_RING_SIZE(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul_smmu.rdy_ring_size)
#define QDF_IPA_PIPE_IN_UL_SMMU_RDY_RING_RP_PA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul_smmu.rdy_ring_rp_pa)
#define QDF_IPA_PIPE_IN_UL_SMMU_RDY_RING_RP_VA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul_smmu.rdy_ring_rp_va)
#define QDF_IPA_PIPE_IN_UL_SMMU_RDY_COMP_RING(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul_smmu.rdy_comp_ring)
#define QDF_IPA_PIPE_IN_UL_SMMU_RDY_COMP_RING_SIZE(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul_smmu.rdy_comp_ring_size)
#define QDF_IPA_PIPE_IN_UL_SMMU_RDY_COMP_RING_WP_PA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul_smmu.rdy_comp_ring_wp_pa)
#define QDF_IPA_PIPE_IN_UL_SMMU_RDY_COMP_RING_WP_VA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul_smmu.rdy_comp_ring_wp_va)
#endif

#define QDF_IPA_PIPE_IN_DL_COMP_RING_BASE_PA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.dl.comp_ring_base_pa)
#define QDF_IPA_PIPE_IN_DL_COMP_RING_SIZE(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.dl.comp_ring_size)
#define QDF_IPA_PIPE_IN_DL_CE_RING_BASE_PA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.dl.ce_ring_base_pa)
#define QDF_IPA_PIPE_IN_DL_CE_RING_SIZE(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.dl.ce_ring_size)
#define QDF_IPA_PIPE_IN_DL_CE_DOOR_BELL_PA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.dl.ce_door_bell_pa)
#define QDF_IPA_PIPE_IN_DL_NUM_TX_BUFFERS(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.dl.num_tx_buffers)

#define QDF_IPA_PIPE_IN_UL_RDY_RING_BASE_PA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul.rdy_ring_base_pa)
#define QDF_IPA_PIPE_IN_UL_RDY_RING_SIZE(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul.rdy_ring_size)
#define QDF_IPA_PIPE_IN_UL_RDY_RING_RP_PA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul.rdy_ring_rp_pa)
#define QDF_IPA_PIPE_IN_UL_RDY_RING_RP_VA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul.rdy_ring_rp_va)
#define QDF_IPA_PIPE_IN_UL_RDY_COMP_RING(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul.rdy_comp_ring_base_pa)
#define QDF_IPA_PIPE_IN_UL_RDY_COMP_RING_SIZE(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul.rdy_comp_ring_size)
#define QDF_IPA_PIPE_IN_UL_RDY_COMP_RING_WP_PA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul.rdy_comp_ring_wp_pa)
#define QDF_IPA_PIPE_IN_UL_RDY_COMP_RING_WP_VA(pipe_in)	\
	(((struct ipa_wdi_in_params *)(pipe_in))->u.ul.rdy_comp_ring_wp_va)

/**
 * __qdf_ipa_wdi_out_params_t - information provided to WDI client
 */
typedef struct ipa_wdi_out_params __qdf_ipa_wdi_out_params_t;

#define QDF_IPA_PIPE_OUT_UC_DOOR_BELL_PA(pipe_out)	\
	(((struct ipa_wdi_out_params *)(pipe_out))->uc_door_bell_pa)
#define QDF_IPA_PIPE_OUT_CLNT_HDL(pipe_out)	\
	(((struct ipa_wdi_out_params *)(pipe_out))->clnt_hdl)

/**
 * __qdf_ipa_wdi_db_params_t - information provided to retrieve
 *       physical address of uC doorbell
 */
typedef struct ipa_wdi_db_params __qdf_ipa_wdi_db_params_t;

/**
 * __qdf_ipa_wdi_uc_ready_params_t - uC ready CB parameters
 */
typedef void (*__qdf_ipa_uc_ready_cb)(void *priv);
typedef struct ipa_wdi_uc_ready_params __qdf_ipa_wdi_uc_ready_params_t;

#define QDF_IPA_UC_READY_PARAMS_IS_UC_READY(uc_ready_param)	\
	(((struct ipa_wdi_uc_ready_params *)(uc_ready_param))->is_uC_ready)
#define QDF_IPA_UC_READY_PARAMS_PRIV(uc_ready_param)	\
	(((struct ipa_wdi_uc_ready_params *)(uc_ready_param))->priv)
#define QDF_IPA_UC_READY_PARAMS_NOTIFY(uc_ready_param)	\
	(((struct ipa_wdi_uc_ready_params *)(uc_ready_param))->notify)

/**
 * __qdf_ipa_wdi_buffer_info_t - address info of a WLAN allocated buffer
 *
 * IPA driver will create/release IOMMU mapping in IPA SMMU from iova->pa
 */
typedef struct ipa_wdi_buffer_info __qdf_ipa_wdi_buffer_info_t;

/**
 * __qdf_ipa_gsi_ep_config_t - IPA GSI endpoint configurations
 */
typedef struct ipa_gsi_ep_config __qdf_ipa_gsi_ep_config_t;

#ifdef WDI3_STATS_UPDATE
/**
 * __qdf_ipa_wdi_tx_info_t - WLAN embedded TX information
 */
typedef struct ipa_wdi_tx_info __qdf_ipa_wdi_tx_info_t;

#define QDF_IPA_WDI_TX_INFO_STA_TX_BYTES(stats_info)	\
	(((struct ipa_wdi_tx_info *)stats_info)->sta_tx)
#define QDF_IPA_WDI_TX_INFO_SAP_TX_BYTES(stats_info)	\
	(((struct ipa_wdi_tx_info *)stats_info)->ap_tx)
/**
 * __qdf_ipa_wdi_bw_info_t - BW levels to be monitored by uC
 */
typedef struct ipa_wdi_bw_info __qdf_ipa_wdi_bw_info_t;

#define QDF_IPA_WDI_BW_INFO_THRESHOLD_LEVEL_1(bw_info)	\
	(((struct ipa_wdi_bw_info *)bw_info)->threshold[0])
#define QDF_IPA_WDI_BW_INFO_THRESHOLD_LEVEL_2(bw_info)	\
	(((struct ipa_wdi_bw_info *)bw_info)->threshold[1])
#define QDF_IPA_WDI_BW_INFO_THRESHOLD_LEVEL_3(bw_info)	\
	(((struct ipa_wdi_bw_info *)bw_info)->threshold[2])
#define QDF_IPA_WDI_BW_INFO_START_STOP(bw_info)	\
	(((struct ipa_wdi_bw_info *)bw_info)->stop)

/**
 * __qdf_ipa_inform_wlan_bw_t - BW information given by IPA driver
 */
typedef struct ipa_inform_wlan_bw  __qdf_ipa_inform_wlan_bw_t;

#define QDF_IPA_INFORM_WLAN_BW_INDEX(bw_inform)	\
	(((struct ipa_inform_wlan_bw*)bw_inform)->index)
#define QDF_IPA_INFORM_WLAN_BW_THROUGHPUT(bw_inform)	\
	(((struct ipa_inform_wlan_bw*)bw_inform)->throughput)

#endif /* WDI3_STATS_UPDATE */

/**
 * __qdf_ipa_dp_evt_type_t - type of event client callback is
 * invoked for on data path
 * @IPA_RECEIVE: data is struct sk_buff
 * @IPA_WRITE_DONE: data is struct sk_buff
 */
typedef enum ipa_dp_evt_type __qdf_ipa_dp_evt_type_t;

typedef struct ipa_hdr_add __qdf_ipa_hdr_add_t;
typedef struct ipa_hdr_del __qdf_ipa_hdr_del_t;
typedef struct ipa_ioc_add_hdr __qdf_ipa_ioc_add_hdr_t;

#define QDF_IPA_IOC_ADD_HDR_COMMIT(ipa_hdr)	\
	(((struct ipa_ioc_add_hdr *)(ipa_hdr))->commit)
#define QDF_IPA_IOC_ADD_HDR_NUM_HDRS(ipa_hdr)	\
	(((struct ipa_ioc_add_hdr *)(ipa_hdr))->num_hdrs)
#define QDF_IPA_IOC_ADD_HDR_NAME(ipa_hdr)	\
	(((struct ipa_ioc_add_hdr *)(ipa_hdr))->hdr[0].name)
#define QDF_IPA_IOC_ADD_HDR_HDR(ipa_hdr)	\
	(((struct ipa_ioc_add_hdr *)(ipa_hdr))->hdr[0].hdr)
#define QDF_IPA_IOC_ADD_HDR_HDR_LEN(ipa_hdr)	\
	(((struct ipa_ioc_add_hdr *)(ipa_hdr))->hdr[0].hdr_len)
#define QDF_IPA_IOC_ADD_HDR_TYPE(ipa_hdr)	\
	(((struct ipa_ioc_add_hdr *)(ipa_hdr))->hdr[0].type)
#define QDF_IPA_IOC_ADD_HDR_IS_PARTIAL(ipa_hdr)	\
	(((struct ipa_ioc_add_hdr *)(ipa_hdr))->hdr[0].is_partial)
#define QDF_IPA_IOC_ADD_HDR_HDR_HDL(ipa_hdr)	\
	(((struct ipa_ioc_add_hdr *)(ipa_hdr))->hdr[0].hdr_hdl)
#define QDF_IPA_IOC_ADD_HDR_STATUS(ipa_hdr)	\
	(((struct ipa_ioc_add_hdr *)(ipa_hdr))->hdr[0].status)
#define QDF_IPA_IOC_ADD_HDR_IS_ETH2_OFST_VALID(ipa_hdr)	\
	(((struct ipa_ioc_add_hdr *)(ipa_hdr))->hdr[0].is_eth2_ofst_valid)
#define QDF_IPA_IOC_ADD_HDR_ETH2_OFST(ipa_hdr)	\
	(((struct ipa_ioc_add_hdr *)(ipa_hdr))->hdr[0].eth2_ofst)

typedef struct ipa_ioc_del_hdr __qdf_ipa_ioc_del_hdr_t;

#define QDF_IPA_IOC_DEL_HDR_COMMIT(ipa_hdr)	\
	(((struct ipa_ioc_del_hdr *)(ipa_hdr))->commit)
#define QDF_IPA_IOC_DEL_HDR_NUM_HDRS(ipa_hdr)	\
	(((struct ipa_ioc_del_hdr *)(ipa_hdr))->num_hdls)
#define QDF_IPA_IOC_DEL_HDR_HDL(ipa_hdr)	\
	(((struct ipa_ioc_del_hdr *)(ipa_hdr))->hdl[0].hdl)
#define QDF_IPA_IOC_DEL_HDR_STATUS(ipa_hdr)	\
	(((struct ipa_ioc_del_hdr *)(ipa_hdr))->hdl[0].status)

typedef struct ipa_ioc_get_hdr __qdf_ipa_ioc_get_hdr_t;

#define QDF_IPA_IOC_GET_HDR_NAME(ipa_hdr)	\
	(((struct ipa_ioc_get_hdr *)(ipa_hdr))->name)
#define QDF_IPA_IOC_GET_HDR_HDL(ipa_hdr)	\
	(((struct ipa_ioc_get_hdr *)(ipa_hdr))->hdl)

typedef struct ipa_ioc_copy_hdr __qdf_ipa_ioc_copy_hdr_t;
typedef struct ipa_ioc_add_hdr_proc_ctx __qdf_ipa_ioc_add_hdr_proc_ctx_t;
typedef struct ipa_ioc_del_hdr_proc_ctx __qdf_ipa_ioc_del_hdr_proc_ctx_t;
typedef struct ipa_msg_meta __qdf_ipa_msg_meta_t;

#define QDF_IPA_MSG_META_MSG_TYPE(meta)	\
	(((struct ipa_msg_meta *)(meta))->msg_type)
#define QDF_IPA_MSG_META_MSG_LEN(meta)	\
	(((struct ipa_msg_meta *)(meta))->msg_len)

typedef enum ipa_client_type __qdf_ipa_client_type_t;
typedef struct IpaHwStatsWDIInfoData_t __qdf_ipa_hw_stats_wdi_info_data_t;
typedef enum ipa_rm_resource_name __qdf_ipa_rm_resource_name_t;
typedef enum ipa_wlan_event __qdf_ipa_wlan_event_t;
typedef struct ipa_wlan_msg __qdf_ipa_wlan_msg_t;

#define QDF_IPA_WLAN_MSG_NAME(ipa_msg)	\
	(((struct ipa_wlan_msg *)(ipa_msg))->name)
#define QDF_IPA_WLAN_MSG_MAC_ADDR(ipa_msg)	\
	(((struct ipa_wlan_msg *)(ipa_msg))->mac_addr)

typedef struct ipa_wlan_msg_ex __qdf_ipa_wlan_msg_ex_t;

#define QDF_IPA_WLAN_MSG_EX_NAME(ipa_msg)	\
	(((struct ipa_wlan_msg_ex *)(ipa_msg))->name)
#define QDF_IPA_WLAN_MSG_EX_EXNUM_OF_ATTRIBS(ipa_msg)	\
	(((struct ipa_wlan_msg_ex *)(ipa_msg))->num_of_attribs)
#define QDF_IPA_WLAN_MSG_EX_ATTRIB_TYPE(ipa_msg)	\
	(((struct ipa_wlan_msg_ex *)(ipa_msg))->attribs.attrib_type)
#define QDF_IPA_WLAN_MSG_EX_OFFSET(ipa_msg)	\
	(((struct ipa_wlan_msg_ex *)(ipa_msg))->attribs.offset)
#define QDF_IPA_WLAN_MSG_EX_MAC_ADDR(ipa_msg)	\
	(((struct ipa_wlan_msg_ex *)(ipa_msg))->attribs.u.mac_addr)

typedef struct ipa_ioc_tx_intf_prop __qdf_ipa_ioc_tx_intf_prop_t;

#define QDF_IPA_IOC_TX_INTF_PROP_IP(tx_prop)	\
	(((struct ipa_ioc_tx_intf_prop *)(tx_prop))->ip)
#define QDF_IPA_IOC_TX_INTF_PROP_ATTRIB_MASK(tx_prop)	\
	(((struct ipa_ioc_tx_intf_prop *)(tx_prop))->attrib.attrib_mask)
#define QDF_IPA_IOC_TX_INTF_PROP_META_DATA(rx_prop)	\
	(((struct ipa_ioc_tx_intf_prop *)(tx_prop))->attrib.meta_data)
#define QDF_IPA_IOC_TX_INTF_PROP_META_DATA_MASK(rx_prop)	\
	(((struct ipa_ioc_tx_intf_prop *)(tx_prop))->attrib.meta_data_mask)
#define QDF_IPA_IOC_TX_INTF_PROP_DST_PIPE(tx_prop)	\
	(((struct ipa_ioc_tx_intf_prop *)(tx_prop))->dst_pipe)
#define QDF_IPA_IOC_TX_INTF_PROP_ALT_DST_PIPE(tx_prop)	\
	(((struct ipa_ioc_tx_intf_prop *)(tx_prop))->alt_dst_pipe)
#define QDF_IPA_IOC_TX_INTF_PROP_HDR_NAME(tx_prop)	\
	(((struct ipa_ioc_tx_intf_prop *)(tx_prop))->hdr_name)
#define QDF_IPA_IOC_TX_INTF_PROP_HDR_L2_TYPE(tx_prop)	\
	(((struct ipa_ioc_tx_intf_prop *)(tx_prop))->hdr_l2_type)

typedef struct ipa_ioc_rx_intf_prop __qdf_ipa_ioc_rx_intf_prop_t;

#define QDF_IPA_IOC_RX_INTF_PROP_IP(rx_prop)	\
	(((struct ipa_ioc_rx_intf_prop *)(rx_prop))->ip)
#define QDF_IPA_IOC_RX_INTF_PROP_ATTRIB_MASK(rx_prop)	\
	(((struct ipa_ioc_rx_intf_prop *)(rx_prop))->attrib.attrib_mask)
#define QDF_IPA_IOC_RX_INTF_PROP_META_DATA(rx_prop)	\
	(((struct ipa_ioc_rx_intf_prop *)(rx_prop))->attrib.meta_data)
#define QDF_IPA_IOC_RX_INTF_PROP_META_DATA_MASK(rx_prop)	\
	(((struct ipa_ioc_rx_intf_prop *)(rx_prop))->attrib.meta_data_mask)
#define QDF_IPA_IOC_RX_INTF_PROP_SRC_PIPE(rx_prop)	\
	(((struct ipa_ioc_rx_intf_prop *)(rx_prop))->src_pipe)
#define QDF_IPA_IOC_RX_INTF_PROP_HDR_L2_TYPE(rx_prop)	\
	(((struct ipa_ioc_rx_intf_prop *)(rx_prop))->hdr_l2_type)

typedef struct ipa_wlan_hdr_attrib_val __qdf_ipa_wlan_hdr_attrib_val_t;

#define __QDF_IPA_SET_META_MSG_TYPE(meta, msg_type)	\
	__qdf_ipa_set_meta_msg_type(meta, msg_type)

#define __QDF_IPA_RM_RESOURCE_GRANTED IPA_RM_RESOURCE_GRANTED
#define __QDF_IPA_RM_RESOURCE_RELEASED IPA_RM_RESOURCE_RELEASED

#define __QDF_IPA_RM_RESOURCE_WLAN_PROD IPA_RM_RESOURCE_WLAN_PROD
#define __QDF_IPA_RM_RESOURCE_WLAN_CONS IPA_RM_RESOURCE_WLAN_CONS
#define __QDF_IPA_RM_RESOURCE_APPS_CONS IPA_RM_RESOURCE_APPS_CONS

#define __QDF_IPA_VOLTAGE_LEVEL IPA_VOLTAGE_SVS

#define __QDF_IPA_CLIENT_WLAN1_PROD IPA_CLIENT_WLAN1_PROD
#define __QDF_IPA_CLIENT_WLAN3_PROD IPA_CLIENT_WLAN3_PROD
#define __QDF_IPA_CLIENT_WLAN1_CONS IPA_CLIENT_WLAN1_CONS
#define __QDF_IPA_CLIENT_WLAN2_CONS IPA_CLIENT_WLAN2_CONS
#define __QDF_IPA_CLIENT_WLAN3_CONS IPA_CLIENT_WLAN3_CONS
#define __QDF_IPA_CLIENT_WLAN4_CONS IPA_CLIENT_WLAN4_CONS
#define __QDF_IPA_CLIENT_WLAN4_PROD IPA_CLIENT_WLAN4_PROD

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0)) && \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0))
#define __QDF_IPA_CLIENT_WLAN_WDI2_CONS IPA_CLIENT_WLAN4_CONS
#define __QDF_IPA_CLIENT_WLAN_WDI2_PROD IPA_CLIENT_WLAN4_PROD
#else
#define __QDF_IPA_CLIENT_WLAN_WDI2_CONS IPA_CLIENT_WLAN1_CONS
#define __QDF_IPA_CLIENT_WLAN_WDI2_PROD IPA_CLIENT_WLAN1_PROD
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
#define IPA_LAN_RX_NAPI_SUPPORT
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0))
/*
 * Resume / Suspend
 */
static inline int __qdf_ipa_reset_endpoint(u32 clnt_hdl)
{
	return ipa_reset_endpoint(clnt_hdl);
}

/*
 * Remove ep delay
 */
static inline int __qdf_ipa_clear_endpoint_delay(u32 clnt_hdl)
{
	return ipa_clear_endpoint_delay(clnt_hdl);
}

/*
 * Header removal / addition
 */
static inline int __qdf_ipa_add_hdr(struct ipa_ioc_add_hdr *hdrs)
{
	return ipa_add_hdr(hdrs);
}

static inline int __qdf_ipa_del_hdr(struct ipa_ioc_del_hdr *hdls)
{
	return ipa_del_hdr(hdls);
}

static inline int __qdf_ipa_commit_hdr(void)
{
	return ipa_commit_hdr();
}

static inline int __qdf_ipa_get_hdr(struct ipa_ioc_get_hdr *lookup)
{
	return ipa_get_hdr(lookup);
}

static inline int __qdf_ipa_put_hdr(u32 hdr_hdl)
{
	return ipa_put_hdr(hdr_hdl);
}

static inline int __qdf_ipa_copy_hdr(struct ipa_ioc_copy_hdr *copy)
{
	return ipa_copy_hdr(copy);
}

static inline int __qdf_ipa_register_pull_msg(struct ipa_msg_meta *meta,
		ipa_msg_pull_fn callback)
{
	return ipa_register_pull_msg(meta, callback);
}

static inline int __qdf_ipa_deregister_pull_msg(struct ipa_msg_meta *meta)
{
	return ipa_deregister_pull_msg(meta);
}

/*
 * Interface
 */
static inline int __qdf_ipa_register_intf(const char *name,
				     const struct ipa_tx_intf *tx,
				     const struct ipa_rx_intf *rx)
{
	return ipa_register_intf(name, tx, rx);
}

static inline int __qdf_ipa_register_intf_ext(const char *name,
		const struct ipa_tx_intf *tx,
		const struct ipa_rx_intf *rx,
		const struct ipa_ext_intf *ext)
{
	return ipa_register_intf_ext(name, tx, rx, ext);
}

static inline int __qdf_ipa_deregister_intf(const char *name)
{
	return ipa_deregister_intf(name);
}

/*
 * Data path
 */
static inline int __qdf_ipa_tx_dp(enum ipa_client_type dst, struct sk_buff *skb,
		struct ipa_tx_meta *metadata)
{
	return ipa_tx_dp(dst, skb, metadata);
}

/*
 * To transfer multiple data packets
 */
static inline int __qdf_ipa_tx_dp_mul(
	enum ipa_client_type dst,
	struct ipa_tx_data_desc *data_desc)
{
	return ipa_tx_dp_mul(dst, data_desc);
}

/*
 * System pipes
 */
static inline u16 __qdf_ipa_get_smem_restr_bytes(void)
{
	return ipa_get_smem_restr_bytes();
}

static inline int __qdf_ipa_connect_wdi_pipe(struct ipa_wdi_in_params *in,
		struct ipa_wdi_out_params *out)
{
	return ipa_connect_wdi_pipe(in, out);
}

static inline int __qdf_ipa_disconnect_wdi_pipe(u32 clnt_hdl)
{
	return ipa_disconnect_wdi_pipe(clnt_hdl);
}

static inline int __qdf_ipa_enable_wdi_pipe(u32 clnt_hdl)
{
	return ipa_enable_wdi_pipe(clnt_hdl);
}

static inline int __qdf_ipa_disable_wdi_pipe(u32 clnt_hdl)
{
	return ipa_disable_wdi_pipe(clnt_hdl);
}

static inline int __qdf_ipa_resume_wdi_pipe(u32 clnt_hdl)
{
	return ipa_resume_wdi_pipe(clnt_hdl);
}

static inline int __qdf_ipa_suspend_wdi_pipe(u32 clnt_hdl)
{
	return ipa_suspend_wdi_pipe(clnt_hdl);
}

static inline int __qdf_ipa_uc_wdi_get_dbpa(
	struct ipa_wdi_db_params *out)
{
	return ipa_uc_wdi_get_dbpa(out);
}

/*
 * Resource manager
 */
static inline int __qdf_ipa_rm_create_resource(
		struct ipa_rm_create_params *create_params)
{
	return ipa_rm_create_resource(create_params);
}

static inline int __qdf_ipa_rm_delete_resource(
		enum ipa_rm_resource_name resource_name)
{
	return ipa_rm_delete_resource(resource_name);
}

static inline int __qdf_ipa_rm_register(enum ipa_rm_resource_name resource_name,
			struct ipa_rm_register_params *reg_params)
{
	return ipa_rm_register(resource_name, reg_params);
}

static inline int __qdf_ipa_rm_set_perf_profile(
		enum ipa_rm_resource_name resource_name,
		struct ipa_rm_perf_profile *profile)
{
	return ipa_rm_set_perf_profile(resource_name, profile);
}

static inline int __qdf_ipa_rm_deregister(enum ipa_rm_resource_name resource_name,
			struct ipa_rm_register_params *reg_params)
{
	return ipa_rm_deregister(resource_name, reg_params);
}

static inline int __qdf_ipa_rm_add_dependency(
		enum ipa_rm_resource_name resource_name,
		enum ipa_rm_resource_name depends_on_name)
{
	return ipa_rm_add_dependency(resource_name, depends_on_name);
}

static inline int __qdf_ipa_rm_add_dependency_sync(
		enum ipa_rm_resource_name resource_name,
		enum ipa_rm_resource_name depends_on_name)
{
	return ipa_rm_add_dependency_sync(resource_name, depends_on_name);
}

static inline int __qdf_ipa_rm_delete_dependency(
		enum ipa_rm_resource_name resource_name,
		enum ipa_rm_resource_name depends_on_name)
{
	return ipa_rm_delete_dependency(resource_name, depends_on_name);
}

static inline int __qdf_ipa_rm_request_resource(
		enum ipa_rm_resource_name resource_name)
{
	return ipa_rm_request_resource(resource_name);
}

static inline int __qdf_ipa_rm_release_resource(
		enum ipa_rm_resource_name resource_name)
{
	return ipa_rm_release_resource(resource_name);
}

static inline int __qdf_ipa_rm_notify_completion(enum ipa_rm_event event,
		enum ipa_rm_resource_name resource_name)
{
	return ipa_rm_notify_completion(event, resource_name);
}

static inline int __qdf_ipa_rm_inactivity_timer_init(
		enum ipa_rm_resource_name resource_name,
			unsigned long msecs)
{
	return ipa_rm_inactivity_timer_init(resource_name, msecs);
}

static inline int __qdf_ipa_rm_inactivity_timer_destroy(
		enum ipa_rm_resource_name resource_name)
{
	return ipa_rm_inactivity_timer_destroy(resource_name);
}

static inline int __qdf_ipa_rm_inactivity_timer_request_resource(
				enum ipa_rm_resource_name resource_name)
{
	return ipa_rm_inactivity_timer_request_resource(resource_name);
}

static inline int __qdf_ipa_rm_inactivity_timer_release_resource(
				enum ipa_rm_resource_name resource_name)
{
	return ipa_rm_inactivity_timer_release_resource(resource_name);
}

/*
 * Miscellaneous
 */
static inline void __qdf_ipa_bam_reg_dump(void)
{
	return ipa_bam_reg_dump();
}

static inline int __qdf_ipa_get_ep_mapping(enum ipa_client_type client)
{
	return ipa_get_ep_mapping(client);
}

static inline void __qdf_ipa_proxy_clk_vote(void)
{
	return ipa_proxy_clk_vote();
}

static inline void __qdf_ipa_proxy_clk_unvote(void)
{
	return ipa_proxy_clk_unvote();
}

static inline bool __qdf_ipa_is_client_handle_valid(u32 clnt_hdl)
{
	return ipa_is_client_handle_valid(clnt_hdl);
}

static inline enum ipa_client_type __qdf_ipa_get_client_mapping(int pipe_idx)
{
	return ipa_get_client_mapping(pipe_idx);
}

static inline enum ipa_rm_resource_name __qdf_ipa_get_rm_resource_from_ep(
	int pipe_idx)
{
	return ipa_get_rm_resource_from_ep(pipe_idx);
}

static inline bool __qdf_ipa_get_modem_cfg_emb_pipe_flt(void)
{
	return ipa_get_modem_cfg_emb_pipe_flt();
}

static inline enum ipa_transport_type __qdf_ipa_get_transport_type(void)
{
	return ipa_get_transport_type();
}

static inline struct device *__qdf_ipa_get_dma_dev(void)
{
	return ipa_get_dma_dev();
}

static inline struct iommu_domain *__qdf_ipa_get_smmu_domain(void)
{
	return ipa_get_smmu_domain();
}

static inline int __qdf_ipa_create_wdi_mapping(u32 num_buffers,
		__qdf_ipa_wdi_buffer_info_t *info)
{
	return ipa_create_wdi_mapping(num_buffers, info);
}

static inline int __qdf_ipa_release_wdi_mapping(u32 num_buffers,
		struct ipa_wdi_buffer_info *info)
{
	return ipa_release_wdi_mapping(num_buffers, info);
}

static inline int __qdf_ipa_disable_apps_wan_cons_deaggr(uint32_t agg_size, uint32_t agg_count)
{
	return ipa_disable_apps_wan_cons_deaggr(agg_size, agg_count);
}

static inline const struct ipa_gsi_ep_config *__qdf_ipa_get_gsi_ep_info(enum ipa_client_type client)
{
	return ipa_get_gsi_ep_info(client);
}

static inline int __qdf_ipa_stop_gsi_channel(u32 clnt_hdl)
{
	return ipa_stop_gsi_channel(clnt_hdl);
}

#endif
static inline void __qdf_ipa_free_skb(struct ipa_rx_data *rx_in)
{
	return ipa_free_skb(rx_in);
}

static inline int __qdf_ipa_get_wdi_stats(struct IpaHwStatsWDIInfoData_t *stats)
{
	return ipa_get_wdi_stats(stats);
}

static inline int __qdf_ipa_uc_reg_rdyCB(struct ipa_wdi_uc_ready_params *param)
{
	return ipa_uc_reg_rdyCB(param);
}

static inline int __qdf_ipa_uc_dereg_rdyCB(void)
{
	return ipa_uc_dereg_rdyCB();
}

static inline int __qdf_ipa_register_ipa_ready_cb(
	void (*ipa_ready_cb)(void *user_data),
	void *user_data)
{
	return ipa_register_ipa_ready_cb(ipa_ready_cb, user_data);
}

static inline
int __qdf_ipa_setup_sys_pipe(struct ipa_sys_connect_params *sys_in,
			     u32 *clnt_hdl)
{
	return ipa_setup_sys_pipe(sys_in, clnt_hdl);
}

static inline int __qdf_ipa_teardown_sys_pipe(u32 clnt_hdl)
{
	return ipa_teardown_sys_pipe(clnt_hdl);
}

/*
 * Messaging
 */
static inline int __qdf_ipa_send_msg(struct ipa_msg_meta *meta, void *buff,
				     ipa_msg_free_fn callback)
{
	return ipa_send_msg(meta, buff, callback);
}

static inline bool __qdf_ipa_is_ready(void)
{
	return ipa_is_ready();
}

#ifdef FEATURE_METERING
static inline int __qdf_ipa_broadcast_wdi_quota_reach_ind(uint32_t index,
							  uint64_t quota_bytes)
{
	return ipa_broadcast_wdi_quota_reach_ind(index, quota_bytes);
}
#endif

void __qdf_ipa_set_meta_msg_type(__qdf_ipa_msg_meta_t *meta, int type);

#ifdef ENABLE_SMMU_S1_TRANSLATION
/**
 * __qdf_get_ipa_smmu_enabled() - to get IPA SMMU enable status
 *
 * Return: true when IPA SMMU enabled, otherwise false
 */
static bool __qdf_get_ipa_smmu_enabled(void)
{
	struct ipa_smmu_in_params params_in;
	struct ipa_smmu_out_params params_out;

	params_in.smmu_client = IPA_SMMU_WLAN_CLIENT;
	ipa_get_smmu_params(&params_in, &params_out);

	return params_out.smmu_enable;
}
#endif

#ifdef IPA_LAN_RX_NAPI_SUPPORT
/**
 * ipa_get_lan_rx_napi() - Check if NAPI is enabled in LAN RX DP
 *
 * Returns: true if enabled, false otherwise
 */
static inline bool __qdf_ipa_get_lan_rx_napi(void)
{
	return ipa_get_lan_rx_napi();
}
#endif /* IPA_LAN_RX_NAPI_SUPPORT */
#endif /* IPA_OFFLOAD */
#endif /* _I_QDF_IPA_H */
