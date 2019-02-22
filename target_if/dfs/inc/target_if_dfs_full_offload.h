/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

/**
 * DOC: target_if_dfs_full_offload.h
 * This file contains dfs target interface for full-offload.
 */

#ifndef _TARGET_IF_DFS_FULL_OFFLOAD_H_
#define _TARGET_IF_DFS_FULL_OFFLOAD_H_

/**
 * target_if_dfs_reg_offload_events() - registers dfs events for full offload.
 * @psoc: Pointer to psoc object.
 *
 * Return: QDF_STATUS
 */
#if defined(WLAN_DFS_FULL_OFFLOAD)
QDF_STATUS target_if_dfs_reg_offload_events(struct wlan_objmgr_psoc *psoc);
#else
static QDF_STATUS
target_if_dfs_reg_offload_events(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * target_process_bang_radar_cmd() - fill unit test args and send bangradar
 * command to firmware.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_unit_test: Pointer to dfs_unit_test structure.
 *
 * Return: QDF_STATUS
 */
#if defined(WLAN_DFS_FULL_OFFLOAD)
QDF_STATUS target_process_bang_radar_cmd(struct wlan_objmgr_pdev *pdev,
		struct dfs_emulate_bang_radar_test_cmd *dfs_unit_test);
#else
static QDF_STATUS target_process_bang_radar_cmd(struct wlan_objmgr_pdev *pdev,
		struct dfs_emulate_bang_radar_test_cmd *dfs_unit_test)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(QCA_SUPPORT_AGILE_DFS)
/**
 * target_send_ocac_abort_cmd() - Send off channel CAC abort to target for
 * to cancel current offchannel CAC
 * @pdev: Pointer to DFS pdev object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS target_send_ocac_abort_cmd(struct wlan_objmgr_pdev *pdev);
/**
 * target_send_agile_ch_cfg_cmd() - Send agile channel to target for
 * off channel precac.
 * @pdev: Pointer to DFS pdev object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS target_send_agile_ch_cfg_cmd(struct wlan_objmgr_pdev *pdev,
					uint8_t *ch_freq);
#else
static inline QDF_STATUS
target_send_ocac_abort_cmd(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
target_send_agile_ch_cfg_cmd(struct wlan_objmgr_pdev *pdev,
			     uint8_t *ch_freq)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(WLAN_DFS_FULL_OFFLOAD) && defined(QCA_DFS_NOL_OFFLOAD)
/**
 * target_send_usenol_pdev_param - send usenol pdev param to FW.
 * @pdev: Pointer to pdev object.
 * @usenol: Value of user configured usenol.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS target_send_usenol_pdev_param(struct wlan_objmgr_pdev *pdev,
					 bool usenol);

/**
 * target_send_subchan_marking_pdev_param - Send subchannel marking
 * pdev param to FW.
 * @pdev: Pointer to pdev object.
 * @subchanmark: Value of user configured subchannel_marking.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_send_subchan_marking_pdev_param(struct wlan_objmgr_pdev *pdev,
				       bool subchanmark);

#else
static inline QDF_STATUS
target_send_usenol_pdev_param(struct wlan_objmgr_pdev *pdev,
			      bool usenol)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
target_send_subchan_marking_pdev_param(struct wlan_objmgr_pdev *pdev,
				       bool subchanmark)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif /* _TARGET_IF_DFS_FULL_OFFLOAD_H_ */

