/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

#ifndef _I_CDF_UTIL_H
#define _I_CDF_UTIL_H

#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <errno.h>

#include <linux/random.h>

#include <cdf_types.h>
#include <cdf_status.h>
#include <qdf_status.h>
#include <asm/byteorder.h>
/*
 * Generic compiler-dependent macros if defined by the OS
 */

#define __cdf_unlikely(_expr)   unlikely(_expr)
#define __cdf_likely(_expr)     likely(_expr)

/**
 * cdf_status_to_os_return(): translates cdf_status types to linux return types
 * @status: status to translate
 *
 * Translates error types that linux may want to handle specially.
 *
 * return: 0 or the linux error code that most closely matches the CDF_STATUS.
 *	defaults to -1 (EPERM)
 */
static inline int __cdf_status_to_os_return(CDF_STATUS status)
{
	switch (status) {
	case CDF_STATUS_SUCCESS:
		return 0;
	case CDF_STATUS_E_NULL_VALUE:
	case CDF_STATUS_E_FAULT:
		return -EFAULT;
	case CDF_STATUS_E_TIMEOUT:
	case CDF_STATUS_E_BUSY:
		return -EBUSY;
	case CDF_STATUS_NOT_INITIALIZED:
	case CDF_STATUS_E_AGAIN:
		return -EAGAIN;
	case CDF_STATUS_E_NOSUPPORT:
		return -ENOSYS;
	case CDF_STATUS_E_ALREADY:
		return -EALREADY;
	case CDF_STATUS_E_NOMEM:
		return -ENOMEM;
	case CDF_STATUS_E_FAILURE:
	case CDF_STATUS_E_INVAL:
		return -EINVAL;
	default:
		return -EPERM;
	}
}


/**
 * @brief memory barriers.
 */

#define __cdf_min(_a, _b)         ((_a) < (_b) ? _a : _b)
#define __cdf_max(_a, _b)         ((_a) > (_b) ? _a : _b)

/**
 * @brief Assert
 */
#define __cdf_assert(expr)  do {    \
		if (unlikely(!(expr))) {				\
			pr_err("Assertion failed! %s:%s %s:%d\n",   \
			       # expr, __func__, __FILE__, __LINE__);      \
			dump_stack();					   \
			BUG_ON(1);	   \
		}     \
} while (0)

#define __cdf_os_cpu_to_le64                cpu_to_le64
#define __cdf_le16_to_cpu                le16_to_cpu
#define __cdf_le32_to_cpu                le32_to_cpu
#define __cdf_container_of(ptr, type, member) container_of(ptr, type, member)

#endif /*_I_CDF_UTIL_H*/
