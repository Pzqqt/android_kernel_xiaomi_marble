/*
 * Copyright (c) 2015-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "targcfg.h"
#include "qdf_lock.h"
#include "qdf_status.h"
#include "qdf_status.h"
#include <qdf_atomic.h>         /* qdf_atomic_read */
#include <targaddrs.h>
#include "hif_io32.h"
#include <hif.h>
#include <target_type.h>
#include "regtable.h"
#define ATH_MODULE_NAME hif
#include <a_debug.h>
#include "hif_main.h"
#include "hif_hw_version.h"
#if (defined(HIF_PCI) || defined(HIF_SNOC) || defined(HIF_AHB) || \
     defined(HIF_IPCI))
#include "ce_tasklet.h"
#include "ce_api.h"
#endif
#include "qdf_trace.h"
#include "qdf_status.h"
#include "hif_debug.h"
#include "mp_dev.h"
#if defined(QCA_WIFI_QCA8074) || defined(QCA_WIFI_QCA6018) || \
	defined(QCA_WIFI_QCA5018) || defined(QCA_WIFI_QCA9574)
#include "hal_api.h"
#endif
#include "hif_napi.h"
#include "hif_unit_test_suspend_i.h"
#include "qdf_module.h"
#ifdef HIF_CE_LOG_INFO
#include <qdf_notifier.h>
#include <qdf_hang_event_notifier.h>
#endif
#include <linux/cpumask.h>

#if defined(HIF_IPCI) && defined(FEATURE_HAL_DELAYED_REG_WRITE)
#include <pld_common.h>
#endif

void hif_dump(struct hif_opaque_softc *hif_ctx, uint8_t cmd_id, bool start)
{
	hif_trigger_dump(hif_ctx, cmd_id, start);
}

/**
 * hif_get_target_id(): hif_get_target_id
 *
 * Return the virtual memory base address to the caller
 *
 * @scn: hif_softc
 *
 * Return: A_target_id_t
 */
A_target_id_t hif_get_target_id(struct hif_softc *scn)
{
	return scn->mem;
}

/**
 * hif_get_targetdef(): hif_get_targetdef
 * @scn: scn
 *
 * Return: void *
 */
void *hif_get_targetdef(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	return scn->targetdef;
}

#ifdef FORCE_WAKE
void hif_srng_init_phase(struct hif_opaque_softc *hif_ctx,
			 bool init_phase)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (ce_srng_based(scn))
		hal_set_init_phase(scn->hal_soc, init_phase);
}
#endif /* FORCE_WAKE */

#ifdef HIF_IPCI
void hif_shutdown_notifier_cb(void *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	scn->recovery = true;
}
#endif

/**
 * hif_vote_link_down(): unvote for link up
 *
 * Call hif_vote_link_down to release a previous request made using
 * hif_vote_link_up. A hif_vote_link_down call should only be made
 * after a corresponding hif_vote_link_up, otherwise you could be
 * negating a vote from another source. When no votes are present
 * hif will not guarantee the linkstate after hif_bus_suspend.
 *
 * SYNCHRONIZE WITH hif_vote_link_up by only calling in MC thread
 * and initialization deinitialization sequencences.
 *
 * Return: n/a
 */
void hif_vote_link_down(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	QDF_BUG(scn);
	if (scn->linkstate_vote == 0)
		QDF_DEBUG_PANIC("linkstate_vote(%d) has already been 0",
				scn->linkstate_vote);

	scn->linkstate_vote--;
	hif_info("Down_linkstate_vote %d", scn->linkstate_vote);
	if (scn->linkstate_vote == 0)
		hif_bus_prevent_linkdown(scn, false);
}

/**
 * hif_vote_link_up(): vote to prevent bus from suspending
 *
 * Makes hif guarantee that fw can message the host normally
 * durring suspend.
 *
 * SYNCHRONIZE WITH hif_vote_link_up by only calling in MC thread
 * and initialization deinitialization sequencences.
 *
 * Return: n/a
 */
void hif_vote_link_up(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	QDF_BUG(scn);
	scn->linkstate_vote++;
	hif_info("Up_linkstate_vote %d", scn->linkstate_vote);
	if (scn->linkstate_vote == 1)
		hif_bus_prevent_linkdown(scn, true);
}

/**
 * hif_can_suspend_link(): query if hif is permitted to suspend the link
 *
 * Hif will ensure that the link won't be suspended if the upperlayers
 * don't want it to.
 *
 * SYNCHRONIZATION: MC thread is stopped before bus suspend thus
 * we don't need extra locking to ensure votes dont change while
 * we are in the process of suspending or resuming.
 *
 * Return: false if hif will guarantee link up durring suspend.
 */
bool hif_can_suspend_link(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	QDF_BUG(scn);
	return scn->linkstate_vote == 0;
}

/**
 * hif_hia_item_address(): hif_hia_item_address
 * @target_type: target_type
 * @item_offset: item_offset
 *
 * Return: n/a
 */
uint32_t hif_hia_item_address(uint32_t target_type, uint32_t item_offset)
{
	switch (target_type) {
	case TARGET_TYPE_AR6002:
		return AR6002_HOST_INTEREST_ADDRESS + item_offset;
	case TARGET_TYPE_AR6003:
		return AR6003_HOST_INTEREST_ADDRESS + item_offset;
	case TARGET_TYPE_AR6004:
		return AR6004_HOST_INTEREST_ADDRESS + item_offset;
	case TARGET_TYPE_AR6006:
		return AR6006_HOST_INTEREST_ADDRESS + item_offset;
	case TARGET_TYPE_AR9888:
		return AR9888_HOST_INTEREST_ADDRESS + item_offset;
	case TARGET_TYPE_AR6320:
	case TARGET_TYPE_AR6320V2:
		return AR6320_HOST_INTEREST_ADDRESS + item_offset;
	case TARGET_TYPE_ADRASTEA:
		/* ADRASTEA doesn't have a host interest address */
		ASSERT(0);
		return 0;
	case TARGET_TYPE_AR900B:
		return AR900B_HOST_INTEREST_ADDRESS + item_offset;
	case TARGET_TYPE_QCA9984:
		return QCA9984_HOST_INTEREST_ADDRESS + item_offset;
	case TARGET_TYPE_QCA9888:
		return QCA9888_HOST_INTEREST_ADDRESS + item_offset;
	case TARGET_TYPE_IPQ4019:
		return IPQ4019_HOST_INTEREST_ADDRESS + item_offset;

	default:
		ASSERT(0);
		return 0;
	}
}

/**
 * hif_max_num_receives_reached() - check max receive is reached
 * @scn: HIF Context
 * @count: unsigned int.
 *
 * Output check status as bool
 *
 * Return: bool
 */
bool hif_max_num_receives_reached(struct hif_softc *scn, unsigned int count)
{
	if (QDF_IS_EPPING_ENABLED(hif_get_conparam(scn)))
		return count > 120;
	else
		return count > MAX_NUM_OF_RECEIVES;
}

/**
 * init_buffer_count() - initial buffer count
 * @maxSize: qdf_size_t
 *
 * routine to modify the initial buffer count to be allocated on an os
 * platform basis. Platform owner will need to modify this as needed
 *
 * Return: qdf_size_t
 */
qdf_size_t init_buffer_count(qdf_size_t maxSize)
{
	return maxSize;
}

/**
 * hif_save_htc_htt_config_endpoint() - save htt_tx_endpoint
 * @hif_ctx: hif context
 * @htc_htt_tx_endpoint: htt_tx_endpoint
 *
 * Return: void
 */
void hif_save_htc_htt_config_endpoint(struct hif_opaque_softc *hif_ctx,
							int htc_htt_tx_endpoint)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (!scn) {
		hif_err("scn or scn->hif_sc is NULL!");
		return;
	}

	scn->htc_htt_tx_endpoint = htc_htt_tx_endpoint;
}
qdf_export_symbol(hif_save_htc_htt_config_endpoint);

