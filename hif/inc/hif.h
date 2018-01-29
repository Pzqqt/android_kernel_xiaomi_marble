/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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

#ifndef _HIF_H_
#define _HIF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Header files */
#include <qdf_status.h>
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
#define ENABLE_MBOX_DUMMY_SPACE_FEATURE 1

typedef void __iomem *A_target_id_t;
typedef void *hif_handle_t;

#define HIF_DBG_PRINT_RATE 1000

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

#ifdef IPA_OFFLOAD
#define DMA_COHERENT_MASK_IPA_VER_3_AND_ABOVE   37
#define DMA_COHERENT_MASK_BELOW_IPA_VER_3       32
#endif

/* enum hif_ic_irq - enum defining integrated chip irq numbers
 * defining irq nubers that can be used by external modules like datapath
 */
enum hif_ic_irq {
	host2wbm_desc_feed = 18,
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
#define CE_COUNT_MAX 12
#define HIF_MAX_GRP_IRQ 16
#define HIF_MAX_GROUP 8

#ifdef CONFIG_SLUB_DEBUG_ON
#ifndef CONFIG_WIN
#define HIF_CONFIG_SLUB_DEBUG_ON
#endif
#endif

#ifndef NAPI_YIELD_BUDGET_BASED
#ifdef HIF_CONFIG_SLUB_DEBUG_ON
#define QCA_NAPI_DEF_SCALE_BIN_SHIFT 1
#else  /* PERF build */
#ifdef CONFIG_WIN
#define QCA_NAPI_DEF_SCALE_BIN_SHIFT 1
#else
#define QCA_NAPI_DEF_SCALE_BIN_SHIFT 4
#endif /* CONFIG_WIN */
#endif /* SLUB_DEBUG_ON */
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
 *
 * needs to be renamed
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
	struct qca_napi_stat stats[NR_CPUS];
	/* will only be present for data rx CE's */
	void (*lro_flush_cb)(void *);
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
	struct qca_napi_info *napis[CE_COUNT_MAX];
	struct qca_napi_cpu  napi_cpu[NR_CPUS];
	int                  lilcl_head, bigcl_head;
	enum qca_napi_tput_state napi_mode;
	struct qdf_cpuhp_handler *cpuhp_handler;
	uint8_t              flags;
};

/**
 * struct hif_config_info - Place Holder for hif confiruation
 * @enable_self_recovery: Self Recovery
 *
 * Structure for holding hif ini parameters.
 */
struct hif_config_info {
	bool enable_self_recovery;
#ifdef FEATURE_RUNTIME_PM
	bool enable_runtime_pm;
	u_int32_t runtime_pm_delay;
#endif
};

/**
 * struct hif_target_info - Target Information
 * @target_version: Target Version
 * @target_type: Target Type
 * @target_revision: Target Revision
 * @soc_version: SOC Version
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
 * @HIF_DEVICE_GET_MBOX_BLOCK_SIZE: get mbox block size
 * @HIF_DEVICE_GET_MBOX_ADDR: get mbox block address
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
	HIF_DEVICE_GET_MBOX_BLOCK_SIZE,
	HIF_DEVICE_GET_MBOX_ADDR,
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
	QDF_STATUS(*rwCompletionHandler)(void *rwContext, QDF_STATUS status);
	QDF_STATUS(*dsrHandler)(void *context);
};

/**
 * struct hif_driver_state_callbacks - Callbacks for HIF to query Driver state
 * @context: Private data context
 * @set_recovery_in_progress: To Set Driver state for recovery in progress
 * @is_recovery_in_progress: Query if driver state is recovery in progress
 * @is_load_unload_in_progress: Query if driver state Load/Unload in Progress
 * @is_driver_unloading: Query if driver is unloading.
 *
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

/*
 * API to handle HIF-specific BMI message exchanges, this API is synchronous
 * and only allowed to be called from a context that can block (sleep)
 */
QDF_STATUS hif_exchange_bmi_msg(struct hif_opaque_softc *hif_ctx,
				qdf_dma_addr_t cmd, qdf_dma_addr_t rsp,
				uint8_t *pSendMessage, uint32_t Length,
				uint8_t *pResponseMessage,
				uint32_t *pResponseLength, uint32_t TimeoutMS);
void hif_register_bmi_callbacks(struct hif_softc *hif_sc);
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
int hif_ce_fastpath_cb_register(struct hif_opaque_softc *hif_ctx,
				fastpath_msg_handler handler, void *context);
#else
static inline int hif_ce_fastpath_cb_register(struct hif_opaque_softc *hif_ctx,
					      fastpath_msg_handler handler,
					      void *context)
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
			     qdf_dma_addr_t *ce_sr_base_paddr,
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
void hif_disable_isr(struct hif_opaque_softc *hif_ctx);
void hif_reset_soc(struct hif_opaque_softc *hif_ctx);
void hif_save_htc_htt_config_endpoint(struct hif_opaque_softc *hif_ctx,
				      int htc_htt_tx_endpoint);
struct hif_opaque_softc *hif_open(qdf_device_t qdf_ctx, uint32_t mode,
				  enum qdf_bus_type bus_type,
				  struct hif_driver_state_callbacks *cbk);
void hif_close(struct hif_opaque_softc *hif_ctx);
QDF_STATUS hif_enable(struct hif_opaque_softc *hif_ctx, struct device *dev,
		      void *bdev, const struct hif_bus_id *bid,
		      enum qdf_bus_type bus_type,
		      enum hif_enable_type type);
