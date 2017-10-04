/*
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
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

/******************************************************************************
* wlan_logging_sock_svc.c
*
******************************************************************************/

#ifdef WLAN_LOGGING_SOCK_SVC_ENABLE
#include <linux/vmalloc.h>
#ifdef CONFIG_MCL
#include <cds_api.h>
#include <host_diag_core_event.h>
#include "cds_utils.h"
#include "csr_api.h"
#include "wlan_hdd_main.h"
#include "wma.h"
#include "ol_txrx_api.h"
#include "pktlog_ac.h"
#endif
#include <wlan_logging_sock_svc.h>
#include <linux/kthread.h>
#include <qdf_time.h>
#include <qdf_trace.h>
#include <qdf_mc_timer.h>
#include <wlan_ptt_sock_svc.h>
#include <host_diag_core_event.h>
#include "host_diag_core_log.h"

#ifdef CNSS_GENL
#include <net/cnss_nl.h>
#endif

#define MAX_NUM_PKT_LOG 32

#define ALLOWED_LOG_LEVELS_TO_CONSOLE(level) \
	((QDF_TRACE_LEVEL_FATAL == (level)) || \
	 (QDF_TRACE_LEVEL_ERROR == (level)) || \
	 (QDF_TRACE_LEVEL_WARN == (level)) || \
	 (QDF_TRACE_LEVEL_INFO == (level)))

/**
 * struct tx_status - tx status
 * @tx_status_ok: successfully sent + acked
 * @tx_status_discard: discard - not sent (congestion control)
 * @tx_status_no_ack: no_ack - sent, but no ack
 * @tx_status_download_fail: download_fail -
 * the host could not deliver the tx frame to the target
 * @tx_status_peer_del: peer_del - tx completion for
 * alreay deleted peer used for HL case
 *
 * This enum has tx status types
 */
enum tx_status {
	tx_status_ok,
	tx_status_discard,
	tx_status_no_ack,
	tx_status_download_fail,
	tx_status_peer_del,
};

#ifdef CONFIG_MCL
static uint8_t gtx_count;
static uint8_t grx_count;
#endif

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
#define MAX_PKTSTATS_LENGTH 2048
#define MAX_PKTSTATS_BUFF   16

#define HOST_LOG_DRIVER_MSG        0x001
#define HOST_LOG_PER_PKT_STATS     0x002
#define HOST_LOG_FW_FLUSH_COMPLETE 0x003
#define DIAG_TYPE_LOGS                 1
#define PTT_MSG_DIAG_CMDS_TYPE    0x5050

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

struct wlan_logging {
	/* Log Fatal and ERROR to console */
	bool log_to_console;
	/* Number of buffers to be used for logging */
	uint32_t num_buf;
	uint32_t buffer_length;
	/* Lock to synchronize access to shared logging resource */
	spinlock_t spin_lock;
	/* Holds the free node which can be used for filling logs */
	struct list_head free_list;
	/* Holds the filled nodes which needs to be indicated to APP */
	struct list_head filled_list;
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
};

static struct wlan_logging gwlan_logging;
static struct log_msg gplog_msg[MAX_LOGMSG_COUNT];
static struct pkt_stats_msg *gpkt_stats_buffers;

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

