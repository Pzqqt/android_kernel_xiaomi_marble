/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
#include "qdf_status.h"
#include "qdf_lock.h"
#include "qdf_trace.h"
#include "qdf_mem.h"
#include "qdf_types.h"
#include "sir_api.h"
#include "csr_internal.h"
#include "sme_qos_api.h"
#include "sme_qos_internal.h"

#include "sme_rrm_api.h"
#include "wlan_serialization_legacy_api.h"
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
/* default sme timeout is set to 30 secs */
#define SME_DEFAULT_CMD_TIMEOUT  30000

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
	struct sme_qos_wmmtspecinfo tspecInfo;
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
	enum wifi_traffic_ac ac;
} tTdlsSendMgmtCmdInfo;

typedef struct TdlsLinkEstablishInfo {
	struct qdf_mac_addr peermac;
	uint8_t uapsdQueues;
	uint8_t maxSp;
	uint8_t isBufSta;
	uint8_t isOffChannelSupported;
	uint8_t isResponder;
	uint8_t supportedChannelsLen;
	uint8_t supportedChannels[SIR_MAC_MAX_SUPP_CHANNELS];
	uint8_t supportedOperClassesLen;
	uint8_t supportedOperClasses[REG_MAX_SUPP_OPER_CLASSES];
} tTdlsLinkEstablishCmdInfo;

typedef struct TdlsAddStaInfo {
	eTdlsAddOper tdlsAddOper;
	struct qdf_mac_addr peermac;
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
	struct qdf_mac_addr peermac;
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
	enum sir_conn_update_reason reason;
};

typedef struct tagSmeCmd {
	tListElem Link;
	eSmeCommandType command;
	uint32_t sessionId;
	union {
		struct roam_cmd roamCmd;
		struct wmstatus_changecmd wmStatusChangeCmd;
		tGenericPmcCmd pmcCmd;
		tGenericQosCmd qosCmd;
		tRemainChlCmd remainChlCmd;
		struct addstafor_sessioncmd addStaSessionCmd;
		struct delstafor_sessionCmd delStaSessionCmd;
#ifdef FEATURE_WLAN_TDLS
		tTdlsCmd tdlsCmd;
#endif
		struct policy_mgr_hw_mode set_hw_mode_cmd;
		struct s_nss_update_cmd nss_update_cmd;
		struct sir_dual_mac_config set_dual_mac_cmd;
		struct sir_antenna_mode_param set_antenna_mode_cmd;
	} u;
} tSmeCmd;

/*--------------------------------------------------------------------------
  Internal to SME
  ------------------------------------------------------------------------*/
/**
 * csr_get_cmd_type() - to convert sme command type to serialization cmd type
 * @sme_cmd: sme command pointer
 *
 * This API will convert SME command type to serialization command type which
 * new serialization module understands
 *
 * Return: serialization cmd type based on sme command type
 */
enum wlan_serialization_cmd_type csr_get_cmd_type(tSmeCmd *sme_cmd);
/**
 * csr_set_serialization_params_to_cmd() - take sme params and create new
 *						serialization command
 * @mac_ctx: pointer to mac context
 * @sme_cmd: sme command pointer
 * @cmd: serialization command pointer
 * @high_priority: if command is high priority
 *
 * Return: QDF_STATUS_SUCCESS or QDF_STATUS_E_FAILURE
 */
QDF_STATUS csr_set_serialization_params_to_cmd(tpAniSirGlobal mac_ctx,
		tSmeCmd *sme_cmd, struct wlan_serialization_command *cmd,
		uint8_t high_priority);
tSmeCmd *sme_get_command_buffer(tpAniSirGlobal pMac);
void sme_release_command(tpAniSirGlobal pMac, tSmeCmd *pCmd);
bool qos_process_command(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void qos_release_command(tpAniSirGlobal pMac, tSmeCmd *pCommand);
QDF_STATUS csr_process_scan_command(tpAniSirGlobal pMac, tSmeCmd *pCommand);
QDF_STATUS csr_roam_process_command(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csr_roam_process_wm_status_change_command(tpAniSirGlobal pMac,
		tSmeCmd *pCommand);
void csr_reinit_roam_cmd(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csr_reinit_wm_status_change_cmd(tpAniSirGlobal pMac, tSmeCmd *pCommand);
QDF_STATUS csr_roam_send_set_key_cmd(tpAniSirGlobal mac_ctx,
		uint32_t session_id, struct setkey_cmd *set_key_cmd);
QDF_STATUS csr_is_valid_channel(tpAniSirGlobal pMac, uint8_t chnNum);

QDF_STATUS sme_acquire_global_lock(tSmeStruct *psSme);
QDF_STATUS sme_release_global_lock(tSmeStruct *psSme);

QDF_STATUS csr_process_add_sta_session_rsp(tpAniSirGlobal pMac, uint8_t *pMsg);
QDF_STATUS csr_process_del_sta_session_rsp(tpAniSirGlobal pMac, uint8_t *pMsg);

bool csr_roamGetConcurrencyConnectStatusForBmps(tpAniSirGlobal pMac);

QDF_STATUS csr_flush_cfg_bg_scan_roam_channel_list(tpAniSirGlobal pMac,
		uint8_t sessionId);
QDF_STATUS csr_create_bg_scan_roam_channel_list(tpAniSirGlobal pMac,
		uint8_t sessionId, const uint8_t *pChannelList,
		const uint8_t numChannels);

#ifdef FEATURE_WLAN_ESE
QDF_STATUS csr_create_roam_scan_channel_list(tpAniSirGlobal pMac,
		uint8_t sessionId,
		uint8_t *pChannelList,
		uint8_t numChannels,
		const enum band_info eBand);
#endif

QDF_STATUS p2p_process_remain_on_channel_cmd(tpAniSirGlobal pMac,
					     tSmeCmd *p2pRemainonChn);
ePhyChanBondState csr_convert_cb_ini_value_to_phy_cb_state(uint32_t cbIniValue);
void csr_process_set_dual_mac_config(tpAniSirGlobal mac, tSmeCmd *command);
void csr_process_set_antenna_mode(tpAniSirGlobal mac, tSmeCmd *command);
void csr_process_set_hw_mode(tpAniSirGlobal mac, tSmeCmd *command);
void csr_process_nss_update_req(tpAniSirGlobal mac, tSmeCmd *command);
#endif /* #if !defined( __SMEINSIDE_H ) */
