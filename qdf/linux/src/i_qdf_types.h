/*
 * Copyright (c) 2014-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: i_qdf_types.h
 * This file provides OS dependent types API's.
 */

#if !defined(__I_QDF_TYPES_H)
#define __I_QDF_TYPES_H

#ifndef __KERNEL__
#define __iomem
#endif
#include <asm/types.h>
#include <asm/byteorder.h>
#include <linux/version.h>

#ifndef __ahdecl
#ifdef __i386__
#define __ahdecl   __attribute__((regparm(0)))
#else
#define __ahdecl
#endif
#endif

#ifdef __KERNEL__
#include <generated/autoconf.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/sched.h>
#include <linux/completion.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <asm/div64.h>
#include <linux/compiler.h>
#include <linux/dma-mapping.h>
#include <linux/wireless.h>
#include <linux/if.h>
#ifdef IPA_OFFLOAD
#include <linux/ipa.h>
#endif

#define __qdf_must_check __must_check

typedef struct sg_table __sgtable_t;

/*
 * The IDs of the various system clocks
 */
#define __QDF_CLOCK_REALTIME CLOCK_REALTIME
#define __QDF_CLOCK_MONOTONIC CLOCK_MONOTONIC

/*
 * Return values for the qdf_hrtimer_data_t callback function
 */
#define __QDF_HRTIMER_NORESTART HRTIMER_NORESTART
#define __QDF_HRTIMER_RESTART HRTIMER_RESTART

/*
 * Mode arguments of qdf_hrtimer_data_t related functions
 */
#define __QDF_HRTIMER_MODE_ABS HRTIMER_MODE_ABS
#define __QDF_HRTIMER_MODE_REL HRTIMER_MODE_REL
#define __QDF_HRTIMER_MODE_PINNED HRTIMER_MODE_PINNED

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

typedef unsigned long phys_addr_t;
typedef unsigned long __sgtable_t;

#ifndef SIOCGIWAP
#define SIOCGIWAP       0
#endif

#ifndef IWEVCUSTOM
#define IWEVCUSTOM      0
#endif

#ifndef IWEVREGISTERED
#define IWEVREGISTERED  0
#endif

#ifndef IWEVEXPIRED
#define IWEVEXPIRED     0
#endif

#ifndef SIOCGIWSCAN
#define SIOCGIWSCAN     0
#endif

#define DMA_TO_DEVICE   0
#define DMA_BIDIRECTIONAL 0
#define DMA_FROM_DEVICE 0
#define __QDF_CLOCK_REALTIME 0
#define __QDF_CLOCK_MONOTONIC 0
#define __QDF_HRTIMER_MODE_ABS 0
#define __QDF_HRTIMER_MODE_REL 0
#define __QDF_HRTIMER_MODE_PINNED 0
#define __QDF_HRTIMER_NORESTART 0
#define __QDF_HRTIMER_RESTART 0
#define __iomem
#endif /* __KERNEL__ */

/*
 * max sg that we support
 */
#define __QDF_MAX_SCATTER        1
#define __QDF_NSEC_PER_MSEC NSEC_PER_MSEC

#if defined(__LITTLE_ENDIAN_BITFIELD)
#define QDF_LITTLE_ENDIAN_MACHINE
#elif defined(__BIG_ENDIAN_BITFIELD)
#define QDF_BIG_ENDIAN_MACHINE
#else
#error  "Please fix <asm/byteorder.h>"
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 20) || !defined(__KERNEL__)
#ifndef __bool_already_defined__
#define __bool_already_defined__

/**
 * bool - This is an enum for boolean
 * @false: zero
 * @true: one
 */
typedef enum bool {
	false = 0,
	true  = 1,
} bool;
#endif /* __bool_already_defined__ */
#endif

#define __qdf_packed    __attribute__((packed))

typedef int (*__qdf_os_intr)(void *);
/**
 * Private definitions of general data types
 */
