/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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

#ifndef _TARGET_IF_CFR_H_
#define _TARGET_IF_CFR_H_

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>

#include "wmi_unified_cfr_api.h"
#include "wmi_unified_param.h"
#include "wmi_unified_cfr_param.h"
#define PEER_CFR_CAPTURE_ENABLE   1
#define PEER_CFR_CAPTURE_DISABLE  0

#define PEER_CFR_CAPTURE_EVT_STATUS_MASK 0x80000000
#define PEER_CFR_CAPTURE_EVT_PS_STATUS_MASK 0x40000000
#define CFR_TX_EVT_STATUS_MASK           0x00000003

/* Status codes used by correlate and relay function */
#define STATUS_STREAM_AND_RELEASE 0
#define STATUS_HOLD               1
#define STATUS_ERROR             -1

/* Module IDs using corrlation function */
#define CORRELATE_DBR_MODULE_ID   0
/*
 * HKV2 - Tx completion event for one-shot capture
 * Cypress - Tx completion event for one-shot capture (or) RXTLV event for RCC
 */
#define CORRELATE_TX_EV_MODULE_ID 1

/**
 * target_if_cfr_init_pdev() - Inits cfr pdev and registers necessary handlers.
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: Registration status for necessary handlers
 */
QDF_STATUS
target_if_cfr_init_pdev(struct wlan_objmgr_psoc *psoc,
			struct wlan_objmgr_pdev *pdev);

/**
 * target_if_cfr_deinit_pdev() - De-inits corresponding pdev and handlers.
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: De-registration status for necessary handlers
 */
QDF_STATUS
target_if_cfr_deinit_pdev(struct wlan_objmgr_psoc *psoc,
			  struct wlan_objmgr_pdev *pdev);

/**
 * target_if_cfr_tx_ops_register() - Registers tx ops for cfr module
 * @tx_ops - pointer to tx_ops structure.
 */
void target_if_cfr_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * target_if_cfr_enable_cfr_timer() - Enables cfr timer
 * @pdev: pointer to pdev object
 * @cfr_timer: Amount of time this timer has to run
 *
 * Return: status of timer
 */
int target_if_cfr_enable_cfr_timer(struct wlan_objmgr_pdev *pdev,
				   uint32_t cfr_timer);

/**
 * target_if_cfr_pdev_set_param() - Function to set params for cfr config
 * @pdev: pointer to pdev object
 * @param_id: param id which has to be set
 * @param_value: value of param being set
 *
 * Return: success/failure of setting param
 */
int target_if_cfr_pdev_set_param(struct wlan_objmgr_pdev *pdev,
				 uint32_t param_id, uint32_t param_value);
/**
 * target_if_cfr_start_capture() - Function to start cfr capture for a peer
 * @pdev: pointer to pdev object
 * @peer: pointer to peer object
 * @cfr_params: capture parameters for this peer
 *
 * Return: success/failure status of start capture
 */
int target_if_cfr_start_capture(struct wlan_objmgr_pdev *pdev,
				struct wlan_objmgr_peer *peer,
				struct cfr_capture_params *cfr_params);
/**
 * target_if_cfr_stop_capture() - Function to stop cfr capture for a peer
 * @pdev: pointer to pdev object
 * @peer: pointer to peer object
 *
 * Return: success/failure status of stop capture
 */
int target_if_cfr_stop_capture(struct wlan_objmgr_pdev *pdev,
			       struct wlan_objmgr_peer *peer);

/**
 * target_if_cfr_get_target_type() - Function to get target type
 * @psoc: pointer to psoc object
 *
 * Return: target type of target
 */
int target_if_cfr_get_target_type(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_cfr_set_cfr_support() - Function to set cfr support
 * @psoc: pointer to psoc object
 * @value: value to be set
 */
void target_if_cfr_set_cfr_support(struct wlan_objmgr_psoc *psoc,
				   uint8_t value);

/**
 * target_if_cfr_set_capture_count_support() - Function to set capture count
 *					       support.
 * @psoc: pointer to psoc object
 * @value: value to be set
 *
 * Return: success/failure
 */
QDF_STATUS
target_if_cfr_set_capture_count_support(struct wlan_objmgr_psoc *psoc,
					uint8_t value);

/**
 * target_if_cfr_set_mo_marking_support() - Function to set MO marking support
 * @psoc: pointer to psoc object
 * @value: value to be set
 *
 * Return: success/failure
 */
QDF_STATUS
target_if_cfr_set_mo_marking_support(struct wlan_objmgr_psoc *psoc,
				     uint8_t value);

/**
 * target_if_cfr_info_send() - Function to send cfr info to upper layers
 * @pdev: pointer to pdev object
 * @head: pointer to cfr info head
 * @hlen: head len
 * @data: pointer to cfr info data
 * @dlen: data len
 * @tail: pointer to cfr info tail
 * @tlen: tail len
 */
void target_if_cfr_info_send(struct wlan_objmgr_pdev *pdev, void *head,
			     size_t hlen, void *data, size_t dlen, void *tail,
			     size_t tlen);

#ifdef WIFI_TARGET_TYPE_2_0
/**
 * cfr_wifi2_0_init_pdev() - Function to init legacy pdev
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: success/failure status of init
 */
QDF_STATUS cfr_wifi2_0_init_pdev(struct wlan_objmgr_psoc *psoc,
				 struct wlan_objmgr_pdev *pdev);

/**
 * cfr_wifi2_0_deinit_pdev() - Function to deinit legacy pdev
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: success/failure status of deinit
 */
QDF_STATUS cfr_wifi2_0_deinit_pdev(struct wlan_objmgr_psoc *psoc,
				   struct wlan_objmgr_pdev *pdev);

#else
#ifndef CFR_USE_FIXED_FOLDER
static QDF_STATUS cfr_wifi2_0_init_pdev(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS cfr_wifi2_0_deinit_pdev(struct wlan_objmgr_psoc *psoc,
					  struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif
#endif
