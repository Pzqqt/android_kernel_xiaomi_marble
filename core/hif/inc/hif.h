/*
 * Copyright (c) 2013-2015 The Linux Foundation. All rights reserved.
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
#include "athdefs.h"
#include "a_types.h"
#include "osapi_linux.h"
#include "cdf_status.h"
#include "cdf_nbuf.h"
#include "ol_if_athvar.h"
#include <linux/platform_device.h>
#ifdef HIF_PCI
#include <linux/pci.h>
#endif /* HIF_PCI */

#define ENABLE_MBOX_DUMMY_SPACE_FEATURE 1

typedef struct htc_callbacks HTC_CALLBACKS;
typedef void __iomem *A_target_id_t;

#define HIF_TYPE_AR6002   2
#define HIF_TYPE_AR6003   3
#define HIF_TYPE_AR6004   5
#define HIF_TYPE_AR9888   6
#define HIF_TYPE_AR6320   7
#define HIF_TYPE_AR6320V2 8
/* For attaching Peregrine 2.0 board host_reg_tbl only */
#define HIF_TYPE_AR9888V2 8
#define HIF_TYPE_QCA6180  9
#define HIF_TYPE_ADRASTEA 10

#define TARGET_TYPE_UNKNOWN   0
#define TARGET_TYPE_AR6001    1
#define TARGET_TYPE_AR6002    2
#define TARGET_TYPE_AR6003    3
#define TARGET_TYPE_AR6004    5
#define TARGET_TYPE_AR6006    6
#define TARGET_TYPE_AR9888    7
#define TARGET_TYPE_AR6320    8
#define TARGET_TYPE_AR900B    9
/* For attach Peregrine 2.0 board target_reg_tbl only */
#define TARGET_TYPE_AR9888V2  10
/* For attach Rome1.0 target_reg_tbl only*/
#define TARGET_TYPE_AR6320V1    11
/* For Rome2.0/2.1 target_reg_tbl ID*/
#define TARGET_TYPE_AR6320V2    12
/* For Rome3.0 target_reg_tbl ID*/
#define TARGET_TYPE_AR6320V3    13
/* For Tufello1.0 target_reg_tbl ID*/
#define TARGET_TYPE_QCA9377V1   14
/* For QCA6180 target */
#define TARGET_TYPE_QCA6180     15
/* For Adrastea target */
#define TARGET_TYPE_ADRASTEA     16

struct CE_state;
#ifdef QCA_WIFI_3_0_ADRASTEA
#define CE_COUNT_MAX 12
#else
#define CE_COUNT_MAX 8
#endif

/* These numbers are selected so that the product is close to current
   higher limit of packets HIF services at one shot (1000) */
#define QCA_NAPI_BUDGET    64
#define QCA_NAPI_DEF_SCALE 16
/* NOTE: This is to adapt non-NAPI solution to use
   the same "budget" as NAPI. Will be removed
   `once decision about NAPI is made */
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
	struct net_device    netdev; /* dummy net_dev */
	struct qca_napi_info napis[CE_COUNT_MAX];
};

struct ol_softc {
	void __iomem *mem;      /* IO mapped memory base address */
	cdf_dma_addr_t mem_pa;
	uint32_t soc_version;
	/*
	 * handle for code that uses the osdep.h version of OS
	 * abstraction primitives
	 */
	struct _NIC_DEV aps_osdev;
	enum ath_hal_bus_type bus_type;
	uint32_t lcr_val;
	bool pkt_log_init;
	bool request_irq_done;
	/*
	 * handle for code that uses cdf version of OS
	 * abstraction primitives
	 */
	cdf_device_t cdf_dev;

	struct ol_version version;

	/* Packet statistics */
	struct ol_ath_stats pkt_stats;

	/* A_TARGET_TYPE_* */
	uint32_t target_type;
	uint32_t target_fw_version;
	uint32_t target_version;
	uint32_t target_revision;
	uint8_t crm_version_string[64];
	uint8_t wlan_version_string[64];
	ol_target_status target_status;
	bool is_sim;
	/* calibration data is stored in flash */
	uint8_t *cal_in_flash;
	/* virtual address for the calibration data on the flash */
	void *cal_mem;
	/* status of target init */
	WLAN_INIT_STATUS wlan_init_status;

	/* BMI info */
	/* OS-dependent private info for BMI */
	void *bmi_ol_priv;
	bool bmi_done;
	bool bmi_ua_done;
	uint8_t *bmi_cmd_buff;
	dma_addr_t bmi_cmd_da;
	OS_DMA_MEM_CONTEXT(bmicmd_dmacontext)

	uint8_t *bmi_rsp_buff;
	dma_addr_t bmi_rsp_da;
	/* length of last response */
	uint32_t last_rxlen;
	OS_DMA_MEM_CONTEXT(bmirsp_dmacontext)

