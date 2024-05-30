/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#ifndef _IPA_LNX_STATS_I_H_
#define _IPA_LNX_STATS_I_H_

/* This whole header file is a copy of ipa_lnx_agent.h */

/*
 * unique magic number of the IPA_LNX_STATS interface
 */
#define IPA_LNX_STATS_IOC_MAGIC 0x72

#define IPA_LNX_IOC_GET_ALLOC_INFO _IOWR(IPA_LNX_STATS_IOC_MAGIC, \
	IPA_LNX_CMD_GET_ALLOC_INFO, \
	struct ipa_lnx_stats_spearhead_ctx)

#define IPA_LNX_IOC_GET_GENERIC_STATS _IOWR(IPA_LNX_STATS_IOC_MAGIC, \
	IPA_LNX_CMD_GENERIC_STATS, \
	struct ipa_lnx_generic_stats)

#define IPA_LNX_IOC_GET_CLOCK_STATS _IOWR(IPA_LNX_STATS_IOC_MAGIC, \
	IPA_LNX_CMD_CLOCK_STATS, \
	struct ipa_lnx_clock_stats)

#define IPA_LNX_IOC_GET_WLAN_INST_STATS _IOWR(IPA_LNX_STATS_IOC_MAGIC, \
	IPA_LNX_CMD_WLAN_INST_STATS, \
	struct ipa_lnx_wlan_inst_stats)

#define IPA_LNX_IOC_GET_ETH_INST_STATS _IOWR(IPA_LNX_STATS_IOC_MAGIC, \
	IPA_LNX_CMD_ETH_INST_STATS, \
	struct ipa_lnx_eth_inst_stats)

#define IPA_LNX_IOC_GET_USB_INST_STATS _IOWR(IPA_LNX_STATS_IOC_MAGIC, \
	IPA_LNX_CMD_USB_INST_STATS, \
	struct ipa_lnx_usb_inst_stats)

#define IPA_LNX_IOC_GET_MHIP_INST_STATS _IOWR(IPA_LNX_STATS_IOC_MAGIC, \
	IPA_LNX_CMD_MHIP_INST_STATS, \
	struct ipa_lnx_mhip_inst_stats)

#define IPA_LNX_IOC_GET_CONSOLIDATED_STATS _IOWR(IPA_LNX_STATS_IOC_MAGIC, \
	IPA_LNX_CMD_CONSOLIDATED_STATS, \
	struct ipa_lnx_consolidated_stats)

#define IPA_LNX_STATS_SUCCESS 0
#define IPA_LNX_STATS_FAILURE -1

#define SPEARHEAD_NUM_MAX_PIPES 6
#define SPEARHEAD_NUM_MAX_TX_INSTANCES 3
#define SPEARHEAD_NUM_MAX_RX_INSTANCES 3

#define SPEARHEAD_NUM_MAX_INSTANCES 2

#define IPA_LNX_PIPE_PAGE_RECYCLING_INTERVAL_COUNT 5
#define IPA_LNX_PIPE_PAGE_RECYCLING_INTERVAL_TIME 10 /* In milli second */

/**
 * This is used to indicate which set of logs is enabled from IPA
 * These bitmapped macros are copied from
 * spearhead/inc/spearhead_client.h
 */
#define SPRHD_IPA_LOG_TYPE_GENERIC_STATS   0x00001
#define SPRHD_IPA_LOG_TYPE_CLOCK_STATS     0x00002
#define SPRHD_IPA_LOG_TYPE_WLAN_STATS      0x00004
#define SPRHD_IPA_LOG_TYPE_ETH_STATS       0x00008
#define SPRHD_IPA_LOG_TYPE_USB_STATS       0x00010
#define SPRHD_IPA_LOG_TYPE_MHIP_STATS      0x00020
#define SPRHD_IPA_LOG_TYPE_RECYCLE_STATS   0x00040


/**
 * Look up table for pm stats client names.
 * New entry to be added when new client
 * registers with pm
 */
struct pm_client_name_lookup { char *name; int idx_hdl;};
static struct pm_client_name_lookup client_lookup_table[] = {
	{"ODL", 1},
	{"IPA_CLIENT_APPS_LAN_CONS", 2},
	{"EMB MODEM", 3},
	{"TETH MODEM", 4},
	{"rmnet_ipa%d", 5},
	{"USB", 6},
	{"USB DPL", 7},
	{"MODEM (USB RMNET)", 8},
	{"IPA_CLIENT_APPS_WAN_CONS", 9}
};

