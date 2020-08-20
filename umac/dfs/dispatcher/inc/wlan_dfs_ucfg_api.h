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

#ifndef _WLAN_DFS_UCFG_API_H_
#define _WLAN_DFS_UCFG_API_H_

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_dfs_ioctl.h>
#include <wlan_dfs_public_struct.h>

/**
 * struct dfs_to_mlme - These are MLME function pointer used by DFS component.
 * @pdev_component_obj_attach:         Attach DFS object to PDEV.
 * @pdev_component_obj_detach:         Detach DFS object from PDEV.
 * @pdev_get_comp_private_obj:         Get DFS object from PDEV.
 * @dfs_start_rcsa:                    Send RCSA to RootAP.
 * @mlme_start_csa:                    Sends CSA.
 * @mlme_proc_cac:                     Process the CAC completion event.
 * @mlme_deliver_event_up_after_cac:   Send a CAC timeout, VAP up event to user
 *                                     space
 * @mlme_get_dfs_ch_nchans:            Get number of channels in the channel
 *                                     list.
 * @mlme_get_extchan:                  Gets the extension channel.
 * @mlme_set_no_chans_available:       Sets no_chans_available flag.
 * @mlme_ieee2mhz:                     Gets Channel freq from ieee number.
 * @mlme_find_dot11_channel:           Find dot11 channel.
 * @mlme_get_dfs_ch_channels:          Get the channel list.
 * @mlme_dfs_ch_flags_ext:             Gets channel extension flag.
 * @mlme_channel_change_by_precac:     Channel change triggered by PreCAC.
 * @mlme_precac_chan_change_csa:       Channel change triggered by PrCAC using
 *                                     Channel Switch Announcement.
 * @mlme_nol_timeout_notification:     NOL timeout notification.
 * @mlme_clist_update:                 Updates the channel list.
 * @mlme_is_opmode_sta:                Check if pdev opmode is STA.
 * @mlme_get_cac_timeout:              Gets the CAC timeout.
 * @mlme_rebuild_chan_list_with_non_dfs_channel: Rebuild channels with non-dfs
 *                                     channels.
 * @mlme_restart_vaps_with_non_dfs_chan: Restart vaps with non-dfs channel.
 * @mlme_check_allowed_prim_chanlist:  Check whether the given channel is
 *                                     present in the primary allowed channel
 *                                     list or not.
 * @mlme_update_scan_channel_list:     Update the scan channel list sent to FW.
 * @mlme_bringdown_vaps:               Bringdown vaps if no chans is present.
 * @mlme_dfs_deliver_event:            Deliver DFS events to user space
 * @mlme_precac_chan_change_csa_for_freq:Channel change triggered by PrCAC using
 *                                     Channel Switch Announcement.
 * @mlme_postnol_chan_switch:          Channel change post NOL using Channel
 *                                     Switch Announcement.
 * @mlme_mark_dfs:                     Mark DFS channel frequency as radar.
 * @mlme_get_extchan_for_freq:         Get the extension channel.
 * @mlme_find_dot11_chan_for_freq:     Find a channel pointer.
 * @mlme_get_dfs_channels_for_freq:    Get DFS channels from current channel
 *                                     list.
 * @mlme_get_cac_timeout_for_freq:     Get CAC timeout for a given channel
 *                                     frequency.
 * @mlme_acquire_radar_mode_switch_lock: Acquire lock for radar processing over
 *                                     mode switch.
 * @mlme_release_radar_mode_switch_lock: Release lock taken for radar processing
 *                                     over mode switch.
 */
