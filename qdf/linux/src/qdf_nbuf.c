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

/**
 * DOC: qdf_nbuf.c
 * QCA driver framework(QDF) network buffer management APIs
 */

#include <linux/hashtable.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <qdf_atomic.h>
#include <qdf_debugfs.h>
#include <qdf_lock.h>
#include <qdf_mem.h>
#include <qdf_module.h>
#include <qdf_nbuf.h>
#include <qdf_status.h>
#include "qdf_str.h"
#include <qdf_trace.h>
#include "qdf_tracker.h"
#include <qdf_types.h>
#include <net/ieee80211_radiotap.h>
#include <pld_common.h>

#if defined(FEATURE_TSO)
#include <net/ipv6.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/if_vlan.h>
#include <linux/ip.h>
#endif /* FEATURE_TSO */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0)

#define qdf_nbuf_users_inc atomic_inc
#define qdf_nbuf_users_dec atomic_dec
#define qdf_nbuf_users_set atomic_set
#define qdf_nbuf_users_read atomic_read
#else
#define qdf_nbuf_users_inc refcount_inc
#define qdf_nbuf_users_dec refcount_dec
#define qdf_nbuf_users_set refcount_set
#define qdf_nbuf_users_read refcount_read
#endif /* KERNEL_VERSION(4, 13, 0) */

#define IEEE80211_RADIOTAP_VHT_BW_20	0
#define IEEE80211_RADIOTAP_VHT_BW_40	1
#define IEEE80211_RADIOTAP_VHT_BW_80	2
#define IEEE80211_RADIOTAP_VHT_BW_160	3

#define RADIOTAP_VHT_BW_20	0
#define RADIOTAP_VHT_BW_40	1
#define RADIOTAP_VHT_BW_80	4
#define RADIOTAP_VHT_BW_160	11

/* channel number to freq conversion */
#define CHANNEL_NUM_14 14
#define CHANNEL_NUM_15 15
#define CHANNEL_NUM_27 27
#define CHANNEL_NUM_35 35
#define CHANNEL_NUM_182 182
#define CHANNEL_NUM_197 197
#define CHANNEL_FREQ_2484 2484
#define CHANNEL_FREQ_2407 2407
#define CHANNEL_FREQ_2512 2512
#define CHANNEL_FREQ_5000 5000
#define CHANNEL_FREQ_4000 4000
#define FREQ_MULTIPLIER_CONST_5MHZ 5
#define FREQ_MULTIPLIER_CONST_20MHZ 20
#define RADIOTAP_5G_SPECTRUM_CHANNEL 0x0100
#define RADIOTAP_2G_SPECTRUM_CHANNEL 0x0080
#define RADIOTAP_CCK_CHANNEL 0x0020
#define RADIOTAP_OFDM_CHANNEL 0x0040

#ifdef FEATURE_NBUFF_REPLENISH_TIMER
#include <qdf_mc_timer.h>

struct qdf_track_timer {
	qdf_mc_timer_t track_timer;
	qdf_atomic_t alloc_fail_cnt;
};

static struct qdf_track_timer alloc_track_timer;

#define QDF_NBUF_ALLOC_EXPIRE_TIMER_MS  5000
#define QDF_NBUF_ALLOC_EXPIRE_CNT_THRESHOLD  50
#endif

/* Packet Counter */
static uint32_t nbuf_tx_mgmt[QDF_NBUF_TX_PKT_STATE_MAX];
static uint32_t nbuf_tx_data[QDF_NBUF_TX_PKT_STATE_MAX];
#ifdef QDF_NBUF_GLOBAL_COUNT
#define NBUF_DEBUGFS_NAME      "nbuf_counters"
static qdf_atomic_t nbuf_count;
#endif

#if defined(NBUF_MEMORY_DEBUG)
static bool is_initial_mem_debug_disabled;
#endif

/**
 * qdf_nbuf_tx_desc_count_display() - Displays the packet counter
 *
 * Return: none
 */
void qdf_nbuf_tx_desc_count_display(void)
{
	qdf_debug("Current Snapshot of the Driver:");
	qdf_debug("Data Packets:");
	qdf_debug("HDD %d TXRX_Q %d TXRX %d HTT %d",
		  nbuf_tx_data[QDF_NBUF_TX_PKT_HDD] -
		  (nbuf_tx_data[QDF_NBUF_TX_PKT_TXRX] +
		  nbuf_tx_data[QDF_NBUF_TX_PKT_TXRX_ENQUEUE] -
		  nbuf_tx_data[QDF_NBUF_TX_PKT_TXRX_DEQUEUE]),
		  nbuf_tx_data[QDF_NBUF_TX_PKT_TXRX_ENQUEUE] -
		  nbuf_tx_data[QDF_NBUF_TX_PKT_TXRX_DEQUEUE],
		  nbuf_tx_data[QDF_NBUF_TX_PKT_TXRX] -
		  nbuf_tx_data[QDF_NBUF_TX_PKT_HTT],
		  nbuf_tx_data[QDF_NBUF_TX_PKT_HTT]  -
		  nbuf_tx_data[QDF_NBUF_TX_PKT_HTC]);
	qdf_debug(" HTC %d  HIF %d CE %d TX_COMP %d",
		  nbuf_tx_data[QDF_NBUF_TX_PKT_HTC] -
		  nbuf_tx_data[QDF_NBUF_TX_PKT_HIF],
		  nbuf_tx_data[QDF_NBUF_TX_PKT_HIF] -
		  nbuf_tx_data[QDF_NBUF_TX_PKT_CE],
		  nbuf_tx_data[QDF_NBUF_TX_PKT_CE] -
		  nbuf_tx_data[QDF_NBUF_TX_PKT_FREE],
		  nbuf_tx_data[QDF_NBUF_TX_PKT_FREE]);
	qdf_debug("Mgmt Packets:");
	qdf_debug("TXRX_Q %d TXRX %d HTT %d HTC %d HIF %d CE %d TX_COMP %d",
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_TXRX_ENQUEUE] -
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_TXRX_DEQUEUE],
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_TXRX] -
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_HTT],
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_HTT] -
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_HTC],
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_HTC] -
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_HIF],
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_HIF] -
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_CE],
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_CE] -
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_FREE],
		  nbuf_tx_mgmt[QDF_NBUF_TX_PKT_FREE]);
}
qdf_export_symbol(qdf_nbuf_tx_desc_count_display);

/**
 * qdf_nbuf_tx_desc_count_update() - Updates the layer packet counter
 * @packet_type   : packet type either mgmt/data
 * @current_state : layer at which the packet currently present
 *
 * Return: none
 */
static inline void qdf_nbuf_tx_desc_count_update(uint8_t packet_type,
			uint8_t current_state)
{
	switch (packet_type) {
	case QDF_NBUF_TX_PKT_MGMT_TRACK:
		nbuf_tx_mgmt[current_state]++;
		break;
	case QDF_NBUF_TX_PKT_DATA_TRACK:
		nbuf_tx_data[current_state]++;
		break;
	default:
		break;
	}
}
qdf_export_symbol(qdf_nbuf_tx_desc_count_update);

/**
 * qdf_nbuf_tx_desc_count_clear() - Clears packet counter for both data, mgmt
 *
 * Return: none
 */
void qdf_nbuf_tx_desc_count_clear(void)
{
	memset(nbuf_tx_mgmt, 0, sizeof(nbuf_tx_mgmt));
	memset(nbuf_tx_data, 0, sizeof(nbuf_tx_data));
}
qdf_export_symbol(qdf_nbuf_tx_desc_count_clear);

/**
 * qdf_nbuf_set_state() - Updates the packet state
 * @nbuf:            network buffer
 * @current_state :  layer at which the packet currently is
 *
 * This function updates the packet state to the layer at which the packet
 * currently is
 *
 * Return: none
 */
void qdf_nbuf_set_state(qdf_nbuf_t nbuf, uint8_t current_state)
{
	/*
	 * Only Mgmt, Data Packets are tracked. WMI messages
	 * such as scan commands are not tracked
	 */
	uint8_t packet_type;

	packet_type = QDF_NBUF_CB_TX_PACKET_TRACK(nbuf);

	if ((packet_type != QDF_NBUF_TX_PKT_DATA_TRACK) &&
		(packet_type != QDF_NBUF_TX_PKT_MGMT_TRACK)) {
		return;
	}
	QDF_NBUF_CB_TX_PACKET_STATE(nbuf) = current_state;
	qdf_nbuf_tx_desc_count_update(packet_type,
					current_state);
}
qdf_export_symbol(qdf_nbuf_set_state);

#ifdef FEATURE_NBUFF_REPLENISH_TIMER
/**
 * __qdf_nbuf_start_replenish_timer - Start alloc fail replenish timer
 *
 * This function starts the alloc fail replenish timer.
 *
 * Return: void
 */
static void __qdf_nbuf_start_replenish_timer(void)
{
	qdf_atomic_inc(&alloc_track_timer.alloc_fail_cnt);
	if (qdf_mc_timer_get_current_state(&alloc_track_timer.track_timer) !=
	    QDF_TIMER_STATE_RUNNING)
		qdf_mc_timer_start(&alloc_track_timer.track_timer,
				   QDF_NBUF_ALLOC_EXPIRE_TIMER_MS);
}

/**
 * __qdf_nbuf_stop_replenish_timer - Stop alloc fail replenish timer
 *
 * This function stops the alloc fail replenish timer.
 *
 * Return: void
 */
static void __qdf_nbuf_stop_replenish_timer(void)
{
	if (qdf_atomic_read(&alloc_track_timer.alloc_fail_cnt) == 0)
		return;

	qdf_atomic_set(&alloc_track_timer.alloc_fail_cnt, 0);
	if (qdf_mc_timer_get_current_state(&alloc_track_timer.track_timer) ==
	    QDF_TIMER_STATE_RUNNING)
		qdf_mc_timer_stop(&alloc_track_timer.track_timer);
}

/**
 * qdf_replenish_expire_handler - Replenish expire handler
 *
 * This function triggers when the alloc fail replenish timer expires.
 *
 * Return: void
 */
static void qdf_replenish_expire_handler(void *arg)
{
	if (qdf_atomic_read(&alloc_track_timer.alloc_fail_cnt) >
	    QDF_NBUF_ALLOC_EXPIRE_CNT_THRESHOLD) {
		qdf_print("ERROR: NBUF allocation timer expired Fail count %d",
			  qdf_atomic_read(&alloc_track_timer.alloc_fail_cnt));

		/* Error handling here */
	}
}

/**
 * __qdf_nbuf_init_replenish_timer - Initialize the alloc replenish timer
 *
 * This function initializes the nbuf alloc fail replenish timer.
 *
 * Return: void
 */
void __qdf_nbuf_init_replenish_timer(void)
{
	qdf_mc_timer_init(&alloc_track_timer.track_timer, QDF_TIMER_TYPE_SW,
			  qdf_replenish_expire_handler, NULL);
}

/**
 * __qdf_nbuf_deinit_replenish_timer - Deinitialize the alloc replenish timer
 *
 * This function deinitializes the nbuf alloc fail replenish timer.
 *
 * Return: void
 */
void __qdf_nbuf_deinit_replenish_timer(void)
{
	__qdf_nbuf_stop_replenish_timer();
	qdf_mc_timer_destroy(&alloc_track_timer.track_timer);
}
#else

static inline void __qdf_nbuf_start_replenish_timer(void) {}
static inline void __qdf_nbuf_stop_replenish_timer(void) {}
#endif

/* globals do not need to be initialized to NULL/0 */
qdf_nbuf_trace_update_t qdf_trace_update_cb;
qdf_nbuf_free_t nbuf_free_cb;

#ifdef QDF_NBUF_GLOBAL_COUNT

/**
 * __qdf_nbuf_count_get() - get nbuf global count
 *
 * Return: nbuf global count
 */
int __qdf_nbuf_count_get(void)
{
	return qdf_atomic_read(&nbuf_count);
}
qdf_export_symbol(__qdf_nbuf_count_get);

/**
 * __qdf_nbuf_count_inc() - increment nbuf global count
 *
 * @buf: sk buff
 *
 * Return: void
 */
void __qdf_nbuf_count_inc(qdf_nbuf_t nbuf)
{
	int num_nbuf = 1;
	qdf_nbuf_t ext_list = qdf_nbuf_get_ext_list(nbuf);

	/* Take care to account for frag_list */
	while (ext_list) {
		++num_nbuf;
		ext_list = qdf_nbuf_queue_next(ext_list);
	}

	qdf_atomic_add(num_nbuf, &nbuf_count);
}
qdf_export_symbol(__qdf_nbuf_count_inc);

/**
 * __qdf_nbuf_count_dec() - decrement nbuf global count
 *
 * @buf: sk buff
 *
 * Return: void
 */
void __qdf_nbuf_count_dec(__qdf_nbuf_t nbuf)
{
	qdf_nbuf_t ext_list;
	int num_nbuf;

	if (qdf_nbuf_get_users(nbuf) > 1)
		return;

	num_nbuf = 1;

	/* Take care to account for frag_list */
	ext_list = qdf_nbuf_get_ext_list(nbuf);
	while (ext_list) {
		if (qdf_nbuf_get_users(ext_list) == 1)
			++num_nbuf;
		ext_list = qdf_nbuf_queue_next(ext_list);
	}

	qdf_atomic_sub(num_nbuf, &nbuf_count);
}
qdf_export_symbol(__qdf_nbuf_count_dec);
#endif

#if defined(QCA_WIFI_QCA8074_VP) && defined(BUILD_X86) && \
	!defined(QCA_WIFI_QCN9000)
struct sk_buff *__qdf_nbuf_alloc(qdf_device_t osdev, size_t size, int reserve,
				 int align, int prio, const char *func,
				 uint32_t line)
{
	struct sk_buff *skb;
	unsigned long offset;
	uint32_t lowmem_alloc_tries = 0;

	if (align)
		size += (align - 1);

realloc:
	skb = dev_alloc_skb(size);

	if (skb)
		goto skb_alloc;

	skb = pld_nbuf_pre_alloc(size);

	if (!skb) {
		qdf_rl_nofl_err("NBUF alloc failed %zuB @ %s:%d",
				size, func, line);
		return NULL;
	}

skb_alloc:
	/* Hawkeye M2M emulation cannot handle memory addresses below 0x50000040
	 * Though we are trying to reserve low memory upfront to prevent this,
	 * we sometimes see SKBs allocated from low memory.
	 */
	if (virt_to_phys(qdf_nbuf_data(skb)) < 0x50000040) {
		lowmem_alloc_tries++;
		if (lowmem_alloc_tries > 100) {
			qdf_nofl_err("NBUF alloc failed %zuB @ %s:%d",
				     size, func, line);
			return NULL;
		} else {
			/* Not freeing to make sure it
			 * will not get allocated again
			 */
			goto realloc;
		}
	}
	memset(skb->cb, 0x0, sizeof(skb->cb));

	/*
	 * The default is for netbuf fragments to be interpreted
	 * as wordstreams rather than bytestreams.
	 */
	QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_EFRAG(skb) = 1;
	QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_NBUF(skb) = 1;

	/*
	 * XXX:how about we reserve first then align
	 * Align & make sure that the tail & data are adjusted properly
	 */

	if (align) {
		offset = ((unsigned long)skb->data) % align;
		if (offset)
			skb_reserve(skb, align - offset);
	}

	/*
	 * NOTE:alloc doesn't take responsibility if reserve unaligns the data
	 * pointer
	 */
	skb_reserve(skb, reserve);
	qdf_nbuf_count_inc(skb);

	return skb;
}
#else
struct sk_buff *__qdf_nbuf_alloc(qdf_device_t osdev, size_t size, int reserve,
				 int align, int prio, const char *func,
				 uint32_t line)
{
	struct sk_buff *skb;
	unsigned long offset;
	int flags = GFP_KERNEL;

	if (align)
		size += (align - 1);

	if (in_interrupt() || irqs_disabled() || in_atomic()) {
		flags = GFP_ATOMIC;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)
		/*
		 * Observed that kcompactd burns out CPU to make order-3 page.
		 *__netdev_alloc_skb has 4k page fallback option just in case of
		 * failing high order page allocation so we don't need to be
		 * hard. Make kcompactd rest in piece.
		 */
		flags = flags & ~__GFP_KSWAPD_RECLAIM;
#endif
	}

	skb = __netdev_alloc_skb(NULL, size, flags);

	if (skb)
		goto skb_alloc;

	skb = pld_nbuf_pre_alloc(size);

	if (!skb) {
		qdf_rl_nofl_err("NBUF alloc failed %zuB @ %s:%d",
				size, func, line);
		__qdf_nbuf_start_replenish_timer();
		return NULL;
	} else {
		__qdf_nbuf_stop_replenish_timer();
	}

skb_alloc:
	memset(skb->cb, 0x0, sizeof(skb->cb));

	/*
	 * The default is for netbuf fragments to be interpreted
	 * as wordstreams rather than bytestreams.
	 */
	QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_EFRAG(skb) = 1;
	QDF_NBUF_CB_TX_EXTRA_FRAG_WORDSTR_NBUF(skb) = 1;

	/*
	 * XXX:how about we reserve first then align
	 * Align & make sure that the tail & data are adjusted properly
	 */

	if (align) {
		offset = ((unsigned long)skb->data) % align;
		if (offset)
			skb_reserve(skb, align - offset);
	}

	/*
	 * NOTE:alloc doesn't take responsibility if reserve unaligns the data
	 * pointer
	 */
	skb_reserve(skb, reserve);
	qdf_nbuf_count_inc(skb);

	return skb;
}
#endif
qdf_export_symbol(__qdf_nbuf_alloc);

/**
 * __qdf_nbuf_free() - free the nbuf its interrupt safe
 * @skb: Pointer to network buffer
 *
 * Return: none
 */

void __qdf_nbuf_free(struct sk_buff *skb)
{
	if (pld_nbuf_pre_alloc_free(skb))
		return;

	qdf_nbuf_count_dec(skb);
	if (nbuf_free_cb)
		nbuf_free_cb(skb);
	else
		dev_kfree_skb_any(skb);
}

qdf_export_symbol(__qdf_nbuf_free);

#ifdef NBUF_MEMORY_DEBUG
enum qdf_nbuf_event_type {
	QDF_NBUF_ALLOC,
	QDF_NBUF_ALLOC_CLONE,
	QDF_NBUF_ALLOC_COPY,
	QDF_NBUF_ALLOC_FAILURE,
	QDF_NBUF_FREE,
	QDF_NBUF_MAP,
	QDF_NBUF_UNMAP,
	QDF_NBUF_ALLOC_COPY_EXPAND,
};

struct qdf_nbuf_event {
	qdf_nbuf_t nbuf;
	char func[QDF_MEM_FUNC_NAME_SIZE];
	uint32_t line;
	enum qdf_nbuf_event_type type;
	uint64_t timestamp;
};

#define QDF_NBUF_HISTORY_SIZE 4096
static qdf_atomic_t qdf_nbuf_history_index;
static struct qdf_nbuf_event qdf_nbuf_history[QDF_NBUF_HISTORY_SIZE];

static int32_t qdf_nbuf_circular_index_next(qdf_atomic_t *index, int size)
{
	int32_t next = qdf_atomic_inc_return(index);

	if (next == size)
		qdf_atomic_sub(size, index);

	return next % size;
}

static void
qdf_nbuf_history_add(qdf_nbuf_t nbuf, const char *func, uint32_t line,
		     enum qdf_nbuf_event_type type)
{
	int32_t idx = qdf_nbuf_circular_index_next(&qdf_nbuf_history_index,
						   QDF_NBUF_HISTORY_SIZE);
	struct qdf_nbuf_event *event = &qdf_nbuf_history[idx];

