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

/* APIs visible to the driver */

/* BMI_1 refers QCA6174 target; the ADDR is AXI addr */
#define BMI_1_TEST_ADDR	(0xa0000)
/* BMI_2 ; */
#define BMI_2_TEST_ADDR	(0x6E0000)
/* Enable BMI_TEST COMMANDs; The Value 0x09 is randomly choosen */
#define BMI_TEST_ENABLE (0x09)

static CDF_STATUS
bmi_command_test(uint32_t command, uint32_t address, uint8_t *data,
				uint32_t length, struct ol_softc *scn)
{
	switch (command) {
	case BMI_NO_COMMAND:
		return bmi_no_command(scn);
	case BMI_WRITE_MEMORY:
		return bmi_write_memory(address, data, length, scn);
	case BMI_READ_MEMORY:
		return bmi_read_memory(address, data, length, scn);
	case BMI_EXECUTE:
		return bmi_execute(address, (uint32_t *)data, scn);
	default:
		break;
	}
	return CDF_STATUS_SUCCESS;
}

CDF_STATUS bmi_init(struct ol_softc *scn)
{
	struct bmi_info *info;
	cdf_device_t cdf_dev = cds_get_context(CDF_MODULE_ID_CDF_DEVICE);

	if (!scn) {
		BMI_ERR("Invalid scn Context");
		bmi_assert(0);
		return CDF_STATUS_NOT_INITIALIZED;
	}

	if (!cdf_dev->dev) {
		BMI_ERR("%s: Invalid Device Pointer", __func__);
		return CDF_STATUS_NOT_INITIALIZED;
	}

	info = hif_get_bmi_ctx(scn);
	info->bmi_done = false;

	if (!info->bmi_cmd_buff) {
		info->bmi_cmd_buff =
			cdf_os_mem_alloc_consistent(cdf_dev, MAX_BMI_CMDBUF_SZ,
							&info->bmi_cmd_da, 0);
		if (!info->bmi_cmd_buff) {
			BMI_ERR("No Memory for BMI Command");
			return CDF_STATUS_E_NOMEM;
		}
	}

	if (!info->bmi_rsp_buff) {
		info->bmi_rsp_buff =
			cdf_os_mem_alloc_consistent(cdf_dev, MAX_BMI_CMDBUF_SZ,
							&info->bmi_rsp_da, 0);
		if (!info->bmi_rsp_buff) {
			BMI_ERR("No Memory for BMI Response");
			goto end;
		}
	}
	return CDF_STATUS_SUCCESS;
end:
	cdf_os_mem_free_consistent(cdf_dev, MAX_BMI_CMDBUF_SZ,
				 info->bmi_cmd_buff, info->bmi_cmd_da, 0);
	info->bmi_cmd_buff = NULL;
	return CDF_STATUS_E_NOMEM;
}

void bmi_cleanup(struct ol_softc *scn)
{
	struct bmi_info *info = hif_get_bmi_ctx(scn);
	cdf_device_t cdf_dev = cds_get_context(CDF_MODULE_ID_CDF_DEVICE);

	if (!cdf_dev->dev) {
		BMI_ERR("%s: Invalid Device Pointer", __func__);
		return;
	}

	if (info->bmi_cmd_buff) {
		cdf_os_mem_free_consistent(cdf_dev, MAX_BMI_CMDBUF_SZ,
				    info->bmi_cmd_buff, info->bmi_cmd_da, 0);
		info->bmi_cmd_buff = NULL;
		info->bmi_cmd_da = 0;
	}

	if (info->bmi_rsp_buff) {
		cdf_os_mem_free_consistent(cdf_dev, MAX_BMI_CMDBUF_SZ,
				    info->bmi_rsp_buff, info->bmi_rsp_da, 0);
		info->bmi_rsp_buff = NULL;
		info->bmi_rsp_da = 0;
	}
}


CDF_STATUS bmi_done(struct ol_context *ol_ctx)
{
	CDF_STATUS status = CDF_STATUS_SUCCESS;
	struct ol_softc *scn = ol_ctx->scn;

	if (NO_BMI)
		return status;

	status = bmi_done_local(scn);

	if (status != CDF_STATUS_SUCCESS)
		BMI_ERR("BMI_DONE Failed status:%d", status);
	return status;
}

