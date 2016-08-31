/*
 * Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _HAL_API_H_
#define _HAL_API_H_

#include "qdf_types.h"
#include "hal_internal.h"
#include "hif_io32.h"

/**
 * hal_attach - Initalize HAL layer
 * @hif_handle: Opaque HIF handle
 * @qdf_dev: QDF device
 *
 * Return: Opaque HAL SOC handle
 *		 NULL on failure (if given ring is not available)
 *
 * This function should be called as part of HIF initialization (for accessing
 * copy engines). DP layer will get hal_soc handle using hif_get_hal_handle()
 */
extern void *hal_attach(void *hif_handle, qdf_device_t qdf_dev);

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
	REO_DST,
	REO_EXCEPTION,
	REO_REINJECT,
	REO_CMD,
	REO_STATUS,
	TCL_DATA,
	TCL_CMD,
	TCL_STATUS,
	CE_SRC,
	CE_DST,
	CE_DST_STATUS,
	WBM_IDLE_LINK,
	SW2WBM_RELEASE,
	WBM2SW_RELEASE,
	RXDMA_BUF,
	RXDMA_DST,
	RXDMA_MONITOR_BUF,
	RXDMA_MONITOR_STATUS,
	RXDMA_MONITOR_DST,
	MAX_RING_TYPES
};

/* SRNG flags passed in hal_srng_params.flags */
#define HAL_SRNG_MSI_SWAP				0x00000008
#define HAL_SRNG_RING_PTR_SWAP			0x00000010
#define HAL_SRNG_DATA_TLV_SWAP			0x00000020
#define HAL_SRNG_LOW_THRES_INTR_ENABLE	0x00010000
#define HAL_SRNG_MSI_INTR				0x00020000

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

/* SRNG parameters to be passed to hal_srng_setup */
struct hal_srng_params {
	/* Physical base address of the ring */
	qdf_dma_addr_t ring_base_paddr;
	/* Virtual base address of the ring */
	void *ring_base_vaddr;
	/* Number of entries in ring */
	uint32_t num_entries;
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
};

/**
 * hal_srng_setup - Initalize HW SRNG ring.
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

/**
 * hal_srng_cleanup - Deinitialize HW SRNG ring.
 * @hal_soc: Opaque HAL SOC handle
 * @hal_srng: Opaque HAL SRNG pointer
 */
extern void hal_srng_cleanup(void *hal_soc, void *hal_srng);

/**
 * hal_srng_access_start_unlocked - Start ring access (unlocked). Should use
 * hal_srng_access_start if locked access is required
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Ring pointer (Source or Destination ring)
 *
 * Return: 0 on success; error on failire
 */
static inline int hal_srng_access_start_unlocked(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;

	if (srng->ring_dir == HAL_SRNG_SRC_RING)
		srng->u.src_ring.cached_tp =
			*(volatile uint32_t *)(srng->u.src_ring.tp_addr);
	else
		srng->u.dst_ring.cached_hp =
			*(volatile uint32_t *)(srng->u.dst_ring.hp_addr);

	return 0;
}

/**
 * hal_srng_access_start - Start (locked) ring access
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Ring pointer (Source or Destination ring)
 *
 * Return: 0 on success; error on failire
 */
static inline int hal_srng_access_start(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;

	SRNG_LOCK(&(srng->lock));

	return hal_srng_access_start_unlocked(hal_soc, hal_ring);
}

/**
 * hal_srng_dst_get_next - Get next entry from a destination ring and move
 * cached tail pointer
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Destination ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline void *hal_srng_dst_get_next(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	uint32_t *desc = &(srng->ring_base_vaddr[srng->u.dst_ring.tp]);
	uint32_t desc_loop_cnt;

	desc_loop_cnt = (desc[srng->entry_size - 1] & SRNG_LOOP_CNT_MASK)
		>> SRNG_LOOP_CNT_LSB;

	if (srng->u.dst_ring.loop_cnt == desc_loop_cnt) {
		srng->u.dst_ring.tp = (srng->u.dst_ring.tp + srng->entry_size) &
			srng->ring_size_mask;
		srng->u.dst_ring.loop_cnt = (srng->u.dst_ring.loop_cnt +
			!srng->u.dst_ring.tp) &
			(SRNG_LOOP_CNT_MASK >> SRNG_LOOP_CNT_LSB);
		/* TODO: Confirm if loop count mask is same for all rings */
		return (void *)desc;
	}
	return NULL;
}

