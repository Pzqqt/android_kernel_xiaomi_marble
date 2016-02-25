/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_napi.c
 *
 * WLAN HDD NAPI interface implementation
 */
#include <smp.h> /* get_cpu */

#include "wlan_hdd_napi.h"
#include "cds_api.h"       /* cds_get_context */
#include "hif.h"           /* hif_map_service...*/
#include "wlan_hdd_main.h" /* hdd_err/warn... */
#include "cdf_types.h"     /* CDF_MODULE_ID_... */
#include "ce_api.h"

/*  guaranteed to be initialized to zero/NULL by the standard */
static struct qca_napi_data *hdd_napi_ctx;

/**
 * hdd_napi_get_all() - return the whole NAPI structure from HIF
 *
 * Gets to the data structure common to all NAPI instances.
 *
 * Return:
 *  NULL  : probably NAPI not initialized yet.
 *  <addr>: the address of the whole NAPI structure
 */
struct qca_napi_data *hdd_napi_get_all(void)
{
	struct qca_napi_data *rp = NULL;
	struct hif_opaque_softc *hif;

	NAPI_DEBUG("-->");

	hif = cds_get_context(CDF_MODULE_ID_HIF);
	if (unlikely(NULL == hif))
		CDF_ASSERT(NULL != hif); /* WARN */
	else
		rp = hif_napi_get_all(hif);

	NAPI_DEBUG("<-- [addr=%p]", rp);
	return rp;
}

/**
 * hdd_napi_get_map() - get a copy of napi pipe map
 *
 * Return:
 *  uint32_t  : copy of pipe map
 */
static uint32_t hdd_napi_get_map(void)
{
	uint32_t map = 0;

	NAPI_DEBUG("-->");
	/* cache once, use forever */
	if (hdd_napi_ctx == NULL)
		hdd_napi_ctx = hdd_napi_get_all();
	if (hdd_napi_ctx != NULL)
		map = hdd_napi_ctx->ce_map;

	NAPI_DEBUG("<-- [map=0x%08x]", map);
	return map;
}

/**
 * hdd_napi_create() - creates the NAPI structures for a given netdev
 *
 * Creates NAPI instances. This function is called
 * unconditionally during initialization. It creates
 * napi structures through the proper HTC/HIF calls.
 * The structures are disabled on creation.
 *
 * Return:
 *   single-queue: <0: err, >0=id, 0 (should not happen)
 *   multi-queue: bitmap of created instances (0: none)
 */
int hdd_napi_create(void)
{
	struct hif_opaque_softc *hif_ctx;
	uint8_t ul, dl;
	int     ul_polled, dl_polled;
	int     rc = 0;

	NAPI_DEBUG("-->");

	hif_ctx = cds_get_context(CDF_MODULE_ID_HIF);
	if (unlikely(NULL == hif_ctx)) {
		CDF_ASSERT(NULL != hif_ctx);
		rc = -EFAULT;
	} else {
		/*
		 * Note: hif_service_to_pipe returns one pipe id per service.
		 * For multi-queue NAPI for Adrastea, we will use multiple
		 * services/calls.
		 * For Rome, there is only one service, hence a single call
		 */
		if (CDF_STATUS_SUCCESS !=
		    hif_map_service_to_pipe(hif_ctx, HTT_DATA_MSG_SVC,
					    &ul, &dl, &ul_polled, &dl_polled)) {
			hdd_err("cannot map service to pipe");
			rc = -EINVAL;
		} else {
			rc = hif_napi_create(hif_ctx, dl, hdd_napi_poll,
					     QCA_NAPI_BUDGET,
					     QCA_NAPI_DEF_SCALE);
			if (rc < 0)
				hdd_err("ERR(%d) creating NAPI on pipe %d",
					rc, dl);
			else {
				hdd_info("napi instance %d created on pipe %d",
					 rc, dl);
				/* rc = (0x01 << rc); -- phase 2 */
			}
		}
	}
	NAPI_DEBUG("<-- [rc=%d]", rc);

	return rc;
}

