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

#ifndef _HIF_H_
#define _HIF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Header files */
#include <qdf_status.h>
#include "qdf_nbuf.h"
#include "ol_if_athvar.h"
#include <linux/platform_device.h>
#ifdef HIF_PCI
#include <linux/pci.h>
#endif /* HIF_PCI */
#ifdef HIF_USB
#include <linux/usb.h>
#endif /* HIF_USB */
#define ENABLE_MBOX_DUMMY_SPACE_FEATURE 1

typedef struct htc_callbacks HTC_CALLBACKS;
typedef void __iomem *A_target_id_t;
typedef void *hif_handle_t;

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

/* TARGET definition needs to be abstracted in fw common
 * header files, below is the placeholder till WIN codebase
 * moved to latest copy of fw common header files.
 */
#ifdef CONFIG_WIN
#define TARGET_TYPE_UNKNOWN   0
#define TARGET_TYPE_AR6001    1
#define TARGET_TYPE_AR6002    2
#define TARGET_TYPE_AR6003    3
#define TARGET_TYPE_AR6004    5
#define TARGET_TYPE_AR6006    6
#define TARGET_TYPE_AR9888    7
#define TARGET_TYPE_AR6320    8
#define TARGET_TYPE_AR900B    9
#define TARGET_TYPE_QCA9984   10
#define TARGET_TYPE_IPQ4019   11
#define TARGET_TYPE_QCA9888   12
/* For attach Peregrine 2.0 board target_reg_tbl only */
#define TARGET_TYPE_AR9888V2  13
/* For attach Rome1.0 target_reg_tbl only*/
#define TARGET_TYPE_AR6320V1    14
/* For Rome2.0/2.1 target_reg_tbl ID*/
#define TARGET_TYPE_AR6320V2    15
/* For Rome3.0 target_reg_tbl ID*/
#define TARGET_TYPE_AR6320V3    16
/* For Tufello1.0 target_reg_tbl ID*/
#define TARGET_TYPE_QCA9377V1   17
/* For Adrastea target */
#define TARGET_TYPE_ADRASTEA     19
#endif

struct CE_state;
#define CE_COUNT_MAX 12

#ifdef CONFIG_SLUB_DEBUG_ON
#define QCA_NAPI_BUDGET    64
#define QCA_NAPI_DEF_SCALE  2
#else  /* PERF build */
#define QCA_NAPI_BUDGET    64
#define QCA_NAPI_DEF_SCALE 16
#endif /* SLUB_DEBUG_ON */
#define HIF_NAPI_MAX_RECEIVES (QCA_NAPI_BUDGET * QCA_NAPI_DEF_SCALE)

/* NOTE: "napi->scale" can be changed,
   but this does not change the number of buckets */
#define QCA_NAPI_NUM_BUCKETS (QCA_NAPI_BUDGET / QCA_NAPI_DEF_SCALE)
struct qca_napi_stat {
	uint32_t napi_schedules;
	uint32_t napi_polls;
	uint32_t napi_completes;
	uint32_t napi_workdone;
	uint32_t napi_budget_uses[QCA_NAPI_NUM_BUCKETS];
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
	struct napi_struct   napi;    /* one NAPI Instance per CE in phase I */
	uint8_t              scale;   /* currently same on all instances */
	uint8_t              id;
	struct qca_napi_stat stats[NR_CPUS];
};

/**
 * NAPI data-sructure common to all NAPI instances.
 *
 * A variable of this type will be stored in hif module context.
 */

struct qca_napi_data {
	/* NOTE: make sure the mutex is inited only at the very beginning
	   once for the lifetime of the driver. For now, granularity of one
	   is OK, but we might want to have a better granularity later */
	struct mutex         mutex;
	uint32_t             state;
	uint32_t             ce_map; /* bitmap of created/registered NAPI
					instances, indexed by pipe_id,
					not used by clients (clients use an
					id returned by create) */
	struct qca_napi_info napis[CE_COUNT_MAX];
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
};

struct hif_opaque_softc {
};

typedef enum {
	HIF_DEVICE_POWER_UP,       /* HIF layer should power up interface
				    * and/or module */
	HIF_DEVICE_POWER_DOWN,     /* HIF layer should initiate bus-specific
				    * measures to minimize power */
	HIF_DEVICE_POWER_CUT       /* HIF layer should initiate bus-specific
				    * AND/OR platform-specific measures
				    * to completely power-off the module and
				    * associated hardware (i.e. cut power
				    * supplies) */
} HIF_DEVICE_POWER_CHANGE_TYPE;

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
 * @HIF_DISABLE_TYPE_REINIT_ERROR: reinit error triggered
 *  							 disable
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
typedef struct _HID_ACCESS_LOG {
	uint32_t seqnum;
	bool is_write;
	void *addr;
	uint32_t value;
} HIF_ACCESS_LOG;
#endif

