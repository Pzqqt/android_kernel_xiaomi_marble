/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#if !defined(WLAN_HDD_HOSTAPD_H)
#define WLAN_HDD_HOSTAPD_H

/**
 * DOC: wlan_hdd_hostapd.h
 *
 * WLAN Host Device driver hostapd header file
 */

/* Include files */

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <qdf_list.h>
#include <qdf_types.h>
#include <wlan_hdd_main.h>

/* Preprocessor definitions and constants */

/* max length of command string in hostapd ioctl */
#define HOSTAPD_IOCTL_COMMAND_STRLEN_MAX   8192

hdd_adapter_t *hdd_wlan_create_ap_dev(hdd_context_t *pHddCtx,
				      tSirMacAddr macAddr,
				      unsigned char name_assign_type,
				      uint8_t *name);

QDF_STATUS hdd_register_hostapd(hdd_adapter_t *pAdapter, uint8_t rtnl_held);

QDF_STATUS hdd_unregister_hostapd(hdd_adapter_t *pAdapter, bool rtnl_held);

eCsrAuthType
hdd_translate_rsn_to_csr_auth_type(uint8_t auth_suite[4]);

int hdd_softap_set_channel_change(struct net_device *dev,
					int target_channel,
					enum phy_ch_width target_bw);

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
void hdd_sap_restart_with_channel_switch(hdd_adapter_t *adapter,
				uint32_t target_channel,
				uint32_t target_bw);
#endif

eCsrEncryptionType
hdd_translate_rsn_to_csr_encryption_type(uint8_t cipher_suite[4]);

eCsrEncryptionType
hdd_translate_rsn_to_csr_encryption_type(uint8_t cipher_suite[4]);

eCsrAuthType
hdd_translate_wpa_to_csr_auth_type(uint8_t auth_suite[4]);

eCsrEncryptionType
hdd_translate_wpa_to_csr_encryption_type(uint8_t cipher_suite[4]);

QDF_STATUS hdd_softap_sta_deauth(hdd_adapter_t *,
		struct tagCsrDelStaParams *);
void hdd_softap_sta_disassoc(hdd_adapter_t *, struct tagCsrDelStaParams *);
void hdd_softap_tkip_mic_fail_counter_measure(hdd_adapter_t *, bool);
int hdd_softap_unpack_ie(tHalHandle halHandle,
			 eCsrEncryptionType *pEncryptType,
			 eCsrEncryptionType *mcEncryptType,
			 eCsrAuthType *pAuthType,
			 bool *pMFPCapable,
			 bool *pMFPRequired,
			 uint16_t gen_ie_len, uint8_t *gen_ie);

QDF_STATUS hdd_hostapd_sap_event_cb(tpSap_Event pSapEvent,
				    void *usrDataForCallback);
QDF_STATUS hdd_init_ap_mode(hdd_adapter_t *pAdapter);
void hdd_set_ap_ops(struct net_device *pWlanHostapdDev);
int hdd_hostapd_stop(struct net_device *dev);
int hdd_sap_context_init(hdd_context_t *hdd_ctx);
void hdd_sap_context_destroy(hdd_context_t *hdd_ctx);
#ifdef FEATURE_WLAN_FORCE_SAP_SCC
void hdd_restart_softap(hdd_context_t *pHddCtx, hdd_adapter_t *pAdapter);
#endif /* FEATURE_WLAN_FORCE_SAP_SCC */
#ifdef QCA_HT_2040_COEX
QDF_STATUS hdd_set_sap_ht2040_mode(hdd_adapter_t *pHostapdAdapter,
				   uint8_t channel_type);
#endif


int wlan_hdd_cfg80211_stop_ap(struct wiphy *wiphy,
			      struct net_device *dev);

int wlan_hdd_cfg80211_start_ap(struct wiphy *wiphy,
			       struct net_device *dev,
			       struct cfg80211_ap_settings *params);

int wlan_hdd_cfg80211_change_beacon(struct wiphy *wiphy,
				    struct net_device *dev,
				    struct cfg80211_beacon_data *params);

QDF_STATUS wlan_hdd_config_acs(hdd_context_t *hdd_ctx, hdd_adapter_t *adapter);
#endif /* end #if !defined(WLAN_HDD_HOSTAPD_H) */
