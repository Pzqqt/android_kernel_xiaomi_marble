/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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

#if !defined(__SMEINTERNAL_H)
#define __SMEINTERNAL_H

/**
 * \file  sme_internal.h
 *
 * \brief prototype for SME internal structures and APIs used for SME and MAC
 */

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
#include "qdf_status.h"
#include "qdf_lock.h"
#include "qdf_trace.h"
#include "qdf_mem.h"
#include "qdf_types.h"
#include "host_diag_core_event.h"
#include "csr_link_list.h"
#include "sme_power_save.h"

struct wmi_twt_enable_complete_event_param;
/*--------------------------------------------------------------------------
  Type declarations
  ------------------------------------------------------------------------*/

/* Mask can be only have one bit set */
typedef enum eSmeCommandType {
	eSmeNoCommand = 0,
	/* this is not a command, it is to identify this is a CSR command */
	eSmeCsrCommandMask = 0x10000,
	eSmeCommandRoam,
	eSmeCommandWmStatusChange,
	e_sme_command_del_sta_session,
	/* QOS */
	eSmeQosCommandMask = 0x40000,   /* To identify Qos commands */
	eSmeCommandAddTs,
	eSmeCommandDelTs,
	e_sme_command_set_hw_mode,
	e_sme_command_nss_update,
	e_sme_command_set_dual_mac_config,
	e_sme_command_set_antenna_mode,
} eSmeCommandType;

typedef enum eSmeState {
	SME_STATE_STOP,
	SME_STATE_START,
	SME_STATE_READY,
} eSmeState;

#define SME_IS_START(mac)  (SME_STATE_STOP != (mac)->sme.state)
#define SME_IS_READY(mac)  (SME_STATE_READY == (mac)->sme.state)

/* HDD Callback function */
typedef void (*pIbssPeerInfoCb)(void *pUserData,
					tSirPeerInfoRspParams *infoParam);

/* Peer info */
typedef struct tagSmePeerInfoHddCbkInfo {
	void *pUserData;
	pIbssPeerInfoCb peerInfoCbk;
} tSmePeerInfoHddCbkInfo;

/**
 * struct stats_ext_event - stats_ext_event payload
 * @vdev_id: ID of the vdev for the stats
 * @event_data_len: length of the @event_data
 * @event_data: actual ext stats
 */
struct stats_ext_event {
	uint32_t vdev_id;
	uint32_t event_data_len;
	uint8_t event_data[];
};

/**
 * typedef stats_ext_cb - stats ext callback
 * @hdd_handle: Opaque handle to the HDD context
 * @data: stats ext payload from firmware
 */
typedef void (*stats_ext_cb)(hdd_handle_t hdd_handle,
			     struct stats_ext_event *data);

/**
 * typedef stats_ext2_cb - stats ext2 callback
 * @hdd_handle: Opaque handle to the HDD context
 * @data: stats ext2 payload from firmware
 */
typedef void (*stats_ext2_cb)(hdd_handle_t hdd_handle,
			      struct sir_sme_rx_aggr_hole_ind *data);

#define MAX_ACTIVE_CMD_STATS    16

typedef struct sActiveCmdStats {
	eSmeCommandType command;
	uint32_t reason;
	uint32_t sessionId;
	uint64_t timestamp;
} tActiveCmdStats;

typedef struct sSelfRecoveryStats {
	tActiveCmdStats activeCmdStats[MAX_ACTIVE_CMD_STATS];
	uint8_t cmdStatsIndx;
} tSelfRecoveryStats;

typedef void (*link_layer_stats_cb)(hdd_handle_t hdd_handle,
				    int indication_type,
				    tSirLLStatsResults *results,
				    void *cookie);

typedef void (*ext_scan_ind_cb)(hdd_handle_t hdd_handle,
				const uint16_t, void *);

typedef void (*ocb_callback)(void *context, void *response);
typedef void (*sme_set_thermal_level_callback)(hdd_handle_t hdd_handle,
					       u_int8_t level);
typedef void (*p2p_lo_callback)(void *context,
				struct sir_p2p_lo_event *event);
#ifdef FEATURE_OEM_DATA_SUPPORT
typedef void (*sme_send_oem_data_rsp_msg)(struct oem_data_rsp *);
#endif

typedef void (*twt_enable_cb)(hdd_handle_t hdd_handle,
			      struct wmi_twt_enable_complete_event_param *params);
