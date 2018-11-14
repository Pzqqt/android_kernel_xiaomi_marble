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

#ifndef QDF_FUNC_TRACKER_H
#define QDF_FUNC_TRACKER_H

#ifdef FUNC_CALL_MAP

#define QDF_FUNCTION_CALL_MAP_BUF_LEN 4096

/**
 * cc_func() - Inserts the function Id into the global
 * function map
 * @track: Function Id which needs to be inserted into the
 * Global function map.
 *
 * Return: None
 */
void cc_func(unsigned int track);

/**
 * qdf_get_func_call_map() - Copies the global function call
 * map into the given buffer
 * @data: Buffer in which the function call map needs to be
 * copied
 *
 * Return: None
 */
void qdf_get_func_call_map(char *data);

/**
 * qdf_clear_func_call_map() - Clears the global function
 * call map
 *
 * Return: None
 */
void qdf_clear_func_call_map(void);
#else
static inline void cc_func(unsigned int track)
{
}

static inline void qdf_get_func_call_map(char *data)
{
}

static inline void qdf_clear_func_call_map(void)
{
}

#endif
#endif
