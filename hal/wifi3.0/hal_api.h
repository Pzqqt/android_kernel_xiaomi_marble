/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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

#ifndef _HAL_API_H_
#define _HAL_API_H_

#include "qdf_types.h"
#include "qdf_util.h"
#include "qdf_atomic.h"
#include "hal_internal.h"
#include "hif.h"
#include "hif_io32.h"

/* calculate the register address offset from bar0 of shadow register x */
#if defined(QCA_WIFI_QCA6390) || defined(QCA_WIFI_QCA6490)
#define SHADOW_REGISTER_START_ADDRESS_OFFSET 0x000008FC
#define SHADOW_REGISTER_END_ADDRESS_OFFSET \
	((SHADOW_REGISTER_START_ADDRESS_OFFSET) + (4 * (MAX_SHADOW_REGISTERS)))
#define SHADOW_REGISTER(x) ((SHADOW_REGISTER_START_ADDRESS_OFFSET) + (4 * (x)))
#elif defined(QCA_WIFI_QCA6290) || defined(QCA_WIFI_QCN9000)
#define SHADOW_REGISTER_START_ADDRESS_OFFSET 0x00003024
#define SHADOW_REGISTER_END_ADDRESS_OFFSET \
	((SHADOW_REGISTER_START_ADDRESS_OFFSET) + (4 * (MAX_SHADOW_REGISTERS)))
#define SHADOW_REGISTER(x) ((SHADOW_REGISTER_START_ADDRESS_OFFSET) + (4 * (x)))
#endif /* QCA_WIFI_QCA6390 || QCA_WIFI_QCA6490 */

#define MAX_UNWINDOWED_ADDRESS 0x80000
#if defined(QCA_WIFI_QCA6390) || defined(QCA_WIFI_QCA6490) || \
	defined(QCA_WIFI_QCN9000)
#define WINDOW_ENABLE_BIT 0x40000000
#else
#define WINDOW_ENABLE_BIT 0x80000000
#endif
#define WINDOW_REG_ADDRESS 0x310C
#define WINDOW_SHIFT 19
#define WINDOW_VALUE_MASK 0x3F
#define WINDOW_START MAX_UNWINDOWED_ADDRESS
#define WINDOW_RANGE_MASK 0x7FFFF
/*
 * BAR + 4K is always accessible, any access outside this
 * space requires force wake procedure.
 * OFFSET = 4K - 32 bytes = 0xFE0
 */
#define MAPPED_REF_OFF 0xFE0

/**
 * hal_ring_desc - opaque handle for DP ring descriptor
 */
struct hal_ring_desc;
typedef struct hal_ring_desc *hal_ring_desc_t;

/**
 * hal_link_desc - opaque handle for DP link descriptor
 */
struct hal_link_desc;
typedef struct hal_link_desc *hal_link_desc_t;

/**
 * hal_rxdma_desc - opaque handle for DP rxdma dst ring descriptor
 */
struct hal_rxdma_desc;
typedef struct hal_rxdma_desc *hal_rxdma_desc_t;

/**
 * hal_buff_addrinfo - opaque handle for DP buffer address info
 */
struct hal_buff_addrinfo;
typedef struct hal_buff_addrinfo *hal_buff_addrinfo_t;

#ifdef ENABLE_VERBOSE_DEBUG
static inline void
hal_set_verbose_debug(bool flag)
{
	is_hal_verbose_debug_enabled = flag;
}
#endif

/**
 * hal_reg_write_result_check() - check register writing result
 * @hal_soc: HAL soc handle
 * @offset: register offset to read
 * @exp_val: the expected value of register
 * @ret_confirm: result confirm flag
 *
 * Return: none
 */
static inline void hal_reg_write_result_check(struct hal_soc *hal_soc,
					      uint32_t offset,
					      uint32_t exp_val)
{
	uint32_t value;

	value = qdf_ioread32(hal_soc->dev_base_addr + offset);
	if (exp_val != value) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "register offset 0x%x write failed!\n", offset);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
			  "the expectation 0x%x, actual value 0x%x\n",
			  exp_val,
			  value);
	}
}

#if !defined(QCA_WIFI_QCA6390) && !defined(QCA_WIFI_QCA6490)
static inline void hal_lock_reg_access(struct hal_soc *soc,
				       unsigned long *flags)
{
	qdf_spin_lock_irqsave(&soc->register_access_lock);
}

static inline void hal_unlock_reg_access(struct hal_soc *soc,
					 unsigned long *flags)
{
	qdf_spin_unlock_irqrestore(&soc->register_access_lock);
}
#else
static inline void hal_lock_reg_access(struct hal_soc *soc,
				       unsigned long *flags)
{
	pld_lock_reg_window(soc->qdf_dev->dev, flags);
}

static inline void hal_unlock_reg_access(struct hal_soc *soc,
					 unsigned long *flags)
{
	pld_unlock_reg_window(soc->qdf_dev->dev, flags);
}
#endif

#ifdef PCIE_REG_WINDOW_LOCAL_NO_CACHE
static inline void hal_select_window(struct hal_soc *hal_soc, uint32_t offset)
{
	uint32_t window = (offset >> WINDOW_SHIFT) & WINDOW_VALUE_MASK;

	qdf_iowrite32(hal_soc->dev_base_addr + WINDOW_REG_ADDRESS,
		      WINDOW_ENABLE_BIT | window);
	hal_soc->register_window = window;
}

/**
 * hal_select_window_confirm() - write remap window register and
				 check writing result
 *
 */
static inline void hal_select_window_confirm(struct hal_soc *hal_soc,
					     uint32_t offset)
{
	uint32_t window = (offset >> WINDOW_SHIFT) & WINDOW_VALUE_MASK;

	qdf_iowrite32(hal_soc->dev_base_addr + WINDOW_REG_ADDRESS,
		      WINDOW_ENABLE_BIT | window);
	hal_soc->register_window = window;

	hal_reg_write_result_check(hal_soc, WINDOW_REG_ADDRESS,
				   WINDOW_ENABLE_BIT | window);
}
#else
static inline void hal_select_window(struct hal_soc *hal_soc, uint32_t offset)
{
	uint32_t window = (offset >> WINDOW_SHIFT) & WINDOW_VALUE_MASK;

	if (window != hal_soc->register_window) {
		qdf_iowrite32(hal_soc->dev_base_addr + WINDOW_REG_ADDRESS,
			      WINDOW_ENABLE_BIT | window);
		hal_soc->register_window = window;
	}
}

static inline void hal_select_window_confirm(struct hal_soc *hal_soc,
					     uint32_t offset)
{
	uint32_t window = (offset >> WINDOW_SHIFT) & WINDOW_VALUE_MASK;

	if (window != hal_soc->register_window) {
		qdf_iowrite32(hal_soc->dev_base_addr + WINDOW_REG_ADDRESS,
			      WINDOW_ENABLE_BIT | window);
		hal_soc->register_window = window;

		hal_reg_write_result_check(
					hal_soc,
					WINDOW_REG_ADDRESS,
					WINDOW_ENABLE_BIT | window);
	}
}
#endif

static inline qdf_iomem_t hal_get_window_address(struct hal_soc *hal_soc,
						 qdf_iomem_t addr)
{
	return hal_soc->ops->hal_get_window_address(hal_soc, addr);
}

