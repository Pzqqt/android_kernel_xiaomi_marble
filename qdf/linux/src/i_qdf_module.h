/*
 * Copyright (c) 2010-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: i_qdf_module.h
 * Linux-specific definitions for QDF module API's
 */

#ifndef _I_QDF_MODULE_H
#define _I_QDF_MODULE_H

#include <linux/version.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <qdf_types.h>


#define __qdf_virt_module_init(_x)  \
	static int _x##_mod(void) \
	{                   \
		uint32_t st;  \
		st = (_x)();         \
		if (st != QDF_STATUS_SUCCESS)  \
			return QDF_STATUS_E_INVAL;            \
		else                    \
			return 0;             \
	}                           \
	module_init(_x##_mod);

#define __qdf_virt_module_exit(_x)  module_exit(_x)

#define __qdf_virt_module_name(_name) MODULE_LICENSE("Proprietary");

#define __qdf_export_symbol(_sym) EXPORT_SYMBOL(_sym)

#define __qdf_declare_param(_name, _type) \
	module_param(_name, _type, 0600)

#endif /* _I_QDF_MODULE_H */
