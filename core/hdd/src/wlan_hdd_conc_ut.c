/*
 * Copyright (c) 2015-2021 The Linux Foundation. All rights reserved.
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

/* Include files */

#include <wlan_hdd_includes.h>
#include <cds_api.h>
#include <cds_sched.h>
#include <wni_api.h>
#include <wlan_hdd_cfg.h>
#include "wlan_hdd_trace.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_hdd_conc_ut.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include "cds_utils.h"
#include "wma_types.h"
#include "wma.h"
#include "wma_api.h"
#include "wlan_policy_mgr_ucfg.h"
#include "wlan_reg_services_api.h"

#define NUMBER_OF_SCENARIO 300
#define MAX_ALLOWED_CHAR_IN_REPORT 50

/**
 * struct report_t: Data structure to fill report
 *
 * @title: title of the concurrency case scenario
 * @first_persona: device type of first persona
 * @second_persona: device type of second persona
 * @third_persona: device type of third persona
 * @dbs_value: string to mention whether dbs enable or disable
 * @system_conf: string to mention what is system's configuration
 * @status: status field
 * @result_code: string to mention whether test case passed or failed
 * @reason: reason why test case failed
 * @pcl: preferred channel list
 *
 * This structure will be used by unit test framework to fill
 * report after running various concurrency scenarios.
 */
struct report_t {
	char title[2 * MAX_ALLOWED_CHAR_IN_REPORT];
	char first_persona[MAX_ALLOWED_CHAR_IN_REPORT];
	char second_persona[MAX_ALLOWED_CHAR_IN_REPORT];
	char third_persona[MAX_ALLOWED_CHAR_IN_REPORT];
	char dbs_value[MAX_ALLOWED_CHAR_IN_REPORT];
	char system_conf[MAX_ALLOWED_CHAR_IN_REPORT];
	bool status;
	char result_code[MAX_ALLOWED_CHAR_IN_REPORT];
	char reason[MAX_ALLOWED_CHAR_IN_REPORT];
	char pcl_freqs[6 * NUM_CHANNELS + 16];
};

static struct report_t report[NUMBER_OF_SCENARIO];
static uint32_t report_idx;
static const char *system_config_to_string(uint8_t idx)
{
	switch (idx) {
	CASE_RETURN_STRING(PM_THROUGHPUT);
	CASE_RETURN_STRING(PM_POWERSAVE);
	CASE_RETURN_STRING(PM_LATENCY);
	default:
		return "Unknown";
	}

}

void clean_report(struct hdd_context *hdd_ctx)
{
	uint32_t idx = 0;

	while (idx < NUMBER_OF_SCENARIO) {
		qdf_mem_zero(&report[idx], sizeof(struct report_t));
		idx++;
	}
	report_idx = 0;
}

void print_report(struct hdd_context *hdd_ctx)
{
	uint32_t idx = 0;

	pr_info("+----------Report start -----------+\n");
	while (idx < report_idx) {
		pr_info("Idx:[%d] Title:%s Result:[%s] 1st_person[%s] 2nd_persona[%s] 3rd_persona[%s] DBS[%s] system_config[%s] reason[%s] pcl_freqs[%s]\n",
			idx,
			report[idx].title, report[idx].result_code,
			report[idx].first_persona, report[idx].second_persona,
			report[idx].third_persona, report[idx].dbs_value,
			report[idx].system_conf, report[idx].reason,
			report[idx].pcl_freqs);
		idx++;
	}
	pr_info("+----------Report end -----------+\n");
}

