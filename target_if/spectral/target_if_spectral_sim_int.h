/*
 * Copyright (c) 2015,2017 The Linux Foundation. All rights reserved.
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

#ifndef _SPECTRAL_SIM_INTERNAL_H_
#define _SPECTRAL_SIM_INTERNAL_H_

#ifdef QCA_SUPPORT_SPECTRAL_SIMULATION
#include "target_if_spectral.h"

/* #define SPECTRAL_SIM_DUMP_PARAM_DATA 1 */
/* Spectral report data instance. Usable in a linked list.
 * - In the case of Direct Attach chipsets, one instance should correspond to
 *   one PHY Data Error frame received from the HW.
 *   XXX Direct Attach support to be implemented if needed. Any modifications
 *   required here can be made at the time of implementation.
 * - In the case of 802.11ac offload chipsets, one instance should correspond to
 *   one report received from HW, inclusive of all TLVs.
 */
typedef struct _ath_spectralsim_report {
	/* 11ac onwards only */
	struct target_if_spectral_rfqual_info rfqual_info;
	/* 11ac onwards only */
	struct target_if_spectral_chan_info chan_info;
	u_int32_t datasize;
	u_int8_t *data;
	struct _ath_spectralsim_report *next;
} ath_spectralsim_report;

/* Set of Spectral report data instances corresponding to one particular
 * configuration. Usable in a linked list.
 */
typedef struct _ath_spectralsim_reportset {
	struct spectral_config config;
	ath_spectralsim_report *headreport;
	ath_spectralsim_report *curr_report;
	struct _ath_spectralsim_reportset *next;
} ath_spectralsim_reportset;

/* Main structure for Spectral simulation. All data and controls get linked
 * here.
 *
 * For each width (20/40/80/160/80+80), we will have a linked list of
 * ath_spectralsim_reportset nodes. Each ath_spectralsim_reportset will have a
 * linked list of ath_spectralsim_report nodes. When the user requests for a
 * given PHY mode and Spectral configuration, we find the appropriate
 * ath_spectralsim_reportset, and then serve ath_spectralsim_report instances
 * from the linked list. If required report count is higher than size of linked
 * list (or infinite), we repeatedly cycle through the linked list.  There can
 * be more elaborate data structures devised taking care of a large number of
 * possibilities, but we stick to a simple scheme given limited simulation
 * needs.
 */
typedef struct _ath_spectralsim_context {
	ath_spectralsim_reportset *bw20_headreportset;
	ath_spectralsim_reportset *bw40_headreportset;
	ath_spectralsim_reportset *bw80_headreportset;
	ath_spectralsim_reportset *bw160_headreportset;
	ath_spectralsim_reportset *bw80_80_headreportset;

	ath_spectralsim_reportset *curr_reportset;
	bool    is_enabled;
	bool    is_active;

	qdf_timer_t ssim_pherrdelivery_timer;
	u_int64_t ssim_starting_tsf64;
	u_int32_t ssim_period_ms; /* TODO: Support in microseconds */
	u_int32_t ssim_count;
	int (*populate_report_static)(ath_spectralsim_report *report,
				      enum phy_ch_width width,
						bool is_80_80);
} ath_spectralsim_context;

/* Helper Macros */

/* Allocate and populate reportset for a single configuration */
#define SPECTRAL_SIM_REPORTSET_ALLOCPOPL_SINGLE(simctx, reportset, width, \
						is_80_80)                 \
	{                                                                 \
	(reportset) = (ath_spectralsim_reportset *)                       \
		qdf_mem_malloc(sizeof(ath_spectralsim_reportset));        \
									  \
	if ((reportset) == NULL) {                                        \
		qdf_print("Spectral simulation: Could not allocate memory " \
			  "for report set\n"); \
		depopulate_simdata((simctx));                             \
		return -EPERM;                                                \
	}                                                                 \
									  \
	qdf_mem_zero((reportset), sizeof(ath_spectralsim_reportset));     \
									  \
	if (populate_reportset_static( \
		(simctx), (reportset), (width), (is_80_80)) != 0) { \
		depopulate_simdata((simctx));                             \
		return -EPERM;                                                \
	}                                                                 \
									  \
	(reportset)->next = NULL;                                         \
	}

