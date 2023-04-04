/*
 * Copyright (c) 2013-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _HIF_H_
#define _HIF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Header files */
#include <qdf_status.h>
#include "qdf_ipa.h"
#include "qdf_nbuf.h"
#include "qdf_lro.h"
#include "ol_if_athvar.h"
#include <linux/platform_device.h>
#ifdef HIF_PCI
#include <linux/pci.h>
#endif /* HIF_PCI */
#ifdef HIF_USB
#include <linux/usb.h>
#endif /* HIF_USB */
#ifdef IPA_OFFLOAD
#include <linux/ipa.h>
#endif
#include "cfg_ucfg_api.h"
#include "qdf_dev.h"
#include <wlan_init_cfg.h>

#define ENABLE_MBOX_DUMMY_SPACE_FEATURE 1

typedef void __iomem *A_target_id_t;
typedef void *hif_handle_t;

#if defined(HIF_IPCI) && defined(FEATURE_HAL_DELAYED_REG_WRITE)
#define HIF_WORK_DRAIN_WAIT_CNT 50

#define HIF_EP_WAKE_RESET_WAIT_CNT 10
#endif

#define HIF_TYPE_AR6002   2
#define HIF_TYPE_AR6003   3
#define HIF_TYPE_AR6004   5
#define HIF_TYPE_AR9888   6
#define HIF_TYPE_AR6320   7
#define HIF_TYPE_AR6320V2 8
/* For attaching Peregrine 2.0 board host_reg_tbl only */
#define HIF_TYPE_AR9888V2 9
#define HIF_TYPE_ADRASTEA 10
#define HIF_TYPE_AR900B 11
#define HIF_TYPE_QCA9984 12
#define HIF_TYPE_IPQ4019 13
#define HIF_TYPE_QCA9888 14
#define HIF_TYPE_QCA8074 15
#define HIF_TYPE_QCA6290 16
#define HIF_TYPE_QCN7605 17
#define HIF_TYPE_QCA6390 18
#define HIF_TYPE_QCA8074V2 19
#define HIF_TYPE_QCA6018  20
#define HIF_TYPE_QCN9000 21
#define HIF_TYPE_QCA6490 22
#define HIF_TYPE_QCA6750 23
#define HIF_TYPE_QCA5018 24
#define HIF_TYPE_QCN6122 25
#define HIF_TYPE_KIWI 26
#define HIF_TYPE_QCN9224 27
#define HIF_TYPE_QCA9574 28

#define DMA_COHERENT_MASK_DEFAULT   37

#ifdef IPA_OFFLOAD
#define DMA_COHERENT_MASK_BELOW_IPA_VER_3       32
#endif

/* enum hif_ic_irq - enum defining integrated chip irq numbers
 * defining irq nubers that can be used by external modules like datapath
 */
enum hif_ic_irq {
	host2wbm_desc_feed = 16,
	host2reo_re_injection,
	host2reo_command,
	host2rxdma_monitor_ring3,
	host2rxdma_monitor_ring2,
	host2rxdma_monitor_ring1,
	reo2host_exception,
	wbm2host_rx_release,
	reo2host_status,
	reo2host_destination_ring4,
	reo2host_destination_ring3,
	reo2host_destination_ring2,
	reo2host_destination_ring1,
	rxdma2host_monitor_destination_mac3,
	rxdma2host_monitor_destination_mac2,
	rxdma2host_monitor_destination_mac1,
	ppdu_end_interrupts_mac3,
	ppdu_end_interrupts_mac2,
	ppdu_end_interrupts_mac1,
	rxdma2host_monitor_status_ring_mac3,
	rxdma2host_monitor_status_ring_mac2,
	rxdma2host_monitor_status_ring_mac1,
	host2rxdma_host_buf_ring_mac3,
	host2rxdma_host_buf_ring_mac2,
	host2rxdma_host_buf_ring_mac1,
	rxdma2host_destination_ring_mac3,
	rxdma2host_destination_ring_mac2,
	rxdma2host_destination_ring_mac1,
	host2tcl_input_ring4,
	host2tcl_input_ring3,
	host2tcl_input_ring2,
	host2tcl_input_ring1,
	wbm2host_tx_completions_ring3,
	wbm2host_tx_completions_ring2,
	wbm2host_tx_completions_ring1,
	tcl2host_status_ring,
};

struct CE_state;
#ifdef QCA_WIFI_QCN9224
#define CE_COUNT_MAX 16
#else
#define CE_COUNT_MAX 12
#endif

#ifndef HIF_MAX_GROUP
#define HIF_MAX_GROUP WLAN_CFG_INT_NUM_CONTEXTS
#endif

#ifdef CONFIG_BERYLLIUM
#define HIF_MAX_GRP_IRQ 25
#else
#define HIF_MAX_GRP_IRQ 16
#endif

#ifndef NAPI_YIELD_BUDGET_BASED
#ifndef QCA_NAPI_DEF_SCALE_BIN_SHIFT
#define QCA_NAPI_DEF_SCALE_BIN_SHIFT   4
#endif
#else  /* NAPI_YIELD_BUDGET_BASED */
#define QCA_NAPI_DEF_SCALE_BIN_SHIFT 2
#endif /* NAPI_YIELD_BUDGET_BASED */

#define QCA_NAPI_BUDGET    64
#define QCA_NAPI_DEF_SCALE  \
	(1 << QCA_NAPI_DEF_SCALE_BIN_SHIFT)

#define HIF_NAPI_MAX_RECEIVES (QCA_NAPI_BUDGET * QCA_NAPI_DEF_SCALE)
/* NOTE: "napi->scale" can be changed,
 * but this does not change the number of buckets
 */
#define QCA_NAPI_NUM_BUCKETS 4

/**
 * qca_napi_stat - stats structure for execution contexts
 * @napi_schedules - number of times the schedule function is called
 * @napi_polls - number of times the execution context runs
 * @napi_completes - number of times that the generating interrupt is reenabled
 * @napi_workdone - cumulative of all work done reported by handler
 * @cpu_corrected - incremented when execution context runs on a different core
 *			than the one that its irq is affined to.
 * @napi_budget_uses - histogram of work done per execution run
 * @time_limit_reache - count of yields due to time limit threshholds
 * @rxpkt_thresh_reached - count of yields due to a work limit
 * @poll_time_buckets - histogram of poll times for the napi
 *
 */
struct qca_napi_stat {
	uint32_t napi_schedules;
	uint32_t napi_polls;
	uint32_t napi_completes;
	uint32_t napi_workdone;
	uint32_t cpu_corrected;
	uint32_t napi_budget_uses[QCA_NAPI_NUM_BUCKETS];
	uint32_t time_limit_reached;
	uint32_t rxpkt_thresh_reached;
	unsigned long long napi_max_poll_time;
#ifdef WLAN_FEATURE_RX_SOFTIRQ_TIME_LIMIT
	uint32_t poll_time_buckets[QCA_NAPI_NUM_BUCKETS];
#endif
};


/**
 * per NAPI instance data structure
 * This data structure holds stuff per NAPI instance.
 * Note that, in the current implementation, though scale is
 * an instance variable, it is set to the same value for all
 * instances.
 */
struct qca_napi_info {
	struct net_device    netdev; /* dummy net_dev */
	void                 *hif_ctx;
	struct napi_struct   napi;
	uint8_t              scale;   /* currently same on all instances */
	uint8_t              id;
	uint8_t              cpu;
	int                  irq;
	cpumask_t            cpumask;
	struct qca_napi_stat stats[NR_CPUS];
#ifdef RECEIVE_OFFLOAD
	/* will only be present for data rx CE's */
	void (*offld_flush_cb)(void *);
	struct napi_struct   rx_thread_napi;
	struct net_device    rx_thread_netdev;
#endif /* RECEIVE_OFFLOAD */
	qdf_lro_ctx_t        lro_ctx;
};

enum qca_napi_tput_state {
	QCA_NAPI_TPUT_UNINITIALIZED,
	QCA_NAPI_TPUT_LO,
	QCA_NAPI_TPUT_HI
};
enum qca_napi_cpu_state {
	QCA_NAPI_CPU_UNINITIALIZED,
	QCA_NAPI_CPU_DOWN,
	QCA_NAPI_CPU_UP };

/**
 * struct qca_napi_cpu - an entry of the napi cpu table
 * @core_id:     physical core id of the core
 * @cluster_id:  cluster this core belongs to
 * @core_mask:   mask to match all core of this cluster
 * @thread_mask: mask for this core within the cluster
 * @max_freq:    maximum clock this core can be clocked at
 *               same for all cpus of the same core.
 * @napis:       bitmap of napi instances on this core
 * @execs:       bitmap of execution contexts on this core
 * cluster_nxt:  chain to link cores within the same cluster
 *
 * This structure represents a single entry in the napi cpu
 * table. The table is part of struct qca_napi_data.
 * This table is initialized by the init function, called while
 * the first napi instance is being created, updated by hotplug
 * notifier and when cpu affinity decisions are made (by throughput
 * detection), and deleted when the last napi instance is removed.
 */
struct qca_napi_cpu {
	enum qca_napi_cpu_state state;
	int			core_id;
	int			cluster_id;
	cpumask_t		core_mask;
	cpumask_t		thread_mask;
	unsigned int		max_freq;
	uint32_t		napis;
	uint32_t		execs;
	int			cluster_nxt;  /* index, not pointer */
};

/**
 * struct qca_napi_data - collection of napi data for a single hif context
 * @hif_softc: pointer to the hif context
 * @lock: spinlock used in the event state machine
 * @state: state variable used in the napi stat machine
 * @ce_map: bit map indicating which ce's have napis running
 * @exec_map: bit map of instanciated exec contexts
 * @user_cpu_affin_map: CPU affinity map from INI config.
 * @napi_cpu: cpu info for irq affinty
 * @lilcl_head:
 * @bigcl_head:
 * @napi_mode: irq affinity & clock voting mode
 * @cpuhp_handler: CPU hotplug event registration handle
 */
struct qca_napi_data {
	struct               hif_softc *hif_softc;
	qdf_spinlock_t       lock;
	uint32_t             state;

	/* bitmap of created/registered NAPI instances, indexed by pipe_id,
	 * not used by clients (clients use an id returned by create)
	 */
	uint32_t             ce_map;
	uint32_t             exec_map;
	uint32_t             user_cpu_affin_mask;
	struct qca_napi_info *napis[CE_COUNT_MAX];
	struct qca_napi_cpu  napi_cpu[NR_CPUS];
	int                  lilcl_head, bigcl_head;
	enum qca_napi_tput_state napi_mode;
	struct qdf_cpuhp_handler *cpuhp_handler;
	uint8_t              flags;
};

