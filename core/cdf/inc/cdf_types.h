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

#if !defined(__CDF_TYPES_H)
#define __CDF_TYPES_H
/**
 * DOC: cdf_types.h
 *
 * Connectivity driver framework (CDF) basic type definitions
 */

/* Include Files */
#include "i_cdf_types.h"
#include <string.h>

/* Preprocessor definitions and constants */

/**
 * CDF_MAX - get maximum of two values
 * @_x: 1st arguement
 * @_y: 2nd arguement
 */
#define CDF_MAX(_x, _y) (((_x) > (_y)) ? (_x) : (_y))

/**
 * CDF_MIN - get minimum of two values
 * @_x: 1st arguement
 * @_y: 2nd arguement
 */
#define CDF_MIN(_x, _y) (((_x) < (_y)) ? (_x) : (_y))

/**
 * CDF_SWAP_U16 - swap input u16 value
 * @_x: variable to swap
 */
#define CDF_SWAP_U16(_x) \
	((((_x) << 8) & 0xFF00) | (((_x) >> 8) & 0x00FF))

/**
 * CDF_SWAP_U32 - swap input u32 value
 * @_x: variable to swap
 */
#define CDF_SWAP_U32(_x) \
	(((((_x) << 24) & 0xFF000000) | (((_x) >> 24) & 0x000000FF)) | \
	 ((((_x) << 8) & 0x00FF0000) | (((_x) >> 8) & 0x0000FF00)))

#define CDF_TICKS_PER_SECOND        (1000)

/**
 * CDF_ARRAY_SIZE - get array size
 * @_arr: array variable name
 */
#define CDF_ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof((_arr)[0]))

/* endian operations for Big Endian and Small Endian modes */
#ifdef ANI_LITTLE_BYTE_ENDIAN

#define cdf_be16_to_cpu(_x) CDF_SWAP_U16(_x)

#endif

#ifdef ANI_BIG_BYTE_ENDIAN

#define cdf_be16_to_cpu(_x) (_x)

#endif

#ifndef __ahdecl
#ifdef __i386__
#define __ahdecl   __attribute__((regparm(0)))
#else
#define __ahdecl
#endif
#endif

#define CDF_OS_MAX_SCATTER  __CDF_OS_MAX_SCATTER

/**
 * @brief denotes structure is packed.
 */
#define cdf_packed __cdf_packed

/**
 * typedef cdf_handle_t - handles opaque to each other
 */
typedef void *cdf_handle_t;

/**
 * typedef cdf_device_t - Platform/bus generic handle.
 *			  Used for bus specific functions.
 */
typedef __cdf_device_t cdf_device_t;

/**
 * typedef cdf_size_t - size of an object
 */
typedef __cdf_size_t cdf_size_t;

/**
 * typedef cdf_dma_map_t - DMA mapping object.
 */
typedef __cdf_dma_map_t cdf_dma_map_t;

/**
 * tyepdef cdf_dma_addr_t - DMA address.
 */
typedef __cdf_dma_addr_t cdf_dma_addr_t;

/**
 * tyepdef cdf_dma_context_t - DMA context.
 */
typedef __cdf_dma_context_t cdf_dma_context_t;


#define cdf_iomem_t   __cdf_iomem_t;
/**
 * typedef enum CDF_TIMER_TYPE - CDF timer type
 * @CDF_TIMER_TYPE_SW: Deferrable SW timer it will not cause CPU to wake up
 *			on expiry
 * @CDF_TIMER_TYPE_WAKE_APPS:  Non deferrable timer which will cause CPU to
 *				wake up on expiry
 */
typedef enum {
	CDF_TIMER_TYPE_SW,
	CDF_TIMER_TYPE_WAKE_APPS
} CDF_TIMER_TYPE;

/**
 * tyepdef cdf_resource_type_t - hw resources
 *
 * @CDF_RESOURCE_TYPE_MEM: memory resource
 * @CDF_RESOURCE_TYPE_IO: io resource
 *
 * Define the hw resources the OS has allocated for the device
 * Note that start defines a mapped area.
 */
typedef enum {
	CDF_RESOURCE_TYPE_MEM,
	CDF_RESOURCE_TYPE_IO,
} cdf_resource_type_t;

/**
 * tyepdef cdf_resource_t - representation of a h/w resource.
 *
 * @start: start
 * @end: end
 * @type: resource type
 */
