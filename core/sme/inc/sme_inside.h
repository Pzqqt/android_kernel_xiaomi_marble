/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
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

#if !defined(__SMEINSIDE_H)
#define __SMEINSIDE_H

/**
 * \file  sme_inside.h
 *
 * \brief prototype for SME structures and APIs used insside SME
 */

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
#include "cdf_status.h"
#include "cdf_lock.h"
#include "cdf_trace.h"
#include "cdf_memory.h"
#include "cdf_types.h"
#include "sir_api.h"
#include "csr_internal.h"
#include "sme_qos_api.h"
#include "sme_qos_internal.h"

#ifdef FEATURE_OEM_DATA_SUPPORT
#include "oem_data_internal.h"
#endif

#if defined WLAN_FEATURE_VOWIFI
#include "sme_rrm_api.h"
#endif
ePhyChanBondState csr_convert_cb_ini_value_to_phy_cb_state(uint32_t cbIniValue);

/*--------------------------------------------------------------------------
  Type declarations
  ------------------------------------------------------------------------*/
/*
 * In case MAX num of STA are connected to SAP, switching off SAP causes
 * two SME cmd to be enqueued for each STA. Keeping SME total cmds as following
 * to make sure we have space for these cmds + some additional cmds.
 */
#define SME_TOTAL_COMMAND                (HAL_NUM_STA * 3)

typedef struct sGenericPmcCmd {
	uint32_t size;          /* sizeof the data in the union, if any */
	uint32_t sessionId;
	/* if true, the cmd shalln't put back to the queue, free mem instead. */
	bool fReleaseWhenDone;
	union {
		tSirSmeWowlEnterParams enterWowlInfo;
		tSirSmeWowlExitParams exitWowlInfo;
	} u;
} tGenericPmcCmd;

typedef struct sGenericQosCmd {
	sme_QosWmmTspecInfo tspecInfo;
	sme_QosEdcaAcType ac;
	uint8_t tspec_mask;
} tGenericQosCmd;

typedef struct sRemainChlCmd {
	uint8_t chn;
	uint8_t phyMode;
	uint32_t duration;
	uint8_t isP2PProbeReqAllowed;
	uint32_t scan_id;
	void *callback;
	void *callbackCtx;
} tRemainChlCmd;

typedef struct sNoACmd {
	tP2pPsConfig NoA;
} tNoACmd;
#ifdef FEATURE_WLAN_TDLS
typedef struct TdlsSendMgmtInfo {
	tSirMacAddr peerMac;
	uint8_t frameType;
	uint8_t dialog;
	uint16_t statusCode;
	uint8_t responder;
	uint32_t peerCapability;
	uint8_t *buf;
	uint8_t len;
} tTdlsSendMgmtCmdInfo;

typedef struct TdlsLinkEstablishInfo {
	struct cdf_mac_addr peermac;
	uint8_t uapsdQueues;
	uint8_t maxSp;
	uint8_t isBufSta;
	uint8_t isOffChannelSupported;
	uint8_t isResponder;
	uint8_t supportedChannelsLen;
	uint8_t supportedChannels[SIR_MAC_MAX_SUPP_CHANNELS];
	uint8_t supportedOperClassesLen;
	uint8_t supportedOperClasses[SIR_MAC_MAX_SUPP_OPER_CLASSES];
} tTdlsLinkEstablishCmdInfo;

typedef struct TdlsAddStaInfo {
	eTdlsAddOper tdlsAddOper;
	tSirMacAddr peerMac;
	uint16_t capability;
	uint8_t extnCapability[SIR_MAC_MAX_EXTN_CAP];
	uint8_t supportedRatesLen;
	uint8_t supportedRates[SIR_MAC_MAX_SUPP_RATES];
	uint8_t htcap_present;
	tSirHTCap HTCap;
	uint8_t vhtcap_present;
	tSirVHTCap VHTCap;
	uint8_t uapsdQueues;
	uint8_t maxSp;
} tTdlsAddStaCmdInfo;

typedef struct TdlsDelStaInfo {
	tSirMacAddr peerMac;
} tTdlsDelStaCmdInfo;
/*
 * TDLS cmd info, CMD from SME to PE.
 */
