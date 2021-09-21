/*
 * Copyright (c) 2011-2021 The Linux Foundation. All rights reserved.
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
 * \file csr_support.h
 *
 * Exports and types for the Common Scan and Roaming supporting interfaces.
 */
#ifndef CSR_SUPPORT_H__
#define CSR_SUPPORT_H__

#include "csr_link_list.h"
#include "csr_api.h"
#include "cds_reg_service.h"

#ifdef FEATURE_WLAN_WAPI
#define CSR_WAPI_OUI_SIZE              (4)
#endif /* FEATURE_WLAN_WAPI */

#define CSR_RSN_OUI_SIZE              (4)
#define CSR_WPA_OUI_SIZE              (4)

#define CSR_DOT11_SUPPORTED_RATES_MAX (12)
#define CSR_DOT11_EXTENDED_SUPPORTED_RATES_MAX (8)

#define CSR_DOT11_BASIC_RATE_MASK    WLAN_DOT11_BASIC_RATE_MASK

/**
 * struct csr_timer_info - CSR-specific timer context
 * @mac: Global MAC context associated with the timer
 * @vdev_id: Session associated with the timer
 */
struct csr_timer_info {
	struct mac_context *mac;
	uint8_t vdev_id;
};

#define CSR_IS_QOS_BSS(pIes)  \
		((pIes)->WMMParams.present || (pIes)->WMMInfoAp.present)

uint32_t csr_get_frag_thresh(struct mac_context *mac_ctx);
uint32_t csr_get_rts_thresh(struct mac_context *mac_ctx);

bool csr_is_bssid_match(struct qdf_mac_addr *pProfBssid,
			struct qdf_mac_addr *BssBssid);
enum bss_type csr_translate_bsstype_to_mac_type(eCsrRoamBssType csrtype);
/*
 * This function will allocate memory for the parsed IEs to the caller.
 * Caller must free the memory. after it is done with the data only if
 * this function succeeds
 */
QDF_STATUS csr_get_parsed_bss_description_ies(struct mac_context *mac_ctx,
					      struct bss_description *bss_desc,
					      tDot11fBeaconIEs **ppIEStruct);

/**
 * csr_is_auth_type_ese() - Checks whether Auth type is ESE or not
 * @AuthType: Authentication type
 *
 * Return: true, if auth type is ese, false otherwise
 */
bool csr_is_auth_type_ese(enum csr_akm_type AuthType);

#endif
