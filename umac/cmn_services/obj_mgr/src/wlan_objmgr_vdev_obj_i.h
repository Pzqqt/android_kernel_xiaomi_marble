/*
 * Copyright (c) 2016,2018 The Linux Foundation. All rights reserved.
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
 * DOC: Public APIs to perform operations on VDEV objects
 */
#ifndef _WLAN_OBJMGR_VDEV_OBJ_I_H_
#define _WLAN_OBJMGR_VDEV_OBJ_I_H_

/**
 * wlan_objmgr_for_each_vdev_peer() - iterate over each peer for @vdev
 * @vdev: the vdev whose peers should be iterated
 * @peer: peer object cursor
 *
 * Note: The caller is responsible for grabbing @vdev's object lock before
 * using this iterator
 */
#define wlan_objmgr_for_each_vdev_peer(vdev, peer) \
	qdf_list_for_each(&(vdev)->vdev_objmgr.wlan_peer_list, peer, vdev_peer)

/**
 * wlan_objmgr_vdev_peer_attach() - attach peer to vdev peer list
 * @vdev: VDEV object
 * @peer: PEER object
 *
 * Attaches PEER to VDEV, stores it in VDEV's peer list
 *
 * Return: SUCCESS
 *         Failure (Max PEERs are exceeded)
 */
QDF_STATUS wlan_objmgr_vdev_peer_attach(struct wlan_objmgr_vdev *vdev,
			struct wlan_objmgr_peer *peer);

/**
 * wlan_objmgr_vdev_peer_detach() - detach peer from vdev peer list
 * @vdev: VDEV object
 * @peer: PEER object
 *
 * detaches PEER from VDEV's peer list
 *
 * Return: SUCCESS
 *         Failure (No PEERs are present)
 */
QDF_STATUS wlan_objmgr_vdev_peer_detach(struct wlan_objmgr_vdev *vdev,
			struct wlan_objmgr_peer *peer);

#endif /* _WLAN_OBJMGR_VDEV_OBJ_I_H_*/
