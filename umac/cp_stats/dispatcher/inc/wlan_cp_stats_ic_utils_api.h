/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cp_stats_ic_utils_api.h
 *
 * This header file provide definitions and declarations required for northbound
 * specific to WIN
 */

#ifndef __WLAN_CP_STATS_IC_UTILS_API_H__
#define __WLAN_CP_STATS_IC_UTILS_API_H__

#ifdef QCA_SUPPORT_CP_STATS
#include "wlan_cp_stats_ic_ucfg_api.h"

#define PEER_CP_STATS_SET_FUNCS(field) \
	static inline void \
	peer_cp_stats_##field##_inc(struct wlan_objmgr_peer *_peer, \
				    uint64_t _val) \
	{ \
		ucfg_peer_cp_stats_##field##_inc(_peer, _val); \
	} \
	static inline void \
	peer_cp_stats_##field##_dec(struct wlan_objmgr_peer *_peer, \
				    uint64_t _val) \
	{ \
		ucfg_peer_cp_stats_##field##_inc(_peer, _val); \
	} \
	static inline void \
	peer_cp_stats_##field##_update(struct wlan_objmgr_peer *_peer, \
				       uint64_t _val) \
	{ \
		ucfg_peer_cp_stats_##field##_update(_peer, _val); \
	}

PEER_CP_STATS_SET_FUNCS(rx_mgmt);
PEER_CP_STATS_SET_FUNCS(tx_mgmt);
PEER_CP_STATS_SET_FUNCS(rx_mgmt_rate);
PEER_CP_STATS_SET_FUNCS(is_tx_not_ok);
PEER_CP_STATS_SET_FUNCS(rx_noprivacy);
PEER_CP_STATS_SET_FUNCS(rx_wepfail);
PEER_CP_STATS_SET_FUNCS(rx_tkipicv);
PEER_CP_STATS_SET_FUNCS(rx_wpimic);
PEER_CP_STATS_SET_FUNCS(rx_ccmpmic);
PEER_CP_STATS_SET_FUNCS(ps_discard);
PEER_CP_STATS_SET_FUNCS(psq_drops);
PEER_CP_STATS_SET_FUNCS(tx_assoc);
PEER_CP_STATS_SET_FUNCS(tx_assoc_fail);
PEER_CP_STATS_SET_FUNCS(is_tx_nobuf);
#ifdef ATH_SUPPORT_IQUE
PEER_CP_STATS_SET_FUNCS(tx_dropblock);
#endif

#define PEER_CP_STATS_GET_FUNCS(field) \
	static inline uint64_t \
	peer_cp_stats_##field##_get(struct wlan_objmgr_peer *_peer) \
	{ \
		return ucfg_peer_cp_stats_##field##_get(_peer); \
	}

PEER_CP_STATS_GET_FUNCS(rx_mgmt_rate);
#ifdef ATH_SUPPORT_IQUE
PEER_CP_STATS_GET_FUNCS(tx_dropblock);
#endif

static inline void
peer_cp_stats_rx_mgmt_rssi_update(struct wlan_objmgr_peer *peer,
				  int8_t rssi)
{
	ucfg_peer_cp_stats_rx_mgmt_rssi_update(peer, rssi);
}

static inline int8_t
peer_cp_stats_rx_mgmt_rssi_get(struct wlan_objmgr_peer *peer)
{
	return ucfg_peer_cp_stats_rx_mgmt_rssi_get(peer);
}

#define VDEV_UCAST_CP_STATS_SET_FUNCS(field) \
	static inline void \
	vdev_ucast_cp_stats_##field##_inc(struct wlan_objmgr_vdev *_vdev, \
					  uint64_t _val) \
	{ \
		ucfg_vdev_ucast_cp_stats_##field##_inc(_vdev, _val); \
	} \
	static inline void \
	vdev_ucast_cp_stats_##field##_update(struct wlan_objmgr_vdev *_vdev, \
					     uint64_t _val) \
	{ \
		ucfg_vdev_ucast_cp_stats_##field##_update(_vdev, _val); \
	}

VDEV_UCAST_CP_STATS_SET_FUNCS(rx_badkeyid);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_decryptok);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_wepfail);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_tkipicv);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_tkipreplay);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_tkipformat);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_ccmpmic);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_ccmpreplay);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_ccmpformat);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_wpimic);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_wpireplay);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_countermeasure);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_mgmt);
VDEV_UCAST_CP_STATS_SET_FUNCS(tx_mgmt);
VDEV_UCAST_CP_STATS_SET_FUNCS(rx_discard);
VDEV_UCAST_CP_STATS_SET_FUNCS(tx_discard);

