/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: wifi_pos_main.c
 * This file defines the important functions pertinent to
 * wifi positioning to initialize and de-initialize the component.
 */
#include "target_if_wifi_pos.h"
#include "os_if_wifi_pos.h"
#include "wifi_pos_utils_i.h"
#include "wifi_pos_api.h"
#include "wifi_pos_main_i.h"
#include "wifi_pos_ucfg_i.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_psoc_obj.h"

#ifdef UMAC_REG_COMPONENT
/* enable this when regulatory component gets merged */
#include "wlan_reg_services_api.h"
/* forward declartion */
struct regulatory_channel;
#endif

/**
 * wifi_pos_get_tlv_support: indicates if firmware supports TLV wifi pos msg
 * @psoc: psoc object
 *
 * Return: status of operation
 */
static bool wifi_pos_get_tlv_support(struct wlan_objmgr_psoc *psoc)
{
	/* this is TBD */
	return true;
}

/**
 * wifi_pos_tlv_callback: wifi pos msg handler registered for TLV type req
 * @wmi_msg: wmi type request msg
 *
 * Return: status of operation
 */
static QDF_STATUS wifi_pos_tlv_callback(struct wlan_objmgr_psoc *psoc,
					struct wifi_pos_req_msg *req)
{
	/* actual implementation of cmds start here */
	/* TBD - decide if ANI_MSG_OEM_DATA_REQ goest to MC thread or not */
	return QDF_STATUS_SUCCESS;
}

/**
 * wifi_pos_non_tlv_callback: wifi pos msg handler registered for non-TLV
 * type req
 * @wmi_msg: wmi type request msg
 *
 * Return: status of operation
 */
static QDF_STATUS wifi_pos_non_tlv_callback(struct wlan_objmgr_psoc *psoc,
					    struct wifi_pos_req_msg *req)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wifi_pos_psoc_obj_created_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list)
{
	QDF_STATUS status;
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj;

	/*
	 * this is for WIN, if they have multiple psoc, we dont want to create
	 * multiple priv object. Since there is just one LOWI app registered to
	 * one driver, avoid 2nd private object with another psoc.
	 */
	if (wifi_pos_get_psoc()) {
		wifi_pos_debug("global psoc obj already set. do not allocate another psoc private object");
		return QDF_STATUS_SUCCESS;
	} else {
		wifi_pos_debug("setting global pos object");
		wifi_pos_set_psoc(psoc);
	}

	/* initialize wifi-pos psoc priv object */
	wifi_pos_obj = qdf_mem_malloc(sizeof(*wifi_pos_obj));
	if (!wifi_pos_obj) {
		wifi_pos_alert("Mem alloc failed for wifi pos psoc priv obj");
		wifi_pos_clear_psoc();
		return QDF_STATUS_E_NOMEM;
	}

	qdf_spinlock_create(&wifi_pos_obj->wifi_pos_lock);
	/* Register TLV or non-TLV callbacks depending on target fw version */
	if (wifi_pos_get_tlv_support(psoc))
		wifi_pos_obj->wifi_pos_req_handler = wifi_pos_tlv_callback;
	else
		wifi_pos_obj->wifi_pos_req_handler = wifi_pos_non_tlv_callback;

	/*
	 * MGMT Rx is not handled in this phase since wifi pos only uses few
	 * measurement subtypes under RRM_RADIO_MEASURE_REQ. Rest of them are
	 * used for 80211k. That part is not yet converged and still follows
	 * legacy MGMT Rx to work. Action frame in new TXRX can be registered
	 * at per ACTION Frame type granularity only.
	 */

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						WLAN_UMAC_COMP_WIFI_POS,
						wifi_pos_obj,
						QDF_STATUS_SUCCESS);

	if (QDF_IS_STATUS_ERROR(status)) {
		wifi_pos_err("obj attach with psoc failed with status: %d",
				status);
		qdf_spinlock_destroy(&wifi_pos_obj->wifi_pos_lock);
		qdf_mem_free(wifi_pos_obj);
		wifi_pos_clear_psoc();
	}

	return status;
}