/* Depopulate and free list of report sets */
#define SPECTRAL_SIM_REPORTSET_DEPOPLFREE_LIST(reportset)                 \
	{                                                                 \
	ath_spectralsim_reportset *curr_reportset = NULL;                 \
	ath_spectralsim_reportset *next_reportset = NULL;                 \
									\
	curr_reportset = (reportset);                                   \
									\
	while (curr_reportset) {                                        \
		next_reportset = curr_reportset->next;                  \
		depopulate_reportset(curr_reportset);                   \
		qdf_mem_free(curr_reportset);                           \
		curr_reportset = next_reportset;                        \
	}                                                               \
									\
	(reportset) = NULL;                                             \
	}

/* Values for static population */

/* 20 MHz */

static uint8_t reportdata_20_gen2[] = {
#ifdef BIG_ENDIAN_HOST
	0xbb,                           /* Signature */
	0xfb,                           /* Tag */
	0x00,                           /* Size */
	0x54,
	0x2e, 0x60, 0x0f, 0xe8,         /* FFT Summary A */
	0x00, 0x00, 0x04, 0x00,         /* FFT Summary B */
	0x00, 0x00, 0x00, 0x00,         /* Segment ID */
#else
	0x54,                           /* Length */
	0x00,
	0xfb,                           /* Tag */
	0xbb,                           /* Signature */
	0xe8, 0x0f, 0x60, 0x2e,         /* FFT Summary A */
	0x00, 0x04, 0x00, 0x00,         /* FFT Summary B */
	0x00, 0x00, 0x00, 0x00,         /* Segment ID */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 1, 2, 0, 1, 1, 1, 0,
	0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1,
	1, 1, 0, 2, 1, 2, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0,
};

static uint8_t reportdata_20_gen3[] = {
#ifdef BIG_ENDIAN_HOST
	0x12, 0x34, 0x56, 0x78,         /* fft_timestamp */
	0xfa,                           /* fft_hdr_sig */
	0x03,                           /* fft_hdr_tag */
	0x00,                           /* fft_hdr_length */
	0x14,
	0x0f, 0xf6, 0x00, 0xe0,
	0x00, 0x00, 0x2f, 0xba,
	0x20, 0xb4, 0x2c, 0x01,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#else
	0x78, 0x56, 0x34, 0x12,         /* fft_timestamp */
	0x14,                           /* fft_hdr_length */
	0x00,
	0x03,                           /* fft_hdr_tag */
	0xfa,                           /* fft_hdr_sig */
	0xe0, 0x00, 0xf6, 0x0f,
	0xba, 0x2f, 0x00, 0x00,
	0x01, 0x2c, 0xb4, 0x20,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 0, 1, 2, 0, 1, 1, 1, 0,
	0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1,
	1, 1, 0, 2, 1, 2, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0,
};

static struct target_if_spectral_rfqual_info rfqual_info_20 = {
				.rssi_comb = 1,

				.pc_rssi_info[0].rssi_pri20 = 1,
				.pc_rssi_info[0].rssi_sec20 = 128,
				.pc_rssi_info[0].rssi_sec40 = 128,
				.pc_rssi_info[0].rssi_sec80 = 128,

				.pc_rssi_info[1].rssi_pri20 = 128,
				.pc_rssi_info[1].rssi_sec20 = 128,
				.pc_rssi_info[1].rssi_sec40 = 128,
				.pc_rssi_info[1].rssi_sec80 = 128,

				.pc_rssi_info[2].rssi_pri20 = 128,
				.pc_rssi_info[2].rssi_sec20 = 128,
				.pc_rssi_info[2].rssi_sec40 = 128,
				.pc_rssi_info[2].rssi_sec80 = 128,

				.pc_rssi_info[3].rssi_pri20 = 128,
				.pc_rssi_info[3].rssi_sec20 = 128,
				.pc_rssi_info[3].rssi_sec40 = 128,
				.pc_rssi_info[3].rssi_sec80 = 128,

				.noise_floor[0] = -90,
				.noise_floor[1] = -90,
				.noise_floor[2] = -90,
				.noise_floor[3] = -90,
				};

