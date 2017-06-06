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

 /**
 * DOC: Public APIs for crypto service
 */
#ifndef _WLAN_CRYPTO_GLOBAL_API_H_
#define _WLAN_CRYPTO_GLOBAL_API_H_


/**
 * wlan_crypto_set_param - called by ucfg to set crypto param
 *
 * @vdev: vdev
 * @param: param to be set.
 * @value: value
 *
 * This function gets called from ucfg to set param
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_param(struct wlan_objmgr_vdev *vdev,
					wlan_crypto_param_type param,
					uint32_t value);

/**
 * wlan_crypto_get_param - called by ucfg to get crypto param
 *
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
 *
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
 *
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
 *
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
 *
 * @vdev: vdev
 * @req_key: key value will be copied in this req_key
 * @mac_address: mac address of the peer for unicast key
 *                   or broadcast address if group key is requested.
 * This function gets called from ucfg to get key
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_getkey(struct wlan_objmgr_vdev *vdev,
					struct wlan_crypto_req_key *req_key,
					uint8_t *mac_addr);

/**
 * wlan_crypto_delkey - called by ucfg to delete key
 *
 * @vdev: vdev
 * @mac_address: mac address of the peer for unicast key
 *                   or broadcast address if group key is deleted.
 * @key_idx: key index to be deleted
 * This function gets called from ucfg to delete key
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_delkey(struct wlan_objmgr_vdev *vdev,
					uint8_t *macaddr,
					uint8_t key_idx);

/**
 * wlan_crypto_default_key - called by ucfg to set default tx key
 *
 * @vdev: vdev
 * @mac_address: mac address of the peer for unicast key
 *                   or broadcast address if group key need to made default.
 * @key_idx: key index to be made as default key
 * @unicast: is key was unicast or group key.
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
 *
 * @vdev: vdev
 * @wbuf: wbuf
 * @macaddr: macaddr
 * @encapdone: is encapdone already or not.
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
 *
 * @vdev: vdev
 * @wbuf: wbuf
 * @macaddr: macaddr
 * @tid: tid of the packet.
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
 *
 * @vdev: vdev
 * @wbuf: wbuf
 * @macaddr: macaddr
 * @encapdone: is encapdone already or not.
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
 *
 * @vdev: vdev
 * @wbuf: wbuf
 * @macaddr: macaddr
 * @tid: tid of the frame
 * This function gets called from mgmt txrx to decap frame.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_demic(struct wlan_objmgr_vdev *vdev,
					qdf_nbuf_t wbuf,
					uint8_t *macaddr,
					uint8_t tid);

/**
 * wlan_crypto_is_pmf_enabled - called by mgmt txrx to check is pmf enabled
 *
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
 * wlan_crypto_add_mmie - called by mgmt txrx to add mmie in frame
 *
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
 *
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
 *
 *
 * @crypto params: crypto params
 * @iebuf: ie buffer
 *
 * This function gets called by mlme to check the contents of wpa is
 * matching with given crypto params
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_wpaie_check(struct wlan_crypto_params *, uint8_t *frm);

/**
 * wlan_crypto_rsnie_check - called by mlme to check the rsnie
 *
 * @crypto params: crypto params
 * @iebuf: ie buffer
 *
 * This function gets called by mlme to check the contents of rsn is
 * matching with given crypto params
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_rsnie_check(struct wlan_crypto_params *, uint8_t *frm);
/**
 * wlan_crypto_build_wpaie - called by mlme to build wpaie
 *
 * @crypto params: crypto params
 * @iebuf: ie buffer
 *
 * This function gets called by mlme to build wpaie from given crypto params
 *
 * Return: end of buffer
 */
uint8_t *wlan_crypto_build_wpaie(struct wlan_crypto_params *,
					uint8_t *iebuf);
/**
 * wlan_crypto_build_rsnie - called by mlme to build rsnie
 *
 * @crypto params: crypto params
 * @iebuf: ie buffer
 *
 * This function gets called by mlme to build rsnie from given crypto params
 *
 * Return: end of buffer
 */
uint8_t *wlan_crypto_build_rsnie(struct wlan_crypto_params *,
					uint8_t *iebuf);

/**
 * wlan_crypto_rsn_info - check is given params matching with vdev params.
 *
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
 *
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
 *
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
 *
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
 *
 * @peer:peer
 *
 * This function gets called by mlme, when auth frame is received.
 * this helps in setting wep keys into peer data structure.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_crypto_set_peer_wep_keys(struct wlan_objmgr_vdev *vdev,
						uint8_t *mac_addr);

/**
 * wlan_crypto_register_crypto_rx_ops - set crypto_rx_ops
 *
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
 *
 * @psoc: psoc
 *
 * This function gets called by umac to get the crypto_rx_ops
 *
 * Return: crypto_rx_ops
 */
struct wlan_lmac_if_crypto_rx_ops *wlan_crypto_get_crypto_rx_ops(
			struct wlan_objmgr_psoc *psoc);
#endif /* end of _WLAN_CRYPTO_GLOBAL_API_H_ */