struct dfs_to_mlme {
	QDF_STATUS (*pdev_component_obj_attach)(struct wlan_objmgr_pdev *pdev,
			enum wlan_umac_comp_id id,
			void *comp_priv_obj,
			QDF_STATUS status);
	QDF_STATUS (*pdev_component_obj_detach)(struct wlan_objmgr_pdev *pdev,
			enum wlan_umac_comp_id id,
			void *comp_priv_obj);
	QDF_STATUS (*dfs_start_rcsa)(struct wlan_objmgr_pdev *pdev,
			bool *wait_for_csa);
	QDF_STATUS (*mlme_mark_dfs)(struct wlan_objmgr_pdev *pdev,
			uint8_t ieee,
			uint16_t freq,
			uint16_t vhtop_ch_freq_seg2,
			uint64_t flags);
#ifdef CONFIG_CHAN_NUM_API
	QDF_STATUS (*mlme_start_csa)(struct wlan_objmgr_pdev *pdev,
			uint8_t ieee_chan, uint16_t freq,
			uint8_t cfreq2, uint64_t flags);
#endif
#ifdef CONFIG_CHAN_FREQ_API
	QDF_STATUS (*mlme_start_csa_for_freq)(struct wlan_objmgr_pdev *pdev,
					      uint8_t ieee_chan, uint16_t freq,
					      uint16_t cfreq2, uint64_t flags);
#endif

	QDF_STATUS (*mlme_proc_cac)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*mlme_deliver_event_up_after_cac)(
			struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*mlme_get_dfs_ch_nchans)(struct wlan_objmgr_pdev *pdev,
			int *nchans);
#ifdef CONFIG_CHAN_NUM_API
	QDF_STATUS (*mlme_get_extchan)(struct wlan_objmgr_pdev *pdev,
			uint16_t *dfs_ch_freq,
			uint64_t *dfs_ch_flags,
			uint16_t *dfs_ch_flagext,
			uint8_t *dfs_ch_ieee,
			uint8_t *dfs_ch_vhtop_ch_freq_seg1,
			uint8_t *dfs_ch_vhtop_ch_freq_seg2);
#endif
#ifdef CONFIG_CHAN_FREQ_API
	QDF_STATUS (*mlme_get_extchan_for_freq)(struct wlan_objmgr_pdev *pdev,
						uint16_t *dfs_ch_freq,
						uint64_t *dfs_ch_flags,
						uint16_t *dfs_ch_flagext,
						uint8_t *dfs_ch_ieee,
						uint8_t *dfs_vhtop_ch_freq_seg1,
						uint8_t *dfs_vhtop_ch_freq_seg2,
						uint16_t *dfs_ch_mhz_freq_seg1,
						uint16_t *dfs_ch_mhz_freq_seg2);
#endif
	QDF_STATUS (*mlme_set_no_chans_available)(struct wlan_objmgr_pdev *pdev,
			int val);
	QDF_STATUS (*mlme_ieee2mhz)(struct wlan_objmgr_pdev *pdev,
			int ieee,
			uint64_t flag,
			int *freq);
#ifdef CONFIG_CHAN_NUM_API
	QDF_STATUS (*mlme_find_dot11_channel)(struct wlan_objmgr_pdev *pdev,
			uint8_t ieee,
			uint8_t des_cfreq2,
			int mode,
			uint16_t *dfs_ch_freq,
			uint64_t *dfs_ch_flags,
			uint16_t *dfs_ch_flagext,
			uint8_t *dfs_ch_ieee,
			uint8_t *dfs_ch_vhtop_ch_freq_seg1,
			uint8_t *dfs_ch_vhtop_ch_freq_seg2);
#endif
#ifdef CONFIG_CHAN_FREQ_API
	QDF_STATUS (*mlme_find_dot11_chan_for_freq)(struct wlan_objmgr_pdev *,
						    uint16_t freq,
						    uint16_t des_cfreq2_mhz,
						    int mode,
						    uint16_t *dfs_ch_freq,
						    uint64_t *dfs_ch_flags,
						    uint16_t *dfs_ch_flagext,
						    uint8_t *dfs_ch_ieee,
						    uint8_t *dfs_ch_freq_seg1,
						    uint8_t *dfs_ch_freq_seg2,
						    uint16_t *dfs_cfreq1_mhz,
						    uint16_t *dfs_cfreq2_mhz);
#endif
#ifdef CONFIG_CHAN_NUM_API
	QDF_STATUS (*mlme_get_dfs_ch_channels)(struct wlan_objmgr_pdev *pdev,
			uint16_t *dfs_ch_freq,
			uint64_t *dfs_ch_flags,
			uint16_t *dfs_ch_flagext,
			uint8_t *dfs_ch_ieee,
			uint8_t *dfs_ch_vhtop_ch_freq_seg1,
			uint8_t *dfs_ch_vhtop_ch_freq_seg2,
			int index);
