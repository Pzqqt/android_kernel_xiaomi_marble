/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 *  DOC: wlan_mgmt_txrx_rx_reo_i.h
 *  This file contains mgmt rx re-ordering related APIs
 */

#ifndef _WLAN_MGMT_TXRX_RX_REO_I_H
#define _WLAN_MGMT_TXRX_RX_REO_I_H

#ifdef WLAN_MGMT_RX_REO_SUPPORT
#include <qdf_list.h>
#include <qdf_timer.h>
#include <qdf_lock.h>
#include <qdf_nbuf.h>
#include <wlan_mgmt_txrx_rx_reo_utils_api.h>
#include <wlan_mgmt_txrx_rx_reo_public_structs.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_psoc_obj.h>

#define MGMT_RX_REO_LIST_MAX_SIZE        (100)
#define MGMT_RX_REO_LIST_TIMEOUT_US      (10 * USEC_PER_MSEC)
#define MGMT_RX_REO_STATUS_WAIT_FOR_FRAME_ON_OTHER_LINKS  (BIT(0))
#define MGMT_RX_REO_STATUS_AGED_OUT                       (BIT(1))

/**
 * TODO: Dummy macro for Maximum MLO links on the system
 * This is added only as a place holder for the time being.
 * Remove this once the actual one is implemented.
 */
#define MGMT_RX_REO_MAX_LINKS (16)
#define MGMT_RX_REO_INVALID_NUM_LINKS (-1)
/* Reason to release an entry from the reorder list */
#define MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_ZERO_WAIT_COUNT           (BIT(0))
#define MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_AGED_OUT                  (BIT(1))
#define MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_OLDER_THAN_AGED_OUT_FRAME (BIT(2))
#define MGMT_RX_REO_LIST_ENTRY_RELEASE_REASON_LIST_MAX_SIZE_EXCEEDED    (BIT(3))

#define MGMT_RX_REO_LIST_ENTRY_IS_WAITING_FOR_FRAME_ON_OTHER_LINK(entry)   \
	((entry)->status & MGMT_RX_REO_STATUS_WAIT_FOR_FRAME_ON_OTHER_LINKS)
#define MGMT_RX_REO_LIST_ENTRY_IS_AGED_OUT(entry)   \
	((entry)->status & MGMT_RX_REO_STATUS_AGED_OUT)
#define MGMT_RX_REO_LIST_ENTRY_IS_OLDER_THAN_LATEST_AGED_OUT_FRAME(ts, entry)  \
	(mgmt_rx_reo_compare_global_timestamps_gte(                            \
	 (ts)->global_ts, mgmt_rx_reo_get_global_ts((entry)->rx_params)))

/*
 * struct mgmt_rx_reo_pdev_info - Pdev information required by the Management
 * Rx REO module
 * @host_snapshot: Latest snapshot seen at the Host.
 * It considers both MGMT Rx and MGMT FW consumed.
 * @last_valid_shared_snapshot: Array of last valid snapshots(for snapshots
 * shared between host and target)
 * @host_target_shared_snapshot: Array of snapshot addresses(for snapshots
 * shared between host and target)
 * @filter: MGMT Rx REO filter
 */
struct mgmt_rx_reo_pdev_info {
	struct mgmt_rx_reo_snapshot_params host_snapshot;
	struct mgmt_rx_reo_snapshot_params last_valid_shared_snapshot
				[MGMT_RX_REO_SHARED_SNAPSHOT_MAX];
	struct mgmt_rx_reo_snapshot *host_target_shared_snapshot
				[MGMT_RX_REO_SHARED_SNAPSHOT_MAX];
	struct mgmt_rx_reo_filter filter;
};

/**
 * mgmt_rx_reo_pdev_obj_create_notification() - pdev create handler for
 * management rx-reorder module
 * @pdev: pointer to pdev object
 * @mgmt_txrx_pdev_ctx: pdev private object of mgmt txrx module
 *
 * This function gets called from object manager when pdev is being created and
 * creates management rx-reorder pdev context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_pdev_obj_create_notification(
	struct wlan_objmgr_pdev *pdev,
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx);

/**
 * mgmt_rx_reo_pdev_obj_destroy_notification() - pdev destroy handler for
 * management rx-reorder feature
 * @pdev: pointer to pdev object
 * @mgmt_txrx_pdev_ctx: pdev private object of mgmt txrx module
 *
 * This function gets called from object manager when pdev is being destroyed
 * and destroys management rx-reorder pdev context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_pdev_obj_destroy_notification(
	struct wlan_objmgr_pdev *pdev,
	struct mgmt_txrx_priv_pdev_context *mgmt_txrx_pdev_ctx);

/**
 * enum mgmt_rx_reo_frame_descriptor_type - Enumeration for management frame
 * descriptor type.
 * @MGMT_RX_REO_FRAME_DESC_HOST_CONSUMED_FRAME: Management frame to be consumed
 * by host.
 * @MGMT_RX_REO_FRAME_DESC_FW_CONSUMED_FRAME: Management frame consumed by FW
 * @MGMT_RX_REO_FRAME_DESC_ERROR_FRAME: Management frame which got dropped
 * at host due to any error
 */
