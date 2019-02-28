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

/*
 * Layer b/w umac and target_if (ol) txops
 * It contains wrapers for txops
 */

#include <wlan_cfr_tgt_api.h>
#include <wlan_cfr_utils_api.h>
#include <target_type.h>
#include <cfr_defs_i.h>

uint32_t tgt_cfr_info_send(struct wlan_objmgr_pdev *pdev, void *head,
			   size_t hlen, void *data, size_t dlen, void *tail,
			   size_t tlen)
{
	struct pdev_cfr *pa;
	uint32_t status;

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);

	if (pa == NULL) {
		cfr_err("pdev_cfr is NULL\n");
		return -1;
	}

	if (head)
		status = cfr_streamfs_write(pa, (const void *)head, hlen);

	if (data)
		status = cfr_streamfs_write(pa, (const void *)data, dlen);

	if (tail)
		status = cfr_streamfs_write(pa, (const void *)tail, tlen);


	/* finalise the write */
	status = cfr_streamfs_flush(pa);

	return status;
}

void tgt_cfr_support_set(struct wlan_objmgr_psoc *psoc, uint32_t value)
{
	struct psoc_cfr *cfr_sc;

	if (psoc == NULL)
		return;

	cfr_sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_CFR);
	if (cfr_sc == NULL)
		return;

	cfr_sc->is_cfr_capable = !!value;
	cfr_debug("CFR:%s FW support advert=%d\n", __func__,
		    cfr_sc->is_cfr_capable);
}

static inline struct wlan_lmac_if_cfr_tx_ops *
	wlan_psoc_get_cfr_txops(struct wlan_objmgr_psoc *psoc)
{
	return &((psoc->soc_cb.tx_ops.cfr_tx_ops));
}

int tgt_cfr_get_target_type(struct wlan_objmgr_psoc *psoc)
{
	uint32_t target_type = 0;
	struct wlan_lmac_if_target_tx_ops *target_type_tx_ops;

	target_type_tx_ops = &psoc->soc_cb.tx_ops.target_tx_ops;

	if (target_type_tx_ops->tgt_get_tgt_type)
		target_type = target_type_tx_ops->tgt_get_tgt_type(psoc);

	return target_type;
}

int tgt_cfr_init_pdev(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	int status = 0;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_init_pdev)
		status = cfr_tx_ops->cfr_init_pdev(psoc, pdev);

	if (status != 0)
		cfr_err("Error occurred with exit code %d\n", status);

	return status;
}

int tgt_cfr_deinit_pdev(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	int status = 0;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_deinit_pdev)
		status = cfr_tx_ops->cfr_deinit_pdev(psoc, pdev);

	if (status != 0)
		cfr_err("Error occurred with exit code %d\n", status);

	return status;
}

int tgt_cfr_start_capture(struct wlan_objmgr_pdev *pdev,
			  struct wlan_objmgr_peer *peer,
			  struct cfr_capture_params *cfr_params)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	int status = 0;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_start_capture)
		status = cfr_tx_ops->cfr_start_capture(pdev, peer, cfr_params);

	if (status != 0)
		cfr_err("Error occurred with exit code %d\n", status);

	return status;
}

int tgt_cfr_stop_capture(struct wlan_objmgr_pdev *pdev,
			 struct wlan_objmgr_peer *peer)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	int status;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_stop_capture)
		status = cfr_tx_ops->cfr_stop_capture(pdev, peer);

	if (status != 0)
		cfr_err("Error occurred with exit code %d\n", status);

	return status;
}

int
tgt_cfr_enable_cfr_timer(struct wlan_objmgr_pdev *pdev, uint32_t cfr_timer)
{
	int status;
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_enable_cfr_timer)
		status = cfr_tx_ops->cfr_enable_cfr_timer(pdev, cfr_timer);

	if (status != 0)
		cfr_err("Error occurred with exit code %d\n", status);

	return status;
}
