/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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
 * NB: Inappropriate references to "HTC" are used in this (and other)
 * HIF implementations.  HTC is typically the calling layer, but it
 * theoretically could be some alternative.
 */

/*
 * This holds all state needed to process a pending send/recv interrupt.
 * The information is saved here as soon as the interrupt occurs (thus
 * allowing the underlying CE to re-use the ring descriptor). The
 * information here is eventually processed by a completion processing
 * thread.
 */

#ifndef __HIF_MAIN_H__
#define __HIF_MAIN_H__

#include <qdf_atomic.h>         /* qdf_atomic_read */
#include "qdf_lock.h"
#include "cepci.h"
#include "hif.h"
#include "multibus.h"
#include "hif_unit_test_suspend_i.h"

#define HIF_MIN_SLEEP_INACTIVITY_TIME_MS     50
#define HIF_SLEEP_INACTIVITY_TIMER_PERIOD_MS 60

#define HIF_MAX_BUDGET 0xFFFF

/*
 * This macro implementation is exposed for efficiency only.
 * The implementation may change and callers should
 * consider the targid to be a completely opaque handle.
 */
#define TARGID_TO_PCI_ADDR(targid) (*((A_target_id_t *)(targid)))

#ifdef QCA_WIFI_3_0
#define DISABLE_L1SS_STATES 1
#endif

#define MAX_NUM_OF_RECEIVES HIF_NAPI_MAX_RECEIVES

#ifdef QCA_WIFI_3_0_ADRASTEA
#define ADRASTEA_BU 1
#else
#define ADRASTEA_BU 0
#endif

#ifdef QCA_WIFI_3_0
#define HAS_FW_INDICATOR 0
#else
#define HAS_FW_INDICATOR 1
#endif


#define AR9888_DEVICE_ID (0x003c)
#define AR6320_DEVICE_ID (0x003e)
#define AR6320_FW_1_1  (0x11)
#define AR6320_FW_1_3  (0x13)
#define AR6320_FW_2_0  (0x20)
#define AR6320_FW_3_0  (0x30)
#define AR6320_FW_3_2  (0x32)
#define QCA6290_EMULATION_DEVICE_ID (0xabcd)
#define QCA6290_DEVICE_ID (0x1100)
#define QCA6390_EMULATION_DEVICE_ID (0x0108)
#define QCA6390_DEVICE_ID (0x1101)
#define ADRASTEA_DEVICE_ID_P2_E12 (0x7021)
#define AR9887_DEVICE_ID    (0x0050)
#define AR900B_DEVICE_ID    (0x0040)
#define QCA9984_DEVICE_ID   (0x0046)
#define QCA9888_DEVICE_ID   (0x0056)
#ifndef IPQ4019_DEVICE_ID
#define IPQ4019_DEVICE_ID   (0x12ef)
#endif
#define QCA8074_DEVICE_ID   (0xffff) /* Todo: replace this with
					actual number once available.
					currently defining this to 0xffff for
					emulation purpose */
#define QCA8074V2_DEVICE_ID (0xfffe) /* Todo: replace this with actual number */
#define QCA6018_DEVICE_ID (0xfffd) /* Todo: replace this with actual number */
/* Genoa */
#define QCN7605_DEVICE_ID  (0x1102) /* Genoa PCIe device ID*/
#define QCN7605_COMPOSITE  (0x9900)
#define QCN7605_STANDALONE  (0x9901)

#define RUMIM2M_DEVICE_ID_NODE0	0xabc0
#define RUMIM2M_DEVICE_ID_NODE1	0xabc1
#define RUMIM2M_DEVICE_ID_NODE2	0xabc2
#define RUMIM2M_DEVICE_ID_NODE3	0xabc3
#define RUMIM2M_DEVICE_ID_NODE4	0xaa10
#define RUMIM2M_DEVICE_ID_NODE5	0xaa11

#define HIF_GET_PCI_SOFTC(scn) ((struct hif_pci_softc *)scn)
#define HIF_GET_CE_STATE(scn) ((struct HIF_CE_state *)scn)
#define HIF_GET_SDIO_SOFTC(scn) ((struct hif_sdio_softc *)scn)
#define HIF_GET_USB_SOFTC(scn) ((struct hif_usb_softc *)scn)
#define HIF_GET_USB_DEVICE(scn) ((struct HIF_DEVICE_USB *)scn)
#define HIF_GET_SOFTC(scn) ((struct hif_softc *)scn)
#define GET_HIF_OPAQUE_HDL(scn) ((struct hif_opaque_softc *)scn)

