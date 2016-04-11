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

#include "targcfg.h"
#include "qdf_lock.h"
#include "qdf_status.h"
#include "qdf_status.h"
#include <qdf_atomic.h>         /* qdf_atomic_read */
#include <targaddrs.h>
#include "hif_io32.h"
#include <hif.h>
#include "regtable.h"
#include <a_debug.h>
#include "hif_main.h"
#include "ce_api.h"
#include "qdf_trace.h"
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#endif
#include "hif_debug.h"

void
hif_dump_target_memory(struct hif_opaque_softc *hif_ctx, void *ramdump_base,
						uint32_t address, uint32_t size)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	uint32_t loc = address;
	uint32_t val = 0;
	uint32_t j = 0;
	u8 *temp = ramdump_base;

	if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
		return;

	while (j < size) {
		val = hif_read32_mb(scn->mem + loc + j);
		qdf_mem_copy(temp, &val, 4);
		j += 4;
		temp += 4;
	}

	Q_TARGET_ACCESS_END(scn);
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

QDF_STATUS
hif_diag_read_mem(struct hif_opaque_softc *hif_ctx, uint32_t address,
					uint8_t *data, int nbytes)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	qdf_dma_addr_t buf;
	unsigned int completed_nbytes, orig_nbytes, remaining_bytes;
	unsigned int id;
	unsigned int flags;
	struct CE_handle *ce_diag;
	qdf_dma_addr_t CE_data;      /* Host buffer address in CE space */
	qdf_dma_addr_t CE_data_base = 0;
	void *data_buf = NULL;
	int i;
	unsigned int mux_id = 0;
	unsigned int transaction_id = 0xffff;
	qdf_dma_addr_t ce_phy_addr = address;
	unsigned int toeplitz_hash_result;
	unsigned int user_flags = 0;

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
			return QDF_STATUS_E_INVAL;

		while ((nbytes >= 4) &&
		       (QDF_STATUS_SUCCESS == (status =
				 hif_diag_read_access(hif_ctx, address,
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
	data_buf = qdf_mem_alloc_consistent(scn->qdf_dev, scn->qdf_dev->dev,
				    orig_nbytes, &CE_data_base);
	if (!data_buf) {
		status = QDF_STATUS_E_NOMEM;
		goto done;
	}
	qdf_mem_set(data_buf, orig_nbytes, 0);
	qdf_mem_dma_sync_single_for_device(scn->qdf_dev, CE_data_base,
				       orig_nbytes, DMA_FROM_DEVICE);

	remaining_bytes = orig_nbytes;
	CE_data = CE_data_base;
	while (remaining_bytes) {
		nbytes = min(remaining_bytes, DIAG_TRANSFER_LIMIT);
		{
			status = ce_recv_buf_enqueue(ce_diag, NULL, CE_data);
			if (status != QDF_STATUS_SUCCESS)
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
			if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
				return QDF_STATUS_E_FAILURE;
			ce_phy_addr =
				TARG_CPU_SPACE_TO_CE_SPACE(scn->mem, address);
			if (Q_TARGET_ACCESS_END(scn) < 0)
				return QDF_STATUS_E_FAILURE;

			status =
				ce_send(ce_diag, NULL, ce_phy_addr, nbytes,
					transaction_id, 0, user_flags);
			if (status != QDF_STATUS_SUCCESS)
				goto done;
		}

		i = 0;
		while (ce_completed_send_next(ce_diag, NULL, NULL, &buf,
				&completed_nbytes, &id, NULL, NULL,
				&toeplitz_hash_result) != QDF_STATUS_SUCCESS) {
			qdf_mdelay(1);
			if (i++ > DIAG_ACCESS_CE_TIMEOUT_MS) {
				status = QDF_STATUS_E_BUSY;
				goto done;
			}
		}
		if (nbytes != completed_nbytes) {
			status = QDF_STATUS_E_FAILURE;
			goto done;
		}
		if (buf != ce_phy_addr) {
			status = QDF_STATUS_E_FAILURE;
			goto done;
		}

		i = 0;
		while (ce_completed_recv_next
				(ce_diag, NULL, NULL, &buf,
				&completed_nbytes, &id,
				 &flags) != QDF_STATUS_SUCCESS) {
			qdf_mdelay(1);
			if (i++ > DIAG_ACCESS_CE_TIMEOUT_MS) {
				status = QDF_STATUS_E_BUSY;
				goto done;
			}
		}
		if (nbytes != completed_nbytes) {
			status = QDF_STATUS_E_FAILURE;
			goto done;
		}
		if (buf != CE_data) {
			status = QDF_STATUS_E_FAILURE;
			goto done;
		}

		remaining_bytes -= nbytes;
		address += nbytes;
		CE_data += nbytes;
	}

done:
	A_TARGET_ACCESS_UNLIKELY(scn);

	if (status == QDF_STATUS_SUCCESS)
		qdf_mem_copy(data, data_buf, orig_nbytes);
	else
		HIF_ERROR("%s failure (0x%x)", __func__, address);

	if (data_buf)
		qdf_mem_free_consistent(scn->qdf_dev, scn->qdf_dev->dev,
				orig_nbytes, data_buf, CE_data_base, 0);

	return status;
}

/* Read 4-byte aligned data from Target memory or register */
QDF_STATUS hif_diag_read_access(struct hif_opaque_softc *hif_ctx,
				uint32_t address, uint32_t *data)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (address >= DRAM_BASE_ADDRESS) {
		/* Assume range doesn't cross this boundary */
		return hif_diag_read_mem(hif_ctx, address, (uint8_t *) data,
					 sizeof(uint32_t));
	} else {
		if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
			return QDF_STATUS_E_FAILURE;
		*data = A_TARGET_READ(scn, address);
		if (Q_TARGET_ACCESS_END(scn) < 0)
			return QDF_STATUS_E_FAILURE;

		return QDF_STATUS_SUCCESS;
	}
}

QDF_STATUS hif_diag_write_mem(struct hif_opaque_softc *hif_ctx,
			      uint32_t address, uint8_t *data, int nbytes)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	qdf_dma_addr_t buf;
	unsigned int completed_nbytes, orig_nbytes, remaining_bytes;
	unsigned int id;
	unsigned int flags;
	struct CE_handle *ce_diag;
	void *data_buf = NULL;
	qdf_dma_addr_t CE_data;      /* Host buffer address in CE space */
	qdf_dma_addr_t CE_data_base = 0;
	int i;
	unsigned int mux_id = 0;
	unsigned int transaction_id = 0xffff;
	qdf_dma_addr_t ce_phy_addr = address;
	unsigned int toeplitz_hash_result;
	unsigned int user_flags = 0;

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
	data_buf = qdf_mem_alloc_consistent(scn->qdf_dev, scn->qdf_dev->dev,
				    orig_nbytes, &CE_data_base);
	if (!data_buf) {
		status = A_NO_MEMORY;
		goto done;
	}

	/* Copy caller's data to allocated DMA buf */
	qdf_mem_copy(data_buf, data, orig_nbytes);
	qdf_mem_dma_sync_single_for_device(scn->qdf_dev, CE_data_base,
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
	if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
		return QDF_STATUS_E_FAILURE;
	ce_phy_addr = TARG_CPU_SPACE_TO_CE_SPACE(scn->mem, address);
	if (Q_TARGET_ACCESS_END(scn) < 0)
		return QDF_STATUS_E_FAILURE;

	remaining_bytes = orig_nbytes;
	CE_data = CE_data_base;
	while (remaining_bytes) {
		nbytes = min(remaining_bytes, DIAG_TRANSFER_LIMIT);

		{   /* Set up to receive directly into Target(!) address */
			status = ce_recv_buf_enqueue(ce_diag,
						NULL, ce_phy_addr);
			if (status != QDF_STATUS_SUCCESS)
				goto done;
		}

		{
			/*
			 * Request CE to send caller-supplied data that
			 * was copied to bounce buffer to Target(!) address.
			 */
			status =
				ce_send(ce_diag, NULL,
					(qdf_dma_addr_t) CE_data, nbytes,
					transaction_id, 0, user_flags);
			if (status != QDF_STATUS_SUCCESS)
				goto done;
		}

		i = 0;
		while (ce_completed_send_next(ce_diag, NULL, NULL, &buf,
			    &completed_nbytes, &id,
			    NULL, NULL, &toeplitz_hash_result) !=
			    QDF_STATUS_SUCCESS) {
			qdf_mdelay(1);
			if (i++ > DIAG_ACCESS_CE_TIMEOUT_MS) {
				status = QDF_STATUS_E_BUSY;
				goto done;
			}
		}

		if (nbytes != completed_nbytes) {
			status = QDF_STATUS_E_FAILURE;
			goto done;
		}

		if (buf != CE_data) {
			status = QDF_STATUS_E_FAILURE;
			goto done;
		}

		i = 0;
		while (ce_completed_recv_next
			(ce_diag, NULL, NULL, &buf,
			&completed_nbytes, &id,
			&flags) != QDF_STATUS_SUCCESS) {
			qdf_mdelay(1);
			if (i++ > DIAG_ACCESS_CE_TIMEOUT_MS) {
				status = QDF_STATUS_E_BUSY;
				goto done;
			}
		}

		if (nbytes != completed_nbytes) {
			status = QDF_STATUS_E_FAILURE;
			goto done;
		}

		if (buf != ce_phy_addr) {
			status = QDF_STATUS_E_FAILURE;
			goto done;
		}

		remaining_bytes -= nbytes;
		address += nbytes;
		CE_data += nbytes;
	}

done:
	A_TARGET_ACCESS_UNLIKELY(scn);

	if (data_buf) {
		qdf_mem_free_consistent(scn->qdf_dev, scn->qdf_dev->dev,
				orig_nbytes, data_buf, CE_data_base, 0);
	}

	if (status != QDF_STATUS_SUCCESS) {
		HIF_ERROR("%s failure (0x%llu)", __func__,
			(uint64_t)ce_phy_addr);
	}

	return status;
}

/* Write 4B data to Target memory or register */
QDF_STATUS hif_diag_write_access(struct hif_opaque_softc *hif_ctx,
				 uint32_t address, uint32_t data)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (address >= DRAM_BASE_ADDRESS) {
		/* Assume range doesn't cross this boundary */
		uint32_t data_buf = data;

		return hif_diag_write_mem(hif_ctx, address,
					  (uint8_t *) &data_buf,
					  sizeof(uint32_t));
	} else {
		if (Q_TARGET_ACCESS_BEGIN(scn) < 0)
			return QDF_STATUS_E_FAILURE;
		A_TARGET_WRITE(scn, address, data);
		if (Q_TARGET_ACCESS_END(scn) < 0)
			return QDF_STATUS_E_FAILURE;

		return QDF_STATUS_SUCCESS;
	}
}