void fill_report(struct hdd_context *hdd_ctx, char *title,
	uint32_t first_persona, uint32_t second_persona, uint32_t third_persona,
	qdf_freq_t chnl_1st_conn, qdf_freq_t chnl_2nd_conn,
	qdf_freq_t chnl_3rd_conn, bool status,
	enum policy_mgr_pcl_type pcl_type, char *reason,
	uint32_t *pcl_freqs, uint32_t pcl_len)
{
	int i;
	char buf[5] = {0};
	uint8_t sys_pref = 0;

	if (report_idx >= NUMBER_OF_SCENARIO) {
		pr_info("report buffer overflow %d", report_idx);
		return;
	}

	ucfg_policy_mgr_get_sys_pref(hdd_ctx->psoc, &sys_pref);

	snprintf(report[report_idx].title,
		2 * MAX_ALLOWED_CHAR_IN_REPORT, "pcl for[%s] pcl_type[%s]",
		title, pcl_type_to_string(pcl_type));
	if (chnl_1st_conn == 0)
		snprintf(report[report_idx].first_persona,
			MAX_ALLOWED_CHAR_IN_REPORT, "%s",
			device_mode_to_string(first_persona));
	else
		snprintf(report[report_idx].first_persona,
			MAX_ALLOWED_CHAR_IN_REPORT,
			"%s-chnl{%d}",
			device_mode_to_string(first_persona), chnl_1st_conn);
	if (chnl_2nd_conn == 0)
		snprintf(report[report_idx].second_persona,
			MAX_ALLOWED_CHAR_IN_REPORT, "%s",
			device_mode_to_string(second_persona));
	else
		snprintf(report[report_idx].second_persona,
			MAX_ALLOWED_CHAR_IN_REPORT,
			"%s-chnl{%d}",
			device_mode_to_string(second_persona), chnl_2nd_conn);
	if (chnl_3rd_conn == 0)
		snprintf(report[report_idx].third_persona,
			MAX_ALLOWED_CHAR_IN_REPORT, "%s",
			device_mode_to_string(third_persona));
	else
		snprintf(report[report_idx].third_persona,
			MAX_ALLOWED_CHAR_IN_REPORT,
			"%s-chnl{%d}",
			device_mode_to_string(third_persona), chnl_3rd_conn);

	report[report_idx].status = status;
	snprintf(report[report_idx].dbs_value,
		MAX_ALLOWED_CHAR_IN_REPORT, "%s",
		policy_mgr_is_hw_dbs_capable(hdd_ctx->psoc)
		? "enable" : "disable");
	snprintf(report[report_idx].system_conf,
		MAX_ALLOWED_CHAR_IN_REPORT, "%s",
		system_config_to_string(sys_pref));
	snprintf(report[report_idx].result_code,
		MAX_ALLOWED_CHAR_IN_REPORT, "%s",
		status ? "PASS" : "FAIL");
	snprintf(report[report_idx].reason,
		MAX_ALLOWED_CHAR_IN_REPORT,
		reason);
	if (pcl_freqs) {
		qdf_mem_zero(report[report_idx].pcl_freqs,
				sizeof(report[report_idx].pcl_freqs));
		snprintf(buf, sizeof(buf), "pcl len %d :", pcl_len);
		strlcat(report[report_idx].pcl_freqs, buf,
			sizeof(report[report_idx].pcl_freqs));
		for (i = 0; i < pcl_len && i < NUM_CHANNELS; i++) {
			qdf_mem_zero(buf, sizeof(buf));
			snprintf(buf, sizeof(buf), "%d ", pcl_freqs[i]);
			strlcat(report[report_idx].pcl_freqs, buf,
				sizeof(report[report_idx].pcl_freqs));
			strlcat(report[report_idx].pcl_freqs, ", ",
				sizeof(report[report_idx].pcl_freqs));
		}
	}
	report_idx++;
}

static void _validate_24g(bool *status, uint32_t *first_idx,
			  uint32_t *pcl_freqs, uint32_t pcl_len,
			  qdf_freq_t first_connection_chnl,
			  qdf_freq_t second_connection_chnl,
			  char *reason, uint32_t reason_length)
{
	bool found2g = false;

	if (!*status)
		return;
	for (; *first_idx < pcl_len; (*first_idx)++) {
		if (WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[(*first_idx)]))
			found2g = true;
		else
			break;
	}
	if (!found2g) {
		snprintf(reason, reason_length,
			 "Not 2g ch list");
		*status = false;
	}
}

static void _validate_5g(bool *status, uint32_t *first_idx,
			 uint32_t *pcl_freqs, uint32_t pcl_len,
			 qdf_freq_t first_connection_chnl,
			 qdf_freq_t second_connection_chnl,
			 char *reason, uint32_t reason_length)
{
	bool found5g = false;

	if (!*status)
		return;
	for (; *first_idx < pcl_len; (*first_idx)++) {
		if (WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[*first_idx]) ||
		    WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[*first_idx])) {
			found5g = true;
		} else {
			break;
		}
	}
	if (!found5g) {
		snprintf(reason, reason_length,
			 "Not 5/6g ch list");
		*status = false;
	}
}

static void _validate_scc(bool *status, uint32_t *first_idx,
			  uint32_t *pcl_freqs, uint32_t pcl_len,
			  qdf_freq_t first_connection_chnl,
			  qdf_freq_t second_connection_chnl,
			  char *reason, uint32_t reason_length)
{
	if (!*status)
		return;

	if (!first_connection_chnl) {
		snprintf(reason, reason_length,
			 "scc ch invalid %d",
			 first_connection_chnl);
		*status = false;
		return;
	}
	if (*first_idx >= pcl_len) {
		snprintf(reason, reason_length,
			 "no scc ch");
		*status = false;
		return;
	}
	if (pcl_freqs[*first_idx] != first_connection_chnl &&
	    !(second_connection_chnl &&
	      pcl_freqs[*first_idx] == second_connection_chnl)) {
		snprintf(reason, reason_length,
			 "1st scc ch is not correct %d expect %d %d",
			 pcl_freqs[*first_idx],
			 first_connection_chnl,
			 second_connection_chnl);
		*status = false;
		return;
	}
	(*first_idx)++;
	if (second_connection_chnl) {
		if (*first_idx >= pcl_len) {
			snprintf(reason, reason_length,
				 "no 2rd scc ch");
			*status = false;
			return;
		}
		if (pcl_freqs[*first_idx] != first_connection_chnl &&
		    pcl_freqs[*first_idx] != second_connection_chnl) {
			snprintf(reason, reason_length,
				 "2rd scc ch is not correct %d expect %d %d",
				 pcl_freqs[*first_idx],
				 first_connection_chnl,
				 second_connection_chnl);
			*status = false;
			return;
		}
		(*first_idx)++;
	}
}