	void *msi_magic;
	dma_addr_t msi_magic_da;
	OS_DMA_MEM_CONTEXT(msi_dmacontext)

	/* Handles for Lower Layers : filled in at init time */
	hif_handle_t hif_hdl;
#ifdef HIF_PCI
	struct hif_pci_softc *hif_sc;
#endif

#ifdef WLAN_FEATURE_FASTPATH
	int fastpath_mode_on; /* Duplicating this for data path efficiency */
#endif /* WLAN_FEATURE_FASTPATH */

	/* HTC handles */
	void *htc_handle;

	bool fEnableBeaconEarlyTermination;
	uint8_t bcnEarlyTermWakeInterval;

	/* UTF event information */
	struct {
		uint8_t *data;
		uint32_t length;
		cdf_size_t offset;
		uint8_t currentSeq;
		uint8_t expectedSeq;
	} utf_event_info;

	struct ol_wow_info *scn_wowInfo;
	/* enable uart/serial prints from target */
	bool enableuartprint;
	/* enable fwlog */
	bool enablefwlog;

	HAL_REG_CAPABILITIES hal_reg_capabilities;
	struct ol_regdmn *ol_regdmn_handle;
	uint8_t bcn_mode;
	uint8_t arp_override;
	/*
	 * Includes host side stack level stats +
	 * radio level athstats
	 */
	struct wlan_dbg_stats ath_stats;
	/* noise_floor */
	int16_t chan_nf;
	uint32_t min_tx_power;
	uint32_t max_tx_power;
	uint32_t txpowlimit2G;
	uint32_t txpowlimit5G;
	uint32_t txpower_scale;
	uint32_t chan_tx_pwr;
	uint32_t vdev_count;
	uint32_t max_bcn_ie_size;
	cdf_spinlock_t scn_lock;
	uint8_t vow_extstats;
	/* if dcs enabled or not */
	uint8_t scn_dcs;
	wdi_event_subscribe scn_rx_peer_invalid_subscriber;
	uint8_t proxy_sta;
	uint8_t bcn_enabled;
	/* Dynamic Tx Chainmask Selection enabled/disabled */
	uint8_t dtcs;
	/* true if vht ies are set on target */
	uint32_t set_ht_vht_ies:1;
	/*CWM enable/disable state */
	bool scn_cwmenable;
	uint8_t max_no_of_peers;
#ifdef CONFIG_CNSS
	struct cnss_fw_files fw_files;
#endif
#if defined(CONFIG_CNSS)
	void *ramdump_base;
	unsigned long ramdump_address;
	unsigned long ramdump_size;
#endif
	bool enable_self_recovery;
#ifdef WLAN_FEATURE_LPSS
	bool enablelpasssupport;
#endif
	bool enable_ramdump_collection;
	struct targetdef_s *targetdef;
	struct ce_reg_def *target_ce_def;
	struct hostdef_s *hostdef;
	struct host_shadow_regs_s *host_shadow_regs;
	bool athdiag_procfs_inited;
	/*
	 * Guard changes to Target HW state and to software
	 * structures that track hardware state.
	 */
	cdf_spinlock_t target_lock;
	unsigned int ce_count;  /* Number of Copy Engines supported */
	struct CE_state *ce_id_to_state[CE_COUNT_MAX];  /* CE id to CE_state */
#ifdef FEATURE_NAPI
	struct qca_napi_data napi_data;
#endif /* FEATURE_NAPI */
	int htc_endpoint;
	bool recovery;
	bool hif_init_done;
	int linkstate_vote;
	atomic_t link_suspended;
	atomic_t wow_done;
	atomic_t tasklet_from_intr;
	atomic_t active_tasklet_cnt;
	bool notice_send;
#ifdef HIF_PCI
	cdf_spinlock_t irq_lock;
	uint32_t ce_irq_summary;
#endif
	uint32_t *vaddr_rri_on_ddr;
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

#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
typedef struct _HID_ACCESS_LOG {
	uint32_t seqnum;
	bool is_write;
	void *addr;
	uint32_t value;
} HIF_ACCESS_LOG;
#endif

#define HIF_MAX_DEVICES                 1

struct htc_callbacks {
	void *context;		/* context to pass to the dsrhandler
				 * note : rwCompletionHandler is provided
				 * the context passed to hif_read_write  */
	int (*rwCompletionHandler)(void *rwContext, int status);
	int (*dsrHandler)(void *context);
};

typedef struct osdrv_callbacks {
	void *context;          /* context to pass for all callbacks
				 * except deviceRemovedHandler
				 * the deviceRemovedHandler is only
				 * called if the device is claimed */
	int (*deviceInsertedHandler)(void *context, void *hif_handle);
	int (*deviceRemovedHandler)(void *claimedContext,
				    void *hif_handle);
	int (*deviceSuspendHandler)(void *context);
	int (*deviceResumeHandler)(void *context);
	int (*deviceWakeupHandler)(void *context);
	int (*devicePowerChangeHandler)(void *context,
					HIF_DEVICE_POWER_CHANGE_TYPE
					config);
} OSDRV_CALLBACKS;

/*
 * This API is used to perform any global initialization of the HIF layer
 * and to set OS driver callbacks (i.e. insertion/removal) to the HIF layer
 *
 */
int hif_init(OSDRV_CALLBACKS *callbacks);

/*
 * This API claims the HIF device and provides a context for handling removal.
 * The device removal callback is only called when the OSDRV layer claims
 * a device.  The claimed context must be non-NULL */
void hif_claim_device(struct ol_softc *scn, void *claimedContext);
/* release the claimed device */
void hif_release_device(struct ol_softc *scn);

/* This API detaches the HTC layer from the HIF device */
void hif_detach_htc(struct ol_softc *scn);

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
CDF_STATUS hif_exchange_bmi_msg(struct ol_softc *scn,
			 uint8_t *pSendMessage,
			 uint32_t Length,
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
CDF_STATUS hif_diag_read_access(struct ol_softc *scn, uint32_t address,
			 uint32_t *data);
CDF_STATUS hif_diag_read_mem(struct ol_softc *scn, uint32_t address,
		      uint8_t *data, int nbytes);
void hif_dump_target_memory(struct ol_softc *scn, void *ramdump_base,
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
CDF_STATUS hif_diag_write_access(struct ol_softc *scn, uint32_t address,
			  uint32_t data);
CDF_STATUS hif_diag_write_mem(struct ol_softc *scn, uint32_t address,
		       uint8_t *data, int nbytes);

/*
 * Set the FASTPATH_mode_on flag in sc, for use by data path
 */
#ifdef WLAN_FEATURE_FASTPATH
void hif_enable_fastpath(struct ol_softc *hif_dev);
#endif

#if defined(HIF_PCI) && !defined(A_SIMOS_DEVHOST)
/*
 * This API allows the Host to access Target registers of a given
 * A_target_id_t directly and relatively efficiently over PCIe.
 * This allows the Host to avoid extra overhead associated with
 * sending a message to firmware and waiting for a response message
 * from firmware, as is done on other interconnects.
 *
 * Yet there is some complexity with direct accesses because the
 * Target's power state is not known a priori. The Host must issue
 * special PCIe reads/writes in order to explicitly wake the Target
 * and to verify that it is awake and will remain awake.
 *
 * NB: Host endianness conversion is left for the caller to handle.
 *     These interfaces handle access; not interpretation.
 *
 * Usage:
 *   During initialization, use A_TARGET_ID to obtain an 'target ID'
 *   for use with these interfaces.
 *
 *   Use A_TARGET_READ and A_TARGET_WRITE to access Target space.
 *   These calls must be bracketed by A_TARGET_ACCESS_BEGIN and
 *   A_TARGET_ACCESS_END.  A single BEGIN/END pair is adequate for
 *   multiple READ/WRITE operations.
 *
 *   Use A_TARGET_ACCESS_BEGIN to put the Target in a state in
 *   which it is legal for the Host to directly access it. This
 *   may involve waking the Target from a low power state, which
 *   may take up to 2Ms!
 *
 *   Use A_TARGET_ACCESS_END to tell the Target that as far as
 *   this code path is concerned, it no longer needs to remain
 *   directly accessible.  BEGIN/END is under a reference counter;
 *   multiple code paths may issue BEGIN/END on a single targid.
 *
 *   For added efficiency, the Host may use A_TARGET_ACCESS_LIKELY.
 *   The LIKELY interface works just like A_TARGET_ACCESS_BEGIN,
 *   except that it may return before the Target is actually
 *   available. It's a vague indication that some Target accesses
 *   are expected "soon".  When the LIKELY API is used,
 *   A_TARGET_ACCESS_BEGIN must be used before any access.
 *
 *   There are several uses for the LIKELY/UNLIKELY API:
 *     -If there is some potential time before Target accesses
 *      and we want to get a head start on waking the Target
 *      (e.g. to overlap Target wake with Host-side malloc)
 *     -High-level code knows that it will call low-level
 *      functions that will use BEGIN/END, and we don't want
 *      to allow the Target to sleep until the entire sequence
 *      has completed.
 *
 *   A_TARGET_ACCESS_OK verifies that the Target can be
 *   accessed. In general, this should not be needed, but it
 *   may be useful for debugging or for special uses.
 *
 *   Note that there must be a matching END for each BEGIN
 *       AND   there must be a matching UNLIKELY for each LIKELY!
 *
 *   NB: This API is designed to allow some flexibility in tradeoffs
 *   between Target power utilization and Host efficiency and
 *   system performance.
 */

/*
 * Enable/disable CDC max performance workaround
 * For max-performace set this to 0
 * To allow SoC to enter sleep set this to 1
 */
#define CONFIG_DISABLE_CDC_MAX_PERF_WAR 0
#endif

#ifdef IPA_OFFLOAD
/*
 * IPA micro controller data path offload feature enabled,
 * HIF should release copy engine related resource information to IPA UC
 * IPA UC will access hardware resource with released information
 */
void hif_ipa_get_ce_resource(struct ol_softc *scn,
			     uint32_t *ce_sr_base_paddr,
			     uint32_t *ce_sr_ring_size,
			     cdf_dma_addr_t *ce_reg_paddr);
#else
static inline void hif_ipa_get_ce_resource(struct ol_softc *scn,
			     uint32_t *ce_sr_base_paddr,
			     uint32_t *ce_sr_ring_size,
			     cdf_dma_addr_t *ce_reg_paddr)
{
	return;
}
#endif /* IPA_OFFLOAD */


void hif_read_phy_mem_base(struct ol_softc *scn,
	cdf_dma_addr_t *bar_value);

/**
 * @brief List of callbacks - filled in by HTC.
 */
struct hif_msg_callbacks {
	void *Context;
	/**< context meaningful to HTC */
	CDF_STATUS (*txCompletionHandler)(void *Context, cdf_nbuf_t wbuf,
					uint32_t transferID,
					uint32_t toeplitz_hash_result);
	CDF_STATUS (*rxCompletionHandler)(void *Context, cdf_nbuf_t wbuf,
					uint8_t pipeID);
	void (*txResourceAvailHandler)(void *context, uint8_t pipe);
	void (*fwEventHandler)(void *context, CDF_STATUS status);
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

#ifdef HIF_PCI
typedef struct pci_device_id hif_bus_id;
#else
typedef struct device hif_bus_id;
#endif

void hif_post_init(struct ol_softc *scn, void *hHTC,
		   struct hif_msg_callbacks *callbacks);
CDF_STATUS hif_start(struct ol_softc *scn);
void hif_stop(struct ol_softc *scn);
void hif_flush_surprise_remove(struct ol_softc *scn);
void hif_dump(struct ol_softc *scn, uint8_t CmdId, bool start);
CDF_STATUS hif_send_head(struct ol_softc *scn, uint8_t PipeID,
				  uint32_t transferID, uint32_t nbytes,
				  cdf_nbuf_t wbuf, uint32_t data_attr);
void hif_send_complete_check(struct ol_softc *scn, uint8_t PipeID,
			     int force);
void hif_cancel_deferred_target_sleep(struct ol_softc *scn);
void hif_get_default_pipe(struct ol_softc *scn, uint8_t *ULPipe,
			  uint8_t *DLPipe);
int hif_map_service_to_pipe(struct ol_softc *scn, uint16_t svc_id,
			uint8_t *ul_pipe, uint8_t *dl_pipe, int *ul_is_polled,
			int *dl_is_polled);
uint16_t hif_get_free_queue_number(struct ol_softc *scn, uint8_t PipeID);
void *hif_get_targetdef(struct ol_softc *scn);
void hi_fsuspendwow(struct ol_softc *scn);
uint32_t hif_hia_item_address(uint32_t target_type, uint32_t item_offset);
void hif_set_target_sleep(struct ol_softc *scn, bool sleep_ok,
		     bool wait_for_it);
int hif_check_fw_reg(struct ol_softc *scn);
int hif_check_soc_status(struct ol_softc *scn);
void dump_ce_debug_register(struct ol_softc *scn);
void hif_get_hw_info(void *scn, u32 *version, u32 *revision,
		     const char **target_name);
void hif_set_fw_info(void *scn, u32 target_fw_version);
void hif_disable_isr(void *scn);
void hif_reset_soc(void *scn);
void hif_disable_aspm(void);
void hif_save_htc_htt_config_endpoint(int htc_endpoint);
CDF_STATUS hif_open(void);
void hif_close(void *hif_ctx);
CDF_STATUS hif_enable(void *hif_ctx, struct device *dev, void *bdev,
	const hif_bus_id *bid, enum ath_hal_bus_type bus_type,
	enum hif_enable_type type);
void hif_disable(void *hif_ctx, enum hif_disable_type type);
void hif_enable_power_gating(void *hif_ctx);
int hif_bus_resume(void);
int hif_bus_suspend(void);
void hif_vote_link_down(void);
void hif_vote_link_up(void);
bool hif_can_suspend_link(void);
int dump_ce_register(struct ol_softc *scn);
int ol_copy_ramdump(struct ol_softc *scn);
void hif_pktlogmod_exit(void *hif_ctx);
void hif_crash_shutdown(void *hif_ctx);
#ifdef __cplusplus
}
#endif
#endif /* _HIF_H_ */
