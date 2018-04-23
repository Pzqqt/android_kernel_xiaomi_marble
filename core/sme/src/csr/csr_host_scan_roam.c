/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: csr_host_scan_roam.c
 *
 * Host based roaming processing scan results and initiating the roaming
 */

#include "wma_types.h"
#include "csr_inside_api.h"
#include "sme_qos_internal.h"
#include "sme_inside.h"
#include "host_diag_core_event.h"
#include "host_diag_core_log.h"
#include "csr_api.h"
#include "sme_api.h"
#include "csr_neighbor_roam.h"
#include "mac_trace.h"
#include "wlan_policy_mgr_api.h"

/**
 * csr_roam_issue_reassociate() - Issue Reassociate
 * @pMac: Global MAC Context
 * @sessionId: SME Session ID
 * @pSirBssDesc: BSS Descriptor
 * @pIes: Pointer to the IE's
 * @pProfile: Roaming profile
 *
 * Return: Success or Failure
 */
QDF_STATUS csr_roam_issue_reassociate(tpAniSirGlobal pMac,
	uint32_t sessionId, tSirBssDescription *pSirBssDesc,
	tDot11fBeaconIEs *pIes, struct csr_roam_profile *pProfile)
{
	csr_roam_state_change(pMac, eCSR_ROAMING_STATE_JOINING, sessionId);
	/* Set the roaming substate to 'join attempt'... */
	csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_REASSOC_REQ,
			sessionId);
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  FL(" calling csr_send_join_req_msg (eWNI_SME_REASSOC_REQ)"));
	/* attempt to Join this BSS... */
	return csr_send_join_req_msg(pMac, sessionId, pSirBssDesc, pProfile,
			pIes, eWNI_SME_REASSOC_REQ);
}

/**
 * csr_roam_issue_reassociate_cmd() - Issue the reassociate command
 * @pMac: Global MAC Context
 * @sessionId: SME Session ID
 *
 * Return: Success or Failure status
 */
QDF_STATUS csr_roam_issue_reassociate_cmd(tpAniSirGlobal pMac,
		uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *pCommand = NULL;
	bool fHighPriority = true;
	bool fRemoveCmd = false;
	tListElem *pEntry;
	tSmeCmd *tmp_command;

	pEntry = csr_nonscan_active_ll_peek_head(pMac, LL_ACCESS_LOCK);
	if (pEntry) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		if (!pCommand) {
			sme_err("fail to get command buffer");
			return QDF_STATUS_E_RESOURCES;
		}
		if (eSmeCommandRoam == pCommand->command) {
			if (pCommand->u.roamCmd.roamReason ==
			    eCsrSmeIssuedAssocToSimilarAP)
				fRemoveCmd =
					csr_nonscan_active_ll_remove_entry(pMac,
							    pEntry,
							    LL_ACCESS_LOCK);
			else
				sme_err("Unexpected roam cmd present");
			if (fRemoveCmd == false)
				pCommand = NULL;
		}
	}
	if (NULL == pCommand) {
		sme_err("fail to get cmd buf based on prev roam command");
		return QDF_STATUS_E_RESOURCES;
	}
	do {
		/*
		 * Get a new sme command to save the necessary info for
		 * the following roaming process, such as BSS list and
		 * roam profile. Or those info will be freed in function
		 * csr_reinit_roam_cmd when releasing the current command.
		 */
		tmp_command = csr_get_command_buffer(pMac);
		if (tmp_command == NULL) {
			sme_err("fail to get cmd buf!");
			csr_release_command(pMac, pCommand);
			return QDF_STATUS_E_RESOURCES;
		}
		qdf_mem_copy(tmp_command, pCommand, sizeof(*pCommand));
		pCommand->u.roamCmd.fReleaseBssList = false;
		pCommand->u.roamCmd.hBSSList = CSR_INVALID_SCANRESULT_HANDLE;
		pCommand->u.roamCmd.fReleaseProfile = false;
		/*
		 * Invoking csr_release_command to release the current command
		 * or the following command will be stuck in pending queue.
		 * Because the API csr_nonscan_active_ll_remove_entry does
		 * not remove the current command from active queue.
		 */
		csr_release_command(pMac, pCommand);

		pCommand = tmp_command;
		/* Change the substate in case it is wait-for-key */
		if (CSR_IS_WAIT_FOR_KEY(pMac, sessionId)) {
			csr_roam_stop_wait_for_key_timer(pMac);
			csr_roam_substate_change(pMac, eCSR_ROAM_SUBSTATE_NONE,
						 sessionId);
		}
		pCommand->command = eSmeCommandRoam;
		pCommand->sessionId = (uint8_t) sessionId;
		pCommand->u.roamCmd.roamReason = eCsrSmeIssuedFTReassoc;
		status = csr_queue_sme_command(pMac, pCommand, fHighPriority);
		if (!QDF_IS_STATUS_SUCCESS(status))
			sme_err("fail to send message status: %d", status);
	} while (0);

	return status;
}

