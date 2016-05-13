/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_ipa.c
 *
 * WLAN HDD and ipa interface implementation
 * Originally written by Qualcomm Atheros, Inc
 */

#ifdef IPA_OFFLOAD

/* Include Files */
#include <linux/ipa.h>
#include <wlan_hdd_includes.h>
#include <wlan_hdd_ipa.h>

#include <linux/etherdevice.h>
#include <linux/atomic.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/list.h>
#include <linux/debugfs.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <wlan_hdd_softap_tx_rx.h>
#include <ol_txrx_osif_api.h>
#include <cdp_txrx_peer_ops.h>

#include "cds_sched.h"

#include "wma.h"
#include "wma_api.h"

#include "cdp_txrx_ipa.h"

#define HDD_IPA_DESC_BUFFER_RATIO          4
#define HDD_IPA_IPV4_NAME_EXT              "_ipv4"
#define HDD_IPA_IPV6_NAME_EXT              "_ipv6"

#define HDD_IPA_RX_INACTIVITY_MSEC_DELAY   1000
#define HDD_IPA_UC_WLAN_8023_HDR_SIZE      14
/* WDI TX and RX PIPE */
#define HDD_IPA_UC_NUM_WDI_PIPE            2
#define HDD_IPA_UC_MAX_PENDING_EVENT       33

#define HDD_IPA_UC_DEBUG_DUMMY_MEM_SIZE    32000
#define HDD_IPA_UC_RT_DEBUG_PERIOD         300
#define HDD_IPA_UC_RT_DEBUG_BUF_COUNT      30
#define HDD_IPA_UC_RT_DEBUG_FILL_INTERVAL  10000

#define HDD_IPA_WLAN_HDR_DES_MAC_OFFSET    0
#define HDD_IPA_MAX_IFACE                  3
#define HDD_IPA_MAX_SYSBAM_PIPE            4
#define HDD_IPA_RX_PIPE                    HDD_IPA_MAX_IFACE
#define HDD_IPA_ENABLE_MASK                BIT(0)
#define HDD_IPA_PRE_FILTER_ENABLE_MASK     BIT(1)
#define HDD_IPA_IPV6_ENABLE_MASK           BIT(2)
#define HDD_IPA_RM_ENABLE_MASK             BIT(3)
#define HDD_IPA_CLK_SCALING_ENABLE_MASK    BIT(4)
#define HDD_IPA_UC_ENABLE_MASK             BIT(5)
#define HDD_IPA_UC_STA_ENABLE_MASK         BIT(6)
#define HDD_IPA_REAL_TIME_DEBUGGING        BIT(8)

#define HDD_IPA_MAX_PENDING_EVENT_COUNT    20

typedef enum {
	HDD_IPA_UC_OPCODE_TX_SUSPEND = 0,
	HDD_IPA_UC_OPCODE_TX_RESUME = 1,
	HDD_IPA_UC_OPCODE_RX_SUSPEND = 2,
	HDD_IPA_UC_OPCODE_RX_RESUME = 3,
	HDD_IPA_UC_OPCODE_STATS = 4,
	/* keep this last */
	HDD_IPA_UC_OPCODE_MAX
} hdd_ipa_uc_op_code;

/**
 * enum - Reason codes for stat query
 *
 * @HDD_IPA_UC_STAT_REASON_NONE: Initial value
 * @HDD_IPA_UC_STAT_REASON_DEBUG: For debug/info
 * @HDD_IPA_UC_STAT_REASON_BW_CAL: For bandwidth calibration
 */
enum {
	HDD_IPA_UC_STAT_REASON_NONE,
	HDD_IPA_UC_STAT_REASON_DEBUG,
	HDD_IPA_UC_STAT_REASON_BW_CAL
};

/**
 * enum hdd_ipa_rm_state - IPA resource manager state
 * @HDD_IPA_RM_RELEASED:      PROD pipe resource released
 * @HDD_IPA_RM_GRANT_PENDING: PROD pipe resource requested but not granted yet
 * @HDD_IPA_RM_GRANTED:       PROD pipe resource granted
 */
enum hdd_ipa_rm_state {
	HDD_IPA_RM_RELEASED,
	HDD_IPA_RM_GRANT_PENDING,
	HDD_IPA_RM_GRANTED,
};

struct llc_snap_hdr {
	uint8_t dsap;
	uint8_t ssap;
	uint8_t resv[4];
	__be16 eth_type;
} __packed;

/**
 * struct hdd_ipa_tx_hdr - header type which IPA should handle to TX packet
 * @eth:      ether II header
 * @llc_snap: LLC snap header
 *
 */
struct hdd_ipa_tx_hdr {
	struct ethhdr eth;
	struct llc_snap_hdr llc_snap;
} __packed;

/**
 * struct frag_header - fragment header type registered to IPA hardware
 * @length:    fragment length
 * @reserved1: Reserved not used
 * @reserved2: Reserved not used
 *
 */
struct frag_header {
	uint16_t length;
	uint32_t reserved1;
	uint32_t reserved2;
} __packed;

/**
 * struct ipa_header - ipa header type registered to IPA hardware
 * @vdev_id:  vdev id
 * @reserved: Reserved not used
 *
 */
struct ipa_header {
	uint32_t
		vdev_id:8,	/* vdev_id field is LSB of IPA DESC */
		reserved:24;
} __packed;

/**
 * struct hdd_ipa_uc_tx_hdr - full tx header registered to IPA hardware
 * @frag_hd: fragment header
 * @ipa_hd:  ipa header
 * @eth:     ether II header
 *
 */
struct hdd_ipa_uc_tx_hdr {
	struct frag_header frag_hd;
	struct ipa_header ipa_hd;
	struct ethhdr eth;
} __packed;

/**
 * struct hdd_ipa_cld_hdr - IPA CLD Header
 * @reserved: reserved fields
 * @iface_id: interface ID
 * @sta_id: Station ID
 *
 * Packed 32-bit structure
 * +----------+----------+--------------+--------+
 * | Reserved | QCMAP ID | interface id | STA ID |
 * +----------+----------+--------------+--------+
 */
struct hdd_ipa_cld_hdr {
	uint8_t reserved[2];
	uint8_t iface_id;
	uint8_t sta_id;
} __packed;

struct hdd_ipa_rx_hdr {
	struct hdd_ipa_cld_hdr cld_hdr;
	struct ethhdr eth;
} __packed;

struct hdd_ipa_pm_tx_cb {
	struct hdd_ipa_iface_context *iface_context;
	struct ipa_rx_data *ipa_tx_desc;
};

struct hdd_ipa_uc_rx_hdr {
	struct ethhdr eth;
} __packed;

struct hdd_ipa_sys_pipe {
	uint32_t conn_hdl;
	uint8_t conn_hdl_valid;
	struct ipa_sys_connect_params ipa_sys_params;
};

struct hdd_ipa_iface_stats {
	uint64_t num_tx;
	uint64_t num_tx_drop;
	uint64_t num_tx_err;
	uint64_t num_tx_cac_drop;
	uint64_t num_rx_prefilter;
	uint64_t num_rx_ipa_excep;
	uint64_t num_rx_recv;
	uint64_t num_rx_recv_mul;
	uint64_t num_rx_send_desc_err;
	uint64_t max_rx_mul;
};

struct hdd_ipa_priv;

struct hdd_ipa_iface_context {
	struct hdd_ipa_priv *hdd_ipa;
	hdd_adapter_t *adapter;
	void *tl_context;

	enum ipa_client_type cons_client;
	enum ipa_client_type prod_client;

	uint8_t iface_id;       /* This iface ID */
	uint8_t sta_id;         /* This iface station ID */
	qdf_spinlock_t interface_lock;
	uint32_t ifa_address;
	struct hdd_ipa_iface_stats stats;
};

struct hdd_ipa_stats {
	uint32_t event[IPA_WLAN_EVENT_MAX];
	uint64_t num_send_msg;
	uint64_t num_free_msg;

	uint64_t num_rm_grant;
	uint64_t num_rm_release;
	uint64_t num_rm_grant_imm;
	uint64_t num_cons_perf_req;
	uint64_t num_prod_perf_req;

	uint64_t num_rx_drop;
	uint64_t num_rx_ipa_tx_dp;
	uint64_t num_rx_ipa_splice;
	uint64_t num_rx_ipa_loop;
	uint64_t num_rx_ipa_tx_dp_err;
	uint64_t num_rx_ipa_write_done;
	uint64_t num_max_ipa_tx_mul;
	uint64_t num_rx_ipa_hw_maxed_out;
	uint64_t max_pend_q_cnt;

	uint64_t num_tx_comp_cnt;
	uint64_t num_tx_queued;
	uint64_t num_tx_dequeued;
	uint64_t num_max_pm_queue;

	uint64_t num_freeq_empty;
	uint64_t num_pri_freeq_empty;
	uint64_t num_rx_excep;
	uint64_t num_tx_bcmc;
	uint64_t num_tx_bcmc_err;
};

struct ipa_uc_stas_map {
	bool is_reserved;
	uint8_t sta_id;
};
struct op_msg_type {
	uint8_t msg_t;
	uint8_t rsvd;
	uint16_t op_code;
	uint16_t len;
	uint16_t rsvd_snd;
};

struct ipa_uc_fw_stats {
	uint32_t tx_comp_ring_base;
	uint32_t tx_comp_ring_size;
	uint32_t tx_comp_ring_dbell_addr;
	uint32_t tx_comp_ring_dbell_ind_val;
	uint32_t tx_comp_ring_dbell_cached_val;
	uint32_t tx_pkts_enqueued;
	uint32_t tx_pkts_completed;
	uint32_t tx_is_suspend;
	uint32_t tx_reserved;
	uint32_t rx_ind_ring_base;
	uint32_t rx_ind_ring_size;
	uint32_t rx_ind_ring_dbell_addr;
	uint32_t rx_ind_ring_dbell_ind_val;
	uint32_t rx_ind_ring_dbell_ind_cached_val;
	uint32_t rx_ind_ring_rdidx_addr;
	uint32_t rx_ind_ring_rd_idx_cached_val;
	uint32_t rx_refill_idx;
	uint32_t rx_num_pkts_indicated;
	uint32_t rx_buf_refilled;
	uint32_t rx_num_ind_drop_no_space;
	uint32_t rx_num_ind_drop_no_buf;
	uint32_t rx_is_suspend;
	uint32_t rx_reserved;
};

struct ipa_uc_pending_event {
	qdf_list_node_t node;
	hdd_adapter_t *adapter;
	enum ipa_wlan_event type;
	uint8_t sta_id;
	uint8_t mac_addr[QDF_MAC_ADDR_SIZE];
};

/**
 * struct uc_rm_work_struct
 * @work: uC RM work
 * @event: IPA RM event
 */
struct uc_rm_work_struct {
	struct work_struct work;
	enum ipa_rm_event event;
};

/**
 * struct uc_op_work_struct
 * @work: uC OP work
 * @msg: OP message
 */
struct uc_op_work_struct {
	struct work_struct work;
	struct op_msg_type *msg;
};
static uint8_t vdev_to_iface[CSR_ROAM_SESSION_MAX];

/**
 * struct uc_rt_debug_info
 * @time: system time
 * @ipa_excep_count: IPA exception packet count
 * @rx_drop_count: IPA Rx drop packet count
 * @net_sent_count: IPA Rx packet sent to network stack count
 * @rx_discard_count: IPA Rx discard packet count
 * @rx_mcbc_count: IPA Rx BCMC packet count
 * @tx_mcbc_count: IPA Tx BCMC packet countt
 * @tx_fwd_count: IPA Tx forward packet count
 * @rx_destructor_call: IPA Rx packet destructor count
 */
struct uc_rt_debug_info {
	unsigned long time;
	uint64_t ipa_excep_count;
	uint64_t rx_drop_count;
	uint64_t net_sent_count;
	uint64_t rx_discard_count;
	uint64_t rx_mcbc_count;
	uint64_t tx_mcbc_count;
	uint64_t tx_fwd_count;
	uint64_t rx_destructor_call;
};

struct hdd_ipa_priv {
	struct hdd_ipa_sys_pipe sys_pipe[HDD_IPA_MAX_SYSBAM_PIPE];
	struct hdd_ipa_iface_context iface_context[HDD_IPA_MAX_IFACE];
	uint8_t num_iface;
	enum hdd_ipa_rm_state rm_state;
	/*
	 * IPA driver can send RM notifications with IRQ disabled so using qdf
	 * APIs as it is taken care gracefully. Without this, kernel would throw
	 * an warning if spin_lock_bh is used while IRQ is disabled
	 */
	qdf_spinlock_t rm_lock;
	struct uc_rm_work_struct uc_rm_work;
	struct uc_op_work_struct uc_op_work[HDD_IPA_UC_OPCODE_MAX];
	qdf_wake_lock_t wake_lock;
	struct delayed_work wake_lock_work;
	bool wake_lock_released;

	enum ipa_client_type prod_client;

	atomic_t tx_ref_cnt;
	qdf_nbuf_queue_t pm_queue_head;
	struct work_struct pm_work;
	qdf_spinlock_t pm_lock;
	bool suspended;

	uint32_t pending_hw_desc_cnt;
	uint32_t hw_desc_cnt;
	spinlock_t q_lock;
	uint32_t freeq_cnt;
	struct list_head free_desc_head;

	uint32_t pend_q_cnt;
	struct list_head pend_desc_head;

	hdd_context_t *hdd_ctx;

	struct dentry *debugfs_dir;
	struct hdd_ipa_stats stats;

	struct notifier_block ipv4_notifier;
	uint32_t curr_prod_bw;
	uint32_t curr_cons_bw;

	uint8_t activated_fw_pipe;
	uint8_t sap_num_connected_sta;
	uint8_t sta_connected;
	uint32_t tx_pipe_handle;
	uint32_t rx_pipe_handle;
	bool resource_loading;
	bool resource_unloading;
	bool pending_cons_req;
	struct ipa_uc_stas_map assoc_stas_map[WLAN_MAX_STA_COUNT];
	qdf_list_t pending_event;
	qdf_mutex_t event_lock;
	bool ipa_pipes_down;
	uint32_t ipa_tx_packets_diff;
	uint32_t ipa_rx_packets_diff;
	uint32_t ipa_p_tx_packets;
	uint32_t ipa_p_rx_packets;
	uint32_t stat_req_reason;
	uint64_t ipa_tx_forward;
	uint64_t ipa_rx_discard;
	uint64_t ipa_rx_net_send_count;
	uint64_t ipa_rx_internel_drop_count;
	uint64_t ipa_rx_destructor_count;
	qdf_mc_timer_t rt_debug_timer;
	struct uc_rt_debug_info rt_bug_buffer[HDD_IPA_UC_RT_DEBUG_BUF_COUNT];
	unsigned int rt_buf_fill_index;
	qdf_mc_timer_t rt_debug_fill_timer;
	qdf_mutex_t rt_debug_lock;
	qdf_mutex_t ipa_lock;
	struct ol_txrx_ipa_resources ipa_resource;
	/* IPA UC doorbell registers paddr */
	qdf_dma_addr_t tx_comp_doorbell_paddr;
	qdf_dma_addr_t rx_ready_doorbell_paddr;
};

/**
 * FIXME: The following conversion routines are just stubs.
 *        They will be implemented fully by another update.
 *        The stubs will let the compile go ahead, and functionality
 *        is broken.
 * This should be OK and IPA is not enabled yet
 */
void *wlan_hdd_stub_priv_to_addr(uint32_t priv)
{
	void    *vaddr;
	uint32_t ipa_priv = priv;

	vaddr = &ipa_priv; /* just to use the var */
	vaddr = NULL;
	return vaddr;
}

uint32_t wlan_hdd_stub_addr_to_priv(void *ptr)
{
	uint32_t       ipa_priv = 0;

	BUG_ON(ptr == NULL);
	return ipa_priv;
}

#define HDD_IPA_WLAN_FRAG_HEADER        sizeof(struct frag_header)
#define HDD_IPA_WLAN_IPA_HEADER         sizeof(struct ipa_header)
#define HDD_IPA_WLAN_CLD_HDR_LEN        sizeof(struct hdd_ipa_cld_hdr)
#define HDD_IPA_UC_WLAN_CLD_HDR_LEN     0
#define HDD_IPA_WLAN_TX_HDR_LEN         sizeof(struct hdd_ipa_tx_hdr)
#define HDD_IPA_UC_WLAN_TX_HDR_LEN      sizeof(struct hdd_ipa_uc_tx_hdr)
#define HDD_IPA_WLAN_RX_HDR_LEN         sizeof(struct hdd_ipa_rx_hdr)
#define HDD_IPA_UC_WLAN_RX_HDR_LEN      sizeof(struct hdd_ipa_uc_rx_hdr)
#define HDD_IPA_UC_WLAN_HDR_DES_MAC_OFFSET \
	(HDD_IPA_WLAN_FRAG_HEADER + HDD_IPA_WLAN_IPA_HEADER)

#define HDD_IPA_FW_RX_DESC_DISCARD_M 0x1
#define HDD_IPA_FW_RX_DESC_FORWARD_M 0x2

#define HDD_IPA_GET_IFACE_ID(_data) \
	(((struct hdd_ipa_cld_hdr *) (_data))->iface_id)

#define HDD_IPA_LOG(LVL, fmt, args ...) \
	QDF_TRACE(QDF_MODULE_ID_HDD, LVL, \
		  "%s:%d: "fmt, __func__, __LINE__, ## args)

#define HDD_IPA_DBG_DUMP(_lvl, _prefix, _buf, _len) \
	do { \
		QDF_TRACE(QDF_MODULE_ID_HDD, _lvl, "%s:", _prefix); \
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD, _lvl, _buf, _len); \
	} while (0)

#define HDD_IPA_IS_CONFIG_ENABLED(_hdd_ctx, _mask) \
	(((_hdd_ctx)->config->IpaConfig & (_mask)) == (_mask))

#define HDD_IPA_INCREASE_INTERNAL_DROP_COUNT(hdd_ipa) \
			do { \
				hdd_ipa->ipa_rx_internel_drop_count++; \
			} while (0)
#define HDD_IPA_INCREASE_NET_SEND_COUNT(hdd_ipa) \
			do { \
				hdd_ipa->ipa_rx_net_send_count++; \
			} while (0)
#define HDD_BW_GET_DIFF(_x, _y) (unsigned long)((ULONG_MAX - (_y)) + (_x) + 1)

#if defined (QCA_WIFI_3_0) && defined (CONFIG_IPA3)
#define HDD_IPA_WDI2_SET(pipe_in, ipa_ctxt) \
do { \
	pipe_in.u.ul.rdy_ring_rp_va = \
		ipa_ctxt->ipa_resource.rx_proc_done_idx_vaddr; \
	pipe_in.u.ul.rdy_comp_ring_base_pa = \
		ipa_ctxt->ipa_resource.rx2_rdy_ring_base_paddr;\
	pipe_in.u.ul.rdy_comp_ring_size = \
		ipa_ctxt->ipa_resource.rx2_rdy_ring_size; \
	pipe_in.u.ul.rdy_comp_ring_wp_pa = \
		ipa_ctxt->ipa_resource.rx2_proc_done_idx_paddr; \
	pipe_in.u.ul.rdy_comp_ring_wp_va = \
		ipa_ctxt->ipa_resource.rx2_proc_done_idx_vaddr; \
} while (0)
#else
/* Do nothing */
#define HDD_IPA_WDI2_SET(pipe_in, ipa_ctxt)
#endif /* IPA3 */

static struct hdd_ipa_adapter_2_client {
	enum ipa_client_type cons_client;
	enum ipa_client_type prod_client;
} hdd_ipa_adapter_2_client[HDD_IPA_MAX_IFACE] = {
	{
		IPA_CLIENT_WLAN2_CONS, IPA_CLIENT_WLAN1_PROD
	}, {
		IPA_CLIENT_WLAN3_CONS, IPA_CLIENT_WLAN1_PROD
	}, {
		IPA_CLIENT_WLAN4_CONS, IPA_CLIENT_WLAN1_PROD
	},
};