#define NUM_PM_CLIENT_NAMES (sizeof(client_lookup_table)/sizeof(struct pm_client_name_lookup))

/**
 * Every structure is associated with the underlying macro
 * for it's length and that has to be updated every time there
 * is structure modification.This is NOT the sizeof(struct) but
 * it is addition of the specified type of variable included
 * inside the structre. Also update the internal structure lengths
 * in ipa_lnx_spearhead_stats.c to overcome backward and forward
 * compatibility between userspace and driver structures.
 */
/* IPA Linux generic stats structures */
struct pg_recycle_stats {
	uint64_t coal_total_repl_buff;
	uint64_t coal_temp_repl_buff;
	uint64_t def_total_repl_buff;
	uint64_t def_temp_repl_buff;
};
#define IPA_LNX_PG_RECYCLE_STATS_STRUCT_LEN_INT 32

struct exception_stats {
	uint32_t excptn_type_none;
	uint32_t excptn_type_deaggr;
	uint32_t excptn_type_iptype;
	uint32_t excptn_type_pkt_len;
	uint32_t excptn_type_pkt_thrshld;
	uint32_t excptn_type_frag_rule_miss;
	uint32_t excptn_type_sw_flt;
	uint32_t excptn_type_nat;
	uint32_t excptn_type_ipv6_ct;
	uint32_t excptn_type_csum;
};
#define IPA_LNX_EXCEPTION_STATS_STRUCT_LEN_INT 40

struct odl_ep_stats {
	uint32_t rx_pkt;
	uint32_t processed_pkt;
	uint32_t dropped_pkt;
	uint32_t num_queue_pkt;
};
#define IPA_LNX_ODL_EP_STATS_STRUCT_LEN_INT 16

struct holb_discard_stats {
	uint32_t client_type;
	uint32_t num_drp_cnt;
	uint32_t num_drp_bytes;
	uint32_t reserved;
};
#define IPA_LNX_HOLB_DISCARD_STATS_STRUCT_LEN_INT 16

struct holb_monitor_stats {
	uint32_t client_type;
	uint32_t curr_index;
	uint32_t num_en_cnt;
	uint32_t num_dis_cnt;
};
#define IPA_LNX_HOLB_MONITOR_STATS_STRUCT_LEN_INT 16

struct holb_drop_and_mon_stats {
	uint32_t num_holb_disc_pipes;
	uint32_t num_holb_mon_clients;
	struct holb_discard_stats holb_disc_stats[0];
	struct holb_monitor_stats holb_mon_stats[0];
};
#define IPA_LNX_HOLB_DROP_AND_MON_STATS_STRUCT_LEN_INT (8 + 16 +16)

struct ipa_lnx_generic_stats {
	uint32_t tx_dma_pkts;
	uint32_t tx_hw_pkts;
	uint32_t tx_non_linear;
	uint32_t tx_pkts_compl;
	uint32_t stats_compl;
	uint32_t active_eps;
	uint32_t wan_rx_empty;
	uint32_t wan_repl_rx_empty;
	uint32_t lan_rx_empty;
	uint32_t lan_repl_rx_empty;
	struct pg_recycle_stats pg_rec_stats;
	struct exception_stats excep_stats;
	struct odl_ep_stats odl_stats;
	struct holb_drop_and_mon_stats holb_stats;
};
#define IPA_LNX_GENERIC_STATS_STRUCT_LEN_INT (40 + 32 + 40 + 16 + 40)

/* IPA Linux clock stats structures */
struct pm_client_stats {
	uint32_t pm_client_state;
	uint32_t pm_client_group;
	uint32_t pm_client_bw;
	uint32_t pm_client_hdl;
	uint32_t pm_client_type;
	uint32_t reserved;
};
#define IPA_LNX_PM_CLIENT_STATS_STRUCT_LEN_INT 24

struct ipa_lnx_clock_stats {
	uint32_t active_clients;
	uint32_t scale_thresh_svs;
	uint32_t scale_thresh_nom;
	uint32_t scale_thresh_tur;
	uint32_t aggr_bw;
	uint32_t curr_clk_vote;
	struct pm_client_stats pm_clnt_stats[0];
};
#define IPA_LNX_CLOCK_STATS_STRUCT_LEN_INT (24 + 24)

