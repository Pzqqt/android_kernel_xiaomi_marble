/*
 * Copyright (c) 2012-2015,2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: osif_cm_roam_rsp.c
 *
 * This file maintains definitaions of roam response apis.
 */

#include <linux/version.h>
#include <linux/nl80211.h>
#include <net/cfg80211.h>
#include <wlan_osif_priv.h>
#include "osif_cm_rsp.h"
#include <osif_cm_util.h>
#include <wlan_cfg80211.h>
#include <wlan_cfg80211_scan.h>
#ifdef CONN_MGR_ADV_FEATURE
#include "wlan_mlme_ucfg_api.h"
#endif
#include "wlan_crypto_global_api.h"

#ifdef CONN_MGR_ADV_FEATURE
#ifdef WLAN_FEATURE_FILS_SK
static inline void osif_update_fils_hlp_data(struct net_device *dev,
					     struct wlan_objmgr_vdev *vdev,
					     struct wlan_cm_connect_resp *rsp)
{
	if (rsp->connect_ies.fils_ie && rsp->connect_ies.fils_ie->hlp_data_len)
		osif_cm_set_hlp_data(dev, vdev, rsp);
}
#else
static inline void osif_update_fils_hlp_data(struct net_device *dev,
					     struct wlan_objmgr_vdev *vdev,
					     struct wlan_cm_connect_resp *rsp)
{
}
#endif

/**
 * osif_roamed_ind() - send roamed indication to cfg80211
 * @dev: network device
 * @bss: cfg80211 roamed bss pointer
 * @req_ie: IEs used in reassociation request
 * @req_ie_len: Length of the @req_ie
 * @resp_ie: IEs received in successful reassociation response
 * @resp_ie_len: Length of @resp_ie
 *
 * Return: none
 */
#if defined CFG80211_ROAMED_API_UNIFIED || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))

#ifdef CFG80211_SINGLE_NETDEV_MULTI_LINK_SUPPORT
static
void osif_copy_roamed_info(struct cfg80211_roam_info *info,
			   struct cfg80211_bss *bss)
{
	info->links[0].bss = bss;
}
#else
static
void osif_copy_roamed_info(struct cfg80211_roam_info *info,
			   struct cfg80211_bss *bss)
{
	info->bss = bss;
}
#endif
static void osif_roamed_ind(struct net_device *dev, struct cfg80211_bss *bss,
			    const uint8_t *req_ie,
			    size_t req_ie_len, const uint8_t *resp_ie,
			    size_t resp_ie_len)
{
	struct cfg80211_roam_info info = {0};

	osif_copy_roamed_info(&info, bss);
	info.req_ie = req_ie;
	info.req_ie_len = req_ie_len;
	info.resp_ie = resp_ie;
	info.resp_ie_len = resp_ie_len;
	cfg80211_roamed(dev, &info, qdf_mem_malloc_flags());
}
#else
static inline void osif_roamed_ind(struct net_device *dev,
				   struct cfg80211_bss *bss,
				   const uint8_t *req_ie, size_t req_ie_len,
				   const uint8_t *resp_ie,
				   size_t resp_ie_len)
{
	cfg80211_roamed_bss(dev, bss, req_ie, req_ie_len, resp_ie, resp_ie_len,
			    qdf_mem_malloc_flags());
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
#ifdef WLAN_FEATURE_FILS_SK
/**
 * wlan_hdd_add_fils_params_roam_auth_event() - Adds FILS params in roam auth
 * @skb: SK buffer
 * @roam_info: Roam info
 *
 * API adds fils params[pmk, pmkid, next sequence number] to roam auth event
 *
 * Return: zero on success, error code on failure
 */
static int
osif_add_fils_params_roam_auth_event(struct sk_buff *skb,
				     struct wlan_roam_sync_info *roam_info)
{
	if (roam_info->pmk_len &&
	    nla_put(skb, QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PMK,
		    roam_info->pmk_len, roam_info->pmk)) {
		osif_err("pmk send fail");
		return -EINVAL;
	}

	if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PMKID,
		    PMKID_LEN, roam_info->pmkid)) {
		osif_err("pmkid send fail");
		return -EINVAL;
	}

	osif_debug("Update ERP Seq Num %d, Next ERP Seq Num %d",
		   roam_info->update_erp_next_seq_num,
		   roam_info->next_erp_seq_num);
	if (roam_info->update_erp_next_seq_num &&
	    nla_put_u16(skb,
			QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_FILS_ERP_NEXT_SEQ_NUM,
			roam_info->next_erp_seq_num)) {
		osif_err("ERP seq num send fail");
		return -EINVAL;
	}

	return 0;
}
#else
static inline int
osif_add_fils_params_roam_auth_event(struct sk_buff *skb,
				     struct wlan_roam_sync_info *roam_info)
{
	return 0;
}
#endif

