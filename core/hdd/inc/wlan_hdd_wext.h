/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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

#ifndef __WEXT_IW_H__
#define __WEXT_IW_H__

#include <linux/version.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/wireless.h>
#include <net/iw_handler.h>
#include <linux/timer.h>
#include "qdf_event.h"

struct sap_Config;

/*
 * order of parameters in addTs private ioctl
 */
#define HDD_WLAN_WMM_PARAM_HANDLE                       0
#define HDD_WLAN_WMM_PARAM_TID                          1
#define HDD_WLAN_WMM_PARAM_DIRECTION                    2
#define HDD_WLAN_WMM_PARAM_APSD                         3
#define HDD_WLAN_WMM_PARAM_USER_PRIORITY                4
#define HDD_WLAN_WMM_PARAM_NOMINAL_MSDU_SIZE            5
#define HDD_WLAN_WMM_PARAM_MAXIMUM_MSDU_SIZE            6
#define HDD_WLAN_WMM_PARAM_MINIMUM_DATA_RATE            7
#define HDD_WLAN_WMM_PARAM_MEAN_DATA_RATE               8
#define HDD_WLAN_WMM_PARAM_PEAK_DATA_RATE               9
#define HDD_WLAN_WMM_PARAM_MAX_BURST_SIZE              10
#define HDD_WLAN_WMM_PARAM_MINIMUM_PHY_RATE            11
#define HDD_WLAN_WMM_PARAM_SURPLUS_BANDWIDTH_ALLOWANCE 12
#define HDD_WLAN_WMM_PARAM_SERVICE_INTERVAL            13
#define HDD_WLAN_WMM_PARAM_SUSPENSION_INTERVAL         14
#define HDD_WLAN_WMM_PARAM_BURST_SIZE_DEFN             15
#define HDD_WLAN_WMM_PARAM_ACK_POLICY                  16
#define HDD_WLAN_WMM_PARAM_INACTIVITY_INTERVAL         17
#define HDD_WLAN_WMM_PARAM_MAX_SERVICE_INTERVAL        18
#define HDD_WLAN_WMM_PARAM_COUNT                       19

#define MHZ 6

#define WE_MAX_STR_LEN                                 IW_PRIV_SIZE_MASK
#define WLAN_HDD_UI_BAND_AUTO                          0
#define WLAN_HDD_UI_BAND_5_GHZ                         1
#define WLAN_HDD_UI_BAND_2_4_GHZ                       2

enum hdd_wlan_wmm_direction {
	HDD_WLAN_WMM_DIRECTION_UPSTREAM = 0,
	HDD_WLAN_WMM_DIRECTION_DOWNSTREAM = 1,
	HDD_WLAN_WMM_DIRECTION_BIDIRECTIONAL = 2,
};

enum hdd_wlan_wmm_power_save {
	HDD_WLAN_WMM_POWER_SAVE_LEGACY = 0,
	HDD_WLAN_WMM_POWER_SAVE_UAPSD = 1,
};