/**
 * struct hif_config_info - Place Holder for HIF configuration
 * @enable_self_recovery: Self Recovery
 * @enable_runtime_pm: Enable Runtime PM
 * @runtime_pm_delay: Runtime PM Delay
 * @rx_softirq_max_yield_duration_ns: Max Yield time duration for RX Softirq
 * @enable_ce_dp_irq_affine: Enable affinity for CE DP IRQs
 *
 * Structure for holding HIF ini parameters.
 */
struct hif_config_info {
	bool enable_self_recovery;
#ifdef FEATURE_RUNTIME_PM
	uint8_t enable_runtime_pm;
	u_int32_t runtime_pm_delay;
#endif
	uint64_t rx_softirq_max_yield_duration_ns;
#ifdef FEATURE_ENABLE_CE_DP_IRQ_AFFINE
	bool enable_ce_dp_irq_affine;
#endif
};

/**
 * struct hif_target_info - Target Information
 * @target_version: Target Version
 * @target_type: Target Type
 * @target_revision: Target Revision
 * @soc_version: SOC Version
 * @hw_name: pointer to hardware name
 *
 * Structure to hold target information.
 */
struct hif_target_info {
	uint32_t target_version;
	uint32_t target_type;
	uint32_t target_revision;
	uint32_t soc_version;
	char *hw_name;
};

struct hif_opaque_softc {
};

/**
 * enum hif_event_type - Type of DP events to be recorded
 * @HIF_EVENT_IRQ_TRIGGER: IRQ trigger event
 * @HIF_EVENT_TIMER_ENTRY: Monitor Timer entry event
 * @HIF_EVENT_TIMER_EXIT: Monitor Timer exit event
 * @HIF_EVENT_BH_SCHED: NAPI POLL scheduled event
 * @HIF_EVENT_SRNG_ACCESS_START: hal ring access start event
 * @HIF_EVENT_SRNG_ACCESS_END: hal ring access end event
 * @HIF_EVENT_BH_COMPLETE: NAPI POLL completion event
 * @HIF_EVENT_BH_FORCE_BREAK: NAPI POLL force break event
 */
enum hif_event_type {
	HIF_EVENT_IRQ_TRIGGER,
	HIF_EVENT_TIMER_ENTRY,
	HIF_EVENT_TIMER_EXIT,
	HIF_EVENT_BH_SCHED,
	HIF_EVENT_SRNG_ACCESS_START,
	HIF_EVENT_SRNG_ACCESS_END,
	HIF_EVENT_BH_COMPLETE,
	HIF_EVENT_BH_FORCE_BREAK,
	/* Do check hif_hist_skip_event_record when adding new events */
};

/**
 * enum hif_system_pm_state - System PM state
 * HIF_SYSTEM_PM_STATE_ON: System in active state
 * HIF_SYSTEM_PM_STATE_BUS_RESUMING: bus resume in progress as part of
 *  system resume
 * HIF_SYSTEM_PM_STATE_BUS_SUSPENDING: bus suspend in progress as part of
 *  system suspend
 * HIF_SYSTEM_PM_STATE_BUS_SUSPENDED: bus suspended as part of system suspend
 */
enum hif_system_pm_state {
	HIF_SYSTEM_PM_STATE_ON,
	HIF_SYSTEM_PM_STATE_BUS_RESUMING,
	HIF_SYSTEM_PM_STATE_BUS_SUSPENDING,
	HIF_SYSTEM_PM_STATE_BUS_SUSPENDED,
};

#ifdef WLAN_FEATURE_DP_EVENT_HISTORY
#define HIF_NUM_INT_CONTEXTS		HIF_MAX_GROUP

#if defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF)
/* HIF_EVENT_HIST_MAX should always be power of 2 */
#define HIF_EVENT_HIST_MAX		512

#define HIF_EVENT_HIST_ENABLE_MASK	0xFF

static inline uint64_t hif_get_log_timestamp(void)
{
	return qdf_get_log_timestamp();
}

#else

#define HIF_EVENT_HIST_MAX		32
/* Enable IRQ TRIGGER, NAPI SCHEDULE, SRNG ACCESS START */
#define HIF_EVENT_HIST_ENABLE_MASK	0x19

static inline uint64_t hif_get_log_timestamp(void)
{
	return qdf_sched_clock();
}

#endif

/**
 * struct hif_event_record - an entry of the DP event history
 * @hal_ring_id: ring id for which event is recorded
 * @hp: head pointer of the ring (may not be applicable for all events)
 * @tp: tail pointer of the ring (may not be applicable for all events)
 * @cpu_id: cpu id on which the event occurred
 * @timestamp: timestamp when event occurred
 * @type: type of the event
 *
 * This structure represents the information stored for every datapath
 * event which is logged in the history.
 */
struct hif_event_record {
	uint8_t hal_ring_id;
	uint32_t hp;
	uint32_t tp;
	int cpu_id;
	uint64_t timestamp;
	enum hif_event_type type;
};

/**
 * struct hif_event_misc - history related misc info
 * @last_irq_index: last irq event index in history
 * @last_irq_ts: last irq timestamp
 */
struct hif_event_misc {
	int32_t last_irq_index;
	uint64_t last_irq_ts;
};

/**
 * struct hif_event_history - history for one interrupt group
 * @index: index to store new event
 * @event: event entry
 *
 * This structure represents the datapath history for one
 * interrupt group.
 */
struct hif_event_history {
	qdf_atomic_t index;
	struct hif_event_misc misc;
	struct hif_event_record event[HIF_EVENT_HIST_MAX];
};

/**
 * hif_hist_record_event() - Record one datapath event in history
 * @hif_ctx: HIF opaque context
 * @event: DP event entry
 * @intr_grp_id: interrupt group ID registered with hif
 *
 * Return: None
 */
void hif_hist_record_event(struct hif_opaque_softc *hif_ctx,
			   struct hif_event_record *event,
			   uint8_t intr_grp_id);

/**
 * hif_event_history_init() - Initialize SRNG event history buffers
 * @hif_ctx: HIF opaque context
 * @id: context group ID for which history is recorded
 *
 * Returns: None
 */
void hif_event_history_init(struct hif_opaque_softc *hif_ctx, uint8_t id);

/**
 * hif_event_history_deinit() - De-initialize SRNG event history buffers
 * @hif_ctx: HIF opaque context
 * @id: context group ID for which history is recorded
 *
 * Returns: None
 */
void hif_event_history_deinit(struct hif_opaque_softc *hif_ctx, uint8_t id);

/**
 * hif_record_event() - Wrapper function to form and record DP event
 * @hif_ctx: HIF opaque context
 * @intr_grp_id: interrupt group ID registered with hif
 * @hal_ring_id: ring id for which event is recorded
 * @hp: head pointer index of the srng
 * @tp: tail pointer index of the srng
 * @type: type of the event to be logged in history
 *
 * Return: None
 */
static inline void hif_record_event(struct hif_opaque_softc *hif_ctx,
				    uint8_t intr_grp_id,
				    uint8_t hal_ring_id,
				    uint32_t hp,
				    uint32_t tp,
				    enum hif_event_type type)
{
	struct hif_event_record event;

	event.hal_ring_id = hal_ring_id;
	event.hp = hp;
	event.tp = tp;
	event.type = type;

	hif_hist_record_event(hif_ctx, &event, intr_grp_id);

	return;
}

#else

static inline void hif_record_event(struct hif_opaque_softc *hif_ctx,
				    uint8_t intr_grp_id,
				    uint8_t hal_ring_id,
				    uint32_t hp,
				    uint32_t tp,
				    enum hif_event_type type)
{
}

static inline void hif_event_history_init(struct hif_opaque_softc *hif_ctx,
					  uint8_t id)
{
}

static inline void hif_event_history_deinit(struct hif_opaque_softc *hif_ctx,
					    uint8_t id)
{
}
#endif /* WLAN_FEATURE_DP_EVENT_HISTORY */

void hif_display_ctrl_traffic_pipes_state(struct hif_opaque_softc *hif_ctx);

#if defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF)
void hif_display_latest_desc_hist(struct hif_opaque_softc *hif_ctx);
#else
static inline void hif_display_latest_desc_hist(struct hif_opaque_softc *hif_ctx) {}
#endif

/**
 * enum HIF_DEVICE_POWER_CHANGE_TYPE: Device Power change type
 *
 * @HIF_DEVICE_POWER_UP:   HIF layer should power up interface and/or module
 * @HIF_DEVICE_POWER_DOWN: HIF layer should initiate bus-specific measures to
 *                         minimize power
 * @HIF_DEVICE_POWER_CUT:  HIF layer should initiate bus-specific AND/OR
 *                         platform-specific measures to completely power-off
 *                         the module and associated hardware (i.e. cut power
 *                         supplies)
 */
enum HIF_DEVICE_POWER_CHANGE_TYPE {
	HIF_DEVICE_POWER_UP,
	HIF_DEVICE_POWER_DOWN,
	HIF_DEVICE_POWER_CUT
};

/**
 * enum hif_enable_type: what triggered the enabling of hif
 *
 * @HIF_ENABLE_TYPE_PROBE: probe triggered enable
 * @HIF_ENABLE_TYPE_REINIT: reinit triggered enable
 */
enum hif_enable_type {
	HIF_ENABLE_TYPE_PROBE,
	HIF_ENABLE_TYPE_REINIT,
	HIF_ENABLE_TYPE_MAX
};

/**
 * enum hif_disable_type: what triggered the disabling of hif
 *
 * @HIF_DISABLE_TYPE_PROBE_ERROR: probe error triggered disable
 * @HIF_DISABLE_TYPE_REINIT_ERROR: reinit error triggered disable
 * @HIF_DISABLE_TYPE_REMOVE: remove triggered disable
 * @HIF_DISABLE_TYPE_SHUTDOWN: shutdown triggered disable
 */
enum hif_disable_type {
	HIF_DISABLE_TYPE_PROBE_ERROR,
	HIF_DISABLE_TYPE_REINIT_ERROR,
	HIF_DISABLE_TYPE_REMOVE,
	HIF_DISABLE_TYPE_SHUTDOWN,
	HIF_DISABLE_TYPE_MAX
};
/**
 * enum hif_device_config_opcode: configure mode
 *
 * @HIF_DEVICE_POWER_STATE: device power state
 * @HIF_DEVICE_GET_BLOCK_SIZE: get block size
 * @HIF_DEVICE_GET_ADDR: get block address
 * @HIF_DEVICE_GET_PENDING_EVENTS_FUNC: get pending events functions
 * @HIF_DEVICE_GET_IRQ_PROC_MODE: get irq proc mode
 * @HIF_DEVICE_GET_RECV_EVENT_MASK_UNMASK_FUNC: receive event function
 * @HIF_DEVICE_POWER_STATE_CHANGE: change power state
 * @HIF_DEVICE_GET_IRQ_YIELD_PARAMS: get yield params
 * @HIF_CONFIGURE_QUERY_SCATTER_REQUEST_SUPPORT: configure scatter request
 * @HIF_DEVICE_GET_OS_DEVICE: get OS device
 * @HIF_DEVICE_DEBUG_BUS_STATE: debug bus state
 * @HIF_BMI_DONE: bmi done
 * @HIF_DEVICE_SET_TARGET_TYPE: set target type
 * @HIF_DEVICE_SET_HTC_CONTEXT: set htc context
 * @HIF_DEVICE_GET_HTC_CONTEXT: get htc context
 */
