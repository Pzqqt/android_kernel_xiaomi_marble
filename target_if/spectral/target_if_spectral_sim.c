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

#ifdef QCA_SUPPORT_SPECTRAL_SIMULATION
#include "target_if_spectral.h"
#include "target_if_spectral_sim.h"
#include "target_if_spectral_sim_int.h"
#include "_ieee80211.h"
#include "ieee80211_api.h"
#include "ieee80211_defines.h"
#include "qdf_types.h"
#include "ieee80211_var.h"
#include <wlan_mlme_dispatcher.h>

/* Helper functions */

int tif_populate_reportset_fromfile(ath_spectralsim_reportset * reportset,
				    enum phy_ch_width width,
				bool is_80_80);
static int populate_report_static_gen2(ath_spectralsim_report *report,
				       enum phy_ch_width width,
					bool is_80_80);
static int populate_report_static_gen3(ath_spectralsim_report *report,
				       enum phy_ch_width width,
					bool is_80_80);
static void depopulate_report(ath_spectralsim_report *report);

static int populate_reportset_static(ath_spectralsim_context *simctx,
				     ath_spectralsim_reportset *reportset,
						enum phy_ch_width width,
						bool is_80_80);
static void depopulate_reportset(ath_spectralsim_reportset *reportset);

static int populate_simdata(ath_spectralsim_context *simctx);
static void depopulate_simdata(ath_spectralsim_context *simctx);
static OS_TIMER_FUNC(spectral_sim_phyerrdelivery_handler);

/* Static configuration.
 * For now, we will be having a single configuration per BW, and a single
 * report per configuration (since we need the data only for ensuring correct
 * format handling).
 *
 * Extend this for more functionality if required in the future.
 */

/* Statically populate simulation data for one report. */
static int populate_report_static_gen2(ath_spectralsim_report *report,
				       enum phy_ch_width width,
					bool is_80_80)
{
	qdf_assert_always(report);

	switch (width) {
	case CH_WIDTH_20MHZ:
		report->data = NULL;
		report->data = (u_int8_t *)
		qdf_mem_malloc(sizeof(reportdata_20_gen2));

		if (!report->data) {
		qdf_print("Spectral simulation: Could not allocate memory for "
				"report data\n");
		goto bad;
		}

		report->datasize = sizeof(reportdata_20_gen2);
		qdf_mem_copy(report->data,
			     reportdata_20_gen2, report->datasize);

		qdf_mem_copy(&report->rfqual_info,
			     &rfqual_info_20, sizeof(report->rfqual_info));

		qdf_mem_copy(&report->chan_info,
			     &chan_info_20, sizeof(report->chan_info));

		break;
	case CH_WIDTH_40MHZ:
		report->data = NULL;
		report->data = (u_int8_t *)
		qdf_mem_malloc(sizeof(reportdata_40_gen2));

		if (!report->data) {
		qdf_print("Spectral simulation: Could not allocate memory for "
				"report data\n");
		goto bad;
		}

		report->datasize = sizeof(reportdata_40_gen2);
		qdf_mem_copy(report->data,
			     reportdata_40_gen2, report->datasize);

		qdf_mem_copy(&report->rfqual_info,
			     &rfqual_info_40, sizeof(report->rfqual_info));

		qdf_mem_copy(&report->chan_info,
			     &chan_info_40, sizeof(report->chan_info));

		break;
	case CH_WIDTH_80MHZ:
		report->data = NULL;
		report->data = (u_int8_t *)
		qdf_mem_malloc(sizeof(reportdata_80_gen2));

		if (!report->data) {
		qdf_print("Spectral simulation: Could not allocate memory for "
				"report data\n");
		goto bad;
		}

		report->datasize = sizeof(reportdata_80_gen2);
		qdf_mem_copy(report->data,
			     reportdata_80_gen2, report->datasize);

		qdf_mem_copy(&report->rfqual_info,
			     &rfqual_info_80, sizeof(report->rfqual_info));

		qdf_mem_copy(&report->chan_info,
			     &chan_info_80, sizeof(report->chan_info));

		break;
	case CH_WIDTH_160MHZ:
		if (is_80_80) {
		report->data = NULL;
		report->data = (u_int8_t *)
			qdf_mem_malloc(sizeof(reportdata_80_80_gen2));

		if (!report->data) {
			qdf_print("Spectral simulation: Could not allocate "
				  "memory for report data\n");
			goto bad;
		}

		report->datasize = sizeof(reportdata_80_80_gen2);
		qdf_mem_copy(report->data,
			     reportdata_80_80_gen2, report->datasize);

		qdf_mem_copy(&report->rfqual_info,
			     &rfqual_info_80_80, sizeof(report->rfqual_info));

		qdf_mem_copy(&report->chan_info,
			     &chan_info_80_80, sizeof(report->chan_info));

		} else {
		report->data = NULL;
		report->data = (u_int8_t *)
			qdf_mem_malloc(sizeof(reportdata_160_gen2));

		if (!report->data) {
			qdf_print("Spectral simulation: Could not allocate "
				  "memory for report data\n");
			goto bad;
		}

		report->datasize = sizeof(reportdata_160_gen2);
		qdf_mem_copy(report->data,
			     reportdata_160_gen2, report->datasize);

		qdf_mem_copy(&report->rfqual_info,
			     &rfqual_info_160, sizeof(report->rfqual_info));

		qdf_mem_copy(&report->chan_info,
			     &chan_info_160, sizeof(report->chan_info));
		}
		break;
	default:
		qdf_print("Unhandled width. Please correct. Asserting\n");
		qdf_assert_always(0);
	}

	return 0;

bad:
	return -EPERM;
}

