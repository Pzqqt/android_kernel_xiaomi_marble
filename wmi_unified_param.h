/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

/*
 * This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI).
 */

#ifndef _WMI_UNIFIED_PARAM_H_
#define _WMI_UNIFIED_PARAM_H_

#include <wlan_scan_public_structs.h>
#ifdef CONVERGED_TDLS_ENABLE
#include <wlan_tdls_public_structs.h>
#endif

#define MAC_MAX_KEY_LENGTH 32
#define MAC_PN_LENGTH 8
#define MAX_MAC_HEADER_LEN 32
#define MIN_MAC_HEADER_LEN 24
#define QOS_CONTROL_LEN 2

#define IEEE80211_ADDR_LEN  6  /* size of 802.11 address */
#define WMI_MAC_MAX_SSID_LENGTH              32
#define WMI_SCAN_MAX_NUM_SSID                0x0A
#define mgmt_tx_dl_frm_len 64
#define WMI_SMPS_MASK_LOWER_16BITS 0xFF
#define WMI_SMPS_MASK_UPPER_3BITS 0x7
#define WMI_SMPS_PARAM_VALUE_S 29
#define WMI_UNIT_TEST_MAX_NUM_ARGS 100
/* The size of the utc time in bytes. */
#define WMI_SIZE_UTC_TIME (10)
/* The size of the utc time error in bytes. */
#define WMI_SIZE_UTC_TIME_ERROR (5)
#define WMI_MCC_MIN_CHANNEL_QUOTA             20
#define WMI_MCC_MAX_CHANNEL_QUOTA             80
#define WMI_MCC_MIN_NON_ZERO_CHANNEL_LATENCY  30
#define WMI_BEACON_TX_BUFFER_SIZE             (512)
#define WMI_WIFI_SCANNING_MAC_OUI_LENGTH      3
#define WMI_EXTSCAN_MAX_SIGNIFICANT_CHANGE_APS   64
#define WMI_RSSI_THOLD_DEFAULT   -300
#define WMI_NLO_FREQ_THRESH          1000
#define WMI_SEC_TO_MSEC(sec)         (sec * 1000)
#define WMI_MSEC_TO_USEC(msec)	     (msec * 1000)
#define WMI_ETH_LEN      64
#define WMI_QOS_NUM_TSPEC_MAX 2
#define WMI_QOS_NUM_AC_MAX 4
#define WMI_IPV4_ADDR_LEN       4
#define WMI_KEEP_ALIVE_NULL_PKT              1
#define WMI_KEEP_ALIVE_UNSOLICIT_ARP_RSP     2
#define WMI_MAC_MAX_KEY_LENGTH          32
#define WMI_KRK_KEY_LEN     16
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
#define WMI_BTK_KEY_LEN     32
#define WMI_ROAM_R0KH_ID_MAX_LEN    48
#define WMI_ROAM_SCAN_PSK_SIZE    32
#endif
#define WMI_NOISE_FLOOR_DBM_DEFAULT      (-96)
#define WMI_EXTSCAN_MAX_HOTLIST_SSIDS                    8
#define WMI_ROAM_MAX_CHANNELS                            80
#ifdef FEATURE_WLAN_EXTSCAN
#define WMI_MAX_EXTSCAN_MSG_SIZE        1536
#define WMI_EXTSCAN_REST_TIME           100
#define WMI_EXTSCAN_MAX_SCAN_TIME       50000
#define WMI_EXTSCAN_BURST_DURATION      150
#endif
#define WMI_SCAN_NPROBES_DEFAULT            (2)
#define WMI_SEC_TO_MSEC(sec)         (sec * 1000)  /* sec to msec */
#define WMI_MSEC_TO_USEC(msec)       (msec * 1000) /* msec to usec */
#define WMI_NLO_FREQ_THRESH          1000       /* in MHz */

#define WMI_SVC_MSG_MAX_SIZE   1536
#define MAX_UTF_EVENT_LENGTH	2048
#define MAX_WMI_UTF_LEN	252
#define MAX_WMI_QVIT_LEN	252
#define THERMAL_LEVELS	4
#define WMI_HOST_BCN_FLT_MAX_SUPPORTED_IES	256
#define WMI_HOST_BCN_FLT_MAX_ELEMS_IE_LIST \
			(WMI_HOST_BCN_FLT_MAX_SUPPORTED_IES/32)
#define LTEU_MAX_BINS	10
#define ATF_ACTIVED_MAX_CLIENTS   50
#define ATF_ACTIVED_MAX_ATFGROUPS 8
#define CTL_5G_SIZE 1536
#define CTL_2G_SIZE 684
#define MAX_CTL_SIZE (CTL_5G_SIZE > CTL_2G_SIZE ? CTL_5G_SIZE : CTL_2G_SIZE)
#define IEEE80211_MICBUF_SIZE   (8+8)
#define IEEE80211_TID_SIZE	17
#define WME_NUM_AC	4
#define SMART_ANT_MODE_SERIAL   0
#define SMART_ANT_MODE_PARALLEL   1
#define IEEE80211_WEP_NKID	4   /* number of key ids */
#define WPI_IV_LEN	16
#define WMI_SCAN_MAX_NUM_BSSID	10
#define MAX_CHANS	1023
#define TARGET_OEM_CONFIGURE_LCI	0x0A
#define RTT_LCI_ALTITUDE_MASK	0x3FFFFFFF
#define TARGET_OEM_CONFIGURE_LCR	0x09
#define RTT_TIMEOUT_MS 180
#define MAX_SUPPORTED_RATES 128
#define WMI_HOST_MAX_BUFFER_SIZE	1712
#define WMI_HAL_MAX_SANTENNA 4
#define WMI_HOST_PDEV_VI_PRIORITY_BIT     (1<<2)
#define WMI_HOST_PDEV_BEACON_PRIORITY_BIT (1<<4)
#define WMI_HOST_PDEV_MGMT_PRIORITY_BIT   (1<<5)

#define FIPS_ALIGN 4
#define FIPS_ALIGNTO(__addr, __to) \
		((((unsigned long int)(__addr)) + (__to) -  1) & ~((__to) - 1))
#define FIPS_IS_ALIGNED(__addr, __to) \
		(!(((unsigned long int)(__addr)) & ((__to)-1)))

#define WMI_HOST_MAX_SERIAL_ANTENNA 2
#define WMI_SMART_ANT_MAX_RATE_SERIES 2

#define WMI_HOST_F_MS(_v, _f)	\
	(((_v) & (_f)) >> (_f##_S))

#define WMI_HOST_F_RMW(_var, _v, _f)	\
	do {	\
		(_var) &= ~(_f);	\
		(_var) |= (((_v) << (_f##_S)) & (_f));	\
	} while (0)

/* vdev capabilities bit mask */
#define WMI_HOST_VDEV_BEACON_SUPPORT  0x1
#define WMI_HOST_VDEV_WDS_LRN_ENABLED 0x2
#define WMI_HOST_VDEV_VOW_ENABLED     0x4
#define WMI_HOST_VDEV_IS_BEACON_SUPPORTED(param) \
	((param) & WMI_HOST_VDEV_BEACON_SUPPORT)
#define WMI_HOST_VDEV_IS_WDS_LRN_ENABLED(param) \
	((param) & WMI_HOST_VDEV_WDS_LRN_ENABLED)
#define WMI_HOST_VDEV_IS_VOW_ENABLED(param) \
	((param) & WMI_HOST_VDEV_VOW_ENABLED)

/* TXBF capabilities masks */
#define WMI_HOST_TXBF_CONF_SU_TX_BFEE_S 0
#define WMI_HOST_TXBF_CONF_SU_TX_BFEE_M 0x1
#define WMI_HOST_TXBF_CONF_SU_TX_BFEE \
	(WMI_HOST_TXBF_CONF_SU_TX_BFEE_M << WMI_HOST_TXBF_CONF_SU_TX_BFEE_S)
#define WMI_HOST_TXBF_CONF_SU_TX_BFEE_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_TXBF_CONF_SU_TX_BFEE)
#define WMI_HOST_TXBF_CONF_SU_TX_BFEE_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_TXBF_CONF_SU_TX_BFEE)


#define WMI_HOST_TXBF_CONF_MU_TX_BFEE_S 1
#define WMI_HOST_TXBF_CONF_MU_TX_BFEE_M 0x1
#define WMI_HOST_TXBF_CONF_MU_TX_BFEE \
	(WMI_HOST_TXBF_CONF_MU_TX_BFEE_M << WMI_HOST_TXBF_CONF_MU_TX_BFEE_S)
#define WMI_HOST_TXBF_CONF_MU_TX_BFEE_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_TXBF_CONF_MU_TX_BFEE)
#define WMI_HOST_TXBF_CONF_MU_TX_BFEE_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_TXBF_CONF_MU_TX_BFEE)

#define WMI_HOST_TXBF_CONF_SU_TX_BFER_S 2
#define WMI_HOST_TXBF_CONF_SU_TX_BFER_M 0x1
#define WMI_HOST_TXBF_CONF_SU_TX_BFER \
	(WMI_HOST_TXBF_CONF_SU_TX_BFER_M << WMI_HOST_TXBF_CONF_SU_TX_BFER_S)
#define WMI_HOST_TXBF_CONF_SU_TX_BFER_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_TXBF_CONF_SU_TX_BFER)
#define WMI_HOST_TXBF_CONF_SU_TX_BFER_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_TXBF_CONF_SU_TX_BFER)

#define WMI_HOST_TXBF_CONF_MU_TX_BFER_S 3
#define WMI_HOST_TXBF_CONF_MU_TX_BFER_M 0x1
#define WMI_HOST_TXBF_CONF_MU_TX_BFER \
	(WMI_HOST_TXBF_CONF_MU_TX_BFER_M << WMI_HOST_TXBF_CONF_MU_TX_BFER_S)
#define WMI_HOST_TXBF_CONF_MU_TX_BFER_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_TXBF_CONF_MU_TX_BFER)
#define WMI_HOST_TXBF_CONF_MU_TX_BFER_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_TXBF_CONF_MU_TX_BFER)

#define WMI_HOST_TXBF_CONF_STS_CAP_S 4
#define WMI_HOST_TXBF_CONF_STS_CAP_M 0x7
#define WMI_HOST_TXBF_CONF_STS_CAP \
	(WMI_HOST_TXBF_CONF_STS_CAP_M << WMI_HOST_TXBF_CONF_STS_CAP_S)
#define WMI_HOST_TXBF_CONF_STS_CAP_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_TXBF_CONF_STS_CAP);
#define WMI_HOST_TXBF_CONF_STS_CAP_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_TXBF_CONF_STS_CAP)

#define WMI_HOST_TXBF_CONF_IMPLICIT_BF_S 7
#define WMI_HOST_TXBF_CONF_IMPLICIT_BF_M 0x1
#define WMI_HOST_TXBF_CONF_IMPLICIT_BF \
	(WMI_HOST_TXBF_CONF_IMPLICIT_BF_M << WMI_HOST_TXBF_CONF_IMPLICIT_BF_S)
#define WMI_HOST_TXBF_CONF_IMPLICIT_BF_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_TXBF_CONF_IMPLICIT_BF)
#define WMI_HOST_TXBF_CONF_IMPLICIT_BF_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_TXBF_CONF_IMPLICIT_BF)

#define WMI_HOST_TXBF_CONF_BF_SND_DIM_S 8
#define WMI_HOST_TXBF_CONF_BF_SND_DIM_M 0x7
#define WMI_HOST_TXBF_CONF_BF_SND_DIM \
	(WMI_HOST_TXBF_CONF_BF_SND_DIM_M << WMI_HOST_TXBF_CONF_BF_SND_DIM_S)
#define WMI_HOST_TXBF_CONF_BF_SND_DIM_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_TXBF_CONF_BF_SND_DIM)
#define WMI_HOST_TXBF_CONF_BF_SND_DIM_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_TXBF_CONF_BF_SND_DIM)

/* HE BF capabilities mask */
#define WMI_HOST_HE_BF_CONF_SU_BFEE_S 0
#define WMI_HOST_HE_BF_CONF_SU_BFEE_M 0x1
#define WMI_HOST_HE_BF_CONF_SU_BFEE \
	(WMI_HOST_HE_BF_CONF_SU_BFEE_M << WMI_HOST_HE_BF_CONF_SU_BFEE_S)
#define WMI_HOST_HE_BF_CONF_SU_BFEE_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_HE_BF_CONF_SU_BFEE)
#define WMI_HOST_HE_BF_CONF_SU_BFEE_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_HE_BF_CONF_SU_BFEE)

#define WMI_HOST_HE_BF_CONF_SU_BFER_S 1
#define WMI_HOST_HE_BF_CONF_SU_BFER_M 0x1
#define WMI_HOST_HE_BF_CONF_SU_BFER \
	(WMI_HOST_HE_BF_CONF_SU_BFER_M << WMI_HOST_HE_BF_CONF_SU_BFER_S)
#define WMI_HOST_HE_BF_CONF_SU_BFER_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_HE_BF_CONF_SU_BFER)
#define WMI_HOST_HE_BF_CONF_SU_BFER_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_HE_BF_CONF_SU_BFER)

#define WMI_HOST_HE_BF_CONF_MU_BFEE_S 2
#define WMI_HOST_HE_BF_CONF_MU_BFEE_M 0x1
#define WMI_HOST_HE_BF_CONF_MU_BFEE \
	(WMI_HOST_HE_BF_CONF_MU_BFEE_M << WMI_HOST_HE_BF_CONF_MU_BFEE_S)
#define WMI_HOST_HE_BF_CONF_MU_BFEE_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_HE_BF_CONF_MU_BFEE)
#define WMI_HOST_HE_BF_CONF_MU_BFEE_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_HE_BF_CONF_MU_BFEE)

#define WMI_HOST_HE_BF_CONF_MU_BFER_S 3
#define WMI_HOST_HE_BF_CONF_MU_BFER_M 0x1
#define WMI_HOST_HE_BF_CONF_MU_BFER \
	(WMI_HOST_HE_BF_CONF_MU_BFER_M << WMI_HOST_HE_BF_CONF_MU_BFER_S)
#define WMI_HOST_HE_BF_CONF_MU_BFER_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_HE_BF_CONF_MU_BFER)
#define WMI_HOST_HE_BF_CONF_MU_BFER_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_HE_BF_CONF_MU_BFER)

#define WMI_HOST_HE_BF_CONF_DL_OFDMA_S 4
#define WMI_HOST_HE_BF_CONF_DL_OFDMA_M 0x1
#define WMI_HOST_HE_BF_CONF_DL_OFDMA \
	(WMI_HOST_HE_BF_CONF_DL_OFDMA_M << WMI_HOST_HE_BF_CONF_DL_OFDMA_S)
#define WMI_HOST_HE_BF_CONF_DL_OFDMA_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_HE_BF_CONF_DL_OFDMA)
#define WMI_HOST_HE_BF_CONF_DL_OFDMA_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_HE_BF_CONF_DL_OFDMA)

#define WMI_HOST_HE_BF_CONF_UL_OFDMA_S 5
#define WMI_HOST_HE_BF_CONF_UL_OFDMA_M 0x1
#define WMI_HOST_HE_BF_CONF_UL_OFDMA \
	(WMI_HOST_HE_BF_CONF_UL_OFDMA_M << WMI_HOST_HE_BF_CONF_UL_OFDMA_S)
#define WMI_HOST_HE_BF_CONF_UL_OFDMA_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_HE_BF_CONF_UL_OFDMA)
#define WMI_HOST_HE_BF_CONF_UL_OFDMA_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_HE_BF_CONF_UL_OFDMA)

#define WMI_HOST_HE_BF_CONF_UL_MUMIMO_S 6
#define WMI_HOST_HE_BF_CONF_UL_MUMIMO_M 0x1
#define WMI_HOST_HE_BF_CONF_UL_MUMIMO \
	(WMI_HOST_HE_BF_CONF_UL_MUMIMO_M << WMI_HOST_HE_BF_CONF_UL_MUMIMO_S)
#define WMI_HOST_HE_BF_CONF_UL_MUMIMO_GET(x) \
	WMI_HOST_F_MS(x, WMI_HOST_HE_BF_CONF_UL_MUMIMO)
#define WMI_HOST_HE_BF_CONF_UL_MUMIMO_SET(x, z) \
	WMI_HOST_F_RMW(x, z, WMI_HOST_HE_BF_CONF_UL_MUMIMO)

#define WMI_HOST_TPC_RATE_MAX	160
#define WMI_HOST_TPC_TX_NUM_CHAIN	4
#define WMI_HOST_RXG_CAL_CHAN_MAX	8
#define WMI_HOST_MAX_NUM_CHAINS	8
#define WMI_MAX_NUM_OF_RATE_THRESH   4

#define WMI_HOST_PDEV_MAX_VDEVS         17

/* for QC98XX only */
/*6 modes (A, HT20, HT40, VHT20, VHT40, VHT80) * 3 reg dommains
 */
#define WMI_HOST_NUM_CTLS_5G                18
/*6 modes (B, G, HT20, HT40, VHT20, VHT40) * 3 reg domains */
#define WMI_HOST_NUM_CTLS_2G                18
#define WMI_HOST_NUM_BAND_EDGES_5G          8
#define WMI_HOST_NUM_BAND_EDGES_2G          4

/*Beelinier 5G*/
#define WMI_HOST_NUM_CTLS_5G_11A            9
#define WMI_HOST_NUM_BAND_EDGES_5G_11A      25
#define WMI_HOST_NUM_CTLS_5G_HT20           24
#define WMI_HOST_NUM_BAND_EDGES_5G_HT20     25
#define WMI_HOST_NUM_CTLS_5G_HT40           18
#define WMI_HOST_NUM_BAND_EDGES_5G_HT40     12
#define WMI_HOST_NUM_CTLS_5G_HT80           18
#define WMI_HOST_NUM_BAND_EDGES_5G_HT80     6
#define WMI_HOST_NUM_CTLS_5G_HT160          9
#define WMI_HOST_NUM_BAND_EDGES_5G_HT160    2

/* Beeliner 2G */
#define WMI_HOST_NUM_CTLS_2G_11B            6
#define WMI_HOST_NUM_BAND_EDGES_2G_11B      9
#define WMI_HOST_NUM_CTLS_2G_20MHZ          30
#define WMI_HOST_NUM_BAND_EDGES_2G_20MHZ    11
#define WMI_HOST_NUM_CTLS_2G_40MHZ          18
#define WMI_HOST_NUM_BAND_EDGES_2G_40MHZ    6

/* for QC98XX only */
#define WMI_HOST_TX_NUM_CHAIN               0x3
#define WMI_HOST_TPC_REGINDEX_MAX           4
#define WMI_HOST_ARRAY_GAIN_NUM_STREAMS     2

#include "qdf_atomic.h"

#ifdef BIG_ENDIAN_HOST
	/* This API is used in copying in elements to WMI message,
	since WMI message uses multilpes of 4 bytes, This API
	converts length into multiples of 4 bytes, and performs copy
	*/
#define WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(destp, srcp, len)  do { \
	int j; \
	u_int32_t *src, *dest; \
	src = (u_int32_t *)srcp; \
	dest = (u_int32_t *)destp; \
	for (j = 0; j < roundup(len, sizeof(u_int32_t))/4; j++) { \
		*(dest+j) = qdf_le32_to_cpu(*(src+j)); \
	} \
} while (0)
#else

#define WMI_HOST_IF_MSG_COPY_CHAR_ARRAY(destp, srcp, len) OS_MEMCPY(destp,\
		srcp, len)

#endif

/** macro to convert MAC address from WMI word format to char array */
#define WMI_HOST_MAC_ADDR_TO_CHAR_ARRAY(pwmi_mac_addr, c_macaddr) do {	\
	(c_macaddr)[0] =  ((pwmi_mac_addr)->mac_addr31to0) & 0xff; \
	(c_macaddr)[1] =  (((pwmi_mac_addr)->mac_addr31to0) >> 8) & 0xff; \
	(c_macaddr)[2] =  (((pwmi_mac_addr)->mac_addr31to0) >> 16) & 0xff; \
	(c_macaddr)[3] =  (((pwmi_mac_addr)->mac_addr31to0) >> 24) & 0xff;  \
	(c_macaddr)[4] =  ((pwmi_mac_addr)->mac_addr47to32) & 0xff; \
	(c_macaddr)[5] =  (((pwmi_mac_addr)->mac_addr47to32) >> 8) & 0xff; \
	} while (0)

#define TARGET_INIT_STATUS_SUCCESS   0x0
#define TARGET_INIT_STATUS_GEN_FAILED   0x1
#define TARGET_GET_INIT_STATUS_REASON(status)	((status) & 0xffff)
#define TARGET_GET_INIT_STATUS_MODULE_ID(status) (((status) >> 16) & 0xffff)

#define MAX_ASSOC_IE_LENGTH 1024
typedef uint32_t TARGET_INIT_STATUS;

/**
 * @brief Opaque handle of wmi structure
 */
struct wmi_unified;
typedef struct wmi_unified *wmi_unified_t;

typedef void *ol_scn_t;
/**
 * @wmi_event_handler function prototype
 */
typedef int (*wmi_unified_event_handler)(ol_scn_t scn_handle,
		 uint8_t *event_buf, uint32_t len);

typedef enum {
	WMI_HOST_MODE_11A	= 0,   /* 11a Mode */
	WMI_HOST_MODE_11G	= 1,   /* 11b/g Mode */
	WMI_HOST_MODE_11B	= 2,   /* 11b Mode */
	WMI_HOST_MODE_11GONLY	= 3,   /* 11g only Mode */
	WMI_HOST_MODE_11NA_HT20   = 4,  /* 11a HT20 mode */
	WMI_HOST_MODE_11NG_HT20   = 5,  /* 11g HT20 mode */
	WMI_HOST_MODE_11NA_HT40   = 6,  /* 11a HT40 mode */
	WMI_HOST_MODE_11NG_HT40   = 7,  /* 11g HT40 mode */
	WMI_HOST_MODE_11AC_VHT20 = 8,
	WMI_HOST_MODE_11AC_VHT40 = 9,
	WMI_HOST_MODE_11AC_VHT80 = 10,
	WMI_HOST_MODE_11AC_VHT20_2G = 11,
	WMI_HOST_MODE_11AC_VHT40_2G = 12,
	WMI_HOST_MODE_11AC_VHT80_2G = 13,
	WMI_HOST_MODE_11AC_VHT80_80 = 14,
	WMI_HOST_MODE_11AC_VHT160 = 15,
	WMI_HOST_MODE_11AX_HE20 = 16,
	WMI_HOST_MODE_11AX_HE40 = 17,
	WMI_HOST_MODE_11AX_HE80 = 18,
	WMI_HOST_MODE_11AX_HE80_80 = 19,
	WMI_HOST_MODE_11AX_HE160 = 20,
	WMI_HOST_MODE_11AX_HE20_2G = 21,
	WMI_HOST_MODE_11AX_HE40_2G = 22,
	WMI_HOST_MODE_11AX_HE80_2G = 23,
	WMI_HOST_MODE_UNKNOWN = 24,
	WMI_HOST_MODE_MAX = 24
} WMI_HOST_WLAN_PHY_MODE;

typedef enum {
	WMI_HOST_VDEV_START_OK = 0,
	WMI_HOST_VDEV_START_CHAN_INVALID,
} WMI_HOST_VDEV_START_STATUS;

/*
 * Needs to be removed and use channel_param based
 * on how it is processed
 */
typedef struct {
	/** primary 20 MHz channel frequency in mhz */
	uint32_t mhz;
	/** Center frequency 1 in MHz*/
	uint32_t band_center_freq1;
	/** Center frequency 2 in MHz - valid only for 11acvht 80plus80 mode*/
	uint32_t band_center_freq2;
	/** channel info described below */
	uint32_t info;
	/** contains min power, max power, reg power and reg class id.  */
	uint32_t reg_info_1;
	/** contains antennamax */
	uint32_t reg_info_2;
} wmi_host_channel;

/**
 * enum WMI_HOST_REGDMN_MODE:
 * @WMI_HOST_REGDMN_MODE_11A: 11a channels
 * @WMI_HOST_REGDMN_MODE_TURBO: 11a turbo-only channels
 * @WMI_HOST_REGDMN_MODE_11B: 11b channels
 * @WMI_HOST_REGDMN_MODE_PUREG: 11g channels (OFDM only)
 * @WMI_HOST_REGDMN_MODE_11G: historical
 * @WMI_HOST_REGDMN_MODE_108G: 11g+Turbo channels
 * @WMI_HOST_REGDMN_MODE_108A: 11a+Turbo channels
 * @WMI_HOST_REGDMN_MODE_XR: XR channels
 * @WMI_HOST_REGDMN_MODE_11A_HALF_RATE: 11a half rate channels
 * @WMI_HOST_REGDMN_MODE_11A_QUARTER_RATE: 11a quarter rate channels
 * @WMI_HOST_REGDMN_MODE_11NG_HT20: 11ng HT20 channels
 * @WMI_HOST_REGDMN_MODE_11NA_HT20: 11na HT20 channels
 * @WMI_HOST_REGDMN_MODE_11NG_HT40PLUS: 11ng HT40+ channels
 * @WMI_HOST_REGDMN_MODE_11NG_HT40MINUS: 11ng HT40- channels
 * @WMI_HOST_REGDMN_MODE_11NA_HT40PLUS: 11na HT40+ channels
 * @WMI_HOST_REGDMN_MODE_11NA_HT40MINUS: 11na HT40- channels
 * @WMI_HOST_REGDMN_MODE_11AC_VHT20: 5GHz, VHT20
 * @WMI_HOST_REGDMN_MODE_11AC_VHT40PLUS: 5GHz, VHT40+ channels
 * @WMI_HOST_REGDMN_MODE_11AC_VHT40MINUS: 5GHz, VHT40- channels
 * @WMI_HOST_REGDMN_MODE_11AC_VHT80: 5GHz, VHT80 channels
 * @WMI_HOST_REGDMN_MODE_11AC_VHT160: 5GHz, VHT160 channels
 * @WMI_HOST_REGDMN_MODE_11AC_VHT80_80: 5GHz, VHT80+80 channels
 * @WMI_HOST_REGDMN_MODE_11AXG_HE20: 11ax 2.4GHz, HE20 channels
 * @WMI_HOST_REGDMN_MODE_11AXA_HE20: 11ax 5GHz, HE20 channels
 * @WMI_HOST_REGDMN_MODE_11AXG_HE40PLUS: 11ax 2.4GHz, HE40+ channels
 * @WMI_HOST_REGDMN_MODE_11AXG_HE40MINUS: 11ax 2.4GHz, HE40- channels
 * @WMI_HOST_REGDMN_MODE_11AXA_HE40PLUS: 11ax 5GHz, HE40+ channels
 * @WMI_HOST_REGDMN_MODE_11AXA_HE40MINUS: 11ax 5GHz, HE40- channels
 * @WMI_HOST_REGDMN_MODE_11AXA_HE80: 11ax 5GHz, HE80 channels
 * @WMI_HOST_REGDMN_MODE_11AXA_HE160: 11ax 5GHz, HE160 channels
 * @WMI_HOST_REGDMN_MODE_11AXA_HE80_80: 11ax 5GHz, HE80+80 channels
 */
typedef enum {
	WMI_HOST_REGDMN_MODE_11A = 0x00000001,
	WMI_HOST_REGDMN_MODE_TURBO = 0x00000002,
	WMI_HOST_REGDMN_MODE_11B = 0x00000004,
	WMI_HOST_REGDMN_MODE_PUREG = 0x00000008,
	WMI_HOST_REGDMN_MODE_11G = 0x00000008,
	WMI_HOST_REGDMN_MODE_108G = 0x00000020,
	WMI_HOST_REGDMN_MODE_108A = 0x00000040,
	WMI_HOST_REGDMN_MODE_XR = 0x00000100,
	WMI_HOST_REGDMN_MODE_11A_HALF_RATE = 0x00000200,
	WMI_HOST_REGDMN_MODE_11A_QUARTER_RATE = 0x00000400,
	WMI_HOST_REGDMN_MODE_11NG_HT20 = 0x00000800,
	WMI_HOST_REGDMN_MODE_11NA_HT20 = 0x00001000,
	WMI_HOST_REGDMN_MODE_11NG_HT40PLUS = 0x00002000,
	WMI_HOST_REGDMN_MODE_11NG_HT40MINUS = 0x00004000,
	WMI_HOST_REGDMN_MODE_11NA_HT40PLUS = 0x00008000,
	WMI_HOST_REGDMN_MODE_11NA_HT40MINUS = 0x00010000,
	WMI_HOST_REGDMN_MODE_11AC_VHT20 = 0x00020000,
	WMI_HOST_REGDMN_MODE_11AC_VHT40PLUS = 0x00040000,
	WMI_HOST_REGDMN_MODE_11AC_VHT40MINUS = 0x00080000,
	WMI_HOST_REGDMN_MODE_11AC_VHT80 = 0x00100000,
	WMI_HOST_REGDMN_MODE_11AC_VHT160 = 0x00200000,
	WMI_HOST_REGDMN_MODE_11AC_VHT80_80 = 0x00400000,
	WMI_HOST_REGDMN_MODE_11AXG_HE20 = 0x00800000,
	WMI_HOST_REGDMN_MODE_11AXA_HE20 = 0x01000000,
	WMI_HOST_REGDMN_MODE_11AXG_HE40PLUS = 0x02000000,
	WMI_HOST_REGDMN_MODE_11AXG_HE40MINUS = 0x04000000,
	WMI_HOST_REGDMN_MODE_11AXA_HE40PLUS = 0x08000000,
	WMI_HOST_REGDMN_MODE_11AXA_HE40MINUS = 0x10000000,
	WMI_HOST_REGDMN_MODE_11AXA_HE80 = 0x20000000,
	WMI_HOST_REGDMN_MODE_11AXA_HE160 = 0x40000000,
	WMI_HOST_REGDMN_MODE_11AXA_HE80_80 = 0x80000000,
	WMI_HOST_REGDMN_MODE_ALL = 0xffffffff
} WMI_HOST_REGDMN_MODE;

/**
 * enum WMI_HOST_WLAN_BAND_CAPABILITY: Band capability (2.4 GHz, 5 GHz). Maps to
 *               WLAN_BAND_CAPABILITY used in firmware header file(s).
 * @WMI_HOST_WLAN_2G_CAPABILITY: 2.4 GHz capable
 * @WMI_HOST_WLAN_5G_CAPABILITY: 5 GHz capable
 */
typedef enum {
	WMI_HOST_WLAN_2G_CAPABILITY = 0x1,
	WMI_HOST_WLAN_5G_CAPABILITY = 0x2,
} WMI_HOST_WLAN_BAND_CAPABILITY;

/**
 * enum wmi_host_channel_width: Channel operating width. Maps to
 *               wmi_channel_width used in firmware header file(s).
 * @WMI_HOST_CHAN_WIDTH_20: 20 MHz channel operating width
 * @WMI_HOST_CHAN_WIDTH_40: 40 MHz channel operating width
 * @WMI_HOST_CHAN_WIDTH_80: 80 MHz channel operating width
 * @WMI_HOST_CHAN_WIDTH_160: 160 MHz channel operating width
 * @WMI_HOST_CHAN_WIDTH_80P80: 80+80 MHz channel operating width
 * @WMI_HOST_CHAN_WIDTH_5: 5 MHz channel operating width
 * @WMI_HOST_CHAN_WIDTH_10: 10 MHz channel operating width
 */
typedef enum {
	WMI_HOST_CHAN_WIDTH_20    = 0,
	WMI_HOST_CHAN_WIDTH_40    = 1,
	WMI_HOST_CHAN_WIDTH_80    = 2,
	WMI_HOST_CHAN_WIDTH_160   = 3,
	WMI_HOST_CHAN_WIDTH_80P80 = 4,
	WMI_HOST_CHAN_WIDTH_5     = 5,
	WMI_HOST_CHAN_WIDTH_10    = 6,
} wmi_host_channel_width;

/**
 * enum wmi_dwelltime_adaptive_mode: dwelltime_mode
 * @WMI_DWELL_MODE_DEFAULT: Use firmware default mode
 * @WMI_DWELL_MODE_CONSERVATIVE: Conservative adaptive mode
 * @WMI_DWELL_MODE_MODERATE: Moderate adaptive mode
 * @WMI_DWELL_MODE_AGGRESSIVE: Aggressive adaptive mode
 * @WMI_DWELL_MODE_STATIC: static adaptive mode
 */
enum wmi_dwelltime_adaptive_mode {
	WMI_DWELL_MODE_DEFAULT = 0,
	WMI_DWELL_MODE_CONSERVATIVE = 1,
	WMI_DWELL_MODE_MODERATE = 2,
	WMI_DWELL_MODE_AGGRESSIVE = 3,
	WMI_DWELL_MODE_STATIC = 4
};

#define MAX_NUM_CHAN 128

#define ATH_EXPONENT_TO_VALUE(v)	((1<<v)-1)
#define ATH_TXOP_TO_US(v)		   (v<<5)
/* WME stream classes */
#define WMI_HOST_AC_BE                          0    /* best effort */
#define WMI_HOST_AC_BK                          1    /* background */
#define WMI_HOST_AC_VI                          2    /* video */
#define WMI_HOST_AC_VO                          3    /* voice */
#define WMI_TID_TO_AC(_tid) (\
		(((_tid) == 0) || ((_tid) == 3)) ? WMI_HOST_AC_BE : \
		(((_tid) == 1) || ((_tid) == 2)) ? WMI_HOST_AC_BK : \
		(((_tid) == 4) || ((_tid) == 5)) ? WMI_HOST_AC_VI : \
		WMI_HOST_AC_VO)

/**
 * struct vdev_create_params - vdev create cmd parameter
 * @if_id: interface id
 * @type: interface type
 * @subtype: interface subtype
 * @nss_2g: NSS for 2G
 * @nss_5g: NSS for 5G
 * @pdev_id: pdev id on pdev for this vdev
 */
struct vdev_create_params {
	uint8_t if_id;
	uint32_t type;
	uint32_t subtype;
	uint8_t nss_2g;
	uint8_t nss_5g;
	uint32_t pdev_id;
};

/**
 * struct vdev_delete_params - vdev delete cmd parameter
 * @if_id: interface id
 */
struct vdev_delete_params {
	uint8_t if_id;
};

/**
 * struct channel_param - Channel paramters with all
 *			info required by target.
 * @chan_id: channel id
 * @pwr: channel power
 * @mhz: channel frequency
 * @half_rate: is half rate
 * @quarter_rate: is quarter rate
 * @dfs_set: is dfs channel
 * @dfs_set_cfreq2: is secondary freq dfs channel
 * @is_chan_passive: is this passive channel
 * @allow_ht: HT allowed in chan
 * @allow_vht: VHT allowed on chan
 * @set_agile: is agile mode
 * @phy_mode: phymode (vht80 or ht40 or ...)
 * @cfreq1: centre frequency on primary
 * @cfreq2: centre frequency on secondary
 * @maxpower: max power for channel
 * @minpower: min power for channel
 * @maxreqpower: Max regulatory power
 * @antennamac: Max antenna
 * @reg_class_id: Regulatory class id.
 */

struct channel_param {
	uint8_t chan_id;
	uint8_t pwr;
	uint32_t mhz;
	uint32_t half_rate:1,
		quarter_rate:1,
		dfs_set:1,
		dfs_set_cfreq2:1,
		is_chan_passive:1,
		allow_ht:1,
		allow_vht:1,
		set_agile:1;
	uint32_t phy_mode;
	uint32_t cfreq1;
	uint32_t cfreq2;
	int8_t   maxpower;
	int8_t   minpower;
	int8_t   maxregpower;
	uint8_t  antennamax;
	uint8_t  reg_class_id;
};

/**
 * struct vdev_stop_params - vdev stop cmd parameter
 * @vdev_id: vdev id
 */
struct vdev_stop_params {
	uint8_t vdev_id;
};

/**
 * struct vdev_up_params - vdev up cmd parameter
 * @vdev_id: vdev id
 * @assoc_id: association id
 */
struct vdev_up_params {
	uint8_t vdev_id;
	uint16_t assoc_id;
};

/**
 * struct vdev_down_params - vdev down cmd parameter
 * @vdev_id: vdev id
 */
struct vdev_down_params {
	uint8_t vdev_id;
};

/**
 * struct mac_ssid - mac ssid structure
 * @length:
 * @mac_ssid[WMI_MAC_MAX_SSID_LENGTH]:
 */
struct mac_ssid {
	uint8_t length;
	uint8_t mac_ssid[WMI_MAC_MAX_SSID_LENGTH];
} qdf_packed;

/**
 * enum wmi_bcn_tx_rate_code - beacon tx rate code
 */
enum wmi_bcn_tx_rate_code {
	WMI_BCN_TX_RATE_CODE_1_M = 0x43,
	WMI_BCN_TX_RATE_CODE_2_M = 0x42,
	WMI_BCN_TX_RATE_CODE_5_5_M = 0x41,
	WMI_BCN_TX_RATE_CODE_6_M = 0x03,
	WMI_BCN_TX_RATE_CODE_9_M = 0x07,
	WMI_BCN_TX_RATE_CODE_11M = 0x40,
	WMI_BCN_TX_RATE_CODE_12_M = 0x02,
	WMI_BCN_TX_RATE_CODE_18_M = 0x06,
	WMI_BCN_TX_RATE_CODE_24_M = 0x01,
	WMI_BCN_TX_RATE_CODE_36_M = 0x05,
	WMI_BCN_TX_RATE_CODE_48_M = 0x00,
	WMI_BCN_TX_RATE_CODE_54_M = 0x04,
};

/**
 * struct vdev_start_params - vdev start cmd parameter
 * @vdev_id: vdev id
 * @chan_freq: channel frequency
 * @chan_mode: channel mode
 * @band_center_freq1: center freq 1
 * @band_center_freq2: center freq 2
 * @flags: flags to set like pmf_enabled etc.
 * @is_dfs: flag to check if dfs enabled
 * @beacon_intval: beacon interval
 * @dtim_period: dtim period
 * @max_txpow: max tx power
 * @is_restart: flag to check if it is vdev
 * @ssid: ssid and ssid length info
 * @preferred_tx_streams: preferred tx streams
 * @preferred_rx_streams: preferred rx streams
 * @intr_update: flag to check if need to update
 *               required wma interface params
 * @intr_ssid: pointer to wma interface ssid
 * @intr_flags: poiter to wma interface flags
 * @requestor_id: to update requestor id
 * @disable_hw_ack: to update disable hw ack flag
 * @info: to update channel info
 * @reg_info_1: to update min power, max power,
 *              reg power and reg class id
 * @reg_info_2: to update antennamax
 * @cac_duration_ms: cac duration in milliseconds
 * @regdomain: Regulatory domain
 * @oper_mode: Operating mode
 * @dfs_pri_multiplier: DFS primary multiplier
 *    allow pulse if they are within multiple of PRI for the radar type
 * @dot11_mode: Phy mode (VHT20/VHT80...)
 * @disable_hw_ack: Disable hw ack if chan is dfs channel for cac
 * @channel_param: Channel params required by target.
 * @bcn_tx_rate_code: Beacon tx rate code.
 * @ldpc_rx_enabled: Enable/Disable LDPC RX for this vdev
 */
struct vdev_start_params {
	uint8_t vdev_id;
	uint32_t chan_freq;
	uint32_t chan_mode;
	uint32_t band_center_freq1;
	uint32_t band_center_freq2;
	uint32_t flags;
	bool is_dfs;
	uint32_t beacon_intval;
	uint32_t dtim_period;
	int32_t max_txpow;
	bool is_restart;
	bool is_half_rate;
	bool is_quarter_rate;
	uint32_t dis_hw_ack;
	uint32_t flag_dfs;
	uint8_t hidden_ssid;
	uint8_t pmf_enabled;
	struct mac_ssid ssid;
	uint32_t num_noa_descriptors;
	uint32_t preferred_rx_streams;
	uint32_t preferred_tx_streams;
	uint32_t cac_duration_ms;
	uint32_t regdomain;
	uint32_t he_ops;
#ifndef CONFIG_MCL
	uint8_t oper_mode;
	int32_t dfs_pri_multiplier;
	uint8_t dot11_mode;
	uint8_t disable_hw_ack;
	struct channel_param channel;
#endif
	enum wmi_bcn_tx_rate_code bcn_tx_rate_code;
	bool ldpc_rx_enabled;
};

/**
 * struct vdev_scan_nac_rssi_params - NAC_RSSI cmd parameter
 * @vdev_id: vdev id
 * @bssid_addr: BSSID address
 * @client_addr: client address
 * @chan_num: channel number
 * @action:NAC_RSSI action,
 */
struct vdev_scan_nac_rssi_params {
	uint32_t vdev_id;
	uint8_t bssid_addr[IEEE80211_ADDR_LEN];
	uint8_t client_addr[IEEE80211_ADDR_LEN];
	uint32_t chan_num;
	uint32_t action; /* WMI_FILTER_NAC_RSSI_ACTION */
};

/**
 * struct hidden_ssid_vdev_restart_params -
 *                    vdev restart cmd parameter
 * @session_id: session id
 * @ssid_len: ssid length
 * @ssid: ssid
 * @flags: flags
 * @requestor_id: requestor id
 * @disable_hw_ack: flag to disable hw ack feature
 * @mhz: channel frequency
 * @band_center_freq1: center freq 1
 * @band_center_freq2: center freq 2
 * @info: channel info
 * @reg_info_1: contains min power, max power,
 *              reg power and reg class id
 * @reg_info_2: contains antennamax
 * @hidden_ssid_restart_in_progress:
 *      flag to check if restart is in progress
 */
struct hidden_ssid_vdev_restart_params {
	uint8_t session_id;
	uint32_t ssid_len;
	uint32_t ssid[8];
	uint32_t flags;
	uint32_t requestor_id;
	uint32_t disable_hw_ack;
	uint32_t mhz;
	uint32_t band_center_freq1;
	uint32_t band_center_freq2;
	uint32_t info;
	uint32_t reg_info_1;
	uint32_t reg_info_2;
	qdf_atomic_t hidden_ssid_restart_in_progress;
};

/**
 * struct vdev_set_params - vdev set cmd parameter
 * @if_id: vdev id
 * @param_id: parameter id
 * @param_value: parameter value
 */
struct vdev_set_params {
	uint32_t if_id;
	uint32_t param_id;
	uint32_t param_value;
};


/**
 * struct peer_delete_params - peer delete cmd parameter
 * @vdev_id: vdev id
 */
struct peer_delete_params {
	uint8_t vdev_id;
};

/**
 * struct peer_flush_params - peer flush cmd parameter
 * @peer_tid_bitmap: peer tid bitmap
 * @vdev_id: vdev id
 */
struct peer_flush_params {
	uint32_t peer_tid_bitmap;
	uint8_t vdev_id;
};

/**
 * struct peer_set_params - peer set cmd parameter
 * @param_id: parameter id
 * @param_value: parameter value
 * @vdev_id: vdev id
 */
struct peer_set_params {
	uint32_t param_id;
	uint32_t param_value;
	uint32_t vdev_id;
};

/**
 * struct peer_create_params - peer create cmd parameter
 * @peer_addr: peer mac addr
 * @peer_type: peer type
 * @vdev_id: vdev id
 */
struct peer_create_params {
	const uint8_t *peer_addr;
	uint32_t peer_type;
	uint32_t vdev_id;
};

/**
 * struct peer_remove_params - peer remove cmd parameter
 * @bssid: bss id
 * @vdev_id: vdev id
 * @roam_synch_in_progress: flag to indicate if roaming is in progress
 */
struct peer_remove_params {
	uint8_t *bssid;
	uint8_t vdev_id;
	bool roam_synch_in_progress;
};

/**
 * Stats ID enums defined in host
 */
typedef enum {
	WMI_HOST_REQUEST_PEER_STAT = 0x01,
	WMI_HOST_REQUEST_AP_STAT = 0x02,
	WMI_HOST_REQUEST_PDEV_STAT = 0x04,
	WMI_HOST_REQUEST_VDEV_STAT = 0x08,
	WMI_HOST_REQUEST_BCNFLT_STAT = 0x10,
	WMI_HOST_REQUEST_VDEV_RATE_STAT = 0x20,
	WMI_HOST_REQUEST_INST_STAT  = 0x40,
	WMI_HOST_REQUEST_PEER_EXTD_STAT =  0x80,
	WMI_HOST_REQUEST_VDEV_EXTD_STAT =  0x100,
	WMI_HOST_REQUEST_NAC_RSSI =  0x200,
	WMI_HOST_REQUEST_BCN_STAT =  0x800,
} wmi_host_stats_id;

typedef struct {
	uint16_t cfg_retry_count;
	uint16_t retry_count;
} wmi_host_inst_rssi_args;

/**
 * struct stats_request_params - stats_request cmd parameter
 * @stats_id: Bit mask of all the STATS request are specified with values from wmi_host_stats_id
 * @vdev_id: vdev id
 * @pdev_id: pdev_id
 * @wmi_host_inst_rssi_args: Instantaneous rssi stats args
 */
struct stats_request_params {
	uint32_t stats_id;
	uint8_t vdev_id;
	uint8_t pdev_id;
	wmi_host_inst_rssi_args rssi_args;
};

/**
 * struct bss_chan_info_request_params - BSS chan info params
 * @param: parameter value
 */
struct bss_chan_info_request_params {
	uint32_t param;
};

/**
 * struct wow_cmd_params - wow cmd parameter
 * @enable: wow enable or disable flag
 * @can_suspend_link: flag to indicate if link can be suspended
 * @pause_iface_config: interface config
 */
struct wow_cmd_params {
	bool enable;
	bool can_suspend_link;
	uint8_t pause_iface_config;
	uint32_t flags;
};

/**
 * struct wow_add_wakeup_params - wow wakeup parameter
 * @type: param type
 */
struct wow_add_wakeup_params {
	uint32_t type;
};

/**
 * struct wow_add_wakeup_pattern_params - Add WoW pattern params
 * @pattern_bytes: pointer to pattern bytes
 * @mask_bytes: pointer to mask bytes
 * @pattern_len: pattern length
 * @pattern_id: pattern id
 */
struct wow_add_wakeup_pattern_params {
	uint8_t *pattern_bytes;
	uint8_t *mask_bytes;
	uint32_t pattern_len;
	uint32_t pattern_id;
};

/**
 * struct wow_remove_wakeup_pattern params - WoW remove pattern param
 * @pattern_bytes: pointer to pattern bytes
 * @mask_bytes: Mask bytes
 * @pattern_id: pattern identifier
 */
struct wow_remove_wakeup_pattern_params {
	uint32_t *pattern_bytes;
	uint32_t *mask_bytes;
	uint32_t pattern_id;
};


/**
 * struct packet_enable_params - packet enable cmd parameter
 * @vdev_id: vdev id
 * @enable: flag to indicate if parameter can be enabled or disabled
 */
struct packet_enable_params {
	uint8_t vdev_id;
	bool enable;
};

/**
 * struct suspend_params - suspend cmd parameter
 * @disable_target_intr: disable target interrupt
 */
struct suspend_params {
	uint8_t disable_target_intr;
};

/**
 * struct pdev_params - pdev set cmd parameter
 * @param_id: parameter id
 * @param_value: parameter value
 */
struct pdev_params {
	uint32_t param_id;
	uint32_t param_value;
};

/**
 * struct beacon_tmpl_params - beacon template cmd parameter
 * @vdev_id: vdev id
 * @tim_ie_offset: tim ie offset
 * @tmpl_len: beacon template length
 * @tmpl_len_aligned: beacon template alignment
 * @csa_switch_count_offset: CSA swith count offset in beacon frame
 * @ext_csa_switch_count_offset: ECSA switch count offset in beacon frame
 * @frm: beacon template parameter
 */
struct beacon_tmpl_params {
	uint8_t vdev_id;
	uint32_t tim_ie_offset;
	uint32_t tmpl_len;
	uint32_t tmpl_len_aligned;
	uint32_t csa_switch_count_offset;
	uint32_t ext_csa_switch_count_offset;
	uint8_t *frm;
};

/**
 * struct beacon_params - beacon cmd parameter
 * @vdev_id: vdev id
 * @beaconInterval: Beacon interval
 * @wbuf: beacon buffer
 * @frame_ctrl: frame control field
 * @bcn_txant: beacon antenna
 * @is_dtim_count_zero: is it dtim beacon
 * @is_bitctl_reqd: is Bit control required
 * @is_high_latency: Is this high latency target
 */
struct beacon_params {
	uint8_t vdev_id;
	uint16_t beaconInterval;
	qdf_nbuf_t wbuf;
	uint16_t frame_ctrl;
	uint32_t bcn_txant;
	bool is_dtim_count_zero;
	bool is_bitctl_reqd;
	bool is_high_latency;
};

/**
 * struct fd_params - FD cmd parameter
 * @vdev_id: vdev id
 * @wbuf: FD buffer
 * @frame_ctrl: frame control field
 */
struct fd_params {
	uint8_t vdev_id;
	qdf_nbuf_t wbuf;
	uint16_t frame_ctrl;
};

/**
 * struct bcn_prb_template_params - beacon probe template parameter
 * @vdev_id: vdev id
 * @buf_len: Template length
 * @caps: capabilities field
 * @erp: ERP field
 */
struct bcn_prb_template_params {
	uint8_t vdev_id;
	int buf_len;
	uint16_t caps;
	uint8_t erp;
};

#define WMI_MAX_SUPPORTED_RATES 128
/**
 * struct target_rate_set - Rate set bitmap
 * @num_rate: number of rates in rates bitmap
 * @rates: rates (each 8bit value) packed into a 32 bit word.
 *      the rates are filled from least significant byte to most
 *      significant byte.
 */
typedef struct {
	uint32_t num_rates;
	uint32_t rates[(WMI_MAX_SUPPORTED_RATES / 4) + 1];
} target_rate_set;


#define WMI_HOST_MAX_NUM_SS		8
#define WMI_HOST_MAX_HECAP_PHY_SIZE	3
#define WMI_HOST_MAX_HE_RATE_SET	3
/**
 * struct wmi_host_ppe_threshold -PPE threshold
 * @numss_m1: NSS - 1
 * @ru_bit_mask: RU bit mask indicating the supported RU's
 * @ppet16_ppet8_ru3_ru0: ppet8 and ppet16 for max num ss
 */
struct wmi_host_ppe_threshold {
	uint32_t numss_m1;
	uint32_t ru_bit_mask;
	uint32_t ppet16_ppet8_ru3_ru0[WMI_HOST_MAX_NUM_SS];
};

/**
 * struct wmi_host_mac_addr - host mac addr 2 word representation of MAC addr
 * @mac_addr31to0: upper 4 bytes of  MAC address
 * @mac_addr47to32: lower 2 bytes of  MAC address
 */
typedef struct {
	uint32_t mac_addr31to0;
	uint32_t mac_addr47to32;
} wmi_host_mac_addr;

/**
 * struct peer_assoc_params - peer assoc cmd parameter
 * @peer_macaddr: peer mac address
 * @vdev_id: vdev id
 * @peer_new_assoc: peer association type
 * @peer_associd: peer association id
 * @peer_flags: peer flags
 * @peer_caps: peer capabalities
 * @peer_listen_intval: peer listen interval
 * @peer_ht_caps: HT capabalities
 * @peer_max_mpdu: 0 : 8k , 1 : 16k, 2 : 32k, 3 : 64k
 * @peer_mpdu_density: 3 : 0~7 : 2^(11nAMPDUdensity -4)
 * @peer_rate_caps: peer rate capabalities
 * @peer_nss: peer nss
 * @peer_phymode: peer phymode
 * @peer_ht_info: peer HT info
 * @peer_legacy_rates: peer legacy rates
 * @peer_ht_rates: peer ht rates
 * @rx_max_rate: max rx rates
 * @rx_mcs_set: rx mcs
 * @tx_max_rate: max tx rates
 * @tx_mcs_set: tx mcs
 * @vht_capable: VHT capabalities
 * @tx_max_mcs_nss: max tx MCS and NSS
 * @peer_bw_rxnss_override: Peer BW RX NSS overriden or not.
 * @is_pmf_enabled: PMF enabled
 * @is_wme_set: WME enabled
 * @qos_flag: QoS Flags
 * @apsd_flag: APSD flags
 * @ht_flag: HT flags
 * @bw_40: 40 capabale
 * @bw_80: 80 capabale
 * @bw_160: 160 capabale
 * @stbc_flag: STBC flag
 * @ldpc_flag: LDPC flag
 * @static_mimops_flag: statis MIMO PS flags
 * @dynamic_mimops_flag: Dynamic MIMO PS flags
 * @spatial_mux_flag: spatial MUX flags
 * @vht_flag: VHT flags
 * @vht_ng_flag: VHT on 11N/G flags
 * @need_ptk_4_way: Needs 4 way handshake
 * @need_gtk_2_way: Needs 2 way GTK
 * @auth_flag: Is peer authenticated
 * @safe_mode_enabled: Safe enabled for this peer
 * @amsdu_disable: AMSDU disble
 * @peer_mac: Peer mac address
 * @he_flag: HE flags
 * @peer_he_cap_macinfo: Peer HE Cap MAC info
 * @peer_he_ops: Peer HE operation info
 * @peer_he_cap_phyinfo: Peer HE Cap PHY info
 * @peer_he_mcs_count: Peer HE MCS TX/RX MAP count
 * @peer_he_rx_mcs_set: Peer HE RX MCS MAP
 * @peer_he_tx_mcs_set: Peer HE TX MCS MAP
 * @peer_ppet: Peer HE PPET info
 */
struct peer_assoc_params {
	wmi_host_mac_addr peer_macaddr;
	uint32_t vdev_id;
	uint32_t peer_new_assoc;
	uint32_t peer_associd;
	uint32_t peer_flags;
	uint32_t peer_caps;
	uint32_t peer_listen_intval;
	uint32_t peer_ht_caps;
	uint32_t peer_max_mpdu;
	uint32_t peer_mpdu_density;
	uint32_t peer_rate_caps;
	uint32_t peer_nss;
	uint32_t peer_vht_caps;
	uint32_t peer_phymode;
	uint32_t peer_ht_info[2];
	target_rate_set peer_legacy_rates;
	target_rate_set peer_ht_rates;
	uint32_t rx_max_rate;
	uint32_t rx_mcs_set;
	uint32_t tx_max_rate;
	uint32_t tx_mcs_set;
	uint8_t vht_capable;
	uint32_t peer_bw_rxnss_override;
#ifndef CONFIG_MCL
	uint32_t tx_max_mcs_nss;
	bool is_pmf_enabled;
	bool is_wme_set;
	bool qos_flag;
	bool apsd_flag;
	bool ht_flag;
	bool bw_40;
	bool bw_80;
	bool bw_160;
	bool stbc_flag;
	bool ldpc_flag;
	bool static_mimops_flag;
	bool dynamic_mimops_flag;
	bool spatial_mux_flag;
	bool vht_flag;
	bool vht_ng_flag;
	bool need_ptk_4_way;
	bool need_gtk_2_way;
	bool auth_flag;
	bool safe_mode_enabled;
	bool amsdu_disable;
	/* Use common structure */
	uint8_t peer_mac[IEEE80211_ADDR_LEN];
#endif
	bool he_flag;
	uint32_t peer_he_cap_macinfo;
	uint32_t peer_he_ops;
	uint32_t peer_he_cap_phyinfo[WMI_HOST_MAX_HECAP_PHY_SIZE];
	uint32_t peer_he_mcs_count;
	uint32_t peer_he_rx_mcs_set[WMI_HOST_MAX_HE_RATE_SET];
	uint32_t peer_he_tx_mcs_set[WMI_HOST_MAX_HE_RATE_SET];
	struct wmi_host_ppe_threshold peer_ppet;
};

/**
 * struct sta_ps_params - sta ps cmd parameter
 * @vdev_id: vdev id
 * @param: sta ps paramter
 * @value: sta ps parameter value
 */
struct sta_ps_params {
	uint32_t vdev_id;
	uint32_t param;
	uint32_t value;
};

/**
 * struct ap_ps_params - ap ps cmd parameter
 * @vdev_id: vdev id
 * @param: ap ps paramter
 * @value: ap ps paramter value
 */
struct ap_ps_params {
	uint32_t vdev_id;
	uint32_t param;
	uint32_t value;
};

#define WMI_HOST_SCAN_CHAN_FREQ_SHIFT	0
#define WMI_HOST_SCAN_CHAN_FREQ_MASK	0xffff
#define WMI_HOST_SCAN_CHAN_MODE_SHIFT	16
#define WMI_HOST_SCAN_CHAN_MODE_MASK	0xff

#define WMI_HOST_MAX_CHANS_PER_WMI_CMD  58

/**
 * struct scan_chan_list_params  - scan channel list cmd parameter
 * @num_scan_chans: no of scan channels
 * @chan_info: pointer to wmi channel info
 */
#ifdef CONFIG_MCL
/* TODO: This needs clean-up based on how its processed. */
typedef struct {
	/* TLV tag and len; tag equals WMITLV_TAG_STRUC_wmi_channel */
	uint32_t tlv_header;
	/** primary 20 MHz channel frequency in mhz */
	uint32_t mhz;
	/** Center frequency 1 in MHz*/
	uint32_t band_center_freq1;
	/** Center frequency 2 in MHz - valid only for 11acvht 80plus80 mode*/
	uint32_t band_center_freq2;
	/** channel info described below */
	uint32_t info;
	/** contains min power, max power, reg power and reg class id.  */
	uint32_t reg_info_1;
	/** contains antennamax */
	uint32_t reg_info_2;
} wmi_channel_param;

struct scan_chan_list_params {
	uint32_t pdev_id;
	uint8_t num_scan_chans;
	wmi_channel_param *chan_info;
};
#else
/**
 * struct scan_chan_list_params  - scan channel list cmd parameter
 * @pdev_id: pdev_id
 * @num_chan: no of scan channels
 * @nallchans: nall chans
 * @append: append to existing chan list
 * @ch_param: pointer to channel_paramw
 */
struct scan_chan_list_params {
	uint32_t pdev_id;
	uint16_t nallchans;
	bool append;
	struct channel_param ch_param[1];
};
#endif

/**
 * struct multiple_vdev_restart_params - Multiple vdev restart cmd parameter
 * @pdev_id: Pdev identifier
 * @requestor_id: Unique id identifying the module
 * @disable_hw_ack: Flag to indicate disabling HW ACK during CAC
 * @cac_duration_ms: CAC duration on the given channel
 * @num_vdevs: No. of vdevs that need to be restarted
 * @ch_param: Pointer to channel_param
 * @vdev_ids: Pointer to array of vdev_ids
 */
struct multiple_vdev_restart_params {
	uint32_t pdev_id;
	uint32_t requestor_id;
	uint32_t disable_hw_ack;
	uint32_t cac_duration_ms;
	uint32_t num_vdevs;
	struct channel_param ch_param;
	uint32_t vdev_ids[WMI_HOST_PDEV_MAX_VDEVS];
};
/**
 * struct fw_hang_params - fw hang command parameters
 * @type: 0:unused 1: ASSERT, 2:not respond detect command, 3:simulate ep-full
 * @delay_time_ms: 0xffffffff means the simulate will delay for random time (0 ~0xffffffff ms)
 */
struct fw_hang_params {
	uint32_t type;
	uint32_t delay_time_ms;
};

/**
 * struct pdev_utf_params - pdev utf command parameters
 * @utf_payload:
 * @len:
 * @is_ar900b: is it 900b target
 */
struct pdev_utf_params {
	uint8_t *utf_payload;
	uint32_t len;
#ifndef CONFIG_MCL
	bool is_ar900b;
#endif
};

/*Adding this due to dependency on wmi_unified.h
 */
typedef struct {
	uint32_t len;
	uint32_t msgref;
	uint32_t segmentInfo;
} QVIT_SEG_HDR_INFO_STRUCT;

struct pdev_qvit_params {
	uint8_t *utf_payload;
	uint32_t len;
};
/**
 * struct crash_inject - crash inject command parameters
 * @type: crash inject type
 * @delay_time_ms: time in milliseconds for FW to delay the crash
 */
struct crash_inject {
	uint32_t type;
	uint32_t delay_time_ms;
};

/**
 * struct dbglog_params - fw deboglog command parameters
 * @param: command parameter
 * @val: parameter value
 * @module_id_bitmap: fixed length module id bitmap
 * @bitmap_len: module id bitmap length
 * @cfgvalid: cfgvalid
 */
struct dbglog_params {
	uint32_t param;
	uint32_t val;
	uint32_t *module_id_bitmap;
	uint32_t bitmap_len;
#ifndef CONFIG_MCL
	uint32_t cfgvalid[2];
#endif
};

/**
 * struct seg_hdr_info - header info
 * @len: length
 * @msgref: message refrence
 * @segmentInfo: segment info
 * @pad: padding
 */
struct seg_hdr_info {
	uint32_t len;
	uint32_t msgref;
	uint32_t segmentInfo;
	uint32_t pad;
};

/**
 * struct tx_send_params - TX parameters
 * @pwr: Tx frame transmission power
 * @mcs_mask: Modulation and coding index mask for transmission
 *	      bit  0 -> CCK 1 Mbps rate is allowed
 *	      bit  1 -> CCK 2 Mbps rate is allowed
 *	      bit  2 -> CCK 5.5 Mbps rate is allowed
 *	      bit  3 -> CCK 11 Mbps rate is allowed
 *	      bit  4 -> OFDM BPSK modulation, 1/2 coding rate is allowed
 *	      bit  5 -> OFDM BPSK modulation, 3/4 coding rate is allowed
 *	      bit  6 -> OFDM QPSK modulation, 1/2 coding rate is allowed
 *	      bit  7 -> OFDM QPSK modulation, 3/4 coding rate is allowed
 *	      bit  8 -> OFDM 16-QAM modulation, 1/2 coding rate is allowed
 *	      bit  9 -> OFDM 16-QAM modulation, 3/4 coding rate is allowed
 *	      bit 10 -> OFDM 64-QAM modulation, 2/3 coding rate is allowed
 *	      bit 11 -> OFDM 64-QAM modulation, 3/4 coding rate is allowed
 * @nss_mask: Spatial streams permitted
 *	      bit 0: if set, Nss = 1 (non-MIMO) is permitted
 *	      bit 1: if set, Nss = 2 (2x2 MIMO) is permitted
 *	      bit 2: if set, Nss = 3 (3x3 MIMO) is permitted
 *	      bit 3: if set, Nss = 4 (4x4 MIMO) is permitted
 *	      bit 4: if set, Nss = 5 (5x5 MIMO) is permitted
 *	      bit 5: if set, Nss = 6 (6x6 MIMO) is permitted
 *	      bit 6: if set, Nss = 7 (7x7 MIMO) is permitted
 *	      bit 7: if set, Nss = 8 (8x8 MIMO) is permitted
 *            If no bits are set, target will choose what NSS type to use
 * @retry_limit: Maximum number of retries before ACK
 * @chain_mask: Chains to be used for transmission
 * @bw_mask: Bandwidth to be used for transmission
 *	     bit  0 -> 5MHz
 *	     bit  1 -> 10MHz
 *	     bit  2 -> 20MHz
 *	     bit  3 -> 40MHz
 *	     bit  4 -> 80MHz
 *	     bit  5 -> 160MHz
 *	     bit  6 -> 80_80MHz
 * @preamble_type: Preamble types for transmission
 *	     bit 0: if set, OFDM
 *	     bit 1: if set, CCK
 *	     bit 2: if set, HT
 *	     bit 3: if set, VHT
 *	     bit 4: if set, HE
 * @frame_type: Data or Management frame
 *	        Data:1 Mgmt:0
 */
struct tx_send_params {
	uint32_t pwr:8,
		 mcs_mask:12,
		 nss_mask:8,
		 retry_limit:4;
	uint32_t chain_mask:8,
		 bw_mask:7,
		 preamble_type:5,
		 frame_type:1,
		 reserved:11;
};

/**
 * struct wmi_mgmt_params - wmi mgmt cmd paramters
 * @tx_frame: management tx frame
 * @frm_len: frame length
 * @vdev_id: vdev id
 * @chanfreq: channel frequency
 * @pdata: frame data
 * @desc_id: descriptor id relyaed back by target
 * @macaddr: macaddr of peer
 * @qdf_ctx: qdf context for qdf_nbuf_map
 * @tx_param: TX send parameters
 * @tx_params_valid: Flag that indicates if TX params are valid
 * @use_6mbps: specify whether management frame to transmit should
 *  use 6 Mbps rather than 1 Mbps min rate(for 5GHz band or P2P)
 * @tx_type: type of managment frame (determines what callback to use)
 */
struct wmi_mgmt_params {
	void *tx_frame;
	uint16_t frm_len;
	uint8_t vdev_id;
	uint16_t chanfreq;
	void *pdata;
	uint16_t desc_id;
	uint8_t *macaddr;
	void *qdf_ctx;
	struct tx_send_params tx_param;
	bool tx_params_valid;
	uint8_t use_6mbps;
	uint8_t tx_type;
};

/**
 * struct wmi_offchan_data_tx_params - wmi offchan data tx cmd paramters
 * @tx_frame: management tx frame
 * @frm_len: frame length
 * @vdev_id: vdev id
 * @chanfreq: channel frequency
 * @pdata: frame data
 * @desc_id: descriptor id relyaed back by target
 * @macaddr: macaddr of peer
 * @qdf_ctx: qdf context for qdf_nbuf_map
 * @tx_param: TX send parameters
 * @tx_params_valid: Flag that indicates if TX params are valid
 */
struct wmi_offchan_data_tx_params {
	void *tx_frame;
	uint16_t frm_len;
	uint8_t vdev_id;
	uint16_t chanfreq;
	void *pdata;
	uint16_t desc_id;
	uint8_t *macaddr;
	void *qdf_ctx;
	struct tx_send_params tx_param;
	bool tx_params_valid;
};

/**
 * struct p2p_ps_params - P2P powersave related params
 * @opp_ps: opportunistic power save
 * @ctwindow: CT window
 * @count: count
 * @duration: duration
 * @interval: interval
 * @single_noa_duration: single shot noa duration
 * @ps_selection: power save selection
 * @session_id: session id
 */
struct p2p_ps_params {
	uint8_t opp_ps;
	uint32_t ctwindow;
	uint8_t count;
	uint32_t duration;
	uint32_t interval;
	uint32_t single_noa_duration;
	uint8_t ps_selection;
	uint8_t session_id;
};

#ifndef CONVERGED_TDLS_ENABLE
/**
 * struct sta_uapsd_params - uapsd auto trig params
 * @wmm_ac: WMM access category from 0 to 3
 * @user_priority: User priority to use in trigger frames
 * @service_interval: service interval
 * @suspend_interval: suspend interval
 * @delay_interval: delay interval
 */
struct sta_uapsd_params {
	uint32_t wmm_ac;
	uint32_t user_priority;
	uint32_t service_interval;
	uint32_t suspend_interval;
	uint32_t delay_interval;
};

/**
 * struct ta_uapsd_trig_params - uapsd trigger parameter
 * @vdevid: vdev id
 * @peer_addr: peer address
 * @auto_triggerparam: trigger parameters
 * @num_ac: no of access category
 */
struct sta_uapsd_trig_params {
		uint32_t vdevid;
		uint8_t peer_addr[IEEE80211_ADDR_LEN];
		struct sta_uapsd_params *auto_triggerparam;
		uint32_t num_ac;
};
#endif

#define WMI_NUM_AC                     (4)
#define WMI_MAX_NUM_AC 4


enum wmi_peer_rate_report_cond_phy_type {
	WMI_PEER_RATE_REPORT_COND_11B = 0,
	WMI_PEER_RATE_REPORT_COND_11A_G,
	WMI_PEER_RATE_REPORT_COND_11N,
	WMI_PEER_RATE_REPORT_COND_11AC,
	WMI_PEER_RATE_REPORT_COND_MAX_NUM
};

/**
 * struct report_rate_delta - peer specific parameters
 * @percent: percentage
 * @delta_min: rate min delta
 */
struct report_rate_delta {
	uint32_t percent; /* in unit of 12.5% */
	uint32_t delta_min;  /* in unit of Mbps */
};

/**
 * struct report_rate_per_phy - per phy report parameters
 * @cond_flags: condition flag val
 * @delta: rate delta
 * @report_rate_threshold: rate threshold
 */
struct report_rate_per_phy {
	/*
	 * PEER_RATE_REPORT_COND_FLAG_DELTA,
	 * PEER_RATE_REPORT_COND_FLAG_THRESHOLD
	 * Any of these two conditions or both of
	 * them can be set.
	 */
	uint32_t cond_flags;
	struct report_rate_delta delta;
	/*
	 * In unit of Mbps. There are at most 4 thresholds
	 * If the threshold count is less than 4, set zero to
	 * the one following the last threshold
	 */
	uint32_t report_rate_threshold[WMI_MAX_NUM_OF_RATE_THRESH];
};

/**
 * struct peer_rate_report_params - peer rate report parameters
 * @rate_report_enable: enable rate report param
 * @backoff_time: backoff time
 * @timer_period: timer
 * @report_per_phy: report per phy type
 */
struct wmi_peer_rate_report_params {
	uint32_t rate_report_enable;
	uint32_t backoff_time;            /* in unit of msecond */
	uint32_t timer_period;            /* in unit of msecond */
	/*
	 *In the following field, the array index means the phy type,
	 * please see enum wmi_peer_rate_report_cond_phy_type for detail
	 */
	struct report_rate_per_phy report_per_phy[
				WMI_PEER_RATE_REPORT_COND_MAX_NUM];

};

/**
 * struct t_thermal_cmd_params - thermal command parameters
 * @min_temp: minimum temprature
 * @max_temp: maximum temprature
 * @thermal_enable: thermal enable
 */
struct thermal_cmd_params {
	uint16_t min_temp;
	uint16_t max_temp;
	uint8_t thermal_enable;
};

#define WMI_LRO_IPV4_SEED_ARR_SZ 5
#define WMI_LRO_IPV6_SEED_ARR_SZ 11

/**
 * struct wmi_lro_config_cmd_t - set LRO init parameters
 * @lro_enable: indicates whether lro is enabled
 * @tcp_flag: If the TCP flags from the packet do not match
 * the values in this field after masking with TCP flags mask
 * below, packet is not LRO eligible
 * @tcp_flag_mask: field for comparing the TCP values provided
 * above with the TCP flags field in the received packet
 * @toeplitz_hash_ipv4: contains seed needed to compute the flow id
 * 5-tuple toeplitz hash for ipv4 packets
 * @toeplitz_hash_ipv6: contains seed needed to compute the flow id
 * 5-tuple toeplitz hash for ipv6 packets
 */
struct wmi_lro_config_cmd_t {
	uint32_t lro_enable;
	uint32_t tcp_flag:9,
		tcp_flag_mask:9;
	uint32_t toeplitz_hash_ipv4[WMI_LRO_IPV4_SEED_ARR_SZ];
	uint32_t toeplitz_hash_ipv6[WMI_LRO_IPV6_SEED_ARR_SZ];
};

/**
 * struct gtx_config_t - GTX config
 * @gtx_rt_mask: for HT and VHT rate masks
 * @gtx_usrcfg: host request for GTX mask
 * @gtx_threshold: PER Threshold (default: 10%)
 * @gtx_margin: PER margin (default: 2%)
 * @gtx_tcpstep: TCP step (default: 1)
 * @gtx_tpcMin: TCP min (default: 5)
 * @gtx_bwmask: BW mask (20/40/80/160 Mhz)
 */
struct wmi_gtx_config {
	uint32_t gtx_rt_mask[2];
	uint32_t gtx_usrcfg;
	uint32_t gtx_threshold;
	uint32_t gtx_margin;
	uint32_t gtx_tpcstep;
	uint32_t gtx_tpcmin;
	uint32_t gtx_bwmask;
};

/**
 * struct wmi_probe_resp_params - send probe response parameters
 * @prb_rsp_template_frm: pointer to template probe response template
 * @prb_rsp_template_len: length of probe response template
 */
struct wmi_probe_resp_params {
	uint8_t *prb_rsp_template_frm;
	uint32_t prb_rsp_template_len;
};

/* struct set_key_params: structure containing
 *                        installation key parameters
 * @vdev_id: vdev id
 * @key_len: key length
 * @key_idx: key index
 * @peer_mac: peer mac address
 * @key_flags: key flags, 0:pairwise key, 1:group key, 2:static key
 * @key_cipher: key cipher based on security mode
 * @key_txmic_len: tx mic length
 * @key_rxmic_len: rx mic length
 * @key_tsc_counter:  key tx sc counter
 * @key_rsc_counter:  key rx sc counter
 * @rx_iv: receive IV, applicable only in case of WAPI
 * @tx_iv: transmit IV, applicable only in case of WAPI
 * @key_data: key data
 */
struct set_key_params {
	uint8_t vdev_id;
	uint16_t key_len;
	uint32_t key_idx;
	uint8_t peer_mac[IEEE80211_ADDR_LEN];
	uint32_t key_flags;
	uint32_t key_cipher;
	uint32_t key_txmic_len;
	uint32_t key_rxmic_len;
	uint64_t key_tsc_counter;
	uint64_t *key_rsc_counter;
#if defined(ATH_SUPPORT_WAPI) || defined(FEATURE_WLAN_WAPI)
	uint8_t rx_iv[16];
	uint8_t tx_iv[16];
#endif
	uint8_t key_data[WMI_MAC_MAX_KEY_LENGTH];
};

/**
 * struct sta_params - sta keep alive parameters
 * @vdev_id: vdev id
 * @method: keep alive method
 * @timeperiod: time to keep alive
 * @hostv4addr: host ipv4 address
 * @destv4addr: destination ipv4 address
 * @destmac: destination mac address
 */
struct sta_params {
	uint8_t vdev_id;
	uint32_t method;
	uint32_t timeperiod;
	uint8_t *hostv4addr;
	uint8_t *destv4addr;
	uint8_t *destmac;
};

/**
 * struct gateway_update_req_param - gateway parameter update request
 * @request_id: request id
 * @session_id: session id
 * @max_retries: Max ARP/NS retry attempts
 * @timeout: Retry interval
 * @ipv4_addr_type: on ipv4 network
 * @ipv6_addr_type: on ipv6 network
 * @gw_mac_addr: gateway mac addr
 * @ipv4_addr: ipv4 addr
 * @ipv6_addr: ipv6 addr
 */
struct gateway_update_req_param {
	uint32_t     request_id;
	uint32_t     session_id;
	uint32_t     max_retries;
	uint32_t     timeout;
	uint32_t     ipv4_addr_type;
	uint32_t     ipv6_addr_type;
	struct qdf_mac_addr  gw_mac_addr;
	uint8_t      ipv4_addr[QDF_IPV4_ADDR_SIZE];
	uint8_t      ipv6_addr[QDF_IPV6_ADDR_SIZE];
};

/**
 * struct rssi_monitor_param - rssi monitoring
 * @request_id: request id
 * @session_id: session id
 * @min_rssi: minimum rssi
 * @max_rssi: maximum rssi
 * @control: flag to indicate start or stop
 */
struct rssi_monitor_param {
	uint32_t request_id;
	uint32_t session_id;
	int8_t   min_rssi;
	int8_t   max_rssi;
	bool     control;
};

/**
 * struct scan_mac_oui - oui paramters
 * @oui: oui parameters
 * @vdev_id: interface id
 * @enb_probe_req_sno_randomization: control probe req sequence no randomization
 * @ie_whitelist: probe req IE whitelist attrs
 */
struct scan_mac_oui {
	uint8_t oui[WMI_WIFI_SCANNING_MAC_OUI_LENGTH];
	uint32_t vdev_id;
	bool enb_probe_req_sno_randomization;
	struct probe_req_whitelist_attr ie_whitelist;
};

#define WMI_PASSPOINT_REALM_LEN 256
#define WMI_PASSPOINT_ROAMING_CONSORTIUM_ID_NUM 16
#define WMI_PASSPOINT_PLMN_LEN 3
/**
 * struct wifi_passpoint_network_param - passpoint network block
 * @id: identifier of this network block
 * @realm: null terminated UTF8 encoded realm, 0 if unspecified
 * @roaming_consortium_ids: roaming consortium ids to match, 0s if unspecified
 * @plmn: mcc/mnc combination as per rules, 0s if unspecified
 */
struct wifi_passpoint_network_param {
	uint32_t id;
	uint8_t  realm[WMI_PASSPOINT_REALM_LEN];
	int64_t  roaming_consortium_ids[WMI_PASSPOINT_ROAMING_CONSORTIUM_ID_NUM];
	uint8_t  plmn[WMI_PASSPOINT_PLMN_LEN];
};

/**
 * struct wifi_passpoint_req_param - passpoint request
 * @request_id: request identifier
 * @num_networks: number of networks
 * @networks: passpoint networks
 */
struct wifi_passpoint_req_param {
	uint32_t request_id;
	uint32_t session_id;
	uint32_t num_networks;
	struct wifi_passpoint_network_param networks[];
};

/* struct mobility_domain_info - structure containing
 *                               mobility domain info
 * @mdie_present: mobility domain present or not
 * @mobility_domain: mobility domain
 */
struct mobility_domain_info {
	uint8_t mdie_present;
	uint16_t mobility_domain;
};

#define WMI_HOST_ROAM_OFFLOAD_NUM_MCS_SET     (16)

/* This TLV will be filled only in case roam offload
 * for wpa2-psk/pmkid/ese/11r is enabled */
typedef struct {
	/*
	 * TLV tag and len; tag equals
	 * WMITLV_TAG_STRUC_wmi_roam_offload_fixed_param
	 */
	uint32_t tlv_header;
	uint32_t rssi_cat_gap;          /* gap for every category bucket */
	uint32_t prefer_5g;             /* prefer select 5G candidate */
	uint32_t select_5g_margin;
	uint32_t reassoc_failure_timeout;       /* reassoc failure timeout */
	uint32_t capability;
	uint32_t ht_caps_info;
	uint32_t ampdu_param;
	uint32_t ht_ext_cap;
	uint32_t ht_txbf;
	uint32_t asel_cap;
	uint32_t qos_enabled;
	uint32_t qos_caps;
	uint32_t wmm_caps;
	/* since this is 4 byte aligned, we don't declare it as tlv array */
	uint32_t mcsset[WMI_HOST_ROAM_OFFLOAD_NUM_MCS_SET >> 2];
} roam_offload_param;

#define WMI_FILS_MAX_RRK_LENGTH 64
#define WMI_FILS_MAX_RIK_LENGTH WMI_FILS_MAX_RRK_LENGTH
#define WMI_FILS_MAX_REALM_LENGTH 256
#define WMI_FILS_MAX_USERNAME_LENGTH 16

/**
 * struct roam_fils_params - Roam FILS params
 * @username: username
 * @username_length: username length
 * @next_erp_seq_num: next ERP sequence number
 * @rrk: RRK
 * @rrk_length: length of @rrk
 * @rik: RIK
 * @rik_length: length of @rik
 * @realm: realm
 * @realm_len: length of @realm
 */
struct roam_fils_params {
	uint8_t username[WMI_FILS_MAX_USERNAME_LENGTH];
	uint32_t username_length;
	uint32_t next_erp_seq_num;
	uint8_t rrk[WMI_FILS_MAX_RRK_LENGTH];
	uint32_t rrk_length;
	uint8_t rik[WMI_FILS_MAX_RIK_LENGTH];
	uint32_t rik_length;
	uint8_t realm[WMI_FILS_MAX_REALM_LENGTH];
	uint32_t realm_len;
};

/* struct roam_offload_scan_params - structure
 *     containing roaming offload scan parameters
 * @is_roam_req_valid: flag to tell whether roam req
 *                     is valid or NULL
 * @mode: stores flags for scan
 * @vdev_id: vdev id
 * @roam_offload_enabled: flag for offload enable
 * @psk_pmk: pre shared key/pairwise master key
 * @pmk_len: length of PMK
 * @prefer_5ghz: prefer select 5G candidate
 * @roam_rssi_cat_gap: gap for every category bucket
 * @select_5ghz_margin: select 5 Ghz margin
 * @krk: KRK
 * @btk: BTK
 * @reassoc_failure_timeout: reassoc failure timeout
 * @rokh_id_length: r0kh id length
 * @rokh_id: r0kh id
 * @roam_key_mgmt_offload_enabled: roam offload flag
 * @auth_mode: authentication mode
 * @fw_okc: use OKC in firmware
 * @fw_pmksa_cache: use PMKSA cache in firmware
 * @is_ese_assoc: flag to determine ese assoc
 * @mdid: mobility domain info
 * @roam_offload_params: roam offload tlv params
 * @assoc_ie_length: Assoc IE length
 * @assoc_ie: Assoc IE buffer
 * @add_fils_tlv: add FILS TLV boolean
 * @roam_fils_params: roam fils params
 */
struct roam_offload_scan_params {
	uint8_t is_roam_req_valid;
	uint32_t mode;
	uint32_t vdev_id;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	uint8_t roam_offload_enabled;
	uint8_t psk_pmk[WMI_ROAM_SCAN_PSK_SIZE];
	uint32_t pmk_len;
	uint8_t prefer_5ghz;
	uint8_t roam_rssi_cat_gap;
	uint8_t select_5ghz_margin;
	uint8_t krk[WMI_KRK_KEY_LEN];
	uint8_t btk[WMI_BTK_KEY_LEN];
	uint32_t reassoc_failure_timeout;
	uint32_t rokh_id_length;
	uint8_t rokh_id[WMI_ROAM_R0KH_ID_MAX_LEN];
	uint8_t roam_key_mgmt_offload_enabled;
	int auth_mode;
	bool fw_okc;
	bool fw_pmksa_cache;
#endif
	bool is_ese_assoc;
	struct mobility_domain_info mdid;
#ifdef CONFIG_MCL
	/* THis is not available in non tlv target.
	* please remove this and replace with a host based
	* structure */
	roam_offload_param roam_offload_params;
#endif
	uint32_t assoc_ie_length;
	uint8_t  assoc_ie[MAX_ASSOC_IE_LENGTH];
	bool add_fils_tlv;
#ifdef WLAN_FEATURE_FILS_SK
	struct roam_fils_params roam_fils_params;
#endif
};

/* struct roam_offload_scan_rssi_params - structure containing
 *              parameters for roam offload scan based on RSSI
 * @rssi_thresh: rssi threshold
 * @rssi_thresh_diff: difference in rssi threshold
 * @hi_rssi_scan_max_count: 5G scan max count
 * @hi_rssi_scan_rssi_delta: 5G scan rssi change threshold value
 * @hi_rssi_scan_rssi_ub: 5G scan upper bound
 * @raise_rssi_thresh_5g: flag to determine penalty and boost thresholds
 * @session_id: vdev id
 * @penalty_threshold_5g: RSSI threshold below which 5GHz RSSI is penalized
 * @boost_threshold_5g: RSSI threshold above which 5GHz RSSI is favored
 * @raise_factor_5g: factor by which 5GHz RSSI is boosted
 * @drop_factor_5g: factor by which 5GHz RSSI is penalized
 * @max_raise_rssi_5g: maximum boost that can be applied to a 5GHz RSSI
 * @max_drop_rssi_5g: maximum penalty that can be applied to a 5GHz RSSI
 * @good_rssi_threshold: RSSI below which roam is kicked in by background
 *                       scan although rssi is still good
 * @roam_earlystop_thres_min: Minimum RSSI threshold value for early stop,
 *                            unit is dB above NF
 * @roam_earlystop_thres_max: Maximum RSSI threshold value for early stop,
 *                            unit is dB above NF
 * @dense_rssi_thresh_offset: dense roam RSSI threshold difference
 * @dense_min_aps_cnt: dense roam minimum APs
 * @initial_dense_status: dense status detected by host
 * @traffic_threshold: dense roam RSSI threshold
 * @bg_scan_bad_rssi_thresh: Bad RSSI threshold to perform bg scan
 * @roam_bad_rssi_thresh_offset_2g: Offset from Bad RSSI threshold for 2G to 5G Roam
 * @bg_scan_client_bitmap: Bitmap used to identify the client scans to snoop
 * @flags: Flags for Background Roaming
 *	Bit 0 : BG roaming enabled when we connect to 2G AP only and roaming to 5G AP only.
 *	Bit 1-31: Reserved
 */
struct roam_offload_scan_rssi_params {
	int8_t rssi_thresh;
	uint8_t rssi_thresh_diff;
	uint32_t hi_rssi_scan_max_count;
	uint32_t hi_rssi_scan_rssi_delta;
	int32_t hi_rssi_scan_rssi_ub;
	int raise_rssi_thresh_5g;
	uint8_t session_id;
	uint32_t penalty_threshold_5g;
	uint32_t boost_threshold_5g;
	uint8_t raise_factor_5g;
	uint8_t drop_factor_5g;
	int max_raise_rssi_5g;
	int max_drop_rssi_5g;
	uint32_t good_rssi_threshold;
	uint32_t roam_earlystop_thres_min;
	uint32_t roam_earlystop_thres_max;
	int dense_rssi_thresh_offset;
	int dense_min_aps_cnt;
	int initial_dense_status;
	int traffic_threshold;
	int32_t rssi_thresh_offset_5g;
	int8_t bg_scan_bad_rssi_thresh;
	uint8_t roam_bad_rssi_thresh_offset_2g;
	uint32_t bg_scan_client_bitmap;
	uint32_t flags;
};

/**
 * struct ap_profile - Structure ap profile to match candidate
 * @flags: flags
 * @rssi_threshold: the value of the the candidate AP should higher by this
 *                  threshold than the rssi of the currrently associated AP
 * @ssid: ssid vlaue to be matched
 * @rsn_authmode: security params to be matched
 * @rsn_ucastcipherset: unicast cipher set
 * @rsn_mcastcipherset: mcast/group cipher set
 * @rsn_mcastmgmtcipherset: mcast/group management frames cipher set
 * @rssi_abs_thresh: the value of the candidate AP should higher than this
 *                   absolute RSSI threshold. Zero means no absolute minimum
 *                   RSSI is required. units are the offset from the noise
 *                   floor in dB
 */
struct ap_profile {
	uint32_t flags;
	uint32_t rssi_threshold;
	struct mac_ssid  ssid;
	uint32_t rsn_authmode;
	uint32_t rsn_ucastcipherset;
	uint32_t rsn_mcastcipherset;
	uint32_t rsn_mcastmgmtcipherset;
	uint32_t rssi_abs_thresh;
};

/**
 * struct rssi_scoring - rssi scoring param to sortlist selected AP
 * @best_rssi_threshold: Roamable AP RSSI equal or better than this threshold,
 *                      full rssi score 100. Units in dBm.
 * @good_rssi_threshold: Below threshold, scoring linear percentage between
 *                      rssi_good_pnt and 100. Units in dBm.
 * @bad_rssi_threshold: Between good and bad rssi threshold, scoring linear
 *                      % between rssi_bad_pcnt and rssi_good_pct in dBm.
 * @good_rssi_pcnt: Used to assigned scoring percentage of each slot between
 *                 best to good rssi threshold. Units in percentage.
 * @bad_rssi_pcnt: Used to assigned scoring percentage of each slot between good
 *                to bad rssi threshold. Unites in percentage.
 * @good_bucket_size : bucket size of slot in good zone
 * @bad_bucket_size : bucket size of slot in bad zone
 * @rssi_pref_5g_rssi_thresh: Below rssi threshold, 5G AP have given preference
 *                           of band percentage. Units in dBm.
 */
struct rssi_scoring {
	int32_t best_rssi_threshold;
	int32_t good_rssi_threshold;
	int32_t  bad_rssi_threshold;
	uint32_t good_rssi_pcnt;
	uint32_t bad_rssi_pcnt;
	uint32_t good_bucket_size;
	uint32_t bad_bucket_size;
	int32_t  rssi_pref_5g_rssi_thresh;
};

/**
 * struct param_slot_scoring - define % score for differents slots for a
 *                             scoring param.
 * @num_slot: number of slots in which the param will be divided.
 *           Max 15. index 0 is used for 'not_present. Num_slot will
 *           equally divide 100. e.g, if num_slot = 4 slot 0 = 0-25%, slot
 *           1 = 26-50% slot 2 = 51-75%, slot 3 = 76-100%
 * @score_pcnt3_to_0: Conatins score percentage for slot 0-3
 *             BITS 0-7   :- the scoring pcnt when not present
 *             BITS 8-15  :- SLOT_1
 *             BITS 16-23 :- SLOT_2
 *             BITS 24-31 :- SLOT_3
 * @score_pcnt7_to_4: Conatins score percentage for slot 4-7
 *             BITS 0-7   :- SLOT_4
 *             BITS 8-15  :- SLOT_5
 *             BITS 16-23 :- SLOT_6
 *             BITS 24-31 :- SLOT_7
 * @score_pcnt11_to_8: Conatins score percentage for slot 8-11
 *             BITS 0-7   :- SLOT_8
 *             BITS 8-15  :- SLOT_9
 *             BITS 16-23 :- SLOT_10
 *             BITS 24-31 :- SLOT_11
 * @score_pcnt15_to_12: Conatins score percentage for slot 12-15
 *             BITS 0-7   :- SLOT_12
 *             BITS 8-15  :- SLOT_13
 *             BITS 16-23 :- SLOT_14
 *             BITS 24-31 :- SLOT_15
 */
struct param_slot_scoring {
	uint32_t num_slot;
	uint32_t score_pcnt3_to_0;
	uint32_t score_pcnt7_to_4;
	uint32_t score_pcnt11_to_8;
	uint32_t score_pcnt15_to_12;
};

/**
 * struct scoring_param - scoring param to sortlist selected AP
 * @disable_bitmap: Each bit will be either allow(0)/disallow(1) to
 *                 considered the roam score param.
 * @rssi_weightage: RSSI weightage out of total score in %
 * @ht_weightage: HT weightage out of total score in %.
 * @vht_weightage: VHT weightage out of total score in %.
 * @he_weightaget: 11ax weightage out of total score in %.
 * @bw_weightage: Bandwidth weightage out of total score in %.
 * @band_weightage: Band(2G/5G) weightage out of total score in %.
 * @nss_weightage: NSS(1x1 / 2x2)weightage out of total score in %.
 * @esp_qbss_weightage: ESP/QBSS weightage out of total score in %.
 * @beamforming_weightage: Beamforming weightage out of total score in %.
 * @pcl_weightage: PCL weightage out of total score in %.
 * @oce_wan_weightage OCE WAN metrics weightage out of total score in %.
 * @bw_index_score: channel BW scoring percentage information.
 *                 BITS 0-7   :- It contains scoring percentage of 20MHz   BW
 *                 BITS 8-15  :- It contains scoring percentage of 40MHz   BW
 *                 BITS 16-23 :- It contains scoring percentage of 80MHz   BW
 *                 BITS 24-31 :- It contains scoring percentage of 1600MHz BW
 *                 The value of each index must be 0-100
 * @band_index_score: band scording percentage information.
 *                   BITS 0-7   :- It contains scoring percentage of 2G
 *                   BITS 8-15  :- It contains scoring percentage of 5G
 *                   BITS 16-23 :- reserved
 *                   BITS 24-31 :- reserved
 *                   The value of each index must be 0-100
 * @nss_index_score: NSS scoring percentage information.
 *                  BITS 0-7   :- It contains scoring percentage of 1x1
 *                  BITS 8-15  :- It contains scoring percentage of 2x2
 *                  BITS 16-23 :- It contains scoring percentage of 3x3
 *                  BITS 24-31 :- It contains scoring percentage of 4x4
 *                  The value of each index must be 0-100
 * @rssi_scoring: RSSI scoring information.
 * @esp_qbss_scoring: ESP/QBSS scoring percentage information
 * @oce_wan_scoring: OCE WAN metrics percentage information
*/
struct scoring_param {
	uint32_t disable_bitmap;
	int32_t rssi_weightage;
	int32_t ht_weightage;
	int32_t vht_weightage;
	int32_t he_weightage;
	int32_t bw_weightage;
	int32_t band_weightage;
	int32_t nss_weightage;
	int32_t esp_qbss_weightage;
	int32_t beamforming_weightage;
	int32_t pcl_weightage;
	int32_t oce_wan_weightage;
	uint32_t bw_index_score;
	uint32_t band_index_score;
	uint32_t nss_index_score;
	struct rssi_scoring rssi_scoring;
	struct param_slot_scoring esp_qbss_scoring;
	struct param_slot_scoring oce_wan_scoring;
};

/**
 * struct ap_profile_params - ap profile params
 * @vdev_id: vdev id
 * @profile: ap profile to match candidate
 * @param: scoring params to short candidate
 */
struct ap_profile_params {
	uint8_t vdev_id;
	struct ap_profile profile;
	struct scoring_param param;
};

/**
 * struct wifi_epno_network - enhanced pno network block
 * @ssid: ssid
 * @rssi_threshold: threshold for considering this SSID as found, required
 *		    granularity for this threshold is 4dBm to 8dBm
 * @flags: WIFI_PNO_FLAG_XXX
 * @auth_bit_field: auth bit field for matching WPA IE
 */
struct wifi_epno_network_params {
	struct mac_ssid  ssid;
	int8_t       rssi_threshold;
	uint8_t      flags;
	uint8_t      auth_bit_field;
};

/**
 * struct wifi_enhanched_pno_params - enhanced pno network params
 * @request_id: request id number
 * @session_id: session_id number
 * @min_5ghz_rssi: minimum 5GHz RSSI for a BSSID to be considered
 * @min_24ghz_rssi: minimum 2.4GHz RSSI for a BSSID to be considered
 * @initial_score_max: maximum score that a network can have before bonuses
 * @current_connection_bonus: only report when there is a network's score this
 *    much higher than the current connection
 * @same_network_bonus: score bonus for all n/w with the same network flag
 * @secure_bonus: score bonus for networks that are not open
 * @band_5ghz_bonus: 5GHz RSSI score bonus (applied to all 5GHz networks)
 * @num_networks: number of ssids
 * @networks: EPNO networks
 */
struct wifi_enhanched_pno_params {
	uint32_t    request_id;
	uint32_t    session_id;
	uint32_t    min_5ghz_rssi;
	uint32_t    min_24ghz_rssi;
	uint32_t    initial_score_max;
	uint32_t    current_connection_bonus;
	uint32_t    same_network_bonus;
	uint32_t    secure_bonus;
	uint32_t    band_5ghz_bonus;
	uint32_t    num_networks;
	struct wifi_epno_network_params networks[];
};

enum {
	WMI_AP_RX_DATA_OFFLOAD             = 0x00,
	WMI_STA_RX_DATA_OFFLOAD            = 0x01,
};

/**
 * enum extscan_configuration_flags - extscan config flags
 * @WMI_EXTSCAN_LP_EXTENDED_BATCHING: extended batching
 */
enum wmi_extscan_configuration_flags {
	WMI_EXTSCAN_LP_EXTENDED_BATCHING = 0x00000001,
};

/**
 * enum extscan_report_events_type - extscan report events type
 * @EXTSCAN_REPORT_EVENTS_BUFFER_FULL: report only when scan history is % full
 * @EXTSCAN_REPORT_EVENTS_EACH_SCAN: report a scan completion event after scan
 * @EXTSCAN_REPORT_EVENTS_FULL_RESULTS: forward scan results
 *		(beacons/probe responses + IEs)
 *		in real time to HAL, in addition to completion events.
 *		Note: To keep backward compatibility,
 *		fire completion events regardless of REPORT_EVENTS_EACH_SCAN.
 * @EXTSCAN_REPORT_EVENTS_NO_BATCH: controls batching,
 *		0 => batching, 1 => no batching
 */
enum wmi_extscan_report_events_type {
	WMI_EXTSCAN_REPORT_EVENTS_BUFFER_FULL   = 0x00,
	WMI_EXTSCAN_REPORT_EVENTS_EACH_SCAN     = 0x01,
	WMI_EXTSCAN_REPORT_EVENTS_FULL_RESULTS  = 0x02,
	WMI_EXTSCAN_REPORT_EVENTS_NO_BATCH      = 0x04,
};

/**
 * struct ipa_offload_control_params - ipa offload parameters
 * @offload_type: ipa offload type
 * @vdev_id: vdev id
 * @enable: ipa offload enable/disable
 */
struct ipa_offload_control_params {
	uint32_t offload_type;
	uint32_t vdev_id;
	uint32_t enable;
};

/**
 * struct extscan_capabilities_params - ext scan capablities
 * @request_id: request_id
 * @session_id: session_id
 */
struct extscan_capabilities_params {
	uint32_t request_id;
	uint8_t session_id;
};

/**
 * struct extscan_capabilities_reset_params - ext scan capablities reset parameter
 * @request_id: request_id
 * @session_id: session_id
 */
struct extscan_capabilities_reset_params {
	uint32_t request_id;
	uint8_t session_id;
};

/**
 * struct extscan_bssid_hotlist_reset_params - ext scan hotlist reset parameter
 * @request_id: request_id
 * @session_id: session_id
 */
struct extscan_bssid_hotlist_reset_params {
	uint32_t request_id;
	uint8_t session_id;
};

/**
 * struct extscan_stop_req_params - ext scan stop parameter
 * @request_id: request_id
 * @session_id: session_id
 */
struct extscan_stop_req_params {
	uint32_t request_id;
	uint8_t session_id;
};

/**
 * struct ap_threshold_params - ap threshold parameter
 * @bssid: mac address
 * @low: low threshold
 * @high: high threshold
 */
struct ap_threshold_params {
	struct qdf_mac_addr bssid;
	int32_t low;
	int32_t high;
};

/**
 * struct extscan_set_sig_changereq_params - ext scan channel parameter
 * @request_id: mac address
 * @session_id: low threshold
 * @rssi_sample_size: Number of samples for averaging RSSI
 * @lostap_sample_size: Number of missed samples to confirm AP loss
 * @min_breaching: Number of APs breaching threshold required for firmware
 * @num_ap: no of scanned ap
 * @ap: ap threshold parameter
 */
struct extscan_set_sig_changereq_params {
	uint32_t request_id;
	uint8_t session_id;
	uint32_t rssi_sample_size;
	uint32_t lostap_sample_size;
	uint32_t min_breaching;
	uint32_t num_ap;
	struct ap_threshold_params ap[WMI_EXTSCAN_MAX_SIGNIFICANT_CHANGE_APS];
};

/**
 * struct extscan_cached_result_params - ext scan cached parameter
 * @request_id: mac address
 * @session_id: low threshold
 * @flush: cached results flush
 */
struct extscan_cached_result_params {
	uint32_t request_id;
	uint8_t session_id;
	bool flush;
};

#define WMI_WLAN_EXTSCAN_MAX_CHANNELS                 36
#define WMI_WLAN_EXTSCAN_MAX_BUCKETS                  16
#define WMI_WLAN_EXTSCAN_MAX_HOTLIST_APS              128
#define WMI_WLAN_EXTSCAN_MAX_SIGNIFICANT_CHANGE_APS   64
#define WMI_EXTSCAN_MAX_HOTLIST_SSIDS            8

/**
 * struct wifi_scan_channelspec_params - wifi scan channel parameter
 * @channel: Frequency in MHz
 * @dwellTimeMs: dwell time
 * @flush: cached results flush
 * @passive: passive scan
 * @chnlClass: channel class
 */
struct wifi_scan_channelspec_params {
	uint32_t channel;
	uint32_t dwellTimeMs;
	bool passive;
	uint8_t chnlClass;
};

/**
 * enum wmi_wifi_band - wifi band
 * @WMI_WIFI_BAND_UNSPECIFIED: unspecified band
 * @WMI_WIFI_BAND_BG: 2.4 GHz
 * @WMI_WIFI_BAND_A: 5 GHz without DFS
 * @WMI_WIFI_BAND_ABG: 2.4 GHz + 5 GHz; no DFS
 * @WMI_WIFI_BAND_A_DFS_ONLY: 5 GHz DFS only
 * @WMI_WIFI_BAND_A_WITH_DFS: 5 GHz with DFS
 * @WMI_WIFI_BAND_ABG_WITH_DFS: 2.4 GHz + 5 GHz with DFS
 * @WMI_WIFI_BAND_MAX: max range
 */
enum wmi_wifi_band {
	WMI_WIFI_BAND_UNSPECIFIED,
	WMI_WIFI_BAND_BG = 1,
	WMI_WIFI_BAND_A = 2,
	WMI_WIFI_BAND_ABG = 3,
	WMI_WIFI_BAND_A_DFS_ONLY = 4,
	/* 5 is reserved */
	WMI_WIFI_BAND_A_WITH_DFS = 6,
	WMI_WIFI_BAND_ABG_WITH_DFS = 7,
	/* Keep it last */
	WMI_WIFI_BAND_MAX
};

/**
 * struct wifi_scan_bucket_params - wifi scan bucket spec
 * @bucket: bucket identifier
 * @band: wifi band
 * @period: Desired period, in millisecond; if this is too
 *		low, the firmware should choose to generate results as fast as
 *		it can instead of failing the command byte
 *		for exponential backoff bucket this is the min_period
 * @reportEvents: 0 => normal reporting (reporting rssi history
 *		only, when rssi history buffer is % full)
 *		1 => same as 0 + report a scan completion event after scanning
 *		this bucket
 *		2 => same as 1 + forward scan results
 *		(beacons/probe responses + IEs) in real time to HAL
 * @max_period: if max_period is non zero or different than period,
 *		then this bucket is an exponential backoff bucket and
 *		the scan period will grow exponentially as per formula:
 *		actual_period(N) = period ^ (N/(step_count+1)) to a
 *		maximum period of max_period
 * @exponent: for exponential back off bucket: multiplier:
 *		new_period = old_period * exponent
 * @step_count: for exponential back off bucket, number of scans performed
 *		at a given period and until the exponent is applied
 * @numChannels: channels to scan; these may include DFS channels
 *		Note that a given channel may appear in multiple buckets
 * @min_dwell_time_active: per bucket minimum active dwell time
 * @max_dwell_time_active: per bucket maximum active dwell time
 * @min_dwell_time_passive: per bucket minimum passive dwell time
 * @max_dwell_time_passive: per bucket maximum passive dwell time
 * @channels: Channel list
 */
struct wifi_scan_bucket_params {
	uint8_t         bucket;
	enum wmi_wifi_band   band;
	uint32_t        period;
	uint32_t        reportEvents;
	uint32_t        max_period;
	uint32_t        exponent;
	uint32_t        step_count;
	uint32_t        numChannels;
	uint32_t        min_dwell_time_active;
	uint32_t        max_dwell_time_active;
	uint32_t        min_dwell_time_passive;
	uint32_t        max_dwell_time_passive;
	struct wifi_scan_channelspec_params channels[WMI_WLAN_EXTSCAN_MAX_CHANNELS];
};

/**
 * struct wifi_scan_cmd_req_params - wifi scan command request params
 * @basePeriod: base timer period
 * @maxAPperScan: max ap per scan
 * @report_threshold_percent: report threshold
 *	in %, when buffer is this much full, wake up host
 * @report_threshold_num_scans: report threshold number of scans
 *	in number of scans, wake up host after these many scans
 * @requestId: request id
 * @sessionId: session id
 * @numBuckets: number of buckets
 * @min_dwell_time_active: per bucket minimum active dwell time
 * @max_dwell_time_active: per bucket maximum active dwell time
 * @min_dwell_time_passive: per bucket minimum passive dwell time
 * @max_dwell_time_passive: per bucket maximum passive dwell time
 * @configuration_flags: configuration flags
 * @extscan_adaptive_dwell_mode: adaptive dwelltime mode for extscan
 * @buckets: buckets array
 */
struct wifi_scan_cmd_req_params {
	uint32_t basePeriod;
	uint32_t maxAPperScan;

	uint32_t report_threshold_percent;
	uint32_t report_threshold_num_scans;

	uint32_t requestId;
	uint8_t  sessionId;
	uint32_t numBuckets;

	uint32_t min_dwell_time_active;
	uint32_t max_dwell_time_active;
	uint32_t min_dwell_time_passive;
	uint32_t max_dwell_time_passive;
	uint32_t configuration_flags;
	enum wmi_dwelltime_adaptive_mode extscan_adaptive_dwell_mode;
	struct wifi_scan_bucket_params buckets[WMI_WLAN_EXTSCAN_MAX_BUCKETS];
};

#define WMI_CFG_VALID_CHANNEL_LIST_LEN    100
/* Occupied channel list remains static */
#define WMI_CHANNEL_LIST_STATIC                   1
/* Occupied channel list can be learnt after init */
#define WMI_CHANNEL_LIST_DYNAMIC_INIT             2
/* Occupied channel list can be learnt after flush */
#define WMI_CHANNEL_LIST_DYNAMIC_FLUSH            3
/* Occupied channel list can be learnt after update */
#define WMI_CHANNEL_LIST_DYNAMIC_UPDATE           4

/**
 * struct plm_req_params - plm req parameter
 * @diag_token: Dialog token
 * @meas_token: measurement token
 * @num_bursts: total number of bursts
 * @burst_int: burst interval in seconds
 * @meas_duration:in TU's,STA goes off-ch
 * @burst_len: no of times the STA should cycle through PLM ch list
 * @desired_tx_pwr: desired tx power
 * @mac_addr: MC dest addr
 * @plm_num_ch: channel numbers
 * @plm_ch_list: channel list
 * @session_id: session id
 * @enable:  enable/disable
 */
struct plm_req_params {
	uint16_t diag_token;
	uint16_t meas_token;
	uint16_t num_bursts;
	uint16_t burst_int;
	uint16_t meas_duration;
	/* no of times the STA should cycle through PLM ch list */
	uint8_t burst_len;
	int8_t desired_tx_pwr;
	struct qdf_mac_addr mac_addr;
	/* no of channels */
	uint8_t plm_num_ch;
	/* channel numbers */
	uint8_t plm_ch_list[WMI_CFG_VALID_CHANNEL_LIST_LEN];
	uint8_t session_id;
	bool enable;
};

#define MAX_SSID_ALLOWED_LIST    4
#define MAX_BSSID_AVOID_LIST     16
#define MAX_BSSID_FAVORED      16
#define MAX_RSSI_AVOID_BSSID_LIST 10

/**
 * struct mac_ts_info_tfc - mac ts info parameters
 * @burstSizeDefn: burst size
 * @reserved: reserved
 * @ackPolicy: ack policy
 * @psb: psb
 * @aggregation: aggregation
 * @accessPolicy: access policy
 * @direction: direction
 * @tsid: direction
 * @trafficType: traffic type
 */
struct mac_ts_info_tfc {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t burstSizeDefn:1;
	uint8_t reserved:7;
#else
	uint8_t reserved:7;
	uint8_t burstSizeDefn:1;
#endif

#ifndef ANI_LITTLE_BIT_ENDIAN
	uint16_t ackPolicy:2;
	uint16_t userPrio:3;
	uint16_t psb:1;
	uint16_t aggregation:1;
	uint16_t accessPolicy:2;
	uint16_t direction:2;
	uint16_t tsid:4;
	uint16_t trafficType:1;
#else
	uint16_t trafficType:1;
	uint16_t tsid:4;
	uint16_t direction:2;
	uint16_t accessPolicy:2;
	uint16_t aggregation:1;
	uint16_t psb:1;
	uint16_t userPrio:3;
	uint16_t ackPolicy:2;
#endif
} qdf_packed;

/**
 * struct mac_ts_info_sch - mac ts info schedule parameters
 * @rsvd: reserved
 * @schedule: schedule bit
 */
struct mac_ts_info_sch {
#ifndef ANI_LITTLE_BIT_ENDIAN
	uint8_t rsvd:7;
	uint8_t schedule:1;
#else
	uint8_t schedule:1;
	uint8_t rsvd:7;
#endif
} qdf_packed;

/**
 * struct mac_ts_info_sch - mac ts info schedule parameters
 * @traffic: mac tfc parameter
 * @schedule: mac schedule parameters
 */
struct mac_ts_info {
	struct mac_ts_info_tfc traffic;
	struct mac_ts_info_sch schedule;
} qdf_packed;

/**
 * struct mac_tspec_ie - mac ts spec
 * @type: type
 * @length: length
 * @tsinfo: tsinfo
 * @nomMsduSz: nomMsduSz
 * @maxMsduSz: maxMsduSz
 * @minSvcInterval: minSvcInterval
 * @maxSvcInterval: maxSvcInterval
 * @inactInterval: inactInterval
 * @suspendInterval: suspendInterval
 * @svcStartTime: svcStartTime
 * @minDataRate: minDataRate
 * @meanDataRate: meanDataRate
 * @peakDataRate: peakDataRate
 * @maxBurstSz: maxBurstSz
 * @delayBound: delayBound
 * @minPhyRate: minPhyRate
 * @surplusBw: surplusBw
 * @mediumTime: mediumTime
 */
struct mac_tspec_ie {
	uint8_t type;
	uint8_t length;
	struct mac_ts_info tsinfo;
	uint16_t nomMsduSz;
	uint16_t maxMsduSz;
	uint32_t minSvcInterval;
	uint32_t maxSvcInterval;
	uint32_t inactInterval;
	uint32_t suspendInterval;
	uint32_t svcStartTime;
	uint32_t minDataRate;
	uint32_t meanDataRate;
	uint32_t peakDataRate;
	uint32_t maxBurstSz;
	uint32_t delayBound;
	uint32_t minPhyRate;
	uint16_t surplusBw;
	uint16_t mediumTime;
} qdf_packed;

/**
 * struct add_ts_param - ADDTS related parameters
 * @staIdx: station index
 * @tspecIdx: TSPEC handler uniquely identifying a TSPEC for a STA in a BSS
 * @tspec: tspec value
 * @status: CDF status
 * @sessionId: session id
 * @tsm_interval: TSM interval period passed from UMAC to WMI
 * @setRICparams: RIC parameters
 * @sme_session_id: sme session id
 */
struct add_ts_param {
	uint16_t staIdx;
	uint16_t tspecIdx;
	struct mac_tspec_ie tspec;
	QDF_STATUS status;
	uint8_t sessionId;
#ifdef FEATURE_WLAN_ESE
	uint16_t tsm_interval;
#endif /* FEATURE_WLAN_ESE */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	uint8_t setRICparams;
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
	uint8_t sme_session_id;
};

/**
 * struct delts_req_info - DELTS request parameter
 * @tsinfo: ts info
 * @tspec: ts spec
 * @wmeTspecPresent: wme ts spec flag
 * @wsmTspecPresent: wsm ts spec flag
 * @lleTspecPresent: lle ts spec flag
 */
struct delts_req_info {
	struct mac_ts_info tsinfo;
	struct mac_tspec_ie tspec;
	uint8_t wmeTspecPresent:1;
	uint8_t wsmTspecPresent:1;
	uint8_t lleTspecPresent:1;
};

/**
 * struct del_ts_params - DELTS related parameters
 * @staIdx: station index
 * @tspecIdx: TSPEC identifier uniquely identifying a TSPEC for a STA in a BSS
 * @bssId: BSSID
 * @sessionId: session id
 * @userPrio: user priority
 * @delTsInfo: DELTS info
 * @setRICparams: RIC parameters
 */
struct del_ts_params {
	uint16_t staIdx;
	uint16_t tspecIdx;
	uint8_t bssId[IEEE80211_ADDR_LEN];
	uint8_t sessionId;
	uint8_t userPrio;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	struct delts_req_info delTsInfo;
	uint8_t setRICparams;
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */
};

/**
 * struct ll_stats_clear_params - ll stats clear parameter
 * @req_id: request id
 * @sta_id: sta id
 * @stats_clear_mask: stats clear mask
 * @stop_req: stop request
 */
struct ll_stats_clear_params {
	uint32_t req_id;
	uint8_t sta_id;
	uint32_t stats_clear_mask;
	uint8_t stop_req;
};

/**
 * struct ll_stats_set_params - ll stats get parameter
 * @req_id: request id
 * @sta_id: sta id
 * @mpdu_size_threshold: mpdu sixe threshold
 * @aggressive_statistics_gathering: aggressive_statistics_gathering
 */
struct ll_stats_set_params {
	uint32_t req_id;
	uint8_t sta_id;
	uint32_t mpdu_size_threshold;
	uint32_t aggressive_statistics_gathering;
};

/**
 * struct ll_stats_get_params - ll stats parameter
 * @req_id: request id
 * @sta_id: sta id
 * @param_id_mask: param is mask
 */
struct ll_stats_get_params {
	uint32_t req_id;
	uint8_t sta_id;
	uint32_t param_id_mask;
};


/**
 * struct link_status_params - link stats parameter
 * @msg_type: message type is same as the request type
 * @msg_len: length of the entire request
 * @link_status: wme ts spec flag
 * @session_id: wsm ts spec flag
 */
struct link_status_params {
	uint16_t msg_type;
	uint16_t msg_len;
	uint8_t link_status;
	uint8_t session_id;
};

/**
 * struct dhcp_stop_ind_params - DHCP Stop indication message
 * @msgtype: message type is same as the request type
 * @msglen: length of the entire request
 * @device_mode: Mode of the device(ex:STA, AP)
 * @adapter_macaddr: MAC address of the adapter
 * @peer_macaddr: MAC address of the connected peer
 */
struct dhcp_stop_ind_params {
	uint16_t msgtype;
	uint16_t msglen;
	uint8_t device_mode;
	struct qdf_mac_addr adapter_macaddr;
	struct qdf_mac_addr peer_macaddr;
};

/**
 * struct aggr_add_ts_param - ADDTS parameters
 * @staIdx: station index
 * @tspecIdx: TSPEC handler uniquely identifying a TSPEC for a STA in a BSS
 * @tspec: tspec value
 * @status: CDF status
 * @sessionId: session id
 */
struct aggr_add_ts_param {
	uint16_t staIdx;
	uint16_t tspecIdx;
	struct mac_tspec_ie tspec[WMI_QOS_NUM_AC_MAX];
	QDF_STATUS status[WMI_QOS_NUM_AC_MAX];
	uint8_t sessionId;
};


/**
 * struct wlm_latency_level_param - WLM parameters
 * @wlm_latency_level: wlm latency level to set
 *  0 - normal, 1 - moderate, 2 - low, 3 - ultralow
 * @wlm_latency_flags: wlm latency flags to set
 *  |31  12|  11  |  10  |9    8|7    6|5    4|3    2|  1  |  0  |
 *  +------+------+------+------+------+------+------+-----+-----+
 *  | RSVD | SSLP | CSLP | RSVD | Roam | RSVD | DWLT | DFS | SUP |
 *  +------+-------------+-------------+-------------------------+
 *  |  WAL |      PS     |     Roam    |         Scan            |
 *
 *  bit 0: Avoid scan request from HLOS if setting
 *  bit 1: Skip DFS channel SCAN if setting
 *  bit 2-3: Define policy of dwell time/duration for each foreign channel
 *     (b2 b3)
 *     (0  0 ): Default scan dwell time
 *     (0  1 ): Reserve
 *     (1  0 ): Shrink off channel dwell time
 *     (1  1 ): Reserve
 *  bit 4-5: Reserve for scan
 *  bit 6-7: Define roaming policy
 *     (b6 b7)
 *     (0  0 ): Default roaming behavior, allow roaming in all scenarios
 *     (0  1 ): Disallow all roaming
 *     (1  0 ): Allow roaming when final bmissed
 *     (1  1 ): Reserve
 *  bit 8-9: Reserve for roaming
 *  bit 10: Disable css power collapse if setting
 *  bit 11: Disable sys sleep if setting
 *  bit 12-31: Reserve for future useage
 * @vdev_id: vdev id
 */
struct wlm_latency_level_param {
	uint16_t wlm_latency_level;
	uint32_t wlm_latency_flags;
	uint16_t vdev_id;
};

#define    WMI_MAX_FILTER_TEST_DATA_LEN       8
#define    WMI_MAX_NUM_MULTICAST_ADDRESS    240
#define    WMI_MAX_NUM_FILTERS               20
#define    WMI_MAX_NUM_TESTS_PER_FILTER      10

/**
 * enum packet_filter_type - packet filter type
 * @WMI_RCV_FILTER_TYPE_INVALID: invalid type
 * @WMI_RCV_FILTER_TYPE_FILTER_PKT: filter packet type
 * @WMI_RCV_FILTER_TYPE_BUFFER_PKT: buffer packet type
 * @WMI_RCV_FILTER_TYPE_MAX_ENUM_SIZE: max enum size
 */
enum packet_filter_type {
	WMI_RCV_FILTER_TYPE_INVALID,
	WMI_RCV_FILTER_TYPE_FILTER_PKT,
	WMI_RCV_FILTER_TYPE_BUFFER_PKT,
	WMI_RCV_FILTER_TYPE_MAX_ENUM_SIZE
};

/**
 * enum packet_protocol_type - packet protocol type
 * @WMI_FILTER_HDR_TYPE_INVALID: invalid type
 * @WMI_FILTER_HDR_TYPE_MAC: mac type
 * @WMI_FILTER_HDR_TYPE_ARP: trp type
 * @WMI_FILTER_HDR_TYPE_IPV4: ipv4 type
 * @WMI_FILTER_HDR_TYPE_IPV6: ipv6 type
 * @WMI_FILTER_HDR_TYPE_UDP: udp type
 * @WMI_FILTER_HDR_TYPE_MAX: max type
 */
enum packet_protocol_type {
	WMI_FILTER_HDR_TYPE_INVALID,
	WMI_FILTER_HDR_TYPE_MAC,
	WMI_FILTER_HDR_TYPE_ARP,
	WMI_FILTER_HDR_TYPE_IPV4,
	WMI_FILTER_HDR_TYPE_IPV6,
	WMI_FILTER_HDR_TYPE_UDP,
	WMI_FILTER_HDR_TYPE_MAX
};

/**
 * enum packet_filter_comp_type - packet filter comparison type
 * @WMI_FILTER_CMP_TYPE_INVALID: invalid type
 * @WMI_FILTER_CMP_TYPE_EQUAL: type equal
 * @WMI_FILTER_CMP_TYPE_MASK_EQUAL: mask equal
 * @WMI_FILTER_CMP_TYPE_NOT_EQUAL: type not equal
 * @WMI_FILTER_CMP_TYPE_MASK_NOT_EQUAL: mask not equal
 * @WMI_FILTER_CMP_TYPE_MAX: max type
 */
enum packet_filter_comp_type {
	WMI_FILTER_CMP_TYPE_INVALID,
	WMI_FILTER_CMP_TYPE_EQUAL,
	WMI_FILTER_CMP_TYPE_MASK_EQUAL,
	WMI_FILTER_CMP_TYPE_NOT_EQUAL,
	WMI_FILTER_CMP_TYPE_MASK_NOT_EQUAL,
	WMI_FILTER_CMP_TYPE_MAX
};

/**
 * struct rcv_pkt_filter_params - recieve packet filter parameters
 * @protocolLayer - protocol layer
 * @cmpFlag - comparison flag
 * @dataLength - data length
 * @dataOffset - data offset
 * @reserved - resserved
 * @compareData - compare data
 * @dataMask - data mask
 */
struct rcv_pkt_filter_params {
	enum packet_protocol_type protocolLayer;
	enum packet_filter_comp_type cmpFlag;
	uint16_t dataLength;
	uint8_t dataOffset;
	uint8_t reserved;
	uint8_t compareData[WMI_MAX_FILTER_TEST_DATA_LEN];
	uint8_t dataMask[WMI_MAX_FILTER_TEST_DATA_LEN];
};

/**
 * struct rcv_pkt_filter_config - recieve packet filter info
 * @filterId - filter id
 * @filterType - filter type
 * @numFieldParams - no of fields
 * @coalesceTime - reserved parameter
 * @self_macaddr - self mac address
 * @bssid - Bssid of the connected AP
 * @paramsData - data parameter
 */
struct rcv_pkt_filter_config {
	uint8_t filterId;
	enum packet_filter_type filterType;
	uint32_t numFieldParams;
	uint32_t coalesceTime;
	struct qdf_mac_addr self_macaddr;
	struct qdf_mac_addr bssid;
	struct rcv_pkt_filter_params paramsData[WMI_MAX_NUM_TESTS_PER_FILTER];
};

/**
 * struct vdev_ie_info_param - IE info
 * @vdev_id - vdev for which the IE is being sent
 * @ie_id - ID of the IE
 * @length - length of the IE data
 * @data - IE data
 *
 * This structure is used to store the IE information.
 */
struct vdev_ie_info_param {
	uint32_t vdev_id;
	uint32_t ie_id;
	uint32_t length;
	uint32_t ie_source;
	uint32_t band;
	uint8_t *data;
};

#define WMI_MAX_NUM_FW_SEGMENTS 4

/**
 * struct fw_dump_seg_req_param - individual segment details
 * @seg_id - segment id.
 * @seg_start_addr_lo - lower address of the segment.
 * @seg_start_addr_hi - higher address of the segment.
 * @seg_length - length of the segment.
 * @dst_addr_lo - lower address of the destination buffer.
 * @dst_addr_hi - higher address of the destination buffer.
 *
 * This structure carries the information to firmware about the
 * individual segments. This structure is part of firmware memory
 * dump request.
 */
struct fw_dump_seg_req_param {
	uint8_t seg_id;
	uint32_t seg_start_addr_lo;
	uint32_t seg_start_addr_hi;
	uint32_t seg_length;
	uint32_t dst_addr_lo;
	uint32_t dst_addr_hi;
};

/**
 * struct fw_dump_req_param - firmware memory dump request details.
 * @request_id - request id.
 * @num_seg - requested number of segments.
 * @fw_dump_seg_req - individual segment information.
 *
 * This structure carries information about the firmware
 * memory dump request.
 */
struct fw_dump_req_param {
	uint32_t request_id;
	uint32_t num_seg;
	struct fw_dump_seg_req_param segment[WMI_MAX_NUM_FW_SEGMENTS];
};

#define WMI_TDLS_MAX_SUPP_CHANNELS       128
#define WMI_TDLS_MAX_SUPP_OPER_CLASSES   32
#define WMI_2_4_GHZ_MAX_FREQ  3000

/**
 * struct tdls_update_ch_params - channel parameters
 * @chanId: ID of the channel
 * @pwr: power level
 * @dfsSet: is dfs supported or not
 * @half_rate: is the channel operating at 10MHz
 * @quarter_rate: is the channel operating at 5MHz
 */
struct tdls_update_ch_params {
	uint8_t chanId;
	uint8_t pwr;
	bool dfsSet;
	bool half_rate;
	bool quarter_rate;
};

/**
 * struct tdls_peer_cap_params - TDLS peer capablities parameters
 * @isPeerResponder: is peer responder or not
 * @peerUapsdQueue: peer uapsd queue
 * @peerMaxSp: peer max SP value
 * @peerBuffStaSupport: peer buffer sta supported or not
 * @peerOffChanSupport: peer offchannel support
 * @peerCurrOperClass: peer current operating class
 * @selfCurrOperClass: self current operating class
 * @peerChanLen: peer channel length
 * @peerChan: peer channel list
 * @peerOperClassLen: peer operating class length
 * @peerOperClass: peer operating class
 * @prefOffChanNum: peer offchannel number
 * @prefOffChanBandwidth: peer offchannel bandwidth
 * @opClassForPrefOffChan: operating class for offchannel
 */
struct tdls_peer_cap_params {
	uint8_t isPeerResponder;
	uint8_t peerUapsdQueue;
	uint8_t peerMaxSp;
	uint8_t peerBuffStaSupport;
	uint8_t peerOffChanSupport;
	uint8_t peerCurrOperClass;
	uint8_t selfCurrOperClass;
	uint8_t peerChanLen;
	struct tdls_update_ch_params peerChan[WMI_TDLS_MAX_SUPP_CHANNELS];
	uint8_t peerOperClassLen;
	uint8_t peerOperClass[WMI_TDLS_MAX_SUPP_OPER_CLASSES];
	uint8_t prefOffChanNum;
	uint8_t prefOffChanBandwidth;
	uint8_t opClassForPrefOffChan;
};

/**
 * struct tdls_peer_state_params - TDLS peer state parameters
 * @vdevId: vdev id
 * @peerMacAddr: peer mac address
 * @peerCap: peer capabality
 */
struct tdls_peer_state_params {
	uint32_t vdevId;
	uint8_t peerMacAddr[IEEE80211_ADDR_LEN];
	uint32_t peerState;
	struct tdls_peer_cap_params peerCap;
};

/**
 * struct wmi_tdls_params - TDLS parameters
 * @vdev_id: vdev id
 * @tdls_state: TDLS state
 * @notification_interval_ms: notification inerval
 * @tx_discovery_threshold: tx discovery threshold
 * @tx_teardown_threshold: tx teardown threashold
 * @rssi_teardown_threshold: RSSI teardown threshold
 * @rssi_delta: RSSI delta
 * @tdls_options: TDLS options
 * @peer_traffic_ind_window: raffic indication window
 * @peer_traffic_response_timeout: traffic response timeout
 * @puapsd_mask: uapsd mask
 * @puapsd_inactivity_time: uapsd inactivity time
 * @puapsd_rx_frame_threshold: uapsd rx frame threshold
 * @teardown_notification_ms: tdls teardown notification interval
 * @tdls_peer_kickout_threshold: tdls packet threshold for
 *    peer kickout operation
 */
struct wmi_tdls_params {
	uint32_t vdev_id;
	uint32_t tdls_state;
	uint32_t notification_interval_ms;
	uint32_t tx_discovery_threshold;
	uint32_t tx_teardown_threshold;
	int32_t rssi_teardown_threshold;
	int32_t rssi_delta;
	uint32_t tdls_options;
	uint32_t peer_traffic_ind_window;
	uint32_t peer_traffic_response_timeout;
	uint32_t puapsd_mask;
	uint32_t puapsd_inactivity_time;
	uint32_t puapsd_rx_frame_threshold;
	uint32_t teardown_notification_ms;
	uint32_t tdls_peer_kickout_threshold;
};

#ifndef CONVERGED_TDLS_ENABLE
/**
 * struct tdls_chan_switch_params - channel switch parameter structure
 * @vdev_id: vdev ID
 * @peer_mac_addr: Peer mac address
 * @tdls_off_ch_bw_offset: Target off-channel bandwitdh offset
 * @tdls_off_ch: Target Off Channel
 * @oper_class: Operating class for target channel
 * @is_responder: Responder or initiator
 */
struct tdls_channel_switch_params {
	uint32_t    vdev_id;
	uint8_t     peer_mac_addr[IEEE80211_ADDR_LEN];
	uint16_t    tdls_off_ch_bw_offset;
	uint8_t     tdls_off_ch;
	uint8_t     tdls_sw_mode;
	uint8_t     oper_class;
	uint8_t     is_responder;
};
#endif

/**
 * struct dhcp_offload_info_params - dhcp offload parameters
 * @vdev_id: request data length
 * @dhcp_offload_enabled: dhcp offload enabled
 * @dhcp_client_num: dhcp client no
 * @dhcp_srv_addr: dhcp server ip
 */
struct dhcp_offload_info_params {
	uint32_t vdev_id;
	bool dhcp_offload_enabled;
	uint32_t dhcp_client_num;
	uint32_t dhcp_srv_addr;
};

/**
 * struct nan_req_params - NAN request params
 * @request_data_len: request data length
 * @request_data: request data
 */
struct nan_req_params {
	uint16_t request_data_len;
	uint8_t request_data[];
};


/**
 * struct app_type2_params - app type2parameter
 * @vdev_id: vdev id
 * @rc4_key: rc4 key
 * @rc4_key_len: rc4 key length
 * @ip_id: NC id
 * @ip_device_ip: NC IP addres
 * @ip_server_ip: Push server IP address
 * @tcp_src_port: NC TCP port
 * @tcp_dst_port: Push server TCP port
 * @tcp_seq: tcp sequence
 * @tcp_ack_seq: tcp ack sequence
 * @keepalive_init: Initial ping interval
 * @keepalive_min: Minimum ping interval
 * @keepalive_max: Maximum ping interval
 * @keepalive_inc: Increment of ping interval
 * @gateway_mac: gateway mac address
 * @tcp_tx_timeout_val: tcp tx timeout value
 * @tcp_rx_timeout_val: tcp rx timeout value
 */
struct app_type2_params {
	uint8_t vdev_id;
	uint8_t rc4_key[16];
	uint32_t rc4_key_len;
	/** ip header parameter */
	uint32_t ip_id;
	uint32_t ip_device_ip;
	uint32_t ip_server_ip;
	/** tcp header parameter */
	uint16_t tcp_src_port;
	uint16_t tcp_dst_port;
	uint32_t tcp_seq;
	uint32_t tcp_ack_seq;
	uint32_t keepalive_init;
	uint32_t keepalive_min;
	uint32_t keepalive_max;
	uint32_t keepalive_inc;
	struct qdf_mac_addr gateway_mac;
	uint32_t tcp_tx_timeout_val;
	uint32_t tcp_rx_timeout_val;
};

/**
 * struct app_type1_params - app type1 parameter
 * @vdev_id: vdev id
 * @wakee_mac_addr: mac address
 * @identification_id: identification id
 * @password: password
 * @id_length: id length
 * @pass_length: password length
 */
struct app_type1_params {
	uint8_t vdev_id;
	struct qdf_mac_addr wakee_mac_addr;
	uint8_t identification_id[8];
	uint8_t password[16];
	uint32_t id_length;
	uint32_t pass_length;
};

/**
 * enum wmi_ext_wow_type - wow type
 * @WMI_EXT_WOW_TYPE_APP_TYPE1: only enable wakeup for app type1
 * @WMI_EXT_WOW_TYPE_APP_TYPE2: only enable wakeup for app type2
 * @WMI_EXT_WOW_TYPE_APP_TYPE1_2: enable wakeup for app type1&2
 */
enum wmi_ext_wow_type {
	WMI_EXT_WOW_TYPE_APP_TYPE1,
	WMI_EXT_WOW_TYPE_APP_TYPE2,
	WMI_EXT_WOW_TYPE_APP_TYPE1_2,
};

/**
 * struct ext_wow_params - ext wow parameters
 * @vdev_id: vdev id
 * @type: wow type
 * @wakeup_pin_num: wake up gpio no
 */
struct ext_wow_params {
	uint8_t vdev_id;
	enum wmi_ext_wow_type type;
	uint32_t wakeup_pin_num;
};

/**
 * struct stats_ext_params - ext stats request
 * @vdev_id: vdev id
 * @request_data_len: request data length
 * @request_data: request data
 */
struct stats_ext_params {
	uint32_t vdev_id;
	uint32_t request_data_len;
	uint8_t request_data[];
};

#define WMI_PERIODIC_TX_PTRN_MAX_SIZE 1536
/**
 * struct periodic_tx_pattern - periodic tx pattern
 * @mac_address: MAC Address for the adapter
 * @ucPtrnId: Pattern ID
 * @ucPtrnSize: Pattern size
 * @usPtrnIntervalMs: in ms
 * @ucPattern: Pattern buffer
 */
struct periodic_tx_pattern {
	struct qdf_mac_addr mac_address;
	uint8_t ucPtrnId;
	uint16_t ucPtrnSize;
	uint32_t usPtrnIntervalMs;
	uint8_t ucPattern[WMI_PERIODIC_TX_PTRN_MAX_SIZE];
};

#define WMI_GTK_OFFLOAD_KEK_BYTES       64
#define WMI_GTK_OFFLOAD_KCK_BYTES       16
#define WMI_GTK_OFFLOAD_ENABLE          0
#define WMI_GTK_OFFLOAD_DISABLE         1

/**
 * struct flashing_req_params - led flashing parameter
 * @reqId: request id
 * @pattern_id: pattern identifier. 0: disconnected 1: connected
 * @led_x0: led flashing parameter0
 * @led_x1: led flashing parameter1
 */
struct flashing_req_params {
	uint32_t req_id;
	uint32_t pattern_id;
	uint32_t led_x0;
	uint32_t led_x1;
};

#define MAX_MEM_CHUNKS 32
/**
 * struct wmi_host_mem_chunk - host memory chunk structure
 * @vaddr: Pointer to virtual address
 * @paddr: Physical address
 * @memctx: qdf memory context for mapped address.
 * @len: length of chunk
 * @req_id: request id from target
 */
struct wmi_host_mem_chunk {
	uint32_t *vaddr;
	uint32_t paddr;
	qdf_dma_mem_context(memctx);
	uint32_t len;
	uint32_t req_id;
};

/**
 * struct target_resource_config - Resource config given to target
 *              This structure is union of wmi_resource_config defined
 *              by both TLV and non-TLV target.
 */
struct target_resource_config {
	uint32_t num_vdevs;
	uint32_t num_peers;
	uint32_t num_active_peers;
	uint32_t num_offload_peers;
	uint32_t num_offload_reorder_buffs;
	uint32_t num_peer_keys;
	uint32_t num_tids;
	uint32_t ast_skid_limit;
	uint32_t tx_chain_mask;
	uint32_t rx_chain_mask;
	uint32_t rx_timeout_pri[4];
	uint32_t rx_decap_mode;
	uint32_t scan_max_pending_req;
	uint32_t bmiss_offload_max_vdev;
	uint32_t roam_offload_max_vdev;
	uint32_t roam_offload_max_ap_profiles;
	uint32_t num_mcast_groups;
	uint32_t num_mcast_table_elems;
	uint32_t mcast2ucast_mode;
	uint32_t tx_dbg_log_size;
	uint32_t num_wds_entries;
	uint32_t dma_burst_size;
	uint32_t mac_aggr_delim;
	uint32_t rx_skip_defrag_timeout_dup_detection_check;
	uint32_t vow_config;
	uint32_t gtk_offload_max_vdev;
	uint32_t num_msdu_desc; /* Number of msdu desc */
	uint32_t max_frag_entries;
    /* End common */

	/* Added in MCL */
	uint32_t num_tdls_vdevs;
	uint32_t num_tdls_conn_table_entries;
	uint32_t beacon_tx_offload_max_vdev;
	uint32_t num_multicast_filter_entries;
	uint32_t num_wow_filters;
	uint32_t num_keep_alive_pattern;
	uint32_t keep_alive_pattern_size;
	uint32_t max_tdls_concurrent_sleep_sta;
	uint32_t max_tdls_concurrent_buffer_sta;
	uint32_t wmi_send_separate;
	uint32_t num_ocb_vdevs;
	uint32_t num_ocb_channels;
	uint32_t num_ocb_schedules;
};

/**
 * struct wmi_wifi_start_log - Structure to store the params sent to start/
 * stop logging
 * @name:          Attribute which indicates the type of logging like per packet
 *                 statistics, connectivity etc.
 * @verbose_level: Verbose level which can be 0,1,2,3
 * @flag:          Flag field for future use
 */
struct wmi_wifi_start_log {
	uint32_t ring_id;
	uint32_t verbose_level;
	uint32_t flag;
};

/**
 * struct wmi_pcl_list - Format of PCL
 * @pcl_list: List of preferred channels
 * @weight_list: Weights of the PCL
 * @pcl_len: Number of channels in the PCL
 */
struct wmi_pcl_list {
	uint8_t pcl_list[128];
	uint8_t weight_list[128];
	uint32_t pcl_len;
};

/**
 * struct wmi_pcl_chan_weights - Params to get the valid weighed list
 * @pcl_list: Preferred channel list already sorted in the order of preference
 * @pcl_len: Length of the PCL
 * @saved_chan_list: Valid channel list updated as part of
 * WMA_UPDATE_CHAN_LIST_REQ
 * @saved_num_chan: Length of the valid channel list
 * @weighed_valid_list: Weights of the valid channel list. This will have one
 * to one mapping with valid_chan_list. FW expects channel order and size to be
 * as per the list provided in WMI_SCAN_CHAN_LIST_CMDID.
 * @weight_list: Weights assigned by policy manager
 */
struct wmi_pcl_chan_weights {
	uint8_t pcl_list[MAX_NUM_CHAN];
	uint32_t pcl_len;
	uint8_t saved_chan_list[MAX_NUM_CHAN];
	uint32_t saved_num_chan;
	uint8_t weighed_valid_list[MAX_NUM_CHAN];
	uint8_t weight_list[MAX_NUM_CHAN];
};

/**
 * struct wmi_hw_mode_params - HW mode params
 * @mac0_tx_ss: MAC0 Tx spatial stream
 * @mac0_rx_ss: MAC0 Rx spatial stream
 * @mac1_tx_ss: MAC1 Tx spatial stream
 * @mac1_rx_ss: MAC1 Rx spatial stream
 * @mac0_bw: MAC0 bandwidth
 * @mac1_bw: MAC1 bandwidth
 * @dbs_cap: DBS capabality
 * @agile_dfs_cap: Agile DFS capabality
 */
struct wmi_hw_mode_params {
	uint8_t mac0_tx_ss;
	uint8_t mac0_rx_ss;
	uint8_t mac1_tx_ss;
	uint8_t mac1_rx_ss;
	uint8_t mac0_bw;
	uint8_t mac1_bw;
	uint8_t dbs_cap;
	uint8_t agile_dfs_cap;
};

/**
 * struct wmi_dual_mac_config - Dual MAC configuration
 * @scan_config: Scan configuration
 * @fw_mode_config: FW mode configuration
 * @set_dual_mac_cb: Callback function to be executed on response to the command
 */
struct wmi_dual_mac_config {
	uint32_t scan_config;
	uint32_t fw_mode_config;
	void *set_dual_mac_cb;
};

/**
 * struct ssid_hotlist_param - param for SSID Hotlist
 * @ssid: SSID which is being hotlisted
 * @band: Band in which the given SSID should be scanned
 * @rssi_low: Low bound on RSSI
 * @rssi_high: High bound on RSSI
 */
struct ssid_hotlist_param {
	struct mac_ssid ssid;
	uint8_t band;
	int32_t rssi_low;
	int32_t rssi_high;
};

/**
 * struct rssi_disallow_bssid - Structure holding Rssi based avoid candidate
 * @bssid: BSSID of the AP
 * @remaining_duration: remaining disallow duration in ms
 * @expected_rssi: RSSI at which STA can initate in dBm
 */
struct rssi_disallow_bssid {
	struct qdf_mac_addr bssid;
	uint32_t remaining_duration;
	int8_t expected_rssi;
};


/**
 * struct roam_scan_filter_params - Structure holding roaming scan
 *                                  parameters
 * @op_bitmap:                bitmap to determine reason of roaming
 * @session_id:               vdev id
 * @num_bssid_black_list:     The number of BSSID's that we should
 *                            avoid connecting to. It is like a
 *                            blacklist of BSSID's.
 * @num_ssid_white_list:      The number of SSID profiles that are
 *                            in the Whitelist. When roaming, we
 *                            consider the BSSID's with this SSID
 *                            also for roaming apart from the connected one's
 * @num_bssid_preferred_list: Number of BSSID's which have a preference over
 *                            others
 * @bssid_avoid_list:         Blacklist SSID's
 * @ssid_allowed_list:        Whitelist SSID's
 * @bssid_favored:            Favorable BSSID's
 * @bssid_favored_factor:     RSSI to be added to this BSSID to prefer it
 * @lca_disallow_config_present: LCA [Last Connected AP] disallow config present
 * @disallow_duration:        How long LCA AP will be disallowed before it
 *                            can be a roaming candidate again, in seconds
 * @rssi_channel_penalization:How much RSSI will be penalized if candidate(s)
 *                            are found in the same channel as disallowed AP's,
 *                            in units of db
 * @num_disallowed_aps:       How many APs the target should maintain in its
 *                            LCA list
 *
 * This structure holds all the key parameters related to
 * initial connection and roaming connections.
 */

struct roam_scan_filter_params {
	uint32_t op_bitmap;
	uint8_t session_id;
	uint32_t num_bssid_black_list;
	uint32_t num_ssid_white_list;
	uint32_t num_bssid_preferred_list;
	struct qdf_mac_addr bssid_avoid_list[MAX_BSSID_AVOID_LIST];
	struct mac_ssid ssid_allowed_list[MAX_SSID_ALLOWED_LIST];
	struct qdf_mac_addr bssid_favored[MAX_BSSID_FAVORED];
	uint8_t bssid_favored_factor[MAX_BSSID_FAVORED];
	uint8_t lca_disallow_config_present;
	uint32_t disallow_duration;
	uint32_t rssi_channel_penalization;
	uint32_t num_disallowed_aps;
	uint32_t num_rssi_rejection_ap;
	struct rssi_disallow_bssid rssi_rejection_ap[MAX_RSSI_AVOID_BSSID_LIST];
};

#define WMI_MAX_HLP_IE_LEN 2048
/**
 * struct hlp_params - HLP info params
 * @vdev_id: vdev id
 * @hlp_ie_len: HLP IE length
 * @hlp_ie: HLP IE
 */
struct hlp_params {
	uint8_t vdev_id;
	uint32_t  hlp_ie_len;
	uint8_t hlp_ie[WMI_MAX_HLP_IE_LEN];
};

#define WMI_UNIFIED_MAX_PMKID_LEN   16
#define WMI_UNIFIED_MAX_PMK_LEN     64

/**
 * struct wmi_unified_pmk_cache - used to set del pmkid cache
 * @tlv_header: TLV header, TLV tag and len; tag equals WMITLV_TAG_ARRAY_UINT32
 * @pmk_len: PMK len
 *	for big-endian hosts, manual endian conversion will be needed to keep
 *	the array values in their original order in spite of the automatic
 *	byte-swap applied to WMI messages during download
 * @pmk: PMK array
 * @pmkid_len: PMK ID Len
 * @pmkid: PMK ID Array
 * @bssid: BSSID
 * @ssid: SSID
 * @cache_id: PMK Cache ID
 * @cat_flag: whether (bssid) or (ssid,cache_id) is valid
 * @action_flag: add/delete the entry
 */
struct wmi_unified_pmk_cache {
	A_UINT32            tlv_header;
	A_UINT32            pmk_len;
	A_UINT8             session_id;
	A_UINT8             pmk[WMI_UNIFIED_MAX_PMK_LEN];
	A_UINT32            pmkid_len;
	A_UINT8             pmkid[WMI_UNIFIED_MAX_PMKID_LEN];
	wmi_host_mac_addr   bssid;
	struct mac_ssid     ssid;
	A_UINT32            cache_id;
	A_UINT32            cat_flag;
	A_UINT32            action_flag;
};


/**
 * struct ssid_hotlist_request_params - set SSID hotlist request struct
 * @request_id: ID of the request
 * @session_id: ID of the session
 * @lost_ssid_sample_size: Number of consecutive scans in which the SSID
 *	must not be seen in order to consider the SSID "lost"
 * @ssid_count: Number of valid entries in the @ssids array
 * @ssids: Array that defines the SSIDs that are in the hotlist
 */
struct ssid_hotlist_request_params {
	uint32_t request_id;
	uint8_t session_id;
	uint32_t lost_ssid_sample_size;
	uint32_t ssid_count;
	struct ssid_hotlist_param ssids[WMI_EXTSCAN_MAX_HOTLIST_SSIDS];
};

/**
 * struct wmi_unit_test_cmd - unit test command parameters
 * @vdev_id: vdev id
 * @module_id: module id
 * @num_args: number of arguments
 * @diag_token: dialog token, which identifies the transaction.
 *              this number is generated by wifitool and may be used to
 *              identify the transaction in the event path
 * @args: arguments
 */
struct wmi_unit_test_cmd {
	uint32_t vdev_id;
	uint32_t module_id;
	uint32_t num_args;
	uint32_t diag_token;
	uint32_t args[WMI_UNIT_TEST_MAX_NUM_ARGS];
};

/**
 * struct wmi_roam_invoke_cmd - roam invoke command
 * @vdev_id: vdev id
 * @bssid: mac address
 * @channel: channel
 * @frame_len: frame length, includs mac header, fixed params and ies
 * @frame_buf: buffer contaning probe response or beacon
 * @is_same_bssid: flag to indicate if roaming is requested for same bssid
 */
struct wmi_roam_invoke_cmd {
	uint32_t vdev_id;
	uint8_t bssid[IEEE80211_ADDR_LEN];
	uint32_t channel;
	uint32_t frame_len;
	uint8_t *frame_buf;
	uint8_t is_same_bssid;
};

/**
 * struct ext_scan_setbssi_hotlist_params - set hotlist request
 * @requestId: request identifier
 * @sessionId: session identifier
 * @lost_ap_sample_size: number of samples to confirm AP loss
 * @numAp: Number of hotlist APs
 * @ap: hotlist APs
 */
struct ext_scan_setbssi_hotlist_params {
	uint32_t  requestId;
	uint8_t   sessionId;

	uint32_t  lost_ap_sample_size;
	uint32_t  numAp;
	struct ap_threshold_params ap[WMI_WLAN_EXTSCAN_MAX_HOTLIST_APS];
};

/**
 * struct host_mem_req - Host memory request paramseters request by target
 * @req_id: Request id to identify the request.
 * @unit_size: Size of single unit requested.
 * @num_unit_info: Memory chunk info
 * @num_units: number of units requested.
 */
typedef struct {
	uint32_t	req_id;
	uint32_t	unit_size;
	uint32_t	num_unit_info;
	uint32_t	num_units;
} host_mem_req;

#define WMI_HOST_DSCP_MAP_MAX	(64)

/**
 * struct wmi_host_ext_resource_config - Extended resource config
 * @host_platform_config: Host plaform configuration.
 * @fw_featuew_bitmap: FW feature requested bitmap.
 */
typedef struct {
	uint32_t host_platform_config;

#define WMI_HOST_FW_FEATURE_LTEU_SUPPORT	           0x0001
#define WMI_HOST_FW_FEATURE_COEX_GPIO_SUPPORT	       0x0002
#define WMI_HOST_FW_FEATURE_AUX_RADIO_SPECTRAL_INTF	   0x0004
#define WMI_HOST_FW_FEATURE_AUX_RADIO_CHAN_LOAD_INTF   0x0008
#define WMI_HOST_FW_FEATURE_BSS_CHANNEL_INFO_64	       0x0010
#define WMI_HOST_FW_FEATURE_PEER_STATS                 0x0020
#define WMI_HOST_FW_FEATURE_VDEV_STATS                 0x0040
	/**
	* @brief fw_feature_bitmask - Enable/Disable features in FW
	* @details
	*  The bits in fw_feature_bitmask are used as shown by the masks below:
	*	0x0001 - LTEU Config enable/disable
	*	0x0002 - COEX GPIO Config enable/disable
	*	0x0004 - Aux Radio enhancement for spectral scan enable/disable
	*	0x0008 - Aux Radio enhancement for chan load scan enable/disable
	*	0x0010 - BSS channel info stats enable/disable
	* The features in question are enabled by setting
	* the feature's bit to 1,
	*  or disabled by setting the feature's bit to 0.
	*/
	uint32_t fw_feature_bitmap;

	/* WLAN priority GPIO number
	 * The target uses a GPIO pin to indicate when it is transmitting
	 * high-priority traffic (e.g. beacon, management, or AC_VI) or
	 * low-priority traffic (e.g. AC_BE, AC_BK).  The HW uses this
	 * WLAN GPIO pin to determine whether to abort WLAN tx in favor of
	 * BT activity.
	 * Which GPIO is used for this WLAN tx traffic priority specification
	 * varies between platforms, so the host needs to indicate to the
	 * target which GPIO to use.
	 */
	uint32_t wlan_priority_gpio;

	/* Host will notify target which coex algorithm has to be
	 * enabled based on HW, FW capability and device tree config.
	 * Till now the coex algorithms were target specific. Now the
	 * same target can choose between multiple coex algorithms
	 * depending on device tree config on host. For backward
	 * compatibility, version support will have option 0 and will
	 * rely on FW compile time flags to decide the coex version
	 * between VERSION_1, VERSION_2 and VERSION_3. Version info is
	 * mandatory from VERSION_4 onwards for any new coex algorithms.
	 *
	 * 0 = no version support
	 * 1 = COEX_VERSION_1 (3 wire coex)
	 * 2 = COEX_VERSION_2 (2.5 wire coex)
	 * 3 = COEX_VERSION_3 (2.5 wire coex+duty cycle)
	 * 4 = COEX_VERSION_4 (4 wire coex)
	 */
	uint32_t coex_version;

	/* There are multiple coex implementations on FW to support different
	 * hardwares. Since the coex algos are mutually exclusive, host will
	 * use below fields to send GPIO info to FW and these GPIO pins will
	 * have different usages depending on the feature enabled. This is to
	 * avoid adding multiple GPIO fields here for different features.
	 *
	 * COEX VERSION_4 (4 wire coex) :
	 * 4 wire coex feature uses 1 common input request line from BT/ZB/
	 * Thread which interrupts the WLAN target processor directly, 1 input
	 * priority line from BT and ZB each, 1 output line to grant access to
	 * requesting IOT subsystem. WLAN uses the input priority line to
	 * identify the requesting IOT subsystem. Request is granted based on
	 * IOT interface priority and WLAN traffic. GPIO pin usage is as below:
	 * coex_gpio_pin_1 = BT PRIORITY INPUT GPIO
	 * coex_gpio_pin_2 = ZIGBEE PRIORITY INPUT GPIO
	 * coex_gpio_pin_3 = GRANT OUTPUT GPIO
	 * when a BT active interrupt is raised, WLAN reads
	 * BT and ZB priority input GPIO pins to compare against the coex
	 * priority table and accordingly sets the grant output GPIO to give
	 * access to requesting IOT subsystem.
	 */
	uint32_t coex_gpio_pin_1;
	uint32_t coex_gpio_pin_2;
	uint32_t coex_gpio_pin_3;

	/* add new members here */
} wmi_host_ext_resource_config;

/**
 * struct set_neighbour_rx_params - Neighbour RX params
 * @vdev_id: vdev id
 * @idx: index of param
 * @action: action
 * @type: Type of param
 */
struct set_neighbour_rx_params {
	uint8_t vdev_id;
	uint32_t idx;
	uint32_t action;
	uint32_t type;
};

/**
 * struct set_fwtest_params - FW test params
 * @arg: FW param id
 * @value: value
 */
struct set_fwtest_params {
	uint32_t arg;
	uint32_t value;
};

/**
 * struct set_custom_aggr_size_params - custom aggr size params
 * @vdev_id      : vdev id
 * @tx_aggr_size : TX aggr size
 * @rx_aggr_size : RX aggr size
 * @enable_bitmap: Bitmap for aggr size check
 */
struct set_custom_aggr_size_params {
	uint32_t  vdev_id;
	uint32_t tx_aggr_size;
	uint32_t rx_aggr_size;
	uint32_t ac:2,
		 aggr_type:1,
		 tx_aggr_size_disable:1,
		 rx_aggr_size_disable:1,
		 tx_ac_enable:1,
		 reserved:26;
};

/**
 * enum wmi_host_custom_aggr_type_t: custon aggregate type
 * @WMI_HOST_CUSTOM_AGGR_TYPE_AMPDU: A-MPDU aggregation
 * @WMI_HOST_CUSTOM_AGGR_TYPE_AMSDU: A-MSDU aggregation
 * @WMI_HOST_CUSTOM_AGGR_TYPE_MAX: Max type
 */
enum wmi_host_custom_aggr_type_t {
	WMI_HOST_CUSTOM_AGGR_TYPE_AMPDU = 0,
	WMI_HOST_CUSTOM_AGGR_TYPE_AMSDU = 1,
	WMI_HOST_CUSTOM_AGGR_TYPE_MAX,
};

/*
 * msduq_update_params - MSDUQ update param structure
 * @tid_num: TID number
 * @msduq_update_mask: update bit mask
 * @qdepth_thresh_value: threshold value for the queue depth
 */

#define QDEPTH_THRESH_MAX_UPDATES 1

typedef struct {
	uint32_t tid_num;
	uint32_t msduq_update_mask;
	uint32_t qdepth_thresh_value;
} msduq_update_params;

/**
 * struct set_qdepth_thresh_params - MSDU Queue Depth Threshold Params
 * @vdev_id: vdev id
 * @pdev_id: pdev id
 * @mac_addr: MAC address
 * @num_of_msduq_updates: holds the number of tid updates
 */

struct set_qdepth_thresh_params {
	uint32_t pdev_id;
	uint32_t vdev_id;
	uint8_t mac_addr[IEEE80211_ADDR_LEN];
	uint32_t num_of_msduq_updates;
	msduq_update_params update_params[QDEPTH_THRESH_MAX_UPDATES];
};



/**
 * struct config_ratemask_params - ratemask config parameters
 * @vdev_id: vdev id
 * @type: Type
 * @lower32: Lower 32 bits
 * @higher32: Hogher 32 bits
 */
struct config_ratemask_params {
	uint8_t vdev_id;
	uint8_t type;
	uint32_t lower32;
	uint32_t higher32;
};

/**
 * struct config_fils_params - FILS config params
 * @vdev_id:  vdev id
 * @fd_period:  0 - Disabled, non-zero - Period in ms (mili seconds)
 */
struct config_fils_params {
	uint8_t vdev_id;
	uint32_t fd_period;
};

/**
 * struct peer_add_wds_entry_params - WDS peer entry add params
 * @dest_addr: Pointer to destination macaddr
 * @peer_addr: Pointer to peer mac addr
 * @flags: flags
 * @vdev_id: Vdev id
 */
struct peer_add_wds_entry_params {
	const uint8_t *dest_addr;
	uint8_t *peer_addr;
	uint32_t flags;
	uint32_t vdev_id;
};

/**
 * struct peer_del_wds_entry_params - WDS peer entry del params
 * @dest_addr: Pointer to destination macaddr
 * @vdev_id: Vdev id
 */
struct peer_del_wds_entry_params {
	uint8_t *dest_addr;
	uint32_t vdev_id;
};

/**
 * struct set_bridge_mac_addr_params - set bridge MAC addr params
 * @dest_addr: Pointer to bridge macaddr
 */
struct set_bridge_mac_addr_params {
	uint8_t *bridge_addr;
};

/**
 * struct peer_updatewds_entry_params - WDS peer entry update params
 * @wds_macaddr: Pointer to destination macaddr
 * @peer_add: Pointer to peer mac addr
 * @flags: flags
 * @vdev_id: Vdev id
 */
struct peer_update_wds_entry_params {
	uint8_t *wds_macaddr;
	uint8_t *peer_macaddr;
	uint32_t flags;
	uint32_t vdev_id;
};

/**
 * struct set_ps_mode_params - PS mode params
 * @vdev_id: vdev id
 * @psmode: PS mode
 */
struct set_ps_mode_params {
	uint8_t vdev_id;
	uint8_t psmode;
};

/**
 * @struct tt_level_config - Set Thermal throttlling config
 * @tmplwm: Temperature low water mark
 * @tmphwm: Temperature high water mark
 * @dcoffpercent: dc off percentage
 * @priority: priority
 */
typedef struct {
	uint32_t tmplwm;
	uint32_t tmphwm;
	uint32_t dcoffpercent;
	uint32_t priority;
} tt_level_config;

/**
 * struct thermal_mitigation_params - Thermal mitigation params
 * @enable: Enable/Disable Thermal mitigation
 * @dc: DC
 * @dc_per_event: DC per event
 * @tt_level_config: TT level config params
 */
struct thermal_mitigation_params {
	uint32_t pdev_id;
	uint32_t enable;
	uint32_t dc;
	uint32_t dc_per_event;
	tt_level_config levelconf[THERMAL_LEVELS];
};

/**
 * struct smart_ant_enable_params - Smart antenna params
 * @enable: Enable/Disable
 * @mode: SA mode
 * @rx_antenna: RX antenna config
 * @gpio_pin : GPIO pin config
 * @gpio_func : GPIO function config
 */
struct smart_ant_enable_params {
	uint32_t enable;
	uint32_t mode;
	uint32_t rx_antenna;
	uint32_t gpio_pin[WMI_HAL_MAX_SANTENNA];
	uint32_t gpio_func[WMI_HAL_MAX_SANTENNA];
	uint32_t pdev_id;
};

/**
 * struct smart_ant_rx_ant_params - RX antenna params
 * @antenna: RX antenna
 */
struct smart_ant_rx_ant_params {
	uint32_t antenna;
	uint32_t pdev_id;
};

/**
 * struct smart_ant_tx_ant_params - TX antenna param
 * @antenna_array: Antenna arry
 * @vdev_id: VDEV id
 */
struct smart_ant_tx_ant_params {
	uint32_t *antenna_array;
	uint8_t vdev_id;
};

/**
 * struct smart_ant_training_info_params - SA training params
 * @vdev_id: VDEV id
 * @rate_array: Rates array
 * @antenna_array: Antenna array
 * @numpkts: num packets for training
 */
struct smart_ant_training_info_params {
	uint8_t vdev_id;
	uint32_t *rate_array;
	uint32_t *antenna_array;
	uint32_t numpkts;
};

/**
 * struct smart_ant_node_config_params - SA node config params
 * @vdev_id: VDEV id
 * @cmd_id: Command id
 * @args_count: Arguments count
 */
struct smart_ant_node_config_params {
	uint8_t vdev_id;
	uint32_t cmd_id;
	uint16_t args_count;
	uint32_t *args_arr;
};
/**
 * struct smart_ant_enable_tx_feedback_params - SA tx feeback params
 * @enable: Enable TX feedback for SA
 */
struct smart_ant_enable_tx_feedback_params {
	int enable;
};

/**
 * struct vdev_spectral_configure_params - SPectral config params
 * @vdev_id: VDEV id
 * @count: count
 * @period: period
 * @spectral_pri: Spectral priority
 * @fft_size: FFT size
 * @gc_enable: GC enable
 * @restart_enable: restart enabled
 * @noise_floor_ref: Noise floor reference
 * @init_delay: Init delays
 * @nb_tone_thr: NB tone threshold
 * @str_bin_thr: STR BIN threshold
 * @wb_rpt_mode: WB BIN threshold
 * @rssi_rpt_mode: RSSI report mode
 * @rssi_thr: RSSI threshold
 * @pwr_format: Power format
 * @rpt_mode: Report mdoe
 * @bin_scale: BIN scale
 * @dbm_adj: DBM adjust
 * @chn_mask: chain mask
 */
struct vdev_spectral_configure_params {
	uint8_t vdev_id;
	uint16_t count;
	uint16_t period;
	uint16_t spectral_pri;
	uint16_t fft_size;
	uint16_t gc_enable;
	uint16_t restart_enable;
	uint16_t noise_floor_ref;
	uint16_t init_delay;
	uint16_t nb_tone_thr;
	uint16_t str_bin_thr;
	uint16_t wb_rpt_mode;
	uint16_t rssi_rpt_mode;
	uint16_t rssi_thr;
	uint16_t pwr_format;
	uint16_t rpt_mode;
	uint16_t bin_scale;
	uint16_t dbm_adj;
	uint16_t chn_mask;
};

/**
 * struct vdev_spectral_enable_params - Spectral enabled params
 * @vdev_id: VDEV id
 * @active_valid: Active valid
 * @active: active
 * @enabled_valid: Enabled valid
 * @enabled: enabled
 */
struct vdev_spectral_enable_params {
	uint8_t vdev_id;
	uint8_t active_valid;
	uint8_t active;
	uint8_t enabled_valid;
	uint8_t enabled;
};

/**
 * struct pdev_set_regdomain_params - PDEV set reg domain params
 * @currentRDinuse: Current Reg domain
 * @currentRD2G: Current Reg domain 2G
 * @currentRD5G: Current Reg domain 5G
 * @ctl_2G: CTL 2G
 * @ctl_5G: CTL 5G
 * @dfsDomain: DFS domain
 * @pdev_id: pdev_id
 */
struct pdev_set_regdomain_params {
	uint16_t currentRDinuse;
	uint16_t currentRD2G;
	uint16_t currentRD5G;
	uint32_t ctl_2G;
	uint32_t ctl_5G;
	uint8_t dfsDomain;
	uint32_t pdev_id;
};

/**
 * struct set_quiet_mode_params - Set quiet mode params
 * @enabled: Enabled
 * @period: Quite period
 * @intval: Quite interval
 * @duration: Quite duration
 * @offset: offset
 */
struct set_quiet_mode_params {
	uint8_t enabled;
	uint8_t period;
	uint16_t intval;
	uint16_t duration;
	uint16_t offset;
};

/**
 * struct set_beacon_filter_params - Set beacon filter params
 * @vdev_id: VDEV id
 * @ie: Pointer to IE fields
 */
struct set_beacon_filter_params {
	uint8_t vdev_id;
	uint32_t *ie;
};

/**
 * struct remove_beacon_filter_params - Remove beacon filter params
 * @vdev_id: VDEV id
 */
struct remove_beacon_filter_params {
	uint8_t vdev_id;
};

/**
 * struct mgmt_params - Mgmt params
 * @vdev_id: vdev id
 * @buf_len: lenght of frame buffer
 * @wbuf: frame buffer
 */
struct mgmt_params {
	int vdev_id;
	uint32_t buf_len;
	qdf_nbuf_t wbuf;
};

/**
 * struct addba_clearresponse_params - Addba clear response params
 * @vdev_id: VDEV id
 */
struct addba_clearresponse_params {
	uint8_t vdev_id;
};

/**
 * struct addba_send_params - ADDBA send params
 * @vdev_id: vdev id
 * @tidno: TID
 * @buffersize: buffer size
 */
struct addba_send_params {
	uint8_t vdev_id;
	uint8_t tidno;
	uint16_t buffersize;
};

/**
 * struct delba_send_params - DELBA send params
 * @vdev_id: vdev id
 * @tidno: TID
 * @initiator: initiator
 * @reasoncode: reason code
 */
struct delba_send_params {
	uint8_t vdev_id;
	uint8_t tidno;
	uint8_t initiator;
	uint16_t reasoncode;
};
/**
 * struct addba_setresponse_arams - Set ADDBA response params
 * @vdev_id: vdev id
 * @tidno: TID
 * @statuscode: status code in response
 */
struct addba_setresponse_params {
	uint8_t vdev_id;
	uint8_t tidno;
	uint16_t statuscode;
};

/**
 * struct singleamsdu_params - Single AMSDU params
 * @vdev_id: vdev is
 * @tidno: TID
 */
struct singleamsdu_params {
	uint8_t vdev_id;
	uint8_t tidno;
};

/**
 * struct set_qbosst_params - Set QBOOST params
 * @vdev_id: vdev id
 * @value: value
 */
struct set_qboost_params {
	uint8_t vdev_id;
	uint32_t value;
};

/**
 * struct mu_scan_params - MU scan params
 * @id: id
 * @type: type
 * @duration: Duration
 * @lteu_tx_power: LTEU tx power
 */
struct mu_scan_params {
	uint8_t id;
	uint8_t type;
	uint32_t duration;
	uint32_t lteu_tx_power;
	uint32_t rssi_thr_bssid;
	uint32_t rssi_thr_sta;
	uint32_t rssi_thr_sc;
	uint32_t plmn_id;
	uint32_t alpha_num_bssid;
};

/**
 * struct lteu_config_params - LTEU config params
 * @lteu_gpio_start: start MU/AP scan after GPIO toggle
 * @lteu_num_bins: no. of elements in the following arrays
 * @use_actual_nf: whether to use the actual NF obtained or a hardcoded one
 * @lteu_weight: weights for MU algo
 * @lteu_thresh: thresholds for MU algo
 * @lteu_gamma:  gamma's for MU algo
 * @lteu_scan_timeout: timeout in ms to gpio toggle
 * @alpha_num_ssid: alpha for num active bssid calculation
 * @wifi_tx_power: Wifi Tx power
 */
struct lteu_config_params {
	uint8_t lteu_gpio_start;
	uint8_t lteu_num_bins;
	uint8_t use_actual_nf;
	uint32_t lteu_weight[LTEU_MAX_BINS];
	uint32_t lteu_thresh[LTEU_MAX_BINS];
	uint32_t lteu_gamma[LTEU_MAX_BINS];
	uint32_t lteu_scan_timeout;
	uint32_t alpha_num_bssid;
	uint32_t wifi_tx_power;
	uint32_t allow_err_packets;
};

struct wmi_macaddr_t {
	/** upper 4 bytes of  MAC address */
	uint32_t mac_addr31to0;
	/** lower 2 bytes of  MAC address */
	uint32_t mac_addr47to32;
};

/**
 * struct atf_peer_info - ATF peer info params
 * @peer_macaddr: peer mac addr
 * @percentage_peer: percentage of air time for this peer
 * @vdev_id: Associated vdev id
 * @pdev_id: Associated pdev id
 */
typedef struct {
	struct wmi_macaddr_t peer_macaddr;
	uint32_t percentage_peer;
	uint32_t vdev_id;
	uint32_t pdev_id;
} atf_peer_info;

/**
 * struct bwf_peer_info_t - BWF peer info params
 * @peer_macaddr: peer mac addr
 * @throughput: Throughput
 * @max_airtime: Max airtime
 * @priority: Priority level
 * @reserved: Reserved array
 * @vdev_id: Associated vdev id
 * @pdev_id: Associated pdev id
 */
typedef struct {
	struct wmi_macaddr_t peer_macaddr;
	uint32_t     throughput;
	uint32_t     max_airtime;
	uint32_t     priority;
	uint32_t     reserved[4];
	uint32_t     vdev_id;
	uint32_t     pdev_id;
} bwf_peer_info;

/**
 * struct set_bwf_params - BWF params
 * @num_peers: number of peers
 * @atf_peer_info: BWF peer info
 */
struct set_bwf_params {
	uint32_t num_peers;
	bwf_peer_info peer_info[1];
};

/**
 * struct atf_peer_ext_info - ATF peer ext info params
 * @peer_macaddr: peer mac address
 * @group_index: group index
 * @atf_index_reserved: ATF index rsvd
 * @vdev_id: Associated vdev id
 * @pdev_id: Associated pdev id
 */
typedef struct {
	struct wmi_macaddr_t peer_macaddr;
	uint32_t group_index;
	uint32_t atf_index_reserved;
	uint16_t vdev_id;
	uint16_t pdev_id;
} atf_peer_ext_info;

/**
 * struct set_atf_params - ATF params
 * @num_peers: number of peers
 * @atf_peer_info: ATF peer info
 */
struct set_atf_params {
	uint32_t num_peers;
	atf_peer_info peer_info[ATF_ACTIVED_MAX_CLIENTS];
};

/**
 * struct atf_peer_request_params - ATF peer req params
 * @num_peers: number of peers
 * @atf_peer_ext_info: ATF peer ext info
 */
struct atf_peer_request_params {
	uint32_t num_peers;
	atf_peer_ext_info peer_ext_info[ATF_ACTIVED_MAX_CLIENTS];
};

/**
 * struct atf_group_info - ATF group info params
 * @percentage_group: Percentage AT for group
 * @atf_group_units_reserved: ATF group information
 * @pdev_id: Associated pdev id
 */
typedef struct {
	uint32_t percentage_group;
	uint32_t atf_group_units_reserved;
	uint32_t pdev_id;
} atf_group_info;

/**
 * struct atf_grouping_params - ATF grouping params
 * @num_groups: number of groups
 * @group_inf: Group informaition
 */
struct atf_grouping_params {
	uint32_t num_groups;
	atf_group_info group_info[ATF_ACTIVED_MAX_ATFGROUPS];
};

/**
 * struct wlan_profile_params - WLAN profile params
 * @param_id: param id
 * @profile_id: profile id
 * @enable: enable
 */
struct wlan_profile_params {
	uint32_t param_id;
	uint32_t profile_id;
	uint32_t enable;
};

/* struct ht_ie_params - HT IE params
 * @ie_len: IE length
 * @ie_data: pointer to IE data
 * @tx_streams: Tx streams supported for this HT IE
 * @rx_streams: Rx streams supported for this HT IE
 */
struct ht_ie_params {
	uint32_t ie_len;
	uint8_t *ie_data;
	uint32_t tx_streams;
	uint32_t rx_streams;
};

/* struct vht_ie_params - VHT IE params
 * @ie_len: IE length
 * @ie_data: pointer to IE data
 * @tx_streams: Tx streams supported for this VHT IE
 * @rx_streams: Rx streams supported for this VHT IE
 */
struct vht_ie_params {
	uint32_t ie_len;
	uint8_t *ie_data;
	uint32_t tx_streams;
	uint32_t rx_streams;
};

/**
 * struct wmi_host_wmeParams - WME params
 * @wmep_acm: ACM paramete
 * @wmep_aifsn:	AIFSN parameters
 * @wmep_logcwmin: cwmin in exponential form
 * @wmep_logcwmax: cwmax in exponential form
 * @wmep_txopLimit: txopLimit
 * @wmep_noackPolicy: No-Ack Policy: 0=ack, 1=no-ack
 */
struct wmi_host_wmeParams {
	u_int8_t	wmep_acm;
	u_int8_t	wmep_aifsn;
	u_int8_t	wmep_logcwmin;
	u_int8_t	wmep_logcwmax;
	u_int16_t   wmep_txopLimit;
	u_int8_t	wmep_noackPolicy;
};

/**
 * struct wmm_update_params - WMM update params
 * @wmep_array: WME params for each AC
 */
struct wmm_update_params {
	struct wmi_host_wmeParams *wmep_array;
};

/**
 * struct wmi_host_wmevParams - WME params
 * @wmep_acm: ACM paramete
 * @wmep_aifsn:	AIFSN parameters
 * @wmep_logcwmin: cwmin in exponential form
 * @wmep_logcwmax: cwmax in exponential form
 * @wmep_txopLimit: txopLimit
 * @wmep_noackPolicy: No-Ack Policy: 0=ack, 1=no-ack
 */
struct wmi_host_wme_vparams {
	u_int32_t	acm;
	u_int32_t	aifs;
	u_int32_t	cwmin;
	u_int32_t	cwmax;
	union {
		u_int32_t txoplimit;
		u_int32_t mu_edca_timer;
	};
	u_int32_t	noackpolicy;
};

/**
 * struct ant_switch_tbl_params - Antenna switch table params
 * @ant_ctrl_common1: ANtenna control common param 1
 * @ant_ctrl_common2: Antenna control commn param 2
 */
struct ant_switch_tbl_params {
	uint32_t ant_ctrl_common1;
	uint32_t ant_ctrl_common2;
	uint32_t pdev_id;
	uint32_t antCtrlChain;
};

/**
 * struct ratepwr_table_params - Rate power table params
 * @ratepwr_tbl: pointer to rate power table
 * @ratepwr_len: rate power table len
 */
struct ratepwr_table_params {
	uint8_t *ratepwr_tbl;
	uint16_t ratepwr_len;
};

/**
 * struct ctl_table_params - Ctl table params
 * @ctl_array: pointer to ctl array
 * @ctl_cmd_len: ctl command length
 * @is_acfg_ctl: is acfg_ctl table
 */
struct ctl_table_params {
	uint8_t *ctl_array;
	uint16_t ctl_cmd_len;
	uint32_t target_type;
	bool is_2g;
	uint32_t ctl_band;
	uint32_t pdev_id;
};

/**
 * struct mimogain_table_params - MIMO gain table params
 * @array_gain: pointer to array gain table
 * @tbl_len: table length
 * @multichain_gain_bypass: bypass multichain gain
 */
struct mimogain_table_params {
	uint8_t *array_gain;
	uint16_t tbl_len;
	bool multichain_gain_bypass;
	uint32_t pdev_id;
};

/**
 * struct ratepwr_chainmask_params - Rate power chainmask params
 * @ratepwr_chain_tbl: pointer to ratepwr chain table
 * @num_rate: number of rate in table
 * @pream_type: preamble type
 * @ops: ops
 */
struct ratepwr_chainmsk_params {
	uint32_t *ratepwr_chain_tbl;
	uint16_t num_rate;
	uint8_t pream_type;
	uint8_t ops;
};

struct macaddr_params {
	uint8_t *macaddr;
};

/**
 * struct acparams_params - acparams config structure
 * @ac: AC to configure
 * @use_rts: Use rts for this AC
 * @aggrsize_scaling: Aggregrate size scaling for the AC
 * @min_kbps: min kbps req
 */
struct acparams_params {
	uint8_t ac;
	uint8_t use_rts;
	uint8_t aggrsize_scaling;
	uint32_t min_kbps;
};

/**
 * struct vap_dscp_tid_map_params - DSCP tid map params
 * @vdev_id: vdev id
 * @dscp_to_tid_map: pointer to arry of tid to dscp map table
 */
struct vap_dscp_tid_map_params {
	uint8_t vdev_id;
	uint32_t *dscp_to_tid_map;
};

/**
 * struct proxy_ast_reserve_params - Proxy AST reserve params
 * @macaddr: macaddr for proxy ast entry
 */
struct proxy_ast_reserve_params {
	uint8_t *macaddr;
};

/**
 * struct fips_params - FIPS params config
 * @key: pointer to key
 * @key_len: length of key
 * @data: pointer data buf
 * @data_len: lenght of data buf
 * @mode: mode
 * @op: operation
 * @pdev_id: pdev_id for identifying the MAC
 */
struct fips_params {
	uint8_t *key;
	uint32_t key_len;
	uint8_t *data;
	uint32_t data_len;
	uint32_t mode;
	uint32_t op;
	uint32_t pdev_id;
};

/**
 * struct mcast_group_update_param - Mcast group table update to target
 * @action: Addition/deletion
 * @wildcard: iwldcard table entry?
 * @mcast_ip_addr: mcast ip address to be updated
 * @mcast_ip_addr_bytes: mcast ip addr bytes
 * @ucast_mac_addr: ucast peer mac subscribed to mcast ip
 * @filter_mode: filter mode
 * @nsrcs: number of entries in source list
 * @srcs: source mac accpted
 * @mask: mask
 * @vap_id: vdev id
 * @is_action_delete: is delete
 * @is_filter_mode_snoop:
 * @is_mcast_addr_len:
 */
struct mcast_group_update_params {
	int action;
	int wildcard;
	uint8_t *mcast_ip_addr;
	int mcast_ip_addr_bytes;
	uint8_t *ucast_mac_addr;
	uint8_t filter_mode;
	uint8_t nsrcs;
	uint8_t *srcs;
	uint8_t *mask;
	uint8_t vap_id;
	bool is_action_delete;
	bool is_filter_mode_snoop;
	bool is_mcast_addr_len;
};

/**
 * struct periodic_chan_stats_param - periodic channel stats req param
 * @stats_period: stats period update
 * @enable: enable/disable
 */
struct periodic_chan_stats_params {
	uint32_t stats_period;
	bool enable;
	uint32_t pdev_id;
};

/**
 * enum wmi_host_packet_power_rate_flags: packer power rate flags
 * @WMI_HOST_FLAG_RTSENA: RTS enabled
 * @WMI_HOST_FLAG_CTSENA: CTS enabled
 * @WMI_HOST_FLAG_STBC: STBC is set
 * @WMI_HOST_FLAG_LDPC: LDPC is set
 * @WMI_HOST_FLAG_TXBF: Tx Bf enabled
 * @WMI_HOST_FLAG_MU2: MU2 data
 * @WMI_HOST_FLAG_MU3: MU3 data
 * @WMI_HOST_FLAG_SERIES1: Rate series 1
 * @WMI_HOST_FLAG_SGI: Short gaurd interval
 */
enum wmi_host_packet_power_rate_flags {
	WMI_HOST_FLAG_RTSENA        =  0x0001,
	WMI_HOST_FLAG_CTSENA        =  0x0002,
	WMI_HOST_FLAG_STBC          =  0x0004,
	WMI_HOST_FLAG_LDPC          =  0x0008,
	WMI_HOST_FLAG_TXBF          =  0x0010,
	WMI_HOST_FLAG_MU2           =  0x0020,
	WMI_HOST_FLAG_MU3           =  0x0040,
	WMI_HOST_FLAG_SERIES1       =  0x0080,
	WMI_HOST_FLAG_SGI           =  0x0100,
};

/**
 * enum wmi_host_su_mu_ofdma_flags: packer power su mu ofdma flags
 * @WMI_HOST_FLAG_SU: SU Data
 * @WMI_HOST_FLAG_DL_MU_MIMO_AC: DL AC MU data
 * @WMI_HOST_FLAG_DL_MU_MIMO_AX: DL AX MU data
 * @WMI_HOST_FLAG_DL_OFDMA: DL OFDMA data
 * @WMI_HOST_FLAG_UL_OFDMA: UL OFDMA data
 * @WMI_HOST_FLAG_UL_MU_MIMO: UL MU data
 */
enum wmi_host_su_mu_ofdma_flags {
	WMI_HOST_FLAG_SU            =  0x0001,
	WMI_HOST_FLAG_DL_MU_MIMO_AC =  0x0002,
	WMI_HOST_FLAG_DL_MU_MIMO_AX =  0x0003,
	WMI_HOST_FLAG_DL_OFDMA      =  0x0004,
	WMI_HOST_FLAG_UL_OFDMA      =  0x0005,
	WMI_HOST_FLAG_UL_MU_MIMO    =  0x0006,
};

/**
 * enum wmi_host_preamble_type: preamble type
 * @WMI_HOST_PREAMBLE_OFDM: ofdm rate
 * @WMI_HOST_PREAMBLE_CCK:  cck rate
 * @WMI_HOST_PREAMBLE_HT: ht rate
 * @WMI_HOST_PREAMBLE_VHT: vht rate
 * @WMI_HOST_PREAMBLE_HE: 11ax he rate
 */
enum wmi_host_preamble_type {
	WMI_HOST_PREAMBLE_OFDM  =  0,
	WMI_HOST_PREAMBLE_CCK   =  1,
	WMI_HOST_PREAMBLE_HT    =  2,
	WMI_HOST_PREAMBLE_VHT   =  3,
	WMI_HOST_PREAMBLE_HE    =  4,
};

/**
 * struct packet_power_info_params - packet power info params
 * @chainmask: chain mask
 * @chan_width: channel bandwidth
 * @rate_flags: rate flags
 * @su_mu_ofdma: su/mu/ofdma flags
 * @nss: number of spatial streams
 * @preamble: preamble
 * @hw_rate:
 */
struct packet_power_info_params {
	uint16_t chainmask;
	uint16_t chan_width;
	uint16_t rate_flags;
	uint16_t su_mu_ofdma;
	uint16_t nss;
	uint16_t preamble;
	uint16_t hw_rate;
	uint32_t pdev_id;
};

/**
 * WMI_GPIO_CONFIG_CMDID
 */
enum {
	WMI_HOST_GPIO_PULL_NONE,
	WMI_HOST_GPIO_PULL_UP,
	WMI_HOST_GPIO_PULL_DOWN,
};

/**
 * WMI_GPIO_INTTYPE
 */
enum {
	WMI_HOST_GPIO_INTTYPE_DISABLE,
	WMI_HOST_GPIO_INTTYPE_RISING_EDGE,
	WMI_HOST_GPIO_INTTYPE_FALLING_EDGE,
	WMI_HOST_GPIO_INTTYPE_BOTH_EDGE,
	WMI_HOST_GPIO_INTTYPE_LEVEL_LOW,
	WMI_HOST_GPIO_INTTYPE_LEVEL_HIGH
};

/**
 * struct wmi_host_gpio_input_event - GPIO input event structure
 * @gpio_num: GPIO number which changed state
 */
typedef struct {
	uint32_t gpio_num;	/* GPIO number which changed state */
} wmi_host_gpio_input_event;

/**
 * struct gpio_config_params - GPIO config params
 * @gpio_num: GPIO number to config
 * @input: input/output
 * @pull_type: pull type
 * @intr_mode: int mode
 */
struct gpio_config_params {
	uint32_t gpio_num;
	uint32_t input;
	uint32_t pull_type;
	uint32_t intr_mode;
};

/**
 * struct gpio_output_params - GPIO output params
 * @gpio_num: GPIO number to configure
 * @set: set/reset
 */
struct gpio_output_params {
	uint32_t gpio_num;
	uint32_t set;
};

/* flags bit 0: to configure wlan priority bitmap */
#define WMI_HOST_BTCOEX_PARAM_FLAGS_WLAN_PRIORITY_BITMAP_BIT (1<<0)
/* flags bit 1: to configure both period and wlan duration */
#define WMI_HOST_BTCOEX_PARAM_FLAGS_DUTY_CYCLE_BIT (1<<1)
struct btcoex_cfg_params {
	/* WLAN priority bitmask for different frame types */
	uint32_t  btcoex_wlan_priority_bitmap;
	/* This command is used to configure different btcoex params
	 * in different situations.The host sets the appropriate bit(s)
	 * in btcoex_param_flags to indicate which configuration parameters
	 * are valid within a particular BT coex config message, so that one
	 * BT configuration parameter can be configured without affecting
	 * other BT configuration parameters.E.g. if the host wants to
	 * configure only btcoex_wlan_priority_bitmap it sets only
	 * WMI_BTCOEX_PARAM_FLAGS_WLAN_PRIORITY_BITMAP_BIT in
	 * btcoex_param_flags so that firmware will not overwrite
	 * other params with default value passed in the command.
	 * Host can also set multiple bits in btcoex_param_flags
	 * to configure more than one param in single message.
	 */
	uint32_t btcoex_param_flags;
	/* period denotes the total time in milliseconds which WLAN and BT share
	 * configured percentage for transmission and reception.
	 */
	uint32_t period;
	/* wlan duration is the time in milliseconds given for wlan
	 * in above period.
	 */
	uint32_t wlan_duration;
};

#define WMI_HOST_COEX_CONFIG_BUF_MAX_LEN 32 /* 128 bytes */
/**
 * coex_ver_cfg_t
 * @coex_version: Version for 4 wire coex
 * @length: Length of payload buffer based on version
 * @config_buf: Payload Buffer
 */
typedef struct {
	/* VERSION_4 (4 wire coex) */
	uint32_t coex_version;

	/* No. of A_UINT32 elements in payload buffer. Will depend on the coex
	 * version
	 */
	uint32_t length;

	/* Payload buffer */
	uint32_t config_buf[WMI_HOST_COEX_CONFIG_BUF_MAX_LEN];
} coex_ver_cfg_t;

#define WMI_HOST_RTT_REPORT_CFR	0
#define WMI_HOST_RTT_NO_REPORT_CFR	1
#define WMI_HOST_RTT_AGGREGATE_REPORT_NON_CFR	2
/**
 * struct rtt_meas_req_test_params
 * @peer: peer mac address
 * @req_frame_type: RTT request frame type
 * @req_bw: requested bandwidth
 * @req_preamble: Preamble
 * @req_num_req: num of requests
 * @req_report_type: report type
 * @num_measurements: number of measurements
 * @asap_mode: priority
 * @lci_requested: LCI requested
 * @loc_civ_requested:
 * @channel_param: channel param
 * @req_id: requested id
 */
struct rtt_meas_req_test_params {
	uint8_t peer[IEEE80211_ADDR_LEN];
	int req_frame_type;
	int req_bw;
	int req_preamble;
	int req_num_req;
	int req_report_type;
	uint32_t num_measurements;
	uint32_t asap_mode;
	uint32_t lci_requested;
	uint32_t loc_civ_requested;
	struct channel_param channel;
	uint8_t req_id;
};

/**
 * struct rtt_meas_req_params - RTT measurement request params
 * @req_id: Request id
 * @vdev_id: vdev id
 * @sta_mac_addr: pointer to station mac address
 * @spoof_mac_addr: pointer to spoof mac address
 * @is_mode_na: 11NA
 * @is_mode_ac: AC
 * @is_bw_20: 20
 * @is_bw_40: 40
 * @is_bw_80: 80
 * @num_probe_rqst: number of probe request
 * @channel_param: channel param
 */
struct rtt_meas_req_params {
	uint8_t req_id;
	uint8_t vdev_id;
	uint8_t *sta_mac_addr;
	uint8_t *spoof_mac_addr;
	bool is_mode_na;
	bool is_mode_ac;
	bool is_bw_20;
	bool is_bw_40;
	bool is_bw_80;
	uint32_t num_probe_rqst;
	struct channel_param channel;
};

/**
 * struct lci_set_params - LCI params
 * @lci_data: pointer to LCI data
 * @latitude_unc: latitude
 * @latitude_0_12: bits 0 to 1 of latitude
 * @latitude_2_33: bits 2 to 33 of latitude
 * @longitude_unc: longitude
 * @longitude_0_1: bits 0 to 1 of longitude
 * @longitude_2_33: bits 2 to 33 of longitude
 * @altitude_type: altitude type
 * @altitude_unc_0_3: altitude bits 0 - 3
 * @altitude_unc_4_5: altitude bits 4 - 5
 * @altitude: altitude
 * @datum: dataum
 * @reg_loc_agmt:
 * @reg_loc_dse:
 * @dep_sta:
 * @version: version
 */
struct lci_set_params {
	void *lci_data;
	uint8_t	latitude_unc:6,
		latitude_0_1:2;
	uint32_t latitude_2_33;
	uint8_t	longitude_unc:6,
		longitude_0_1:2;
	uint32_t longitude_2_33;
	uint8_t	altitude_type:4,
		altitude_unc_0_3:4;
	uint32_t altitude_unc_4_5:2,
		altitude:30;
	uint8_t	datum:3,
		reg_loc_agmt:1,
		reg_loc_dse:1,
		dep_sta:1,
		version:2;
	uint8_t *colocated_bss;
	int msg_len;
};

/**
 * struct lcr_set_params - LCR params
 * @lcr_data: pointer to lcr data
 */
struct lcr_set_params {
	void *lcr_data;
	int msg_len;
};

/**
 * struct rtt_keepalive_req_params - RTT keepalive params
 * @macaddr: pointer to macaddress
 * @req_id: Request id
 * @vdev_id: vdev id
 * @stop: start/stop
 */
struct rtt_keepalive_req_params {
	uint8_t *macaddr;
	uint8_t req_id;
	uint8_t vdev_id;
	bool stop;
};

/**
 * struct rx_reorder_queue_setup_params  - Reorder queue setup params
 * @peer_mac_addr: Peer mac address
 * @tid: TID
 * @vdev_id: vdev id
 * @hw_qdesc_paddr_lo: lower 32 bits of queue desc adddress
 * @hw_qdesc_paddr_hi: upper 32 bits of queue desc adddress
 * @queue_no: 16-bit number assigned by host for queue
 */
struct rx_reorder_queue_setup_params {
	uint8_t *peer_macaddr;
	uint16_t tid;
	uint16_t vdev_id;
	uint32_t hw_qdesc_paddr_lo;
	uint32_t hw_qdesc_paddr_hi;
	uint16_t queue_no;
};

/**
 * struct rx_reorder_queue_remove_params  - Reorder queue setup params
 * @peer_mac_addr: Peer mac address
 * @vdev_id: vdev id
 * @peer_tid_bitmap: peer tid bitmap
 */
struct rx_reorder_queue_remove_params {
	uint8_t *peer_macaddr;
	uint16_t vdev_id;
	uint32_t peer_tid_bitmap;
};

/**
 * struct wmi_host_stats_event - Stats event params
 * @stats_id: stats id of type wmi_host_stats_event
 * @num_pdev_stats: number of pdev stats event structures 0 or 1
 * @num_pdev_ext_stats: number of pdev ext stats event structures
 * @num_vdev_stats: number of vdev stats
 * @num_peer_stats: number of peer stats event structures 0 or max peers
 * @num_bcnflt_stats: number of beacon filter stats
 * @num_chan_stats: number of channel stats
 * @pdev_id: device id for the radio
 * @num_bcn_stats: number of beacon stats
 */
typedef struct {
	wmi_host_stats_id stats_id;
	uint32_t num_pdev_stats;
	uint32_t num_pdev_ext_stats;
	uint32_t num_vdev_stats;
	uint32_t num_peer_stats;
	uint32_t num_bcnflt_stats;
	uint32_t num_chan_stats;
	uint32_t pdev_id;
	uint32_t num_bcn_stats;
} wmi_host_stats_event;

/**
 * struct wmi_host_peer_extd_stats - peer extd stats event structure
 * @peer_macaddr: Peer mac address
 * @inactive_time: inactive time in secs
 * @peer_chain_rssi: peer rssi
 * @rx_duration: RX duration
 * @peer_tx_bytes: TX bytes
 * @peer_rx_bytes: RX bytes
 * @last_tx_rate_code: Tx rate code of last frame
 * @last_tx_power: Tx power latest
 * @atf_tokens_allocated: atf tokens allocated
 * @atf_tokens_utilized: atf tokens utilized
 * @reserved: for future use
 */
typedef struct {
	wmi_host_mac_addr peer_macaddr;
	uint32_t inactive_time;
	uint32_t peer_chain_rssi;
	uint32_t rx_duration;
	uint32_t peer_tx_bytes;
	uint32_t peer_rx_bytes;
	uint32_t last_tx_rate_code;
	uint32_t last_tx_power;
	uint32_t atf_tokens_allocated;
	uint32_t atf_tokens_utilized;
	uint32_t reserved[4];
} wmi_host_peer_extd_stats;

/**
 * struct wmi_host_pdev_ext_stats - peer ext stats structure
 * @rx_rssi_comb: RX rssi
 * @rx_rssi_chain0: RX rssi chain 0
 * @rx_rssi_chain1: RX rssi chain 1
 * @rx_rssi_chain2: RX rssi chain 2
 * @rx_rssi_chain3: RX rssi chain 3
 * @rx_mcs: RX MCS array
 * @tx_mcs: TX MCS array
 * @ack_rssi: Ack rssi
 */
typedef struct {
	uint32_t	rx_rssi_comb;
	uint32_t	rx_rssi_chain0;
	uint32_t	rx_rssi_chain1;
	uint32_t	rx_rssi_chain2;
	uint32_t	rx_rssi_chain3;
	uint32_t	rx_mcs[10];
	uint32_t	tx_mcs[10];
	uint32_t	ack_rssi;
} wmi_host_pdev_ext_stats;

/**
 * struct wmi_host_dbg_tx_stats - Debug stats
 * @comp_queued: Num HTT cookies queued to dispatch list
 * @comp_delivered: Num HTT cookies dispatched
 * @msdu_enqued: Num MSDU queued to WAL
 * @mpdu_enqued: Num MPDU queue to WAL
 * @wmm_drop: Num MSDUs dropped by WMM limit
 * @local_enqued: Num Local frames queued
 * @local_freed: Num Local frames done
 * @hw_queued: Num queued to HW
 * @hw_reaped: Num PPDU reaped from HW
 * @underrun: Num underruns
 * @hw_paused: HW Paused.
 * @tx_abort: Num PPDUs cleaned up in TX abort
 * @mpdus_requed: Num MPDUs requed by SW
 * @tx_ko: excessive retries
 * @tx_xretry:
 * @data_rc: data hw rate code
 * @self_triggers: Scheduler self triggers
 * @sw_retry_failure: frames dropped due to excessive sw retries
 * @illgl_rate_phy_err: illegal rate phy errors
 * @pdev_cont_xretry: wal pdev continous xretry
 * @pdev_tx_timeout: wal pdev continous xretry
 * @pdev_resets: wal pdev resets
 * @stateless_tid_alloc_failure: frames dropped due to non-availability of
 *                               stateless TIDs
 * @phy_underrun: PhY/BB underrun
 * @txop_ovf: MPDU is more than txop limit
 * @seq_posted: Number of Sequences posted
 * @seq_failed_queueing: Number of Sequences failed queueing
 * @seq_completed: Number of Sequences completed
 * @seq_restarted: Number of Sequences restarted
 * @mu_seq_posted: Number of MU Sequences posted
 * @mpdus_sw_flush: Num MPDUs flushed by SW, HWPAUSED, SW TXABORT
 *                  (Reset,channel change)
 * @mpdus_hw_filter: Num MPDUs filtered by HW, all filter condition
 *                   (TTL expired)
 * @mpdus_truncated: Num MPDUs truncated by PDG (TXOP, TBTT,
 *                   PPDU_duration based on rate, dyn_bw)
 * @mpdus_ack_failed: Num MPDUs that was tried but didn't receive ACK or BA
 * @mpdus_expired: Num MPDUs that was dropped du to expiry.
 * @mc_dropr: Num mc drops
 */
typedef struct {
	int32_t comp_queued;
	int32_t comp_delivered;
	int32_t msdu_enqued;
	int32_t mpdu_enqued;
	int32_t wmm_drop;
	int32_t local_enqued;
	int32_t local_freed;
	int32_t hw_queued;
	int32_t hw_reaped;
	int32_t underrun;
	uint32_t hw_paused;
	int32_t tx_abort;
	int32_t mpdus_requed;
	uint32_t tx_ko;
	uint32_t tx_xretry;
	uint32_t data_rc;
	uint32_t self_triggers;
	uint32_t sw_retry_failure;
	uint32_t illgl_rate_phy_err;
	uint32_t pdev_cont_xretry;
	uint32_t pdev_tx_timeout;
	uint32_t pdev_resets;
	uint32_t stateless_tid_alloc_failure;
	uint32_t phy_underrun;
	uint32_t txop_ovf;
	uint32_t seq_posted;
	uint32_t seq_failed_queueing;
	uint32_t seq_completed;
	uint32_t seq_restarted;
	uint32_t mu_seq_posted;
	int32_t mpdus_sw_flush;
	int32_t mpdus_hw_filter;
	int32_t mpdus_truncated;
	int32_t mpdus_ack_failed;
	int32_t mpdus_expired;
	uint32_t mc_drop;
} wmi_host_dbg_tx_stats;

/**
 * struct wmi_host_dbg_rx_stats - RX Debug stats
 * @mid_ppdu_route_change: Cnts any change in ring routing mid-ppdu
 * @status_rcvd: Total number of statuses processed
 * @r0_frags: Extra frags on rings 0
 * @r1_frags: Extra frags on rings 1
 * @r2_frags: Extra frags on rings 2
 * @r3_frags: Extra frags on rings 3
 * @htt_msdus: MSDUs delivered to HTT
 * @htt_mpdus: MPDUs delivered to HTT
 * @loc_msdus: MSDUs delivered to local stack
 * @loc_mpdus: MPDUS delivered to local stack
 * @oversize_amsdu: AMSDUs that have more MSDUs than the status ring size
 * @phy_errs: Number of PHY errors
 * @phy_err_drop: Number of PHY errors drops
 * @mpdu_errs: Number of mpdu errors - FCS, MIC, ENC etc.
 * @pdev_rx_timeout: Number of rx inactivity timeouts
 * @rx_ovfl_errs: Number of rx overflow errors.
 */
typedef struct {
	int32_t mid_ppdu_route_change;
	int32_t status_rcvd;
	int32_t r0_frags;
	int32_t r1_frags;
	int32_t r2_frags;
	int32_t r3_frags;
	int32_t htt_msdus;
	int32_t htt_mpdus;
	int32_t loc_msdus;
	int32_t loc_mpdus;
	int32_t oversize_amsdu;
	int32_t phy_errs;
	int32_t phy_err_drop;
	int32_t mpdu_errs;
	uint32_t pdev_rx_timeout;
	int32_t rx_ovfl_errs;
} wmi_host_dbg_rx_stats;

/** struct wmi_host_dbg_mem_stats - memory stats
 * @iram_free_size: IRAM free size on target
 * @dram_free_size: DRAM free size on target
 * @sram_free_size: SRAM free size on target
 */
typedef struct {
	uint32_t iram_free_size;
	uint32_t dram_free_size;
	/* Only Non-TLV */
	uint32_t sram_free_size;
} wmi_host_dbg_mem_stats;

typedef struct {
	/* Only TLV */
	int32_t dummy;/* REMOVE THIS ONCE REAL PEER STAT COUNTERS ARE ADDED */
} wmi_host_dbg_peer_stats;

/**
 * struct wmi_host_dbg_stats - host debug stats
 * @tx: TX stats of type wmi_host_dbg_tx_stats
 * @rx: RX stats of type wmi_host_dbg_rx_stats
 * @mem: Memory stats of type wmi_host_dbg_mem_stats
 * @peer: peer stats of type wmi_host_dbg_peer_stats
 */
typedef struct {
	wmi_host_dbg_tx_stats tx;
	wmi_host_dbg_rx_stats rx;
	wmi_host_dbg_mem_stats mem;
	wmi_host_dbg_peer_stats peer;
} wmi_host_dbg_stats;

/**
 * struct wmi_host_pdev_stats - PDEV stats
 * @chan_nf: Channel noise floor
 * @tx_frame_count: TX frame count
 * @rx_frame_count: RX frame count
 * @rx_clear_count: rx clear count
 * @cycle_count: cycle count
 * @phy_err_count: Phy error count
 * @chan_tx_pwr: Channel Tx Power
 * @pdev_stats: WAL dbg stats
 * @ackRcvBad:
 * @rtsBad:
 * @rtsGood:
 * @fcsBad:
 * @noBeacons:
 * @mib_int_count:
 */
typedef struct {
	int32_t chan_nf;
	uint32_t tx_frame_count;
	uint32_t rx_frame_count;
	uint32_t rx_clear_count;
	uint32_t cycle_count;
	uint32_t phy_err_count;
	uint32_t chan_tx_pwr;
	wmi_host_dbg_stats pdev_stats;
	uint32_t	ackRcvBad;
	uint32_t	rtsBad;
	uint32_t	rtsGood;
	uint32_t	fcsBad;
	uint32_t	noBeacons;
	uint32_t	mib_int_count;
} wmi_host_pdev_stats;


/**
 * struct wmi_unit_test_event - Structure corresponding to WMI Unit test event
 * @vdev_id: VDEV ID
 * @module_id: MODULE ID
 * @diag_token: Diag Token (the number that was generated in the unit-test cmd)
 * @flag: flag has 2 bits 0x1 indicates status, and 0x2 indicates done-bit
 * @payload_len: payload_len (blindly copied from payload_len field in WMI)
 * @buffer_len: actual number of data bytes in the variable data size TLV
 *              buffer_len is likely to be the nearest multiple of 4 (from
 *              payload_len). both buffer_len and payload_len need to be
 *              passed to wifitool so that the driver can be agnostic
 *              regarding these differences.
 * @buffer: data buffer
 */
typedef struct {
	uint32_t vdev_id;
	uint32_t module_id;
	uint32_t diag_token;
	uint32_t flag;
	uint32_t payload_len;
	uint32_t buffer_len;
	uint8_t buffer[1];
} wmi_unit_test_event;


/**
 * struct wmi_host_snr_info - WMI host Signal to noise ration info
 * @bcn_snr: beacon SNR
 * @dat_snr: Data frames SNR
 */
typedef struct {
	int32_t bcn_snr;
	int32_t dat_snr;
} wmi_host_snr_info;

#define WMI_HOST_MAX_TX_RATE_VALUES	10	/*Max Tx Rates */
#define WMI_HOST_MAX_RSSI_VALUES	10	/*Max Rssi values */

/* The WLAN_MAX_AC macro cannot be changed without breaking
 *  * WMI compatibility.
 *   * The maximum value of access category
 *	*/
#define WMI_HOST_WLAN_MAX_AC  4

/**
 * struct wmi_host_vdev_stats - vdev stats structure
 * @vdev_id: unique id identifying the VDEV, generated by the caller
 *        Rest all Only TLV
 * @vdev_snr: wmi_host_snr_info
 * @tx_frm_cnt: Total number of packets(per AC) that were successfully
 *              transmitted (with and without retries,
 *              including multi-cast, broadcast)
 * @rx_frm_cnt: Total number of packets that were successfully received
 *             (after appropriate filter rules including multi-cast, broadcast)
 * @multiple_retry_cnt: The number of MSDU packets and MMPDU frames per AC
 *      that the 802.11 station successfully transmitted after
 *      more than one retransmission attempt
 * @fail_cnt: Total number packets(per AC) failed to transmit
 * @rts_fail_cnt: Total number of RTS/CTS sequence failures for transmission
 *      of a packet
 * @rts_succ_cnt: Total number of RTS/CTS sequence success for transmission
 *      of a packet
 * @rx_err_cnt: The receive error count. HAL will provide the
 *      RxP FCS error global
 * @rx_discard_cnt: The sum of the receive error count and
 *      dropped-receive-buffer error count (FCS error)
 * @ack_fail_cnt: Total number packets failed transmit because of no
 *      ACK from the remote entity
 * @tx_rate_history:History of last ten transmit rate, in units of 500 kbit/sec
 * @bcn_rssi_history: History of last ten Beacon rssi of the connected Bss
 */
typedef struct {
	uint32_t vdev_id;
	/* Rest all Only TLV */
	wmi_host_snr_info vdev_snr;
	uint32_t tx_frm_cnt[WMI_HOST_WLAN_MAX_AC];
	uint32_t rx_frm_cnt;
	uint32_t multiple_retry_cnt[WMI_HOST_WLAN_MAX_AC];
	uint32_t fail_cnt[WMI_HOST_WLAN_MAX_AC];
	uint32_t rts_fail_cnt;
	uint32_t rts_succ_cnt;
	uint32_t rx_err_cnt;
	uint32_t rx_discard_cnt;
	uint32_t ack_fail_cnt;
	uint32_t tx_rate_history[WMI_HOST_MAX_TX_RATE_VALUES];
	uint32_t bcn_rssi_history[WMI_HOST_MAX_RSSI_VALUES];
} wmi_host_vdev_stats;

/**
 * struct wmi_host_vdev_stats - vdev stats structure
 * @vdev_id: unique id identifying the VDEV, generated by the caller
 * @tx_bcn_succ_cnt: Total number of beacon frame transmitted successfully
 * @tx_bcn_outage_cnt: Total number of failed beacons
 */
typedef struct {
	uint32_t vdev_id;
	uint32_t tx_bcn_succ_cnt;
	uint32_t tx_bcn_outage_cnt;
} wmi_host_bcn_stats;

/**
 * struct wmi_host_vdev_extd_stats - VDEV extended stats
 * @vdev_id: unique id identifying the VDEV, generated by the caller
 * @ppdu_aggr_cnt: No of Aggrs Queued to HW
 * @ppdu_noack: No of PPDU's not Acked includes both aggr and nonaggr's
 * @mpdu_queued: No of MPDU/Subframes's queued to HW in Aggregates
 * @ppdu_nonaggr_cnt: No of NonAggr/MPDU/Subframes's queued to HW
 *         in Legacy NonAggregates
 * @mpdu_sw_requed: No of MPDU/Subframes's SW requeued includes
 *         both Aggr and NonAggr
 * @mpdu_suc_retry: No of MPDU/Subframes's transmitted Successfully
 *         after Single/mul HW retry
 * @mpdu_suc_multitry: No of MPDU/Subframes's transmitted Success
 *         after Multiple HW retry
 * @mpdu_fail_retry: No of MPDU/Subframes's failed transmission
 *         after Multiple HW retry
 * @reserved[13]: for future extensions set to 0x0
 */
typedef struct {
	uint32_t vdev_id;
	uint32_t ppdu_aggr_cnt;
	uint32_t ppdu_noack;
	uint32_t mpdu_queued;
	uint32_t ppdu_nonaggr_cnt;
	uint32_t mpdu_sw_requed;
	uint32_t mpdu_suc_retry;
	uint32_t mpdu_suc_multitry;
	uint32_t mpdu_fail_retry;
	uint32_t reserved[13];
} wmi_host_vdev_extd_stats;

/**
 * struct wmi_host_vdev_nac_rssi_event - VDEV nac rssi stats
 * @vdev_id: unique id identifying the VDEV, generated by the caller
 * @last_rssi: rssi
 * @avg_rssi: averge rssi
 * @rssi_seq_num: rssi sequence number
 */
struct wmi_host_vdev_nac_rssi_event {
	uint32_t vdev_id;
	uint32_t last_rssi;
	uint32_t avg_rssi;
	uint32_t rssi_seq_num;
};

/**
 * struct wmi_host_peer_stats - peer stats
 * @peer_macaddr: peer MAC address
 * @peer_rssi: rssi
 * @peer_rssi_seq_num: rssi sequence number
 * @peer_tx_rate: last tx data rate used for peer
 * @peer_rx_rate: last rx data rate used for peer
 * @currentper: Current PER
 * @retries: Retries happend during transmission
 * @txratecount: Maximum Aggregation Size
 * @max4msframelen: Max4msframelen of tx rates used
 * @totalsubframes: Total no of subframes
 * @txbytes: No of bytes transmitted to the client
 * @nobuffs[4]: Packet Loss due to buffer overflows
 * @excretries[4]: Packet Loss due to excessive retries
 * @peer_rssi_changed: how many times peer's RSSI changed by a
 *        non-negligible amount
 */
typedef struct {
	wmi_host_mac_addr peer_macaddr;
	uint32_t  peer_rssi;
	uint32_t  peer_rssi_seq_num;
	uint32_t  peer_tx_rate;
	uint32_t  peer_rx_rate;
	uint32_t  currentper;
	uint32_t  retries;
	uint32_t  txratecount;
	uint32_t  max4msframelen;
	uint32_t  totalsubframes;
	uint32_t txbytes;
	uint32_t  nobuffs[4];
	uint32_t  excretries[4];
	uint32_t  peer_rssi_changed;
} wmi_host_peer_stats;

typedef struct {
	uint32_t dummy;
} wmi_host_bcnflt_stats;

/**
 * struct wmi_host_chan_stats - WMI chan stats
 * @chan_mhz: Primary channel freq of the channel for which stats are sent
 * @sampling_period_us: Time spent on the channel
 * @rx_clear_count: Aggregate duration over a sampling period for
 *       which channel activity was observed
 * @tx_duration_us: Accumalation of the TX PPDU duration over a sampling period
 * @rx_duration_us: Accumalation of the RX PPDU duration over a sampling period
 */
typedef struct {
	uint32_t chan_mhz;
	uint32_t sampling_period_us;
	uint32_t rx_clear_count;
	uint32_t tx_duration_us;
	uint32_t rx_duration_us;
} wmi_host_chan_stats;

#define WMI_EVENT_ID_INVALID 0
/**
 * Host based ENUM IDs for events to abstract target enums for event_id
 */
typedef enum {
	wmi_service_ready_event_id = 0,
	wmi_ready_event_id,
	wmi_dbg_msg_event_id,
	wmi_scan_event_id,
	wmi_echo_event_id,
	wmi_update_stats_event_id,
	wmi_inst_rssi_stats_event_id,
	wmi_vdev_start_resp_event_id,
	wmi_vdev_standby_req_event_id,
	wmi_vdev_resume_req_event_id,
	wmi_vdev_stopped_event_id,
	wmi_peer_sta_kickout_event_id,
	wmi_host_swba_event_id,
	wmi_tbttoffset_update_event_id,
	wmi_mgmt_rx_event_id,
	wmi_chan_info_event_id,
	wmi_phyerr_event_id,
	wmi_roam_event_id,
	wmi_profile_match,
	wmi_debug_print_event_id,
	wmi_pdev_qvit_event_id,
	wmi_wlan_profile_data_event_id,
	wmi_rtt_meas_report_event_id,
	wmi_tsf_meas_report_event_id,
	wmi_rtt_error_report_event_id,
	wmi_rtt_keepalive_event_id,
	wmi_oem_cap_event_id,
	wmi_oem_meas_report_event_id,
	wmi_oem_report_event_id,
	wmi_nan_event_id,
	wmi_wow_wakeup_host_event_id,
	wmi_gtk_offload_status_event_id,
	wmi_gtk_rekey_fail_event_id,
	wmi_dcs_interference_event_id,
	wmi_pdev_tpc_config_event_id,
	wmi_csa_handling_event_id,
	wmi_gpio_input_event_id,
	wmi_peer_ratecode_list_event_id,
	wmi_generic_buffer_event_id,
	wmi_mcast_buf_release_event_id,
	wmi_mcast_list_ageout_event_id,
	wmi_vdev_get_keepalive_event_id,
	wmi_wds_peer_event_id,
	wmi_peer_sta_ps_statechg_event_id,
	wmi_pdev_fips_event_id,
	wmi_tt_stats_event_id,
	wmi_pdev_channel_hopping_event_id,
	wmi_pdev_ani_cck_level_event_id,
	wmi_pdev_ani_ofdm_level_event_id,
	wmi_pdev_reserve_ast_entry_event_id,
	wmi_pdev_nfcal_power_event_id,
	wmi_pdev_tpc_event_id,
	wmi_pdev_get_ast_info_event_id,
	wmi_pdev_temperature_event_id,
	wmi_pdev_nfcal_power_all_channels_event_id,
	wmi_pdev_bss_chan_info_event_id,
	wmi_mu_report_event_id,
	wmi_pdev_utf_event_id,
	wmi_pdev_dump_event_id,
	wmi_tx_pause_event_id,
	wmi_dfs_radar_event_id,
	wmi_pdev_l1ss_track_event_id,
	wmi_service_ready_ext_event_id,
	wmi_vdev_install_key_complete_event_id,
	wmi_vdev_mcc_bcn_intvl_change_req_event_id,
	wmi_vdev_tsf_report_event_id,
	wmi_peer_info_event_id,
	wmi_peer_tx_fail_cnt_thr_event_id,
	wmi_peer_estimated_linkspeed_event_id,
	wmi_peer_state_event_id,
	wmi_offload_bcn_tx_status_event_id,
	wmi_offload_prob_resp_tx_status_event_id,
	wmi_mgmt_tx_completion_event_id,
	wmi_tx_delba_complete_event_id,
	wmi_tx_addba_complete_event_id,
	wmi_ba_rsp_ssn_event_id,
	wmi_aggr_state_trig_event_id,
	wmi_roam_synch_event_id,
	wmi_roam_synch_frame_event_id,
	wmi_p2p_disc_event_id,
	wmi_p2p_noa_event_id,
	wmi_p2p_lo_stop_event_id,
	wmi_pdev_resume_event_id,
	wmi_d0_wow_disable_ack_event_id,
	wmi_wow_initial_wakeup_event_id,
	wmi_stats_ext_event_id,
	wmi_iface_link_stats_event_id,
	wmi_peer_link_stats_event_id,
	wmi_radio_link_stats_link,
	wmi_update_fw_mem_dump_event_id,
	wmi_diag_event_id_log_supported_event_id,
	wmi_nlo_match_event_id,
	wmi_nlo_scan_complete_event_id,
	wmi_apfind_event_id,
	wmi_passpoint_match_event_id,
	wmi_chatter_pc_query_event_id,
	wmi_pdev_ftm_intg_event_id,
	wmi_wlan_freq_avoid_event_id,
	wmi_thermal_mgmt_event_id,
	wmi_diag_container_event_id,
	wmi_host_auto_shutdown_event_id,
	wmi_update_whal_mib_stats_event_id,
	wmi_update_vdev_rate_stats_event_id,
	wmi_diag_event_id,
	wmi_unit_test_event_id,
	wmi_ocb_set_sched_event_id,
	wmi_dbg_mesg_flush_complete_event_id,
	wmi_rssi_breach_event_id,
	wmi_uploadh_event_id,
	wmi_captureh_event_id,
	wmi_rfkill_state_change_event_id,
	wmi_tdls_peer_event_id,
	wmi_batch_scan_enabled_event_id,
	wmi_batch_scan_result_event_id,
	wmi_lpi_result_event_id,
	wmi_lpi_status_event_id,
	wmi_lpi_handoff_event_id,
	wmi_extscan_start_stop_event_id,
	wmi_extscan_operation_event_id,
	wmi_extscan_table_usage_event_id,
	wmi_extscan_cached_results_event_id,
	wmi_extscan_wlan_change_results_event_id,
	wmi_extscan_hotlist_match_event_id,
	wmi_extscan_capabilities_event_id,
	wmi_extscan_hotlist_ssid_match_event_id,
	wmi_mdns_stats_event_id,
	wmi_sap_ofl_add_sta_event_id,
	wmi_sap_ofl_del_sta_event_id,
	wmi_ocb_set_config_resp_event_id,
	wmi_ocb_get_tsf_timer_resp_event_id,
	wmi_dcc_get_stats_resp_event_id,
	wmi_dcc_update_ndl_resp_event_id,
	wmi_dcc_stats_event_id,
	wmi_soc_set_hw_mode_resp_event_id,
	wmi_soc_hw_mode_transition_event_id,
	wmi_soc_set_dual_mac_config_resp_event_id,
	wmi_tx_data_traffic_ctrl_event_id,
	wmi_peer_tx_mu_txmit_count_event_id,
	wmi_peer_gid_userpos_list_event_id,
	wmi_pdev_check_cal_version_event_id,
	wmi_atf_peer_stats_event_id,
	wmi_peer_delete_response_event_id,
	wmi_pdev_csa_switch_count_status_event_id,
	wmi_reg_chan_list_cc_event_id,
	wmi_offchan_data_tx_completion_event,
	wmi_dfs_cac_complete_id,
	wmi_dfs_radar_detection_event_id,
	wmi_ext_tbttoffset_update_event_id,
	wmi_11d_new_country_event_id,
	wmi_get_arp_stats_req_id,
	wmi_service_available_event_id,
	wmi_update_rcpi_event_id,
	wmi_pdev_wds_entry_list_event_id,
	wmi_ndp_initiator_rsp_event_id,
	wmi_ndp_indication_event_id,
	wmi_ndp_confirm_event_id,
	wmi_ndp_responder_rsp_event_id,
	wmi_ndp_end_indication_event_id,
	wmi_ndp_end_rsp_event_id,
	wmi_oem_response_event_id,
	wmi_peer_stats_info_event_id,
	wmi_pdev_chip_power_stats_event_id,
	wmi_ap_ps_egap_info_event_id,
	wmi_peer_assoc_conf_event_id,
	wmi_vdev_delete_resp_event_id,
	wmi_bpf_capability_info_event_id,
	wmi_vdev_encrypt_decrypt_data_rsp_event_id,
	wmi_report_rx_aggr_failure_event_id,
	wmi_pdev_chip_pwr_save_failure_detect_event_id,
	wmi_peer_antdiv_info_event_id,
	wmi_pdev_set_hw_mode_rsp_event_id,
	wmi_pdev_hw_mode_transition_event_id,
	wmi_pdev_set_mac_config_resp_event_id,
	wmi_coex_bt_activity_event_id,
	wmi_mgmt_tx_bundle_completion_event_id,
	wmi_radio_tx_power_level_stats_event_id,
	wmi_report_stats_event_id,
	wmi_dma_buf_release_event_id,
	wmi_sap_obss_detection_report_event_id,
	wmi_obss_color_collision_report_event_id,
	wmi_host_swfda_event_id,
	wmi_sar_get_limits_event_id,

	wmi_events_max,
} wmi_conv_event_id;

#define WMI_UNAVAILABLE_PARAM 0
/**
 * Host based ENUM IDs for PDEV params to abstract target enums
 */
typedef enum {
	wmi_pdev_param_tx_chain_mask = 0,
	wmi_pdev_param_rx_chain_mask,
	wmi_pdev_param_txpower_limit2g,
	wmi_pdev_param_txpower_limit5g,
	wmi_pdev_param_txpower_scale,
	wmi_pdev_param_beacon_gen_mode,
	wmi_pdev_param_beacon_tx_mode,
	wmi_pdev_param_resmgr_offchan_mode,
	wmi_pdev_param_protection_mode,
	wmi_pdev_param_dynamic_bw,
	wmi_pdev_param_non_agg_sw_retry_th,
	wmi_pdev_param_agg_sw_retry_th,
	wmi_pdev_param_sta_kickout_th,
	wmi_pdev_param_ac_aggrsize_scaling,
	wmi_pdev_param_ltr_enable,
	wmi_pdev_param_ltr_ac_latency_be,
	wmi_pdev_param_ltr_ac_latency_bk,
	wmi_pdev_param_ltr_ac_latency_vi,
	wmi_pdev_param_ltr_ac_latency_vo,
	wmi_pdev_param_ltr_ac_latency_timeout,
	wmi_pdev_param_ltr_sleep_override,
	wmi_pdev_param_ltr_rx_override,
	wmi_pdev_param_ltr_tx_activity_timeout,
	wmi_pdev_param_l1ss_enable,
	wmi_pdev_param_dsleep_enable,
	wmi_pdev_param_pcielp_txbuf_flush,
	wmi_pdev_param_pcielp_txbuf_watermark,
	wmi_pdev_param_pcielp_txbuf_tmo_en,
	wmi_pdev_param_pcielp_txbuf_tmo_value,
	wmi_pdev_param_pdev_stats_update_period,
	wmi_pdev_param_vdev_stats_update_period,
	wmi_pdev_param_peer_stats_update_period,
	wmi_pdev_param_bcnflt_stats_update_period,
	wmi_pdev_param_pmf_qos,
	wmi_pdev_param_arp_ac_override,
	wmi_pdev_param_dcs,
	wmi_pdev_param_ani_enable,
	wmi_pdev_param_ani_poll_period,
	wmi_pdev_param_ani_listen_period,
	wmi_pdev_param_ani_ofdm_level,
	wmi_pdev_param_ani_cck_level,
	wmi_pdev_param_dyntxchain,
	wmi_pdev_param_proxy_sta,
	wmi_pdev_param_idle_ps_config,
	wmi_pdev_param_power_gating_sleep,
	wmi_pdev_param_aggr_burst,
	wmi_pdev_param_rx_decap_mode,
	wmi_pdev_param_fast_channel_reset,
	wmi_pdev_param_burst_dur,
	wmi_pdev_param_burst_enable,
	wmi_pdev_param_smart_antenna_default_antenna,
	wmi_pdev_param_igmpmld_override,
	wmi_pdev_param_igmpmld_tid,
	wmi_pdev_param_antenna_gain,
	wmi_pdev_param_rx_filter,
	wmi_pdev_set_mcast_to_ucast_tid,
	wmi_pdev_param_proxy_sta_mode,
	wmi_pdev_param_set_mcast2ucast_mode,
	wmi_pdev_param_set_mcast2ucast_buffer,
	wmi_pdev_param_remove_mcast2ucast_buffer,
	wmi_pdev_peer_sta_ps_statechg_enable,
	wmi_pdev_param_igmpmld_ac_override,
	wmi_pdev_param_block_interbss,
	wmi_pdev_param_set_disable_reset_cmdid,
	wmi_pdev_param_set_msdu_ttl_cmdid,
	wmi_pdev_param_set_ppdu_duration_cmdid,
	wmi_pdev_param_txbf_sound_period_cmdid,
	wmi_pdev_param_set_promisc_mode_cmdid,
	wmi_pdev_param_set_burst_mode_cmdid,
	wmi_pdev_param_en_stats,
	wmi_pdev_param_mu_group_policy,
	wmi_pdev_param_noise_detection,
	wmi_pdev_param_noise_threshold,
	wmi_pdev_param_dpd_enable,
	wmi_pdev_param_set_mcast_bcast_echo,
	wmi_pdev_param_atf_strict_sch,
	wmi_pdev_param_atf_sched_duration,
	wmi_pdev_param_ant_plzn,
	wmi_pdev_param_mgmt_retry_limit,
	wmi_pdev_param_sensitivity_level,
	wmi_pdev_param_signed_txpower_2g,
	wmi_pdev_param_signed_txpower_5g,
	wmi_pdev_param_enable_per_tid_amsdu,
	wmi_pdev_param_enable_per_tid_ampdu,
	wmi_pdev_param_cca_threshold,
	wmi_pdev_param_rts_fixed_rate,
	wmi_pdev_param_cal_period,
	wmi_pdev_param_pdev_reset,
	wmi_pdev_param_wapi_mbssid_offset,
	wmi_pdev_param_arp_srcaddr,
	wmi_pdev_param_arp_dstaddr,
	wmi_pdev_param_txpower_decr_db,
	wmi_pdev_param_rx_batchmode,
	wmi_pdev_param_packet_aggr_delay,
	wmi_pdev_param_atf_obss_noise_sch,
	wmi_pdev_param_atf_obss_noise_scaling_factor,
	wmi_pdev_param_cust_txpower_scale,
	wmi_pdev_param_atf_dynamic_enable,
	wmi_pdev_param_atf_ssid_group_policy,
	wmi_pdev_param_rfkill_enable,
	wmi_pdev_param_hw_rfkill_config,
	wmi_pdev_param_low_power_rf_enable,
	wmi_pdev_param_l1ss_track,
	wmi_pdev_param_hyst_en,
	wmi_pdev_param_power_collapse_enable,
	wmi_pdev_param_led_sys_state,
	wmi_pdev_param_led_enable,
	wmi_pdev_param_audio_over_wlan_latency,
	wmi_pdev_param_audio_over_wlan_enable,
	wmi_pdev_param_whal_mib_stats_update_enable,
	wmi_pdev_param_vdev_rate_stats_update_period,
	wmi_pdev_param_cts_cbw,
	wmi_pdev_param_wnts_config,
	wmi_pdev_param_adaptive_early_rx_enable,
	wmi_pdev_param_adaptive_early_rx_min_sleep_slop,
	wmi_pdev_param_adaptive_early_rx_inc_dec_step,
	wmi_pdev_param_early_rx_fix_sleep_slop,
	wmi_pdev_param_bmiss_based_adaptive_bto_enable,
	wmi_pdev_param_bmiss_bto_min_bcn_timeout,
	wmi_pdev_param_bmiss_bto_inc_dec_step,
	wmi_pdev_param_bto_fix_bcn_timeout,
	wmi_pdev_param_ce_based_adaptive_bto_enable,
	wmi_pdev_param_ce_bto_combo_ce_value,
	wmi_pdev_param_tx_chain_mask_2g,
	wmi_pdev_param_rx_chain_mask_2g,
	wmi_pdev_param_tx_chain_mask_5g,
	wmi_pdev_param_rx_chain_mask_5g,
	wmi_pdev_param_tx_chain_mask_cck,
	wmi_pdev_param_tx_chain_mask_1ss,
	wmi_pdev_param_enable_btcoex,
	wmi_pdev_param_atf_peer_stats,
	wmi_pdev_param_btcoex_cfg,
	wmi_pdev_param_mesh_mcast_enable,
	wmi_pdev_param_tx_ack_timeout,
	wmi_pdev_param_soft_tx_chain_mask,
	wmi_pdev_param_cck_tx_enable,

	wmi_pdev_param_max,
} wmi_conv_pdev_params_id;


/**
 * Host based ENUM IDs for VDEV params to abstract target enums
 */
typedef enum {
	wmi_vdev_param_rts_threshold = 0,
	wmi_vdev_param_fragmentation_threshold,
	wmi_vdev_param_beacon_interval,
	wmi_vdev_param_listen_interval,
	wmi_vdev_param_multicast_rate,
	wmi_vdev_param_mgmt_tx_rate,
	wmi_vdev_param_slot_time,
	wmi_vdev_param_preamble,
	wmi_vdev_param_swba_time,
	wmi_vdev_stats_update_period,
	wmi_vdev_pwrsave_ageout_time,
	wmi_vdev_host_swba_interval,
	wmi_vdev_param_dtim_period,
	wmi_vdev_oc_scheduler_air_time_limit,
	wmi_vdev_param_wds,
	wmi_vdev_param_atim_window,
	wmi_vdev_param_bmiss_count_max,
	wmi_vdev_param_bmiss_first_bcnt,
	wmi_vdev_param_bmiss_final_bcnt,
	wmi_vdev_param_feature_wmm,
	wmi_vdev_param_chwidth,
	wmi_vdev_param_chextoffset,
	wmi_vdev_param_disable_htprotection,
	wmi_vdev_param_sta_quickkickout,
	wmi_vdev_param_mgmt_rate,
	wmi_vdev_param_protection_mode,
	wmi_vdev_param_fixed_rate,
	wmi_vdev_param_sgi,
	wmi_vdev_param_ldpc,
	wmi_vdev_param_tx_stbc,
	wmi_vdev_param_rx_stbc,
	wmi_vdev_param_intra_bss_fwd,
	wmi_vdev_param_def_keyid,
	wmi_vdev_param_nss,
	wmi_vdev_param_bcast_data_rate,
	wmi_vdev_param_mcast_data_rate,
	wmi_vdev_param_mcast_indicate,
	wmi_vdev_param_dhcp_indicate,
	wmi_vdev_param_unknown_dest_indicate,
	wmi_vdev_param_ap_keepalive_min_idle_inactive_time_secs,
	wmi_vdev_param_ap_keepalive_max_idle_inactive_time_secs,
	wmi_vdev_param_ap_keepalive_max_unresponsive_time_secs,
	wmi_vdev_param_ap_enable_nawds,
	wmi_vdev_param_mcast2ucast_set,
	wmi_vdev_param_enable_rtscts,
	wmi_vdev_param_rc_num_retries,
	wmi_vdev_param_txbf,
	wmi_vdev_param_packet_powersave,
	wmi_vdev_param_drop_unencry,
	wmi_vdev_param_tx_encap_type,
	wmi_vdev_param_ap_detect_out_of_sync_sleeping_sta_time_secs,
	wmi_vdev_param_cabq_maxdur,
	wmi_vdev_param_mfptest_set,
	wmi_vdev_param_rts_fixed_rate,
	wmi_vdev_param_vht_sgimask,
	wmi_vdev_param_vht80_ratemask,
	wmi_vdev_param_early_rx_adjust_enable,
	wmi_vdev_param_early_rx_tgt_bmiss_num,
	wmi_vdev_param_early_rx_bmiss_sample_cycle,
	wmi_vdev_param_early_rx_slop_step,
	wmi_vdev_param_early_rx_init_slop,
	wmi_vdev_param_early_rx_adjust_pause,
	wmi_vdev_param_proxy_sta,
	wmi_vdev_param_meru_vc,
	wmi_vdev_param_rx_decap_type,
	wmi_vdev_param_bw_nss_ratemask,
	wmi_vdev_param_sensor_ap,
	wmi_vdev_param_beacon_rate,
	wmi_vdev_param_dtim_enable_cts,
	wmi_vdev_param_sta_kickout,
	wmi_vdev_param_tx_pwrlimit,
	wmi_vdev_param_snr_num_for_cal,
	wmi_vdev_param_roam_fw_offload,
	wmi_vdev_param_enable_rmc,
	wmi_vdev_param_ibss_max_bcn_lost_ms,
	wmi_vdev_param_max_rate,
	wmi_vdev_param_early_rx_drift_sample,
	wmi_vdev_param_set_ibss_tx_fail_cnt_thr,
	wmi_vdev_param_ebt_resync_timeout,
	wmi_vdev_param_aggr_trig_event_enable,
	wmi_vdev_param_is_ibss_power_save_allowed,
	wmi_vdev_param_is_power_collapse_allowed,
	wmi_vdev_param_is_awake_on_txrx_enabled,
	wmi_vdev_param_inactivity_cnt,
	wmi_vdev_param_txsp_end_inactivity_time_ms,
	wmi_vdev_param_dtim_policy,
	wmi_vdev_param_ibss_ps_warmup_time_secs,
	wmi_vdev_param_ibss_ps_1rx_chain_in_atim_window_enable,
	wmi_vdev_param_rx_leak_window,
	wmi_vdev_param_stats_avg_factor,
	wmi_vdev_param_disconnect_th,
	wmi_vdev_param_rtscts_rate,
	wmi_vdev_param_mcc_rtscts_protection_enable,
	wmi_vdev_param_mcc_broadcast_probe_enable,
	wmi_vdev_param_capabilities,
	wmi_vdev_param_mgmt_tx_power,
	wmi_vdev_param_atf_ssid_sched_policy,
	wmi_vdev_param_disable_dyn_bw_rts,
	wmi_vdev_param_ampdu_subframe_size_per_ac,
	wmi_vdev_param_he_dcm_enable,
	wmi_vdev_param_he_bss_color,
	wmi_vdev_param_he_range_ext_enable,
	wmi_vdev_param_set_hemu_mode,
	wmi_vdev_param_set_he_ltf,
	wmi_vdev_param_set_heop,
	wmi_vdev_param_disable_cabq,
	wmi_vdev_param_rate_dropdown_bmap,
	wmi_vdev_param_tx_power,
	wmi_vdev_param_set_ba_mode,
	wmi_vdev_param_max,
} wmi_conv_vdev_param_id;

/**
 * Host based ENUM IDs for service bits to abstract target enums
 */
typedef enum {
	wmi_service_beacon_offload = 0,
	wmi_service_scan_offload,
	wmi_service_roam_offload,
	wmi_service_bcn_miss_offload,
	wmi_service_sta_pwrsave,
	wmi_service_sta_advanced_pwrsave,
	wmi_service_ap_uapsd,
	wmi_service_ap_dfs,
	wmi_service_11ac,
	wmi_service_blockack,
	wmi_service_phyerr,
	wmi_service_bcn_filter,
	wmi_service_rtt,
	wmi_service_ratectrl,
	wmi_service_wow,
	wmi_service_ratectrl_cache,
	wmi_service_iram_tids,
	wmi_service_burst,
	wmi_service_smart_antenna_sw_support,
	wmi_service_gtk_offload,
	wmi_service_scan_sch,
	wmi_service_csa_offload,
	wmi_service_chatter,
	wmi_service_coex_freqavoid,
	wmi_service_packet_power_save,
	wmi_service_force_fw_hang,
	wmi_service_smart_antenna_hw_support,
	wmi_service_gpio,
	wmi_sta_uapsd_basic_auto_trig,
	wmi_sta_uapsd_var_auto_trig,
	wmi_service_sta_keep_alive,
	wmi_service_tx_encap,
	wmi_service_ap_ps_detect_out_of_sync,
	wmi_service_early_rx,
	wmi_service_enhanced_proxy_sta,
	wmi_service_tt,
	wmi_service_atf,
	wmi_service_peer_caching,
	wmi_service_coex_gpio,
	wmi_service_aux_spectral_intf,
	wmi_service_aux_chan_load_intf,
	wmi_service_bss_channel_info_64,
	wmi_service_ext_res_cfg_support,
	wmi_service_mesh,
	wmi_service_restrt_chnl_support,
	wmi_service_roam_scan_offload,
	wmi_service_arpns_offload,
	wmi_service_nlo,
	wmi_service_sta_dtim_ps_modulated_dtim,
	wmi_service_sta_smps,
	wmi_service_fwtest,
	wmi_service_sta_wmmac,
	wmi_service_tdls,
	wmi_service_mcc_bcn_interval_change,
	wmi_service_adaptive_ocs,
	wmi_service_ba_ssn_support,
	wmi_service_filter_ipsec_natkeepalive,
	wmi_service_wlan_hb,
	wmi_service_lte_ant_share_support,
	wmi_service_batch_scan,
	wmi_service_qpower,
	wmi_service_plmreq,
	wmi_service_thermal_mgmt,
	wmi_service_rmc,
	wmi_service_mhf_offload,
	wmi_service_coex_sar,
	wmi_service_bcn_txrate_override,
	wmi_service_nan,
	wmi_service_l1ss_stat,
	wmi_service_estimate_linkspeed,
	wmi_service_obss_scan,
	wmi_service_tdls_offchan,
	wmi_service_tdls_uapsd_buffer_sta,
	wmi_service_tdls_uapsd_sleep_sta,
	wmi_service_ibss_pwrsave,
	wmi_service_lpass,
	wmi_service_extscan,
	wmi_service_d0wow,
	wmi_service_hsoffload,
	wmi_service_roam_ho_offload,
	wmi_service_rx_full_reorder,
	wmi_service_dhcp_offload,
	wmi_service_sta_rx_ipa_offload_support,
	wmi_service_mdns_offload,
	wmi_service_sap_auth_offload,
	wmi_service_dual_band_simultaneous_support,
	wmi_service_ocb,
	wmi_service_ap_arpns_offload,
	wmi_service_per_band_chainmask_support,
	wmi_service_packet_filter_offload,
	wmi_service_mgmt_tx_htt,
	wmi_service_mgmt_tx_wmi,
	wmi_service_ext_msg,
	wmi_service_mawc,

	wmi_service_peer_stats,
	wmi_service_mesh_11s,
	wmi_service_periodic_chan_stat_support,
	wmi_service_tx_mode_push_only,
	wmi_service_tx_mode_push_pull,
	wmi_service_tx_mode_dynamic,
	wmi_service_check_cal_version,
	wmi_service_btcoex_duty_cycle,
	wmi_service_4_wire_coex_support,
	wmi_service_multiple_vdev_restart,
	wmi_service_peer_assoc_conf,
	wmi_service_egap,
	wmi_service_sta_pmf_offload,
	wmi_service_unified_wow_capability,
	wmi_service_enterprise_mesh,
	wmi_service_bpf_offload,
	wmi_service_sync_delete_cmds,
	wmi_service_ratectrl_limit_max_min_rates,
	wmi_service_nan_data,
	wmi_service_nan_rtt,
	wmi_service_11ax,
	wmi_service_deprecated_replace,
	wmi_service_tdls_conn_tracker_in_host_mode,
	wmi_service_enhanced_mcast_filter,
	wmi_service_half_rate_quarter_rate_support,
	wmi_service_vdev_rx_filter,
	wmi_service_p2p_listen_offload_support,
	wmi_service_mark_first_wakeup_packet,
	wmi_service_multiple_mcast_filter_set,
	wmi_service_host_managed_rx_reorder,
	wmi_service_flash_rdwr_support,
	wmi_service_wlan_stats_report,
	wmi_service_tx_msdu_id_new_partition_support,
	wmi_service_dfs_phyerr_offload,
	wmi_service_rcpi_support,
	wmi_service_fw_mem_dump_support,
	wmi_service_peer_stats_info,
	wmi_service_regulatory_db,
	wmi_service_11d_offload,
	wmi_service_hw_data_filtering,
	wmi_service_pkt_routing,
	wmi_service_offchan_tx_wmi,
	wmi_service_chan_load_info,
	wmi_service_extended_nss_support,
	wmi_service_ack_timeout,
	wmi_service_widebw_scan,
	wmi_service_bcn_offload_start_stop_support,
	wmi_service_offchan_data_tid_support,
	wmi_service_support_dma,
	wmi_service_8ss_tx_bfee,
	wmi_service_fils_support,
	wmi_service_mawc_support,
	wmi_service_wow_wakeup_by_timer_pattern,
	wmi_service_11k_neighbour_report_support,
	wmi_service_ap_obss_detection_offload,
	wmi_service_bss_color_offload,
	wmi_service_gmac_offload_support,

	wmi_services_max,
} wmi_conv_service_ids;
#define WMI_SERVICE_UNAVAILABLE 0xFFFF

/**
 * enum WMI_DBG_PARAM - Debug params
 * @WMI_DBGLOG_LOG_LEVEL: Set the loglevel
 * @WMI_DBGLOG_VAP_ENABLE:  Enable VAP level debug
 * @WMI_DBGLOG_VAP_DISABLE: Disable VAP level debug
 * @WMI_DBGLOG_MODULE_ENABLE: Enable MODULE level debug
 * @WMI_DBGLOG_MODULE_DISABLE: Disable MODULE level debug
 * @WMI_DBGLOG_MOD_LOG_LEVEL: Enable MODULE level debug
 * @WMI_DBGLOG_TYPE: set type of the debug output
 * @WMI_DBGLOG_REPORT_ENABLE: Enable Disable debug
 */
typedef enum {
	WMI_DBGLOG_LOG_LEVEL = 0x1,
	WMI_DBGLOG_VAP_ENABLE,
	WMI_DBGLOG_VAP_DISABLE,
	WMI_DBGLOG_MODULE_ENABLE,
	WMI_DBGLOG_MODULE_DISABLE,
	WMI_DBGLOG_MOD_LOG_LEVEL,
	WMI_DBGLOG_TYPE,
	WMI_DBGLOG_REPORT_ENABLE
} WMI_DBG_PARAM;

/**
 * struct wmi_host_fw_ver - FW version in non-tlv target
 * @sw_version: Versin info
 * @sw_version_1: Second dword of version
 */
struct wmi_host_fw_ver {
	uint32_t    sw_version;
	uint32_t    sw_version_1;
};

/**
 * struct wmi_host_fw_abi_ver - FW version in non-tlv target
 * @sw_version: Versin info
 * @abi_version: ABI version
 */
struct wmi_host_fw_abi_ver {
	uint32_t    sw_version;
	uint32_t    abi_version;
};

/**
 * struct target_resource_config - Resource config sent from host to target
 *               abstracted out to include union of both configs
 * @num_vdevs: Number vdevs configured
 * @num_peers: Number of peers
 * @num_active_peers: Number of active peers for peer cache
 * @num_offload_peers: Number of offload peers
 * @num_offload_reorder_buffs: number of offload reorder buffs
 * @num_peer_keys: number of peer keys
 * @num_tids: number of tids
 * @ast_skid_limit: AST skid limit
 * @tx_chain_mask: TX chain mask
 * @rx_chain_mask: RX chain mask
 * @rx_timeout_pri: RX reorder timeout per AC
 * @rx_decap_mode: RX decap mode
 * @scan_max_pending_req: Scan mac pending req
 * @bmiss_offload_max_vdev: Beacom miss offload max vdevs
 * @roam_offload_max_vdev: Roam offload max vdevs
 * @roam_offload_max_ap_profiles: roam offload max ap profiles
 * @num_mcast_groups: num mcast groups
 * @num_mcast_table_elems: number of macst table elems
 * @mcast2ucast_mode: mcast enhance mode
 * @tx_dbg_log_size: DBG log buf size
 * @num_wds_entries: number of WDS entries
 * @dma_burst_size: DMA burst size.
 * @mac_aggr_delim: Mac aggr delim
 * @rx_skip_defrag_timeout_dup_detection_check: Defrag dup check in host?
 * @vow_config: vow configuration
 * @gtk_offload_max_vdev: Max vdevs for GTK offload
 * @num_msdu_desc: Number of msdu desc
 * @max_frag_entries: Max frag entries
 *      End common
 * @max_peer_ext_stats: Max peer EXT stats
 * @smart_ant_cap: Smart antenna capabilities
 * @BK_Minfree: BIN configuration for BK traffic
 * @BE_Minfree: BIN configuration for BE traffic
 * @VI_Minfree: BIN configuration for VI traffic
 * @VO_Minfree: BIN configuration for VO traffic
 * @rx_batchmode: RX batch mode
 * @tt_support: Thermal throttling support
 * @atf_config: ATF config
 * @mgmt_comp_evt_bundle_support: bundle support required for mgmt complete evt
 * @tx_msdu_new_partition_id_support: new partiition id support for tx msdu
 * @iphdr_pad_config: ipheader pad config
 * @qwrap_config: Qwrap configuration
 * @alloc_frag_desc_for_data_pkt: Frag desc for data
 *      Added in MCL
 * @num_tdls_vdevs:
 * @num_tdls_conn_table_entries:
 * @beacon_tx_offload_max_vdev:
 * @num_multicast_filter_entries:
 * @num_wow_filters:
 * @num_keep_alive_pattern:
 * @keep_alive_pattern_size:
 * @max_tdls_concurrent_sleep_sta:
 * @max_tdls_concurrent_buffer_sta:
 * @wmi_send_separate:
 * @num_ocb_vdevs:
 * @num_ocb_channels:
 * @num_ocb_schedules:
 * @num_ns_ext_tuples_cfg:
 * @bpf_instruction_size:
 * @max_bssid_rx_filters:
 * @use_pdev_id:
 * @max_num_dbs_scan_duty_cycle: max dbs can duty cycle value
 * @cce_disable: disable cce component
 */
typedef struct {
	uint32_t num_vdevs;
	uint32_t num_peers;
	uint32_t num_active_peers;
	uint32_t num_offload_peers;
	uint32_t num_offload_reorder_buffs;
	uint32_t num_peer_keys;
	uint32_t num_tids;
	uint32_t ast_skid_limit;
	uint32_t tx_chain_mask;
	uint32_t rx_chain_mask;
	uint32_t rx_timeout_pri[4];
	uint32_t rx_decap_mode;
	uint32_t scan_max_pending_req;
	uint32_t bmiss_offload_max_vdev;
	uint32_t roam_offload_max_vdev;
	uint32_t roam_offload_max_ap_profiles;
	uint32_t num_mcast_groups;
	uint32_t num_mcast_table_elems;
	uint32_t mcast2ucast_mode;
	uint32_t tx_dbg_log_size;
	uint32_t num_wds_entries;
	uint32_t dma_burst_size;
	uint32_t mac_aggr_delim;
	uint32_t rx_skip_defrag_timeout_dup_detection_check;
	uint32_t vow_config;
	uint32_t gtk_offload_max_vdev;
	uint32_t num_msdu_desc; /* Number of msdu desc */
	uint32_t max_frag_entries;
	uint32_t scheduler_params;
	/* End common */

	/* Added for Beeliner */
	uint32_t max_peer_ext_stats;
	uint32_t smart_ant_cap;
	uint32_t BK_Minfree;
	uint32_t BE_Minfree;
	uint32_t VI_Minfree;
	uint32_t VO_Minfree;
	uint32_t rx_batchmode;
	uint32_t tt_support;
	uint32_t atf_config:1,
		 mgmt_comp_evt_bundle_support:1,
		 tx_msdu_new_partition_id_support:1;
	uint32_t iphdr_pad_config;
	uint32_t
		qwrap_config:16,
		alloc_frag_desc_for_data_pkt:16;

	/* Added in MCL */
	uint32_t num_tdls_vdevs;
	uint32_t num_tdls_conn_table_entries;
	uint32_t beacon_tx_offload_max_vdev;
	uint32_t num_multicast_filter_entries;
	uint32_t num_wow_filters;
	uint32_t num_keep_alive_pattern;
	uint32_t keep_alive_pattern_size;
	uint32_t max_tdls_concurrent_sleep_sta;
	uint32_t max_tdls_concurrent_buffer_sta;
	uint32_t wmi_send_separate;
	uint32_t num_ocb_vdevs;
	uint32_t num_ocb_channels;
	uint32_t num_ocb_schedules;
	uint32_t num_ns_ext_tuples_cfg;
	uint32_t bpf_instruction_size;
	uint32_t max_bssid_rx_filters;
	uint32_t use_pdev_id;
	uint32_t max_num_dbs_scan_duty_cycle;
	bool cce_disable;
} target_resource_config;

/**
 * struct wds_addr_event - WDS addr event structure
 * @event_type: event type add/delete
 * @peer_mac: peer mac
 * @dest_mac: destination mac address
 * @vdev_id: vdev id
 */
typedef struct {
	uint32_t event_type[4];
	u_int8_t peer_mac[IEEE80211_ADDR_LEN];
	u_int8_t dest_mac[IEEE80211_ADDR_LEN];
	uint32_t vdev_id;
} wds_addr_event_t;
/**
 * Enum replicated for host abstraction with FW
 */
typedef enum {
	/* Event respose of START CMD */
	WMI_HOST_VDEV_START_RESP_EVENT = 0,
	/* Event respose of RESTART CMD */
	WMI_HOST_VDEV_RESTART_RESP_EVENT,
} WMI_HOST_START_EVENT_PARAM;

/**
 * struct wmi_host_vdev_start_resp - VDEV start response
 * @vdev_id: vdev id
 * @requestor_id: requestor id that requested the VDEV start request
 * @resp_type: Respose of Event type START/RESTART
 * @status: status of the response
 * @chain_mask: Vdev chain mask
 * @smps_mode: Vdev mimo power save mode
 * @mac_id: mac_id field contains the MAC identifier that the
 *          VDEV is bound to. The valid range is 0 to (num_macs-1).
 * @cfgd_tx_streams: Configured Transmit Streams
 * @cfgd_rx_streams: Configured Receive Streams
 */
typedef struct {
	uint32_t vdev_id;
	uint32_t requestor_id;
	WMI_HOST_START_EVENT_PARAM resp_type;
	uint32_t status;
	uint32_t chain_mask;
	uint32_t smps_mode;
	uint32_t mac_id;
	uint32_t cfgd_tx_streams;
	uint32_t cfgd_rx_streams;
} wmi_host_vdev_start_resp;

/**
 * struct wmi_host_vdev_delete_resp - VDEV delete response
 * @vdev_id: vdev id
 */
struct wmi_host_vdev_delete_resp {
	uint32_t vdev_id;
};

/**
 * struct wmi_host_roam_event - host roam event param
 * @vdev_id: vdev id
 * @reason: roam reason
 * @rssi: RSSI
 */
typedef struct {
	uint32_t vdev_id;
	uint32_t reason;
	uint32_t rssi;
} wmi_host_roam_event;

/**
 * ENUM wmi_host_scan_event_type - Scan event type
 */
enum wmi_host_scan_event_type {
	WMI_HOST_SCAN_EVENT_STARTED = 0x1,
	WMI_HOST_SCAN_EVENT_COMPLETED = 0x2,
	WMI_HOST_SCAN_EVENT_BSS_CHANNEL = 0x4,
	WMI_HOST_SCAN_EVENT_FOREIGN_CHANNEL = 0x8,
	WMI_HOST_SCAN_EVENT_DEQUEUED = 0x10,
	WMI_HOST_SCAN_EVENT_PREEMPTED = 0x20,
	WMI_HOST_SCAN_EVENT_START_FAILED = 0x40,
	WMI_HOST_SCAN_EVENT_RESTARTED = 0x80,
	WMI_HOST_SCAN_EVENT_FOREIGN_CHANNEL_EXIT = 0x100,
	WMI_HOST_SCAN_EVENT_INVALID = 0x200,
	WMI_HOST_SCAN_EVENT_GPIO_TIMEOUT = 0x400,
	WMI_HOST_SCAN_EVENT_MAX = 0x8000
};

/**
 * ENUM wmi_host_scan_completion_reason - Scan completion event type
 */
enum wmi_host_scan_completion_reason {
	/** scan related events */
	WMI_HOST_SCAN_REASON_NONE = 0xFF,
	WMI_HOST_SCAN_REASON_COMPLETED = 0,
	WMI_HOST_SCAN_REASON_CANCELLED = 1,
	WMI_HOST_SCAN_REASON_PREEMPTED = 2,
	WMI_HOST_SCAN_REASON_TIMEDOUT = 3,
	WMI_HOST_SCAN_REASON_INTERNAL_FAILURE = 4,
	WMI_HOST_SCAN_REASON_MAX,
};

/**
 * struct wmi_host_scan_event - Scan event response from target
 * @event: event type
 * @reason: Reason for event
 * @channel_freq: channel frequency
 * @requestor: requestor id
 * @scan_id: scan id
 * @vdev_id: vdev id
 */
typedef struct {
	uint32_t event;
	uint32_t reason;
	uint32_t channel_freq;
	uint32_t requestor;
	uint32_t scan_id;
	uint32_t vdev_id;
} wmi_host_scan_event;

/**
 * struct wmi_host_pdev_reserve_ast_entry_event - Reserve AST entry
 * @result: result
 */
typedef struct {
	uint32_t result;
} wmi_host_pdev_reserve_ast_entry_event;

/**
 * struct wmi_host_mcast_ageout_entry - mcast aged-out entry
 * @grp_addr: IPv4/6 mcast group addr
 * @vdev_id: vdev id
 */
typedef struct {
	uint8_t grp_addr[16];
	uint32_t vdev_id;
} wmi_host_mcast_ageout_entry;

/**
 * struct wmi_host_mcast_list_ageout_event - List of mcast entry aged-out
 * @num_entry: Number of mcast entries timed-out
 * @entry: List of wmi_host_mcast_ageout_entry
 */
typedef struct {
	uint32_t num_entry;
	wmi_host_mcast_ageout_entry entry[1];
} wmi_host_mcast_list_ageout_event;

/**
 * struct wmi_host_pdev_nfcal_power_all_channels_event - NF cal event data
 * @nfdbr:
 *   chan[0 ~ 7]: {NFCalPower_chain0, NFCalPower_chain1,
 *                 NFCalPower_chain2, NFCalPower_chain3,
 *                 NFCalPower_chain4, NFCalPower_chain5,
 *                 NFCalPower_chain6, NFCalPower_chain7},
 * @nfdbm:
 *   chan[0 ~ 7]: {NFCalPower_chain0, NFCalPower_chain1,
 *                 NFCalPower_chain2, NFCalPower_chain3,
 *                 NFCalPower_chain4, NFCalPower_chain5,
 *                 NFCalPower_chain6, NFCalPower_chain7},
 * @freqnum:
 *   chan[0 ~ 7]: frequency number
 * @pdev_id: pdev_id
 */
typedef struct {
	int8_t nfdbr[WMI_HOST_RXG_CAL_CHAN_MAX * WMI_HOST_MAX_NUM_CHAINS];
	int8_t nfdbm[WMI_HOST_RXG_CAL_CHAN_MAX * WMI_HOST_MAX_NUM_CHAINS];
	uint32_t freqnum[WMI_HOST_RXG_CAL_CHAN_MAX];
	uint32_t pdev_id;
} wmi_host_pdev_nfcal_power_all_channels_event;

/**
 * enum wmi_host_pdev_tpc_event_offset: offsets of TPC events
 * @WMI_HOST_TX_POWER_MAX: offset of max tx power
 * @WMI_HOST_TX_POWER_MIN: offset of min tx power
 * @WMI_HOST_TX_POWER_LEN: size of tpc values
 */
enum wmi_host_pdev_tpc_event_offset {
	WMI_HOST_TX_POWER_MAX,
	WMI_HOST_TX_POWER_MIN,
	WMI_HOST_TX_POWER_LEN,
};

/**
 * struct wmi_host_pdev_tpc_event - WMI host pdev TPC event
 * @pdev_id: pdev_id
 * @tpc:
 */
typedef struct {
	uint32_t pdev_id;
	int32_t tpc[WMI_HOST_TX_POWER_LEN];
} wmi_host_pdev_tpc_event;

/**
 * struct wmi_host_pdev_generic_buffer_event
 * @buf_type: Buffer type
 * @frag_id: Frag id
 * @more_frag: more frags pending
 * @buf_len: buffer length
 * @buf_info: variable length buffer
 */
typedef struct {
	uint32_t buf_type;
	uint32_t frag_id;
	uint32_t more_frag;
	uint32_t buf_len;
	uint32_t buf_info[1];
} wmi_host_pdev_generic_buffer_event;
/**
 * Enum for host buffer event
 */
enum {
	WMI_HOST_BUFFER_TYPE_RATEPWR_TABLE,
	WMI_HOST_BUFFER_TYPE_CTL_TABLE,
};

/**
 * struct wmi_host_pdev_tpc_config_event - host pdev tpc config event
 * @pdev_id: pdev_id
 * @regDomain:
 * @chanFreq:
 * @phyMode:
 * @twiceAntennaReduction:
 * @twiceMaxRDPower:
 * @twiceAntennaGain:
 * @powerLimit:
 * @rateMax:
 * @numTxChain:
 * @ctl:
 * @flags:
 * @maxRegAllowedPower:
 * @maxRegAllowedPowerAGCDD:
 * @maxRegAllowedPowerAGSTBC:
 * @maxRegAllowedPowerAGTXBF:
 * @ratesArray:
 */
typedef struct {
	uint32_t pdev_id;
	uint32_t regDomain;
	uint32_t chanFreq;
	uint32_t phyMode;
	uint32_t twiceAntennaReduction;
	uint32_t twiceMaxRDPower;
	int32_t  twiceAntennaGain;
	uint32_t powerLimit;
	uint32_t rateMax;
	uint32_t numTxChain;
	uint32_t ctl;
	uint32_t flags;
	int8_t  maxRegAllowedPower[WMI_HOST_TPC_TX_NUM_CHAIN];
	int8_t  maxRegAllowedPowerAGCDD[WMI_HOST_TPC_TX_NUM_CHAIN][WMI_HOST_TPC_TX_NUM_CHAIN];
	int8_t  maxRegAllowedPowerAGSTBC[WMI_HOST_TPC_TX_NUM_CHAIN][WMI_HOST_TPC_TX_NUM_CHAIN];
	int8_t  maxRegAllowedPowerAGTXBF[WMI_HOST_TPC_TX_NUM_CHAIN][WMI_HOST_TPC_TX_NUM_CHAIN];
	uint8_t ratesArray[WMI_HOST_TPC_RATE_MAX];
} wmi_host_pdev_tpc_config_event;
/**
 * Enums for TPC event
 */
typedef enum {
	WMI_HOST_TPC_CONFIG_EVENT_FLAG_TABLE_CDD	= 0x1,
	WMI_HOST_TPC_CONFIG_EVENT_FLAG_TABLE_STBC	= 0x2,
	WMI_HOST_TPC_CONFIG_EVENT_FLAG_TABLE_TXBF	= 0x4,
} WMI_HOST_TPC_CONFIG_EVENT_FLAG;

/**
 * Medium Utilization evaluation algorithms
 * These algorithms can be complementary rather than exclusive.
 */
typedef enum {
	WMI_HOST_MU_BASIC_ALGO = 0x1,
	WMI_HOST_MU_PER_BSSID_ALGO = 0x2,
	WMI_HOST_MU_HIDDEN_NODE_ALGO = 0x4,
} WMI_HOST_MU_ALGO_TYPE;
/* max MU alg combinations supported by target */
#define WMI_HOST_MU_MAX_ALGO_TYPE 3

/**
 * struct wmi_host_mu_db_entry
 * @event_type: 0=AP, 1=STA, 2=Small Cell(SC)
 * @bssid_mac_addr: Transmitter MAC if entry is WiFi node. PLMNID if SC
 * @tx_addr: Transmitter MAC if entry is WiFi node. PLMNID if SC
 * @avg_duration_us: Avg. duration for which node was transmitting
 * @avg_rssi: Avg. RSSI of all TX packets by node. Unit dBm
 * @mu_percent: % medium utilization by node
 */
typedef struct {
	uint32_t     entry_type;
	wmi_host_mac_addr bssid_mac_addr;
	wmi_host_mac_addr tx_addr;
	uint32_t     avg_duration_us;
	uint32_t     avg_rssi;
	uint32_t     mu_percent;
} wmi_host_mu_db_entry;

/**
 * struct wmi_host_mu_report_event - WMI_MU_REPORT_EVENTID
 * @mu_request_id: request id
 * @status_reason: MU_STATUS_REASON
 * @total_mu: MU_ALG_TYPE combinations
 * @num_active_bssid: number of active bssid
 * @hidden_node_mu : hidden node algo MU per bin
 * @num_TA_entries : No. of entries found in MU db report
 */
typedef struct {
	uint32_t mu_request_id;
	uint32_t status_reason;
	uint32_t total_mu[WMI_HOST_MU_MAX_ALGO_TYPE];
	uint32_t num_active_bssid;
	uint32_t hidden_node_mu[LTEU_MAX_BINS];
	uint32_t num_TA_entries;
} wmi_host_mu_report_event;

/**
 * struct wmi_host_mgmt_tx_compl_event - TX completion event
 * @desc_id: from tx_send_cmd
 * @status: WMI_MGMT_TX_COMP_STATUS_TYPE
 * @pdev_id: pdev_id
 */
typedef struct {
	uint32_t	desc_id;
	uint32_t	status;
	uint32_t	pdev_id;
} wmi_host_mgmt_tx_compl_event;

/**
 * struct wmi_host_offchan_data_tx_compl_event - TX completion event
 * @desc_id: from tx_send_cmd
 * @status: VWMI_MGMT_TX_COMP_STATUS_TYPE
 * @pdev_id: pdev_id
 */
struct wmi_host_offchan_data_tx_compl_event {
	uint32_t desc_id;
	uint32_t status;
	uint32_t pdev_id;
};

#define WMI_HOST_TIM_BITMAP_ARRAY_SIZE 17

/**
 * struct wmi_host_tim_info - TIM info in SWBA event
 * @tim_len: TIM length
 * @tim_mcast:
 * @tim_bitmap: TIM bitmap
 * @tim_changed: TIM changed
 * @tim_num_ps_pending: TIM num PS sta pending
 * @vdev_id: Vdev id
 */
typedef struct {
	uint32_t tim_len;
	uint32_t tim_mcast;
	uint32_t tim_bitmap[WMI_HOST_TIM_BITMAP_ARRAY_SIZE];
	uint32_t tim_changed;
	uint32_t tim_num_ps_pending;
	uint32_t vdev_id;
} wmi_host_tim_info;

/**
 * struct wmi_host_p2p_noa_descriptor - NoA desc in SWBA event
 * @type_count: Absence count
 * @duration: NoA duration
 * @interval: NoA interval
 * @start_time: start time
 */
typedef struct {
	uint32_t type_count;
	uint32_t duration;
	uint32_t interval;
	uint32_t start_time;
} wmi_host_p2p_noa_descriptor;
/* Maximum number of NOA Descriptors supported */
#define WMI_HOST_P2P_MAX_NOA_DESCRIPTORS 4
/**
 * struct wmi_host_p2p_noa_info - p2p noa information
 * @modified: NoA modified
 * @index: Index
 * @oppPS: Oppurtunstic ps
 * @ctwindow: CT window
 * @num_descriptors: number of descriptors
 * @noa_descriptors: noa descriptors
 * @vdev_id: Vdev id
 */
typedef struct {
	uint8_t modified;
	uint8_t index;
	uint8_t oppPS;
	uint8_t ctwindow;
	uint8_t num_descriptors;
	wmi_host_p2p_noa_descriptor
		noa_descriptors[WMI_HOST_P2P_MAX_NOA_DESCRIPTORS];
	uint32_t vdev_id;
} wmi_host_p2p_noa_info;

/**
 * struct wmi_host_peer_sta_kickout_event
 * @peer_macaddr: peer mac address
 * @reason: kickout reason
 * @rssi: rssi
 * @pdev_id: pdev_id
 */
typedef struct {
	uint8_t peer_macaddr[IEEE80211_ADDR_LEN];
	uint32_t reason;
	uint32_t rssi;
} wmi_host_peer_sta_kickout_event;

/**
 * struct wmi_host_peer_sta_ps_statechange_event - ST ps state change event
 * @peer_macaddr: peer mac address
 * @peer_ps_stats: peer PS state
 * @pdev_id: pdev_id
 */
typedef struct {
	uint8_t peer_macaddr[IEEE80211_ADDR_LEN];
	uint32_t peer_ps_state;
} wmi_host_peer_sta_ps_statechange_event;

/* Maximum CCK, OFDM rates supported */
#define WMI_SA_MAX_CCK_OFDM_RATES 12
/* Maximum MCS rates supported; 4 rates in each dword */
#define WMI_SA_MAX_MCS_RATES 40
#define WMI_SA_MAX_RATE_COUNTERS 4
/* Maximum rate series used for transmission */
#define SA_MAX_RATE_SERIES 2

#define SA_MAX_LEGACY_RATE_DWORDS 3
#define SA_MAX_HT_RATE_DWORDS 10
#define SA_BYTES_IN_DWORD 4
#define SA_MASK_BYTE 0xff
/* TODO: ratecode_160 needs to add for future chips */
/**
 * struct wmi_sa_rate_cap - smart antenna rat capabilities
 * @pdev_id: pdev_id
 * @ratecode_legacy: Rate code array for CCK OFDM
 * @ratecode_20: Rate code array for 20MHz BW
 * @ratecode_40: Rate code array for 40MHz BW
 * @ratecode_80: Rate code array for 80MHz BW
 * @ratecount: Max Rate count for each mode
 */
typedef struct {
	uint8_t ratecode_legacy[WMI_SA_MAX_CCK_OFDM_RATES];
	uint8_t ratecode_20[WMI_SA_MAX_MCS_RATES];
	uint8_t ratecode_40[WMI_SA_MAX_MCS_RATES];
	uint8_t ratecode_80[WMI_SA_MAX_MCS_RATES];
	uint8_t ratecount[WMI_SA_MAX_RATE_COUNTERS];
} wmi_sa_rate_cap;

/** Preamble types to be used with VDEV fixed rate configuration */
typedef enum {
	WMI_HOST_RATE_PREAMBLE_OFDM,
	WMI_HOST_RATE_PREAMBLE_CCK,
	WMI_HOST_RATE_PREAMBLE_HT,
	WMI_HOST_RATE_PREAMBLE_VHT,
	WMI_HOST_RATE_PREAMBLE_HE,
} WMI_HOST_RATE_PREAMBLE;

#define WMI_HOST_FIXED_RATE_NONE	(0xff)

/** slot time long */
#define WMI_HOST_VDEV_SLOT_TIME_LONG	0x1
/** slot time short */
#define WMI_HOST_VDEV_SLOT_TIME_SHORT	0x2
/** preablbe long */
#define WMI_HOST_VDEV_PREAMBLE_LONG	0x1
/** preablbe short */
#define WMI_HOST_VDEV_PREAMBLE_SHORT	0x2
/** found a better AP */
#define WMI_HOST_ROAM_REASON_BETTER_AP  0x1
/** beacon miss detected */
#define WMI_HOST_ROAM_REASON_BMISS	0x2
/** deauth/disassoc received */
#define WMI_HOST_ROAM_REASON_DEAUTH	0x2
/** connected AP's low rssi condition detected */
#define WMI_HOST_ROAM_REASON_LOW_RSSI  0x3
/** found another AP that matches SSID and Security profile in
 * WMI_ROAM_AP_PROFILE, found during scan triggered upon FINAL_BMISS
 */
#define WMI_HOST_ROAM_REASON_SUITABLE_AP 0x4
/** LFR3.0 roaming failed, indicate the disconnection to host */
#define WMI_HOST_ROAM_REASON_HO_FAILED  0x5

/** values for vdev_type */
#define WMI_HOST_VDEV_TYPE_AP	0x1
#define WMI_HOST_VDEV_TYPE_STA	0x2
#define WMI_HOST_VDEV_TYPE_IBSS	0x3
#define WMI_HOST_VDEV_TYPE_MONITOR	0x4

/** values for vdev_subtype */
#define WMI_HOST_VDEV_SUBTYPE_P2P_DEVICE 0x1
#define WMI_HOST_VDEV_SUBTYPE_P2P_CLIENT 0x2
#define WMI_HOST_VDEV_SUBTYPE_P2P_GO	0x3
#define WMI_HOST_VDEV_SUBTYPE_PROXY_STA  0x4
#define WMI_HOST_VDEV_SUBTYPE_MESH	0x5

#define WMI_HOST_MGMT_TID	17
/* Disable aging & learning */
#define WMI_HOST_WDS_FLAG_STATIC	0x1

/**
 * Peer param enum abstracted from target
 */
typedef enum {
	/** mimo powersave state */
	WMI_HOST_PEER_MIMO_PS_STATE = 0x1,
	/** enable/disable AMPDU . initial value (enabled) */
	WMI_HOST_PEER_AMPDU = 0x2,
	/** authorize/unauthorize peer. initial value is unauthorized (0)  */
	WMI_HOST_PEER_AUTHORIZE = 0x3,
	/** peer channel bandwidth */
	WMI_HOST_PEER_CHWIDTH = 0x4,
	/** peer NSS */
	WMI_HOST_PEER_NSS = 0x5,
	/** USE 4 ADDR */
	WMI_HOST_PEER_USE_4ADDR = 0x6,
	/** Enable extended peer stats */
	WMI_HOST_PEER_EXT_STATS_ENABLE = 0x7,
	/*Use FIXED Pwr */
	WMI_HOST_PEER_USE_FIXED_PWR = 0x8,
	/* Set peer fixed rate */
	WMI_HOST_PEER_PARAM_FIXED_RATE = 0x9,
	/* Whitelist peer TIDs */
	WMI_HOST_PEER_SET_MU_WHITELIST = 0xa,
	/* set group membership status */
	WMI_HOST_PEER_MEMBERSHIP = 0xb,
	WMI_HOST_PEER_USERPOS = 0xc,
	WMI_HOST_PEER_CRIT_PROTO_HINT_ENABLED = 0xd,
	WMI_HOST_PEER_TX_FAIL_CNT_THR = 0xe,
	WMI_HOST_PEER_SET_HW_RETRY_CTS2S = 0xf,
	WMI_HOST_PEER_IBSS_ATIM_WINDOW_LENGTH = 0x10,
	WMI_HOST_PEER_PHYMODE = 0x11,
	WMI_HOST_PEER_SET_MAC_TX_RATE = 0x12,
	/* Set default Rx routing */
	WMI_HOST_PEER_SET_DEFAULT_ROUTING = 0x13,
	WMI_HOST_PEER_SET_MIN_TX_RATE = 0x14,
	/* peer NSS for 160Mhx */
	WMI_HOST_PEER_NSS_VHT160 = 0x15,
	/* peer NSS for 160Mhx */
	WMI_HOST_PEER_NSS_VHT80_80 = 0x16,
	/* Set SU sounding interval */
	WMI_HOST_PEER_PARAM_SU_TXBF_SOUNDING_INTERVAL = 0x17,
	/* Set MU sounding interval */
	WMI_HOST_PEER_PARAM_MU_TXBF_SOUNDING_INTERVAL = 0x18,
	/* Enable sounding interval set */
	WMI_HOST_PEER_PARAM_TXBF_SOUNDING_ENABLE = 0x19,
	/* Enable MU support */
	WMI_HOST_PEER_PARAM_MU_ENABLE = 0x1a,
	/* Enable OFDMA support */
	WMI_HOST_PEER_PARAM_OFDMA_ENABLE = 0x1b,
} PEER_PARAM_ENUM;
#define WMI_HOST_PEER_MIMO_PS_NONE	0x0
#define WMI_HOST_PEER_MIMO_PS_STATIC	0x1
#define WMI_HOST_PEER_MIMO_PS_DYNAMIC	0x2
typedef enum {
	HOST_PLATFORM_HIGH_PERF,
	HOST_PLATFORM_LOW_PERF,
	HOST_PLATFORM_LOW_PERF_NO_FETCH,
} HOST_PLATFORM_TYPE;

enum wmi_host_sta_ps_mode {
	/** enable power save for the given STA VDEV */
	WMI_HOST_STA_PS_MODE_DISABLED = 0,
	/** disable power save  for a given STA VDEV */
	WMI_HOST_STA_PS_MODE_ENABLED = 1,
};
enum wmi_host_sta_powersave_param {
	/**
	* Controls how frames are retrievd from AP while STA is sleeping
	*
	* (see enum wmi_sta_ps_param_rx_wake_policy)
	*/
	WMI_HOST_STA_PS_PARAM_RX_WAKE_POLICY = 0,

	/**
	* The STA will go active after this many TX
	*
	* (see enum wmi_sta_ps_param_tx_wake_threshold)
	*/
	WMI_HOST_STA_PS_PARAM_TX_WAKE_THRESHOLD = 1,

	/**
	* Number of PS-Poll to send before STA wakes up
	*
	* (see enum wmi_sta_ps_param_pspoll_count)
	*
	*/
	WMI_HOST_STA_PS_PARAM_PSPOLL_COUNT = 2,

	/**
	* TX/RX inactivity time in msec before going to sleep.
	*
	* The power save SM will monitor tx/rx activity on the VDEV, if no
	* activity for the specified msec of the parameter
	* the Power save SM will go to sleep.
	*/
	WMI_HOST_STA_PS_PARAM_INACTIVITY_TIME = 3,

	/**
	* Set uapsd configuration.
	*
	* (see enum wmi_sta_ps_param_uapsd)
	*/
	WMI_HOST_STA_PS_PARAM_UAPSD = 4,
};
/* prefix used by scan requestor ids on the host
 * replicated here form wmi_unified.h*/
#define WMI_HOST_P_SCAN_REQUESTOR_ID_PREFIX 0xA000
/* prefix used by scan request ids generated on the host */
/* host cycles through the lower 12 bits to generate ids */
#define WMI_HOST_P_SCAN_REQ_ID_PREFIX 0xA000

#define WMI_HOST_RC_DS_FLAG	0x01	/* Dual stream flag */
#define WMI_HOST_RC_CW40_FLAG	0x02	/* CW 40 */
#define WMI_HOST_RC_SGI_FLAG	0x04	/* Short Guard Interval */
#define WMI_HOST_RC_HT_FLAG	0x08	/* HT */
#define WMI_HOST_RC_RTSCTS_FLAG	0x10	/* RTS-CTS */
#define WMI_HOST_RC_TX_STBC_FLAG	0x20	/* TX STBC */
#define WMI_HOST_RC_RX_STBC_FLAG	0xC0	/* RX STBC ,2 bits */
#define WMI_HOST_RC_RX_STBC_FLAG_S   6	/* RX STBC ,2 bits */
#define WMI_HOST_RC_WEP_TKIP_FLAG	0x100   /* WEP/TKIP encryption */
#define WMI_HOST_RC_TS_FLAG	0x200   /* Three stream flag */
#define WMI_HOST_RC_UAPSD_FLAG	0x400   /* UAPSD Rate Control */

/** HT Capabilities*/
#define WMI_HOST_HT_CAP_ENABLED	0x0001   /* HT Enabled/ disabled */
/* Short Guard Interval with HT20 */
#define WMI_HOST_HT_CAP_HT20_SGI	0x0002
#define WMI_HOST_HT_CAP_DYNAMIC_SMPS	0x0004   /* Dynamic MIMO powersave */
#define WMI_HOST_HT_CAP_TX_STBC	0x0008   /* B3 TX STBC */
#define WMI_HOST_HT_CAP_TX_STBC_MASK_SHIFT	3
#define WMI_HOST_HT_CAP_RX_STBC	0x0030   /* B4-B5 RX STBC */
#define WMI_HOST_HT_CAP_RX_STBC_MASK_SHIFT	4
#define WMI_HOST_HT_CAP_LDPC	0x0040   /* LDPC supported */
#define WMI_HOST_HT_CAP_L_SIG_TXOP_PROT	0x0080   /* L-SIG TXOP Protection */
#define WMI_HOST_HT_CAP_MPDU_DENSITY	0x0700   /* MPDU Density */
#define WMI_HOST_HT_CAP_MPDU_DENSITY_MASK_SHIFT 8
#define WMI_HOST_HT_CAP_HT40_SGI	0x0800
#define WMI_HOST_HT_CAP_RX_LDPC         0x1000
#define WMI_HOST_HT_CAP_TX_LDPC         0x2000
#define WMI_HOST_HT_CAP_IBF_BFER	0x4000

/* These macros should be used when we wish to advertise STBC support for
 * only 1SS or 2SS or 3SS. */
#define WMI_HOST_HT_CAP_RX_STBC_1SS	0x0010   /* B4-B5 RX STBC */
#define WMI_HOST_HT_CAP_RX_STBC_2SS	0x0020   /* B4-B5 RX STBC */
#define WMI_HOST_HT_CAP_RX_STBC_3SS	0x0030   /* B4-B5 RX STBC */


#define WMI_HOST_HT_CAP_DEFAULT_ALL (WMI_HOST_HT_CAP_ENABLED	| \
	WMI_HOST_HT_CAP_HT20_SGI	| \
	WMI_HOST_HT_CAP_HT40_SGI	| \
	WMI_HOST_HT_CAP_TX_STBC	| \
	WMI_HOST_HT_CAP_RX_STBC	| \
	WMI_HOST_HT_CAP_LDPC)

/* WMI_HOST_VHT_CAP_* these maps to ieee 802.11ac vht capability information
   field. The fields not defined here are not supported, or reserved.
   Do not change these masks and if you have to add new one follow the
   bitmask as specified by 802.11ac draft.
*/

#define WMI_HOST_VHT_CAP_MAX_MPDU_LEN_MASK	0x00000003
#define WMI_HOST_VHT_CAP_RX_LDPC	0x00000010
#define WMI_HOST_VHT_CAP_SGI_80MHZ	0x00000020
#define WMI_HOST_VHT_CAP_SGI_160MHZ	0x00000040
#define WMI_HOST_VHT_CAP_TX_STBC	0x00000080
#define WMI_HOST_VHT_CAP_RX_STBC_MASK	0x00000300
#define WMI_HOST_VHT_CAP_RX_STBC_MASK_SHIFT	8
#define WMI_HOST_VHT_CAP_SU_BFER	0x00000800
#define WMI_HOST_VHT_CAP_SU_BFEE	0x00001000
#define WMI_HOST_VHT_CAP_MAX_CS_ANT_MASK	0x0000E000
#define WMI_HOST_VHT_CAP_MAX_CS_ANT_MASK_SHIFT	13
#define WMI_HOST_VHT_CAP_MAX_SND_DIM_MASK	0x00070000
#define WMI_HOST_VHT_CAP_MAX_SND_DIM_MASK_SHIFT	16
#define WMI_HOST_VHT_CAP_MU_BFER	0x00080000
#define WMI_HOST_VHT_CAP_MU_BFEE	0x00100000
#define WMI_HOST_VHT_CAP_MAX_AMPDU_LEN_EXP	0x03800000
#define WMI_HOST_VHT_CAP_MAX_AMPDU_LEN_EXP_SHIT	23
#define WMI_HOST_VHT_CAP_RX_FIXED_ANT	0x10000000
#define WMI_HOST_VHT_CAP_TX_FIXED_ANT	0x20000000

#define WMI_HOST_VHT_CAP_MAX_MPDU_LEN_11454	0x00000002

/* These macros should be used when we wish to advertise STBC support for
 * only 1SS or 2SS or 3SS. */
#define WMI_HOST_VHT_CAP_RX_STBC_1SS	0x00000100
#define WMI_HOST_VHT_CAP_RX_STBC_2SS	0x00000200
#define WMI_HOST_VHT_CAP_RX_STBC_3SS	0x00000300

#define WMI_HOST_VHT_CAP_DEFAULT_ALL (WMI_HOST_VHT_CAP_MAX_MPDU_LEN_11454  | \
	WMI_HOST_VHT_CAP_SGI_80MHZ	|	\
	WMI_HOST_VHT_CAP_TX_STBC	|	\
	WMI_HOST_VHT_CAP_RX_STBC_MASK	|	\
	WMI_HOST_VHT_CAP_RX_LDPC	|	\
	WMI_HOST_VHT_CAP_MAX_AMPDU_LEN_EXP   |	\
	WMI_HOST_VHT_CAP_RX_FIXED_ANT	|	\
	WMI_HOST_VHT_CAP_TX_FIXED_ANT)

/* Interested readers refer to Rx/Tx MCS Map definition as defined in
   802.11ac
*/
#define WMI_HOST_VHT_MAX_MCS_4_SS_MASK(r, ss) ((3 & (r)) << (((ss) - 1) << 1))
#define WMI_HOST_VHT_MAX_SUPP_RATE_MASK	0x1fff0000
#define WMI_HOST_VHT_MAX_SUPP_RATE_MASK_SHIFT 16

/** U-APSD configuration of peer station from (re)assoc request and TSPECs */
enum wmi_host_ap_ps_param_uapsd {
	WMI_HOST_AP_PS_UAPSD_AC0_DELIVERY_EN = (1 << 0),
	WMI_HOST_AP_PS_UAPSD_AC0_TRIGGER_EN  = (1 << 1),
	WMI_HOST_AP_PS_UAPSD_AC1_DELIVERY_EN = (1 << 2),
	WMI_HOST_AP_PS_UAPSD_AC1_TRIGGER_EN  = (1 << 3),
	WMI_HOST_AP_PS_UAPSD_AC2_DELIVERY_EN = (1 << 4),
	WMI_HOST_AP_PS_UAPSD_AC2_TRIGGER_EN  = (1 << 5),
	WMI_HOST_AP_PS_UAPSD_AC3_DELIVERY_EN = (1 << 6),
	WMI_HOST_AP_PS_UAPSD_AC3_TRIGGER_EN  = (1 << 7),
};
/** U-APSD maximum service period of peer station */
enum wmi_host_ap_ps_peer_param_max_sp {
	WMI_HOST_AP_PS_PEER_PARAM_MAX_SP_UNLIMITED = 0,
	WMI_HOST_AP_PS_PEER_PARAM_MAX_SP_2 = 1,
	WMI_HOST_AP_PS_PEER_PARAM_MAX_SP_4 = 2,
	WMI_HOST_AP_PS_PEER_PARAM_MAX_SP_6 = 3,

	/* keep last! */
	MAX_HOST_WMI_AP_PS_PEER_PARAM_MAX_SP,
};

#define WMI_HOST_UAPSD_AC_TYPE_DELI 0
#define WMI_HOST_UAPSD_AC_TYPE_TRIG 1

#define WMI_HOST_UAPSD_AC_BIT_MASK(ac, type) \
	((type ==  WMI_HOST_UAPSD_AC_TYPE_DELI) ? (1<<(ac<<1)) :\
	 (1<<((ac<<1)+1)))

enum wmi_host_ap_ps_peer_param_wnm_sleep {
	WMI_HOST_AP_PS_PEER_PARAM_WNM_SLEEP_ENABLE,
	WMI_HOST_AP_PS_PEER_PARAM_WNM_SLEEP_DISABLE,
};

enum wmi_host_ap_ps_peer_param {
	/** Set uapsd configuration for a given peer.
	*
	* This will include the delivery and trigger enabled state for every AC.
	* The host  MLME needs to set this based on AP capability and stations
	* request Set in the association request  received from the station.
	*
	* Lower 8 bits of the value specify the UAPSD configuration.
	*
	* (see enum wmi_ap_ps_param_uapsd)
	* The default value is 0.
	*/
	WMI_HOST_AP_PS_PEER_PARAM_UAPSD = 0,

	/**
	* Set the service period for a UAPSD capable station
	*
	* The service period from wme ie in the (re)assoc request frame.
	*
	* (see enum wmi_ap_ps_peer_param_max_sp)
	*/
	WMI_HOST_AP_PS_PEER_PARAM_MAX_SP = 1,

	/** Time in seconds for aging out buffered frames
      * for STA in power save */
	WMI_HOST_AP_PS_PEER_PARAM_AGEOUT_TIME = 2,

	/** Specify frame types that are considered SIFS
      * RESP trigger frame */
	WMI_HOST_AP_PS_PEER_PARAM_SIFS_RESP_FRMTYPE = 3,

	/** Specifies the trigger state of TID.
	* Valid only for UAPSD frame type  */
	WMI_HOST_AP_PS_PEER_PARAM_SIFS_RESP_UAPSD = 4,

	/** Specifies the WNM sleep state of a STA */
	WMI_HOST_AP_PS_PEER_PARAM_WNM_SLEEP = 5,
};
#define WMI_HOST_RXERR_CRC	0x01 /* CRC error on frame */
#define WMI_HOST_RXERR_DECRYPT	0x08 /* non-Michael decrypt error */
#define WMI_HOST_RXERR_MIC	0x10 /* Michael MIC decrypt error */
#define WMI_HOST_RXERR_KEY_CACHE_MISS 0x20 /* No/incorrect key matter in h/w */

enum wmi_host_sta_ps_param_uapsd {
	WMI_HOST_STA_PS_UAPSD_AC0_DELIVERY_EN = (1 << 0),
	WMI_HOST_STA_PS_UAPSD_AC0_TRIGGER_EN  = (1 << 1),
	WMI_HOST_STA_PS_UAPSD_AC1_DELIVERY_EN = (1 << 2),
	WMI_HOST_STA_PS_UAPSD_AC1_TRIGGER_EN  = (1 << 3),
	WMI_HOST_STA_PS_UAPSD_AC2_DELIVERY_EN = (1 << 4),
	WMI_HOST_STA_PS_UAPSD_AC2_TRIGGER_EN  = (1 << 5),
	WMI_HOST_STA_PS_UAPSD_AC3_DELIVERY_EN = (1 << 6),
	WMI_HOST_STA_PS_UAPSD_AC3_TRIGGER_EN  = (1 << 7),
};

enum wmi_host_sta_ps_param_rx_wake_policy {
	/* Wake up when ever there is an  RX activity on the VDEV. In this mode
	* the Power save SM(state machine) will come out of sleep by either
	* sending null frame (or) a data frame (with PS==0) in response to TIM
	* bit set in the received beacon frame from AP.
	*/
	WMI_HOST_STA_PS_RX_WAKE_POLICY_WAKE = 0,

	/* Here the power save state machine will not wakeup in response to TIM
	* bit, instead it will send a PSPOLL (or) UASPD trigger based on UAPSD
	* configuration setup by WMISET_PS_SET_UAPSD  WMI command.  When all
	* access categories are delivery-enabled, the station will send a UAPSD
	* trigger frame, otherwise it will send a PS-Poll.
	*/
	WMI_HOST_STA_PS_RX_WAKE_POLICY_POLL_UAPSD = 1,
};
enum wmi_host_sta_ps_param_pspoll_count {
	WMI_HOST_STA_PS_PSPOLL_COUNT_NO_MAX = 0,
	/* Values greater than 0 indicate the maximum numer of PS-Poll frames FW
	* will send before waking up.
	*/
};
/** Number of tx frames/beacon  that cause the power save SM to wake up.
 *
 * Value 1 causes the SM to wake up for every TX. Value 0 has a special
 * meaning, It will cause the SM to never wake up. This is useful if you want
 * to keep the system to sleep all the time for some kind of test mode . host
 * can change this parameter any time.  It will affect at the next tx frame.
 */
enum wmi_host_sta_ps_param_tx_wake_threshold {
	WMI_HOST_STA_PS_TX_WAKE_THRESHOLD_NEVER = 0,
	WMI_HOST_STA_PS_TX_WAKE_THRESHOLD_ALWAYS = 1,

	/* Values greater than one indicate that many TX attempts per beacon
	* interval before the STA will wake up
	*/
};
/*
 * Transmit power scale factor.
 *
 */
typedef enum {
	WMI_HOST_TP_SCALE_MAX	= 0,	/* no scaling (default) */
	WMI_HOST_TP_SCALE_50	= 1,	/* 50% of max (-3 dBm) */
	WMI_HOST_TP_SCALE_25	= 2,	/* 25% of max (-6 dBm) */
	WMI_HOST_TP_SCALE_12	= 3,	/* 12% of max (-9 dBm) */
	WMI_HOST_TP_SCALE_MIN	= 4,	/* min, but still on   */
	WMI_HOST_TP_SCALE_SIZE   = 5,	/* max num of enum	*/
} WMI_HOST_TP_SCALE;
enum {
	WMI_HOST_RATEPWR_TABLE_OPS_SET,
	WMI_HOST_RATEPWR_TABLE_OPS_GET,
};
/* reserved up through 0xF */
/**
 *  struct wmi_host_dcs_mib_stats - WLAN IM stats from target to host
 *      Below statistics are sent from target to host periodically.
 *      These are collected at target as long as target is running
 *      and target chip is not in sleep.
 * @listen_time:
 * @reg_tx_frame_cnt:
 * @reg_rx_frame_cnt:
 * @reg_rxclr_cnt:
 * @reg_cycle_cnt: delta cycle count
 * @reg_rxclr_ext_cnt:
 * @reg_ofdm_phyerr_cnt:
 * @reg_cck_phyerr_cnt: CCK err count since last reset, read from register
 */
typedef struct _hp_dcs_mib_stats  {
	int32_t    listen_time;
	uint32_t   reg_tx_frame_cnt;
	uint32_t   reg_rx_frame_cnt;
	uint32_t   reg_rxclr_cnt;
	uint32_t   reg_cycle_cnt;
	uint32_t   reg_rxclr_ext_cnt;
	uint32_t   reg_ofdm_phyerr_cnt;
	uint32_t   reg_cck_phyerr_cnt;
} wmi_host_dcs_mib_stats_t;

/**
 * struct wmi_host_dcs_im_tgt_stats - DCS IM target stats
 * @reg_tsf32: current running TSF from the TSF-1
 * @last_ack_rssi: Known last frame rssi, in case of multiple stations, if
 *      and at different ranges, this would not gaurantee that
 *      this is the least rssi.
 * @tx_waste_time: Sum of all the failed durations in the last
 *      one second interval.
 * @rx_time: count how many times the hal_rxerr_phy is marked, in this
 *      time period
 * @phyerr_cnt:
 * @mib_stats: wmi_host_dcs_mib_stats_t - collected mib stats as explained
 *      in mib structure
 * @chan_nf: Channel noise floor (Units are in dBm)
 * @my_bss_rx_cycle_count: BSS rx cycle count
 */
typedef struct _wmi_host_dcs_im_tgt_stats {
	uint32_t                     reg_tsf32;
	uint32_t                     last_ack_rssi;
	uint32_t                     tx_waste_time;
	uint32_t                     rx_time;
	uint32_t                     phyerr_cnt;
	wmi_host_dcs_mib_stats_t     mib_stats;
	uint32_t		     chan_nf;
	uint32_t		     my_bss_rx_cycle_count;
} wmi_host_dcs_im_tgt_stats_t;

/**
 * Enum for pktlog req
 */
typedef enum {
	WMI_HOST_PKTLOG_EVENT_RX	= 0x1,
	WMI_HOST_PKTLOG_EVENT_TX	= 0x2,
	WMI_HOST_PKTLOG_EVENT_RCF	= 0x4, /* Rate Control Find */
	WMI_HOST_PKTLOG_EVENT_RCU	= 0x8, /* Rate Control Update */
	WMI_HOST_PKTLOG_EVENT_DBG_PRINT = 0x10, /* DEBUG prints */
	/* To support Smart Antenna */
	WMI_HOST_PKTLOG_EVENT_SMART_ANTENNA = 0x20,
	WMI_HOST_PKTLOG_EVENT_H_INFO = 0x40,
	WMI_HOST_PKTLOG_EVENT_STEERING = 0x80,
	/* To support Tx data Capture */
	WMI_HOST_PKTLOG_EVENT_TX_DATA_CAPTURE = 0x100,
} WMI_HOST_PKTLOG_EVENT;

/**
 * wmi_host_phyerr
 *
 */
#define WMI_HOST_PHY_ERROR_SPECTRAL_SCAN	0x26
#define WMI_HOST_PHY_ERROR_FALSE_RADAR_EXT	0x24

#define WMI_HOST_AR900B_DFS_PHYERR_MASK	0x4
#define WMI_HOST_AR900B_SPECTRAL_PHYERR_MASK	0x4000000

/**
 * struct wmi_host_perchain_rssi_info - per chain RSSI info
 * @rssi_pri20: RSSI on primary 20
 * @rssi_sec20: RSSI on secomdary 20
 * @rssi_sec40: RSSI secondary 40
 * @rssi_sec80: RSSI secondary 80
 */
typedef struct wmi_host_perchain_rssi_info {
	int8_t rssi_pri20;
	int8_t rssi_sec20;
	int8_t rssi_sec40;
	int8_t rssi_sec80;
} wmi_host_perchain_rssi_info_t;

/**
 * struct _wmi_host_rf_info - RF measurement information
 * @rssi_comb: RSSI Information
 * @pc_rssi_info[4]: For now, we know we are getting information
 *      for only 4 chains at max. For future extensions
 *      use a define
 * @noise_floor: Noise floor information
 */
typedef struct _wmi_host_rf_info {
	int8_t rssi_comb;
	wmi_host_perchain_rssi_info_t pc_rssi_info[4];
	int16_t noise_floor[4];
} wmi_host_rf_info_t;

/**
 * struct _wmi_host_chan_info
 * @center_freq1: center frequency 1 in MHz
 * @center_freq2: center frequency 2 in MHz -valid only for
 *                11ACVHT 80PLUS80 mode
 * @chan_width: channel width in MHz
 */
typedef struct _wmi_host_chan_info {
	u_int16_t center_freq1;
	u_int16_t center_freq2;
	u_int8_t chan_width;
} wmi_host_chan_info_t;

/**
 * struct wmi_host_phyerr
 * @rf_info:
 * @chan_info:
 * @tsf64:
 * @phy_err_code:
 * @tsf_timestamp:
 * @bufp:
 * @buf_len:
 * @phy_err_mask0:
 * @phy_err_mask1:
 * @pdev_id: pdev_id
 */
typedef struct _wmi_host_phyerr {
	wmi_host_rf_info_t rf_info;
	wmi_host_chan_info_t chan_info;
	uint64_t tsf64;
	int32_t phy_err_code;
	uint32_t tsf_timestamp;
	uint8_t *bufp;
	uint32_t buf_len;
	uint32_t phy_err_mask0;
	uint32_t phy_err_mask1;
	uint32_t pdev_id;
} wmi_host_phyerr_t;

/**
 * struct wmi_host_rtt_event_hdr
 * @req_id: request id
 * @status: status
 * @meas_done: measurement done flag
 * @meas_type: measurement type
 * @report_type: report type
 * @v3_status: v2 status
 * @v3_finish:
 * @v3_tm_start:
 * @num_ap: number of AP
 * @result: resuult
 * @dest_mac: destination mac
 */
typedef struct {
	uint16_t req_id;
	uint16_t status:1,
		meas_done:1,
		meas_type:3,
		report_type:3,
		v3_status:2,
		v3_finish:1,
		v3_tm_start:1,
		num_ap:4;
	uint16_t result;
	uint8_t  dest_mac[IEEE80211_ADDR_LEN];
} wmi_host_rtt_event_hdr;

/**
 * struct wmi_host_rtt_meas_event - RTT measurement event
 * @chain_mask:
 * @bw:
 * @rsvd:
 * @txrxchain_mask: Bit:0-3:chain mask
 *                  Bit 4-5: band width info
 *                  00 --Legacy 20, 01 --HT/VHT20
 *                  10 --HT/VHT40, 11 -- VHT80
 * @tod: resolution of 0.1ns
 * @toa: resolution of 0.1ns
 * @t3:
 * @t4:
 * @rssi0:
 * @rssi1:
 * @rssi2:
 * @rssi3:
 */
typedef struct {
	uint32_t chain_mask:3,
		bw:2,
		rsvd:27;
	uint32_t txrxchain_mask;
	uint64_t tod;
	uint64_t toa;
	uint64_t t3;
	uint64_t t4;
	uint32_t rssi0;
	uint32_t rssi1;
	uint32_t rssi2;
	uint32_t rssi3;
} wmi_host_rtt_meas_event;

/*----RTT Report event definition  ----*/
typedef enum {
	/* rtt cmd header parsing error  --terminate */
	WMI_HOST_RTT_COMMAND_HEADER_ERROR = 0,
	/* rtt body parsing error -- skip current STA REQ */
	WMI_HOST_RTT_COMMAND_ERROR,
	/* rtt no resource	-- terminate */
	WMI_HOST_RTT_MODULE_BUSY,
	/* STA exceed the support limit -- only server the first n STA */
	WMI_HOST_RTT_TOO_MANY_STA,
	/* any allocate failure */
	WMI_HOST_RTT_NO_RESOURCE,
	/* can not find vdev with vdev ID - skip current STA REQ */
	WMI_HOST_RTT_VDEV_ERROR,
	/* Tx failure   -- continiue and measure number */
	WMI_HOST_RTT_TRANSIMISSION_ERROR,
	/* wait for first TM timer expire-terminate current STA measurement */
	WMI_HOST_RTT_TM_TIMER_EXPIRE,
	/* we do not support RTT measurement with this type of frame */
	WMI_HOST_RTT_FRAME_TYPE_NOSUPPORT,
	/* whole RTT measurement timer expire-terminate
	** current STA measurement */
	WMI_HOST_RTT_TIMER_EXPIRE,
	/* channel swicth failed */
	WMI_HOST_RTT_CHAN_SWITCH_ERROR,
	/* TMR trans error, this dest peer will be skipped */
	WMI_HOST_RTT_TMR_TRANS_ERROR,
	/* V3 only. If both CFR and Token mismatch, do not report */
	WMI_HOST_RTT_NO_REPORT_BAD_CFR_TOKEN,
	/* For First TM, if CFR is bad, then do not report */
	WMI_HOST_RTT_NO_REPORT_FIRST_TM_BAD_CFR,
	/* do not allow report type2 mix with type 0, 1 */
	WMI_HOST_RTT_REPORT_TYPE2_MIX,
	/* LCI Configuration OK. - Responder only */
	WMI_HOST_RTT_LCI_CFG_OK,
	/* LCR configuration OK. - Responder only */
	WMI_HOST_RTT_LCR_CFG_OK,
	/* Bad configuration LCI (or) LCR request - Responder only */
	WMI_HOST_RTT_CFG_ERROR,
	WMI_HOST_WMI_RTT_REJECT_MAX,
} WMI_HOST_RTT_ERROR_INDICATOR;
typedef struct {
	wmi_host_rtt_event_hdr hdr;
	WMI_HOST_RTT_ERROR_INDICATOR reject_reason;
} wmi_host_rtt_error_report_event;

#if defined(AR9888)
typedef enum {
	WMI_HOST_PROF_CPU_IDLE,
	WMI_HOST_PROF_PPDU_PROC,
	WMI_HOST_PROF_PPDU_POST,
	WMI_HOST_PROF_HTT_TX_INPUT,
	WMI_HOST_PROF_MSDU_ENQ,
	WMI_HOST_PROF_PPDU_POST_HAL,
	WMI_HOST_PROF_COMPUTE_TX_TIME,

	/* Add new ID's above this. */
	WMI_HOST_PROF_MAX_ID,
} wmi_host_profile_id_t;
#endif

#define WMI_HOST_WLAN_PROFILE_MAX_HIST	3
#define WMI_HOST_WLAN_PROFILE_MAX_BIN_CNT 32

#if defined(AR9888)
#define WMI_HOST_MAX_PROFILE	WMI_HOST_PROF_MAX_ID
#else
#define WMI_HOST_MAX_PROFILE	WMI_HOST_WLAN_PROFILE_MAX_BIN_CNT
#endif

/**
 * struct wmi_host_wlan_profile - Host profile param
 * @id: profile id
 * @cnt: Count
 * @tot:
 * @min: minimum
 * @max: Mac
 * @hist_intvl: history interval
 * @hist: profile data history
 */
typedef struct {
	uint32_t id;
	uint32_t cnt;
	uint32_t tot;
	uint32_t min;
	uint32_t max;
	uint32_t hist_intvl;
	uint32_t hist[WMI_HOST_WLAN_PROFILE_MAX_HIST];
} wmi_host_wlan_profile_t;

/**
 * struct wmi_host_wlan_profile_ctx_t - profile context
 * @tot: time in us
 * @tx_msdu_cnt: MSDU TX count
 * @tx_mpdu_cnt: MPDU tx count
 * @tx_ppdu_cnt: PPDU tx count
 * @rx_msdu_cnt: MSDU RX count
 * @rx_mpdu_cnt: MPDU RXcount
 * @bin_count: Bin count
 */
typedef struct {
	uint32_t tot;
	uint32_t tx_msdu_cnt;
	uint32_t tx_mpdu_cnt;
	uint32_t tx_ppdu_cnt;
	uint32_t rx_msdu_cnt;
	uint32_t rx_mpdu_cnt;
	uint32_t bin_count;
} wmi_host_wlan_profile_ctx_t;

/**
 * struct wmi_host_chan_info_event - Channel info WMI event
 * @pdev_id: pdev_id
 * @err_code: Error code
 * @freq: Channel freq
 * @cmd_flags: Read flags
 * @noise_floor: Noise Floor value
 * @rx_clear_count: rx clear count
 * @cycle_count: cycle count
 * @chan_tx_pwr_range: channel tx power per range
 * @chan_tx_pwr_tp: channel tx power per throughput
 * @rx_frame_count: rx frame count
 * @rx_11b_mode_data_duration: 11b mode data duration
 * @my_bss_rx_cycle_count: BSS rx cycle count
 * @tx_frame_cnt: tx frame count
 * @mac_clk_mhz: mac clock
 * @vdev_id: unique id identifying the VDEV
 */
typedef struct {
	uint32_t pdev_id;
	uint32_t err_code;
	uint32_t freq;
	uint32_t cmd_flags;
	uint32_t noise_floor;
	uint32_t rx_clear_count;
	uint32_t cycle_count;
	uint32_t chan_tx_pwr_range;
	uint32_t chan_tx_pwr_tp;
	uint32_t rx_frame_count;
	uint32_t rx_11b_mode_data_duration;
	uint32_t my_bss_rx_cycle_count;
	uint32_t tx_frame_cnt;
	uint32_t mac_clk_mhz;
	uint32_t vdev_id;
} wmi_host_chan_info_event;

/**
 * struct wmi_host_pdev_channel_hopping_event
 * @pdev_id: pdev_id
 * @noise_floor_report_iter: Noise threshold iterations with high values
 * @noise_floor_total_iter: Total noise threshold iterations
 */
typedef struct {
	uint32_t pdev_id;
	uint32_t noise_floor_report_iter;
	uint32_t noise_floor_total_iter;
} wmi_host_pdev_channel_hopping_event;

/**
 * struct wmi_host_pdev_bss_chan_info_event
 * @pdev_id: pdev_id
 * @freq: Units in MHz
 * @noise_floor: units are dBm
 * @rx_clear_count_low:
 * @rx_clear_count_high:
 * @cycle_count_low:
 * @cycle_count_high:
 * @tx_cycle_count_low:
 * @tx_cycle_count_high:
 * @rx_cycle_count_low:
 * @rx_cycle_count_high:
 * @rx_bss_cycle_count_low:
 * @rx_bss_cycle_count_high:
 * @reserved:
 */
typedef struct {
	uint32_t pdev_id;
	uint32_t freq;
	uint32_t noise_floor;
	uint32_t rx_clear_count_low;
	uint32_t rx_clear_count_high;
	uint32_t cycle_count_low;
	uint32_t cycle_count_high;
	uint32_t tx_cycle_count_low;
	uint32_t tx_cycle_count_high;
	uint32_t rx_cycle_count_low;
	uint32_t rx_cycle_count_high;
	uint32_t rx_bss_cycle_count_low;
	uint32_t rx_bss_cycle_count_high;
	uint32_t reserved;
} wmi_host_pdev_bss_chan_info_event;

#define WMI_HOST_INST_STATS_INVALID_RSSI 0
/**
 * struct wmi_host_inst_stats_resp
 * @iRSSI: Instantaneous RSSI
 * @peer_macaddr: peer mac address
 * @pdev_id: pdev_id
 */
typedef struct {
	uint32_t iRSSI;
	wmi_host_mac_addr peer_macaddr;
	uint32_t pdev_id;
} wmi_host_inst_stats_resp;

/* Event definition and new structure addition to send event
 * to host to block/unblock tx data traffic based on peer_ast_idx or vdev id
 */
#define WMI_HOST_INVALID_PEER_AST_INDEX              0xffff
#define WMI_HOST_TX_DATA_TRAFFIC_CTRL_BLOCK          0x1
#define WMI_HOST_TX_DATA_TRAFFIC_CTRL_UNBLOCK        0x2
/**
 * struct wmi_host_tx_data_traffic_ctrl_event
 * @peer_ast_idx: For vdev based control, peer_ast_idx will be
 *                WMI_INVALID_PEER_AST_INDEX
 * @vdev_id: only applies if peer_ast_idx == INVALID
 * @ctrl_cmd: WMI_TX_DATA_TRAFFIC_CTRL_BLOCK or
 *                WMI_TX_DATA_TRAFFIC_CTRL_UNBLOCK
 */
typedef struct {
	uint32_t peer_ast_idx;
	uint32_t vdev_id;
	uint32_t ctrl_cmd;
} wmi_host_tx_data_traffic_ctrl_event;

enum {
	WMI_HOST_ATF_PEER_STATS_DISABLED = 0,
	WMI_HOST_ATF_PEER_STATS_ENABLED  = 1,
};

#define WMI_HOST_ATF_PEER_STATS_GET_PEER_AST_IDX(token_info) \
	(token_info.field1 & 0xffff)

#define WMI_HOST_ATF_PEER_STATS_GET_USED_TOKENS(token_info) \
	((token_info.field2 & 0xffff0000) >> 16)

#define WMI_HOST_ATF_PEER_STATS_GET_UNUSED_TOKENS(token_info) \
	(token_info.field2 & 0xffff)

#define WMI_HOST_ATF_PEER_STATS_SET_PEER_AST_IDX(token_info, peer_ast_idx) \
	do { \
		token_info.field1 &= 0xffff0000; \
		token_info.field1 |= ((peer_ast_idx) & 0xffff); \
	} while (0)

#define WMI_HOST_ATF_PEER_STATS_SET_USED_TOKENS(token_info, used_token) \
	do { \
		token_info.field2 &= 0x0000ffff; \
		token_info.field2 |= (((used_token) & 0xffff) << 16); \
	} while (0)

#define WMI_HOST_ATF_PEER_STATS_SET_UNUSED_TOKENS(token_info, unused_token) \
	do { \
		token_info.field2 &= 0xffff0000; \
		token_info.field2 |= ((unused_token) & 0xffff); \
	} while (0)

/**
 * struct wmi_host_atf_peer_stats_info
 * @field1: bits 15:0   peer_ast_index  WMI_ATF_PEER_STATS_GET_PEER_AST_IDX
 *          bits 31:16  reserved
 * @field2: bits 15:0   used tokens     WMI_ATF_PEER_STATS_GET_USED_TOKENS
 *          bits 31:16  unused tokens   WMI_ATF_PEER_STATS_GET_UNUSED_TOKENS
 * @field3: for future use
 */
typedef struct {
	uint32_t    field1;
	uint32_t    field2;
	uint32_t    field3;
} wmi_host_atf_peer_stats_info;

/**
 * struct wmi_host_atf_peer_stats_event
 * @pdev_id: pdev_id
 * @num_atf_peers: number of peers in token_info_list
 * @comp_usable_airtime: computed usable airtime in tokens
 * @reserved[4]: reserved for future use
 * @wmi_host_atf_peer_stats_info token_info_list: list of num_atf_peers
 */
typedef struct {
	uint32_t pdev_id;
	uint32_t num_atf_peers;
	uint32_t comp_usable_airtime;
	uint32_t reserved[4];
	wmi_host_atf_peer_stats_info token_info_list[1];
} wmi_host_atf_peer_stats_event;

/**
 * struct wmi_host_ath_dcs_cw_int
 * @channel: either number or freq in mhz
 */
typedef struct {
	uint32_t channel;
} wmi_host_ath_dcs_cw_int;

#define WMI_MAX_POWER_DBG_ARGS 8

/**
 * struct wmi_power_dbg_params - power debug command parameter
 * @pdev_id: subsystem identifier
 * @module_id: parameter id
 * @num_arg: no of arguments
 * @args: arguments
 */
struct wmi_power_dbg_params {
	uint32_t pdev_id;
	uint32_t module_id;
	uint32_t num_args;
	uint32_t args[WMI_MAX_POWER_DBG_ARGS];
};

/**
 * struct wmi_adaptive_dwelltime_params - the adaptive dwelltime params
 * @vdev_id: vdev id
 * @is_enabled: Adaptive dwell time is enabled/disabled
 * @dwelltime_mode: global default adaptive dwell mode
 * @lpf_weight: weight to calculate the average low pass
 * filter for channel congestion
 * @passive_mon_intval: intval to monitor wifi activity in passive scan in msec
 * @wifi_act_threshold: % of wifi activity used in passive scan 0-100
 *
 */
struct wmi_adaptive_dwelltime_params {
	uint32_t vdev_id;
	bool is_enabled;
	enum wmi_dwelltime_adaptive_mode dwelltime_mode;
	uint8_t lpf_weight;
	uint8_t passive_mon_intval;
	uint8_t wifi_act_threshold;
};

/**
 * struct wmi_per_roam_config - per based roaming parameters
 * @enable: if PER based roaming is enabled/disabled
 * @tx_high_rate_thresh: high rate threshold at which PER based
 *     roam will stop in tx path
 * @rx_high_rate_thresh: high rate threshold at which PER based
 *     roam will stop in rx path
 * @tx_low_rate_thresh: rate below which traffic will be considered
 *     for PER based roaming in Tx path
 * @rx_low_rate_thresh: rate below which traffic will be considered
 *     for PER based roaming in Tx path
 * @tx_rate_thresh_percnt: % above which when traffic is below low_rate_thresh
 *     will be considered for PER based scan in tx path
 * @rx_rate_thresh_percnt: % above which when traffic is below low_rate_thresh
 *     will be considered for PER based scan in rx path
 * @per_rest_time: time for which PER based roam will wait once it
 *     issues a roam scan.
 * @tx_per_mon_time: Minimum time required to be considered as valid scenario
 *     for PER based roam in tx path
 * @rx_per_mon_time: Minimum time required to be considered as valid scenario
 *     for PER based roam in rx path
 * @min_candidate_rssi: Minimum RSSI threshold for candidate AP to be used for
 *     PER based roaming
 */
struct wmi_per_roam_config {
	uint32_t enable;
	uint32_t tx_high_rate_thresh;
	uint32_t rx_high_rate_thresh;
	uint32_t tx_low_rate_thresh;
	uint32_t rx_low_rate_thresh;
	uint32_t tx_rate_thresh_percnt;
	uint32_t rx_rate_thresh_percnt;
	uint32_t per_rest_time;
	uint32_t tx_per_mon_time;
	uint32_t rx_per_mon_time;
	uint32_t min_candidate_rssi;
};

/**
 * struct wmi_per_roam_config_req: PER based roaming config request
 * @vdev_id: vdev id on which config needs to be set
 * @per_config: PER config
 */
struct wmi_per_roam_config_req {
	uint8_t vdev_id;
	struct wmi_per_roam_config per_config;
};

/**
 * struct wmi_fw_dump_seg_req - individual segment details
 * @seg_id - segment id.
 * @seg_start_addr_lo - lower address of the segment.
 * @seg_start_addr_hi - higher address of the segment.
 * @seg_length - length of the segment.
 * @dst_addr_lo - lower address of the destination buffer.
 * @dst_addr_hi - higher address of the destination buffer.
 *
 * This structure carries the information to firmware about the
 * individual segments. This structure is part of firmware memory
 * dump request.
 */
struct wmi_fw_dump_seg_req {
	uint8_t seg_id;
	uint32_t seg_start_addr_lo;
	uint32_t seg_start_addr_hi;
	uint32_t seg_length;
	uint32_t dst_addr_lo;
	uint32_t dst_addr_hi;
};

/**
 * enum wmi_userspace_log_level - Log level at userspace
 * @WMI_LOG_LEVEL_NO_COLLECTION: verbose_level 0 corresponds to no collection
 * @WMI_LOG_LEVEL_NORMAL_COLLECT: verbose_level 1 correspond to normal log
 * level with minimal user impact. This is the default value.
 * @WMI_LOG_LEVEL_ISSUE_REPRO: verbose_level 2 are enabled when user is lazily
 * trying to reproduce a problem, wifi performances and power can be impacted
 * but device should not otherwise be significantly impacted
 * @WMI_LOG_LEVEL_ACTIVE: verbose_level 3+ are used when trying to
 * actively debug a problem
 *
 * Various log levels defined in the userspace for logging applications
 */
enum wmi_userspace_log_level {
	WMI_LOG_LEVEL_NO_COLLECTION,
	WMI_LOG_LEVEL_NORMAL_COLLECT,
	WMI_LOG_LEVEL_ISSUE_REPRO,
	WMI_LOG_LEVEL_ACTIVE,
};

/**
 * HW mode config type replicated from FW header
 * @WMI_HOST_HW_MODE_SINGLE: Only one PHY is active.
 * @WMI_HOST_HW_MODE_DBS: Both PHYs are active in different bands,
 *                        one in 2G and another in 5G.
 * @WMI_HOST_HW_MODE_SBS_PASSIVE: Both PHYs are in passive mode (only rx) in
 *                        same band; no tx allowed.
 * @WMI_HOST_HW_MODE_SBS: Both PHYs are active in the same band.
 *                        Support for both PHYs within one band is planned
 *                        for 5G only(as indicated in WMI_MAC_PHY_CAPABILITIES),
 *                        but could be extended to other bands in the future.
 *                        The separation of the band between the two PHYs needs
 *                        to be communicated separately.
 * @WMI_HOST_HW_MODE_DBS_SBS: 3 PHYs, with 2 on the same band doing SBS
 *                           as in WMI_HW_MODE_SBS, and 3rd on the other band
 * @WMI_HOST_HW_MODE_DBS_OR_SBS: Two PHY with one PHY capabale of both 2G and
 *                        5G. It can support SBS (5G + 5G) OR DBS (5G + 2G).
 * @WMI_HOST_HW_MODE_MAX: Max hw_mode_id. Used to indicate invalid mode.
 */
enum wmi_host_hw_mode_config_type {
	WMI_HOST_HW_MODE_SINGLE       = 0,
	WMI_HOST_HW_MODE_DBS          = 1,
	WMI_HOST_HW_MODE_SBS_PASSIVE  = 2,
	WMI_HOST_HW_MODE_SBS          = 3,
	WMI_HOST_HW_MODE_DBS_SBS      = 4,
	WMI_HOST_HW_MODE_DBS_OR_SBS   = 5,
	WMI_HOST_HW_MODE_MAX,
};

/*
 * struct wmi_host_peer_txmu_cnt_event
 * @tx_mu_transmitted - MU-MIMO tx count
 */
typedef struct {
	uint32_t tx_mu_transmitted;
} wmi_host_peer_txmu_cnt_event;

#define MAX_SAR_LIMIT_ROWS_SUPPORTED 64
/**
 * struct sar_limit_cmd_row - sar limits row
 * @band_id: Optional param for frequency band
 *           See %enum wmi_sar_band_id_flags for possible values
 * @chain_id: Optional param for antenna chain id
 * @mod_id: Optional param for modulation scheme
 *          See %enum wmi_sar_mod_id_flags for possible values
 * @limit_value: Mandatory param providing power limits in steps of 0.5 dbm
 * @validity_bitmap: bitmap of valid optional params in sar_limit_cmd_row struct
 *                   See WMI_SAR_*_VALID_MASK for possible values
 */
struct sar_limit_cmd_row {
	uint32_t band_id;
	uint32_t chain_id;
	uint32_t mod_id;
	uint32_t limit_value;
	uint32_t validity_bitmap;
};

/**
 * struct sar_limit_cmd_params - sar limits params
 * @sar_enable: flag to enable SAR
 *              See %enum wmi_sar_feature_state_flags for possible values
 * @num_limit_rows: number of items in sar_limits
 * @commit_limits: indicates firmware to start apply new SAR values
 * @sar_limit_row_list: pointer to array of sar limit rows
 */
struct sar_limit_cmd_params {
	uint32_t sar_enable;
	uint32_t num_limit_rows;
	uint32_t commit_limits;
	struct sar_limit_cmd_row *sar_limit_row_list;
};

/**
 * struct sar_limit_event_row - sar limits row
 * @band_id: Frequency band.
 *           See %enum wmi_sar_band_id_flags for possible values
 * @chain_id: Chain id
 * @mod_id: Modulation scheme
 *          See %enum wmi_sar_mod_id_flags for possible values
 * @limit_value: Power limits in steps of 0.5 dbm that is currently active for
 *     the given @band_id, @chain_id, and @mod_id
 */
struct sar_limit_event_row {
	uint32_t band_id;
	uint32_t chain_id;
	uint32_t mod_id;
	uint32_t limit_value;
};

/**
 * struct sar_limit_event - sar limits params
 * @sar_enable: Current status of SAR enablement.
 *              See %enum wmi_sar_feature_state_flags for possible values
 * @num_limit_rows: number of items in sar_limits
 * @sar_limit_row: array of sar limit rows. Only @num_limit_rows
 *                 should be considered valid.
 */
struct sar_limit_event {
	uint32_t sar_enable;
	uint32_t num_limit_rows;
	struct sar_limit_event_row
			sar_limit_row[MAX_SAR_LIMIT_ROWS_SUPPORTED];
};

/*
 * struct wmi_peer_gid_userpos_list_event
 * @usr_list - User list
 */
#define GID_OVERLOAD_GROUP_COUNT  15
typedef struct {
	uint32_t usr_list[GID_OVERLOAD_GROUP_COUNT];
} wmi_host_peer_gid_userpos_list_event;

/**
 * enum rcpi_measurement_type - for identifying type of rcpi measurement
 * @RCPI_MEASUREMENT_TYPE_AVG_MGMT: avg rcpi of mgmt frames
 * @RCPI_MEASUREMENT_TYPE_AVG_DATA: avg rcpi of data frames
 * @RCPI_MEASUREMENT_TYPE_LAST_MGMT: rcpi of last mgmt frame
 * @RCPI_MEASUREMENT_TYPE_LAST_DATA: rcpi of last data frame
 * @RCPI_MEASUREMENT_TYPE_INVALID: invalid rcpi measurement type
 */
enum rcpi_measurement_type {
	RCPI_MEASUREMENT_TYPE_AVG_MGMT  = 0x1,
	RCPI_MEASUREMENT_TYPE_AVG_DATA  = 0x2,
	RCPI_MEASUREMENT_TYPE_LAST_MGMT = 0x3,
	RCPI_MEASUREMENT_TYPE_LAST_DATA = 0x4,
	RCPI_MEASUREMENT_TYPE_INVALID = 0x5,
};

/**
 * struct rcpi_req - RCPI req parameter
 * @vdev_id: virtual device id
 * @measurement_type: type of rcpi from enum wmi_rcpi_measurement_type
 * @mac_addr: peer mac addr for which measurement is required
 */
struct rcpi_req {
	uint32_t vdev_id;
	enum rcpi_measurement_type measurement_type;
	uint8_t mac_addr[IEEE80211_ADDR_LEN];
};

/**
 * struct rcpi_res - RCPI response parameter
 * @vdev_id: virtual device id
 * @measurement_type: type of rcpi from enum wmi_rcpi_measurement_type
 * @mac_addr: peer mac addr for which measurement is required
 * @rcpi_value: value of RCPI computed by firmware
 */
struct rcpi_res {
	uint32_t vdev_id;
	enum rcpi_measurement_type measurement_type;
	uint8_t mac_addr[IEEE80211_ADDR_LEN];
	int32_t rcpi_value;
};

#define WMI_HOST_BOARD_MCN_STRING_MAX_SIZE 19
#define WMI_HOST_BOARD_MCN_STRING_BUF_SIZE \
	(WMI_HOST_BOARD_MCN_STRING_MAX_SIZE+1) /* null-terminator */

typedef struct {
	uint32_t software_cal_version;
	uint32_t board_cal_version;
	/* board_mcn_detail:
	 * Provide a calibration message string for the host to display.
	 * Note: on a big-endian host, the 4 bytes within each A_UINT32 portion
	 * of a WMI message will be automatically byteswapped by the copy engine
	 * as the messages are transferred between host and target, to convert
	 * between the target's little-endianness and the host's big-endianness.
	 * Consequently, a big-endian host should manually unswap the bytes
	 * within the board_mcn_detail string buffer to get the bytes back into
	 * the desired natural order.
	 */
	uint8_t board_mcn_detail[WMI_HOST_BOARD_MCN_STRING_BUF_SIZE];
	uint32_t cal_ok; /* filled with CALIBRATION_STATUS enum value */
} wmi_host_pdev_check_cal_version_event;

/**
 * enum WMI_HOST_CALIBRATION_STATUS - Host defined Enums for cal status
 * @WMI_HOST_NO_FEATURE: The board was calibrated with a meta
 *                       which did not have this feature
 * @WMI_HOST_CALIBRATION_OK: The calibration status is OK
 * @WMI_HOST_CALIBRATION_NOT_OK: The calibration status is NOT OK
 */
enum WMI_HOST_CALIBRATION_STATUS {
	WMI_HOST_NO_FEATURE = 0,
	WMI_HOST_CALIBRATION_OK,
	WMI_HOST_CALIBRATION_NOT_OK,
};

/**
 * struct wmi_host_pdev_utf_event - Host defined struct to hold utf event data
 * @data:        Pointer to data
 * @datalen:     Data length
 * @pdev_id:     Pdev_id of data
 *
 */
struct wmi_host_pdev_utf_event {
	uint8_t *data;
	uint16_t datalen;
	uint32_t pdev_id;
};

/**
 * struct wmi_host_utf_seg_header_info - Host defined struct to map seg info in
 *                         UTF event
 * @len:        segment length
 * @msgref:     message reference
 * @segment_info: segment info
 * @pdev_id:  pdev_id
 *
 */
struct wmi_host_utf_seg_header_info {
	uint32_t len;
	uint32_t msgref;
	uint32_t segment_info;
	uint32_t pdev_id;
};

/**
 * struct wmi_host_pdev_qvit_event - Host defined struct to hold qvit event data
 * @data:        Pointer to data
 * @datalen:     Data length
 *
 */
struct wmi_host_pdev_qvit_event {
	uint8_t *data;
	uint16_t datalen;
	uint32_t pdev_id;
};

/**
 * struct wmi_host_peer_delete_response_event - Peer Delete response event param
 * @vdev_id: vdev id
 * @mac_address: Peer Mac Address
 *
 */
struct wmi_host_peer_delete_response_event {
	uint32_t vdev_id;
	struct qdf_mac_addr mac_address;
};

/**
 * @struct wmi_host_dcs_interference_param
 * @interference_type: Type of DCS Interference
 * @uint32_t pdev_id: pdev id
 */
struct wmi_host_dcs_interference_param {
	uint32_t interference_type;
	uint32_t pdev_id;
};

/*
 * struct wmi_host_fips_event_param: FIPS event param
 * @pdev_id: pdev id
 * @error_status: Error status: 0 (no err), 1, or OPER_TIMEOUR
 * @data_len: FIPS data lenght
 * @data: pointer to data
 */
struct wmi_host_fips_event_param {
	uint32_t pdev_id;
	uint32_t error_status;
	uint32_t data_len;
	uint32_t *data;
};

/**
 * struct wmi_host_proxy_ast_reserve_param
 * @pdev_id: pdev id
 * @result: result
 */
struct wmi_host_proxy_ast_reserve_param {
	uint32_t pdev_id;
	uint32_t result;
};

/**
 * struct wmi_host_pdev_band_to_mac - freq range for mac
 * @pdev_id: PDEV ID to identifiy mac
 * @start_freq: start frequency value
 * @end_freq: end frequency value
 */
struct wmi_host_pdev_band_to_mac {
	uint32_t pdev_id;
	uint32_t start_freq;
	uint32_t end_freq;
};
#define WMI_HOST_MAX_PDEV 3

/**
 * struct wmi_init_cmd_param - INIT command params
 * @target_resource_config: pointer to resource config
 * @num_mem_chunks: number of memory chunks
 * @struct wmi_host_mem_chunk: pointer to memory chunks
 * @hw_mode_index: HW mode index chosen
 * @num_band_to_mac: Number of band to mac setting
 * @struct wmi_host_pdev_band_to_mac: band to mac setting
 */
struct wmi_init_cmd_param {
	target_resource_config *res_cfg;
	uint8_t num_mem_chunks;
	struct wmi_host_mem_chunk *mem_chunks;
	uint32_t hw_mode_id;
	uint32_t num_band_to_mac;
	struct wmi_host_pdev_band_to_mac band_to_mac[WMI_HOST_MAX_PDEV];
};

/**
 * struct pdev_csa_switch_count_status - CSA switch count status event param
 * @pdev_id: Physical device identifier
 * @current_switch_count: Current CSA switch count
 * @num_vdevs: Number of vdevs that need restart
 * @vdev_ids: Array containing the vdev ids that need restart
 */
struct pdev_csa_switch_count_status {
	uint32_t pdev_id;
	uint32_t current_switch_count;
	uint32_t num_vdevs;
	uint32_t *vdev_ids;
};

/**
 * enum wmi_host_active-bpf_mode - FW_ACTIVE_BPF_MODE, replicated from FW header
 * @WMI_HOST_ACTIVE_BPF_DISABLED: BPF is disabled for all packets in active mode
 * @WMI_HOST_ACTIVE_BPF_ENABLED: BPF is enabled for all packets in active mode
 * @WMI_HOST_ACTIVE_BPF_ADAPTIVE: BPF is enabled for packets up to some
 *	threshold in active mode
 */
enum wmi_host_active_bpf_mode {
	WMI_HOST_ACTIVE_BPF_DISABLED =	(1 << 1),
	WMI_HOST_ACTIVE_BPF_ENABLED =	(1 << 2),
	WMI_HOST_ACTIVE_BPF_ADAPTIVE =	(1 << 3)
};

/**
 * struct coex_config_params - Coex config command params
 * @vdev_id: Virtual AP device identifier
 * @config_type: Configuration type - wmi_coex_config_type enum
 * @config_arg1: Configuration argument based on config type
 * @config_arg2: Configuration argument based on config type
 * @config_arg3: Configuration argument based on config type
 * @config_arg4: Configuration argument based on config type
 * @config_arg5: Configuration argument based on config type
 * @config_arg6: Configuration argument based on config type
 */
struct coex_config_params {
	uint32_t vdev_id;
	uint32_t config_type;
	uint32_t config_arg1;
	uint32_t config_arg2;
	uint32_t config_arg3;
	uint32_t config_arg4;
	uint32_t config_arg5;
	uint32_t config_arg6;
};

#define WMI_HOST_PDEV_ID_SOC 0xFF
#define WMI_HOST_PDEV_ID_0   0
#define WMI_HOST_PDEV_ID_1   1
#define WMI_HOST_PDEV_ID_2   2

/**
 * struct tbttoffset_params - Tbttoffset event params
 * @vdev_id: Virtual AP device identifier
 * @tbttoffset : Tbttoffset for the virtual AP device
 */
struct tbttoffset_params {
	uint32_t vdev_id;
	uint32_t tbttoffset;
};

#define WMI_SCAN_CLIENT_MAX        7

/**
 * struct wmi_dbs_scan_sel_params - DBS scan selection params
 * @num_clients: Number of scan clients dutycycle
 * @pdev_id: pdev_id for identifying the MAC
 * @module_id: scan client module id
 * @num_dbs_scans: number of DBS scans
 * @num_non_dbs_scans: number of non-DBS scans
 */
struct wmi_dbs_scan_sel_params {
	uint32_t num_clients;
	uint32_t pdev_id;
	uint32_t module_id[WMI_SCAN_CLIENT_MAX];
	uint32_t num_dbs_scans[WMI_SCAN_CLIENT_MAX];
	uint32_t num_non_dbs_scans[WMI_SCAN_CLIENT_MAX];
};

/**
 * struct wmi_limit_off_chan_param - limit off channel parameters
 * @vdev_id: vdev id
 * @status: status of the command (enable/disable)
 * @max_offchan_time: max off channel time
 * @rest_time: home channel time
 * @skip_dfs_chans: skip dfs channels during scan
 */
struct wmi_limit_off_chan_param {
	uint32_t vdev_id;
	bool status;
	uint32_t max_offchan_time;
	uint32_t rest_time;
	bool skip_dfs_chans;
};

/**
 * struct wmi_mawc_roam_params - Motion Aided wireless connectivity params
 * @vdev_id: VDEV on which the parameters should be applied
 * @enable: MAWC roaming feature enable/disable
 * @traffic_load_threshold: Traffic threshold in kBps for MAWC roaming
 * @best_ap_rssi_threshold: AP RSSI Threshold for MAWC roaming
 * @rssi_stationary_high_adjust: High RSSI adjustment value to suppress scan
 * @rssi_stationary_low_adjust: Low RSSI adjustment value to suppress scan
 */
struct wmi_mawc_roam_params {
	uint8_t vdev_id;
	bool enable;
	uint32_t traffic_load_threshold;
	uint32_t best_ap_rssi_threshold;
	uint8_t rssi_stationary_high_adjust;
	uint8_t rssi_stationary_low_adjust;
};
/**
 * struct wmi_btm_config - BSS Transition Management offload params
 * @vdev_id: VDEV on which the parameters should be applied
 * @btm_offload_config: BTM config
 * @btm_solicited_timeout: Timeout value for waiting BTM request
 * @btm_max_attempt_cnt: Maximum attempt for sending BTM query to ESS
 * @btm_sticky_time: Stick time after roaming to new AP by BTM
 */
struct wmi_btm_config {
	uint8_t vdev_id;
	uint32_t btm_offload_config;
	uint32_t btm_solicited_timeout;
	uint32_t btm_max_attempt_cnt;
	uint32_t btm_sticky_time;
};

/**
 * struct set_arp_stats - set/reset arp stats
 * @vdev_id: session id
 * @flag: enable/disable stats
 * @pkt_type: type of packet(1 - arp)
 * @ip_addr: subnet ipv4 address in case of encrypted packets
 * @pkt_type_bitmap: pkt bitmap
 * @tcp_src_port: tcp src port for pkt tracking
 * @tcp_dst_port: tcp dst port for pkt tracking
 * @icmp_ipv4: target ipv4 address to track ping packets
 * @reserved: reserved
 */
struct set_arp_stats {
	uint32_t vdev_id;
	uint8_t flag;
	uint8_t pkt_type;
	uint32_t ip_addr;
	uint32_t pkt_type_bitmap;
	uint32_t tcp_src_port;
	uint32_t tcp_dst_port;
	uint32_t icmp_ipv4;
	uint32_t reserved;
};

/**
 * struct get_arp_stats - get arp stats from firmware
 * @pkt_type: packet type(1 - ARP)
 * @vdev_id: session id
 */
struct get_arp_stats {
	uint8_t pkt_type;
	uint32_t vdev_id;
};

/**
 * struct wmi_host_ready_ev_param - Data revieved in ready event
 * @status:         FW init status. Success or Failure.
 * @num_dscp_table: Number of DSCP table supported in FW
 * @num_extra_mac_addr: Extra mac address present in ready event. Used
 *                  in DBDC mode to provide multiple mac per pdev.
 * @num_total_peer: Total number of peers FW could allocate. Zero means
 *                  FW could  allocate num peers requested by host in init.
 *                  Otherwise, host need update it max_peer to this value.
 * @num_extra_peer: Number of extra peers created and used within FW. Host
 *                  should expect peer_id can be num_total_peer + num_extra_peer
 *                  but it can create only upto num_total_peer.
 * @agile_capability: Boolean specification of whether the target supports
 *                  agile DFS, by means of using one 80 MHz radio chain for
 *                  radar detection, concurrently with using another radio
 *                  chain for non-160 MHz regular operation.
 */
struct wmi_host_ready_ev_param {
	uint32_t status;
	uint32_t num_dscp_table;
	uint32_t num_extra_mac_addr;
	uint32_t num_total_peer;
	uint32_t num_extra_peer;
	bool agile_capability;
};

/**
 * struct bcn_offload_control - Beacon offload control params
 * @vdev_id: vdev identifer of VAP to control beacon tx
 * @bcn_tx_enable: Enable or Disable beacon TX in offload mode
 */
struct bcn_offload_control {
	uint32_t vdev_id;
	bool bcn_tx_enable;
};

/**
 * struct wds_entry - WDS entry structure
 * @peer_mac: peer mac
 * @wds_mac: wds mac address
 * @flags: flags
 */
struct wdsentry {
	u_int8_t peer_mac[IEEE80211_ADDR_LEN];
	u_int8_t wds_mac[IEEE80211_ADDR_LEN];
	A_UINT32 flags;
};

#define WMI_HOST_DBR_RING_ADDR_LO_S 0
#define WMI_HOST_DBR_RING_ADDR_LO 0xffffffff

#define WMI_HOST_DBR_RING_ADDR_LO_GET(dword) \
			WMI_HOST_F_MS(dword, WMI_HOST_DBR_RING_ADDR_LO)
#define WMI_HOST_DBR_RING_ADDR_LO_SET(dword, val) \
			WMI_HOST_F_RMW(dword, val, WMI_HOST_DBR_RING_ADDR_LO)

#define WMI_HOST_DBR_RING_ADDR_HI_S 0
#define WMI_HOST_DBR_RING_ADDR_HI 0xf

#define WMI_HOST_DBR_RING_ADDR_HI_GET(dword) \
			WMI_HOST_F_MS(dword, WMI_HOST_DBR_RING_ADDR_HI)
#define WMI_HOST_DBR_RING_ADDR_HI_SET(dword, val) \
			WMI_HOST_F_RMW(dword, val, WMI_HOST_DBR_RING_ADDR_HI)

#define WMI_HOST_DBR_DATA_ADDR_LO_S 0
#define WMI_HOST_DBR_DATA_ADDR_LO 0xffffffff

#define WMI_HOST_DBR_DATA_ADDR_LO_GET(dword) \
			WMI_HOST_F_MS(dword, WMI_HOST_DBR_DATA_ADDR_LO)
#define WMI_HOST_DBR_DATA_ADDR_LO_SET(dword, val) \
			WMI_HOST_F_RMW(dword, val, WMI_HOST_DBR_DATA_ADDR_LO)

#define WMI_HOST_DBR_DATA_ADDR_HI_S 0
#define WMI_HOST_DBR_DATA_ADDR_HI 0xf

#define WMI_HOST_DBR_DATA_ADDR_HI_GET(dword) \
			WMI_HOST_F_MS(dword, WMI_HOST_DBR_DATA_ADDR_HI)
#define WMI_HOST_DBR_DATA_ADDR_HI_SET(dword, val) \
			WMI_HOST_F_RMW(dword, val, WMI_HOST_DBR_DATA_ADDR_HI)

#define WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA_S 12
#define WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA 0xfffff

#define WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA_GET(dword) \
		WMI_HOST_F_MS(dword, WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA)
#define WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA_SET(dword, val) \
		WMI_HOST_F_RMW(dword, val, WMI_HOST_DBR_DATA_ADDR_HI_HOST_DATA)

/**
 * struct direct_buf_rx_entry: direct buffer rx release entry structure
 *
 * @addr_lo: LSB 32-bits of the buffer
 * @addr_hi: MSB 32-bits of the buffer
 * @len: Length of the buffer
 */
struct direct_buf_rx_entry {
	uint32_t paddr_lo;
	uint32_t paddr_hi;
	uint32_t len;
};

/**
 * struct direct_buf_rx_rsp: direct buffer rx response structure
 *
 * @pdev_id: Index of the pdev for which response is received
 * @mod_mod: Index of the module for which respone is received
 * @num_buf_release_entry: Number of buffers released through event
 * @dbr_entries: Pointer to direct buffer rx entry struct
 */
struct direct_buf_rx_rsp {
	uint32_t pdev_id;
	uint32_t mod_id;
	uint32_t num_buf_release_entry;
	struct direct_buf_rx_entry *dbr_entries;
};

/**
 * struct direct_buf_rx_cfg_req: direct buffer rx config request structure
 *
 * @pdev_id: Index of the pdev for which response is received
 * @mod_id: Index of the module for which respone is received
 * @base_paddr_lo: Lower 32bits of ring base address
 * @base_paddr_hi: Higher 32bits of ring base address
 * @head_idx_paddr_lo: Lower 32bits of head idx register address
 * @head_idx_paddr_hi: Higher 32bits of head idx register address
 * @tail_idx_paddr_lo: Lower 32bits of tail idx register address
 * @tail_idx_paddr_hi: Higher 32bits of tail idx register address
 * @buf_size: Size of the buffer for each pointer in the ring
 * @num_elems: Number of pointers allocated and part of the source ring
 */
struct direct_buf_rx_cfg_req {
	uint32_t pdev_id;
	uint32_t mod_id;
	uint32_t base_paddr_lo;
	uint32_t base_paddr_hi;
	uint32_t head_idx_paddr_lo;
	uint32_t head_idx_paddr_hi;
	uint32_t tail_idx_paddr_hi;
	uint32_t tail_idx_paddr_lo;
	uint32_t buf_size;
	uint32_t num_elems;
	uint32_t event_timeout_ms;
	uint32_t num_resp_per_event;
};

/**
 * struct wmi_obss_detection_cfg_param - obss detection cfg
 * @vdev_id: vdev id
 * @obss_detect_period_ms: detection period in ms
 * @obss_11b_ap_detect_mode: detect whether there is 11b ap/ibss
 * @obss_11b_sta_detect_mode: detect whether there is 11b sta
 *                            connected with other APs
 * @obss_11g_ap_detect_mode: detect whether there is 11g AP
 * @obss_11a_detect_mode: detect whether there is legacy 11a traffic
 * @obss_ht_legacy_detect_mode: detect whether there is ap which is
 *                              ht legacy mode
 * @obss_ht_mixed_detect_mode: detect whether there is ap which is ht mixed mode
 * @obss_ht_20mhz_detect_mode: detect whether there is ap which has 20M only
 *                             station
 */
struct wmi_obss_detection_cfg_param {
	uint32_t vdev_id;
	uint32_t obss_detect_period_ms;
	uint32_t obss_11b_ap_detect_mode;
	uint32_t obss_11b_sta_detect_mode;
	uint32_t obss_11g_ap_detect_mode;
	uint32_t obss_11a_detect_mode;
	uint32_t obss_ht_legacy_detect_mode;
	uint32_t obss_ht_mixed_detect_mode;
	uint32_t obss_ht_20mhz_detect_mode;
};

/**
 * enum wmi_obss_detection_reason - obss detection event reasons
 * @OBSS_OFFLOAD_DETECTION_DISABLED: OBSS detection disabled
 * @OBSS_OFFLOAD_DETECTION_PRESENT: OBSS present detection
 * @OBSS_OFFLOAD_DETECTION_ABSENT: OBSS absent detection
 *
 * Defines different types of reasons for obss detection event from firmware.
 */
enum wmi_obss_detection_reason {
	OBSS_OFFLOAD_DETECTION_DISABLED = 0,
	OBSS_OFFLOAD_DETECTION_PRESENT  = 1,
	OBSS_OFFLOAD_DETECTION_ABSENT   = 2,
};

/**
 * struct wmi_obss_detect_info - OBSS detection info from firmware
 * @vdev_id: ID of the vdev to which this info belongs.
 * @reason: Indicate if present or Absent detection,
 *          also if not supported offload for this vdev.
 * @matched_detection_masks: Detection bit map.
 * @matched_bssid_addr: MAC address valid for only if info is present detection.
 */
struct wmi_obss_detect_info {
	uint32_t vdev_id;
	enum wmi_obss_detection_reason reason;
	uint32_t matched_detection_masks;
	uint8_t matched_bssid_addr[IEEE80211_ADDR_LEN];
};

/**
 * @time_offset: time offset after 11k offload command to trigger a neighbor
 *	report request (in seconds)
 * @low_rssi_offset: Offset from rssi threshold to trigger a neighbor
 *	report request (in dBm)
 * @bmiss_count_trigger: Number of beacon miss events to trigger neighbor
 *	report request
 * @per_threshold_offset: offset from PER threshold to trigger neighbor
 *	report request (in %)
 * @neighbor_report_cache_timeout: timeout after which new trigger can enable
 *	sending of a neighbor report request (in seconds)
 * @max_neighbor_report_req_cap: max number of neighbor report requests that
 *	can be sent to the peer in the current session
 * @ssid: Current connect SSID info
 */
struct wmi_11k_offload_neighbor_report_params {
	uint32_t time_offset;
	uint32_t low_rssi_offset;
	uint32_t bmiss_count_trigger;
	uint32_t per_threshold_offset;
	uint32_t neighbor_report_cache_timeout;
	uint32_t max_neighbor_report_req_cap;
	struct mac_ssid ssid;
};

/**
 * struct wmi_11k_offload_params - offload 11k features to FW
 * @vdev_id: vdev id
 * @offload_11k_bitmask: bitmask to specify offloaded features
 *	B0: Neighbor Report Request offload
 *	B1-B31: Reserved
 * @neighbor_report_params: neighbor report offload params
 */
struct wmi_11k_offload_params {
	uint32_t vdev_id;
	uint32_t offload_11k_bitmask;
	struct wmi_11k_offload_neighbor_report_params neighbor_report_params;
};

/**
 * struct wmi_invoke_neighbor_report_params - Invoke neighbor report request
 *	from IW to FW
 * @vdev_id: vdev id
 * @send_resp_to_host: bool to send response to host or not
 * @ssid: ssid given from the IW command
 */
struct wmi_invoke_neighbor_report_params {
	uint32_t vdev_id;
	uint32_t send_resp_to_host;
	struct mac_ssid ssid;
};

/**
 * enum wmi_obss_color_collision_evt_type - bss color collision event type
 * @OBSS_COLOR_COLLISION_DETECTION_DISABLE: OBSS color detection disabled
 * @OBSS_COLOR_COLLISION_DETECTION: OBSS color collision detection
 * @OBSS_COLOR_FREE_SLOT_TIMER_EXPIRY: OBSS free slot detection with
 *                                     within expiry period
 * @OBSS_COLOR_FREE_SLOT_AVAILABLE: OBSS free slot detection
 *
 * Defines different types of type for obss color collision event type.
 */
enum wmi_obss_color_collision_evt_type {
	OBSS_COLOR_COLLISION_DETECTION_DISABLE = 0,
	OBSS_COLOR_COLLISION_DETECTION = 1,
	OBSS_COLOR_FREE_SLOT_TIMER_EXPIRY = 2,
	OBSS_COLOR_FREE_SLOT_AVAILABLE = 3,
};

/**
 * struct wmi_obss_color_collision_cfg_param - obss color collision cfg
 * @vdev_id: vdev id
 * @flags: proposed for future use cases, currently not used.
 * @evt_type: bss color collision event.
 * @current_bss_color: current bss color.
 * @detection_period_ms: scan interval for both AP and STA mode.
 * @scan_period_ms: scan period for passive scan to detect collision.
 * @free_slot_expiry_time_ms: FW to notify host at timer expiry after
 *                            which Host will disable the bss color.
 */
struct wmi_obss_color_collision_cfg_param {
	uint32_t vdev_id;
	uint32_t flags;
	enum wmi_obss_color_collision_evt_type evt_type;
	uint32_t current_bss_color;
	uint32_t detection_period_ms;
	uint32_t scan_period_ms;
	uint32_t free_slot_expiry_time_ms;
};

/**
 * struct wmi_obss_color_collision_info - bss color detection info from firmware
 * @vdev_id: ID of the vdev to which this info belongs.
 * @evt_type: bss color collision event.
 * @obss_color_bitmap_bit0to31: Bit set indicating BSS color present.
 * @obss_color_bitmap_bit32to63: Bit set indicating BSS color present.
 */
struct wmi_obss_color_collision_info {
	uint32_t vdev_id;
	enum wmi_obss_color_collision_evt_type evt_type;
	uint32_t obss_color_bitmap_bit0to31;
	uint32_t obss_color_bitmap_bit32to63;
};
#endif /* _WMI_UNIFIED_PARAM_H_ */
