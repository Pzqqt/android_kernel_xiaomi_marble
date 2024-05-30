/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC : contains interface prototypes for son api
 */
#ifndef _SON_API_H_
#define _SON_API_H_

#include <qdf_types.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <reg_services_public_struct.h>

/**
 * mlme_deliver_cb - cb to deliver mlme event
 * @vdev: pointer to vdev
 * @event_len: event length
 * @event_buf: event buffer
 *
 * @Return: 0 if event is sent successfully
 */
typedef int (*mlme_deliver_cb)(struct wlan_objmgr_vdev *vdev,
			       uint32_t event_len,
			       const uint8_t *event_buf);

/**
 * enum SON_MLME_DELIVER_CB_TYPE - mlme deliver cb type
 * @SON_MLME_DELIVER_CB_TYPE_OPMODE: cb to deliver opmode
 * @SON_MLME_DELIVER_CB_TYPE_SMPS: cb to deliver smps
 */
enum SON_MLME_DELIVER_CB_TYPE {
	SON_MLME_DELIVER_CB_TYPE_OPMODE,
	SON_MLME_DELIVER_CB_TYPE_SMPS,
};

/**
 * wlan_son_register_mlme_deliver_cb - register mlme deliver cb
 * @psoc: pointer to psoc
 * @cb: mlme deliver cb
 * @type: mlme deliver cb type
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_son_register_mlme_deliver_cb(struct wlan_objmgr_psoc *psoc,
				  mlme_deliver_cb cb,
				  enum SON_MLME_DELIVER_CB_TYPE type);

/**
 * wlan_son_get_chan_flag() - get chan flag
 * @pdev: pointer to pdev
 * @freq: qdf_freq_t
 * @flag_160: If true, 160 channel info will be obtained;
 *            otherwise 80+80, 80 channel info will be obtained
 * @chan_params: chan parameters
 *
 * Return: combination of enum qca_wlan_vendor_channel_prop_flags and
 *         enum qca_wlan_vendor_channel_prop_flags_2
 */
uint32_t wlan_son_get_chan_flag(struct wlan_objmgr_pdev *pdev,
				qdf_freq_t freq, bool flag_160,
				struct ch_params *chan_params);

/**
 * wlan_son_peer_set_kickout_allow() - set the peer is allowed to kickout
 * @vdev: pointer to vdev
 * @peer: pointer to peer
 * @kickout_allow: kickout_allow to set
 *
 * Return: QDF_STATUS_SUCCESS on Success else failure.
 */
QDF_STATUS wlan_son_peer_set_kickout_allow(struct wlan_objmgr_vdev *vdev,
					   struct wlan_objmgr_peer *peer,
					   bool kickout_allow);

#ifdef WLAN_FEATURE_SON
/**
 * wlan_son_peer_is_kickout_allow() - Is peer is allowed to kickout
 * @vdev: pointer to vdev
 * @macaddr: mac addr of the peer
 *
 * Return: True if it is allowed to kickout.
 */
bool wlan_son_peer_is_kickout_allow(struct wlan_objmgr_vdev *vdev,
				    uint8_t *macaddr);

/**
 * wlan_son_ind_assoc_req_frm() - indicate assoc req frame to son
 * @vdev: pointer to vdev
 * @is_reassoc: true if it is reassoc req
 * @frame: frame body
 * @frame_len: frame body length
 * @status: assoc req frame is handled successfully
 *
 * Return: Void
 */
void wlan_son_ind_assoc_req_frm(struct wlan_objmgr_vdev *vdev,
				uint8_t *macaddr, bool is_reassoc,
				uint8_t *frame, uint16_t frame_len,
				QDF_STATUS status);
/**
 * wlan_son_deliver_tx_power() - notify son module of tx power
 * @vdev: vdev
 * @max_pwr: max power in dBm unit
 *
 * Return: 0 if event is sent successfully
 */
int wlan_son_deliver_tx_power(struct wlan_objmgr_vdev *vdev,
			      int32_t max_pwr);

/**
 * wlan_son_deliver_vdev_stop() - notify son module of vdev stop
 * @vdev: vdev
 *
 * Return: 0 if event is sent successfully
 */
int wlan_son_deliver_vdev_stop(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_son_deliver_inst_rssi() - notify son module of inst rssi
 * @vdev: vdev
 * @peer: peer device
 * @irssi: inst rssi above the noise floor in dB unit
 *
 * Return: 0 if event is sent successfully
 */
int wlan_son_deliver_inst_rssi(struct wlan_objmgr_vdev *vdev,
			       struct wlan_objmgr_peer *peer,
			       uint32_t irssi);

/**
 * wlan_son_deliver_opmode() - notity user app of opmode
 * @vdev: vdev objmgr
 * @bw: channel width defined in enum eSirMacHTChannelWidth
 * @nss: supported rx nss
 * @addr: source addr
 *
 * Return: 0 if event is sent successfully
 */
int wlan_son_deliver_opmode(struct wlan_objmgr_vdev *vdev,
			    uint8_t bw,
			    uint8_t nss,
			    uint8_t *addr);

/**
 * wlan_son_deliver_smps() - notity user app of smps
 * @vdev: vdev objmgr
 * @is_static: is_static
 * @addr: source addr
 *
 * Return: 0 if event is sent successfully
 */
int wlan_son_deliver_smps(struct wlan_objmgr_vdev *vdev,
			  uint8_t is_static,
			  uint8_t *addr);

/**
 * wlan_son_deliver_rrm_rpt() - notity son module of rrm rpt
 * @vdev: vdev objmgr
 * @addr: sender addr
 * @frm: points to measurement report
 * @flen: frame length
 *
 * Return: 0 if event is sent successfully
 */
int wlan_son_deliver_rrm_rpt(struct wlan_objmgr_vdev *vdev,
			     uint8_t *addr,
			     uint8_t *frm,
			     uint32_t flen);

#else

static inline bool wlan_son_peer_is_kickout_allow(struct wlan_objmgr_vdev *vdev,
						  uint8_t *macaddr)
{
	return true;
}

static inline
void wlan_son_ind_assoc_req_frm(struct wlan_objmgr_vdev *vdev,
				uint8_t *macaddr, bool is_reassoc,
				uint8_t *frame, uint16_t frame_len,
				QDF_STATUS status)
{
}

static inline
int wlan_son_deliver_tx_power(struct wlan_objmgr_vdev *vdev,
			      int32_t max_pwr)
{
	return -EINVAL;
}

static inline
int wlan_son_deliver_vdev_stop(struct wlan_objmgr_vdev *vdev)
{
	return -EINVAL;
}

static inline
int wlan_son_deliver_inst_rssi(struct wlan_objmgr_vdev *vdev,
			       struct wlan_objmgr_peer *peer,
			       uint32_t irssi)
{
	return -EINVAL;
}

static inline
int wlan_son_deliver_opmode(struct wlan_objmgr_vdev *vdev,
			    uint8_t bw,
			    uint8_t nss,
			    uint8_t *addr)
{
	return -EINVAL;
}

static inline
int wlan_son_deliver_smps(struct wlan_objmgr_vdev *vdev,
			  uint8_t is_static,
			  uint8_t *addr)
{
	return -EINVAL;
}

static inline
int wlan_son_deliver_rrm_rpt(struct wlan_objmgr_vdev *vdev,
			     uint8_t *mac_addr,
			     uint8_t *frm,
			     uint32_t flen)
{
	return -EINVAL;
}
#endif /*WLAN_FEATURE_SON*/
#endif