	event->nbuf = nbuf;
	qdf_str_lcopy(event->func, func, QDF_MEM_FUNC_NAME_SIZE);
	event->line = line;
	event->type = type;
	event->timestamp = qdf_get_log_timestamp();
}
#endif /* NBUF_MEMORY_DEBUG */

#ifdef NBUF_MAP_UNMAP_DEBUG
#define qdf_nbuf_map_tracker_bits 11 /* 2048 buckets */
qdf_tracker_declare(qdf_nbuf_map_tracker, qdf_nbuf_map_tracker_bits,
		    "nbuf map-no-unmap events", "nbuf map", "nbuf unmap");

static void qdf_nbuf_map_tracking_init(void)
{
	qdf_tracker_init(&qdf_nbuf_map_tracker);
}

static void qdf_nbuf_map_tracking_deinit(void)
{
	qdf_tracker_deinit(&qdf_nbuf_map_tracker);
}

static QDF_STATUS
qdf_nbuf_track_map(qdf_nbuf_t nbuf, const char *func, uint32_t line)
{
	QDF_STATUS status;

	if (is_initial_mem_debug_disabled)
		return QDF_STATUS_SUCCESS;

	status = qdf_tracker_track(&qdf_nbuf_map_tracker, nbuf, func, line);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	qdf_nbuf_history_add(nbuf, func, line, QDF_NBUF_MAP);

	return QDF_STATUS_SUCCESS;
}

static void
qdf_nbuf_untrack_map(qdf_nbuf_t nbuf, const char *func, uint32_t line)
{
	if (is_initial_mem_debug_disabled)
		return;

	qdf_nbuf_history_add(nbuf, func, line, QDF_NBUF_UNMAP);
	qdf_tracker_untrack(&qdf_nbuf_map_tracker, nbuf, func, line);
}

void qdf_nbuf_map_check_for_leaks(void)
{
	qdf_tracker_check_for_leaks(&qdf_nbuf_map_tracker);
}

QDF_STATUS qdf_nbuf_map_debug(qdf_device_t osdev,
			      qdf_nbuf_t buf,
			      qdf_dma_dir_t dir,
			      const char *func,
			      uint32_t line)
{
	QDF_STATUS status;

	status = qdf_nbuf_track_map(buf, func, line);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = __qdf_nbuf_map(osdev, buf, dir);
	if (QDF_IS_STATUS_ERROR(status))
		qdf_nbuf_untrack_map(buf, func, line);

	return status;
}

qdf_export_symbol(qdf_nbuf_map_debug);

void qdf_nbuf_unmap_debug(qdf_device_t osdev,
			  qdf_nbuf_t buf,
			  qdf_dma_dir_t dir,
			  const char *func,
			  uint32_t line)
{
	qdf_nbuf_untrack_map(buf, func, line);
	__qdf_nbuf_unmap_single(osdev, buf, dir);
}

qdf_export_symbol(qdf_nbuf_unmap_debug);

QDF_STATUS qdf_nbuf_map_single_debug(qdf_device_t osdev,
				     qdf_nbuf_t buf,
				     qdf_dma_dir_t dir,
				     const char *func,
				     uint32_t line)
{
	QDF_STATUS status;

	status = qdf_nbuf_track_map(buf, func, line);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = __qdf_nbuf_map_single(osdev, buf, dir);
	if (QDF_IS_STATUS_ERROR(status))
		qdf_nbuf_untrack_map(buf, func, line);

	return status;
}

qdf_export_symbol(qdf_nbuf_map_single_debug);

void qdf_nbuf_unmap_single_debug(qdf_device_t osdev,
				 qdf_nbuf_t buf,
				 qdf_dma_dir_t dir,
				 const char *func,
				 uint32_t line)
{
	qdf_nbuf_untrack_map(buf, func, line);
	__qdf_nbuf_unmap_single(osdev, buf, dir);
}

qdf_export_symbol(qdf_nbuf_unmap_single_debug);

QDF_STATUS qdf_nbuf_map_nbytes_debug(qdf_device_t osdev,
				     qdf_nbuf_t buf,
				     qdf_dma_dir_t dir,
				     int nbytes,
				     const char *func,
				     uint32_t line)
{
	QDF_STATUS status;

	status = qdf_nbuf_track_map(buf, func, line);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = __qdf_nbuf_map_nbytes(osdev, buf, dir, nbytes);
	if (QDF_IS_STATUS_ERROR(status))
		qdf_nbuf_untrack_map(buf, func, line);

	return status;
}

qdf_export_symbol(qdf_nbuf_map_nbytes_debug);

void qdf_nbuf_unmap_nbytes_debug(qdf_device_t osdev,
				 qdf_nbuf_t buf,
				 qdf_dma_dir_t dir,
				 int nbytes,
				 const char *func,
				 uint32_t line)
{
	qdf_nbuf_untrack_map(buf, func, line);
	__qdf_nbuf_unmap_nbytes(osdev, buf, dir, nbytes);
}

qdf_export_symbol(qdf_nbuf_unmap_nbytes_debug);

QDF_STATUS qdf_nbuf_map_nbytes_single_debug(qdf_device_t osdev,
					    qdf_nbuf_t buf,
					    qdf_dma_dir_t dir,
					    int nbytes,
					    const char *func,
					    uint32_t line)
{
	QDF_STATUS status;

	status = qdf_nbuf_track_map(buf, func, line);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = __qdf_nbuf_map_nbytes_single(osdev, buf, dir, nbytes);
	if (QDF_IS_STATUS_ERROR(status))
		qdf_nbuf_untrack_map(buf, func, line);

	return status;
}

qdf_export_symbol(qdf_nbuf_map_nbytes_single_debug);

void qdf_nbuf_unmap_nbytes_single_debug(qdf_device_t osdev,
					qdf_nbuf_t buf,
					qdf_dma_dir_t dir,
					int nbytes,
					const char *func,
					uint32_t line)
{
	qdf_nbuf_untrack_map(buf, func, line);
	__qdf_nbuf_unmap_nbytes_single(osdev, buf, dir, nbytes);
}

qdf_export_symbol(qdf_nbuf_unmap_nbytes_single_debug);

static void qdf_nbuf_panic_on_free_if_mapped(qdf_nbuf_t nbuf,
					     const char *func,
					     uint32_t line)
{
	char map_func[QDF_TRACKER_FUNC_SIZE];
	uint32_t map_line;

	if (!qdf_tracker_lookup(&qdf_nbuf_map_tracker, nbuf,
				&map_func, &map_line))
		return;

	QDF_DEBUG_PANIC("Nbuf freed @ %s:%u while mapped from %s:%u",
			func, line, map_func, map_line);
}
#else
static inline void qdf_nbuf_map_tracking_init(void)
{
}

static inline void qdf_nbuf_map_tracking_deinit(void)
{
}

static inline void qdf_nbuf_panic_on_free_if_mapped(qdf_nbuf_t nbuf,
						    const char *func,
						    uint32_t line)
{
}
#endif /* NBUF_MAP_UNMAP_DEBUG */

/**
 * __qdf_nbuf_map() - map a buffer to local bus address space
 * @osdev: OS device
 * @bmap: Bitmap
 * @skb: Pointer to network buffer
 * @dir: Direction
 *
 * Return: QDF_STATUS
 */
#ifdef QDF_OS_DEBUG
QDF_STATUS
__qdf_nbuf_map(qdf_device_t osdev, struct sk_buff *skb, qdf_dma_dir_t dir)
{
	struct skb_shared_info *sh = skb_shinfo(skb);

	qdf_assert((dir == QDF_DMA_TO_DEVICE)
			|| (dir == QDF_DMA_FROM_DEVICE));

	/*
	 * Assume there's only a single fragment.
	 * To support multiple fragments, it would be necessary to change
	 * qdf_nbuf_t to be a separate object that stores meta-info
	 * (including the bus address for each fragment) and a pointer
	 * to the underlying sk_buff.
	 */
	qdf_assert(sh->nr_frags == 0);

	return __qdf_nbuf_map_single(osdev, skb, dir);
}
qdf_export_symbol(__qdf_nbuf_map);

#else
QDF_STATUS
__qdf_nbuf_map(qdf_device_t osdev, struct sk_buff *skb, qdf_dma_dir_t dir)
{
	return __qdf_nbuf_map_single(osdev, skb, dir);
}
qdf_export_symbol(__qdf_nbuf_map);
#endif
/**
 * __qdf_nbuf_unmap() - to unmap a previously mapped buf
 * @osdev: OS device
 * @skb: Pointer to network buffer
 * @dir: dma direction
 *
 * Return: none
 */
void
__qdf_nbuf_unmap(qdf_device_t osdev, struct sk_buff *skb,
			qdf_dma_dir_t dir)
{
	qdf_assert((dir == QDF_DMA_TO_DEVICE)
		   || (dir == QDF_DMA_FROM_DEVICE));

	/*
	 * Assume there's a single fragment.
	 * If this is not true, the assertion in __qdf_nbuf_map will catch it.
	 */
	__qdf_nbuf_unmap_single(osdev, skb, dir);
}
qdf_export_symbol(__qdf_nbuf_unmap);

/**
 * __qdf_nbuf_map_single() - map a single buffer to local bus address space
 * @osdev: OS device
 * @skb: Pointer to network buffer
 * @dir: Direction
 *
 * Return: QDF_STATUS
 */
#if defined(A_SIMOS_DEVHOST) || defined(HIF_USB) || defined(HIF_SDIO)
QDF_STATUS
__qdf_nbuf_map_single(qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	qdf_dma_addr_t paddr;

	QDF_NBUF_CB_PADDR(buf) = paddr = (uintptr_t)buf->data;
	BUILD_BUG_ON(sizeof(paddr) < sizeof(buf->data));
	BUILD_BUG_ON(sizeof(QDF_NBUF_CB_PADDR(buf)) < sizeof(buf->data));
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(__qdf_nbuf_map_single);
#else
QDF_STATUS
__qdf_nbuf_map_single(qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	qdf_dma_addr_t paddr;

	/* assume that the OS only provides a single fragment */
	QDF_NBUF_CB_PADDR(buf) = paddr =
		dma_map_single(osdev->dev, buf->data,
				skb_end_pointer(buf) - buf->data,
				__qdf_dma_dir_to_os(dir));
	return dma_mapping_error(osdev->dev, paddr)
		? QDF_STATUS_E_FAILURE
		: QDF_STATUS_SUCCESS;
}
qdf_export_symbol(__qdf_nbuf_map_single);
#endif
/**
 * __qdf_nbuf_unmap_single() -  unmap a previously mapped buf
 * @osdev: OS device
 * @skb: Pointer to network buffer
 * @dir: Direction
 *
 * Return: none
 */
#if defined(A_SIMOS_DEVHOST) || defined(HIF_USB) || defined(HIF_SDIO)
void __qdf_nbuf_unmap_single(qdf_device_t osdev, qdf_nbuf_t buf,
				qdf_dma_dir_t dir)
{
}
#else
void __qdf_nbuf_unmap_single(qdf_device_t osdev, qdf_nbuf_t buf,
					qdf_dma_dir_t dir)
{
	if (QDF_NBUF_CB_PADDR(buf))
		dma_unmap_single(osdev->dev, QDF_NBUF_CB_PADDR(buf),
			skb_end_pointer(buf) - buf->data,
			__qdf_dma_dir_to_os(dir));
}
#endif
qdf_export_symbol(__qdf_nbuf_unmap_single);

/**
 * __qdf_nbuf_set_rx_cksum() - set rx checksum
 * @skb: Pointer to network buffer
 * @cksum: Pointer to checksum value
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
__qdf_nbuf_set_rx_cksum(struct sk_buff *skb, qdf_nbuf_rx_cksum_t *cksum)
{
	switch (cksum->l4_result) {
	case QDF_NBUF_RX_CKSUM_NONE:
		skb->ip_summed = CHECKSUM_NONE;
		break;
	case QDF_NBUF_RX_CKSUM_TCP_UDP_UNNECESSARY:
		skb->ip_summed = CHECKSUM_UNNECESSARY;
		break;
	case QDF_NBUF_RX_CKSUM_TCP_UDP_HW:
		skb->ip_summed = CHECKSUM_PARTIAL;
		skb->csum = cksum->val;
		break;
	default:
		pr_err("Unknown checksum type\n");
		qdf_assert(0);
		return QDF_STATUS_E_NOSUPPORT;
	}
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(__qdf_nbuf_set_rx_cksum);

/**
 * __qdf_nbuf_get_tx_cksum() - get tx checksum
 * @skb: Pointer to network buffer
 *
 * Return: TX checksum value
 */
qdf_nbuf_tx_cksum_t __qdf_nbuf_get_tx_cksum(struct sk_buff *skb)
{
	switch (skb->ip_summed) {
	case CHECKSUM_NONE:
		return QDF_NBUF_TX_CKSUM_NONE;
	case CHECKSUM_PARTIAL:
		return QDF_NBUF_TX_CKSUM_TCP_UDP;
	case CHECKSUM_COMPLETE:
		return QDF_NBUF_TX_CKSUM_TCP_UDP_IP;
	default:
		return QDF_NBUF_TX_CKSUM_NONE;
	}
}
qdf_export_symbol(__qdf_nbuf_get_tx_cksum);

/**
 * __qdf_nbuf_get_tid() - get tid
 * @skb: Pointer to network buffer
 *
 * Return: tid
 */
uint8_t __qdf_nbuf_get_tid(struct sk_buff *skb)
{
	return skb->priority;
}
qdf_export_symbol(__qdf_nbuf_get_tid);

/**
 * __qdf_nbuf_set_tid() - set tid
 * @skb: Pointer to network buffer
 *
 * Return: none
 */
void __qdf_nbuf_set_tid(struct sk_buff *skb, uint8_t tid)
{
	skb->priority = tid;
}
qdf_export_symbol(__qdf_nbuf_set_tid);

/**
 * __qdf_nbuf_set_tid() - set tid
 * @skb: Pointer to network buffer
 *
 * Return: none
 */
uint8_t __qdf_nbuf_get_exemption_type(struct sk_buff *skb)
{
	return QDF_NBUF_EXEMPT_NO_EXEMPTION;
}
qdf_export_symbol(__qdf_nbuf_get_exemption_type);

/**
 * __qdf_nbuf_reg_trace_cb() - register trace callback
 * @cb_func_ptr: Pointer to trace callback function
 *
 * Return: none
 */
void __qdf_nbuf_reg_trace_cb(qdf_nbuf_trace_update_t cb_func_ptr)
{
	qdf_trace_update_cb = cb_func_ptr;
}
qdf_export_symbol(__qdf_nbuf_reg_trace_cb);

/**
 * __qdf_nbuf_data_get_dhcp_subtype() - get the subtype
 *              of DHCP packet.
 * @data: Pointer to DHCP packet data buffer
 *
 * This func. returns the subtype of DHCP packet.
 *
 * Return: subtype of the DHCP packet.
 */
enum qdf_proto_subtype
__qdf_nbuf_data_get_dhcp_subtype(uint8_t *data)
{
	enum qdf_proto_subtype subtype = QDF_PROTO_INVALID;

	if ((data[QDF_DHCP_OPTION53_OFFSET] == QDF_DHCP_OPTION53) &&
		(data[QDF_DHCP_OPTION53_LENGTH_OFFSET] ==
					QDF_DHCP_OPTION53_LENGTH)) {

		switch (data[QDF_DHCP_OPTION53_STATUS_OFFSET]) {
		case QDF_DHCP_DISCOVER:
			subtype = QDF_PROTO_DHCP_DISCOVER;
			break;
		case QDF_DHCP_REQUEST:
			subtype = QDF_PROTO_DHCP_REQUEST;
			break;
		case QDF_DHCP_OFFER:
			subtype = QDF_PROTO_DHCP_OFFER;
			break;
		case QDF_DHCP_ACK:
			subtype = QDF_PROTO_DHCP_ACK;
			break;
		case QDF_DHCP_NAK:
			subtype = QDF_PROTO_DHCP_NACK;
			break;
		case QDF_DHCP_RELEASE:
			subtype = QDF_PROTO_DHCP_RELEASE;
			break;
		case QDF_DHCP_INFORM:
			subtype = QDF_PROTO_DHCP_INFORM;
			break;
		case QDF_DHCP_DECLINE:
			subtype = QDF_PROTO_DHCP_DECLINE;
			break;
		default:
			break;
		}
	}

	return subtype;
}

/**
 * __qdf_nbuf_data_get_eapol_subtype() - get the subtype
 *            of EAPOL packet.
 * @data: Pointer to EAPOL packet data buffer
 *
 * This func. returns the subtype of EAPOL packet.
 *
 * Return: subtype of the EAPOL packet.
 */
enum qdf_proto_subtype
__qdf_nbuf_data_get_eapol_subtype(uint8_t *data)
{
	uint16_t eapol_key_info;
	enum qdf_proto_subtype subtype = QDF_PROTO_INVALID;
	uint16_t mask;

	eapol_key_info = (uint16_t)(*(uint16_t *)
			(data + EAPOL_KEY_INFO_OFFSET));

	mask = eapol_key_info & EAPOL_MASK;
	switch (mask) {
	case EAPOL_M1_BIT_MASK:
		subtype = QDF_PROTO_EAPOL_M1;
		break;
	case EAPOL_M2_BIT_MASK:
		subtype = QDF_PROTO_EAPOL_M2;
		break;
	case EAPOL_M3_BIT_MASK:
		subtype = QDF_PROTO_EAPOL_M3;
		break;
	case EAPOL_M4_BIT_MASK:
		subtype = QDF_PROTO_EAPOL_M4;
		break;
	default:
		break;
	}

	return subtype;
}

/**
 * __qdf_nbuf_data_get_arp_subtype() - get the subtype
 *            of ARP packet.
 * @data: Pointer to ARP packet data buffer
 *
 * This func. returns the subtype of ARP packet.
 *
 * Return: subtype of the ARP packet.
 */
enum qdf_proto_subtype
__qdf_nbuf_data_get_arp_subtype(uint8_t *data)
{
	uint16_t subtype;
	enum qdf_proto_subtype proto_subtype = QDF_PROTO_INVALID;

	subtype = (uint16_t)(*(uint16_t *)
			(data + ARP_SUB_TYPE_OFFSET));

	switch (QDF_SWAP_U16(subtype)) {
	case ARP_REQUEST:
		proto_subtype = QDF_PROTO_ARP_REQ;
		break;
	case ARP_RESPONSE:
		proto_subtype = QDF_PROTO_ARP_RES;
		break;
	default:
		break;
	}

	return proto_subtype;
}

/**
 * __qdf_nbuf_data_get_icmp_subtype() - get the subtype
 *            of IPV4 ICMP packet.
 * @data: Pointer to IPV4 ICMP packet data buffer
 *
 * This func. returns the subtype of ICMP packet.
 *
 * Return: subtype of the ICMP packet.
 */
enum qdf_proto_subtype
__qdf_nbuf_data_get_icmp_subtype(uint8_t *data)
{
	uint8_t subtype;
	enum qdf_proto_subtype proto_subtype = QDF_PROTO_INVALID;

	subtype = (uint8_t)(*(uint8_t *)
			(data + ICMP_SUBTYPE_OFFSET));

	switch (subtype) {
	case ICMP_REQUEST:
		proto_subtype = QDF_PROTO_ICMP_REQ;
		break;
	case ICMP_RESPONSE:
		proto_subtype = QDF_PROTO_ICMP_RES;
		break;
	default:
		break;
	}

	return proto_subtype;
}

/**
 * __qdf_nbuf_data_get_icmpv6_subtype() - get the subtype
 *            of IPV6 ICMPV6 packet.
 * @data: Pointer to IPV6 ICMPV6 packet data buffer
 *
 * This func. returns the subtype of ICMPV6 packet.
 *
 * Return: subtype of the ICMPV6 packet.
 */
enum qdf_proto_subtype
__qdf_nbuf_data_get_icmpv6_subtype(uint8_t *data)
{
	uint8_t subtype;
	enum qdf_proto_subtype proto_subtype = QDF_PROTO_INVALID;

	subtype = (uint8_t)(*(uint8_t *)
			(data + ICMPV6_SUBTYPE_OFFSET));

	switch (subtype) {
	case ICMPV6_REQUEST:
		proto_subtype = QDF_PROTO_ICMPV6_REQ;
		break;
	case ICMPV6_RESPONSE:
		proto_subtype = QDF_PROTO_ICMPV6_RES;
		break;
	case ICMPV6_RS:
		proto_subtype = QDF_PROTO_ICMPV6_RS;
		break;
	case ICMPV6_RA:
		proto_subtype = QDF_PROTO_ICMPV6_RA;
		break;
	case ICMPV6_NS:
		proto_subtype = QDF_PROTO_ICMPV6_NS;
		break;
	case ICMPV6_NA:
		proto_subtype = QDF_PROTO_ICMPV6_NA;
		break;
	default:
		break;
	}

	return proto_subtype;
}

/**
 * __qdf_nbuf_data_get_ipv4_proto() - get the proto type
 *            of IPV4 packet.
 * @data: Pointer to IPV4 packet data buffer
 *
 * This func. returns the proto type of IPV4 packet.
 *
 * Return: proto type of IPV4 packet.
 */
uint8_t
__qdf_nbuf_data_get_ipv4_proto(uint8_t *data)
{
	uint8_t proto_type;

	proto_type = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_TRAC_IPV4_PROTO_TYPE_OFFSET));
	return proto_type;
}

