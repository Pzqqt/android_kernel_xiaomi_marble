/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#include "hal_hw_headers.h"
#include "hal_api.h"
#include "target_type.h"
#include "wcss_version.h"
#include "qdf_module.h"
#ifdef QCA_WIFI_QCA8074
void hal_qca6290_attach(struct hal_soc *hal);
#endif
#ifdef QCA_WIFI_QCA8074
void hal_qca8074_attach(struct hal_soc *hal);
#endif
#if defined(QCA_WIFI_QCA8074V2) || defined(QCA_WIFI_QCA6018)
void hal_qca8074v2_attach(struct hal_soc *hal);
#endif
#ifdef QCA_WIFI_QCA6390
void hal_qca6390_attach(struct hal_soc *hal);
#endif

#ifdef ENABLE_VERBOSE_DEBUG
bool is_hal_verbose_debug_enabled;
#endif

/**
 * hal_get_srng_ring_id() - get the ring id of a descriped ring
 * @hal: hal_soc data structure
 * @ring_type: type enum describing the ring
 * @ring_num: which ring of the ring type
 * @mac_id: which mac does the ring belong to (or 0 for non-lmac rings)
 *
 * Return: the ring id or -EINVAL if the ring does not exist.
 */
static int hal_get_srng_ring_id(struct hal_soc *hal, int ring_type,
				int ring_num, int mac_id)
{
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);
	int ring_id;

	if (ring_num >= ring_config->max_rings) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
			  "%s: ring_num exceeded maximum no. of supported rings",
			  __func__);
		/* TODO: This is a programming error. Assert if this happens */
		return -EINVAL;
	}

	if (ring_config->lmac_ring) {
		ring_id = ring_config->start_ring_id + ring_num +
			(mac_id * HAL_MAX_RINGS_PER_LMAC);
	} else {
		ring_id = ring_config->start_ring_id + ring_num;
	}

	return ring_id;
}

static struct hal_srng *hal_get_srng(struct hal_soc *hal, int ring_id)
{
	/* TODO: Should we allocate srng structures dynamically? */
	return &(hal->srng_list[ring_id]);
}

#define HP_OFFSET_IN_REG_START 1
#define OFFSET_FROM_HP_TO_TP 4
static void hal_update_srng_hp_tp_address(void *hal_soc,
					  int shadow_config_index,
					  int ring_type,
					  int ring_num)
{
	struct hal_srng *srng;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	int ring_id;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);

	ring_id = hal_get_srng_ring_id(hal_soc, ring_type, ring_num, 0);
	if (ring_id < 0)
		return;

	srng = hal_get_srng(hal_soc, ring_id);

	if (ring_config->ring_dir == HAL_SRNG_DST_RING) {
		srng->u.dst_ring.tp_addr = SHADOW_REGISTER(shadow_config_index)
			+ hal->dev_base_addr;
		hal_debug("tp_addr=%pK dev base addr %pK index %u",
			  srng->u.dst_ring.tp_addr, hal->dev_base_addr,
			  shadow_config_index);
	} else {
		srng->u.src_ring.hp_addr = SHADOW_REGISTER(shadow_config_index)
			+ hal->dev_base_addr;
		hal_debug("hp_addr=%pK dev base addr %pK index %u",
			  srng->u.src_ring.hp_addr,
			  hal->dev_base_addr, shadow_config_index);
	}

}

