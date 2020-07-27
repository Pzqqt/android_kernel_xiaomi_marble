/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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

/**
 * DOC: Contains mandatory API from legacy
 */

#ifndef _WLAN_UTILITY_H_
#define _WLAN_UTILITY_H_

#include <qdf_types.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>

#define TGT_INVALID_SNR         (0)
#define TGT_MAX_SNR             (TGT_NOISE_FLOOR_DBM * (-1))
#define TGT_NOISE_FLOOR_DBM     (-96)
#define TGT_IS_VALID_SNR(x)     ((x) >= 0 && (x) < TGT_MAX_SNR)
#define TGT_IS_VALID_RSSI(x)    ((x) != 0xFF)

/**
 * struct wlan_vdev_ch_check_filter - vdev chan check filter object
 * @flag:     matches or not
 * @vdev:     vdev to be checked against all the active vdevs
 */
struct wlan_vdev_ch_check_filter {
	uint8_t flag;
	struct wlan_objmgr_vdev *vdev;
};

/**
 * struct wlan_peer_count- vdev connected peer count
 * @opmode: QDF mode
 * @peer_count: peer count
 **/
struct wlan_op_mode_peer_count {
	enum QDF_OPMODE opmode;
	uint16_t peer_count;
};

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
 * wlan_util_is_vdev_active() - Check for vdev active
 * @pdev: pdev pointer
 * @dbg_id: debug id for ref counting
 *
 * Return: QDF_STATUS_SUCCESS in case of vdev active
 *          QDF_STATUS_E_INVAL, if dev is not active
 */
