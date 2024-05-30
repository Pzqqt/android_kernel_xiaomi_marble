/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * DOC: contains MLO manager public file containing STA functionality
 */
#ifndef _WLAN_MLO_MGR_STA_H_
#define _WLAN_MLO_MGR_STA_H_

#include <wlan_cm_ucfg_api.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_mlo_mgr_cmn.h>
#ifdef WLAN_FEATURE_11BE_MLO
#include <wlan_mlo_mgr_public_structs.h>

/**
 * mlo_connect - Start the connection process
 *
 * @vdev: pointer to vdev
 * @req: connection request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlo_connect(struct wlan_objmgr_vdev *vdev,
		       struct wlan_cm_connect_req *req);

/**
 * mlo_sta_link_connect_notify - Called by connection manager to notify the
 * STA link connect is complete
 *
 * @vdev: pointer to vdev
 * @mlo_ie: MLO information element
 *
 * Connection manager will notify the MLO manager when the link has started
 * and MLO manager will start the subsequent connections, if necessary
 *
 * Return: none
 */
void
mlo_sta_link_connect_notify(struct wlan_objmgr_vdev *vdev,
			    struct wlan_cm_connect_resp *rsp);

/**
 * mlo_disconnect - Start the disconnection process
 *
 * @vdev: pointer to vdev
 * @source: source of the request (can be connect or disconnect request)
 * @reason_code: reason for disconnect
 * @bssid: BSSID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlo_disconnect(struct wlan_objmgr_vdev *vdev,
			  enum wlan_cm_source source,
			  enum wlan_reason_code reason_code,
			  struct qdf_mac_addr *bssid);

/**
 * mlo_sync_disconnect - Start the sync disconnection process
 *
 * @vdev: pointer to vdev
 * @source: source of the request (can be connect or disconnect request)
 * @reason_code: reason for disconnect
 * @bssid: BSSID
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlo_sync_disconnect(struct wlan_objmgr_vdev *vdev,
			       enum wlan_cm_source source,
			       enum wlan_reason_code reason_code,
			       struct qdf_mac_addr *bssid);

/**
 * mlo_sta_link_disconn_notify - Notifies that STA link disconnect completion
 *
 * @vdev: pointer to vdev
 * @resp: disconnect resp
 *
 * Return: none
 */
void mlo_sta_link_disconn_notify(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_discon_rsp *resp);

/**
 * mlo_is_mld_sta - Check if MLD associated with the vdev is a station
 *
 * @vdev: pointer to vdev
 *
 * Return: true if MLD is a station, false otherwise
 */
bool mlo_is_mld_sta(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_mlo_is_mld_disconnected - Check whether MLD is disconnected
 *
 * @vdev: pointer to vdev
 *
 * Return: true if mld is disconnected, false otherwise
 */
bool ucfg_mlo_is_mld_disconnected(struct wlan_objmgr_vdev *vdev);

#ifndef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
/**
 * ucfg_mlo_is_mld_connected - Check whether MLD is connected
 *
 * @vdev: pointer to vdev
 *
 * Return: true if mld is connected, false otherwise
 */
bool ucfg_mlo_is_mld_connected(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_mlo_mld_clear_mlo_cap - Clear MLO cap for all vdevs in MLD
 *
 * @vdev: pointer to vdev
 *
 * Return: None
 */
void ucfg_mlo_mld_clear_mlo_cap(struct wlan_objmgr_vdev *vdev);
#endif

/*
 * ucfg_mlo_get_assoc_link_vdev - API to get assoc link vdev
 *
 * @mlo_dev_ctx: mlo dev ctx
 *
 * Return: MLD assoc link vdev
 */
struct wlan_objmgr_vdev *
ucfg_mlo_get_assoc_link_vdev(struct wlan_objmgr_vdev *vdev);

/*
 * wlan_mlo_get_assoc_link_vdev - API to get assoc link vdev
 *
 * @mlo_dev_ctx: mlo dev ctx
 *
 * Return: MLD assoc link vdev
 */
struct wlan_objmgr_vdev *
wlan_mlo_get_assoc_link_vdev(struct wlan_objmgr_vdev *vdev);

/*
 * mlo_update_connected_links_bmap: update connected links bitmap
 *
 * @mlo_dev_ctx: mlo dev context ptr
 * @ml_partner_info: ml parnter info ptr
 *
 * Return: none
 */
void
mlo_update_connected_links_bmap(struct wlan_mlo_dev_context *mlo_dev_ctx,
				struct mlo_partner_info ml_parnter_info);

/*
 * API to have operation on ml vdevs
 */
typedef void (*mlo_vdev_op_handler)(struct wlan_objmgr_vdev *vdev,
				    void *arg);

/*
 * mlo_iterate_connected_vdev_list: Iterate on connected ML links
 *
 * @vdev: vdev object
 * @handler: the handler will be called for each object in ML list
 * @arg: argumet to be passed to handler
 *
 * Return: none
 */
static inline
void mlo_iterate_connected_vdev_list(struct wlan_objmgr_vdev *vdev,
				     mlo_vdev_op_handler handler,
				     void *arg)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = NULL;
	uint8_t i = 0;

	if (!mlo_dev_ctx || !(wlan_vdev_mlme_is_mlo_vdev(vdev)))
		return;

	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx)
		return;

	for (i =  0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;
		if (qdf_test_bit(i, sta_ctx->wlan_connected_links)) {
			if (handler)
				handler(mlo_dev_ctx->wlan_vdev_list[i], arg);
		}
	}
}