/* For Tx pipes, use Ethernet-II Header format */
struct hdd_ipa_uc_tx_hdr ipa_uc_tx_hdr = {
	{
		0x0000,
		0x00000000,
		0x00000000
	},
	{
		0x00000000
	},
	{
		{0x00, 0x03, 0x7f, 0xaa, 0xbb, 0xcc},
		{0x00, 0x03, 0x7f, 0xdd, 0xee, 0xff},
		0x0008
	}
};

/* For Tx pipes, use 802.3 Header format */
static struct hdd_ipa_tx_hdr ipa_tx_hdr = {
	{
		{0xDE, 0xAD, 0xBE, 0xEF, 0xFF, 0xFF},
		{0xDE, 0xAD, 0xBE, 0xEF, 0xFF, 0xFF},
		0x00            /* length can be zero */
	},
	{
		/* LLC SNAP header 8 bytes */
		0xaa, 0xaa,
		{0x03, 0x00, 0x00, 0x00},
		0x0008          /* type value(2 bytes) ,filled by wlan  */
		/* 0x0800 - IPV4, 0x86dd - IPV6 */
	}
};

static const char *op_string[] = {
	"TX_SUSPEND",
	"TX_RESUME",
	"RX_SUSPEND",
	"RX_RESUME",
	"STATS",
};

static struct hdd_ipa_priv *ghdd_ipa;

/* Local Function Prototypes */
static void hdd_ipa_i2w_cb(void *priv, enum ipa_dp_evt_type evt,
			   unsigned long data);
static void hdd_ipa_w2i_cb(void *priv, enum ipa_dp_evt_type evt,
			   unsigned long data);

static void hdd_ipa_cleanup_iface(struct hdd_ipa_iface_context *iface_context);
static void hdd_ipa_uc_proc_pending_event (struct hdd_ipa_priv *hdd_ipa);

/**
 * hdd_ipa_is_enabled() - Is IPA enabled?
 * @hdd_ctx: Global HDD context
 *
 * Return: true if IPA is enabled, false otherwise
 */
bool hdd_ipa_is_enabled(hdd_context_t *hdd_ctx)
{
	return HDD_IPA_IS_CONFIG_ENABLED(hdd_ctx, HDD_IPA_ENABLE_MASK);
}

/**
 * hdd_ipa_uc_is_enabled() - Is IPA uC offload enabled?
 * @hdd_ctx: Global HDD context
 *
 * Return: true if IPA uC offload is enabled, false otherwise
 */
bool hdd_ipa_uc_is_enabled(hdd_context_t *hdd_ctx)
{
	return HDD_IPA_IS_CONFIG_ENABLED(hdd_ctx, HDD_IPA_UC_ENABLE_MASK);
}

/**
 * hdd_ipa_uc_sta_is_enabled() - Is STA mode IPA uC offload enabled?
 * @hdd_ctx: Global HDD context
 *
 * Return: true if STA mode IPA uC offload is enabled, false otherwise
 */
static inline bool hdd_ipa_uc_sta_is_enabled(hdd_context_t *hdd_ctx)
{
	return HDD_IPA_IS_CONFIG_ENABLED(hdd_ctx, HDD_IPA_UC_STA_ENABLE_MASK);
}

/**
 * hdd_ipa_is_pre_filter_enabled() - Is IPA pre-filter enabled?
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: true if pre-filter is enabled, otherwise false
 */
static inline bool hdd_ipa_is_pre_filter_enabled(hdd_context_t *hdd_ctx)
{
	return HDD_IPA_IS_CONFIG_ENABLED(hdd_ctx,
					 HDD_IPA_PRE_FILTER_ENABLE_MASK);
}

/**
 * hdd_ipa_is_ipv6_enabled() - Is IPA IPv6 enabled?
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: true if IPv6 is enabled, otherwise false
 */
static inline bool hdd_ipa_is_ipv6_enabled(hdd_context_t *hdd_ctx)
{
	return HDD_IPA_IS_CONFIG_ENABLED(hdd_ctx, HDD_IPA_IPV6_ENABLE_MASK);
}

/**
 * hdd_ipa_is_rm_enabled() - Is IPA resource manager enabled?
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: true if resource manager is enabled, otherwise false
 */
static inline bool hdd_ipa_is_rm_enabled(hdd_context_t *hdd_ctx)
{
	return HDD_IPA_IS_CONFIG_ENABLED(hdd_ctx, HDD_IPA_RM_ENABLE_MASK);
}

/**
 * hdd_ipa_is_rt_debugging_enabled() - Is IPA real-time debug enabled?
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: true if resource manager is enabled, otherwise false
 */
static inline bool hdd_ipa_is_rt_debugging_enabled(hdd_context_t *hdd_ctx)
{
	return HDD_IPA_IS_CONFIG_ENABLED(hdd_ctx, HDD_IPA_REAL_TIME_DEBUGGING);
}

/**
 * hdd_ipa_is_clk_scaling_enabled() - Is IPA clock scaling enabled?
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: true if clock scaling is enabled, otherwise false
 */
static inline bool hdd_ipa_is_clk_scaling_enabled(hdd_context_t *hdd_ctx)
{
	return HDD_IPA_IS_CONFIG_ENABLED(hdd_ctx,
					 HDD_IPA_CLK_SCALING_ENABLE_MASK |
					 HDD_IPA_RM_ENABLE_MASK);
}

/**
 * hdd_ipa_uc_rt_debug_host_fill - fill rt debug buffer
 * @ctext: pointer to hdd context.
 *
 * If rt debug enabled, periodically called, and fill debug buffer
 *
 * Return: none
 */
static void hdd_ipa_uc_rt_debug_host_fill(void *ctext)
{
	hdd_context_t *hdd_ctx = (hdd_context_t *)ctext;
	struct hdd_ipa_priv *hdd_ipa;
	struct uc_rt_debug_info *dump_info = NULL;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	if (!hdd_ctx->hdd_ipa || !hdd_ipa_uc_is_enabled(hdd_ctx)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			"%s: IPA UC is not enabled", __func__);
		return;
	}

	hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;

	qdf_mutex_acquire(&hdd_ipa->rt_debug_lock);
	dump_info = &hdd_ipa->rt_bug_buffer[
		hdd_ipa->rt_buf_fill_index % HDD_IPA_UC_RT_DEBUG_BUF_COUNT];

	dump_info->time = qdf_mc_timer_get_system_time();
	dump_info->ipa_excep_count = hdd_ipa->stats.num_rx_excep;
	dump_info->rx_drop_count = hdd_ipa->ipa_rx_internel_drop_count;
	dump_info->net_sent_count = hdd_ipa->ipa_rx_net_send_count;
	dump_info->rx_discard_count = hdd_ipa->ipa_rx_discard;
	dump_info->tx_mcbc_count = hdd_ipa->stats.num_tx_bcmc;
	dump_info->tx_fwd_count = hdd_ipa->ipa_tx_forward;
	dump_info->rx_destructor_call = hdd_ipa->ipa_rx_destructor_count;
	hdd_ipa->rt_buf_fill_index++;
	qdf_mutex_release(&hdd_ipa->rt_debug_lock);

	qdf_mc_timer_start(&hdd_ipa->rt_debug_fill_timer,
		HDD_IPA_UC_RT_DEBUG_FILL_INTERVAL);
}

/**
 * hdd_ipa_uc_rt_debug_host_dump - dump rt debug buffer
 * @hdd_ctx: pointer to hdd context.
 *
 * If rt debug enabled, dump debug buffer contents based on requirement
 *
 * Return: none
 */
void hdd_ipa_uc_rt_debug_host_dump(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa;
	unsigned int dump_count;
	unsigned int dump_index;
	struct uc_rt_debug_info *dump_info = NULL;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	hdd_ipa = hdd_ctx->hdd_ipa;
	if (!hdd_ipa || !hdd_ipa_uc_is_enabled(hdd_ctx)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			"%s: IPA UC is not enabled", __func__);
		return;
	}

	HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
		"========= WLAN-IPA DEBUG BUF DUMP ==========\n");
	HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
		"     TM     :   EXEP   :   DROP   :   NETS   :   MCBC   :   TXFD   :   DSTR   :   DSCD\n");

	qdf_mutex_acquire(&hdd_ipa->rt_debug_lock);
	for (dump_count = 0;
		dump_count < HDD_IPA_UC_RT_DEBUG_BUF_COUNT;
		dump_count++) {
		dump_index = (hdd_ipa->rt_buf_fill_index + dump_count) %
			HDD_IPA_UC_RT_DEBUG_BUF_COUNT;
		dump_info = &hdd_ipa->rt_bug_buffer[dump_index];
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"%12lu:%10llu:%10llu:%10llu:%10llu:%10llu:%10llu:%10llu\n",
			dump_info->time, dump_info->ipa_excep_count,
			dump_info->rx_drop_count, dump_info->net_sent_count,
			dump_info->tx_mcbc_count, dump_info->tx_fwd_count,
			dump_info->rx_destructor_call,
			dump_info->rx_discard_count);
	}
	qdf_mutex_release(&hdd_ipa->rt_debug_lock);
	HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
		"======= WLAN-IPA DEBUG BUF DUMP END ========\n");
}

/**
 * hdd_ipa_uc_rt_debug_handler - periodic memory health monitor handler
 * @ctext: pointer to hdd context.
 *
 * periodically called by timer expire
 * will try to alloc dummy memory and detect out of memory condition
 * if out of memory detected, dump wlan-ipa stats
 *
 * Return: none
 */
static void hdd_ipa_uc_rt_debug_handler(void *ctext)
{
	hdd_context_t *hdd_ctx = (hdd_context_t *)ctext;
	struct hdd_ipa_priv *hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;
	void *dummy_ptr = NULL;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	if (!hdd_ipa_is_rt_debugging_enabled(hdd_ctx)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			"%s: IPA RT debug is not enabled", __func__);
		return;
	}

	/* Allocate dummy buffer periodically and free immediately. this will
	 * proactively detect OOM and if allocation fails dump ipa stats
	 */
	dummy_ptr = kmalloc(HDD_IPA_UC_DEBUG_DUMMY_MEM_SIZE,
		GFP_KERNEL | GFP_ATOMIC);
	if (!dummy_ptr) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_FATAL,
			"%s: Dummy alloc fail", __func__);
		hdd_ipa_uc_rt_debug_host_dump(hdd_ctx);
		hdd_ipa_uc_stat_request(
			hdd_get_adapter(hdd_ctx, QDF_SAP_MODE), 1);
	} else {
		kfree(dummy_ptr);
	}

	qdf_mc_timer_start(&hdd_ipa->rt_debug_timer,
		HDD_IPA_UC_RT_DEBUG_PERIOD);
}

/**
 * hdd_ipa_uc_rt_debug_destructor - called by data packet free
 * @skb: packet pinter
 *
 * when free data packet, will be invoked by wlan client and will increase
 * free counter
 *
 * Return: none
 */
void hdd_ipa_uc_rt_debug_destructor(struct sk_buff *skb)
{
	if (!ghdd_ipa) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"%s: invalid hdd context", __func__);
		return;
	}

	ghdd_ipa->ipa_rx_destructor_count++;
}

/**
 * hdd_ipa_uc_rt_debug_deinit - remove resources to handle rt debugging
 * @hdd_ctx: hdd main context
 *
 * free all rt debugging resources
 *
 * Return: none
 */
static void hdd_ipa_uc_rt_debug_deinit(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;

	if (QDF_TIMER_STATE_STOPPED !=
		qdf_mc_timer_get_current_state(&hdd_ipa->rt_debug_fill_timer)) {
		qdf_mc_timer_stop(&hdd_ipa->rt_debug_fill_timer);
	}
	qdf_mc_timer_destroy(&hdd_ipa->rt_debug_fill_timer);
	qdf_mutex_destroy(&hdd_ipa->rt_debug_lock);

	if (!hdd_ipa_is_rt_debugging_enabled(hdd_ctx)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			"%s: IPA RT debug is not enabled", __func__);
		return;
	}

	if (QDF_TIMER_STATE_STOPPED !=
		qdf_mc_timer_get_current_state(&hdd_ipa->rt_debug_timer)) {
		qdf_mc_timer_stop(&hdd_ipa->rt_debug_timer);
	}
	qdf_mc_timer_destroy(&hdd_ipa->rt_debug_timer);
}

/**
 * hdd_ipa_uc_rt_debug_init - intialize resources to handle rt debugging
 * @hdd_ctx: hdd main context
 *
 * alloc and initialize all rt debugging resources
 *
 * Return: none
 */
static void hdd_ipa_uc_rt_debug_init(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;

	qdf_mutex_create(&hdd_ipa->rt_debug_lock);
	qdf_mc_timer_init(&hdd_ipa->rt_debug_fill_timer, QDF_TIMER_TYPE_SW,
		hdd_ipa_uc_rt_debug_host_fill, (void *)hdd_ctx);
	hdd_ipa->rt_buf_fill_index = 0;
	qdf_mem_zero(hdd_ipa->rt_bug_buffer,
		sizeof(struct uc_rt_debug_info) *
		HDD_IPA_UC_RT_DEBUG_BUF_COUNT);
	hdd_ipa->ipa_tx_forward = 0;
	hdd_ipa->ipa_rx_discard = 0;
	hdd_ipa->ipa_rx_net_send_count = 0;
	hdd_ipa->ipa_rx_internel_drop_count = 0;
	hdd_ipa->ipa_rx_destructor_count = 0;

	qdf_mc_timer_start(&hdd_ipa->rt_debug_fill_timer,
		HDD_IPA_UC_RT_DEBUG_FILL_INTERVAL);

	/* Reatime debug enable on feature enable */
	if (!hdd_ipa_is_rt_debugging_enabled(hdd_ctx)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			"%s: IPA RT debug is not enabled", __func__);
		return;
	}
	qdf_mc_timer_init(&hdd_ipa->rt_debug_timer, QDF_TIMER_TYPE_SW,
		hdd_ipa_uc_rt_debug_handler, (void *)hdd_ctx);
	qdf_mc_timer_start(&hdd_ipa->rt_debug_timer,
		HDD_IPA_UC_RT_DEBUG_PERIOD);

}

/**
 * hdd_ipa_uc_stat_query() - Query the IPA stats
 * @hdd_ctx: Global HDD context
 * @ipa_tx_diff: tx packet count diff from previous
 * 		 tx packet count
 * @ipa_rx_diff: rx packet count diff from previous
 * 		 rx packet count
 *
 * Return: true if IPA is enabled, false otherwise
 */
void hdd_ipa_uc_stat_query(hdd_context_t *pHddCtx,
	uint32_t *ipa_tx_diff, uint32_t *ipa_rx_diff)
{
	struct hdd_ipa_priv *hdd_ipa;

	hdd_ipa = (struct hdd_ipa_priv *)pHddCtx->hdd_ipa;
	*ipa_tx_diff = 0;
	*ipa_rx_diff = 0;

	if (!hdd_ipa_is_enabled(pHddCtx) ||
		!(hdd_ipa_uc_is_enabled(pHddCtx))) {
		return;
	}

	qdf_mutex_acquire(&hdd_ipa->ipa_lock);
	if ((HDD_IPA_UC_NUM_WDI_PIPE == hdd_ipa->activated_fw_pipe) &&
		(false == hdd_ipa->resource_loading)) {
		*ipa_tx_diff = hdd_ipa->ipa_tx_packets_diff;
		*ipa_rx_diff = hdd_ipa->ipa_rx_packets_diff;
		HDD_IPA_LOG(LOG1, "STAT Query TX DIFF %d, RX DIFF %d",
			    *ipa_tx_diff, *ipa_rx_diff);
	}
	qdf_mutex_release(&hdd_ipa->ipa_lock);
	return;
}

/**
 * hdd_ipa_uc_stat_request() - Get IPA stats from IPA.
 * @adapter: network adapter
 * @reason: STAT REQ Reason
 *
 * Return: None
 */
void hdd_ipa_uc_stat_request(hdd_adapter_t *adapter, uint8_t reason)
{
	hdd_context_t *pHddCtx;
	struct hdd_ipa_priv *hdd_ipa;

	if (!adapter) {
		return;
	}

	pHddCtx = (hdd_context_t *)adapter->pHddCtx;
	hdd_ipa = (struct hdd_ipa_priv *)pHddCtx->hdd_ipa;
	if (!hdd_ipa_is_enabled(pHddCtx) ||
		!(hdd_ipa_uc_is_enabled(pHddCtx))) {
		return;
	}

	HDD_IPA_LOG(LOG1, "STAT REQ Reason %d", reason);
	qdf_mutex_acquire(&hdd_ipa->ipa_lock);
	if ((HDD_IPA_UC_NUM_WDI_PIPE == hdd_ipa->activated_fw_pipe) &&
		(false == hdd_ipa->resource_loading)) {
		hdd_ipa->stat_req_reason = reason;
		wma_cli_set_command(
			(int)adapter->sessionId,
			(int)WMA_VDEV_TXRX_GET_IPA_UC_FW_STATS_CMDID,
			0, VDEV_CMD);
	}
	qdf_mutex_release(&hdd_ipa->ipa_lock);
}

/**
 * hdd_ipa_uc_find_add_assoc_sta() - Find associated station
 * @hdd_ipa: Global HDD IPA context
 * @sta_add: Should station be added
 * @sta_id: ID of the station being queried
 *
 * Return: true if the station was found
 */
static bool hdd_ipa_uc_find_add_assoc_sta(struct hdd_ipa_priv *hdd_ipa,
					  bool sta_add, uint8_t sta_id)
{
	bool sta_found = false;
	uint8_t idx;
	for (idx = 0; idx < WLAN_MAX_STA_COUNT; idx++) {
		if ((hdd_ipa->assoc_stas_map[idx].is_reserved) &&
		    (hdd_ipa->assoc_stas_map[idx].sta_id == sta_id)) {
			sta_found = true;
			break;
		}
	}
	if (sta_add && sta_found) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: STA ID %d already exist, cannot add",
			    __func__, sta_id);
		return sta_found;
	}
	if (sta_add) {
		for (idx = 0; idx < WLAN_MAX_STA_COUNT; idx++) {
			if (!hdd_ipa->assoc_stas_map[idx].is_reserved) {
				hdd_ipa->assoc_stas_map[idx].is_reserved = true;
				hdd_ipa->assoc_stas_map[idx].sta_id = sta_id;
				return sta_found;
			}
		}
	}
	if (!sta_add && !sta_found) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: STA ID %d does not exist, cannot delete",
			    __func__, sta_id);
		return sta_found;
	}
	if (!sta_add) {
		for (idx = 0; idx < WLAN_MAX_STA_COUNT; idx++) {
			if ((hdd_ipa->assoc_stas_map[idx].is_reserved) &&
			    (hdd_ipa->assoc_stas_map[idx].sta_id == sta_id)) {
				hdd_ipa->assoc_stas_map[idx].is_reserved =
					false;
				hdd_ipa->assoc_stas_map[idx].sta_id = 0xFF;
				return sta_found;
			}
		}
	}
	return sta_found;
}

/**
 * hdd_ipa_uc_enable_pipes() - Enable IPA uC pipes
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: 0 on success, negative errno if error
 */
static int hdd_ipa_uc_enable_pipes(struct hdd_ipa_priv *hdd_ipa)
{
	int result;
	p_cds_contextType cds_ctx = hdd_ipa->hdd_ctx->pcds_context;

	/* ACTIVATE TX PIPE */
	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			"%s: Enable TX PIPE(tx_pipe_handle=%d)",
			__func__, hdd_ipa->tx_pipe_handle);
	result = ipa_enable_wdi_pipe(hdd_ipa->tx_pipe_handle);
	if (result) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: Enable TX PIPE fail, code %d",
			    __func__, result);
		return result;
	}
	result = ipa_resume_wdi_pipe(hdd_ipa->tx_pipe_handle);
	if (result) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: Resume TX PIPE fail, code %d",
			    __func__, result);
		return result;
	}
	ol_txrx_ipa_uc_set_active(cds_ctx->pdev_txrx_ctx, true, true);

	/* ACTIVATE RX PIPE */
	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			"%s: Enable RX PIPE(rx_pipe_handle=%d)",
			__func__, hdd_ipa->rx_pipe_handle);
	result = ipa_enable_wdi_pipe(hdd_ipa->rx_pipe_handle);
	if (result) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: Enable RX PIPE fail, code %d",
			    __func__, result);
		return result;
	}
	result = ipa_resume_wdi_pipe(hdd_ipa->rx_pipe_handle);
	if (result) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: Resume RX PIPE fail, code %d",
			    __func__, result);
		return result;
	}
	ol_txrx_ipa_uc_set_active(cds_ctx->pdev_txrx_ctx, true, false);
	hdd_ipa->ipa_pipes_down = false;
	return 0;
}