static const struct qwlan_hw qwlan_hw_list[] = {
	{
		.id = AR6320_REV1_VERSION,
		.subid = 0,
		.name = "QCA6174_REV1",
	},
	{
		.id = AR6320_REV1_1_VERSION,
		.subid = 0x1,
		.name = "QCA6174_REV1_1",
	},
	{
		.id = AR6320_REV1_3_VERSION,
		.subid = 0x2,
		.name = "QCA6174_REV1_3",
	},
	{
		.id = AR6320_REV2_1_VERSION,
		.subid = 0x4,
		.name = "QCA6174_REV2_1",
	},
	{
		.id = AR6320_REV2_1_VERSION,
		.subid = 0x5,
		.name = "QCA6174_REV2_2",
	},
	{
		.id = AR6320_REV3_VERSION,
		.subid = 0x6,
		.name = "QCA6174_REV2.3",
	},
	{
		.id = AR6320_REV3_VERSION,
		.subid = 0x8,
		.name = "QCA6174_REV3",
	},
	{
		.id = AR6320_REV3_VERSION,
		.subid = 0x9,
		.name = "QCA6174_REV3_1",
	},
	{
		.id = AR6320_REV3_2_VERSION,
		.subid = 0xA,
		.name = "AR6320_REV3_2_VERSION",
	},
	{
		.id = QCA6390_V1,
		.subid = 0x0,
		.name = "QCA6390_V1",
	},
	{
		.id = QCA6490_V1,
		.subid = 0x0,
		.name = "QCA6490_V1",
	},
	{
		.id = WCN3990_v1,
		.subid = 0x0,
		.name = "WCN3990_V1",
	},
	{
		.id = WCN3990_v2,
		.subid = 0x0,
		.name = "WCN3990_V2",
	},
	{
		.id = WCN3990_v2_1,
		.subid = 0x0,
		.name = "WCN3990_V2.1",
	},
	{
		.id = WCN3998,
		.subid = 0x0,
		.name = "WCN3998",
	},
	{
		.id = QCA9379_REV1_VERSION,
		.subid = 0xC,
		.name = "QCA9379_REV1",
	},
	{
		.id = QCA9379_REV1_VERSION,
		.subid = 0xD,
		.name = "QCA9379_REV1_1",
	},
	{
		.id = KIWI_V1,
		.subid = 0,
		.name = "KIWI_V1",
	},
	{
		.id = KIWI_V2,
		.subid = 0,
		.name = "KIWI_V2",
	},
	{
		.id = WCN6750_V1,
		.subid = 0,
		.name = "WCN6750_V1",
	},
	{
		.id = QCA6490_v2_1,
		.subid = 0,
		.name = "QCA6490",
	},
	{
		.id = QCA6490_v2,
		.subid = 0,
		.name = "QCA6490",
	},
	{
		.id = WCN3990_v2_2,
		.subid = 0,
		.name = "WCN3990_v2_2",
	},
	{
		.id = WCN3990_TALOS,
		.subid = 0,
		.name = "WCN3990",
	},
	{
		.id = WCN3990_MOOREA,
		.subid = 0,
		.name = "WCN3990",
	},
	{
		.id = WCN3990_SAIPAN,
		.subid = 0,
		.name = "WCN3990",
	},
	{
		.id = WCN3990_RENNELL,
		.subid = 0,
		.name = "WCN3990",
	},
	{
		.id = WCN3990_BITRA,
		.subid = 0,
		.name = "WCN3990",
	},
	{
		.id = WCN3990_DIVAR,
		.subid = 0,
		.name = "WCN3990",
	},
	{
		.id = WCN3990_ATHERTON,
		.subid = 0,
		.name = "WCN3990",
	},
	{
		.id = WCN3990_STRAIT,
		.subid = 0,
		.name = "WCN3990",
	},
	{
		.id = WCN3990_NETRANI,
		.subid = 0,
		.name = "WCN3990",
	},
	{
		.id = WCN3990_CLARENCE,
		.subid = 0,
		.name = "WCN3990",
	}
};

/**
 * hif_get_hw_name(): get a human readable name for the hardware
 * @info: Target Info
 *
 * Return: human readable name for the underlying wifi hardware.
 */
static const char *hif_get_hw_name(struct hif_target_info *info)
{
	int i;

	hif_debug("target version = %d, target revision = %d",
		  info->target_version,
		  info->target_revision);

	if (info->hw_name)
		return info->hw_name;

	for (i = 0; i < ARRAY_SIZE(qwlan_hw_list); i++) {
		if (info->target_version == qwlan_hw_list[i].id &&
		    info->target_revision == qwlan_hw_list[i].subid) {
			return qwlan_hw_list[i].name;
		}
	}

	info->hw_name = qdf_mem_malloc(64);
	if (!info->hw_name)
		return "Unknown Device (nomem)";

	i = qdf_snprint(info->hw_name, 64, "HW_VERSION=%x.",
			info->target_version);
	if (i < 0)
		return "Unknown Device (snprintf failure)";
	else
		return info->hw_name;
}

/**
 * hif_get_hw_info(): hif_get_hw_info
 * @scn: scn
 * @version: version
 * @revision: revision
 *
 * Return: n/a
 */
void hif_get_hw_info(struct hif_opaque_softc *scn, u32 *version, u32 *revision,
			const char **target_name)
{
	struct hif_target_info *info = hif_get_target_info_handle(scn);
	struct hif_softc *sc = HIF_GET_SOFTC(scn);

	if (sc->bus_type == QDF_BUS_TYPE_USB)
		hif_usb_get_hw_info(sc);

	*version = info->target_version;
	*revision = info->target_revision;
	*target_name = hif_get_hw_name(info);
}

/**
 * hif_get_dev_ba(): API to get device base address.
 * @scn: scn
 * @version: version
 * @revision: revision
 *
 * Return: n/a
 */
void *hif_get_dev_ba(struct hif_opaque_softc *hif_handle)
{
	struct hif_softc *scn = (struct hif_softc *)hif_handle;

	return scn->mem;
}
qdf_export_symbol(hif_get_dev_ba);

/**
 * hif_get_dev_ba_ce(): API to get device ce base address.
 * @scn: scn
 *
 * Return: dev mem base address for CE
 */
void *hif_get_dev_ba_ce(struct hif_opaque_softc *hif_handle)
{
	struct hif_softc *scn = (struct hif_softc *)hif_handle;

	return scn->mem_ce;
}

qdf_export_symbol(hif_get_dev_ba_ce);

#ifdef WLAN_CE_INTERRUPT_THRESHOLD_CONFIG
/**
 * hif_get_cfg_from_psoc() - Retrieve ini cfg from psoc
 * @scn: hif context
 * @psoc: psoc objmgr handle
 *
 * Return: None
 */
static inline
void hif_get_cfg_from_psoc(struct hif_softc *scn,
			   struct wlan_objmgr_psoc *psoc)
{
	if (psoc) {
		scn->ini_cfg.ce_status_ring_timer_threshold =
			cfg_get(psoc,
				CFG_CE_STATUS_RING_TIMER_THRESHOLD);
		scn->ini_cfg.ce_status_ring_batch_count_threshold =
			cfg_get(psoc,
				CFG_CE_STATUS_RING_BATCH_COUNT_THRESHOLD);
	}
}
#else
static inline
void hif_get_cfg_from_psoc(struct hif_softc *scn,
			   struct wlan_objmgr_psoc *psoc)
{
}
#endif /* WLAN_CE_INTERRUPT_THRESHOLD_CONFIG */

#if defined(HIF_CE_LOG_INFO) || defined(HIF_BUS_LOG_INFO)
/**
 * hif_recovery_notifier_cb - Recovery notifier callback to log
 *  hang event data
 * @block: notifier block
 * @state: state
 * @data: notifier data
 *
 * Return: status
 */
static
int hif_recovery_notifier_cb(struct notifier_block *block, unsigned long state,
			     void *data)
{
	struct qdf_notifer_data *notif_data = data;
	qdf_notif_block *notif_block;
	struct hif_softc *hif_handle;
	bool bus_id_invalid;

	if (!data || !block)
		return -EINVAL;

	notif_block = qdf_container_of(block, qdf_notif_block, notif_block);

	hif_handle = notif_block->priv_data;
	if (!hif_handle)
		return -EINVAL;

	bus_id_invalid = hif_log_bus_info(hif_handle, notif_data->hang_data,
					  &notif_data->offset);
	if (bus_id_invalid)
		return NOTIFY_STOP_MASK;

	hif_log_ce_info(hif_handle, notif_data->hang_data,
			&notif_data->offset);

	return 0;
}

/**
 * hif_register_recovery_notifier - Register hif recovery notifier
 * @hif_handle: hif handle
 *
 * Return: status
 */
static
QDF_STATUS hif_register_recovery_notifier(struct hif_softc *hif_handle)
{
	qdf_notif_block *hif_notifier;

	if (!hif_handle)
		return QDF_STATUS_E_FAILURE;

	hif_notifier = &hif_handle->hif_recovery_notifier;

	hif_notifier->notif_block.notifier_call = hif_recovery_notifier_cb;
	hif_notifier->priv_data = hif_handle;
	return qdf_hang_event_register_notifier(hif_notifier);
}

/**
 * hif_unregister_recovery_notifier - Un-register hif recovery notifier
 * @hif_handle: hif handle
 *
 * Return: status
 */