/**
 * csr_neighbor_roam_process_scan_results() - build roaming candidate list
 *
 * @mac_ctx: The handle returned by mac_open.
 * @sessionid: Session information
 * @scan_results_list: List obtained from csr_scan_get_result()
 *
 * This function applies various candidate checks like LFR, 11r, preauth, ESE
 * and builds a roamable AP list. It applies age limit only if no suitable
 * recent candidates are found.
 *
 * Output list is built in mac_ctx->roam.neighborRoamInfo[sessionid].
 *
 * Return: void
 */

void csr_neighbor_roam_process_scan_results(tpAniSirGlobal mac_ctx,
		uint8_t sessionid, tScanResultHandle *scan_results_list)
{
	tCsrScanResultInfo *scan_result;
	tpCsrNeighborRoamControlInfo n_roam_info =
		&mac_ctx->roam.neighborRoamInfo[sessionid];
	tpCsrNeighborRoamBSSInfo bss_info;
	uint64_t age = 0;
	uint8_t num_candidates = 0;
	uint8_t num_dropped = 0;
	/*
	 * first iteration of scan list should consider
	 * age constraint for candidates
	 */
	bool age_constraint = true;
#ifdef FEATURE_WLAN_ESE
	uint16_t qpresent;
	uint16_t qavail;
	bool voadmitted;
#endif
	/*
	 * Expecting the scan result already to be in the sorted order based on
	 * RSSI. Based on the previous state we need to check whether the list
	 * should be sorted again taking neighbor score into consideration. If
	 * previous state is CFG_CHAN_LIST_SCAN, there should not be a neighbor
	 * score associated with any of the BSS. If the previous state is
	 * REPORT_QUERY, then there will be neighbor score for each of the APs.
	 * For now, let us take top of the list provided as it is by CSR Scan
	 * result API. Hence it is assumed that neighbor score and rssi score
	 * are in the same order. This will be taken care later.
	 */

	do {
		while (true) {
			tSirBssDescription *descr;

			scan_result = csr_scan_result_get_next(
						mac_ctx, *scan_results_list);
			if (NULL == scan_result)
				break;
			descr = &scan_result->BssDescriptor;
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  FL("Scan result: BSSID " MAC_ADDRESS_STR
				     " (Rssi %d, Ch:%d)"),
				  MAC_ADDR_ARRAY(descr->bssId),
				  (int)abs(descr->rssi), descr->channelId);

			if (!qdf_mem_cmp(descr->bssId,
					n_roam_info->currAPbssid.bytes,
					sizeof(tSirMacAddr))) {
				/*
				 * currently associated AP. Do not have this
				 * in the roamable AP list
				 */
				QDF_TRACE(QDF_MODULE_ID_SME,
					  QDF_TRACE_LEVEL_DEBUG,
					  "SKIP-currently associated AP");
				continue;
			}

			/*
			 * Continue if MCC is disabled in INI and if AP
			 * will create MCC
			 */
			if (policy_mgr_concurrent_open_sessions_running(
				mac_ctx->psoc) &&
				!mac_ctx->roam.configParam.fenableMCCMode) {
				uint8_t conc_channel;

				conc_channel =
				  csr_get_concurrent_operation_channel(mac_ctx);
				if (conc_channel &&
				   (conc_channel !=
				   scan_result->BssDescriptor.channelId)) {
					sme_debug("MCC not supported so Ignore AP on channel %d",
					  scan_result->BssDescriptor.channelId);
					continue;
				}
			}
			/*
			 * In case of reassoc requested by upper layer, look
			 * for exact match of bssid & channel. csr cache might
			 * have duplicates
			 */
			if ((n_roam_info->uOsRequestedHandoff) &&
			    ((qdf_mem_cmp(descr->bssId,
					n_roam_info->handoffReqInfo.bssid.bytes,
					sizeof(tSirMacAddr)))
			     || (descr->channelId !=
				 n_roam_info->handoffReqInfo.channel))) {
				QDF_TRACE(QDF_MODULE_ID_SME,
					  QDF_TRACE_LEVEL_DEBUG,
					  "SKIP-not a candidate AP for OS requested roam");
				continue;
			}

			if ((n_roam_info->is11rAssoc) &&
			    (!csr_neighbor_roam_is_preauth_candidate(mac_ctx,
					sessionid, descr->bssId))) {
				sme_err("BSSID in preauth fail list. Ignore");
				continue;
			}

#ifdef FEATURE_WLAN_ESE
			if (!csr_roam_is_roam_offload_scan_enabled(mac_ctx) &&
			    (n_roam_info->isESEAssoc) &&
			    !csr_neighbor_roam_is_preauth_candidate(mac_ctx,
				sessionid, descr->bssId)) {
				sme_err("BSSID in preauth faillist. Ignore");
				continue;
			}

			qpresent = descr->QBSSLoad_present;
			qavail = descr->QBSSLoad_avail;
			voadmitted = n_roam_info->isVOAdmitted;
			if (voadmitted)
				sme_debug("New QBSS=%s,BWavail=0x%x,req=0x%x",
					((qpresent) ? "yes" : "no"), qavail,
					n_roam_info->MinQBssLoadRequired);
			if (voadmitted && qpresent &&
			    (qavail < n_roam_info->MinQBssLoadRequired)) {
				QDF_TRACE(QDF_MODULE_ID_SME,
					QDF_TRACE_LEVEL_DEBUG,
					"BSSID:" MAC_ADDRESS_STR "has no BW",
					MAC_ADDR_ARRAY(descr->bssId));
				continue;
			}
			if (voadmitted && !qpresent) {
				QDF_TRACE(QDF_MODULE_ID_SME,
					QDF_TRACE_LEVEL_DEBUG,
					"BSSID:" MAC_ADDRESS_STR "no LOAD IE",
					MAC_ADDR_ARRAY(descr->bssId));
				continue;
			}
#endif /* FEATURE_WLAN_ESE */

			/*
			 * If we are supporting legacy roaming, and
			 * if the candidate is on the "pre-auth failed" list,
			 * ignore it.
			 */
			if (csr_roam_is_fast_roam_enabled(mac_ctx, sessionid) &&
			    !csr_neighbor_roam_is_preauth_candidate(mac_ctx,
				sessionid, descr->bssId)) {
				sme_err("BSSID in preauth faillist Ignore");
				continue;
			}

			/* check the age of the AP */
			age = (uint64_t) qdf_mc_timer_get_system_time() -
					descr->received_time;
			if (age_constraint == true &&
				age > ROAM_AP_AGE_LIMIT_MS) {
				num_dropped++;
				QDF_TRACE(QDF_MODULE_ID_SME,
					QDF_TRACE_LEVEL_WARN,
					FL("Old AP (probe rsp/beacon) skipped.")
					);
				continue;
			}

			/* Finished all checks, now add it to candidate list */
			bss_info =
				qdf_mem_malloc(sizeof(tCsrNeighborRoamBSSInfo));
			if (NULL == bss_info) {
				sme_err("Memory alloc fail");
				continue;
			}
			bss_info->pBssDescription =
				qdf_mem_malloc(descr->length +
					sizeof(descr->length));
			if (bss_info->pBssDescription != NULL) {
				qdf_mem_copy(bss_info->pBssDescription, descr,
					descr->length + sizeof(descr->length));
			} else {
				sme_err("Memory alloc fail");
				qdf_mem_free(bss_info);
				continue;
			}
			/*
			 * Assign some preference value for now. Need to
			 * calculate theactual score based on RSSI and neighbor
			 * AP score
			 */
			bss_info->apPreferenceVal = 10;
			num_candidates++;
			csr_ll_insert_tail(&n_roam_info->roamableAPList,
				&bss_info->List, LL_ACCESS_LOCK);
		} /* end of while (csr_scan_result_get_next) */

		/* if some candidates were found, then no need to repeat */
		if (num_candidates)
			break;
		/*
		 * if age_constraint is already false, we have done two
		 * iterations and no candidate were found
		 */
		if (age_constraint == false) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				  "%s: No roam able candidates found",
				  __func__);
			break;
		}
		/*
		 * if all candidates were dropped rescan the scan
		 * list but this time without age constraint.
		 */
		age_constraint = false;
		/* if no candidates were dropped no need to repeat */
	} while (num_dropped);

	/*
	 * Now we have all the scan results in our local list. Good time to free
	 * up the the list we got as a part of csrGetScanResult
	 */
	csr_scan_result_purge(mac_ctx, *scan_results_list);
}

