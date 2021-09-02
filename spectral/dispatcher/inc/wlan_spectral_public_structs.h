/*
 * Copyright (c) 2011,2017-2021 The Linux Foundation. All rights reserved.
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

#include <qdf_types.h>
#include "wlan_dfs_ioctl.h"
#include <spectral_ioctl.h>

#ifndef __KERNEL__
#include <math.h>
#endif /*  __KERNEL__  */

#ifndef _WLAN_SPECTRAL_PUBLIC_STRUCTS_H_
#define _WLAN_SPECTRAL_PUBLIC_STRUCTS_H_

#ifndef AH_MAX_CHAINS
#define AH_MAX_CHAINS 3
#endif

#define MAX_NUM_CHANNELS             255
#define SPECTRAL_PHYERR_PARAM_NOVAL  65535

#ifdef SPECTRAL_USE_EMU_DEFAULTS
/* Use defaults from emulation */
#define SPECTRAL_SCAN_ACTIVE_DEFAULT           (0x0)
#define SPECTRAL_SCAN_ENABLE_DEFAULT           (0x0)
#define SPECTRAL_SCAN_COUNT_DEFAULT            (0x0)
#define SPECTRAL_SCAN_PERIOD_DEFAULT           (250)
#define SPECTRAL_SCAN_PRIORITY_DEFAULT         (0x1)
#define SPECTRAL_SCAN_FFT_SIZE_DEFAULT         (0x7)
#define SPECTRAL_SCAN_GC_ENA_DEFAULT           (0x1)
#define SPECTRAL_SCAN_RESTART_ENA_DEFAULT      (0x0)
#define SPECTRAL_SCAN_NOISE_FLOOR_REF_DEFAULT  (0xa0)
#define SPECTRAL_SCAN_INIT_DELAY_DEFAULT       (0x50)
#define SPECTRAL_SCAN_NB_TONE_THR_DEFAULT      (0xc)
#define SPECTRAL_SCAN_STR_BIN_THR_DEFAULT      (0x7)
#define SPECTRAL_SCAN_WB_RPT_MODE_DEFAULT      (0x0)
#define SPECTRAL_SCAN_RSSI_RPT_MODE_DEFAULT    (0x1)
#define SPECTRAL_SCAN_RSSI_THR_DEFAULT         (0xf)
#define SPECTRAL_SCAN_PWR_FORMAT_DEFAULT       (0x1)
#define SPECTRAL_SCAN_RPT_MODE_DEFAULT         (0x2)
#define SPECTRAL_SCAN_BIN_SCALE_DEFAULT        (0x1)
#define SPECTRAL_SCAN_DBM_ADJ_DEFAULT          (0x0)
#define SPECTRAL_SCAN_CHN_MASK_DEFAULT         (0x1)
#else
/*
 * Static default values for spectral state and configuration.
 * These definitions should be treated as temporary. Ideally,
 * we should get the defaults from firmware - this will be discussed.
 *
 * Use defaults from Spectral Hardware Micro-Architecture
 * document (v1.0)
 */
