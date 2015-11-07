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

#ifdef WLAN_FEATURE_VOWIFI_11R
/**=========================================================================

   \brief Definitions for SME FT APIs

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <sms_debug.h>
#include <csr_inside_api.h>
#include <csr_neighbor_roam.h>

/*--------------------------------------------------------------------------
   Initialize the FT context.
   ------------------------------------------------------------------------*/
void sme_ft_open(tHalHandle hHal, uint32_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (NULL != pSession) {
		/* Clean up the context */
		cdf_mem_set(&pSession->ftSmeContext, sizeof(tftSMEContext), 0);

		pSession->ftSmeContext.pUsrCtx =
			cdf_mem_malloc(sizeof(tFTRoamCallbackUsrCtx));

		if (NULL == pSession->ftSmeContext.pUsrCtx) {
			sms_log(pMac, LOGE, FL("Memory allocation failure"));
			return;
		}
		pSession->ftSmeContext.pUsrCtx->pMac = pMac;
		pSession->ftSmeContext.pUsrCtx->sessionId = sessionId;

		status =
			cdf_mc_timer_init(&pSession->ftSmeContext.
					  preAuthReassocIntvlTimer,
					  CDF_TIMER_TYPE_SW,
					  sme_preauth_reassoc_intvl_timer_callback,
					  (void *)pSession->ftSmeContext.pUsrCtx);

		if (CDF_STATUS_SUCCESS != status) {
			sms_log(pMac, LOGE,
				FL
					("Preauth Reassoc interval Timer allocation failed"));
			cdf_mem_free(pSession->ftSmeContext.pUsrCtx);
			pSession->ftSmeContext.pUsrCtx = NULL;
			return;
		}
	}
}

/*--------------------------------------------------------------------------
   Cleanup the SME FT Global context.
   ------------------------------------------------------------------------*/
void sme_ft_close(tHalHandle hHal, uint32_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = NULL;

	/* Clear the FT Context */
	sme_ft_reset(hHal, sessionId);

	pSession = CSR_GET_SESSION(pMac, sessionId);
	if (NULL != pSession) {
		/* check if the timer is running */
		if (CDF_TIMER_STATE_RUNNING ==
		    cdf_mc_timer_get_current_state(&pSession->ftSmeContext.
						   preAuthReassocIntvlTimer)) {
			cdf_mc_timer_stop(&pSession->ftSmeContext.
					  preAuthReassocIntvlTimer);
		}

		if (CDF_STATUS_SUCCESS !=
		    cdf_mc_timer_destroy(&pSession->ftSmeContext.
					 preAuthReassocIntvlTimer)) {
			sms_log(pMac, LOGE,
				FL("preAuthReAssocTimer destroy failed"));
		}

		if (pSession->ftSmeContext.pUsrCtx != NULL) {
			sms_log(pMac, LOG1,
				FL
					("Freeing ftSmeContext.pUsrCtx and setting to NULL"));
			cdf_mem_free(pSession->ftSmeContext.pUsrCtx);
			pSession->ftSmeContext.pUsrCtx = NULL;
		}
	}
}

void sme_set_ft_pre_auth_state(tHalHandle hHal, uint32_t sessionId, bool state)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if (pSession)
		pSession->ftSmeContext.setFTPreAuthState = state;
}

bool sme_get_ft_pre_auth_state(tHalHandle hHal, uint32_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);

	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	if (pSession)
		return pSession->ftSmeContext.setFTPreAuthState;

	return false;
}

/**
 * sme_set_ft_ies() - to set FT IEs
 * @hal_ptr: pointer to HAL
 * @session_id: sme session id
 * @ft_ies: pointer to FT IEs
 * @ft_ies_length: length of FT IEs
 *
 * Each time the supplicant sends down the FT IEs to the driver. This function
 * is called in SME. This fucntion packages and sends the FT IEs to PE.
 *
 * Return: none
 */
void sme_set_ft_ies(tHalHandle hal_ptr, uint32_t session_id,
		const uint8_t *ft_ies, uint16_t ft_ies_length)
{
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal_ptr);
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);
	CDF_STATUS status = CDF_STATUS_E_FAILURE;

	if (NULL == session || NULL == ft_ies) {
		sms_log(mac_ctx, LOGE, FL(" ft ies or session is NULL"));
		return;
	}

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (!(CDF_IS_STATUS_SUCCESS(status)))
		return;

#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
	sms_log(mac_ctx, LOG1, "FT IEs Req is received in state %d",
			session->ftSmeContext.FTState);