enum hif_device_config_opcode {
	HIF_DEVICE_POWER_STATE = 0,
	HIF_DEVICE_GET_BLOCK_SIZE,
	HIF_DEVICE_GET_FIFO_ADDR,
	HIF_DEVICE_GET_PENDING_EVENTS_FUNC,
	HIF_DEVICE_GET_IRQ_PROC_MODE,
	HIF_DEVICE_GET_RECV_EVENT_MASK_UNMASK_FUNC,
	HIF_DEVICE_POWER_STATE_CHANGE,
	HIF_DEVICE_GET_IRQ_YIELD_PARAMS,
	HIF_CONFIGURE_QUERY_SCATTER_REQUEST_SUPPORT,
	HIF_DEVICE_GET_OS_DEVICE,
	HIF_DEVICE_DEBUG_BUS_STATE,
	HIF_BMI_DONE,
	HIF_DEVICE_SET_TARGET_TYPE,
	HIF_DEVICE_SET_HTC_CONTEXT,
	HIF_DEVICE_GET_HTC_CONTEXT,
};

#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
struct HID_ACCESS_LOG {
	uint32_t seqnum;
	bool is_write;
	void *addr;
	uint32_t value;
};
#endif

void hif_reg_write(struct hif_opaque_softc *hif_ctx, uint32_t offset,
		uint32_t value);
uint32_t hif_reg_read(struct hif_opaque_softc *hif_ctx, uint32_t offset);

#define HIF_MAX_DEVICES                 1
/**
 * struct htc_callbacks - Structure for HTC Callbacks methods
 * @context:             context to pass to the dsrhandler
 *                       note : rwCompletionHandler is provided the context
 *                       passed to hif_read_write
 * @rwCompletionHandler: Read / write completion handler
 * @dsrHandler:          DSR Handler
 */
struct htc_callbacks {
	void *context;
	QDF_STATUS(*rw_compl_handler)(void *rw_ctx, QDF_STATUS status);
	QDF_STATUS(*dsr_handler)(void *context);
};

/**
 * struct hif_driver_state_callbacks - Callbacks for HIF to query Driver state
 * @context: Private data context
 * @set_recovery_in_progress: To Set Driver state for recovery in progress
 * @is_recovery_in_progress: Query if driver state is recovery in progress
 * @is_load_unload_in_progress: Query if driver state Load/Unload in Progress
 * @is_driver_unloading: Query if driver is unloading.
 * @get_bandwidth_level: Query current bandwidth level for the driver
 * @prealloc_get_consistent_mem_unligned: get prealloc unaligned consistent mem
 * @prealloc_put_consistent_mem_unligned: put unaligned consistent mem to pool
 * This Structure provides callback pointer for HIF to query hdd for driver
 * states.
 */
struct hif_driver_state_callbacks {
	void *context;
	void (*set_recovery_in_progress)(void *context, uint8_t val);
	bool (*is_recovery_in_progress)(void *context);
	bool (*is_load_unload_in_progress)(void *context);
	bool (*is_driver_unloading)(void *context);
	bool (*is_target_ready)(void *context);
	int (*get_bandwidth_level)(void *context);
	void *(*prealloc_get_consistent_mem_unaligned)(qdf_size_t size,
						       qdf_dma_addr_t *paddr,
						       uint32_t ring_type);
	void (*prealloc_put_consistent_mem_unaligned)(void *vaddr);
};

/* This API detaches the HTC layer from the HIF device */
void hif_detach_htc(struct hif_opaque_softc *hif_ctx);

/****************************************************************/
/* BMI and Diag window abstraction                              */
/****************************************************************/

#define HIF_BMI_EXCHANGE_NO_TIMEOUT  ((uint32_t)(0))

#define DIAG_TRANSFER_LIMIT 2048U   /* maximum number of bytes that can be
				     * handled atomically by
				     * DiagRead/DiagWrite
				     */

#ifdef WLAN_FEATURE_BMI
/*
 * API to handle HIF-specific BMI message exchanges, this API is synchronous
 * and only allowed to be called from a context that can block (sleep)
 */
QDF_STATUS hif_exchange_bmi_msg(struct hif_opaque_softc *hif_ctx,
				qdf_dma_addr_t cmd, qdf_dma_addr_t rsp,
				uint8_t *pSendMessage, uint32_t Length,
				uint8_t *pResponseMessage,
				uint32_t *pResponseLength, uint32_t TimeoutMS);
void hif_register_bmi_callbacks(struct hif_opaque_softc *hif_ctx);
bool hif_needs_bmi(struct hif_opaque_softc *hif_ctx);
#else /* WLAN_FEATURE_BMI */
static inline void
hif_register_bmi_callbacks(struct hif_opaque_softc *hif_ctx)
{
}

static inline bool
hif_needs_bmi(struct hif_opaque_softc *hif_ctx)
{
	return false;
}
#endif /* WLAN_FEATURE_BMI */

#ifdef HIF_CPU_CLEAR_AFFINITY
/**
 * hif_config_irq_clear_cpu_affinity() - Remove cpu affinity of IRQ
 * @scn: HIF handle
 * @intr_ctxt_id: interrupt group index
 * @cpu: CPU core to clear
 *
 * Return: None
 */
void hif_config_irq_clear_cpu_affinity(struct hif_opaque_softc *scn,
				       int intr_ctxt_id, int cpu);
#else
static inline
void hif_config_irq_clear_cpu_affinity(struct hif_opaque_softc *scn,
				       int intr_ctxt_id, int cpu)
{
}
#endif

/*
 * APIs to handle HIF specific diagnostic read accesses. These APIs are
 * synchronous and only allowed to be called from a context that
 * can block (sleep). They are not high performance APIs.
 *
 * hif_diag_read_access reads a 4 Byte aligned/length value from a
 * Target register or memory word.
 *
 * hif_diag_read_mem reads an arbitrary length of arbitrarily aligned memory.
 */
QDF_STATUS hif_diag_read_access(struct hif_opaque_softc *hif_ctx,
				uint32_t address, uint32_t *data);
QDF_STATUS hif_diag_read_mem(struct hif_opaque_softc *hif_ctx, uint32_t address,
		      uint8_t *data, int nbytes);
void hif_dump_target_memory(struct hif_opaque_softc *hif_ctx,
			void *ramdump_base, uint32_t address, uint32_t size);
/*
 * APIs to handle HIF specific diagnostic write accesses. These APIs are
 * synchronous and only allowed to be called from a context that
 * can block (sleep).
 * They are not high performance APIs.
 *
 * hif_diag_write_access writes a 4 Byte aligned/length value to a
 * Target register or memory word.
 *
 * hif_diag_write_mem writes an arbitrary length of arbitrarily aligned memory.
 */
QDF_STATUS hif_diag_write_access(struct hif_opaque_softc *hif_ctx,
				 uint32_t address, uint32_t data);
QDF_STATUS hif_diag_write_mem(struct hif_opaque_softc *hif_ctx,
			uint32_t address, uint8_t *data, int nbytes);

typedef void (*fastpath_msg_handler)(void *, qdf_nbuf_t *, uint32_t);

void hif_enable_polled_mode(struct hif_opaque_softc *hif_ctx);
bool hif_is_polled_mode_enabled(struct hif_opaque_softc *hif_ctx);

/*
 * Set the FASTPATH_mode_on flag in sc, for use by data path
 */
#ifdef WLAN_FEATURE_FASTPATH
void hif_enable_fastpath(struct hif_opaque_softc *hif_ctx);
bool hif_is_fastpath_mode_enabled(struct hif_opaque_softc *hif_ctx);
void *hif_get_ce_handle(struct hif_opaque_softc *hif_ctx, int ret);

/**
 * hif_ce_fastpath_cb_register() - Register callback for fastpath msg handler
 * @handler: Callback funtcion
 * @context: handle for callback function
 *
 * Return: QDF_STATUS_SUCCESS on success or QDF_STATUS_E_FAILURE
 */
QDF_STATUS hif_ce_fastpath_cb_register(
		struct hif_opaque_softc *hif_ctx,
		fastpath_msg_handler handler, void *context);
#else
static inline QDF_STATUS hif_ce_fastpath_cb_register(
		struct hif_opaque_softc *hif_ctx,
		fastpath_msg_handler handler, void *context)
{
	return QDF_STATUS_E_FAILURE;
}

static inline void *hif_get_ce_handle(struct hif_opaque_softc *hif_ctx, int ret)
{
	return NULL;
}

#endif

/*
 * Enable/disable CDC max performance workaround
 * For max-performace set this to 0
 * To allow SoC to enter sleep set this to 1
 */
#define CONFIG_DISABLE_CDC_MAX_PERF_WAR 0

void hif_ipa_get_ce_resource(struct hif_opaque_softc *hif_ctx,
			     qdf_shared_mem_t **ce_sr,
			     uint32_t *ce_sr_ring_size,
			     qdf_dma_addr_t *ce_reg_paddr);

/**
 * @brief List of callbacks - filled in by HTC.
 */
struct hif_msg_callbacks {
	void *Context;
	/**< context meaningful to HTC */
	QDF_STATUS (*txCompletionHandler)(void *Context, qdf_nbuf_t wbuf,
					uint32_t transferID,
					uint32_t toeplitz_hash_result);
	QDF_STATUS (*rxCompletionHandler)(void *Context, qdf_nbuf_t wbuf,
					uint8_t pipeID);
	void (*txResourceAvailHandler)(void *context, uint8_t pipe);
	void (*fwEventHandler)(void *context, QDF_STATUS status);
	void (*update_bundle_stats)(void *context, uint8_t no_of_pkt_in_bundle);
};

enum hif_target_status {
	TARGET_STATUS_CONNECTED = 0,  /* target connected */
	TARGET_STATUS_RESET,  /* target got reset */
	TARGET_STATUS_EJECT,  /* target got ejected */
	TARGET_STATUS_SUSPEND /*target got suspend */
};

/**
 * enum hif_attribute_flags: configure hif
 *
 * @HIF_LOWDESC_CE_CFG: Configure HIF with Low descriptor CE
 * @HIF_LOWDESC_CE_NO_PKTLOG_CFG: Configure HIF with Low descriptor
 *  							+ No pktlog CE
 */
enum hif_attribute_flags {
	HIF_LOWDESC_CE_CFG = 1,
	HIF_LOWDESC_CE_NO_PKTLOG_CFG
};