/**
 * hdd_ipa_uc_disable_pipes() - Disable IPA uC pipes
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: 0 on success, negative errno if error
 */
static int hdd_ipa_uc_disable_pipes(struct hdd_ipa_priv *hdd_ipa)
{
	int result;

	hdd_ipa->ipa_pipes_down = true;

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "%s: Disable RX PIPE", __func__);
	result = ipa_suspend_wdi_pipe(hdd_ipa->rx_pipe_handle);
	if (result) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: Suspend RX PIPE fail, code %d",
			    __func__, result);
		return result;
	}
	result = ipa_disable_wdi_pipe(hdd_ipa->rx_pipe_handle);
	if (result) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: Disable RX PIPE fail, code %d",
			    __func__, result);
		return result;
	}

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "%s: Disable TX PIPE", __func__);
	result = ipa_suspend_wdi_pipe(hdd_ipa->tx_pipe_handle);
	if (result) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: Suspend TX PIPE fail, code %d",
			    __func__, result);
		return result;
	}
	result = ipa_disable_wdi_pipe(hdd_ipa->tx_pipe_handle);
	if (result) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: Disable TX PIPE fail, code %d",
			    __func__, result);
		return result;
	}

	return 0;
}

/**
 * hdd_ipa_uc_handle_first_con() - Handle first uC IPA connection
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: 0 on success, negative errno if error
 */
static int hdd_ipa_uc_handle_first_con(struct hdd_ipa_priv *hdd_ipa)
{
	hdd_ipa->activated_fw_pipe = 0;
	hdd_ipa->resource_loading = true;

	/* If RM feature enabled
	 * Request PROD Resource first
	 * PROD resource may return sync or async manners */
	if (hdd_ipa_is_rm_enabled(hdd_ipa->hdd_ctx)) {
		if (!ipa_rm_request_resource(IPA_RM_RESOURCE_WLAN_PROD)) {
			/* RM PROD request sync return
			 * enable pipe immediately
			 */
			if (hdd_ipa_uc_enable_pipes(hdd_ipa)) {
				HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
					"%s: IPA WDI Pipe activation failed",
					__func__);
				hdd_ipa->resource_loading = false;
				return -EBUSY;
			}
		}
	} else {
		/* RM Disabled
		 * Just enabled all the PIPEs
		 */
		if (hdd_ipa_uc_enable_pipes(hdd_ipa)) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "%s: IPA WDI Pipe activation failed",
				    __func__);
			hdd_ipa->resource_loading = false;
			return -EBUSY;
		}
		hdd_ipa->resource_loading = false;
	}

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			"%s: IPA WDI Pipes activated successfully", __func__);
	return 0;
}

/**
 * hdd_ipa_uc_handle_last_discon() - Handle last uC IPA disconnection
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: None
 */
static void hdd_ipa_uc_handle_last_discon(struct hdd_ipa_priv *hdd_ipa)
{
	p_cds_contextType cds_ctx = hdd_ipa->hdd_ctx->pcds_context;

	hdd_ipa->resource_unloading = true;
	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "%s: Disable FW RX PIPE", __func__);
	ol_txrx_ipa_uc_set_active(cds_ctx->pdev_txrx_ctx, false, false);
	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "%s: Disable FW TX PIPE", __func__);
	ol_txrx_ipa_uc_set_active(cds_ctx->pdev_txrx_ctx, false, true);
}

/**
 * hdd_ipa_uc_rm_notify_handler() - IPA uC resource notification handler
 * @context: User context registered with TL (the IPA Global context is
 *	registered
 * @rxpkt: Packet containing the notification
 * @staid: ID of the station associated with the packet
 *
 * Return: None
 */
static void
hdd_ipa_uc_rm_notify_handler(void *context, enum ipa_rm_event event)
{
	struct hdd_ipa_priv *hdd_ipa = context;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/*
	 * When SSR is going on or driver is unloading, just return.
	 */
	status = wlan_hdd_validate_context(hdd_ipa->hdd_ctx);
	if (status)
		return;

	if (!hdd_ipa_is_rm_enabled(hdd_ipa->hdd_ctx))
		return;

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "%s, event code %d",
		    __func__, event);

	switch (event) {
	case IPA_RM_RESOURCE_GRANTED:
		/* Differed RM Granted */
		hdd_ipa_uc_enable_pipes(hdd_ipa);
		qdf_mutex_acquire(&hdd_ipa->ipa_lock);
		if ((false == hdd_ipa->resource_unloading) &&
			(!hdd_ipa->activated_fw_pipe)) {
			hdd_ipa_uc_enable_pipes(hdd_ipa);
		}
		qdf_mutex_release(&hdd_ipa->ipa_lock);
		break;

	case IPA_RM_RESOURCE_RELEASED:
		/* Differed RM Released */
		hdd_ipa->resource_unloading = false;
		break;

	default:
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s, invalid event code %d", __func__, event);
		break;
	}
}

/**
 * hdd_ipa_uc_rm_notify_defer() - Defer IPA uC notification
 * @hdd_ipa: Global HDD IPA context
 * @event: IPA resource manager event to be deferred
 *
 * This function is called when a resource manager event is received
 * from firmware in interrupt context.  This function will defer the
 * handling to the OL RX thread
 *
 * Return: None
 */
static void hdd_ipa_uc_rm_notify_defer(struct work_struct *work)
{
	enum ipa_rm_event event;
	struct uc_rm_work_struct *uc_rm_work = container_of(work,
			struct uc_rm_work_struct, work);
	struct hdd_ipa_priv *hdd_ipa = container_of(uc_rm_work,
			struct hdd_ipa_priv, uc_rm_work);

	cds_ssr_protect(__func__);
	event = uc_rm_work->event;
	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO_HIGH,
		"%s, posted event %d", __func__, event);

	hdd_ipa_uc_rm_notify_handler(hdd_ipa, event);
	cds_ssr_unprotect(__func__);

	return;
}

/**
 * hdd_ipa_uc_op_cb() - IPA uC operation callback
 * @op_msg: operation message received from firmware
 * @usr_ctxt: user context registered with TL (we register the HDD Global
 *	context)
 *
 * Return: None
 */
static void hdd_ipa_uc_op_cb(struct op_msg_type *op_msg, void *usr_ctxt)
{
	struct op_msg_type *msg = op_msg;
	struct ipa_uc_fw_stats *uc_fw_stat;
	struct IpaHwStatsWDIInfoData_t ipa_stat;
	struct hdd_ipa_priv *hdd_ipa;
	hdd_context_t *hdd_ctx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!op_msg || !usr_ctxt) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "%s, INVALID ARG", __func__);
		return;
	}

	if (HDD_IPA_UC_OPCODE_MAX <= msg->op_code) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s, INVALID OPCODE %d", __func__, msg->op_code);
		return;
	}

	hdd_ctx = (hdd_context_t *) usr_ctxt;

	/*
	 * When SSR is going on or driver is unloading, just return.
	 */
	status = wlan_hdd_validate_context(hdd_ctx);
	if (status) {
		qdf_mem_free(op_msg);
		return;
	}

	hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
		    "%s, OPCODE %s", __func__, op_string[msg->op_code]);

	if ((HDD_IPA_UC_OPCODE_TX_RESUME == msg->op_code) ||
	    (HDD_IPA_UC_OPCODE_RX_RESUME == msg->op_code)) {
		qdf_mutex_acquire(&hdd_ipa->ipa_lock);
		hdd_ipa->activated_fw_pipe++;
		if (HDD_IPA_UC_NUM_WDI_PIPE == hdd_ipa->activated_fw_pipe) {
			hdd_ipa->resource_loading = false;
			hdd_ipa_uc_proc_pending_event(hdd_ipa);
			if (hdd_ipa->pending_cons_req)
				ipa_rm_notify_completion(
						IPA_RM_RESOURCE_GRANTED,
						IPA_RM_RESOURCE_WLAN_CONS);
			hdd_ipa->pending_cons_req = false;
		}
		qdf_mutex_release(&hdd_ipa->ipa_lock);
	}

	if ((HDD_IPA_UC_OPCODE_TX_SUSPEND == msg->op_code) ||
	    (HDD_IPA_UC_OPCODE_RX_SUSPEND == msg->op_code)) {
		qdf_mutex_acquire(&hdd_ipa->ipa_lock);
		hdd_ipa->activated_fw_pipe--;
		if (!hdd_ipa->activated_fw_pipe) {
			hdd_ipa_uc_disable_pipes(hdd_ipa);
			if (hdd_ipa_is_rm_enabled(hdd_ipa->hdd_ctx))
				ipa_rm_release_resource(
					IPA_RM_RESOURCE_WLAN_PROD);
			/* Sync return success from IPA
			* Enable/resume all the PIPEs */
			hdd_ipa->resource_unloading = false;
			hdd_ipa_uc_proc_pending_event(hdd_ipa);
			hdd_ipa->pending_cons_req = false;
		}
		qdf_mutex_release(&hdd_ipa->ipa_lock);
	}

	if ((HDD_IPA_UC_OPCODE_STATS == msg->op_code) &&
		(HDD_IPA_UC_STAT_REASON_DEBUG == hdd_ipa->stat_req_reason)) {
		struct ol_txrx_ipa_resources *res = &hdd_ipa->ipa_resource;
		/* STATs from host */
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "==== IPA_UC WLAN_HOST CE ====\n"
			  "CE RING BASE: 0x%llx\n"
			  "CE RING SIZE: %d\n"
			  "CE REG ADDR : 0x%llx",
			  (unsigned long long)res->ce_sr_base_paddr,
			  res->ce_sr_ring_size,
			  (unsigned long long)res->ce_reg_paddr);
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "==== IPA_UC WLAN_HOST TX ====\n"
			  "COMP RING BASE: 0x%llx\n"
			  "COMP RING SIZE: %d\n"
			  "NUM ALLOC BUF: %d\n"
			  "COMP RING DBELL : 0x%llx",
			  (unsigned long long)res->tx_comp_ring_base_paddr,
			  res->tx_comp_ring_size,
			  res->tx_num_alloc_buffer,
			  (unsigned long long)hdd_ipa->tx_comp_doorbell_paddr);
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "==== IPA_UC WLAN_HOST RX ====\n"
			  "IND RING BASE: 0x%llx\n"
			  "IND RING SIZE: %d\n"
			  "IND RING DBELL : 0x%llx\n"
			  "PROC DONE IND ADDR : 0x%llx\n"
			  "NUM EXCP PKT : %llu\n"
			  "NUM TX BCMC : %llu\n"
			  "NUM TX BCMC ERR : %llu",
			  (unsigned long long)res->rx_rdy_ring_base_paddr,
			  res->rx_rdy_ring_size,
			  (unsigned long long)hdd_ipa->rx_ready_doorbell_paddr,
			  (unsigned long long)hdd_ipa->ipa_resource.
				 rx_proc_done_idx_paddr,
			  hdd_ipa->stats.num_rx_excep,
			  hdd_ipa->stats.num_tx_bcmc,
			  (unsigned long long)hdd_ipa->stats.num_tx_bcmc_err);
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "==== IPA_UC WLAN_HOST CONTROL ====\n"
			  "SAP NUM STAs: %d\n"
			  "STA CONNECTED: %d\n"
			  "TX PIPE HDL: %d\n"
			  "RX PIPE HDL : %d\n"
			  "RSC LOADING : %d\n"
			  "RSC UNLOADING : %d\n"
			  "PNDNG CNS RQT : %d",
			  hdd_ipa->sap_num_connected_sta,
			  hdd_ipa->sta_connected,
			  hdd_ipa->tx_pipe_handle,
			  hdd_ipa->rx_pipe_handle,
			  (unsigned int)hdd_ipa->resource_loading,
			  (unsigned int)hdd_ipa->resource_unloading,
			  (unsigned int)hdd_ipa->pending_cons_req);

		/* STATs from FW */
		uc_fw_stat = (struct ipa_uc_fw_stats *)
			     ((uint8_t *)op_msg + sizeof(struct op_msg_type));
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "==== IPA_UC WLAN_FW TX ====\n"
			  "COMP RING BASE: 0x%x\n"
			  "COMP RING SIZE: %d\n"
			  "COMP RING DBELL : 0x%x\n"
			  "COMP RING DBELL IND VAL : %d\n"
			  "COMP RING DBELL CACHED VAL : %d\n"
			  "COMP RING DBELL CACHED VAL : %d\n"
			  "PKTS ENQ : %d\n"
			  "PKTS COMP : %d\n"
			  "IS SUSPEND : %d\n"
			  "RSVD : 0x%x",
			  uc_fw_stat->tx_comp_ring_base,
			  uc_fw_stat->tx_comp_ring_size,
			  uc_fw_stat->tx_comp_ring_dbell_addr,
			  uc_fw_stat->tx_comp_ring_dbell_ind_val,
			  uc_fw_stat->tx_comp_ring_dbell_cached_val,
			  uc_fw_stat->tx_comp_ring_dbell_cached_val,
			  uc_fw_stat->tx_pkts_enqueued,
			  uc_fw_stat->tx_pkts_completed,
			  uc_fw_stat->tx_is_suspend, uc_fw_stat->tx_reserved);
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "==== IPA_UC WLAN_FW RX ====\n"
			  "IND RING BASE: 0x%x\n"
			  "IND RING SIZE: %d\n"
			  "IND RING DBELL : 0x%x\n"
			  "IND RING DBELL IND VAL : %d\n"
			  "IND RING DBELL CACHED VAL : %d\n"
			  "RDY IND ADDR : 0x%x\n"
			  "RDY IND CACHE VAL : %d\n"
			  "RFIL IND : %d\n"
			  "NUM PKT INDICAT : %d\n"
			  "BUF REFIL : %d\n"
			  "NUM DROP NO SPC : %d\n"
			  "NUM DROP NO BUF : %d\n"
			  "IS SUSPND : %d\n"
			  "RSVD : 0x%x\n",
			  uc_fw_stat->rx_ind_ring_base,
			  uc_fw_stat->rx_ind_ring_size,
			  uc_fw_stat->rx_ind_ring_dbell_addr,
			  uc_fw_stat->rx_ind_ring_dbell_ind_val,
			  uc_fw_stat->rx_ind_ring_dbell_ind_cached_val,
			  uc_fw_stat->rx_ind_ring_rdidx_addr,
			  uc_fw_stat->rx_ind_ring_rd_idx_cached_val,
			  uc_fw_stat->rx_refill_idx,
			  uc_fw_stat->rx_num_pkts_indicated,
			  uc_fw_stat->rx_buf_refilled,
			  uc_fw_stat->rx_num_ind_drop_no_space,
			  uc_fw_stat->rx_num_ind_drop_no_buf,
			  uc_fw_stat->rx_is_suspend, uc_fw_stat->rx_reserved);
		/* STATs from IPA */
		ipa_get_wdi_stats(&ipa_stat);
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "==== IPA_UC IPA TX ====\n"
			  "NUM PROCD : %d\n"
			  "CE DBELL : 0x%x\n"
			  "NUM DBELL FIRED : %d\n"
			  "COMP RNG FULL : %d\n"
			  "COMP RNG EMPT : %d\n"
			  "COMP RNG USE HGH : %d\n"
			  "COMP RNG USE LOW : %d\n"
			  "BAM FIFO FULL : %d\n"
			  "BAM FIFO EMPT : %d\n"
			  "BAM FIFO USE HGH : %d\n"
			  "BAM FIFO USE LOW : %d\n"
			  "NUM DBELL : %d\n"
			  "NUM UNEXP DBELL : %d\n"
			  "NUM BAM INT HDL : 0x%x\n"
			  "NUM BAM INT NON-RUN : 0x%x\n"
			  "NUM QMB INT HDL : 0x%x",
			  ipa_stat.tx_ch_stats.num_pkts_processed,
			  ipa_stat.tx_ch_stats.copy_engine_doorbell_value,
			  ipa_stat.tx_ch_stats.num_db_fired,
			  ipa_stat.tx_ch_stats.tx_comp_ring_stats.ringFull,
			  ipa_stat.tx_ch_stats.tx_comp_ring_stats.ringEmpty,
			  ipa_stat.tx_ch_stats.tx_comp_ring_stats.ringUsageHigh,
			  ipa_stat.tx_ch_stats.tx_comp_ring_stats.ringUsageLow,
			  ipa_stat.tx_ch_stats.bam_stats.bamFifoFull,
			  ipa_stat.tx_ch_stats.bam_stats.bamFifoEmpty,
			  ipa_stat.tx_ch_stats.bam_stats.bamFifoUsageHigh,
			  ipa_stat.tx_ch_stats.bam_stats.bamFifoUsageLow,
			  ipa_stat.tx_ch_stats.num_db,
			  ipa_stat.tx_ch_stats.num_unexpected_db,
			  ipa_stat.tx_ch_stats.num_bam_int_handled,
			  ipa_stat.tx_ch_stats.
			  num_bam_int_in_non_runnning_state,
			  ipa_stat.tx_ch_stats.num_qmb_int_handled);

		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "==== IPA_UC IPA RX ====\n"
			  "MAX OST PKT : %d\n"
			  "NUM PKT PRCSD : %d\n"
			  "RNG RP : 0x%x\n"
			  "COMP RNG FULL : %d\n"
			  "COMP RNG EMPT : %d\n"
			  "COMP RNG USE HGH : %d\n"
			  "COMP RNG USE LOW : %d\n"
			  "BAM FIFO FULL : %d\n"
			  "BAM FIFO EMPT : %d\n"
			  "BAM FIFO USE HGH : %d\n"
			  "BAM FIFO USE LOW : %d\n"
			  "NUM DB : %d\n"
			  "NUM UNEXP DB : %d\n"
			  "NUM BAM INT HNDL : 0x%x\n",
			  ipa_stat.rx_ch_stats.max_outstanding_pkts,
			  ipa_stat.rx_ch_stats.num_pkts_processed,
			  ipa_stat.rx_ch_stats.rx_ring_rp_value,
			  ipa_stat.rx_ch_stats.rx_ind_ring_stats.ringFull,
			  ipa_stat.rx_ch_stats.rx_ind_ring_stats.ringEmpty,
			  ipa_stat.rx_ch_stats.rx_ind_ring_stats.ringUsageHigh,
			  ipa_stat.rx_ch_stats.rx_ind_ring_stats.ringUsageLow,
			  ipa_stat.rx_ch_stats.bam_stats.bamFifoFull,
			  ipa_stat.rx_ch_stats.bam_stats.bamFifoEmpty,
			  ipa_stat.rx_ch_stats.bam_stats.bamFifoUsageHigh,
			  ipa_stat.rx_ch_stats.bam_stats.bamFifoUsageLow,
			  ipa_stat.rx_ch_stats.num_db,
			  ipa_stat.rx_ch_stats.num_unexpected_db,
			  ipa_stat.rx_ch_stats.num_bam_int_handled);
	} else if ((HDD_IPA_UC_OPCODE_STATS == msg->op_code) &&
		(HDD_IPA_UC_STAT_REASON_BW_CAL == hdd_ipa->stat_req_reason)) {
		/* STATs from FW */
		uc_fw_stat = (struct ipa_uc_fw_stats *)
			((uint8_t *)op_msg + sizeof(struct op_msg_type));
		qdf_mutex_acquire(&hdd_ipa->ipa_lock);
		hdd_ipa->ipa_tx_packets_diff = HDD_BW_GET_DIFF(
			uc_fw_stat->tx_pkts_completed,
			hdd_ipa->ipa_p_tx_packets);
		hdd_ipa->ipa_rx_packets_diff = HDD_BW_GET_DIFF(
			(uc_fw_stat->rx_num_ind_drop_no_space +
			uc_fw_stat->rx_num_ind_drop_no_buf +
			uc_fw_stat->rx_num_pkts_indicated),
			hdd_ipa->ipa_p_rx_packets);

		hdd_ipa->ipa_p_tx_packets = uc_fw_stat->tx_pkts_completed;
		hdd_ipa->ipa_p_rx_packets =
			(uc_fw_stat->rx_num_ind_drop_no_space +
			uc_fw_stat->rx_num_ind_drop_no_buf +
			uc_fw_stat->rx_num_pkts_indicated);
		qdf_mutex_release(&hdd_ipa->ipa_lock);
	} else {
		HDD_IPA_LOG(LOGE, "INVALID REASON %d",
			    hdd_ipa->stat_req_reason);
	}
	qdf_mem_free(op_msg);
}