static
QDF_STATUS hif_unregister_recovery_notifier(struct hif_softc *hif_handle)
{
	qdf_notif_block *hif_notifier = &hif_handle->hif_recovery_notifier;

	return qdf_hang_event_unregister_notifier(hif_notifier);
}
#else
static inline
QDF_STATUS hif_register_recovery_notifier(struct hif_softc *hif_handle)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS hif_unregister_recovery_notifier(struct hif_softc *hif_handle)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef HIF_CPU_PERF_AFFINE_MASK
/**
 * __hif_cpu_hotplug_notify() - CPU hotplug event handler
 * @cpu: CPU Id of the CPU generating the event
 * @cpu_up: true if the CPU is online
 *
 * Return: None
 */
static void __hif_cpu_hotplug_notify(void *context,
				     uint32_t cpu, bool cpu_up)
{
	struct hif_softc *scn = context;

	if (!scn)
		return;
	if (hif_is_driver_unloading(scn) || hif_is_recovery_in_progress(scn))
		return;

	if (cpu_up) {
		hif_config_irq_set_perf_affinity_hint(GET_HIF_OPAQUE_HDL(scn));
		hif_debug("Setting affinity for online CPU: %d", cpu);
	} else {
		hif_debug("Skip setting affinity for offline CPU: %d", cpu);
	}
}

/**
 * hif_cpu_hotplug_notify - cpu core up/down notification
 * handler
 * @cpu: CPU generating the event
 * @cpu_up: true if the CPU is online
 *
 * Return: None
 */
static void hif_cpu_hotplug_notify(void *context, uint32_t cpu, bool cpu_up)
{
	struct qdf_op_sync *op_sync;

	if (qdf_op_protect(&op_sync))
		return;

	__hif_cpu_hotplug_notify(context, cpu, cpu_up);

	qdf_op_unprotect(op_sync);
}

static void hif_cpu_online_cb(void *context, uint32_t cpu)
{
	hif_cpu_hotplug_notify(context, cpu, true);
}

static void hif_cpu_before_offline_cb(void *context, uint32_t cpu)
{
	hif_cpu_hotplug_notify(context, cpu, false);
}

static void hif_cpuhp_register(struct hif_softc *scn)
{
	if (!scn) {
		hif_info_high("cannot register hotplug notifiers");
		return;
	}
	qdf_cpuhp_register(&scn->cpuhp_event_handle,
			   scn,
			   hif_cpu_online_cb,
			   hif_cpu_before_offline_cb);
}

static void hif_cpuhp_unregister(struct hif_softc *scn)
{
	if (!scn) {
		hif_info_high("cannot unregister hotplug notifiers");
		return;
	}
	qdf_cpuhp_unregister(&scn->cpuhp_event_handle);
}

#else
static void hif_cpuhp_register(struct hif_softc *scn)
{
}

static void hif_cpuhp_unregister(struct hif_softc *scn)
{
}
#endif /* ifdef HIF_CPU_PERF_AFFINE_MASK */

#ifdef HIF_DETECTION_LATENCY_ENABLE

void hif_tasklet_latency(struct hif_softc *scn, bool from_timer)
{
	qdf_time_t ce2_tasklet_sched_time =
		scn->latency_detect.ce2_tasklet_sched_time;
	qdf_time_t ce2_tasklet_exec_time =
		scn->latency_detect.ce2_tasklet_exec_time;
	qdf_time_t curr_jiffies = qdf_system_ticks();
	uint32_t detect_latency_threshold =
		scn->latency_detect.detect_latency_threshold;
	int cpu_id = qdf_get_cpu();

	/* 2 kinds of check here.
	 * from_timer==true:  check if tasklet stall
	 * from_timer==false: check tasklet execute comes late
	 */

	if ((from_timer ?
	    qdf_system_time_after(ce2_tasklet_sched_time,
				  ce2_tasklet_exec_time) :
	    qdf_system_time_after(ce2_tasklet_exec_time,
				  ce2_tasklet_sched_time)) &&
	    qdf_system_time_after(
		curr_jiffies,
		ce2_tasklet_sched_time +
		qdf_system_msecs_to_ticks(detect_latency_threshold))) {
		hif_err("tasklet ce2 latency: from_timer %d, curr_jiffies %lu, ce2_tasklet_sched_time %lu,ce2_tasklet_exec_time %lu, detect_latency_threshold %ums detect_latency_timer_timeout %ums, cpu_id %d, called: %ps",
			from_timer, curr_jiffies, ce2_tasklet_sched_time,
			ce2_tasklet_exec_time, detect_latency_threshold,
			scn->latency_detect.detect_latency_timer_timeout,
			cpu_id, (void *)_RET_IP_);
		goto latency;
	}
	return;

latency:
	qdf_trigger_self_recovery(NULL, QDF_TASKLET_CREDIT_LATENCY_DETECT);
}

void hif_credit_latency(struct hif_softc *scn, bool from_timer)
{
	qdf_time_t credit_request_time =
		scn->latency_detect.credit_request_time;
	qdf_time_t credit_report_time =
		scn->latency_detect.credit_report_time;
	qdf_time_t curr_jiffies = qdf_system_ticks();
	uint32_t detect_latency_threshold =
		scn->latency_detect.detect_latency_threshold;
	int cpu_id = qdf_get_cpu();

	/* 2 kinds of check here.
	 * from_timer==true:  check if credit report stall
	 * from_timer==false: check credit report comes late
	 */

	if ((from_timer ?
	    qdf_system_time_after(credit_request_time,
				  credit_report_time) :
	    qdf_system_time_after(credit_report_time,
				  credit_request_time)) &&
	    qdf_system_time_after(
		curr_jiffies,
		credit_request_time +
		qdf_system_msecs_to_ticks(detect_latency_threshold))) {
		hif_err("credit report latency: from timer %d, curr_jiffies %lu, credit_request_time %lu,credit_report_time %lu, detect_latency_threshold %ums, detect_latency_timer_timeout %ums, cpu_id %d, called: %ps",
			from_timer, curr_jiffies, credit_request_time,
			credit_report_time, detect_latency_threshold,
			scn->latency_detect.detect_latency_timer_timeout,
			cpu_id, (void *)_RET_IP_);
		goto latency;
	}
	return;

latency:
	qdf_trigger_self_recovery(NULL, QDF_TASKLET_CREDIT_LATENCY_DETECT);
}

/**
 * hif_check_detection_latency(): to check if latency for tasklet/credit
 *
 * @scn: hif context
 * @from_timer: if called from timer handler
 * @bitmap_type: indicate if check tasklet or credit
 *
 * Return: none
 */
void hif_check_detection_latency(struct hif_softc *scn,
				 bool from_timer,
				 uint32_t bitmap_type)
{
	if (QDF_GLOBAL_MISSION_MODE != hif_get_conparam(scn))
		return;

	if (!scn->latency_detect.enable_detection)
		return;

	if (bitmap_type & BIT(HIF_DETECT_TASKLET))
		hif_tasklet_latency(scn, from_timer);

	if (bitmap_type & BIT(HIF_DETECT_CREDIT))
		hif_credit_latency(scn, from_timer);
}

static void hif_latency_detect_timeout_handler(void *arg)
{
	struct hif_softc *scn = (struct hif_softc *)arg;
	int next_cpu;

	hif_check_detection_latency(scn, true,
				    BIT(HIF_DETECT_TASKLET) |
				    BIT(HIF_DETECT_CREDIT));

	/* it need to make sure timer start on a differnt cpu,
	 * so it can detect the tasklet schedule stall, but there
	 * is still chance that, after timer has been started, then
	 * irq/tasklet happens on the same cpu, then tasklet will
	 * execute before softirq timer, if this tasklet stall, the
	 * timer can't detect it, we can accept this as a limition,
	 * if tasklet stall, anyway other place will detect it, just
	 * a little later.
	 */
	next_cpu = cpumask_any_but(
			cpu_active_mask,
			scn->latency_detect.ce2_tasklet_sched_cpuid);

	if (qdf_unlikely(next_cpu >= nr_cpu_ids)) {
		hif_debug("start timer on local");
		/* it doesn't found a available cpu, start on local cpu*/
		qdf_timer_mod(
			&scn->latency_detect.detect_latency_timer,
			scn->latency_detect.detect_latency_timer_timeout);
	} else {
		qdf_timer_start_on(
			&scn->latency_detect.detect_latency_timer,
			scn->latency_detect.detect_latency_timer_timeout,
			next_cpu);
	}
}