#define VDEV_UCAST_CP_STATS_GET_FUNCS(field) \
	static inline uint64_t \
	vdev_ucast_cp_stats_##field##_get(struct wlan_objmgr_vdev *_vdev) \
	{ \
		return ucfg_vdev_ucast_cp_stats_##field##_get(_vdev); \
	}

VDEV_UCAST_CP_STATS_GET_FUNCS(rx_wepfail);
VDEV_UCAST_CP_STATS_GET_FUNCS(rx_decryptok);
VDEV_UCAST_CP_STATS_GET_FUNCS(rx_ccmpmic);
VDEV_UCAST_CP_STATS_GET_FUNCS(rx_ccmpreplay);

#define VDEV_MCAST_CP_STATS_SET_FUNCS(field) \
	static inline void \
	vdev_mcast_cp_stats_##field##_inc(struct wlan_objmgr_vdev *_vdev, \
					  uint64_t _val) \
	{ \
		ucfg_vdev_mcast_cp_stats_##field##_inc(_vdev, _val); \
	} \
	static inline void \
	vdev_mcast_cp_stats_##field##_update(struct wlan_objmgr_vdev *_vdev, \
					     uint64_t _val) \
	{ \
		ucfg_vdev_mcast_cp_stats_##field##_update(_vdev, _val); \
	}

VDEV_MCAST_CP_STATS_SET_FUNCS(rx_badkeyid);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_decryptok);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_wepfail);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_tkipicv);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_tkipreplay);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_tkipformat);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_ccmpmic);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_ccmpreplay);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_ccmpformat);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_wpimic);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_wpireplay);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_countermeasure);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_mgmt);
VDEV_MCAST_CP_STATS_SET_FUNCS(tx_mgmt);
VDEV_MCAST_CP_STATS_SET_FUNCS(rx_discard);
VDEV_MCAST_CP_STATS_SET_FUNCS(tx_discard);

#define VDEV_MCAST_CP_STATS_GET_FUNCS(field) \
	static inline uint64_t \
	vdev_mcast_cp_stats_##field##_get(struct wlan_objmgr_vdev *_vdev) \
	{ \
		return ucfg_vdev_mcast_cp_stats_##field##_get(_vdev); \
	}

VDEV_MCAST_CP_STATS_GET_FUNCS(rx_wepfail);
VDEV_MCAST_CP_STATS_GET_FUNCS(rx_decryptok);
VDEV_MCAST_CP_STATS_GET_FUNCS(rx_ccmpmic);
VDEV_MCAST_CP_STATS_GET_FUNCS(rx_ccmpreplay);

#define VDEV_CP_STATS_SET_FUNCS(field) \
	static inline void \
	vdev_cp_stats_##field##_inc(struct wlan_objmgr_vdev *_vdev, \
				    uint64_t _val) \
	{ \
		ucfg_vdev_cp_stats_##field##_inc(_vdev, _val); \
	} \
	static inline void \
	vdev_cp_stats_##field##_update(struct wlan_objmgr_vdev *_vdev, \
				       uint64_t _val) \
	{ \
		ucfg_vdev_cp_stats_##field##_update(_vdev, _val); \
	} \
	static inline void \
	vdev_cp_stats_##field##_dec(struct wlan_objmgr_vdev *_vdev, \
				    uint64_t _val) \
	{ \
		ucfg_vdev_cp_stats_##field##_dec(_vdev, _val); \
	}

