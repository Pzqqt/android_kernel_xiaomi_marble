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
#include <a_debug.h>
#include "hif_main.h"
#include "ce_api.h"
#include "cdf_trace.h"
#include "cds_api.h"
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#endif
#include "hif_debug.h"
#include "epping_main.h"
#include "cds_concurrency.h"

void hif_dump_target_memory(struct ol_softc *scn, void *ramdump_base,
			    uint32_t address, uint32_t size)
{
	uint32_t loc = address;
	uint32_t val = 0;
	uint32_t j = 0;
	u8 *temp = ramdump_base;

	A_TARGET_ACCESS_BEGIN(scn);
	while (j < size) {
		val = hif_read32_mb(scn->mem + loc + j);
		cdf_mem_copy(temp, &val, 4);
		j += 4;
		temp += 4;
	}
	A_TARGET_ACCESS_END(scn);
}
/*
 * TBDXXX: Should be a function call specific to each Target-type.
 * This convoluted macro converts from Target CPU Virtual Address
 * Space to CE Address Space. As part of this process, we
 * conservatively fetch the current PCIE_BAR. MOST of the time,
 * this should match the upper bits of PCI space for this device;
 * but that's not guaranteed.
 */
#ifdef QCA_WIFI_3_0
#define TARG_CPU_SPACE_TO_CE_SPACE(pci_addr, addr) \
	(scn->mem_pa + addr)
#else
#define TARG_CPU_SPACE_TO_CE_SPACE(pci_addr, addr) \
	(((hif_read32_mb((pci_addr) + \
	(SOC_CORE_BASE_ADDRESS|CORE_CTRL_ADDRESS)) & 0x7ff) << 21) \
	 | 0x100000 | ((addr) & 0xfffff))
#endif
/* Wait up to this many Ms for a Diagnostic Access CE operation to complete */
#define DIAG_ACCESS_CE_TIMEOUT_MS 10

/*
 * Diagnostic read/write access is provided for startup/config/debug usage.
 * Caller must guarantee proper alignment, when applicable, and single user
 * at any moment.
 */

