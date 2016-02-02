/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
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
#ifndef _ICNSS_WLAN_H_
#define _ICNSS_WLAN_H_

#include <linux/device.h>
#include <linux/pci.h>
#include <linux/irqreturn.h>

#define ICNSS_MAX_IRQ_REGISTRATIONS 12

/**
 * struct ce_tgt_pipe_cfg
 *
 * @pipenum: pipe_num
 * @pipedir: pipe_dir
 * @nentries: nentries
 * @nbytes_max: nbytes_max
 * @flags: flags
 * @reserved: reserved
 */
struct ce_tgt_pipe_cfg {
	uint32_t pipe_num;
	uint32_t pipe_dir;
	uint32_t nentries;
	uint32_t nbytes_max;
	uint32_t flags;
	uint32_t reserved;
};

/**
 * struct ce_svc_pipe_cfg
 *
 * @service_id: service_id
 * @pipedir: pipedir
 * @pipenum: pipenum
 */
struct ce_svc_pipe_cfg {
	uint32_t service_id;
	uint32_t pipedir;
	uint32_t pipenum;
};

/**
 * struct icnss_shadow_reg_cfg
 *
 * @ce_id: Copy engine id
 * @reg_offset: Register offset
 */
struct icnss_shadow_reg_cfg {
	u16 ce_id;
	u16 reg_offset;
};
/**
 * struct icnss_wlan_enable_cfg
 *
 * @num_ce_tgt_cfg: num_ce_tgt_cfg
 * @ce_tgt_cfg: ce_tgt_cfg
 * @num_ce_svc_pipe_cfg: num_ce_svc_pipe_cfg
 * @ce_svc_cfg: ce_svc_cfg
 */
struct icnss_wlan_enable_cfg {
	uint32_t num_ce_tgt_cfg;
	struct ce_tgt_pipe_cfg *ce_tgt_cfg;
	uint32_t num_ce_svc_pipe_cfg;
	struct ce_svc_pipe_cfg *ce_svc_cfg;
	u32 num_shadow_reg_cfg;
	struct icnss_shadow_reg_cfg *shadow_reg_cfg;
};

/**
 * enum driver_mode
 *
 * @driver_mode: driver_mode
 */
enum icnss_driver_mode {
	ICNSS_MISSION,
	ICNSS_FTM,
	ICNSS_EPPING,
};

/**
 * struct icnss_soc_info
 *
 * @v_addr: virtual address
 * @p_addr: physical address
 * @ver: version
 */
struct icnss_soc_info {
	void __iomem *v_addr;
	phys_addr_t p_addr;
	uint32_t version;
};

int icnss_wlan_enable(struct icnss_wlan_enable_cfg *config,
	enum icnss_driver_mode mode, const char *host_version);
int icnss_wlan_disable(enum icnss_driver_mode mode);
int icnss_set_fw_debug_mode(bool mode);
int icnss_ce_request_irq(int ce_id,
	irqreturn_t (*handler)(int ce_id, void *arg),
	unsigned long flags, const char *name, void *context);
int icnss_ce_free_irq(int irq, void *context);
void icnss_enable_irq(unsigned int ce_id);
void icnss_disable_irq(unsigned int ce_id);
int icnss_get_soc_info(void *hif_ctx, struct icnss_soc_info *info);
int icnss_get_ce_id(int irq);
#endif /* _ICNSS_WLAN_H_ */
#endif /* HIF_PCI */