CDF_STATUS
bmi_get_target_info(struct bmi_target_info *targ_info,
						struct ol_softc *scn)
{
	int status = 0;
	struct bmi_info *info = hif_get_bmi_ctx(scn);
	uint8_t *bmi_cmd_buff = info->bmi_cmd_buff;
	uint8_t *bmi_rsp_buff = info->bmi_rsp_buff;
	uint32_t cid, length;
	cdf_dma_addr_t cmd = info->bmi_cmd_da;
	cdf_dma_addr_t rsp = info->bmi_rsp_da;

	if (info->bmi_done) {
		BMI_ERR("BMI Phase is Already Done");
		return CDF_STATUS_E_PERM;
	}

	if (!bmi_cmd_buff || !bmi_rsp_buff) {
		BMI_ERR("%s:BMI CMD/RSP Buffer is NULL", __func__);
		return CDF_STATUS_NOT_INITIALIZED;
	}
	cid = BMI_GET_TARGET_INFO;

	cdf_mem_copy(bmi_cmd_buff, &cid, sizeof(cid));
	length = sizeof(struct bmi_target_info);

	status = hif_exchange_bmi_msg(scn, cmd, rsp, bmi_cmd_buff, sizeof(cid),
					(uint8_t *)bmi_rsp_buff, &length,
					BMI_EXCHANGE_TIMEOUT_MS);
	if (status) {
		BMI_ERR("Failed to target info: status:%d", status);
		return CDF_STATUS_E_FAILURE;
	}

	cdf_mem_copy(targ_info, bmi_rsp_buff, length);
	return CDF_STATUS_SUCCESS;
}

#ifdef FEATURE_BMI_2
static inline uint32_t bmi_get_test_addr(void)
{
	return BMI_2_TEST_ADDR;
}
#else
static inline uint32_t bmi_get_test_addr(void)
{
	return BMI_1_TEST_ADDR;
}
#endif

CDF_STATUS bmi_download_firmware(struct ol_context *ol_ctx)
{
	uint8_t data[10], out[10];
	uint32_t address;
	int32_t ret;
	struct ol_softc *scn = ol_ctx->scn;

	if (NO_BMI)
		return CDF_STATUS_SUCCESS; /* no BMI for Q6 bring up */

	if (!scn) {
		BMI_ERR("Invalid scn context");
		bmi_assert(0);
		return CDF_STATUS_NOT_INITIALIZED;
	}
#ifdef CONFIG_CNSS
	if (BMI_TEST_ENABLE == cnss_get_bmi_setup()) {
		ret = snprintf(data, 10, "ABCDEFGHI");
		BMI_DBG("ret:%d writing data:%s\n", ret, data);
		address = bmi_get_test_addr();

		if (bmi_init(scn) != CDF_STATUS_SUCCESS) {
			BMI_WARN("BMI_INIT Failed; No Memory!");
			goto end;
		}
		bmi_command_test(BMI_NO_COMMAND, address, data, 9, scn);
		bmi_command_test(BMI_WRITE_MEMORY, address, data, 9, scn);
		bmi_command_test(BMI_READ_MEMORY, address, out, 9, scn);
		BMI_DBG("Output:%s", out);
	}
#endif
end:
	return bmi_firmware_download(scn);
}

