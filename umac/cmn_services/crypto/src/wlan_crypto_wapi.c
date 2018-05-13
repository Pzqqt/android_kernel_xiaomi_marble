/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: Public API intialization of crypto service with object manager
 */
#include <qdf_types.h>
#include <wlan_cmn.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>

#include "wlan_crypto_global_def.h"
#include "wlan_crypto_def_i.h"
#include "wlan_crypto_main_i.h"
#include "wlan_crypto_obj_mgr_i.h"


static QDF_STATUS wapi_setkey(struct wlan_crypto_key *key)
{
	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS wapi_encap(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t encapdone,
				uint8_t hdrlen){
	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS wapi_decap(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t tid,
				uint8_t hdrlen){
	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS wapi_enmic(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t encapdone,
				uint8_t hdrlen){
	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS wapi_demic(struct wlan_crypto_key *key,
				qdf_nbuf_t wbuf,
				uint8_t tid,
				uint8_t hdrlen){
	return QDF_STATUS_SUCCESS;
}

const struct wlan_crypto_cipher wapi_cipher_table = {
	"WPI_SMS4",
	WLAN_CRYPTO_CIPHER_WAPI_SMS4,
	WLAN_CRYPTO_WPI_SMS4_IVLEN + WLAN_CRYPTO_WPI_SMS4_KIDLEN
		+ WLAN_CRYPTO_WPI_SMS4_PADLEN,
	WLAN_CRYPTO_WPI_SMS4_MICLEN,
	0,
	256,
	wapi_setkey,
	wapi_encap,
	wapi_decap,
	wapi_enmic,
	wapi_demic,
};

const struct wlan_crypto_cipher *wapi_register(void)
{
	return &wapi_cipher_table;
}