#endif
#ifdef CONFIG_CHAN_FREQ_API
	QDF_STATUS (*mlme_get_dfs_channels_for_freq)(
			struct wlan_objmgr_pdev *pdev,
			uint16_t *dfs_chan_freq,
			uint64_t *dfs_chan_flags,
			uint16_t *dfs_chan_flagext,
			uint8_t *dfs_chan_ieee,
			uint8_t *dfs_chan_vhtop_ch_freq_seg1,
			uint8_t *dfs_chan_vhtop_ch_freq_seg2,
			uint16_t *dfs_chan_mhz_freq_seg1,
			uint16_t *dfs_chan_mhz_freq_seg2,
			int index);
#endif
	QDF_STATUS (*mlme_dfs_ch_flags_ext)(struct wlan_objmgr_pdev *pdev,
			uint16_t *flag_ext);
	QDF_STATUS (*mlme_channel_change_by_precac)(
			struct wlan_objmgr_pdev *pdev);
#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
#ifdef CONFIG_CHAN_FREQ_API
	QDF_STATUS
	    (*mlme_precac_chan_change_csa_for_freq)(struct wlan_objmgr_pdev *,
						    uint16_t des_chan_freq,
						    enum wlan_phymode des_mode);
#endif
#ifdef CONFIG_CHAN_NUM_API
	QDF_STATUS
		(*mlme_precac_chan_change_csa)(struct wlan_objmgr_pdev *,
					       uint8_t des_chan,
					       enum wlan_phymode des_mode);
#endif
#endif
#ifdef QCA_SUPPORT_DFS_CHAN_POSTNOL
	QDF_STATUS
	(*mlme_postnol_chan_switch)(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t des_chan_freq,
				    qdf_freq_t des_cfreq2,
				    enum wlan_phymode des_mode);
#endif
	QDF_STATUS (*mlme_nol_timeout_notification)(
			struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*mlme_clist_update)(struct wlan_objmgr_pdev *pdev,
			void *nollist,
			int nentries);
	bool (*mlme_is_opmode_sta)(struct wlan_objmgr_pdev *pdev);
#ifdef CONFIG_CHAN_NUM_API
	QDF_STATUS (*mlme_get_cac_timeout)(struct wlan_objmgr_pdev *pdev,
			uint16_t dfs_ch_freq,
			uint8_t c_vhtop_ch_freq_seg2,
			uint64_t dfs_ch_flags,
			int *cac_timeout);
#endif
#ifdef CONFIG_CHAN_FREQ_API
	QDF_STATUS
	    (*mlme_get_cac_timeout_for_freq)(struct wlan_objmgr_pdev *pdev,
					     uint16_t dfs_ch_freq,
					     uint16_t c_vhtop_ch_freq_seg2,
					     uint64_t dfs_ch_flags,
					     int *cac_timeout);
#endif
	QDF_STATUS (*mlme_rebuild_chan_list_with_non_dfs_channels)
			(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*mlme_restart_vaps_with_non_dfs_chan)
			(struct wlan_objmgr_pdev *pdev, int no_chans_avail);
	bool (*mlme_check_allowed_prim_chanlist)
			(struct wlan_objmgr_pdev *pdev, uint32_t chan);
	QDF_STATUS (*mlme_update_scan_channel_list)
			(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*mlme_bringdown_vaps)
			(struct wlan_objmgr_pdev *pdev);
	void (*mlme_dfs_deliver_event)
			(struct wlan_objmgr_pdev *pdev,
			 uint16_t freq,
			 enum WLAN_DFS_EVENTS event);
	bool (*mlme_is_inter_band_chan_switch_allowed)
			(struct wlan_objmgr_pdev *pdev);
	void (*mlme_acquire_radar_mode_switch_lock)
			(struct wlan_objmgr_pdev *pdev);
	void (*mlme_release_radar_mode_switch_lock)
			(struct wlan_objmgr_pdev *pdev);
};

