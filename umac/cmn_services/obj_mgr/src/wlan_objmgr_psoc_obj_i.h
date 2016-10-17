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
  * DOC: Public APIs to perform operations on Global objects
  */
#ifndef _WLAN_OBJMGR_PSOC_OBJ_I_H_
#define _WLAN_OBJMGR_PSOC_OBJ_I_H_

/**
 * wlan_objmgr_psoc_pdev_attach() - store pdev in psoc's pdev list
 * @psoc - PSOC object
 * @pdev - PDEV object
 *
 * Attaches PDEV to PSOC, allocates PDEV id
 *
 * Return: SUCCESS
 *         Failure (Max PDEVs are exceeded)
 */
QDF_STATUS wlan_objmgr_psoc_pdev_attach(struct wlan_objmgr_psoc *psoc,
					 struct wlan_objmgr_pdev *pdev);

/**
 * wlan_objmgr_psoc_pdev_detach() - remove pdev from psoc's pdev list
 * @psoc - PSOC object
 * @pdev - PDEV object
 *
 * detaches PDEV to PSOC, frees PDEV id
 *
 * Return: SUCCESS
 *         Failure (No PDEVs are present)
 */
QDF_STATUS wlan_objmgr_psoc_pdev_detach(struct wlan_objmgr_psoc *psoc,
						struct wlan_objmgr_pdev *pdev);

/**
 * wlan_objmgr_psoc_vdev_attach() - store vdev in psoc's vdev list
 * @psoc - PSOC object
 * @vdev - VDEV object
 *
 * Attaches VDEV to PSOC, allocates VDEV id
 *
 * Return: SUCCESS
 *         Failure (Max VDEVs are exceeded)
 */
QDF_STATUS wlan_objmgr_psoc_vdev_attach(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_vdev *vdev);

/**
 * wlan_objmgr_psoc_vdev_detach() - remove vdev from psoc's vdev list
 * @psoc - PSOC object
 * @vdev - VDEV object
 *
 * detaches VDEV to PSOC, frees VDEV id
 *
 * Return: SUCCESS
 *         Failure (No VDEVs are present)
 */
QDF_STATUS wlan_objmgr_psoc_vdev_detach(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_vdev *vdev);

/**
 * wlan_objmgr_psoc_peer_attach() - store peer in psoc's peer table
 * @psoc - PSOC object
 * @peer - PEER object
 *
 * Attaches PEER to PSOC, derives the HASH, add peer to its peer list
 *
 * Return: SUCCESS
 *         Failure (Max PEERs are exceeded)
 */
QDF_STATUS wlan_objmgr_psoc_peer_attach(struct wlan_objmgr_psoc *psoc,
					 struct wlan_objmgr_peer *peer);

/**
 * wlan_objmgr_psoc_peer_detach() - remove peer from psoc's peer table
 * @psoc - PSOC object
 * @peer - PEER object
 *
 * detaches PEER to PSOC, removes the peer from the peer list
 *
 * Return: SUCCESS
 *         Failure (PEER is not present)
 */
QDF_STATUS wlan_objmgr_psoc_peer_detach(struct wlan_objmgr_psoc *psoc,
						struct wlan_objmgr_peer *peer);

/**
 * wlan_objmgr_psoc_object_attach() - attach psoc to global object
 * @psoc - PSOC object
 *
 * attaches PSOC to global psoc list
 *
 * Return: SUCCESS
 *         Failure (Max supported PSOCs exceeded)
 */
QDF_STATUS wlan_objmgr_psoc_object_attach(
			struct wlan_objmgr_psoc *psoc);

/**
 * wlan_objmgr_psoc_object_detach() - detach psoc from global object
 * @psoc - PSOC object
 *
 * detaches PSOC from global psoc list
 *
 * Return: SUCCESS
 *         Failure (if list is empty and PSOC is not present)
 */
QDF_STATUS wlan_objmgr_psoc_object_detach(
			struct wlan_objmgr_psoc *psoc);

#endif /* _WLAN_OBJMGR_PSOC_OBJ_I_H_ */
