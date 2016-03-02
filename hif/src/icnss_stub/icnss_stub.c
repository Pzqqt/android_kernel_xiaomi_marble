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

#ifdef HIF_PCI

#include "icnss_stub.h"
#include "hif_io32.h"
#include <hif.h>
#include "regtable.h"
#include "hif_debug.h"
#include "cds_api.h"
#include "cdf_status.h"
#include "qwlan_version.h"
#include <net/cnss.h>

static int icnss_get_irq_num(int ce_id);

/**
 * struct icnss_stub_entry
 *
 * @irq_handler: irq_handler
 * @data: data
 * @name: name
 * @ce_id: ce_id
 */
struct icnss_stub_entry {
	irqreturn_t (*irq_handler)(int, void *);
	void *data;
	const char *name;
	int ce_id;
};

/**
 * struct icnss_stub_context
 *
 * @stub: icnss_stub_entry
 * @regged_irq: regged_irq
 */
struct icnss_stub_context {
	struct icnss_stub_entry stub[ICNSS_MAX_IRQ_REGISTRATIONS];
	uint32_t regged_irq;
};

static struct icnss_stub_context cnss_stub;

#ifndef QCA_WIFI_3_0_ADRASTEA
/**
 * icnss_wlan_enable() - icnss_wlan_enable
 * @config: ce configuration information
 * @mode: driver_mode
 * @host_version: version string to send to the fw
 *
 * Return: int
 */
int icnss_wlan_enable(struct icnss_wlan_enable_cfg *config,
	enum icnss_driver_mode mode, const char *host_version)
{
	return 0;
}

/**
 * icnss_wlan_disable() - icnss_wlan_disable
 * @mode: driver_mode
 *
 * Return: int
 */
int icnss_wlan_disable(enum icnss_driver_mode mode)
{
	return 0;
}

/**
 * icnss_set_fw_debug_mode() - icnss_set_fw_debug_mode
 * @mode: fw debug mode, 0 for QXDM, 1 for WMI
 *
 * Return: int
 */
int icnss_set_fw_debug_mode(bool mode)
{
	return 0;
}

#else

/**
 * icnss_wlan_enable(): call the platform driver to enable wlan
 * @config: ce configuration information
 * @mode: driver_mode
 * @host_version: version string to send to the fw
 *
 * This function passes the con_mode and CE configuration to
 * platform driver to enable wlan.
 * cnss_wlan_enable has been hacked to do a qmi handshake with fw.
 * this is not needed for rome.
 *
 * Return: 0 on success, error number otherwise.
 */
int icnss_wlan_enable(struct icnss_wlan_enable_cfg *config,
	enum icnss_driver_mode mode, const char *host_version)
{
	struct cnss_wlan_enable_cfg cfg;
	enum cnss_driver_mode cnss_mode;

	cfg.num_ce_tgt_cfg = config->num_ce_tgt_cfg;
	cfg.ce_tgt_cfg = (struct cnss_ce_tgt_pipe_cfg *)
		config->ce_tgt_cfg;
	cfg.num_ce_svc_pipe_cfg = config->num_ce_svc_pipe_cfg;
	cfg.ce_svc_cfg = (struct cnss_ce_svc_pipe_cfg *)
		config->ce_svc_cfg;

	cfg.num_shadow_reg_cfg = config->num_shadow_reg_cfg;
	cfg.shadow_reg_cfg = (struct cnss_shadow_reg_cfg *)
		config->shadow_reg_cfg;

	switch (mode) {
	case ICNSS_FTM:
		cnss_mode = CNSS_FTM;
		break;
	case ICNSS_EPPING:
		cnss_mode = CNSS_EPPING;
		break;
	default:
		cnss_mode = CNSS_MISSION;
		break;
	}
	return cnss_wlan_enable(&cfg, cnss_mode, host_version);
}

/**
 * icnss_wlan_disable(): call the platform driver to disable wlan
 *
 * This function passes the con_mode to platform driver to disable wlan.
 * cnss_wlan_disable has been hacked to do a qmi handshake with fw.
 * this is not needed for rome.
 *
 * Return: void
 */
int icnss_wlan_disable(enum icnss_driver_mode con_mode)
{
	enum cnss_driver_mode mode;

	switch (con_mode) {
	case ICNSS_FTM:
		mode = CNSS_FTM;
		break;
	case ICNSS_EPPING:
		mode = CNSS_EPPING;
		break;
	default:
		mode = CNSS_MISSION;
		break;
	}

	cnss_wlan_disable(mode);
	return 0;
}

/**
 * icnss_set_fw_debug_mode() - call the platform driver to set fw
 * debug mode
 * @mode: fw debug mode, 0 for QXDM, 1 for WMI
 *
 * This function passes the fw debug mode to platform driver.
 * cnss_set_fw_debug_mode has been hacked to do a qmi handshake with fw.
 * This is not needed for rome.
 *
 * Return: int
 */
int icnss_set_fw_debug_mode(bool mode)
{
	return cnss_set_fw_debug_mode(mode);
}
#endif

/**
 * icnss_ce_request_irq() - register an irq handler
 * @ce_id: ce_id
 * @handler: handler
 * @flags: flags to pass to the kernel api
 * @name: name
 * @context: context to pass to the irq handler
 *
 * Return: integer status
 */
