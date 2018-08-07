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

/*
 *
 * This file lim_api.h contains the definitions exported by
 * LIM module.
 * Author:        Chandra Modumudi
 * Date:          02/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */
#ifndef __LIM_API_H
#define __LIM_API_H
#include "wni_api.h"
#include "sir_api.h"
#include "ani_global.h"
#include "sir_mac_prot_def.h"
#include "sir_common.h"
#include "sir_debug.h"
#include "sch_global.h"
#include "utils_api.h"
#include "lim_global.h"
#include "wma_if.h"
#include "wma_types.h"
#include "scheduler_api.h"

/* Macro to count heartbeat */
#define limResetHBPktCount(psessionEntry)   (psessionEntry->LimRxedBeaconCntDuringHB = 0)

/* Useful macros for fetching various states in pMac->lim */
/* gLimSystemRole */
#define GET_LIM_SYSTEM_ROLE(psessionEntry)      (psessionEntry->limSystemRole)
#define LIM_IS_AP_ROLE(psessionEntry)           (GET_LIM_SYSTEM_ROLE(psessionEntry) == eLIM_AP_ROLE)
#define LIM_IS_STA_ROLE(psessionEntry)          (GET_LIM_SYSTEM_ROLE(psessionEntry) == eLIM_STA_ROLE)
#define LIM_IS_IBSS_ROLE(psessionEntry)         (GET_LIM_SYSTEM_ROLE(psessionEntry) == eLIM_STA_IN_IBSS_ROLE)
#define LIM_IS_UNKNOWN_ROLE(psessionEntry)      (GET_LIM_SYSTEM_ROLE(psessionEntry) == eLIM_UNKNOWN_ROLE)
#define LIM_IS_P2P_DEVICE_ROLE(psessionEntry)   (GET_LIM_SYSTEM_ROLE(psessionEntry) == eLIM_P2P_DEVICE_ROLE)
#define LIM_IS_P2P_DEVICE_GO(psessionEntry)     (GET_LIM_SYSTEM_ROLE(psessionEntry) == eLIM_P2P_DEVICE_GO)
#define LIM_IS_NDI_ROLE(psessionEntry) \
		(GET_LIM_SYSTEM_ROLE(psessionEntry) == eLIM_NDI_ROLE)
/* gLimSmeState */
#define GET_LIM_SME_STATE(pMac)                 (pMac->lim.gLimSmeState)
#define SET_LIM_SME_STATE(pMac, state)          (pMac->lim.gLimSmeState = state)
/* gLimMlmState */
#define GET_LIM_MLM_STATE(pMac)                 (pMac->lim.gLimMlmState)
#define SET_LIM_MLM_STATE(pMac, state)          (pMac->lim.gLimMlmState = state)
/*tpdphHashNode mlmStaContext*/
#define GET_LIM_STA_CONTEXT_MLM_STATE(pStaDs)   (pStaDs->mlmStaContext.mlmState)
#define SET_LIM_STA_CONTEXT_MLM_STATE(pStaDs, state)  (pStaDs->mlmStaContext.mlmState = state)
/* gLimQuietState */
#define GET_LIM_QUIET_STATE(pMac)               (pMac->lim.gLimSpecMgmt.quietState)
#define SET_LIM_QUIET_STATE(pMac, state)        (pMac->lim.gLimSpecMgmt.quietState = state)
#define LIM_IS_CONNECTION_ACTIVE(psessionEntry)  (psessionEntry->LimRxedBeaconCntDuringHB)
/*pMac->lim.gLimProcessDefdMsgs*/
#define GET_LIM_PROCESS_DEFD_MESGS(pMac) (pMac->lim.gLimProcessDefdMsgs)
#define SET_LIM_PROCESS_DEFD_MESGS(pMac, val) \
		pMac->lim.gLimProcessDefdMsgs = val; \
		pe_debug("%s Defer LIM messages - value %d", __func__, val);

/* LIM exported function templates */
#define LIM_MIN_BCN_PR_LENGTH  12
#define LIM_BCN_PR_CAPABILITY_OFFSET 10
#define LIM_ASSOC_REQ_IE_OFFSET 4

