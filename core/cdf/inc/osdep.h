/*
 * Copyright (c) 2013-2015 The Linux Foundation. All rights reserved.
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

#ifndef _OSDEP_H
#define _OSDEP_H

#include <qdf_types.h>
#include <cdf_memory.h>
#include <cdf_lock.h>
#include <qdf_time.h>
#include <qdf_timer.h>
#include <qdf_defer.h>
#include <cdf_nbuf.h>
#include <cds_if_upperproto.h>

#include <cds_queue.h>

/**
 * enum ath_hal_bus_type - Supported Bus types
 * @HAL_BUS_TYPE_PCI: PCI Bus
 * @HAL_BUS_TYPE_AHB: AHB Bus
 * @HAL_BUS_TYPE_SNOC: SNOC Bus
 * @HAL_BUS_TYPE_SIM: Simulator
 */
enum ath_hal_bus_type {
	HAL_BUS_TYPE_PCI,
	HAL_BUS_TYPE_AHB,
	HAL_BUS_TYPE_SNOC,
	HAL_BUS_TYPE_SIM
};

/**
 * sturct hal_bus_context - Bus to hal context handoff
 * @bc_tag:     bus context tag
 * @bc_handle:  bus context handle
 * @bc_bustype: bus type
 */
typedef struct hal_bus_context {
        int bc_tag;
        char *bc_handle;
        enum ath_hal_bus_type bc_bustype;
} HAL_BUS_CONTEXT;

#define INLINE   inline

/* ATH_DEBUG -
 * Control whether debug features (printouts, assertions) are compiled
 * into the driver.
 */
#ifndef ATH_DEBUG
#define ATH_DEBUG 1             /* default: include debug code */
#endif

#if ATH_DEBUG
#ifndef ASSERT
#define ASSERT(expr)  cdf_assert(expr)
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
#define ath_sysctl_decl(f, ctl, write, filp, buffer, lenp, ppos) \
	f(struct ctl_table *ctl, int write, void *buffer,		     \
	  size_t *lenp, loff_t *ppos)
#define ATH_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer, lenp, ppos) \
	proc_dointvec(ctl, write, buffer, lenp, ppos)
#define ATH_SYSCTL_PROC_DOSTRING(ctl, write, filp, buffer, lenp, ppos) \
	proc_dostring(ctl, write, filp, buffer, lenp, ppos)

/*
 * Byte Order stuff
 */
#define    le16toh(_x)    le16_to_cpu(_x)
#define    htole16(_x)    cpu_to_le16(_x)
#define    htobe16(_x)    cpu_to_be16(_x)
#define    le32toh(_x)    le32_to_cpu(_x)
#define    htole32(_x)    cpu_to_le32(_x)
#define    be16toh(_x)    be16_to_cpu(_x)
#define    be32toh(_x)    be32_to_cpu(_x)
#define    htobe32(_x)    cpu_to_be32(_x)

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
#define OS_MEMCPY(_dst, _src, _len)       cdf_mem_copy(_dst, _src, _len)
#define OS_MEMMOVE(_dst, _src, _len)      cdf_mem_move(_dst, _src, _len)
#define OS_MEMZERO(_buf, _len)            cdf_mem_zero(_buf, _len)
#define OS_MEMSET(_buf, _ch, _len)        cdf_mem_set(_buf, _len, _ch)
#define OS_MEMCMP(_mem1, _mem2, _len)     cdf_mem_compare(_mem1, _mem2, _len)

#ifdef CONFIG_SMP
/* Undo the one provided by the kernel to debug spin locks */
#undef spin_lock
#undef spin_unlock
#undef spin_trylock

#define spin_lock(x) \
	do { \
		spin_lock_bh(x); \
	} while (0)

#define spin_unlock(x) \
	do { \
		if (!spin_is_locked(x)) { \
			WARN_ON(1); \
			printk(KERN_EMERG " %s:%d unlock addr=%p, %s \n", __func__, __LINE__, x, \
			       !spin_is_locked(x) ? "Not locked" : "");	\
		} \
		spin_unlock_bh(x); \
	} while (0)

#define spin_trylock(x) spin_trylock_bh(x)

#define OS_SUPPORT_ASYNC_Q 1    /* support for handling asyn function calls */

#else
#define OS_SUPPORT_ASYNC_Q 0
#endif /* ifdef CONFIG_SMP */


/*
 * System time interface
 */
typedef qdf_time_t systime_t;
typedef qdf_time_t systick_t;

static INLINE qdf_time_t os_get_timestamp(void)
{
	return qdf_system_ticks();      /* Fix double conversion from jiffies to ms */
}

struct _NIC_DEV;

typedef struct _NIC_DEV *osdev_t;

typedef struct timer_list os_timer_t;

typedef struct _os_mesg_t {
	STAILQ_ENTRY(_os_mesg_t) mesg_next;
	uint16_t mesg_type;
	uint16_t mesg_len;
	/* followed by mesg_len bytes */
} os_mesg_t;

typedef void (*os_mesg_handler_t)(void *ctx,
				  uint16_t mesg_type,
				  uint16_t mesg_len, void *mesg);

typedef struct {
	osdev_t dev_handle;
	int32_t num_queued;
	int32_t mesg_len;
	uint8_t *mesg_queue_buf;
	STAILQ_HEAD(, _os_mesg_t) mesg_head;    /* queued mesg buffers */
	STAILQ_HEAD(, _os_mesg_t) mesg_free_head;       /* free mesg buffers  */
	spinlock_t lock;
	spinlock_t ev_handler_lock;
#ifdef USE_SOFTINTR
	void *_task;
#else
	os_timer_t _timer;
#endif
	os_mesg_handler_t handler;
	void *ctx;
	uint8_t is_synchronous : 1;
} os_mesg_queue_t;

/*
 * Definition of OS-dependent device structure.
 * It'll be opaque to the actual ATH layer.
 */
struct _NIC_DEV {
	void *bdev;             /* bus device handle */
	struct net_device *netdev;      /* net device handle (wifi%d) */
	qdf_bh_t intr_tq;       /* tasklet */
	struct net_device_stats devstats;       /* net device statisitics */
	HAL_BUS_CONTEXT bc;
#ifdef ATH_PERF_PWR_OFFLOAD
	struct device *device;  /* generic device */
	wait_queue_head_t event_queue;
#endif /* PERF_PWR_OFFLOAD */
#if OS_SUPPORT_ASYNC_Q
	os_mesg_queue_t async_q;        /* mesgq to handle async calls */
#endif
#ifdef ATH_BUS_PM
	uint8_t isDeviceAsleep;
#endif /* ATH_BUS_PM */
};

static INLINE unsigned char *os_malloc(osdev_t pNicDev,
				       unsigned long ulSizeInBytes, int gfp)
{
	return cdf_mem_malloc(ulSizeInBytes);
}

#define OS_FREE(_p)                     cdf_mem_free(_p)

#define OS_DMA_MEM_CONTEXT(context)	    \
	dma_addr_t context;

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
#define OS_DECLARE_TIMER(_fn)                  void _fn(void *)

#define os_timer_func(_fn)		       \
	void _fn(void *timer_arg)

#define OS_GET_TIMER_ARG(_arg, _type)	       \
	(_arg) = (_type)(timer_arg)

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
