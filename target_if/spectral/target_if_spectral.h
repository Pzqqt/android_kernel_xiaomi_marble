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

#ifndef _TARGET_IF_SPECTRAL_H_
#define _TARGET_IF_SPECTRAL_H_

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_reg_services_api.h>
#include <qdf_lock.h>
#include <wlan_spectral_public_structs.h>
#include <reg_services_public_struct.h>
#ifdef DIRECT_BUF_RX_ENABLE
#include <target_if_direct_buf_rx_api.h>
#endif
#ifdef WIN32
#pragma pack(push, target_if_spectral, 1)
#define __ATTRIB_PACK
#else
#ifndef __ATTRIB_PACK
#define __ATTRIB_PACK __attribute__ ((packed))
#endif
#endif

#include <spectral_defs_i.h>
#include <wmi_unified_param.h>

#define FREQ_OFFSET_10MHZ (10)
#define FREQ_OFFSET_40MHZ (40)
#define FREQ_OFFSET_80MHZ (80)
#define FREQ_OFFSET_85MHZ (85)
#ifndef SPECTRAL_USE_NL_BCAST
#define SPECTRAL_USE_NL_BCAST  (0)
#endif

#define STATUS_PASS       1
#define STATUS_FAIL       0
#undef spectral_dbg_line
#define spectral_dbg_line() \
	spectral_debug("----------------------------------------------------")

#undef spectral_ops_not_registered
#define spectral_ops_not_registered(str) \
	spectral_info("SPECTRAL : %s not registered\n", (str))
#undef not_yet_implemented
#define not_yet_implemented() \
	spectral_info("SPECTRAL : %s : %d Not yet implemented\n", \
		      __func__, __LINE__)

#define SPECTRAL_HT20_NUM_BINS               56
#define SPECTRAL_HT20_FFT_LEN                56
#define SPECTRAL_HT20_DC_INDEX               (SPECTRAL_HT20_FFT_LEN / 2)
#define SPECTRAL_HT20_DATA_LEN               60
#define SPECTRAL_HT20_TOTAL_DATA_LEN         (SPECTRAL_HT20_DATA_LEN + 3)
#define SPECTRAL_HT40_TOTAL_NUM_BINS         128
#define SPECTRAL_HT40_DATA_LEN               135
#define SPECTRAL_HT40_TOTAL_DATA_LEN         (SPECTRAL_HT40_DATA_LEN + 3)
#define SPECTRAL_HT40_FFT_LEN                128
#define SPECTRAL_HT40_DC_INDEX               (SPECTRAL_HT40_FFT_LEN / 2)

/*
 * Used for the SWAR to obtain approximate combined rssi
 * in secondary 80Mhz segment
 */
#define OFFSET_CH_WIDTH_20	65
#define OFFSET_CH_WIDTH_40	62
#define OFFSET_CH_WIDTH_80	56
#define OFFSET_CH_WIDTH_160	50

/* Min and max for relevant Spectral params */
#define SPECTRAL_PARAM_FFT_SIZE_MIN_GEN2          (1)
#define SPECTRAL_PARAM_FFT_SIZE_MAX_GEN2          (9)
#define SPECTRAL_PARAM_FFT_SIZE_MIN_GEN3          (5)
#define SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_DEFAULT  (9)
#define SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_QCN9000  (10)
#define SPECTRAL_PARAM_FFT_SIZE_MIN_GEN3_BE       (5)
#define SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_BE       (11)
#define SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_BE_20MHZ (9)
#define SPECTRAL_PARAM_FFT_SIZE_MAX_GEN3_BE_40MHZ (10)
#define INVALID_FFT_SIZE                          (0xFFFF)
#define SPECTRAL_PARAM_RPT_MODE_MIN               (0)
#define SPECTRAL_PARAM_RPT_MODE_MAX               (3)
#define MAX_FFTBIN_VALUE                          (255)
#define SPECTRAL_DWORD_SIZE                       (4)

/* DBR ring debug size for Spectral */
#define SPECTRAL_DBR_RING_DEBUG_SIZE 512

#ifdef BIG_ENDIAN_HOST
#define SPECTRAL_MESSAGE_COPY_CHAR_ARRAY(destp, srcp, len)  do { \
	int j; \
	uint32_t *src, *dest; \
	src = (uint32_t *)(srcp); \
	dest = (uint32_t *)(destp); \
	for (j = 0; j < roundup((len), sizeof(uint32_t)) / 4; j++) { \
	*(dest + j) = qdf_le32_to_cpu(*(src + j)); \
	} \
	} while (0)
#else
#define SPECTRAL_MESSAGE_COPY_CHAR_ARRAY(destp, srcp, len) \
	OS_MEMCPY((destp), (srcp), (len));
#endif

#define DUMMY_NF_VALUE          (-123)
/* 5 categories x (lower + upper) bands */
#define MAX_INTERF                   10
#define HOST_MAX_ANTENNA         3
/* Mask for time stamp from descriptor */
#define SPECTRAL_TSMASK              0xFFFFFFFF
#define SPECTRAL_SIGNATURE           0xdeadbeef
/* Signature to write onto spectral buffer and then later validate */
#define MEM_POISON_SIGNATURE (htobe32(0xdeadbeef))

/* START of spectral GEN II HW specific details */
#define SPECTRAL_PHYERR_SIGNATURE_GEN2           0xbb
#define TLV_TAG_SPECTRAL_SUMMARY_REPORT_GEN2     0xF9
#define TLV_TAG_ADC_REPORT_GEN2                  0xFA
#define TLV_TAG_SEARCH_FFT_REPORT_GEN2           0xFB

/**
 * The Maximum number of detector informations to be filled in the SAMP msg
 * is 3, only for 165MHz case. For all other cases this value will be 1.
 */
#define MAX_NUM_DEST_DETECTOR_INFO    (3)
#define MAX_DETECTORS_PER_PDEV        (3)
#define FFT_BIN_SIZE_1BYTE            (1)

#ifdef OPTIMIZED_SAMP_MESSAGE
/**
 * enum spectral_160mhz_report_delivery_state - 160 MHz state machine states
 * @SPECTRAL_REPORT_WAIT_PRIMARY80:   Wait for primary80 report
 * @SPECTRAL_REPORT_WAIT_SECONDARY80: Wait for secondory 80 report
 */
enum spectral_160mhz_report_delivery_state {
	SPECTRAL_REPORT_WAIT_PRIMARY80,
	SPECTRAL_REPORT_WAIT_SECONDARY80,
};
#else
/**
 * enum spectral_160mhz_report_delivery_state - 160 MHz state machine states
 * @SPECTRAL_REPORT_WAIT_PRIMARY80:   Wait for primary80 report
 * @SPECTRAL_REPORT_RX_PRIMARY80:     Receive primary 80 report
 * @SPECTRAL_REPORT_WAIT_SECONDARY80: Wait for secondory 80 report
 * @SPECTRAL_REPORT_RX_SECONDARY80:   Receive secondary 80 report
 */
enum spectral_160mhz_report_delivery_state {
	SPECTRAL_REPORT_WAIT_PRIMARY80,
	SPECTRAL_REPORT_RX_PRIMARY80,
	SPECTRAL_REPORT_WAIT_SECONDARY80,
	SPECTRAL_REPORT_RX_SECONDARY80,
};
#endif /* OPTIMIZED_SAMP_MESSAGE */

/**
 * enum spectral_freq_span_id - Spectral frequency span id
 * @SPECTRAL_FREQ_SPAN_ID_0: Frequency span 0
 * @SPECTRAL_FREQ_SPAN_ID_1: Frequency span 1
 * @SPECTRAL_FREQ_SPAN_ID_2: Frequency span 2
 */
enum spectral_freq_span_id {
	SPECTRAL_FREQ_SPAN_ID_0,
	SPECTRAL_FREQ_SPAN_ID_1,
	SPECTRAL_FREQ_SPAN_ID_2,
};

/**
 * enum spectral_detector_id - Spectral detector id
 * @SPECTRAL_DETECTOR_ID_0: Spectral detector 0
 * @SPECTRAL_DETECTOR_ID_1: Spectral detector 1
 * @SPECTRAL_DETECTOR_ID_2: Spectral detector 2
 * @SPECTRAL_DETECTOR_ID_MAX: Max Spectral detector ID
 * @SPECTRAL_DETECTOR_ID_INVALID: Invalid Spectral detector ID
 */
enum spectral_detector_id {
	SPECTRAL_DETECTOR_ID_0,
	SPECTRAL_DETECTOR_ID_1,
	SPECTRAL_DETECTOR_ID_2,
	SPECTRAL_DETECTOR_ID_MAX,
	SPECTRAL_DETECTOR_ID_INVALID = 0xff,
};

/**
 * struct spectral_search_fft_info_gen2 - spectral search fft report for gen2
 * @relpwr_db:       Total bin power in db
 * @num_str_bins_ib: Number of strong bins
 * @base_pwr:        Base power
 * @total_gain_info: Total gain
 * @fft_chn_idx:     FFT chain on which report is originated
 * @avgpwr_db:       Average power in db
 * @peak_mag:        Peak power seen in the bins
 * @peak_inx:        Index of bin holding peak power
 */
struct spectral_search_fft_info_gen2 {
	uint32_t relpwr_db;
	uint32_t num_str_bins_ib;
	uint32_t base_pwr;
	uint32_t total_gain_info;
	uint32_t fft_chn_idx;
	uint32_t avgpwr_db;
	uint32_t peak_mag;
	int16_t  peak_inx;
};

/*
 * XXX Check if we should be handling the endinness difference in some
 * other way opaque to the host
 */
#ifdef BIG_ENDIAN_HOST

/**
 * struct spectral_phyerr_tlv_gen2 - phyerr tlv info for big endian host
 * @signature: signature
 * @tag:       tag
 * @length:    length
 */
struct spectral_phyerr_tlv_gen2 {
	uint8_t  signature;
	uint8_t  tag;
	uint16_t length;
} __ATTRIB_PACK;

#else

/**
 * struct spectral_phyerr_tlv_gen2 - phyerr tlv info for little endian host
 * @length:    length
 * @tag:       tag
 * @signature: signature
 */
struct spectral_phyerr_tlv_gen2 {
	uint16_t length;
	uint8_t  tag;
	uint8_t  signature;
} __ATTRIB_PACK;

#endif /* BIG_ENDIAN_HOST */

/**
 * struct spectral_phyerr_hdr_gen2 - phyerr header for gen2 HW
 * @hdr_a: Header[0:31]
 * @hdr_b: Header[32:63]
 */
struct spectral_phyerr_hdr_gen2 {
	uint32_t hdr_a;
	uint32_t hdr_b;
};

/*
 * Segment ID information for 80+80.
 *
 * If the HW micro-architecture specification extends this DWORD for other
 * purposes, then redefine+rename accordingly. For now, the specification
 * mentions only segment ID (though this doesn't require an entire DWORD)
 * without mention of any generic terminology for the DWORD, or any reservation.
 * We use nomenclature accordingly.
 */
typedef uint32_t SPECTRAL_SEGID_INFO;

/**
 * struct spectral_phyerr_fft_gen2 - fft info in phyerr event
 * @buf: fft report
 */
struct spectral_phyerr_fft_gen2 {
	uint8_t buf[0];
};

/**
 * struct spectral_process_phyerr_info_gen2 - Processed phyerr info structures
 * needed to fill SAMP params for gen2
 * @p_rfqual: Pointer to RF quality info
 * @p_sfft: Pointer to Search fft report info
 * @pfft: Pointer to FFT info in Phyerr event
 * @acs_stats: Pointer to ACS stats struct
 * @tsf64: 64 bit TSF value
 * @seg_d : Segment ID
 */
struct spectral_process_phyerr_info_gen2 {
	struct target_if_spectral_rfqual_info *p_rfqual;
	struct spectral_search_fft_info_gen2 *p_sfft;
	struct spectral_phyerr_fft_gen2 *pfft;
	struct target_if_spectral_acs_stats *acs_stats;
	uint64_t tsf64;
	uint8_t seg_id;
};

/* END of spectral GEN II HW specific details */

/* START of spectral GEN III HW specific details */

#define get_bitfield(value, size, pos) \
	(((value) >> (pos)) & ((1 << (size)) - 1))
#define unsigned_to_signed(value, width) \
	(((value) >= (1 << ((width) - 1))) ? \
		(value - (1 << (width))) : (value))

