/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_hdd_wext.c
 *
 * Linux Wireless Extensions Implementation
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wireless.h>
#include <mac_trace.h>
#include <wlan_hdd_includes.h>
#include <cds_api.h>
#include <net/arp.h>
#include <cdp_txrx_stats.h>
#include "sir_params.h"
#include "csr_api.h"
#include "csr_inside_api.h"
#include "sme_rrm_internal.h"
#include <ani_global.h>
#include "dot11f.h"
#include <wlan_hdd_wowl.h>
#include <wlan_hdd_cfg.h>
#include <wlan_hdd_wmm.h>
#include "utils_api.h"
#include "wlan_hdd_p2p.h"
#ifdef FEATURE_WLAN_TDLS
#include "wlan_hdd_tdls.h"
#endif

#include "cds_ieee80211_common.h"
#include "ol_if_athvar.h"
#include "dbglog_host.h"
#include "wma.h"

#include "wlan_hdd_power.h"
#include "qwlan_version.h"
#include "wlan_hdd_host_offload.h"

#include <linux/wireless.h>
#include <net/cfg80211.h>

#include "wlan_hdd_misc.h"

#include "qc_sap_ioctl.h"
#include "sme_api.h"
#include "wma_types.h"
#include "qdf_trace.h"
#include "wlan_hdd_assoc.h"
#include "wlan_hdd_ioctl.h"
#include "wlan_hdd_scan.h"
#include "sme_power_save_api.h"
#include "cds_concurrency.h"
#include "wlan_hdd_conc_ut.h"
#include "wlan_hdd_tsf.h"
#include "wlan_hdd_ocb.h"
#include "wlan_hdd_napi.h"
#include "cdp_txrx_flow_ctrl_legacy.h"

#ifdef QCA_PKT_PROTO_TRACE
#include "cds_packet.h"
#endif /* QCA_PKT_PROTO_TRACE */

#define HDD_FINISH_ULA_TIME_OUT         800
#define HDD_SET_MCBC_FILTERS_TO_FW      1
#define HDD_DELETE_MCBC_FILTERS_FROM_FW 0

static int ioctl_debug;
module_param(ioctl_debug, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

/* To Validate Channel against the Frequency and Vice-Versa */
static const hdd_freq_chan_map_t freq_chan_map[] = {
	{2412, 1}, {2417, 2}, {2422, 3}, {2427, 4}, {2432, 5}, {2437, 6},
	{2442, 7}, {2447, 8}, {2452, 9}, {2457, 10}, {2462, 11}, {2467, 12},
	{2472, 13}, {2484, 14}, {4920, 240}, {4940, 244}, {4960, 248},
	{4980, 252}, {5040, 208}, {5060, 212}, {5080, 216}, {5180, 36},
	{5200, 40}, {5220, 44}, {5240, 48}, {5260, 52}, {5280, 56},
	{5300, 60}, {5320, 64}, {5500, 100}, {5520, 104}, {5540, 108},
	{5560, 112}, {5580, 116}, {5600, 120}, {5620, 124}, {5640, 128},
	{5660, 132}, {5680, 136}, {5700, 140}, {5720, 144}, {5745, 149},
	{5765, 153}, {5785, 157}, {5805, 161}, {5825, 165}, {5852, 170},
	{5855, 171}, {5860, 172}, {5865, 173}, {5870, 174}, {5875, 175},
	{5880, 176}, {5885, 177}, {5890, 178}, {5895, 179}, {5900, 180},
	{5905, 181}, {5910, 182}, {5915, 183}, {5920, 184} };

#define FREQ_CHAN_MAP_TABLE_SIZE \
		(sizeof(freq_chan_map) / sizeof(freq_chan_map[0]))

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_INT_GET_NONE    (SIOCIWFIRSTPRIV + 0)
#define WE_SET_11D_STATE     1
#define WE_WOWL              2
#define WE_SET_POWER         3
#define WE_SET_MAX_ASSOC     4
#define WE_SET_SCAN_DISABLE  5
#define WE_SET_DATA_INACTIVITY_TO  6
#define WE_SET_MAX_TX_POWER  7
#define WE_SET_HIGHER_DTIM_TRANSITION   8
#define WE_SET_TM_LEVEL      9
#define WE_SET_PHYMODE       10
#define WE_SET_NSS           11
#define WE_SET_LDPC          12
#define WE_SET_TX_STBC       13
#define WE_SET_RX_STBC       14
#define WE_SET_SHORT_GI      15
#define WE_SET_RTSCTS        16
#define WE_SET_CHWIDTH       17
#define WE_SET_ANI_EN_DIS    18
#define WE_SET_ANI_POLL_PERIOD    19
#define WE_SET_ANI_LISTEN_PERIOD  20
#define WE_SET_ANI_OFDM_LEVEL     21
#define WE_SET_ANI_CCK_LEVEL      22
#define WE_SET_DYNAMIC_BW         23
#define WE_SET_TX_CHAINMASK  24
#define WE_SET_RX_CHAINMASK  25
#define WE_SET_11N_RATE      26
#define WE_SET_AMPDU         27
#define WE_SET_AMSDU         28
#define WE_SET_TXPOW_2G      29
#define WE_SET_TXPOW_5G      30
/* Private ioctl for firmware debug log */
#define WE_DBGLOG_LOG_LEVEL             31
#define WE_DBGLOG_VAP_ENABLE            32
#define WE_DBGLOG_VAP_DISABLE           33
#define WE_DBGLOG_MODULE_ENABLE         34
#define WE_DBGLOG_MODULE_DISABLE        35
#define WE_DBGLOG_MOD_LOG_LEVEL         36
#define WE_DBGLOG_TYPE                  37
#define WE_SET_TXRX_FWSTATS             38
#define WE_SET_VHT_RATE                 39
#define WE_DBGLOG_REPORT_ENABLE         40
#define WE_TXRX_FWSTATS_RESET           41
#define WE_SET_MAX_TX_POWER_2_4   42
#define WE_SET_MAX_TX_POWER_5_0   43
/* 44 is unused */
/* Private ioctl for packet powe save */
#define  WE_PPS_PAID_MATCH              45
#define  WE_PPS_GID_MATCH               46
#define  WE_PPS_EARLY_TIM_CLEAR         47
#define  WE_PPS_EARLY_DTIM_CLEAR        48
#define  WE_PPS_EOF_PAD_DELIM           49
#define  WE_PPS_MACADDR_MISMATCH        50
#define  WE_PPS_DELIM_CRC_FAIL          51
#define  WE_PPS_GID_NSTS_ZERO           52
#define  WE_PPS_RSSI_CHECK              53
#define WE_SET_SAP_AUTO_CHANNEL_SELECTION     54
#define WE_SET_HTSMPS                   55
/* Private ioctl for QPower */
#define WE_SET_QPOWER_MAX_PSPOLL_COUNT            56
#define WE_SET_QPOWER_MAX_TX_BEFORE_WAKE          57
#define WE_SET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL   58
#define WE_SET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL 59

#define WE_SET_BURST_ENABLE             60
#define WE_SET_BURST_DUR                61
/* GTX Commands */
#define WE_SET_GTX_HT_MCS               62
#define WE_SET_GTX_VHT_MCS              63
#define WE_SET_GTX_USRCFG               64
#define WE_SET_GTX_THRE                 65
#define WE_SET_GTX_MARGIN               66
#define WE_SET_GTX_STEP                 67
#define WE_SET_GTX_MINTPC               68
#define WE_SET_GTX_BWMASK               69
/* Private ioctl to configure MCC home channels time quota and latency */
#define WE_MCC_CONFIG_LATENCY           70
#define WE_MCC_CONFIG_QUOTA             71
/* Private IOCTL for debug connection issues */
#define WE_SET_DEBUG_LOG                72
#ifdef WE_SET_TX_POWER
#undef WE_SET_TX_POWER
#endif
#define WE_SET_TX_POWER                 74
/* Private ioctl for earlyrx power save feature */
#define WE_SET_EARLY_RX_ADJUST_ENABLE         75
#define WE_SET_EARLY_RX_TGT_BMISS_NUM         76
#define WE_SET_EARLY_RX_BMISS_SAMPLE_CYCLE    77
#define WE_SET_EARLY_RX_SLOP_STEP             78
#define WE_SET_EARLY_RX_INIT_SLOP             79
#define WE_SET_EARLY_RX_ADJUST_PAUSE          80
#define WE_SET_MC_RATE                        81
#define WE_SET_EARLY_RX_DRIFT_SAMPLE          82
/* Private ioctl for packet power save */
#define WE_PPS_5G_EBT                         83
#define WE_SET_CTS_CBW                        84
#define WE_DUMP_STATS                         85
#define WE_CLEAR_STATS                        86
/* Private sub ioctl for starting/stopping the profiling */
#define WE_START_FW_PROFILE                      87
#define WE_SET_CHANNEL                        88
#define WE_SET_CONC_SYSTEM_PREF               89

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_NONE_GET_INT    (SIOCIWFIRSTPRIV + 1)
#define WE_GET_11D_STATE     1
#define WE_IBSS_STATUS       2
#define WE_SET_SAP_CHANNELS  3
#define WE_GET_WLAN_DBG      4
#define WE_GET_MAX_ASSOC     6
/* 7 is unused */
#define WE_GET_SAP_AUTO_CHANNEL_SELECTION 8
#define WE_GET_CONCURRENCY_MODE 9
#define WE_GET_NSS           11
#define WE_GET_LDPC          12
#define WE_GET_TX_STBC       13
#define WE_GET_RX_STBC       14
#define WE_GET_SHORT_GI      15
#define WE_GET_RTSCTS        16
#define WE_GET_CHWIDTH       17
#define WE_GET_ANI_EN_DIS    18
#define WE_GET_ANI_POLL_PERIOD    19
#define WE_GET_ANI_LISTEN_PERIOD  20
#define WE_GET_ANI_OFDM_LEVEL     21
#define WE_GET_ANI_CCK_LEVEL      22
#define WE_GET_DYNAMIC_BW         23
#define WE_GET_TX_CHAINMASK  24
#define WE_GET_RX_CHAINMASK  25
#define WE_GET_11N_RATE      26
#define WE_GET_AMPDU         27
#define WE_GET_AMSDU         28
#define WE_GET_TXPOW_2G      29
#define WE_GET_TXPOW_5G      30
/* 31 is unused */
#define WE_GET_PPS_PAID_MATCH           32
#define WE_GET_PPS_GID_MATCH            33
#define WE_GET_PPS_EARLY_TIM_CLEAR      34
#define WE_GET_PPS_EARLY_DTIM_CLEAR     35
#define WE_GET_PPS_EOF_PAD_DELIM        36
#define WE_GET_PPS_MACADDR_MISMATCH     37
#define WE_GET_PPS_DELIM_CRC_FAIL       38
#define WE_GET_PPS_GID_NSTS_ZERO        39
#define WE_GET_PPS_RSSI_CHECK           40
/* Private ioctl for QPower */
#define WE_GET_QPOWER_MAX_PSPOLL_COUNT            41
#define WE_GET_QPOWER_MAX_TX_BEFORE_WAKE          42
#define WE_GET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL   43
#define WE_GET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL 44
#define WE_GET_BURST_ENABLE             45
#define WE_GET_BURST_DUR                46
/* GTX Commands */
#define WE_GET_GTX_HT_MCS               47
#define WE_GET_GTX_VHT_MCS              48
#define WE_GET_GTX_USRCFG               49
#define WE_GET_GTX_THRE                 50
#define WE_GET_GTX_MARGIN               51
#define WE_GET_GTX_STEP                 52
#define WE_GET_GTX_MINTPC               53
#define WE_GET_GTX_BWMASK               54
#define WE_GET_TEMPERATURE              56
#define WE_CAP_TSF                      58
#define WE_GET_ROAM_SYNCH_DELAY         59

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_INT_GET_INT     (SIOCIWFIRSTPRIV + 2)

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_CHAR_GET_NONE   (SIOCIWFIRSTPRIV + 3)
#define WE_WOWL_ADD_PTRN     1
#define WE_WOWL_DEL_PTRN     2
#define WE_NEIGHBOR_REPORT_REQUEST 3
#define WE_SET_AP_WPS_IE     4  /* This is called in station mode to set probe rsp ie. */
#define WE_SET_CONFIG        5

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_THREE_INT_GET_NONE   (SIOCIWFIRSTPRIV + 4)
#define WE_SET_WLAN_DBG      1
#define WE_SET_DP_TRACE      2
#define WE_SET_SAP_CHANNELS  3

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_GET_CHAR_SET_NONE   (SIOCIWFIRSTPRIV + 5)
#define WE_WLAN_VERSION      1
#define WE_GET_STATS         2
#define WE_GET_CFG           3
#define WE_GET_WMM_STATUS    4
#define WE_GET_CHANNEL_LIST  5
#define WE_GET_RSSI          6
#ifdef FEATURE_WLAN_TDLS
#define WE_GET_TDLS_PEERS    8
#endif
#ifdef WLAN_FEATURE_11W
#define WE_GET_11W_INFO      9
#endif
#define WE_GET_STATES        10
#define WE_GET_IBSS_STA_INFO 11
#define WE_GET_PHYMODE       12
#ifdef FEATURE_OEM_DATA_SUPPORT
#define WE_GET_OEM_DATA_CAP  13
#endif
#define WE_GET_SNR           14
#define WE_LIST_FW_PROFILE      15

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_NONE_GET_NONE   (SIOCIWFIRSTPRIV + 6)
#define WE_SET_REASSOC_TRIGGER     8
#define WE_IBSS_GET_PEER_INFO_ALL 10
#define WE_DUMP_AGC_START          11
#define WE_DUMP_AGC                12
#define WE_DUMP_CHANINFO_START     13
#define WE_DUMP_CHANINFO           14
#define WE_DUMP_WATCHDOG           15
#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
#define WE_DUMP_PCIE_LOG           16
#endif
#define WE_GET_RECOVERY_STAT       17
#define WE_GET_FW_PROFILE_DATA     18
#define WE_STOP_OBSS_SCAN          19

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_VAR_INT_GET_NONE   (SIOCIWFIRSTPRIV + 7)

#define WE_P2P_NOA_CMD       2

/* subcommands 3 and 4 are unused */

#ifdef FEATURE_WLAN_TDLS
#define WE_TDLS_CONFIG_PARAMS   5
#endif
#define WE_IBSS_GET_PEER_INFO   6
#define WE_UNIT_TEST_CMD   7

#define WE_MTRACE_DUMP_CMD    8
#define WE_MTRACE_SELECTIVE_MODULE_LOG_ENABLE_CMD    9


#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
#define WE_LED_FLASHING_PARAM    10
#endif

#define WE_POLICY_MANAGER_CLIST_CMD    11
#define WE_POLICY_MANAGER_DLIST_CMD    12
#define WE_POLICY_MANAGER_DBS_CMD      13
#define WE_POLICY_MANAGER_PCL_CMD      14
#define WE_POLICY_MANAGER_CINFO_CMD    15
#define WE_POLICY_MANAGER_ULIST_CMD    16
#define WE_POLICY_MANAGER_QUERY_ACTION_CMD    17
#define WE_POLICY_MANAGER_QUERY_ALLOW_CMD    18
#define WE_POLICY_MANAGER_SCENARIO_CMD 19
#define WE_POLICY_SET_HW_MODE_CMD 20

#define WE_SET_DUAL_MAC_SCAN_CONFIG    21
#define WE_SET_DUAL_MAC_FW_MODE_CONFIG 22
#define WE_SET_MON_MODE_CHAN 23

#ifdef FEATURE_WLAN_TDLS
#undef  MAX_VAR_ARGS
#define MAX_VAR_ARGS         11
#else
#undef  MAX_VAR_ARGS
#define MAX_VAR_ARGS         9
#endif

/* Private ioctls (with no sub-ioctls) */
/* note that they must be odd so that they have "get" semantics */
#define WLAN_PRIV_ADD_TSPEC (SIOCIWFIRSTPRIV +  9)
#define WLAN_PRIV_DEL_TSPEC (SIOCIWFIRSTPRIV + 11)
#define WLAN_PRIV_GET_TSPEC (SIOCIWFIRSTPRIV + 13)

/* (SIOCIWFIRSTPRIV + 8)  is currently unused */
/* (SIOCIWFIRSTPRIV + 10) is currently unused */
/* (SIOCIWFIRSTPRIV + 12) is currently unused */
/* (SIOCIWFIRSTPRIV + 14) is currently unused */
#define WLAN_PRIV_SET_NONE_GET_THREE_INT   (SIOCIWFIRSTPRIV + 15)
#define WE_GET_TSF      1
/* (SIOCIWFIRSTPRIV + 16) is currently unused */
/* (SIOCIWFIRSTPRIV + 17) is currently unused */
/* (SIOCIWFIRSTPRIV + 19) is currently unused */

#define WLAN_PRIV_SET_FTIES             (SIOCIWFIRSTPRIV + 20)

/* Private ioctl for setting the host offload feature */
#define WLAN_PRIV_SET_HOST_OFFLOAD (SIOCIWFIRSTPRIV + 18)

/* Private ioctl to get the statistics */
#define WLAN_GET_WLAN_STATISTICS (SIOCIWFIRSTPRIV + 21)

/* Private ioctl to set the Keep Alive Params */
#define WLAN_SET_KEEPALIVE_PARAMS (SIOCIWFIRSTPRIV + 22)

#ifdef WLAN_FEATURE_PACKET_FILTERING
/* Private ioctl to set the packet filtering params */
#define WLAN_SET_PACKET_FILTER_PARAMS (SIOCIWFIRSTPRIV + 23)
#endif


#ifdef FEATURE_WLAN_SCAN_PNO
/* Private ioctl to get the statistics */
#define WLAN_SET_PNO (SIOCIWFIRSTPRIV + 24)
#endif

#define WLAN_SET_BAND_CONFIG  (SIOCIWFIRSTPRIV + 25)

/* (SIOCIWFIRSTPRIV + 26) is currently unused */
/* (SIOCIWFIRSTPRIV + 27) is currently unused */

/* Private ioctls and their sub-ioctls */
#define WLAN_PRIV_SET_TWO_INT_GET_NONE   (SIOCIWFIRSTPRIV + 28)
#define WE_SET_SMPS_PARAM    1
#ifdef DEBUG
#define WE_SET_FW_CRASH_INJECT    2
#endif
#define WE_DUMP_DP_TRACE_LEVEL    3
#define DUMP_DP_TRACE       0
/* Private sub ioctl for enabling and setting histogram interval of profiling */
#define WE_ENABLE_FW_PROFILE    4
#define WE_SET_FW_PROFILE_HIST_INTVL    5

/* (SIOCIWFIRSTPRIV + 29) is currently unused */

/* 802.11p IOCTL */
#define WLAN_SET_DOT11P_CHANNEL_SCHED    (SIOCIWFIRSTPRIV + 30)

#define WLAN_GET_LINK_SPEED          (SIOCIWFIRSTPRIV + 31)

#define WLAN_STATS_INVALID            0
#define WLAN_STATS_RETRY_CNT          1
#define WLAN_STATS_MUL_RETRY_CNT      2
#define WLAN_STATS_TX_FRM_CNT         3
#define WLAN_STATS_RX_FRM_CNT         4
#define WLAN_STATS_FRM_DUP_CNT        5
#define WLAN_STATS_FAIL_CNT           6
#define WLAN_STATS_RTS_FAIL_CNT       7
#define WLAN_STATS_ACK_FAIL_CNT       8
#define WLAN_STATS_RTS_SUC_CNT        9
#define WLAN_STATS_RX_DISCARD_CNT     10
#define WLAN_STATS_RX_ERROR_CNT       11
#define WLAN_STATS_TX_BYTE_CNT        12

#define WLAN_STATS_RX_BYTE_CNT        13
#define WLAN_STATS_RX_RATE            14
#define WLAN_STATS_TX_RATE            15

#define WLAN_STATS_RX_UC_BYTE_CNT     16
#define WLAN_STATS_RX_MC_BYTE_CNT     17
#define WLAN_STATS_RX_BC_BYTE_CNT     18
#define WLAN_STATS_TX_UC_BYTE_CNT     19
#define WLAN_STATS_TX_MC_BYTE_CNT     20
#define WLAN_STATS_TX_BC_BYTE_CNT     21

#define FILL_TLV(__p, __type, __size, __val, __tlen) do {		\
		if ((__tlen + __size + 2) < WE_MAX_STR_LEN) {		\
			*__p++ = __type;				\
			*__p++ = __size;				\
			memcpy(__p, __val, __size);			\
			__p += __size;					\
			__tlen += __size + 2;				\
		} else {						\
			hddLog(QDF_TRACE_LEVEL_ERROR, "FILL_TLV Failed!!!");  \
		}							\
	} while (0)

#define VERSION_VALUE_MAX_LEN 32

#define TX_PER_TRACKING_DEFAULT_RATIO             5
#define TX_PER_TRACKING_MAX_RATIO                10
#define TX_PER_TRACKING_DEFAULT_WATERMARK         5

#define WLAN_ADAPTER 0
#define P2P_ADAPTER  1

/**
 * mem_alloc_copy_from_user_helper - copy from user helper
 * @wrqu_data: wireless extensions request data
 * @len: length of @wrqu_data
 *
 * Helper function to allocate buffer and copy user data.
 *
 * Return: On success return a pointer to a kernel buffer containing a
 * copy of the userspace data (with an additional NUL character
 * appended for safety). On failure return %NULL.
 */
void *mem_alloc_copy_from_user_helper(const __user void *wrqu_data, size_t len)
{
	u8 *ptr = NULL;

	/* in order to protect the code, an extra byte is post
	 * appended to the buffer and the null termination is added.
	 * However, when allocating (len+1) byte of memory, we need to
	 * make sure that there is no uint overflow when doing
	 * addition. In theory check len < UINT_MAX protects the uint
	 * overflow. For wlan private ioctl, the buffer size is much
	 * less than UINT_MAX, as a good guess, now, it is assumed
	 * that the private command buffer size is no greater than 4K
	 * (4096 bytes). So we use 4096 as the upper boundary for now.
	 */
	if (len > MAX_USER_COMMAND_SIZE) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "Invalid length");
		return NULL;
	}

	ptr = kmalloc(len + 1, GFP_KERNEL);
	if (NULL == ptr) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "unable to allocate memory");
		return NULL;
	}

	if (copy_from_user(ptr, wrqu_data, len)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: failed to copy data to user buffer", __func__);
		kfree(ptr);
		return NULL;
	}
	ptr[len] = '\0';
	return ptr;
}

/**
 * hdd_priv_get_data() - Get pointer to ioctl private data
 * @p_priv_data: pointer to iw_point struct to be filled
 * @wrqu: Pointer to IOCTL Data received from userspace
 *
 * Helper function to get compatible struct iw_point passed to ioctl
 *
 * Return - 0 if p_priv_data successfully filled, error otherwise
 */
int hdd_priv_get_data(struct iw_point *p_priv_data, union iwreq_data *wrqu)
{
	if ((NULL == p_priv_data) || (NULL == wrqu)) {
		return -EINVAL;
	}
#ifdef CONFIG_COMPAT
	if (is_compat_task()) {
		struct compat_iw_point *p_compat_priv_data;

		/* Compat task:
		 * typecast to compat structure and copy the members.
		 */
		p_compat_priv_data = (struct compat_iw_point *)&wrqu->data;

		p_priv_data->pointer = compat_ptr(p_compat_priv_data->pointer);
		p_priv_data->length = p_compat_priv_data->length;
		p_priv_data->flags = p_compat_priv_data->flags;
	} else {
#endif /* #ifdef CONFIG_COMPAT */

		/* Non compat task: directly copy the structure. */
		memcpy(p_priv_data, &wrqu->data, sizeof(struct iw_point));

#ifdef CONFIG_COMPAT
	}
#endif /* #ifdef CONFIG_COMPAT */

	return 0;
}


/**
 * hdd_wlan_get_stats() - Get txrx stats in SAP mode
 * @pAdapter: Pointer to the hdd adapter.
 * @length:   Size of the data copied
 * @buffer:   Pointer to char buffer.
 * @buf_len:  Length of the char buffer.
 *
 * This function called when the "iwpriv wlan0 get_stats" command is given.
 * It used to collect the txrx stats when the device is configured in SAP mode.
 *
 * Return - none
 */
void hdd_wlan_get_stats(hdd_adapter_t *pAdapter, uint16_t *length,
			char *buffer, uint16_t buf_len)
{
	hdd_tx_rx_stats_t *pStats = &pAdapter->hdd_stats.hddTxRxStats;
	uint32_t len = 0;
	uint32_t total_rx_pkt = 0, total_rx_dropped = 0;
	uint32_t total_rx_delv = 0, total_rx_refused = 0;
	int i = 0;

	for (; i < NUM_CPUS; i++) {
		total_rx_pkt += pStats->rxPackets[i];
		total_rx_dropped += pStats->rxDropped[i];
		total_rx_delv += pStats->rxDelivered[i];
		total_rx_refused += pStats->rxRefused[i];
	}

	len = scnprintf(buffer, buf_len,
		"\nTransmit"
		"\ncalled %u, dropped %u,"
		"\n      dropped BK %u, BE %u, VI %u, VO %u"
		"\n   classified BK %u, BE %u, VI %u, VO %u"
		"\ncompleted %u,"
		"\n\nReceive  Total"
		"\n packets %u, dropped %u, delivered %u, refused %u"
		"\n",
		pStats->txXmitCalled,
		pStats->txXmitDropped,

		pStats->txXmitDroppedAC[SME_AC_BK],
		pStats->txXmitDroppedAC[SME_AC_BE],
		pStats->txXmitDroppedAC[SME_AC_VI],
		pStats->txXmitDroppedAC[SME_AC_VO],

		pStats->txXmitClassifiedAC[SME_AC_BK],
		pStats->txXmitClassifiedAC[SME_AC_BE],
		pStats->txXmitClassifiedAC[SME_AC_VI],
		pStats->txXmitClassifiedAC[SME_AC_VO],

		pStats->txCompleted,
		total_rx_pkt, total_rx_dropped, total_rx_delv, total_rx_refused
		);

	for (i = 0; i < NUM_CPUS; i++) {
		len += scnprintf(buffer + len, buf_len - len,
			"\nReceive CPU: %d"
			"\n  packets %u, dropped %u, delivered %u, refused %u",
			i, pStats->rxPackets[i], pStats->rxDropped[i],
			pStats->rxDelivered[i], pStats->rxRefused[i]);
	}

	len += scnprintf(buffer + len, buf_len - len,
		"\n\nTX_FLOW"
		"\nCurrent status: %s"
		"\ntx-flow timer start count %u"
		"\npause count %u, unpause count %u",
		(pStats->is_txflow_paused == true ? "PAUSED" : "UNPAUSED"),
		pStats->txflow_timer_cnt,
		pStats->txflow_pause_cnt,
		pStats->txflow_unpause_cnt);

	len += ol_txrx_stats(pAdapter->sessionId,
		&buffer[len], (buf_len - len));

	len += hdd_napi_stats(buffer + len, buf_len - len,
			   NULL, hdd_napi_get_all());

	*length = len + 1;
}

/**
 * hdd_wlan_list_fw_profile() - Get fw profiling points
 * @length:   Size of the data copied
 * @buffer:   Pointer to char buffer.
 * @buf_len:  Length of the char buffer.
 *
 * This function called when the "iwpriv wlan0 listProfile" command is given.
 * It is used to get the supported profiling points in FW.
 *
 * Return - none
 */
void hdd_wlan_list_fw_profile(uint16_t *length,
			char *buffer, uint16_t buf_len)
{
	uint32_t len = 0;

	len = scnprintf(buffer, buf_len,
		    "PROF_CPU_IDLE: %u\n"
		    "PROF_PPDU_PROC: %u\n"
		    "PROF_PPDU_POST: %u\n"
		    "PROF_HTT_TX_INPUT: %u\n"
		    "PROF_MSDU_ENQ: %u\n"
		    "PROF_PPDU_POST_HAL: %u\n"
		    "PROF_COMPUTE_TX_TIME: %u\n",
		    PROF_CPU_IDLE,
		    PROF_PPDU_PROC,
		    PROF_PPDU_POST,
		    PROF_HTT_TX_INPUT,
		    PROF_MSDU_ENQ,
		    PROF_PPDU_POST_HAL,
		    PROF_COMPUTE_TX_TIME);

	*length = len + 1;
}

/**
 * hdd_wlan_dump_stats() - display dump Stats
 * @adapter: adapter handle
 * @value: value from user
 *
 * Return: none
 */
void hdd_wlan_dump_stats(hdd_adapter_t *adapter, int value)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	switch (value) {

	case WLAN_TXRX_HIST_STATS:
		wlan_hdd_display_tx_rx_histogram(hdd_ctx);
		break;
	case WLAN_HDD_NETIF_OPER_HISTORY:
		wlan_hdd_display_netif_queue_history(hdd_ctx);
		break;
	default:
		ol_txrx_display_stats(value);
		break;
	}
}

/**
 * hdd_wlan_get_version() - Get driver version information
 * @pAdapter: Pointer to the adapter.
 * @wrqu: Pointer to IOCTL REQUEST Data.
 * @extra: Pointer to destination buffer
 *
 * This function is used to get Wlan Driver, Firmware, & Hardware
 * Version information.  If @wrqu and @extra are specified, then the
 * version string is returned.  Otherwise it is simply printed to the
 * kernel log.
 *
 * Return: none
 */
void hdd_wlan_get_version(hdd_adapter_t *pAdapter, union iwreq_data *wrqu,
			  char *extra)
{
	tSirVersionString wcnss_SW_version;
	const char *pSWversion;
	const char *pHWversion;
	uint32_t MSPId = 0, mSPId = 0, SIId = 0, CRMId = 0;

	hdd_context_t *pHddContext;

	pHddContext = WLAN_HDD_GET_CTX(pAdapter);
	if (!pHddContext) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s:Invalid context, HDD context is null", __func__);
		goto error;
	}

	snprintf(wcnss_SW_version, sizeof(tSirVersionString), "%08x",
		 pHddContext->target_fw_version);

	pSWversion = wcnss_SW_version;
	MSPId = (pHddContext->target_fw_version & 0xf0000000) >> 28;
	mSPId = (pHddContext->target_fw_version & 0xf000000) >> 24;
	SIId = (pHddContext->target_fw_version & 0xf00000) >> 20;
	CRMId = pHddContext->target_fw_version & 0x7fff;

	pHWversion = pHddContext->target_hw_name;

	if (wrqu && extra) {
		wrqu->data.length =
			scnprintf(extra, WE_MAX_STR_LEN,
				  "Host SW:%s, FW:%d.%d.%d.%d, HW:%s",
				  QWLAN_VERSIONSTR,
				  MSPId, mSPId, SIId, CRMId, pHWversion);
	} else {
		pr_info("Host SW:%s, FW:%d.%d.%d.%d, HW:%s\n",
			QWLAN_VERSIONSTR,
			MSPId, mSPId, SIId, CRMId, pHWversion);
	}
error:
	return;
}

/**
 * hdd_wlan_get_ibss_mac_addr_from_staid() - Get IBSS MAC address
 * @pAdapter: Adapter upon which the IBSS client is active
 * @staIdx: Station index of the IBSS peer
 *
 * Return: a pointer to the MAC address of the IBSS peer if the peer is
 *	   found, otherwise %NULL.
 */
struct qdf_mac_addr *
hdd_wlan_get_ibss_mac_addr_from_staid(hdd_adapter_t *pAdapter,
				      uint8_t staIdx)
{
	uint8_t idx;
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	for (idx = 0; idx < MAX_IBSS_PEERS; idx++) {
		if (0 != pHddStaCtx->conn_info.staId[idx] &&
		    staIdx == pHddStaCtx->conn_info.staId[idx]) {
			return &pHddStaCtx->conn_info.peerMacAddress[idx];
		}
	}
	return NULL;
}

/**
 * hdd_wlan_get_ibss_peer_info() - Print IBSS peer information
 * @pAdapter: Adapter upon which the IBSS client is active
 * @staIdx: Station index of the IBSS peer
 *
 * Return: QDF_STATUS_STATUS if the peer was found and displayed,
 * otherwise an appropriate QDF_STATUS_E_* failure code.
 */
QDF_STATUS hdd_wlan_get_ibss_peer_info(hdd_adapter_t *pAdapter, uint8_t staIdx)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	hdd_station_ctx_t *pStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	tSirPeerInfoRspParams *pPeerInfo = &pStaCtx->ibss_peer_info;

	status =
		sme_request_ibss_peer_info(hHal, pAdapter, hdd_get_ibss_peer_info_cb,
					   false, staIdx);

	INIT_COMPLETION(pAdapter->ibss_peer_info_comp);

	if (QDF_STATUS_SUCCESS == status) {
		unsigned long rc;
		rc = wait_for_completion_timeout
			     (&pAdapter->ibss_peer_info_comp,
			     msecs_to_jiffies(IBSS_PEER_INFO_REQ_TIMOEUT));
		if (!rc) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("failed wait on ibss_peer_info_comp"));
			return QDF_STATUS_E_FAILURE;
		}

		/** Print the peer info */
		hdd_info("pPeerInfo->numIBSSPeers = %d ", pPeerInfo->numPeers);
		{
			uint8_t mac_addr[QDF_MAC_ADDR_SIZE];
			uint32_t tx_rate = pPeerInfo->peerInfoParams[0].txRate;

			qdf_mem_copy(mac_addr, pPeerInfo->peerInfoParams[0].
					mac_addr, sizeof(mac_addr));
			hdd_info("PEER ADDR : %pM TxRate: %d Mbps  RSSI: %d",
				mac_addr, (int)tx_rate,
				(int)pPeerInfo->peerInfoParams[0].rssi);
		}
	} else {
		hddLog(QDF_TRACE_LEVEL_WARN,
		       "%s: Warning: sme_request_ibss_peer_info Request failed",
		       __func__);
	}

	return status;
}

/**
 * hdd_wlan_get_ibss_peer_info_all() - Print all IBSS peers
 * @pAdapter: Adapter upon which the IBSS clients are active
 *
 * Return: QDF_STATUS_STATUS if the peer information was retrieved and
 * displayed, otherwise an appropriate QDF_STATUS_E_* failure code.
 */
QDF_STATUS hdd_wlan_get_ibss_peer_info_all(hdd_adapter_t *pAdapter)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	hdd_station_ctx_t *pStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	tSirPeerInfoRspParams *pPeerInfo = &pStaCtx->ibss_peer_info;
	int i;

	status =
		sme_request_ibss_peer_info(hHal, pAdapter, hdd_get_ibss_peer_info_cb,
					   true, 0xFF);
	INIT_COMPLETION(pAdapter->ibss_peer_info_comp);

	if (QDF_STATUS_SUCCESS == status) {
		unsigned long rc;
		rc = wait_for_completion_timeout
			     (&pAdapter->ibss_peer_info_comp,
			     msecs_to_jiffies(IBSS_PEER_INFO_REQ_TIMOEUT));
		if (!rc) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("failed wait on ibss_peer_info_comp"));
			return QDF_STATUS_E_FAILURE;
		}

		/** Print the peer info */
		hdd_info("pPeerInfo->numIBSSPeers = %d ",
			(int)pPeerInfo->numPeers);
		for (i = 0; i < pPeerInfo->numPeers; i++) {
			uint8_t mac_addr[QDF_MAC_ADDR_SIZE];
			uint32_t tx_rate;

			tx_rate = pPeerInfo->peerInfoParams[i].txRate;
			qdf_mem_copy(mac_addr,
				pPeerInfo->peerInfoParams[i].mac_addr,
				sizeof(mac_addr));

			hdd_info(" PEER ADDR : %pM TxRate: %d Mbps RSSI: %d",
				mac_addr, (int)tx_rate,
				(int)pPeerInfo->peerInfoParams[i].rssi);
		}
	} else {
		hddLog(QDF_TRACE_LEVEL_WARN,
		       "%s: Warning: sme_request_ibss_peer_info Request failed",
		       __func__);
	}

	return status;
}

/**
 * hdd_wlan_get_rts_threshold() - Get RTS threshold
 * @pAdapter: adapter upon which the request was received
 * @wrqu: pointer to the ioctl request
 *
 * This function retrieves the current RTS threshold value and stores
 * it in the ioctl request structure
 *
 * Return: 0 if valid data was returned, non-zero on error
 */
int hdd_wlan_get_rts_threshold(hdd_adapter_t *pAdapter, union iwreq_data *wrqu)
{
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	uint32_t threshold = 0;
	hdd_context_t *hdd_ctx;
	int ret = 0;

	ENTER();

	if (NULL == pAdapter) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: Adapter is NULL", __func__);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (QDF_STATUS_SUCCESS !=
	    sme_cfg_get_int(hHal, WNI_CFG_RTS_THRESHOLD, &threshold)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  FL
				  ("failed to get ini parameter, WNI_CFG_RTS_THRESHOLD"));
		return -EIO;
	}
	wrqu->rts.value = threshold;

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  ("Rts-Threshold=%d!!"), wrqu->rts.value);

	EXIT();

	return 0;
}

/**
 * hdd_wlan_get_frag_threshold() - Get fragmentation threshold
 * @pAdapter: adapter upon which the request was received
 * @wrqu: pointer to the ioctl request
 *
 * This function retrieves the current fragmentation threshold value
 * and stores it in the ioctl request structure
 *
 * Return: 0 if valid data was returned, non-zero on error
 */
int hdd_wlan_get_frag_threshold(hdd_adapter_t *pAdapter,
				union iwreq_data *wrqu)
{
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	uint32_t threshold = 0, status = 0;
	hdd_context_t *hdd_ctx;

	ENTER();

	if (NULL == pAdapter) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: Adapter is NULL", __func__);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return status;

	if (sme_cfg_get_int(hHal, WNI_CFG_FRAGMENTATION_THRESHOLD, &threshold)
	    != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  FL
				  ("failed to get ini parameter, WNI_CFG_FRAGMENTATION_THRESHOLD"));
		return -EIO;
	}
	wrqu->frag.value = threshold;

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  ("Frag-Threshold=%d!!"), wrqu->frag.value);

	EXIT();

	return 0;
}

/**
 * hdd_wlan_get_freq() - Convert channel to frequency
 * @channel: channel to be converted
 * @pfreq: where to store the frequency
 *
 * Return: 1 on success, otherwise a negative errno
 */
int hdd_wlan_get_freq(uint32_t channel, uint32_t *pfreq)
{
	int i;
	if (channel > 0) {
		for (i = 0; i < FREQ_CHAN_MAP_TABLE_SIZE; i++) {
			if (channel == freq_chan_map[i].chan) {
				*pfreq = freq_chan_map[i].freq;
				return 1;
			}
		}
	}
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  ("Invalid channel no=%d!!"), channel);
	return -EINVAL;
}

/**
 * hdd_is_auth_type_rsn() - RSN authentication type check
 * @authType: authentication type to be checked
 *
 * Return: true if @authType is an RSN authentication type,
 *	   false if it is not
 */
