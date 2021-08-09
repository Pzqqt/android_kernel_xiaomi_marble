
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
 * DOC : os_if_son.h
 *
 * WLAN Host Device Driver file for son (Self Organizing Network)
 * support.
 *
 */
#ifndef _OS_IF_SON_H_
#define _OS_IF_SON_H_

#include <qdf_types.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_reg_ucfg_api.h>
#include <ieee80211_external.h>

#define INVALID_WIDTH 0xFF

/**
 * struct son_callbacks - struct containing callback to non-converged driver
 * @os_if_is_acs_in_progress: whether acs is in progress or not
 * @os_if_set_chan_ext_offset: set chan extend offset
 * @os_if_get_chan_ext_offset: get chan extend offset
 * @os_if_set_bandwidth: set band width
 * @os_if_get_bandwidth: get band width
 * @os_if_set_chan: set chan
 * @os_if_set_country_code: set country code
 * @os_if_set_candidate_freq: set freq to switch after radar detection
 * @os_if_get_candidate_freq: get freq to switch after radar detection
 * @os_if_set_phymode: set phy mode
 * @os_if_get_phymode: get phy mode
 * @os_if_get_rx_nss: Gets number of RX spatial streams
 * @os_if_set_acl_policy: set acl policy
 * @os_if_get_acl_policy: get acl policy
 * @os_if_add_acl_mac: add mac to acl
 * @os_if_del_acl_mac: del mac from acl
 * @os_if_kickout_mac: kickout sta with given mac
 * @os_if_set_chwidth: set chan width
 * @os_if_get_chwidth: get chan width
 * @os_if_deauth_sta: Deauths the target peer
 * @os_if_modify_acl: Add/Del target peer in ACL
 */
struct son_callbacks {
	uint32_t (*os_if_is_acs_in_progress)(struct wlan_objmgr_vdev *vdev);
	int (*os_if_set_chan_ext_offset)(
				struct wlan_objmgr_vdev *vdev,
				enum sec20_chan_offset son_chan_ext_offset);
	enum sec20_chan_offset (*os_if_get_chan_ext_offset)(
				struct wlan_objmgr_vdev *vdev);
	int (*os_if_set_bandwidth)(struct wlan_objmgr_vdev *vdev,
				   uint32_t son_bandwidth);
	uint32_t (*os_if_get_bandwidth)(struct wlan_objmgr_vdev *vdev);
	int (*os_if_set_chan)(struct wlan_objmgr_vdev *vdev, int chan,
			      enum wlan_band_id son_band);
	uint32_t (*os_if_get_sta_count)(struct wlan_objmgr_vdev *vdev);
	int (*os_if_set_country_code)(struct wlan_objmgr_vdev *vdev,
				      char *country_code);
	int (*os_if_set_candidate_freq)(struct wlan_objmgr_vdev *vdev,
					qdf_freq_t freq);
	qdf_freq_t (*os_if_get_candidate_freq)(struct wlan_objmgr_vdev *vdev);
	int (*os_if_set_phymode)(struct wlan_objmgr_vdev *vdev,
				 enum ieee80211_phymode mode);
	enum ieee80211_phymode (*os_if_get_phymode)(
					struct wlan_objmgr_vdev *vdev);
	uint8_t (*os_if_get_rx_nss)(struct wlan_objmgr_vdev *vdev);
	QDF_STATUS (*os_if_set_acl_policy)(struct wlan_objmgr_vdev *vdev,
					   ieee80211_acl_cmd son_acl_policy);
	ieee80211_acl_cmd (*os_if_get_acl_policy)(
						struct wlan_objmgr_vdev *vdev);
	int (*os_if_add_acl_mac)(struct wlan_objmgr_vdev *vdev,
				 struct qdf_mac_addr *acl_mac);
	int (*os_if_del_acl_mac)(struct wlan_objmgr_vdev *vdev,
				 struct qdf_mac_addr *acl_mac);
	int (*os_if_kickout_mac)(struct wlan_objmgr_vdev *vdev,
				 struct qdf_mac_addr *acl_mac);
	int (*os_if_set_chwidth)(struct wlan_objmgr_vdev *vdev,
				 enum ieee80211_cwm_width son_chwidth);
	enum ieee80211_cwm_width (*os_if_get_chwidth)(
				struct wlan_objmgr_vdev *vdev);
	void (*os_if_deauth_sta)(struct wlan_objmgr_vdev *vdev,
				 uint8_t *peer_mac,
				 bool ignore_frame);
	void (*os_if_modify_acl)(struct wlan_objmgr_vdev *vdev,
				 uint8_t *peer_mac,
				 bool allow_auth);
};