typedef dma_addr_t __qdf_dma_addr_t;
typedef size_t __qdf_dma_size_t;
typedef dma_addr_t __qdf_dma_context_t;
typedef struct net_device *__qdf_netdev_t;
typedef struct cpumask __qdf_cpu_mask;
typedef __le16 __qdf_le16_t;
typedef __le32 __qdf_le32_t;
typedef __le64 __qdf_le64_t;
typedef __be16 __qdf_be16_t;
typedef __be32 __qdf_be32_t;
typedef __be64 __qdf_be64_t;

#if defined(IPA_OFFLOAD) && defined(__KERNEL__)
typedef struct ipa_wdi_buffer_info __qdf_mem_info_t;
#else
/**
 * struct __qdf_shared_mem_info - shared mem info struct
 * @pa : physical address
 * @iova: i/o virtual address
 * @size: allocated memory size
 * @result: status
 */
typedef struct __qdf_shared_mem_info {
	phys_addr_t pa;
	unsigned long iova;
	size_t size;
	int result;
} __qdf_mem_info_t;
#endif /* IPA_OFFLOAD */

#define qdf_dma_mem_context(context) dma_addr_t context
#define qdf_get_dma_mem_context(var, field)   ((qdf_dma_context_t)(var->field))

/**
 * typedef struct __qdf_resource_t - qdf resource type
 * @paddr: Physical address
 * @paddr: Virtual address
 * @len: Length
 */
typedef struct __qdf_resource {
	unsigned long paddr;
	void __iomem *vaddr;
	unsigned long len;
} __qdf_resource_t;

struct __qdf_mempool_ctxt;

#define MAX_MEM_POOLS 64

/**
 * enum qdf_bus_type - Supported Bus types
 * @QDF_BUS_TYPE_NONE: None Bus type for error check
 * @QDF_BUS_TYPE_PCI: PCI Bus
 * @QDF_BUS_TYPE_AHB: AHB Bus
 * @QDF_BUS_TYPE_SNOC: SNOC Bus
 * @QDF_BUS_TYPE_SIM: Simulator
 * @QDF_BUS_TYPE_USB: USB Bus
 * @QDF_BUS_TYPE_IPCI: IPCI Bus
 */
enum qdf_bus_type {
	QDF_BUS_TYPE_NONE = -1,
	QDF_BUS_TYPE_PCI = 0,
	QDF_BUS_TYPE_AHB,
	QDF_BUS_TYPE_SNOC,
	QDF_BUS_TYPE_SIM,
	QDF_BUS_TYPE_SDIO,
	QDF_BUS_TYPE_USB,
	QDF_BUS_TYPE_IPCI
};

/**
 * struct __qdf_device - generic qdf device type
 * @drv: Pointer to driver
 * @drv_hdl: Pointer to driver handle
 * @drv_name: Pointer to driver name
 * @irq: IRQ
 * @dev: Pointer to device
 * @res: QDF resource
 * @func: Interrupt handler
 * @mem_pool: array of pointers to mem pool context
 * @bus_type: Bus type
 * @bid: Bus ID
 * @smmu_s1_enabled: SMMU S1 enabled or not
 * @iommu_mapping: DMA iommu mapping pointer
 */
struct __qdf_device {
	void *drv;
	void *drv_hdl;
	char *drv_name;
	int irq;
	struct device *dev;
	__qdf_resource_t res;
	__qdf_os_intr func;
	struct __qdf_mempool_ctxt *mem_pool[MAX_MEM_POOLS];
	enum qdf_bus_type bus_type;
	const struct hif_bus_id *bid;
	bool smmu_s1_enabled;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
	struct iommu_domain *domain;
#else
#ifdef ENABLE_SMMU_S1_TRANSLATION
	struct dma_iommu_mapping *iommu_mapping;
#endif
#endif
};
typedef struct __qdf_device *__qdf_device_t;

typedef size_t __qdf_size_t;
typedef off_t __qdf_off_t;
typedef void __iomem* __qdf_iomem_t;

typedef uint32_t ath_dma_addr_t;

/**
 * typedef __qdf_segment_t - segment of memory
 * @daddr: dma address
 * @len: length of segment
 */
typedef struct __qdf_segment {
	dma_addr_t  daddr;
	uint32_t    len;
} __qdf_segment_t;