struct hif_ce_stats {
	int hif_pipe_no_resrc_count;
	int ce_ring_delta_fail_count;
};

/*
 * Note: For MCL, #if defined (HIF_CONFIG_SLUB_DEBUG_ON) needs to be checked
 * for defined here
 */
#if defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF)
struct ce_desc_hist {
	qdf_atomic_t history_index[CE_COUNT_MAX];
	uint32_t enable[CE_COUNT_MAX];
	bool data_enable[CE_COUNT_MAX];
	qdf_mutex_t ce_dbg_datamem_lock[CE_COUNT_MAX];
	uint32_t hist_index;
	uint32_t hist_id;
	void *hist_ev[CE_COUNT_MAX];
};
#endif /*defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF)*/

struct hif_softc {
	struct hif_opaque_softc osc;
	struct hif_config_info hif_config;
	struct hif_target_info target_info;
	void __iomem *mem;
	enum qdf_bus_type bus_type;
	struct hif_bus_ops bus_ops;
	void *ce_id_to_state[CE_COUNT_MAX];
	qdf_device_t qdf_dev;
	bool hif_init_done;
	bool request_irq_done;
	bool ext_grp_irq_configured;
	/* Packet statistics */
	struct hif_ce_stats pkt_stats;
	enum hif_target_status target_status;
	uint64_t event_disable_mask;

	struct targetdef_s *targetdef;
	struct ce_reg_def *target_ce_def;
	struct hostdef_s *hostdef;
	struct host_shadow_regs_s *host_shadow_regs;

	bool recovery;
	bool notice_send;
	bool per_ce_irq;
	uint32_t ce_irq_summary;
	/* No of copy engines supported */
	unsigned int ce_count;
	atomic_t active_tasklet_cnt;
	atomic_t active_grp_tasklet_cnt;
	atomic_t link_suspended;
	uint32_t *vaddr_rri_on_ddr;
	qdf_dma_addr_t paddr_rri_on_ddr;
	int linkstate_vote;
	bool fastpath_mode_on;
	atomic_t tasklet_from_intr;
	int htc_htt_tx_endpoint;
	qdf_dma_addr_t mem_pa;
	bool athdiag_procfs_inited;
#ifdef FEATURE_NAPI
	struct qca_napi_data napi_data;
#endif /* FEATURE_NAPI */
	/* stores ce_service_max_yield_time in ns */
	unsigned long long ce_service_max_yield_time;
	uint8_t ce_service_max_rx_ind_flush;
	struct hif_driver_state_callbacks callbacks;
	uint32_t hif_con_param;
#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
	uint32_t nss_wifi_ol_mode;
#endif
	void *hal_soc;
	struct hif_ut_suspend_context ut_suspend_ctx;
	uint32_t hif_attribute;
	int wake_irq;
	void (*initial_wakeup_cb)(void *);
	void *initial_wakeup_priv;
#ifdef REMOVE_PKT_LOG
	/* Handle to pktlog device */
	void *pktlog_dev;
#endif

/*
 * Note: For MCL, #if defined (HIF_CONFIG_SLUB_DEBUG_ON) needs to be checked
 * for defined here
 */
#if defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF)
	struct ce_desc_hist hif_ce_desc_hist;
#endif /*defined(HIF_CONFIG_SLUB_DEBUG_ON) || defined(HIF_CE_DEBUG_DATA_BUF)*/

#ifdef IPA_OFFLOAD
	qdf_shared_mem_t *ipa_ce_ring;
#endif
};

static inline void *hif_get_hal_handle(void *hif_hdl)
{
	struct hif_softc *sc = (struct hif_softc *)hif_hdl;

	if (!sc)
		return NULL;

	return sc->hal_soc;
}

#ifdef QCA_NSS_WIFI_OFFLOAD_SUPPORT
static inline bool hif_is_nss_wifi_enabled(struct hif_softc *sc)
{
	return !!(sc->nss_wifi_ol_mode);
}
#else
static inline bool hif_is_nss_wifi_enabled(struct hif_softc *sc)
{
	return false;
}
#endif

static inline uint8_t hif_is_attribute_set(struct hif_softc *sc,
						uint32_t hif_attrib)
{
	return sc->hif_attribute == hif_attrib;
}

#ifdef WLAN_FEATURE_DP_EVENT_HISTORY
static inline void hif_set_event_hist_mask(struct hif_opaque_softc *hif_handle)
{
	struct hif_softc *scn = (struct hif_softc *)hif_handle;

	scn->event_disable_mask = HIF_EVENT_HIST_DISABLE_MASK;
}
#else
static inline void hif_set_event_hist_mask(struct hif_opaque_softc *hif_handle)
{
}
#endif /* WLAN_FEATURE_DP_EVENT_HISTORY */