extern struct dfs_to_mlme global_dfs_to_mlme;

/**
 * wlan_dfs_pdev_obj_create_notification() - DFS pdev object create handler.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS wlan_dfs_pdev_obj_create_notification(struct wlan_objmgr_pdev *pdev,
		void *arg);

/**
 * wlan_dfs_pdev_obj_destroy_notification() - DFS pdev object delete handler.
 * @pdev: Pointer to DFS pdev object.
 */
QDF_STATUS wlan_dfs_pdev_obj_destroy_notification(struct wlan_objmgr_pdev *pdev,
		void *arg);

/**
 * ucfg_dfs_is_ap_cac_timer_running() - Returns the dfs cac timer.
 * @pdev: Pointer to DFS pdev object.
 * @is_ap_cac_timer_running: Pointer to save dfs_cac_timer_running value.
 *
 * Wrapper function for dfs_is_ap_cac_timer_running().
 * This function called from outside of dfs component.
 */
QDF_STATUS ucfg_dfs_is_ap_cac_timer_running(struct wlan_objmgr_pdev *pdev,
		int *is_ap_cac_timer_running);

/**
 * ucfg_dfs_getnol() - Wrapper function for dfs_get_nol()
 * @pdev: Pointer to DFS pdev object.
 * @dfs_nolinfo: Pointer to dfsreq_nolinfo structure.
 *
 * Wrapper function for dfs_getnol().
 * This function called from outside of dfs component.
 */
QDF_STATUS ucfg_dfs_getnol(struct wlan_objmgr_pdev *pdev, void *dfs_nolinfo);

/**
 * ucfg_dfs_override_cac_timeout() -  Override the default CAC timeout.
 * @pdev: Pointer to DFS pdev object.
 * @cac_timeout: CAC timeout value.
 *
 * Wrapper function for dfs_override_cac_timeout().
 * This function called from outside of dfs component.
 */
QDF_STATUS ucfg_dfs_override_cac_timeout(struct wlan_objmgr_pdev *pdev,
		int cac_timeout, int *status);

/**
 * ucfg_dfs_get_override_cac_timeout() -  Get override CAC timeout value.
 * @pdev: Pointer to DFS pdev object.
 * @cac_timeout: Pointer to save the CAC timeout value.
 *
 * Wrapper function for dfs_get_override_cac_timeout().
 * This function called from outside of dfs component.
 */
QDF_STATUS ucfg_dfs_get_override_cac_timeout(struct wlan_objmgr_pdev *pdev,
		int *cac_timeout, int *status);

/**
 * ucfg_dfs_get_override_precac_timeout() - Get precac timeout.
 * @pdev: Pointer to DFS pdev object.
 * @precac_timeout: Get precac timeout value in this variable.
 *
 * Wrapper function for dfs_get_override_precac_timeout().
 * This function called from outside of dfs component.
 */
QDF_STATUS ucfg_dfs_get_override_precac_timeout(struct wlan_objmgr_pdev *pdev,
		int *precac_timeout);

/**
 * ucfg_dfs_override_precac_timeout() - Override the default precac timeout.
 * @pdev: Pointer to DFS pdev object.
 * @precac_timeout: Precac timeout value.
 *
 * Wrapper function for dfs_override_precac_timeout().
 * This function called from outside of dfs component.
 */
QDF_STATUS ucfg_dfs_override_precac_timeout(struct wlan_objmgr_pdev *pdev,
		int precac_timeout);

/**
 * ucfg_dfs_set_precac_enable() - Set precac enable flag.
 * @pdev: Pointer to DFS pdev object.
 * @value: input value for dfs_legacy_precac_ucfg flag.
 *
 * Wrapper function for dfs_set_precac_enable().
 * This function called from outside of dfs component.
 */
QDF_STATUS ucfg_dfs_set_precac_enable(struct wlan_objmgr_pdev *pdev,
				      uint32_t value);