/**
 * hal_srng_dst_peek - Get next entry from a ring without moving tail pointer.
 * hal_srng_dst_get_next should be called subsequently to move the tail pointer
 * TODO: See if we need an optimized version of get_next that doesn't check for
 * loop_cnt
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Destination ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline void *hal_srng_dst_peek(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	uint32_t *desc = &(srng->ring_base_vaddr[srng->u.dst_ring.tp]);
	uint32_t desc_loop_cnt;

	desc_loop_cnt = (desc[srng->entry_size - 1] & SRNG_LOOP_CNT_MASK)
		>> SRNG_LOOP_CNT_LSB;

	if (srng->u.dst_ring.loop_cnt == desc_loop_cnt)
		return (void *)desc;
	return NULL;
}

/**
 * hal_srng_dst_num_valid - Returns number of valid entries (to be processed
 * by SW) in destination ring
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Destination ring pointer
 * @sync_hw_ptr: Sync cached head pointer with HW
 *
 */
static inline uint32_t hal_srng_dst_num_valid(void *hal_soc, void *hal_ring,
	int sync_hw_ptr)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	uint32 hp;
	uint32 tp = srng->u.dst_ring.tp;

	if (sync_hw_ptr) {
		hp = *(srng->u.dst_ring.hp_addr);
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
 * hal_srng_src_reap_next - Reap next entry from a source ring and move reap
 * pointer. This can be used to release any buffers associated with completed
 * ring entries. Note that this should not be used for posting new descriptor
 * entries. Posting of new entries should be done only using
 * hal_srng_src_get_next_reaped when this function is used for reaping.
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Source ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline void *hal_srng_src_reap_next(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	uint32_t *desc;
	uint32_t next_reap_hp = (srng->u.src_ring.reap_hp + srng->entry_size) &
		srng->ring_size_mask;

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
 * @hal_ring: Source ring pointer
 *
 * Return: Opaque pointer for next (reaped) source ring entry; NULL on failire
 */
static inline void *hal_srng_src_get_next_reaped(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	uint32_t *desc;

	if (srng->u.src_ring.hp != srng->u.src_ring.reap_hp) {
		desc = &(srng->ring_base_vaddr[srng->u.src_ring.hp]);
		srng->u.src_ring.hp = (srng->u.src_ring.hp + srng->entry_size) &
			srng->ring_size_mask;
		return (void *)desc;
	}

	return NULL;
}

/**
 * hal_srng_src_done_val -
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Source ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline uint32_t hal_srng_src_done_val(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	uint32_t next_reap_hp = (srng->u.src_ring.reap_hp + srng->entry_size) &
		srng->ring_size_mask;

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
 * hal_srng_src_get_next - Get next entry from a source ring and move cached tail pointer
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Source ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline void *hal_srng_src_get_next(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	uint32_t *desc;
	uint32_t next_hp = (srng->u.src_ring.hp + srng->entry_size) &
		srng->ring_size_mask;

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
 * @hal_ring: Source ring pointer
 *
 * Return: Opaque pointer for next ring entry; NULL on failire
 */
static inline void *hal_srng_src_peek(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	uint32_t *desc;

	if (((srng->u.src_ring.hp + srng->entry_size) &
		srng->ring_size_mask) != srng->u.src_ring.cached_tp) {
		desc = &(srng->ring_base_vaddr[srng->u.src_ring.hp]);
		return (void *)desc;
	}

	return NULL;
}

/**
 * hal_srng_src_num_avail - Returns number of available entries in src ring
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Source ring pointer
 * @sync_hw_ptr: Sync cached tail pointer with HW
 *
 */
static inline uint32_t hal_srng_src_num_avail(void *hal_soc,
	void *hal_ring, int sync_hw_ptr)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	uint32 tp;
	uint32 hp = srng->u.src_ring.hp;

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
 * @hal_ring: Ring pointer (Source or Destination ring)
 *
 * Return: 0 on success; error on failire
 */
static inline void hal_srng_access_end_unlocked(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;

	/* TODO: See if we need a write memory barrier here */
	if (srng->flags & HAL_SRNG_LMAC_RING) {
		/* For LMAC rings, ring pointer updates are done through FW and
		 * hence written to a shared memory location that is read by FW
		 */
		if (srng->ring_dir == HAL_SRNG_SRC_RING)
			*(srng->u.src_ring.hp_addr) = srng->u.src_ring.hp;
		else
			*(srng->u.src_ring.tp_addr) = srng->u.dst_ring.tp;
	} else {
		if (srng->ring_dir == HAL_SRNG_SRC_RING)
			hif_write32_mb(srng->u.src_ring.hp_addr,
				srng->u.src_ring.hp);
		else
			hif_write32_mb(srng->u.dst_ring.tp_addr,
				srng->u.dst_ring.tp);
	}
}

/**
 * hal_srng_access_end - Unlock ring access and update cached ring head/tail
 * pointers to HW
 * This should be used only if hal_srng_access_start to start ring access
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Ring pointer (Source or Destination ring)
 *
 * Return: 0 on success; error on failire
 */
static inline void hal_srng_access_end(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;

	hal_srng_access_end_unlocked(hal_soc, hal_ring);
	SRNG_UNLOCK(&(srng->lock));
}


/* TODO: Check if the following definitions is available in HW headers */
#define WBM_IDLE_DESC_LIST 1
#define WBM_IDLE_SCATTER_BUF_SIZE 32704
#define NUM_MPDUS_PER_LINK_DESC 6
#define NUM_MSDUS_PER_LINK_DESC 7
#define REO_QUEUE_DESC_ALIGN 128

#define LINK_DESC_SIZE (NUM_OF_DWORDS_RX_MSDU_LINK << 2)
#define LINK_DESC_ALIGN 128

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
 * hal_set_link_desc_addr - Setup link descriptor in a buffer_addr_info
 * HW structure
 *
 * @desc: Descriptor entry (from WBM_IDLE_LINK ring)
 * @cookie: SW cookie for the buffer/descriptor
 * @link_desc_paddr: Physical address of link descriptor entry
 *
 */
static inline void hal_set_link_desc_addr(void *desc, uint32_t cookie,
	qdf_dma_addr_t link_desc_paddr)
{
	uint32_t *buf_addr = (uint32_t *)desc;
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO_0, BUFFER_ADDR_31_0,
		link_desc_paddr & 0xffffffff);
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO_1, BUFFER_ADDR_39_32,
		(uint64_t)link_desc_paddr >> 32);
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO_1, RETURN_BUFFER_MANAGER,
		WBM_IDLE_DESC_LIST);
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO_1, SW_BUFFER_COOKIE,
		cookie);
}