static struct target_if_spectral_chan_info chan_info_20 = {
				.center_freq1 = 5180,
				.center_freq2 = 0,
				.chan_width = 20,
				};

static struct spectral_config config_20_1 =  {
				.ss_fft_period = 1,
				.ss_period = 35,
				.ss_count = 0,
				.ss_short_report = 1,
				.radar_bin_thresh_sel = 0,
				.ss_spectral_pri = 1,
				.ss_fft_size = 7,
				.ss_gc_ena = 1,
				.ss_restart_ena = 0,
				.ss_noise_floor_ref = 65440,
				.ss_init_delay = 80,
				.ss_nb_tone_thr = 12,
				.ss_str_bin_thr = 8,
				.ss_wb_rpt_mode = 0,
				.ss_rssi_rpt_mode = 0,
				.ss_rssi_thr = 240,
				.ss_pwr_format = 0,
				.ss_rpt_mode = 2,
				.ss_bin_scale = 1,
				.ss_dbm_adj = 1,
				.ss_chn_mask = 1,
				.ss_nf_cal[0] = 0,
				.ss_nf_cal[1] = 0,
				.ss_nf_cal[2] = 0,
				.ss_nf_cal[3] = 0,
				.ss_nf_cal[4] = 0,
				.ss_nf_cal[5] = 0,
				.ss_nf_pwr[0] = 0,
				.ss_nf_pwr[1] = 0,
				.ss_nf_pwr[2] = 0,
				.ss_nf_pwr[3] = 0,
				.ss_nf_pwr[4] = 0,
				.ss_nf_pwr[5] = 0,
				.ss_nf_temp_data = 0,
				};

/* 40 MHz */

static uint8_t reportdata_40_gen2[] = {
#ifdef BIG_ENDIAN_HOST
	0xbb,                           /* Signature */
	0xfb,                           /* Tag */
	0x00,                           /* Size */
	0x94,
	0x2e, 0x61, 0x0f, 0x80,         /* FFT Summary A */
	0x00, 0x00, 0x06, 0x00,         /* FFT Summary B */
	0x00, 0x00, 0x00, 0x00,         /* Segment ID */
#else
	0x94,                           /* Length */
	0x00,
	0xfb,                           /* Tag */
	0xbb,                           /* Signature */
	0x80, 0x0f, 0x61, 0x2e,         /* FFT Summary A */
	0x00, 0x06, 0x00, 0x00,         /* FFT Summary B */
	0x00, 0x00, 0x00, 0x00,         /* Segment ID */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1,
	0, 0, 0, 1, 0, 0, 0, 0, 2, 1, 0, 2, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0,
	1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0,
};

static uint8_t reportdata_40_gen3[] = {
#ifdef BIG_ENDIAN_HOST
	0x12, 0x34, 0x56, 0x78,         /* fft_timestamp */
	0xfa,                           /* fft_hdr_sig */
	0x03,                           /* fft_hdr_tag */
	0x00,                           /* fft_hdr_length */
	0x24,
	0x0f, 0xf6, 0x00, 0xe0,
	0x00, 0x00, 0x2f, 0xba,
	0x20, 0xb4, 0x2c, 0x01,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#else
	0x78, 0x56, 0x34, 0x12,         /* fft_timestamp */
	0x24,                           /* fft_hdr_length */
	0x00,
	0x03,                           /* fft_hdr_tag */
	0xfa,                           /* fft_hdr_sig */
	0xe0, 0x00, 0xf6, 0x0f,
	0xba, 0x2f, 0x00, 0x00,
	0x01, 0x2c, 0xb4, 0x20,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1,
	0, 0, 0, 1, 0, 0, 0, 0, 2, 1, 0, 2, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0,
	1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 0, 1, 0, 0, 0, 0,
};

static struct target_if_spectral_rfqual_info rfqual_info_40 = {
				.rssi_comb = 1,

				.pc_rssi_info[0].rssi_pri20 = 1,
				.pc_rssi_info[0].rssi_sec20 = 2,
				.pc_rssi_info[0].rssi_sec40 = 128,
				.pc_rssi_info[0].rssi_sec80 = 128,