/**
 * __qdf_dma_map - dma map of memory
 * @mapped: mapped address
 * @nsegs: number of segments
 * @coherent: coherency status
 * @seg: segment of memory
 */
struct __qdf_dma_map {
	uint32_t                mapped;
	uint32_t                nsegs;
	uint32_t                coherent;
	__qdf_segment_t      seg[__QDF_MAX_SCATTER];
};
typedef struct  __qdf_dma_map  *__qdf_dma_map_t;

/**
 * __qdf_net_wireless_evcode - enum for event code
 * @__QDF_IEEE80211_ASSOC: association event code
 * @__QDF_IEEE80211_REASSOC: reassociation event code
 * @__QDF_IEEE80211_DISASSOC: disassociation event code
 * @__QDF_IEEE80211_JOIN: join event code
 * @__QDF_IEEE80211_LEAVE: leave event code
 * @__QDF_IEEE80211_SCAN: scan event code
 * @__QDF_IEEE80211_REPLAY: replay event code
 * @__QDF_IEEE80211_MICHAEL:michael event code
 * @__QDF_IEEE80211_REJOIN: rejoin event code
 * @__QDF_CUSTOM_PUSH_BUTTON: push button event code
 */
enum __qdf_net_wireless_evcode {
	__QDF_IEEE80211_ASSOC = SIOCGIWAP,
	__QDF_IEEE80211_REASSOC = IWEVCUSTOM,
	__QDF_IEEE80211_DISASSOC = SIOCGIWAP,
	__QDF_IEEE80211_JOIN = IWEVREGISTERED,
	__QDF_IEEE80211_LEAVE = IWEVEXPIRED,
	__QDF_IEEE80211_SCAN = SIOCGIWSCAN,
	__QDF_IEEE80211_REPLAY = IWEVCUSTOM,
	__QDF_IEEE80211_MICHAEL = IWEVCUSTOM,
	__QDF_IEEE80211_REJOIN = IWEVCUSTOM,
	__QDF_CUSTOM_PUSH_BUTTON = IWEVCUSTOM,
};

#define __qdf_snprint             snprintf
#define __qdf_vsnprint            vsnprintf
#define __qdf_toupper            toupper
#define qdf_kstrtoint            __qdf_kstrtoint
#define qdf_kstrtouint            __qdf_kstrtouint

#define __qdf_kstrtoint          kstrtoint
#define __qdf_kstrtouint          kstrtouint

#define __QDF_DMA_BIDIRECTIONAL  DMA_BIDIRECTIONAL
#define __QDF_DMA_TO_DEVICE      DMA_TO_DEVICE
#ifndef __ubicom32__
#define __QDF_DMA_FROM_DEVICE    DMA_FROM_DEVICE
#else
#define __QDF_DMA_FROM_DEVICE    DMA_TO_DEVICE
#endif
#define __qdf_inline             inline

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

/*
 * MACROs to help with compiler and OS specifics. May need to get a little
 * more sophisticated than this and define these to specific 'VERSIONS' of
 * the compiler and OS.  Until we have a need for that, lets go with this
 */
#if defined(_MSC_VER)

#define QDF_COMPILER_MSC
/* assuming that if we build with MSC, OS is WinMobile */
#define QDF_OS_WINMOBILE

#elif defined(__GNUC__)

#define QDF_COMPILER_GNUC
#define QDF_OS_LINUX /* assuming if building with GNUC, OS is Linux */

#endif

#if defined(QDF_COMPILER_MSC)


/*
 * Does nothing on Windows.  packing individual structs is not
 * supported on the Windows compiler
 */
#define QDF_PACK_STRUCT_1
#define QDF_PACK_STRUCT_2
#define QDF_PACK_STRUCT_4
#define QDF_PACK_STRUCT_8
#define QDF_PACK_STRUCT_16

#elif defined(QDF_COMPILER_GNUC)

#else
#error "Compiling with an unknown compiler!!"
#endif

#ifndef fallthrough

#define fallthrough do {} while (0) /* fallthrough */

#endif

#endif /* __I_QDF_TYPES_H */