typedef struct s_tdls_cmd {
	uint32_t size;
	union {
		tTdlsLinkEstablishCmdInfo tdlsLinkEstablishCmdInfo;
		tTdlsSendMgmtCmdInfo tdlsSendMgmtCmdInfo;
		tTdlsAddStaCmdInfo tdlsAddStaCmdInfo;
		tTdlsDelStaCmdInfo tdlsDelStaCmdInfo;
	} u;
} tTdlsCmd;
#endif /* FEATURE_WLAN_TDLS */

/**
 * struct s_nss_update_cmd - Format of nss update request
 * @new_nss: new nss value
 * @session_id: Session ID
 * @set_hw_mode_cb: HDD nss update callback
 * @context: Adapter context
 */
struct s_nss_update_cmd {
	uint32_t new_nss;
	uint32_t session_id;
	void *nss_update_cb;
	void *context;
	uint8_t next_action;
};

typedef struct tagSmeCmd {
	tListElem Link;
	eSmeCommandType command;
	uint32_t sessionId;
	union {
		tScanCmd scanCmd;
		tRoamCmd roamCmd;
		tWmStatusChangeCmd wmStatusChangeCmd;
		tSetKeyCmd setKeyCmd;
		tGenericPmcCmd pmcCmd;
		tGenericQosCmd qosCmd;
#ifdef FEATURE_OEM_DATA_SUPPORT
		tOemDataCmd oemDataCmd;
#endif
		tRemainChlCmd remainChlCmd;
		tNoACmd NoACmd;
		tAddStaForSessionCmd addStaSessionCmd;
		tDelStaForSessionCmd delStaSessionCmd;
#ifdef FEATURE_WLAN_TDLS
		tTdlsCmd tdlsCmd;
#endif
		struct sir_hw_mode set_hw_mode_cmd;
		struct s_nss_update_cmd nss_update_cmd;
		struct sir_dual_mac_config set_dual_mac_cmd;
	} u;
} tSmeCmd;

/*--------------------------------------------------------------------------
  Internal to SME
  ------------------------------------------------------------------------*/
tSmeCmd *sme_get_command_buffer(tpAniSirGlobal pMac);
void sme_push_command(tpAniSirGlobal pMac, tSmeCmd *pCmd, bool fHighPriority);
void sme_process_pending_queue(tpAniSirGlobal pMac);
void sme_release_command(tpAniSirGlobal pMac, tSmeCmd *pCmd);
void purge_sme_session_cmd_list(tpAniSirGlobal pMac, uint32_t sessionId,
		tDblLinkList *pList);
