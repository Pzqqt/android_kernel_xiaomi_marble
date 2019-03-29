/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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

/**
 * DOC: qdf_status
 * QCA driver framework (QDF) status codes
 * Basic status codes/definitions used by QDF
 */

#ifndef __QDF_STATUS_H
#define __QDF_STATUS_H

/**
 * typedef QDF_STATUS - QDF error codes
 * @QDF_STATUS_SUCCESS: success
 * @QDF_STATUS_E_RESOURCES: system resource(other than memory) not available
 * @QDF_STATUS_E_NOMEM: not enough memory
 * @QDF_STATUS_E_AGAIN: try again
 * @QDF_STATUS_E_INVAL: invalid request
 * @QDF_STATUS_E_FAULT: system fault
 * @QDF_STATUS_E_ALREADY: another request already in progress
 * @QDF_STATUS_E_BADMSG: bad message
 * @QDF_STATUS_E_BUSY: device or resource busy
 * @QDF_STATUS_E_CANCELED: request cancelled
 * @QDF_STATUS_E_ABORTED: request aborted
 * @QDF_STATUS_E_NOSUPPORT: request not supported
 * @QDF_STATUS_E_PERM: operation not permitted
 * @QDF_STATUS_E_EMPTY: empty condition
 * @QDF_STATUS_E_EXISTS: existence failure
 * @QDF_STATUS_E_TIMEOUT: operation timeout
 * @QDF_STATUS_E_FAILURE: unknown reason do not use unless nothign else applies
 * @QDF_STATUS_E_NOENT: No such file or directory
 * @QDF_STATUS_E_E2BIG: Arg list too long
 * @QDF_STATUS_E_NOSPC: no space left on device
 * @QDF_STATUS_E_ADDRNOTAVAIL: Cannot assign requested address
 * @QDF_STATUS_E_ENXIO: No such device or address
 * @QDF_STATUS_E_NETDOWN: network is down
 * @QDF_STATUS_E_IO: I/O Error
 * @QDF_STATUS_E_PENDING: pending status
 * @QDF_STATUS_E_NETRESET: Network dropped connection because of reset
 * @QDF_STATUS_E_SIG: Exit due to received SIGINT
 * @QDF_STATUS_E_PROTO: protocol error
 * @QDF_STATUS_NOT_INITIALIZED: resource not initialized
 * @QDF_STATUS_E_NULL_VALUE: request is null
 * @QDF_STATUS_PMC_PENDING: request pendign in pmc
 * @QDF_STATUS_PMC_DISABLED: pmc is disabled
 * @QDF_STATUS_PMC_NOT_NOW: pmc not ready now
 * @QDF_STATUS_PMC_AC_POWER: pmc ac power
 * @QDF_STATUS_PMC_SYS_ERROR: pmc system error
 * @QDF_STATUS_HEARTBEAT_TMOUT: hearbeat timeout error
 * @QDF_STATUS_NTH_BEACON_DELIVERY: Nth beacon delivery
 * @QDF_STATUS_CSR_WRONG_STATE: csr in wrong state
 * @QDF_STATUS_FT_PREAUTH_KEY_SUCCESS: ft preauth key success
 * @QDF_STATUS_FT_PREAUTH_KEY_FAILED: ft preauth key failed
 * @QDF_STATUS_CMD_NOT_QUEUED: command not queued
 * @QDF_STATUS_FW_MSG_TIMEDOUT: target message timeout
 * @QDF_STATUS_E_USB_ERROR: USB transaction error
 * @QDF_STATUS_MAXCOMP_FAIL: Component id is more than MAX UMAC components
 * @QDF_STATUS_COMP_DISABLED: UMAC Component is disabled
 * @QDF_STATUS_COMP_ASYNC: UMAC component runs in asynchronous communication
 * @QDF_STATUS_CRYPTO_PN_ERROR: PN ERROR in received frame
 * @QDF_STATUS_CRYPTO_MIC_FAILURE: MIC failure in received frame
 * @QDF_STATUS_CRYPTO_ENCRYPT_FAILED: encryption failed
 * @QDF_STATUS_CRYPTO_DECRYPT_FAILED: decryption failed
 * @QDF_STATUS_E_RANGE: result/parameter/operation was out of range
 * @QDF_STATUS_E_GRO_DROP: return code for GRO drop
 * @QDF_STATUS_MAX: not a realy value just a place holder for max
 */
