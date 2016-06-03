/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_types.h
 * QCA driver framework (QDF) basic type definitions
 */

#if !defined(__QDF_TYPES_H)
#define __QDF_TYPES_H

/* Include Files */
#include <i_qdf_types.h>

/* Preprocessor definitions and constants */
#define QDF_MAX_SGLIST 4

/**
 * struct qdf_sglist - scatter-gather list
 * @nsegs: total number of segments
 * struct __sg_segs - scatter-gather segment list
 * @vaddr: Virtual address of the segment
 * @len: Length of the segment
 */
typedef struct qdf_sglist {
	uint32_t nsegs;
	struct __sg_segs {
		uint8_t  *vaddr;
		uint32_t len;
	} sg_segs[QDF_MAX_SGLIST];
} qdf_sglist_t;

#define QDF_MAX_SCATTER __QDF_MAX_SCATTER

/**
 * QDF_SWAP_U16 - swap input u16 value
 * @_x: variable to swap
 */
#define QDF_SWAP_U16(_x) \
	((((_x) << 8) & 0xFF00) | (((_x) >> 8) & 0x00FF))

/**
 * QDF_SWAP_U32 - swap input u32 value
 * @_x: variable to swap
 */
#define QDF_SWAP_U32(_x) \
	(((((_x) << 24) & 0xFF000000) | (((_x) >> 24) & 0x000000FF)) | \
	 ((((_x) << 8) & 0x00FF0000) | (((_x) >> 8) & 0x0000FF00)))

/* ticks per second */
#define QDF_TICKS_PER_SECOND (1000)

/**
 * QDF_ARRAY_SIZE - get array size
 * @_arr: array variable name
 */
#define QDF_ARRAY_SIZE(_arr) (sizeof(_arr) / sizeof((_arr)[0]))

#define QDF_MAX_SCATTER __QDF_MAX_SCATTER

/**
 * qdf_packed - denotes structure is packed.
 */
#define qdf_packed __qdf_packed

typedef void *qdf_net_handle_t;

typedef void *qdf_netlink_handle_t;
typedef void *qdf_drv_handle_t;
typedef void *qdf_os_handle_t;
typedef void *qdf_pm_t;


/**
 * typedef qdf_handle_t - handles opaque to each other
 */
typedef void *qdf_handle_t;

/**
 * typedef qdf_device_t - Platform/bus generic handle.
 * Used for bus specific functions.
 */
typedef __qdf_device_t qdf_device_t;

/**
 * typedef qdf_size_t - size of an object
 */
typedef __qdf_size_t qdf_size_t;

/**
 * typedef __qdf_off_t - offset for API's that need them.
 */
typedef __qdf_off_t      qdf_off_t;

/**
 * typedef qdf_dma_map_t - DMA mapping object.
 */
typedef __qdf_dma_map_t qdf_dma_map_t;

/**
 * tyepdef qdf_dma_addr_t - DMA address.
 */
typedef __qdf_dma_addr_t qdf_dma_addr_t;

/**
 * typedef __qdf_dma_size_t - DMA size.
 */
typedef __qdf_dma_size_t     qdf_dma_size_t;

/**
 * tyepdef qdf_dma_context_t - DMA context.
 */
typedef __qdf_dma_context_t qdf_dma_context_t;

/**
 * struct qdf_dma_map_info - Information inside a DMA map.
 * @nsegs: total number mapped segments
 * struct __dma_segs - Information of physical address.
 * @paddr: physical(dam'able) address of the segment
 * @len: length of the segment
 */
typedef struct qdf_dma_map_info {
	uint32_t nsegs;
	struct __dma_segs {
		qdf_dma_addr_t paddr;
		qdf_dma_size_t len;
	} dma_segs[QDF_MAX_SCATTER];
} qdf_dmamap_info_t;

#define qdf_iomem_t __qdf_iomem_t;

/**
 * typedef enum QDF_TIMER_TYPE - QDF timer type
 * @QDF_TIMER_TYPE_SW: Deferrable SW timer it will not cause CPU to wake up
 * on expiry
 * @QDF_TIMER_TYPE_WAKE_APPS: Non deferrable timer which will cause CPU to
 * wake up on expiry
 */
typedef enum {
	QDF_TIMER_TYPE_SW,
	QDF_TIMER_TYPE_WAKE_APPS
} QDF_TIMER_TYPE;

/**
 * tyepdef qdf_resource_type_t - hw resources
 * @QDF_RESOURCE_TYPE_MEM: memory resource
 * @QDF_RESOURCE_TYPE_IO: io resource
 * Define the hw resources the OS has allocated for the device
 * Note that start defines a mapped area.
 */