typedef struct {
	uint64_t start;
	uint64_t end;
	cdf_resource_type_t type;
} cdf_resource_t;

/**
 * typedef cdf_dma_dir_t - DMA directions
 *
 * @CDF_DMA_BIDIRECTIONAL: bidirectional data
 * @CDF_DMA_TO_DEVICE: data going from device to memory
 * @CDF_DMA_FROM_DEVICE: data going from memory to device
 */
typedef enum {
	CDF_DMA_BIDIRECTIONAL = __CDF_DMA_BIDIRECTIONAL,
	CDF_DMA_TO_DEVICE = __CDF_DMA_TO_DEVICE,
	CDF_DMA_FROM_DEVICE = __CDF_DMA_FROM_DEVICE,
} cdf_dma_dir_t;

/* work queue(kernel thread)/DPC function callback */
typedef void (*cdf_defer_fn_t)(void *);

/* Prototype of the critical region function that is to be
 * executed with spinlock held and interrupt disalbed
 */
typedef bool (*cdf_irqlocked_func_t)(void *);

/* Prototype of timer function */
typedef void (*cdf_softirq_timer_func_t)(void *);

#define cdf_offsetof(type, field) offsetof(type, field)

/**
 * typedef CDF_MODULE_ID - CDF Module IDs
 *
 * @CDF_MODULE_ID_TLSHIM: TLSHIM module ID
 * @CDF_MODULE_ID_WMI: WMI module ID
 * @CDF_MODULE_ID_HTT: HTT module ID
 * @CDF_MODULE_ID_RSV4: Reserved
 * @CDF_MODULE_ID_HDD: HDD module ID
 * @CDF_MODULE_ID_SME: SME module ID
 * @CDF_MODULE_ID_PE: PE module ID
 * @CDF_MODULE_ID_WMA: WMA module ID
 * @CDF_MODULE_ID_SYS: SYS module ID
 * @CDF_MODULE_ID_CDF: CDF module ID
 * @CDF_MODULE_ID_SAP: SAP module ID
 * @CDF_MODULE_ID_HDD_SOFTAP: HDD SAP module ID
 * @CDF_MODULE_ID_HDD_DATA: HDD DATA module ID
 * @CDF_MODULE_ID_HDD_SAP_DATA: HDD SAP DATA module ID
 * @CDF_MODULE_ID_HIF: HIF module ID
 * @CDF_MODULE_ID_HTC: HTC module ID
 * @CDF_MODULE_ID_TXRX: TXRX module ID
 * @CDF_MODULE_ID_CDF_DEVICE: CDF DEVICE module ID
 * @CDF_MODULE_ID_CFG: CFG module ID
 * @CDF_MODULE_ID_BMI: BMI module ID
 * @CDF_MODULE_ID_EPPING: EPPING module ID
 * @CDF_MODULE_ID_MAX: Max place holder module ID
 *
 * These are generic IDs that identify the various modules in the software
 * system
 * 0 is unused for historical purposes
 * 3 & 4 are unused for historical purposes
 */
typedef enum {
	CDF_MODULE_ID_TLSHIM = 1,
	CDF_MODULE_ID_WMI = 2,
	CDF_MODULE_ID_HTT = 3,
	CDF_MODULE_ID_RSV4 = 4,
	CDF_MODULE_ID_HDD = 5,
	CDF_MODULE_ID_SME = 6,
	CDF_MODULE_ID_PE = 7,
	CDF_MODULE_ID_WMA = 8,
	CDF_MODULE_ID_SYS = 9,
	CDF_MODULE_ID_CDF = 10,
	CDF_MODULE_ID_SAP = 11,
	CDF_MODULE_ID_HDD_SOFTAP = 12,
	CDF_MODULE_ID_HDD_DATA = 14,
	CDF_MODULE_ID_HDD_SAP_DATA = 15,

	CDF_MODULE_ID_HIF = 16,
	CDF_MODULE_ID_HTC = 17,
	CDF_MODULE_ID_TXRX = 18,
	CDF_MODULE_ID_CDF_DEVICE = 19,
	CDF_MODULE_ID_CFG = 20,
	CDF_MODULE_ID_BMI = 21,
	CDF_MODULE_ID_EPPING = 22,

	CDF_MODULE_ID_MAX
} CDF_MODULE_ID;