typedef enum {
	QDF_STATUS_SUCCESS,
	QDF_STATUS_E_RESOURCES,
	QDF_STATUS_E_NOMEM,
	QDF_STATUS_E_AGAIN,
	QDF_STATUS_E_INVAL,
	QDF_STATUS_E_FAULT,
	QDF_STATUS_E_ALREADY,
	QDF_STATUS_E_BADMSG,
	QDF_STATUS_E_BUSY,
	QDF_STATUS_E_CANCELED,
	QDF_STATUS_E_ABORTED,
	QDF_STATUS_E_NOSUPPORT,
	QDF_STATUS_E_PERM,
	QDF_STATUS_E_EMPTY,
	QDF_STATUS_E_EXISTS,
	QDF_STATUS_E_TIMEOUT,
	QDF_STATUS_E_FAILURE,
	QDF_STATUS_E_NOENT,
	QDF_STATUS_E_E2BIG,
	QDF_STATUS_E_NOSPC,
	QDF_STATUS_E_ADDRNOTAVAIL,
	QDF_STATUS_E_ENXIO,
	QDF_STATUS_E_NETDOWN,
	QDF_STATUS_E_IO,
	QDF_STATUS_E_PENDING,
	QDF_STATUS_E_NETRESET,
	QDF_STATUS_E_SIG,
	QDF_STATUS_E_PROTO,
	QDF_STATUS_NOT_INITIALIZED,
	QDF_STATUS_E_NULL_VALUE,
	QDF_STATUS_PMC_PENDING,
	QDF_STATUS_PMC_DISABLED,
	QDF_STATUS_PMC_NOT_NOW,
	QDF_STATUS_PMC_AC_POWER,
	QDF_STATUS_PMC_SYS_ERROR,
	QDF_STATUS_HEARTBEAT_TMOUT,
	QDF_STATUS_NTH_BEACON_DELIVERY,
	QDF_STATUS_CSR_WRONG_STATE,
	QDF_STATUS_FT_PREAUTH_KEY_SUCCESS,
	QDF_STATUS_FT_PREAUTH_KEY_FAILED,
	QDF_STATUS_CMD_NOT_QUEUED,
	QDF_STATUS_FW_MSG_TIMEDOUT,
	QDF_STATUS_E_USB_ERROR,
	QDF_STATUS_MAXCOMP_FAIL,
	QDF_STATUS_COMP_DISABLED,
	QDF_STATUS_COMP_ASYNC,
	QDF_STATUS_CRYPTO_PN_ERROR,
	QDF_STATUS_CRYPTO_MIC_FAILURE,
	QDF_STATUS_CRYPTO_ENCRYPT_FAILED,
	QDF_STATUS_CRYPTO_DECRYPT_FAILED,
	QDF_STATUS_E_DEFRAG_ERROR,
	QDF_STATUS_E_RANGE,
	QDF_STATUS_E_GRO_DROP,
	QDF_STATUS_MAX
} QDF_STATUS;

#define QDF_IS_STATUS_SUCCESS(status) (QDF_STATUS_SUCCESS == (status))
#define QDF_IS_STATUS_ERROR(status) (QDF_STATUS_SUCCESS != (status))

/**
 * qdf_status_to_os_return() - map a QDF_STATUS into an OS specific return code
 * @status: QDF_STATUS to map
 *
 * Return: an OS specific error code
 */
int qdf_status_to_os_return(QDF_STATUS status);

/**
 * qdf_status_from_os_return() - map an OS specific return code to a QDF_STATUS
 * @rc: the input return code to map
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_status_from_os_return(int rc);

#endif /* __QDF_STATUS_H */

