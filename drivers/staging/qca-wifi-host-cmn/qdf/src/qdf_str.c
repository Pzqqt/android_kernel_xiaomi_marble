/*
 * Copyright (c) 2018, 2020 The Linux Foundation. All rights reserved.
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

#include "qdf_mem.h"
#include "qdf_module.h"
#include "qdf_str.h"
#include "qdf_trace.h"

QDF_STATUS qdf_str_dup(char **dest, const char *src)
{
	qdf_size_t size;
	char *dup;

	*dest = NULL;

	QDF_BUG(src);
	if (!src)
		return QDF_STATUS_E_INVAL;

	/* size = length + null-terminator */
	size = qdf_str_len(src) + 1;
	dup = qdf_mem_malloc(size);
	if (!dup)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_copy(dup, src, size);
	*dest = dup;

	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(qdf_str_dup);

void qdf_str_right_trim(char *str)
{
	char *end = str + qdf_str_len(str) - 1;

	while (end >= str && qdf_is_space(*end))
		end--;

	end[1] = '\0';
}
qdf_export_symbol(qdf_str_right_trim);

uint32_t
qdf_str_copy_all_before_char(char *str, uint32_t str_len,
			     char *dst, uint32_t dst_len, char c)
{
	uint32_t len = 0;

	if (!str)
		return len;

	while ((len < str_len) && (len < dst_len) &&
	       (*str != '\0') && (*str != c)) {
		*dst++ = *str++;
		len++;
	}

	return len;
}
qdf_export_symbol(qdf_str_copy_all_before_char);