void hif_reg_write(struct hif_opaque_softc *hif_ctx, uint32_t offset,
		uint32_t value);
uint32_t hif_reg_read(struct hif_opaque_softc *hif_ctx, uint32_t offset);

#define HIF_MAX_DEVICES                 1

struct htc_callbacks {
	void *context;		/* context to pass to the dsrhandler
				 * note : rwCompletionHandler is provided
				 * the context passed to hif_read_write  */
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
};

/* This API detaches the HTC layer from the HIF device */
void hif_detach_htc(struct hif_opaque_softc *scn);

/****************************************************************/
/* BMI and Diag window abstraction                              */
/****************************************************************/

#define HIF_BMI_EXCHANGE_NO_TIMEOUT  ((uint32_t)(0))

#define DIAG_TRANSFER_LIMIT 2048U   /* maximum number of bytes that can be
				     * handled atomically by
				     * DiagRead/DiagWrite */

/*
 * API to handle HIF-specific BMI message exchanges, this API is synchronous
 * and only allowed to be called from a context that can block (sleep) */
QDF_STATUS hif_exchange_bmi_msg(struct hif_opaque_softc *scn,
				qdf_dma_addr_t cmd, qdf_dma_addr_t rsp,
				uint8_t *pSendMessage, uint32_t Length,
				uint8_t *pResponseMessage,
				uint32_t *pResponseLength, uint32_t TimeoutMS);

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
QDF_STATUS hif_diag_read_access(struct hif_opaque_softc *scn, uint32_t address,
			 uint32_t *data);
QDF_STATUS hif_diag_read_mem(struct hif_opaque_softc *scn, uint32_t address,
		      uint8_t *data, int nbytes);
void hif_dump_target_memory(struct hif_opaque_softc *scn, void *ramdump_base,
			    uint32_t address, uint32_t size);
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
QDF_STATUS hif_diag_write_access(struct hif_opaque_softc *scn, uint32_t address,
				 uint32_t data);
QDF_STATUS hif_diag_write_mem(struct hif_opaque_softc *scn, uint32_t address,
		       uint8_t *data, int nbytes);

typedef void (*fastpath_msg_handler)(void *, qdf_nbuf_t *, uint32_t);

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

