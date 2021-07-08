/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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
	uint32_t status, total_len;
	uint8_t *nl_data = NULL;

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);

	if (pa == NULL) {
		cfr_err("pdev_cfr is NULL\n");
		return -1;
	}

	/* If CFR data transport mode is NL event then send single event*/
	if (pa->nl_cb.cfr_nl_cb) {
		total_len = hlen + dlen + tlen;

		nl_data = qdf_mem_malloc(total_len);
		if (!nl_data) {
			cfr_err("failed to alloc memory, len %d, vdev_id %d",
				total_len, pa->nl_cb.vdev_id);
			return QDF_STATUS_E_FAILURE;
		}

		if (hlen)
			qdf_mem_copy(nl_data, head, hlen);

		if (dlen)
			qdf_mem_copy(nl_data + hlen, data, dlen);

		if (tlen)
			qdf_mem_copy(nl_data + hlen + dlen, tail, tlen);

		pa->nl_cb.cfr_nl_cb(pa->nl_cb.vdev_id, pa->nl_cb.pid,
				    (const void *)nl_data, total_len);
		qdf_mem_free(nl_data);

		return QDF_STATUS_SUCCESS;
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
	cfr_debug("CFR: FW support advert=%d", cfr_sc->is_cfr_capable);
}

static inline struct wlan_lmac_if_cfr_tx_ops *
	wlan_psoc_get_cfr_txops(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_tx_ops *tx_ops;

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		cfr_err("tx_ops is NULL");
		return NULL;
	}
	return &tx_ops->cfr_tx_ops;
}

int tgt_cfr_get_target_type(struct wlan_objmgr_psoc *psoc)
{
	uint32_t target_type = 0;
	struct wlan_lmac_if_target_tx_ops *target_type_tx_ops;
	struct wlan_lmac_if_tx_ops *tx_ops;

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		cfr_err("tx_ops is NULL");
		return target_type;
	}
	target_type_tx_ops = &tx_ops->target_tx_ops;

	if (target_type_tx_ops->tgt_get_tgt_type)
		target_type = target_type_tx_ops->tgt_get_tgt_type(psoc);

	return target_type;
}

int tgt_cfr_validate_period(struct wlan_objmgr_psoc *psoc, u_int32_t period)
{
	uint32_t target_type = tgt_cfr_get_target_type(psoc);
	int status = 0;

	if (target_type == TARGET_TYPE_UNKNOWN) {
		cfr_err("cfr period validation fail due to invalid target type");
		return status;
	}

	/* Basic check is the period should be between 0 and MAX_CFR_PRD */
	if ((period < 0) || (period > MAX_CFR_PRD)) {
		cfr_err("Invalid period value: %d\n", period);
		return status;
	}

	if (target_type == TARGET_TYPE_QCN9000 ||
	    target_type == TARGET_TYPE_QCA6018 ||
	    target_type == TARGET_TYPE_QCA8074V2 ||
	    target_type == TARGET_TYPE_QCA5018) {
		/* No additional check required for these targets */
		status = 1;
	} else {
		if (!(period % CFR_MOD_PRD)) {
			status = 1;
		} else {
			cfr_err("Invalid period value. Value must be mod of %d",
				CFR_MOD_PRD);
		}
	}
	return status;
}

QDF_STATUS tgt_cfr_init_pdev(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_init_pdev)
		status = cfr_tx_ops->cfr_init_pdev(psoc, pdev);

	if (QDF_IS_STATUS_ERROR(status))
		cfr_err("Error occurred with exit code %d\n", status);

	return status;
}

QDF_STATUS tgt_cfr_deinit_pdev(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_deinit_pdev)
		status = cfr_tx_ops->cfr_deinit_pdev(psoc, pdev);

	if (QDF_IS_STATUS_ERROR(status))
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
	int status = 0;
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
	int status = 0;
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_enable_cfr_timer)
		status = cfr_tx_ops->cfr_enable_cfr_timer(pdev, cfr_timer);

	if (status != 0)
		cfr_err("Error occurred with exit code %d\n", status);

	return status;
}

#ifdef WLAN_ENH_CFR_ENABLE
QDF_STATUS
tgt_cfr_config_rcc(struct wlan_objmgr_pdev *pdev,
		   struct cfr_rcc_param *rcc_param)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_config_rcc)
		status = cfr_tx_ops->cfr_config_rcc(pdev, rcc_param);

	if (status != QDF_STATUS_SUCCESS)
		cfr_err("Error occurred with exit code %d\n", status);

	return status;
}

void tgt_cfr_start_lut_age_timer(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc) {
		cfr_err("Invalid PSOC: Flush LUT Timer cannot be started\n");
		return;
	}

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_start_lut_timer)
		cfr_tx_ops->cfr_start_lut_timer(pdev);
}