#define SPECTRAL_SCAN_ACTIVE_DEFAULT           (0)
#define SPECTRAL_SCAN_ENABLE_DEFAULT           (0)
#define SPECTRAL_SCAN_COUNT_DEFAULT            (0)
#define SPECTRAL_SCAN_PERIOD_GEN_I_DEFAULT     (35)
#define SPECTRAL_SCAN_PERIOD_GEN_II_DEFAULT    (35)
#define SPECTRAL_SCAN_PERIOD_GEN_III_DEFAULT   (224)
#define SPECTRAL_SCAN_PRIORITY_DEFAULT         (1)
#define SPECTRAL_SCAN_FFT_SIZE_DEFAULT         (7)
#define SPECTRAL_SCAN_GC_ENA_DEFAULT           (1)
#define SPECTRAL_SCAN_RESTART_ENA_DEFAULT      (0)
#define SPECTRAL_SCAN_NOISE_FLOOR_REF_DEFAULT  (-96)
#define SPECTRAL_SCAN_INIT_DELAY_DEFAULT       (80)
#define SPECTRAL_SCAN_NB_TONE_THR_DEFAULT      (12)
#define SPECTRAL_SCAN_STR_BIN_THR_DEFAULT      (8)
#define SPECTRAL_SCAN_WB_RPT_MODE_DEFAULT      (0)
#define SPECTRAL_SCAN_RSSI_RPT_MODE_DEFAULT    (0)
#define SPECTRAL_SCAN_RSSI_THR_DEFAULT         (0xf0)
#define SPECTRAL_SCAN_PWR_FORMAT_DEFAULT       (0)
#define SPECTRAL_SCAN_RPT_MODE_DEFAULT         (2)
#define SPECTRAL_SCAN_BIN_SCALE_DEFAULT        (1)
#define SPECTRAL_SCAN_DBM_ADJ_DEFAULT          (1)
#define SPECTRAL_SCAN_CHN_MASK_DEFAULT         (1)
#define SPECTRAL_SCAN_FREQUENCY_DEFAULT        (0)
#endif				/* SPECTRAL_USE_EMU_DEFAULTS */

/* The below two definitions apply only to pre-11ac chipsets */
#define SPECTRAL_SCAN_SHORT_REPORT_DEFAULT     (1)
#define SPECTRAL_SCAN_FFT_PERIOD_DEFAULT       (1)

/*
 * Definitions to help in scaling of gen3 linear format Spectral bins to values
 * similar to those from gen2 chipsets.
 */

/*
 * Max gain for QCA9984. Since this chipset is a prime representative of gen2
 * chipsets, it is chosen for this value.
 */
#define SPECTRAL_QCA9984_MAX_GAIN                               (78)

/* Temporary section for hard-coded values. These need to come from FW. */

/* Max gain for IPQ8074 */
#define SPECTRAL_IPQ8074_DEFAULT_MAX_GAIN_HARDCODE              (62)

/*
 * Section for values needing tuning per customer platform. These too may need
 * to come from FW. To be considered as hard-coded for now.
 */

/*
 * If customers have a different gain line up than QCA reference designs for
 * IPQ8074 and/or QCA9984, they may have to tune the low level threshold and
 * the RSSI threshold.
 */
#define SPECTRAL_SCALING_LOW_LEVEL_OFFSET                       (7)
#define SPECTRAL_SCALING_RSSI_THRESH                            (5)

/*
 * If customers set the AGC backoff differently, they may have to tune the high
 * level threshold.
 */
#define SPECTRAL_SCALING_HIGH_LEVEL_OFFSET                      (5)

/* End of section for values needing fine tuning. */
/* End of temporary section for hard-coded values */

/**
 * enum spectral_msg_buf_type - Spectral message buffer type
 * @SPECTRAL_MSG_BUF_NEW: Allocate new buffer
 * @SPECTRAL_MSG_BUF_SAVED: Reuse last buffer, used for secondary segment report
 *                          in case of 160 MHz.
 */
enum spectral_msg_buf_type {
	SPECTRAL_MSG_BUF_NEW,
	SPECTRAL_MSG_BUF_SAVED,
	SPECTRAL_MSG_BUF_TYPE_MAX,
};

/**
 * enum spectral_msg_type - Spectral SAMP message type
 * @SPECTRAL_MSG_NORMAL_MODE: Normal mode Spectral SAMP message
 * @SPECTRAL_MSG_AGILE_MODE: Agile mode Spectral SAMP message
 * @SPECTRAL_MSG_INTERFERENCE_NOTIFICATION: Interference notification to
 *                                          external auto channel selection
 *                                          entity
 * @SPECTRAL_MSG_TYPE_MAX: Spectral SAMP message type max
 */
enum spectral_msg_type {
	SPECTRAL_MSG_NORMAL_MODE,
	SPECTRAL_MSG_AGILE_MODE,
	SPECTRAL_MSG_INTERFERENCE_NOTIFICATION,
	SPECTRAL_MSG_TYPE_MAX,
};