/**
 * hdd_ipa_uc_offload_enable_disable() - wdi enable/disable notify to fw
 * @adapter: device adapter instance
 * @offload_type: MCC or SCC
 * @enable: TX offload enable or disable
 *
 * Return: none
 */
static void hdd_ipa_uc_offload_enable_disable(hdd_adapter_t *adapter,
			uint32_t offload_type, uint32_t enable)
{
	struct sir_ipa_offload_enable_disable ipa_offload_enable_disable;

	/* Lower layer may send multiple START_BSS_EVENT in DFS mode or during
	 * channel change indication. Since these indications are sent by lower
	 * layer as SAP updates and IPA doesn't have to do anything for these
	 * updates so ignoring!
	 */
	if (QDF_SAP_MODE == adapter->device_mode && adapter->ipa_context)
		return;

	/* Lower layer may send multiple START_BSS_EVENT in DFS mode or during
	 * channel change indication. Since these indications are sent by lower
	 * layer as SAP updates and IPA doesn't have to do anything for these
	 * updates so ignoring!
	*/
	if (adapter->ipa_context)
		return;

	qdf_mem_zero(&ipa_offload_enable_disable,
		sizeof(ipa_offload_enable_disable));
	ipa_offload_enable_disable.offload_type = offload_type;
	ipa_offload_enable_disable.vdev_id = adapter->sessionId;
	ipa_offload_enable_disable.enable = enable;

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
		"%s: offload_type=%d, vdev_id=%d, enable=%d", __func__,
		ipa_offload_enable_disable.offload_type,
		ipa_offload_enable_disable.vdev_id,
		ipa_offload_enable_disable.enable);

	if (QDF_STATUS_SUCCESS !=
		sme_ipa_offload_enable_disable(WLAN_HDD_GET_HAL_CTX(adapter),
			adapter->sessionId, &ipa_offload_enable_disable)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"%s: Failure to enable IPA offload \
			(offload_type=%d, vdev_id=%d, enable=%d)", __func__,
			ipa_offload_enable_disable.offload_type,
			ipa_offload_enable_disable.vdev_id,
			ipa_offload_enable_disable.enable);
	}
}

/**
 * hdd_ipa_uc_fw_op_event_handler - IPA uC FW OPvent handler
 * @work: uC OP work
 *
 * Return: None
 */
static void hdd_ipa_uc_fw_op_event_handler(struct work_struct *work)
{
	struct op_msg_type *msg;
	struct uc_op_work_struct *uc_op_work = container_of(work,
			struct uc_op_work_struct, work);
	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;

	cds_ssr_protect(__func__);

	msg = uc_op_work->msg;
	uc_op_work->msg = NULL;
	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO_HIGH,
			"%s, posted msg %d", __func__, msg->op_code);

	hdd_ipa_uc_op_cb(msg, hdd_ipa->hdd_ctx);

	cds_ssr_unprotect(__func__);

	return;
}

/**
 * hdd_ipa_uc_op_event_handler() - Adapter lookup
 * hdd_ipa_uc_fw_op_event_handler - IPA uC FW OPvent handler
 * @op_msg: operation message received from firmware
 * @hdd_ctx: Global HDD context
 *
 * Return: None
 */
static void hdd_ipa_uc_op_event_handler(uint8_t *op_msg, void *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa;
	struct op_msg_type *msg;
	struct uc_op_work_struct *uc_op_work;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wlan_hdd_validate_context(hdd_ctx);
	if (status)
		goto end;

	msg = (struct op_msg_type *)op_msg;
	hdd_ipa = ((hdd_context_t *)hdd_ctx)->hdd_ipa;

	if (unlikely(!hdd_ipa))
		goto end;

	if (HDD_IPA_UC_OPCODE_MAX <= msg->op_code) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "%s: Invalid OP Code (%d)",
				__func__, msg->op_code);
		goto end;
	}

	uc_op_work = &hdd_ipa->uc_op_work[msg->op_code];
	if (uc_op_work->msg)
		/* When the same uC OPCODE is already pended, just return */
		goto end;

	uc_op_work->msg = msg;
	schedule_work(&uc_op_work->work);
	return;

end:
	qdf_mem_free(op_msg);
}

/**
 * hdd_ipa_init_uc_op_work - init ipa uc op work
 * @work: struct work_struct
 * @work_handler: work_handler
 *
 * Return: none
 */
static void hdd_ipa_init_uc_op_work(struct work_struct *work,
					work_func_t work_handler)
{
	INIT_WORK(work, work_handler);
}


/**
 * hdd_ipa_uc_ol_init() - Initialize IPA uC offload
 * @hdd_ctx: Global HDD context
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS hdd_ipa_uc_ol_init(hdd_context_t *hdd_ctx)
{
	struct ipa_wdi_in_params pipe_in;
	struct ipa_wdi_out_params pipe_out;
	struct hdd_ipa_priv *ipa_ctxt = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;
	p_cds_contextType cds_ctx = hdd_ctx->pcds_context;
	uint8_t i;

	qdf_mem_zero(&pipe_in, sizeof(struct ipa_wdi_in_params));
	qdf_mem_zero(&pipe_out, sizeof(struct ipa_wdi_out_params));

	qdf_list_create(&ipa_ctxt->pending_event, 1000);
	qdf_mutex_create(&ipa_ctxt->event_lock);
	qdf_mutex_create(&ipa_ctxt->ipa_lock);

	/* TX PIPE */
	pipe_in.sys.ipa_ep_cfg.nat.nat_en = IPA_BYPASS_NAT;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_len = HDD_IPA_UC_WLAN_TX_HDR_LEN;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid = 1;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_ofst_pkt_size = 0;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_additional_const_len =
		HDD_IPA_UC_WLAN_8023_HDR_SIZE;
	pipe_in.sys.ipa_ep_cfg.mode.mode = IPA_BASIC;
	pipe_in.sys.client = IPA_CLIENT_WLAN1_CONS;
	pipe_in.sys.desc_fifo_sz = hdd_ctx->config->IpaDescSize;
	pipe_in.sys.priv = hdd_ctx->hdd_ipa;
	pipe_in.sys.ipa_ep_cfg.hdr_ext.hdr_little_endian = true;
	pipe_in.sys.notify = hdd_ipa_i2w_cb;
	if (!hdd_ipa_is_rm_enabled(hdd_ctx)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			    "%s: IPA RM DISABLED, IPA AWAKE", __func__);
		pipe_in.sys.keep_ipa_awake = true;
	}

	pipe_in.u.dl.comp_ring_base_pa =
		 ipa_ctxt->ipa_resource.tx_comp_ring_base_paddr;
	pipe_in.u.dl.comp_ring_size =
		 ipa_ctxt->ipa_resource.tx_comp_ring_size *
			 sizeof(qdf_dma_addr_t);
	pipe_in.u.dl.ce_ring_base_pa =
		 ipa_ctxt->ipa_resource.ce_sr_base_paddr;
	pipe_in.u.dl.ce_door_bell_pa = ipa_ctxt->ipa_resource.ce_reg_paddr;
	pipe_in.u.dl.ce_ring_size =
		 ipa_ctxt->ipa_resource.ce_sr_ring_size;
	pipe_in.u.dl.num_tx_buffers =
		 ipa_ctxt->ipa_resource.tx_num_alloc_buffer;

	/* Connect WDI IPA PIPE */
	ipa_connect_wdi_pipe(&pipe_in, &pipe_out);
	/* Micro Controller Doorbell register */
	ipa_ctxt->tx_comp_doorbell_paddr = pipe_out.uc_door_bell_pa;
	/* WLAN TX PIPE Handle */
	ipa_ctxt->tx_pipe_handle = pipe_out.clnt_hdl;
	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO_HIGH,
		    "TX : CRBPA 0x%x, CRS %d, CERBPA 0x%x, CEDPA 0x%x,"
		    " CERZ %d, NB %d, CDBPAD 0x%x",
		    (unsigned int)pipe_in.u.dl.comp_ring_base_pa,
		    pipe_in.u.dl.comp_ring_size,
		    (unsigned int)pipe_in.u.dl.ce_ring_base_pa,
		    (unsigned int)pipe_in.u.dl.ce_door_bell_pa,
		    pipe_in.u.dl.ce_ring_size,
		    pipe_in.u.dl.num_tx_buffers,
		    (unsigned int)ipa_ctxt->tx_comp_doorbell_paddr);

	/* RX PIPE */
	pipe_in.sys.ipa_ep_cfg.nat.nat_en = IPA_BYPASS_NAT;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_len = HDD_IPA_UC_WLAN_RX_HDR_LEN;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_ofst_metadata_valid = 0;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_metadata_reg_valid = 1;
	pipe_in.sys.ipa_ep_cfg.mode.mode = IPA_BASIC;
	pipe_in.sys.client = IPA_CLIENT_WLAN1_PROD;
	pipe_in.sys.desc_fifo_sz = hdd_ctx->config->IpaDescSize +
				   sizeof(struct sps_iovec);
	pipe_in.sys.notify = hdd_ipa_w2i_cb;
	if (!hdd_ipa_is_rm_enabled(hdd_ctx)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: IPA RM DISABLED, IPA AWAKE", __func__);
		pipe_in.sys.keep_ipa_awake = true;
	}

	pipe_in.u.ul.rdy_ring_base_pa =
		 ipa_ctxt->ipa_resource.rx_rdy_ring_base_paddr;
	pipe_in.u.ul.rdy_ring_size =
		 ipa_ctxt->ipa_resource.rx_rdy_ring_size;
	pipe_in.u.ul.rdy_ring_rp_pa =
		 ipa_ctxt->ipa_resource.rx_proc_done_idx_paddr;
	HDD_IPA_WDI2_SET(pipe_in, ipa_ctxt);
	ipa_connect_wdi_pipe(&pipe_in, &pipe_out);
	ipa_ctxt->rx_ready_doorbell_paddr = pipe_out.uc_door_bell_pa;
	ipa_ctxt->rx_pipe_handle = pipe_out.clnt_hdl;
	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO_HIGH,
		    "RX : RRBPA 0x%x, RRS %d, PDIPA 0x%x, RDY_DB_PAD 0x%x",
		    (unsigned int)pipe_in.u.ul.rdy_ring_base_pa,
		    pipe_in.u.ul.rdy_ring_size,
		    (unsigned int)pipe_in.u.ul.rdy_ring_rp_pa,
		    (unsigned int)ipa_ctxt->rx_ready_doorbell_paddr);

	ol_txrx_ipa_uc_set_doorbell_paddr(cds_ctx->pdev_txrx_ctx,
				     ipa_ctxt->tx_comp_doorbell_paddr,
				     ipa_ctxt->rx_ready_doorbell_paddr);

	ol_txrx_ipa_uc_register_op_cb(cds_ctx->pdev_txrx_ctx,
				  hdd_ipa_uc_op_event_handler, (void *)hdd_ctx);

	for (i = 0; i < HDD_IPA_UC_OPCODE_MAX; i++) {
		hdd_ipa_init_uc_op_work(&ipa_ctxt->uc_op_work[i].work,
			hdd_ipa_uc_fw_op_event_handler);
		ipa_ctxt->uc_op_work[i].msg = NULL;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_ipa_uc_force_pipe_shutdown() - Force shutdown IPA pipe
 * @hdd_ctx: hdd main context
 *
 * Force shutdown IPA pipe
 * Independent of FW pipe status, IPA pipe shutdonw progress
 * in case, any STA does not leave properly, IPA HW pipe should cleaned up
 * independent from FW pipe status
 *
 * Return: NONE
 */
void hdd_ipa_uc_force_pipe_shutdown(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa;

	if (!hdd_ipa_is_enabled(hdd_ctx) || !hdd_ctx->hdd_ipa)
		return;

	hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;
	if (false == hdd_ipa->ipa_pipes_down) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				"IPA pipes are not down yet, force shutdown");
		hdd_ipa_uc_disable_pipes(hdd_ipa);
	} else {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
				"IPA pipes are down, do nothing");
	}

	return;
}

/**
 * hdd_ipa_uc_ssr_deinit() - handle ipa deinit for SSR
 *
 * Deinit basic IPA UC host side to be in sync reloaded FW during
 * SSR
 *
 * Return: 0 - Success
 */
int hdd_ipa_uc_ssr_deinit(void)
{
	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;
	int idx;
	struct hdd_ipa_iface_context *iface_context;

	if ((!hdd_ipa) || (!hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)))
		return 0;

	/* Clean up HDD IPA interfaces */
	for (idx = 0; (hdd_ipa->num_iface > 0) &&
		(idx < HDD_IPA_MAX_IFACE); idx++) {
		iface_context = &hdd_ipa->iface_context[idx];
		if (iface_context && iface_context->adapter)
			hdd_ipa_cleanup_iface(iface_context);
	}

	/* After SSR, wlan driver reloads FW again. But we need to protect
	 * IPA submodule during SSR transient state. So deinit basic IPA
	 * UC host side to be in sync with reloaded FW during SSR
	 */
	if (!hdd_ipa->ipa_pipes_down)
		hdd_ipa_uc_disable_pipes(hdd_ipa);

	qdf_mutex_acquire(&hdd_ipa->ipa_lock);
	for (idx = 0; idx < WLAN_MAX_STA_COUNT; idx++) {
		hdd_ipa->assoc_stas_map[idx].is_reserved = false;
		hdd_ipa->assoc_stas_map[idx].sta_id = 0xFF;
	}
	qdf_mutex_release(&hdd_ipa->ipa_lock);

	/* Full IPA driver cleanup not required since wlan driver is now
	 * unloaded and reloaded after SSR.
	 */
	return 0;
}

/**
 * hdd_ipa_uc_ssr_reinit() - handle ipa reinit after SSR
 *
 * Init basic IPA UC host side to be in sync with reloaded FW after
 * SSR to resume IPA UC operations
 *
 * Return: 0 - Success
 */
int hdd_ipa_uc_ssr_reinit(void)
{

	/* After SSR is complete, IPA UC can resume operation. But now wlan
	 * driver will be unloaded and reloaded, which takes care of IPA cleanup
	 * and initialization. This is a placeholder func if IPA has to resume
	 * operations without driver reload.
	 */
	return 0;
}

/**
 * hdd_ipa_tx_packet_ipa() - send packet to IPA
 * @hdd_ctx:    Global HDD context
 * @skb:        skb sent to IPA
 * @session_id: send packet instance session id
 *
 * Send TX packet which generated by system to IPA.
 * This routine only will be used for function verification
 *
 * Return: NULL packet sent to IPA properly
 *         NULL invalid packet drop
 *         skb packet not sent to IPA. legacy data path should handle
 */
struct sk_buff *hdd_ipa_tx_packet_ipa(hdd_context_t *hdd_ctx,
	struct sk_buff *skb, uint8_t session_id)
{
	struct ipa_header *ipa_header;
	struct frag_header *frag_header;
	struct hdd_ipa_priv *hdd_ipa = hdd_ctx->hdd_ipa;

	if (!hdd_ipa_uc_is_enabled(hdd_ctx))
		return skb;

	if (!hdd_ipa)
		return skb;

	if (HDD_IPA_UC_NUM_WDI_PIPE != hdd_ipa->activated_fw_pipe)
		return skb;

	if (skb_headroom(skb) <
		(sizeof(struct ipa_header) + sizeof(struct frag_header)))
		return skb;

	ipa_header = (struct ipa_header *) skb_push(skb,
		sizeof(struct ipa_header));
	if (!ipa_header) {
		/* No headroom, legacy */
		return skb;
	}
	memset(ipa_header, 0, sizeof(*ipa_header));
	ipa_header->vdev_id = 0;

	frag_header = (struct frag_header *) skb_push(skb,
		sizeof(struct frag_header));
	if (!frag_header) {
		/* No headroom, drop */
		kfree_skb(skb);
		return NULL;
	}
	memset(frag_header, 0, sizeof(*frag_header));
	frag_header->length = skb->len - sizeof(struct frag_header)
		- sizeof(struct ipa_header);

	ipa_tx_dp(IPA_CLIENT_WLAN1_CONS, skb, NULL);
	return NULL;
}

/**
 * hdd_ipa_wake_lock_timer_func() - Wake lock work handler
 * @work: scheduled work
 *
 * When IPA resources are released in hdd_ipa_rm_try_release() we do
 * not want to immediately release the wake lock since the system
 * would then potentially try to suspend when there is a healthy data
 * rate.  Deferred work is scheduled and this function handles the
 * work.  When this function is called, if the IPA resource is still
 * released then we release the wake lock.
 *
 * Return: None
 */
static void hdd_ipa_wake_lock_timer_func(struct work_struct *work)
{
	struct hdd_ipa_priv *hdd_ipa = container_of(to_delayed_work(work),
						    struct hdd_ipa_priv,
						    wake_lock_work);

	qdf_spin_lock_bh(&hdd_ipa->rm_lock);

	if (hdd_ipa->rm_state != HDD_IPA_RM_RELEASED)
		goto end;

	hdd_ipa->wake_lock_released = true;
	qdf_wake_lock_release(&hdd_ipa->wake_lock,
			      WIFI_POWER_EVENT_WAKELOCK_IPA);

end:
	qdf_spin_unlock_bh(&hdd_ipa->rm_lock);
}

/**
 * hdd_ipa_rm_request() - Request resource from IPA
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: 0 on success, negative errno on error
 */
static int hdd_ipa_rm_request(struct hdd_ipa_priv *hdd_ipa)
{
	int ret = 0;

	if (!hdd_ipa_is_rm_enabled(hdd_ipa->hdd_ctx))
		return 0;

	qdf_spin_lock_bh(&hdd_ipa->rm_lock);

	switch (hdd_ipa->rm_state) {
	case HDD_IPA_RM_GRANTED:
		qdf_spin_unlock_bh(&hdd_ipa->rm_lock);
		return 0;
	case HDD_IPA_RM_GRANT_PENDING:
		qdf_spin_unlock_bh(&hdd_ipa->rm_lock);
		return -EINPROGRESS;
	case HDD_IPA_RM_RELEASED:
		hdd_ipa->rm_state = HDD_IPA_RM_GRANT_PENDING;
		break;
	}

	qdf_spin_unlock_bh(&hdd_ipa->rm_lock);

	ret = ipa_rm_inactivity_timer_request_resource(
			IPA_RM_RESOURCE_WLAN_PROD);

	qdf_spin_lock_bh(&hdd_ipa->rm_lock);
	if (ret == 0) {
		hdd_ipa->rm_state = HDD_IPA_RM_GRANTED;
		hdd_ipa->stats.num_rm_grant_imm++;
	}

	cancel_delayed_work(&hdd_ipa->wake_lock_work);
	if (hdd_ipa->wake_lock_released) {
		qdf_wake_lock_acquire(&hdd_ipa->wake_lock,
				      WIFI_POWER_EVENT_WAKELOCK_IPA);
		hdd_ipa->wake_lock_released = false;
	}
	qdf_spin_unlock_bh(&hdd_ipa->rm_lock);

	return ret;
}

/**
 * hdd_ipa_rm_try_release() - Attempt to release IPA resource
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: 0 if resources released, negative errno otherwise
 */
