/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: Declare public API related to the wlan ipa called by north bound
 */

#ifndef _WLAN_IPA_UCFG_API_H_
#define _WLAN_IPA_UCFG_API_H_

#include "wlan_ipa_public_struct.h"
#include "wlan_ipa_obj_mgmt_api.h"
#include "wlan_objmgr_pdev_obj.h"

#ifdef IPA_OFFLOAD

/**
 * ucfg_ipa_is_present() - get IPA hw status
 *
 * ipa_uc_reg_rdyCB is not directly designed to check
 * ipa hw status. This is an undocumented function which
 * has confirmed with IPA team.
 *
 * Return: true - ipa hw present
 *         false - ipa hw not present
 */
bool ucfg_ipa_is_present(void);

/**
 * ucfg_ipa_update_config() - Update IPA component config
 *
 * Return: None
 */
void ucfg_ipa_update_config(struct wlan_ipa_config *config);

/**
 * ucfg_ipa_set_dp_handle() - register DP handle
 * @psoc: psoc handle
 * @dp_soc: data path soc handle
 *
 * Return: None
 */
void ucfg_ipa_set_dp_handle(struct wlan_objmgr_psoc *psoc,
			       void *dp_soc);

/**
 * ucfg_ipa_set_txrx_handle() - register pdev txrx handler
 * @psoc: psoc handle
 * @psoc: psoc obj
 * @txrx_handle: data path pdev txrx handle
 *
 * Return: None
 */
void ucfg_ipa_set_txrx_handle(struct wlan_objmgr_psoc *psoc,
			      void *txrx_handle);

/**
 * ucfg_ipa_set_perf_level() - Set IPA perf level
 * @pdev: pdev obj
 * @tx_packets: Number of packets transmitted in the last sample period
 * @rx_packets: Number of packets received in the last sample period
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS ucfg_ipa_set_perf_level(struct wlan_objmgr_pdev *pdev,
				   uint64_t tx_packets, uint64_t rx_packets);

/**
 * ucfg_ipa_uc_info() - Print IPA uC resource and session information
 * @pdev: pdev obj
 *
 * Return: None
 */