				.pc_rssi_info[1].rssi_pri20 = 128,
				.pc_rssi_info[1].rssi_sec20 = 128,
				.pc_rssi_info[1].rssi_sec40 = 128,
				.pc_rssi_info[1].rssi_sec80 = 128,

				.pc_rssi_info[2].rssi_pri20 = 128,
				.pc_rssi_info[2].rssi_sec20 = 128,
				.pc_rssi_info[2].rssi_sec40 = 128,
				.pc_rssi_info[2].rssi_sec80 = 128,

				.pc_rssi_info[3].rssi_pri20 = 128,
				.pc_rssi_info[3].rssi_sec20 = 128,
				.pc_rssi_info[3].rssi_sec40 = 128,
				.pc_rssi_info[3].rssi_sec80 = 128,

				.noise_floor[0] = -90,
				.noise_floor[1] = -90,
				.noise_floor[2] = -90,
				.noise_floor[3] = -90,
				};

static struct target_if_spectral_chan_info chan_info_40 = {
				.center_freq1 = 5180,
				.center_freq2 = 0,
				.chan_width = 40,
				};

static struct spectral_config config_40_1 =  {
				.ss_fft_period = 1,
				.ss_period = 35,
				.ss_count = 0,
				.ss_short_report = 1,
				.radar_bin_thresh_sel = 0,
				.ss_spectral_pri = 1,
				.ss_fft_size = 8,
				.ss_gc_ena = 1,
				.ss_restart_ena = 0,
				.ss_noise_floor_ref = 65440,
				.ss_init_delay = 80,
				.ss_nb_tone_thr = 12,
				.ss_str_bin_thr = 8,
				.ss_wb_rpt_mode = 0,
				.ss_rssi_rpt_mode = 0,
				.ss_rssi_thr = 240,
				.ss_pwr_format = 0,
				.ss_rpt_mode = 2,
				.ss_bin_scale = 1,
				.ss_dbm_adj = 1,
				.ss_chn_mask = 1,
				.ss_nf_cal[0] = 0,
				.ss_nf_cal[1] = 0,
				.ss_nf_cal[2] = 0,
				.ss_nf_cal[3] = 0,
				.ss_nf_cal[4] = 0,
				.ss_nf_cal[5] = 0,
				.ss_nf_pwr[0] = 0,
				.ss_nf_pwr[1] = 0,
				.ss_nf_pwr[2] = 0,
				.ss_nf_pwr[3] = 0,
				.ss_nf_pwr[4] = 0,
				.ss_nf_pwr[5] = 0,
				.ss_nf_temp_data = 0,
				};

/* 80 MHz */

