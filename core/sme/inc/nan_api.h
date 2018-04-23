/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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
 *
 * Name:  nan_api.h
 *
 * Description: NAN FSM defines.
 *
 */

#ifndef __NAN_API_H__
#define __NAN_API_H__

#include "qdf_types.h"

typedef void (*nan_callback)(void *, tSirNanEvent *);

#ifdef WLAN_FEATURE_NAN
typedef struct sNanRequestReq {
	uint16_t request_data_len;
	const uint8_t *request_data;
} tNanRequestReq, *tpNanRequestReq;

void sme_nan_register_callback(tHalHandle hHal, nan_callback callback);
void sme_nan_deregister_callback(tHalHandle hHal);
QDF_STATUS sme_nan_request(tpNanRequestReq input);
#else
static inline void sme_nan_register_callback(tHalHandle hHal,
					     nan_callback callback)
{
}
static inline void sme_nan_deregister_callback(tHalHandle hHal)
{
}
#endif /* WLAN_FEATURE_NAN */

#endif /* __NAN_API_H__ */