/**
 * hdd_napi_destroy() - destroys the NAPI structures for a given netdev
 * @force: if set, will force-disable the instance before _del'ing
 *
 * Destroy NAPI instances. This function is called
 * unconditionally during module removal. It destroy
 * napi structures through the proper HTC/HIF calls.
 *
 * Return:
 *    number of NAPI instances destroyed
 */
int hdd_napi_destroy(int force)
{
	int rc = 0;
	int i;
	uint32_t hdd_napi_map = hdd_napi_get_map();

	NAPI_DEBUG("--> (force=%d)", force);
	if (hdd_napi_map) {
		struct hif_opaque_softc *hif_ctx;

		hif_ctx = cds_get_context(CDF_MODULE_ID_HIF);
		if (unlikely(NULL == hif_ctx))
			CDF_ASSERT(NULL != hif_ctx);
		else
			for (i = 0; i < CE_COUNT_MAX; i++)
				if (hdd_napi_map & (0x01 << i)) {
					if (0 <= hif_napi_destroy(
						    hif_ctx,
						    NAPI_PIPE2ID(i), force)) {
						rc++;
						hdd_napi_map &= ~(0x01 << i);
					} else
						hdd_err("cannot destroy napi %d: (pipe:%d), f=%d\n",
							i,
							NAPI_PIPE2ID(i), force);
				}
	}

	/* if all instances are removed, it is likely that hif_context has been
	 * removed as well, so the cached value of the napi context also needs
	 * to be removed
	 */
	if (force)
		CDF_ASSERT(hdd_napi_map == 0);
	if (0 == hdd_napi_map)
		hdd_napi_ctx = NULL;

	NAPI_DEBUG("<-- [rc=%d]", rc);
	return rc;
}

/**
 * hdd_napi_enabled() - checks if NAPI is enabled (for a given id)
 * @id: the id of the NAPI to check (any= -1)
 *
 * Return:
 *   int: 0  = false (NOT enabled)
 *        !0 = true  (enabbled)
 */
int hdd_napi_enabled(int id)
{
	struct hif_opaque_softc *hif;
	int rc = 0; /* NOT enabled */

	hif = cds_get_context(CDF_MODULE_ID_HIF);
	if (unlikely(NULL == hif))
		CDF_ASSERT(hif != NULL); /* WARN_ON; rc = 0 */
	else if (-1 == id)
		rc = hif_napi_enabled(hif, id);
	else
		rc = hif_napi_enabled(hif, NAPI_ID2PIPE(id));
	return rc;
}

/**
 * hdd_napi_event() - relay the event detected by HDD to HIF NAPI decision maker
 * @event: event code
 * @data : event-specific auxiliary data
 *
 * Return code does not indicate a change, but whether or not NAPI is
 * enabled at the time of the return of the function. That is, if NAPI
 * was disabled before the call, and the event does not cause NAPI to be
 * enabled, a value of 0 will be returned indicating that it is (still)
 * disabled.
 *
 * Return:
 *  < 0: error code
 *  = 0: NAPI state = disabled (after processing the event)
 *  = 1: NAPI state = enabled  (after processing the event)
 */
int hdd_napi_event(enum qca_napi_event event, void *data)
{
	int rc = -EFAULT;  /* assume err */
	struct hif_opaque_softc *hif;

	NAPI_DEBUG("-->(event=%d, aux=%p)", event, data);

	hif = cds_get_context(CDF_MODULE_ID_HIF);
	if (unlikely(NULL == hif))
		CDF_ASSERT(hif != NULL);
	else
		rc = hif_napi_event(hif, event, data);

	NAPI_DEBUG("<--[rc=%d]", rc);
	return rc;
}

/**
 * hdd_napi_poll() - NAPI poll function
 * @napi  : pointer to NAPI struct
 * @budget: the pre-declared budget
 *
 * Implementation of poll function. This function is called
 * by kernel during softirq processing.
 *
 * NOTE FOR THE MAINTAINER:
 *   Make sure this is very close to the ce_tasklet code.
 *
 * Return:
 *   int: the amount of work done ( <= budget )
 */
int hdd_napi_poll(struct napi_struct *napi, int budget)
{
	return hif_napi_poll(cds_get_context(CDF_MODULE_ID_HIF), napi, budget);
}
