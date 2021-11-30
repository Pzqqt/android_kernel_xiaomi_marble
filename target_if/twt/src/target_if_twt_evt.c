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
 *  DOC: target_if_twt_evt.c
 *  This file contains twt component's target related function definitions
 */
#include <target_if_twt.h>
#include <target_if_twt_evt.h>
#include <target_if_ext_twt.h>
#include "twt/core/src/wlan_twt_priv.h"
#include <wlan_twt_api.h>
#include <wmi_unified_twt_api.h>

int
target_if_twt_en_complete_event_handler(ol_scn_t scn,
					uint8_t *data, uint32_t datalen)
{
	return 0;
}

int
target_if_twt_disable_comp_event_handler(ol_scn_t scn,
					 uint8_t *data, uint32_t datalen)
{
	return 0;
}