/* Generic instance structures */
struct ipa_lnx_gsi_rx_debug_stats {
	uint32_t rx_client;
	uint32_t num_rx_ring_100_perc_with_pack;
	uint32_t num_rx_ring_0_perc_with_pack;
	uint32_t num_rx_ring_above_75_perc_pack;
	uint32_t num_rx_ring_above_25_perc_pack;
	uint32_t num_rx_ring_stats_polled;
	uint32_t num_rx_drop_stats;
	uint32_t gsi_debug1;
	uint32_t gsi_debug2;
	uint32_t gsi_debug3;
	uint32_t gsi_debug4;
	uint32_t rx_summary;
};
#define IPA_LNX_GSI_RX_DEBUG_STATS_STRUCT_LEN_INT 48

struct ipa_lnx_gsi_tx_debug_stats {
	uint32_t tx_client;
	uint32_t num_tx_ring_100_perc_with_cred;
	uint32_t num_tx_ring_0_perc_with_cred;
	uint32_t num_tx_ring_above_75_perc_cred;
	uint32_t num_tx_ring_above_25_perc_cred;
	uint32_t num_tx_ring_stats_polled;
	uint32_t num_tx_oob;
	uint32_t num_tx_oob_time;
	uint32_t gsi_debug1;
	uint32_t gsi_debug2;
	uint32_t gsi_debug3;
	uint32_t gsi_debug4;
	uint32_t tx_summary;
	uint32_t reserved;
};
#define IPA_LNX_GSI_TX_DEBUG_STATS_STRUCT_LEN_INT 56

struct ipa_lnx_gsi_debug_stats {
	uint32_t num_tx_instances;
	uint32_t num_rx_instances;
	struct ipa_lnx_gsi_tx_debug_stats gsi_tx_dbg_stats[0];
	struct ipa_lnx_gsi_rx_debug_stats gsi_rx_dbg_stats[0];
};
#define IPA_LNX_GSI_DEBUG_STATS_STRUCT_LEN_INT (8 + 48 + 56)

struct ipa_lnx_pipe_info {
	uint64_t gsi_chan_ring_bp;
	uint64_t gsi_chan_ring_rp;
	uint64_t gsi_chan_ring_wp;
	uint64_t gsi_evt_ring_bp;
	uint64_t gsi_evt_ring_rp;
	uint64_t gsi_evt_ring_wp;
	uint32_t gsi_evt_ring_len;
	uint32_t gsi_chan_ring_len;
	uint32_t buff_size;
	uint32_t num_free_buff;
	uint32_t gsi_ipa_if_tlv;
	uint32_t gsi_ipa_if_aos;
	uint32_t gsi_desc_size;
	uint32_t pipe_num;
	uint32_t direction;
	uint32_t client_type;
	uint32_t gsi_chan_num;
	uint32_t gsi_evt_num;
	uint32_t is_common_evt_ring;
	uint32_t gsi_prot_type;
	uint32_t gsi_chan_state;
	uint32_t gsi_chan_stop_stm;
	uint32_t gsi_poll_mode;
	uint32_t gsi_db_in_bytes;
};
#define IPA_LNX_PIPE_INFO_STATS_STRUCT_LEN_INT 120

/* IPA Linux wlan instance stats structures */
struct wlan_instance_info {
	uint32_t instance_id;
	uint32_t wdi_ver;
	uint32_t wlan_mode;
	uint32_t wdi_over_gsi;
	uint32_t dbs_mode;
	uint32_t pm_bandwidth;
	uint32_t num_pipes;
	uint32_t reserved;
	struct ipa_lnx_gsi_debug_stats gsi_debug_stats;
	struct ipa_lnx_pipe_info pipe_info[0];
};
#define IPA_LNX_WLAN_INSTANCE_INFO_STRUCT_LEN_INT (32 + 112 + 120)

struct ipa_lnx_wlan_inst_stats {
	uint32_t num_wlan_instance;
	uint32_t reserved;
	struct wlan_instance_info instance_info[0];
};
#define IPA_LNX_WLAN_INST_STATS_STRUCT_LEN_INT (8 + 264)

