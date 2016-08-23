/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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

#ifndef __PLD_COMMON_H__
#define __PLD_COMMON_H__

#ifdef CONFIG_PLD_STUB

/**
 * struct pld_ce_tgt_pipe_cfg - copy engine target pipe configuration
 * @pipe_num: pipe number
 * @pipe_dir: pipe direction
 * @nentries: number of entries
 * @nbytes_max: max number of bytes
 * @flags: flags
 * @reserved: reserved
 *
 * pld_ce_tgt_pipe_cfg is used to store copy engine target pipe
 * configuration.
 */
struct pld_ce_tgt_pipe_cfg {
	u32 pipe_num;
	u32 pipe_dir;
	u32 nentries;
	u32 nbytes_max;
	u32 flags;
	u32 reserved;
};

/**
 * struct pld_ce_svc_pipe_cfg - copy engine service pipe configuration
 * @service_id: service ID
 * @pipe_dir: pipe direction
 * @pipe_num: pipe number
 *
 * pld_ce_svc_pipe_cfg is used to store copy engine service pipe
 * configuration.
 */
struct pld_ce_svc_pipe_cfg {
	u32 service_id;
	u32 pipe_dir;
	u32 pipe_num;
};

/**
 * struct pld_shadow_reg_cfg - shadow register configuration
 * @ce_id: copy engine ID
 * @reg_offset: register offset
 *
 * pld_shadow_reg_cfg is used to store shadow register configuration.
 */
struct pld_shadow_reg_cfg {
	u16 ce_id;
	u16 reg_offset;
};

/**
 * struct pld_wlan_enable_cfg - WLAN FW configuration
 * @num_ce_tgt_cfg: number of CE target configuration
 * @ce_tgt_cfg: CE target configuration
 * @num_ce_svc_pipe_cfg: number of CE service configuration
 * @ce_svc_cfg: CE service configuration
 * @num_shadow_reg_cfg: number of shadow register configuration
 * @shadow_reg_cfg: shadow register configuration
 *
 * pld_wlan_enable_cfg stores WLAN FW configurations. It will be
 * passed to WLAN FW when WLAN host driver calls wlan_enable.
 */
struct pld_wlan_enable_cfg {
	u32 num_ce_tgt_cfg;
	struct pld_ce_tgt_pipe_cfg *ce_tgt_cfg;
	u32 num_ce_svc_pipe_cfg;
	struct pld_ce_svc_pipe_cfg *ce_svc_cfg;
	u32 num_shadow_reg_cfg;
	struct pld_shadow_reg_cfg *shadow_reg_cfg;
};

/**
 * enum pld_driver_mode - WLAN host driver mode
 * @PLD_MISSION: mission mode
 * @PLD_FTM: FTM mode
 * @PLD_EPPING: EPPING mode
 * @PLD_WALTEST: WAL test mode, FW standalone test mode
 * @PLD_OFF: OFF mode
 */
enum pld_driver_mode {
	PLD_MISSION,
	PLD_FTM,
	PLD_EPPING,
	PLD_WALTEST,
	PLD_OFF
};

#define PLD_MAX_TIMESTAMP_LEN 32

/**
 * struct pld_soc_info - SOC information
 * @v_addr: virtual address of preallocated memory
 * @p_addr: physical address of preallcoated memory
 * @chip_id: chip ID
 * @chip_family: chip family
 * @board_id: board ID
 * @soc_id: SOC ID
 * @fw_version: FW version
 * @fw_build_timestamp: FW build timestamp
 *
 * pld_soc_info is used to store WLAN SOC information.
 */
struct pld_soc_info {
	void __iomem *v_addr;
	phys_addr_t p_addr;
	u32 chip_id;
	u32 chip_family;
	u32 board_id;
	u32 soc_id;
	u32 fw_version;
	char fw_build_timestamp[PLD_MAX_TIMESTAMP_LEN + 1];
};

/**
 * enum pld_runtime_request - PM runtime request
 * @PLD_PM_RUNTIME_GET: pm_runtime_get
 * @PLD_PM_RUNTIME_PUT: pm_runtime_put
 * @PLD_PM_RUNTIME_MARK_LAST_BUSY: pm_runtime_mark_last_busy
 * @PLD_PM_RUNTIME_RESUME: pm_runtime_resume
 * @PLD_PM_RUNTIME_PUT_NOIDLE: pm_runtime_put_noidle
 * @PLD_PM_REQUEST_RESUME: pm_request_resume
 * @PLD_PM_RUNTIME_PUT_AUTO: pm_runtime_put_auto
 */
enum pld_runtime_request {
	PLD_PM_RUNTIME_GET,
	PLD_PM_RUNTIME_PUT,
	PLD_PM_RUNTIME_MARK_LAST_BUSY,
	PLD_PM_RUNTIME_RESUME,
	PLD_PM_RUNTIME_PUT_NOIDLE,
	PLD_PM_REQUEST_RESUME,
	PLD_PM_RUNTIME_PUT_AUTO,
};

static inline int pld_wlan_enable(struct device *dev,
				  struct pld_wlan_enable_cfg *config,
				  enum pld_driver_mode mode,
				  const char *host_version)
{
	return 0;
}
static inline int pld_wlan_disable(struct device *dev,
				   enum pld_driver_mode mode)
{
	return 0;
}
static inline void pld_is_pci_link_down(struct device *dev)
{
	return;
}
static inline int pld_wlan_pm_control(struct device *dev, bool vote)
{
	return 0;
}
static inline void pld_intr_notify_q6(struct device *dev)
{
	return;
}
static inline int pld_ce_request_irq(struct device *dev, unsigned int ce_id,
				     irqreturn_t (*handler)(int, void *),
				     unsigned long flags, const char *name,
				     void *ctx)
{
	return 0;
}
static inline int pld_ce_free_irq(struct device *dev,
				  unsigned int ce_id, void *ctx)
{
	return 0;
}
static inline int pld_get_soc_info(struct device *dev,
				   struct pld_soc_info *info)
{
	return 0;
}
static inline int pld_get_ce_id(struct device *dev, int irq)
{
	return 0;
}
static inline int pld_pm_runtime_request(struct device *dev,
					 enum pld_runtime_request request)
{
	return 0;
}
static inline void pld_runtime_init(struct device *dev, int auto_delay)
{
	return;
}
static inline void pld_runtime_exit(struct device *dev)
{
	return;
}

#endif
#endif
