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
 * QCA abstraction layer (QAL) device config APIs
 */

#if !defined(__I_QAL_DEVNODE_H)
#define __I_QAL_DEVNODE_H

/* Include Files */
#include <linux/of_pci.h>
#include <linux/of.h>
#include "qdf_types.h"

#define PCI_DOMAIN_ID_MIN   0x0000
#define PCI_DOMAIN_ID_MAX   0xFFFF

typedef struct device_node *__qdf_devnode_t;

/**
 * __qal_devnode_read_u32_array() - Find and read an array of 32 bit integers
 * from a property.
 * @devnode: device node from which the property value is to be read.
 * @pname: name of the property to be searched.
 * @u32_val: pointer to return value, modified only if return value is 0.
 * @elem: number of array elements to read
 *
 * Return: QDF_STATUS_SUCCESS if valid value can be decoded,
 * error code otherwise
 */
static inline QDF_STATUS
__qal_devnode_read_u32_array(const __qdf_devnode_t devnode,
			     const char *pname, u32 *u32_val, size_t elem)
{
	int ret;

	ret = of_property_read_u32_array(devnode, pname, u32_val, elem);

	return qdf_status_from_os_return(ret);
}

/**
 * __qal_devnode_read_u32() - Find and read 32 bit integer from a property.
 * @devnode: device node from which the property value is to be read.
 * @pname: name of the property to be searched.
 * @u32_val: pointer to return value, modified only if return value is 0.
 *
 * Return: QDF_STATUS_SUCCESS if valid value can be decoded,
 * error code otherwise
 */
static inline QDF_STATUS
__qal_devnode_read_u32(const __qdf_devnode_t devnode,
		       const char *pname, u32 *u32_val)
{
	int ret;

	ret = of_property_read_u32(devnode, pname, u32_val);

	return qdf_status_from_os_return(ret);
}

#endif /* __I_QAL_DEVNODE_H */
