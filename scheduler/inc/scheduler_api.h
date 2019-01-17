/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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

#if !defined(__SCHEDULER_API_H)
#define __SCHEDULER_API_H

#include <qdf_event.h>
#include <qdf_types.h>
#include <qdf_lock.h>
#include <qdf_mc_timer.h>
#include <qdf_status.h>

/* Controller thread various event masks
 * MC_POST_EVENT_MASK: wake up thread after posting message
 * MC_SUSPEND_EVENT_MASK: signal thread to suspend during kernel pm suspend
 * MC_SHUTDOWN_EVENT_MASK: signal thread to shutdown and exit during unload
 */
#define MC_POST_EVENT_MASK               0x001
#define MC_SUSPEND_EVENT_MASK            0x002
#define MC_SHUTDOWN_EVENT_MASK           0x010

/*
 * Cookie for timer messages.  Note that anyone posting a timer message
 * has to write the COOKIE in the reserved field of the message.  The
 * timer queue handler relies on this COOKIE
 */
#define SYS_MSG_COOKIE      0xFACE

#define scheduler_get_src_id(qid)       (((qid) >> 20) & 0x3FF)
#define scheduler_get_dest_id(qid)      (((qid) >> 10) & 0x3FF)
#define scheduler_get_que_id(qid)       ((qid) & 0x3FF)
#define scheduler_get_qid(src, dest, que_id)    ((que_id) | ((dest) << 10) |\
					     ((src) << 20))

typedef enum {
	SYS_MSG_ID_MC_TIMER,
	SYS_MSG_ID_FTM_RSP,
	SYS_MSG_ID_QVIT,
	SYS_MSG_ID_DATA_STALL_MSG,
	SYS_MSG_ID_UMAC_STOP,
} SYS_MSG_ID;

/**
 * struct scheduler_msg: scheduler message structure
 * @type: message type
 * @reserved: reserved field
 * @bodyval: message body val
 * @bodyptr: message body pointer based on the type either a bodyptr pointer
 *     into memory or bodyval as a 32 bit data is used. bodyptr is always a
 *     freeable pointer, one should always make sure that bodyptr is always
 *     freeable.
 * Messages should use either bodyptr or bodyval; not both !!!
 * @callback: callback to be called by scheduler thread once message is posted
 *   and scheduler thread has started processing the message.
 * @flush_callback: flush callback which will be invoked during driver unload
 *   such that component can release the ref count of common global objects
 *   like PSOC, PDEV, VDEV and PEER. A component needs to populate flush
 *   callback in message body pointer for those messages which have taken ref
 *   count for above mentioned common objects.
 * @node: list node for queue membership
 * @queue_id: Id of the queue the message was added to
 * @queue_depth: depth of the queue when the message was queued
 * @queued_at_us: timestamp when the message was queued in microseconds
 */
struct scheduler_msg {
	uint16_t type;
	uint16_t reserved;
	uint32_t bodyval;
	void *bodyptr;
	void *callback;
	void *flush_callback;
	qdf_list_node_t node;
#ifdef WLAN_SCHED_HISTORY_SIZE
	QDF_MODULE_ID queue_id;
	uint32_t queue_depth;
	uint64_t queued_at_us;
#endif /* WLAN_SCHED_HISTORY_SIZE */
};

typedef QDF_STATUS (*scheduler_msg_process_fn_t) (struct scheduler_msg  *msg);
typedef void (*hdd_suspend_callback)(void);

/**
 * scheduler_init() - initialize control path scheduler
 *
 * This API initializes control path scheduler.
 *
 * Return: QDF status
 */
QDF_STATUS scheduler_init(void);

/**
 * scheduler_deinit() - de-initialize control path scheduler
 *
 * This API de-initializes control path scheduler.
 *
 * Return: QDF status
 */
QDF_STATUS scheduler_deinit(void);

/**
 * scheduler_enable() - start the scheduler module
 *
 * Ready the scheduler module to service requests, and start the scheduler's
 * message processing thread. Must only be called after scheduler_init().
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scheduler_enable(void);

/**
 * scheduler_disable() - stop the scheduler module
 *
 * Stop the scheduler module from servicing requests, and terminate the
 * scheduler's message processing thread. Must be called before
 * scheduler_deinit().
 *
 * Return: QDF_STATUS
 */
QDF_STATUS scheduler_disable(void);

/**
 * scheduler_register_module() - register input module/queue id
 * @qid: queue id to get registered
 * @callback: queue message to be called when a message is posted
 *
 * Return: QDF status
 */
QDF_STATUS scheduler_register_module(QDF_MODULE_ID qid,
		scheduler_msg_process_fn_t callback);

/**
 * scheduler_deregister_module() - deregister input module/queue id
 * @qid: queue id to get deregistered
 *
 * Return: QDF status
 */
QDF_STATUS scheduler_deregister_module(QDF_MODULE_ID qid);

/**
 * scheduler_post_msg_by_priority() - post messages by priority
 * @qid: queue id to which the message has to be posted.
 * @msg: message pointer
 * @is_high_priority: set to true for high priority message else false
 *
 * Return: QDF status
 */
QDF_STATUS scheduler_post_msg_by_priority(uint32_t qid,
					  struct scheduler_msg *msg,
					  bool is_high_priority);

