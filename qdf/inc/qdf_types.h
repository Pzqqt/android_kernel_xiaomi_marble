/*
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: qdf_types.h
 * QCA driver framework (QDF) basic type definitions
 */

#if !defined(__QDF_TYPES_H)
#define __QDF_TYPES_H

#define qdf_must_check __qdf_must_check

/* Include Files */
#include <i_qdf_types.h>
#ifdef TSOSEG_DEBUG
#include <qdf_atomic.h>
#endif
#include "qdf_status.h"

/* Preprocessor definitions and constants */
#define QDF_MAX_SGLIST 4

/* Define a QDF macro for declaring flexible arrays */
#define QDF_FLEX_ARRAY(type, name) \
	union { \
		type name ## _first_element; \
		struct { \
			struct {} dummy_struct; \
			type name[]; \
		}; \
	}

#define CPU_CLUSTER_TYPE_LITTLE 0
#define CPU_CLUSTER_TYPE_PERF 1

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
#define QDF_NSEC_PER_MSEC __QDF_NSEC_PER_MSEC

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

/**
 * qdf_toupper - char lower to upper.
 */
#define qdf_toupper __qdf_toupper

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
 * typedef qdf_freq_t - define frequency as a 16 bit/32 bit
 * unsigned integer depending on the requirement
 */
#ifdef CONFIG_16_BIT_FREQ_TYPE
typedef uint16_t qdf_freq_t;
#else
typedef uint32_t qdf_freq_t;
#endif
/**
 * typedef qdf_device_t - Platform/bus generic handle.
 * Used for bus specific functions.
 */
typedef __qdf_device_t qdf_device_t;

/* Byte order identifiers */
typedef __qdf_le16_t qdf_le16_t;
typedef __qdf_le32_t qdf_le32_t;
typedef __qdf_le64_t qdf_le64_t;
typedef __qdf_be16_t qdf_be16_t;
typedef __qdf_be32_t qdf_be32_t;
typedef __qdf_be64_t qdf_be64_t;

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

typedef __qdf_mem_info_t qdf_mem_info_t;
typedef __sgtable_t sgtable_t;

/**
 * typepdef qdf_cpu_mask - CPU Mask
 */
typedef __qdf_cpu_mask qdf_cpu_mask;

/**
 * pointer to net device
 */
typedef __qdf_netdev_t qdf_netdev_t;

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

/**
 * struct qdf_shared_mem - Shared memory resource
 * @mem_info: memory info struct
 * @vaddr: virtual address
 * @sgtable: scatter-gather table
 * @memctx: dma address
 */
typedef struct qdf_shared_mem {
	qdf_mem_info_t mem_info;
	void *vaddr;
	sgtable_t sgtable;
	qdf_dma_mem_context(memctx);
} qdf_shared_mem_t;

#define qdf_iomem_t __qdf_iomem_t

/**
 * typedef enum QDF_TIMER_TYPE - QDF timer type
 * @QDF_TIMER_TYPE_SW: Deferrable SW timer it will not cause CPU to wake up
 * on expiry
 * @QDF_TIMER_TYPE_WAKE_APPS: Non deferrable timer which will cause CPU to
 * wake up on expiry
 * @QDF_TIMER_TYPE_SW_SPIN: Deferrable&Pinned SW timer, it will not cause cpu
 * to wake up on expiry and be able to queue on assigned cpu by add_timer_on
 */