#define HIF_DATA_ATTR_SET_TX_CLASSIFY(attr, v) \
	(attr |= (v & 0x01) << 5)
#define HIF_DATA_ATTR_SET_ENCAPSULATION_TYPE(attr, v) \
	(attr |= (v & 0x03) << 6)
#define HIF_DATA_ATTR_SET_ADDR_X_SEARCH_DISABLE(attr, v) \
	(attr |= (v & 0x01) << 13)
#define HIF_DATA_ATTR_SET_ADDR_Y_SEARCH_DISABLE(attr, v) \
	(attr |= (v & 0x01) << 14)
#define HIF_DATA_ATTR_SET_TOEPLITZ_HASH_ENABLE(attr, v) \
	(attr |= (v & 0x01) << 15)
#define HIF_DATA_ATTR_SET_PACKET_OR_RESULT_OFFSET(attr, v) \
	(attr |= (v & 0x0FFF) << 16)
#define HIF_DATA_ATTR_SET_ENABLE_11H(attr, v) \
	(attr |= (v & 0x01) << 30)

struct hif_ul_pipe_info {
	unsigned int nentries;
	unsigned int nentries_mask;
	unsigned int sw_index;
	unsigned int write_index; /* cached copy */
	unsigned int hw_index;    /* cached copy */
	void *base_addr_owner_space; /* Host address space */
	qdf_dma_addr_t base_addr_CE_space; /* CE address space */
};

struct hif_dl_pipe_info {
	unsigned int nentries;
	unsigned int nentries_mask;
	unsigned int sw_index;
	unsigned int write_index; /* cached copy */
	unsigned int hw_index;    /* cached copy */
	void *base_addr_owner_space; /* Host address space */
	qdf_dma_addr_t base_addr_CE_space; /* CE address space */
};

struct hif_pipe_addl_info {
	uint32_t pci_mem;
	uint32_t ctrl_addr;
	struct hif_ul_pipe_info ul_pipe;
	struct hif_dl_pipe_info dl_pipe;
};

#ifdef CONFIG_SLUB_DEBUG_ON
#define MSG_FLUSH_NUM 16
#else /* PERF build */
#define MSG_FLUSH_NUM 32
#endif /* SLUB_DEBUG_ON */

struct hif_bus_id;

void hif_claim_device(struct hif_opaque_softc *hif_ctx);
QDF_STATUS hif_get_config_item(struct hif_opaque_softc *hif_ctx,
		     int opcode, void *config, uint32_t config_len);
void hif_set_mailbox_swap(struct hif_opaque_softc *hif_ctx);
void hif_mask_interrupt_call(struct hif_opaque_softc *hif_ctx);
void hif_post_init(struct hif_opaque_softc *hif_ctx, void *hHTC,
		   struct hif_msg_callbacks *callbacks);
QDF_STATUS hif_start(struct hif_opaque_softc *hif_ctx);
void hif_stop(struct hif_opaque_softc *hif_ctx);
void hif_flush_surprise_remove(struct hif_opaque_softc *hif_ctx);
void hif_dump(struct hif_opaque_softc *hif_ctx, uint8_t CmdId, bool start);
void hif_trigger_dump(struct hif_opaque_softc *hif_ctx,
		      uint8_t cmd_id, bool start);

QDF_STATUS hif_send_head(struct hif_opaque_softc *hif_ctx, uint8_t PipeID,
				  uint32_t transferID, uint32_t nbytes,
				  qdf_nbuf_t wbuf, uint32_t data_attr);
void hif_send_complete_check(struct hif_opaque_softc *hif_ctx, uint8_t PipeID,
			     int force);
void hif_shut_down_device(struct hif_opaque_softc *hif_ctx);
void hif_get_default_pipe(struct hif_opaque_softc *hif_ctx, uint8_t *ULPipe,
			  uint8_t *DLPipe);
int hif_map_service_to_pipe(struct hif_opaque_softc *hif_ctx, uint16_t svc_id,
			uint8_t *ul_pipe, uint8_t *dl_pipe, int *ul_is_polled,
			int *dl_is_polled);
uint16_t
hif_get_free_queue_number(struct hif_opaque_softc *hif_ctx, uint8_t PipeID);
void *hif_get_targetdef(struct hif_opaque_softc *hif_ctx);
uint32_t hif_hia_item_address(uint32_t target_type, uint32_t item_offset);
void hif_set_target_sleep(struct hif_opaque_softc *hif_ctx, bool sleep_ok,
		     bool wait_for_it);
int hif_check_fw_reg(struct hif_opaque_softc *hif_ctx);
#ifndef HIF_PCI
static inline int hif_check_soc_status(struct hif_opaque_softc *hif_ctx)
{
	return 0;
}
#else
int hif_check_soc_status(struct hif_opaque_softc *hif_ctx);
#endif
void hif_get_hw_info(struct hif_opaque_softc *hif_ctx, u32 *version,
			u32 *revision, const char **target_name);

#ifdef RECEIVE_OFFLOAD
/**
 * hif_offld_flush_cb_register() - Register the offld flush callback
 * @scn: HIF opaque context
 * @offld_flush_handler: Flush callback is either ol_flush, incase of rx_thread
 *			 Or GRO/LRO flush when RxThread is not enabled. Called
 *			 with corresponding context for flush.
 * Return: None
 */
void hif_offld_flush_cb_register(struct hif_opaque_softc *scn,
				 void (offld_flush_handler)(void *ol_ctx));

/**
 * hif_offld_flush_cb_deregister() - deRegister the offld flush callback
 * @scn: HIF opaque context
 *
 * Return: None
 */
void hif_offld_flush_cb_deregister(struct hif_opaque_softc *scn);
#endif

#ifdef WLAN_FEATURE_RX_SOFTIRQ_TIME_LIMIT
/**
 * hif_exec_should_yield() - Check if hif napi context should yield
 * @hif_ctx - HIF opaque context
 * @grp_id - grp_id of the napi for which check needs to be done
 *
 * The function uses grp_id to look for NAPI and checks if NAPI needs to
 * yield. HIF_EXT_GROUP_MAX_YIELD_DURATION_NS is the duration used for
 * yield decision.
 *
 * Return: true if NAPI needs to yield, else false
 */
bool hif_exec_should_yield(struct hif_opaque_softc *hif_ctx, uint grp_id);
#else
static inline bool hif_exec_should_yield(struct hif_opaque_softc *hif_ctx,
					 uint grp_id)
{
	return false;
}
#endif

void hif_disable_isr(struct hif_opaque_softc *hif_ctx);
void hif_reset_soc(struct hif_opaque_softc *hif_ctx);
void hif_save_htc_htt_config_endpoint(struct hif_opaque_softc *hif_ctx,
				      int htc_htt_tx_endpoint);

/**
 * hif_open() - Create hif handle
 * @qdf_ctx: qdf context
 * @mode: Driver Mode
 * @bus_type: Bus Type
 * @cbk: CDS Callbacks
 * @psoc: psoc object manager
 *
 * API to open HIF Context
 *
 * Return: HIF Opaque Pointer
 */
struct hif_opaque_softc *hif_open(qdf_device_t qdf_ctx,
				  uint32_t mode,
				  enum qdf_bus_type bus_type,
				  struct hif_driver_state_callbacks *cbk,
				  struct wlan_objmgr_psoc *psoc);

/**
 * hif_init_dma_mask() - Set dma mask for the dev
 * @dev: dev for which DMA mask is to be set
 * @bus_type: bus type for the target
 *
 * This API sets the DMA mask for the device. before the datapath
 * memory pre-allocation is done. If the DMA mask is not set before
 * requesting the DMA memory, kernel defaults to a 32-bit DMA mask,
 * and does not utilize the full device capability.
 *
 * Return: 0 - success, non-zero on failure.
 */
int hif_init_dma_mask(struct device *dev, enum qdf_bus_type bus_type);
void hif_close(struct hif_opaque_softc *hif_ctx);
QDF_STATUS hif_enable(struct hif_opaque_softc *hif_ctx, struct device *dev,
		      void *bdev, const struct hif_bus_id *bid,
		      enum qdf_bus_type bus_type,
		      enum hif_enable_type type);
void hif_disable(struct hif_opaque_softc *hif_ctx, enum hif_disable_type type);
#ifdef CE_TASKLET_DEBUG_ENABLE
void hif_enable_ce_latency_stats(struct hif_opaque_softc *hif_ctx,
				 uint8_t value);
#endif
void hif_display_stats(struct hif_opaque_softc *hif_ctx);
void hif_clear_stats(struct hif_opaque_softc *hif_ctx);

/**
 * enum hif_pm_wake_irq_type - Wake interrupt type for Power Management
 * HIF_PM_INVALID_WAKE: Wake irq is invalid or not configured
 * HIF_PM_MSI_WAKE: Wake irq is MSI interrupt
 * HIF_PM_CE_WAKE: Wake irq is CE interrupt
 */
typedef enum {
	HIF_PM_INVALID_WAKE,
	HIF_PM_MSI_WAKE,
	HIF_PM_CE_WAKE,
} hif_pm_wake_irq_type;

/**
 * hif_pm_get_wake_irq_type - Get wake irq type for Power Management
 * @hif_ctx: HIF context
 *
 * Return: enum hif_pm_wake_irq_type
 */
hif_pm_wake_irq_type hif_pm_get_wake_irq_type(struct hif_opaque_softc *hif_ctx);

/**
 * enum wlan_rtpm_dbgid - runtime pm put/get debug id
 * @RTPM_ID_RESVERD:       Reserved
 * @RTPM_ID_WMI:           WMI sending msg, expect put happen at
 *                         tx completion from CE level directly.
 * @RTPM_ID_HTC:           pkt sending by HTT_DATA_MSG_SVC, expect
 *                         put from fw response or just in
 *                         htc_issue_packets
 * @RTPM_ID_QOS_NOTIFY:    pm qos notifer
 * @RTPM_ID_DP_TX_DESC_ALLOC_FREE:      tx desc alloc/free
 * @RTPM_ID_CE_SEND_FAST:  operation in ce_send_fast, not include
 *                         the pkt put happens outside this function
 * @RTPM_ID_SUSPEND_RESUME:     suspend/resume in hdd
 * @RTPM_ID_DW_TX_HW_ENQUEUE:   operation in functin dp_tx_hw_enqueue
 * @RTPM_ID_HAL_REO_CMD:        HAL_REO_CMD operation
 * @RTPM_ID_DP_PRINT_RING_STATS:  operation in dp_print_ring_stats
 * @RTPM_ID_PM_STOP:        operation in hif_pm_runtime_stop
 * @RTPM_ID_CONN_DISCONNECT:operation when issue disconnect
 * @RTPM_ID_SOC_REMOVE: operation in soc remove
 * @RTPM_ID_DRIVER_UNLOAD: operation in driver unload
 * @RTPM_ID_CE_INTR_HANDLER: operation from ce interrupt handler
 * @RTPM_ID_WAKE_INTR_HANDLER: operation from wake interrupt handler
 * @RTPM_ID_SOC_IDLE_SHUTDOWN: operation in soc idle shutdown
 * @RTPM_ID_HIF_FORCE_WAKE: operation in hif force wake
 */