#endif

	/* Global Station FT State */
	switch (session->ftSmeContext.FTState) {
	case eFT_START_READY:
	case eFT_AUTH_REQ_READY:
		if ((session->ftSmeContext.auth_ft_ies) &&
			(session->ftSmeContext.auth_ft_ies_length)) {
			/* Free the one we recvd last from supplicant */
			cdf_mem_free(session->ftSmeContext.auth_ft_ies);
			session->ftSmeContext.auth_ft_ies_length = 0;
			session->ftSmeContext.auth_ft_ies = NULL;
		}
		/* Save the FT IEs */
		session->ftSmeContext.auth_ft_ies =
					cdf_mem_malloc(ft_ies_length);
		if (NULL == session->ftSmeContext.auth_ft_ies) {
			sms_log(mac_ctx, LOGE,
				FL("Mem alloc failed for auth_ft_ies"));
			sme_release_global_lock(&mac_ctx->sme);
			return;
		}
		session->ftSmeContext.auth_ft_ies_length = ft_ies_length;
		cdf_mem_copy((uint8_t *)session->ftSmeContext.auth_ft_ies,
				ft_ies, ft_ies_length);
		session->ftSmeContext.FTState = eFT_AUTH_REQ_READY;

#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
		sms_log(mac_ctx, LOG1,
			FL("ft_ies_length=%d"), ft_ies_length);
#endif
		break;

	case eFT_AUTH_COMPLETE:
		/*
		 * We will need to re-start preauth. If we received FT
		 * IEs in eFT_PRE_AUTH_DONE state, it implies there was
		 * a rekey in our pre-auth state. Hence this implies we
		 * need Pre-auth again. OK now inform SME we have no
		 * pre-auth list. Delete the pre-auth node locally. Set
		 * your self back to restart pre-auth
		 */
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
		sms_log(mac_ctx, LOG1,
			FL("Preauth done & rcving AUTHREQ in state %d"),
			session->ftSmeContext.FTState);
		sms_log(mac_ctx, LOG1,
			FL("Unhandled reception of FT IES in state %d"),
			session->ftSmeContext.FTState);
#endif
		break;

	case eFT_REASSOC_REQ_WAIT:
		/*
		 * We are done with pre-auth, hence now waiting for
		 * reassoc req. This is the new FT Roaming in place At
		 * this juncture we'r ready to start sending Reassoc req
		 */
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
		sms_log(mac_ctx, LOG1, FL("New Reassoc Req=%p in state %d"),
			ft_ies, session->ftSmeContext.FTState);
#endif
		if ((session->ftSmeContext.reassoc_ft_ies) &&
			(session->ftSmeContext.reassoc_ft_ies_length)) {
			/* Free the one we recvd last from supplicant */
			cdf_mem_free(session->ftSmeContext.reassoc_ft_ies);
			session->ftSmeContext.reassoc_ft_ies_length = 0;
		}
		/* Save the FT IEs */
		session->ftSmeContext.reassoc_ft_ies =
					cdf_mem_malloc(ft_ies_length);
		if (NULL == session->ftSmeContext.reassoc_ft_ies) {
			sms_log(mac_ctx, LOGE,
				FL("Mem alloc fail for reassoc_ft_ie"));
			sme_release_global_lock(&mac_ctx->sme);
			return;
		}
		session->ftSmeContext.reassoc_ft_ies_length =
							ft_ies_length;
		cdf_mem_copy((uint8_t *)session->ftSmeContext.reassoc_ft_ies,
				ft_ies, ft_ies_length);

		session->ftSmeContext.FTState = eFT_SET_KEY_WAIT;
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
		sms_log(mac_ctx, LOG1,
			FL("ft_ies_length=%d state=%d"), ft_ies_length,
			session->ftSmeContext.FTState);
#endif

		break;

	default:
		sms_log(mac_ctx, LOGE, FL("Unhandled state=%d"),
			session->ftSmeContext.FTState);
		break;
	}
	sme_release_global_lock(&mac_ctx->sme);
}

/**
 * sme_ft_send_update_key_ind() - To send key update indication for FT session
 * @hal: pointer to HAL
 * @session_id: sme session id
 * @ftkey_info: FT key information
 *
 * To send key update indication for FT session
 *
 * Return: CDF_STATUS
 */