#define SSCAN_SUMMARY_REPORT_HDR_A_DETECTOR_ID_POS_GEN3         (29)
#define SSCAN_SUMMARY_REPORT_HDR_A_DETECTOR_ID_SIZE_GEN3        (2)
#define SSCAN_SUMMARY_REPORT_HDR_A_AGC_TOTAL_GAIN_POS_GEN3      (0)
#define SSCAN_SUMMARY_REPORT_HDR_A_AGC_TOTAL_GAIN_SIZE_GEN3     (8)
#define SSCAN_SUMMARY_REPORT_HDR_A_INBAND_PWR_DB_POS_GEN3       (18)
#define SSCAN_SUMMARY_REPORT_HDR_A_INBAND_PWR_DB_SIZE_GEN3      (10)
#define SSCAN_SUMMARY_REPORT_HDR_A_PRI80_POS_GEN3               (31)
#define SSCAN_SUMMARY_REPORT_HDR_A_PRI80_SIZE_GEN3              (1)
#define SSCAN_SUMMARY_REPORT_HDR_B_GAINCHANGE_POS_GEN3_V1       (30)
#define SSCAN_SUMMARY_REPORT_HDR_B_GAINCHANGE_SIZE_GEN3_V1      (1)
#define SSCAN_SUMMARY_REPORT_HDR_C_GAINCHANGE_POS_GEN3_V2       (16)
#define SSCAN_SUMMARY_REPORT_HDR_C_GAINCHANGE_SIZE_GEN3_V2      (1)
#define SPECTRAL_REPORT_LTS_HDR_LENGTH_POS_GEN3                 (0)
#define SPECTRAL_REPORT_LTS_HDR_LENGTH_SIZE_GEN3                (16)
#define SPECTRAL_REPORT_LTS_TAG_POS_GEN3                        (16)
#define SPECTRAL_REPORT_LTS_TAG_SIZE_GEN3                       (8)
#define SPECTRAL_REPORT_LTS_SIGNATURE_POS_GEN3                  (24)
#define SPECTRAL_REPORT_LTS_SIGNATURE_SIZE_GEN3                 (8)
#define FFT_REPORT_HDR_A_DETECTOR_ID_POS_GEN3                   (0)
#define FFT_REPORT_HDR_A_DETECTOR_ID_SIZE_GEN3                  (2)
#define FFT_REPORT_HDR_A_FFT_NUM_POS_GEN3                       (2)
#define FFT_REPORT_HDR_A_FFT_NUM_SIZE_GEN3                      (3)
#define FFT_REPORT_HDR_A_RADAR_CHECK_POS_GEN3_V1                (5)
#define FFT_REPORT_HDR_A_RADAR_CHECK_SIZE_GEN3_V1               (12)
#define FFT_REPORT_HDR_A_RADAR_CHECK_POS_GEN3_V2                (5)
#define FFT_REPORT_HDR_A_RADAR_CHECK_SIZE_GEN3_V2               (14)
#define FFT_REPORT_HDR_A_PEAK_INDEX_POS_GEN3_V1                 (17)
#define FFT_REPORT_HDR_A_PEAK_INDEX_SIZE_GEN3_V1                (11)
#define FFT_REPORT_HDR_A_PEAK_INDEX_POS_GEN3_V2                 (19)
#define FFT_REPORT_HDR_A_PEAK_INDEX_SIZE_GEN3_V2                (11)
#define FFT_REPORT_HDR_A_CHAIN_INDEX_POS_GEN3_V1                (28)
#define FFT_REPORT_HDR_A_CHAIN_INDEX_SIZE_GEN3_V1               (3)
#define FFT_REPORT_HDR_B_CHAIN_INDEX_POS_GEN3_V2                (0)
#define FFT_REPORT_HDR_B_CHAIN_INDEX_SIZE_GEN3_V2               (3)
#define FFT_REPORT_HDR_B_BASE_PWR_POS_GEN3_V1                   (0)
#define FFT_REPORT_HDR_B_BASE_PWR_SIZE_GEN3_V1                  (9)
#define FFT_REPORT_HDR_B_BASE_PWR_POS_GEN3_V2                   (3)
#define FFT_REPORT_HDR_B_BASE_PWR_SIZE_GEN3_V2                  (9)
#define FFT_REPORT_HDR_B_TOTAL_GAIN_POS_GEN3_V1                 (9)
#define FFT_REPORT_HDR_B_TOTAL_GAIN_SIZE_GEN3_V1                (8)
#define FFT_REPORT_HDR_B_TOTAL_GAIN_POS_GEN3_V2                 (12)
#define FFT_REPORT_HDR_B_TOTAL_GAIN_SIZE_GEN3_V2                (8)
#define FFT_REPORT_HDR_C_NUM_STRONG_BINS_POS_GEN3               (0)
#define FFT_REPORT_HDR_C_NUM_STRONG_BINS_SIZE_GEN3              (8)
#define FFT_REPORT_HDR_C_PEAK_MAGNITUDE_POS_GEN3                (8)
#define FFT_REPORT_HDR_C_PEAK_MAGNITUDE_SIZE_GEN3               (10)
#define FFT_REPORT_HDR_C_AVG_PWR_POS_GEN3                       (18)
#define FFT_REPORT_HDR_C_AVG_PWR_SIZE_GEN3                      (7)
#define FFT_REPORT_HDR_C_RELATIVE_PWR_POS_GEN3                  (25)
#define FFT_REPORT_HDR_C_RELATIVE_PWR_SIZE_GEN3                 (7)

#define SPECTRAL_PHYERR_SIGNATURE_GEN3                          (0xFA)
#define TLV_TAG_SPECTRAL_SUMMARY_REPORT_GEN3                    (0x02)
#define TLV_TAG_SEARCH_FFT_REPORT_GEN3                          (0x03)
#define SPECTRAL_PHYERR_TLVSIZE_GEN3                            (4)

#define NUM_SPECTRAL_DETECTORS_GEN3_V1                     (3)
#define NUM_SPECTRAL_DETECTORS_GEN3_V2                     (2)
#define FFT_REPORT_HEADER_LENGTH_GEN3_V2                   (24)
#define FFT_REPORT_HEADER_LENGTH_GEN3_V1                   (16)
#define NUM_PADDING_BYTES_SSCAN_SUMARY_REPORT_GEN3_V1      (0)
#define NUM_PADDING_BYTES_SSCAN_SUMARY_REPORT_GEN3_V2      (16)

#define SPECTRAL_PHYERR_HDR_LTS_POS \
	(offsetof(struct spectral_phyerr_fft_report_gen3, fft_hdr_lts))
#define SPECTRAL_FFT_BINS_POS \
	(offsetof(struct spectral_phyerr_fft_report_gen3, buf))

/**
 * struct phyerr_info - spectral search fft report for gen3
 * @data:       handle to phyerror buffer
 * @datalen:    length of phyerror bufer
 * @p_rfqual:   rf quality matrices
 * @p_chaninfo: pointer to chaninfo
 * @tsf64:      64 bit TSF
 * @acs_stats:  acs stats
 */
struct phyerr_info {
	uint8_t *data;
	uint32_t datalen;
	struct target_if_spectral_rfqual_info *p_rfqual;
	struct target_if_spectral_chan_info *p_chaninfo;
	uint64_t tsf64;
	struct target_if_spectral_acs_stats *acs_stats;
};

/**
 * struct spectral_search_fft_info_gen3 - spectral search fft report for gen3
 * @timestamp:           Timestamp at which fft report was generated
 * @last_raw_timestamp:  Previous FFT report's raw timestamp
 * @adjusted_timestamp:  Adjusted timestamp to account for target reset
 * @fft_detector_id:     Which radio generated this report
 * @fft_num:             The FFT count number. Set to 0 for short FFT.
 * @fft_radar_check:     NA for spectral
 * @fft_peak_sidx:       Index of bin with maximum power
 * @fft_chn_idx:         Rx chain index
 * @fft_base_pwr_db:     Base power in dB
 * @fft_total_gain_db:   Total gain in dB
 * @fft_num_str_bins_ib: Number of strong bins in the report
 * @fft_peak_mag:        Peak magnitude
 * @fft_avgpwr_db:       Average power in dB
 * @fft_relpwr_db:       Relative power in dB
 * @fft_bin_count:       Number of FFT bins in the FFT report
 * @fft_bin_size:        Size of one FFT bin in bytes
 * @bin_pwr_data:        Contains FFT bins extracted from the report
 */
struct spectral_search_fft_info_gen3 {
	uint32_t timestamp;
	uint32_t last_raw_timestamp;
	uint32_t adjusted_timestamp;
	uint32_t fft_detector_id;
	uint32_t fft_num;
	uint32_t fft_radar_check;
	int32_t  fft_peak_sidx;
	uint32_t fft_chn_idx;
	uint32_t fft_base_pwr_db;
	uint32_t fft_total_gain_db;
	uint32_t fft_num_str_bins_ib;
	int32_t  fft_peak_mag;
	uint32_t fft_avgpwr_db;
	uint32_t fft_relpwr_db;
	uint32_t fft_bin_count;
	uint8_t  fft_bin_size;
	uint8_t  *bin_pwr_data;
};

/**
 * struct spectral_phyerr_sfftreport_gen3 - fft info in phyerr event
 * @fft_timestamp:  Timestamp at which fft report was generated
 * @fft_hdr_lts:    length, tag, signature fields
 * @hdr_a:          Header[0:31]
 * @hdr_b:          Header[32:63]
 * @hdr_c:          Header[64:95]
 * @resv:           Header[96:127]
 * @buf:            fft bins
 */
struct spectral_phyerr_fft_report_gen3 {
	uint32_t fft_timestamp;
	uint32_t fft_hdr_lts;
	uint32_t hdr_a;
	uint32_t hdr_b;
	uint32_t hdr_c;
	uint32_t resv;
	uint8_t buf[0];
} __ATTRIB_PACK;

/**
 * struct sscan_report_fields_gen3 - Fields of spectral report
 * @sscan_agc_total_gain:  The AGC total gain in DB.
 * @inband_pwr_db: The in-band power of the signal in 1/2 DB steps
 * @sscan_gainchange: This bit is set to 1 if a gainchange occurred during
 *                 the spectral scan FFT.  Software may choose to
 *                 disregard the results.
 * @sscan_pri80: This is set to 1 to indicate that the Spectral scan was
 *                 performed on the pri80 segment. Software may choose to
 *                 disregard the FFT sample if this is set to 1 but detector ID
 *                 does not correspond to the ID for the pri80 segment.
 * @sscan_detector_id: Detector ID in Spectral scan report
 */
struct sscan_report_fields_gen3 {
	uint8_t sscan_agc_total_gain;
	int16_t inband_pwr_db;
	uint8_t sscan_gainchange;
	uint8_t sscan_pri80;
	uint8_t sscan_detector_id;
};

/**
 * struct spectral_sscan_summary_report_gen3 - Spectral summary report
 * event
 * @sscan_timestamp:  Timestamp at which fft report was generated
 * @sscan_hdr_lts:    length, tag, signature fields
 * @hdr_a:          Header[0:31]
 * @resv:           Header[32:63]
 * @hdr_b:          Header[64:95]
 * @hdr_c:          Header[96:127]
 */
struct spectral_sscan_summary_report_gen3 {
	u_int32_t sscan_timestamp;
	u_int32_t sscan_hdr_lts;
	u_int32_t hdr_a;
	u_int32_t res1;
	u_int32_t hdr_b;
	u_int32_t hdr_c;
} __ATTRIB_PACK;

#ifdef DIRECT_BUF_RX_ENABLE
/**
 * struct Spectral_report - spectral report
 * @data: Report buffer
 * @noisefloor: Noise floor values
 * @reset_delay: Time taken for warm reset in us
 * @cfreq1: center frequency 1
 * @cfreq2: center frequency 2
 * @ch_width: channel width
 */
struct spectral_report {
	uint8_t *data;
	int32_t noisefloor[DBR_MAX_CHAINS];
	uint32_t reset_delay;
	uint32_t cfreq1;
	uint32_t cfreq2;
	uint32_t ch_width;
};
#endif
/* END of spectral GEN III HW specific details */

typedef signed char pwr_dbm;

/**
 * enum spectral_gen - spectral hw generation
 * @SPECTRAL_GEN1 : spectral hw gen 1
 * @SPECTRAL_GEN2 : spectral hw gen 2
 * @SPECTRAL_GEN3 : spectral hw gen 3
 */
enum spectral_gen {
	SPECTRAL_GEN1,
	SPECTRAL_GEN2,
	SPECTRAL_GEN3,
};

/**
 * enum spectral_fftbin_size_war - spectral fft bin size war
 * @SPECTRAL_FFTBIN_SIZE_NO_WAR : No WAR applicable for Spectral FFT bin size
 * @SPECTRAL_FFTBIN_SIZE_2BYTE_TO_1BYTE : Spectral FFT bin size: Retain only
 *                                        least significant byte from 2 byte
 *                                        FFT bin transferred by HW
 * @SPECTRAL_FFTBIN_SIZE_4BYTE_TO_1BYTE : Spectral FFT bin size: Retain only
 *                                        least significant byte from 4 byte
 *                                        FFT bin transferred by HW
 */
enum spectral_fftbin_size_war {
	SPECTRAL_FFTBIN_SIZE_NO_WAR = 0,
	SPECTRAL_FFTBIN_SIZE_WAR_2BYTE_TO_1BYTE = 1,
	SPECTRAL_FFTBIN_SIZE_WAR_4BYTE_TO_1BYTE = 2,
};

/**
 * enum spectral_report_format_version - This represents the report format
 * version number within each Spectral generation.
 * @SPECTRAL_REPORT_FORMAT_VERSION_1 : version 1
 * @SPECTRAL_REPORT_FORMAT_VERSION_2 : version 2
 */
enum spectral_report_format_version {
	SPECTRAL_REPORT_FORMAT_VERSION_1,
	SPECTRAL_REPORT_FORMAT_VERSION_2,
};

/**
 * struct spectral_fft_bin_len_adj_swar - Encapsulate information required for
 * Spectral FFT bin length adjusting software WARS.
 * @inband_fftbin_size_adj: Whether to carry out FFT bin size adjustment for
 * in-band report format. This would be required on some chipsets under the
 * following circumstances: In report mode 2 only the in-band bins are DMA'ed.
 * Scatter/gather is used. However, the HW generates all bins, not just in-band,
 * and reports the number of bins accordingly. The subsystem arranging for the
 * DMA cannot change this value. On such chipsets the adjustment required at the
 * host driver is to check if report format is 2, and if so halve the number of
 * bins reported to get the number actually DMA'ed.
 * @null_fftbin_adj: Whether to remove NULL FFT bins for report mode (1) in
 * which only summary of metrics for each completed FFT + spectral scan summary
 * report are to be provided. This would be required on some chipsets under the
 * following circumstances: In report mode 1, HW reports a length corresponding
 * to all bins, and provides bins with value 0. This is because the subsystem
 * arranging for the FFT information does not arrange for DMA of FFT bin values
 * (as expected), but cannot arrange for a smaller length to be reported by HW.
 * In these circumstances, the driver would have to disregard the NULL bins and
 * report a bin count of 0 to higher layers.
 * @packmode_fftbin_size_adj: Pack mode in HW refers to packing of each Spectral
 * FFT bin into 2 bytes. But due to a bug HW reports 2 times the expected length
 * when packmode is enabled. This SWAR compensates this bug by dividing the
 * length with 2.
 * @fftbin_size_war: Type of FFT bin size SWAR
 */
struct spectral_fft_bin_len_adj_swar {
	u_int8_t inband_fftbin_size_adj;
	u_int8_t null_fftbin_adj;
	uint8_t packmode_fftbin_size_adj;
	enum spectral_fftbin_size_war fftbin_size_war;
};