static bool hdd_is_auth_type_rsn(eCsrAuthType authType)
{
	bool rsnType = false;
	/* is the authType supported? */
	switch (authType) {
	case eCSR_AUTH_TYPE_NONE:       /* never used */
		rsnType = false;
		break;
	/* MAC layer authentication types */
	case eCSR_AUTH_TYPE_OPEN_SYSTEM:
		rsnType = false;
		break;
	case eCSR_AUTH_TYPE_SHARED_KEY:
		rsnType = false;
		break;
	case eCSR_AUTH_TYPE_AUTOSWITCH:
		rsnType = false;
		break;

	/* Upper layer authentication types */
	case eCSR_AUTH_TYPE_WPA:
		rsnType = true;
		break;
	case eCSR_AUTH_TYPE_WPA_PSK:
		rsnType = true;
		break;
	case eCSR_AUTH_TYPE_WPA_NONE:
		rsnType = true;
		break;
	case eCSR_AUTH_TYPE_FT_RSN:
	case eCSR_AUTH_TYPE_RSN:
		rsnType = true;
		break;
	case eCSR_AUTH_TYPE_FT_RSN_PSK:
	case eCSR_AUTH_TYPE_RSN_PSK:
#ifdef WLAN_FEATURE_11W
	case eCSR_AUTH_TYPE_RSN_PSK_SHA256:
	case eCSR_AUTH_TYPE_RSN_8021X_SHA256:
#endif
		rsnType = true;
		break;
	/* case eCSR_AUTH_TYPE_FAILED: */
	case eCSR_AUTH_TYPE_UNKNOWN:
		rsnType = false;
		break;
	default:
		hddLog(LOGE, FL("unknown authType %d, treat as open"),
		       authType);
		rsnType = false;
		break;
	}
	hddLog(LOG1, FL("called with authType: %d, returned: %d"),
	       authType, rsnType);
	return rsnType;
}

/**
 * hdd_get_rssi_cb() - "Get RSSI" callback function
 * @rssi: Current RSSI of the station
 * @staId: ID of the station
 * @pContext: opaque context originally passed to SME.  HDD always passes
 *	a &struct statsContext
 *
 * Return: None
 */
static void hdd_get_rssi_cb(int8_t rssi, uint32_t staId, void *pContext)
{
	struct statsContext *pStatsContext;
	hdd_adapter_t *pAdapter;

	if (ioctl_debug) {
		pr_info("%s: rssi [%d] STA [%d] pContext [%p]\n",
			__func__, (int)rssi, (int)staId, pContext);
	}

	if (NULL == pContext) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Bad param, pContext [%p]", __func__, pContext);
		return;
	}

	pStatsContext = pContext;
	pAdapter = pStatsContext->pAdapter;

	/* there is a race condition that exists between this callback
	 * function and the caller since the caller could time out
	 * either before or while this code is executing.  we use a
	 * spinlock to serialize these actions
	 */
	spin_lock(&hdd_context_lock);

	if ((NULL == pAdapter) ||
	    (RSSI_CONTEXT_MAGIC != pStatsContext->magic)) {
		/* the caller presumably timed out so there is nothing
		 * we can do
		 */
		spin_unlock(&hdd_context_lock);
		hddLog(QDF_TRACE_LEVEL_WARN,
		       "%s: Invalid context, pAdapter [%p] magic [%08x]",
		       __func__, pAdapter, pStatsContext->magic);
		if (ioctl_debug) {
			pr_info("%s: Invalid context, pAdapter [%p] magic [%08x]\n",
				__func__, pAdapter, pStatsContext->magic);
		}
		return;
	}

	/* context is valid so caller is still waiting */

	/* paranoia: invalidate the magic */
	pStatsContext->magic = 0;

	/* copy over the rssi */
	pAdapter->rssi = rssi;

	if (pAdapter->rssi > 0)
		pAdapter->rssi = 0;

	/* notify the caller */
	complete(&pStatsContext->completion);

	/* serialization is complete */
	spin_unlock(&hdd_context_lock);
}

/**
 * hdd_get_snr_cb() - "Get SNR" callback function
 * @snr: Current SNR of the station
 * @staId: ID of the station
 * @pContext: opaque context originally passed to SME.  HDD always passes
 *	a &struct statsContext
 *
 * Return: None
 */
static void hdd_get_snr_cb(int8_t snr, uint32_t staId, void *pContext)
{
	struct statsContext *pStatsContext;
	hdd_adapter_t *pAdapter;

	if (ioctl_debug) {
		pr_info("%s: snr [%d] STA [%d] pContext [%p]\n",
			__func__, (int)snr, (int)staId, pContext);
	}

	if (NULL == pContext) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Bad param, pContext [%p]", __func__, pContext);
		return;
	}

	pStatsContext = pContext;
	pAdapter = pStatsContext->pAdapter;

	/* there is a race condition that exists between this callback
	 * function and the caller since the caller could time out
	 * either before or while this code is executing.  we use a
	 * spinlock to serialize these actions
	 */
	spin_lock(&hdd_context_lock);

	if ((NULL == pAdapter) || (SNR_CONTEXT_MAGIC != pStatsContext->magic)) {
		/* the caller presumably timed out so there is nothing
		 * we can do
		 */
		spin_unlock(&hdd_context_lock);
		hddLog(QDF_TRACE_LEVEL_WARN,
		       "%s: Invalid context, pAdapter [%p] magic [%08x]",
		       __func__, pAdapter, pStatsContext->magic);
		if (ioctl_debug) {
			pr_info("%s: Invalid context, pAdapter [%p] magic [%08x]\n",
				__func__, pAdapter, pStatsContext->magic);
		}
		return;
	}

	/* context is valid so caller is still waiting */

	/* paranoia: invalidate the magic */
	pStatsContext->magic = 0;

	/* copy over the snr */
	pAdapter->snr = snr;

	/* notify the caller */
	complete(&pStatsContext->completion);

	/* serialization is complete */
	spin_unlock(&hdd_context_lock);
}

/**
 * wlan_hdd_get_rssi() - Get the current RSSI
 * @pAdapter: adapter upon which the measurement is requested
 * @rssi_value: pointer to where the RSSI should be returned
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_hdd_get_rssi(hdd_adapter_t *pAdapter, int8_t *rssi_value)
{
	struct statsContext context;
	hdd_context_t *pHddCtx;
	hdd_station_ctx_t *pHddStaCtx;
	QDF_STATUS hstatus;
	unsigned long rc;

	if (NULL == pAdapter) {
		hddLog(QDF_TRACE_LEVEL_WARN,
		       "%s: Invalid context, pAdapter", __func__);
		return QDF_STATUS_E_FAULT;
	}
	if (cds_is_driver_recovering()) {
		hdd_err("Recovery in Progress. State: 0x%x Ignore!!!",
			cds_get_driver_state());
		/* return a cached value */
		*rssi_value = pAdapter->rssi;
		return QDF_STATUS_SUCCESS;
	}

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	if (eConnectionState_Associated != pHddStaCtx->conn_info.connState) {
		hdd_err("Not associated!, return last connected AP rssi!");
		*rssi_value = pAdapter->rssi;
		return QDF_STATUS_SUCCESS;
	}

	if (pHddStaCtx->hdd_ReassocScenario) {
		hdd_info("Roaming in progress, return cached RSSI");
		*rssi_value = pAdapter->rssi;
		return QDF_STATUS_SUCCESS;
	}

	init_completion(&context.completion);
	context.pAdapter = pAdapter;
	context.magic = RSSI_CONTEXT_MAGIC;

	hstatus = sme_get_rssi(pHddCtx->hHal, hdd_get_rssi_cb,
			       pHddStaCtx->conn_info.staId[0],
			       pHddStaCtx->conn_info.bssId, pAdapter->rssi,
			       &context, pHddCtx->pcds_context);
	if (QDF_STATUS_SUCCESS != hstatus) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s: Unable to retrieve RSSI",
		       __func__);
		/* we'll returned a cached value below */
	} else {
		/* request was sent -- wait for the response */
		rc = wait_for_completion_timeout(&context.completion,
						 msecs_to_jiffies
							 (WLAN_WAIT_TIME_STATS));
		if (!rc) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("SME timed out while retrieving RSSI"));
			/* we'll now returned a cached value below */
		}
	}

	/* either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.  if
	 * we never sent a request or if we sent a request and got a
	 * response, we want to clear the magic out of paranoia.  if
	 * we timed out there is a race condition such that the
	 * callback function could be executing at the same time we
	 * are. of primary concern is if the callback function had
	 * already verified the "magic" but had not yet set the
	 * completion variable when a timeout occurred. we serialize
	 * these activities by invalidating the magic while holding a
	 * shared spinlock which will cause us to block if the
	 * callback is currently executing
	 */
	spin_lock(&hdd_context_lock);
	context.magic = 0;
	spin_unlock(&hdd_context_lock);

	*rssi_value = pAdapter->rssi;

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_hdd_get_snr() - Get the current SNR
 * @pAdapter: adapter upon which the measurement is requested
 * @snr: pointer to where the SNR should be returned
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_hdd_get_snr(hdd_adapter_t *pAdapter, int8_t *snr)
{
	struct statsContext context;
	hdd_context_t *pHddCtx;
	hdd_station_ctx_t *pHddStaCtx;
	QDF_STATUS hstatus;
	unsigned long rc;
	int valid;

	ENTER();

	if (NULL == pAdapter) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Invalid context, pAdapter", __func__);
		return QDF_STATUS_E_FAULT;
	}

	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	valid = wlan_hdd_validate_context(pHddCtx);
	if (0 != valid)
		return QDF_STATUS_E_FAULT;

	pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	init_completion(&context.completion);
	context.pAdapter = pAdapter;
	context.magic = SNR_CONTEXT_MAGIC;

	hstatus = sme_get_snr(pHddCtx->hHal, hdd_get_snr_cb,
			      pHddStaCtx->conn_info.staId[0],
			      pHddStaCtx->conn_info.bssId, &context);
	if (QDF_STATUS_SUCCESS != hstatus) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s: Unable to retrieve RSSI",
		       __func__);
		/* we'll returned a cached value below */
	} else {
		/* request was sent -- wait for the response */
		rc = wait_for_completion_timeout(&context.completion,
						 msecs_to_jiffies
							 (WLAN_WAIT_TIME_STATS));
		if (!rc) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("SME timed out while retrieving SNR"));
			/* we'll now returned a cached value below */
		}
	}

	/* either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.  if
	 * we never sent a request or if we sent a request and got a
	 * response, we want to clear the magic out of paranoia.  if
	 * we timed out there is a race condition such that the
	 * callback function could be executing at the same time we
	 * are. of primary concern is if the callback function had
	 * already verified the "magic" but had not yet set the
	 * completion variable when a timeout occurred. we serialize
	 * these activities by invalidating the magic while holding a
	 * shared spinlock which will cause us to block if the
	 * callback is currently executing
	 */
	spin_lock(&hdd_context_lock);
	context.magic = 0;
	spin_unlock(&hdd_context_lock);

	*snr = pAdapter->snr;
	EXIT();
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_get_link_speed_cb() - Get link speed callback function
 * @pLinkSpeed: pointer to the link speed record
 * @pContext: pointer to the user context passed to SME
 *
 * This function is passed as the callback function to
 * sme_get_link_speed() by wlan_hdd_get_linkspeed_for_peermac().  By
 * agreement a &struct linkspeedContext is passed as @pContext.  If
 * the context is valid, then the contents of @pLinkSpeed are copied
 * into the adapter record referenced by @pContext where they can be
 * subsequently retrieved.  If the context is invalid, then this
 * function does nothing since it is assumed the caller has already
 * timed-out and destroyed the context.
 *
 * Return: None.
 */
static void
hdd_get_link_speed_cb(tSirLinkSpeedInfo *pLinkSpeed, void *pContext)
{
	struct linkspeedContext *pLinkSpeedContext;
	hdd_adapter_t *pAdapter;

	if ((NULL == pLinkSpeed) || (NULL == pContext)) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Bad param, pLinkSpeed [%p] pContext [%p]",
		       __func__, pLinkSpeed, pContext);
		return;
	}
	spin_lock(&hdd_context_lock);
	pLinkSpeedContext = pContext;
	pAdapter = pLinkSpeedContext->pAdapter;

	/* there is a race condition that exists between this callback
	 * function and the caller since the caller could time out either
	 * before or while this code is executing.  we use a spinlock to
	 * serialize these actions
	 */

	if ((NULL == pAdapter) ||
	    (LINK_CONTEXT_MAGIC != pLinkSpeedContext->magic)) {
		/* the caller presumably timed out so there is nothing
		 * we can do
		 */
		spin_unlock(&hdd_context_lock);
		hddLog(QDF_TRACE_LEVEL_WARN,
		       "%s: Invalid context, pAdapter [%p] magic [%08x]",
		       __func__, pAdapter, pLinkSpeedContext->magic);
		if (ioctl_debug) {
			pr_info("%s: Invalid context, pAdapter [%p] magic [%08x]\n",
				__func__, pAdapter, pLinkSpeedContext->magic);
		}
		return;
	}

	/* context is valid so caller is still waiting */

	/* paranoia: invalidate the magic */
	pLinkSpeedContext->magic = 0;

	/* copy over the stats. do so as a struct copy */
	pAdapter->ls_stats = *pLinkSpeed;

	/* notify the caller */
	complete(&pLinkSpeedContext->completion);

	/* serialization is complete */
	spin_unlock(&hdd_context_lock);
}

/**
 * wlan_hdd_get_linkspeed_for_peermac() - Get link speed for a peer
 * @pAdapter: adapter upon which the peer is active
 * @macAddress: MAC address of the peer
 *
 * This function will send a query to SME for the linkspeed of the
 * given peer, and then wait for the callback to be invoked.
 *
 * Return: QDF_STATUS_SUCCESS if linkspeed data is available,
 * otherwise a QDF_STATUS_E_** error.
 */
QDF_STATUS wlan_hdd_get_linkspeed_for_peermac(hdd_adapter_t *pAdapter,
					      struct qdf_mac_addr macAddress) {
	QDF_STATUS status;
	unsigned long rc;
	struct linkspeedContext context;
	tSirLinkSpeedInfo *linkspeed_req;

	if (NULL == pAdapter) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s: pAdapter is NULL", __func__);
		return QDF_STATUS_E_FAULT;
	}
	linkspeed_req = qdf_mem_malloc(sizeof(*linkspeed_req));
	if (NULL == linkspeed_req) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s Request Buffer Alloc Fail", __func__);
		return QDF_STATUS_E_NOMEM;
	}
	init_completion(&context.completion);
	context.pAdapter = pAdapter;
	context.magic = LINK_CONTEXT_MAGIC;

	qdf_copy_macaddr(&linkspeed_req->peer_macaddr, &macAddress);
	status = sme_get_link_speed(WLAN_HDD_GET_HAL_CTX(pAdapter),
				    linkspeed_req,
				    &context, hdd_get_link_speed_cb);
	if (QDF_STATUS_SUCCESS != status) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Unable to retrieve statistics for link speed",
		       __func__);
		qdf_mem_free(linkspeed_req);
	} else {
		rc = wait_for_completion_timeout
			(&context.completion,
			 msecs_to_jiffies(WLAN_WAIT_TIME_STATS));
		if (!rc) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: SME timed out while retrieving link speed",
			       __func__);
		}
	}

	/* either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.  if
	 * we never sent a request or if we sent a request and got a
	 * response, we want to clear the magic out of paranoia.  if
	 * we timed out there is a race condition such that the
	 * callback function could be executing at the same time we
	 * are. of primary concern is if the callback function had
	 * already verified the "magic" but had not yet set the
	 * completion variable when a timeout occurred. we serialize
	 * these activities by invalidating the magic while holding a
	 * shared spinlock which will cause us to block if the
	 * callback is currently executing
	 */
	spin_lock(&hdd_context_lock);
	context.magic = 0;
	spin_unlock(&hdd_context_lock);
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_hdd_get_link_speed() - get link speed
 * @pAdapter:     pointer to the adapter
 * @link_speed:   pointer to link speed
 *
 * This function fetches per bssid link speed.
 *
 * Return: if associated, link speed shall be returned.
 *         if not associated, link speed of 0 is returned.
 *         On error, error number will be returned.
 */
int wlan_hdd_get_link_speed(hdd_adapter_t *sta_adapter, uint32_t *link_speed)
{
	hdd_context_t *hddctx = WLAN_HDD_GET_CTX(sta_adapter);
	hdd_station_ctx_t *hdd_stactx =
				WLAN_HDD_GET_STATION_CTX_PTR(sta_adapter);
	int ret;

	ret = wlan_hdd_validate_context(hddctx);
	if (ret)
		return ret;

	if (eConnectionState_Associated != hdd_stactx->conn_info.connState) {
		/* we are not connected so we don't have a classAstats */
		*link_speed = 0;
	} else {
		QDF_STATUS status;
		struct qdf_mac_addr bssid;

		qdf_copy_macaddr(&bssid, &hdd_stactx->conn_info.bssId);

		status = wlan_hdd_get_linkspeed_for_peermac(sta_adapter, bssid);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			hddLog(LOGE, FL("Unable to retrieve SME linkspeed"));
			return -EINVAL;
		}
		*link_speed = sta_adapter->ls_stats.estLinkSpeed;
		/* linkspeed in units of 500 kbps */
		*link_speed = (*link_speed) / 500;
	}
	return 0;
}

/**
 * hdd_statistics_cb() - "Get statistics" callback function
 * @pStats: statistics payload
 * @pContext: opaque context originally passed to SME.  HDD always passes
 *	a pointer to an adapter
 *
 * Return: None
 */
void hdd_statistics_cb(void *pStats, void *pContext)
{
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) pContext;
	hdd_stats_t *pStatsCache = NULL;
	hdd_wext_state_t *pWextState;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	tCsrSummaryStatsInfo *pSummaryStats = NULL;
	tCsrGlobalClassAStatsInfo *pClassAStats = NULL;
	tCsrGlobalClassBStatsInfo *pClassBStats = NULL;
	tCsrGlobalClassCStatsInfo *pClassCStats = NULL;
	tCsrGlobalClassDStatsInfo *pClassDStats = NULL;
	tCsrPerStaStatsInfo *pPerStaStats = NULL;

	if (pAdapter != NULL)
		pStatsCache = &pAdapter->hdd_stats;

	pSummaryStats = (tCsrSummaryStatsInfo *) pStats;
	pClassAStats = (tCsrGlobalClassAStatsInfo *) (pSummaryStats + 1);
	pClassBStats = (tCsrGlobalClassBStatsInfo *) (pClassAStats + 1);
	pClassCStats = (tCsrGlobalClassCStatsInfo *) (pClassBStats + 1);
	pClassDStats = (tCsrGlobalClassDStatsInfo *) (pClassCStats + 1);
	pPerStaStats = (tCsrPerStaStatsInfo *) (pClassDStats + 1);

	if (pStatsCache != NULL) {
		/* copy the stats into the cache we keep in the
		 * adapter instance structure
		 */
		qdf_mem_copy(&pStatsCache->summary_stat, pSummaryStats,
			     sizeof(pStatsCache->summary_stat));
		qdf_mem_copy(&pStatsCache->ClassA_stat, pClassAStats,
			     sizeof(pStatsCache->ClassA_stat));
		qdf_mem_copy(&pStatsCache->ClassB_stat, pClassBStats,
			     sizeof(pStatsCache->ClassB_stat));
		qdf_mem_copy(&pStatsCache->ClassC_stat, pClassCStats,
			     sizeof(pStatsCache->ClassC_stat));
		qdf_mem_copy(&pStatsCache->ClassD_stat, pClassDStats,
			     sizeof(pStatsCache->ClassD_stat));
		qdf_mem_copy(&pStatsCache->perStaStats, pPerStaStats,
			     sizeof(pStatsCache->perStaStats));
	}

	if (pAdapter) {
		pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
		qdf_status = qdf_event_set(&pWextState->hdd_qdf_event);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hddLog(LOGE, FL("qdf_event_set failed"));
			return;
		}
	}
}

/**
 * hdd_clear_roam_profile_ie() - Clear Roam Profile IEs
 * @pAdapter: adapter who's IEs are to be cleared
 *
 * Return: None
 */
void hdd_clear_roam_profile_ie(hdd_adapter_t *pAdapter)
{
	int i = 0;
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

	/* clear WPA/RSN/WSC IE information in the profile */
	pWextState->roamProfile.nWPAReqIELength = 0;
	pWextState->roamProfile.pWPAReqIE = (uint8_t *) NULL;
	pWextState->roamProfile.nRSNReqIELength = 0;
	pWextState->roamProfile.pRSNReqIE = (uint8_t *) NULL;

#ifdef FEATURE_WLAN_WAPI
	pWextState->roamProfile.nWAPIReqIELength = 0;
	pWextState->roamProfile.pWAPIReqIE = (uint8_t *) NULL;
#endif

	pWextState->roamProfile.bWPSAssociation = false;
	pWextState->roamProfile.bOSENAssociation = false;
	pWextState->roamProfile.pAddIEScan = (uint8_t *) NULL;
	pWextState->roamProfile.nAddIEScanLength = 0;
	pWextState->roamProfile.pAddIEAssoc = (uint8_t *) NULL;
	pWextState->roamProfile.nAddIEAssocLength = 0;

	pWextState->roamProfile.EncryptionType.numEntries = 1;
	pWextState->roamProfile.EncryptionType.encryptionType[0]
		= eCSR_ENCRYPT_TYPE_NONE;

	pWextState->roamProfile.mcEncryptionType.numEntries = 1;
	pWextState->roamProfile.mcEncryptionType.encryptionType[0]
		= eCSR_ENCRYPT_TYPE_NONE;

	pWextState->roamProfile.AuthType.numEntries = 1;
	pWextState->roamProfile.AuthType.authType[0] =
		eCSR_AUTH_TYPE_OPEN_SYSTEM;

#ifdef WLAN_FEATURE_11W
	pWextState->roamProfile.MFPEnabled = false;
	pWextState->roamProfile.MFPRequired = 0;
	pWextState->roamProfile.MFPCapable = 0;
#endif

	pWextState->authKeyMgmt = 0;

	for (i = 0; i < CSR_MAX_NUM_KEY; i++) {
		if (pWextState->roamProfile.Keys.KeyMaterial[i]) {
			pWextState->roamProfile.Keys.KeyLength[i] = 0;
		}
	}
#ifdef FEATURE_WLAN_WAPI
	pAdapter->wapi_info.wapiAuthMode = WAPI_AUTH_MODE_OPEN;
	pAdapter->wapi_info.nWapiMode = 0;
#endif

	qdf_zero_macaddr(&pWextState->req_bssId);

}

/**
 * wlan_hdd_get_vendor_oui_ie_ptr() - Find a vendor OUI
 * @oui: The OUI that is being searched for
 * @oui_size: The length of @oui
 * @ie: The set of IEs within which we're trying to find @oui
 * @ie_len: The length of @ie
 *
 * This function will scan the IEs contained within @ie looking for @oui.
 *
 * Return: Pointer to @oui embedded within @ie if it is present, NULL
 * if @oui is not present within @ie.
 */
uint8_t *wlan_hdd_get_vendor_oui_ie_ptr(uint8_t *oui, uint8_t oui_size,
					uint8_t *ie, int ie_len)
{
	int left = ie_len;
	uint8_t *ptr = ie;
	uint8_t elem_id, elem_len;
	uint8_t eid = 0xDD;

	if (NULL == ie || 0 == ie_len)
		return NULL;

	while (left >= 2) {
		elem_id = ptr[0];
		elem_len = ptr[1];
		left -= 2;
		if (elem_len > left) {
			hddLog(QDF_TRACE_LEVEL_FATAL,
			       FL
				       ("****Invalid IEs eid = %d elem_len=%d left=%d*****"),
			       eid, elem_len, left);
			return NULL;
		}
		if (elem_id == eid) {
			if (memcmp(&ptr[2], oui, oui_size) == 0)
				return ptr;
		}

		left -= elem_len;
		ptr += (elem_len + 2);
	}
	return NULL;
}

/**
 * __iw_set_commit() - SIOCSIWCOMMIT ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_commit(struct net_device *dev, struct iw_request_info *info,
			 union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *adapter;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/* Do nothing for now */
	return 0;
}

/**
 * iw_set_commit() - SSR wrapper function for __iw_set_commit
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: extra
 *
 * Return: 0 on success, error number otherwise
 */
int iw_set_commit(struct net_device *dev, struct iw_request_info *info,
		  union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_commit(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_name() - SIOCGIWNAME ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_name(struct net_device *dev,
		       struct iw_request_info *info, char *wrqu, char *extra)
{
	hdd_adapter_t *adapter;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	adapter  = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	strlcpy(wrqu, "Qcom:802.11n", IFNAMSIZ);
	EXIT();
	return 0;
}

/**
 * __iw_get_name() - SSR wrapper for __iw_get_name
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: extra
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_name(struct net_device *dev,
			 struct iw_request_info *info,
			 char *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_name(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_mode() - ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_mode(struct net_device *dev,
			 struct iw_request_info *info,
			 union iwreq_data *wrqu, char *extra)
{
	hdd_wext_state_t *pWextState;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	tCsrRoamProfile *pRoamProfile;
	eCsrRoamBssType LastBSSType;
	eMib_dot11DesiredBssType connectedBssType;
	struct hdd_config *pConfig;
	struct wireless_dev *wdev;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	wdev = dev->ieee80211_ptr;
	pRoamProfile = &pWextState->roamProfile;
	LastBSSType = pRoamProfile->BSSType;

	hddLog(LOG1, "%s Old Bss type = %d", __func__, LastBSSType);

	switch (wrqu->mode) {
	case IW_MODE_ADHOC:
		hddLog(LOG1, "%s Setting AP Mode as IW_MODE_ADHOC", __func__);
		pRoamProfile->BSSType = eCSR_BSS_TYPE_START_IBSS;
		/* Set the phymode correctly for IBSS. */
		pConfig = (WLAN_HDD_GET_CTX(pAdapter))->config;
		pWextState->roamProfile.phyMode =
			hdd_cfg_xlate_to_csr_phy_mode(pConfig->dot11Mode);
		pAdapter->device_mode = QDF_IBSS_MODE;
		wdev->iftype = NL80211_IFTYPE_ADHOC;
		break;
	case IW_MODE_INFRA:
		hddLog(LOG1, "%s Setting AP Mode as IW_MODE_INFRA", __func__);
		pRoamProfile->BSSType = eCSR_BSS_TYPE_INFRASTRUCTURE;
		wdev->iftype = NL80211_IFTYPE_STATION;
		break;
	case IW_MODE_AUTO:
		hddLog(LOG1, "%s Setting AP Mode as IW_MODE_AUTO", __func__);
		pRoamProfile->BSSType = eCSR_BSS_TYPE_ANY;
		break;
	default:
		hddLog(LOGE, "%s Unknown AP Mode value %d ", __func__,
		       wrqu->mode);
		return -EOPNOTSUPP;
	}

	if (LastBSSType != pRoamProfile->BSSType) {
		/* the BSS mode changed.  We need to issue disconnect
		 * if connected or in IBSS disconnect state
		 */
		if (hdd_conn_get_connected_bss_type
			    (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter), &connectedBssType)
		    || (eCSR_BSS_TYPE_START_IBSS == LastBSSType)) {
			QDF_STATUS qdf_status;
			/* need to issue a disconnect to CSR. */
			INIT_COMPLETION(pAdapter->disconnect_comp_var);
			qdf_status =
				sme_roam_disconnect(WLAN_HDD_GET_HAL_CTX(pAdapter),
						    pAdapter->sessionId,
						    eCSR_DISCONNECT_REASON_IBSS_LEAVE);
			if (QDF_STATUS_SUCCESS == qdf_status) {
				unsigned long rc;
				rc = wait_for_completion_timeout(&pAdapter->
								 disconnect_comp_var,
								 msecs_to_jiffies
									 (WLAN_WAIT_TIME_DISCONNECT));
				if (!rc)
					hddLog(QDF_TRACE_LEVEL_ERROR,
					       FL
						       ("failed wait on disconnect_comp_var"));
			}
		}
	}

	EXIT();
	return 0;
}

/**
 * iw_set_mode() - SSR wrapper for __iw_set_mode()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_set_mode(struct net_device *dev, struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_mode(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_mode() - SIOCGIWMODE ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int
__iw_get_mode(struct net_device *dev, struct iw_request_info *info,
	      union iwreq_data *wrqu, char *extra)
{
	hdd_wext_state_t *pWextState;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

	switch (pWextState->roamProfile.BSSType) {
	case eCSR_BSS_TYPE_INFRASTRUCTURE:
		hddLog(LOG1, FL("returns IW_MODE_INFRA"));
		wrqu->mode = IW_MODE_INFRA;
		break;
	case eCSR_BSS_TYPE_IBSS:
	case eCSR_BSS_TYPE_START_IBSS:
		hddLog(LOG1, FL("returns IW_MODE_ADHOC"));
		wrqu->mode = IW_MODE_ADHOC;
		break;
	case eCSR_BSS_TYPE_ANY:
	default:
		hddLog(LOG1, FL("returns IW_MODE_AUTO"));
		wrqu->mode = IW_MODE_AUTO;
		break;
	}

	EXIT();
	return 0;
}

/**
 * iw_get_mode() - SSR wrapper for __iw_get_mode()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_mode(struct net_device *dev, struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_mode(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_freq() - SIOCSIWFREQ ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_freq(struct net_device *dev, struct iw_request_info *info,
			 union iwreq_data *wrqu, char *extra)
{
	uint32_t numChans = 0;
	uint8_t validChan[WNI_CFG_VALID_CHANNEL_LIST_LEN];
	uint32_t indx = 0;
	int ret;
	hdd_wext_state_t *pWextState;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	tCsrRoamProfile *pRoamProfile;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

	pRoamProfile = &pWextState->roamProfile;

	hddLog(LOG1, "setCHANNEL ioctl");

	/* Link is up then return cant set channel */
	if (eConnectionState_IbssConnected == pHddStaCtx->conn_info.connState ||
	    eConnectionState_Associated == pHddStaCtx->conn_info.connState) {
		hddLog(LOGE, "IBSS Associated");
		return -EOPNOTSUPP;
	}

	/* Settings by Frequency as input */
	if ((wrqu->freq.e == 1) && (wrqu->freq.m >= (uint32_t) 2.412e8) &&
	    (wrqu->freq.m <= (uint32_t) 5.825e8)) {
		uint32_t freq = wrqu->freq.m / 100000;

		while ((indx < FREQ_CHAN_MAP_TABLE_SIZE)
		       && (freq != freq_chan_map[indx].freq))
			indx++;
		if (indx >= FREQ_CHAN_MAP_TABLE_SIZE) {
			return -EINVAL;
		}
		wrqu->freq.e = 0;
		wrqu->freq.m = freq_chan_map[indx].chan;

	}

	if (wrqu->freq.e == 0) {
		if ((wrqu->freq.m < WNI_CFG_CURRENT_CHANNEL_STAMIN) ||
		    (wrqu->freq.m > WNI_CFG_CURRENT_CHANNEL_STAMAX)) {
			hddLog(LOG1,
			       "%s: Channel [%d] is outside valid range from %d to %d",
			       __func__, wrqu->freq.m,
			       WNI_CFG_CURRENT_CHANNEL_STAMIN,
			       WNI_CFG_CURRENT_CHANNEL_STAMAX);
			return -EINVAL;
		}

		numChans = WNI_CFG_VALID_CHANNEL_LIST_LEN;

		if (sme_cfg_get_str(hHal, WNI_CFG_VALID_CHANNEL_LIST,
				    validChan, &numChans) !=
				QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN, FL
				  ("failed to get ini parameter, WNI_CFG_VALID_CHANNEL_LIST"));
			return -EIO;
		}

		for (indx = 0; indx < numChans; indx++) {
			if (wrqu->freq.m == validChan[indx]) {
				break;
			}
		}
	} else {

		return -EINVAL;
	}

	if (indx >= numChans) {
		return -EINVAL;
	}

	/* Set the Operational Channel */
	numChans = pRoamProfile->ChannelInfo.numOfChannels = 1;
	pHddStaCtx->conn_info.operationChannel = wrqu->freq.m;
	pRoamProfile->ChannelInfo.ChannelList =
		&pHddStaCtx->conn_info.operationChannel;

	hddLog(LOG1, "pRoamProfile->operationChannel  = %d", wrqu->freq.m);

	EXIT();

	return ret;
}

/**
 * iw_set_freq() - SSR wrapper for __iw_set_freq()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_set_freq(struct net_device *dev, struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_freq(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_freq() - SIOCGIWFREQ ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_freq(struct net_device *dev, struct iw_request_info *info,
			 struct iw_freq *fwrq, char *extra)
{
	uint32_t status = false, channel = 0, freq = 0;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal;
	hdd_wext_state_t *pWextState;
	tCsrRoamProfile *pRoamProfile;
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);

	pRoamProfile = &pWextState->roamProfile;

	if (pHddStaCtx->conn_info.connState == eConnectionState_Associated) {
		if (sme_get_operation_channel(hHal, &channel, pAdapter->sessionId)
		    != QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				  FL("failed to get operating channel %u"),
				  pAdapter->sessionId);
			return -EIO;
		} else {
			status = hdd_wlan_get_freq(channel, &freq);
			if (true == status) {
				/* Set Exponent parameter as 6 (MHZ)
				 * in struct iw_freq iwlist & iwconfig
				 * command shows frequency into proper
				 * format (2.412 GHz instead of 246.2
				 * MHz)
				 */
				fwrq->m = freq;
				fwrq->e = MHZ;
			}
		}
	} else {
		/* Set Exponent parameter as 6 (MHZ) in struct iw_freq
		 * iwlist & iwconfig command shows frequency into proper
		 * format (2.412 GHz instead of 246.2 MHz)
		 */
		fwrq->m = 0;
		fwrq->e = MHZ;
	}
	return 0;
}

/**
 * iw_get_freq() - SSR wrapper for __iw_get_freq()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @fwrq: pointer to frequency data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_freq(struct net_device *dev, struct iw_request_info *info,
		       struct iw_freq *fwrq, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_freq(dev, info, fwrq, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_tx_power() - SIOCGIWTXPOW ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_tx_power(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{

	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	int ret;

	ENTER_DEV(dev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (eConnectionState_Associated != pHddStaCtx->conn_info.connState) {
		wrqu->txpower.value = 0;
		return 0;
	}
	wlan_hdd_get_class_astats(pAdapter);
	wrqu->txpower.value = pAdapter->hdd_stats.ClassA_stat.max_pwr;

	return 0;
}

/**
 * iw_get_tx_power() - SSR wrapper for __iw_get_tx_power()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_tx_power(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_tx_power(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_tx_power() - SIOCSIWTXPOW ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_tx_power(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (sme_cfg_set_int(hHal, WNI_CFG_CURRENT_TX_POWER_LEVEL,
				wrqu->txpower.value) != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR, FL
				("failed to set ini parameter, WNI_CFG_CURRENT_TX_POWER_LEVEL"));
		return -EIO;
	}

	EXIT();

	return 0;
}

/**
 * iw_set_tx_power() - SSR wrapper for __iw_set_tx_power()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_set_tx_power(struct net_device *dev,
			   struct iw_request_info *info,
			   union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_tx_power(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_bitrate() - SIOCGIWRATE ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_bitrate(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *extra)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	hdd_wext_state_t *pWextState;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (cds_is_driver_recovering()) {
		hdd_alert("Recovery in Progress. State: 0x%x Ignore!!!",
			  cds_get_driver_state());
		return status;
	}

	if (eConnectionState_Associated != pHddStaCtx->conn_info.connState) {
		wrqu->bitrate.value = 0;
	} else {
		status =
			sme_get_statistics(WLAN_HDD_GET_HAL_CTX(pAdapter),
					   eCSR_HDD,
					   SME_SUMMARY_STATS |
					   SME_GLOBAL_CLASSA_STATS |
					   SME_GLOBAL_CLASSB_STATS |
					   SME_GLOBAL_CLASSC_STATS |
					   SME_GLOBAL_CLASSD_STATS |
					   SME_PER_STA_STATS,
					   hdd_statistics_cb, 0,
					   false,
					   pHddStaCtx->conn_info.staId[0],
					   pAdapter, pAdapter->sessionId);

		if (QDF_STATUS_SUCCESS != status) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: Unable to retrieve statistics", __func__);
			return status;
		}

		pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

		qdf_status =
			qdf_wait_single_event(&pWextState->hdd_qdf_event,
					      WLAN_WAIT_TIME_STATS);

		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: SME timeout while retrieving statistics",
			       __func__);
			return QDF_STATUS_E_FAILURE;
		}

		wrqu->bitrate.value =
			pAdapter->hdd_stats.ClassA_stat.tx_rate * 500 * 1000;
	}

	EXIT();

	return qdf_status;
}

/**
 * iw_get_bitrate() - SSR wrapper for __iw_get_bitrate()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_bitrate(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_bitrate(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_bitrate() - SIOCSIWRATE ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_bitrate(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_wext_state_t *pWextState;
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	uint8_t supp_rates[WNI_CFG_SUPPORTED_RATES_11A_LEN];
	uint32_t a_len = WNI_CFG_SUPPORTED_RATES_11A_LEN;
	uint32_t b_len = WNI_CFG_SUPPORTED_RATES_11B_LEN;
	uint32_t i, rate;
	uint32_t valid_rate = false, active_phy_mode = 0;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

	if (eConnectionState_Associated != pHddStaCtx->conn_info.connState) {
		return -ENXIO;
	}

	rate = wrqu->bitrate.value;

	if (rate == -1) {
		rate = WNI_CFG_FIXED_RATE_AUTO;
		valid_rate = true;
	} else if (sme_cfg_get_int(WLAN_HDD_GET_HAL_CTX(pAdapter),
				   WNI_CFG_DOT11_MODE,
				   &active_phy_mode) == QDF_STATUS_SUCCESS) {
		if (active_phy_mode == WNI_CFG_DOT11_MODE_11A
		    || active_phy_mode == WNI_CFG_DOT11_MODE_11G
		    || active_phy_mode == WNI_CFG_DOT11_MODE_11B) {
			if ((sme_cfg_get_str(WLAN_HDD_GET_HAL_CTX(pAdapter),
				     WNI_CFG_SUPPORTED_RATES_11A, supp_rates,
				     &a_len) == QDF_STATUS_SUCCESS)
			    &&
			    (sme_cfg_get_str(WLAN_HDD_GET_HAL_CTX(pAdapter),
				     WNI_CFG_SUPPORTED_RATES_11B, supp_rates,
				     &b_len) == QDF_STATUS_SUCCESS)) {
				for (i = 0; i < (b_len + a_len); ++i) {
					/* supported rates returned is double
					 * the actual rate so we divide it by 2
					 */
					if ((supp_rates[i] & 0x7F) / 2 ==
							rate) {
						valid_rate = true;
						rate = i +
						WNI_CFG_FIXED_RATE_1MBPS;
						break;
					}
				}
			}
		}
	}
	if (valid_rate != true) {
		return -EINVAL;
	}
	if (sme_cfg_set_int(WLAN_HDD_GET_HAL_CTX(pAdapter),
			    WNI_CFG_FIXED_RATE, rate) != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR, FL
				("failed to set ini parameter, WNI_CFG_FIXED_RATE"));
		return -EIO;
	}
	return 0;
}

