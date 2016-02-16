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

#if !defined(__CDF_STATUS_H)
#define __CDF_STATUS_H

#include <qdf_status.h>
/**
 * DOC:  cdf_status
 *
 * Connectivity driver framework (CDF) status codes
 *
 * Basic status codes/definitions used by CDF
 */

/**
 * typedef CDF_STATUS - CDF error codes
 * @CDF_STATUS_SUCCESS: success
 * @CDF_STATUS_E_RESOURCES: system resource(other than memory) not available
 * @CDF_STATUS_E_NOMEM: not enough memory
 * @CDF_STATUS_E_AGAIN: try again
 * @CDF_STATUS_E_INVAL: invalid request
 * @CDF_STATUS_E_FAULT: system fault
 * @CDF_STATUS_E_ALREADY: another request already in progress
 * @CDF_STATUS_E_BADMSG: bad message
 * @CDF_STATUS_E_BUSY: device or resource busy
 * @CDF_STATUS_E_CANCELED: request cancelled
 * @CDF_STATUS_E_ABORTED: request aborted
 * @CDF_STATUS_E_NOSUPPORT: request not supported
 * @CDF_STATUS_E_PERM: operation not permitted
 * @CDF_STATUS_E_EMPTY: empty condition
 * @CDF_STATUS_E_EXISTS: existence failure
 * @CDF_STATUS_E_TIMEOUT: operation timeout
 * @CDF_STATUS_E_FAILURE: unknown reason do not use unless nothign else applies
 * @CDF_STATUS_NOT_INITIALIZED: resource not initialized
 * @CDF_STATUS_E_NULL_VALUE: request is null
 * @CDF_STATUS_PMC_PENDING: request pendign in pmc
 * @CDF_STATUS_PMC_DISABLED: pmc is disabled
 * @CDF_STATUS_PMC_NOT_NOW: pmc not ready now
 * @CDF_STATUS_PMC_AC_POWER: pmc ac power
 * @CDF_STATUS_PMC_SYS_ERROR: pmc system error
 * @CDF_STATUS_HEARTBEAT_TMOUT: hearbeat timeout error
 * @CDF_STATUS_NTH_BEACON_DELIVERY: Nth beacon delivery
 * @CDF_STATUS_CSR_WRONG_STATE: csr in wrong state
 * @CDF_STATUS_FT_PREAUTH_KEY_SUCCESS: ft preauth key success
 * @CDF_STATUS_FT_PREAUTH_KEY_FAILED: ft preauth key failed
 * @CDF_STATUS_CMD_NOT_QUEUED: command not queued
 * @CDF_STATUS_FW_MSG_TIMEDOUT: target message timeout
 * @CDF_STATUS_MAX: not a realy value just a place holder for max
 */
typedef enum {
	CDF_STATUS_SUCCESS,
	CDF_STATUS_E_RESOURCES,
	CDF_STATUS_E_NOMEM,
	CDF_STATUS_E_AGAIN,
	CDF_STATUS_E_INVAL,
	CDF_STATUS_E_FAULT,
	CDF_STATUS_E_ALREADY,
	CDF_STATUS_E_BADMSG,
	CDF_STATUS_E_BUSY,
	CDF_STATUS_E_CANCELED,
	CDF_STATUS_E_ABORTED,
	CDF_STATUS_E_NOSUPPORT,
	CDF_STATUS_E_PERM,
	CDF_STATUS_E_EMPTY,
	CDF_STATUS_E_EXISTS,
	CDF_STATUS_E_TIMEOUT,
	CDF_STATUS_E_FAILURE,
	CDF_STATUS_NOT_INITIALIZED,
	CDF_STATUS_E_NULL_VALUE,
	CDF_STATUS_PMC_PENDING,
	CDF_STATUS_PMC_DISABLED,
	CDF_STATUS_PMC_NOT_NOW,
	CDF_STATUS_PMC_AC_POWER,
	CDF_STATUS_PMC_SYS_ERROR,
	CDF_STATUS_HEARTBEAT_TMOUT,
	CDF_STATUS_NTH_BEACON_DELIVERY,
	CDF_STATUS_CSR_WRONG_STATE,
	CDF_STATUS_FT_PREAUTH_KEY_SUCCESS,
	CDF_STATUS_FT_PREAUTH_KEY_FAILED,
	CDF_STATUS_CMD_NOT_QUEUED,
	CDF_STATUS_FW_MSG_TIMEDOUT,
	CDF_STATUS_MAX
} CDF_STATUS;

#define CDF_IS_STATUS_SUCCESS(status) (CDF_STATUS_SUCCESS == (status))

#endif /* if !defined __CDF_STATUS_H */