typedef enum {
	QDF_TIMER_TYPE_SW,
	QDF_TIMER_TYPE_WAKE_APPS,
	QDF_TIMER_TYPE_SW_SPIN
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

/**
 * enum qdf_driver_type - Indicate the driver type and based on this
 * do appropriate initialization.
 *
 * @QDF_DRIVER_TYPE_PRODUCTION: Driver used in the production
 * @QDF_DRIVER_TYPE_MFG: Driver used in the Factory
 * @QDF_DRIVER_TYPE_INVALID: Invalid and unrecognized type
 *
 */
enum qdf_driver_type {
	QDF_DRIVER_TYPE_PRODUCTION = 0,
	QDF_DRIVER_TYPE_MFG = 1,
	QDF_DRIVER_TYPE_INVALID = 0x7FFFFFFF
};

/* work queue(kernel thread)/DPC function callback */
typedef void (*qdf_defer_fn_t)(void *);

/*
 * Prototype of the critical region function that is to be
 * executed with spinlock held and interrupt disalbed
 */
typedef bool (*qdf_irqlocked_func_t)(void *);

#define qdf_offsetof(type, field) offsetof(type, field)

/**
 * typedef enum QDF_MODULE_ID  - Debug category level
 * @QDF_MODULE_ID_MIN: The smallest/starting module id
 * @QDF_MODULE_ID_TDLS: TDLS
 * @QDF_MODULE_ID_ACS: auto channel selection
 * @QDF_MODULE_ID_SCAN_SM: scan state machine
 * @QDF_MODULE_ID_SCANENTRY: scan entry
 * @QDF_MODULE_ID_WDS: WDS handling
 * @QDF_MODULE_ID_ACTION: action management frames
 * @QDF_MODULE_ID_ROAM: sta mode roaming
 * @QDF_MODULE_ID_INACT: inactivity handling
 * @QDF_MODULE_ID_DOTH: 11.h
 * @QDF_MODULE_ID_IQUE: IQUE features
 * @QDF_MODULE_ID_WME: WME protocol
 * @QDF_MODULE_ID_ACL: ACL handling
 * @QDF_MODULE_ID_WPA: WPA/RSN protocol
 * @QDF_MODULE_ID_RADKEYS: dump 802.1x keys
 * @QDF_MODULE_ID_RADDUMP: dump 802.1x radius packets
 * @QDF_MODULE_ID_RADIUS: 802.1x radius client
 * @QDF_MODULE_ID_DOT1XSM: 802.1x state machine
 * @QDF_MODULE_ID_DOT1X: 802.1x authenticator
 * @QDF_MODULE_ID_POWER: power save handling
 * @QDF_MODULE_ID_STATS: state machine
 * @QDF_MODULE_ID_OUTPUT: output handling
 * @QDF_MODULE_ID_SCAN: scanning
 * @QDF_MODULE_ID_AUTH: authentication handling
 * @QDF_MODULE_ID_ASSOC: association handling
 * @QDF_MODULE_ID_NODE: node handling
 * @QDF_MODULE_ID_ELEMID: element id parsing
 * @QDF_MODULE_ID_XRATE: rate set handling
 * @QDF_MODULE_ID_INPUT: input handling
 * @QDF_MODULE_ID_CRYPTO: crypto work
 * @QDF_MODULE_ID_DUMPPKTS: IFF_LINK2 equivalant
 * @QDF_MODULE_ID_DEBUG: IFF_DEBUG equivalent
 * @QDF_MODULE_ID_MLME: MLME
 * @QDF_MODULE_ID_RRM: Radio resource measurement
 * @QDF_MODULE_ID_WNM: Wireless Network Management
 * @QDF_MODULE_ID_P2P_PROT: P2P Protocol driver
 * @QDF_MODULE_ID_PROXYARP: 11v Proxy ARP
 * @QDF_MODULE_ID_L2TIF: Hotspot 2.0 L2 TIF
 * @QDF_MODULE_ID_WIFIPOS: WifiPositioning Feature
 * @QDF_MODULE_ID_WRAP: WRAP or Wireless ProxySTA
 * @QDF_MODULE_ID_DFS: DFS debug mesg
 * @QDF_MODULE_ID_TLSHIM: TLSHIM module ID
 * @QDF_MODULE_ID_WMI: WMI module ID
 * @QDF_MODULE_ID_HTT: HTT module ID
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
 * @QDF_MODULE_ID_DP: Data-path module ID
 * @QDF_MODULE_ID_HAL: Hal abstraction module ID
 * @QDF_MODULE_ID_SOC: SOC module ID
 * @QDF_MODULE_ID_OS_IF: OS-interface module ID
 * @QDF_MODULE_ID_TARGET_IF: targer interface module ID
 * @QDF_MODULE_ID_SCHEDULER: schduler module ID
 * @QDF_MODULE_ID_MGMT_TXRX: management TX/RX module ID
 * @QDF_MODULE_ID_SERIALIZATION: serialization module ID
 * @QDF_MODULE_ID_PMO: PMO (power manager and offloads) Module ID
 * @QDF_MODULE_ID_P2P: P2P module ID
 * @QDF_MODULE_ID_POLICY_MGR: Policy Manager module ID
 * @QDF_MODULE_ID_CONFIG: CFG (configuration) component ID
 * @QDF_MODULE_ID_REGULATORY: REGULATORY module ID
 * @QDF_MODULE_ID_NAN: NAN module ID
 * @QDF_MODULE_ID_SPECTRAL: Spectral module ID
 * @QDF_MODULE_ID_ROAM_DEBUG: Roam Debug logging
 * @QDF_MODULE_ID_CDP: Converged Data Path module ID
 * @QDF_MODULE_ID_DIRECT_BUF_RX: Direct Buffer Receive module ID
 * @QDF_MODULE_ID_DISA: DISA (encryption test) module ID
 * @QDF_MODULE_ID_GREEN_AP: Green AP related logging
 * @QDF_MODULE_ID_FTM: FTM module ID
 * @QDF_MODULE_ID_FD: FILS discovery logging
 * @QDF_MODULE_ID_OCB: OCB module ID
 * @QDF_MODULE_ID_IPA: IPA module ID
 * @QDF_MODULE_ID_CP_STATS: Control Plane Statistics ID
 * @QDF_MODULE_ID_ACTION_OUI: ACTION OUI module ID
 * @QDF_MODULE_ID_TARGET: Target module ID
 * @QDF_MODULE_ID_MBSSIE: MBSS IE ID
 * @QDF_MODULE_ID_FWOL: FW Offload module ID
 * @QDF_MODULE_ID_SM_ENGINE: SM engine module ID
 * @QDF_MODULE_ID_CMN_MLME: CMN MLME module ID
 * @QDF_MODULE_ID_CFR: CFR module ID
 * @QDF_MODULE_ID_DP_TX_CAPTURE: Tx capture enhancement feature ID
 * @QDF_MODULE_ID_INTEROP_ISSUES_AP: interop issues ap module ID
 * @QDF_MODULE_ID_BLACKLIST_MGR: Blacklist Manager module
 * @QDF_MODULE_ID_QLD: QCA Live Debug module ID
 * @QDF_MODULE_ID_DYNAMIC_MODE_CHG: Dynamic mode change module ID
 * @QDF_MODULE_ID_COEX: Coex related config module ID
 * @QDF_MODULE_ID_FTM_TIME_SYNC: FTM Time sync module ID
 * @QDF_MODULE_ID_PKT_CAPTURE: PACKET CAPTURE module ID
 * @QDF_MODULE_ID_MON_FILTER: Monitor filter related config module ID
 * @QDF_MODULE_ID_DCS: DCS module ID
 * @QDF_MODULE_ID_RPTR: Repeater module ID
 * @QDF_MODULE_ID_6GHZ: 6Ghz specific feature ID
 * @QDF_MODULE_ID_IOT_SIM: IOT Simulation for rogue AP module ID
 * @QDF_MODULE_ID_IFMGR: Interface Manager feature ID
 * @QDF_MODULE_ID_MSCS: MSCS feature ID
 * @QDF_MODULE_ID_GPIO: GPIO configuration module ID
 * @QDF_MODULE_ID_DIAG: Host diag module ID
 * @QDF_MODULE_ID_DP_INIT: INIT/DEINIT path of datapath module ID
 * @QDF_MODULE_ID_DP_TX: TX path of datapath module ID
 * @QDF_MODULE_ID_DP_RX: RX path of datapath module ID
 * @QDF_MODULE_ID_DP_STATS: TX/RX stats, AST stats module ID
 * @QDF_MODULE_ID_DP_HTT: Firmware to host DP event handling module ID
 * @QDF_MODULE_ID_DP_PEER: DP peer module ID
 * @QDF_MODULE_ID_DP_RX_ERROR: Packet handling from WBM release ring module ID
 * @QDF_MODULE_ID_DP_HTT_TX_STATS: FW to host Tx  PPDU stats module ID
 * @QDF_MODULE_ID_DP_RX_MON_STATUS: RX mon status ring module ID
 * @QDF_MODULE_ID_DP_RX_MON_DEST: Monitor ode processing module ID
 * @QDF_MODULE_ID_DP_REO: REO command status module ID
 * @QDF_MODULE_ID_DP_TX_COMP: TX completion module ID
 * @QDF_MODULE_ID_DP_VDEV: DP Vdev module ID
 * @QDF_MODULE_ID_DP_CDP: Configuration module ID
 * @QDF_MODULE_ID_TSO: TSO module ID
 * @QDF_MODULE_ID_ME: Multicast Enhancement module ID
 * @QDF_MODULE_ID_QWRAP: QWRAP module ID
 * @QDF_MODULE_ID_DBDC_REP: DBDC repeater module ID
 * @QDF_MODULE_ID_EXT_AP: Extended AP module ID
 * @QDF_MODULE_ID_MLO: MLO Manager module ID
 * @QDF_MODULE_ID_MLOIE: MLO related IE protocol processing module ID
 * @QDF_MODULE_ID_MBSS: MBSS Framework module ID
 * @QDF_MODULE_ID_MON: Monitor module ID
 * @QDF_MODULE_ID_MGMT_RX_REO: Management rx-reorder module ID
 * @QDF_MODULE_ID_AFC: AFC module ID
 * @QDF_MODULE_ID_WIFI_RADAR: WIFI RADAR module ID
 * @QDF_MODULE_ID_TWT: TWT module ID
 * @QDF_MODULE_ID_COAP: Constrained Application Protocol module ID
 * @QDF_MODULE_ID_ANY: anything
 * @QDF_MODULE_ID_MAX: Max place holder module ID
 *
 * New module ID needs to be added in qdf trace along with this enum.
 */
typedef enum {
	QDF_MODULE_ID_MIN       = 0,
	QDF_MODULE_ID_TDLS      = QDF_MODULE_ID_MIN,
	QDF_MODULE_ID_ACS,
	QDF_MODULE_ID_SCAN_SM,
	QDF_MODULE_ID_SCANENTRY,
	QDF_MODULE_ID_WDS,
	QDF_MODULE_ID_ACTION,
	QDF_MODULE_ID_ROAM,
	QDF_MODULE_ID_INACT,
	QDF_MODULE_ID_DOTH      = 8,
	QDF_MODULE_ID_IQUE,
	QDF_MODULE_ID_WME,
	QDF_MODULE_ID_ACL,
	QDF_MODULE_ID_WPA,
	QDF_MODULE_ID_RADKEYS,
	QDF_MODULE_ID_RADDUMP,
	QDF_MODULE_ID_RADIUS,
	QDF_MODULE_ID_DOT1XSM   = 16,
	QDF_MODULE_ID_DOT1X,
	QDF_MODULE_ID_POWER,
	QDF_MODULE_ID_STATE,
	QDF_MODULE_ID_OUTPUT,
	QDF_MODULE_ID_SCAN,
	QDF_MODULE_ID_AUTH,
	QDF_MODULE_ID_ASSOC,
	QDF_MODULE_ID_NODE      = 24,
	QDF_MODULE_ID_ELEMID,
	QDF_MODULE_ID_XRATE,
	QDF_MODULE_ID_INPUT,
	QDF_MODULE_ID_CRYPTO,
	QDF_MODULE_ID_DUMPPKTS,
	QDF_MODULE_ID_DEBUG,
	QDF_MODULE_ID_MLME,
	QDF_MODULE_ID_RRM       = 32,
	QDF_MODULE_ID_WNM,
	QDF_MODULE_ID_P2P_PROT,
	QDF_MODULE_ID_PROXYARP,
	QDF_MODULE_ID_L2TIF,
	QDF_MODULE_ID_WIFIPOS,
	QDF_MODULE_ID_WRAP,
	QDF_MODULE_ID_DFS,
	QDF_MODULE_ID_ATF       = 40,
	QDF_MODULE_ID_SPLITMAC,
	QDF_MODULE_ID_IOCTL,
	QDF_MODULE_ID_NAC,
	QDF_MODULE_ID_MESH,
	QDF_MODULE_ID_MBO,
	QDF_MODULE_ID_EXTIOCTL_CHANSWITCH,
	QDF_MODULE_ID_EXTIOCTL_CHANSSCAN,
	QDF_MODULE_ID_TLSHIM    = 48,
	QDF_MODULE_ID_WMI,
	QDF_MODULE_ID_HTT,
	QDF_MODULE_ID_HDD,
	QDF_MODULE_ID_SME,
	QDF_MODULE_ID_PE,
	QDF_MODULE_ID_WMA,
	QDF_MODULE_ID_SYS,
	QDF_MODULE_ID_QDF       = 56,
	QDF_MODULE_ID_SAP,
	QDF_MODULE_ID_HDD_SOFTAP,
	QDF_MODULE_ID_HDD_DATA,
	QDF_MODULE_ID_HDD_SAP_DATA,
	QDF_MODULE_ID_HIF,
	QDF_MODULE_ID_HTC,
	QDF_MODULE_ID_TXRX,
	QDF_MODULE_ID_QDF_DEVICE = 64,
	QDF_MODULE_ID_CFG,
	QDF_MODULE_ID_BMI,
	QDF_MODULE_ID_EPPING,
	QDF_MODULE_ID_QVIT,
	QDF_MODULE_ID_DP,
	QDF_MODULE_ID_HAL,
	QDF_MODULE_ID_SOC,
	QDF_MODULE_ID_OS_IF,
	QDF_MODULE_ID_TARGET_IF,
	QDF_MODULE_ID_SCHEDULER,
	QDF_MODULE_ID_MGMT_TXRX,
	QDF_MODULE_ID_SERIALIZATION,
	QDF_MODULE_ID_PMO,
	QDF_MODULE_ID_P2P,
	QDF_MODULE_ID_POLICY_MGR,
	QDF_MODULE_ID_CONFIG,
	QDF_MODULE_ID_REGULATORY,
	QDF_MODULE_ID_SA_API,
	QDF_MODULE_ID_NAN,
	QDF_MODULE_ID_OFFCHAN_TXRX,
	QDF_MODULE_ID_SON,
	QDF_MODULE_ID_SPECTRAL,
	QDF_MODULE_ID_OBJ_MGR,
	QDF_MODULE_ID_NSS,
	QDF_MODULE_ID_ROAM_DEBUG,
	QDF_MODULE_ID_CDP,
	QDF_MODULE_ID_DIRECT_BUF_RX,
	QDF_MODULE_ID_DISA,
	QDF_MODULE_ID_GREEN_AP,
	QDF_MODULE_ID_FTM,
	QDF_MODULE_ID_FD,
	QDF_MODULE_ID_OCB,
	QDF_MODULE_ID_IPA,
	QDF_MODULE_ID_CP_STATS,
	QDF_MODULE_ID_ACTION_OUI,
	QDF_MODULE_ID_TARGET,
	QDF_MODULE_ID_MBSSIE,
	QDF_MODULE_ID_FWOL,
	QDF_MODULE_ID_SM_ENGINE,
	QDF_MODULE_ID_CMN_MLME,
	QDF_MODULE_ID_BSSCOLOR,
	QDF_MODULE_ID_CFR,
	QDF_MODULE_ID_DP_TX_CAPTURE,
	QDF_MODULE_ID_INTEROP_ISSUES_AP,
	QDF_MODULE_ID_BLACKLIST_MGR,
	QDF_MODULE_ID_QLD,
	QDF_MODULE_ID_DYNAMIC_MODE_CHG,
	QDF_MODULE_ID_COEX,
	QDF_MODULE_ID_FTM_TIME_SYNC,
	QDF_MODULE_ID_PKT_CAPTURE,
	QDF_MODULE_ID_MON_FILTER,
	QDF_MODULE_ID_DCS,
	QDF_MODULE_ID_RPTR,
	QDF_MODULE_ID_6GHZ,
	QDF_MODULE_ID_IOT_SIM,
	QDF_MODULE_ID_IFMGR,
	QDF_MODULE_ID_MSCS,
	QDF_MODULE_ID_GPIO,
	QDF_MODULE_ID_DIAG,
	QDF_MODULE_ID_DP_INIT,
	QDF_MODULE_ID_DP_TX,
	QDF_MODULE_ID_DP_RX,
	QDF_MODULE_ID_DP_STATS,
	QDF_MODULE_ID_DP_HTT,
	QDF_MODULE_ID_DP_PEER,
	QDF_MODULE_ID_DP_RX_ERROR,
	QDF_MODULE_ID_DP_HTT_TX_STATS,
	QDF_MODULE_ID_DP_RX_MON_STATUS,
	QDF_MODULE_ID_DP_RX_MON_DEST,
	QDF_MODULE_ID_DP_REO,
	QDF_MODULE_ID_DP_TX_COMP,
	QDF_MODULE_ID_DP_VDEV,
	QDF_MODULE_ID_DP_CDP,
	QDF_MODULE_ID_TSO,
	QDF_MODULE_ID_ME,
	QDF_MODULE_ID_QWRAP,
	QDF_MODULE_ID_DBDC_REP,
	QDF_MODULE_ID_EXT_AP,
	QDF_MODULE_ID_MLO,
	QDF_MODULE_ID_MLOIE,
	QDF_MODULE_ID_MBSS,
	QDF_MODULE_ID_MON,
	QDF_MODULE_ID_MGMT_RX_REO,
	QDF_MODULE_ID_AFC,
	QDF_MODULE_ID_WIFI_RADAR,
	QDF_MODULE_ID_TWT,
	QDF_MODULE_ID_COAP,
	QDF_MODULE_ID_ANY,
	QDF_MODULE_ID_MAX,
} QDF_MODULE_ID;

/**
 * typedef enum QDF_TRACE_LEVEL - Debug verbose level
 * @QDF_TRACE_LEVEL_NONE: no trace will be logged. This value is in place
 *			  for the qdf_trace_setlevel() to allow the user
 *			  to turn off all traces
 * @QDF_TRACE_LEVEL_FATAL: Indicates fatal error conditions
 * @QDF_TRACE_LEVEL_ERROR: Indicates error conditions
 * @QDF_TRACE_LEVEL_WARN: May indicate that an error will occur if action
 *			  is not taken
 * @QDF_TRACE_LEVEL_INFO: Normal operational messages that require no action
 * @QDF_TRACE_LEVEL_INFO_HIGH: High level operational messages that require
 *			       no action
 * @QDF_TRACE_LEVEL_INFO_MED: Middle level operational messages that require
 *			      no action
 * @QDF_TRACE_LEVEL_INFO_LOW: Low level operational messages that require
 *			      no action
 * @QDF_TRACE_LEVEL_DEBUG: Information useful to developers for debugging
 * @QDF_TRACE_LEVEL_TRACE: Indicates trace level for automation scripts,
 *			whenever there is a context switch in driver, one
 *			print using this trace level will be added with
 *			the help of qdf_trace api.
 * @QDF_TRACE_LEVEL_ALL: All trace levels
 * @QDF_TRACE_LEVEL_MAX: Max trace level
 */
typedef enum {
	QDF_TRACE_LEVEL_NONE,
	QDF_TRACE_LEVEL_FATAL,
	QDF_TRACE_LEVEL_ERROR,
	QDF_TRACE_LEVEL_WARN,
	QDF_TRACE_LEVEL_INFO,
	QDF_TRACE_LEVEL_INFO_HIGH,
	QDF_TRACE_LEVEL_INFO_MED,
	QDF_TRACE_LEVEL_INFO_LOW,
	QDF_TRACE_LEVEL_DEBUG,
	QDF_TRACE_LEVEL_TRACE,
	QDF_TRACE_LEVEL_ALL,
	QDF_TRACE_LEVEL_MAX
} QDF_TRACE_LEVEL;

/**
 * enum QDF_OPMODE - vdev operating mode
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
 * @QDF_NDI_MODE: NAN datapath mode
 * @QDF_WDS_MODE: WDS mode
 * @QDF_BTAMP_MODE: BTAMP mode
 * @QDF_AHDEMO_MODE: AHDEMO mode
 * @QDF_TDLS_MODE: TDLS device mode
 * @QDF_NAN_DISC_MODE: NAN Discovery device mode
 * @QDF_MAX_NO_OF_MODE: Max place holder
 *
 * These are generic IDs that identify the various roles
 * in the software system
 */
enum QDF_OPMODE {
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
	QDF_NDI_MODE,
	QDF_WDS_MODE,
	QDF_BTAMP_MODE,
	QDF_AHDEMO_MODE,
	QDF_TDLS_MODE,
	QDF_NAN_DISC_MODE,

	/* Add new OP Modes to qdf_opmode_str as well */

	QDF_MAX_NO_OF_MODE
};

/**
 * qdf_opmode_str() - Return a human readable string representation of @opmode
 * @opmode: The opmode to convert
 *
 * Return: string representation of @opmode
 */
const char *qdf_opmode_str(const enum QDF_OPMODE opmode);

/**
 * enum QDF_GLOBAL_MODE - global mode when driver is loaded.
 *
 * @QDF_GLOBAL_MISSION_MODE: mission mode (STA, SAP...)
 * @QDF_GLOBAL_WALTEST_MODE: WAL Test Mode
 * @QDF_GLOBAL_MONITOR_MODE: Monitor Mode
 * @QDF_GLOBAL_FTM_MODE: FTM mode
 * @QDF_GLOBAL_IBSS_MODE: IBSS mode
 * @QDF_GLOBAL_COLDBOOT_CALIB_MODEL: Cold Boot Calibration Mode
 * @QDF_GLOBAL_EPPING_MODE: EPPING mode
 * @QDF_GLOBAL_QVIT_MODE: QVIT global mode
 * @QDF_GLOBAL_FTM_COLDBOOT_CALIB_MODE: Cold Boot Calibration in FTM Mode
 * @QDF_GLOBAL_MAX_MODE: Max place holder
 */
enum QDF_GLOBAL_MODE {
	QDF_GLOBAL_MISSION_MODE,
	QDF_GLOBAL_WALTEST_MODE = 3,
	QDF_GLOBAL_MONITOR_MODE = 4,
	QDF_GLOBAL_FTM_MODE = 5,
	QDF_GLOBAL_IBSS_MODE = 6,
	QDF_GLOBAL_COLDBOOT_CALIB_MODE = 7,
	QDF_GLOBAL_EPPING_MODE = 8,
	QDF_GLOBAL_QVIT_MODE = 9,
	QDF_GLOBAL_FTM_COLDBOOT_CALIB_MODE = 10,
	QDF_GLOBAL_MAX_MODE
};

#define  QDF_IS_EPPING_ENABLED(mode) (mode == QDF_GLOBAL_EPPING_MODE)

#ifdef QDF_TRACE_PRINT_ENABLE
#define qdf_print(args...) QDF_TRACE_INFO(QDF_MODULE_ID_ANY, ## args)
#define qdf_alert(args...) QDF_TRACE_FATAL(QDF_MODULE_ID_ANY, ## args)
#define qdf_err(args...)   QDF_TRACE_ERROR(QDF_MODULE_ID_ANY, ## args)
#define qdf_warn(args...)  QDF_TRACE_WARN(QDF_MODULE_ID_ANY, ## args)
#define qdf_info(args...)  QDF_TRACE_INFO(QDF_MODULE_ID_ANY, ## args)
#define qdf_debug(args...) QDF_TRACE_DEBUG(QDF_MODULE_ID_ANY, ## args)

#define qdf_nofl_print(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_ANY, ## params)
#define qdf_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_ANY, ## params)
#define qdf_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_ANY, ## params)
#define qdf_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_ANY, ## params)
#define qdf_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_ANY, ## params)
#define qdf_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_ANY, ## params)

#else /* QDF_TRACE_PRINT_ENABLE */
#define qdf_print(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_QDF, ## params)
#define qdf_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_QDF, ## params)
#define qdf_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_QDF, ## params)
#define qdf_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_QDF, ## params)
#define qdf_info(params...) QDF_TRACE_INFO(QDF_MODULE_ID_QDF, ## params)
#define qdf_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_QDF, ## params)

#define qdf_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_QDF, ## params)
#define qdf_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_QDF, ## params)
#define qdf_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_QDF, ## params)
#define qdf_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_QDF, ## params)
#define qdf_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_QDF, ## params)

#endif /* QDF_TRACE_PRINT_ENABLE */

#define qdf_rl_alert(params...) QDF_TRACE_FATAL_RL(QDF_MODULE_ID_QDF, ## params)
#define qdf_rl_err(params...) QDF_TRACE_ERROR_RL(QDF_MODULE_ID_QDF, ## params)
#define qdf_rl_warn(params...) QDF_TRACE_WARN_RL(QDF_MODULE_ID_QDF, ## params)
#define qdf_rl_info(params...) QDF_TRACE_INFO_RL(QDF_MODULE_ID_QDF, ## params)
#define qdf_rl_debug(params...) QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_QDF, ## params)

#define qdf_rl_nofl_alert(params...) \
	QDF_TRACE_FATAL_RL_NO_FL(QDF_MODULE_ID_QDF, ## params)
#define qdf_rl_nofl_err(params...) \
	QDF_TRACE_ERROR_RL_NO_FL(QDF_MODULE_ID_QDF, ## params)
#define qdf_rl_nofl_warn(params...) \
	QDF_TRACE_WARN_RL_NO_FL(QDF_MODULE_ID_QDF, ## params)
#define qdf_rl_nofl_info(params...) \
	QDF_TRACE_INFO_RL_NO_FL(QDF_MODULE_ID_QDF, ## params)
#define qdf_rl_nofl_debug(params...) \
	QDF_TRACE_DEBUG_RL_NO_FL(QDF_MODULE_ID_QDF, ## params)

#define qdf_snprint   __qdf_snprint

#define qdf_kstrtoint __qdf_kstrtoint
#define qdf_kstrtouint __qdf_kstrtouint

#ifdef WLAN_OPEN_P2P_INTERFACE
/* This should match with WLAN_MAX_INTERFACES */
#define QDF_MAX_CONCURRENCY_PERSONA  (WLAN_MAX_VDEVS)
#else
#define QDF_MAX_CONCURRENCY_PERSONA  (WLAN_MAX_VDEVS - 1)
#endif

#define QDF_STA_MASK (1 << QDF_STA_MODE)
#define QDF_SAP_MASK (1 << QDF_SAP_MODE)
#define QDF_P2P_CLIENT_MASK (1 << QDF_P2P_CLIENT_MODE)
#define QDF_P2P_GO_MASK (1 << QDF_P2P_GO_MODE)
#define QDF_MONITOR_MASK (1 << QDF_MONITOR_MODE)

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH

/**
 * typedef tQDF_MCC_TO_SCC_SWITCH_MODE - MCC to SCC switch mode.
 * @QDF_MCC_TO_SCC_SWITCH_DISABLE: Disable switch
 * @QDF_MCC_TO_SCC_SWITCH_FORCE_WITHOUT_DISCONNECTION: Force switch without
 * restart of SAP
 * @QDF_MCC_TO_SCC_SWITCH_WITH_FAVORITE_CHANNEL: Switch using fav channel(s)
 * without SAP restart
 * @QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION: Force switch
 * without SAP restart. MCC is allowed only in below exception cases:
 *	Exception Case-1: When STA is operating on DFS channel.
 *	Exception Case-2: When STA is operating on LTE-CoEx channel.
 *	Exception Case-3: When STA is operating on AP disabled channel.
 * @QDF_MCC_TO_SCC_WITH_PREFERRED_BAND: Force SCC only in user preferred band.
 * Allow MCC if STA is operating or comes up on other than user preferred band.
 *
 * @QDF_MCC_TO_SCC_SWITCH_MAX: max switch
 */
typedef enum {
	QDF_MCC_TO_SCC_SWITCH_DISABLE = 0,
	QDF_MCC_TO_SCC_SWITCH_FORCE_WITHOUT_DISCONNECTION = 3,
	QDF_MCC_TO_SCC_SWITCH_WITH_FAVORITE_CHANNEL,
	QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION,
	QDF_MCC_TO_SCC_WITH_PREFERRED_BAND,
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

/**
 * qdf_bool_parse() - parse the given string as a boolean value
 * @bool_str: the input boolean string to parse
 * @out_bool: the output boolean value, populated on success
 *
 * 1, y, Y are mapped to true, 0, n, N are mapped to false.
 * Leading/trailing whitespace is ignored.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_bool_parse(const char *bool_str, bool *out_bool);

/**
 * qdf_int32_parse() - parse the given string as a 32-bit signed integer
 * @int_str: the input integer string to parse
 * @out_int: the output integer value, populated on success
 *
 * Supports binary (0b), octal (0o), decimal (no prefix), and hexadecimal (0x)
 * encodings via typical prefix notation. Leading/trailing whitespace is
 * ignored.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_int32_parse(const char *int_str, int32_t *out_int);

/**
 * qdf_uint32_parse() - parse the given string as a 32-bit unsigned integer
 * @int_str: the input integer string to parse
 * @out_int: the output integer value, populated on success
 *
 * Supports binary (0b), octal (0o), decimal (no prefix), and hexadecimal (0x)
 * encodings via typical prefix notation. Leading/trailing whitespace is
 * ignored.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_uint32_parse(const char *int_str, uint32_t *out_int);

/**
 * qdf_int64_parse() - parse the given string as a 64-bit signed integer
 * @int_str: the input integer string to parse
 * @out_int: the output integer value, populated on success
 *
 * Supports binary (0b), octal (0o), decimal (no prefix), and hexadecimal (0x)
 * encodings via typical prefix notation. Leading/trailing whitespace is
 * ignored.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_int64_parse(const char *int_str, int64_t *out_int);

/**
 * qdf_uint64_parse() - parse the given string as a 64-bit unsigned integer
 * @int_str: the input integer string to parse
 * @out_int: the output integer value, populated on success
 *
 * Supports binary (0b), octal (0o), decimal (no prefix), and hexadecimal (0x)
 * encodings via typical prefix notation. Leading/trailing whitespace is
 * ignored.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_uint64_parse(const char *int_str, uint64_t *out_int);

#define QDF_MAC_ADDR_SIZE 6

/**
 * If the feature CONFIG_WLAN_TRACE_HIDE_MAC_ADDRESS is enabled,
 * then the requirement is to hide 2nd, 3rd and 4th octet of the
 * MAC address in the kernel logs and driver logs.
 * But other management interfaces like ioctl, debugfs, sysfs,
 * wext, unit test code or non-production simulator sw (iot_sim)
 * should continue to log the full mac address.
 *
 * Developers must use QDF_FULL_MAC_FMT instead of "%pM",
 * as this macro helps avoid accidentally breaking the feature
 * CONFIG_WLAN_TRACE_HIDE_MAC_ADDRESS if enabled and code auditing
 * becomes easy.
 */
#define QDF_FULL_MAC_FMT "%pM"
#define QDF_FULL_MAC_REF(a) (a)

#if defined(WLAN_TRACE_HIDE_MAC_ADDRESS)
#define QDF_MAC_ADDR_FMT "%02x:**:**:**:%02x:%02x"

/*
 * The input data type for QDF_MAC_ADDR_REF can be pointer or an array.
 * In case of array, compiler was throwing following warning
 * 'address of array will always evaluate as ‘true’
 * and if the pointer is NULL, zero is passed to the format specifier
 * which results in zero mac address (00:**:**:**:00:00)
 * For this reason, input data type is typecasted to (uintptr_t).
 */
#define QDF_MAC_ADDR_REF(a) \
	(((uintptr_t)NULL != (uintptr_t)(a)) ? (a)[0] : 0), \
	(((uintptr_t)NULL != (uintptr_t)(a)) ? (a)[4] : 0), \
	(((uintptr_t)NULL != (uintptr_t)(a)) ? (a)[5] : 0)
#else
#define QDF_MAC_ADDR_FMT "%pM"
#define QDF_MAC_ADDR_REF(a) (a)
#endif /* WLAN_TRACE_HIDE_MAC_ADDRESS */

#define QDF_MAC_ADDR_BCAST_INIT { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } }
#define QDF_MAC_ADDR_ZERO_INIT { { 0, 0, 0, 0, 0, 0 } }

/**
 * struct qdf_mac_addr - A MAC address
 * @bytes: the raw address bytes array
 */
struct qdf_mac_addr {
	uint8_t bytes[QDF_MAC_ADDR_SIZE];
};

/**
 * enum qdf_proto_subtype - subtype of packet
 * @QDF_PROTO_EAPOL_M1 - EAPOL 1/4
 * @QDF_PROTO_EAPOL_M2 - EAPOL 2/4
 * @QDF_PROTO_EAPOL_M3 - EAPOL 3/4
 * @QDF_PROTO_EAPOL_M4 - EAPOL 4/4
 * @QDF_PROTO_DHCP_DISCOVER - discover
 * @QDF_PROTO_DHCP_REQUEST - request
 * @QDF_PROTO_DHCP_OFFER - offer
 * @QDF_PROTO_DHCP_ACK - ACK
 * @QDF_PROTO_DHCP_NACK - NACK
 * @QDF_PROTO_DHCP_RELEASE - release
 * @QDF_PROTO_DHCP_INFORM - inform
 * @QDF_PROTO_DHCP_DECLINE - decline
 * @QDF_PROTO_ARP_REQ - arp request
 * @QDF_PROTO_ARP_RES - arp response
 * @QDF_PROTO_ICMP_REQ - icmp request
 * @QDF_PROTO_ICMP_RES - icmp response
 * @QDF_PROTO_ICMPV6_REQ - icmpv6 request
 * @QDF_PROTO_ICMPV6_RES - icmpv6 response
 * @QDF_PROTO_ICMPV6_RS - icmpv6 rs packet
 * @QDF_PROTO_ICMPV6_RA - icmpv6 ra packet
 * @QDF_PROTO_ICMPV6_NS - icmpv6 ns packet
 * @QDF_PROTO_ICMPV6_NA - icmpv6 na packet
 * @QDF_PROTO_IPV4_UDP - ipv4 udp
 * @QDF_PROTO_IPV4_TCP - ipv4 tcp
 * @QDF_PROTO_IPV6_UDP - ipv6 udp
 * @QDF_PROTO_IPV6_TCP - ipv6 tcp
 * @QDF_PROTO_MGMT_ASSOC -assoc
 * @QDF_PROTO_MGMT_DISASSOC - disassoc
 * @QDF_PROTO_MGMT_AUTH - auth
 * @QDF_PROTO_MGMT_DEAUTH - deauth
 * @QDF_ROAM_SYNCH - roam synch indication from fw
 * @QDF_ROAM_COMPLETE - roam complete cmd to fw
 * @QDF_ROAM_EVENTID - roam eventid from fw
 * @QDF_PROTO_DNS_QUERY - dns query
 * @QDF_PROTO_DNS_RES -dns response
 */
enum qdf_proto_subtype {
	QDF_PROTO_INVALID,
	QDF_PROTO_EAPOL_M1,
	QDF_PROTO_EAPOL_M2,
	QDF_PROTO_EAPOL_M3,
	QDF_PROTO_EAPOL_M4,
	QDF_PROTO_DHCP_DISCOVER,
	QDF_PROTO_DHCP_REQUEST,
	QDF_PROTO_DHCP_OFFER,
	QDF_PROTO_DHCP_ACK,
	QDF_PROTO_DHCP_NACK,
	QDF_PROTO_DHCP_RELEASE,
	QDF_PROTO_DHCP_INFORM,
	QDF_PROTO_DHCP_DECLINE,
	QDF_PROTO_ARP_REQ,
	QDF_PROTO_ARP_RES,
	QDF_PROTO_ICMP_REQ,
	QDF_PROTO_ICMP_RES,
	QDF_PROTO_ICMPV6_REQ,
	QDF_PROTO_ICMPV6_RES,
	QDF_PROTO_ICMPV6_RS,
	QDF_PROTO_ICMPV6_RA,
	QDF_PROTO_ICMPV6_NS,
	QDF_PROTO_ICMPV6_NA,
	QDF_PROTO_IPV4_UDP,
	QDF_PROTO_IPV4_TCP,
	QDF_PROTO_IPV6_UDP,
	QDF_PROTO_IPV6_TCP,
	QDF_PROTO_MGMT_ASSOC,
	QDF_PROTO_MGMT_DISASSOC,
	QDF_PROTO_MGMT_AUTH,
	QDF_PROTO_MGMT_DEAUTH,
	QDF_ROAM_SYNCH,
	QDF_ROAM_COMPLETE,
	QDF_ROAM_EVENTID,
	QDF_PROTO_DNS_QUERY,
	QDF_PROTO_DNS_RES,
	QDF_PROTO_SUBTYPE_MAX
};

/**
 * qdf_mac_parse() - parse the given string as a MAC address
 * @mac_str: the input MAC address string to parse
 * @out_addr: the output MAC address value, populated on success
 *
 * A MAC address is a set of 6, colon-delimited, hexadecimal encoded octets.
 *
 * E.g.
 *	00:00:00:00:00:00 (zero address)
 *	ff:ff:ff:ff:ff:ff (broadcast address)
 *	12:34:56:78:90:ab (an arbitrary address)
 *
 * This implementation also accepts MAC addresses without colons. Historically,
 * other delimiters and groupings have been used to represent MAC addresses, but
 * these are not supported here. Hexadecimal digits may be in either upper or
 * lower case.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_mac_parse(const char *mac_str, struct qdf_mac_addr *out_addr);

#define QDF_IPV4_ADDR_SIZE 4
#define QDF_IPV4_ADDR_STR "%d.%d.%d.%d"
#define QDF_IPV4_ADDR_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3]
#define QDF_IPV4_ADDR_ZERO_INIT { { 0, 0, 0, 0 } }

/**
 * struct qdf_ipv4_addr - An IPV4 address
 * @bytes: the raw address bytes array
 */
struct qdf_ipv4_addr {
	uint8_t bytes[QDF_IPV4_ADDR_SIZE];
};

/**
 * qdf_ipv4_parse() - parse the given string as an IPV4 address
 * @ipv4_str: the input IPV4 address string to parse
 * @out_addr: the output IPV4 address value, populated on success
 *
 * An IPV4 address is a set of 4, dot-delimited, decimal encoded octets.
 *
 * E.g.
 *	0.0.0.0 (wildcard address)
 *	127.0.0.1 (loopback address)
 *	255.255.255.255 (broadcast address)
 *	192.168.0.1 (an arbitrary address)
 *
 * Historically, non-decimal encodings have also been used to represent IPV4
 * addresses, but these are not supported here.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_ipv4_parse(const char *ipv4_str, struct qdf_ipv4_addr *out_addr);

#define QDF_IPV6_ADDR_SIZE 16
#define QDF_IPV6_ADDR_HEXTET_COUNT 8
#define QDF_IPV6_ADDR_STR "%x:%x:%x:%x:%x:%x:%x:%x"
#define QDF_IPV6_ADDR_ARRAY(a) \
	((a)[0] << 8) + (a)[1], ((a)[2] << 8) + (a)[3], \
	((a)[4] << 8) + (a)[5], ((a)[6] << 8) + (a)[7], \
	((a)[8] << 8) + (a)[9], ((a)[10] << 8) + (a)[11], \
	((a)[12] << 8) + (a)[13], ((a)[14] << 8) + (a)[15]
#define QDF_IPV6_ADDR_ZERO_INIT \
	{ { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

/**
 * struct qdf_ipv6_addr - An IPV6 address
 * @bytes: the raw address bytes array
 */
struct qdf_ipv6_addr {
	uint8_t bytes[QDF_IPV6_ADDR_SIZE];
};

/**
 * qdf_ipv6_parse() - parse the given string as an IPV6 address
 * @ipv6_str: the input IPV6 address string to parse
 * @out_addr: the output IPV6 address value, populated on success
 *
 * A hextet is a pair of octets. An IPV6 address is a set of 8, colon-delimited,
 * hexadecimal encoded hextets. Each hextet may omit leading zeros. One or more
 * zero-hextets may be "compressed" using a pair of colons ("::"). Up to one
 * such zero-compression is allowed per address.
 *
 * E.g.
 *	0:0:0:0:0:0:0:0 (unspecified address)
 *	:: (also the unspecified address)
 *	0:0:0:0:0:0:0:1 (loopback address)
 *	::1 (also the loopback address)
 *	900a:ae7::6 (an arbitrary address)
 *	900a:ae7:0:0:0:0:0:6 (the same arbitrary address)
 *
 * Hexadecimal digits may be in either upper or lower case.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_ipv6_parse(const char *ipv6_str, struct qdf_ipv6_addr *out_addr);

/**
 * qdf_uint32_array_parse() - parse the given string as uint32 array
 * @in_str: the input string to parse
 * @out_array: the output uint32 array, populated on success
 * @array_size: size of the array
 * @out_size: size of the populated array
 *
 * This API is called to convert string (each value separated by
 * a comma) into an uint32 array
 *
 * Return: QDF_STATUS
 */

QDF_STATUS qdf_uint32_array_parse(const char *in_str, uint32_t *out_array,
				  qdf_size_t array_size, qdf_size_t *out_size);

/**
 * qdf_uint16_array_parse() - parse the given string as uint16 array
 * @in_str: the input string to parse
 * @out_array: the output uint16 array, populated on success
 * @array_size: size of the array
 * @out_size: size of the populated array
 *
 * This API is called to convert string (each value separated by
 * a comma) into an uint16 array
 *
 * Return: QDF_STATUS
 */

QDF_STATUS qdf_uint16_array_parse(const char *in_str, uint16_t *out_array,
				  qdf_size_t array_size, qdf_size_t *out_size);

/**
 * qdf_uint8_array_parse() - parse the given string as uint8 array
 * @in_str: the input string to parse
 * @out_array: the output uint8 array, populated on success
 * @array_size: size of the array
 * @out_size: size of the populated array
 *
 * This API is called to convert string (each byte separated by
 * a comma) into an u8 array
 *
 * Return: QDF_STATUS
 */
QDF_STATUS qdf_uint8_array_parse(const char *in_str, uint8_t *out_array,
				 qdf_size_t array_size, qdf_size_t *out_size);

#define QDF_BCAST_MAC_ADDR (0xFF)
#define QDF_MCAST_IPV4_MAC_ADDR (0x01)
#define QDF_MCAST_IPV6_MAC_ADDR (0x33)

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
#define TSO_SEG_MAGIC_COOKIE 0x1EED

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
 * @total_len: total length of the packet
 * @tso_frags: array holding the fragments
 *
 * This structure holds the information of a single TSO segment of a jumbo
 * TSO network buffer
 */
struct qdf_tso_seg_t {
	struct qdf_tso_flags_t tso_flags;
	uint32_t num_frags;
	uint32_t total_len;
	struct qdf_tso_frag_t tso_frags[FRAG_NUM_MAX];
};

/**
 * TSO seg elem action caller locations: goes into dbg.history below.
 * Needed to be defined outside of the feature so that
 * callers can be coded without ifdefs (even if they get
 * resolved to nothing)
 */
enum tsoseg_dbg_caller_e {
	TSOSEG_LOC_UNDEFINED,
	TSOSEG_LOC_INIT1,
	TSOSEG_LOC_INIT2,
	TSOSEG_LOC_FREE,
	TSOSEG_LOC_ALLOC,
	TSOSEG_LOC_DEINIT,
	TSOSEG_LOC_GETINFO,
	TSOSEG_LOC_FILLHTTSEG,
	TSOSEG_LOC_FILLCMNSEG,
	TSOSEG_LOC_PREPARETSO,
	TSOSEG_LOC_TXPREPLLFAST,
	TSOSEG_LOC_UNMAPTSO,
	TSOSEG_LOC_UNMAPLAST,
	TSOSEG_LOC_FORCE_FREE,
};
#ifdef TSOSEG_DEBUG

/**
 * WARNING: Don't change the history size without changing the wrap
 *  code in qdf_tso_seg_dbg_record function
 */
#define MAX_TSO_SEG_ACT_HISTORY 16
struct qdf_tso_seg_dbg_history_t {
	uint64_t ts;
	short    id;
};
struct qdf_tso_seg_dbg_t {
	void    *txdesc;  /* owner - (ol_txrx_tx_desc_t *) */
	qdf_atomic_t cur; /* index of last valid entry */
	struct qdf_tso_seg_dbg_history_t h[MAX_TSO_SEG_ACT_HISTORY];
};
#endif /* TSOSEG_DEBUG */

/**
 * qdf_tso_seg_elem_t - tso segment element
 * @next: pointer to the next segment
 * @seg: instance of segment
 */
struct qdf_tso_seg_elem_t {
	struct qdf_tso_seg_elem_t *next;
	struct qdf_tso_seg_t seg;
	uint32_t cookie:13,
		on_freelist:1,
		sent_to_target:1,
		force_free:1;
#ifdef TSOSEG_DEBUG
	struct qdf_tso_seg_dbg_t dbg;
#endif /* TSOSEG_DEBUG */
};

/**
 * struct qdf_tso_num_seg_t - single element to count for num of seg
 * @tso_cmn_num_seg: num of seg in a jumbo skb
 *
 * This structure holds the information of num of segments of a jumbo
 * TSO network buffer.
 */
struct qdf_tso_num_seg_t {
	uint32_t tso_cmn_num_seg;
};

/**
 * qdf_tso_num_seg_elem_t - num of tso segment element for jumbo skb
 * @next: pointer to the next segment
 * @num_seg: instance of num of seg
 */
struct qdf_tso_num_seg_elem_t {
	struct qdf_tso_num_seg_elem_t *next;
	struct qdf_tso_num_seg_t num_seg;
};

/**
 * struct qdf_tso_info_t - TSO information extracted
 * @is_tso: is this is a TSO frame
 * @num_segs: number of segments
 * @tso_seg_list: list of TSO segments for this jumbo packet
 * @curr_seg: segment that is currently being processed
 * @tso_num_seg_list: num of tso seg for this jumbo packet
 * @msdu_stats_idx: msdu index for tso stats
 *
 * This structure holds the TSO information extracted after parsing the TSO
 * jumbo network buffer. It contains a chain of the TSO segments belonging to
 * the jumbo packet
 */
struct qdf_tso_info_t {
	uint8_t is_tso;
	uint32_t num_segs;
	struct qdf_tso_seg_elem_t *tso_seg_list;
	struct qdf_tso_seg_elem_t *curr_seg;
	struct qdf_tso_num_seg_elem_t *tso_num_seg_list;
	uint32_t msdu_stats_idx;
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
 * @QDF_UNIT_TEST_WOW_SUSPEND: WoW unit test suspend
 */
enum qdf_suspend_type {
	QDF_SYSTEM_SUSPEND,
	QDF_RUNTIME_SUSPEND,
	QDF_UNIT_TEST_WOW_SUSPEND
};

/**
 * enum qdf_hang_reason - host hang/ssr reason
 * @QDF_REASON_UNSPECIFIED: Unspecified reason
 * @QDF_RX_HASH_NO_ENTRY_FOUND: No Map for the MAC entry for the received frame
 * @QDF_PEER_DELETION_TIMEDOUT: peer deletion timeout happened
 * @QDF_PEER_UNMAP_TIMEDOUT: peer unmap timeout
 * @QDF_SCAN_REQ_EXPIRED: Scan request timed out
 * @QDF_SCAN_ATTEMPT_FAILURES: Consecutive Scan attempt failures
 * @QDF_GET_MSG_BUFF_FAILURE: Unable to get the message buffer
 * @QDF_ACTIVE_LIST_TIMEOUT: Current command processing is timedout
 * @QDF_SUSPEND_TIMEOUT: Timeout for an ACK from FW for suspend request
 * @QDF_RESUME_TIMEOUT: Timeout for an ACK from FW for resume request
 * @QDF_WMI_EXCEED_MAX_PENDING_CMDS: wmi exceed max pending cmd
 * @QDF_AP_STA_CONNECT_REQ_TIMEOUT: SAP peer assoc timeout from FW
 * @QDF_STA_AP_CONNECT_REQ_TIMEOUT: STA peer assoc timeout from FW
 * @QDF_MAC_HW_MODE_CHANGE_TIMEOUT: HW mode change timeout from FW
 * @QDF_MAC_HW_MODE_CONFIG_TIMEOUT: HW dual mac cfg timeout from FW
 * @QDF_VDEV_START_RESPONSE_TIMED_OUT: Start response timeout from FW
 * @QDF_VDEV_RESTART_RESPONSE_TIMED_OUT: Restart response timeout from FW
 * @QDF_VDEV_STOP_RESPONSE_TIMED_OUT: Stop response timeout from FW
 * @QDF_VDEV_DELETE_RESPONSE_TIMED_OUT: Delete response timeout from FW
 * @QDF_VDEV_PEER_DELETE_ALL_RESPONSE_TIMED_OUT: Peer delete all resp timeout
 * @QDF_WMI_BUF_SEQUENCE_MISMATCH: WMI Tx completion buffer sequence mismatch
 * @QDF_HAL_REG_WRITE_FAILURE: HAL register writing failures
 * @QDF_SUSPEND_NO_CREDIT: host lack of credit after suspend
 * @QCA_HANG_BUS_FAILURE: Bus failure
 * @QDF_TASKLET_CREDIT_LATENCY_DETECT: tasklet or credit latency happened
 * @QDF_RX_REG_PKT_ROUTE_ERR: MSDU buf errors exceed thresh in REO err path
 * @QDF_VDEV_SM_OUT_OF_SYNC: Vdev SM is out of sync and connect req received
 * when already connected
 * @QDF_STATS_REQ_TIMEDOUT: Stats request timedout
 * @QDF_RSO_STOP_RSP_TIMEOUT: Firmware hasn't sent RSO stop response
 * @QDF_HOST_WAKEUP_REASON_PAGEFAULT: Host wakeup because of pagefault
 * @QDF_SCHED_TIMEOUT: Scheduler watchdog timedout
 * @QDF_SELF_PEER_DEL_FAILED: Failed to send self peer deletion cmd to fw
 * @QDF_DEL_SELF_STA_FAILED: Received del self sta without del bss
 * @QDF_FLUSH_LOGS : Recovery needed when sending flush completion to userspace
 */
enum qdf_hang_reason {
	QDF_REASON_UNSPECIFIED,
	QDF_RX_HASH_NO_ENTRY_FOUND,
	QDF_PEER_DELETION_TIMEDOUT,
	QDF_PEER_UNMAP_TIMEDOUT,
	QDF_SCAN_REQ_EXPIRED,
	QDF_SCAN_ATTEMPT_FAILURES,
	QDF_GET_MSG_BUFF_FAILURE,
	QDF_ACTIVE_LIST_TIMEOUT,
	QDF_SUSPEND_TIMEOUT,
	QDF_RESUME_TIMEOUT,
	QDF_WMI_EXCEED_MAX_PENDING_CMDS,
	QDF_AP_STA_CONNECT_REQ_TIMEOUT,
	QDF_STA_AP_CONNECT_REQ_TIMEOUT,
	QDF_MAC_HW_MODE_CHANGE_TIMEOUT,
	QDF_MAC_HW_MODE_CONFIG_TIMEOUT,
	QDF_VDEV_START_RESPONSE_TIMED_OUT,
	QDF_VDEV_RESTART_RESPONSE_TIMED_OUT,
	QDF_VDEV_STOP_RESPONSE_TIMED_OUT,
	QDF_VDEV_DELETE_RESPONSE_TIMED_OUT,
	QDF_VDEV_PEER_DELETE_ALL_RESPONSE_TIMED_OUT,
	QDF_WMI_BUF_SEQUENCE_MISMATCH,
	QDF_HAL_REG_WRITE_FAILURE,
	QDF_SUSPEND_NO_CREDIT,
	QCA_HANG_BUS_FAILURE,
	QDF_TASKLET_CREDIT_LATENCY_DETECT,
	QDF_RX_REG_PKT_ROUTE_ERR,
	QDF_VDEV_SM_OUT_OF_SYNC,
	QDF_STATS_REQ_TIMEDOUT,
	QDF_TX_DESC_LEAK,
	QDF_RSO_STOP_RSP_TIMEOUT,
	QDF_HOST_WAKEUP_REASON_PAGEFAULT,
	QDF_SCHED_TIMEOUT,
	QDF_SELF_PEER_DEL_FAILED,
	QDF_DEL_SELF_STA_FAILED,
	QDF_FLUSH_LOGS,
};

/**
 * enum qdf_stats_verbosity_level - Verbosity levels for stats
 * for which want to have different levels
 * @QDF_STATS_VERBOSITY_LEVEL_LOW: Stats verbosity level low
 * @QDF_STATS_VERBOSITY_LEVEL_HIGH: Stats verbosity level high
 */
enum qdf_stats_verbosity_level {
	QDF_STATS_VERBOSITY_LEVEL_LOW,
	QDF_STATS_VERBOSITY_LEVEL_HIGH
};

/**
 * enum qdf_clock_id - The clock IDs of the various system clocks
 * @QDF_CLOCK_REALTIME: Clock is close to current time of day
 * @QDF_CLOCK_MONOTONIC: Clock is absolute elapsed time
 */
enum qdf_clock_id {
	QDF_CLOCK_REALTIME = __QDF_CLOCK_REALTIME,
	QDF_CLOCK_MONOTONIC = __QDF_CLOCK_MONOTONIC
};

/**
 * enum qdf_hrtimer_mode - Mode arguments of qdf_hrtimer_data_t
 * related functions
 * @QDF_HRTIMER_MODE_ABS: Time value is absolute
 * @QDF_HRTIMER_MODE_REL: Time value is relative to now
 * @QDF_HRTIMER_MODE_PINNED: Timer is bound to CPU
 */
enum qdf_hrtimer_mode {
	QDF_HRTIMER_MODE_ABS = __QDF_HRTIMER_MODE_ABS,
	QDF_HRTIMER_MODE_REL = __QDF_HRTIMER_MODE_REL,
	QDF_HRTIMER_MODE_PINNED =  __QDF_HRTIMER_MODE_PINNED,
};

/**
 * enum qdf_hrtimer_restart_status - Return values for the
 * qdf_hrtimer_data_t callback function
 * @QDF_HRTIMER_NORESTART: Timer is not restarted
 * @QDF_HRTIMER_RESTART: Timer must be restarted
 */
enum qdf_hrtimer_restart_status {
	QDF_HRTIMER_NORESTART = __QDF_HRTIMER_NORESTART,
	QDF_HRTIMER_RESTART = __QDF_HRTIMER_RESTART,
};

/**
 * enum qdf_context_mode - Values for the
 * hrtimer context
 * @QDF_CONTEXT_HARDWARE: Runs in hw interrupt context
 * @QDF_CONTEXT_TASKLET: Runs in tasklet context
 */
enum qdf_context_mode {
	QDF_CONTEXT_HARDWARE = 0,
	QDF_CONTEXT_TASKLET = 1,
};

/**
 * enum qdf_dp_tx_rx_status - TX/RX packet status
 * @QDF_TX_RX_STATUS_INVALID: default invalid status
 * @QDF_TX_RX_STATUS_OK: successfully sent + acked
 * @QDF_TX_RX_STATUS_DISCARD: queued but not sent over air
 * @QDF_TX_RX_STATUS_NO_ACK: packet sent but no ack received
 * @QDF_TX_RX_STATUS_DROP: packet dropped due to congestion
 * @QDF_TX_RX_STATUS_DOWNLOAD_SUCC: packet delivered to target
 * @QDF_TX_RX_STATUS_DEFAULT: default status
 * @QDF_TX_RX_STATUS_MAX:
 */
enum qdf_dp_tx_rx_status {
	QDF_TX_RX_STATUS_INVALID,
	QDF_TX_RX_STATUS_OK,
	QDF_TX_RX_STATUS_FW_DISCARD,
	QDF_TX_RX_STATUS_NO_ACK,
	QDF_TX_RX_STATUS_DROP,
	QDF_TX_RX_STATUS_DOWNLOAD_SUCC,
	QDF_TX_RX_STATUS_DEFAULT,
	QDF_TX_RX_STATUS_MAX
};

/**
 * enum qdf_pkt_type - TX/RX packet type
 * @QDF_TX_MGMT_PKT: Tx Management Packet
 * @QDF_TX_DATA_PKT: Tx data Packet
 * @QDF_RX_MGMT_PKT: Rx Management Packet
 * @QDF_RX_DATA_PKT: Rx data Packet
 * @QDF_INVALID_PKT: Invalid Packet type
 */
enum qdf_pkt_type {
	QDF_TX_MGMT_PKT,
	QDF_TX_DATA_PKT,
	QDF_RX_MGMT_PKT,
	QDF_RX_DATA_PKT,
	QDF_INVALID_PKT
};

/**
 * enum qdf_dp_a_status - A_STATUS
 * @QDF_A_STATUS_ERROR: Generic error return
 * @QDF_A_STATUS_OK: success
 */
enum qdf_dp_a_status {
	QDF_A_STATUS_ERROR = -1,
	QDF_A_STATUS_OK,
};

/**
 * enum qdf_iommu_attr- qdf iommu domain attribute
 * @QDF_DOMAIN_ATTR_GEOMETRY: Domain attribute geometry
 * @QDF_DOMAIN_ATTR_PAGING: Domain attribute paging
 * @QDF_DOMAIN_ATTR_WINDOWS: Domain attribute windows
 * @QDF_DOMAIN_ATTR_FSL_PAMU_STASH: Domain attribute fsl pamu stash
 * @QDF_DOMAIN_ATTR_FSL_PAMU_ENABLE: Domain attribute fsl pamu enable
 * @QDF_DOMAIN_ATTR_FSL_PAMUV1: Domain attribute fsl pamu v1
 * @QDF_DOMAIN_ATTR_NESTING: Domain attribute Nesting
 * @QDF_DOMAIN_ATTR_DMA_USE_FLUSH_QUEUE: Domain attribute dma use flush queue
 * @QDF_DOMAIN_ATTR_PT_BASE_ADDR: Domain attribute pt base address
 * @QDF_DOMAIN_ATTR_CONTEXT_BANK: Domain attribute context bank
 * @QDF_DOMAIN_ATTR_DYNAMIC: Domain attribute dynamic
 * @QDF_DOMAIN_ATTR_TTBR0: Domain attribute TTBR0
 * @QDF_DOMAIN_ATTR_CONTEXTIDR: Domain attribute contextidr
 * @QDF_DOMAIN_ATTR_PROCID: Domain attribute procid
 * @QDF_DOMAIN_ATTR_NON_FATAL_FAULTS: Domain attribute non fatal faults
 * @QDF_DOMAIN_ATTR_S1_BYPASS: Domain attribute S1 bypass
 * @QDF_DOMAIN_ATTR_ATOMIC: Domain attribute atomic
 * @QDF_DOMAIN_ATTR_SECURE_VMID: Domain attribute secure cmid
 * @QDF_DOMAIN_ATTR_FAST: Domain attribute fast
 * @QDF_DOMAIN_ATTR_PGTBL_INFO: Domain attribute pgtbl info
 * @QDF_DOMAIN_ATTR_USE_UPSTREAM_HINT: Domain attribute use upsteram hint
 * @QDF_DOMAIN_ATTR_EARLY_MAP: Domain attribute early map
 * @QDF_DOMAIN_ATTR_PAGE_TABLE_IS_COHERENT: Domain attribute page table
 * is coherrent
 * @QDF_DOMAIN_ATTR_PAGE_TABLE_FORCE_COHERENT: Domain attribute page table
 * force coherrent
 * @QDF_DOMAIN_ATTR_USE_LLC_NWA: Domain attribute use llc nwa
 * @QDF_DOMAIN_ATTR_SPLIT_TABLES: Domain attribute split tables
 * @QDF_DOMAIN_ATTR_FAULT_MODEL_NO_CFRE: Domain attribute fault model no cfre
 * @QDF_DOMAIN_ATTR_FAULT_MODEL_NO_STALL: Domain attribute fault model no stall
 * @QDF_DOMAIN_ATTR_FAULT_MODEL_HUPCF: Domain attribute fault model hupcf
 * @QDF_DOMAIN_ATTR_MAX: Domain attribute max
 */
enum qdf_iommu_attr {
	QDF_DOMAIN_ATTR_GEOMETRY,
	QDF_DOMAIN_ATTR_PAGING,
	QDF_DOMAIN_ATTR_WINDOWS,
	QDF_DOMAIN_ATTR_FSL_PAMU_STASH,
	QDF_DOMAIN_ATTR_FSL_PAMU_ENABLE,
	QDF_DOMAIN_ATTR_FSL_PAMUV1,
	QDF_DOMAIN_ATTR_NESTING,
	QDF_DOMAIN_ATTR_DMA_USE_FLUSH_QUEUE,
	QDF_DOMAIN_ATTR_CONTEXT_BANK,
	QDF_DOMAIN_ATTR_NON_FATAL_FAULTS,
	QDF_DOMAIN_ATTR_S1_BYPASS,
	QDF_DOMAIN_ATTR_ATOMIC,
	QDF_DOMAIN_ATTR_SECURE_VMID,
	QDF_DOMAIN_ATTR_FAST,
	QDF_DOMAIN_ATTR_PGTBL_INFO,
	QDF_DOMAIN_ATTR_USE_UPSTREAM_HINT,
	QDF_DOMAIN_ATTR_EARLY_MAP,
	QDF_DOMAIN_ATTR_PAGE_TABLE_IS_COHERENT,
	QDF_DOMAIN_ATTR_PAGE_TABLE_FORCE_COHERENT,
	QDF_DOMAIN_ATTR_USE_LLC_NWA,
	QDF_DOMAIN_ATTR_SPLIT_TABLES,
	QDF_DOMAIN_ATTR_FAULT_MODEL_NO_CFRE,
	QDF_DOMAIN_ATTR_FAULT_MODEL_NO_STALL,
	QDF_DOMAIN_ATTR_FAULT_MODEL_HUPCF,
	QDF_DOMAIN_ATTR_MAX,
};

#endif /* __QDF_TYPES_H */
