/*
 * Copyright (c) 2012-2019 The Linux Foundation. All rights reserved.
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

#include "cds_api.h"
#include "ani_global.h"
#include "sir_common.h"
#include "wni_cfg.h"
#include "lim_utils.h"
#include "lim_assoc_utils.h"
#include "lim_sta_hash_api.h"
#include "sch_api.h"             /* sch_set_fixed_beacon_fields for IBSS coalesce */
#include "lim_security_utils.h"
#include "lim_send_messages.h"
#include "lim_ft_defs.h"
#include "lim_session.h"
#include "lim_ibss_peer_mgmt.h"
#include "lim_types.h"
#include "wlan_mlme_api.h"
#include "cfg_ucfg_api.h"

/**
 * ibss_peer_find
 *
 ***FUNCTION:
 * This function is called while adding a context at
 * DPH & Polaris for a peer in IBSS.
 * If peer is found in the list, capabilities from the
 * returned BSS description are used at DPH node & Polaris.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  macAddr - MAC address of the peer
 *
 * @return Pointer to peer node if found, else NULL
 */

static tLimIbssPeerNode *ibss_peer_find(struct mac_context *mac,
					tSirMacAddr macAddr)
{
	tLimIbssPeerNode *pTempNode = mac->lim.gLimIbssPeerList;

	while (pTempNode) {
		if (!qdf_mem_cmp((uint8_t *) macAddr,
				    (uint8_t *) &pTempNode->peerMacAddr,
				    sizeof(tSirMacAddr)))
			break;
		pTempNode = pTempNode->next;
	}
	return pTempNode;
} /*** end ibss_peer_find() ***/

/**
 * ibss_peer_add
 *
 ***FUNCTION:
 * This is called on a STA in IBSS upon receiving Beacon/
 * Probe Response from a peer.
 *
 ***LOGIC:
 * Node is always added to the front of the list
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  mac      - Pointer to Global MAC structure
 * @param  pPeerNode - Pointer to peer node to be added to the list.
 *
 * @return None
 */

static QDF_STATUS
ibss_peer_add(struct mac_context *mac, tLimIbssPeerNode *pPeerNode)
{
#ifdef ANI_SIR_IBSS_PEER_CACHING
	uint32_t numIbssPeers = (2 * mac->lim.maxStation);

	if (mac->lim.gLimNumIbssPeers >= numIbssPeers) {
		/**
		 * Reached max number of peers to be maintained.
		 * Delete last entry & add new entry at the beginning.
		 */
		tLimIbssPeerNode *pTemp, *pPrev;

		pTemp = pPrev = mac->lim.gLimIbssPeerList;
		while (pTemp->next) {
			pPrev = pTemp;
			pTemp = pTemp->next;
		}
		if (pTemp->beacon) {
			qdf_mem_free(pTemp->beacon);
		}

		qdf_mem_free(pTemp);
		pPrev->next = NULL;
	} else
#endif
	mac->lim.gLimNumIbssPeers++;

	pPeerNode->next = mac->lim.gLimIbssPeerList;
	mac->lim.gLimIbssPeerList = pPeerNode;

	return QDF_STATUS_SUCCESS;

} /*** end limAddIbssPeerToList() ***/

/**
 * ibss_peer_collect
 *
 ***FUNCTION:
 * This is called to collect IBSS peer information
 * from received Beacon/Probe Response frame from it.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  mac    - Pointer to Global MAC structure
 * @param  pBeacon - Parsed Beacon Frame structure
 * @param  pBD     - Pointer to received BD
 * @param  peer   - Pointer to IBSS peer node
 *
 * @return None
 */

static void
ibss_peer_collect(struct mac_context *mac,
		  tpSchBeaconStruct pBeacon,
		  tpSirMacMgmtHdr pHdr,
		  tLimIbssPeerNode *peer, struct pe_session *pe_session)
{
	qdf_mem_copy(peer->peerMacAddr, pHdr->sa, sizeof(tSirMacAddr));

	peer->capabilityInfo = pBeacon->capabilityInfo;
	peer->extendedRatesPresent = pBeacon->extendedRatesPresent;
	peer->edcaPresent = pBeacon->edcaPresent;
	peer->wmeEdcaPresent = pBeacon->wmeEdcaPresent;
	peer->wmeInfoPresent = pBeacon->wmeInfoPresent;

	if (pBeacon->IBSSParams.present) {
		peer->atimIePresent = pBeacon->IBSSParams.present;
		peer->peerAtimWindowLength = pBeacon->IBSSParams.atim;
	}

	if (IS_DOT11_MODE_HT(pe_session->dot11mode) &&
	    (pBeacon->HTCaps.present)) {
		peer->htCapable = pBeacon->HTCaps.present;
		qdf_mem_copy((uint8_t *) peer->supportedMCSSet,
			     (uint8_t *) pBeacon->HTCaps.supportedMCSSet,
			     sizeof(peer->supportedMCSSet));
		peer->htGreenfield = (uint8_t) pBeacon->HTCaps.greenField;
		peer->htSupportedChannelWidthSet =
			(uint8_t) pBeacon->HTCaps.supportedChannelWidthSet;
		peer->htMIMOPSState =
			(tSirMacHTMIMOPowerSaveState) pBeacon->HTCaps.mimoPowerSave;
		peer->htMaxAmsduLength =
			(uint8_t) pBeacon->HTCaps.maximalAMSDUsize;
		peer->htAMpduDensity = pBeacon->HTCaps.mpduDensity;
		peer->htDsssCckRate40MHzSupport =
			(uint8_t) pBeacon->HTCaps.dsssCckMode40MHz;
		peer->htShortGI20Mhz = (uint8_t) pBeacon->HTCaps.shortGI20MHz;
		peer->htShortGI40Mhz = (uint8_t) pBeacon->HTCaps.shortGI40MHz;
		peer->htMaxRxAMpduFactor = pBeacon->HTCaps.maxRxAMPDUFactor;
		peer->htSecondaryChannelOffset =
			pBeacon->HTInfo.secondaryChannelOffset;
		peer->htLdpcCapable = (uint8_t) pBeacon->HTCaps.advCodingCap;
	}

	/* Collect peer VHT capabilities based on the received beacon from the peer */
	if (pBeacon->VHTCaps.present) {
		peer->vhtSupportedChannelWidthSet =
			pBeacon->VHTOperation.chanWidth;
		peer->vhtCapable = pBeacon->VHTCaps.present;

		/* Collect VHT capabilities from beacon */
		qdf_mem_copy((uint8_t *) &peer->VHTCaps,
			     (uint8_t *) &pBeacon->VHTCaps,
			     sizeof(tDot11fIEVHTCaps));
	}
	peer->erpIePresent = pBeacon->erpPresent;

	qdf_mem_copy((uint8_t *) &peer->supportedRates,
		     (uint8_t *) &pBeacon->supportedRates,
		     pBeacon->supportedRates.numRates + 1);
	if (peer->extendedRatesPresent)
		qdf_mem_copy((uint8_t *) &peer->extendedRates,
			     (uint8_t *) &pBeacon->extendedRates,
			     pBeacon->extendedRates.numRates + 1);
	else
		peer->extendedRates.numRates = 0;

	peer->next = NULL;
} /*** end ibss_peer_collect() ***/

