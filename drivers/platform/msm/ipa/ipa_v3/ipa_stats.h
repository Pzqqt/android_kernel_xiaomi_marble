/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#define IPA_LNX_STATS_SUCCESS 0
#define IPA_LNX_STATS_FAILURE -1

#define SPEARHEAD_NUM_MAX_PIPES 6
#define SPEARHEAD_NUM_MAX_TX_INSTANCES 3
#define SPEARHEAD_NUM_MAX_RX_INSTANCES 3

#define SPEARHEAD_NUM_MAX_INSTANCES 2

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
	uint32_t reserved;
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
	/**
	 * IPA_LNX_CMD_GENERIC_STATS - Includes following fields (in bytes)
	 *							(min - 296 bytes, max - 300 bytes)
	 *
	 * tx_dma_pkts(4)		- Packets sent to IPA with IP_PACKET_INIT command
	 * tx_hw_pkts(4)		- Packets sent to IPA without PACKET_INIT.
	 *							These packets go through IPA HW processing
	 * tx_non_linear(4)		- Non linear TX packets
	 * tx_pkts_compl(4)		- No of TX packets processed by IPA
	 * stats_compl(4)		- No of TX commands and LAN packets processed by IPA
	 * active_eps(4)		- No of active end points
	 * wan_rx_empty(4) 		- No of times WAN_CONS/COAL pipes have buffers less than threshold of 32
	 * wan_repl_rx_empty(4)	- No of times there are no pages in temp cache for WAN pipe
	 * lan_rx_empty(4)		- No of times LAN_CONS pipe has buffers less than threshold of 32
	 * lan_repl_rx_empty(4)	- No of times LAN_CONS pipe has replinished buffers
	 * pg_recycle_stats(32)	- Page recycling stats
	 *		|______	coal_total_repl_buff(8)	- Total no of buffers replenished for coal pipe
	 *				coal_temp_repl_buff(8)	- Total no of buffers replenished from temp cache
	 *				def_total_repl_buff(8)	- Total no of buffers replenished for default pipe
	 *				def_temp_repl_buff(8)	- Total no of buffers replenished from temp cache
	 * exception_stats(40)	- Exception path stats
	 *		|______	excptn_type_none(4)	- No of packets with exception type as None
	 *				excptn_type_deaggr(4)- No of packets with exception type as deaggr
	 *				excptn_type_iptype(4)- No of packets with exception type as IP type
	 *				excptn_type_pkt_len(4)- No of packets with exception type as packet length
	 *				excptn_type_pkt_thrshld(4)- No of packets with exception type as packet threshold
	 *				excptn_type_frag_rule_miss(4)- No of packets with exception type as frag rule
	 *				excptn_type_sw_flt(4)- No of packets with exception type as sw filter
	 *				excptn_type_nat(4)- No of packets with exception type as NAT
	 *				excptn_type_ipv6_ct(4)- No of packets with exception type as IPv6 CT
	 *				excptn_type_csum(4)- No of packets with exception type as checksum
	 * odl_stats(16)		- ODL stats
	 *		|______	rx_pkt(4)		- Total no of packets received
	 *				processed_pkt(4)- Total no of processed packets
	 *				dropped_pkt(4)	- Total no of dropped packets
	 *				num_queue_pkt(4)- Total no of packets in queue
	 * holb_stats(168+)	- HOLB stats
	 *		|______	num_pipes(4)			- Total num of pipes for which HOLB is enabled(currently 5)
	 *		|______	num_holb_mon_clients(4)	- Total num of pipes for which HOLB is enabled(currently 5)
	 *		|______	holb_discard_stats(80)	- HOLB Discard Stats
	 *		|				|______ client_type(4)	- IPA Client type
	 *		|						num_drp_cnt(4)	- Total number of dropped pkts
	 *		|						num_drp_bytes(4)- Total number of dropped bytes
	 *		|						reserved(4)	- Reserved.
	 *		|______	holb_monitor_stats(80)	- No of clients for which HOLB monitrng is enabled(currently 5)
	 *						|______ client_type(4)	- IPA Client type
	 *								curr_index(4)	- Current HOLB monitoring index
	 *								num_en_cnt(4)	- Number of times peripheral went to bad state
	 *								num_dis_cnt(4)	- Number of times peripheral was recovered
	 */
	IPA_LNX_CMD_GENERIC_STATS,
	/**
	 * IPA_LNX_CMD_CLOCK_STATS - Includes following fields (in bytes)
	 *							(min - 888 bytes, max - 900 bytes)
	 *
	 * active_clients(4)	- No of active clock votes
	 * scale_thresh_svs(4)	- BW threshold value to be met for voting for SVS
	 * scale_thresh_nom(4)	- BW threshold value to be met for voting for nominal
	 * scale_thresh_tur(4)	- BW threshold value to be met for voting for turbo
	 * aggr_bw(4)			- Total BW required from the clients for caculating the vote.
	 * curr_clk_vote(4)		- Current active clock vote
	 * pm_client_stats(864+)	- Power Management stats (36 clients)
	 *		|______	pm_client_state(4)	- State of the PM client
	 *				pm_client_group(4)	- Group of the PM client
	 *				pm_client_bw(4)		- BW requested by PM client
	 *				pm_client_hdl(4)	- PM Client hdl
	 *				pm_client_type(4)	- Client type of the PM client
	 *				reserved(4)			- Reserved.
	 */
	IPA_LNX_CMD_CLOCK_STATS,
	/**
	 * IPA_LNX_CMD_WLAN_INST_STATS - Includes following fields (in bytes)
	 *							(min - 558 bytes, max - 600 bytes)
	 *
	 * num_wlan_instance(4)		- No of WLAN attaches
	 * reserved(4)				- Reserved.
	 * wlan_instance_info(550)	- Each WLAN Instance Info
	 *		|______	instance_id(4)	- Instance id of the WLAN
	 *		|		wdi_ver(4)		- WDI version in use
	 *		|		wlan_mode(4)	- Indicates the WLAN mode
	 *		|		wdi_over_gsi(4)	- Indicates whether communication is over GSI or uC
	 *		|		dbs_mode(4)		- Indicates whether DBS mode is enabled
	 *		|		pm_bandwidth(4)	- Bandwidth voted by the client
	 *		|		num_pipes(4)	- Number of pipes associated with WLAN
	 *		|		reserved(4)		- Reserved.
	 *		|______ pipe_info(360)	- Pipe Information (120 x 3 pipes)
	 *		|				|______ gsi_chan_ring_bp(8)	- Gsi channel ring base pointer address
	 *		|						gsi_chan_ring_rp(8)	- Transfer Ring Current read pointer address
	 *		|						gsi_chan_ring_wp(8)	- Transfer Ring Current write pointer address
	 *		|						gsi_evt_ring_bp(8)	- Event ring base pointer address
	 *		|						gsi_evt_ring_rp(8)	- Event Ring Current read pointer address
	 *		|						gsi_evt_ring_wp(8)	- Event Ring Current write pointer address
	 *		|						gsi_evt_ring_len(4)	- Transfer Ring length
	 *		|						gsi_chan_ring_len(4)- Transfer Ring length
	 *		|						buff_size(4)	- Size of buffer
	 *		|						num_free_buff(4)- Number of free credits with HW
	 *		|						gsi_ipa_if_tlv(4)	- Number of IPA_IF TLV
	 *		|						gsi_ipa_if_aos(4)	- Number of IPA_IF AOS
	 *		|						gsi_desc_size(4)	- Descriptor Size
	 *		|						pipe_num(4)	- Pipe number of the client
	 *		|						direction(4)	- Pipe direction(0 – IPA Consumer, 1 – IPA Producer)
	 *		|						client_type(4)	- Client type
	 *		|						gsi_chan_num(4)	- GSI channel number associated with Pipe
	 *		|						gsi_evt_num(4)	- GSI event number associated with Pipe
	 *		|						is_common_evt_ring(4)- Indicates whether common evt ring is used
	 *		|						gsi_prot_type(4)- GSI Protocol type
	 *		|						gsi_chan_state(4)-GSI Channel state
	 *		|						gsi_chan_stop_stm(4)- GSI channel stop state machine
	 *		|						gsi_poll_mode(4)- GSI Current Mode:- Polling/Interrupt
	 *		|						gsi_db_in_bytes(4)	- Indicates whether DB in bytes
	 *		|______ gsi_debug_stats(158)- GSI debug information
	 *						|______ num_tx_instances(4)	- Number of tx instances
	 *						|______ num_rx_instances(4)	- Number of rx instances
	 *						|______	gsi_tx_debug_stats(102)- GSI TX Debug Stats Info (2 X 56)
	 *						|			|______ tx_client(4) - TX client type
	 *						|					num_tx_ring_100_perc_with_cred(4) - Total number of times the ring is full of free credits
	 *						|					num_tx_ring_0_perc_with_cred(4) - Total number of times the ring has empty credits
	 *						|					num_tx_ring_above_75_perc_cred(4) - Total number of times ring has > 75% free credits
	 *						|					num_tx_ring_above_25_perc_cred(4) - Total number of times ring has < 25% of free credits
	 *						|					num_tx_ring_stats_polled(4) - Total number of times TX ring stats are counted
	 *						|					num_tx_oob(4) - Number of times GSI encountered OOB
	 *						|					num_tx_oob_time(4) - Total time GSI was in OOB state i.e no credits available
	 *						|					gsi_debug1(4) - Additional GSI Debug information
	 *						|					gsi_debug2(4) - Additional GSI Debug information
	 *						|					gsi_debug3(4) - Additional GSI Debug information
	 *						|					gsi_debug4(4) - Additional GSI Debug information
	 *						|					tx_summary(4) - 1 – Peripheral is bad in replenishing credits, 2 – IPA is not giving packets fast enough
	 *						|					reserved(4)	- Reserved.
	 *						|______	gsi_rx_debug_stats(48)- GSI RX Debug Stats Info (1 X 48)
	 *									|______ rx_client(4) - RX client type
	 *											num_rx_ring_100_perc_with_pack(4) - Total number of times the ring is full of packets
	 *											num_rx_ring_0_perc_with_pack(4) - Total number of times the ring has 0 packets
	 *											num_rx_ring_above_75_perc_pack(4) - Total number of times ring has > 75% packets
	 *											num_rx_ring_above_25_perc_pack(4) - Total number of times ring has < 25% packets
	 *											num_rx_ring_stats_polled(4) - Total number of times RX ring stats are counted
	 *											num_rx_drop_stats(4) - Total number of times GSI dropped packets
	 *											gsi_debug1(4) - Additional GSI Debug information
	 *											gsi_debug2(4) - Additional GSI Debug information
	 *											gsi_debug3(4) - Additional GSI Debug information
	 *											gsi_debug4(4) - Additional GSI Debug information
	 *											rx_summary(4) - 1 – Peripheral is bad in providing packets, 2 – IPA is not processing packets fast enough
	 */
	IPA_LNX_CMD_WLAN_INST_STATS,
	/**
	 * IPA_LNX_CMD_ETH_INST_STATS - Includes following fields (in bytes)
	 *							(min - 724 bytes, max - 800 bytes)
	 *
	 * num_eth_instance(4)		- No of ETH attaches
	 * reserved(4)				- Reserved.
	 * eth_instance_info(716)	- Each ETH Instance Info (358 x 2)
	 *		|______	instance_id(4)	- Instance id of the ETH
	 *		|		eth_mode(4)		- Ethernet mode
	 *		|		pm_bandwidth(4)	- Bandwidth voted by the client
	 *		|		num_pipes(4)	- Number of pipes associated with ETH
	 *		|______ pipe_info(240)	- Pipe Information (120 x 2 pipes)
	 *		|				|______ gsi_chan_ring_bp(8)	- Gsi channel ring base pointer address
	 *		|						gsi_chan_ring_rp(8)	- Transfer Ring Current read pointer address
	 *		|						gsi_chan_ring_wp(8)	- Transfer Ring Current write pointer address
	 *		|						gsi_evt_ring_bp(8)	- Event ring base pointer address
	 *		|						gsi_evt_ring_rp(8)	- Event Ring Current read pointer address
	 *		|						gsi_evt_ring_wp(8)	- Event Ring Current write pointer address
	 *		|						gsi_evt_ring_len(4)	- Transfer Ring length
	 *		|						gsi_chan_ring_len(4)- Transfer Ring length
	 *		|						buff_size(4)	- Size of buffer
	 *		|						num_free_buff(4)- Number of free credits with HW
	 *		|						gsi_ipa_if_tlv(4)	- Number of IPA_IF TLV
	 *		|						gsi_ipa_if_aos(4)	- Number of IPA_IF AOS
	 *		|						gsi_desc_size(4)	- Descriptor Size
	 *		|						pipe_num(4)	- Pipe number of the client
	 *		|						direction(4)	- Pipe direction(0 – IPA Consumer, 1 – IPA Producer)
	 *		|						client_type(4)	- Client type
	 *		|						gsi_chan_num(4)	- GSI channel number associated with Pipe
	 *		|						gsi_evt_num(4)	- GSI event number associated with Pipe
	 *		|						is_common_evt_ring(4)- Indicates whether common evt ring is used
	 *		|						gsi_prot_type(4)- GSI Protocol type
	 *		|						gsi_chan_state(4)-GSI Channel state
	 *		|						gsi_chan_stop_stm(4)- GSI channel stop state machine
	 *		|						gsi_poll_mode(4)- GSI Current Mode:- Polling/Interrupt
	 *		|						gsi_db_in_bytes(4)	- Indicates whether DB in bytes
	 *		|______ gsi_debug_stats(102)- GSI debug information
	 *						|______ num_tx_instances(4)	- Number of tx instances
	 *						|______ num_rx_instances(4)	- Number of rx instances
	 *						|______	gsi_tx_debug_stats(56)- GSI TX Debug Stats Info (1 X 56)
	 *						|			|______ tx_client(4) - TX client type
	 *						|					num_tx_ring_100_perc_with_cred(4) - Total number of times the ring is full of free credits
	 *						|					num_tx_ring_0_perc_with_cred(4) - Total number of times the ring has empty credits
	 *						|					num_tx_ring_above_75_perc_cred(4) - Total number of times ring has > 75% free credits
	 *						|					num_tx_ring_above_25_perc_cred(4) - Total number of times ring has < 25% of free credits
	 *						|					num_tx_ring_stats_polled(4) - Total number of times TX ring stats are counted
	 *						|					num_tx_oob(4) - Number of times GSI encountered OOB
	 *						|					num_tx_oob_time(4) - Total time GSI was in OOB state i.e no credits available
	 *						|					gsi_debug1(4) - Additional GSI Debug information
	 *						|					gsi_debug2(4) - Additional GSI Debug information
	 *						|					gsi_debug3(4) - Additional GSI Debug information
	 *						|					gsi_debug4(4) - Additional GSI Debug information
	 *						|					tx_summary(4) - 1 – Peripheral is bad in replenishing credits, 2 – IPA is not giving packets fast enough
	 *						|					reserved(4)	- Reserved.
	 *						|______	gsi_rx_debug_stats(48)- GSI RX Debug Stats Info (1 X 48)
	 *									|______ rx_client(4) - RX client type
	 *											num_rx_ring_100_perc_with_pack(4) - Total number of times the ring is full of packets
	 *											num_rx_ring_0_perc_with_pack(4) - Total number of times the ring has 0 packets
	 *											num_rx_ring_above_75_perc_pack(4) - Total number of times ring has > 75% packets
	 *											num_rx_ring_above_25_perc_pack(4) - Total number of times ring has < 25% packets
	 *											num_rx_ring_stats_polled(4) - Total number of times RX ring stats are counted
	 *											num_rx_drop_stats(4) - Total number of times GSI dropped packets
	 *											gsi_debug1(4) - Additional GSI Debug information
	 *											gsi_debug2(4) - Additional GSI Debug information
	 *											gsi_debug3(4) - Additional GSI Debug information
	 *											gsi_debug4(4) - Additional GSI Debug information
	 *											rx_summary(4) - 1 – Peripheral is bad in providing packets, 2 – IPA is not processing packets fast enough
	 */
	IPA_LNX_CMD_ETH_INST_STATS,
	/**
	 * IPA_LNX_CMD_USB_INST_STATS - Includes following fields (in bytes)
	 *							(min - 366 bytes, max - 400 bytes)
	 *
	 * num_usb_instance(4)	- No of USB attaches
	 * reserved(4)			- Reserved.
	 * usb_instance_info(358)	- Each USB Instance Info
	 *		|______	instance_id(4)	- Instance id of the USB
	 *		|		usb_mode(4)		- USB mode
	 *		|		pm_bandwidth(4)	- Bandwidth voted by the client
	 *		|		num_pipes(4)	- Number of pipes associated with USB
	 *		|______ pipe_info(240)	- Pipe Information (120 x 2 pipes)
	 *		|				|______ gsi_chan_ring_bp(8)	- Gsi channel ring base pointer address
	 *		|						gsi_chan_ring_rp(8)	- Transfer Ring Current read pointer address
	 *		|						gsi_chan_ring_wp(8)	- Transfer Ring Current write pointer address
	 *		|						gsi_evt_ring_bp(8)	- Event ring base pointer address
	 *		|						gsi_evt_ring_rp(8)	- Event Ring Current read pointer address
	 *		|						gsi_evt_ring_wp(8)	- Event Ring Current write pointer address
	 *		|						gsi_evt_ring_len(4)	- Transfer Ring length
	 *		|						gsi_chan_ring_len(4)- Transfer Ring length
	 *		|						buff_size(4)	- Size of buffer
	 *		|						num_free_buff(4)- Number of free credits with HW
	 *		|						gsi_ipa_if_tlv(4)	- Number of IPA_IF TLV
	 *		|						gsi_ipa_if_aos(4)	- Number of IPA_IF AOS
	 *		|						gsi_desc_size(4)	- Descriptor Size
	 *		|						pipe_num(4)	- Pipe number of the client
	 *		|						direction(4)	- Pipe direction(0 – IPA Consumer, 1 – IPA Producer)
	 *		|						client_type(4)	- Client type
	 *		|						gsi_chan_num(4)	- GSI channel number associated with Pipe
	 *		|						gsi_evt_num(4)	- GSI event number associated with Pipe
	 *		|						is_common_evt_ring(4)- Indicates whether common evt ring is used
	 *		|						gsi_prot_type(4)- GSI Protocol type
	 *		|						gsi_chan_state(4)-GSI Channel state
	 *		|						gsi_chan_stop_stm(4)- GSI channel stop state machine
	 *		|						gsi_poll_mode(4)- GSI Current Mode:- Polling/Interrupt
	 *		|						gsi_db_in_bytes(4)	- Indicates whether DB in bytes
	 *		|______ gsi_debug_stats(102)- GSI debug information
	 *						|______ num_tx_instances(4)	- Number of tx instances
	 *						|______ num_rx_instances(4)	- Number of rx instances
	 *						|______	gsi_tx_debug_stats(56)- GSI TX Debug Stats Info (1 X 56)
	 *						|			|______ tx_client(4) - TX client type
	 *						|					num_tx_ring_100_perc_with_cred(4) - Total number of times the ring is full of free credits
	 *						|					num_tx_ring_0_perc_with_cred(4) - Total number of times the ring has empty credits
	 *						|					num_tx_ring_above_75_perc_cred(4) - Total number of times ring has > 75% free credits
	 *						|					num_tx_ring_above_25_perc_cred(4) - Total number of times ring has < 25% of free credits
	 *						|					num_tx_ring_stats_polled(4) - Total number of times TX ring stats are counted
	 *						|					num_tx_oob(4) - Number of times GSI encountered OOB
	 *						|					num_tx_oob_time(4) - Total time GSI was in OOB state i.e no credits available
	 *						|					gsi_debug1(4) - Additional GSI Debug information
	 *						|					gsi_debug2(4) - Additional GSI Debug information
	 *						|					gsi_debug3(4) - Additional GSI Debug information
	 *						|					gsi_debug4(4) - Additional GSI Debug information
	 *						|					tx_summary(4) - 1 – Peripheral is bad in replenishing credits, 2 – IPA is not giving packets fast enough
	 *						|					reserved(4)	- Reserved.
	 *						|______	gsi_rx_debug_stats(48)- GSI RX Debug Stats Info (1 X 48)
	 *									|______ rx_client(4) - RX client type
	 *											num_rx_ring_100_perc_with_pack(4) - Total number of times the ring is full of packets
	 *											num_rx_ring_0_perc_with_pack(4) - Total number of times the ring has 0 packets
	 *											num_rx_ring_above_75_perc_pack(4) - Total number of times ring has > 75% packets
	 *											num_rx_ring_above_25_perc_pack(4) - Total number of times ring has < 25% packets
	 *											num_rx_ring_stats_polled(4) - Total number of times RX ring stats are counted
	 *											num_rx_drop_stats(4) - Total number of times GSI dropped packets
	 *											gsi_debug1(4) - Additional GSI Debug information
	 *											gsi_debug2(4) - Additional GSI Debug information
	 *											gsi_debug3(4) - Additional GSI Debug information
	 *											gsi_debug4(4) - Additional GSI Debug information
	 *											rx_summary(4) - 1 – Peripheral is bad in providing packets, 2 – IPA is not processing packets fast enough
	 */
	IPA_LNX_CMD_USB_INST_STATS,
	/**
	 * IPA_LNX_CMD_MHIP_INST_STATS - Includes following fields (in bytes)
	 *							(min - 710 bytes, max - 800 bytes)
	 *
	 * num_mhip_instance(4)	- No of MHIP attaches
	 * reserved(4)			- Reserved.
	 * mhip_instance_info(702)	- Each MHIP Instance Info
	 *		|______	instance_id(4)	- Instance id of the MHIP
	 *		|		mhip_mode(4)	- MHIP mode
	 *		|		pm_bandwidth(4)	- Bandwidth voted by the client
	 *		|		num_pipes(4)	- Number of pipes associated with USB
	 *		|______ pipe_info(480)	- Pipe Information (120 x 4 pipes)
	 *		|				|______ gsi_chan_ring_bp(8)	- Gsi channel ring base pointer address
	 *		|						gsi_chan_ring_rp(8)	- Transfer Ring Current read pointer address
	 *		|						gsi_chan_ring_wp(8)	- Transfer Ring Current write pointer address
	 *		|						gsi_evt_ring_bp(8)	- Event ring base pointer address
	 *		|						gsi_evt_ring_rp(8)	- Event Ring Current read pointer address
	 *		|						gsi_evt_ring_wp(8)	- Event Ring Current write pointer address
	 *		|						gsi_evt_ring_len(4)	- Transfer Ring length
	 *		|						gsi_chan_ring_len(4)- Transfer Ring length
	 *		|						buff_size(4)	- Size of buffer
	 *		|						num_free_buff(4)- Number of free credits with HW
	 *		|						gsi_ipa_if_tlv(4)	- Number of IPA_IF TLV
	 *		|						gsi_ipa_if_aos(4)	- Number of IPA_IF AOS
	 *		|						gsi_desc_size(4)	- Descriptor Size
	 *		|						pipe_num(4)	- Pipe number of the client
	 *		|						direction(4)	- Pipe direction(0 – IPA Consumer, 1 – IPA Producer)
	 *		|						client_type(4)	- Client type
	 *		|						gsi_chan_num(4)	- GSI channel number associated with Pipe
	 *		|						gsi_evt_num(4)	- GSI event number associated with Pipe
	 *		|						is_common_evt_ring(4)- Indicates whether common evt ring is used
	 *		|						gsi_prot_type(4)- GSI Protocol type
	 *		|						gsi_chan_state(4)-GSI Channel state
	 *		|						gsi_chan_stop_stm(4)- GSI channel stop state machine
	 *		|						gsi_poll_mode(4)- GSI Current Mode:- Polling/Interrupt
	 *		|						gsi_db_in_bytes(4)	- Indicates whether DB in bytes
	 *		|______ gsi_debug_stats(206)- GSI debug information
	 *						|______ num_tx_instances(4)	- Number of tx instances
	 *						|______ num_rx_instances(4)	- Number of rx instances
	 *						|______	gsi_tx_debug_stats(102)- GSI TX Debug Stats Info (2 X 56)
	 *						|			|______ tx_client(4) - TX client type
	 *						|					num_tx_ring_100_perc_with_cred(4) - Total number of times the ring is full of free credits
	 *						|					num_tx_ring_0_perc_with_cred(4) - Total number of times the ring has empty credits
	 *						|					num_tx_ring_above_75_perc_cred(4) - Total number of times ring has > 75% free credits
	 *						|					num_tx_ring_above_25_perc_cred(4) - Total number of times ring has < 25% of free credits
	 *						|					num_tx_ring_stats_polled(4) - Total number of times TX ring stats are counted
	 *						|					num_tx_oob(4) - Number of times GSI encountered OOB
	 *						|					num_tx_oob_time(4) - Total time GSI was in OOB state i.e no credits available
	 *						|					gsi_debug1(4) - Additional GSI Debug information
	 *						|					gsi_debug2(4) - Additional GSI Debug information
	 *						|					gsi_debug3(4) - Additional GSI Debug information
	 *						|					gsi_debug4(4) - Additional GSI Debug information
	 *						|					tx_summary(4) - 1 – Peripheral is bad in replenishing credits, 2 – IPA is not giving packets fast enough
	 *						|					reserved(4)	- Reserved.
	 *						|______	gsi_rx_debug_stats(96)- GSI RX Debug Stats Info (2 X 48)
	 *									|______ rx_client(4) - RX client type
	 *											num_rx_ring_100_perc_with_pack(4) - Total number of times the ring is full of packets
	 *											num_rx_ring_0_perc_with_pack(4) - Total number of times the ring has 0 packets
	 *											num_rx_ring_above_75_perc_pack(4) - Total number of times ring has > 75% packets
	 *											num_rx_ring_above_25_perc_pack(4) - Total number of times ring has < 25% packets
	 *											num_rx_ring_stats_polled(4) - Total number of times RX ring stats are counted
	 *											num_rx_drop_stats(4) - Total number of times GSI dropped packets
	 *											gsi_debug1(4) - Additional GSI Debug information
	 *											gsi_debug2(4) - Additional GSI Debug information
	 *											gsi_debug3(4) - Additional GSI Debug information
	 *											gsi_debug4(4) - Additional GSI Debug information
	 *											rx_summary(4) - 1 – Peripheral is bad in providing packets, 2 – IPA is not processing packets fast enough
	 */
	IPA_LNX_CMD_MHIP_INST_STATS,
	IPA_LNX_CMD_STATS_MAX,
};

int ipa_spearhead_stats_init(void);

#endif // _UAPI_IPA_LNX_STATS_H_