QDF_STATUS hal_set_one_shadow_config(void *hal_soc,
				     int ring_type,
				     int ring_num)
{
	uint32_t target_register;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	struct hal_hw_srng_config *srng_config = &hal->hw_srng_table[ring_type];
	int shadow_config_index = hal->num_shadow_registers_configured;

	if (shadow_config_index >= MAX_SHADOW_REGISTERS) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_RESOURCES;
	}

	hal->num_shadow_registers_configured++;

	target_register = srng_config->reg_start[HP_OFFSET_IN_REG_START];
	target_register += (srng_config->reg_size[HP_OFFSET_IN_REG_START]
			    *ring_num);

	/* if the ring is a dst ring, we need to shadow the tail pointer */
	if (srng_config->ring_dir == HAL_SRNG_DST_RING)
		target_register += OFFSET_FROM_HP_TO_TP;

	hal->shadow_config[shadow_config_index].addr = target_register;

	/* update hp/tp addr in the hal_soc structure*/
	hal_update_srng_hp_tp_address(hal_soc, shadow_config_index, ring_type,
				      ring_num);

	hal_debug("target_reg %x, shadow register 0x%x shadow_index 0x%x, ring_type %d, ring num %d",
		  target_register,
		  SHADOW_REGISTER(shadow_config_index),
		  shadow_config_index,
		  ring_type, ring_num);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(hal_set_one_shadow_config);

QDF_STATUS hal_construct_shadow_config(void *hal_soc)
{
	int ring_type, ring_num;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	for (ring_type = 0; ring_type < MAX_RING_TYPES; ring_type++) {
		struct hal_hw_srng_config *srng_config =
			&hal->hw_srng_table[ring_type];

		if (ring_type == CE_SRC ||
		    ring_type == CE_DST ||
		    ring_type == CE_DST_STATUS)
			continue;

		if (srng_config->lmac_ring)
			continue;

		for (ring_num = 0; ring_num < srng_config->max_rings;
		     ring_num++)
			hal_set_one_shadow_config(hal_soc, ring_type, ring_num);
	}

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(hal_construct_shadow_config);

void hal_get_shadow_config(void *hal_soc,
	struct pld_shadow_reg_v2_cfg **shadow_config,
	int *num_shadow_registers_configured)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	*shadow_config = hal->shadow_config;
	*num_shadow_registers_configured =
		hal->num_shadow_registers_configured;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s", __func__);
}

qdf_export_symbol(hal_get_shadow_config);


static void hal_validate_shadow_register(struct hal_soc *hal,
				  uint32_t *destination,
				  uint32_t *shadow_address)
{
	unsigned int index;
	uint32_t *shadow_0_offset = SHADOW_REGISTER(0) + hal->dev_base_addr;
	int destination_ba_offset =
		((char *)destination) - (char *)hal->dev_base_addr;

	index =	shadow_address - shadow_0_offset;

	if (index >= MAX_SHADOW_REGISTERS) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: index %x out of bounds", __func__, index);
		goto error;
	} else if (hal->shadow_config[index].addr != destination_ba_offset) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: sanity check failure, expected %x, found %x",
			__func__, destination_ba_offset,
			hal->shadow_config[index].addr);
		goto error;
	}
	return;
error:
	qdf_print("%s: baddr %pK, desination %pK, shadow_address %pK s0offset %pK index %x",
		  __func__, hal->dev_base_addr, destination, shadow_address,
		  shadow_0_offset, index);
	QDF_BUG(0);
	return;
}

static void hal_target_based_configure(struct hal_soc *hal)
{
	switch (hal->target_type) {
#ifdef QCA_WIFI_QCA6290
	case TARGET_TYPE_QCA6290:
		hal->use_register_windowing = true;
		hal_qca6290_attach(hal);
	break;
#endif
#ifdef QCA_WIFI_QCA6390
	case TARGET_TYPE_QCA6390:
		hal->use_register_windowing = true;
		hal_qca6390_attach(hal);
	break;
#endif
#if defined(QCA_WIFI_QCA8074) && defined(CONFIG_WIN)
	case TARGET_TYPE_QCA8074:
		hal_qca8074_attach(hal);
	break;
#endif

#if defined(QCA_WIFI_QCA8074V2) && defined(CONFIG_WIN)
	case TARGET_TYPE_QCA8074V2:
		hal_qca8074v2_attach(hal);
	break;
#endif

#if defined(QCA_WIFI_QCA6018) && defined(CONFIG_WIN)
	case TARGET_TYPE_QCA6018:
		hal_qca8074v2_attach(hal);
	break;
#endif
	default:
	break;
	}
}

uint32_t hal_get_target_type(struct hal_soc *hal)
{
	struct hif_target_info *tgt_info =
		hif_get_target_info_handle(hal->hif_handle);

	return tgt_info->target_type;
}