/**
 * iw_set_bitrate() - SSR wrapper for __iw_set_bitrate()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_set_bitrate(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_bitrate(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_genie() - SIOCSIWGENIE ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_genie(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	uint8_t *genie = NULL;
	uint8_t *base_genie = NULL;
	uint16_t remLen;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (!wrqu->data.length) {
		hdd_clear_roam_profile_ie(pAdapter);
		EXIT();
		return 0;
	}

	base_genie = mem_alloc_copy_from_user_helper(wrqu->data.pointer,
						     wrqu->data.length);
	if (NULL == base_genie) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "mem_alloc_copy_from_user_helper fail");
		return -ENOMEM;
	}

	genie = base_genie;

	remLen = wrqu->data.length;

	hddLog(LOG1, "iw_set_genie ioctl IE[0x%X], LEN[%d]", genie[0],
	       genie[1]);

	/* clear any previous genIE before this call */
	memset(&pWextState->genIE, 0, sizeof(pWextState->genIE));

	while (remLen >= 2) {
		uint16_t eLen = 0;
		uint8_t elementId;
		elementId = *genie++;
		eLen = *genie++;
		remLen -= 2;

		hddLog(QDF_TRACE_LEVEL_INFO, "%s: IE[0x%X], LEN[%d]",
		       __func__, elementId, eLen);

		switch (elementId) {
		case IE_EID_VENDOR:
			if ((IE_LEN_SIZE + IE_EID_SIZE + IE_VENDOR_OUI_SIZE) > eLen) {  /* should have at least OUI */
				ret = -EINVAL;
				goto exit;
			}

			if (0 == memcmp(&genie[0], "\x00\x50\xf2\x04", 4)) {
				uint16_t curGenIELen = pWextState->genIE.length;
				hddLog(QDF_TRACE_LEVEL_INFO,
				       "%s Set WPS OUI(%02x %02x %02x %02x) IE(len %d)",
				       __func__, genie[0], genie[1], genie[2],
				       genie[3], eLen + 2);

				if (SIR_MAC_MAX_IE_LENGTH <
				    (pWextState->genIE.length + eLen)) {
					hddLog(QDF_TRACE_LEVEL_FATAL,
					       "Cannot accommodate genIE. "
					       "Need bigger buffer space");
					QDF_ASSERT(0);
					ret = -ENOMEM;
					goto exit;
				}
				/* save to Additional IE ; it should be accumulated to handle WPS IE + other IE */
				memcpy(pWextState->genIE.addIEdata +
				       curGenIELen, genie - 2, eLen + 2);
				pWextState->genIE.length += eLen + 2;
			} else if (0 == memcmp(&genie[0], "\x00\x50\xf2", 3)) {
				hddLog(QDF_TRACE_LEVEL_INFO,
				       "%s Set WPA IE (len %d)", __func__,
				       eLen + 2);
				if ((eLen + 2) > (sizeof(pWextState->WPARSNIE))) {
					hdd_warn("Cannot accommodate genIE, Need bigger buffer space");
					ret = -EINVAL;
					QDF_ASSERT(0);
					goto exit;
				}
				memset(pWextState->WPARSNIE, 0,
				       MAX_WPA_RSN_IE_LEN);
				memcpy(pWextState->WPARSNIE, genie - 2,
				       (eLen + 2));
				pWextState->roamProfile.pWPAReqIE =
					pWextState->WPARSNIE;
				pWextState->roamProfile.nWPAReqIELength =
					eLen + 2;
			} else {        /* any vendorId except WPA IE should be accumulated to genIE */

				uint16_t curGenIELen = pWextState->genIE.length;
				hddLog(QDF_TRACE_LEVEL_INFO,
				       "%s Set OUI(%02x %02x %02x %02x) IE(len %d)",
				       __func__, genie[0], genie[1], genie[2],
				       genie[3], eLen + 2);

				if (SIR_MAC_MAX_IE_LENGTH <
				    (pWextState->genIE.length + eLen)) {
					hddLog(QDF_TRACE_LEVEL_FATAL,
					       "Cannot accommodate genIE. "
					       "Need bigger buffer space");
					QDF_ASSERT(0);
					ret = -ENOMEM;
					goto exit;
				}
				/* save to Additional IE ; it should be accumulated to handle WPS IE + other IE */
				memcpy(pWextState->genIE.addIEdata +
				       curGenIELen, genie - 2, eLen + 2);
				pWextState->genIE.length += eLen + 2;
			}
			break;
		case DOT11F_EID_RSN:
			hddLog(LOG1, "%s Set RSN IE (len %d)", __func__,
			       eLen + 2);
			if ((eLen + 2) > (sizeof(pWextState->WPARSNIE))) {
				hdd_warn("Cannot accommodate genIE, Need bigger buffer space");
				ret = -EINVAL;
				QDF_ASSERT(0);
				goto exit;
			}
			memset(pWextState->WPARSNIE, 0, MAX_WPA_RSN_IE_LEN);
			memcpy(pWextState->WPARSNIE, genie - 2, (eLen + 2));
			pWextState->roamProfile.pRSNReqIE =
				pWextState->WPARSNIE;
			pWextState->roamProfile.nRSNReqIELength = eLen + 2;
			break;

		default:
			hddLog(LOGE, "%s Set UNKNOWN IE %X", __func__,
			       elementId);
			goto exit;
		}
		genie += eLen;
		remLen -= eLen;
	}
exit:
	EXIT();
	kfree(base_genie);
	return ret;
}

/**
 * iw_set_genie() - SSR wrapper for __iw_set_genie()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_set_genie(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_genie(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_genie() - SIOCGIWGENIE ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_genie(struct net_device *dev,
			  struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	hdd_wext_state_t *pWextState;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	QDF_STATUS status;
	uint32_t length = DOT11F_IE_RSN_MAX_LEN;
	uint8_t genIeBytes[DOT11F_IE_RSN_MAX_LEN];
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	hddLog(LOG1, "getGEN_IE ioctl");

	pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

	if (pHddStaCtx->conn_info.connState == eConnectionState_NotConnected) {
		return -ENXIO;
	}

	/* Return something ONLY if we are associated with an RSN or
	 * WPA network
	 */
	if (!hdd_is_auth_type_rsn(pWextState->roamProfile.negotiatedAuthType)) {
		return -ENXIO;
	}

	/* Actually retrieve the RSN IE from CSR.  (We previously sent
	 * it down in the CSR Roam Profile.)
	 */
	status = csr_roam_get_wpa_rsn_req_ie(WLAN_HDD_GET_HAL_CTX(pAdapter),
					     pAdapter->sessionId,
					     &length, genIeBytes);
	length = QDF_MIN((uint16_t) length, DOT11F_IE_RSN_MAX_LEN);
	if (wrqu->data.length < length) {
		hddLog(LOG1, "%s: failed to copy data to user buffer",
		       __func__);
		return -EFAULT;
	}
	qdf_mem_copy(extra, (void *)genIeBytes, length);
	wrqu->data.length = length;

	hddLog(LOG1, "%s: RSN IE of %d bytes returned", __func__,
	       wrqu->data.length);

	EXIT();

	return 0;
}

/**
 * iw_get_genie() - SSR wrapper for __iw_get_genie()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_genie(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_genie(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_encode() - SIOCGIWENCODE ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_encode(struct net_device *dev,
			   struct iw_request_info *info,
			   struct iw_point *dwrq, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	tCsrRoamProfile *pRoamProfile = &(pWextState->roamProfile);
	int keyId;
	eCsrAuthType authType = eCSR_AUTH_TYPE_NONE;
	int i;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	keyId = pRoamProfile->Keys.defaultIndex;

	if (keyId < 0 || keyId >= MAX_WEP_KEYS) {
		hddLog(LOG1, "%s: Invalid keyId : %d", __func__, keyId);
		return -EINVAL;
	}

	if (pRoamProfile->Keys.KeyLength[keyId] > 0) {
		dwrq->flags |= IW_ENCODE_ENABLED;
		dwrq->length = pRoamProfile->Keys.KeyLength[keyId];
		qdf_mem_copy(extra, &(pRoamProfile->Keys.KeyMaterial[keyId][0]),
			     pRoamProfile->Keys.KeyLength[keyId]);

		dwrq->flags |= (keyId + 1);

	} else {
		dwrq->flags |= IW_ENCODE_DISABLED;
	}

	for (i = 0; i < MAX_WEP_KEYS; i++) {
		if (pRoamProfile->Keys.KeyMaterial[i] == NULL) {
			continue;
		} else {
			break;
		}
	}

	if (MAX_WEP_KEYS == i) {
		dwrq->flags |= IW_ENCODE_NOKEY;
	}

	authType =
		((hdd_station_ctx_t *) WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->
		conn_info.authType;

	if (eCSR_AUTH_TYPE_OPEN_SYSTEM == authType) {
		dwrq->flags |= IW_ENCODE_OPEN;
	} else {
		dwrq->flags |= IW_ENCODE_RESTRICTED;
	}
	EXIT();
	return 0;
}

/**
 * iw_get_encode() - SSR wrapper for __iw_get_encode()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @dwrq: pointer to encoding information
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_encode(struct net_device *dev, struct iw_request_info *info,
			 struct iw_point *dwrq, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_encode(dev, info, dwrq, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_rts_threshold() - SIOCGIWRTS ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_rts_threshold(struct net_device *dev,
				  struct iw_request_info *info,
				  union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	uint32_t status = 0;

	ENTER_DEV(dev);

	status = hdd_wlan_get_rts_threshold(pAdapter, wrqu);

	return status;
}

/**
 * __iw_set_rts_threshold() - SIOCSIWRTS ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_rts_threshold(struct net_device *dev,
				  struct iw_request_info *info,
				  union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (wrqu->rts.value < WNI_CFG_RTS_THRESHOLD_STAMIN
	    || wrqu->rts.value > WNI_CFG_RTS_THRESHOLD_STAMAX) {
		return -EINVAL;
	}

	if (sme_cfg_set_int(hHal, WNI_CFG_RTS_THRESHOLD, wrqu->rts.value) !=
			QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR, FL
				("failed to set ini parameter, WNI_CFG_RTS_THRESHOLD"));
		return -EIO;
	}

	EXIT();

	return 0;
}

/**
 * iw_get_rts_threshold() - SSR wrapper for __iw_get_rts_threshold()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_rts_threshold(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_rts_threshold(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_set_rts_threshold() - SSR wrapper for __iw_set_rts_threshold()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_set_rts_threshold(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_rts_threshold(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_frag_threshold() - SIOCGIWFRAG ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_frag_threshold(struct net_device *dev,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	uint32_t status = 0;

	ENTER_DEV(dev);

	status = hdd_wlan_get_frag_threshold(pAdapter, wrqu);

	return status;
}

/**
 * iw_get_frag_threshold() - SSR wrapper for __iw_get_frag_threshold()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_frag_threshold(struct net_device *dev,
				 struct iw_request_info *info,
				 union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_frag_threshold(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_frag_threshold() - SIOCSIWFRAG ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_frag_threshold(struct net_device *dev,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (wrqu->frag.value < WNI_CFG_FRAGMENTATION_THRESHOLD_STAMIN
	    || wrqu->frag.value > WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX) {
		return -EINVAL;
	}

	if (sme_cfg_set_int
		    (hHal, WNI_CFG_FRAGMENTATION_THRESHOLD, wrqu->frag.value)
		    != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR, FL
				("failed to set ini parameter, WNI_CFG_FRAGMENTATION_THRESHOLD"));
		return -EIO;
	}

	EXIT();

	return 0;
}

/**
 * iw_set_frag_threshold() - SSR wrapper for __iw_set_frag_threshold()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_set_frag_threshold(struct net_device *dev,
				 struct iw_request_info *info,
				 union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_frag_threshold(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_power_mode() - SIOCGIWPOWER ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_power_mode(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *adapter;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	return -EOPNOTSUPP;
}

/**
 * iw_get_power_mode() - SSR wrapper function for __iw_get_power_mode
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: extra
 *
 * Return: 0 on success, error number otherwise
 */
int iw_get_power_mode(struct net_device *dev,
		      struct iw_request_info *info,
		      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_power_mode(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_power_mode() - SIOCSIWPOWER ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_power_mode(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *adapter;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	return -EOPNOTSUPP;
}

/**
 * iw_set_power_mode() - SSR wrapper function for __iw_set_power_mode
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: extra
 *
 * Return: 0 on success, error number otherwise
 */
int iw_set_power_mode(struct net_device *dev,
		      struct iw_request_info *info,
		      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_power_mode(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_range() - SIOCGIWRANGE ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_range(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	struct iw_range *range = (struct iw_range *)extra;

	uint8_t channels[WNI_CFG_VALID_CHANNEL_LIST_LEN];

	uint32_t num_channels = sizeof(channels);
	uint8_t supp_rates[WNI_CFG_SUPPORTED_RATES_11A_LEN];
	uint32_t a_len;
	uint32_t b_len;
	uint32_t active_phy_mode = 0;
	uint8_t index = 0, i;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret =  wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	wrqu->data.length = sizeof(struct iw_range);
	memset(range, 0, sizeof(struct iw_range));


	/*Get the phy mode */
	if (sme_cfg_get_int(hHal,
			    WNI_CFG_DOT11_MODE,
			    &active_phy_mode) == QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  "active_phy_mode = %d", active_phy_mode);

		if (active_phy_mode == WNI_CFG_DOT11_MODE_11A
		    || active_phy_mode == WNI_CFG_DOT11_MODE_11G) {
			/*Get the supported rates for 11G band */
			a_len = WNI_CFG_SUPPORTED_RATES_11A_LEN;
			if (sme_cfg_get_str(hHal,
					    WNI_CFG_SUPPORTED_RATES_11A,
					    supp_rates,
					    &a_len) == QDF_STATUS_SUCCESS) {
				if (a_len > WNI_CFG_SUPPORTED_RATES_11A_LEN) {
					a_len = WNI_CFG_SUPPORTED_RATES_11A_LEN;
				}
				for (i = 0; i < a_len; i++) {
					range->bitrate[i] =
						((supp_rates[i] & 0x7F) / 2) *
						1000000;
				}
				range->num_bitrates = a_len;
			} else {
				return -EIO;
			}
		} else if (active_phy_mode == WNI_CFG_DOT11_MODE_11B) {
			/*Get the supported rates for 11B band */
			b_len = WNI_CFG_SUPPORTED_RATES_11B_LEN;
			if (sme_cfg_get_str(hHal,
					    WNI_CFG_SUPPORTED_RATES_11B,
					    supp_rates,
					    &b_len) == QDF_STATUS_SUCCESS) {
				if (b_len > WNI_CFG_SUPPORTED_RATES_11B_LEN) {
					b_len = WNI_CFG_SUPPORTED_RATES_11B_LEN;
				}
				for (i = 0; i < b_len; i++) {
					range->bitrate[i] =
						((supp_rates[i] & 0x7F) / 2) *
						1000000;
				}
				range->num_bitrates = b_len;
			} else {
				return -EIO;
			}
		}
	}

	range->max_rts = WNI_CFG_RTS_THRESHOLD_STAMAX;
	range->min_frag = WNI_CFG_FRAGMENTATION_THRESHOLD_STAMIN;
	range->max_frag = WNI_CFG_FRAGMENTATION_THRESHOLD_STAMAX;

	range->encoding_size[0] = 5;
	range->encoding_size[1] = 13;
	range->num_encoding_sizes = 2;
	range->max_encoding_tokens = MAX_WEP_KEYS;

	/* we support through Wireless Extensions 22 */
	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 22;

	/*Supported Channels and Frequencies */
	if (sme_cfg_get_str
		    ((hHal), WNI_CFG_VALID_CHANNEL_LIST, channels,
		    &num_channels) != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  FL
				  ("failed to get ini parameter, WNI_CFG_VALID_CHANNEL_LIST"));
		return -EIO;
	}
	if (num_channels > IW_MAX_FREQUENCIES) {
		num_channels = IW_MAX_FREQUENCIES;
	}

	range->num_channels = num_channels;
	range->num_frequency = num_channels;

	for (index = 0; index < num_channels; index++) {
		uint32_t frq_indx = 0;

		range->freq[index].i = channels[index];
		while (frq_indx < FREQ_CHAN_MAP_TABLE_SIZE) {
			if (channels[index] == freq_chan_map[frq_indx].chan) {
				range->freq[index].m =
					freq_chan_map[frq_indx].freq * 100000;
				range->freq[index].e = 1;
				break;
			}
			frq_indx++;
		}
	}

	/* Event capability (kernel + driver) */
	range->event_capa[0] = (IW_EVENT_CAPA_K_0 |
				IW_EVENT_CAPA_MASK(SIOCGIWAP) |
				IW_EVENT_CAPA_MASK(SIOCGIWSCAN));
	range->event_capa[1] = IW_EVENT_CAPA_K_1;

	/*Encryption capability */
	range->enc_capa = IW_ENC_CAPA_WPA | IW_ENC_CAPA_WPA2 |
			  IW_ENC_CAPA_CIPHER_TKIP | IW_ENC_CAPA_CIPHER_CCMP;

	/* Txpower capability */
	range->txpower_capa = IW_TXPOW_MWATT;

	/*Scanning capability */
#if WIRELESS_EXT >= 22
	range->scan_capa =
		IW_SCAN_CAPA_ESSID | IW_SCAN_CAPA_TYPE | IW_SCAN_CAPA_CHANNEL;
#endif

	EXIT();
	return 0;
}

/**
 * iw_get_range() - SSR wrapper for __iw_get_range()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_range(struct net_device *dev, struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_range(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_get_class_a_statistics_cb() - Get Class A stats callback function
 * @pStats: pointer to Class A stats
 * @pContext: user context originally registered with SME
 *
 * Return: None
 */
static void hdd_get_class_a_statistics_cb(void *pStats, void *pContext)
{
	struct statsContext *pStatsContext;
	tCsrGlobalClassAStatsInfo *pClassAStats;
	hdd_adapter_t *pAdapter;

	if (ioctl_debug) {
		pr_info("%s: pStats [%p] pContext [%p]\n",
			__func__, pStats, pContext);
	}

	if ((NULL == pStats) || (NULL == pContext)) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Bad param, pStats [%p] pContext [%p]",
		       __func__, pStats, pContext);
		return;
	}

	pClassAStats = pStats;
	pStatsContext = pContext;
	pAdapter = pStatsContext->pAdapter;

	/* there is a race condition that exists between this callback
	 * function and the caller since the caller could time out
	 * either before or while this code is executing.  we use a
	 * spinlock to serialize these actions
	 */
	spin_lock(&hdd_context_lock);

	if ((NULL == pAdapter) ||
	    (STATS_CONTEXT_MAGIC != pStatsContext->magic)) {
		/* the caller presumably timed out so there is nothing
		 * we can do
		 */
		spin_unlock(&hdd_context_lock);
		hddLog(QDF_TRACE_LEVEL_WARN,
		       "%s: Invalid context, pAdapter [%p] magic [%08x]",
		       __func__, pAdapter, pStatsContext->magic);
		if (ioctl_debug) {
			pr_info("%s: Invalid context, pAdapter [%p] magic [%08x]\n",
				__func__, pAdapter, pStatsContext->magic);
		}
		return;
	}

	/* context is valid so caller is still waiting */

	/* paranoia: invalidate the magic */
	pStatsContext->magic = 0;

	/* copy over the stats. do so as a struct copy */
	pAdapter->hdd_stats.ClassA_stat = *pClassAStats;

	/* notify the caller */
	complete(&pStatsContext->completion);

	/* serialization is complete */
	spin_unlock(&hdd_context_lock);
}

/**
 * wlan_hdd_get_class_astats() - Get Class A statistics
 * @pAdapter: adapter for which statistics are desired
 *
 * Return: QDF_STATUS_SUCCESS if adapter's Class A statistics were updated
 */
QDF_STATUS wlan_hdd_get_class_astats(hdd_adapter_t *pAdapter)
{
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	QDF_STATUS hstatus;
	unsigned long rc;
	struct statsContext context;

	if (NULL == pAdapter) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s: pAdapter is NULL", __func__);
		return QDF_STATUS_E_FAULT;
	}
	if (cds_is_driver_recovering()) {
		hdd_err("Recovery in Progress. State: 0x%x Ignore!!!",
			 cds_get_driver_state());
		return QDF_STATUS_SUCCESS;
	}

	/* we are connected so prepare our callback context */
	init_completion(&context.completion);
	context.pAdapter = pAdapter;
	context.magic = STATS_CONTEXT_MAGIC;
	/* query only for Class A statistics (which include link speed) */
	hstatus = sme_get_statistics(WLAN_HDD_GET_HAL_CTX(pAdapter),
				     eCSR_HDD, SME_GLOBAL_CLASSA_STATS,
				     hdd_get_class_a_statistics_cb,
				     0, /* not periodic */
				     false, /* non-cached results */
				     pHddStaCtx->conn_info.staId[0],
				     &context, pAdapter->sessionId);
	if (QDF_STATUS_SUCCESS != hstatus) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Unable to retrieve Class A statistics", __func__);
		/* we'll returned a cached value below */
	} else {
		/* request was sent -- wait for the response */
		rc = wait_for_completion_timeout
			(&context.completion,
			 msecs_to_jiffies(WLAN_WAIT_TIME_STATS));
		if (!rc) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("SME timed out while retrieving Class A statistics"));
		}
	}

	/* either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.  if
	 * we never sent a request or if we sent a request and got a
	 * response, we want to clear the magic out of paranoia.  if
	 * we timed out there is a race condition such that the
	 * callback function could be executing at the same time we
	 * are. of primary concern is if the callback function had
	 * already verified the "magic" but had not yet set the
	 * completion variable when a timeout occurred. we serialize
	 * these activities by invalidating the magic while holding a
	 * shared spinlock which will cause us to block if the
	 * callback is currently executing
	 */
	spin_lock(&hdd_context_lock);
	context.magic = 0;
	spin_unlock(&hdd_context_lock);

	/* either callback updated pAdapter stats or it has cached data */
	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_get_station_statistics_cb() - Get stats callback function
 * @pStats: pointer to Class A stats
 * @pContext: user context originally registered with SME
 *
 * Return: None
 */
static void hdd_get_station_statistics_cb(void *pStats, void *pContext)
{
	struct statsContext *pStatsContext;
	tCsrSummaryStatsInfo *pSummaryStats;
	tCsrGlobalClassAStatsInfo *pClassAStats;
	hdd_adapter_t *pAdapter;

	if (ioctl_debug) {
		pr_info("%s: pStats [%p] pContext [%p]\n",
			__func__, pStats, pContext);
	}

	if ((NULL == pStats) || (NULL == pContext)) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Bad param, pStats [%p] pContext [%p]",
		       __func__, pStats, pContext);
		return;
	}

	/* there is a race condition that exists between this callback
	 * function and the caller since the caller could time out
	 * either before or while this code is executing.  we use a
	 * spinlock to serialize these actions
	 */
	spin_lock(&hdd_context_lock);

	pSummaryStats = (tCsrSummaryStatsInfo *) pStats;
	pClassAStats = (tCsrGlobalClassAStatsInfo *) (pSummaryStats + 1);
	pStatsContext = pContext;
	pAdapter = pStatsContext->pAdapter;
	if ((NULL == pAdapter) ||
	    (STATS_CONTEXT_MAGIC != pStatsContext->magic)) {
		/* the caller presumably timed out so there is nothing
		 * we can do
		 */
		spin_unlock(&hdd_context_lock);
		hddLog(QDF_TRACE_LEVEL_WARN,
		       "%s: Invalid context, pAdapter [%p] magic [%08x]",
		       __func__, pAdapter, pStatsContext->magic);
		if (ioctl_debug) {
			pr_info("%s: Invalid context, pAdapter [%p] magic [%08x]\n",
				__func__, pAdapter, pStatsContext->magic);
		}
		return;
	}

	/* context is valid so caller is still waiting */

	/* paranoia: invalidate the magic */
	pStatsContext->magic = 0;

	/* copy over the stats. do so as a struct copy */
	pAdapter->hdd_stats.summary_stat = *pSummaryStats;
	pAdapter->hdd_stats.ClassA_stat = *pClassAStats;

	/* notify the caller */
	complete(&pStatsContext->completion);

	/* serialization is complete */
	spin_unlock(&hdd_context_lock);
}

/**
 * wlan_hdd_get_station_stats() - Get station statistics
 * @pAdapter: adapter for which statistics are desired
 *
 * Return: QDF_STATUS_SUCCESS if adapter's statistics were updated
 */
QDF_STATUS wlan_hdd_get_station_stats(hdd_adapter_t *pAdapter)
{
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	QDF_STATUS hstatus;
	unsigned long rc;
	struct statsContext context;

	if (NULL == pAdapter) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s: pAdapter is NULL", __func__);
		return QDF_STATUS_SUCCESS;
	}

	/* we are connected so prepare our callback context */
	init_completion(&context.completion);
	context.pAdapter = pAdapter;
	context.magic = STATS_CONTEXT_MAGIC;

	/* query only for Summary & Class A statistics */
	hstatus = sme_get_statistics(WLAN_HDD_GET_HAL_CTX(pAdapter),
				     eCSR_HDD,
				     SME_SUMMARY_STATS |
					SME_GLOBAL_CLASSA_STATS,
				     hdd_get_station_statistics_cb,
				     0, /* not periodic */
				     false, /* non-cached results */
				     pHddStaCtx->conn_info.staId[0],
				     &context, pAdapter->sessionId);
	if (QDF_STATUS_SUCCESS != hstatus) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Unable to retrieve statistics", __func__);
		/* we'll return with cached values */
	} else {
		/* request was sent -- wait for the response */
		rc = wait_for_completion_timeout
			(&context.completion,
			 msecs_to_jiffies(WLAN_WAIT_TIME_STATS));

		if (!rc) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL("SME timed out while retrieving statistics"));
		}
	}

	/* either we never sent a request, we sent a request and
	 * received a response or we sent a request and timed out.  if
	 * we never sent a request or if we sent a request and got a
	 * response, we want to clear the magic out of paranoia.  if
	 * we timed out there is a race condition such that the
	 * callback function could be executing at the same time we
	 * are. of primary concern is if the callback function had
	 * already verified the "magic" but had not yet set the
	 * completion variable when a timeout occurred. we serialize
	 * these activities by invalidating the magic while holding a
	 * shared spinlock which will cause us to block if the
	 * callback is currently executing
	 */
	spin_lock(&hdd_context_lock);
	context.magic = 0;
	spin_unlock(&hdd_context_lock);

	/* either callback updated pAdapter stats or it has cached data */
	return QDF_STATUS_SUCCESS;
}

/**
 * iw_get_linkspeed() - Get current link speed ioctl
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: extra ioctl buffer
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_linkspeed(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	char *pLinkSpeed = (char *)extra;
	int len = sizeof(uint32_t) + 1;
	uint32_t link_speed = 0;
	hdd_context_t *hdd_ctx;
	int rc, valid;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	valid = wlan_hdd_validate_context(hdd_ctx);
	if (0 != valid)
		return valid;

	rc = wlan_hdd_get_link_speed(pAdapter, &link_speed);
	if (0 != rc) {
		return rc;
	}

	wrqu->data.length = len;
	/* return the linkspeed as a string */
	rc = snprintf(pLinkSpeed, len, "%u", link_speed);
	if ((rc < 0) || (rc >= len)) {
		/* encoding or length error? */
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("Unable to encode link speed"));
		return -EIO;
	}

	EXIT();
	/* a value is being successfully returned */
	return 0;
}

static int iw_get_linkspeed(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_linkspeed(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_change_country_code_callback() - Change country code callback
 * @context: opaque context originally passed to SME.  All functions
 * which use this callback pass the adapter upon which the country
 * code change is active
 *
 * This function is registered as the callback function when
 * sme_change_country_code() is invoked.  Callers of
 * sme_change_country_code() subsequently wait for the adapter's
 * @change_country_code completion variable, so all this function
 * needs to do is set that completion variable so that execution can
 * continue.
 *
 * Return: none
 */
void wlan_hdd_change_country_code_callback(void *context)
{

	hdd_adapter_t *adapter = context;

	if (adapter && (WLAN_HDD_ADAPTER_MAGIC == adapter->magic))
		complete(&adapter->change_country_code);

	return;
}

/**
 * __iw_set_nick() - SIOCSIWNICKN ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_nick(struct net_device *dev,
		       struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *adapter;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	return 0;
}

/**
 * iw_set_nick() - SSR wrapper for __iw_set_nick
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: extra
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_set_nick(struct net_device *dev,
		       struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_nick(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_nick() - SIOCGIWNICKN ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_nick(struct net_device *dev,
		       struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *adapter;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	return 0;
}

/**
 * iw_get_nick() - SSR wrapper for __iw_get_nick
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: extra
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_nick(struct net_device *dev,
		       struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_nick(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_encode() - SIOCSIWENCODE ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_encode(struct net_device *dev, struct iw_request_info *info,
			   union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	hdd_context_t *hdd_ctx;
	struct iw_point *encoderq = &(wrqu->encoding);
	uint32_t keyId;
	uint8_t key_length;
	eCsrEncryptionType encryptionType = eCSR_ENCRYPT_TYPE_NONE;
	bool fKeyPresent = 0;
	int i;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	keyId = encoderq->flags & IW_ENCODE_INDEX;

	if (keyId) {
		if (keyId > MAX_WEP_KEYS) {
			return -EINVAL;
		}

		fKeyPresent = 1;
		keyId--;
	} else {
		fKeyPresent = 0;
	}

	if (wrqu->data.flags & IW_ENCODE_DISABLED) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  "****iwconfig wlan0 key off*****");
		if (!fKeyPresent) {

			for (i = 0; i < CSR_MAX_NUM_KEY; i++) {

				if (pWextState->roamProfile.Keys.KeyMaterial[i])
					pWextState->roamProfile.Keys.
					KeyLength[i] = 0;
			}
		}
		pHddStaCtx->conn_info.authType = eCSR_AUTH_TYPE_OPEN_SYSTEM;
		pWextState->wpaVersion = IW_AUTH_WPA_VERSION_DISABLED;
		pWextState->roamProfile.EncryptionType.encryptionType[0] =
			eCSR_ENCRYPT_TYPE_NONE;
		pWextState->roamProfile.mcEncryptionType.encryptionType[0] =
			eCSR_ENCRYPT_TYPE_NONE;

		pHddStaCtx->conn_info.ucEncryptionType = eCSR_ENCRYPT_TYPE_NONE;
		pHddStaCtx->conn_info.mcEncryptionType = eCSR_ENCRYPT_TYPE_NONE;

		if (eConnectionState_Associated ==
		    pHddStaCtx->conn_info.connState) {
			INIT_COMPLETION(pAdapter->disconnect_comp_var);
			status =
				sme_roam_disconnect(WLAN_HDD_GET_HAL_CTX(pAdapter),
						    pAdapter->sessionId,
						    eCSR_DISCONNECT_REASON_UNSPECIFIED);
			if (QDF_STATUS_SUCCESS == status) {
				unsigned long rc;
				rc = wait_for_completion_timeout(&pAdapter->
								 disconnect_comp_var,
								 msecs_to_jiffies
									 (WLAN_WAIT_TIME_DISCONNECT));
				if (!rc)
					hddLog(QDF_TRACE_LEVEL_ERROR,
					       FL
						       ("failed wait on disconnect_comp_var"));
			}
		}

		return status;

	}

	if (wrqu->data.flags & (IW_ENCODE_OPEN | IW_ENCODE_RESTRICTED)) {
		hddLog(QDF_TRACE_LEVEL_INFO, "iwconfig wlan0 key on");

		pHddStaCtx->conn_info.authType =
			(encoderq->
			 flags & IW_ENCODE_RESTRICTED) ? eCSR_AUTH_TYPE_SHARED_KEY :
			eCSR_AUTH_TYPE_OPEN_SYSTEM;

	}

	if (wrqu->data.length > 0) {
		hddLog(QDF_TRACE_LEVEL_INFO, "%s : wrqu->data.length : %d",
		       __func__, wrqu->data.length);

		key_length = wrqu->data.length;

		/* IW_ENCODING_TOKEN_MAX is the value that is set for wrqu->data.length by iwconfig.c when 'iwconfig wlan0 key on' is issued. */

		if (5 == key_length) {
			hddLog(QDF_TRACE_LEVEL_INFO,
			       "%s: Call with WEP40,key_len=%d", __func__,
			       key_length);

			if ((IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt)
			    && (eCSR_AUTH_TYPE_OPEN_SYSTEM ==
				pHddStaCtx->conn_info.authType)) {
				encryptionType = eCSR_ENCRYPT_TYPE_WEP40;
			} else {
				encryptionType =
					eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
			}
		} else if (13 == key_length) {
			hddLog(QDF_TRACE_LEVEL_INFO,
			       "%s:Call with WEP104,key_len:%d", __func__,
			       key_length);

			if ((IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt)
			    && (eCSR_AUTH_TYPE_OPEN_SYSTEM ==
				pHddStaCtx->conn_info.authType)) {
				encryptionType = eCSR_ENCRYPT_TYPE_WEP104;
			} else {
				encryptionType =
					eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
			}
		} else {
			hddLog(QDF_TRACE_LEVEL_WARN,
			       "%s: Invalid WEP key length :%d", __func__,
			       key_length);
			return -EINVAL;
		}

		pHddStaCtx->conn_info.ucEncryptionType = encryptionType;
		pHddStaCtx->conn_info.mcEncryptionType = encryptionType;
		pWextState->roamProfile.EncryptionType.numEntries = 1;
		pWextState->roamProfile.EncryptionType.encryptionType[0] =
			encryptionType;
		pWextState->roamProfile.mcEncryptionType.numEntries = 1;
		pWextState->roamProfile.mcEncryptionType.encryptionType[0] =
			encryptionType;

		if ((eConnectionState_NotConnected ==
		     pHddStaCtx->conn_info.connState)
		    &&
		    ((eCSR_AUTH_TYPE_OPEN_SYSTEM ==
		      pHddStaCtx->conn_info.authType)
		     || (eCSR_AUTH_TYPE_SHARED_KEY ==
			 pHddStaCtx->conn_info.authType))) {

			qdf_mem_copy(&pWextState->roamProfile.Keys.
				     KeyMaterial[keyId][0], extra, key_length);

			pWextState->roamProfile.Keys.KeyLength[keyId] =
				(uint8_t) key_length;
			pWextState->roamProfile.Keys.defaultIndex =
				(uint8_t) keyId;

			return status;
		}
	}

	return 0;
}

