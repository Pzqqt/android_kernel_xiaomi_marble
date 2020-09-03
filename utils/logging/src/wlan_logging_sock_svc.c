/*
 * Copyright (c) 2014-2020 The Linux Foundation. All rights reserved.
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

/******************************************************************************
* wlan_logging_sock_svc.c
*
******************************************************************************/

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
#include <linux/vmalloc.h>
#include <wlan_logging_sock_svc.h>
#include <linux/kthread.h>
#include <qdf_time.h>
#include <qdf_trace.h>
#include <qdf_mc_timer.h>
#include <qdf_timer.h>
#include <qdf_lock.h>
#include <wlan_ptt_sock_svc.h>
#include <host_diag_core_event.h>
#include "host_diag_core_log.h"
#include <qdf_event.h>
#include <qdf_module.h>

#ifdef CNSS_GENL
#include <net/cnss_nl.h>
#endif

#if defined(FEATURE_FW_LOG_PARSING) || defined(FEATURE_WLAN_DIAG_SUPPORT) || \
	defined(FEATURE_PKTLOG)
#include <cds_api.h>
#include "ani_global.h"
#endif

#ifdef FEATURE_PKTLOG
#ifndef REMOVE_PKT_LOG
#include "wma.h"
#include "pktlog_ac.h"
#include <cdp_txrx_misc.h>
#endif
#endif

#define MAX_NUM_PKT_LOG 32