static
CDF_STATUS sme_ft_send_update_key_ind(tHalHandle hal, uint32_t session_id,
				      tCsrRoamSetKey *ftkey_info)
{
	tSirFTUpdateKeyInfo *msg;
	uint16_t msglen;
	CDF_STATUS status = CDF_STATUS_E_FAILURE;
	tSirKeyMaterial *keymaterial = NULL;
	tAniEdType ed_type;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
	int i = 0;

	sms_log(mac_ctx, LOG1, FL("keyLength %d"), ftkey_info->keyLength);
	for (i = 0; i < ftkey_info->keyLength; i++)
		sms_log(mac_ctx, LOG1, FL("%02x"), ftkey_info->Key[i]);
#endif

	if (ftkey_info->keyLength > CSR_MAX_KEY_LEN) {
		sms_log(mac_ctx, LOGE, FL("invalid keyLength %d"),
			ftkey_info->keyLength);
		return CDF_STATUS_E_FAILURE;
	}
	msglen  = sizeof(tSirFTUpdateKeyInfo);

	msg = cdf_mem_malloc(msglen);
	if (NULL == msg)
		return CDF_STATUS_E_NOMEM;

	cdf_mem_set(msg, msglen, 0);
	msg->messageType = eWNI_SME_FT_UPDATE_KEY;
	msg->length = msglen;

	keymaterial = &msg->keyMaterial;
	keymaterial->length = ftkey_info->keyLength;
	ed_type = csr_translate_encrypt_type_to_ed_type(ftkey_info->encType);
	keymaterial->edType = ed_type;
	keymaterial->numKeys = 1;
	keymaterial->key[0].keyId = ftkey_info->keyId;
	keymaterial->key[0].unicast = (uint8_t) true;
	keymaterial->key[0].keyDirection = ftkey_info->keyDirection;

	cdf_mem_copy(&keymaterial->key[0].keyRsc,
			ftkey_info->keyRsc, CSR_MAX_RSC_LEN);
	keymaterial->key[0].paeRole = ftkey_info->paeRole;
	keymaterial->key[0].keyLength = ftkey_info->keyLength;

	if (ftkey_info->keyLength && ftkey_info->Key) {
		cdf_mem_copy(&keymaterial->key[0].key, ftkey_info->Key,
				ftkey_info->keyLength);
		if (ftkey_info->keyLength == 16) {
			sms_log(mac_ctx, LOG1,
				FL("set update ind keyidx(%d) encType(%d) key = %02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X"),
				msg->keyMaterial.key[0].keyId,
				(tAniEdType) msg->keyMaterial.edType,
				msg->keyMaterial.key[0].key[0],
				msg->keyMaterial.key[0].key[1],
				msg->keyMaterial.key[0].key[2],
				msg->keyMaterial.key[0].key[3],
				msg->keyMaterial.key[0].key[4],
				msg->keyMaterial.key[0].key[5],
				msg->keyMaterial.key[0].key[6],
				msg->keyMaterial.key[0].key[7],
				msg->keyMaterial.key[0].key[8],
				msg->keyMaterial.key[0].key[9],
				msg->keyMaterial.key[0].key[10],
				msg->keyMaterial.key[0].key[11],
				msg->keyMaterial.key[0].key[12],
				msg->keyMaterial.key[0].key[13],
				msg->keyMaterial.key[0].key[14],
				msg->keyMaterial.key[0].key[15]);
		}
	}

	cdf_mem_copy(&msg->bssId[0],
			&ftkey_info->peerMac.bytes[0], CDF_MAC_ADDR_SIZE);
	msg->smeSessionId = session_id;
	sms_log(mac_ctx, LOG1, "BSSID = " MAC_ADDRESS_STR,
		MAC_ADDR_ARRAY(msg->bssId));
	status = cds_send_mb_message_to_mac(msg);

	return status;
}

bool sme_get_ftptk_state(tHalHandle hHal, uint32_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("pSession is NULL"));
		return false;
	}
	return pSession->ftSmeContext.setFTPTKState;
}

void sme_set_ftptk_state(tHalHandle hHal, uint32_t sessionId, bool state)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sms_log(pMac, LOGE, FL("pSession is NULL"));
		return;
	}
	pSession->ftSmeContext.setFTPTKState = state;
}