static void hif_latency_detect_timer_init(struct hif_softc *scn)
{
	if (!scn) {
		hif_info_high("scn is null");
		return;
	}

	if (QDF_GLOBAL_MISSION_MODE != hif_get_conparam(scn))
		return;

	scn->latency_detect.detect_latency_timer_timeout =
		DETECTION_TIMER_TIMEOUT;
	scn->latency_detect.detect_latency_threshold =
		DETECTION_LATENCY_THRESHOLD;

	hif_info("timer timeout %u, latency threshold %u",
		 scn->latency_detect.detect_latency_timer_timeout,
		 scn->latency_detect.detect_latency_threshold);

	scn->latency_detect.is_timer_started = false;

	qdf_timer_init(NULL,
		       &scn->latency_detect.detect_latency_timer,
		       &hif_latency_detect_timeout_handler,
		       scn,
		       QDF_TIMER_TYPE_SW_SPIN);
}

static void hif_latency_detect_timer_deinit(struct hif_softc *scn)
{
	if (QDF_GLOBAL_MISSION_MODE != hif_get_conparam(scn))
		return;

	hif_info("deinit timer");
	qdf_timer_free(&scn->latency_detect.detect_latency_timer);
}

void hif_latency_detect_timer_start(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (QDF_GLOBAL_MISSION_MODE != hif_get_conparam(scn))
		return;

	hif_debug_rl("start timer");
	if (scn->latency_detect.is_timer_started) {
		hif_info("timer has been started");
		return;
	}

	qdf_timer_start(&scn->latency_detect.detect_latency_timer,
			scn->latency_detect.detect_latency_timer_timeout);
	scn->latency_detect.is_timer_started = true;
}

void hif_latency_detect_timer_stop(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (QDF_GLOBAL_MISSION_MODE != hif_get_conparam(scn))
		return;

	hif_debug_rl("stop timer");

	qdf_timer_sync_cancel(&scn->latency_detect.detect_latency_timer);
	scn->latency_detect.is_timer_started = false;
}

void hif_latency_detect_credit_record_time(
	enum hif_credit_exchange_type type,
	struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (!scn) {
		hif_err("Could not do runtime put, scn is null");
		return;
	}

	if (QDF_GLOBAL_MISSION_MODE != hif_get_conparam(scn))
		return;

	if (HIF_REQUEST_CREDIT == type)
		scn->latency_detect.credit_request_time = qdf_system_ticks();
	else if (HIF_PROCESS_CREDIT_REPORT == type)
		scn->latency_detect.credit_report_time = qdf_system_ticks();

	hif_check_detection_latency(scn, false, BIT(HIF_DETECT_CREDIT));
}

void hif_set_enable_detection(struct hif_opaque_softc *hif_ctx, bool value)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (!scn) {
		hif_err("Could not do runtime put, scn is null");
		return;
	}

	if (QDF_GLOBAL_MISSION_MODE != hif_get_conparam(scn))
		return;

	scn->latency_detect.enable_detection = value;
}
#else
static void hif_latency_detect_timer_init(struct hif_softc *scn)
{}

static void hif_latency_detect_timer_deinit(struct hif_softc *scn)
{}
#endif
struct hif_opaque_softc *hif_open(qdf_device_t qdf_ctx,
				  uint32_t mode,
				  enum qdf_bus_type bus_type,
				  struct hif_driver_state_callbacks *cbk,
				  struct wlan_objmgr_psoc *psoc)
{
	struct hif_softc *scn;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	int bus_context_size = hif_bus_get_context_size(bus_type);

	if (bus_context_size == 0) {
		hif_err("context size 0 not allowed");
		return NULL;
	}

	scn = (struct hif_softc *)qdf_mem_malloc(bus_context_size);
	if (!scn)
		return GET_HIF_OPAQUE_HDL(scn);

	scn->qdf_dev = qdf_ctx;
	scn->hif_con_param = mode;
	qdf_atomic_init(&scn->active_tasklet_cnt);

	qdf_atomic_init(&scn->active_grp_tasklet_cnt);
	qdf_atomic_init(&scn->link_suspended);
	qdf_atomic_init(&scn->tasklet_from_intr);
	hif_system_pm_set_state_on(GET_HIF_OPAQUE_HDL(scn));
	qdf_mem_copy(&scn->callbacks, cbk,
		     sizeof(struct hif_driver_state_callbacks));
	scn->bus_type  = bus_type;

	hif_pm_set_link_state(GET_HIF_OPAQUE_HDL(scn), HIF_PM_LINK_STATE_DOWN);
	hif_allow_ep_vote_access(GET_HIF_OPAQUE_HDL(scn));
	hif_get_cfg_from_psoc(scn, psoc);

	hif_set_event_hist_mask(GET_HIF_OPAQUE_HDL(scn));
	status = hif_bus_open(scn, bus_type);
	if (status != QDF_STATUS_SUCCESS) {
		hif_err("hif_bus_open error = %d, bus_type = %d",
			status, bus_type);
		qdf_mem_free(scn);
		scn = NULL;
		goto out;
	}

	hif_cpuhp_register(scn);
	hif_latency_detect_timer_init(scn);

out:
	return GET_HIF_OPAQUE_HDL(scn);
}

#ifdef ADRASTEA_RRI_ON_DDR
/**
 * hif_uninit_rri_on_ddr(): free consistent memory allocated for rri
 * @scn: hif context
 *
 * Return: none
 */
void hif_uninit_rri_on_ddr(struct hif_softc *scn)
{
	if (scn->vaddr_rri_on_ddr)
		qdf_mem_free_consistent(scn->qdf_dev, scn->qdf_dev->dev,
					(CE_COUNT * sizeof(uint32_t)),
					scn->vaddr_rri_on_ddr,
					scn->paddr_rri_on_ddr, 0);
	scn->vaddr_rri_on_ddr = NULL;
}
#endif

/**
 * hif_close(): hif_close
 * @hif_ctx: hif_ctx
 *
 * Return: n/a
 */
void hif_close(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (!scn) {
		hif_err("hif_opaque_softc is NULL");
		return;
	}

	hif_latency_detect_timer_deinit(scn);

	if (scn->athdiag_procfs_inited) {
		athdiag_procfs_remove();
		scn->athdiag_procfs_inited = false;
	}

	if (scn->target_info.hw_name) {
		char *hw_name = scn->target_info.hw_name;

		scn->target_info.hw_name = "ErrUnloading";
		qdf_mem_free(hw_name);
	}

	hif_uninit_rri_on_ddr(scn);
	hif_cleanup_static_buf_to_target(scn);
	hif_cpuhp_unregister(scn);

	hif_bus_close(scn);

	qdf_mem_free(scn);
}

/**
 * hif_get_num_active_grp_tasklets() - get the number of active
 *		datapath group tasklets pending to be completed.
 * @scn: HIF context
 *
 * Returns: the number of datapath group tasklets which are active
 */
static inline int hif_get_num_active_grp_tasklets(struct hif_softc *scn)
{
	return qdf_atomic_read(&scn->active_grp_tasklet_cnt);
}

#if (defined(QCA_WIFI_QCA8074) || defined(QCA_WIFI_QCA6018) || \
	defined(QCA_WIFI_QCA6290) || defined(QCA_WIFI_QCA6390) || \
	defined(QCA_WIFI_QCN9000) || defined(QCA_WIFI_QCA6490) || \
	defined(QCA_WIFI_QCA6750) || defined(QCA_WIFI_QCA5018) || \
	defined(QCA_WIFI_KIWI) || defined(QCA_WIFI_QCN9224) || \
	defined(QCA_WIFI_QCA9574))
/**
 * hif_get_num_pending_work() - get the number of entries in
 *		the workqueue pending to be completed.
 * @scn: HIF context
 *
 * Returns: the number of tasklets which are active
 */
static inline int hif_get_num_pending_work(struct hif_softc *scn)
{
	return hal_get_reg_write_pending_work(scn->hal_soc);
}
#else

static inline int hif_get_num_pending_work(struct hif_softc *scn)
{
	return 0;
}
#endif

QDF_STATUS hif_try_complete_tasks(struct hif_softc *scn)
{
	uint32_t task_drain_wait_cnt = 0;
	int tasklet = 0, grp_tasklet = 0, work = 0;

	while ((tasklet = hif_get_num_active_tasklets(scn)) ||
	       (grp_tasklet = hif_get_num_active_grp_tasklets(scn)) ||
	       (work = hif_get_num_pending_work(scn))) {
		if (++task_drain_wait_cnt > HIF_TASK_DRAIN_WAIT_CNT) {
			hif_err("pending tasklets %d grp tasklets %d work %d",
				tasklet, grp_tasklet, work);
			return QDF_STATUS_E_FAULT;
		}
		hif_info("waiting for tasklets %d grp tasklets %d work %d",
			 tasklet, grp_tasklet, work);
		msleep(10);
	}

	return QDF_STATUS_SUCCESS;
}