typedef enum {
	QDF_RESOURCE_TYPE_MEM,
	QDF_RESOURCE_TYPE_IO,
} qdf_resource_type_t;

/**
 * tyepdef qdf_resource_t - representation of a h/w resource.
 * @start: start
 * @end: end
 * @type: resource type
 */
typedef struct {
	uint64_t start;
	uint64_t end;
	qdf_resource_type_t type;
} qdf_resource_t;

/**
 * typedef qdf_dma_dir_t - DMA directions
 * @QDF_DMA_BIDIRECTIONAL: bidirectional data
 * @QDF_DMA_TO_DEVICE: data going from device to memory
 * @QDF_DMA_FROM_DEVICE: data going from memory to device
 */
typedef enum {
	QDF_DMA_BIDIRECTIONAL = __QDF_DMA_BIDIRECTIONAL,
	QDF_DMA_TO_DEVICE = __QDF_DMA_TO_DEVICE,
	QDF_DMA_FROM_DEVICE = __QDF_DMA_FROM_DEVICE,
} qdf_dma_dir_t;

/* work queue(kernel thread)/DPC function callback */
typedef void (*qdf_defer_fn_t)(void *);

/*
 * Prototype of the critical region function that is to be
 * executed with spinlock held and interrupt disalbed
 */
typedef bool (*qdf_irqlocked_func_t)(void *);

/* Prototype of timer function */
typedef void (*qdf_timer_func_t)(void *);

#define qdf_offsetof(type, field) offsetof(type, field)

/**
 * typedef QDF_MODULE_ID - QDF Module IDs
 * @QDF_MODULE_ID_TLSHIM: TLSHIM module ID
 * @QDF_MODULE_ID_WMI: WMI module ID
 * @QDF_MODULE_ID_HTT: HTT module ID
 * @QDF_MODULE_ID_RSV4: Reserved
 * @QDF_MODULE_ID_HDD: HDD module ID
 * @QDF_MODULE_ID_SME: SME module ID
 * @QDF_MODULE_ID_PE: PE module ID
 * @QDF_MODULE_ID_WMA: WMA module ID
 * @QDF_MODULE_ID_SYS: SYS module ID
 * @QDF_MODULE_ID_QDF: QDF module ID
 * @QDF_MODULE_ID_SAP: SAP module ID
 * @QDF_MODULE_ID_HDD_SOFTAP: HDD SAP module ID
 * @QDF_MODULE_ID_HDD_DATA: HDD DATA module ID
 * @QDF_MODULE_ID_HDD_SAP_DATA: HDD SAP DATA module ID
 * @QDF_MODULE_ID_HIF: HIF module ID
 * @QDF_MODULE_ID_HTC: HTC module ID
 * @QDF_MODULE_ID_TXRX: TXRX module ID
 * @QDF_MODULE_ID_QDF_DEVICE: QDF DEVICE module ID
 * @QDF_MODULE_ID_CFG: CFG module ID
 * @QDF_MODULE_ID_BMI: BMI module ID
 * @QDF_MODULE_ID_EPPING: EPPING module ID
 * @QDF_MODULE_ID_QVIT: QVIT module ID
 * @QDF_MODULE_ID_MAX: Max place holder module ID
 *
 * These are generic IDs that identify the various modules in the software
 * system
 * 0 is unused for historical purposes
 * 3 & 4 are unused for historical purposes
 */
typedef enum {
	QDF_MODULE_ID_TLSHIM = 1,
	QDF_MODULE_ID_WMI = 2,
	QDF_MODULE_ID_HTT = 3,
	QDF_MODULE_ID_RSV4 = 4,
	QDF_MODULE_ID_HDD = 5,
	QDF_MODULE_ID_SME = 6,
	QDF_MODULE_ID_PE = 7,
	QDF_MODULE_ID_WMA = 8,
	QDF_MODULE_ID_SYS = 9,
	QDF_MODULE_ID_QDF = 10,
	QDF_MODULE_ID_SAP = 11,
	QDF_MODULE_ID_HDD_SOFTAP = 12,
	QDF_MODULE_ID_HDD_DATA = 14,
	QDF_MODULE_ID_HDD_SAP_DATA = 15,
	QDF_MODULE_ID_HIF = 16,
	QDF_MODULE_ID_HTC = 17,
	QDF_MODULE_ID_TXRX = 18,
	QDF_MODULE_ID_QDF_DEVICE = 19,
	QDF_MODULE_ID_CFG = 20,
	QDF_MODULE_ID_BMI = 21,
	QDF_MODULE_ID_EPPING = 22,
	QDF_MODULE_ID_QVIT = 23,
	QDF_MODULE_ID_MAX
} QDF_MODULE_ID;