/* IPA Linux eth instance stats structures */
struct eth_instance_info {
	uint32_t instance_id;
	uint32_t eth_mode;
	uint32_t pm_bandwidth;
	uint32_t num_pipes;
	struct ipa_lnx_gsi_debug_stats gsi_debug_stats;
	struct ipa_lnx_pipe_info pipe_info[0];
};
#define IPA_LNX_ETH_INSTANCE_INFO_STRUCT_LEN_INT (16 + 112 + 120)

struct ipa_lnx_eth_inst_stats {
	uint32_t num_eth_instance;
	uint32_t reserved;
	struct eth_instance_info instance_info[0];
};
#define IPA_LNX_ETH_INST_STATS_STRUCT_LEN_INT (8 + 248)

/* IPA Linux usb instance stats structures */
struct usb_instance_info {
	uint32_t instance_id;
	uint32_t usb_mode;
	uint32_t pm_bandwidth;
	uint32_t num_pipes;
	struct ipa_lnx_gsi_debug_stats gsi_debug_stats;
	struct ipa_lnx_pipe_info pipe_info[0];
};
#define IPA_LNX_USB_INSTANCE_INFO_STRUCT_LEN_INT (16 + 112 + 120)

struct ipa_lnx_usb_inst_stats {
	uint32_t num_usb_instance;
	uint32_t reserved;
	struct usb_instance_info instance_info[0];
};
#define IPA_LNX_USB_INST_STATS_STRUCT_LEN_INT (8 + 248)

/* IPA Linux mhip instance stats structures */
struct mhip_instance_info {
	uint32_t instance_id;
	uint32_t mhip_mode;
	uint32_t pm_bandwidth;
	uint32_t num_pipes;
	struct ipa_lnx_gsi_debug_stats gsi_debug_stats;
	struct ipa_lnx_pipe_info pipe_info[0];
};
#define IPA_LNX_MHIP_INSTANCE_INFO_STRUCT_LEN_INT (16 + 112 + 120)

struct ipa_lnx_mhip_inst_stats {
	uint32_t num_mhip_instance;
	uint32_t reserved;
	struct mhip_instance_info instance_info[0];
};
#define IPA_LNX_MHIP_INST_STATS_STRUCT_LEN_INT (8 + 248)

struct ipa_lnx_consolidated_stats {
	uint64_t log_type_mask;
	struct ipa_lnx_generic_stats *generic_stats;
	struct ipa_lnx_clock_stats *clock_stats;
	struct ipa_lnx_wlan_inst_stats *wlan_stats;
	struct ipa_lnx_eth_inst_stats *eth_stats;
	struct ipa_lnx_usb_inst_stats *usb_stats;
	struct ipa_lnx_mhip_inst_stats *mhip_stats;
	struct ipa_lnx_pipe_page_recycling_stats *recycle_stats;
};
#define IPA_LNX_CONSOLIDATED_STATS_STRUCT_LEN_INT (8 + 48)

enum rx_channel_type {
	RX_WAN_COALESCING,
	RX_WAN_DEFAULT,
	RX_WAN_LOW_LAT_DATA,
	RX_CHANNEL_MAX,
};

struct ipa_lnx_recycling_stats {
	uint64_t total_cumulative;
	uint64_t recycle_cumulative;
	uint64_t temp_cumulative;
	uint64_t total_diff;
	uint64_t recycle_diff;
	uint64_t temp_diff;
	uint64_t valid;
};

/**
 * The consolidated stats will be in the 0th index.
 * Diff. between each interval values will be in
 * indices 1 to (IPA_LNX_PIPE_PAGE_RECYCLING_INTERVAL_COUNT - 1)
 * @new_set: Indicates if this is the new set of data or previous data.
 * @interval_time_ms: Interval time in millisecond
 */
struct ipa_lnx_pipe_page_recycling_stats {
	uint32_t interval_time_in_ms;
	uint32_t default_coal_stats_index;
	uint32_t low_lat_stats_index;
	uint32_t sequence_id;
	uint64_t reserved;
	struct ipa_lnx_recycling_stats rx_channel[RX_CHANNEL_MAX][IPA_LNX_PIPE_PAGE_RECYCLING_INTERVAL_COUNT];
};