#define cdf_print(args...) \
	CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR, ## args)
#define cdf_vprint        __cdf_vprint
#define cdf_snprint       __cdf_snprint

/**
 * enum tCDF_ADAPTER_MODE - adapter role.
 *
 * @CDF_STA_MODE: STA mode
 * @CDF_SAP_MODE: SAP mode
 * @CDF_P2P_CLIENT_MODE: P2P client mode
 * @CDF_P2P_GO_MODE: P2P GO mode
 * @CDF_FTM_MODE: FTM mode
 * @CDF_IBSS_MODE: IBSS mode
 * @CDF_P2P_DEVICE_MODE: P2P device mode
 * @CDF_EPPING_MODE: EPPING device mode
 * @CDF_OCB_MODE: OCB device mode
 * @CDF_MAX_NO_OF_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum tCDF_ADAPTER_MODE {
	CDF_STA_MODE = 0,
	CDF_SAP_MODE = 1,
	CDF_P2P_CLIENT_MODE,
	CDF_P2P_GO_MODE,
	CDF_FTM_MODE,
	CDF_IBSS_MODE,
	CDF_P2P_DEVICE_MODE,
	CDF_EPPING_MODE,
	CDF_OCB_MODE,
	CDF_MAX_NO_OF_MODE
};

/**
 * enum tCDF_GLOBAL_CON_MODE - global config mode when
 * driver is loaded.
 *
 * @CDF_GLOBAL_MISSION_MODE: mission mode (STA, SAP...)
 * @CDF_GLOBAL_FTM_MODE: FTM mode
 * @CDF_GLOBAL_EPPING_MODE: EPPING mode
 * @CDF_GLOBAL_MAX_MODE: Max place holder
 */
enum tCDF_GLOBAL_CON_MODE {
	CDF_GLOBAL_MISSION_MODE,
	CDF_GLOBAL_FTM_MODE = 5,
	CDF_GLOBAL_EPPING_MODE = 8,
	CDF_GLOBAL_MAX_MODE
};


#ifdef WLAN_OPEN_P2P_INTERFACE
/* This should match with WLAN_MAX_INTERFACES */
#define CDF_MAX_CONCURRENCY_PERSONA    (4)
#else
#define CDF_MAX_CONCURRENCY_PERSONA    (3)
#endif

#define CDF_STA_MASK (1 << CDF_STA_MODE)
#define CDF_SAP_MASK (1 << CDF_SAP_MODE)
#define CDF_P2P_CLIENT_MASK (1 << CDF_P2P_CLIENT_MODE)
#define CDF_P2P_GO_MASK (1 << CDF_P2P_GO_MODE)

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
typedef enum {
	CDF_MCC_TO_SCC_SWITCH_DISABLE = 0,
	CDF_MCC_TO_SCC_SWITCH_ENABLE,
	CDF_MCC_TO_SCC_SWITCH_FORCE,
	CDF_MCC_TO_SCC_SWITCH_MAX
} tCDF_MCC_TO_SCC_SWITCH_MODE;
#endif

#if !defined(NULL)
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif

/* 'Time' type */
typedef unsigned long v_TIME_t;

/* typedef for CDF Context... */
typedef void *v_CONTEXT_t;

#define CDF_MAC_ADDR_SIZE (6)

/**
 * struct cdf_mac_addr - mac address array
 * @bytes: MAC address bytes
 */
struct cdf_mac_addr {
	uint8_t bytes[CDF_MAC_ADDR_SIZE];
};

/* This macro is used to initialize a CDF MacAddress to the broadcast
 * MacAddress.  It is used like this...
 * struct cdf_mac_addr macAddress = CDF_MAC_ADDR_BROADCAST_INITIALIZER
 */
#define CDF_MAC_ADDR_BROADCAST_INITIALIZER { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } }

/* This macro is used to initialize a CDF MacAddress to zero
 * It is used like this...
 * struct cdf_mac_addr macAddress = CDF_MAC_ADDR_ZERO_INITIALIZER
 */
#define CDF_MAC_ADDR_ZERO_INITIALIZER { { 0, 0, 0, 0, 0, 0 } }

#define CDF_IPV4_ADDR_SIZE (4)
#define CDF_IPV6_ADDR_SIZE (16)