/* New value added to the enum must also be reflected in function
 *  rtpm_string_from_dbgid()
 */
typedef enum {
	RTPM_ID_RESVERD   = 0,
	RTPM_ID_WMI,
	RTPM_ID_HTC,
	RTPM_ID_QOS_NOTIFY,
	RTPM_ID_DP_TX_DESC_ALLOC_FREE,
	RTPM_ID_CE_SEND_FAST,
	RTPM_ID_SUSPEND_RESUME,
	RTPM_ID_DW_TX_HW_ENQUEUE,
	RTPM_ID_HAL_REO_CMD,
	RTPM_ID_DP_PRINT_RING_STATS,
	RTPM_ID_PM_STOP,
	RTPM_ID_CONN_DISCONNECT,
	RTPM_ID_SOC_REMOVE,
	RTPM_ID_DRIVER_UNLOAD,
	RTPM_ID_CE_INTR_HANDLER,
	RTPM_ID_WAKE_INTR_HANDLER,
	RTPM_ID_SOC_IDLE_SHUTDOWN,
	RTPM_ID_HIF_FORCE_WAKE,

	RTPM_ID_MAX,
} wlan_rtpm_dbgid;

/**
 * rtpm_string_from_dbgid() - Convert dbgid to respective string
 * @id -  debug id
 *
 * Debug support function to convert  dbgid to string.
 * Please note to add new string in the array at index equal to
 * its enum value in wlan_rtpm_dbgid.
 */
static inline char *rtpm_string_from_dbgid(wlan_rtpm_dbgid id)
{
	static const char *strings[] = { "RTPM_ID_RESVERD",
					"RTPM_ID_WMI",
					"RTPM_ID_HTC",
					"RTPM_ID_QOS_NOTIFY",
					"RTPM_ID_DP_TX_DESC_ALLOC_FREE",
					"RTPM_ID_CE_SEND_FAST",
					"RTPM_ID_SUSPEND_RESUME",
					"RTPM_ID_DW_TX_HW_ENQUEUE",
					"RTPM_ID_HAL_REO_CMD",
					"RTPM_ID_DP_PRINT_RING_STATS",
					"RTPM_ID_PM_STOP",
					"RTPM_ID_CONN_DISCONNECT",
					"RTPM_ID_SOC_REMOVE",
					"RTPM_ID_DRIVER_UNLOAD",
					"RTPM_ID_CE_INTR_HANDLER",
					"RTPM_ID_WAKE_INTR_HANDLER",
					"RTPM_ID_SOC_IDLE_SHUTDOWN",
					"RTPM_ID_MAX"};

	return (char *)strings[id];
}

/**
 * enum hif_ep_vote_type - hif ep vote type
 * HIF_EP_VOTE_DP_ACCESS: vote type is specific DP
 * HIF_EP_VOTE_NONDP_ACCESS: ep vote for over all access
 */
enum hif_ep_vote_type {
	HIF_EP_VOTE_DP_ACCESS,
	HIF_EP_VOTE_NONDP_ACCESS
};

/**
 * enum hif_ep_vote_access - hif ep vote access
 * HIF_EP_VOTE_ACCESS_ENABLE: Enable ep voting
 * HIF_EP_VOTE_INTERMEDIATE_ACCESS: allow during transistion
 * HIF_EP_VOTE_ACCESS_DISABLE: disable ep voting
 */
enum hif_ep_vote_access {
	HIF_EP_VOTE_ACCESS_ENABLE,
	HIF_EP_VOTE_INTERMEDIATE_ACCESS,
	HIF_EP_VOTE_ACCESS_DISABLE
};

/**
 * enum hif_pm_link_state - hif link state
 * HIF_PM_LINK_STATE_DOWN: hif link state is down
 * HIF_PM_LINK_STATE_UP: hif link state is up
 */
enum hif_pm_link_state {
	HIF_PM_LINK_STATE_DOWN,
	HIF_PM_LINK_STATE_UP
};

/**
 * enum hif_pm_htc_stats - hif runtime PM stats for HTC layer
 * HIF_PM_HTC_STATS_GET_HTT_RESPONSE: PM stats for RTPM GET for HTT packets
				      with response
 * HIF_PM_HTC_STATS_GET_HTT_NO_RESPONSE: PM stats for RTPM GET for HTT packets
					 with no response
 * HIF_PM_HTC_STATS_PUT_HTT_RESPONSE: PM stats for RTPM PUT for HTT packets
				      with response
 * HIF_PM_HTC_STATS_PUT_HTT_NO_RESPONSE: PM stats for RTPM PUT for HTT packets
					 with no response
 * HIF_PM_HTC_STATS_PUT_HTT_ERROR: PM stats for RTPM PUT for failed HTT packets
 * HIF_PM_HTC_STATS_PUT_HTC_CLEANUP: PM stats for RTPM PUT during HTC cleanup
 * HIF_PM_HTC_STATS_GET_HTC_KICK_QUEUES: PM stats for RTPM GET done during
 *                                       htc_kick_queues()
 * HIF_PM_HTC_STATS_PUT_HTC_KICK_QUEUES: PM stats for RTPM PUT done during
 *                                       htc_kick_queues()
 * HIF_PM_HTC_STATS_GET_HTT_FETCH_PKTS: PM stats for RTPM GET while fetching
 *                                      HTT packets from endpoint TX queue
 * HIF_PM_HTC_STATS_PUT_HTT_FETCH_PKTS: PM stats for RTPM PUT while fetching
 *                                      HTT packets from endpoint TX queue
 */
enum hif_pm_htc_stats {
	HIF_PM_HTC_STATS_GET_HTT_RESPONSE,
	HIF_PM_HTC_STATS_GET_HTT_NO_RESPONSE,
	HIF_PM_HTC_STATS_PUT_HTT_RESPONSE,
	HIF_PM_HTC_STATS_PUT_HTT_NO_RESPONSE,
	HIF_PM_HTC_STATS_PUT_HTT_ERROR,
	HIF_PM_HTC_STATS_PUT_HTC_CLEANUP,
	HIF_PM_HTC_STATS_GET_HTC_KICK_QUEUES,
	HIF_PM_HTC_STATS_PUT_HTC_KICK_QUEUES,
	HIF_PM_HTC_STATS_GET_HTT_FETCH_PKTS,
	HIF_PM_HTC_STATS_PUT_HTT_FETCH_PKTS,
};

#ifdef FEATURE_RUNTIME_PM
struct hif_pm_runtime_lock;

void hif_fastpath_resume(struct hif_opaque_softc *hif_ctx);
int hif_pm_runtime_get_sync(struct hif_opaque_softc *hif_ctx,
			    wlan_rtpm_dbgid rtpm_dbgid);
int hif_pm_runtime_put_sync_suspend(struct hif_opaque_softc *hif_ctx,
				    wlan_rtpm_dbgid rtpm_dbgid);
int hif_pm_runtime_request_resume(struct hif_opaque_softc *hif_ctx,
				  wlan_rtpm_dbgid rtpm_dbgid);
int hif_pm_runtime_get(struct hif_opaque_softc *hif_ctx,
		       wlan_rtpm_dbgid rtpm_dbgid,
		       bool is_critical_ctx);
void hif_pm_runtime_get_noresume(struct hif_opaque_softc *hif_ctx,
				 wlan_rtpm_dbgid rtpm_dbgid);
int hif_pm_runtime_put(struct hif_opaque_softc *hif_ctx,
		       wlan_rtpm_dbgid rtpm_dbgid);
int hif_pm_runtime_put_noidle(struct hif_opaque_softc *hif_ctx,
			      wlan_rtpm_dbgid rtpm_dbgid);
void hif_pm_runtime_mark_last_busy(struct hif_opaque_softc *hif_ctx);
int hif_runtime_lock_init(qdf_runtime_lock_t *lock, const char *name);
void hif_runtime_lock_deinit(struct hif_opaque_softc *hif_ctx,
			struct hif_pm_runtime_lock *lock);
int hif_pm_runtime_prevent_suspend(struct hif_opaque_softc *ol_sc,
		struct hif_pm_runtime_lock *lock);
int hif_pm_runtime_allow_suspend(struct hif_opaque_softc *ol_sc,
		struct hif_pm_runtime_lock *lock);
bool hif_pm_runtime_is_suspended(struct hif_opaque_softc *hif_ctx);
void hif_pm_runtime_suspend_lock(struct hif_opaque_softc *hif_ctx);
void hif_pm_runtime_suspend_unlock(struct hif_opaque_softc *hif_ctx);
int hif_pm_runtime_get_monitor_wake_intr(struct hif_opaque_softc *hif_ctx);
void hif_pm_runtime_set_monitor_wake_intr(struct hif_opaque_softc *hif_ctx,
					  int val);
void hif_pm_runtime_check_and_request_resume(struct hif_opaque_softc *hif_ctx);
void hif_pm_runtime_mark_dp_rx_busy(struct hif_opaque_softc *hif_ctx);
int hif_pm_runtime_is_dp_rx_busy(struct hif_opaque_softc *hif_ctx);
qdf_time_t hif_pm_runtime_get_dp_rx_busy_mark(struct hif_opaque_softc *hif_ctx);
int hif_pm_runtime_sync_resume(struct hif_opaque_softc *hif_ctx,
			       wlan_rtpm_dbgid rtpm_dbgid);
void hif_pm_runtime_update_stats(struct hif_opaque_softc *hif_ctx,
				 wlan_rtpm_dbgid rtpm_dbgid,
				 enum hif_pm_htc_stats stats);

/**
 * hif_pm_set_link_state() - set link state during RTPM
 * @hif_sc: HIF Context
 *
 * Return: None
 */
void hif_pm_set_link_state(struct hif_opaque_softc *hif_handle, uint8_t val);

/**
 * hif_is_link_state_up() - Is link state up
 * @hif_sc: HIF Context
 *
 * Return: 1 link is up, 0 link is down
 */
uint8_t hif_pm_get_link_state(struct hif_opaque_softc *hif_handle);

/**
 * hif_pm_runtime_set_delay() - Set delay to trigger RTPM suspend
 * @hif_sc: HIF Context
 * @delay: delay in ms to be set
 *
 * Return: Success if delay is set successfully
 */
QDF_STATUS hif_pm_runtime_set_delay(struct hif_opaque_softc *hif_ctx,
				    int delay);

/**
 * hif_pm_runtime_restore_delay() - Restore delay value to default value
 *
 * Return: Success if reset done. E_ALREADY if delay same as config value
 */