static void _validate_mcc(bool *status, uint32_t *first_idx,
			  uint32_t *pcl_freqs, uint32_t pcl_len,
			  qdf_freq_t first_connection_chnl,
			  qdf_freq_t second_connection_chnl,
			  char *reason, uint32_t reason_length)
{
	if (!*status)
		return;

	if (!first_connection_chnl || !second_connection_chnl ||
	    first_connection_chnl == second_connection_chnl) {
		snprintf(reason, reason_length,
			 "mcc ch invalid %d %d",
			 first_connection_chnl,
			 second_connection_chnl);
		*status = false;
		return;
	}
	if (*first_idx >= pcl_len) {
		snprintf(reason, reason_length,
			 "no mcc ch");
		*status = false;
		return;
	}
	if (pcl_freqs[*first_idx] != first_connection_chnl &&
	    pcl_freqs[*first_idx] != second_connection_chnl) {
		snprintf(reason, reason_length,
			 "mcc ch is not correct %d",
			 pcl_freqs[*first_idx]);
		*status = false;
		return;
	}
	(*first_idx)++;
	if (pcl_freqs[*first_idx] != first_connection_chnl &&
	    pcl_freqs[*first_idx] != second_connection_chnl) {
		snprintf(reason, reason_length,
			 "mcc ch is not correct %d",
			 pcl_freqs[*first_idx]);
		*status = false;
		return;
	}
	(*first_idx)++;
}

static void _validate_sbs(struct wlan_objmgr_psoc *psoc,
			  bool *status, uint32_t *first_idx,
			  uint32_t *pcl_freqs, uint32_t pcl_len,
			  qdf_freq_t first_connection_chnl,
			  qdf_freq_t second_connection_chnl,
			  char *reason, uint32_t reason_length)
{
	bool found_sbs = false;
	uint32_t non_sbs_freq = 0;

	if (!*status)
		return;

	if (*first_idx >= pcl_len) {
		snprintf(reason, reason_length,
			 "no sbs ch list");
		*status = false;
		return;
	}
	for (; *first_idx < pcl_len; (*first_idx)++) {
		if (policy_mgr_are_sbs_chan(psoc, pcl_freqs[*first_idx],
					    first_connection_chnl)) {
			found_sbs = true;
		} else {
			non_sbs_freq = pcl_freqs[*first_idx];
			break;
		}
	}
	if (!found_sbs) {
		snprintf(reason, reason_length,
			 "not sbs ch list %d",
			 non_sbs_freq);
		*status = false;
		return;
	}
}

static void _validate_end(bool *status, uint32_t *first_idx,
			  uint32_t *pcl_freqs, uint32_t pcl_len,
			  qdf_freq_t first_connection_chnl,
			  qdf_freq_t second_connection_chnl,
			  char *reason, uint32_t reason_length)
{
	if (!*status)
		return;

	if (*first_idx < pcl_len) {
		snprintf(reason, reason_length,
			 "unexpected ch in pcl");
		*status = false;
		return;
	}
}

#define validate_24g _validate_24g(					\
			&status, &first_idx, pcl_freqs, pcl_len,	\
			first_connection_chnl, second_connection_chnl,	\
			reason, reason_length)

#define validate_5g _validate_5g(					\
			&status, &first_idx, pcl_freqs, pcl_len,	\
			first_connection_chnl, second_connection_chnl,	\
			reason, reason_length)

#define validate_scc _validate_scc(					\
			&status, &first_idx, pcl_freqs, pcl_len,	\
			first_connection_chnl, second_connection_chnl,	\
			reason, reason_length)

#define validate_mcc _validate_mcc(					\
			&status, &first_idx, pcl_freqs, pcl_len,	\
			first_connection_chnl, second_connection_chnl,	\
			reason, reason_length)

#define validate_sbs _validate_sbs(					\
			psoc, &status, &first_idx, pcl_freqs, pcl_len,	\
			first_connection_chnl, second_connection_chnl,	\
			reason, reason_length)

