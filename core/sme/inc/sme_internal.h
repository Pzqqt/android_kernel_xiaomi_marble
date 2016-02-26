/*
 * Copyright (c) 2011-2015 The Linux Foundation. All rights reserved.
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
#include "cdf_status.h"
#include "cdf_lock.h"
#include "cdf_trace.h"
#include "cdf_memory.h"
#include "cdf_types.h"
#include "host_diag_core_event.h"
#include "csr_link_list.h"
#include "sme_power_save.h"

/*--------------------------------------------------------------------------
  Type declarations
  ------------------------------------------------------------------------*/

/* Mask can be only have one bit set */
typedef enum eSmeCommandType {
	eSmeNoCommand = 0,
	eSmeDropCommand,
	/* this is not a command, it is to identify this is a CSR command */
	eSmeCsrCommandMask = 0x10000,
	eSmeCommandScan,
	eSmeCommandRoam,
	eSmeCommandWmStatusChange,
	eSmeCommandSetKey,
	eSmeCommandAddStaSession,
	eSmeCommandDelStaSession,
#ifdef FEATURE_WLAN_TDLS
	/*
	 * eSmeTdlsCommandMask = 0x80000,
	 * To identify TDLS commands <TODO>
	 * These can be considered as csr commands.
	 */
	eSmeCommandTdlsSendMgmt,
	eSmeCommandTdlsAddPeer,
	eSmeCommandTdlsDelPeer,
	eSmeCommandTdlsLinkEstablish,
#endif
	/* PMC */
	eSmePmcCommandMask = 0x20000,   /* To identify PMC commands */
	eSmeCommandEnterBmps,
	eSmeCommandExitBmps,
	eSmeCommandEnterUapsd,
	eSmeCommandExitUapsd,
	eSmeCommandExitWowl,
	eSmeCommandEnterStandby,
	/* QOS */
	eSmeQosCommandMask = 0x40000,   /* To identify Qos commands */
	eSmeCommandAddTs,
	eSmeCommandDelTs,
#ifdef FEATURE_OEM_DATA_SUPPORT
	eSmeCommandOemDataReq = 0x80000, /* To identify the oem data commands */
#endif
	eSmeCommandRemainOnChannel,
	e_sme_command_set_hw_mode,
	e_sme_command_nss_update,
	e_sme_command_set_dual_mac_config,
} eSmeCommandType;

typedef enum eSmeState {
	SME_STATE_STOP,
	SME_STATE_START,
	SME_STATE_READY,
} eSmeState;

#define SME_IS_START(pMac)  (SME_STATE_STOP != (pMac)->sme.state)
#define SME_IS_READY(pMac)  (SME_STATE_READY == (pMac)->sme.state)

typedef struct sStatsExtEvent {
	uint32_t vdev_id;
	uint32_t event_data_len;
	uint8_t event_data[];
} tStatsExtEvent, *tpStatsExtEvent;

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

#ifdef WLAN_FEATURE_GTK_OFFLOAD
/* GTK Offload Information Callback declaration */
typedef void (*gtk_offload_get_info_callback)(void *callback_context,
		tpSirGtkOffloadGetInfoRspParams
		pGtkOffloadGetInfoRsp);
#endif
#ifdef FEATURE_WLAN_SCAN_PNO
/*Pref netw found Cb declaration*/
typedef void (*preferred_network_found_ind_cb)(void *callback_context,
		tpSirPrefNetworkFoundInd
		pPrefNetworkFoundInd);
#endif

typedef void (*ocb_callback)(void *context, void *response);
typedef void (*sme_set_thermal_level_callback)(void *context, u_int8_t level);

typedef struct tagSmeStruct {
	eSmeState state;
	cdf_mutex_t lkSmeGlobalLock;
	uint32_t totalSmeCmd;
	/* following pointer contains array of pointers for tSmeCmd* */
	void **pSmeCmdBufAddr;
	tDblLinkList smeCmdActiveList;
	tDblLinkList smeCmdPendingList;
	tDblLinkList smeCmdFreeList;    /* preallocated roam cmd list */
	tCDF_CON_MODE currDeviceMode;
#ifdef FEATURE_WLAN_LPHB
	void (*pLphbIndCb)(void *pHddCtx, tSirLPHBInd *indParam);
#endif /* FEATURE_WLAN_LPHB */
	/* pending scan command list */
	tDblLinkList smeScanCmdPendingList;
	/* active scan command list */
	tDblLinkList smeScanCmdActiveList;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	host_event_wlan_status_payload_type eventPayload;
#endif
#ifdef FEATURE_WLAN_CH_AVOID
	void (*pChAvoidNotificationCb)(void *hdd_context, void *indi_param);
#endif /* FEATURE_WLAN_CH_AVOID */
#ifdef WLAN_FEATURE_LINK_LAYER_STATS
	void (*pLinkLayerStatsIndCallback)(void *callbackContext,
			int indType, void *pRsp);
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
	void (*pAutoShutdownNotificationCb)(void);
#endif
	/* Maximum interfaces allowed by the host */
	uint8_t max_intf_count;
	void (*StatsExtCallback)(void *, tStatsExtEvent *);
	/* linkspeed callback */
	void (*pLinkSpeedIndCb)(tSirLinkSpeedInfo *indParam,
			void *pDevContext);
	void *pLinkSpeedCbContext;
#ifdef FEATURE_WLAN_EXTSCAN
	void (*pExtScanIndCb)(void *, const uint16_t, void *);
#endif /* FEATURE_WLAN_EXTSCAN */
#ifdef WLAN_FEATURE_NAN
	void (*nanCallback)(void *, tSirNanEvent *);
#endif
	bool enableSelfRecovery;
	tCsrLinkStatusCallback linkStatusCallback;
	void *linkStatusContext;
	/* get temperature event context and callback */
	void *pTemperatureCbContext;
	void (*pGetTemperatureCb)(int temperature, void *context);
	uint8_t miracast_value;
	struct ps_global_info  ps_global_info;
#ifdef WLAN_FEATURE_GTK_OFFLOAD
	/* routine to call for GTK Offload Information */
	gtk_offload_get_info_callback gtk_offload_get_info_cb;
	/* value to be passed as parameter to routine specified above */
	void *gtk_offload_get_info_cb_context;
#endif /* WLAN_FEATURE_GTK_OFFLOAD */
#ifdef FEATURE_WLAN_SCAN_PNO
	/* routine to call for Preferred Network Found Indication */
	preferred_network_found_ind_cb pref_netw_found_cb;
	/* value to be passed as parameter to routine specified above */
	void *preferred_network_found_ind_cb_ctx;
#endif /* FEATURE_WLAN_SCAN_PNO */
	void (*rssi_threshold_breached_cb)(void *, struct rssi_breach_event *);
#ifdef WLAN_FEATURE_MEMDUMP
	void (*fw_dump_callback)(void *context, struct fw_dump_rsp *rsp);
#endif
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
	void *saved_scan_cmd;
} tSmeStruct, *tpSmeStruct;

#endif /* #if !defined( __SMEINTERNAL_H ) */