/* handle change in peer qos/wme capabilities */
static void
ibss_sta_caps_update(struct mac_context *mac,
		     tLimIbssPeerNode *pPeerNode, struct pe_session *pe_session)
{
	uint16_t peerIdx;
	tpDphHashNode sta;

	pPeerNode->beaconHBCount++;     /* Update beacon count. */

	/* if the peer node exists, update its qos capabilities */
	sta = dph_lookup_hash_entry(mac, pPeerNode->peerMacAddr, &peerIdx,
				       &pe_session->dph.dphHashTable);
	if (!sta)
		return;

	/* Update HT Capabilities */
	if (IS_DOT11_MODE_HT(pe_session->dot11mode)) {
		sta->mlmStaContext.htCapability = pPeerNode->htCapable;
		if (pPeerNode->htCapable) {
			sta->htGreenfield = pPeerNode->htGreenfield;
			sta->htSupportedChannelWidthSet =
				pPeerNode->htSupportedChannelWidthSet;
			sta->htSecondaryChannelOffset =
				pPeerNode->htSecondaryChannelOffset;
			sta->htMIMOPSState = pPeerNode->htMIMOPSState;
			sta->htMaxAmsduLength = pPeerNode->htMaxAmsduLength;
			sta->htAMpduDensity = pPeerNode->htAMpduDensity;
			sta->htDsssCckRate40MHzSupport =
				pPeerNode->htDsssCckRate40MHzSupport;
			sta->htShortGI20Mhz = pPeerNode->htShortGI20Mhz;
			sta->htShortGI40Mhz = pPeerNode->htShortGI40Mhz;
			sta->htMaxRxAMpduFactor =
				pPeerNode->htMaxRxAMpduFactor;
			sta->htLdpcCapable = pPeerNode->htLdpcCapable;
		}
	}
	if (IS_DOT11_MODE_VHT(pe_session->dot11mode)) {
		sta->mlmStaContext.vhtCapability = pPeerNode->vhtCapable;
		if (pPeerNode->vhtCapable) {
			sta->vhtSupportedChannelWidthSet =
				pPeerNode->vhtSupportedChannelWidthSet;

			/* If in 11AC mode and if session requires 11AC mode, consider peer's */
			/* max AMPDU length factor */
			sta->htMaxRxAMpduFactor =
				pPeerNode->VHTCaps.maxAMPDULenExp;
			sta->vhtLdpcCapable =
				(uint8_t) pPeerNode->VHTCaps.ldpcCodingCap;
		}
	}
	/* peer is 11e capable but is not 11e enabled yet */
	/* some STA's when joining Airgo IBSS, assert qos capability even when */
	/* they don't support qos. however, they do not include the edca parameter */
	/* set. so let's check for edcaParam in addition to the qos capability */
	if (pPeerNode->capabilityInfo.qos && (pe_session->limQosEnabled)
	    && pPeerNode->edcaPresent) {
		sta->qosMode = 1;
		sta->wmeEnabled = 0;
		if (!sta->lleEnabled) {
			sta->lleEnabled = 1;
			/* dphSetACM(mac, sta); */
		}
		return;
	}
	/* peer is not 11e capable now but was 11e enabled earlier */
	else if (sta->lleEnabled) {
		sta->qosMode = 0;
		sta->lleEnabled = 0;
	}
	/* peer is wme capable but is not wme enabled yet */
	if (pPeerNode->wmeInfoPresent && pe_session->limWmeEnabled) {
		sta->qosMode = 1;
		sta->lleEnabled = 0;
		if (!sta->wmeEnabled) {
			sta->wmeEnabled = 1;
		}
		return;
	}
	/* When the peer device supports EDCA parameters, then we were not
	   considering. Added this code when we saw that one of the Peer Device
	   was advertising WMM param where we were not honouring that. CR# 210756
	 */
	if (pPeerNode->wmeEdcaPresent && pe_session->limWmeEnabled) {
		sta->qosMode = 1;
		sta->lleEnabled = 0;
		if (!sta->wmeEnabled) {
			sta->wmeEnabled = 1;
		}
		return;
	}
	/* peer is not wme capable now but was wme enabled earlier */
	else if (sta->wmeEnabled) {
		sta->qosMode = 0;
		sta->wmeEnabled = 0;
	}

}

static void
ibss_sta_rates_update(struct mac_context *mac,
		      tpDphHashNode sta,
		      tLimIbssPeerNode *peer, struct pe_session *pe_session)
{
	lim_populate_matching_rate_set(mac, sta, &peer->supportedRates,
				       &peer->extendedRates,
				       peer->supportedMCSSet, pe_session,
				       &peer->VHTCaps, NULL);
	sta->mlmStaContext.capabilityInfo = peer->capabilityInfo;
} /*** end ibss_sta_info_update() ***/

/**
 * ibss_sta_info_update
 *
 ***FUNCTION:
 * This is called to program both SW & Polaris context
 * for peer in IBSS.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  mac   - Pointer to Global MAC structure
 * @param  sta - Pointer to DPH node
 * @param  peer  - Pointer to IBSS peer node
 *
 * @return None
 */

static void
ibss_sta_info_update(struct mac_context *mac,
		     tpDphHashNode sta,
		     tLimIbssPeerNode *peer, struct pe_session *pe_session)
{
	sta->staType = STA_ENTRY_PEER;
	ibss_sta_caps_update(mac, peer, pe_session);
	ibss_sta_rates_update(mac, sta, peer, pe_session);
} /*** end ibss_sta_info_update() ***/

static void ibss_coalesce_free(struct mac_context *mac)
{
	qdf_mem_free(mac->lim.ibss_info.mac_hdr);
	mac->lim.ibss_info.mac_hdr = NULL;
	qdf_mem_free(mac->lim.ibss_info.beacon);
	mac->lim.ibss_info.beacon = NULL;
}

/*
 * save the beacon params for use when adding the bss
 */
static void
ibss_coalesce_save(struct mac_context *mac,
		   tpSirMacMgmtHdr pHdr, tpSchBeaconStruct pBeacon)
{
	/* get rid of any saved info */
	ibss_coalesce_free(mac);

	mac->lim.ibss_info.mac_hdr = qdf_mem_malloc(sizeof(*pHdr));
	if (!mac->lim.ibss_info.mac_hdr)
		return;

	mac->lim.ibss_info.beacon = qdf_mem_malloc(sizeof(*pBeacon));
	if (!mac->lim.ibss_info.beacon) {
		ibss_coalesce_free(mac);
		return;
	}

	qdf_mem_copy(mac->lim.ibss_info.mac_hdr, pHdr, sizeof(*pHdr));
	qdf_mem_copy(mac->lim.ibss_info.beacon, pBeacon, sizeof(*pBeacon));
}

static QDF_STATUS lim_ibss_add_bss(
			struct mac_context *mac,
			struct pe_session *session,
			tLimMlmStartReq mlmStartReq)
{
	return wlan_vdev_mlme_sm_deliver_evt(
				session->vdev,
				WLAN_VDEV_SM_EV_START,
				sizeof(mlmStartReq), &mlmStartReq);
}

void lim_ibss_delete(struct mac_context *mac, struct pe_session *session)
{
	ibss_coalesce_free(mac);
}

/*
 * tries to add a new entry to dph hash node
 * if necessary, an existing entry is eliminated
 */
static QDF_STATUS
ibss_dph_entry_add(struct mac_context *mac,
		   tSirMacAddr peerAddr,
		   tpDphHashNode *ppSta, struct pe_session *pe_session)
{
	uint16_t peerIdx;
	tpDphHashNode sta;

	*ppSta = NULL;

	sta =
		dph_lookup_hash_entry(mac, peerAddr, &peerIdx,
				      &pe_session->dph.dphHashTable);
	if (sta) {
		/* Trying to add context for already existing STA in IBSS */
		pe_err("STA exists already");
		lim_print_mac_addr(mac, peerAddr, LOGE);
		return QDF_STATUS_E_FAILURE;
	}

	/**
	 * Assign an AID, delete context existing with that
	 * AID and then add an entry to hash table maintained
	 * by DPH module.
	 */
	peerIdx = lim_assign_peer_idx(mac, pe_session);

	sta =
		dph_get_hash_entry(mac, peerIdx, &pe_session->dph.dphHashTable);
	if (sta) {
		(void)lim_del_sta(mac, sta, false /*asynchronous */,
				  pe_session);
		lim_delete_dph_hash_entry(mac, sta->staAddr, peerIdx,
					  pe_session);
	}

	sta =
		dph_add_hash_entry(mac, peerAddr, peerIdx,
				   &pe_session->dph.dphHashTable);
	if (!sta) {
		/* Could not add hash table entry */
		pe_err("could not add hash entry at DPH for peerIdx/aid: %d MACaddr:",
			       peerIdx);
		lim_print_mac_addr(mac, peerAddr, LOGE);
		return QDF_STATUS_E_FAILURE;
	}

	*ppSta = sta;
	return QDF_STATUS_SUCCESS;
}