/**
 * enum spectral_debug - Spectral debug level
 * @DEBUG_SPECTRAL:  Minimal SPECTRAL debug
 * @DEBUG_SPECTRAL1: Normal SPECTRAL debug
 * @DEBUG_SPECTRAL2: Maximal SPECTRAL debug
 * @DEBUG_SPECTRAL3: Matched filterID display
 * @DEBUG_SPECTRAL4: One time dump of FFT report
 */
enum spectral_debug {
	DEBUG_SPECTRAL = 0x00000100,
	DEBUG_SPECTRAL1 = 0x00000200,
	DEBUG_SPECTRAL2 = 0x00000400,
	DEBUG_SPECTRAL3 = 0x00000800,
	DEBUG_SPECTRAL4 = 0x00001000,
};

/**
 * enum spectral_capability_type - Spectral capability type
 * @SPECTRAL_CAP_PHYDIAG:              Phydiag capability
 * @SPECTRAL_CAP_RADAR:                Radar detection capability
 * @SPECTRAL_CAP_SPECTRAL_SCAN:        Spectral capability
 * @SPECTRAL_CAP_ADVNCD_SPECTRAL_SCAN: Advanced spectral capability
 */
enum spectral_capability_type {
	SPECTRAL_CAP_PHYDIAG,
	SPECTRAL_CAP_RADAR,
	SPECTRAL_CAP_SPECTRAL_SCAN,
	SPECTRAL_CAP_ADVNCD_SPECTRAL_SCAN,
};

/**
 * enum spectral_cp_error_code - Spectral control path response code
 * @SPECTRAL_SCAN_RESP_ERR_INVALID: Invalid error identifier
 * @SPECTRAL_SCAN_RESP_ERR_PARAM_UNSUPPORTED: parameter unsupported
 * @SPECTRAL_SCAN_RESP_ERR_MODE_UNSUPPORTED: mode unsupported
 * @SPECTRAL_SCAN_RESP_ERR_PARAM_INVALID_VALUE: invalid parameter value
 * @SPECTRAL_SCAN_RESP_ERR_PARAM_NOT_INITIALIZED: parameter uninitialized
 */
enum spectral_cp_error_code {
	SPECTRAL_SCAN_ERR_INVALID,
	SPECTRAL_SCAN_ERR_PARAM_UNSUPPORTED,
	SPECTRAL_SCAN_ERR_MODE_UNSUPPORTED,
	SPECTRAL_SCAN_ERR_PARAM_INVALID_VALUE,
	SPECTRAL_SCAN_ERR_PARAM_NOT_INITIALIZED,
};

/**
 * enum spectral_dma_debug -   Spectral DMA debug
 * @SPECTRAL_DMA_RING_DEBUG:   Spectral DMA ring debug
 * @SPECTRAL_DMA_BUFFER_DEBUG: Spectral DMA buffer debug
 */
enum spectral_dma_debug {
	SPECTRAL_DMA_RING_DEBUG,
	SPECTRAL_DMA_BUFFER_DEBUG,
};

struct wiphy;
struct wlan_objmgr_pdev;
struct wlan_objmgr_vdev;
/**
 * struct spectral_cfg80211_vendor_cmd_handlers - Spectral vendor command
 * handlers
 * @wlan_cfg80211_spectral_scan_start: start scan handler
 * @wlan_cfg80211_spectral_scan_stop: stop scan handler
 * @wlan_cfg80211_spectral_scan_get_config: get config handler
 * @wlan_cfg80211_spectral_scan_get_diag_stats: get diag stats handler
 * @wlan_cfg80211_spectral_scan_get_cap: get capability handler
 * @wlan_cfg80211_spectral_scan_get_status: get status handler
 */