/**
 * iw_set_encode() - SSR wrapper for __iw_set_encode()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_set_encode(struct net_device *dev, struct iw_request_info *info,
			 union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_encode(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_encodeext() - SIOCGIWENCODEEXT ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_encodeext(struct net_device *dev,
			      struct iw_request_info *info,
			      struct iw_point *dwrq, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	tCsrRoamProfile *pRoamProfile = &(pWextState->roamProfile);
	int keyId;
	eCsrEncryptionType encryptionType = eCSR_ENCRYPT_TYPE_NONE;
	eCsrAuthType authType = eCSR_AUTH_TYPE_NONE;
	int i, ret;
	hdd_context_t *hdd_ctx;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	keyId = pRoamProfile->Keys.defaultIndex;

	if (keyId < 0 || keyId >= MAX_WEP_KEYS) {
		hddLog(LOG1, "%s: Invalid keyId : %d", __func__, keyId);
		return -EINVAL;
	}

	if (pRoamProfile->Keys.KeyLength[keyId] > 0) {
		dwrq->flags |= IW_ENCODE_ENABLED;
		dwrq->length = pRoamProfile->Keys.KeyLength[keyId];
		qdf_mem_copy(extra, &(pRoamProfile->Keys.KeyMaterial[keyId][0]),
			     pRoamProfile->Keys.KeyLength[keyId]);
	} else {
		dwrq->flags |= IW_ENCODE_DISABLED;
	}

	for (i = 0; i < MAX_WEP_KEYS; i++) {
		if (pRoamProfile->Keys.KeyMaterial[i] == NULL) {
			continue;
		} else {
			break;
		}
	}

	if (MAX_WEP_KEYS == i) {
		dwrq->flags |= IW_ENCODE_NOKEY;
	} else {
		dwrq->flags |= IW_ENCODE_ENABLED;
	}

	encryptionType = pRoamProfile->EncryptionType.encryptionType[0];

	if (eCSR_ENCRYPT_TYPE_NONE == encryptionType) {
		dwrq->flags |= IW_ENCODE_DISABLED;
	}

	authType = (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.authType;

	if (IW_AUTH_ALG_OPEN_SYSTEM == authType) {
		dwrq->flags |= IW_ENCODE_OPEN;
	} else {
		dwrq->flags |= IW_ENCODE_RESTRICTED;
	}
	EXIT();
	return 0;

}

/**
 * iw_get_encodeext() - SSR wrapper for __iw_get_encodeext()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @dwrq: pointer to encoding information
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_encodeext(struct net_device *dev,
			    struct iw_request_info *info,
			    struct iw_point *dwrq, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_encodeext(dev, info, dwrq, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_encodeext() - SIOCSIWENCODEEXT ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_encodeext(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	hdd_context_t *hdd_ctx;
	QDF_STATUS qdf_ret_status = QDF_STATUS_SUCCESS;
	tCsrRoamProfile *pRoamProfile = &pWextState->roamProfile;
	int ret;
	struct iw_encode_ext *ext = (struct iw_encode_ext *)extra;
	int key_index;
	struct iw_point *encoding = &wrqu->encoding;
	tCsrRoamSetKey setKey;
	uint32_t roamId = 0xFF;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	key_index = encoding->flags & IW_ENCODE_INDEX;

	if (key_index > 0) {

		/*Convert from 1-based to 0-based keying */
		key_index--;
	}
	if (!ext->key_len) {

		/*Set the encrytion type to NONE */
		pRoamProfile->EncryptionType.encryptionType[0] =
			eCSR_ENCRYPT_TYPE_NONE;
		return ret;
	}

	if (eConnectionState_NotConnected == pHddStaCtx->conn_info.connState &&
	    (IW_ENCODE_ALG_WEP == ext->alg)) {
		if (IW_AUTH_KEY_MGMT_802_1X == pWextState->authKeyMgmt) {

			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				  ("Invalid Configuration:%s"), __func__);
			return -EINVAL;
		} else {
			/*Static wep, update the roam profile with the keys */
			if (ext->key
			    && (ext->key_len <=
				eCSR_SECURITY_WEP_KEYSIZE_MAX_BYTES)
			    && key_index < CSR_MAX_NUM_KEY) {
				qdf_mem_copy(&pRoamProfile->Keys.
					     KeyMaterial[key_index][0],
					     ext->key, ext->key_len);
				pRoamProfile->Keys.KeyLength[key_index] =
					(uint8_t) ext->key_len;

				if (ext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY)
					pRoamProfile->Keys.defaultIndex =
						(uint8_t) key_index;

			}
		}
		return ret;
	}

	qdf_mem_zero(&setKey, sizeof(tCsrRoamSetKey));

	setKey.keyId = key_index;
	setKey.keyLength = ext->key_len;

	if (ext->key_len <= CSR_MAX_KEY_LEN) {
		qdf_mem_copy(&setKey.Key[0], ext->key, ext->key_len);
	}

	if (ext->ext_flags & IW_ENCODE_EXT_GROUP_KEY) {
		/*Key direction for group is RX only */
		setKey.keyDirection = eSIR_RX_ONLY;
		qdf_set_macaddr_broadcast(&setKey.peerMac);
	} else {

		setKey.keyDirection = eSIR_TX_RX;
		qdf_mem_copy(setKey.peerMac.bytes, ext->addr.sa_data,
			     QDF_MAC_ADDR_SIZE);
	}

	/*For supplicant pae role is zero */
	setKey.paeRole = 0;

	switch (ext->alg) {
	case IW_ENCODE_ALG_NONE:
		setKey.encType = eCSR_ENCRYPT_TYPE_NONE;
		break;

	case IW_ENCODE_ALG_WEP:
		setKey.encType =
			(ext->key_len ==
			 5) ? eCSR_ENCRYPT_TYPE_WEP40 : eCSR_ENCRYPT_TYPE_WEP104;
		break;

	case IW_ENCODE_ALG_TKIP:
	{
		uint8_t *pKey = &setKey.Key[0];

		setKey.encType = eCSR_ENCRYPT_TYPE_TKIP;

		qdf_mem_zero(pKey, CSR_MAX_KEY_LEN);

		/* Supplicant sends the 32bytes key in this order
		 * |--------------|----------|----------|
		 * |   Tk1        | TX MIC   |  RX MIC  |
		 * |--------------|----------|----------|
		 * <---16bytes---><--8bytes--><--8bytes-->
		 *
		 *
		 * Sme expects the 32 bytes key to be in the below order
		 * |--------------|----------|----------|
		 * |   Tk1        | RX MIC   |  TX MIC  |
		 * |--------------|----------|----------|
		 * <---16bytes---><--8bytes--><--8bytes-->
		 */

		/* Copy the Temporal Key 1 (TK1) */
		qdf_mem_copy(pKey, ext->key, 16);

		/* Copy the rx mic first */
		qdf_mem_copy(&pKey[16], &ext->key[24], 8);

		/* Copy the tx mic */
		qdf_mem_copy(&pKey[24], &ext->key[16], 8);

	}
	break;

	case IW_ENCODE_ALG_CCMP:
		setKey.encType = eCSR_ENCRYPT_TYPE_AES;
		break;

#ifdef FEATURE_WLAN_ESE
#define IW_ENCODE_ALG_KRK 6
	case IW_ENCODE_ALG_KRK:
		setKey.encType = eCSR_ENCRYPT_TYPE_KRK;
		break;
#endif /* FEATURE_WLAN_ESE */

	default:
		setKey.encType = eCSR_ENCRYPT_TYPE_NONE;
		break;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  ("%s:cipher_alg:%d key_len[%d] *pEncryptionType :%d"),
		  __func__, (int)ext->alg, (int)ext->key_len, setKey.encType);

	/* The supplicant may attempt to set the PTK once
	 * pre-authentication is done. Save the key in the UMAC and
	 * include it in the ADD BSS request
	 */
	qdf_ret_status = sme_ft_update_key(WLAN_HDD_GET_HAL_CTX(pAdapter),
					   pAdapter->sessionId, &setKey);
	if (qdf_ret_status == QDF_STATUS_FT_PREAUTH_KEY_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_INFO_MED,
		       "%s: Update PreAuth Key success", __func__);
		return 0;
	} else if (qdf_ret_status == QDF_STATUS_FT_PREAUTH_KEY_FAILED) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Update PreAuth Key failed", __func__);
		return -EINVAL;
	}

	pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_SETTING_KEY;

	qdf_ret_status = sme_roam_set_key(WLAN_HDD_GET_HAL_CTX(pAdapter),
					  pAdapter->sessionId,
					  &setKey, &roamId);

	if (qdf_ret_status != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "[%4d] sme_roam_set_key returned ERROR status= %d",
			  __LINE__, qdf_ret_status);

		pHddStaCtx->roam_info.roamingState = HDD_ROAM_STATE_NONE;
	}

	return qdf_ret_status;
}

/**
 * iw_set_encodeext() - SSR wrapper for __iw_set_encodeext()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_set_encodeext(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_encodeext(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_retry() - SIOCSIWRETRY ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_retry(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (wrqu->retry.value < WNI_CFG_LONG_RETRY_LIMIT_STAMIN ||
	    wrqu->retry.value > WNI_CFG_LONG_RETRY_LIMIT_STAMAX) {

		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  ("Invalid Retry-Limit=%d!!"), wrqu->retry.value);

		return -EINVAL;
	}

	if (wrqu->retry.flags & IW_RETRY_LIMIT) {

		if ((wrqu->retry.flags & IW_RETRY_LONG)) {
			if (sme_cfg_set_int (hHal, WNI_CFG_LONG_RETRY_LIMIT,
						wrqu->retry.value) !=
					QDF_STATUS_SUCCESS) {
				QDF_TRACE(QDF_MODULE_ID_HDD,
					  QDF_TRACE_LEVEL_ERROR, FL
					  ("failed to set ini parameter, WNI_CFG_LONG_RETRY_LIMIT"));
				return -EIO;
			}
		} else if ((wrqu->retry.flags & IW_RETRY_SHORT)) {
			if (sme_cfg_set_int (hHal, WNI_CFG_SHORT_RETRY_LIMIT,
						wrqu->retry.value) !=
					QDF_STATUS_SUCCESS) {
				QDF_TRACE(QDF_MODULE_ID_HDD,
					QDF_TRACE_LEVEL_ERROR, FL
					("failed to set ini parameter, WNI_CFG_LONG_RETRY_LIMIT"));
				return -EIO;
			}
		}
	} else {
		return -EOPNOTSUPP;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  ("Set Retry-Limit=%d!!"), wrqu->retry.value);

	EXIT();

	return 0;

}

/**
 * iw_set_retry() - SSR wrapper for __iw_set_retry()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_set_retry(struct net_device *dev, struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_retry(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_get_retry() - SIOCGIWRETRY ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_retry(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	uint32_t retry = 0;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if ((wrqu->retry.flags & IW_RETRY_LONG)) {
		wrqu->retry.flags = IW_RETRY_LIMIT | IW_RETRY_LONG;

		if (sme_cfg_get_int(hHal, WNI_CFG_LONG_RETRY_LIMIT, &retry) !=
		    QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
				  FL
					  ("failed to get ini parameter, WNI_CFG_LONG_RETRY_LIMIT"));
			return -EIO;
		}

		wrqu->retry.value = retry;
	} else if ((wrqu->retry.flags & IW_RETRY_SHORT)) {
		wrqu->retry.flags = IW_RETRY_LIMIT | IW_RETRY_SHORT;

		if (sme_cfg_get_int(hHal, WNI_CFG_SHORT_RETRY_LIMIT, &retry) !=
		    QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
				  FL
					  ("failed to get ini parameter, WNI_CFG_LONG_RETRY_LIMIT"));
			return -EIO;
		}

		wrqu->retry.value = retry;
	} else {
		return -EOPNOTSUPP;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO, ("Retry-Limit=%d!!"),
		  retry);

	EXIT();

	return 0;
}

/**
 * iw_get_retry() - SSR wrapper for __iw_get_retry()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_get_retry(struct net_device *dev, struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_retry(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_mlme() - SIOCSIWMLME ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_mlme(struct net_device *dev,
			 struct iw_request_info *info,
			 union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	struct iw_mlme *mlme = (struct iw_mlme *)extra;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/* reason_code is unused. By default it is set to
	 * eCSR_DISCONNECT_REASON_UNSPECIFIED
	 */
	switch (mlme->cmd) {
	case IW_MLME_DISASSOC:
	case IW_MLME_DEAUTH:

		if (pHddStaCtx->conn_info.connState ==
		    eConnectionState_Associated) {
			eCsrRoamDisconnectReason reason =
				eCSR_DISCONNECT_REASON_UNSPECIFIED;

			if (mlme->reason_code == HDD_REASON_MICHAEL_MIC_FAILURE)
				reason = eCSR_DISCONNECT_REASON_MIC_ERROR;

			INIT_COMPLETION(pAdapter->disconnect_comp_var);
			status =
				sme_roam_disconnect(WLAN_HDD_GET_HAL_CTX(pAdapter),
						    pAdapter->sessionId, reason);

			if (QDF_STATUS_SUCCESS == status) {
				unsigned long rc;
				rc = wait_for_completion_timeout(&pAdapter->
								 disconnect_comp_var,
								 msecs_to_jiffies
									 (WLAN_WAIT_TIME_DISCONNECT));
				if (!rc)
					hddLog(QDF_TRACE_LEVEL_ERROR,
					       FL
						       ("failed wait on disconnect_comp_var"));
			} else
				hddLog(LOGE,
				       "%s %d Command Disassociate/Deauthenticate : csr_roam_disconnect failure returned %d",
				       __func__, (int)mlme->cmd, (int)status);

			/* Resetting authKeyMgmt */
			(WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter))->authKeyMgmt =
				0;

			hddLog(LOG1, FL("Disabling queues"));
			wlan_hdd_netif_queue_control(pAdapter,
					WLAN_NETIF_TX_DISABLE_N_CARRIER,
					WLAN_CONTROL_PATH);

		} else {
			hddLog(LOGE,
			       "%s %d Command Disassociate/Deauthenticate called but station is not in associated state",
			       __func__, (int)mlme->cmd);
		}
		break;
	default:
		hddLog(LOGE,
		       "%s %d Command should be Disassociate/Deauthenticate",
		       __func__, (int)mlme->cmd);
		return -EINVAL;
	} /* end of switch */

	EXIT();

	return status;

}

/**
 * iw_set_mlme() - SSR wrapper for __iw_set_mlme()
 * @dev: pointer to net_device
 * @info: pointer to iw_request_info
 * @wrqu: pointer to iwreq_data
 * @extra: pointer to extra ioctl payload
 *
 * Return: 0 on success, error number otherwise
 */
