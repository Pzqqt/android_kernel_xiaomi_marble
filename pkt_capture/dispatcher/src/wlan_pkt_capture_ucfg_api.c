/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: Public API implementation of pkt_capture called by north bound HDD/OSIF.
 */

#include "wlan_pkt_capture_objmgr.h"
#include "wlan_pkt_capture_main.h"
#include "wlan_pkt_capture_ucfg_api.h"
#include "wlan_pkt_capture_mon_thread.h"
#include "wlan_pkt_capture_mgmt_txrx.h"

enum pkt_capture_mode ucfg_pkt_capture_get_mode(struct wlan_objmgr_psoc *psoc)
{
	return pkt_capture_get_mode(psoc);
}

/**
 * ucfg_pkt_capture_register_callbacks - Register packet capture callbacks
 * @vdev: pointer to wlan vdev object manager
 * mon_cb: callback to call
 * context: callback context
 *
 * Return: 0 in case of success, invalid in case of failure.
 */
QDF_STATUS
ucfg_pkt_capture_register_callbacks(struct wlan_objmgr_vdev *vdev,
				    QDF_STATUS (*mon_cb)(void *, qdf_nbuf_t),
				    void *context)
{
	return pkt_capture_register_callbacks(vdev, mon_cb, context);
}

/**
 * ucfg_pkt_capture_deregister_callbacks - De-register packet capture callbacks
 * @vdev: pointer to wlan vdev object manager
 *
 * Return: 0 in case of success, invalid in case of failure.
 */
QDF_STATUS ucfg_pkt_capture_deregister_callbacks(struct wlan_objmgr_vdev *vdev)
{
	return pkt_capture_deregister_callbacks(vdev);
}

/**
 * ucfg_pkt_capture_set_pktcap_mode - Set packet capture mode
 * @psoc: pointer to psoc object
 * @mode: mode to be set
 *
 * Return: None
 */
void ucfg_pkt_capture_set_pktcap_mode(struct wlan_objmgr_psoc *psoc,
				      enum pkt_capture_mode mode)
{
	pkt_capture_set_pktcap_mode(psoc, mode);
}

/**
 * ucfg_pkt_capture_get_pktcap_mode - Get packet capture mode
 * @psoc: pointer to psoc object
 *
 * Return: enum pkt_capture_mode
 */
enum pkt_capture_mode
ucfg_pkt_capture_get_pktcap_mode(struct wlan_objmgr_psoc *psoc)
{
	return pkt_capture_get_pktcap_mode(psoc);
}

/**
 * ucfg_pkt_capture_init() - Packet capture component initialization.
 *
 * This function gets called when packet capture initializing.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success.
 */
QDF_STATUS ucfg_pkt_capture_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_psoc_create_notification,
				NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		pkt_capture_err("Failed to register psoc create handler");
		return status;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_psoc_destroy_notification,
				NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		pkt_capture_err("Failed to register psoc delete handler");
		goto fail_destroy_psoc;
	}

	status = wlan_objmgr_register_vdev_create_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_vdev_create_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		pkt_capture_err("Failed to register vdev create handler");
		goto fail_create_vdev;
	}

	status = wlan_objmgr_register_vdev_destroy_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_vdev_destroy_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		pkt_capture_err("Failed to register vdev destroy handler");
		goto fail_destroy_vdev;
	}
	return status;

fail_destroy_vdev:
	wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_PKT_CAPTURE,
		pkt_capture_vdev_create_notification, NULL);

fail_create_vdev:
	wlan_objmgr_unregister_psoc_destroy_handler(WLAN_UMAC_COMP_PKT_CAPTURE,
		pkt_capture_psoc_destroy_notification, NULL);

fail_destroy_psoc:
	wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_PKT_CAPTURE,
		pkt_capture_psoc_create_notification, NULL);

	return status;
}

void ucfg_pkt_capture_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_vdev_destroy_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_vdev_destroy_notification,
				NULL);
	if (QDF_IS_STATUS_ERROR(status))
		pkt_capture_err("Failed to unregister vdev destroy handler");

	status = wlan_objmgr_unregister_vdev_create_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_vdev_create_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status))
		pkt_capture_err("Failed to unregister vdev create handler");

	status = wlan_objmgr_unregister_psoc_destroy_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_psoc_destroy_notification,
				NULL);
	if (QDF_IS_STATUS_ERROR(status))
		pkt_capture_err("Failed to unregister psoc destroy handler");

	status = wlan_objmgr_unregister_psoc_create_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_psoc_create_notification,
				NULL);
	if (QDF_IS_STATUS_ERROR(status))
		pkt_capture_err("Failed to unregister psoc create handler");
}

int ucfg_pkt_capture_suspend_mon_thread(struct wlan_objmgr_vdev *vdev)
{
	return pkt_capture_suspend_mon_thread(vdev);
}

void ucfg_pkt_capture_resume_mon_thread(struct wlan_objmgr_vdev *vdev)
{
	pkt_capture_resume_mon_thread(vdev);
}

/**
 * ucfg_process_pktcapture_mgmt_tx_data() - process management tx packets
 * @pdev: pointer to pdev object
 * @params: management offload event params
 * @nbuf: netbuf
 * @status: status
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_pkt_capture_process_mgmt_tx_data(struct wlan_objmgr_pdev *pdev,
				      struct mgmt_offload_event_params *params,
				      qdf_nbuf_t nbuf,
				      uint8_t status)
{
	return pkt_capture_process_mgmt_tx_data(pdev, params, nbuf, status);
}

/**
 * ucfg_process_pktcapture_mgmt_tx_completion(): process mgmt tx completion
 * for pkt capture mode
 * @pdev: pointer to pdev object
 * @desc_id: desc_id
 * @status: status
 * @params: management offload event params
 *
 * Return: none
 */
void
ucfg_pkt_capture_mgmt_tx_completion(struct wlan_objmgr_pdev *pdev,
				    uint32_t desc_id,
				    uint32_t status,
				    struct mgmt_offload_event_params *params)
{
	pkt_capture_mgmt_tx_completion(pdev, desc_id, status, params);
}