qdf_export_symbol(hal_get_target_type);

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
 *
 */
void *hal_attach(void *hif_handle, qdf_device_t qdf_dev)
{
	struct hal_soc *hal;
	int i;

	hal = qdf_mem_malloc(sizeof(*hal));

	if (!hal) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: hal_soc allocation failed", __func__);
		goto fail0;
	}
	qdf_minidump_log((void *)hal, sizeof(*hal), "hal_soc");
	hal->hif_handle = hif_handle;
	hal->dev_base_addr = hif_get_dev_ba(hif_handle);
	hal->qdf_dev = qdf_dev;
	hal->shadow_rdptr_mem_vaddr = (uint32_t *)qdf_mem_alloc_consistent(
		qdf_dev, qdf_dev->dev, sizeof(*(hal->shadow_rdptr_mem_vaddr)) *
		HAL_SRNG_ID_MAX, &(hal->shadow_rdptr_mem_paddr));
	if (!hal->shadow_rdptr_mem_paddr) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: hal->shadow_rdptr_mem_paddr allocation failed",
			__func__);
		goto fail1;
	}
	qdf_mem_zero(hal->shadow_rdptr_mem_vaddr,
		     sizeof(*(hal->shadow_rdptr_mem_vaddr)) * HAL_SRNG_ID_MAX);

	hal->shadow_wrptr_mem_vaddr =
		(uint32_t *)qdf_mem_alloc_consistent(qdf_dev, qdf_dev->dev,
		sizeof(*(hal->shadow_wrptr_mem_vaddr)) * HAL_MAX_LMAC_RINGS,
		&(hal->shadow_wrptr_mem_paddr));
	if (!hal->shadow_wrptr_mem_vaddr) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: hal->shadow_wrptr_mem_vaddr allocation failed",
			__func__);
		goto fail2;
	}
	qdf_mem_zero(hal->shadow_wrptr_mem_vaddr,
		sizeof(*(hal->shadow_wrptr_mem_vaddr)) * HAL_MAX_LMAC_RINGS);

	for (i = 0; i < HAL_SRNG_ID_MAX; i++) {
		hal->srng_list[i].initialized = 0;
		hal->srng_list[i].ring_id = i;
	}

	qdf_spinlock_create(&hal->register_access_lock);
	hal->register_window = 0;
	hal->target_type = hal_get_target_type(hal);

	hal_target_based_configure(hal);

	return (void *)hal;

fail2:
	qdf_mem_free_consistent(qdf_dev, qdf_dev->dev,
		sizeof(*(hal->shadow_rdptr_mem_vaddr)) * HAL_SRNG_ID_MAX,
		hal->shadow_rdptr_mem_vaddr, hal->shadow_rdptr_mem_paddr, 0);
fail1:
	qdf_mem_free(hal);
fail0:
	return NULL;
}
qdf_export_symbol(hal_attach);

/**
 * hal_mem_info - Retrieve hal memory base address
 *
 * @hal_soc: Opaque HAL SOC handle
 * @mem: pointer to structure to be updated with hal mem info
 */
void hal_get_meminfo(void *hal_soc, struct hal_mem_info *mem )
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	mem->dev_base_addr = (void *)hal->dev_base_addr;
        mem->shadow_rdptr_mem_vaddr = (void *)hal->shadow_rdptr_mem_vaddr;
	mem->shadow_wrptr_mem_vaddr = (void *)hal->shadow_wrptr_mem_vaddr;
        mem->shadow_rdptr_mem_paddr = (void *)hal->shadow_rdptr_mem_paddr;
	mem->shadow_wrptr_mem_paddr = (void *)hal->shadow_wrptr_mem_paddr;
	hif_read_phy_mem_base(hal->hif_handle, (qdf_dma_addr_t *)&mem->dev_base_paddr);
	return;
}
qdf_export_symbol(hal_get_meminfo);