/**
 * osif_get_roam_reason() - convert wmi roam reason to
 * enum qca_roam_reason
 * @roam_scan_trigger: wmi roam scan trigger ID
 *
 * Return: Meaningful qca_roam_reason from enum WMI_ROAM_TRIGGER_REASON_ID
 */
static enum qca_roam_reason osif_get_roam_reason(uint16_t roam_scan_trigger)
{
	switch (roam_scan_trigger) {
	case ROAM_TRIGGER_REASON_PER:
		return QCA_ROAM_REASON_PER;
	case ROAM_TRIGGER_REASON_BMISS:
		return QCA_ROAM_REASON_BEACON_MISS;
	case ROAM_TRIGGER_REASON_LOW_RSSI:
	case ROAM_TRIGGER_REASON_BACKGROUND:
		return QCA_ROAM_REASON_POOR_RSSI;
	case ROAM_TRIGGER_REASON_HIGH_RSSI:
		return QCA_ROAM_REASON_BETTER_RSSI;
	case ROAM_TRIGGER_REASON_DENSE:
		return QCA_ROAM_REASON_CONGESTION;
	case ROAM_TRIGGER_REASON_FORCED:
		return QCA_ROAM_REASON_USER_TRIGGER;
	case ROAM_TRIGGER_REASON_BTM:
		return QCA_ROAM_REASON_BTM;
	case ROAM_TRIGGER_REASON_BSS_LOAD:
		return QCA_ROAM_REASON_BSS_LOAD;
	default:
		return QCA_ROAM_REASON_UNKNOWN;
	}

	return QCA_ROAM_REASON_UNKNOWN;
}

/**
 * osif_send_roam_auth_event() - API to send roam auth event response to kernel
 * @vdev: vdev pointer
 * @osif_priv: OS private structure of vdev
 * @rsp: Connection manager response
 *
 * This is called when wlan driver needs to send the roaming and
 * authorization information after roaming.
 *
 * The information that would be sent is the request RSN IE, response
 * RSN IE and BSSID of the newly roamed AP.
 *
 * If the Authorized status is authenticated, then additional parameters
 * like PTK's KCK and KEK and Replay Counter would also be passed to the
 * supplicant.
 *
 * The supplicant upon receiving this event would ignore the legacy
 * cfg80211_roamed call and use the entire information from this event.
 * The cfg80211_roamed should still co-exist since the kernel will
 * make use of the parameters even if the supplicant ignores it.
 *
 *
 * Context: Any context.
 * Return: int
 */