/**
 * csr_neighbor_roam_trigger_handoff() - Start roaming
 * @mac_ctx: Global MAC Context
 * @session_id: SME Session ID
 *
 * Return: None
 */
void csr_neighbor_roam_trigger_handoff(tpAniSirGlobal mac_ctx,
				      uint8_t session_id)
{
	if (csr_roam_is_fast_roam_enabled(mac_ctx, session_id))
		csr_neighbor_roam_issue_preauth_req(mac_ctx, session_id);
	else
		sme_err("Roaming is disabled");
}

/**
 * csr_neighbor_roam_process_scan_complete() - Post process the scan results
 * @pMac: Global MAC Context
 * @sessionId: SME Session ID
 *
 * Return: Success or Failure
 */
QDF_STATUS csr_neighbor_roam_process_scan_complete(tpAniSirGlobal pMac,
		uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	tCsrScanResultFilter scanFilter;
	tScanResultHandle scanResult;
	uint32_t tempVal = 0;
	QDF_STATUS hstatus;

	hstatus = csr_neighbor_roam_prepare_scan_profile_filter(pMac,
								&scanFilter,
								sessionId);
	sme_debug("Prepare scan to find neighbor AP filter status: %d",
		hstatus);
	if (QDF_STATUS_SUCCESS != hstatus) {
		sme_err("Scan Filter prep fail for Assoc %d Bail out",
			tempVal);
		return QDF_STATUS_E_FAILURE;
	}
	hstatus = csr_scan_get_result(pMac, &scanFilter, &scanResult);
	if (hstatus != QDF_STATUS_SUCCESS)
		sme_err("Get Scan Result status code %d", hstatus);
	/* Process the scan results and update roamable AP list */
	csr_neighbor_roam_process_scan_results(pMac, sessionId, &scanResult);

	/* Free the scan filter */
	csr_free_scan_filter(pMac, &scanFilter);

	tempVal = csr_ll_count(&pNeighborRoamInfo->roamableAPList);

	if (tempVal) {
		csr_neighbor_roam_trigger_handoff(pMac, sessionId);
		return QDF_STATUS_SUCCESS;
	}

	if (csr_roam_is_roam_offload_scan_enabled(pMac)) {
		if (pNeighborRoamInfo->uOsRequestedHandoff) {
			csr_roam_offload_scan(pMac, sessionId,
				ROAM_SCAN_OFFLOAD_START,
				REASON_NO_CAND_FOUND_OR_NOT_ROAMING_NOW);
			pNeighborRoamInfo->uOsRequestedHandoff = 0;
		} else {
			/* There is no candidate or We are not roaming Now.
			 * Inform the FW to restart Roam Offload Scan
			 */
			csr_roam_offload_scan(pMac, sessionId,
				ROAM_SCAN_OFFLOAD_RESTART,
				REASON_NO_CAND_FOUND_OR_NOT_ROAMING_NOW);
		}
		csr_neighbor_roam_state_transition(pMac,
				eCSR_NEIGHBOR_ROAM_STATE_CONNECTED, sessionId);
	}
	return QDF_STATUS_SUCCESS;

}