/**
 * hal_detach - Detach HAL layer
 * @hal_soc: HAL SOC handle
 *
 * Return: Opaque HAL SOC handle
 *		 NULL on failure (if given ring is not available)
 *
 * This function should be called as part of HIF initialization (for accessing
 * copy engines). DP layer will get hal_soc handle using hif_get_hal_handle()
 *
 */
extern void hal_detach(void *hal_soc)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	qdf_mem_free_consistent(hal->qdf_dev, hal->qdf_dev->dev,
		sizeof(*(hal->shadow_rdptr_mem_vaddr)) * HAL_SRNG_ID_MAX,
		hal->shadow_rdptr_mem_vaddr, hal->shadow_rdptr_mem_paddr, 0);
	qdf_mem_free_consistent(hal->qdf_dev, hal->qdf_dev->dev,
		sizeof(*(hal->shadow_wrptr_mem_vaddr)) * HAL_MAX_LMAC_RINGS,
		hal->shadow_wrptr_mem_vaddr, hal->shadow_wrptr_mem_paddr, 0);
	qdf_mem_free(hal);

	return;
}
qdf_export_symbol(hal_detach);


/**
 * hal_ce_dst_setup - Initialize CE destination ring registers
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline void hal_ce_dst_setup(struct hal_soc *hal, struct hal_srng *srng,
				    int ring_num)
{
	uint32_t reg_val = 0;
	uint32_t reg_addr;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, CE_DST);

	/* set DEST_MAX_LENGTH according to ce assignment */
	reg_addr = HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_CTRL_ADDR(
			ring_config->reg_start[R0_INDEX] +
			(ring_num * ring_config->reg_size[R0_INDEX]));

	reg_val = HAL_REG_READ(hal, reg_addr);
	reg_val &= ~HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_CTRL_DEST_MAX_LENGTH_BMSK;
	reg_val |= srng->u.dst_ring.max_buffer_length &
		HWIO_WFSS_CE_CHANNEL_DST_R0_DEST_CTRL_DEST_MAX_LENGTH_BMSK;
	HAL_REG_WRITE(hal, reg_addr, reg_val);
}

/**
 * hal_reo_read_write_ctrl_ix - Read or write REO_DESTINATION_RING_CTRL_IX
 * @hal: HAL SOC handle
 * @read: boolean value to indicate if read or write
 * @ix0: pointer to store IX0 reg value
 * @ix1: pointer to store IX1 reg value
 * @ix2: pointer to store IX2 reg value
 * @ix3: pointer to store IX3 reg value
 */
void hal_reo_read_write_ctrl_ix(struct hal_soc *hal, bool read, uint32_t *ix0,
				uint32_t *ix1, uint32_t *ix2, uint32_t *ix3)
{
	uint32_t reg_offset;

	if (read) {
		if (ix0) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			*ix0 = HAL_REG_READ(hal, reg_offset);
		}

		if (ix1) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_1_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			*ix1 = HAL_REG_READ(hal, reg_offset);
		}

		if (ix2) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			*ix2 = HAL_REG_READ(hal, reg_offset);
		}

		if (ix3) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			*ix3 = HAL_REG_READ(hal, reg_offset);
		}
	} else {
		if (ix0) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_0_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			HAL_REG_WRITE(hal, reg_offset, *ix0);
		}

		if (ix1) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_1_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			HAL_REG_WRITE(hal, reg_offset, *ix1);
		}

		if (ix2) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_2_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			HAL_REG_WRITE(hal, reg_offset, *ix2);
		}

		if (ix3) {
			reg_offset =
				HWIO_REO_R0_DESTINATION_RING_CTRL_IX_3_ADDR(
						SEQ_WCSS_UMAC_REO_REG_OFFSET);
			HAL_REG_WRITE(hal, reg_offset, *ix3);
		}
	}
}

/**
 * hal_srng_dst_set_hp_paddr() - Set physical address to dest ring head pointer
 * @srng: sring pointer
 * @paddr: physical address
 */
void hal_srng_dst_set_hp_paddr(struct hal_srng *srng,
			       uint64_t paddr)
{
	SRNG_DST_REG_WRITE(srng, HP_ADDR_LSB,
			   paddr & 0xffffffff);
	SRNG_DST_REG_WRITE(srng, HP_ADDR_MSB,
			   paddr >> 32);
}