static int hdd_ipa_rm_try_release(struct hdd_ipa_priv *hdd_ipa)
{
	int ret = 0;

	if (!hdd_ipa_is_rm_enabled(hdd_ipa->hdd_ctx))
		return 0;

	if (atomic_read(&hdd_ipa->tx_ref_cnt))
		return -EAGAIN;

	spin_lock_bh(&hdd_ipa->q_lock);
	if (!hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx) &&
		(hdd_ipa->pending_hw_desc_cnt || hdd_ipa->pend_q_cnt)) {
		spin_unlock_bh(&hdd_ipa->q_lock);
		return -EAGAIN;
	}
	spin_unlock_bh(&hdd_ipa->q_lock);

	qdf_spin_lock_bh(&hdd_ipa->pm_lock);

	if (!qdf_nbuf_is_queue_empty(&hdd_ipa->pm_queue_head)) {
		qdf_spin_unlock_bh(&hdd_ipa->pm_lock);
		return -EAGAIN;
	}
	qdf_spin_unlock_bh(&hdd_ipa->pm_lock);

	qdf_spin_lock_bh(&hdd_ipa->rm_lock);
	switch (hdd_ipa->rm_state) {
	case HDD_IPA_RM_GRANTED:
		break;
	case HDD_IPA_RM_GRANT_PENDING:
		qdf_spin_unlock_bh(&hdd_ipa->rm_lock);
		return -EINPROGRESS;
	case HDD_IPA_RM_RELEASED:
		qdf_spin_unlock_bh(&hdd_ipa->rm_lock);
		return 0;
	}

	/* IPA driver returns immediately so set the state here to avoid any
	 * race condition.
	 */
	hdd_ipa->rm_state = HDD_IPA_RM_RELEASED;
	hdd_ipa->stats.num_rm_release++;
	qdf_spin_unlock_bh(&hdd_ipa->rm_lock);

	ret =
		ipa_rm_inactivity_timer_release_resource(IPA_RM_RESOURCE_WLAN_PROD);

	qdf_spin_lock_bh(&hdd_ipa->rm_lock);
	if (unlikely(ret != 0)) {
		hdd_ipa->rm_state = HDD_IPA_RM_GRANTED;
		WARN_ON(1);
	}

	/*
	 * If wake_lock is released immediately, kernel would try to suspend
	 * immediately as well, Just avoid ping-pong between suspend-resume
	 * while there is healthy amount of data transfer going on by
	 * releasing the wake_lock after some delay.
	 */
	schedule_delayed_work(&hdd_ipa->wake_lock_work,
			      msecs_to_jiffies
				      (HDD_IPA_RX_INACTIVITY_MSEC_DELAY));

	qdf_spin_unlock_bh(&hdd_ipa->rm_lock);

	return ret;
}

/**
 * hdd_ipa_rm_notify() - IPA resource manager notifier callback
 * @user_data: user data registered with IPA
 * @event: the IPA resource manager event that occurred
 * @data: the data associated with the event
 *
 * Return: None
 */
static void hdd_ipa_rm_notify(void *user_data, enum ipa_rm_event event,
			      unsigned long data)
{
	struct hdd_ipa_priv *hdd_ipa = user_data;

	if (unlikely(!hdd_ipa))
		return;

	if (!hdd_ipa_is_rm_enabled(hdd_ipa->hdd_ctx))
		return;

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "Evt: %d", event);

	switch (event) {
	case IPA_RM_RESOURCE_GRANTED:
		if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			/* RM Notification comes with ISR context
			 * it should be serialized into work queue to avoid
			 * ISR sleep problem
			 */
			hdd_ipa->uc_rm_work.event = event;
			schedule_work(&hdd_ipa->uc_rm_work.work);
			break;
		}
		qdf_spin_lock_bh(&hdd_ipa->rm_lock);
		hdd_ipa->rm_state = HDD_IPA_RM_GRANTED;
		qdf_spin_unlock_bh(&hdd_ipa->rm_lock);
		hdd_ipa->stats.num_rm_grant++;
		break;

	case IPA_RM_RESOURCE_RELEASED:
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "RM Release");
		hdd_ipa->resource_unloading = false;
		break;

	default:
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "Unknown RM Evt: %d", event);
		break;
	}
}

/**
 * hdd_ipa_rm_cons_release() - WLAN consumer resource release handler
 *
 * Callback function registered with IPA that is called when IPA wants
 * to release the WLAN consumer resource
 *
 * Return: 0 if the request is granted, negative errno otherwise
 */
static int hdd_ipa_rm_cons_release(void)
{
	return 0;
}

/**
 * hdd_ipa_rm_cons_request() - WLAN consumer resource request handler
 *
 * Callback function registered with IPA that is called when IPA wants
 * to access the WLAN consumer resource
 *
 * Return: 0 if the request is granted, negative errno otherwise
 */
static int hdd_ipa_rm_cons_request(void)
{
	int ret = 0;

	if (ghdd_ipa->resource_loading) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_FATAL,
			    "%s: IPA resource loading in progress",
			    __func__);
		ghdd_ipa->pending_cons_req = true;
		ret = -EINPROGRESS;
	} else if (ghdd_ipa->resource_unloading) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_FATAL,
				"%s: IPA resource unloading in progress",
				__func__);
		ghdd_ipa->pending_cons_req = true;
		ret = -EPERM;
	}

	return ret;
}

/**
 * hdd_ipa_set_perf_level() - Set IPA performance level
 * @hdd_ctx: Global HDD context
 * @tx_packets: Number of packets transmitted in the last sample period
 * @rx_packets: Number of packets received in the last sample period
 *
 * Return: 0 on success, negative errno on error
 */
int hdd_ipa_set_perf_level(hdd_context_t *hdd_ctx, uint64_t tx_packets,
			   uint64_t rx_packets)
{
	uint32_t next_cons_bw, next_prod_bw;
	struct hdd_ipa_priv *hdd_ipa = hdd_ctx->hdd_ipa;
	struct ipa_rm_perf_profile profile;
	int ret;

	if ((!hdd_ipa_is_enabled(hdd_ctx)) ||
		(!hdd_ipa_is_clk_scaling_enabled(hdd_ctx)))
		return 0;

	memset(&profile, 0, sizeof(profile));

	if (tx_packets > (hdd_ctx->config->busBandwidthHighThreshold / 2))
		next_cons_bw = hdd_ctx->config->IpaHighBandwidthMbps;
	else if (tx_packets >
		 (hdd_ctx->config->busBandwidthMediumThreshold / 2))
		next_cons_bw = hdd_ctx->config->IpaMediumBandwidthMbps;
	else
		next_cons_bw = hdd_ctx->config->IpaLowBandwidthMbps;

	if (rx_packets > (hdd_ctx->config->busBandwidthHighThreshold / 2))
		next_prod_bw = hdd_ctx->config->IpaHighBandwidthMbps;
	else if (rx_packets >
		 (hdd_ctx->config->busBandwidthMediumThreshold / 2))
		next_prod_bw = hdd_ctx->config->IpaMediumBandwidthMbps;
	else
		next_prod_bw = hdd_ctx->config->IpaLowBandwidthMbps;

	HDD_IPA_LOG(LOG1,
		"CONS perf curr: %d, next: %d",
		hdd_ipa->curr_cons_bw, next_cons_bw);
	HDD_IPA_LOG(LOG1,
		"PROD perf curr: %d, next: %d",
		hdd_ipa->curr_prod_bw, next_prod_bw);

	if (hdd_ipa->curr_cons_bw != next_cons_bw) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			    "Requesting CONS perf curr: %d, next: %d",
			    hdd_ipa->curr_cons_bw, next_cons_bw);
		profile.max_supported_bandwidth_mbps = next_cons_bw;
		ret = ipa_rm_set_perf_profile(IPA_RM_RESOURCE_WLAN_CONS,
					      &profile);
		if (ret) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "RM CONS set perf profile failed: %d", ret);

			return ret;
		}
		hdd_ipa->curr_cons_bw = next_cons_bw;
		hdd_ipa->stats.num_cons_perf_req++;
	}

	if (hdd_ipa->curr_prod_bw != next_prod_bw) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			    "Requesting PROD perf curr: %d, next: %d",
			    hdd_ipa->curr_prod_bw, next_prod_bw);
		profile.max_supported_bandwidth_mbps = next_prod_bw;
		ret = ipa_rm_set_perf_profile(IPA_RM_RESOURCE_WLAN_PROD,
					      &profile);
		if (ret) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "RM PROD set perf profile failed: %d", ret);
			return ret;
		}
		hdd_ipa->curr_prod_bw = next_prod_bw;
		hdd_ipa->stats.num_prod_perf_req++;
	}

	return 0;
}

/**
 * hdd_ipa_init_uc_rm_work - init ipa uc resource manager work
 * @work: struct work_struct
 * @work_handler: work_handler
 *
 * Return: none
 */
static void hdd_ipa_init_uc_rm_work(struct work_struct *work,
					work_func_t work_handler)
{
	INIT_WORK(work, work_handler);
}

/**
 * hdd_ipa_setup_rm() - Setup IPA resource management
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: 0 on success, negative errno on error
 */
static int hdd_ipa_setup_rm(struct hdd_ipa_priv *hdd_ipa)
{
	struct ipa_rm_create_params create_params = { 0 };
	int ret;

	if (!hdd_ipa_is_rm_enabled(hdd_ipa->hdd_ctx))
		return 0;

	hdd_ipa_init_uc_rm_work(&hdd_ipa->uc_rm_work.work,
		hdd_ipa_uc_rm_notify_defer);
	memset(&create_params, 0, sizeof(create_params));
	create_params.name = IPA_RM_RESOURCE_WLAN_PROD;
	create_params.reg_params.user_data = hdd_ipa;
	create_params.reg_params.notify_cb = hdd_ipa_rm_notify;
	create_params.floor_voltage = IPA_VOLTAGE_SVS;

	ret = ipa_rm_create_resource(&create_params);
	if (ret) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "Create RM resource failed: %d", ret);
		goto setup_rm_fail;
	}

	memset(&create_params, 0, sizeof(create_params));
	create_params.name = IPA_RM_RESOURCE_WLAN_CONS;
	create_params.request_resource = hdd_ipa_rm_cons_request;
	create_params.release_resource = hdd_ipa_rm_cons_release;
	create_params.floor_voltage = IPA_VOLTAGE_SVS;

	ret = ipa_rm_create_resource(&create_params);
	if (ret) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "Create RM CONS resource failed: %d", ret);
		goto delete_prod;
	}

	ipa_rm_add_dependency(IPA_RM_RESOURCE_WLAN_PROD,
			      IPA_RM_RESOURCE_APPS_CONS);

	ret = ipa_rm_inactivity_timer_init(IPA_RM_RESOURCE_WLAN_PROD,
					   HDD_IPA_RX_INACTIVITY_MSEC_DELAY);
	if (ret) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "Timer init failed: %d",
			    ret);
		goto timer_init_failed;
	}

	/* Set the lowest bandwidth to start with */
	ret = hdd_ipa_set_perf_level(hdd_ipa->hdd_ctx, 0, 0);

	if (ret) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "Set perf level failed: %d", ret);
		goto set_perf_failed;
	}

	qdf_wake_lock_create(&hdd_ipa->wake_lock, "wlan_ipa");
	INIT_DELAYED_WORK(&hdd_ipa->wake_lock_work,
			  hdd_ipa_wake_lock_timer_func);
	qdf_spinlock_create(&hdd_ipa->rm_lock);
	hdd_ipa->rm_state = HDD_IPA_RM_RELEASED;
	hdd_ipa->wake_lock_released = true;
	atomic_set(&hdd_ipa->tx_ref_cnt, 0);

	return ret;

set_perf_failed:
	ipa_rm_inactivity_timer_destroy(IPA_RM_RESOURCE_WLAN_PROD);

timer_init_failed:
	ipa_rm_delete_resource(IPA_RM_RESOURCE_WLAN_CONS);

delete_prod:
	ipa_rm_delete_resource(IPA_RM_RESOURCE_WLAN_PROD);

setup_rm_fail:
	return ret;
}

/**
 * hdd_ipa_destroy_rm_resource() - Destroy IPA resources
 * @hdd_ipa: Global HDD IPA context
 *
 * Destroys all resources associated with the IPA resource manager
 *
 * Return: None
 */
static void hdd_ipa_destroy_rm_resource(struct hdd_ipa_priv *hdd_ipa)
{
	int ret;

	if (!hdd_ipa_is_rm_enabled(hdd_ipa->hdd_ctx))
		return;

	cancel_delayed_work_sync(&hdd_ipa->wake_lock_work);
	qdf_wake_lock_destroy(&hdd_ipa->wake_lock);

#ifdef WLAN_OPEN_SOURCE
	cancel_work_sync(&hdd_ipa->uc_rm_work.work);
#endif
	qdf_spinlock_destroy(&hdd_ipa->rm_lock);

	ipa_rm_inactivity_timer_destroy(IPA_RM_RESOURCE_WLAN_PROD);

	ret = ipa_rm_delete_resource(IPA_RM_RESOURCE_WLAN_PROD);
	if (ret)
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "RM PROD resource delete failed %d", ret);

	ret = ipa_rm_delete_resource(IPA_RM_RESOURCE_WLAN_CONS);
	if (ret)
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "RM CONS resource delete failed %d", ret);
}

/**
 * hdd_ipa_send_skb_to_network() - Send skb to kernel
 * @skb: network buffer
 * @adapter: network adapter
 *
 * Called when a network buffer is received which should not be routed
 * to the IPA module.
 *
 * Return: None
 */
static void hdd_ipa_send_skb_to_network(qdf_nbuf_t skb,
	hdd_adapter_t *adapter)
{
	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;
	unsigned int cpu_index;

	if (!adapter || adapter->magic != WLAN_HDD_ADAPTER_MAGIC) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO_LOW, "Invalid adapter: 0x%p",
			    adapter);
		HDD_IPA_INCREASE_INTERNAL_DROP_COUNT(hdd_ipa);
		qdf_nbuf_free(skb);
		return;
	}

	if (cds_is_driver_unloading()) {
		HDD_IPA_INCREASE_INTERNAL_DROP_COUNT(hdd_ipa);
		qdf_nbuf_free(skb);
		return;
	}

	skb->destructor = hdd_ipa_uc_rt_debug_destructor;
	skb->dev = adapter->dev;
	skb->protocol = eth_type_trans(skb, skb->dev);
	skb->ip_summed = CHECKSUM_NONE;

	cpu_index = wlan_hdd_get_cpu();

	++adapter->hdd_stats.hddTxRxStats.rxPackets[cpu_index];
	if (netif_rx_ni(skb) == NET_RX_SUCCESS)
		++adapter->hdd_stats.hddTxRxStats.rxDelivered[cpu_index];
	else
		++adapter->hdd_stats.hddTxRxStats.rxRefused[cpu_index];

	HDD_IPA_INCREASE_NET_SEND_COUNT(hdd_ipa);
	adapter->dev->last_rx = jiffies;
}

/**
 * hdd_ipa_w2i_cb() - WLAN to IPA callback handler
 * @priv: pointer to private data registered with IPA (we register a
 *	pointer to the global IPA context)
 * @evt: the IPA event which triggered the callback
 * @data: data associated with the event
 *
 * Return: None
 */
static void hdd_ipa_w2i_cb(void *priv, enum ipa_dp_evt_type evt,
			   unsigned long data)
{
	struct hdd_ipa_priv *hdd_ipa = NULL;
	hdd_adapter_t *adapter = NULL;
	qdf_nbuf_t skb;
	uint8_t iface_id;
	uint8_t session_id;
	struct hdd_ipa_iface_context *iface_context;
	qdf_nbuf_t copy;
	uint8_t fw_desc;
	int ret;

	hdd_ipa = (struct hdd_ipa_priv *)priv;

	switch (evt) {
	case IPA_RECEIVE:
		skb = (qdf_nbuf_t) data;
		if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			session_id = (uint8_t)skb->cb[0];
			iface_id = vdev_to_iface[session_id];
			HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO_HIGH,
				"IPA_RECEIVE: session_id=%u, iface_id=%u",
				session_id, iface_id);
		} else {
			iface_id = HDD_IPA_GET_IFACE_ID(skb->data);
		}

		if (iface_id >= HDD_IPA_MAX_IFACE) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "IPA_RECEIVE: Invalid iface_id: %u",
				    iface_id);
			HDD_IPA_DBG_DUMP(QDF_TRACE_LEVEL_INFO_HIGH,
				"w2i -- skb", skb->data, 8);
			HDD_IPA_INCREASE_INTERNAL_DROP_COUNT(hdd_ipa);
			qdf_nbuf_free(skb);
			return;
		}

		iface_context = &hdd_ipa->iface_context[iface_id];
		adapter = iface_context->adapter;

		HDD_IPA_DBG_DUMP(QDF_TRACE_LEVEL_DEBUG,
				"w2i -- skb", skb->data, 8);
		if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			hdd_ipa->stats.num_rx_excep++;
			skb_pull(skb, HDD_IPA_UC_WLAN_CLD_HDR_LEN);
		} else {
			skb_pull(skb, HDD_IPA_WLAN_CLD_HDR_LEN);
		}

		iface_context->stats.num_rx_ipa_excep++;

		/* Disable to forward Intra-BSS Rx packets when
		 * ap_isolate=1 in hostapd.conf
		 */
		if (adapter->sessionCtx.ap.apDisableIntraBssFwd) {
			/*
			 * When INTRA_BSS_FWD_OFFLOAD is enabled, FW will send
			 * all Rx packets to IPA uC, which need to be forwarded
			 * to other interface.
			 * And, IPA driver will send back to WLAN host driver
			 * through exception pipe with fw_desc field set by FW.
			 * Here we are checking fw_desc field for FORWARD bit
			 * set, and forward to Tx. Then copy to kernel stack
			 * only when DISCARD bit is not set.
			 */
			fw_desc = (uint8_t)skb->cb[1];

			if (fw_desc & HDD_IPA_FW_RX_DESC_FORWARD_M) {
				HDD_IPA_LOG(
					QDF_TRACE_LEVEL_DEBUG,
					"Forward packet to Tx (fw_desc=%d)",
					fw_desc);
				copy = qdf_nbuf_copy(skb);
				if (copy) {
					hdd_ipa->ipa_tx_forward++;
					ret = hdd_softap_hard_start_xmit(
						(struct sk_buff *)copy,
						adapter->dev);
					if (ret) {
						HDD_IPA_LOG(
							QDF_TRACE_LEVEL_DEBUG,
							"Forward packet tx fail");
						hdd_ipa->stats.
							num_tx_bcmc_err++;
					} else {
						hdd_ipa->stats.num_tx_bcmc++;
					}
				}
			}

			if (fw_desc & HDD_IPA_FW_RX_DESC_DISCARD_M) {
				HDD_IPA_INCREASE_INTERNAL_DROP_COUNT(hdd_ipa);
				hdd_ipa->ipa_rx_discard++;
				qdf_nbuf_free(skb);
				break;
			}

		} else {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO_HIGH,
				"Intra-BSS FWD is disabled-skip forward to Tx");
		}

		hdd_ipa_send_skb_to_network(skb, adapter);
		break;

	default:
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "w2i cb wrong event: 0x%x", evt);
		return;
	}
}

/**
 * hdd_ipa_nbuf_cb() - IPA TX complete callback
 * @skb: packet buffer which was transmitted
 *
 * Return: None
 */
void hdd_ipa_nbuf_cb(qdf_nbuf_t skb)
{
	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%p",
		wlan_hdd_stub_priv_to_addr(QDF_NBUF_CB_TX_IPA_PRIV(skb)));
	/* FIXME: This is broken; PRIV_DATA is now 31 bits */
	ipa_free_skb((struct ipa_rx_data *)
		wlan_hdd_stub_priv_to_addr(QDF_NBUF_CB_TX_IPA_PRIV(skb)));

	hdd_ipa->stats.num_tx_comp_cnt++;

	atomic_dec(&hdd_ipa->tx_ref_cnt);

	hdd_ipa_rm_try_release(hdd_ipa);
}

/**
 * hdd_ipa_send_pkt_to_tl() - Send an IPA packet to TL
 * @iface_context: interface-specific IPA context
 * @ipa_tx_desc: packet data descriptor
 *
 * Return: None
 */
static void hdd_ipa_send_pkt_to_tl(
		struct hdd_ipa_iface_context *iface_context,
		struct ipa_rx_data *ipa_tx_desc)
{
	struct hdd_ipa_priv *hdd_ipa = iface_context->hdd_ipa;
	uint8_t interface_id;
	hdd_adapter_t *adapter = NULL;
	qdf_nbuf_t skb;

