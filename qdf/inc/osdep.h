/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
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
 * DOC: osdep
 * This file provides OS abstraction for osdependent APIs.
 */

#ifndef _OSDEP_H
#define _OSDEP_H

#include <qdf_types.h>
#include <qdf_mem.h>
#include <qdf_lock.h>
#include <qdf_time.h>
#include <qdf_timer.h>
#include <qdf_defer.h>
#include <qdf_nbuf.h>
#include <i_osdep.h>

/*
 * ATH_DEBUG -
 * Control whether debug features (printouts, assertions) are compiled
 * into the driver.
 */
#ifndef ATH_DEBUG
#define ATH_DEBUG 1             /* default: include debug code */
#endif

#if ATH_DEBUG
#ifndef ASSERT
#define ASSERT(expr)  qdf_assert(expr)
#endif
#else
#define ASSERT(expr)
#endif /* ATH_DEBUG */

/*
 * Need to define byte order based on the CPU configuration.
 */
#ifndef _LITTLE_ENDIAN
#define _LITTLE_ENDIAN  1234
#endif
#ifndef _BIG_ENDIAN
#define _BIG_ENDIAN 4321
#endif
#ifdef __BIG_ENDIAN
#define _BYTE_ORDER    _BIG_ENDIAN
#else
#define _BYTE_ORDER    _LITTLE_ENDIAN
#endif

/*
 * Deduce if tasklets are available.  If not then
 * fall back to using the immediate work queue.
 */
#define qdf_sysctl_decl(f, ctl, write, filp, buffer, lenp, ppos) \
	f(struct ctl_table *ctl, int32_t write, void *buffer, \
	size_t *lenp, loff_t *ppos)

#define QDF_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer, lenp, ppos) \
	__QDF_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer, lenp, ppos)

#define QDF_SYSCTL_PROC_DOSTRING(ctl, write, filp, buffer, lenp, ppos) \
	__QDF_SYSCTL_PROC_DOSTRING(ctl, write, filp, buffer, lenp, ppos)

#define EOK    (0)

#ifndef false
#define false 0
#endif
#ifndef true
#define true  1
#endif

#ifndef ARPHRD_IEEE80211
#define ARPHRD_IEEE80211 801    /* IEEE 802.11.  */
#endif

/*
 * Normal Delay functions. Time specified in microseconds.
 */
#define OS_DELAY(_us)                     qdf_udelay(_us)

/*
 * memory data manipulation functions.
 */
#define OS_MEMCPY(_dst, _src, _len)       qdf_mem_copy(_dst, _src, _len)
#define OS_MEMMOVE(_dst, _src, _len)      qdf_mem_move(_dst, _src, _len)
#define OS_MEMZERO(_buf, _len)            qdf_mem_zero(_buf, _len)
#define OS_MEMSET(_buf, _ch, _len)        qdf_mem_set(_buf, _len, _ch)
#define OS_MEMCMP(_mem1, _mem2, _len)     qdf_mem_cmp(_mem1, _mem2, _len)


/*
 * System time interface
 */
typedef qdf_time_t systime_t;
typedef qdf_time_t systick_t;

/**
 * os_get_timestamp() - gives the timestamp in ticks
 * Return: unsigned long
 */
static inline qdf_time_t os_get_timestamp(void)
{
	/* Fix double conversion from jiffies to ms */
	return qdf_system_ticks();
}

struct _NIC_DEV;

static inline unsigned char *os_malloc(osdev_t nic_dev,
				       unsigned long size_in_bytes,
				       int32_t gfp)
{
	return qdf_mem_malloc(size_in_bytes);
}

#define OS_FREE(_p)                     qdf_mem_free(_p)

#define OS_DMA_MEM_CONTEXT(context)	    \
		dma_addr_t context

#define OS_GET_DMA_MEM_CONTEXT(var, field)  \
		&(var->field)

#define OS_COPY_DMA_MEM_CONTEXT(dst, src)   \
		*dst = *src

#define OS_ZERO_DMA_MEM_CONTEXT(context)   \
		*context = 0

/*
 * Timer Interfaces. Use these macros to declare timer
 * and retrieve timer argument. This is mainly for resolving
 * different argument types for timer function in different OS.
 */
#define os_timer_func(_fn) \
	void _fn(void *timer_arg)

#define OS_GET_TIMER_ARG(_arg, _type) \
	((_arg) = (_type)(timer_arg))

#define OS_DECLARE_TIMER(_fn)                  void _fn(void *)

#define OS_INIT_TIMER(_osdev, _timer, _fn, _ctx, type)  \
			qdf_timer_init(_osdev, _timer, _fn, _ctx, type)

#define OS_SET_TIMER(_timer, _ms)      qdf_timer_mod(_timer, _ms)

#define OS_CANCEL_TIMER(_timer)        qdf_timer_stop(_timer)

#define OS_FREE_TIMER(_timer)          qdf_timer_stop(_timer)

/*
 * These are required for network manager support
 */
#ifndef SET_NETDEV_DEV
#define    SET_NETDEV_DEV(ndev, pdev)
#endif

#endif /* end of _OSDEP_H */