/**
 * hal_write32_mb() - Access registers to update configuration
 * @hal_soc: hal soc handle
 * @offset: offset address from the BAR
 * @value: value to write
 *
 * Return: None
 *
 * Description: Register address space is split below:
 *     SHADOW REGION       UNWINDOWED REGION    WINDOWED REGION
 *  |--------------------|-------------------|------------------|
 * BAR  NO FORCE WAKE  BAR+4K  FORCE WAKE  BAR+512K  FORCE WAKE
 *
 * 1. Any access to the shadow region, doesn't need force wake
 *    and windowing logic to access.
 * 2. Any access beyond BAR + 4K:
 *    If init_phase enabled, no force wake is needed and access
 *    should be based on windowed or unwindowed access.
 *    If init_phase disabled, force wake is needed and access
 *    should be based on windowed or unwindowed access.
 *
 * note1: WINDOW_RANGE_MASK = (1 << WINDOW_SHIFT) -1
 * note2: 1 << WINDOW_SHIFT = MAX_UNWINDOWED_ADDRESS
 * note3: WINDOW_VALUE_MASK = big enough that trying to write past
 *                            that window would be a bug
 */
#if !defined(QCA_WIFI_QCA6390) && !defined(QCA_WIFI_QCA6490)
static inline void hal_write32_mb(struct hal_soc *hal_soc, uint32_t offset,
				  uint32_t value)
{
	unsigned long flags;
	qdf_iomem_t new_addr;

	if (!hal_soc->use_register_windowing ||
	    offset < MAX_UNWINDOWED_ADDRESS) {
		qdf_iowrite32(hal_soc->dev_base_addr + offset, value);
	} else if (hal_soc->static_window_map) {
		new_addr = hal_get_window_address(hal_soc,
				hal_soc->dev_base_addr + offset);
		qdf_iowrite32(new_addr, value);
	} else {
		hal_lock_reg_access(hal_soc, &flags);
		hal_select_window(hal_soc, offset);
		qdf_iowrite32(hal_soc->dev_base_addr + WINDOW_START +
			  (offset & WINDOW_RANGE_MASK), value);
		hal_unlock_reg_access(hal_soc, &flags);
	}
}

/**
 * hal_write_address_32_mb - write a value to a register
 *
 */
static inline
void hal_write_address_32_mb(struct hal_soc *hal_soc,
			     qdf_iomem_t addr, uint32_t value)
{
	uint32_t offset;
	qdf_iomem_t new_addr;

	if (!hal_soc->use_register_windowing)
		return qdf_iowrite32(addr, value);

	offset = addr - hal_soc->dev_base_addr;
	if (hal_soc->static_window_map) {
		new_addr = hal_get_window_address(hal_soc, addr);
		return qdf_iowrite32(new_addr, value);
	}
	hal_write32_mb(hal_soc, offset, value);
}

#define hal_write32_mb_confirm(_hal_soc, _offset, _value) \
		hal_write32_mb(_hal_soc, _offset, _value)
#else
static inline void hal_write32_mb(struct hal_soc *hal_soc, uint32_t offset,
				  uint32_t value)
{
	int ret;
	unsigned long flags;

	/* Region < BAR + 4K can be directly accessed */
	if (offset < MAPPED_REF_OFF) {
		qdf_iowrite32(hal_soc->dev_base_addr + offset, value);
		return;
	}

	/* Region greater than BAR + 4K */
	if (!hal_soc->init_phase) {
		ret = hif_force_wake_request(hal_soc->hif_handle);
		if (ret) {
			hal_err("Wake up request failed");
			QDF_BUG(0);
			return;
		}
	}

	if (!hal_soc->use_register_windowing ||
	    offset < MAX_UNWINDOWED_ADDRESS) {
		qdf_iowrite32(hal_soc->dev_base_addr + offset, value);
	} else {
		hal_lock_reg_access(hal_soc, &flags);
		hal_select_window(hal_soc, offset);
		qdf_iowrite32(hal_soc->dev_base_addr + WINDOW_START +
			  (offset & WINDOW_RANGE_MASK), value);
		hal_unlock_reg_access(hal_soc, &flags);
	}

	if (!hal_soc->init_phase) {
		ret = hif_force_wake_release(hal_soc->hif_handle);
		if (ret) {
			hal_err("Wake up request failed");
			QDF_BUG(0);
			return;
		}
	}
}

/**
 * hal_write32_mb_confirm() - write register and check wirting result
 *
 */
static inline void hal_write32_mb_confirm(struct hal_soc *hal_soc,
					  uint32_t offset,
					  uint32_t value)
{
	int ret;
	unsigned long flags;

	/* Region < BAR + 4K can be directly accessed */
	if (offset < MAPPED_REF_OFF) {
		qdf_iowrite32(hal_soc->dev_base_addr + offset, value);
		return;
	}

	/* Region greater than BAR + 4K */
	if (!hal_soc->init_phase) {
		ret = hif_force_wake_request(hal_soc->hif_handle);
		if (ret) {
			hal_err("Wake up request failed");
			QDF_BUG(0);
			return;
		}
	}

	if (!hal_soc->use_register_windowing ||
	    offset < MAX_UNWINDOWED_ADDRESS) {
		qdf_iowrite32(hal_soc->dev_base_addr + offset, value);
		hal_reg_write_result_check(hal_soc, offset,
					   value);
	} else {
		hal_lock_reg_access(hal_soc, &flags);
		hal_select_window_confirm(hal_soc, offset);
		qdf_iowrite32(hal_soc->dev_base_addr + WINDOW_START +
			  (offset & WINDOW_RANGE_MASK), value);

		hal_reg_write_result_check(
				hal_soc,
				WINDOW_START + (offset & WINDOW_RANGE_MASK),
				value);
		hal_unlock_reg_access(hal_soc, &flags);
	}

	if (!hal_soc->init_phase) {
		ret = hif_force_wake_release(hal_soc->hif_handle);
		if (ret) {
			hal_err("Wake up request failed");
			QDF_BUG(0);
			return;
		}
	}
}

/**
 * hal_write_address_32_mb - write a value to a register
 *
 */
static inline
void hal_write_address_32_mb(struct hal_soc *hal_soc,
			     qdf_iomem_t addr, uint32_t value)
{
	uint32_t offset;

	if (!hal_soc->use_register_windowing)
		return qdf_iowrite32(addr, value);

	offset = addr - hal_soc->dev_base_addr;
	hal_write32_mb(hal_soc, offset, value);
}
#endif

#ifdef DP_HAL_MULTIWINDOW_DIRECT_ACCESS
#define hal_srng_write_address_32_mb(_a, _b, _c) qdf_iowrite32(_b, _c)
#else
#define hal_srng_write_address_32_mb(_a, _b, _c) \
		hal_write_address_32_mb(_a, _b, _c)
#endif

#if !defined(QCA_WIFI_QCA6390) && !defined(QCA_WIFI_QCA6490)
/**
 * hal_read32_mb() - Access registers to read configuration
 * @hal_soc: hal soc handle
 * @offset: offset address from the BAR
 * @value: value to write
 *
 * Description: Register address space is split below:
 *     SHADOW REGION       UNWINDOWED REGION    WINDOWED REGION
 *  |--------------------|-------------------|------------------|
 * BAR  NO FORCE WAKE  BAR+4K  FORCE WAKE  BAR+512K  FORCE WAKE
 *
 * 1. Any access to the shadow region, doesn't need force wake
 *    and windowing logic to access.
 * 2. Any access beyond BAR + 4K:
 *    If init_phase enabled, no force wake is needed and access
 *    should be based on windowed or unwindowed access.
 *    If init_phase disabled, force wake is needed and access
 *    should be based on windowed or unwindowed access.
 *
 * Return: < 0 for failure/>= 0 for success
 */
