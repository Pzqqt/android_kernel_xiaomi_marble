/*
 * Copyright (c) 2014-2015 The Linux Foundation. All rights reserved.
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
 * DOC: i_cdf_types.h
 *
 * Connectivity driver framework (CDF) types
 */

#if !defined(__I_CDF_TYPES_H)
#define __I_CDF_TYPES_H
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <asm/div64.h>

#ifndef __KERNEL__
#define __iomem
#endif
#include <asm/types.h>
#include <asm/byteorder.h>
#include <linux/version.h>

#ifdef __KERNEL__
#include <generated/autoconf.h>
#include <linux/compiler.h>
#include <linux/dma-mapping.h>
#include <linux/wireless.h>
#include <linux/if.h>
#else

/*
 * Hack - coexist with prior defs of dma_addr_t.
 * Eventually all other defs of dma_addr_t should be removed.
 * At that point, the "already_defined" wrapper can be removed.
 */
#ifndef __dma_addr_t_already_defined__
#define __dma_addr_t_already_defined__
typedef unsigned long dma_addr_t;
#endif

#define SIOCGIWAP       0
#define IWEVCUSTOM      0
#define IWEVREGISTERED  0
#define IWEVEXPIRED     0
#define SIOCGIWSCAN     0
#define DMA_TO_DEVICE   0
#define DMA_FROM_DEVICE 0
#define __iomem
#endif /* __KERNEL__ */

/**
 * max sg that we support
 */
#define __CDF_OS_MAX_SCATTER        1

#if defined(__LITTLE_ENDIAN_BITFIELD)
#define CDF_LITTLE_ENDIAN_MACHINE
#elif defined (__BIG_ENDIAN_BITFIELD)
#define CDF_BIG_ENDIAN_MACHINE
#else
#error  "Please fix <asm/byteorder.h>"
#endif

#define __cdf_packed          __attribute__ ((packed))

typedef int (*__cdf_os_intr)(void *);
/**
 * Private definitions of general data types
 */
typedef dma_addr_t __cdf_dma_addr_t;
typedef dma_addr_t __cdf_dma_context_t;

#define cdf_dma_mem_context(context) dma_addr_t context
#define cdf_get_dma_mem_context(var, field)   ((cdf_dma_context_t)(var->field))

/**
 * typedef struct __cdf_resource_t - cdf resource type
 * @paddr: Physical address
 * @paddr: Virtual address
 * @len: Length
 */
typedef struct __cdf_os_resource {
	unsigned long paddr;
	void __iomem *vaddr;
	unsigned long len;
} __cdf_resource_t;

/**
 * struct __cdf_device - generic cdf device type
 * @drv: Pointer to driver
 * @drv_hdl: Pointer to driver handle
 * @drv_name: Pointer to driver name
 * @irq: IRQ
 * @dev: Pointer to device
 * @res: CDF resource
 * @func: Interrupt handler
 */
struct __cdf_device {
	void *drv;
	void *drv_hdl;
	char *drv_name;
	int irq;
	struct device *dev;
	__cdf_resource_t res;
	__cdf_os_intr func;
};

typedef struct __cdf_device *__cdf_device_t;

typedef size_t __cdf_size_t;
typedef uint8_t __iomem *__cdf_iomem_t;

/**
 * typedef struct __cdf_segment_t - cdf segment
 * @daddr: DMA address
 * @len: Length
 */
typedef struct __cdf_segment {
	dma_addr_t daddr;
	uint32_t len;
} __cdf_segment_t;

/**
 * struct __cdf_dma_map - dma map
 * @mapped: dma is mapped or not
 * @nsegs: Number of segments
 * @coherent: Coherent
 * @seg: Segment array
 */
struct __cdf_dma_map {
	uint32_t mapped;
	uint32_t nsegs;
	uint32_t coherent;
	__cdf_segment_t seg[__CDF_OS_MAX_SCATTER];
};
typedef struct __cdf_dma_map *__cdf_dma_map_t;
typedef uint32_t ath_dma_addr_t;

#define __cdf_print               printk
#define __cdf_vprint              vprintk
#define __cdf_snprint             snprintf
#define __cdf_vsnprint            vsnprintf

#define __CDF_DMA_BIDIRECTIONAL  DMA_BIDIRECTIONAL
#define __CDF_DMA_TO_DEVICE      DMA_TO_DEVICE
#define __CDF_DMA_FROM_DEVICE    DMA_FROM_DEVICE
#define __cdf_inline             inline

/*
 * 1. GNU C/C++ Compiler
 *
 * How to detect gcc : __GNUC__
 * How to detect gcc version :
 *   major version : __GNUC__ (2 = 2.x, 3 = 3.x, 4 = 4.x)
 *   minor version : __GNUC_MINOR__
 *
 * 2. Microsoft C/C++ Compiler
 *
 * How to detect msc : _MSC_VER
 * How to detect msc version :
 *   _MSC_VER (1200 = MSVC 6.0, 1300 = MSVC 7.0, ...)
 *
 */

/* MACROs to help with compiler and OS specifics. May need to get a little
 * more sophisticated than this and define these to specific 'VERSIONS' of
 * the compiler and OS.  Until we have a need for that, lets go with this
 */
#if defined(_MSC_VER)

#define CDF_COMPILER_MSC
/* assuming that if we build with MSC, OS is WinMobile */
#define CDF_OS_WINMOBILE

#elif defined(__GNUC__)

#define CDF_COMPILER_GNUC
#define CDF_OS_LINUX /* assuming if building with GNUC, OS is Linux */

#endif

#if defined(CDF_COMPILER_MSC)

#define CDF_INLINE_FN  __inline

/* Does nothing on Windows.  packing individual structs is not
 * supported on the Windows compiler
 */
#define CDF_PACK_STRUCT_1
#define CDF_PACK_STRUCT_2
#define CDF_PACK_STRUCT_4
#define CDF_PACK_STRUCT_8
#define CDF_PACK_STRUCT_16

#elif defined(CDF_COMPILER_GNUC)

#define CDF_INLINE_FN  static inline

#else
#error "Compiling with an unknown compiler!!"
#endif

#endif /* __I_CDF_TYPES_H */