typedef enum {
	/* TSPEC/re-assoc done, async */
	HDD_WLAN_WMM_STATUS_SETUP_SUCCESS = 0,
	/* no need to setup TSPEC since ACM=0 and no UAPSD desired,
	 * sync + async
	 */
	HDD_WLAN_WMM_STATUS_SETUP_SUCCESS_NO_ACM_NO_UAPSD = 1,
	/* no need to setup TSPEC since ACM=0 and UAPSD already exists,
	 * sync + async
	 */
	HDD_WLAN_WMM_STATUS_SETUP_SUCCESS_NO_ACM_UAPSD_EXISTING = 2,
	/* TSPEC result pending, sync */
	HDD_WLAN_WMM_STATUS_SETUP_PENDING = 3,
	/* TSPEC/re-assoc failed, sync + async */
	HDD_WLAN_WMM_STATUS_SETUP_FAILED = 4,
	/* Request rejected due to invalid params, sync + async */
	HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM = 5,
	/* TSPEC request rejected since AP!=QAP, sync */
	HDD_WLAN_WMM_STATUS_SETUP_FAILED_NO_WMM = 6,

	/* TSPEC modification/re-assoc successful, async */
	HDD_WLAN_WMM_STATUS_MODIFY_SUCCESS = 7,
	/* TSPEC modification a no-op since ACM=0 and
	 * no change in UAPSD, sync + async
	 */
	HDD_WLAN_WMM_STATUS_MODIFY_SUCCESS_NO_ACM_NO_UAPSD = 8,
	/* TSPEC modification a no-op since ACM=0 and
	 * requested U-APSD already exists, sync + async
	 */
	HDD_WLAN_WMM_STATUS_MODIFY_SUCCESS_NO_ACM_UAPSD_EXISTING = 9,
	/* TSPEC result pending, sync */
	HDD_WLAN_WMM_STATUS_MODIFY_PENDING = 10,
	/* TSPEC modification failed, prev TSPEC in effect, sync + async */
	HDD_WLAN_WMM_STATUS_MODIFY_FAILED = 11,
	/* TSPEC modification request rejected due to invalid params,
	 * sync + async
	 */
	HDD_WLAN_WMM_STATUS_MODIFY_FAILED_BAD_PARAM = 12,

	/* TSPEC release successful, sync and also async */
	HDD_WLAN_WMM_STATUS_RELEASE_SUCCESS = 13,
	/* TSPEC release pending, sync */
	HDD_WLAN_WMM_STATUS_RELEASE_PENDING = 14,
	/* TSPEC release failed, sync + async */
	HDD_WLAN_WMM_STATUS_RELEASE_FAILED = 15,
	/* TSPEC release rejected due to invalid params, sync */
	HDD_WLAN_WMM_STATUS_RELEASE_FAILED_BAD_PARAM = 16,
	/* TSPEC modified due to the mux'ing of requests on ACs, async */

	HDD_WLAN_WMM_STATUS_MODIFIED = 17,
	/* TSPEC revoked by AP, async */
	HDD_WLAN_WMM_STATUS_LOST = 18,
	/* some internal failure like memory allocation failure, etc, sync */
	HDD_WLAN_WMM_STATUS_INTERNAL_FAILURE = 19,

	/* U-APSD failed during setup but OTA setup (whether TSPEC exchnage or
	 * re-assoc) was done so app should release this QoS, async
	 */
	HDD_WLAN_WMM_STATUS_SETUP_UAPSD_SET_FAILED = 20,
	/* U-APSD failed during modify, but OTA setup (whether TSPEC exchnage or
	 * re-assoc) was done so app should release this QoS, async
	 */
	HDD_WLAN_WMM_STATUS_MODIFY_UAPSD_SET_FAILED = 21
} hdd_wlan_wmm_status_e;

/** TS Info Ack Policy */
enum hdd_wlan_wmm_ts_info_ack_policy {
	HDD_WLAN_WMM_TS_INFO_ACK_POLICY_NORMAL_ACK = 0,
	HDD_WLAN_WMM_TS_INFO_ACK_POLICY_HT_IMMEDIATE_BLOCK_ACK = 1,
};

/** Maximum Length of WPA/RSN IE */
#define MAX_WPA_RSN_IE_LEN 40

/** Enable 11d */
#define ENABLE_11D  1

/** Disable 11d */
#define DISABLE_11D 0

/*
 * refer wpa.h in wpa supplicant code for REASON_MICHAEL_MIC_FAILURE
 *
 * supplicant sets REASON_MICHAEL_MIC_FAILURE as the reason code when it
 * sends the MLME deauth IOCTL for TKIP counter measures
 */
#define HDD_REASON_MICHAEL_MIC_FAILURE 14

/*
 * These are for TLV fields in WPS IE
 */
#define HDD_WPS_UUID_LEN                    16
#define HDD_WPS_ELEM_VERSION                0x104a
#define HDD_WPS_ELEM_REQUEST_TYPE           0x103a
#define HDD_WPS_ELEM_CONFIG_METHODS         0x1008
#define HDD_WPS_ELEM_UUID_E                 0x1047
#define HDD_WPS_ELEM_PRIMARY_DEVICE_TYPE    0x1054
#define HDD_WPS_ELEM_RF_BANDS               0x103c
#define HDD_WPS_ELEM_ASSOCIATION_STATE      0x1002
#define HDD_WPS_ELEM_CONFIGURATION_ERROR    0x1009
#define HDD_WPS_ELEM_DEVICE_PASSWORD_ID     0x1012