enum mgmt_rx_reo_frame_descriptor_type {
	MGMT_RX_REO_FRAME_DESC_HOST_CONSUMED_FRAME,
	MGMT_RX_REO_FRAME_DESC_FW_CONSUMED_FRAME,
	MGMT_RX_REO_FRAME_DESC_ERROR_FRAME,
};

/**
 * struct mgmt_rx_reo_global_ts_info - This structure holds the global time
 * stamp information of a frame.
 * @global_ts: Global time stamp value
 * @valid: Indicates whether global time stamp is valid
 */
struct mgmt_rx_reo_global_ts_info {
	bool valid;
	uint32_t global_ts;
};

/**
 * struct mgmt_rx_reo_list – Linked list used to reorder the management frames
 * received. Each list entry would correspond to a management frame. List
 * entries would be sorted in the same order in which they are received by MAC
 * HW.
 * @list: List used for reordering
 * @list_lock: Lock to protect the list
 * @max_list_size: Maximum size of the reorder list
 * @list_entry_timeout_us: Time out value(microsecond) for the reorder list
 * entries
 * @ageout_timer: Periodic timer to age-out the list entries
 * @ts_latest_aged_out_frame: Stores the global time stamp for the latest aged
 * out frame. Latest aged out frame is the aged out frame in reorder list which
 * has the largest global time stamp value.
 * @ts_last_delivered_frame: Stores the global time stamp for the last frame
 * delivered to the upper layer
 */
struct mgmt_rx_reo_list {
	qdf_list_t list;
	qdf_spinlock_t list_lock;
	uint32_t max_list_size;
	uint32_t list_entry_timeout_us;
	qdf_timer_t ageout_timer;
	struct mgmt_rx_reo_global_ts_info ts_latest_aged_out_frame;
	struct mgmt_rx_reo_global_ts_info ts_last_delivered_frame;
};

/*
 * struct mgmt_rx_reo_wait_count - Wait count for a mgmt frame
 * @per_link_count: Array of wait counts for all MLO links. Each array entry
 * holds the number of frames this mgmt frame should wait for on that
 * particular link.
 * @total_count: Sum of entries in @per_link_count
 */
struct mgmt_rx_reo_wait_count {
	unsigned int per_link_count[MGMT_RX_REO_MAX_LINKS];
	unsigned long long int total_count;
};

/**
 * struct mgmt_rx_reo_list_entry - Entry in the Management reorder list
 * @node: List node
 * @nbuf: nbuf corresponding to this frame
 * @rx_params: Management rx event parameters
 * @wait_count: Wait counts for the frame
 * @insertion_ts: Host time stamp when this entry is inserted to
 * the list.
 * @status: Status for this entry
 */
struct mgmt_rx_reo_list_entry {
	qdf_list_node_t node;
	qdf_nbuf_t nbuf;
	struct mgmt_rx_event_params *rx_params;
	struct mgmt_rx_reo_wait_count wait_count;
	uint64_t insertion_ts;
	uint32_t status;
};

/**
 * struct mgmt_rx_reo_context - This structure holds the info required for
 * management rx-reordering. Reordering is done across all the psocs.
 * So there should be only one instance of this structure defined.
 * @reo_list: Linked list used for reordering
 * @num_mlo_links: Number of MLO links on the system
 * @reo_algo_entry_lock: Spin lock to protect reo algorithm entry critical
 * section execution
 */
struct mgmt_rx_reo_context {
	struct mgmt_rx_reo_list reo_list;
	uint8_t num_mlo_links;
	qdf_spinlock_t reo_algo_entry_lock;
};

/**
 * struct mgmt_rx_reo_frame_descriptor - Frame Descriptor used to describe
 * a management frame in mgmt rx reo module.
 * @type: Frame descriptor type
 * @nbuf: nbuf corresponding to this frame
 * @rx_params: Management rx event parameters
 * @wait_count: Wait counts for the frame
 */