/**
 * hal_srng_dst_init_hp() - Initilaize destination ring head pointer
 * @srng: sring pointer
 * @vaddr: virtual address
 */
void hal_srng_dst_init_hp(struct hal_srng *srng,
			  uint32_t *vaddr)
{
	if (!srng)
		return;

	srng->u.dst_ring.hp_addr = vaddr;
	SRNG_DST_REG_WRITE(srng, HP, srng->u.dst_ring.cached_hp);

	if (vaddr) {
		*srng->u.dst_ring.hp_addr = srng->u.dst_ring.cached_hp;
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "hp_addr=%pK, cached_hp=%d, hp=%d",
			  (void *)srng->u.dst_ring.hp_addr,
			  srng->u.dst_ring.cached_hp,
			  *srng->u.dst_ring.hp_addr);
	}
}

/**
 * hal_srng_hw_init - Private function to initialize SRNG HW
 * @hal_soc: HAL SOC handle
 * @srng: SRNG ring pointer
 */
static inline void hal_srng_hw_init(struct hal_soc *hal,
	struct hal_srng *srng)
{
	if (srng->ring_dir == HAL_SRNG_SRC_RING)
		hal_srng_src_hw_init(hal, srng);
	else
		hal_srng_dst_hw_init(hal, srng);
}

#ifdef CONFIG_SHADOW_V2
#define ignore_shadow false
#define CHECK_SHADOW_REGISTERS true
#else
#define ignore_shadow true
#define CHECK_SHADOW_REGISTERS false
#endif

/**
 * hal_srng_setup - Initialize HW SRNG ring.
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 * @ring_num: Ring number if there are multiple rings of same type (staring
 * from 0)
 * @mac_id: valid MAC Id should be passed if ring type is one of lmac rings
 * @ring_params: SRNG ring params in hal_srng_params structure.

 * Callers are expected to allocate contiguous ring memory of size
 * 'num_entries * entry_size' bytes and pass the physical and virtual base
 * addresses through 'ring_base_paddr' and 'ring_base_vaddr' in
 * hal_srng_params structure. Ring base address should be 8 byte aligned
 * and size of each ring entry should be queried using the API
 * hal_srng_get_entrysize
 *
 * Return: Opaque pointer to ring on success
 *		 NULL on failure (if given ring is not available)
 */
void *hal_srng_setup(void *hal_soc, int ring_type, int ring_num,
	int mac_id, struct hal_srng_params *ring_params)
{
	int ring_id;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	struct hal_srng *srng;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);
	void *dev_base_addr;
	int i;

	ring_id = hal_get_srng_ring_id(hal_soc, ring_type, ring_num, mac_id);
	if (ring_id < 0)
		return NULL;

	hal_verbose_debug("mac_id %d ring_id %d", mac_id, ring_id);

	srng = hal_get_srng(hal_soc, ring_id);

	if (srng->initialized) {
		hal_verbose_debug("Ring (ring_type, ring_num) already initialized");
		return NULL;
	}

	dev_base_addr = hal->dev_base_addr;
	srng->ring_id = ring_id;
	srng->ring_dir = ring_config->ring_dir;
	srng->ring_base_paddr = ring_params->ring_base_paddr;
	srng->ring_base_vaddr = ring_params->ring_base_vaddr;
	srng->entry_size = ring_config->entry_size;
	srng->num_entries = ring_params->num_entries;
	srng->ring_size = srng->num_entries * srng->entry_size;
	srng->ring_size_mask = srng->ring_size - 1;
	srng->msi_addr = ring_params->msi_addr;
	srng->msi_data = ring_params->msi_data;
	srng->intr_timer_thres_us = ring_params->intr_timer_thres_us;
	srng->intr_batch_cntr_thres_entries =
		ring_params->intr_batch_cntr_thres_entries;
	srng->hal_soc = hal_soc;

	for (i = 0 ; i < MAX_SRNG_REG_GROUPS; i++) {
		srng->hwreg_base[i] = dev_base_addr + ring_config->reg_start[i]
			+ (ring_num * ring_config->reg_size[i]);
	}

	/* Zero out the entire ring memory */
	qdf_mem_zero(srng->ring_base_vaddr, (srng->entry_size *
		srng->num_entries) << 2);

	srng->flags = ring_params->flags;
