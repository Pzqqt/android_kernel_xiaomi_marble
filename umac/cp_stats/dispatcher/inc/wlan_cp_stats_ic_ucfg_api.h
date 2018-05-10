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
 * DOC: wlan_cp_stats_ic_ucfg_api.h
 *
 */

#ifndef __WLAN_CP_STATS_IC_UCFG_API_H__
#define __WLAN_CP_STATS_IC_UCFG_API_H__

#ifdef QCA_SUPPORT_CP_STATS
#include <wlan_objmgr_cmn.h>
#include <wlan_cp_stats_ic_defs.h>
#include <wlan_cp_stats_ic_dcs_defs.h>
#include "../../core/src/wlan_cp_stats_defs.h"
#ifdef WLAN_ATF_ENABLE
#include <wlan_cp_stats_ic_atf_defs.h>
#endif

#define UCFG_PDEV_CP_STATS_SET_FUNCS(field) \
	static inline void \
	ucfg_pdev_cp_stats_##field##_inc(struct wlan_objmgr_pdev *_pdev, \
					 uint64_t _val) \
	{ \
		struct pdev_cp_stats *_pdev_cs = \
			wlan_cp_stats_get_pdev_stats_obj(_pdev); \
		if (_pdev_cs) { \
			struct pdev_ic_cp_stats *_pdev_ics = \
						_pdev_cs->pdev_stats; \
			if (_pdev_ics) { \
				_pdev_ics->stats.cs_##field += _val;\
			} \
		} \
	} \
	static inline void \
	ucfg_pdev_cp_stats_##field##_dec(struct wlan_objmgr_pdev *_pdev, \
					 uint64_t _val) \
	{ \
		struct pdev_cp_stats *_pdev_cs = \
			wlan_cp_stats_get_pdev_stats_obj(_pdev); \
		if (_pdev_cs) { \
			struct pdev_ic_cp_stats *_pdev_ics = \
						_pdev_cs->pdev_stats; \
			if (_pdev_ics) { \
				_pdev_ics->stats.cs_##field -= _val;\
			} \
		} \
	} \
	static inline void \
	ucfg_pdev_cp_stats_##field##_update(struct wlan_objmgr_pdev *_pdev, \
					    uint64_t _val) \
	{ \
		struct pdev_cp_stats *_pdev_cs = \
				wlan_cp_stats_get_pdev_stats_obj(_pdev); \
		if (_pdev_cs) { \
			struct pdev_ic_cp_stats *_pdev_ics = \
						_pdev_cs->pdev_stats; \
			if (_pdev_ics) { \
				_pdev_ics->stats.cs_##field = _val;\
			} \
		} \
	}