/* Explain below structures */
struct ipa_lnx_each_inst_alloc_info {
	uint32_t pipes_client_type[SPEARHEAD_NUM_MAX_PIPES];
	uint32_t tx_inst_client_type[SPEARHEAD_NUM_MAX_TX_INSTANCES];
	uint32_t rx_inst_client_type[SPEARHEAD_NUM_MAX_RX_INSTANCES];
	uint32_t num_pipes;
	uint32_t num_tx_instances;
	uint32_t num_rx_instances;
	uint32_t reserved;
};
#define IPA_LNX_EACH_INST_ALLOC_INFO_STRUCT_LEN_INT (24 + 12 + 12 + 16)

struct ipa_lnx_stats_alloc_info {
	uint32_t num_holb_drop_stats_clients;
	uint32_t num_holb_mon_stats_clients;
	uint32_t num_pm_clients;
	uint32_t num_wlan_instances;
	uint32_t num_eth_instances;
	uint32_t num_usb_instances;
	uint32_t num_mhip_instances;
	uint32_t num_page_rec_interval;
	struct ipa_lnx_each_inst_alloc_info wlan_inst_info[SPEARHEAD_NUM_MAX_INSTANCES];
	struct ipa_lnx_each_inst_alloc_info eth_inst_info[SPEARHEAD_NUM_MAX_INSTANCES];
	struct ipa_lnx_each_inst_alloc_info usb_inst_info[SPEARHEAD_NUM_MAX_INSTANCES];
	struct ipa_lnx_each_inst_alloc_info mhip_inst_info[SPEARHEAD_NUM_MAX_INSTANCES];
};
#define IPA_LNX_STATS_ALL_INFO_STRUCT_LEN_INT (32 + 128 + 128 + 128)

struct ipa_lnx_stats_spearhead_ctx {
	uint32_t usb_teth_prot[SPEARHEAD_NUM_MAX_INSTANCES];
	uint32_t log_type_mask;
	struct ipa_lnx_stats_alloc_info alloc_info;
};
#define IPA_LNX_STATS_SPEARHEAD_CTX_STRUCT_LEN_INT (8 + 4 + 416)

/* enum ipa_lnx_stats_ioc_cmd_type - IOCTL Command types for IPA lnx stats
 *
 */
enum ipa_lnx_stats_ioc_cmd_type {
	IPA_LNX_CMD_GET_ALLOC_INFO,
	IPA_LNX_CMD_GENERIC_STATS,
	IPA_LNX_CMD_CLOCK_STATS,
	IPA_LNX_CMD_WLAN_INST_STATS,
	IPA_LNX_CMD_ETH_INST_STATS,
	IPA_LNX_CMD_USB_INST_STATS,
	IPA_LNX_CMD_MHIP_INST_STATS,
	IPA_LNX_CMD_CONSOLIDATED_STATS,
	IPA_LNX_CMD_STATS_MAX,
};

int ipa_spearhead_stats_init(void);

/* Peripheral stats for Q6, should be in the same order, defined by Q6 */
struct ipa_peripheral_mdm_stats {
	uint32_t canary;

	uint16_t num_entries;
	uint16_t reserved;

	/* TLV for number of peripherals connected to APROC */
	/* value = IPA_PER_STATS_TYPE_NUM_PERS */
	uint16_t periph_id;
	uint16_t periph_len;
	uint32_t periph_val;

	/* TLV for number of periphers from/to traffic flowing from modem */
	/* value = IPA_PER_STATS_TYPE_NUM_PERS_WWAN */
	uint16_t periph_wwan_id;
	uint16_t periph_wwan_len;
	uint32_t periph_wwan_val;

	/* TLV for bitmask for active/connected peripherals */
	/* value = IPA_PER_STATS_TYPE_PER_TYPE */
	uint16_t periph_type_id;
	uint16_t periph_type_len;
	uint32_t periph_type_val;

	/* TLV for Current gen info if PCIe interconnect is valid */
	/* value = IPA_PER_STATS_TYPE_PCIE_GEN */
	uint16_t pcie_gen_type_id;
	uint16_t pcie_gen_type_len;
	uint32_t pcie_gen_type_val;

	/* TLV for Current width info if PCIe interconnect is valid */
	/* value = IPA_PER_STATS_TYPE_PCIE_WIDTH */
	uint16_t pcie_width_type_id;
	uint16_t pcie_width_type_len;
	uint32_t pcie_width_type_val;

	/* TLV for Max PCIe speed in current gen in Mbps */
	/* value = IPA_PER_STATS_TYPE_PCIE_MAX_SPEED */
	uint16_t pcie_max_speed_id;
	uint16_t pcie_max_speed_len;
	uint32_t pcie_max_speed_val;

