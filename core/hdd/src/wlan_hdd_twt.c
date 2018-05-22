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
 * DOC : wlan_hdd_twt.c
 *
 * WLAN Host Device Driver file for TWT (Target Wake Time) support.
 *
 */

#include "wlan_hdd_twt.h"
#include "wlan_hdd_main.h"
#include "wlan_hdd_cfg.h"
#include "cfg_api.h"

void hdd_twt_print_ini_config(struct hdd_context *hdd_ctx)
{
	hdd_debug("Name = [%s] Value = [%d]", CFG_ENABLE_TWT_NAME,
		  hdd_ctx->config->enable_twt);
	hdd_debug("Name = [%s] Value = [%d]", CFG_TWT_CONGESTION_TIMEOUT_NAME,
		  hdd_ctx->config->twt_congestion_timeout);
}

void hdd_update_tgt_twt_cap(struct hdd_context *hdd_ctx,
			    struct wma_tgt_cfg *cfg)
{
	struct wma_tgt_services *services = &cfg->services;

	hdd_debug("TWT: enable_twt=%d, tgt Req=%d, Res=%d",
		  hdd_ctx->config->enable_twt, services->twt_requestor,
		  services->twt_responder);

	sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_TWT_REQUESTOR,
			QDF_MIN(services->twt_requestor,
				hdd_ctx->config->enable_twt));

	sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_TWT_RESPONDER,
			QDF_MIN(services->twt_responder,
				hdd_ctx->config->enable_twt));

	/*
	 * Currently broadcast TWT is not supported
	 */
	sme_cfg_set_int(hdd_ctx->hHal, WNI_CFG_BCAST_TWT,
			QDF_MIN(0, hdd_ctx->config->enable_twt));
}