/**
 * struct spectral_report_params - Parameters related to format of Spectral
 * report.
 * @version: This represents the report format version number within each
 * Spectral generation.
 * @ssumaary_padding_bytes: Number of bytes of padding after Spectral summary
 * report
 * @fft_report_hdr_len: Number of bytes in the header of the FFT report. This
 * has to be subtracted from the length field of FFT report to find the length
 * of FFT bins.
 * @fragmentation_160: This indicates whether Spectral reports in 160/80p80 is
 * fragmented.
 * @detid_mode_table: Detector ID to Spectral scan mode table
 * @num_spectral_detectors: Total number of Spectral detectors
 * @marker: Describes the boundaries of pri80, 5 MHz and sec80 bins
 * @hw_fft_bin_width: FFT bin width reported by the HW
 */
struct spectral_report_params {
	enum spectral_report_format_version version;
	uint8_t ssumaary_padding_bytes;
	uint8_t fft_report_hdr_len;
	bool fragmentation_160[SPECTRAL_SCAN_MODE_MAX];
	enum spectral_scan_mode detid_mode_table[SPECTRAL_DETECTOR_ID_MAX];
	uint8_t num_spectral_detectors;
	struct spectral_fft_bin_markers_160_165mhz
				marker[SPECTRAL_SCAN_MODE_MAX];
	uint8_t hw_fft_bin_width;
};

/**
 * struct spectral_param_min_max - Spectral parameter minimum and maximum values
 * @fft_size_min: Minimum value of fft_size
 * @fft_size_max: Maximum value of fft_size for each BW
 */
struct spectral_param_min_max {
	uint16_t fft_size_min;
	uint16_t fft_size_max[CH_WIDTH_MAX];
};

/**
 * struct spectral_timestamp_swar - Spectral time stamp WAR related parameters
 * @timestamp_war_offset: Offset to be added to correct timestamp
 * @target_reset_count: Number of times target exercised the reset routine
 * @last_fft_timestamp: last fft report timestamp
 */
struct spectral_timestamp_war {
	uint32_t timestamp_war_offset[SPECTRAL_SCAN_MODE_MAX];
	uint64_t target_reset_count;
	uint32_t last_fft_timestamp[SPECTRAL_SCAN_MODE_MAX];
};

#if ATH_PERF_PWR_OFFLOAD
/**
 * enum target_if_spectral_info - Enumerations for specifying which spectral
 *                              information (among parameters and states)
 *                              is desired.
 * @TARGET_IF_SPECTRAL_INFO_ACTIVE:  Indicated whether spectral is active
 * @TARGET_IF_SPECTRAL_INFO_ENABLED: Indicated whether spectral is enabled
 * @TARGET_IF_SPECTRAL_INFO_PARAMS:  Config params
 */
enum target_if_spectral_info {
	TARGET_IF_SPECTRAL_INFO_ACTIVE,
	TARGET_IF_SPECTRAL_INFO_ENABLED,
	TARGET_IF_SPECTRAL_INFO_PARAMS,
};
#endif /* ATH_PERF_PWR_OFFLOAD */

/* forward declaration */
struct target_if_spectral;

/**
 * struct target_if_spectral_chan_info - Channel information
 * @center_freq1: center frequency 1 in MHz
 * @center_freq2: center frequency 2 in MHz -valid only for
 *		 11ACVHT 80PLUS80 mode
 * @chan_width:   channel width in MHz
 */
struct target_if_spectral_chan_info {
	uint16_t    center_freq1;
	uint16_t    center_freq2;
	uint8_t     chan_width;
};

/**
 * struct target_if_spectral_acs_stats - EACS stats from spectral samples
 * @nfc_ctl_rssi: Control chan rssi
 * @nfc_ext_rssi: Extension chan rssi
 * @ctrl_nf:      Control chan Noise Floor
 * @ext_nf:       Extension chan Noise Floor
 */
struct target_if_spectral_acs_stats {
	int8_t nfc_ctl_rssi;
	int8_t nfc_ext_rssi;
	int8_t ctrl_nf;
	int8_t ext_nf;
};

/**
 * struct target_if_spectral_perchain_rssi_info - per chain rssi info
 * @rssi_pri20: Rssi of primary 20 Mhz
 * @rssi_sec20: Rssi of secondary 20 Mhz
 * @rssi_sec40: Rssi of secondary 40 Mhz
 * @rssi_sec80: Rssi of secondary 80 Mhz
 */
struct target_if_spectral_perchain_rssi_info {
	int8_t    rssi_pri20;
	int8_t    rssi_sec20;
	int8_t    rssi_sec40;
	int8_t    rssi_sec80;
};

/**
 * struct target_if_spectral_rfqual_info - RF measurement information
 * @rssi_comb:    RSSI Information
 * @pc_rssi_info: XXX : For now, we know we are getting information
 *                for only 4 chains at max. For future extensions
 *                use a define
 * @noise_floor:  Noise floor information
 */
struct target_if_spectral_rfqual_info {
	int8_t    rssi_comb;
	struct    target_if_spectral_perchain_rssi_info pc_rssi_info[4];
	int16_t   noise_floor[4];
};

#define GET_TARGET_IF_SPECTRAL_OPS(spectral) \
	((struct target_if_spectral_ops *)(&((spectral)->spectral_ops)))

/**
 * struct target_if_spectral_ops - spectral low level ops table
 * @get_tsf64:               Get 64 bit TSF value
 * @get_capability:          Get capability info
 * @set_rxfilter:            Set rx filter
 * @get_rxfilter:            Get rx filter
 * @is_spectral_active:      Check whether icm is active
 * @is_spectral_enabled:     Check whether spectral is enabled
 * @start_spectral_scan:     Start spectral scan
 * @stop_spectral_scan:      Stop spectral scan
 * @get_extension_channel:   Get extension channel
 * @get_ctl_noisefloor:      Get control noise floor
 * @get_ext_noisefloor:      Get extension noise floor
 * @configure_spectral:      Set spectral configurations
 * @get_spectral_config:     Get spectral configurations
 * @get_ent_spectral_mask:   Get spectral mask
 * @get_mac_address:         Get mac address
 * @get_current_channel:     Get current channel
 * @reset_hw:                Reset HW
 * @get_chain_noise_floor:   Get Channel noise floor
 * @spectral_process_phyerr: Process phyerr event
 * @process_spectral_report: Process spectral report
 * @byte_swap_headers:       Apply byte-swap on report headers
 * @byte_swap_fft_bins:      Apply byte-swap on FFT bins
 */
struct target_if_spectral_ops {
	uint64_t (*get_tsf64)(void *arg);
	uint32_t (*get_capability)(
		void *arg, enum spectral_capability_type type);
	uint32_t (*set_rxfilter)(void *arg, int rxfilter);
	uint32_t (*get_rxfilter)(void *arg);
	uint32_t (*is_spectral_active)(void *arg,
				       enum spectral_scan_mode smode);
	uint32_t (*is_spectral_enabled)(void *arg,
					enum spectral_scan_mode smode);
	uint32_t (*start_spectral_scan)(void *arg,
					enum spectral_scan_mode smode,
					enum spectral_cp_error_code *err);
	uint32_t (*stop_spectral_scan)(void *arg,
				       enum spectral_scan_mode smode);
	uint32_t (*get_extension_channel)(void *arg,
					  enum spectral_scan_mode smode);
	int8_t    (*get_ctl_noisefloor)(void *arg);
	int8_t    (*get_ext_noisefloor)(void *arg);
	uint32_t (*configure_spectral)(
			void *arg,
			struct spectral_config *params,
			enum spectral_scan_mode smode);
	uint32_t (*get_spectral_config)(
			void *arg,
			struct spectral_config *params,
			enum spectral_scan_mode smode);
	uint32_t (*get_ent_spectral_mask)(void *arg);
	uint32_t (*get_mac_address)(void *arg, char *addr);
	uint32_t (*get_current_channel)(void *arg,
					enum spectral_scan_mode smode);
	uint32_t (*reset_hw)(void *arg);
	uint32_t (*get_chain_noise_floor)(void *arg, int16_t *nf_buf);
	int (*spectral_process_phyerr)(struct target_if_spectral *spectral,
				       uint8_t *data, uint32_t datalen,
			struct target_if_spectral_rfqual_info *p_rfqual,
			struct target_if_spectral_chan_info *p_chaninfo,
			uint64_t tsf64,
			struct target_if_spectral_acs_stats *acs_stats);
	int (*process_spectral_report)(struct wlan_objmgr_pdev *pdev,
				       void *payload);
	QDF_STATUS (*byte_swap_headers)(
		struct target_if_spectral *spectral,
		void *data);
	QDF_STATUS (*byte_swap_fft_bins)(
		const struct spectral_report_params *rparams,
		void *bin_pwr_data, size_t num_fftbins);
};

/**
 * struct target_if_spectral_stats - spectral stats info
 * @num_spectral_detects: Total num. of spectral detects
 * @total_phy_errors:     Total number of phyerrors
 * @owl_phy_errors:       Indicated phyerrors in old gen1 chipsets
 * @pri_phy_errors:       Phyerrors in primary channel
 * @ext_phy_errors:       Phyerrors in secondary channel
 * @dc_phy_errors:        Phyerrors due to dc
 * @early_ext_phy_errors: Early secondary channel phyerrors
 * @bwinfo_errors:        Bandwidth info errors
 * @datalen_discards:     Invalid data length errors, seen in gen1 chipsets
 * @rssi_discards bw:     Indicates reports dropped due to RSSI threshold
 * @last_reset_tstamp:    Last reset time stamp
 */
struct target_if_spectral_stats {
	uint32_t    num_spectral_detects;
	uint32_t    total_phy_errors;
	uint32_t    owl_phy_errors;
	uint32_t    pri_phy_errors;
	uint32_t    ext_phy_errors;
	uint32_t    dc_phy_errors;
	uint32_t    early_ext_phy_errors;
	uint32_t    bwinfo_errors;
	uint32_t    datalen_discards;
	uint32_t    rssi_discards;
	uint64_t    last_reset_tstamp;
};

/**
 * struct target_if_spectral_event - spectral event structure
 * @se_ts:        Original 15 bit recv timestamp
 * @se_full_ts:   64-bit full timestamp from interrupt time
 * @se_rssi:      Rssi of spectral event
 * @se_bwinfo:    Rssi of spectral event
 * @se_dur:       Duration of spectral pulse
 * @se_chanindex: Channel of event
 * @se_list:      List of spectral events
 */
struct target_if_spectral_event {
	uint32_t                       se_ts;
	uint64_t                       se_full_ts;
	uint8_t                        se_rssi;
	uint8_t                        se_bwinfo;
	uint8_t                        se_dur;
	uint8_t                        se_chanindex;

	STAILQ_ENTRY(spectral_event)    se_list;
};

/**
 * struct target_if_chain_noise_pwr_info - Noise power info for each channel
 * @rptcount:        Count of reports in pwr array
 * @un_cal_nf:       Uncalibrated noise floor
 * @factory_cal_nf:  Noise floor as calibrated at the factory for module
 * @median_pwr:      Median power (median of pwr array)
 * @pwr:             Power reports
 */
struct target_if_chain_noise_pwr_info {
	int        rptcount;
	pwr_dbm    un_cal_nf;
	pwr_dbm    factory_cal_nf;
	pwr_dbm    median_pwr;
	pwr_dbm    pwr[];
} __ATTRIB_PACK;

/**
 * struct target_if_spectral_chan_stats - Channel information
 * @cycle_count:         Cycle count
 * @channel_load:        Channel load
 * @per:                 Period
 * @noisefloor:          Noise floor
 * @comp_usablity:       Computed usability
 * @maxregpower:         Maximum allowed regulatary power
 * @comp_usablity_sec80: Computed usability of secondary 80 Mhz
 * @maxregpower_sec80:   Max regulatory power in secondary 80 Mhz
 */
struct target_if_spectral_chan_stats {
	int          cycle_count;
	int          channel_load;
	int          per;
	int          noisefloor;
	uint16_t    comp_usablity;
	int8_t       maxregpower;
	uint16_t    comp_usablity_sec80;
	int8_t       maxregpower_sec80;
};

#if ATH_PERF_PWR_OFFLOAD

/**
 * struct target_if_spectral_cache - Cache used to minimize WMI operations
 *                             in offload architecture
 * @osc_spectral_enabled: Whether Spectral is enabled
 * @osc_spectral_active:  Whether spectral is active
 *                        XXX: Ideally, we should NOT cache this
 *                        since the hardware can self clear the bit,
 *                        the firmware can possibly stop spectral due to
 *                        intermittent off-channel activity, etc
 *                        A WMI read command should be introduced to handle
 *                        this This will be discussed.
 * @osc_params:           Spectral parameters
 * @osc_is_valid:         Whether the cache is valid
 */
struct target_if_spectral_cache {
	uint8_t                  osc_spectral_enabled;
	uint8_t                  osc_spectral_active;
	struct spectral_config    osc_params;
	uint8_t                  osc_is_valid;
};

/**
 * struct target_if_spectral_param_state_info - Structure used to represent and
 *                                        manage spectral information
 *                                        (parameters and states)
 * @osps_lock:  Lock to synchronize accesses to information
 * @osps_cache: Cacheable' information
 */
struct target_if_spectral_param_state_info {
	qdf_spinlock_t               osps_lock;
	struct target_if_spectral_cache    osps_cache;
	/* XXX - Non-cacheable information goes here, in the future */
};
#endif /* ATH_PERF_PWR_OFFLOAD */

struct vdev_spectral_configure_params;
struct vdev_spectral_enable_params;

/**
 * struct spectral_wmi_ops - structure used holding the operations
 * related to Spectral WMI
 * @wmi_spectral_configure_cmd_send: Configure Spectral parameters
 * @wmi_spectral_enable_cmd_send: Enable/Disable Spectral
 * @wmi_spectral_crash_inject: Inject FW crash
 * @wmi_extract_pdev_sscan_fw_cmd_fixed_param: Extract Fixed params from
 * start scan response event
 * @wmi_extract_pdev_sscan_fft_bin_index: Extract TLV which describes FFT
 * bin indices from start scan response event
 * @wmi_unified_register_event_handler: Register WMI event handler
 * @wmi_unified_unregister_event_handler: Unregister WMI event handler
 * @wmi_service_enabled: API to check whether a given WMI service is enabled
 * @extract_pdev_spectral_session_chan_info: Extract Spectral scan session
 * channel information
 * @extract_pdev_spectral_session_detector_info: Extract Spectral scan session
 * detector information
 */