/**
 * enum lim_vendor_ie_access_policy - vendor ie access policy
 * @LIM_ACCESS_POLICY_NONE: access policy not valid
 * @LIM_ACCESS_POLICY_RESPOND_IF_IE_IS_PRESENT: respond only if vendor ie
 *         is present in probe request and assoc request frames
 * @LIM_ACCESS_POLICY_DONOT_RESPOND_IF_IE_IS_PRESENT: do not respond if vendor
 *         ie is present in probe request or assoc request frames
 */
enum lim_vendor_ie_access_policy {
	LIM_ACCESS_POLICY_NONE,
	LIM_ACCESS_POLICY_RESPOND_IF_IE_IS_PRESENT,
	LIM_ACCESS_POLICY_DONOT_RESPOND_IF_IE_IS_PRESENT,
};

typedef enum eMgmtFrmDropReason {
	eMGMT_DROP_NO_DROP,
	eMGMT_DROP_NOT_LAST_IBSS_BCN,
	eMGMT_DROP_INFRA_BCN_IN_IBSS,
	eMGMT_DROP_SCAN_MODE_FRAME,
	eMGMT_DROP_NON_SCAN_MODE_FRAME,
	eMGMT_DROP_INVALID_SIZE,
	eMGMT_DROP_SPURIOUS_FRAME,
	eMGMT_DROP_DUPLICATE_AUTH_FRAME,
	eMGMT_DROP_EXCESSIVE_MGMT_FRAME,
} tMgmtFrmDropReason;

/**
 * Function to initialize LIM state machines.
 * This called upon LIM thread creation.
 */
extern QDF_STATUS lim_initialize(tpAniSirGlobal);
QDF_STATUS pe_open(tpAniSirGlobal pMac, struct cds_config_info *cds_cfg);
QDF_STATUS pe_close(tpAniSirGlobal pMac);
void pe_register_tl_handle(tpAniSirGlobal pMac);
QDF_STATUS lim_start(tpAniSirGlobal pMac);
QDF_STATUS pe_start(tpAniSirGlobal pMac);
void pe_stop(tpAniSirGlobal pMac);
QDF_STATUS peProcessMsg(tpAniSirGlobal pMac, struct scheduler_msg *limMsg);

/**
 * pe_register_mgmt_rx_frm_callback() - registers callback for receiving
 *                                      mgmt rx frames
 * @mac_ctx: mac global ctx
 *
 * This function registers a PE function to mgmt txrx component and a WMA
 * function to WMI layer as event handler for receiving mgmt frames.
 *
 * Return: None
 */
void pe_register_mgmt_rx_frm_callback(tpAniSirGlobal mac_ctx);

/**
 * pe_deregister_mgmt_rx_frm_callback() - degisters callback for receiving
 *                                        mgmt rx frames
 * @mac_ctx: mac global ctx
 *
 * This function deregisters the PE function registered to mgmt txrx component
 * and the WMA function registered to WMI layer as event handler for receiving
 * mgmt frames.
 *
 * Return: None
 */
void pe_deregister_mgmt_rx_frm_callback(tpAniSirGlobal mac_ctx);

/**
 * pe_register_callbacks_with_wma() - register SME and PE callback functions to
 * WMA.
 * @pMac: mac global ctx
 * @ready_req: Ready request parameters, containing callback pointers
 *
 * Return: None
 */
void pe_register_callbacks_with_wma(tpAniSirGlobal pMac,
				    tSirSmeReadyReq *ready_req);

/**
 * Function to cleanup LIM state.
 * This called upon reset/persona change etc
 */
extern void lim_cleanup(tpAniSirGlobal);

/**
 * lim_post_msg_api() - post normal priority PE message
 * @mac: mac context
 * @msg: message to be posted
 *
 * This function is called to post a message to the tail of the PE
 * message queue to be processed in the MC Thread with normal
 * priority.
 *
 * Return: QDF_STATUS_SUCCESS on success, other QDF_STATUS on error
 */
QDF_STATUS lim_post_msg_api(tpAniSirGlobal mac, struct scheduler_msg *msg);

/**
 * lim_post_msg_high_priority() - post high priority PE message
 * @mac: mac context
 * @msg: message to be posted
 *
 * This function is called to post a message to the head of the PE
 * message queue to be processed in the MC Thread with expedited
 * priority.
 *
 * Return: QDF_STATUS_SUCCESS on success, other QDF_STATUS on error
 */
QDF_STATUS lim_post_msg_high_priority(tpAniSirGlobal mac,
				      struct scheduler_msg *msg);