	/* TLV for number PCIe LPM transitions */
	/* value = IPA_PER_STATS_TYPE_PCIE_NUM_LPM */
	uint16_t pcie_num_lpm_trans_id;
	uint16_t pcie_num_lpm_trans_len;
	uint16_t pcie_num_lpm_trans_d3;
	uint16_t pcie_num_lpm_trans_m1;
	uint16_t pcie_num_lpm_trans_m2;
	uint16_t pcie_num_lpm_trans_m0;

	/* TLV for USB enumeration type */
	/* value = IPA_PER_STATS_TYPE_USB_TYPE */
	uint16_t usb_enum_id;
	uint16_t usb_enum_len;
	uint32_t usb_enum_value;

	/* TLV for Current USB protocol enumeration if active */
	/* value = IPA_PER_STATS_TYPE_USB_PROT */
	uint16_t usb_prot_enum_id;
	uint16_t usb_prot_enum_len;
	uint32_t usb_prot_enum_value;

	/* TLV for Max USB speed in current gen in Mbps */
	/* value = IPA_PER_STATS_TYPE_USB_MAX_SPEED */
	uint16_t usb_max_speed_id;
	uint16_t usb_max_speed_len;
	uint32_t usb_max_speed_val;

	/* TLV for Total number of USB plug in/outs */
	/* value = IPA_PER_STATS_TYPE_USB_PIPO */
	uint16_t usb_pipo_id;
	uint16_t usb_pipo_len;
	uint32_t usb_pipo_val;

	/* TLV for Wifi enumeration type*/
	/* value = IPA_PER_STATS_TYPE_WIFI_ENUM_TYPE */
	uint16_t wifi_enum_type_id;
	uint16_t wifi_enum_type_len;
	uint32_t wifi_enum_type_val;

	/* TLV for Theoritical Max WLAN speed in current gen in Mbps (pipe for 5GHz in case of dual band) */
	/* value = IPA_PER_STATS_TYPE_WIFI_MAX_SPEED */
	uint16_t wifi_max_speed_id;
	uint16_t wifi_max_speed_len;
	uint32_t wifi_max_speed_val;

	/* TLV for Theoretical Max WLAN speed on the 2.4GHz pipe, value of 0 means disabled */
	/* value = IPA_PER_STATS_TYPE_WIFI_DUAL_BAND_EN */
	uint16_t wifi_dual_band_enabled_id;
	uint16_t wifi_dual_band_enabled_len;
	uint32_t wifi_dual_band_enabled_val;

	/* TLV for the type of ethernet client - Realtek/AQC */
	/* value = IPA_PER_STATS_TYPE_ETH_CLIENT */
	uint16_t eth_client_id;
	uint16_t eth_client_len;
	uint32_t eth_client_val;

	/* TLV for Max Eth link speed */
	/* value = IPA_PER_STATS_TYPE_ETH_MAX_SPEED */
	uint16_t eth_max_speed_id;
	uint16_t eth_max_speed_len;
	uint32_t eth_max_speed_val;

	/* TLV for Total number of bytes txferred through IPA DMA channels over PCIe */
	/* For cases where GSI used for QDSS direct DMA, need to extract bytes stats from GSI FW */
	/* value = IPA_PER_STATS_TYPE_IPA_DMA_BYTES */
	uint16_t ipa_dma_bytes_id;
	uint16_t ipa_dma_bytes_len;
	uint32_t ipa_dma_bytes_val;

	/* TLV for number of wifi peripherals connected to APROC */
	/* value = IPA_PER_STATS_TYPE_WIFI_HOLB_UC */
	uint16_t wifi_holb_uc_stats_id;
	uint16_t wifi_holb_uc_stats_len;
	uint16_t wifi_holb_uc_stats_num_periph_bad;
	uint16_t wifi_holb_uc_stats_num_periph_recovered;

	/* TLV for number of eth peripherals connected to APROC */
	/* value = IPA_PER_STATS_TYPE_ETH_HOLB_UC */
	uint16_t eth_holb_uc_stats_id;
	uint16_t eth_holb_uc_stats_len;
	uint16_t eth_holb_uc_stats_num_periph_bad;
	uint16_t eth_holb_uc_stats_num_periph_recovered;

