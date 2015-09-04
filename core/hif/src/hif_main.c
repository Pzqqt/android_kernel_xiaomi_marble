/*
 * Copyright (c) 2015 The Linux Foundation. All rights reserved.
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

#include <osdep.h>
#include "a_types.h"
#include "athdefs.h"
#include "osapi_linux.h"
#include "targcfg.h"
#include "cdf_lock.h"
#include "cdf_status.h"
#include <cdf_atomic.h>         /* cdf_atomic_read */
#include <targaddrs.h>
#include <bmi_msg.h>
#include "hif_io32.h"
#include <hif.h>
#include <htc_services.h>
#include "regtable.h"
#define ATH_MODULE_NAME hif
#include <a_debug.h>
#include "hif_main.h"
#include "hif_hw_version.h"
#include "ce_api.h"
#include "ce_tasklet.h"
#include "cdf_trace.h"
#include "cdf_status.h"
#include "cds_api.h"
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#endif
#include "epping_main.h"
#include "hif_debug.h"
#include "mp_dev.h"
#ifdef HIF_PCI
#include "icnss_stub.h"
#else
#include <soc/qcom/icnss.h>
#endif

#ifndef REMOVE_PKT_LOG
#include "pktlog_ac.h"
#endif

#include "cds_concurrency.h"

#define AGC_DUMP         1
#define CHANINFO_DUMP    2
#define BB_WATCHDOG_DUMP 3
#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
#define PCIE_ACCESS_DUMP 4
#endif

void hif_dump(struct ol_softc *scn, uint8_t cmd_id, bool start)
{
	switch (cmd_id) {
	case AGC_DUMP:
		if (start)
			priv_start_agc(scn);
		else
			priv_dump_agc(scn);
		break;

	case CHANINFO_DUMP:
		if (start)
			priv_start_cap_chaninfo(scn);
		else
			priv_dump_chaninfo(scn);
		break;

	case BB_WATCHDOG_DUMP:
		priv_dump_bbwatchdog(scn);
		break;

#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
	case PCIE_ACCESS_DUMP:
		hif_target_dump_access_log();
		break;
#endif
	default:
		HIF_ERROR("%s: Invalid htc dump command", __func__);
		break;
	}
}

/**
 * hif_shut_down_device() - hif_shut_down_device
 *
 * SThis fucntion shuts down the device
 *
 * @scn: ol_softc
 *
 * Return: void
 */
void hif_shut_down_device(struct ol_softc *scn)
{
	if (scn && scn->hif_hdl) {
		struct HIF_CE_state *hif_state =
			(struct HIF_CE_state *)scn->hif_hdl;

		hif_stop(scn);
		cdf_mem_free(hif_state);
		scn->hif_hdl = NULL;
	}

}



/**
 * hif_cancel_deferred_target_sleep() - cancel deferred target sleep
 *
 * This function cancels the defered target sleep
 *
 * @scn: ol_softc
 *
 * Return: void
 */
void hif_cancel_deferred_target_sleep(struct ol_softc *scn)
{
	hif_pci_cancel_deferred_target_sleep(scn);
}

/**
 * hif_get_target_id(): hif_get_target_id
 *
 * Return the virtual memory base address to the caller
 *
 * @scn: ol_softc
 *
 * Return: A_target_id_t
 */
A_target_id_t hif_get_target_id(struct ol_softc *scn)
{
	return scn->mem;
}

/**
 * hif_set_target_sleep(): hif_set_target_sleep
 * @scn: scn
 * @sleep_ok: sleep_ok
 * @wait_for_it: wait
 *
 * Return: void
 */
void hif_set_target_sleep(struct ol_softc *scn,
		     bool sleep_ok, bool wait_for_it)
{
	hif_target_sleep_state_adjust(scn,
				      sleep_ok, wait_for_it);
}

/**
 * hif_target_forced_awake(): hif_target_forced_awake
 * @scn: scn
 *
 * Return: bool
 */
