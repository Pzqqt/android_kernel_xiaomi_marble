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
 * DOC: qdf_idr(ID Allocation)
 * QCA driver framework (QDF) ID allocation APIs
 */

#if !defined(__QDF_IDR_H)
#define __QDF_IDR_H

/* Include Files */
#include <qdf_types.h>
#include <qdf_status.h>
#include <i_qdf_idr.h>

/**
 * qdf_idr - platform idr object
 */
typedef __qdf_idr qdf_idr;

/**
 * qdf_idr_create() - idr initialization function
 * @idp: pointer to qdf idr
 *
 * Return: QDF status
 */
QDF_STATUS qdf_idr_create(qdf_idr *idp);

/**
 * qdf_idr_destroy() - idr deinitialization function
 * @idp: pointer to qdf idr
 *
 * Return: QDF status
 */
QDF_STATUS qdf_idr_destroy(qdf_idr *idp);

/**
 * qdf_idr_alloc() - Allocates an unused ID
 * @idp: pointer to qdf idr
 * @ptr: pointer to be associated with the new ID
 * @id:  pointer to return new ID
 *
 * Return: QDF status
 */
QDF_STATUS qdf_idr_alloc(qdf_idr *idp, void *ptr, int32_t *id);

/**
 * qdf_idr_remove() - Removes this ID from the IDR.
 * @idp: pointer to qdf idr
 * @id:  ID to be remove
 *
 * Return: QDF status
 */
QDF_STATUS qdf_idr_remove(qdf_idr *idp, int32_t id);

/**
 * qdf_idr_find() - find the user pointer from the IDR by id.
 * @idp: pointer to qdf idr
 * @id:  ID to be remove
 * @ptr: pointer to return user pointer for given ID
 *
 * Return: QDF status
 */
QDF_STATUS qdf_idr_find(qdf_idr *idp, int32_t id, void **ptr);

#endif /* __QDF_IDR_H */