UCFG_PDEV_CP_STATS_SET_FUNCS(tx_beacon);
UCFG_PDEV_CP_STATS_SET_FUNCS(be_nobuf);
UCFG_PDEV_CP_STATS_SET_FUNCS(tx_buf_count);
UCFG_PDEV_CP_STATS_SET_FUNCS(tx_packets);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_packets);
UCFG_PDEV_CP_STATS_SET_FUNCS(tx_mgmt);
UCFG_PDEV_CP_STATS_SET_FUNCS(tx_num_data);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_num_data);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_mgmt);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_num_mgmt);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_num_ctl);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_ctrl);
UCFG_PDEV_CP_STATS_SET_FUNCS(tx_ctrl);
UCFG_PDEV_CP_STATS_SET_FUNCS(tx_rssi);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_rssi_comb);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_bytes);
UCFG_PDEV_CP_STATS_SET_FUNCS(tx_bytes);
UCFG_PDEV_CP_STATS_SET_FUNCS(tx_compaggr);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_aggr);
UCFG_PDEV_CP_STATS_SET_FUNCS(tx_bawadv);
UCFG_PDEV_CP_STATS_SET_FUNCS(tx_compunaggr);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_overrun);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_crypt_err);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_mic_err);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_crc_err);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_phy_err);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_ack_err);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_rts_err);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_rts_success);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_fcs_err);
UCFG_PDEV_CP_STATS_SET_FUNCS(no_beacons);
UCFG_PDEV_CP_STATS_SET_FUNCS(mib_int_count);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_looplimit_start);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_looplimit_end);
UCFG_PDEV_CP_STATS_SET_FUNCS(ap_stats_tx_cal_enable);
UCFG_PDEV_CP_STATS_SET_FUNCS(tgt_asserts);
UCFG_PDEV_CP_STATS_SET_FUNCS(chan_nf);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_last_msdu_unset_cnt);
UCFG_PDEV_CP_STATS_SET_FUNCS(chan_nf_sec80);
UCFG_PDEV_CP_STATS_SET_FUNCS(wmi_tx_mgmt);
UCFG_PDEV_CP_STATS_SET_FUNCS(wmi_tx_mgmt_completions);
UCFG_PDEV_CP_STATS_SET_FUNCS(wmi_tx_mgmt_completion_err);
UCFG_PDEV_CP_STATS_SET_FUNCS(peer_delete_req);
UCFG_PDEV_CP_STATS_SET_FUNCS(peer_delete_resp);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_mgmt_rssi_drop);
UCFG_PDEV_CP_STATS_SET_FUNCS(tx_retries);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_data_bytes);
UCFG_PDEV_CP_STATS_SET_FUNCS(tx_frame_count);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_frame_count);
UCFG_PDEV_CP_STATS_SET_FUNCS(rx_clear_count);
UCFG_PDEV_CP_STATS_SET_FUNCS(cycle_count);
UCFG_PDEV_CP_STATS_SET_FUNCS(phy_err_count);
UCFG_PDEV_CP_STATS_SET_FUNCS(chan_tx_pwr);
UCFG_PDEV_CP_STATS_SET_FUNCS(self_bss_util);
UCFG_PDEV_CP_STATS_SET_FUNCS(obss_util);

#define UCFG_PDEV_CP_STATS_GET_FUNCS(field) \
	static inline uint64_t \
	ucfg_pdev_cp_stats_##field##_get(struct wlan_objmgr_pdev *_pdev) \
	{ \
		struct pdev_cp_stats *_pdev_cs = \
				wlan_cp_stats_get_pdev_stats_obj(_pdev); \
		struct pdev_ic_cp_stats *_pdev_ics; \
		if (_pdev_cs) { \
			_pdev_ics = _pdev_cs->pdev_stats; \
			if (_pdev_ics) \
				return _pdev_ics->stats.cs_##field; \
		} \
		return 0; \
	}

UCFG_PDEV_CP_STATS_GET_FUNCS(ap_stats_tx_cal_enable);
UCFG_PDEV_CP_STATS_GET_FUNCS(wmi_tx_mgmt);
UCFG_PDEV_CP_STATS_GET_FUNCS(wmi_tx_mgmt_completions);
UCFG_PDEV_CP_STATS_GET_FUNCS(wmi_tx_mgmt_completion_err);
UCFG_PDEV_CP_STATS_GET_FUNCS(tgt_asserts);

static inline void ucfg_pdev_cp_stats_reset(struct wlan_objmgr_pdev *_pdev)
{
	struct pdev_cp_stats *pdev_cps = NULL;

	pdev_cps = wlan_cp_stats_get_pdev_stats_obj(_pdev);
	if (!pdev_cps)
		return;

	qdf_mem_zero(pdev_cps->pdev_stats, sizeof(struct pdev_ic_cp_stats));
}