#ifdef BIG_ENDIAN_HOST
		/* TODO: See if we should we get these flags from caller */
	srng->flags |= HAL_SRNG_DATA_TLV_SWAP;
	srng->flags |= HAL_SRNG_MSI_SWAP;
	srng->flags |= HAL_SRNG_RING_PTR_SWAP;
#endif

	if (srng->ring_dir == HAL_SRNG_SRC_RING) {
		srng->u.src_ring.hp = 0;
		srng->u.src_ring.reap_hp = srng->ring_size -
			srng->entry_size;
		srng->u.src_ring.tp_addr =
			&(hal->shadow_rdptr_mem_vaddr[ring_id]);
		srng->u.src_ring.low_threshold =
			ring_params->low_threshold * srng->entry_size;
		if (ring_config->lmac_ring) {
			/* For LMAC rings, head pointer updates will be done
			 * through FW by writing to a shared memory location
			 */
			srng->u.src_ring.hp_addr =
				&(hal->shadow_wrptr_mem_vaddr[ring_id -
					HAL_SRNG_LMAC1_ID_START]);
			srng->flags |= HAL_SRNG_LMAC_RING;
		} else if (ignore_shadow || (srng->u.src_ring.hp_addr == 0)) {
			srng->u.src_ring.hp_addr = SRNG_SRC_ADDR(srng, HP);

			if (CHECK_SHADOW_REGISTERS) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
				    QDF_TRACE_LEVEL_ERROR,
				    "%s: Ring (%d, %d) missing shadow config",
				    __func__, ring_type, ring_num);
			}
		} else {
			hal_validate_shadow_register(hal,
						     SRNG_SRC_ADDR(srng, HP),
						     srng->u.src_ring.hp_addr);
		}
	} else {
		/* During initialization loop count in all the descriptors
		 * will be set to zero, and HW will set it to 1 on completing
		 * descriptor update in first loop, and increments it by 1 on
		 * subsequent loops (loop count wraps around after reaching
		 * 0xffff). The 'loop_cnt' in SW ring state is the expected
		 * loop count in descriptors updated by HW (to be processed
		 * by SW).
		 */
		srng->u.dst_ring.loop_cnt = 1;
		srng->u.dst_ring.tp = 0;
		srng->u.dst_ring.hp_addr =
			&(hal->shadow_rdptr_mem_vaddr[ring_id]);
		if (ring_config->lmac_ring) {
			/* For LMAC rings, tail pointer updates will be done
			 * through FW by writing to a shared memory location
			 */
			srng->u.dst_ring.tp_addr =
				&(hal->shadow_wrptr_mem_vaddr[ring_id -
				HAL_SRNG_LMAC1_ID_START]);
			srng->flags |= HAL_SRNG_LMAC_RING;
		} else if (ignore_shadow || srng->u.dst_ring.tp_addr == 0) {
			srng->u.dst_ring.tp_addr = SRNG_DST_ADDR(srng, TP);

			if (CHECK_SHADOW_REGISTERS) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
				    QDF_TRACE_LEVEL_ERROR,
				    "%s: Ring (%d, %d) missing shadow config",
				    __func__, ring_type, ring_num);
			}
		} else {
			hal_validate_shadow_register(hal,
						     SRNG_DST_ADDR(srng, TP),
						     srng->u.dst_ring.tp_addr);
		}
	}

	if (!(ring_config->lmac_ring)) {
		hal_srng_hw_init(hal, srng);

		if (ring_type == CE_DST) {
			srng->u.dst_ring.max_buffer_length = ring_params->max_buffer_length;
			hal_ce_dst_setup(hal, srng, ring_num);
		}
	}

	SRNG_LOCK_INIT(&srng->lock);

	srng->initialized = true;

	return (void *)srng;
}
qdf_export_symbol(hal_srng_setup);

