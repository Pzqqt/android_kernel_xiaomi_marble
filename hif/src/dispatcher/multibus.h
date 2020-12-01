/*
 * Copyright (c) 2016-2018, 2020 The Linux Foundation. All rights reserved.
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

#ifndef _MULTIBUS_H_
#define _MULTIBUS_H_

#include "osdep.h"
#include "qdf_status.h"
#include "hif_debug.h"

struct hif_softc;
struct hif_exec_context;

struct hif_bus_ops {
	QDF_STATUS (*hif_bus_open)(struct hif_softc *hif_sc,
					enum qdf_bus_type bus_type);
	void (*hif_bus_close)(struct hif_softc *hif_sc);
	void (*hif_bus_prevent_linkdown)(struct hif_softc *hif_sc, bool flag);
	void (*hif_reset_soc)(struct hif_softc *hif_sc);
	int (*hif_bus_early_suspend)(struct hif_softc *hif_ctx);
	int (*hif_bus_late_resume)(struct hif_softc *hif_ctx);
	int (*hif_bus_suspend)(struct hif_softc *hif_ctx);
	int (*hif_bus_resume)(struct hif_softc *hif_ctx);
	int (*hif_bus_suspend_noirq)(struct hif_softc *hif_ctx);
	int (*hif_bus_resume_noirq)(struct hif_softc *hif_ctx);
	int (*hif_target_sleep_state_adjust)(struct hif_softc *scn,
			bool sleep_ok, bool wait_for_it);
	void (*hif_disable_isr)(struct hif_softc *hif_sc);
	void (*hif_nointrs)(struct hif_softc *hif_sc);
	QDF_STATUS (*hif_enable_bus)(struct hif_softc *hif_sc,
				      struct device *dev,
				      void *bdev,
				      const struct hif_bus_id *bid,
				      enum hif_enable_type type);
	void (*hif_disable_bus)(struct hif_softc *hif_sc);
#ifdef FEATURE_RUNTIME_PM
	struct hif_runtime_pm_ctx *(*hif_bus_get_rpm_ctx)(
						struct hif_softc *hif_sc);
	struct device *(*hif_bus_get_dev)(struct hif_softc *hif_sc);
#endif
	int (*hif_bus_configure)(struct hif_softc *hif_sc);
	QDF_STATUS (*hif_get_config_item)(struct hif_softc *hif_sc,
			     int opcode, void *config, uint32_t config_len);
	void (*hif_set_mailbox_swap)(struct hif_softc *hif_sc);
	void (*hif_claim_device)(struct hif_softc *hif_sc);
	void (*hif_shutdown_device)(struct hif_softc *hif_sc);
	void (*hif_stop)(struct hif_softc *hif_sc);
	void (*hif_cancel_deferred_target_sleep)(struct hif_softc *hif_sc);
	void (*hif_irq_disable)(struct hif_softc *hif_sc, int ce_id);
	void (*hif_irq_enable)(struct hif_softc *hif_sc, int ce_id);
	int (*hif_grp_irq_configure)(struct hif_softc *hif_sc,
				     struct hif_exec_context *exec);
	int (*hif_dump_registers)(struct hif_softc *hif_sc);
	void (*hif_dump_target_memory)(struct hif_softc *hif_sc,
				       void *ramdump_base,
				       uint32_t address, uint32_t size);
	uint32_t (*hif_reg_read32)(struct hif_softc *hif_sc,
				   uint32_t offset);
	void (*hif_reg_write32)(struct hif_softc *hif_sc,
				uint32_t offset,
				uint32_t value);
	void (*hif_ipa_get_ce_resource)(struct hif_softc *hif_sc,
					qdf_shared_mem_t **ce_sr,
					uint32_t *sr_ring_size,
					qdf_dma_addr_t *reg_paddr);
	void (*hif_mask_interrupt_call)(struct hif_softc *hif_sc);
	void (*hif_enable_power_management)(struct hif_softc *hif_ctx,
				 bool is_packet_log_enabled);
	void (*hif_disable_power_management)(struct hif_softc *hif_ctx);
	void (*hif_display_stats)(struct hif_softc *hif_ctx);
	void (*hif_clear_stats)(struct hif_softc *hif_ctx);
	void (*hif_set_bundle_mode)(struct hif_softc *hif_ctx, bool enabled,
					int rx_bundle_cnt);
	int (*hif_bus_reset_resume)(struct hif_softc *hif_ctx);
	int (*hif_map_ce_to_irq)(struct hif_softc *hif_sc, int ce_id);
	int (*hif_addr_in_boundary)(struct hif_softc *scn, uint32_t offset);
	bool (*hif_needs_bmi)(struct hif_softc *hif_sc);
	void (*hif_config_irq_affinity)(struct hif_softc *hif_sc);
	int (*hif_config_irq_by_ceid)(struct hif_softc *hif_sc, int ce_id);
	bool (*hif_log_bus_info)(struct hif_softc *scn, uint8_t *data,
				 unsigned int *offset);
};

#ifdef HIF_SNOC
QDF_STATUS hif_initialize_snoc_ops(struct hif_bus_ops *hif_sc);
int hif_snoc_get_context_size(void);
#else
static inline QDF_STATUS hif_initialize_snoc_ops(struct hif_bus_ops *hif_sc)
{
	hif_warn("not supported");
	return QDF_STATUS_E_NOSUPPORT;
}
/**
 * hif_snoc_get_context_size() - dummy when snoc isn't supported
 *
 * Return: 0 as an invalid size to indicate no support
 */
