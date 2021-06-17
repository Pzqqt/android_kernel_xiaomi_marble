/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: contains MLO manager public file containing peer functionality
 */
#ifndef _WLAN_MLO_MGR_PEER_H_
#define _WLAN_MLO_MGR_PEER_H_

/**
 * mlo_peer_create - Initiatiate peer create on secondary link(s)
 * by posting a message
 *
 * @vdev: pointer to vdev
 * @peer: pointer to peer context
 * @mlo_ie: MLO information element
 * @aid: association ID
 *
 * Initiate the peer on the second link
 *
 * Return: none
 */
void mlo_peer_create(struct wlan_objmgr_vdev *vdev,
			       struct wlan_objmgr_peer *peer, uint8_t *mlo_ie,
			       uint8_t aid);

/**
 * mlo_peer_attach - Attaches the peer by updating the MLO peer context with
 * the new link information
 *
 * @vdev: pointer to vdev
 * @peer: pointer to peer context
 *
 * Return: none
 */
void mlo_peer_attach(struct wlan_objmgr_vdev *vdev,
		     struct wlan_objmgr_peer *peer);

/**
 * mlo_peer_setup_failed_notify - Notify MLO manager that peer setup has failed
 * and to cleanup by deleting the partner peers
 *
 * @vdev: pointer to vdev
 *
 * This API is called in scenarios where peer create or peer assoc fails
 *
 * Return: none
 */
void mlo_peer_setup_failed_notify(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_peer_disconnect_notify - Notify MLO manager that peer has disconnected
 * and to clean up by deleting partner peers
 *
 * @vdev: pointer to vdev
 *
 * Return: none
 */
void mlo_peer_disconnect_notify(struct wlan_objmgr_peer *peer);

/**
 * wlan_peer_delete_complete - Notify MLO manager that peer delete is completed
 * and to clean up by unlinking the peer object
 *
 * @peer: pointer to peer context
 *
 * Return: none
 */
void wlan_peer_delete_complete(struct wlan_objmgr_peer *peer);

/**
 * mlo_peer_delete - Delete the peer object
 *
 * @peer: pointer to peer context
 *
 * Return: none
 */
void mlo_peer_delete(struct wlan_objmgr_peer *peer);

/**
 * is_mlo_all_peer_links_deleted - Check if all the peer links are deleted
 *
 * Return: true if all the peer links are deleted, false otherwise
 */
bool is_mlo_all_peer_links_deleted(void);

/**
 * mlo_get_aid - Get the association ID
 *
 * @vdev: pointer to vdev
 *
 * Return: AID value
 */
uint8_t mlo_get_aid(struct wlan_objmgr_vdev *vdev);

#endif