/* Statically populate simulation data for one report. */
static int populate_report_static_gen3(ath_spectralsim_report *report,
				       enum phy_ch_width width,
					bool is_80_80)
{
	qdf_assert_always(report);

	switch (width) {
	case CH_WIDTH_20MHZ:
		report->data = NULL;
		report->data = (u_int8_t *)
		qdf_mem_malloc(sizeof(reportdata_20_gen3));

		if (!report->data) {
			qdf_print("Spectral simulation: Could not allocate "
				  "memory for report data\n");
		goto bad;
		}

		report->datasize = sizeof(reportdata_20_gen3);
		qdf_mem_copy(report->data,
			     reportdata_20_gen3, report->datasize);

		qdf_mem_copy(&report->rfqual_info,
			     &rfqual_info_20, sizeof(report->rfqual_info));

		qdf_mem_copy(&report->chan_info,
			     &chan_info_20, sizeof(report->chan_info));

		break;
	case CH_WIDTH_40MHZ:
		report->data = NULL;
		report->data = (u_int8_t *)
		qdf_mem_malloc(sizeof(reportdata_40_gen3));

		if (!report->data) {
			qdf_print("Spectral simulation: Could not allocate "
				  "memory for report data\n");
		goto bad;
		}

		report->datasize = sizeof(reportdata_40_gen3);
		qdf_mem_copy(report->data,
			     reportdata_40_gen3, report->datasize);

		qdf_mem_copy(&report->rfqual_info,
			     &rfqual_info_40, sizeof(report->rfqual_info));

		qdf_mem_copy(&report->chan_info,
			     &chan_info_40, sizeof(report->chan_info));

		break;
	case CH_WIDTH_80MHZ:
		report->data = NULL;
		report->data = (u_int8_t *)
		qdf_mem_malloc(sizeof(reportdata_80_gen3));

		if (!report->data) {
		qdf_print("Spectral simulation: Could not allocate memory for "
				"report data\n");
		goto bad;
		}

		report->datasize = sizeof(reportdata_80_gen3);
		qdf_mem_copy(report->data,
			     reportdata_80_gen3, report->datasize);

		qdf_mem_copy(&report->rfqual_info,
			     &rfqual_info_80, sizeof(report->rfqual_info));

		qdf_mem_copy(&report->chan_info,
			     &chan_info_80, sizeof(report->chan_info));

		break;
	case CH_WIDTH_160MHZ:
		if (is_80_80) {
		report->data = NULL;
		report->data = (u_int8_t *)
			qdf_mem_malloc(sizeof(reportdata_80_80_gen3));

		if (!report->data) {
			qdf_print("Spectral simulation: Could not allocate "
				  "memory for report data\n");
			goto bad;
		}

		report->datasize = sizeof(reportdata_80_80_gen3);
		qdf_mem_copy(report->data,
			     reportdata_80_80_gen3, report->datasize);

		qdf_mem_copy(&report->rfqual_info,
			     &rfqual_info_80_80, sizeof(report->rfqual_info));

		qdf_mem_copy(&report->chan_info,
			     &chan_info_80_80, sizeof(report->chan_info));

		} else {
		report->data = NULL;
		report->data = (u_int8_t *)
			qdf_mem_malloc(sizeof(reportdata_160_gen3));

		if (!report->data) {
			qdf_print("Spectral simulation: Could not allocate "
				  "memory for report data\n");
			goto bad;
		}

		report->datasize = sizeof(reportdata_160_gen3);
		qdf_mem_copy(report->data,
			     reportdata_160_gen3, report->datasize);

		qdf_mem_copy(&report->rfqual_info,
			     &rfqual_info_160, sizeof(report->rfqual_info));

		qdf_mem_copy(&report->chan_info,
			     &chan_info_160, sizeof(report->chan_info));
		}
		break;
	default:
		qdf_print("Unhandled width. Please correct. Asserting\n");
		qdf_assert_always(0);
	}

	return 0;

bad:
	return -EPERM;
}

