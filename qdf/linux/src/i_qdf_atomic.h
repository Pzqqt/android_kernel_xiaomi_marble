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

/**
 * DOC: i_qdf_atomic.h
 * This file provides OS dependent atomic APIs.
 */

#ifndef I_QDF_ATOMIC_H
#define I_QDF_ATOMIC_H

#include <qdf_status.h>         /* QDF_STATUS */
#include <linux/atomic.h>

typedef atomic_t __qdf_atomic_t;

/**
 * __qdf_atomic_init() - initialize an atomic type variable
 * @v: A pointer to an opaque atomic variable
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS __qdf_atomic_init(__qdf_atomic_t *v)
{
	atomic_set(v, 0);
	return QDF_STATUS_SUCCESS;
}

/**
 * __qdf_atomic_read() - read the value of an atomic variable
 * @v: A pointer to an opaque atomic variable
 *
 * Return: The current value of the variable
 */
static inline int32_t __qdf_atomic_read(__qdf_atomic_t *v)
{
	return atomic_read(v);
}

/**
 * __qdf_atomic_inc() - increment the value of an atomic variable
 * @v: A pointer to an opaque atomic variable
 *
 * Return: None
 */
static inline void __qdf_atomic_inc(__qdf_atomic_t *v)
{
	atomic_inc(v);
}

/**
 * __qdf_atomic_dec() - decrement the value of an atomic variable
 * @v: A pointer to an opaque atomic variable
 *
 * Return: None
 */
static inline void __qdf_atomic_dec(__qdf_atomic_t *v)
{
	atomic_dec(v);
}

/**
 * __qdf_atomic_add() - add a value to the value of an atomic variable
 * @i: The amount by which to increase the atomic counter
 * @v: A pointer to an opaque atomic variable
 *
 * Return: None
 */
static inline void __qdf_atomic_add(int i, __qdf_atomic_t *v)
{
	atomic_add(i, v);
}

/**
 * __qdf_atomic_sub() - Subtract a value from an atomic variable
 * @i: the amount by which to decrease the atomic counter
 * @v: a pointer to an opaque atomic variable
 *
 * Return: none
 */
static inline void __qdf_atomic_sub(int i, __qdf_atomic_t *v)
{
	atomic_sub(i, v);
}

/**
 * __qdf_atomic_dec_and_test() - decrement an atomic variable and check if the
 * new value is zero
 * @v: A pointer to an opaque atomic variable
 *
 * Return:
 * true (non-zero) if the new value is zero,
 * false (0) if the new value is non-zero
 */
static inline int32_t __qdf_atomic_dec_and_test(__qdf_atomic_t *v)
{
	return atomic_dec_and_test(v);
}

/**
 * __qdf_atomic_set() - set a value to the value of an atomic variable
 * @v: A pointer to an opaque atomic variable
 *
 * Return: None
 */
static inline void __qdf_atomic_set(__qdf_atomic_t *v, int i)
{
	atomic_set(v, i);
}

/**
 * __qdf_atomic_inc_return() - return the incremented value of an atomic variable
 * @v: A pointer to an opaque atomic variable
 *
 * Return: The current value of the variable
 */
static inline int32_t __qdf_atomic_inc_return(__qdf_atomic_t *v)
{
	return atomic_inc_return(v);
}

#endif
