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

#ifndef _WLAN_DFS_TGT_API_H_
#define _WLAN_DFS_TGT_API_H_

#include <wlan_dfs_public_struct.h>

#define WLAN_DFS_RESET_TIME_S 7
#define WLAN_DFS_WAIT (60 + WLAN_DFS_RESET_TIME_S) /* 60 seconds */
#define WLAN_DFS_WAIT_MS ((WLAN_DFS_WAIT) * 1000)  /*in MS*/

/* Command id to send test radar to firmware */
#define DFS_PHYERR_OFFLOAD_TEST_SET_RADAR 0

/* Segment ID corresponding to primary segment */
#define SEG_ID_PRIMARY 0

/* Segment ID corresponding to secondary segment */
#define SEG_ID_SECONDARY 1

/* dfs_radar_args_for_unit_test: Radar parameters to be sent in unit test cmd.
 * @IDX_CMD_ID:          Index id pointing to command id value
 * @IDX_PDEV_ID:         Index id pointing to pdev id value
 * @IDX_RADAR_PARAM1_ID: Index pointing to packed arguments value that includes
 *                         1). Segment ID,
 *                         2). Chirp information (is chirp or non chirp),
 *                         3). Frequency offset.
 *                         4). Detector ID.
 *
 * The packed argument structure is:
 *
 * ------------------------------32 bits arg----------------------------
 *
 * ------------19 bits-------|--2 bits-|-------8 bits------|1 bit|2 bits|
 * ______________________________________________________________________
 *|                          |   | |   | | | | | | | | | | |     |   |   |
 *|------19 Unused bits------| x | | x |x|x|x| |x|x|x|x| |x|  x  | x | x |
 *|__________________________|___|_|___|_|_|_|_|_|_|_|_|_|_|_____|___|___|
 *
 *                           |_________|___________________|_____|_______|
 *                              det.ID     freq.offset      Chirp  seg.ID
 *
 * @DFS_UNIT_TEST_NUM_ARGS:     Number of arguments for bangradar unit test
 *                              command.
 * @DFS_MAX_NUM_UNIT_TEST_ARGS: Maximum number of arguments for unit test
 *                              command in radar simulation.
 */
enum {
	IDX_CMD_ID = 0,
	IDX_PDEV_ID,
	IDX_RADAR_PARAM1_ID,
	DFS_UNIT_TEST_NUM_ARGS,
	DFS_MAX_NUM_UNIT_TEST_ARGS = DFS_UNIT_TEST_NUM_ARGS
};

#define SEG_ID_SHIFT         0
#define IS_CHIRP_SHIFT       2
#define FREQ_OFF_SHIFT       3
#define DET_ID_SHIFT        11
#define SEG_ID_MASK       0x03
#define IS_CHIRP_MASK     0x01
#define FREQ_OFFSET_MASK  0xFF
#define DET_ID_MASK       0x03

/**
 * struct dfs_emulate_bang_radar_test_cmd - Unit test command structure to send
 *                                          WMI command to firmware from host
 *                                          and simulate bangradar event.
 * @vdev_id: vdev id
 * @num_args: number of arguments
 * @args: arguments
 */
struct dfs_emulate_bang_radar_test_cmd {
	uint32_t vdev_id;
	uint32_t num_args;
	uint32_t args[DFS_MAX_NUM_UNIT_TEST_ARGS];
};

/**
 * struct vdev_adfs_complete_status - OCAC complete status event param
 * @vdev_id: Physical device identifier
 * @chan_freq: Channel number
 * @chan_width: Channel Width
 * @center_freq1 : For 20/40/80/160Mhz, it is the center of the corresponding
 * band. For 80P80/165MHz, it is the center of the left 80MHz.
 * @center_freq2 : It is valid and non-zero only for 80P80/165MHz. It indicates
 * the Center Frequency of the right 80MHz segment.
 * @ocac_status: off channel cac status
 */
struct vdev_adfs_complete_status {
	uint32_t vdev_id;
	uint32_t chan_freq;
	uint32_t chan_width;
	uint32_t center_freq1;
	uint32_t center_freq2;
	uint32_t ocac_status;
};

