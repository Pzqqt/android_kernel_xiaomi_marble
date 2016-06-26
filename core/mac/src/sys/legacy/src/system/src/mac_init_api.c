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

/*
 *
 * mac_init_api.c - This file has all the mac level init functions
 *                   for all the defined threads at system level.
 * Author:    Dinesh Upadhyay
 * Date:      04/23/2007
 * History:-
 * Date: 04/08/2008       Modified by: Santosh Mandiganal
 * Modification Information: Code to allocate and free the  memory for DumpTable entry.
 * --------------------------------------------------------------------------
 *
 */
/* Standard include files */
#include "cfg_api.h"             /* cfg_cleanup */
#include "lim_api.h"             /* lim_cleanup */
#include "sir_types.h"
#include "sys_debug.h"
#include "sys_entry_func.h"
#include "mac_init_api.h"

#ifdef TRACE_RECORD
#include "mac_trace.h"
#endif

extern tSirRetStatus halDoCfgInit(tpAniSirGlobal pMac);
extern tSirRetStatus halProcessStartEvent(tpAniSirGlobal pMac);

tSirRetStatus mac_start(tHalHandle hHal, void *pHalMacStartParams)
{
	tSirRetStatus status = eSIR_SUCCESS;
	tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

	if (NULL == pMac) {
		QDF_ASSERT(0);
		status = eSIR_FAILURE;
		return status;
	}

	pMac->gDriverType =
		((tHalMacStartParameters *) pHalMacStartParams)->driverType;

	sys_log(pMac, LOG2, FL("called\n"));

	if (ANI_DRIVER_TYPE(pMac) != eDRIVER_TYPE_MFG) {
		status = pe_start(pMac);
	}

	return status;
}

/** -------------------------------------------------------------
   \fn mac_stop
   \brief this function will be called from HDD to stop MAC. This function will stop all the mac modules.
 \       memory with global context will only be initialized not freed here.
   \param   tHalHandle hHal
   \param tHalStopType
   \return tSirRetStatus
   -------------------------------------------------------------*/

tSirRetStatus mac_stop(tHalHandle hHal, tHalStopType stopType)
{
	tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;
	pe_stop(pMac);
	cfg_cleanup(pMac);

	return eSIR_SUCCESS;
}

/** -------------------------------------------------------------
   \fn mac_open
   \brief this function will be called during init. This function is suppose to allocate all the
 \       memory with the global context will be allocated here.
   \param   tHalHandle pHalHandle
   \param   tHddHandle hHdd
   \param   tHalOpenParameters* pHalOpenParams
   \return tSirRetStatus
   -------------------------------------------------------------*/

tSirRetStatus mac_open(tHalHandle *pHalHandle, tHddHandle hHdd,
		       struct cds_config_info *cds_cfg)
{
	tpAniSirGlobal p_mac = NULL;
	tSirRetStatus status = eSIR_SUCCESS;

	if (pHalHandle == NULL)
		return eSIR_FAILURE;

	/*
	 * Make sure this adapter is not already opened. (Compare pAdapter pointer in already
	 * allocated p_mac structures.)
	 * If it is opened just return pointer to previously allocated p_mac pointer.
	 * Or should this result in error?
	 */

	/* Allocate p_mac */
	p_mac = qdf_mem_malloc(sizeof(tAniSirGlobal));
	if (NULL == p_mac)
		return eSIR_MEM_ALLOC_FAILED;

	/* Initialize the p_mac structure */
	qdf_mem_set(p_mac, sizeof(tAniSirGlobal), 0);

	/*
	 * Set various global fields of p_mac here
	 * (Could be platform dependant as some variables in p_mac are platform
	 * dependant)
	 */
	p_mac->hHdd = hHdd;
	*pHalHandle = (tHalHandle) p_mac;

	{
		/* Call various PE (and other layer init here) */
		if (eSIR_SUCCESS != log_init(p_mac)) {
			qdf_mem_free(p_mac);
			return eSIR_FAILURE;
		}

		/* Call routine to initialize CFG data structures */
		if (eSIR_SUCCESS != cfg_init(p_mac)) {
			qdf_mem_free(p_mac);
			return eSIR_FAILURE;
		}

		sys_init_globals(p_mac);
	}

	/* FW: 0 to 2047 and Host: 2048 to 4095 */
	p_mac->mgmtSeqNum = WLAN_HOST_SEQ_NUM_MIN - 1;
	p_mac->first_scan_done = false;

	status =  pe_open(p_mac, cds_cfg);
	if (eSIR_SUCCESS != status) {
		sys_log(p_mac, LOGE, FL("mac_open failure\n"));
		qdf_mem_free(p_mac);
	}

	return status;
}

/** -------------------------------------------------------------
   \fn mac_close
   \brief this function will be called in shutdown sequence from HDD. All the
 \       allocated memory with global context will be freed here.
   \param   tpAniSirGlobal pMac
   \return none
   -------------------------------------------------------------*/

tSirRetStatus mac_close(tHalHandle hHal)
{

	tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

	if (!pMac)
		return eSIR_FAILURE;

	pe_close(pMac);

	/* Call routine to free-up all CFG data structures */
	cfg_de_init(pMac);

	log_deinit(pMac);

	/* Finally, de-allocate the global MAC datastructure: */
	qdf_mem_free(pMac);

	return eSIR_SUCCESS;
}