struct spectral_wmi_ops {
	QDF_STATUS (*wmi_spectral_configure_cmd_send)(
		    wmi_unified_t wmi_hdl,
		    struct vdev_spectral_configure_params *param);
	QDF_STATUS (*wmi_spectral_enable_cmd_send)(
		    wmi_unified_t wmi_hdl,
		    struct vdev_spectral_enable_params *param);
	QDF_STATUS (*wmi_spectral_crash_inject)(
		wmi_unified_t wmi_handle, struct crash_inject *param);
	QDF_STATUS (*wmi_extract_pdev_sscan_fw_cmd_fixed_param)(
				wmi_unified_t wmi_handle, uint8_t *evt_buf,
				struct spectral_startscan_resp_params *param);
	QDF_STATUS (*wmi_extract_pdev_sscan_fft_bin_index)(
			wmi_unified_t wmi_handle, uint8_t *evt_buf,
			struct spectral_fft_bin_markers_160_165mhz *param);
	QDF_STATUS (*wmi_unified_register_event_handler)(
				wmi_unified_t wmi_handle,
				wmi_conv_event_id event_id,
				wmi_unified_event_handler handler_func,
				uint8_t rx_ctx);
	QDF_STATUS (*wmi_unified_unregister_event_handler)(
				wmi_unified_t wmi_handle,
				wmi_conv_event_id event_id);
	bool (*wmi_service_enabled)(wmi_unified_t wmi_handle,
				    uint32_t service_id);
	QDF_STATUS (*extract_pdev_spectral_session_chan_info)(
			wmi_unified_t wmi_handle, void *event,
			struct spectral_session_chan_info *chan_info);
	QDF_STATUS (*extract_pdev_spectral_session_detector_info)(
		wmi_unified_t wmi_handle, void *event,
		struct spectral_session_det_info *det_info,
		uint8_t det_info_idx);
};

/**
 * struct spectral_tgt_ops - structure used holding the operations
 * related to target operations
 * @tgt_get_psoc_from_scn_hdl: Function to get psoc from scn
 */
struct spectral_tgt_ops {
	struct wlan_objmgr_psoc *(*tgt_get_psoc_from_scn_hdl)(void *scn_handle);
};

/**
 * struct spectral_param_properties - structure holding Spectral
 *                                    parameter properties
 * @supported: Parameter is supported or not
 * @common_all_modes: Parameter should be common for all modes or not
 */
struct spectral_param_properties {
	bool supported;
	bool common_all_modes;
};

/**
 * struct target_if_finite_spectral_scan_params - Parameters related to finite
 * Spectral scan
 * @finite_spectral_scan: Indicates the Spectrl scan is finite/infinite
 * @num_reports_expected: Number of Spectral reports expected from target for a
 * finite Spectral scan
 */
struct target_if_finite_spectral_scan_params {
	bool finite_spectral_scan;
	uint32_t num_reports_expected;
};

/**
 * struct per_session_dest_det_info - Per-session Detector information to be
 * filled to samp_detector_info
 * @freq_span_id: Contiguous frequency span ID within the SAMP message
 * @is_sec80: Indicates pri80/sec80 segment for 160/80p80 BW
 * @det_id: Detector ID within samp_freq_span_info corresponding to
 * freq_span_id
 * @dest_start_bin_idx: Start index of FFT bins within SAMP msg's bin_pwr array
 * @dest_end_bin_idx: End index of FFT bins within SAMP msg's bin_pwr array
 * @lb_extrabins_start_idx: Left band edge extra bins start index
 * @lb_extrabins_num: Number of left band edge extra bins
 * @rb_extrabins_start_idx: Right band edge extra bins start index
 * @rb_extrabins_num: Number of right band edge extra bins
 * @start_freq: Indicates start frequency per-detector (in MHz)
 * @end_freq: Indicates last frequency per-detector (in MHz)
 * @src_start_bin_idx: Start index within the Spectral report's bin_pwr array,
 * where the FFT bins corresponding to this dest_det_id start
 */
struct per_session_dest_det_info {
	uint8_t freq_span_id;
	bool is_sec80;
	uint8_t det_id;
	uint16_t dest_start_bin_idx;
	uint16_t dest_end_bin_idx;
	uint16_t lb_extrabins_start_idx;
	uint16_t lb_extrabins_num;
	uint16_t rb_extrabins_start_idx;
	uint16_t rb_extrabins_num;
	uint32_t start_freq;
	uint32_t end_freq;
	uint16_t src_start_bin_idx;
};

/**
 * struct per_session_det_map - A map of per-session detector information,
 * keyed by the detector id obtained from the Spectral FFT report, mapping to
 * detination detector info in SAMP message.
 * @dest_det_info: Struct containing per-session detector information
 * @num_dest_det_info: Number of destination detectors to which information
 * of this detector is to be filled
 * @spectral_msg_buf_type: Spectral message buffer type
 * @send_to_upper_layers: Indicates whether to send SAMP msg to upper layers
 * @det_map_valid: Indicates whether detector map is valid or not
 */
struct per_session_det_map {
	struct per_session_dest_det_info
			dest_det_info[MAX_NUM_DEST_DETECTOR_INFO];
	uint8_t num_dest_det_info;
	enum spectral_msg_buf_type buf_type;
	bool send_to_upper_layers;
	bool det_map_valid;
};

/**
 * struct per_session_report_info - Consists of per-session Spectral report
 * information to be filled at report level in SAMP message.
 * @pri20_freq: Primary 20MHz operating frequency in MHz
 * @cfreq1: Centre frequency of the frequency span for 20/40/80 MHz BW.
 * Segment 1 centre frequency in MHz for 80p80/160 BW.
 * @cfreq2: For 80p80, indicates segment 2 centre frequency in MHz. For 160MHz,
 * indicates the center frequency of 160MHz span.
 * @operating_bw: Device's operating bandwidth.Valid values = enum phy_ch_width
 * @sscan_cfreq1: Normal/Agile scan Centre frequency of the frequency span for
 * 20/40/80 MHz BW. Center frequency of Primary Segment in MHz for 80p80/160 BW
 * Based on Spectral scan mode.
 * @sscan_cfreq2: For 80p80, Normal/Agile scan Center frequency for Sec80
 * segment. For 160MHz, indicates the center frequency of 160MHz span. Based on
 * spectral scan mode
 * @sscan_bw: Normal/Agile Scan BW based on Spectral scan mode.
 * Valid values = enum phy_ch_width
 * @num_spans: Number of frequency spans
 * @valid: Indicated whether report info is valid
 */
struct per_session_report_info {
	uint32_t pri20_freq;
	uint32_t cfreq1;
	uint32_t cfreq2;
	enum phy_ch_width operating_bw;
	uint32_t sscan_cfreq1;
	uint32_t sscan_cfreq2;
	enum phy_ch_width sscan_bw;
	uint8_t num_spans;
	bool valid;
};

/**
 * struct sscan_detector_list - Spectral scan Detector list, for given Spectral
 * scan mode and operating BW
 * @detectors: List of detectors
 * @num_detectors: Number of detectors for given spectral scan mode, BW
 *                 and target type
 */
struct sscan_detector_list {
	uint8_t detectors[SPECTRAL_DETECTOR_ID_MAX];
	uint8_t num_detectors;
};

/**
 * struct spectral_supported_bws - Supported sscan bandwidths
 * @bandwidths: bitmap of supported sscan bandwidths. Make sure to maintain this
 * bitmap in the increasing order of bandwidths.
 */
struct spectral_supported_bws {
	union {
		struct {
			uint32_t supports_sscan_bw_5:1,
				 supports_sscan_bw_10:1,
				 supports_sscan_bw_20:1,
				 supports_sscan_bw_40:1,
				 supports_sscan_bw_80:1,
				 supports_sscan_bw_160:1,
				 supports_sscan_bw_80_80:1,
				 supports_sscan_bw_320:1,
				 reserved:24;
		};
		uint32_t bandwidths;
	};
};

/**
 * get_supported_sscan_bw_pos() - Get the position of a given sscan_bw inside
 * the supported sscan bandwidths bitmap
 * @sscan_bw: Spectral scan bandwidth
 *
 * Return: bit position for a valid sscan bandwidth, else -1
 */
int get_supported_sscan_bw_pos(enum phy_ch_width sscan_bw);

/**
 * struct target_if_spectral - main spectral structure
 * @pdev: Pointer to pdev
 * @spectral_ops: Target if internal Spectral low level operations table
 * @capability: Spectral capabilities structure
 * @properties: Spectral parameter properties per mode
 * @spectral_lock: Lock used for internal Spectral operations
 * @vdev_id: VDEV id for all spectral modes
 * @spectral_curchan_radindex: Current channel spectral index
 * @spectral_extchan_radindex: Extension channel spectral index
 * @spectraldomain: Current Spectral domain
 * @spectral_proc_phyerr:  Flags to process for PHY errors
 * @spectral_defaultparams: Default PHY params per Spectral stat
 * @spectral_stats:  Spectral related stats
 * @events:   Events structure
 * @sc_spectral_ext_chan_ok:  Can spectral be detected on the extension channel?
 * @sc_spectral_combined_rssi_ok:  Can use combined spectral RSSI?
 * @sc_spectral_20_40_mode:  Is AP in 20-40 mode?
 * @sc_spectral_noise_pwr_cal:  Noise power cal required?
 * @sc_spectral_non_edma:  Is the spectral capable device Non-EDMA?
 * @upper_is_control: Upper segment is primary
 * @upper_is_extension: Upper segment is secondary
 * @lower_is_control: Lower segment is primary
 * @lower_is_extension: Lower segment is secondary
 * @sc_spectraltest_ieeechan:  IEEE channel number to return to after a spectral
 * mute test
 * @spectral_numbins: Number of bins
 * @spectral_fft_len: FFT length
 * @spectral_data_len: Total phyerror report length
 * @lb_edge_extrabins: Number of extra bins on left band edge
 * @rb_edge_extrabins: Number of extra bins on right band edge
 * @spectral_max_index_offset: Max FFT index offset (20 MHz mode)
 * @spectral_upper_max_index_offset: Upper max FFT index offset (20/40 MHz mode)
 * @spectral_lower_max_index_offset: Lower max FFT index offset (20/40 MHz mode)
 * @spectral_dc_index: At which index DC is present
 * @send_single_packet: Deprecated
 * @spectral_sent_msg: Indicates whether we send report to upper layers
 * @params: Spectral parameters
 * @last_capture_time: Indicates timestamp of previouse report
 * @num_spectral_data: Number of Spectral samples received in current session
 * @total_spectral_data: Total number of Spectral samples received
 * @max_rssi: Maximum RSSI
 * @detects_control_channel: NA
 * @detects_extension_channel: NA
 * @detects_below_dc: NA
 * @detects_above_dc: NA
 * @sc_scanning: Indicates active wifi scan
 * @sc_spectral_scan: Indicates active specral scan
 * @sc_spectral_full_scan: Deprecated
 * @scan_start_tstamp: Deprecated
 * @last_tstamp: Deprecated
 * @first_tstamp: Deprecated
 * @spectral_samp_count: Deprecated
 * @sc_spectral_samp_count: Deprecated
 * @noise_pwr_reports_reqd: Number of noise power reports required
 * @noise_pwr_reports_recv: Number of noise power reports received
 * @noise_pwr_reports_lock: Lock used for Noise power report processing
 * @noise_pwr_chain_ctl: Noise power report - control channel
 * @noise_pwr_chain_ext: Noise power report - extension channel
 * @chaninfo: Channel statistics
 * @tsf64: Latest TSF Value
 * @param_info: Offload architecture Spectral parameter cache information
 * @ch_width: Indicates Channel Width 20/40/80/160 MHz for each Spectral mode
 * @sscan_width_configured: Whether user has configured sscan bandwidth
 * @diag_stats: Diagnostic statistics
 * @is_160_format:  Indicates whether information provided by HW is in altered
 * format for 802.11ac 160/80+80 MHz support (QCA9984 onwards)
 * @is_lb_edge_extrabins_format:  Indicates whether information provided by
 * HW has 4 extra bins, at left band edge, for report mode 2
 * @is_rb_edge_extrabins_format:   Indicates whether information provided
 * by HW has 4 extra bins, at right band edge, for report mode 2
 * @is_sec80_rssi_war_required: Indicates whether the software workaround is
 * required to obtain approximate combined RSSI for secondary 80Mhz segment
 * @simctx: Spectral Simulation context
 * @spectral_gen: Spectral hardware generation
 * @hdr_sig_exp: Expected signature in PHYERR TLV header, for the given hardware
 * generation
 * @tag_sscan_summary_exp: Expected Spectral Scan Summary tag in PHYERR TLV
 * header, for the given hardware generation
 * @tag_sscan_fft_exp: Expected Spectral Scan FFT report tag in PHYERR TLV
 * header, for the given hardware generation
 * @tlvhdr_size: Expected PHYERR TLV header size, for the given hardware
 * generation
 * @nl_cb: Netlink callbacks
 * @use_nl_bcast: Whether to use Netlink broadcast/unicast
 * @send_phy_data: Send data to the application layer for a particular msg type
 * @len_adj_swar: Spectral fft bin length adjustment SWAR related info
 * @timestamp_war: Spectral time stamp WAR related info
 * @dbr_ring_debug: Whether Spectral DBR ring debug is enabled
 * @dbr_buff_debug: Whether Spectral DBR buffer debug is enabled
 * @direct_dma_support: Whether Direct-DMA is supported on the current radio
 * @prev_tstamp: Timestamp of the previously received sample, which has to be
 * compared with the current tstamp to check descrepancy
 * @rparams: Parameters related to Spectral report structure
 * @param_min_max: Spectral parameter's minimum and maximum values
 * @finite_scan: Parameters for finite Spectral scan
 * @detector_list: Detector list for a given Spectral scan mode and channel
 * width, based on the target type.
 * @detector_list_lock: Lock to synchronize accesses to detector list
 * @det_map: Map of per-session detector information keyed by the Spectral HW
 * detector id.
 * @session_det_map_lock: Lock to synchronize accesses to session detector map
 * @report_info: Per session info to be filled at report level in SAMP message
 * @session_report_info_lock: Lock to synchronize access to session report info
 * @supported_bws: Supported sscan bandwidths for all sscan modes and
 * operating widths
 * @supported_sscan_bw_list: List of supported sscan widths for all sscan modes
 */