	qdf_spin_lock_bh(&iface_context->interface_lock);
	adapter = iface_context->adapter;
	if (!adapter) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_WARN, "Interface Down");
		ipa_free_skb(ipa_tx_desc);
		iface_context->stats.num_tx_drop++;
		qdf_spin_unlock_bh(&iface_context->interface_lock);
		hdd_ipa_rm_try_release(hdd_ipa);
		return;
	}

	/*
	 * During CAC period, data packets shouldn't be sent over the air so
	 * drop all the packets here
	 */
	if (WLAN_HDD_GET_AP_CTX_PTR(adapter)->dfs_cac_block_tx) {
		ipa_free_skb(ipa_tx_desc);
		qdf_spin_unlock_bh(&iface_context->interface_lock);
		iface_context->stats.num_tx_cac_drop++;
		hdd_ipa_rm_try_release(hdd_ipa);
		return;
	}

	interface_id = adapter->sessionId;
	++adapter->stats.tx_packets;

	qdf_spin_unlock_bh(&iface_context->interface_lock);

	skb = ipa_tx_desc->skb;

	qdf_mem_set(skb->cb, sizeof(skb->cb), 0);
	qdf_nbuf_ipa_owned_set(skb);
	/* FIXME: This is broken. No such field in cb any more:
	   NBUF_CALLBACK_FN(skb) = hdd_ipa_nbuf_cb; */
	if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx)) {
		qdf_nbuf_mapped_paddr_set(skb,
					  ipa_tx_desc->dma_addr
					  + HDD_IPA_WLAN_FRAG_HEADER
					  + HDD_IPA_WLAN_IPA_HEADER);
		ipa_tx_desc->skb->len -=
			HDD_IPA_WLAN_FRAG_HEADER + HDD_IPA_WLAN_IPA_HEADER;
	} else
		qdf_nbuf_mapped_paddr_set(skb, ipa_tx_desc->dma_addr);

	/* FIXME: This is broken: priv_data is 31 bits */
	qdf_nbuf_ipa_priv_set(skb, wlan_hdd_stub_addr_to_priv(ipa_tx_desc));

	adapter->stats.tx_bytes += ipa_tx_desc->skb->len;

	skb = ol_tx_send_ipa_data_frame(iface_context->tl_context,
					 ipa_tx_desc->skb);
	if (skb) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "TLSHIM tx fail");
		ipa_free_skb(ipa_tx_desc);
		iface_context->stats.num_tx_err++;
		hdd_ipa_rm_try_release(hdd_ipa);
		return;
	}

	atomic_inc(&hdd_ipa->tx_ref_cnt);

	iface_context->stats.num_tx++;

}

/**
 * hdd_ipa_pm_send_pkt_to_tl() - Send queued packets to TL
 * @work: pointer to the scheduled work
 *
 * Called during PM resume to send packets to TL which were queued
 * while host was in the process of suspending.
 *
 * Return: None
 */
static void hdd_ipa_pm_send_pkt_to_tl(struct work_struct *work)
{
	struct hdd_ipa_priv *hdd_ipa = container_of(work,
						    struct hdd_ipa_priv,
						    pm_work);
	struct hdd_ipa_pm_tx_cb *pm_tx_cb = NULL;
	qdf_nbuf_t skb;
	uint32_t dequeued = 0;

	qdf_spin_lock_bh(&hdd_ipa->pm_lock);

	while (((skb = qdf_nbuf_queue_remove(&hdd_ipa->pm_queue_head))
								!= NULL)) {
		qdf_spin_unlock_bh(&hdd_ipa->pm_lock);

		pm_tx_cb = (struct hdd_ipa_pm_tx_cb *)skb->cb;

		dequeued++;

		hdd_ipa_send_pkt_to_tl(pm_tx_cb->iface_context,
				       pm_tx_cb->ipa_tx_desc);

		qdf_spin_lock_bh(&hdd_ipa->pm_lock);
	}

	qdf_spin_unlock_bh(&hdd_ipa->pm_lock);

	hdd_ipa->stats.num_tx_dequeued += dequeued;
	if (dequeued > hdd_ipa->stats.num_max_pm_queue)
		hdd_ipa->stats.num_max_pm_queue = dequeued;
}

/**
 * hdd_ipa_i2w_cb() - IPA to WLAN callback
 * @priv: pointer to private data registered with IPA (we register a
 *	pointer to the interface-specific IPA context)
 * @evt: the IPA event which triggered the callback
 * @data: data associated with the event
 *
 * Return: None
 */
static void hdd_ipa_i2w_cb(void *priv, enum ipa_dp_evt_type evt,
			   unsigned long data)
{
	struct hdd_ipa_priv *hdd_ipa = NULL;
	struct ipa_rx_data *ipa_tx_desc;
	struct hdd_ipa_iface_context *iface_context;
	qdf_nbuf_t skb;
	struct hdd_ipa_pm_tx_cb *pm_tx_cb = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	iface_context = (struct hdd_ipa_iface_context *)priv;
	if (evt != IPA_RECEIVE) {
		skb = (qdf_nbuf_t) data;
		dev_kfree_skb_any(skb);
		iface_context->stats.num_tx_drop++;
		return;
	}

	ipa_tx_desc = (struct ipa_rx_data *)data;

	hdd_ipa = iface_context->hdd_ipa;

	/*
	 * When SSR is going on or driver is unloading, just drop the packets.
	 * During SSR, there is no use in queueing the packets as STA has to
	 * connect back any way
	 */
	status = wlan_hdd_validate_context(hdd_ipa->hdd_ctx);
	if (status) {
		ipa_free_skb(ipa_tx_desc);
		iface_context->stats.num_tx_drop++;
		return;
	}

	skb = ipa_tx_desc->skb;

	HDD_IPA_DBG_DUMP(QDF_TRACE_LEVEL_DEBUG, "i2w", skb->data, 8);

	/*
	 * If PROD resource is not requested here then there may be cases where
	 * IPA hardware may be clocked down because of not having proper
	 * dependency graph between WLAN CONS and modem PROD pipes. Adding the
	 * workaround to request PROD resource while data is going over CONS
	 * pipe to prevent the IPA hardware clockdown.
	 */
	hdd_ipa_rm_request(hdd_ipa);

	qdf_spin_lock_bh(&hdd_ipa->pm_lock);
	/*
	 * If host is still suspended then queue the packets and these will be
	 * drained later when resume completes. When packet is arrived here and
	 * host is suspended, this means that there is already resume is in
	 * progress.
	 */
	if (hdd_ipa->suspended) {
		qdf_mem_set(skb->cb, sizeof(skb->cb), 0);
		pm_tx_cb = (struct hdd_ipa_pm_tx_cb *)skb->cb;
		pm_tx_cb->iface_context = iface_context;
		pm_tx_cb->ipa_tx_desc = ipa_tx_desc;
		qdf_nbuf_queue_add(&hdd_ipa->pm_queue_head, skb);
		hdd_ipa->stats.num_tx_queued++;

		qdf_spin_unlock_bh(&hdd_ipa->pm_lock);
		return;
	}

	qdf_spin_unlock_bh(&hdd_ipa->pm_lock);

	/*
	 * If we are here means, host is not suspended, wait for the work queue
	 * to finish.
	 */
#ifdef WLAN_OPEN_SOURCE
	flush_work(&hdd_ipa->pm_work);
#endif

	return hdd_ipa_send_pkt_to_tl(iface_context, ipa_tx_desc);
}

/**
 * hdd_ipa_suspend() - Suspend IPA
 * @hdd_ctx: Global HDD context
 *
 * Return: 0 on success, negativer errno on error
 */
int hdd_ipa_suspend(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa = hdd_ctx->hdd_ipa;

	if (!hdd_ipa_is_enabled(hdd_ctx))
		return 0;

	/*
	 * Check if IPA is ready for suspend, If we are here means, there is
	 * high chance that suspend would go through but just to avoid any race
	 * condition after suspend started, these checks are conducted before
	 * allowing to suspend.
	 */
	if (atomic_read(&hdd_ipa->tx_ref_cnt))
		return -EAGAIN;

	qdf_spin_lock_bh(&hdd_ipa->rm_lock);

	if (hdd_ipa->rm_state != HDD_IPA_RM_RELEASED) {
		qdf_spin_unlock_bh(&hdd_ipa->rm_lock);
		return -EAGAIN;
	}
	qdf_spin_unlock_bh(&hdd_ipa->rm_lock);

	qdf_spin_lock_bh(&hdd_ipa->pm_lock);
	hdd_ipa->suspended = true;
	qdf_spin_unlock_bh(&hdd_ipa->pm_lock);

	return 0;
}

/**
 * hdd_ipa_resume() - Resume IPA following suspend
 * hdd_ctx: Global HDD context
 *
 * Return: 0 on success, negative errno on error
 */
int hdd_ipa_resume(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa = hdd_ctx->hdd_ipa;

	if (!hdd_ipa_is_enabled(hdd_ctx))
		return 0;

	schedule_work(&hdd_ipa->pm_work);

	qdf_spin_lock_bh(&hdd_ipa->pm_lock);
	hdd_ipa->suspended = false;
	qdf_spin_unlock_bh(&hdd_ipa->pm_lock);

	return 0;
}

/**
 * hdd_ipa_setup_sys_pipe() - Setup all IPA Sys pipes
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: 0 on success, negative errno on error
 */
static int hdd_ipa_setup_sys_pipe(struct hdd_ipa_priv *hdd_ipa)
{
	int i, ret = 0;
	struct ipa_sys_connect_params *ipa;
	uint32_t desc_fifo_sz;

	/* The maximum number of descriptors that can be provided to a BAM at
	 * once is one less than the total number of descriptors that the buffer
	 * can contain.
	 * If max_num_of_descriptors = (BAM_PIPE_DESCRIPTOR_FIFO_SIZE / sizeof
	 * (SPS_DESCRIPTOR)), then (max_num_of_descriptors - 1) descriptors can
	 * be provided at once.
	 * Because of above requirement, one extra descriptor will be added to
	 * make sure hardware always has one descriptor.
	 */
	desc_fifo_sz = hdd_ipa->hdd_ctx->config->IpaDescSize
		       + sizeof(struct sps_iovec);

	/*setup TX pipes */
	for (i = 0; i < HDD_IPA_MAX_IFACE; i++) {
		ipa = &hdd_ipa->sys_pipe[i].ipa_sys_params;

		ipa->client = hdd_ipa_adapter_2_client[i].cons_client;
		ipa->desc_fifo_sz = desc_fifo_sz;
		ipa->priv = &hdd_ipa->iface_context[i];
		ipa->notify = hdd_ipa_i2w_cb;

		if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx)) {
			ipa->ipa_ep_cfg.hdr.hdr_len =
				HDD_IPA_UC_WLAN_TX_HDR_LEN;
			ipa->ipa_ep_cfg.nat.nat_en = IPA_BYPASS_NAT;
			ipa->ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid = 1;
			ipa->ipa_ep_cfg.hdr.hdr_ofst_pkt_size = 0;
			ipa->ipa_ep_cfg.hdr.hdr_additional_const_len =
				HDD_IPA_UC_WLAN_8023_HDR_SIZE;
			ipa->ipa_ep_cfg.hdr_ext.hdr_little_endian = true;
		} else {
			ipa->ipa_ep_cfg.hdr.hdr_len = HDD_IPA_WLAN_TX_HDR_LEN;
		}
		ipa->ipa_ep_cfg.mode.mode = IPA_BASIC;

		if (!hdd_ipa_is_rm_enabled(hdd_ipa->hdd_ctx))
			ipa->keep_ipa_awake = 1;

		ret = ipa_setup_sys_pipe(ipa, &(hdd_ipa->sys_pipe[i].conn_hdl));
		if (ret) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "Failed for pipe %d"
				    " ret: %d", i, ret);
			goto setup_sys_pipe_fail;
		}
		hdd_ipa->sys_pipe[i].conn_hdl_valid = 1;
	}

	if (!hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx)) {
		/*
		 * Hard code it here, this can be extended if in case
		 * PROD pipe is also per interface.
		 * Right now there is no advantage of doing this.
		 */
		hdd_ipa->prod_client = IPA_CLIENT_WLAN1_PROD;

		ipa = &hdd_ipa->sys_pipe[HDD_IPA_RX_PIPE].ipa_sys_params;

		ipa->client = hdd_ipa->prod_client;

		ipa->desc_fifo_sz = desc_fifo_sz;
		ipa->priv = hdd_ipa;
		ipa->notify = hdd_ipa_w2i_cb;

		ipa->ipa_ep_cfg.nat.nat_en = IPA_BYPASS_NAT;
		ipa->ipa_ep_cfg.hdr.hdr_len = HDD_IPA_WLAN_RX_HDR_LEN;
		ipa->ipa_ep_cfg.hdr.hdr_ofst_metadata_valid = 1;
		ipa->ipa_ep_cfg.mode.mode = IPA_BASIC;

		if (!hdd_ipa_is_rm_enabled(hdd_ipa->hdd_ctx))
			ipa->keep_ipa_awake = 1;

		ret = ipa_setup_sys_pipe(ipa, &(hdd_ipa->sys_pipe[i].conn_hdl));
		if (ret) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
					"Failed for RX pipe: %d", ret);
			goto setup_sys_pipe_fail;
		}
		hdd_ipa->sys_pipe[HDD_IPA_RX_PIPE].conn_hdl_valid = 1;
	}

	return ret;

setup_sys_pipe_fail:

	while (--i >= 0) {
		ipa_teardown_sys_pipe(hdd_ipa->sys_pipe[i].conn_hdl);
		qdf_mem_zero(&hdd_ipa->sys_pipe[i],
			     sizeof(struct hdd_ipa_sys_pipe));
	}

	return ret;
}

/**
 * hdd_ipa_teardown_sys_pipe() - Tear down all IPA Sys pipes
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: None
 */
static void hdd_ipa_teardown_sys_pipe(struct hdd_ipa_priv *hdd_ipa)
{
	int ret = 0, i;
	for (i = 0; i < HDD_IPA_MAX_SYSBAM_PIPE; i++) {
		if (hdd_ipa->sys_pipe[i].conn_hdl_valid) {
			ret =
				ipa_teardown_sys_pipe(hdd_ipa->sys_pipe[i].
						      conn_hdl);
			if (ret)
				HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "Failed: %d",
					    ret);

			hdd_ipa->sys_pipe[i].conn_hdl_valid = 0;
		}
	}
}

/**
 * hdd_ipa_register_interface() - register IPA interface
 * @hdd_ipa: Global IPA context
 * @iface_context: Per-interface IPA context
 *
 * Return: 0 on success, negative errno on error
 */
static int hdd_ipa_register_interface(struct hdd_ipa_priv *hdd_ipa,
				      struct hdd_ipa_iface_context
				      *iface_context)
{
	struct ipa_tx_intf tx_intf;
	struct ipa_rx_intf rx_intf;
	struct ipa_ioc_tx_intf_prop *tx_prop = NULL;
	struct ipa_ioc_rx_intf_prop *rx_prop = NULL;
	char *ifname = iface_context->adapter->dev->name;

	char ipv4_hdr_name[IPA_RESOURCE_NAME_MAX];
	char ipv6_hdr_name[IPA_RESOURCE_NAME_MAX];

	int num_prop = 1;
	int ret = 0;

	if (hdd_ipa_is_ipv6_enabled(hdd_ipa->hdd_ctx))
		num_prop++;

	/* Allocate TX properties for TOS categories, 1 each for IPv4 & IPv6 */
	tx_prop =
		qdf_mem_malloc(sizeof(struct ipa_ioc_tx_intf_prop) * num_prop);
	if (!tx_prop) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "tx_prop allocation failed");
		goto register_interface_fail;
	}

	/* Allocate RX properties, 1 each for IPv4 & IPv6 */
	rx_prop =
		qdf_mem_malloc(sizeof(struct ipa_ioc_rx_intf_prop) * num_prop);
	if (!rx_prop) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "rx_prop allocation failed");
		goto register_interface_fail;
	}

	qdf_mem_zero(&tx_intf, sizeof(tx_intf));
	qdf_mem_zero(&rx_intf, sizeof(rx_intf));

	snprintf(ipv4_hdr_name, IPA_RESOURCE_NAME_MAX, "%s%s",
		 ifname, HDD_IPA_IPV4_NAME_EXT);
	snprintf(ipv6_hdr_name, IPA_RESOURCE_NAME_MAX, "%s%s",
		 ifname, HDD_IPA_IPV6_NAME_EXT);

	rx_prop[IPA_IP_v4].ip = IPA_IP_v4;
	rx_prop[IPA_IP_v4].src_pipe = iface_context->prod_client;
	rx_prop[IPA_IP_v4].hdr_l2_type = IPA_HDR_L2_ETHERNET_II;
	rx_prop[IPA_IP_v4].attrib.attrib_mask = IPA_FLT_META_DATA;

	/*
	 * Interface ID is 3rd byte in the CLD header. Add the meta data and
	 * mask to identify the interface in IPA hardware
	 */
	rx_prop[IPA_IP_v4].attrib.meta_data =
		htonl(iface_context->adapter->sessionId << 16);
	rx_prop[IPA_IP_v4].attrib.meta_data_mask = htonl(0x00FF0000);

	rx_intf.num_props++;
	if (hdd_ipa_is_ipv6_enabled(hdd_ipa->hdd_ctx)) {
		rx_prop[IPA_IP_v6].ip = IPA_IP_v6;
		rx_prop[IPA_IP_v6].src_pipe = iface_context->prod_client;
		rx_prop[IPA_IP_v6].hdr_l2_type = IPA_HDR_L2_ETHERNET_II;
		rx_prop[IPA_IP_v4].attrib.attrib_mask = IPA_FLT_META_DATA;
		rx_prop[IPA_IP_v4].attrib.meta_data =
			htonl(iface_context->adapter->sessionId << 16);
		rx_prop[IPA_IP_v4].attrib.meta_data_mask = htonl(0x00FF0000);

		rx_intf.num_props++;
	}

	tx_prop[IPA_IP_v4].ip = IPA_IP_v4;
	tx_prop[IPA_IP_v4].hdr_l2_type = IPA_HDR_L2_ETHERNET_II;
	tx_prop[IPA_IP_v4].dst_pipe = IPA_CLIENT_WLAN1_CONS;
	tx_prop[IPA_IP_v4].alt_dst_pipe = iface_context->cons_client;
	strlcpy(tx_prop[IPA_IP_v4].hdr_name, ipv4_hdr_name,
			IPA_RESOURCE_NAME_MAX);
	tx_intf.num_props++;

	if (hdd_ipa_is_ipv6_enabled(hdd_ipa->hdd_ctx)) {
		tx_prop[IPA_IP_v6].ip = IPA_IP_v6;
		tx_prop[IPA_IP_v6].hdr_l2_type = IPA_HDR_L2_ETHERNET_II;
		tx_prop[IPA_IP_v6].dst_pipe = IPA_CLIENT_WLAN1_CONS;
		tx_prop[IPA_IP_v6].alt_dst_pipe = iface_context->cons_client;
		strlcpy(tx_prop[IPA_IP_v6].hdr_name, ipv6_hdr_name,
				IPA_RESOURCE_NAME_MAX);
		tx_intf.num_props++;
	}

	tx_intf.prop = tx_prop;
	rx_intf.prop = rx_prop;

	/* Call the ipa api to register interface */
	ret = ipa_register_intf(ifname, &tx_intf, &rx_intf);

register_interface_fail:
	qdf_mem_free(tx_prop);
	qdf_mem_free(rx_prop);
	return ret;
}

/**
 * hdd_remove_ipa_header() - Remove a specific header from IPA
 * @name: Name of the header to be removed
 *
 * Return: None
 */
static void hdd_ipa_remove_header(char *name)
{
	struct ipa_ioc_get_hdr hdrlookup;
	int ret = 0, len;
	struct ipa_ioc_del_hdr *ipa_hdr;

	qdf_mem_zero(&hdrlookup, sizeof(hdrlookup));
	strlcpy(hdrlookup.name, name, sizeof(hdrlookup.name));
	ret = ipa_get_hdr(&hdrlookup);
	if (ret) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "Hdr deleted already %s, %d",
			    name, ret);
		return;
	}

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "hdl: 0x%x", hdrlookup.hdl);
	len = sizeof(struct ipa_ioc_del_hdr) + sizeof(struct ipa_hdr_del) * 1;
	ipa_hdr = (struct ipa_ioc_del_hdr *)qdf_mem_malloc(len);
	if (ipa_hdr == NULL) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "ipa_hdr allocation failed");
		return;
	}
	ipa_hdr->num_hdls = 1;
	ipa_hdr->commit = 0;
	ipa_hdr->hdl[0].hdl = hdrlookup.hdl;
	ipa_hdr->hdl[0].status = -1;
	ret = ipa_del_hdr(ipa_hdr);
	if (ret != 0)
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "Delete header failed: %d",
			    ret);

	qdf_mem_free(ipa_hdr);
}