static inline
uint32_t hal_read32_mb(struct hal_soc *hal_soc, uint32_t offset)
{
	uint32_t ret;
	unsigned long flags;
	qdf_iomem_t new_addr;

	if (!hal_soc->use_register_windowing ||
	    offset < MAX_UNWINDOWED_ADDRESS) {
		return qdf_ioread32(hal_soc->dev_base_addr + offset);
	} else if (hal_soc->static_window_map) {
		new_addr = hal_get_window_address(hal_soc, hal_soc->dev_base_addr + offset);
		return qdf_ioread32(new_addr);
	}

	hal_lock_reg_access(hal_soc, &flags);
	hal_select_window(hal_soc, offset);
	ret = qdf_ioread32(hal_soc->dev_base_addr + WINDOW_START +
		       (offset & WINDOW_RANGE_MASK));
	hal_unlock_reg_access(hal_soc, &flags);

	return ret;
}
#else
static
uint32_t hal_read32_mb(struct hal_soc *hal_soc, uint32_t offset)
{
	uint32_t ret;
	unsigned long flags;

	/* Region < BAR + 4K can be directly accessed */
	if (offset < MAPPED_REF_OFF)
		return qdf_ioread32(hal_soc->dev_base_addr + offset);

	if ((!hal_soc->init_phase) &&
	    hif_force_wake_request(hal_soc->hif_handle)) {
		hal_err("Wake up request failed");
		QDF_BUG(0);
		return 0;
	}

	if (!hal_soc->use_register_windowing ||
	    offset < MAX_UNWINDOWED_ADDRESS) {
		ret = qdf_ioread32(hal_soc->dev_base_addr + offset);
	} else {
		hal_lock_reg_access(hal_soc, &flags);
		hal_select_window(hal_soc, offset);
		ret = qdf_ioread32(hal_soc->dev_base_addr + WINDOW_START +
			       (offset & WINDOW_RANGE_MASK));
		hal_unlock_reg_access(hal_soc, &flags);
	}

	if ((!hal_soc->init_phase) &&
	    hif_force_wake_release(hal_soc->hif_handle)) {
		hal_err("Wake up release failed");
		QDF_BUG(0);
		return 0;
	}

	return ret;
}
#endif

/**
 * hal_read_address_32_mb() - Read 32-bit value from the register
 * @soc: soc handle
 * @addr: register address to read
 *
 * Return: 32-bit value
 */
static inline
uint32_t hal_read_address_32_mb(struct hal_soc *soc,
				qdf_iomem_t addr)
{
	uint32_t offset;
	uint32_t ret;
	qdf_iomem_t new_addr;

	if (!soc->use_register_windowing)
		return qdf_ioread32(addr);

	offset = addr - soc->dev_base_addr;
	if (soc->static_window_map) {
		new_addr = hal_get_window_address(soc, addr);
		return qdf_ioread32(new_addr);
	}

	ret = hal_read32_mb(soc, offset);
	return ret;
}

/**
 * hal_attach - Initialize HAL layer
 * @hif_handle: Opaque HIF handle
 * @qdf_dev: QDF device
 *
 * Return: Opaque HAL SOC handle
 *		 NULL on failure (if given ring is not available)
 *
 * This function should be called as part of HIF initialization (for accessing
 * copy engines). DP layer will get hal_soc handle using hif_get_hal_handle()
 */
void *hal_attach(struct hif_opaque_softc *hif_handle, qdf_device_t qdf_dev);

/**
 * hal_detach - Detach HAL layer
 * @hal_soc: HAL SOC handle
 *
 * This function should be called as part of HIF detach
 *
 */
extern void hal_detach(void *hal_soc);

/* SRNG type to be passed in APIs hal_srng_get_entrysize and hal_srng_setup */
enum hal_ring_type {
	REO_DST = 0,
	REO_EXCEPTION = 1,
	REO_REINJECT = 2,
	REO_CMD = 3,
	REO_STATUS = 4,
	TCL_DATA = 5,
	TCL_CMD = 6,
	TCL_STATUS = 7,
	CE_SRC = 8,
	CE_DST = 9,
	CE_DST_STATUS = 10,
	WBM_IDLE_LINK = 11,
	SW2WBM_RELEASE = 12,
	WBM2SW_RELEASE = 13,
	RXDMA_BUF = 14,
	RXDMA_DST = 15,
	RXDMA_MONITOR_BUF = 16,
	RXDMA_MONITOR_STATUS = 17,
	RXDMA_MONITOR_DST = 18,
	RXDMA_MONITOR_DESC = 19,
	DIR_BUF_RX_DMA_SRC = 20,
#ifdef WLAN_FEATURE_CIF_CFR
	WIFI_POS_SRC,
#endif
	MAX_RING_TYPES
};

#define HAL_SRNG_LMAC_RING 0x80000000
/* SRNG flags passed in hal_srng_params.flags */
#define HAL_SRNG_MSI_SWAP				0x00000008
#define HAL_SRNG_RING_PTR_SWAP			0x00000010
#define HAL_SRNG_DATA_TLV_SWAP			0x00000020
#define HAL_SRNG_LOW_THRES_INTR_ENABLE	0x00010000
#define HAL_SRNG_MSI_INTR				0x00020000
#define HAL_SRNG_CACHED_DESC		0x00040000

#define PN_SIZE_24 0
#define PN_SIZE_48 1
#define PN_SIZE_128 2

#ifdef FORCE_WAKE
/**
 * hal_set_init_phase() - Indicate initialization of
 *                        datapath rings
 * @soc: hal_soc handle
 * @init_phase: flag to indicate datapath rings
 *              initialization status
 *
 * Return: None
 */
void hal_set_init_phase(hal_soc_handle_t soc, bool init_phase);
#else
static inline
void hal_set_init_phase(hal_soc_handle_t soc, bool init_phase)
{
}
#endif /* FORCE_WAKE */

/**
 * hal_srng_get_entrysize - Returns size of ring entry in bytes. Should be
 * used by callers for calculating the size of memory to be allocated before
 * calling hal_srng_setup to setup the ring
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 *
 */
extern uint32_t hal_srng_get_entrysize(void *hal_soc, int ring_type);

/**
 * hal_srng_max_entries - Returns maximum possible number of ring entries
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 *
 * Return: Maximum number of entries for the given ring_type
 */
uint32_t hal_srng_max_entries(void *hal_soc, int ring_type);

/**
 * hal_srng_dump - Dump ring status
 * @srng: hal srng pointer
 */
void hal_srng_dump(struct hal_srng *srng);

/**
 * hal_srng_get_dir - Returns the direction of the ring
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 *
 * Return: Ring direction
 */
enum hal_srng_dir hal_srng_get_dir(void *hal_soc, int ring_type);

/* HAL memory information */
struct hal_mem_info {
	/* dev base virutal addr */
	void *dev_base_addr;
	/* dev base physical addr */
	void *dev_base_paddr;
	/* Remote virtual pointer memory for HW/FW updates */
	void *shadow_rdptr_mem_vaddr;
	/* Remote physical pointer memory for HW/FW updates */
	void *shadow_rdptr_mem_paddr;
	/* Shared memory for ring pointer updates from host to FW */
	void *shadow_wrptr_mem_vaddr;
	/* Shared physical memory for ring pointer updates from host to FW */
	void *shadow_wrptr_mem_paddr;
};