struct target_if_spectral {
	struct wlan_objmgr_pdev *pdev_obj;
	struct target_if_spectral_ops                 spectral_ops;
	struct spectral_caps                    capability;
	struct spectral_param_properties
			properties[SPECTRAL_SCAN_MODE_MAX][SPECTRAL_PARAM_MAX];
	qdf_spinlock_t                          spectral_lock;
	uint8_t                                 vdev_id[SPECTRAL_SCAN_MODE_MAX];
	int16_t                                 spectral_curchan_radindex;
	int16_t                                 spectral_extchan_radindex;
	uint32_t                               spectraldomain;
	uint32_t                               spectral_proc_phyerr;
	struct spectral_config                  spectral_defaultparams;
	struct target_if_spectral_stats         spectral_stats;
	struct target_if_spectral_event *events;
	unsigned int                            sc_spectral_ext_chan_ok:1,
						sc_spectral_combined_rssi_ok:1,
						sc_spectral_20_40_mode:1,
						sc_spectral_noise_pwr_cal:1,
						sc_spectral_non_edma:1;
	int                                     upper_is_control;
	int                                     upper_is_extension;
	int                                     lower_is_control;
	int                                     lower_is_extension;
	uint8_t                                sc_spectraltest_ieeechan;
	int                                     spectral_numbins;
	int                                     spectral_fft_len;
	int                                     spectral_data_len;

	/*
	 * For 11ac chipsets prior to AR900B version 2.0, a max of 512 bins are
	 * delivered.  However, there can be additional bins reported for
	 * AR900B version 2.0 and QCA9984 as described next:
	 *
	 * AR900B version 2.0: An additional tone is processed on the right
	 * hand side in order to facilitate detection of radar pulses out to
	 * the extreme band-edge of the channel frequency. Since the HW design
	 * processes four tones at a time, this requires one additional Dword
	 * to be added to the search FFT report.
	 *
	 * QCA9984: When spectral_scan_rpt_mode = 2, i.e 2-dword summary +
	 * 1x-oversampled bins (in-band) per FFT, then 8 more bins
	 * (4 more on left side and 4 more on right side)are added.
	 */

	int                                     lb_edge_extrabins;
	int                                     rb_edge_extrabins;
	int                                     spectral_max_index_offset;
	int                                     spectral_upper_max_index_offset;
	int                                     spectral_lower_max_index_offset;
	int                                     spectral_dc_index;
	int                                     send_single_packet;
	int                                     spectral_sent_msg;
	int                                     classify_scan;
	qdf_timer_t                             classify_timer;
	struct spectral_config params[SPECTRAL_SCAN_MODE_MAX];
	bool params_valid[SPECTRAL_SCAN_MODE_MAX];
	struct spectral_classifier_params       classifier_params;
	int                                     last_capture_time;
	int                                     num_spectral_data;
	int                                     total_spectral_data;
	int                                     max_rssi;
	int                                     detects_control_channel;
	int                                     detects_extension_channel;
	int                                     detects_below_dc;
	int                                     detects_above_dc;
	int                                     sc_scanning;
	int                                     sc_spectral_scan;
	int                                     sc_spectral_full_scan;
	uint64_t                               scan_start_tstamp;
	uint32_t                               last_tstamp;
	uint32_t                               first_tstamp;
	uint32_t                               spectral_samp_count;
	uint32_t                               sc_spectral_samp_count;
	int                                     noise_pwr_reports_reqd;
	int                                     noise_pwr_reports_recv;
	qdf_spinlock_t                          noise_pwr_reports_lock;
	struct target_if_chain_noise_pwr_info
		*noise_pwr_chain_ctl[HOST_MAX_ANTENNA];
	struct target_if_chain_noise_pwr_info
		*noise_pwr_chain_ext[HOST_MAX_ANTENNA];
	uint64_t                               tsf64;
#if ATH_PERF_PWR_OFFLOAD
	struct target_if_spectral_param_state_info
					param_info[SPECTRAL_SCAN_MODE_MAX];
#endif
	enum phy_ch_width ch_width[SPECTRAL_SCAN_MODE_MAX];
	bool sscan_width_configured[SPECTRAL_SCAN_MODE_MAX];
	struct spectral_diag_stats              diag_stats;
	bool                                    is_160_format;
	bool                                    is_lb_edge_extrabins_format;
	bool                                    is_rb_edge_extrabins_format;
	bool                                    is_sec80_rssi_war_required;
#ifdef QCA_SUPPORT_SPECTRAL_SIMULATION
	void                                    *simctx;
#endif
	enum spectral_gen                       spectral_gen;
	uint8_t                                hdr_sig_exp;
	uint8_t                                tag_sscan_summary_exp;
	uint8_t                                tag_sscan_fft_exp;
	uint8_t                                tlvhdr_size;
	struct spectral_nl_cb nl_cb;
	bool use_nl_bcast;
	int (*send_phy_data)(struct wlan_objmgr_pdev *pdev,
			     enum spectral_msg_type smsg_type);
	struct spectral_fft_bin_len_adj_swar len_adj_swar;
	struct spectral_timestamp_war timestamp_war;
	enum spectral_160mhz_report_delivery_state
			state_160mhz_delivery[SPECTRAL_SCAN_MODE_MAX];
	bool dbr_ring_debug;
	bool dbr_buff_debug;
	bool direct_dma_support;
#ifdef OPTIMIZED_SAMP_MESSAGE
	uint32_t prev_tstamp[MAX_DETECTORS_PER_PDEV];
#else
	uint32_t prev_tstamp;
#endif
	struct spectral_report_params rparams;
	struct spectral_param_min_max param_min_max;
	struct target_if_finite_spectral_scan_params
					finite_scan[SPECTRAL_SCAN_MODE_MAX];
	struct sscan_detector_list
			detector_list[SPECTRAL_SCAN_MODE_MAX][CH_WIDTH_MAX];
	qdf_spinlock_t detector_list_lock;
	struct per_session_det_map det_map[MAX_DETECTORS_PER_PDEV];
	qdf_spinlock_t session_det_map_lock;
	struct per_session_report_info report_info[SPECTRAL_SCAN_MODE_MAX];
	qdf_spinlock_t session_report_info_lock;
	struct spectral_supported_bws
		supported_bws[SPECTRAL_SCAN_MODE_MAX][CH_WIDTH_MAX];
	/* Whether a given sscan BW is supported on a given smode */
	bool supported_sscan_bw_list[SPECTRAL_SCAN_MODE_MAX][CH_WIDTH_MAX];
};

/**
 * struct target_if_psoc_spectral - Target if psoc Spectral object
 * @psoc_obj:  psoc object
 * @wmi_ops:  Spectral WMI operations
 */
struct target_if_psoc_spectral {
	struct wlan_objmgr_psoc *psoc_obj;
	struct spectral_wmi_ops wmi_ops;
};

#ifdef OPTIMIZED_SAMP_MESSAGE
/**
 * struct target_if_samp_msg_params - Spectral Analysis Messaging Protocol
 * data format
 * @hw_detector_id: Spectral HW detector ID
 * @rssi: Spectral RSSI
 * @lower_rssi: RSSI of lower band
 * @upper_rssi: RSSI of upper band
 * @chain_ctl_rssi: RSSI for control channel, for all antennas
 * @chain_ext_rssi: RSSI for extension channel, for all antennas
 * @last_raw_timestamp: Previous FFT report's raw timestamp.
 * @raw_timestamp: FFT timestamp reported by HW on primary segment.
 * @timestamp: timestamp
 * @reset_delay: Time gap between the last spectral report before reset and the
 *               end of reset.
 * @max_mag: maximum magnitude
 * @max_index: index of max magnitude
 * @noise_floor: current noise floor
 * @pri80ind: Indication from hardware that the sample was received on the
 *            primary 80 MHz segment. If this is set when smode =
 *            SPECTRAL_SCAN_MODE_AGILE, it indicates that Spectral was carried
 *            out on pri80 instead of the Agile frequency due to a channel
 *            switch - Software may choose to ignore the sample in this case.
 * @bin_pwr_data: Contains FFT magnitudes
 */
struct target_if_samp_msg_params {
	uint8_t hw_detector_id;
	int8_t rssi;
	int8_t lower_rssi;
	int8_t upper_rssi;
	int8_t chain_ctl_rssi[HOST_MAX_ANTENNA];
	int8_t chain_ext_rssi[HOST_MAX_ANTENNA];
	uint32_t last_raw_timestamp;
	uint32_t raw_timestamp;
	uint32_t timestamp;
	uint32_t reset_delay;
	uint16_t max_mag;
	uint16_t max_index;
	int16_t noise_floor;
	uint8_t agc_total_gain;
	uint8_t gainchange;
	uint8_t pri80ind;
	uint8_t *bin_pwr_data;
};

#else
/**
 * struct target_if_samp_msg_params - Spectral Analysis Messaging Protocol
 * data format
 * @rssi:  RSSI (except for secondary 80 segment)
 * @rssi_sec80:  RSSI for secondary 80 segment
 * @lower_rssi:  RSSI of lower band
 * @upper_rssi:  RSSI of upper band
 * @chain_ctl_rssi: RSSI for control channel, for all antennas
 * @chain_ext_rssi: RSSI for extension channel, for all antennas
 * @bwinfo:  bandwidth info
 * @data_len:  length of FFT data (except for secondary 80 segment)
 * @data_len_sec80:  length of FFT data for secondary 80 segment
 * @tstamp:  timestamp
 * @last_tstamp:  last time stamp
 * @max_mag:  maximum magnitude (except for secondary 80 segment)
 * @max_mag_sec80:  maximum magnitude for secondary 80 segment
 * @max_index:  index of max magnitude (except for secondary 80 segment)
 * @max_index_sec80:  index of max magnitude for secondary 80 segment
 * @max_exp:  max exp
 * @peak: peak frequency (obsolete)
 * @pwr_count:  number of FFT bins (except for secondary 80 segment)
 * @pwr_count_5mhz:  number of FFT bins in extra 5 MHz in
 *                   165 MHz/restricted 80p80 mode
 * @pwr_count_sec80:  number of FFT bins in secondary 80 segment
 * @nb_lower: This is deprecated
 * @nb_upper: This is deprecated
 * @max_upper_index:  index of max mag in upper band
 * @max_lower_index:  index of max mag in lower band
 * @bin_pwr_data: Contains FFT magnitudes (except for secondary 80 segment)
 * @bin_pwr_data_5mhz: Contains FFT magnitudes for the extra 5 MHz
 *                     in 165 MHz/restricted 80p80 mode
 * @bin_pwr_data_sec80: Contains FFT magnitudes for the secondary 80 segment
 * @freq: Center frequency of primary 20MHz channel in MHz
 * @vhtop_ch_freq_seg1: VHT operation first segment center frequency in MHz
 * @vhtop_ch_freq_seg2: VHT operation second segment center frequency in MHz
 * @agile_freq1:        Center frequency in MHz of the entire span(for 80+80 MHz
 *                      agile Scan it is primary 80 MHz span) across which
 *                      Agile Spectral is carried out. Applicable only for Agile
 *                      Spectral samples.
 * @agile_freq2:        Center frequency in MHz of the secondary 80 MHz span
 *                      across which Agile Spectral is carried out. Applicable
 *                      only for Agile Spectral samples in 80+80 MHz mode.
 * @freq_loading: spectral control duty cycles
 * @noise_floor:  current noise floor (except for secondary 80 segment)
 * @noise_floor_sec80:  current noise floor for secondary 80 segment
 * @interf_list: List of interfernce sources
 * @classifier_params:  classifier parameters
 * @sc:  classifier parameters
 * @pri80ind: Indication from hardware that the sample was received on the
 * primary 80 MHz segment. If this is set when smode =
 * SPECTRAL_SCAN_MODE_AGILE, it indicates that Spectral was carried out on
 * pri80 instead of the Agile frequency due to a channel switch - Software may
 * choose to ignore the sample in this case.
 * @pri80ind_sec80: Indication from hardware that the sample was received on the
 * primary 80 MHz segment instead of the secondary 80 MHz segment due to a
 * channel switch - Software may choose to ignore the sample if this is set.
 * Applicable only if smode = SPECTRAL_SCAN_MODE_NORMAL and for 160/80+80 MHz
 * Spectral operation and if the chipset supports fragmented 160/80+80 MHz
 * operation.
 * @last_raw_timestamp: Previous FFT report's raw timestamp. In case of 160MHz
 * it will be primary 80 segment's timestamp as both primary & secondary
 * segment's timestamps are expected to be almost equal
 * @timestamp_war_offset: Offset calculated based on reset_delay and
 * last_raw_stamp. It will be added to raw_timestamp to get tstamp.
 * @raw_timestamp: FFT timestamp reported by HW on primary segment.
 * @raw_timestamp_sec80: FFT timestamp reported by HW on secondary 80 segment.
 * @reset_delay: Time gap between the last spectral report before reset and the
 * end of reset.
 * @target_reset_count: Indicates the the number of times the target went
 * through reset routine after spectral was enabled.
 */
struct target_if_samp_msg_params {
	int8_t      rssi;
	int8_t      rssi_sec80;
	int8_t      lower_rssi;
	int8_t      upper_rssi;
	int8_t      chain_ctl_rssi[HOST_MAX_ANTENNA];
	int8_t      chain_ext_rssi[HOST_MAX_ANTENNA];
	uint16_t    bwinfo;
	uint16_t    datalen;
	uint16_t    datalen_sec80;
	uint32_t    tstamp;
	uint32_t    last_tstamp;
	uint16_t    max_mag;
	uint16_t    max_mag_sec80;
	uint16_t    max_index;
	uint16_t    max_index_sec80;
	uint8_t     max_exp;
	int         peak;
	int         pwr_count;
	int         pwr_count_5mhz;
	int         pwr_count_sec80;
	int8_t      nb_lower;
	int8_t      nb_upper;
	uint16_t    max_lower_index;
	uint16_t    max_upper_index;
	uint8_t    *bin_pwr_data;
	uint8_t    *bin_pwr_data_5mhz;
	uint8_t    *bin_pwr_data_sec80;
	uint16_t   freq;
	uint16_t   vhtop_ch_freq_seg1;
	uint16_t   vhtop_ch_freq_seg2;
	uint16_t   agile_freq1;
	uint16_t   agile_freq2;
	uint16_t   freq_loading;
	int16_t     noise_floor;
	int16_t     noise_floor_sec80;
	struct interf_src_rsp interf_list;
	struct spectral_classifier_params classifier_params;
	struct ath_softc *sc;
	uint8_t agc_total_gain;
	uint8_t agc_total_gain_sec80;
	uint8_t gainchange;
	uint8_t gainchange_sec80;
	enum spectral_scan_mode smode;
	uint8_t pri80ind;
	uint8_t pri80ind_sec80;
	uint32_t last_raw_timestamp;
	uint32_t timestamp_war_offset;
	uint32_t raw_timestamp;
	uint32_t raw_timestamp_sec80;
	uint32_t reset_delay;
	uint32_t target_reset_count;
};
#endif