#if defined(HIF_IPCI) && defined(FEATURE_HAL_DELAYED_REG_WRITE)
QDF_STATUS hif_try_prevent_ep_vote_access(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	uint32_t work_drain_wait_cnt = 0;
	uint32_t wait_cnt = 0;
	int work = 0;

	qdf_atomic_set(&scn->dp_ep_vote_access,
		       HIF_EP_VOTE_ACCESS_DISABLE);
	qdf_atomic_set(&scn->ep_vote_access,
		       HIF_EP_VOTE_ACCESS_DISABLE);

	while ((work = hif_get_num_pending_work(scn))) {
		if (++work_drain_wait_cnt > HIF_WORK_DRAIN_WAIT_CNT) {
			qdf_atomic_set(&scn->dp_ep_vote_access,
				       HIF_EP_VOTE_ACCESS_ENABLE);
			qdf_atomic_set(&scn->ep_vote_access,
				       HIF_EP_VOTE_ACCESS_ENABLE);
			hif_err("timeout wait for pending work %d ", work);
			return QDF_STATUS_E_FAULT;
		}
		qdf_sleep(10);
	}

	if (pld_is_pci_ep_awake(scn->qdf_dev->dev) == -ENOTSUPP)
	return QDF_STATUS_SUCCESS;

	while (pld_is_pci_ep_awake(scn->qdf_dev->dev)) {
		if (++wait_cnt > HIF_EP_WAKE_RESET_WAIT_CNT) {
			hif_err("Release EP vote is not proceed by Fw");
			return QDF_STATUS_E_FAULT;
		}
		qdf_sleep(5);
	}

	return QDF_STATUS_SUCCESS;
}

void hif_set_ep_intermediate_vote_access(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	uint8_t vote_access;

	vote_access = qdf_atomic_read(&scn->ep_vote_access);

	if (vote_access != HIF_EP_VOTE_ACCESS_DISABLE)
		hif_info("EP vote changed from:%u to intermediate state",
			 vote_access);

	if (QDF_IS_STATUS_ERROR(hif_try_prevent_ep_vote_access(hif_ctx)))
		QDF_BUG(0);

	qdf_atomic_set(&scn->ep_vote_access,
		       HIF_EP_VOTE_INTERMEDIATE_ACCESS);
}

void hif_allow_ep_vote_access(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	qdf_atomic_set(&scn->dp_ep_vote_access,
		       HIF_EP_VOTE_ACCESS_ENABLE);
	qdf_atomic_set(&scn->ep_vote_access,
		       HIF_EP_VOTE_ACCESS_ENABLE);
}

void hif_set_ep_vote_access(struct hif_opaque_softc *hif_ctx,
			    uint8_t type, uint8_t access)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (type == HIF_EP_VOTE_DP_ACCESS)
		qdf_atomic_set(&scn->dp_ep_vote_access, access);
	else
		qdf_atomic_set(&scn->ep_vote_access, access);
}

uint8_t hif_get_ep_vote_access(struct hif_opaque_softc *hif_ctx,
			       uint8_t type)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (type == HIF_EP_VOTE_DP_ACCESS)
		return qdf_atomic_read(&scn->dp_ep_vote_access);
	else
		return qdf_atomic_read(&scn->ep_vote_access);
}
#endif

#if (defined(QCA_WIFI_QCA8074) || defined(QCA_WIFI_QCA6018) || \
	defined(QCA_WIFI_QCA6290) || defined(QCA_WIFI_QCA6390) || \
	defined(QCA_WIFI_QCN9000) || defined(QCA_WIFI_QCA6490) || \
	defined(QCA_WIFI_QCA6750) || defined(QCA_WIFI_QCA5018) || \
	defined(QCA_WIFI_KIWI) || defined(QCA_WIFI_QCN9224) || \
	defined(QCA_WIFI_QCA9574))
static QDF_STATUS hif_hal_attach(struct hif_softc *scn)
{
	if (ce_srng_based(scn)) {
		scn->hal_soc = hal_attach(
					hif_softc_to_hif_opaque_softc(scn),
					scn->qdf_dev);
		if (!scn->hal_soc)
			return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS hif_hal_detach(struct hif_softc *scn)
{
	if (ce_srng_based(scn)) {
		hal_detach(scn->hal_soc);
		scn->hal_soc = NULL;
	}

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS hif_hal_attach(struct hif_softc *scn)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS hif_hal_detach(struct hif_softc *scn)
{
	return QDF_STATUS_SUCCESS;
}
#endif

int hif_init_dma_mask(struct device *dev, enum qdf_bus_type bus_type)
{
	int ret;

	switch (bus_type) {
	case QDF_BUS_TYPE_IPCI:
		ret = qdf_set_dma_coherent_mask(dev,
						DMA_COHERENT_MASK_DEFAULT);
		if (ret) {
			hif_err("Failed to set dma mask error = %d", ret);
			return ret;
		}

		break;
	default:
		/* Follow the existing sequence for other targets */
		break;
	}

	return 0;
}

/**
 * hif_enable(): hif_enable
 * @hif_ctx: hif_ctx
 * @dev: dev
 * @bdev: bus dev
 * @bid: bus ID
 * @bus_type: bus type
 * @type: enable type
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hif_enable(struct hif_opaque_softc *hif_ctx, struct device *dev,
					  void *bdev,
					  const struct hif_bus_id *bid,
					  enum qdf_bus_type bus_type,
					  enum hif_enable_type type)
{
	QDF_STATUS status;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (!scn) {
		hif_err("hif_ctx = NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = hif_enable_bus(scn, dev, bdev, bid, type);
	if (status != QDF_STATUS_SUCCESS) {
		hif_err("hif_enable_bus error = %d", status);
		return status;
	}

	hif_pm_set_link_state(GET_HIF_OPAQUE_HDL(scn), HIF_PM_LINK_STATE_UP);
	status = hif_hal_attach(scn);
	if (status != QDF_STATUS_SUCCESS) {
		hif_err("hal attach failed");
		goto disable_bus;
	}

	if (hif_bus_configure(scn)) {
		hif_err("Target probe failed");
		status = QDF_STATUS_E_FAILURE;
		goto hal_detach;
	}

	hif_ut_suspend_init(scn);
	hif_register_recovery_notifier(scn);
	hif_latency_detect_timer_start(hif_ctx);

	/*
	 * Flag to avoid potential unallocated memory access from MSI
	 * interrupt handler which could get scheduled as soon as MSI
	 * is enabled, i.e to take care of the race due to the order
	 * in where MSI is enabled before the memory, that will be
	 * in interrupt handlers, is allocated.
	 */

	scn->hif_init_done = true;

	hif_debug("OK");

	return QDF_STATUS_SUCCESS;

hal_detach:
	hif_hal_detach(scn);
disable_bus:
	hif_disable_bus(scn);
	return status;
}

void hif_disable(struct hif_opaque_softc *hif_ctx, enum hif_disable_type type)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (!scn)
		return;

	hif_set_enable_detection(hif_ctx, false);
	hif_latency_detect_timer_stop(hif_ctx);

	hif_unregister_recovery_notifier(scn);

	hif_nointrs(scn);
	if (scn->hif_init_done == false)
		hif_shutdown_device(hif_ctx);
	else
		hif_stop(hif_ctx);

	hif_hal_detach(scn);

	hif_pm_set_link_state(hif_ctx, HIF_PM_LINK_STATE_DOWN);
	hif_disable_bus(scn);

	hif_wlan_disable(scn);

	scn->notice_send = false;

	hif_debug("X");
}

#ifdef CE_TASKLET_DEBUG_ENABLE
void hif_enable_ce_latency_stats(struct hif_opaque_softc *hif_ctx, uint8_t val)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (!scn)
		return;

	scn->ce_latency_stats = val;
}
#endif

void hif_display_stats(struct hif_opaque_softc *hif_ctx)
{
	hif_display_bus_stats(hif_ctx);
}

qdf_export_symbol(hif_display_stats);

void hif_clear_stats(struct hif_opaque_softc *hif_ctx)
{
	hif_clear_bus_stats(hif_ctx);
}

/**
 * hif_crash_shutdown_dump_bus_register() - dump bus registers
 * @hif_ctx: hif_ctx
 *
 * Return: n/a
 */
#if defined(TARGET_RAMDUMP_AFTER_KERNEL_PANIC) && defined(WLAN_FEATURE_BMI)