#define UCFG_VDEV_CP_STATS_SET_FUNCS(field) \
	static inline void \
	ucfg_vdev_cp_stats_##field##_inc(struct wlan_objmgr_vdev *_vdev, \
					 uint64_t _val) \
	{ \
		struct vdev_cp_stats *_vdev_cs = \
			wlan_cp_stats_get_vdev_stats_obj(_vdev); \
		if (_vdev_cs) { \
			struct vdev_ic_cp_stats *_vdev_ics = \
						_vdev_cs->vdev_stats; \
			if (_vdev_ics) { \
				_vdev_ics->stats.cs_##field += _val;\
			} \
		} \
	} \
	static inline void \
	ucfg_vdev_cp_stats_##field##_dec(struct wlan_objmgr_vdev *_vdev, \
					 uint64_t _val) \
	{ \
		struct vdev_cp_stats *_vdev_cs = \
			wlan_cp_stats_get_vdev_stats_obj(_vdev); \
		if (_vdev_cs) { \
			struct vdev_ic_cp_stats *_vdev_ics = \
						_vdev_cs->vdev_stats; \
			if (_vdev_ics) { \
				_vdev_ics->stats.cs_##field -= _val;\
			} \
		} \
	} \
	static inline void \
	ucfg_vdev_cp_stats_##field##_update(struct wlan_objmgr_vdev *_vdev, \
					    uint64_t _val) \
	{ \
		struct vdev_cp_stats *_vdev_cs = \
			wlan_cp_stats_get_vdev_stats_obj(_vdev); \
		if (_vdev_cs) { \
			struct vdev_ic_cp_stats *_vdev_ics = \
						_vdev_cs->vdev_stats; \
			if (_vdev_ics) { \
				_vdev_ics->stats.cs_##field = _val;\
			} \
		} \
	}

UCFG_VDEV_CP_STATS_SET_FUNCS(rx_wrongbss);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_wrongdir);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_not_assoc);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_noprivacy);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_mgmt_discard);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_ctl);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_rs_too_big);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_elem_missing);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_elem_too_big);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_chan_err);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_node_alloc);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_ssid_mismatch);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_auth_unsupported);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_auth_fail);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_auth_countermeasures);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_assoc_bss);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_assoc_notauth);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_assoc_cap_mismatch);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_assoc_norate);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_assoc_wpaie_err);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_action);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_auth_err);
UCFG_VDEV_CP_STATS_SET_FUNCS(tx_nodefkey);
UCFG_VDEV_CP_STATS_SET_FUNCS(tx_noheadroom);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_acl);
UCFG_VDEV_CP_STATS_SET_FUNCS(rx_nowds);
UCFG_VDEV_CP_STATS_SET_FUNCS(tx_nobuf);
UCFG_VDEV_CP_STATS_SET_FUNCS(tx_nonode);
UCFG_VDEV_CP_STATS_SET_FUNCS(tx_cipher_err);
UCFG_VDEV_CP_STATS_SET_FUNCS(tx_not_ok);
UCFG_VDEV_CP_STATS_SET_FUNCS(tx_bcn_swba);
UCFG_VDEV_CP_STATS_SET_FUNCS(node_timeout);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_nomem);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_tkip);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_tkipenmic);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_tkipcm);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_ccmp);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_wep);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_setkey_cipher);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_setkey_nokey);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_delkey);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_cipher_err);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_attach_fail);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_swfallback);
UCFG_VDEV_CP_STATS_SET_FUNCS(crypto_keyfail);
UCFG_VDEV_CP_STATS_SET_FUNCS(ibss_capmismatch);
UCFG_VDEV_CP_STATS_SET_FUNCS(ps_unassoc);
UCFG_VDEV_CP_STATS_SET_FUNCS(ps_aid_err);
UCFG_VDEV_CP_STATS_SET_FUNCS(tx_offchan_mgmt);
UCFG_VDEV_CP_STATS_SET_FUNCS(tx_offchan_data);
UCFG_VDEV_CP_STATS_SET_FUNCS(tx_offchan_fail);
UCFG_VDEV_CP_STATS_SET_FUNCS(invalid_macaddr_nodealloc_fail);
UCFG_VDEV_CP_STATS_SET_FUNCS(tx_bcn_success);
UCFG_VDEV_CP_STATS_SET_FUNCS(tx_bcn_outage);
UCFG_VDEV_CP_STATS_SET_FUNCS(sta_xceed_rlim);
UCFG_VDEV_CP_STATS_SET_FUNCS(sta_xceed_vlim);
UCFG_VDEV_CP_STATS_SET_FUNCS(mlme_auth_attempt);
UCFG_VDEV_CP_STATS_SET_FUNCS(mlme_auth_success);
UCFG_VDEV_CP_STATS_SET_FUNCS(authorize_attempt);
UCFG_VDEV_CP_STATS_SET_FUNCS(authorize_success);