/**
 * struct target_if_spectral_agile_mode_cap - Structure to hold agile
 * Spetcral scan capability
 * @agile_spectral_cap: agile Spectral scan capability for 20/40/80 MHz
 * @agile_spectral_cap_160: agile Spectral scan capability for 160 MHz
 * @agile_spectral_cap_80p80: agile Spectral scan capability for 80+80 MHz
 * @agile_spectral_cap_320: agile Spectral scan capability for 320 MHz
 */
struct target_if_spectral_agile_mode_cap {
	bool agile_spectral_cap;
	bool agile_spectral_cap_160;
	bool agile_spectral_cap_80p80;
	bool agile_spectral_cap_320;
};

#ifdef WLAN_CONV_SPECTRAL_ENABLE
/**
 * target_if_spectral_dump_fft() - Dump Spectral FFT
 * @pfft: Pointer to Spectral Phyerr FFT
 * @fftlen: FFT length
 *
 * Return: Success or failure
 */
int target_if_spectral_dump_fft(uint8_t *pfft, int fftlen);

/**
 * target_if_dbg_print_samp_param() - Print contents of SAMP struct
 * @p: Pointer to SAMP message
 *
 * Return: Void
 */
void target_if_dbg_print_samp_param(struct target_if_samp_msg_params *p);

/**
 * target_if_get_offset_swar_sec80() - Get offset for SWAR according to
 *                                     the channel width
 * @channel_width: Channel width
 *
 * Return: Offset for SWAR
 */
uint32_t target_if_get_offset_swar_sec80(uint32_t channel_width);

/**
 * target_if_sptrl_register_tx_ops() - Register Spectral target_if Tx Ops
 * @tx_ops: Tx Ops
 *
 * Return: void
 */
void target_if_sptrl_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);

#ifndef OPTIMIZED_SAMP_MESSAGE
/**
 * target_if_spectral_create_samp_msg() - Create the spectral samp message
 * @spectral : Pointer to spectral internal structure
 * @params : spectral samp message parameters
 *
 * API to create the spectral samp message
 *
 * Return: void
 */
void target_if_spectral_create_samp_msg(
	struct target_if_spectral *spectral,
	struct target_if_samp_msg_params *params);
#endif

#ifdef OPTIMIZED_SAMP_MESSAGE
/**
 * target_if_spectral_fill_samp_msg() - Fill the Spectral SAMP message
 * @spectral : Pointer to spectral internal structure
 * @params: Spectral SAMP message fields
 *
 * Fill the spectral SAMP message fields using params and detector map.
 *
 * Return: Success/Failure
 */
QDF_STATUS target_if_spectral_fill_samp_msg(
			struct target_if_spectral *spectral,
			struct target_if_samp_msg_params *params);
#endif

/**
 * target_if_spectral_process_phyerr_gen3() - Process phyerror event for gen3
 * @pdev:    Pointer to pdev object
 * @payload: Pointer to spectral report
 *
 * Process phyerror event for gen3
 *
 * Return: Success/Failure
 */
int target_if_spectral_process_report_gen3(
	struct wlan_objmgr_pdev *pdev,
	void *buf);

/**
 * target_if_process_phyerr_gen2() - Process PHY Error for gen2
 * @spectral: Pointer to Spectral object
 * @data: Pointer to phyerror event buffer
 * @datalen: Data length
 * @p_rfqual: RF quality info
 * @p_chaninfo: Channel info
 * @tsf64: 64 bit tsf timestamp
 * @acs_stats: ACS stats
 *
 * Process PHY Error for gen2
 *
 * Return: Success/Failure
 */
int target_if_process_phyerr_gen2(
	struct target_if_spectral *spectral,
	uint8_t *data,
	uint32_t datalen, struct target_if_spectral_rfqual_info *p_rfqual,
	struct target_if_spectral_chan_info *p_chaninfo,
	uint64_t tsf64,
	struct target_if_spectral_acs_stats *acs_stats);

/**
 * target_if_spectral_send_intf_found_msg() - Indicate to application layer that
 * interference has been found
 * @pdev: Pointer to pdev
 * @cw_int: 1 if CW interference is found, 0 if WLAN interference is found
 * @dcs_enabled: 1 if DCS is enabled, 0 if DCS is disabled
 *
 * Send message to application layer
 * indicating that interference has been found
 *
 * Return: None
 */
void target_if_spectral_send_intf_found_msg(
	struct wlan_objmgr_pdev *pdev,
	uint16_t cw_int, uint32_t dcs_enabled);

/**
 * target_if_stop_spectral_scan() - Stop spectral scan
 * @pdev: Pointer to pdev object
 * @smode: Spectral scan mode
 * @err: Pointer to error code
 *
 * API to stop the current on-going spectral scan
 *
 * Return: QDF_STATUS_SUCCESS in case of success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS target_if_stop_spectral_scan(struct wlan_objmgr_pdev *pdev,
					const enum spectral_scan_mode smode,
					enum spectral_cp_error_code *err);

/**
 * target_if_spectral_get_vdev() - Get pointer to vdev to be used for Spectral
 * operations
 * @spectral: Pointer to Spectral target_if internal private data
 * @smode: spectral scan mode
 *
 * Spectral operates on pdev. However, in order to retrieve some WLAN
 * properties, a vdev is required. To facilitate this, the function returns the
 * first vdev in our pdev. The caller should release the reference to the vdev
 * once it is done using it.
 * TODO: If the framework later provides an API to obtain the first active
 * vdev, then it would be preferable to use this API.
 *
 * Return: Pointer to vdev on success, NULL on failure
 */
struct wlan_objmgr_vdev *target_if_spectral_get_vdev(
	struct target_if_spectral *spectral,
	enum spectral_scan_mode smode);

/**
 * target_if_spectral_dump_hdr_gen2() - Dump Spectral header for gen2
 * @phdr: Pointer to Spectral Phyerr Header
 *
 * Dump Spectral header
 *
 * Return: Success/Failure
 */
int target_if_spectral_dump_hdr_gen2(struct spectral_phyerr_hdr_gen2 *phdr);

/**
 * target_if_get_combrssi_sec80_seg_gen2() - Get approximate combined RSSI
 *                                           for Secondary 80 segment
 * @spectral: Pointer to spectral object
 * @p_sfft_sec80: Pointer to search fft info of secondary 80 segment
 *
 * Get approximate combined RSSI for Secondary 80 segment
 *
 * Return: Combined RSSI for secondary 80Mhz segment
 */
int8_t target_if_get_combrssi_sec80_seg_gen2(
	struct target_if_spectral *spectral,
	struct spectral_search_fft_info_gen2 *p_sfft_sec80);

/**
 * target_if_spectral_dump_tlv_gen2() - Dump Spectral TLV for gen2
 * @ptlv: Pointer to Spectral Phyerr TLV
 * @is_160_format: Indicates 160 format
 *
 * Dump Spectral TLV for gen2
 *
 * Return: Success/Failure
 */
int target_if_spectral_dump_tlv_gen2(
	struct spectral_phyerr_tlv_gen2 *ptlv, bool is_160_format);

/**
 * target_if_spectral_dump_phyerr_data_gen2() - Dump Spectral
 * related PHY Error for gen2
 * @data: Pointer to phyerror buffer
 * @datalen: Data length
 * @is_160_format: Indicates 160 format
 *
 * Dump Spectral related PHY Error for gen2
 *
 * Return: Success/Failure
 */
int target_if_spectral_dump_phyerr_data_gen2(
	uint8_t *data,
	uint32_t datalen,
	bool is_160_format);

/**
 * target_if_dbg_print_samp_msg() - Print contents of SAMP Message
 * @p: Pointer to SAMP message
 *
 * Print contents of SAMP Message
 *
 * Return: Void
 */
void target_if_dbg_print_samp_msg(struct spectral_samp_msg *pmsg);

/**
 * get_target_if_spectral_handle_from_pdev() - Get handle to target_if internal
 * Spectral data
 * @pdev: Pointer to pdev
 *
 * Return: Handle to target_if internal Spectral data on success, NULL on
 * failure
 */
struct target_if_spectral *get_target_if_spectral_handle_from_pdev(
	struct wlan_objmgr_pdev *pdev);

/**
 * get_target_if_spectral_handle_from_psoc() - Get handle to psoc target_if
 * internal Spectral data
 * @psoc: Pointer to psoc
 *
 * Return: Handle to target_if psoc internal Spectral data on success, NULL on
 * failure
 */
static inline
struct target_if_psoc_spectral *get_target_if_spectral_handle_from_psoc(
	struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_rx_ops *rx_ops;
	struct target_if_psoc_spectral *psoc_spectral;

	if (!psoc) {
		spectral_err("psoc is null");
		return NULL;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		spectral_err("rx_ops is null");
		return NULL;
	}

	psoc_spectral = (struct target_if_psoc_spectral *)
		rx_ops->sptrl_rx_ops.sptrlro_get_psoc_target_handle(psoc);

	return psoc_spectral;
}

/**
 * target_if_vdev_get_chan_freq() - Get the operating channel frequency of a
 * given vdev
 * @pdev: Pointer to vdev
 *
 * Get the operating channel frequency of a given vdev
 *
 * Return: Operating channel frequency of a vdev
 */
static inline
int16_t target_if_vdev_get_chan_freq(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_rx_ops *rx_ops;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		spectral_err("psoc is NULL");
		return -EINVAL;
	}
	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		spectral_err("rx_ops is null");
		return -EINVAL;
	}

	return rx_ops->sptrl_rx_ops.sptrlro_vdev_get_chan_freq(
		vdev);
}

/**
 * target_if_vdev_get_chan_freq_seg2() - Get center frequency of secondary 80 of
 * given vdev
 * @vdev: Pointer to vdev
 *
 * Get the center frequency of secondary 80 of given vdev
 *
 * Return: center frequency of secondary 80
 */
static inline
int16_t target_if_vdev_get_chan_freq_seg2(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_rx_ops *rx_ops;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		spectral_err("psoc is NULL");
		return -EINVAL;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		spectral_err("rx_ops is null");
		return -EINVAL;
	}

	return rx_ops->sptrl_rx_ops.sptrlro_vdev_get_chan_freq_seg2(vdev);
}

/**
 * target_if_vdev_get_ch_width() - Get the operating channel bandwidth of a
 * given vdev
 * @pdev: Pointer to vdev
 *
 * Get the operating channel bandwidth of a given vdev
 *
 * Return: channel bandwidth enumeration corresponding to the vdev
 */
static inline
enum phy_ch_width target_if_vdev_get_ch_width(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	enum phy_ch_width ch_width;
	struct wlan_lmac_if_rx_ops *rx_ops;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		spectral_err("psoc is NULL");
		return CH_WIDTH_INVALID;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		spectral_err("rx_ops is null");
		return CH_WIDTH_INVALID;
	}

	ch_width = rx_ops->sptrl_rx_ops.sptrlro_vdev_get_ch_width(vdev);

	if (ch_width == CH_WIDTH_160MHZ) {
		int16_t cfreq2;

		cfreq2 = target_if_vdev_get_chan_freq_seg2(vdev);
		if (cfreq2 < 0) {
			spectral_err("Invalid value for cfreq2 %d", cfreq2);
			return CH_WIDTH_INVALID;
		}

		/* Use non zero cfreq2 to identify 80p80 */
		if (cfreq2)
			ch_width = CH_WIDTH_80P80MHZ;
	}

	return ch_width;
}

/**
 * target_if_vdev_get_sec20chan_freq_mhz() - Get the frequency of secondary
 * 20 MHz channel for a given vdev
 * @pdev: Pointer to vdev
 *
 * Get the frequency of secondary 20Mhz channel for a given vdev
 *
 * Return: Frequency of secondary 20Mhz channel for a given vdev
 */
static inline
int target_if_vdev_get_sec20chan_freq_mhz(
	struct wlan_objmgr_vdev *vdev,
	uint16_t *sec20chan_freq)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct wlan_lmac_if_rx_ops *rx_ops;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		spectral_err("psoc is NULL");
		return -EINVAL;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		spectral_err("rx_ops is null");
		return -EINVAL;
	}

	return rx_ops->sptrl_rx_ops.
		sptrlro_vdev_get_sec20chan_freq_mhz(vdev, sec20chan_freq);
}

/**
 * target_if_spectral_is_feature_disabled_psoc() - Check if Spectral feature is
 * disabled for a given psoc
 * @psoc: Pointer to psoc
 *
 * Return: true or false
 */
static inline
bool target_if_spectral_is_feature_disabled_psoc(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_lmac_if_rx_ops *rx_ops;

	if (!psoc) {
		spectral_err("psoc is NULL");
		return true;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		spectral_err("rx_ops is null");
		return true;
	}

	if (rx_ops->sptrl_rx_ops.
	    sptrlro_spectral_is_feature_disabled_psoc)
		return rx_ops->sptrl_rx_ops.
		       sptrlro_spectral_is_feature_disabled_psoc(psoc);

	return true;
}

/**
 * target_if_spectral_is_feature_disabled_pdev() - Check if Spectral feature is
 * disabled for a given pdev
 * @pdev: Pointer to pdev
 *
 * Return: true or false
 */