/**
 * scheduler_post_msg() - post normal messages(no priority)
 * @qid: queue id to which the message has to be posted.
 * @msg: message pointer
 *
 * Return: QDF status
 */
static inline QDF_STATUS scheduler_post_msg(uint32_t qid,
					    struct scheduler_msg *msg)
{
	return scheduler_post_msg_by_priority(qid, msg, false);
}

/**
 * scheduler_post_message() - post normal messages(no priority)
 * @src_id: Source module of the message
 * @dest_id: Destination module of the message
 * @que_id: Queue to which the message has to posted.
 * @msg: message pointer
 *
 * This function will mask the src_id, and destination id to qid of
 * scheduler_post_msg
 * Return: QDF status
 */
QDF_STATUS scheduler_post_message_debug(QDF_MODULE_ID src_id,
					QDF_MODULE_ID dest_id,
					QDF_MODULE_ID que_id,
					struct scheduler_msg *msg,
					int line,
					const char *func);

#define scheduler_post_message(src_id, dest_id, que_id, msg) \
	scheduler_post_message_debug(src_id, dest_id, que_id, msg, \
				     __LINE__, __func__)

/**
 * scheduler_resume() - resume scheduler thread
 *
 * Complete scheduler thread resume wait event such that scheduler
 * thread can wake up and process message queues
 *
 * Return: none
 */
void scheduler_resume(void);

/**
 * scheduler_register_hdd_suspend_callback() - suspend callback to hdd
 * @callback: hdd callback to be called when controllred thread is suspended
 *
 * Return: none
 */
void scheduler_register_hdd_suspend_callback(hdd_suspend_callback callback);

/**
 * scheduler_wake_up_controller_thread() - wake up controller thread
 *
 * Wake up controller thread to process a critical message.
 *
 * Return: none
 */
void scheduler_wake_up_controller_thread(void);

/**
 * scheduler_set_event_mask() - set given event mask
 * @event_mask: event mask to set
 *
 * Set given event mask such that controller scheduler thread can do
 * specified work after wake up.
 *
 * Return: none
 */
void scheduler_set_event_mask(uint32_t event_mask);

/**
 * scheduler_clear_event_mask() - clear given event mask
 * @event_mask: event mask to set
 *
 * Return: none
 */
void scheduler_clear_event_mask(uint32_t event_mask);

/**
 * scheduler_target_if_mq_handler() - top level message queue handler for
 *                                    target_if message queue
 * @msg: pointer to actual message being handled
 *
 * Return: none
 */
QDF_STATUS scheduler_target_if_mq_handler(struct scheduler_msg *msg);

/**
 * scheduler_os_if_mq_handler() - top level message queue handler for
 *                                os_if message queue
 * @msg: pointer to actual message being handled
 *
 * Return: none
 */
QDF_STATUS scheduler_os_if_mq_handler(struct scheduler_msg *msg);

/**
 * scheduler_timer_q_mq_handler() - top level message queue handler for
 *                                timer queue
 * @msg: pointer to actual message being handled
 *
 * Return: none
 */
QDF_STATUS scheduler_timer_q_mq_handler(struct scheduler_msg *msg);

/**
 * scheduler_mlme_mq_handler() - top level message queue handler for
 *                               mlme queue
 * @msg: pointer to actual message being handled
 *
 * Return: QDF status
 */
QDF_STATUS scheduler_mlme_mq_handler(struct scheduler_msg *msg);

/**
 * scheduler_scan_mq_handler() - top level message queue handler for
 *                               scan queue
 * @msg: pointer to actual message being handled
 *
 * Return: QDF status
 */
QDF_STATUS scheduler_scan_mq_handler(struct scheduler_msg *msg);

/**
 * scheduler_register_wma_legacy_handler() - register legacy wma handler
 * @callback: legacy wma handler to be called for WMA messages
 *
 * Return: QDF status
 */
QDF_STATUS scheduler_register_wma_legacy_handler(scheduler_msg_process_fn_t
						callback);

/**
 * scheduler_register_sys_legacy_handler() - register legacy sys handler
 * @callback: legacy sys handler to be called for sys messages
 *
 * Return: QDF status
 */
QDF_STATUS scheduler_register_sys_legacy_handler(scheduler_msg_process_fn_t
						callback);
/**
 * scheduler_deregister_sys_legacy_handler() - deregister legacy sys handler
 *
 * Return: QDF status
 */
QDF_STATUS scheduler_deregister_sys_legacy_handler(void);

/**
 * scheduler_deregister_wma_legacy_handler() - deregister legacy wma handler
 *
 * Return: QDF status
 */
QDF_STATUS scheduler_deregister_wma_legacy_handler(void);

/**
 * scheduler_mc_timer_callback() - timer callback, gets called at time out
 * @timer: holds the mc timer object.
 *
 * Return: None
 */
void scheduler_mc_timer_callback(qdf_mc_timer_t *timer);

/**
 * scheduler_get_queue_size() - Get the current size of the scheduler queue
 * @qid: Queue ID for which the size is requested
 * @size: Pointer to size where the size would be returned to the caller
 *
 * This API finds the size of the scheduler queue for the given Queue ID
 *
 * Return: QDF Status
 */
QDF_STATUS scheduler_get_queue_size(QDF_MODULE_ID qid, uint32_t *size);
#endif
