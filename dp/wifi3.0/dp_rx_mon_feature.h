/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

#ifndef _DP_RX_MON_FEATURE_H_
#define _DP_RX_MON_FEATURE_H_

#ifdef WLAN_RX_PKT_CAPTURE_ENH
/*
 * dp_rx_populate_cdp_indication_mpdu_info() - Populate cdp rx indication
 * MPDU info structure
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from monitor status ring
 * @cdp_mpdu_info: cdp rx indication MPDU info structure
 * @user: user ID
 *
 * Return: none
 */
void
dp_rx_populate_cdp_indication_mpdu_info(
	struct dp_pdev *pdev, struct hal_rx_ppdu_info *ppdu_info,
	struct cdp_rx_indication_mpdu_info *cdp_mpdu_info,
	uint32_t user);

/*
 * dp_rx_handle_enh_capture() - Deliver Rx enhanced capture data
 * @pdev: pdev ctx
 * @ppdu_info: ppdu info structure from monitor status ring
 *
 * Return: QDF status
 */
QDF_STATUS
dp_rx_handle_enh_capture(struct dp_soc *soc, struct dp_pdev *pdev,
			 struct hal_rx_ppdu_info *ppdu_info);

/*
 * dp_rx_mon_enh_capture_process() - Rx enhanced capture mode
 *	processing.
 * @pdev: pdev structure
 * @tlv_status: processed TLV status
 * @status_nbuf: monitor status ring buffer
 * @ppdu_info: ppdu info structure from monitor status ring
 * @nbuf_used: nbuf need a clone
 * @rx_enh_capture_mode: Rx enhanced capture mode
 *
 * Return: none
 */
void
dp_rx_mon_enh_capture_process(struct dp_pdev *pdev, uint32_t tlv_status,
			      qdf_nbuf_t status_nbuf,
			      struct hal_rx_ppdu_info *ppdu_info,
			      bool *nbuf_used,
			      uint32_t rx_enh_capture_mode);

/*
 * dp_config_enh_rx_capture()- API to enable/disable enhanced rx capture
 * @pdev_handle: DP_PDEV handle
 * @val: user provided value
 *
 * Return: 0 for success. nonzero for failure.
 */
QDF_STATUS
dp_config_enh_rx_capture(struct cdp_pdev *pdev_handle, int val);
#endif
#endif
