/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
 * DOC: This file has the DFS dispatcher API which is exposed to outside of DFS
 * component.
 */

#ifdef QCA_SUPPORT_DFS_CHAN_POSTNOL
/**
 * ucfg_dfs_set_postnol_freq() - Set PostNOL freq.
 * @pdev: Pointer to DFS pdev object.
 * @postnol_freq: User configured freq to switch to, post NOL, in MHZ.
 *
 */
QDF_STATUS ucfg_dfs_set_postnol_freq(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t postnol_freq);

/**
 * ucfg_dfs_set_postnol_mode() - Set PostNOL mode.
 * @pdev: Pointer to DFS pdev object.
 * @postnol_mode: User configured mode to switch to, post NOL, in MHZ.
 *
 */
QDF_STATUS ucfg_dfs_set_postnol_mode(struct wlan_objmgr_pdev *pdev,
				     uint8_t postnol_mode);

/**
 * ucfg_dfs_set_postnol_cfreq2() - Set PostNOL secondary center frequency.
 * @pdev: Pointer to DFS pdev object.
 * @postnol_freq: User configured secondary center frequency to switch to,
 * post NOL, in MHZ.
 *
 */
QDF_STATUS ucfg_dfs_set_postnol_cfreq2(struct wlan_objmgr_pdev *pdev,
				       qdf_freq_t postnol_cfreq2);

/**
 * ucfg_dfs_get_postnol_freq() - Get PostNOL freq.
 * @pdev: Pointer to DFS pdev object.
 * @postnol_freq: Pointer to user configured freq to switch to, post NOL.
 *
 */
QDF_STATUS ucfg_dfs_get_postnol_freq(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t *postnol_freq);

/**
 * ucfg_dfs_get_postnol_mode() - Set PostNOL mode.
 * @pdev: Pointer to DFS pdev object.
 * @postnol_mode: Pointer to user configured mode to switch to, post NOL.
 *
 */
QDF_STATUS ucfg_dfs_get_postnol_mode(struct wlan_objmgr_pdev *pdev,
				     uint8_t *postnol_mode);

/**
 * ucfg_dfs_get_postnol_cfreq2() - Set PostNOL secondary center frequency.
 * @pdev: Pointer to DFS pdev object.
 * @postnol_freq: Pointer to user configured secondary center frequency to
 * switch to post NOL.
 *
 */
QDF_STATUS ucfg_dfs_get_postnol_cfreq2(struct wlan_objmgr_pdev *pdev,
				       qdf_freq_t *postnol_cfreq2);
#else
static inline QDF_STATUS
ucfg_dfs_set_postnol_freq(struct wlan_objmgr_pdev *pdev,
			  qdf_freq_t postnol_freq)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_dfs_set_postnol_mode(struct wlan_objmgr_pdev *pdev,
			  uint8_t postnol_mode)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_dfs_set_postnol_cfreq2(struct wlan_objmgr_pdev *pdev,
			    qdf_freq_t postnol_cfreq2)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_dfs_get_postnol_freq(struct wlan_objmgr_pdev *pdev,
			  qdf_freq_t *postnol_freq)
{
	*postnol_freq = 0;
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_dfs_get_postnol_mode(struct wlan_objmgr_pdev *pdev,
			  uint8_t *postnol_mode)
{
	*postnol_mode = CH_WIDTH_INVALID;
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_dfs_get_postnol_cfreq2(struct wlan_objmgr_pdev *pdev,
			    qdf_freq_t *postnol_cfreq2)
{
	*postnol_cfreq2 = 0;
	return QDF_STATUS_SUCCESS;
}
#endif