/* send a status change notification */
static void
ibss_status_chg_notify(struct mac_context *mac, tSirMacAddr peerAddr,
		       uint16_t staIndex, uint16_t status, uint8_t sessionId)
{

	tLimIbssPeerNode *peerNode;
	uint8_t *beacon = NULL;
	uint16_t bcnLen = 0;

	peerNode = ibss_peer_find(mac, peerAddr);
	if (peerNode) {
		if (!peerNode->beacon)
			peerNode->beaconLen = 0;
		beacon = peerNode->beacon;
		bcnLen = peerNode->beaconLen;
		peerNode->beacon = NULL;
		peerNode->beaconLen = 0;
	}

	lim_send_sme_ibss_peer_ind(mac, peerAddr, staIndex,
				   beacon, bcnLen, status, sessionId);

	if (beacon) {
		qdf_mem_free(beacon);
	}
}

void ibss_bss_add(struct mac_context *mac, struct pe_session *pe_session)
{
	tLimMlmStartReq mlmStartReq;
	uint32_t cfg;
	tpSirMacMgmtHdr pHdr = mac->lim.ibss_info.mac_hdr;
	tpSchBeaconStruct pBeacon = mac->lim.ibss_info.beacon;
	qdf_size_t num_ext_rates = 0;
	QDF_STATUS status;

	if ((!pHdr) || (!pBeacon)) {
		pe_err("Unable to add BSS (no cached BSS info)");
		return;
	}

	qdf_mem_copy(pe_session->bssId, pHdr->bssId, sizeof(tSirMacAddr));

	sir_copy_mac_addr(pHdr->bssId, pe_session->bssId);

	/* Copy beacon interval from sessionTable */
	cfg = pe_session->beaconParams.beaconInterval;
	if (cfg != pBeacon->beaconInterval)
		pe_session->beaconParams.beaconInterval =
			pBeacon->beaconInterval;

	/* This function ibss_bss_add (and hence the below code) is only called during ibss coalescing. We need to
	 * adapt to peer's capability with respect to short slot time. Changes have been made to lim_apply_configuration()
	 * so that the IBSS doesn't blindly start with short slot = 1. If IBSS start is part of coalescing then it will adapt
	 * to peer's short slot using code below.
	 */
	/* If cfg is already set to current peer's capability then no need to set it again */
	if (pe_session->shortSlotTimeSupported !=
	    pBeacon->capabilityInfo.shortSlotTime) {
		pe_session->shortSlotTimeSupported =
			pBeacon->capabilityInfo.shortSlotTime;
	}
	qdf_mem_copy((uint8_t *) &pe_session->pLimStartBssReq->
		     operationalRateSet, (uint8_t *) &pBeacon->supportedRates,
		     pBeacon->supportedRates.numRates);

	/**
	 * WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET CFG needs to be reset, when
	 * there is no extended rate IE present in beacon. This is especially important when
	 * supportedRateSet IE contains all the extended rates as well and STA decides to coalesce.
	 * In this IBSS coalescing scenario LIM will tear down the BSS and Add a new one. So LIM needs to
	 * reset this CFG, just in case CSR originally had set this CFG when IBSS was started from the local profile.
	 * If IBSS was started by CSR from the BssDescription, then it would reset this CFG before StartBss is issued.
	 * The idea is that the count of OpRateSet and ExtendedOpRateSet rates should not be more than 12.
	 */

	if (pBeacon->extendedRatesPresent)
		num_ext_rates = pBeacon->extendedRates.numRates;
	if (wlan_mlme_set_cfg_str(
		(uint8_t *)&pBeacon->extendedRates.rate,
		&mac->mlme_cfg->rates.ext_opr_rate_set,
		num_ext_rates) != QDF_STATUS_SUCCESS) {
		pe_err("could not update ExtendedOperRateset at CFG");
		return;
	}

	/*
	 * Each IBSS node will advertise its own HT Capabilities instead of adapting to the Peer's capabilities
	 * If we don't do this then IBSS may not go back to full capabilities when the STA with lower capabilities
	 * leaves the IBSS.  e.g. when non-CB STA joins an IBSS and then leaves, the IBSS will be stuck at non-CB mode
	 * even though all the nodes are capable of doing CB.
	 * so it is decided to leave the self HT capabilties intact. This may change if some issues are found in interop.
	 */
	qdf_mem_zero((void *)&mlmStartReq, sizeof(mlmStartReq));

	qdf_mem_copy(mlmStartReq.bssId, pHdr->bssId, sizeof(tSirMacAddr));
	mlmStartReq.rateSet.numRates =
		pe_session->pLimStartBssReq->operationalRateSet.numRates;
	qdf_mem_copy(&mlmStartReq.rateSet.rate[0],
		     &pe_session->pLimStartBssReq->operationalRateSet.
		     rate[0], mlmStartReq.rateSet.numRates);
	mlmStartReq.bssType = eSIR_IBSS_MODE;
	mlmStartReq.beaconPeriod = pBeacon->beaconInterval;
	mlmStartReq.nwType = pe_session->pLimStartBssReq->nwType;    /* pe_session->nwType is also OK???? */
	mlmStartReq.htCapable = pe_session->htCapability;
	mlmStartReq.htOperMode = mac->lim.gHTOperMode;
	mlmStartReq.dualCTSProtection = mac->lim.gHTDualCTSProtection;
	mlmStartReq.txChannelWidthSet = pe_session->htRecommendedTxWidthSet;

	/* reading the channel num from session Table */
	mlmStartReq.channelNumber = pe_session->currentOperChannel;

	mlmStartReq.cbMode = pe_session->pLimStartBssReq->cbMode;

	/* Copy the SSID for RxP filtering based on SSID. */
	qdf_mem_copy((uint8_t *) &mlmStartReq.ssId,
		     (uint8_t *) &pe_session->pLimStartBssReq->ssId,
		     pe_session->pLimStartBssReq->ssId.length + 1);

	pe_debug("invoking ADD_BSS as part of coalescing!");

	status = lim_ibss_add_bss(mac, pe_session, mlmStartReq);
	if (QDF_IS_STATUS_ERROR(status)) {
		pe_err("AddBss failure");
		return;
	}
	/* Update fields in Beacon */
	if (sch_set_fixed_beacon_fields(mac, pe_session) != QDF_STATUS_SUCCESS) {
		pe_err("Unable to set fixed Beacon fields");
		return;
	}

}

void ibss_bss_delete(struct mac_context *mac_ctx, struct pe_session *session)
{
	QDF_STATUS status;

	pe_debug("Initiating IBSS Delete BSS");
	if (session->limMlmState != eLIM_MLM_BSS_STARTED_STATE) {
		pe_warn("Incorrect LIM MLM state for delBss: %d",
			session->limMlmState);
		return;
	}
	status = lim_del_bss(mac_ctx, NULL, session->bss_idx, session);
	if (QDF_IS_STATUS_ERROR(status))
		pe_err("delBss failed for bss: %d", session->bss_idx);
}

/**
 * lim_ibss_init
 *
 ***FUNCTION:
 * This function is called while starting an IBSS
 * to initialize list used to maintain IBSS peers.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  mac - Pointer to Global MAC structure
 * @return None
 */

void lim_ibss_init(struct mac_context *mac)
{
	mac->lim.gLimIbssCoalescingHappened = 0;
	mac->lim.gLimIbssPeerList = NULL;
	mac->lim.gLimNumIbssPeers = 0;

	/* ibss info - params for which ibss to join while coalescing */
	qdf_mem_zero(&mac->lim.ibss_info, sizeof(mac->lim.ibss_info));
} /*** end lim_ibss_init() ***/

/**
 * lim_ibss_delete_all_peers
 *
 ***FUNCTION:
 * This function is called to delete all peers.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  mac - Pointer to Global MAC structure
 * @return None
 */
void lim_ibss_delete_all_peers(struct mac_context *mac,
			       struct pe_session *pe_session)
{
	tLimIbssPeerNode *pCurrNode, *pTempNode;
	tpDphHashNode sta;
	uint16_t peerIdx;

	pCurrNode = pTempNode = mac->lim.gLimIbssPeerList;