/**
 * __qdf_nbuf_data_get_ipv6_proto() - get the proto type
 *            of IPV6 packet.
 * @data: Pointer to IPV6 packet data buffer
 *
 * This func. returns the proto type of IPV6 packet.
 *
 * Return: proto type of IPV6 packet.
 */
uint8_t
__qdf_nbuf_data_get_ipv6_proto(uint8_t *data)
{
	uint8_t proto_type;

	proto_type = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_TRAC_IPV6_PROTO_TYPE_OFFSET));
	return proto_type;
}

/**
 * __qdf_nbuf_data_is_ipv4_pkt() - check if packet is a ipv4 packet
 * @data: Pointer to network data
 *
 * This api is for Tx packets.
 *
 * Return: true if packet is ipv4 packet
 *	   false otherwise
 */
bool __qdf_nbuf_data_is_ipv4_pkt(uint8_t *data)
{
	uint16_t ether_type;

	ether_type = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_TRAC_ETH_TYPE_OFFSET));

	if (ether_type == QDF_SWAP_U16(QDF_NBUF_TRAC_IPV4_ETH_TYPE))
		return true;
	else
		return false;
}
qdf_export_symbol(__qdf_nbuf_data_is_ipv4_pkt);

/**
 * __qdf_nbuf_data_is_ipv4_dhcp_pkt() - check if skb data is a dhcp packet
 * @data: Pointer to network data buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: true if packet is DHCP packet
 *	   false otherwise
 */
bool __qdf_nbuf_data_is_ipv4_dhcp_pkt(uint8_t *data)
{
	uint16_t sport;
	uint16_t dport;

	sport = (uint16_t)(*(uint16_t *)(data + QDF_NBUF_TRAC_IPV4_OFFSET +
					 QDF_NBUF_TRAC_IPV4_HEADER_SIZE));
	dport = (uint16_t)(*(uint16_t *)(data + QDF_NBUF_TRAC_IPV4_OFFSET +
					 QDF_NBUF_TRAC_IPV4_HEADER_SIZE +
					 sizeof(uint16_t)));

	if (((sport == QDF_SWAP_U16(QDF_NBUF_TRAC_DHCP_SRV_PORT)) &&
	     (dport == QDF_SWAP_U16(QDF_NBUF_TRAC_DHCP_CLI_PORT))) ||
	    ((sport == QDF_SWAP_U16(QDF_NBUF_TRAC_DHCP_CLI_PORT)) &&
	     (dport == QDF_SWAP_U16(QDF_NBUF_TRAC_DHCP_SRV_PORT))))
		return true;
	else
		return false;
}
qdf_export_symbol(__qdf_nbuf_data_is_ipv4_dhcp_pkt);

/**
 * __qdf_nbuf_data_is_ipv4_eapol_pkt() - check if skb data is a eapol packet
 * @data: Pointer to network data buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: true if packet is EAPOL packet
 *	   false otherwise.
 */
bool __qdf_nbuf_data_is_ipv4_eapol_pkt(uint8_t *data)
{
	uint16_t ether_type;

	ether_type = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_TRAC_ETH_TYPE_OFFSET));

	if (ether_type == QDF_SWAP_U16(QDF_NBUF_TRAC_EAPOL_ETH_TYPE))
		return true;
	else
		return false;
}
qdf_export_symbol(__qdf_nbuf_data_is_ipv4_eapol_pkt);

/**
 * __qdf_nbuf_is_ipv4_wapi_pkt() - check if skb data is a wapi packet
 * @skb: Pointer to network buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: true if packet is WAPI packet
 *	   false otherwise.
 */
bool __qdf_nbuf_is_ipv4_wapi_pkt(struct sk_buff *skb)
{
	uint16_t ether_type;

	ether_type = (uint16_t)(*(uint16_t *)(skb->data +
				QDF_NBUF_TRAC_ETH_TYPE_OFFSET));

	if (ether_type == QDF_SWAP_U16(QDF_NBUF_TRAC_WAPI_ETH_TYPE))
		return true;
	else
		return false;
}
qdf_export_symbol(__qdf_nbuf_is_ipv4_wapi_pkt);

/**
 * __qdf_nbuf_is_ipv4_tdls_pkt() - check if skb data is a tdls packet
 * @skb: Pointer to network buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: true if packet is tdls packet
 *	   false otherwise.
 */
bool __qdf_nbuf_is_ipv4_tdls_pkt(struct sk_buff *skb)
{
	uint16_t ether_type;

	ether_type = *(uint16_t *)(skb->data +
				QDF_NBUF_TRAC_ETH_TYPE_OFFSET);

	if (ether_type == QDF_SWAP_U16(QDF_NBUF_TRAC_TDLS_ETH_TYPE))
		return true;
	else
		return false;
}
qdf_export_symbol(__qdf_nbuf_is_ipv4_tdls_pkt);

/**
 * __qdf_nbuf_data_is_ipv4_arp_pkt() - check if skb data is a arp packet
 * @data: Pointer to network data buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: true if packet is ARP packet
 *	   false otherwise.
 */
bool __qdf_nbuf_data_is_ipv4_arp_pkt(uint8_t *data)
{
	uint16_t ether_type;

	ether_type = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_TRAC_ETH_TYPE_OFFSET));

	if (ether_type == QDF_SWAP_U16(QDF_NBUF_TRAC_ARP_ETH_TYPE))
		return true;
	else
		return false;
}
qdf_export_symbol(__qdf_nbuf_data_is_ipv4_arp_pkt);

/**
 * __qdf_nbuf_data_is_arp_req() - check if skb data is a arp request
 * @data: Pointer to network data buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: true if packet is ARP request
 *	   false otherwise.
 */
bool __qdf_nbuf_data_is_arp_req(uint8_t *data)
{
	uint16_t op_code;

	op_code = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_PKT_ARP_OPCODE_OFFSET));

	if (op_code == QDF_SWAP_U16(QDF_NBUF_PKT_ARPOP_REQ))
		return true;
	return false;
}

/**
 * __qdf_nbuf_data_is_arp_rsp() - check if skb data is a arp response
 * @data: Pointer to network data buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: true if packet is ARP response
 *	   false otherwise.
 */
bool __qdf_nbuf_data_is_arp_rsp(uint8_t *data)
{
	uint16_t op_code;

	op_code = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_PKT_ARP_OPCODE_OFFSET));

	if (op_code == QDF_SWAP_U16(QDF_NBUF_PKT_ARPOP_REPLY))
		return true;
	return false;
}

/**
 * __qdf_nbuf_data_get_arp_src_ip() - get arp src IP
 * @data: Pointer to network data buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: ARP packet source IP value.
 */
uint32_t  __qdf_nbuf_get_arp_src_ip(uint8_t *data)
{
	uint32_t src_ip;

	src_ip = (uint32_t)(*(uint32_t *)(data +
				QDF_NBUF_PKT_ARP_SRC_IP_OFFSET));

	return src_ip;
}

/**
 * __qdf_nbuf_data_get_arp_tgt_ip() - get arp target IP
 * @data: Pointer to network data buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: ARP packet target IP value.
 */
uint32_t  __qdf_nbuf_get_arp_tgt_ip(uint8_t *data)
{
	uint32_t tgt_ip;

	tgt_ip = (uint32_t)(*(uint32_t *)(data +
				QDF_NBUF_PKT_ARP_TGT_IP_OFFSET));

	return tgt_ip;
}

/**
 * __qdf_nbuf_get_dns_domain_name() - get dns domain name
 * @data: Pointer to network data buffer
 * @len: length to copy
 *
 * This api is for dns domain name
 *
 * Return: dns domain name.
 */
uint8_t *__qdf_nbuf_get_dns_domain_name(uint8_t *data, uint32_t len)
{
	uint8_t *domain_name;

	domain_name = (uint8_t *)
			(data + QDF_NBUF_PKT_DNS_NAME_OVER_UDP_OFFSET);
	return domain_name;
}


/**
 * __qdf_nbuf_data_is_dns_query() - check if skb data is a dns query
 * @data: Pointer to network data buffer
 *
 * This api is for dns query packet.
 *
 * Return: true if packet is dns query packet.
 *	   false otherwise.
 */
bool __qdf_nbuf_data_is_dns_query(uint8_t *data)
{
	uint16_t op_code;
	uint16_t tgt_port;

	tgt_port = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_PKT_DNS_DST_PORT_OFFSET));
	/* Standard DNS query always happen on Dest Port 53. */
	if (tgt_port == QDF_SWAP_U16(QDF_NBUF_PKT_DNS_STANDARD_PORT)) {
		op_code = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_PKT_DNS_OVER_UDP_OPCODE_OFFSET));
		if ((QDF_SWAP_U16(op_code) & QDF_NBUF_PKT_DNSOP_BITMAP) ==
				QDF_NBUF_PKT_DNSOP_STANDARD_QUERY)
			return true;
	}
	return false;
}

/**
 * __qdf_nbuf_data_is_dns_response() - check if skb data is a dns response
 * @data: Pointer to network data buffer
 *
 * This api is for dns query response.
 *
 * Return: true if packet is dns response packet.
 *	   false otherwise.
 */
bool __qdf_nbuf_data_is_dns_response(uint8_t *data)
{
	uint16_t op_code;
	uint16_t src_port;

	src_port = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_PKT_DNS_SRC_PORT_OFFSET));
	/* Standard DNS response always comes on Src Port 53. */
	if (src_port == QDF_SWAP_U16(QDF_NBUF_PKT_DNS_STANDARD_PORT)) {
		op_code = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_PKT_DNS_OVER_UDP_OPCODE_OFFSET));

		if ((QDF_SWAP_U16(op_code) & QDF_NBUF_PKT_DNSOP_BITMAP) ==
				QDF_NBUF_PKT_DNSOP_STANDARD_RESPONSE)
			return true;
	}
	return false;
}

/**
 * __qdf_nbuf_data_is_tcp_syn() - check if skb data is a tcp syn
 * @data: Pointer to network data buffer
 *
 * This api is for tcp syn packet.
 *
 * Return: true if packet is tcp syn packet.
 *	   false otherwise.
 */
bool __qdf_nbuf_data_is_tcp_syn(uint8_t *data)
{
	uint8_t op_code;

	op_code = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_PKT_TCP_OPCODE_OFFSET));

	if (op_code == QDF_NBUF_PKT_TCPOP_SYN)
		return true;
	return false;
}

/**
 * __qdf_nbuf_data_is_tcp_syn_ack() - check if skb data is a tcp syn ack
 * @data: Pointer to network data buffer
 *
 * This api is for tcp syn ack packet.
 *
 * Return: true if packet is tcp syn ack packet.
 *	   false otherwise.
 */
bool __qdf_nbuf_data_is_tcp_syn_ack(uint8_t *data)
{
	uint8_t op_code;

	op_code = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_PKT_TCP_OPCODE_OFFSET));

	if (op_code == QDF_NBUF_PKT_TCPOP_SYN_ACK)
		return true;
	return false;
}

/**
 * __qdf_nbuf_data_is_tcp_ack() - check if skb data is a tcp ack
 * @data: Pointer to network data buffer
 *
 * This api is for tcp ack packet.
 *
 * Return: true if packet is tcp ack packet.
 *	   false otherwise.
 */
bool __qdf_nbuf_data_is_tcp_ack(uint8_t *data)
{
	uint8_t op_code;

	op_code = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_PKT_TCP_OPCODE_OFFSET));

	if (op_code == QDF_NBUF_PKT_TCPOP_ACK)
		return true;
	return false;
}

/**
 * __qdf_nbuf_data_get_tcp_src_port() - get tcp src port
 * @data: Pointer to network data buffer
 *
 * This api is for tcp packet.
 *
 * Return: tcp source port value.
 */
uint16_t __qdf_nbuf_data_get_tcp_src_port(uint8_t *data)
{
	uint16_t src_port;

	src_port = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_PKT_TCP_SRC_PORT_OFFSET));

	return src_port;
}

/**
 * __qdf_nbuf_data_get_tcp_dst_port() - get tcp dst port
 * @data: Pointer to network data buffer
 *
 * This api is for tcp packet.
 *
 * Return: tcp destination port value.
 */
uint16_t __qdf_nbuf_data_get_tcp_dst_port(uint8_t *data)
{
	uint16_t tgt_port;

	tgt_port = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_PKT_TCP_DST_PORT_OFFSET));

	return tgt_port;
}

/**
 * __qdf_nbuf_data_is_icmpv4_req() - check if skb data is a icmpv4 request
 * @data: Pointer to network data buffer
 *
 * This api is for ipv4 req packet.
 *
 * Return: true if packet is icmpv4 request
 *	   false otherwise.
 */
bool __qdf_nbuf_data_is_icmpv4_req(uint8_t *data)
{
	uint8_t op_code;

	op_code = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_PKT_ICMPv4_OPCODE_OFFSET));

	if (op_code == QDF_NBUF_PKT_ICMPv4OP_REQ)
		return true;
	return false;
}

/**
 * __qdf_nbuf_data_is_icmpv4_rsp() - check if skb data is a icmpv4 res
 * @data: Pointer to network data buffer
 *
 * This api is for ipv4 res packet.
 *
 * Return: true if packet is icmpv4 response
 *	   false otherwise.
 */
bool __qdf_nbuf_data_is_icmpv4_rsp(uint8_t *data)
{
	uint8_t op_code;

	op_code = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_PKT_ICMPv4_OPCODE_OFFSET));

	if (op_code == QDF_NBUF_PKT_ICMPv4OP_REPLY)
		return true;
	return false;
}

/**
 * __qdf_nbuf_data_get_icmpv4_src_ip() - get icmpv4 src IP
 * @data: Pointer to network data buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: icmpv4 packet source IP value.
 */
uint32_t __qdf_nbuf_get_icmpv4_src_ip(uint8_t *data)
{
	uint32_t src_ip;

	src_ip = (uint32_t)(*(uint32_t *)(data +
				QDF_NBUF_PKT_ICMPv4_SRC_IP_OFFSET));

	return src_ip;
}

/**
 * __qdf_nbuf_data_get_icmpv4_tgt_ip() - get icmpv4 target IP
 * @data: Pointer to network data buffer
 *
 * This api is for ipv4 packet.
 *
 * Return: icmpv4 packet target IP value.
 */
uint32_t __qdf_nbuf_get_icmpv4_tgt_ip(uint8_t *data)
{
	uint32_t tgt_ip;

	tgt_ip = (uint32_t)(*(uint32_t *)(data +
				QDF_NBUF_PKT_ICMPv4_TGT_IP_OFFSET));

	return tgt_ip;
}


/**
 * __qdf_nbuf_data_is_ipv6_pkt() - check if it is IPV6 packet.
 * @data: Pointer to IPV6 packet data buffer
 *
 * This func. checks whether it is a IPV6 packet or not.
 *
 * Return: TRUE if it is a IPV6 packet
 *         FALSE if not
 */
bool __qdf_nbuf_data_is_ipv6_pkt(uint8_t *data)
{
	uint16_t ether_type;

	ether_type = (uint16_t)(*(uint16_t *)(data +
				QDF_NBUF_TRAC_ETH_TYPE_OFFSET));

	if (ether_type == QDF_SWAP_U16(QDF_NBUF_TRAC_IPV6_ETH_TYPE))
		return true;
	else
		return false;
}
qdf_export_symbol(__qdf_nbuf_data_is_ipv6_pkt);

/**
 * __qdf_nbuf_data_is_ipv6_dhcp_pkt() - check if skb data is a dhcp packet
 * @data: Pointer to network data buffer
 *
 * This api is for ipv6 packet.
 *
 * Return: true if packet is DHCP packet
 *	   false otherwise
 */
bool __qdf_nbuf_data_is_ipv6_dhcp_pkt(uint8_t *data)
{
	uint16_t sport;
	uint16_t dport;

	sport = *(uint16_t *)(data + QDF_NBUF_TRAC_IPV6_OFFSET +
				QDF_NBUF_TRAC_IPV6_HEADER_SIZE);
	dport = *(uint16_t *)(data + QDF_NBUF_TRAC_IPV6_OFFSET +
					QDF_NBUF_TRAC_IPV6_HEADER_SIZE +
					sizeof(uint16_t));

	if (((sport == QDF_SWAP_U16(QDF_NBUF_TRAC_DHCP6_SRV_PORT)) &&
	     (dport == QDF_SWAP_U16(QDF_NBUF_TRAC_DHCP6_CLI_PORT))) ||
	    ((sport == QDF_SWAP_U16(QDF_NBUF_TRAC_DHCP6_CLI_PORT)) &&
	     (dport == QDF_SWAP_U16(QDF_NBUF_TRAC_DHCP6_SRV_PORT))))
		return true;
	else
		return false;
}
qdf_export_symbol(__qdf_nbuf_data_is_ipv6_dhcp_pkt);

/**
 * __qdf_nbuf_data_is_ipv6_mdns_pkt() - check if skb data is a mdns packet
 * @data: Pointer to network data buffer
 *
 * This api is for ipv6 packet.
 *
 * Return: true if packet is MDNS packet
 *	   false otherwise
 */
bool __qdf_nbuf_data_is_ipv6_mdns_pkt(uint8_t *data)
{
	uint16_t sport;
	uint16_t dport;

	sport = *(uint16_t *)(data + QDF_NBUF_TRAC_IPV6_OFFSET +
				QDF_NBUF_TRAC_IPV6_HEADER_SIZE);
	dport = *(uint16_t *)(data + QDF_NBUF_TRAC_IPV6_OFFSET +
					QDF_NBUF_TRAC_IPV6_HEADER_SIZE +
					sizeof(uint16_t));

	if (sport == QDF_SWAP_U16(QDF_NBUF_TRAC_MDNS_SRC_N_DST_PORT) &&
	    dport == sport)
		return true;
	else
		return false;
}

qdf_export_symbol(__qdf_nbuf_data_is_ipv6_mdns_pkt);

