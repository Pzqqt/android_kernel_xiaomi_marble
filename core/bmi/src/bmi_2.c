/*
 * copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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
#include "i_bmi.h"
#include "cds_api.h"
/* This need to defined in firmware interface files.
 * Defining here to address compilation issues.
 * Will be deleted once firmware interface files for
 * target are merged
 */
#define BMI_LOAD_IMAGE              18

QDF_STATUS
bmi_no_command(struct ol_context *ol_ctx)
{
	struct hif_opaque_softc *scn = ol_ctx->scn;
	uint32_t cid;
	int status;
	uint32_t length;
	uint8_t ret = 0;
	struct bmi_info *info = GET_BMI_CONTEXT(ol_ctx);
	uint8_t *bmi_cmd_buff = info->bmi_cmd_buff;
	uint8_t *bmi_rsp_buff = info->bmi_rsp_buff;
	qdf_dma_addr_t cmd = info->bmi_cmd_da;
	qdf_dma_addr_t rsp = info->bmi_rsp_da;

	if (info->bmi_done) {
		BMI_ERR("Command disallowed: BMI DONE ALREADY");
		return QDF_STATUS_E_PERM;
	}

	if (!bmi_cmd_buff || !bmi_rsp_buff) {
		BMI_ERR("No Memory Allocated for BMI CMD/RSP Buffer");
		return QDF_STATUS_NOT_INITIALIZED;
	}
	cid = BMI_NO_COMMAND;

	qdf_mem_copy(bmi_cmd_buff, &cid, sizeof(cid));
	length = sizeof(ret);

	status = hif_exchange_bmi_msg(scn, cmd, rsp, bmi_cmd_buff, sizeof(cid),
			bmi_rsp_buff, &length, BMI_EXCHANGE_TIMEOUT_MS);

	if (status) {
		BMI_ERR("Failed to write bmi no command status:%d", status);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(&ret, bmi_rsp_buff, length);
	if (ret != 0) {
		BMI_ERR("bmi no command response error ret 0x%x", ret);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
bmi_done_local(struct ol_context *ol_ctx)
{
	struct hif_opaque_softc *scn = ol_ctx->scn;
	uint32_t cid;
	int status;
	uint32_t length;
	uint8_t ret = 0;
	struct bmi_info *info = GET_BMI_CONTEXT(ol_ctx);
	uint8_t *bmi_cmd_buff = info->bmi_cmd_buff;
	uint8_t *bmi_rsp_buff = info->bmi_rsp_buff;
	qdf_device_t cdf_dev = ol_ctx->cdf_dev;
	qdf_dma_addr_t cmd = info->bmi_cmd_da;
	qdf_dma_addr_t rsp = info->bmi_rsp_da;

	if (info->bmi_done) {
		BMI_ERR("Command disallowed");
		return QDF_STATUS_E_PERM;
	}

	if (!bmi_cmd_buff || !bmi_rsp_buff) {
		BMI_ERR("No Memory Allocated for BMI CMD/RSP Buffer");
		return QDF_STATUS_NOT_INITIALIZED;
	}

	if (!cdf_dev->dev) {
		BMI_ERR("%s Invalid Device pointer", __func__);
		return QDF_STATUS_NOT_INITIALIZED;
	}

	cid = BMI_DONE;

	qdf_mem_copy(bmi_cmd_buff, &cid, sizeof(cid));
	length = sizeof(ret);

	status = hif_exchange_bmi_msg(scn, cmd, rsp, bmi_cmd_buff, sizeof(cid),
		   bmi_rsp_buff, &length, BMI_EXCHANGE_TIMEOUT_MS);

	if (status) {
		BMI_ERR("Failed to close BMI on target status:%d", status);
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_copy(&ret, bmi_rsp_buff, length);

	if (ret != 0) {
		BMI_ERR("BMI DONE response failed:%d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	if (info->bmi_cmd_buff) {
		qdf_mem_free_consistent(cdf_dev, MAX_BMI_CMDBUF_SZ,
				    info->bmi_cmd_buff, info->bmi_cmd_da, 0);
		info->bmi_cmd_buff = NULL;
		info->bmi_cmd_da = 0;
	}

	if (info->bmi_rsp_buff) {
		qdf_mem_free_consistent(cdf_dev, MAX_BMI_CMDBUF_SZ,
				    info->bmi_rsp_buff, info->bmi_rsp_da, 0);
		info->bmi_rsp_buff = NULL;
		info->bmi_rsp_da = 0;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS bmi_write_memory(uint32_t address, uint8_t *buffer, uint32_t length,
			    struct ol_context *ol_ctx)
{
	uint32_t cid;
	int status;
	uint32_t rsp_len;
	uint8_t ret = 0;
	uint32_t offset;
	uint32_t remaining, txlen;
	const uint32_t header = sizeof(cid) + sizeof(address) + sizeof(length);
	uint8_t aligned_buffer[BMI_DATASZ_MAX];
	uint8_t *src;
	struct hif_opaque_softc *scn = ol_ctx->scn;
	struct bmi_info *info = GET_BMI_CONTEXT(ol_ctx);
	uint8_t *bmi_cmd_buff = info->bmi_cmd_buff;
	uint8_t *bmi_rsp_buff = info->bmi_rsp_buff;
	qdf_dma_addr_t cmd = info->bmi_cmd_da;
	qdf_dma_addr_t rsp = info->bmi_rsp_da;

	if (info->bmi_done) {
		BMI_ERR("Command disallowed");
		return QDF_STATUS_E_PERM;
	}

	if (!bmi_cmd_buff || !bmi_rsp_buff) {
		BMI_ERR("BMI Initialization is not happened");
		return QDF_STATUS_NOT_INITIALIZED;
	}

	bmi_assert(BMI_COMMAND_FITS(BMI_DATASZ_MAX + header));
	qdf_mem_set(bmi_cmd_buff, 0, BMI_DATASZ_MAX + header);

	cid = BMI_WRITE_MEMORY;
	rsp_len = sizeof(ret);

	remaining = length;
	while (remaining) {
		src = &buffer[length - remaining];
		if (remaining < (BMI_DATASZ_MAX - header)) {
			if (remaining & 3) {
				/* align it with 4 bytes */
				remaining = remaining + (4 - (remaining & 3));
				memcpy(aligned_buffer, src, remaining);
				src = aligned_buffer;
			}
			txlen = remaining;
		} else {
			txlen = (BMI_DATASZ_MAX - header);
		}
		offset = 0;
		qdf_mem_copy(&(bmi_cmd_buff[offset]), &cid, sizeof(cid));
		offset += sizeof(cid);
		qdf_mem_copy(&(bmi_cmd_buff[offset]), &address,
						sizeof(address));
		offset += sizeof(address);
		qdf_mem_copy(&(bmi_cmd_buff[offset]), &txlen, sizeof(txlen));
		offset += sizeof(txlen);
		qdf_mem_copy(&(bmi_cmd_buff[offset]), src, txlen);
		offset += txlen;
		status = hif_exchange_bmi_msg(scn, cmd, rsp, bmi_cmd_buff,
						offset, bmi_rsp_buff, &rsp_len,
						BMI_EXCHANGE_TIMEOUT_MS);
		if (status) {
			BMI_ERR("BMI Write Memory Failed status:%d", status);
			return QDF_STATUS_E_FAILURE;
		}
		qdf_mem_copy(&ret, bmi_rsp_buff, rsp_len);
		if (ret != 0) {
			BMI_ERR("BMI Write memory response fail: %x", ret);
			return QDF_STATUS_E_FAILURE;
		}
		remaining -= txlen; address += txlen;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
bmi_read_memory(uint32_t address, uint8_t *buffer,
		uint32_t length, struct ol_context *ol_ctx)
{
	struct hif_opaque_softc *scn = ol_ctx->scn;
	uint32_t cid;
	int status;
	uint8_t ret = 0;
	uint32_t offset;
	uint32_t remaining, rxlen, rsp_len, total_len;
	struct bmi_info *info = GET_BMI_CONTEXT(ol_ctx);
	uint8_t *bmi_cmd_buff = info->bmi_cmd_buff;
	/* note we reuse the same buffer to receive on */
	uint8_t *bmi_rsp_buff = info->bmi_rsp_buff;
	uint32_t size = sizeof(cid) + sizeof(address) + sizeof(length);
	qdf_dma_addr_t cmd = info->bmi_cmd_da;
	qdf_dma_addr_t rsp = info->bmi_rsp_da;

	if (info->bmi_done) {
		BMI_ERR("Command disallowed");
		return QDF_STATUS_E_PERM;
	}
	if (!bmi_cmd_buff || !bmi_rsp_buff) {
		BMI_ERR("BMI Initialization is not done");
		return QDF_STATUS_NOT_INITIALIZED;
	}

	bmi_assert(BMI_COMMAND_FITS(BMI_DATASZ_MAX + size));
	qdf_mem_set(bmi_cmd_buff, 0, BMI_DATASZ_MAX + size);
	qdf_mem_set(bmi_rsp_buff, 0, BMI_DATASZ_MAX + size);

	cid = BMI_READ_MEMORY;
	rsp_len = sizeof(ret);
	remaining = length;

	while (remaining) {
		rxlen = (remaining < BMI_DATASZ_MAX - rsp_len) ? remaining :
						(BMI_DATASZ_MAX - rsp_len);
		offset = 0;
		qdf_mem_copy(&(bmi_cmd_buff[offset]), &cid, sizeof(cid));
		offset += sizeof(cid);
		qdf_mem_copy(&(bmi_cmd_buff[offset]), &address,
						sizeof(address));
		offset += sizeof(address);
		qdf_mem_copy(&(bmi_cmd_buff[offset]), &rxlen, sizeof(rxlen));
		offset += sizeof(length);

		total_len = rxlen + rsp_len;

		status = hif_exchange_bmi_msg(scn, cmd, rsp,
					   bmi_cmd_buff,
					   offset,
					   bmi_rsp_buff,
					   &total_len,
					   BMI_EXCHANGE_TIMEOUT_MS);

		if (status) {
			BMI_ERR("BMI Read memory failed status:%d", status);
			return QDF_STATUS_E_FAILURE;
		}

		qdf_mem_copy(&ret, bmi_rsp_buff, rsp_len);

		if (ret != 0) {
			BMI_ERR("bmi read memory response fail %x", ret);
			return QDF_STATUS_E_FAILURE;
		}

		qdf_mem_copy(&buffer[length - remaining],
				(uint8_t *)bmi_rsp_buff	+ rsp_len, rxlen);
		remaining -= rxlen; address += rxlen;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
bmi_execute(uint32_t address, uint32_t *param, struct ol_context *ol_ctx)
{
	struct hif_opaque_softc *scn = ol_ctx->scn;
	uint32_t cid;
	int status;
	uint32_t length;
	uint8_t ret = 0;
	struct bmi_info *info = GET_BMI_CONTEXT(ol_ctx);
	uint8_t *bmi_cmd_buff = info->bmi_cmd_buff;
	uint8_t *bmi_rsp_buff = info->bmi_rsp_buff;
	qdf_dma_addr_t cmd = info->bmi_cmd_da;
	qdf_dma_addr_t rsp = info->bmi_rsp_da;

	if (info->bmi_done) {
		BMI_ERR("Command disallowed");
		return QDF_STATUS_E_PERM;
	}

	if (!bmi_cmd_buff || !bmi_rsp_buff) {
		BMI_ERR("No Memory Allocated for bmi buffers");
		return QDF_STATUS_NOT_INITIALIZED;
	}

	cid = BMI_EXECUTE;

	qdf_mem_copy(bmi_cmd_buff, &cid, sizeof(cid));
	length = sizeof(ret);

	status = hif_exchange_bmi_msg(scn, cmd, rsp, bmi_cmd_buff, sizeof(cid),
		   bmi_rsp_buff, &length, BMI_EXCHANGE_TIMEOUT_MS);

	if (status) {
		BMI_ERR("Failed to do BMI_EXECUTE status:%d", status);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(&ret, bmi_rsp_buff, length);

	if (ret != 0) {
		BMI_ERR("%s: ret 0x%x", __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
bmi_load_image(dma_addr_t address,
		uint32_t size, struct ol_context *ol_ctx)
{
	uint32_t cid;
	QDF_STATUS status;
	uint32_t offset;
	uint32_t length;
	uint8_t ret = 0;
	struct hif_opaque_softc *scn = ol_ctx->scn;
	struct bmi_info *info = GET_BMI_CONTEXT(ol_ctx);
	uint8_t *bmi_cmd_buff = info->bmi_cmd_buff;
	uint8_t *bmi_rsp_buff = info->bmi_rsp_buff;
	qdf_dma_addr_t cmd = info->bmi_cmd_da;
	qdf_dma_addr_t rsp = info->bmi_rsp_da;

	uint32_t addr_h, addr_l;

	if (info->bmi_done) {
		BMI_ERR("Command disallowed");
		return QDF_STATUS_E_PERM;
	}

	if (!bmi_cmd_buff || !bmi_rsp_buff) {
		BMI_ERR("No Memory Allocated for BMI CMD/RSP Buffer");
		return QDF_STATUS_NOT_INITIALIZED;
	}

	bmi_assert(BMI_COMMAND_FITS(sizeof(cid) + sizeof(address)));
	qdf_mem_set(bmi_cmd_buff, 0, sizeof(cid) + sizeof(address));


	BMI_DBG("%s: Enter device: 0x%p, size %d", __func__, scn, size);

	cid = BMI_LOAD_IMAGE;

	offset = 0;
	qdf_mem_copy(&(bmi_cmd_buff[offset]), &cid, sizeof(cid));
	offset += sizeof(cid);
	addr_l = address & 0xffffffff;
	addr_h = 0x00;
	qdf_mem_copy(&(bmi_cmd_buff[offset]), &addr_l, sizeof(addr_l));
	offset += sizeof(addr_l);
	qdf_mem_copy(&(bmi_cmd_buff[offset]), &addr_h, sizeof(addr_h));
	offset += sizeof(addr_h);
	qdf_mem_copy(&(bmi_cmd_buff[offset]), &size, sizeof(size));
	offset += sizeof(size);
	length = sizeof(ret);

	status = hif_exchange_bmi_msg(scn, cmd, rsp, bmi_cmd_buff, offset,
		   bmi_rsp_buff, &length, BMI_EXCHANGE_TIMEOUT_MS);

	if (status) {
		BMI_ERR("BMI Load Image Failed; status:%d", status);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(&ret, bmi_rsp_buff, length);
	if (ret != 0) {
		BMI_ERR("%s: ret 0x%x", __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS bmi_enable(struct ol_context *ol_ctx)
{
	struct hif_opaque_softc *scn = ol_ctx->scn;
	struct bmi_target_info targ_info;
	struct image_desc_info image_desc_info;
	QDF_STATUS status;
	struct hif_target_info *tgt_info;
	struct bmi_info *info = GET_BMI_CONTEXT(ol_ctx);

	if (!scn) {
		BMI_ERR("Invalid scn context");
		bmi_assert(0);
		return QDF_STATUS_NOT_INITIALIZED;
	}

	tgt_info = hif_get_target_info_handle(scn);

	if (info->bmi_cmd_buff == NULL || info->bmi_rsp_buff == NULL) {
		BMI_ERR("bmi_open failed!");
		return QDF_STATUS_NOT_INITIALIZED;
	}

	status = bmi_get_target_info(&targ_info, ol_ctx);
	if (status != QDF_STATUS_SUCCESS)
			return status;

	BMI_DBG("%s: target type 0x%x, target ver 0x%x", __func__,
	       targ_info.target_type, targ_info.target_ver);

	tgt_info->target_type = targ_info.target_type;
	tgt_info->target_version = targ_info.target_ver;

	if (cnss_get_fw_image(&image_desc_info) != 0) {
		BMI_ERR("Failed to get fw image");
		return QDF_STATUS_E_FAILURE;
	}

	status = bmi_load_image(image_desc_info.bdata_addr,
				image_desc_info.bdata_size,
				ol_ctx);
	if (status != QDF_STATUS_SUCCESS) {
		BMI_ERR("Load board data failed! status:%d", status);
		return status;
	}

	status = bmi_load_image(image_desc_info.fw_addr,
				image_desc_info.fw_size,
				ol_ctx);
	if (status != QDF_STATUS_SUCCESS)
		BMI_ERR("Load fw image failed! status:%d", status);

	return status;
}

QDF_STATUS bmi_firmware_download(struct ol_context *ol_ctx)
{
	QDF_STATUS status;

	if (NO_BMI)
		return QDF_STATUS_SUCCESS;

	status = bmi_init(ol_ctx);
	if (status != QDF_STATUS_SUCCESS) {
		BMI_ERR("BMI_INIT Failed status:%d", status);
		goto end;
	}

	status = bmi_enable(ol_ctx);
	if (status != QDF_STATUS_SUCCESS) {
		BMI_ERR("BMI_ENABLE failed status:%d\n", status);
		goto err_bmi_enable;
	}

	return status;
err_bmi_enable:
	bmi_cleanup(ol_ctx);
end:
	return status;
}