CDF_STATUS
hif_diag_read_mem(struct ol_softc *scn, uint32_t address, uint8_t *data,
		  int nbytes)
{
	struct HIF_CE_state *hif_state;
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	cdf_dma_addr_t buf;
	unsigned int completed_nbytes, orig_nbytes, remaining_bytes;
	unsigned int id;
	unsigned int flags;
	struct CE_handle *ce_diag;
	cdf_dma_addr_t CE_data;      /* Host buffer address in CE space */
	cdf_dma_addr_t CE_data_base = 0;
	void *data_buf = NULL;
	int i;
	unsigned int mux_id = 0;
	unsigned int transaction_id = 0xffff;
	cdf_dma_addr_t ce_phy_addr = address;
	unsigned int toeplitz_hash_result;
	unsigned int user_flags = 0;

	hif_state = (struct HIF_CE_state *)scn->hif_hdl;

	transaction_id = (mux_id & MUX_ID_MASK) |
		 (transaction_id & TRANSACTION_ID_MASK);
#ifdef QCA_WIFI_3_0
	user_flags &= DESC_DATA_FLAG_MASK;
#endif

	/* This code cannot handle reads to non-memory space. Redirect to the
	 * register read fn but preserve the multi word read capability of
	 * this fn
	 */
	if (address < DRAM_BASE_ADDRESS) {

		if ((address & 0x3) || ((uintptr_t) data & 0x3))
			return CDF_STATUS_E_INVAL;

		while ((nbytes >= 4) &&
		       (CDF_STATUS_SUCCESS == (status =
				 hif_diag_read_access(scn, address,
				       (uint32_t *)data)))) {

			nbytes -= sizeof(uint32_t);
			address += sizeof(uint32_t);
			data += sizeof(uint32_t);

		}

		return status;
	}
	ce_diag = hif_state->ce_diag;

	A_TARGET_ACCESS_LIKELY(scn);

	/*
	 * Allocate a temporary bounce buffer to hold caller's data
	 * to be DMA'ed from Target. This guarantees
	 *   1) 4-byte alignment
	 *   2) Buffer in DMA-able space
	 */
	orig_nbytes = nbytes;
	data_buf = cdf_os_mem_alloc_consistent(scn->cdf_dev,
				    orig_nbytes, &CE_data_base, 0);
	if (!data_buf) {
		status = CDF_STATUS_E_NOMEM;
		goto done;
	}
	cdf_mem_set(data_buf, orig_nbytes, 0);
	cdf_os_mem_dma_sync_single_for_device(scn->cdf_dev, CE_data_base,
				       orig_nbytes, DMA_FROM_DEVICE);

	remaining_bytes = orig_nbytes;
	CE_data = CE_data_base;
	while (remaining_bytes) {
		nbytes = min(remaining_bytes, DIAG_TRANSFER_LIMIT);
		{
			status = ce_recv_buf_enqueue(ce_diag, NULL, CE_data);
			if (status != CDF_STATUS_SUCCESS)
				goto done;
		}

		{	/* Request CE to send from Target(!)
			 * address to Host buffer */
			/*
			 * The address supplied by the caller is in the
			 * Target CPU virtual address space.
			 *
			 * In order to use this address with the diagnostic CE,
			 * convert it from
			 *    Target CPU virtual address space
			 * to
			 *    CE address space
			 */
			A_TARGET_ACCESS_BEGIN_RET(scn);
			ce_phy_addr =
				TARG_CPU_SPACE_TO_CE_SPACE(scn->mem, address);
			A_TARGET_ACCESS_END_RET(scn);

			status =
				ce_send(ce_diag, NULL, ce_phy_addr, nbytes,
					transaction_id, 0, user_flags);
			if (status != CDF_STATUS_SUCCESS)
				goto done;
		}

		i = 0;
		while (ce_completed_send_next(ce_diag, NULL, NULL, &buf,
				&completed_nbytes, &id, NULL, NULL,
				&toeplitz_hash_result) != CDF_STATUS_SUCCESS) {
			cdf_mdelay(1);
			if (i++ > DIAG_ACCESS_CE_TIMEOUT_MS) {
				status = CDF_STATUS_E_BUSY;
				goto done;
			}
		}
		if (nbytes != completed_nbytes) {
			status = CDF_STATUS_E_FAILURE;
			goto done;
		}
		if (buf != ce_phy_addr) {
			status = CDF_STATUS_E_FAILURE;
			goto done;
		}

		i = 0;
		while (ce_completed_recv_next
				(ce_diag, NULL, NULL, &buf,
				&completed_nbytes, &id,
				 &flags) != CDF_STATUS_SUCCESS) {
			cdf_mdelay(1);
			if (i++ > DIAG_ACCESS_CE_TIMEOUT_MS) {
				status = CDF_STATUS_E_BUSY;
				goto done;
			}
		}
		if (nbytes != completed_nbytes) {
			status = CDF_STATUS_E_FAILURE;
			goto done;
		}
		if (buf != CE_data) {
			status = CDF_STATUS_E_FAILURE;
			goto done;
		}

		remaining_bytes -= nbytes;
		address += nbytes;
		CE_data += nbytes;
	}

done:
	A_TARGET_ACCESS_UNLIKELY(scn);

	if (status == CDF_STATUS_SUCCESS)
		cdf_mem_copy(data, data_buf, orig_nbytes);
	else
		HIF_ERROR("%s failure (0x%x)", __func__, address);

	if (data_buf)
		cdf_os_mem_free_consistent(scn->cdf_dev, orig_nbytes,
				    data_buf, CE_data_base, 0);

	return status;
}

/* Read 4-byte aligned data from Target memory or register */
CDF_STATUS hif_diag_read_access(struct ol_softc *scn,
				uint32_t address, uint32_t *data)
{
	struct HIF_CE_state *hif_state;

	hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	if (address >= DRAM_BASE_ADDRESS) {
		/* Assume range doesn't cross this boundary */
		return hif_diag_read_mem(scn, address, (uint8_t *) data,
					 sizeof(uint32_t));
	} else {
		A_TARGET_ACCESS_BEGIN_RET(scn);
		*data = A_TARGET_READ(scn, address);
		A_TARGET_ACCESS_END_RET(scn);

		return CDF_STATUS_SUCCESS;
	}
}

