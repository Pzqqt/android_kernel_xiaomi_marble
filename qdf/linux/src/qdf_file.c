/*
 * Copyright (c) 2018-2019, 2021 The Linux Foundation. All rights reserved.
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

#include <linux/firmware.h>
#include "qdf_file.h"
#include "qdf_mem.h"
#include "qdf_module.h"
#include "qdf_status.h"
#include "qdf_trace.h"
#include "qdf_types.h"

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0))
static inline
int qdf_firmware_request_nowarn(const struct firmware **fw,
				const char *name,
				struct device *device)
{
	return firmware_request_nowarn(fw, name, device);
}
#else
static inline
int qdf_firmware_request_nowarn(const struct firmware **fw,
				const char *name,
				struct device *device)
{
	return request_firmware(fw, name, device);
}
#endif


QDF_STATUS qdf_file_read(const char *path, char **out_buf)
{
	int errno;
	const struct firmware *fw;
	char *buf;

	*out_buf = NULL;

	errno = qdf_firmware_request_nowarn(&fw, path, NULL);
	if (errno) {
		qdf_err("Failed to read file %s", path);
		return QDF_STATUS_E_FAILURE;
	}

	/* qdf_mem_malloc zeros new memory; +1 size ensures null-termination */
	buf = qdf_mem_malloc(fw->size + 1);
	if (!buf) {
		release_firmware(fw);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(buf, fw->data, fw->size);
	release_firmware(fw);
	*out_buf = buf;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_file_read);

void qdf_file_buf_free(char *file_buf)
{
	QDF_BUG(file_buf);
	if (!file_buf)
		return;

	qdf_mem_free(file_buf);
}
qdf_export_symbol(qdf_file_buf_free);

