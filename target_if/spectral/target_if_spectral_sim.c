/*
 * Copyright (c) 2015,2017-2018 The Linux Foundation. All rights reserved.
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
#include <qdf_module.h>

/* Helper functions */

static int target_if_populate_report_static_gen2(
	struct spectralsim_report *report,
	enum phy_ch_width width, bool is_80_80);
static int target_if_populate_report_static_gen3(
	struct spectralsim_report *report,
	enum phy_ch_width width, bool is_80_80);
static void target_if_depopulate_report(
	struct spectralsim_report *report);

static int target_if_populate_reportset_static(
	struct spectralsim_context *simctx,
	struct spectralsim_reportset *reportset,
	enum phy_ch_width width, bool is_80_80);
static void target_if_depopulate_reportset(
	struct spectralsim_reportset *
	reportset);

static int target_if_populate_simdata(struct spectralsim_context *simctx);
static void target_if_depopulate_simdata(struct spectralsim_context *simctx);
static OS_TIMER_FUNC(target_if_spectral_sim_phyerrdelivery_handler);

/*
 * Static configuration.
 * For now, we will be having a single configuration per BW, and a single
 * report per configuration (since we need the data only for ensuring correct
 * format handling).
 *
 * Extend this for more functionality if required in the future.
 */

/**
 * target_if_populate_report_static_gen2() - Statically populate simulation
 * data for one report for generation 2 chipsets
 * @report: Pointer to spectral report data instance
 * @width : Channel bandwidth enumeration
 * @is_80_80: Whether the channel is operating in 80-80 mode
 *
 * Statically populate simulation data for one report for generation 2 chipsets
 *
 * Return: 0 on success, negative error code on failure
 */
static int
target_if_populate_report_static_gen2(
	struct spectralsim_report *report,
	enum phy_ch_width width, bool is_80_80)
{
	qdf_assert_always(report);