bool hif_target_forced_awake(struct ol_softc *scn)
{
	A_target_id_t addr = scn->mem;
	bool awake;
	bool forced_awake;

	awake = hif_targ_is_awake(scn, addr);

	forced_awake =
		!!(hif_read32_mb
			   (addr + PCIE_LOCAL_BASE_ADDRESS +
			   PCIE_SOC_WAKE_ADDRESS) & PCIE_SOC_WAKE_V_MASK);

	return awake && forced_awake;
}


static inline void hif_fw_event_handler(struct HIF_CE_state *hif_state)
{
	struct hif_msg_callbacks *msg_callbacks =
		&hif_state->msg_callbacks_current;

	if (!msg_callbacks->fwEventHandler)
		return;

	msg_callbacks->fwEventHandler(msg_callbacks->Context,
			CDF_STATUS_E_FAILURE);
}

/**
 * hif_fw_interrupt_handler(): FW interrupt handler
 *
 * This function is the FW interrupt handlder
 *
 * @irq: irq number
 * @arg: the user pointer
 *
 * Return: bool
 */
#ifndef QCA_WIFI_3_0
irqreturn_t hif_fw_interrupt_handler(int irq, void *arg)
{
	struct ol_softc *scn = arg;
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	uint32_t fw_indicator_address, fw_indicator;

	A_TARGET_ACCESS_BEGIN_RET(scn);

	fw_indicator_address = hif_state->fw_indicator_address;
	/* For sudden unplug this will return ~0 */
	fw_indicator = A_TARGET_READ(scn, fw_indicator_address);

	if ((fw_indicator != ~0) && (fw_indicator & FW_IND_EVENT_PENDING)) {
		/* ACK: clear Target-side pending event */
		A_TARGET_WRITE(scn, fw_indicator_address,
			       fw_indicator & ~FW_IND_EVENT_PENDING);
		A_TARGET_ACCESS_END_RET(scn);

		if (hif_state->started) {
			hif_fw_event_handler(hif_state);
		} else {
			/*
			 * Probable Target failure before we're prepared
			 * to handle it.  Generally unexpected.
			 */
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s: Early firmware event indicated\n",
				 __func__));
		}
	} else {
		A_TARGET_ACCESS_END_RET(scn);
	}

	return ATH_ISR_SCHED;
}
#else
irqreturn_t hif_fw_interrupt_handler(int irq, void *arg)
{
	return ATH_ISR_SCHED;
}
#endif /* #ifdef QCA_WIFI_3_0 */

/**
 * hif_get_targetdef(): hif_get_targetdef
 * @scn: scn
 *
 * Return: void *
 */
void *hif_get_targetdef(struct ol_softc *scn)
{
	return scn->targetdef;
}

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
void hif_vote_link_down(void)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);
	CDF_BUG(scn);

	scn->linkstate_vote--;
	if (scn->linkstate_vote == 0)
		hif_bus_prevent_linkdown(false);
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
void hif_vote_link_up(void)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);
	CDF_BUG(scn);

	scn->linkstate_vote++;
	if (scn->linkstate_vote == 1)
		hif_bus_prevent_linkdown(true);
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
bool hif_can_suspend_link(void)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);
	CDF_BUG(scn);

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
	case TARGET_TYPE_QCA6180:
		return QCA6180_HOST_INTEREST_ADDRESS + item_offset;
	case TARGET_TYPE_ADRASTEA:
		/* ADRASTEA doesn't have a host interest address */
		ASSERT(0);
		return 0;
	default:
		ASSERT(0);
		return 0;
	}
}

/**
 * hif_max_num_receives_reached() - check max receive is reached
 * @count: unsigned int.
 *
 * Output check status as bool
 *
 * Return: bool
 */
bool hif_max_num_receives_reached(unsigned int count)
{
	if (WLAN_IS_EPPING_ENABLED(cds_get_conparam()))
		return count > 120;
	else
		return count > MAX_NUM_OF_RECEIVES;
}

/**
 * init_buffer_count() - initial buffer count
 * @maxSize: cdf_size_t
 *
 * routine to modify the initial buffer count to be allocated on an os
 * platform basis. Platform owner will need to modify this as needed
 *
 * Return: cdf_size_t
 */
cdf_size_t init_buffer_count(cdf_size_t maxSize)
{
	return maxSize;
}