typedef void (*twt_disable_cb)(hdd_handle_t hdd_handle);

#ifdef FEATURE_WLAN_APF
/**
 * typedef apf_get_offload_cb - APF offload callback signature
 * @context: Opaque context that the client can use to associate the
 *    callback with the request
 * @caps: APF offload capabilities as reported by firmware
 */
struct sir_apf_get_offload;
typedef void (*apf_get_offload_cb)(void *context,
				   struct sir_apf_get_offload *caps);

/**
 * typedef apf_read_mem_cb - APF read memory response callback
 * @context: Opaque context that the client can use to associate the
 *    callback with the request
 * @evt: APF read memory response event parameters
 */
typedef void (*apf_read_mem_cb)(void *context,
				struct wmi_apf_read_memory_resp_event_params
									  *evt);
#endif /* FEATURE_WLAN_APF */

/**
 * typedef rssi_threshold_breached_cb - RSSI threshold breach callback
 * @hdd_handle: Opaque handle to the HDD context
 * @event: The RSSI breach event
 */
typedef void (*rssi_threshold_breached_cb)(hdd_handle_t hdd_handle,
					   struct rssi_breach_event *event);

/**
 * typedef sme_encrypt_decrypt_callback - encrypt/decrypt callback
 *    signature
 * @context: Opaque context that the client can use to associate the
 *    callback with the request
 * @response: Encrypt/Decrypt response from firmware
 */
struct sir_encrypt_decrypt_rsp_params;
typedef void (*sme_encrypt_decrypt_callback)(
			void *context,
			struct sir_encrypt_decrypt_rsp_params *response);

/**
 * typedef get_chain_rssi_callback - get chain rssi callback
 * @context: Opaque context that the client can use to associate the
 *    callback with the request
 * @data: chain rssi result reported by firmware
 */
struct chain_rssi_result;
typedef void (*get_chain_rssi_callback)(void *context,
					struct chain_rssi_result *data);

typedef void (*tx_queue_cb)(hdd_handle_t hdd_handle, uint32_t vdev_id,
			    enum netif_action_type action,
			    enum netif_reason_type reason);

/**
 * typedef pwr_save_fail_cb - power save fail callback function
 * @hdd_handle: HDD handle registered with SME
 * @params: failure parameters
 */
struct chip_pwr_save_fail_detected_params;
typedef void (*pwr_save_fail_cb)(hdd_handle_t hdd_handle,
			struct chip_pwr_save_fail_detected_params *params);

/**
 * typedef bt_activity_info_cb - bluetooth activity callback function
 * @hdd_handle: HDD handle registered with SME
 * @bt_activity: bluetooth activity information
 */
typedef void (*bt_activity_info_cb)(hdd_handle_t hdd_handle,
				    uint32_t bt_activity);

/**
 * typedef congestion_cb - congestion callback function
 * @hdd_handle: HDD handle registered with SME
 * @congestion: Current congestion value
 * @vdev_id: ID of the vdev for which congestion is being reported
 */
typedef void (*congestion_cb)(hdd_handle_t hdd_handle, uint32_t congestion,
			      uint32_t vdev_id);

/**
 * typedef rso_cmd_status_cb - RSO command status  callback function
 * @hdd_handle: HDD handle registered with SME
 * @rso_status: Status of the operation
 */
typedef void (*rso_cmd_status_cb)(hdd_handle_t hdd_handle,
				  struct rso_cmd_status *rso_status);

/**
 * typedef lost_link_info_cb - lost link indication callback function
 * @hdd_handle: HDD handle registered with SME
 * @lost_link_info: Information about the lost link
 */
typedef void (*lost_link_info_cb)(hdd_handle_t hdd_handle,
				  struct sir_lost_link_info *lost_link_info);

