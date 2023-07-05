/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
 * wlan_objmgr_for_each_psoc_pdev() - iterate over each pdev for @psoc
 * @psoc: the psoc whose pdevs should be iterated
 * @pdev_id: pdev Id index cursor
 * @pdev: pdev object cursor
 *
 * Note: The caller is responsible for grabbing @psoc's object lock before
 * using this iterator
 */
#define wlan_objmgr_for_each_psoc_pdev(psoc, pdev_id, pdev) \
	for (pdev_id = 0; pdev_id < WLAN_UMAC_MAX_PDEVS; pdev_id++) \
		if ((pdev = (psoc)->soc_objmgr.wlan_pdev_list[pdev_id]))

/**
 * wlan_objmgr_for_each_psoc_vdev() - iterate over each vdev for @psoc
 * @psoc: the psoc whose vdevs should be iterated
 * @vdev_id: vdev Id index cursor
 * @vdev: vdev object cursor
 *
 * Note: The caller is responsible for grabbing @psoc's object lock before
 * using this iterator
 */
#define wlan_objmgr_for_each_psoc_vdev(psoc, vdev_id, vdev) \
	for (vdev_id = 0; vdev_id < WLAN_UMAC_PSOC_MAX_VDEVS; vdev_id++) \
		if ((vdev = (psoc)->soc_objmgr.wlan_vdev_list[vdev_id]))

/**
 * wlan_objmgr_for_each_refs() - iterate non-zero ref counts in @ref_id_dbg
 * @ref_id_dbg: the ref count array to iterate
 * @ref_id: the reference Id index cursor
 * @refs: the ref count cursor
 *
 * Note: The caller is responsible for grabbing @ref_id_dbg's parent object lock
 * before using this iterator
 */
#define wlan_objmgr_for_each_refs(ref_id_dbg, ref_id, refs) \
	for (ref_id = 0; ref_id < WLAN_REF_ID_MAX; ref_id++) \
		if ((refs = qdf_atomic_read(&(ref_id_dbg)[ref_id])) > 0)

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
#endif /* _WLAN_OBJMGR_PSOC_OBJ_I_H_ */