/* SRNG parameters to be passed to hal_srng_setup */
struct hal_srng_params {
	/* Physical base address of the ring */
	qdf_dma_addr_t ring_base_paddr;
	/* Virtual base address of the ring */
	void *ring_base_vaddr;
	/* Number of entries in ring */
	uint32_t num_entries;
	/* max transfer length */
	uint16_t max_buffer_length;
	/* MSI Address */
	qdf_dma_addr_t msi_addr;
	/* MSI data */
	uint32_t msi_data;
	/* Interrupt timer threshold – in micro seconds */
	uint32_t intr_timer_thres_us;
	/* Interrupt batch counter threshold – in number of ring entries */
	uint32_t intr_batch_cntr_thres_entries;
	/* Low threshold – in number of ring entries
	 * (valid for src rings only)
	 */
	uint32_t low_threshold;
	/* Misc flags */
	uint32_t flags;
	/* Unique ring id */
	uint8_t ring_id;
	/* Source or Destination ring */
	enum hal_srng_dir ring_dir;
	/* Size of ring entry */
	uint32_t entry_size;
	/* hw register base address */
	void *hwreg_base[MAX_SRNG_REG_GROUPS];
};

/* hal_construct_shadow_config() - initialize the shadow registers for dp rings
 * @hal_soc: hal handle
 *
 * Return: QDF_STATUS_OK on success
 */
extern QDF_STATUS hal_construct_shadow_config(void *hal_soc);

/* hal_set_one_shadow_config() - add a config for the specified ring
 * @hal_soc: hal handle
 * @ring_type: ring type
 * @ring_num: ring num
 *
 * The ring type and ring num uniquely specify the ring.  After this call,
 * the hp/tp will be added as the next entry int the shadow register
 * configuration table.  The hal code will use the shadow register address
 * in place of the hp/tp address.
 *
 * This function is exposed, so that the CE module can skip configuring shadow
 * registers for unused ring and rings assigned to the firmware.
 *
 * Return: QDF_STATUS_OK on success
 */
extern QDF_STATUS hal_set_one_shadow_config(void *hal_soc, int ring_type,
					    int ring_num);
/**
 * hal_get_shadow_config() - retrieve the config table
 * @hal_soc: hal handle
 * @shadow_config: will point to the table after
 * @num_shadow_registers_configured: will contain the number of valid entries
 */
extern void hal_get_shadow_config(void *hal_soc,
				  struct pld_shadow_reg_v2_cfg **shadow_config,
				  int *num_shadow_registers_configured);
/**
 * hal_srng_setup - Initialize HW SRNG ring.
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 * @ring_num: Ring number if there are multiple rings of
 *		same type (staring from 0)
 * @mac_id: valid MAC Id should be passed if ring type is one of lmac rings
 * @ring_params: SRNG ring params in hal_srng_params structure.

 * Callers are expected to allocate contiguous ring memory of size
 * 'num_entries * entry_size' bytes and pass the physical and virtual base
 * addresses through 'ring_base_paddr' and 'ring_base_vaddr' in hal_srng_params
 * structure. Ring base address should be 8 byte aligned and size of each ring
 * entry should be queried using the API hal_srng_get_entrysize
 *
 * Return: Opaque pointer to ring on success
 *		 NULL on failure (if given ring is not available)
 */
extern void *hal_srng_setup(void *hal_soc, int ring_type, int ring_num,
	int mac_id, struct hal_srng_params *ring_params);

/* Remapping ids of REO rings */
#define REO_REMAP_TCL 0
#define REO_REMAP_SW1 1
#define REO_REMAP_SW2 2
#define REO_REMAP_SW3 3
#define REO_REMAP_SW4 4
#define REO_REMAP_RELEASE 5
#define REO_REMAP_FW 6
#define REO_REMAP_UNUSED 7

/*
 * Macro to access HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0
 * to map destination to rings
 */