/**
 * hal_srng_cleanup - Deinitialize HW SRNG ring.
 * @hal_soc: Opaque HAL SOC handle
 * @hal_srng: Opaque HAL SRNG pointer
 */
void hal_srng_cleanup(void *hal_soc, void *hal_srng)
{
	struct hal_srng *srng = (struct hal_srng *)hal_srng;
	SRNG_LOCK_DESTROY(&srng->lock);
	srng->initialized = 0;
}
qdf_export_symbol(hal_srng_cleanup);

/**
 * hal_srng_get_entrysize - Returns size of ring entry in bytes
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 *
 */
uint32_t hal_srng_get_entrysize(void *hal_soc, int ring_type)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);
	return ring_config->entry_size << 2;
}
qdf_export_symbol(hal_srng_get_entrysize);

/**
 * hal_srng_max_entries - Returns maximum possible number of ring entries
 * @hal_soc: Opaque HAL SOC handle
 * @ring_type: one of the types from hal_ring_type
 *
 * Return: Maximum number of entries for the given ring_type
 */
uint32_t hal_srng_max_entries(void *hal_soc, int ring_type)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);

	return ring_config->max_size / ring_config->entry_size;
}
qdf_export_symbol(hal_srng_max_entries);

enum hal_srng_dir hal_srng_get_dir(void *hal_soc, int ring_type)
{
	struct hal_soc *hal = (struct hal_soc *)hal_soc;
	struct hal_hw_srng_config *ring_config =
		HAL_SRNG_CONFIG(hal, ring_type);

	return ring_config->ring_dir;
}

/**
 * hal_srng_dump - Dump ring status
 * @srng: hal srng pointer
 */
void hal_srng_dump(struct hal_srng *srng)
{
	if (srng->ring_dir == HAL_SRNG_SRC_RING) {
		qdf_print("=== SRC RING %d ===", srng->ring_id);
		qdf_print("hp %u, reap_hp %u, tp %u, cached tp %u",
			  srng->u.src_ring.hp,
			  srng->u.src_ring.reap_hp,
			  *srng->u.src_ring.tp_addr,
			  srng->u.src_ring.cached_tp);
	} else {
		qdf_print("=== DST RING %d ===", srng->ring_id);
		qdf_print("tp %u, hp %u, cached tp %u, loop_cnt %u",
			  srng->u.dst_ring.tp,
			  *srng->u.dst_ring.hp_addr,
			  srng->u.dst_ring.cached_hp,
			  srng->u.dst_ring.loop_cnt);
	}
}

/**
 * hal_get_srng_params - Retrieve SRNG parameters for a given ring from HAL
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Ring pointer (Source or Destination ring)
 * @ring_params: SRNG parameters will be returned through this structure
 */
extern void hal_get_srng_params(void *hal_soc, void *hal_ring,
	struct hal_srng_params *ring_params)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	int i =0;
	ring_params->ring_id = srng->ring_id;
	ring_params->ring_dir = srng->ring_dir;
	ring_params->entry_size = srng->entry_size;

	ring_params->ring_base_paddr = srng->ring_base_paddr;
	ring_params->ring_base_vaddr = srng->ring_base_vaddr;
	ring_params->num_entries = srng->num_entries;
	ring_params->msi_addr = srng->msi_addr;
	ring_params->msi_data = srng->msi_data;
	ring_params->intr_timer_thres_us = srng->intr_timer_thres_us;
	ring_params->intr_batch_cntr_thres_entries =
		srng->intr_batch_cntr_thres_entries;
	ring_params->low_threshold = srng->u.src_ring.low_threshold;
	ring_params->flags = srng->flags;
	ring_params->ring_id = srng->ring_id;
	for (i = 0 ; i < MAX_SRNG_REG_GROUPS; i++)
		ring_params->hwreg_base[i] = srng->hwreg_base[i];
}
qdf_export_symbol(hal_get_srng_params);