/**
 * __qdf_nbuf_data_is_ipv4_mcast_pkt() - check if it is IPV4 multicast packet.
 * @data: Pointer to IPV4 packet data buffer
 *
 * This func. checks whether it is a IPV4 multicast packet or not.
 *
 * Return: TRUE if it is a IPV4 multicast packet
 *         FALSE if not
 */
bool __qdf_nbuf_data_is_ipv4_mcast_pkt(uint8_t *data)
{
	if (__qdf_nbuf_data_is_ipv4_pkt(data)) {
		uint32_t *dst_addr =
		      (uint32_t *)(data + QDF_NBUF_TRAC_IPV4_DEST_ADDR_OFFSET);

		/*
		 * Check first word of the IPV4 address and if it is
		 * equal to 0xE then it represents multicast IP.
		 */
		if ((*dst_addr & QDF_NBUF_TRAC_IPV4_ADDR_BCAST_MASK) ==
				QDF_NBUF_TRAC_IPV4_ADDR_MCAST_MASK)
			return true;
		else
			return false;
	} else
		return false;
}

/**
 * __qdf_nbuf_data_is_ipv6_mcast_pkt() - check if it is IPV6 multicast packet.
 * @data: Pointer to IPV6 packet data buffer
 *
 * This func. checks whether it is a IPV6 multicast packet or not.
 *
 * Return: TRUE if it is a IPV6 multicast packet
 *         FALSE if not
 */
bool __qdf_nbuf_data_is_ipv6_mcast_pkt(uint8_t *data)
{
	if (__qdf_nbuf_data_is_ipv6_pkt(data)) {
		uint16_t *dst_addr;

		dst_addr = (uint16_t *)
			(data + QDF_NBUF_TRAC_IPV6_DEST_ADDR_OFFSET);

		/*
		 * Check first byte of the IP address and if it
		 * 0xFF00 then it is a IPV6 mcast packet.
		 */
		if (*dst_addr ==
		     QDF_SWAP_U16(QDF_NBUF_TRAC_IPV6_DEST_ADDR))
			return true;
		else
			return false;
	} else
		return false;
}

/**
 * __qdf_nbuf_data_is_icmp_pkt() - check if it is IPV4 ICMP packet.
 * @data: Pointer to IPV4 ICMP packet data buffer
 *
 * This func. checks whether it is a ICMP packet or not.
 *
 * Return: TRUE if it is a ICMP packet
 *         FALSE if not
 */
bool __qdf_nbuf_data_is_icmp_pkt(uint8_t *data)
{
	if (__qdf_nbuf_data_is_ipv4_pkt(data)) {
		uint8_t pkt_type;

		pkt_type = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_TRAC_IPV4_PROTO_TYPE_OFFSET));

		if (pkt_type == QDF_NBUF_TRAC_ICMP_TYPE)
			return true;
		else
			return false;
	} else
		return false;
}

/**
 * __qdf_nbuf_data_is_icmpv6_pkt() - check if it is IPV6 ICMPV6 packet.
 * @data: Pointer to IPV6 ICMPV6 packet data buffer
 *
 * This func. checks whether it is a ICMPV6 packet or not.
 *
 * Return: TRUE if it is a ICMPV6 packet
 *         FALSE if not
 */
bool __qdf_nbuf_data_is_icmpv6_pkt(uint8_t *data)
{
	if (__qdf_nbuf_data_is_ipv6_pkt(data)) {
		uint8_t pkt_type;

		pkt_type = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_TRAC_IPV6_PROTO_TYPE_OFFSET));

		if (pkt_type == QDF_NBUF_TRAC_ICMPV6_TYPE)
			return true;
		else
			return false;
	} else
		return false;
}

/**
 * __qdf_nbuf_data_is_ipv4_udp_pkt() - check if it is IPV4 UDP packet.
 * @data: Pointer to IPV4 UDP packet data buffer
 *
 * This func. checks whether it is a IPV4 UDP packet or not.
 *
 * Return: TRUE if it is a IPV4 UDP packet
 *         FALSE if not
 */
bool __qdf_nbuf_data_is_ipv4_udp_pkt(uint8_t *data)
{
	if (__qdf_nbuf_data_is_ipv4_pkt(data)) {
		uint8_t pkt_type;

		pkt_type = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_TRAC_IPV4_PROTO_TYPE_OFFSET));

		if (pkt_type == QDF_NBUF_TRAC_UDP_TYPE)
			return true;
		else
			return false;
	} else
		return false;
}

/**
 * __qdf_nbuf_data_is_ipv4_tcp_pkt() - check if it is IPV4 TCP packet.
 * @data: Pointer to IPV4 TCP packet data buffer
 *
 * This func. checks whether it is a IPV4 TCP packet or not.
 *
 * Return: TRUE if it is a IPV4 TCP packet
 *         FALSE if not
 */
bool __qdf_nbuf_data_is_ipv4_tcp_pkt(uint8_t *data)
{
	if (__qdf_nbuf_data_is_ipv4_pkt(data)) {
		uint8_t pkt_type;

		pkt_type = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_TRAC_IPV4_PROTO_TYPE_OFFSET));

		if (pkt_type == QDF_NBUF_TRAC_TCP_TYPE)
			return true;
		else
			return false;
	} else
		return false;
}

/**
 * __qdf_nbuf_data_is_ipv6_udp_pkt() - check if it is IPV6 UDP packet.
 * @data: Pointer to IPV6 UDP packet data buffer
 *
 * This func. checks whether it is a IPV6 UDP packet or not.
 *
 * Return: TRUE if it is a IPV6 UDP packet
 *         FALSE if not
 */
bool __qdf_nbuf_data_is_ipv6_udp_pkt(uint8_t *data)
{
	if (__qdf_nbuf_data_is_ipv6_pkt(data)) {
		uint8_t pkt_type;

		pkt_type = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_TRAC_IPV6_PROTO_TYPE_OFFSET));

		if (pkt_type == QDF_NBUF_TRAC_UDP_TYPE)
			return true;
		else
			return false;
	} else
		return false;
}

/**
 * __qdf_nbuf_data_is_ipv6_tcp_pkt() - check if it is IPV6 TCP packet.
 * @data: Pointer to IPV6 TCP packet data buffer
 *
 * This func. checks whether it is a IPV6 TCP packet or not.
 *
 * Return: TRUE if it is a IPV6 TCP packet
 *         FALSE if not
 */
bool __qdf_nbuf_data_is_ipv6_tcp_pkt(uint8_t *data)
{
	if (__qdf_nbuf_data_is_ipv6_pkt(data)) {
		uint8_t pkt_type;

		pkt_type = (uint8_t)(*(uint8_t *)(data +
				QDF_NBUF_TRAC_IPV6_PROTO_TYPE_OFFSET));

		if (pkt_type == QDF_NBUF_TRAC_TCP_TYPE)
			return true;
		else
			return false;
	} else
		return false;
}

/**
 * __qdf_nbuf_is_bcast_pkt() - is destination address broadcast
 * @nbuf - sk buff
 *
 * Return: true if packet is broadcast
 *	   false otherwise
 */
bool __qdf_nbuf_is_bcast_pkt(qdf_nbuf_t nbuf)
{
	struct ethhdr *eh = (struct ethhdr *)qdf_nbuf_data(nbuf);
	return qdf_is_macaddr_broadcast((struct qdf_mac_addr *)eh->h_dest);
}
qdf_export_symbol(__qdf_nbuf_is_bcast_pkt);

#ifdef NBUF_MEMORY_DEBUG
#define QDF_NET_BUF_TRACK_MAX_SIZE    (1024)

/**
 * struct qdf_nbuf_track_t - Network buffer track structure
 *
 * @p_next: Pointer to next
 * @net_buf: Pointer to network buffer
 * @func_name: Function name
 * @line_num: Line number
 * @size: Size
 */
struct qdf_nbuf_track_t {
	struct qdf_nbuf_track_t *p_next;
	qdf_nbuf_t net_buf;
	char func_name[QDF_MEM_FUNC_NAME_SIZE];
	uint32_t line_num;
	size_t size;
};

static spinlock_t g_qdf_net_buf_track_lock[QDF_NET_BUF_TRACK_MAX_SIZE];
typedef struct qdf_nbuf_track_t QDF_NBUF_TRACK;

static QDF_NBUF_TRACK *gp_qdf_net_buf_track_tbl[QDF_NET_BUF_TRACK_MAX_SIZE];
static struct kmem_cache *nbuf_tracking_cache;
static QDF_NBUF_TRACK *qdf_net_buf_track_free_list;
static spinlock_t qdf_net_buf_track_free_list_lock;
static uint32_t qdf_net_buf_track_free_list_count;
static uint32_t qdf_net_buf_track_used_list_count;
static uint32_t qdf_net_buf_track_max_used;
static uint32_t qdf_net_buf_track_max_free;
static uint32_t qdf_net_buf_track_max_allocated;

/**
 * update_max_used() - update qdf_net_buf_track_max_used tracking variable
 *
 * tracks the max number of network buffers that the wlan driver was tracking
 * at any one time.
 *
 * Return: none
 */
static inline void update_max_used(void)
{
	int sum;

	if (qdf_net_buf_track_max_used <
	    qdf_net_buf_track_used_list_count)
		qdf_net_buf_track_max_used = qdf_net_buf_track_used_list_count;
	sum = qdf_net_buf_track_free_list_count +
		qdf_net_buf_track_used_list_count;
	if (qdf_net_buf_track_max_allocated < sum)
		qdf_net_buf_track_max_allocated = sum;
}

/**
 * update_max_free() - update qdf_net_buf_track_free_list_count
 *
 * tracks the max number tracking buffers kept in the freelist.
 *
 * Return: none
 */
static inline void update_max_free(void)
{
	if (qdf_net_buf_track_max_free <
	    qdf_net_buf_track_free_list_count)
		qdf_net_buf_track_max_free = qdf_net_buf_track_free_list_count;
}

/**
 * qdf_nbuf_track_alloc() - allocate a cookie to track nbufs allocated by wlan
 *
 * This function pulls from a freelist if possible and uses kmem_cache_alloc.
 * This function also ads fexibility to adjust the allocation and freelist
 * scheems.
 *
 * Return: a pointer to an unused QDF_NBUF_TRACK structure may not be zeroed.
 */
static QDF_NBUF_TRACK *qdf_nbuf_track_alloc(void)
{
	int flags = GFP_KERNEL;
	unsigned long irq_flag;
	QDF_NBUF_TRACK *new_node = NULL;

	spin_lock_irqsave(&qdf_net_buf_track_free_list_lock, irq_flag);
	qdf_net_buf_track_used_list_count++;
	if (qdf_net_buf_track_free_list) {
		new_node = qdf_net_buf_track_free_list;
		qdf_net_buf_track_free_list =
			qdf_net_buf_track_free_list->p_next;
		qdf_net_buf_track_free_list_count--;
	}
	update_max_used();
	spin_unlock_irqrestore(&qdf_net_buf_track_free_list_lock, irq_flag);

	if (new_node)
		return new_node;

	if (in_interrupt() || irqs_disabled() || in_atomic())
		flags = GFP_ATOMIC;

	return kmem_cache_alloc(nbuf_tracking_cache, flags);
}

/* FREEQ_POOLSIZE initial and minimum desired freelist poolsize */
#define FREEQ_POOLSIZE 2048

/**
 * qdf_nbuf_track_free() - free the nbuf tracking cookie.
 *
 * Matches calls to qdf_nbuf_track_alloc.
 * Either frees the tracking cookie to kernel or an internal
 * freelist based on the size of the freelist.
 *
 * Return: none
 */
static void qdf_nbuf_track_free(QDF_NBUF_TRACK *node)
{
	unsigned long irq_flag;

	if (!node)
		return;

	/* Try to shrink the freelist if free_list_count > than FREEQ_POOLSIZE
	 * only shrink the freelist if it is bigger than twice the number of
	 * nbufs in use. If the driver is stalling in a consistent bursty
	 * fasion, this will keep 3/4 of thee allocations from the free list
	 * while also allowing the system to recover memory as less frantic
	 * traffic occurs.
	 */

	spin_lock_irqsave(&qdf_net_buf_track_free_list_lock, irq_flag);

	qdf_net_buf_track_used_list_count--;
	if (qdf_net_buf_track_free_list_count > FREEQ_POOLSIZE &&
	   (qdf_net_buf_track_free_list_count >
	    qdf_net_buf_track_used_list_count << 1)) {
		kmem_cache_free(nbuf_tracking_cache, node);
	} else {
		node->p_next = qdf_net_buf_track_free_list;
		qdf_net_buf_track_free_list = node;
		qdf_net_buf_track_free_list_count++;
	}
	update_max_free();
	spin_unlock_irqrestore(&qdf_net_buf_track_free_list_lock, irq_flag);
}

/**
 * qdf_nbuf_track_prefill() - prefill the nbuf tracking cookie freelist
 *
 * Removes a 'warmup time' characteristic of the freelist.  Prefilling
 * the freelist first makes it performant for the first iperf udp burst
 * as well as steady state.
 *
 * Return: None
 */
static void qdf_nbuf_track_prefill(void)
{
	int i;
	QDF_NBUF_TRACK *node, *head;

	/* prepopulate the freelist */
	head = NULL;
	for (i = 0; i < FREEQ_POOLSIZE; i++) {
		node = qdf_nbuf_track_alloc();
		if (!node)
			continue;
		node->p_next = head;
		head = node;
	}
	while (head) {
		node = head->p_next;
		qdf_nbuf_track_free(head);
		head = node;
	}

	/* prefilled buffers should not count as used */
	qdf_net_buf_track_max_used = 0;
}

/**
 * qdf_nbuf_track_memory_manager_create() - manager for nbuf tracking cookies
 *
 * This initializes the memory manager for the nbuf tracking cookies.  Because
 * these cookies are all the same size and only used in this feature, we can
 * use a kmem_cache to provide tracking as well as to speed up allocations.
 * To avoid the overhead of allocating and freeing the buffers (including SLUB
 * features) a freelist is prepopulated here.
 *
 * Return: None
 */
static void qdf_nbuf_track_memory_manager_create(void)
{
	spin_lock_init(&qdf_net_buf_track_free_list_lock);
	nbuf_tracking_cache = kmem_cache_create("qdf_nbuf_tracking_cache",
						sizeof(QDF_NBUF_TRACK),
						0, 0, NULL);

	qdf_nbuf_track_prefill();
}

/**
 * qdf_nbuf_track_memory_manager_destroy() - manager for nbuf tracking cookies
 *
 * Empty the freelist and print out usage statistics when it is no longer
 * needed. Also the kmem_cache should be destroyed here so that it can warn if
 * any nbuf tracking cookies were leaked.
 *
 * Return: None
 */
static void qdf_nbuf_track_memory_manager_destroy(void)
{
	QDF_NBUF_TRACK *node, *tmp;
	unsigned long irq_flag;

	spin_lock_irqsave(&qdf_net_buf_track_free_list_lock, irq_flag);
	node = qdf_net_buf_track_free_list;

	if (qdf_net_buf_track_max_used > FREEQ_POOLSIZE * 4)
		qdf_print("%s: unexpectedly large max_used count %d",
			  __func__, qdf_net_buf_track_max_used);

	if (qdf_net_buf_track_max_used < qdf_net_buf_track_max_allocated)
		qdf_print("%s: %d unused trackers were allocated",
			  __func__,
			  qdf_net_buf_track_max_allocated -
			  qdf_net_buf_track_max_used);

	if (qdf_net_buf_track_free_list_count > FREEQ_POOLSIZE &&
	    qdf_net_buf_track_free_list_count > 3*qdf_net_buf_track_max_used/4)
		qdf_print("%s: check freelist shrinking functionality",
			  __func__);

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		  "%s: %d residual freelist size",
		  __func__, qdf_net_buf_track_free_list_count);

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		  "%s: %d max freelist size observed",
		  __func__, qdf_net_buf_track_max_free);

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		  "%s: %d max buffers used observed",
		  __func__, qdf_net_buf_track_max_used);

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		  "%s: %d max buffers allocated observed",
		  __func__, qdf_net_buf_track_max_allocated);

	while (node) {
		tmp = node;
		node = node->p_next;
		kmem_cache_free(nbuf_tracking_cache, tmp);
		qdf_net_buf_track_free_list_count--;
	}

	if (qdf_net_buf_track_free_list_count != 0)
		qdf_info("%d unfreed tracking memory lost in freelist",
			 qdf_net_buf_track_free_list_count);

	if (qdf_net_buf_track_used_list_count != 0)
		qdf_info("%d unfreed tracking memory still in use",
			 qdf_net_buf_track_used_list_count);

	spin_unlock_irqrestore(&qdf_net_buf_track_free_list_lock, irq_flag);
	kmem_cache_destroy(nbuf_tracking_cache);
	qdf_net_buf_track_free_list = NULL;
}

/**
 * qdf_net_buf_debug_init() - initialize network buffer debug functionality
 *
 * QDF network buffer debug feature tracks all SKBs allocated by WLAN driver
 * in a hash table and when driver is unloaded it reports about leaked SKBs.
 * WLAN driver module whose allocated SKB is freed by network stack are
 * suppose to call qdf_net_buf_debug_release_skb() such that the SKB is not
 * reported as memory leak.
 *
 * Return: none
 */
void qdf_net_buf_debug_init(void)
{
	uint32_t i;

	is_initial_mem_debug_disabled = qdf_mem_debug_config_get();

	if (is_initial_mem_debug_disabled)
		return;

	qdf_atomic_set(&qdf_nbuf_history_index, -1);

	qdf_nbuf_map_tracking_init();
	qdf_nbuf_track_memory_manager_create();

	for (i = 0; i < QDF_NET_BUF_TRACK_MAX_SIZE; i++) {
		gp_qdf_net_buf_track_tbl[i] = NULL;
		spin_lock_init(&g_qdf_net_buf_track_lock[i]);
	}
}
qdf_export_symbol(qdf_net_buf_debug_init);

/**
 * qdf_net_buf_debug_init() - exit network buffer debug functionality
 *
 * Exit network buffer tracking debug functionality and log SKB memory leaks
 * As part of exiting the functionality, free the leaked memory and
 * cleanup the tracking buffers.
 *
 * Return: none
 */
void qdf_net_buf_debug_exit(void)
{
	uint32_t i;
	uint32_t count = 0;
	unsigned long irq_flag;
	QDF_NBUF_TRACK *p_node;
	QDF_NBUF_TRACK *p_prev;

	if (is_initial_mem_debug_disabled)
		return;

	for (i = 0; i < QDF_NET_BUF_TRACK_MAX_SIZE; i++) {
		spin_lock_irqsave(&g_qdf_net_buf_track_lock[i], irq_flag);
		p_node = gp_qdf_net_buf_track_tbl[i];
		while (p_node) {
			p_prev = p_node;
			p_node = p_node->p_next;
			count++;
			qdf_info("SKB buf memory Leak@ Func %s, @Line %d, size %zu, nbuf %pK",
				 p_prev->func_name, p_prev->line_num,
				 p_prev->size, p_prev->net_buf);
			qdf_nbuf_track_free(p_prev);
		}
		spin_unlock_irqrestore(&g_qdf_net_buf_track_lock[i], irq_flag);
	}

	qdf_nbuf_track_memory_manager_destroy();
	qdf_nbuf_map_tracking_deinit();

#ifdef CONFIG_HALT_KMEMLEAK
	if (count) {
		qdf_err("%d SKBs leaked .. please fix the SKB leak", count);
		QDF_BUG(0);
	}
#endif
}
qdf_export_symbol(qdf_net_buf_debug_exit);

/**
 * qdf_net_buf_debug_hash() - hash network buffer pointer
 *
 * Return: hash value
 */