/*
 * mlo_update_connect_req_links: update connect req links index
 *
 * @vdev: vdev object
 * @value: set/clear the bit
 *
 * Return: none
 */
static inline void
mlo_update_connect_req_links(struct wlan_objmgr_vdev *vdev, uint8_t value)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = NULL;
	uint8_t i = 0;

	if (!mlo_dev_ctx)
		return;

	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx)
		return;

	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (vdev == mlo_dev_ctx->wlan_vdev_list[i]) {
			if (value)
				qdf_set_bit(i, sta_ctx->wlan_connect_req_links);
			else
				qdf_clear_bit(
					i, sta_ctx->wlan_connect_req_links);
		}
	}
}

/*
 * mlo_is_vdev_connect_req_link: API to check if vdev is in active connection
 *
 * @vdev: vdev object
 *
 * Return: true is vdev is participating in active connect else false
 */
static inline bool
mlo_is_vdev_connect_req_link(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = NULL;

	if (!mlo_dev_ctx)
		return false;

	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx)
		return false;

	if (qdf_test_bit(
		mlo_get_link_vdev_ix(vdev->mlo_dev_ctx, vdev),
		sta_ctx->wlan_connect_req_links))
		return true;

	return false;
}

/*
 * mlo_clear_connect_req_links: clear connect req links bitmap
 *
 * @vdev: vdev object
 *
 * Return: none
 */
static inline
void mlo_clear_connect_req_links_bmap(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = NULL;

	if (!mlo_dev_ctx)
		return;

	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx)
		return;

	qdf_mem_zero(sta_ctx->wlan_connect_req_links,
		     sizeof(sta_ctx->wlan_connect_req_links));
}

/*
 * mlo_update_connected_links: update connected links index
 *
 * @vdev: vdev object
 * @value: set/clear the bit
 *
 * Return: none
 */
static inline void
mlo_update_connected_links(struct wlan_objmgr_vdev *vdev, uint8_t value)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = NULL;
	uint8_t i = 0;

	if (!mlo_dev_ctx)
		return;

	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx)
		return;

	for (i = 0; i < WLAN_UMAC_MLO_MAX_VDEVS; i++) {
		if (!mlo_dev_ctx->wlan_vdev_list[i])
			continue;

		if (vdev == mlo_dev_ctx->wlan_vdev_list[i]) {
			if (value)
				qdf_set_bit(i, sta_ctx->wlan_connected_links);
			else
				qdf_clear_bit(i, sta_ctx->wlan_connected_links);
		}
	}
}

/*
 * mlo_clear_connected_links: clear connected links bitmap
 *
 * @vdev: vdev object
 *
 * Return: none
 */
