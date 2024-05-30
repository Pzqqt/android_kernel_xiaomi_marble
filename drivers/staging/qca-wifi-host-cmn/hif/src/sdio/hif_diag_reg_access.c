/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
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

#include "athdefs.h"
#include "a_types.h"
#include "a_osapi.h"
#define ATH_MODULE_NAME hif
#include "a_debug.h"

#include "targaddrs.h"
#include "hif.h"
#include "if_sdio.h"
#include "regtable_sdio.h"
#include "hif_sdio_dev.h"
#include "qdf_module.h"

#define CPU_DBG_SEL_ADDRESS                      0x00000483
#define CPU_DBG_ADDRESS                          0x00000484
#define WORD_NON_ALIGNMENT_MASK                  0x03

/**
 * hif_ar6000_set_address_window_register - set the window address register
 *                                          (using 4-byte register access).
 * @hif_device: hif context
 * @register_addr: register address
 * @addr: addr
 *
 * This mitigates host interconnect issues with non-4byte aligned bus requests,
 * some interconnects use bus adapters that impose strict limitations.
 * Since diag window access is not intended for performance critical operations,
 * the 4byte mode should be satisfactory as it generates 4X the bus activity.
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
static
QDF_STATUS hif_ar6000_set_address_window_register(
			struct hif_sdio_dev *hif_device,
			uint32_t register_addr,
			uint32_t addr)
{
	QDF_STATUS status;
	static uint32_t address;

	address = addr;
	/*AR6320,just write the 4-byte address to window register*/
	status = hif_read_write(hif_device,
				register_addr,
				(char *) (&address),
				4, HIF_WR_SYNC_BYTE_INC, NULL);

	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_LOG_ERR,
			("Cannot write 0x%x to window reg: 0x%X\n",
			 addr, register_addr));
		return status;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_diag_read_access - Read from the AR6000 through its diagnostic window.
 * @hif_ctx: hif context
 * @address: address
 * @data: data
 *
 * No cooperation from the Target is required for this.
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
QDF_STATUS hif_diag_read_access(struct hif_opaque_softc *hif_ctx,
				uint32_t address,
				uint32_t *data)
{
	QDF_STATUS status;
	static uint32_t readvalue;
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *hif_device = scn->hif_handle;

	if (address & WORD_NON_ALIGNMENT_MASK) {
		AR_DEBUG_PRINTF(ATH_LOG_ERR,
			("[%s]addr is not 4 bytes align.addr[0x%08x]\n",
			 __func__, address));
		return QDF_STATUS_E_FAILURE;
	}

	/* set window register to start read cycle */
	status = hif_ar6000_set_address_window_register(hif_device,
						WINDOW_READ_ADDR_ADDRESS,
						address);

	if (status != QDF_STATUS_SUCCESS)
		return status;

	/* read the data */
	status = hif_read_write(hif_device,
				WINDOW_DATA_ADDRESS,
				(char *) &readvalue,
				sizeof(uint32_t), HIF_RD_SYNC_BYTE_INC, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_LOG_ERR,
			("Cannot read from WINDOW_DATA_ADDRESS\n"));
		return status;
	}

	*data = readvalue;
	return status;
}

/**
 * hif_diag_write_access - Write to the AR6000 through its diagnostic window.
 * @hif_ctx: hif context
 * @address: address
 * @data: data
 *
 * No cooperation from the Target is required for this.
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
QDF_STATUS hif_diag_write_access(struct hif_opaque_softc *hif_ctx,
				 uint32_t address, uint32_t data)
{
	QDF_STATUS status;
	static uint32_t write_value;
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *hif_device = scn->hif_handle;

	if (address & WORD_NON_ALIGNMENT_MASK) {
		AR_DEBUG_PRINTF(ATH_LOG_ERR,
			("[%s]addr is not 4 bytes align.addr[0x%08x]\n",
			 __func__, address));
		return QDF_STATUS_E_FAILURE;
	}

	write_value = data;

	/* set write data */
	status = hif_read_write(hif_device,
				WINDOW_DATA_ADDRESS,
				(char *) &write_value,
				sizeof(uint32_t), HIF_WR_SYNC_BYTE_INC, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_LOG_ERR,
			("Cannot write 0x%x to WINDOW_DATA_ADDRESS\n",
			 data));
		return status;
	}

	/* set window register, which starts the write cycle */
	return hif_ar6000_set_address_window_register(hif_device,
						  WINDOW_WRITE_ADDR_ADDRESS,
						  address);
}