#define UCFG_VDEV_CP_STATS_GET_FUNCS(field) \
	static inline uint64_t \
	ucfg_vdev_cp_stats_##field##_get(struct wlan_objmgr_vdev *_vdev) { \
		struct vdev_cp_stats *_vdev_cs = \
				wlan_cp_stats_get_vdev_stats_obj(_vdev); \
		struct vdev_ic_cp_stats *_vdev_ics; \
		if (_vdev_cs) { \
			_vdev_ics = _vdev_cs->vdev_stats; \
			if (_vdev_ics) \
				return _vdev_ics->stats.cs_##field; \
		} \
		return 0; \
	}

UCFG_VDEV_CP_STATS_GET_FUNCS(rx_wrongbss);
UCFG_VDEV_CP_STATS_GET_FUNCS(rx_wrongdir);
UCFG_VDEV_CP_STATS_GET_FUNCS(rx_ssid_mismatch);

#define UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(field) \
	static inline void \
	ucfg_vdev_ucast_cp_stats_##field##_inc(struct wlan_objmgr_vdev *_vdev, \
					       uint64_t _val) \
	{ \
		struct vdev_cp_stats *_vdev_cs = \
			wlan_cp_stats_get_vdev_stats_obj(_vdev); \
		if (_vdev_cs) { \
			struct vdev_ic_cp_stats *_vdev_ics = \
						_vdev_cs->vdev_stats; \
			if (_vdev_ics) { \
				_vdev_ics->ucast_stats.cs_##field += _val;\
			} \
		} \
	} \
	static inline void \
	ucfg_vdev_ucast_cp_stats_##field##_dec(struct wlan_objmgr_vdev *_vdev, \
					       uint64_t _val) \
	{ \
		struct vdev_cp_stats *_vdev_cs = \
			wlan_cp_stats_get_vdev_stats_obj(_vdev); \
		if (_vdev_cs) { \
			struct vdev_ic_cp_stats *_vdev_ics = \
						_vdev_cs->vdev_stats; \
			if (_vdev_ics) { \
				_vdev_ics->ucast_stats.cs_##field -= _val;\
			} \
		} \
	} \
	static inline void ucfg_vdev_ucast_cp_stats_##field##_update( \
			struct wlan_objmgr_vdev *_vdev, uint64_t _val) \
	{ \
		struct vdev_cp_stats *_vdev_cs = \
			wlan_cp_stats_get_vdev_stats_obj(_vdev); \
		if (_vdev_cs) { \
			struct vdev_ic_cp_stats *_vdev_ics = \
						_vdev_cs->vdev_stats; \
			if (_vdev_ics) { \
				_vdev_ics->ucast_stats.cs_##field = _val;\
			} \
		} \
	}

UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_badkeyid);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_decryptok);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_wepfail);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_tkipicv);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_tkipreplay);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_tkipformat);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_ccmpmic);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_ccmpreplay);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_ccmpformat);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_wpimic);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_wpireplay);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_countermeasure);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(rx_mgmt);
UCFG_VDEV_UCAST_CP_STATS_SET_FUNCS(tx_mgmt);

#define UCFG_VDEV_UCAST_CP_STATS_GET_FUNCS(field) \
	static inline uint64_t \
	ucfg_vdev_ucast_cp_stats_##field##_get(struct wlan_objmgr_vdev *_vdev) \
	{ \
		struct vdev_cp_stats *_vdev_cs = \
				wlan_cp_stats_get_vdev_stats_obj(_vdev); \
		struct vdev_ic_cp_stats *_vdev_ics; \
		if (_vdev_cs) { \
			_vdev_ics = _vdev_cs->vdev_stats; \
			if (_vdev_ics) \
				return  _vdev_ics->ucast_stats.cs_##field; \
		} \
			return 0; \
	}

UCFG_VDEV_UCAST_CP_STATS_GET_FUNCS(rx_decryptok);
UCFG_VDEV_UCAST_CP_STATS_GET_FUNCS(rx_ccmpmic);
UCFG_VDEV_UCAST_CP_STATS_GET_FUNCS(rx_ccmpreplay);
UCFG_VDEV_UCAST_CP_STATS_GET_FUNCS(rx_wepfail);