CDF_STATUS
bmi_read_soc_register(uint32_t address, uint32_t *param, struct ol_softc *scn)
{
	uint32_t cid;
	int status;
	uint32_t offset, param_len;
	struct bmi_info *info = hif_get_bmi_ctx(scn);
	uint8_t *bmi_cmd_buff = info->bmi_cmd_buff;
	uint8_t *bmi_rsp_buff = info->bmi_rsp_buff;
	cdf_dma_addr_t cmd = info->bmi_cmd_da;
	cdf_dma_addr_t rsp = info->bmi_rsp_da;

	bmi_assert(BMI_COMMAND_FITS(sizeof(cid) + sizeof(address)));
	cdf_mem_set(bmi_cmd_buff, 0, sizeof(cid) + sizeof(address));
	cdf_mem_set(bmi_rsp_buff, 0, sizeof(cid) + sizeof(address));

	if (info->bmi_done) {
		BMI_DBG("Command disallowed");
		return CDF_STATUS_E_PERM;
	}

	BMI_DBG("BMI Read SOC Register:device: 0x%p, address: 0x%x",
			 scn, address);

	cid = BMI_READ_SOC_REGISTER;

	offset = 0;
	cdf_mem_copy(&(bmi_cmd_buff[offset]), &cid, sizeof(cid));
	offset += sizeof(cid);
	cdf_mem_copy(&(bmi_cmd_buff[offset]), &address, sizeof(address));
	offset += sizeof(address);
	param_len = sizeof(*param);
	status = hif_exchange_bmi_msg(scn, cmd, rsp, bmi_cmd_buff, offset,
			bmi_rsp_buff, &param_len, BMI_EXCHANGE_TIMEOUT_MS);
	if (status) {
		BMI_DBG("Unable to read from the device; status:%d", status);
		return CDF_STATUS_E_FAILURE;
	}
	cdf_mem_copy(param, bmi_rsp_buff, sizeof(*param));

	BMI_DBG("BMI Read SOC Register: Exit value: %d", *param);
	return CDF_STATUS_SUCCESS;
}

CDF_STATUS
bmi_write_soc_register(uint32_t address, uint32_t param, struct ol_softc *scn)
{
	uint32_t cid;
	int status;
	uint32_t offset;
	struct bmi_info *info = hif_get_bmi_ctx(scn);
	uint8_t *bmi_cmd_buff = info->bmi_cmd_buff;
	uint32_t size = sizeof(cid) + sizeof(address) + sizeof(param);
	cdf_dma_addr_t cmd = info->bmi_cmd_da;
	cdf_dma_addr_t rsp = info->bmi_rsp_da;

	bmi_assert(BMI_COMMAND_FITS(size));
	cdf_mem_set(bmi_cmd_buff, 0, size);

	if (info->bmi_done) {
		BMI_DBG("Command disallowed");
		return CDF_STATUS_E_FAILURE;
	}

	BMI_DBG("SOC Register Write:device:0x%p, addr:0x%x, param:%d",
						scn, address, param);

	cid = BMI_WRITE_SOC_REGISTER;

	offset = 0;
	cdf_mem_copy(&(bmi_cmd_buff[offset]), &cid, sizeof(cid));
	offset += sizeof(cid);
	cdf_mem_copy(&(bmi_cmd_buff[offset]), &address, sizeof(address));
	offset += sizeof(address);
	cdf_mem_copy(&(bmi_cmd_buff[offset]), &param, sizeof(param));
	offset += sizeof(param);
	status = hif_exchange_bmi_msg(scn, cmd, rsp, bmi_cmd_buff, offset,
						NULL, NULL, 0);
	if (status) {
		BMI_ERR("Unable to write to the device: status:%d", status);
		return CDF_STATUS_E_FAILURE;
	}

	BMI_DBG("BMI Read SOC Register: Exit");
	return CDF_STATUS_SUCCESS;
}

