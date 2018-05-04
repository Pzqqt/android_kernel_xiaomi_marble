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
 * DOC: i_qdf_idr.h (ID Allocation)
 * Linux-specific definitions for QDF ID Allocation API's
 */

#if !defined(__I_QDF_IDR_H)
#define __I_QDF_IDR_H

#include <linux/idr.h>
#include <qdf_lock.h>

/**
 * struct __qdf_idr_s
 * @lock: qdf spinlock
 * @idr:  idr handler
 */
struct __qdf_idr_s {
	qdf_spinlock_t lock;
	struct idr idr;
};

typedef struct __qdf_idr_s __qdf_idr;

#endif /* __I_QDF_IDR_H */