#define LOGGING_TRACE(level, args ...) \
	QDF_TRACE(QDF_MODULE_ID_HDD, level, ## args)

/* Global variables */

#define ANI_NL_MSG_LOG_TYPE 89
#define ANI_NL_MSG_READY_IND_TYPE 90
#ifndef MAX_LOGMSG_COUNT
#define MAX_LOGMSG_COUNT 256
#endif
#define MAX_LOGMSG_LENGTH 2048
#define MAX_SKBMSG_LENGTH 4096

#define WLAN_LOG_BUFFER_SIZE 2048
#if defined(FEATURE_PKTLOG) && !defined(REMOVE_PKT_LOG)
/**
 * Buffer to accommodate -
 * pktlog buffer (2048 bytes)
 * ath_pktlog_hdr (16 bytes)
 * pkt_dump (8 bytes)
 * extra padding (40 bytes)
 *
 * Note: pktlog buffer size is dependent on RX_BUFFER_SIZE and
 * HTT_T2H_MAX_MSG_SIZE. Adjust WLAN_LOG_BUFFER_SIZE
 * based on the above mentioned macros.
 */
#define ATH_PKTLOG_HDR_SIZE (sizeof(struct ath_pktlog_hdr))
#define PKT_DUMP_HDR_SIZE (sizeof(struct packet_dump))
#define EXTRA_PADDING 40

#define MAX_PKTSTATS_LENGTH \
	((WLAN_LOG_BUFFER_SIZE) + (ATH_PKTLOG_HDR_SIZE) + \
	 (PKT_DUMP_HDR_SIZE) + (EXTRA_PADDING))
#else
#define MAX_PKTSTATS_LENGTH WLAN_LOG_BUFFER_SIZE
#endif /* FEATURE_PKTLOG */

#define MAX_PKTSTATS_BUFF   16
#define HOST_LOG_DRIVER_MSG        0x001
#define HOST_LOG_PER_PKT_STATS     0x002
#define HOST_LOG_FW_FLUSH_COMPLETE 0x003
#define DIAG_TYPE_LOGS                 1
#define PTT_MSG_DIAG_CMDS_TYPE    0x5050
#define MAX_LOG_LINE 500

/* default rate limit period - 2sec */
#define PANIC_WIFILOG_PRINT_RATE_LIMIT_PERIOD (2*HZ)
/* default burst for rate limit */
#define PANIC_WIFILOG_PRINT_RATE_LIMIT_BURST_DEFAULT 250
DEFINE_RATELIMIT_STATE(panic_wifilog_ratelimit,
		       PANIC_WIFILOG_PRINT_RATE_LIMIT_PERIOD,
		       PANIC_WIFILOG_PRINT_RATE_LIMIT_BURST_DEFAULT);

struct log_msg {
	struct list_head node;
	unsigned int radio;
	unsigned int index;
	/* indicates the current filled log length in logbuf */
	unsigned int filled_length;
	/*
	 * Buf to hold the log msg
	 * tAniHdr + log
	 */
	char logbuf[MAX_LOGMSG_LENGTH];
};

/**
 * struct packet_dump - This data structure contains the
 * Tx/Rx packet stats
 * @status: Status
 * @type: Type
 * @driver_ts: driver timestamp
 * @fw_ts: fw timestamp
 */
struct packet_dump {
	unsigned char status;
	unsigned char type;
	uint32_t driver_ts;
	uint16_t fw_ts;
} __attribute__((__packed__));

/**
 * struct pkt_stats_msg - This data structure contains the
 * pkt stats node for link list
 * @node: LinkList node
 * @node: Pointer to skb
 */
struct pkt_stats_msg {
	struct list_head node;
	struct sk_buff *skb;
};

#define MAX_FLUSH_TIMER_PERIOD_VALUE 3600000 /* maximum of 1 hour (in ms) */
struct wlan_logging {
	/* Console log levels */
	uint32_t console_log_levels;
	/* Number of buffers to be used for logging */
	uint32_t num_buf;
	uint32_t buffer_length;
	/* Lock to synchronize access to shared logging resource */
	spinlock_t spin_lock;
	/* Holds the free node which can be used for filling logs */
	struct list_head free_list;
	/* Holds the filled nodes which needs to be indicated to APP */
	struct list_head filled_list;
	/* Holds nodes for console printing in case of kernel panic */
	struct list_head panic_list;
	/* Wait queue for Logger thread */
	wait_queue_head_t wait_queue;
	/* Logger thread */
	struct task_struct *thread;
	/* Logging thread sets this variable on exit */
	struct completion shutdown_comp;
	/* Indicates to logger thread to exit */
	bool exit;
	/* Holds number of dropped logs */
	unsigned int drop_count;
	/* current logbuf to which the log will be filled to */
	struct log_msg *pcur_node;
	/* Event flag used for wakeup and post indication*/
	unsigned long eventFlag;
	/* Indicates logger thread is activated */
	bool is_active;
	/* Flush completion check */
	bool is_flush_complete;
	/* paramaters  for pkt stats */
	struct list_head pkt_stat_free_list;
	struct list_head pkt_stat_filled_list;
	struct pkt_stats_msg *pkt_stats_pcur_node;
	unsigned int pkt_stat_drop_cnt;
	spinlock_t pkt_stats_lock;
	unsigned int pkt_stats_msg_idx;
	qdf_timer_t flush_timer;
	bool is_flush_timer_initialized;
	uint32_t flush_timer_period;
	qdf_spinlock_t flush_timer_lock;
};

/* This global variable is intentionally not marked static because it
 * is used by offline tools. Please do not use it outside this file.
 */
struct wlan_logging gwlan_logging;
static struct pkt_stats_msg *gpkt_stats_buffers;

#ifdef WLAN_LOGGING_BUFFERS_DYNAMICALLY

static struct log_msg *gplog_msg;

static inline QDF_STATUS allocate_log_msg_buffer(void)
{
	gplog_msg = vzalloc(MAX_LOGMSG_COUNT * sizeof(*gplog_msg));

	return gplog_msg ? QDF_STATUS_SUCCESS : QDF_STATUS_E_NOMEM;
}

static inline void free_log_msg_buffer(void)
{
	vfree(gplog_msg);
	gplog_msg = NULL;
}

#else
static struct log_msg gplog_msg[MAX_LOGMSG_COUNT];

static inline QDF_STATUS allocate_log_msg_buffer(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline void free_log_msg_buffer(void)
{ }
#endif

/* Need to call this with spin_lock acquired */
static int wlan_queue_logmsg_for_app(void)
{
	char *ptr;
	int ret = 0;
	ptr = &gwlan_logging.pcur_node->logbuf[sizeof(tAniHdr)];
	ptr[gwlan_logging.pcur_node->filled_length] = '\0';

	*(unsigned short *)(gwlan_logging.pcur_node->logbuf) =
		ANI_NL_MSG_LOG_TYPE;
	*(unsigned short *)(gwlan_logging.pcur_node->logbuf + 2) =
		gwlan_logging.pcur_node->filled_length;
	list_add_tail(&gwlan_logging.pcur_node->node,
		      &gwlan_logging.filled_list);

	if (!list_empty(&gwlan_logging.free_list)) {
		/* Get buffer from free list */
		gwlan_logging.pcur_node =
			(struct log_msg *)(gwlan_logging.free_list.next);
		list_del_init(gwlan_logging.free_list.next);
	} else if (!list_empty(&gwlan_logging.filled_list)) {
		/* Get buffer from filled list */
		/* This condition will drop the packet from being
		 * indicated to app
		 */
		gwlan_logging.pcur_node =
			(struct log_msg *)(gwlan_logging.filled_list.next);
		++gwlan_logging.drop_count;
		list_del_init(gwlan_logging.filled_list.next);
		ret = 1;
	}

	/* Reset the current node values */
	gwlan_logging.pcur_node->filled_length = 0;
	return ret;
}

static const char *current_process_name(void)
{
	if (in_irq())
		return "irq";

	if (in_softirq())
		return "soft_irq";

	return current->comm;
}

/**
 * wlan_add_user_log_time_stamp() - populate firmware and kernel timestamps
 * @tbuf: Pointer to time stamp buffer
 * @tbuf_sz: Time buffer size
 * @ts: Time stamp value
 *
 * For adrastea time stamp is QTIMER raw tick which will be used by cnss_diag
 * to convert it into user visible time stamp. In adrstea FW also uses QTIMER
 * raw ticks which is needed to synchronize host and fw log time stamps
 *
 * Also add logcat timestamp so that driver logs and
 * logcat logs can be co-related
 *
 * For discrete solution e.g rome use system tick and convert it into
 * seconds.milli seconds
 *
 * Return: number of characters written in target buffer not including
 *		trailing '/0'
 */
static int wlan_add_user_log_time_stamp(char *tbuf, size_t tbuf_sz, uint64_t ts)
{
	char time_buf[20];

	qdf_get_time_of_the_day_in_hr_min_sec_usec(time_buf, sizeof(time_buf));

	return scnprintf(tbuf, tbuf_sz, "[%.6s][0x%llx]%s",
			 current_process_name(), (unsigned long long)ts,
			 time_buf);
}

#ifdef WLAN_MAX_LOGS_PER_SEC
static qdf_time_t __log_window_end_ticks;
static qdf_atomic_t __log_window_count;

/**
 * assert_on_excessive_logging() - Check for and panic on excessive logging
 *
 * Track logging count using a quasi-tumbling window, 1 second long. If the max
 * logging count for a given window is exceeded, panic.
 *
 * Return: None
 */
static void assert_on_excessive_logging(void)
{
	qdf_time_t now = qdf_system_ticks();

	/*
	 * If 'now' is more recent than the end of the window, reset.
	 *
	 * Note: This is not thread safe, and can result in more than one reset.
	 * For our purposes, this is fine.
	 */
	if (!qdf_atomic_read(&__log_window_count)) {
		__log_window_end_ticks = now + qdf_system_ticks_per_sec;
	} else if (qdf_system_time_after(now, __log_window_end_ticks)) {
		__log_window_end_ticks = now + qdf_system_ticks_per_sec;
		qdf_atomic_set(&__log_window_count, 0);
	}

	/* this _is_ thread safe, and results in at most one panic */
	if (qdf_atomic_inc_return(&__log_window_count) == WLAN_MAX_LOGS_PER_SEC)
		QDF_DEBUG_PANIC("Exceeded %d logs per second",
				WLAN_MAX_LOGS_PER_SEC);
}
#else
static inline void assert_on_excessive_logging(void) { }
#endif /* WLAN_MAX_LOGS_PER_SEC */

static inline void
log_to_console(QDF_TRACE_LEVEL level, const char *timestamp, const char *msg)
{
	switch (level) {
	case QDF_TRACE_LEVEL_FATAL:
		pr_alert("%s %s\n", timestamp, msg);
		assert_on_excessive_logging();
		break;
	case QDF_TRACE_LEVEL_ERROR:
		pr_err("%s %s\n", timestamp, msg);
		assert_on_excessive_logging();
		break;
	case QDF_TRACE_LEVEL_WARN:
		pr_warn("%s %s\n", timestamp, msg);
		assert_on_excessive_logging();
		break;
	case QDF_TRACE_LEVEL_INFO:
		pr_info("%s %s\n", timestamp, msg);
		assert_on_excessive_logging();
		break;
	case QDF_TRACE_LEVEL_INFO_HIGH:
	case QDF_TRACE_LEVEL_INFO_MED:
	case QDF_TRACE_LEVEL_INFO_LOW:
	case QDF_TRACE_LEVEL_DEBUG:
	default:
		/* these levels should not be logged to console */
		break;
	}
}

int wlan_log_to_user(QDF_TRACE_LEVEL log_level, char *to_be_sent, int length)
{
	char *ptr;
	char tbuf[60];
	int tlen;
	int total_log_len;
	unsigned int *pfilled_length;
	bool wake_up_thread = false;
	unsigned long flags;
	uint64_t ts;

	/* Add the current time stamp */
	ts = qdf_get_log_timestamp();
	tlen = wlan_add_user_log_time_stamp(tbuf, sizeof(tbuf), ts);

	/* if logging isn't up yet, just dump to dmesg */
	if (!gwlan_logging.is_active) {
		log_to_console(log_level, tbuf, to_be_sent);
		return 0;
	}

	/* 1+1 indicate '\n'+'\0' */
	total_log_len = length + tlen + 1 + 1;

	spin_lock_irqsave(&gwlan_logging.spin_lock, flags);
	/* wlan logging svc resources are not yet initialized */
	if (!gwlan_logging.pcur_node) {
		spin_unlock_irqrestore(&gwlan_logging.spin_lock, flags);
		return -EIO;
	}

	pfilled_length = &gwlan_logging.pcur_node->filled_length;

	/* Check if we can accommodate more log into current node/buffer */
	if ((MAX_LOGMSG_LENGTH - (*pfilled_length +
			sizeof(tAniNlHdr))) < total_log_len) {
		wake_up_thread = true;
		wlan_queue_logmsg_for_app();
		pfilled_length = &gwlan_logging.pcur_node->filled_length;
	}

	ptr = &gwlan_logging.pcur_node->logbuf[sizeof(tAniHdr)];

	if (unlikely(MAX_LOGMSG_LENGTH < (sizeof(tAniNlHdr) + total_log_len))) {
		/*
		 * Assumption here is that we receive logs which is less than
		 * MAX_LOGMSG_LENGTH, where we can accommodate the
		 * tAniNlHdr + [context][timestamp] + log
		 * If log length is over MAX_LOGMSG_LENGTH,
		 * the overflow part will be discarded.
		 */
		length = MAX_LOGMSG_LENGTH - sizeof(tAniNlHdr) - tlen - 2;
		/*
		 * QDF_ASSERT if complete log was not accommodated into
		 * the available buffer.
		 */
		QDF_ASSERT(0);
	}

	memcpy(&ptr[*pfilled_length], tbuf, tlen);
	memcpy(&ptr[*pfilled_length + tlen], to_be_sent, length);
	*pfilled_length += tlen + length;
	ptr[*pfilled_length] = '\n';
	*pfilled_length += 1;

	spin_unlock_irqrestore(&gwlan_logging.spin_lock, flags);

	/* Wakeup logger thread */
	if (wake_up_thread) {
		set_bit(HOST_LOG_DRIVER_MSG, &gwlan_logging.eventFlag);
		wake_up_interruptible(&gwlan_logging.wait_queue);
	}

	if (gwlan_logging.console_log_levels & BIT(log_level))
		log_to_console(log_level, tbuf, to_be_sent);

	return 0;
}

/**
 * nl_srv_bcast_host_logs() - Wrapper to send bcast msgs to host logs mcast grp
 * @skb: sk buffer pointer
 *
 * Sends the bcast message to host logs multicast group with generic nl socket
 * if CNSS_GENL is enabled. Else, use the legacy netlink socket to send.
 *
 * Return: zero on success, error code otherwise
 */
#ifdef CNSS_GENL
static int nl_srv_bcast_host_logs(struct sk_buff *skb)
{
	return nl_srv_bcast(skb, CLD80211_MCGRP_HOST_LOGS, ANI_NL_MSG_LOG);
}
#else
static int nl_srv_bcast_host_logs(struct sk_buff *skb)
{
	return nl_srv_bcast(skb);
}
#endif

#ifndef REMOVE_PKT_LOG
/**
 * pkt_stats_fill_headers() - This function adds headers to skb
 * @skb: skb to which headers need to be added
 *
 * Return: 0 on success or Errno on failure
 */
static int pkt_stats_fill_headers(struct sk_buff *skb)
{
	struct host_log_pktlog_info cds_pktlog;
	int cds_pkt_size = sizeof(struct host_log_pktlog_info);
	tAniNlHdr msg_header;
	int extra_header_len, nl_payload_len;
	static int nlmsg_seq;
	int diag_type;

	qdf_mem_zero(&cds_pktlog, cds_pkt_size);
	cds_pktlog.version = VERSION_LOG_WLAN_PKT_LOG_INFO_C;
	cds_pktlog.buf_len = skb->len;
	cds_pktlog.seq_no = gwlan_logging.pkt_stats_msg_idx++;
	host_diag_log_set_code(&cds_pktlog, LOG_WLAN_PKT_LOG_INFO_C);
	host_diag_log_set_length(&cds_pktlog.log_hdr, skb->len +
				cds_pkt_size);

	if (unlikely(skb_headroom(skb) < cds_pkt_size)) {
		qdf_nofl_err("VPKT [%d]: Insufficient headroom, head[%pK], data[%pK], req[%zu]",
			     __LINE__, skb->head, skb->data,
			     sizeof(msg_header));
		return -EIO;
	}

	qdf_mem_copy(skb_push(skb, cds_pkt_size),
			&cds_pktlog, cds_pkt_size);

	if (unlikely(skb_headroom(skb) < sizeof(int))) {
		qdf_nofl_err("VPKT [%d]: Insufficient headroom, head[%pK], data[%pK], req[%zu]",
			     __LINE__, skb->head, skb->data,
			     sizeof(int));
		return -EIO;
	}

	diag_type = DIAG_TYPE_LOGS;
	qdf_mem_copy(skb_push(skb, sizeof(int)), &diag_type, sizeof(int));

	extra_header_len = sizeof(msg_header.radio) + sizeof(tAniHdr) +
				sizeof(struct nlmsghdr);
	nl_payload_len = extra_header_len + skb->len;

	msg_header.nlh.nlmsg_type = ANI_NL_MSG_PUMAC;
	msg_header.nlh.nlmsg_len = nl_payload_len;
	msg_header.nlh.nlmsg_flags = NLM_F_REQUEST;
	msg_header.nlh.nlmsg_pid = 0;
	msg_header.nlh.nlmsg_seq = nlmsg_seq++;
	msg_header.radio = 0;
	msg_header.wmsg.type = PTT_MSG_DIAG_CMDS_TYPE;
	msg_header.wmsg.length = cpu_to_be16(skb->len);

	if (unlikely(skb_headroom(skb) < sizeof(msg_header))) {
		qdf_nofl_err("VPKT [%d]: Insufficient headroom, head[%pK], data[%pK], req[%zu]",
			     __LINE__, skb->head, skb->data,
			     sizeof(msg_header));
		return -EIO;
	}

	qdf_mem_copy(skb_push(skb, sizeof(msg_header)), &msg_header,
			sizeof(msg_header));

	return 0;
}

/**
 * nl_srv_bcast_diag() - Wrapper to send bcast msgs to diag events mcast grp
 * @skb: sk buffer pointer
 *
 * Sends the bcast message to diag events multicast group with generic nl socket
 * if CNSS_GENL is enabled. Else, use the legacy netlink socket to send.
 *
 * Return: zero on success, error code otherwise
 */
static int nl_srv_bcast_diag(struct sk_buff *skb)
{
#ifdef CNSS_GENL
	return nl_srv_bcast(skb, CLD80211_MCGRP_DIAG_EVENTS, ANI_NL_MSG_PUMAC);
#else
	return nl_srv_bcast(skb);
#endif
}

/**
 * pktlog_send_per_pkt_stats_to_user() - This function is used to send the per
 * packet statistics to the user
 *
 * This function is used to send the per packet statistics to the user
 *
 * Return: Success if the message is posted to user
 */
static int pktlog_send_per_pkt_stats_to_user(void)
{
	int ret = -1;
	struct pkt_stats_msg *pstats_msg;
	unsigned long flags;
	struct sk_buff *skb_new = NULL;
	static int rate_limit;
	bool free_old_skb = false;

	while (!list_empty(&gwlan_logging.pkt_stat_filled_list)
		&& !gwlan_logging.exit) {
		skb_new = dev_alloc_skb(MAX_SKBMSG_LENGTH);
		if (!skb_new) {
			if (!rate_limit) {
				qdf_err("dev_alloc_skb() failed for msg size[%d] drop count = %u",
					MAX_SKBMSG_LENGTH,
					gwlan_logging.drop_count);
			}
			rate_limit = 1;
			ret = -ENOMEM;
			break;
		}

		spin_lock_irqsave(&gwlan_logging.pkt_stats_lock, flags);

		pstats_msg = (struct pkt_stats_msg *)
			(gwlan_logging.pkt_stat_filled_list.next);
		list_del_init(gwlan_logging.pkt_stat_filled_list.next);
		spin_unlock_irqrestore(&gwlan_logging.pkt_stats_lock, flags);

		ret = pkt_stats_fill_headers(pstats_msg->skb);
		if (ret < 0) {
			qdf_err("Failed to fill headers %d", ret);
			free_old_skb = true;
			goto err;
		}
		ret = nl_srv_bcast_diag(pstats_msg->skb);
		if (ret < 0) {
			qdf_info("Send Failed %d drop_count = %u", ret,
				++gwlan_logging.pkt_stat_drop_cnt);
		} else {
			ret = 0;
		}
err:
	/*
	 * Free old skb in case or error before assigning new skb
	 * to the free list.
	 */
	if (free_old_skb)
		dev_kfree_skb(pstats_msg->skb);

		spin_lock_irqsave(&gwlan_logging.pkt_stats_lock, flags);
		pstats_msg->skb = skb_new;
		list_add_tail(&pstats_msg->node,
				&gwlan_logging.pkt_stat_free_list);
		spin_unlock_irqrestore(&gwlan_logging.pkt_stats_lock, flags);
		ret = 0;
	}

	return ret;

}
#else
static inline
int pktlog_send_per_pkt_stats_to_user(void)
{
	return 0;
}
#endif

static int send_filled_buffers_to_user(void)
{
	int ret = -1;
	struct log_msg *plog_msg;
	int payload_len;
	int tot_msg_len;
	tAniNlHdr *wnl;
	struct sk_buff *skb = NULL;
	struct nlmsghdr *nlh;
	static int nlmsg_seq;
	unsigned long flags;
	static int rate_limit;

	while (!list_empty(&gwlan_logging.filled_list)
	       && !gwlan_logging.exit) {

		skb = dev_alloc_skb(MAX_LOGMSG_LENGTH);
		if (!skb) {
			if (!rate_limit) {
				qdf_err("dev_alloc_skb() failed for msg size[%d] drop count = %u",
					MAX_LOGMSG_LENGTH,
					gwlan_logging.drop_count);
			}
			rate_limit = 1;
			ret = -ENOMEM;
			break;
		}
		rate_limit = 0;

		spin_lock_irqsave(&gwlan_logging.spin_lock, flags);

		plog_msg = (struct log_msg *)
			   (gwlan_logging.filled_list.next);
		list_del_init(gwlan_logging.filled_list.next);
		spin_unlock_irqrestore(&gwlan_logging.spin_lock, flags);
		/* 4 extra bytes for the radio idx */
		payload_len = plog_msg->filled_length +
			      sizeof(wnl->radio) + sizeof(tAniHdr);

		tot_msg_len = NLMSG_SPACE(payload_len);
		nlh = nlmsg_put(skb, 0, nlmsg_seq++,
				ANI_NL_MSG_LOG, payload_len, NLM_F_REQUEST);
		if (!nlh) {
			spin_lock_irqsave(&gwlan_logging.spin_lock, flags);
			list_add_tail(&plog_msg->node,
				      &gwlan_logging.free_list);
			spin_unlock_irqrestore(&gwlan_logging.spin_lock, flags);
			qdf_err("drop_count = %u", ++gwlan_logging.drop_count);
			qdf_err("nlmsg_put() failed for msg size[%d]",
				tot_msg_len);
			dev_kfree_skb(skb);
			skb = NULL;
			ret = -EINVAL;
			continue;
		}

		wnl = (tAniNlHdr *) nlh;
		wnl->radio = plog_msg->radio;
		memcpy(&wnl->wmsg, plog_msg->logbuf,
		       plog_msg->filled_length + sizeof(tAniHdr));

		spin_lock_irqsave(&gwlan_logging.spin_lock, flags);
		list_add_tail(&plog_msg->node, &gwlan_logging.free_list);
		spin_unlock_irqrestore(&gwlan_logging.spin_lock, flags);

		ret = nl_srv_bcast_host_logs(skb);
		/* print every 64th drop count */
		if (ret < 0 && (!(gwlan_logging.drop_count % 0x40))) {
			qdf_err("Send Failed %d drop_count = %u",
				ret, ++gwlan_logging.drop_count);
		}
	}

	return ret;
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT
/**
 * wlan_report_log_completion() - Report bug report completion to userspace
 * @is_fatal: Type of event, fatal or not
 * @indicator: Source of bug report, framework/host/firmware
 * @reason_code: Reason for triggering bug report
 * @ring_id: Ring id of logging entities
 *
 * This function is used to report the bug report completion to userspace
 *
 * Return: None
 */
void wlan_report_log_completion(uint32_t is_fatal,
		uint32_t indicator,
		uint32_t reason_code,
		uint8_t ring_id)
{
	WLAN_HOST_DIAG_EVENT_DEF(wlan_diag_event,
			struct host_event_wlan_log_complete);

	wlan_diag_event.is_fatal = is_fatal;
	wlan_diag_event.indicator = indicator;
	wlan_diag_event.reason_code = reason_code;
	wlan_diag_event.reserved = ring_id;

	WLAN_HOST_DIAG_EVENT_REPORT(&wlan_diag_event, EVENT_WLAN_LOG_COMPLETE);
}
#endif

#ifdef FEATURE_WLAN_DIAG_SUPPORT
/**
 * send_flush_completion_to_user() - Indicate flush completion to the user
 * @ring_id:  Ring id of logging entities
 *
 * This function is used to send the flush completion message to user space
 *
 * Return: None
 */
static void send_flush_completion_to_user(uint8_t ring_id)
{
	uint32_t is_fatal, indicator, reason_code;
	bool recovery_needed;

	cds_get_and_reset_log_completion(&is_fatal,
		&indicator, &reason_code, &recovery_needed);

	/* Error on purpose, so that it will get logged in the kmsg */
	LOGGING_TRACE(QDF_TRACE_LEVEL_DEBUG,
			"%s: Sending flush done to userspace reason code %d",
			__func__, reason_code);

	wlan_report_log_completion(is_fatal, indicator, reason_code, ring_id);

	if (recovery_needed)
		cds_trigger_recovery(QDF_REASON_UNSPECIFIED);
}
#endif

static void setup_flush_timer(void)
{
	qdf_spin_lock(&gwlan_logging.flush_timer_lock);
	if (!gwlan_logging.is_flush_timer_initialized ||
	    (gwlan_logging.flush_timer_period == 0)) {
		qdf_spin_unlock(&gwlan_logging.flush_timer_lock);
		return;
	}
	qdf_timer_mod(&gwlan_logging.flush_timer,
		      gwlan_logging.flush_timer_period);
	qdf_spin_unlock(&gwlan_logging.flush_timer_lock);
}

/**
 * wlan_logging_thread() - The WLAN Logger thread
 * @Arg - pointer to the HDD context
 *
 * This thread logs log message to App registered for the logs.
 */
static int wlan_logging_thread(void *Arg)
{
	int ret_wait_status = 0;
	int ret = 0;
	unsigned long flags;

	while (!gwlan_logging.exit) {
		setup_flush_timer();
		ret_wait_status =
			wait_event_interruptible(gwlan_logging.wait_queue,
						 (!list_empty
							  (&gwlan_logging.filled_list)
						  || test_bit(
						     HOST_LOG_DRIVER_MSG,
						     &gwlan_logging.eventFlag)
						  || test_bit(
						     HOST_LOG_PER_PKT_STATS,
						     &gwlan_logging.eventFlag)
						  || test_bit(
						     HOST_LOG_FW_FLUSH_COMPLETE,
						     &gwlan_logging.eventFlag)
						  || gwlan_logging.exit));

		if (ret_wait_status == -ERESTARTSYS) {
			qdf_err("wait_event_interruptible returned -ERESTARTSYS");
			break;
		}

		if (gwlan_logging.exit)
			break;


		if (test_and_clear_bit(HOST_LOG_DRIVER_MSG,
					&gwlan_logging.eventFlag)) {
			ret = send_filled_buffers_to_user();
			if (-ENOMEM == ret)
				msleep(200);
#ifdef FEATURE_WLAN_DIAG_SUPPORT
			if (WLAN_LOG_INDICATOR_HOST_ONLY ==
			   cds_get_log_indicator()) {
				send_flush_completion_to_user(
						RING_ID_DRIVER_DEBUG);
			}
#endif
		}

		if (test_and_clear_bit(HOST_LOG_PER_PKT_STATS,
					&gwlan_logging.eventFlag)) {
			ret = pktlog_send_per_pkt_stats_to_user();
			if (-ENOMEM == ret)
				msleep(200);
		}

		if (test_and_clear_bit(HOST_LOG_FW_FLUSH_COMPLETE,
					&gwlan_logging.eventFlag)) {
			/* Flush bit could have been set while we were mid
			 * way in the logging thread. So, need to check other
			 * buffers like log messages, per packet stats again
			 * to flush any residual data in them
			 */
			if (gwlan_logging.is_flush_complete == true) {
				gwlan_logging.is_flush_complete = false;
#ifdef FEATURE_WLAN_DIAG_SUPPORT
				send_flush_completion_to_user(
						RING_ID_DRIVER_DEBUG);
#endif
			} else {
				gwlan_logging.is_flush_complete = true;
				/* Flush all current host logs*/
				spin_lock_irqsave(&gwlan_logging.spin_lock,
					flags);
				wlan_queue_logmsg_for_app();
				spin_unlock_irqrestore(&gwlan_logging.spin_lock,
					flags);
				set_bit(HOST_LOG_DRIVER_MSG,
						&gwlan_logging.eventFlag);
				set_bit(HOST_LOG_PER_PKT_STATS,
						&gwlan_logging.eventFlag);
				set_bit(HOST_LOG_FW_FLUSH_COMPLETE,
						&gwlan_logging.eventFlag);
				wake_up_interruptible(
						&gwlan_logging.wait_queue);
			}
		}
	}

	complete_and_exit(&gwlan_logging.shutdown_comp, 0);

	return 0;
}

void wlan_logging_set_active(bool active)
{
	gwlan_logging.is_active = active;
}

void wlan_set_console_log_levels(uint32_t console_log_levels)
{
	gwlan_logging.console_log_levels = console_log_levels;
}

qdf_export_symbol(wlan_set_console_log_levels);

static void flush_log_buffers_timer(void *dummy)
{
	wlan_flush_host_logs_for_fatal();
}

int wlan_logging_set_flush_timer(uint32_t milliseconds)
{
	if (milliseconds > MAX_FLUSH_TIMER_PERIOD_VALUE) {
		QDF_TRACE_ERROR(QDF_MODULE_ID_QDF,
				"ERROR! value should be (0 - %d)\n",
				MAX_FLUSH_TIMER_PERIOD_VALUE);
		return -EINVAL;
	}
	if (!gwlan_logging.is_active) {
		QDF_TRACE_ERROR(QDF_MODULE_ID_QDF,
				"WLAN-Logging not active");
		return -EINVAL;
	}
	qdf_spin_lock(&gwlan_logging.flush_timer_lock);
	if (!gwlan_logging.is_flush_timer_initialized) {
		qdf_spin_unlock(&gwlan_logging.flush_timer_lock);
		return -EINVAL;
	}
	gwlan_logging.flush_timer_period = milliseconds;
	if (milliseconds) {
		qdf_timer_mod(&gwlan_logging.flush_timer,
			      gwlan_logging.flush_timer_period);
	}
	qdf_spin_unlock(&gwlan_logging.flush_timer_lock);
	return 0;
}

static int panic_wifilog_ratelimit_print(void)
{
	return __ratelimit(&panic_wifilog_ratelimit);
}

/**
 * wlan_logging_dump_last_logs() - Panic notifier callback's helper function
 *
 * This function prints buffered logs in chunks of MAX_LOG_LINE.
 */
static void wlan_logging_dump_last_logs(void)
{
	char *log;
	struct log_msg *plog_msg;
	char textbuf[MAX_LOG_LINE];
	unsigned int filled_length;
	unsigned int text_len;
	unsigned long flags;

	/* Iterate over panic list */
	pr_err("\n");
	while (!list_empty(&gwlan_logging.panic_list)) {
		plog_msg = (struct log_msg *)
			   (gwlan_logging.panic_list.next);
		list_del_init(gwlan_logging.panic_list.next);
		log = &plog_msg->logbuf[sizeof(tAniHdr)];
		filled_length = plog_msg->filled_length;
		while (filled_length) {
			text_len = scnprintf(textbuf,
					     sizeof(textbuf),
					     "%s", log);
			if (panic_wifilog_ratelimit_print())
				pr_err("%s\n", textbuf);
			log += text_len;
			filled_length -= text_len;
		}
		spin_lock_irqsave(&gwlan_logging.spin_lock, flags);
		list_add_tail(&plog_msg->node,
			      &gwlan_logging.free_list);
		spin_unlock_irqrestore(&gwlan_logging.spin_lock, flags);
	}
}

/**
 * wlan_logging_panic_handler() - Panic notifier callback
 *
 * This function extracts log buffers in filled list and
 * current node.Sends them to helper function for printing.
 */
static int wlan_logging_panic_handler(struct notifier_block *this,
				      unsigned long event, void *ptr)
{
	char *log;
	struct log_msg *plog_msg;
	unsigned long flags;

	spin_lock_irqsave(&gwlan_logging.spin_lock, flags);
	/* Iterate over nodes queued for app */
	while (!list_empty(&gwlan_logging.filled_list)) {
		plog_msg = (struct log_msg *)
			   (gwlan_logging.filled_list.next);
		list_del_init(gwlan_logging.filled_list.next);
		list_add_tail(&plog_msg->node,
			      &gwlan_logging.panic_list);
	}
	/* Check current node */
	if (gwlan_logging.pcur_node &&
	    gwlan_logging.pcur_node->filled_length) {
		plog_msg = gwlan_logging.pcur_node;
		log = &plog_msg->logbuf[sizeof(tAniHdr)];
		log[plog_msg->filled_length] = '\0';
		list_add_tail(&gwlan_logging.pcur_node->node,
			      &gwlan_logging.panic_list);
		if (!list_empty(&gwlan_logging.free_list)) {
			gwlan_logging.pcur_node =
				(struct log_msg *)(gwlan_logging.free_list.next);
			list_del_init(gwlan_logging.free_list.next);
			gwlan_logging.pcur_node->filled_length = 0;
		} else
			gwlan_logging.pcur_node = NULL;
	}
	spin_unlock_irqrestore(&gwlan_logging.spin_lock, flags);

	wlan_logging_dump_last_logs();

	return NOTIFY_DONE;
}

static struct notifier_block panic_nb = {
	.notifier_call  = wlan_logging_panic_handler,
};

int wlan_logging_notifier_init(bool dump_at_kernel_enable)
{
	int ret;

	if (gwlan_logging.is_active &&
	    !dump_at_kernel_enable) {
		ret = atomic_notifier_chain_register(&panic_notifier_list,
						     &panic_nb);
		if (ret) {
			QDF_TRACE_ERROR(QDF_MODULE_ID_QDF,
					"Failed to register panic notifier");
			return -EINVAL;
		}
	}

	return 0;
}

int wlan_logging_notifier_deinit(bool dump_at_kernel_enable)
{
	if (gwlan_logging.is_active &&
	    !dump_at_kernel_enable) {
		atomic_notifier_chain_unregister(&panic_notifier_list,
						 &panic_nb);
	}

	return 0;
}

static void flush_timer_init(void)
{
	qdf_spinlock_create(&gwlan_logging.flush_timer_lock);
	qdf_timer_init(NULL, &gwlan_logging.flush_timer,
		       flush_log_buffers_timer, NULL,
		       QDF_TIMER_TYPE_SW);
	gwlan_logging.is_flush_timer_initialized = true;
	gwlan_logging.flush_timer_period = 0;
}

int wlan_logging_sock_init_svc(void)
{
	int i = 0, j, pkt_stats_size;
	unsigned long irq_flag;

	flush_timer_init();
	spin_lock_init(&gwlan_logging.spin_lock);
	spin_lock_init(&gwlan_logging.pkt_stats_lock);

	gwlan_logging.console_log_levels = 0;
	gwlan_logging.num_buf = MAX_LOGMSG_COUNT;
	gwlan_logging.buffer_length = MAX_LOGMSG_LENGTH;

	if (allocate_log_msg_buffer() != QDF_STATUS_SUCCESS) {
		qdf_err("Could not allocate memory for log_msg");
		return -ENOMEM;
	}

	spin_lock_irqsave(&gwlan_logging.spin_lock, irq_flag);
	INIT_LIST_HEAD(&gwlan_logging.free_list);
	INIT_LIST_HEAD(&gwlan_logging.filled_list);
	INIT_LIST_HEAD(&gwlan_logging.panic_list);

	for (i = 0; i < gwlan_logging.num_buf; i++) {
		list_add(&gplog_msg[i].node, &gwlan_logging.free_list);
		gplog_msg[i].index = i;
	}
	gwlan_logging.pcur_node = (struct log_msg *)
				  (gwlan_logging.free_list.next);
	list_del_init(gwlan_logging.free_list.next);
	spin_unlock_irqrestore(&gwlan_logging.spin_lock, irq_flag);

	/* Initialize the pktStats data structure here */
	pkt_stats_size = sizeof(struct pkt_stats_msg);
	gpkt_stats_buffers = vmalloc(MAX_PKTSTATS_BUFF * pkt_stats_size);
	if (!gpkt_stats_buffers) {
		qdf_err("Could not allocate memory for Pkt stats");
		goto err1;
	}
	qdf_mem_zero(gpkt_stats_buffers,
			MAX_PKTSTATS_BUFF * pkt_stats_size);

	spin_lock_irqsave(&gwlan_logging.pkt_stats_lock, irq_flag);
	gwlan_logging.pkt_stats_msg_idx = 0;
	INIT_LIST_HEAD(&gwlan_logging.pkt_stat_free_list);
	INIT_LIST_HEAD(&gwlan_logging.pkt_stat_filled_list);
	spin_unlock_irqrestore(&gwlan_logging.pkt_stats_lock, irq_flag);


	for (i = 0; i < MAX_PKTSTATS_BUFF; i++) {
		gpkt_stats_buffers[i].skb = dev_alloc_skb(MAX_PKTSTATS_LENGTH);
		if (!gpkt_stats_buffers[i].skb) {
			qdf_err("Memory alloc failed for skb");
			/* free previously allocated skb and return */
			for (j = 0; j < i ; j++)
				dev_kfree_skb(gpkt_stats_buffers[j].skb);
			goto err2;
		}
		spin_lock_irqsave(&gwlan_logging.pkt_stats_lock, irq_flag);
		list_add(&gpkt_stats_buffers[i].node,
			&gwlan_logging.pkt_stat_free_list);
		spin_unlock_irqrestore(&gwlan_logging.pkt_stats_lock, irq_flag);
	}
	spin_lock_irqsave(&gwlan_logging.pkt_stats_lock, irq_flag);
	gwlan_logging.pkt_stats_pcur_node = (struct pkt_stats_msg *)
		(gwlan_logging.pkt_stat_free_list.next);
	list_del_init(gwlan_logging.pkt_stat_free_list.next);
	spin_unlock_irqrestore(&gwlan_logging.pkt_stats_lock, irq_flag);
	/* Pkt Stats intialization done */

	init_waitqueue_head(&gwlan_logging.wait_queue);
	gwlan_logging.exit = false;
	clear_bit(HOST_LOG_DRIVER_MSG, &gwlan_logging.eventFlag);
	clear_bit(HOST_LOG_PER_PKT_STATS, &gwlan_logging.eventFlag);
	clear_bit(HOST_LOG_FW_FLUSH_COMPLETE, &gwlan_logging.eventFlag);
	init_completion(&gwlan_logging.shutdown_comp);
	gwlan_logging.thread = kthread_create(wlan_logging_thread, NULL,
					      "wlan_logging_thread");
	if (IS_ERR(gwlan_logging.thread)) {
		qdf_err("Could not Create LogMsg Thread Controller");
		goto err3;
	}
	wake_up_process(gwlan_logging.thread);
	gwlan_logging.is_active = true;
	gwlan_logging.is_flush_complete = false;

	return 0;

err3:
	for (i = 0; i < MAX_PKTSTATS_BUFF; i++) {
		if (gpkt_stats_buffers[i].skb)
			dev_kfree_skb(gpkt_stats_buffers[i].skb);
	}
err2:
	spin_lock_irqsave(&gwlan_logging.pkt_stats_lock, irq_flag);
	gwlan_logging.pkt_stats_pcur_node = NULL;
	spin_unlock_irqrestore(&gwlan_logging.pkt_stats_lock, irq_flag);
	vfree(gpkt_stats_buffers);
	gpkt_stats_buffers = NULL;
err1:
	spin_lock_irqsave(&gwlan_logging.spin_lock, irq_flag);
	gwlan_logging.pcur_node = NULL;
	spin_unlock_irqrestore(&gwlan_logging.spin_lock, irq_flag);
	free_log_msg_buffer();

	return -ENOMEM;
}

static void flush_timer_deinit(void)
{
	gwlan_logging.is_flush_timer_initialized = false;
	qdf_spin_lock(&gwlan_logging.flush_timer_lock);
	qdf_timer_stop(&gwlan_logging.flush_timer);
	qdf_timer_free(&gwlan_logging.flush_timer);
	qdf_spin_unlock(&gwlan_logging.flush_timer_lock);
	qdf_spinlock_destroy(&gwlan_logging.flush_timer_lock);
}

int wlan_logging_sock_deinit_svc(void)
{
	unsigned long irq_flag;
	int i;

	if (!gwlan_logging.pcur_node)
		return 0;

	INIT_COMPLETION(gwlan_logging.shutdown_comp);
	gwlan_logging.exit = true;
	gwlan_logging.is_active = false;
#if defined(FEATURE_FW_LOG_PARSING) || defined(FEATURE_WLAN_DIAG_SUPPORT)
	cds_set_multicast_logging(0);
#endif
	gwlan_logging.is_flush_complete = false;
	clear_bit(HOST_LOG_DRIVER_MSG, &gwlan_logging.eventFlag);
	clear_bit(HOST_LOG_PER_PKT_STATS, &gwlan_logging.eventFlag);
	clear_bit(HOST_LOG_FW_FLUSH_COMPLETE, &gwlan_logging.eventFlag);
	wake_up_interruptible(&gwlan_logging.wait_queue);
	wait_for_completion(&gwlan_logging.shutdown_comp);

	spin_lock_irqsave(&gwlan_logging.spin_lock, irq_flag);
	gwlan_logging.pcur_node = NULL;
	spin_unlock_irqrestore(&gwlan_logging.spin_lock, irq_flag);

	spin_lock_irqsave(&gwlan_logging.pkt_stats_lock, irq_flag);
	gwlan_logging.pkt_stats_pcur_node = NULL;
	gwlan_logging.pkt_stats_msg_idx = 0;
	gwlan_logging.pkt_stat_drop_cnt = 0;
	for (i = 0; i < MAX_PKTSTATS_BUFF; i++) {
		if (gpkt_stats_buffers[i].skb)
			dev_kfree_skb(gpkt_stats_buffers[i].skb);
	}
	spin_unlock_irqrestore(&gwlan_logging.pkt_stats_lock, irq_flag);

	vfree(gpkt_stats_buffers);
	gpkt_stats_buffers = NULL;
	free_log_msg_buffer();
	flush_timer_deinit();

	return 0;
}

/**
 * wlan_logging_set_per_pkt_stats() - This function triggers per packet logging
 *
 * This function is used to send signal to the logger thread for logging per
 * packet stats
 *
 * Return: None
 *
 */
void wlan_logging_set_per_pkt_stats(void)
{
	if (gwlan_logging.is_active == false)
		return;

	set_bit(HOST_LOG_PER_PKT_STATS, &gwlan_logging.eventFlag);
	wake_up_interruptible(&gwlan_logging.wait_queue);
}

/*
 * wlan_logging_set_fw_flush_complete() - FW log flush completion
 *
 * This function is used to send signal to the logger thread to indicate
 * that the flushing of FW logs is complete by the FW
 *
 * Return: None
 *
 */
void wlan_logging_set_fw_flush_complete(void)
{
	if (!gwlan_logging.is_active)
		return;

	set_bit(HOST_LOG_FW_FLUSH_COMPLETE, &gwlan_logging.eventFlag);
	wake_up_interruptible(&gwlan_logging.wait_queue);
}

/**
 * wlan_flush_host_logs_for_fatal() - Flush host logs
 *
 * This function is used to send signal to the logger thread to
 * Flush the host logs
 *
 * Return: None
 */
void wlan_flush_host_logs_for_fatal(void)
{
	unsigned long flags;

	if (gwlan_logging.flush_timer_period == 0)
		qdf_info("Flush all host logs Setting HOST_LOG_POST_MAS");
	spin_lock_irqsave(&gwlan_logging.spin_lock, flags);
	wlan_queue_logmsg_for_app();
	spin_unlock_irqrestore(&gwlan_logging.spin_lock, flags);
	set_bit(HOST_LOG_DRIVER_MSG, &gwlan_logging.eventFlag);
	wake_up_interruptible(&gwlan_logging.wait_queue);
}

#ifdef FEATURE_PKTLOG
#ifndef REMOVE_PKT_LOG

static uint8_t gtx_count;
static uint8_t grx_count;

/**
 * wlan_get_pkt_stats_free_node() - Get the free node for pkt stats
 *
 * This function is used to get the free node for pkt stats from
 * free list/filles list
 *
 * Return: int
 *
 */
static int wlan_get_pkt_stats_free_node(void)
{
	int ret = 0;

	list_add_tail(&gwlan_logging.pkt_stats_pcur_node->node,
			&gwlan_logging.pkt_stat_filled_list);

	if (!list_empty(&gwlan_logging.pkt_stat_free_list)) {
		/* Get buffer from free list */
		gwlan_logging.pkt_stats_pcur_node =
		(struct pkt_stats_msg *)(gwlan_logging.pkt_stat_free_list.next);
		list_del_init(gwlan_logging.pkt_stat_free_list.next);
	} else if (!list_empty(&gwlan_logging.pkt_stat_filled_list)) {
		/* Get buffer from filled list. This condition will drop the
		 * packet from being indicated to app
		 */
		gwlan_logging.pkt_stats_pcur_node =
			(struct pkt_stats_msg *)
				(gwlan_logging.pkt_stat_filled_list.next);
		++gwlan_logging.pkt_stat_drop_cnt;
		/* print every 64th drop count */
		if (
			cds_is_multicast_logging() &&
			(!(gwlan_logging.pkt_stat_drop_cnt % 0x40))) {
			qdf_err("drop_count = %u",
				gwlan_logging.pkt_stat_drop_cnt);
		}
		list_del_init(gwlan_logging.pkt_stat_filled_list.next);
		ret = 1;
	}

	/* Reset the skb values, essential if dequeued from filled list */
	skb_trim(gwlan_logging.pkt_stats_pcur_node->skb, 0);
	return ret;
}

/**
 * wlan_pkt_stats_to_logger_thread() - Add the pkt stats to SKB
 * @pl_hdr: Pointer to pl_hdr
 * @pkt_dump: Pointer to pkt_dump
 * @data: Pointer to data
 *
 * This function adds the pktstats hdr and data to current
 * skb node of free list.
 *
 * Return: None
 */
void wlan_pkt_stats_to_logger_thread(void *pl_hdr, void *pkt_dump, void *data)
{
	struct ath_pktlog_hdr *pktlog_hdr;
	struct packet_dump *pkt_stats_dump;
	int total_stats_len = 0;
	bool wake_up_thread = false;
	unsigned long flags;
	struct sk_buff *ptr;
	int hdr_size;

	pktlog_hdr = (struct ath_pktlog_hdr *)pl_hdr;

	if (!pktlog_hdr) {
		qdf_err("Invalid pkt_stats_header");
		return;
	}

	pkt_stats_dump = (struct packet_dump *)pkt_dump;
	total_stats_len = sizeof(struct ath_pktlog_hdr) +
					pktlog_hdr->size;

	spin_lock_irqsave(&gwlan_logging.pkt_stats_lock, flags);

	if (!gwlan_logging.pkt_stats_pcur_node) {
		spin_unlock_irqrestore(&gwlan_logging.pkt_stats_lock, flags);
		return;
	}

	/* Check if we can accommodate more log into current node/buffer */
	hdr_size = sizeof(struct host_log_pktlog_info) +
			sizeof(tAniNlHdr);
	if ((total_stats_len +  hdr_size) >=
		skb_tailroom(gwlan_logging.pkt_stats_pcur_node->skb)) {
		wake_up_thread = true;
		wlan_get_pkt_stats_free_node();
	}

	ptr = gwlan_logging.pkt_stats_pcur_node->skb;
	qdf_mem_copy(skb_put(ptr,
			sizeof(struct ath_pktlog_hdr)),
			pktlog_hdr,
			sizeof(struct ath_pktlog_hdr));

	if (pkt_stats_dump) {
		qdf_mem_copy(skb_put(ptr,
				sizeof(struct packet_dump)),
				pkt_stats_dump,
				sizeof(struct packet_dump));
		pktlog_hdr->size -= sizeof(struct packet_dump);
	}

	if (data)
		qdf_mem_copy(skb_put(ptr,
				pktlog_hdr->size),
				data, pktlog_hdr->size);

	if (pkt_stats_dump && pkt_stats_dump->type == STOP_MONITOR) {
		wake_up_thread = true;
		wlan_get_pkt_stats_free_node();
	}

	spin_unlock_irqrestore(&gwlan_logging.pkt_stats_lock, flags);

	/* Wakeup logger thread */
	if (true == wake_up_thread) {
		set_bit(HOST_LOG_PER_PKT_STATS, &gwlan_logging.eventFlag);
		wake_up_interruptible(&gwlan_logging.wait_queue);
	}
}

/**
 * driver_hal_status_map() - maps driver to hal
 * status
 * @status: status to be mapped
 *
 * This function is used to map driver to hal status
 *
 * Return: None
 *
 */
static void driver_hal_status_map(uint8_t *status)
{
	switch (*status) {
	case tx_status_ok:
		*status = TX_PKT_FATE_ACKED;
		break;
	case tx_status_discard:
		*status = TX_PKT_FATE_DRV_DROP_OTHER;
		break;
	case tx_status_no_ack:
		*status = TX_PKT_FATE_SENT;
		break;
	case tx_status_download_fail:
		*status = TX_PKT_FATE_FW_QUEUED;
		break;
	default:
		*status = TX_PKT_FATE_DRV_DROP_OTHER;
		break;
	}
}

/*
 * send_packetdump() - send packet dump
 * @soc: soc handle
 * @vdev_id: ID of the virtual device handle
 * @netbuf: netbuf
 * @status: status of tx packet
 * @type: type of packet
 *
 * This function is used to send packet dump to HAL layer
 * using wlan_pkt_stats_to_logger_thread
 *
 * Return: None
 *
 */
static void send_packetdump(ol_txrx_soc_handle soc,
			    uint8_t vdev_id, qdf_nbuf_t netbuf,
			    uint8_t status, uint8_t type)
{
	struct ath_pktlog_hdr pktlog_hdr = {0};
	struct packet_dump pd_hdr = {0};

	if (!netbuf) {
		qdf_err("Invalid netbuf");
		return;
	}

	/* Send packet dump only for STA interface */
	if (wlan_op_mode_sta != cdp_get_opmode(soc, vdev_id))
		return;

#if defined(HELIUMPLUS)
	pktlog_hdr.flags |= PKTLOG_HDR_SIZE_16;
#endif

	pktlog_hdr.log_type = PKTLOG_TYPE_PKT_DUMP;
	pktlog_hdr.size = sizeof(pd_hdr) + netbuf->len;

	pd_hdr.status = status;
	pd_hdr.type = type;
	pd_hdr.driver_ts = qdf_get_monotonic_boottime();

	if ((type == TX_MGMT_PKT) || (type == TX_DATA_PKT))
		gtx_count++;
	else if ((type == RX_MGMT_PKT) || (type == RX_DATA_PKT))
		grx_count++;

	wlan_pkt_stats_to_logger_thread(&pktlog_hdr, &pd_hdr, netbuf->data);
}


/*
 * send_packetdump_monitor() - sends start/stop packet dump indication
 * @type: type of packet
 *
 * This function is used to indicate HAL layer to start/stop monitoring
 * of packets
 *
 * Return: None
 *
 */
static void send_packetdump_monitor(uint8_t type)
{
	struct ath_pktlog_hdr pktlog_hdr = {0};
	struct packet_dump pd_hdr = {0};

#if defined(HELIUMPLUS)
	pktlog_hdr.flags |= PKTLOG_HDR_SIZE_16;
#endif

	pktlog_hdr.log_type = PKTLOG_TYPE_PKT_DUMP;
	pktlog_hdr.size = sizeof(pd_hdr);

	pd_hdr.type = type;

	LOGGING_TRACE(QDF_TRACE_LEVEL_DEBUG,
			"fate Tx-Rx %s: type: %d", __func__, type);

	wlan_pkt_stats_to_logger_thread(&pktlog_hdr, &pd_hdr, NULL);
}

void wlan_deregister_txrx_packetdump(uint8_t pdev_id)
{
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!soc)
		return;

	if (gtx_count || grx_count) {
		cdp_deregister_packetdump_cb(soc, pdev_id);
		wma_deregister_packetdump_callback();
		send_packetdump_monitor(STOP_MONITOR);
		csr_packetdump_timer_stop();

		gtx_count = 0;
		grx_count = 0;
	} else
		LOGGING_TRACE(QDF_TRACE_LEVEL_DEBUG,
			"%s: deregistered packetdump already", __func__);
}

/*
 * check_txrx_packetdump_count() - function to check
 * tx/rx packet dump global counts
 * @pdev_id: datapath pdev identifier
 *
 * This function is used to check global counts of tx/rx
 * packet dump functionality.
 *
 * Return: 1 if either gtx_count or grx_count reached 32
 *             0 otherwise
 *
 */
static bool check_txrx_packetdump_count(uint8_t pdev_id)
{
	if (gtx_count == MAX_NUM_PKT_LOG ||
		grx_count == MAX_NUM_PKT_LOG) {
		LOGGING_TRACE(QDF_TRACE_LEVEL_DEBUG,
			"%s gtx_count: %d grx_count: %d deregister packetdump",
			__func__, gtx_count, grx_count);
		wlan_deregister_txrx_packetdump(pdev_id);
		return 1;
	}
	return 0;
}

/*
 * tx_packetdump_cb() - tx packet dump callback
 * @soc: soc handle
 * @pdev_id: datapath pdev id
 * @vdev_id: vdev id
 * @netbuf: netbuf
 * @status: status of tx packet
 * @type: packet type
 *
 * This function is used to send tx packet dump to HAL layer
 * and deregister packet dump callbacks
 *
 * Return: None
 *
 */
static void tx_packetdump_cb(ol_txrx_soc_handle soc,
			     uint8_t pdev_id, uint8_t vdev_id,
			     qdf_nbuf_t netbuf,
			     uint8_t status, uint8_t type)
{
	bool temp;

	if (!soc)
		return;

	temp = check_txrx_packetdump_count(pdev_id);
	if (temp)
		return;

	driver_hal_status_map(&status);
	send_packetdump(soc, vdev_id, netbuf, status, type);
}


/*
 * rx_packetdump_cb() - rx packet dump callback
 * @soc: soc handle
 * @pdev_id: datapath pdev id
 * @vdev_id: vdev id
 * @netbuf: netbuf
 * @status: status of rx packet
 * @type: packet type
 *
 * This function is used to send rx packet dump to HAL layer
 * and deregister packet dump callbacks
 *
 * Return: None
 *
 */
static void rx_packetdump_cb(ol_txrx_soc_handle soc,
			     uint8_t pdev_id, uint8_t vdev_id,
			     qdf_nbuf_t netbuf,
			     uint8_t status, uint8_t type)
{
	bool temp;

	if (!soc)
		return;

	temp = check_txrx_packetdump_count(pdev_id);
	if (temp)
		return;

	send_packetdump(soc, vdev_id, netbuf, status, type);
}

void wlan_register_txrx_packetdump(uint8_t pdev_id)
{
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!soc)
		return;

	cdp_register_packetdump_cb(soc, pdev_id,
				   tx_packetdump_cb, rx_packetdump_cb);
	wma_register_packetdump_callback(tx_packetdump_cb,
			rx_packetdump_cb);
	send_packetdump_monitor(START_MONITOR);

	gtx_count = 0;
	grx_count = 0;

	csr_packetdump_timer_start();
}
#endif /* REMOVE_PKT_LOG */
#endif /* FEATURE_PKTLOG */
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */
