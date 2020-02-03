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
 * DOC: Declare public API related to the pkt_capture called by north bound
 * HDD/OSIF/LIM
 */

#ifndef _WLAN_PKT_CAPTURE_UCFG_API_H_
#define _WLAN_PKT_CAPTURE_UCFG_API_H_

#include <qdf_status.h>
#include <qdf_types.h>
#include "wlan_pkt_capture_objmgr.h"
#include "wlan_pkt_capture_public_structs.h"

#ifdef WLAN_FEATURE_PKT_CAPTURE
/**
 * ucfg_pkt_capture_init() - Packet capture component initialization.
 *
 * This function gets called when packet capture initializing.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_pkt_capture_init(void);

/**
 * ucfg_pkt_capture_deinit() - Packet capture component de-init.
 *
 * This function gets called when packet capture de-init.
 *
 * Return: None
 */
void ucfg_pkt_capture_deinit(void);

/**
 * ucfg_pkt_capture_get_mode() - get packet capture mode
 * @psoc: pointer to psoc object
 *
 * Return: enum pkt_capture_mode
 */
enum pkt_capture_mode
ucfg_pkt_capture_get_mode(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_pkt_capture_suspend_mon_thread() - suspend packet capture mon thread
 * vdev: pointer to vdev object manager
 *
 * Return: 0 on success, -EINVAL on failure
 */
int ucfg_pkt_capture_suspend_mon_thread(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_pkt_capture_resume_mon_thread() - resume packet capture mon thread
 * vdev: pointer to vdev object manager
 *
 * Resume packet capture MON thread by completing RX thread resume event
 *
 * Return: None
 */
void ucfg_pkt_capture_resume_mon_thread(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_pkt_capture_register_callbacks - Register packet capture callbacks
 * @vdev: pointer to wlan vdev object manager
 * mon_cb: callback to call
 * context: callback context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_pkt_capture_register_callbacks(struct wlan_objmgr_vdev *vdev,
				    QDF_STATUS (*mon_cb)(void *, qdf_nbuf_t),
				    void *context);

/**
 * ucfg_pkt_capture_deregister_callbacks - De-register packet capture callbacks
 * @vdev: pointer to wlan vdev object manager
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_pkt_capture_deregister_callbacks(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_pkt_capturee_set_pktcap_mode - Set packet capture mode
 * @psoc: pointer to psoc object
 * @mode: mode to be set
 *
 * Return: None
 */
void ucfg_pkt_capture_set_pktcap_mode(struct wlan_objmgr_psoc *psoc,
				      enum pkt_capture_mode val);

/**
 * ucfg_pkt_capture_get_pktcap_mode - Get packet capture mode
 * @psoc: pointer to psoc object
 *
 * Return: enum pkt_capture_mode
 */
enum pkt_capture_mode
ucfg_pkt_capture_get_pktcap_mode(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_pkt_capture_process_mgmt_tx_data() - process management tx packets
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
				      uint8_t status);

/**
 * ucfg_pkt_capture_mgmt_tx_completion(): process mgmt tx completion
 * for pkt capture mode
 * @pdev: pointer to pdev object
 * @desc_id: desc_id
 * @status: status
 * @params: management offload event params
 *
 * Return: none
 */
void
ucfg_pkt_capture_mgmt_tx_completion(
				struct wlan_objmgr_pdev *pdev,
				uint32_t desc_id,
				uint32_t status,
				struct mgmt_offload_event_params *params);

/**
 * ucfg_pkt_capture_enable_ops - Enable packet capture tx and rx ops handlers
 * @wlan_objmgr_vdev: wlan vdev object manager
 *
 * Return: 0 on success, -EINVAL on failure
 */
int ucfg_pkt_capture_enable_ops(struct wlan_objmgr_vdev *vdev);
#else
static inline
QDF_STATUS ucfg_pkt_capture_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void ucfg_pkt_capture_deinit(void)
{
}

static inline
enum pkt_capture_mode ucfg_pkt_capture_get_mode(struct wlan_objmgr_psoc *psoc)
{
	return PACKET_CAPTURE_MODE_DISABLE;
}

static inline
void ucfg_pkt_capture_resume_mon_thread(struct wlan_objmgr_vdev *vdev)
{
}

static inline
int ucfg_pkt_capture_suspend_mon_thread(struct wlan_objmgr_vdev *vdev)
{
	return 0;
}

static inline QDF_STATUS
ucfg_pkt_capture_register_callbacks(struct wlan_objmgr_vdev *vdev,
				    QDF_STATUS (*mon_cb)(void *, qdf_nbuf_t),
				    void *context)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_pkt_capture_deregister_callbacks(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void ucfg_pkt_capture_set_pktcap_mode(struct wlan_objmgr_psoc *psoc,
				      uint8_t val)
{
}

static inline enum pkt_capture_mode
ucfg_pkt_capture_get_pktcap_mode(struct wlan_objmgr_psoc *psoc)
{
	return PACKET_CAPTURE_MODE_DISABLE;
}

static inline QDF_STATUS
ucfg_pkt_capture_process_mgmt_tx_data(
				struct mgmt_offload_event_params *params,
				qdf_nbuf_t nbuf,
				uint8_t status)
{
	return 0;
}

static inline void
ucfg_pkt_capture_mgmt_tx_completion(struct wlan_objmgr_pdev *pdev,
				    uint32_t desc_id,
				    uint32_t status,
				    struct mgmt_offload_event_params *params)
{
}
#endif /* WLAN_FEATURE_PKT_CAPTURE */
#endif /* _WLAN_PKT_CAPTURE_UCFG_API_H_ */