static int osif_send_roam_auth_event(struct wlan_objmgr_vdev *vdev,
				     struct vdev_osif_priv *osif_priv,
				     struct wlan_cm_connect_resp *rsp,
				     const uint8_t *req_ie,
				     size_t req_ie_len, const uint8_t *resp_ie,
				     size_t resp_ie_len)
{
	struct wlan_objmgr_psoc *psoc;
	uint32_t fils_params_len;
	struct sk_buff *skb = NULL;
	struct wlan_roam_sync_info *roaming_info;
	int status;
	int32_t akm;
	bool roam_offload_enable;

	psoc = wlan_vdev_get_psoc(vdev);
	ucfg_mlme_get_roaming_offload(psoc, &roam_offload_enable);

	if (!roam_offload_enable)
		return 0;

	roaming_info = rsp->roaming_info;

	/*
	 * PMK is sent from FW in Roam Synch Event for FILS Roaming.
	 * In that case, add three more NL attributes.ie. PMK, PMKID
	 * and ERP next sequence number. Add corresponding lengths
	 * with 3 extra NL message headers for each of the
	 * aforementioned params.
	 */
	fils_params_len = roaming_info->pmk_len + PMKID_LEN +
			  sizeof(uint16_t) + (3 * NLMSG_HDRLEN);

	skb = cfg80211_vendor_event_alloc(osif_priv->wdev->wiphy,
			osif_priv->wdev,
			ETH_ALEN + req_ie_len +
			resp_ie_len +
			sizeof(uint8_t) + REPLAY_CTR_LEN +
			KCK_KEY_LEN + roaming_info->kek_len +
			sizeof(uint16_t) + sizeof(uint8_t) +
			(9 * NLMSG_HDRLEN) + fils_params_len,
			QCA_NL80211_VENDOR_SUBCMD_KEY_MGMT_ROAM_AUTH_INDEX,
			qdf_mem_malloc_flags());

	if (!skb) {
		osif_err("cfg80211_vendor_event_alloc failed");
		return -1;
	}

	if (nla_put(skb, QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_BSSID,
		    ETH_ALEN, rsp->bssid.bytes) ||
	    nla_put(skb, QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_REQ_IE,
		    req_ie_len, req_ie) ||
	    nla_put(skb, QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_RESP_IE,
		    resp_ie_len, resp_ie)) {
		osif_err("nla put fail");
		goto nla_put_failure;
	}

	if (roaming_info->auth_status == ROAM_AUTH_STATUS_AUTHENTICATED) {
		osif_debug("Include Auth Params TLV's");
		if (nla_put_u8(skb,
			       QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_AUTHORIZED,
			       true)) {
			osif_err("nla put fail");
			goto nla_put_failure;
		}
		akm = wlan_crypto_get_param(vdev,
					    WLAN_CRYPTO_PARAM_KEY_MGMT);
		/* if FT or CCKM connection: dont send replay counter */
		if (!QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X) &&
		    !QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_PSK) &&
		    !QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_SAE) &&
		    !QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_FT_IEEE8021X_SHA384) &&
		    !QDF_HAS_PARAM(akm, WLAN_CRYPTO_KEY_MGMT_CCKM) &&
		    nla_put(skb,
			    QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_KEY_REPLAY_CTR,
			    REPLAY_CTR_LEN,
			    roaming_info->replay_ctr)) {
			osif_err("non FT/non CCKM connection");
			osif_err("failed to send replay counter");
			goto nla_put_failure;
		}
		if (roaming_info->kek_len > MAX_KEK_LENGTH ||
		    nla_put(skb,
			    QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PTK_KCK,
			    roaming_info->kck_len, roaming_info->kck) ||
		    nla_put(skb,
			    QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_PTK_KEK,
			    roaming_info->kek_len, roaming_info->kek)) {
			osif_err("nla put fail, kek_len %d",
				 roaming_info->kek_len);
			goto nla_put_failure;
		}

		if (nla_put_u16(skb,
				QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_REASON,
				osif_get_roam_reason(roaming_info->roam_reason))) {
			osif_err("roam reason send failure");
			goto nla_put_failure;
		}

		status = osif_add_fils_params_roam_auth_event(skb,
							      roaming_info);
		if (status)
			goto nla_put_failure;
		/*
		 * Save the gtk rekey parameters in HDD STA context. They will
		 * be used next time when host enables GTK offload and goes
		 * into power save state.
		 */
		osif_cm_save_gtk(vdev, rsp);
		osif_debug("roam_info_ptr->replay_ctr 0x%llx",
			   *((uint64_t *)roaming_info->replay_ctr));

	} else {
		osif_debug("No Auth Params TLV's");
		if (nla_put_u8(skb,
			       QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_AUTHORIZED,
			       false)) {
			osif_err("nla put fail");
			goto nla_put_failure;
		}
	}

	osif_debug("Auth Status = %d Subnet Change Status = %d",
		   roaming_info->auth_status,
		   roaming_info->subnet_change_status);
	/*
	 * Add subnet change status if subnet has changed
	 * 0 = unchanged
	 * 1 = changed
	 * 2 = unknown
	 */
	if (roaming_info->subnet_change_status) {
		if (nla_put_u8(skb,
			       QCA_WLAN_VENDOR_ATTR_ROAM_AUTH_SUBNET_STATUS,
			       roaming_info->subnet_change_status)) {
			osif_err("nla put fail");
			goto nla_put_failure;
		}
	}

	cfg80211_vendor_event(skb, qdf_mem_malloc_flags());
	return 0;