static void depopulate_report(ath_spectralsim_report *report)
{
	if (!report)
	return;

	if (report->data) {
	qdf_mem_free(report->data);
	report->data = NULL;
	report->datasize = 0;
	}
}

/* Statically populate simulation data for a given configuration. */
static int populate_reportset_static(ath_spectralsim_context *simctx,
				     ath_spectralsim_reportset *reportset,
						enum phy_ch_width width,
						bool is_80_80)
{
	int ret = 0;
	ath_spectralsim_report *report = NULL;

	qdf_assert_always(reportset);

	reportset->headreport = NULL;
	reportset->curr_report = NULL;

	/* For now, we populate only one report */
	report = (ath_spectralsim_report *)
	qdf_mem_malloc(sizeof(ath_spectralsim_report));

	if (!report) {
		qdf_print("Spectral simulation: Could not allocate memory "
			  "for report.\n");
		goto bad;
	}

	qdf_mem_zero(report, sizeof(*report));

	switch (width) {
	case CH_WIDTH_20MHZ:
		qdf_mem_copy(&reportset->config,
			     &config_20_1, sizeof(reportset->config));

		ret = simctx->populate_report_static(report, CH_WIDTH_20MHZ, 0);
		if (ret != 0)
		goto bad;

		report->next = NULL;
		reportset->headreport = report;
		break;
	case CH_WIDTH_40MHZ:
		qdf_mem_copy(&reportset->config,
			     &config_40_1, sizeof(reportset->config));

		ret = simctx->populate_report_static(report, CH_WIDTH_40MHZ, 0);
		if (ret != 0)
		goto bad;

		report->next = NULL;
		reportset->headreport = report;
		break;
	case CH_WIDTH_80MHZ:
		qdf_mem_copy(&reportset->config,
			     &config_80_1, sizeof(reportset->config));

		ret = simctx->populate_report_static(report, CH_WIDTH_80MHZ, 0);
		if (ret != 0)
		goto bad;

		report->next = NULL;
		reportset->headreport = report;
		break;
	case CH_WIDTH_160MHZ:
		if (is_80_80) {
		qdf_mem_copy(&reportset->config,
			     &config_80_80_1, sizeof(reportset->config));

		ret = simctx->populate_report_static(
			report,
			 CH_WIDTH_160MHZ,
			 1);
		if (ret != 0)
			goto bad;

		report->next = NULL;
		reportset->headreport = report;
		} else {
		qdf_mem_copy(&reportset->config,
			     &config_160_1, sizeof(reportset->config));

		ret = simctx->populate_report_static(
			report,
			 CH_WIDTH_160MHZ,
			 0);
		if (ret != 0)
			goto bad;

		report->next = NULL;
		reportset->headreport = report;
		}
		break;
	default:
		qdf_print("Unhandled width. Please rectify.\n");
		qdf_assert_always(0);
	};

	reportset->curr_report = reportset->headreport;

	return 0;

bad:
	depopulate_reportset(reportset);
	return -EPERM;
}