extern struct dfs_to_mlme global_dfs_to_mlme;

/**
 * tgt_dfs_set_current_channel() - Fill dfs channel structure from
 *                                 dfs_channel structure.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_ch_freq: Frequency in Mhz.
 * @dfs_ch_flags: Channel flags.
 * @dfs_ch_flagext: Extended channel flags.
 * @dfs_ch_ieee: IEEE channel number.
 * @dfs_ch_vhtop_ch_freq_seg1: Channel Center frequency1.
 * @dfs_ch_vhtop_ch_freq_seg2: Channel Center frequency2.
 */
#ifdef DFS_COMPONENT_ENABLE
#ifdef CONFIG_CHAN_NUM_API
QDF_STATUS tgt_dfs_set_current_channel(struct wlan_objmgr_pdev *pdev,
		uint16_t dfs_ch_freq,
		uint64_t dfs_ch_flags,
		uint16_t dfs_ch_flagext,
		uint8_t dfs_ch_ieee,
		uint8_t dfs_ch_vhtop_ch_freq_seg1,
		uint8_t dfs_ch_vhtop_ch_freq_seg2);
#endif

/**
 * tgt_dfs_set_current_channel_for_freq() - Fill dfs channel structure from
 *                                          dfs_channel structure.
 * @pdev: Pointer to DFS pdev object.
 * @dfs_ch_freq: Frequency in Mhz.
 * @dfs_ch_flags: Channel flags.
 * @dfs_ch_flagext: Extended channel flags.
 * @dfs_ch_ieee: IEEE channel number.
 * @dfs_ch_vhtop_ch_freq_seg1: Channel Center frequency1.
 * @dfs_ch_vhtop_ch_freq_seg2: Channel Center frequency2.
 * @dfs_ch_mhz_freq_seg1:  Channel center frequency of primary segment in MHZ.
 * @dfs_ch_mhz_freq_seg2:  Channel center frequency of secondary segment in MHZ
 *                         applicable only for 80+80MHZ mode of operation.
 * @is_channel_updated: Boolean to represent channel update.
 */
#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS
tgt_dfs_set_current_channel_for_freq(struct wlan_objmgr_pdev *pdev,
				     uint16_t dfs_ch_freq,
				     uint64_t dfs_ch_flags,
				     uint16_t dfs_ch_flagext,
				     uint8_t dfs_ch_ieee,
				     uint8_t dfs_ch_vhtop_ch_freq_seg1,
				     uint8_t dfs_ch_vhtop_ch_freq_seg2,
				     uint16_t dfs_ch_mhz_freq_seg1,
				     uint16_t dfs_ch_mhz_freq_seg2,
				     bool *is_channel_updated);
#endif

/**
 * tgt_dfs_radar_enable() - Enables the radar.
 * @pdev: Pointer to DFS pdev object.
 * @no_cac: If no_cac is 0, it cancels the CAC.
 * @enable: disable/enable radar
 *
 * This is called each time a channel change occurs, to (potentially) enable
 * the radar code.
 */
QDF_STATUS tgt_dfs_radar_enable(
	struct wlan_objmgr_pdev *pdev,
	int no_cac, uint32_t opmode, bool enable);

/**
 * tgt_dfs_control()- Used to process ioctls related to DFS.
 * @pdev: Pointer to DFS pdev object.
 * @id: Command type.
 * @indata: Input buffer.
 * @insize: size of the input buffer.
 * @outdata: A buffer for the results.
 * @outsize: Size of the output buffer.
 */
QDF_STATUS tgt_dfs_control(struct wlan_objmgr_pdev *pdev,
	u_int id,
	void *indata,
	uint32_t insize,
	void *outdata,
	uint32_t *outsize,
	int *error);

/**
 * tgt_dfs_get_radars() - Based on the chipset, calls init radar table functions
 * @pdev: Pointer to DFS pdev object.
 *
 * Wrapper function for dfs_get_radars(). This function called from
 * outside of DFS component.
 */
