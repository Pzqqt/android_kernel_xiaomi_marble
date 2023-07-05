/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: target_if_twt_evt.h
 */

#ifndef _TARGET_IF_TWT_EVT_H_
#define _TARGET_IF_TWT_EVT_H_

#include <wmi_unified_param.h>
#include <wlan_lmac_if_def.h>
#include <target_if.h>
#include <wlan_twt_public_structs.h>

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
/**
 * target_if_twt_en_complete_event_handler - TWT enable complete event handler
 * @scn: scn handle
 * @data: buffer with event
 * @datalen: buffer length
 *
 * Return: 0 on success, negative value on failure
 */
int
target_if_twt_en_complete_event_handler(ol_scn_t scn,
					uint8_t *data, uint32_t datalen);

/**
 * target_if_twt_disable_comp_event_handler - TWT disable complete event handler
 * @scn: scn handle
 * @data: buffer with event
 * @datalen: buffer length
 *
 * Return: 0 on success, negative value on failure
 */
int
target_if_twt_disable_comp_event_handler(ol_scn_t scn,
					 uint8_t *data, uint32_t datalen);

#else
static inline int
target_if_twt_en_complete_event_handler(ol_scn_t scn,
					uint8_t *data, uint32_t datalen)
{
	return 0;
}

static inline int
target_if_twt_disable_comp_event_handler(ol_scn_t scn,
					 uint8_t *data, uint32_t datalen)
{
	return 0;
}

#endif

#endif /* _TARGET_IF_TWT_EVT_H_ */