/**
 * hif_diag_write_mem - Write a block data to the AR6000 through its diagnostic
 *                      window.
 * @scn: hif context
 * @address: address
 * @data: data
 * @nbytes: nbytes
 *
 * This function may take some time.
 * No cooperation from the Target is required for this.
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
QDF_STATUS hif_diag_write_mem(struct hif_opaque_softc *scn, uint32_t address,
			      uint8_t *data, int nbytes)
{
	QDF_STATUS status;
	int32_t i;
	uint32_t tmp_data;

	if ((address & WORD_NON_ALIGNMENT_MASK) ||
				(nbytes & WORD_NON_ALIGNMENT_MASK)) {
		AR_DEBUG_PRINTF(ATH_LOG_ERR,
			("[%s]addr or length is not 4 bytes align.addr[0x%08x] len[0x%08x]\n",
			 __func__, address, nbytes));
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < nbytes; i += 4) {
		tmp_data =
			data[i] | (data[i + 1] << 8) | (data[i + 2] << 16) |
			(data[i + 3] << 24);
		status = hif_diag_write_access(scn, address + i, tmp_data);
		if (status != QDF_STATUS_SUCCESS) {
			AR_DEBUG_PRINTF(ATH_LOG_ERR,
				("Diag Write mem failed.addr[0x%08x] value[0x%08x]\n",
				 address + i, tmp_data));
			return status;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_diag_read_mem - Read a block data to the AR6000 through its diagnostic
 *                     window.
 * @scn: hif context
 * @data: data
 * @nbytes: nbytes
 *
 * This function may take some time.
 * No cooperation from the Target is required for this.
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
QDF_STATUS hif_diag_read_mem(struct hif_opaque_softc *scn,
			     uint32_t address, uint8_t *data,
			     int nbytes)
{
	QDF_STATUS status;
	int32_t i;
	uint32_t tmp_data;

	if ((address & WORD_NON_ALIGNMENT_MASK) ||
					(nbytes & WORD_NON_ALIGNMENT_MASK)) {
		AR_DEBUG_PRINTF(ATH_LOG_ERR,
			("[%s]addr or length is not 4 bytes align.addr[0x%08x] len[0x%08x]\n",
			 __func__, address, nbytes));
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < nbytes; i += 4) {
		status = hif_diag_read_access(scn, address + i, &tmp_data);
		if (status != QDF_STATUS_SUCCESS) {
			AR_DEBUG_PRINTF(ATH_LOG_ERR,
					("Diag Write mem failed.addr[0x%08x] value[0x%08x]\n",
					 address + i, tmp_data));
			return status;
		}
		data[i] = tmp_data & 0xff;
		data[i + 1] = tmp_data >> 8 & 0xff;
		data[i + 2] = tmp_data >> 16 & 0xff;
		data[i + 3] = tmp_data >> 24 & 0xff;
	}

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(hif_diag_read_mem);

/**
 * hif_ar6k_read_target_register - call to read target register values
 * @hif_device: hif context
 * @regsel: register selection
 * @regval: reg value
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
static QDF_STATUS hif_ar6k_read_target_register(struct hif_sdio_dev *hif_device,
					 int regsel, uint32_t *regval)
{
	QDF_STATUS status;
	char vals[4];
	char register_selection[4];

	register_selection[0] = regsel & 0xff;
	register_selection[1] = regsel & 0xff;
	register_selection[2] = regsel & 0xff;
	register_selection[3] = regsel & 0xff;
	status = hif_read_write(hif_device, CPU_DBG_SEL_ADDRESS,
				register_selection, 4,
				HIF_WR_SYNC_BYTE_FIX, NULL);

	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_LOG_ERR,
			("Cannot write CPU_DBG_SEL (%d)\n", regsel));
		return status;
	}

	status = hif_read_write(hif_device,
				CPU_DBG_ADDRESS,
				(char *) vals,
				sizeof(vals), HIF_RD_SYNC_BYTE_INC, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_LOG_ERR,
				("Cannot read from CPU_DBG_ADDRESS\n"));
		return status;
	}

	*regval = vals[0] << 0 | vals[1] << 8 |
			vals[2] << 16 | vals[3] << 24;

	return status;
}

/**
 * hif_ar6k_fetch_target_regs - call to fetch target reg values
 * @hif_device: hif context
 * @targregs: target regs
 *
 * Return: None
 */
void hif_ar6k_fetch_target_regs(struct hif_sdio_dev *hif_device,
				uint32_t *targregs)
{
	int i;
	uint32_t val;

	for (i = 0; i < AR6003_FETCH_TARG_REGS_COUNT; i++) {
		val = 0xffffffff;
		hif_ar6k_read_target_register(hif_device, i, &val);
		targregs[i] = val;
	}
}
