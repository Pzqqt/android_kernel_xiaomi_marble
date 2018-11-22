/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

/**
 * struct dfs_to_mlme - These are MLME function pointer used by DFS component.
 * @pdev_component_obj_attach:         Attach DFS object to PDEV.
 * @pdev_component_obj_detach:         Detach DFS object from PDEV.
 * @pdev_get_comp_private_obj:         Get DFS object from PDEV.
 * @dfs_start_rcsa:                    Send RCSA to RootAP.
 * @mlme_mark_dfs:                     Calls dfs_action function.
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
			uint8_t vhtop_ch_freq_seg2,
			uint64_t flags);
	QDF_STATUS (*mlme_start_csa)(struct wlan_objmgr_pdev *pdev,
			uint8_t ieee_chan, uint16_t freq,
			uint8_t cfreq2, uint64_t flags);
	QDF_STATUS (*mlme_proc_cac)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*mlme_deliver_event_up_after_cac)(
			struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*mlme_get_dfs_ch_nchans)(struct wlan_objmgr_pdev *pdev,
			int *nchans);
	QDF_STATUS (*mlme_get_extchan)(struct wlan_objmgr_pdev *pdev,
			uint16_t *dfs_ch_freq,
			uint64_t *dfs_ch_flags,
			uint16_t *dfs_ch_flagext,
			uint8_t *dfs_ch_ieee,
			uint8_t *dfs_ch_vhtop_ch_freq_seg1,
			uint8_t *dfs_ch_vhtop_ch_freq_seg2);
	QDF_STATUS (*mlme_set_no_chans_available)(struct wlan_objmgr_pdev *pdev,
			int val);
	QDF_STATUS (*mlme_ieee2mhz)(struct wlan_objmgr_pdev *pdev,
			int ieee,
			uint64_t flag,
			int *freq);
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

	QDF_STATUS (*mlme_get_dfs_ch_channels)(struct wlan_objmgr_pdev *pdev,
			uint16_t *dfs_ch_freq,
			uint64_t *dfs_ch_flags,
			uint16_t *dfs_ch_flagext,
			uint8_t *dfs_ch_ieee,
			uint8_t *dfs_ch_vhtop_ch_freq_seg1,
			uint8_t *dfs_ch_vhtop_ch_freq_seg2,
			int index);
	QDF_STATUS (*mlme_dfs_ch_flags_ext)(struct wlan_objmgr_pdev *pdev,
			uint16_t *flag_ext);
	QDF_STATUS (*mlme_channel_change_by_precac)(
			struct wlan_objmgr_pdev *pdev);
#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
	QDF_STATUS (*mlme_precac_chan_change_csa)(struct wlan_objmgr_pdev *pdev,
						  uint8_t des_chan,
						  enum wlan_phymode des_mode);
#endif
	QDF_STATUS (*mlme_nol_timeout_notification)(
			struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*mlme_clist_update)(struct wlan_objmgr_pdev *pdev,
			void *nollist,
			int nentries);
	bool (*mlme_is_opmode_sta)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*mlme_get_cac_timeout)(struct wlan_objmgr_pdev *pdev,
			uint16_t dfs_ch_freq,
			uint8_t c_vhtop_ch_freq_seg2,
			uint64_t dfs_ch_flags,
			int *cac_timeout);
	QDF_STATUS (*mlme_rebuild_chan_list_with_non_dfs_channels)
			(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*mlme_restart_vaps_with_non_dfs_chan)
			(struct wlan_objmgr_pdev *pdev, int no_chans_avail);
	bool (*mlme_check_allowed_prim_chanlist)
			(struct wlan_objmgr_pdev *pdev, uint32_t chan_num);
	QDF_STATUS (*mlme_update_scan_channel_list)
			(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*mlme_bringdown_vaps)
			(struct wlan_objmgr_pdev *pdev);
	void (*mlme_dfs_deliver_event)
			(struct wlan_objmgr_pdev *pdev,
			 uint16_t freq,
			 enum WLAN_DFS_EVENTS event);
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
 * @value: input value for dfs_precac_enable flag.
 *
 * Wrapper function for dfs_set_precac_enable().
 * This function called from outside of dfs component.
 */
QDF_STATUS ucfg_dfs_set_precac_enable(struct wlan_objmgr_pdev *pdev,
				      uint32_t value);

/**
 * ucfg_dfs_get_precac_enable() - Get precac enable flag.
 * @pdev: Pointer to DFS pdev object.
 * @buff: Pointer to save precac_enable value.
 *
 * Wrapper function for dfs_get_precac_enable().
 * This function called from outside of dfs component.
 */
QDF_STATUS ucfg_dfs_get_precac_enable(struct wlan_objmgr_pdev *pdev, int *buff);

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
enum precac_chan_state
ucfg_dfs_get_precac_chan_state(struct wlan_objmgr_pdev *pdev,
			       uint8_t precac_chan);
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
#endif /* _WLAN_DFS_UCFG_API_H_ */
