
/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC : target_if_cfr_adrastea.h
 *
 * Target interface of CFR for ADRASTEA implementation
 *
 */

#ifndef _TARGET_IF_CFR_ADRASTEA_H
#define _TARGET_IF_CFR_ADRASTEA_H

/* Max size:
 * 532 = 112 bytes(csi_cfr_header) + 416 bytes(cfr payload) + 4 bytes(trailer)
 */
#define STREAMFS_MAX_SUBBUF_ADRASTEA 532

#define STREAMFS_NUM_BUF_SUBBUF_ADRASTEA 255

#ifdef WLAN_CFR_ADRASTEA
/**
 * cfr_adrastea_init_pdev() - Init pdev cfr for ADRASTEA
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Init pdev cfr parameters
 *
 * Return: QDF status
 */
QDF_STATUS cfr_adrastea_init_pdev(struct wlan_objmgr_psoc *psoc,
				  struct wlan_objmgr_pdev *pdev);

/**
 * cfr_adrastea_deinit_pdev() - De-init pdev cfr for ADRASTEA
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Deinit pdev cfr parameters
 *
 * Return: QDF status
 */
QDF_STATUS cfr_adrastea_deinit_pdev(struct wlan_objmgr_psoc *psoc,
				    struct wlan_objmgr_pdev *pdev);
#else
static inline
QDF_STATUS cfr_adrastea_init_pdev(struct wlan_objmgr_psoc *psoc,
				  struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS cfr_adrastea_deinit_pdev(struct wlan_objmgr_psoc *psoc,
				    struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#endif /* _TARGET_IF_CFR_ADRASTEA_H */

