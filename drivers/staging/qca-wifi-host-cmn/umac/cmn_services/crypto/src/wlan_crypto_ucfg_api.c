/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: contains crypto north bound interface definitions
 */

#include <qdf_types.h>
#include <wlan_crypto_global_def.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_crypto_global_api.h>
QDF_STATUS ucfg_crypto_set_key_req(struct wlan_objmgr_vdev *vdev,
				   struct wlan_crypto_key *req,
				   enum wlan_crypto_key_type key_type)
{
	/*
	 * It is the job of dispatcher to decide whether the
	 * request has to be sent to scheduler or should be
	 * sent as a offload request or process directly.
	 *
	 * Current requirement is to process set key request
	 * as run to completion without posting any messages.
	 * Hence the request handler is directly called from
	 * here.
	 */
	return wlan_crypto_set_key_req(vdev, req, key_type);
}

