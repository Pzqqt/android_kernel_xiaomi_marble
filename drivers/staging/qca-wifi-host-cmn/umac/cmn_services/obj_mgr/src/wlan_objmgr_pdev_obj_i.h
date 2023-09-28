/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
  * DOC: Public APIs to perform operations on PDEV object
  */
#ifndef _WLAN_OBJMGR_PDEV_OBJ_I_H_
#define _WLAN_OBJMGR_PDEV_OBJ_I_H_

/**
 * wlan_objmgr_pdev_vdev_attach() - attach vdev to pdev
 * @pdev: PDEV object
 * @vdev: VDEV object
 *
 * API to be used for adding the VDEV object in PDEV's VDEV object list
 *
 * Return: SUCCESS on successful storing of VDEV object
 *         FAILURE
 */
QDF_STATUS wlan_objmgr_pdev_vdev_attach(struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_vdev *vdev);

/**
 * wlan_objmgr_pdev_vdev_detach() - detach vdev from pdev
 * @pdev: PDEV object
 * @vdev: VDEV object
 *
 * API to be used for removing the VDEV object from PDEV's VDEV object list
 *
 * Return: SUCCESS on successful removal of VDEV object
 *         FAILURE
 */
QDF_STATUS wlan_objmgr_pdev_vdev_detach(struct wlan_objmgr_pdev *pdev,
				struct wlan_objmgr_vdev *vdev);

#endif /* _WLAN_OBJMGR_PDEV_OBJ_I_H_*/