static inline int hif_snoc_get_context_size(void)
{
	return 0;
}
#endif /* HIF_SNOC */

#ifdef HIF_PCI
QDF_STATUS hif_initialize_pci_ops(struct hif_softc *hif_sc);
QDF_STATUS hif_update_irq_ops_with_pci(struct hif_softc *hif_sc);
int hif_pci_get_context_size(void);
#else
static inline QDF_STATUS hif_initialize_pci_ops(struct hif_softc *hif_sc)
{
	hif_warn("not supported");
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
QDF_STATUS hif_update_irq_ops_with_pci(struct hif_softc *hif_sc)
{
	hif_err("not supported");
	return QDF_STATUS_E_NOSUPPORT;
}
/**
 * hif_pci_get_context_size() - dummy when pci isn't supported
 *
 * Return: 0 as an invalid size to indicate no support
 */
static inline int hif_pci_get_context_size(void)
{
	return 0;
}
#endif /* HIF_PCI */

#ifdef HIF_IPCI
/**
 * hif_initialize_ipci_ops() - initialize the pci ops
 * @hif_sc: pointer to hif context
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS hif_initialize_ipci_ops(struct hif_softc *hif_sc);

/**
 * hif_ipci_get_context_size() - return the size of the ipci context
 *
 * Return the size of the context.  (0 for invalid bus)
 */
int hif_ipci_get_context_size(void);
#else
static inline QDF_STATUS hif_initialize_ipci_ops(struct hif_softc *hif_sc)
{
	hif_warn("not supported");
	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * hif_ipci_get_context_size() - dummy when ipci isn't supported
 *
 * Return: 0 as an invalid size to indicate no support
 */
static inline int hif_ipci_get_context_size(void)
{
	return 0;
}
#endif /* HIF_IPCI */

#ifdef HIF_AHB
QDF_STATUS hif_initialize_ahb_ops(struct hif_bus_ops *bus_ops);
int hif_ahb_get_context_size(void);
#else
/**
 * hif_initialize_ahb_ops() - dummy for when ahb not supported
 *
 * Return: QDF_STATUS_E_NOSUPPORT
 */
static inline QDF_STATUS hif_initialize_ahb_ops(struct hif_bus_ops *bus_ops)
{
	hif_warn("not supported");
	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * hif_ahb_get_context_size() - dummy for when ahb not supported
 *
 * Return: 0 as an invalid size to indicate no support
 */
static inline int hif_ahb_get_context_size(void)
{
	return 0;
}
#endif

#ifdef HIF_SDIO
QDF_STATUS hif_initialize_sdio_ops(struct hif_softc *hif_sc);
int hif_sdio_get_context_size(void);
#else
/**
 * hif_initialize_sdio_ops() - dummy for when sdio not supported
 *
 * Return: QDF_STATUS_E_NOSUPPORT
 */

static inline QDF_STATUS hif_initialize_sdio_ops(struct hif_softc *hif_sc)
{
	hif_warn("not supported");
	return QDF_STATUS_E_NOSUPPORT;
}

/**
 * hif_sdio_get_context_size() - dummy when sdio isn't supported
 *
 * Return: 0 as an invalid size to indicate no support
 */
static inline int hif_sdio_get_context_size(void)
{
	return 0;
}
#endif /* HIF_SDIO */

int hif_grp_irq_configure(struct hif_softc *hif_sc,
			  struct hif_exec_context *hif_exec);
#ifdef HIF_USB
QDF_STATUS hif_initialize_usb_ops(struct hif_bus_ops *bus_ops);
int hif_usb_get_context_size(void);
#else
static inline QDF_STATUS hif_initialize_usb_ops(struct hif_bus_ops *bus_ops)
{
	hif_warn("not supported");
	return QDF_STATUS_E_NOSUPPORT;
}
/**
 * hif_usb_get_context_size() - dummy when usb isn't supported
 *
 * Return: 0 as an invalid size to indicate no support
 */
static inline int hif_usb_get_context_size(void)
{
	return 0;
}
#endif /* HIF_USB */

/**
 * hif_config_irq_affinity() - Set IRQ affinity for WLAN IRQs
 * @hif_sc - hif context
 *
 * Set IRQ affinity hint for WLAN IRQs in order to affine to
 * gold cores.
 *
 * Return: None
 */
void hif_config_irq_affinity(struct hif_softc *hif_sc);

/**
 * hif_config_irq_by_ceid() - register irq by CE id
 * @hif_sc - hif context
 * @ce_id - Copy Engine id for which the irq need to be configured
 *
 * Return: 0 on success, negative value on error.
 */
int hif_config_irq_by_ceid(struct hif_softc *hif_sc, int ce_id);

#ifdef HIF_BUS_LOG_INFO
/**
 * hif_log_bus_info() - API to log bus related info
 * @scn: hif handle
 * @data: hang event data buffer
 * @offset: offset at which data needs to be written
 *
 * Return:  true if bus_id is invalid else false
 */
bool hif_log_bus_info(struct hif_softc *scn, uint8_t *data,
		      unsigned int *offset);
#else
static inline
bool hif_log_bus_info(struct hif_softc *scn, uint8_t *data,
		      unsigned int *offset)
{
	return false;
}
#endif
#endif /* _MULTIBUS_H_ */