#define validate_end _validate_end(					\
			&status, &first_idx, pcl_freqs, pcl_len,	\
			first_connection_chnl, second_connection_chnl,	\
			reason, reason_length)

static bool wlan_hdd_validate_pcl(struct hdd_context *hdd_ctx,
	enum policy_mgr_pcl_type pcl_type, uint32_t *pcl_freqs,
	uint32_t pcl_len, qdf_freq_t first_connection_chnl,
	qdf_freq_t second_connection_chnl, char *reason,
	uint32_t reason_length)
{
	bool status = true;
	uint32_t first_idx = 0;
	struct wlan_objmgr_psoc *psoc = hdd_ctx->psoc;

	if ((pcl_type != PM_NONE) && (pcl_len == 0)) {
		snprintf(reason, reason_length, "no of channels = 0");
		return false;
	}

	switch (pcl_type) {
	case PM_NONE:
		if (pcl_len != 0) {
			snprintf(reason, reason_length, "no of channels>0");
			return false;
		}
		break;
	case PM_5G:
		for (first_idx = 0; first_idx < pcl_len; first_idx++) {
			if (!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[first_idx]) &&
			    !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[first_idx])) {
				snprintf(reason, reason_length,
					"2G channel found");
				return false;
			}
		}
		break;
	case PM_24G:
		for (first_idx = 0; first_idx < pcl_len; first_idx++) {
			if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[first_idx])) {
				snprintf(reason, reason_length,
					"5G channel found");
				return false;
			}
		}
		break;
	case PM_SCC_CH:
		if (second_connection_chnl > 0 &&
			(first_connection_chnl != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"invalid connections");
			return false;
		}
		if (pcl_freqs[0] != first_connection_chnl) {
			snprintf(reason, reason_length,
				"No SCC found");
			return false;
		}
		break;
	case PM_MCC_CH:
		if ((pcl_freqs[0] != first_connection_chnl) &&
				((second_connection_chnl > 0) &&
				 (pcl_freqs[0] != second_connection_chnl))) {
			snprintf(reason, reason_length,
				"MCC invalid");
			return false;
		}
		if ((second_connection_chnl > 0) &&
				(pcl_freqs[1] != first_connection_chnl &&
				 pcl_freqs[1] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"MCC invalid");
			return false;
		}
		break;
	case PM_SCC_CH_24G:
		if (second_connection_chnl > 0 &&
			(first_connection_chnl != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"invalid connections");
			return false;
		}
		if (pcl_freqs[0] != first_connection_chnl) {
			snprintf(reason, reason_length,
				"No SCC found");
			return false;
		}
		if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[pcl_len - 1])) {
			snprintf(reason, reason_length,
				"No 2.4Ghz chnl");
			return false;
		}
		break;
	case PM_SCC_CH_5G:
		if (second_connection_chnl > 0 &&
			(first_connection_chnl != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"invalid connections");
			return false;
		}
		if (pcl_freqs[0] != first_connection_chnl) {
			snprintf(reason, reason_length,
				"No SCC found");
			return false;
		}
		if (!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[pcl_len - 1]) &&
		    !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[pcl_len - 1])) {
			snprintf(reason, reason_length,
				"No 5Ghz chnl");
			return false;
		}
		break;
	case PM_24G_SCC_CH:
		if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[0])) {
			snprintf(reason, reason_length,
				"No 2.4Ghz chnl");
			return false;
		}
		if (second_connection_chnl > 0 &&
			(first_connection_chnl != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"invalid connections");
			return false;
		}
		if (pcl_freqs[pcl_len-1] != first_connection_chnl) {
			snprintf(reason, reason_length,
				"No SCC found");
			return false;
		}
		break;
	case PM_5G_SCC_CH:
		if (!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[0]) &&
		    !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[0])) {
			snprintf(reason, reason_length,
				"No 5Ghz chnl");
			return false;
		}
		if (second_connection_chnl > 0 &&
			(first_connection_chnl != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"invalid connections");
			return false;
		}
		if (pcl_freqs[pcl_len-1] != first_connection_chnl) {
			snprintf(reason, reason_length,
				"No SCC found");
			return false;
		}
		break;
	case PM_MCC_CH_24G:
		if ((pcl_freqs[0] != first_connection_chnl) &&
			((second_connection_chnl > 0) &&
			 (pcl_freqs[0] != second_connection_chnl))) {
			snprintf(reason, reason_length,
				"MCC invalid");
			return false;
		}
		if ((second_connection_chnl > 0) &&
			(pcl_freqs[1] != first_connection_chnl &&
			 pcl_freqs[1] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"MCC invalid");
			return false;
		}
		if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[pcl_len - 1])) {
			snprintf(reason, reason_length,
				"No 24Ghz chnl");
			return false;
		}
		break;
	case PM_MCC_CH_5G:
		if ((pcl_freqs[0] != first_connection_chnl) &&
			((second_connection_chnl > 0) &&
			 (pcl_freqs[0] != second_connection_chnl))) {
			snprintf(reason, reason_length,
				"MCC invalid");
			return false;
		}
		if ((second_connection_chnl > 0) &&
			(pcl_freqs[1] != first_connection_chnl &&
			 pcl_freqs[1] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"MCC invalid");
			return false;
		}
		if (!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[pcl_len - 1]) &&
		    !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[pcl_len - 1])) {
			snprintf(reason, reason_length,
				"No 5Ghz chnl");
			return false;
		}
		break;
	case PM_24G_MCC_CH:
		if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[0])) {
			snprintf(reason, reason_length,
				"No 24Ghz chnl");
			return false;
		}
		if ((pcl_freqs[pcl_len-1] != first_connection_chnl) &&
			((second_connection_chnl > 0) &&
			 (pcl_freqs[pcl_len-1] != second_connection_chnl))) {
			snprintf(reason, reason_length,
				"MCC invalid");
			return false;
		}
		if ((second_connection_chnl > 0) &&
			(pcl_freqs[pcl_len-2] != first_connection_chnl &&
			 pcl_freqs[pcl_len-2] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"MCC invalid");
			return false;
		}
		break;
	case PM_5G_MCC_CH:
		if (!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[0]) &&
		    !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[0])) {
			snprintf(reason, reason_length,
				"No 5Ghz chnl");
			return false;
		}
		if ((pcl_freqs[pcl_len-1] != first_connection_chnl) &&
			((second_connection_chnl > 0) &&
			 (pcl_freqs[pcl_len-1] != second_connection_chnl))) {
			snprintf(reason, reason_length,
				"MCC invalid");
			return false;
		}
		if ((second_connection_chnl > 0) &&
			(pcl_freqs[pcl_len-2] != first_connection_chnl &&
			 pcl_freqs[pcl_len-2] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"MCC invalid");
			return false;
		}
		break;
	case PM_SCC_ON_5_SCC_ON_24_24G:
		if ((!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[0]) &&
		     !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[0])) ||
			(pcl_freqs[0] != first_connection_chnl &&
			 pcl_freqs[0] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"No 5Ghz chnl/scc");
			return false;
		}
		if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[1]) ||
			(pcl_freqs[1] != first_connection_chnl &&
			 pcl_freqs[1] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"No 24Ghz chnl/scc");
			return false;
		}
		if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[pcl_len - 1])) {
			snprintf(reason, reason_length,
				"No 24Ghz chnls");
			return false;
		}
		break;
	case PM_SCC_ON_5_SCC_ON_24_5G:
		if ((!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[0]) &&
		     !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[0])) ||
			(pcl_freqs[0] != first_connection_chnl &&
			 pcl_freqs[0] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"No 5Ghz chnl/scc");
			return false;
		}
		if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[1]) ||
			(pcl_freqs[1] != first_connection_chnl &&
			 pcl_freqs[1] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"No 24Ghz chnl/scc");
			return false;
		}
		if (!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[pcl_len - 1]) &&
		    !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[pcl_len - 1])) {
			snprintf(reason, reason_length,
				"No 5Ghz chnls");
			return false;
		}
		break;
	case PM_SCC_ON_24_SCC_ON_5_24G:
		if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[0]) ||
			(pcl_freqs[0] != first_connection_chnl &&
			 pcl_freqs[0] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"No 24Ghz chnl/scc");
			return false;
		}
		if ((!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[1]) &&
		     !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[1])) ||
			(pcl_freqs[1] != first_connection_chnl &&
			 pcl_freqs[1] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"No 5Ghz chnl/scc");
			return false;
		}
		if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[pcl_len - 1])) {
			snprintf(reason, reason_length,
				"No 24Ghz chnls");
			return false;
		}
		break;
	case PM_SCC_ON_24_SCC_ON_5_5G:
		if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[0]) ||
			(pcl_freqs[0] != first_connection_chnl &&
			 pcl_freqs[0] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"No 24Ghz chnl/scc");
			return false;
		}
		if ((!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[1]) &&
		     !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[1])) ||
			(pcl_freqs[1] != first_connection_chnl &&
			 pcl_freqs[1] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"No 5Ghz chnl/scc");
			return false;
		}
		if (!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[pcl_len - 1]) &&
		    !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[pcl_len - 1])) {
			snprintf(reason, reason_length,
				"No 5Ghz chnls");
			return false;
		}
		break;
	case PM_SCC_ON_5_SCC_ON_24:
		if ((!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[0]) &&
		     !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[0])) ||
			(pcl_freqs[0] != first_connection_chnl &&
			 pcl_freqs[0] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"No 5Ghz chnl/scc");
			return false;
		}
		if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[1]) ||
			(pcl_freqs[1] != first_connection_chnl &&
			 pcl_freqs[1] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"No 24Ghz chnl/scc");
			return false;
		}
		if (pcl_len != 2) {
			snprintf(reason, reason_length,
				"more than 2 chnls");
			return false;
		}
		break;
	case PM_SCC_ON_24_SCC_ON_5:
		if (!WLAN_REG_IS_24GHZ_CH_FREQ(pcl_freqs[0]) ||
			(pcl_freqs[0] != first_connection_chnl &&
			 pcl_freqs[0] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"No 24Ghz chnl/scc");
			return false;
		}
		if ((!WLAN_REG_IS_5GHZ_CH_FREQ(pcl_freqs[1]) &&
		     !WLAN_REG_IS_6GHZ_CHAN_FREQ(pcl_freqs[1])) ||
			(pcl_freqs[1] != first_connection_chnl &&
			 pcl_freqs[1] != second_connection_chnl)) {
			snprintf(reason, reason_length,
				"No 5Ghz chnl/scc");
			return false;
		}
		if (pcl_len != 2) {
			snprintf(reason, reason_length,
				"more than 2 chnls");
			return false;
		}
		break;
	case PM_SBS_CH:
		validate_scc;
		validate_end;
		break;
	case PM_SBS_CH_5G:
		validate_scc;
		validate_5g;
		validate_end;
		break;
	case PM_24G_SCC_CH_SBS_CH:
		validate_24g;
		validate_scc;
		validate_sbs;
		validate_end;
		break;
	case PM_24G_SCC_CH_SBS_CH_5G:
		validate_24g;
		validate_scc;
		validate_sbs;
		validate_5g;
		validate_end;
		break;
	case PM_24G_SBS_CH_MCC_CH:
		validate_24g;
		validate_sbs;
		validate_mcc;
		validate_end;
		break;
	case PM_SBS_CH_24G_SCC_CH:
		validate_sbs;
		validate_24g;
		validate_scc;
		validate_end;
		break;
	case PM_SBS_CH_SCC_CH_24G:
		validate_sbs;
		validate_scc;
		validate_24g;
		validate_end;
		break;
	case PM_SCC_CH_SBS_CH_24G:
		validate_scc;
		validate_sbs;
		validate_24g;
		validate_end;
		break;
	case PM_SBS_CH_SCC_CH_5G_24G:
		validate_sbs;
		validate_scc;
		validate_5g;
		validate_24g;
		validate_end;
		break;
	case PM_SCC_CH_MCC_CH_SBS_CH_24G:
		validate_scc;
		validate_5g;
		validate_24g;
		validate_end;
		break;
	default:
		snprintf(reason, reason_length,
			"Unknown option");
		status = false;
	}
	if (status == true) {
		snprintf(reason, reason_length,
			"success");
	}
	return status;
}

