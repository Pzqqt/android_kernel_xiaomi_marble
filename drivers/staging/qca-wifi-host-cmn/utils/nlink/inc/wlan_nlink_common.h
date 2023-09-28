/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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

/*===========================================================================
   \file wlan_nlink_common.h

   Exports and types for the Netlink Service interface. This header file contains
   message types and definitions that is shared between the user space service
   (e.g. logging service) and WLAN kernel module.

   ===========================================================================*/

#ifndef WLAN_NLINK_COMMON_H__
#define WLAN_NLINK_COMMON_H__

#include <linux/netlink.h>

#ifdef __KERNEL__
#include <linux/if.h>
#else
#include <net/if.h>
#endif

/*---------------------------------------------------------------------------
 * External Functions
 *-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-------------------------------------------------------------------------*/
#define WLAN_NL_MAX_PAYLOAD   5120       /* maximum size for netlink message */
#define WLAN_NLINK_PROTO_FAMILY  NETLINK_USERSOCK
#define WLAN_NLINK_MCAST_GRP_ID  0x01

/*---------------------------------------------------------------------------
 * Type Declarations
 *-------------------------------------------------------------------------*/

/*
 * The following enum defines the target service within WLAN driver for which the
 * message is intended for. Each service along with its counterpart
 * in the user space, define a set of messages they recognize.
 * Each of this message will have an header of type tAniMsgHdr defined below.
 * Each Netlink message to/from a kernel module will contain only one
 * message which is preceded by a tAniMsgHdr. The maximun size (in bytes) of
 * a netlink message is assumed to be MAX_PAYLOAD bytes.
 *
 *         +------------+-------+----------+----------+
 *         |Netlink hdr | Align |tAniMsgHdr| msg body |
 *         +------------+-------+----------|----------+
 */

/* Message Types */
#define WLAN_SVC_FW_CRASHED_IND     0x100
#define WLAN_SVC_LTE_COEX_IND       0x101
#define WLAN_SVC_WLAN_AUTO_SHUTDOWN_IND 0x102
#define WLAN_SVC_DFS_CAC_START_IND      0x103
#define WLAN_SVC_DFS_CAC_END_IND        0x104
#define WLAN_SVC_DFS_RADAR_DETECT_IND   0x105
#define WLAN_SVC_WLAN_STATUS_IND    0x106
#define WLAN_SVC_WLAN_VERSION_IND   0x107
#define WLAN_SVC_DFS_ALL_CHANNEL_UNAVAIL_IND 0x108
#define WLAN_SVC_WLAN_TP_IND        0x109
#define WLAN_SVC_RPS_ENABLE_IND     0x10A
#define WLAN_SVC_WLAN_TP_TX_IND     0x10B
#define WLAN_SVC_WLAN_AUTO_SHUTDOWN_CANCEL_IND 0x10C
#define WLAN_SVC_WLAN_RADIO_INDEX 0x10D
#define WLAN_SVC_FW_SHUTDOWN_IND  0x10E
#define WLAN_SVC_CORE_MINFREQ     0x10F
#define WLAN_SVC_MAX_SSID_LEN    32
#define WLAN_SVC_MAX_BSSID_LEN   6
#define WLAN_SVC_MAX_STR_LEN     16
#define WLAN_SVC_MAX_NUM_CHAN    128
#define WLAN_SVC_COUNTRY_CODE_LEN 3

#define ANI_NL_MSG_BASE     0x10        /* Some arbitrary base */

typedef enum eAniNlModuleTypes {
	ANI_NL_MSG_PUMAC = ANI_NL_MSG_BASE + 0x01,      /* PTT Socket App */
	ANI_NL_MSG_PTT = ANI_NL_MSG_BASE + 0x07,        /* Quarky GUI */
	WLAN_NL_MSG_OEM = ANI_NL_MSG_BASE + 0x09,
	WLAN_NL_MSG_SVC = ANI_NL_MSG_BASE + 0x0a,
	WLAN_NL_MSG_CNSS_DIAG = ANI_NL_MSG_BASE + 0x0B, /* Value needs to be 27 */
	ANI_NL_MSG_LOG,
	WLAN_NL_MSG_SPECTRAL_SCAN,
	ANI_NL_MSG_MAX
} tAniNlModTypes, tWlanNlModTypes;

#define WLAN_NL_MSG_BASE ANI_NL_MSG_BASE
#define WLAN_NL_MSG_MAX  ANI_NL_MSG_MAX

/* All Netlink messages must contain this header */
typedef struct sAniHdr {
	unsigned short type;
	unsigned short length;
} tAniHdr, tAniMsgHdr;

typedef struct sAniNlMsg {
	struct nlmsghdr nlh;    /* Netlink Header */
	int radio;              /* unit number of the radio */
	tAniHdr wmsg;           /* Airgo Message Header */
} tAniNlHdr;

struct radio_index_tlv {
	unsigned short type;
	unsigned short length;
	int radio;
};

/**
 * struct svc_channel_info - Channel information
 * @chan_id: Channel ID
 * @reserved0: Reserved for padding and future use
 * @mhz: Primary 20 MHz channel frequency in MHz
 * @band_center_freq1: Center frequency 1 in MHz
 * @band_center_freq2: Center frequency 2 in MHz
 * @info: Channel info
 * @reg_info_1: Regulatory information field 1 which contains
 *              MIN power, MAX power, reg power and reg class ID
 * @reg_info_2: Regulatory information field 2 which contains antennamax
 */
struct svc_channel_info {
	uint32_t chan_id;
	uint32_t reserved0;
	uint32_t mhz;
	uint32_t band_center_freq1;
	uint32_t band_center_freq2;
	uint32_t info;
	uint32_t reg_info_1;
	uint32_t reg_info_2;
};