VDEV_CP_STATS_SET_FUNCS(rx_wrongbss);
VDEV_CP_STATS_SET_FUNCS(rx_tooshort);
VDEV_CP_STATS_SET_FUNCS(rx_wrongdir);
VDEV_CP_STATS_SET_FUNCS(rx_not_assoc);
VDEV_CP_STATS_SET_FUNCS(rx_noprivacy);
VDEV_CP_STATS_SET_FUNCS(rx_mgmt_discard);
VDEV_CP_STATS_SET_FUNCS(rx_ctl);
VDEV_CP_STATS_SET_FUNCS(rx_rs_too_big);
VDEV_CP_STATS_SET_FUNCS(rx_elem_missing);
VDEV_CP_STATS_SET_FUNCS(rx_elem_too_big);
VDEV_CP_STATS_SET_FUNCS(rx_chan_err);
VDEV_CP_STATS_SET_FUNCS(rx_node_alloc);
VDEV_CP_STATS_SET_FUNCS(rx_ssid_mismatch);
VDEV_CP_STATS_SET_FUNCS(rx_auth_unsupported);
VDEV_CP_STATS_SET_FUNCS(rx_auth_fail);
VDEV_CP_STATS_SET_FUNCS(rx_auth_countermeasures);
VDEV_CP_STATS_SET_FUNCS(rx_assoc_bss);
VDEV_CP_STATS_SET_FUNCS(rx_assoc_notauth);
VDEV_CP_STATS_SET_FUNCS(rx_assoc_cap_mismatch);
VDEV_CP_STATS_SET_FUNCS(rx_assoc_norate);
VDEV_CP_STATS_SET_FUNCS(rx_assoc_wpaie_err);
VDEV_CP_STATS_SET_FUNCS(rx_action);
VDEV_CP_STATS_SET_FUNCS(rx_auth_err);
VDEV_CP_STATS_SET_FUNCS(tx_nodefkey);
VDEV_CP_STATS_SET_FUNCS(tx_noheadroom);
VDEV_CP_STATS_SET_FUNCS(rx_acl);
VDEV_CP_STATS_SET_FUNCS(rx_nowds);
VDEV_CP_STATS_SET_FUNCS(tx_nobuf);
VDEV_CP_STATS_SET_FUNCS(tx_nonode);
VDEV_CP_STATS_SET_FUNCS(tx_cipher_err);
VDEV_CP_STATS_SET_FUNCS(tx_not_ok);
VDEV_CP_STATS_SET_FUNCS(tx_bcn_swba);
VDEV_CP_STATS_SET_FUNCS(node_timeout);
VDEV_CP_STATS_SET_FUNCS(crypto_nomem);
VDEV_CP_STATS_SET_FUNCS(crypto_tkip);
VDEV_CP_STATS_SET_FUNCS(crypto_tkipenmic);
VDEV_CP_STATS_SET_FUNCS(crypto_tkipcm);
VDEV_CP_STATS_SET_FUNCS(crypto_ccmp);
VDEV_CP_STATS_SET_FUNCS(crypto_wep);
VDEV_CP_STATS_SET_FUNCS(crypto_setkey_cipher);
VDEV_CP_STATS_SET_FUNCS(crypto_setkey_nokey);
VDEV_CP_STATS_SET_FUNCS(crypto_delkey);
VDEV_CP_STATS_SET_FUNCS(crypto_cipher_err);
VDEV_CP_STATS_SET_FUNCS(crypto_attach_fail);
VDEV_CP_STATS_SET_FUNCS(crypto_swfallback);
VDEV_CP_STATS_SET_FUNCS(crypto_keyfail);
VDEV_CP_STATS_SET_FUNCS(ibss_capmismatch);
VDEV_CP_STATS_SET_FUNCS(ps_unassoc);
VDEV_CP_STATS_SET_FUNCS(ps_aid_err);
VDEV_CP_STATS_SET_FUNCS(tx_offchan_mgmt);
VDEV_CP_STATS_SET_FUNCS(tx_offchan_data);
VDEV_CP_STATS_SET_FUNCS(tx_offchan_fail);
VDEV_CP_STATS_SET_FUNCS(invalid_macaddr_nodealloc_fail);
VDEV_CP_STATS_SET_FUNCS(tx_bcn_success);
VDEV_CP_STATS_SET_FUNCS(tx_bcn_outage);
VDEV_CP_STATS_SET_FUNCS(sta_xceed_rlim);
VDEV_CP_STATS_SET_FUNCS(sta_xceed_vlim);
VDEV_CP_STATS_SET_FUNCS(mlme_auth_attempt);
VDEV_CP_STATS_SET_FUNCS(mlme_auth_success);
VDEV_CP_STATS_SET_FUNCS(authorize_attempt);
VDEV_CP_STATS_SET_FUNCS(authorize_success);
VDEV_CP_STATS_SET_FUNCS(peer_delete_req);
VDEV_CP_STATS_SET_FUNCS(peer_delete_resp);

#define VDEV_CP_STATS_GET_FUNCS(field) \
	static inline uint64_t \
	vdev_cp_stats_##field##_get(struct wlan_objmgr_vdev *_vdev) \
	{ \
		return ucfg_vdev_cp_stats_##field##_get(_vdev); \
	}

