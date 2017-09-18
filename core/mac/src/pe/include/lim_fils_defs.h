/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

#define FILS_EAP_TLV_MAX_DATA_LEN 255
#define FILS_SHA256_128_AUTH_TAG 16
#define FILS_SHA256_256_AUTH_TAG 32

#define FILS_SHA256_CRYPTO_TYPE "hmac(sha256)"
#define FILS_SHA384_CRYPTO_TYPE "hmac(sha384)"

/* RFC 6696 */
#define RMSK_LABEL "Re-authentication Master Session Key@ietf.org"

/* 12.12.2.5.3 80211-ai draft */
#define PTK_KEY_LABEL "FILS PTK Derivation"
#define MAX_ICK_LEN 48
#define MAX_KEK_LEN 64
#define MAX_TK_LEN 32
#define MAX_KEY_AUTH_DATA_LEN 48
#define MAX_GTK_LEN 255
#define MAX_IGTK_LEN 255
#define SIR_FILS_SESSION_IE_LEN 11
#define FILS_KEY_RSC_LEN 8
#define FILS_MAX_KEY_AUTH_LEN (MAX_ICK_LEN + MAX_KEK_LEN + MAX_TK_LEN)

#define IPN_LEN 6
#define FILS_SESSION_LENGTH 8
#define FILS_MAX_KDE_LIST_LEN 255
#define FILS_MAX_HLP_DATA_LEN 2048

/* 12.12.2.5.3 80211-ai draft */
#define FILS_SHA384_KEK_LEN 64
#define FILS_SHA256_KEK_LEN 32

/* 12.12.2.5.3 80211-ai draft */
#define FILS_SHA256_ICK_LEN 32
#define FILS_SHA384_ICK_LEN 48

#define TK_LEN_TKIP 32
#define TK_LEN_CCMP 16
#define TK_LEN_AES_128_CMAC 32

#define FILS_SHA256_PKM_LEN 32
#define FILS_SHA384_PKM_LEN 48

#define PMKID_LEN 16

#define MAX_PRF_INTERATIONS_COUNT 255

/* 9.4.2.180 FILS Session element */
#define SIR_FILS_SESSION_LENGTH    8
#define SIR_FILS_SESSION_EXT_EID   4

/* 9.4.2.184 FILS HLP Container Element */
#define SIR_FILS_HLP_EXT_EID 5

/* 9.4.2.190 FILS Nonce element */
#define SIR_FILS_NONCE_LENGTH      16
#define SIR_FILS_NONCE_EXT_EID   13

/*9.4.2.188 FILS Wrapped Data element */
#define SIR_FILS_WRAPPED_DATA_MAX_SIZE 255
#define SIR_FILS_WRAPPED_DATA_EXT_EID   8

#define MAX_IE_LENGTH 255

/* RFC 6696 5.3.1: EAP-Initiate/Re-auth-Start Packet */
#define SIR_FILS_EAP_REAUTH_PACKET_TYPE 1
#define SIR_FILS_EAP_INIT_PACKET_TYPE 2

#define FILS_AUTH_TAG_MAX_LENGTH 32

#define SIR_FILS_OPTIONAL_DATA_LEN 3
/* RFC 6696 4.3: RiK deriavtion */
#define SIR_FILS_RIK_LABEL "Re-authentication Integrity Key@ietf.org"

/* RFC 6696 5.3.1: EAP-Initiate/Re-auth-Start Packet */
#define SIR_FILS_EAP_TLV_KEYNAME_NAI 1
#define SIR_FILS_EAP_TLV_R_RK_LIFETIME 2
#define SIR_FILS_EAP_TLV_R_MSK_LIFETIME 3
#define SIR_FILS_EAP_TLV_DOMAIN_NAME 4
#define SIR_FILS_EAP_TLV_CRYPTO_LIST 5
#define SIR_FILS_EAP_TLV_AUTH_INDICATION 6

#define DATA_TYPE_GTK 1
#define DATA_TYPE_IGTK 9
#define KEY_RSC_LEN 8
#define KDE_IE_DATA_OFFSET 4
#define KDE_DATA_TYPE_OFFSET 3
#define GTK_OFFSET 2
#define IPN_OFFSET 2
#define IGTK_OFFSET 8

#define KDE_OUI_TYPE   "\x00\x0F\xAC"
#define KDE_OUI_TYPE_SIZE  3

#define SINGLE_ELEMENT_HASH_CNT 1

/*
 * struct eap_auth_reserved: this structure defines flags format in eap packets
 * as defined in RFC 6696 5.3.1
 * flag_r:
 * flag_b:
 * flag_l:
 */
struct eap_auth_reserved {
	uint8_t flag_r:1;
	uint8_t flag_b:1;
	uint8_t flag_l:1;
	uint8_t reverved:5;
};

/*
 * enum fils_erp_cryptosuite: this enum defines the cryptosuites used
 * to calculate auth tag and auth tag length as defined by RFC 6696 5.3.1
 * @HMAC_SHA256_64: sha256 with auth tag len as 64 bits
 * @HMAC_SHA256_128: sha256 with auth tag len as 128 bits
 * @HMAC_SHA256_256: sha256 with auth tag len as 256 bits
 */
enum fils_erp_cryptosuite {
	INVALID_CRYPTO = 0, /* reserved */
	HMAC_SHA256_64,
	HMAC_SHA256_128,
	HMAC_SHA256_256,
};

