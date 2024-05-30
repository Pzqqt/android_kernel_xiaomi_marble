/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_time
 * QCA driver framework (QDF) timer APIs
 */

#include <i_qdf_time.h>
#include "qdf_time.h"
#include <qdf_module.h>

qdf_ktime_t qdf_ns_to_ktime(uint64_t ns)
{
	return __qdf_ns_to_ktime(ns);
}

qdf_export_symbol(qdf_ns_to_ktime);

qdf_ktime_t qdf_ktime_add(qdf_ktime_t ktime1, qdf_ktime_t ktime2)
{
	return __qdf_ktime_add(ktime1, ktime2);
}

qdf_export_symbol(qdf_ktime_add);

qdf_ktime_t qdf_ktime_get(void)
{
	return __qdf_ktime_get();
}

qdf_export_symbol(qdf_ktime_get);

qdf_ktime_t qdf_ktime_real_get(void)
{
	return __qdf_ktime_real_get();
}

qdf_export_symbol(qdf_ktime_real_get);

qdf_ktime_t qdf_ktime_add_ns(qdf_ktime_t ktime, int64_t ns)
{
	return __qdf_ktime_add_ns(ktime, ns);
}

qdf_export_symbol(qdf_ktime_add_ns);

int64_t qdf_ktime_to_ms(qdf_ktime_t ktime)
{
	return __qdf_ktime_to_ms(ktime);
}

qdf_export_symbol(qdf_ktime_to_ms);

int64_t qdf_ktime_to_ns(qdf_ktime_t ktime)
{
	return __qdf_ktime_to_ns(ktime);
}

qdf_export_symbol(qdf_ktime_to_ns);

qdf_time_t qdf_system_ticks(void)
{
	return __qdf_system_ticks();
}

qdf_export_symbol(qdf_system_ticks);

uint32_t qdf_system_ticks_to_msecs(unsigned long clock_ticks)
{
	return __qdf_system_ticks_to_msecs(clock_ticks);
}

qdf_export_symbol(qdf_system_ticks_to_msecs);

qdf_time_t qdf_system_msecs_to_ticks(uint32_t msecs)
{
	return __qdf_system_msecs_to_ticks(msecs);
}

qdf_export_symbol(qdf_system_msecs_to_ticks);

qdf_time_t qdf_get_system_uptime(void)
{
	return __qdf_get_system_uptime();
}

qdf_export_symbol(qdf_get_system_uptime);

uint64_t qdf_get_bootbased_boottime_ns(void)
{
	return __qdf_get_bootbased_boottime_ns();
}

qdf_export_symbol(qdf_get_bootbased_boottime_ns);

unsigned long qdf_get_system_timestamp(void)
{
	return __qdf_get_system_timestamp();
}

qdf_export_symbol(qdf_get_system_timestamp);

void qdf_udelay(int usecs)
{
	__qdf_udelay(usecs);
}

qdf_export_symbol(qdf_udelay);

void qdf_mdelay(int msecs)
{
	__qdf_mdelay(msecs);
}

qdf_export_symbol(qdf_mdelay);

bool qdf_system_time_after(qdf_time_t a, qdf_time_t b)
{
	return __qdf_system_time_after(a, b);
}

qdf_export_symbol(qdf_system_time_after);

bool qdf_system_time_before(qdf_time_t a, qdf_time_t b)
{
	return __qdf_system_time_before(a, b);
}

qdf_export_symbol(qdf_system_time_before);

bool qdf_system_time_after_eq(qdf_time_t a, qdf_time_t b)
{
	return __qdf_system_time_after_eq(a, b);
}

qdf_export_symbol(qdf_system_time_after_eq);

#ifdef MSM_PLATFORM
uint64_t qdf_log_timestamp_to_usecs(uint64_t time)
{
	/*
	 * Try to preserve precision by multiplying by 10 first.
	 * If that would cause a wrap around, divide first instead.
	 */
	if (time * 10 < time) {
		do_div(time, QDF_LOG_TIMESTAMP_CYCLES_PER_10_US);
		return time * 10;
	}

	time = time * 10;
	do_div(time, QDF_LOG_TIMESTAMP_CYCLES_PER_10_US);

	return time;
}

qdf_export_symbol(qdf_log_timestamp_to_usecs);
#else
uint64_t qdf_log_timestamp_to_usecs(uint64_t time)
{
	/* timestamps are already in micro seconds */
	return time;
}

qdf_export_symbol(qdf_log_timestamp_to_usecs);
#endif /* end of MSM_PLATFORM */

void qdf_log_timestamp_to_secs(uint64_t time, uint64_t *secs,
			       uint64_t *usecs)
{
	*secs = qdf_log_timestamp_to_usecs(time);
	*usecs = do_div(*secs, 1000000ul);
}

qdf_export_symbol(qdf_log_timestamp_to_secs);

uint64_t qdf_usecs_to_log_timestamp(uint64_t usecs)
{
	uint64_t ts;

	ts = (usecs * QDF_LOG_TIMESTAMP_CYCLES_PER_10_US);

	return do_div(ts, 10ul);
}

qdf_export_symbol(qdf_usecs_to_log_timestamp);

uint64_t qdf_get_log_timestamp(void)
{
	return __qdf_get_log_timestamp();
}

qdf_export_symbol(qdf_get_log_timestamp);

uint64_t qdf_get_log_timestamp_usecs(void)
{
	return qdf_log_timestamp_to_usecs(qdf_get_log_timestamp());
}

qdf_export_symbol(qdf_get_log_timestamp_usecs);

uint64_t qdf_get_monotonic_boottime(void)
{
	return __qdf_get_monotonic_boottime();
}

qdf_export_symbol(qdf_get_monotonic_boottime);

void qdf_time_ktime_get_real_time(qdf_timespec_t *ts)
{
	return __qdf_time_ktime_get_real_time(ts);
}

qdf_export_symbol(qdf_time_ktime_get_real_time);

unsigned long long qdf_time_sched_clock(void)
{
	return __qdf_time_sched_clock();
}

qdf_export_symbol(qdf_time_sched_clock);
