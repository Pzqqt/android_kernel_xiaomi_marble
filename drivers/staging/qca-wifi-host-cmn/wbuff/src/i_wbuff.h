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
 * DOC: i_wbuff.h
 * wbuff private
 */

#ifndef _I_WBUFF_H
#define _I_WBUFF_H

#include <qdf_nbuf.h>

/* Number of modules supported by wbuff */
#define WBUFF_MAX_MODULES 4

/* Number of pools supported per module */
#define WBUFF_MAX_POOLS 4

/* Max buffer size supported by wbuff in bytes */
#define WBUFF_MAX_BUFFER_SIZE 2048

/* wbuff pool buffer lengths in bytes*/
#define WBUFF_LEN_POOL0 256
#define WBUFF_LEN_POOL1 512
#define WBUFF_LEN_POOL2 1024
#define WBUFF_LEN_POOL3 2048

/* wbuff max pool sizes */
/* Allocation of size 256 bytes */
#define WBUFF_POOL_0_MAX 256
/* Allocation of size 512 bytes */
#define WBUFF_POOL_1_MAX 128
/* Allocation of size 1024 bytes */
#define WBUFF_POOL_2_MAX 64
/* Allocation of size 2048 bytes */
#define WBUFF_POOL_3_MAX 32

#define WBUFF_MSLOT_SHIFT 4
#define WBUFF_MSLOT_BITMASK 0xF0

#define WBUFF_PSLOT_SHIFT 1
#define WBUFF_PSLOT_BITMASK 0xE

/* Comparison array for maximum allocation per pool*/
uint16_t wbuff_alloc_max[WBUFF_MAX_POOLS] = {WBUFF_POOL_0_MAX,
					     WBUFF_POOL_1_MAX,
					     WBUFF_POOL_2_MAX,
					     WBUFF_POOL_3_MAX};

/**
 * struct wbuff_handle - wbuff handle to the registered module
 * @id: the identifier for the registered module.
 */
struct wbuff_handle {
	uint8_t id;
};

/**
 * struct wbuff_module - allocation holder for wbuff registered module
 * @registered: To identify whether module is registered
 * @pending_returns: Number of buffers pending to be returned to
 * wbuff by the module
 * @lock: Lock for accessing per module buffer slots
 * @handle: wbuff handle for the registered module
 * @reserve: nbuf headroom to start with
 * @align: alignment for the nbuf
 * @pool[]: pools for all available buffers for the module
 */
struct wbuff_module {
	bool registered;
	uint16_t pending_returns;
	qdf_spinlock_t lock;
	struct wbuff_handle handle;
	int reserve;
	int align;
	qdf_nbuf_t pool[WBUFF_MAX_POOLS];
};

/**
 * struct wbuff_holder - allocation holder for wbuff
 * @initialized: to identified whether module is initialized
 */
struct wbuff_holder {
	bool initialized;
	struct wbuff_module mod[WBUFF_MAX_MODULES];
};
#endif /* _WBUFF_H */
