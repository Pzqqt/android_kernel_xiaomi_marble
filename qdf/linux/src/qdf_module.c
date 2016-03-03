/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

#include <linux/module.h>
#include <qdf_perf.h>

MODULE_AUTHOR("Qualcomm Atheros Inc.");
MODULE_DESCRIPTION("Qualcomm Atheros Device Framework Module");
MODULE_LICENSE("Dual BSD/GPL");

#ifndef EXPORT_SYMTAB
#define EXPORT_SYMTAB
#endif

/**
 * qdf_mod_init() - module initialization
 *
 * Return: int
 */
static int __init
qdf_mod_init(void)
{
	qdf_perfmod_init();
	return 0;
}
module_init(qdf_mod_init);

/**
 * qdf_mod_exit() - module remove
 *
 * Return: int
 */
static void __exit
qdf_mod_exit(void)
{
	qdf_perfmod_exit();
}
module_exit(qdf_mod_exit);