/**
 * hif_init_cdf_ctx(): hif_init_cdf_ctx
 * @hif_ctx: hif_ctx
 *
 * Return: int
 */
int hif_init_cdf_ctx(void *hif_ctx)
{
	cdf_device_t cdf_ctx;
	struct ol_softc *scn = (struct ol_softc *)hif_ctx;

	cdf_ctx = cds_get_context(CDF_MODULE_ID_CDF_DEVICE);
	if (!cdf_ctx) {
		HIF_ERROR("%s: CDF is NULL", __func__);
		return -ENOMEM;
	}

	cdf_ctx->drv = &scn->aps_osdev;
	cdf_ctx->drv_hdl = scn->aps_osdev.bdev;
	cdf_ctx->dev = scn->aps_osdev.device;
	scn->cdf_dev = cdf_ctx;
	return 0;
}

/**
 * hif_deinit_cdf_ctx(): hif_deinit_cdf_ctx
 * @hif_ctx: hif_ctx
 *
 * Return: void
 */
void hif_deinit_cdf_ctx(void *hif_ctx)
{
	struct ol_softc *scn = (struct ol_softc *)hif_ctx;

	if (scn == NULL || !scn->cdf_dev)
		return;
	scn->cdf_dev = NULL;
}

/**
 * hif_save_htc_htt_config_endpoint():
 * hif_save_htc_htt_config_endpoint
 * @htc_endpoint: htc_endpoint
 *
 * Return: void
 */
void hif_save_htc_htt_config_endpoint(int htc_endpoint)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);

	if (!scn) {
		HIF_ERROR("%s: error: scn or scn->hif_sc is NULL!",
		       __func__);
		return;
	}

	scn->htc_endpoint = htc_endpoint;
}

/**
 * hif_get_hw_name(): get a human readable name for the hardware
 *
 * Return: human readible name for the underlying wifi hardware.
 */
const char *hif_get_hw_name(struct ol_softc *scn)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(qwlan_hw_list); i++) {
		if (scn->target_version == qwlan_hw_list[i].id &&
		    scn->target_revision == qwlan_hw_list[i].subid) {
			return qwlan_hw_list[i].name;
		}
	}

	return "Unknown Device";
}

/**
 * hif_get_hw_info(): hif_get_hw_info
 * @scn: scn
 * @version: version
 * @revision: revision
 *
 * Return: n/a
 */
void hif_get_hw_info(void *scn, u32 *version, u32 *revision,
			const char **target_name)
{
	*version = ((struct ol_softc *)scn)->target_version;
	*revision = ((struct ol_softc *)scn)->target_revision;
	*target_name = hif_get_hw_name((struct ol_softc *)scn);
}

/**
 * hif_set_fw_info(): set the target_fw_version
 * @scn: scn
 * @target_fw_version: target_fw_version
 *
 * Return: n/a
 */
void hif_set_fw_info(void *scn, uint32_t target_fw_version)
{
	((struct ol_softc *)scn)->target_fw_version = target_fw_version;
}

/**
 * hif_open(): hif_open
 *
 * Return: scn
 */
CDF_STATUS hif_open(void)
{
	struct ol_softc *scn;
	v_CONTEXT_t cds_context;
	CDF_STATUS status = CDF_STATUS_SUCCESS;

	cds_context = cds_get_global_context();
	status = cds_alloc_context(cds_context, CDF_MODULE_ID_HIF,
				(void **)&scn, sizeof(*scn));
	if (status != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: cannot alloc ol_sc", __func__);
		return status;
	}

	cdf_mem_zero(scn, sizeof(*scn));
	scn->enableuartprint = 0;
	scn->enablefwlog = 0;
	scn->max_no_of_peers = 1;
	scn->pkt_log_init = false;
	cdf_atomic_init(&scn->wow_done);
	cdf_atomic_init(&scn->active_tasklet_cnt);
	cdf_atomic_init(&scn->link_suspended);
	cdf_atomic_init(&scn->tasklet_from_intr);
	init_waitqueue_head(&scn->aps_osdev.event_queue);
	cdf_spinlock_init(&scn->target_lock);
	scn->linkstate_vote = 0;
	return status;
}