static uint8_t reportdata_80_gen2[] = {
#ifdef BIG_ENDIAN_HOST
	0xbb,                           /* Signature */
	0xfb,                           /* Tag */
	0x01,                           /* Size */
	0x14,
	0x19, 0xeb, 0x80, 0x40,         /* FFT Summary A */
	0x00, 0x00, 0x10, 0x00,         /* FFT Summary B */
	0x00, 0x00, 0x00, 0x00,         /* Segment ID */
#else
	0x14,                           /* Length */
	0x01,
	0xfb,                           /* Tag */
	0xbb,                           /* Signature */
	0x40, 0x80, 0xeb, 0x19,         /* FFT Summary A */
	0x00, 0x10, 0x00, 0x00,         /* FFT Summary B */
	0x00, 0x00, 0x00, 0x00,         /* Segment ID */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static uint8_t reportdata_80_gen3[] = {
#ifdef BIG_ENDIAN_HOST
	0x12, 0x34, 0x56, 0x78,         /* fft_timestamp */
	0xfa,                           /* fft_hdr_sig */
	0x03,                           /* fft_hdr_tag */
	0x00,                           /* fft_hdr_length */
	0x44,
	0x0f, 0xf6, 0x00, 0xe0,
	0x00, 0x00, 0x2f, 0xba,
	0x20, 0xb4, 0x2c, 0x01,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#else
	0x78, 0x56, 0x34, 0x12,         /* fft_timestamp */
	0x44,                           /* fft_hdr_length */
	0x00,
	0x03,                           /* fft_hdr_tag */
	0xfa,                           /* fft_hdr_sig */
	0xe0, 0x00, 0xf6, 0x0f,
	0xba, 0x2f, 0x00, 0x00,
	0x01, 0x2c, 0xb4, 0x20,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static struct target_if_spectral_rfqual_info rfqual_info_80 = {
				.rssi_comb = 16,

				.pc_rssi_info[0].rssi_pri20 = 16,
				.pc_rssi_info[0].rssi_sec20 = 17,
				.pc_rssi_info[0].rssi_sec40 = 0,
				.pc_rssi_info[0].rssi_sec80 = 128,

				.pc_rssi_info[1].rssi_pri20 = 128,
				.pc_rssi_info[1].rssi_sec20 = 128,
				.pc_rssi_info[1].rssi_sec40 = 128,
				.pc_rssi_info[1].rssi_sec80 = 128,

				.pc_rssi_info[2].rssi_pri20 = 128,
				.pc_rssi_info[2].rssi_sec20 = 128,
				.pc_rssi_info[2].rssi_sec40 = 128,
				.pc_rssi_info[2].rssi_sec80 = 128,

				.pc_rssi_info[3].rssi_pri20 = 128,
				.pc_rssi_info[3].rssi_sec20 = 128,
				.pc_rssi_info[3].rssi_sec40 = 128,
				.pc_rssi_info[3].rssi_sec80 = 128,

				.noise_floor[0] = -90,
				.noise_floor[1] = -90,
				.noise_floor[2] = -90,
				.noise_floor[3] = -90,
				};

static struct target_if_spectral_chan_info chan_info_80 = {
				.center_freq1 = 5210,
				.center_freq2 = 0,
				.chan_width = 80,
				};

static struct spectral_config config_80_1 =  {
				.ss_fft_period = 1,
				.ss_period = 35,
				.ss_count = 0,
				.ss_short_report = 1,
				.radar_bin_thresh_sel = 0,
				.ss_spectral_pri = 1,
				.ss_fft_size = 9,
				.ss_gc_ena = 1,
				.ss_restart_ena = 0,
				.ss_noise_floor_ref = 65440,
				.ss_init_delay = 80,
				.ss_nb_tone_thr = 12,
				.ss_str_bin_thr = 8,
				.ss_wb_rpt_mode = 0,
				.ss_rssi_rpt_mode = 0,
				.ss_rssi_thr = 240,
				.ss_pwr_format = 0,
				.ss_rpt_mode = 2,
				.ss_bin_scale = 1,
				.ss_dbm_adj = 1,
				.ss_chn_mask = 1,
				.ss_nf_cal[0] = 0,
				.ss_nf_cal[1] = 0,
				.ss_nf_cal[2] = 0,
				.ss_nf_cal[3] = 0,
				.ss_nf_cal[4] = 0,
				.ss_nf_cal[5] = 0,
				.ss_nf_pwr[0] = 0,
				.ss_nf_pwr[1] = 0,
				.ss_nf_pwr[2] = 0,
				.ss_nf_pwr[3] = 0,
				.ss_nf_pwr[4] = 0,
				.ss_nf_pwr[5] = 0,
				.ss_nf_temp_data = 0,
				};

/* 160 MHz */

static uint8_t reportdata_160_gen2[] = {
	/* Segment 1 */
#ifdef BIG_ENDIAN_HOST
	0xbb,                           /* Signature */
	0xfb,                           /* Tag */
	0x01,                           /* Size */
	0x14,
	0x23, 0x66, 0x00, 0x40,         /* FFT Summary A */
	0x5c, 0x5c, 0x78, 0x00,         /* FFT Summary B */
	0x00, 0x00, 0x00, 0x00,         /* Segment ID */
#else
	0x14,                           /* Length */
	0x01,
	0xfb,                           /* Tag */
	0xbb,                           /* Signature */
	0x40, 0x00, 0x66, 0x23,         /* FFT Summary A */
	0x00, 0x78, 0x5c, 0x5c,         /* FFT Summary B */
	0x00, 0x00, 0x00, 0x00,         /* Segment ID */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 2, 4, 60, 4, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,

	/* Segment 2 */
#ifdef BIG_ENDIAN_HOST
	0xbb,                           /* Signature */
	0xfb,                           /* Tag */
	0x01,                           /* Size */
	0x14,
	0x23, 0x66, 0x00, 0x40,         /* FFT Summary A */
	0x5c, 0x5c, 0x78, 0x00,         /* FFT Summary B */
	0x00, 0x00, 0x00, 0x01,         /* Segment ID */
#else
	0x14,                           /* Length */
	0x01,
	0xfb,                           /* Tag */
	0xbb,                           /* Signature */
	0x40, 0x00, 0x66, 0x23,         /* FFT Summary A */
	0x00, 0x78, 0x5c, 0x5c,         /* FFT Summary B */
	0x01, 0x00, 0x00, 0x00,         /* Segment ID */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 2, 4, 60, 4, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,
};

static uint8_t reportdata_160_gen3[] = {
	/* Segment 1 */
#ifdef BIG_ENDIAN_HOST
	0x12, 0x34, 0x56, 0x78,         /* fft_timestamp */
	0xfa,                           /* fft_hdr_sig */
	0x03,                           /* fft_hdr_tag */
	0x00,                           /* fft_hdr_length */
	0x44,
	0x0f, 0xf6, 0x00, 0xe0,
	0x00, 0x00, 0x2f, 0xba,
	0x20, 0xb4, 0x2c, 0x01,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#else
	0x78, 0x56, 0x34, 0x12,         /* fft_timestamp */
	0x44,                           /* fft_hdr_length */
	0x00,
	0x03,                           /* fft_hdr_tag */
	0xfa,                           /* fft_hdr_sig */
	0xe0, 0x00, 0xf6, 0x0f,
	0xba, 0x2f, 0x00, 0x00,
	0x01, 0x2c, 0xb4, 0x20,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 2, 4, 60, 4, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	/* Segment 2 */
#ifdef BIG_ENDIAN_HOST
	0x12, 0x34, 0x56, 0x78,         /* fft_timestamp */
	0xfa,                           /* fft_hdr_sig */
	0x03,                           /* fft_hdr_tag */
	0x00,                           /* fft_hdr_length */
	0x44,
	0x0f, 0xf6, 0x00, 0xe1,
	0x00, 0x00, 0x2f, 0xba,
	0x20, 0xb4, 0x2c, 0x01,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#else
	0x78, 0x56, 0x34, 0x12,         /* fft_timestamp */
	0x44,                           /* fft_hdr_length */
	0x00,
	0x03,                           /* fft_hdr_tag */
	0xfa,                           /* fft_hdr_sig */
	0xe1, 0x00, 0xf6, 0x0f,
	0xba, 0x2f, 0x00, 0x00,
	0x01, 0x2c, 0xb4, 0x20,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 2, 4, 60, 4, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static struct target_if_spectral_rfqual_info rfqual_info_160 = {
				.rssi_comb = 3,

				.pc_rssi_info[0].rssi_pri20 = 3,
				.pc_rssi_info[0].rssi_sec20 = 12,
				.pc_rssi_info[0].rssi_sec40 = 41,
				.pc_rssi_info[0].rssi_sec80 = 128,

				.pc_rssi_info[1].rssi_pri20 = 128,
				.pc_rssi_info[1].rssi_sec20 = 128,
				.pc_rssi_info[1].rssi_sec40 = 128,
				.pc_rssi_info[1].rssi_sec80 = 128,

				.pc_rssi_info[2].rssi_pri20 = 128,
				.pc_rssi_info[2].rssi_sec20 = 128,
				.pc_rssi_info[2].rssi_sec40 = 128,
				.pc_rssi_info[2].rssi_sec80 = 128,

				.pc_rssi_info[3].rssi_pri20 = 128,
				.pc_rssi_info[3].rssi_sec20 = 128,
				.pc_rssi_info[3].rssi_sec40 = 128,
				.pc_rssi_info[3].rssi_sec80 = 128,

				.noise_floor[0] = -90,
				.noise_floor[1] = -90,
				.noise_floor[2] = -90,
				.noise_floor[3] = -90,
				};

static struct target_if_spectral_chan_info chan_info_160 = {
				.center_freq1 = 5250,
				.center_freq2 = 0,
				.chan_width = 160,
				};

static struct spectral_config config_160_1 =  {
				.ss_fft_period = 1,
				.ss_period = 35,
				.ss_count = 0,
				.ss_short_report = 1,
				.radar_bin_thresh_sel = 0,
				.ss_spectral_pri = 1,
				.ss_fft_size = 9,
				.ss_gc_ena = 1,
				.ss_restart_ena = 0,
				.ss_noise_floor_ref = 65440,
				.ss_init_delay = 80,
				.ss_nb_tone_thr = 12,
				.ss_str_bin_thr = 8,
				.ss_wb_rpt_mode = 0,
				.ss_rssi_rpt_mode = 0,
				.ss_rssi_thr = 240,
				.ss_pwr_format = 0,
				.ss_rpt_mode = 2,
				.ss_bin_scale = 1,
				.ss_dbm_adj = 1,
				.ss_chn_mask = 1,
				.ss_nf_cal[0] = 0,
				.ss_nf_cal[1] = 0,
				.ss_nf_cal[2] = 0,
				.ss_nf_cal[3] = 0,
				.ss_nf_cal[4] = 0,
				.ss_nf_cal[5] = 0,
				.ss_nf_pwr[0] = 0,
				.ss_nf_pwr[1] = 0,
				.ss_nf_pwr[2] = 0,
				.ss_nf_pwr[3] = 0,
				.ss_nf_pwr[4] = 0,
				.ss_nf_pwr[5] = 0,
				.ss_nf_temp_data = 0,
				};

/* 80+80 MHz */

static uint8_t reportdata_80_80_gen2[] = {
	/* Segment 1 */
#ifdef BIG_ENDIAN_HOST
	0xbb,                           /* Signature */
	0xfb,                           /* Tag */
	0x01,                           /* Size */
	0x14,
	0x23, 0x66, 0x00, 0x40,         /* FFT Summary A */
	0x64, 0x64, 0x89, 0x00,         /* FFT Summary B */
	0x00, 0x00, 0x00, 0x00,         /* Segment ID */
#else
	0x14,                           /* Length */
	0x01,
	0xfb,                           /* Tag */
	0xbb,                           /* Signature */
	0x40, 0x00, 0x66, 0x23,         /* FFT Summary A */
	0x00, 0x89, 0x64, 0x64,         /* FFT Summary B */
	0x00, 0x00, 0x00, 0x00,         /* Segment ID */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 2, 6, 68, 5, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,

	/* Segment 2 */
#ifdef BIG_ENDIAN_HOST
	0xbb,                           /* Signature */
	0xfb,                           /* Tag */
	0x01,                           /* Size */
	0x14,
	0x23, 0x66, 0x00, 0x40,         /* FFT Summary A */
	0x64, 0x64, 0x89, 0x00,         /* FFT Summary B */
	0x00, 0x00, 0x00, 0x01,         /* Segment ID */
#else
	0x14,                           /* Length */
	0x01,
	0xfb,                           /* Tag */
	0xbb,                           /* Signature */
	0x40, 0x00, 0x66, 0x23,         /* FFT Summary A */
	0x00, 0x89, 0x64, 0x64,         /* FFT Summary B */
	0x01, 0x00, 0x00, 0x00,         /* Segment ID */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 2, 6, 68, 5, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0,
};

static uint8_t reportdata_80_80_gen3[] = {
	/* Segment 1 */
#ifdef BIG_ENDIAN_HOST
	0x12, 0x34, 0x56, 0x78,         /* fft_timestamp */
	0xfa,                           /* fft_hdr_sig */
	0x03,                           /* fft_hdr_tag */
	0x00,                           /* fft_hdr_length */
	0x44,
	0x0f, 0xf6, 0x00, 0xe0,
	0x00, 0x00, 0x2f, 0xba,
	0x20, 0xb4, 0x2c, 0x01,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#else
	0x78, 0x56, 0x34, 0x12,         /* fft_timestamp */
	0x44,                           /* fft_hdr_length */
	0x00,
	0x03,                           /* fft_hdr_tag */
	0xfa,                           /* fft_hdr_sig */
	0xe0, 0x00, 0xf6, 0x0f,
	0xba, 0x2f, 0x00, 0x00,
	0x01, 0x2c, 0xb4, 0x20,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 2, 6, 68, 5, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

	/* Segment 2 */
#ifdef BIG_ENDIAN_HOST
	0x12, 0x34, 0x56, 0x78,         /* fft_timestamp */
	0xfa,                           /* fft_hdr_sig */
	0x03,                           /* fft_hdr_tag */
	0x00,                           /* fft_hdr_length */
	0x44,
	0x0f, 0xf6, 0x00, 0xe1,
	0x00, 0x00, 0x2f, 0xba,
	0x20, 0xb4, 0x2c, 0x01,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#else
	0x78, 0x56, 0x34, 0x12,         /* fft_timestamp */
	0x44,                           /* fft_hdr_length */
	0x00,
	0x03,                           /* fft_hdr_tag */
	0xfa,                           /* fft_hdr_sig */
	0xe1, 0x00, 0xf6, 0x0f,
	0xba, 0x2f, 0x00, 0x00,
	0x01, 0x2c, 0xb4, 0x20,
	0x00, 0x00, 0x00, 0x00,         /* reserved */
#endif /* BIG_ENDIAN_HOST */
	/* FFT Data */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
	1, 1, 2, 6, 68, 5, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

static struct target_if_spectral_rfqual_info rfqual_info_80_80 = {
				.rssi_comb = 1,

				.pc_rssi_info[0].rssi_pri20 = 1,
				.pc_rssi_info[0].rssi_sec20 = 17,
				.pc_rssi_info[0].rssi_sec40 = 40,
				.pc_rssi_info[0].rssi_sec80 = 128,

				.pc_rssi_info[1].rssi_pri20 = 128,
				.pc_rssi_info[1].rssi_sec20 = 128,
				.pc_rssi_info[1].rssi_sec40 = 128,
				.pc_rssi_info[1].rssi_sec80 = 128,

				.pc_rssi_info[2].rssi_pri20 = 128,
				.pc_rssi_info[2].rssi_sec20 = 128,
				.pc_rssi_info[2].rssi_sec40 = 128,
				.pc_rssi_info[2].rssi_sec80 = 128,

				.pc_rssi_info[3].rssi_pri20 = 128,
				.pc_rssi_info[3].rssi_sec20 = 128,
				.pc_rssi_info[3].rssi_sec40 = 128,
				.pc_rssi_info[3].rssi_sec80 = 128,

				.noise_floor[0] = -90,
				.noise_floor[1] = -90,
				.noise_floor[2] = -90,
				.noise_floor[3] = -90,
				};

static struct target_if_spectral_chan_info chan_info_80_80 = {
				.center_freq1 = 5210,
				.center_freq2 = 5530,
				.chan_width = 160,
				};

static struct spectral_config config_80_80_1 =  {
				.ss_fft_period = 1,
				.ss_period = 35,
				.ss_count = 0,
				.ss_short_report = 1,
				.radar_bin_thresh_sel = 0,
				.ss_spectral_pri = 1,
				.ss_fft_size = 9,
				.ss_gc_ena = 1,
				.ss_restart_ena = 0,
				.ss_noise_floor_ref = 65440,
				.ss_init_delay = 80,
				.ss_nb_tone_thr = 12,
				.ss_str_bin_thr = 8,
				.ss_wb_rpt_mode = 0,
				.ss_rssi_rpt_mode = 0,
				.ss_rssi_thr = 240,
				.ss_pwr_format = 0,
				.ss_rpt_mode = 2,
				.ss_bin_scale = 1,
				.ss_dbm_adj = 1,
				.ss_chn_mask = 1,
				.ss_nf_cal[0] = 0,
				.ss_nf_cal[1] = 0,
				.ss_nf_cal[2] = 0,
				.ss_nf_cal[3] = 0,
				.ss_nf_cal[4] = 0,
				.ss_nf_cal[5] = 0,
				.ss_nf_pwr[0] = 0,
				.ss_nf_pwr[1] = 0,
				.ss_nf_pwr[2] = 0,
				.ss_nf_pwr[3] = 0,
				.ss_nf_pwr[4] = 0,
				.ss_nf_pwr[5] = 0,
				.ss_nf_temp_data = 0,
				};

#endif /* QCA_SUPPORT_SPECTRAL_SIMULATION */
#endif /* _SPECTRAL_SIM_INTERNAL_H_ */