QDF_STATUS wlan_util_is_vdev_active(struct wlan_objmgr_pdev *pdev,
				    wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_vdev_is_up() - Check for vdev is in UP state
 * @vdev: vdev pointer
 *
 * Return: QDF_STATUS_SUCCESS, if vdev is in up, otherwise QDF_STATUS_E_FAILURE
 */
QDF_STATUS wlan_vdev_is_up(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_util_pdev_vdevs_deschan_match() - function to check des channel matches
 *                                        with other vdevs in pdev
 * @pdev: pdev object
 * @vdev: vdev object
 * @ref_id: object manager ref id
 *
 * This function checks the vdev desired channel with other vdev channels
 *
 * Return: QDF_STATUS_SUCCESS, if it matches, otherwise QDF_STATUS_E_FAILURE
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
 * Return: void
 */
void wlan_util_change_map_index(unsigned long *map, uint8_t id, uint8_t set);

/**
 * wlan_util_map_index_is_set() - function to check whether given index bit is
 *                                set
 * @map: bitmpap
 * @id: bit index
 *
 * This function checks the given index bit is set
 *
 * Return: true, if bit is set, otherwise false
 */
bool wlan_util_map_index_is_set(unsigned long *map, uint8_t id);

/**
 * wlan_util_map_is_any_index_set() - Check if any bit is set in given bitmap
 * @map: bitmap
 * @nbytes: number of bytes in bitmap
 *
 * Return: true, if any of the bit is set, otherwise false
 */
bool wlan_util_map_is_any_index_set(unsigned long *map, unsigned long nbytes);

/**
 * wlan_pdev_chan_change_pending_vdevs() - function to test/set channel change
 *                                         pending flag
 * @pdev: pdev object
 * @vdev_id_map: bitmap to derive channel change vdevs
 * @ref_id: object manager ref id
 *
 * This function test/set channel change pending flag
 *
 * Return: QDF_STATUS_SUCCESS, if it iterates through all vdevs,
 *         otherwise QDF_STATUS_E_FAILURE
 */
QDF_STATUS wlan_pdev_chan_change_pending_vdevs(struct wlan_objmgr_pdev *pdev,
					       unsigned long *vdev_id_map,
					       wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_pdev_chan_change_pending_vdevs_down() - function to test/set down
 *                                              change pending flag
 * @pdev: pdev object
 * @vdev_id_map: bitmap to derive channel change vdevs
 * @ref_id: object manager ref id
 *
 * This function test/set channel change pending flag
 *
 * Return: QDF_STATUS_SUCCESS, if it iterates through all vdevs,
 *         otherwise QDF_STATUS_E_FAILURE
 */
QDF_STATUS wlan_pdev_chan_change_pending_vdevs_down(
					struct wlan_objmgr_pdev *pdev,
					unsigned long *vdev_id_map,
					wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_pdev_chan_change_pending_ap_vdevs_down() - function to test/set channel
 *                                            change pending flag for AP VDEVs
 * @pdev: pdev object
 * @vdev_id_map: bitmap to derive channel change AP vdevs
 * @ref_id: object manager ref id
 *
 * This function test/set channel change pending flag for AP vdevs
 *
 * Return: QDF_STATUS_SUCCESS, if it iterates through all vdevs,
 *         otherwise QDF_STATUS_E_FAILURE
 */
QDF_STATUS wlan_pdev_chan_change_pending_ap_vdevs_down(
					struct wlan_objmgr_pdev *pdev,
					unsigned long *vdev_id_map,
					wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_chan_eq() - function to check whether both channels are same
 * @chan1: channel1 object
 * @chan2: channel2 object
 *
 * This function checks the chan1 and chan2 are same
 *
 * Return: QDF_STATUS_SUCCESS, if it matches, otherwise QDF_STATUS_E_FAILURE
 */
QDF_STATUS wlan_chan_eq(struct wlan_channel *chan1, struct wlan_channel *chan2);

/**
 * wlan_chan_copy() - function to copy channel
 * @tgt:  target channel object
 * @src:  src achannel object
 *
 * This function copies channel data from src to tgt
 *
 * Return: void
 */
void wlan_chan_copy(struct wlan_channel *tgt, struct wlan_channel *src);

/**
 * wlan_vdev_get_active_channel() - derives the vdev operating channel
 * @vdev:  VDEV object
 *
 * This function checks vdev state and return the channel pointer accordingly
 *
 * Return: active channel, if vdev chan config is valid
 *         NULL, if VDEV is in INIT or STOP state
 */
struct wlan_channel *wlan_vdev_get_active_channel
				(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_util_stats_get_rssi() - API to get rssi in dbm
 * @db2dbm_enabled: If db2dbm capability is enabled
 * @bcn_snr: beacon snr
 * @dat_snr: data snr
 * @rssi: rssi
 *
 * This function gets the rssi based on db2dbm support. If this feature is
 * present in hw then it means firmware directly sends rssi and no converstion
 * is required. If this capablity is not present then host needs to convert
 * snr to rssi
 *
 * Return: None
 */
void
wlan_util_stats_get_rssi(bool db2dbm_enabled, int32_t bcn_snr, int32_t dat_snr,
			 int8_t *rssi);

/**
 * wlan_util_is_pdev_restart_progress() - Check if any vdev is in restart state
 * @pdev: pdev pointer
 * @dbg_id: module id
 *
 * Iterates through all vdevs, checks if any VDEV is in RESTART_PROGRESS
 * substate
 *
 * Return: QDF_STATUS_SUCCESS,if any vdev is in RESTART_PROGRESS substate
 *         otherwise QDF_STATUS_E_FAILURE
 */
QDF_STATUS wlan_util_is_pdev_restart_progress(struct wlan_objmgr_pdev *pdev,
					      wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_util_is_pdev_scan_allowed() - Check for vdev is allowed to scan
 * @pdev: pdev pointer
 * @dbg_id: module id
 *
 * Iterates through all vdevs, checks if any VDEV is not either in S_INIT or in
 * S_UP state
 *
 * Return: QDF_STATUS_SUCCESS,if scan is allowed, otherwise QDF_STATUS_E_FAILURE
 */
QDF_STATUS wlan_util_is_pdev_scan_allowed(struct wlan_objmgr_pdev *pdev,
					  wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_util_get_peer_count_for_mode - This api gives vdev mode specific
 * peer count`
 * @pdev: PDEV object
 * @mode: Operation mode.
 *
 * Return: int- peer count for operating mode
 */
uint16_t wlan_util_get_peer_count_for_mode(struct wlan_objmgr_pdev *pdev,
					   enum QDF_OPMODE mode);

/**
 * wlan_minidump_host_data - Data structure type logged in Minidump
 * @WLAN_MD_CP_EXT_PDEV - ol_ath_softc_net80211
 * @WLAN_MD_CP_EXT_PSOC - ol_ath_soc_softc
 * @WLAN_MD_CP_EXT_VDEV - ieee80211vap
 * @WLAN_MD_CP_EXT_PEER - ieee80211_node
 * @WLAN_MD_DP_SOC - dp_soc
 * @WLAN_MD_DP_PDEV - dp_pdev
 * @WLAN_MD_DP_VDEV - dp_vdev
 * @WLAN_MD_DP_PEER - dp_peer
 * @WLAN_MD_DP_SRNG_REO_DEST - dp_srng type for reo dest
 * @WLAN_MD_DP_SRNG_REO_EXCEPTION - dp_srng type for reo exception
 * @WLAN_MD_DP_SRNG_REO_CMD - dp_srng type for reo cmd
 * @WLAN_MD_DP_SRNG_RX_REL - dp_srng type for reo release
 * @WLAN_MD_DP_SRNG_REO_REINJECT - dp_srng type for reo reinject
 * @WLAN_MD_DP_SRNG_REO_STATUS - dp_srng type for reo status
 * @WLAN_MD_DP_SRNG_TCL_DATA - dp_srng type for tcl data
 * @WLAN_MD_DP_SRNG_TCL_STATUS - dp_srng type for tcl status
 * @WLAN_MD_DP_SRNG_TX_COMP - dp_srng type for tcl comp
 * @WLAN_MD_DP_SRNG_WBM_DESC_REL - dp_srng_type for wbm desc rel
 * @WLAN_MD_DP_SRNG_WBM_IDLE_LINK - dp_srng type for wbm idle link
 * @WLAN_MD_DP_LINK_DESC_BANK - Wbm link_desc_bank
 * @WLAN_MD_DP_SRNG_RXDMA_MON_STATUS - dp_srng type for rxdma mon status
 * @WLAN_MD_DP_SRNG_RXDMA_MON_BUF - dp_srng type for rxdma mon buf
 * @WLAN_MD_DP_SRNG_RXDMA_MON_DST - dp_srng type for rxdma mon dest
 * @WLAN_MD_DP_SRNG_RXDMA_MON_DESC - dp_srng type for rxdma mon desc
 * @WLAN_MD_DP_SRNG_RXDMA_ERR_DST - dp_srng type for rxdma err dst
 * @WLAN_MD_DP_HAL_SOC - hal_soc
 * @WLAN_MD_OBJMGR_PSOC - wlan_objmgr_psoc
 * @WLAN_MD_OBJMGR_PSOC_TGT_INFO - wlan_objmgr_tgt_psoc_info
 * @WLAN_MD_OBJMGR_PDEV - wlan_objmgr_pdev
 * @WLAN_MD_OBJMGR_PDEV_MLME - pdev_mlme
 * @WLAN_MD_OBJMGR_VDEV - wlan_objmgr_vdev
 * @WLAN_MD_OBJMGR_VDEV_MLME -vdev mlme
 * @WLAN_MD_OBJMGR_VDEV_SM - wlan_sm
 * @WLAN_MD_MAX - Max value
 */
enum wlan_minidump_host_data {
	WLAN_MD_CP_EXT_PDEV,
	WLAN_MD_CP_EXT_PSOC,
	WLAN_MD_CP_EXT_VDEV,
	WLAN_MD_CP_EXT_PEER,
	WLAN_MD_DP_SOC,
	WLAN_MD_DP_PDEV,
	WLAN_MD_DP_VDEV,
	WLAN_MD_DP_PEER,
	WLAN_MD_DP_SRNG_REO_DEST,
	WLAN_MD_DP_SRNG_REO_EXCEPTION,
	WLAN_MD_DP_SRNG_REO_CMD,
	WLAN_MD_DP_SRNG_RX_REL,
	WLAN_MD_DP_SRNG_REO_REINJECT,
	WLAN_MD_DP_SRNG_REO_STATUS,
	WLAN_MD_DP_SRNG_TCL_DATA,
	WLAN_MD_DP_SRNG_TCL_CMD,
	WLAN_MD_DP_SRNG_TCL_STATUS,
	WLAN_MD_DP_SRNG_TX_COMP,
	WLAN_MD_DP_SRNG_WBM_DESC_REL,
	WLAN_MD_DP_SRNG_WBM_IDLE_LINK,
	WLAN_MD_DP_LINK_DESC_BANK,
	WLAN_MD_DP_SRNG_RXDMA_MON_STATUS,
	WLAN_MD_DP_SRNG_RXDMA_MON_BUF,
	WLAN_MD_DP_SRNG_RXDMA_MON_DST,
	WLAN_MD_DP_SRNG_RXDMA_MON_DESC,
	WLAN_MD_DP_SRNG_RXDMA_ERR_DST,
	WLAN_MD_DP_HAL_SOC,
	WLAN_MD_OBJMGR_PSOC,
	WLAN_MD_OBJMGR_PSOC_TGT_INFO,
	WLAN_MD_OBJMGR_PDEV,
	WLAN_MD_OBJMGR_PDEV_MLME,
	WLAN_MD_OBJMGR_VDEV,
	WLAN_MD_OBJMGR_VDEV_MLME,
	WLAN_MD_OBJMGR_VDEV_SM,
	WLAN_MD_MAX
};

void wlan_minidump_log(void *start_addr, size_t size,
		       void *psoc,
		       enum wlan_minidump_host_data type,
		       const char *name);

void wlan_minidump_remove(void *addr);

#endif /* _WLAN_UTILITY_H_ */
