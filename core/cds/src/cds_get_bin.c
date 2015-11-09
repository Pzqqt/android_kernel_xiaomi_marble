/*
 * Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
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

#include <cds_get_bin.h>
#include <cds_api.h>
#include <cds_sched.h>
#include <wlan_hdd_misc.h>
#include <wlan_hdd_main.h>

tCDF_CON_MODE cds_get_conparam(void)
{
	tCDF_CON_MODE con_mode;
	con_mode = hdd_get_conparam();
	return con_mode;
}

bool cds_concurrent_open_sessions_running(void)
{
	uint8_t i = 0;
	uint8_t j = 0;
	hdd_context_t *pHddCtx;

	pHddCtx = cds_get_context(CDF_MODULE_ID_HDD);
	if (NULL != pHddCtx) {
		for (i = 0; i < CDF_MAX_NO_OF_MODE; i++) {
			j += pHddCtx->no_of_open_sessions[i];
		}
	}

	return j > 1;
}

#ifdef WLAN_FEATURE_MBSSID
bool cds_concurrent_beaconing_sessions_running(void)
{
	uint8_t i = 0;
	hdd_context_t *pHddCtx;

	pHddCtx = cds_get_context(CDF_MODULE_ID_HDD);
	if (NULL != pHddCtx) {
		i = pHddCtx->no_of_open_sessions[CDF_SAP_MODE] +
		    pHddCtx->no_of_open_sessions[CDF_P2P_GO_MODE] +
		    pHddCtx->no_of_open_sessions[CDF_IBSS_MODE];
	}
	return i > 1;
}
#endif

/**---------------------------------------------------------------------------
*
*   \brief cds_max_concurrent_connections_reached()
*
*   This function checks for presence of concurrency where more than
*   one connection exists and it returns true if the max concurrency is
*   reached.
*
*   Example:
*   STA + STA (wlan0 and wlan1 are connected) - returns true
*   STA + STA (wlan0 connected and wlan1 disconnected) - returns false
*   DUT with P2P-GO + P2P-CLIENT connection) - returns true
*
*   \param  - None
*
*   \return - true or false
*
* --------------------------------------------------------------------------*/
bool cds_max_concurrent_connections_reached(void)
{
	uint8_t i = 0, j = 0;
	hdd_context_t *pHddCtx;

	pHddCtx = cds_get_context(CDF_MODULE_ID_HDD);
	if (NULL != pHddCtx) {
		for (i = 0; i < CDF_MAX_NO_OF_MODE; i++)
			j += pHddCtx->no_of_active_sessions[i];
		return j >
			(pHddCtx->config->
			 gMaxConcurrentActiveSessions - 1);
	}

	return false;
}

void cds_clear_concurrent_session_count(void)
{
	uint8_t i = 0;
	hdd_context_t *pHddCtx;

	pHddCtx = cds_get_context(CDF_MODULE_ID_HDD);
	if (NULL != pHddCtx) {
		for (i = 0; i < CDF_MAX_NO_OF_MODE; i++)
			pHddCtx->no_of_active_sessions[i] = 0;
	}
}

/**---------------------------------------------------------------------------
*
*   \brief cds_is_multiple_active_sta_sessions()
*
*   This function checks for presence of multiple active sta connections
*   and it returns true if the more than 1 active sta connection exists.
*
*   \param  - None
*
*   \return - true or false
*
* --------------------------------------------------------------------------*/
bool cds_is_multiple_active_sta_sessions(void)
{
	hdd_context_t *pHddCtx;
	uint8_t j = 0;

	pHddCtx = cds_get_context(CDF_MODULE_ID_HDD);
	if (NULL != pHddCtx)
		j = pHddCtx->no_of_active_sessions[CDF_STA_MODE];

	return j > 1;
}

/**---------------------------------------------------------------------------
*
*   \brief cds_is_sta_active_connection_exists()
*
*   This function checks for the presence of active sta connection
*   and it returns true if exists.
*
*   \param  - None
*
*   \return - true or false
*
* --------------------------------------------------------------------------*/
bool cds_is_sta_active_connection_exists(void)
{
	hdd_context_t *pHddCtx;
	uint8_t j = 0;

	pHddCtx = cds_get_context(CDF_MODULE_ID_HDD);
	if (NULL != pHddCtx)
		j = pHddCtx->no_of_active_sessions[CDF_STA_MODE];

	return j ? true : false;
}