#define HDD_WPA_ELEM_VENDOR_EXTENSION       0x1049

#define HDD_WPS_MANUFACTURER_LEN            64
#define HDD_WPS_MODEL_NAME_LEN              32
#define HDD_WPS_MODEL_NUM_LEN               32
#define HDD_WPS_SERIAL_NUM_LEN              32
#define HDD_WPS_DEVICE_OUI_LEN               4
#define HDD_WPS_DEVICE_NAME_LEN             32

#define HDD_WPS_ELEM_WPS_STATE              0x1044
#define HDD_WPS_ELEM_APSETUPLOCK            0x1057
#define HDD_WPS_ELEM_SELECTEDREGISTRA       0x1041
#define HDD_WPS_ELEM_RSP_TYPE               0x103B
#define HDD_WPS_ELEM_MANUFACTURER           0x1021
#define HDD_WPS_ELEM_MODEL_NAME             0x1023
#define HDD_WPS_ELEM_MODEL_NUM              0x1024
#define HDD_WPS_ELEM_SERIAL_NUM             0x1042
#define HDD_WPS_ELEM_DEVICE_NAME            0x1011
#define HDD_WPS_ELEM_REGISTRA_CONF_METHODS  0x1053

#define HDD_RTSCTS_EN_MASK                  0xF
#define HDD_RTSCTS_ENABLE                   1
#define HDD_CTS_ENABLE                      2

#define WPS_OUI_TYPE   "\x00\x50\xf2\x04"
#define WPS_OUI_TYPE_SIZE  4

#define SS_OUI_TYPE    "\x00\x16\x32"
#define SS_OUI_TYPE_SIZE   3

#define P2P_OUI_TYPE   "\x50\x6f\x9a\x09"
#define P2P_OUI_TYPE_SIZE  4

#define HS20_OUI_TYPE   "\x50\x6f\x9a\x10"
#define HS20_OUI_TYPE_SIZE  4

#define OSEN_OUI_TYPE   "\x50\x6f\x9a\x12"
#define OSEN_OUI_TYPE_SIZE  4

#ifdef WLAN_FEATURE_WFD
#define WFD_OUI_TYPE   "\x50\x6f\x9a\x0a"
#define WFD_OUI_TYPE_SIZE  4
#endif

#define MBO_OUI_TYPE   "\x50\x6f\x9a\x16"
#define MBO_OUI_TYPE_SIZE  4

#define QCN_OUI_TYPE   "\x8c\xfd\xf0\x01"
#define QCN_OUI_TYPE_SIZE  4

enum hdd_wps_mode {
	eWEXT_WPS_OFF = 0,
	eWEXT_WPS_ON = 1,
};

/*
 * This structure contains the interface level (granularity)
 * configuration information in support of wireless extensions.
 */
typedef struct hdd_wext_state_s {
	/** The CSR "desired" Profile */
	tCsrRoamProfile roamProfile;

	/** BSSID to which connect request is received */
	struct qdf_mac_addr req_bssId;

	/** The association status code */
	uint32_t statusCode;

	/** wpa version WPA/WPA2/None*/
	int32_t wpaVersion;

	/**WPA or RSN IE*/
	uint8_t WPARSNIE[MAX_WPA_RSN_IE_LEN];

	/**gen IE */
	tSirAddie genIE;

	/**Additional IE for assoc */
	tSirAddie assocAddIE;

	/**auth key mgmt */
	int32_t authKeyMgmt;

	/* qdf event */
	qdf_event_t hdd_qdf_event;

	/**Counter measure state, Started/Stopped*/
	bool mTKIPCounterMeasures;

#ifdef FEATURE_WLAN_ESE
	/* ESE state variables */
	bool isESEConnection;
#endif
} hdd_wext_state_t;

struct ccp_freq_chan_map {
	/* List of frequencies */
	uint32_t freq;
	uint32_t chan;
};