static inline
bool target_if_spectral_is_feature_disabled_pdev(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_lmac_if_rx_ops *rx_ops;
	struct wlan_objmgr_psoc *psoc;

	if (!pdev) {
		spectral_err("pdev is NULL");
		return true;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is NULL");
		return true;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		spectral_err("rx_ops is null");
		return true;
	}

	if (rx_ops->sptrl_rx_ops.
	    sptrlro_spectral_is_feature_disabled_pdev)
		return rx_ops->sptrl_rx_ops.
		       sptrlro_spectral_is_feature_disabled_pdev(pdev);

	return true;
}

/**
 * target_if_spectral_set_rxchainmask() - Set Spectral Rx chainmask
 * @pdev: Pointer to pdev
 * @spectral_rx_chainmask: Spectral Rx chainmask
 *
 * Return: None
 */
static inline
void target_if_spectral_set_rxchainmask(struct wlan_objmgr_pdev *pdev,
					uint8_t spectral_rx_chainmask)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct target_if_spectral *spectral = NULL;
	enum spectral_scan_mode smode = SPECTRAL_SCAN_MODE_NORMAL;
	struct wlan_lmac_if_rx_ops *rx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is NULL");
		return;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		spectral_err("rx_ops is null");
		return;
	}

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err("Invalid Spectral mode %u", smode);
		return;
	}

	if (rx_ops->sptrl_rx_ops.
	    sptrlro_spectral_is_feature_disabled_pdev(pdev)) {
		spectral_info("Spectral feature is disabled");
		return;
	}

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("Spectral target if object is null");
		return;
	}

	/* set chainmask for all the modes */
	for (; smode < SPECTRAL_SCAN_MODE_MAX; smode++)
		spectral->params[smode].ss_chn_mask = spectral_rx_chainmask;
}

/**
 * target_if_spectral_process_phyerr() - Process Spectral PHY error
 * @pdev: Pointer to pdev
 * @data: PHY error data received from FW
 * @datalen: Length of data
 * @p_rfqual: Pointer to RF Quality information
 * @p_chaninfo: Pointer to channel information
 * @tsf: TSF time instance at which the Spectral sample was received
 * @acs_stats: ACS stats
 *
 * Process Spectral PHY error by extracting necessary information from the data
 * sent by FW, and send the extracted information to application layer.
 *
 * Return: None
 */
static inline
void target_if_spectral_process_phyerr(
	struct wlan_objmgr_pdev *pdev,
	uint8_t *data, uint32_t datalen,
	struct target_if_spectral_rfqual_info *p_rfqual,
	struct target_if_spectral_chan_info *p_chaninfo,
	uint64_t tsf64,
	struct target_if_spectral_acs_stats *acs_stats)
{
	struct target_if_spectral *spectral = NULL;
	struct target_if_spectral_ops *p_sops = NULL;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("Spectral target if object is null");
		return;
	}

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	if (!p_sops->spectral_process_phyerr) {
		spectral_err("null spectral_process_phyerr");
		return;
	}
	p_sops->spectral_process_phyerr(spectral, data, datalen,
					p_rfqual, p_chaninfo,
					tsf64, acs_stats);
}

static QDF_STATUS
target_if_get_spectral_msg_type(enum spectral_scan_mode smode,
				enum spectral_msg_type *msg_type) {

	switch (smode) {
	case SPECTRAL_SCAN_MODE_NORMAL:
		*msg_type = SPECTRAL_MSG_NORMAL_MODE;
		break;

	case SPECTRAL_SCAN_MODE_AGILE:
		*msg_type = SPECTRAL_MSG_AGILE_MODE;
		break;

	default:
		spectral_err("Invalid spectral mode");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static inline bool
is_ch_width_160_or_80p80(enum phy_ch_width ch_width)
{
	return (ch_width == CH_WIDTH_160MHZ || ch_width == CH_WIDTH_80P80MHZ);
}

/**
 * free_samp_msg_skb() - Free SAMP message skb
 * @spectral: Pointer to Spectral
 * @smode: Spectral Scan mode
 *
 * Free SAMP message skb, if error in report processing
 *
 * Return: void
 */
static inline void
free_samp_msg_skb(struct target_if_spectral *spectral,
		  enum spectral_scan_mode smode)
{
	enum spectral_msg_type smsg_type;
	QDF_STATUS ret;

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("Invalid Spectral mode %d", smode);
		return;
	}

	if (is_ch_width_160_or_80p80(spectral->ch_width[smode])) {
		ret = target_if_get_spectral_msg_type(smode, &smsg_type);
		if (QDF_IS_STATUS_ERROR(ret)) {
			spectral_err("Failed to get spectral message type");
			return;
		}
		spectral->nl_cb.free_sbuff(spectral->pdev_obj,
					   smsg_type);
	}
}

/**
 * init_160mhz_delivery_state_machine() - Initialize 160MHz Spectral
 *                                        state machine
 * @spectral: Pointer to Spectral
 *
 * Initialize 160MHz Spectral state machine
 *
 * Return: void
 */
static inline void
init_160mhz_delivery_state_machine(struct target_if_spectral *spectral)
{
	uint8_t smode;

	smode = 0;
	for (; smode < SPECTRAL_SCAN_MODE_MAX; smode++)
		spectral->state_160mhz_delivery[smode] =
				SPECTRAL_REPORT_WAIT_PRIMARY80;
}

/**
 * reset_160mhz_delivery_state_machine() - Reset 160MHz Spectral state machine
 * @spectral: Pointer to Spectral
 *
 * Reset 160MHz Spectral state machine
 *
 * Return: void
 */
static inline void
reset_160mhz_delivery_state_machine(struct target_if_spectral *spectral,
				    enum spectral_scan_mode smode)
{
	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("Invalid Spectral mode %d", smode);
		return;
	}

	free_samp_msg_skb(spectral, smode);

	if (is_ch_width_160_or_80p80(spectral->ch_width[smode])) {
		spectral->state_160mhz_delivery[smode] =
			SPECTRAL_REPORT_WAIT_PRIMARY80;
	}
}

/**
 * is_secondaryseg_expected() - Is waiting for secondary 80 report
 * @spectral: Pointer to Spectral
 * @smode: Spectral scan mode
 *
 * Return true if secondary 80 report expected and mode is 160 MHz
 *
 * Return: true or false
 */
static inline
bool is_secondaryseg_expected(struct target_if_spectral *spectral,
			      enum spectral_scan_mode smode)
{
	return
	(is_ch_width_160_or_80p80(spectral->ch_width[smode]) &&
	 spectral->rparams.fragmentation_160[smode] &&
	 (spectral->state_160mhz_delivery[smode] ==
	  SPECTRAL_REPORT_WAIT_SECONDARY80));
}

/**
 * is_primaryseg_expected() - Is waiting for primary 80 report
 * @spectral: Pointer to Spectral
 * @smode: Spectral scan mode
 *
 * Return true if mode is 160 Mhz and primary 80 report expected or
 * mode is not 160 Mhz
 *
 * Return: true or false
 */
static inline
bool is_primaryseg_expected(struct target_if_spectral *spectral,
			    enum spectral_scan_mode smode)
{
	return
	(!is_ch_width_160_or_80p80(spectral->ch_width[smode]) ||
	 !spectral->rparams.fragmentation_160[smode] ||
	 (spectral->state_160mhz_delivery[smode] ==
	  SPECTRAL_REPORT_WAIT_PRIMARY80));
}

#ifndef OPTIMIZED_SAMP_MESSAGE
/**
 * is_primaryseg_rx_inprog() - Is primary 80 report processing is in progress
 * @spectral: Pointer to Spectral
 * @smode: Spectral scan mode
 *
 * Is primary 80 report processing is in progress
 *
 * Return: true or false
 */
static inline
bool is_primaryseg_rx_inprog(struct target_if_spectral *spectral,
			     enum spectral_scan_mode smode)
{
	return
	(!is_ch_width_160_or_80p80(spectral->ch_width[smode]) ||
	 spectral->spectral_gen == SPECTRAL_GEN2 ||
	 (spectral->spectral_gen == SPECTRAL_GEN3 &&
	  (!spectral->rparams.fragmentation_160[smode] ||
	   spectral->state_160mhz_delivery[smode] ==
	   SPECTRAL_REPORT_RX_PRIMARY80)));
}

/**
 * is_secondaryseg_rx_inprog() - Is secondary80 report processing is in progress
 * @spectral: Pointer to Spectral
 * @smode: Spectral scan mode
 *
 * Is secondary 80 report processing is in progress
 *
 * Return: true or false
 */
static inline
bool is_secondaryseg_rx_inprog(struct target_if_spectral *spectral,
			       enum spectral_scan_mode smode)
{
	return
	(is_ch_width_160_or_80p80(spectral->ch_width[smode]) &&
	 (spectral->spectral_gen == SPECTRAL_GEN2 ||
	  ((spectral->spectral_gen == SPECTRAL_GEN3) &&
	   (!spectral->rparams.fragmentation_160[smode] ||
	    spectral->state_160mhz_delivery[smode] ==
	    SPECTRAL_REPORT_RX_SECONDARY80))));
}
#endif

/**
 * target_if_160mhz_delivery_state_change() - State transition for 160Mhz
 *                                            Spectral
 * @spectral: Pointer to spectral object
 * @smode: Spectral scan mode
 * @detector_id: Detector id
 *
 * Move the states of state machine for 160MHz spectral scan report receive
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_160mhz_delivery_state_change(struct target_if_spectral *spectral,
				       enum spectral_scan_mode smode,
				       uint8_t detector_id);

/**
 * target_if_sops_is_spectral_enabled() - Get whether Spectral is enabled
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * Function to check whether Spectral is enabled
 *
 * Return: True if Spectral is enabled, false if Spectral is not enabled
 */
uint32_t target_if_sops_is_spectral_enabled(void *arg,
					    enum spectral_scan_mode smode);

/**
 * target_if_sops_is_spectral_active() - Get whether Spectral is active
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * Function to check whether Spectral is active
 *
 * Return: True if Spectral is active, false if Spectral is not active
 */
uint32_t target_if_sops_is_spectral_active(void *arg,
					   enum spectral_scan_mode smode);

/**
 * target_if_sops_start_spectral_scan() - Start Spectral scan
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @smode: Spectral scan mode
 * @err: Pointer to error code
 *
 * Function to start spectral scan
 *
 * Return: 0 on success else failure
 */
uint32_t target_if_sops_start_spectral_scan(void *arg,
					    enum spectral_scan_mode smode,
					    enum spectral_cp_error_code *err);

/**
 * target_if_sops_stop_spectral_scan() - Stop Spectral scan
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * Function to stop spectral scan
 *
 * Return: 0 in case of success, -1 on failure
 */
uint32_t target_if_sops_stop_spectral_scan(void *arg,
					   enum spectral_scan_mode smode);

/**
 * target_if_spectral_get_extension_channel() - Get the current Extension
 *                                              channel (in MHz)
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * Return: Current Extension channel (in MHz) on success, 0 on failure or if
 * extension channel is not present.
 */
uint32_t
target_if_spectral_get_extension_channel(void *arg,
					 enum spectral_scan_mode smode);

/**
 * target_if_spectral_get_current_channel() - Get the current channel (in MHz)
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * Return: Current channel (in MHz) on success, 0 on failure
 */
uint32_t
target_if_spectral_get_current_channel(void *arg,
				       enum spectral_scan_mode smode);


/**
 * target_if_spectral_reset_hw() - Reset the hardware
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * This is only a placeholder since it is not currently required in the offload
 * case.
 *
 * Return: 0
 */
uint32_t target_if_spectral_reset_hw(void *arg);

/**
 * target_if_spectral_get_chain_noise_floor() - Get the Chain noise floor from
 * Noisefloor history buffer
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @nf_buf: Pointer to buffer into which chain Noise Floor data should be copied
 *
 * This is only a placeholder since it is not currently required in the offload
 * case.
 *
 * Return: 0
 */
uint32_t target_if_spectral_get_chain_noise_floor(void *arg, int16_t *nf_buf);

/**
 * target_if_spectral_get_ext_noisefloor() - Get the extension channel
 * noisefloor
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * This is only a placeholder since it is not currently required in the offload
 * case.
 *
 * Return: 0
 */
int8_t target_if_spectral_get_ext_noisefloor(void *arg);

/**
 * target_if_spectral_get_ctl_noisefloor() - Get the control channel noisefloor
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * This is only a placeholder since it is not currently required in the offload
 * case.
 *
 * Return: 0
 */
int8_t target_if_spectral_get_ctl_noisefloor(void *arg);

/**
 * target_if_spectral_get_capability() - Get whether a given Spectral hardware
 * capability is available
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @type: Spectral hardware capability type
 *
 * Return: True if the capability is available, false if the capability is not
 * available
 */
uint32_t target_if_spectral_get_capability(
	void *arg, enum spectral_capability_type type);

/**
 * target_if_spectral_set_rxfilter() - Set the RX Filter before Spectral start
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @rxfilter: Rx filter to be used
 *
 * Note: This is only a placeholder function. It is not currently required since
 * FW should be taking care of setting the required filters.
 *
 * Return: 0
 */
uint32_t target_if_spectral_set_rxfilter(void *arg, int rxfilter);

/**
 * target_if_spectral_sops_configure_params() - Configure user supplied Spectral
 * parameters
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @params: Spectral parameters
 * @smode: Spectral scan mode
 *
 * Return: 0 in case of success, -1 on failure
 */
uint32_t target_if_spectral_sops_configure_params(
				void *arg, struct spectral_config *params,
				enum spectral_scan_mode smode);

/**
 * target_if_spectral_get_rxfilter() - Get the current RX Filter settings
 * @arg: Pointer to handle for Spectral target_if internal private data
 *
 * Note: This is only a placeholder function. It is not currently required since
 * FW should be taking care of setting the required filters.
 *
 * Return: 0
 */
uint32_t target_if_spectral_get_rxfilter(void *arg);

/**
 * target_if_pdev_spectral_deinit() - De-initialize target_if Spectral
 * functionality for the given pdev
 * @pdev: Pointer to pdev object
 *
 * Return: None
 */
void target_if_pdev_spectral_deinit(struct wlan_objmgr_pdev *pdev);

