/*
 * Copyright (c) 2013-2019 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "athdefs.h"
#include "a_types.h"
#include "a_osapi.h"
#define ATH_MODULE_NAME hif
#include "a_debug.h"
#define ATH_DEBUG_BMI  ATH_DEBUG_MAKE_MODULE_MASK(0)
#include "hif.h"
#include "bmi.h"
#include "htc_api.h"
#include "if_sdio.h"
#include "regtable_sdio.h"
#include "hif_sdio_dev.h"

#define BMI_COMMUNICATION_TIMEOUT       100000

static bool pending_events_func_check;
static uint32_t command_credits;
static uint32_t *p_bmi_cmd_credits = &command_credits;

/* BMI Access routines */

/**
 * hif_bmi_buffer_send - call to send bmi buffer
 * @device: hif context
 * @buffer: buffer
 * @length: length
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
static QDF_STATUS
hif_bmi_buffer_send(struct hif_sdio_softc *scn, struct hif_sdio_dev *device,
		    char *buffer, uint32_t length)
{
	QDF_STATUS status;
	uint32_t timeout;
	uint32_t address;
	uint32_t mbox_address[HTC_MAILBOX_NUM_MAX];

	hif_configure_device(NULL, device, HIF_DEVICE_GET_FIFO_ADDR,
			     &mbox_address[0], sizeof(mbox_address));

	*p_bmi_cmd_credits = 0;
	timeout = BMI_COMMUNICATION_TIMEOUT;

	while (timeout-- && !(*p_bmi_cmd_credits)) {
		/* Read the counter register to get the command credits */
		address =
		      COUNT_DEC_ADDRESS + (HTC_MAILBOX_NUM_MAX + ENDPOINT1) * 4;
		/* hit the credit counter with a 4-byte access, the first
		 * byte read will hit the counter and cause
		 * a decrement, while the remaining 3 bytes has no effect.
		 * The rationale behind this is to make all HIF accesses
		 * 4-byte aligned
		 */
		status =
			hif_read_write(device, address,
				       (uint8_t *) p_bmi_cmd_credits, 4,
				       HIF_RD_SYNC_BYTE_INC, NULL);
		if (status != QDF_STATUS_SUCCESS) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			  ("%s:Unable to decrement the credit count register\n",
			  __func__));
			return QDF_STATUS_E_FAILURE;
		}
		/* the counter is only 8=bits, ignore anything in the
		 * upper 3 bytes
		 */
		(*p_bmi_cmd_credits) &= 0xFF;
	}

	if (*p_bmi_cmd_credits) {
		address = mbox_address[ENDPOINT1];
		status = hif_read_write(device, address, buffer, length,
					HIF_WR_SYNC_BYTE_INC, NULL);
		if (status != QDF_STATUS_SUCCESS) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			  ("%s:Unable to send the BMI data to the device\n",
			  __func__));
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			("%s:BMI Communication timeout - hif_bmi_buffer_send\n",
			__func__));
		return QDF_STATUS_E_FAILURE;
	}

	return status;
}

#if defined(SDIO_3_0)

static QDF_STATUS
hif_bmi_read_write(struct hif_sdio_dev *device,
		   char *buffer, uint32_t length)
{
	QDF_STATUS status;

	status = hif_read_write(device, HOST_INT_STATUS_ADDRESS,
				buffer, length,
				HIF_RD_SYNC_BYTE_INC, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s:Unable to read int status reg\n",
				 __func__));
		return QDF_STATUS_E_FAILURE;
	}
	*buffer = (HOST_INT_STATUS_MBOX_DATA_GET(*buffer) & (1 << ENDPOINT1));
	return status;
}
#else

static QDF_STATUS
hif_bmi_read_write(struct hif_sdio_dev *device,
		   char *buffer, uint32_t length)
{
	QDF_STATUS status;

	status = hif_read_write(device, RX_LOOKAHEAD_VALID_ADDRESS,
				buffer, length,
				HIF_RD_SYNC_BYTE_INC, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s:Unable to read rx lookahead reg\n",
				 __func__));
		return QDF_STATUS_E_FAILURE;
	}
	*buffer &= (1 << ENDPOINT1);
	return status;
}
#endif