VDEV_CP_STATS_GET_FUNCS(rx_wrongbss);
VDEV_CP_STATS_GET_FUNCS(rx_wrongdir);
VDEV_CP_STATS_GET_FUNCS(rx_ssid_mismatch);

static inline void vdev_cp_stats_reset(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_cp_stats *vdev_cps;
	struct vdev_ic_cp_stats *vdev_cs;

	if (!vdev)
		return;

	vdev_cps = wlan_cp_stats_get_vdev_stats_obj(vdev);
	if (!vdev_cps)
		return;

	vdev_cs = vdev_cps->vdev_stats;
	qdf_mem_set(vdev_cs, sizeof(struct vdev_ic_cp_stats), 0x0);
}

#define PDEV_CP_STATS_SET_FUNCS(field) \
	static inline void \
	pdev_cp_stats_##field##_inc(struct wlan_objmgr_pdev *_pdev, \
				    uint64_t _val) \
	{ \
		ucfg_pdev_cp_stats_##field##_inc(_pdev, _val); \
	} \
	static inline void \
	pdev_cp_stats_##field##_update(struct wlan_objmgr_pdev *_pdev, \
				       uint64_t _val) \
	{ \
		ucfg_pdev_cp_stats_##field##_update(_pdev, _val); \
	}

PDEV_CP_STATS_SET_FUNCS(tx_beacon);
PDEV_CP_STATS_SET_FUNCS(be_nobuf);
PDEV_CP_STATS_SET_FUNCS(tx_buf_count);
PDEV_CP_STATS_SET_FUNCS(tx_mgmt);
PDEV_CP_STATS_SET_FUNCS(rx_mgmt);
PDEV_CP_STATS_SET_FUNCS(rx_num_mgmt);
PDEV_CP_STATS_SET_FUNCS(rx_num_ctl);
PDEV_CP_STATS_SET_FUNCS(tx_rssi);
PDEV_CP_STATS_SET_FUNCS(rx_rssi_comb);
PDEV_CP_STATS_SET_FUNCS(rx_overrun);
PDEV_CP_STATS_SET_FUNCS(rx_phy_err);
PDEV_CP_STATS_SET_FUNCS(rx_ack_err);
PDEV_CP_STATS_SET_FUNCS(rx_rts_err);
PDEV_CP_STATS_SET_FUNCS(rx_rts_success);
PDEV_CP_STATS_SET_FUNCS(no_beacons);
PDEV_CP_STATS_SET_FUNCS(mib_int_count);
PDEV_CP_STATS_SET_FUNCS(rx_looplimit_start);
PDEV_CP_STATS_SET_FUNCS(rx_looplimit_end);
PDEV_CP_STATS_SET_FUNCS(ap_stats_tx_cal_enable);
PDEV_CP_STATS_SET_FUNCS(tgt_asserts);
PDEV_CP_STATS_SET_FUNCS(chan_nf);
PDEV_CP_STATS_SET_FUNCS(chan_nf_sec80);
PDEV_CP_STATS_SET_FUNCS(wmi_tx_mgmt);
PDEV_CP_STATS_SET_FUNCS(wmi_tx_mgmt_completions);
PDEV_CP_STATS_SET_FUNCS(wmi_tx_mgmt_completion_err);
PDEV_CP_STATS_SET_FUNCS(rx_mgmt_rssi_drop);
PDEV_CP_STATS_SET_FUNCS(tx_retries);
PDEV_CP_STATS_SET_FUNCS(tx_frame_count);
PDEV_CP_STATS_SET_FUNCS(rx_frame_count);
PDEV_CP_STATS_SET_FUNCS(rx_clear_count);
PDEV_CP_STATS_SET_FUNCS(cycle_count);
PDEV_CP_STATS_SET_FUNCS(phy_err_count);
PDEV_CP_STATS_SET_FUNCS(chan_tx_pwr);

#define PDEV_CHAN_STATS_GET_FUNCS(field) \
	static inline uint32_t \
	pdev_chan_stats_##field##_get(struct wlan_objmgr_pdev *_pdev) \
	{ \
		return ucfg_pdev_chan_stats_##field##_get(_pdev); \
	}