/**
 * hal_idle_list_scatter_buf_size - Get the size of each scatter buffer
 * in an idle list
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline uint32_t hal_idle_list_scatter_buf_size(void *hal_soc)
{
	return WBM_IDLE_SCATTER_BUF_SIZE;
}

/**
 * hal_get_link_desc_size - Get the size of each link descriptor
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline uint32_t hal_get_link_desc_size(void *hal_soc)
{
	return LINK_DESC_SIZE;
}

/**
 * hal_get_link_desc_align - Get the required start address alignment for
 * link descriptors
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline uint32_t hal_get_link_desc_align(void *hal_soc)
{
	return LINK_DESC_ALIGN;
}

/**
 * hal_num_mpdus_per_link_desc - Get number of mpdus each link desc can hold
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline uint32_t hal_num_mpdus_per_link_desc(void *hal_soc)
{
	return NUM_MPDUS_PER_LINK_DESC;
}

/**
 * hal_num_msdus_per_link_desc - Get number of msdus each link desc can hold
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline uint32_t hal_num_msdus_per_link_desc(void *hal_soc)
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
static inline uint32_t hal_num_mpdu_links_per_queue_desc(void *hal_soc)
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
static inline uint32_t hal_idle_scatter_buf_num_entries(void *hal_soc,
	uint32_t scatter_buf_size)
{
	return (scatter_buf_size - WBM_IDLE_SCATTER_BUF_NEXT_PTR_SIZE) /
		hal_srng_get_entrysize(hal_soc, WBM_IDLE_LINK);
}

/**
 * hal_idle_scatter_buf_setup - Setup scattered idle list using the buffer list
 * provided
 *
 * @hal_soc: Opaque HAL SOC handle
 * @idle_scatter_bufs_base_paddr: Array of physical base addresses
 * @idle_scatter_bufs_base_vaddr: Array of virtual base addresses
 * @num_scatter_bufs: Number of scatter buffers in the above lists
 * @scatter_buf_size: Size of each scatter buffer
 *
 */
