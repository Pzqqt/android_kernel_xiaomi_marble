/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#include "osdep.h"
#include "wmi.h"
#include "wmi_unified_priv.h"

/**
 * extract_dcs_interference_type_tlv() - extract dcs interference type
 * from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param param: Pointer to hold dcs interference param
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_dcs_interference_type_tlv(
		wmi_unified_t wmi_handle,
		void *evt_buf, struct wlan_host_dcs_interference_param *param)
{
	WMI_DCS_INTERFERENCE_EVENTID_param_tlvs *param_buf;

	param_buf = (WMI_DCS_INTERFERENCE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid dcs interference event buffer");
		return QDF_STATUS_E_INVAL;
	}

	if (!param_buf->fixed_param) {
		wmi_err("Invalid fixed param");
		return QDF_STATUS_E_INVAL;
	}

	param->interference_type = param_buf->fixed_param->interference_type;
	/* Just support tlv currently */
	param->pdev_id = wmi_handle->ops->convert_target_pdev_id_to_host(
					wmi_handle,
					param_buf->fixed_param->pdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * extract_dcs_im_tgt_stats_tlv() - extract dcs im target stats from event
 * @wmi_handle: wmi handle
 * @param evt_buf: pointer to event buffer
 * @param wlan_stat: Pointer to hold wlan stats
 *
 * Return: 0 for success or error code
 */
static QDF_STATUS extract_dcs_im_tgt_stats_tlv(
			wmi_unified_t wmi_handle,
			void *evt_buf,
			struct wlan_host_dcs_im_tgt_stats *wlan_stat)
{
	WMI_DCS_INTERFERENCE_EVENTID_param_tlvs *param_buf;
	wlan_dcs_im_tgt_stats_t *ev;

	param_buf = (WMI_DCS_INTERFERENCE_EVENTID_param_tlvs *)evt_buf;
	if (!param_buf) {
		wmi_err("Invalid dcs interference event buffer");
		return QDF_STATUS_E_INVAL;
	}

	ev = param_buf->wlan_stat;
	if (!ev) {
		wmi_err("Invalid wlan stat");
		return QDF_STATUS_E_INVAL;
	}

	wlan_stat->reg_tsf32 = ev->reg_tsf32;
	wlan_stat->last_ack_rssi = ev->last_ack_rssi;
	wlan_stat->tx_waste_time = ev->tx_waste_time;
	wlan_stat->rx_time = ev->rx_time;
	wlan_stat->phyerr_cnt = ev->phyerr_cnt;
	wlan_stat->mib_stats.listen_time = ev->listen_time;
	wlan_stat->mib_stats.reg_tx_frame_cnt = ev->reg_tx_frame_cnt;
	wlan_stat->mib_stats.reg_rx_frame_cnt = ev->reg_rx_frame_cnt;
	wlan_stat->mib_stats.reg_rxclr_cnt = ev->reg_rxclr_cnt;
	wlan_stat->mib_stats.reg_cycle_cnt = ev->reg_cycle_cnt;
	wlan_stat->mib_stats.reg_rxclr_ext_cnt = ev->reg_rxclr_ext_cnt;
	wlan_stat->mib_stats.reg_ofdm_phyerr_cnt = ev->reg_ofdm_phyerr_cnt;
	wlan_stat->mib_stats.reg_cck_phyerr_cnt = ev->reg_cck_phyerr_cnt;
	wlan_stat->chan_nf = ev->chan_nf;
	wlan_stat->my_bss_rx_cycle_count = ev->my_bss_rx_cycle_count;

	return QDF_STATUS_SUCCESS;
}

void wmi_dcs_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;

	ops->extract_dcs_interference_type = extract_dcs_interference_type_tlv;
	ops->extract_dcs_im_tgt_stats = extract_dcs_im_tgt_stats_tlv;
}

