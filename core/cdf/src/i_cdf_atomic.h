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
#ifndef I_CDF_ATOMIC_H
#define I_CDF_ATOMIC_H

#include <cdf_status.h>         /* CDF_STATUS */

#include <linux/atomic.h>

typedef atomic_t __cdf_atomic_t;

static inline CDF_STATUS __cdf_atomic_init(__cdf_atomic_t *v)
{
	atomic_set(v, 0);

	return CDF_STATUS_SUCCESS;
}

static inline int32_t __cdf_atomic_read(__cdf_atomic_t *v)
{
	return atomic_read(v);
}

static inline void __cdf_atomic_inc(__cdf_atomic_t *v)
{
	atomic_inc(v);
}

static inline void __cdf_atomic_dec(__cdf_atomic_t *v)
{
	atomic_dec(v);
}

static inline void __cdf_atomic_add(int i, __cdf_atomic_t *v)
{
	atomic_add(i, v);
}

/**
 * cdf_atomic_sub() - Subtract a value from an atomic variable
 * @i: the amount by which to decrease the atomic counter
 * @v: a pointer to an opaque atomic variable
 *
 * Return: none
 */
static inline void __cdf_atomic_sub(int i, __cdf_atomic_t *v)
{
	atomic_sub(i, v);
}

static inline int32_t __cdf_atomic_dec_and_test(__cdf_atomic_t *v)
{
	return atomic_dec_and_test(v);
}

static inline void __cdf_atomic_set(__cdf_atomic_t *v, int i)
{
	atomic_set(v, i);
}

static inline int32_t __cdf_atomic_inc_return(__cdf_atomic_t *v)
{
	return atomic_inc_return(v);
}

#endif