#define UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(field) \
	static inline void \
	ucfg_vdev_mcast_cp_stats_##field##_inc(struct wlan_objmgr_vdev *_vdev, \
					       uint64_t _val) \
	{ \
		struct vdev_cp_stats *_vdev_cs = \
			wlan_cp_stats_get_vdev_stats_obj(_vdev); \
		if (_vdev_cs) { \
			struct vdev_ic_cp_stats *_vdev_ics = \
						_vdev_cs->vdev_stats; \
			if (_vdev_ics) { \
				_vdev_ics->mcast_stats.cs_##field += _val;\
			} \
		} \
	} \
	static inline void \
	ucfg_vdev_mcast_cp_stats_##field##_dec(struct wlan_objmgr_vdev *_vdev, \
					       uint64_t _val) \
	{ \
		struct vdev_cp_stats *_vdev_cs = \
			wlan_cp_stats_get_vdev_stats_obj(_vdev); \
		if (_vdev_cs) { \
			struct vdev_ic_cp_stats *_vdev_ics = \
						_vdev_cs->vdev_stats; \
			if (_vdev_ics) { \
				_vdev_ics->mcast_stats.cs_##field -= _val;\
			} \
		} \
	} \
	static inline void ucfg_vdev_mcast_cp_stats_##field##_update( \
			struct wlan_objmgr_vdev *_vdev, uint64_t _val) \
	{ \
		struct vdev_cp_stats *_vdev_cs = \
			wlan_cp_stats_get_vdev_stats_obj(_vdev); \
		if (_vdev_cs) { \
			struct vdev_ic_cp_stats *_vdev_ics = \
						_vdev_cs->vdev_stats; \
			if (_vdev_ics) { \
				_vdev_ics->mcast_stats.cs_##field = _val;\
			} \
		} \
	}

UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_badkeyid);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_decryptok);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_wepfail);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_tkipicv);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_tkipreplay);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_tkipformat);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_ccmpmic);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_ccmpreplay);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_ccmpformat);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_wpimic);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_wpireplay);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_countermeasure);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(rx_mgmt);
UCFG_VDEV_MCAST_CP_STATS_SET_FUNCS(tx_mgmt);

#define UCFG_VDEV_MCAST_CP_STATS_GET_FUNCS(field) \
	static inline uint64_t \
	ucfg_vdev_mcast_cp_stats_##field##_get(struct wlan_objmgr_vdev *_vdev) \
	{ \
		struct vdev_cp_stats *_vdev_cs = \
				wlan_cp_stats_get_vdev_stats_obj(_vdev); \
		struct vdev_ic_cp_stats *_vdev_ics; \
		if (_vdev_cs) { \
			_vdev_ics = _vdev_cs->vdev_stats; \
			if (_vdev_ics) \
				return  _vdev_ics->mcast_stats.cs_##field; \
		} \
			return 0; \
	}

UCFG_VDEV_MCAST_CP_STATS_GET_FUNCS(rx_decryptok);
UCFG_VDEV_MCAST_CP_STATS_GET_FUNCS(rx_ccmpmic);
UCFG_VDEV_MCAST_CP_STATS_GET_FUNCS(rx_ccmpreplay);
UCFG_VDEV_MCAST_CP_STATS_GET_FUNCS(rx_wepfail);

#define UCFG_PEER_CP_STATS_SET_FUNCS(field) \
	static inline void \
	ucfg_peer_cp_stats_##field##_inc(struct wlan_objmgr_peer *_peer, \
					 uint32_t _val) \
	{ \
		struct peer_cp_stats *_peer_cs = \
			wlan_cp_stats_get_peer_stats_obj(_peer); \
		if (_peer_cs) { \
			struct peer_ic_cp_stats *_peer_ics = \
						_peer_cs->peer_stats; \
			if (_peer_ics) { \
				_peer_ics->cs_##field += _val;\
			} \
		} \
	} \
	static inline void \
	ucfg_peer_cp_stats_##field##_dec(struct wlan_objmgr_peer *_peer, \
					 uint32_t _val) \
	{ \
		struct peer_cp_stats *_peer_cs = \
			wlan_cp_stats_get_peer_stats_obj(_peer); \
		if (_peer_cs) { \
			struct peer_ic_cp_stats *_peer_ics = \
						_peer_cs->peer_stats; \
			if (_peer_ics) { \
				_peer_ics->cs_##field -= _val;\
			} \
		} \
	} \
	static inline void \
	ucfg_peer_cp_stats_##field##_update(struct wlan_objmgr_peer *_peer, \
					    uint32_t _val) \
	{ \
		struct peer_cp_stats *_peer_cs = \
				wlan_cp_stats_get_peer_stats_obj(_peer); \
		if (_peer_cs) { \
			struct peer_ic_cp_stats *_peer_ics = \
							_peer_cs->peer_stats; \
			if (_peer_ics) { \
				_peer_ics->cs_##field = _val;\
			} \
		} \
	}

