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

/**
 * DOC: qal_devcfg
 * QCA abstraction layer (QAL) device config APIs
 */

#if !defined(__I_QAL_DEVCFG_H)
#define __I_QAL_DEVCFG_H

/* Include Files */
#include <qdf_types.h>
#include <net/cfg80211.h>

/**
 * __qal_devcfg_send_response() - send devcfg response
 * @cfgbuf: response buffer
 *
 * This function will send the response for a config request
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
static inline QDF_STATUS
__qal_devcfg_send_response(qdf_nbuf_t cfgbuf)
{
	cfg80211_vendor_cmd_reply(cfgbuf);

	return QDF_STATUS_SUCCESS;
}
#endif /* __I_QAL_DEVCFG_H */