	switch (width) {
	case CH_WIDTH_20MHZ:
		report->data = NULL;
		report->data = (uint8_t *)
		    qdf_mem_malloc(sizeof(reportdata_20_gen2));

		if (!report->data) {
			spectral_err("Spectral simulation: Could not allocate memory for report data");
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
		report->data = (uint8_t *)
		    qdf_mem_malloc(sizeof(reportdata_40_gen2));

		if (!report->data) {
			spectral_err("Spectral simulation: Could not allocate memory for report data");
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
		report->data = (uint8_t *)
		    qdf_mem_malloc(sizeof(reportdata_80_gen2));

		if (!report->data) {
			spectral_err("Spectral simulation: Could not allocate memory for report data");
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
			report->data = (uint8_t *)
			    qdf_mem_malloc(sizeof(reportdata_80_80_gen2));

			if (!report->data) {
				spectral_err("Spectral simulation: Could not allocate memory for report data");
				goto bad;
			}

			report->datasize = sizeof(reportdata_80_80_gen2);
			qdf_mem_copy(report->data,
				     reportdata_80_80_gen2, report->datasize);

			qdf_mem_copy(&report->rfqual_info,
				     &rfqual_info_80_80,
				     sizeof(report->rfqual_info));

			qdf_mem_copy(&report->chan_info,
				     &chan_info_80_80,
				     sizeof(report->chan_info));

		} else {
			report->data = NULL;
			report->data = (uint8_t *)
			    qdf_mem_malloc(sizeof(reportdata_160_gen2));

			if (!report->data) {
				spectral_err("Spectral simulation: Could not allocate memory for report data");
				goto bad;
			}

			report->datasize = sizeof(reportdata_160_gen2);
			qdf_mem_copy(report->data,
				     reportdata_160_gen2, report->datasize);

			qdf_mem_copy(&report->rfqual_info,
				     &rfqual_info_160,
				     sizeof(report->rfqual_info));

			qdf_mem_copy(&report->chan_info,
				     &chan_info_160, sizeof(report->chan_info));
		}
		break;
	default:
		spectral_err("Unhandled width. Please correct. Asserting");
		qdf_assert_always(0);
	}

	return 0;

 bad:
	return -EPERM;
}

/**
 * target_if_populate_report_static_gen3() - Statically populate simulation
 * data for one report for generation 3 chipsets
 * @report: Pointer to spectral report data instance
 * @width : Channel bandwidth enumeration
 * @is_80_80: Whether the channel is operating in 80-80 mode
 *
 * Statically populate simulation data for one report for generation 3 chipsets
 *
 * Return: 0 on success, negative error code on failure
 */
static int
target_if_populate_report_static_gen3(
	struct spectralsim_report *report,
	enum phy_ch_width width, bool is_80_80)
{
	qdf_assert_always(report);

	switch (width) {
	case CH_WIDTH_20MHZ:
		report->data = NULL;
		report->data = (uint8_t *)
		    qdf_mem_malloc(sizeof(reportdata_20_gen3));

		if (!report->data) {
			spectral_err("Spectral simulation: Could not allocate memory for report data");
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
		report->data = (uint8_t *)
		    qdf_mem_malloc(sizeof(reportdata_40_gen3));

		if (!report->data) {
			spectral_err("Spectral simulation: Could not allocate memory for report data");
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
		report->data = (uint8_t *)
		    qdf_mem_malloc(sizeof(reportdata_80_gen3));

		if (!report->data) {
			spectral_err("Spectral simulation: Could not allocate memory for report data");
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
			report->data = (uint8_t *)
			    qdf_mem_malloc(sizeof(reportdata_80_80_gen3));

			if (!report->data) {
				spectral_err("Spectral simulation: Could not allocate memory for report data");
				goto bad;
			}

			report->datasize = sizeof(reportdata_80_80_gen3);
			qdf_mem_copy(report->data,
				     reportdata_80_80_gen3, report->datasize);

			qdf_mem_copy(&report->rfqual_info,
				     &rfqual_info_80_80,
				     sizeof(report->rfqual_info));

			qdf_mem_copy(&report->chan_info,
				     &chan_info_80_80,
				     sizeof(report->chan_info));

		} else {
			report->data = NULL;
			report->data = (uint8_t *)
			    qdf_mem_malloc(sizeof(reportdata_160_gen3));

			if (!report->data) {
				spectral_err("Spectral simulation: Could not allocate memory for report data");
				goto bad;
			}

			report->datasize = sizeof(reportdata_160_gen3);
			qdf_mem_copy(report->data,
				     reportdata_160_gen3, report->datasize);

			qdf_mem_copy(&report->rfqual_info,
				     &rfqual_info_160,
				     sizeof(report->rfqual_info));

			qdf_mem_copy(&report->chan_info,
				     &chan_info_160, sizeof(report->chan_info));
		}
		break;
	default:
		spectral_err("Unhandled width. Please correct. Asserting");
		qdf_assert_always(0);
	}

	return 0;

 bad:
	return -EPERM;
}

/**
 * target_if_depopulate_report() - Free the given instances of
 * struct spectralsim_report
 * @report: instance of struct spectralsim_report
 *
 * Free the given instances of struct spectralsim_report
 *
 * Return: None
 */
static void
target_if_depopulate_report(
	struct spectralsim_report *report)
{
	if (!report)
		return;

	if (report->data) {
		qdf_mem_free(report->data);
		report->data = NULL;
		report->datasize = 0;
	}
}

/**
 * target_if_populate_reportset_static() - Statically populate simulation data
 * for a given configuration
 * @simctx: Pointer to struct spectralsim_context
 * @reportset: Set of spectral report data instances
 * @width : Channel bandwidth enumeration
 * @is_80_80: Whether the channel is operating in 80+80 mode
 *
 * Statically populate simulation data for a given configuration
 *
 * Return: 0 on success, negative error code on failure
 */
static int
target_if_populate_reportset_static(
	struct spectralsim_context *simctx,
	struct spectralsim_reportset *reportset,
	enum phy_ch_width width, bool is_80_80)
{
	int ret = 0;
	struct spectralsim_report *report = NULL;

	qdf_assert_always(reportset);

	reportset->headreport = NULL;
	reportset->curr_report = NULL;

	/* For now, we populate only one report */
	report = (struct spectralsim_report *)
	    qdf_mem_malloc(sizeof(struct spectralsim_report));

	if (!report) {
		spectral_err("Spectral simulation: Could not allocate memory for report.");
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
				     &config_80_80_1,
				     sizeof(reportset->config));

			ret = simctx->populate_report_static(report,
							     CH_WIDTH_160MHZ,
							     1);
			if (ret != 0)
				goto bad;

			report->next = NULL;
			reportset->headreport = report;
		} else {
			qdf_mem_copy(&reportset->config,
				     &config_160_1, sizeof(reportset->config));

			ret = simctx->populate_report_static(report,
							     CH_WIDTH_160MHZ,
							     0);
			if (ret != 0)
				goto bad;

			report->next = NULL;
			reportset->headreport = report;
		}
		break;
	default:
		spectral_err("Unhandled width. Please rectify.");
		qdf_assert_always(0);
	};

	reportset->curr_report = reportset->headreport;

	return 0;

 bad:
	target_if_depopulate_reportset(reportset);
	return -EPERM;
}

/**
 * target_if_depopulate_reportset() - Free all the instances of
 * struct spectralsim_reportset
 * @report: head pointer to struct spectralsim_reportset linked list
 *
 * Free all the instances of struct spectralsim_reportset
 *
 * Return: None
 */
static void
target_if_depopulate_reportset(
	struct spectralsim_reportset *reportset)
{
	struct spectralsim_report *curr_report = NULL;
	struct spectralsim_report *next_report = NULL;

	if (!reportset)
		return;

	curr_report = reportset->headreport;

	while (curr_report) {
		next_report = curr_report->next;
		target_if_depopulate_report(curr_report);
		qdf_mem_free(curr_report);
		curr_report = next_report;
	}
}

/**
 * target_if_populate_simdata() - Populate simulation data
 * @simctx: Pointer to struct spectralsim_context
 *
 * Populate simulation data
 *
 * Return: 0 on success, negative error code on failure
 */
static int
target_if_populate_simdata(
	struct spectralsim_context *simctx)
{
	/*
	 * For now, we use static population. Switch to loading from a file if
	 * needed in the future.
	 */

	simctx->bw20_headreportset = NULL;
	SPECTRAL_SIM_REPORTSET_ALLOCPOPL_SINGLE(simctx,
						simctx->bw20_headreportset,
						CH_WIDTH_20MHZ, 0);

	simctx->bw40_headreportset = NULL;
	SPECTRAL_SIM_REPORTSET_ALLOCPOPL_SINGLE(simctx,
						simctx->bw40_headreportset,
						CH_WIDTH_40MHZ, 0);

	simctx->bw80_headreportset = NULL;
	SPECTRAL_SIM_REPORTSET_ALLOCPOPL_SINGLE(simctx,
						simctx->bw80_headreportset,
						CH_WIDTH_80MHZ, 0);

	simctx->bw160_headreportset = NULL;
	SPECTRAL_SIM_REPORTSET_ALLOCPOPL_SINGLE(simctx,
						simctx->bw160_headreportset,
						CH_WIDTH_160MHZ, 0);

	simctx->bw80_80_headreportset = NULL;
	SPECTRAL_SIM_REPORTSET_ALLOCPOPL_SINGLE(simctx,
						simctx->bw80_80_headreportset,
						CH_WIDTH_160MHZ, 1);

	simctx->curr_reportset = NULL;

	simctx->is_enabled = false;
	simctx->is_active = false;

	simctx->ssim_starting_tsf64 = 0;
	simctx->ssim_count = 0;
	simctx->ssim_period_ms = 0;

	return 0;
}

/**
 * target_if_depopulate_simdata() - De-populate simulation data
 * @simctx: Pointer to struct spectralsim_context
 *
 * De-populate simulation data
 *
 * Return: none
 */
static void
target_if_depopulate_simdata(
	struct spectralsim_context *simctx)
{
	if (!simctx)
		return;

	SPECTRAL_SIM_REPORTSET_DEPOPLFREE_LIST(simctx->bw20_headreportset);
	SPECTRAL_SIM_REPORTSET_DEPOPLFREE_LIST(simctx->bw40_headreportset);
	SPECTRAL_SIM_REPORTSET_DEPOPLFREE_LIST(simctx->bw80_headreportset);
	SPECTRAL_SIM_REPORTSET_DEPOPLFREE_LIST(simctx->bw160_headreportset);
	SPECTRAL_SIM_REPORTSET_DEPOPLFREE_LIST(simctx->bw80_80_headreportset);
}

/**
 * target_if_spectral_sim_phyerrdelivery_handler() - Phyerr delivery handler
 *
 * Return: none
 */
static
OS_TIMER_FUNC(target_if_spectral_sim_phyerrdelivery_handler)
{
	struct target_if_spectral *spectral = NULL;
	struct spectralsim_context *simctx = NULL;
	struct spectralsim_reportset *curr_reportset = NULL;
	struct spectralsim_report *curr_report = NULL;
	struct target_if_spectral_acs_stats acs_stats;
	uint64_t curr_tsf64 = 0;
	struct target_if_spectral_ops *p_sops;

	OS_GET_TIMER_ARG(spectral, struct target_if_spectral *);
	qdf_assert_always(spectral);

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	qdf_assert_always(spectral);

	simctx = (struct spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	if (!simctx->is_active)
		return;

	curr_reportset = simctx->curr_reportset;
	qdf_assert_always(curr_reportset);

	curr_report = curr_reportset->curr_report;
	qdf_assert_always(curr_report);

	qdf_assert_always(curr_reportset->headreport);

	/*
	 * We use a simulation TSF since in offload architectures we can't
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
					curr_tsf64, &acs_stats);

	simctx->ssim_count++;

	if (curr_report->next)
		curr_reportset->curr_report = curr_report->next;
	else
		curr_reportset->curr_report = curr_reportset->headreport;

	if (curr_reportset->config.ss_count != 0 &&
	    simctx->ssim_count == curr_reportset->config.ss_count) {
		target_if_spectral_sops_sim_stop_scan(spectral);
	} else {
		qdf_timer_start(&simctx->ssim_pherrdelivery_timer,
				simctx->ssim_period_ms);
	}
}

/* Module services */

int
target_if_spectral_sim_attach(struct target_if_spectral *spectral)
{
	struct spectralsim_context *simctx = NULL;

	qdf_assert_always(spectral);

	simctx = (struct spectralsim_context *)
	    qdf_mem_malloc(sizeof(struct spectralsim_context));

	if (!simctx) {
		spectral_err("Spectral simulation: Could not allocate memory for context");
		return -EPERM;
	}

	qdf_mem_zero(simctx, sizeof(*simctx));

	spectral->simctx = simctx;

	if (spectral->spectral_gen == SPECTRAL_GEN2)
		simctx->populate_report_static =
			target_if_populate_report_static_gen2;
	else if (spectral->spectral_gen == SPECTRAL_GEN3)
		simctx->populate_report_static =
			target_if_populate_report_static_gen3;

	if (target_if_populate_simdata(simctx) != 0) {
		qdf_mem_free(simctx);
		spectral->simctx = NULL;
		spectral_err("Spectral simulation attach failed");
		return -EPERM;
	}

	qdf_timer_init(NULL,
		       &simctx->ssim_pherrdelivery_timer,
		       target_if_spectral_sim_phyerrdelivery_handler,
		       (void *)(spectral), QDF_TIMER_TYPE_WAKE_APPS);

	spectral_info("Spectral simulation attached");

	return 0;
}

void
target_if_spectral_sim_detach(struct target_if_spectral *spectral)
{
	struct spectralsim_context *simctx = NULL;

	qdf_assert_always(spectral);

	simctx = (struct spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	qdf_timer_free(&simctx->ssim_pherrdelivery_timer);

	target_if_depopulate_simdata(simctx);
	qdf_mem_free(simctx);
	spectral->simctx = NULL;

	spectral_info("Spectral simulation detached");
}

uint32_t
target_if_spectral_sops_sim_is_active(void *arg)
{
	struct target_if_spectral *spectral = NULL;
	struct spectralsim_context *simctx = NULL;

	spectral = (struct target_if_spectral *)arg;
	qdf_assert_always(spectral);

	simctx = (struct spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	return simctx->is_active;
}
qdf_export_symbol(target_if_spectral_sops_sim_is_active);

uint32_t
target_if_spectral_sops_sim_is_enabled(void *arg)
{
	struct target_if_spectral *spectral = NULL;
	struct spectralsim_context *simctx = NULL;

	spectral = (struct target_if_spectral *)arg;
	qdf_assert_always(spectral);

	simctx = (struct spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	return simctx->is_enabled;
}
qdf_export_symbol(target_if_spectral_sops_sim_is_enabled);

uint32_t
target_if_spectral_sops_sim_start_scan(void *arg)
{
	struct target_if_spectral *spectral = NULL;
	struct spectralsim_context *simctx = NULL;

	spectral = (struct target_if_spectral *)arg;
	qdf_assert_always(spectral);

	simctx = (struct spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	if (!simctx->curr_reportset) {
		spectral_err("Spectral simulation: No current report set configured  - unable to start simulated Spectral scan");
		return 0;
	}

	if (!simctx->curr_reportset->curr_report) {
		spectral_err("Spectral simulation: No report data instances populated - unable to start simulated Spectral scan");
		return 0;
	}

	if (!simctx->is_enabled)
		simctx->is_enabled = true;

	simctx->is_active = true;

	/* Hardcoding current time as zero since it is simulation */
	simctx->ssim_starting_tsf64 = 0;
	simctx->ssim_count = 0;

	/*
	 * TODO: Support high resolution timer in microseconds if required, so
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
qdf_export_symbol(target_if_spectral_sops_sim_start_scan);

uint32_t
target_if_spectral_sops_sim_stop_scan(void *arg)
{
	struct target_if_spectral *spectral = NULL;
	struct spectralsim_context *simctx = NULL;

	spectral = (struct target_if_spectral *)arg;
	qdf_assert_always(spectral);

	simctx = (struct spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	qdf_timer_stop(&simctx->ssim_pherrdelivery_timer);

	simctx->is_active = false;
	simctx->is_enabled = false;

	simctx->ssim_starting_tsf64 = 0;
	simctx->ssim_count = 0;
	simctx->ssim_period_ms = 0;

	return 1;
}
qdf_export_symbol(target_if_spectral_sops_sim_stop_scan);

#ifdef SPECTRAL_SIM_DUMP_PARAM_DATA
static void
target_if_log_sim_spectral_params(struct spectral_config *params)
{
	int i = 0;

	spectral_debug("\n");

	spectral_debug("Spectral simulation: Param data dump:\nss_fft_period=%hu\nss_period=%hu\nss_count=%hu\nss_short_report=%hu\nradar_bin_thresh_sel=%hhu\nss_spectral_pri=%hu\nss_fft_size=%hu\nss_gc_ena=%hu\nss_restart_ena=%hu\nss_noise_floor_ref=%hu\nss_init_delay=%hu\nss_nb_tone_thr=%hu\nss_str_bin_thr=%hu\nss_wb_rpt_mode=%hu\nss_rssi_rpt_mode=%hu\nss_rssi_thr=%hu\nss_pwr_format=%hu\nss_rpt_mode=%hu\nss_bin_scale=%hu\nss_dbm_adj=%hu\nss_chn_mask=%hu\nss_nf_temp_data=%d",
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
		       params->ss_chn_mask, params->ss_nf_temp_data);

	for (i = 0; i < AH_MAX_CHAINS * 2; i++)
		spectral_debug("ss_nf_cal[%d]=%hhd", i, params->ss_nf_cal[i]);

	for (i = 0; i < AH_MAX_CHAINS * 2; i++)
		spectral_debug("ss_nf_pwr[%d]=%hhd", i, params->ss_nf_pwr[i]);

	spectral_info("\n");
}
#else

static void
target_if_log_sim_spectral_params(struct spectral_config *params)
{
}
#endif				/* SPECTRAL_SIM_DUMP_PARAM_DATA */

uint32_t
target_if_spectral_sops_sim_configure_params(
	void *arg,
	struct spectral_config *params)
{
	struct target_if_spectral *spectral = NULL;
	struct spectralsim_context *simctx = NULL;
	enum wlan_phymode phymode;
	uint8_t bw;
	struct spectralsim_reportset *des_headreportset = NULL;
	struct spectralsim_reportset *temp_reportset = NULL;
	bool is_invalid_width = false;
	struct wlan_objmgr_vdev *vdev = NULL;

	qdf_assert_always(params);
	target_if_log_sim_spectral_params(params);
	spectral = (struct target_if_spectral *)arg;
	qdf_assert_always(spectral);

	simctx = (struct spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	vdev = target_if_spectral_get_vdev(spectral);
	if (!vdev) {
		spectral_warn("Spectral simulation: No VAPs found - not proceeding with param config.");
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
			spectral_err("Spectral simulation: Unexpected PHY mode %u found for width 160 MHz...asserting.",
				     phymode);
			qdf_assert_always(0);
		}
		break;

	case IEEE80211_CWM_WIDTHINVALID:
		spectral_err("Spectral simulation: Invalid width configured - not proceeding with param config.");
		is_invalid_width = true;
	default:
		spectral_err("Spectral simulation: Unknown width %u...asserting",
			     bw);
		qdf_assert_always(0);
		break;
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	if (is_invalid_width)
		return 0;

	if (!des_headreportset) {
		spectral_warn("Spectral simulation: No simulation data present for configured bandwidth/PHY mode - unable to proceed  with param config.");
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
		spectral_warn("Spectral simulation: No simulation data present for desired Spectral configuration - unable to proceed with param config.");
		return 0;
	}

	if (!simctx->curr_reportset->curr_report) {
		spectral_warn("Spectral simulation: No report data instances populated for desired Spectral configuration - unable to proceed with param config");
		return 0;
	}

	return 1;
}
qdf_export_symbol(target_if_spectral_sops_sim_configure_params);

uint32_t
target_if_spectral_sops_sim_get_params(
	void *arg, struct spectral_config *params)
{
	struct target_if_spectral *spectral = NULL;
	struct spectralsim_context *simctx = NULL;

	qdf_assert_always(params);

	spectral = (struct target_if_spectral *)arg;
	qdf_assert_always(spectral);

	simctx = (struct spectralsim_context *)spectral->simctx;
	qdf_assert_always(simctx);

	if (!simctx->curr_reportset) {
		spectral_warn("Spectral simulation: No configured reportset found.");
		return 0;
	}

	qdf_mem_copy(params, &simctx->curr_reportset->config, sizeof(*params));

	return 1;
}
qdf_export_symbol(target_if_spectral_sops_sim_get_params);

#endif				/* QCA_SUPPORT_SPECTRAL_SIMULATION */