#define HAL_REO_REMAP_IX0(_VALUE, _OFFSET) \
	((_VALUE) << \
	 (HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_DEST_RING_MAPPING_ ## \
	  _OFFSET ## _SHFT))

/*
 * Macro to access HWIO_REO_R0_DESTINATION_RING_CTRL_IX_1
 * to map destination to rings
 */
#define HAL_REO_REMAP_IX2(_VALUE, _OFFSET) \
	((_VALUE) << \
	 (HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_DEST_RING_MAPPING_ ## \
	  _OFFSET ## _SHFT))

/*
 * Macro to access HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3
 * to map destination to rings
 */
#define HAL_REO_REMAP_IX3(_VALUE, _OFFSET) \
	((_VALUE) << \
	 (HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_DEST_RING_MAPPING_ ## \
	  _OFFSET ## _SHFT))

/**
 * hal_reo_read_write_ctrl_ix - Read or write REO_DESTINATION_RING_CTRL_IX
 * @hal_soc_hdl: HAL SOC handle
 * @read: boolean value to indicate if read or write
 * @ix0: pointer to store IX0 reg value
 * @ix1: pointer to store IX1 reg value
 * @ix2: pointer to store IX2 reg value
 * @ix3: pointer to store IX3 reg value
 */
void hal_reo_read_write_ctrl_ix(hal_soc_handle_t hal_soc_hdl, bool read,
				uint32_t *ix0, uint32_t *ix1,
				uint32_t *ix2, uint32_t *ix3);

/**
 * hal_srng_set_hp_paddr() - Set physical address to dest SRNG head pointer
 * @sring: sring pointer
 * @paddr: physical address
 */
extern void hal_srng_dst_set_hp_paddr(struct hal_srng *sring, uint64_t paddr);

/**
 * hal_srng_dst_init_hp() - Initilaize head pointer with cached head pointer
 * @srng: sring pointer
 * @vaddr: virtual address
 */
extern void hal_srng_dst_init_hp(struct hal_srng *srng, uint32_t *vaddr);

/**
 * hal_srng_cleanup - Deinitialize HW SRNG ring.
 * @hal_soc: Opaque HAL SOC handle
 * @hal_srng: Opaque HAL SRNG pointer
 */
void hal_srng_cleanup(void *hal_soc, hal_ring_handle_t hal_ring_hdl);

static inline bool hal_srng_initialized(hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	return !!srng->initialized;
}

/**
 * hal_srng_dst_peek - Check if there are any entries in the ring (peek)
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Destination ring pointer
 *
 * Caller takes responsibility for any locking needs.
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline
void *hal_srng_dst_peek(hal_soc_handle_t hal_soc_hdl,
			hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	if (srng->u.dst_ring.tp != srng->u.dst_ring.cached_hp)
		return (void *)(&srng->ring_base_vaddr[srng->u.dst_ring.tp]);

	return NULL;
}

/**
 * hal_srng_access_start_unlocked - Start ring access (unlocked). Should use
 * hal_srng_access_start if locked access is required
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Ring pointer (Source or Destination ring)
 *
 * Return: 0 on success; error on failire
 */
static inline int
hal_srng_access_start_unlocked(hal_soc_handle_t hal_soc_hdl,
			       hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	struct hal_soc *soc = (struct hal_soc *)hal_soc_hdl;
	uint32_t *desc;

	if (srng->ring_dir == HAL_SRNG_SRC_RING)
		srng->u.src_ring.cached_tp =
			*(volatile uint32_t *)(srng->u.src_ring.tp_addr);
	else {
		srng->u.dst_ring.cached_hp =
			*(volatile uint32_t *)(srng->u.dst_ring.hp_addr);

		if (srng->flags & HAL_SRNG_CACHED_DESC) {
			desc = hal_srng_dst_peek(hal_soc_hdl, hal_ring_hdl);
			if (qdf_likely(desc)) {
				qdf_mem_dma_cache_sync(soc->qdf_dev,
						       qdf_mem_virt_to_phys
						       (desc),
						       QDF_DMA_FROM_DEVICE,
						       (srng->entry_size *
							sizeof(uint32_t)));
				qdf_prefetch(desc);
			}
		}
	}

	return 0;
}

/**
 * hal_srng_access_start - Start (locked) ring access
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Ring pointer (Source or Destination ring)
 *
 * Return: 0 on success; error on failire
 */
static inline int hal_srng_access_start(hal_soc_handle_t hal_soc_hdl,
					hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	if (qdf_unlikely(!hal_ring_hdl)) {
		qdf_print("Error: Invalid hal_ring\n");
		return -EINVAL;
	}

	SRNG_LOCK(&(srng->lock));

	return hal_srng_access_start_unlocked(hal_soc_hdl, hal_ring_hdl);
}

/**
 * hal_srng_dst_get_next - Get next entry from a destination ring and move
 * cached tail pointer
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Destination ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline
void *hal_srng_dst_get_next(void *hal_soc,
			    hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	struct hal_soc *soc = (struct hal_soc *)hal_soc;
	uint32_t *desc;
	uint32_t *desc_next;
	uint32_t tp;

	if (srng->u.dst_ring.tp != srng->u.dst_ring.cached_hp) {
		desc = &(srng->ring_base_vaddr[srng->u.dst_ring.tp]);
		/* TODO: Using % is expensive, but we have to do this since
		 * size of some SRNG rings is not power of 2 (due to descriptor
		 * sizes). Need to create separate API for rings used
		 * per-packet, with sizes power of 2 (TCL2SW, REO2SW,
		 * SW2RXDMA and CE rings)
		 */
		srng->u.dst_ring.tp = (srng->u.dst_ring.tp + srng->entry_size) %
			srng->ring_size;

		if (srng->flags & HAL_SRNG_CACHED_DESC) {
			tp = srng->u.dst_ring.tp;
			desc_next = &srng->ring_base_vaddr[tp];
			qdf_mem_dma_cache_sync(soc->qdf_dev,
					       qdf_mem_virt_to_phys(desc_next),
					       QDF_DMA_FROM_DEVICE,
					       (srng->entry_size *
						sizeof(uint32_t)));
			qdf_prefetch(desc_next);
		}

		return (void *)desc;
	}

	return NULL;
}

/**
 * hal_srng_dst_get_next_hp - Get next entry from a destination ring and move
 * cached head pointer
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Destination ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline void *
hal_srng_dst_get_next_hp(hal_soc_handle_t hal_soc_hdl,
			 hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	uint32_t *desc;
	/* TODO: Using % is expensive, but we have to do this since
	 * size of some SRNG rings is not power of 2 (due to descriptor
	 * sizes). Need to create separate API for rings used
	 * per-packet, with sizes power of 2 (TCL2SW, REO2SW,
	 * SW2RXDMA and CE rings)
	 */
	uint32_t next_hp = (srng->u.dst_ring.cached_hp + srng->entry_size) %
		srng->ring_size;

	if (next_hp != srng->u.dst_ring.tp) {
		desc = &(srng->ring_base_vaddr[srng->u.dst_ring.cached_hp]);
		srng->u.dst_ring.cached_hp = next_hp;
		return (void *)desc;
	}

	return NULL;
}

/**
 * hal_srng_dst_peek_sync - Check if there are any entries in the ring (peek)
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Destination ring pointer
 *
 * Sync cached head pointer with HW.
 * Caller takes responsibility for any locking needs.
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline
void *hal_srng_dst_peek_sync(hal_soc_handle_t hal_soc_hdl,
			     hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	srng->u.dst_ring.cached_hp =
		*(volatile uint32_t *)(srng->u.dst_ring.hp_addr);

	if (srng->u.dst_ring.tp != srng->u.dst_ring.cached_hp)
		return (void *)(&(srng->ring_base_vaddr[srng->u.dst_ring.tp]));

	return NULL;
}

/**
 * hal_srng_dst_peek_sync_locked - Peek for any entries in the ring
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Destination ring pointer
 *
 * Sync cached head pointer with HW.
 * This function takes up SRNG_LOCK. Should not be called with SRNG lock held.
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline
void *hal_srng_dst_peek_sync_locked(hal_soc_handle_t hal_soc_hdl,
				    hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	void *ring_desc_ptr = NULL;

	if (qdf_unlikely(!hal_ring_hdl)) {
		qdf_print("Error: Invalid hal_ring\n");
		return  NULL;
	}

	SRNG_LOCK(&srng->lock);

	ring_desc_ptr = hal_srng_dst_peek_sync(hal_soc_hdl, hal_ring_hdl);

	SRNG_UNLOCK(&srng->lock);

	return ring_desc_ptr;
}

/**
 * hal_srng_dst_num_valid - Returns number of valid entries (to be processed
 * by SW) in destination ring
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Destination ring pointer
 * @sync_hw_ptr: Sync cached head pointer with HW
 *
 */
static inline
uint32_t hal_srng_dst_num_valid(void *hal_soc,
				hal_ring_handle_t hal_ring_hdl,
				int sync_hw_ptr)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	uint32_t hp;
	uint32_t tp = srng->u.dst_ring.tp;

	if (sync_hw_ptr) {
		hp = *(volatile uint32_t *)(srng->u.dst_ring.hp_addr);
		srng->u.dst_ring.cached_hp = hp;
	} else {
		hp = srng->u.dst_ring.cached_hp;
	}

	if (hp >= tp)
		return (hp - tp) / srng->entry_size;
	else
		return (srng->ring_size - tp + hp) / srng->entry_size;
}

/**
 * hal_srng_dst_num_valid_locked - Returns num valid entries to be processed
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Destination ring pointer
 * @sync_hw_ptr: Sync cached head pointer with HW
 *
 * Returns number of valid entries to be processed by the host driver. The
 * function takes up SRNG lock.
 *
 * Return: Number of valid destination entries
 */
static inline uint32_t
hal_srng_dst_num_valid_locked(hal_soc_handle_t hal_soc,
			      hal_ring_handle_t hal_ring_hdl,
			      int sync_hw_ptr)
{
	uint32_t num_valid;
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	SRNG_LOCK(&srng->lock);
	num_valid = hal_srng_dst_num_valid(hal_soc, hal_ring_hdl, sync_hw_ptr);
	SRNG_UNLOCK(&srng->lock);

	return num_valid;
}

/**
 * hal_srng_src_reap_next - Reap next entry from a source ring and move reap
 * pointer. This can be used to release any buffers associated with completed
 * ring entries. Note that this should not be used for posting new descriptor
 * entries. Posting of new entries should be done only using
 * hal_srng_src_get_next_reaped when this function is used for reaping.
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Source ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline void *
hal_srng_src_reap_next(void *hal_soc, hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	uint32_t *desc;

	/* TODO: Using % is expensive, but we have to do this since
	 * size of some SRNG rings is not power of 2 (due to descriptor
	 * sizes). Need to create separate API for rings used
	 * per-packet, with sizes power of 2 (TCL2SW, REO2SW,
	 * SW2RXDMA and CE rings)
	 */
	uint32_t next_reap_hp = (srng->u.src_ring.reap_hp + srng->entry_size) %
		srng->ring_size;

	if (next_reap_hp != srng->u.src_ring.cached_tp) {
		desc = &(srng->ring_base_vaddr[next_reap_hp]);
		srng->u.src_ring.reap_hp = next_reap_hp;
		return (void *)desc;
	}

	return NULL;
}

/**
 * hal_srng_src_get_next_reaped - Get next entry from a source ring that is
 * already reaped using hal_srng_src_reap_next, for posting new entries to
 * the ring
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Source ring pointer
 *
 * Return: Opaque pointer for next (reaped) source ring entry; NULL on failire
 */
static inline void *
hal_srng_src_get_next_reaped(void *hal_soc, hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	uint32_t *desc;

	if (srng->u.src_ring.hp != srng->u.src_ring.reap_hp) {
		desc = &(srng->ring_base_vaddr[srng->u.src_ring.hp]);
		srng->u.src_ring.hp = (srng->u.src_ring.hp + srng->entry_size) %
			srng->ring_size;

		return (void *)desc;
	}

	return NULL;
}

/**
 * hal_srng_src_pending_reap_next - Reap next entry from a source ring and
 * move reap pointer. This API is used in detach path to release any buffers
 * associated with ring entries which are pending reap.
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Source ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline void *
hal_srng_src_pending_reap_next(void *hal_soc, hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	uint32_t *desc;

	uint32_t next_reap_hp = (srng->u.src_ring.reap_hp + srng->entry_size) %
		srng->ring_size;

	if (next_reap_hp != srng->u.src_ring.hp) {
		desc = &(srng->ring_base_vaddr[next_reap_hp]);
		srng->u.src_ring.reap_hp = next_reap_hp;
		return (void *)desc;
	}

	return NULL;
}

/**
 * hal_srng_src_done_val -
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Source ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline uint32_t
hal_srng_src_done_val(void *hal_soc, hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	/* TODO: Using % is expensive, but we have to do this since
	 * size of some SRNG rings is not power of 2 (due to descriptor
	 * sizes). Need to create separate API for rings used
	 * per-packet, with sizes power of 2 (TCL2SW, REO2SW,
	 * SW2RXDMA and CE rings)
	 */
	uint32_t next_reap_hp = (srng->u.src_ring.reap_hp + srng->entry_size) %
		srng->ring_size;

	if (next_reap_hp == srng->u.src_ring.cached_tp)
		return 0;

	if (srng->u.src_ring.cached_tp > next_reap_hp)
		return (srng->u.src_ring.cached_tp - next_reap_hp) /
			srng->entry_size;
	else
		return ((srng->ring_size - next_reap_hp) +
			srng->u.src_ring.cached_tp) / srng->entry_size;
}