typedef struct tagSmeStruct {
	eSmeState state;
	qdf_mutex_t lkSmeGlobalLock;
	uint32_t totalSmeCmd;
	/* following pointer contains array of pointers for tSmeCmd* */
	void **pSmeCmdBufAddr;
	tDblLinkList smeCmdFreeList;    /* preallocated roam cmd list */
	enum QDF_OPMODE currDeviceMode;
	tSmePeerInfoHddCbkInfo peerInfoParams;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	host_event_wlan_status_payload_type eventPayload;
#endif
	void *ll_stats_context;
	link_layer_stats_cb link_layer_stats_cb;
	void (*link_layer_stats_ext_cb)(hdd_handle_t callback_ctx,
					tSirLLStatsResults *rsp);
#ifdef WLAN_POWER_DEBUGFS
	void *power_debug_stats_context;
	void (*power_stats_resp_callback)(struct power_stats_response *rsp,
						void *callback_context);
#endif
#ifdef WLAN_FEATURE_BEACON_RECEPTION_STATS
	void *beacon_stats_context;
	void (*beacon_stats_resp_callback)(struct bcn_reception_stats_rsp *rsp,
					   void *callback_context);
#endif
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	void (*pAutoShutdownNotificationCb)(void);
#endif
	/* Maximum interfaces allowed by the host */
	uint8_t max_intf_count;
	stats_ext_cb stats_ext_cb;
	stats_ext2_cb stats_ext2_cb;
	/* linkspeed callback */
	void (*pLinkSpeedIndCb)(tSirLinkSpeedInfo *indParam,
			void *pDevContext);
	void *pLinkSpeedCbContext;
	/* get peer info callback */
	void (*pget_peer_info_ind_cb)(struct sir_peer_info_resp *param,
		void *pcontext);
	void *pget_peer_info_cb_context;
	/* get extended peer info callback */
	void (*pget_peer_info_ext_ind_cb)(struct sir_peer_info_ext_resp *param,
		void *pcontext);
	void *pget_peer_info_ext_cb_context;
#ifdef FEATURE_WLAN_EXTSCAN
	ext_scan_ind_cb ext_scan_ind_cb;
#endif /* FEATURE_WLAN_EXTSCAN */
	csr_link_status_callback link_status_callback;
	void *link_status_context;
	int (*get_tsf_cb)(void *pcb_cxt, struct stsf *ptsf);
	void *get_tsf_cxt;
	/* get temperature event context and callback */
	void *pTemperatureCbContext;
	void (*pGetTemperatureCb)(int temperature, void *context);
	uint8_t miracast_value;
	struct ps_global_info  ps_global_info;
	rssi_threshold_breached_cb rssi_threshold_breached_cb;
	hw_mode_transition_cb sme_hw_mode_trans_cb;
	/* OCB callbacks */
	void *ocb_set_config_context;
	ocb_callback ocb_set_config_callback;
	void *ocb_get_tsf_timer_context;
	ocb_callback ocb_get_tsf_timer_callback;
	void *dcc_get_stats_context;
	ocb_callback dcc_get_stats_callback;
	void *dcc_update_ndl_context;
	ocb_callback dcc_update_ndl_callback;
	void *dcc_stats_event_context;
	ocb_callback dcc_stats_event_callback;
	sme_set_thermal_level_callback set_thermal_level_cb;
	void *apf_get_offload_context;
#ifdef FEATURE_P2P_LISTEN_OFFLOAD
	p2p_lo_callback p2p_lo_event_callback;
	void *p2p_lo_event_context;
#endif
#ifdef FEATURE_OEM_DATA_SUPPORT
	sme_send_oem_data_rsp_msg oem_data_rsp_callback;
#endif
	sme_encrypt_decrypt_callback encrypt_decrypt_cb;
	void *encrypt_decrypt_context;
	lost_link_info_cb lost_link_info_cb;

	bool (*set_connection_info_cb)(bool);
	bool (*get_connection_info_cb)(uint8_t *session_id,
			enum scan_reject_states *reason);
	rso_cmd_status_cb rso_cmd_status_cb;
	congestion_cb congestion_cb;
	pwr_save_fail_cb chip_power_save_fail_cb;
	bt_activity_info_cb bt_activity_info_cb;
	void *get_arp_stats_context;
	void (*get_arp_stats_cb)(void *, struct rsp_stats *, void *);
	get_chain_rssi_callback get_chain_rssi_cb;
	void *get_chain_rssi_context;
	tx_queue_cb tx_queue_cb;
	twt_enable_cb twt_enable_cb;
	twt_disable_cb twt_disable_cb;
#ifdef FEATURE_WLAN_APF
	apf_get_offload_cb apf_get_offload_cb;
	apf_read_mem_cb apf_read_mem_cb;
#endif
} tSmeStruct, *tpSmeStruct;

#endif /* #if !defined( __SMEINTERNAL_H ) */