CDF_STATUS
bmilz_data(uint8_t *buffer, uint32_t length, struct ol_softc *scn)
{
	uint32_t cid;
	int status;
	uint32_t offset;
	uint32_t remaining, txlen;
	const uint32_t header = sizeof(cid) + sizeof(length);
	struct bmi_info *info = hif_get_bmi_ctx(scn);
	uint8_t *bmi_cmd_buff = info->bmi_cmd_buff;
	cdf_dma_addr_t cmd = info->bmi_cmd_da;
	cdf_dma_addr_t rsp = info->bmi_rsp_da;

	bmi_assert(BMI_COMMAND_FITS(BMI_DATASZ_MAX + header));
	cdf_mem_set(bmi_cmd_buff, 0, BMI_DATASZ_MAX + header);

	if (info->bmi_done) {
		BMI_ERR("Command disallowed");
		return CDF_STATUS_E_PERM;
	}

	BMI_DBG("BMI Send LZ Data: device: 0x%p, length: %d",
						scn, length);

	cid = BMI_LZ_DATA;

	remaining = length;
	while (remaining) {
		txlen = (remaining < (BMI_DATASZ_MAX - header)) ?
			remaining : (BMI_DATASZ_MAX - header);
		offset = 0;
		cdf_mem_copy(&(bmi_cmd_buff[offset]), &cid, sizeof(cid));
		offset += sizeof(cid);
		cdf_mem_copy(&(bmi_cmd_buff[offset]), &txlen, sizeof(txlen));
		offset += sizeof(txlen);
		cdf_mem_copy(&(bmi_cmd_buff[offset]),
			&buffer[length - remaining], txlen);
		offset += txlen;
		status = hif_exchange_bmi_msg(scn, cmd, rsp,
						bmi_cmd_buff, offset,
						NULL, NULL, 0);
		if (status) {
			BMI_ERR("Failed to write to the device: status:%d",
								status);
			return CDF_STATUS_E_FAILURE;
		}
		remaining -= txlen;
	}

	BMI_DBG("BMI LZ Data: Exit");

	return CDF_STATUS_SUCCESS;
}

CDF_STATUS
bmi_sign_stream_start(uint32_t address,
		      uint8_t *buffer, uint32_t length, struct ol_softc *scn)
{
	uint32_t cid;
	int status;
	uint32_t offset;
	const uint32_t header = sizeof(cid) + sizeof(address) + sizeof(length);
	uint8_t aligned_buf[BMI_DATASZ_MAX + 4];
	uint8_t *src;
	struct bmi_info *info = hif_get_bmi_ctx(scn);
	uint8_t *bmi_cmd_buff = info->bmi_cmd_buff;
	uint32_t remaining, txlen;
	cdf_dma_addr_t cmd = info->bmi_cmd_da;
	cdf_dma_addr_t rsp = info->bmi_rsp_da;

	bmi_assert(BMI_COMMAND_FITS(BMI_DATASZ_MAX + header));
	cdf_mem_set(bmi_cmd_buff, 0, BMI_DATASZ_MAX + header);

	if (info->bmi_done) {
		BMI_ERR("Command disallowed");
		return CDF_STATUS_E_PERM;
	}

	BMI_ERR("Sign Stream start:device:0x%p, addr:0x%x, length:%d",
						scn, address, length);

	cid = BMI_SIGN_STREAM_START;
	remaining = length;
	while (remaining) {
		src = &buffer[length - remaining];
		if (remaining < (BMI_DATASZ_MAX - header)) {
			if (remaining & 0x3) {
				remaining = remaining + (4 - (remaining & 0x3));
				memcpy(aligned_buf, src, remaining);
				src = aligned_buf;
			}
			txlen = remaining;
		} else {
			txlen = (BMI_DATASZ_MAX - header);
		}

		offset = 0;
		cdf_mem_copy(&(bmi_cmd_buff[offset]), &cid, sizeof(cid));
		offset += sizeof(cid);
		cdf_mem_copy(&(bmi_cmd_buff[offset]), &address,
						sizeof(address));
		offset += sizeof(offset);
		cdf_mem_copy(&(bmi_cmd_buff[offset]), &txlen, sizeof(txlen));
		offset += sizeof(txlen);
		cdf_mem_copy(&(bmi_cmd_buff[offset]), src, txlen);
		offset += txlen;
		status = hif_exchange_bmi_msg(scn, cmd, rsp,
						bmi_cmd_buff, offset, NULL,
						NULL, BMI_EXCHANGE_TIMEOUT_MS);
		if (status) {
			BMI_ERR("Unable to write to the device: status:%d",
								status);
			return CDF_STATUS_E_FAILURE;
		}
		remaining -= txlen;
	}
	BMI_DBG("BMI SIGN Stream Start: Exit");

	return CDF_STATUS_SUCCESS;
}

