/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
#include <qdf_module.h>

int
ucfg_spectral_control(struct wlan_objmgr_pdev *pdev,
		      u_int id,
		      void *indata,
		      uint32_t insize, void *outdata, uint32_t *outsize)
{
	struct spectral_context *sc;

	if (!pdev) {
		spectral_err("PDEV is NULL!");
		return -EPERM;
	}
	sc = spectral_get_spectral_ctx_from_pdev(pdev);
	if (!sc) {
		spectral_err("spectral context is NULL!");
		return -EPERM;
	}

	return sc->sptrlc_spectral_control(pdev,
					   id,
					   indata, insize, outdata, outsize);
}
qdf_export_symbol(ucfg_spectral_control);

void ucfg_spectral_scan_set_ppid(struct wlan_objmgr_pdev *pdev, uint32_t ppid)
{
	struct pdev_spectral *ps = NULL;

	if (!pdev) {
		spectral_err("PDEV is NULL!");
		return;
	}
	ps = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						   WLAN_UMAC_COMP_SPECTRAL);
	if (!ps) {
		spectral_err("spectral context is NULL!");
		return;
	}
	ps->spectral_pid = ppid;
	spectral_debug("spectral ppid: %d", ppid);

	return;
}