/* Packet Types. */
#define WLAN_KEEP_ALIVE_UNSOLICIT_ARP_RSP     2
#define WLAN_KEEP_ALIVE_NULL_PKT              1

#define wlan_hdd_get_wps_ie_ptr(ie, ie_len) \
	wlan_hdd_get_vendor_oui_ie_ptr(WPS_OUI_TYPE, WPS_OUI_TYPE_SIZE, \
	ie, ie_len)

#define wlan_hdd_get_p2p_ie_ptr(ie, ie_len) \
	wlan_hdd_get_vendor_oui_ie_ptr(P2P_OUI_TYPE, P2P_OUI_TYPE_SIZE, \
	ie, ie_len)

#ifdef WLAN_FEATURE_WFD
#define wlan_hdd_get_wfd_ie_ptr(ie, ie_len) \
	wlan_hdd_get_vendor_oui_ie_ptr(WFD_OUI_TYPE, WFD_OUI_TYPE_SIZE, \
	ie, ie_len)
#endif

#define wlan_hdd_get_mbo_ie_ptr(ie, ie_len) \
	wlan_hdd_get_vendor_oui_ie_ptr(MBO_OUI_TYPE, MBO_OUI_TYPE_SIZE, \
	ie, ie_len)
/*
 * Defines for fw_test command
 */
#define HDD_FWTEST_PARAMS 3
#define HDD_FWTEST_SU_PARAM_ID 53
#define HDD_FWTEST_MU_PARAM_ID 2
#define HDD_FWTEST_SU_DEFAULT_VALUE 100
#define HDD_FWTEST_MU_DEFAULT_VALUE 40
#define HDD_FWTEST_MAX_VALUE 500

extern int hdd_unregister_wext(struct net_device *dev);
extern int hdd_register_wext(struct net_device *dev);
extern int hdd_wlan_get_freq(uint32_t chan, uint32_t *freq);
extern void hdd_display_stats_help(void);
extern void hdd_wlan_get_version(hdd_context_t *hdd_ctx,
				 union iwreq_data *wrqu, char *extra);

extern void hdd_wlan_get_stats(hdd_adapter_t *pAdapter, uint16_t *length,
			       char *buffer, uint16_t buf_len);
extern void hdd_wlan_list_fw_profile(uint16_t *length,
			       char *buffer, uint16_t buf_len);

int iw_get_rts_threshold(struct net_device *dev,
			 struct iw_request_info *info,
			 union iwreq_data *wrqu, char *extra);

int iw_get_frag_threshold(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra);

extern int iw_set_essid(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra);

extern int iw_get_essid(struct net_device *dev,
			struct iw_request_info *info,
			struct iw_point *dwrq, char *extra);

extern int iw_set_ap_address(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra);

extern int iw_get_ap_address(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra);

extern int iw_set_auth(struct net_device *dev, struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra);

extern int iw_get_auth(struct net_device *dev, struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra);

extern int iw_set_var_ints_getnone(struct net_device *dev,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu, char *extra);

extern int iw_set_three_ints_getnone(struct net_device *dev,
				     struct iw_request_info *info,
				     union iwreq_data *wrqu, char *extra);

extern int hdd_priv_get_data(struct iw_point *p_priv_data,
			     union iwreq_data *wrqu);

extern void *mem_alloc_copy_from_user_helper(const void *wrqu_data, size_t len);

/**
 * wlan_hdd_get_linkspeed_for_peermac() - Get link speed for a peer
 * @adapter: adapter upon which the peer is active
 * @mac_address: MAC address of the peer
 * @linkspeed: pointer to memory where returned link speed is to be placed
 *
 * This function will send a query to SME for the linkspeed of the
 * given peer, and then wait for the callback to be invoked.
 *
 * Return: 0 if linkspeed data is available, negative errno otherwise
 */
int wlan_hdd_get_linkspeed_for_peermac(hdd_adapter_t *adapter,
				       struct qdf_mac_addr *mac_address,
				       uint32_t *linkspeed);
void hdd_clear_roam_profile_ie(hdd_adapter_t *pAdapter);