	while (pCurrNode) {
		if (!mac->lim.gLimNumIbssPeers) {
			pe_err("Number of peers in the list is zero and node present");
			return;
		}
		/* Delete the dph entry for the station
		 * Since it is called to remove all peers, just delete from dph,
		 * no need to do any beacon related params i.e., dont call lim_delete_dph_hash_entry
		 */
		sta =
			dph_lookup_hash_entry(mac, pCurrNode->peerMacAddr, &peerIdx,
					      &pe_session->dph.dphHashTable);
		if (sta) {

			ibss_status_chg_notify(mac, pCurrNode->peerMacAddr,
					       sta->staIndex,
					       eWNI_SME_IBSS_PEER_DEPARTED_IND,
					       pe_session->smeSessionId);
			lim_del_sta(mac, sta, false, pe_session);
			lim_release_peer_idx(mac, peerIdx, pe_session);
			dph_delete_hash_entry(mac, sta->staAddr, peerIdx,
					      &pe_session->dph.dphHashTable);
		}

		pTempNode = pCurrNode->next;

		/* TODO :Sessionize this code */
		/* Fix CR 227642: PeerList should point to the next node since the current node is being
		 * freed in the next line. In ibss_peerfind in ibss_status_chg_notify above, we use this
		 * peer list to find the next peer. So this list needs to be updated with the no of peers left
		 * after each iteration in this while loop since one by one peers are deleted (freed) in this
		 * loop causing the lim.gLimIbssPeerList to point to some freed memory.
		 */
		mac->lim.gLimIbssPeerList = pTempNode;

		if (pCurrNode->beacon) {
			qdf_mem_free(pCurrNode->beacon);
		}
		qdf_mem_free(pCurrNode);
		if (mac->lim.gLimNumIbssPeers > 0) /* be paranoid */
			mac->lim.gLimNumIbssPeers--;
		pCurrNode = pTempNode;
	}

	if (mac->lim.gLimNumIbssPeers)
		pe_err("Number of peers: %d in the list is non-zero",
			mac->lim.gLimNumIbssPeers);

	mac->lim.gLimNumIbssPeers = 0;
	mac->lim.gLimIbssPeerList = NULL;
	lim_disconnect_complete(pe_session, false);
}

/** -------------------------------------------------------------
   \fn lim_ibss_set_protection
   \brief Decides all the protection related information.
 \
   \param  struct mac_context *   mac
   \param  tSirMacAddr peerMacAddr
   \param  tpUpdateBeaconParams pBeaconParams
   \return None
   -------------------------------------------------------------*/
static void
lim_ibss_set_protection(struct mac_context *mac, uint8_t enable,
			tpUpdateBeaconParams pBeaconParams,
			struct pe_session *pe_session)
{

	if (!mac->lim.cfgProtection.fromllb) {
		pe_err("protection from 11b is disabled");
		return;
	}

	if (enable) {
		pe_session->gLim11bParams.protectionEnabled = true;
		if (false ==
		    pe_session->beaconParams.
		    llbCoexist /*mac->lim.llbCoexist */) {
			pe_debug("=> IBSS: Enable Protection");
			pBeaconParams->llbCoexist =
				pe_session->beaconParams.llbCoexist = true;
			pBeaconParams->paramChangeBitmap |=
				PARAM_llBCOEXIST_CHANGED;
		}
	} else if (true ==
		   pe_session->beaconParams.
		   llbCoexist /*mac->lim.llbCoexist */) {
		pe_session->gLim11bParams.protectionEnabled = false;
		pe_debug("===> IBSS: Disable protection");
		pBeaconParams->llbCoexist =
			pe_session->beaconParams.llbCoexist = false;
		pBeaconParams->paramChangeBitmap |= PARAM_llBCOEXIST_CHANGED;
	}
	return;
}

/** -------------------------------------------------------------
   \fn lim_ibss_update_protection_params
   \brief Decides all the protection related information.
 \
   \param  struct mac_context *   mac
   \param  tSirMacAddr peerMacAddr
   \param  tpUpdateBeaconParams pBeaconParams
   \return None
   -------------------------------------------------------------*/
static void
lim_ibss_update_protection_params(struct mac_context *mac,
				  tSirMacAddr peerMacAddr,
				  tLimProtStaCacheType protStaCacheType,
				  struct pe_session *pe_session)
{
	uint32_t i;

	pe_debug("STA is associated Addr :");
	lim_print_mac_addr(mac, peerMacAddr, LOGD);

	for (i = 0; i < LIM_PROT_STA_CACHE_SIZE; i++) {
		if (mac->lim.protStaCache[i].active) {
			pe_debug("Addr:");
			lim_print_mac_addr
				(mac, mac->lim.protStaCache[i].addr, LOGD);

			if (!qdf_mem_cmp(mac->lim.protStaCache[i].addr,
					    peerMacAddr,
					    sizeof(tSirMacAddr))) {
				pe_debug("matching cache entry at: %d already active",
					i);
				return;
			}
		}
	}

	for (i = 0; i < LIM_PROT_STA_CACHE_SIZE; i++) {
		if (!mac->lim.protStaCache[i].active)
			break;
	}

	if (i >= LIM_PROT_STA_CACHE_SIZE) {
		pe_err("No space in ProtStaCache");
		return;
	}

	qdf_mem_copy(mac->lim.protStaCache[i].addr,
		     peerMacAddr, sizeof(tSirMacAddr));

	mac->lim.protStaCache[i].protStaCacheType = protStaCacheType;
	mac->lim.protStaCache[i].active = true;
	if (eLIM_PROT_STA_CACHE_TYPE_llB == protStaCacheType) {
		pe_session->gLim11bParams.numSta++;
	} else if (eLIM_PROT_STA_CACHE_TYPE_llG == protStaCacheType) {
		pe_session->gLim11gParams.numSta++;
	}
}

/** -------------------------------------------------------------
   \fn lim_ibss_decide_protection
   \brief Decides all the protection related information.
 \
   \param  struct mac_context *   mac
   \param  tSirMacAddr peerMacAddr
   \param  tpUpdateBeaconParams pBeaconParams
   \return None
   -------------------------------------------------------------*/
static void
lim_ibss_decide_protection(struct mac_context *mac, tpDphHashNode sta,
			   tpUpdateBeaconParams pBeaconParams,
			   struct pe_session *pe_session)
{
	enum band_info rfBand = BAND_UNKNOWN;
	uint32_t phyMode;
	tLimProtStaCacheType protStaCacheType =
		eLIM_PROT_STA_CACHE_TYPE_INVALID;

	pBeaconParams->paramChangeBitmap = 0;

	if (!sta) {
		pe_err("sta is NULL");
		return;
	}

	lim_get_rf_band_new(mac, &rfBand, pe_session);
	if (BAND_2G == rfBand) {
		lim_get_phy_mode(mac, &phyMode, pe_session);

		/* We are 11G or 11n. Check if we need protection from 11b Stations. */
		if ((phyMode == WNI_CFG_PHY_MODE_11G)
		    || (pe_session->htCapability)) {
			/* As we found in the past, it is possible that a 11n STA sends
			 * Beacon with HT IE but not ERP IE.  So the absence of ERP IE
			 * in the Beacon is not enough to conclude that STA is 11b.
			 */
			if ((sta->erpEnabled == eHAL_CLEAR) &&
			    (!sta->mlmStaContext.htCapability)) {
				protStaCacheType = eLIM_PROT_STA_CACHE_TYPE_llB;
				pe_err("Enable protection from 11B");
				lim_ibss_set_protection(mac, true,
							pBeaconParams,
							pe_session);
			}
		}
	}
	lim_ibss_update_protection_params(mac, sta->staAddr, protStaCacheType,
					  pe_session);
	return;
}

/**
 * lim_ibss_peer_find()
 *
 ***FUNCTION:
 * This function is called while adding a context at
 * DPH & Polaris for a peer in IBSS.
 * If peer is found in the list, capabilities from the
 * returned BSS description are used at DPH node & Polaris.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  macAddr - MAC address of the peer
 *
 * @return Pointer to peer node if found, else NULL
 */
tLimIbssPeerNode *lim_ibss_peer_find(struct mac_context *mac, tSirMacAddr macAddr)
{
	return ibss_peer_find(mac, macAddr);
}

/**
 * lim_ibss_sta_add()
 *
 ***FUNCTION:
 * This function is called to add an STA context in IBSS role
 * whenever a data frame is received from/for a STA that failed
 * hash lookup at DPH.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * NA
 *
 ***NOTE:
 * NA
 *
 * @param  mac       Pointer to Global MAC structure
 * @param  peerAdddr  MAC address of the peer being added
 * @return retCode    Indicates success or failure return code
 * @return
 */

