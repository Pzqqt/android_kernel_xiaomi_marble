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
 * DOC: qdf_dev
 * QCA driver framework (QDF) device management APIs
 */

#if !defined(__QDF_DEV_H)
#define __QDF_DEV_H

/* Include Files */
#include <qdf_types.h>

struct qdf_cpu_mask;
struct qdf_devm;
struct qdf_dev;

/**
 * qdf_dev_alloc_mem() - allocate memory
 * @qdfdev: Device handle
 * @mrptr: Pointer to the allocated memory
 * @reqsize: Allocation request in bytes
 * @mask: Property mask to be associated to the allocated memory
 *
 * This function will acquire memory to be associated with a device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qdf_dev_alloc_mem(struct qdf_dev *qdfdev, struct qdf_devm **mrptr,
		  uint32_t reqsize, uint32_t mask);

/**
 * qdf_dev_release_mem() - release memory
 * @qdfdev: Device handle
 * @mrptr: Pointer to the allocated memory
 *
 * This function will acquire memory to be associated with a device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qdf_dev_release_mem(struct qdf_dev *qdfdev, struct qdf_devm *mrptr);

/**
 * qdf_dev_modify_irq() - modify irq
 * @irnum: irq number
 * @cmask: Bitmap to be cleared for the property mask
 * @smask: Bitmap to be set for the property mask
 *
 * This function will acquire memory to be associated with a device
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qdf_dev_modify_irq_status(uint32_t irnum, unsigned long cmask,
			  unsigned long smask);

/**
 * qdf_dev_set_irq_affinity() - set irq affinity
 * @irnum: irq number
 * @cpmask: cpu affinity bitmap
 *
 * This function will set the affinity level for an irq
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS
qdf_dev_set_irq_affinity(uint32_t irnum, struct qdf_cpu_mask *cpmask);
#endif