struct spectral_cfg80211_vendor_cmd_handlers {
	int (*wlan_cfg80211_spectral_scan_start)(struct wiphy *wiphy,
						 struct wlan_objmgr_pdev *pdev,
						 struct wlan_objmgr_vdev *vdev,
						 const void *data,
						 int data_len);
	int (*wlan_cfg80211_spectral_scan_stop)(struct wiphy *wiphy,
						struct wlan_objmgr_pdev *pdev,
						struct wlan_objmgr_vdev *vdev,
						const void *data,
						int data_len);
	int (*wlan_cfg80211_spectral_scan_get_config)(
						struct wiphy *wiphy,
						struct wlan_objmgr_pdev *pdev,
						struct wlan_objmgr_vdev *vdev,
						const void *data,
						int data_len);
	int (*wlan_cfg80211_spectral_scan_get_diag_stats)(
						struct wiphy *wiphy,
						struct wlan_objmgr_pdev *pdev,
						struct wlan_objmgr_vdev *vdev,
						const void *data,
						int data_len);
	int (*wlan_cfg80211_spectral_scan_get_cap)(
						struct wiphy *wiphy,
						struct wlan_objmgr_pdev *pdev,
						struct wlan_objmgr_vdev *vdev,
						const void *data,
						int data_len);
	int (*wlan_cfg80211_spectral_scan_get_status)(
						struct wiphy *wiphy,
						struct wlan_objmgr_pdev *pdev,
						struct wlan_objmgr_vdev *vdev,
						const void *data,
						int data_len);
};

/**
 * struct spectral_cp_param - Spectral control path data structure which
 * contains parameter and its value
 * @id: Parameter ID
 * @value: Single parameter value
 * @freq: Spectral scan frequency
 */
struct spectral_cp_param {
	uint32_t id;
	union {
		uint32_t value;
		struct spectral_config_frequency freq;
	};
};

/**
 * struct spectral_chan_stats - channel status info
 * @cycle_count:         Cycle count
 * @channel_load:        Channel load
 * @per:                 Period
 * @noisefloor:          Noise floor
 * @comp_usablity:       Computed usability
 * @maxregpower:         Maximum allowed regulatory power
 * @comp_usablity_sec80: Computed usability of secondary 80 Mhz
 * @maxregpower_sec80:   Max regulatory power of secondary 80 Mhz
 */
struct spectral_chan_stats {
	int cycle_count;
	int channel_load;
	int per;
	int noisefloor;
	uint16_t comp_usablity;
	int8_t maxregpower;
	uint16_t comp_usablity_sec80;
	int8_t maxregpower_sec80;
};

/**
 * struct spectral_diag_stats - spectral diag stats
 * @spectral_mismatch:             Spectral TLV signature mismatches
 * @spectral_sec80_sfft_insufflen: Insufficient length when parsing for
 *                                 Secondary 80 Search FFT report
 * @spectral_no_sec80_sfft:        Secondary 80 Search FFT report
 *                                 TLV not found
 * @spectral_vhtseg1id_mismatch:   VHT Operation Segment 1 ID
 *                                 mismatches in Search FFT report
 * @spectral_vhtseg2id_mismatch:   VHT Operation Segment 2 ID
 *                                 mismatches in Search FFT report
 * @spectral_invalid_detector_id:  Invalid detector id
 */
struct spectral_diag_stats {
	uint64_t spectral_mismatch;
	uint64_t spectral_sec80_sfft_insufflen;
	uint64_t spectral_no_sec80_sfft;
	uint64_t spectral_vhtseg1id_mismatch;
	uint64_t spectral_vhtseg2id_mismatch;
	uint64_t spectral_invalid_detector_id;
};

/**
 * struct spectral_scan_state - State of spectral scan
 * @is_active:  Is spectral scan active
 * @is_enabled: Is spectral scan enabled
 */
struct spectral_scan_state {
	uint8_t is_active;
	uint8_t is_enabled;
};

/* Forward declarations */
struct wlan_objmgr_pdev;

/**
 * struct spectral_nl_cb - Spectral Netlink callbacks
 * @get_sbuff:      Get the socket buffer to send the data to the application
 * @send_nl_bcast:  Send data to the application using netlink broadcast
 * @send_nl_unicast:  Send data to the application using netlink unicast
 * @free_sbuff: Free the socket buffer for a particular message type
 */
