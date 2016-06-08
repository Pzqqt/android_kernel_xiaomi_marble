/*
 * Copyright (c) 2013-2014, 2016 The Linux Foundation. All rights reserved.
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

/*
 * LMAC offload interface functions for WMI TLV Interface
 */

#include "ol_if_athvar.h"
#include <qdf_mem.h>         /* qdf_mem_malloc,free, etc. */
#include <osdep.h>
#include "htc_api.h"
#include "wmi.h"


/* Following macro definitions use OS or platform specific functions */
#define dummy_print(fmt, ...) {}
#define wmi_tlv_print_verbose dummy_print
#define wmi_tlv_print_error   qdf_print
#define wmi_tlv_OS_MEMCPY     OS_MEMCPY
#define wmi_tlv_OS_MEMZERO    OS_MEMZERO
#define wmi_tlv_OS_MEMMOVE    OS_MEMMOVE

#ifndef NO_DYNAMIC_MEM_ALLOC
#define wmi_tlv_os_mem_alloc(scn, ptr, numBytes) \
	{ \
		(ptr) = qdf_mem_malloc(numBytes); \
	}
#define wmi_tlv_os_mem_free   qdf_mem_free
#endif