static int iw_set_mlme(struct net_device *dev, struct iw_request_info *info,
		       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_mlme(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_update_phymode() - handle change in PHY mode
 * @net: device upon which PHY mode change was received
 * @hal: umac handle for the driver
 * @new_phymode: new PHY mode for the device
 * @phddctx: pointer to the HDD context
 *
 * This function is called when the device is set to a new PHY mode.
 * It takes a holistic look at the desired PHY mode along with the
 * configured capabilities of the driver and the reported capabilities
 * of the hardware in order to correctly configure all PHY-related
 * parameters.
 *
 * Return: 0 on success, negative errno value on error
 */
int wlan_hdd_update_phymode(struct net_device *net, tHalHandle hal,
			    int new_phymode, hdd_context_t *phddctx)
{
#ifdef QCA_HT_2040_COEX
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(net);
	QDF_STATUS halStatus = QDF_STATUS_E_FAILURE;
#endif
	bool band_24 = false, band_5g = false;
	bool ch_bond24 = false, ch_bond5g = false;
	tSmeConfigParams smeconfig;
	uint32_t chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
	uint32_t vhtchanwidth;
	eCsrPhyMode phymode = -EIO, old_phymode;
	eHddDot11Mode hdd_dot11mode = phddctx->config->dot11Mode;
	eCsrBand curr_band = eCSR_BAND_ALL;

	old_phymode = sme_get_phy_mode(hal);

	if (WNI_CFG_CHANNEL_BONDING_MODE_DISABLE !=
	    sme_get_cb_phy_state_from_cb_ini_value(phddctx->config->
						   nChannelBondingMode24GHz))
		ch_bond24 = true;

	if (WNI_CFG_CHANNEL_BONDING_MODE_DISABLE !=
	    sme_get_cb_phy_state_from_cb_ini_value(phddctx->config->
						   nChannelBondingMode5GHz))
		ch_bond5g = true;

	if (phddctx->config->nBandCapability == eCSR_BAND_ALL) {
		band_24 = band_5g = true;
	} else if (phddctx->config->nBandCapability == eCSR_BAND_24) {
		band_24 = true;
	} else if (phddctx->config->nBandCapability == eCSR_BAND_5G) {
		band_5g = true;
	}

	vhtchanwidth = phddctx->config->vhtChannelWidth;
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN, ("ch_bond24=%d "
		"ch_bond5g=%d band_24=%d band_5g=%d VHT_ch_width=%u"),
		ch_bond24, ch_bond5g, band_24, band_5g, vhtchanwidth);

	switch (new_phymode) {
	case IEEE80211_MODE_AUTO:
		sme_set_phy_mode(hal, eCSR_DOT11_MODE_AUTO);
		if (hdd_set_band(net, WLAN_HDD_UI_BAND_AUTO) == 0) {
			phymode = eCSR_DOT11_MODE_AUTO;
			hdd_dot11mode = eHDD_DOT11_MODE_AUTO;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			curr_band = eCSR_BAND_ALL;
			vhtchanwidth = eHT_CHANNEL_WIDTH_80MHZ;
		} else {
			sme_set_phy_mode(hal, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11A:
		sme_set_phy_mode(hal, eCSR_DOT11_MODE_11a);
		if ((hdd_set_band(net, WLAN_HDD_UI_BAND_5_GHZ) == 0)) {
			phymode = eCSR_DOT11_MODE_11a;
			hdd_dot11mode = eHDD_DOT11_MODE_11a;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
			curr_band = eCSR_BAND_5G;
		} else {
			sme_set_phy_mode(hal, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11B:
		sme_set_phy_mode(hal, eCSR_DOT11_MODE_11b);
		if ((hdd_set_band(net, WLAN_HDD_UI_BAND_2_4_GHZ) == 0)) {
			phymode = eCSR_DOT11_MODE_11b;
			hdd_dot11mode = eHDD_DOT11_MODE_11b;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
			curr_band = eCSR_BAND_24;
		} else {
			sme_set_phy_mode(hal, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11G:
		sme_set_phy_mode(hal, eCSR_DOT11_MODE_11g);
		if ((hdd_set_band(net, WLAN_HDD_UI_BAND_2_4_GHZ) == 0)) {
			phymode = eCSR_DOT11_MODE_11g;
			hdd_dot11mode = eHDD_DOT11_MODE_11g;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
			curr_band = eCSR_BAND_24;
		} else {
			sme_set_phy_mode(hal, old_phymode);
			return -EIO;
		}
		break;
	/* UMAC doesnt have option to set MODE_11NA/MODE_11NG as phymode
	 * so setting phymode as eCSR_DOT11_MODE_11n and updating the band
	 * and channel bonding in configuration to reflect MODE_11NA/MODE_11NG
	 */
	case IEEE80211_MODE_11NA_HT20:
		sme_set_phy_mode(hal, eCSR_DOT11_MODE_11n);
		if ((hdd_set_band(net, WLAN_HDD_UI_BAND_5_GHZ) == 0)) {
			phymode = eCSR_DOT11_MODE_11n;
			hdd_dot11mode = eHDD_DOT11_MODE_11n;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
			curr_band = eCSR_BAND_5G;
		} else {
			sme_set_phy_mode(hal, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11NA_HT40:
		sme_set_phy_mode(hal, eCSR_DOT11_MODE_11n);
		if ((hdd_set_band(net, WLAN_HDD_UI_BAND_5_GHZ) == 0)) {
			phymode = eCSR_DOT11_MODE_11n;
			hdd_dot11mode = eHDD_DOT11_MODE_11n;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			curr_band = eCSR_BAND_5G;
		} else {
			sme_set_phy_mode(hal, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11NG_HT20:
		sme_set_phy_mode(hal, eCSR_DOT11_MODE_11n);
		if ((hdd_set_band(net, WLAN_HDD_UI_BAND_2_4_GHZ) == 0)) {
			phymode = eCSR_DOT11_MODE_11n;
			hdd_dot11mode = eHDD_DOT11_MODE_11n;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
			curr_band = eCSR_BAND_24;
		} else {
			sme_set_phy_mode(hal, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11NG_HT40:
		sme_set_phy_mode(hal, eCSR_DOT11_MODE_11n);
		if ((hdd_set_band(net, WLAN_HDD_UI_BAND_2_4_GHZ) == 0)) {
			phymode = eCSR_DOT11_MODE_11n;
			hdd_dot11mode = eHDD_DOT11_MODE_11n;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			curr_band = eCSR_BAND_24;
		} else {
			sme_set_phy_mode(hal, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11AC_VHT20:
	case IEEE80211_MODE_11AC_VHT40:
	case IEEE80211_MODE_11AC_VHT80:
		sme_set_phy_mode(hal, eCSR_DOT11_MODE_11ac);
		if ((hdd_set_band(net, WLAN_HDD_UI_BAND_5_GHZ) == 0)) {
			phymode = eCSR_DOT11_MODE_11ac;
			hdd_dot11mode = eHDD_DOT11_MODE_11ac;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			curr_band = eCSR_BAND_5G;
		} else {
			sme_set_phy_mode(hal, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_2G_AUTO:
		sme_set_phy_mode(hal, eCSR_DOT11_MODE_AUTO);
		if ((hdd_set_band(net, WLAN_HDD_UI_BAND_2_4_GHZ) == 0)) {
			phymode = eCSR_DOT11_MODE_AUTO;
			hdd_dot11mode = eHDD_DOT11_MODE_AUTO;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			curr_band = eCSR_BAND_24;
		} else {
			sme_set_phy_mode(hal, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_5G_AUTO:
		sme_set_phy_mode(hal, eCSR_DOT11_MODE_AUTO);
		if ((hdd_set_band(net, WLAN_HDD_UI_BAND_5_GHZ) == 0)) {
			phymode = eCSR_DOT11_MODE_AUTO;
			hdd_dot11mode = eHDD_DOT11_MODE_AUTO;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			vhtchanwidth = eHT_CHANNEL_WIDTH_80MHZ;
			curr_band = eCSR_BAND_5G;
		} else {
			sme_set_phy_mode(hal, old_phymode);
			return -EIO;
		}
		break;
	case IEEE80211_MODE_11AGN:
		sme_set_phy_mode(hal, eCSR_DOT11_MODE_11n);
		if ((hdd_set_band(net, WLAN_HDD_UI_BAND_AUTO) == 0)) {
			phymode = eCSR_DOT11_MODE_11n;
			hdd_dot11mode = eHDD_DOT11_MODE_11n;
			chwidth = WNI_CFG_CHANNEL_BONDING_MODE_ENABLE;
			curr_band = eCSR_BAND_ALL;
		} else {
			sme_set_phy_mode(hal, old_phymode);
			return -EIO;
		}
		break;
	default:
		return -EIO;
	}

	switch (new_phymode) {
	case IEEE80211_MODE_11AC_VHT20:
		chwidth = WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
		vhtchanwidth = eHT_CHANNEL_WIDTH_20MHZ;
		break;
	case IEEE80211_MODE_11AC_VHT40:
		vhtchanwidth = eHT_CHANNEL_WIDTH_40MHZ;
		break;
	case IEEE80211_MODE_11AC_VHT80:
		vhtchanwidth = eHT_CHANNEL_WIDTH_80MHZ;
		break;
	default:
		vhtchanwidth = phddctx->config->vhtChannelWidth;
		break;
	}

	if (phymode != -EIO) {
		sme_get_config_param(hal, &smeconfig);
		smeconfig.csrConfig.phyMode = phymode;
#ifdef QCA_HT_2040_COEX
		if (phymode == eCSR_DOT11_MODE_11n &&
		    chwidth == WNI_CFG_CHANNEL_BONDING_MODE_DISABLE) {
			smeconfig.csrConfig.obssEnabled = false;
			halStatus = sme_set_ht2040_mode(hal,
							pAdapter->sessionId,
							eHT_CHAN_HT20, false);
			if (halStatus == QDF_STATUS_E_FAILURE) {
				hddLog(LOGE, FL("Failed to disable OBSS"));
				return -EIO;
			}
		} else if (phymode == eCSR_DOT11_MODE_11n &&
			   chwidth == WNI_CFG_CHANNEL_BONDING_MODE_ENABLE) {
			smeconfig.csrConfig.obssEnabled = true;
			halStatus = sme_set_ht2040_mode(hal,
							pAdapter->sessionId,
							eHT_CHAN_HT20, true);
			if (halStatus == QDF_STATUS_E_FAILURE) {
				hddLog(LOGE, FL("Failed to enable OBSS"));
				return -EIO;
			}
		}
#endif
		smeconfig.csrConfig.eBand = curr_band;
		smeconfig.csrConfig.bandCapability = curr_band;
		if (curr_band == eCSR_BAND_24)
			smeconfig.csrConfig.Is11hSupportEnabled = 0;
		else
			smeconfig.csrConfig.Is11hSupportEnabled =
				phddctx->config->Is11hSupportEnabled;
		if (curr_band == eCSR_BAND_24)
			smeconfig.csrConfig.channelBondingMode24GHz = chwidth;
		else if (curr_band == eCSR_BAND_24)
			smeconfig.csrConfig.channelBondingMode5GHz = chwidth;
		else {
			smeconfig.csrConfig.channelBondingMode24GHz = chwidth;
			smeconfig.csrConfig.channelBondingMode5GHz = chwidth;
		}
		smeconfig.csrConfig.nVhtChannelWidth = vhtchanwidth;
		sme_update_config(hal, &smeconfig);

		phddctx->config->dot11Mode = hdd_dot11mode;
		phddctx->config->nBandCapability = curr_band;
		phddctx->config->nChannelBondingMode24GHz =
			smeconfig.csrConfig.channelBondingMode24GHz;
		phddctx->config->nChannelBondingMode5GHz =
			smeconfig.csrConfig.channelBondingMode5GHz;
		phddctx->config->vhtChannelWidth = vhtchanwidth;
		if (hdd_update_config_dat(phddctx) == false) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				  "%s: could not update config_dat", __func__);
			return -EIO;
		}
		if (phddctx->config->nChannelBondingMode5GHz)
			phddctx->wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.cap
				|= IEEE80211_HT_CAP_SUP_WIDTH_20_40;
		else
			phddctx->wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.cap
				&= ~IEEE80211_HT_CAP_SUP_WIDTH_20_40;

		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			"New_Phymode= %d ch_bonding=%d band=%d VHT_ch_width=%u",
			phymode, chwidth, curr_band, vhtchanwidth);
	}

	return 0;
}

/**
 * hdd_get_temperature_cb() - "Get Temperature" callback function
 * @temperature: measured temperature
 * @pContext: callback context
 *
 * This function is passed to sme_get_temperature() as the callback
 * function to be invoked when the temperature measurement is
 * available.
 *
 * Return: None
 */
static void hdd_get_temperature_cb(int temperature, void *pContext)
{
	struct statsContext *pTempContext;
	hdd_adapter_t *pAdapter;
	ENTER();
	if (NULL == pContext) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("pContext is NULL"));
		return;
	}
	pTempContext = pContext;
	pAdapter = pTempContext->pAdapter;
	spin_lock(&hdd_context_lock);
	if ((NULL == pAdapter) || (TEMP_CONTEXT_MAGIC != pTempContext->magic)) {
		spin_unlock(&hdd_context_lock);
		hddLog(QDF_TRACE_LEVEL_WARN,
		       FL("Invalid context, pAdapter [%p] magic [%08x]"),
		       pAdapter, pTempContext->magic);
		return;
	}
	if (temperature != 0) {
		pAdapter->temperature = temperature;
	}
	complete(&pTempContext->completion);
	spin_unlock(&hdd_context_lock);
	EXIT();
}

/**
 * wlan_hdd_get_temperature() - get current device temperature
 * @pAdapter: device upon which the request was made
 * @temperature: pointer to where the temperature is to be returned
 *
 * Return: 0 if a temperature value (either current or cached) was
 * returned, otherwise a negative errno is returned.
 *
 */
int wlan_hdd_get_temperature(hdd_adapter_t *pAdapter, int *temperature)
{
	QDF_STATUS status;
	struct statsContext tempContext;
	unsigned long rc;

	ENTER();
	if (NULL == pAdapter) {
		hddLog(QDF_TRACE_LEVEL_ERROR, FL("pAdapter is NULL"));
		return -EPERM;
	}
	init_completion(&tempContext.completion);
	tempContext.pAdapter = pAdapter;
	tempContext.magic = TEMP_CONTEXT_MAGIC;
	status = sme_get_temperature(WLAN_HDD_GET_HAL_CTX(pAdapter),
				     &tempContext, hdd_get_temperature_cb);
	if (QDF_STATUS_SUCCESS != status) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("Unable to retrieve temperature"));
	} else {
		rc = wait_for_completion_timeout(&tempContext.completion,
						 msecs_to_jiffies
							 (WLAN_WAIT_TIME_STATS));
		if (!rc) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       FL
				       ("SME timed out while retrieving temperature"));
		}
	}
	spin_lock(&hdd_context_lock);
	tempContext.magic = 0;
	spin_unlock(&hdd_context_lock);
	*temperature = pAdapter->temperature;
	EXIT();
	return 0;
}

/**
 * iw_setint_getnone() - Generic "set integer" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_setint_getnone(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	hdd_wext_state_t *pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	hdd_context_t *hdd_ctx;
	tSmeConfigParams smeConfig;
	int *value = (int *)extra;
	int sub_cmd = value[0];
	int set_value = value[1];
	int ret;
	int enable_pbm, enable_mp;
	QDF_STATUS status;

	ENTER_DEV(dev);

	INIT_COMPLETION(pWextState->completion_var);
	memset(&smeConfig, 0x00, sizeof(smeConfig));

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	switch (sub_cmd) {
	case WE_SET_11D_STATE:
	{
		if ((ENABLE_11D == set_value)
		    || (DISABLE_11D == set_value)) {

			sme_get_config_param(hHal, &smeConfig);
			smeConfig.csrConfig.Is11dSupportEnabled =
				(bool) set_value;

			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_INFO,
				  ("11D state=%d!!"),
				  smeConfig.csrConfig.
				  Is11dSupportEnabled);

			sme_update_config(hHal, &smeConfig);
		} else {
			return -EINVAL;
		}
		break;
	}

	case WE_WOWL:
	{
		switch (set_value) {
		case 0x00:
			hdd_exit_wowl(pAdapter);
			break;
		case 0x01:
		case 0x02:
		case 0x03:
			enable_mp = (set_value & 0x01) ? 1 : 0;
			enable_pbm = (set_value & 0x02) ? 1 : 0;
			hddLog(LOGE,
			       "magic packet ? = %s pattern byte matching ? = %s",
			       (enable_mp ? "YES" : "NO"),
			       (enable_pbm ? "YES" : "NO"));
			hdd_enter_wowl(pAdapter, enable_mp, enable_pbm);
			break;
		default:
			hddLog(LOGE, "Invalid arg  %d in WE_WOWL IOCTL",
			       set_value);
			ret = -EINVAL;
			break;
		}

		break;
	}
	case WE_SET_POWER:
	{
		switch (set_value) {
		case 1:
			/* Enable PowerSave */
			sme_ps_enable_disable(hHal, pAdapter->sessionId,
					SME_PS_ENABLE);
			break;
		case 2:
			/* Disable PowerSave */
			sme_ps_enable_disable(hHal, pAdapter->sessionId,
					SME_PS_DISABLE);
			break;
		case 3:          /* Enable UASPD */
			sme_ps_uapsd_enable(hHal, pAdapter->sessionId);
			break;
		case 4:          /* Disable UASPD */
			sme_ps_uapsd_disable(hHal, pAdapter->sessionId);
			break;
		default:
			hddLog(LOGE,
				"Invalid arg  %d in WE_SET_POWER IOCTL",
				set_value);
			ret = -EINVAL;
			break;
		}
		break;
	}

	case WE_SET_MAX_ASSOC:
	{
		if ((WNI_CFG_ASSOC_STA_LIMIT_STAMIN > set_value) ||
		    (WNI_CFG_ASSOC_STA_LIMIT_STAMAX < set_value)) {
			ret = -EINVAL;
		} else if (sme_cfg_set_int(hHal, WNI_CFG_ASSOC_STA_LIMIT,
					set_value)
			   != QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR, FL
				  ("failed to set ini parameter, WNI_CFG_ASSOC_STA_LIMIT"));
			ret = -EIO;
		}
		break;
	}

	case WE_SET_SAP_AUTO_CHANNEL_SELECTION:
		if (set_value == 0 || set_value == 1)
			(WLAN_HDD_GET_CTX(pAdapter))->config->force_sap_acs =
								set_value;
		else
			ret = -EINVAL;
		break;

	case WE_SET_DATA_INACTIVITY_TO:
	{
		if ((set_value < CFG_DATA_INACTIVITY_TIMEOUT_MIN) ||
		    (set_value > CFG_DATA_INACTIVITY_TIMEOUT_MAX) ||
		    (sme_cfg_set_int((WLAN_HDD_GET_CTX(pAdapter))->hHal,
				     WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT,
				     set_value) == QDF_STATUS_E_FAILURE)) {
			hddLog(LOGE, "Failure: Could not pass on "
			       "WNI_CFG_PS_DATA_INACTIVITY_TIMEOUT configuration info "
			       "to CCM");
			ret = -EINVAL;
		}
		break;
	}
	case WE_SET_MC_RATE:
	{
		ret = wlan_hdd_set_mc_rate(pAdapter, set_value);
		break;
	}
	case WE_SET_TX_POWER:
	{
		struct qdf_mac_addr bssid;

		qdf_copy_macaddr(&bssid, &pHddStaCtx->conn_info.bssId);
		if (sme_set_tx_power
			    (hHal, pAdapter->sessionId, bssid,
			    pAdapter->device_mode,
			    set_value) != QDF_STATUS_SUCCESS) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: Setting tx power failed", __func__);
			return -EIO;
		}
		break;
	}
	case WE_SET_MAX_TX_POWER:
	{
		struct qdf_mac_addr bssid;
		struct qdf_mac_addr selfMac;

		hddLog(QDF_TRACE_LEVEL_INFO,
		       "%s: Setting maximum tx power %d dBm", __func__,
		       set_value);
		qdf_copy_macaddr(&bssid, &pHddStaCtx->conn_info.bssId);
		qdf_copy_macaddr(&selfMac, &pHddStaCtx->conn_info.bssId);

		if (sme_set_max_tx_power(hHal, bssid, selfMac, set_value)
		    != QDF_STATUS_SUCCESS) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: Setting maximum tx power failed",
			       __func__);
			return -EIO;
		}

		break;
	}
	case WE_SET_MAX_TX_POWER_2_4:
	{
		hddLog(QDF_TRACE_LEVEL_INFO,
		       "%s: Setting maximum tx power %d dBm for 2.4 GHz band",
		       __func__, set_value);
		if (sme_set_max_tx_power_per_band(eCSR_BAND_24, set_value) !=
		    QDF_STATUS_SUCCESS) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: Setting maximum tx power failed for 2.4 GHz band",
			       __func__);
			return -EIO;
		}

		break;
	}
	case WE_SET_MAX_TX_POWER_5_0:
	{
		hddLog(QDF_TRACE_LEVEL_INFO,
		       "%s: Setting maximum tx power %d dBm for 5.0 GHz band",
		       __func__, set_value);
		if (sme_set_max_tx_power_per_band(eCSR_BAND_5G, set_value) !=
		    QDF_STATUS_SUCCESS) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: Setting maximum tx power failed for 5.0 GHz band",
			       __func__);
			return -EIO;
		}

		break;
	}
	case WE_SET_HIGHER_DTIM_TRANSITION:
	{
		if (!((set_value == false) || (set_value == true))) {
			hddLog(LOGE, "Dynamic DTIM Incorrect data:%d",
			       set_value);
			ret = -EINVAL;
		} else {
			if (pAdapter->higherDtimTransition != set_value) {
				pAdapter->higherDtimTransition =
					set_value;
				hddLog(LOG1,
				       "%s: higherDtimTransition set to :%d",
				       __func__,
				       pAdapter->higherDtimTransition);
			}
		}

		break;
	}

	case WE_SET_TM_LEVEL:
	{
		hddLog(QDF_TRACE_LEVEL_INFO,
		       "Set Thermal Mitigation Level %d", set_value);
		(void)sme_set_thermal_level(hHal, set_value);
		break;
	}

	case WE_SET_PHYMODE:
	{
		hdd_context_t *phddctx = WLAN_HDD_GET_CTX(pAdapter);

		ret =
			wlan_hdd_update_phymode(dev, hHal, set_value,
						phddctx);
		break;
	}

	case WE_SET_NSS:
	{
		hddLog(LOG1, "Set NSS = %d", set_value);
		if ((set_value > 2) || (set_value <= 0)) {
			hddLog(LOGE, "NSS greater than 2 not supported");
			ret = -EINVAL;
		} else {
			if (QDF_STATUS_SUCCESS !=
				hdd_update_nss(WLAN_HDD_GET_CTX(pAdapter),
				set_value))
				ret = -EINVAL;
		}
		break;
	}

	case WE_SET_GTX_HT_MCS:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_HT_MCS %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_HT_MCS,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_VHT_MCS:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_VHT_MCS %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_VHT_MCS,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_USRCFG:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_USR_CFG %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_USR_CFG,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_THRE:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_THRE %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_THRE,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_MARGIN:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_MARGIN %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_MARGIN,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_STEP:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_STEP %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_STEP,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_MINTPC:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_MINTPC %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_MINTPC,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_GTX_BWMASK:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_GTX_BWMASK %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_GTX_BW_MASK,
					  set_value, GTX_CMD);
		break;
	}

	case WE_SET_LDPC:
	{
		uint32_t value;
		union {
			uint16_t nCfgValue16;
			tSirMacHTCapabilityInfo htCapInfo;
		} uHTCapabilityInfo;

		hddLog(LOG1, "LDPC val %d", set_value);
		/* get the HT capability info */
		ret = sme_cfg_get_int(hHal, WNI_CFG_HT_CAP_INFO, &value);
		if (QDF_STATUS_SUCCESS != ret) {
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  "%s: could not get HT capability info",
				  __func__);
			return -EIO;
		}

		uHTCapabilityInfo.nCfgValue16 = 0xFFFF & value;
		if ((set_value
		     && (uHTCapabilityInfo.htCapInfo.advCodingCap))
		    || (!set_value)) {
			ret =
				sme_update_ht_config(hHal,
						     pAdapter->sessionId,
						     WNI_CFG_HT_CAP_INFO_ADVANCE_CODING,
						     set_value);
		}

		if (ret)
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  "Failed to set LDPC value");

		break;
	}

	case WE_SET_TX_STBC:
	{
		uint32_t value;
		union {
			uint16_t nCfgValue16;
			tSirMacHTCapabilityInfo htCapInfo;
		} uHTCapabilityInfo;

		hddLog(LOG1, "TX_STBC val %d", set_value);
		/* get the HT capability info */
		ret = sme_cfg_get_int(hHal, WNI_CFG_HT_CAP_INFO, &value);
		if (QDF_STATUS_SUCCESS != ret) {
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  "%s: could not get HT capability info",
				  __func__);
			return -EIO;
		}

		uHTCapabilityInfo.nCfgValue16 = 0xFFFF & value;
		if ((set_value && (uHTCapabilityInfo.htCapInfo.txSTBC))
		    || (!set_value)) {
			ret =
				sme_update_ht_config(hHal,
						     pAdapter->sessionId,
						     WNI_CFG_HT_CAP_INFO_TX_STBC,
						     set_value);
		}

		if (ret)
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  "Failed to set TX STBC value");

		break;
	}

	case WE_SET_RX_STBC:
	{
		uint32_t value;
		union {
			uint16_t nCfgValue16;
			tSirMacHTCapabilityInfo htCapInfo;
		} uHTCapabilityInfo;

		hddLog(LOG1, "WMI_VDEV_PARAM_RX_STBC val %d",
		       set_value);
		/* get the HT capability info */
		ret = sme_cfg_get_int(hHal, WNI_CFG_HT_CAP_INFO, &value);
		if (QDF_STATUS_SUCCESS != ret) {
			QDF_TRACE(QDF_MODULE_ID_QDF,
				  QDF_TRACE_LEVEL_ERROR,
				  "%s: could not get HT capability info",
				  __func__);
			return -EIO;
		}

		uHTCapabilityInfo.nCfgValue16 = 0xFFFF & value;
		if ((set_value && (uHTCapabilityInfo.htCapInfo.rxSTBC))
		    || (!set_value)) {
			ret =
				sme_update_ht_config(hHal,
						     pAdapter->sessionId,
						     WNI_CFG_HT_CAP_INFO_RX_STBC,
						     (!set_value) ? set_value
						     : uHTCapabilityInfo.
						     htCapInfo.rxSTBC);
		}

		if (ret)
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  "Failed to set RX STBC value");
		break;
	}

	case WE_SET_SHORT_GI:
	{
		hddLog(LOG1, "WMI_VDEV_PARAM_SGI val %d", set_value);
		ret = sme_update_ht_config(hHal, pAdapter->sessionId,
					   WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ,
					   set_value);
		if (ret)
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  "Failed to set ShortGI value");
		break;
	}

	case WE_SET_RTSCTS:
	{
		uint32_t value;

		hddLog(LOG1, "WMI_VDEV_PARAM_ENABLE_RTSCTS val 0x%x",
		       set_value);

		if ((set_value & HDD_RTSCTS_EN_MASK) ==
		    HDD_RTSCTS_ENABLE)
			value =
				(WLAN_HDD_GET_CTX(pAdapter))->config->
				RTSThreshold;
		else if (((set_value & HDD_RTSCTS_EN_MASK) == 0)
			 || ((set_value & HDD_RTSCTS_EN_MASK) ==
			     HDD_CTS_ENABLE))
			value = WNI_CFG_RTS_THRESHOLD_STAMAX;
		else
			return -EIO;

		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_ENABLE_RTSCTS,
					  set_value, VDEV_CMD);
		if (!ret) {
			if (sme_cfg_set_int
				    (hHal, WNI_CFG_RTS_THRESHOLD, value) !=
				    QDF_STATUS_SUCCESS) {
				hddLog(LOGE, "FAILED TO SET RTSCTS");
				return -EIO;
			}
		}

		break;
	}

	case WE_SET_CHWIDTH:
	{
		bool chwidth = false;
		hdd_context_t *phddctx = WLAN_HDD_GET_CTX(pAdapter);
		/*updating channel bonding only on 5Ghz */
		hddLog(LOG1, "WMI_VDEV_PARAM_CHWIDTH val %d",
		       set_value);
		if (set_value > eHT_CHANNEL_WIDTH_80MHZ) {
			hddLog(LOGE,
			       "Invalid channel width 0->20 1->40 2->80");
			return -EINVAL;
		}

		if ((WNI_CFG_CHANNEL_BONDING_MODE_DISABLE !=
		     csr_convert_cb_ini_value_to_phy_cb_state(phddctx->config->
							      nChannelBondingMode5GHz)))
			chwidth = true;

		sme_get_config_param(hHal, &smeConfig);
		switch (set_value) {
		case eHT_CHANNEL_WIDTH_20MHZ:
			smeConfig.csrConfig.channelBondingMode5GHz =
				WNI_CFG_CHANNEL_BONDING_MODE_DISABLE;
			break;
		case eHT_CHANNEL_WIDTH_40MHZ:
			if (chwidth)
				smeConfig.csrConfig.
				channelBondingMode5GHz =
					phddctx->config->
					nChannelBondingMode5GHz;
			else
				return -EINVAL;

			break;
		case eHT_CHANNEL_WIDTH_80MHZ:
			if (chwidth)
				smeConfig.csrConfig.
				channelBondingMode5GHz =
					phddctx->config->
					nChannelBondingMode5GHz;
			else
				return -EINVAL;

			break;

		default:
			return -EINVAL;
		}

		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_CHWIDTH,
					  set_value, VDEV_CMD);
		if (!ret)
			sme_update_config(hHal, &smeConfig);

		break;
	}

	case WE_SET_ANI_EN_DIS:
	{
		hddLog(LOG1, "WMI_PDEV_PARAM_ANI_ENABLE val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_PDEV_PARAM_ANI_ENABLE,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_ANI_POLL_PERIOD:
	{
		hddLog(LOG1, "WMI_PDEV_PARAM_ANI_POLL_PERIOD val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_PDEV_PARAM_ANI_POLL_PERIOD,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_ANI_LISTEN_PERIOD:
	{
		hddLog(LOG1, "WMI_PDEV_PARAM_ANI_LISTEN_PERIOD val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_PDEV_PARAM_ANI_LISTEN_PERIOD,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_ANI_OFDM_LEVEL:
	{
		hddLog(LOG1, "WMI_PDEV_PARAM_ANI_OFDM_LEVEL val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_PDEV_PARAM_ANI_OFDM_LEVEL,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_ANI_CCK_LEVEL:
	{
		hddLog(LOG1, "WMI_PDEV_PARAM_ANI_CCK_LEVEL val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_PDEV_PARAM_ANI_CCK_LEVEL,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_DYNAMIC_BW:
	{
		hddLog(LOG1, "WMI_PDEV_PARAM_DYNAMIC_BW val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_PDEV_PARAM_DYNAMIC_BW,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_CTS_CBW:
	{
		hddLog(LOG1, "WE_SET_CTS_CBW val %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_PDEV_PARAM_CTS_CBW,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_11N_RATE:
	{
		uint8_t preamble = 0, nss = 0, rix = 0;
		hddLog(LOG1, "WMI_VDEV_PARAM_FIXED_RATE val %d",
		       set_value);

		if (set_value != 0xff) {
			rix = RC_2_RATE_IDX(set_value);
			if (set_value & 0x80) {
				preamble = WMI_RATE_PREAMBLE_HT;
				nss = HT_RC_2_STREAMS(set_value) - 1;
			} else {
				nss = 0;
				rix = RC_2_RATE_IDX(set_value);
				if (set_value & 0x10) {
					preamble =
						WMI_RATE_PREAMBLE_CCK;
					if (rix != 0x3)
						/* Enable Short
						 * preamble always for
						 * CCK except 1mbps
						 */
						rix |= 0x4;
				} else {
					preamble =
						WMI_RATE_PREAMBLE_OFDM;
				}
			}
			set_value = (preamble << 6) | (nss << 4) | rix;
		}
		hdd_info("WMI_VDEV_PARAM_FIXED_RATE val %d rix %d preamble %x nss %d",
			 set_value, rix, preamble, nss);

		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_FIXED_RATE,
					  set_value, VDEV_CMD);
		break;
	}

	case WE_SET_VHT_RATE:
	{
		uint8_t preamble = 0, nss = 0, rix = 0;

		if (set_value != 0xff) {
			rix = RC_2_RATE_IDX_11AC(set_value);
			preamble = WMI_RATE_PREAMBLE_VHT;
			nss = HT_RC_2_STREAMS_11AC(set_value) - 1;

			set_value = (preamble << 6) | (nss << 4) | rix;
		}
		hdd_info("WMI_VDEV_PARAM_FIXED_RATE val %d rix %d preamble %x nss %d",
			 set_value, rix, preamble, nss);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_FIXED_RATE,
					  set_value, VDEV_CMD);
		break;
	}

	case WE_SET_AMPDU:
	{
		hddLog(LOG1, "SET AMPDU val %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  GEN_VDEV_PARAM_AMPDU,
					  set_value, GEN_CMD);
		break;
	}

	case WE_SET_AMSDU:
	{
		hddLog(LOG1, "SET AMSDU val %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  GEN_VDEV_PARAM_AMSDU,
					  set_value, GEN_CMD);
		break;
	}

	case WE_SET_BURST_ENABLE:
	{
		hddLog(LOG1, "SET Burst enable val %d", set_value);
		if ((set_value == 0) || (set_value == 1)) {
			ret = wma_cli_set_command(pAdapter->sessionId,
						  WMI_PDEV_PARAM_BURST_ENABLE,
						  set_value, PDEV_CMD);
		} else
			ret = -EINVAL;
		break;
	}
	case WE_SET_BURST_DUR:
	{
		hddLog(LOG1, "SET Burst duration val %d", set_value);
		if ((set_value > 0) && (set_value <= 8192))
			ret = wma_cli_set_command(pAdapter->sessionId,
						  WMI_PDEV_PARAM_BURST_DUR,
						  set_value,  PDEV_CMD);
		else
			ret = -EINVAL;
		break;
	}

	case WE_SET_TX_CHAINMASK:
	{
		hddLog(LOG1, "WMI_PDEV_PARAM_TX_CHAIN_MASK val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_PDEV_PARAM_TX_CHAIN_MASK,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_RX_CHAINMASK:
	{
		hddLog(LOG1, "WMI_PDEV_PARAM_RX_CHAIN_MASK val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_PDEV_PARAM_RX_CHAIN_MASK,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_TXPOW_2G:
	{
		hddLog(LOG1, "WMI_PDEV_PARAM_TXPOWER_LIMIT2G val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_PDEV_PARAM_TXPOWER_LIMIT2G,
					  set_value, PDEV_CMD);
		break;
	}

	case WE_SET_TXPOW_5G:
	{
		hddLog(LOG1, "WMI_PDEV_PARAM_TXPOWER_LIMIT5G val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_PDEV_PARAM_TXPOWER_LIMIT5G,
					  set_value, PDEV_CMD);
		break;
	}

	/* Firmware debug log */
	case WE_DBGLOG_LOG_LEVEL:
	{
		hddLog(LOG1, "WE_DBGLOG_LOG_LEVEL val %d", set_value);
		hdd_ctx->fw_log_settings.dl_loglevel = set_value;
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_DBGLOG_LOG_LEVEL,
					  set_value, DBG_CMD);
		break;
	}

	case WE_DBGLOG_VAP_ENABLE:
	{
		hddLog(LOG1, "WE_DBGLOG_VAP_ENABLE val %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_DBGLOG_VAP_ENABLE,
					  set_value, DBG_CMD);
		break;
	}

	case WE_DBGLOG_VAP_DISABLE:
	{
		hddLog(LOG1, "WE_DBGLOG_VAP_DISABLE val %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_DBGLOG_VAP_DISABLE,
					  set_value, DBG_CMD);
		break;
	}

	case WE_DBGLOG_MODULE_ENABLE:
	{
		hddLog(LOG1, "WE_DBGLOG_MODULE_ENABLE val %d",
		       set_value);
		hdd_ctx->fw_log_settings.enable = set_value;
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_DBGLOG_MODULE_ENABLE,
					  set_value, DBG_CMD);
		break;
	}

	case WE_DBGLOG_MODULE_DISABLE:
	{
		hddLog(LOG1, "WE_DBGLOG_MODULE_DISABLE val %d",
		       set_value);
		hdd_ctx->fw_log_settings.enable = set_value;
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_DBGLOG_MODULE_DISABLE,
					  set_value, DBG_CMD);
		break;
	}
	case WE_DBGLOG_MOD_LOG_LEVEL:
	{
		hddLog(LOG1, "WE_DBGLOG_MOD_LOG_LEVEL val %d",
		       set_value);

		if (hdd_ctx->fw_log_settings.index >= MAX_MOD_LOGLEVEL)
			hdd_ctx->fw_log_settings.index = 0;

		hdd_ctx->fw_log_settings.
		dl_mod_loglevel[hdd_ctx->fw_log_settings.index] =
			set_value;
		hdd_ctx->fw_log_settings.index++;

		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_DBGLOG_MOD_LOG_LEVEL,
					  set_value, DBG_CMD);
		break;
	}

	case WE_DBGLOG_TYPE:
	{
		hddLog(LOG1, "WE_DBGLOG_TYPE val %d", set_value);
		hdd_ctx->fw_log_settings.dl_type = set_value;
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_DBGLOG_TYPE,
					  set_value, DBG_CMD);
		break;
	}
	case WE_DBGLOG_REPORT_ENABLE:
	{
		hddLog(LOG1, "WE_DBGLOG_REPORT_ENABLE val %d",
		       set_value);
		hdd_ctx->fw_log_settings.dl_report = set_value;
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_DBGLOG_REPORT_ENABLE,
					  set_value, DBG_CMD);
		break;
	}

	case WE_SET_TXRX_FWSTATS:
	{
		hddLog(LOG1, "WE_SET_TXRX_FWSTATS val %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMA_VDEV_TXRX_FWSTATS_ENABLE_CMDID,
					  set_value, VDEV_CMD);
		break;
	}

	case WE_TXRX_FWSTATS_RESET:
	{
		hddLog(LOG1, "WE_TXRX_FWSTATS_RESET val %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMA_VDEV_TXRX_FWSTATS_RESET_CMDID,
					  set_value, VDEV_CMD);
		break;
	}

	case WE_DUMP_STATS:
	{
		hddLog(LOG1, "WE_DUMP_STATS val %d", set_value);
		hdd_wlan_dump_stats(pAdapter, set_value);
		break;
	}

	case WE_CLEAR_STATS:
	{
		hddLog(LOG1, "WE_CLEAR_STATS val %d", set_value);
		switch (set_value) {
		case WLAN_HDD_STATS:
			memset(&pAdapter->stats, 0, sizeof(pAdapter->stats));
			memset(&pAdapter->hdd_stats, 0,
					sizeof(pAdapter->hdd_stats));
			break;
		case WLAN_TXRX_HIST_STATS:
			wlan_hdd_clear_tx_rx_histogram(hdd_ctx);
			break;
		case WLAN_HDD_NETIF_OPER_HISTORY:
			wlan_hdd_clear_netif_queue_history(hdd_ctx);
			break;
		default:
			ol_txrx_clear_stats(set_value);
		}
		break;
	}

	case WE_PPS_PAID_MATCH:
	{
		if (pAdapter->device_mode != QDF_STA_MODE)
			return EINVAL;

		hddLog(LOG1, "WMI_VDEV_PPS_PAID_MATCH val %d ",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PPS_PAID_MATCH,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_GID_MATCH:
	{
		if (pAdapter->device_mode != QDF_STA_MODE)
			return EINVAL;
		hddLog(LOG1, "WMI_VDEV_PPS_GID_MATCH val %d ",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PPS_GID_MATCH,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_EARLY_TIM_CLEAR:
	{
		if (pAdapter->device_mode != QDF_STA_MODE)
			return EINVAL;
		hddLog(LOG1, " WMI_VDEV_PPS_EARLY_TIM_CLEAR val %d ",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PPS_EARLY_TIM_CLEAR,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_EARLY_DTIM_CLEAR:
	{
		if (pAdapter->device_mode != QDF_STA_MODE)
			return EINVAL;
		hddLog(LOG1, "WMI_VDEV_PPS_EARLY_DTIM_CLEAR val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PPS_EARLY_DTIM_CLEAR,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_EOF_PAD_DELIM:
	{
		if (pAdapter->device_mode != QDF_STA_MODE)
			return EINVAL;
		hddLog(LOG1, "WMI_VDEV_PPS_EOF_PAD_DELIM val %d ",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PPS_EOF_PAD_DELIM,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_MACADDR_MISMATCH:
	{
		if (pAdapter->device_mode != QDF_STA_MODE)
			return EINVAL;
		hddLog(LOG1, "WMI_VDEV_PPS_MACADDR_MISMATCH val %d ",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PPS_MACADDR_MISMATCH,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_DELIM_CRC_FAIL:
	{
		if (pAdapter->device_mode != QDF_STA_MODE)
			return EINVAL;
		hddLog(LOG1, "WMI_VDEV_PPS_DELIM_CRC_FAIL val %d ",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PPS_DELIM_CRC_FAIL,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_GID_NSTS_ZERO:
	{
		if (pAdapter->device_mode != QDF_STA_MODE)
			return EINVAL;
		hddLog(LOG1, "WMI_VDEV_PPS_GID_NSTS_ZERO val %d ",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PPS_GID_NSTS_ZERO,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_RSSI_CHECK:
	{
		if (pAdapter->device_mode != QDF_STA_MODE)
			return EINVAL;
		hddLog(LOG1, "WMI_VDEV_PPS_RSSI_CHECK val %d ",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PPS_RSSI_CHECK,
					  set_value, PPS_CMD);
		break;
	}

	case WE_PPS_5G_EBT:
	{
		if (pAdapter->device_mode != QDF_STA_MODE)
			return -EINVAL;

		hddLog(LOG1, "WMI_VDEV_PPS_5G_EBT val %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PPS_5G_EBT,
					  set_value, PPS_CMD);
		break;
	}

	case WE_SET_HTSMPS:
	{
		hddLog(LOG1, "WE_SET_HTSMPS val %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_STA_SMPS_FORCE_MODE_CMDID,
					  set_value, VDEV_CMD);
		break;
	}

	case WE_SET_QPOWER_MAX_PSPOLL_COUNT:
	{
		hddLog(LOG1, "WE_SET_QPOWER_MAX_PSPOLL_COUNT val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_STA_PS_PARAM_QPOWER_PSPOLL_COUNT,
					  set_value, QPOWER_CMD);
		break;
	}

	case WE_SET_QPOWER_MAX_TX_BEFORE_WAKE:
	{
		hddLog(LOG1, "WE_SET_QPOWER_MAX_TX_BEFORE_WAKE val %d",
		       set_value);
		ret = wma_cli_set_command(
				pAdapter->sessionId,
				WMI_STA_PS_PARAM_QPOWER_MAX_TX_BEFORE_WAKE,
				set_value, QPOWER_CMD);
		break;
	}

	case WE_SET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL:
	{
		hddLog(LOG1,
		       "WE_SET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL val %d",
		       set_value);
		ret = wma_cli_set_command(
			pAdapter->sessionId,
			WMI_STA_PS_PARAM_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL,
			set_value, QPOWER_CMD);
		break;
	}

	case WE_SET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL:
	{
		hddLog(LOG1,
		       "WE_SET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL val %d",
		       set_value);
		ret = wma_cli_set_command(
			pAdapter->sessionId,
			WMI_STA_PS_PARAM_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL,
			set_value, QPOWER_CMD);
		break;
	}

	case WE_MCC_CONFIG_LATENCY:
	{
		cds_set_mcc_latency(pAdapter, set_value);
		break;
	}

	case WE_MCC_CONFIG_QUOTA:
	{
		hddLog(LOG1, "iwpriv cmd to set MCC quota with val %dms",
				set_value);
		ret = cds_set_mcc_p2p_quota(pAdapter, set_value);
		break;
	}
	case WE_SET_DEBUG_LOG:
	{
		hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
#ifdef QCA_PKT_PROTO_TRACE
		/* Trace buffer dump only */
		if (CDS_PKT_TRAC_DUMP_CMD == set_value) {
			cds_pkt_trace_buf_dump();
			break;
		}
#endif /* QCA_PKT_PROTO_TRACE */
		hdd_ctx->config->gEnableDebugLog = set_value;
		sme_update_connect_debug(hdd_ctx->hHal, set_value);
		break;
	}
	case WE_SET_EARLY_RX_ADJUST_ENABLE:
	{
		hddLog(LOG1, "SET early_rx enable val %d", set_value);
		if ((set_value == 0) || (set_value == 1))
			ret = wma_cli_set_command(
					pAdapter->sessionId,
					WMI_VDEV_PARAM_EARLY_RX_ADJUST_ENABLE,
					set_value, VDEV_CMD);
		else
			ret = -EINVAL;
		break;
	}
	case WE_SET_EARLY_RX_TGT_BMISS_NUM:
	{
		hddLog(LOG1, "SET early_rx bmiss val %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_EARLY_RX_TGT_BMISS_NUM,
					  set_value, VDEV_CMD);
		break;
	}
	case WE_SET_EARLY_RX_BMISS_SAMPLE_CYCLE:
	{
		hddLog(LOG1, "SET early_rx bmiss sample cycle %d",
		       set_value);
		ret = wma_cli_set_command(
				pAdapter->sessionId,
				WMI_VDEV_PARAM_EARLY_RX_BMISS_SAMPLE_CYCLE,
				set_value, VDEV_CMD);
		break;
	}
	case WE_SET_EARLY_RX_SLOP_STEP:
	{
		hddLog(LOG1, "SET early_rx bmiss slop step val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_EARLY_RX_SLOP_STEP,
					  set_value, VDEV_CMD);
		break;
	}
	case WE_SET_EARLY_RX_INIT_SLOP:
	{
		hddLog(LOG1, "SET early_rx init slop step val %d",
		       set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_EARLY_RX_INIT_SLOP,
					  set_value, VDEV_CMD);
		break;
	}
	case WE_SET_EARLY_RX_ADJUST_PAUSE:
	{
		hddLog(LOG1, "SET early_rx adjust pause %d", set_value);
		if ((set_value == 0) || (set_value == 1))
			ret = wma_cli_set_command(
					pAdapter->sessionId,
					WMI_VDEV_PARAM_EARLY_RX_ADJUST_PAUSE,
					set_value, VDEV_CMD);
		else
			ret = -EINVAL;
		break;
	}
	case WE_SET_EARLY_RX_DRIFT_SAMPLE:
	{
		hddLog(LOG1, "SET early_rx drift sample %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_VDEV_PARAM_EARLY_RX_DRIFT_SAMPLE,
					  set_value, VDEV_CMD);
		break;
	}
	case WE_SET_SCAN_DISABLE:
	{
		hddLog(LOG1, "SET SCAN DISABLE %d", set_value);
		sme_set_scan_disable(WLAN_HDD_GET_HAL_CTX(pAdapter), set_value);
		break;
	}
	case WE_START_FW_PROFILE:
	{
		hddLog(LOG1, "WE_START_FW_PROFILE %d", set_value);
		ret = wma_cli_set_command(pAdapter->sessionId,
					WMI_WLAN_PROFILE_TRIGGER_CMDID,
					set_value, DBG_CMD);
		break;
	}
	case WE_SET_CHANNEL:
	{
		hddLog(LOG1, "Set Channel %d Session ID %d mode %d", set_value,
				  pAdapter->sessionId, pAdapter->device_mode);

		if ((QDF_STA_MODE == pAdapter->device_mode) ||
		   (QDF_P2P_CLIENT_MODE == pAdapter->device_mode)) {

			status = sme_ext_change_channel(hHal,
				   set_value, pAdapter->sessionId);
			if (status != QDF_STATUS_SUCCESS) {
				hddLog(LOGE,
				  FL("Error in change channel status %d"),
				  status);
				ret = -EINVAL;
			}
		} else {
			hddLog(LOGE,
			  FL("change channel not supported for device mode %d"),
			  pAdapter->device_mode);
			ret = -EINVAL;
		}
		break;
	}
	case WE_SET_CONC_SYSTEM_PREF:
	{
		hdd_info("New preference: %d", set_value);
		if (!((set_value >= CFG_CONC_SYSTEM_PREF_MIN) &&
				(set_value <= CFG_CONC_SYSTEM_PREF_MAX))) {
			hdd_err("Invalid system preference %d", set_value);
			return -EINVAL;
		}

		/* hdd_ctx, hdd_ctx->config are already checked for null */
		hdd_ctx->config->conc_system_pref = set_value;
		break;
	}
	default:
	{
		hddLog(LOGE, "%s: Invalid sub command %d", __func__,
		       sub_cmd);
		ret = -EINVAL;
		break;
	}
	}
	EXIT();
	return ret;
}

static int iw_setint_getnone(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu,
			     char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_setint_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_setnone_get_threeint() - return three value to up layer.
 *
 * @dev: pointer of net_device of this wireless card
 * @info: meta data about Request sent
 * @wrqu: include request info
 * @extra: buf used for in/Output
 *
 * Return: execute result
 */
static int __iw_setnone_get_threeint(struct net_device *dev,
					struct iw_request_info *info,
					union iwreq_data *wrqu, char *extra)
{
	int ret = 0; /* success */
	uint32_t *value = (int *)extra;
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	ENTER_DEV(dev);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	hdd_info(FL("param = %d"), value[0]);
	switch (value[0]) {
	case WE_GET_TSF:
		ret = hdd_indicate_tsf(adapter, value, 3);
		break;
	default:
		hdd_err("Invalid IOCTL get_value command %d", value[0]);
		break;
	}
	return ret;
}

/**
 * iw_setnone_get_threeint() - return three value to up layer.
 *
 * @dev: pointer of net_device of this wireless card
 * @info: meta data about Request sent
 * @wrqu: include request info
 * @extra: buf used for in/Output
 *
 * Return: execute result
 */
static int iw_setnone_get_threeint(struct net_device *dev,
					struct iw_request_info *info,
					union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_setnone_get_threeint(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_setchar_getnone() - Generic "set string" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_setchar_getnone(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	QDF_STATUS vstatus;
	int sub_cmd;
	int ret;
	char *pBuffer = NULL;
	hdd_adapter_t *pAdapter = (netdev_priv(dev));
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	struct hdd_config *pConfig = hdd_ctx->config;
	struct iw_point s_priv_data;

	ENTER_DEV(dev);

	if (!capable(CAP_NET_ADMIN)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
		FL("permission check failed"));
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/* helper function to get iwreq_data with compat handling. */
	if (hdd_priv_get_data(&s_priv_data, wrqu)) {
		return -EINVAL;
	}

	/* make sure all params are correctly passed to function */
	if ((NULL == s_priv_data.pointer) || (0 == s_priv_data.length)) {
		return -EINVAL;
	}

	sub_cmd = s_priv_data.flags;

	/* ODD number is used for set, copy data using copy_from_user */
	pBuffer = mem_alloc_copy_from_user_helper(s_priv_data.pointer,
						  s_priv_data.length);
	if (NULL == pBuffer) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "mem_alloc_copy_from_user_helper fail");
		return -ENOMEM;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received length %d", __func__, s_priv_data.length);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: Received data %s", __func__, pBuffer);

	switch (sub_cmd) {
	case WE_WOWL_ADD_PTRN:
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO, "ADD_PTRN");
		hdd_add_wowl_ptrn(pAdapter, pBuffer);
		break;
	case WE_WOWL_DEL_PTRN:
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO, "DEL_PTRN");
		hdd_del_wowl_ptrn(pAdapter, pBuffer);
		break;
	case WE_NEIGHBOR_REPORT_REQUEST:
	{
		tRrmNeighborReq neighborReq;
		tRrmNeighborRspCallbackInfo callbackInfo;

		if (pConfig->fRrmEnable) {
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_INFO,
				  "Neighbor Request");
			neighborReq.no_ssid =
				(s_priv_data.length - 1) ? false : true;
			if (!neighborReq.no_ssid) {
				neighborReq.ssid.length =
					(s_priv_data.length - 1) >
					32 ? 32 : (s_priv_data.length - 1);
				qdf_mem_copy(neighborReq.ssid.ssId,
					     pBuffer,
					     neighborReq.ssid.length);
			}

			callbackInfo.neighborRspCallback = NULL;
			callbackInfo.neighborRspCallbackContext = NULL;
			callbackInfo.timeout = 5000;            /* 5 seconds */
			sme_neighbor_report_request(WLAN_HDD_GET_HAL_CTX
							    (pAdapter),
						    pAdapter->sessionId,
						    &neighborReq,
						    &callbackInfo);
		} else {
			hddLog(LOGE,
			       "%s: Ignoring neighbor request as RRM is not enabled",
			       __func__);
			ret = -EINVAL;
		}
	}
	break;
	case WE_SET_AP_WPS_IE:
		hddLog(LOGE, "Received WE_SET_AP_WPS_IE");
		sme_update_p2p_ie(WLAN_HDD_GET_HAL_CTX(pAdapter), pBuffer,
				  s_priv_data.length);
		break;
	case WE_SET_CONFIG:
		vstatus = hdd_execute_global_config_command(hdd_ctx, pBuffer);
		if (QDF_STATUS_SUCCESS != vstatus) {
			ret = -EINVAL;
		}
		break;
	default:
	{
		hddLog(LOGE, "%s: Invalid sub command %d", __func__,
		       sub_cmd);
		ret = -EINVAL;
		break;
	}
	}
	kfree(pBuffer);
	EXIT();
	return ret;
}

static int iw_setchar_getnone(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_setchar_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_setnone_getint() - Generic "get integer" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_setnone_getint(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	int *value = (int *)extra;
	int ret;
	tSmeConfigParams smeConfig;
	hdd_context_t *hdd_ctx;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	switch (value[0]) {
	case WE_GET_11D_STATE:
	{
		sme_get_config_param(hHal, &smeConfig);

		*value = smeConfig.csrConfig.Is11dSupportEnabled;

		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  ("11D state=%d!!"), *value);

		break;
	}

	case WE_IBSS_STATUS:
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  "****Return IBSS Status*****");
		break;

	case WE_GET_WLAN_DBG:
	{
		qdf_trace_display();
		*value = 0;
		break;
	}
	case WE_GET_MAX_ASSOC:
	{
		if (sme_cfg_get_int
			    (hHal, WNI_CFG_ASSOC_STA_LIMIT,
			    (uint32_t *) value) != QDF_STATUS_SUCCESS) {
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_WARN, FL
					  ("failed to get ini parameter, WNI_CFG_ASSOC_STA_LIMIT"));
			ret = -EIO;
		}
		break;
	}
	case WE_GET_SAP_AUTO_CHANNEL_SELECTION:
		*value = (WLAN_HDD_GET_CTX(
				pAdapter))->config->force_sap_acs;
		break;

	case WE_GET_CONCURRENCY_MODE:
	{
		*value = cds_get_concurrency_mode();

		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  ("concurrency mode=%d"), *value);
		break;
	}

	case WE_GET_NSS:
	{
		sme_get_config_param(hHal, &smeConfig);
		*value = (smeConfig.csrConfig.enable2x2 == 0) ? 1 : 2;
		hddLog(LOG1, "GET_NSS: Current NSS:%d", *value);
		break;
	}

	case WE_GET_GTX_HT_MCS:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_HT_MCS");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_HT_MCS,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_VHT_MCS:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_VHT_MCS");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_VHT_MCS,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_USRCFG:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_USR_CFG");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_USR_CFG,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_THRE:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_THRE");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_THRE,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_MARGIN:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_MARGIN");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_MARGIN,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_STEP:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_STEP");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_STEP,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_MINTPC:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_MINTPC");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_MINTPC,
					     GTX_CMD);
		break;
	}

	case WE_GET_GTX_BWMASK:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_GTX_BW_MASK");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PARAM_GTX_BW_MASK,
					     GTX_CMD);
		break;
	}

	case WE_GET_LDPC:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_LDPC");
		*value = sme_get_ht_config(hHal, pAdapter->sessionId,
					   WNI_CFG_HT_CAP_INFO_ADVANCE_CODING);
		break;
	}

	case WE_GET_TX_STBC:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_TX_STBC");
		*value = sme_get_ht_config(hHal, pAdapter->sessionId,
					   WNI_CFG_HT_CAP_INFO_TX_STBC);
		break;
	}

	case WE_GET_RX_STBC:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_RX_STBC");
		*value = sme_get_ht_config(hHal, pAdapter->sessionId,
					   WNI_CFG_HT_CAP_INFO_RX_STBC);
		break;
	}

	case WE_GET_SHORT_GI:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_SGI");
		*value = sme_get_ht_config(hHal, pAdapter->sessionId,
					   WNI_CFG_HT_CAP_INFO_SHORT_GI_20MHZ);
		break;
	}

	case WE_GET_RTSCTS:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_ENABLE_RTSCTS");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PARAM_ENABLE_RTSCTS,
					     VDEV_CMD);
		break;
	}

	case WE_GET_CHWIDTH:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_CHWIDTH");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PARAM_CHWIDTH,
					     VDEV_CMD);
		break;
	}

	case WE_GET_ANI_EN_DIS:
	{
		hddLog(LOG1, "GET WMI_PDEV_PARAM_ANI_ENABLE");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_PDEV_PARAM_ANI_ENABLE,
					     PDEV_CMD);
		break;
	}

	case WE_GET_ANI_POLL_PERIOD:
	{
		hddLog(LOG1, "GET WMI_PDEV_PARAM_ANI_POLL_PERIOD");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_PDEV_PARAM_ANI_POLL_PERIOD,
					     PDEV_CMD);
		break;
	}

	case WE_GET_ANI_LISTEN_PERIOD:
	{
		hddLog(LOG1, "GET WMI_PDEV_PARAM_ANI_LISTEN_PERIOD");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_PDEV_PARAM_ANI_LISTEN_PERIOD,
					     PDEV_CMD);
		break;
	}

	case WE_GET_ANI_OFDM_LEVEL:
	{
		hddLog(LOG1, "GET WMI_PDEV_PARAM_ANI_OFDM_LEVEL");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_PDEV_PARAM_ANI_OFDM_LEVEL,
					     PDEV_CMD);
		break;
	}

	case WE_GET_ANI_CCK_LEVEL:
	{
		hddLog(LOG1, "GET WMI_PDEV_PARAM_ANI_CCK_LEVEL");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_PDEV_PARAM_ANI_CCK_LEVEL,
					     PDEV_CMD);
		break;
	}

	case WE_GET_DYNAMIC_BW:
	{
		hddLog(LOG1, "GET WMI_PDEV_PARAM_ANI_CCK_LEVEL");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_PDEV_PARAM_DYNAMIC_BW,
					     PDEV_CMD);
		break;
	}

	case WE_GET_11N_RATE:
	{
		hddLog(LOG1, "GET WMI_VDEV_PARAM_FIXED_RATE");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PARAM_FIXED_RATE,
					     VDEV_CMD);
		break;
	}

	case WE_GET_AMPDU:
	{
		hddLog(LOG1, "GET AMPDU");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     GEN_VDEV_PARAM_AMPDU,
					     GEN_CMD);
		break;
	}

	case WE_GET_AMSDU:
	{
		hddLog(LOG1, "GET AMSDU");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     GEN_VDEV_PARAM_AMSDU,
					     GEN_CMD);
		break;
	}

	case WE_GET_ROAM_SYNCH_DELAY:
	{
		hddLog(LOG1, "GET ROAM SYNCH DELAY");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     GEN_VDEV_ROAM_SYNCH_DELAY,
					     GEN_CMD);
		break;
	}

	case WE_GET_BURST_ENABLE:
	{
		hddLog(LOG1, "GET Burst enable value");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_PDEV_PARAM_BURST_ENABLE,
					     PDEV_CMD);
		break;
	}
	case WE_GET_BURST_DUR:
	{
		hddLog(LOG1, "GET Burst Duration value");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_PDEV_PARAM_BURST_DUR,
					     PDEV_CMD);
		break;
	}

	case WE_GET_TX_CHAINMASK:
	{
		hddLog(LOG1, "GET WMI_PDEV_PARAM_TX_CHAIN_MASK");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_PDEV_PARAM_TX_CHAIN_MASK,
					     PDEV_CMD);
		break;
	}

	case WE_GET_RX_CHAINMASK:
	{
		hddLog(LOG1, "GET WMI_PDEV_PARAM_RX_CHAIN_MASK");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_PDEV_PARAM_RX_CHAIN_MASK,
					     PDEV_CMD);
		break;
	}

	case WE_GET_TXPOW_2G:
	{
		uint32_t txpow2g = 0;
		tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
		hddLog(LOG1, "GET WMI_PDEV_PARAM_TXPOWER_LIMIT2G");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_PDEV_PARAM_TXPOWER_LIMIT2G,
					     PDEV_CMD);
		if (QDF_STATUS_SUCCESS !=
		    sme_cfg_get_int(hHal, WNI_CFG_CURRENT_TX_POWER_LEVEL,
				    &txpow2g)) {
			return -EIO;
		}
		hddLog(LOG1, "2G tx_power %d", txpow2g);
		break;
	}

	case WE_GET_TXPOW_5G:
	{
		uint32_t txpow5g = 0;
		tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
		hddLog(LOG1, "GET WMI_PDEV_PARAM_TXPOWER_LIMIT5G");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_PDEV_PARAM_TXPOWER_LIMIT5G,
					     PDEV_CMD);
		if (QDF_STATUS_SUCCESS !=
		    sme_cfg_get_int(hHal, WNI_CFG_CURRENT_TX_POWER_LEVEL,
				    &txpow5g)) {
			return -EIO;
		}
		hddLog(LOG1, "5G tx_power %d", txpow5g);
		break;
	}

	case WE_GET_PPS_PAID_MATCH:
	{
		hddLog(LOG1, "GET WMI_VDEV_PPS_PAID_MATCH");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PPS_PAID_MATCH,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_GID_MATCH:
	{
		hddLog(LOG1, "GET WMI_VDEV_PPS_GID_MATCH");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PPS_GID_MATCH,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_EARLY_TIM_CLEAR:
	{
		hddLog(LOG1, "GET WMI_VDEV_PPS_EARLY_TIM_CLEAR");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PPS_EARLY_TIM_CLEAR,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_EARLY_DTIM_CLEAR:
	{
		hddLog(LOG1, "GET WMI_VDEV_PPS_EARLY_DTIM_CLEAR");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PPS_EARLY_DTIM_CLEAR,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_EOF_PAD_DELIM:
	{
		hddLog(LOG1, "GET WMI_VDEV_PPS_EOF_PAD_DELIM");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PPS_EOF_PAD_DELIM,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_MACADDR_MISMATCH:
	{
		hddLog(LOG1, "GET WMI_VDEV_PPS_MACADDR_MISMATCH");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PPS_MACADDR_MISMATCH,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_DELIM_CRC_FAIL:
	{
		hddLog(LOG1, "GET WMI_VDEV_PPS_DELIM_CRC_FAIL");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PPS_DELIM_CRC_FAIL,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_GID_NSTS_ZERO:
	{
		hddLog(LOG1, "GET WMI_VDEV_PPS_GID_NSTS_ZERO");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PPS_GID_NSTS_ZERO,
					     PPS_CMD);
		break;
	}

	case WE_GET_PPS_RSSI_CHECK:
	{

		hddLog(LOG1, "GET WMI_VDEV_PPS_RSSI_CHECK");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_VDEV_PPS_RSSI_CHECK,
					     PPS_CMD);
		break;
	}

	case WE_GET_QPOWER_MAX_PSPOLL_COUNT:
	{
		hddLog(LOG1, "WE_GET_QPOWER_MAX_PSPOLL_COUNT");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_STA_PS_PARAM_QPOWER_PSPOLL_COUNT,
					     QPOWER_CMD);
		break;
	}

	case WE_GET_QPOWER_MAX_TX_BEFORE_WAKE:
	{
		hddLog(LOG1, "WE_GET_QPOWER_MAX_TX_BEFORE_WAKE");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_STA_PS_PARAM_QPOWER_MAX_TX_BEFORE_WAKE,
					     QPOWER_CMD);
		break;
	}

	case WE_GET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL:
	{
		hddLog(LOG1, "WE_GET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_STA_PS_PARAM_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL,
					     QPOWER_CMD);
		break;
	}

	case WE_GET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL:
	{
		hddLog(LOG1, "WE_GET_QPOWER_MAX_PSPOLL_COUNT");
		*value = wma_cli_get_command(pAdapter->sessionId,
					     WMI_STA_PS_PARAM_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL,
					     QPOWER_CMD);
		break;
	}
	case WE_CAP_TSF:
		ret = hdd_capture_tsf(pAdapter, (uint32_t *)value, 1);
		break;
	case WE_GET_TEMPERATURE:
	{
		hddLog(QDF_TRACE_LEVEL_INFO, "WE_GET_TEMPERATURE");
		ret = wlan_hdd_get_temperature(pAdapter, value);
		break;
	}
	default:
	{
		hddLog(LOGE, "Invalid IOCTL get_value command %d",
		       value[0]);
		break;
	}
	}
	EXIT();
	return ret;
}

static int iw_setnone_getint(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_setnone_getint(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_set_three_ints_getnone() - Generic "set 3 params" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_three_ints_getnone(struct net_device *dev,
				       struct iw_request_info *info,
				       union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int *value = (int *)extra;
	int sub_cmd = value[0];
	int ret;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);

	ENTER_DEV(dev);

	if (!capable(CAP_NET_ADMIN)) {
		hddLog(LOGE, FL("permission check failed"));
		return -EPERM;
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	switch (sub_cmd) {

	case WE_SET_WLAN_DBG:
		qdf_trace_set_value(value[1], value[2], value[3]);
		break;
	case WE_SET_DP_TRACE:
		qdf_dp_trace_set_value(value[1], value[2], value[3]);
		break;

	/* value[3] the acs band is not required as start and end channels are
	 * enough but this cmd is maintained under set three ints for historic
	 * reasons.
	 */
	case WE_SET_SAP_CHANNELS:
		if (wlan_hdd_validate_operation_channel(pAdapter, value[1]) !=
			QDF_STATUS_SUCCESS ||
			wlan_hdd_validate_operation_channel(pAdapter,
					value[2]) != QDF_STATUS_SUCCESS) {
			ret = -EINVAL;
		} else {
			hdd_ctx->config->force_sap_acs_st_ch = value[1];
			hdd_ctx->config->force_sap_acs_end_ch = value[2];
		}
		break;
	case WE_SET_DUAL_MAC_SCAN_CONFIG:
		hdd_debug("Ioctl to set dual mac scan config");
		if (hdd_ctx->config->dual_mac_feature_disable) {
			hdd_err("Dual mac feature is disabled from INI");
			return -EPERM;
		}
		hdd_debug("%d %d %d", value[1], value[2], value[3]);
		cds_set_dual_mac_scan_config(value[1], value[2], value[3]);
		break;
	default:
		hddLog(LOGE, "%s: Invalid IOCTL command %d", __func__, sub_cmd);
		break;

	}
	EXIT();
	return ret;
}

int iw_set_three_ints_getnone(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_three_ints_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_connection_state_string() - Get connection state string
 * @connection_state: enum to be converted to a string
 *
 * Return: the string equivalent of @connection_state
 */
static const char *
hdd_connection_state_string(eConnectionState connection_state)
{
	switch (connection_state) {
		CASE_RETURN_STRING(eConnectionState_NotConnected);
		CASE_RETURN_STRING(eConnectionState_Connecting);
		CASE_RETURN_STRING(eConnectionState_Associated);
		CASE_RETURN_STRING(eConnectionState_IbssDisconnected);
		CASE_RETURN_STRING(eConnectionState_IbssConnected);
		CASE_RETURN_STRING(eConnectionState_Disconnecting);
	default:
		return "UNKNOWN";
	}
}

/**
 * iw_get_char_setnone() - Generic "get string" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_char_setnone(struct net_device *dev,
				 struct iw_request_info *info,
				 union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int sub_cmd = wrqu->data.flags;
	hdd_context_t *hdd_ctx;
	int ret;
#ifdef WLAN_FEATURE_11W
	hdd_wext_state_t *pWextState;
#endif

#ifdef WLAN_FEATURE_11W
	pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
#endif

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	switch (sub_cmd) {
	case WE_WLAN_VERSION:
	{
		hdd_wlan_get_version(pAdapter, wrqu, extra);
		break;
	}

	case WE_GET_STATS:
	{
		hdd_wlan_get_stats(pAdapter, &(wrqu->data.length),
				   extra, WE_MAX_STR_LEN);
		break;
	}

	case WE_LIST_FW_PROFILE:
		hdd_wlan_list_fw_profile(&(wrqu->data.length),
					extra, WE_MAX_STR_LEN);
		break;

	/* The case prints the current state of the HDD, SME, CSR, PE,
	 * TL it can be extended for WDI Global State as well.  And
	 * currently it only checks P2P_CLIENT adapter.  P2P_DEVICE
	 * and P2P_GO have not been added as of now.
	 */
	case WE_GET_STATES:
	{
		int buf = 0, len = 0;
		int adapter_num = 0;
		int count = 0, check = 1;

		tHalHandle hHal = NULL;
		tpAniSirGlobal pMac = NULL;
		hdd_station_ctx_t *pHddStaCtx = NULL;

		hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
		hdd_adapter_t *useAdapter = NULL;

		/* Print wlan0 or p2p0 states based on the adapter_num
		 * by using the correct adapter
		 */
		while (adapter_num < 2) {
			if (WLAN_ADAPTER == adapter_num) {
				useAdapter = pAdapter;
				buf =
					scnprintf(extra + len,
						  WE_MAX_STR_LEN - len,
						  "\n\n wlan0 States:-");
				len += buf;
			} else if (P2P_ADAPTER == adapter_num) {
				buf =
					scnprintf(extra + len,
						  WE_MAX_STR_LEN - len,
						  "\n\n p2p0 States:-");
				len += buf;

				if (!pHddCtx) {
					buf =
						scnprintf(extra + len,
							  WE_MAX_STR_LEN -
							  len,
							  "\n pHddCtx is NULL");
					len += buf;
					break;
				}

				/* Printing p2p0 states only in the
				 * case when the device is configured
				 * as a p2p_client
				 */
				useAdapter =
					hdd_get_adapter(pHddCtx,
							QDF_P2P_CLIENT_MODE);
				if (!useAdapter) {
					buf =
						scnprintf(extra + len,
							  WE_MAX_STR_LEN -
							  len,
							  "\n Device not configured as P2P_CLIENT.");
					len += buf;
					break;
				}
			}

			hHal = WLAN_HDD_GET_HAL_CTX(useAdapter);
			if (!hHal) {
				buf =
					scnprintf(extra + len,
						  WE_MAX_STR_LEN - len,
						  "\n pMac is NULL");
				len += buf;
				break;
			}
			pMac = PMAC_STRUCT(hHal);
			if (!pMac) {
				buf =
					scnprintf(extra + len,
						  WE_MAX_STR_LEN - len,
						  "\n pMac is NULL");
				len += buf;
				break;
			}
			pHddStaCtx =
				WLAN_HDD_GET_STATION_CTX_PTR(useAdapter);


			buf =
				scnprintf(extra + len, WE_MAX_STR_LEN - len,
					  "\n HDD Conn State - %s "
					  "\n \n SME State:"
					  "\n Neighbour Roam State - %s"
					  "\n CSR State - %s"
					  "\n CSR Substate - %s",
					  hdd_connection_state_string
						  (pHddStaCtx->conn_info.connState),
					  mac_trace_get_neighbour_roam_state
						  (sme_get_neighbor_roam_state
							  (hHal, useAdapter->sessionId)),
					  mac_trace_getcsr_roam_state
						  (sme_get_current_roam_state
							  (hHal, useAdapter->sessionId)),
					  mac_trace_getcsr_roam_sub_state
						  (sme_get_current_roam_sub_state
							  (hHal, useAdapter->sessionId))
					  );
			len += buf;
			adapter_num++;
		}

		if (hHal) {
			/* Printing Lim State starting with global lim states */
			buf =
				scnprintf(extra + len, WE_MAX_STR_LEN - len,
					  "\n \n LIM STATES:-"
					  "\n Global Sme State - %s "
					  "\n Global mlm State - %s " "\n",
					  mac_trace_get_lim_sme_state
						  (sme_get_lim_sme_state(hHal)),
					  mac_trace_get_lim_mlm_state
						  (sme_get_lim_sme_state(hHal))
					  );
			len += buf;

			/* Printing the PE Sme and Mlm states for valid lim sessions */
			while (check < 3 && count < 255) {
				if (sme_is_lim_session_valid(hHal, count)) {
					buf =
						scnprintf(extra + len,
							  WE_MAX_STR_LEN -
							  len,
							  "\n Lim Valid Session %d:-"
							  "\n PE Sme State - %s "
							  "\n PE Mlm State - %s "
							  "\n", check,
							  mac_trace_get_lim_sme_state
								  (sme_get_lim_sme_session_state
									  (hHal, count)),
							  mac_trace_get_lim_mlm_state
								  (sme_get_lim_mlm_session_state
									  (hHal, count))
							  );

					len += buf;
					check++;
				}
				count++;
			}
		}

		wrqu->data.length = strlen(extra) + 1;
		break;
	}

	case WE_GET_CFG:
	{
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  "%s: Printing CLD global INI Config",
			  __func__);
		hdd_cfg_get_global_config(WLAN_HDD_GET_CTX(pAdapter),
					  extra,
					  QCSAP_IOCTL_MAX_STR_LEN);
		wrqu->data.length = strlen(extra) + 1;
		break;
	}
	case WE_GET_RSSI:
	{
		int8_t s7Rssi = 0;
		wlan_hdd_get_rssi(pAdapter, &s7Rssi);
		snprintf(extra, WE_MAX_STR_LEN, "rssi=%d", s7Rssi);
		wrqu->data.length = strlen(extra) + 1;
		break;
	}

	case WE_GET_WMM_STATUS:
	{
		snprintf(extra, WE_MAX_STR_LEN,
			 "\nDir: 0=up, 1=down, 3=both\n"
			 "|------------------------|\n"
			 "|AC | ACM |Admitted| Dir |\n"
			 "|------------------------|\n"
			 "|VO |  %d  |  %3s   |  %d  |\n"
			 "|VI |  %d  |  %3s   |  %d  |\n"
			 "|BE |  %d  |  %3s   |  %d  |\n"
			 "|BK |  %d  |  %3s   |  %d  |\n"
			 "|------------------------|\n",
			 pAdapter->hddWmmStatus.
			 wmmAcStatus[SME_AC_VO].wmmAcAccessRequired,
			 pAdapter->hddWmmStatus.
			 wmmAcStatus[SME_AC_VO].
			 wmmAcAccessAllowed ? "YES" : "NO",
			 pAdapter->hddWmmStatus.
			 wmmAcStatus[SME_AC_VO].wmmAcTspecInfo.
			 ts_info.direction,
			 pAdapter->hddWmmStatus.
			 wmmAcStatus[SME_AC_VI].wmmAcAccessRequired,
			 pAdapter->hddWmmStatus.
			 wmmAcStatus[SME_AC_VI].
			 wmmAcAccessAllowed ? "YES" : "NO",
			 pAdapter->hddWmmStatus.
			 wmmAcStatus[SME_AC_VI].wmmAcTspecInfo.
			 ts_info.direction,
			 pAdapter->hddWmmStatus.
			 wmmAcStatus[SME_AC_BE].wmmAcAccessRequired,
			 pAdapter->hddWmmStatus.
			 wmmAcStatus[SME_AC_BE].
			 wmmAcAccessAllowed ? "YES" : "NO",
			 pAdapter->hddWmmStatus.
			 wmmAcStatus[SME_AC_BE].wmmAcTspecInfo.
			 ts_info.direction,
			 pAdapter->hddWmmStatus.
			 wmmAcStatus[SME_AC_BK].wmmAcAccessRequired,
			 pAdapter->hddWmmStatus.
			 wmmAcStatus[SME_AC_BK].
			 wmmAcAccessAllowed ? "YES" : "NO",
			 pAdapter->hddWmmStatus.
			 wmmAcStatus[SME_AC_BK].wmmAcTspecInfo.
			 ts_info.direction);

		wrqu->data.length = strlen(extra) + 1;
		break;
	}
	case WE_GET_CHANNEL_LIST:
	{
		QDF_STATUS status;
		uint8_t i, len;
		char *buf;
		uint8_t ubuf[WNI_CFG_COUNTRY_CODE_LEN];
		uint8_t ubuf_len = WNI_CFG_COUNTRY_CODE_LEN;
		hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);

		tChannelListInfo channel_list;

		memset(&channel_list, 0, sizeof(channel_list));
		status =
			iw_softap_get_channel_list(dev, info, wrqu,
						   (char *)&channel_list);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			hddLog(LOGE, FL("GetChannelList Failed!!!"));
			return -EINVAL;
		}
		buf = extra;
		/*
		 * Maximum channels = WNI_CFG_VALID_CHANNEL_LIST_LEN.
		 * Maximum buffer needed = 5 * number of channels.
		 * Check ifsufficient buffer is available and then
		 * proceed to fill the buffer.
		 */
		if (WE_MAX_STR_LEN <
		    (5 * WNI_CFG_VALID_CHANNEL_LIST_LEN)) {
			hddLog(LOGE,
			       FL("Insufficient Buffer to populate channel list"));
			return -EINVAL;
		}
		len = scnprintf(buf, WE_MAX_STR_LEN, "%u ",
				channel_list.num_channels);
		if (QDF_STATUS_SUCCESS == sme_get_country_code(hdd_ctx->hHal,
						ubuf, &ubuf_len)) {
			/* Printing Country code in getChannelList */
			for (i = 0; i < (ubuf_len - 1); i++)
				len += scnprintf(buf + len,
						WE_MAX_STR_LEN - len,
						"%c", ubuf[i]);
		}
		for (i = 0; i < channel_list.num_channels; i++) {
			len +=
				scnprintf(buf + len, WE_MAX_STR_LEN - len,
					  " %u", channel_list.channels[i]);
		}
		wrqu->data.length = strlen(extra) + 1;

		break;
	}
#ifdef FEATURE_WLAN_TDLS
	case WE_GET_TDLS_PEERS:
	{
		wrqu->data.length =
			wlan_hdd_tdls_get_all_peers(pAdapter, extra,
						    WE_MAX_STR_LEN) + 1;
		break;
	}
#endif
#ifdef WLAN_FEATURE_11W
	case WE_GET_11W_INFO:
	{
		hddLog(LOGE, "WE_GET_11W_ENABLED = %d",
		       pWextState->roamProfile.MFPEnabled);

		snprintf(extra, WE_MAX_STR_LEN,
			 "\n BSSID %02X:%02X:%02X:%02X:%02X:%02X, Is PMF Assoc? %d"
			 "\n Number of Unprotected Disassocs %d"
			 "\n Number of Unprotected Deauths %d",
			 pWextState->roamProfile.BSSIDs.bssid->bytes[0],
			 pWextState->roamProfile.BSSIDs.bssid->bytes[1],
			 pWextState->roamProfile.BSSIDs.bssid->bytes[2],
			 pWextState->roamProfile.BSSIDs.bssid->bytes[3],
			 pWextState->roamProfile.BSSIDs.bssid->bytes[4],
			 pWextState->roamProfile.BSSIDs.bssid->bytes[5],
			 pWextState->roamProfile.MFPEnabled,
			 pAdapter->hdd_stats.hddPmfStats.
			 numUnprotDisassocRx,
			 pAdapter->hdd_stats.hddPmfStats.
			 numUnprotDeauthRx);

		wrqu->data.length = strlen(extra) + 1;
		break;
	}
#endif
	case WE_GET_IBSS_STA_INFO:
	{
		hdd_station_ctx_t *pHddStaCtx =
			WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
		int idx = 0;
		int length = 0, buf = 0;

		for (idx = 0; idx < MAX_IBSS_PEERS; idx++) {
			if (0 != pHddStaCtx->conn_info.staId[idx]) {
				buf = snprintf
					      ((extra + length),
					      WE_MAX_STR_LEN - length,
					      "\n%d .%02x:%02x:%02x:%02x:%02x:%02x\n",
					      pHddStaCtx->conn_info.staId[idx],
					      pHddStaCtx->conn_info.
					      peerMacAddress[idx].bytes[0],
					      pHddStaCtx->conn_info.
					      peerMacAddress[idx].bytes[1],
					      pHddStaCtx->conn_info.
					      peerMacAddress[idx].bytes[2],
					      pHddStaCtx->conn_info.
					      peerMacAddress[idx].bytes[3],
					      pHddStaCtx->conn_info.
					      peerMacAddress[idx].bytes[4],
					      pHddStaCtx->conn_info.
					      peerMacAddress[idx].bytes[5]
					      );
				length += buf;
			}
		}
		wrqu->data.length = strlen(extra) + 1;
		break;
	}
	case WE_GET_PHYMODE:
	{
		bool ch_bond24 = false, ch_bond5g = false;
		hdd_context_t *hddctx = WLAN_HDD_GET_CTX(pAdapter);
		tHalHandle hal = WLAN_HDD_GET_HAL_CTX(pAdapter);
		eCsrPhyMode phymode;
		eCsrBand currBand;
		tSmeConfigParams smeconfig;

		sme_get_config_param(hal, &smeconfig);
		if (WNI_CFG_CHANNEL_BONDING_MODE_DISABLE !=
		    smeconfig.csrConfig.channelBondingMode24GHz)
			ch_bond24 = true;

		if (WNI_CFG_CHANNEL_BONDING_MODE_DISABLE !=
		    smeconfig.csrConfig.channelBondingMode5GHz)
			ch_bond5g = true;

		phymode = sme_get_phy_mode(hal);
		if ((QDF_STATUS_SUCCESS !=
		     sme_get_freq_band(hal, &currBand))) {
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_INFO,
				  "%s: Failed to get current band config",
				  __func__);
			return -EIO;
		}

		switch (phymode) {
		case eCSR_DOT11_MODE_AUTO:
			snprintf(extra, WE_MAX_STR_LEN, "AUTO MODE");
			break;
		case eCSR_DOT11_MODE_11n:
		case eCSR_DOT11_MODE_11n_ONLY:
			if (currBand == eCSR_BAND_24) {
				if (ch_bond24)
					snprintf(extra, WE_MAX_STR_LEN,
						 "11NGHT40");
				else
					snprintf(extra, WE_MAX_STR_LEN,
						 "11NGHT20");
			} else if (currBand == eCSR_BAND_5G) {
				if (ch_bond5g)
					snprintf(extra, WE_MAX_STR_LEN,
						 "11NAHT40");
				else
					snprintf(extra, WE_MAX_STR_LEN,
						 "11NAHT20");
			} else {
				snprintf(extra, WE_MAX_STR_LEN, "11N");
			}
			break;
		case eCSR_DOT11_MODE_abg:
			snprintf(extra, WE_MAX_STR_LEN, "11ABG");
			break;
		case eCSR_DOT11_MODE_11a:
			snprintf(extra, WE_MAX_STR_LEN, "11A");
			break;
		case eCSR_DOT11_MODE_11b:
		case eCSR_DOT11_MODE_11b_ONLY:
			snprintf(extra, WE_MAX_STR_LEN, "11B");
			break;
		case eCSR_DOT11_MODE_11g:
		case eCSR_DOT11_MODE_11g_ONLY:
			snprintf(extra, WE_MAX_STR_LEN, "11G");
			break;
		case eCSR_DOT11_MODE_11ac:
		case eCSR_DOT11_MODE_11ac_ONLY:
			if (hddctx->config->vhtChannelWidth ==
			    eHT_CHANNEL_WIDTH_20MHZ)
				snprintf(extra, WE_MAX_STR_LEN,
					 "11ACVHT20");
			else if (hddctx->config->vhtChannelWidth ==
				 eHT_CHANNEL_WIDTH_40MHZ)
				snprintf(extra, WE_MAX_STR_LEN,
					 "11ACVHT40");
			else if (hddctx->config->vhtChannelWidth ==
				 eHT_CHANNEL_WIDTH_80MHZ)
				snprintf(extra, WE_MAX_STR_LEN,
					 "11ACVHT80");
			else if (hddctx->config->vhtChannelWidth ==
				 eHT_CHANNEL_WIDTH_160MHZ)
				snprintf(extra, WE_MAX_STR_LEN,
					 "11ACVHT160");
			break;
		}

		wrqu->data.length = strlen(extra) + 1;
		break;
	}

