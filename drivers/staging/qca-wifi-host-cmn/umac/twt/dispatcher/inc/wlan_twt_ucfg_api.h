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
 * DOC: wlan_twt_ucfg_api.h
 * This file provides TWT ucfg apis.
 */
#ifndef _WLAN_TWT_UCFG_API_H_
#define _WLAN_TWT_UCFG_API_H_

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_twt_public_structs.h>

/**
 * ucfg_twt_get_requestor() - twt get requestor
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_get_requestor(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 * ucfg_twt_get_responder() - twt get responder
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_get_responder(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 * ucfg_twt_get_legacy_bcast_twt_support() - get legacy bcast support
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_get_legacy_bcast_twt_support(struct wlan_objmgr_psoc *psoc,
				      bool *val);

/**
 * ucfg_twt_get_twt_bcast_req_support() - get bcast requestor support
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_get_twt_bcast_req_support(struct wlan_objmgr_psoc *psoc,
				   bool *val);
/**
 * ucfg_twt_get_twt_bcast_res_support() - get bcast responder support
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_get_twt_bcast_res_support(struct wlan_objmgr_psoc *psoc,
				   bool *val);

/**
 * ucfg_twt_get_twt_nudge_enabled() - get twt nudge enabled
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_get_twt_nudge_enabled(struct wlan_objmgr_psoc *psoc,
			       bool *val);

/**
 * ucfg_twt_get_all_twt_enabled() - get all twt enabled
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_get_all_twt_enabled(struct wlan_objmgr_psoc *psoc,
			     bool *val);

/**
 * ucfg_twt_check_all_twt_support() - Check if all TWT sessions capability is
 * supported or not
 * @psoc: Pointer to psoc object
 * @dialog_id: dialod id
 *
 * Return: QDF Status
 */
QDF_STATUS
ucfg_twt_check_all_twt_support(struct wlan_objmgr_psoc *psoc,
			       uint32_t dialog_id);