/**
 * hdd_ipa_add_header_info() - Add IPA header for a given interface
 * @hdd_ipa: Global HDD IPA context
 * @iface_context: Interface-specific HDD IPA context
 * @mac_addr: Interface MAC address
 *
 * Return: 0 on success, negativer errno value on error
 */
static int hdd_ipa_add_header_info(struct hdd_ipa_priv *hdd_ipa,
				   struct hdd_ipa_iface_context *iface_context,
				   uint8_t *mac_addr)
{
	hdd_adapter_t *adapter = iface_context->adapter;
	char *ifname;
	struct ipa_ioc_add_hdr *ipa_hdr = NULL;
	int ret = -EINVAL;
	struct hdd_ipa_tx_hdr *tx_hdr = NULL;
	struct hdd_ipa_uc_tx_hdr *uc_tx_hdr = NULL;

	ifname = adapter->dev->name;

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "Add Partial hdr: %s, %pM",
		    ifname, mac_addr);

	/* dynamically allocate the memory to add the hdrs */
	ipa_hdr = qdf_mem_malloc(sizeof(struct ipa_ioc_add_hdr)
				 + sizeof(struct ipa_hdr_add));
	if (!ipa_hdr) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: ipa_hdr allocation failed", ifname);
		ret = -ENOMEM;
		goto end;
	}

	ipa_hdr->commit = 0;
	ipa_hdr->num_hdrs = 1;

	if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
		uc_tx_hdr = (struct hdd_ipa_uc_tx_hdr *)ipa_hdr->hdr[0].hdr;
		memcpy(uc_tx_hdr, &ipa_uc_tx_hdr, HDD_IPA_UC_WLAN_TX_HDR_LEN);
		memcpy(uc_tx_hdr->eth.h_source, mac_addr, ETH_ALEN);
		uc_tx_hdr->ipa_hd.vdev_id = iface_context->adapter->sessionId;
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
			"ifname=%s, vdev_id=%d",
			ifname, uc_tx_hdr->ipa_hd.vdev_id);
		snprintf(ipa_hdr->hdr[0].name, IPA_RESOURCE_NAME_MAX, "%s%s",
				ifname, HDD_IPA_IPV4_NAME_EXT);
		ipa_hdr->hdr[0].hdr_len = HDD_IPA_UC_WLAN_TX_HDR_LEN;
		ipa_hdr->hdr[0].type = IPA_HDR_L2_ETHERNET_II;
		ipa_hdr->hdr[0].is_partial = 1;
		ipa_hdr->hdr[0].hdr_hdl = 0;
		ipa_hdr->hdr[0].is_eth2_ofst_valid = 1;
		ipa_hdr->hdr[0].eth2_ofst = HDD_IPA_UC_WLAN_HDR_DES_MAC_OFFSET;

		ret = ipa_add_hdr(ipa_hdr);
	} else {
		tx_hdr = (struct hdd_ipa_tx_hdr *)ipa_hdr->hdr[0].hdr;

		/* Set the Source MAC */
		memcpy(tx_hdr, &ipa_tx_hdr, HDD_IPA_WLAN_TX_HDR_LEN);
		memcpy(tx_hdr->eth.h_source, mac_addr, ETH_ALEN);

		snprintf(ipa_hdr->hdr[0].name, IPA_RESOURCE_NAME_MAX, "%s%s",
				ifname, HDD_IPA_IPV4_NAME_EXT);
		ipa_hdr->hdr[0].hdr_len = HDD_IPA_WLAN_TX_HDR_LEN;
		ipa_hdr->hdr[0].is_partial = 1;
		ipa_hdr->hdr[0].hdr_hdl = 0;
		ipa_hdr->hdr[0].is_eth2_ofst_valid = 1;
		ipa_hdr->hdr[0].eth2_ofst = HDD_IPA_WLAN_HDR_DES_MAC_OFFSET;

		/* Set the type to IPV4 in the header */
		tx_hdr->llc_snap.eth_type = cpu_to_be16(ETH_P_IP);

		ret = ipa_add_hdr(ipa_hdr);
	}
	if (ret) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "%s IPv4 add hdr failed: %d",
			    ifname, ret);
		goto end;
	}

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "%s: IPv4 hdr_hdl: 0x%x",
		    ipa_hdr->hdr[0].name, ipa_hdr->hdr[0].hdr_hdl);

	if (hdd_ipa_is_ipv6_enabled(hdd_ipa->hdd_ctx)) {
		snprintf(ipa_hdr->hdr[0].name, IPA_RESOURCE_NAME_MAX, "%s%s",
			 ifname, HDD_IPA_IPV6_NAME_EXT);

		if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			uc_tx_hdr =
				(struct hdd_ipa_uc_tx_hdr *)ipa_hdr->hdr[0].hdr;
			uc_tx_hdr->eth.h_proto = cpu_to_be16(ETH_P_IPV6);
		} else {
			/* Set the type to IPV6 in the header */
			tx_hdr = (struct hdd_ipa_tx_hdr *)ipa_hdr->hdr[0].hdr;
			tx_hdr->llc_snap.eth_type = cpu_to_be16(ETH_P_IPV6);
		}

		ret = ipa_add_hdr(ipa_hdr);
		if (ret) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "%s: IPv6 add hdr failed: %d", ifname, ret);
			goto clean_ipv4_hdr;
		}

		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "%s: IPv6 hdr_hdl: 0x%x",
			    ipa_hdr->hdr[0].name, ipa_hdr->hdr[0].hdr_hdl);
	}

	qdf_mem_free(ipa_hdr);

	return ret;

clean_ipv4_hdr:
	snprintf(ipa_hdr->hdr[0].name, IPA_RESOURCE_NAME_MAX, "%s%s",
		 ifname, HDD_IPA_IPV4_NAME_EXT);
	hdd_ipa_remove_header(ipa_hdr->hdr[0].name);
end:
	if (ipa_hdr)
		qdf_mem_free(ipa_hdr);

	return ret;
}

/**
 * hdd_ipa_clean_hdr() - Cleanup IPA on a given adapter
 * @adapter: Adapter upon which IPA was previously configured
 *
 * Return: None
 */
static void hdd_ipa_clean_hdr(hdd_adapter_t *adapter)
{
	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;
	int ret;
	char name_ipa[IPA_RESOURCE_NAME_MAX];

	/* Remove the headers */
	snprintf(name_ipa, IPA_RESOURCE_NAME_MAX, "%s%s",
		 adapter->dev->name, HDD_IPA_IPV4_NAME_EXT);
	hdd_ipa_remove_header(name_ipa);

	if (hdd_ipa_is_ipv6_enabled(hdd_ipa->hdd_ctx)) {
		snprintf(name_ipa, IPA_RESOURCE_NAME_MAX, "%s%s",
			 adapter->dev->name, HDD_IPA_IPV6_NAME_EXT);
		hdd_ipa_remove_header(name_ipa);
	}
	/* unregister the interface with IPA */
	ret = ipa_deregister_intf(adapter->dev->name);
	if (ret)
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			    "%s: ipa_deregister_intf fail: %d",
			    adapter->dev->name, ret);
}

/**
 * hdd_ipa_cleanup_iface() - Cleanup IPA on a given interface
 * @iface_context: interface-specific IPA context
 *
 * Return: None
 */
static void hdd_ipa_cleanup_iface(struct hdd_ipa_iface_context *iface_context)
{
	if (iface_context == NULL)
		return;

	hdd_ipa_clean_hdr(iface_context->adapter);

	qdf_spin_lock_bh(&iface_context->interface_lock);
	iface_context->adapter->ipa_context = NULL;
	iface_context->adapter = NULL;
	iface_context->tl_context = NULL;
	qdf_spin_unlock_bh(&iface_context->interface_lock);
	iface_context->ifa_address = 0;
	if (!iface_context->hdd_ipa->num_iface) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"NUM INTF 0, Invalid");
		QDF_ASSERT(0);
	}
	iface_context->hdd_ipa->num_iface--;
}

/**
 * hdd_ipa_setup_iface() - Setup IPA on a given interface
 * @hdd_ipa: HDD IPA global context
 * @adapter: Interface upon which IPA is being setup
 * @sta_id: Station ID of the API instance
 *
 * Return: 0 on success, negative errno value on error
 */
static int hdd_ipa_setup_iface(struct hdd_ipa_priv *hdd_ipa,
			       hdd_adapter_t *adapter, uint8_t sta_id)
{
	struct hdd_ipa_iface_context *iface_context = NULL;
	void *tl_context = NULL;
	int i, ret = 0;

	/* Lower layer may send multiple START_BSS_EVENT in DFS mode or during
	 * channel change indication. Since these indications are sent by lower
	 * layer as SAP updates and IPA doesn't have to do anything for these
	 * updates so ignoring!
	 */
	if (QDF_SAP_MODE == adapter->device_mode && adapter->ipa_context)
		return 0;

	for (i = 0; i < HDD_IPA_MAX_IFACE; i++) {
		if (hdd_ipa->iface_context[i].adapter == NULL) {
			iface_context = &(hdd_ipa->iface_context[i]);
			break;
		}
	}

	if (iface_context == NULL) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "All the IPA interfaces are in use");
		ret = -ENOMEM;
		goto end;
	}

	adapter->ipa_context = iface_context;
	iface_context->adapter = adapter;
	iface_context->sta_id = sta_id;
	tl_context = ol_txrx_get_vdev_by_sta_id(sta_id);

	if (tl_context == NULL) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "Not able to get TL context sta_id: %d", sta_id);
		ret = -EINVAL;
		goto end;
	}

	iface_context->tl_context = tl_context;

	ret = hdd_ipa_add_header_info(hdd_ipa, iface_context,
				      adapter->dev->dev_addr);

	if (ret)
		goto end;

	/* Configure the TX and RX pipes filter rules */
	ret = hdd_ipa_register_interface(hdd_ipa, iface_context);
	if (ret)
		goto cleanup_header;

	hdd_ipa->num_iface++;
	return ret;

cleanup_header:

	hdd_ipa_clean_hdr(adapter);
end:
	if (iface_context)
		hdd_ipa_cleanup_iface(iface_context);
	return ret;
}

/**
 * hdd_ipa_msg_free_fn() - Free an IPA message
 * @buff: pointer to the IPA message
 * @len: length of the IPA message
 * @type: type of IPA message
 *
 * Return: None
 */
static void hdd_ipa_msg_free_fn(void *buff, uint32_t len, uint32_t type)
{
	hddLog(LOG1, "msg type:%d, len:%d", type, len);
	ghdd_ipa->stats.num_free_msg++;
	qdf_mem_free(buff);
}

/**
 * hdd_ipa_send_mcc_scc_msg() - send IPA WLAN_SWITCH_TO_MCC/SCC message
 * @mcc_mode: 0=MCC/1=SCC
 *
 * Return: 0 on success, negative errno value on error
 */
int hdd_ipa_send_mcc_scc_msg(hdd_context_t *pHddCtx, bool mcc_mode)
{
	hdd_adapter_list_node_t *adapter_node = NULL, *next = NULL;
	QDF_STATUS status;
	hdd_adapter_t *pAdapter;
	struct ipa_msg_meta meta;
	struct ipa_wlan_msg *msg;
	int ret;

	if (!hdd_ipa_uc_sta_is_enabled(pHddCtx))
		return -EINVAL;

	if (!pHddCtx->mcc_mode) {
		/* Flush TxRx queue for each adapter before switch to SCC */
		status =  hdd_get_front_adapter(pHddCtx, &adapter_node);
		while (NULL != adapter_node && QDF_STATUS_SUCCESS == status) {
			pAdapter = adapter_node->pAdapter;
			if (pAdapter->device_mode == QDF_STA_MODE ||
				pAdapter->device_mode == QDF_SAP_MODE) {
				hddLog(QDF_TRACE_LEVEL_INFO,
					"MCC->SCC: Flush TxRx queue(d_mode=%d)",
					pAdapter->device_mode);
				hdd_deinit_tx_rx(pAdapter);
			}
			status = hdd_get_next_adapter(
					pHddCtx, adapter_node, &next);
			adapter_node = next;
		}
	}

	/* Send SCC/MCC Switching event to IPA */
	meta.msg_len = sizeof(*msg);
	msg = qdf_mem_malloc(meta.msg_len);
	if (msg == NULL) {
		hddLog(LOGE, "msg allocation failed");
		return -ENOMEM;
	}

	meta.msg_type = mcc_mode ?
			WLAN_SWITCH_TO_MCC : WLAN_SWITCH_TO_SCC;
	hddLog(LOG1, "ipa_send_msg(Evt:%d)", meta.msg_type);

	ret = ipa_send_msg(&meta, msg, hdd_ipa_msg_free_fn);

	if (ret) {
		hddLog(LOGE, "ipa_send_msg(Evt:%d) - fail=%d",
			meta.msg_type,  ret);
		qdf_mem_free(msg);
	}

	return ret;
}

/**
 * hdd_ipa_wlan_event_to_str() - convert IPA WLAN event to string
 * @event: IPA WLAN event to be converted to a string
 *
 * Return: ASCII string representing the IPA WLAN event
 */
static inline char *hdd_ipa_wlan_event_to_str(enum ipa_wlan_event event)
{
	switch (event) {
	case WLAN_CLIENT_CONNECT:
		return "WLAN_CLIENT_CONNECT";
	case WLAN_CLIENT_DISCONNECT:
		return "WLAN_CLIENT_DISCONNECT";
	case WLAN_CLIENT_POWER_SAVE_MODE:
		return "WLAN_CLIENT_POWER_SAVE_MODE";
	case WLAN_CLIENT_NORMAL_MODE:
		return "WLAN_CLIENT_NORMAL_MODE";
	case SW_ROUTING_ENABLE:
		return "SW_ROUTING_ENABLE";
	case SW_ROUTING_DISABLE:
		return "SW_ROUTING_DISABLE";
	case WLAN_AP_CONNECT:
		return "WLAN_AP_CONNECT";
	case WLAN_AP_DISCONNECT:
		return "WLAN_AP_DISCONNECT";
	case WLAN_STA_CONNECT:
		return "WLAN_STA_CONNECT";
	case WLAN_STA_DISCONNECT:
		return "WLAN_STA_DISCONNECT";
	case WLAN_CLIENT_CONNECT_EX:
		return "WLAN_CLIENT_CONNECT_EX";

	case IPA_WLAN_EVENT_MAX:
	default:
		return "UNKNOWN";
	}
}

/**
 * hdd_to_ipa_wlan_event() - convert hdd_ipa_wlan_event to ipa_wlan_event
 * @hdd_ipa_event_type: HDD IPA WLAN event to be converted to an ipa_wlan_event
 *
 * Return: ipa_wlan_event representing the hdd_ipa_wlan_event
 */
static enum ipa_wlan_event
hdd_to_ipa_wlan_event(enum hdd_ipa_wlan_event hdd_ipa_event_type)
{
	enum ipa_wlan_event ipa_event;

	switch (hdd_ipa_event_type) {
	case HDD_IPA_CLIENT_CONNECT:
		ipa_event = WLAN_CLIENT_CONNECT;
		break;
	case HDD_IPA_CLIENT_DISCONNECT:
		ipa_event = WLAN_CLIENT_DISCONNECT;
		break;
	case HDD_IPA_AP_CONNECT:
		ipa_event = WLAN_AP_CONNECT;
		break;
	case HDD_IPA_AP_DISCONNECT:
		ipa_event = WLAN_AP_DISCONNECT;
		break;
	case HDD_IPA_STA_CONNECT:
		ipa_event = WLAN_STA_CONNECT;
		break;
	case HDD_IPA_STA_DISCONNECT:
		ipa_event = WLAN_STA_DISCONNECT;
		break;
	case HDD_IPA_CLIENT_CONNECT_EX:
		ipa_event = WLAN_CLIENT_CONNECT_EX;
		break;
	case HDD_IPA_WLAN_EVENT_MAX:
	default:
		ipa_event = IPA_WLAN_EVENT_MAX;
		break;
	}
	return ipa_event;

}

/**
 * __hdd_ipa_wlan_evt() - IPA event handler
 * @adapter: adapter upon which the event was received
 * @sta_id: station id for the event
 * @type: event enum of type ipa_wlan_event
 * @mac_address: MAC address associated with the event
 *
 * This function is meant to be called from within wlan_hdd_ipa.c
 *
 * Return: 0 on success, negative errno value on error
 */
static int __hdd_ipa_wlan_evt(hdd_adapter_t *adapter, uint8_t sta_id,
		     enum ipa_wlan_event type, uint8_t *mac_addr)
{
	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;
	struct ipa_msg_meta meta;
	struct ipa_wlan_msg *msg;
	struct ipa_wlan_msg_ex *msg_ex = NULL;
	int ret;

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "%s: %s evt, MAC: %pM sta_id: %d",
		    adapter->dev->name, hdd_ipa_wlan_event_to_str(type),
		    mac_addr, sta_id);

	if (type >= IPA_WLAN_EVENT_MAX)
		return -EINVAL;

	if (WARN_ON(is_zero_ether_addr(mac_addr)))
		return -EINVAL;

	if (!hdd_ipa || !hdd_ipa_is_enabled(hdd_ipa->hdd_ctx)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "IPA OFFLOAD NOT ENABLED");
		return -EINVAL;
	}

	if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx) &&
		!hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx) &&
		(QDF_SAP_MODE != adapter->device_mode)) {
		return 0;
	}

	/*
	 * During IPA UC resource loading/unloading new events can be issued.
	 * Store the events separately and handle them later.
	 */
	if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx) &&
		((hdd_ipa->resource_loading) ||
		(hdd_ipa->resource_unloading))) {
		unsigned int pending_event_count;
		struct ipa_uc_pending_event *pending_event = NULL;

		hdd_err("IPA resource %s inprogress",
			hdd_ipa->resource_loading ? "load":"unload");

		qdf_mutex_acquire(&hdd_ipa->event_lock);

		pending_event_count = qdf_list_size(&hdd_ipa->pending_event);
		if (pending_event_count >= HDD_IPA_MAX_PENDING_EVENT_COUNT) {
			hdd_notice("Reached max pending event count");
			qdf_list_remove_front(&hdd_ipa->pending_event,
				(qdf_list_node_t **)&pending_event);
		} else {
			pending_event =
				(struct ipa_uc_pending_event *)qdf_mem_malloc(
					sizeof(struct ipa_uc_pending_event));
		}

		if (!pending_event) {
			hdd_err("Pending event memory alloc fail");
			qdf_mutex_release(&hdd_ipa->event_lock);
			return -ENOMEM;
		}

		pending_event->adapter = adapter;
		pending_event->sta_id = sta_id;
		pending_event->type = type;
		qdf_mem_copy(pending_event->mac_addr,
			mac_addr,
			QDF_MAC_ADDR_SIZE);
		qdf_list_insert_back(&hdd_ipa->pending_event,
				&pending_event->node);

		qdf_mutex_release(&hdd_ipa->event_lock);
		return 0;
	}

	hdd_ipa->stats.event[type]++;

	meta.msg_type = type;
	switch (type) {
	case WLAN_STA_CONNECT:
		/* STA already connected and without disconnect, connect again
		 * This is Roaming scenario
		 */
		if (hdd_ipa->sta_connected)
			hdd_ipa_cleanup_iface(adapter->ipa_context);

		if ((hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx)) &&
			(!hdd_ipa->sta_connected))
			hdd_ipa_uc_offload_enable_disable(adapter,
				SIR_STA_RX_DATA_OFFLOAD, 1);

		qdf_mutex_acquire(&hdd_ipa->event_lock);

		if (!hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
				"%s: Evt: %d, IPA UC OFFLOAD NOT ENABLED",
				msg_ex->name, meta.msg_type);
		} else if ((!hdd_ipa->sap_num_connected_sta) &&
			(!hdd_ipa->sta_connected)) {
			/* Enable IPA UC TX PIPE when STA connected */
			ret = hdd_ipa_uc_handle_first_con(hdd_ipa);
			if (ret) {
				qdf_mutex_release(&hdd_ipa->event_lock);
				HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
					"handle 1st con ret %d", ret);
				hdd_ipa_uc_offload_enable_disable(adapter,
					SIR_STA_RX_DATA_OFFLOAD, 0);
				goto end;
			}
		}
		ret = hdd_ipa_setup_iface(hdd_ipa, adapter, sta_id);
		if (ret) {
			qdf_mutex_release(&hdd_ipa->event_lock);
			hdd_ipa_uc_offload_enable_disable(adapter,
				SIR_STA_RX_DATA_OFFLOAD, 0);
			goto end;

#ifdef IPA_UC_OFFLOAD
		vdev_to_iface[adapter->sessionId] =
			((struct hdd_ipa_iface_context *)
				(adapter->ipa_context))->iface_id;
#endif /* IPA_UC_OFFLOAD */
		}

		qdf_mutex_release(&hdd_ipa->event_lock);

		hdd_ipa->sta_connected = 1;
		break;

	case WLAN_AP_CONNECT:
		/* For DFS channel we get two start_bss event (before and after
		 * CAC). Also when ACS range includes both DFS and non DFS
		 * channels, we could possibly change channel many times due to
		 * RADAR detection and chosen channel may not be a DFS channels.
		 * So dont return error here. Just discard the event.
		 */
		if (adapter->ipa_context)
			return 0;

		if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			hdd_ipa_uc_offload_enable_disable(adapter,
				SIR_AP_RX_DATA_OFFLOAD, 1);
		}
		qdf_mutex_acquire(&hdd_ipa->event_lock);
		ret = hdd_ipa_setup_iface(hdd_ipa, adapter, sta_id);
		if (ret) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
				"%s: Evt: %d, Interface setup failed",
				msg_ex->name, meta.msg_type);
			qdf_mutex_release(&hdd_ipa->event_lock);
			goto end;

