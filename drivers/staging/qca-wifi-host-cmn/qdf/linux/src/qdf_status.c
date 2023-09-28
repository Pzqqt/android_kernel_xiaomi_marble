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

#include "linux/errno.h"
#include "qdf_module.h"
#include "qdf_status.h"

int qdf_status_to_os_return(QDF_STATUS status)
{
	switch (status) {
	case QDF_STATUS_SUCCESS:
		return 0;
	case QDF_STATUS_E_RESOURCES:
		return -EBUSY;
	case QDF_STATUS_E_NOMEM:
		return -ENOMEM;
	case QDF_STATUS_E_AGAIN:
		return -EAGAIN;
	case QDF_STATUS_E_INVAL:
		return -EINVAL;
	case QDF_STATUS_E_FAULT:
		return -EFAULT;
	case QDF_STATUS_E_ALREADY:
		return -EALREADY;
	case QDF_STATUS_E_BADMSG:
		return -EBADMSG;
	case QDF_STATUS_E_BUSY:
		return -EBUSY;
	case QDF_STATUS_E_CANCELED:
		return -ECANCELED;
	case QDF_STATUS_E_ABORTED:
		return -ECONNABORTED;
	case QDF_STATUS_E_PERM:
		return -EPERM;
	case QDF_STATUS_E_EXISTS:
		return -EEXIST;
	case QDF_STATUS_E_NOENT:
		return -ENOENT;
	case QDF_STATUS_E_E2BIG:
		return -E2BIG;
	case QDF_STATUS_E_NOSPC:
		return -ENOSPC;
	case QDF_STATUS_E_ADDRNOTAVAIL:
		return -EADDRNOTAVAIL;
	case QDF_STATUS_E_ENXIO:
		return -ENXIO;
	case QDF_STATUS_E_NETDOWN:
		return -ENETDOWN;
	case QDF_STATUS_E_IO:
		return -EIO;
	case QDF_STATUS_E_NETRESET:
		return -ENETRESET;
	case QDF_STATUS_E_PENDING:
		return -EINPROGRESS;
	case QDF_STATUS_E_TIMEOUT:
		return -ETIMEDOUT;
	default:
		return -EPERM;
	}
}
qdf_export_symbol(qdf_status_to_os_return);

QDF_STATUS qdf_status_from_os_return(int rc)
{
	switch (rc) {
	case 0:
		return QDF_STATUS_SUCCESS;
	case -ENOMEM:
		return QDF_STATUS_E_NOMEM;
	case -EAGAIN:
		return QDF_STATUS_E_AGAIN;
	case -EINVAL:
		return QDF_STATUS_E_INVAL;
	case -EFAULT:
		return QDF_STATUS_E_FAULT;
	case -EALREADY:
		return QDF_STATUS_E_ALREADY;
	case -EBADMSG:
		return QDF_STATUS_E_BADMSG;
	case -EBUSY:
		return QDF_STATUS_E_BUSY;
	case -ECANCELED:
		return QDF_STATUS_E_CANCELED;
	case -ECONNABORTED:
		return QDF_STATUS_E_ABORTED;
	case -EPERM:
		return QDF_STATUS_E_PERM;
	case -EEXIST:
		return QDF_STATUS_E_EXISTS;
	case -ENOENT:
		return QDF_STATUS_E_NOENT;
	case -E2BIG:
		return QDF_STATUS_E_E2BIG;
	case -ENOSPC:
		return QDF_STATUS_E_NOSPC;
	case -EADDRNOTAVAIL:
		return QDF_STATUS_E_ADDRNOTAVAIL;
	case -ENXIO:
		return QDF_STATUS_E_ENXIO;
	case -ENETDOWN:
		return QDF_STATUS_E_NETDOWN;
	case -EIO:
		return QDF_STATUS_E_IO;
	case -ENETRESET:
		return QDF_STATUS_E_NETRESET;
	case -EINPROGRESS:
		return QDF_STATUS_E_PENDING;
	case -ETIMEDOUT:
		return QDF_STATUS_E_TIMEOUT;
	default:
		return QDF_STATUS_E_PERM;
	}
}
qdf_export_symbol(qdf_status_from_os_return);