/**
 * Function to process messages posted to LIM thread
 * and dispatch to various sub modules within LIM module.
 */
extern void lim_message_processor(tpAniSirGlobal, struct scheduler_msg *);
/**
 * Function to check the LIM state if system is in Scan/Learn state.
 */
extern uint8_t lim_is_system_in_scan_state(tpAniSirGlobal);
/**
 * Function to handle IBSS coalescing.
 * Beacon Processing module to call this.
 */
extern QDF_STATUS lim_handle_ibss_coalescing(tpAniSirGlobal,
						tpSchBeaconStruct,
						uint8_t *, tpPESession);
/* / Function used by other Sirius modules to read global SME state */
static inline tLimSmeStates lim_get_sme_state(tpAniSirGlobal pMac)
{
	return pMac->lim.gLimSmeState;
}

extern void lim_received_hb_handler(tpAniSirGlobal, uint8_t, tpPESession);
extern void limCheckAndQuietBSS(tpAniSirGlobal);
/* / Function that triggers STA context deletion */
extern void lim_trigger_sta_deletion(tpAniSirGlobal pMac, tpDphHashNode pStaDs,
				     tpPESession psessionEntry);

#ifdef FEATURE_WLAN_TDLS
/* Function that sends TDLS Del Sta indication to SME */
extern void lim_send_sme_tdls_del_sta_ind(tpAniSirGlobal pMac, tpDphHashNode pStaDs,
					  tpPESession psessionEntry,
					  uint16_t reasonCode);
/**
 * lim_set_tdls_flags() - update tdls flags based on newer STA connection
 * information
 * @roam_sync_ind_ptr: pointer to roam offload structure
 * @ft_session_ptr: pointer to PE session
 *
 * Set TDLS flags as per new STA connection capabilities.
 *
 * Return: None
 */
void lim_set_tdls_flags(roam_offload_synch_ind *roam_sync_ind_ptr,
		   tpPESession ft_session_ptr);
#else
static inline void lim_set_tdls_flags(roam_offload_synch_ind *roam_sync_ind_ptr,
		   tpPESession ft_session_ptr)
{
}
#endif

/* / Function that checks for change in AP's capabilties on STA */
extern void lim_detect_change_in_ap_capabilities(tpAniSirGlobal,
						 tpSirProbeRespBeacon, tpPESession);
QDF_STATUS lim_update_short_slot(tpAniSirGlobal pMac,
				    tpSirProbeRespBeacon pBeacon,
				    tpUpdateBeaconParams pBeaconParams,
				    tpPESession);

void lim_ps_offload_handle_missed_beacon_ind(tpAniSirGlobal pMac,
					     struct scheduler_msg *pMsg);
void lim_send_heart_beat_timeout_ind(tpAniSirGlobal pMac, tpPESession psessionEntry);
tMgmtFrmDropReason lim_is_pkt_candidate_for_drop(tpAniSirGlobal pMac,
						 uint8_t *pRxPacketInfo,
						 uint32_t subType);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS pe_roam_synch_callback(tpAniSirGlobal mac_ctx,
	struct sSirSmeRoamOffloadSynchInd *roam_sync_ind_ptr,
	tpSirBssDescription  bss_desc_ptr, enum sir_roam_op_code reason);
#else
static inline QDF_STATUS pe_roam_synch_callback(tpAniSirGlobal mac_ctx,
	struct sSirSmeRoamOffloadSynchInd *roam_sync_ind_ptr,
	tpSirBssDescription  bss_desc_ptr, enum sir_roam_op_code reason)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

/**
 * lim_update_lost_link_info() - update lost link information to SME
 * @mac: global MAC handle
 * @session: PE session
 * @rssi: rssi value from the received frame
 *
 * Return: None
 */
void lim_update_lost_link_info(tpAniSirGlobal mac, tpPESession session,
				int32_t rssi);

/**
 * lim_mon_init_session() - create PE session for monitor mode operation
 * @mac_ptr: mac pointer
 * @msg: Pointer to struct sir_create_session type.
 *
 * Return: NONE
 */
void lim_mon_init_session(tpAniSirGlobal mac_ptr,
			  struct sir_create_session *msg);