/**
 * ucfg_dfs_get_legacy_precac_enable() - Get the legacy precac enable flag.
 * @pdev: Pointer to DFS pdev object.
 * @buff: Pointer to save precac_enable value.
 *
 * Wrapper function for dfs_is_legacy_precac_enabled() and returns the
 * legacy precac enable flag for partial offload chipsets.
 * This function called from outside of dfs component.
 */
QDF_STATUS ucfg_dfs_get_legacy_precac_enable(struct wlan_objmgr_pdev *pdev,
					     bool *buff);

/**
 * ucfg_dfs_get_agile_precac_enable() - Get agile precac enable flag.
 * @pdev: Pointer to DFS pdev object.
 * @buff: Pointer to save dfs_agile_precac_ucfg value.
 *
 * Wrapper function for dfs_is_legacy_precac_enabled().
 * This function called from outside of dfs component.
 *
 * Return:
 * * QDF_STATUS_SUCCESS: Successfully able to get agile precac flag.
 * * QDF_STATUS_E_FAILURE: Failed to get agile precac flag.
 */
QDF_STATUS ucfg_dfs_get_agile_precac_enable(struct wlan_objmgr_pdev *pdev,
					    bool *buff);

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
/**
 * ucfg_dfs_set_precac_intermediate_chan() - Set intermediate channel
 *                                           for preCAC.
 * @pdev: Pointer to DFS pdev object.
 * @value: Channel number of intermediate channel
 *
 * Wrapper function for dfs_set_precac_intermediate_chan().
 * This function is called from outside of dfs component.
 *
 * Return:
 * * QDF_STATUS_SUCCESS  : Successfully set intermediate channel.
 * * QDF_STATUS_E_FAILURE: Failed to set intermediate channel.
 */
QDF_STATUS ucfg_dfs_set_precac_intermediate_chan(struct wlan_objmgr_pdev *pdev,
						 uint32_t value);

/**
 * ucfg_dfs_get_precac_intermediate_chan() - Get intermediate channel
 *						for preCAC.
 * @pdev: Pointer to DFS pdev object.
 * @buff: Pointer to Channel number of intermediate channel.
 *
 * Wrapper function for dfs_get_precac_intermediate_chan().
 * This function is called from outside of dfs component.
 *
 * Return: Configured intermediate precac channel.
 */
QDF_STATUS ucfg_dfs_get_precac_intermediate_chan(struct wlan_objmgr_pdev *pdev,
						 int *buff);

/**
 * ucfg_dfs_get_precac_chan_state() - Get precac status for the given channel.
 * @pdev: Pointer to DFS pdev object.
 * @precac_chan: Channel number for which precac state needs to be determined.
 *
 * Wrapper function for dfs_get_precac_chan_state().
 * This function called from outside of dfs component.
 *
 * Return: Precac state of the given channel.
 */
#ifdef CONFIG_CHAN_NUM_API
enum precac_chan_state
ucfg_dfs_get_precac_chan_state(struct wlan_objmgr_pdev *pdev,
			       uint8_t precac_chan);
#endif

/**
 * ucfg_dfs_get_precac_chan_state_for_freq() - Get precac status for the
 * given channel.
 * @pdev: Pointer to DFS pdev object.
 * @precac_chan: Channel frequency for which precac state needs to be
 *               determined.
 *
 * Wrapper function for dfs_get_precac_chan_state().
 * This function called from outside of dfs component.
 *
 * Return: Precac state of the given channel.
 */
#ifdef CONFIG_CHAN_FREQ_API
enum precac_chan_state
ucfg_dfs_get_precac_chan_state_for_freq(struct wlan_objmgr_pdev *pdev,
					uint16_t precac_freq);
#endif

#endif

