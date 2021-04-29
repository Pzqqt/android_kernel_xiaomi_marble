/*
 * Copyright (c) 2013-2016, 2018, 2019-2021 The Linux Foundation. All rights reserved.
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

#if !defined(__SME_FTAPI_H)
#define __SME_FTAPI_H

#ifdef WLAN_FEATURE_HOST_ROAM

#ifndef FEATURE_CM_ENABLE
/* FT neighbor roam callback user context */
typedef struct sFTRoamCallbackUsrCtx {
	struct mac_context *mac;
	uint8_t sessionId;
} tFTRoamCallbackUsrCtx, *tpFTRoamCallbackUsrCtx;

typedef struct sFTSMEContext {
	/* Saved pFTPreAuthRsp */
	tpSirFTPreAuthRsp psavedFTPreAuthRsp;
	/* Time to trigger reassoc once pre-auth is successful */
	qdf_mc_timer_t preAuthReassocIntvlTimer;
	tSirMacAddr preAuthbssId;       /* BSSID to preauth to */
	/* User context for the timer callback */
	tpFTRoamCallbackUsrCtx pUsrCtx;
} tftSMEContext, *tpftSMEContext;


/*--------------------------------------------------------------------------
  Prototype functions
  ------------------------------------------------------------------------*/
void sme_ft_open(mac_handle_t mac_handle, uint8_t sessionId);
void sme_ft_close(mac_handle_t mac_handle, uint8_t sessionId);
void sme_get_ft_pre_auth_response(mac_handle_t mac_handle, uint8_t sessionId,
				  uint8_t *ft_ies, uint32_t ft_ies_ip_len,
				  uint16_t *ft_ies_length);
void sme_get_rici_es(mac_handle_t mac_handle, uint8_t sessionId,
		     uint8_t *ric_ies,
		     uint32_t ric_ies_ip_len, uint32_t *ric_ies_length);
void sme_preauth_reassoc_intvl_timer_callback(void *context);
#endif /* FEATURE_CM_ENABLE */

void sme_ft_reset(mac_handle_t mac_handle, uint8_t sessionId);

/**
 * sme_set_ft_ies() - to set FT IEs
 * @mac_handle: opaque handle to the global MAC context
 * @session_id: sme session id
 * @ft_ies: pointer to FT IEs
 * @ft_ies_length: length of FT IEs
 *
 * Each time the supplicant sends down the FT IEs to the driver. This function
 * is called in SME. This function packages and sends the FT IEs to PE.
 *
 * Return: none
 */
void sme_set_ft_ies(mac_handle_t mac_handle, uint8_t sessionId,
		    const uint8_t *ft_ies, uint16_t ft_ies_length);

/**
 * sme_check_ft_status() - Check for key wait status in FT mode
 * @mac_handle: MAC handle
 * @session_id: vdev identifier
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_check_ft_status(mac_handle_t mac_handle, uint8_t session_id);

/**
 * sme_ft_key_ready_for_install() - API to check ft key ready for install
 * @mac_handle: MAC handle
 * @session_id: vdev identifier
 *
 * It is only applicable for LFR2.0 enabled
 *
 * Return: true when ft key is ready otherwise false
 */
bool sme_ft_key_ready_for_install(mac_handle_t mac_handle, uint8_t session_id);
void sme_set_ft_pre_auth_state(mac_handle_t mac_handle, uint8_t sessionId,
			       bool state);
bool sme_get_ft_pre_auth_state(mac_handle_t mac_handle, uint8_t sessionId);

#else /* WLAN_FEATURE_HOST_ROAM */

#ifndef FEATURE_CM_ENABLE
static inline void sme_ft_open(mac_handle_t mac_handle, uint8_t sessionId) {}
static inline void sme_ft_close(mac_handle_t mac_handle, uint8_t sessionId) {}
static inline  void sme_get_ft_pre_auth_response(mac_handle_t mac_handle,
						 uint8_t sessionId,
						 uint8_t *ft_ies,
						 uint32_t ft_ies_ip_len,
						 uint16_t *ft_ies_length) {}
static inline void sme_get_rici_es(mac_handle_t mac_handle, uint8_t sessionId,
				   uint8_t *ric_ies,
				   uint32_t ric_ies_ip_len,
				   uint32_t *ric_ies_length) {}
static inline void sme_preauth_reassoc_intvl_timer_callback(void *context) {}
#endif

static inline void sme_ft_reset(mac_handle_t mac_handle, uint8_t sessionId) {}
static inline
void sme_set_ft_ies(mac_handle_t mac_handle, uint8_t sessionId,
		    const uint8_t *ft_ies, uint16_t ft_ies_length) {}

static inline QDF_STATUS sme_check_ft_status(mac_handle_t mac_handle,
					     uint8_t session_id)
{
	return QDF_STATUS_E_FAILURE;
}

static inline bool
sme_ft_key_ready_for_install(mac_handle_t mac_handle, uint8_t session_id)
{
	return false;
}

static inline void sme_set_ft_pre_auth_state(mac_handle_t mac_handle,
					     uint8_t sessionId, bool state) {}
static inline bool sme_get_ft_pre_auth_state(mac_handle_t mac_handle,
					     uint8_t sessionId)
{
	return false;
}
#endif /* WLAN_FEATURE_HOST_ROAM */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * sme_reset_key() -Reset key information
 * @mac_handle: MAC handle
 * @vdev_id: vdev identifier
 *
 * Return: None
 */
void sme_reset_key(mac_handle_t mac_handle, uint8_t vdev_id);
#else
static inline void sme_reset_key(mac_handle_t mac_handle, uint8_t vdev_id) {}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

#endif
