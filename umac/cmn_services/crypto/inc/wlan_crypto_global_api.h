/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: Public APIs for crypto service
 */
#ifndef _WLAN_CRYPTO_GLOBAL_API_H_
#define _WLAN_CRYPTO_GLOBAL_API_H_

#include "wlan_crypto_global_def.h"
#include <qdf_crypto.h>
/**
 * wlan_crypto_set_vdev_param - called by ucfg to set crypto param
 * @vdev: vdev
 * @param: param to be set.
 * @value: value
 *
 * This function gets called from ucfg to set param
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_vdev_param(struct wlan_objmgr_vdev *vdev,
					wlan_crypto_param_type param,
					uint32_t value);

/**
 * wlan_crypto_set_peer_param - called by ucfg to set crypto param
 *
 * @peer: peer
 * @param: param to be set.
 * @value: value
 *
 * This function gets called from ucfg to set param
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_peer_param(struct wlan_objmgr_peer *peer,
						wlan_crypto_param_type param,
						uint32_t value);

/**
 * wlan_crypto_get_param - called by ucfg to get crypto param
 * @vdev: vdev
 * @param: param to be get.
 *
 * This function gets called from ucfg to get param
 *
 * Return: value or -1 for failure
 */
int32_t wlan_crypto_get_param(struct wlan_objmgr_vdev *vdev,
					wlan_crypto_param_type param);
/**
 * wlan_crypto_get_peer_param - called by ucfg to get crypto peer param
 * @peer: peer
 * @param: param to be get.
 *
 * This function gets called from ucfg to get peer param
 *
 * Return: value or -1 for failure
 */
int32_t wlan_crypto_get_peer_param(struct wlan_objmgr_peer *peer,
					wlan_crypto_param_type param);

/**
 * wlan_crypto_is_htallowed - called by ucfg to check if HT rates is allowed
 * @vdev: Vdev
 * @peer: Peer
 *
 * This function is called to check if HT rates is allowed
 *
 * Return: 0 for not allowed and +ve for allowed
 */
uint8_t wlan_crypto_is_htallowed(struct wlan_objmgr_vdev *vdev,
				 struct wlan_objmgr_peer *peer);
/**
 * wlan_crypto_setkey - called by ucfg to setkey
 * @vdev: vdev
 * @req_key: req_key with cipher type, key macaddress
 *
 * This function gets called from ucfg to sey key
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_setkey(struct wlan_objmgr_vdev *vdev,
					struct wlan_crypto_req_key *req_key);

/**
 * wlan_crypto_getkey - called by ucfg to get key
 * @vdev: vdev
 * @req_key: key value will be copied in this req_key
 * @mac_address: mac address of the peer for unicast key
 *                   or broadcast address if group key is requested.
 *
 * This function gets called from ucfg to get key
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_getkey(struct wlan_objmgr_vdev *vdev,
					struct wlan_crypto_req_key *req_key,
					uint8_t *mac_addr);

/**
 * wlan_crypto_delkey - called by ucfg to delete key
 * @vdev: vdev
 * @mac_address: mac address of the peer for unicast key
 *                   or broadcast address if group key is deleted.
 * @key_idx: key index to be deleted
 *
 * This function gets called from ucfg to delete key
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_delkey(struct wlan_objmgr_vdev *vdev,
					uint8_t *macaddr,
					uint8_t key_idx);

/**
 * wlan_crypto_default_key - called by ucfg to set default tx key
 * @vdev: vdev
 * @mac_address: mac address of the peer for unicast key
 *                   or broadcast address if group key need to made default.
 * @key_idx: key index to be made as default key
 * @unicast: is key was unicast or group key.
 *
 * This function gets called from ucfg to set default key
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_default_key(struct wlan_objmgr_vdev *vdev,
					uint8_t *macaddr,
					uint8_t key_idx,
					bool unicast);

/**
 * wlan_crypto_encap - called by mgmt for encap the frame based on cipher
 * @vdev: vdev
 * @wbuf: wbuf
 * @macaddr: macaddr
 * @encapdone: is encapdone already or not.
 *
 * This function gets called from mgmt txrx to encap frame.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_encap(struct wlan_objmgr_vdev *vdev,
					qdf_nbuf_t wbuf,
					uint8_t *macaddr,
					uint8_t encapdone);

/**
 * wlan_crypto_decap - called by mgmt for decap the frame based on cipher
 * @vdev: vdev
 * @wbuf: wbuf
 * @macaddr: macaddr
 * @tid: tid of the packet.
 *
 * This function gets called from mgmt txrx to decap frame.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_decap(struct wlan_objmgr_vdev *vdev,
					qdf_nbuf_t wbuf,
					uint8_t *macaddr,
					uint8_t tid);

/**
 * wlan_crypto_enmic - called by mgmt for adding mic in frame based on cipher
 * @vdev: vdev
 * @wbuf: wbuf
 * @macaddr: macaddr
 * @encapdone: is encapdone already or not.
 *
 * This function gets called from mgmt txrx to adding mic to the frame.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_enmic(struct wlan_objmgr_vdev *vdev,
					qdf_nbuf_t wbuf,
					uint8_t *macaddr,
					uint8_t encapdone);

/**
 * wlan_crypto_demic - called by mgmt for remove and check mic for
 *                                    the frame based on cipher
 * @vdev: vdev
 * @wbuf: wbuf
 * @macaddr: macaddr
 * @tid: tid of the frame
 * @keyid: keyid in the received frame
 *
 * This function gets called from mgmt txrx to decap frame.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_demic(struct wlan_objmgr_vdev *vdev,
			     qdf_nbuf_t wbuf,
			     uint8_t *macaddr,
			     uint8_t tid,
			     uint8_t keyid);

/**
 * wlan_crypto_vdev_is_pmf_enabled - called to check is pmf enabled in vdev
 * @vdev: vdev
 *
 * This function gets called to check is pmf enabled or not in vdev.
 *
 * Return: true or false
 */