static inline
void mlo_clear_connected_links_bmap(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_mlo_dev_context *mlo_dev_ctx = vdev->mlo_dev_ctx;
	struct wlan_mlo_sta *sta_ctx = NULL;

	if (!mlo_dev_ctx)
		return;

	sta_ctx = mlo_dev_ctx->sta_ctx;
	if (!sta_ctx)
		return;

	qdf_mem_zero(sta_ctx->wlan_connected_links,
		     sizeof(sta_ctx->wlan_connected_links));

}

#ifndef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
/*
 * mlo_get_ml_vdev_by_mac: get ml vdev from mac
 *
 * @vdev: vdev object
 * @macaddr: mac of vdev to be returned
 *
 * Return: vdev object if found else NULL
 */
struct wlan_objmgr_vdev *
mlo_get_ml_vdev_by_mac(struct wlan_objmgr_vdev *vdev,
		       struct qdf_mac_addr *macaddr);
#endif

/*
 * mlo_get_chan_freq_by_bssid - Get channel freq by bssid
 *
 * @pdev: pdev pointer
 * @bssid: link mac address
 *
 * Return: chan frequency
 */
qdf_freq_t
mlo_get_chan_freq_by_bssid(struct wlan_objmgr_pdev *pdev,
			   struct qdf_mac_addr *bssid);

/**
 * mlo_get_assoc_rsp - Get Assoc response from mlo manager
 *
 * @vdev: vdev obj mgr
 * @assoc_rsp_frame: association response frame ptr
 *
 * Return: none
 */
void mlo_get_assoc_rsp(struct wlan_objmgr_vdev *vdev,
		       struct element_info *assoc_rsp_frame);

/**
 * mlo_sta_save_quiet_status - save quiet status for given link of mlo station
 * @mlo_dev_ctx: mlo context
 * @link_id: link id
 * @quiet_status: True if quiet starts. False if quiet stopps.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlo_sta_save_quiet_status(struct wlan_mlo_dev_context *mlo_dev_ctx,
				     uint8_t link_id,
				     bool quiet_status);

/**
 * mlo_is_sta_in_quiet_status - is the link of given mlo sta is in quiet status
 * @mlo_dev_ctx: mlo context
 * @link_id: link id
 *
 * Return: true if the link of given mlo sta is in quiet status
 */
bool mlo_is_sta_in_quiet_status(struct wlan_mlo_dev_context *mlo_dev_ctx,
				uint8_t link_id);

/**
 * mlo_is_sta_inactivity_allowed_with_quiet() - Is link OK to force inactivity
 *                                              based on current quiet status
 *                                              of mlo connection
 * @psoc: pointer to psoc
 * @vdev_id_list: vdev id list
 * @num_mlo: number of mlo vdev
 * @mlo_idx: list of index of vdev_id_list if it is vdev id of mlo vdev
 * @affected_links: number of links to be set inactivity
 * @affected_list: list of vdev id to be set inactivity
 *
 * Return: true if any link not in mlo_vdev_list is not in quiet mode
 */
bool mlo_is_sta_inactivity_allowed_with_quiet(struct wlan_objmgr_psoc *psoc,
					      uint8_t *vdev_id_list,
					      uint8_t num_mlo, uint8_t *mlo_idx,
					      uint8_t affected_links,
					      uint8_t *affected_list);

/**
 * mlo_is_sta_csa_synced - Is mlo sta csa parameters are synced or not
 *
 * @mlo_dev_ctx: mlo context
 * @link_id: link id
 *
 * Return: true if mlo sta csa parameters of given link id is synced
 */
bool mlo_is_sta_csa_synced(struct wlan_mlo_dev_context *mlo_dev_ctx,
			   uint8_t link_id);

/**
 * mlo_sta_csa_save_params - Save csa parameters for mlo station
 * @mlo_dev_ctx: mlo context
 * @link_id: link id
 * @csa_param: csa parameters to be saved
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlo_sta_csa_save_params(struct wlan_mlo_dev_context *mlo_dev_ctx,
				   uint8_t link_id,
				   struct csa_offload_params *csa_param);

/**
 * mlo_sta_up_active_notify - mlo sta up active notify
 * @vdev: vdev obj mgr
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlo_sta_up_active_notify(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_is_sta_csa_param_handled - Is given csa_param handled or not
 * @vdev: vdev obj mgr
 * @csa_param: csa parameters to be checked
 *
 * Return: true if given csa parameters is handled
 */