#ifdef QCA_MCL_DFS_SUPPORT
/**
 * ucfg_dfs_update_config() - Update DFS user config.
 * @psoc: Pointer to psoc.
 * @req: DFS user config.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_dfs_update_config(struct wlan_objmgr_psoc *psoc,
		struct dfs_user_config *req);
#else
static inline QDF_STATUS ucfg_dfs_update_config(struct wlan_objmgr_psoc *psoc,
		struct dfs_user_config *req)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * ucfg_dfs_set_override_status_timeout() - override the value of host dfs
 * status wait timeout.
 * @pdev: Pointer to DFS pdev object.
 * @status_timeout: timeout value.
 *
 * Wrapper function for dfs_set_override_status_timeout().
 * This function called from outside of dfs component.
 *
 * Return: QDF_STATUS
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS ucfg_dfs_set_override_status_timeout(struct wlan_objmgr_pdev *pdev,
					    int status_timeout);
#else
static inline
QDF_STATUS ucfg_dfs_set_override_status_timeout(struct wlan_objmgr_pdev *pdev,
					    int status_timeout)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(WLAN_DFS_SYNTHETIC_RADAR)
/**
 * ucfg_dfs_allow_hw_pulses() - Set or unset dfs-allow_hw_pulses
 * which isolates synthetic radar pulse detection from actual radar detection.
 * @pdev: Pointer to DFS pdev object.
 * @allow_hw_pulses: Allow synthetic pulse detection true/false.
 *
 * Wrapper function for dfs_set_allow_hw_pulses().
 * This function called from outside of dfs component.
 *
 * Return: void
 */
void ucfg_dfs_allow_hw_pulses(struct wlan_objmgr_pdev *pdev,
			      bool allow_hw_pulses);

/**
 * ucfg_dfs_is_hw_pulses_allowed() - Check if actual radar detection is allowed
 * or synthetic pulse detection is enabled.
 * @pdev: Pointer to DFS pdev object.
 *
 * Wrapper function for dfs_is_hw_pulses_allowed().
 * This function called from outside of dfs component.
 *
 * Return: bool
 */
bool ucfg_dfs_is_hw_pulses_allowed(struct wlan_objmgr_pdev *pdev);
#else
static inline
void ucfg_dfs_allow_hw_pulses(struct wlan_objmgr_pdev *pdev,
			      bool allow_hw_pulses)
{
}

static inline
bool ucfg_dfs_is_hw_pulses_allowed(struct wlan_objmgr_pdev *pdev)
{
	return true;
}
#endif

/**
 * ucfg_dfs_get_override_status_timeout() - Get the value of host dfs status
 * wait timeout.
 * @pdev: Pointer to DFS pdev object.
 * @status_timeout: Pointer to save the timeout value.
 *
 * Wrapper function for dfs_get_override_status_timeout().
 * This function called from outside of dfs component.
 *
 * Return: QDF_STATUS
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
QDF_STATUS ucfg_dfs_get_override_status_timeout(struct wlan_objmgr_pdev *pdev,
						int *status_timeout);
#else
static inline
QDF_STATUS ucfg_dfs_get_override_status_timeout(struct wlan_objmgr_pdev *pdev,
						int *status_timeout)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * ucfg_dfs_set_nol_subchannel_marking() - Set or unset NOL subchannel marking.
 * @pdev: Pointer to DFS pdev object.
 * @nol_subchannel_marking: Set NOL subchannel marking based on this value.
 *
 * Wrapper function for dfs_set_nol_subchannel_marking().
 * This function is called from outside of dfs component.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_dfs_set_nol_subchannel_marking(struct wlan_objmgr_pdev *pdev,
					       bool nol_subchannel_marking);

/**
 * ucfg_dfs_get_nol_subchannel_marking() - Get the value of NOL subchannel
 * marking.
 * @pdev: Pointer to DFS pdev object.
 * @nol_subchannel_marking: Store the value of  NOL subchannel marking.
 *
 * Wrapper function for dfs_get_nol_subchannel_marking().
 * This function is called from outside of dfs component.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_dfs_get_nol_subchannel_marking(struct wlan_objmgr_pdev *pdev,
					       bool *nol_subchannel_marking);
/**
 * ucfg_dfs_reinit_timers() - Init DFS timers.
 * @pdev: Pointer to wlan_objmgr_pdev structure.
 *
 * Wrapper function to reset CAC, NOL, DFS Test Timer and ZeroCAC Timer.
 * This is invoked per pdev to reinitialize timers after HW Mode Switch is
 * triggered.
 */