void hif_ipa_get_ce_resource(struct hif_opaque_softc *scn,
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
typedef struct hif_bus_id hif_bus_id;

void hif_claim_device(struct hif_opaque_softc *hif_ctx);
QDF_STATUS hif_get_config_item(struct hif_opaque_softc *hif_ctx,
		     int opcode, void *config, uint32_t config_len);
void hif_set_mailbox_swap(struct hif_opaque_softc *hif_ctx);
void hif_mask_interrupt_call(struct hif_opaque_softc *scn);
void hif_post_init(struct hif_opaque_softc *scn, void *hHTC,
		   struct hif_msg_callbacks *callbacks);
QDF_STATUS hif_start(struct hif_opaque_softc *scn);
void hif_stop(struct hif_opaque_softc *scn);
void hif_flush_surprise_remove(struct hif_opaque_softc *scn);
void hif_dump(struct hif_opaque_softc *scn, uint8_t CmdId, bool start);
void hif_trigger_dump(struct hif_opaque_softc *hif_ctx,
		      uint8_t cmd_id, bool start);

QDF_STATUS hif_send_head(struct hif_opaque_softc *scn, uint8_t PipeID,
				  uint32_t transferID, uint32_t nbytes,
				  qdf_nbuf_t wbuf, uint32_t data_attr);
void hif_send_complete_check(struct hif_opaque_softc *scn, uint8_t PipeID,
			     int force);
void hif_shut_down_device(struct hif_opaque_softc *scn);
void hif_get_default_pipe(struct hif_opaque_softc *scn, uint8_t *ULPipe,
			  uint8_t *DLPipe);
int hif_map_service_to_pipe(struct hif_opaque_softc *scn, uint16_t svc_id,
			uint8_t *ul_pipe, uint8_t *dl_pipe, int *ul_is_polled,
			int *dl_is_polled);
uint16_t
hif_get_free_queue_number(struct hif_opaque_softc *scn, uint8_t PipeID);
void *hif_get_targetdef(struct hif_opaque_softc *scn);
uint32_t hif_hia_item_address(uint32_t target_type, uint32_t item_offset);
void hif_set_target_sleep(struct hif_opaque_softc *scn, bool sleep_ok,
		     bool wait_for_it);
int hif_check_fw_reg(struct hif_opaque_softc *scn);
#ifndef HIF_PCI
static inline int hif_check_soc_status(struct hif_opaque_softc *scn)
{
	return 0;
}
#else
int hif_check_soc_status(struct hif_opaque_softc *scn);
#endif
void hif_get_hw_info(struct hif_opaque_softc *scn, u32 *version, u32 *revision,
		     const char **target_name);
void hif_disable_isr(struct hif_opaque_softc *scn);
void hif_reset_soc(struct hif_opaque_softc *scn);
void hif_save_htc_htt_config_endpoint(struct hif_opaque_softc *hif_ctx,
				      int htc_htt_tx_endpoint);
struct hif_opaque_softc *hif_open(qdf_device_t qdf_ctx, uint32_t mode,
				  enum qdf_bus_type bus_type,
				  struct hif_driver_state_callbacks *cbk);
void hif_close(struct hif_opaque_softc *hif_ctx);
QDF_STATUS hif_enable(struct hif_opaque_softc *hif_ctx, struct device *dev,
		      void *bdev, const hif_bus_id *bid,
		      enum qdf_bus_type bus_type,
		      enum hif_enable_type type);
void hif_disable(struct hif_opaque_softc *hif_ctx, enum hif_disable_type type);
void hif_display_stats(struct hif_opaque_softc *hif_ctx);
void hif_clear_stats(struct hif_opaque_softc *hif_ctx);
#ifdef FEATURE_RUNTIME_PM
struct hif_pm_runtime_lock;
int hif_pm_runtime_get(struct hif_opaque_softc *hif_ctx);
void hif_pm_runtime_get_noresume(struct hif_opaque_softc *hif_ctx);
int hif_pm_runtime_put(struct hif_opaque_softc *hif_ctx);
struct hif_pm_runtime_lock *hif_runtime_lock_init(const char *name);
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

static inline void hif_pm_runtime_get_noresume(struct hif_opaque_softc *hif_ctx)
{}

static inline int hif_pm_runtime_get(struct hif_opaque_softc *hif_ctx)
{ return 0; }
static inline int hif_pm_runtime_put(struct hif_opaque_softc *hif_ctx)
{ return 0; }
static inline struct hif_pm_runtime_lock *hif_runtime_lock_init(
		const char *name)
{ return NULL; }
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

void hif_vote_link_down(struct hif_opaque_softc *);
void hif_vote_link_up(struct hif_opaque_softc *);
bool hif_can_suspend_link(struct hif_opaque_softc *);

int hif_bus_resume(struct hif_opaque_softc *);
int hif_bus_suspend(struct hif_opaque_softc *);

#ifdef FEATURE_RUNTIME_PM
int hif_pre_runtime_suspend(struct hif_opaque_softc *hif_ctx);
void hif_pre_runtime_resume(struct hif_opaque_softc *hif_ctx);
int hif_runtime_suspend(struct hif_opaque_softc *hif_ctx);
int hif_runtime_resume(struct hif_opaque_softc *hif_ctx);
void hif_process_runtime_suspend_success(struct hif_opaque_softc *);
void hif_process_runtime_suspend_failure(struct hif_opaque_softc *);
void hif_process_runtime_resume_success(struct hif_opaque_softc *);
#endif

int hif_dump_registers(struct hif_opaque_softc *scn);
int ol_copy_ramdump(struct hif_opaque_softc *scn);
void hif_crash_shutdown(struct hif_opaque_softc *hif_ctx);
void hif_get_hw_info(struct hif_opaque_softc *scn, u32 *version, u32 *revision,
		     const char **target_name);
void hif_lro_flush_cb_register(struct hif_opaque_softc *scn,
			       void (handler)(void *), void *data);
void hif_lro_flush_cb_deregister(struct hif_opaque_softc *scn);
bool hif_needs_bmi(struct hif_opaque_softc *scn);
enum qdf_bus_type hif_get_bus_type(struct hif_opaque_softc *hif_hdl);
struct hif_target_info *hif_get_target_info_handle(struct hif_opaque_softc *
						   scn);
struct hif_config_info *hif_get_ini_handle(struct hif_opaque_softc *scn);
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

void hif_set_bundle_mode(struct hif_opaque_softc *scn, bool enabled,
				int rx_bundle_cnt);
int hif_bus_reset_resume(struct hif_opaque_softc *scn);

#ifdef WLAN_SUSPEND_RESUME_TEST
typedef void (*hdd_fake_resume_callback)(uint32_t val);
void hif_fake_apps_suspend(hdd_fake_resume_callback callback);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _HIF_H_ */
