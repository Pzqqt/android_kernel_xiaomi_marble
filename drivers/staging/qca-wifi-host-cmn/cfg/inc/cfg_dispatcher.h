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
 * DOC: Dispatcher related handler APIs for the configuration component
 */
#ifndef __CFG_DISPATCHER_H_
#define __CFG_DISPATCHER_H_

#include <qdf_status.h>

/**
 * cfg_dispatcher_init() - Configuration component global init handler
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cfg_dispatcher_init(void);

/**
 * cfg_dispatcher_deinit() - Configuration component global deinit handler
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cfg_dispatcher_deinit(void);

#endif /* __CFG_DISPATCHER_H */