QDF_STATUS hif_pm_runtime_restore_delay(struct hif_opaque_softc *hif_ctx);

/**
 * hif_pm_runtime_get_delay() -Get delay to trigger RTPM suspend
 *
 * Return: Delay in ms
 */
int hif_pm_runtime_get_delay(struct hif_opaque_softc *hif_ctx);

#else
struct hif_pm_runtime_lock {
	const char *name;
};
static inline void hif_fastpath_resume(struct hif_opaque_softc *hif_ctx) {}
static inline int
hif_pm_runtime_get_sync(struct hif_opaque_softc *hif_ctx,
			wlan_rtpm_dbgid rtpm_dbgid)
{ return 0; }
static inline int
hif_pm_runtime_put_sync_suspend(struct hif_opaque_softc *hif_ctx,
				wlan_rtpm_dbgid rtpm_dbgid)
{ return 0; }
static inline int
hif_pm_runtime_request_resume(struct hif_opaque_softc *hif_ctx,
			      wlan_rtpm_dbgid rtpm_dbgid)
{ return 0; }
static inline void
hif_pm_runtime_get_noresume(struct hif_opaque_softc *hif_ctx,
			    wlan_rtpm_dbgid rtpm_dbgid)
{}

static inline int
hif_pm_runtime_get(struct hif_opaque_softc *hif_ctx, wlan_rtpm_dbgid rtpm_dbgid,
		   bool is_critical_ctx)
{ return 0; }
static inline int
hif_pm_runtime_put(struct hif_opaque_softc *hif_ctx, wlan_rtpm_dbgid rtpm_dbgid)
{ return 0; }
static inline int
hif_pm_runtime_put_noidle(struct hif_opaque_softc *hif_ctx,
			  wlan_rtpm_dbgid rtpm_dbgid)
{ return 0; }
static inline void
hif_pm_runtime_mark_last_busy(struct hif_opaque_softc *hif_ctx) {};
static inline int hif_runtime_lock_init(qdf_runtime_lock_t *lock,
					const char *name)
{ return 0; }
static inline void
hif_runtime_lock_deinit(struct hif_opaque_softc *hif_ctx,
			struct hif_pm_runtime_lock *lock) {}

static inline int hif_pm_runtime_prevent_suspend(struct hif_opaque_softc *ol_sc,
		struct hif_pm_runtime_lock *lock)
{ return 0; }
static inline int hif_pm_runtime_allow_suspend(struct hif_opaque_softc *ol_sc,
		struct hif_pm_runtime_lock *lock)
{ return 0; }
static inline bool hif_pm_runtime_is_suspended(struct hif_opaque_softc *hif_ctx)
{ return false; }
static inline void
hif_pm_runtime_suspend_lock(struct hif_opaque_softc *hif_ctx)
{ return; }
static inline void
hif_pm_runtime_suspend_unlock(struct hif_opaque_softc *hif_ctx)
{ return; }
static inline int
hif_pm_runtime_get_monitor_wake_intr(struct hif_opaque_softc *hif_ctx)
{ return 0; }
static inline void
hif_pm_runtime_set_monitor_wake_intr(struct hif_opaque_softc *hif_ctx, int val)
{ return; }
static inline void
hif_pm_runtime_check_and_request_resume(struct hif_opaque_softc *hif_ctx)
{ return; }
static inline void
hif_pm_runtime_mark_dp_rx_busy(struct hif_opaque_softc *hif_ctx) {};
static inline int
hif_pm_runtime_is_dp_rx_busy(struct hif_opaque_softc *hif_ctx)
{ return 0; }
static inline qdf_time_t
hif_pm_runtime_get_dp_rx_busy_mark(struct hif_opaque_softc *hif_ctx)
{ return 0; }
static inline int hif_pm_runtime_sync_resume(struct hif_opaque_softc *hif_ctx,
					     wlan_rtpm_dbgid rtpm_dbgid)
{ return 0; }
static inline
void hif_pm_set_link_state(struct hif_opaque_softc *hif_handle, uint8_t val)
{}

static inline
void hif_pm_runtime_update_stats(struct hif_opaque_softc *hif_ctx,
				 wlan_rtpm_dbgid rtpm_dbgid,
				 enum hif_pm_htc_stats stats)
{}

static inline
QDF_STATUS hif_pm_runtime_set_delay(struct hif_opaque_softc *hif_ctx, int delay)
{ return QDF_STATUS_SUCCESS; }

static inline
QDF_STATUS hif_pm_runtime_restore_delay(struct hif_opaque_softc *hif_ctx)
{ return QDF_STATUS_SUCCESS; }

static inline
int hif_pm_runtime_get_delay(struct hif_opaque_softc *hif_ctx)
{ return 0; }
#endif

void hif_enable_power_management(struct hif_opaque_softc *hif_ctx,
				 bool is_packet_log_enabled);
void hif_disable_power_management(struct hif_opaque_softc *hif_ctx);

void hif_vote_link_down(struct hif_opaque_softc *hif_ctx);
void hif_vote_link_up(struct hif_opaque_softc *hif_ctx);
bool hif_can_suspend_link(struct hif_opaque_softc *hif_ctx);

#ifdef IPA_OFFLOAD
/**
 * hif_get_ipa_hw_type() - get IPA hw type
 *
 * This API return the IPA hw type.
 *
 * Return: IPA hw type
 */
static inline
enum ipa_hw_type hif_get_ipa_hw_type(void)
{
	return ipa_get_hw_type();
}

/**
 * hif_get_ipa_present() - get IPA hw status
 *
 * This API return the IPA hw status.
 *
 * Return: true if IPA is present or false otherwise
 */
static inline
bool hif_get_ipa_present(void)
{
	if (qdf_ipa_uc_reg_rdyCB(NULL) != -EPERM)
		return true;
	else
		return false;
}
#endif
int hif_bus_resume(struct hif_opaque_softc *hif_ctx);
/**
 * hif_bus_ealry_suspend() - stop non wmi tx traffic
 * @context: hif context
 */
int hif_bus_early_suspend(struct hif_opaque_softc *hif_ctx);

/**
 * hif_bus_late_resume() - resume non wmi traffic
 * @context: hif context
 */
int hif_bus_late_resume(struct hif_opaque_softc *hif_ctx);
int hif_bus_suspend(struct hif_opaque_softc *hif_ctx);
int hif_bus_resume_noirq(struct hif_opaque_softc *hif_ctx);
int hif_bus_suspend_noirq(struct hif_opaque_softc *hif_ctx);

/**
 * hif_apps_irqs_enable() - Enables all irqs from the APPS side
 * @hif_ctx: an opaque HIF handle to use
 *
 * As opposed to the standard hif_irq_enable, this function always applies to
 * the APPS side kernel interrupt handling.
 *
 * Return: errno
 */
int hif_apps_irqs_enable(struct hif_opaque_softc *hif_ctx);

/**
 * hif_apps_irqs_disable() - Disables all irqs from the APPS side
 * @hif_ctx: an opaque HIF handle to use
 *
 * As opposed to the standard hif_irq_disable, this function always applies to
 * the APPS side kernel interrupt handling.
 *
 * Return: errno
 */
int hif_apps_irqs_disable(struct hif_opaque_softc *hif_ctx);

/**
 * hif_apps_wake_irq_enable() - Enables the wake irq from the APPS side
 * @hif_ctx: an opaque HIF handle to use
 *
 * As opposed to the standard hif_irq_enable, this function always applies to
 * the APPS side kernel interrupt handling.
 *
 * Return: errno
 */
int hif_apps_wake_irq_enable(struct hif_opaque_softc *hif_ctx);

/**
 * hif_apps_wake_irq_disable() - Disables the wake irq from the APPS side
 * @hif_ctx: an opaque HIF handle to use
 *
 * As opposed to the standard hif_irq_disable, this function always applies to
 * the APPS side kernel interrupt handling.
 *
 * Return: errno
 */
int hif_apps_wake_irq_disable(struct hif_opaque_softc *hif_ctx);

/**
 * hif_apps_enable_irq_wake() - Enables the irq wake from the APPS side
 * @hif_ctx: an opaque HIF handle to use
 *
 * This function always applies to the APPS side kernel interrupt handling
 * to wake the system from suspend.
 *
 * Return: errno
 */
int hif_apps_enable_irq_wake(struct hif_opaque_softc *hif_ctx);

/**
 * hif_apps_disable_irq_wake() - Disables the wake irq from the APPS side
 * @hif_ctx: an opaque HIF handle to use
 *
 * This function always applies to the APPS side kernel interrupt handling
 * to disable the wake irq.
 *
 * Return: errno
 */
int hif_apps_disable_irq_wake(struct hif_opaque_softc *hif_ctx);

/**
 * hif_apps_enable_irqs_except_wake_irq() - Enables all irqs except wake_irq
 * @hif_ctx: an opaque HIF handle to use
 *
 * As opposed to the standard hif_irq_enable, this function always applies to
 * the APPS side kernel interrupt handling.
 *
 * Return: errno
 */
int hif_apps_enable_irqs_except_wake_irq(struct hif_opaque_softc *hif_ctx);

/**
 * hif_apps_disable_irqs_except_wake_irq() - Disables all irqs except wake_irq
 * @hif_ctx: an opaque HIF handle to use
 *
 * As opposed to the standard hif_irq_disable, this function always applies to
 * the APPS side kernel interrupt handling.
 *
 * Return: errno
 */
int hif_apps_disable_irqs_except_wake_irq(struct hif_opaque_softc *hif_ctx);

#ifdef FEATURE_RUNTIME_PM
void hif_print_runtime_pm_prevent_list(struct hif_opaque_softc *hif_ctx);
int hif_pre_runtime_suspend(struct hif_opaque_softc *hif_ctx);
void hif_pre_runtime_resume(struct hif_opaque_softc *hif_ctx);
int hif_runtime_suspend(struct hif_opaque_softc *hif_ctx);
int hif_runtime_resume(struct hif_opaque_softc *hif_ctx);
void hif_process_runtime_suspend_success(struct hif_opaque_softc *hif_ctx);
void hif_process_runtime_suspend_failure(struct hif_opaque_softc *hif_ctx);
void hif_process_runtime_resume_success(struct hif_opaque_softc *hif_ctx);
#else
static inline void
hif_print_runtime_pm_prevent_list(struct hif_opaque_softc *hif_ctx)
{}
#endif

int hif_get_irq_num(struct hif_opaque_softc *scn, int *irq, uint32_t size);
int hif_dump_registers(struct hif_opaque_softc *scn);
int ol_copy_ramdump(struct hif_opaque_softc *scn);
void hif_crash_shutdown(struct hif_opaque_softc *hif_ctx);
void hif_get_hw_info(struct hif_opaque_softc *hif_ctx, u32 *version,
		     u32 *revision, const char **target_name);