static uint32_t qdf_net_buf_debug_hash(qdf_nbuf_t net_buf)
{
	uint32_t i;

	i = (uint32_t) (((uintptr_t) net_buf) >> 4);
	i += (uint32_t) (((uintptr_t) net_buf) >> 14);
	i &= (QDF_NET_BUF_TRACK_MAX_SIZE - 1);

	return i;
}

/**
 * qdf_net_buf_debug_look_up() - look up network buffer in debug hash table
 *
 * Return: If skb is found in hash table then return pointer to network buffer
 *	else return %NULL
 */
static QDF_NBUF_TRACK *qdf_net_buf_debug_look_up(qdf_nbuf_t net_buf)
{
	uint32_t i;
	QDF_NBUF_TRACK *p_node;

	i = qdf_net_buf_debug_hash(net_buf);
	p_node = gp_qdf_net_buf_track_tbl[i];

	while (p_node) {
		if (p_node->net_buf == net_buf)
			return p_node;
		p_node = p_node->p_next;
	}

	return NULL;
}

/**
 * qdf_net_buf_debug_add_node() - store skb in debug hash table
 *
 * Return: none
 */
void qdf_net_buf_debug_add_node(qdf_nbuf_t net_buf, size_t size,
				const char *func_name, uint32_t line_num)
{
	uint32_t i;
	unsigned long irq_flag;
	QDF_NBUF_TRACK *p_node;
	QDF_NBUF_TRACK *new_node;

	if (is_initial_mem_debug_disabled)
		return;

	new_node = qdf_nbuf_track_alloc();

	i = qdf_net_buf_debug_hash(net_buf);
	spin_lock_irqsave(&g_qdf_net_buf_track_lock[i], irq_flag);

	p_node = qdf_net_buf_debug_look_up(net_buf);

	if (p_node) {
		qdf_print("Double allocation of skb ! Already allocated from %pK %s %d current alloc from %pK %s %d",
			  p_node->net_buf, p_node->func_name, p_node->line_num,
			  net_buf, func_name, line_num);
		qdf_nbuf_track_free(new_node);
	} else {
		p_node = new_node;
		if (p_node) {
			p_node->net_buf = net_buf;
			qdf_str_lcopy(p_node->func_name, func_name,
				      QDF_MEM_FUNC_NAME_SIZE);
			p_node->line_num = line_num;
			p_node->size = size;
			qdf_mem_skb_inc(size);
			p_node->p_next = gp_qdf_net_buf_track_tbl[i];
			gp_qdf_net_buf_track_tbl[i] = p_node;
		} else
			qdf_print(
				  "Mem alloc failed ! Could not track skb from %s %d of size %zu",
				  func_name, line_num, size);
	}

	spin_unlock_irqrestore(&g_qdf_net_buf_track_lock[i], irq_flag);
}
qdf_export_symbol(qdf_net_buf_debug_add_node);

void qdf_net_buf_debug_update_node(qdf_nbuf_t net_buf, const char *func_name,
				   uint32_t line_num)
{
	uint32_t i;
	unsigned long irq_flag;
	QDF_NBUF_TRACK *p_node;

	if (is_initial_mem_debug_disabled)
		return;

	i = qdf_net_buf_debug_hash(net_buf);
	spin_lock_irqsave(&g_qdf_net_buf_track_lock[i], irq_flag);

	p_node = qdf_net_buf_debug_look_up(net_buf);

	if (p_node) {
		qdf_str_lcopy(p_node->func_name, kbasename(func_name),
			      QDF_MEM_FUNC_NAME_SIZE);
		p_node->line_num = line_num;
	}

	spin_unlock_irqrestore(&g_qdf_net_buf_track_lock[i], irq_flag);
}

qdf_export_symbol(qdf_net_buf_debug_update_node);

/**
 * qdf_net_buf_debug_delete_node() - remove skb from debug hash table
 *
 * Return: none
 */
void qdf_net_buf_debug_delete_node(qdf_nbuf_t net_buf)
{
	uint32_t i;
	QDF_NBUF_TRACK *p_head;
	QDF_NBUF_TRACK *p_node = NULL;
	unsigned long irq_flag;
	QDF_NBUF_TRACK *p_prev;

	if (is_initial_mem_debug_disabled)
		return;

	i = qdf_net_buf_debug_hash(net_buf);
	spin_lock_irqsave(&g_qdf_net_buf_track_lock[i], irq_flag);

	p_head = gp_qdf_net_buf_track_tbl[i];

	/* Unallocated SKB */
	if (!p_head)
		goto done;

	p_node = p_head;
	/* Found at head of the table */
	if (p_head->net_buf == net_buf) {
		gp_qdf_net_buf_track_tbl[i] = p_node->p_next;
		goto done;
	}

	/* Search in collision list */
	while (p_node) {
		p_prev = p_node;
		p_node = p_node->p_next;
		if ((p_node) && (p_node->net_buf == net_buf)) {
			p_prev->p_next = p_node->p_next;
			break;
		}
	}

done:
	spin_unlock_irqrestore(&g_qdf_net_buf_track_lock[i], irq_flag);

	if (p_node) {
		qdf_mem_skb_dec(p_node->size);
		qdf_nbuf_track_free(p_node);
	} else {
		qdf_print("Unallocated buffer ! Double free of net_buf %pK ?",
			  net_buf);
		QDF_BUG(0);
	}
}
qdf_export_symbol(qdf_net_buf_debug_delete_node);

void qdf_net_buf_debug_acquire_skb(qdf_nbuf_t net_buf,
				   const char *func_name, uint32_t line_num)
{
	qdf_nbuf_t ext_list = qdf_nbuf_get_ext_list(net_buf);

	if (is_initial_mem_debug_disabled)
		return;

	while (ext_list) {
		/*
		 * Take care to add if it is Jumbo packet connected using
		 * frag_list
		 */
		qdf_nbuf_t next;

		next = qdf_nbuf_queue_next(ext_list);
		qdf_net_buf_debug_add_node(ext_list, 0, func_name, line_num);
		ext_list = next;
	}
	qdf_net_buf_debug_add_node(net_buf, 0, func_name, line_num);
}
qdf_export_symbol(qdf_net_buf_debug_acquire_skb);

/**
 * qdf_net_buf_debug_release_skb() - release skb to avoid memory leak
 * @net_buf: Network buf holding head segment (single)
 *
 * WLAN driver module whose allocated SKB is freed by network stack are
 * suppose to call this API before returning SKB to network stack such
 * that the SKB is not reported as memory leak.
 *
 * Return: none
 */
void qdf_net_buf_debug_release_skb(qdf_nbuf_t net_buf)
{
	qdf_nbuf_t ext_list = qdf_nbuf_get_ext_list(net_buf);

	if (is_initial_mem_debug_disabled)
		return;

	while (ext_list) {
		/*
		 * Take care to free if it is Jumbo packet connected using
		 * frag_list
		 */
		qdf_nbuf_t next;

		next = qdf_nbuf_queue_next(ext_list);

		if (qdf_nbuf_get_users(ext_list) > 1) {
			ext_list = next;
			continue;
		}

		qdf_net_buf_debug_delete_node(ext_list);
		ext_list = next;
	}

	if (qdf_nbuf_get_users(net_buf) > 1)
		return;

	qdf_net_buf_debug_delete_node(net_buf);
}
qdf_export_symbol(qdf_net_buf_debug_release_skb);

qdf_nbuf_t qdf_nbuf_alloc_debug(qdf_device_t osdev, qdf_size_t size,
				int reserve, int align, int prio,
				const char *func, uint32_t line)
{
	qdf_nbuf_t nbuf;

	if (is_initial_mem_debug_disabled)
		return __qdf_nbuf_alloc(osdev, size,
					reserve, align,
					prio, func, line);

	nbuf = __qdf_nbuf_alloc(osdev, size, reserve, align, prio, func, line);

	/* Store SKB in internal QDF tracking table */
	if (qdf_likely(nbuf)) {
		qdf_net_buf_debug_add_node(nbuf, size, func, line);
		qdf_nbuf_history_add(nbuf, func, line, QDF_NBUF_ALLOC);
	} else {
		qdf_nbuf_history_add(nbuf, func, line, QDF_NBUF_ALLOC_FAILURE);
	}

	return nbuf;
}
qdf_export_symbol(qdf_nbuf_alloc_debug);

void qdf_nbuf_free_debug(qdf_nbuf_t nbuf, const char *func, uint32_t line)
{
	qdf_nbuf_t ext_list;

	if (qdf_unlikely(!nbuf))
		return;

	if (is_initial_mem_debug_disabled)
		goto free_buf;

	if (qdf_nbuf_get_users(nbuf) > 1)
		goto free_buf;

	/* Remove SKB from internal QDF tracking table */
	qdf_nbuf_panic_on_free_if_mapped(nbuf, func, line);
	qdf_net_buf_debug_delete_node(nbuf);
	qdf_nbuf_history_add(nbuf, func, line, QDF_NBUF_FREE);

	/* Take care to delete the debug entries for frag_list */
	ext_list = qdf_nbuf_get_ext_list(nbuf);
	while (ext_list) {
		if (qdf_nbuf_get_users(ext_list) == 1) {
			qdf_nbuf_panic_on_free_if_mapped(ext_list, func, line);
			qdf_net_buf_debug_delete_node(ext_list);
		}

		ext_list = qdf_nbuf_queue_next(ext_list);
	}

free_buf:
	__qdf_nbuf_free(nbuf);
}
qdf_export_symbol(qdf_nbuf_free_debug);

qdf_nbuf_t qdf_nbuf_clone_debug(qdf_nbuf_t buf, const char *func, uint32_t line)
{
	qdf_nbuf_t cloned_buf = __qdf_nbuf_clone(buf);

	if (is_initial_mem_debug_disabled)
		return cloned_buf;

	if (qdf_unlikely(!cloned_buf))
		return NULL;

	/* Store SKB in internal QDF tracking table */
	qdf_net_buf_debug_add_node(cloned_buf, 0, func, line);
	qdf_nbuf_history_add(cloned_buf, func, line, QDF_NBUF_ALLOC_CLONE);

	return cloned_buf;
}
qdf_export_symbol(qdf_nbuf_clone_debug);

qdf_nbuf_t qdf_nbuf_copy_debug(qdf_nbuf_t buf, const char *func, uint32_t line)
{
	qdf_nbuf_t copied_buf = __qdf_nbuf_copy(buf);

	if (is_initial_mem_debug_disabled)
		return copied_buf;

	if (qdf_unlikely(!copied_buf))
		return NULL;

	/* Store SKB in internal QDF tracking table */
	qdf_net_buf_debug_add_node(copied_buf, 0, func, line);
	qdf_nbuf_history_add(copied_buf, func, line, QDF_NBUF_ALLOC_COPY);

	return copied_buf;
}
qdf_export_symbol(qdf_nbuf_copy_debug);

qdf_nbuf_t
qdf_nbuf_copy_expand_debug(qdf_nbuf_t buf, int headroom, int tailroom,
			   const char *func, uint32_t line)
{
	qdf_nbuf_t copied_buf = __qdf_nbuf_copy_expand(buf, headroom, tailroom);

	if (qdf_unlikely(!copied_buf))
		return NULL;

	if (is_initial_mem_debug_disabled)
		return copied_buf;

	/* Store SKB in internal QDF tracking table */
	qdf_net_buf_debug_add_node(copied_buf, 0, func, line);
	qdf_nbuf_history_add(copied_buf, func, line,
			     QDF_NBUF_ALLOC_COPY_EXPAND);

	return copied_buf;
}

qdf_export_symbol(qdf_nbuf_copy_expand_debug);

#endif /* NBUF_MEMORY_DEBUG */

#if defined(FEATURE_TSO)

/**
 * struct qdf_tso_cmn_seg_info_t - TSO common info structure
 *
 * @ethproto: ethernet type of the msdu
 * @ip_tcp_hdr_len: ip + tcp length for the msdu
 * @l2_len: L2 length for the msdu
 * @eit_hdr: pointer to EIT header
 * @eit_hdr_len: EIT header length for the msdu
 * @eit_hdr_dma_map_addr: dma addr for EIT header
 * @tcphdr: pointer to tcp header
 * @ipv4_csum_en: ipv4 checksum enable
 * @tcp_ipv4_csum_en: TCP ipv4 checksum enable
 * @tcp_ipv6_csum_en: TCP ipv6 checksum enable
 * @ip_id: IP id
 * @tcp_seq_num: TCP sequence number
 *
 * This structure holds the TSO common info that is common
 * across all the TCP segments of the jumbo packet.
 */
struct qdf_tso_cmn_seg_info_t {
	uint16_t ethproto;
	uint16_t ip_tcp_hdr_len;
	uint16_t l2_len;
	uint8_t *eit_hdr;
	uint32_t eit_hdr_len;
	qdf_dma_addr_t eit_hdr_dma_map_addr;
	struct tcphdr *tcphdr;
	uint16_t ipv4_csum_en;
	uint16_t tcp_ipv4_csum_en;
	uint16_t tcp_ipv6_csum_en;
	uint16_t ip_id;
	uint32_t tcp_seq_num;
};

/**
 * __qdf_nbuf_get_tso_cmn_seg_info() - get TSO common
 * information
 * @osdev: qdf device handle
 * @skb: skb buffer
 * @tso_info: Parameters common to all segements
 *
 * Get the TSO information that is common across all the TCP
 * segments of the jumbo packet
 *
 * Return: 0 - success 1 - failure
 */
static uint8_t __qdf_nbuf_get_tso_cmn_seg_info(qdf_device_t osdev,
			struct sk_buff *skb,
			struct qdf_tso_cmn_seg_info_t *tso_info)
{
	/* Get ethernet type and ethernet header length */
	tso_info->ethproto = vlan_get_protocol(skb);

	/* Determine whether this is an IPv4 or IPv6 packet */
	if (tso_info->ethproto == htons(ETH_P_IP)) { /* IPv4 */
		/* for IPv4, get the IP ID and enable TCP and IP csum */
		struct iphdr *ipv4_hdr = ip_hdr(skb);

		tso_info->ip_id = ntohs(ipv4_hdr->id);
		tso_info->ipv4_csum_en = 1;
		tso_info->tcp_ipv4_csum_en = 1;
		if (qdf_unlikely(ipv4_hdr->protocol != IPPROTO_TCP)) {
			qdf_err("TSO IPV4 proto 0x%x not TCP",
				ipv4_hdr->protocol);
			return 1;
		}
	} else if (tso_info->ethproto == htons(ETH_P_IPV6)) { /* IPv6 */
		/* for IPv6, enable TCP csum. No IP ID or IP csum */
		tso_info->tcp_ipv6_csum_en = 1;
	} else {
		qdf_err("TSO: ethertype 0x%x is not supported!",
			tso_info->ethproto);
		return 1;
	}
	tso_info->l2_len = (skb_network_header(skb) - skb_mac_header(skb));
	tso_info->tcphdr = tcp_hdr(skb);
	tso_info->tcp_seq_num = ntohl(tcp_hdr(skb)->seq);
	/* get pointer to the ethernet + IP + TCP header and their length */
	tso_info->eit_hdr = skb->data;
	tso_info->eit_hdr_len = (skb_transport_header(skb)
		 - skb_mac_header(skb)) + tcp_hdrlen(skb);
	tso_info->eit_hdr_dma_map_addr = dma_map_single(osdev->dev,
							tso_info->eit_hdr,
							tso_info->eit_hdr_len,
							DMA_TO_DEVICE);
	if (unlikely(dma_mapping_error(osdev->dev,
				       tso_info->eit_hdr_dma_map_addr))) {
		qdf_err("DMA mapping error!");
		qdf_assert(0);
		return 1;
	}

	if (tso_info->ethproto == htons(ETH_P_IP)) {
		/* inlcude IPv4 header length for IPV4 (total length) */
		tso_info->ip_tcp_hdr_len =
			tso_info->eit_hdr_len - tso_info->l2_len;
	} else if (tso_info->ethproto == htons(ETH_P_IPV6)) {
		/* exclude IPv6 header length for IPv6 (payload length) */
		tso_info->ip_tcp_hdr_len = tcp_hdrlen(skb);
	}
	/*
	 * The length of the payload (application layer data) is added to
	 * tso_info->ip_tcp_hdr_len before passing it on to the msdu link ext
	 * descriptor.
	 */

	TSO_DEBUG("%s seq# %u eit hdr len %u l2 len %u  skb len %u\n", __func__,
		tso_info->tcp_seq_num,
		tso_info->eit_hdr_len,
		tso_info->l2_len,
		skb->len);
	return 0;
}


/**
 * __qdf_nbuf_fill_tso_cmn_seg_info() - Init function for each TSO nbuf segment
 *
 * @curr_seg: Segment whose contents are initialized
 * @tso_cmn_info: Parameters common to all segements
 *
 * Return: None
 */
static inline void __qdf_nbuf_fill_tso_cmn_seg_info(
				struct qdf_tso_seg_elem_t *curr_seg,
				struct qdf_tso_cmn_seg_info_t *tso_cmn_info)
{
	/* Initialize the flags to 0 */
	memset(&curr_seg->seg, 0x0, sizeof(curr_seg->seg));

	/*
	 * The following fields remain the same across all segments of
	 * a jumbo packet
	 */
	curr_seg->seg.tso_flags.tso_enable = 1;
	curr_seg->seg.tso_flags.ipv4_checksum_en =
		tso_cmn_info->ipv4_csum_en;
	curr_seg->seg.tso_flags.tcp_ipv6_checksum_en =
		tso_cmn_info->tcp_ipv6_csum_en;
	curr_seg->seg.tso_flags.tcp_ipv4_checksum_en =
		tso_cmn_info->tcp_ipv4_csum_en;
	curr_seg->seg.tso_flags.tcp_flags_mask = 0x1FF;

	/* The following fields change for the segments */
	curr_seg->seg.tso_flags.ip_id = tso_cmn_info->ip_id;
	tso_cmn_info->ip_id++;

	curr_seg->seg.tso_flags.syn = tso_cmn_info->tcphdr->syn;
	curr_seg->seg.tso_flags.rst = tso_cmn_info->tcphdr->rst;
	curr_seg->seg.tso_flags.psh = tso_cmn_info->tcphdr->psh;
	curr_seg->seg.tso_flags.ack = tso_cmn_info->tcphdr->ack;
	curr_seg->seg.tso_flags.urg = tso_cmn_info->tcphdr->urg;
	curr_seg->seg.tso_flags.ece = tso_cmn_info->tcphdr->ece;
	curr_seg->seg.tso_flags.cwr = tso_cmn_info->tcphdr->cwr;

	curr_seg->seg.tso_flags.tcp_seq_num = tso_cmn_info->tcp_seq_num;

	/*
	 * First fragment for each segment always contains the ethernet,
	 * IP and TCP header
	 */
	curr_seg->seg.tso_frags[0].vaddr = tso_cmn_info->eit_hdr;
	curr_seg->seg.tso_frags[0].length = tso_cmn_info->eit_hdr_len;
	curr_seg->seg.total_len = curr_seg->seg.tso_frags[0].length;
	curr_seg->seg.tso_frags[0].paddr = tso_cmn_info->eit_hdr_dma_map_addr;

	TSO_DEBUG("%s %d eit hdr %pK eit_hdr_len %d tcp_seq_num %u tso_info->total_len %u\n",
		   __func__, __LINE__, tso_cmn_info->eit_hdr,
		   tso_cmn_info->eit_hdr_len,
		   curr_seg->seg.tso_flags.tcp_seq_num,
		   curr_seg->seg.total_len);
	qdf_tso_seg_dbg_record(curr_seg, TSOSEG_LOC_FILLCMNSEG);
}

