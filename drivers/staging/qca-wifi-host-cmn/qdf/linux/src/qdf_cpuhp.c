/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_cpuhp
 * This file provides OS dependent QDF CPU hotplug APIs
 */

#include "i_qdf_cpuhp.h"
#include "qdf_trace.h"
#include "linux/cpu.h"
#include "linux/notifier.h"
#include "linux/version.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0)
#include "linux/cpuhotplug.h"
#endif

static __qdf_cpuhp_emit __qdf_cpuhp_on_up;
static __qdf_cpuhp_emit __qdf_cpuhp_on_down;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0)
static int qdf_cpuhp_legacy_handler(struct notifier_block *block,
				    unsigned long state,
				    void *hcpu)
{
	unsigned long cpu = (unsigned long)hcpu;

	switch (state) {
	case CPU_ONLINE:
		__qdf_cpuhp_on_up(cpu);
		break;

	case CPU_DOWN_PREPARE:
	case CPU_DOWN_PREPARE_FROZEN:
		__qdf_cpuhp_on_down(cpu);
		break;

	default:
		break;
	}

	return NOTIFY_OK;
}

static struct notifier_block qdf_cpuhp_notifier_block = {
	.notifier_call = qdf_cpuhp_legacy_handler,
};

static inline void qdf_cpuhp_register_callbacks(void)
{
	register_hotcpu_notifier(&qdf_cpuhp_notifier_block);
}

static inline void qdf_cpuhp_unregister_callbacks(void)
{
	unregister_hotcpu_notifier(&qdf_cpuhp_notifier_block);
}
#else
static enum cpuhp_state registered_hotplug_state;

static int qdf_cpuhp_up_handler(unsigned int cpu)
{
	__qdf_cpuhp_on_up(cpu);

	return 0;
}

static int qdf_cpuhp_down_handler(unsigned int cpu)
{
	__qdf_cpuhp_on_down(cpu);

	return 0;
}

static inline void qdf_cpuhp_register_callbacks(void)
{
	registered_hotplug_state = cpuhp_setup_state(CPUHP_AP_ONLINE_DYN,
						     "wlan/qca-qdf:online",
						     qdf_cpuhp_up_handler,
						     qdf_cpuhp_down_handler);
}

static inline void qdf_cpuhp_unregister_callbacks(void)
{
	QDF_BUG(registered_hotplug_state);
	if (registered_hotplug_state)
		cpuhp_remove_state(registered_hotplug_state);
}
#endif /* KERNEL_VERSION(4, 6, 0) */

void __qdf_cpuhp_os_init(__qdf_cpuhp_emit on_up, __qdf_cpuhp_emit on_down)
{
	__qdf_cpuhp_on_up = on_up;
	__qdf_cpuhp_on_down = on_down;

	qdf_cpuhp_register_callbacks();
}

void __qdf_cpuhp_os_deinit(void)
{
	qdf_cpuhp_unregister_callbacks();
}