enum qdf_bus_type hif_get_bus_type(struct hif_opaque_softc *hif_hdl);
struct hif_target_info *hif_get_target_info_handle(struct hif_opaque_softc *
						   scn);
struct hif_config_info *hif_get_ini_handle(struct hif_opaque_softc *hif_ctx);
struct ramdump_info *hif_get_ramdump_ctx(struct hif_opaque_softc *hif_ctx);
enum hif_target_status hif_get_target_status(struct hif_opaque_softc *hif_ctx);
void hif_set_target_status(struct hif_opaque_softc *hif_ctx, enum
			   hif_target_status);
void hif_init_ini_config(struct hif_opaque_softc *hif_ctx,
			 struct hif_config_info *cfg);
void hif_update_tx_ring(struct hif_opaque_softc *osc, u_int32_t num_htt_cmpls);
qdf_nbuf_t hif_batch_send(struct hif_opaque_softc *osc, qdf_nbuf_t msdu,
		uint32_t transfer_id, u_int32_t len, uint32_t sendhead);
QDF_STATUS hif_send_single(struct hif_opaque_softc *osc, qdf_nbuf_t msdu,
			   uint32_t transfer_id, u_int32_t len);
int hif_send_fast(struct hif_opaque_softc *osc, qdf_nbuf_t nbuf,
	uint32_t transfer_id, uint32_t download_len);
void hif_pkt_dl_len_set(void *hif_sc, unsigned int pkt_download_len);
void hif_ce_war_disable(void);
void hif_ce_war_enable(void);
void hif_disable_interrupt(struct hif_opaque_softc *osc, uint32_t pipe_num);
#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
struct hif_pipe_addl_info *hif_get_addl_pipe_info(struct hif_opaque_softc *osc,
		struct hif_pipe_addl_info *hif_info, uint32_t pipe_number);
uint32_t hif_set_nss_wifiol_mode(struct hif_opaque_softc *osc,
		uint32_t pipe_num);
int32_t hif_get_nss_wifiol_bypass_nw_process(struct hif_opaque_softc *osc);
#endif /* QCA_NSS_WIFI_OFFLOAD_SUPPORT */

void hif_set_bundle_mode(struct hif_opaque_softc *hif_ctx, bool enabled,
				int rx_bundle_cnt);
int hif_bus_reset_resume(struct hif_opaque_softc *hif_ctx);

void hif_set_attribute(struct hif_opaque_softc *osc, uint8_t hif_attrib);

void *hif_get_lro_info(int ctx_id, struct hif_opaque_softc *hif_hdl);

enum hif_exec_type {
	HIF_EXEC_NAPI_TYPE,
	HIF_EXEC_TASKLET_TYPE,
};

typedef uint32_t (*ext_intr_handler)(void *, uint32_t);

/**
 * hif_get_int_ctx_irq_num() - retrieve an irq num for an interrupt context id
 * @softc: hif opaque context owning the exec context
 * @id: the id of the interrupt context
 *
 * Return: IRQ number of the first (zero'th) IRQ within the interrupt context ID
 *         'id' registered with the OS
 */
int32_t hif_get_int_ctx_irq_num(struct hif_opaque_softc *softc,
				uint8_t id);

/**
 * hif_configure_ext_group_interrupts() - Congigure ext group intrrupts
 * @hif_ctx: hif opaque context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hif_configure_ext_group_interrupts(struct hif_opaque_softc *hif_ctx);

/**
 * hif_deconfigure_ext_group_interrupts() - Deconfigure ext group intrrupts
 * @hif_ctx: hif opaque context
 *
 * Return: None
 */
void hif_deconfigure_ext_group_interrupts(struct hif_opaque_softc *hif_ctx);

/**
 * hif_register_ext_group() - API to register external group
 * interrupt handler.
 * @hif_ctx : HIF Context
 * @numirq: number of irq's in the group
 * @irq: array of irq values
 * @handler: callback interrupt handler function
 * @cb_ctx: context to passed in callback
 * @type: napi vs tasklet
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hif_register_ext_group(struct hif_opaque_softc *hif_ctx,
				  uint32_t numirq, uint32_t irq[],
				  ext_intr_handler handler,
				  void *cb_ctx, const char *context_name,
				  enum hif_exec_type type, uint32_t scale);

void hif_deregister_exec_group(struct hif_opaque_softc *hif_ctx,
				const char *context_name);

void hif_update_pipe_callback(struct hif_opaque_softc *osc,
				u_int8_t pipeid,
				struct hif_msg_callbacks *callbacks);

/**
 * hif_print_napi_stats() - Display HIF NAPI stats
 * @hif_ctx - HIF opaque context
 *
 * Return: None
 */
void hif_print_napi_stats(struct hif_opaque_softc *hif_ctx);

/* hif_clear_napi_stats() - function clears the stats of the
 * latency when called.
 * @hif_ctx - the HIF context to assign the callback to
 *
 * Return: None
 */
void hif_clear_napi_stats(struct hif_opaque_softc *hif_ctx);

#ifdef __cplusplus
}
#endif

#ifdef FORCE_WAKE
/**
 * hif_force_wake_request() - Function to wake from power collapse
 * @handle: HIF opaque handle
 *
 * Description: API to check if the device is awake or not before
 * read/write to BAR + 4K registers. If device is awake return
 * success otherwise write '1' to
 * PCIE_PCIE_LOCAL_REG_PCIE_SOC_WAKE_PCIE_LOCAL_REG which will interrupt
 * the device and does wakeup the PCI and MHI within 50ms
 * and then the device writes a value to
 * PCIE_SOC_PCIE_REG_PCIE_SCRATCH_0_SOC_PCIE_REG to complete the
 * handshake process to let the host know the device is awake.
 *
 * Return: zero - success/non-zero - failure
 */
int hif_force_wake_request(struct hif_opaque_softc *handle);

/**
 * hif_force_wake_release() - API to release/reset the SOC wake register
 * from interrupting the device.
 * @handle: HIF opaque handle
 *
 * Description: API to set the
 * PCIE_PCIE_LOCAL_REG_PCIE_SOC_WAKE_PCIE_LOCAL_REG to '0'
 * to release the interrupt line.
 *
 * Return: zero - success/non-zero - failure
 */
int hif_force_wake_release(struct hif_opaque_softc *handle);
#else
static inline
int hif_force_wake_request(struct hif_opaque_softc *handle)
{
	return 0;
}

static inline
int hif_force_wake_release(struct hif_opaque_softc *handle)
{
	return 0;
}
#endif /* FORCE_WAKE */

#ifdef FEATURE_HAL_DELAYED_REG_WRITE
/**
 * hif_prevent_link_low_power_states() - Prevent from going to low power states
 * @hif - HIF opaque context
 *
 * Return: 0 on success. Error code on failure.
 */
int hif_prevent_link_low_power_states(struct hif_opaque_softc *hif);

/**
 * hif_allow_link_low_power_states() - Allow link to go to low power states
 * @hif - HIF opaque context
 *
 * Return: None
 */
void hif_allow_link_low_power_states(struct hif_opaque_softc *hif);

#else

static inline
int hif_prevent_link_low_power_states(struct hif_opaque_softc *hif)
{
	return 0;
}

static inline
void hif_allow_link_low_power_states(struct hif_opaque_softc *hif)
{
}
#endif

void *hif_get_dev_ba(struct hif_opaque_softc *hif_handle);
void *hif_get_dev_ba_ce(struct hif_opaque_softc *hif_handle);

/**
 * hif_set_initial_wakeup_cb() - set the initial wakeup event handler function
 * @hif_ctx - the HIF context to assign the callback to
 * @callback - the callback to assign
 * @priv - the private data to pass to the callback when invoked
 *
 * Return: None
 */
void hif_set_initial_wakeup_cb(struct hif_opaque_softc *hif_ctx,
			       void (*callback)(void *),
			       void *priv);
/*
 * Note: For MCL, #if defined (HIF_CONFIG_SLUB_DEBUG_ON) needs to be checked
 * for defined here
 */
#if defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF)
ssize_t hif_dump_desc_trace_buf(struct device *dev,
				struct device_attribute *attr, char *buf);
ssize_t hif_input_desc_trace_buf_index(struct hif_softc *scn,
					const char *buf, size_t size);
ssize_t hif_ce_en_desc_hist(struct hif_softc *scn,
				const char *buf, size_t size);
ssize_t hif_disp_ce_enable_desc_data_hist(struct hif_softc *scn, char *buf);
ssize_t hif_dump_desc_event(struct hif_softc *scn, char *buf);
#endif/*#if defined(HIF_CONFIG_SLUB_DEBUG_ON)||defined(HIF_CE_DEBUG_DATA_BUF)*/

/**
 * hif_set_ce_service_max_yield_time() - sets CE service max yield time
 * @hif: hif context
 * @ce_service_max_yield_time: CE service max yield time to set
 *
 * This API storess CE service max yield time in hif context based
 * on ini value.
 *
 * Return: void
 */
void hif_set_ce_service_max_yield_time(struct hif_opaque_softc *hif,
				       uint32_t ce_service_max_yield_time);

/**
 * hif_get_ce_service_max_yield_time() - get CE service max yield time
 * @hif: hif context
 *
 * This API returns CE service max yield time.
 *
 * Return: CE service max yield time
 */
unsigned long long
hif_get_ce_service_max_yield_time(struct hif_opaque_softc *hif);

/**
 * hif_set_ce_service_max_rx_ind_flush() - sets CE service max rx ind flush
 * @hif: hif context
 * @ce_service_max_rx_ind_flush: CE service max rx ind flush to set
 *
 * This API stores CE service max rx ind flush in hif context based
 * on ini value.
 *
 * Return: void
 */
void hif_set_ce_service_max_rx_ind_flush(struct hif_opaque_softc *hif,
					 uint8_t ce_service_max_rx_ind_flush);

#ifdef OL_ATH_SMART_LOGGING
/*
 * hif_log_ce_dump() - Copy all the CE DEST ring to buf
 * @scn : HIF handler
 * @buf_cur: Current pointer in ring buffer
 * @buf_init:Start of the ring buffer
 * @buf_sz: Size of the ring buffer
 * @ce: Copy Engine id
 * @skb_sz: Max size of the SKB buffer to be copied
 *
 * Calls the respective function to dump all the CE SRC/DEST ring descriptors
 * and buffers pointed by them in to the given buf
 *
 * Return: Current pointer in ring buffer
 */
uint8_t *hif_log_dump_ce(struct hif_softc *scn, uint8_t *buf_cur,
			 uint8_t *buf_init, uint32_t buf_sz,
			 uint32_t ce, uint32_t skb_sz);
#endif /* OL_ATH_SMART_LOGGING */

/*
 * hif_softc_to_hif_opaque_softc - API to convert hif_softc handle
 * to hif_opaque_softc handle
 * @hif_handle - hif_softc type
 *
 * Return: hif_opaque_softc type
 */