/**
 * hif_close(): hif_close
 * @hif_ctx: hif_ctx
 *
 * Return: n/a
 */
void hif_close(void *hif_ctx)
{
	struct ol_softc *scn = hif_ctx;

	if (scn == NULL) {
		HIF_ERROR("%s: ol_softc is NULL", __func__);
		return;
	}

	if (scn->athdiag_procfs_inited) {
		athdiag_procfs_remove();
		scn->athdiag_procfs_inited = false;
	}

	if (scn->hif_hdl) {
		cdf_mem_free(scn->hif_hdl);
		scn->hif_hdl = NULL;
	}
	hif_bus_close(scn);
	cds_free_context(cds_get_global_context(),
		CDF_MODULE_ID_HIF, hif_ctx);
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
 * Return: CDF_STATUS
 */
CDF_STATUS hif_enable(void *hif_ctx, struct device *dev,
					  void *bdev, const hif_bus_id *bid,
					  enum ath_hal_bus_type bus_type,
					  enum hif_enable_type type)
{
	CDF_STATUS status;
	struct ol_softc *scn = hif_ctx;

	if (scn == NULL) {
		HIF_ERROR("%s: hif_ctx = NULL", __func__);
		return CDF_STATUS_E_NULL_VALUE;
	}

	status = hif_bus_open(scn, bus_type);
	if (status != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s: hif_bus_open error = %d, bus_type = %d",
				  __func__, status, bus_type);
		return status;
	}

	status = hif_enable_bus(scn, dev, bdev, bid, type);
	if (status != CDF_STATUS_SUCCESS) {
		hif_bus_close(scn);
		HIF_ERROR("%s: hif_enable_bus error = %d",
				  __func__, status);
		return status;
	}

	if (ADRASTEA_BU)
		hif_vote_link_up();

	if (hif_config_ce(scn)) {
		HIF_ERROR("%s: Target probe failed.", __func__);
		hif_disable_bus(scn->aps_osdev.bdev);
		hif_bus_close(scn);
		status = CDF_STATUS_E_FAILURE;
		return status;
	}
	/*
	 * Flag to avoid potential unallocated memory access from MSI
	 * interrupt handler which could get scheduled as soon as MSI
	 * is enabled, i.e to take care of the race due to the order
	 * in where MSI is enabled before the memory, that will be
	 * in interrupt handlers, is allocated.
	 */

#ifdef HIF_PCI
	status = hif_configure_irq(scn->hif_sc);
	if (status < 0) {
		HIF_ERROR("%s: ERROR - configure_IRQ_and_CE failed, status = %d",
			   __func__, status);
		return CDF_STATUS_E_FAILURE;
	}
#endif

	scn->hif_init_done = true;

	HIF_TRACE("%s: X OK", __func__);

	return CDF_STATUS_SUCCESS;
}

/**
 * hif_pktlogmod_exit(): hif_pktlogmod_exit
 * @scn: scn
 *
 * Return: n/a
 */
#ifndef REMOVE_PKT_LOG
void hif_pktlogmod_exit(void *hif_ctx)
{
	struct ol_softc *scn = hif_ctx;

	if (scn && cds_get_conparam() != CDF_FTM_MODE &&
	    !WLAN_IS_EPPING_ENABLED(cds_get_conparam()) && scn->pkt_log_init) {
		pktlogmod_exit(scn);
		scn->pkt_log_init = false;
	}
}
#else
void hif_pktlogmod_exit(void *hif_ctx)
{
}
#endif

#if ((!defined(QCA_WIFI_3_0_IHELIUM) && !defined(QCA_WIFI_3_0_ADRASTEA)) || defined(CONFIG_ICNSS))
static inline void cnss_pcie_notify_q6(void)
{
	return;
}
#endif

/**
 * hif_wlan_disable(): call the platform driver to disable wlan
 *
 * This function passes the con_mode to platform driver to disable
 * wlan.
 *
 * Return: void
 */