UCFG_PEER_CP_STATS_SET_FUNCS(rx_mgmt);
UCFG_PEER_CP_STATS_SET_FUNCS(tx_mgmt);
UCFG_PEER_CP_STATS_SET_FUNCS(rx_mgmt_rate);
UCFG_PEER_CP_STATS_SET_FUNCS(is_tx_not_ok);
UCFG_PEER_CP_STATS_SET_FUNCS(rx_noprivacy);
UCFG_PEER_CP_STATS_SET_FUNCS(rx_wepfail);
UCFG_PEER_CP_STATS_SET_FUNCS(rx_tkipicv);
UCFG_PEER_CP_STATS_SET_FUNCS(rx_wpimic);
UCFG_PEER_CP_STATS_SET_FUNCS(rx_ccmpmic);
UCFG_PEER_CP_STATS_SET_FUNCS(ps_discard);
UCFG_PEER_CP_STATS_SET_FUNCS(psq_drops);
UCFG_PEER_CP_STATS_SET_FUNCS(tx_assoc);
UCFG_PEER_CP_STATS_SET_FUNCS(tx_assoc_fail);
#ifdef ATH_SUPPORT_IQUE
UCFG_PEER_CP_STATS_SET_FUNCS(tx_dropblock);
#endif
#ifdef WLAN_ATH_SUPPORT_EXT_STAT
UCFG_PEER_CP_STATS_SET_FUNCS(tx_bytes_rate);
UCFG_PEER_CP_STATS_SET_FUNCS(rx_bytes_rate);
UCFG_PEER_CP_STATS_SET_FUNCS(tx_data_rate);
UCFG_PEER_CP_STATS_SET_FUNCS(rx_data_rate);
UCFG_PEER_CP_STATS_SET_FUNCS(rx_bytes_last);
UCFG_PEER_CP_STATS_SET_FUNCS(rx_data_last);
UCFG_PEER_CP_STATS_SET_FUNCS(tx_bytes_success_last);
UCFG_PEER_CP_STATS_SET_FUNCS(tx_data_success_last);
#endif

static inline
void ucfg_peer_cp_stats_rx_mgmt_rssi_update(struct wlan_objmgr_peer *peer,
					    int8_t rssi)
{
	struct peer_cp_stats *peer_cs;
	struct peer_ic_cp_stats *peer_cps;

	if (!peer)
		return;

	peer_cs = wlan_cp_stats_get_peer_stats_obj(peer);
	if (!peer_cs)
		return;

	peer_cps = peer_cs->peer_stats;
	if (peer_cps)
		peer_cps->cs_rx_mgmt_rssi = rssi;
}

static inline
int8_t ucfg_peer_cp_stats_rx_mgmt_rssi_get(struct wlan_objmgr_peer *peer)
{
	struct peer_cp_stats *peer_cs;
	struct peer_ic_cp_stats *peer_cps;
	int8_t val = -1;

	if (!peer)
		return val;

	peer_cs = wlan_cp_stats_get_peer_stats_obj(peer);
	if (!peer_cs)
		return val;

	peer_cps = peer_cs->peer_stats;
	if (peer_cps)
		val = peer_cps->cs_rx_mgmt_rssi;

	return val;
}