/**
 * enum tQDF_ADAPTER_MODE - Concurrency role.
 * @QDF_STA_MODE: STA mode
 * @QDF_SAP_MODE: SAP mode
 * @QDF_P2P_CLIENT_MODE: P2P client mode
 * @QDF_P2P_GO_MODE: P2P GO mode
 * @QDF_FTM_MODE: FTM mode
 * @QDF_IBSS_MODE: IBSS mode
 * @QDF_MONITOR_MODE: Monitor mode
 * @QDF_P2P_DEVICE_MODE: P2P device mode
 * @QDF_OCB_MODE: OCB device mode
 * @QDF_EPPING_MODE: EPPING device mode
 * @QDF_QVIT_MODE: QVIT device mode
 * @QDF_MAX_NO_OF_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum tQDF_ADAPTER_MODE {
	QDF_STA_MODE,
	QDF_SAP_MODE,
	QDF_P2P_CLIENT_MODE,
	QDF_P2P_GO_MODE,
	QDF_FTM_MODE,
	QDF_IBSS_MODE,
	QDF_MONITOR_MODE,
	QDF_P2P_DEVICE_MODE,
	QDF_OCB_MODE,
	QDF_EPPING_MODE,
	QDF_QVIT_MODE,
	QDF_MAX_NO_OF_MODE
};

/**
 * enum tQDF_GLOBAL_CON_MODE - global config mode when
 * driver is loaded.
 *
 * @QDF_GLOBAL_MISSION_MODE: mission mode (STA, SAP...)
 * @QDF_GLOBAL_MONITOR_MODE: Monitor Mode
 * @QDF_GLOBAL_FTM_MODE: FTM mode
 * @QDF_GLOBAL_EPPING_MODE: EPPING mode
 * @QDF_GLOBAL_QVIT_MODE: QVIT global mode
 * @QDF_GLOBAL_MAX_MODE: Max place holder
 */
enum tQDF_GLOBAL_CON_MODE {
	QDF_GLOBAL_MISSION_MODE,
	QDF_GLOBAL_MONITOR_MODE = 4,
	QDF_GLOBAL_FTM_MODE = 5,
	QDF_GLOBAL_EPPING_MODE = 8,
	QDF_GLOBAL_QVIT_MODE = 9,
	QDF_GLOBAL_MAX_MODE
};

#define  QDF_IS_EPPING_ENABLED(mode) (mode == QDF_GLOBAL_EPPING_MODE)

#ifdef CONFIG_MCL
#define qdf_print(args...) \
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR, ## args)

#else
#define qdf_print printk
#endif /* CONFIG_MCL */

#define qdf_vprint    __qdf_vprint
#define qdf_snprint   __qdf_snprint

#ifdef WLAN_OPEN_P2P_INTERFACE
/* This should match with WLAN_MAX_INTERFACES */
#define QDF_MAX_CONCURRENCY_PERSONA  (4)
#else
#define QDF_MAX_CONCURRENCY_PERSONA  (3)
#endif

#define QDF_STA_MASK (1 << QDF_STA_MODE)
#define QDF_SAP_MASK (1 << QDF_SAP_MODE)
#define QDF_P2P_CLIENT_MASK (1 << QDF_P2P_CLIENT_MODE)
#define QDF_P2P_GO_MASK (1 << QDF_P2P_GO_MODE)

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH

/**
 * typedef tQDF_MCC_TO_SCC_SWITCH_MODE - MCC to SCC switch mode.
 * @QDF_MCC_TO_SCC_SWITCH_DISABLE: Disable switch
 * @QDF_MCC_TO_SCC_SWITCH_ENABLE: Enable switch
 * @QDF_MCC_TO_SCC_SWITCH_FORCE: force switch
 * @QDF_MCC_TO_SCC_SWITCH_MAX: max switch
 */
typedef enum {
	QDF_MCC_TO_SCC_SWITCH_DISABLE = 0,
	QDF_MCC_TO_SCC_SWITCH_ENABLE,
	QDF_MCC_TO_SCC_SWITCH_FORCE,
	QDF_MCC_TO_SCC_SWITCH_FORCE_WITHOUT_DISCONNECTION,
	QDF_MCC_TO_SCC_SWITCH_MAX
} tQDF_MCC_TO_SCC_SWITCH_MODE;
#endif

#if !defined(NULL)
#ifdef __cplusplus
#define NULL   0
#else
#define NULL   ((void *)0)
#endif
#endif