static void depopulate_reportset(ath_spectralsim_reportset *reportset)
{
	ath_spectralsim_report *curr_report = NULL;
	ath_spectralsim_report *next_report = NULL;

	if (!reportset)
	return;

	curr_report = reportset->headreport;

	while (curr_report) {
	next_report = curr_report->next;
	depopulate_report(curr_report);
	qdf_mem_free(curr_report);
	curr_report = next_report;
	}
}

/* Populate simulation data for a given bandwidth by loading from a file.
 * This is a place-holder only. To be implemented in the future on a need
 * basis.
 *
 * A different file per bandwidth is suggested for better segregation of data
 * sets (since data is likely to be very different across BWs).
 */
int tif_populate_reportset_fromfile(ath_spectralsim_reportset *reportset,
				    enum phy_ch_width width,
					bool is_80_80)
{
	qdf_print("%s: To be implemented if required\n", __func__);

	return 0;
}

/* Populate simulation data */
static int populate_simdata(ath_spectralsim_context *simctx)
{
	/* For now, we use static population. Switch to loading from a file if
	 * needed in the future.
	 */

	simctx->bw20_headreportset = NULL;
	SPECTRAL_SIM_REPORTSET_ALLOCPOPL_SINGLE(simctx,
						simctx->bw20_headreportset,
		CH_WIDTH_20MHZ,
		0);

	simctx->bw40_headreportset = NULL;
	SPECTRAL_SIM_REPORTSET_ALLOCPOPL_SINGLE(simctx,
						simctx->bw40_headreportset,
		CH_WIDTH_40MHZ,
		0);

	simctx->bw80_headreportset = NULL;
	SPECTRAL_SIM_REPORTSET_ALLOCPOPL_SINGLE(simctx,
						simctx->bw80_headreportset,
		CH_WIDTH_80MHZ,
		0);

	simctx->bw160_headreportset = NULL;
	SPECTRAL_SIM_REPORTSET_ALLOCPOPL_SINGLE(simctx,
						simctx->bw160_headreportset,
		CH_WIDTH_160MHZ,
		0);

	simctx->bw80_80_headreportset = NULL;
	SPECTRAL_SIM_REPORTSET_ALLOCPOPL_SINGLE(simctx,
						simctx->bw80_80_headreportset,
		CH_WIDTH_160MHZ,
		1);

	simctx->curr_reportset = NULL;

	simctx->is_enabled = false;
	simctx->is_active = false;

	simctx->ssim_starting_tsf64 = 0;
	simctx->ssim_count = 0;
	simctx->ssim_period_ms = 0;

	return 0;
}

static void depopulate_simdata(ath_spectralsim_context *simctx)
{
	if (!simctx)
		return;

	SPECTRAL_SIM_REPORTSET_DEPOPLFREE_LIST(simctx->bw20_headreportset);
	SPECTRAL_SIM_REPORTSET_DEPOPLFREE_LIST(simctx->bw40_headreportset);
	SPECTRAL_SIM_REPORTSET_DEPOPLFREE_LIST(simctx->bw80_headreportset);
	SPECTRAL_SIM_REPORTSET_DEPOPLFREE_LIST(simctx->bw160_headreportset);
	SPECTRAL_SIM_REPORTSET_DEPOPLFREE_LIST(simctx->bw80_80_headreportset);
}