CDF_STATUS hif_diag_write_mem(struct ol_softc *scn,
			      uint32_t address, uint8_t *data, int nbytes)
{
	struct HIF_CE_state *hif_state;
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	cdf_dma_addr_t buf;
	unsigned int completed_nbytes, orig_nbytes, remaining_bytes;
	unsigned int id;
	unsigned int flags;
	struct CE_handle *ce_diag;
	void *data_buf = NULL;
	cdf_dma_addr_t CE_data;      /* Host buffer address in CE space */
	cdf_dma_addr_t CE_data_base = 0;
	int i;
	unsigned int mux_id = 0;
	unsigned int transaction_id = 0xffff;
	cdf_dma_addr_t ce_phy_addr = address;
	unsigned int toeplitz_hash_result;
	unsigned int user_flags = 0;

	hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	ce_diag = hif_state->ce_diag;
	transaction_id = (mux_id & MUX_ID_MASK) |
		(transaction_id & TRANSACTION_ID_MASK);
#ifdef QCA_WIFI_3_0
	user_flags &= DESC_DATA_FLAG_MASK;
#endif

	A_TARGET_ACCESS_LIKELY(scn);

	/*
	 * Allocate a temporary bounce buffer to hold caller's data
	 * to be DMA'ed to Target. This guarantees
	 *   1) 4-byte alignment
	 *   2) Buffer in DMA-able space
	 */
	orig_nbytes = nbytes;
	data_buf = cdf_os_mem_alloc_consistent(scn->cdf_dev,
				    orig_nbytes, &CE_data_base, 0);
	if (!data_buf) {
		status = A_NO_MEMORY;
		goto done;
	}

	/* Copy caller's data to allocated DMA buf */
	cdf_mem_copy(data_buf, data, orig_nbytes);
	cdf_os_mem_dma_sync_single_for_device(scn->cdf_dev, CE_data_base,
				       orig_nbytes, DMA_TO_DEVICE);

	/*
	 * The address supplied by the caller is in the
	 * Target CPU virtual address space.
	 *
	 * In order to use this address with the diagnostic CE,
	 * convert it from
	 *    Target CPU virtual address space
	 * to
	 *    CE address space
	 */
	A_TARGET_ACCESS_BEGIN_RET(scn);
	ce_phy_addr = TARG_CPU_SPACE_TO_CE_SPACE(scn->mem, address);
	A_TARGET_ACCESS_END_RET(scn);

	remaining_bytes = orig_nbytes;
	CE_data = CE_data_base;
	while (remaining_bytes) {
		nbytes = min(remaining_bytes, DIAG_TRANSFER_LIMIT);

		{   /* Set up to receive directly into Target(!) address */
			status = ce_recv_buf_enqueue(ce_diag,
						NULL, ce_phy_addr);
			if (status != CDF_STATUS_SUCCESS)
				goto done;
		}

		{
			/*
			 * Request CE to send caller-supplied data that
			 * was copied to bounce buffer to Target(!) address.
			 */
			status =
				ce_send(ce_diag, NULL,
					(cdf_dma_addr_t) CE_data, nbytes,
					transaction_id, 0, user_flags);
			if (status != CDF_STATUS_SUCCESS)
				goto done;
		}

		i = 0;
		while (ce_completed_send_next(ce_diag, NULL, NULL, &buf,
			    &completed_nbytes, &id,
			    NULL, NULL, &toeplitz_hash_result) !=
			    CDF_STATUS_SUCCESS) {
			cdf_mdelay(1);
			if (i++ > DIAG_ACCESS_CE_TIMEOUT_MS) {
				status = CDF_STATUS_E_BUSY;
				goto done;
			}
		}

		if (nbytes != completed_nbytes) {
			status = CDF_STATUS_E_FAILURE;
			goto done;
		}

		if (buf != CE_data) {
			status = CDF_STATUS_E_FAILURE;
			goto done;
		}

		i = 0;
		while (ce_completed_recv_next
			(ce_diag, NULL, NULL, &buf,
			&completed_nbytes, &id,
			&flags) != CDF_STATUS_SUCCESS) {
			cdf_mdelay(1);
			if (i++ > DIAG_ACCESS_CE_TIMEOUT_MS) {
				status = CDF_STATUS_E_BUSY;
				goto done;
			}
		}

		if (nbytes != completed_nbytes) {
			status = CDF_STATUS_E_FAILURE;
			goto done;
		}

		if (buf != ce_phy_addr) {
			status = CDF_STATUS_E_FAILURE;
			goto done;
		}

		remaining_bytes -= nbytes;
		address += nbytes;
		CE_data += nbytes;
	}

done:
	A_TARGET_ACCESS_UNLIKELY(scn);

	if (data_buf) {
		cdf_os_mem_free_consistent(scn->cdf_dev, orig_nbytes,
				    data_buf, CE_data_base, 0);
	}

	if (status != CDF_STATUS_SUCCESS) {
		HIF_ERROR("%s failure (0x%llu)", __func__,
			(uint64_t)ce_phy_addr);
	}

	return status;
}

/* Write 4B data to Target memory or register */
CDF_STATUS
hif_diag_write_access(struct ol_softc *scn, uint32_t address, uint32_t data)
{
	struct HIF_CE_state *hif_state;

	hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	if (address >= DRAM_BASE_ADDRESS) {
		/* Assume range doesn't cross this boundary */
		uint32_t data_buf = data;

		return hif_diag_write_mem(scn, address,
					  (uint8_t *) &data_buf,
					  sizeof(uint32_t));
	} else {
		A_TARGET_ACCESS_BEGIN_RET(scn);
		A_TARGET_WRITE(scn, address, data);
		A_TARGET_ACCESS_END_RET(scn);

		return CDF_STATUS_SUCCESS;
	}
}