bool wlan_crypto_vdev_is_pmf_enabled(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_crypto_vdev_is_pmf_required - called to check is pmf required in vdev
 * @vdev: vdev
 *
 * This function gets called to check is pmf required or not in vdev.
 *
 * Return: true or false
 */
bool wlan_crypto_vdev_is_pmf_required(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_crypto_is_pmf_enabled - called by mgmt txrx to check is pmf enabled
 * @vdev: vdev
 * @peer: peer
 *
 * This function gets called by mgmt txrx to check is pmf enabled or not.
 *
 * Return: true or false
 */
bool wlan_crypto_is_pmf_enabled(struct wlan_objmgr_vdev *vdev,
					struct wlan_objmgr_peer *peer);

/**
 * wlan_crypto_is_key_valid - called by mgmt txrx to check if key is valid
 * @vdev: vdev
 * @peer: peer
 * @keyidx : key index
 *
 * This function gets called by mgmt txrx to check if key is valid
 *
 * Return: true or false
 */
bool wlan_crypto_is_key_valid(struct wlan_objmgr_vdev *vdev,
			      struct wlan_objmgr_peer *peer,
			      uint16_t keyidx);

/**
 * wlan_crypto_add_mmie - called by mgmt txrx to add mmie in frame
 * @vdev: vdev
 * @frm:  frame starting pointer
 * @len: length of the frame
 *
 * This function gets called by mgmt txrx to add mmie in frame
 *
 * Return: end of frame or NULL in case failure
 */
uint8_t *wlan_crypto_add_mmie(struct wlan_objmgr_vdev *vdev,
					uint8_t *frm,
					uint32_t len);

/**
 * wlan_crypto_is_mmie_valid - called by mgmt txrx to check mmie of the frame
 * @vdev: vdev
 * @frm:  frame starting pointer
 * @efrm: end of frame pointer
 *
 * This function gets called by mgmt txrx to check mmie of the frame
 *
 * Return: true or false
 */
bool wlan_crypto_is_mmie_valid(struct wlan_objmgr_vdev *vdev,
					uint8_t *frm,
					uint8_t *efrm);

/**
 * wlan_crypto_wpaie_check - called by mlme to check the wpaie
 * @crypto_params: crypto params
 * @frm: ie buffer
 *
 * This function gets called by mlme to check the contents of wpa is
 * matching with given crypto params
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_wpaie_check(struct wlan_crypto_params *crypto_params,
				   const uint8_t *frm);

/**
 * wlan_crypto_rsnie_check - called by mlme to check the rsnie
 * @crypto_params: crypto params
 * @frm: ie buffer
 *
 * This function gets called by mlme to check the contents of rsn is
 * matching with given crypto params
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_rsnie_check(struct wlan_crypto_params *crypto_params,
				   const uint8_t *frm);
/**
 * wlan_crypto_build_wpaie - called by mlme to build wpaie
 * @vdev: vdev
 * @iebuf: ie buffer
 *
 * This function gets called by mlme to build wpaie from given vdev
 *
 * Return: end of buffer
 */
uint8_t *wlan_crypto_build_wpaie(struct wlan_objmgr_vdev *vdev,
					uint8_t *iebuf);

/**
 * wlan_crypto_build_rsnie_with_pmksa() - called by mlme to build rsnie
 * @vdev: vdev
 * @iebuf: ie buffer
 * @pmksa: pmksa struct
 *
 * This function gets called by mlme to build rsnie from given vdev
 *
 * Return: end of buffer
 */
uint8_t *wlan_crypto_build_rsnie_with_pmksa(struct wlan_objmgr_vdev *vdev,
					    uint8_t *iebuf,
					    struct wlan_crypto_pmksa *pmksa);

/**
 * wlan_crypto_build_rsnie - called by mlme to build rsnie
 * @vdev: vdev
 * @iebuf: ie buffer
 * @bssid: bssid mac address to add pmkid in rsnie
 *
 * This function gets called by mlme to build rsnie from given vdev
 *
 * Return: end of buffer
 */
uint8_t *wlan_crypto_build_rsnie(struct wlan_objmgr_vdev *vdev,
					uint8_t *iebuf,
					struct qdf_mac_addr *bssid);

/**
 * wlan_crypto_wapiie_check - called by mlme to check the wapiie
 * @crypto_params: crypto params
 * @frm: ie buffer
 *
 * This function gets called by mlme to check the contents of wapi is
 * matching with given crypto params
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_wapiie_check(struct wlan_crypto_params *crypto_params,
				    const uint8_t *frm);

/**
 * wlan_crypto_build_wapiie - called by mlme to build wapi ie
 * @vdev: vdev
 * @iebuf: ie buffer
 *
 * This function gets called by mlme to build wapi ie from given vdev
 *
 * Return: end of buffer
 */
uint8_t *wlan_crypto_build_wapiie(struct wlan_objmgr_vdev *vdev,
					uint8_t *iebuf);
/**
 * wlan_crypto_rsn_info - check is given params matching with vdev params.
 * @vdev: vdev
 * @crypto params: crypto params
 *
 * This function gets called by mlme to check is given params matching with
 * vdev params.
 *
 * Return: true success or false for failure.
 */
bool wlan_crypto_rsn_info(struct wlan_objmgr_vdev *vdev,
				struct wlan_crypto_params *crypto_params);
/**
 * wlan_crypto_pn_check - called by data patch for PN check
 * @vdev: vdev
 * @wbuf: wbuf
 *
 * This function gets called by data patch for PN check
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_crypto_pn_check(struct wlan_objmgr_vdev *vdev,
					qdf_nbuf_t wbuf);
/**
 * wlan_crypto_vdev_get_crypto_params - called by mlme to get crypto params
 * @vdev:vdev
 *
 * This function gets called by mlme to get crypto params
 *
 * Return: wlan_crypto_params or NULL in case of failure
 */
struct wlan_crypto_params *wlan_crypto_vdev_get_crypto_params(
						struct wlan_objmgr_vdev *vdev);
/**
 * wlan_crypto_peer_get_crypto_params - called by mlme to get crypto params
 * @peer:peer
 *
 * This function gets called by mlme to get crypto params
 *
 * Return: wlan_crypto_params or NULL in case of failure
 */
struct wlan_crypto_params *wlan_crypto_peer_get_crypto_params(
						struct wlan_objmgr_peer *peer);

/**
 * wlan_crypto_set_peer_wep_keys - set wep keys into peer entries
 * @vdev:vdev
 * @peer:peer
 *
 * This function gets called by mlme, when auth frame is received.
 * this helps in setting wep keys into peer data structure.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_crypto_set_peer_wep_keys(struct wlan_objmgr_vdev *vdev,
					struct wlan_objmgr_peer *peer);

/**
 * wlan_crypto_register_crypto_rx_ops - set crypto_rx_ops
 * @crypto_rx_ops: crypto_rx_ops
 *
 * This function gets called by object manger to register crypto rx ops.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_crypto_register_crypto_rx_ops(
			struct wlan_lmac_if_crypto_rx_ops *crypto_rx_ops);

/**
 * wlan_crypto_get_crypto_rx_ops - get crypto_rx_ops from psoc
 * @psoc: psoc
 *
 * This function gets called by umac to get the crypto_rx_ops
 *
 * Return: crypto_rx_ops
 */
struct wlan_lmac_if_crypto_rx_ops *wlan_crypto_get_crypto_rx_ops(
			struct wlan_objmgr_psoc *psoc);
/**
 * wlan_crypto_vdev_has_auth_mode - check authmode for vdev
 * @vdev: vdev
 * @authvalue: authvalue to be checked
 *
 * This function check is authvalue passed is set in vdev or not
 *
 * Return: true or false
 */
bool wlan_crypto_vdev_has_auth_mode(struct wlan_objmgr_vdev *vdev,
					wlan_crypto_auth_mode authmode);

/**
 * wlan_crypto_peer_has_auth_mode - check authmode for peer
 * @peer: peer
 * @authvalue: authvalue to be checked
 *
 * This function check is authvalue passed is set in peer or not
 *
 * Return: true or false
 */
bool wlan_crypto_peer_has_auth_mode(struct wlan_objmgr_peer *peer,
					wlan_crypto_auth_mode authvalue);

/**
 * wlan_crypto_vdev_has_ucastcipher - check ucastcipher for vdev
 * @vdev: vdev
 * @ucastcipher: ucastcipher to be checked
 *
 * This function check is ucastcipher passed is set in vdev or not
 *
 * Return: true or false
 */
bool wlan_crypto_vdev_has_ucastcipher(struct wlan_objmgr_vdev *vdev,
					wlan_crypto_cipher_type ucastcipher);

/**
 * wlan_crypto_peer_has_ucastcipher - check ucastcipher for peer
 * @peer: peer
 * @ucastcipher: ucastcipher to be checked
 *
 * This function check is ucastcipher passed is set in peer or not
 *
 * Return: true or false
 */
bool wlan_crypto_peer_has_ucastcipher(struct wlan_objmgr_peer *peer,
					wlan_crypto_cipher_type ucastcipher);


/**
 * wlan_crypto_vdev_has_mcastcipher - check mcastcipher for vdev
 * @vdev: vdev
 * @mcastcipher: mcastcipher to be checked
 *
 * This function check is mcastcipher passed is set in vdev or not
 *
 * Return: true or false
 */
bool wlan_crypto_vdev_has_mcastcipher(struct wlan_objmgr_vdev *vdev,
					wlan_crypto_cipher_type mcastcipher);

/**
 * wlan_crypto_peer_has_mcastcipher - check mcastcipher for peer
 * @peer: peer
 * @mcastcipher: mcastcipher to be checked
 *
 * This function check is mcastcipher passed is set in peer or not
 *
 * Return: true or false
 */
bool wlan_crypto_peer_has_mcastcipher(struct wlan_objmgr_peer *peer,
					wlan_crypto_cipher_type mcastcipher);

/**
 * wlan_crypto_vdev_has_mgmtcipher - check mgmtcipher for vdev
 * @vdev: vdev
 * @mgmtcipher: mgmtcipher to be checked
 *
 * This function checks any one of mgmtciphers are supported by vdev or not.
 *
 * Return: true or false
 */
bool wlan_crypto_vdev_has_mgmtcipher(struct wlan_objmgr_vdev *vdev,
				     uint32_t mgmtcipher);

/**
 * wlan_crypto_peer_has_mgmtcipher - check mgmtcipher for peer
 * @peer: peer
 * @mgmtcipher: mgmtcipher to be checked
 *
 * This function checks any one of mgmtciphers are supported by peer or not.
 *
 * Return: true or false
 */
bool wlan_crypto_peer_has_mgmtcipher(struct wlan_objmgr_peer *peer,
				     uint32_t mgmtcipher);

/**
 * wlan_crypto_get_keytype - get keytype
 * @key: key
 *
 * This function gets keytype from key
 *
 * Return: keytype
 */
wlan_crypto_cipher_type wlan_crypto_get_key_type(
						struct wlan_crypto_key *key);

/**
 * wlan_crypto_vdev_getkey - get key from vdev
 * @vdev: vdev
 * @keyix: keyix
 *
 * This function gets key from vdev
 *
 * Return: key or NULL
 */
struct wlan_crypto_key *wlan_crypto_vdev_getkey(struct wlan_objmgr_vdev *vdev,
						uint16_t keyix);
/**
 * wlan_crypto_peer_getkey - get key from peer
 * @peer: peer
 * @keyix: keyix
 *
 * This function gets key from peer
 *
 * Return: key or NULL
 */
struct wlan_crypto_key *wlan_crypto_peer_getkey(struct wlan_objmgr_peer *peer,
						uint16_t keyix);
/**
 * wlan_crypto_get_peer_fils_aead - Get peer fils aead set flag
 * @peer: Peer object
 *
 * This function returns the peer fils aead set flag value.
 *
 * Return: 1 for enabled, 0 for disabled
 */
uint8_t wlan_crypto_get_peer_fils_aead(struct wlan_objmgr_peer *peer);

/**
 * wlan_crypto_set_peer_fils_aead - Set peer fils aead set flag
 * @peer: Peer object
 * @value: Value to set the flag
 *
 * This function set the peer fils aead set flag once FILS AUTH received.
 *
 * Return: None
 */
void wlan_crypto_set_peer_fils_aead(
			struct wlan_objmgr_peer *peer, uint8_t value);

/**
 * wlan_crypto_get_key_header - get header length
 * @key: key
 *
 * This function gets header length based on keytype
 *
 * Return: header length
 */
uint8_t wlan_crypto_get_key_header(struct wlan_crypto_key *key);

/**
 * wlan_crypto_get_key_trailer - get cipher trailer length
 * @key: key
 *
 * This function gets cipher trailer length based on keytype
 *
 * Return: cipher trailer length
 */
uint8_t wlan_crypto_get_key_trailer(struct wlan_crypto_key *key);

/**
 * wlan_crypto_get_key_miclen - get cipher miclen length
 * @key: key
 *
 * This function gets cipher miclen length based on keytype
 *
 * Return: cipher miclen length
 */
uint8_t wlan_crypto_get_key_miclen(struct wlan_crypto_key *key);

/**
 * wlan_crypto_get_keyid - get keyid from frame
 * @data: frame
 * @hdrlen: 802.11 header length
 *
 * This function parse frame and returns keyid
 *
 * Return: keyid
 */
uint16_t wlan_crypto_get_keyid(uint8_t *data, int hdrlen);

/**
 * wlan_crypto_restore_keys - restore crypto keys in hw keycache
 * @vdev: vdev
 *
 * This function restores keys in hw keycache
 *
 * Return: void
 */
void wlan_crypto_restore_keys(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_crypto_check_open_none - called by ucfg to check for open security
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 *
 * This function gets called from ucfg to check open security.
 *
 * Return: true or false
 */
bool wlan_crypto_check_open_none(struct wlan_objmgr_psoc *psoc,
				 uint8_t vedv_id);

/**
 * wlan_crypto_check_wep - called by ucfg to check for WEP security
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 *
 * This function gets called from ucfg to check WEP security.
 *
 * Return: true or false
 */
bool wlan_crypto_check_wep(struct wlan_objmgr_psoc *psoc, uint8_t vedv_id);

/**
 * wlan_crypto_check_rsn_match - called by ucfg to check for RSN match
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @ie_ptr: pointer to IEs
 * @ie_len: IE length
 * @peer_crypto_params: return peer crypto parameters
 *
 * This function gets called from ucfg to check RSN match.
 *
 * Return: true or false
 */
bool wlan_crypto_check_rsn_match(struct wlan_objmgr_psoc *psoc,
				 uint8_t vedv_id, uint8_t *ie_ptr,
				 uint16_t ie_len, struct wlan_crypto_params *
				 peer_crypto_params);

/**
 * wlan_crypto_check_rsn_match - called by ucfg to check for WPA match
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @ie_ptr: pointer to IEs
 * @ie_len: IE length
 * @peer_crypto_params: return peer crypto parameters
 *
 * This function gets called from ucfg to check WPA match.
 *
 * Return: true or false
 */
bool wlan_crypto_check_wpa_match(struct wlan_objmgr_psoc *psoc,
				 uint8_t vedv_id, uint8_t *ie_ptr,
				 uint16_t ie_len, struct wlan_crypto_params *
				 peer_crypto_params);

/**
 * wlan_crypto_parse_rsnxe_ie() - parse RSNXE IE
 * @rsnxe_ie: RSNXE IE pointer
 * @cap_len: pointer to hold len of ext capability
 *
 * Return: pointer to RSNXE capability or NULL
 */
const uint8_t *
wlan_crypto_parse_rsnxe_ie(const uint8_t *rsnxe_ie, uint8_t *cap_len);

/**
 * wlan_get_crypto_params_from_wapi_ie - Function to get crypto params
 * from wapi ie
 * @crypto_params: return crypto parameters
 * @ie_ptr: pointer to IEs
 * @ie_len: IE length
 *
 * This function is used to get the crypto parameters from wapi ie
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
#ifdef FEATURE_WLAN_WAPI
QDF_STATUS
wlan_get_crypto_params_from_wapi_ie(struct wlan_crypto_params *crypto_params,
				    const uint8_t *ie_ptr, uint16_t ie_len);

#else
static inline QDF_STATUS
wlan_get_crypto_params_from_wapi_ie(struct wlan_crypto_params *crypto_params,
				    const uint8_t *ie_ptr, uint16_t ie_len)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

/**
 * wlan_get_crypto_params_from_wpa_ie - Function to get crypto params
 * from wpa ie
 * @crypto_params: return crypto parameters
 * @ie_ptr: pointer to IEs
 * @ie_len: IE length
 *
 * This function is used to get the crypto parameters from wpa ie
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_get_crypto_params_from_wpa_ie(struct wlan_crypto_params *crypto_params,
				   const uint8_t *ie_ptr, uint16_t ie_len);

/**
 * wlan_get_crypto_params_from_rsn_ie - Function to get crypto params
 * from rsn ie
 * @crypto_params: return crypto parameters
 * @ie_ptr: pointer to IEs
 * @ie_len: IE length
 *
 * This function is used to get the crypto parameters from rsn ie
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_get_crypto_params_from_rsn_ie(struct wlan_crypto_params *crypto_params,
				   const uint8_t *ie_ptr, uint16_t ie_len);

/**
 * wlan_set_vdev_crypto_prarams_from_ie - Sets vdev crypto params from IE info
 * @vdev: vdev pointer
 * @ie_ptr: pointer to IE
 * @ie_len: IE length
 *
 * This function gets called from ucfg to set crypto params from IE data.
 *
 * Return: QDF_STATUS_SUCCESS or error code
 */
QDF_STATUS wlan_set_vdev_crypto_prarams_from_ie(struct wlan_objmgr_vdev *vdev,
						uint8_t *ie_ptr,
						uint16_t ie_len);
#ifdef WLAN_CRYPTO_GCM_OS_DERIVATIVE
static inline int wlan_crypto_aes_gmac(const uint8_t *key, size_t key_len,
				       uint8_t *iv, size_t iv_len,
				       const uint8_t *aad, size_t aad_len,
				       uint8_t *tag)
{
	return qdf_crypto_aes_gmac(key, key_len, iv, aad,
				   aad + AAD_LEN,
				   aad_len - AAD_LEN -
				   IEEE80211_MMIE_GMAC_MICLEN,
				   tag);
}
#endif
#ifdef WLAN_CRYPTO_OMAC1_OS_DERIVATIVE
static inline int omac1_aes_128(const uint8_t *key, const uint8_t *data,
				size_t data_len, uint8_t *mac)
{
	return qdf_crypto_aes_128_cmac(key, data, data_len, mac);
}

static inline int omac1_aes_256(const uint8_t *key, const uint8_t *data,
				size_t data_len, uint8_t *mac)
{
	return 0;
}
#endif

/**
 * ucfg_crypto_set_key_req() - Set key request to UCFG
 * @vdev: vdev object
 * @req: key request information
 * @key_type: indicates the type of key to be set, unicast or group key
 *
 * Return: None
 */
QDF_STATUS ucfg_crypto_set_key_req(struct wlan_objmgr_vdev *vdev,
				   struct wlan_crypto_key *req,
				   enum wlan_crypto_key_type key_type);

/**
 * wlan_crypto_get_default_key_idx() - Get the default key index
 * @vdev: vdev object
 * @igtk: denotes if the request is for igtk key type or not
 *
 * Return: Index of the requested key
 */
int8_t wlan_crypto_get_default_key_idx(struct wlan_objmgr_vdev *vdev,
				       bool igtk);

/**
 * wlan_crypto_get_cipher() - Get the cipher type for the vdev
 * @vdev: vdev object
 * @pairwise: denotes if the request is for pairwise cipher or not
 * @key_index: Index of the key whose cipher type has to be returned
 *
 * Return: enum wlan_crypto_cipher_type
 */
enum wlan_crypto_cipher_type
wlan_crypto_get_cipher(struct wlan_objmgr_vdev *vdev,
		       bool pairwise, uint8_t key_index);

#ifdef CRYPTO_SET_KEY_CONVERGED
/**
 * wlan_crypto_update_set_key_peer() - Update the peer for set key
 * @vdev: vdev object
 * @pairwise: denotes if the request is for pairwise cipher or not
 * @key_index: Index of the key whose peer has to be set
 * @peer_mac: MAC address of the peer
 *
 * Return: None
 */
void wlan_crypto_update_set_key_peer(struct wlan_objmgr_vdev *vdev,
				     bool pairwise, uint8_t key_index,
				     struct qdf_mac_addr *peer_mac);

/**
 * wlan_crypto_validate_key_params() - validates key parameters
 * @cipher: cipher type
 * @key_index: the index of the key
 * @key_len: key length
 * @seq_len: sequence counter length
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_crypto_validate_key_params(enum wlan_crypto_cipher_type cipher,
					   uint8_t key_index, uint8_t key_len,
					   uint8_t seq_len);

/**
 * wlan_crypto_save_key() - Allocate memory for storing key
 * @vdev: vdev object
 * @key_index: the index of the key that needs to be allocated
 * @crypto_key: Pointer to crypto key
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_crypto_save_key(struct wlan_objmgr_vdev *vdev,
				uint8_t key_index,
				struct wlan_crypto_key *crypto_key);

/**
 * wlan_crypto_get_key() - Get the stored key information
 * @vdev: vdev object
 * @key_index: the index of the key that needs to be retrieved
 *
 * Return: Key material
 */
struct wlan_crypto_key *wlan_crypto_get_key(struct wlan_objmgr_vdev *vdev,
					    uint8_t key_index);

/**
 * wlan_crypto_set_key_req() - Set key request
 * @vdev: vdev object
 * @req: key request information
 * @key_type: indicates the type of key to be set, unicast or group key
 *
 * Return: QDF status
 */
QDF_STATUS wlan_crypto_set_key_req(struct wlan_objmgr_vdev *vdev,
				   struct wlan_crypto_key *req,
				   enum wlan_crypto_key_type key_type);

/**
 * wlan_crypto_free_vdev_key - Free keys for vdev
 * @vdev: vdev object
 *
 * This function frees keys stored in vdev crypto object.
 *
 * Return: None
 */
void wlan_crypto_free_vdev_key(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_crypto_reset_vdev_params - Reset params for vdev
 * @vdev: vdev object
 *
 * This function reset params stored in vdev crypto object.
 *
 * Return: None
 */
void wlan_crypto_reset_vdev_params(struct wlan_objmgr_vdev *vdev);
#else
static inline void wlan_crypto_update_set_key_peer(
						struct wlan_objmgr_vdev *vdev,
						bool pairwise,
						uint8_t key_index,
						struct qdf_mac_addr *peer_mac)
{
}

static inline QDF_STATUS
wlan_crypto_save_key(struct wlan_objmgr_vdev *vdev, uint8_t key_index,
		     struct wlan_crypto_key *crypto_key)
{
	return QDF_STATUS_SUCCESS;
}

static inline struct wlan_crypto_key *
wlan_crypto_get_key(struct wlan_objmgr_vdev *vdev, uint8_t key_index)
{
	return NULL;
}

static inline
QDF_STATUS wlan_crypto_set_key_req(struct wlan_objmgr_vdev *vdev,
				   struct wlan_crypto_key *req,
				   enum wlan_crypto_key_type key_type)
{
	return QDF_STATUS_SUCCESS;
}

static inline void wlan_crypto_free_vdev_key(struct wlan_objmgr_vdev *vdev)
{
}

static inline void wlan_crypto_reset_vdev_prarams(struct wlan_objmgr_vdev *vdev)
{
}
#endif /* CRYPTO_SET_KEY_CONVERGED */

/**
 * wlan_crypto_get_peer_pmksa() - called to get pmksa based on pmksa parameter
 * @vdev: vdev
 * @pmksa: bssid
 *
 * This function is to get pmksa based on pmksa parameter
 *
 * Return: wlan_crypto_pmksa when match found else NULL.
 */
struct wlan_crypto_pmksa *
wlan_crypto_get_peer_pmksa(struct wlan_objmgr_vdev *vdev,
			   struct wlan_crypto_pmksa *pmksa);

/**
 * wlan_crypto_get_pmksa - called to get pmksa of bssid passed.
 * @vdev: vdev
 * @bssid: bssid
 *
 * This function gets called from to get pmksa for the bssid.
 *
 * Return: wlan_crypto_pmksa when match found else NULL.
 */
struct wlan_crypto_pmksa *
wlan_crypto_get_pmksa(struct wlan_objmgr_vdev *vdev,
		      struct qdf_mac_addr *bssid);

/**
 * wlan_crypto_get_fils_pmksa  - Get the PMKSA for FILS
 * SSID, if the SSID and cache id matches
 * @vdev:     Pointer with VDEV object
 * @cache_id: Cache id
 * @ssid:     Pointer to ssid
 * @ssid_len: SSID length
 *
 * Return: PMKSA entry if the cache id and SSID matches
 */
struct wlan_crypto_pmksa *
wlan_crypto_get_fils_pmksa(struct wlan_objmgr_vdev *vdev,
			   uint8_t *cache_id, uint8_t *ssid,
			   uint8_t ssid_len);

/**
 * wlan_crypto_pmksa_flush - called to flush saved pmksa
 * @crypto_params: crypto_params
 *
 * This function flush saved pmksa from crypto params.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_pmksa_flush(struct wlan_crypto_params *crypto_params);

/**
 * wlan_crypto_set_vdev_param - called by ucfg to set crypto param
 * @vdev: vdev
 * @pmksa: pmksa to be set/del.
 * @set: set(set=1) or del(set=0) pmksa from the list.
 *
 * This function gets called from ucfg to set or del pmksa.
 * when given pmksa is NULL and set is 0, it is for flush all entries.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_del_pmksa(struct wlan_objmgr_vdev *vdev,
				     struct wlan_crypto_pmksa *pmksa,
				     bool set);

/**
 * wlan_crypto_update_pmk_cache_ft - Updates the mobility domain information
 * for a BSSID in the PMKSA Cache table.
 * @vdev: vdev
 * @pmksa: pmksa to be updated.
 *
 * This function gets called from ucfg to update pmksa with mdid.
 * And flush the matching mdid entries.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_update_pmk_cache_ft(struct wlan_objmgr_vdev *vdev,
					   struct wlan_crypto_pmksa *pmksa);

#if defined(WLAN_SAE_SINGLE_PMK) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
/**
 * wlan_crypto_selective_clear_sae_single_pmk_entries - Clear the PMK entries
 * for BSS which have the single PMK flag set other than the current connected
 * AP
 * @vdev:       Vdev
 * @conn_bssid: Connected bssid
 */
void
wlan_crypto_selective_clear_sae_single_pmk_entries(
		struct wlan_objmgr_vdev *vdev, struct qdf_mac_addr *conn_bssid);

/**
 * wlan_crypto_set_sae_single_pmk_bss_cap - Set the peer SAE sinlge pmk
 * feature supported status
 * @vdev: Vdev
 * @bssid: BSSID for which the flag is to be set
 * @single_pmk_capable_bss: Flag to indicate Sae single pmk supported BSSID or
 * not
 */
void wlan_crypto_set_sae_single_pmk_bss_cap(struct wlan_objmgr_vdev *vdev,
					    struct qdf_mac_addr *bssid,
					    bool single_pmk_capable_bss);
#else
static inline void
wlan_crypto_selective_clear_sae_single_pmk_entries(
		struct wlan_objmgr_vdev *vdev, struct qdf_mac_addr *conn_bssid)
{
}

static inline
void wlan_crypto_set_sae_single_pmk_bss_cap(struct wlan_objmgr_vdev *vdev,
					    struct qdf_mac_addr *bssid,
					    bool single_pmk_capable_bss)
{
}
#endif

#ifdef WLAN_FEATURE_FILS_SK
/**
 * lim_create_fils_rik()- This API create rik using rrk coming from
 * supplicant.
 * @rrk: input rrk
 * @rrk_len: rrk length
 * @rik: Created rik
 * @rik_len: rik length to be filled
 *
 * rIK = KDF (K, S), where
 * K = rRK and
 * S = rIK Label + "\0" + cryptosuite + length
 * The rIK Label is the 8-bit ASCII string:
 * Re-authentication Integrity Key@ietf.org
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_crypto_create_fils_rik(uint8_t *rrk, uint8_t rrk_len,
				       uint8_t *rik, uint32_t *rik_len);
#endif /* WLAN_FEATURE_FILS_SK */
#endif /* end of _WLAN_CRYPTO_GLOBAL_API_H_ */