#ifdef FEATURE_OEM_DATA_SUPPORT
	case WE_GET_OEM_DATA_CAP:
	{
		return iw_get_oem_data_cap(dev, info, wrqu, extra);
	}
#endif /* FEATURE_OEM_DATA_SUPPORT */
	case WE_GET_SNR:
	{
		int8_t s7snr = 0;
		int status = 0;
		hdd_context_t *pHddCtx;
		hdd_station_ctx_t *pHddStaCtx;
		pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
		status = wlan_hdd_validate_context(pHddCtx);
		if (status)
			return status;

		pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
		if (0 == pHddCtx->config->fEnableSNRMonitoring ||
		    eConnectionState_Associated !=
		    pHddStaCtx->conn_info.connState) {
			hddLog(LOGE,
			       "%s: getSNR failed: Enable SNR Monitoring-%d,"
			       " ConnectionState-%d", __func__,
			       pHddCtx->config->fEnableSNRMonitoring,
			       pHddStaCtx->conn_info.connState);
			return -ENONET;
		}
		wlan_hdd_get_snr(pAdapter, &s7snr);
		snprintf(extra, WE_MAX_STR_LEN, "snr=%d", s7snr);
		wrqu->data.length = strlen(extra) + 1;
		break;
	}
	default:
	{
		hddLog(LOGE, "%s: Invalid IOCTL command %d", __func__,
		       sub_cmd);
		break;
	}
	}
	EXIT();
	return 0;
}

static int iw_get_char_setnone(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_char_setnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_setnone_getnone() - Generic "action" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_setnone_getnone(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	int ret;
	int sub_cmd;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

#ifdef CONFIG_COMPAT
	/* this ioctl is a special case where a sub-ioctl is used and both
	 * the number of get and set args is 0.  in this specific case the
	 * logic in iwpriv places the sub_cmd in the data.flags portion of
	 * the iwreq.  unfortunately the location of this field will be
	 * different between 32-bit and 64-bit userspace, and the standard
	 * compat support in the kernel does not handle this case.  so we
	 * need to explicitly handle it here.
	 */
	if (is_compat_task()) {
		struct compat_iw_point *compat_iw_point =
			(struct compat_iw_point *)&wrqu->data;
		sub_cmd = compat_iw_point->flags;
	} else {
		sub_cmd = wrqu->data.flags;
	}
#else
	sub_cmd = wrqu->data.flags;
#endif

	switch (sub_cmd) {
	case WE_GET_RECOVERY_STAT:
	{
		tHalHandle hal = WLAN_HDD_GET_HAL_CTX(adapter);
		sme_get_recovery_stats(hal);
		break;
	}

	case WE_GET_FW_PROFILE_DATA:
		ret = wma_cli_set_command(adapter->sessionId,
				WMI_WLAN_PROFILE_GET_PROFILE_DATA_CMDID,
				0, DBG_CMD);
		break;

	case WE_IBSS_GET_PEER_INFO_ALL:
	{
		hdd_wlan_get_ibss_peer_info_all(adapter);
		break;
	}

	case WE_SET_REASSOC_TRIGGER:
	{
		hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
		tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(adapter);
		uint32_t roamId = 0;
		tCsrRoamModifyProfileFields modProfileFields;
		hdd_station_ctx_t *hdd_sta_ctx =
				   WLAN_HDD_GET_STATION_CTX_PTR(adapter);

		/* Reassoc to same AP, only supported for Open Security*/
		if ((hdd_sta_ctx->conn_info.ucEncryptionType ||
			  hdd_sta_ctx->conn_info.mcEncryptionType)) {
			hddLog(LOGE,
			  FL("Reassoc to same AP, only supported for Open Security"));
			return -ENOTSUPP;
		}

		sme_get_modify_profile_fields(hHal, adapter->sessionId,
					      &modProfileFields);
		sme_roam_reassoc(hHal, adapter->sessionId,
				 NULL, modProfileFields, &roamId, 1);
		return 0;
	}

	case WE_DUMP_AGC_START:
	{
		hddLog(LOG1, "WE_DUMP_AGC_START");
		ret = wma_cli_set_command(adapter->sessionId,
					  GEN_PARAM_DUMP_AGC_START,
					  0, GEN_CMD);
		break;
	}
	case WE_DUMP_AGC:
	{
		hddLog(LOG1, "WE_DUMP_AGC");
		ret = wma_cli_set_command(adapter->sessionId,
					  GEN_PARAM_DUMP_AGC,
					  0, GEN_CMD);
		break;
	}

	case WE_DUMP_CHANINFO_START:
	{
		hddLog(LOG1, "WE_DUMP_CHANINFO_START");
		ret = wma_cli_set_command(adapter->sessionId,
					  GEN_PARAM_DUMP_CHANINFO_START,
					  0, GEN_CMD);
		break;
	}
	case WE_DUMP_CHANINFO:
	{
		hddLog(LOG1, "WE_DUMP_CHANINFO_START");
		ret = wma_cli_set_command(adapter->sessionId,
					  GEN_PARAM_DUMP_CHANINFO,
					  0, GEN_CMD);
		break;
	}
	case WE_DUMP_WATCHDOG:
	{
		hddLog(LOG1, "WE_DUMP_WATCHDOG");
		ret = wma_cli_set_command(adapter->sessionId,
					  GEN_PARAM_DUMP_WATCHDOG,
					  0, GEN_CMD);
		break;
	}
#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
	case WE_DUMP_PCIE_LOG:
	{
		hddLog(LOGE, "WE_DUMP_PCIE_LOG");
		ret = wma_cli_set_command(adapter->sessionId,
					  GEN_PARAM_DUMP_PCIE_ACCESS_LOG,
					  0, GEN_CMD);
		break;
	}
#endif
	case WE_STOP_OBSS_SCAN:
	{
		/*
		 * 1.OBSS Scan is mandatory while operating in 2.4GHz
		 * 2.OBSS scan is stopped by Firmware during the disassociation
		 * 3.OBSS stop comamnd is added for debugging purpose
		 */
		tHalHandle hal;

		hal = WLAN_HDD_GET_HAL_CTX(adapter);
		if (hal == NULL) {
			hdd_err("hal context is NULL");
			return -EINVAL;
		}
		sme_ht40_stop_obss_scan(hal, adapter->sessionId);
	}
	break;
	default:
	{
		hddLog(LOGE, "%s: unknown ioctl %d", __func__, sub_cmd);
		break;
	}
	}
	EXIT();
	return ret;
}

static int iw_setnone_getnone(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_setnone_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __iw_set_var_ints_getnone - Generic "set many" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * This is an SSR-protected generic handler for private ioctls which
 * take multiple arguments.  Note that this implementation is also
 * somewhat unique in that it is shared by both STA-mode and SAP-mode
 * interfaces.
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_var_ints_getnone(struct net_device *dev,
				     struct iw_request_info *info,
				     union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	int sub_cmd;
	int *apps_args = (int *) extra;
	hdd_context_t *hdd_ctx;
	int ret, num_args;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (extra == NULL) {
		hddLog(LOGE, FL("NULL extra buffer pointer"));
		return -EINVAL;
	}

	sub_cmd = wrqu->data.flags;
	num_args = wrqu->data.length;

	hddLog(LOG1, FL("Received length %d"), wrqu->data.length);

	switch (sub_cmd) {
	case WE_IBSS_GET_PEER_INFO:
	{
		pr_info("Station ID = %d\n", apps_args[0]);
		hdd_wlan_get_ibss_peer_info(pAdapter, apps_args[0]);
	}
	break;

	case WE_P2P_NOA_CMD:
	{
		p2p_app_setP2pPs_t p2pNoA;

		if (pAdapter->device_mode != QDF_P2P_GO_MODE) {
			hdd_err("Setting NoA is not allowed in Device mode %s(%d)",
				hdd_device_mode_to_string(
					pAdapter->device_mode),
				pAdapter->device_mode);
			return -EINVAL;
		}

		p2pNoA.opp_ps = apps_args[0];
		p2pNoA.ctWindow = apps_args[1];
		p2pNoA.duration = apps_args[2];
		p2pNoA.interval = apps_args[3];
		p2pNoA.count = apps_args[4];
		p2pNoA.single_noa_duration = apps_args[5];
		p2pNoA.psSelection = apps_args[6];

		hddLog(LOG1,
		       "%s: P2P_NOA_ATTR:oppPS %d ctWindow %d duration %d "
		       "interval %d count %d single noa duration %d PsSelection %x",
		       __func__, apps_args[0], apps_args[1],
		       apps_args[2], apps_args[3], apps_args[4],
		       apps_args[5], apps_args[6]);

		hdd_set_p2p_ps(dev, &p2pNoA);

	}
	break;

	case WE_MTRACE_SELECTIVE_MODULE_LOG_ENABLE_CMD:
	{
		hddLog(LOG1, "%s: SELECTIVE_MODULE_LOG %d arg1 %d arg2",
		       __func__, apps_args[0], apps_args[1]);
		qdf_trace_enable(apps_args[0], apps_args[1]);
	}
	break;

	case WE_MTRACE_DUMP_CMD:
	{
		hddLog(LOG1,
		       "%s: MTRACE_DUMP code %d session %d count %d "
		       "bitmask_of_module %d ", __func__, apps_args[0],
		       apps_args[1], apps_args[2], apps_args[3]);
		qdf_trace_dump_all((void *)hHal, apps_args[0],
				   apps_args[1], apps_args[2],
				   apps_args[3]);

	}
	break;

	case WE_POLICY_MANAGER_CLIST_CMD:
	{
		hddLog(LOGE,
			FL("<iwpriv wlan0 pm_clist> is called\n"));
		cds_incr_connection_count_utfw(apps_args[0],
			apps_args[1], apps_args[2], apps_args[3],
			apps_args[4], apps_args[5], apps_args[6],
			apps_args[7]);
	}
	break;

	case WE_POLICY_MANAGER_DLIST_CMD:
	{
		hddLog(LOGE,
			FL("<iwpriv wlan0 pm_dlist> is called\n"));
		cds_decr_connection_count_utfw(apps_args[0],
			apps_args[1]);
	}
	break;

	case WE_POLICY_MANAGER_ULIST_CMD:
	{
		hddLog(LOGE,
			FL("<iwpriv wlan0 pm_ulist> is called\n"));
		cds_update_connection_info_utfw(apps_args[0],
			apps_args[1], apps_args[2], apps_args[3],
			apps_args[4], apps_args[5], apps_args[6],
			apps_args[7]);
	}
	break;

	case WE_POLICY_MANAGER_DBS_CMD:
	{
		hddLog(LOGE,
			FL("<iwpriv wlan0 pm_dbs> is called\n"));
		if (apps_args[0] == 0)
			wma_set_dbs_capability_ut(0);
		else
			wma_set_dbs_capability_ut(1);

		if (apps_args[1] >= CDS_THROUGHPUT &&
			apps_args[1] <= CDS_LATENCY) {
			pr_info("setting system pref to [%d]\n", apps_args[1]);
			hdd_ctx->config->conc_system_pref = apps_args[1];
		}
	}
	break;

	case WE_POLICY_MANAGER_PCL_CMD:
	{
		uint8_t pcl[MAX_NUM_CHAN] = {0};
		uint8_t weight_list[MAX_NUM_CHAN] = {0};
		uint32_t pcl_len = 0, i = 0;

		hddLog(LOGE,
			FL("<iwpriv wlan0 pm_pcl> is called\n"));

		cds_get_pcl(apps_args[0],
				pcl, &pcl_len,
				weight_list, QDF_ARRAY_SIZE(weight_list));
		pr_info("PCL list for role[%d] is {", apps_args[0]);
		for (i = 0 ; i < pcl_len; i++)
			pr_info(" %d, ", pcl[i]);
		pr_info("}--------->\n");
	}
	break;

	case WE_POLICY_MANAGER_CINFO_CMD:
	{
		struct cds_conc_connection_info *conn_info;
		uint32_t i = 0, len = 0;

		hddLog(LOGE,
			FL("<iwpriv wlan0 pm_cinfo> is called\n"));
		conn_info = cds_get_conn_info(&len);
		pr_info("+-----------------------------+\n");
		for (i = 0; i < len; i++) {
			pr_info("|table_index[%d]\t\t|\n", i);
			pr_info("|\t|vdev_id - %d\t\t|\n", conn_info->vdev_id);
			pr_info("|\t|tx_spatial_stream - %d\t|\n",
						conn_info->tx_spatial_stream);
			pr_info("|\t|rx_spatial_stream - %d\t|\n",
						conn_info->rx_spatial_stream);
			pr_info("|\t|chain_mask - %d\t\t|\n",
						conn_info->chain_mask);
			pr_info("|\t|chan - %d\t\t|\n", conn_info->chan);
			pr_info("|\t|bw - %d\t\t|\n", conn_info->bw);
			pr_info("|\t|mode - %d\t\t|\n", conn_info->mode);
			pr_info("|\t|mac - %d\t\t|\n", conn_info->mac);
			pr_info("|\t|in_use - %d\t\t|\n", conn_info->in_use);
			pr_info("+-----------------------------+\n");
			conn_info++;
		}
	}
	break;

	case WE_POLICY_SET_HW_MODE_CMD:
	{
		if (apps_args[0] == 0) {
			hddLog(LOGE,
				FL("set hw mode for single mac\n"));
			cds_pdev_set_hw_mode(
					pAdapter->sessionId,
					HW_MODE_SS_2x2,
					HW_MODE_80_MHZ,
					HW_MODE_SS_0x0, HW_MODE_BW_NONE,
					HW_MODE_DBS_NONE,
					HW_MODE_AGILE_DFS_NONE,
					SIR_UPDATE_REASON_UT);
		} else if (apps_args[0] == 1) {
			hddLog(LOGE,
				FL("set hw mode for dual mac\n"));
			cds_pdev_set_hw_mode(
					pAdapter->sessionId,
					HW_MODE_SS_1x1,
					HW_MODE_80_MHZ,
					HW_MODE_SS_1x1, HW_MODE_40_MHZ,
					HW_MODE_DBS,
					HW_MODE_AGILE_DFS_NONE,
					SIR_UPDATE_REASON_UT);
		}
	}
	break;

	case WE_POLICY_MANAGER_QUERY_ACTION_CMD:
	{
		enum cds_conc_next_action action;
		hddLog(LOGE,
			FL("<iwpriv wlan0 pm_query_action> is called\n"));
		action = cds_current_connections_update(pAdapter->sessionId,
						apps_args[0],
						SIR_UPDATE_REASON_UT);
		pr_info("next action is %d {HDD_NOP = 0, HDD_DBS, HDD_DBS_DOWNGRADE, HDD_MCC, HDD_MCC_UPGRADE}", action);
	}
	break;
	case WE_POLICY_MANAGER_QUERY_ALLOW_CMD:
	{
		bool allow;
		hddLog(LOGE,
			FL("<iwpriv wlan0 pm_query_allow> is called\n"));
		allow = cds_allow_concurrency(
				apps_args[0], apps_args[1], apps_args[2]);
		pr_info("allow %d {0 = don't allow, 1 = allow}", allow);
	}
	break;

	case WE_POLICY_MANAGER_SCENARIO_CMD:
	{
		clean_report(hdd_ctx);
		if (apps_args[0] == 1) {
			wlan_hdd_one_connection_scenario(hdd_ctx);
		} else if (apps_args[0] == 2) {
			wlan_hdd_two_connections_scenario(hdd_ctx,
				6, CDS_TWO_TWO);
			wlan_hdd_two_connections_scenario(hdd_ctx,
				36, CDS_TWO_TWO);
			wlan_hdd_two_connections_scenario(hdd_ctx,
				6, CDS_ONE_ONE);
			wlan_hdd_two_connections_scenario(hdd_ctx,
				36, CDS_ONE_ONE);
		} else if (apps_args[0] == 3) {
			/* MCC on same band with 2x2 same mac*/
			wlan_hdd_three_connections_scenario(hdd_ctx,
				6, 11, CDS_TWO_TWO, 0);
			/* MCC on diff band with 2x2 same mac*/
			wlan_hdd_three_connections_scenario(hdd_ctx,
				6, 36, CDS_TWO_TWO, 0);
			/* MCC on diff band with 1x1 diff mac */
			wlan_hdd_three_connections_scenario(hdd_ctx,
				36, 6, CDS_ONE_ONE, 0);
			/* MCC on diff band with 1x1 same mac */
			wlan_hdd_three_connections_scenario(hdd_ctx,
				36, 6, CDS_ONE_ONE, 1);
			/* SCC on same band with 2x2 same mac */
			wlan_hdd_three_connections_scenario(hdd_ctx,
				36, 36, CDS_TWO_TWO, 0);
			/* SCC on same band with 1x1 same mac */
			wlan_hdd_three_connections_scenario(hdd_ctx,
				36, 36, CDS_ONE_ONE, 1);
			/* MCC on same band with 2x2 same mac */
			wlan_hdd_three_connections_scenario(hdd_ctx,
				36, 149, CDS_TWO_TWO, 0);
			/* MCC on same band with 1x1 same mac */
			wlan_hdd_three_connections_scenario(hdd_ctx,
				36, 149, CDS_ONE_ONE, 1);
		}
		print_report(hdd_ctx);
	}
	break;

#ifdef FEATURE_WLAN_TDLS
	case WE_TDLS_CONFIG_PARAMS:
	{
		tdls_config_params_t tdlsParams;

		tdlsParams.tdls = apps_args[0];
		tdlsParams.tx_period_t = apps_args[1];
		tdlsParams.tx_packet_n = apps_args[2];
		/* ignore args[3] as discovery_period is not used anymore */
		tdlsParams.discovery_tries_n = apps_args[4];
		/* ignore args[5] as idle_timeout is not used anymore */
		tdlsParams.idle_packet_n = apps_args[6];
		/* ignore args[7] as rssi_hysteresis is not used anymore */
		tdlsParams.rssi_trigger_threshold = apps_args[8];
		tdlsParams.rssi_teardown_threshold = apps_args[9];
		tdlsParams.rssi_delta = apps_args[10];

		wlan_hdd_tdls_set_params(dev, &tdlsParams);
	}
	break;
#endif
	case WE_UNIT_TEST_CMD:
	{
		t_wma_unit_test_cmd *unitTestArgs;
		cds_msg_t msg = { 0 };
		int i, j;
		if ((apps_args[0] < WLAN_MODULE_ID_MIN) ||
		    (apps_args[0] >= WLAN_MODULE_ID_MAX)) {
			hddLog(LOGE, FL("Invalid MODULE ID %d"),
			       apps_args[0]);
			return -EINVAL;
		}
		if (apps_args[1] > (WMA_MAX_NUM_ARGS)) {
			hddLog(LOGE, FL("Too Many args %d"),
			       apps_args[1]);
			return -EINVAL;
		}
		unitTestArgs = qdf_mem_malloc(sizeof(*unitTestArgs));
		if (NULL == unitTestArgs) {
			hddLog(LOGE,
			       FL("qdf_mem_malloc failed for unitTestArgs"));
			return -ENOMEM;
		}
		unitTestArgs->vdev_id = (int)pAdapter->sessionId;
		unitTestArgs->module_id = apps_args[0];
		unitTestArgs->num_args = apps_args[1];
		for (i = 0, j = 2; i < unitTestArgs->num_args; i++, j++) {
			unitTestArgs->args[i] = apps_args[j];
		}
		msg.type = SIR_HAL_UNIT_TEST_CMD;
		msg.reserved = 0;
		msg.bodyptr = unitTestArgs;
		if (QDF_STATUS_SUCCESS !=
		    cds_mq_post_message(QDF_MODULE_ID_WMA, &msg)) {
			qdf_mem_free(unitTestArgs);
			QDF_TRACE(QDF_MODULE_ID_HDD,
				  QDF_TRACE_LEVEL_ERROR,
				  FL
					  ("Not able to post UNIT_TEST_CMD message to WMA"));
			return -EINVAL;
		}
	}
	break;
#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
	case WE_LED_FLASHING_PARAM:
	{
		int i;
		if (num_args != 4) {
			hddLog(LOGE,
			       FL("gpio_control: 4 parameters are required"));
			return -EINVAL;
		}
		for (i = 0; i < num_args; i++) {
			if (apps_args[i] >= 0x7fffffff) {
				hddLog(LOGE,
				       FL("gpio_control: parameter should be less than 0x7fffffff"));
				return -EINVAL;
			}
		}
		sme_set_led_flashing(WLAN_HDD_GET_HAL_CTX(pAdapter),
				     0, apps_args[0], apps_args[1]);
		sme_set_led_flashing(WLAN_HDD_GET_HAL_CTX(pAdapter),
				     1, apps_args[2], apps_args[3]);
	}
	break;
#endif
	default:
	{
		hddLog(LOGE, FL("Invalid IOCTL command %d"), sub_cmd);
	}
	break;
	}
	EXIT();
	return 0;
}

/**
 * iw_hdd_set_var_ints_getnone() - set var ints getnone callback
 * @dev: pointer to net_device structure
 * @info: pointer to iw_request_info structure
 * @wrqu: pointer to iwreq_data
 * @extra; extra
 *
 * Return: 0 on success, error number otherwise
 *
 */
static int iw_hdd_set_var_ints_getnone(struct net_device *dev,
				       struct iw_request_info *info,
				       union iwreq_data *wrqu, char *extra)
{
	union iwreq_data u_priv_wrqu;
	int apps_args[MAX_VAR_ARGS] = {0};
	int ret, num_args;

	if (!capable(CAP_NET_ADMIN)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			FL("permission check failed"));
		return -EPERM;
	}

	/* Helper function to get iwreq_data with compat handling. */
	if (hdd_priv_get_data(&u_priv_wrqu.data, wrqu))
		return -EINVAL;

	if (NULL == u_priv_wrqu.data.pointer) {
		hddLog(LOGE, FL("NULL data pointer"));
		return -EINVAL;
	}

	num_args = u_priv_wrqu.data.length;
	if (num_args > MAX_VAR_ARGS)
		num_args = MAX_VAR_ARGS;

	if (copy_from_user(apps_args, u_priv_wrqu.data.pointer,
			  (sizeof(int)) * num_args)) {
		hddLog(LOGE, FL("failed to copy data from user buffer"));
		return -EFAULT;
	}

	cds_ssr_protect(__func__);
	ret = __iw_set_var_ints_getnone(dev, info, &u_priv_wrqu,
					(char *)&apps_args);
	cds_ssr_unprotect(__func__);
	return ret;
}

/**
 * iw_set_var_ints_getnone - Generic "set many" private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * This is a generic handler for private ioctls which take multiple
 * arguments.  Note that this implementation is also somewhat unique
 * in that it is shared by both STA-mode and SAP-mode interfaces.
 *
 * Return: 0 on success, non-zero on error
 */
int iw_set_var_ints_getnone(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_var_ints_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);
	return ret;
}

