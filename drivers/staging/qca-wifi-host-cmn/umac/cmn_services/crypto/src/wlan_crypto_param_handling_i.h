/*
 * Copyright (c) 2017-2018, 2021 The Linux Foundation. All rights reserved.
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
/* include files */
#ifndef __WLAN_CRYPTO_PARAM_HANDLING_I_H_
#define __WLAN_CRYPTO_PARAM_HANDLING_I_H_
/**
 * wlan_crypto_set_authmode - called by ucfg to configure authmode for vdev
 * @vdev: vdev
 * @authmode: authmode
 *
 * This function gets called from ucfg to configure authmode for vdev.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_authmode(struct wlan_crypto_params *crypto_params,
					uint32_t authmode);

/**
 * wlan_crypto_get_authmode - called by ucfg to get authmode of particular vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get authmode of particular vdev
 *
 * Return: authmode
 */
int32_t wlan_crypto_get_authmode(struct wlan_crypto_params *crypto_params);

/**
 * wlan_crypto_set_mcastcipher - called by ucfg to configure mcastcipher in vdev
 * @vdev: vdev
 * @wlan_crypto_cipher_type: mcast cipher value.
 *
 * This function gets called from ucfg to configure mcastcipher in vdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_mcastcipher(struct wlan_crypto_params *crypto_params,
					wlan_crypto_cipher_type cipher);
/**
 * wlan_crypto_get_mcastcipher - called by ucfg to get mcastcipher from vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get mcastcipher of particular vdev
 *
 * Return: mcast cipher
 */
int32_t wlan_crypto_get_mcastcipher(struct wlan_crypto_params *crypto_params);

/**
 * wlan_crypto_set_ucastciphers - called by ucfg to configure
 *                                        unicast ciphers in vdev
 * @vdev: vdev
 * @ciphers: bitmap value of all supported unicast ciphers
 *
 * This function gets called from ucfg to configure unicast ciphers in vdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_ucastciphers(struct wlan_crypto_params *,
						uint32_t ciphers);
/**
 * wlan_crypto_get_ucastciphers - called by ucfg to get ucastcipher from vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get supported unicast ciphers
 *
 * Return: bitmap value of all supported unicast ciphers
 */
int32_t wlan_crypto_get_ucastciphers(struct wlan_crypto_params *crypto_params);

/**
 * wlan_crypto_set_mgmtcipher - called by ucfg to configure
 *                                        mgmt ciphers in vdev
 * @vdev: vdev
 * @ciphers: bitmap value of all supported mgmt ciphers
 *
 * This function gets called from ucfg to configure mgmt ciphers in vdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_mgmtcipher(struct wlan_crypto_params *crypto_params,
					uint32_t ciphers);

/**
 * wlan_crypto_get_mgmtciphers - called by ucfg to get mgmtcipher from vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get supported unicast ciphers
 *
 * Return: bitmap value of all supported unicast ciphers
 */
int32_t wlan_crypto_get_mgmtciphers(struct wlan_crypto_params *crypto_params);

/**
 * wlan_crypto_set_cipher_cap - called by ucfg to configure
 *                                        cipher cap in vdev
 * @vdev: vdev
 * @ciphers: bitmap value of all supported unicast ciphers
 *
 * This function gets called from ucfg to configure unicast ciphers in vdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_cipher_cap(struct wlan_crypto_params *crypto_params,
					uint32_t ciphers);

/**
 * wlan_crypto_get_cipher_cap - called by ucfg to get cipher caps from vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get supported unicast ciphers
 *
 * Return: bitmap value of all supported unicast ciphers
 */
int32_t wlan_crypto_get_cipher_cap(struct wlan_crypto_params *crypto_params);

/**
 * wlan_crypto_set_rsn_cap - called by ucfg to configure
 *                                        cipher cap in vdev
 * @vdev: vdev
 * @ciphers: bitmap value of all supported unicast ciphers
 *
 * This function gets called from ucfg to configure unicast ciphers in vdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_rsn_cap(struct wlan_crypto_params *crypto_params,
					uint32_t ciphers);

/**
 * wlan_crypto_get_rsn_cap - called by ucfg to get rsn caps from vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get supported unicast ciphers
 *
 * Return: bitmap value of all supported unicast ciphers
 */
int32_t wlan_crypto_get_rsn_cap(struct wlan_crypto_params *crypto_params);


/**
 * wlan_crypto_set_key_mgmt - called by ucfg to configure
 *                                        key_mgmt in vdev
 * @vdev: vdev
 * @ciphers: bitmap value of all supported unicast ciphers
 *
 * This function gets called from ucfg to configure unicast ciphers in vdev
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
QDF_STATUS wlan_crypto_set_key_mgmt(struct wlan_crypto_params *crypto_params,
					uint32_t ciphers);

/**
 * wlan_crypto_get_key_mgmt - called by ucfg to get key mgmt from vdev
 * @vdev: vdev
 *
 * This function gets called from ucfg to get supported unicast ciphers
 *
 * Return: bitmap value of all supported unicast ciphers
 */
int32_t wlan_crypto_get_key_mgmt(struct wlan_crypto_params *crypto_params);
#endif /* __WLAN_CRYPTO_PARAM_HANDLING_I_H_ */