/**
 * csr_neighbor_roam_candidate_found_ind_hdlr()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: pointer to msg buff
 *
 * This function is called by CSR as soon as TL posts the candidate
 * found indication to SME via MC thread
 *
 * Return: QDF_STATUS_SUCCESS on success, corresponding error code otherwise
 */
QDF_STATUS csr_neighbor_roam_candidate_found_ind_hdlr(tpAniSirGlobal pMac,
		void *pMsg)
{
	tSirSmeCandidateFoundInd *pSirSmeCandidateFoundInd =
		(tSirSmeCandidateFoundInd *) pMsg;
	uint32_t sessionId = pSirSmeCandidateFoundInd->sessionId;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	sme_debug("Received indication from firmware");

	/* we must be in connected state, if not ignore it */
	if ((eCSR_NEIGHBOR_ROAM_STATE_CONNECTED !=
	     pNeighborRoamInfo->neighborRoamState)
	    || (pNeighborRoamInfo->uOsRequestedHandoff)) {
		sme_err("Recvd in NotCONNECTED or OsReqHandoff. Ignore");
		status = QDF_STATUS_E_FAILURE;
	} else {
		/* Future enhancements:
		 * If firmware tags candidate beacons, give them preference
		 * for roaming.
		 * Age out older entries so that new candidate beacons
		 * will get preference.
		 */
		status = csr_neighbor_roam_process_scan_complete(pMac,
								 sessionId);
		if (QDF_STATUS_SUCCESS != status) {
			sme_err("scan process complete failed, status %d",
				status);
			return QDF_STATUS_E_FAILURE;
		}
	}

	return status;
}

