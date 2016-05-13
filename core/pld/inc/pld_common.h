/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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

#ifndef __PLD_COMMON_H__
#define __PLD_COMMON_H__

#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/pm.h>

#define PLD_IMAGE_FILE               "athwlan.bin"
#define PLD_UTF_FIRMWARE_FILE        "utf.bin"
#define PLD_BOARD_DATA_FILE          "fakeboar.bin"
#define PLD_OTP_FILE                 "otp.bin"
#define PLD_SETUP_FILE               "athsetup.bin"
#define PLD_EPPING_FILE              "epping.bin"
#define PLD_EVICTED_FILE             ""

/**
 * enum pld_bus_type - bus type
 * @PLD_BUS_TYPE_NONE: invalid bus type, only return in error cases
 * @PLD_BUS_TYPE_PCIE: PCIE bus
 * @PLD_BUS_TYPE_SNOC: SNOC bus
 */
enum pld_bus_type {
	PLD_BUS_TYPE_NONE = -1,
	PLD_BUS_TYPE_PCIE = 0,
	PLD_BUS_TYPE_SNOC,
	PLD_BUS_TYPE_SDIO,
	PLD_BUS_TYPE_USB
};

#define PLD_MAX_FIRMWARE_SIZE (1 * 1024 * 1024)

/**
 * enum pld_bus_width_type - bus bandwith
 * @PLD_BUS_WIDTH_NONE: don't vote for bus bandwidth
 * @PLD_BUS_WIDTH_LOW: vote for low bus bandwidth
 * @PLD_BUS_WIDTH_MEDIUM: vote for medium bus bandwidth
 * @PLD_BUS_WIDTH_HIGH: vote for high bus bandwidth
 */
enum pld_bus_width_type {
	PLD_BUS_WIDTH_NONE,
	PLD_BUS_WIDTH_LOW,
	PLD_BUS_WIDTH_MEDIUM,
	PLD_BUS_WIDTH_HIGH
};

#define PLD_MAX_FILE_NAME 20

/**
 * struct pld_fw_file - WLAN FW file names
 * @image_file: WLAN FW image file
 * @board_data: WLAN FW board data file
 * @otp_data: WLAN FW OTP file
 * @utf_file: WLAN FW UTF file
 * @utf_board_data: WLAN FW UTF board data file
 * @epping_file: WLAN FW EPPING mode file
 * @evicted_data: WLAN FW evicted file
 * @setup_file: WLAN FW setup file
 *
 * pld_fw_files is used to store WLAN FW file names
 */
struct pld_fw_files {
	char image_file[PLD_MAX_FILE_NAME];
	char board_data[PLD_MAX_FILE_NAME];
	char otp_data[PLD_MAX_FILE_NAME];
	char utf_file[PLD_MAX_FILE_NAME];
	char utf_board_data[PLD_MAX_FILE_NAME];
	char epping_file[PLD_MAX_FILE_NAME];
	char evicted_data[PLD_MAX_FILE_NAME];
	char setup_file[PLD_MAX_FILE_NAME];
};

/**
 * struct pld_image_desc_info - FW image description
 * @fw_addr: FW image address
 * @fw_size: FW image size
 * @bdata_addr: FW board data address
 * @bdata_size: FW board data size
 *
 * pld_image_desc_info is used to store FW image description
 * information.
 */
struct pld_image_desc_info {
	dma_addr_t fw_addr;
	u32 fw_size;
	dma_addr_t bdata_addr;
	u32 bdata_size;
};

#define PLD_CODESWAP_MAX_CODESEGS 16

/**
 * struct pld_codeswap_codeseg_info - code swap segment information
 * @codeseg_total_bytes: total bytes of segments
 * @num_codesegs: number of code segments
 * @codeseg_size: code segment size
 * @codeseg_size_log2: log2 of code segment size
 * @codeseg_busaddr: array of addresses of each code segment
 *
 * pld_codeswap_codeseg_info is used to store code swap segment
 * information.
 */
struct pld_codeswap_codeseg_info {
	u32   codeseg_total_bytes;
	u32   num_codesegs;
	u32   codeseg_size;
	u32   codeseg_size_log2;
	void *codeseg_busaddr[PLD_CODESWAP_MAX_CODESEGS];
};

/**
 * enum pld_platform_cap_flag - platform capability flag
 * @PLD_HAS_EXTERNAL_SWREG: has external regulator
 * @PLD_HAS_UART_ACCESS: has UART access
 */
enum pld_platform_cap_flag {
	PLD_HAS_EXTERNAL_SWREG = 0x01,
	PLD_HAS_UART_ACCESS = 0x02,
};

/**
 * struct pld_platform_cap - platform capabilities
 * @cap_flag: capabilities flag
 *
 * pld_platform_cap provides platform capabilities which are
 * extracted from DTS.
 */
struct pld_platform_cap {
	u32 cap_flag;
};

/**
 * enum pld_driver_status - WLAN driver status
 * @PLD_UNINITIALIZED: driver is uninitialized
 * @PLD_INITIALIZED: driver is initialized
 * @PLD_LOAD_UNLOADL: driver is in load-unload status
 */