/**
 * hif_bmi_buffer_receive - call when bmi buffer is received
 * @device: hif context
 * @buffer: buffer
 * @length: length
 * @want_timeout: timeout is needed or not
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
static QDF_STATUS
hif_bmi_buffer_receive(struct hif_sdio_dev *device,
		       char *buffer, uint32_t length, bool want_timeout)
{
	QDF_STATUS status;
	uint32_t address;
	uint32_t mbox_address[HTC_MAILBOX_NUM_MAX];
	struct _HIF_PENDING_EVENTS_INFO hif_pending_events;

	static HIF_PENDING_EVENTS_FUNC get_pending_events_func;

	if (!pending_events_func_check) {
		/* see if the HIF layer implements an alternative
		 * function to get pending events
		 * do this only once!
		 */
		hif_configure_device(NULL, device,
				     HIF_DEVICE_GET_PENDING_EVENTS_FUNC,
				     &get_pending_events_func,
				     sizeof(get_pending_events_func));
		pending_events_func_check = true;
	}

	hif_configure_device(NULL, device, HIF_DEVICE_GET_FIFO_ADDR,
			     &mbox_address[0], sizeof(mbox_address));

	/*
	 * During normal bootup, small reads may be required.
	 * Rather than issue an HIF Read and then wait as the Target
	 * adds successive bytes to the FIFO, we wait here until
	 * we know that response data is available.
	 *
	 * This allows us to cleanly timeout on an unexpected
	 * Target failure rather than risk problems at the HIF level.  In
	 * particular, this avoids SDIO timeouts and possibly garbage
	 * data on some host controllers.  And on an interconnect
	 * such as Compact Flash (as well as some SDIO masters) which
	 * does not provide any indication on data timeout, it avoids
	 * a potential hang or garbage response.
	 *
	 * Synchronization is more difficult for reads larger than the
	 * size of the MBOX FIFO (128B), because the Target is unable
	 * to push the 129th byte of data until AFTER the Host posts an
	 * HIF Read and removes some FIFO data.  So for large reads the
	 * Host proceeds to post an HIF Read BEFORE all the data is
	 * actually available to read.  Fortunately, large BMI reads do
	 * not occur in practice -- they're supported for debug/development.
	 *
	 * So Host/Target BMI synchronization is divided into these cases:
	 *  CASE 1: length < 4
	 *        Should not happen
	 *
	 *  CASE 2: 4 <= length <= 128
	 *        Wait for first 4 bytes to be in FIFO
	 *        If CONSERVATIVE_BMI_READ is enabled, also wait for
	 *        a BMI command credit, which indicates that the ENTIRE
	 *        response is available in the the FIFO
	 *
	 *  CASE 3: length > 128
	 *        Wait for the first 4 bytes to be in FIFO
	 *
	 * For most uses, a small timeout should be sufficient and we will
	 * usually see a response quickly; but there may be some unusual
	 * (debug) cases of BMI_EXECUTE where we want an larger timeout.
	 * For now, we use an unbounded busy loop while waiting for
	 * BMI_EXECUTE.
	 *
	 * If BMI_EXECUTE ever needs to support longer-latency execution,
	 * especially in production, this code needs to be enhanced to sleep
	 * and yield.  Also note that BMI_COMMUNICATION_TIMEOUT is currently
	 * a function of Host processor speed.
	 */
	if (length >= 4) {      /* NB: Currently, always true */
		/*
		 * NB: word_available is declared static for esoteric reasons
		 * having to do with protection on some OSes.
		 */
		static uint32_t word_available;
		uint32_t timeout;

		word_available = 0;
		timeout = BMI_COMMUNICATION_TIMEOUT;
		while ((!want_timeout || timeout--) && !word_available) {

			if (get_pending_events_func) {
				status = get_pending_events_func(device,
							&hif_pending_events,
							NULL);
				if (status != QDF_STATUS_SUCCESS) {
					AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					  ("%s:Failed to get pending events\n",
					  __func__));
					break;
				}

				if (hif_pending_events.available_recv_bytes >=
							sizeof(uint32_t)) {
					word_available = 1;
				}
				continue;
			}
			status = hif_bmi_read_write(device,
					(uint8_t *) &word_available,
					sizeof(word_available));
			if (status != QDF_STATUS_SUCCESS)
				return QDF_STATUS_E_FAILURE;
		}

		if (!word_available) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s:BMI Communication timeout FIFO empty\n",
				__func__));
			return QDF_STATUS_E_FAILURE;
		}
	}

	address = mbox_address[ENDPOINT1];
	status = hif_read_write(device, address, buffer, length,
				HIF_RD_SYNC_BYTE_INC, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			("%s:Unable to read the BMI data from the device\n",
			__func__));
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_reg_based_get_target_info - to retrieve target info
 * @hif_ctx: hif context
 * @targ_info: bmi target info
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
QDF_STATUS
hif_reg_based_get_target_info(struct hif_opaque_softc *hif_ctx,
			      struct bmi_target_info *targ_info)
{
	QDF_STATUS status;
	uint32_t cid;
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *device = scn->hif_handle;

	AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			("BMI Get Target Info: Enter (device: 0x%pK)\n",
			device));
	cid = BMI_GET_TARGET_INFO;
	status = hif_bmi_buffer_send(scn, device, (char *)&cid, sizeof(cid));
	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s:Unable to write to the device\n",
				 __func__));
		return QDF_STATUS_E_FAILURE;
	}

	status = hif_bmi_buffer_receive(device,
					(char *) &targ_info->target_ver,
					sizeof(targ_info->target_ver), true);
	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			("%s:Unable to read Target Version from the device\n",
			 __func__));
		return QDF_STATUS_E_FAILURE;
	}

	if (targ_info->target_ver == TARGET_VERSION_SENTINAL) {
		/* Determine how many bytes are in the Target's targ_info */
		status = hif_bmi_buffer_receive(device,
						(char *) &targ_info->
						target_info_byte_count,
						sizeof(targ_info->
							target_info_byte_count),
						true);
		if (status != QDF_STATUS_SUCCESS) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s:Unable to read target Info\n",
					 __func__));
			return QDF_STATUS_E_FAILURE;
		}

		/*
		 * The Target's targ_info doesn't match the Host's targ_info.
		 * We need to do some backwards compatibility work to make this
		 * OK.
		 */
		QDF_ASSERT(targ_info->target_info_byte_count ==
			 sizeof(*targ_info));
		/* Read the remainder of the targ_info */
		status = hif_bmi_buffer_receive(device,
					    ((char *) targ_info) +
					    sizeof(targ_info->
						   target_info_byte_count),
					    sizeof(*targ_info) -
					    sizeof(targ_info->
						   target_info_byte_count),
					    true);
		if (status != QDF_STATUS_SUCCESS) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s:Unable to read Target Info (%d bytes)\n",
				__func__, targ_info->target_info_byte_count));
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		/*
		 * Target must be an AR6001 whose firmware does not
		 * support BMI_GET_TARGET_INFO.  Construct the data
		 * that it would have sent.
		 */
		targ_info->target_info_byte_count = sizeof(*targ_info);
		targ_info->target_type = TARGET_TYPE_AR6001;
	}

	AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			("BMI Get Target Info: Exit (ver: 0x%x type: 0x%x)\n",
			 targ_info->target_ver,
			 targ_info->target_type));

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_exchange_bmi_msg - API to handle HIF-specific BMI message exchanges
 * @hif_ctx: hif context
 * @bmi_cmd_da: bmi cmd
 * @bmi_rsp_da: bmi rsp
 * @send_message: send message
 * @length: length
 * @response_message: response message
 * @response_length: response length
 * @timeout_ms: timeout in ms
 *
 * This API is synchronous
 * and only allowed to be called from a context that can block (sleep)
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
QDF_STATUS hif_exchange_bmi_msg(struct hif_opaque_softc *hif_ctx,
				qdf_dma_addr_t bmi_cmd_da,
				qdf_dma_addr_t bmi_rsp_da,
				uint8_t *send_message,
				uint32_t length,
				uint8_t *response_message,
				uint32_t *response_length,
				uint32_t timeout_ms) {
	struct hif_sdio_softc *scn = HIF_GET_SDIO_SOFTC(hif_ctx);
	struct hif_sdio_dev *device = scn->hif_handle;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!device) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
			("%s:Null device argument\n",
			__func__));
		return QDF_STATUS_E_INVAL;
	}

	status = hif_bmi_buffer_send(scn, device, send_message, length);
	if (QDF_IS_STATUS_ERROR(status)) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s:Unable to Send Message to device\n",
				 __func__));
		return status;
	}

	if (response_message) {
		status = hif_bmi_buffer_receive(device, response_message,
						*response_length,
						timeout_ms ? true : false);
		if (QDF_IS_STATUS_ERROR(status)) {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
					("%s:Unable to read response\n",
					 __func__));
			return status;
		}
	}

	return status;
}

void hif_register_bmi_callbacks(struct hif_opaque_softc *hif_ctx)
{
}

#ifdef BRINGUP_DEBUG
#define SDIO_SCRATCH_1_ADDRESS 0x864
/*Functions used for debugging*/
/**
 * hif_bmi_write_scratch_register - API to write scratch register
 * @device: hif context
 * @buffer: buffer
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
QDF_STATUS hif_bmi_write_scratch_register(struct hif_sdio_dev *device,
				    uint32_t buffer) {
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = hif_read_write(device, SDIO_SCRATCH_1_ADDRESS,
				(uint8_t *) &buffer, 4,
				HIF_WR_SYNC_BYTE_INC, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s: Unable to write to 0x%x\n",
				 __func__, SDIO_SCRATCH_1_ADDRESS));
		return QDF_STATUS_E_FAILURE;
	}
	AR_DEBUG_PRINTF(ATH_DEBUG_ERR, ("%s: wrote 0x%x to 0x%x\n", __func__,
			 buffer, SDIO_SCRATCH_1_ADDRESS));

	return status;
}

/**
 * hif_bmi_read_scratch_register - API to read from scratch register
 * @device: hif context
 *
 * Return: QDF_STATUS_SUCCESS for success.
 */
QDF_STATUS hif_bmi_read_scratch_register(struct hif_sdio_dev *device)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t buffer = 0;

	status = hif_read_write(device, SDIO_SCRATCH_1_ADDRESS,
				(uint8_t *) &buffer, 4,
				HIF_RD_SYNC_BYTE_INC, NULL);
	if (status != QDF_STATUS_SUCCESS) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("%s: Unable to read from 0x%x\n",
				 __func__, SDIO_SCRATCH_1_ADDRESS));
		return QDF_STATUS_E_FAILURE;
	}
	AR_DEBUG_PRINTF(ATH_DEBUG_ERR, ("%s: read 0x%x from 0x%x\n", __func__,
			 buffer, SDIO_SCRATCH_1_ADDRESS));

	return status;
}
#endif