bool sme_command_pending(tpAniSirGlobal pMac);
bool pmc_process_command(tpAniSirGlobal pMac, tSmeCmd *pCommand);
bool qos_process_command(tpAniSirGlobal pMac, tSmeCmd *pCommand);
CDF_STATUS csr_process_scan_command(tpAniSirGlobal pMac, tSmeCmd *pCommand);
CDF_STATUS csr_roam_process_command(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csr_roam_process_wm_status_change_command(tpAniSirGlobal pMac,
		tSmeCmd *pCommand);
void csr_reinit_roam_cmd(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csr_reinit_wm_status_change_cmd(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csr_reinit_set_key_cmd(tpAniSirGlobal pMac, tSmeCmd *pCommand);
CDF_STATUS csr_roam_process_set_key_command(tpAniSirGlobal pMac,
		tSmeCmd *pCommand);
void csr_release_command_set_key(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csr_abort_command(tpAniSirGlobal pMac, tSmeCmd *pCommand, bool fStopping);

CDF_STATUS csr_is_valid_channel(tpAniSirGlobal pMac, uint8_t chnNum);
bool csr_roam_is_valid40_mhz_channel(tpAniSirGlobal pmac, uint8_t channel);

CDF_STATUS sme_acquire_global_lock(tSmeStruct *psSme);
CDF_STATUS sme_release_global_lock(tSmeStruct *psSme);

#ifdef FEATURE_OEM_DATA_SUPPORT
CDF_STATUS oem_data_process_oem_data_req_command(tpAniSirGlobal pMac,
		tSmeCmd *pCommand);
#endif

CDF_STATUS csr_process_add_sta_session_command(tpAniSirGlobal pMac,
		tSmeCmd *pCommand);
CDF_STATUS csr_process_add_sta_session_rsp(tpAniSirGlobal pMac, uint8_t *pMsg);
CDF_STATUS csr_process_del_sta_session_command(tpAniSirGlobal pMac,
		tSmeCmd *pCommand);
CDF_STATUS csr_process_del_sta_session_rsp(tpAniSirGlobal pMac, uint8_t *pMsg);

#ifdef FEATURE_WLAN_SCAN_PNO
CDF_STATUS pmc_set_preferred_network_list(tHalHandle hHal,
		tpSirPNOScanReq pRequest,
		uint8_t sessionId,
		preferred_network_found_ind_cb
		callbackRoutine, void *callbackContext);
#endif /* FEATURE_WLAN_SCAN_PNO */
bool csr_roamGetConcurrencyConnectStatusForBmps(tpAniSirGlobal pMac);
#ifdef FEATURE_WLAN_TDLS
CDF_STATUS csr_tdls_send_mgmt_req(tHalHandle hHal, uint8_t sessionId,
		tCsrTdlsSendMgmt * tdlsSendMgmt);
CDF_STATUS csr_tdls_send_link_establish_params(tHalHandle hHal,
		uint8_t sessionId, const tSirMacAddr peerMac,
		tCsrTdlsLinkEstablishParams *tdlsLinkEstablishParams);
CDF_STATUS csr_tdls_add_peer_sta(tHalHandle hHal, uint8_t sessionId,
		const tSirMacAddr peerMac);
CDF_STATUS csr_tdls_change_peer_sta(tHalHandle hHal, uint8_t sessionId,
		const tSirMacAddr peerMac,
		tCsrStaParams *pstaParams);
CDF_STATUS csr_tdls_del_peer_sta(tHalHandle hHal, uint8_t sessionId,
		const tSirMacAddr peerMac);
CDF_STATUS csr_tdls_process_cmd(tpAniSirGlobal pMac, tSmeCmd *pCommand);
CDF_STATUS csr_tdls_process_link_establish(tpAniSirGlobal pMac, tSmeCmd *cmd);
CDF_STATUS tdls_msg_processor(tpAniSirGlobal pMac, uint16_t msg_type,
		void *pMsgBuf);
#endif /* FEATURE_WLAN_TDLS */

#if  defined(WLAN_FEATURE_VOWIFI_11R) || defined(FEATURE_WLAN_ESE) || \
		 defined(FEATURE_WLAN_LFR)
CDF_STATUS csr_flush_cfg_bg_scan_roam_channel_list(tpAniSirGlobal pMac,
		uint8_t sessionId);
CDF_STATUS csr_create_bg_scan_roam_channel_list(tpAniSirGlobal pMac,
		uint8_t sessionId, const uint8_t *pChannelList,
		const uint8_t numChannels);
CDF_STATUS csr_update_bg_scan_config_ini_channel_list(tpAniSirGlobal pMac,
		uint8_t sessionId, eCsrBand eBand);
#endif

#if defined(FEATURE_WLAN_ESE) && defined(FEATURE_WLAN_ESE_UPLOAD)
CDF_STATUS csr_create_roam_scan_channel_list(tpAniSirGlobal pMac,
		uint8_t sessionId,
		uint8_t *pChannelList,
		uint8_t numChannels,
		const eCsrBand eBand);
#endif

ePhyChanBondState csr_convert_cb_ini_value_to_phy_cb_state(uint32_t cbIniValue);
void active_list_cmd_timeout_handle(void *userData);
void csr_process_set_dual_mac_config(tpAniSirGlobal mac, tSmeCmd *command);
void csr_process_set_hw_mode(tpAniSirGlobal mac, tSmeCmd *command);
void csr_process_nss_update_req(tpAniSirGlobal mac, tSmeCmd *command);
#endif /* #if !defined( __SMEINSIDE_H ) */