#ifdef QCA_WIFI_3_0_ADRASTEA
/**
 * wlan_add_user_log_radio_time_stamp() - add radio, firmware timestamp and
 * time stamp in log buffer
 * @tbuf: Pointer to time stamp buffer
 * @tbuf_sz: Time buffer size
 * @ts: Time stamp value
 * @radoi: the radio index
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
static int wlan_add_user_log_radio_time_stamp(char *tbuf, size_t tbuf_sz,
					      uint64_t ts, int radio)
{
	int tlen;
	char time_buf[20];

	qdf_get_time_of_the_day_in_hr_min_sec_usec(time_buf, sizeof(time_buf));

	tlen = scnprintf(tbuf, tbuf_sz, "R%d: [%.16s][%llu] %s ", radio,
			((in_irq() ? "irq" : in_softirq() ?  "soft_irq" :
			current->comm)),
			ts, time_buf);
	return tlen;
}
#else
/**
 * wlan_add_user_log_radio_time_stamp() - add radio, firmware timestamp and
 * logcat timestamp in log buffer
 * @tbuf: Pointer to time stamp buffer
 * @tbuf_sz: Time buffer size
 * @ts: Time stamp value
 * @radio: the radio index
 *
 * For adrastea time stamp QTIMER raw tick which will be used by cnss_diag
 * to convert it into user visible time stamp
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
static int wlan_add_user_log_radio_time_stamp(char *tbuf, size_t tbuf_sz,
					      uint64_t ts, int radio)
{
	int tlen;
	uint32_t rem;
	char time_buf[20];

	qdf_get_time_of_the_day_in_hr_min_sec_usec(time_buf, sizeof(time_buf));

	rem = do_div(ts, QDF_MC_TIMER_TO_SEC_UNIT);
	tlen = scnprintf(tbuf, tbuf_sz, "R%d: [%.16s][%lu.%06lu] %s ", radio,
			((in_irq() ? "irq" : in_softirq() ?  "soft_irq" :
			current->comm)),
			(unsigned long) ts,
			(unsigned long)rem, time_buf);
	return tlen;
}
#endif

#ifdef CONFIG_MCL
static inline void print_to_console(char *tbuf, char *to_be_sent)
{
	pr_err("%s %s\n", tbuf, to_be_sent);
}
#else
#define print_to_console(str1, str2)
#endif


int wlan_log_to_user(QDF_TRACE_LEVEL log_level, char *to_be_sent, int length)
{
	/* Add the current time stamp */
	char *ptr;
	char tbuf[60];
	int tlen;
	int total_log_len;
	unsigned int *pfilled_length;
	bool wake_up_thread = false;
	unsigned long flags;
	uint64_t ts;
	int radio = 0;
	bool log_overflow = false;

#ifdef CONFIG_MCL
	radio = cds_get_radio_index();
#endif

	if (
#ifdef CONFIG_MCL
		!cds_is_multicast_logging() ||
#endif
		 (radio == -EINVAL) ||
		(!gwlan_logging.is_active)) {
		/*
		 * This is to make sure that we print the logs to kmsg console
		 * when no logger app is running. This is also needed to
		 * log the initial messages during loading of driver where even
		 * if app is running it will not be able to
		 * register with driver immediately and start logging all the
		 * messages.
		 */
		/*
		 * R%d: if the radio index is invalid, just post the message
		 * to console.
		 * Also the radio index shouldn't happen to be EINVAL, but if
		 * that happen just print it, so that the logging would be
		 * aware the cnss_logger is somehow failed.
		 */
		pr_info("R%d: %s\n", radio, to_be_sent);
		return 0;
	}

	ts = qdf_get_log_timestamp();
	tlen = wlan_add_user_log_radio_time_stamp(tbuf, sizeof(tbuf), ts,
						  radio);

	/* 1+1 indicate '\n'+'\0' */
	total_log_len = length + tlen + 1 + 1;

	spin_lock_irqsave(&gwlan_logging.spin_lock, flags);
	/* wlan logging svc resources are not yet initialized */
	if (!gwlan_logging.pcur_node) {
		spin_unlock_irqrestore(&gwlan_logging.spin_lock, flags);
		return -EIO;
	}

	pfilled_length = &gwlan_logging.pcur_node->filled_length;

	/* Check if we can accomodate more log into current node/buffer */
	if ((MAX_LOGMSG_LENGTH <= (*pfilled_length + sizeof(tAniNlHdr))) ||
		((MAX_LOGMSG_LENGTH - (*pfilled_length +
			sizeof(tAniNlHdr))) < total_log_len)) {
		wake_up_thread = true;
		wlan_queue_logmsg_for_app();
		pfilled_length = &gwlan_logging.pcur_node->filled_length;
	}

	ptr = &gwlan_logging.pcur_node->logbuf[sizeof(tAniHdr)];

	/* Assumption here is that we receive logs which is always less than
	 * MAX_LOGMSG_LENGTH, where we can accomodate the
	 *   tAniNlHdr + [context][timestamp] + log
	 *
	 * Continue and copy logs to the available length and discard the rest.
	 */
	if (MAX_LOGMSG_LENGTH < (sizeof(tAniNlHdr) + total_log_len)) {
		log_overflow = true;
		total_log_len = MAX_LOGMSG_LENGTH - sizeof(tAniNlHdr) - 2;
	}

	memcpy(&ptr[*pfilled_length], tbuf, tlen);
	memcpy(&ptr[*pfilled_length + tlen], to_be_sent,
	       min(length, (total_log_len - tlen)));
	*pfilled_length += tlen + min(length, total_log_len - tlen);
	ptr[*pfilled_length] = '\n';
	*pfilled_length += 1;

	spin_unlock_irqrestore(&gwlan_logging.spin_lock, flags);
	/*
	 * QDF_ASSERT if complete log was not accomodated into
	 * the available buffer.
	 */
	QDF_ASSERT(!log_overflow);

	/* Wakeup logger thread */
	if ((true == wake_up_thread)) {
			set_bit(HOST_LOG_DRIVER_MSG, &gwlan_logging.eventFlag);
			wake_up_interruptible(&gwlan_logging.wait_queue);
	}

	if (gwlan_logging.log_to_console
	    && ALLOWED_LOG_LEVELS_TO_CONSOLE(log_level)) {
		print_to_console(tbuf, to_be_sent);
	}

	return 0;
}

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
#ifdef CONFIG_MCL
	host_diag_log_set_code(&cds_pktlog, LOG_WLAN_PKT_LOG_INFO_C);
	host_diag_log_set_length(&cds_pktlog.log_hdr, skb->len +
				cds_pkt_size);
