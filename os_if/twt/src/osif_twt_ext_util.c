/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 *  DOC: osif_twt_ext_util.c
 */
#include <include/wlan_mlme_cmn.h>
#include <osif_twt_util.h>
#include <osif_twt_rsp.h>
#include <osif_twt_ext_rsp.h>
#include <wlan_twt_ucfg_ext_api.h>

static struct mlme_twt_ops twt_ops = {
	.mlme_twt_enable_complete_cb = osif_twt_enable_complete_cb,
	.mlme_twt_disable_complete_cb = osif_twt_disable_complete_cb,
	.mlme_twt_setup_complete_cb = osif_twt_setup_complete_cb,
	.mlme_twt_teardown_complete_cb = osif_twt_teardown_complete_cb,
	.mlme_twt_pause_complete_cb = osif_twt_pause_complete_cb,
	.mlme_twt_resume_complete_cb = osif_twt_resume_complete_cb,
	.mlme_twt_nudge_complete_cb = osif_twt_nudge_complete_cb,
	.mlme_twt_notify_complete_cb = osif_twt_notify_complete_cb,
	.mlme_twt_ack_complete_cb = osif_twt_ack_complete_cb,
};

/**
 * osif_twt_get_global_ops() - Get twt global ops
 *
 * Return: twt global ops
 */
static struct mlme_twt_ops *osif_twt_get_global_ops(void)
{
	return &twt_ops;
}

QDF_STATUS osif_twt_register_cb(void)
{
	return ucfg_twt_set_osif_cb(osif_twt_get_global_ops);
}

