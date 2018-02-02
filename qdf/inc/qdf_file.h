/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: Thin filesystem API abstractions
 */

#ifndef __QDF_FILE_H
#define __QDF_FILE_H

#include "qdf_status.h"

/**
 * qdf_file_read() - read the entire contents of a file
 * @path: the full path of the file to read
 * @out_buf: double pointer for referring to the file contents buffer
 *
 * This API allocates a new, null-terminated buffer containing the contents of
 * the file at @path. On success, @out_buf points to this new buffer, otherwise
 * @out_buf is set to NULL.
 *
 * Consumers must free the allocated buffer by calling qdf_file_buf_free().
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_file_read(const char *path, char **out_buf);

/**
 * qdf_file_buf_free() - free a previously allocated file buffer
 * @file_buf: pointer to the file buffer to free
 *
 * This API is used in conjunction with qdf_file_read().
 *
 * Return: None
 */
void qdf_file_buf_free(char *file_buf);

#endif /* __QDF_FILE_H */

