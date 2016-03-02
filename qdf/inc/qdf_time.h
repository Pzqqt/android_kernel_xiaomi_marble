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
 * DOC: cdf_time
 * This file abstracts time related functionality.
 */

#ifndef _CDF_OS_TIME_H
#define _CDF_OS_TIME_H

#include <i_cdf_time.h>
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#endif

typedef __cdf_time_t cdf_time_t;

/**
 * cdf_system_ticks() - Count the number of ticks elapsed from the time when
 *			the system booted
 *
 * Return: ticks
 */
static inline unsigned long cdf_system_ticks(void)
{
	return __cdf_system_ticks();
}

/**
 * cdf_system_ticks_to_msecs() - convert ticks to milliseconds
 * @clock_ticks: Number of ticks
 *
 * Return: Time in milliseconds
 */
static inline uint32_t cdf_system_ticks_to_msecs(unsigned long clock_ticks)
{
	return __cdf_system_ticks_to_msecs(clock_ticks);
}

/**
 * cdf_system_msecs_to_ticks() - convert milliseconds to ticks
 * @msec: Time in milliseconds
 *
 * Return: number of ticks
 */
static inline unsigned long cdf_system_msecs_to_ticks(uint32_t msecs)
{
	return __cdf_system_msecs_to_ticks(msecs);
}

/**
 * cdf_get_system_uptime() - Return a monotonically increasing time.
 * This increments once per HZ ticks
 *
 * Return: system up time
 */
static inline unsigned long cdf_get_system_uptime(void)
{
	return __cdf_get_system_uptime();
}

/**
 * cdf_get_system_timestamp() - brief Return current timestamp
 *
 * Return: none
 */
static inline unsigned long cdf_get_system_timestamp(void)
{
	return __cdf_get_system_timestamp();
}

/**
 * cdf_udelay() - delay in microseconds
 * @usecs: Number of microseconds to delay
 *
 * Return: none
 */
static inline void cdf_udelay(int usecs)
{
	__cdf_udelay(usecs);
}

/**
 * cdf_mdelay() - Delay in milliseconds.
 * @msec: Number of milliseconds to delay
 *
 * Return: none
 */
static inline void cdf_mdelay(int msecs)
{
	__cdf_mdelay(msecs);
}

/* Check if _a is later than _b */
#define cdf_system_time_after(_a, _b)       __cdf_system_time_after(_a, _b)

/* Check if _a is prior to _b */
#define cdf_system_time_before(_a, _b)      __cdf_system_time_before(_a, _b)

/* Check if _a atleast as recent as _b, if not later */
#define cdf_system_time_after_eq(_a, _b)    __cdf_system_time_after_eq(_a, _b)

/**
 * enum cdf_timestamp_unit - what unit the cdf timestamp is in
 * @KERNEL_LOG: boottime time in uS (micro seconds)
 * @KERNEL_LOG: QTIME in (1/19200)S
 *
 * This enum is used to distinguish which timer source is used.
 */
enum cdf_timestamp_unit {
	KERNEL_LOG,
	QTIMER,
};

#ifdef QCA_WIFI_3_0_ADRASTEA
#define CDF_LOG_TIMESTAMP_UNIT QTIMER
#else
#define CDF_LOG_TIMESTAMP_UNIT KERNEL_LOG
#endif

#ifdef QCA_WIFI_3_0_ADRASTEA
/**
 * cdf_get_log_timestamp() - get time stamp for logging
 *
 * For adrastea this API returns QTIMER tick which is needed to synchronize
 * host and fw log timestamps
 *
 * For ROME and other discrete solution this API returns system boot time stamp
 *
 * Return:
 *	QTIMER ticks(19.2MHz) for adrastea
 *	System tick for rome and other future discrete solutions
 */
static inline uint64_t cdf_get_log_timestamp(void)
{
	return __cdf_get_qtimer_ticks();
}
#else
/**
 * cdf_get_log_timestamp() - get time stamp for logging
 *
 * For adrastea this API returns QTIMER tick which is needed to synchronize
 * host and fw log timestamps
 *
 * For ROME and other discrete solution this API returns system boot time stamp
 *
 * Return:
 *	QTIMER ticks(19.2MHz) for adrastea
 *	System tick for rome and other future discrete solutions
 */
static inline uint64_t cdf_get_log_timestamp(void)
{
#ifdef CONFIG_CNSS
	struct timespec ts;

	cnss_get_boottime(&ts);

	return ((uint64_t) ts.tv_sec * 1000000) + (ts.tv_nsec / 1000);
#else
	return cdf_system_ticks_to_msecs(cdf_system_ticks()) * 1000;
#endif /* CONFIG_CNSS */
}
#endif /* QCA_WIFI_3_0_ADRASTEA */

/**
 * cdf_get_monotonic_boottime() - get monotonic kernel boot time
 * This API is similar to cdf_get_system_boottime but it includes
 * time spent in suspend.
 *
 * Return: Time in microseconds
 */
static inline uint64_t cdf_get_monotonic_boottime(void)
{
	return __cdf_get_monotonic_boottime();
}

#endif
