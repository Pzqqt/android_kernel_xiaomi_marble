/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 *
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

#include <wlan_spectral_ucfg_api.h>
#include "../../core/spectral_cmn_api_i.h"
#include <wlan_spectral_utils_api.h>

int ucfg_spectral_control(
	struct wlan_objmgr_pdev *pdev,
	u_int id,
	void *indata,
	u_int32_t insize,
	void *outdata, u_int32_t *outsize)
{
	struct spectral_context *sc;

	if (!pdev) {
		spectral_err("PDEV is NULL!\n");
		return -EPERM;
	}
	sc = spectral_get_spectral_ctx_from_pdev(pdev);
	if (!sc) {
		spectral_err("spectral context is NULL!\n");
		return -EPERM;
	}
	return sc->sptrlc_spectral_control(
		pdev,
		 id,
		 indata,
		 insize,
		 outdata,
		 outsize);
}
EXPORT_SYMBOL(ucfg_spectral_control);
