/*
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_time
 * This file provides OS dependent time API's.
 */

#ifndef _I_QDF_TIME_H
#define _I_QDF_TIME_H

#include <linux/version.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
#include <linux/sched/clock.h>
#else
#include <linux/sched.h>
#endif
#include <linux/ktime.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))
#include <linux/timekeeping.h>
#else
#include <linux/hrtimer.h>
#endif
#ifdef MSM_PLATFORM
#include <asm/arch_timer.h>
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0))
#include <linux/sched/clock.h>
#else
#include <linux/sched.h>
#endif

typedef unsigned long __qdf_time_t;
typedef ktime_t  __qdf_ktime_t;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
typedef struct timespec64 __qdf_timespec_t;
#else
typedef struct timeval __qdf_timespec_t;
#endif

typedef struct work_struct __qdf_work_struct_t;

/**
 * __qdf_ns_to_ktime() - Converts nanoseconds to a ktime object
 * @ns: time in nanoseconds
 *
 * Return: nanoseconds as ktime object
 */
static inline ktime_t __qdf_ns_to_ktime(uint64_t ns)
{
	return ns_to_ktime(ns);
}

/**
 * __qdf_ktime_add() - Adds two ktime objects and returns
 * a ktime object
 * @time1: time as ktime object
 * @time2: time as ktime object
 *
 * Return: sum of ktime objects as ktime object
 */
static inline ktime_t __qdf_ktime_add(ktime_t ktime1, ktime_t ktime2)
{
	return ktime_add(ktime1, ktime2);
}

/**
 * __qdf_ktime_get() - Gets the current time as ktime object
 *
 * Return: current time as ktime object
 */
static inline ktime_t __qdf_ktime_get(void)
{
	return ktime_get();
}

/**
 * __qdf_ktime_real_get() - Gets the current wall clock as ktime object
 *
 * Return: current wall clock as ktime object
 */
static inline ktime_t __qdf_ktime_real_get(void)
{
	return ktime_get_real();
}

/**
 * __qdf_ktime_add_ns() - Adds ktime object and nanoseconds value and
 * returns the ktime object
 *
 * Return: ktime object
 */
static inline ktime_t __qdf_ktime_add_ns(ktime_t ktime, int64_t ns)
{
	return ktime_add_ns(ktime, ns);
}

/**
 * __qdf_ktime_to_ns() - convert ktime to nanoseconds
 * @ktime: time as ktime object
 * @ns: time in nanoseconds
 *
 * Return: ktime in nanoseconds
 */
static inline int64_t __qdf_ktime_to_ns(ktime_t ktime)
{
	return ktime_to_ns(ktime);
}

/**
 * __qdf_ktime_to_ms() - convert ktime to milliseconds
 * @ktime: time as ktime object
 *
 * Return: ktime in milliseconds
 */
static inline int64_t __qdf_ktime_to_ms(ktime_t ktime)
{
	return ktime_to_ms(ktime);
}


/**
 * __qdf_system_ticks() - get system ticks
 *
 * Return: system tick in jiffies
 */
static inline __qdf_time_t __qdf_system_ticks(void)
{
	return jiffies;
}

#define __qdf_system_ticks_per_sec HZ

/**
 * __qdf_system_ticks_to_msecs() - convert system ticks into milli seconds
 * @ticks: System ticks
 *
 * Return: system tick converted into milli seconds
 */
static inline uint32_t __qdf_system_ticks_to_msecs(unsigned long ticks)
{
	return jiffies_to_msecs(ticks);
}

/**
 * __qdf_system_msecs_to_ticks() - convert milli seconds into system ticks
 * @msecs: Milli seconds
 *
 * Return: milli seconds converted into system ticks
 */
static inline __qdf_time_t __qdf_system_msecs_to_ticks(uint32_t msecs)
{
	return msecs_to_jiffies(msecs);
}

/**
 * __qdf_get_system_uptime() - get system uptime
 *
 * Return: system uptime in jiffies
 */
static inline __qdf_time_t __qdf_get_system_uptime(void)
{
	return jiffies;
}

static inline unsigned long __qdf_get_system_timestamp(void)
{
	return (jiffies / HZ) * 1000 + (jiffies % HZ) * (1000 / HZ);
}

#ifdef CONFIG_ARM
/**
 * __qdf_udelay() - delay execution for given microseconds
 * @usecs: Micro seconds to delay
 *
 * Return: none
 */
static inline void __qdf_udelay(uint32_t usecs)
{
	/*
	 * This is in support of XScale build.  They have a limit on the udelay
	 * value, so we have to make sure we don't approach the limit
	 */
	uint32_t mticks;
	uint32_t leftover;
	int i;
	/* slice into 1024 usec chunks (simplifies calculation) */
	mticks = usecs >> 10;
	leftover = usecs - (mticks << 10);
	for (i = 0; i < mticks; i++)
		udelay(1024);
	udelay(leftover);
}
#else
static inline void __qdf_udelay(uint32_t usecs)
{
	/* Normal Delay functions. Time specified in microseconds */
	udelay(usecs);
}
#endif

/**
 * __qdf_mdelay() - delay execution for given milliseconds
 * @usecs: Milliseconds to delay
 *
 * Return: none
 */
static inline void __qdf_mdelay(uint32_t msecs)
{
	mdelay(msecs);
}

/**
 * __qdf_system_time_after() - Check if a is later than b
 * @a: Time stamp value a
 * @b: Time stamp value b
 *
 * Return:
 * true if a < b else false
 */
static inline bool __qdf_system_time_after(__qdf_time_t a, __qdf_time_t b)
{
	return (long)(b) - (long)(a) < 0;
}