void wlan_hdd_one_connection_scenario(struct hdd_context *hdd_ctx)
{
	enum policy_mgr_con_mode sub_type;
	uint8_t weight_list[NUM_CHANNELS] = {0};
	uint32_t pcl_len = 0;
	uint32_t pcl_freqs[NUM_CHANNELS] = {0};
	bool status = false;
	enum policy_mgr_pcl_type pcl_type;
	char reason[20] = {0};
	QDF_STATUS ret;
	struct policy_mgr_sme_cbacks sme_cbacks;
	uint8_t system_pref = 0;

	ucfg_policy_mgr_get_sys_pref(hdd_ctx->psoc, &system_pref);

	sme_cbacks.sme_get_nss_for_vdev = sme_get_vdev_type_nss;
	/* flush the entire table first */
	policy_mgr_psoc_disable(hdd_ctx->psoc);
	ret = policy_mgr_psoc_enable(hdd_ctx->psoc);
	if (!QDF_IS_STATUS_SUCCESS(ret)) {
		hdd_err("Policy manager initialization failed");
		return;
	}

	for (sub_type = 0; sub_type < PM_MAX_NUM_OF_MODE; sub_type++) {
		/* validate one connection is created or no */
		if (policy_mgr_get_connection_count(hdd_ctx->psoc) != 0) {
			hdd_err("Test failed - No. of connection is not 0");
			return;
		}
		pcl_len = 0;
		pcl_type = policy_mgr_get_pcl_from_first_conn_table(
			sub_type, system_pref);

		/* check PCL value for second connection is correct or no */
		policy_mgr_get_pcl(hdd_ctx->psoc, sub_type, pcl_freqs, &pcl_len,
				   weight_list, QDF_ARRAY_SIZE(weight_list));

		status = wlan_hdd_validate_pcl(hdd_ctx,
				pcl_type, pcl_freqs, pcl_len, 0, 0,
				reason, sizeof(reason));
		if ((pcl_type == PM_MAX_PCL_TYPE) && (pcl_freqs[0] == 0))
			continue;

		fill_report(hdd_ctx, "1 connection", sub_type,
				PM_MAX_NUM_OF_MODE,
				PM_MAX_NUM_OF_MODE,
				0, 0, 0,
				status, pcl_type, reason, pcl_freqs,
				pcl_len);
	}
}