#define limGetQosMode(psessionEntry, pVal) (*(pVal) = (psessionEntry)->limQosEnabled)
#define limGetWmeMode(psessionEntry, pVal) (*(pVal) = (psessionEntry)->limWmeEnabled)
#define limGetWsmMode(psessionEntry, pVal) (*(pVal) = (psessionEntry)->limWsmEnabled)
#define limGet11dMode(psessionEntry, pVal) (*(pVal) = (psessionEntry)->lim11dEnabled)
/* ----------------------------------------------------------------------- */
static inline void lim_get_phy_mode(tpAniSirGlobal pMac, uint32_t *phyMode,
				    tpPESession psessionEntry)
{
	*phyMode =
		psessionEntry ? psessionEntry->gLimPhyMode : pMac->lim.gLimPhyMode;
}

/* ----------------------------------------------------------------------- */
static inline void lim_get_rf_band_new(tpAniSirGlobal pMac,
				       enum band_info *band,
				       tpPESession psessionEntry)
{
	*band = psessionEntry ? psessionEntry->limRFBand : BAND_UNKNOWN;
}

/**
 * pe_mc_process_handler() - Message Processor for PE
 * @msg: Pointer to the message structure
 *
 * Verifies the system is in a mode where messages are expected to be
 * processed, and if so, routes the message to the appropriate handler
 * based upon message type.
 *
 * Return: QDF_STATUS_SUCCESS if the message was handled, otherwise an
 *         appropriate QDF_STATUS error code
 */
QDF_STATUS pe_mc_process_handler(struct scheduler_msg *msg);

/** -------------------------------------------------------------
   \fn pe_free_msg
   \brief Called by CDS scheduler (function cds_sched_flush_mc_mqs)
 \      to free a given PE message on the TX and MC thread.
 \      This happens when there are messages pending in the PE
 \      queue when system is being stopped and reset.
   \param   tpAniSirGlobal pMac
   \param   struct scheduler_msg       pMsg
   \return none
   -----------------------------------------------------------------*/
void pe_free_msg(tpAniSirGlobal pMac, struct scheduler_msg *pMsg);

/*--------------------------------------------------------------------------

   \brief lim_remain_on_chn_rsp() - API for sending remain on channel response.

   LIM calls this api to send the remain on channel response to SME.

   \param pMac - Pointer to Global MAC structure
   \param status - status of the response
   \param data - pointer to msg

   \return  void

   --------------------------------------------------------------------------*/
void lim_remain_on_chn_rsp(tpAniSirGlobal pMac, QDF_STATUS status, uint32_t *data);

/**
 * lim_process_abort_scan_ind() - abort the scan which is presently being run
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @vdev_id: vdev_id
 * @scan_id: Scan ID from the scan request
 * @scan_requesor_id: Entity requesting the scan
 *
 * @return: None
 */
void lim_process_abort_scan_ind(tpAniSirGlobal pMac, uint8_t vdev_id,
	uint32_t scan_id, uint32_t scan_requestor_id);

void __lim_process_sme_assoc_cnf_new(tpAniSirGlobal, uint32_t, uint32_t *);

/**
 * lim_process_sme_addts_rsp_timeout(): Send addts rsp timeout to SME
 * @pMac: Pointer to Global MAC structure
 * @param: Addts rsp timer count
 *
 * This function is used to reset the addts sent flag and
 * send addts rsp timeout to SME
 *
 * Return: None
 */
void lim_process_sme_addts_rsp_timeout(tpAniSirGlobal pMac, uint32_t param);
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
void lim_fill_join_rsp_ht_caps(tpPESession session, tpSirSmeJoinRsp rsp);
#else
static inline void lim_fill_join_rsp_ht_caps(tpPESession session,
	tpSirSmeJoinRsp rsp)
{}
#endif
QDF_STATUS lim_update_ext_cap_ie(tpAniSirGlobal mac_ctx,
	uint8_t *ie_data, uint8_t *local_ie_buf, uint16_t *local_ie_len);

/**
 * lim_handle_sap_beacon(): Handle the beacon received from scan module for SAP
 * @pdev: pointer to the pdev object
 * @scan_entry: pointer to the scan cache entry for the beacon
 *
 * Registered as callback to the scan module for handling beacon frames.
 * This API filters the and allows beacons for SAP protection mechanisms
 * if there are active SAP sessions and the received beacon's channel
 * matches the SAP active channel
 *
 * Return: None
 */
void lim_handle_sap_beacon(struct wlan_objmgr_pdev *pdev,
					struct scan_cache_entry *scan_entry);

/************************************************************/
#endif /* __LIM_API_H */