QDF_STATUS  wifi_pos_psoc_obj_destroyed_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list)
{
	QDF_STATUS status;
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj = NULL;

	if (wifi_pos_get_psoc() == psoc) {
		wifi_pos_debug("deregistering wifi_pos_psoc object");
		wifi_pos_clear_psoc();
	} else {
		wifi_pos_warn("un-related PSOC closed. do nothing");
		return QDF_STATUS_SUCCESS;
	}

	wifi_pos_obj = wifi_pos_get_psoc_priv_obj(psoc);
	if (!wifi_pos_obj) {
		wifi_pos_err("wifi_pos_obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						WLAN_UMAC_COMP_WIFI_POS,
						wifi_pos_obj);
	if (status != QDF_STATUS_SUCCESS)
		wifi_pos_err("wifi_pos_obj detach failed");

	wifi_pos_debug("wifi_pos_obj deleted with status %d", status);
	qdf_spinlock_destroy(&wifi_pos_obj->wifi_pos_lock);
	qdf_mem_free(wifi_pos_obj);

	return status;
}

#ifdef UMAC_REG_COMPONENT
/* enable this when regulatory component gets merged */
static void get_ch_info(struct wlan_objmgr_psoc *psoc,
			struct wifi_pos_driver_caps *caps)
{
	QDF_STATUS status;
	uint32_t i, num_ch = 0;
	struct regulatory_channel ch_lst[OEM_CAP_MAX_NUM_CHANNELS];
	struct reg_freq_range freq_range;

	freq_range.low_freq = WLAN_REG_CH_TO_FREQ(MIN_24GHZ_CHANNEL);
	freq_range.high_freq = WLAN_REG_CH_TO_FREQ(MAX_5GHZ_CHANNEL);
	status = wlan_reg_get_current_chan_list_by_range(psoc, ch_lst, &num_ch);
	if (QDF_IS_STATUS_ERROR(status)) {
		wifi_pos_err("wlan_reg_get_current_chan_list_by_range failed");
		return;
	}

	if (num_ch > OEM_CAP_MAX_NUM_CHANNELS) {
		wifi_pos_err("num channels: %d more than MAX: %d",
			num_ch, OEM_CAP_MAX_NUM_CHANNELS);
		return;
	}

	for (i = 0; i < num_ch; i++)
		caps->channel_list[i] = ch_lst[i].chan_num;

	caps->num_channels = num_ch;
}
#endif

QDF_STATUS wifi_pos_populate_caps(struct wlan_objmgr_psoc *psoc,
			   struct wifi_pos_driver_caps *caps)
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_obj =
					wifi_pos_get_psoc_priv_obj(psoc);

	wifi_pos_debug("Enter");
	if (!wifi_pos_obj) {
		wifi_pos_err("wifi_pos_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	strlcpy(caps->oem_target_signature,
		OEM_TARGET_SIGNATURE,
		OEM_TARGET_SIGNATURE_LEN);
	caps->oem_target_type = wifi_pos_obj->oem_target_type;
	caps->oem_fw_version = wifi_pos_obj->oem_fw_version;
	caps->driver_version.major = wifi_pos_obj->driver_version.major;
	caps->driver_version.minor = wifi_pos_obj->driver_version.minor;
	caps->driver_version.patch = wifi_pos_obj->driver_version.patch;
	caps->driver_version.build = wifi_pos_obj->driver_version.build;
	caps->allowed_dwell_time_min = wifi_pos_obj->allowed_dwell_time_min;
	caps->allowed_dwell_time_max = wifi_pos_obj->allowed_dwell_time_max;
	caps->curr_dwell_time_min = wifi_pos_obj->current_dwell_time_min;
	caps->curr_dwell_time_max = wifi_pos_obj->current_dwell_time_max;
	caps->supported_bands = wlan_objmgr_psoc_get_band_capability(psoc);
	/*
	 * wifi_pos_populate_caps does not have alternate definition.
	 * following #ifdef will be removed once regulatory comp gets merged
	 */
#ifdef UMAC_REG_COMPONENT
	get_ch_info(psoc, caps);
#endif
	return QDF_STATUS_SUCCESS;
}