/**
 * __qdf_nbuf_get_tso_info() - function to divide a TSO nbuf
 * into segments
 * @nbuf: network buffer to be segmented
 * @tso_info: This is the output. The information about the
 *           TSO segments will be populated within this.
 *
 * This function fragments a TCP jumbo packet into smaller
 * segments to be transmitted by the driver. It chains the TSO
 * segments created into a list.
 *
 * Return: number of TSO segments
 */
uint32_t __qdf_nbuf_get_tso_info(qdf_device_t osdev, struct sk_buff *skb,
		struct qdf_tso_info_t *tso_info)
{
	/* common across all segments */
	struct qdf_tso_cmn_seg_info_t tso_cmn_info;
	/* segment specific */
	void *tso_frag_vaddr;
	qdf_dma_addr_t tso_frag_paddr = 0;
	uint32_t num_seg = 0;
	struct qdf_tso_seg_elem_t *curr_seg;
	struct qdf_tso_num_seg_elem_t *total_num_seg;
	skb_frag_t *frag = NULL;
	uint32_t tso_frag_len = 0; /* tso segment's fragment length*/
	uint32_t skb_frag_len = 0; /* skb's fragment length (contiguous memory)*/
	uint32_t skb_proc = skb->len; /* bytes of skb pending processing */
	uint32_t tso_seg_size = skb_shinfo(skb)->gso_size;
	int j = 0; /* skb fragment index */

	memset(&tso_cmn_info, 0x0, sizeof(tso_cmn_info));
	total_num_seg = tso_info->tso_num_seg_list;
	curr_seg = tso_info->tso_seg_list;
	total_num_seg->num_seg.tso_cmn_num_seg = 0;

	if (qdf_unlikely(__qdf_nbuf_get_tso_cmn_seg_info(osdev,
						skb, &tso_cmn_info))) {
		qdf_warn("TSO: error getting common segment info");
		return 0;
	}

	/* length of the first chunk of data in the skb */
	skb_frag_len = skb_headlen(skb);

	/* the 0th tso segment's 0th fragment always contains the EIT header */
	/* update the remaining skb fragment length and TSO segment length */
	skb_frag_len -= tso_cmn_info.eit_hdr_len;
	skb_proc -= tso_cmn_info.eit_hdr_len;

	/* get the address to the next tso fragment */
	tso_frag_vaddr = skb->data + tso_cmn_info.eit_hdr_len;
	/* get the length of the next tso fragment */
	tso_frag_len = min(skb_frag_len, tso_seg_size);

	if (tso_frag_len != 0) {
		tso_frag_paddr = dma_map_single(osdev->dev,
				tso_frag_vaddr, tso_frag_len, DMA_TO_DEVICE);
	}

	if (unlikely(dma_mapping_error(osdev->dev,
					tso_frag_paddr))) {
		qdf_err("DMA mapping error!");
		qdf_assert(0);
		return 0;
	}
	TSO_DEBUG("%s[%d] skb frag len %d tso frag len %d\n", __func__,
		__LINE__, skb_frag_len, tso_frag_len);
	num_seg = tso_info->num_segs;
	tso_info->num_segs = 0;
	tso_info->is_tso = 1;

	while (num_seg && curr_seg) {
		int i = 1; /* tso fragment index */
		uint8_t more_tso_frags = 1;

		curr_seg->seg.num_frags = 0;
		tso_info->num_segs++;
		total_num_seg->num_seg.tso_cmn_num_seg++;

		__qdf_nbuf_fill_tso_cmn_seg_info(curr_seg,
						 &tso_cmn_info);

		if (unlikely(skb_proc == 0))
			return tso_info->num_segs;

		curr_seg->seg.tso_flags.ip_len = tso_cmn_info.ip_tcp_hdr_len;
		curr_seg->seg.tso_flags.l2_len = tso_cmn_info.l2_len;
		/* frag len is added to ip_len in while loop below*/

		curr_seg->seg.num_frags++;

		while (more_tso_frags) {
			if (tso_frag_len != 0) {
				curr_seg->seg.tso_frags[i].vaddr =
					tso_frag_vaddr;
				curr_seg->seg.tso_frags[i].length =
					tso_frag_len;
				curr_seg->seg.total_len += tso_frag_len;
				curr_seg->seg.tso_flags.ip_len +=  tso_frag_len;
				curr_seg->seg.num_frags++;
				skb_proc = skb_proc - tso_frag_len;

				/* increment the TCP sequence number */

				tso_cmn_info.tcp_seq_num += tso_frag_len;
				curr_seg->seg.tso_frags[i].paddr =
					tso_frag_paddr;
			}

			TSO_DEBUG("%s[%d] frag %d frag len %d total_len %u vaddr %pK\n",
					__func__, __LINE__,
					i,
					tso_frag_len,
					curr_seg->seg.total_len,
					curr_seg->seg.tso_frags[i].vaddr);

			/* if there is no more data left in the skb */
			if (!skb_proc)
				return tso_info->num_segs;

			/* get the next payload fragment information */
			/* check if there are more fragments in this segment */
			if (tso_frag_len < tso_seg_size) {
				more_tso_frags = 1;
				if (tso_frag_len != 0) {
					tso_seg_size = tso_seg_size -
						tso_frag_len;
					i++;
					if (curr_seg->seg.num_frags ==
								FRAG_NUM_MAX) {
						more_tso_frags = 0;
						/*
						 * reset i and the tso
						 * payload size
						 */
						i = 1;
						tso_seg_size =
							skb_shinfo(skb)->
								gso_size;
					}
				}
			} else {
				more_tso_frags = 0;
				/* reset i and the tso payload size */
				i = 1;
				tso_seg_size = skb_shinfo(skb)->gso_size;
			}

			/* if the next fragment is contiguous */
			if ((tso_frag_len != 0)  && (tso_frag_len < skb_frag_len)) {
				tso_frag_vaddr = tso_frag_vaddr + tso_frag_len;
				skb_frag_len = skb_frag_len - tso_frag_len;
				tso_frag_len = min(skb_frag_len, tso_seg_size);

			} else { /* the next fragment is not contiguous */
				if (skb_shinfo(skb)->nr_frags == 0) {
					qdf_info("TSO: nr_frags == 0!");
					qdf_assert(0);
					return 0;
				}
				if (j >= skb_shinfo(skb)->nr_frags) {
					qdf_info("TSO: nr_frags %d j %d",
						 skb_shinfo(skb)->nr_frags, j);
					qdf_assert(0);
					return 0;
				}
				frag = &skb_shinfo(skb)->frags[j];
				skb_frag_len = skb_frag_size(frag);
				tso_frag_len = min(skb_frag_len, tso_seg_size);
				tso_frag_vaddr = skb_frag_address_safe(frag);
				j++;
			}

			TSO_DEBUG("%s[%d] skb frag len %d tso frag %d len tso_seg_size %d\n",
				__func__, __LINE__, skb_frag_len, tso_frag_len,
				tso_seg_size);

			if (!(tso_frag_vaddr)) {
				TSO_DEBUG("%s: Fragment virtual addr is NULL",
						__func__);
				return 0;
			}

			tso_frag_paddr =
					 dma_map_single(osdev->dev,
						 tso_frag_vaddr,
						 tso_frag_len,
						 DMA_TO_DEVICE);
			if (unlikely(dma_mapping_error(osdev->dev,
							tso_frag_paddr))) {
				qdf_err("DMA mapping error!");
				qdf_assert(0);
				return 0;
			}
		}
		TSO_DEBUG("%s tcp_seq_num: %u", __func__,
				curr_seg->seg.tso_flags.tcp_seq_num);
		num_seg--;
		/* if TCP FIN flag was set, set it in the last segment */
		if (!num_seg)
			curr_seg->seg.tso_flags.fin = tso_cmn_info.tcphdr->fin;

		qdf_tso_seg_dbg_record(curr_seg, TSOSEG_LOC_GETINFO);
		curr_seg = curr_seg->next;
	}
	return tso_info->num_segs;
}
qdf_export_symbol(__qdf_nbuf_get_tso_info);

/**
 * __qdf_nbuf_unmap_tso_segment() - function to dma unmap TSO segment element
 *
 * @osdev: qdf device handle
 * @tso_seg: TSO segment element to be unmapped
 * @is_last_seg: whether this is last tso seg or not
 *
 * Return: none
 */
void __qdf_nbuf_unmap_tso_segment(qdf_device_t osdev,
			  struct qdf_tso_seg_elem_t *tso_seg,
			  bool is_last_seg)
{
	uint32_t num_frags = 0;

	if (tso_seg->seg.num_frags > 0)
		num_frags = tso_seg->seg.num_frags - 1;

	/*Num of frags in a tso seg cannot be less than 2 */
	if (num_frags < 1) {
		/*
		 * If Num of frags is 1 in a tso seg but is_last_seg true,
		 * this may happen when qdf_nbuf_get_tso_info failed,
		 * do dma unmap for the 0th frag in this seg.
		 */
		if (is_last_seg && tso_seg->seg.num_frags == 1)
			goto last_seg_free_first_frag;

		qdf_assert(0);
		qdf_err("ERROR: num of frags in a tso segment is %d",
			(num_frags + 1));
		return;
	}

	while (num_frags) {
		/*Do dma unmap the tso seg except the 0th frag */
		if (0 ==  tso_seg->seg.tso_frags[num_frags].paddr) {
			qdf_err("ERROR: TSO seg frag %d mapped physical address is NULL",
				num_frags);
			qdf_assert(0);
			return;
		}
		dma_unmap_single(osdev->dev,
				 tso_seg->seg.tso_frags[num_frags].paddr,
				 tso_seg->seg.tso_frags[num_frags].length,
				 __qdf_dma_dir_to_os(QDF_DMA_TO_DEVICE));
		tso_seg->seg.tso_frags[num_frags].paddr = 0;
		num_frags--;
		qdf_tso_seg_dbg_record(tso_seg, TSOSEG_LOC_UNMAPTSO);
	}

last_seg_free_first_frag:
	if (is_last_seg) {
		/*Do dma unmap for the tso seg 0th frag */
		if (0 ==  tso_seg->seg.tso_frags[0].paddr) {
			qdf_err("ERROR: TSO seg frag 0 mapped physical address is NULL");
			qdf_assert(0);
			return;
		}
		dma_unmap_single(osdev->dev,
				 tso_seg->seg.tso_frags[0].paddr,
				 tso_seg->seg.tso_frags[0].length,
				 __qdf_dma_dir_to_os(QDF_DMA_TO_DEVICE));
		tso_seg->seg.tso_frags[0].paddr = 0;
		qdf_tso_seg_dbg_record(tso_seg, TSOSEG_LOC_UNMAPLAST);
	}
}
qdf_export_symbol(__qdf_nbuf_unmap_tso_segment);

size_t __qdf_nbuf_get_tcp_payload_len(struct sk_buff *skb)
{
	size_t packet_len;

	packet_len = skb->len -
		((skb_transport_header(skb) - skb_mac_header(skb)) +
		 tcp_hdrlen(skb));

	return packet_len;
}

qdf_export_symbol(__qdf_nbuf_get_tcp_payload_len);

/**
 * __qdf_nbuf_get_tso_num_seg() - function to divide a TSO nbuf
 * into segments
 * @nbuf:   network buffer to be segmented
 * @tso_info:  This is the output. The information about the
 *      TSO segments will be populated within this.
 *
 * This function fragments a TCP jumbo packet into smaller
 * segments to be transmitted by the driver. It chains the TSO
 * segments created into a list.
 *
 * Return: 0 - success, 1 - failure
 */
#ifndef BUILD_X86
uint32_t __qdf_nbuf_get_tso_num_seg(struct sk_buff *skb)
{
	uint32_t tso_seg_size = skb_shinfo(skb)->gso_size;
	uint32_t remainder, num_segs = 0;
	uint8_t skb_nr_frags = skb_shinfo(skb)->nr_frags;
	uint8_t frags_per_tso = 0;
	uint32_t skb_frag_len = 0;
	uint32_t eit_hdr_len = (skb_transport_header(skb)
			 - skb_mac_header(skb)) + tcp_hdrlen(skb);
	skb_frag_t *frag = NULL;
	int j = 0;
	uint32_t temp_num_seg = 0;

	/* length of the first chunk of data in the skb minus eit header*/
	skb_frag_len = skb_headlen(skb) - eit_hdr_len;

	/* Calculate num of segs for skb's first chunk of data*/
	remainder = skb_frag_len % tso_seg_size;
	num_segs = skb_frag_len / tso_seg_size;
	/**
	 * Remainder non-zero and nr_frags zero implies end of skb data.
	 * In that case, one more tso seg is required to accommodate
	 * remaining data, hence num_segs++. If nr_frags is non-zero,
	 * then remaining data will be accomodated while doing the calculation
	 * for nr_frags data. Hence, frags_per_tso++.
	 */
	if (remainder) {
		if (!skb_nr_frags)
			num_segs++;
		else
			frags_per_tso++;
	}

	while (skb_nr_frags) {
		if (j >= skb_shinfo(skb)->nr_frags) {
			qdf_info("TSO: nr_frags %d j %d",
				 skb_shinfo(skb)->nr_frags, j);
			qdf_assert(0);
			return 0;
		}
		/**
		 * Calculate the number of tso seg for nr_frags data:
		 * Get the length of each frag in skb_frag_len, add to
		 * remainder.Get the number of segments by dividing it to
		 * tso_seg_size and calculate the new remainder.
		 * Decrement the nr_frags value and keep
		 * looping all the skb_fragments.
		 */
		frag = &skb_shinfo(skb)->frags[j];
		skb_frag_len = skb_frag_size(frag);
		temp_num_seg = num_segs;
		remainder += skb_frag_len;
		num_segs += remainder / tso_seg_size;
		remainder = remainder % tso_seg_size;
		skb_nr_frags--;
		if (remainder) {
			if (num_segs > temp_num_seg)
				frags_per_tso = 0;
			/**
			 * increment the tso per frags whenever remainder is
			 * positive. If frags_per_tso reaches the (max-1),
			 * [First frags always have EIT header, therefore max-1]
			 * increment the num_segs as no more data can be
			 * accomodated in the curr tso seg. Reset the remainder
			 * and frags per tso and keep looping.
			 */
			frags_per_tso++;
			if (frags_per_tso == FRAG_NUM_MAX - 1) {
				num_segs++;
				frags_per_tso = 0;
				remainder = 0;
			}
			/**
			 * If this is the last skb frag and still remainder is
			 * non-zero(frags_per_tso is not reached to the max-1)
			 * then increment the num_segs to take care of the
			 * remaining length.
			 */
			if (!skb_nr_frags && remainder) {
				num_segs++;
				frags_per_tso = 0;
			}
		} else {
			 /* Whenever remainder is 0, reset the frags_per_tso. */
			frags_per_tso = 0;
		}
		j++;
	}

	return num_segs;
}
#elif !defined(QCA_WIFI_QCN9000)
uint32_t __qdf_nbuf_get_tso_num_seg(struct sk_buff *skb)
{
	uint32_t i, gso_size, tmp_len, num_segs = 0;
	skb_frag_t *frag = NULL;

	/*
	 * Check if the head SKB or any of frags are allocated in < 0x50000000
	 * region which cannot be accessed by Target
	 */
	if (virt_to_phys(skb->data) < 0x50000040) {
		TSO_DEBUG("%s %d: Invalid Address nr_frags = %d, paddr = %pK \n",
				__func__, __LINE__, skb_shinfo(skb)->nr_frags,
				virt_to_phys(skb->data));
		goto fail;

	}

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		frag = &skb_shinfo(skb)->frags[i];

		if (!frag)
			goto fail;

		if (virt_to_phys(skb_frag_address_safe(frag)) < 0x50000040)
			goto fail;
	}


	gso_size = skb_shinfo(skb)->gso_size;
	tmp_len = skb->len - ((skb_transport_header(skb) - skb_mac_header(skb))
			+ tcp_hdrlen(skb));
	while (tmp_len) {
		num_segs++;
		if (tmp_len > gso_size)
			tmp_len -= gso_size;
		else
			break;
	}

	return num_segs;

	/*
	 * Do not free this frame, just do socket level accounting
	 * so that this is not reused.
	 */
fail:
	if (skb->sk)
		atomic_sub(skb->truesize, &(skb->sk->sk_wmem_alloc));

	return 0;
}
#else
uint32_t __qdf_nbuf_get_tso_num_seg(struct sk_buff *skb)
{
	uint32_t i, gso_size, tmp_len, num_segs = 0;
	skb_frag_t *frag = NULL;

	for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
		frag = &skb_shinfo(skb)->frags[i];

		if (!frag)
			goto fail;
	}

	gso_size = skb_shinfo(skb)->gso_size;
	tmp_len = skb->len - ((skb_transport_header(skb) - skb_mac_header(skb))
			+ tcp_hdrlen(skb));
	while (tmp_len) {
		num_segs++;
		if (tmp_len > gso_size)
			tmp_len -= gso_size;
		else
			break;
	}

	return num_segs;

	/*
	 * Do not free this frame, just do socket level accounting
	 * so that this is not reused.
	 */
fail:
	if (skb->sk)
		atomic_sub(skb->truesize, &(skb->sk->sk_wmem_alloc));

	return 0;
}
#endif
qdf_export_symbol(__qdf_nbuf_get_tso_num_seg);

#endif /* FEATURE_TSO */

/**
 * qdf_dmaaddr_to_32s - return high and low parts of dma_addr
 *
 * Returns the high and low 32-bits of the DMA addr in the provided ptrs
 *
 * Return: N/A
 */
void __qdf_dmaaddr_to_32s(qdf_dma_addr_t dmaaddr,
			  uint32_t *lo, uint32_t *hi)
{
	if (sizeof(dmaaddr) > sizeof(uint32_t)) {
		*lo = lower_32_bits(dmaaddr);
		*hi = upper_32_bits(dmaaddr);
	} else {
		*lo = dmaaddr;
		*hi = 0;
	}
}

qdf_export_symbol(__qdf_dmaaddr_to_32s);

struct sk_buff *__qdf_nbuf_inc_users(struct sk_buff *skb)
{
	qdf_nbuf_users_inc(&skb->users);
	return skb;
}
qdf_export_symbol(__qdf_nbuf_inc_users);

int __qdf_nbuf_get_users(struct sk_buff *skb)
{
	return qdf_nbuf_users_read(&skb->users);
}
qdf_export_symbol(__qdf_nbuf_get_users);

/**
 * __qdf_nbuf_ref() - Reference the nbuf so it can get held until the last free.
 * @skb: sk_buff handle
 *
 * Return: none
 */

void __qdf_nbuf_ref(struct sk_buff *skb)
{
	skb_get(skb);
}
qdf_export_symbol(__qdf_nbuf_ref);

/**
 * __qdf_nbuf_shared() - Check whether the buffer is shared
 *  @skb: sk_buff buffer
 *
 *  Return: true if more than one person has a reference to this buffer.
 */
int __qdf_nbuf_shared(struct sk_buff *skb)
{
	return skb_shared(skb);
}
qdf_export_symbol(__qdf_nbuf_shared);

/**
 * __qdf_nbuf_dmamap_create() - create a DMA map.
 * @osdev: qdf device handle
 * @dmap: dma map handle
 *
 * This can later be used to map networking buffers. They :
 * - need space in adf_drv's software descriptor
 * - are typically created during adf_drv_create
 * - need to be created before any API(qdf_nbuf_map) that uses them
 *
 * Return: QDF STATUS
 */