void hif_wlan_disable(void)
{
	enum icnss_driver_mode mode;
	uint32_t con_mode = cds_get_conparam();

	switch (con_mode) {
	case CDF_FTM_MODE:
		mode = ICNSS_FTM;
		break;
	case CDF_EPPING_MODE:
		mode = ICNSS_EPPING;
		break;
	default:
		mode = ICNSS_MISSION;
		break;
	}

	icnss_wlan_disable(mode);
}

void hif_disable(void *hif_ctx, enum hif_disable_type type)
{
	struct ol_softc *scn = hif_ctx;

	if (!scn)
		return;

	hif_nointrs(scn);
	if (scn->hif_init_done == false)
		hif_shut_down_device(scn);
	else
		hif_stop(scn);

	if (ADRASTEA_BU)
		hif_vote_link_down();

	if (scn->aps_osdev.bdev)
		hif_disable_bus(scn->aps_osdev.bdev);

	if (IHELIUM_BU) {
		cnss_pcie_notify_q6();
		HIF_TRACE("%s: cnss_pcie_notify_q6 done, notice_send= %d",
			  __func__, scn->notice_send);
	}

	hif_wlan_disable();

	scn->notice_send = false;

	HIF_INFO("%s: X", __func__);
}


/**
 * hif_crash_shutdown_dump_ce_register():
 * hif_crash_shutdown_dump_ce_register
 * @hif_ctx: hif_ctx
 *
 * Return: n/a
 */
#if defined(TARGET_RAMDUMP_AFTER_KERNEL_PANIC) \
&& defined(HIF_PCI) && defined(DEBUG)

static void hif_crash_shutdown_dump_ce_register(void *hif_ctx)
{
	struct ol_softc *scn = hif_ctx;

	if (hif_check_soc_status(scn)
	    || dump_ce_register(scn)) {
		return;
	}

	dump_ce_debug_register(scn);
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
void hif_crash_shutdown(void *hif_ctx)
{
	struct ol_softc *scn = hif_ctx;
	struct HIF_CE_state *hif_state;

	if (!scn)
		return;

	hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	if (!hif_state)
		return;


	if (OL_TRGET_STATUS_RESET == scn->target_status) {
		HIF_INFO_MED("%s: Target is already asserted, ignore!",
			    __func__);
		return;
	}

	if (cds_is_load_unload_in_progress()) {
		HIF_ERROR("%s: Load/unload is in progress, ignore!", __func__);
		return;
	}

	hif_crash_shutdown_dump_ce_register(hif_ctx);

	if (ol_copy_ramdump(scn))
		goto out;

	HIF_INFO_MED("%s: RAM dump collecting completed!", __func__);

out:
	return;
}
#else
void hif_crash_shutdown(void *hif_ctx)
{
	HIF_INFO_MED("%s: Collecting target RAM dump disabled",
		__func__);
	return;
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
int hif_check_fw_reg(struct ol_softc *scn)
{
	return 0;
}
#endif

#ifdef IPA_OFFLOAD
/**
 * hif_read_phy_mem_base(): hif_read_phy_mem_base
 * @scn: scn
 * @phy_mem_base: physical mem base
 *
 * Return: n/a
 */
void hif_read_phy_mem_base(struct ol_softc *scn, cdf_dma_addr_t *phy_mem_base)
{
	*phy_mem_base = scn->mem_pa;
}
#endif /* IPA_OFFLOAD */

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
#ifdef QCA_WIFI_3_0_ADRASTEA
	case ADRASTEA_DEVICE_ID:
	case ADRASTEA_DEVICE_ID_P2_E12:

		*hif_type = HIF_TYPE_ADRASTEA;
		*target_type = TARGET_TYPE_ADRASTEA;
		break;
#else
	case QCA6180_DEVICE_ID:
		*hif_type = HIF_TYPE_QCA6180;
		*target_type = TARGET_TYPE_QCA6180;
		break;
#endif

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
			HIF_ERROR("%s: error - dev_id = 0x%x, rev_id = 0x%x",
				   __func__, device_id, revision_id);
			ret = -ENODEV;
			goto end;
		}
		break;

	default:
		HIF_ERROR("%s: Unsupported device ID!", __func__);
		ret = -ENODEV;
		break;
	}
end:
	return ret;
}
