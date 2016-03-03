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
 * @file qdf_module.h
 * This file abstracts "kernel module" semantics.
 */

#ifndef _QDF_MODULE_H
#define _QDF_MODULE_H

#include <i_qdf_module.h>

typedef uint32_t (*module_init_func_t)(void);

/**
 * qdf_virt_module_init - Specify the module's entry point.
 */
#define qdf_virt_module_init(_mod_init_func) \
	__qdf_virt_module_init(_mod_init_func)

/**
 * qdf_virt_module_exit - Specify the module's exit point.
 */
#define qdf_virt_module_exit(_mod_exit_func) \
	__qdf_virt_module_exit(_mod_exit_func)

/**
 * qdf_virt_module_name - Specify the module's name.
 */
#define qdf_virt_module_name(_name)      __qdf_virt_module_name(_name)


/**
 * qdf_export_symbol - Export a symbol from a module.
 */
#define qdf_export_symbol(_sym)         __qdf_export_symbol(_sym)

/**
 * qdf_declare_param - Declare a module parameter.
 */
#define qdf_declare_param(name, _type) __qdf_declare_param(name, _type)

#endif /*_QDF_MODULE_H*/
