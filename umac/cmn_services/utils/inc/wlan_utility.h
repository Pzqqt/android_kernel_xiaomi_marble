/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: Contains mandatory API from legacy
 */

#ifndef _WLAN_UTILITY_H_
#define _WLAN_UTILITY_H_

#include <qdf_types.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>

/**
 * struct wlan_find_vdev_filter - find vdev filter object. this can be extended
 * @ifname:           interface name of vdev
 * @found_vdev:       found vdev object matching one or more of above params
 */
struct wlan_find_vdev_filter {
	char *ifname;
	struct wlan_objmgr_vdev *found_vdev;
};

#ifdef CMN_VDEV_MLME_SM_ENABLE
/**
 * struct wlan_vdev_ch_check_filter - vdev chan check filter object
 * @flag:     matches or not
 * @vdev:     vdev to be checked against all the active vdevs
 */
struct wlan_vdev_ch_check_filter {
	uint8_t flag;
	struct wlan_objmgr_vdev *vdev;
};
#endif

/**
 * wlan_chan_to_freq() - converts channel to frequency
 * @chan: channel number
 *
 * @return frequency of the channel
 */
uint32_t wlan_chan_to_freq(uint8_t chan);

/**
 * wlan_freq_to_chan() - converts frequency to channel
 * @freq: frequency
 *
 * Return: channel of frequency
 */
uint8_t wlan_freq_to_chan(uint32_t freq);

/**
 * wlan_is_ie_valid() - Determine if an IE sequence is valid
 * @ie: Pointer to the IE buffer
 * @ie_len: Length of the IE buffer @ie
 *
 * This function validates that the IE sequence is valid by verifying
 * that the sum of the lengths of the embedded elements match the
 * length of the sequence.
 *
 * Note well that a 0-length IE sequence is considered valid.
 *
 * Return: true if the IE sequence is valid, false if it is invalid
 */
bool wlan_is_ie_valid(const uint8_t *ie, size_t ie_len);

/**
 * wlan_get_ie_ptr_from_eid() - Find out ie from eid
 * @eid: element id
 * @ie: source ie address
 * @ie_len: source ie length
 *
 * Return: vendor ie address - success
 *         NULL - failure
 */
const uint8_t *wlan_get_ie_ptr_from_eid(uint8_t eid,
					const uint8_t *ie,
					int ie_len);

/**
 * wlan_get_vendor_ie_ptr_from_oui() - Find out vendor ie
 * @oui: oui buffer
 * @oui_size: oui size
 * @ie: source ie address
 * @ie_len: source ie length
 *
 * This function find out vendor ie by pass source ie and vendor oui.
 *
 * Return: vendor ie address - success
 *         NULL - failure
 */
const uint8_t *wlan_get_vendor_ie_ptr_from_oui(const uint8_t *oui,
					       uint8_t oui_size,
					       const uint8_t *ie,
					       uint16_t ie_len);

/**
 * wlan_get_ext_ie_ptr_from_ext_id() - Find out ext ie
 * @oui: oui buffer
 * @oui_size: oui size
 * @ie: source ie address
 * @ie_len: source ie length
 *
 * This function find out ext ie from ext id (passed oui)
 *
 * Return: vendor ie address - success
 *         NULL - failure
 */
const uint8_t *wlan_get_ext_ie_ptr_from_ext_id(const uint8_t *oui,
					       uint8_t oui_size,
					       const uint8_t *ie,
					       uint16_t ie_len);

/**
 * wlan_is_emulation_platform() - check if platform is emulation based
 * @phy_version - psoc nif phy_version
 *
 * Return: boolean value based on platform type
 */
bool wlan_is_emulation_platform(uint32_t phy_version);

/**
 * wlan_get_pdev_id_from_vdev_id() - Helper func to derive pdev id from vdev_id
 * @psoc: psoc object
 * @vdev_id: vdev identifier
 * @dbg_id: object manager debug id
 *
 * This function is used to derive the pdev id from vdev id for a psoc
 *
 * Return : pdev_id - +ve integer for success and WLAN_INVALID_PDEV_ID
 *          for failure
 */