/*
 * struct fils_eap_tlv: this structure defines the eap header
 * for eap packet present in warpped data element IE
 * @type: type of packet
 * @length: length of packet
 * @data: pointer to eap data
 */
struct fils_eap_tlv {
	uint8_t type;
	uint8_t length;
	uint8_t data[FILS_EAP_TLV_MAX_DATA_LEN];
};

/* struct fils_auth_rsp_info: this structure saves the info from
 * fils auth response.
 * @keyname: pointer to keyname nai
 * @keylength: keyname nai length
 * @domain_name: pointer to domain name
 * @domain_len: domain length
 * @r_rk_lifetime: rRk lifetime
 * @r_msk_lifetime: RMSK lifetime
 * @sequence: sequence number to be validated
 * @fils_nonce: anonce
 * @assoc_delay: time in ms, DUT needs to wait after association req
 */
struct fils_auth_rsp_info {
	uint8_t *keyname;
	uint8_t keylength;
	uint8_t *domain_name;
	uint8_t domain_len;
	uint32_t r_rk_lifetime;
	uint32_t r_msk_lifetime;
	uint16_t sequence;
	uint8_t fils_nonce[SIR_FILS_NONCE_LENGTH];
	uint8_t assoc_delay;
};

/*
 * struct pe_fils_session: fils session info used in PE session
 * @is_fils_connection: whether connection is fils or not
 * @keyname_nai_data: keyname nai data
 * @keyname_nai_length: keyname nai length
 * @akm: akm type will be used
 * @auth: authentication type
 * @cipher: cipher type
 * @fils_erp_reauth_pkt: pointer to fils reauth packet data
 * @fils_erp_reauth_pkt_len: reauth packet length
 * @fils_rrk: pointer to fils rRk
 * @fils_rrk_len: fils rRk length
 * @fils_rik: pointer to fils rIk
 * @fils_rik_len: fils rIk length
 * @sequence_number: sequence number needs to be used in eap packet
 * @fils_session: fils session IE element
 * @fils_nonce: fils snonce
 * @rsn_ie: rsn ie used in auth request
 * @rsn_ie_len: rsn ie length
 * @fils_eap_finish_pkt: pointer to eap finish packet
 * @fils_eap_finish_pkt_len: eap finish packet length
 * @fils_rmsk: rmsk data pointer
 * @fils_rmsk_len: rmsk data length
 * @fils_pmk: pointer to pmk data
 * @fils_pmk_len: pmk length
 * @fils_pmkid: pointer to pmkid derived
 * @auth_info: data obtained from auth response
 * @ick: pointer to ick
 * @ick_len: ick length
 * @kek: pointer to kek
 * @kek_len: kek length
 * @tk: pointer to tk
 * @tk_len: tk length
 * @key_auth: data needs to be sent in assoc req, will be validated by AP
 * @key_auth_len: key auth data length
 * @ap_key_auth_data: data needs to be validated in assoc rsp
 * @ap_key_auth_len:  ap key data length
 * @gtk_len: gtk key length
 * @gtk: pointer to gtk data
 * @rsc: rsc value
 * @igtk_len: igtk length
 * @igtk: igtk data pointer
 * @ipn: pointer to ipn data
 * @dst_mac: HLP destination mac address
 * @src_mac: HLP source mac address
 * @hlp_data_len: HLP data length
 * @hlp_data: pointer to HLP data
 */
struct pe_fils_session {
	bool is_fils_connection;
	uint8_t *keyname_nai_data;
	uint8_t keyname_nai_length;
	uint8_t akm;
	uint8_t auth;
	uint8_t cipher;
	uint8_t *fils_erp_reauth_pkt;
	uint32_t fils_erp_reauth_pkt_len;
	uint8_t *fils_rrk;
	uint8_t fils_rrk_len;
	uint8_t *fils_rik;
	uint32_t fils_rik_len;
	uint16_t sequence_number;
	uint8_t fils_session[SIR_FILS_SESSION_LENGTH];
	uint8_t fils_nonce[SIR_FILS_NONCE_LENGTH];
	uint8_t rsn_ie[MAX_IE_LENGTH];
	uint8_t rsn_ie_len;
	uint8_t *fils_eap_finish_pkt;
	uint8_t fils_eap_finish_pkt_len;
	uint8_t *fils_rmsk;
	uint8_t fils_rmsk_len;
	uint8_t *fils_pmk;
	uint8_t fils_pmk_len;
	uint8_t fils_pmkid[PMKID_LEN];
	struct fils_auth_rsp_info auth_info;
	uint8_t ick[MAX_ICK_LEN];
	uint8_t ick_len;
	uint8_t kek[MAX_KEK_LEN];
	uint8_t kek_len;
	uint8_t tk[MAX_TK_LEN];
	uint8_t tk_len;
	uint8_t key_auth[MAX_KEY_AUTH_DATA_LEN];
	uint8_t key_auth_len;
	uint8_t ap_key_auth_data[MAX_KEY_AUTH_DATA_LEN];
	uint8_t ap_key_auth_len;
	uint8_t gtk_len;
	uint8_t gtk[MAX_GTK_LEN];
	uint8_t rsc;
	uint8_t igtk_len;
	uint8_t igtk[MAX_IGTK_LEN];
	uint8_t ipn[IPN_LEN];
	struct qdf_mac_addr dst_mac;
	struct qdf_mac_addr src_mac;
	uint16_t hlp_data_len;
	uint8_t *hlp_data;
};