static void hif_crash_shutdown_dump_bus_register(void *hif_ctx)
{
	struct hif_opaque_softc *scn = hif_ctx;

	if (hif_check_soc_status(scn))
		return;

	if (hif_dump_registers(scn))
		hif_err("Failed to dump bus registers!");
}

/**
 * hif_crash_shutdown(): hif_crash_shutdown
 *
 * This function is called by the platform driver to dump CE registers
 *
 * @hif_ctx: hif_ctx
 *
 * Return: n/a
 */
void hif_crash_shutdown(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (!hif_ctx)
		return;

	if (scn->bus_type == QDF_BUS_TYPE_SNOC) {
		hif_warn("RAM dump disabled for bustype %d", scn->bus_type);
		return;
	}

	if (TARGET_STATUS_RESET == scn->target_status) {
		hif_warn("Target is already asserted, ignore!");
		return;
	}

	if (hif_is_load_or_unload_in_progress(scn)) {
		hif_err("Load/unload is in progress, ignore!");
		return;
	}

	hif_crash_shutdown_dump_bus_register(hif_ctx);
	hif_set_target_status(hif_ctx, TARGET_STATUS_RESET);

	if (ol_copy_ramdump(hif_ctx))
		goto out;

	hif_info("RAM dump collecting completed!");

out:
	return;
}
#else
void hif_crash_shutdown(struct hif_opaque_softc *hif_ctx)
{
	hif_debug("Collecting target RAM dump disabled");
}
#endif /* TARGET_RAMDUMP_AFTER_KERNEL_PANIC */

#ifdef QCA_WIFI_3_0
/**
 * hif_check_fw_reg(): hif_check_fw_reg
 * @scn: scn
 * @state:
 *
 * Return: int
 */
int hif_check_fw_reg(struct hif_opaque_softc *scn)
{
	return 0;
}
#endif

/**
 * hif_read_phy_mem_base(): hif_read_phy_mem_base
 * @scn: scn
 * @phy_mem_base: physical mem base
 *
 * Return: n/a
 */
void hif_read_phy_mem_base(struct hif_softc *scn, qdf_dma_addr_t *phy_mem_base)
{
	*phy_mem_base = scn->mem_pa;
}
qdf_export_symbol(hif_read_phy_mem_base);

/**
 * hif_get_device_type(): hif_get_device_type
 * @device_id: device_id
 * @revision_id: revision_id
 * @hif_type: returned hif_type
 * @target_type: returned target_type
 *
 * Return: int
 */
int hif_get_device_type(uint32_t device_id,
			uint32_t revision_id,
			uint32_t *hif_type, uint32_t *target_type)
{
	int ret = 0;

	switch (device_id) {
	case ADRASTEA_DEVICE_ID_P2_E12:

		*hif_type = HIF_TYPE_ADRASTEA;
		*target_type = TARGET_TYPE_ADRASTEA;
		break;

	case AR9888_DEVICE_ID:
		*hif_type = HIF_TYPE_AR9888;
		*target_type = TARGET_TYPE_AR9888;
		break;

	case AR6320_DEVICE_ID:
		switch (revision_id) {
		case AR6320_FW_1_1:
		case AR6320_FW_1_3:
			*hif_type = HIF_TYPE_AR6320;
			*target_type = TARGET_TYPE_AR6320;
			break;

		case AR6320_FW_2_0:
		case AR6320_FW_3_0:
		case AR6320_FW_3_2:
			*hif_type = HIF_TYPE_AR6320V2;
			*target_type = TARGET_TYPE_AR6320V2;
			break;

		default:
			hif_err("dev_id = 0x%x, rev_id = 0x%x",
				device_id, revision_id);
			ret = -ENODEV;
			goto end;
		}
		break;

	case AR9887_DEVICE_ID:
		*hif_type = HIF_TYPE_AR9888;
		*target_type = TARGET_TYPE_AR9888;
		hif_info(" *********** AR9887 **************");
		break;

	case QCA9984_DEVICE_ID:
		*hif_type = HIF_TYPE_QCA9984;
		*target_type = TARGET_TYPE_QCA9984;
		hif_info(" *********** QCA9984 *************");
		break;

	case QCA9888_DEVICE_ID:
		*hif_type = HIF_TYPE_QCA9888;
		*target_type = TARGET_TYPE_QCA9888;
		hif_info(" *********** QCA9888 *************");
		break;

	case AR900B_DEVICE_ID:
		*hif_type = HIF_TYPE_AR900B;
		*target_type = TARGET_TYPE_AR900B;
		hif_info(" *********** AR900B *************");
		break;

	case IPQ4019_DEVICE_ID:
		*hif_type = HIF_TYPE_IPQ4019;
		*target_type = TARGET_TYPE_IPQ4019;
		hif_info(" *********** IPQ4019  *************");
		break;

	case QCA8074_DEVICE_ID:
		*hif_type = HIF_TYPE_QCA8074;
		*target_type = TARGET_TYPE_QCA8074;
		hif_info(" *********** QCA8074  *************");
		break;

	case QCA6290_EMULATION_DEVICE_ID:
	case QCA6290_DEVICE_ID:
		*hif_type = HIF_TYPE_QCA6290;
		*target_type = TARGET_TYPE_QCA6290;
		hif_info(" *********** QCA6290EMU *************");
		break;

	case QCN9000_DEVICE_ID:
		*hif_type = HIF_TYPE_QCN9000;
		*target_type = TARGET_TYPE_QCN9000;
		hif_info(" *********** QCN9000 *************");
		break;

	case QCN9224_DEVICE_ID:
		*hif_type = HIF_TYPE_QCN9224;
		*target_type = TARGET_TYPE_QCN9224;
		hif_info(" *********** QCN9224 *************");
		break;

	case QCN6122_DEVICE_ID:
		*hif_type = HIF_TYPE_QCN6122;
		*target_type = TARGET_TYPE_QCN6122;
		hif_info(" *********** QCN6122 *************");
		break;

	case QCN7605_DEVICE_ID:
	case QCN7605_COMPOSITE:
	case QCN7605_STANDALONE:
	case QCN7605_STANDALONE_V2:
	case QCN7605_COMPOSITE_V2:
		*hif_type = HIF_TYPE_QCN7605;
		*target_type = TARGET_TYPE_QCN7605;
		hif_info(" *********** QCN7605 *************");
		break;

	case QCA6390_DEVICE_ID:
	case QCA6390_EMULATION_DEVICE_ID:
		*hif_type = HIF_TYPE_QCA6390;
		*target_type = TARGET_TYPE_QCA6390;
		hif_info(" *********** QCA6390 *************");
		break;

	case QCA6490_DEVICE_ID:
	case QCA6490_EMULATION_DEVICE_ID:
		*hif_type = HIF_TYPE_QCA6490;
		*target_type = TARGET_TYPE_QCA6490;
		hif_info(" *********** QCA6490 *************");
		break;

	case QCA6750_DEVICE_ID:
	case QCA6750_EMULATION_DEVICE_ID:
		*hif_type = HIF_TYPE_QCA6750;
		*target_type = TARGET_TYPE_QCA6750;
		hif_info(" *********** QCA6750 *************");
		break;

	case KIWI_DEVICE_ID:
		*hif_type = HIF_TYPE_KIWI;
		*target_type = TARGET_TYPE_KIWI;
		hif_info(" *********** KIWI *************");
		break;

	case QCA8074V2_DEVICE_ID:
		*hif_type = HIF_TYPE_QCA8074V2;
		*target_type = TARGET_TYPE_QCA8074V2;
		hif_info(" *********** QCA8074V2 *************");
		break;

	case QCA6018_DEVICE_ID:
	case RUMIM2M_DEVICE_ID_NODE0:
	case RUMIM2M_DEVICE_ID_NODE1:
	case RUMIM2M_DEVICE_ID_NODE2:
	case RUMIM2M_DEVICE_ID_NODE3:
	case RUMIM2M_DEVICE_ID_NODE4:
	case RUMIM2M_DEVICE_ID_NODE5:
		*hif_type = HIF_TYPE_QCA6018;
		*target_type = TARGET_TYPE_QCA6018;
		hif_info(" *********** QCA6018 *************");
		break;

	case QCA5018_DEVICE_ID:
		*hif_type = HIF_TYPE_QCA5018;
		*target_type = TARGET_TYPE_QCA5018;
		hif_info(" *********** qca5018 *************");
		break;

	case QCA9574_DEVICE_ID:
		*hif_type = HIF_TYPE_QCA9574;
		*target_type = TARGET_TYPE_QCA9574;
		hif_info(" *********** QCA9574 *************");
		break;

	default:
		hif_err("Unsupported device ID = 0x%x!", device_id);
		ret = -ENODEV;
		break;
	}

	if (*target_type == TARGET_TYPE_UNKNOWN) {
		hif_err("Unsupported target_type!");
		ret = -ENODEV;
	}
end:
	return ret;
}