QDF_STATUS
lim_ibss_sta_add(struct mac_context *mac, void *pBody, struct pe_session *pe_session)
{
	QDF_STATUS retCode = QDF_STATUS_SUCCESS;
	tpDphHashNode sta;
	tLimIbssPeerNode *pPeerNode;
	tLimMlmStates prevState;
	tSirMacAddr *pPeerAddr = (tSirMacAddr *) pBody;
	tUpdateBeaconParams beaconParams;

	qdf_mem_zero((uint8_t *) &beaconParams, sizeof(tUpdateBeaconParams));

	if (pBody == 0) {
		pe_err("Invalid IBSS AddSta");
		return QDF_STATUS_E_FAILURE;
	}

	pe_debug("Rx Add-Ibss-Sta for MAC:");
	lim_print_mac_addr(mac, *pPeerAddr, LOGD);

	pPeerNode = ibss_peer_find(mac, *pPeerAddr);
	if (pPeerNode) {
		retCode =
			ibss_dph_entry_add(mac, *pPeerAddr, &sta,
					   pe_session);
		if (QDF_STATUS_SUCCESS == retCode) {
			prevState = sta->mlmStaContext.mlmState;
			sta->erpEnabled = pPeerNode->erpIePresent;

			ibss_sta_info_update(mac, sta, pPeerNode,
					     pe_session);
			pe_debug("initiating ADD STA for the IBSS peer");
			retCode =
				lim_add_sta(mac, sta, false, pe_session);
			if (retCode != QDF_STATUS_SUCCESS) {
				pe_err("ibss-sta-add failed (reason %x)",
					       retCode);
				lim_print_mac_addr(mac, *pPeerAddr, LOGE);
				sta->mlmStaContext.mlmState = prevState;
				dph_delete_hash_entry(mac, sta->staAddr,
						      sta->assocId,
						      &pe_session->dph.
						      dphHashTable);
			} else {
				if (mac->lim.gLimProtectionControl !=
				    MLME_FORCE_POLICY_PROTECTION_DISABLE)
					lim_ibss_decide_protection(mac, sta,
								   &beaconParams,
								   pe_session);

				if (beaconParams.paramChangeBitmap) {
					pe_debug("---> Update Beacon Params");
					sch_set_fixed_beacon_fields(mac,
								    pe_session);
					beaconParams.bss_idx =
						pe_session->bss_idx;
					lim_send_beacon_params(mac, &beaconParams,
							       pe_session);
				}
			}
		} else {
			pe_err("hashTblAdd failed reason: %x", retCode);
			lim_print_mac_addr(mac, *pPeerAddr, LOGE);
		}
	} else {
		retCode = QDF_STATUS_E_FAILURE;
	}

	return retCode;
}

/**
 * lim_ibss_search_and_delete_peer()- to cleanup the IBSS
 * peer from lim ibss peer list
 *
 * @mac_ptr: Pointer to Global MAC structure
 * @session_entry: Session entry
 * @mac_addr: Mac Address of the IBSS peer
 *
 * This function is called to cleanup the IBSS peer from
 * lim ibss peer list
 *
 * Return: None
 *
 */
static void
lim_ibss_search_and_delete_peer(struct mac_context *mac_ctx,
			struct pe_session *session_entry, tSirMacAddr mac_addr)
{
	tLimIbssPeerNode *temp_node, *prev_node;
	tLimIbssPeerNode *temp_next_node = NULL;

	prev_node = temp_node = mac_ctx->lim.gLimIbssPeerList;

	pe_debug(" PEER ADDR :" QDF_MAC_ADDR_STR,
		QDF_MAC_ADDR_ARRAY(mac_addr));

	/** Compare Peer */
	while (temp_node) {
		temp_next_node = temp_node->next;

		/* Delete the STA with MAC address */
		if (!qdf_mem_cmp((uint8_t *) mac_addr,
				    (uint8_t *) &temp_node->peerMacAddr,
				    sizeof(tSirMacAddr))) {
			if (temp_node ==
			   mac_ctx->lim.gLimIbssPeerList) {
				mac_ctx->lim.gLimIbssPeerList =
					temp_node->next;
				prev_node =
					mac_ctx->lim.gLimIbssPeerList;
			} else
				prev_node->next = temp_node->next;
			if (temp_node->beacon)
				qdf_mem_free(temp_node->beacon);

			qdf_mem_free(temp_node);
			mac_ctx->lim.gLimNumIbssPeers--;

			temp_node = temp_next_node;
			break;
		}
		prev_node = temp_node;
		temp_node = temp_next_node;
	}
	/*
	 * if it is the last peer walking out, we better
	 * we set IBSS state to inactive.
	 */
	if (0 == mac_ctx->lim.gLimNumIbssPeers) {
		pe_debug("Last STA from IBSS walked out");
		session_entry->limIbssActive = false;
	}
}

/**
 * lim_ibss_delete_peer()- to delete IBSS peer
 *
 * @mac_ptr: Pointer to Global MAC structure
 * @session_entry: Session entry
 * @mac_addr: Mac Address of the IBSS peer
 *
 * This function is called delete IBSS peer.
 *
 * Return: None
 *
 */
static void
lim_ibss_delete_peer(struct mac_context *mac_ctx,
			struct pe_session *session_entry, tSirMacAddr mac_addr)
{
	tpDphHashNode sta = NULL;
	uint16_t peer_idx = 0;

	pe_debug("Delete peer :" QDF_MAC_ADDR_STR,
		QDF_MAC_ADDR_ARRAY(mac_addr));

	sta = dph_lookup_hash_entry(mac_ctx, mac_addr,
			&peer_idx,
			&session_entry->dph.
			dphHashTable);

	if (!sta) {
		pe_err("DPH Entry for STA %pM is missing",
			mac_addr);
		return;
	}

	if (STA_INVALID_IDX != sta->staIndex) {
		lim_del_sta(mac_ctx, sta,
			  true, session_entry);
	} else {
		/*
		 * This mean ADD STA failed, thus remove the sta from
		 * from database and no need to send del sta to firmware
		 * and peer departed indication to upper layer.
		 */
		lim_delete_dph_hash_entry(mac_ctx, sta->staAddr,
			  peer_idx, session_entry);
		lim_release_peer_idx(mac_ctx,
			peer_idx, session_entry);
		lim_ibss_search_and_delete_peer(mac_ctx,
			session_entry, mac_addr);
	}

}

void lim_process_ibss_del_sta_rsp(struct mac_context *mac_ctx,
	struct scheduler_msg *lim_msg,
	struct pe_session *pe_session)
{
	tpDphHashNode sta_ds = NULL;
	tpDeleteStaParams del_sta_params = (tpDeleteStaParams) lim_msg->bodyptr;
	tSirResultCodes status = eSIR_SME_SUCCESS;

	if (!del_sta_params) {
		pe_err("del_sta_params is NULL");
		return;
	}
	if (!LIM_IS_IBSS_ROLE(pe_session)) {
		pe_err("Session %d is not IBSS role", del_sta_params->assocId);
		status = eSIR_SME_REFUSED;
		goto skip_event;
	}

	sta_ds = dph_get_hash_entry(mac_ctx, del_sta_params->assocId,
			&pe_session->dph.dphHashTable);
	if (!sta_ds) {
		pe_err("DPH Entry for STA %X is missing",
			del_sta_params->assocId);
		status = eSIR_SME_REFUSED;
		goto skip_event;
	}

	if (QDF_STATUS_SUCCESS != del_sta_params->status) {
		pe_err("DEL STA failed!");
		status = eSIR_SME_REFUSED;
		goto skip_event;
	}
	pe_debug("Deleted STA associd %d staId %d MAC " QDF_MAC_ADDR_STR,
		sta_ds->assocId, sta_ds->staIndex,
		QDF_MAC_ADDR_ARRAY(sta_ds->staAddr));

	lim_delete_dph_hash_entry(mac_ctx, sta_ds->staAddr,
			  del_sta_params->assocId, pe_session);
	lim_release_peer_idx(mac_ctx,
			del_sta_params->assocId, pe_session);

	ibss_status_chg_notify(mac_ctx,
		del_sta_params->staMac,
		sta_ds->staIndex,
		eWNI_SME_IBSS_PEER_DEPARTED_IND,
		pe_session->smeSessionId);

	lim_ibss_search_and_delete_peer(mac_ctx,
				pe_session, del_sta_params->staMac);

skip_event:
	qdf_mem_free(del_sta_params);
	lim_msg->bodyptr = NULL;
}