	/* TLV for number of usb peripherals connected to APROC */
	/* value = IPA_PER_STATS_TYPE_USB_HOLB_UC */
	uint16_t usb_holb_uc_stats_id;
	uint16_t usb_holb_uc_stats_len;
	uint16_t usb_holb_uc_stats_num_periph_bad;
	uint16_t usb_holb_uc_stats_num_periph_recovered;
};

struct ipa_peripheral_msm_stats {
	uint32_t canary;

	uint16_t num_entries;
	uint16_t reserved;

	/* TLV for number of peripherals connected to APROC */
	/* value = IPA_PER_STATS_TYPE_NUM_PERS */
	uint16_t periph_id;
	uint16_t periph_len;
	uint32_t periph_val;

	/* TLV for number of periphers from/to traffic flowing from modem */
	/* value = IPA_PER_STATS_TYPE_NUM_PERS_WWAN */
	uint16_t periph_wwan_id;
	uint16_t periph_wwan_len;
	uint32_t periph_wwan_val;

	/* TLV for bitmask for active/connected peripherals */
	/* value = IPA_PER_STATS_TYPE_PER_TYPE */
	uint16_t periph_type_id;
	uint16_t periph_type_len;
	uint32_t periph_type_val;

	/* TLV for USB enumeration type */
	/* value = IPA_PER_STATS_TYPE_USB_TYPE */
	uint16_t usb_enum_id;
	uint16_t usb_enum_len;
	uint32_t usb_enum_value;

	/* TLV for Current USB protocol enumeration if active */
	/* value = IPA_PER_STATS_TYPE_USB_PROT */
	uint16_t usb_prot_enum_id;
	uint16_t usb_prot_enum_len;
	uint32_t usb_prot_enum_value;

	/* TLV for Max USB speed in current gen in Mbps */
	/* value = IPA_PER_STATS_TYPE_USB_MAX_SPEED */
	uint16_t usb_max_speed_id;
	uint16_t usb_max_speed_len;
	uint32_t usb_max_speed_val;

	/* TLV for Total number of USB plug in/outs */
	/* value = IPA_PER_STATS_TYPE_USB_PIPO */
	uint16_t usb_pipo_id;
	uint16_t usb_pipo_len;
	uint32_t usb_pipo_val;

	/* TLV for Wifi enumeration type*/
	/* value = IPA_PER_STATS_TYPE_WIFI_ENUM_TYPE */
	uint16_t wifi_enum_type_id;
	uint16_t wifi_enum_type_len;
	uint32_t wifi_enum_type_val;

	/* TLV for Theoritical Max WLAN speed in current gen in Mbps (pipe for 5GHz in case of dual band) */
	/* value = IPA_PER_STATS_TYPE_WIFI_MAX_SPEED */
	uint16_t wifi_max_speed_id;
	uint16_t wifi_max_speed_len;
	uint32_t wifi_max_speed_val;

	/* TLV for Theoretical Max WLAN speed on the 2.4GHz pipe, value of 0 means disabled */
	/* value = IPA_PER_STATS_TYPE_WIFI_DUAL_BAND_EN */
	uint16_t wifi_dual_band_enabled_id;
	uint16_t wifi_dual_band_enabled_len;
	uint32_t wifi_dual_band_enabled_val;

	/* TLV for number of wifi peripherals connected to APROC */
	/* value = IPA_PER_STATS_TYPE_WIFI_HOLB_UC */
	uint16_t wifi_holb_uc_stats_id;
	uint16_t wifi_holb_uc_stats_len;
	uint16_t wifi_holb_uc_stats_num_periph_bad;
	uint16_t wifi_holb_uc_stats_num_periph_recovered;

	/* TLV for number of usb peripherals connected to APROC */
	/* value = IPA_PER_STATS_TYPE_USB_HOLB_UC */
	uint16_t usb_holb_uc_stats_id;
	uint16_t usb_holb_uc_stats_len;
	uint16_t usb_holb_uc_stats_num_periph_bad;
	uint16_t usb_holb_uc_stats_num_periph_recovered;
};

union ipa_peripheral_stats {
	struct ipa_peripheral_mdm_stats mdm;
	struct ipa_peripheral_msm_stats msm;
};

int ipa3_peripheral_stats_init(union ipa_peripheral_stats *smem_addr);

#endif // _UAPI_IPA_LNX_STATS_H_