/**
 * os_if_son_register_hdd_callbacks() - register son hdd callback
 * @psoc: psoc
 * @cb_obj: pointer to callback
 *
 * Return: void
 */
void os_if_son_register_hdd_callbacks(struct wlan_objmgr_psoc *psoc,
				      struct son_callbacks *cb_obj);

/**
 * os_if_son_get_freq() - get freq
 * @vdev: vdev
 *
 * Return: freq of given vdev
 */
qdf_freq_t os_if_son_get_freq(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_is_acs_in_progress() - whether ACS in progress or not
 * @vdev: vdev
 *
 * Return: true if ACS is in progress
 */
uint32_t os_if_son_is_acs_in_progress(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_is_cac_in_progress() - whether CAC is in progress or not
 * @vdev: vdev
 *
 * Return: true if CAC is in progress
 */
uint32_t os_if_son_is_cac_in_progress(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_set_chan_ext_offset() - set chan extend offset
 * @vdev: vdev
 * @son_chan_ext_offset son chan extend offset
 *
 * Return: 0 on success, negative errno on failure
 */
int os_if_son_set_chan_ext_offset(struct wlan_objmgr_vdev *vdev,
				  enum sec20_chan_offset son_chan_ext_offset);

/**
 * os_if_son_get_chan_ext_offset() - get chan extend offset
 * @vdev: vdev
 *
 * Return: enum sec20_chan_offset
 */
enum sec20_chan_offset os_if_son_get_chan_ext_offset(
					struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_set_bandwidth() - set band width
 * @vdev: vdev
 * @son_bandwidth: band width
 *
 * Return: 0 on success, negative errno on failure
 */
int os_if_son_set_bandwidth(struct wlan_objmgr_vdev *vdev,
			    uint32_t son_bandwidth);

/**
 * os_if_son_get_bandwidth() - get band width
 * @vdev: vdev
 *
 * Return: band width
 */
uint32_t os_if_son_get_bandwidth(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_get_band_info() - get band info
 * @vdev: vdev
 *
 * Return: band info
 */
uint32_t os_if_son_get_band_info(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_get_chan_list() - get a list of chan information
 * @vdev: vdev
 * @ic_chans: chan information array to get
 * @chan_info: pointer to ieee80211_channel_info to get
 * @ic_nchans: number of chan information it gets
 * @flag_160: flag indicating the API to fill the center frequencies of 160MHz.
 *
 * Return: 0 on success, negative errno on failure
 */
int os_if_son_get_chan_list(struct wlan_objmgr_vdev *vdev,
			    struct ieee80211_ath_channel *ic_chans,
			    struct ieee80211_channel_info *chan_info,
			    uint8_t *ic_nchans, bool flag_160);

/**
 * os_if_son_get_sta_count() - get connected STA count
 * @vdev: vdev
 *
 * Return: connected STA count
 */
uint32_t os_if_son_get_sta_count(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_get_bssid() - get bssid of given vdev
 * @vdev: vdev
 * @bssid: pointer to BSSID
 *
 * Return: 0 if BSSID is gotten successfully
 */
int os_if_son_get_bssid(struct wlan_objmgr_vdev *vdev,
			uint8_t bssid[QDF_MAC_ADDR_SIZE]);

/**
 * os_if_son_get_ssid() - get ssid of given vdev
 * @vdev: vdev
 * @ssid: pointer to SSID
 * @ssid_len: ssid length
 *
 * Return: 0 if SSID is gotten successfully
 */
int os_if_son_get_ssid(struct wlan_objmgr_vdev *vdev,
		       char ssid[WLAN_SSID_MAX_LEN + 1],
		       uint8_t *ssid_len);

/**
 * os_if_son_set_chan() - set chan
 * @vdev: vdev
 * @chan: given chan
 * @son_band: given band
 *
 * Return: 0 on success, negative errno on failure
 */
int os_if_son_set_chan(struct wlan_objmgr_vdev *vdev,
		       int chan, enum wlan_band_id son_band);

/**
 * os_if_son_set_cac_timeout() - set cac timeout
 * @vdev: vdev
 * @cac_timeout: cac timeount to set
 *
 * Return: 0 if cac time out is set successfully
 */
int os_if_son_set_cac_timeout(struct wlan_objmgr_vdev *vdev,
			      int cac_timeout);

/**
 * os_if_son_get_cac_timeout() - get cac timeout
 * @vdev: vdev
 * @cac_timeout: cac timeout to get
 *
 * Return 0 if cac time out is get successfully
 */
int os_if_son_get_cac_timeout(struct wlan_objmgr_vdev *vdev,
			      int *cac_timeout);

/**
 * os_if_son_set_country_code() - set country code
 * @vdev: vdev
 * @country_code: country code to set
 *
 * Return: 0 if country code is set successfully
 */
int os_if_son_set_country_code(struct wlan_objmgr_vdev *vdev,
			       char *country_code);

/**
 * os_if_son_get_country_code() - get country code
 * @vdev: vdev
 * @country_code: country code to get
 *
 * Return: 0 if country code is get successfully
 */
int os_if_son_get_country_code(struct wlan_objmgr_vdev *vdev,
			       char *country_code);

/**
 * os_if_son_set_candidate_freq() - set freq to switch after radar detection
 * @vdev: vdev
 * @freq: freq to switch
 *
 * Return: 0 if candidate freq is set successfully
 */
int os_if_son_set_candidate_freq(struct wlan_objmgr_vdev *vdev,
				 qdf_freq_t freq);

/**
 * os_if_son_get_candidate_freq() - get freq to switch after radar detection
 * @vdev: vdev
 *
 * Return: candidate freq to switch after radar detection
 */
qdf_freq_t os_if_son_get_candidate_freq(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_get_phymode() - get phy mode
 * @vdev: vdev
 *
 * Return: enum ieee80211_phymode
 */
enum ieee80211_phymode os_if_son_get_phymode(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_set_phymode() - set phy mode
 * @vdev: vdev
 * @mode: son phy mode to set
 *
 * Return: 0 on success, negative errno on failure
 */
int os_if_son_set_phymode(struct wlan_objmgr_vdev *vdev,
			  enum ieee80211_phymode mode);

/**
 * os_if_son_get_phy_stats() - get phy stats
 * @vdev: vdev
 * @phy_stats: phy stats
 *
 * Return: void
 */
void os_if_son_get_phy_stats(struct wlan_objmgr_vdev *vdev,
			     struct ol_ath_radiostats *phy_stats);

/**
 * os_if_son_get_chan_util() - get chan utilization
 * @vdev: vdev
 *
 * Return: chan utilization (0 - 100)
 */
uint8_t os_if_son_get_chan_util(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_pdev_ops() - Handles PDEV specific SON commands
 * @pdev: pdev
 * @type: SON command to handle
 * @data: Input Data
 * @ret: Output Data
 *
 * Return: QDF_SUCCCESS_SUCCESS in case of success
 */
QDF_STATUS os_if_son_pdev_ops(struct wlan_objmgr_pdev *pdev,
			      enum wlan_mlme_pdev_param type,
			      void *data, void *ret);

/**
 * os_if_son_vdev_ops() - Handles VDEV specific SON commands
 * @vdev: vdev
 * @type: SON command to handle
 * @data: Input Data
 * @ret: Output Data
 *
 * Return: QDF_SUCCCESS_SUCCESS in case of success
 */
QDF_STATUS os_if_son_vdev_ops(struct wlan_objmgr_vdev *pdev,
			      enum wlan_mlme_vdev_param type,
			      void *data, void *ret);

/**
 * os_if_son_peer_ops() - Handles PEER specific SON commands
 * @peer: peer
 * @type: SON command to handle
 * @data: Input Data. Pointer to wlan_mlme_peer_data
 * @ret: Output Data. Pointer to wlan_mlme_peer_data
 *
 * Return: QDF_SUCCCESS_SUCCESS in case of success
 */
QDF_STATUS os_if_son_peer_ops(struct wlan_objmgr_peer *peer,
			      enum wlan_mlme_peer_param type,
			      union wlan_mlme_peer_data *data,
			      union wlan_mlme_peer_data *ret);

/**
 * os_if_son_scan_db_iterate() - get country code
 * @pdev: pdev
 * @handler: scan_iterator
 * @arg: argument to be passed to handler
 *
 * Return: QDF_SUCCCESS_SUCCESS in case of success
 */
QDF_STATUS os_if_son_scan_db_iterate(struct wlan_objmgr_pdev *pdev,
				     scan_iterator_func handler, void *arg);

/**
 * os_if_son_acl_is_probe_wh_set() - Withheld probes for given mac_addr,
 *				     not supported
 * @vdev: vdev
 * @mac_addr: 6-Byte MAC address
 * @probe_rssi: Probe Request RSSI
 *
 * Return: true / false
 */
bool os_if_son_acl_is_probe_wh_set(struct wlan_objmgr_vdev *vdev,
				   const uint8_t *mac_addr,
				   uint8_t probe_rssi);

/**
 * os_if_son_get_rx_streams() - Gets number of RX spatial streams
 * @vdev: target vdev
 *
 * Return: number of spatial stream
 */
uint8_t os_if_son_get_rx_streams(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_cfg80211_reply() - replies to cfg80211
 * @sk_buf: sk_buff to uper layer
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS os_if_son_cfg80211_reply(qdf_nbuf_t sk_buf);

/**
 * os_if_son_vdev_is_wds() - checks if wds capability is supported or not
 * @vdev: Pointer to vdev
 *
 * Return: true if wds is supported
 */
bool os_if_son_vdev_is_wds(struct wlan_objmgr_vdev *vdev);

/*
 * os_if_son_set_acl_policy() - set acl policy
 * @vdev: vdev
 * @son_acl_policy: son acl policy. enum ieee80211_acl_cmd
 *
 * Return: QDF_STATUS
 */
QDF_STATUS os_if_son_set_acl_policy(struct wlan_objmgr_vdev *vdev,
				    ieee80211_acl_cmd son_acl_policy);

/**
 * os_if_son_get_acl_policy() - get acl policy
 * @vdev: vdev
 *
 * Return: acl policy. enum ieee80211_acl_cmd
 */
ieee80211_acl_cmd os_if_son_get_acl_policy(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_add_acl_mac() - add mac to acl
 * @vdev: vdev
 * @acl_mac: mac to add
 *
 * Return: 0 on success, negative errno on failure
 */
int os_if_son_add_acl_mac(struct wlan_objmgr_vdev *vdev,
			  struct qdf_mac_addr *acl_mac);

/**
 * os_if_son_del_acl_mac() - del mac from acl
 * @vdev: vdev
 * @acl_mac: mac to del
 *
 * Return: 0 on success, negative errno on failure
 */
int os_if_son_del_acl_mac(struct wlan_objmgr_vdev *vdev,
			  struct qdf_mac_addr *acl_mac);

/**
 * os_if_son_kickout_mac() - kickout sta with given mac
 * @vdev: vdev
 * @acl_mac: sta mac to kickout
 *
 * Return: 0 on success, negative errno on failure
 */
int os_if_son_kickout_mac(struct wlan_objmgr_vdev *vdev,
			  struct qdf_mac_addr *mac);

/**
 * os_if_son_set_chwidth() - set chan width
 * @vdev: vdev
 * @son_chwidth: son chan width
 *
 * Return: 0 on success, negative errno on failure
 */
int os_if_son_set_chwidth(struct wlan_objmgr_vdev *vdev,
			  enum ieee80211_cwm_width son_chwidth);

/**
 * os_if_son_get_chwidth() - get chan width
 * @vdev: vdev
 *
 * Return: son chan width
 */
enum ieee80211_cwm_width os_if_son_get_chwidth(struct wlan_objmgr_vdev *vdev);

/**
 * os_if_son_deauth_peer_sta - Deauths specified STA
 * @vdev: vdev
 * @peer_mac: Target peer MAC address
 * @ignore_frame: True to silently deauth the peer
 *
 * Return: void
 */
void os_if_son_deauth_peer_sta(struct wlan_objmgr_vdev *vdev,
			       uint8_t *peer_mac,
			       bool ignore_frame);

/**
 * os_if_son_modify_acl - Updates ACL with given peer
 * @vdev: vdev
 * @peer_mac: Target peer MAC address
 * @allow_auth: True to allow specified peer to connect
 *
 * Return: void
 */
void os_if_son_modify_acl(struct wlan_objmgr_vdev *vdev,
			  uint8_t *peer_mac,
			  bool allow_auth);
#endif