void wlan_hdd_two_connections_scenario(
		struct hdd_context *hdd_ctx,
		qdf_freq_t first_chnl,
		enum policy_mgr_chain_mode first_chain_mask)
{
	uint8_t vdevid = 0, tx_stream = 2, rx_stream = 2;
	uint8_t mac_id = 1;
	uint8_t weight_list[NUM_CHANNELS] = {0};
	uint32_t pcl_len = 0;
	uint32_t pcl_freqs[NUM_CHANNELS];
	enum policy_mgr_chain_mode chain_mask = first_chain_mask;
	enum policy_mgr_con_mode sub_type, next_sub_type;
	enum policy_mgr_pcl_type pcl_type;
	enum policy_mgr_one_connection_mode second_index;
	char reason[20] = {0};
	bool status = false;
	QDF_STATUS ret;
	uint8_t system_pref = 0;

	ucfg_policy_mgr_get_sys_pref(hdd_ctx->psoc, &system_pref);

	for (sub_type = PM_STA_MODE;
		sub_type < PM_MAX_NUM_OF_MODE; sub_type++) {

		/* flush the entire table first */
		policy_mgr_psoc_disable(hdd_ctx->psoc);
		ret = policy_mgr_psoc_enable(hdd_ctx->psoc);
		if (!QDF_IS_STATUS_SUCCESS(ret)) {
			hdd_err("Policy manager initialization failed");
			return;
		}

		/* add first connection as STA */
		policy_mgr_incr_connection_count_utfw(
			hdd_ctx->psoc, vdevid, tx_stream,
			rx_stream, chain_mask, sub_type,
			first_chnl, mac_id);
		/* validate one connection is created or no */
		if (policy_mgr_get_connection_count(hdd_ctx->psoc) != 1) {
			hdd_err("Test failed - No. of connection is not 1");
			return;
		}
		next_sub_type = PM_STA_MODE;
		while (next_sub_type < PM_MAX_NUM_OF_MODE) {
			/* get the PCL value & check the channels accordingly */
			second_index =
			policy_mgr_get_second_connection_pcl_table_index(
				hdd_ctx->psoc);
			if (PM_MAX_ONE_CONNECTION_MODE == second_index) {
				/* not valid combination*/
				hdd_err("couldn't find index for 2nd connection pcl table");
				next_sub_type++;
				continue;
			}
			pcl_len = 0;
			pcl_type = policy_mgr_get_pcl_from_second_conn_table(
				second_index, next_sub_type, system_pref,
				policy_mgr_is_hw_dbs_capable(
					hdd_ctx->psoc));
			/* check PCL for second connection is correct or no */
			policy_mgr_get_pcl(hdd_ctx->psoc,
					   next_sub_type, pcl_freqs, &pcl_len,
					   weight_list,
					   QDF_ARRAY_SIZE(weight_list));
			status = wlan_hdd_validate_pcl(hdd_ctx,
					pcl_type, pcl_freqs, pcl_len,
					first_chnl, 0,
					reason, sizeof(reason));
			if ((pcl_type == PM_MAX_PCL_TYPE) && (pcl_freqs[0] == 0)) {
				next_sub_type++;
				continue;
			}
			fill_report(hdd_ctx, "2 connections", sub_type,
					next_sub_type,
					PM_MAX_NUM_OF_MODE, first_chnl,
					0, 0, status, pcl_type, reason,
					pcl_freqs, pcl_len);
			next_sub_type++;
		}
		policy_mgr_decr_connection_count_utfw(
				hdd_ctx->psoc, false, vdevid);
	}
}