/* typedef for QDF Context... */
typedef void *v_CONTEXT_t;

#define QDF_MAC_ADDR_SIZE (6)

/**
 * struct qdf_mac_addr - mac address array
 * @bytes: MAC address bytes
 */
struct qdf_mac_addr {
	uint8_t bytes[QDF_MAC_ADDR_SIZE];
};

/**
 * This macro is used to initialize a QDF MacAddress to the broadcast
 * MacAddress. It is used like this...
 */
#define QDF_MAC_ADDR_BROADCAST_INITIALIZER \
	{ { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } }

/**
 * This macro is used to initialize a QDF MacAddress to zero
 * It is used like this...
 */
#define QDF_MAC_ADDR_ZERO_INITIALIZER { { 0, 0, 0, 0, 0, 0 } }

#define QDF_IPV4_ADDR_SIZE (4)
#define QDF_IPV6_ADDR_SIZE (16)

/**
 * struct qdf_tso_frag_t - fragments of a single TCP segment
 * @paddr_low_32: Lower 32 bits of the buffer pointer
 * @paddr_upper_16: upper 16 bits of the buffer pointer
 * @length: length of the buffer
 * @vaddr: virtual address
 *
 * This structure holds the fragments of a single TCP segment of a
 * given jumbo TSO network buffer
 */
struct qdf_tso_frag_t {
	uint16_t length;
	unsigned char *vaddr;
	qdf_dma_addr_t paddr;
};

#define FRAG_NUM_MAX 6

/**
 * struct qdf_tso_flags_t - TSO specific flags
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
struct qdf_tso_flags_t {
	uint32_t tso_enable:1,
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

	uint32_t l2_len:16,
			ip_len:16;

	uint32_t tcp_seq_num;

	uint32_t ip_id:16,
			ipv4_checksum_en:1,
			udp_ipv4_checksum_en:1,
			udp_ipv6_checksum_en:1,
			tcp_ipv4_checksum_en:1,
			tcp_ipv6_checksum_en:1,
			partial_checksum_en:1,
			reserved_3a:10;

	uint32_t checksum_offset:14,
			reserved_4a:2,
			payload_start_offset:14,
			reserved_4b:2;

	uint32_t payload_end_offset:14,
			reserved_5:18;
};

/**
 * struct qdf_tso_seg_t - single TSO segment
 * @tso_flags: TSO flags
 * @num_frags: number of fragments
 * @tso_frags: array holding the fragments
 *
 * This structure holds the information of a single TSO segment of a jumbo
 * TSO network buffer
 */
struct qdf_tso_seg_t {
	struct qdf_tso_flags_t tso_flags;
	uint32_t num_frags;
	struct qdf_tso_frag_t tso_frags[FRAG_NUM_MAX];
};

/**
 * qdf_tso_seg_elem_t - tso segment element
 * @seg: instance of segment
 * @next: pointer to the next segment
 */
struct qdf_tso_seg_elem_t {
	struct qdf_tso_seg_t seg;
	struct qdf_tso_seg_elem_t *next;
};

/**
 * struct qdf_tso_info_t - TSO information extracted
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
struct qdf_tso_info_t {
	uint8_t is_tso;
	uint32_t num_segs;
	uint32_t total_len;
	struct qdf_tso_seg_elem_t *tso_seg_list;
	struct qdf_tso_seg_elem_t *curr_seg;
};

/**
 * Used to set classify bit in CE desc.
 */
#define QDF_CE_TX_CLASSIFY_BIT_S   5

/**
 * QDF_CE_TX_PKT_TYPE_BIT_S - 2 bits starting at bit 6 in CE desc.
 */
#define QDF_CE_TX_PKT_TYPE_BIT_S   6

/**
 * QDF_CE_TX_PKT_OFFSET_BIT_S - 12 bits --> 16-27, in the CE desciptor
 *  the length of HTT/HTC descriptor
 */
#define QDF_CE_TX_PKT_OFFSET_BIT_S  16

/**
 * QDF_CE_TX_PKT_OFFSET_BIT_M - Mask for packet offset in the CE descriptor.
 */
#define QDF_CE_TX_PKT_OFFSET_BIT_M   0x0fff0000

/**
 * enum qdf_suspend_type - type of suspend
 * @QDF_SYSTEM_SUSPEND: System suspend triggered wlan suspend
 * @QDF_RUNTIME_SUSPEND: Runtime pm inactivity timer triggered wlan suspend
 */
enum qdf_suspend_type {
	QDF_SYSTEM_SUSPEND,
	QDF_RUNTIME_SUSPEND
};

#endif /* __QDF_TYPES_H */