/**
 * ucfg_twt_get_twt_stats_enabled() - get twt stats enabled
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_get_twt_stats_enabled(struct wlan_objmgr_psoc *psoc,
			       bool *val);

/**
 * ucfg_twt_get_twt_ack_supported() - get twt ack supported
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_get_twt_ack_supported(struct wlan_objmgr_psoc *psoc,
			       bool *val);

/**
 * ucfg_twt_requestor_disable() - twt requestor disable
 * @psoc: psoc handle
 * @req: twt disable request structure
 * @context: context
 *
 * Following fields in request structure @twt_enable_param shall be
 * filled by the user space/upper layer. if any field is not applicable
 * then set it to 0.
 *      pdev_id;
 *      mbss_support;
 *      default_slot_size;
 *      congestion_thresh_setup;
 *      congestion_thresh_teardown;
 *      congestion_thresh_critical;
 *      interference_thresh_teardown;
 *      interference_thresh_setup;
 *      min_no_sta_setup;
 *      min_no_sta_teardown;
 *      no_of_bcast_mcast_slots;
 *      min_no_twt_slots;
 *      max_no_sta_twt;
 *      mode_check_interval;
 *      add_sta_slot_interval;
 *      remove_sta_slot_interval;
 *      ext_conf_present;
 *      b_twt_legacy_mbss_enable:1,
 *      b_twt_ax_mbss_enable:1;
 *
 * Following fields in request structure @twt_enable_param shall be
 * filled by the UMAC TWT Component itself
 *      sta_cong_timer_ms;
 *      twt_role;
 *      twt_oper;
 *      b_twt_enable;
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_requestor_disable(struct wlan_objmgr_psoc *psoc,
			   struct twt_disable_param *req,
			   void *context);

/**
 * ucfg_twt_responder_disable() - twt responder disable
 * @psoc: psoc handle
 * @req: twt disable request structure
 * @context: context
 *
 * Following fields in request structure @twt_enable_param shall be
 * filled by the user space/upper layer. if any field is not applicable
 * then set it to 0.
 *      pdev_id;
 *      mbss_support;
 *      default_slot_size;
 *      congestion_thresh_setup;
 *      congestion_thresh_teardown;
 *      congestion_thresh_critical;
 *      interference_thresh_teardown;
 *      interference_thresh_setup;
 *      min_no_sta_setup;
 *      min_no_sta_teardown;
 *      no_of_bcast_mcast_slots;
 *      min_no_twt_slots;
 *      max_no_sta_twt;
 *      mode_check_interval;
 *      add_sta_slot_interval;
 *      remove_sta_slot_interval;
 *      ext_conf_present;
 *      b_twt_legacy_mbss_enable:1,
 *      b_twt_ax_mbss_enable:1;
 *
 * Following fields in request structure @twt_enable_param shall be
 * filled by the UMAC TWT Component itself
 *      sta_cong_timer_ms;
 *      twt_role;
 *      twt_oper;
 *      b_twt_enable;
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_responder_disable(struct wlan_objmgr_psoc *psoc,
			   struct twt_disable_param *req,
			   void *context);

/**
 * ucfg_twt_requestor_enable() - twt requestor enable
 * @psoc: psoc handle
 * @req: twt enable request structure
 * @context: context
 *
 * Following fields in request structure @twt_enable_param shall be
 * filled by the user space/upper layer. if any field is not applicable
 * then set it to 0.
 *      pdev_id;
 *      mbss_support;
 *      default_slot_size;
 *      congestion_thresh_setup;
 *      congestion_thresh_teardown;
 *      congestion_thresh_critical;
 *      interference_thresh_teardown;
 *      interference_thresh_setup;
 *      min_no_sta_setup;
 *      min_no_sta_teardown;
 *      no_of_bcast_mcast_slots;
 *      min_no_twt_slots;
 *      max_no_sta_twt;
 *      mode_check_interval;
 *      add_sta_slot_interval;
 *      remove_sta_slot_interval;
 *      ext_conf_present;
 *      b_twt_legacy_mbss_enable:1,
 *      b_twt_ax_mbss_enable:1;
 *
 * Following fields in request structure @twt_enable_param shall be
 * filled by the UMAC TWT Component itself
 *      sta_cong_timer_ms;
 *      twt_role;
 *      twt_oper;
 *      b_twt_enable;
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_requestor_enable(struct wlan_objmgr_psoc *psoc,
			  struct twt_enable_param *req,
			  void *context);

/**
 * ucfg_twt_responder_enable() - twt responder enable
 * @psoc: psoc handle
 * @req: twt enable request structure
 * @context: context
 *
 * Following fields in request structure @twt_enable_param shall be
 * filled by the user space/upper layer. if any field is not applicable
 * then set it to 0.
 *      pdev_id;
 *      mbss_support;
 *      default_slot_size;
 *      congestion_thresh_setup;
 *      congestion_thresh_teardown;
 *      congestion_thresh_critical;
 *      interference_thresh_teardown;
 *      interference_thresh_setup;
 *      min_no_sta_setup;
 *      min_no_sta_teardown;
 *      no_of_bcast_mcast_slots;
 *      min_no_twt_slots;
 *      max_no_sta_twt;
 *      mode_check_interval;
 *      add_sta_slot_interval;
 *      remove_sta_slot_interval;
 *      ext_conf_present;
 *      b_twt_legacy_mbss_enable:1,
 *      b_twt_ax_mbss_enable:1;
 *
 * Following fields in request structure @twt_enable_param shall be
 * filled by the UMAC TWT Component itself
 *      sta_cong_timer_ms;
 *      twt_role;
 *      twt_oper;
 *      b_twt_enable;
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_responder_enable(struct wlan_objmgr_psoc *psoc,
			  struct twt_enable_param *req,
			  void *context);

/**
 * ucfg_twt_set_peer_capabilities() - set twt peer capabilities
 * @psoc: psoc handle
 * @peer_mac: peer mac address
 * @peer_cap: TWT peer capability bitmap. Refer enum
 * wlan_twt_capabilities for representation.
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_set_peer_capabilities(struct wlan_objmgr_psoc *psoc,
			       struct qdf_mac_addr *peer_mac,
			       uint8_t peer_cap);

/**
 * ucfg_twt_get_peer_capabilities() - get twt peer capabilities
 * @psoc: psoc handle
 * @peer_mac: peer mac address
 * @peer_cap: Pointer to output variable to hold TWT peer capability bitmap.
 * Refer enum wlan_twt_capabilities for representation.
 *
 * return: QDF_STATUS
 */
QDF_STATUS
ucfg_twt_get_peer_capabilities(struct wlan_objmgr_psoc *psoc,
			       struct qdf_mac_addr *peer_mac,
			       uint8_t *peer_cap);
#endif /* _WLAN_TWT_UCFG_API_H_ */