bool mlo_is_sta_csa_param_handled(struct wlan_objmgr_vdev *vdev,
				  struct csa_offload_params *csa_param);

/**
 * mlo_internal_disconnect_links - Internal disconnect for connection manager
 *
 * @vdev: vdev obj mgr
 *
 * Return: none
 */
void mlo_internal_disconnect_links(struct wlan_objmgr_vdev *vdev);
#else
static inline
QDF_STATUS mlo_connect(struct wlan_objmgr_vdev *vdev,
		       struct wlan_cm_connect_req *req)
{
	return wlan_cm_start_connect(vdev, req);
}

static inline
void mlo_sta_link_connect_notify(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_connect_resp *rsp)
{ }

static inline
QDF_STATUS mlo_disconnect(struct wlan_objmgr_vdev *vdev,
			  enum wlan_cm_source source,
			  enum wlan_reason_code reason_code,
			  struct qdf_mac_addr *bssid)
{
	QDF_STATUS status;

	status = wlan_cm_disconnect(vdev, source,
				    reason_code,
				    bssid);
	return status;
}

static inline
QDF_STATUS mlo_sync_disconnect(struct wlan_objmgr_vdev *vdev,
			       enum wlan_cm_source source,
			       enum wlan_reason_code reason_code,
			       struct qdf_mac_addr *bssid)
{
	return wlan_cm_disconnect_sync(vdev, CM_OSIF_DISCONNECT,
				       reason_code);
}

static inline
void mlo_sta_link_disconn_notify(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_discon_rsp *resp)
{ }

#ifndef WLAN_FEATURE_11BE_MLO_ADV_FEATURE
static inline
bool ucfg_mlo_is_mld_connected(struct wlan_objmgr_vdev *vdev)
{
	return true;
}

static inline
bool ucfg_mlo_is_mld_disconnected(struct wlan_objmgr_vdev *vdev)
{
	return true;
}
#endif

static inline
bool mlo_is_mld_sta(struct wlan_objmgr_vdev *vdev)
{
	return false;
}

static inline
struct wlan_objmgr_vdev *
ucfg_mlo_get_assoc_link_vdev(struct wlan_objmgr_vdev *vdev)
{
	return vdev;
}

static inline void
mlo_update_connect_req_links(struct wlan_objmgr_vdev *vdev, uint8_t value)
{ }

static inline void
mlo_update_connected_links_bmap(struct wlan_mlo_dev_context *mlo_dev_ctx,
				struct mlo_partner_info ml_parnter_info)
{ }

static inline bool
mlo_is_vdev_connect_req_link(struct wlan_objmgr_vdev *vdev)
{
	return true;
}

static inline void
mlo_update_connected_links(struct wlan_objmgr_vdev *vdev, uint8_t value)
{ }

static inline void
mlo_clear_connect_req_links_bmap(struct wlan_objmgr_vdev *vdev)
{ }

static inline void
mlo_clear_connected_links_bmap(struct wlan_objmgr_vdev *vdev)
{ }

static inline struct wlan_objmgr_vdev *
mlo_get_ml_vdev_by_mac(struct wlan_objmgr_vdev *vdev,
		       struct qdf_mac_addr *macaddr)
{
	return vdev;
}

static inline qdf_freq_t
mlo_get_chan_freq_by_bssid(struct wlan_objmgr_pdev *pdev,
			   struct qdf_mac_addr *bssid)
{
	return 0;
}

static inline void
mlo_get_assoc_rsp(struct wlan_objmgr_vdev *vdev,
		  struct element_info *assoc_rsp_frame)
{
}

static inline bool
mlo_is_sta_csa_param_handled(struct wlan_objmgr_vdev *vdev,
			     struct csa_offload_params *csa_param)
{
	return false;
}

static inline void mlo_internal_disconnect_links(struct wlan_objmgr_vdev *vdev)
{
}
#endif
#endif