CDF_STATUS sme_ft_update_key(tHalHandle hHal, uint32_t sessionId,
			     tCsrRoamSetKey *pFTKeyInfo)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	CDF_STATUS status = CDF_STATUS_E_FAILURE;

	if (!pSession) {
		sms_log(pMac, LOGE, FL("pSession is NULL"));
		return CDF_STATUS_E_FAILURE;
	}

	if (pFTKeyInfo == NULL) {
		sms_log(pMac, LOGE, "%s: pFTKeyInfo is NULL", __func__);
		return CDF_STATUS_E_FAILURE;
	}

	status = sme_acquire_global_lock(&pMac->sme);
	if (!(CDF_IS_STATUS_SUCCESS(status))) {
		return CDF_STATUS_E_FAILURE;
	}
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
	sms_log(pMac, LOG1, "sme_ft_update_key is received in state %d",
		pSession->ftSmeContext.FTState);
#endif

	/* Global Station FT State */
	switch (pSession->ftSmeContext.FTState) {
	case eFT_SET_KEY_WAIT:
		if (sme_get_ft_pre_auth_state(hHal, sessionId) == true) {
			status =
				sme_ft_send_update_key_ind(pMac, sessionId, pFTKeyInfo);
			if (status != 0) {
				sms_log(pMac, LOGE, "%s: Key set failure %d",
					__func__, status);
				pSession->ftSmeContext.setFTPTKState = false;
				status = CDF_STATUS_FT_PREAUTH_KEY_FAILED;
			} else {
				pSession->ftSmeContext.setFTPTKState = true;
				status = CDF_STATUS_FT_PREAUTH_KEY_SUCCESS;
				sms_log(pMac, LOG1, "%s: Key set success",
					__func__);
			}
			sme_set_ft_pre_auth_state(hHal, sessionId, false);
		}

		pSession->ftSmeContext.FTState = eFT_START_READY;
#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
		sms_log(pMac, LOG1, "%s: state changed to %d status %d",
			__func__, pSession->ftSmeContext.FTState, status);
#endif
		break;

	default:
		sms_log(pMac, LOGW, "%s: Unhandled state=%d", __func__,
			pSession->ftSmeContext.FTState);
		status = CDF_STATUS_E_FAILURE;
		break;
	}
	sme_release_global_lock(&pMac->sme);

	return status;
}

/*--------------------------------------------------------------------------
 *
 * HDD Interface to SME. SME now sends the Auth 2 and RIC IEs up to the supplicant.
 * The supplicant will then proceed to send down the
 * Reassoc Req.
 *
 *------------------------------------------------------------------------*/
void sme_get_ft_pre_auth_response(tHalHandle hHal, uint32_t sessionId,
				  uint8_t *ft_ies, uint32_t ft_ies_ip_len,
				  uint16_t *ft_ies_length)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	CDF_STATUS status = CDF_STATUS_E_FAILURE;

	if (!pSession) {
		sms_log(pMac, LOGE, FL("pSession is NULL"));
		return;
	}

	*ft_ies_length = 0;

	status = sme_acquire_global_lock(&pMac->sme);
	if (!(CDF_IS_STATUS_SUCCESS(status)))
		return;

	/* All or nothing - proceed only if both BSSID and FT IE fit */
	if ((CDF_MAC_ADDR_SIZE +
	     pSession->ftSmeContext.psavedFTPreAuthRsp->ft_ies_length) >
	    ft_ies_ip_len) {
		sme_release_global_lock(&pMac->sme);
		return;
	}
	/* hdd needs to pack the bssid also along with the */
	/* auth response to supplicant */
	cdf_mem_copy(ft_ies, pSession->ftSmeContext.preAuthbssId,
		     CDF_MAC_ADDR_SIZE);

	/* Copy the auth resp FTIEs */
	cdf_mem_copy(&(ft_ies[CDF_MAC_ADDR_SIZE]),
		     pSession->ftSmeContext.psavedFTPreAuthRsp->ft_ies,
		     pSession->ftSmeContext.psavedFTPreAuthRsp->ft_ies_length);

	*ft_ies_length = CDF_MAC_ADDR_SIZE +
			 pSession->ftSmeContext.psavedFTPreAuthRsp->ft_ies_length;

	pSession->ftSmeContext.FTState = eFT_REASSOC_REQ_WAIT;

#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
	sms_log(pMac, LOG1, FL(" Filled auth resp = %d"), *ft_ies_length);
#endif
	sme_release_global_lock(&pMac->sme);
	return;
}

/*--------------------------------------------------------------------------
 *
 * SME now sends the RIC IEs up to the supplicant.
 * The supplicant will then proceed to send down the
 * Reassoc Req.
 *
 *------------------------------------------------------------------------*/