/**
 * hal_get_entrysize_from_srng() - Retrieve ring entry size
 * @hal_ring_hdl: Source ring pointer
 *
 * Return: uint8_t
 */
static inline
uint8_t hal_get_entrysize_from_srng(hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	return srng->entry_size;
}

/**
 * hal_get_sw_hptp - Get SW head and tail pointer location for any ring
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Source ring pointer
 * @tailp: Tail Pointer
 * @headp: Head Pointer
 *
 * Return: Update tail pointer and head pointer in arguments.
 */
static inline
void hal_get_sw_hptp(void *hal_soc, hal_ring_handle_t hal_ring_hdl,
		     uint32_t *tailp, uint32_t *headp)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	if (srng->ring_dir == HAL_SRNG_SRC_RING) {
		*headp = srng->u.src_ring.hp;
		*tailp = *srng->u.src_ring.tp_addr;
	} else {
		*tailp = srng->u.dst_ring.tp;
		*headp = *srng->u.dst_ring.hp_addr;
	}
}

/**
 * hal_srng_src_get_next - Get next entry from a source ring and move cached tail pointer
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Source ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline
void *hal_srng_src_get_next(void *hal_soc,
			    hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	uint32_t *desc;
	/* TODO: Using % is expensive, but we have to do this since
	 * size of some SRNG rings is not power of 2 (due to descriptor
	 * sizes). Need to create separate API for rings used
	 * per-packet, with sizes power of 2 (TCL2SW, REO2SW,
	 * SW2RXDMA and CE rings)
	 */
	uint32_t next_hp = (srng->u.src_ring.hp + srng->entry_size) %
		srng->ring_size;

	if (next_hp != srng->u.src_ring.cached_tp) {
		desc = &(srng->ring_base_vaddr[srng->u.src_ring.hp]);
		srng->u.src_ring.hp = next_hp;
		/* TODO: Since reap function is not used by all rings, we can
		 * remove the following update of reap_hp in this function
		 * if we can ensure that only hal_srng_src_get_next_reaped
		 * is used for the rings requiring reap functionality
		 */
		srng->u.src_ring.reap_hp = next_hp;
		return (void *)desc;
	}

	return NULL;
}

/**
 * hal_srng_src_peek - Get next entry from a ring without moving head pointer.
 * hal_srng_src_get_next should be called subsequently to move the head pointer
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Source ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline
void *hal_srng_src_peek(hal_soc_handle_t hal_soc_hdl,
			hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	uint32_t *desc;

	/* TODO: Using % is expensive, but we have to do this since
	 * size of some SRNG rings is not power of 2 (due to descriptor
	 * sizes). Need to create separate API for rings used
	 * per-packet, with sizes power of 2 (TCL2SW, REO2SW,
	 * SW2RXDMA and CE rings)
	 */
	if (((srng->u.src_ring.hp + srng->entry_size) %
		srng->ring_size) != srng->u.src_ring.cached_tp) {
		desc = &(srng->ring_base_vaddr[srng->u.src_ring.hp]);
		return (void *)desc;
	}

	return NULL;
}

/**
 * hal_srng_src_num_avail - Returns number of available entries in src ring
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Source ring pointer
 * @sync_hw_ptr: Sync cached tail pointer with HW
 *
 */
static inline uint32_t
hal_srng_src_num_avail(void *hal_soc,
		       hal_ring_handle_t hal_ring_hdl, int sync_hw_ptr)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	uint32_t tp;
	uint32_t hp = srng->u.src_ring.hp;

	if (sync_hw_ptr) {
		tp = *(srng->u.src_ring.tp_addr);
		srng->u.src_ring.cached_tp = tp;
	} else {
		tp = srng->u.src_ring.cached_tp;
	}

	if (tp > hp)
		return ((tp - hp) / srng->entry_size) - 1;
	else
		return ((srng->ring_size - hp + tp) / srng->entry_size) - 1;
}

/**
 * hal_srng_access_end_unlocked - End ring access (unlocked) - update cached
 * ring head/tail pointers to HW.
 * This should be used only if hal_srng_access_start_unlocked to start ring
 * access
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Ring pointer (Source or Destination ring)
 *
 * Return: 0 on success; error on failire
 */
static inline void
hal_srng_access_end_unlocked(void *hal_soc, hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	/* TODO: See if we need a write memory barrier here */
	if (srng->flags & HAL_SRNG_LMAC_RING) {
		/* For LMAC rings, ring pointer updates are done through FW and
		 * hence written to a shared memory location that is read by FW
		 */
		if (srng->ring_dir == HAL_SRNG_SRC_RING) {
			*(srng->u.src_ring.hp_addr) = srng->u.src_ring.hp;
		} else {
			*(srng->u.dst_ring.tp_addr) = srng->u.dst_ring.tp;
		}
	} else {
		if (srng->ring_dir == HAL_SRNG_SRC_RING)
			hal_srng_write_address_32_mb(hal_soc,
						     srng->u.src_ring.hp_addr,
						     srng->u.src_ring.hp);
		else
			hal_srng_write_address_32_mb(hal_soc,
						     srng->u.dst_ring.tp_addr,
						     srng->u.dst_ring.tp);
	}
}

/**
 * hal_srng_access_end - Unlock ring access and update cached ring head/tail
 * pointers to HW
 * This should be used only if hal_srng_access_start to start ring access
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Ring pointer (Source or Destination ring)
 *
 * Return: 0 on success; error on failire
 */
static inline void
hal_srng_access_end(void *hal_soc, hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	if (qdf_unlikely(!hal_ring_hdl)) {
		qdf_print("Error: Invalid hal_ring\n");
		return;
	}

	hal_srng_access_end_unlocked(hal_soc, hal_ring_hdl);
	SRNG_UNLOCK(&(srng->lock));
}

/**
 * hal_srng_access_end_reap - Unlock ring access
 * This should be used only if hal_srng_access_start to start ring access
 * and should be used only while reaping SRC ring completions
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Ring pointer (Source or Destination ring)
 *
 * Return: 0 on success; error on failire
 */