CDF_STATUS
bmilz_stream_start(uint32_t address, struct ol_softc *scn)
{
	uint32_t cid;
	int status;
	uint32_t offset;
	struct bmi_info *info = hif_get_bmi_ctx(scn);
	uint8_t *bmi_cmd_buff = info->bmi_cmd_buff;
	cdf_dma_addr_t cmd = info->bmi_cmd_da;
	cdf_dma_addr_t rsp = info->bmi_rsp_da;

	bmi_assert(BMI_COMMAND_FITS(sizeof(cid) + sizeof(address)));
	cdf_mem_set(bmi_cmd_buff, 0, sizeof(cid) + sizeof(address));

	if (info->bmi_done) {
		BMI_DBG("Command disallowed");
		return CDF_STATUS_E_PERM;
	}
	BMI_DBG("BMI LZ Stream Start: (device: 0x%p, address: 0x%x)",
						scn, address);

	cid = BMI_LZ_STREAM_START;
	offset = 0;
	cdf_mem_copy(&(bmi_cmd_buff[offset]), &cid, sizeof(cid));
	offset += sizeof(cid);
	cdf_mem_copy(&(bmi_cmd_buff[offset]), &address, sizeof(address));
	offset += sizeof(address);
	status = hif_exchange_bmi_msg(scn, cmd, rsp, bmi_cmd_buff, offset,
						NULL, NULL, 0);
	if (status) {
		BMI_ERR("Unable to Start LZ Stream to the device status:%d",
								status);
		return CDF_STATUS_E_FAILURE;
	}
	BMI_DBG("BMI LZ Stream: Exit");
	return CDF_STATUS_SUCCESS;
}

CDF_STATUS
bmi_fast_download(uint32_t address, uint8_t *buffer,
		  uint32_t length, struct ol_softc *scn)
{
	CDF_STATUS status = CDF_STATUS_E_FAILURE;
	uint32_t last_word = 0;
	uint32_t last_word_offset = length & ~0x3;
	uint32_t unaligned_bytes = length & 0x3;

	status = bmilz_stream_start(address, scn);
	if (status != CDF_STATUS_SUCCESS)
		goto end;

	/* copy the last word into a zero padded buffer */
	if (unaligned_bytes)
		cdf_mem_copy(&last_word, &buffer[last_word_offset],
						unaligned_bytes);

	status = bmilz_data(buffer, last_word_offset, scn);

	if (status != CDF_STATUS_SUCCESS)
		goto end;

	if (unaligned_bytes)
		status = bmilz_data((uint8_t *) &last_word, 4, scn);

	if (status != CDF_STATUS_SUCCESS)
		/*
		 * Close compressed stream and open a new (fake) one.
		 * This serves mainly to flush Target caches.
		 */
		status = bmilz_stream_start(0x00, scn);
end:
	return status;
}

/**
 * ol_cds_init() - API to initialize global CDS OL Context
 * @cdf_dev: CDF Device
 * @hif_ctx: HIF Context
 *
 * Return: Success/Failure
 */
CDF_STATUS ol_cds_init(cdf_device_t cdf_dev, void *hif_ctx)
{
	struct ol_context *ol_info;
	CDF_STATUS status = CDF_STATUS_SUCCESS;

	if (NO_BMI)
		return CDF_STATUS_SUCCESS; /* no BMI for Q6 bring up */

	status = cds_alloc_context(cds_get_global_context(), CDF_MODULE_ID_BMI,
					(void **)&ol_info, sizeof(*ol_info));

	if (status != CDF_STATUS_SUCCESS) {
		BMI_ERR("%s: CDS Allocation failed for ol_bmi context",
								__func__);
		return status;
	}

	ol_info->cdf_dev = cdf_dev;
	ol_info->scn = hif_ctx;

	return status;
}

/**
 * ol_cds_free() - API to free the global CDS OL Context
 *
 * Return: void
 */
void ol_cds_free(void)
{
	struct ol_context *ol_info = cds_get_context(CDF_MODULE_ID_BMI);

	if (NO_BMI)
		return;

	cds_free_context(cds_get_global_context(), CDF_MODULE_ID_BMI, ol_info);
}
