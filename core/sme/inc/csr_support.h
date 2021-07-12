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
#define CSR_WAPI_VERSION_SUPPORTED     (1)
#define CSR_WAPI_MAX_AUTH_SUITES       (2)
#define CSR_WAPI_MAX_CYPHERS           (5)
#define CSR_WAPI_MAX_UNICAST_CYPHERS   (5)
#define CSR_WAPI_MAX_MULTICAST_CYPHERS (1)
#endif /* FEATURE_WLAN_WAPI */

#define CSR_RSN_OUI_SIZE              (4)
#define CSR_RSN_VERSION_SUPPORTED     (1)
#define CSR_RSN_MAX_AUTH_SUITES       (5)
#define CSR_RSN_MAX_CYPHERS           (5)
#define CSR_RSN_MAX_UNICAST_CYPHERS   (5)
#define CSR_RSN_MAX_MULTICAST_CYPHERS (1)

#define CSR_WPA_OUI_SIZE              (4)
#define CSR_WPA_VERSION_SUPPORTED     (1)
#define CSR_WME_OUI_SIZE (4)
#define CSR_WPA_MAX_AUTH_SUITES       (2)
#define CSR_WPA_MAX_CYPHERS           (5)
#define CSR_WPA_MAX_UNICAST_CYPHERS   (5)
#define CSR_WPA_MAX_MULTICAST_CYPHERS (1)
/* minimum size of the IE->length is the size of the Oui + Version. */
#define CSR_WPA_IE_MIN_SIZE           (6)
#define CSR_WPA_IE_MIN_SIZE_W_MULTICAST (HDD_WPA_IE_MIN_SIZE + HDD_WPA_OUI_SIZE)
#define CSR_WPA_IE_MIN_SIZE_W_UNICAST   (HDD_WPA_IE_MIN_SIZE + \
		HDD_WPA_OUI_SIZE + sizeof(pWpaIe->cUnicastCyphers))

#define CSR_DOT11_SUPPORTED_RATES_MAX (12)
#define CSR_DOT11_EXTENDED_SUPPORTED_RATES_MAX (8)

#define CSR_DOT11_BASIC_RATE_MASK    WLAN_DOT11_BASIC_RATE_MASK

/* NOTE these index are use as array index for csr_rsn_oui */
#define CSR_OUI_USE_GROUP_CIPHER_INDEX 0x00
#define CSR_OUI_WEP40_OR_1X_INDEX      0x01
#define CSR_OUI_TKIP_OR_PSK_INDEX      0x02
#define CSR_OUI_RESERVED_INDEX         0x03
#define CSR_OUI_AES_INDEX              0x04
#define CSR_OUI_WEP104_INDEX           0x05
/* ENUM_FILS_SHA384 9 */
/* ENUM_FILS_SHA384 10 */
/* ENUM_FT_FILS_SHA256 11 */
/* ENUM_FT_FILS_SHA384 12 */
#define CSR_OUI_AES_GCMP_INDEX         0x0D
#define CSR_OUI_AES_GCMP_256_INDEX     0x0E

#ifdef FEATURE_WLAN_WAPI
#define CSR_OUI_WAPI_RESERVED_INDEX    0x00
#define CSR_OUI_WAPI_WAI_CERT_OR_SMS4_INDEX    0x01
#define CSR_OUI_WAPI_WAI_PSK_INDEX     0x02
/* max idx, should be last & highest */
#define CSR_OUI_WAPI_WAI_MAX_INDEX     0x03
#endif /* FEATURE_WLAN_WAPI */

/* Generic Information Element Structure */
typedef struct sDot11IEHeader {
	uint8_t ElementID;
	uint8_t Length;
} qdf_packed tDot11IEHeader;

typedef struct tagCsrWpaIe {
	tDot11IEHeader IeHeader;
	uint8_t Oui[CSR_WPA_OUI_SIZE];
	uint16_t Version;
	uint8_t MulticastOui[CSR_WPA_OUI_SIZE];
	uint16_t cUnicastCyphers;
	struct {
		uint8_t Oui[CSR_WPA_OUI_SIZE];
	} qdf_packed UnicastOui[1];
} qdf_packed tCsrWpaIe;

typedef struct tagCsrWpaAuthIe {
	uint16_t cAuthenticationSuites;
	struct {
		uint8_t Oui[CSR_WPA_OUI_SIZE];
	} qdf_packed AuthOui[1];
} qdf_packed tCsrWpaAuthIe;

