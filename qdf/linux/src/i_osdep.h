/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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
 *  DOC: i_osdep
 *  QCA driver framework OS dependent types
 */

#ifndef _I_OSDEP_H
#define _I_OSDEP_H

#ifdef CONFIG_MCL
#include <cds_queue.h>
#include <cds_if_upperproto.h>
#else
#include <sys/queue.h>
#endif

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

typedef struct timer_list os_timer_t;

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
			printk(KERN_EMERG " %s:%d unlock addr=%p, %s \n", __func__,  __LINE__, x, \
			       !spin_is_locked(x) ? "Not locked" : "");	\
		} \
		spin_unlock_bh(x); \
	} while (0)
#define spin_trylock(x) spin_trylock_bh(x)
#define OS_SUPPORT_ASYNC_Q 1    /* support for handling asyn function calls */

#else
#define OS_SUPPORT_ASYNC_Q 0
#endif /* ifdef CONFIG_SMP */

/**
 * struct os_mest_t - maintain attributes of message
 * @mesg_next: pointer to the nexgt message
 * @mest_type: type of message
 * @mesg_len: length of the message
 */
typedef struct _os_mesg_t {
	STAILQ_ENTRY(_os_mesg_t) mesg_next;
	uint16_t mesg_type;
	uint16_t mesg_len;
} os_mesg_t;

/**
 * struct qdf_bus_context - Bus to hal context handoff
 * @bc_tag: bus context tag
 * @cal_in_flash: calibration data stored in flash
 * @bc_handle: bus context handle
 * @bc_bustype: bus type
 */
typedef struct qdf_bus_context {
	void *bc_tag;
	int cal_in_flash;
	char *bc_handle;
	enum qdf_bus_type bc_bustype;
} QDF_BUS_CONTEXT;

typedef struct _NIC_DEV *osdev_t;

typedef void (*os_mesg_handler_t)(void *ctx, uint16_t mesg_type,
				  uint16_t mesg_len,
				  void *mesg);


/**
 * typedef os_mesg_queue_t - Object to maintain message queue
 * @dev_handle: OS handle
 * @num_queued: number of queued messages
 * @mesg_len: message length
 * @mesg_queue_buf: pointer to message queue buffer
 * @mesg_head: queued mesg buffers
 * @mesg_free_head: free mesg buffers
 * @lock: spinlock object
 * @ev_handler_lock: spinlock object to event handler
 * @task: pointer to task
 * @_timer: instance of timer
 * @handler: message handler
 * @ctx: pointer to context
 * @is_synchronous: bit to save synchronous status
 * @del_progress: delete in progress
 */
typedef struct {
	osdev_t dev_handle;
	int32_t num_queued;
	int32_t mesg_len;
	uint8_t *mesg_queue_buf;
	STAILQ_HEAD(, _os_mesg_t) mesg_head;
	STAILQ_HEAD(, _os_mesg_t) mesg_free_head;
	spinlock_t lock;
	spinlock_t ev_handler_lock;
#ifdef USE_SOFTINTR
	void *_task;
#else
	os_timer_t _timer;
#endif
	os_mesg_handler_t handler;
	void *ctx;
	uint8_t is_synchronous:1;
	uint8_t del_progress;
} os_mesg_queue_t;

/**
 * struct _NIC_DEV - Definition of OS-dependent device structure.
 * It'll be opaque to the actual ATH layer.
 * @qdf_dev: qdf device
 * @bdev: bus device handle
 * @netdev: net device handle (wifi%d)
 * @intr_tq: tasklet
 * @devstats: net device statistics
 * @bc: hal bus context
 * @device: generic device
 * @event_queue: instance to wait queue
 * @wmi_timeout: init timeout
 * @wmi_timeout_unintr: init timeout (uninterrupted wait)
 * @is_device_asleep: keep device status, sleep or awakei
 * @acfg_event_list: event list
 * @acfg_event_queue_lock: queue lock
 * @acfg_event_os_work: schedule or create work
 * @acfg_netlink_wq_init_done: Work queue ready
 * @osdev_acfg_handle: acfg handle
 */
struct _NIC_DEV {
	qdf_device_t qdf_dev;
	void *bdev;
	struct net_device *netdev;
	qdf_bh_t intr_tq;
	struct net_device_stats devstats;
	QDF_BUS_CONTEXT bc;
#ifdef ATH_PERF_PWR_OFFLOAD
	struct device *device;
	wait_queue_head_t event_queue;
	unsigned int wmi_timeout;
	unsigned int wmi_timeout_unintr;
#endif /* PERF_PWR_OFFLOAD */
#if OS_SUPPORT_ASYNC_Q
	os_mesg_queue_t async_q;
#endif
#ifdef ATH_BUS_PM
	uint8_t is_device_asleep;
#endif /* ATH_BUS_PM */
	qdf_nbuf_queue_t acfg_event_list;
	qdf_spinlock_t acfg_event_queue_lock;
	qdf_work_t acfg_event_os_work;
	uint8_t acfg_netlink_wq_init_done;

#ifdef UMAC_SUPPORT_ACFG
#ifdef ACFG_NETLINK_TX
	void *osdev_acfg_handle;
#endif /* ACFG_NETLINK_TX */
#endif /* UMAC_SUPPORT_ACFG */

};

#define __QDF_SYSCTL_PROC_DOINTVEC(ctl, write, filp, buffer, lenp, ppos) \
	proc_dointvec(ctl, write, buffer, lenp, ppos)

#define __QDF_SYSCTL_PROC_DOSTRING(ctl, write, filp, buffer, lenp, ppos) \
	proc_dostring(ctl, write, filp, buffer, lenp, ppos)

#endif /* _I_OSDEP_H */