struct spectral_nl_cb {
	void *(*get_sbuff)(struct wlan_objmgr_pdev *pdev,
			   enum spectral_msg_type smsg_type,
			   enum spectral_msg_buf_type buf_type);
	int (*send_nl_bcast)(struct wlan_objmgr_pdev *pdev,
			     enum spectral_msg_type smsg_type);
	int (*send_nl_unicast)(struct wlan_objmgr_pdev *pdev,
			       enum spectral_msg_type smsg_type);
	void (*free_sbuff)(struct wlan_objmgr_pdev *pdev,
			   enum spectral_msg_type smsg_type);
	int (*convert_to_nl_ch_width)(uint8_t phy_chwidth);
	uint8_t (*convert_to_phy_ch_width)(uint8_t nl_chwidth);
};

/**
 * struct spectral_scan_config_request - Config request
 * @sscan_config: Spectral parameters
 * @sscan_err_code: Spectral scan error code
 */
struct spectral_scan_config_request {
	struct spectral_config sscan_config;
	enum spectral_cp_error_code sscan_err_code;
};

/**
 * struct spectral_scan_action_request - Action request
 * @sscan_err_code: Spectral scan error code
 */
struct spectral_scan_action_request {
	enum spectral_cp_error_code sscan_err_code;
};

/**
 * struct spectral_scan_get_caps_request - Get caps request
 * @sscan_caps: Spectral capabilities
 * @sscan_err_code: Spectral scan error code
 */
struct spectral_scan_get_caps_request {
	struct spectral_caps sscan_caps;
	enum spectral_cp_error_code sscan_err_code;
};

/**
 * struct spectral_scan_get_diag_request - Get diag request
 * @sscan_diag: Spectral diag stats
 * @sscan_err_code: Spectral scan error code
 */
struct spectral_scan_get_diag_request {
	struct spectral_diag_stats sscan_diag;
	enum spectral_cp_error_code sscan_err_code;
};

/**
 * struct spectral_scan_get_chan_width_request - Get channel width request
 * @chan_width: Channel width
 * @sscan_err_code: Spectral scan error code
 */
struct spectral_scan_get_chan_width_request {
	uint32_t chan_width;
	enum spectral_cp_error_code sscan_err_code;
};

/**
 * struct spectral_scan_get_status_request - Get status request
 * @is_active: is Spectral scan active
 * @is_enabled: is Spectral scan enabled
 * @sscan_err_code: Spectral scan error code
 */
struct spectral_scan_get_status_request {
	bool is_active;
	bool is_enabled;
	enum spectral_cp_error_code sscan_err_code;
};

/**
 * struct spectral_scan_debug_request - Get/set debug level request
 * @spectral_dbg_level: Spectral debug level
 * @sscan_err_code: Spectral scan error code
 */
struct spectral_scan_debug_request {
	uint32_t spectral_dbg_level;
	enum spectral_cp_error_code sscan_err_code;
};

/**
 * struct spectral_scan_dma_debug_request - DMA debug request
 * @dma_debug_enable: Enable/disable @dma_debug_type
 * @dma_debug_type: Type of Spectral DMA debug i.e., ring or buffer debug
 * @sscan_err_code: Spectral scan error code
 */
struct spectral_scan_dma_debug_request {
	bool dma_debug_enable;
	enum spectral_dma_debug dma_debug_type;
	enum spectral_cp_error_code sscan_err_code;
};

/**
 * struct spectral_cp_request - Spectral control path request
 *                              Creating request and extracting response has to
 *                              be atomic.
 * @ss_mode: Spectral scan mode
 * @req_id: Request identifier
 * @vdev_id: VDEV id
 * @dma_debug_req: Spectral DMA debug request
 */
struct spectral_cp_request {
	enum spectral_scan_mode ss_mode;
	uint8_t req_id;
	uint8_t vdev_id;
	union {
		struct spectral_scan_config_request config_req;
		struct spectral_scan_action_request action_req;
		struct spectral_scan_get_caps_request caps_req;
		struct spectral_scan_get_diag_request diag_req;
		struct spectral_scan_get_chan_width_request chan_width_req;
		struct spectral_scan_get_status_request status_req;
		struct spectral_scan_debug_request debug_req;
		struct spectral_scan_dma_debug_request dma_debug_req;
	};
};