void ucfg_ipa_uc_info(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_ipa_uc_stat() - Print IPA uC stats
 * @pdev: pdev obj
 *
 * Return: None
 */
void ucfg_ipa_uc_stat(struct wlan_objmgr_pdev *pdev);


/**
 * ucfg_ipa_uc_rt_debug_host_dump() - IPA rt debug host dump
 * @pdev: pdev obj
 *
 * Return: None
 */
void ucfg_ipa_uc_rt_debug_host_dump(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_ipa_dump_info() - Dump IPA context information
 * @pdev: pdev obj
 *
 * Return: None
 */
void ucfg_ipa_dump_info(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_ipa_uc_stat_request() - Get IPA stats from IPA.
 * @pdev: pdev obj
 * @reason: STAT REQ Reason
 *
 * Return: None
 */
void ucfg_ipa_uc_stat_request(struct wlan_objmgr_pdev *pdev,
			      uint8_t reason);

/**
 * ucfg_ipa_uc_stat_query() - Query the IPA stats
 * @pdev: pdev obj
 * @ipa_tx_diff: tx packet count diff from previous tx packet count
 * @ipa_rx_diff: rx packet count diff from previous rx packet count
 *
 * Return: None
 */
void ucfg_ipa_uc_stat_query(struct wlan_objmgr_pdev *pdev,
			    uint32_t *ipa_tx_diff, uint32_t *ipa_rx_diff);

/**
 * ucfg_ipa_reg_sap_xmit_cb() - Register upper layer SAP cb to transmit
 * @pdev: pdev obj
 * @cb: callback
 *
 * Return: None
 */
void ucfg_ipa_reg_sap_xmit_cb(struct wlan_objmgr_pdev *pdev, void *cb);

/**
 * ucfg_ipa_reg_send_to_nw_cb() - Register cb to send IPA Rx packet to network
 * @pdev: pdev obj
 * @cb: callback
 *
 * Return: None
 */
void ucfg_ipa_reg_send_to_nw_cb(struct wlan_objmgr_pdev *pdev, void *cb);

/**
 * ucfg_ipa_set_mcc_mode() - Set MCC mode
 * @pdev: pdev obj
 * @mcc_mode: 0=MCC/1=SCC
 *
 * Return: void
 */
void ucfg_ipa_set_mcc_mode(struct wlan_objmgr_pdev *pdev, bool mcc_mode);

/**
 * ucfg_ipa_set_dfs_cac_tx() - Set DFS cac tx block
 * @pdev: pdev obj
 * @tx_block: dfs cac tx block
 *
 * Return: void
 */
void ucfg_ipa_set_dfs_cac_tx(struct wlan_objmgr_pdev *pdev, bool tx_block);

/**
 * ucfg_ipa_set_ap_ibss_fwd() - Set AP intra bss forward
 * @pdev: pdev obj
 * @intra_bss: enable or disable ap intra bss forward
 *
 * Return: void
 */
void ucfg_ipa_set_ap_ibss_fwd(struct wlan_objmgr_pdev *pdev, bool intra_bss);

/**
 * ucfg_ipa_uc_force_pipe_shutdown() - Force shutdown IPA pipe
 * @pdev: pdev obj
 *
 * Return: void
 */
void ucfg_ipa_uc_force_pipe_shutdown(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_ipa_flush() - flush IPA exception path SKB's
 * @pdev: pdev obj
 *
 * Return: None
 */
void ucfg_ipa_flush(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_ipa_suspend() - Suspend IPA
 * @pdev: pdev obj
 *
 * Return: QDF STATUS
 */
QDF_STATUS ucfg_ipa_suspend(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_ipa_resume() - Resume IPA
 * @pdev: pdev obj
 *
 * Return: QDF STATUS
 */
QDF_STATUS ucfg_ipa_resume(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_ipa_uc_ol_init() - Initialize IPA uC offload
 * @pdev: pdev obj
 * @osdev: OS dev
 *
 * Return: QDF STATUS
 */
QDF_STATUS ucfg_ipa_uc_ol_init(struct wlan_objmgr_pdev *pdev,
			       qdf_device_t osdev);

/**
 * ucfg_ipa_uc_ol_deinit() - Deinitialize IPA uC offload
 * @pdev: pdev obj
 *
 * Return: QDF STATUS
 */
QDF_STATUS ucfg_ipa_uc_ol_deinit(struct wlan_objmgr_pdev *pdev);

#else

static inline bool ucfg_ipa_is_present(void)
{
	return false;
}

static inline void ucfg_ipa_update_config(struct wlan_ipa_config *config)
{
}

static inline
QDF_STATUS ucfg_ipa_set_dp_handle(struct wlan_objmgr_psoc *psoc,
				     void *dp_soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_ipa_set_txrx_handle(struct wlan_objmgr_psoc *psoc,
				    void *txrx_handle)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_ipa_set_perf_level(struct wlan_objmgr_pdev *pdev,
				   uint64_t tx_packets, uint64_t rx_packets)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void ucfg_ipa_uc_info(struct wlan_objmgr_pdev *pdev)
{
}

static inline
void ucfg_ipa_uc_stat(struct wlan_objmgr_pdev *pdev)
{
}

static inline
void ucfg_ipa_uc_rt_debug_host_dump(struct wlan_objmgr_pdev *pdev)
{
}

static inline
void ucfg_ipa_dump_info(struct wlan_objmgr_pdev *pdev)
{
}

static inline
void ucfg_ipa_uc_stat_request(struct wlan_objmgr_pdev *pdev,
			      uint8_t reason)
{
}

static inline
void ucfg_ipa_uc_stat_query(struct wlan_objmgr_pdev *pdev,
			    uint32_t *ipa_tx_diff, uint32_t *ipa_rx_diff)
{
}

static inline
void ucfg_ipa_reg_sap_xmit_cb(struct wlan_objmgr_pdev *pdev, void *cb)
{
}

static inline
void ucfg_ipa_reg_send_to_nw_cb(struct wlan_objmgr_pdev *pdev, void *cb)
{
}

static inline
void ucfg_ipa_set_mcc_mode(struct wlan_objmgr_pdev *pdev, bool mcc_mode)
{
}

static inline
void ucfg_ipa_set_dfs_cac_tx(struct wlan_objmgr_pdev *pdev, bool tx_block)
{
}

static inline
void ucfg_ipa_set_ap_ibss_fwd(struct wlan_objmgr_pdev *pdev, bool intra_bss)
{
}

static inline
void ucfg_ipa_uc_force_pipe_shutdown(struct wlan_objmgr_pdev *pdev)
{
}

static inline
void ucfg_ipa_flush(struct wlan_objmgr_pdev *pdev)
{
}

static inline
QDF_STATUS ucfg_ipa_suspend(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_ipa_resume(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_ipa_uc_ol_init(struct wlan_objmgr_pdev *pdev,
			       qdf_device_t osdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS ucfg_ipa_uc_ol_deinit(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* IPA_OFFLOAD */
#endif /* _WLAN_IPA_UCFG_API_H_ */