/* handle the response from HAL for an ADD STA request */
QDF_STATUS
lim_ibss_add_sta_rsp(struct mac_context *mac, void *msg, struct pe_session *pe_session)
{
	tpDphHashNode sta;
	uint16_t peerIdx;
	tpAddStaParams pAddStaParams = (tpAddStaParams) msg;

	SET_LIM_PROCESS_DEFD_MESGS(mac, true);
	if (!pAddStaParams) {
		pe_err("IBSS: ADD_STA_RSP with no body!");
		return QDF_STATUS_E_FAILURE;
	}

	sta =
		dph_lookup_hash_entry(mac, pAddStaParams->staMac, &peerIdx,
				      &pe_session->dph.dphHashTable);
	if (!sta) {
		pe_err("IBSS: ADD_STA_RSP for unknown MAC addr: "QDF_MAC_ADDR_STR,
			QDF_MAC_ADDR_ARRAY(pAddStaParams->staMac));
		qdf_mem_free(pAddStaParams);
		return QDF_STATUS_E_FAILURE;
	}

	if (pAddStaParams->status != QDF_STATUS_SUCCESS) {
		pe_err("IBSS: ADD_STA_RSP error: %x for MAC:"QDF_MAC_ADDR_STR,
			pAddStaParams->status,
			QDF_MAC_ADDR_ARRAY(pAddStaParams->staMac));
		lim_ibss_delete_peer(mac,
			pe_session, pAddStaParams->staMac);
		qdf_mem_free(pAddStaParams);
		return QDF_STATUS_E_FAILURE;
	}

	sta->bssId = pAddStaParams->bss_idx;
	sta->staIndex = pAddStaParams->staIdx;
	sta->valid = 1;
	sta->mlmStaContext.mlmState = eLIM_MLM_LINK_ESTABLISHED_STATE;

	pe_debug("IBSS: sending IBSS_NEW_PEER msg to SME!");

	ibss_status_chg_notify(mac, pAddStaParams->staMac,
			       sta->staIndex,
			       eWNI_SME_IBSS_NEW_PEER_IND,
			       pe_session->smeSessionId);

	qdf_mem_free(pAddStaParams);

	return QDF_STATUS_SUCCESS;
}

void lim_ibss_del_bss_rsp_when_coalescing(struct mac_context *mac, void *msg,
					  struct pe_session *pe_session)
{
	tpDeleteBssParams pDelBss = (tpDeleteBssParams) msg;

	pe_debug("IBSS: DEL_BSS_RSP Rcvd during coalescing!");

	if (!pDelBss) {
		pe_err("IBSS: DEL_BSS_RSP(coalesce) with no body!");
		goto end;
	}

	if (pDelBss->status != QDF_STATUS_SUCCESS) {
		pe_err("IBSS: DEL_BSS_RSP(coalesce) error: %x Bss: %d",
			pDelBss->status, pDelBss->bss_idx);
		goto end;
	}

	/* Delete peer entries. */
	/* add the new bss */
	ibss_bss_add(mac, pe_session);
end:
	if (pDelBss)
		qdf_mem_free(pDelBss);
}

void lim_ibss_add_bss_rsp_when_coalescing(struct mac_context *mac, void *msg,
					  struct pe_session *pe_session)
{
	uint8_t infoLen;
	struct new_bss_info newBssInfo;
	tpAddBssParams pAddBss = msg;
	tpSirMacMgmtHdr pHdr = mac->lim.ibss_info.mac_hdr;
	tpSchBeaconStruct pBeacon = mac->lim.ibss_info.beacon;

	if ((!pHdr) || (!pBeacon)) {
		pe_err("Unable to handle AddBssRspWhenCoalescing (no cached BSS info)");
		goto end;
	}
	/* Inform Host of IBSS coalescing */
	infoLen = sizeof(tSirMacAddr) + sizeof(tSirMacChanNum) +
		  sizeof(uint8_t) + pBeacon->ssId.length + 1;

	qdf_mem_zero((void *)&newBssInfo, sizeof(newBssInfo));
	qdf_mem_copy(newBssInfo.bssId.bytes, pHdr->bssId, QDF_MAC_ADDR_SIZE);
	newBssInfo.channelNumber = (tSirMacChanNum) pAddBss->currentOperChannel;
	qdf_mem_copy((uint8_t *) &newBssInfo.ssId,
		     (uint8_t *) &pBeacon->ssId, pBeacon->ssId.length + 1);

	pe_debug("Sending JOINED_NEW_BSS notification to SME");

	lim_send_sme_wm_status_change_ntf(mac, eSIR_SME_JOINED_NEW_BSS,
					  (uint32_t *) &newBssInfo,
					  infoLen, pe_session->smeSessionId);
	{
		/* Configure beacon and send beacons to HAL */
		lim_send_beacon(mac, pe_session);
	}

end:
	ibss_coalesce_free(mac);
}

void lim_ibss_del_bss_rsp(struct mac_context *mac, void *msg, struct pe_session *pe_session)
{
	tSirResultCodes rc = eSIR_SME_SUCCESS;
	tpDeleteBssParams pDelBss = (tpDeleteBssParams) msg;
	tSirMacAddr nullBssid = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	SET_LIM_PROCESS_DEFD_MESGS(mac, true);
	if (!pDelBss) {
		pe_err("IBSS: DEL_BSS_RSP with no body!");
		rc = eSIR_SME_REFUSED;
		goto end;
	}

	pe_session = pe_find_session_by_session_id(mac, pDelBss->sessionId);
	if (!pe_session) {
		pe_err("Session Does not exist for given sessionID");
		goto end;
	}

	/*
	 * If delBss was issued as part of IBSS Coalescing, gLimIbssCoalescingHappened flag will be true.
	 * BSS has to be added again in this scenario, so this case needs to be handled separately.
	 * If delBss was issued as a result of trigger from SME_STOP_BSS Request, then limSme state changes to
	 * 'IDLE' and gLimIbssCoalescingHappened flag will be false. In this case STOP BSS RSP has to be sent to SME.
	 */
	if (true == mac->lim.gLimIbssCoalescingHappened) {

		lim_ibss_del_bss_rsp_when_coalescing(mac, msg, pe_session);
		return;
	}

	if (pDelBss->status != QDF_STATUS_SUCCESS) {
		pe_err("IBSS: DEL_BSS_RSP error: %x Bss: %d",
			       pDelBss->status, pDelBss->bss_idx);
		rc = eSIR_SME_STOP_BSS_FAILURE;
		goto end;
	}

	if (lim_set_link_state(mac, eSIR_LINK_IDLE_STATE, nullBssid,
			       pe_session->self_mac_addr, NULL,
			       NULL) != QDF_STATUS_SUCCESS) {
		pe_err("IBSS: DEL_BSS_RSP setLinkState failed");
		rc = eSIR_SME_REFUSED;
		goto end;
	}

	lim_ibss_delete(mac, pe_session);

	dph_hash_table_init(mac, &pe_session->dph.dphHashTable);
	lim_delete_pre_auth_list(mac);

	pe_session->limMlmState = eLIM_MLM_IDLE_STATE;

	MTRACE(mac_trace
		       (mac, TRACE_CODE_MLM_STATE, pe_session->peSessionId,
		       pe_session->limMlmState));

	pe_session->limSystemRole = eLIM_STA_ROLE;

	/* Change the short slot operating mode to Default (which is 1 for now) so that when IBSS starts next time with Libra
	 * as originator, it picks up the default. This enables us to remove hard coding of short slot = 1 from lim_apply_configuration
	 */
	pe_session->shortSlotTimeSupported =
		cfg_default(CFG_SHORT_SLOT_TIME_ENABLED);

end:
	if (pDelBss)
		qdf_mem_free(pDelBss);
	/* Delete PE session once BSS is deleted */
	if (pe_session) {
		lim_send_sme_rsp(mac, eWNI_SME_STOP_BSS_RSP, rc,
				 pe_session->smeSessionId);
		pe_delete_session(mac, pe_session);
		pe_session = NULL;
	}
}

static void lim_ibss_bss_delete(struct mac_context *mac,
				struct pe_session *pe_session)
{
	QDF_STATUS status;

	status = wlan_vdev_mlme_sm_deliver_evt(
				pe_session->vdev,
				WLAN_VDEV_SM_EV_DOWN,
				sizeof(*pe_session),
				pe_session);
	if (QDF_IS_STATUS_ERROR(status))
		pe_err("Deliver WLAN_VDEV_SM_EV_DOWN failed");
}