/**
 * iw_add_tspec - Add TSpec private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_add_tspec(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	hdd_wlan_wmm_status_e *pStatus = (hdd_wlan_wmm_status_e *) extra;
	int params[HDD_WLAN_WMM_PARAM_COUNT];
	sme_QosWmmTspecInfo tSpec;
	uint32_t handle;
	struct iw_point s_priv_data;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/* make sure the application is sufficiently priviledged */
	/* note that the kernel will do this for "set" ioctls, but since */
	/* this ioctl wants to return status to user space it must be */
	/* defined as a "get" ioctl */
	if (!capable(CAP_NET_ADMIN)) {
		return -EPERM;
	}

	/* we must be associated in order to add a tspec */
	if (eConnectionState_Associated != pHddStaCtx->conn_info.connState) {
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}
	/* since we are defined to be a "get" ioctl, and since the number */
	/* of params exceeds the number of params that wireless extensions */
	/* will pass down in the iwreq_data, we must copy the "set" params. */
	/* We must handle the compat for iwreq_data in 32U/64K environment. */

	/* helper function to get iwreq_data with compat handling. */
	if (hdd_priv_get_data(&s_priv_data, wrqu)) {
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}
	/* make sure all params are correctly passed to function */
	if ((NULL == s_priv_data.pointer) ||
	    (HDD_WLAN_WMM_PARAM_COUNT != s_priv_data.length)) {
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}
	/* from user space ourselves */
	if (copy_from_user(&params, s_priv_data.pointer, sizeof(params))) {
		/* hmmm, can't get them */
		return -EIO;
	}
	/* clear the tspec */
	memset(&tSpec, 0, sizeof(tSpec));

	/* validate the handle */
	handle = params[HDD_WLAN_WMM_PARAM_HANDLE];
	if (HDD_WMM_HANDLE_IMPLICIT == handle) {
		/* that one is reserved */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}
	/* validate the TID */
	if (params[HDD_WLAN_WMM_PARAM_TID] > 7) {
		/* out of range */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}
	tSpec.ts_info.tid = params[HDD_WLAN_WMM_PARAM_TID];

	/* validate the direction */
	switch (params[HDD_WLAN_WMM_PARAM_DIRECTION]) {
	case HDD_WLAN_WMM_DIRECTION_UPSTREAM:
		tSpec.ts_info.direction = SME_QOS_WMM_TS_DIR_UPLINK;
		break;

	case HDD_WLAN_WMM_DIRECTION_DOWNSTREAM:
		tSpec.ts_info.direction = SME_QOS_WMM_TS_DIR_DOWNLINK;
		break;

	case HDD_WLAN_WMM_DIRECTION_BIDIRECTIONAL:
		tSpec.ts_info.direction = SME_QOS_WMM_TS_DIR_BOTH;
		break;

	default:
		/* unknown */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}

	tSpec.ts_info.psb = params[HDD_WLAN_WMM_PARAM_APSD];

	/* validate the user priority */
	if (params[HDD_WLAN_WMM_PARAM_USER_PRIORITY] >= SME_QOS_WMM_UP_MAX) {
		/* out of range */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}
	tSpec.ts_info.up = params[HDD_WLAN_WMM_PARAM_USER_PRIORITY];
	if (0 > tSpec.ts_info.up || SME_QOS_WMM_UP_MAX < tSpec.ts_info.up) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "***ts_info.up out of bounds***");
		return 0;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s:TS_INFO PSB %d UP %d !!!", __func__,
		  tSpec.ts_info.psb, tSpec.ts_info.up);

	tSpec.nominal_msdu_size = params[HDD_WLAN_WMM_PARAM_NOMINAL_MSDU_SIZE];
	tSpec.maximum_msdu_size = params[HDD_WLAN_WMM_PARAM_MAXIMUM_MSDU_SIZE];
	tSpec.min_data_rate = params[HDD_WLAN_WMM_PARAM_MINIMUM_DATA_RATE];
	tSpec.mean_data_rate = params[HDD_WLAN_WMM_PARAM_MEAN_DATA_RATE];
	tSpec.peak_data_rate = params[HDD_WLAN_WMM_PARAM_PEAK_DATA_RATE];
	tSpec.max_burst_size = params[HDD_WLAN_WMM_PARAM_MAX_BURST_SIZE];
	tSpec.min_phy_rate = params[HDD_WLAN_WMM_PARAM_MINIMUM_PHY_RATE];
	tSpec.surplus_bw_allowance =
		params[HDD_WLAN_WMM_PARAM_SURPLUS_BANDWIDTH_ALLOWANCE];
	tSpec.min_service_interval =
		params[HDD_WLAN_WMM_PARAM_SERVICE_INTERVAL];
	tSpec.max_service_interval =
		params[HDD_WLAN_WMM_PARAM_MAX_SERVICE_INTERVAL];
	tSpec.suspension_interval =
		params[HDD_WLAN_WMM_PARAM_SUSPENSION_INTERVAL];
	tSpec.inactivity_interval =
		params[HDD_WLAN_WMM_PARAM_INACTIVITY_INTERVAL];

	tSpec.ts_info.burst_size_defn =
		params[HDD_WLAN_WMM_PARAM_BURST_SIZE_DEFN];

	/* validate the ts info ack policy */
	switch (params[HDD_WLAN_WMM_PARAM_ACK_POLICY]) {
	case HDD_WLAN_WMM_TS_INFO_ACK_POLICY_NORMAL_ACK:
		tSpec.ts_info.ack_policy = SME_QOS_WMM_TS_ACK_POLICY_NORMAL_ACK;
		break;

	case HDD_WLAN_WMM_TS_INFO_ACK_POLICY_HT_IMMEDIATE_BLOCK_ACK:
		tSpec.ts_info.ack_policy =
			SME_QOS_WMM_TS_ACK_POLICY_HT_IMMEDIATE_BLOCK_ACK;
		break;

	default:
		/* unknown */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}

	*pStatus = hdd_wmm_addts(pAdapter, handle, &tSpec);
	EXIT();
	return 0;
}

static int iw_add_tspec(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_add_tspec(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_del_tspec - Delete TSpec private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_del_tspec(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	int *params = (int *)extra;
	hdd_wlan_wmm_status_e *pStatus = (hdd_wlan_wmm_status_e *) extra;
	uint32_t handle;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/* make sure the application is sufficiently priviledged */
	/* note that the kernel will do this for "set" ioctls, but since */
	/* this ioctl wants to return status to user space it must be */
	/* defined as a "get" ioctl */
	if (!capable(CAP_NET_ADMIN)) {
		return -EPERM;
	}

	/* although we are defined to be a "get" ioctl, the params we require */
	/* will fit in the iwreq_data, therefore unlike iw_add_tspec() there */
	/* is no need to copy the params from user space */

	/* validate the handle */
	handle = params[HDD_WLAN_WMM_PARAM_HANDLE];
	if (HDD_WMM_HANDLE_IMPLICIT == handle) {
		/* that one is reserved */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}

	*pStatus = hdd_wmm_delts(pAdapter, handle);
	EXIT();
	return 0;
}

static int iw_del_tspec(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_del_tspec(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_get_tspec - Get TSpec private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_get_tspec(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	int *params = (int *)extra;
	hdd_wlan_wmm_status_e *pStatus = (hdd_wlan_wmm_status_e *) extra;
	uint32_t handle;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	/* although we are defined to be a "get" ioctl, the params we require */
	/* will fit in the iwreq_data, therefore unlike iw_add_tspec() there */
	/* is no need to copy the params from user space */

	/* validate the handle */
	handle = params[HDD_WLAN_WMM_PARAM_HANDLE];
	if (HDD_WMM_HANDLE_IMPLICIT == handle) {
		/* that one is reserved */
		*pStatus = HDD_WLAN_WMM_STATUS_SETUP_FAILED_BAD_PARAM;
		return 0;
	}

	*pStatus = hdd_wmm_checkts(pAdapter, handle);
	EXIT();
	return 0;
}

static int iw_get_tspec(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_tspec(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_set_fties - Set FT IEs private ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Each time the supplicant has the auth_request or reassoc request
 * IEs ready they are pushed to the driver. The driver will in turn
 * use it to send out the auth req and reassoc req for 11r FT Assoc.
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_fties(struct net_device *dev, struct iw_request_info *info,
			  union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (!wrqu->data.length) {
		hddLog(LOGE, FL("called with 0 length IEs"));
		return -EINVAL;
	}
	if (wrqu->data.pointer == NULL) {
		hddLog(LOGE, FL("called with NULL IE"));
		return -EINVAL;
	}
	/* Added for debug on reception of Re-assoc Req. */
	if (eConnectionState_Associated != pHddStaCtx->conn_info.connState) {
		hddLog(LOGE,
		       FL("Called with Ie of length = %d when not associated"),
		       wrqu->data.length);
		hddLog(LOGE, FL("Should be Re-assoc Req IEs"));
	}
	hddLog(LOG1, FL("%s called with Ie of length = %d"), __func__,
	       wrqu->data.length);

	/* Pass the received FT IEs to SME */
	sme_set_ft_ies(WLAN_HDD_GET_HAL_CTX(pAdapter), pAdapter->sessionId,
			extra, wrqu->data.length);
	EXIT();
	return 0;
}

static int iw_set_fties(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_fties(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_set_host_offload - Set host offload ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_host_offload(struct net_device *dev,
				 struct iw_request_info *info,
				 union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tpHostOffloadRequest pRequest = (tpHostOffloadRequest) extra;
	tSirHostOffloadReq offloadRequest;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (!hdd_conn_is_connected(WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_FATAL,
			  "%s:LOGP dev is not in CONNECTED state, ignore!!!",
			  __func__);
		return -EINVAL;
	}

	/* Debug display of request components. */
	switch (pRequest->offloadType) {
	case WLAN_IPV4_ARP_REPLY_OFFLOAD:
		hddLog(QDF_TRACE_LEVEL_WARN,
		       "%s: Host offload request: ARP reply", __func__);
		switch (pRequest->enableOrDisable) {
		case WLAN_OFFLOAD_DISABLE:
			hddLog(QDF_TRACE_LEVEL_WARN, "   disable");
			break;
		case WLAN_OFFLOAD_ARP_AND_BC_FILTER_ENABLE:
			hddLog(QDF_TRACE_LEVEL_WARN, "   BC Filtering enable");
		case WLAN_OFFLOAD_ENABLE:
			hddLog(QDF_TRACE_LEVEL_WARN, "   ARP offload enable");
			hddLog(QDF_TRACE_LEVEL_WARN,
			       "   IP address: %d.%d.%d.%d",
			       pRequest->params.hostIpv4Addr[0],
			       pRequest->params.hostIpv4Addr[1],
			       pRequest->params.hostIpv4Addr[2],
			       pRequest->params.hostIpv4Addr[3]);
		}
		break;

	case WLAN_IPV6_NEIGHBOR_DISCOVERY_OFFLOAD:
		hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
		       "%s: Host offload request: neighbor discovery",
		       __func__);
		switch (pRequest->enableOrDisable) {
		case WLAN_OFFLOAD_DISABLE:
			hddLog(QDF_TRACE_LEVEL_INFO_HIGH, "   disable");
			break;
		case WLAN_OFFLOAD_ENABLE:
			hddLog(QDF_TRACE_LEVEL_INFO_HIGH, "   enable");
			hddLog(QDF_TRACE_LEVEL_INFO_HIGH,
			       "   IP address: %x:%x:%x:%x:%x:%x:%x:%x",
			       *(uint16_t *) (pRequest->params.hostIpv6Addr),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      2),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      4),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      6),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      8),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      10),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      12),
			       *(uint16_t *) (pRequest->params.hostIpv6Addr +
					      14));
		}
	}

	/* Execute offload request. The reason that we can copy the
	 * request information from the ioctl structure to the SME
	 * structure is that they are laid out exactly the same.
	 * Otherwise, each piece of information would have to be
	 * copied individually.
	 */
	memcpy(&offloadRequest, pRequest, wrqu->data.length);
	if (QDF_STATUS_SUCCESS !=
	    sme_set_host_offload(WLAN_HDD_GET_HAL_CTX(pAdapter),
				 pAdapter->sessionId, &offloadRequest)) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       "%s: Failure to execute host offload request", __func__);
		return -EINVAL;
	}
	EXIT();
	return 0;
}

static int iw_set_host_offload(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_host_offload(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * iw_set_keepalive_params - Set keepalive params ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_keepalive_params(struct net_device *dev,
				     struct iw_request_info *info,
				     union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tpSirKeepAliveReq request = (tpSirKeepAliveReq) extra;
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (wrqu->data.length != sizeof(*request)) {
		hdd_err("Invalid length %d", wrqu->data.length);
		return -EINVAL;
	}

	if (request->timePeriod > WNI_CFG_INFRA_STA_KEEP_ALIVE_PERIOD_STAMAX) {
		hdd_err("Value of timePeriod %d exceed Max limit %d",
			request->timePeriod,
			WNI_CFG_INFRA_STA_KEEP_ALIVE_PERIOD_STAMAX);
		return -EINVAL;
	}

	/* Debug display of request components. */
	hdd_info("Set Keep Alive Request : TimePeriod %d size %zu",
		request->timePeriod, sizeof(tSirKeepAliveReq));

	switch (request->packetType) {
	case WLAN_KEEP_ALIVE_NULL_PKT:
		hdd_info("Keep Alive Request: Tx NULL");
		break;

	case WLAN_KEEP_ALIVE_UNSOLICIT_ARP_RSP:
		hdd_info("Keep Alive Request: Tx UnSolicited ARP RSP");

		hdd_info("Host IP address: %d.%d.%d.%d",
		       request->hostIpv4Addr[0], request->hostIpv4Addr[1],
		       request->hostIpv4Addr[2], request->hostIpv4Addr[3]);

		hdd_info("Dest IP address: %d.%d.%d.%d",
		       request->destIpv4Addr[0], request->destIpv4Addr[1],
		       request->destIpv4Addr[2], request->destIpv4Addr[3]);

		hdd_info("Dest MAC address: "MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(request->dest_macaddr.bytes));
		break;
	}

	hdd_info("Keep alive period  %d", request->timePeriod);

	if (QDF_STATUS_SUCCESS !=
	    sme_set_keep_alive(WLAN_HDD_GET_HAL_CTX(pAdapter),
		pAdapter->sessionId, request)) {
		hdd_err("Failure to execute Keep Alive");
		return -EINVAL;
	}
	EXIT();
	return 0;
}

static int iw_set_keepalive_params(struct net_device *dev,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu,
				   char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_keepalive_params(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef WLAN_FEATURE_PACKET_FILTERING
/**
 * wlan_hdd_set_filter() - Set packet filter
 * @hdd_ctx: Global HDD context
 * @request: Packet filter request struct
 * @sessionId: Target session for the request
 *
 * Return: 0 on success, non-zero on error
 */
static int wlan_hdd_set_filter(hdd_context_t *hdd_ctx,
				struct pkt_filter_cfg *request,
				uint8_t sessionId)
{
	tSirRcvPktFilterCfgType packetFilterSetReq = {0};
	tSirRcvFltPktClearParam packetFilterClrReq = {0};
	int i = 0;

	if (hdd_ctx->config->disablePacketFilter) {
		hdd_err("packet filtering disabled in ini returning");
		return 0;
	}

	/* Debug display of request components. */
	hdd_info("Packet Filter Request : FA %d params %d",
		request->filter_action, request->num_params);

	switch (request->filter_action) {
	case HDD_RCV_FILTER_SET:
		hdd_info("Set Packet Filter Request for Id: %d",
			request->filter_id);

		packetFilterSetReq.filterId = request->filter_id;
		if (request->num_params >= HDD_MAX_CMP_PER_PACKET_FILTER) {
			hdd_err("Number of Params exceed Max limit %d",
				request->num_params);
			return -EINVAL;
		}
		packetFilterSetReq.numFieldParams = request->num_params;
		packetFilterSetReq.coalesceTime = 0;
		packetFilterSetReq.filterType = HDD_RCV_FILTER_SET;
		for (i = 0; i < request->num_params; i++) {
			packetFilterSetReq.paramsData[i].protocolLayer =
				request->params_data[i].protocol_layer;
			packetFilterSetReq.paramsData[i].cmpFlag =
				request->params_data[i].compare_flag;
			packetFilterSetReq.paramsData[i].dataOffset =
				request->params_data[i].data_offset;
			packetFilterSetReq.paramsData[i].dataLength =
				request->params_data[i].data_length;
			packetFilterSetReq.paramsData[i].reserved = 0;

			if (request->params_data[i].data_length >
				SIR_MAX_FILTER_TEST_DATA_LEN) {
				hdd_err("Error invalid data length %d",
					request->params_data[i].data_length);
				return -EINVAL;
			}

			hdd_info("Proto %d Comp Flag %d Filter Type %d",
				request->params_data[i].protocol_layer,
				request->params_data[i].compare_flag,
				packetFilterSetReq.filterType);

			hdd_info("Data Offset %d Data Len %d",
				request->params_data[i].data_offset,
				request->params_data[i].data_length);

			if (sizeof(packetFilterSetReq.paramsData[i].compareData)
				< (request->params_data[i].data_length)) {
				hdd_err("Error invalid data length %d",
					request->params_data[i].data_length);
				return -EINVAL;
			}

			memcpy(&packetFilterSetReq.paramsData[i].compareData,
			       request->params_data[i].compare_data,
			       request->params_data[i].data_length);
			memcpy(&packetFilterSetReq.paramsData[i].dataMask,
			       request->params_data[i].data_mask,
			       request->params_data[i].data_length);

			hdd_info("CData %d CData %d CData %d CData %d CData %d CData %d",
				request->params_data[i].compare_data[0],
				request->params_data[i].compare_data[1],
				request->params_data[i].compare_data[2],
				request->params_data[i].compare_data[3],
				request->params_data[i].compare_data[4],
				request->params_data[i].compare_data[5]);

			hdd_info("MData %d MData %d MData %d MData %d MData %d MData %d",
				request->params_data[i].data_mask[0],
				request->params_data[i].data_mask[1],
				request->params_data[i].data_mask[2],
				request->params_data[i].data_mask[3],
				request->params_data[i].data_mask[4],
				request->params_data[i].data_mask[5]);
		}

		if (QDF_STATUS_SUCCESS !=
			sme_receive_filter_set_filter(hdd_ctx->hHal,
				&packetFilterSetReq,
				sessionId)) {
			hdd_err("Failure to execute Set Filter");
			return -EINVAL;
		}

		break;

	case HDD_RCV_FILTER_CLEAR:

		hdd_info("Clear Packet Filter Request for Id: %d",
			request->filter_id);
		packetFilterClrReq.filterId = request->filter_id;
		if (QDF_STATUS_SUCCESS !=
		    sme_receive_filter_clear_filter(hdd_ctx->hHal,
						    &packetFilterClrReq,
						    sessionId)) {
			hdd_err("Failure to execute Clear Filter");
			return -EINVAL;
		}
		break;

	default:
		hdd_err("Packet Filter Request: Invalid %d",
		       request->filter_action);
		return -EINVAL;
	}
	return 0;
}

/**
 * __iw_set_packet_filter_params() - set packet filter parameters in target
 * @dev: Pointer to netdev
 * @info: Pointer to iw request info
 * @wrqu: Pointer to data
 * @extra: Pointer to extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int __iw_set_packet_filter_params(struct net_device *dev,
				       struct iw_request_info *info,
				       union iwreq_data *wrqu, char *extra)
{
	int ret;
	hdd_context_t *hdd_ctx;
	struct iw_point priv_data;
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct pkt_filter_cfg *request = NULL;

	if (!capable(CAP_NET_ADMIN)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			FL("permission check failed"));
		return -EPERM;
	}

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (hdd_priv_get_data(&priv_data, wrqu)) {
		hdd_err("failed to get priv data");
		return -EINVAL;
	}

	if ((NULL == priv_data.pointer) || (0 == priv_data.length)) {
		hdd_err("invalid priv data %p or invalid priv data length %d",
			priv_data.pointer, priv_data.length);
		return -EINVAL;
	}

	/* copy data using copy_from_user */
	request = mem_alloc_copy_from_user_helper(priv_data.pointer,
						   priv_data.length);
	if (NULL == request) {
		hdd_err("mem_alloc_copy_from_user_helper fail");
		return -ENOMEM;
	}

	ret = wlan_hdd_set_filter(hdd_ctx, request, adapter->sessionId);

	kfree(request);
	EXIT();
	return ret;
}

/**
 * iw_set_packet_filter_params() - set packet filter parameters in target
 * @dev: Pointer to netdev
 * @info: Pointer to iw request info
 * @wrqu: Pointer to data
 * @extra: Pointer to extra data
 *
 * Return: 0 on success, non-zero on error
 */
static int iw_set_packet_filter_params(struct net_device *dev,
				       struct iw_request_info *info,
				       union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_packet_filter_params(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif


static int __iw_get_statistics(struct net_device *dev,
			       struct iw_request_info *info,
			       union iwreq_data *wrqu, char *extra)
{

	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	hdd_wext_state_t *pWextState;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	char *p = extra;
	int tlen = 0;
	tCsrSummaryStatsInfo *pStats = &(pAdapter->hdd_stats.summary_stat);
	tCsrGlobalClassAStatsInfo *aStats = &(pAdapter->hdd_stats.ClassA_stat);
	tCsrGlobalClassDStatsInfo *dStats = &(pAdapter->hdd_stats.ClassD_stat);
	int ret;

	ENTER_DEV(dev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (eConnectionState_Associated !=
	    (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter))->conn_info.connState) {

		wrqu->txpower.value = 0;
	} else {
		status = sme_get_statistics(hdd_ctx->hHal, eCSR_HDD,
					    SME_SUMMARY_STATS |
					    SME_GLOBAL_CLASSA_STATS |
					    SME_GLOBAL_CLASSB_STATS |
					    SME_GLOBAL_CLASSC_STATS |
					    SME_GLOBAL_CLASSD_STATS |
					    SME_PER_STA_STATS,
					    hdd_statistics_cb, 0, false,
					    (WLAN_HDD_GET_STATION_CTX_PTR
						     (pAdapter))->conn_info.staId[0],
					    pAdapter, pAdapter->sessionId);

		if (QDF_STATUS_SUCCESS != status) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: Unable to retrieve SME statistics",
			       __func__);
			return -EINVAL;
		}

		pWextState = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

		qdf_status =
			qdf_wait_single_event(&pWextState->hdd_qdf_event,
					      WLAN_WAIT_TIME_STATS);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "%s: SME timeout while retrieving statistics",
			       __func__);
			/*Remove the SME statistics list by passing NULL in callback argument */
			status = sme_get_statistics(hdd_ctx->hHal, eCSR_HDD,
						    SME_SUMMARY_STATS |
						    SME_GLOBAL_CLASSA_STATS |
						    SME_GLOBAL_CLASSB_STATS |
						    SME_GLOBAL_CLASSC_STATS |
						    SME_GLOBAL_CLASSD_STATS |
						    SME_PER_STA_STATS,
						    NULL, 0, false,
						    (WLAN_HDD_GET_STATION_CTX_PTR
							     (pAdapter))->conn_info.
						    staId[0], pAdapter,
						    pAdapter->sessionId);

			return -EINVAL;
		}
		FILL_TLV(p, (uint8_t) WLAN_STATS_RETRY_CNT,
			 (uint8_t) sizeof(pStats->retry_cnt),
			 (char *)&(pStats->retry_cnt[0]), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_MUL_RETRY_CNT,
			 (uint8_t) sizeof(pStats->multiple_retry_cnt),
			 (char *)&(pStats->multiple_retry_cnt[0]), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_TX_FRM_CNT,
			 (uint8_t) sizeof(pStats->tx_frm_cnt),
			 (char *)&(pStats->tx_frm_cnt[0]), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_RX_FRM_CNT,
			 (uint8_t) sizeof(pStats->rx_frm_cnt),
			 (char *)&(pStats->rx_frm_cnt), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_FRM_DUP_CNT,
			 (uint8_t) sizeof(pStats->frm_dup_cnt),
			 (char *)&(pStats->frm_dup_cnt), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_FAIL_CNT,
			 (uint8_t) sizeof(pStats->fail_cnt),
			 (char *)&(pStats->fail_cnt[0]), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_RTS_FAIL_CNT,
			 (uint8_t) sizeof(pStats->rts_fail_cnt),
			 (char *)&(pStats->rts_fail_cnt), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_ACK_FAIL_CNT,
			 (uint8_t) sizeof(pStats->ack_fail_cnt),
			 (char *)&(pStats->ack_fail_cnt), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_RTS_SUC_CNT,
			 (uint8_t) sizeof(pStats->rts_succ_cnt),
			 (char *)&(pStats->rts_succ_cnt), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_RX_DISCARD_CNT,
			 (uint8_t) sizeof(pStats->rx_discard_cnt),
			 (char *)&(pStats->rx_discard_cnt), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_RX_ERROR_CNT,
			 (uint8_t) sizeof(pStats->rx_error_cnt),
			 (char *)&(pStats->rx_error_cnt), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_TX_BYTE_CNT,
			 (uint8_t) sizeof(dStats->tx_uc_byte_cnt[0]),
			 (char *)&(dStats->tx_uc_byte_cnt[0]), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_RX_BYTE_CNT,
			 (uint8_t) sizeof(dStats->rx_byte_cnt),
			 (char *)&(dStats->rx_byte_cnt), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_RX_RATE,
			 (uint8_t) sizeof(dStats->rx_rate),
			 (char *)&(dStats->rx_rate), tlen);

		/* Transmit rate, in units of 500 kbit/sec */
		FILL_TLV(p, (uint8_t) WLAN_STATS_TX_RATE,
			 (uint8_t) sizeof(aStats->tx_rate),
			 (char *)&(aStats->tx_rate), tlen);

		FILL_TLV(p, (uint8_t) WLAN_STATS_RX_UC_BYTE_CNT,
			 (uint8_t) sizeof(dStats->rx_uc_byte_cnt[0]),
			 (char *)&(dStats->rx_uc_byte_cnt[0]), tlen);
		FILL_TLV(p, (uint8_t) WLAN_STATS_RX_MC_BYTE_CNT,
			 (uint8_t) sizeof(dStats->rx_mc_byte_cnt),
			 (char *)&(dStats->rx_mc_byte_cnt), tlen);
		FILL_TLV(p, (uint8_t) WLAN_STATS_RX_BC_BYTE_CNT,
			 (uint8_t) sizeof(dStats->rx_bc_byte_cnt),
			 (char *)&(dStats->rx_bc_byte_cnt), tlen);
		FILL_TLV(p, (uint8_t) WLAN_STATS_TX_UC_BYTE_CNT,
			 (uint8_t) sizeof(dStats->tx_uc_byte_cnt[0]),
			 (char *)&(dStats->tx_uc_byte_cnt[0]), tlen);
		FILL_TLV(p, (uint8_t) WLAN_STATS_TX_MC_BYTE_CNT,
			 (uint8_t) sizeof(dStats->tx_mc_byte_cnt),
			 (char *)&(dStats->tx_mc_byte_cnt), tlen);
		FILL_TLV(p, (uint8_t) WLAN_STATS_TX_BC_BYTE_CNT,
			 (uint8_t) sizeof(dStats->tx_bc_byte_cnt),
			 (char *)&(dStats->tx_bc_byte_cnt), tlen);

		wrqu->data.length = tlen;

	}

	EXIT();

	return 0;
}

static int iw_get_statistics(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_get_statistics(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef FEATURE_WLAN_SCAN_PNO

/*Max Len for PNO notification*/
#define MAX_PNO_NOTIFY_LEN 100
void found_pref_network_cb(void *callbackContext,
			   tSirPrefNetworkFoundInd *pPrefNetworkFoundInd)
{
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) callbackContext;
	union iwreq_data wrqu;
	char buf[MAX_PNO_NOTIFY_LEN + 1];

	hddLog(QDF_TRACE_LEVEL_WARN,
	       "A preferred network was found: %s with rssi: -%d",
	       pPrefNetworkFoundInd->ssId.ssId, pPrefNetworkFoundInd->rssi);

	/* create the event */
	memset(&wrqu, 0, sizeof(wrqu));
	memset(buf, 0, sizeof(buf));

	snprintf(buf, MAX_PNO_NOTIFY_LEN,
		 "QCOM: Found preferred network: %s with RSSI of -%u",
		 pPrefNetworkFoundInd->ssId.ssId,
		 (unsigned int)pPrefNetworkFoundInd->rssi);

	wrqu.data.pointer = buf;
	wrqu.data.length = strlen(buf);

	/* send the event */

	wireless_send_event(pAdapter->dev, IWEVCUSTOM, &wrqu, buf);

}

/**
 * __iw_set_pno() - Preferred Network Offload ioctl handler
 * @dev: device upon which the ioctl was received
 * @info: ioctl request information
 * @wrqu: ioctl request data
 * @extra: ioctl extra data
 *
 * This function parses a Preferred Network Offload command
 * Input is string based and expected to be of the form:
 *
 * <enable(1) | disable(0)>
 * when enabling:
 * <number of networks>
 * for each network:
 *    <ssid_len> <ssid> <authentication> <encryption>
 *    <ch_num> <channel_list optional> <bcast_type> <rssi_threshold>
 * <number of scan timers>
 * for each timer:
 *    <scan_time> <scan_repeat>
 * <suspend mode>
 *
 * e.g:
 * 1 2 4 test 0 0 3 1 6 11 2 40 5 test2 4 4 6 1 2 3 4 5 6 1 0 2 5 2 300 0 1
 *
 * this translates into:
 * -----------------------------
 * enable PNO
 * 2 networks
 * Network 1:
 *   test - with authentication type 0 and encryption type 0,
 *   search on 3 channels: 1 6 and 11,
 *   SSID bcast type is unknown (directed probe will be sent if
 *   AP not found) and must meet -40dBm RSSI
 * Network 2:
 *   test2 - with authentication type 4 and encryption type 4,
 *   search on 6 channels 1, 2, 3, 4, 5 and 6
 *   bcast type is non-bcast (directed probe will be sent)
 *   and must not meet any RSSI threshold
 * 2 scan timers:
 *   scan every 5 seconds 2 times
 *   then scan every 300 seconds until stopped
 * enable on suspend
 */
static int __iw_set_pno(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	int ret;
	int offset;
	char *ptr;
	uint8_t i, j, params, mode;

	/* request is a large struct, so we make it static to avoid
	 * stack overflow.  This API is only invoked via ioctl, so it
	 * is serialized by the kernel rtnl_lock and hence does not
	 * need to be reentrant
	 */
	static tSirPNOScanReq request;

	ENTER_DEV(dev);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (ret)
		return ret;

	hdd_notice("PNO data len %d data %s", wrqu->data.length, extra);

	request.enable = 0;
	request.ucNetworksCount = 0;

	ptr = extra;

	if (1 != sscanf(ptr, "%hhu%n", &(request.enable), &offset)) {
		hdd_err("PNO enable input is not valid %s", ptr);
		return -EINVAL;
	}

	if (0 == request.enable) {
		/* Disable PNO, ignore any other params */
		memset(&request, 0, sizeof(request));
		sme_set_preferred_network_list(WLAN_HDD_GET_HAL_CTX(adapter),
					       &request, adapter->sessionId,
					       found_pref_network_cb, adapter);
		return 0;
	}

	ptr += offset;

	if (1 !=
	    sscanf(ptr, "%hhu %n", &(request.ucNetworksCount), &offset)) {
		hdd_err("PNO count input not valid %s", ptr);
		return -EINVAL;

	}

	hdd_info("PNO enable %d networks count %d offset %d",
		 request.enable, request.ucNetworksCount, offset);

	if ((0 == request.ucNetworksCount) ||
	    (request.ucNetworksCount > SIR_PNO_MAX_SUPP_NETWORKS)) {
		hdd_err("Network count %d invalid",
			request.ucNetworksCount);
		return -EINVAL;
	}

	ptr += offset;

	for (i = 0; i < request.ucNetworksCount; i++) {

		request.aNetworks[i].ssId.length = 0;

		params = sscanf(ptr, "%hhu %n",
				  &(request.aNetworks[i].ssId.length),
				  &offset);

		if (1 != params) {
			hdd_err("PNO ssid length input is not valid %s", ptr);
			return -EINVAL;
		}

		if ((0 == request.aNetworks[i].ssId.length) ||
		    (request.aNetworks[i].ssId.length > 32)) {
			hdd_err("SSID Len %d is not correct for network %d",
				  request.aNetworks[i].ssId.length, i);
			return -EINVAL;
		}

		/* Advance to SSID */
		ptr += offset;

		memcpy(request.aNetworks[i].ssId.ssId, ptr,
		       request.aNetworks[i].ssId.length);
		ptr += request.aNetworks[i].ssId.length;

		params = sscanf(ptr, "%u %u %hhu %n",
				  &(request.aNetworks[i].authentication),
				  &(request.aNetworks[i].encryption),
				  &(request.aNetworks[i].ucChannelCount),
				  &offset);

		if (3 != params) {
			hdd_warn("Incorrect cmd %s", ptr);
			return -EINVAL;
		}

		hdd_notice("PNO len %d ssid %.*s auth %d encry %d channel count %d offset %d",
			  request.aNetworks[i].ssId.length,
			  request.aNetworks[i].ssId.length,
			  request.aNetworks[i].ssId.ssId,
			  request.aNetworks[i].authentication,
			  request.aNetworks[i].encryption,
			  request.aNetworks[i].ucChannelCount, offset);

		/* Advance to channel list */
		ptr += offset;

		if (SIR_PNO_MAX_NETW_CHANNELS <
		    request.aNetworks[i].ucChannelCount) {
			hdd_warn("Incorrect number of channels");
			return -EINVAL;
		}

		if (0 != request.aNetworks[i].ucChannelCount) {
			for (j = 0; j < request.aNetworks[i].ucChannelCount;
			     j++) {
				if (1 !=
				    sscanf(ptr, "%hhu %n",
					   &(request.aNetworks[i].
					     aChannels[j]), &offset)) {
					hdd_err("PNO network channel input is not valid %s",
						  ptr);
					return -EINVAL;
				}
				/* Advance to next channel number */
				ptr += offset;
			}
		}

		if (1 != sscanf(ptr, "%u %n",
				&(request.aNetworks[i].bcastNetwType),
				&offset)) {
			hdd_err("PNO broadcast network type input is not valid %s",
				  ptr);
			return -EINVAL;
		}

		hdd_notice("PNO bcastNetwType %d offset %d",
			  request.aNetworks[i].bcastNetwType, offset);

		/* Advance to rssi Threshold */
		ptr += offset;
		if (1 != sscanf(ptr, "%d %n",
				&(request.aNetworks[i].rssiThreshold),
				&offset)) {
			hdd_err("PNO rssi threshold input is not valid %s",
				  ptr);
			return -EINVAL;
		}
		hdd_notice("PNO rssi %d offset %d",
			  request.aNetworks[i].rssiThreshold, offset);
		/* Advance to next network */
		ptr += offset;
	} /* For ucNetworkCount */

	params = sscanf(ptr, "%hhu %n", &(mode), &offset);

	request.modePNO = mode;
	/* for LA we just expose suspend option */
	if ((1 != params) || (mode >= SIR_PNO_MODE_MAX)) {
		request.modePNO = SIR_PNO_MODE_ON_SUSPEND;
	}

	sme_set_preferred_network_list(WLAN_HDD_GET_HAL_CTX(adapter),
				       &request,
				       adapter->sessionId,
				       found_pref_network_cb, adapter);

	return 0;
}

static int iw_set_pno(struct net_device *dev,
		      struct iw_request_info *info,
		      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_pno(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif /* FEATURE_WLAN_SCAN_PNO */

/* Common function to SetBand */
int hdd_set_band(struct net_device *dev, u8 ui_band)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	eCsrBand band;

	QDF_STATUS status;
	hdd_context_t *pHddCtx;
	hdd_adapter_list_node_t *pAdapterNode, *pNext;
	eCsrBand currBand = eCSR_BAND_MAX;
	eCsrBand connectedBand;

	pAdapterNode = NULL;
	pNext = NULL;
	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	switch (ui_band) {
	case WLAN_HDD_UI_BAND_AUTO:
		band = eCSR_BAND_ALL;
		break;
	case WLAN_HDD_UI_BAND_5_GHZ:
		band = eCSR_BAND_5G;
		break;
	case WLAN_HDD_UI_BAND_2_4_GHZ:
		band = eCSR_BAND_24;
		break;
	default:
		band = eCSR_BAND_MAX;
	}

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: change band to %u", __func__, band);

	if (band == eCSR_BAND_MAX) {
		/* Received change band request with invalid band value */
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: Invalid band value %u", __func__, ui_band);
		return -EINVAL;
	}

	if ((band == eCSR_BAND_24 && pHddCtx->config->nBandCapability == 2) ||
	    (band == eCSR_BAND_5G && pHddCtx->config->nBandCapability == 1)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: band value %u violate INI settings %u", __func__,
			  band, pHddCtx->config->nBandCapability);
		return -EIO;
	}

	if (band == eCSR_BAND_ALL) {
		hddLog(LOG1,
			FL("Auto band received. Setting band same as ini value %d"),
			pHddCtx->config->nBandCapability);
		band = pHddCtx->config->nBandCapability;
	}

	if (QDF_STATUS_SUCCESS != sme_get_freq_band(hHal, &currBand)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  "%s: Failed to get current band config", __func__);
		return -EIO;
	}

	if (currBand != band) {
		/* Change band request received.
		 * Abort pending scan requests, flush the existing scan results,
		 * and change the band capability
		 */
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
			  "%s: Current band value = %u, new setting %u ",
			  __func__, currBand, band);

		status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
		while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
			pAdapter = pAdapterNode->pAdapter;
			hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
			hdd_abort_mac_scan(pHddCtx, pAdapter->sessionId,
					   eCSR_SCAN_ABORT_DUE_TO_BAND_CHANGE);
			connectedBand =
				hdd_conn_get_connected_band
					(WLAN_HDD_GET_STATION_CTX_PTR(pAdapter));

			/* Handling is done only for STA and P2P */
			if (band != eCSR_BAND_ALL &&
			    ((pAdapter->device_mode == QDF_STA_MODE)
			     || (pAdapter->device_mode == QDF_P2P_CLIENT_MODE))
			    &&
			    (hdd_conn_is_connected
				     (WLAN_HDD_GET_STATION_CTX_PTR(pAdapter)))
			    && (connectedBand != band)) {
				QDF_STATUS status = QDF_STATUS_SUCCESS;
				long lrc;

				/* STA already connected on current band, So issue disconnect
				 * first, then change the band*/

				hddLog(LOG1,
					FL("STA (Device mode %s(%d)) connected in band %u, Changing band to %u, Issuing Disconnect"),
					hdd_device_mode_to_string(pAdapter->device_mode),
					pAdapter->device_mode, currBand, band);
				INIT_COMPLETION(pAdapter->disconnect_comp_var);

				status =
					sme_roam_disconnect(WLAN_HDD_GET_HAL_CTX
								    (pAdapter),
							    pAdapter->sessionId,
							    eCSR_DISCONNECT_REASON_UNSPECIFIED);

				if (QDF_STATUS_SUCCESS != status) {
					hddLog(QDF_TRACE_LEVEL_ERROR,
					       "%s csr_roam_disconnect failure, returned %d",
					       __func__, (int)status);
					return -EINVAL;
				}

				lrc =
					wait_for_completion_timeout(&pAdapter->
								    disconnect_comp_var,
								    msecs_to_jiffies
									    (WLAN_WAIT_TIME_DISCONNECT));

				if (lrc == 0) {
					hddLog(QDF_TRACE_LEVEL_ERROR,
					       "%s:Timeout while waiting for csr_roam_disconnect",
					       __func__);
					return -ETIMEDOUT;
				}
			}

			sme_scan_flush_result(hHal);

			status =
				hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
			pAdapterNode = pNext;
		}

		if (QDF_STATUS_SUCCESS !=
		    sme_set_freq_band(hHal, pAdapter->sessionId, band)) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_FATAL,
				  FL("Failed to set the band value to %u"),
				  band);
			return -EINVAL;
		}
		wlan_hdd_cfg80211_update_band(pHddCtx->wiphy, (eCsrBand) band);
	}
	return 0;
}