uint32_t wlan_get_pdev_id_from_vdev_id(struct wlan_objmgr_psoc *psoc,
				 uint8_t vdev_id,
				 wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_util_get_vdev_by_ifname() - function to return vdev object from psoc
 * matching given interface name
 * @psoc: psoc object
 * @ifname: interface name
 * @ref_id: object manager ref id
 *
 * This function returns vdev object from psoc by interface name. If found this
 * will also take reference with given ref_id
 *
 * Return : vdev object if found, NULL otherwise
 */
struct wlan_objmgr_vdev *wlan_util_get_vdev_by_ifname(
				struct wlan_objmgr_psoc *psoc, char *ifname,
				wlan_objmgr_ref_dbgid ref_id);

/**
 * wlan_util_vdev_get_if_name() - get vdev's interface name
 * @vdev: VDEV object
 *
 * API to get vdev's interface name
 *
 * Return:
 * @id: vdev's interface name
 */
uint8_t *wlan_util_vdev_get_if_name(struct wlan_objmgr_vdev *vdev);

/*
 * wlan_util_is_vdev_active() - Check for vdev active
 * @pdev: pdev pointer
 * @dbg_id: debug id for ref counting
 *
 * @Return: QDF_STATUS_SUCCESS in case of vdev active
 */
QDF_STATUS wlan_util_is_vdev_active(struct wlan_objmgr_pdev *pdev,
				    wlan_objmgr_ref_dbgid dbg_id);

/*
 * wlan_vdev_is_up() - Check for vdev is in UP state
 * @vdev: vdev pointer
 *
 * @Return: true in case of vdev is in UP state
 */
bool wlan_vdev_is_up(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_util_pdev_vdevs_deschan_match() - function to check des channel matches
 *                                        with other vdevs in pdev
 * @pdev: pdev object
 * @vdev: vdev object
 * @ref_id: object manager ref id
 *
 * This function checks the vdev desired channel with other vdev channels
 *
 * Return : SUCCESS, if it matches, otherwise FAILURE
 */
QDF_STATUS wlan_util_pdev_vdevs_deschan_match(struct wlan_objmgr_pdev *pdev,
					      struct wlan_objmgr_vdev *vdev,
					      wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_util_change_map_index() - function to set/reset given index bit
 * @map: bitmpap
 * @id: bit index
 * @set: 1 for set, 0 of reset
 *
 * This function set/reset given index bit
 *
 * Return : void
 */
void wlan_util_change_map_index(uint32_t *map, uint8_t id, uint8_t set);

/**
 * wlan_util_map_index_is_set() - function to check whether given index bit is
 *                                set
 * @map: bitmpap
 * @id: bit index
 *
 * This function checks the given index bit is set
 *
 * Return : true, if bit is set, otherwise false
 */
bool wlan_util_map_index_is_set(uint32_t *map, uint8_t id);

/**
 * wlan_pdev_chan_change_pending_vdevs() - function to test/set channel change
 *                                         pending flag
 * @pdev: pdev object
 * @vdev_id_map: bitmap to derive channel change vdevs
 * @ref_id: object manager ref id
 *
 * This function test/set channel change pending flag
 *
 * Return : SUCCESS, if it iterates through all vdevs, otherwise FAILURE
 */
QDF_STATUS wlan_pdev_chan_change_pending_vdevs(struct wlan_objmgr_pdev *pdev,
					       uint32_t *vdev_id_map,
					       wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_chan_eq() - function to check whether both channels are same
 * @chan1: channel1 object
 * @chan2: channel2 object
 *
 * This function checks the chan1 and chan2 are same
 *
 * Return : SUCCESS, if it matches, otherwise FAILURE
 */
QDF_STATUS wlan_chan_eq(struct wlan_channel *chan1, struct wlan_channel *chan2);

#endif /* _WLAN_UTILITY_H_ */