static
OS_TIMER_FUNC(spectral_sim_phyerrdelivery_handler)
{
	struct target_if_spectral *spectral = NULL;
	ath_spectralsim_context *simctx = NULL;
	ath_spectralsim_reportset *curr_reportset = NULL;
	ath_spectralsim_report *curr_report = NULL;
	struct target_if_spectral_acs_stats acs_stats;
	u_int64_t curr_tsf64 = 0;
	struct target_if_spectral_ops *p_sops;

	OS_GET_TIMER_ARG(spectral, struct target_if_spectral *);
	qdf_assert_always(spectral);

	p_sops = GET_TIF_SPECTRAL_OPS(spectral);
	qdf_assert_always(spectral);

	simctx = (ath_spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	if (!simctx->is_active)
		return;

	curr_reportset = simctx->curr_reportset;
	qdf_assert_always(curr_reportset);

	curr_report = curr_reportset->curr_report;
	qdf_assert_always(curr_report);

	qdf_assert_always(curr_reportset->headreport);

	/* We use a simulation TSF since in offload architectures we can't
	 * expect to
	 * get an accurate current TSF from HW.
	 * In case of TSF wrap over, we'll use it as-is for now since the
	 * simulation
	 * is intended only for format verification.
	 */
	curr_tsf64 = simctx->ssim_starting_tsf64 +
	((simctx->ssim_period_ms * simctx->ssim_count) * 1000);

	p_sops->spectral_process_phyerr(spectral,
		curr_report->data,
		curr_report->datasize,
		&curr_report->rfqual_info,
		&curr_report->chan_info,
		curr_tsf64,
		&acs_stats);

	simctx->ssim_count++;

	if (curr_report->next)
		curr_reportset->curr_report = curr_report->next;
	else
		curr_reportset->curr_report = curr_reportset->headreport;

	if (curr_reportset->config.ss_count != 0 &&
	    simctx->ssim_count == curr_reportset->config.ss_count) {
	tif_spectral_sim_stop_spectral_scan(spectral);
	} else {
	qdf_timer_start(&simctx->ssim_pherrdelivery_timer,
			simctx->ssim_period_ms);
	}
}

/* Module services */

int target_if_spectral_sim_attach(struct target_if_spectral *spectral)
{
	ath_spectralsim_context *simctx = NULL;

	qdf_assert_always(spectral);

	simctx = (ath_spectralsim_context *)
		qdf_mem_malloc(sizeof(ath_spectralsim_context));

	if (!simctx) {
		qdf_print("Spectral simulation: Could not allocate memory for "
			  "context\n");
		return -EPERM;
	}

	qdf_mem_zero(simctx, sizeof(*simctx));

	spectral->simctx = simctx;

	if (spectral->spectral_gen == SPECTRAL_GEN2)
		simctx->populate_report_static = populate_report_static_gen2;
	else if (spectral->spectral_gen == SPECTRAL_GEN3)
		simctx->populate_report_static = populate_report_static_gen3;

	if (populate_simdata(simctx) != 0) {
	qdf_mem_free(simctx);
	spectral->simctx = NULL;
	qdf_print("Spectral simulation attach failed\n");
	return -EPERM;
	}

	qdf_timer_init(NULL,
		       &simctx->ssim_pherrdelivery_timer,
		spectral_sim_phyerrdelivery_handler,
		(void *)(spectral),
		QDF_TIMER_TYPE_WAKE_APPS);

	qdf_print("Spectral simulation attached\n");

	return 0;
}

void target_if_spectral_sim_detach(struct target_if_spectral *spectral)
{
	ath_spectralsim_context *simctx = NULL;

	qdf_assert_always(spectral);

	simctx = (ath_spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	qdf_timer_free(&simctx->ssim_pherrdelivery_timer);

	depopulate_simdata(simctx);
	qdf_mem_free(simctx);
	spectral->simctx = NULL;

	qdf_print("Spectral simulation detached\n");
}

u_int32_t tif_spectral_sim_is_spectral_active(void *arg)
{
	struct target_if_spectral *spectral = NULL;
	ath_spectralsim_context *simctx = NULL;

	spectral = (struct target_if_spectral *)arg;
	qdf_assert_always(spectral);

	simctx = (ath_spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	return simctx->is_active;
}
EXPORT_SYMBOL(tif_spectral_sim_is_spectral_active);

u_int32_t tif_spectral_sim_is_spectral_enabled(void *arg)
{
	struct target_if_spectral *spectral = NULL;
	ath_spectralsim_context *simctx = NULL;

	spectral = (struct target_if_spectral *)arg;
	qdf_assert_always(spectral);

	simctx = (ath_spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	return simctx->is_enabled;
}
EXPORT_SYMBOL(tif_spectral_sim_is_spectral_enabled);

u_int32_t tif_spectral_sim_start_spectral_scan(void *arg)
{
	struct target_if_spectral *spectral = NULL;
	ath_spectralsim_context *simctx = NULL;

	spectral = (struct target_if_spectral *)arg;
	qdf_assert_always(spectral);

	simctx = (ath_spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	if (!simctx->curr_reportset) {
		qdf_print("Spectral simulation: No current report set "
			  "configured  - unable to start simulated Spectral "
			  "scan\n");
		return 0;
	}

	if (!simctx->curr_reportset->curr_report) {
		qdf_print("Spectral simulation: No report data instances "
			  "populated - unable to start simulated Spectral "
			  "scan\n");
	return 0;
	}

	if (!simctx->is_enabled)
		simctx->is_enabled = true;

	simctx->is_active = true;

	/* Hardcoding current time as zero since it is simulation */
	simctx->ssim_starting_tsf64 = 0;
	simctx->ssim_count = 0;

	/* TODO: Support high resolution timer in microseconds if required, so
	 * that
	 * we can support default periods such as ~200 us.  For now, we use 1
	 * millisecond since the current use case for the simulation is to
	 * validate
	 * formats rather than have a time dependent classification.
	 */
	simctx->ssim_period_ms = 1;

	qdf_timer_start(&simctx->ssim_pherrdelivery_timer,
			simctx->ssim_period_ms);

	return 1;
}
EXPORT_SYMBOL(tif_spectral_sim_start_spectral_scan);

u_int32_t tif_spectral_sim_stop_spectral_scan(void *arg)
{
	struct target_if_spectral *spectral = NULL;
	ath_spectralsim_context *simctx = NULL;

	spectral = (struct target_if_spectral *)arg;
	qdf_assert_always(spectral);

	simctx = (ath_spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	qdf_timer_stop(&simctx->ssim_pherrdelivery_timer);

	simctx->is_active = false;
	simctx->is_enabled = false;

	simctx->ssim_starting_tsf64 = 0;
	simctx->ssim_count = 0;
	simctx->ssim_period_ms = 0;

	return 1;
}
EXPORT_SYMBOL(tif_spectral_sim_stop_spectral_scan);

u_int32_t tif_spectral_sim_configure_params(
	void *arg,
	 struct spectral_config *params)
{
	struct target_if_spectral *spectral = NULL;
	ath_spectralsim_context *simctx = NULL;
	enum wlan_phymode phymode;
	u_int8_t bw;
	ath_spectralsim_reportset *des_headreportset = NULL;
	ath_spectralsim_reportset *temp_reportset = NULL;
	bool is_invalid_width = false;
	struct wlan_objmgr_vdev *vdev = NULL;

	qdf_assert_always(params);

#ifdef SPECTRAL_SIM_DUMP_PARAM_DATA
	{
	int i = 0;

	qdf_print("\n");

	qdf_print("Spectral simulation: Param data dump:\n"
			"ss_fft_period=%hu\n"
			"ss_period=%hu\n"
			"ss_count=%hu\n"
			"ss_short_report=%hu\n"
			"radar_bin_thresh_sel=%hhu\n"
			"ss_spectral_pri=%hu\n"
			"ss_fft_size=%hu\n"
			"ss_gc_ena=%hu\n"
			"ss_restart_ena=%hu\n"
			"ss_noise_floor_ref=%hu\n"
			"ss_init_delay=%hu\n"
			"ss_nb_tone_thr=%hu\n"
			"ss_str_bin_thr=%hu\n"
			"ss_wb_rpt_mode=%hu\n"
			"ss_rssi_rpt_mode=%hu\n"
			"ss_rssi_thr=%hu\n"
			"ss_pwr_format=%hu\n"
			"ss_rpt_mode=%hu\n"
			"ss_bin_scale=%hu\n"
			"ss_dbm_adj=%hu\n"
			"ss_chn_mask=%hu\n"
			"ss_nf_temp_data=%d\n",
			params->ss_fft_period,
			params->ss_period,
			params->ss_count,
			params->ss_short_report,
			params->radar_bin_thresh_sel,
			params->ss_spectral_pri,
			params->ss_fft_size,
			params->ss_gc_ena,
			params->ss_restart_ena,
			params->ss_noise_floor_ref,
			params->ss_init_delay,
			params->ss_nb_tone_thr,
			params->ss_str_bin_thr,
			params->ss_wb_rpt_mode,
			params->ss_rssi_rpt_mode,
			params->ss_rssi_thr,
			params->ss_pwr_format,
			params->ss_rpt_mode,
			params->ss_bin_scale,
			params->ss_dbm_adj,
			params->ss_chn_mask,
			params->ss_nf_temp_data);

	for (i = 0; i < AH_MAX_CHAINS * 2; i++)
		qdf_print("ss_nf_cal[%d]=%hhd\n", i, params->ss_nf_cal[i]);

	for (i = 0; i < AH_MAX_CHAINS * 2; i++)
		qdf_print("ss_nf_pwr[%d]=%hhd\n", i, params->ss_nf_pwr[i]);

	qdf_print("\n");
	}
#endif /* SPECTRAL_SIM_DUMP_PARAM_DATA */

	spectral = (struct target_if_spectral *)arg;
	qdf_assert_always(spectral);

	simctx = (ath_spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	vdev = target_if_spectral_get_vdev(spectral);
	if (!vdev) {
		qdf_print("Spectral simulation: No VAPs found - not proceeding"
			  " with param config.\n");
		return 0;
	}

	bw = target_if_vdev_get_ch_width(vdev);

	switch (bw) {
	case CH_WIDTH_20MHZ:
		des_headreportset = simctx->bw20_headreportset;
		break;
	case CH_WIDTH_40MHZ:
		des_headreportset = simctx->bw40_headreportset;
		break;
	case CH_WIDTH_80MHZ:
		des_headreportset = simctx->bw80_headreportset;
		break;
	case CH_WIDTH_160MHZ:
		phymode = wlan_vdev_get_phymode(vdev);
		if (phymode == WLAN_PHYMODE_11AC_VHT160) {
			des_headreportset = simctx->bw160_headreportset;
		} else if (phymode == WLAN_PHYMODE_11AC_VHT80_80) {
			des_headreportset = simctx->bw80_80_headreportset;
		} else {
			qdf_print("Spectral simulation: Unexpected PHY mode %u"
				  " found for width 160 MHz...asserting.\n",
				  phymode);
			qdf_assert_always(0);
		}
		break;

	case IEEE80211_CWM_WIDTHINVALID:
		qdf_print("Spectral simulation: Invalid width configured - not"
			  " proceeding with param config.\n");
		is_invalid_width = true;
	default:
		qdf_print("Spectral simulation: Unknown width %u...asserting\n"
			  , bw);
		qdf_assert_always(0);
		break;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	if (is_invalid_width)
		return 0;

	if (!des_headreportset) {
		qdf_print("Spectral simulation: No simulation data present for"
			  " configured bandwidth/PHY mode - unable to proceed "
			  " with param config.\n");
	return 0;
	}

	simctx->curr_reportset = NULL;
	temp_reportset = des_headreportset;

	while (temp_reportset) {
	if (qdf_mem_cmp(&temp_reportset->config,
			params, sizeof(struct spectral_config)) == 0) {
		/* Found a matching config. We are done. */
		simctx->curr_reportset = temp_reportset;
		break;
	}

	temp_reportset = temp_reportset->next;
	}

	if (!simctx->curr_reportset) {
		qdf_print("Spectral simulation: No simulation data present for"
			  " desired Spectral configuration - unable to proceed"
			  " with param config.\n");
	return 0;
	}

	if (!simctx->curr_reportset->curr_report) {
		qdf_print("Spectral simulation: No report data instances "
			  "populated for desired Spectral configuration - "
			  "unable to proceed with param config\n");
		return 0;
	}

	return 1;
}
EXPORT_SYMBOL(tif_spectral_sim_configure_params);

u_int32_t tif_spectral_sim_get_params(
	void *arg,
	struct spectral_config *params)
{
	struct target_if_spectral *spectral = NULL;
	ath_spectralsim_context *simctx = NULL;

	qdf_assert_always(params);

	spectral = (struct target_if_spectral *)arg;
	qdf_assert_always(spectral);

	simctx = (ath_spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	if (!simctx->curr_reportset) {
	qdf_print("Spectral simulation: No configured reportset found.\n");
	return 0;
	}

	qdf_mem_copy(params, &simctx->curr_reportset->config, sizeof(*params));

	return 1;
}
EXPORT_SYMBOL(tif_spectral_sim_get_params);

#endif /* QCA_SUPPORT_SPECTRAL_SIMULATION */