/**
 * hif_get_bus_type() - return the bus type
 *
 * Return: enum qdf_bus_type
 */
enum qdf_bus_type hif_get_bus_type(struct hif_opaque_softc *hif_hdl)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_hdl);

	return scn->bus_type;
}

/**
 * Target info and ini parameters are global to the driver
 * Hence these structures are exposed to all the modules in
 * the driver and they don't need to maintains multiple copies
 * of the same info, instead get the handle from hif and
 * modify them in hif
 */

/**
 * hif_get_ini_handle() - API to get hif_config_param handle
 * @hif_ctx: HIF Context
 *
 * Return: pointer to hif_config_info
 */
struct hif_config_info *hif_get_ini_handle(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *sc = HIF_GET_SOFTC(hif_ctx);

	return &sc->hif_config;
}

/**
 * hif_get_target_info_handle() - API to get hif_target_info handle
 * @hif_ctx: HIF context
 *
 * Return: Pointer to hif_target_info
 */
struct hif_target_info *hif_get_target_info_handle(
					struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *sc = HIF_GET_SOFTC(hif_ctx);

	return &sc->target_info;

}
qdf_export_symbol(hif_get_target_info_handle);

#ifdef RECEIVE_OFFLOAD
void hif_offld_flush_cb_register(struct hif_opaque_softc *scn,
				 void (offld_flush_handler)(void *))
{
	if (hif_napi_enabled(scn, -1))
		hif_napi_rx_offld_flush_cb_register(scn, offld_flush_handler);
	else
		hif_err("NAPI not enabled");
}
qdf_export_symbol(hif_offld_flush_cb_register);

void hif_offld_flush_cb_deregister(struct hif_opaque_softc *scn)
{
	if (hif_napi_enabled(scn, -1))
		hif_napi_rx_offld_flush_cb_deregister(scn);
	else
		hif_err("NAPI not enabled");
}
qdf_export_symbol(hif_offld_flush_cb_deregister);

int hif_get_rx_ctx_id(int ctx_id, struct hif_opaque_softc *hif_hdl)
{
	if (hif_napi_enabled(hif_hdl, -1))
		return NAPI_PIPE2ID(ctx_id);
	else
		return ctx_id;
}
#else /* RECEIVE_OFFLOAD */
int hif_get_rx_ctx_id(int ctx_id, struct hif_opaque_softc *hif_hdl)
{
	return 0;
}
qdf_export_symbol(hif_get_rx_ctx_id);
#endif /* RECEIVE_OFFLOAD */

#if defined(FEATURE_LRO)

/**
 * hif_get_lro_info - Returns LRO instance for instance ID
 * @ctx_id: LRO instance ID
 * @hif_hdl: HIF Context
 *
 * Return: Pointer to LRO instance.
 */
void *hif_get_lro_info(int ctx_id, struct hif_opaque_softc *hif_hdl)
{
	void *data;

	if (hif_napi_enabled(hif_hdl, -1))
		data = hif_napi_get_lro_info(hif_hdl, ctx_id);
	else
		data = hif_ce_get_lro_ctx(hif_hdl, ctx_id);

	return data;
}
#endif

/**
 * hif_get_target_status - API to get target status
 * @hif_ctx: HIF Context
 *
 * Return: enum hif_target_status
 */
enum hif_target_status hif_get_target_status(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	return scn->target_status;
}
qdf_export_symbol(hif_get_target_status);

/**
 * hif_set_target_status() - API to set target status
 * @hif_ctx: HIF Context
 * @status: Target Status
 *
 * Return: void
 */
void hif_set_target_status(struct hif_opaque_softc *hif_ctx, enum
			   hif_target_status status)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	scn->target_status = status;
}

/**
 * hif_init_ini_config() - API to initialize HIF configuration parameters
 * @hif_ctx: HIF Context
 * @cfg: HIF Configuration
 *
 * Return: void
 */
void hif_init_ini_config(struct hif_opaque_softc *hif_ctx,
			 struct hif_config_info *cfg)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	qdf_mem_copy(&scn->hif_config, cfg, sizeof(struct hif_config_info));
}

/**
 * hif_get_conparam() - API to get driver mode in HIF
 * @scn: HIF Context
 *
 * Return: driver mode of operation
 */
uint32_t hif_get_conparam(struct hif_softc *scn)
{
	if (!scn)
		return 0;

	return scn->hif_con_param;
}

/**
 * hif_get_callbacks_handle() - API to get callbacks Handle
 * @scn: HIF Context
 *
 * Return: pointer to HIF Callbacks
 */
struct hif_driver_state_callbacks *hif_get_callbacks_handle(
							struct hif_softc *scn)
{
	return &scn->callbacks;
}

/**
 * hif_is_driver_unloading() - API to query upper layers if driver is unloading
 * @scn: HIF Context
 *
 * Return: True/False
 */
bool hif_is_driver_unloading(struct hif_softc *scn)
{
	struct hif_driver_state_callbacks *cbk = hif_get_callbacks_handle(scn);

	if (cbk && cbk->is_driver_unloading)
		return cbk->is_driver_unloading(cbk->context);

	return false;
}

/**
 * hif_is_load_or_unload_in_progress() - API to query upper layers if
 * load/unload in progress
 * @scn: HIF Context
 *
 * Return: True/False
 */
bool hif_is_load_or_unload_in_progress(struct hif_softc *scn)
{
	struct hif_driver_state_callbacks *cbk = hif_get_callbacks_handle(scn);

	if (cbk && cbk->is_load_unload_in_progress)
		return cbk->is_load_unload_in_progress(cbk->context);

	return false;
}

/**
 * hif_is_recovery_in_progress() - API to query upper layers if recovery in
 * progress
 * @scn: HIF Context
 *
 * Return: True/False
 */
bool hif_is_recovery_in_progress(struct hif_softc *scn)
{
	struct hif_driver_state_callbacks *cbk = hif_get_callbacks_handle(scn);

	if (cbk && cbk->is_recovery_in_progress)
		return cbk->is_recovery_in_progress(cbk->context);

	return false;
}

#if defined(HIF_PCI) || defined(HIF_SNOC) || defined(HIF_AHB) || \
    defined(HIF_IPCI)

/**
 * hif_update_pipe_callback() - API to register pipe specific callbacks
 * @osc: Opaque softc
 * @pipeid: pipe id
 * @callbacks: callbacks to register
 *
 * Return: void
 */

void hif_update_pipe_callback(struct hif_opaque_softc *osc,
					u_int8_t pipeid,
					struct hif_msg_callbacks *callbacks)
{
	struct hif_softc *scn = HIF_GET_SOFTC(osc);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct HIF_CE_pipe_info *pipe_info;

	QDF_BUG(pipeid < CE_COUNT_MAX);

	hif_debug("pipeid: %d", pipeid);

	pipe_info = &hif_state->pipe_info[pipeid];

	qdf_mem_copy(&pipe_info->pipe_callbacks,
			callbacks, sizeof(pipe_info->pipe_callbacks));
}
qdf_export_symbol(hif_update_pipe_callback);

/**
 * hif_is_target_ready() - API to query if target is in ready state
 * progress
 * @scn: HIF Context
 *
 * Return: True/False
 */
bool hif_is_target_ready(struct hif_softc *scn)
{
	struct hif_driver_state_callbacks *cbk = hif_get_callbacks_handle(scn);

	if (cbk && cbk->is_target_ready)
		return cbk->is_target_ready(cbk->context);
	/*
	 * if callback is not registered then there is no way to determine
	 * if target is ready. In-such case return true to indicate that
	 * target is ready.
	 */
	return true;
}
qdf_export_symbol(hif_is_target_ready);

int hif_get_bandwidth_level(struct hif_opaque_softc *hif_handle)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_handle);
	struct hif_driver_state_callbacks *cbk = hif_get_callbacks_handle(scn);

	if (cbk && cbk->get_bandwidth_level)
		return cbk->get_bandwidth_level(cbk->context);

	return 0;
}

qdf_export_symbol(hif_get_bandwidth_level);

