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

/**
 * DOC: cdf_atomic.h
 * This file abstracts an atomic counter.
 */

#ifndef _CDF_ATOMIC_H
#define _CDF_ATOMIC_H

#include <i_cdf_atomic.h>

/**
 * cdf_atomic_t - atomic type of variable
 *
 * Use this when you want a simple resource counter etc. which is atomic
 * across multiple CPU's. These maybe slower than usual counters on some
 * platforms/OS'es, so use them with caution.
 */

typedef __cdf_atomic_t cdf_atomic_t;

/**
 * cdf_atomic_init() - initialize an atomic type variable
 * @v:	A pointer to an opaque atomic variable
 *
 * Return: None
 */
static inline void cdf_atomic_init(cdf_atomic_t *v)
{
	__cdf_atomic_init(v);
}

/**
 * cdf_atomic_read() - read the value of an atomic variable
 * @v:	A pointer to an opaque atomic variable
 *
 * Return: The current value of the variable
 */
static inline int32_t cdf_atomic_read(cdf_atomic_t *v)
{
	return __cdf_atomic_read(v);
}

/**
 * cdf_atomic_inc() - increment the value of an atomic variable
 * @v:	A pointer to an opaque atomic variable
 *
 * Return: None
 */
static inline void cdf_atomic_inc(cdf_atomic_t *v)
{
	__cdf_atomic_inc(v);
}

/**
 * cdf_atomic_dec() - decrement the value of an atomic variable
 * @v:	A pointer to an opaque atomic variable
 *
 * Return: None
 */
static inline void cdf_atomic_dec(cdf_atomic_t *v)
{
	__cdf_atomic_dec(v);
}

/**
 * cdf_atomic_add() - add a value to the value of an atomic variable
 * @v:	A pointer to an opaque atomic variable
 * @i:	The amount by which to increase the atomic counter
 *
 * Return: None
 */
static inline void cdf_atomic_add(int i, cdf_atomic_t *v)
{
	__cdf_atomic_add(i, v);
}

/**
 * cdf_atomic_sub() - Subtract a value from an atomic variable.
 * @i: the amount by which to decrease the atomic counter
 * @v: a pointer to an opaque atomic variable
 *
 * Return: none
 */
static inline void cdf_atomic_sub(int i, cdf_atomic_t *v)
{
	__cdf_atomic_sub(i, v);
}

/**
 * cdf_atomic_dec_and_test() - decrement an atomic variable and check if the
 *				new value is zero
 * @v: A pointer to an opaque atomic variable
 *
 * Return:
 *    true (non-zero) if the new value is zero,
 *    or false (0) if the new value is non-zero
 */
static inline int32_t cdf_atomic_dec_and_test(cdf_atomic_t *v)
{
	return __cdf_atomic_dec_and_test(v);
}

/**
 * cdf_atomic_set() - set a value to the value of an atomic variable
 * @v: A pointer to an opaque atomic variable
 *
 * Return: None
 */
static inline void cdf_atomic_set(cdf_atomic_t *v, int i)
{
	__cdf_atomic_set(v, i);
}

/**
 * cdf_atomic_inc_return() - return the incremented value of an atomic variable
 * @v: A pointer to an opaque atomic variable
 *
 * Return: The current value of the variable
 */
static inline int32_t cdf_atomic_inc_return(cdf_atomic_t *v)
{
	return __cdf_atomic_inc_return(v);
}

#endif