QDF_STATUS ucfg_dfs_reinit_timers(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_dfs_reset_agile_config() - Reset ADFS config.
 * @pdev: Pointer to wlan_objmgr_pdev structure.
 *
 * Wrapper function to reset Agile DFS config such as the variables which hold
 * information about the state of the preCAC timer, active precac
 * dfs index and OCAC status. It is invoked before HW Mode switch is triggered
 * to ensure ADFS config is in a well known consistent state.
 */
#ifdef QCA_SUPPORT_AGILE_DFS
QDF_STATUS ucfg_dfs_reset_agile_config(struct wlan_objmgr_psoc *psoc);
#else
static inline QDF_STATUS ucfg_dfs_reset_agile_config(struct wlan_objmgr_psoc
						    *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * ucfg_dfs_set_rcac_enable() - Set rcac enable flag.
 * @pdev: Pointer to DFS pdev object.
 * @rcac_en: User input value to enable/disable rolling cac feature.
 *
 */
#ifdef QCA_SUPPORT_ADFS_RCAC
QDF_STATUS ucfg_dfs_set_rcac_enable(struct wlan_objmgr_pdev *pdev,
				    bool rcac_en);
#else
static inline QDF_STATUS
ucfg_dfs_set_rcac_enable(struct wlan_objmgr_pdev *pdev,
			 bool rcac_en)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * ucfg_dfs_get_rcac_enable() - Get rcac enable flag.
 * @pdev: Pointer to DFS pdev object.
 * @rcac_en: Pointer to hold the "rcac" config.
 *
 */
#ifdef QCA_SUPPORT_ADFS_RCAC
QDF_STATUS ucfg_dfs_get_rcac_enable(struct wlan_objmgr_pdev *pdev,
				    bool *rcac_en);
#else
static inline QDF_STATUS
ucfg_dfs_get_rcac_enable(struct wlan_objmgr_pdev *pdev,
			 bool *rcac_en)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * ucfg_dfs_set_rcac_freq() - Set rcac freq.
 * @pdev: Pointer to DFS pdev object.
 * @rcac_freq: User configured rcac freq in MHZ.
 *
 */
#ifdef QCA_SUPPORT_ADFS_RCAC
QDF_STATUS ucfg_dfs_set_rcac_freq(struct wlan_objmgr_pdev *pdev,
				  qdf_freq_t rcac_freq);
#else
static inline QDF_STATUS
ucfg_dfs_set_rcac_freq(struct wlan_objmgr_pdev *pdev,
		       qdf_freq_t rcac_freq)
{
	return QDF_STATUS_SUCCESS;
}
#endif

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

/**
 * ucfg_dfs_get_rcac_freq() - Get rcac freq.
 * @pdev: Pointer to DFS pdev object.
 * @rcac_freq: Pointer to store the user configured rcac freq in MHZ.
 *
 */
#ifdef QCA_SUPPORT_ADFS_RCAC
QDF_STATUS ucfg_dfs_get_rcac_freq(struct wlan_objmgr_pdev *pdev,
				  qdf_freq_t *rcac_freq);
#else
static inline QDF_STATUS
ucfg_dfs_get_rcac_freq(struct wlan_objmgr_pdev *pdev,
		       qdf_freq_t *rcac_freq)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * ucfg_dfs_is_agile_rcac_enabled() - Determine if Rolling CAC is supported
 * or not.
 * @pdev: Pointer to struct wlan_objmgr_pdev.
 *
 * Following are the conditions needed to assertain that rolling CAC
 * is enabled:
 * 1. DFS domain of the PDEV must be FCC or MKK.
 * 2. User has enabled Rolling CAC configuration.
 * 3. FW capability to support ADFS.
 *
 * Return: True if RCAC support is enabled, false otherwise.
 */
#ifdef QCA_SUPPORT_ADFS_RCAC
bool ucfg_dfs_is_agile_rcac_enabled(struct wlan_objmgr_pdev *pdev);
#else
static inline bool
ucfg_dfs_is_agile_rcac_enabled(struct wlan_objmgr_pdev *pdev)
{
	return false;
}
#endif

#endif /* _WLAN_DFS_UCFG_API_H_ */