QDF_STATUS
__qdf_nbuf_dmamap_create(qdf_device_t osdev, __qdf_dma_map_t *dmap)
{
	QDF_STATUS error = QDF_STATUS_SUCCESS;
	/*
	 * driver can tell its SG capablity, it must be handled.
	 * Bounce buffers if they are there
	 */
	(*dmap) = kzalloc(sizeof(struct __qdf_dma_map), GFP_KERNEL);
	if (!(*dmap))
		error = QDF_STATUS_E_NOMEM;

	return error;
}
qdf_export_symbol(__qdf_nbuf_dmamap_create);
/**
 * __qdf_nbuf_dmamap_destroy() - delete a dma map
 * @osdev: qdf device handle
 * @dmap: dma map handle
 *
 * Return: none
 */
void
__qdf_nbuf_dmamap_destroy(qdf_device_t osdev, __qdf_dma_map_t dmap)
{
	kfree(dmap);
}
qdf_export_symbol(__qdf_nbuf_dmamap_destroy);

/**
 * __qdf_nbuf_map_nbytes_single() - map nbytes
 * @osdev: os device
 * @buf: buffer
 * @dir: direction
 * @nbytes: number of bytes
 *
 * Return: QDF_STATUS
 */
#ifdef A_SIMOS_DEVHOST
QDF_STATUS __qdf_nbuf_map_nbytes_single(
		qdf_device_t osdev, struct sk_buff *buf,
		 qdf_dma_dir_t dir, int nbytes)
{
	qdf_dma_addr_t paddr;

	QDF_NBUF_CB_PADDR(buf) = paddr = buf->data;
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(__qdf_nbuf_map_nbytes_single);
#else
QDF_STATUS __qdf_nbuf_map_nbytes_single(
		qdf_device_t osdev, struct sk_buff *buf,
		 qdf_dma_dir_t dir, int nbytes)
{
	qdf_dma_addr_t paddr;

	/* assume that the OS only provides a single fragment */
	QDF_NBUF_CB_PADDR(buf) = paddr =
		dma_map_single(osdev->dev, buf->data,
			nbytes, __qdf_dma_dir_to_os(dir));
	return dma_mapping_error(osdev->dev, paddr) ?
		QDF_STATUS_E_FAULT : QDF_STATUS_SUCCESS;
}
qdf_export_symbol(__qdf_nbuf_map_nbytes_single);
#endif
/**
 * __qdf_nbuf_unmap_nbytes_single() - unmap nbytes
 * @osdev: os device
 * @buf: buffer
 * @dir: direction
 * @nbytes: number of bytes
 *
 * Return: none
 */
#if defined(A_SIMOS_DEVHOST)
void
__qdf_nbuf_unmap_nbytes_single(
	qdf_device_t osdev, struct sk_buff *buf, qdf_dma_dir_t dir, int nbytes)
{
}
qdf_export_symbol(__qdf_nbuf_unmap_nbytes_single);

#else
void
__qdf_nbuf_unmap_nbytes_single(
	qdf_device_t osdev, struct sk_buff *buf, qdf_dma_dir_t dir, int nbytes)
{
	if (0 ==  QDF_NBUF_CB_PADDR(buf)) {
		qdf_err("ERROR: NBUF mapped physical address is NULL");
		return;
	}
	dma_unmap_single(osdev->dev, QDF_NBUF_CB_PADDR(buf),
			nbytes, __qdf_dma_dir_to_os(dir));
}
qdf_export_symbol(__qdf_nbuf_unmap_nbytes_single);
#endif
/**
 * __qdf_nbuf_map_nbytes() - get the dma map of the nbuf
 * @osdev: os device
 * @skb: skb handle
 * @dir: dma direction
 * @nbytes: number of bytes to be mapped
 *
 * Return: QDF_STATUS
 */
#ifdef QDF_OS_DEBUG
QDF_STATUS
__qdf_nbuf_map_nbytes(
	qdf_device_t osdev,
	struct sk_buff *skb,
	qdf_dma_dir_t dir,
	int nbytes)
{
	struct skb_shared_info  *sh = skb_shinfo(skb);

	qdf_assert((dir == QDF_DMA_TO_DEVICE) || (dir == QDF_DMA_FROM_DEVICE));

	/*
	 * Assume there's only a single fragment.
	 * To support multiple fragments, it would be necessary to change
	 * adf_nbuf_t to be a separate object that stores meta-info
	 * (including the bus address for each fragment) and a pointer
	 * to the underlying sk_buff.
	 */
	qdf_assert(sh->nr_frags == 0);

	return __qdf_nbuf_map_nbytes_single(osdev, skb, dir, nbytes);
}
qdf_export_symbol(__qdf_nbuf_map_nbytes);
#else
QDF_STATUS
__qdf_nbuf_map_nbytes(
	qdf_device_t osdev,
	struct sk_buff *skb,
	qdf_dma_dir_t dir,
	int nbytes)
{
	return __qdf_nbuf_map_nbytes_single(osdev, skb, dir, nbytes);
}
qdf_export_symbol(__qdf_nbuf_map_nbytes);
#endif
/**
 * __qdf_nbuf_unmap_nbytes() - to unmap a previously mapped buf
 * @osdev: OS device
 * @skb: skb handle
 * @dir: direction
 * @nbytes: number of bytes
 *
 * Return: none
 */
void
__qdf_nbuf_unmap_nbytes(
	qdf_device_t osdev,
	struct sk_buff *skb,
	qdf_dma_dir_t dir,
	int nbytes)
{
	qdf_assert((dir == QDF_DMA_TO_DEVICE) || (dir == QDF_DMA_FROM_DEVICE));

	/*
	 * Assume there's a single fragment.
	 * If this is not true, the assertion in __adf_nbuf_map will catch it.
	 */
	__qdf_nbuf_unmap_nbytes_single(osdev, skb, dir, nbytes);
}
qdf_export_symbol(__qdf_nbuf_unmap_nbytes);

/**
 * __qdf_nbuf_dma_map_info() - return the dma map info
 * @bmap: dma map
 * @sg: dma map info
 *
 * Return: none
 */
void
__qdf_nbuf_dma_map_info(__qdf_dma_map_t bmap, qdf_dmamap_info_t *sg)
{
	qdf_assert(bmap->mapped);
	qdf_assert(bmap->nsegs <= QDF_MAX_SCATTER);

	memcpy(sg->dma_segs, bmap->seg, bmap->nsegs *
			sizeof(struct __qdf_segment));
	sg->nsegs = bmap->nsegs;
}
qdf_export_symbol(__qdf_nbuf_dma_map_info);
/**
 * __qdf_nbuf_frag_info() - return the frag data & len, where frag no. is
 *			specified by the index
 * @skb: sk buff
 * @sg: scatter/gather list of all the frags
 *
 * Return: none
 */
#if defined(__QDF_SUPPORT_FRAG_MEM)
void
__qdf_nbuf_frag_info(struct sk_buff *skb, qdf_sglist_t  *sg)
{
	qdf_assert(skb);
	sg->sg_segs[0].vaddr = skb->data;
	sg->sg_segs[0].len   = skb->len;
	sg->nsegs            = 1;

	for (int i = 1; i <= sh->nr_frags; i++) {
		skb_frag_t    *f        = &sh->frags[i - 1];

		sg->sg_segs[i].vaddr    = (uint8_t *)(page_address(f->page) +
			f->page_offset);
		sg->sg_segs[i].len      = f->size;

		qdf_assert(i < QDF_MAX_SGLIST);
	}
	sg->nsegs += i;

}
qdf_export_symbol(__qdf_nbuf_frag_info);
#else
#ifdef QDF_OS_DEBUG
void
__qdf_nbuf_frag_info(struct sk_buff *skb, qdf_sglist_t  *sg)
{

	struct skb_shared_info  *sh = skb_shinfo(skb);

	qdf_assert(skb);
	sg->sg_segs[0].vaddr = skb->data;
	sg->sg_segs[0].len   = skb->len;
	sg->nsegs            = 1;

	qdf_assert(sh->nr_frags == 0);
}
qdf_export_symbol(__qdf_nbuf_frag_info);
#else
void
__qdf_nbuf_frag_info(struct sk_buff *skb, qdf_sglist_t  *sg)
{
	sg->sg_segs[0].vaddr = skb->data;
	sg->sg_segs[0].len   = skb->len;
	sg->nsegs            = 1;
}
qdf_export_symbol(__qdf_nbuf_frag_info);
#endif
#endif
/**
 * __qdf_nbuf_get_frag_size() - get frag size
 * @nbuf: sk buffer
 * @cur_frag: current frag
 *
 * Return: frag size
 */
uint32_t
__qdf_nbuf_get_frag_size(__qdf_nbuf_t nbuf, uint32_t cur_frag)
{
	struct skb_shared_info  *sh = skb_shinfo(nbuf);
	const skb_frag_t *frag = sh->frags + cur_frag;

	return skb_frag_size(frag);
}
qdf_export_symbol(__qdf_nbuf_get_frag_size);

/**
 * __qdf_nbuf_frag_map() - dma map frag
 * @osdev: os device
 * @nbuf: sk buff
 * @offset: offset
 * @dir: direction
 * @cur_frag: current fragment
 *
 * Return: QDF status
 */
#ifdef A_SIMOS_DEVHOST
QDF_STATUS __qdf_nbuf_frag_map(
	qdf_device_t osdev, __qdf_nbuf_t nbuf,
	int offset, qdf_dma_dir_t dir, int cur_frag)
{
	int32_t paddr, frag_len;

	QDF_NBUF_CB_PADDR(nbuf) = paddr = nbuf->data;
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(__qdf_nbuf_frag_map);
#else
QDF_STATUS __qdf_nbuf_frag_map(
	qdf_device_t osdev, __qdf_nbuf_t nbuf,
	int offset, qdf_dma_dir_t dir, int cur_frag)
{
	dma_addr_t paddr, frag_len;
	struct skb_shared_info *sh = skb_shinfo(nbuf);
	const skb_frag_t *frag = sh->frags + cur_frag;

	frag_len = skb_frag_size(frag);

	QDF_NBUF_CB_TX_EXTRA_FRAG_PADDR(nbuf) = paddr =
		skb_frag_dma_map(osdev->dev, frag, offset, frag_len,
					__qdf_dma_dir_to_os(dir));
	return dma_mapping_error(osdev->dev, paddr) ?
			QDF_STATUS_E_FAULT : QDF_STATUS_SUCCESS;
}
qdf_export_symbol(__qdf_nbuf_frag_map);
#endif
/**
 * __qdf_nbuf_dmamap_set_cb() - setup the map callback for a dma map
 * @dmap: dma map
 * @cb: callback
 * @arg: argument
 *
 * Return: none
 */
void
__qdf_nbuf_dmamap_set_cb(__qdf_dma_map_t dmap, void *cb, void *arg)
{
	return;
}
qdf_export_symbol(__qdf_nbuf_dmamap_set_cb);


/**
 * __qdf_nbuf_sync_single_for_cpu() - nbuf sync
 * @osdev: os device
 * @buf: sk buff
 * @dir: direction
 *
 * Return: none
 */
#if defined(A_SIMOS_DEVHOST)
static void __qdf_nbuf_sync_single_for_cpu(
	qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	return;
}
#else
static void __qdf_nbuf_sync_single_for_cpu(
	qdf_device_t osdev, qdf_nbuf_t buf, qdf_dma_dir_t dir)
{
	if (0 ==  QDF_NBUF_CB_PADDR(buf)) {
		qdf_err("ERROR: NBUF mapped physical address is NULL");
		return;
	}
	dma_sync_single_for_cpu(osdev->dev, QDF_NBUF_CB_PADDR(buf),
		skb_end_offset(buf) - skb_headroom(buf),
		__qdf_dma_dir_to_os(dir));
}
#endif
/**
 * __qdf_nbuf_sync_for_cpu() - nbuf sync
 * @osdev: os device
 * @skb: sk buff
 * @dir: direction
 *
 * Return: none
 */
void
__qdf_nbuf_sync_for_cpu(qdf_device_t osdev,
	struct sk_buff *skb, qdf_dma_dir_t dir)
{
	qdf_assert(
	(dir == QDF_DMA_TO_DEVICE) || (dir == QDF_DMA_FROM_DEVICE));

	/*
	 * Assume there's a single fragment.
	 * If this is not true, the assertion in __adf_nbuf_map will catch it.
	 */
	__qdf_nbuf_sync_single_for_cpu(osdev, skb, dir);
}
qdf_export_symbol(__qdf_nbuf_sync_for_cpu);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 10, 0))
/**
 * qdf_nbuf_update_radiotap_vht_flags() - Update radiotap header VHT flags
 * @rx_status: Pointer to rx_status.
 * @rtap_buf: Buf to which VHT info has to be updated.
 * @rtap_len: Current length of radiotap buffer
 *
 * Return: Length of radiotap after VHT flags updated.
 */
static unsigned int qdf_nbuf_update_radiotap_vht_flags(
					struct mon_rx_status *rx_status,
					int8_t *rtap_buf,
					uint32_t rtap_len)
{
	uint16_t vht_flags = 0;

	rtap_len = qdf_align(rtap_len, 2);

	/* IEEE80211_RADIOTAP_VHT u16, u8, u8, u8[4], u8, u8, u16 */
	vht_flags |= IEEE80211_RADIOTAP_VHT_KNOWN_STBC |
		IEEE80211_RADIOTAP_VHT_KNOWN_GI |
		IEEE80211_RADIOTAP_VHT_KNOWN_LDPC_EXTRA_OFDM_SYM |
		IEEE80211_RADIOTAP_VHT_KNOWN_BEAMFORMED |
		IEEE80211_RADIOTAP_VHT_KNOWN_BANDWIDTH |
		IEEE80211_RADIOTAP_VHT_KNOWN_GROUP_ID;
	put_unaligned_le16(vht_flags, &rtap_buf[rtap_len]);
	rtap_len += 2;

	rtap_buf[rtap_len] |=
		(rx_status->is_stbc ?
		 IEEE80211_RADIOTAP_VHT_FLAG_STBC : 0) |
		(rx_status->sgi ? IEEE80211_RADIOTAP_VHT_FLAG_SGI : 0) |
		(rx_status->ldpc ?
		 IEEE80211_RADIOTAP_VHT_FLAG_LDPC_EXTRA_OFDM_SYM : 0) |
		(rx_status->beamformed ?
		 IEEE80211_RADIOTAP_VHT_FLAG_BEAMFORMED : 0);
	rtap_len += 1;
	switch (rx_status->vht_flag_values2) {
	case IEEE80211_RADIOTAP_VHT_BW_20:
		rtap_buf[rtap_len] = RADIOTAP_VHT_BW_20;
		break;
	case IEEE80211_RADIOTAP_VHT_BW_40:
		rtap_buf[rtap_len] = RADIOTAP_VHT_BW_40;
		break;
	case IEEE80211_RADIOTAP_VHT_BW_80:
		rtap_buf[rtap_len] = RADIOTAP_VHT_BW_80;
		break;
	case IEEE80211_RADIOTAP_VHT_BW_160:
		rtap_buf[rtap_len] = RADIOTAP_VHT_BW_160;
		break;
	}
	rtap_len += 1;
	rtap_buf[rtap_len] = (rx_status->vht_flag_values3[0]);
	rtap_len += 1;
	rtap_buf[rtap_len] = (rx_status->vht_flag_values3[1]);
	rtap_len += 1;
	rtap_buf[rtap_len] = (rx_status->vht_flag_values3[2]);
	rtap_len += 1;
	rtap_buf[rtap_len] = (rx_status->vht_flag_values3[3]);
	rtap_len += 1;
	rtap_buf[rtap_len] = (rx_status->vht_flag_values4);
	rtap_len += 1;
	rtap_buf[rtap_len] = (rx_status->vht_flag_values5);
	rtap_len += 1;
	put_unaligned_le16(rx_status->vht_flag_values6,
			   &rtap_buf[rtap_len]);
	rtap_len += 2;

	return rtap_len;
}

/**
 * qdf_nbuf_update_radiotap_he_flags() - Update radiotap header from rx_status
 * @rx_status: Pointer to rx_status.
 * @rtap_buf: buffer to which radiotap has to be updated
 * @rtap_len: radiotap length
 *
 * API update high-efficiency (11ax) fields in the radiotap header
 *
 * Return: length of rtap_len updated.
 */
static unsigned int
qdf_nbuf_update_radiotap_he_flags(struct mon_rx_status *rx_status,
				     int8_t *rtap_buf, uint32_t rtap_len)
{
	/*
	 * IEEE80211_RADIOTAP_HE u16, u16, u16, u16, u16, u16
	 * Enable all "known" HE radiotap flags for now
	 */
	rtap_len = qdf_align(rtap_len, 2);

	put_unaligned_le16(rx_status->he_data1, &rtap_buf[rtap_len]);
	rtap_len += 2;

	put_unaligned_le16(rx_status->he_data2, &rtap_buf[rtap_len]);
	rtap_len += 2;

	put_unaligned_le16(rx_status->he_data3, &rtap_buf[rtap_len]);
	rtap_len += 2;

	put_unaligned_le16(rx_status->he_data4, &rtap_buf[rtap_len]);
	rtap_len += 2;

	put_unaligned_le16(rx_status->he_data5, &rtap_buf[rtap_len]);
	rtap_len += 2;

	put_unaligned_le16(rx_status->he_data6, &rtap_buf[rtap_len]);
	rtap_len += 2;
	qdf_debug("he data %x %x %x %x %x %x",
		  rx_status->he_data1,
		  rx_status->he_data2, rx_status->he_data3,
		  rx_status->he_data4, rx_status->he_data5,
		  rx_status->he_data6);
	return rtap_len;
}


/**
 * qdf_nbuf_update_radiotap_he_mu_flags() - update he-mu radiotap flags
 * @rx_status: Pointer to rx_status.
 * @rtap_buf: buffer to which radiotap has to be updated
 * @rtap_len: radiotap length
 *
 * API update HE-MU fields in the radiotap header
 *
 * Return: length of rtap_len updated.
 */
static unsigned int
qdf_nbuf_update_radiotap_he_mu_flags(struct mon_rx_status *rx_status,
				     int8_t *rtap_buf, uint32_t rtap_len)
{
	rtap_len = qdf_align(rtap_len, 2);

	/*
	 * IEEE80211_RADIOTAP_HE_MU u16, u16, u8[4]
	 * Enable all "known" he-mu radiotap flags for now
	 */
	put_unaligned_le16(rx_status->he_flags1, &rtap_buf[rtap_len]);
	rtap_len += 2;

	put_unaligned_le16(rx_status->he_flags2, &rtap_buf[rtap_len]);
	rtap_len += 2;

	rtap_buf[rtap_len] = rx_status->he_RU[0];
	rtap_len += 1;

	rtap_buf[rtap_len] = rx_status->he_RU[1];
	rtap_len += 1;

	rtap_buf[rtap_len] = rx_status->he_RU[2];
	rtap_len += 1;

	rtap_buf[rtap_len] = rx_status->he_RU[3];
	rtap_len += 1;
	qdf_debug("he_flags %x %x he-RU %x %x %x %x",
		  rx_status->he_flags1,
		  rx_status->he_flags2, rx_status->he_RU[0],
		  rx_status->he_RU[1], rx_status->he_RU[2],
		  rx_status->he_RU[3]);

	return rtap_len;
}

/**
 * qdf_nbuf_update_radiotap_he_mu_other_flags() - update he_mu_other flags
 * @rx_status: Pointer to rx_status.
 * @rtap_buf: buffer to which radiotap has to be updated
 * @rtap_len: radiotap length
 *
 * API update he-mu-other fields in the radiotap header
 *
 * Return: length of rtap_len updated.
 */