static inline void
hal_srng_access_end_reap(void *hal_soc, hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	SRNG_UNLOCK(&(srng->lock));
}

/* TODO: Check if the following definitions is available in HW headers */
#define WBM_IDLE_SCATTER_BUF_SIZE 32704
#define NUM_MPDUS_PER_LINK_DESC 6
#define NUM_MSDUS_PER_LINK_DESC 7
#define REO_QUEUE_DESC_ALIGN 128

#define LINK_DESC_ALIGN 128

#define ADDRESS_MATCH_TAG_VAL 0x5
/* Number of mpdu link pointers is 9 in case of TX_MPDU_QUEUE_HEAD and 14 in
 * of TX_MPDU_QUEUE_EXT. We are defining a common average count here
 */
#define NUM_MPDU_LINKS_PER_QUEUE_DESC 12

/* TODO: Check with HW team on the scatter buffer size supported. As per WBM
 * MLD, scatter_buffer_size in IDLE_LIST_CONTROL register is 9 bits and size
 * should be specified in 16 word units. But the number of bits defined for
 * this field in HW header files is 5.
 */
#define WBM_IDLE_SCATTER_BUF_NEXT_PTR_SIZE 8


/**
 * hal_idle_list_scatter_buf_size - Get the size of each scatter buffer
 * in an idle list
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline
uint32_t hal_idle_list_scatter_buf_size(hal_soc_handle_t hal_soc_hdl)
{
	return WBM_IDLE_SCATTER_BUF_SIZE;
}

/**
 * hal_get_link_desc_size - Get the size of each link descriptor
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline uint32_t hal_get_link_desc_size(hal_soc_handle_t hal_soc_hdl)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	if (!hal_soc || !hal_soc->ops) {
		qdf_print("Error: Invalid ops\n");
		QDF_BUG(0);
		return -EINVAL;
	}
	if (!hal_soc->ops->hal_get_link_desc_size) {
		qdf_print("Error: Invalid function pointer\n");
		QDF_BUG(0);
		return -EINVAL;
	}
	return hal_soc->ops->hal_get_link_desc_size();
}

/**
 * hal_get_link_desc_align - Get the required start address alignment for
 * link descriptors
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline
uint32_t hal_get_link_desc_align(hal_soc_handle_t hal_soc_hdl)
{
	return LINK_DESC_ALIGN;
}

/**
 * hal_num_mpdus_per_link_desc - Get number of mpdus each link desc can hold
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline
uint32_t hal_num_mpdus_per_link_desc(hal_soc_handle_t hal_soc_hdl)
{
	return NUM_MPDUS_PER_LINK_DESC;
}

/**
 * hal_num_msdus_per_link_desc - Get number of msdus each link desc can hold
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline
uint32_t hal_num_msdus_per_link_desc(hal_soc_handle_t hal_soc_hdl)
{
	return NUM_MSDUS_PER_LINK_DESC;
}

/**
 * hal_num_mpdu_links_per_queue_desc - Get number of mpdu links each queue
 * descriptor can hold
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline
uint32_t hal_num_mpdu_links_per_queue_desc(hal_soc_handle_t hal_soc_hdl)
{
	return NUM_MPDU_LINKS_PER_QUEUE_DESC;
}

/**
 * hal_idle_list_scatter_buf_num_entries - Get the number of link desc entries
 * that the given buffer size
 *
 * @hal_soc: Opaque HAL SOC handle
 * @scatter_buf_size: Size of scatter buffer
 *
 */
static inline
uint32_t hal_idle_scatter_buf_num_entries(hal_soc_handle_t hal_soc_hdl,
					  uint32_t scatter_buf_size)
{
	return (scatter_buf_size - WBM_IDLE_SCATTER_BUF_NEXT_PTR_SIZE) /
		hal_srng_get_entrysize(hal_soc_hdl, WBM_IDLE_LINK);
}

/**
 * hal_idle_list_num_scatter_bufs - Get the number of sctater buffer
 * each given buffer size
 *
 * @hal_soc: Opaque HAL SOC handle
 * @total_mem: size of memory to be scattered
 * @scatter_buf_size: Size of scatter buffer
 *
 */
static inline
uint32_t hal_idle_list_num_scatter_bufs(hal_soc_handle_t hal_soc_hdl,
					uint32_t total_mem,
					uint32_t scatter_buf_size)
{
	uint8_t rem = (total_mem % (scatter_buf_size -
			WBM_IDLE_SCATTER_BUF_NEXT_PTR_SIZE)) ? 1 : 0;

	uint32_t num_scatter_bufs = (total_mem / (scatter_buf_size -
				WBM_IDLE_SCATTER_BUF_NEXT_PTR_SIZE)) + rem;

	return num_scatter_bufs;
}

enum hal_pn_type {
	HAL_PN_NONE,
	HAL_PN_WPA,
	HAL_PN_WAPI_EVEN,
	HAL_PN_WAPI_UNEVEN,
};

#define HAL_RX_MAX_BA_WINDOW 256

/**
 * hal_get_reo_qdesc_align - Get start address alignment for reo
 * queue descriptors
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline
uint32_t hal_get_reo_qdesc_align(hal_soc_handle_t hal_soc_hdl)
{
	return REO_QUEUE_DESC_ALIGN;
}

/**
 * hal_reo_qdesc_setup - Setup HW REO queue descriptor
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ba_window_size: BlockAck window size
 * @start_seq: Starting sequence number
 * @hw_qdesc_vaddr: Virtual address of REO queue descriptor memory
 * @hw_qdesc_paddr: Physical address of REO queue descriptor memory
 * @pn_type: PN type (one of the types defined in 'enum hal_pn_type')
 *
 */
void hal_reo_qdesc_setup(hal_soc_handle_t hal_soc_hdl,
			 int tid, uint32_t ba_window_size,
			 uint32_t start_seq, void *hw_qdesc_vaddr,
			 qdf_dma_addr_t hw_qdesc_paddr,
			 int pn_type);

/**
 * hal_srng_get_hp_addr - Get head pointer physical address
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Ring pointer (Source or Destination ring)
 *
 */
static inline qdf_dma_addr_t
hal_srng_get_hp_addr(void *hal_soc,
		     hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	if (srng->ring_dir == HAL_SRNG_SRC_RING) {
		return hal->shadow_wrptr_mem_paddr +
		  ((unsigned long)(srng->u.src_ring.hp_addr) -
		  (unsigned long)(hal->shadow_wrptr_mem_vaddr));
	} else {
		return hal->shadow_rdptr_mem_paddr +
		  ((unsigned long)(srng->u.dst_ring.hp_addr) -
		   (unsigned long)(hal->shadow_rdptr_mem_vaddr));
	}
}

/**
 * hal_srng_get_tp_addr - Get tail pointer physical address
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Ring pointer (Source or Destination ring)
 *
 */
static inline qdf_dma_addr_t
hal_srng_get_tp_addr(void *hal_soc, hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	if (srng->ring_dir == HAL_SRNG_SRC_RING) {
		return hal->shadow_rdptr_mem_paddr +
			((unsigned long)(srng->u.src_ring.tp_addr) -
			(unsigned long)(hal->shadow_rdptr_mem_vaddr));
	} else {
		return hal->shadow_wrptr_mem_paddr +
			((unsigned long)(srng->u.dst_ring.tp_addr) -
			(unsigned long)(hal->shadow_wrptr_mem_vaddr));
	}
}

/**
 * hal_srng_get_num_entries - Get total entries in the HAL Srng
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Ring pointer (Source or Destination ring)
 *
 * Return: total number of entries in hal ring
 */
