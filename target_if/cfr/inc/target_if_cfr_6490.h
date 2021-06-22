/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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
 * DOC : target_if_cfr_6490.h
 *
 * Target interface of CFR for QCA6490 implementation
 *
 */

#ifndef _TARGET_IF_CFR_6490_H
#define _TARGET_IF_CFR_6490_H

#if defined(QCA_WIFI_QCA6490) || defined(QCA_WIFI_WCN7850)
#define CFR_MAC_ID_24G 1
#define CFR_MAC_ID_5G  0
#endif /* QCA_WIFI_QCA6490 */

/**
 * target_if_cfr_subscribe_ppdu_desc() - subscribe ppdu description
 * for CFR component
 *
 * @pdev: pointer to pdev object
 * @is_subscribe: subscribe or unsubscribe
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_cfr_subscribe_ppdu_desc(struct wlan_objmgr_pdev *pdev,
				  bool is_subscribe);

#endif /* _TARGET_IF_CFR_6490_H */