uint8_t *wlan_hdd_get_vendor_oui_ie_ptr(uint8_t *oui, uint8_t oui_size,
					uint8_t *ie, int ie_len);

QDF_STATUS wlan_hdd_get_class_astats(hdd_adapter_t *pAdapter);

QDF_STATUS wlan_hdd_get_station_stats(hdd_adapter_t *pAdapter);

QDF_STATUS wlan_hdd_get_rssi(hdd_adapter_t *pAdapter, int8_t *rssi_value);

QDF_STATUS wlan_hdd_get_snr(hdd_adapter_t *pAdapter, int8_t *snr);

int hdd_get_ldpc(hdd_adapter_t *adapter, int *value);
int hdd_set_ldpc(hdd_adapter_t *adapter, int value);
int hdd_get_tx_stbc(hdd_adapter_t *adapter, int *value);
int hdd_set_tx_stbc(hdd_adapter_t *adapter, int value);
int hdd_get_rx_stbc(hdd_adapter_t *adapter, int *value);
int hdd_set_rx_stbc(hdd_adapter_t *adapter, int value);

/**
 * hdd_assemble_rate_code() - assemble rate code to be sent to FW
 * @preamble: rate preamble
 * @nss: number of streams
 * @rate: rate index
 *
 * Rate code assembling is different for targets which are 11ax capable.
 * Check for the target support and assemble the rate code accordingly.
 *
 * Return: assembled rate code
 */
int hdd_assemble_rate_code(uint8_t preamble, uint8_t nss, uint8_t rate);

/**
 * hdd_set_11ax_rate() - set 11ax rate
 * @adapter: adapter being modified
 * @value: new 11ax rate code
 * @sap_config: pointer to SAP config to check HW mode
 *              this will be NULL for call from STA persona
 *
 * Return: 0 on success, negative errno on failure
 */
int hdd_set_11ax_rate(hdd_adapter_t *adapter, int value,
		      struct sap_Config *sap_config);

/**
 * hdd_set_peer_rate() - set peer rate
 * @adapter: adapter being modified
 * @value: rate code with AID
 *
 * Return: 0 on success, negative errno on failure
 */
int hdd_set_peer_rate(hdd_adapter_t *adapter, int value);

void wlan_hdd_change_country_code_callback(void *pAdapter);

int wlan_hdd_update_phymode(struct net_device *net, tHalHandle hal,
			    int new_phymode, hdd_context_t *phddctx);

int wlan_hdd_get_temperature(hdd_adapter_t *pAdapter, int *temperature);
int wlan_hdd_get_link_speed(hdd_adapter_t *sta_adapter, uint32_t *link_speed);

struct iw_request_info;
/**
 * hdd_check_standard_wext_control() - Check to see if standard
 *      wireless extensions ioctls are allowed
 * @hdd_ctx: Global HDD context
 * @info: Wireless extensions ioctl information passed by the kernel
 *
 * This function will examine the "standard_wext_control" configuration
 * item to determine whether or not standard wireless extensions ioctls
 * are allowed.
 *
 * Return: 0 if the ioctl is allowed to be processed, -ENOTSUPP if the
 * ioctls have been disabled. Note that in addition to returning
 * status, this function will log a message if the ioctls are disabled
 * or deprecated.
 */
int hdd_check_standard_wext_control(struct hdd_context_s *hdd_ctx,
				    struct iw_request_info *info);

/**
 * hdd_check_private_wext_control() - Check to see if private
 *      wireless extensions ioctls are allowed
 * @hdd_ctx: Global HDD context
 * @info: Wireless extensions ioctl information passed by the kernel
 *
 * This function will examine the "private_wext_control" configuration
 * item to determine whether or not private wireless extensions ioctls
 * are allowed.
 *
 * Return: 0 if the ioctl is allowed to be processed, -ENOTSUPP if the
 * ioctls have been disabled. Note that in addition to returning
 * status, this function will log a message if the ioctls are disabled
 * or deprecated.
 */
int hdd_check_private_wext_control(struct hdd_context_s *hdd_ctx,
				   struct iw_request_info *info);

#endif /* __WEXT_IW_H__ */