static inline
uint32_t hal_srng_get_num_entries(hal_soc_handle_t hal_soc_hdl,
				  hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	return srng->num_entries;
}

/**
 * hal_get_srng_params - Retrieve SRNG parameters for a given ring from HAL
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Ring pointer (Source or Destination ring)
 * @ring_params: SRNG parameters will be returned through this structure
 */
void hal_get_srng_params(hal_soc_handle_t hal_soc_hdl,
			 hal_ring_handle_t hal_ring_hdl,
			 struct hal_srng_params *ring_params);

/**
 * hal_mem_info - Retrieve hal memory base address
 *
 * @hal_soc: Opaque HAL SOC handle
 * @mem: pointer to structure to be updated with hal mem info
 */
void hal_get_meminfo(hal_soc_handle_t hal_soc_hdl, struct hal_mem_info *mem);

/**
 * hal_get_target_type - Return target type
 *
 * @hal_soc: Opaque HAL SOC handle
 */
uint32_t hal_get_target_type(hal_soc_handle_t hal_soc_hdl);

/**
 * hal_get_ba_aging_timeout - Retrieve BA aging timeout
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ac: Access category
 * @value: timeout duration in millisec
 */
void hal_get_ba_aging_timeout(hal_soc_handle_t hal_soc_hdl, uint8_t ac,
			      uint32_t *value);
/**
 * hal_set_aging_timeout - Set BA aging timeout
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ac: Access category in millisec
 * @value: timeout duration value
 */
void hal_set_ba_aging_timeout(hal_soc_handle_t hal_soc_hdl, uint8_t ac,
			      uint32_t value);
/**
 * hal_srng_dst_hw_init - Private function to initialize SRNG
 * destination ring HW
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline void hal_srng_dst_hw_init(struct hal_soc *hal,
	struct hal_srng *srng)
{
	hal->ops->hal_srng_dst_hw_init(hal, srng);
}

/**
 * hal_srng_src_hw_init - Private function to initialize SRNG
 * source ring HW
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline void hal_srng_src_hw_init(struct hal_soc *hal,
	struct hal_srng *srng)
{
	hal->ops->hal_srng_src_hw_init(hal, srng);
}

/**
 * hal_get_hw_hptp()  - Get HW head and tail pointer value for any ring
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Source ring pointer
 * @headp: Head Pointer
 * @tailp: Tail Pointer
 * @ring_type: Ring
 *
 * Return: Update tail pointer and head pointer in arguments.
 */
static inline
void hal_get_hw_hptp(hal_soc_handle_t hal_soc_hdl,
		     hal_ring_handle_t hal_ring_hdl,
		     uint32_t *headp, uint32_t *tailp,
		     uint8_t ring_type)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_get_hw_hptp(hal_soc, hal_ring_hdl,
			headp, tailp, ring_type);
}

/**
 * hal_reo_setup - Initialize HW REO block
 *
 * @hal_soc: Opaque HAL SOC handle
 * @reo_params: parameters needed by HAL for REO config
 */
static inline void hal_reo_setup(hal_soc_handle_t hal_soc_hdl,
				 void *reoparams)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_reo_setup(hal_soc, reoparams);
}

/**
 * hal_setup_link_idle_list - Setup scattered idle list using the
 * buffer list provided
 *
 * @hal_soc: Opaque HAL SOC handle
 * @scatter_bufs_base_paddr: Array of physical base addresses
 * @scatter_bufs_base_vaddr: Array of virtual base addresses
 * @num_scatter_bufs: Number of scatter buffers in the above lists
 * @scatter_buf_size: Size of each scatter buffer
 * @last_buf_end_offset: Offset to the last entry
 * @num_entries: Total entries of all scatter bufs
 *
 */
static inline
void hal_setup_link_idle_list(hal_soc_handle_t hal_soc_hdl,
			      qdf_dma_addr_t scatter_bufs_base_paddr[],
			      void *scatter_bufs_base_vaddr[],
			      uint32_t num_scatter_bufs,
			      uint32_t scatter_buf_size,
			      uint32_t last_buf_end_offset,
			      uint32_t num_entries)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_soc->ops->hal_setup_link_idle_list(hal_soc, scatter_bufs_base_paddr,
			scatter_bufs_base_vaddr, num_scatter_bufs,
			scatter_buf_size, last_buf_end_offset,
			num_entries);

}

/**
 * hal_srng_dump_ring_desc() - Dump ring descriptor info
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Source ring pointer
 * @ring_desc: Opaque ring descriptor handle
 */
static inline void hal_srng_dump_ring_desc(hal_soc_handle_t hal_soc_hdl,
					   hal_ring_handle_t hal_ring_hdl,
					   hal_ring_desc_t ring_desc)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
			   ring_desc, (srng->entry_size << 2));
}

/**
 * hal_srng_dump_ring() - Dump last 128 descs of the ring
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring_hdl: Source ring pointer
 */
static inline void hal_srng_dump_ring(hal_soc_handle_t hal_soc_hdl,
				      hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;
	uint32_t *desc;
	uint32_t tp, i;

	tp = srng->u.dst_ring.tp;

	for (i = 0; i < 128; i++) {
		if (!tp)
			tp = srng->ring_size;

		desc = &srng->ring_base_vaddr[tp - srng->entry_size];
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_DP,
				   QDF_TRACE_LEVEL_DEBUG,
				   desc, (srng->entry_size << 2));

		tp -= srng->entry_size;
	}
}

/*
 * hal_rxdma_desc_to_hal_ring_desc - API to convert rxdma ring desc
 * to opaque dp_ring desc type
 * @ring_desc - rxdma ring desc
 *
 * Return: hal_rxdma_desc_t type
 */
static inline
hal_ring_desc_t hal_rxdma_desc_to_hal_ring_desc(hal_rxdma_desc_t ring_desc)
{
	return (hal_ring_desc_t)ring_desc;
}

/**
 * hal_srng_set_event() - Set hal_srng event
 * @hal_ring_hdl: Source ring pointer
 * @event: SRNG ring event
 *
 * Return: None
 */
static inline void hal_srng_set_event(hal_ring_handle_t hal_ring_hdl, int event)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	qdf_atomic_set_bit(event, &srng->srng_event);
}

/**
 * hal_srng_clear_event() - Clear hal_srng event
 * @hal_ring_hdl: Source ring pointer
 * @event: SRNG ring event
 *
 * Return: None
 */
static inline
void hal_srng_clear_event(hal_ring_handle_t hal_ring_hdl, int event)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	qdf_atomic_clear_bit(event, &srng->srng_event);
}

/**
 * hal_srng_get_clear_event() - Clear srng event and return old value
 * @hal_ring_hdl: Source ring pointer
 * @event: SRNG ring event
 *
 * Return: Return old event value
 */
static inline
int hal_srng_get_clear_event(hal_ring_handle_t hal_ring_hdl, int event)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	return qdf_atomic_test_and_clear_bit(event, &srng->srng_event);
}

/**
 * hal_srng_set_flush_last_ts() - Record last flush time stamp
 * @hal_ring_hdl: Source ring pointer
 *
 * Return: None
 */
static inline void hal_srng_set_flush_last_ts(hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	srng->last_flush_ts = qdf_get_log_timestamp();
}

/**
 * hal_srng_inc_flush_cnt() - Increment flush counter
 * @hal_ring_hdl: Source ring pointer
 *
 * Return: None
 */
static inline void hal_srng_inc_flush_cnt(hal_ring_handle_t hal_ring_hdl)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring_hdl;

	srng->flush_count++;
}
#endif /* _HAL_APIH_ */