PDEV_CHAN_STATS_GET_FUNCS(self_bss_util);
PDEV_CHAN_STATS_GET_FUNCS(obss_util);
PDEV_CHAN_STATS_GET_FUNCS(obss_rx_util);
PDEV_CHAN_STATS_GET_FUNCS(ap_rx_util);
PDEV_CHAN_STATS_GET_FUNCS(ap_tx_util);
PDEV_CHAN_STATS_GET_FUNCS(free_medium);
PDEV_CHAN_STATS_GET_FUNCS(non_wifi_util);
PDEV_CHAN_STATS_GET_FUNCS(ss_under_util);
PDEV_CHAN_STATS_GET_FUNCS(sec_20_util);
PDEV_CHAN_STATS_GET_FUNCS(sec_40_util);
PDEV_CHAN_STATS_GET_FUNCS(sec_80_util);

#define PDEV_CP_STATS_GET_FUNCS(field) \
	static inline uint64_t \
	pdev_cp_stats_##field##_get(struct wlan_objmgr_pdev *_pdev) \
	{ \
		return ucfg_pdev_cp_stats_##field##_get(_pdev); \
	}

PDEV_CP_STATS_GET_FUNCS(tx_mgmt);
PDEV_CP_STATS_GET_FUNCS(rx_num_mgmt);
PDEV_CP_STATS_GET_FUNCS(ap_stats_tx_cal_enable);
PDEV_CP_STATS_GET_FUNCS(wmi_tx_mgmt);
PDEV_CP_STATS_GET_FUNCS(wmi_tx_mgmt_completions);
PDEV_CP_STATS_GET_FUNCS(wmi_tx_mgmt_completion_err);
PDEV_CP_STATS_GET_FUNCS(tgt_asserts);
PDEV_CP_STATS_GET_FUNCS(rx_phy_err);
PDEV_CP_STATS_GET_FUNCS(fcsbad);

static inline void pdev_cp_stats_reset(struct wlan_objmgr_pdev *pdev)
{
	ucfg_pdev_cp_stats_reset(pdev);
}

#define PDEV_LMAC_CP_STATS_SET_FUNCS(field) \
	static inline void \
	pdev_lmac_cp_stats_##field##_inc(struct wlan_objmgr_pdev *_pdev, \
					 uint64_t _val) \
	{ \
		ucfg_pdev_lmac_cp_stats_##field##_inc(_pdev, _val); \
	} \
	static inline void \
	pdev_lmac_cp_stats_##field##_update(struct wlan_objmgr_pdev *_pdev, \
					    uint64_t _val) \
	{ \
		ucfg_pdev_lmac_cp_stats_##field##_update(_pdev, _val); \
	}

PDEV_LMAC_CP_STATS_SET_FUNCS(ast_reset_on_error);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_hardware);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_halresets);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_bmiss);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_brssi);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_tx_fifoerr);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_tx_filtered);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_tx_noack);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_tx_shortpre);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_tx_altrate);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_tx_protect);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_rx_nobuf);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_rx_hal_in_progress);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_rx_num_unknown);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_per_cal);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_per_calfail);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_per_rfgain);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_ant_defswitch);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_bb_hang);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_mac_hang);
#ifdef ATH_WOW
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_wow_wakeups);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_wow_wakeupsok);
#endif
#ifdef ATH_SUPPORT_CFEND
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_cfend_sched);
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_cfend_sent);
#endif
PDEV_LMAC_CP_STATS_SET_FUNCS(ast_noise_floor);

#define PDEV_LMAC_CP_STATS_GET_FUNCS(field) \
	static inline uint64_t \
	pdev_lmac_cp_stats_##field##_get(struct wlan_objmgr_pdev *_pdev) \
	{ \
		return ucfg_pdev_lmac_cp_stats_##field##_get(_pdev); \
	}
PDEV_LMAC_CP_STATS_GET_FUNCS(ast_tx_fifoerr);
PDEV_LMAC_CP_STATS_GET_FUNCS(ast_tx_filtered);
PDEV_LMAC_CP_STATS_GET_FUNCS(ast_noise_floor);

/**
 * wlan_get_pdev_cp_stats_ref() - API to reference to pdev cp stats object
 * @pdev: pointer to pdev object
 *
 * Return: pdev_ic_cp_stats or NULL
 */
struct pdev_ic_cp_stats
*wlan_get_pdev_cp_stats_ref(struct wlan_objmgr_pdev *pdev);

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_IC_UTILS_API_H__ */