/**
 * target_if_set_spectral_config() - Set spectral config
 * @pdev:       Pointer to pdev object
 * @param: Spectral parameter id and value
 * @smode: Spectral scan mode
 * @err: Pointer to Spectral error code
 *
 * API to set spectral configurations
 *
 * Return: QDF_STATUS_SUCCESS in case of success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS target_if_set_spectral_config(struct wlan_objmgr_pdev *pdev,
					 const struct spectral_cp_param *param,
					 const enum spectral_scan_mode smode,
					 enum spectral_cp_error_code *err);

/**
 * target_if_pdev_spectral_init() - Initialize target_if Spectral
 * functionality for the given pdev
 * @pdev: Pointer to pdev object
 *
 * Return: On success, pointer to Spectral target_if internal private data, on
 * failure, NULL
 */
void *target_if_pdev_spectral_init(struct wlan_objmgr_pdev *pdev);

/**
 * target_if_spectral_sops_get_params() - Get user configured Spectral
 * parameters
 * @arg: Pointer to handle for Spectral target_if internal private data
 * @params: Pointer to buffer into which Spectral parameters should be copied
 * @smode: Spectral scan mode
 *
 * Return: 0 in case of success, -1 on failure
 */
uint32_t target_if_spectral_sops_get_params(
			void *arg, struct spectral_config *params,
			enum spectral_scan_mode smode);

/**
 * target_if_init_spectral_capability() - Initialize Spectral capability
 *
 * @spectral: Pointer to Spectral target_if internal private data
 * @target_type: target type
 *
 * This is a workaround.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
target_if_init_spectral_capability(struct target_if_spectral *spectral,
				   uint32_t target_type);

/**
 * target_if_start_spectral_scan() - Start spectral scan
 * @pdev: Pointer to pdev object
 * @vdev_id: VDEV id
 * @smode: Spectral scan mode
 * @err: Spectral error code
 *
 * API to start spectral scan
 *
 * Return: QDF_STATUS_SUCCESS in case of success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS target_if_start_spectral_scan(struct wlan_objmgr_pdev *pdev,
					 uint8_t vdev_id,
					 enum spectral_scan_mode smode,
					 enum spectral_cp_error_code *err);

/**
 * target_if_get_spectral_config() - Get spectral configuration
 * @pdev: Pointer to pdev object
 * @param: Pointer to spectral_config structure in which the configuration
 * should be returned
 * @smode: Spectral scan mode
 *
 * API to get the current spectral configuration
 *
 * Return: QDF_STATUS_SUCCESS in case of success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS target_if_get_spectral_config(struct wlan_objmgr_pdev *pdev,
					 struct spectral_config *param,
					 enum spectral_scan_mode smode);

/**
 * target_if_spectral_scan_enable_params() - Enable use of desired Spectral
 *                                           parameters
 * @spectral: Pointer to Spectral target_if internal private data
 * @spectral_params: Pointer to Spectral parameters
 * @smode: Spectral scan mode
 * @err: Spectral error code
 *
 * Enable use of desired Spectral parameters by configuring them into HW, and
 * starting Spectral scan
 *
 * Return: 0 on success, 1 on failure
 */
int target_if_spectral_scan_enable_params(
		struct target_if_spectral *spectral,
		struct spectral_config *spectral_params,
		enum spectral_scan_mode smode,
		enum spectral_cp_error_code *err);

/**
 * target_if_is_spectral_active() - Get whether Spectral is active
 * @pdev: Pointer to pdev object
 * @smode: Spectral scan mode
 *
 * Return: True if Spectral is active, false if Spectral is not active
 */
bool target_if_is_spectral_active(struct wlan_objmgr_pdev *pdev,
				  enum spectral_scan_mode smode);

/**
 * target_if_is_spectral_enabled() - Get whether Spectral is enabled
 * @pdev: Pointer to pdev object
 * @smode: Spectral scan mode
 *
 * Return: True if Spectral is enabled, false if Spectral is not enabled
 */
bool target_if_is_spectral_enabled(struct wlan_objmgr_pdev *pdev,
				   enum spectral_scan_mode smode);

/**
 * target_if_set_debug_level() - Set debug level for Spectral
 * @pdev: Pointer to pdev object
 * @debug_level: Debug level
 *
 * Return: QDF_STATUS_SUCCESS in case of success, else QDF_STATUS_E_FAILURE
 *
 */
QDF_STATUS target_if_set_debug_level(struct wlan_objmgr_pdev *pdev,
				     uint32_t debug_level);

/**
 * target_if_get_debug_level() - Get debug level for Spectral
 * @pdev: Pointer to pdev object
 *
 * Return: Current debug level
 */
uint32_t target_if_get_debug_level(struct wlan_objmgr_pdev *pdev);


/**
 * target_if_get_spectral_capinfo() - Get Spectral capability information
 * @pdev: Pointer to pdev object
 * @scaps: Buffer into which data should be copied
 *
 * Return: QDF_STATUS_SUCCESS in case of success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS target_if_get_spectral_capinfo(struct wlan_objmgr_pdev *pdev,
					  struct spectral_caps *scaps);


/**
 * target_if_get_spectral_diagstats() - Get Spectral diagnostic statistics
 * @pdev:  Pointer to pdev object
 * @stats: Buffer into which data should be copied
 *
 * Return: QDF_STATUS_SUCCESS in case of success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS target_if_get_spectral_diagstats(struct wlan_objmgr_pdev *pdev,
					    struct spectral_diag_stats *stats);

QDF_STATUS
target_if_160mhz_delivery_state_change(struct target_if_spectral *spectral,
				       enum spectral_scan_mode smode,
				       uint8_t detector_id);

#ifdef OPTIMIZED_SAMP_MESSAGE
/**
 * target_if_spectral_get_num_fft_bins() - Get number of FFT bins from FFT size
 * according to the Spectral report mode.
 * @fft_size: FFT length
 * @report_mode: Spectral report mode
 *
 * Get number of FFT bins from FFT size according to the Spectral
 * report mode.
 *
 * Return: Number of FFT bins
 */
static inline uint32_t
target_if_spectral_get_num_fft_bins(uint32_t fft_size,
				    enum spectral_report_mode report_mode)
{
	switch (report_mode) {
	case SPECTRAL_REPORT_MODE_0:
	case SPECTRAL_REPORT_MODE_1:
		return 0;
	case SPECTRAL_REPORT_MODE_2:
		return (1 << (fft_size - 1));
	case SPECTRAL_REPORT_MODE_3:
		return (1 << fft_size);
	default:
		return -EINVAL;
	}
}
#endif /* OPTIMIZED_SAMP_MESSAGE */

#ifdef OPTIMIZED_SAMP_MESSAGE
/**
 * target_if_get_detector_chwidth() - Get per-detector bandwidth
 * based on channel width and fragmentation.
 * @ch_width: Spectral scan channel width
 * @fragmentation_160: Target type has fragmentation or not
 *
 * Get per-detector BW.
 *
 * Return: detector BW
 */
static inline
enum phy_ch_width target_if_get_detector_chwidth(enum phy_ch_width ch_width,
						 bool fragmentation_160)
{
	return ((ch_width == CH_WIDTH_160MHZ && fragmentation_160) ?
		CH_WIDTH_80MHZ : ((ch_width == CH_WIDTH_80P80MHZ) ?
		CH_WIDTH_80MHZ : ch_width));
}

/**
 * target_if_spectral_set_start_end_freq() - Set start and end frequencies for
 * a given center frequency
 * @cfreq: Center frequency for which start and end freq need to be set
 * @ch_width: Spectral scan Channel width
 * @fragmentation_160: Target type has fragmentation or not
 * @start_end_freq: Array containing start and end frequency of detector
 *
 * Set the start and end frequencies for given center frequency in destination
 * detector info struct
 *
 * Return: void
 */
static inline
void target_if_spectral_set_start_end_freq(uint32_t cfreq,
					   enum phy_ch_width ch_width,
					   bool fragmentation_160,
					   uint32_t *start_end_freq)
{
	enum phy_ch_width det_ch_width;

	det_ch_width = target_if_get_detector_chwidth(ch_width,
						      fragmentation_160);

	start_end_freq[0] = cfreq - (wlan_reg_get_bw_value(det_ch_width) >> 1);
	start_end_freq[1] = cfreq + (wlan_reg_get_bw_value(det_ch_width) >> 1);
}
#endif /* OPTIMIZED_SAMP_MESSAGE */

#ifdef DIRECT_BUF_RX_ENABLE
/**
 * target_if_consume_sfft_report_gen3() -  Process fft report for gen3
 * @spectral: Pointer to spectral object
 * @report: Pointer to spectral report
 *
 * Process fft report for gen3
 *
 * Return: Success/Failure
 */
int
target_if_consume_spectral_report_gen3(
	 struct target_if_spectral *spectral,
	 struct spectral_report *report);
#endif

/**
 * target_if_spectral_fw_hang() - Crash the FW from Spectral module
 * @spectral: Pointer to Spectral LMAC object
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS target_if_spectral_fw_hang(struct target_if_spectral *spectral);

/**
 * target_if_spectral_finite_scan_update() - Update scan count for finite scan
 * and stop Spectral scan when required
 * @spectral: Pointer to Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * This API decrements the number of Spectral reports expected from target for
 * a finite Spectral scan. When expected number of reports are received from
 * target Spectral scan is stopped.
 *
 * Return: QDF_STATUS on success
 */
QDF_STATUS
target_if_spectral_finite_scan_update(struct target_if_spectral *spectral,
				      enum spectral_scan_mode smode);

/**
 * target_if_spectral_is_finite_scan() - Check Spectral scan is finite/infinite
 * @spectral: Pointer to Spectral target_if internal private data
 * @smode: Spectral scan mode
 *
 * API to check whether Spectral scan is finite/infinite for the give mode.
 * A non zero scan count indicates that scan is finite. Scan count of 0
 * indicates an infinite Spectral scan.
 *
 * Return: QDF_STATUS on success
 */
QDF_STATUS
target_if_spectral_is_finite_scan(struct target_if_spectral *spectral,
				  enum spectral_scan_mode smode,
				  bool *finite_spectral_scan);

#ifdef BIG_ENDIAN_HOST
/**
 * target_if_byte_swap_spectral_headers_gen3() - Apply byte-swap on headers
 * @spectral: Pointer to Spectral target_if internal private data
 * @data: Pointer to the start of Spectral Scan Summary report
 *
 * This API is only required for Big-endian Host platforms.
 * It applies 32-bit byte-swap on Spectral Scan Summary and Search FFT reports
 * and copies them back to the source location.
 * Padding bytes that lie between the reports won't be touched.
 *
 * Return: QDF_STATUS_SUCCESS in case of success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS target_if_byte_swap_spectral_headers_gen3(
	 struct target_if_spectral *spectral,
	 void *data);

/**
 * target_if_byte_swap_spectral_fft_bins_gen3() - Apply byte-swap on FFT bins
 * @rparams: Pointer to Spectral report parameters
 * @bin_pwr_data: Pointer to the start of FFT bins
 * @num_fftbins: Number of FFT bins
 *
 * This API is only required for Big-endian Host platforms.
 * It applies pack-mode-aware byte-swap on the FFT bins as below:
 *   1. pack-mode 0 (i.e., 1 FFT bin per DWORD):
 *        Reads the least significant 2 bytes of each DWORD, applies 16-bit
 *        byte-swap on that value, and copies it back to the source location.
 *   2. pack-mode 1 (i.e., 2 FFT bins per DWORD):
 *        Reads each FFT bin, applies 16-bit byte-swap on that value,
 *        and copies it back to the source location.
 *   3. pack-mode 2 (4 FFT bins per DWORD):
 *        Nothing
 *
 * Return: QDF_STATUS_SUCCESS in case of success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS target_if_byte_swap_spectral_fft_bins_gen3(
	const struct spectral_report_params *rparams,
	void *bin_pwr_data, size_t num_fftbins);
#endif /* BIG_ENDIAN_HOST */

#ifdef OPTIMIZED_SAMP_MESSAGE
/**
 * target_if_populate_fft_bins_info() - Populate the start and end bin
 * indices, on per-detector level.
 * @spectral: Pointer to target_if spectral internal structure
 * @smode: Spectral scan mode
 *
 * Populate the start and end bin indices, on per-detector level.
 *
 * Return: Success/Failure
 */
QDF_STATUS
target_if_populate_fft_bins_info(struct target_if_spectral *spectral,
				 enum spectral_scan_mode smode);
#else
static inline QDF_STATUS
target_if_populate_fft_bins_info(struct target_if_spectral *spectral,
				 enum spectral_scan_mode smode)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * spectral_is_session_info_expected_from_target() - Check if spectral scan
 * session is expected from target
 * @pdev: pdev pointer
 * @is_session_info_expected: Pointer to caller variable
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
spectral_is_session_info_expected_from_target(struct wlan_objmgr_pdev *pdev,
					      bool *is_session_info_expected);

#ifdef WIN32
#pragma pack(pop, target_if_spectral)
#endif
#ifdef __ATTRIB_PACK
#undef __ATTRIB_PACK
#endif

/**
 * target_if_spectral_copy_fft_bins() - Copy FFT bins from source buffer to
 * destination buffer
 * @spectral: Pointer to Spectral LMAC object
 * @src_fft_buf: Pointer to source FFT buffer
 * @dest_fft_buf: Pointer to destination FFT buffer
 * @fft_bin_count: Number of FFT bins to copy
 * @bytes_copied: Number of bytes copied by this API
 *
 * Different targets supports different FFT bin widths. This API encapsulates
 * all those details and copies 8-bit FFT value into the destination buffer.
 * Also, this API takes care of handling big-endian mode.
 * In essence, it does the following.
 *   - Read DWORDs one by one
 *   - Extract individual FFT bins out of it
 *   - Copy the FFT bin to destination buffer
 *
 * Return: QDF_STATUS_SUCCESS in case of success, else QDF_STATUS_E_FAILURE
 */
QDF_STATUS
target_if_spectral_copy_fft_bins(struct target_if_spectral *spectral,
				 const void *src_fft_buf,
				 void *dest_fft_buf,
				 uint32_t fft_bin_count,
				 uint32_t *bytes_copied);
#endif /* WLAN_CONV_SPECTRAL_ENABLE */
#endif /* _TARGET_IF_SPECTRAL_H_ */