typedef struct tagCsrRSNIe {
	tDot11IEHeader IeHeader;
	uint16_t Version;
	uint8_t MulticastOui[CSR_RSN_OUI_SIZE];
	uint16_t cUnicastCyphers;
	struct {
		uint8_t Oui[CSR_RSN_OUI_SIZE];
	} qdf_packed UnicastOui[1];
} qdf_packed tCsrRSNIe;

typedef struct tagCsrRSNAuthIe {
	uint16_t cAuthenticationSuites;
	struct {
		uint8_t Oui[CSR_RSN_OUI_SIZE];
	} qdf_packed AuthOui[1];
} qdf_packed tCsrRSNAuthIe;

typedef struct tagCsrRSNPMKIe {
	uint16_t cPMKIDs;
	struct {
		uint8_t PMKID[PMKID_LEN];
	} qdf_packed PMKIDList[1];
} qdf_packed tCsrRSNPMKIe;

typedef struct tCsrIELenInfo {
	uint8_t min;
	uint8_t max;
} qdf_packed tCsrIELenInfo;

#ifdef FEATURE_WLAN_WAPI
typedef struct tagCsrWapiIe {
	tDot11IEHeader IeHeader;
	uint16_t Version;
	uint16_t cAuthenticationSuites;
	struct {
		uint8_t Oui[CSR_WAPI_OUI_SIZE];
	} qdf_packed AuthOui[1];
	uint16_t cUnicastCyphers;
	struct {
		uint8_t Oui[CSR_WAPI_OUI_SIZE];
	} qdf_packed UnicastOui[1];
	uint8_t MulticastOui[CSR_WAPI_OUI_SIZE];
	struct {
		uint16_t PreAuthSupported:1;
		uint16_t Reserved:15;
	} qdf_packed tCsrWapiCapabilities;
} qdf_packed tCsrWapiIe;
#endif /* FEATURE_WLAN_WAPI */

/**
 * struct csr_timer_info - CSR-specific timer context
 * @mac: Global MAC context associated with the timer
 * @vdev_id: Session associated with the timer
 */
struct csr_timer_info {
	struct mac_context *mac;
	uint8_t vdev_id;
};

#define CSR_IS_11A_BSS(bss_desc)    (eSIR_11A_NW_TYPE == (bss_desc)->nwType)
#define CSR_IS_BASIC_RATE(rate)     ((rate) & CSR_DOT11_BASIC_RATE_MASK)
#define CSR_IS_QOS_BSS(pIes)  \
		((pIes)->WMMParams.present || (pIes)->WMMInfoAp.present)
#define CSR_IS_UAPSD_BSS(pIes) \
	(((pIes)->WMMParams.present && \
	 ((pIes)->WMMParams.qosInfo & SME_QOS_AP_SUPPORTS_APSD)) || \
	 ((pIes)->WMMInfoAp.present && (pIes)->WMMInfoAp.uapsd))

eCsrMediaAccessType
csr_get_qos_from_bss_desc(struct mac_context *mac_ctx,
			  struct bss_description *pSirBssDesc,
			  tDot11fBeaconIEs *pIes);

bool csr_is_nullssid(uint8_t *pBssSsid, uint8_t len);
bool csr_is_infra_bss_desc(struct bss_description *pSirBssDesc);

tSirResultCodes csr_get_de_auth_rsp_status_code(struct deauth_rsp *pSmeRsp);
uint32_t csr_get_frag_thresh(struct mac_context *mac_ctx);
uint32_t csr_get_rts_thresh(struct mac_context *mac_ctx);

bool csr_rates_is_dot11_rate11a_supported_rate(uint8_t dot11Rate);
tAniEdType csr_translate_encrypt_type_to_ed_type(
		eCsrEncryptionType EncryptType);

bool csr_is_bssid_match(struct qdf_mac_addr *pProfBssid,
			struct qdf_mac_addr *BssBssid);
enum bss_type csr_translate_bsstype_to_mac_type(eCsrRoamBssType csrtype);
/* Caller allocates memory for pIEStruct */
QDF_STATUS csr_parse_bss_description_ies(struct mac_context *mac_ctx,
					 struct bss_description *bss_desc,
					 tDot11fBeaconIEs *pIEStruct);
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