#ifdef IPA_UC_OFFLOAD
		vdev_to_iface[adapter->sessionId] =
			((struct hdd_ipa_iface_context *)
				(adapter->ipa_context))->iface_id;
#endif /* IPA_UC_OFFLOAD */
		}
		qdf_mutex_release(&hdd_ipa->event_lock);
		break;

	case WLAN_STA_DISCONNECT:
		qdf_mutex_acquire(&hdd_ipa->event_lock);
		hdd_ipa_cleanup_iface(adapter->ipa_context);

		if (!hdd_ipa->sta_connected) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
				"%s: Evt: %d, STA already disconnected",
				msg_ex->name, meta.msg_type);
			qdf_mutex_release(&hdd_ipa->event_lock);
			return -EINVAL;
		}
		hdd_ipa->sta_connected = 0;
		if (!hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
				"%s: IPA UC OFFLOAD NOT ENABLED",
				msg_ex->name);
		} else {
			/* Disable IPA UC TX PIPE when STA disconnected */
			if ((!hdd_ipa->sap_num_connected_sta) ||
				((!hdd_ipa->num_iface) &&
					(HDD_IPA_UC_NUM_WDI_PIPE ==
					hdd_ipa->activated_fw_pipe))) {
				hdd_ipa_uc_handle_last_discon(hdd_ipa);
			}
		}

		if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx)) {
			hdd_ipa_uc_offload_enable_disable(adapter,
				SIR_STA_RX_DATA_OFFLOAD, 0);
			vdev_to_iface[adapter->sessionId] = HDD_IPA_MAX_IFACE;
		}

		qdf_mutex_release(&hdd_ipa->event_lock);
		break;

	case WLAN_AP_DISCONNECT:
		if (!adapter->ipa_context) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
				"%s: Evt: %d, SAP already disconnected",
				msg_ex->name, meta.msg_type);
			return -EINVAL;
		}

		qdf_mutex_acquire(&hdd_ipa->event_lock);
		hdd_ipa_cleanup_iface(adapter->ipa_context);
		if ((!hdd_ipa->num_iface) &&
			(HDD_IPA_UC_NUM_WDI_PIPE ==
				hdd_ipa->activated_fw_pipe)) {
			if (cds_is_driver_unloading()) {
				/*
				 * We disable WDI pipes directly here since
				 * IPA_OPCODE_TX/RX_SUSPEND message will not be
				 * processed when unloading WLAN driver is in
				 * progress
				 */
				hdd_ipa_uc_disable_pipes(hdd_ipa);
			} else {
				HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
					"NO INTF left but still pipe clean up");
				hdd_ipa_uc_handle_last_discon(hdd_ipa);
			}
		}

		if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			hdd_ipa_uc_offload_enable_disable(adapter,
				SIR_AP_RX_DATA_OFFLOAD, 0);
			vdev_to_iface[adapter->sessionId] = HDD_IPA_MAX_IFACE;
		}
		qdf_mutex_release(&hdd_ipa->event_lock);
		break;

	case WLAN_CLIENT_CONNECT_EX:
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "%d %d",
			    adapter->dev->ifindex, sta_id);

		if (!hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
				"%s: Evt: %d, IPA UC OFFLOAD NOT ENABLED",
				adapter->dev->name, meta.msg_type);
			return 0;
		}

		qdf_mutex_acquire(&hdd_ipa->event_lock);
		if (hdd_ipa_uc_find_add_assoc_sta(hdd_ipa,
				true, sta_id)) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				"%s: STA ID %d found, not valid",
				adapter->dev->name, sta_id);
			qdf_mutex_release(&hdd_ipa->event_lock);
			return 0;
		}

		/* Enable IPA UC Data PIPEs when first STA connected */
		if ((0 == hdd_ipa->sap_num_connected_sta) &&
		   (!hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx) ||
		   !hdd_ipa->sta_connected)) {
			ret = hdd_ipa_uc_handle_first_con(hdd_ipa);
			if (ret) {
				qdf_mutex_release(&hdd_ipa->event_lock);
				HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
					    "%s: handle 1st con ret %d",
					    adapter->dev->name, ret);
				return ret;
			}
		}

		hdd_ipa->sap_num_connected_sta++;

		qdf_mutex_release(&hdd_ipa->event_lock);

		meta.msg_type = type;
		meta.msg_len = (sizeof(struct ipa_wlan_msg_ex) +
				sizeof(struct ipa_wlan_hdr_attrib_val));
		msg_ex = qdf_mem_malloc(meta.msg_len);

		if (msg_ex == NULL) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "msg_ex allocation failed");
			return -ENOMEM;
		}
		strlcpy(msg_ex->name, adapter->dev->name,
			IPA_RESOURCE_NAME_MAX);
		msg_ex->num_of_attribs = 1;
		msg_ex->attribs[0].attrib_type = WLAN_HDR_ATTRIB_MAC_ADDR;
		if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			msg_ex->attribs[0].offset =
				HDD_IPA_UC_WLAN_HDR_DES_MAC_OFFSET;
		} else {
			msg_ex->attribs[0].offset =
				HDD_IPA_WLAN_HDR_DES_MAC_OFFSET;
		}
		memcpy(msg_ex->attribs[0].u.mac_addr, mac_addr,
		       IPA_MAC_ADDR_SIZE);

		ret = ipa_send_msg(&meta, msg_ex, hdd_ipa_msg_free_fn);

		if (ret) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "%s: Evt: %d : %d",
				    msg_ex->name, meta.msg_type, ret);
			qdf_mem_free(msg_ex);
			return ret;
		}
		hdd_ipa->stats.num_send_msg++;
		return ret;

	case WLAN_CLIENT_DISCONNECT:
		if (!hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
					"%s: IPA UC OFFLOAD NOT ENABLED",
					msg_ex->name);
			return 0;
		}

		qdf_mutex_acquire(&hdd_ipa->event_lock);
		if (!hdd_ipa_uc_find_add_assoc_sta(hdd_ipa, false, sta_id)) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "%s: STA ID %d NOT found, not valid",
				    msg_ex->name, sta_id);
			qdf_mutex_release(&hdd_ipa->event_lock);
			return 0;
		}
		hdd_ipa->sap_num_connected_sta--;
		/* Disable IPA UC TX PIPE when last STA disconnected */
		if (!hdd_ipa->sap_num_connected_sta
			&& (!hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx) ||
			!hdd_ipa->sta_connected)
			&& (false == hdd_ipa->resource_unloading)
			&& (HDD_IPA_UC_NUM_WDI_PIPE ==
				hdd_ipa->activated_fw_pipe))
			hdd_ipa_uc_handle_last_discon(hdd_ipa);
		qdf_mutex_release(&hdd_ipa->event_lock);
		break;

	default:
		return 0;
	}

	meta.msg_len = sizeof(struct ipa_wlan_msg);
	msg = qdf_mem_malloc(meta.msg_len);
	if (msg == NULL) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "msg allocation failed");
		return -ENOMEM;
	}

	meta.msg_type = type;
	strlcpy(msg->name, adapter->dev->name, IPA_RESOURCE_NAME_MAX);
	memcpy(msg->mac_addr, mac_addr, ETH_ALEN);

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "%s: Evt: %d",
		    msg->name, meta.msg_type);

	ret = ipa_send_msg(&meta, msg, hdd_ipa_msg_free_fn);

	if (ret) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "%s: Evt: %d fail:%d",
			    msg->name, meta.msg_type, ret);
		qdf_mem_free(msg);
		return ret;
	}

	hdd_ipa->stats.num_send_msg++;

end:
	return ret;
}

/**
 * hdd_ipa_wlan_evt() - IPA event handler
 * @adapter: adapter upon which the event was received
 * @sta_id: station id for the event
 * @hdd_event_type: event enum of type hdd_ipa_wlan_event
 * @mac_address: MAC address associated with the event
 *
 * This function is meant to be called from outside of wlan_hdd_ipa.c.
 *
 * Return: 0 on success, negative errno value on error
 */
int hdd_ipa_wlan_evt(hdd_adapter_t *adapter, uint8_t sta_id,
	enum hdd_ipa_wlan_event hdd_event_type, uint8_t *mac_addr)
{
	enum ipa_wlan_event type = hdd_to_ipa_wlan_event(hdd_event_type);

	return __hdd_ipa_wlan_evt(adapter, sta_id, type, mac_addr);
}

/**
 * hdd_ipa_uc_proc_pending_event() - Process IPA uC pending events
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: None
 */
static void
hdd_ipa_uc_proc_pending_event(struct hdd_ipa_priv *hdd_ipa)
{
	unsigned int pending_event_count;
	struct ipa_uc_pending_event *pending_event = NULL;

	pending_event_count = qdf_list_size(&hdd_ipa->pending_event);
	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
		"%s, Pending Event Count %d", __func__, pending_event_count);
	if (!pending_event_count) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			"%s, No Pending Event", __func__);
		return;
	}

	qdf_list_remove_front(&hdd_ipa->pending_event,
			(qdf_list_node_t **)&pending_event);
	while (pending_event != NULL) {
		__hdd_ipa_wlan_evt(pending_event->adapter,
			pending_event->type,
			pending_event->sta_id,
			pending_event->mac_addr);
		qdf_mem_free(pending_event);
		pending_event = NULL;
		qdf_list_remove_front(&hdd_ipa->pending_event,
			(qdf_list_node_t **)&pending_event);
	}
}

/**
 * hdd_ipa_rm_state_to_str() - Convert IPA RM state to string
 * @state: IPA RM state value
 *
 * Return: ASCII string representing the IPA RM state
 */
static inline char *hdd_ipa_rm_state_to_str(enum hdd_ipa_rm_state state)
{
	switch (state) {
	case HDD_IPA_RM_RELEASED:
		return "RELEASED";
	case HDD_IPA_RM_GRANT_PENDING:
		return "GRANT_PENDING";
	case HDD_IPA_RM_GRANTED:
		return "GRANTED";
	}

	return "UNKNOWN";
}

/**
 * hdd_ipa_init() - IPA initialization function
 * @hdd_ctx: HDD global context
 *
 * Allocate hdd_ipa resources, ipa pipe resource and register
 * wlan interface with IPA module.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_ipa_init(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa = NULL;
	int ret, i;
	struct hdd_ipa_iface_context *iface_context = NULL;
	struct ol_txrx_pdev_t *pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!hdd_ipa_is_enabled(hdd_ctx))
		return QDF_STATUS_SUCCESS;

	if (!pdev) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_FATAL, "pdev is NULL");
		goto fail_return;
	}

	hdd_ipa = qdf_mem_malloc(sizeof(*hdd_ipa));
	if (!hdd_ipa) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_FATAL, "hdd_ipa allocation failed");
		goto fail_return;
	}

	hdd_ctx->hdd_ipa = hdd_ipa;
	ghdd_ipa = hdd_ipa;
	hdd_ipa->hdd_ctx = hdd_ctx;
	hdd_ipa->num_iface = 0;
	ol_txrx_ipa_uc_get_resource(pdev, &hdd_ipa->ipa_resource);
	if ((0 == hdd_ipa->ipa_resource.ce_sr_base_paddr) ||
	    (0 == hdd_ipa->ipa_resource.tx_comp_ring_base_paddr) ||
	    (0 == hdd_ipa->ipa_resource.rx_rdy_ring_base_paddr) ||
	    (0 == hdd_ipa->ipa_resource.rx2_rdy_ring_base_paddr)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_FATAL,
			"IPA UC resource alloc fail");
		goto fail_get_resource;
	}

	/* Create the interface context */
	for (i = 0; i < HDD_IPA_MAX_IFACE; i++) {
		iface_context = &hdd_ipa->iface_context[i];
		iface_context->hdd_ipa = hdd_ipa;
		iface_context->cons_client =
			hdd_ipa_adapter_2_client[i].cons_client;
		iface_context->prod_client =
			hdd_ipa_adapter_2_client[i].prod_client;
		iface_context->iface_id = i;
		iface_context->adapter = NULL;
		qdf_spinlock_create(&iface_context->interface_lock);
	}

	INIT_WORK(&hdd_ipa->pm_work, hdd_ipa_pm_send_pkt_to_tl);
	qdf_spinlock_create(&hdd_ipa->pm_lock);
	qdf_nbuf_queue_init(&hdd_ipa->pm_queue_head);

	ret = hdd_ipa_setup_rm(hdd_ipa);
	if (ret)
		goto fail_setup_rm;

	if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
		hdd_ipa_uc_rt_debug_init(hdd_ctx);
		qdf_mem_zero(&hdd_ipa->stats, sizeof(hdd_ipa->stats));
		hdd_ipa->sap_num_connected_sta = 0;
		hdd_ipa->ipa_tx_packets_diff = 0;
		hdd_ipa->ipa_rx_packets_diff = 0;
		hdd_ipa->ipa_p_tx_packets = 0;
		hdd_ipa->ipa_p_rx_packets = 0;
		hdd_ipa->resource_loading = false;
		hdd_ipa->resource_unloading = false;
		hdd_ipa->sta_connected = 0;
		hdd_ipa->ipa_pipes_down = true;
		/* Setup IPA sys_pipe for MCC */
		if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx)) {
			ret = hdd_ipa_setup_sys_pipe(hdd_ipa);
			if (ret)
				goto fail_create_sys_pipe;
		}
		hdd_ipa_uc_ol_init(hdd_ctx);
	} else {
		ret = hdd_ipa_setup_sys_pipe(hdd_ipa);
		if (ret)
			goto fail_create_sys_pipe;
	}

	return QDF_STATUS_SUCCESS;

fail_create_sys_pipe:
	hdd_ipa_destroy_rm_resource(hdd_ipa);
fail_setup_rm:
	qdf_spinlock_destroy(&hdd_ipa->pm_lock);
fail_get_resource:
	qdf_mem_free(hdd_ipa);
	hdd_ctx->hdd_ipa = NULL;
	ghdd_ipa = NULL;
fail_return:
	return QDF_STATUS_E_FAILURE;
}

/**
 * hdd_ipa_cleanup_pending_event() - Cleanup IPA pending event list
 * @hdd_ipa: pointer to HDD IPA struct
 *
 * Return: none
 */
void hdd_ipa_cleanup_pending_event(struct hdd_ipa_priv *hdd_ipa)
{
	struct ipa_uc_pending_event *pending_event = NULL;

	while (qdf_list_remove_front(&hdd_ipa->pending_event,
		(qdf_list_node_t **)&pending_event) == QDF_STATUS_SUCCESS) {
		qdf_mem_free(pending_event);
	}

	qdf_list_destroy(&hdd_ipa->pending_event);
}

/**
 * hdd_ipa_cleanup - IPA cleanup function
 * @hdd_ctx: HDD global context
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_ipa_cleanup(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa = hdd_ctx->hdd_ipa;
	int i;
	struct hdd_ipa_iface_context *iface_context = NULL;
	qdf_nbuf_t skb;
	struct hdd_ipa_pm_tx_cb *pm_tx_cb = NULL;

	if (!hdd_ipa_is_enabled(hdd_ctx))
		return QDF_STATUS_SUCCESS;

	if (!hdd_ipa_uc_is_enabled(hdd_ctx)) {
		unregister_inetaddr_notifier(&hdd_ipa->ipv4_notifier);
		hdd_ipa_teardown_sys_pipe(hdd_ipa);
	}

	/* Teardown IPA sys_pipe for MCC */
	if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx))
		hdd_ipa_teardown_sys_pipe(hdd_ipa);

	hdd_ipa_destroy_rm_resource(hdd_ipa);

#ifdef WLAN_OPEN_SOURCE
	cancel_work_sync(&hdd_ipa->pm_work);
#endif

	qdf_spin_lock_bh(&hdd_ipa->pm_lock);

	while (((skb = qdf_nbuf_queue_remove(&hdd_ipa->pm_queue_head))
								!= NULL)) {
		qdf_spin_unlock_bh(&hdd_ipa->pm_lock);

		pm_tx_cb = (struct hdd_ipa_pm_tx_cb *)skb->cb;
		ipa_free_skb(pm_tx_cb->ipa_tx_desc);

		qdf_spin_lock_bh(&hdd_ipa->pm_lock);
	}
	qdf_spin_unlock_bh(&hdd_ipa->pm_lock);

	qdf_spinlock_destroy(&hdd_ipa->pm_lock);

	/* destory the interface lock */
	for (i = 0; i < HDD_IPA_MAX_IFACE; i++) {
		iface_context = &hdd_ipa->iface_context[i];
		qdf_spinlock_destroy(&iface_context->interface_lock);
	}

	/* This should never hit but still make sure that there are no pending
	 * descriptor in IPA hardware
	 */
	if (hdd_ipa->pending_hw_desc_cnt != 0) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "IPA Pending write done: %d Waiting!",
			    hdd_ipa->pending_hw_desc_cnt);

		for (i = 0; hdd_ipa->pending_hw_desc_cnt != 0 && i < 10; i++) {
			usleep_range(100, 100);
		}

		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "IPA Pending write done: desc: %d %s(%d)!",
			    hdd_ipa->pending_hw_desc_cnt,
			    hdd_ipa->pending_hw_desc_cnt == 0 ? "completed"
			    : "leak", i);
	}
	if (hdd_ipa_uc_is_enabled(hdd_ctx)) {
		hdd_ipa_uc_rt_debug_deinit(hdd_ctx);
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			    "%s: Disconnect TX PIPE", __func__);
		ipa_disconnect_wdi_pipe(hdd_ipa->tx_pipe_handle);
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO,
			    "%s: Disconnect RX PIPE", __func__);
		ipa_disconnect_wdi_pipe(hdd_ipa->rx_pipe_handle);
		qdf_mutex_destroy(&hdd_ipa->event_lock);
		qdf_mutex_destroy(&hdd_ipa->ipa_lock);
		hdd_ipa_cleanup_pending_event(hdd_ipa);

#ifdef WLAN_OPEN_SOURCE
		for (i = 0; i < HDD_IPA_UC_OPCODE_MAX; i++) {
			cancel_work_sync(&hdd_ipa->uc_op_work[i].work);
			hdd_ipa->uc_op_work[i].msg = NULL;
		}
#endif
	}

	qdf_mem_free(hdd_ipa);
	hdd_ctx->hdd_ipa = NULL;

	return QDF_STATUS_SUCCESS;
}
#endif /* IPA_OFFLOAD */