#define UCFG_PEER_CP_STATS_GET_FUNCS(field) \
	static inline uint32_t \
	ucfg_peer_cp_stats_##field##_get(struct wlan_objmgr_peer *_peer) \
	{ \
		struct peer_cp_stats *_peer_cs = \
				wlan_cp_stats_get_peer_stats_obj(_peer); \
		struct peer_ic_cp_stats *_peer_ics; \
		if (_peer_cs) { \
			_peer_ics = _peer_cs->peer_stats; \
			if (_peer_ics) \
				return  _peer_ics->cs_##field; \
		} \
		return 0; \
	}

UCFG_PEER_CP_STATS_GET_FUNCS(rx_mgmt_rate);
#ifdef ATH_SUPPORT_IQUE
UCFG_PEER_CP_STATS_GET_FUNCS(tx_dropblock);
#endif

/**
 * wlan_ucfg_get_peer_cp_stats() - ucfg API to get peer cp stats
 * @peer_obj: pointer to peer object
 * @peer_cps: pointer to peer cp stats object to populate
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_ucfg_get_peer_cp_stats(struct wlan_objmgr_peer *peer,
				       struct peer_ic_cp_stats *peer_cps);

/**
 * wlan_ucfg_get_vdev_cp_stats() - ucfg API to get vdev cp stats
 * @vdev_obj: pointer to vdev object
 * @vdev_cps: pointer to vdev cp stats object to populate
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_ucfg_get_vdev_cp_stats(struct wlan_objmgr_vdev *vdev,
				       struct vdev_ic_cp_stats *vdev_cps);

/**
 * wlan_ucfg_get_pdev_cp_stats_ref() - API to get reference to pdev cp stats
 * @pdev_obj: pointer to pdev object
 *
 * Return: pdev_ic_cp_stats or NULL
 */
struct pdev_ic_cp_stats
*wlan_ucfg_get_pdev_cp_stats_ref(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_ucfg_get_pdev_cp_stats() - ucfg API to get pdev cp stats
 * @pdev_obj: pointer to pdev object
 * @pdev_cps: pointer to pdev cp stats object to populate
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_ucfg_get_pdev_cp_stats(struct wlan_objmgr_pdev *pdev,
				       struct pdev_ic_cp_stats *pdev_cps);

/**
 * wlan_ucfg_get_pdev_cp_stats() - ucfg API to get pdev hw stats
 * @pdev_obj: pointer to pdev object
 * @hw_stats: pointer to pdev hw cp stats to populate
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_ucfg_get_pdev_hw_cp_stats(struct wlan_objmgr_pdev *pdev,
					  struct pdev_hw_stats *hw_stats);

/**
 * wlan_ucfg_set_pdev_cp_stats() - ucfg API to set pdev hw stats
 * @pdev_obj: pointer to pdev object
 * @hw_stats: pointer to pdev hw cp stats
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_ucfg_set_pdev_hw_cp_stats(struct wlan_objmgr_pdev *pdev,
					  struct pdev_hw_stats *hw_stats);

#ifdef WLAN_ATF_ENABLE
/**
 * wlan_ucfg_get_atf_peer_cp_stats() - ucfg API to get ATF peer cp stats
 * @peer_obj: pointer to peer object
 * @atf_cps: pointer to atf peer cp stats object to populate
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_ucfg_get_atf_peer_cp_stats(struct wlan_objmgr_peer *peer,
					   struct atf_peer_cp_stats *atf_cps);

/**
 * wlan_ucfg_get_atf_peer_cp_stats_from_mac() - ucfg API to get ATF
 * peer cp stats from peer mac address
 * @vdev_obj: pointer to vdev object
 * @mac: pointer to peer mac address
 * @atf_cps: pointer to atf peer cp stats object to populate
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS
wlan_ucfg_get_atf_peer_cp_stats_from_mac(struct wlan_objmgr_vdev *vdev,
					 uint8_t *mac,
					 struct atf_peer_cp_stats *astats);

#endif

/**
 * wlan_ucfg_get_dcs_chan_stats() - ucfg API to set dcs chan stats
 * @pdev_obj: pointer to pdev object
 * @dcs_chan_stats: pointer to dcs chan stats structure
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS
wlan_ucfg_get_dcs_chan_stats(struct wlan_objmgr_pdev *pdev,
			     struct pdev_dcs_chan_stats *dcs_chan_stats);

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_IC_UCFG_API_H__ */
