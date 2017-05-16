/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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

static tLimIbssPeerNode *ibss_peer_find(tpAniSirGlobal pMac,
					tSirMacAddr macAddr)
{
	tLimIbssPeerNode *pTempNode = pMac->lim.gLimIbssPeerList;

	while (pTempNode != NULL) {
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
 * @param  pMac      - Pointer to Global MAC structure
 * @param  pPeerNode - Pointer to peer node to be added to the list.
 *
 * @return None
 */

static tSirRetStatus
ibss_peer_add(tpAniSirGlobal pMac, tLimIbssPeerNode *pPeerNode)
{
#ifdef ANI_SIR_IBSS_PEER_CACHING
	uint32_t numIbssPeers = (2 * pMac->lim.maxStation);

	if (pMac->lim.gLimNumIbssPeers >= numIbssPeers) {
		/**
		 * Reached max number of peers to be maintained.
		 * Delete last entry & add new entry at the beginning.
		 */
		tLimIbssPeerNode *pTemp, *pPrev;
		pTemp = pPrev = pMac->lim.gLimIbssPeerList;
		while (pTemp->next != NULL) {
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
	pMac->lim.gLimNumIbssPeers++;

	pPeerNode->next = pMac->lim.gLimIbssPeerList;
	pMac->lim.gLimIbssPeerList = pPeerNode;

	return eSIR_SUCCESS;

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
 * @param  pMac    - Pointer to Global MAC structure
 * @param  pBeacon - Parsed Beacon Frame structure
 * @param  pBD     - Pointer to received BD
 * @param  pPeer   - Pointer to IBSS peer node
 *
 * @return None
 */

static void
ibss_peer_collect(tpAniSirGlobal pMac,
		  tpSchBeaconStruct pBeacon,
		  tpSirMacMgmtHdr pHdr,
		  tLimIbssPeerNode *pPeer, tpPESession psessionEntry)
{
	qdf_mem_copy(pPeer->peerMacAddr, pHdr->sa, sizeof(tSirMacAddr));

	pPeer->capabilityInfo = pBeacon->capabilityInfo;
	pPeer->extendedRatesPresent = pBeacon->extendedRatesPresent;
	pPeer->edcaPresent = pBeacon->edcaPresent;
	pPeer->wmeEdcaPresent = pBeacon->wmeEdcaPresent;
	pPeer->wmeInfoPresent = pBeacon->wmeInfoPresent;

	if (pBeacon->IBSSParams.present) {
		pPeer->atimIePresent = pBeacon->IBSSParams.present;
		pPeer->peerAtimWindowLength = pBeacon->IBSSParams.atim;
	}

	if (IS_DOT11_MODE_HT(psessionEntry->dot11mode) &&
	    (pBeacon->HTCaps.present)) {
		pPeer->htCapable = pBeacon->HTCaps.present;
		qdf_mem_copy((uint8_t *) pPeer->supportedMCSSet,
			     (uint8_t *) pBeacon->HTCaps.supportedMCSSet,
			     sizeof(pPeer->supportedMCSSet));
		pPeer->htGreenfield = (uint8_t) pBeacon->HTCaps.greenField;
		pPeer->htSupportedChannelWidthSet =
			(uint8_t) pBeacon->HTCaps.supportedChannelWidthSet;
		pPeer->htMIMOPSState =
			(tSirMacHTMIMOPowerSaveState) pBeacon->HTCaps.mimoPowerSave;
		pPeer->htMaxAmsduLength =
			(uint8_t) pBeacon->HTCaps.maximalAMSDUsize;
		pPeer->htAMpduDensity = pBeacon->HTCaps.mpduDensity;
		pPeer->htDsssCckRate40MHzSupport =
			(uint8_t) pBeacon->HTCaps.dsssCckMode40MHz;
		pPeer->htShortGI20Mhz = (uint8_t) pBeacon->HTCaps.shortGI20MHz;
		pPeer->htShortGI40Mhz = (uint8_t) pBeacon->HTCaps.shortGI40MHz;
		pPeer->htMaxRxAMpduFactor = pBeacon->HTCaps.maxRxAMPDUFactor;
		pPeer->htSecondaryChannelOffset =
			pBeacon->HTInfo.secondaryChannelOffset;
		pPeer->htLdpcCapable = (uint8_t) pBeacon->HTCaps.advCodingCap;
	}

	/* Collect peer VHT capabilities based on the received beacon from the peer */
	if (pBeacon->VHTCaps.present) {
		pPeer->vhtSupportedChannelWidthSet =
			pBeacon->VHTOperation.chanWidth;
		pPeer->vhtCapable = pBeacon->VHTCaps.present;

		/* Collect VHT capabilities from beacon */
		qdf_mem_copy((uint8_t *) &pPeer->VHTCaps,
			     (uint8_t *) &pBeacon->VHTCaps,
			     sizeof(tDot11fIEVHTCaps));
	}
	pPeer->erpIePresent = pBeacon->erpPresent;

	qdf_mem_copy((uint8_t *) &pPeer->supportedRates,
		     (uint8_t *) &pBeacon->supportedRates,
		     pBeacon->supportedRates.numRates + 1);
	if (pPeer->extendedRatesPresent)
		qdf_mem_copy((uint8_t *) &pPeer->extendedRates,
			     (uint8_t *) &pBeacon->extendedRates,
			     pBeacon->extendedRates.numRates + 1);
	else
		pPeer->extendedRates.numRates = 0;

	pPeer->next = NULL;
} /*** end ibss_peer_collect() ***/

/* handle change in peer qos/wme capabilities */
static void
ibss_sta_caps_update(tpAniSirGlobal pMac,
		     tLimIbssPeerNode *pPeerNode, tpPESession psessionEntry)
{
	uint16_t peerIdx;
	tpDphHashNode pStaDs;

	pPeerNode->beaconHBCount++;     /* Update beacon count. */

	/* if the peer node exists, update its qos capabilities */
	pStaDs = dph_lookup_hash_entry(pMac, pPeerNode->peerMacAddr, &peerIdx,
				       &psessionEntry->dph.dphHashTable);
	if (pStaDs == NULL)
		return;

	/* Update HT Capabilities */
	if (IS_DOT11_MODE_HT(psessionEntry->dot11mode)) {
		pStaDs->mlmStaContext.htCapability = pPeerNode->htCapable;
		if (pPeerNode->htCapable) {
			pStaDs->htGreenfield = pPeerNode->htGreenfield;
			pStaDs->htSupportedChannelWidthSet =
				pPeerNode->htSupportedChannelWidthSet;
			pStaDs->htSecondaryChannelOffset =
				pPeerNode->htSecondaryChannelOffset;
			pStaDs->htMIMOPSState = pPeerNode->htMIMOPSState;
			pStaDs->htMaxAmsduLength = pPeerNode->htMaxAmsduLength;
			pStaDs->htAMpduDensity = pPeerNode->htAMpduDensity;
			pStaDs->htDsssCckRate40MHzSupport =
				pPeerNode->htDsssCckRate40MHzSupport;
			pStaDs->htShortGI20Mhz = pPeerNode->htShortGI20Mhz;
			pStaDs->htShortGI40Mhz = pPeerNode->htShortGI40Mhz;
			pStaDs->htMaxRxAMpduFactor =
				pPeerNode->htMaxRxAMpduFactor;
			/* In the future, may need to check for "delayedBA" */
			/* For now, it is IMMEDIATE BA only on ALL TID's */
			pStaDs->baPolicyFlag = 0xFF;
			pStaDs->htLdpcCapable = pPeerNode->htLdpcCapable;
		}
	}
	if (IS_DOT11_MODE_VHT(psessionEntry->dot11mode)) {
		pStaDs->mlmStaContext.vhtCapability = pPeerNode->vhtCapable;
		if (pPeerNode->vhtCapable) {
			pStaDs->vhtSupportedChannelWidthSet =
				pPeerNode->vhtSupportedChannelWidthSet;

			/* If in 11AC mode and if session requires 11AC mode, consider peer's */
			/* max AMPDU length factor */
			pStaDs->htMaxRxAMpduFactor =
				pPeerNode->VHTCaps.maxAMPDULenExp;
			pStaDs->vhtLdpcCapable =
				(uint8_t) pPeerNode->VHTCaps.ldpcCodingCap;
		}
	}
	/* peer is 11e capable but is not 11e enabled yet */
	/* some STA's when joining Airgo IBSS, assert qos capability even when */
	/* they don't suport qos. however, they do not include the edca parameter */
	/* set. so let's check for edcaParam in addition to the qos capability */
	if (pPeerNode->capabilityInfo.qos && (psessionEntry->limQosEnabled)
	    && pPeerNode->edcaPresent) {
		pStaDs->qosMode = 1;
		pStaDs->wmeEnabled = 0;
		if (!pStaDs->lleEnabled) {
			pStaDs->lleEnabled = 1;
			/* dphSetACM(pMac, pStaDs); */
		}
		return;
	}
	/* peer is not 11e capable now but was 11e enabled earlier */
	else if (pStaDs->lleEnabled) {
		pStaDs->qosMode = 0;
		pStaDs->lleEnabled = 0;
	}
	/* peer is wme capable but is not wme enabled yet */
	if (pPeerNode->wmeInfoPresent && psessionEntry->limWmeEnabled) {
		pStaDs->qosMode = 1;
		pStaDs->lleEnabled = 0;
		if (!pStaDs->wmeEnabled) {
			pStaDs->wmeEnabled = 1;
		}
		return;
	}
	/* When the peer device supports EDCA parameters, then we were not
	   considering. Added this code when we saw that one of the Peer Device
	   was advertising WMM param where we were not honouring that. CR# 210756
	 */
	if (pPeerNode->wmeEdcaPresent && psessionEntry->limWmeEnabled) {
		pStaDs->qosMode = 1;
		pStaDs->lleEnabled = 0;
		if (!pStaDs->wmeEnabled) {
			pStaDs->wmeEnabled = 1;
		}
		return;
	}
	/* peer is not wme capable now but was wme enabled earlier */
	else if (pStaDs->wmeEnabled) {
		pStaDs->qosMode = 0;
		pStaDs->wmeEnabled = 0;
	}

}

static void
ibss_sta_rates_update(tpAniSirGlobal pMac,
		      tpDphHashNode pStaDs,
		      tLimIbssPeerNode *pPeer, tpPESession psessionEntry)
{
	lim_populate_matching_rate_set(pMac, pStaDs, &pPeer->supportedRates,
				       &pPeer->extendedRates,
				       pPeer->supportedMCSSet, psessionEntry,
				       &pPeer->VHTCaps, NULL);
	pStaDs->mlmStaContext.capabilityInfo = pPeer->capabilityInfo;
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
 * @param  pMac   - Pointer to Global MAC structure
 * @param  pStaDs - Pointer to DPH node
 * @param  pPeer  - Pointer to IBSS peer node
 *
 * @return None
 */

static void
ibss_sta_info_update(tpAniSirGlobal pMac,
		     tpDphHashNode pStaDs,
		     tLimIbssPeerNode *pPeer, tpPESession psessionEntry)
{
	pStaDs->staType = STA_ENTRY_PEER;
	ibss_sta_caps_update(pMac, pPeer, psessionEntry);
	ibss_sta_rates_update(pMac, pStaDs, pPeer, psessionEntry);
} /*** end ibss_sta_info_update() ***/

static void ibss_coalesce_free(tpAniSirGlobal pMac)
{
	if (pMac->lim.ibssInfo.pHdr != NULL)
		qdf_mem_free(pMac->lim.ibssInfo.pHdr);
	if (pMac->lim.ibssInfo.pBeacon != NULL)
		qdf_mem_free(pMac->lim.ibssInfo.pBeacon);

	pMac->lim.ibssInfo.pHdr = NULL;
	pMac->lim.ibssInfo.pBeacon = NULL;
}

/*
 * save the beacon params for use when adding the bss
 */
static void
ibss_coalesce_save(tpAniSirGlobal pMac,
		   tpSirMacMgmtHdr pHdr, tpSchBeaconStruct pBeacon)
{
	/* get rid of any saved info */
	ibss_coalesce_free(pMac);

	pMac->lim.ibssInfo.pHdr = qdf_mem_malloc(sizeof(*pHdr));
	if (NULL == pMac->lim.ibssInfo.pHdr) {
		pe_err("ibbs-save: Failed malloc pHdr");
		return;
	}
	pMac->lim.ibssInfo.pBeacon = qdf_mem_malloc(sizeof(*pBeacon));
	if (NULL == pMac->lim.ibssInfo.pBeacon) {
		pe_err("ibbs-save: Failed malloc pBeacon");
		ibss_coalesce_free(pMac);
		return;
	}

	qdf_mem_copy(pMac->lim.ibssInfo.pHdr, pHdr, sizeof(*pHdr));
	qdf_mem_copy(pMac->lim.ibssInfo.pBeacon, pBeacon, sizeof(*pBeacon));
}

/*
 * tries to add a new entry to dph hash node
 * if necessary, an existing entry is eliminated
 */
static tSirRetStatus
ibss_dph_entry_add(tpAniSirGlobal pMac,
		   tSirMacAddr peerAddr,
		   tpDphHashNode *ppSta, tpPESession psessionEntry)
{
	uint16_t peerIdx;
	tpDphHashNode pStaDs;

	*ppSta = NULL;

	pStaDs =
		dph_lookup_hash_entry(pMac, peerAddr, &peerIdx,
				      &psessionEntry->dph.dphHashTable);
	if (pStaDs != NULL) {
		/* Trying to add context for already existing STA in IBSS */
		pe_err("STA exists already");
		lim_print_mac_addr(pMac, peerAddr, LOGE);
		return eSIR_FAILURE;
	}

	/**
	 * Assign an AID, delete context existing with that
	 * AID and then add an entry to hash table maintained
	 * by DPH module.
	 */
	peerIdx = lim_assign_peer_idx(pMac, psessionEntry);

	pStaDs =
		dph_get_hash_entry(pMac, peerIdx, &psessionEntry->dph.dphHashTable);
	if (pStaDs) {
		(void)lim_del_sta(pMac, pStaDs, false /*asynchronous */,
				  psessionEntry);
		lim_delete_dph_hash_entry(pMac, pStaDs->staAddr, peerIdx,
					  psessionEntry);
	}

	pStaDs =
		dph_add_hash_entry(pMac, peerAddr, peerIdx,
				   &psessionEntry->dph.dphHashTable);
	if (pStaDs == NULL) {
		/* Could not add hash table entry */
		pe_err("could not add hash entry at DPH for peerIdx/aid: %d MACaddr:",
			       peerIdx);
		lim_print_mac_addr(pMac, peerAddr, LOGE);
		return eSIR_FAILURE;
	}

	*ppSta = pStaDs;
	return eSIR_SUCCESS;
}

/* send a status change notification */
static void
ibss_status_chg_notify(tpAniSirGlobal pMac,
		       tSirMacAddr peerAddr,
		       uint16_t staIndex,
		       uint8_t ucastSig,
		       uint8_t bcastSig, uint16_t status, uint8_t sessionId)
{

	tLimIbssPeerNode *peerNode;
	uint8_t *beacon = NULL;
	uint16_t bcnLen = 0;

	peerNode = ibss_peer_find(pMac, peerAddr);
	if (peerNode != NULL) {
		if (peerNode->beacon == NULL)
			peerNode->beaconLen = 0;
		beacon = peerNode->beacon;
		bcnLen = peerNode->beaconLen;
		peerNode->beacon = NULL;
		peerNode->beaconLen = 0;
	}

	lim_send_sme_ibss_peer_ind(pMac, peerAddr, staIndex, ucastSig, bcastSig,
				   beacon, bcnLen, status, sessionId);

	if (beacon != NULL) {
		qdf_mem_free(beacon);
	}
}

static void ibss_bss_add(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	tLimMlmStartReq mlmStartReq;
	uint32_t cfg;
	tpSirMacMgmtHdr pHdr = (tpSirMacMgmtHdr) pMac->lim.ibssInfo.pHdr;
	tpSchBeaconStruct pBeacon =
		(tpSchBeaconStruct) pMac->lim.ibssInfo.pBeacon;
	uint8_t numExtRates = 0;

	if ((pHdr == NULL) || (pBeacon == NULL)) {
		pe_err("Unable to add BSS (no cached BSS info)");
		return;
	}

	qdf_mem_copy(psessionEntry->bssId, pHdr->bssId, sizeof(tSirMacAddr));

	sir_copy_mac_addr(pHdr->bssId, psessionEntry->bssId);

	/* Copy beacon interval from sessionTable */
	cfg = psessionEntry->beaconParams.beaconInterval;
	if (cfg != pBeacon->beaconInterval)
		psessionEntry->beaconParams.beaconInterval =
			pBeacon->beaconInterval;

	/* This function ibss_bss_add (and hence the below code) is only called during ibss coalescing. We need to
	 * adapt to peer's capability with respect to short slot time. Changes have been made to lim_apply_configuration()
	 * so that the IBSS doesnt blindly start with short slot = 1. If IBSS start is part of coalescing then it will adapt
	 * to peer's short slot using code below.
	 */
	/* If cfg is already set to current peer's capability then no need to set it again */
	if (psessionEntry->shortSlotTimeSupported !=
	    pBeacon->capabilityInfo.shortSlotTime) {
		psessionEntry->shortSlotTimeSupported =
			pBeacon->capabilityInfo.shortSlotTime;
	}
	qdf_mem_copy((uint8_t *) &psessionEntry->pLimStartBssReq->
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
		numExtRates = pBeacon->extendedRates.numRates;
	if (cfg_set_str(pMac, WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET,
			(uint8_t *) &pBeacon->extendedRates.rate,
			numExtRates) != eSIR_SUCCESS) {
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
	qdf_mem_set((void *)&mlmStartReq, sizeof(mlmStartReq), 0);

	qdf_mem_copy(mlmStartReq.bssId, pHdr->bssId, sizeof(tSirMacAddr));
	mlmStartReq.rateSet.numRates =
		psessionEntry->pLimStartBssReq->operationalRateSet.numRates;
	qdf_mem_copy(&mlmStartReq.rateSet.rate[0],
		     &psessionEntry->pLimStartBssReq->operationalRateSet.
		     rate[0], mlmStartReq.rateSet.numRates);
	mlmStartReq.bssType = eSIR_IBSS_MODE;
	mlmStartReq.beaconPeriod = pBeacon->beaconInterval;
	mlmStartReq.nwType = psessionEntry->pLimStartBssReq->nwType;    /* psessionEntry->nwType is also OK???? */
	mlmStartReq.htCapable = psessionEntry->htCapability;
	mlmStartReq.htOperMode = pMac->lim.gHTOperMode;
	mlmStartReq.dualCTSProtection = pMac->lim.gHTDualCTSProtection;
	mlmStartReq.txChannelWidthSet = psessionEntry->htRecommendedTxWidthSet;

	/* reading the channel num from session Table */
	mlmStartReq.channelNumber = psessionEntry->currentOperChannel;

	mlmStartReq.cbMode = psessionEntry->pLimStartBssReq->cbMode;

	/* Copy the SSID for RxP filtering based on SSID. */
	qdf_mem_copy((uint8_t *) &mlmStartReq.ssId,
		     (uint8_t *) &psessionEntry->pLimStartBssReq->ssId,
		     psessionEntry->pLimStartBssReq->ssId.length + 1);

	pe_debug("invoking ADD_BSS as part of coalescing!");
	if (lim_mlm_add_bss(pMac, &mlmStartReq, psessionEntry) !=
	    eSIR_SME_SUCCESS) {
		pe_err("AddBss failure");
		return;
	}
	/* Update fields in Beacon */
	if (sch_set_fixed_beacon_fields(pMac, psessionEntry) != eSIR_SUCCESS) {
		pe_err("Unable to set fixed Beacon fields");
		return;
	}

}

/* delete the current BSS */
static void ibss_bss_delete(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	tSirRetStatus status;
	pe_debug("Initiating IBSS Delete BSS");
	if (psessionEntry->limMlmState != eLIM_MLM_BSS_STARTED_STATE) {
		pe_warn("Incorrect LIM MLM state for delBss: %d",
			psessionEntry->limMlmState);
		return;
	}
	status = lim_del_bss(pMac, NULL, psessionEntry->bssIdx, psessionEntry);
	if (status != eSIR_SUCCESS)
		pe_err("delBss failed for bss: %d",
			       psessionEntry->bssIdx);
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
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

void lim_ibss_init(tpAniSirGlobal pMac)
{
	pMac->lim.gLimIbssCoalescingHappened = 0;
	pMac->lim.gLimIbssPeerList = NULL;
	pMac->lim.gLimNumIbssPeers = 0;

	/* ibss info - params for which ibss to join while coalescing */
	qdf_mem_set(&pMac->lim.ibssInfo, sizeof(tAniSirLimIbss), 0);
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
 * @param  pMac - Pointer to Global MAC structure
 * @return None
 */

static void lim_ibss_delete_all_peers(tpAniSirGlobal pMac,
				      tpPESession psessionEntry)
{
	tLimIbssPeerNode *pCurrNode, *pTempNode;
	tpDphHashNode pStaDs;
	uint16_t peerIdx;

	pCurrNode = pTempNode = pMac->lim.gLimIbssPeerList;

	while (pCurrNode != NULL) {
		if (!pMac->lim.gLimNumIbssPeers) {
			pe_err("Number of peers in the list is zero and node present");
			return;
		}
		/* Delete the dph entry for the station
		 * Since it is called to remove all peers, just delete from dph,
		 * no need to do any beacon related params i.e., dont call lim_delete_dph_hash_entry
		 */
		pStaDs =
			dph_lookup_hash_entry(pMac, pCurrNode->peerMacAddr, &peerIdx,
					      &psessionEntry->dph.dphHashTable);
		if (pStaDs) {

			ibss_status_chg_notify(pMac, pCurrNode->peerMacAddr,
					       pStaDs->staIndex,
					       pStaDs->ucUcastSig,
					       pStaDs->ucBcastSig,
					       eWNI_SME_IBSS_PEER_DEPARTED_IND,
					       psessionEntry->smeSessionId);
			lim_release_peer_idx(pMac, peerIdx, psessionEntry);
			dph_delete_hash_entry(pMac, pStaDs->staAddr, peerIdx,
					      &psessionEntry->dph.dphHashTable);
		}

		pTempNode = pCurrNode->next;

		/* TODO :Sessionize this code */
		/* Fix CR 227642: PeerList should point to the next node since the current node is being
		 * freed in the next line. In ibss_peerfind in ibss_status_chg_notify above, we use this
		 * peer list to find the next peer. So this list needs to be updated with the no of peers left
		 * after each iteration in this while loop since one by one peers are deleted (freed) in this
		 * loop causing the lim.gLimIbssPeerList to point to some freed memory.
		 */
		pMac->lim.gLimIbssPeerList = pTempNode;

		if (pCurrNode->beacon) {
			qdf_mem_free(pCurrNode->beacon);
		}
		qdf_mem_free(pCurrNode);
		if (pMac->lim.gLimNumIbssPeers > 0) /* be paranoid */
			pMac->lim.gLimNumIbssPeers--;
		pCurrNode = pTempNode;
	}

	if (pMac->lim.gLimNumIbssPeers)
		pe_err("Number of peers: %d in the list is non-zero",
			pMac->lim.gLimNumIbssPeers);

	pMac->lim.gLimNumIbssPeers = 0;
	pMac->lim.gLimIbssPeerList = NULL;

}

/**
 * lim_ibss_delete() - This function is called while tearing down an IBSS
 *
 * @pMac: Pointer to Global MAC structure
 * @psessionEntry: Pointer to session entry
 *
 * Return: none
 */

void lim_ibss_delete(tpAniSirGlobal pMac, tpPESession psessionEntry)
{
	lim_ibss_delete_all_peers(pMac, psessionEntry);
	ibss_coalesce_free(pMac);
}

/** -------------------------------------------------------------
   \fn lim_ibss_set_protection
   \brief Decides all the protection related information.
 \
   \param  tpAniSirGlobal    pMac
   \param  tSirMacAddr peerMacAddr
   \param  tpUpdateBeaconParams pBeaconParams
   \return None
   -------------------------------------------------------------*/
static void
lim_ibss_set_protection(tpAniSirGlobal pMac, uint8_t enable,
			tpUpdateBeaconParams pBeaconParams,
			tpPESession psessionEntry)
{

	if (!pMac->lim.cfgProtection.fromllb) {
		pe_err("protection from 11b is disabled");
		return;
	}

	if (enable) {
		psessionEntry->gLim11bParams.protectionEnabled = true;
		if (false ==
		    psessionEntry->beaconParams.
		    llbCoexist /*pMac->lim.llbCoexist */) {
			pe_debug("=> IBSS: Enable Protection");
			pBeaconParams->llbCoexist =
				psessionEntry->beaconParams.llbCoexist = true;
			pBeaconParams->paramChangeBitmap |=
				PARAM_llBCOEXIST_CHANGED;
		}
	} else if (true ==
		   psessionEntry->beaconParams.
		   llbCoexist /*pMac->lim.llbCoexist */) {
		psessionEntry->gLim11bParams.protectionEnabled = false;
		pe_debug("===> IBSS: Disable protection");
		pBeaconParams->llbCoexist =
			psessionEntry->beaconParams.llbCoexist = false;
		pBeaconParams->paramChangeBitmap |= PARAM_llBCOEXIST_CHANGED;
	}
	return;
}

/** -------------------------------------------------------------
   \fn lim_ibss_update_protection_params
   \brief Decides all the protection related information.
 \
   \param  tpAniSirGlobal    pMac
   \param  tSirMacAddr peerMacAddr
   \param  tpUpdateBeaconParams pBeaconParams
   \return None
   -------------------------------------------------------------*/
static void
lim_ibss_update_protection_params(tpAniSirGlobal pMac,
				  tSirMacAddr peerMacAddr,
				  tLimProtStaCacheType protStaCacheType,
				  tpPESession psessionEntry)
{
	uint32_t i;

	pe_debug("STA is associated Addr :");
	lim_print_mac_addr(pMac, peerMacAddr, LOGD);

	for (i = 0; i < LIM_PROT_STA_CACHE_SIZE; i++) {
		if (pMac->lim.protStaCache[i].active) {
			pe_debug("Addr:");
			lim_print_mac_addr
				(pMac, pMac->lim.protStaCache[i].addr, LOGD);

			if (!qdf_mem_cmp(pMac->lim.protStaCache[i].addr,
					    peerMacAddr,
					    sizeof(tSirMacAddr))) {
				pe_debug("matching cache entry at: %d already active",
					i);
				return;
			}
		}
	}

	for (i = 0; i < LIM_PROT_STA_CACHE_SIZE; i++) {
		if (!pMac->lim.protStaCache[i].active)
			break;
	}

	if (i >= LIM_PROT_STA_CACHE_SIZE) {
		pe_err("No space in ProtStaCache");
		return;
	}

	qdf_mem_copy(pMac->lim.protStaCache[i].addr,
		     peerMacAddr, sizeof(tSirMacAddr));

	pMac->lim.protStaCache[i].protStaCacheType = protStaCacheType;
	pMac->lim.protStaCache[i].active = true;
	if (eLIM_PROT_STA_CACHE_TYPE_llB == protStaCacheType) {
		psessionEntry->gLim11bParams.numSta++;
	} else if (eLIM_PROT_STA_CACHE_TYPE_llG == protStaCacheType) {
		psessionEntry->gLim11gParams.numSta++;
	}
}

/** -------------------------------------------------------------
   \fn lim_ibss_decide_protection
   \brief Decides all the protection related information.
 \
   \param  tpAniSirGlobal    pMac
   \param  tSirMacAddr peerMacAddr
   \param  tpUpdateBeaconParams pBeaconParams
   \return None
   -------------------------------------------------------------*/
static void
lim_ibss_decide_protection(tpAniSirGlobal pMac, tpDphHashNode pStaDs,
			   tpUpdateBeaconParams pBeaconParams,
			   tpPESession psessionEntry)
{
	tSirRFBand rfBand = SIR_BAND_UNKNOWN;
	uint32_t phyMode;
	tLimProtStaCacheType protStaCacheType =
		eLIM_PROT_STA_CACHE_TYPE_INVALID;

	pBeaconParams->paramChangeBitmap = 0;

	if (NULL == pStaDs) {
		pe_err("pStaDs is NULL");
		return;
	}

	lim_get_rf_band_new(pMac, &rfBand, psessionEntry);
	if (SIR_BAND_2_4_GHZ == rfBand) {
		lim_get_phy_mode(pMac, &phyMode, psessionEntry);

		/* We are 11G or 11n. Check if we need protection from 11b Stations. */
		if ((phyMode == WNI_CFG_PHY_MODE_11G)
		    || (psessionEntry->htCapability)) {
			/* As we found in the past, it is possible that a 11n STA sends
			 * Beacon with HT IE but not ERP IE.  So the absense of ERP IE
			 * in the Beacon is not enough to conclude that STA is 11b.
			 */
			if ((pStaDs->erpEnabled == eHAL_CLEAR) &&
			    (!pStaDs->mlmStaContext.htCapability)) {
				protStaCacheType = eLIM_PROT_STA_CACHE_TYPE_llB;
				pe_err("Enable protection from 11B");
				lim_ibss_set_protection(pMac, true,
							pBeaconParams,
							psessionEntry);
			}
		}
	}
	lim_ibss_update_protection_params(pMac, pStaDs->staAddr, protStaCacheType,
					  psessionEntry);
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
tLimIbssPeerNode *lim_ibss_peer_find(tpAniSirGlobal pMac, tSirMacAddr macAddr)
{
	return ibss_peer_find(pMac, macAddr);
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
 * @param  pMac       Pointer to Global MAC structure
 * @param  peerAdddr  MAC address of the peer being added
 * @return retCode    Indicates success or failure return code
 * @return
 */

tSirRetStatus
lim_ibss_sta_add(tpAniSirGlobal pMac, void *pBody, tpPESession psessionEntry)
{
	tSirRetStatus retCode = eSIR_SUCCESS;
	tpDphHashNode pStaDs;
	tLimIbssPeerNode *pPeerNode;
	tLimMlmStates prevState;
	tSirMacAddr *pPeerAddr = (tSirMacAddr *) pBody;
	tUpdateBeaconParams beaconParams;

	qdf_mem_set((uint8_t *) &beaconParams, sizeof(tUpdateBeaconParams), 0);

	if (pBody == 0) {
		pe_err("Invalid IBSS AddSta");
		return eSIR_FAILURE;
	}

	pe_debug("Rx Add-Ibss-Sta for MAC:");
	lim_print_mac_addr(pMac, *pPeerAddr, LOGD);

	pPeerNode = ibss_peer_find(pMac, *pPeerAddr);
	if (NULL != pPeerNode) {
		retCode =
			ibss_dph_entry_add(pMac, *pPeerAddr, &pStaDs,
					   psessionEntry);
		if (eSIR_SUCCESS == retCode) {
			prevState = pStaDs->mlmStaContext.mlmState;
			pStaDs->erpEnabled = pPeerNode->erpIePresent;

			ibss_sta_info_update(pMac, pStaDs, pPeerNode,
					     psessionEntry);
			pe_debug("initiating ADD STA for the IBSS peer");
			retCode =
				lim_add_sta(pMac, pStaDs, false, psessionEntry);
			if (retCode != eSIR_SUCCESS) {
				pe_err("ibss-sta-add failed (reason %x)",
					       retCode);
				lim_print_mac_addr(pMac, *pPeerAddr, LOGE);
				pStaDs->mlmStaContext.mlmState = prevState;
				dph_delete_hash_entry(pMac, pStaDs->staAddr,
						      pStaDs->assocId,
						      &psessionEntry->dph.
						      dphHashTable);
			} else {
				if (pMac->lim.gLimProtectionControl !=
				    WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
					lim_ibss_decide_protection(pMac, pStaDs,
								   &beaconParams,
								   psessionEntry);

				if (beaconParams.paramChangeBitmap) {
					pe_debug("---> Update Beacon Params");
					sch_set_fixed_beacon_fields(pMac,
								    psessionEntry);
					beaconParams.bssIdx =
						psessionEntry->bssIdx;
					lim_send_beacon_params(pMac, &beaconParams,
							       psessionEntry);
				}
			}
		} else {
			pe_err("hashTblAdd failed reason: %x", retCode);
			lim_print_mac_addr(pMac, *pPeerAddr, LOGE);
		}
	} else {
		retCode = eSIR_FAILURE;
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
lim_ibss_search_and_delete_peer(tpAniSirGlobal mac_ctx,
			tpPESession session_entry, tSirMacAddr mac_addr)
{
	tLimIbssPeerNode *temp_node, *prev_node;
	tLimIbssPeerNode *temp_next_node = NULL;

	prev_node = temp_node = mac_ctx->lim.gLimIbssPeerList;

	pe_debug(" PEER ADDR :" MAC_ADDRESS_STR,
		MAC_ADDR_ARRAY(mac_addr));

	/** Compare Peer */
	while (NULL != temp_node) {
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
lim_ibss_delete_peer(tpAniSirGlobal mac_ctx,
			tpPESession session_entry, tSirMacAddr mac_addr)
{
	tpDphHashNode sta = NULL;
	uint16_t peer_idx = 0;

	pe_debug("Delete peer :" MAC_ADDRESS_STR,
		MAC_ADDR_ARRAY(mac_addr));

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

void lim_process_ibss_del_sta_rsp(tpAniSirGlobal mac_ctx,
	struct scheduler_msg *lim_msg,
	tpPESession pe_session)
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
	pe_debug("Deleted STA associd %d staId %d MAC " MAC_ADDRESS_STR,
		sta_ds->assocId, sta_ds->staIndex,
		MAC_ADDR_ARRAY(sta_ds->staAddr));

	lim_delete_dph_hash_entry(mac_ctx, sta_ds->staAddr,
			  del_sta_params->assocId, pe_session);
	lim_release_peer_idx(mac_ctx,
			del_sta_params->assocId, pe_session);

	ibss_status_chg_notify(mac_ctx,
		del_sta_params->staMac,
		sta_ds->staIndex,
		sta_ds->ucUcastSig, sta_ds->ucBcastSig,
		eWNI_SME_IBSS_PEER_DEPARTED_IND,
		pe_session->smeSessionId);

	lim_ibss_search_and_delete_peer(mac_ctx,
				pe_session, del_sta_params->staMac);

skip_event:
	qdf_mem_free(del_sta_params);
	lim_msg->bodyptr = NULL;
}

/* handle the response from HAL for an ADD STA request */
tSirRetStatus
lim_ibss_add_sta_rsp(tpAniSirGlobal pMac, void *msg, tpPESession psessionEntry)
{
	tpDphHashNode pStaDs;
	uint16_t peerIdx;
	tpAddStaParams pAddStaParams = (tpAddStaParams) msg;

	SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
	if (pAddStaParams == NULL) {
		pe_err("IBSS: ADD_STA_RSP with no body!");
		return eSIR_FAILURE;
	}

	pStaDs =
		dph_lookup_hash_entry(pMac, pAddStaParams->staMac, &peerIdx,
				      &psessionEntry->dph.dphHashTable);
	if (pStaDs == NULL) {
		pe_err("IBSS: ADD_STA_RSP for unknown MAC addr: "MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(pAddStaParams->staMac));
		qdf_mem_free(pAddStaParams);
		return eSIR_FAILURE;
	}

	if (pAddStaParams->status != QDF_STATUS_SUCCESS) {
		pe_err("IBSS: ADD_STA_RSP error: %x for MAC:"MAC_ADDRESS_STR,
			pAddStaParams->status,
			MAC_ADDR_ARRAY(pAddStaParams->staMac));
		lim_ibss_delete_peer(pMac,
			psessionEntry, pAddStaParams->staMac);
		qdf_mem_free(pAddStaParams);
		return eSIR_FAILURE;
	}

	pStaDs->bssId = pAddStaParams->bssIdx;
	pStaDs->staIndex = pAddStaParams->staIdx;
	pStaDs->ucUcastSig = pAddStaParams->ucUcastSig;
	pStaDs->ucBcastSig = pAddStaParams->ucBcastSig;
	pStaDs->valid = 1;
	pStaDs->mlmStaContext.mlmState = eLIM_MLM_LINK_ESTABLISHED_STATE;

	pe_debug("IBSS: sending IBSS_NEW_PEER msg to SME!");

	ibss_status_chg_notify(pMac, pAddStaParams->staMac,
			       pStaDs->staIndex, pStaDs->ucUcastSig,
			       pStaDs->ucBcastSig,
			       eWNI_SME_IBSS_NEW_PEER_IND,
			       psessionEntry->smeSessionId);

	qdf_mem_free(pAddStaParams);

	return eSIR_SUCCESS;
}

void lim_ibss_del_bss_rsp_when_coalescing(tpAniSirGlobal pMac, void *msg,
					  tpPESession psessionEntry)
{
	tpDeleteBssParams pDelBss = (tpDeleteBssParams) msg;

	pe_debug("IBSS: DEL_BSS_RSP Rcvd during coalescing!");

	if (pDelBss == NULL) {
		pe_err("IBSS: DEL_BSS_RSP(coalesce) with no body!");
		goto end;
	}

	if (pDelBss->status != QDF_STATUS_SUCCESS) {
		pe_err("IBSS: DEL_BSS_RSP(coalesce) error: %x Bss: %d",
			pDelBss->status, pDelBss->bssIdx);
		goto end;
	}
	/* Delete peer entries. */
	lim_ibss_delete_all_peers(pMac, psessionEntry);

	/* add the new bss */
	ibss_bss_add(pMac, psessionEntry);

end:
	if (pDelBss != NULL)
		qdf_mem_free(pDelBss);
}

void lim_ibss_add_bss_rsp_when_coalescing(tpAniSirGlobal pMac, void *msg,
					  tpPESession pSessionEntry)
{
	uint8_t infoLen;
	tSirSmeNewBssInfo newBssInfo;

	tpAddBssParams pAddBss = (tpAddBssParams) msg;

	tpSirMacMgmtHdr pHdr = (tpSirMacMgmtHdr) pMac->lim.ibssInfo.pHdr;
	tpSchBeaconStruct pBeacon =
		(tpSchBeaconStruct) pMac->lim.ibssInfo.pBeacon;

	if ((pHdr == NULL) || (pBeacon == NULL)) {
		pe_err("Unable to handle AddBssRspWhenCoalescing (no cached BSS info)");
		goto end;
	}
	/* Inform Host of IBSS coalescing */
	infoLen = sizeof(tSirMacAddr) + sizeof(tSirMacChanNum) +
		  sizeof(uint8_t) + pBeacon->ssId.length + 1;

	qdf_mem_set((void *)&newBssInfo, sizeof(newBssInfo), 0);
	qdf_mem_copy(newBssInfo.bssId.bytes, pHdr->bssId, QDF_MAC_ADDR_SIZE);
	newBssInfo.channelNumber = (tSirMacChanNum) pAddBss->currentOperChannel;
	qdf_mem_copy((uint8_t *) &newBssInfo.ssId,
		     (uint8_t *) &pBeacon->ssId, pBeacon->ssId.length + 1);

	pe_debug("Sending JOINED_NEW_BSS notification to SME");

	lim_send_sme_wm_status_change_ntf(pMac, eSIR_SME_JOINED_NEW_BSS,
					  (uint32_t *) &newBssInfo,
					  infoLen, pSessionEntry->smeSessionId);
	{
		/* Configure beacon and send beacons to HAL */
		lim_send_beacon_ind(pMac, pSessionEntry);
	}

end:
	ibss_coalesce_free(pMac);
}

void lim_ibss_del_bss_rsp(tpAniSirGlobal pMac, void *msg, tpPESession psessionEntry)
{
	tSirResultCodes rc = eSIR_SME_SUCCESS;
	tpDeleteBssParams pDelBss = (tpDeleteBssParams) msg;
	tSirMacAddr nullBssid = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
	if (pDelBss == NULL) {
		pe_err("IBSS: DEL_BSS_RSP with no body!");
		rc = eSIR_SME_REFUSED;
		goto end;
	}

	psessionEntry = pe_find_session_by_session_id(pMac, pDelBss->sessionId);
	if (psessionEntry == NULL) {
		pe_err("Session Does not exist for given sessionID");
		goto end;
	}

	/*
	 * If delBss was issued as part of IBSS Coalescing, gLimIbssCoalescingHappened flag will be true.
	 * BSS has to be added again in this scenario, so this case needs to be handled separately.
	 * If delBss was issued as a result of trigger from SME_STOP_BSS Request, then limSme state changes to
	 * 'IDLE' and gLimIbssCoalescingHappened flag will be false. In this case STOP BSS RSP has to be sent to SME.
	 */
	if (true == pMac->lim.gLimIbssCoalescingHappened) {

		lim_ibss_del_bss_rsp_when_coalescing(pMac, msg, psessionEntry);
		return;
	}

	if (pDelBss->status != QDF_STATUS_SUCCESS) {
		pe_err("IBSS: DEL_BSS_RSP error: %x Bss: %d",
			       pDelBss->status, pDelBss->bssIdx);
		rc = eSIR_SME_STOP_BSS_FAILURE;
		goto end;
	}

	if (lim_set_link_state(pMac, eSIR_LINK_IDLE_STATE, nullBssid,
			       psessionEntry->selfMacAddr, NULL,
			       NULL) != eSIR_SUCCESS) {
		pe_err("IBSS: DEL_BSS_RSP setLinkState failed");
		rc = eSIR_SME_REFUSED;
		goto end;
	}

	lim_ibss_delete(pMac, psessionEntry);

	dph_hash_table_class_init(pMac, &psessionEntry->dph.dphHashTable);
	lim_delete_pre_auth_list(pMac);

	psessionEntry->limMlmState = eLIM_MLM_IDLE_STATE;

	MTRACE(mac_trace
		       (pMac, TRACE_CODE_MLM_STATE, psessionEntry->peSessionId,
		       psessionEntry->limMlmState));

	psessionEntry->limSystemRole = eLIM_STA_ROLE;

	/* Change the short slot operating mode to Default (which is 1 for now) so that when IBSS starts next time with Libra
	 * as originator, it picks up the default. This enables us to remove hard coding of short slot = 1 from lim_apply_configuration
	 */
	psessionEntry->shortSlotTimeSupported = WNI_CFG_SHORT_SLOT_TIME_STADEF;

end:
	if (pDelBss != NULL)
		qdf_mem_free(pDelBss);
	/* Delete PE session once BSS is deleted */
	if (NULL != psessionEntry) {
		lim_send_sme_rsp(pMac, eWNI_SME_STOP_BSS_RSP, rc,
				 psessionEntry->smeSessionId,
				 psessionEntry->transactionId);
		pe_delete_session(pMac, psessionEntry);
		psessionEntry = NULL;
	}
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
 * @param  pMac    - Pointer to Global MAC structure
 * @param  pBeacon - Parsed Beacon Frame structure
 * @param  pBD     - Pointer to received BD
 *
 * @return Status whether to process or ignore received Beacon Frame
 */

tSirRetStatus
lim_ibss_coalesce(tpAniSirGlobal pMac,
		  tpSirMacMgmtHdr pHdr,
		  tpSchBeaconStruct pBeacon,
		  uint8_t *pIEs,
		  uint32_t ieLen, uint16_t fTsfLater, tpPESession psessionEntry)
{
	uint16_t peerIdx;
	tSirMacAddr currentBssId;
	tLimIbssPeerNode *pPeerNode;
	tpDphHashNode pStaDs;
	tUpdateBeaconParams beaconParams;

	qdf_mem_set((uint8_t *) &beaconParams, sizeof(tUpdateBeaconParams), 0);

	sir_copy_mac_addr(currentBssId, psessionEntry->bssId);

	pe_debug("Current BSSID :" MAC_ADDRESS_STR " Received BSSID :"
		   MAC_ADDRESS_STR, MAC_ADDR_ARRAY(currentBssId),
		MAC_ADDR_ARRAY(pHdr->bssId));

	/* Check for IBSS Coalescing only if Beacon is from different BSS */
	if (qdf_mem_cmp(currentBssId, pHdr->bssId, sizeof(tSirMacAddr))
	    && psessionEntry->isCoalesingInIBSSAllowed) {
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
		pPeerNode = ibss_peer_find(pMac, pHdr->sa);
		if (NULL != pPeerNode) {
			lim_ibss_delete_peer(pMac, psessionEntry,
							  pHdr->sa);
			pe_warn("Peer attempting to reconnect before HB timeout, deleted");
			return eSIR_LIM_IGNORE_BEACON;
		}

		if (!fTsfLater) { /* No Coalescing happened. */
			pe_warn("No Coalescing happened");
			return eSIR_LIM_IGNORE_BEACON;
		}
		/*
		 * IBSS Coalescing happened.
		 * save the received beacon, and delete the current BSS. The rest of the
		 * processing will be done in the delBss response processing
		 */
		pMac->lim.gLimIbssCoalescingHappened = true;
		ibss_coalesce_save(pMac, pHdr, pBeacon);
		pe_debug("IBSS Coalescing happened Delete BSSID :" MAC_ADDRESS_STR,
			MAC_ADDR_ARRAY(currentBssId));
		ibss_bss_delete(pMac, psessionEntry);
		return eSIR_SUCCESS;
	} else {
		if (qdf_mem_cmp
			    (currentBssId, pHdr->bssId, sizeof(tSirMacAddr)))
			return eSIR_LIM_IGNORE_BEACON;
	}

	/* STA in IBSS mode and SSID matches with ours */
	pPeerNode = ibss_peer_find(pMac, pHdr->sa);
	if (pPeerNode == NULL) {
		/* Peer not in the list - Collect BSS description & add to the list */
		uint32_t frameLen;
		tSirRetStatus retCode;

		/*
		 * Limit the Max number of IBSS Peers allowed as the max
		 * number of STA's allowed
		 * pMac->lim.gLimNumIbssPeers will be increamented after exiting
		 * this function. so we will add additional 1 to compare against
		 * pMac->lim.gLimIbssStaLimit
		 */
		if ((pMac->lim.gLimNumIbssPeers + 1) >=
		    pMac->lim.gLimIbssStaLimit) {
			/*Print every 100th time */
			if (pMac->lim.ibss_retry_cnt % 100 == 0) {
				pe_debug("**** MAX STA LIMIT HAS REACHED ****");
			}
			pMac->lim.ibss_retry_cnt++;
			return eSIR_LIM_MAX_STA_REACHED_ERROR;
		}
		pe_debug("IBSS Peer node does not exist, adding it");
		frameLen =
			sizeof(tLimIbssPeerNode) + ieLen - sizeof(uint32_t);

		pPeerNode = qdf_mem_malloc((uint16_t) frameLen);
		if (NULL == pPeerNode) {
			pe_err("alloc fail %d bytes storing IBSS peer info",
				frameLen);
			return eSIR_MEM_ALLOC_FAILED;
		}

		pPeerNode->beacon = NULL;
		pPeerNode->beaconLen = 0;

		ibss_peer_collect(pMac, pBeacon, pHdr, pPeerNode,
				  psessionEntry);
		pPeerNode->beacon = qdf_mem_malloc(ieLen);
		if (NULL == pPeerNode->beacon) {
			pe_err("Unable to allocate memory to store beacon");
		} else {
			qdf_mem_copy(pPeerNode->beacon, pIEs, ieLen);
			pPeerNode->beaconLen = (uint16_t) ieLen;
		}
		ibss_peer_add(pMac, pPeerNode);

		pStaDs =
			dph_lookup_hash_entry(pMac, pPeerNode->peerMacAddr, &peerIdx,
					      &psessionEntry->dph.dphHashTable);
		if (pStaDs != NULL) {
			/* / DPH node already exists for the peer */
			pe_warn("DPH Node present for just learned peer");
			lim_print_mac_addr(pMac, pPeerNode->peerMacAddr, LOGD);
			ibss_sta_info_update(pMac, pStaDs, pPeerNode,
					     psessionEntry);
			return eSIR_SUCCESS;
		}
		retCode =
			lim_ibss_sta_add(pMac, pPeerNode->peerMacAddr, psessionEntry);
		if (retCode != eSIR_SUCCESS) {
			pe_err("lim-ibss-sta-add failed reason: %x", retCode);
			lim_print_mac_addr(pMac, pPeerNode->peerMacAddr, LOGE);
			return retCode;
		}
		/* Decide protection mode */
		pStaDs =
			dph_lookup_hash_entry(pMac, pPeerNode->peerMacAddr, &peerIdx,
					      &psessionEntry->dph.dphHashTable);
		if (pMac->lim.gLimProtectionControl !=
		    WNI_CFG_FORCE_POLICY_PROTECTION_DISABLE)
			lim_ibss_decide_protection(pMac, pStaDs, &beaconParams,
						   psessionEntry);

		if (beaconParams.paramChangeBitmap) {
			pe_err("beaconParams.paramChangeBitmap=1 ---> Update Beacon Params");
			sch_set_fixed_beacon_fields(pMac, psessionEntry);
			beaconParams.bssIdx = psessionEntry->bssIdx;
			lim_send_beacon_params(pMac, &beaconParams, psessionEntry);
		}
	} else
		ibss_sta_caps_update(pMac, pPeerNode, psessionEntry);

	if (psessionEntry->limSmeState != eLIM_SME_NORMAL_STATE)
		return eSIR_SUCCESS;

	/* Received Beacon from same IBSS we're */
	/* currently part of. Inform Roaming algorithm */
	/* if not already that IBSS is active. */
	if (psessionEntry->limIbssActive == false) {
		limResetHBPktCount(psessionEntry);
		pe_warn("Partner joined our IBSS, Sending IBSS_ACTIVE Notification to SME");
		psessionEntry->limIbssActive = true;
		lim_send_sme_wm_status_change_ntf(pMac, eSIR_SME_IBSS_ACTIVE, NULL, 0,
						  psessionEntry->smeSessionId);
	}

	return eSIR_SUCCESS;
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
void lim_ibss_heart_beat_handle(tpAniSirGlobal mac_ctx, tpPESession session)
{
	tLimIbssPeerNode *tempnode, *prevnode;
	tLimIbssPeerNode *temp_next = NULL;
	uint16_t peer_idx = 0;
	tpDphHashNode stads = 0;
	uint32_t threshold = 0;
	uint16_t sta_idx = 0;
	uint8_t ucast_sig = 0;
	uint8_t bcast_sig = 0;

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
	while (tempnode != NULL) {
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
				ucast_sig = stads->ucUcastSig;
				bcast_sig = stads->ucBcastSig;

				(void)lim_del_sta(mac_ctx, stads, false,
						  session);
				lim_delete_dph_hash_entry(mac_ctx,
					stads->staAddr, peer_idx, session);
				lim_release_peer_idx(mac_ctx, peer_idx,
						     session);
				/* Send indication. */
				ibss_status_chg_notify(mac_ctx,
					tempnode->peerMacAddr, sta_idx,
					ucast_sig, bcast_sig,
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
void lim_ibss_decide_protection_on_delete(tpAniSirGlobal mac_ctx,
					  tpDphHashNode stads,
					  tpUpdateBeaconParams bcn_param,
					  tpPESession session)
{
	uint32_t phymode;
	tHalBitVal erpenabled = eHAL_CLEAR;
	tSirRFBand rfband = SIR_BAND_UNKNOWN;
	uint32_t i;

	if (NULL == stads)
		return;

	lim_get_rf_band_new(mac_ctx, &rfband, session);
	if (SIR_BAND_2_4_GHZ != rfband)
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

/** -----------------------------------------------------------------
   \fn __lim_ibss_peer_inactivity_handler
   \brief Internal function. Deletes FW indicated peer which is inactive
 \
   \param  tpAniSirGlobal    pMac
   \param  tpPESession       psessionEntry
   \param  tpSirIbssPeerInactivityInd peerInactivityInd
   \return None
   -----------------------------------------------------------------*/
static void
__lim_ibss_peer_inactivity_handler(tpAniSirGlobal pMac,
				   tpPESession psessionEntry,
				   tpSirIbssPeerInactivityInd peerInactivityInd)
{
	if (psessionEntry->limMlmState != eLIM_MLM_BSS_STARTED_STATE) {
		return;
	}

	/* delete the peer for which heartbeat is observed */
	lim_ibss_delete_peer(pMac, psessionEntry,
					  peerInactivityInd->peer_addr.bytes);
}

/** -------------------------------------------------------------
   \fn lim_process_ibss_peer_inactivity
   \brief Peer inactivity message handler
 \
   \param  tpAniSirGlobal    pMac
   \param  void*             buf
   \return None
   -------------------------------------------------------------*/
void lim_process_ibss_peer_inactivity(tpAniSirGlobal pMac, void *buf)
{
	/*
	 * --------------- HEARTBEAT OFFLOAD CASE ------------------
	 * This message handler is executed when the firmware identifies
	 * inactivity from one or more peer devices. We will come here
	 * for every inactive peer device
	 */
	uint8_t i;

	tSirIbssPeerInactivityInd *peerInactivityInd =
		(tSirIbssPeerInactivityInd *) buf;

	/*
	 * If IBSS is not started or heartbeat offload is not enabled
	 * we should not handle this request
	 */
	if (eLIM_STA_IN_IBSS_ROLE != pMac->lim.gLimSystemRole &&
	    !IS_IBSS_HEARTBEAT_OFFLOAD_FEATURE_ENABLE) {
		return;
	}

	/** If LinkMonitor is Disabled */
	if (!pMac->sys.gSysEnableLinkMonitorMode) {
		return;
	}

	for (i = 0; i < pMac->lim.maxBssId; i++) {
		if (true == pMac->lim.gpSession[i].valid &&
		    eSIR_IBSS_MODE == pMac->lim.gpSession[i].bssType) {
			__lim_ibss_peer_inactivity_handler(pMac,
							   &pMac->lim.gpSession[i],
							   peerInactivityInd);
			break;
		}
	}
}
