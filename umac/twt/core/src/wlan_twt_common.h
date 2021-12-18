/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**
 *  DOC: wlan_twt_common.h
 */
#include <wlan_twt_api.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_twt_public_structs.h>

/**
 * wlan_twt_tgt_caps_get_requestor() - twt get requestor
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_tgt_caps_get_requestor(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 * wlan_twt_tgt_caps_get_responder() - twt get responder
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_tgt_caps_get_responder(struct wlan_objmgr_psoc *psoc, bool *val);

/**
 * wlan_twt_tgt_caps_get_legacy_bcast_support() - get legacy bcast support
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_tgt_caps_get_legacy_bcast_support(struct wlan_objmgr_psoc *psoc,
					   bool *val);

/**
 * wlan_twt_tgt_caps_get_bcast_req_support() - get bcast requestor support
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_tgt_caps_get_bcast_req_support(struct wlan_objmgr_psoc *psoc,
					bool *val);

/**
 * wlan_twt_tgt_caps_get_bcast_res_support() - get bcast responder support
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_tgt_caps_get_bcast_res_support(struct wlan_objmgr_psoc *psoc,
					bool *val);

/**
 * wlan_twt_tgt_caps_get_nudge_enabled() - get nudge enabled
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_tgt_caps_get_nudge_enabled(struct wlan_objmgr_psoc *psoc,
				    bool *val);

/**
 * wlan_twt_tgt_caps_get_all_twt_enabled() - get all twt enabled
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_tgt_caps_get_all_twt_enabled(struct wlan_objmgr_psoc *psoc,
				      bool *val);

/**
 * wlan_twt_tgt_caps_get_stats_enabled() - get twt stats enabled
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_tgt_caps_get_stats_enabled(struct wlan_objmgr_psoc *psoc,
				    bool *val);

/**
 * wlan_twt_tgt_caps_get_ack_supported() - get twt ack supported
 * @psoc: psoc handle
 * @val: pointer to the output variable
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_tgt_caps_get_ack_supported(struct wlan_objmgr_psoc *psoc,
				    bool *val);

/**
 * wlan_twt_check_all_twt_support() - Check if all TWT sessions capability is
 * supported or not
 * @psoc: Pointer to psoc object
 * @dialog_id: dialod id
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_twt_check_all_twt_support(struct wlan_objmgr_psoc *psoc,
					  uint32_t dialog_id);

/**
 * wlan_twt_requestor_disable() - twt requestor disable
 * @psoc: psoc handle
 * @req: twt disable request structure
 * @context: context
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_requestor_disable(struct wlan_objmgr_psoc *psoc,
			   struct twt_disable_param *req,
			   void *context);

/**
 * wlan_twt_responder_disable() - twt responder disable
 * @psoc: psoc handle
 * @req: twt disable request structure
 * @context: context
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_responder_disable(struct wlan_objmgr_psoc *psoc,
			   struct twt_disable_param *req,
			   void *context);

/**
 * wlan_twt_requestor_enable() - twt requestor enable
 * @psoc: psoc handle
 * @req: twt enable request structure
 * @context: context
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_requestor_enable(struct wlan_objmgr_psoc *psoc,
			  struct twt_enable_param *req,
			  void *context);

/**
 * wlan_twt_responder_enable() - twt responder enable
 * @psoc: psoc handle
 * @req: twt enable request structure
 * @context: context
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_responder_enable(struct wlan_objmgr_psoc *psoc,
			  struct twt_enable_param *req,
			  void *context);

/**
 * wlan_twt_set_peer_capabilities() - set twt peer capabilities
 * @psoc: psoc handle
 * @peer_mac: peer mac address
 * @peer_cap: TWT peer capability bitmap. Refer enum
 * wlan_twt_capabilities for representation.
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_set_peer_capabilities(struct wlan_objmgr_psoc *psoc,
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
wlan_twt_get_peer_capabilities(struct wlan_objmgr_psoc *psoc,
			       struct qdf_mac_addr *peer_mac,
			       uint8_t *peer_cap);

/**
 * wlan_twt_enable_event_handler() - twt enable handler
 * @psoc: psoc handle
 * @event: twt enable event structure
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_enable_event_handler(struct wlan_objmgr_psoc *psoc,
			      struct twt_enable_complete_event_param *event);

/**
 * wlan_twt_disable_event_handler() - twt disable handler
 * @psoc: psoc handle
 * @event: twt disable event structure
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_twt_disable_event_handler(struct wlan_objmgr_psoc *psoc,
			       struct twt_disable_complete_event_param *event);
