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

#ifndef _WLAN_CFR_TGT_API_H_
#define _WLAN_CFR_TGT_API_H_

#include <wlan_objmgr_peer_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_cmn.h>
#include <qdf_types.h>

/* tgt layer has APIs in application, to access functions in target
 * through tx_ops.
 */

/**
 * tgt_cfr_init_pdev() - API that registers CFR to handlers.
 * @pdev: pointer to pdev_object
 *
 * Return: success/failure of init
 */
QDF_STATUS tgt_cfr_init_pdev(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_cfr_deinit_pdev() - API that de-registers CFR to handlers.
 * @pdev: pointer to pdev_object
 *
 * Return: success/failure of de-init
 */
QDF_STATUS tgt_cfr_deinit_pdev(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_cfr_get_target_type() - API to determine target type.
 * @psoc: pointer to psoc_object
 *
 * Return: enum value of target type
 */
int tgt_cfr_get_target_type(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_cfr_start_capture() - API to start cfr capture on a peer.
 * @pdev: pointer to pdev_object
 * @peer: pointer to peer_object
 * @cfr_params: pointer to config cfr_params
 *
 * Return: success/failure of start capture
 */
int tgt_cfr_start_capture(struct wlan_objmgr_pdev *pdev,
			  struct wlan_objmgr_peer *peer,
			  struct cfr_capture_params *cfr_params);

/**
 * tgt_cfr_stop_capture() - API to stop cfr capture on a peer.
 * @pdev: pointer to pdev_object
 * @peer: pointer to peer_object
 *
 * Return: success/failure of stop capture
 */
int tgt_cfr_stop_capture(struct wlan_objmgr_pdev *pdev,
			 struct wlan_objmgr_peer *peer);

/**
 * tgt_cfr_validate_period() - API to validate cfr period configured by user
 * @psoc: pointer to the psoc object
 * @period: period value to validate
 *
 * Return: success/failure of periodicity validation
 */
int tgt_cfr_validate_period(struct wlan_objmgr_psoc *psoc, u_int32_t period);

/**
 * tgt_cfr_enable_cfr_timer() - API to enable cfr timer
 * @pdev: pointer to pdev_object
 * @cfr_timer: Amount of time this timer has to run. If 0, it disables timer.
 *
 * Return: success/failure of timer enable
 */
int
tgt_cfr_enable_cfr_timer(struct wlan_objmgr_pdev *pdev, uint32_t cfr_timer);

/**
 * tgt_cfr_support_set() - API to set cfr support
 * @psoc: pointer to psoc_object
 * @value: value to be set
 */
void tgt_cfr_support_set(struct wlan_objmgr_psoc *psoc, uint32_t value);

/**
 * tgt_cfr_capture_count_support_set() - API to set capture_count support
 * @psoc: pointer to psoc_object
 * @value: value to be set
 *
 * Return: success/failure
 */
QDF_STATUS
tgt_cfr_capture_count_support_set(struct wlan_objmgr_psoc *psoc,
				  uint32_t value);

/**
 * tgt_cfr_mo_marking_support_set() - API to set MO marking support
 * @psoc: pointer to psoc_object
 * @value: value to be set
 *
 * Return: success/failure
 */
QDF_STATUS
tgt_cfr_mo_marking_support_set(struct wlan_objmgr_psoc *psoc, uint32_t value);

/**
 * tgt_cfr_aoa_for_rcc_support_set() - API to set AoA for RCC support
 * @psoc: pointer to psoc_object
 * @value: value to be set
 *
 * Return: success/failure
 */
QDF_STATUS
tgt_cfr_aoa_for_rcc_support_set(struct wlan_objmgr_psoc *psoc, uint32_t value);

/**
 * tgt_cfr_info_send() - API to send cfr info
 * @pdev: pointer to pdev_object
 * @head: pointer to cfr info head
 * @hlen: head len
 * @data: pointer to cfr info data
 * @dlen: data len
 * @tail: pointer to cfr info tail
 * @tlen: tail len
 *
 * Return: success/failure of cfr info send
 */
uint32_t tgt_cfr_info_send(struct wlan_objmgr_pdev *pdev, void *head,
			   size_t hlen, void *data, size_t dlen, void *tail,
			   size_t tlen);

#ifdef WLAN_ENH_CFR_ENABLE
/**
 * tgt_cfr_config_rcc() - API to set RCC
 * @pdev: pointer to pdev_object
 * @rcc_param: rcc configurations
 *
 * Return: succcess / failure
 */
QDF_STATUS tgt_cfr_config_rcc(struct wlan_objmgr_pdev *pdev,
			      struct cfr_rcc_param *rcc_param);

/**
 * tgt_cfr_start_lut_age_timer() - API to start timer to flush aged out LUT
 * entries
 * @pdev: pointer to pdev_object
 *
 * Return: None
 */
void tgt_cfr_start_lut_age_timer(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_cfr_stop_lut_age_timer() - API to stop timer to flush aged out LUT
 * entries
 * @pdev: pointer to pdev_object
 *
 * Return: None
 */
void tgt_cfr_stop_lut_age_timer(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_cfr_default_ta_ra_cfg() - API to configure default values in TA_RA mode
 * entries
 * @pdev: pointer to pdev_object
 *
 * Return: none
 */
void tgt_cfr_default_ta_ra_cfg(struct wlan_objmgr_pdev *pdev,
			       struct cfr_rcc_param *rcc_param,
			       bool allvalid, uint16_t reset_cfg);

/**
 * tgt_cfr_dump_lut_enh() - Print all LUT entries
 * @pdev: pointer to pdev_object
 */
void tgt_cfr_dump_lut_enh(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_cfr_rx_tlv_process() - Process PPDU status TLVs
 * @pdev_obj: pointer to pdev_object
 * @nbuf: pointer to cdp_rx_indication_ppdu
 */
void tgt_cfr_rx_tlv_process(struct wlan_objmgr_pdev *pdev, void *nbuf);

/**
 * tgt_cfr_update_global_cfg() - Update global config after successful commit
 * @pdev: pointer to pdev_object
 */
void tgt_cfr_update_global_cfg(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_cfr_subscribe_ppdu_desc() - Target interface to
 * subscribe/unsubscribe WDI PPDU desc event
 * @pdev: pointer to pdev_object
 * @is_subscribe: subscribe or unsubscribei
 *
 * return QDF status
 */
QDF_STATUS tgt_cfr_subscribe_ppdu_desc(struct wlan_objmgr_pdev *pdev,
				       bool is_subscribe);
#endif
#endif