#ifndef __KERNEL__

static inline int16_t
spectral_pwfactor_max(int16_t pwfactor1,
		      int16_t pwfactor2)
{
	return ((pwfactor1 > pwfactor2) ? pwfactor1 : pwfactor2);
}

/**
 * get_spectral_scale_rssi_corr() - Compute RSSI correction factor for scaling
 * @agc_total_gain_db: AGC total gain in dB steps
 * @gen3_defmaxgain: Default max gain value of the gen III chipset
 * @gen2_maxgain: Max gain value used by the reference gen II chipset
 * @lowlevel_offset: Low level offset for scaling
 * @inband_pwr: In band power in dB steps
 * @rssi_thr: RSSI threshold for scaling
 *
 * Helper function to compute RSSI correction factor for Gen III linear format
 * Spectral scaling. It is the responsibility of the caller to ensure that
 * correct values are passed.
 *
 * Return: RSSI correction factor
 */
static inline int16_t
get_spectral_scale_rssi_corr(u_int8_t agc_total_gain_db,
			     u_int8_t gen3_defmaxgain, u_int8_t gen2_maxgain,
			     int16_t lowlevel_offset, int16_t inband_pwr,
			     int16_t rssi_thr)
{
	return ((agc_total_gain_db < gen3_defmaxgain) ?
		(gen2_maxgain - gen3_defmaxgain + lowlevel_offset) :
		spectral_pwfactor_max((inband_pwr - rssi_thr), 0));
}

/**
 * spectral_scale_linear_to_gen2() - Scale linear bin value to gen II equivalent
 * @gen3_binmag: Captured FFT bin value from the Spectral Search FFT report
 * generated by the Gen III chipset
 * @gen2_maxgain: Max gain value used by the reference gen II chipset
 * @gen3_defmaxgain: Default max gain value of the gen III chipset
 * @lowlevel_offset: Low level offset for scaling
 * @inband_pwr: In band power in dB steps
 * @rssi_thr: RSSI threshold for scaling
 * @agc_total_gain_db: AGC total gain in dB steps
 * @highlevel_offset: High level offset for scaling
 * @gen2_bin_scale: Bin scale value used on reference gen II chipset
 * @gen3_bin_scale: Bin scale value used on gen III chipset
 *
 * Helper function to scale a given gen III linear format bin value into an
 * approximately equivalent gen II value. The scaled value can possibly be
 * higher than 8 bits.  If the caller is incapable of handling values larger
 * than 8 bits, the caller can saturate the value at 255. This function does not
 * carry out this saturation for the sake of flexibility so that callers
 * interested in the larger values can avail of this. Also note it is the
 * responsibility of the caller to ensure that correct values are passed.
 *
 * Return: Scaled bin value
 */
static inline u_int32_t
spectral_scale_linear_to_gen2(u_int8_t gen3_binmag,
			      u_int8_t gen2_maxgain, u_int8_t gen3_defmaxgain,
			      int16_t lowlevel_offset, int16_t inband_pwr,
			      int16_t rssi_thr, u_int8_t agc_total_gain_db,
			      int16_t highlevel_offset, u_int8_t gen2_bin_scale,
			      u_int8_t gen3_bin_scale)
{
	return (gen3_binmag *
		sqrt(pow(10, (((double)spectral_pwfactor_max(gen2_maxgain -
			gen3_defmaxgain + lowlevel_offset -
			get_spectral_scale_rssi_corr(agc_total_gain_db,
						     gen3_defmaxgain,
						     gen2_maxgain,
						     lowlevel_offset,
						     inband_pwr,
						     rssi_thr),
			(agc_total_gain_db < gen3_defmaxgain) *
				highlevel_offset)) / 10))) *
		 pow(2, (gen3_bin_scale - gen2_bin_scale)));
}

#endif /*  __KERNEL__  */

#endif				/* _WLAN_SPECTRAL_PUBLIC_STRUCTS_H_ */