/**
 * csr_neighbor_roam_free_roamable_bss_list() - Frees roamable APs list
 * @mac_ctx: The handle returned by mac_open.
 * @llist: Neighbor Roam BSS List to be emptied
 *
 * Empties and frees all the nodes in the roamable AP list
 *
 * Return: none
 */
void csr_neighbor_roam_free_roamable_bss_list(tpAniSirGlobal mac_ctx,
					      tDblLinkList *llist)
{
	tpCsrNeighborRoamBSSInfo result = NULL;

	sme_debug("Emptying the BSS list. Current count: %d",
		csr_ll_count(llist));

	/*
	 * Pick up the head, remove and free the node till
	 * the list becomes empty
	 */
	while ((result = csr_neighbor_roam_next_roamable_ap(mac_ctx, llist,
							NULL)) != NULL) {
		csr_neighbor_roam_remove_roamable_ap_list_entry(mac_ctx,
			llist, result);
		csr_neighbor_roam_free_neighbor_roam_bss_node(mac_ctx, result);
	}
}

/**
 * csr_neighbor_roam_remove_roamable_ap_list_entry()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @pList: The list from which the entry should be removed
 * @pNeighborEntry: Neighbor Roam BSS Node to be removed
 *
 * This function removes a given entry from the given list
 *
 * Return: true if successfully removed, else false
 */
bool csr_neighbor_roam_remove_roamable_ap_list_entry(tpAniSirGlobal pMac,
						     tDblLinkList *pList,
						     tpCsrNeighborRoamBSSInfo
						     pNeighborEntry)
{
	if (pList) {
		return csr_ll_remove_entry(pList, &pNeighborEntry->List,
					   LL_ACCESS_LOCK);
	}

	sme_debug("Remove neigh BSS node from fail list. Current count: %d",
		csr_ll_count(pList));

	return false;
}

/**
 * csr_neighbor_roam_next_roamable_ap() - Get next AP from roamable AP list
 * @mac_ctx - The handle returned by mac_open.
 * @plist - The list from which the entry should be returned
 * @neighbor_entry - Neighbor Roam BSS Node whose next entry should be returned
 *
 * Gets the entry next to passed entry. If NULL is passed, return the entry
 * in the head of the list
 *
 * Return: Neighbor Roam BSS Node to be returned
 */
tpCsrNeighborRoamBSSInfo csr_neighbor_roam_next_roamable_ap(
				tpAniSirGlobal mac_ctx, tDblLinkList *llist,
				tpCsrNeighborRoamBSSInfo neighbor_entry)
{
	tListElem *entry = NULL;
	tpCsrNeighborRoamBSSInfo result = NULL;

	if (llist) {
		if (NULL == neighbor_entry)
			entry = csr_ll_peek_head(llist, LL_ACCESS_LOCK);
		else
			entry = csr_ll_next(llist, &neighbor_entry->List,
					LL_ACCESS_LOCK);
		if (entry)
			result = GET_BASE_ADDR(entry, tCsrNeighborRoamBSSInfo,
					List);
	}

	return result;
}