int icnss_ce_request_irq(int ce_id,
	irqreturn_t (*handler)(int, void *),
	unsigned long flags, const char *name,
	void *context)
{
	if (ce_id >= ICNSS_MAX_IRQ_REGISTRATIONS) {
		HIF_ERROR("%s: invalid ce_id = %d", __func__, ce_id);
		return -EINVAL;
	}

	cnss_stub.stub[ce_id].irq_handler = handler;
	cnss_stub.stub[ce_id].ce_id = ce_id;
	cnss_stub.stub[ce_id].data = context;
	cnss_stub.stub[ce_id].name = name;
	cnss_stub.regged_irq |= (1 << ce_id);
	return 0;
}

/**
 * icnss_ce_free_irq() - icnss_unregister_irq
 * @ce_id: the ce_id that the irq belongs to
 * @context: context with witch the irq was requested.
 * Return: integer status
 */
int icnss_ce_free_irq(int ce_id, void *context)
{
	if (ce_id >= ICNSS_MAX_IRQ_REGISTRATIONS) {
		HIF_ERROR("%s: invalid ce_id = %d", __func__, ce_id);
		return -EINVAL;
	}

	if (cnss_stub.stub[ce_id].data != context) {
		HIF_ERROR("%s: context match failure for ce_id %d",
		__func__, ce_id);
		return -EINVAL;
	}

	if (cnss_stub.regged_irq & (1 << ce_id)) {
		cnss_stub.stub[ce_id].irq_handler = NULL;
		cnss_stub.stub[ce_id].ce_id = 0;
		cnss_stub.stub[ce_id].data = 0;
		cnss_stub.stub[ce_id].name = NULL;
		cnss_stub.regged_irq &= ~(1 << ce_id);
	}
	return 0;
}

/**
 * icnss_dispatch_one_ce_irq() - icnss_dispatch_one_ce_irq
 * @ce_id: ce_id
 *
 * Return: irqreturn_t
 */
static irqreturn_t icnss_dispatch_one_ce_irq(int ce_id)
{
	irqreturn_t ret = IRQ_NONE;

	if (cnss_stub.stub[ce_id].irq_handler)
		ret = cnss_stub.stub[ce_id].irq_handler(
			icnss_get_irq_num(ce_id),
			(void *)cnss_stub.stub[ce_id].data);
	else
		HIF_ERROR(
			"%sd: error - ce_id = %d, no IRQ handler",
			__func__, ce_id);

	return ret;
}

/**
 * icnss_dispatch_ce_irq() - icnss_dispatch_ce_irq
 * @scn: scn
 *
 * Return: N/A
 */
void icnss_dispatch_ce_irq(struct ol_softc *scn)
{
	uint32_t intr_summary;
	int id;
	irqreturn_t ret;

	if (scn->hif_init_done != true)
		return;

	A_TARGET_ACCESS_BEGIN(scn);
	intr_summary = CE_INTERRUPT_SUMMARY(scn);

	if (intr_summary == 0) {
		if ((scn->target_status != OL_TRGET_STATUS_RESET) &&
			(!cdf_atomic_read(&scn->link_suspended))) {

			hif_write32_mb(scn->mem +
				(SOC_CORE_BASE_ADDRESS |
				PCIE_INTR_ENABLE_ADDRESS),
				HOST_GROUP0_MASK);

			hif_read32_mb(scn->mem +
					(SOC_CORE_BASE_ADDRESS |
					PCIE_INTR_ENABLE_ADDRESS));
		}
		A_TARGET_ACCESS_END(scn);
		return;
	} else {
		A_TARGET_ACCESS_END(scn);
	}

	scn->ce_irq_summary = intr_summary;
	for (id = 0; intr_summary && (id < scn->ce_count); id++) {
		if (intr_summary & (1 << id)) {
			intr_summary &= ~(1 << id);
			ret = icnss_dispatch_one_ce_irq(id);
		}
	}
}

/**
 * icnss_get_soc_info() - get soc info
 *
 * This function query the soc information from the platform
 * driver
 *
 * @info: struct icnss_soc_info
 *
 * Return: 0 for success
 */
int icnss_get_soc_info(struct icnss_soc_info *info)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);

	if (!scn) {
		HIF_ERROR("%s: SCN = NULL", __func__);
		return -EINVAL;
	}
	info->v_addr = scn->mem;
	info->p_addr = scn->mem_pa;
	info->version = 0;
	return 0;
}


/* icnss_get_irq_num() - generate a number to represent an irq number
*/
static int icnss_get_irq_num(int ce_id)
{
	if (ce_id < CE_COUNT_MAX && ce_id >= 0)
		return ce_id + 100;

	pr_err("icnss: No irq registered for CE id %d\n", ce_id);
	return -EINVAL;
}

int icnss_get_ce_id(int irq)
{
	int ce_id = irq - 100;
	if (ce_id < CE_COUNT_MAX && ce_id >= 0)
		return ce_id;

	pr_err("icnss: No matching CE id for irq %d\n", irq);
	return -EINVAL;
}
#endif /* HIF_PCI */
