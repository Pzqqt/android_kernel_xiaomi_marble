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
 * DOC: Add 11d utility functions
 */

#ifdef TARGET_11D_SCAN
/**
 * reg_run_11d_state_machine() - 11d state machine function.
 * @psoc: soc context
 */
void reg_run_11d_state_machine(struct wlan_objmgr_psoc *psoc);

/**
 * reg_set_11d_country() - Set the 11d regulatory country
 * @pdev: pdev device for country information
 * @country: country value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_11d_country(struct wlan_objmgr_pdev *pdev, uint8_t *country);

/**
 * reg_is_11d_scan_inprogress() - Check 11d scan is supported
 * @psoc: psoc ptr
 *
 * Return: true if 11d scan supported, else false.
 */
bool reg_is_11d_scan_inprogress(struct wlan_objmgr_psoc *psoc);

/**
 * reg_save_new_11d_country() - Save the 11d new country
 * @psoc: psoc for country information
 * @country: country value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_save_new_11d_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country);

/**
 * reg_is_11d_offloaded() - whether 11d offloaded supported or not
 * @psoc: psoc ptr
 *
 * Return: bool
 */
bool reg_is_11d_offloaded(struct wlan_objmgr_psoc *psoc);

/**
 * reg_11d_enabled_on_host() - know whether 11d enabled on host
 * @psoc: psoc ptr
 *
 * Return: bool
 */
bool reg_11d_enabled_on_host(struct wlan_objmgr_psoc *psoc);

/**
 * reg_11d_vdev_created_update() - vdev obj create callback
 * @vdev: vdev pointer
 *
 * updates 11d state when a vdev is created.
 *
 * Return: Success or Failure
 */
QDF_STATUS reg_11d_vdev_created_update(struct wlan_objmgr_vdev *vdev);

/**
 * reg_11d_vdev_delete_update() - update 11d state upon vdev delete
 * @vdev: vdev pointer
 *
 * Return: Success or Failure
 */
QDF_STATUS reg_11d_vdev_delete_update(struct wlan_objmgr_vdev *vdev);

/**
 * reg_set_11d_offloaded() - Set 11d offloaded flag
 * @psoc: psoc ptr
 * @val: 11d offloaded value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_11d_offloaded(struct wlan_objmgr_psoc *psoc, bool val);

#else

static inline void reg_run_11d_state_machine(struct wlan_objmgr_psoc *psoc)
{
}

static inline QDF_STATUS reg_set_11d_country(struct wlan_objmgr_pdev *pdev,
					     uint8_t *country)
{
	return QDF_STATUS_SUCCESS;
}

static inline bool reg_is_11d_scan_inprogress(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

static inline QDF_STATUS reg_save_new_11d_country(struct wlan_objmgr_psoc *psoc,
						  uint8_t *country)
{
	return QDF_STATUS_SUCCESS;
}

static inline bool reg_is_11d_offloaded(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

static inline bool reg_11d_enabled_on_host(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

static inline QDF_STATUS reg_11d_vdev_created_update(
	struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS reg_11d_vdev_delete_update(
	struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS reg_set_11d_offloaded(struct wlan_objmgr_psoc *psoc,
					       bool val)
{
	return QDF_STATUS_SUCCESS;
}
#endif