enum pld_driver_status {
	PLD_UNINITIALIZED,
	PLD_INITIALIZED,
	PLD_LOAD_UNLOAD
};

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
 * struct pld_driver_ops - driver callback functions
 * @probe: required operation, will be called when device is detected
 * @remove: required operation, will be called when device is removed
 * @shutdown: optional operation, will be called during SSR
 * @reinit: optional operation, will be called during SSR
 * @crash_shutdown: optional operation, will be called when a crash is
 *                  detected
 * @suspend: required operation, will be called for power management
 *           is enabled
 * @resume: required operation, will be called for power management
 *          is enabled
 * @modem_status: optional operation, will be called when platform driver
 *                sending modem power status to WLAN FW
 * @runtime_suspend: optional operation, prepare the device for a condition
 *                   in which it won't be able to communicate with the CPU(s)
 *                   and RAM due to power management.
 * @runtime_resume: optional operation, put the device into the fully
 *                  active state in response to a wakeup event generated by
 *                  hardware or at the request of software.
 */
struct pld_driver_ops {
	int (*probe)(struct device *dev,
		     enum pld_bus_type bus_type,
		     void *bdev, void *id);
	void (*remove)(struct device *dev,
		       enum pld_bus_type bus_type);
	void (*shutdown)(struct device *dev,
			 enum pld_bus_type bus_type);
	int (*reinit)(struct device *dev,
		      enum pld_bus_type bus_type,
		      void *bdev, void *id);
	void (*crash_shutdown)(struct device *dev,
			       enum pld_bus_type bus_type);
	int (*suspend)(struct device *dev,
		       enum pld_bus_type bus_type,
		       pm_message_t state);
	int (*resume)(struct device *dev,
		      enum pld_bus_type bus_type);
	int (*reset_resume)(struct device *dev,
		      enum pld_bus_type bus_type);
	void (*modem_status)(struct device *dev,
			     enum pld_bus_type bus_type,
			     int state);
	int (*runtime_suspend)(struct device *dev,
			       enum pld_bus_type bus_type);
	int (*runtime_resume)(struct device *dev,
			      enum pld_bus_type bus_type);
};

int pld_init(void);
void pld_deinit(void);

int pld_register_driver(struct pld_driver_ops *ops);
void pld_unregister_driver(void);

int pld_wlan_enable(struct device *dev, struct pld_wlan_enable_cfg *config,
		    enum pld_driver_mode mode, const char *host_version);
int pld_wlan_disable(struct device *dev, enum pld_driver_mode mode);
int pld_set_fw_debug_mode(struct device *dev, bool enablefwlog);
void pld_get_default_fw_files(struct pld_fw_files *pfw_files);
int pld_get_fw_files_for_target(struct device *dev,
				struct pld_fw_files *pfw_files,
				u32 target_type, u32 target_version);
int pld_get_fw_image(struct device *dev,
		     struct pld_image_desc_info *image_desc_info);
void pld_is_pci_link_down(struct device *dev);
int pld_shadow_control(struct device *dev, bool enable);
int pld_get_codeswap_struct(struct device *dev,
			    struct pld_codeswap_codeseg_info *swap_seg);
int pld_set_wlan_unsafe_channel(struct device *dev, u16 *unsafe_ch_list,
				u16 ch_count);
int pld_get_wlan_unsafe_channel(struct device *dev, u16 *unsafe_ch_list,
				u16 *ch_count, u16 buf_len);
int pld_wlan_set_dfs_nol(struct device *dev, void *info, u16 info_len);
int pld_wlan_get_dfs_nol(struct device *dev, void *info, u16 info_len);
void pld_schedule_recovery_work(struct device *dev);
int pld_wlan_pm_control(struct device *dev, bool vote);
void *pld_get_virt_ramdump_mem(struct device *dev, unsigned long *size);
void pld_device_crashed(struct device *dev);
void pld_device_self_recovery(struct device *dev);
void pld_intr_notify_q6(struct device *dev);
void pld_request_pm_qos(struct device *dev, u32 qos_val);
void pld_remove_pm_qos(struct device *dev);
int pld_request_bus_bandwidth(struct device *dev, int bandwidth);
int pld_get_platform_cap(struct device *dev, struct pld_platform_cap *cap);
void pld_set_driver_status(struct device *dev, enum pld_driver_status status);
int pld_get_sha_hash(struct device *dev, const u8 *data,
		     u32 data_len, u8 *hash_idx, u8 *out);
void *pld_get_fw_ptr(struct device *dev);
int pld_auto_suspend(struct device *dev);
int pld_auto_resume(struct device *dev);

int pld_ce_request_irq(struct device *dev, unsigned int ce_id,
		       irqreturn_t (*handler)(int, void *),
		       unsigned long flags, const char *name, void *ctx);
int pld_ce_free_irq(struct device *dev, unsigned int ce_id, void *ctx);
void pld_enable_irq(struct device *dev, unsigned int ce_id);
void pld_disable_irq(struct device *dev, unsigned int ce_id);
int pld_get_soc_info(struct device *dev, struct pld_soc_info *info);
int pld_get_ce_id(struct device *dev, int irq);
int pld_get_irq(struct device *dev, int ce_id);
void pld_lock_pm_sem(struct device *dev);
void pld_release_pm_sem(struct device *dev);
int pld_power_on(struct device *dev);
int pld_power_off(struct device *dev);
#endif