A_target_id_t hif_get_target_id(struct hif_softc *scn);
void hif_dump_pipe_debug_count(struct hif_softc *scn);
void hif_display_bus_stats(struct hif_opaque_softc *scn);
void hif_clear_bus_stats(struct hif_opaque_softc *scn);
bool hif_max_num_receives_reached(struct hif_softc *scn, unsigned int count);
void hif_shutdown_device(struct hif_opaque_softc *hif_ctx);
int hif_bus_configure(struct hif_softc *scn);
void hif_cancel_deferred_target_sleep(struct hif_softc *scn);
int hif_config_ce(struct hif_softc *scn);
void hif_unconfig_ce(struct hif_softc *scn);
void hif_ce_prepare_config(struct hif_softc *scn);
QDF_STATUS hif_ce_open(struct hif_softc *scn);
void hif_ce_close(struct hif_softc *scn);
int athdiag_procfs_init(void *scn);
void athdiag_procfs_remove(void);
/* routine to modify the initial buffer count to be allocated on an os
 * platform basis. Platform owner will need to modify this as needed
 */
qdf_size_t init_buffer_count(qdf_size_t maxSize);

irqreturn_t hif_fw_interrupt_handler(int irq, void *arg);
int hif_get_device_type(uint32_t device_id,
			uint32_t revision_id,
			uint32_t *hif_type, uint32_t *target_type);
/*These functions are exposed to HDD*/
void hif_nointrs(struct hif_softc *scn);
void hif_bus_close(struct hif_softc *ol_sc);
QDF_STATUS hif_bus_open(struct hif_softc *ol_sc,
	enum qdf_bus_type bus_type);
QDF_STATUS hif_enable_bus(struct hif_softc *ol_sc, struct device *dev,
	void *bdev, const struct hif_bus_id *bid, enum hif_enable_type type);
void hif_disable_bus(struct hif_softc *scn);
void hif_bus_prevent_linkdown(struct hif_softc *scn, bool flag);
int hif_bus_get_context_size(enum qdf_bus_type bus_type);
void hif_read_phy_mem_base(struct hif_softc *scn, qdf_dma_addr_t *bar_value);
uint32_t hif_get_conparam(struct hif_softc *scn);
struct hif_driver_state_callbacks *hif_get_callbacks_handle(
							struct hif_softc *scn);
bool hif_is_driver_unloading(struct hif_softc *scn);
bool hif_is_load_or_unload_in_progress(struct hif_softc *scn);
bool hif_is_recovery_in_progress(struct hif_softc *scn);
bool hif_is_target_ready(struct hif_softc *scn);
void hif_wlan_disable(struct hif_softc *scn);
int hif_target_sleep_state_adjust(struct hif_softc *scn,
					 bool sleep_ok,
					 bool wait_for_it);
/**
 * hif_get_rx_ctx_id() - Returns NAPI instance ID based on CE ID
 * @ctx_id: Rx CE context ID
 * @hif_hdl: HIF Context
 *
 * Return: Rx instance ID
 */
int hif_get_rx_ctx_id(int ctx_id, struct hif_opaque_softc *hif_hdl);
void hif_ramdump_handler(struct hif_opaque_softc *scn);
#ifdef HIF_USB
void hif_usb_get_hw_info(struct hif_softc *scn);
void hif_usb_ramdump_handler(struct hif_opaque_softc *scn);
#else
static inline void hif_usb_get_hw_info(struct hif_softc *scn) {}
static inline void hif_usb_ramdump_handler(struct hif_opaque_softc *scn) {}
#endif

/**
 * hif_wake_interrupt_handler() - interrupt handler for standalone wake irq
 * @irq: the irq number that fired
 * @context: the opaque pointer passed to request_irq()
 *
 * Return: an irq return type
 */
irqreturn_t hif_wake_interrupt_handler(int irq, void *context);

#ifdef HIF_SNOC
bool hif_is_target_register_access_allowed(struct hif_softc *hif_sc);
#else
static inline
bool hif_is_target_register_access_allowed(struct hif_softc *hif_sc)
{
	return true;
}
#endif

#ifdef ADRASTEA_RRI_ON_DDR
void hif_uninit_rri_on_ddr(struct hif_softc *scn);
#else
static inline
void hif_uninit_rri_on_ddr(struct hif_softc *scn) {}
#endif
#endif /* __HIF_MAIN_H__ */