extern void hal_setup_link_idle_list(void *hal_soc,
	qdf_dma_addr_t scatter_bufs_base_paddr[],
	void *scatter_bufs_base_vaddr[], uint32_t num_scatter_bufs,
	uint32_t scatter_buf_size, uint32_t last_buf_end_offset);

/**
 * hal_reo_setup - Initialize HW REO block
 *
 * @hal_soc: Opaque HAL SOC handle
 */
extern void hal_reo_setup(void *hal_soc);

enum hal_pn_type {
	HAL_PN_NONE,
	HAL_PN_WPA,
	HAL_PN_WAPI_EVEN,
	HAL_PN_WAPI_UNEVEN,
};

#define HAL_RX_MAX_BA_WINDOW 256
/**
 * hal_get_reo_qdesc_size - Get size of reo queue descriptor
 *
 * @hal_soc: Opaque HAL SOC handle
 * @ba_window_size: BlockAck window size
 *
 */
static inline uint32_t hal_get_reo_qdesc_size(void *hal_soc,
	uint32_t ba_window_size)
{
	if (ba_window_size <= 1)
		return sizeof(struct rx_reo_queue);

	if (ba_window_size <= 105)
		return sizeof(struct rx_reo_queue) +
			sizeof(struct rx_reo_queue_ext);

	if (ba_window_size <= 210)
		return sizeof(struct rx_reo_queue) +
			(2 * sizeof(struct rx_reo_queue_ext));

	return sizeof(struct rx_reo_queue) +
		(3 * sizeof(struct rx_reo_queue_ext));
}

/**
 * hal_get_reo_qdesc_align - Get start address alignment for reo
 * queue descriptors
 *
 * @hal_soc: Opaque HAL SOC handle
 *
 */
static inline uint32_t hal_get_reo_qdesc_align(void *hal_soc)
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
extern void hal_reo_qdesc_setup(void *hal_soc, int tid, uint32_t ba_window_size,
	uint32_t start_seq, void *hw_qdesc_vaddr, qdf_dma_addr_t hw_qdesc_paddr,
	int pn_type);

/**
 * hal_srng_get_hp_addr - Get head pointer physical address
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Ring pointer (Source or Destination ring)
 *
 */
static inline qdf_dma_addr_t hal_srng_get_hp_addr(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	if (!(srng->flags & HAL_SRNG_LMAC_RING)) {
		/* Currently this interface is required only for LMAC rings */
		return (qdf_dma_addr_t)NULL;
	}

	if (srng->ring_dir == HAL_SRNG_SRC_RING) {
		return hal->shadow_wrptr_mem_paddr + (srng->u.src_ring.hp_addr -
			hal->shadow_wrptr_mem_vaddr);
	} else {
		return hal->shadow_rdptr_mem_paddr + (srng->u.dst_ring.hp_addr -
			hal->shadow_rdptr_mem_vaddr);
	}
}

/**
 * hal_srng_get_tp_addr - Get tail pointer physical address
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Ring pointer (Source or Destination ring)
 *
 */
static inline qdf_dma_addr_t hal_srng_get_tp_addr(void *hal_soc, void *hal_ring)
{
	struct hal_srng *srng = (struct hal_srng *)hal_ring;
	struct hal_soc *hal = (struct hal_soc *)hal_soc;

	if (!(srng->flags & HAL_SRNG_LMAC_RING)) {
		/* Currently this interface is required only for LMAC rings */
		return (qdf_dma_addr_t)NULL;
	}

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
 * hal_get_srng_params - Retreive SRNG parameters for a given ring from HAL
 *
 * @hal_soc: Opaque HAL SOC handle
 * @hal_ring: Ring pointer (Source or Destination ring)
 * @ring_params: SRNG parameters will be returned through this structure
 */
extern void hal_get_srng_params(void *hal_soc, void *hal_ring,
	struct hal_srng_params *ring_params);
#endif /* _HAL_API_H_ */