QDF_STATUS tgt_dfs_get_radars(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_dfs_process_radar_ind() - Process radar found indication.
 * @pdev: Pointer to DFS pdev object.
 * @radar_found: radar found info.
 *
 * Process radar found indication.
 *
 * Return QDF_STATUS.
 */
QDF_STATUS tgt_dfs_process_radar_ind(struct wlan_objmgr_pdev *pdev,
		struct radar_found_info *radar_found);
#else
static inline QDF_STATUS tgt_dfs_set_current_channel(
		struct wlan_objmgr_pdev *pdev,
		uint16_t dfs_ch_freq,
		uint64_t dfs_ch_flags,
		uint16_t dfs_ch_flagext,
		uint8_t dfs_ch_ieee,
		uint8_t dfs_ch_vhtop_ch_freq_seg1,
		uint8_t dfs_ch_vhtop_ch_freq_seg2)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS tgt_dfs_radar_enable(
	struct wlan_objmgr_pdev *pdev,
	int no_cac, uint32_t opmode, bool enable)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS tgt_dfs_control(struct wlan_objmgr_pdev *pdev,
	u_int id,
	void *indata,
	uint32_t insize,
	void *outdata,
	uint32_t *outsize,
	int *error)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS tgt_dfs_get_radars(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS tgt_dfs_process_radar_ind(
		struct wlan_objmgr_pdev *pdev,
		struct radar_found_info *radar_found)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * tgt_dfs_process_phyerr() - Process phyerr.
 * @pdev: Pointer to DFS pdev object.
 * @buf: Phyerr buffer.
 * @datalen: phyerr buffer length.
 * @r_rssi: RSSI.
 * @r_ext_rssi: Extension channel RSSI.
 * @r_rs_tstamp: Timestamp.
 * @r_fulltsf: TSF64.
 *
 * Wrapper function for dfs_process_phyerr(). This function called from
 * outside of DFS component.
 */
QDF_STATUS tgt_dfs_process_phyerr(struct wlan_objmgr_pdev *pdev,
	void *buf,
	uint16_t datalen,
	uint8_t r_rssi,
	uint8_t r_ext_rssi,
	uint32_t r_rs_tstamp,
	uint64_t r_fulltsf);

/**
 * tgt_dfs_process_phyerr_filter_offload() - Process radar event.
 * Wrapper function for dfs_process_phyerr_filter_offload(). This function
 * called from outside of DFS component.
 * @pdev: Pointer to DFS pdev object.
 * @wlan_radar_event: pointer to radar_event_info.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_dfs_process_phyerr_filter_offload(struct wlan_objmgr_pdev *pdev,
	struct radar_event_info *wlan_radar_event);

/**
 * tgt_dfs_is_phyerr_filter_offload() - Is phyerr filter offload.
 * @psoc: Pointer to psoc object.
 * @is_phyerr_filter_offload: Pointer to is_phyerr_filter_offload.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_dfs_is_phyerr_filter_offload(struct wlan_objmgr_psoc *psoc,
					    bool *is_phyerr_filter_offload);

/**
 * tgt_dfs_destroy_object() - Destroys the DFS object.
 * @pdev: Pointer to DFS pdev object.
 *
 * Wrapper function for  dfs_destroy_object(). This function called from
 * outside of DFS component.
 */
QDF_STATUS tgt_dfs_destroy_object(struct wlan_objmgr_pdev *pdev);

#ifdef QCA_MCL_DFS_SUPPORT
/**
 * tgt_dfs_set_tx_leakage_threshold() - set tx_leakage_threshold.
 * @pdev: Pointer to DFS pdev object.
 *
 * Return QDF_STATUS.
 */
QDF_STATUS tgt_dfs_set_tx_leakage_threshold(struct wlan_objmgr_pdev *pdev);
#else
static inline QDF_STATUS tgt_dfs_set_tx_leakage_threshold
		(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * tgt_dfs_is_precac_timer_running() - Check whether precac timer is running.
 * @pdev: Pointer to DFS pdev object.
 * @is_precac_timer_running: Pointer to save precac timer value.
 *
 * Wrapper function for dfs_is_precac_timer_running(). This function called from
 * outside of DFS component.
 */
QDF_STATUS tgt_dfs_is_precac_timer_running(struct wlan_objmgr_pdev *pdev,
	bool *is_precac_timer_running);

/**
 * tgt_dfs_set_agile_precac_state() - set state for Agile Precac.
 *
 * @pdev: Pointer to DFS pdev object.
 * @agile_precac_state: Agile Precac state
 *
 * wrapper function for  dfs_set_agile_precac_state.
 * This function called from outside of dfs component.
 */
QDF_STATUS tgt_dfs_set_agile_precac_state(struct wlan_objmgr_pdev *pdev,
					  int agile_precac_state);

/**
 * tgt_dfs_agile_precac_start() - Start agile precac
 *
 * @pdev: Pointer to DFS pdev object.
 *
 * wrapper function for  dfs_set_agile_precac_state.
 * This function called from outside of dfs component.
 */
QDF_STATUS tgt_dfs_agile_precac_start(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_dfs_ocac_complete() - Process off channel cac complete indication.
 * @pdev: Pointer to DFS pdev object.
 * @vdev_adfs_complete_status: Off channel CAC complete status.
 *
 * wrapper function for  dfs_set_agile_precac_state.
 * This function called from outside of dfs component.
 */
QDF_STATUS tgt_dfs_ocac_complete(struct wlan_objmgr_pdev *pdev,
				 struct vdev_adfs_complete_status *ocac_status);

/**
 * utils_dfs_find_vht80_chan_for_precac() - Find VHT80 channel for precac.
 * @pdev: Pointer to DFS pdev object.
 * @chan_mode: Channel mode.
 * @ch_freq_seg1: Segment1 channel freq.
 * @cfreq1: cfreq1.
 * @cfreq2: cfreq2.
 * @phy_mode: Precac phymode.
 * @dfs_set_cfreq2: Precac cfreq2
 * @set_agile: Agile mode flag.
 *
 * wrapper function for  dfs_find_vht80_chan_for_precacdfs_cancel_cac_timer().
 * This function called from outside of dfs component.
 */
#ifdef CONFIG_CHAN_NUM_API
QDF_STATUS tgt_dfs_find_vht80_chan_for_precac(struct wlan_objmgr_pdev *pdev,
		uint32_t chan_mode,
		uint8_t ch_freq_seg1,
		uint32_t *cfreq1,
		uint32_t *cfreq2,
		uint32_t *phy_mode,
		bool *dfs_set_cfreq2,
		bool *set_agile);
#endif

/**
 * tgt_dfs_find_vht80_precac_chan_freq() - Find VHT80 channel for precac
 * @pdev: Pointer to DFS pdev object.
 * @chan_mode: Channel mode.
 * @ch_freq_seg1_mhz: Segment1 channel freq in MHZ.
 * @cfreq1: cfreq1.
 * @cfreq2: cfreq2.
 * @phy_mode: Precac phymode.
 * @dfs_set_cfreq2: Precac cfreq2
 * @set_agile: Agile mode flag.
 *
 * wrapper function for  dfs_find_vht80_chan_for_precac_for_freq().
 */
#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS
tgt_dfs_find_vht80_precac_chan_freq(struct wlan_objmgr_pdev *pdev,
				    uint32_t chan_mode,
				    uint16_t ch_freq_mhz_seg1,
				    uint32_t *cfreq1,
				    uint32_t *cfreq2,
				    uint32_t *phy_mode,
				    bool *dfs_set_cfreq2,
				    bool *set_agile);
#endif

/**
 * tgt_dfs_cac_complete() - Process cac complete indication.
 * @pdev: Pointer to DFS pdev object.
 * @vdev_id: vdev id.
 *
 * Process cac complete indication from firmware.
 *
 * Return QDF_STATUS.
 */
QDF_STATUS tgt_dfs_cac_complete(struct wlan_objmgr_pdev *pdev,
		uint32_t vdev_id);

/**
 * tgt_dfs_reg_ev_handler() - Register dfs events.
 * @psoc: Pointer to psoc.
 *
 * Register dfs events.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS tgt_dfs_reg_ev_handler(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_dfs_stop() - Clear dfs timers.
 * @dfs: Pointer to wlan_dfs structure.
 */
QDF_STATUS tgt_dfs_stop(struct wlan_objmgr_pdev *pdev);

/**
* tgt_dfs_process_emulate_bang_radar_cmd() - Process to emulate dfs bangradar
*                                            command.
* @pdev: Pointer to DFS pdev object.
*
* Process  to emulate dfs bangradar command.
*
* Return: QDF_STATUS.
*/
QDF_STATUS tgt_dfs_process_emulate_bang_radar_cmd(struct wlan_objmgr_pdev *pdev,
		struct dfs_emulate_bang_radar_test_cmd *dfs_unit_test);

#ifdef QCA_MCL_DFS_SUPPORT
/**
 * tgt_dfs_set_phyerr_filter_offload() - config phyerr filter offload
 * @pdev: Pointer to DFS pdev object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_dfs_set_phyerr_filter_offload(struct wlan_objmgr_pdev *pdev);
#else
static inline QDF_STATUS tgt_dfs_set_phyerr_filter_offload
	(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
/**
 * tgt_dfs_send_avg_params_to_fw() - send average radar parameters to fw.
 * @pdev: Pointer to DFS pdev object.
 * @params: Pointer to dfs radar average parameters.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
tgt_dfs_send_avg_params_to_fw(struct wlan_objmgr_pdev *pdev,
			      struct dfs_radar_found_params *params);
#endif

/**
 * tgt_dfs_action_on_status_from_fw() - trigger the action to be taken based on
 * host dfs status received from fw.
 * @pdev: Pointer to pdev object.
 * @status: Pointer to the host dfs status received from fw.
 *
 * Return: QDF_STATUS
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS tgt_dfs_action_on_status_from_fw(struct wlan_objmgr_pdev *pdev,
					    uint32_t *status);
#else
static inline
QDF_STATUS tgt_dfs_action_on_status_from_fw(struct wlan_objmgr_pdev *pdev,
					    uint32_t *status)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * tgt_dfs_is_radar_enabled() - checks if radar detection is enabled.
 * @pdev: Pointer to pdev object.
 * @ignore_dfs: Pointer to check the value. If 1, radar detection is disabled.
 */
void tgt_dfs_is_radar_enabled(struct wlan_objmgr_pdev *pdev, int *ignore_dfs);

/**
 * tgt_dfs_reset_spoof_test() - reset the dfs spoof check variables
 * @pdev: Pointer to pdev object.
 *
 * Return: QDF_STATUS
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS tgt_dfs_reset_spoof_test(struct wlan_objmgr_pdev *pdev);
#else
static inline
QDF_STATUS tgt_dfs_reset_spoof_test(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * tgt_dfs_enable_stadfs() - Enable/Disable STADFS capability.
 * @pdev: Pointer to DFS pdev object.
 * @val: input value.
 */
void tgt_dfs_enable_stadfs(struct wlan_objmgr_pdev *pdev, bool val);

/**
 * tgt_dfs_is_stadfs_enabled() - Get STADFS capability
 * @pdev: Pointer to DFS pdev object.
 *
 * Return: true if STADFS is enabled, else false.
 */
bool tgt_dfs_is_stadfs_enabled(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_dfs_is_pdev_5ghz() - Check if the input pdev is 5GHZ.
 * @pdev: Pointer to DFS pdev object.
 *
 * Return: true if the pdev supports 5GHz, else false.
 */
bool tgt_dfs_is_pdev_5ghz(struct wlan_objmgr_pdev *pdev);

#if defined(WLAN_DFS_FULL_OFFLOAD) && defined(QCA_DFS_NOL_OFFLOAD)
/**
 * tgt_dfs_send_usenol_pdev_param() - Send usenol pdev param to FW.
 * @pdev: Pointer to pdev object.
 * @usenol: Value of usenol
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_dfs_send_usenol_pdev_param(struct wlan_objmgr_pdev *pdev,
					  bool usenol);

/**
 * tgt_dfs_send_subchan_marking() - Send subchannel marking pdev param to FW.
 * @pdev: Pointer to pdev object.
 * @subchanmark: Value of subchannel_marking.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_dfs_send_subchan_marking(struct wlan_objmgr_pdev *pdev,
					bool subchanmark);
#else
static inline
QDF_STATUS tgt_dfs_send_usenol_pdev_param(struct wlan_objmgr_pdev *pdev,
					  bool usenol)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
tgt_dfs_send_subchan_marking(struct wlan_objmgr_pdev *pdev, bool subchanmark)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#ifdef QCA_SUPPORT_AGILE_DFS
/**
 * tgt_dfs_set_fw_adfs_support() - Set FW aDFS support in dfs object.
 * @pdev: Pointer to pdev object.
 * @fw_adfs_support_160: aDFS enabled when pdev is on 160/80P80MHz.
 * @fw_adfs_support_non_160: aDFS enabled when pdev is on 20/40/80MHz.
 *
 * Return: void.
 */
void tgt_dfs_set_fw_adfs_support(struct wlan_objmgr_pdev *pdev,
				 bool fw_adfs_support_160,
				 bool fw_adfs_support_non_160);
#else
static inline
void tgt_dfs_set_fw_adfs_support(struct wlan_objmgr_pdev *pdev,
				 bool fw_adfs_support_160,
				 bool fw_adfs_support_non_160)
{
}
#endif

/**
 * tgt_dfs_init_tmp_psoc_nol() - Init temporary psoc NOL structure.
 * @pdev: Pointer to pdev object.
 * @num_radios: Number of radios in the psoc.
 *
 * Return: void.
 */
void tgt_dfs_init_tmp_psoc_nol(struct wlan_objmgr_pdev *pdev,
			       uint8_t num_radios);

/**
 * tgt_dfs_deinit_tmp_psoc_nol() - De-init temporary psoc NOL structure.
 * @pdev: Pointer to pdev object.
 *
 * Return: void.
 */
void tgt_dfs_deinit_tmp_psoc_nol(struct wlan_objmgr_pdev *pdev);

/**
 * tgt_dfs_save_dfs_nol_in_psoc() - Save NOL data of given pdev.
 * @pdev: Pointer to pdev object.
 * @pdev_id: The pdev ID which will have the NOL data.
 *
 * Based on the frequency of the NOL channel, copy it to the target pdev_id
 * structure in psoc.
 *
 * Return: void.
 */
void tgt_dfs_save_dfs_nol_in_psoc(struct wlan_objmgr_pdev *pdev,
				  uint8_t pdev_id);

/**
 * tgt_dfs_reinit_nol_from_psoc_copy() - Reinit saved NOL data to corresponding
 * pdevs.
 * @pdev: Pointer to pdev object.
 * @pdev_id: pdev_id of the given pdev.
 * @low_5ghz_freq: The low 5GHz frequency value of the target pdev id.
 * @high_5ghz_freq: The high 5GHz frequency value of the target pdev id.
 *
 * Return: void.
 */
void tgt_dfs_reinit_nol_from_psoc_copy(struct wlan_objmgr_pdev *pdev,
				       uint8_t pdev_id,
				       uint16_t low_5ghz_freq,
				       uint16_t high_5ghz_freq);

/**
 * tgt_dfs_reinit_precac_lists() - Reinit preCAC lists.
 * @src_pdev: Source pdev object from which the preCAC list is copied.
 * @dest_pdev: Destination pdev object to which the preCAC list is copied.
 * @low_5g_freq: Low 5G frequency value of the destination DFS.
 * @high_5g_freq: High 5G frequency value of the destination DFS.
 *
 * Copy all the preCAC list entries from the source pdev object to the
 * destination pdev object which fall within the frequency range of
 * low_5g_freq and high_5g_freq.
 *
 * Return: None (void).
 */
void tgt_dfs_reinit_precac_lists(struct wlan_objmgr_pdev *src_pdev,
				 struct wlan_objmgr_pdev *dest_pdev,
				 uint16_t low_5g_freq,
				 uint16_t high_5g_freq);

/**
 * tgt_dfs_complete_deferred_tasks() - Process HW mode switch completion and
 * handle deferred tasks.
 * @pdev: Pointer to primary pdev object.
 *
 * Return: void.
 */
void tgt_dfs_complete_deferred_tasks(struct wlan_objmgr_pdev *pdev);
#endif /* _WLAN_DFS_TGT_API_H_ */
