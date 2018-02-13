/*
 * Copyright (c) 2011,2017-2018 The Linux Foundation. All rights reserved.
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

#include "spectral_cmn_api_i.h"
#include "../dispatcher/inc/wlan_spectral_tgt_api.h"

void
spectral_ctx_init_da(struct spectral_context *sc)
{
	if (!sc) {
		spectral_err("spectral context is null!");
		return;
	}
	sc->sptrlc_spectral_control = tgt_spectral_control;
	sc->sptrlc_pdev_spectral_init = tgt_pdev_spectral_init;
	sc->sptrlc_pdev_spectral_deinit = tgt_pdev_spectral_deinit;
	sc->sptrlc_set_spectral_config = tgt_set_spectral_config;
	sc->sptrlc_get_spectral_config = tgt_get_spectral_config;
	sc->sptrlc_start_spectral_scan = tgt_start_spectral_scan;
	sc->sptrlc_stop_spectral_scan = tgt_stop_spectral_scan;
	sc->sptrlc_is_spectral_active = tgt_is_spectral_active;
	sc->sptrlc_is_spectral_enabled = tgt_is_spectral_enabled;
	sc->sptrlc_set_debug_level = tgt_set_debug_level;
	sc->sptrlc_get_debug_level = tgt_get_debug_level;
	sc->sptrlc_get_spectral_capinfo = tgt_get_spectral_capinfo;
	sc->sptrlc_get_spectral_diagstats = tgt_get_spectral_diagstats;
	sc->sptrlc_register_wmi_spectral_cmd_ops =
	    tgt_register_wmi_spectral_cmd_ops;
	sc->sptrlc_register_netlink_cb = tgt_spectral_register_nl_cb;
	sc->sptrlc_use_nl_bcast = tgt_spectral_use_nl_bcast;
	sc->sptrlc_deregister_netlink_cb = tgt_spectral_deregister_nl_cb;
}
