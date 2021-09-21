/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
 * DOC: Implement API's specific to WDS component.
 */

#include <wmi_unified_priv.h>
#include <wmi_unified_wds_api.h>

QDF_STATUS
wmi_unified_peer_add_wds_entry_cmd(wmi_unified_t wmi_hdl,
				   struct peer_add_wds_entry_params *param)
{
	if (wmi_hdl->ops->send_peer_add_wds_entry_cmd)
		return wmi_hdl->ops->send_peer_add_wds_entry_cmd(wmi_hdl,
								 param);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_peer_del_wds_entry_cmd(wmi_unified_t wmi_hdl,
				   struct peer_del_wds_entry_params *param)
{
	if (wmi_hdl->ops->send_peer_del_wds_entry_cmd)
		return wmi_hdl->ops->send_peer_del_wds_entry_cmd(wmi_hdl,
								 param);
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS
wmi_unified_update_wds_entry_cmd(wmi_unified_t wmi_hdl,
				 struct peer_update_wds_entry_params *param)
{
	if (wmi_hdl->ops->send_peer_update_wds_entry_cmd)
		return wmi_hdl->ops->send_peer_update_wds_entry_cmd(wmi_hdl,
								    param);
	return QDF_STATUS_E_FAILURE;
}
