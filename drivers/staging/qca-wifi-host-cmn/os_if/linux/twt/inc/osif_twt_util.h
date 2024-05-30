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
 * DOC : osif_twt_util.h
 *
 */

#ifndef _OSIF_TWT_UTIL_H_
#define _OSIF_TWT_UTIL_H_

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
/**
 * struct twt_en_dis_priv - twt enable/disable private context
 * @vdev_id: vdev id
 */
struct twt_en_dis_priv {
	uint32_t pdev_id;
	uint32_t status;
};

/**
 * osif_twt_register_cb() - Set TWT osif callbacks
 *
 * API to set twt callbacks to osif
 *
 * Return: QDF_STATUS
 */
QDF_STATUS osif_twt_register_cb(void);

#else
static inline QDF_STATUS osif_twt_register_cb(void)
{
	return QDF_STATUS_SUCCESS;
}

#endif
#endif /* _OSIF_TWT_UTIL_H_ */