static unsigned int
qdf_nbuf_update_radiotap_he_mu_other_flags(struct mon_rx_status *rx_status,
				     int8_t *rtap_buf, uint32_t rtap_len)
{
	rtap_len = qdf_align(rtap_len, 2);

	/*
	 * IEEE80211_RADIOTAP_HE-MU-OTHER u16, u16, u8, u8
	 * Enable all "known" he-mu-other radiotap flags for now
	 */
	put_unaligned_le16(rx_status->he_per_user_1, &rtap_buf[rtap_len]);
	rtap_len += 2;

	put_unaligned_le16(rx_status->he_per_user_2, &rtap_buf[rtap_len]);
	rtap_len += 2;

	rtap_buf[rtap_len] = rx_status->he_per_user_position;
	rtap_len += 1;

	rtap_buf[rtap_len] = rx_status->he_per_user_known;
	rtap_len += 1;
	qdf_debug("he_per_user %x %x pos %x knwn %x",
		  rx_status->he_per_user_1,
		  rx_status->he_per_user_2, rx_status->he_per_user_position,
		  rx_status->he_per_user_known);
	return rtap_len;
}


/**
 * This is the length for radiotap, combined length
 * (Mandatory part struct ieee80211_radiotap_header + RADIOTAP_HEADER_LEN)
 * cannot be more than available headroom_sz.
 * increase this when we add more radiotap elements.
 * Number after '+' indicates maximum possible increase due to alignment
 */

#define RADIOTAP_VHT_FLAGS_LEN (12 + 1)
#define RADIOTAP_HE_FLAGS_LEN (12 + 1)
#define RADIOTAP_HE_MU_FLAGS_LEN (8 + 1)
#define RADIOTAP_HE_MU_OTHER_FLAGS_LEN (18 + 1)
#define RADIOTAP_FIXED_HEADER_LEN 17
#define RADIOTAP_HT_FLAGS_LEN 3
#define RADIOTAP_AMPDU_STATUS_LEN (8 + 3)
#define RADIOTAP_VENDOR_NS_LEN \
	(sizeof(struct qdf_radiotap_vendor_ns_ath) + 1)
#define RADIOTAP_HEADER_LEN (sizeof(struct ieee80211_radiotap_header) + \
				RADIOTAP_FIXED_HEADER_LEN + \
				RADIOTAP_HT_FLAGS_LEN + \
				RADIOTAP_VHT_FLAGS_LEN + \
				RADIOTAP_AMPDU_STATUS_LEN + \
				RADIOTAP_HE_FLAGS_LEN + \
				RADIOTAP_HE_MU_FLAGS_LEN + \
				RADIOTAP_HE_MU_OTHER_FLAGS_LEN + \
				RADIOTAP_VENDOR_NS_LEN)

#define IEEE80211_RADIOTAP_HE 23
#define IEEE80211_RADIOTAP_HE_MU	24
#define IEEE80211_RADIOTAP_HE_MU_OTHER	25
uint8_t ATH_OUI[] = {0x00, 0x03, 0x7f}; /* Atheros OUI */

/**
 * radiotap_num_to_freq() - Get frequency from chan number
 * @chan_num - Input channel number
 *
 * Return - Channel frequency in Mhz
 */
static uint16_t radiotap_num_to_freq (uint16_t chan_num)
{
	if (chan_num == CHANNEL_NUM_14)
		return CHANNEL_FREQ_2484;
	if (chan_num < CHANNEL_NUM_14)
		return CHANNEL_FREQ_2407 +
			(chan_num * FREQ_MULTIPLIER_CONST_5MHZ);

	if (chan_num < CHANNEL_NUM_27)
		return CHANNEL_FREQ_2512 +
			((chan_num - CHANNEL_NUM_15) *
			 FREQ_MULTIPLIER_CONST_20MHZ);

	if (chan_num > CHANNEL_NUM_182 &&
			chan_num < CHANNEL_NUM_197)
		return ((chan_num * FREQ_MULTIPLIER_CONST_5MHZ) +
			CHANNEL_FREQ_4000);

	return CHANNEL_FREQ_5000 +
		(chan_num * FREQ_MULTIPLIER_CONST_5MHZ);
}

/**
 * qdf_nbuf_update_radiotap_ampdu_flags() - Update radiotap header ampdu flags
 * @rx_status: Pointer to rx_status.
 * @rtap_buf: Buf to which AMPDU info has to be updated.
 * @rtap_len: Current length of radiotap buffer
 *
 * Return: Length of radiotap after AMPDU flags updated.
 */
static unsigned int qdf_nbuf_update_radiotap_ampdu_flags(
					struct mon_rx_status *rx_status,
					uint8_t *rtap_buf,
					uint32_t rtap_len)
{
	/*
	 * IEEE80211_RADIOTAP_AMPDU_STATUS u32 u16 u8 u8
	 * First 32 bits of AMPDU represents the reference number
	 */

	uint32_t ampdu_reference_num = rx_status->ppdu_id;
	uint16_t ampdu_flags = 0;
	uint16_t ampdu_reserved_flags = 0;

	rtap_len = qdf_align(rtap_len, 4);

	put_unaligned_le32(ampdu_reference_num, &rtap_buf[rtap_len]);
	rtap_len += 4;
	put_unaligned_le16(ampdu_flags, &rtap_buf[rtap_len]);
	rtap_len += 2;
	put_unaligned_le16(ampdu_reserved_flags, &rtap_buf[rtap_len]);
	rtap_len += 2;

	return rtap_len;
}

/**
 * qdf_nbuf_update_radiotap() - Update radiotap header from rx_status
 * @rx_status: Pointer to rx_status.
 * @nbuf:      nbuf pointer to which radiotap has to be updated
 * @headroom_sz: Available headroom size.
 *
 * Return: length of rtap_len updated.
 */
unsigned int qdf_nbuf_update_radiotap(struct mon_rx_status *rx_status,
				      qdf_nbuf_t nbuf, uint32_t headroom_sz)
{
	uint8_t rtap_buf[RADIOTAP_HEADER_LEN] = {0};
	struct ieee80211_radiotap_header *rthdr =
		(struct ieee80211_radiotap_header *)rtap_buf;
	uint32_t rtap_hdr_len = sizeof(struct ieee80211_radiotap_header);
	uint32_t rtap_len = rtap_hdr_len;
	uint8_t length = rtap_len;
	struct qdf_radiotap_vendor_ns_ath *radiotap_vendor_ns_ath;

	/* IEEE80211_RADIOTAP_TSFT              __le64       microseconds*/
	rthdr->it_present = (1 << IEEE80211_RADIOTAP_TSFT);
	put_unaligned_le64(rx_status->tsft, &rtap_buf[rtap_len]);
	rtap_len += 8;

	/* IEEE80211_RADIOTAP_FLAGS u8 */
	rthdr->it_present |= (1 << IEEE80211_RADIOTAP_FLAGS);

	if (rx_status->rs_fcs_err)
		rx_status->rtap_flags |= IEEE80211_RADIOTAP_F_BADFCS;

	rtap_buf[rtap_len] = rx_status->rtap_flags;
	rtap_len += 1;

	/* IEEE80211_RADIOTAP_RATE  u8           500kb/s */
	if (!rx_status->ht_flags && !rx_status->vht_flags &&
	    !rx_status->he_flags) {
		rthdr->it_present |= (1 << IEEE80211_RADIOTAP_RATE);
		rtap_buf[rtap_len] = rx_status->rate;
	} else
		rtap_buf[rtap_len] = 0;
	rtap_len += 1;

	/* IEEE80211_RADIOTAP_CHANNEL 2 x __le16   MHz, bitmap */
	rthdr->it_present |= (1 << IEEE80211_RADIOTAP_CHANNEL);
	rx_status->chan_freq = radiotap_num_to_freq(rx_status->chan_num);
	put_unaligned_le16(rx_status->chan_freq, &rtap_buf[rtap_len]);
	rtap_len += 2;
	/* Channel flags. */
	if (rx_status->chan_num > CHANNEL_NUM_35)
		rx_status->chan_flags = RADIOTAP_5G_SPECTRUM_CHANNEL;
	else
		rx_status->chan_flags = RADIOTAP_2G_SPECTRUM_CHANNEL;
	if (rx_status->cck_flag)
		rx_status->chan_flags |= RADIOTAP_CCK_CHANNEL;
	if (rx_status->ofdm_flag)
		rx_status->chan_flags |= RADIOTAP_OFDM_CHANNEL;
	put_unaligned_le16(rx_status->chan_flags, &rtap_buf[rtap_len]);
	rtap_len += 2;

	/* IEEE80211_RADIOTAP_DBM_ANTSIGNAL s8  decibels from one milliwatt
	 *					(dBm)
	 */
	rthdr->it_present |= (1 << IEEE80211_RADIOTAP_DBM_ANTSIGNAL);
	/*
	 * rssi_comb is int dB, need to convert it to dBm.
	 * normalize value to noise floor of -96 dBm
	 */
	rtap_buf[rtap_len] = rx_status->rssi_comb + rx_status->chan_noise_floor;
	rtap_len += 1;

	/* RX signal noise floor */
	rthdr->it_present |= (1 << IEEE80211_RADIOTAP_DBM_ANTNOISE);
	rtap_buf[rtap_len] = (uint8_t)rx_status->chan_noise_floor;
	rtap_len += 1;

	/* IEEE80211_RADIOTAP_ANTENNA   u8      antenna index */
	rthdr->it_present |= (1 << IEEE80211_RADIOTAP_ANTENNA);
	rtap_buf[rtap_len] = rx_status->nr_ant;
	rtap_len += 1;

	if ((rtap_len - length) > RADIOTAP_FIXED_HEADER_LEN) {
		qdf_print("length is greater than RADIOTAP_FIXED_HEADER_LEN");
		return 0;
	}

	if (rx_status->ht_flags) {
		length = rtap_len;
		/* IEEE80211_RADIOTAP_VHT u8, u8, u8 */
		rthdr->it_present |= (1 << IEEE80211_RADIOTAP_MCS);
		rtap_buf[rtap_len] = IEEE80211_RADIOTAP_MCS_HAVE_BW |
					IEEE80211_RADIOTAP_MCS_HAVE_MCS |
					IEEE80211_RADIOTAP_MCS_HAVE_GI;
		rtap_len += 1;

		if (rx_status->sgi)
			rtap_buf[rtap_len] |= IEEE80211_RADIOTAP_MCS_SGI;
		if (rx_status->bw)
			rtap_buf[rtap_len] |= IEEE80211_RADIOTAP_MCS_BW_40;
		else
			rtap_buf[rtap_len] |= IEEE80211_RADIOTAP_MCS_BW_20;
		rtap_len += 1;

		rtap_buf[rtap_len] = rx_status->ht_mcs;
		rtap_len += 1;

		if ((rtap_len - length) > RADIOTAP_HT_FLAGS_LEN) {
			qdf_print("length is greater than RADIOTAP_HT_FLAGS_LEN");
			return 0;
		}
	}

	if (rx_status->rs_flags & IEEE80211_AMPDU_FLAG) {
		/* IEEE80211_RADIOTAP_AMPDU_STATUS u32 u16 u8 u8 */
		rthdr->it_present |= (1 << IEEE80211_RADIOTAP_AMPDU_STATUS);
		rtap_len = qdf_nbuf_update_radiotap_ampdu_flags(rx_status,
								rtap_buf,
								rtap_len);
	}

	if (rx_status->vht_flags) {
		length = rtap_len;
		/* IEEE80211_RADIOTAP_VHT u16, u8, u8, u8[4], u8, u8, u16 */
		rthdr->it_present |= (1 << IEEE80211_RADIOTAP_VHT);
		rtap_len = qdf_nbuf_update_radiotap_vht_flags(rx_status,
								rtap_buf,
								rtap_len);

		if ((rtap_len - length) > RADIOTAP_VHT_FLAGS_LEN) {
			qdf_print("length is greater than RADIOTAP_VHT_FLAGS_LEN");
			return 0;
		}
	}

	if (rx_status->he_flags) {
		length = rtap_len;
		/* IEEE80211_RADIOTAP_HE */
		rthdr->it_present |= (1 << IEEE80211_RADIOTAP_HE);
		rtap_len = qdf_nbuf_update_radiotap_he_flags(rx_status,
								rtap_buf,
								rtap_len);

		if ((rtap_len - length) > RADIOTAP_HE_FLAGS_LEN) {
			qdf_print("length is greater than RADIOTAP_HE_FLAGS_LEN");
			return 0;
		}
	}

	if (rx_status->he_mu_flags) {
		length = rtap_len;
		/* IEEE80211_RADIOTAP_HE-MU */
		rthdr->it_present |= (1 << IEEE80211_RADIOTAP_HE_MU);
		rtap_len = qdf_nbuf_update_radiotap_he_mu_flags(rx_status,
								rtap_buf,
								rtap_len);

		if ((rtap_len - length) > RADIOTAP_HE_MU_FLAGS_LEN) {
			qdf_print("length is greater than RADIOTAP_HE_MU_FLAGS_LEN");
			return 0;
		}
	}

	if (rx_status->he_mu_other_flags) {
		length = rtap_len;
		/* IEEE80211_RADIOTAP_HE-MU-OTHER */
		rthdr->it_present |= (1 << IEEE80211_RADIOTAP_HE_MU_OTHER);
		rtap_len =
			qdf_nbuf_update_radiotap_he_mu_other_flags(rx_status,
								rtap_buf,
								rtap_len);

		if ((rtap_len - length) > RADIOTAP_HE_MU_OTHER_FLAGS_LEN) {
			qdf_print("length is greater than RADIOTAP_HE_MU_OTHER_FLAGS_LEN");
			return 0;
		}
	}

	rtap_len = qdf_align(rtap_len, 2);
	/*
	 * Radiotap Vendor Namespace
	 */
	rthdr->it_present |= (1 << IEEE80211_RADIOTAP_VENDOR_NAMESPACE);
	radiotap_vendor_ns_ath = (struct qdf_radiotap_vendor_ns_ath *)
					(rtap_buf + rtap_len);
	/*
	 * Copy Atheros OUI - 3 bytes (4th byte is 0)
	 */
	qdf_mem_copy(radiotap_vendor_ns_ath->hdr.oui, ATH_OUI, sizeof(ATH_OUI));
	/*
	 * Name space selector = 0
	 * We only will have one namespace for now
	 */
	radiotap_vendor_ns_ath->hdr.selector = 0;
	radiotap_vendor_ns_ath->hdr.skip_length = cpu_to_le16(
					sizeof(*radiotap_vendor_ns_ath) -
					sizeof(radiotap_vendor_ns_ath->hdr));
	radiotap_vendor_ns_ath->device_id = cpu_to_le32(rx_status->device_id);
	radiotap_vendor_ns_ath->lsig = cpu_to_le32(rx_status->l_sig_a_info);
	radiotap_vendor_ns_ath->lsig_b = cpu_to_le32(rx_status->l_sig_b_info);
	radiotap_vendor_ns_ath->ppdu_start_timestamp =
				cpu_to_le32(rx_status->ppdu_timestamp);
	rtap_len += sizeof(*radiotap_vendor_ns_ath);

	rthdr->it_len = cpu_to_le16(rtap_len);
	rthdr->it_present = cpu_to_le32(rthdr->it_present);

	if (headroom_sz < rtap_len) {
		qdf_err("ERROR: not enough space to update radiotap");
		return 0;
	}
	qdf_nbuf_push_head(nbuf, rtap_len);
	qdf_mem_copy(qdf_nbuf_data(nbuf), rtap_buf, rtap_len);
	return rtap_len;
}
#else
static unsigned int qdf_nbuf_update_radiotap_vht_flags(
					struct mon_rx_status *rx_status,
					int8_t *rtap_buf,
					uint32_t rtap_len)
{
	qdf_err("ERROR: struct ieee80211_radiotap_header not supported");
	return 0;
}

unsigned int qdf_nbuf_update_radiotap_he_flags(struct mon_rx_status *rx_status,
				      int8_t *rtap_buf, uint32_t rtap_len)
{
	qdf_err("ERROR: struct ieee80211_radiotap_header not supported");
	return 0;
}

static unsigned int qdf_nbuf_update_radiotap_ampdu_flags(
					struct mon_rx_status *rx_status,
					uint8_t *rtap_buf,
					uint32_t rtap_len)
{
	qdf_err("ERROR: struct ieee80211_radiotap_header not supported");
	return 0;
}

unsigned int qdf_nbuf_update_radiotap(struct mon_rx_status *rx_status,
				      qdf_nbuf_t nbuf, uint32_t headroom_sz)
{
	qdf_err("ERROR: struct ieee80211_radiotap_header not supported");
	return 0;
}
#endif
qdf_export_symbol(qdf_nbuf_update_radiotap);

/**
 * __qdf_nbuf_reg_free_cb() - register nbuf free callback
 * @cb_func_ptr: function pointer to the nbuf free callback
 *
 * This function registers a callback function for nbuf free.
 *
 * Return: none
 */
void __qdf_nbuf_reg_free_cb(qdf_nbuf_free_t cb_func_ptr)
{
	nbuf_free_cb = cb_func_ptr;
}

/**
 * qdf_nbuf_classify_pkt() - classify packet
 * @skb - sk buff
 *
 * Return: none
 */
void qdf_nbuf_classify_pkt(struct sk_buff *skb)
{
	struct ethhdr *eh = (struct ethhdr *)skb->data;

	/* check destination mac address is broadcast/multicast */
	if (is_broadcast_ether_addr((uint8_t *)eh))
		QDF_NBUF_CB_SET_BCAST(skb);
	else if (is_multicast_ether_addr((uint8_t *)eh))
		QDF_NBUF_CB_SET_MCAST(skb);

	if (qdf_nbuf_is_ipv4_arp_pkt(skb))
		QDF_NBUF_CB_GET_PACKET_TYPE(skb) =
			QDF_NBUF_CB_PACKET_TYPE_ARP;
	else if (qdf_nbuf_is_ipv4_dhcp_pkt(skb))
		QDF_NBUF_CB_GET_PACKET_TYPE(skb) =
			QDF_NBUF_CB_PACKET_TYPE_DHCP;
	else if (qdf_nbuf_is_ipv4_eapol_pkt(skb))
		QDF_NBUF_CB_GET_PACKET_TYPE(skb) =
			QDF_NBUF_CB_PACKET_TYPE_EAPOL;
	else if (qdf_nbuf_is_ipv4_wapi_pkt(skb))
		QDF_NBUF_CB_GET_PACKET_TYPE(skb) =
			QDF_NBUF_CB_PACKET_TYPE_WAPI;
}
qdf_export_symbol(qdf_nbuf_classify_pkt);

void __qdf_nbuf_init(__qdf_nbuf_t nbuf)
{
	qdf_nbuf_users_set(&nbuf->users, 1);
	nbuf->data = nbuf->head + NET_SKB_PAD;
	skb_reset_tail_pointer(nbuf);
}
qdf_export_symbol(__qdf_nbuf_init);

#ifdef WLAN_FEATURE_FASTPATH
void qdf_nbuf_init_fast(qdf_nbuf_t nbuf)
{
	qdf_nbuf_users_set(&nbuf->users, 1);
	nbuf->data = nbuf->head + NET_SKB_PAD;
	skb_reset_tail_pointer(nbuf);
}
qdf_export_symbol(qdf_nbuf_init_fast);
#endif /* WLAN_FEATURE_FASTPATH */


#ifdef QDF_NBUF_GLOBAL_COUNT
/**
 * __qdf_nbuf_mod_init() - Intialization routine for qdf_nuf
 *
 * Return void
 */
void __qdf_nbuf_mod_init(void)
{
	qdf_atomic_init(&nbuf_count);
	qdf_debugfs_create_atomic(NBUF_DEBUGFS_NAME, S_IRUSR, NULL, &nbuf_count);
}

/**
 * __qdf_nbuf_mod_exit() - Unintialization routine for qdf_nuf
 *
 * Return void
 */
void __qdf_nbuf_mod_exit(void)
{
}
#endif