/**
 * csr_neighbor_roam_request_handoff() - Handoff to a different AP
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: Session ID
 *
 * This function triggers actual switching from one AP to the new AP.
 * It issues disassociate with reason code as Handoff and CSR as a part of
 * handling disassoc rsp, issues reassociate to the new AP
 *
 * Return: none
 */
void csr_neighbor_roam_request_handoff(tpAniSirGlobal mac_ctx,
		uint8_t session_id)
{
	struct csr_roam_info roam_info;
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	tCsrNeighborRoamBSSInfo handoff_node;
	uint32_t roamid = 0;
	QDF_STATUS status;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG, "%s session_id=%d",
		  __func__, session_id);

	if (neighbor_roam_info->neighborRoamState !=
		eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE) {
		sme_err("Roam requested when Neighbor roam is in %s state",
			mac_trace_get_neighbour_roam_state(
			neighbor_roam_info->neighborRoamState));
		return;
	}

	if (false == csr_neighbor_roam_get_handoff_ap_info(mac_ctx,
			&handoff_node, session_id)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
		FL("failed to obtain handoff AP"));
		return;
	}
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  FL("HANDOFF CANDIDATE BSSID "MAC_ADDRESS_STR),
		  MAC_ADDR_ARRAY(handoff_node.pBssDescription->bssId));

	qdf_mem_zero(&roam_info, sizeof(struct csr_roam_info));
	csr_roam_call_callback(mac_ctx, session_id, &roam_info, roamid,
			       eCSR_ROAM_FT_START, eCSR_ROAM_RESULT_SUCCESS);

	qdf_mem_zero(&roam_info, sizeof(struct csr_roam_info));
	csr_neighbor_roam_state_transition(mac_ctx,
			eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING, session_id);

	csr_neighbor_roam_send_lfr_metric_event(mac_ctx, session_id,
		handoff_node.pBssDescription->bssId,
		eCSR_ROAM_HANDOVER_SUCCESS);
	/* Free the profile.. Just to make sure we dont leak memory here */
	csr_release_profile(mac_ctx,
		&neighbor_roam_info->csrNeighborRoamProfile);
	/*
	 * Create the Handoff AP profile. Copy the currently connected profile
	 * and update only the BSSID and channel number. This should happen
	 * before issuing disconnect
	 */
	status = csr_roam_copy_connected_profile(mac_ctx, session_id,
			&neighbor_roam_info->csrNeighborRoamProfile);
	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("csr_roam_copy_connected_profile failed %d"),
			status);
		return;
	}
	qdf_mem_copy(neighbor_roam_info->csrNeighborRoamProfile.BSSIDs.bssid,
		     handoff_node.pBssDescription->bssId, sizeof(tSirMacAddr));
	neighbor_roam_info->csrNeighborRoamProfile.ChannelInfo.ChannelList[0] =
		handoff_node.pBssDescription->channelId;

	sme_debug("csr_roamHandoffRequested: disassociating with current AP");

	if (!QDF_IS_STATUS_SUCCESS
		    (csr_roam_issue_disassociate_cmd
			    (mac_ctx, session_id,
			    eCSR_DISCONNECT_REASON_HANDOFF))) {
		sme_warn("csr_roamHandoffRequested: fail to issue disassoc");
		return;
	}
	/* notify HDD for handoff, providing the BSSID too */
	roam_info.reasonCode = eCsrRoamReasonBetterAP;

	qdf_mem_copy(roam_info.bssid.bytes,
		     handoff_node.pBssDescription->bssId,
		     sizeof(struct qdf_mac_addr));

	csr_roam_call_callback(mac_ctx, session_id, &roam_info, 0,
			       eCSR_ROAM_ROAMING_START, eCSR_ROAM_RESULT_NONE);

}