void wlan_hdd_three_connections_scenario(struct hdd_context *hdd_ctx,
		qdf_freq_t first_chnl, qdf_freq_t second_chnl,
		enum policy_mgr_chain_mode chain_mask, uint8_t use_same_mac)
{
	uint8_t vdevid_1 = 0, tx_stream_1 = 2, rx_stream_1 = 2;
	uint8_t vdevid_2 = 1, tx_stream_2 = 2, rx_stream_2 = 2;
	uint8_t mac_id_1, mac_id_2;
	uint8_t weight_list[NUM_CHANNELS] = {0};
	uint32_t pcl_len = 0;
	uint32_t pcl_freqs[NUM_CHANNELS];
	enum policy_mgr_chain_mode chain_mask_1;
	enum policy_mgr_chain_mode chain_mask_2;
	enum policy_mgr_con_mode sub_type_1, sub_type_2, next_sub_type;
	enum policy_mgr_pcl_type pcl_type;
	enum policy_mgr_two_connection_mode third_index;
	char reason[20] = {0};
	bool status = false;
	QDF_STATUS ret;
	uint8_t system_pref = 0;

	ucfg_policy_mgr_get_sys_pref(hdd_ctx->psoc, &system_pref);

	/* let's set the chain_mask, mac_ids*/
	if (chain_mask == POLICY_MGR_TWO_TWO) {
		if (use_same_mac) {
			mac_id_1 = 1;
			mac_id_2 = 1;
		} else {
			mac_id_1 = 1;
			mac_id_2 = 2;
		}
		chain_mask_1 = POLICY_MGR_TWO_TWO;
		chain_mask_2 = POLICY_MGR_TWO_TWO;
	} else if (use_same_mac == 1) {
		mac_id_1 = 1;
		mac_id_2 = 1;
		chain_mask_1 = POLICY_MGR_ONE_ONE;
		chain_mask_2 = POLICY_MGR_ONE_ONE;
	} else {
		mac_id_1 = 1;
		mac_id_2 = 2;
		chain_mask_1 = POLICY_MGR_ONE_ONE;
		chain_mask_2 = POLICY_MGR_ONE_ONE;
	}