int hdd_set_band_helper(struct net_device *dev, const char *command)
{
	uint8_t band;
	int ret;

	/* Convert the band value from ascii to integer */
	command += WLAN_HDD_UI_SET_BAND_VALUE_OFFSET;
	ret = kstrtou8(command, 10, &band);
	if (ret < 0) {
		hddLog(LOGE, FL("kstrtou8 failed"));
		return -EINVAL;
	}

	return hdd_set_band(dev, band);
}

static int __iw_set_band_config(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	int *value = (int *)extra;

	ENTER_DEV(dev);

	if (!capable(CAP_NET_ADMIN)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			FL("permission check failed"));
		return -EPERM;
	}

	return hdd_set_band(dev, value[0]);
}

static int iw_set_band_config(struct net_device *dev,
			      struct iw_request_info *info,
			      union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_band_config(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_set_mon_chan() - Set capture channel on the monitor mode interface.
 * @adapter: Handle to adapter
 * @chan: Monitor mode channel
 * @bandwidth: Capture channel bandwidth
 *
 * Return: 0 on success else error code.
 */
static int wlan_hdd_set_mon_chan(hdd_adapter_t *adapter, uint32_t chan,
				 uint32_t bandwidth)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	hdd_station_ctx_t *sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	struct hdd_mon_set_ch_info *ch_info = &sta_ctx->ch_info;
	QDF_STATUS status;
	tHalHandle hal_hdl = hdd_ctx->hHal;
	struct qdf_mac_addr bssid;
	tCsrRoamProfile roam_profile;
	struct ch_params_s ch_params;

	if (QDF_GLOBAL_MONITOR_MODE != hdd_get_conparam()) {
		hdd_err("Not supported, device is not in monitor mode");
		return -EINVAL;
	}

	hdd_info("Set monitor mode Channel %d", chan);
	hdd_select_cbmode(adapter, chan);
	roam_profile.ChannelInfo.ChannelList = &ch_info->channel;
	roam_profile.ChannelInfo.numOfChannels = 1;
	roam_profile.phyMode = ch_info->phy_mode;
	roam_profile.ch_params.ch_width = bandwidth;

	qdf_mem_copy(bssid.bytes, adapter->macAddressCurrent.bytes,
		     QDF_MAC_ADDR_SIZE);

	ch_params.ch_width = bandwidth;
	sme_set_ch_params(hal_hdl, ch_info->phy_mode, chan, 0, &ch_params);
	status = sme_roam_channel_change_req(hal_hdl, bssid, &ch_params,
					     &roam_profile);
	if (status) {
		hdd_err("Status: %d Failed to set sme_roam Channel for monitor mode",
			status);
	}

	return qdf_status_to_os_return(status);
}

static int __iw_set_two_ints_getnone(struct net_device *dev,
				     struct iw_request_info *info,
				     union iwreq_data *wrqu, char *extra)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int *value = (int *)extra;
	int sub_cmd = value[0];
	int ret;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);

	ENTER_DEV(dev);

	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	switch (sub_cmd) {
	case WE_SET_SMPS_PARAM:
		hddLog(LOG1, "WE_SET_SMPS_PARAM val %d %d", value[1], value[2]);
		ret = wma_cli_set_command(pAdapter->sessionId,
					  WMI_STA_SMPS_PARAM_CMDID,
					  value[1] << WMA_SMPS_PARAM_VALUE_S
					      | value[2],
					  VDEV_CMD);
		break;
#ifdef DEBUG
	case WE_SET_FW_CRASH_INJECT:
		hddLog(LOGE, "WE_SET_FW_CRASH_INJECT: %d %d",
		       value[1], value[2]);
		pr_err("SSR is triggered by iwpriv CRASH_INJECT: %d %d\n",
			   value[1], value[2]);
		ret = wma_cli_set2_command(pAdapter->sessionId,
					   GEN_PARAM_CRASH_INJECT,
					   value[1], value[2], GEN_CMD);
		break;
#endif
	case WE_ENABLE_FW_PROFILE:
		hddLog(LOGE, "WE_ENABLE_FW_PROFILE: %d %d",
		       value[1], value[2]);
		ret = wma_cli_set2_command(pAdapter->sessionId,
				 WMI_WLAN_PROFILE_ENABLE_PROFILE_ID_CMDID,
					value[1], value[2], DBG_CMD);
		break;
	case WE_SET_FW_PROFILE_HIST_INTVL:
		hddLog(LOGE, "WE_SET_FW_PROFILE_HIST_INTVL: %d %d",
		       value[1], value[2]);
		ret = wma_cli_set2_command(pAdapter->sessionId,
					WMI_WLAN_PROFILE_SET_HIST_INTVL_CMDID,
					value[1], value[2], DBG_CMD);
		break;
	case WE_SET_DUAL_MAC_FW_MODE_CONFIG:
		hdd_debug("Ioctl to set dual fw mode config");
		if (hdd_ctx->config->dual_mac_feature_disable) {
			hdd_err("Dual mac feature is disabled from INI");
			return -EPERM;
		}
		hdd_debug("%d %d", value[1], value[2]);
		cds_set_dual_mac_fw_mode_config(value[1], value[2]);
		break;
	case WE_DUMP_DP_TRACE_LEVEL:
		hdd_info("WE_DUMP_DP_TRACE_LEVEL: %d %d",
		       value[1], value[2]);
		if (value[1] == DUMP_DP_TRACE)
			qdf_dp_trace_dump_all(value[2]);
		break;
	case WE_SET_MON_MODE_CHAN:
		ret = wlan_hdd_set_mon_chan(pAdapter, value[1], value[2]);
		break;
	default:
		hddLog(LOGE, "%s: Invalid IOCTL command %d", __func__, sub_cmd);
		break;
	}

	return ret;
}

static int iw_set_two_ints_getnone(struct net_device *dev,
				   struct iw_request_info *info,
				   union iwreq_data *wrqu, char *extra)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __iw_set_two_ints_getnone(dev, info, wrqu, extra);
	cds_ssr_unprotect(__func__);

	return ret;
}

/* Define the Wireless Extensions to the Linux Network Device structure */
/* A number of these routines are NULL (meaning they are not implemented.) */

static const iw_handler we_handler[] = {
	(iw_handler) iw_set_commit,     /* SIOCSIWCOMMIT */
	(iw_handler) iw_get_name,       /* SIOCGIWNAME */
	(iw_handler) NULL,      /* SIOCSIWNWID */
	(iw_handler) NULL,      /* SIOCGIWNWID */
	(iw_handler) iw_set_freq,       /* SIOCSIWFREQ */
	(iw_handler) iw_get_freq,       /* SIOCGIWFREQ */
	(iw_handler) iw_set_mode,       /* SIOCSIWMODE */
	(iw_handler) iw_get_mode,       /* SIOCGIWMODE */
	(iw_handler) NULL,      /* SIOCSIWSENS */
	(iw_handler) NULL,      /* SIOCGIWSENS */
	(iw_handler) NULL,      /* SIOCSIWRANGE */
	(iw_handler) iw_get_range,      /* SIOCGIWRANGE */
	(iw_handler) NULL,      /* SIOCSIWPRIV */
	(iw_handler) NULL,      /* SIOCGIWPRIV */
	(iw_handler) NULL,      /* SIOCSIWSTATS */
	(iw_handler) NULL,      /* SIOCGIWSTATS */
	(iw_handler) NULL,      /* SIOCSIWSPY */
	(iw_handler) NULL,      /* SIOCGIWSPY */
	(iw_handler) NULL,      /* SIOCSIWTHRSPY */
	(iw_handler) NULL,      /* SIOCGIWTHRSPY */
	(iw_handler) iw_set_ap_address, /* SIOCSIWAP */
	(iw_handler) iw_get_ap_address, /* SIOCGIWAP */
	(iw_handler) iw_set_mlme,       /* SIOCSIWMLME */
	(iw_handler) NULL,      /* SIOCGIWAPLIST */
	(iw_handler) iw_set_scan,       /* SIOCSIWSCAN */
	(iw_handler) iw_get_scan,       /* SIOCGIWSCAN */
	(iw_handler) iw_set_essid,      /* SIOCSIWESSID */
	(iw_handler) iw_get_essid,      /* SIOCGIWESSID */
	(iw_handler) iw_set_nick,       /* SIOCSIWNICKN */
	(iw_handler) iw_get_nick,       /* SIOCGIWNICKN */
	(iw_handler) NULL,      /* -- hole -- */
	(iw_handler) NULL,      /* -- hole -- */
	(iw_handler) iw_set_bitrate,    /* SIOCSIWRATE */
	(iw_handler) iw_get_bitrate,    /* SIOCGIWRATE */
	(iw_handler) iw_set_rts_threshold,      /* SIOCSIWRTS */
	(iw_handler) iw_get_rts_threshold,      /* SIOCGIWRTS */
	(iw_handler) iw_set_frag_threshold,     /* SIOCSIWFRAG */
	(iw_handler) iw_get_frag_threshold,     /* SIOCGIWFRAG */
	(iw_handler) iw_set_tx_power,   /* SIOCSIWTXPOW */
	(iw_handler) iw_get_tx_power,   /* SIOCGIWTXPOW */
	(iw_handler) iw_set_retry,      /* SIOCSIWRETRY */
	(iw_handler) iw_get_retry,      /* SIOCGIWRETRY */
	(iw_handler) iw_set_encode,     /* SIOCSIWENCODE */
	(iw_handler) iw_get_encode,     /* SIOCGIWENCODE */
	(iw_handler) iw_set_power_mode, /* SIOCSIWPOWER */
	(iw_handler) iw_get_power_mode, /* SIOCGIWPOWER */
	(iw_handler) NULL,      /* -- hole -- */
	(iw_handler) NULL,      /* -- hole -- */
	(iw_handler) iw_set_genie,      /* SIOCSIWGENIE */
	(iw_handler) iw_get_genie,      /* SIOCGIWGENIE */
	(iw_handler) iw_set_auth,       /* SIOCSIWAUTH */
	(iw_handler) iw_get_auth,       /* SIOCGIWAUTH */
	(iw_handler) iw_set_encodeext,  /* SIOCSIWENCODEEXT */
	(iw_handler) iw_get_encodeext,  /* SIOCGIWENCODEEXT */
	(iw_handler) NULL,      /* SIOCSIWPMKSA */
};

static const iw_handler we_private[] = {

	[WLAN_PRIV_SET_INT_GET_NONE - SIOCIWFIRSTPRIV] = iw_setint_getnone,     /* set priv ioctl */
	[WLAN_PRIV_SET_NONE_GET_INT - SIOCIWFIRSTPRIV] = iw_setnone_getint,     /* get priv ioctl */
	[WLAN_PRIV_SET_CHAR_GET_NONE - SIOCIWFIRSTPRIV] = iw_setchar_getnone,   /* get priv ioctl */
	[WLAN_PRIV_SET_THREE_INT_GET_NONE - SIOCIWFIRSTPRIV] =
		iw_set_three_ints_getnone,
	[WLAN_PRIV_GET_CHAR_SET_NONE - SIOCIWFIRSTPRIV] = iw_get_char_setnone,
	[WLAN_PRIV_SET_NONE_GET_NONE - SIOCIWFIRSTPRIV] = iw_setnone_getnone,   /* action priv ioctl */
	[WLAN_PRIV_SET_VAR_INT_GET_NONE - SIOCIWFIRSTPRIV] =
		iw_hdd_set_var_ints_getnone,
	[WLAN_PRIV_SET_NONE_GET_THREE_INT - SIOCIWFIRSTPRIV] =
							iw_setnone_get_threeint,
	[WLAN_PRIV_ADD_TSPEC - SIOCIWFIRSTPRIV] = iw_add_tspec,
	[WLAN_PRIV_DEL_TSPEC - SIOCIWFIRSTPRIV] = iw_del_tspec,
	[WLAN_PRIV_GET_TSPEC - SIOCIWFIRSTPRIV] = iw_get_tspec,
	[WLAN_PRIV_SET_FTIES - SIOCIWFIRSTPRIV] = iw_set_fties,
	[WLAN_PRIV_SET_HOST_OFFLOAD - SIOCIWFIRSTPRIV] = iw_set_host_offload,
	[WLAN_GET_WLAN_STATISTICS - SIOCIWFIRSTPRIV] = iw_get_statistics,
	[WLAN_SET_KEEPALIVE_PARAMS - SIOCIWFIRSTPRIV] =
		iw_set_keepalive_params,
#ifdef WLAN_FEATURE_PACKET_FILTERING
	[WLAN_SET_PACKET_FILTER_PARAMS - SIOCIWFIRSTPRIV] =
		iw_set_packet_filter_params,
#endif
#ifdef FEATURE_WLAN_SCAN_PNO
	[WLAN_SET_PNO - SIOCIWFIRSTPRIV] = iw_set_pno,
#endif
	[WLAN_SET_BAND_CONFIG - SIOCIWFIRSTPRIV] = iw_set_band_config,
	[WLAN_GET_LINK_SPEED - SIOCIWFIRSTPRIV] = iw_get_linkspeed,
	[WLAN_PRIV_SET_TWO_INT_GET_NONE - SIOCIWFIRSTPRIV] =
		iw_set_two_ints_getnone,
	[WLAN_SET_DOT11P_CHANNEL_SCHED - SIOCIWFIRSTPRIV] =
		iw_set_dot11p_channel_sched,
};

/*Maximum command length can be only 15 */
static const struct iw_priv_args we_private_args[] = {

	/* handlers for main ioctl */
	{WLAN_PRIV_SET_INT_GET_NONE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 ""},

	/* handlers for sub-ioctl */
	{WE_SET_11D_STATE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "set11Dstate"},

	{WE_WOWL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "wowl"},

	{WE_SET_POWER,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setPower"},

	{WE_SET_MAX_ASSOC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setMaxAssoc"},

	{WE_SET_SAP_AUTO_CHANNEL_SELECTION,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0,
		"setAutoChannel" },

	{WE_SET_SCAN_DISABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "scan_disable"},

	{WE_SET_DATA_INACTIVITY_TO,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "inactivityTO"},

	{WE_SET_MAX_TX_POWER,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setMaxTxPower"},

	{WE_SET_TX_POWER,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setTxPower"},

	{WE_SET_MC_RATE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setMcRate"},

	{WE_SET_MAX_TX_POWER_2_4,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setTxMaxPower2G"},

	{WE_SET_MAX_TX_POWER_5_0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setTxMaxPower5G"},

	/* SAP has TxMax whereas STA has MaxTx, adding TxMax for STA
	 * as well to keep same syntax as in SAP. Now onwards, STA
	 * will support both */
	{WE_SET_MAX_TX_POWER,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setTxMaxPower"},

	/* set Higher DTIM Transition (DTIM1 to DTIM3)
	 * 1 = enable and 0 = disable */
	{
		WE_SET_HIGHER_DTIM_TRANSITION,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0,
		"setHDtimTransn"
	},

	{WE_SET_TM_LEVEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setTmLevel"},

	{WE_SET_PHYMODE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "setphymode"},

	{WE_SET_NSS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "nss"},

	{WE_SET_LDPC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "ldpc"},

	{WE_SET_TX_STBC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "tx_stbc"},

	{WE_SET_RX_STBC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "rx_stbc"},

	{WE_SET_SHORT_GI,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "shortgi"},

	{WE_SET_RTSCTS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "enablertscts"},

	{WE_SET_CHWIDTH,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "chwidth"},

	{WE_SET_ANI_EN_DIS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "anienable"},

	{WE_SET_ANI_POLL_PERIOD,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "aniplen"},

	{WE_SET_ANI_LISTEN_PERIOD,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "anilislen"},

	{WE_SET_ANI_OFDM_LEVEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "aniofdmlvl"},

	{WE_SET_ANI_CCK_LEVEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "aniccklvl"},

	{WE_SET_DYNAMIC_BW,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "cwmenable"},

	{WE_SET_CTS_CBW,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "cts_cbw" },

	{WE_SET_GTX_HT_MCS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxHTMcs"},

	{WE_SET_GTX_VHT_MCS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxVHTMcs"},

	{WE_SET_GTX_USRCFG,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxUsrCfg"},

	{WE_SET_GTX_THRE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxThre"},

	{WE_SET_GTX_MARGIN,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxMargin"},

	{WE_SET_GTX_STEP,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxStep"},

	{WE_SET_GTX_MINTPC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxMinTpc"},

	{WE_SET_GTX_BWMASK,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "gtxBWMask"},

	{WE_SET_TX_CHAINMASK,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "txchainmask"},

	{WE_SET_RX_CHAINMASK,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "rxchainmask"},

	{WE_SET_11N_RATE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "set11NRates"},

	{WE_SET_VHT_RATE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "set11ACRates"},

	{WE_SET_AMPDU,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "ampdu"},

	{WE_SET_AMSDU,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "amsdu"},

	{WE_SET_BURST_ENABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "burst_enable"},

	{WE_SET_BURST_DUR,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "burst_dur"},

	{WE_SET_TXPOW_2G,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "txpow2g"},

	{WE_SET_TXPOW_5G,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "txpow5g"},

	/* Sub-cmds DBGLOG specific commands */
	{WE_DBGLOG_LOG_LEVEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_loglevel"},

	{WE_DBGLOG_VAP_ENABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_vapon"},

	{WE_DBGLOG_VAP_DISABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_vapoff"},

	{WE_DBGLOG_MODULE_ENABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_modon"},

	{WE_DBGLOG_MODULE_DISABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_modoff"},

	{WE_DBGLOG_MOD_LOG_LEVEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_mod_loglevel"},

	{WE_DBGLOG_TYPE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_type"},
	{WE_DBGLOG_REPORT_ENABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "dl_report"},

	{WE_SET_TXRX_FWSTATS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "txrx_fw_stats"},

	{WE_TXRX_FWSTATS_RESET,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0,
	 "txrx_fw_st_rst"},

	{WE_PPS_PAID_MATCH,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "paid_match"},

	{WE_PPS_GID_MATCH,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "gid_match"},

	{WE_PPS_EARLY_TIM_CLEAR,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "tim_clear"},

	{WE_PPS_EARLY_DTIM_CLEAR,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "dtim_clear"},

	{WE_PPS_EOF_PAD_DELIM,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "eof_delim"},

	{WE_PPS_MACADDR_MISMATCH,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "mac_match"},

	{WE_PPS_DELIM_CRC_FAIL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "delim_fail"},

	{WE_PPS_GID_NSTS_ZERO,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "nsts_zero"},

	{WE_PPS_RSSI_CHECK,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "rssi_chk"},

	{WE_PPS_5G_EBT,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "5g_ebt"},

	{WE_SET_HTSMPS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "htsmps"},

	{WE_SET_QPOWER_MAX_PSPOLL_COUNT,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "set_qpspollcnt"},

	{WE_SET_QPOWER_MAX_TX_BEFORE_WAKE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "set_qtxwake"},

	{WE_SET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "set_qwakeintv"},

	{WE_SET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "set_qnodatapoll"},

	/* handlers for MCC time quota and latency sub ioctls */
	{WE_MCC_CONFIG_LATENCY,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "setMccLatency"},

	{WE_MCC_CONFIG_QUOTA,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "setMccQuota"},

	{WE_SET_DEBUG_LOG,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "setDbgLvl"},

	/* handlers for early_rx power save */
	{WE_SET_EARLY_RX_ADJUST_ENABLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_enable"},

	{WE_SET_EARLY_RX_TGT_BMISS_NUM,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_bmiss_val"},

	{WE_SET_EARLY_RX_BMISS_SAMPLE_CYCLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_bmiss_smpl"},

	{WE_SET_EARLY_RX_SLOP_STEP,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_slop_step"},

	{WE_SET_EARLY_RX_INIT_SLOP,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_init_slop"},

	{WE_SET_EARLY_RX_ADJUST_PAUSE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_adj_pause"},

	{WE_SET_EARLY_RX_DRIFT_SAMPLE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "erx_dri_sample"},

	{WE_DUMP_STATS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "dumpStats"},

	{WE_CLEAR_STATS,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "clearStats"},

	{WE_START_FW_PROFILE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "startProfile"},

	{WE_SET_CHANNEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "setChanChange" },

	{WE_SET_CONC_SYSTEM_PREF,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 0, "setConcSysPref" },

	{WLAN_PRIV_SET_NONE_GET_INT,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 ""},

	/* handlers for sub-ioctl */
	{WE_GET_11D_STATE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get11Dstate"},

	{WE_IBSS_STATUS,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "getAdhocStatus"},

	{WE_GET_WLAN_DBG,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "getwlandbg"},

	{WE_GET_MAX_ASSOC,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "getMaxAssoc"},

	{WE_GET_SAP_AUTO_CHANNEL_SELECTION,
		0, IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		"getAutoChannel" },

	{WE_GET_CONCURRENCY_MODE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "getconcurrency"},

	{WE_GET_NSS,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_nss"},

	{WE_GET_LDPC,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_ldpc"},

	{WE_GET_TX_STBC,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_tx_stbc"},

	{WE_GET_RX_STBC,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_rx_stbc"},

	{WE_GET_SHORT_GI,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_shortgi"},

	{WE_GET_RTSCTS,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_rtscts"},

	{WE_GET_CHWIDTH,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_chwidth"},

	{WE_GET_ANI_EN_DIS,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_anienable"},

	{WE_GET_ANI_POLL_PERIOD,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_aniplen"},

	{WE_GET_ANI_LISTEN_PERIOD,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_anilislen"},

	{WE_GET_ANI_OFDM_LEVEL,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_aniofdmlvl"},

	{WE_GET_ANI_CCK_LEVEL,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_aniccklvl"},

	{WE_GET_DYNAMIC_BW,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_cwmenable"},

	{WE_GET_GTX_HT_MCS,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxHTMcs"},

	{WE_GET_GTX_VHT_MCS,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxVHTMcs"},

	{WE_GET_GTX_USRCFG,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxUsrCfg"},

	{WE_GET_GTX_THRE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxThre"},

	{WE_GET_GTX_MARGIN,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxMargin"},

	{WE_GET_GTX_STEP,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxStep"},

	{WE_GET_GTX_MINTPC,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxMinTpc"},

	{WE_GET_GTX_BWMASK,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gtxBWMask"},

	{WE_GET_TX_CHAINMASK,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_txchainmask"},

	{WE_GET_RX_CHAINMASK,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_rxchainmask"},

	{WE_GET_11N_RATE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_11nrate"},

	{WE_GET_AMPDU,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_ampdu"},

	{WE_GET_AMSDU,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_amsdu"},

	{WE_GET_BURST_ENABLE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_burst_en"},

	{WE_GET_BURST_DUR,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_burst_dur"},

	{WE_GET_TXPOW_2G,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_txpow2g"},

	{WE_GET_TXPOW_5G,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_txpow5g"},

	{WE_GET_PPS_PAID_MATCH,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_paid_match"},

	{WE_GET_PPS_GID_MATCH,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_gid_match"},

	{WE_GET_PPS_EARLY_TIM_CLEAR,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_tim_clear"},

	{WE_GET_PPS_EARLY_DTIM_CLEAR,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_dtim_clear"},

	{WE_GET_PPS_EOF_PAD_DELIM,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_eof_delim"},

	{WE_GET_PPS_MACADDR_MISMATCH,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_mac_match"},

	{WE_GET_PPS_DELIM_CRC_FAIL,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_delim_fail"},

	{WE_GET_PPS_GID_NSTS_ZERO,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_nsts_zero"},

	{WE_GET_PPS_RSSI_CHECK,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_rssi_chk"},

	{WE_GET_QPOWER_MAX_PSPOLL_COUNT,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_qpspollcnt"},

	{WE_GET_QPOWER_MAX_TX_BEFORE_WAKE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_qtxwake"},

	{WE_GET_QPOWER_SPEC_PSPOLL_WAKE_INTERVAL,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_qwakeintv"},

	{WE_GET_QPOWER_SPEC_MAX_SPEC_NODATA_PSPOLL,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_qnodatapoll"},

	{WE_CAP_TSF,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "cap_tsf"},

	{WE_GET_TEMPERATURE,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "get_temp"},
	/* handlers for main ioctl */
	{WLAN_PRIV_SET_CHAR_GET_NONE,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 ""},

	/* handlers for sub-ioctl */
	{WE_WOWL_ADD_PTRN,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 "wowlAddPtrn"},

	{WE_WOWL_DEL_PTRN,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 "wowlDelPtrn"},

	/* handlers for sub-ioctl */
	{WE_NEIGHBOR_REPORT_REQUEST,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 "neighbor"},

	{WE_SET_AP_WPS_IE,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 "set_ap_wps_ie"},

	{WE_SET_CONFIG,
	 IW_PRIV_TYPE_CHAR | 512,
	 0,
	 "setConfig"},

	/* handlers for main ioctl */
	{WLAN_PRIV_SET_THREE_INT_GET_NONE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	 0,
	 ""},

	/* handlers for sub-ioctl */
	{WE_SET_WLAN_DBG,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	 0,
	 "setwlandbg"},

	/* handlers for sub-ioctl */
	{WE_SET_DP_TRACE,
	IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	0,
	"set_dp_trace"},

	{WE_SET_SAP_CHANNELS,
	IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	0,
	"setsapchannels"},
	/* handlers for main ioctl */
	{WLAN_PRIV_SET_NONE_GET_THREE_INT,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	 "" },
	{WE_GET_TSF,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	 "get_tsf" },

	{WE_SET_DUAL_MAC_SCAN_CONFIG,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3,
	 0,
	 "set_scan_cfg"},

	/* handlers for main ioctl */
	{WLAN_PRIV_GET_CHAR_SET_NONE,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 ""},

	/* handlers for sub-ioctl */
	{WE_WLAN_VERSION,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "version"},
	{WE_GET_STATS,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getStats"},
	{WE_LIST_FW_PROFILE,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "listProfile"},
	{WE_GET_STATES,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getHostStates"},
	{WE_GET_CFG,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getConfig"},
	{WE_GET_RSSI,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getRSSI"},
	{WE_GET_WMM_STATUS,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getWmmStatus"},
	{
		WE_GET_CHANNEL_LIST,
		0,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
		"getChannelList"
	},
#ifdef FEATURE_WLAN_TDLS
	{
		WE_GET_TDLS_PEERS,
		0,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
		"getTdlsPeers"
	},
#endif
#ifdef WLAN_FEATURE_11W
	{
		WE_GET_11W_INFO,
		0,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
		"getPMFInfo"
	},
#endif
	{
		WE_GET_IBSS_STA_INFO,
		0,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
		"getIbssSTAs"
	},
	{WE_GET_PHYMODE,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getphymode"},
#ifdef FEATURE_OEM_DATA_SUPPORT
	{WE_GET_OEM_DATA_CAP,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getOemDataCap"},
#endif /* FEATURE_OEM_DATA_SUPPORT */
	{WE_GET_SNR,
	 0,
	 IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
	 "getSNR"},

	/* handlers for main ioctl */
	{WLAN_PRIV_SET_NONE_GET_NONE,
	 0,
	 0,
	 ""},

	/* handlers for sub-ioctl */
	{
		WE_IBSS_GET_PEER_INFO_ALL,
		0,
		0,
		"ibssPeerInfoAll"
	},
	{WE_GET_RECOVERY_STAT,
	 0,
	 0,
	 "getRecoverStat"},

	{WE_GET_FW_PROFILE_DATA,
	 0,
	 0,
	 "getProfileData"},

	{WE_SET_REASSOC_TRIGGER,
	0,
	0,
	"reassoc"},

	{WE_DUMP_AGC_START,
	 0,
	 0,
	 "dump_agc_start"},

	{WE_DUMP_AGC,
	 0,
	 0,
	 "dump_agc"},

	{WE_DUMP_CHANINFO_START,
	 0,
	 0,
	 "dump_chninfo_en"},

	{WE_DUMP_CHANINFO,
	 0,
	 0,
	 "dump_chninfo"},

	{WE_DUMP_WATCHDOG,
	 0,
	 0,
	 "dump_watchdog"},
#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
	{WE_DUMP_PCIE_LOG,
	 0,
	 0,
	 "dump_pcie_log"},
#endif
	{WE_STOP_OBSS_SCAN,
	 0,
	 0,
	 "stop_obss_scan"},
	/* handlers for main ioctl */
	{WLAN_PRIV_SET_VAR_INT_GET_NONE,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 ""},

	/* handlers for sub-ioctl */
	{WE_IBSS_GET_PEER_INFO,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "ibssPeerInfo"},

	/* handlers for sub-ioctl */
	{WE_MTRACE_SELECTIVE_MODULE_LOG_ENABLE_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "setdumplog"},

	{WE_MTRACE_DUMP_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "dumplog"},
#ifdef MPC_UT_FRAMEWORK
	{WE_POLICY_MANAGER_CLIST_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_clist"},

	{WE_POLICY_MANAGER_DLIST_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_dlist"},

	{WE_POLICY_MANAGER_DBS_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_dbs"},

	{WE_POLICY_MANAGER_PCL_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_pcl"},

	{WE_POLICY_MANAGER_CINFO_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_cinfo"},

	{WE_POLICY_MANAGER_ULIST_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_ulist"},

	{WE_POLICY_MANAGER_QUERY_ACTION_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_query_action"},

	{WE_POLICY_MANAGER_QUERY_ALLOW_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_query_allow"},

	{WE_POLICY_MANAGER_SCENARIO_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_run_scenario"},

	{WE_POLICY_SET_HW_MODE_CMD,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "pm_set_hw_mode"},
#endif
#ifdef FEATURE_WLAN_TDLS
	/* handlers for sub ioctl */
	{
		WE_TDLS_CONFIG_PARAMS,
		IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
		0,
		"setTdlsConfig"
	},
#endif
	{
		WE_UNIT_TEST_CMD,
		IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
		0,
		"setUnitTestCmd"
	},

#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
	{WE_LED_FLASHING_PARAM,
	 IW_PRIV_TYPE_INT | MAX_VAR_ARGS,
	 0,
	 "gpio_control"},
#endif
	/* handlers for main ioctl */
	{WLAN_PRIV_ADD_TSPEC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | HDD_WLAN_WMM_PARAM_COUNT,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "addTspec"},

	/* handlers for main ioctl */
	{WLAN_PRIV_DEL_TSPEC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "delTspec"},

	/* handlers for main ioctl */
	{WLAN_PRIV_GET_TSPEC,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "getTspec"},

	/* handlers for main ioctl - host offload */
	{
		WLAN_PRIV_SET_HOST_OFFLOAD,
		IW_PRIV_TYPE_BYTE | sizeof(tHostOffloadRequest),
		0,
		"setHostOffload"
	}
	,

	{
		WLAN_GET_WLAN_STATISTICS,
		0,
		IW_PRIV_TYPE_BYTE | WE_MAX_STR_LEN,
		"getWlanStats"
	}
	,

	{
		WLAN_SET_KEEPALIVE_PARAMS,
		sizeof(tSirKeepAliveReq) | IW_PRIV_SIZE_FIXED,
		0,
		"setKeepAlive"
	}
	,
#ifdef WLAN_FEATURE_PACKET_FILTERING
	{
		WLAN_SET_PACKET_FILTER_PARAMS,
		IW_PRIV_TYPE_BYTE | IW_PRIV_SIZE_FIXED |
					sizeof(struct pkt_filter_cfg),
		0,
		"setPktFilter"
	}
	,
#endif
#ifdef FEATURE_WLAN_SCAN_PNO
	{
		WLAN_SET_PNO,
		IW_PRIV_TYPE_CHAR | WE_MAX_STR_LEN,
		0,
		"setpno"
	}
	,
#endif
	{
		WLAN_SET_BAND_CONFIG,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
		0,
		"SETBAND"
	}
	,
	{
		WLAN_GET_LINK_SPEED,
		IW_PRIV_TYPE_CHAR | 18,
		IW_PRIV_TYPE_CHAR | 5, "getLinkSpeed"
	}
	,

	/* handlers for main ioctl */
	{WLAN_PRIV_SET_TWO_INT_GET_NONE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0,
	 ""}
	,
	{WE_SET_SMPS_PARAM,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "set_smps_param"}
	,
	{WLAN_SET_DOT11P_CHANNEL_SCHED,
	 IW_PRIV_TYPE_BYTE | sizeof(struct dot11p_channel_sched),
	 0, "set_dot11p" }
	,
#ifdef DEBUG
	{WE_SET_FW_CRASH_INJECT,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "crash_inject"}
	,
#endif
	{WE_ENABLE_FW_PROFILE,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "enableProfile"}
	,
	{WE_SET_FW_PROFILE_HIST_INTVL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "set_hist_intvl"}
	,
	{WE_SET_DUAL_MAC_FW_MODE_CONFIG,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "set_fw_mode_cfg"}
	,
	{WE_DUMP_DP_TRACE_LEVEL,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "dump_dp_trace"}
	,
	{WE_SET_MON_MODE_CHAN,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2,
	 0, "setMonChan"}
	,

	{WE_GET_ROAM_SYNCH_DELAY,
	 0,
	 IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1,
	 "hostroamdelay"}
	,
};

const struct iw_handler_def we_handler_def = {
	.num_standard = QDF_ARRAY_SIZE(we_handler),
	.num_private = QDF_ARRAY_SIZE(we_private),
	.num_private_args = QDF_ARRAY_SIZE(we_private_args),

	.standard = (iw_handler *) we_handler,
	.private = (iw_handler *) we_private,
	.private_args = we_private_args,
	.get_wireless_stats = NULL,
};

int hdd_set_wext(hdd_adapter_t *pAdapter)
{
	hdd_wext_state_t *pwextBuf;
	hdd_station_ctx_t *pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(pAdapter);

	pwextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);

	/* Now configure the roaming profile links. To SSID and bssid. */
	pwextBuf->roamProfile.SSIDs.numOfSSIDs = 0;
	pwextBuf->roamProfile.SSIDs.SSIDList = &pHddStaCtx->conn_info.SSID;

	pwextBuf->roamProfile.BSSIDs.numOfBSSIDs = 0;
	pwextBuf->roamProfile.BSSIDs.bssid = &pHddStaCtx->conn_info.bssId;

	/*Set the numOfChannels to zero to scan all the channels */
	pwextBuf->roamProfile.ChannelInfo.numOfChannels = 0;
	pwextBuf->roamProfile.ChannelInfo.ChannelList = NULL;

	/* Default is no encryption */
	pwextBuf->roamProfile.EncryptionType.numEntries = 1;
	pwextBuf->roamProfile.EncryptionType.encryptionType[0] =
		eCSR_ENCRYPT_TYPE_NONE;

	pwextBuf->roamProfile.mcEncryptionType.numEntries = 1;
	pwextBuf->roamProfile.mcEncryptionType.encryptionType[0] =
		eCSR_ENCRYPT_TYPE_NONE;

	pwextBuf->roamProfile.BSSType = eCSR_BSS_TYPE_INFRASTRUCTURE;

	/* Default is no authentication */
	pwextBuf->roamProfile.AuthType.numEntries = 1;
	pwextBuf->roamProfile.AuthType.authType[0] = eCSR_AUTH_TYPE_OPEN_SYSTEM;

	pwextBuf->roamProfile.phyMode = eCSR_DOT11_MODE_AUTO;
	pwextBuf->wpaVersion = IW_AUTH_WPA_VERSION_DISABLED;

	/*Set the default scan mode */
	pAdapter->scan_info.scan_mode = eSIR_ACTIVE_SCAN;

	hdd_clear_roam_profile_ie(pAdapter);

	return QDF_STATUS_SUCCESS;

}

int hdd_register_wext(struct net_device *dev)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_wext_state_t *pwextBuf = WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter);
	QDF_STATUS status;

	ENTER();

	/* Zero the memory.  This zeros the profile structure. */
	memset(pwextBuf, 0, sizeof(hdd_wext_state_t));

	init_completion(&(WLAN_HDD_GET_WEXT_STATE_PTR(pAdapter))->
			completion_var);

	status = hdd_set_wext(pAdapter);

	if (!QDF_IS_STATUS_SUCCESS(status)) {

		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  ("ERROR: hdd_set_wext failed!!"));
		return QDF_STATUS_E_FAILURE;
	}

	if (!QDF_IS_STATUS_SUCCESS(qdf_event_create(&pwextBuf->hdd_qdf_event))) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  ("ERROR: HDD qdf event init failed!!"));
		return QDF_STATUS_E_FAILURE;
	}

	if (!QDF_IS_STATUS_SUCCESS(qdf_event_create(&pwextBuf->scanevent))) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  ("ERROR: HDD scan event init failed!!"));
		return QDF_STATUS_E_FAILURE;
	}
	/* Register as a wireless device */
	dev->wireless_handlers = (struct iw_handler_def *)&we_handler_def;

	EXIT();
	return 0;
}

int hdd_unregister_wext(struct net_device *dev)
{
	hddLog(LOG1, FL("dev(%p)"), dev);

	if (dev != NULL) {
		rtnl_lock();
		dev->wireless_handlers = NULL;
		rtnl_unlock();
	}

	return 0;
}