#ifdef DP_MEM_PRE_ALLOC
void *hif_mem_alloc_consistent_unaligned(struct hif_softc *scn,
					 qdf_size_t size,
					 qdf_dma_addr_t *paddr,
					 uint32_t ring_type,
					 uint8_t *is_mem_prealloc)
{
	void *vaddr = NULL;
	struct hif_driver_state_callbacks *cbk =
				hif_get_callbacks_handle(scn);

	*is_mem_prealloc = false;
	if (cbk && cbk->prealloc_get_consistent_mem_unaligned) {
		vaddr = cbk->prealloc_get_consistent_mem_unaligned(size,
								   paddr,
								   ring_type);
		if (vaddr) {
			*is_mem_prealloc = true;
			goto end;
		}
	}

	vaddr = qdf_mem_alloc_consistent(scn->qdf_dev,
					 scn->qdf_dev->dev,
					 size,
					 paddr);
end:
	dp_info("%s va_unaligned %pK pa_unaligned %pK size %d ring_type %d",
		*is_mem_prealloc ? "pre-alloc" : "dynamic-alloc", vaddr,
		(void *)*paddr, (int)size, ring_type);

	return vaddr;
}

void hif_mem_free_consistent_unaligned(struct hif_softc *scn,
				       qdf_size_t size,
				       void *vaddr,
				       qdf_dma_addr_t paddr,
				       qdf_dma_context_t memctx,
				       uint8_t is_mem_prealloc)
{
	struct hif_driver_state_callbacks *cbk =
				hif_get_callbacks_handle(scn);

	if (is_mem_prealloc) {
		if (cbk && cbk->prealloc_put_consistent_mem_unaligned) {
			cbk->prealloc_put_consistent_mem_unaligned(vaddr);
		} else {
			dp_warn("dp_prealloc_put_consistent_unligned NULL");
			QDF_BUG(0);
		}
	} else {
		qdf_mem_free_consistent(scn->qdf_dev, scn->qdf_dev->dev,
					size, vaddr, paddr, memctx);
	}
}
#endif

/**
 * hif_batch_send() - API to access hif specific function
 * ce_batch_send.
 * @osc: HIF Context
 * @msdu : list of msdus to be sent
 * @transfer_id : transfer id
 * @len : donwloaded length
 *
 * Return: list of msds not sent
 */
qdf_nbuf_t hif_batch_send(struct hif_opaque_softc *osc, qdf_nbuf_t msdu,
		uint32_t transfer_id, u_int32_t len, uint32_t sendhead)
{
	void *ce_tx_hdl = hif_get_ce_handle(osc, CE_HTT_TX_CE);

	if (!ce_tx_hdl)
		return NULL;

	return ce_batch_send((struct CE_handle *)ce_tx_hdl, msdu, transfer_id,
			len, sendhead);
}
qdf_export_symbol(hif_batch_send);

/**
 * hif_update_tx_ring() - API to access hif specific function
 * ce_update_tx_ring.
 * @osc: HIF Context
 * @num_htt_cmpls : number of htt compl received.
 *
 * Return: void
 */
void hif_update_tx_ring(struct hif_opaque_softc *osc, u_int32_t num_htt_cmpls)
{
	void *ce_tx_hdl = hif_get_ce_handle(osc, CE_HTT_TX_CE);

	ce_update_tx_ring(ce_tx_hdl, num_htt_cmpls);
}
qdf_export_symbol(hif_update_tx_ring);


/**
 * hif_send_single() - API to access hif specific function
 * ce_send_single.
 * @osc: HIF Context
 * @msdu : msdu to be sent
 * @transfer_id: transfer id
 * @len : downloaded length
 *
 * Return: msdu sent status
 */
QDF_STATUS hif_send_single(struct hif_opaque_softc *osc, qdf_nbuf_t msdu,
			   uint32_t transfer_id, u_int32_t len)
{
	void *ce_tx_hdl = hif_get_ce_handle(osc, CE_HTT_TX_CE);

	if (!ce_tx_hdl)
		return QDF_STATUS_E_NULL_VALUE;

	return ce_send_single((struct CE_handle *)ce_tx_hdl, msdu, transfer_id,
			len);
}
qdf_export_symbol(hif_send_single);
#endif

/**
 * hif_reg_write() - API to access hif specific function
 * hif_write32_mb.
 * @hif_ctx : HIF Context
 * @offset : offset on which value has to be written
 * @value : value to be written
 *
 * Return: None
 */
void hif_reg_write(struct hif_opaque_softc *hif_ctx, uint32_t offset,
		uint32_t value)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	hif_write32_mb(scn, scn->mem + offset, value);

}
qdf_export_symbol(hif_reg_write);

/**
 * hif_reg_read() - API to access hif specific function
 * hif_read32_mb.
 * @hif_ctx : HIF Context
 * @offset : offset from which value has to be read
 *
 * Return: Read value
 */
uint32_t hif_reg_read(struct hif_opaque_softc *hif_ctx, uint32_t offset)
{

	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	return hif_read32_mb(scn, scn->mem + offset);
}
qdf_export_symbol(hif_reg_read);

/**
 * hif_ramdump_handler(): generic ramdump handler
 * @scn: struct hif_opaque_softc
 *
 * Return: None
 */
void hif_ramdump_handler(struct hif_opaque_softc *scn)
{
	if (hif_get_bus_type(scn) == QDF_BUS_TYPE_USB)
		hif_usb_ramdump_handler(scn);
}

hif_pm_wake_irq_type hif_pm_get_wake_irq_type(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	return scn->wake_irq_type;
}

irqreturn_t hif_wake_interrupt_handler(int irq, void *context)
{
	struct hif_softc *scn = context;
	struct hif_opaque_softc *hif_ctx = GET_HIF_OPAQUE_HDL(scn);

	hif_info("wake interrupt received on irq %d", irq);

	hif_pm_runtime_set_monitor_wake_intr(hif_ctx, 0);
	hif_pm_runtime_request_resume(hif_ctx, RTPM_ID_WAKE_INTR_HANDLER);

	if (scn->initial_wakeup_cb)
		scn->initial_wakeup_cb(scn->initial_wakeup_priv);

	if (hif_is_ut_suspended(scn))
		hif_ut_fw_resume(scn);

	qdf_pm_system_wakeup();

	return IRQ_HANDLED;
}

void hif_set_initial_wakeup_cb(struct hif_opaque_softc *hif_ctx,
			       void (*callback)(void *),
			       void *priv)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	scn->initial_wakeup_cb = callback;
	scn->initial_wakeup_priv = priv;
}

void hif_set_ce_service_max_yield_time(struct hif_opaque_softc *hif,
				       uint32_t ce_service_max_yield_time)
{
	struct hif_softc *hif_ctx = HIF_GET_SOFTC(hif);

	hif_ctx->ce_service_max_yield_time =
		ce_service_max_yield_time * 1000;
}

unsigned long long
hif_get_ce_service_max_yield_time(struct hif_opaque_softc *hif)
{
	struct hif_softc *hif_ctx = HIF_GET_SOFTC(hif);

	return hif_ctx->ce_service_max_yield_time;
}

void hif_set_ce_service_max_rx_ind_flush(struct hif_opaque_softc *hif,
				       uint8_t ce_service_max_rx_ind_flush)
{
	struct hif_softc *hif_ctx = HIF_GET_SOFTC(hif);

	if (ce_service_max_rx_ind_flush == 0 ||
	    ce_service_max_rx_ind_flush > MSG_FLUSH_NUM)
		hif_ctx->ce_service_max_rx_ind_flush = MSG_FLUSH_NUM;
	else
		hif_ctx->ce_service_max_rx_ind_flush =
						ce_service_max_rx_ind_flush;
}

#ifdef SYSTEM_PM_CHECK
void __hif_system_pm_set_state(struct hif_opaque_softc *hif,
			       enum hif_system_pm_state state)
{
	struct hif_softc *hif_ctx = HIF_GET_SOFTC(hif);

	qdf_atomic_set(&hif_ctx->sys_pm_state, state);
}

int32_t hif_system_pm_get_state(struct hif_opaque_softc *hif)
{
	struct hif_softc *hif_ctx = HIF_GET_SOFTC(hif);

	return qdf_atomic_read(&hif_ctx->sys_pm_state);
}

int hif_system_pm_state_check(struct hif_opaque_softc *hif)
{
	struct hif_softc *hif_ctx = HIF_GET_SOFTC(hif);
	int32_t sys_pm_state;

	if (!hif_ctx) {
		hif_err("hif context is null");
		return -EFAULT;
	}

	sys_pm_state = qdf_atomic_read(&hif_ctx->sys_pm_state);
	if (sys_pm_state == HIF_SYSTEM_PM_STATE_BUS_SUSPENDING ||
	    sys_pm_state == HIF_SYSTEM_PM_STATE_BUS_SUSPENDED) {
		hif_info("Triggering system wakeup");
		qdf_pm_system_wakeup();
		return -EAGAIN;
	}

	return 0;
}
#endif