	for (sub_type_1 = PM_STA_MODE;
		sub_type_1 < PM_MAX_NUM_OF_MODE; sub_type_1++) {

		/* flush the entire table first */
		policy_mgr_psoc_disable(hdd_ctx->psoc);
		ret = policy_mgr_psoc_enable(hdd_ctx->psoc);
		if (!QDF_IS_STATUS_SUCCESS(ret)) {
			hdd_err("Policy manager initialization failed");
			return;
		}

		/* add first connection as STA */
		policy_mgr_incr_connection_count_utfw(
			hdd_ctx->psoc, vdevid_1, tx_stream_1, rx_stream_1,
			chain_mask_1, sub_type_1,
			first_chnl, mac_id_1);
		/* validate one connection is created or no */
		if (policy_mgr_get_connection_count(hdd_ctx->psoc) != 1) {
			hdd_err("Test fail - No. of connection not 1");
			return;
		}
		for (sub_type_2 = PM_STA_MODE;
			sub_type_2 < PM_MAX_NUM_OF_MODE; sub_type_2++) {

			policy_mgr_incr_connection_count_utfw(
				hdd_ctx->psoc, vdevid_2, tx_stream_2,
				rx_stream_2, chain_mask_2, sub_type_2,
				second_chnl, mac_id_2);
			/* validate two connections are created or no */
			if (policy_mgr_get_connection_count(hdd_ctx->psoc)
				!= 2) {
				hdd_err("Test fail - No. connection not 2");
				return;
			}
			next_sub_type = PM_STA_MODE;
			while (next_sub_type < PM_MAX_NUM_OF_MODE) {
				third_index =
				policy_mgr_get_third_connection_pcl_table_index(
						hdd_ctx->psoc);
				if (PM_MAX_TWO_CONNECTION_MODE ==
						third_index) {
					/* not valid combination */
					next_sub_type++;
					continue;
				}
				pcl_len = 0;
				pcl_type =
				policy_mgr_get_pcl_from_third_conn_table(
					third_index, next_sub_type,
					system_pref,
					policy_mgr_is_hw_dbs_capable(
					hdd_ctx->psoc));
				policy_mgr_get_pcl(
					hdd_ctx->psoc, next_sub_type,
					pcl_freqs, &pcl_len, weight_list,
					QDF_ARRAY_SIZE(weight_list));
				status = wlan_hdd_validate_pcl(hdd_ctx,
					pcl_type, pcl_freqs, pcl_len,
					first_chnl, second_chnl,
					reason, sizeof(reason));
				if ((pcl_type == PM_MAX_PCL_TYPE) &&
					(pcl_freqs[0] == 0)) {
					next_sub_type++;
					continue;
				}
				fill_report(hdd_ctx, "3 connections",
					sub_type_1, sub_type_2,
					next_sub_type, first_chnl,
					second_chnl, 0, status,
					pcl_type, reason, pcl_freqs,
					pcl_len);
				next_sub_type++;
			}
			/* remove entry to make a room for next iteration */
			policy_mgr_decr_connection_count_utfw(
				hdd_ctx->psoc, false, vdevid_2);
		}
		policy_mgr_decr_connection_count_utfw(
				hdd_ctx->psoc, false, vdevid_1);

		next_sub_type = PM_STA_MODE;
	}
}
