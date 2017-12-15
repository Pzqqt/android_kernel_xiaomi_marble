/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: Implements public API for disa to interact with target/WMI
 */

#include "wlan_disa_tgt_api.h"
#include "wlan_disa_main.h"
#include "wlan_disa_public_struct.h"

/**
 * tgt_disa_encrypt_decrypt_req() - send encrypt/decrypt request to target if
 * @psoc: objmgr psoc object
 * @req: encrypt/decrypt parameters
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_disa_encrypt_decrypt_req(struct wlan_objmgr_psoc *psoc,
		struct disa_encrypt_decrypt_req_params *req)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * tgt_disa_encrypt_decrypt_resp() - receive encrypt/decrypt response
 *	from target if
 * @psoc: objmgr psoc object
 * @resp: encrypt/decrypt response containing results
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_disa_encrypt_decrypt_resp(struct wlan_objmgr_psoc *psoc,
		struct disa_encrypt_decrypt_resp_params *resp)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * tgt_disa_register_ev_handlers() - API to register disa event handlers
 * @psoc: objmgr psoc object
 *
 * Return: QDF_STATUS_SUCCESS in case of success else return error
 */
QDF_STATUS tgt_disa_register_ev_handlers(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * tgt_disa_unregister_ev_handlers() - API to unregister disa event handlers
 * @psoc: objmgr psoc object
 *
 * Return: QDF_STATUS_SUCCESS in case of success else return error
 */
QDF_STATUS tgt_disa_unregister_ev_handlers(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

