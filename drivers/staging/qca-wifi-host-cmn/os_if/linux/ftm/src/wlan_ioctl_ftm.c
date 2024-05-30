/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: implementation of the driver FTM functions interfacing with linux kernel
 */

#include <qdf_util.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_ftm_ucfg_api.h>
#include <qdf_types.h>
#include <qdf_module.h>
#include <wlan_cfg80211_ftm.h>
#include <wlan_ioctl_ftm.h>

static QDF_STATUS
wlan_process_ftm_ioctl_cmd(struct wlan_objmgr_pdev *pdev, uint8_t *userdata)
{
	uint8_t *buffer;
	QDF_STATUS error;
	int length;

	if (get_user(length, (uint32_t *)userdata) != 0)
		return QDF_STATUS_E_FAILURE;

	if (length > WLAN_FTM_DATA_MAX_LEN)
		return QDF_STATUS_E_FAILURE;

	buffer = qdf_mem_malloc(length);
	if (!buffer)
		return QDF_STATUS_E_NOMEM;

	if (copy_from_user(buffer, &userdata[sizeof(length)], length))
		error = QDF_STATUS_E_FAILURE;
	else
		error = ucfg_wlan_ftm_testmode_cmd(pdev, buffer, length);

	qdf_mem_free(buffer);

	return error;
}

static QDF_STATUS
wlan_process_ftm_ioctl_rsp(struct wlan_objmgr_pdev *pdev, uint8_t *userdata)
{
	uint8_t *buffer;
	QDF_STATUS error;
	int length;

	length = WLAN_FTM_DATA_MAX_LEN + sizeof(u_int32_t);

	buffer = qdf_mem_malloc(length);
	if (!buffer)
		return QDF_STATUS_E_NOMEM;

	error = ucfg_wlan_ftm_testmode_rsp(pdev, buffer);
	if (!error)
		error = copy_to_user((userdata - sizeof(int)), buffer, length);
	else
		error = QDF_STATUS_E_AGAIN;

	qdf_mem_free(buffer);

	return error;
}

int
wlan_ioctl_ftm_testmode_cmd(struct wlan_objmgr_pdev *pdev, int cmd,
			    uint8_t *userdata)
{
	QDF_STATUS error;
	struct wifi_ftm_pdev_priv_obj *ftm_pdev_obj;

	ftm_pdev_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
			WLAN_UMAC_COMP_FTM);
	if (!ftm_pdev_obj) {
		ftm_err("Failed to get ftm pdev component");
		return QDF_STATUS_E_FAILURE;
	}

	ftm_pdev_obj->cmd_type = WIFI_FTM_CMD_IOCTL;

	switch (cmd) {
	case FTM_IOCTL_UNIFIED_UTF_CMD:
		error = wlan_process_ftm_ioctl_cmd(pdev, userdata);
		break;
	case FTM_IOCTL_UNIFIED_UTF_RSP:
		error = wlan_process_ftm_ioctl_rsp(pdev, userdata);
		break;
	default:
		ftm_err("FTM Unknown cmd - not supported");
		error = QDF_STATUS_E_NOSUPPORT;
	}

	return qdf_status_to_os_return(error);
}

qdf_export_symbol(wlan_ioctl_ftm_testmode_cmd);
