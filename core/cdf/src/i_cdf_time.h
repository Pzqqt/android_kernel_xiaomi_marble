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
 * DOC: i_cdf_time.h
 *
 * Linux specific CDF timing APIs implementation
 */

#ifndef _I_CDF_TIME_H
#define _I_CDF_TIME_H

#include <linux/jiffies.h>
#include <linux/delay.h>
#include <asm/arch_timer.h>
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#endif

typedef unsigned long __cdf_time_t;

/**
 * __cdf_system_ticks() - get system ticks
 *
 * Return: system tick in jiffies
 */
static inline __cdf_time_t __cdf_system_ticks(void)
{
	return jiffies;
}

/**
 * __cdf_system_ticks_to_msecs() - convert system ticks into milli seconds
 * @ticks: System ticks
 *
 * Return: system tick converted into milli seconds
 */
static inline uint32_t __cdf_system_ticks_to_msecs(unsigned long ticks)
{
	return jiffies_to_msecs(ticks);
}

/**
 * __cdf_system_msecs_to_ticks() - convert milli seconds into system ticks
 * @msecs: Milli seconds
 *
 * Return: milli seconds converted into system ticks
 */
static inline __cdf_time_t __cdf_system_msecs_to_ticks(uint32_t msecs)
{
	return msecs_to_jiffies(msecs);
}

/**
 * __cdf_get_system_uptime() - get system uptime
 *
 * Return: system uptime in jiffies
 */
static inline __cdf_time_t __cdf_get_system_uptime(void)
{
	return jiffies;
}

static inline __cdf_time_t __cdf_get_system_timestamp(void)
{
	return (jiffies / HZ) * 1000 + (jiffies % HZ) * (1000 / HZ);
}

/**
 * __cdf_udelay() - delay execution for given microseconds
 * @usecs: Micro seconds to delay
 *
 * Return: none
 */
static inline void __cdf_udelay(uint32_t usecs)
{
#ifdef CONFIG_ARM
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

#else
	/* Normal Delay functions. Time specified in microseconds */
	udelay(usecs);

#endif
}

/**
 * __cdf_mdelay() - delay execution for given milli seconds
 * @usecs: Milli seconds to delay
 *
 * Return: none
 */
static inline void __cdf_mdelay(uint32_t msecs)
{
	mdelay(msecs);
}

/**
 * __cdf_system_time_after() - Check if a is later than b
 * @a: Time stamp value a
 * @b: Time stamp value b
 *
 * Return:
 *	true if a < b else false
 */
static inline bool __cdf_system_time_after(__cdf_time_t a, __cdf_time_t b)
{
	return (long)(b) - (long)(a) < 0;
}

/**
 * __cdf_system_time_before() - Check if a is before b
 * @a: Time stamp value a
 * @b: Time stamp value b
 *
 * Return:
 *	true if a is before b else false
 */
static inline bool __cdf_system_time_before(__cdf_time_t a, __cdf_time_t b)
{
	return __cdf_system_time_after(b, a);
}

/**
 * __cdf_system_time_before() - Check if a atleast as recent as b, if not
 *				later
 * @a: Time stamp value a
 * @b: Time stamp value b
 *
 * Return:
 *	true if a >= b else false
 */
static inline bool __cdf_system_time_after_eq(__cdf_time_t a, __cdf_time_t b)
{
	return (long)(a) - (long)(b) >= 0;
}

/**
 * __cdf_get_monotonic_boottime() - get monotonic kernel boot time
 * This API is similar to cdf_get_system_boottime but it includes
 * time spent in suspend.
 *
 * Return: Time in microseconds
 */
#ifdef CONFIG_CNSS
static inline uint64_t __cdf_get_monotonic_boottime(void)
{
	struct timespec ts;

	cnss_get_monotonic_boottime(&ts);

	return ((uint64_t) ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
}
#else
static inline uint64_t __cdf_get_monotonic_boottime(void)
{
	return __cdf_system_ticks_to_msecs(__cdf_system_ticks()) * 1000;
}
#endif /* CONFIG_CNSS */

#ifdef QCA_WIFI_3_0_ADRASTEA
/**
 * __cdf_get_qtimer_ticks() - get QTIMER ticks
 *
 * Returns QTIMER(19.2 MHz) clock ticks. To convert it into seconds
 * divide it by 19200.
 *
 * Return: QTIMER(19.2 MHz) clock ticks
 */
static inline uint64_t __cdf_get_qtimer_ticks(void)
{
	return arch_counter_get_cntpct();
}
#endif /* QCA_WIFI_3_0_ADRASTEA */

#endif
