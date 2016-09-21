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
 * DOC: i_qdf_time
 * This file provides OS dependent time API's.
 */

#ifndef _I_QDF_TIME_H
#define _I_QDF_TIME_H

#include <linux/jiffies.h>
#include <linux/delay.h>
#ifdef MSM_PLATFORM
#include <asm/arch_timer.h>
#else
#include <linux/ktime.h>
#endif
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#endif

typedef unsigned long __qdf_time_t;

/**
 * __qdf_system_ticks() - get system ticks
 *
 * Return: system tick in jiffies
 */
static inline __qdf_time_t __qdf_system_ticks(void)
{
	return jiffies;
}

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
 * __qdf_get_monotonic_boottime() - get monotonic kernel boot time
 * This API is similar to qdf_get_system_boottime but it includes
 * time spent in suspend.
 *
 * Return: Time in microseconds
 */
static inline uint64_t __qdf_get_monotonic_boottime(void)
{
	struct timespec ts;

	get_monotonic_boottime(&ts);

	return ((uint64_t) ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}

#ifdef QCA_WIFI_3_0_ADRASTEA
#include <asm/arch_timer.h>

/**
 * __qdf_get_log_timestamp() - get QTIMER ticks
 *
 * Returns QTIMER(19.2 MHz) clock ticks. To convert it into seconds
 * divide it by 19200.
 *
 * Return: QTIMER(19.2 MHz) clock ticks
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
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
 * For adrastea this API returns QTIMER tick which is needed to synchronize
 * host and fw log timestamps
 * For ROME and other discrete solution this API returns system boot time stamp
 *
 * Return:
 * QTIMER ticks(19.2MHz) for adrastea
 * System tick for rome and other future discrete solutions
 */
static inline uint64_t __qdf_get_log_timestamp(void)
{
	struct timespec ts;

	ktime_get_ts(&ts);

	return ((uint64_t) ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}
#endif /* QCA_WIFI_3_0_ADRASTEA */

#endif
