/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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

#define __qdf_virt_module_name(_name) MODULE_LICENSE("Dual BSD/GPL")

#ifdef WLAN_DISABLE_EXPORT_SYMBOL
#define __qdf_export_symbol(_sym)
#else
#define __qdf_export_symbol(_sym) EXPORT_SYMBOL(_sym)
#endif

#define __qdf_declare_param(_name, _type) \
	module_param(_name, _type, 0600)

#define __qdf_declare_param_array(_name, _type, _num) \
	module_param_array(_name, _type, _num, 0600)

#endif /* _I_QDF_MODULE_H */
