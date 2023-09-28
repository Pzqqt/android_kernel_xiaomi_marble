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
 * DOC: qdf_idr
 * This file provides the ability to map an ID to a pointer
 */

/* Include files */
#include <qdf_idr.h>
#include <qdf_module.h>

#define QDF_IDR_START     0x100
#define QDF_IDR_END       0

static int qdf_idr_gpf_flag(void)
{
	if (in_interrupt() || irqs_disabled() || in_atomic())
		return GFP_ATOMIC;

	return GFP_KERNEL;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 9, 0)
/**
 * __qdf_idr_alloc() - Allocates an unused ID
 * @idp:   pointer to qdf idr
 * @ptr:   pointer to be associated with the new ID
 * @start: the minimum ID
 * @end:   the maximum ID
 *
 * Return: new ID
 */
static inline int32_t
__qdf_idr_alloc(qdf_idr *idp, void *ptr, int32_t start, int32_t end)
{
	int32_t id = 0;

	idr_get_new(&idp->idr, ptr, &id);

	return id;
}
#else
static inline int32_t
__qdf_idr_alloc(qdf_idr *idp, void *ptr, int32_t start, int32_t end)
{
	return idr_alloc(&idp->idr, ptr, start, end, qdf_idr_gpf_flag());
}
#endif

QDF_STATUS qdf_idr_create(qdf_idr *idp)
{
	if (!idp)
		return QDF_STATUS_E_INVAL;

	qdf_spinlock_create(&idp->lock);

	idr_init(&idp->idr);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qdf_idr_create);

QDF_STATUS qdf_idr_destroy(qdf_idr *idp)
{
	if (!idp)
		return QDF_STATUS_E_INVAL;

	qdf_spinlock_destroy(&idp->lock);
	idr_destroy(&idp->idr);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qdf_idr_destroy);

QDF_STATUS qdf_idr_alloc(qdf_idr *idp, void *ptr, int32_t *id)
{
	int local_id;

	if (!idp || !ptr)
		return QDF_STATUS_E_INVAL;

	qdf_spinlock_acquire(&idp->lock);
	local_id = __qdf_idr_alloc(idp, ptr, QDF_IDR_START, QDF_IDR_END);
	qdf_spinlock_release(&idp->lock);
	if (local_id < QDF_IDR_START)
		return QDF_STATUS_E_FAILURE;

	*id = local_id;

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qdf_idr_alloc);

QDF_STATUS qdf_idr_remove(qdf_idr *idp, int32_t id)
{
	if (!idp || id < QDF_IDR_START)
		return QDF_STATUS_E_INVAL;

	qdf_spinlock_acquire(&idp->lock);
	if (idr_find(&idp->idr, id))
		idr_remove(&idp->idr, id);
	qdf_spinlock_release(&idp->lock);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qdf_idr_remove);

QDF_STATUS qdf_idr_find(qdf_idr *idp, int32_t id, void **ptr)
{
	if (!ptr || (id < QDF_IDR_START))
		return QDF_STATUS_E_INVAL;

	qdf_spinlock_acquire(&idp->lock);
	*ptr = idr_find(&idp->idr, id);
	qdf_spinlock_release(&idp->lock);
	if (!(*ptr))
		return QDF_STATUS_E_INVAL;
	else
		return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qdf_idr_find);