/**
 * __qdf_system_time_before() - Check if a is before b
 * @a: Time stamp value a
 * @b: Time stamp value b
 *
 * Return:
 * true if a is before b else false
 */
static inline bool __qdf_system_time_before(__qdf_time_t a, __qdf_time_t b)
{
	return __qdf_system_time_after(b, a);
}

/**
 * __qdf_system_time_after_eq() - Check if a atleast as recent as b, if not
 * later
 * @a: Time stamp value a
 * @b: Time stamp value b
 *
 * Return:
 * true if a >= b else false
 */
static inline bool __qdf_system_time_after_eq(__qdf_time_t a, __qdf_time_t b)
{
	return (long)(a) - (long)(b) >= 0;
}

/**
 * qdf_sched_clock() - use light weight timer to get timestamp
 *
 * Return: timestamp in ns
 */
static inline uint64_t __qdf_sched_clock(void)
{
	return sched_clock();
}

/**
 * __qdf_get_monotonic_boottime() - get monotonic kernel boot time
 * This API is similar to qdf_get_system_boottime but it includes
 * time spent in suspend.
 *
 * Return: Time in microseconds
 */
static inline uint64_t __qdf_get_monotonic_boottime(void)
{
	return (uint64_t)ktime_to_us(ktime_get_boottime());
}

#if defined (MSM_PLATFORM)

/**
 * __qdf_get_log_timestamp() - get msm timer ticks
 *
 * Returns QTIMER(19.2 MHz) clock ticks. To convert it into seconds
 * divide it by 19200.
 *
 * Return: QTIMER(19.2 MHz) clock ticks
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
static inline uint64_t __qdf_get_log_timestamp(void)
{
	return __arch_counter_get_cntvct();
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
static inline uint64_t __qdf_get_log_timestamp(void)
{
	return arch_counter_get_cntvct();
}
#else
static inline uint64_t __qdf_get_log_timestamp(void)
{
	return arch_counter_get_cntpct();
}
#endif /* LINUX_VERSION_CODE */
#else

/**
 * __qdf_get_log_timestamp - get time stamp for logging
 *
 * Return: system tick for non MSM platfroms
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))
static inline uint64_t __qdf_get_log_timestamp(void)
{
	struct timespec64 ts;

	ktime_get_ts64(&ts);

	return ((uint64_t)ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}
#else
static inline uint64_t __qdf_get_log_timestamp(void)
{
	struct timespec ts;

	ktime_get_ts(&ts);

	return ((uint64_t) ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}
#endif
#endif

/**
 * __qdf_get_bootbased_boottime_ns() - Get the bootbased time in nanoseconds
 *
 * __qdf_get_bootbased_boottime_ns() function returns the number of nanoseconds
 * that have elapsed since the system was booted. It also includes the time when
 * system was suspended.
 *
 * Return:
 * The time since system booted in nanoseconds
 */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 3, 0))
static inline uint64_t __qdf_get_bootbased_boottime_ns(void)
{
	return ktime_get_boottime_ns();
}

#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0))
static inline uint64_t __qdf_get_bootbased_boottime_ns(void)
{
	return ktime_get_boot_ns();
}

#else
static inline uint64_t __qdf_get_bootbased_boottime_ns(void)
{
	return ktime_to_ns(ktime_get_boottime());
}
#endif

/**
 * __qdf_time_ms_to_ktime() - Converts milliseconds to a ktime object
 * @ms: time in milliseconds
 *
 * Return: milliseconds as ktime object
 */
static inline ktime_t __qdf_time_ms_to_ktime(uint64_t ms)
{
	return ms_to_ktime(ms);
}

/**
 * __qdf_time_ktime_real_get() - Gets the current wall clock as ktime object
 *
 * Return: current wall clock as ktime object
 */
static inline ktime_t __qdf_time_ktime_real_get(void)
{
	return ktime_get_real();
}

/**
 * __qdf_time_sched_clock() - schedule clock
 *
 * Return: returns current time in nanosec units.
 */
static inline unsigned long long __qdf_time_sched_clock(void)
{
	return sched_clock();
}

/**
 * __qdf_time_ktime_sub() - Subtract two ktime objects and returns
 * a ktime object
 * @time1: time as ktime object
 * @time2: time as ktime object
 *
 * Return: subtraction of ktime objects as ktime object
 */
static inline ktime_t __qdf_time_ktime_sub(ktime_t ktime1, ktime_t ktime2)
{
	return ktime_sub(ktime1, ktime2);
}

/**
 * __qdf_time_ktime_set() - Set a ktime_t variable from a seconds/nanoseconds
 * value
 * @secs: seconds to set
 * @nsecs: nanoseconds to set
 *
 * Return: The ktime_t representation of the value.
 */
static inline ktime_t __qdf_time_ktime_set(const s64 secs,
					   const unsigned long nsecs)
{
	return ktime_set(secs, nsecs);
}

/**
 * __qdf_time_ktime_to_us() - Convert the ktime_t object into microseconds
 * @ktime: time as ktime_t object
 *
 * Return: ktime_t in microseconds
 */
static inline int64_t __qdf_time_ktime_to_us(ktime_t ktime)
{
	return ktime_to_us(ktime);
}

/**
 * __qdf_time_ktime_get_real_time() - Get the time of day in qdf_timespec_t
 * @ts: pointer to the qdf_timespec_t to be set
 *
 * Return: none
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
static inline void __qdf_time_ktime_get_real_time(__qdf_timespec_t *ts)
{
	ktime_get_real_ts64(ts);
}
#else
static inline void __qdf_time_ktime_get_real_time(__qdf_timespec_t *ts)
{
	do_gettimeofday(ts);
}
#endif

#endif
