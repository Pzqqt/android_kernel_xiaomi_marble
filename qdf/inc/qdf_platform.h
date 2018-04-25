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
 * DOC: qdf_platform.h
 * This file defines platform API abstractions.
 */

#ifndef _QDF_PLATFORM_H
#define _QDF_PLATFORM_H

/**
 * qdf_is_fw_down_callback - callback to query if fw is down
 *
 * Return: true if fw is down and false if fw is not down
 */
typedef bool (*qdf_is_fw_down_callback)(void);

/**
 * qdf_register_fw_down_callback() - API to register fw down callback
 * @is_fw_down: callback to query if fw is down or not
 *
 * Return: none
 */
void qdf_register_fw_down_callback(qdf_is_fw_down_callback is_fw_down);

/**
 * qdf_is_fw_down() - API to check if fw is down or not
 *
 * Return: true: if fw is down
 *	   false: if fw is not down
 */
bool qdf_is_fw_down(void);
#endif /*_QDF_PLATFORM_H*/