struct wlan_status_data {
	uint8_t lpss_support;
	uint8_t is_on;
	uint8_t vdev_id;
	uint8_t is_connected;
	int8_t rssi;
	uint8_t ssid_len;
	uint8_t country_code[WLAN_SVC_COUNTRY_CODE_LEN];
	uint32_t vdev_mode;
	uint32_t freq;
	uint32_t numChannels;
	uint8_t channel_list[WLAN_SVC_MAX_NUM_CHAN];
	uint8_t ssid[WLAN_SVC_MAX_SSID_LEN];
	uint8_t bssid[WLAN_SVC_MAX_BSSID_LEN];
	struct svc_channel_info channel_info[WLAN_SVC_MAX_NUM_CHAN];
};

struct wlan_version_data {
	uint32_t chip_id;
	char chip_name[WLAN_SVC_MAX_STR_LEN];
	char chip_from[WLAN_SVC_MAX_STR_LEN];
	char host_version[WLAN_SVC_MAX_STR_LEN];
	char fw_version[WLAN_SVC_MAX_STR_LEN];
};

struct wlan_dfs_info {
	uint16_t channel;
	uint8_t country_code[WLAN_SVC_COUNTRY_CODE_LEN];
};

/*
 * Maximim number of queues supported by WLAN driver. Setting an upper
 * limit. Actual number of queues may be smaller than this value.
 */
#define WLAN_SVC_IFACE_NUM_QUEUES 6

/**
 * struct wlan_rps_data - structure to send RPS info to cnss-daemon
 * @ifname:         interface name for which the RPS data belongs to
 * @num_queues:     number of rx queues for which RPS data is being sent
 * @cpu_map_list:   array of cpu maps for different rx queues supported by
 *                  the wlan driver
 *
 * The structure specifies the format of data exchanged between wlan
 * driver and cnss-daemon. On receipt of the data, cnss-daemon is expected
 * to apply the 'cpu_map' for each rx queue belonging to the interface 'ifname'
 */
struct wlan_rps_data {
	char ifname[IFNAMSIZ];
	uint16_t num_queues;
	uint16_t cpu_map_list[WLAN_SVC_IFACE_NUM_QUEUES];
};

/**
 * enum wlan_tp_level - indicates wlan throughput level
 * @WLAN_SVC_TP_NONE:	 used for initialization
 * @WLAN_SVC_TP_LOW:	 used to identify low throughput level
 * @WLAN_SVC_TP_MEDIUM:	 used to identify medium throughput level
 * @WLAN_SVC_TP_HIGH:	 used to identify high throughput level
 *
 * The different throughput levels are determined on the basis of # of tx and
 * rx packets and other threshold values. For example, if the # of total
 * packets sent or received by the driver is greater than 500 in the last 100ms
 * , the driver has a high throughput requirement. The driver may tweak certain
 * system parameters based on the throughput level.
 */
enum wlan_tp_level {
	WLAN_SVC_TP_NONE,
	WLAN_SVC_TP_LOW,
	WLAN_SVC_TP_MEDIUM,
	WLAN_SVC_TP_HIGH,
};

/**
 * struct wlan_core_minfreq - msg to [re]set the min freq of a set of cores
 * @magic:            signature token: 0xBABA
 * @reserved:         unused for now
 * @coremask:         bitmap of cores (16 bits) bit0=CORE0, bit1=CORE1, ...
 *                     coremask is ONLY valid for set command
 *                     valid values: 0xf0, or 0x0f
 * @freq:             frequency in KH
 *                     >  0: "set to the given frequency"
 *                     == 0: "free; remove the lock"
 *
 * Msg structure passed by the driver to cnss-daemon.
 *
 * Semantical Alert:
 *   There can be only one outstanding lock, even for different masks.
 */
#define WLAN_CORE_MINFREQ_MAGIC 0xBABA
struct wlan_core_minfreq {
	uint16_t magic;
	uint16_t reserved;
	uint16_t coremask;
	uint16_t freq;
};

/* Indication to enable TCP delayed ack in TPUT indication */
#define TCP_DEL_ACK_IND	(1 << 0)
#define TCP_DEL_ACK_IND_MASK	0x1
/* Indication to enable TCP advance window scaling in TPUT indication */
#define TCP_ADV_WIN_SCL	(1 << 1)
#define TCP_ADV_WIN_SCL_MASK	0x2

/* TCP limit output bytes for low and high TPUT */
#define TCP_LIMIT_OUTPUT_BYTES_LOW	506072
#define TCP_LIMIT_OUTPUT_BYTES_HI	4048579

/* TCP window scale for low and high TPUT */
#define WIN_SCALE_LOW	2
#define WIN_SCALE_HI	1

/* TCP DEL ACK value for low and high TPUT */
#define TCP_DEL_ACK_LOW		0
#define TCP_DEL_ACK_HI		20

/**
 * struct wlan_rx_tp_data - msg to TCP delayed ack and advance window scaling
 * @level:            Throughput level.
 * @rx_tp_flags:      Bit map of flags, for which this indcation will take
 *                    effect, bit map for TCP_ADV_WIN_SCL and TCP_DEL_ACK_IND.
 */
struct wlan_rx_tp_data {
	enum wlan_tp_level level;
	uint16_t rx_tp_flags;
};

/**
 * struct wlan_tx_tp_data - msg to TCP for Tx Dir
 * @level:            Throughput level.
 * @tcp_limit_output: Tcp limit output flag.
 *
 */
struct wlan_tx_tp_data {
	enum wlan_tp_level level;
	bool tcp_limit_output;
};

#endif /* WLAN_NLINK_COMMON_H__ */