void sme_get_rici_es(tHalHandle hHal, uint32_t sessionId, uint8_t *ric_ies,
		     uint32_t ric_ies_ip_len, uint32_t *ric_ies_length)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	CDF_STATUS status = CDF_STATUS_E_FAILURE;

	if (!pSession) {
		sms_log(pMac, LOGE, FL("pSession is NULL"));
		return;
	}

	*ric_ies_length = 0;

	status = sme_acquire_global_lock(&pMac->sme);
	if (!(CDF_IS_STATUS_SUCCESS(status)))
		return;

	/* All or nothing */
	if (pSession->ftSmeContext.psavedFTPreAuthRsp->ric_ies_length >
	    ric_ies_ip_len) {
		sme_release_global_lock(&pMac->sme);
		return;
	}

	cdf_mem_copy(ric_ies,
		     pSession->ftSmeContext.psavedFTPreAuthRsp->ric_ies,
		     pSession->ftSmeContext.psavedFTPreAuthRsp->ric_ies_length);

	*ric_ies_length =
		pSession->ftSmeContext.psavedFTPreAuthRsp->ric_ies_length;

#ifdef WLAN_FEATURE_VOWIFI_11R_DEBUG
	sms_log(pMac, LOG1, FL(" Filled ric ies = %d"), *ric_ies_length);
#endif

	sme_release_global_lock(&pMac->sme);
	return;
}

/*--------------------------------------------------------------------------
 *
 * Timer callback for the timer that is started between the preauth completion and
 * reassoc request to the PE. In this interval, it is expected that the pre-auth response
 * and RIC IEs are passed up to the WPA supplicant and received back the necessary FTIEs
 * required to be sent in the reassoc request
 *
 *------------------------------------------------------------------------*/
void sme_preauth_reassoc_intvl_timer_callback(void *context)
{
	tFTRoamCallbackUsrCtx *pUsrCtx = (tFTRoamCallbackUsrCtx *) context;

	if (pUsrCtx) {
		csr_neighbor_roam_request_handoff(pUsrCtx->pMac,
						  pUsrCtx->sessionId);
	}
	return;
}

/*--------------------------------------------------------------------------
   Reset the FT context.
   ------------------------------------------------------------------------*/
void sme_ft_reset(tHalHandle hHal, uint32_t sessionId)
{
	tpAniSirGlobal pMac = PMAC_STRUCT(hHal);
	tCsrRoamSession *pSession = NULL;

	if (pMac == NULL) {
		CDF_TRACE(CDF_MODULE_ID_SME, CDF_TRACE_LEVEL_ERROR,
			  FL("pMac is NULL"));
		return;
	}

	pSession = CSR_GET_SESSION(pMac, sessionId);
	if (NULL != pSession) {
		if (pSession->ftSmeContext.auth_ft_ies != NULL) {
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
			sms_log(pMac, LOG1,
				FL("Freeing FT Auth IE %p and setting to NULL"),
				pSession->ftSmeContext.auth_ft_ies);
#endif
			cdf_mem_free(pSession->ftSmeContext.auth_ft_ies);
			pSession->ftSmeContext.auth_ft_ies = NULL;
		}
		pSession->ftSmeContext.auth_ft_ies_length = 0;

		if (pSession->ftSmeContext.reassoc_ft_ies != NULL) {
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
			sms_log(pMac, LOG1,
				FL
					("Freeing FT Reassoc IE %p and setting to NULL"),
				pSession->ftSmeContext.reassoc_ft_ies);
#endif
			cdf_mem_free(pSession->ftSmeContext.reassoc_ft_ies);
			pSession->ftSmeContext.reassoc_ft_ies = NULL;
		}
		pSession->ftSmeContext.reassoc_ft_ies_length = 0;

		if (pSession->ftSmeContext.psavedFTPreAuthRsp != NULL) {
#if defined WLAN_FEATURE_VOWIFI_11R_DEBUG
			sms_log(pMac, LOG1,
				FL
					("Freeing FtPreAuthRsp %p and setting to NULL"),
				pSession->ftSmeContext.psavedFTPreAuthRsp);
#endif
			cdf_mem_free(pSession->ftSmeContext.psavedFTPreAuthRsp);
			pSession->ftSmeContext.psavedFTPreAuthRsp = NULL;
		}
		pSession->ftSmeContext.setFTPreAuthState = false;
		pSession->ftSmeContext.setFTPTKState = false;

		cdf_mem_zero(pSession->ftSmeContext.preAuthbssId,
			     CDF_MAC_ADDR_SIZE);
		pSession->ftSmeContext.FTState = eFT_START_READY;
	}
}

/* End of File */
#endif /* WLAN_FEATURE_VOWIFI_11R */