void hif_disable(struct hif_opaque_softc *hif_ctx, enum hif_disable_type type);
void hif_display_stats(struct hif_opaque_softc *hif_ctx);
void hif_clear_stats(struct hif_opaque_softc *hif_ctx);
#ifdef FEATURE_RUNTIME_PM
struct hif_pm_runtime_lock;
void hif_fastpath_resume(struct hif_opaque_softc *hif_ctx);
int hif_pm_runtime_get(struct hif_opaque_softc *hif_ctx);
void hif_pm_runtime_get_noresume(struct hif_opaque_softc *hif_ctx);
int hif_pm_runtime_put(struct hif_opaque_softc *hif_ctx);
int hif_runtime_lock_init(qdf_runtime_lock_t *lock, const char *name);
void hif_runtime_lock_deinit(struct hif_opaque_softc *hif_ctx,
			struct hif_pm_runtime_lock *lock);
int hif_pm_runtime_prevent_suspend(struct hif_opaque_softc *ol_sc,
		struct hif_pm_runtime_lock *lock);
int hif_pm_runtime_allow_suspend(struct hif_opaque_softc *ol_sc,
		struct hif_pm_runtime_lock *lock);
int hif_pm_runtime_prevent_suspend_timeout(struct hif_opaque_softc *ol_sc,
		struct hif_pm_runtime_lock *lock, unsigned int delay);
#else
struct hif_pm_runtime_lock {
	const char *name;
};
static inline void hif_fastpath_resume(struct hif_opaque_softc *hif_ctx) {}
static inline void hif_pm_runtime_get_noresume(struct hif_opaque_softc *hif_ctx)
{}

static inline int hif_pm_runtime_get(struct hif_opaque_softc *hif_ctx)
{ return 0; }
static inline int hif_pm_runtime_put(struct hif_opaque_softc *hif_ctx)
{ return 0; }
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
static inline int
hif_pm_runtime_prevent_suspend_timeout(struct hif_opaque_softc *ol_sc,
		struct hif_pm_runtime_lock *lock, unsigned int delay)
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
	if (ipa_uc_reg_rdyCB(NULL) != -EPERM)
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

#ifdef FEATURE_RUNTIME_PM
int hif_pre_runtime_suspend(struct hif_opaque_softc *hif_ctx);
void hif_pre_runtime_resume(struct hif_opaque_softc *hif_ctx);
int hif_runtime_suspend(struct hif_opaque_softc *hif_ctx);
int hif_runtime_resume(struct hif_opaque_softc *hif_ctx);
void hif_process_runtime_suspend_success(struct hif_opaque_softc *hif_ctx);
void hif_process_runtime_suspend_failure(struct hif_opaque_softc *hif_ctx);
void hif_process_runtime_resume_success(struct hif_opaque_softc *hif_ctx);
#endif

int hif_get_irq_num(struct hif_opaque_softc *scn, int *irq, uint32_t size);
int hif_dump_registers(struct hif_opaque_softc *scn);
int ol_copy_ramdump(struct hif_opaque_softc *scn);
void hif_crash_shutdown(struct hif_opaque_softc *hif_ctx);
void hif_get_hw_info(struct hif_opaque_softc *hif_ctx, u32 *version,
		     u32 *revision, const char **target_name);
void hif_lro_flush_cb_register(struct hif_opaque_softc *hif_ctx,
			       void (lro_flush_handler)(void *arg),
			       void *(lro_init_handler)(void));
void hif_lro_flush_cb_deregister(struct hif_opaque_softc *hif_ctx,
				 void (lro_deinit_cb)(void *arg));
bool hif_needs_bmi(struct hif_opaque_softc *hif_ctx);
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
int hif_send_single(struct hif_opaque_softc *osc, qdf_nbuf_t msdu, uint32_t
		transfer_id, u_int32_t len);
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
uint32_t hif_configure_ext_group_interrupts(struct hif_opaque_softc *hif_ctx);
uint32_t  hif_register_ext_group(struct hif_opaque_softc *hif_ctx,
		uint32_t numirq, uint32_t irq[], ext_intr_handler handler,
		void *cb_ctx, const char *context_name,
		enum hif_exec_type type, uint32_t scale);

void hif_deregister_exec_group(struct hif_opaque_softc *hif_ctx,
				const char *context_name);

void hif_update_pipe_callback(struct hif_opaque_softc *osc,
				u_int8_t pipeid,
				struct hif_msg_callbacks *callbacks);

void hif_print_napi_stats(struct hif_opaque_softc *hif_ctx);
#ifdef __cplusplus
}
#endif

void *hif_get_dev_ba(struct hif_opaque_softc *hif_handle);

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
#ifndef CONFIG_WIN
#ifndef HIF_CE_DEBUG_DATA_BUF
#define HIF_CE_DEBUG_DATA_BUF 0
#endif
#endif
/*
 * Note: For MCL, #if defined (HIF_CONFIG_SLUB_DEBUG_ON) needs to be checked
 * for defined here
 */
#if HIF_CE_DEBUG_DATA_BUF
ssize_t hif_dump_desc_trace_buf(struct device *dev,
				struct device_attribute *attr, char *buf);
ssize_t hif_input_desc_trace_buf_index(struct hif_softc *scn,
					const char *buf, size_t size);
ssize_t hif_ce_en_desc_hist(struct hif_softc *scn,
				const char *buf, size_t size);
ssize_t hif_disp_ce_enable_desc_data_hist(struct hif_softc *scn, char *buf);
ssize_t hif_dump_desc_event(struct hif_softc *scn, char *buf);
#endif /* Note: for MCL, #if defined(HIF_CONFIG_SLUB_DEBUG_ON) || HIF_CE_DEBUG_DATA_BUF */
#endif /* _HIF_H_ */