struct mgmt_rx_reo_frame_descriptor {
	enum mgmt_rx_reo_frame_descriptor_type type;
	qdf_nbuf_t nbuf;
	struct mgmt_rx_event_params *rx_params;
	struct mgmt_rx_reo_wait_count wait_count;
};

/**
 * mgmt_rx_reo_get_global_ts() - Helper API to get global time stamp
 * corresponding to the mgmt rx event
 * @rx_params: Management rx event params
 *
 * Return: global time stamp corresponding to the mgmt rx event
 */
static inline uint32_t
mgmt_rx_reo_get_global_ts(struct mgmt_rx_event_params *rx_params)
{
	qdf_assert_always(rx_params);
	qdf_assert_always(rx_params->reo_params);

	return rx_params->reo_params->global_timestamp;
}

/**
 * mgmt_rx_reo_get_pkt_counter() - Helper API to get packet counter
 * corresponding to the mgmt rx event
 * @rx_params: Management rx event params
 *
 * Return: Management packet counter corresponding to the mgmt rx event
 */
static inline uint16_t
mgmt_rx_reo_get_pkt_counter(struct mgmt_rx_event_params *rx_params)
{
	qdf_assert_always(rx_params);
	qdf_assert_always(rx_params->reo_params);

	return rx_params->reo_params->mgmt_pkt_ctr;
}

/**
 * mgmt_rx_reo_get_link_id() - Helper API to get link id corresponding to the
 * mgmt rx event
 * @rx_params: Management rx event params
 *
 * Return: link id corresponding to the mgmt rx event
 */
static inline uint8_t
mgmt_rx_reo_get_link_id(struct mgmt_rx_event_params *rx_params)
{
	qdf_assert_always(rx_params);
	qdf_assert_always(rx_params->reo_params);

	return rx_params->reo_params->link_id;
}

/**
 * mgmt_rx_reo_get_pdev_id() - Helper API to get pdev id corresponding to the
 * mgmt rx event
 * @rx_params: Management rx event params
 *
 * Return: pdev id corresponding to the mgmt rx event
 */
static inline uint8_t
mgmt_rx_reo_get_pdev_id(struct mgmt_rx_event_params *rx_params)
{
	qdf_assert_always(rx_params);
	qdf_assert_always(rx_params->reo_params);

	return rx_params->reo_params->pdev_id;
}

/**
 * mgmt_rx_reo_init_context() - Initialize the management rx-reorder context
 *
 * API to initialize the global management rx-reorder context object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_init_context(void);

/**
 * mgmt_rx_reo_deinit_context() - De initialize the management rx-reorder
 * context
 *
 * API to de initialize the global management rx-reorder context object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
mgmt_rx_reo_deinit_context(void);

/**
 * is_mgmt_rx_reo_required() - Whether MGMT REO required for this frame/event
 * @pdev: pdev for which this frame/event is intended
 * @desc: Descriptor corresponding to this frame/event
 *
 * Return: true if REO is required; else false
 */
static inline bool is_mgmt_rx_reo_required(
			struct wlan_objmgr_pdev *pdev,
			struct mgmt_rx_reo_frame_descriptor *desc)
{
	/**
	 * NOTE: Implementing a simple policy based on INI and WMI serive bit
	 * for now. Finer policies like checking whether this pdev has
	 * any MLO VAPs or checking the frame type can be implemented later.
	 */
	return wlan_mgmt_rx_reo_is_feature_enabled_at_pdev(pdev);
}

/**
 * wlan_mgmt_rx_reo_algo_entry() - Entry point to the MGMT Rx REO algorithm for
 * a given MGMT frame/event.
 * @pdev: pdev for which this frame/event is intended
 * @desc: Descriptor corresponding to this frame/event
 * @is_queued: Whether this frame/event is queued in the REO list
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
wlan_mgmt_rx_reo_algo_entry(struct wlan_objmgr_pdev *pdev,
			    struct mgmt_rx_reo_frame_descriptor *desc,
			    bool *is_queued);

/**
 * mgmt_rx_reo_list_max_size_exceeded() - Helper API to check whether
 * list has exceeded the maximum configured size
 * @reo_list: Pointer to reorder list
 *
 * This API expects the caller to acquire the spin lock protecting the reorder
 * list.
 *
 * Return: true if reorder list has exceeded the max size
 */
static inline bool
mgmt_rx_reo_list_max_size_exceeded(struct mgmt_rx_reo_list *reo_list)
{
	return (qdf_list_size(&reo_list->list) > reo_list->max_list_size);
}
#endif /* WLAN_MGMT_RX_REO_SUPPORT */
#endif /* _WLAN_MGMT_TXRX_RX_REO_I_H */
