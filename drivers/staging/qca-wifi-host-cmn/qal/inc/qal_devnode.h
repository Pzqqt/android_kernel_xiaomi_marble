/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
 * DOC: qal_devnode
 * QCA driver framework for device node related APIs prototype
 */

#ifndef __QAL_DEVNODE_H
#define __QAL_DEVNODE_H

/* Include Files */
#include "qdf_types.h"
#include "qdf_trace.h"
#include "i_qal_devnode.h"

#define PCI_DOMAIN_ID_MIN   0x0000
#define PCI_DOMAIN_ID_MAX   0xFFFF

typedef __qdf_devnode_t qdf_devnode_t;

#ifdef ENHANCED_OS_ABSTRACTION

/**
 * qal_devnode_read_u32_array() - Find and read an array of 32 bit integers
 * from a property.
 * @devnode: device node from which the property value is to be read.
 * @pname: name of the property to be searched.
 * @u32_val: pointer to return value, modified only if return value is 0.
 * @elem: number of array elements to read
 *
 * Return: QDF_STATUS_SUCCESS on success, error code
 */
QDF_STATUS
qal_devnode_read_u32_array(const qdf_devnode_t devnode,
			   const char *pname, uint32_t *u32_val, size_t elem);
/**
 * qal_devnode_read_u32() - Find and read 32 bit integer from a property.
 * @devnode: device node from which the property value is to be read.
 * @pname: name of the property to be searched.
 * @u32_val: pointer to return value, modified only if return value is 0.
 *
 * Return: QDF_STATUS_SUCCESS on success, error code
 */
QDF_STATUS
qal_devnode_read_u32(const qdf_devnode_t devnode,
		     const char *pname, uint32_t *u32_val);
#else

static inline QDF_STATUS
qal_devnode_read_u32_array(const qdf_devnode_t devnode,
			   const char *pname, uint32_t *u32_val, size_t elem)
{
	return __qal_devnode_read_u32_array(devnode, pname, u32_val, elem);
}

static inline QDF_STATUS
qal_devnode_read_u32(const qdf_devnode_t devnode,
		     const char *pname, uint32_t *u32_val)
{
	return __qal_devnode_read_u32(devnode, pname, u32_val);
}
#endif

#endif /* __QAL_DEVNODE_H */