#endif

	if (unlikely(skb_headroom(skb) < cds_pkt_size)) {
		pr_err("VPKT [%d]: Insufficient headroom, head[%pK], data[%pK], req[%zu]",
			__LINE__, skb->head, skb->data, sizeof(msg_header));
		return -EIO;
	}

	qdf_mem_copy(skb_push(skb, cds_pkt_size),
			&cds_pktlog, cds_pkt_size);

	if (unlikely(skb_headroom(skb) < sizeof(int))) {
		pr_err("VPKT [%d]: Insufficient headroom, head[%pK], data[%pK], req[%zu]",
			__LINE__, skb->head, skb->data, sizeof(int));
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
		pr_err("VPKT [%d]: Insufficient headroom, head[%pK], data[%pK], req[%zu]",
			__LINE__, skb->head, skb->data, sizeof(msg_header));
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
 * nl_srv_bcast_host_logs() - Wrapper to send bcast msgs to host logs mcast grp
 * @skb: sk buffer pointer
 *
 * Sends the bcast message to host logs multicast group with generic nl socket
 * if CNSS_GENL is enabled. Else, use the legacy netlink socket to send.
 *
 * Return: zero on success, error code otherwise
 */
static int nl_srv_bcast_host_logs(struct sk_buff *skb)
{
#ifdef CNSS_GENL
	return nl_srv_bcast(skb, CLD80211_MCGRP_HOST_LOGS, ANI_NL_MSG_LOG);
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
int pktlog_send_per_pkt_stats_to_user(void)
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
		if (skb_new == NULL) {
			if (!rate_limit) {
				pr_err("%s: dev_alloc_skb() failed for msg size[%d] drop count = %u\n",
					__func__, MAX_SKBMSG_LENGTH,
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
			pr_err("%s failed to fill headers %d\n", __func__, ret);
			free_old_skb = true;
			goto err;
		}
		ret = nl_srv_bcast_diag(pstats_msg->skb);
		if (ret < 0) {
			pr_info("%s: Send Failed %d drop_count = %u\n",
				__func__, ret,
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
		if (skb == NULL) {
			if (!rate_limit) {
				pr_err
					("%s: dev_alloc_skb() failed for msg size[%d] drop count = %u\n",
					__func__, MAX_LOGMSG_LENGTH,
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
		if (NULL == nlh) {
			spin_lock_irqsave(&gwlan_logging.spin_lock, flags);
			list_add_tail(&plog_msg->node,
				      &gwlan_logging.free_list);
			spin_unlock_irqrestore(&gwlan_logging.spin_lock, flags);
			pr_err("%s: drop_count = %u\n", __func__,
			       ++gwlan_logging.drop_count);
			pr_err("%s: nlmsg_put() failed for msg size[%d]\n",
			       __func__, tot_msg_len);
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
			pr_err("%s: Send Failed %d drop_count = %u\n",
			       __func__, ret, ++gwlan_logging.drop_count);
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

#ifdef CONFIG_MCL
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
			"%s: Sending flush done to userspace", __func__);

	wlan_report_log_completion(is_fatal, indicator, reason_code, ring_id);

	if (recovery_needed)
		cds_trigger_recovery(QDF_REASON_UNSPECIFIED);
}
#endif

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
			pr_err
				("%s: wait_event_interruptible returned -ERESTARTSYS",
				__func__);
			break;
		}

		if (gwlan_logging.exit)
			break;


		if (test_and_clear_bit(HOST_LOG_DRIVER_MSG,
					&gwlan_logging.eventFlag)) {
			ret = send_filled_buffers_to_user();
			if (-ENOMEM == ret)
				msleep(200);
#ifdef CONFIG_MCL
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
#ifdef CONFIG_MCL
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

void wlan_logging_set_log_to_console(bool log_to_console)
{
	gwlan_logging.log_to_console = log_to_console;
}

int wlan_logging_sock_init_svc(void)
{
	int i = 0, j, pkt_stats_size;
	unsigned long irq_flag;

	spin_lock_init(&gwlan_logging.spin_lock);
	spin_lock_init(&gwlan_logging.pkt_stats_lock);

	gwlan_logging.log_to_console = true;
	gwlan_logging.num_buf = MAX_LOGMSG_COUNT;
	gwlan_logging.buffer_length = MAX_LOGMSG_LENGTH;

	spin_lock_irqsave(&gwlan_logging.spin_lock, irq_flag);
	INIT_LIST_HEAD(&gwlan_logging.free_list);
	INIT_LIST_HEAD(&gwlan_logging.filled_list);

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
		pr_err("%s: Could not allocate memory for Pkt stats\n",
			__func__);
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
		if (gpkt_stats_buffers[i].skb == NULL) {
			pr_err("%s: Memory alloc failed for skb", __func__);
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
		pr_err("%s: Could not Create LogMsg Thread Controller",
		       __func__);
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

	return -ENOMEM;
}

int wlan_logging_sock_deinit_svc(void)
{
	unsigned long irq_flag;
	int i;

	if (!gwlan_logging.pcur_node)
		return 0;

#ifdef CONFIG_MCL
	INIT_COMPLETION(gwlan_logging.shutdown_comp);
#endif
	gwlan_logging.exit = true;
	gwlan_logging.is_active = false;
#ifdef CONFIG_MCL
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
	if (gwlan_logging.is_active == false
#ifdef CONFIG_MCL
	    || !cds_is_fatal_event_enabled()
#endif
	   )
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

#ifdef CONFIG_MCL
	if (cds_is_log_report_in_progress()) {
#endif
		pr_info("%s:flush all host logs Setting HOST_LOG_POST_MASK\n",
			__func__);
		spin_lock_irqsave(&gwlan_logging.spin_lock, flags);
		wlan_queue_logmsg_for_app();
		spin_unlock_irqrestore(&gwlan_logging.spin_lock, flags);
		set_bit(HOST_LOG_DRIVER_MSG, &gwlan_logging.eventFlag);
		wake_up_interruptible(&gwlan_logging.wait_queue);
#ifdef CONFIG_MCL
	}
#endif
}

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
#ifdef CONFIG_MCL
			cds_is_multicast_logging() &&
#endif
			(!(gwlan_logging.pkt_stat_drop_cnt % 0x40))) {
			pr_err("%s: drop_count = %u\n",
				__func__, gwlan_logging.pkt_stat_drop_cnt);
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
#ifdef CONFIG_MCL
	struct ath_pktlog_hdr *pktlog_hdr;
	struct packet_dump *pkt_stats_dump;
	int total_stats_len = 0;
	bool wake_up_thread = false;
	unsigned long flags;
	struct sk_buff *ptr;
	int hdr_size;

	pktlog_hdr = (struct ath_pktlog_hdr *)pl_hdr;

	if (pktlog_hdr == NULL) {
		pr_err("%s : Invalid pkt_stats_header\n", __func__);
		return;
	}

	pkt_stats_dump = (struct packet_dump *)pkt_dump;
	total_stats_len = sizeof(struct ath_pktlog_hdr) +
					pktlog_hdr->size;

	spin_lock_irqsave(&gwlan_logging.pkt_stats_lock, flags);

	if (!gwlan_logging.pkt_stats_pcur_node || (NULL == pkt_stats_dump)) {
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

	if (pkt_stats_dump->type == STOP_MONITOR) {
		wake_up_thread = true;
		wlan_get_pkt_stats_free_node();
	}

	spin_unlock_irqrestore(&gwlan_logging.pkt_stats_lock, flags);

	/* Wakeup logger thread */
	if (true == wake_up_thread) {
		set_bit(HOST_LOG_PER_PKT_STATS, &gwlan_logging.eventFlag);
		wake_up_interruptible(&gwlan_logging.wait_queue);
	}
#endif
}

#ifdef CONFIG_MCL
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
 * @netbuf: netbuf
 * @status: status of tx packet
 * @vdev_id: virtual device id
 * @type: type of packet
 *
 * This function is used to send packet dump to HAL layer
 * using wlan_pkt_stats_to_logger_thread
 *
 * Return: None
 *
 */
static void send_packetdump(qdf_nbuf_t netbuf, uint8_t status,
				uint8_t vdev_id, uint8_t type)
{
	struct ath_pktlog_hdr pktlog_hdr = {0};
	struct packet_dump pd_hdr = {0};
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *adapter;

	hdd_ctx = (struct hdd_context *)cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx)
		return;

	adapter = hdd_get_adapter_by_vdev(hdd_ctx, vdev_id);
	if (!adapter)
		return;

	/* Send packet dump only for STA interface */
	if (adapter->device_mode != QDF_STA_MODE)
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

/**
 * wlan_deregister_txrx_packetdump() - tx/rx packet dump
 *  deregistration
 *
 * This function is used to deregister tx/rx packet dump callbacks
 * with ol, pe and htt layers
 *
 * Return: None
 *
 */
void wlan_deregister_txrx_packetdump(void)
{
	if (gtx_count || grx_count) {
		ol_deregister_packetdump_callback();
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
 *
 * This function is used to check global counts of tx/rx
 * packet dump functionality.
 *
 * Return: 1 if either gtx_count or grx_count reached 32
 *             0 otherwise
 *
 */
static bool check_txrx_packetdump_count(void)
{
	if (gtx_count == MAX_NUM_PKT_LOG ||
		grx_count == MAX_NUM_PKT_LOG) {
		LOGGING_TRACE(QDF_TRACE_LEVEL_DEBUG,
			"%s gtx_count: %d grx_count: %d deregister packetdump",
			__func__, gtx_count, grx_count);
		wlan_deregister_txrx_packetdump();
		return 1;
	}
	return 0;
}

/*
 * tx_packetdump_cb() - tx packet dump callback
 * @netbuf: netbuf
 * @status: status of tx packet
 * @vdev_id: virtual device id
 * @type: packet type
 *
 * This function is used to send tx packet dump to HAL layer
 * and deregister packet dump callbacks
 *
 * Return: None
 *
 */
static void tx_packetdump_cb(qdf_nbuf_t netbuf, uint8_t status,
				uint8_t vdev_id, uint8_t type)
{
	bool temp;

	temp = check_txrx_packetdump_count();
	if (temp)
		return;

	driver_hal_status_map(&status);
	send_packetdump(netbuf, status, vdev_id, type);
}


/*
 * rx_packetdump_cb() - rx packet dump callback
 * @netbuf: netbuf
 * @status: status of rx packet
 * @vdev_id: virtual device id
 * @type: packet type
 *
 * This function is used to send rx packet dump to HAL layer
 * and deregister packet dump callbacks
 *
 * Return: None
 *
 */
static void rx_packetdump_cb(qdf_nbuf_t netbuf, uint8_t status,
				uint8_t vdev_id, uint8_t type)
{
	bool temp;

	temp = check_txrx_packetdump_count();
	if (temp)
		return;

	send_packetdump(netbuf, status, vdev_id, type);
}


/**
 * wlan_register_txrx_packetdump() - tx/rx packet dump
 * registration
 *
 * This function is used to register tx/rx packet dump callbacks
 * with ol, pe and htt layers
 *
 * Return: None
 *
 */
void wlan_register_txrx_packetdump(void)
{
	ol_register_packetdump_callback(tx_packetdump_cb,
			rx_packetdump_cb);
	wma_register_packetdump_callback(tx_packetdump_cb,
			rx_packetdump_cb);
	send_packetdump_monitor(START_MONITOR);

	gtx_count = 0;
	grx_count = 0;
}
#endif
#endif /* WLAN_LOGGING_SOCK_SVC_ENABLE */