/**
 * lim_ibss_coalesce()
 *
 ***FUNCTION:
 * This function is called upon receiving Beacon/Probe Response
 * while operating in IBSS mode.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 *
 ***NOTE:
 *
 * @param  mac    - Pointer to Global MAC structure
 * @param  pBeacon - Parsed Beacon Frame structure
 * @param  pBD     - Pointer to received BD
 *
 * @return Status whether to process or ignore received Beacon Frame
 */

QDF_STATUS
lim_ibss_coalesce(struct mac_context *mac,
		  tpSirMacMgmtHdr pHdr,
		  tpSchBeaconStruct pBeacon,
		  uint8_t *pIEs,
		  uint32_t ieLen, uint16_t fTsfLater, struct pe_session *pe_session)
{
	uint16_t peerIdx;
	tSirMacAddr currentBssId;
	tLimIbssPeerNode *pPeerNode;
	tpDphHashNode sta;
	tUpdateBeaconParams beaconParams;

	qdf_mem_zero((uint8_t *) &beaconParams, sizeof(tUpdateBeaconParams));

	sir_copy_mac_addr(currentBssId, pe_session->bssId);

	pe_debug("Current BSSID :" QDF_MAC_ADDR_STR " Received BSSID :"
		   QDF_MAC_ADDR_STR, QDF_MAC_ADDR_ARRAY(currentBssId),
		QDF_MAC_ADDR_ARRAY(pHdr->bssId));

	/* Check for IBSS Coalescing only if Beacon is from different BSS */
	if (qdf_mem_cmp(currentBssId, pHdr->bssId, sizeof(tSirMacAddr))
	    && pe_session->isCoalesingInIBSSAllowed) {
		/*
		 * If STA entry is already available in the LIM hash table, then it is
		 * possible that the peer may have left and rejoined within the heartbeat
		 * timeout. In the offloaded case with 32 peers, the HB timeout is whopping
		 * 128 seconds. In that case, the FW will not let any frames come in until
		 * atleast the last sequence number is received before the peer is left
		 * Hence, if the coalescing peer is already there in the peer list and if
		 * the BSSID matches then, invoke delSta() to cleanup the entries. We will
		 * let the peer coalesce when we receive next beacon from the peer
		 */
		pPeerNode = ibss_peer_find(mac, pHdr->sa);
		if (pPeerNode) {
			lim_ibss_delete_peer(mac, pe_session,
							  pHdr->sa);
			pe_warn("Peer attempting to reconnect before HB timeout, deleted");
			return QDF_STATUS_E_INVAL;
		}

		if (!fTsfLater) { /* No Coalescing happened. */
			pe_warn("No Coalescing happened");
			return QDF_STATUS_E_INVAL;
		}
		/*
		 * IBSS Coalescing happened.
		 * save the received beacon, and delete the current BSS. The rest of the
		 * processing will be done in the delBss response processing
		 */
		mac->lim.gLimIbssCoalescingHappened = true;
		ibss_coalesce_save(mac, pHdr, pBeacon);
		pe_debug("IBSS Coalescing happened Delete BSSID :" QDF_MAC_ADDR_STR,
			QDF_MAC_ADDR_ARRAY(currentBssId));
		lim_ibss_bss_delete(mac, pe_session);

		return QDF_STATUS_SUCCESS;
	} else {
		if (qdf_mem_cmp
			    (currentBssId, pHdr->bssId, sizeof(tSirMacAddr)))
			return QDF_STATUS_E_INVAL;
	}

	/* STA in IBSS mode and SSID matches with ours */
	pPeerNode = ibss_peer_find(mac, pHdr->sa);
	if (!pPeerNode) {
		/* Peer not in the list - Collect BSS description & add to the list */
		uint32_t frameLen;
		QDF_STATUS retCode;

		/*
		 * Limit the Max number of IBSS Peers allowed as the max
		 * number of STA's allowed
		 * mac->lim.gLimNumIbssPeers will be increamented after exiting
		 * this function. so we will add additional 1 to compare against
		 * mac->lim.gLimIbssStaLimit
		 */
		if ((mac->lim.gLimNumIbssPeers + 1) >=
		    mac->lim.gLimIbssStaLimit) {
			/*Print every 100th time */
			if (mac->lim.ibss_retry_cnt % 100 == 0) {
				pe_debug("**** MAX STA LIMIT HAS REACHED ****");
			}
			mac->lim.ibss_retry_cnt++;
			return QDF_STATUS_E_NOSPC;
		}
		pe_debug("IBSS Peer node does not exist, adding it");
		frameLen =
			sizeof(tLimIbssPeerNode) + ieLen - sizeof(uint32_t);

		pPeerNode = qdf_mem_malloc((uint16_t) frameLen);
		if (!pPeerNode)
			return QDF_STATUS_E_NOMEM;

		pPeerNode->beacon = NULL;
		pPeerNode->beaconLen = 0;

		ibss_peer_collect(mac, pBeacon, pHdr, pPeerNode,
				  pe_session);
		pPeerNode->beacon = qdf_mem_malloc(ieLen);
		if (pPeerNode->beacon) {
			qdf_mem_copy(pPeerNode->beacon, pIEs, ieLen);
			pPeerNode->beaconLen = (uint16_t) ieLen;
		}
		ibss_peer_add(mac, pPeerNode);

		sta =
			dph_lookup_hash_entry(mac, pPeerNode->peerMacAddr, &peerIdx,
					      &pe_session->dph.dphHashTable);
		if (sta) {
			/* / DPH node already exists for the peer */
			pe_warn("DPH Node present for just learned peer");
			lim_print_mac_addr(mac, pPeerNode->peerMacAddr, LOGD);
			ibss_sta_info_update(mac, sta, pPeerNode,
					     pe_session);
			return QDF_STATUS_SUCCESS;
		}
		retCode =
			lim_ibss_sta_add(mac, pPeerNode->peerMacAddr, pe_session);
		if (retCode != QDF_STATUS_SUCCESS) {
			pe_err("lim-ibss-sta-add failed reason: %x", retCode);
			lim_print_mac_addr(mac, pPeerNode->peerMacAddr, LOGE);
			return retCode;
		}
		/* Decide protection mode */
		sta =
			dph_lookup_hash_entry(mac, pPeerNode->peerMacAddr, &peerIdx,
					      &pe_session->dph.dphHashTable);
		if (mac->lim.gLimProtectionControl !=
		    MLME_FORCE_POLICY_PROTECTION_DISABLE)
			lim_ibss_decide_protection(mac, sta, &beaconParams,
						   pe_session);

		if (beaconParams.paramChangeBitmap) {
			pe_err("beaconParams.paramChangeBitmap=1 ---> Update Beacon Params");
			sch_set_fixed_beacon_fields(mac, pe_session);
			beaconParams.bss_idx = pe_session->bss_idx;
			lim_send_beacon_params(mac, &beaconParams, pe_session);
		}
	} else
		ibss_sta_caps_update(mac, pPeerNode, pe_session);

	if (pe_session->limSmeState != eLIM_SME_NORMAL_STATE)
		return QDF_STATUS_SUCCESS;

	/* Received Beacon from same IBSS we're */
	/* currently part of. Inform Roaming algorithm */
	/* if not already that IBSS is active. */
	if (pe_session->limIbssActive == false) {
		limResetHBPktCount(pe_session);
		pe_warn("Partner joined our IBSS, Sending IBSS_ACTIVE Notification to SME");
		pe_session->limIbssActive = true;
		lim_send_sme_wm_status_change_ntf(mac, eSIR_SME_IBSS_ACTIVE, NULL, 0,
						  pe_session->smeSessionId);
	}

	return QDF_STATUS_SUCCESS;
} /*** end lim_handle_ibs_scoalescing() ***/

/**
 * lim_ibss_heart_beat_handle() - handle IBSS hearbeat failure
 *
 * @mac_ctx: global mac context
 * @session: PE session entry
 *
 * Hanlde IBSS hearbeat failure.
 *
 * Return: None.
 */
