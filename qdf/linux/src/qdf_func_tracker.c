/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#include <linux/string.h>
#include <qdf_func_tracker.h>
#include <qdf_mem.h>
#include <qdf_module.h>

#ifdef FUNC_CALL_MAP
char qdf_func_call_map_buf[QDF_FUNCTION_CALL_MAP_BUF_LEN] = {0};

void cc_func(unsigned int track)
{
	unsigned int index = 0;
	unsigned int bit = 0;

	index = track / 8;
	bit = track % 8;
	qdf_func_call_map_buf[index] |= (char)(1 << bit);
}

qdf_export_symbol(cc_func);

void qdf_get_func_call_map(char *data)
{
	qdf_mem_copy(data, qdf_func_call_map_buf,
		     QDF_FUNCTION_CALL_MAP_BUF_LEN);
}

qdf_export_symbol(qdf_get_func_call_map);

void qdf_clear_func_call_map(void)
{
	qdf_mem_zero(qdf_func_call_map_buf, QDF_FUNCTION_CALL_MAP_BUF_LEN);
}

qdf_export_symbol(qdf_clear_func_call_map);

#endif