static inline struct hif_opaque_softc *
hif_softc_to_hif_opaque_softc(struct hif_softc *hif_handle)
{
	return (struct hif_opaque_softc *)hif_handle;
}

#if defined(HIF_IPCI) && defined(FEATURE_HAL_DELAYED_REG_WRITE)
QDF_STATUS hif_try_prevent_ep_vote_access(struct hif_opaque_softc *hif_ctx);

/**
 * hif_set_ep_intermediate_vote_access() - Set intermediate EP vote access
 * @hif_ctx: opaque softc handle
 *
 * Return: QDF_STATUS of operation
 */
QDF_STATUS
hif_set_ep_intermediate_vote_access(struct hif_opaque_softc *hif_ctx);

void hif_allow_ep_vote_access(struct hif_opaque_softc *hif_ctx);
void hif_set_ep_vote_access(struct hif_opaque_softc *hif_ctx,
			    uint8_t type, uint8_t access);
uint8_t hif_get_ep_vote_access(struct hif_opaque_softc *hif_ctx,
			       uint8_t type);
#else
static inline QDF_STATUS
hif_try_prevent_ep_vote_access(struct hif_opaque_softc *hif_ctx)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
hif_set_ep_intermediate_vote_access(struct hif_opaque_softc *hif_ctx)
{
	return QDF_STATUS_SUCCESS;
}

static inline void
hif_allow_ep_vote_access(struct hif_opaque_softc *hif_ctx)
{
}

static inline void
hif_set_ep_vote_access(struct hif_opaque_softc *hif_ctx,
		       uint8_t type, uint8_t access)
{
}

static inline uint8_t
hif_get_ep_vote_access(struct hif_opaque_softc *hif_ctx,
		       uint8_t type)
{
	return HIF_EP_VOTE_ACCESS_ENABLE;
}
#endif

#ifdef FORCE_WAKE
/**
 * hif_srng_init_phase(): Indicate srng initialization phase
 * to avoid force wake as UMAC power collapse is not yet
 * enabled
 * @hif_ctx: hif opaque handle
 * @init_phase: initialization phase
 *
 * Return:  None
 */
void hif_srng_init_phase(struct hif_opaque_softc *hif_ctx,
			 bool init_phase);
#else
static inline
void hif_srng_init_phase(struct hif_opaque_softc *hif_ctx,
			 bool init_phase)
{
}
#endif /* FORCE_WAKE */

#ifdef HIF_IPCI
/**
 * hif_shutdown_notifier_cb - Call back for shutdown notifier
 * @ctx: hif handle
 *
 * Return:  None
 */
void hif_shutdown_notifier_cb(void *ctx);
#else
static inline
void hif_shutdown_notifier_cb(void *ctx)
{
}
#endif /* HIF_IPCI */

#ifdef HIF_CE_LOG_INFO
/**
 * hif_log_ce_info() - API to log ce info
 * @scn: hif handle
 * @data: hang event data buffer
 * @offset: offset at which data needs to be written
 *
 * Return:  None
 */
void hif_log_ce_info(struct hif_softc *scn, uint8_t *data,
		     unsigned int *offset);
#else
static inline
void hif_log_ce_info(struct hif_softc *scn, uint8_t *data,
		     unsigned int *offset)
{
}
#endif

#if defined(HIF_CPU_PERF_AFFINE_MASK) || \
    defined(FEATURE_ENABLE_CE_DP_IRQ_AFFINE)
/**
 * hif_config_irq_set_perf_affinity_hint() - API to set affinity
 * @hif_ctx: hif opaque handle
 *
 * This function is used to move the WLAN IRQs to perf cores in
 * case of defconfig builds.
 *
 * Return:  None
 */
void hif_config_irq_set_perf_affinity_hint(
	struct hif_opaque_softc *hif_ctx);

#else
static inline void hif_config_irq_set_perf_affinity_hint(
	struct hif_opaque_softc *hif_ctx)
{
}
#endif

/**
 * hif_apps_grp_irqs_enable() - enable ext grp irqs
 * @hif - HIF opaque context
 *
 * Return: 0 on success. Error code on failure.
 */
int hif_apps_grp_irqs_enable(struct hif_opaque_softc *hif_ctx);

/**
 * hif_apps_grp_irqs_disable() - disable ext grp irqs
 * @hif - HIF opaque context
 *
 * Return: 0 on success. Error code on failure.
 */
int hif_apps_grp_irqs_disable(struct hif_opaque_softc *hif_ctx);

/**
 * hif_disable_grp_irqs() - disable ext grp irqs
 * @hif - HIF opaque context
 *
 * Return: 0 on success. Error code on failure.
 */
int hif_disable_grp_irqs(struct hif_opaque_softc *scn);

/**
 * hif_enable_grp_irqs() - enable ext grp irqs
 * @hif - HIF opaque context
 *
 * Return: 0 on success. Error code on failure.
 */
int hif_enable_grp_irqs(struct hif_opaque_softc *scn);

enum hif_credit_exchange_type {
	HIF_REQUEST_CREDIT,
	HIF_PROCESS_CREDIT_REPORT,
};

enum hif_detect_latency_type {
	HIF_DETECT_TASKLET,
	HIF_DETECT_CREDIT,
	HIF_DETECT_UNKNOWN
};

#ifdef HIF_DETECTION_LATENCY_ENABLE
void hif_latency_detect_credit_record_time(
	enum hif_credit_exchange_type type,
	struct hif_opaque_softc *hif_ctx);

void hif_latency_detect_timer_start(struct hif_opaque_softc *hif_ctx);
void hif_latency_detect_timer_stop(struct hif_opaque_softc *hif_ctx);
void hif_tasklet_latency(struct hif_softc *scn, bool from_timer);
void hif_credit_latency(struct hif_softc *scn, bool from_timer);
void hif_check_detection_latency(struct hif_softc *scn,
				 bool from_timer,
				 uint32_t bitmap_type);
void hif_set_enable_detection(struct hif_opaque_softc *hif_ctx, bool value);
#else
static inline
void hif_latency_detect_timer_start(struct hif_opaque_softc *hif_ctx)
{}

static inline
void hif_latency_detect_timer_stop(struct hif_opaque_softc *hif_ctx)
{}

static inline
void hif_latency_detect_credit_record_time(
	enum hif_credit_exchange_type type,
	struct hif_opaque_softc *hif_ctx)
{}
static inline
void hif_check_detection_latency(struct hif_softc *scn,
				 bool from_timer,
				 uint32_t bitmap_type)
{}

static inline
void hif_set_enable_detection(struct hif_opaque_softc *hif_ctx, bool value)
{}
#endif

#ifdef SYSTEM_PM_CHECK
/**
 * __hif_system_pm_set_state() - Set system pm state
 * @hif: hif opaque handle
 * @state: system state
 *
 * Return:  None
 */
void __hif_system_pm_set_state(struct hif_opaque_softc *hif,
			       enum hif_system_pm_state state);

/**
 * hif_system_pm_set_state_on() - Set system pm state to ON
 * @hif: hif opaque handle
 *
 * Return:  None
 */
static inline
void hif_system_pm_set_state_on(struct hif_opaque_softc *hif)
{
	__hif_system_pm_set_state(hif, HIF_SYSTEM_PM_STATE_ON);
}

/**
 * hif_system_pm_set_state_resuming() - Set system pm state to resuming
 * @hif: hif opaque handle
 *
 * Return:  None
 */
static inline
void hif_system_pm_set_state_resuming(struct hif_opaque_softc *hif)
{
	__hif_system_pm_set_state(hif, HIF_SYSTEM_PM_STATE_BUS_RESUMING);
}

/**
 * hif_system_pm_set_state_suspending() - Set system pm state to suspending
 * @hif: hif opaque handle
 *
 * Return:  None
 */
static inline
void hif_system_pm_set_state_suspending(struct hif_opaque_softc *hif)
{
	__hif_system_pm_set_state(hif, HIF_SYSTEM_PM_STATE_BUS_SUSPENDING);
}

/**
 * hif_system_pm_set_state_suspended() - Set system pm state to suspended
 * @hif: hif opaque handle
 *
 * Return:  None
 */
static inline
void hif_system_pm_set_state_suspended(struct hif_opaque_softc *hif)
{
	__hif_system_pm_set_state(hif, HIF_SYSTEM_PM_STATE_BUS_SUSPENDED);
}

/**
 * hif_system_pm_get_state() - Get system pm state
 * @hif: hif opaque handle
 *
 * Return:  system state
 */
int32_t hif_system_pm_get_state(struct hif_opaque_softc *hif);

/**
 * hif_system_pm_state_check() - Check system state and trigger resume
 *  if required
 * @hif: hif opaque handle
 *
 * Return: 0 if system is in on state else error code
 */
int hif_system_pm_state_check(struct hif_opaque_softc *hif);
#else
static inline
void __hif_system_pm_set_state(struct hif_opaque_softc *hif,
			       enum hif_system_pm_state state)
{
}

static inline
void hif_system_pm_set_state_on(struct hif_opaque_softc *hif)
{
}

static inline
void hif_system_pm_set_state_resuming(struct hif_opaque_softc *hif)
{
}

static inline
void hif_system_pm_set_state_suspending(struct hif_opaque_softc *hif)
{
}

static inline
void hif_system_pm_set_state_suspended(struct hif_opaque_softc *hif)
{
}

static inline
int32_t hif_system_pm_get_state(struct hif_opaque_softc *hif)
{
	return 0;
}

static inline int hif_system_pm_state_check(struct hif_opaque_softc *hif)
{
	return 0;
}
#endif

#ifdef FEATURE_IRQ_AFFINITY
/**
 * hif_set_grp_intr_affinity() - API to set affinity for grp
 *  intrs set in the bitmap
 * @scn: hif handle
 * @grp_intr_bitmask: grp intrs for which perf affinity should be
 *  applied
 * @perf: affine to perf or non-perf cluster
 *
 * Return: None
 */
void hif_set_grp_intr_affinity(struct hif_opaque_softc *scn,
			       uint32_t grp_intr_bitmask, bool perf);
#else
static inline
void hif_set_grp_intr_affinity(struct hif_opaque_softc *scn,
			       uint32_t grp_intr_bitmask, bool perf)
{
}
#endif

static inline QDF_STATUS
hif_irq_set_affinity_hint(int irq_num, qdf_cpu_mask *cpu_mask)
{
	QDF_STATUS status;

	qdf_dev_modify_irq_status(irq_num, IRQ_NO_BALANCING, 0);
	status = qdf_dev_set_irq_affinity(irq_num,
					  (struct qdf_cpu_mask *)cpu_mask);
	qdf_dev_modify_irq_status(irq_num, 0, IRQ_NO_BALANCING);

	return status;
}
#endif /* _HIF_H_ */