void lim_ibss_heart_beat_handle(struct mac_context *mac_ctx, struct pe_session *session)
{
	tLimIbssPeerNode *tempnode, *prevnode;
	tLimIbssPeerNode *temp_next = NULL;
	uint16_t peer_idx = 0;
	tpDphHashNode stads = 0;
	uint32_t threshold = 0;
	uint16_t sta_idx = 0;

	/*
	 * MLM BSS is started and if PE in scanmode then MLM state will be
	 * waiting for probe resp. If Heart beat timeout triggers during this
	 * corner case then we need to reactivate HeartBeat timer.
	 */
	if (session->limMlmState != eLIM_MLM_BSS_STARTED_STATE)
		return;

	/* If LinkMonitor is Disabled */
	if (!mac_ctx->sys.gSysEnableLinkMonitorMode)
		return;

	prevnode = tempnode = mac_ctx->lim.gLimIbssPeerList;
	threshold = (mac_ctx->lim.gLimNumIbssPeers / 4) + 1;

	/* Monitor the HeartBeat with the Individual PEERS in the IBSS */
	while (tempnode) {
		temp_next = tempnode->next;
		if (tempnode->beaconHBCount) {
			/* There was a beacon for this peer during heart beat */
			tempnode->beaconHBCount = 0;
			tempnode->heartbeatFailure = 0;
			prevnode = tempnode;
			tempnode = temp_next;
			continue;
		}

		/* There wasnt any beacon received during heartbeat timer. */
		tempnode->heartbeatFailure++;
		pe_err("Heartbeat fail: %d  thres: %d",
		    tempnode->heartbeatFailure, mac_ctx->lim.gLimNumIbssPeers);
		if (tempnode->heartbeatFailure >= threshold) {
			/* Remove this entry from the list. */
			stads = dph_lookup_hash_entry(mac_ctx,
					tempnode->peerMacAddr, &peer_idx,
					&session->dph.dphHashTable);
			if (stads) {
				sta_idx = stads->staIndex;

				(void)lim_del_sta(mac_ctx, stads, false,
						  session);
				lim_delete_dph_hash_entry(mac_ctx,
					stads->staAddr, peer_idx, session);
				lim_release_peer_idx(mac_ctx, peer_idx,
						     session);
				/* Send indication. */
				ibss_status_chg_notify(mac_ctx,
					tempnode->peerMacAddr, sta_idx,
					eWNI_SME_IBSS_PEER_DEPARTED_IND,
					session->smeSessionId);
			}
			if (tempnode == mac_ctx->lim.gLimIbssPeerList) {
				mac_ctx->lim.gLimIbssPeerList = tempnode->next;
				prevnode = mac_ctx->lim.gLimIbssPeerList;
			} else {
				prevnode->next = tempnode->next;
			}

			if (tempnode->beacon)
				qdf_mem_free(tempnode->beacon);
			qdf_mem_free(tempnode);
			mac_ctx->lim.gLimNumIbssPeers--;

			/* we deleted current node, so prevNode remains same. */
			tempnode = temp_next;
			continue;
		}
		prevnode = tempnode;
		tempnode = temp_next;
	}

	/*
	 * General IBSS Activity Monitor,
	 * check if in IBSS Mode we are received any Beacons
	 */
	if (mac_ctx->lim.gLimNumIbssPeers) {
		if (session->LimRxedBeaconCntDuringHB <
		    MAX_NO_BEACONS_PER_HEART_BEAT_INTERVAL)
			mac_ctx->lim.gLimHeartBeatBeaconStats[
				session->LimRxedBeaconCntDuringHB]++;
		else
			mac_ctx->lim.gLimHeartBeatBeaconStats[0]++;

		/* Reset number of beacons received */
		limResetHBPktCount(session);
		return;
	} else {
		pe_warn("Heartbeat Failure");
		mac_ctx->lim.gLimHBfailureCntInLinkEstState++;

		if (session->limIbssActive == true) {
			/*
			 * We don't receive Beacon frames from any
			 * other STA in IBSS. Announce IBSS inactive
			 * to Roaming algorithm
			 */
			pe_warn("Alone in IBSS");
			session->limIbssActive = false;

			lim_send_sme_wm_status_change_ntf(mac_ctx,
				eSIR_SME_IBSS_INACTIVE, NULL, 0,
				session->smeSessionId);
		}
	}
}

/**
 * lim_ibss_decide_protection_on_delete() - decides protection related info.
 *
 * @mac_ctx: global mac context
 * @stads: station hash node
 * @bcn_param: beacon parameters
 * @session: PE session entry
 *
 * Decides all the protection related information.
 *
 * Return: None
 */
void lim_ibss_decide_protection_on_delete(struct mac_context *mac_ctx,
					  tpDphHashNode stads,
					  tpUpdateBeaconParams bcn_param,
					  struct pe_session *session)
{
	uint32_t phymode;
	tHalBitVal erpenabled = eHAL_CLEAR;
	enum band_info rfband = BAND_UNKNOWN;
	uint32_t i;

	if (!stads)
		return;

	lim_get_rf_band_new(mac_ctx, &rfband, session);
	if (BAND_2G != rfband)
		return;

	lim_get_phy_mode(mac_ctx, &phymode, session);
	erpenabled = stads->erpEnabled;
	/* we are HT or 11G and 11B station is getting deleted. */
	if (((phymode == WNI_CFG_PHY_MODE_11G) ||
	     session->htCapability) && (erpenabled == eHAL_CLEAR)) {
		pe_err("%d A legacy STA is disassociated Addr is",
			session->gLim11bParams.numSta);
			lim_print_mac_addr(mac_ctx, stads->staAddr, LOGE);
		if (session->gLim11bParams.numSta == 0) {
			pe_err("No 11B STA exists. Disable protection");
			lim_ibss_set_protection(mac_ctx, false,
				bcn_param, session);
		}

		for (i = 0; i < LIM_PROT_STA_CACHE_SIZE; i++) {
			if (!mac_ctx->lim.protStaCache[i].active)
				continue;
			if (!qdf_mem_cmp(mac_ctx->lim.protStaCache[i].addr,
				stads->staAddr, sizeof(tSirMacAddr))) {
				session->gLim11bParams.numSta--;
				mac_ctx->lim.protStaCache[i].active = false;
				break;
			}
		}

	}
}

/**
 * __lim_ibss_peer_inactivity_handler() - Handle inactive IBSS peer
 * @mac: Global MAC context
 * @pe_session: PE session
 * @ind: IBSS peer inactivity indication
 *
 * Internal function. Deletes FW indicated peer which is inactive
 *
 * Return: None
 */
static void
__lim_ibss_peer_inactivity_handler(struct mac_context *mac,
				   struct pe_session *pe_session,
				   struct ibss_peer_inactivity_ind *ind)
{
	if (pe_session->limMlmState != eLIM_MLM_BSS_STARTED_STATE) {
		return;
	}

	/* delete the peer for which heartbeat is observed */
	lim_ibss_delete_peer(mac, pe_session, ind->peer_addr.bytes);
}

/** -------------------------------------------------------------
   \fn lim_process_ibss_peer_inactivity
   \brief Peer inactivity message handler
 \
   \param  struct mac_context *   mac
   \param  void*             buf
   \return None
   -------------------------------------------------------------*/
void lim_process_ibss_peer_inactivity(struct mac_context *mac, void *buf)
{
	/*
	 * --------------- HEARTBEAT OFFLOAD CASE ------------------
	 * This message handler is executed when the firmware identifies
	 * inactivity from one or more peer devices. We will come here
	 * for every inactive peer device
	 */
	uint8_t i;

	struct ibss_peer_inactivity_ind *peerInactivityInd = buf;

	/*
	 * If IBSS is not started or heartbeat offload is not enabled
	 * we should not handle this request
	 */
	if (eLIM_STA_IN_IBSS_ROLE != mac->lim.gLimSystemRole &&
	    !IS_IBSS_HEARTBEAT_OFFLOAD_FEATURE_ENABLE) {
		return;
	}

	/** If LinkMonitor is Disabled */
	if (!mac->sys.gSysEnableLinkMonitorMode) {
		return;
	}

	for (i = 0; i < mac->lim.maxBssId; i++) {
		if (true == mac->lim.gpSession[i].valid &&
		    eSIR_IBSS_MODE == mac->lim.gpSession[i].bssType) {
			__lim_ibss_peer_inactivity_handler(mac,
							   &mac->lim.gpSession[i],
							   peerInactivityInd);
			break;
		}
	}
}