nla_put_failure:
	kfree_skb(skb);
	return -1;
}
#else
static inline int
osif_send_roam_auth_event(struct wlan_objmgr_vdev *vdev,
			  struct vdev_osif_priv *osif_priv,
			  struct wlan_cm_connect_resp *rsp,
			  const uint8_t *req_ie,
			  size_t req_ie_len, const uint8_t *resp_ie,
			  size_t resp_ie_len)
{
	return 0;
}
#endif

static void osif_cm_get_reassoc_req_ie_data(struct element_info *assoc_req,
					    size_t *ie_data_len,
					    const uint8_t **ie_data_ptr)
{
	/* Validate IE and length */
	if (!assoc_req->len || !assoc_req->ptr ||
	    assoc_req->len <= WLAN_REASSOC_REQ_IES_OFFSET)
		return;

	*ie_data_len = assoc_req->len - WLAN_REASSOC_REQ_IES_OFFSET;
	*ie_data_ptr = assoc_req->ptr + WLAN_REASSOC_REQ_IES_OFFSET;
}

void osif_indicate_reassoc_results(struct wlan_objmgr_vdev *vdev,
				   struct vdev_osif_priv *osif_priv,
				   struct wlan_cm_connect_resp *rsp)
{
	struct net_device *dev = osif_priv->wdev->netdev;
	size_t req_len = 0;
	const uint8_t *req_ie = NULL;
	size_t rsp_len = 0;
	const uint8_t *rsp_ie = NULL;
	struct cfg80211_bss *bss;
	struct ieee80211_channel *chan;
	struct wlan_objmgr_psoc *psoc;

	if (QDF_IS_STATUS_ERROR(rsp->connect_status))
		return;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return;

	chan = ieee80211_get_channel(osif_priv->wdev->wiphy,
				     rsp->freq);
	bss = wlan_cfg80211_get_bss(osif_priv->wdev->wiphy, chan,
				    rsp->bssid.bytes, rsp->ssid.ssid,
				    rsp->ssid.length);
	if (!bss)
		osif_warn("not able to find bss");
	if (rsp->is_assoc)
		osif_cm_get_assoc_req_ie_data(&rsp->connect_ies.assoc_req,
					      &req_len, &req_ie);
	else
		osif_cm_get_reassoc_req_ie_data(&rsp->connect_ies.assoc_req,
						&req_len, &req_ie);
	osif_cm_get_assoc_rsp_ie_data(&rsp->connect_ies.assoc_rsp,
				      &rsp_len, &rsp_ie);
	osif_roamed_ind(dev, bss, req_ie, req_len, rsp_ie, rsp_len);
	osif_send_roam_auth_event(vdev, osif_priv, rsp, req_ie, req_len, rsp_ie,
				  rsp_len);

	osif_update_fils_hlp_data(dev, vdev, rsp);
}

QDF_STATUS
osif_pmksa_candidate_notify(struct wlan_objmgr_vdev *vdev,
			    struct qdf_mac_addr *bssid,
			    int index, bool preauth)
{
	struct vdev_osif_priv *osif_priv = wlan_vdev_get_ospriv(vdev);
	struct wireless_dev *wdev;

	if (!osif_priv) {
		osif_err("Invalid vdev osif priv");
		return QDF_STATUS_E_INVAL;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		osif_err("wdev is null");
		return QDF_STATUS_E_INVAL;
	}

	osif_debug("is going to notify supplicant of:");
	osif_info(QDF_MAC_ADDR_FMT, QDF_MAC_ADDR_REF(bssid->bytes));

	cfg80211_pmksa_candidate_notify(wdev->netdev, index,
					bssid->bytes,
					preauth, qdf_mem_malloc_flags());
	return QDF_STATUS_SUCCESS;
}
#endif /* CONN_MGR_ADV_FEATURE */