/**
 * csr_neighbor_roam_get_handoff_ap_info - Identifies the best AP for roaming
 *
 * @pMac:        mac context
 * @session_id:     Session Id
 * @hand_off_node:    AP node that is the handoff candidate returned
 *
 * This function returns the best possible AP for handoff. For 11R case, it
 * returns the 1st entry from pre-auth done list. For non-11r case, it returns
 * the 1st entry from roamable AP list
 *
 * Return: true if able find handoff AP, false otherwise
 */

bool csr_neighbor_roam_get_handoff_ap_info(tpAniSirGlobal pMac,
			tpCsrNeighborRoamBSSInfo hand_off_node,
			uint8_t session_id)
{
	tpCsrNeighborRoamControlInfo ngbr_roam_info =
		&pMac->roam.neighborRoamInfo[session_id];
	tpCsrNeighborRoamBSSInfo bss_node = NULL;

	if (NULL == hand_off_node) {
		QDF_ASSERT(NULL != hand_off_node);
		return false;
	}
	if (ngbr_roam_info->is11rAssoc) {
		/* Always the BSS info in the head is the handoff candidate */
		bss_node = csr_neighbor_roam_next_roamable_ap(
			pMac,
			&ngbr_roam_info->FTRoamInfo.preAuthDoneList,
			NULL);
		sme_debug("Number of Handoff candidates: %d",
			csr_ll_count(&
				ngbr_roam_info->FTRoamInfo.preAuthDoneList));
	} else
#ifdef FEATURE_WLAN_ESE
	if (ngbr_roam_info->isESEAssoc) {
		/* Always the BSS info in the head is the handoff candidate */
		bss_node =
			csr_neighbor_roam_next_roamable_ap(pMac,
				&ngbr_roam_info->FTRoamInfo.preAuthDoneList,
				NULL);
		sme_debug("Number of Handoff candidates: %d",
			csr_ll_count(&ngbr_roam_info->FTRoamInfo.
			preAuthDoneList));
	} else
#endif
	if (csr_roam_is_fast_roam_enabled(pMac, session_id)) {
		/* Always the BSS info in the head is the handoff candidate */
		bss_node =
			csr_neighbor_roam_next_roamable_ap(pMac,
			&ngbr_roam_info->FTRoamInfo.preAuthDoneList,
			NULL);
		sme_debug("Number of Handoff candidates: %d",
			csr_ll_count(
				&ngbr_roam_info->FTRoamInfo.preAuthDoneList));
	} else {
		bss_node =
			csr_neighbor_roam_next_roamable_ap(pMac,
				&ngbr_roam_info->roamableAPList,
				NULL);
		sme_debug("Number of Handoff candidates: %d",
			csr_ll_count(&ngbr_roam_info->roamableAPList));
	}
	if (NULL == bss_node)
		return false;
	qdf_mem_copy(hand_off_node, bss_node, sizeof(tCsrNeighborRoamBSSInfo));
	return true;
}

/**
 * csr_neighbor_roam_is_handoff_in_progress()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: Session ID
 *
 * This function returns whether handoff is in progress or not based on
 * the current neighbor roam state
 *
 * Return: true if reassoc in progress, false otherwise
 */
bool csr_neighbor_roam_is_handoff_in_progress(tpAniSirGlobal pMac,
		uint8_t sessionId)
{
	if (eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING ==
	    pMac->roam.neighborRoamInfo[sessionId].neighborRoamState)
		return true;

	return false;
}

/**
 * csr_neighbor_roam_free_neighbor_roam_bss_node()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @neighborRoamBSSNode: Neighbor Roam BSS Node to be freed
 *
 * This function frees all the internal pointers CSR NeighborRoam BSS Info
 * and also frees the node itself
 *
 * Return: None
 */
void csr_neighbor_roam_free_neighbor_roam_bss_node(tpAniSirGlobal pMac,
						   tpCsrNeighborRoamBSSInfo
						   neighborRoamBSSNode)
{
	if (neighborRoamBSSNode) {
		if (neighborRoamBSSNode->pBssDescription) {
			qdf_mem_free(neighborRoamBSSNode->pBssDescription);
			neighborRoamBSSNode->pBssDescription = NULL;
		}
		qdf_mem_free(neighborRoamBSSNode);
		neighborRoamBSSNode = NULL;
	}
}