/**
 * struct cdf_tso_frag_t - fragments of a single TCP segment
 * @paddr_low_32:	Lower 32 bits of the buffer pointer
 * @paddr_upper_16:	upper 16 bits of the buffer pointer
 * @length:	length of the buffer
 * @vaddr:	virtual address
 *
 * This structure holds the fragments of a single TCP segment of a
 * given jumbo TSO network buffer
 */
struct cdf_tso_frag_t {
	uint32_t paddr_low_32;
	uint32_t paddr_upper_16:16,
		     length:16;
	unsigned char *vaddr;
};

#define FRAG_NUM_MAX 6

/**
 * struct cdf_tso_flags_t - TSO specific flags
 * @tso_enable: Enable transmit segmentation offload
 * @tcp_flags_mask: Tcp_flag is inserted into the header based
 * on the mask
 * @l2_len: L2 length for the msdu
 * @ip_len: IP length for the msdu
 * @tcp_seq_num: TCP sequence number
 * @ip_id: IP identification number
 *
 * This structure holds the TSO specific flags extracted from the TSO network
 * buffer for a given TCP segment
 */
struct cdf_tso_flags_t {
	u_int32_t tso_enable:1,
			reserved_0a:6,
			fin:1,
			syn:1,
			rst:1,
			psh:1,
			ack:1,
			urg:1,
			ece:1,
			cwr:1,
			ns:1,
			tcp_flags_mask:9,
			reserved_0b:7;
/* ------------------------------------------------------------------- */

	u_int32_t l2_len:16,
			ip_len:16;
/* ------------------------------------------------------------------- */

	u_int32_t tcp_seq_num;
/* ------------------------------------------------------------------- */

	u_int32_t ip_id:16,
			ipv4_checksum_en:1,
			udp_ipv4_checksum_en:1,
			udp_ipv6_checksum_en:1,
			tcp_ipv4_checksum_en:1,
			tcp_ipv6_checksum_en:1,
			partial_checksum_en:1,
			reserved_3a:10;
/* ------------------------------------------------------------------- */

	u_int32_t checksum_offset:14,
			reserved_4a:2,
			payload_start_offset:14,
			reserved_4b:2;
/* ------------------------------------------------------------------- */

	u_int32_t payload_end_offset:14,
			reserved_5:18;
};

/**
 * struct cdf_tso_seg_t - single TSO segment
 * @tso_flags:	TSO flags
 * @num_frags:	number of fragments
 * @tso_frags:	array holding the fragments
 *
 * This structure holds the information of a single TSO segment of a jumbo
 * TSO network buffer
 */
struct cdf_tso_seg_t {
	struct cdf_tso_flags_t tso_flags;
/* ------------------------------------------------------------------- */
	uint32_t num_frags;
	struct cdf_tso_frag_t tso_frags[FRAG_NUM_MAX];
};

struct cdf_tso_seg_elem_t {
	struct cdf_tso_seg_t seg;
	struct cdf_tso_seg_elem_t *next;
};

/**
 * struct cdf_tso_info_t - TSO information extracted
 * @is_tso: is this is a TSO frame
 * @num_segs: number of segments
 * @total_len: total length of the packet
 * @tso_seg_list: list of TSO segments for this jumbo packet
 * @curr_seg: segment that is currently being processed
 *
 * This structure holds the TSO information extracted after parsing the TSO
 * jumbo network buffer. It contains a chain of the TSO segments belonging to
 * the jumbo packet
 */
struct cdf_tso_info_t {
	uint8_t is_tso;
	uint32_t num_segs;
	uint32_t total_len;
	struct cdf_tso_seg_elem_t *tso_seg_list;
	struct cdf_tso_seg_elem_t *curr_seg;
};

/**
 * Used to set classify bit in CE desc.
 */
#define CDF_CE_TX_CLASSIFY_BIT_S	5

/**
 * 2 bits starting at bit 6 in CE desc.
 */
#define CDF_CE_TX_PKT_TYPE_BIT_S	6

/**
 * 12 bits --> 16-27, in the CE desciptor, the length of HTT/HTC descriptor
 */
#define CDF_CE_TX_PKT_OFFSET_BIT_S	16

/**
 * Mask for packet offset in the CE descriptor.
 */
#define CDF_CE_TX_PKT_OFFSET_BIT_M	0x0fff0000

#endif /* if !defined __CDF_TYPES_H */