void tgt_cfr_stop_lut_age_timer(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc) {
		cfr_err("Invalid PSOC: Flush LUT Timer cannot be stopped\n");
		return;
	}

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_stop_lut_timer)
		cfr_tx_ops->cfr_stop_lut_timer(pdev);
}

void tgt_cfr_default_ta_ra_cfg(struct wlan_objmgr_pdev *pdev,
			       struct cfr_rcc_param *rcc_param,
			       bool allvalid, uint16_t reset_cfg)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_default_ta_ra_cfg)
		cfr_tx_ops->cfr_default_ta_ra_cfg(rcc_param,
						 allvalid, reset_cfg);
}

void tgt_cfr_dump_lut_enh(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_dump_lut_enh)
		cfr_tx_ops->cfr_dump_lut_enh(pdev);
}

void tgt_cfr_rx_tlv_process(struct wlan_objmgr_pdev *pdev, void *nbuf)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_rx_tlv_process)
		cfr_tx_ops->cfr_rx_tlv_process(pdev, nbuf);
}

void tgt_cfr_update_global_cfg(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc) {
		cfr_err("Invalid PSOC:Cannot update global config.\n");
		return;
	}

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_update_global_cfg)
		cfr_tx_ops->cfr_update_global_cfg(pdev);
}

QDF_STATUS tgt_cfr_subscribe_ppdu_desc(struct wlan_objmgr_pdev *pdev,
				       bool is_subscribe)
{
	struct wlan_lmac_if_cfr_tx_ops *cfr_tx_ops = NULL;
	struct wlan_objmgr_psoc *psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc) {
		cfr_err("Invalid psoc\n");
		return QDF_STATUS_E_INVAL;
	}

	cfr_tx_ops = wlan_psoc_get_cfr_txops(psoc);

	if (cfr_tx_ops->cfr_subscribe_ppdu_desc)
		return cfr_tx_ops->cfr_subscribe_ppdu_desc(pdev,
							   is_subscribe);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_cfr_capture_count_support_set(struct wlan_objmgr_psoc *psoc,
				  uint32_t value)
{
	struct psoc_cfr *cfr_sc;

	if (!psoc) {
		cfr_err("CFR: NULL PSOC!!");
		return QDF_STATUS_E_INVAL;
	}

	cfr_sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						       WLAN_UMAC_COMP_CFR);

	if (!cfr_sc) {
		cfr_err("Failed to get CFR component priv obj!!");
		return QDF_STATUS_E_INVAL;
	}

	cfr_sc->is_cap_interval_mode_sel_support = !!value;
	cfr_debug("CFR: cap_interval_mode_sel_support is %s\n",
		  (cfr_sc->is_cap_interval_mode_sel_support) ?
		  "enabled" :
		  "disabled");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_cfr_mo_marking_support_set(struct wlan_objmgr_psoc *psoc, uint32_t value)
{
	struct psoc_cfr *cfr_sc;

	if (!psoc) {
		cfr_err("CFR: NULL PSOC!!");
		return QDF_STATUS_E_INVAL;
	}

	cfr_sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						       WLAN_UMAC_COMP_CFR);
	if (!cfr_sc) {
		cfr_err("Failed to get CFR component priv obj!!");
		return QDF_STATUS_E_INVAL;
	}

	cfr_sc->is_mo_marking_support = !!value;
	cfr_debug("CFR: mo_marking_support is %s\n",
		  (cfr_sc->is_mo_marking_support) ? "enabled" : "disabled");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
tgt_cfr_aoa_for_rcc_support_set(struct wlan_objmgr_psoc *psoc, uint32_t value)
{
	struct psoc_cfr *cfr_sc;

	if (!psoc) {
		cfr_err("CFR: NULL PSOC!!");
		return QDF_STATUS_E_INVAL;
	}

	cfr_sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						       WLAN_UMAC_COMP_CFR);

	if (!cfr_sc) {
		cfr_err("Failed to get CFR component priv obj!!");
		return QDF_STATUS_E_INVAL;
	}

	cfr_sc->is_aoa_for_rcc_support = !!value;
	cfr_debug("CFR: aoa_for_rcc_support is %s\n",
		  (cfr_sc->is_aoa_for_rcc_support) ? "enabled" : "disabled");

	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS
tgt_cfr_capture_count_support_set(struct wlan_objmgr_psoc *psoc,
				  uint32_t value)
{
	return QDF_STATUS_E_NOSUPPORT;
}

QDF_STATUS
tgt_cfr_mo_marking_support_set(struct wlan_objmgr_psoc *psoc,
			       uint32_t value)
{
	return QDF_STATUS_E_NOSUPPORT;
}

QDF_STATUS
tgt_cfr_aoa_for_rcc_support_set(struct wlan_objmgr_psoc *psoc, uint32_t value)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif
