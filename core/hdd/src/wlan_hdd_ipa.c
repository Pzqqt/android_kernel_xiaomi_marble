/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
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
#include <ol_txrx.h>
#include <cdp_txrx_peer_ops.h>

#include "cds_sched.h"

#include "wma.h"
#include "wma_api.h"
#include "wal_rx_desc.h"

#include "cdp_txrx_ipa.h"
#include <cdp_txrx_handle.h>
#include "wlan_policy_mgr_api.h"

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

enum hdd_ipa_uc_op_code {
	HDD_IPA_UC_OPCODE_TX_SUSPEND = 0,
	HDD_IPA_UC_OPCODE_TX_RESUME = 1,
	HDD_IPA_UC_OPCODE_RX_SUSPEND = 2,
	HDD_IPA_UC_OPCODE_RX_RESUME = 3,
	HDD_IPA_UC_OPCODE_STATS = 4,
#ifdef FEATURE_METERING
	HDD_IPA_UC_OPCODE_SHARING_STATS = 5,
	HDD_IPA_UC_OPCODE_QUOTA_RSP = 6,
	HDD_IPA_UC_OPCODE_QUOTA_IND = 7,
#endif
	HDD_IPA_UC_OPCODE_UC_READY = 8,
	/* keep this last */
	HDD_IPA_UC_OPCODE_MAX
};

/**
 * enum - Reason codes for stat query
 *
 * @HDD_IPA_UC_STAT_REASON_NONE: Initial value
 * @HDD_IPA_UC_STAT_REASON_DEBUG: For debug/info
 * @HDD_IPA_UC_STAT_REASON_BW_CAL: For bandwidth calibration
 * @HDD_IPA_UC_STAT_REASON_DUMP_INFO: For debug info dump
 */
enum {
	HDD_IPA_UC_STAT_REASON_NONE,
	HDD_IPA_UC_STAT_REASON_DEBUG,
	HDD_IPA_UC_STAT_REASON_BW_CAL,
	HDD_IPA_UC_STAT_REASON_DUMP_INFO
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
	bool exception;
	hdd_adapter_t *adapter;
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

	uint64_t num_tx_desc_q_cnt;
	uint64_t num_tx_desc_error;
	uint64_t num_tx_comp_cnt;
	uint64_t num_tx_queued;
	uint64_t num_tx_dequeued;
	uint64_t num_max_pm_queue;

	uint64_t num_freeq_empty;
	uint64_t num_pri_freeq_empty;
	uint64_t num_rx_excep;
	uint64_t num_tx_fwd_ok;
	uint64_t num_tx_fwd_err;
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

/**
 * struct uc_rt_debug_info
 * @time: system time
 * @ipa_excep_count: IPA exception packet count
 * @rx_drop_count: IPA Rx drop packet count
 * @net_sent_count: IPA Rx packet sent to network stack count
 * @rx_discard_count: IPA Rx discard packet count
 * @tx_fwd_ok_count: IPA Tx forward success packet count
 * @tx_fwd_count: IPA Tx forward packet count
 * @rx_destructor_call: IPA Rx packet destructor count
 */
struct uc_rt_debug_info {
	uint64_t time;
	uint64_t ipa_excep_count;
	uint64_t rx_drop_count;
	uint64_t net_sent_count;
	uint64_t rx_discard_count;
	uint64_t tx_fwd_ok_count;
	uint64_t tx_fwd_count;
	uint64_t rx_destructor_call;
};

#ifdef FEATURE_METERING
struct ipa_uc_sharing_stats {
	uint64_t ipv4_rx_packets;
	uint64_t ipv4_rx_bytes;
	uint64_t ipv6_rx_packets;
	uint64_t ipv6_rx_bytes;
	uint64_t ipv4_tx_packets;
	uint64_t ipv4_tx_bytes;
	uint64_t ipv6_tx_packets;
	uint64_t ipv6_tx_bytes;
};

struct ipa_uc_quota_rsp {
	uint8_t success;
	uint8_t reserved[3];
	uint32_t quota_lo;		/* quota limit low bytes */
	uint32_t quota_hi;		/* quota limit high bytes */
};

struct ipa_uc_quota_ind {
	uint64_t quota_bytes;		/* quota limit in bytes */
};
#endif

/**
 * struct hdd_ipa_tx_desc
 * @link: link to list head
 * @priv: pointer to priv list entry
 * @id: Tx desc idex
 * @ipa_tx_desc_ptr: pointer to IPA Tx descriptor
 */
struct hdd_ipa_tx_desc {
	struct list_head link;
	void *priv;
	uint32_t id;
	struct ipa_rx_data *ipa_tx_desc_ptr;
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

	qdf_spinlock_t q_lock;

	struct list_head pend_desc_head;
	struct hdd_ipa_tx_desc *tx_desc_list;
	struct list_head free_tx_desc_head;

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
	struct ipa_wdi_in_params cons_pipe_in;
	struct ipa_wdi_in_params prod_pipe_in;
	bool uc_loaded;
	bool wdi_enabled;
	qdf_mc_timer_t rt_debug_fill_timer;
	qdf_mutex_t rt_debug_lock;
	qdf_mutex_t ipa_lock;
	struct ol_txrx_ipa_resources ipa_resource;
	/* IPA UC doorbell registers paddr */
	qdf_dma_addr_t tx_comp_doorbell_paddr;
	qdf_dma_addr_t rx_ready_doorbell_paddr;
	uint8_t vdev_to_iface[CSR_ROAM_SESSION_MAX];
	bool vdev_offload_enabled[CSR_ROAM_SESSION_MAX];
#ifdef FEATURE_METERING
	struct ipa_uc_sharing_stats ipa_sharing_stats;
	struct ipa_uc_quota_rsp ipa_quota_rsp;
	struct ipa_uc_quota_ind ipa_quota_ind;
	struct completion ipa_uc_sharing_stats_comp;
	struct completion ipa_uc_set_quota_comp;
#endif
};

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

#define HDD_IPA_GET_IFACE_ID(_data) \
	(((struct hdd_ipa_cld_hdr *) (_data))->iface_id)

#define HDD_IPA_LOG(LVL, fmt, args ...) \
	QDF_TRACE(QDF_MODULE_ID_HDD, LVL, \
		  "%s:%d: "fmt, __func__, __LINE__, ## args)

#define HDD_IPA_DP_LOG(LVL, fmt, args...) \
	QDF_TRACE(QDF_MODULE_ID_HDD_DATA, LVL, \
		  "%s:%d: "fmt, __func__, __LINE__, ## args)

#define HDD_IPA_DBG_DUMP(_lvl, _prefix, _buf, _len) \
	do { \
		QDF_TRACE(QDF_MODULE_ID_HDD_DATA, _lvl, "%s:", _prefix); \
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_HDD_DATA, _lvl, _buf, _len); \
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

#if defined(QCA_WIFI_3_0) && defined(CONFIG_IPA3)
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

#define HDD_IPA_CHECK_HW() ipa_uc_reg_rdyCB(NULL)
#else
/* Do nothing */
#define HDD_IPA_WDI2_SET(pipe_in, ipa_ctxt)
#define HDD_IPA_CHECK_HW() 0
#endif /* IPA3 */

#define HDD_IPA_DBG_DUMP_RX_LEN 32
#define HDD_IPA_DBG_DUMP_TX_LEN 48

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

#ifdef FEATURE_METERING
#define IPA_UC_SHARING_STATES_WAIT_TIME	500
#define IPA_UC_SET_QUOTA_WAIT_TIME	500
#endif

static struct hdd_ipa_priv *ghdd_ipa;

/* Local Function Prototypes */
static void hdd_ipa_i2w_cb(void *priv, enum ipa_dp_evt_type evt,
			   unsigned long data);
static void hdd_ipa_w2i_cb(void *priv, enum ipa_dp_evt_type evt,
			   unsigned long data);
static void hdd_ipa_msg_free_fn(void *buff, uint32_t len, uint32_t type);

static void hdd_ipa_cleanup_iface(struct hdd_ipa_iface_context *iface_context);
static void hdd_ipa_uc_proc_pending_event(struct hdd_ipa_priv *hdd_ipa);

#if ((defined(QCA_WIFI_3_0) && defined(CONFIG_IPA3)) || \
	defined(IPA_CLIENT_IS_MHI_CONS))
/**
 * hdd_ipa_uc_get_db_paddr() - Get Doorbell physical address
 * @db_paddr: Doorbell physical address should be given bu IPA
 * @client: IPA client type
 *
 * Query doorbell physical address from IPA
 * IPA will give physical address for TX COMP and RX READY
 *
 * Return: None
 */
static void hdd_ipa_uc_get_db_paddr(qdf_dma_addr_t *db_paddr,
					  enum ipa_client_type client)
{
	struct ipa_wdi_db_params dbpa;

	dbpa.client = client;
	ipa_uc_wdi_get_dbpa(&dbpa);
	*db_paddr = dbpa.uc_door_bell_pa;
	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s PROD DB get dbpa 0x%x",
		__func__, (unsigned int)dbpa.uc_door_bell_pa);
}

/**
 * hdd_ipa_uc_loaded_uc_cb() - IPA UC loaded event callback
 * @priv_ctxt: hdd ipa local context
 *
 * Will be called by IPA context.
 * It's atomic context, then should be scheduled to kworker thread
 *
 * Return: None
 */
static void hdd_ipa_uc_loaded_uc_cb(void *priv_ctxt)
{
	struct hdd_ipa_priv *hdd_ipa;
	struct op_msg_type *msg;
	struct uc_op_work_struct *uc_op_work;

	if (priv_ctxt == NULL) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "Invalid IPA context");
		return;
	}

	hdd_ipa = (struct hdd_ipa_priv *)priv_ctxt;
	msg = (struct op_msg_type *)qdf_mem_malloc(sizeof(*msg));
	if (!msg) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "op_msg allocation fails");
		return;
	}

	msg->op_code = HDD_IPA_UC_OPCODE_UC_READY;

	uc_op_work = &hdd_ipa->uc_op_work[msg->op_code];

	/* When the same uC OPCODE is already pended, just return */
	if (uc_op_work->msg)
		return;

	uc_op_work->msg = msg;
	schedule_work(&uc_op_work->work);
}

/**
 * hdd_ipa_uc_send_wdi_control_msg() - Set WDI control message
 * @ctrl: WDI control value
 *
 * Send WLAN_WDI_ENABLE for ctrl = true and WLAN_WDI_DISABLE otherwise.
 *
 * Return: 0 on message send to ipa, -1 on failure
 */
static int hdd_ipa_uc_send_wdi_control_msg(bool ctrl)
{
	struct ipa_msg_meta meta;
	struct ipa_wlan_msg *ipa_msg;
	int ret = 0;

	/* WDI enable message to IPA */
	meta.msg_len = sizeof(*ipa_msg);
	ipa_msg = qdf_mem_malloc(meta.msg_len);
	if (ipa_msg == NULL) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"msg allocation failed");
		return -ENOMEM;
	}

	if (ctrl == true)
		meta.msg_type = WLAN_WDI_ENABLE;
	else
		meta.msg_type = WLAN_WDI_DISABLE;

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
		    "ipa_send_msg(Evt:%d)", meta.msg_type);
	ret = ipa_send_msg(&meta, ipa_msg, hdd_ipa_msg_free_fn);
	if (ret) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"ipa_send_msg(Evt:%d)-fail=%d",
			meta.msg_type,  ret);
		qdf_mem_free(ipa_msg);
	}
	return ret;
}

/**
 * hdd_ipa_uc_register_uc_ready() - Register UC ready callback function to IPA
 * @hdd_ipa: HDD IPA local context
 *
 * Register IPA UC ready callback function to IPA kernel driver
 * Even IPA UC loaded later than WLAN kernel driver, WLAN kernel driver will
 * open WDI pipe after WLAN driver loading finished
 *
 * Return: 0 Success
 *         -EPERM Registration fail
 */
static int hdd_ipa_uc_register_uc_ready(struct hdd_ipa_priv *hdd_ipa)
{
	struct ipa_wdi_uc_ready_params uc_ready_param;
	int ret = 0;

	hdd_ipa->uc_loaded = false;
	uc_ready_param.priv = (void *)hdd_ipa;
	uc_ready_param.notify = hdd_ipa_uc_loaded_uc_cb;
	if (ipa_uc_reg_rdyCB(&uc_ready_param)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"UC Ready CB register fail");
		return -EPERM;
	}
	if (true == uc_ready_param.is_uC_ready) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "UC Ready");
		hdd_ipa->uc_loaded = true;
	} else {
		ret = hdd_ipa_uc_send_wdi_control_msg(false);
	}

	return ret;
}
#else
static void hdd_ipa_uc_get_db_paddr(qdf_dma_addr_t *db_paddr,
					  enum ipa_client_type client)
{
	/* Do nothing */
}

static int hdd_ipa_uc_register_uc_ready(struct hdd_ipa_priv *hdd_ipa)
{
	hdd_ipa->uc_loaded = true;
	return 0;
}

static int hdd_ipa_uc_send_wdi_control_msg(bool ctrl)
{
	return 0;
}
#endif

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
 * hdd_ipa_uc_sta_reset_sta_connected() - Reset sta_connected flag
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: None
 */
static inline void hdd_ipa_uc_sta_reset_sta_connected(
		struct hdd_ipa_priv *hdd_ipa)
{
	qdf_mutex_acquire(&hdd_ipa->ipa_lock);
	hdd_ipa->sta_connected = 0;
	qdf_mutex_release(&hdd_ipa->ipa_lock);
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
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
			"%s: IPA UC is not enabled", __func__);
		return;
	}

	hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;

	qdf_mutex_acquire(&hdd_ipa->rt_debug_lock);
	dump_info = &hdd_ipa->rt_bug_buffer[
		hdd_ipa->rt_buf_fill_index % HDD_IPA_UC_RT_DEBUG_BUF_COUNT];

	dump_info->time = (uint64_t)qdf_mc_timer_get_system_time();
	dump_info->ipa_excep_count = hdd_ipa->stats.num_rx_excep;
	dump_info->rx_drop_count = hdd_ipa->ipa_rx_internel_drop_count;
	dump_info->net_sent_count = hdd_ipa->ipa_rx_net_send_count;
	dump_info->tx_fwd_count = hdd_ipa->ipa_tx_forward;
	dump_info->tx_fwd_ok_count = hdd_ipa->stats.num_tx_fwd_ok;
	dump_info->rx_discard_count = hdd_ipa->ipa_rx_discard;
	dump_info->rx_destructor_call = hdd_ipa->ipa_rx_destructor_count;
	hdd_ipa->rt_buf_fill_index++;
	qdf_mutex_release(&hdd_ipa->rt_debug_lock);

	qdf_mc_timer_start(&hdd_ipa->rt_debug_fill_timer,
		HDD_IPA_UC_RT_DEBUG_FILL_INTERVAL);
}

/**
 * __hdd_ipa_uc_rt_debug_host_dump - dump rt debug buffer
 * @hdd_ctx: pointer to hdd context.
 *
 * If rt debug enabled, dump debug buffer contents based on requirement
 *
 * Return: none
 */
static void __hdd_ipa_uc_rt_debug_host_dump(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa;
	unsigned int dump_count;
	unsigned int dump_index;
	struct uc_rt_debug_info *dump_info = NULL;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	hdd_ipa = hdd_ctx->hdd_ipa;
	if (!hdd_ipa || !hdd_ipa_uc_is_enabled(hdd_ctx)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
			"%s: IPA UC is not enabled", __func__);
		return;
	}

	HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
		"========= WLAN-IPA DEBUG BUF DUMP ==========\n");
	HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
		"     TM     :   EXEP   :   DROP   :   NETS   :   FWOK   :   TXFD   :   DSTR   :   DSCD\n");

	qdf_mutex_acquire(&hdd_ipa->rt_debug_lock);
	for (dump_count = 0;
		dump_count < HDD_IPA_UC_RT_DEBUG_BUF_COUNT;
		dump_count++) {
		dump_index = (hdd_ipa->rt_buf_fill_index + dump_count) %
			HDD_IPA_UC_RT_DEBUG_BUF_COUNT;
		dump_info = &hdd_ipa->rt_bug_buffer[dump_index];
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"%12llu:%10llu:%10llu:%10llu:%10llu:%10llu:%10llu:%10llu\n",
			dump_info->time, dump_info->ipa_excep_count,
			dump_info->rx_drop_count, dump_info->net_sent_count,
			dump_info->tx_fwd_ok_count, dump_info->tx_fwd_count,
			dump_info->rx_destructor_call,
			dump_info->rx_discard_count);
	}
	qdf_mutex_release(&hdd_ipa->rt_debug_lock);
	HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
		"======= WLAN-IPA DEBUG BUF DUMP END ========\n");
}

/**
 * hdd_ipa_uc_rt_debug_host_dump - SSR wrapper for
 * __hdd_ipa_uc_rt_debug_host_dump
 * @hdd_ctx: pointer to hdd context.
 *
 * If rt debug enabled, dump debug buffer contents based on requirement
 *
 * Return: none
 */
void hdd_ipa_uc_rt_debug_host_dump(hdd_context_t *hdd_ctx)
{
	cds_ssr_protect(__func__);
	__hdd_ipa_uc_rt_debug_host_dump(hdd_ctx);
	cds_ssr_unprotect(__func__);
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
	struct hdd_ipa_priv *hdd_ipa;
	void *dummy_ptr = NULL;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;

	if (!hdd_ipa_is_rt_debugging_enabled(hdd_ctx)) {
		hdd_debug("IPA RT debug is not enabled");
		return;
	}

	/* Allocate dummy buffer periodically and free immediately. this will
	 * proactively detect OOM and if allocation fails dump ipa stats
	 */
	dummy_ptr = kmalloc(HDD_IPA_UC_DEBUG_DUMMY_MEM_SIZE,
		GFP_KERNEL | GFP_ATOMIC);
	if (!dummy_ptr) {
		hdd_ipa_uc_rt_debug_host_dump(hdd_ctx);
		hdd_ipa_uc_stat_request(
			hdd_get_adapter(hdd_ctx, QDF_SAP_MODE),
			HDD_IPA_UC_STAT_REASON_DEBUG);
	} else {
		kfree(dummy_ptr);
	}

	qdf_mc_timer_start(&hdd_ipa->rt_debug_timer,
		HDD_IPA_UC_RT_DEBUG_PERIOD);
}

/**
 * hdd_ipa_uc_rt_debug_destructor() - called by data packet free
 * @skb: packet pinter
 *
 * when free data packet, will be invoked by wlan client and will increase
 * free counter
 *
 * Return: none
 */
static void hdd_ipa_uc_rt_debug_destructor(struct sk_buff *skb)
{
	if (!ghdd_ipa) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"%s: invalid hdd context", __func__);
		return;
	}

	ghdd_ipa->ipa_rx_destructor_count++;
}

/**
 * hdd_ipa_uc_rt_debug_deinit() - remove resources to handle rt debugging
 * @hdd_ctx: hdd main context
 *
 * free all rt debugging resources
 *
 * Return: none
 */
static void hdd_ipa_uc_rt_debug_deinit(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;

	qdf_mutex_destroy(&hdd_ipa->rt_debug_lock);

	if (!hdd_ipa_is_rt_debugging_enabled(hdd_ctx)) {
		hdd_debug("IPA RT debug is not enabled");
		return;
	}

	if (QDF_TIMER_STATE_STOPPED !=
		qdf_mc_timer_get_current_state(&hdd_ipa->rt_debug_fill_timer)) {
		qdf_mc_timer_stop(&hdd_ipa->rt_debug_fill_timer);
	}
	qdf_mc_timer_destroy(&hdd_ipa->rt_debug_fill_timer);

	if (QDF_TIMER_STATE_STOPPED !=
		qdf_mc_timer_get_current_state(&hdd_ipa->rt_debug_timer)) {
		qdf_mc_timer_stop(&hdd_ipa->rt_debug_timer);
	}
	qdf_mc_timer_destroy(&hdd_ipa->rt_debug_timer);
}

/**
 * hdd_ipa_uc_rt_debug_init() - intialize resources to handle rt debugging
 * @hdd_ctx: hdd main context
 *
 * alloc and initialize all rt debugging resources
 *
 * Return: none
 */
static void hdd_ipa_uc_rt_debug_init(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;

	qdf_mutex_create(&hdd_ipa->rt_debug_lock);
	hdd_ipa->rt_buf_fill_index = 0;
	qdf_mem_zero(hdd_ipa->rt_bug_buffer,
		sizeof(struct uc_rt_debug_info) *
		HDD_IPA_UC_RT_DEBUG_BUF_COUNT);
	hdd_ipa->ipa_tx_forward = 0;
	hdd_ipa->ipa_rx_discard = 0;
	hdd_ipa->ipa_rx_net_send_count = 0;
	hdd_ipa->ipa_rx_internel_drop_count = 0;
	hdd_ipa->ipa_rx_destructor_count = 0;

	/* Reatime debug enable on feature enable */
	if (!hdd_ipa_is_rt_debugging_enabled(hdd_ctx)) {
		hdd_debug("IPA RT debug is not enabled");
		return;
	}

	qdf_mc_timer_init(&hdd_ipa->rt_debug_fill_timer, QDF_TIMER_TYPE_SW,
		hdd_ipa_uc_rt_debug_host_fill, (void *)hdd_ctx);
	qdf_mc_timer_start(&hdd_ipa->rt_debug_fill_timer,
		HDD_IPA_UC_RT_DEBUG_FILL_INTERVAL);

	qdf_mc_timer_init(&hdd_ipa->rt_debug_timer, QDF_TIMER_TYPE_SW,
		hdd_ipa_uc_rt_debug_handler, (void *)hdd_ctx);
	qdf_mc_timer_start(&hdd_ipa->rt_debug_timer,
		HDD_IPA_UC_RT_DEBUG_PERIOD);

}

/**
 * hdd_ipa_dump_hdd_ipa() - dump entries in HDD IPA struct
 * @hdd_ipa: HDD IPA struct
 *
 * Dump entries in struct hdd_ipa
 *
 * Return: none
 */
static void hdd_ipa_dump_hdd_ipa(struct hdd_ipa_priv *hdd_ipa)
{
	int i;

	/* HDD IPA */
	hdd_info("==== HDD IPA ====\n"
		"num_iface: %d\n"
		"rm_state: %d\n"
		"rm_lock: %p\n"
		"uc_rm_work: %p\n"
		"uc_op_work: %p\n"
		"wake_lock: %p\n"
		"wake_lock_work: %p\n"
		"wake_lock_released: %d\n"
		"prod_client: %d\n"
		"tx_ref_cnt: %d\n"
		"pm_queue_head----\n"
		"\thead: %p\n"
		"\ttail: %p\n"
		"\tqlen: %d\n"
		"pm_work: %p\n"
		"pm_lock: %p\n"
		"suspended: %d\n",
		hdd_ipa->num_iface,
		hdd_ipa->rm_state,
		&hdd_ipa->rm_lock,
		&hdd_ipa->uc_rm_work,
		&hdd_ipa->uc_op_work,
		&hdd_ipa->wake_lock,
		&hdd_ipa->wake_lock_work,
		hdd_ipa->wake_lock_released,
		hdd_ipa->prod_client,
		hdd_ipa->tx_ref_cnt.counter,
		hdd_ipa->pm_queue_head.head,
		hdd_ipa->pm_queue_head.tail,
		hdd_ipa->pm_queue_head.qlen,
		&hdd_ipa->pm_work,
		&hdd_ipa->pm_lock,
		hdd_ipa->suspended);
	hdd_err("\nq_lock: %p\n"
		"pend_desc_head----\n"
		"\tnext: %p\n"
		"\tprev: %p\n"
		"hdd_ctx: %p\n"
		"debugfs_dir: %p\n"
		"stats: %p\n"
		"ipv4_notifier: %p\n"
		"curr_prod_bw: %d\n"
		"curr_cons_bw: %d\n"
		"activated_fw_pipe: %d\n"
		"sap_num_connected_sta: %d\n"
		"sta_connected: %d\n",
		&hdd_ipa->q_lock,
		hdd_ipa->pend_desc_head.next,
		hdd_ipa->pend_desc_head.prev,
		hdd_ipa->hdd_ctx,
		hdd_ipa->debugfs_dir,
		&hdd_ipa->stats,
		&hdd_ipa->ipv4_notifier,
		hdd_ipa->curr_prod_bw,
		hdd_ipa->curr_cons_bw,
		hdd_ipa->activated_fw_pipe,
		hdd_ipa->sap_num_connected_sta,
		(unsigned int)hdd_ipa->sta_connected
				);
	hdd_info("\ntx_pipe_handle: 0x%x\n"
		"rx_pipe_handle: 0x%x\n"
		"resource_loading: %d\n"
		"resource_unloading: %d\n"
		"pending_cons_req: %d\n"
		"pending_event----\n"
		"\tanchor.next: %p\n"
		"\tanchor.prev: %p\n"
		"\tcount: %d\n"
		"\tmax_size: %d\n"
		"event_lock: %p\n"
		"ipa_tx_packets_diff: %d\n"
		"ipa_rx_packets_diff: %d\n"
		"ipa_p_tx_packets: %d\n"
		"ipa_p_rx_packets: %d\n"
		"stat_req_reason: %d\n",
		hdd_ipa->tx_pipe_handle,
		hdd_ipa->rx_pipe_handle,
		hdd_ipa->resource_loading,
		hdd_ipa->resource_unloading,
		hdd_ipa->pending_cons_req,
		hdd_ipa->pending_event.anchor.next,
		hdd_ipa->pending_event.anchor.prev,
		hdd_ipa->pending_event.count,
		hdd_ipa->pending_event.max_size,
		&hdd_ipa->event_lock,
		hdd_ipa->ipa_tx_packets_diff,
		hdd_ipa->ipa_rx_packets_diff,
		hdd_ipa->ipa_p_tx_packets,
		hdd_ipa->ipa_p_rx_packets,
		hdd_ipa->stat_req_reason);

	hdd_info("assoc_stas_map([id]is_reserved/sta_id): ");
	for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
		hdd_info(" [%d]%d/%d", i,
			hdd_ipa->assoc_stas_map[i].is_reserved,
			hdd_ipa->assoc_stas_map[i].sta_id);
	}
}

/**
 * hdd_ipa_dump_sys_pipe() - dump HDD IPA SYS Pipe struct
 * @hdd_ipa: HDD IPA struct
 *
 * Dump entire struct hdd_ipa_sys_pipe
 *
 * Return: none
 */
static void hdd_ipa_dump_sys_pipe(struct hdd_ipa_priv *hdd_ipa)
{
	int i;

	/* IPA SYS Pipes */
	hdd_info("==== IPA SYS Pipes ====\n");

	for (i = 0; i < HDD_IPA_MAX_SYSBAM_PIPE; i++) {
		struct hdd_ipa_sys_pipe *sys_pipe;
		struct ipa_sys_connect_params *ipa_sys_params;

		sys_pipe = &hdd_ipa->sys_pipe[i];
		ipa_sys_params = &sys_pipe->ipa_sys_params;

		hdd_info("sys_pipe[%d]----\n"
			"\tconn_hdl: 0x%x\n"
			"\tconn_hdl_valid: %d\n"
			"\tnat_en: %d\n"
			"\thdr_len %d\n"
			"\thdr_additional_const_len: %d\n"
			"\thdr_ofst_pkt_size_valid: %d\n"
			"\thdr_ofst_pkt_size: %d\n"
			"\thdr_little_endian: %d\n"
			"\tmode: %d\n"
			"\tclient: %d\n"
			"\tdesc_fifo_sz: %d\n"
			"\tpriv: %p\n"
			"\tnotify: %p\n"
			"\tskip_ep_cfg: %d\n"
			"\tkeep_ipa_awake: %d\n",
			i,
			sys_pipe->conn_hdl,
			sys_pipe->conn_hdl_valid,
			ipa_sys_params->ipa_ep_cfg.nat.nat_en,
			ipa_sys_params->ipa_ep_cfg.hdr.hdr_len,
			ipa_sys_params->ipa_ep_cfg.hdr.hdr_additional_const_len,
			ipa_sys_params->ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid,
			ipa_sys_params->ipa_ep_cfg.hdr.hdr_ofst_pkt_size,
			ipa_sys_params->ipa_ep_cfg.hdr_ext.hdr_little_endian,
			ipa_sys_params->ipa_ep_cfg.mode.mode,
			ipa_sys_params->client,
			ipa_sys_params->desc_fifo_sz,
			ipa_sys_params->priv,
			ipa_sys_params->notify,
			ipa_sys_params->skip_ep_cfg,
			ipa_sys_params->keep_ipa_awake);
	}
}

/**
 * hdd_ipa_dump_iface_context() - dump HDD IPA Interface Context struct
 * @hdd_ipa: HDD IPA struct
 *
 * Dump entire struct hdd_ipa_iface_context
 *
 * Return: none
 */
static void hdd_ipa_dump_iface_context(struct hdd_ipa_priv *hdd_ipa)
{
	int i;

	/* IPA Interface Contexts */
	hdd_info("==== IPA Interface Contexts ====\n");

	for (i = 0; i < HDD_IPA_MAX_IFACE; i++) {
		struct hdd_ipa_iface_context *iface_context;

		iface_context = &hdd_ipa->iface_context[i];

		hdd_info("iface_context[%d]----\n"
			"\thdd_ipa: %p\n"
			"\tadapter: %p\n"
			"\ttl_context: %p\n"
			"\tcons_client: %d\n"
			"\tprod_client: %d\n"
			"\tiface_id: %d\n"
			"\tsta_id: %d\n"
			"\tinterface_lock: %p\n"
			"\tifa_address: 0x%x\n",
			i,
			iface_context->hdd_ipa,
			iface_context->adapter,
			iface_context->tl_context,
			iface_context->cons_client,
			iface_context->prod_client,
			iface_context->iface_id,
			iface_context->sta_id,
			&iface_context->interface_lock,
			iface_context->ifa_address);
	}
}

/**
 * hdd_ipa_dump_info() - dump HDD IPA struct
 * @pHddCtx: hdd main context
 *
 * Dump entire struct hdd_ipa
 *
 * Return: none
 */
void hdd_ipa_dump_info(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;

	hdd_ipa_dump_hdd_ipa(hdd_ipa);
	hdd_ipa_dump_sys_pipe(hdd_ipa);
	hdd_ipa_dump_iface_context(hdd_ipa);
}

/**
 * hdd_ipa_set_tx_flow_info() - To set TX flow info if IPA is
 * enabled
 *
 * This routine is called to set TX flow info if IPA is enabled
 *
 * Return: None
 */
void hdd_ipa_set_tx_flow_info(void)
{
	hdd_adapter_list_node_t *adapterNode = NULL, *pNext = NULL;
	QDF_STATUS status;
	hdd_adapter_t *adapter;
	hdd_station_ctx_t *pHddStaCtx;
	hdd_ap_ctx_t *hdd_ap_ctx;
	hdd_hostapd_state_t *hostapd_state;
	struct qdf_mac_addr staBssid = QDF_MAC_ADDR_ZERO_INITIALIZER;
	struct qdf_mac_addr p2pBssid = QDF_MAC_ADDR_ZERO_INITIALIZER;
	struct qdf_mac_addr apBssid = QDF_MAC_ADDR_ZERO_INITIALIZER;
	uint8_t staChannel = 0, p2pChannel = 0, apChannel = 0;
	const char *p2pMode = "DEV";
	hdd_context_t *hdd_ctx;
	cds_context_type *cds_ctx;
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
	uint8_t targetChannel = 0;
	uint8_t preAdapterChannel = 0;
	uint8_t channel24;
	uint8_t channel5;
	hdd_adapter_t *preAdapterContext = NULL;
	hdd_adapter_t *adapter2_4 = NULL;
	hdd_adapter_t *adapter5 = NULL;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
	struct wlan_objmgr_psoc *psoc;

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx) {
		cds_err("HDD context is NULL");
		return;
	}

	cds_ctx = cds_get_context(QDF_MODULE_ID_QDF);
	if (!cds_ctx) {
		cds_err("Invalid CDS Context");
		return;
	}

	psoc = hdd_ctx->hdd_psoc;
	status = hdd_get_front_adapter(hdd_ctx, &adapterNode);
	while (NULL != adapterNode && QDF_STATUS_SUCCESS == status) {
		adapter = adapterNode->pAdapter;
		switch (adapter->device_mode) {
		case QDF_STA_MODE:
			pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
			if (eConnectionState_Associated ==
			    pHddStaCtx->conn_info.connState) {
				staChannel =
					pHddStaCtx->conn_info.operationChannel;
				qdf_copy_macaddr(&staBssid,
						 &pHddStaCtx->conn_info.bssId);
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
				targetChannel = staChannel;
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
			}
			break;
		case QDF_P2P_CLIENT_MODE:
			pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
			if (eConnectionState_Associated ==
			    pHddStaCtx->conn_info.connState) {
				p2pChannel =
					pHddStaCtx->conn_info.operationChannel;
				qdf_copy_macaddr(&p2pBssid,
						&pHddStaCtx->conn_info.bssId);
				p2pMode = "CLI";
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
				targetChannel = p2pChannel;
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
			}
			break;
		case QDF_P2P_GO_MODE:
			hdd_ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
			hostapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);
			if (hostapd_state->bssState == BSS_START
			    && hostapd_state->qdf_status ==
			    QDF_STATUS_SUCCESS) {
				p2pChannel = hdd_ap_ctx->operatingChannel;
				qdf_copy_macaddr(&p2pBssid,
						 &adapter->macAddressCurrent);
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
				targetChannel = p2pChannel;
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
			}
			p2pMode = "GO";
			break;
		case QDF_SAP_MODE:
			hdd_ap_ctx = WLAN_HDD_GET_AP_CTX_PTR(adapter);
			hostapd_state = WLAN_HDD_GET_HOSTAP_STATE_PTR(adapter);
			if (hostapd_state->bssState == BSS_START
			    && hostapd_state->qdf_status ==
			    QDF_STATUS_SUCCESS) {
				apChannel = hdd_ap_ctx->operatingChannel;
				qdf_copy_macaddr(&apBssid,
						&adapter->macAddressCurrent);
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
				targetChannel = apChannel;
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
			}
			break;
		case QDF_IBSS_MODE:
		default:
			break;
		}
#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
		if (targetChannel) {
			/*
			 * This is first adapter detected as active
			 * set as default for none concurrency case
			 */
			if (!preAdapterChannel) {
				/* If IPA UC data path is enabled,
				 * target should reserve extra tx descriptors
				 * for IPA data path.
				 * Then host data path should allow less TX
				 * packet pumping in case IPA
				 * data path enabled
				 */
				if (hdd_ipa_uc_is_enabled(hdd_ctx) &&
				    (QDF_SAP_MODE == adapter->device_mode)) {
					adapter->tx_flow_low_watermark =
					hdd_ctx->config->TxFlowLowWaterMark +
					WLAN_TFC_IPAUC_TX_DESC_RESERVE;
				} else {
					adapter->tx_flow_low_watermark =
						hdd_ctx->config->
							TxFlowLowWaterMark;
				}
				adapter->tx_flow_high_watermark_offset =
				   hdd_ctx->config->TxFlowHighWaterMarkOffset;
				cdp_fc_ll_set_tx_pause_q_depth(soc,
					adapter->sessionId,
					hdd_ctx->config->TxFlowMaxQueueDepth);
				cds_info("MODE %d,CH %d,LWM %d,HWM %d,TXQDEP %d",
				    adapter->device_mode,
				    targetChannel,
				    adapter->tx_flow_low_watermark,
				    adapter->tx_flow_low_watermark +
				    adapter->tx_flow_high_watermark_offset,
				    hdd_ctx->config->TxFlowMaxQueueDepth);
				preAdapterChannel = targetChannel;
				preAdapterContext = adapter;
			} else {
				/*
				 * SCC, disable TX flow control for both
				 * SCC each adapter cannot reserve dedicated
				 * channel resource, as a result, if any adapter
				 * blocked OS Q by flow control,
				 * blocked adapter will lost chance to recover
				 */
				if (preAdapterChannel == targetChannel) {
					/* Current adapter */
					adapter->tx_flow_low_watermark = 0;
					adapter->
					tx_flow_high_watermark_offset = 0;
					cdp_fc_ll_set_tx_pause_q_depth(soc,
						adapter->sessionId,
						hdd_ctx->config->
						TxHbwFlowMaxQueueDepth);
					cds_info("SCC: MODE %s(%d), CH %d, LWM %d, HWM %d, TXQDEP %d",
					       hdd_device_mode_to_string(
							adapter->device_mode),
					       adapter->device_mode,
					       targetChannel,
					       adapter->tx_flow_low_watermark,
					       adapter->tx_flow_low_watermark +
					       adapter->
					       tx_flow_high_watermark_offset,
					       hdd_ctx->config->
					       TxHbwFlowMaxQueueDepth);

					if (!preAdapterContext) {
						cds_err("SCC: Previous adapter context NULL");
						continue;
					}

					/* Previous adapter */
					preAdapterContext->
					tx_flow_low_watermark = 0;
					preAdapterContext->
					tx_flow_high_watermark_offset = 0;
					cdp_fc_ll_set_tx_pause_q_depth(soc,
						preAdapterContext->sessionId,
						hdd_ctx->config->
						TxHbwFlowMaxQueueDepth);
					cds_info("SCC: MODE %s(%d), CH %d, LWM %d, HWM %d, TXQDEP %d",
					       hdd_device_mode_to_string(
						preAdapterContext->device_mode
							  ),
					       preAdapterContext->device_mode,
					       targetChannel,
					       preAdapterContext->
					       tx_flow_low_watermark,
					       preAdapterContext->
					       tx_flow_low_watermark +
					       preAdapterContext->
					       tx_flow_high_watermark_offset,
					       hdd_ctx->config->
					       TxHbwFlowMaxQueueDepth);
				}
				/*
				 * MCC, each adapter will have dedicated
				 * resource
				 */
				else {
					/* current channel is 2.4 */
					if (targetChannel <=
				     WLAN_HDD_TX_FLOW_CONTROL_MAX_24BAND_CH) {
						channel24 = targetChannel;
						channel5 = preAdapterChannel;
						adapter2_4 = adapter;
						adapter5 = preAdapterContext;
					} else {
						/* Current channel is 5 */
						channel24 = preAdapterChannel;
						channel5 = targetChannel;
						adapter2_4 = preAdapterContext;
						adapter5 = adapter;
					}

					if (!adapter5) {
						cds_err("MCC: 5GHz adapter context NULL");
						continue;
					}
					adapter5->tx_flow_low_watermark =
						hdd_ctx->config->
						TxHbwFlowLowWaterMark;
					adapter5->
					tx_flow_high_watermark_offset =
						hdd_ctx->config->
						TxHbwFlowHighWaterMarkOffset;
					cdp_fc_ll_set_tx_pause_q_depth(soc,
						adapter5->sessionId,
						hdd_ctx->config->
						TxHbwFlowMaxQueueDepth);
					cds_info("MCC: MODE %s(%d), CH %d, LWM %d, HWM %d, TXQDEP %d",
					    hdd_device_mode_to_string(
						    adapter5->device_mode),
					    adapter5->device_mode,
					    channel5,
					    adapter5->tx_flow_low_watermark,
					    adapter5->
					    tx_flow_low_watermark +
					    adapter5->
					    tx_flow_high_watermark_offset,
					    hdd_ctx->config->
					    TxHbwFlowMaxQueueDepth);

					if (!adapter2_4) {
						cds_err("MCC: 2.4GHz adapter context NULL");
						continue;
					}
					adapter2_4->tx_flow_low_watermark =
						hdd_ctx->config->
						TxLbwFlowLowWaterMark;
					adapter2_4->
					tx_flow_high_watermark_offset =
						hdd_ctx->config->
						TxLbwFlowHighWaterMarkOffset;
					cdp_fc_ll_set_tx_pause_q_depth(soc,
						adapter2_4->sessionId,
						hdd_ctx->config->
						TxLbwFlowMaxQueueDepth);
					cds_info("MCC: MODE %s(%d), CH %d, LWM %d, HWM %d, TXQDEP %d",
						hdd_device_mode_to_string(
						    adapter2_4->device_mode),
						adapter2_4->device_mode,
						channel24,
						adapter2_4->
						tx_flow_low_watermark,
						adapter2_4->
						tx_flow_low_watermark +
						adapter2_4->
						tx_flow_high_watermark_offset,
						hdd_ctx->config->
						TxLbwFlowMaxQueueDepth);

				}
			}
		}
		targetChannel = 0;
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
		status = hdd_get_next_adapter(hdd_ctx, adapterNode, &pNext);
		adapterNode = pNext;
	}
	hdd_ctx->mcc_mode = policy_mgr_current_concurrency_is_mcc(psoc);
}

/**
 * __hdd_ipa_uc_stat_query() - Query the IPA stats
 * @hdd_ctx: Global HDD context
 * @ipa_tx_diff: tx packet count diff from previous tx packet count
 * @ipa_rx_diff: rx packet count diff from previous rx packet count
 *
 * Return: true if IPA is enabled, false otherwise
 */
static void __hdd_ipa_uc_stat_query(hdd_context_t *hdd_ctx,
	uint32_t *ipa_tx_diff, uint32_t *ipa_rx_diff)
{
	struct hdd_ipa_priv *hdd_ipa;

	*ipa_tx_diff = 0;
	*ipa_rx_diff = 0;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;

	if (!hdd_ipa_is_enabled(hdd_ctx) ||
	    !(hdd_ipa_uc_is_enabled(hdd_ctx))) {
		return;
	}

	qdf_mutex_acquire(&hdd_ipa->ipa_lock);
	if ((HDD_IPA_UC_NUM_WDI_PIPE == hdd_ipa->activated_fw_pipe) &&
		(false == hdd_ipa->resource_loading)) {
		*ipa_tx_diff = hdd_ipa->ipa_tx_packets_diff;
		*ipa_rx_diff = hdd_ipa->ipa_rx_packets_diff;
		hdd_debug("STAT Query TX DIFF %d, RX DIFF %d",
			    *ipa_tx_diff, *ipa_rx_diff);
	}
	qdf_mutex_release(&hdd_ipa->ipa_lock);
}

/**
 * hdd_ipa_uc_stat_query() - SSR wrapper for __hdd_ipa_uc_stat_query
 * @hdd_ctx: Global HDD context
 * @ipa_tx_diff: tx packet count diff from previous tx packet count
 * @ipa_rx_diff: rx packet count diff from previous rx packet count
 *
 * Return: true if IPA is enabled, false otherwise
 */
void hdd_ipa_uc_stat_query(hdd_context_t *hdd_ctx,
	uint32_t *ipa_tx_diff, uint32_t *ipa_rx_diff)
{
	cds_ssr_protect(__func__);
	__hdd_ipa_uc_stat_query(hdd_ctx, ipa_tx_diff, ipa_rx_diff);
	cds_ssr_unprotect(__func__);
}

/**
 * __hdd_ipa_uc_stat_request() - Get IPA stats from IPA.
 * @adapter: network adapter
 * @reason: STAT REQ Reason
 *
 * Return: None
 */
static void __hdd_ipa_uc_stat_request(hdd_adapter_t *adapter, uint8_t reason)
{
	hdd_context_t *hdd_ctx;
	struct hdd_ipa_priv *hdd_ipa;

	if (!adapter)
		return;

	hdd_ctx = (hdd_context_t *)adapter->pHddCtx;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;
	if (!hdd_ipa_is_enabled(hdd_ctx) ||
	    !(hdd_ipa_uc_is_enabled(hdd_ctx))) {
		return;
	}

	hdd_debug("STAT REQ Reason %d", reason);
	qdf_mutex_acquire(&hdd_ipa->ipa_lock);
	if ((HDD_IPA_UC_NUM_WDI_PIPE == hdd_ipa->activated_fw_pipe) &&
		(false == hdd_ipa->resource_loading)) {
		hdd_ipa->stat_req_reason = reason;
		qdf_mutex_release(&hdd_ipa->ipa_lock);
		wma_cli_set_command(
			(int)adapter->sessionId,
			(int)WMA_VDEV_TXRX_GET_IPA_UC_FW_STATS_CMDID,
			0, VDEV_CMD);
	} else {
		qdf_mutex_release(&hdd_ipa->ipa_lock);
	}
}

/**
 * hdd_ipa_uc_stat_request() - SSR wrapper for __hdd_ipa_uc_stat_request
 * @adapter: network adapter
 * @reason: STAT REQ Reason
 *
 * Return: None
 */
void hdd_ipa_uc_stat_request(hdd_adapter_t *adapter, uint8_t reason)
{
	cds_ssr_protect(__func__);
	__hdd_ipa_uc_stat_request(adapter, reason);
	cds_ssr_unprotect(__func__);
}

#ifdef FEATURE_METERING
/**
 * hdd_ipa_uc_sharing_stats_request() - Get IPA stats from IPA.
 * @adapter: network adapter
 * @reset_stats: reset stat countis after response
 *
 * Return: None
 */
void hdd_ipa_uc_sharing_stats_request(hdd_adapter_t *adapter,
				      uint8_t reset_stats)
{
	hdd_context_t *pHddCtx;
	struct hdd_ipa_priv *hdd_ipa;

	if (!adapter)
		return;

	pHddCtx = adapter->pHddCtx;
	hdd_ipa = pHddCtx->hdd_ipa;
	if (!hdd_ipa_is_enabled(pHddCtx) ||
		!(hdd_ipa_uc_is_enabled(pHddCtx))) {
		return;
	}

	HDD_IPA_LOG(LOG1, "SHARING_STATS: reset_stats=%d", reset_stats);
	qdf_mutex_acquire(&hdd_ipa->ipa_lock);
	if (false == hdd_ipa->resource_loading) {
		qdf_mutex_release(&hdd_ipa->ipa_lock);
		wma_cli_set_command(
			(int)adapter->sessionId,
			(int)WMA_VDEV_TXRX_GET_IPA_UC_SHARING_STATS_CMDID,
			reset_stats, VDEV_CMD);
	} else {
		qdf_mutex_release(&hdd_ipa->ipa_lock);
	}
}

/**
 * hdd_ipa_uc_set_quota() - Set quota limit bytes from IPA.
 * @adapter: network adapter
 * @set_quota: when 1, FW starts quota monitoring
 * @quota_bytes: quota limit in bytes
 *
 * Return: None
 */
void hdd_ipa_uc_set_quota(hdd_adapter_t *adapter, uint8_t set_quota,
			  uint64_t quota_bytes)
{
	hdd_context_t *pHddCtx;
	struct hdd_ipa_priv *hdd_ipa;

	if (!adapter)
		return;

	pHddCtx = adapter->pHddCtx;
	hdd_ipa = pHddCtx->hdd_ipa;
	if (!hdd_ipa_is_enabled(pHddCtx) ||
		!(hdd_ipa_uc_is_enabled(pHddCtx))) {
		return;
	}

	HDD_IPA_LOG(LOG1, "SET_QUOTA: set_quota=%d, quota_bytes=%llu",
		    set_quota, quota_bytes);

	qdf_mutex_acquire(&hdd_ipa->ipa_lock);
	if (false == hdd_ipa->resource_loading) {
		qdf_mutex_release(&hdd_ipa->ipa_lock);
		wma_cli_set2_command(
			(int)adapter->sessionId,
			(int)WMA_VDEV_TXRX_SET_IPA_UC_QUOTA_CMDID,
			(set_quota ? quota_bytes&0xffffffff : 0),
			(set_quota ? quota_bytes>>32 : 0),
			VDEV_CMD);
	} else {
		qdf_mutex_release(&hdd_ipa->ipa_lock);
	}
}
#endif

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
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	/* ACTIVATE TX PIPE */
	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
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
	cdp_ipa_set_active(soc,
			(struct cdp_pdev *)cds_ctx->pdev_txrx_ctx,
			true, true);

	/* ACTIVATE RX PIPE */
	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
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
	cdp_ipa_set_active(soc,
		(struct cdp_pdev *)cds_ctx->pdev_txrx_ctx,
		true, false);
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

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s: Disable RX PIPE", __func__);
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

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s: Disable TX PIPE", __func__);
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
	 * PROD resource may return sync or async manners
	 */
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

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
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
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (!cds_ctx || !cds_ctx->pdev_txrx_ctx) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "txrx context is NULL");
		QDF_ASSERT(0);
		return;
	}

	hdd_ipa->resource_unloading = true;
	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s: Disable FW RX PIPE", __func__);
	cdp_ipa_set_active(soc,
		(struct cdp_pdev *)cds_ctx->pdev_txrx_ctx,
		false, false);
	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s: Disable FW TX PIPE", __func__);
	cdp_ipa_set_active(soc,
		(struct cdp_pdev *)cds_ctx->pdev_txrx_ctx,
		false, true);
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

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s, event code %d",
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
	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
		"%s, posted event %d", __func__, event);

	hdd_ipa_uc_rm_notify_handler(hdd_ipa, event);
	cds_ssr_unprotect(__func__);
}

/**
 * hdd_ipa_uc_loaded_handler() - Process IPA uC loaded indication
 * @ipa_ctxt: hdd ipa local context
 *
 * Will handle IPA UC image loaded indication comes from IPA kernel
 *
 * Return: None
 */
static void hdd_ipa_uc_loaded_handler(struct hdd_ipa_priv *ipa_ctxt)
{
	struct ipa_wdi_out_params pipe_out;

	HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "%s : UC READY", __func__);
	if (true == ipa_ctxt->uc_loaded) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s : UC already loaded",
			 __func__);
		return;
	}

	ipa_ctxt->uc_loaded = true;
	/* Connect pipe */
	ipa_connect_wdi_pipe(&ipa_ctxt->cons_pipe_in, &pipe_out);
	ipa_ctxt->tx_pipe_handle = pipe_out.clnt_hdl;
	ipa_ctxt->tx_comp_doorbell_paddr = pipe_out.uc_door_bell_pa;
	HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
		"%s : TX PIPE Handle %d, DBPA 0x%llx",
		__func__, ipa_ctxt->tx_pipe_handle,
		(unsigned long long) pipe_out.uc_door_bell_pa);

	ipa_connect_wdi_pipe(&ipa_ctxt->prod_pipe_in, &pipe_out);
	ipa_ctxt->rx_pipe_handle = pipe_out.clnt_hdl;
	ipa_ctxt->rx_ready_doorbell_paddr = pipe_out.uc_door_bell_pa;
	HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
		"%s : RX PIPE Handle %d, DBPA 0x%llx",
		__func__, ipa_ctxt->rx_pipe_handle,
		(unsigned long long) pipe_out.uc_door_bell_pa);

	/* If already any STA connected, enable IPA/FW PIPEs */
	if (ipa_ctxt->sap_num_connected_sta) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
			"Client already connected, enable IPA/FW PIPEs");
		hdd_ipa_uc_handle_first_con(ipa_ctxt);
	}
}

/**
 * hdd_ipa_uc_op_metering() - IPA uC operation for stats and quota limit
 * @hdd_ctx: Global HDD context
 * @op_msg: operation message received from firmware
 *
 * Return: QDF_STATUS enumeration
 */
#ifdef FEATURE_METERING
static QDF_STATUS hdd_ipa_uc_op_metering(hdd_context_t *hdd_ctx,
					 struct op_msg_type *op_msg)
{
	struct op_msg_type *msg = op_msg;
	struct ipa_uc_sharing_stats *uc_sharing_stats;
	struct ipa_uc_quota_rsp *uc_quota_rsp;
	struct ipa_uc_quota_ind *uc_quota_ind;
	struct hdd_ipa_priv *hdd_ipa;
	hdd_adapter_t *adapter;

	hdd_ipa = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;

	if (HDD_IPA_UC_OPCODE_SHARING_STATS == msg->op_code) {
		/* fill-up ipa_uc_sharing_stats structure from FW */
		uc_sharing_stats = (struct ipa_uc_sharing_stats *)
			     ((uint8_t *)op_msg + sizeof(struct op_msg_type));

		memcpy(&(hdd_ipa->ipa_sharing_stats), uc_sharing_stats,
		       sizeof(struct ipa_uc_sharing_stats));

		complete(&hdd_ipa->ipa_uc_sharing_stats_comp);

		HDD_IPA_DP_LOG(QDF_TRACE_LEVEL_DEBUG,
			       "%s: %llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu",
			       "HDD_IPA_UC_OPCODE_SHARING_STATS",
			       hdd_ipa->ipa_sharing_stats.ipv4_rx_packets,
			       hdd_ipa->ipa_sharing_stats.ipv4_rx_bytes,
			       hdd_ipa->ipa_sharing_stats.ipv6_rx_packets,
			       hdd_ipa->ipa_sharing_stats.ipv6_rx_bytes,
			       hdd_ipa->ipa_sharing_stats.ipv4_tx_packets,
			       hdd_ipa->ipa_sharing_stats.ipv4_tx_bytes,
			       hdd_ipa->ipa_sharing_stats.ipv6_tx_packets,
			       hdd_ipa->ipa_sharing_stats.ipv6_tx_bytes);
	} else if (HDD_IPA_UC_OPCODE_QUOTA_RSP == msg->op_code) {
		/* received set quota response */
		uc_quota_rsp = (struct ipa_uc_quota_rsp *)
			     ((uint8_t *)op_msg + sizeof(struct op_msg_type));

		memcpy(&(hdd_ipa->ipa_quota_rsp), uc_quota_rsp,
			   sizeof(struct ipa_uc_quota_rsp));

		complete(&hdd_ipa->ipa_uc_set_quota_comp);
		HDD_IPA_DP_LOG(QDF_TRACE_LEVEL_DEBUG,
			      "%s: success=%d, quota_bytes=%llu",
			      "HDD_IPA_UC_OPCODE_QUOTA_RSP",
			      hdd_ipa->ipa_quota_rsp.success,
			      ((uint64_t)(hdd_ipa->ipa_quota_rsp.quota_hi)<<32)|
			      hdd_ipa->ipa_quota_rsp.quota_lo);
	} else if (HDD_IPA_UC_OPCODE_QUOTA_IND == msg->op_code) {
		/* hit quota limit */
		uc_quota_ind = (struct ipa_uc_quota_ind *)
			     ((uint8_t *)op_msg + sizeof(struct op_msg_type));

		hdd_ipa->ipa_quota_ind.quota_bytes =
					uc_quota_ind->quota_bytes;

		/* send quota exceeded indication to IPA */
		HDD_IPA_DP_LOG(QDF_TRACE_LEVEL_DEBUG,
			"OPCODE_QUOTA_IND: quota exceed! (quota_bytes=%llu)",
			hdd_ipa->ipa_quota_ind.quota_bytes);

		adapter = hdd_get_adapter(hdd_ipa->hdd_ctx, QDF_STA_MODE);
		if (adapter)
			ipa_broadcast_wdi_quota_reach_ind(
						adapter->dev->ifindex,
						uc_quota_ind->quota_bytes);
		else
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
					"Failed quota_reach_ind: NULL adapter");
	} else {
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}
#else
static QDF_STATUS hdd_ipa_uc_op_metering(hdd_context_t *hdd_ctx,
					 struct op_msg_type *op_msg)
{
	return QDF_STATUS_E_INVAL;
}
#endif

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

	HDD_IPA_DP_LOG(QDF_TRACE_LEVEL_DEBUG,
		       "OPCODE=%d", msg->op_code);

	if ((HDD_IPA_UC_OPCODE_TX_RESUME == msg->op_code) ||
	    (HDD_IPA_UC_OPCODE_RX_RESUME == msg->op_code)) {
		qdf_mutex_acquire(&hdd_ipa->ipa_lock);
		hdd_ipa->activated_fw_pipe++;
		if (HDD_IPA_UC_NUM_WDI_PIPE == hdd_ipa->activated_fw_pipe) {
			hdd_ipa->resource_loading = false;
			if (hdd_ipa->wdi_enabled == false) {
				hdd_ipa->wdi_enabled = true;
				if (hdd_ipa_uc_send_wdi_control_msg(true) == 0)
					hdd_ipa_send_mcc_scc_msg(hdd_ctx,
							 hdd_ctx->mcc_mode);
			}
			hdd_ipa_uc_proc_pending_event(hdd_ipa);
			if (hdd_ipa->pending_cons_req)
				ipa_rm_notify_completion(
						IPA_RM_RESOURCE_GRANTED,
						IPA_RM_RESOURCE_WLAN_CONS);
			hdd_ipa->pending_cons_req = false;
		}
		qdf_mutex_release(&hdd_ipa->ipa_lock);
	} else if ((HDD_IPA_UC_OPCODE_TX_SUSPEND == msg->op_code) ||
	    (HDD_IPA_UC_OPCODE_RX_SUSPEND == msg->op_code)) {
		qdf_mutex_acquire(&hdd_ipa->ipa_lock);
		hdd_ipa->activated_fw_pipe--;
		if (!hdd_ipa->activated_fw_pipe) {
			hdd_ipa_uc_disable_pipes(hdd_ipa);
			if (hdd_ipa_is_rm_enabled(hdd_ipa->hdd_ctx))
				ipa_rm_release_resource(
					IPA_RM_RESOURCE_WLAN_PROD);
			/*
			 * Sync return success from IPA
			 * Enable/resume all the PIPEs
			 */
			hdd_ipa->resource_unloading = false;
			hdd_ipa_uc_proc_pending_event(hdd_ipa);
			hdd_ipa->pending_cons_req = false;
		}
		qdf_mutex_release(&hdd_ipa->ipa_lock);
	} else if ((HDD_IPA_UC_OPCODE_STATS == msg->op_code) &&
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
			  "NUM TX FWD OK : %llu\n"
			  "NUM TX FWD ERR : %llu",
			  (unsigned long long)res->rx_rdy_ring_base_paddr,
			  res->rx_rdy_ring_size,
			  (unsigned long long)hdd_ipa->rx_ready_doorbell_paddr,
			  (unsigned long long)res->rx_proc_done_idx_paddr,
			  hdd_ipa->stats.num_rx_excep,
			  hdd_ipa->stats.num_tx_fwd_ok,
			  hdd_ipa->stats.num_tx_fwd_err);
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "==== IPA_UC WLAN_HOST CONTROL ====\n"
			  "SAP NUM STAs: %d\n"
			  "STA CONNECTED: %d\n"
			  "CONCURRENT MODE: %s\n"
			  "TX PIPE HDL: 0x%x\n"
			  "RX PIPE HDL : 0x%x\n"
			  "RSC LOADING : %d\n"
			  "RSC UNLOADING : %d\n"
			  "PNDNG CNS RQT : %d",
			  hdd_ipa->sap_num_connected_sta,
			  hdd_ipa->sta_connected,
			  (hdd_ctx->mcc_mode ? "MCC" : "SCC"),
			  hdd_ipa->tx_pipe_handle,
			  hdd_ipa->rx_pipe_handle,
			  hdd_ipa->resource_loading,
			  hdd_ipa->resource_unloading,
			  hdd_ipa->pending_cons_req);

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
			  uc_fw_stat->tx_is_suspend,
			  uc_fw_stat->tx_reserved);
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
			  uc_fw_stat->rx_is_suspend,
			  uc_fw_stat->rx_reserved);
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
	} else if (msg->op_code == HDD_IPA_UC_OPCODE_UC_READY) {
		qdf_mutex_acquire(&hdd_ipa->ipa_lock);
		hdd_ipa_uc_loaded_handler(hdd_ipa);
		qdf_mutex_release(&hdd_ipa->ipa_lock);
	} else if (hdd_ipa_uc_op_metering(hdd_ctx, op_msg)) {
		HDD_IPA_LOG(LOGE, "Invalid message: op_code=%d, reason=%d",
			    msg->op_code, hdd_ipa->stat_req_reason);
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
			uint32_t offload_type, bool enable)
{
	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;
	struct sir_ipa_offload_enable_disable ipa_offload_enable_disable;
	struct hdd_ipa_iface_context *iface_context = NULL;
	uint8_t session_id;

	if (!adapter || !hdd_ipa)
		return;

	iface_context = adapter->ipa_context;
	session_id = adapter->sessionId;

	if (!iface_context) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "Interface context is NULL");
		return;
	}

	if (enable == hdd_ipa->vdev_offload_enabled[session_id]) {
		/* IPA offload status is already set as desired */
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: (offload_type=%d, vdev_id=%d, enable=%d)",
			    "IPA offload status is already set",
			    offload_type, session_id, enable);
		return;
	}

	if (wlan_hdd_validate_session_id(adapter->sessionId)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"invalid session id: %d, offload_type=%d, enable=%d",
			adapter->sessionId, offload_type, enable);
		return;
	}

	qdf_mem_zero(&ipa_offload_enable_disable,
		sizeof(ipa_offload_enable_disable));
	ipa_offload_enable_disable.offload_type = offload_type;
	ipa_offload_enable_disable.vdev_id = session_id;
	ipa_offload_enable_disable.enable = enable;

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
		"offload_type=%d, vdev_id=%d, enable=%d",
		ipa_offload_enable_disable.offload_type,
		ipa_offload_enable_disable.vdev_id,
		ipa_offload_enable_disable.enable);

	if (QDF_STATUS_SUCCESS !=
		sme_ipa_offload_enable_disable(WLAN_HDD_GET_HAL_CTX(adapter),
			adapter->sessionId, &ipa_offload_enable_disable)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "%s: Failure to enable IPA offload (offload_type=%d, vdev_id=%d, enable=%d)",
			    __func__,
			    ipa_offload_enable_disable.offload_type,
			    ipa_offload_enable_disable.vdev_id,
			    ipa_offload_enable_disable.enable);
	} else {
		/* Update the IPA offload status */
		hdd_ipa->vdev_offload_enabled[session_id] =
			ipa_offload_enable_disable.enable;
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
	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
			"%s, posted msg %d", __func__, msg->op_code);

	hdd_ipa_uc_op_cb(msg, hdd_ipa->hdd_ctx);

	cds_ssr_unprotect(__func__);
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

#ifdef FEATURE_METERING
/**
 * __hdd_ipa_wdi_meter_notifier_cb() - WLAN to IPA callback handler.
 * IPA calls to get WLAN stats or set quota limit.
 * @priv: pointer to private data registered with IPA (we register a
 *»       pointer to the global IPA context)
 * @evt: the IPA event which triggered the callback
 * @data: data associated with the event
 *
 * Return: None
 */
static void __hdd_ipa_wdi_meter_notifier_cb(enum ipa_wdi_meter_evt_type evt,
					  void *data)
{
	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;
	hdd_adapter_t *adapter = NULL;
	struct ipa_get_wdi_sap_stats *wdi_sap_stats;
	struct ipa_set_wifi_quota *ipa_set_quota;
	int ret = 0;

	if (wlan_hdd_validate_context(hdd_ipa->hdd_ctx))
		return;

	adapter = hdd_get_adapter(hdd_ipa->hdd_ctx, QDF_STA_MODE);

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO, "event=%d", evt);

	switch (evt) {
	case IPA_GET_WDI_SAP_STATS:
		/* fill-up ipa_get_wdi_sap_stats structure after getting
		   ipa_uc_fw_stats from FW */
		wdi_sap_stats = data;

		if (!adapter) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				"IPA uC share stats failed - no adapter");
			wdi_sap_stats->stats_valid = 0;
			return;
		}

		INIT_COMPLETION(hdd_ipa->ipa_uc_sharing_stats_comp);
		INIT_COMPLETION(hdd_ipa->ipa_uc_set_quota_comp);
		hdd_ipa_uc_sharing_stats_request(adapter,
					     wdi_sap_stats->reset_stats);
		ret = wait_for_completion_timeout(
			&hdd_ipa->ipa_uc_sharing_stats_comp,
			msecs_to_jiffies(IPA_UC_SHARING_STATES_WAIT_TIME));
		if (!ret) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
					"IPA uC share stats request timed out");
			wdi_sap_stats->stats_valid = 0;
		} else {
			wdi_sap_stats->stats_valid = 1;

			wdi_sap_stats->ipv4_rx_packets =
				hdd_ipa->ipa_sharing_stats.ipv4_rx_packets;
			wdi_sap_stats->ipv4_rx_bytes =
				hdd_ipa->ipa_sharing_stats.ipv4_rx_bytes;
			wdi_sap_stats->ipv6_rx_packets =
				hdd_ipa->ipa_sharing_stats.ipv6_rx_packets;
			wdi_sap_stats->ipv6_rx_bytes =
				hdd_ipa->ipa_sharing_stats.ipv6_rx_bytes;
			wdi_sap_stats->ipv4_tx_packets =
				hdd_ipa->ipa_sharing_stats.ipv4_tx_packets;
			wdi_sap_stats->ipv4_tx_bytes =
				hdd_ipa->ipa_sharing_stats.ipv4_tx_bytes;
			wdi_sap_stats->ipv6_tx_packets =
				hdd_ipa->ipa_sharing_stats.ipv6_tx_packets;
			wdi_sap_stats->ipv6_tx_bytes =
				hdd_ipa->ipa_sharing_stats.ipv6_tx_bytes;
			HDD_IPA_DP_LOG(QDF_TRACE_LEVEL_DEBUG,
				"%s:%d,%llu,%llu,%llu,%llu,%llu,%llu,%llu,%llu",
				"IPA_GET_WDI_SAP_STATS",
				wdi_sap_stats->stats_valid,
				wdi_sap_stats->ipv4_rx_packets,
				wdi_sap_stats->ipv4_rx_bytes,
				wdi_sap_stats->ipv6_rx_packets,
				wdi_sap_stats->ipv6_rx_bytes,
				wdi_sap_stats->ipv4_tx_packets,
				wdi_sap_stats->ipv4_tx_bytes,
				wdi_sap_stats->ipv6_tx_packets,
				wdi_sap_stats->ipv6_tx_bytes);
		}
		break;
	case IPA_SET_WIFI_QUOTA:
		/* get ipa_set_wifi_quota structure from IPA and pass to FW
		   through quota_exceeded field in ipa_uc_fw_stats */
		ipa_set_quota = data;

		if (!adapter) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				"IPA uC set quota failed - no adapter");
			ipa_set_quota->set_valid = 0;
			return;
		}

		hdd_ipa_uc_set_quota(adapter, ipa_set_quota->set_quota,
				     ipa_set_quota->quota_bytes);

		ret = wait_for_completion_timeout(
				&hdd_ipa->ipa_uc_set_quota_comp,
				msecs_to_jiffies(IPA_UC_SET_QUOTA_WAIT_TIME));
		if (!ret) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
					"IPA uC set quota request timed out");
			ipa_set_quota->set_valid = 0;
		} else {
			ipa_set_quota->quota_bytes =
				((uint64_t)(hdd_ipa->ipa_quota_rsp.quota_hi)
				  <<32)|hdd_ipa->ipa_quota_rsp.quota_lo;
			ipa_set_quota->set_valid =
				hdd_ipa->ipa_quota_rsp.success;
		}

		HDD_IPA_DP_LOG(QDF_TRACE_LEVEL_DEBUG, "SET_QUOTA: %llu, %d",
			       ipa_set_quota->quota_bytes,
			       ipa_set_quota->set_valid);
		break;
	}
}

/**
 * hdd_ipa_wdi_meter_notifier_cb() - WLAN to IPA callback handler.
 * IPA calls to get WLAN stats or set quota limit.
 * @priv: pointer to private data registered with IPA (we register a
 *»       pointer to the global IPA context)
 * @evt: the IPA event which triggered the callback
 * @data: data associated with the event
 *
 * Return: None
 */
static void hdd_ipa_wdi_meter_notifier_cb(enum ipa_wdi_meter_evt_type evt,
					  void *data)
{
	cds_ssr_protect(__func__);
	__hdd_ipa_wdi_meter_notifier_cb(evt, data);
	cds_ssr_unprotect(__func__);
}

static void hdd_ipa_init_metering(struct hdd_ipa_priv *ipa_ctxt,
				  struct ipa_wdi_in_params *pipe_in)
{
	pipe_in->wdi_notify = hdd_ipa_wdi_meter_notifier_cb;

	init_completion(&ipa_ctxt->ipa_uc_sharing_stats_comp);
	init_completion(&ipa_ctxt->ipa_uc_set_quota_comp);
}
#else
static void hdd_ipa_init_metering(struct hdd_ipa_priv *ipa_ctxt,
				  struct ipa_wdi_in_params *pipe_in)
{
}
#endif

/**
 * hdd_ipa_uc_ol_init() - Initialize IPA uC offload
 * @hdd_ctx: Global HDD context
 *
 * This function is called to update IPA pipe configuration with resources
 * allocated by wlan driver (cds_pre_enable) before enabling it in FW
 * (cds_enable)
 *
 * Return: QDF_STATUS
 */
QDF_STATUS hdd_ipa_uc_ol_init(hdd_context_t *hdd_ctx)
{
	struct ipa_wdi_in_params pipe_in;
	struct ipa_wdi_out_params pipe_out;
	struct hdd_ipa_priv *ipa_ctxt = (struct hdd_ipa_priv *)hdd_ctx->hdd_ipa;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	struct ol_txrx_pdev_t *pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	int ret;
	QDF_STATUS stat = QDF_STATUS_SUCCESS;

	if (!hdd_ipa_uc_is_enabled(hdd_ctx))
		return QDF_STATUS_SUCCESS;

	ENTER();
	/* Do only IPA Pipe specific configuration here. All one time
	 * initialization wrt IPA UC shall in hdd_ipa_init and those need
	 * to be reinit at SSR shall in be SSR deinit / reinit functions.
	 */
	if (!pdev || !soc) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_FATAL, "DP context is NULL");
		stat = QDF_STATUS_E_FAILURE;
		goto fail_return;
	}

	cdp_ipa_get_resource(soc, (void *)pdev, &ipa_ctxt->ipa_resource);
	if ((ipa_ctxt->ipa_resource.ce_sr_base_paddr == 0) ||
	    (ipa_ctxt->ipa_resource.tx_comp_ring_base_paddr == 0) ||
	    (ipa_ctxt->ipa_resource.rx_rdy_ring_base_paddr == 0) ||
	    (ipa_ctxt->ipa_resource.rx2_rdy_ring_base_paddr == 0)) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_FATAL,
			"IPA UC resource alloc fail");
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_zero(&ipa_ctxt->cons_pipe_in, sizeof(struct ipa_wdi_in_params));
	qdf_mem_zero(&ipa_ctxt->prod_pipe_in, sizeof(struct ipa_wdi_in_params));
	qdf_mem_zero(&pipe_in, sizeof(struct ipa_wdi_in_params));
	qdf_mem_zero(&pipe_out, sizeof(struct ipa_wdi_out_params));

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
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
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

	qdf_mem_copy(&ipa_ctxt->cons_pipe_in, &pipe_in,
		     sizeof(struct ipa_wdi_in_params));
	hdd_ipa_uc_get_db_paddr(&ipa_ctxt->tx_comp_doorbell_paddr,
				IPA_CLIENT_WLAN1_CONS);

	if (true == ipa_ctxt->uc_loaded) {
		/* Connect WDI IPA PIPE */
		ret = ipa_connect_wdi_pipe(&ipa_ctxt->cons_pipe_in, &pipe_out);
		if (ret) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				"ipa_connect_wdi_pipe falied for Tx: ret=%d",
				ret);
			stat = QDF_STATUS_E_FAILURE;
			goto fail_return;
		}

		/* Micro Controller Doorbell register */
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
			    "CONS DB pipe out 0x%x TX PIPE Handle 0x%x",
			    (unsigned int)pipe_out.uc_door_bell_pa,
			    ipa_ctxt->tx_pipe_handle);
		ipa_ctxt->tx_comp_doorbell_paddr = pipe_out.uc_door_bell_pa;

		/* WLAN TX PIPE Handle */
		ipa_ctxt->tx_pipe_handle = pipe_out.clnt_hdl;
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
		  "TX: %s 0x%x, %s %d, %s 0x%x, %s 0x%x, %s %d, %s %d, %s 0x%x",
		  "comp_ring_base_pa",
		  (unsigned int)pipe_in.u.dl.comp_ring_base_pa,
		  "comp_ring_size",
		  pipe_in.u.dl.comp_ring_size,
		  "ce_ring_base_pa",
		  (unsigned int)pipe_in.u.dl.ce_ring_base_pa,
		  "ce_door_bell_pa",
		  (unsigned int)pipe_in.u.dl.ce_door_bell_pa,
		  "ce_ring_size",
		  pipe_in.u.dl.ce_ring_size,
		  "num_tx_buffers",
		  pipe_in.u.dl.num_tx_buffers,
		  "tx_comp_doorbell_paddr",
		  (unsigned int)ipa_ctxt->tx_comp_doorbell_paddr);
	}

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

	hdd_ipa_init_metering(ipa_ctxt, &pipe_in);

	qdf_mem_copy(&ipa_ctxt->prod_pipe_in, &pipe_in,
		     sizeof(struct ipa_wdi_in_params));
	hdd_ipa_uc_get_db_paddr(&ipa_ctxt->rx_ready_doorbell_paddr,
				IPA_CLIENT_WLAN1_PROD);

	if (true == ipa_ctxt->uc_loaded) {
		ret = ipa_connect_wdi_pipe(&ipa_ctxt->prod_pipe_in, &pipe_out);
		if (ret) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				"ipa_connect_wdi_pipe falied for Rx: ret=%d",
				ret);
			stat = QDF_STATUS_E_FAILURE;
			goto fail_return;

		}
		ipa_ctxt->rx_ready_doorbell_paddr = pipe_out.uc_door_bell_pa;
		ipa_ctxt->rx_pipe_handle = pipe_out.clnt_hdl;
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
			"PROD DB pipe out 0x%x TX PIPE Handle 0x%x",
			(unsigned int)pipe_out.uc_door_bell_pa,
			ipa_ctxt->tx_pipe_handle);
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
			"RX: %s 0x%x, %s %d, %s 0x%x, %s 0x%x",
			"rdy_ring_base_pa",
			(unsigned int)pipe_in.u.ul.rdy_ring_base_pa,
			"rdy_ring_size",
			pipe_in.u.ul.rdy_ring_size,
			"rdy_ring_rp_pa",
			(unsigned int)pipe_in.u.ul.rdy_ring_rp_pa,
			"rx_ready_doorbell_paddr",
			(unsigned int)ipa_ctxt->rx_ready_doorbell_paddr);
	}

	cdp_ipa_set_doorbell_paddr(soc, (void *)pdev,
				   ipa_ctxt->tx_comp_doorbell_paddr,
				   ipa_ctxt->rx_ready_doorbell_paddr);

	cdp_ipa_register_op_cb(soc, (void *)pdev,
			       hdd_ipa_uc_op_event_handler, (void *)hdd_ctx);

	HDD_IPA_LOG(QDF_TRACE_LEVEL_INFO_HIGH,
	     "ipa_uc_op_cb=0x%p, tx_comp_idx_paddr=0x%x, rx_rdy_idx_paddr=0x%x",
	     pdev->ipa_uc_op_cb,
	     (unsigned int)pdev->htt_pdev->ipa_uc_tx_rsc.tx_comp_idx_paddr,
	     (unsigned int)pdev->htt_pdev->ipa_uc_rx_rsc.rx_rdy_idx_paddr);

fail_return:
	EXIT();
	return stat;
}

/**
 * __hdd_ipa_uc_force_pipe_shutdown() - Force shutdown IPA pipe
 * @hdd_ctx: hdd main context
 *
 * Force shutdown IPA pipe
 * Independent of FW pipe status, IPA pipe shutdonw progress
 * in case, any STA does not leave properly, IPA HW pipe should cleaned up
 * independent from FW pipe status
 *
 * Return: NONE
 */
static void __hdd_ipa_uc_force_pipe_shutdown(hdd_context_t *hdd_ctx)
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
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
				"IPA pipes are down, do nothing");
	}
}

/**
 * hdd_ipa_uc_force_pipe_shutdown() - SSR wrapper for
 * __hdd_ipa_uc_force_pipe_shutdown
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
	cds_ssr_protect(__func__);
	__hdd_ipa_uc_force_pipe_shutdown(hdd_ctx);
	cds_ssr_unprotect(__func__);
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
	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "msg type:%d, len:%d", type, len);
	ghdd_ipa->stats.num_free_msg++;
	qdf_mem_free(buff);
}

/**
 * hdd_ipa_uc_send_evt() - send event to ipa
 * @hdd_ctx: pointer to hdd context
 * @type: event type
 * @mac_addr: pointer to mac address
 *
 * Send event to IPA driver
 *
 * Return: 0 - Success
 */
static int hdd_ipa_uc_send_evt(hdd_adapter_t *adapter,
	enum ipa_wlan_event type, uint8_t *mac_addr)
{
	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;
	struct ipa_msg_meta meta;
	struct ipa_wlan_msg *msg;
	int ret = 0;

	meta.msg_len = sizeof(struct ipa_wlan_msg);
	msg = qdf_mem_malloc(meta.msg_len);
	if (msg == NULL) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"msg allocation failed");
		return -ENOMEM;
	}

	meta.msg_type = type;
	strlcpy(msg->name, adapter->dev->name,
		IPA_RESOURCE_NAME_MAX);
	memcpy(msg->mac_addr, mac_addr, ETH_ALEN);
	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s: Evt: %d",
		msg->name, meta.msg_type);
	ret = ipa_send_msg(&meta, msg, hdd_ipa_msg_free_fn);
	if (ret) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"%s: Evt: %d fail:%d",
			msg->name, meta.msg_type,  ret);
		qdf_mem_free(msg);
		return ret;
	}

	hdd_ipa->stats.num_send_msg++;

	return ret;
}

/**
 * hdd_ipa_uc_disconnect_client() - send client disconnect event
 * @hdd_ctx: pointer to hdd adapter
 *
 * Send disconnect client event to IPA driver during SSR
 *
 * Return: 0 - Success
 */
static int hdd_ipa_uc_disconnect_client(hdd_adapter_t *adapter)
{
	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;
	int ret = 0;
	int i;

	for (i = 0; i < WLAN_MAX_STA_COUNT; i++) {
		if (qdf_is_macaddr_broadcast(&adapter->aStaInfo[i].macAddrSTA))
			continue;
		if ((adapter->aStaInfo[i].isUsed) &&
		   (!adapter->aStaInfo[i].isDeauthInProgress) &&
		   hdd_ipa->sap_num_connected_sta) {
			hdd_ipa_uc_send_evt(adapter, WLAN_CLIENT_DISCONNECT,
				adapter->aStaInfo[i].macAddrSTA.bytes);
			hdd_ipa->sap_num_connected_sta--;
		}
	}

	return ret;
}

/**
 * hdd_ipa_uc_disconnect_ap() - send ap disconnect event
 * @hdd_ctx: pointer to hdd adapter
 *
 * Send disconnect ap event to IPA driver during SSR
 *
 * Return: 0 - Success
 */

static int hdd_ipa_uc_disconnect_ap(hdd_adapter_t *adapter)
{
	int ret = 0;

	if (adapter->ipa_context)
		hdd_ipa_uc_send_evt(adapter, WLAN_AP_DISCONNECT,
			adapter->dev->dev_addr);

	return ret;
}

/**
 * hdd_ipa_uc_disconnect_sta() - send sta disconnect event
 * @hdd_ctx: pointer to hdd adapter
 *
 * Send disconnect sta event to IPA driver during SSR
 *
 * Return: 0 - Success
 */
static int hdd_ipa_uc_disconnect_sta(hdd_adapter_t *adapter)
{
	hdd_station_ctx_t *pHddStaCtx;
	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;
	int ret = 0;

	if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx) &&
	    hdd_ipa->sta_connected) {
		pHddStaCtx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		hdd_ipa_uc_send_evt(adapter, WLAN_STA_DISCONNECT,
				pHddStaCtx->conn_info.bssId.bytes);
	}

	return ret;
}

/**
 * hdd_ipa_uc_disconnect() - send disconnect ipa event
 * @hdd_ctx: pointer to hdd context
 *
 * Send disconnect event to IPA driver during SSR
 *
 * Return: 0 - Success
 */
static int hdd_ipa_uc_disconnect(hdd_context_t *hdd_ctx)
{
	hdd_adapter_list_node_t *adapter_node = NULL, *next = NULL;
	QDF_STATUS status;
	hdd_adapter_t *adapter;
	int ret = 0;

	status =  hdd_get_front_adapter(hdd_ctx, &adapter_node);
	while (NULL != adapter_node && QDF_STATUS_SUCCESS == status) {
		adapter = adapter_node->pAdapter;
		if (adapter->device_mode == QDF_SAP_MODE) {
			hdd_ipa_uc_disconnect_client(adapter);
			hdd_ipa_uc_disconnect_ap(adapter);
		} else if (adapter->device_mode == QDF_STA_MODE) {
			hdd_ipa_uc_disconnect_sta(adapter);
		}

		status = hdd_get_next_adapter(
				hdd_ctx, adapter_node, &next);
		adapter_node = next;
	}

	return ret;
}

/**
 * __hdd_ipa_uc_ssr_deinit() - handle ipa deinit for SSR
 *
 * Deinit basic IPA UC host side to be in sync reloaded FW during
 * SSR
 *
 * Return: 0 - Success
 */
static int __hdd_ipa_uc_ssr_deinit(void)
{
	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;
	int idx;
	struct hdd_ipa_iface_context *iface_context;
	hdd_context_t *hdd_ctx;

	if (!hdd_ipa)
		return 0;

	hdd_ctx = hdd_ipa->hdd_ctx;
	if (!hdd_ipa_uc_is_enabled(hdd_ctx))
		return 0;

	/* send disconnect to ipa driver */
	hdd_ipa_uc_disconnect(hdd_ctx);

	/* Clean up HDD IPA interfaces */
	for (idx = 0; (hdd_ipa->num_iface > 0) &&
		(idx < HDD_IPA_MAX_IFACE); idx++) {
		iface_context = &hdd_ipa->iface_context[idx];
		if (iface_context->adapter && iface_context->adapter->magic ==
					      WLAN_HDD_ADAPTER_MAGIC)
			hdd_ipa_cleanup_iface(iface_context);
	}
	hdd_ipa->num_iface = 0;
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

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
		    "%s: Disconnect TX PIPE tx_pipe_handle=0x%x",
		    __func__, hdd_ipa->tx_pipe_handle);
	ipa_disconnect_wdi_pipe(hdd_ipa->tx_pipe_handle);

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
		    "%s: Disconnect RX PIPE rx_pipe_handle=0x%x",
		    __func__, hdd_ipa->rx_pipe_handle);
	ipa_disconnect_wdi_pipe(hdd_ipa->rx_pipe_handle);

	if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx))
		hdd_ipa_uc_sta_reset_sta_connected(hdd_ipa);

	for (idx = 0; idx < HDD_IPA_UC_OPCODE_MAX; idx++) {
		cancel_work_sync(&hdd_ipa->uc_op_work[idx].work);
		qdf_mem_free(hdd_ipa->uc_op_work[idx].msg);
		hdd_ipa->uc_op_work[idx].msg = NULL;
	}
	return 0;
}

/**
 * hdd_ipa_uc_ssr_deinit() - SSR wrapper for __hdd_ipa_uc_ssr_deinit
 *
 * Deinit basic IPA UC host side to be in sync reloaded FW during
 * SSR
 *
 * Return: 0 - Success
 */
int hdd_ipa_uc_ssr_deinit(void)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_ipa_uc_ssr_deinit();
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __hdd_ipa_uc_ssr_reinit() - handle ipa reinit after SSR
 *
 * Init basic IPA UC host side to be in sync with reloaded FW after
 * SSR to resume IPA UC operations
 *
 * Return: 0 - Success
 */
static int __hdd_ipa_uc_ssr_reinit(hdd_context_t *hdd_ctx)
{

	struct hdd_ipa_priv *hdd_ipa = ghdd_ipa;
	int i;
	struct hdd_ipa_iface_context *iface_context = NULL;

	if (!hdd_ipa || !hdd_ipa_uc_is_enabled(hdd_ctx))
		return 0;

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
	}
	for (i = 0; i < CSR_ROAM_SESSION_MAX; i++) {
		hdd_ipa->vdev_to_iface[i] = CSR_ROAM_SESSION_MAX;
		hdd_ipa->vdev_offload_enabled[i] = false;
	}

	if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
		hdd_ipa->resource_loading = false;
		hdd_ipa->resource_unloading = false;
		hdd_ipa->sta_connected = 0;
		hdd_ipa->ipa_pipes_down = true;
		hdd_ipa->uc_loaded = true;
	}

	return 0;
}

/**
 * hdd_ipa_uc_ssr_reinit() - SSR wrapper for __hdd_ipa_uc_ssr_reinit
 *
 * Init basic IPA UC host side to be in sync with reloaded FW after
 * SSR to resume IPA UC operations
 *
 * Return: 0 - Success
 */
int hdd_ipa_uc_ssr_reinit(hdd_context_t *hdd_ctx)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_ipa_uc_ssr_reinit(hdd_ctx);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __hdd_ipa_tx_packet_ipa() - send packet to IPA
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
static struct sk_buff *__hdd_ipa_tx_packet_ipa(hdd_context_t *hdd_ctx,
	struct sk_buff *skb, uint8_t session_id)
{
	struct ipa_header *ipa_header;
	struct frag_header *frag_header;
	struct hdd_ipa_priv *hdd_ipa;

	if (wlan_hdd_validate_context(hdd_ctx))
		return skb;

	hdd_ipa = hdd_ctx->hdd_ipa;

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
 * hdd_ipa_tx_packet_ipa() - SSR wrapper for __hdd_ipa_tx_packet_ipa
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
	struct sk_buff *ret;

	cds_ssr_protect(__func__);
	ret = __hdd_ipa_tx_packet_ipa(hdd_ctx, skb, session_id);
	cds_ssr_unprotect(__func__);

	return ret;
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

	ret = ipa_rm_inactivity_timer_release_resource(
				IPA_RM_RESOURCE_WLAN_PROD);

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

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "Evt: %d", event);

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
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "RM Release");
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
 * __hdd_ipa_set_perf_level() - Set IPA performance level
 * @hdd_ctx: Global HDD context
 * @tx_packets: Number of packets transmitted in the last sample period
 * @rx_packets: Number of packets received in the last sample period
 *
 * Return: 0 on success, negative errno on error
 */
static int __hdd_ipa_set_perf_level(hdd_context_t *hdd_ctx, uint64_t tx_packets,
			   uint64_t rx_packets)
{
	uint32_t next_cons_bw, next_prod_bw;
	struct hdd_ipa_priv *hdd_ipa;
	struct ipa_rm_perf_profile profile;
	int ret;

	if (wlan_hdd_validate_context(hdd_ctx))
		return 0;

	hdd_ipa = hdd_ctx->hdd_ipa;

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

	HDD_IPA_DP_LOG(QDF_TRACE_LEVEL_DEBUG,
		"CONS perf curr: %d, next: %d",
		hdd_ipa->curr_cons_bw, next_cons_bw);
	HDD_IPA_DP_LOG(QDF_TRACE_LEVEL_DEBUG,
		"PROD perf curr: %d, next: %d",
		hdd_ipa->curr_prod_bw, next_prod_bw);

	if (hdd_ipa->curr_cons_bw != next_cons_bw) {
		hdd_debug("Requesting CONS perf curr: %d, next: %d",
			    hdd_ipa->curr_cons_bw, next_cons_bw);
		profile.max_supported_bandwidth_mbps = next_cons_bw;
		ret = ipa_rm_set_perf_profile(IPA_RM_RESOURCE_WLAN_CONS,
					      &profile);
		if (ret) {
			hdd_err("RM CONS set perf profile failed: %d", ret);

			return ret;
		}
		hdd_ipa->curr_cons_bw = next_cons_bw;
		hdd_ipa->stats.num_cons_perf_req++;
	}

	if (hdd_ipa->curr_prod_bw != next_prod_bw) {
		hdd_debug("Requesting PROD perf curr: %d, next: %d",
			    hdd_ipa->curr_prod_bw, next_prod_bw);
		profile.max_supported_bandwidth_mbps = next_prod_bw;
		ret = ipa_rm_set_perf_profile(IPA_RM_RESOURCE_WLAN_PROD,
					      &profile);
		if (ret) {
			hdd_err("RM PROD set perf profile failed: %d", ret);
			return ret;
		}
		hdd_ipa->curr_prod_bw = next_prod_bw;
		hdd_ipa->stats.num_prod_perf_req++;
	}

	return 0;
}

/**
 * hdd_ipa_set_perf_level() - SSR wrapper for __hdd_ipa_set_perf_level
 * @hdd_ctx: Global HDD context
 * @tx_packets: Number of packets transmitted in the last sample period
 * @rx_packets: Number of packets received in the last sample period
 *
 * Return: 0 on success, negative errno on error
 */
int hdd_ipa_set_perf_level(hdd_context_t *hdd_ctx, uint64_t tx_packets,
			   uint64_t rx_packets)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_ipa_set_perf_level(hdd_ctx, tx_packets, rx_packets);
	cds_ssr_unprotect(__func__);

	return ret;
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

	cancel_work_sync(&hdd_ipa->uc_rm_work.work);
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
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "Invalid adapter: 0x%p",
			    adapter);
		HDD_IPA_INCREASE_INTERNAL_DROP_COUNT(hdd_ipa);
		kfree_skb(skb);
		return;
	}

	if (cds_is_driver_unloading()) {
		HDD_IPA_INCREASE_INTERNAL_DROP_COUNT(hdd_ipa);
		kfree_skb(skb);
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
 * hdd_ipa_forward() - handle packet forwarding to wlan tx
 * @hdd_ipa: pointer to hdd ipa context
 * @adapter: network adapter
 * @skb: data pointer
 *
 * if exception packet has set forward bit, copied new packet should be
 * forwarded to wlan tx. if wlan subsystem is in suspend state, packet should
 * put into pm queue and tx procedure will be differed
 *
 * Return: None
 */
static void hdd_ipa_forward(struct hdd_ipa_priv *hdd_ipa,
			    hdd_adapter_t *adapter, qdf_nbuf_t skb)
{
	struct hdd_ipa_pm_tx_cb *pm_tx_cb;

	qdf_spin_lock_bh(&hdd_ipa->pm_lock);
	/* WLAN subsystem is in suspend, put int queue */
	if (hdd_ipa->suspended) {
		qdf_spin_unlock_bh(&hdd_ipa->pm_lock);
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			"TX in SUSPEND PUT QUEUE");
		qdf_mem_set(skb->cb, sizeof(skb->cb), 0);
		pm_tx_cb = (struct hdd_ipa_pm_tx_cb *)skb->cb;
		pm_tx_cb->exception = true;
		pm_tx_cb->adapter = adapter;
		qdf_spin_lock_bh(&hdd_ipa->pm_lock);
		qdf_nbuf_queue_add(&hdd_ipa->pm_queue_head, skb);
		qdf_spin_unlock_bh(&hdd_ipa->pm_lock);
		hdd_ipa->stats.num_tx_queued++;
	} else {
		/* Resume, put packet into WLAN TX */
		qdf_spin_unlock_bh(&hdd_ipa->pm_lock);
		if (hdd_softap_hard_start_xmit(skb, adapter->dev)) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "packet tx fail");
			hdd_ipa->stats.num_tx_fwd_err++;
		} else {
			hdd_ipa->stats.num_tx_fwd_ok++;
			hdd_ipa->ipa_tx_forward++;
		}
	}
}

/**
 * hdd_ipa_intrabss_forward() - Forward intra bss packets.
 * @hdd_ipa: pointer to HDD IPA struct
 * @adapter: hdd adapter pointer
 * @desc: Firmware descriptor
 * @skb: Data buffer
 *
 * Return:
 *      HDD_IPA_FORWARD_PKT_NONE
 *      HDD_IPA_FORWARD_PKT_DISCARD
 *      HDD_IPA_FORWARD_PKT_LOCAL_STACK
 *
 */

static enum hdd_ipa_forward_type hdd_ipa_intrabss_forward(
		struct hdd_ipa_priv *hdd_ipa,
		hdd_adapter_t *adapter,
		uint8_t desc,
		qdf_nbuf_t skb)
{
	int ret = HDD_IPA_FORWARD_PKT_NONE;

	if ((desc & FW_RX_DESC_FORWARD_M)) {
		if (!ol_txrx_fwd_desc_thresh_check(
			(struct ol_txrx_vdev_t *)ol_txrx_get_vdev_from_vdev_id(
							adapter->sessionId))) {
			/* Drop the packet*/
			hdd_ipa->stats.num_tx_fwd_err++;
			kfree_skb(skb);
			ret = HDD_IPA_FORWARD_PKT_DISCARD;
			return ret;
		}
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
				"Forward packet to Tx (fw_desc=%d)", desc);
		hdd_ipa->ipa_tx_forward++;

		if ((desc & FW_RX_DESC_DISCARD_M)) {
			hdd_ipa_forward(hdd_ipa, adapter, skb);
			hdd_ipa->ipa_rx_internel_drop_count++;
			hdd_ipa->ipa_rx_discard++;
			ret = HDD_IPA_FORWARD_PKT_DISCARD;
		} else {
			struct sk_buff *cloned_skb = skb_clone(skb, GFP_ATOMIC);

			if (cloned_skb)
				hdd_ipa_forward(hdd_ipa, adapter, cloned_skb);
			else
				HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
						"%s: tx skb alloc failed",
						__func__);
			ret = HDD_IPA_FORWARD_PKT_LOCAL_STACK;
		}
	}

	return ret;
}

/**
 * __hdd_ipa_w2i_cb() - WLAN to IPA callback handler
 * @priv: pointer to private data registered with IPA (we register a
 *	pointer to the global IPA context)
 * @evt: the IPA event which triggered the callback
 * @data: data associated with the event
 *
 * Return: None
 */
static void __hdd_ipa_w2i_cb(void *priv, enum ipa_dp_evt_type evt,
			   unsigned long data)
{
	struct hdd_ipa_priv *hdd_ipa = NULL;
	hdd_adapter_t *adapter = NULL;
	qdf_nbuf_t skb;
	uint8_t iface_id;
	uint8_t session_id;
	struct hdd_ipa_iface_context *iface_context;
	uint8_t fw_desc;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	hdd_ipa = (struct hdd_ipa_priv *)priv;

	if (!hdd_ipa || wlan_hdd_validate_context(hdd_ipa->hdd_ctx))
		return;

	switch (evt) {
	case IPA_RECEIVE:
		skb = (qdf_nbuf_t) data;

		/*
		 * When SSR is going on or driver is unloading,
		 * just drop the packets.
		 */
		status = wlan_hdd_validate_context(hdd_ipa->hdd_ctx);
		if (0 != status) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
					"Invalid context: drop packet");
			HDD_IPA_INCREASE_INTERNAL_DROP_COUNT(hdd_ipa);
			kfree_skb(skb);
			return;
		}

		if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			session_id = (uint8_t)skb->cb[0];
			iface_id = hdd_ipa->vdev_to_iface[session_id];
			HDD_IPA_DP_LOG(QDF_TRACE_LEVEL_DEBUG,
				"IPA_RECEIVE: session_id=%u, iface_id=%u",
				session_id, iface_id);
		} else {
			iface_id = HDD_IPA_GET_IFACE_ID(skb->data);
		}

		if (iface_id >= HDD_IPA_MAX_IFACE) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "IPA_RECEIVE: Invalid iface_id: %u",
				    iface_id);
			HDD_IPA_DBG_DUMP(QDF_TRACE_LEVEL_DEBUG,
				"w2i -- skb",
				skb->data, HDD_IPA_DBG_DUMP_RX_LEN);
			HDD_IPA_INCREASE_INTERNAL_DROP_COUNT(hdd_ipa);
			kfree_skb(skb);
			return;
		}

		iface_context = &hdd_ipa->iface_context[iface_id];
		adapter = iface_context->adapter;
		if (!adapter) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "IPA_RECEIVE: Adapter is NULL");
			HDD_IPA_INCREASE_INTERNAL_DROP_COUNT(hdd_ipa);
			kfree_skb(skb);
			return;
		}

		HDD_IPA_DBG_DUMP(QDF_TRACE_LEVEL_DEBUG,
				"w2i -- skb",
				skb->data, HDD_IPA_DBG_DUMP_RX_LEN);
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
		if (!adapter->sessionCtx.ap.apDisableIntraBssFwd) {
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
			if (HDD_IPA_FORWARD_PKT_DISCARD ==
			    hdd_ipa_intrabss_forward(hdd_ipa, adapter,
						     fw_desc, skb))
				break;
		} else {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
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
 * hdd_ipa_w2i_cb() - SSR wrapper for __hdd_ipa_w2i_cb
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
	cds_ssr_protect(__func__);
	__hdd_ipa_w2i_cb(priv, evt, data);
	cds_ssr_unprotect(__func__);
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
	struct ipa_rx_data *ipa_tx_desc;
	struct hdd_ipa_tx_desc *tx_desc;
	uint16_t id;

	if (!qdf_nbuf_ipa_owned_get(skb)) {
		dev_kfree_skb_any(skb);
		return;
	}

	/* Get Tx desc pointer from SKB CB */
	id = QDF_NBUF_CB_TX_IPA_PRIV(skb);
	tx_desc = hdd_ipa->tx_desc_list + id;
	ipa_tx_desc = tx_desc->ipa_tx_desc_ptr;

	/* Return Tx Desc to IPA */
	ipa_free_skb(ipa_tx_desc);

	/* Return to free tx desc list */
	qdf_spin_lock_bh(&hdd_ipa->q_lock);
	tx_desc->ipa_tx_desc_ptr = NULL;
	list_add_tail(&tx_desc->link, &hdd_ipa->free_tx_desc_head);
	hdd_ipa->stats.num_tx_desc_q_cnt--;
	qdf_spin_unlock_bh(&hdd_ipa->q_lock);

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
	hdd_adapter_t *adapter = NULL;
	qdf_nbuf_t skb;
	struct hdd_ipa_tx_desc *tx_desc;

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

	++adapter->stats.tx_packets;

	qdf_spin_unlock_bh(&iface_context->interface_lock);

	skb = ipa_tx_desc->skb;

	qdf_mem_set(skb->cb, sizeof(skb->cb), 0);

	/* Store IPA Tx buffer ownership into SKB CB */
	qdf_nbuf_ipa_owned_set(skb);
	if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx)) {
		qdf_nbuf_mapped_paddr_set(skb,
					  ipa_tx_desc->dma_addr
					  + HDD_IPA_WLAN_FRAG_HEADER
					  + HDD_IPA_WLAN_IPA_HEADER);
		ipa_tx_desc->skb->len -=
			HDD_IPA_WLAN_FRAG_HEADER + HDD_IPA_WLAN_IPA_HEADER;
	} else
		qdf_nbuf_mapped_paddr_set(skb, ipa_tx_desc->dma_addr);

	qdf_spin_lock_bh(&hdd_ipa->q_lock);
	/* get free Tx desc and assign ipa_tx_desc pointer */
	if (!list_empty(&hdd_ipa->free_tx_desc_head)) {
		tx_desc = list_first_entry(&hdd_ipa->free_tx_desc_head,
					   struct hdd_ipa_tx_desc, link);
		list_del(&tx_desc->link);
		tx_desc->ipa_tx_desc_ptr = ipa_tx_desc;
		hdd_ipa->stats.num_tx_desc_q_cnt++;
		qdf_spin_unlock_bh(&hdd_ipa->q_lock);
		/* Store Tx Desc index into SKB CB */
		QDF_NBUF_CB_TX_IPA_PRIV(skb) = tx_desc->id;
	} else {
		hdd_ipa->stats.num_tx_desc_error++;
		qdf_spin_unlock_bh(&hdd_ipa->q_lock);
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "No free Tx desc!");
		ipa_free_skb(ipa_tx_desc);
		hdd_ipa_rm_try_release(hdd_ipa);
		return;
	}

	adapter->stats.tx_bytes += ipa_tx_desc->skb->len;

	skb = cdp_ipa_tx_send_data_frame(cds_get_context(QDF_MODULE_ID_SOC),
		(struct cdp_vdev *)iface_context->tl_context,
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
 * hdd_ipa_is_present() - get IPA hw status
 * @hdd_ctx: pointer to hdd context
 *
 * ipa_uc_reg_rdyCB is not directly designed to check
 * ipa hw status. This is an undocumented function which
 * has confirmed with IPA team.
 *
 * Return: true - ipa hw present
 *         false - ipa hw not present
 */
bool hdd_ipa_is_present(hdd_context_t *hdd_ctx)
{
	/* Check if ipa hw is enabled */
	if (HDD_IPA_CHECK_HW() != -EPERM)
		return true;
	else
		return false;
}

/**
 * hdd_ipa_pm_flush() - flush queued packets
 * @work: pointer to the scheduled work
 *
 * Called during PM resume to send packets to TL which were queued
 * while host was in the process of suspending.
 *
 * Return: None
 */
static void hdd_ipa_pm_flush(struct work_struct *work)
{
	struct hdd_ipa_priv *hdd_ipa = container_of(work,
						    struct hdd_ipa_priv,
						    pm_work);
	struct hdd_ipa_pm_tx_cb *pm_tx_cb = NULL;
	qdf_nbuf_t skb;
	uint32_t dequeued = 0;

	qdf_wake_lock_acquire(&hdd_ipa->wake_lock,
			      WIFI_POWER_EVENT_WAKELOCK_IPA);
	qdf_spin_lock_bh(&hdd_ipa->pm_lock);
	while (((skb = qdf_nbuf_queue_remove(&hdd_ipa->pm_queue_head))
								!= NULL)) {
		qdf_spin_unlock_bh(&hdd_ipa->pm_lock);

		pm_tx_cb = (struct hdd_ipa_pm_tx_cb *)skb->cb;
		dequeued++;
		if (pm_tx_cb->exception) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				"FLUSH EXCEPTION");
			hdd_softap_hard_start_xmit(skb, pm_tx_cb->adapter->dev);
		} else {
			hdd_ipa_send_pkt_to_tl(pm_tx_cb->iface_context,
				       pm_tx_cb->ipa_tx_desc);
		}
		qdf_spin_lock_bh(&hdd_ipa->pm_lock);
	}
	qdf_spin_unlock_bh(&hdd_ipa->pm_lock);
	qdf_wake_lock_release(&hdd_ipa->wake_lock,
			      WIFI_POWER_EVENT_WAKELOCK_IPA);

	hdd_ipa->stats.num_tx_dequeued += dequeued;
	if (dequeued > hdd_ipa->stats.num_max_pm_queue)
		hdd_ipa->stats.num_max_pm_queue = dequeued;
}

/**
 * __hdd_ipa_i2w_cb() - IPA to WLAN callback
 * @priv: pointer to private data registered with IPA (we register a
 *	pointer to the interface-specific IPA context)
 * @evt: the IPA event which triggered the callback
 * @data: data associated with the event
 *
 * Return: None
 */
static void __hdd_ipa_i2w_cb(void *priv, enum ipa_dp_evt_type evt,
			   unsigned long data)
{
	struct hdd_ipa_priv *hdd_ipa = NULL;
	struct ipa_rx_data *ipa_tx_desc;
	struct hdd_ipa_iface_context *iface_context;
	qdf_nbuf_t skb;
	struct hdd_ipa_pm_tx_cb *pm_tx_cb = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	iface_context = (struct hdd_ipa_iface_context *)priv;
	ipa_tx_desc = (struct ipa_rx_data *)data;
	hdd_ipa = iface_context->hdd_ipa;

	if (evt != IPA_RECEIVE) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR, "Event is not IPA_RECEIVE");
		ipa_free_skb(ipa_tx_desc);
		iface_context->stats.num_tx_drop++;
		return;
	}

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

	HDD_IPA_DBG_DUMP(QDF_TRACE_LEVEL_DEBUG,
			 "i2w", skb->data, HDD_IPA_DBG_DUMP_TX_LEN);

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
	flush_work(&hdd_ipa->pm_work);

	return hdd_ipa_send_pkt_to_tl(iface_context, ipa_tx_desc);
}

/*
 * hdd_ipa_i2w_cb() - SSR wrapper for __hdd_ipa_i2w_cb
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
	cds_ssr_protect(__func__);
	__hdd_ipa_i2w_cb(priv, evt, data);
	cds_ssr_unprotect(__func__);
}

/**
 * __hdd_ipa_suspend() - Suspend IPA
 * @hdd_ctx: Global HDD context
 *
 * Return: 0 on success, negativer errno on error
 */
static int __hdd_ipa_suspend(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa;

	if (wlan_hdd_validate_context(hdd_ctx))
		return 0;

	hdd_ipa = hdd_ctx->hdd_ipa;

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
 * hdd_ipa_suspend() - SSR wrapper for __hdd_ipa_suspend
 * @hdd_ctx: Global HDD context
 *
 * Return: 0 on success, negativer errno on error
 */
int hdd_ipa_suspend(hdd_context_t *hdd_ctx)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_ipa_suspend(hdd_ctx);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __hdd_ipa_resume() - Resume IPA following suspend
 * hdd_ctx: Global HDD context
 *
 * Return: 0 on success, negative errno on error
 */
static int __hdd_ipa_resume(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa;

	if (wlan_hdd_validate_context(hdd_ctx))
		return 0;

	hdd_ipa = hdd_ctx->hdd_ipa;

	if (!hdd_ipa_is_enabled(hdd_ctx))
		return 0;

	schedule_work(&hdd_ipa->pm_work);

	qdf_spin_lock_bh(&hdd_ipa->pm_lock);
	hdd_ipa->suspended = false;
	qdf_spin_unlock_bh(&hdd_ipa->pm_lock);

	return 0;
}

/**
 * hdd_ipa_resume() - SSR wrapper for __hdd_ipa_resume
 * hdd_ctx: Global HDD context
 *
 * Return: 0 on success, negative errno on error
 */
int hdd_ipa_resume(hdd_context_t *hdd_ctx)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_ipa_resume(hdd_ctx);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * hdd_ipa_alloc_tx_desc_list() - Allocate IPA Tx desc list
 * @hdd_ipa: Global HDD IPA context
 *
 * Return: 0 on success, negative errno on error
 */
static int hdd_ipa_alloc_tx_desc_list(struct hdd_ipa_priv *hdd_ipa)
{
	int i;
	uint32_t max_desc_cnt;
	struct hdd_ipa_tx_desc *tmp_desc;

	max_desc_cnt = hdd_ipa->hdd_ctx->config->IpaUcTxBufCount;

	INIT_LIST_HEAD(&hdd_ipa->free_tx_desc_head);

	tmp_desc = qdf_mem_malloc(sizeof(struct hdd_ipa_tx_desc)*max_desc_cnt);

	if (!tmp_desc) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
			    "Free Tx descriptor allocation failed");
		return -ENOMEM;
	}

	hdd_ipa->tx_desc_list = tmp_desc;

	qdf_spin_lock_bh(&hdd_ipa->q_lock);
	for (i = 0; i < max_desc_cnt; i++) {
		tmp_desc->id = i;
		tmp_desc->ipa_tx_desc_ptr = NULL;
		list_add_tail(&tmp_desc->link,
			      &hdd_ipa->free_tx_desc_head);
		tmp_desc++;
	}

	hdd_ipa->stats.num_tx_desc_q_cnt = 0;
	hdd_ipa->stats.num_tx_desc_error = 0;

	qdf_spin_unlock_bh(&hdd_ipa->q_lock);

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
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "Failed for pipe %d ret: %d", i, ret);
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

       /* Allocate free Tx desc list */
	ret = hdd_ipa_alloc_tx_desc_list(hdd_ipa);
	if (ret)
		goto setup_sys_pipe_fail;

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
	uint32_t max_desc_cnt;
	struct hdd_ipa_tx_desc *tmp_desc;
	struct ipa_rx_data *ipa_tx_desc;

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

	if (hdd_ipa->tx_desc_list) {
		max_desc_cnt = hdd_ipa->hdd_ctx->config->IpaUcTxBufCount;

		qdf_spin_lock_bh(&hdd_ipa->q_lock);
		for (i = 0; i < max_desc_cnt; i++) {
			tmp_desc = hdd_ipa->tx_desc_list + i;
			ipa_tx_desc = tmp_desc->ipa_tx_desc_ptr;
			if (ipa_tx_desc)
				ipa_free_skb(ipa_tx_desc);
		}
		tmp_desc = hdd_ipa->tx_desc_list;
		hdd_ipa->tx_desc_list = NULL;
		hdd_ipa->stats.num_tx_desc_q_cnt = 0;
		hdd_ipa->stats.num_tx_desc_error = 0;
		qdf_spin_unlock_bh(&hdd_ipa->q_lock);
		qdf_mem_free(tmp_desc);
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

	/* Register IPA Tx desc free callback */
	qdf_nbuf_reg_free_cb(hdd_ipa_nbuf_cb);

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
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "Hdr deleted already %s, %d",
			    name, ret);
		return;
	}

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "hdl: 0x%x", hdrlookup.hdl);
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
 * wlan_ipa_add_hdr() - Add IPA Tx header
 * @ipa_hdr: pointer to IPA header addition parameters
 *
 * Call IPA API to add IPA Tx header descriptor
 * and dump Tx header struct
 *
 * Return: 0 for success, non-zero for failure
 */
static int wlan_ipa_add_hdr(struct ipa_ioc_add_hdr *ipa_hdr)
{
	int ret;

	hdd_debug("==== IPA Tx Header ====\n"
			"name: %s\n"
			"hdr_len: %d\n"
			"type: %d\n"
			"is_partial: %d\n"
			"hdr_hdl: 0x%x\n"
			"status: %d\n"
			"is_eth2_ofst_valid: %d\n"
			"eth2_ofst: %d\n",
			ipa_hdr->hdr[0].name,
			ipa_hdr->hdr[0].hdr_len,
			ipa_hdr->hdr[0].type,
			ipa_hdr->hdr[0].is_partial,
			ipa_hdr->hdr[0].hdr_hdl,
			ipa_hdr->hdr[0].status,
			ipa_hdr->hdr[0].is_eth2_ofst_valid,
			ipa_hdr->hdr[0].eth2_ofst);

	HDD_IPA_DBG_DUMP(QDF_TRACE_LEVEL_DEBUG, "hdr:",
			ipa_hdr->hdr[0].hdr, HDD_IPA_UC_WLAN_TX_HDR_LEN);

	ret = ipa_add_hdr(ipa_hdr);
	return ret;
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

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "Add Partial hdr: %s, %pM",
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

		ret = wlan_ipa_add_hdr(ipa_hdr);
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

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s: IPv4 hdr_hdl: 0x%x",
		    ipa_hdr->hdr[0].name, ipa_hdr->hdr[0].hdr_hdl);

	if (hdd_ipa_is_ipv6_enabled(hdd_ipa->hdd_ctx)) {
		snprintf(ipa_hdr->hdr[0].name, IPA_RESOURCE_NAME_MAX, "%s%s",
			 ifname, HDD_IPA_IPV6_NAME_EXT);

		if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			uc_tx_hdr =
				(struct hdd_ipa_uc_tx_hdr *)ipa_hdr->hdr[0].hdr;
			uc_tx_hdr->eth.h_proto = cpu_to_be16(ETH_P_IPV6);
			ret = wlan_ipa_add_hdr(ipa_hdr);
		} else {
			/* Set the type to IPV6 in the header */
			tx_hdr = (struct hdd_ipa_tx_hdr *)ipa_hdr->hdr[0].hdr;
			tx_hdr->llc_snap.eth_type = cpu_to_be16(ETH_P_IPV6);
			ret = ipa_add_hdr(ipa_hdr);
		}

		if (ret) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "%s: IPv6 add hdr failed: %d", ifname, ret);
			goto clean_ipv4_hdr;
		}

		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s: IPv6 hdr_hdl: 0x%x",
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
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
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
	tl_context = (void *)cdp_peer_get_vdev_by_sta_id(
				cds_get_context(QDF_MODULE_ID_SOC), sta_id);
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

#ifndef QCA_LL_TX_FLOW_CONTROL_V2
/**
 * __hdd_ipa_send_mcc_scc_msg() - send IPA WLAN_SWITCH_TO_MCC/SCC message
 * @mcc_mode: 0=MCC/1=SCC
 *
 * Return: 0 on success, negative errno value on error
 */
static int __hdd_ipa_send_mcc_scc_msg(hdd_context_t *hdd_ctx, bool mcc_mode)
{
	hdd_adapter_list_node_t *adapter_node = NULL, *next = NULL;
	QDF_STATUS status;
	hdd_adapter_t *pAdapter;
	struct ipa_msg_meta meta;
	struct ipa_wlan_msg *msg;
	int ret;

	if (wlan_hdd_validate_context(hdd_ctx))
		return -EINVAL;

	if (!hdd_ipa_uc_sta_is_enabled(hdd_ctx))
		return -EINVAL;

	if (!hdd_ctx->mcc_mode) {
		/* Flush TxRx queue for each adapter before switch to SCC */
		status =  hdd_get_front_adapter(hdd_ctx, &adapter_node);
		while (NULL != adapter_node && QDF_STATUS_SUCCESS == status) {
			pAdapter = adapter_node->pAdapter;
			if (pAdapter->device_mode == QDF_STA_MODE ||
			    pAdapter->device_mode == QDF_SAP_MODE) {
				hdd_debug("MCC->SCC: Flush TxRx queue(d_mode=%d)",
					 pAdapter->device_mode);
				hdd_deinit_tx_rx(pAdapter);
			}
			status = hdd_get_next_adapter(
					hdd_ctx, adapter_node, &next);
			adapter_node = next;
		}
	}

	/* Send SCC/MCC Switching event to IPA */
	meta.msg_len = sizeof(*msg);
	msg = qdf_mem_malloc(meta.msg_len);
	if (msg == NULL) {
		hdd_err("msg allocation failed");
		return -ENOMEM;
	}

	meta.msg_type = mcc_mode ?
			WLAN_SWITCH_TO_MCC : WLAN_SWITCH_TO_SCC;
	hdd_debug("ipa_send_msg(Evt:%d)", meta.msg_type);

	ret = ipa_send_msg(&meta, msg, hdd_ipa_msg_free_fn);

	if (ret) {
		hdd_err("ipa_send_msg(Evt:%d) - fail=%d",
			meta.msg_type,  ret);
		qdf_mem_free(msg);
	}

	return ret;
}

/**
 * hdd_ipa_send_mcc_scc_msg() - SSR wrapper for __hdd_ipa_send_mcc_scc_msg
 * @mcc_mode: 0=MCC/1=SCC
 *
 * Return: 0 on success, negative errno value on error
 */
int hdd_ipa_send_mcc_scc_msg(hdd_context_t *hdd_ctx, bool mcc_mode)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __hdd_ipa_send_mcc_scc_msg(hdd_ctx, mcc_mode);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

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

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s: %s evt, MAC: %pM sta_id: %d",
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
	if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
		if (hdd_ipa->resource_loading) {
			unsigned int pending_event_count;
			struct ipa_uc_pending_event *pending_event = NULL;

			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "IPA resource load in progress");

			qdf_mutex_acquire(&hdd_ipa->ipa_lock);

			pending_event_count =
					qdf_list_size(&hdd_ipa->pending_event);
			if (pending_event_count >=
				HDD_IPA_MAX_PENDING_EVENT_COUNT) {
				hdd_debug("Reached max pending event count");
				qdf_list_remove_front(&hdd_ipa->pending_event,
					(qdf_list_node_t **)&pending_event);
			} else {
				pending_event =
					qdf_mem_malloc(sizeof(*pending_event));
			}

			if (!pending_event) {
				qdf_mutex_release(&hdd_ipa->ipa_lock);
				HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "Pending event memory alloc fail");
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

			qdf_mutex_release(&hdd_ipa->ipa_lock);
			return 0;
		} else if (hdd_ipa->resource_unloading) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "IPA resource unload in progress");
			return 0;
		}
	}

	hdd_ipa->stats.event[type]++;

	meta.msg_type = type;
	switch (type) {
	case WLAN_STA_CONNECT:
		qdf_mutex_acquire(&hdd_ipa->event_lock);

		/* STA already connected and without disconnect, connect again
		 * This is Roaming scenario
		 */
		if (hdd_ipa->sta_connected)
			hdd_ipa_cleanup_iface(adapter->ipa_context);

		ret = hdd_ipa_setup_iface(hdd_ipa, adapter, sta_id);
		if (ret) {
			qdf_mutex_release(&hdd_ipa->event_lock);
			goto end;
		}

		if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx) &&
		    (hdd_ipa->sap_num_connected_sta > 0) &&
		    !hdd_ipa->sta_connected) {
			qdf_mutex_release(&hdd_ipa->event_lock);
			hdd_ipa_uc_offload_enable_disable(adapter,
				SIR_STA_RX_DATA_OFFLOAD, true);
			qdf_mutex_acquire(&hdd_ipa->event_lock);
		}

		hdd_ipa->vdev_to_iface[adapter->sessionId] =
			((struct hdd_ipa_iface_context *)
			(adapter->ipa_context))->iface_id;

		hdd_ipa->sta_connected = 1;

		qdf_mutex_release(&hdd_ipa->event_lock);
		break;

	case WLAN_AP_CONNECT:
		qdf_mutex_acquire(&hdd_ipa->event_lock);

		/* For DFS channel we get two start_bss event (before and after
		 * CAC). Also when ACS range includes both DFS and non DFS
		 * channels, we could possibly change channel many times due to
		 * RADAR detection and chosen channel may not be a DFS channels.
		 * So dont return error here. Just discard the event.
		 */
		if (adapter->ipa_context) {
			qdf_mutex_release(&hdd_ipa->event_lock);
			return 0;
		}

		ret = hdd_ipa_setup_iface(hdd_ipa, adapter, sta_id);
		if (ret) {
			qdf_mutex_release(&hdd_ipa->event_lock);
			hdd_err("%s: Evt: %d, Interface setup failed",
				msg_ex->name, meta.msg_type);
			goto end;
		}

		if (hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			qdf_mutex_release(&hdd_ipa->event_lock);
			hdd_ipa_uc_offload_enable_disable(adapter,
				SIR_AP_RX_DATA_OFFLOAD, true);
			qdf_mutex_acquire(&hdd_ipa->event_lock);
		}

		hdd_ipa->vdev_to_iface[adapter->sessionId] =
			((struct hdd_ipa_iface_context *)
			(adapter->ipa_context))->iface_id;

		qdf_mutex_release(&hdd_ipa->event_lock);
		break;

	case WLAN_STA_DISCONNECT:
		qdf_mutex_acquire(&hdd_ipa->event_lock);

		if (!hdd_ipa->sta_connected) {
			qdf_mutex_release(&hdd_ipa->event_lock);
			hdd_err("%s: Evt: %d, STA already disconnected",
				msg_ex->name, meta.msg_type);
			return -EINVAL;
		}

		hdd_ipa->sta_connected = 0;

		if (!hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			hdd_debug("%s: IPA UC OFFLOAD NOT ENABLED",
				msg_ex->name);
		} else {
			/* Disable IPA UC TX PIPE when STA disconnected */
			if (!hdd_ipa->num_iface &&
			    (HDD_IPA_UC_NUM_WDI_PIPE ==
			    hdd_ipa->activated_fw_pipe) &&
				!hdd_ipa->ipa_pipes_down)
				hdd_ipa_uc_handle_last_discon(hdd_ipa);
		}

		if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx) &&
		    (hdd_ipa->sap_num_connected_sta > 0)) {
			qdf_mutex_release(&hdd_ipa->event_lock);
			hdd_ipa_uc_offload_enable_disable(adapter,
				SIR_STA_RX_DATA_OFFLOAD, false);
			qdf_mutex_acquire(&hdd_ipa->event_lock);
			hdd_ipa->vdev_to_iface[adapter->sessionId] =
				CSR_ROAM_SESSION_MAX;
		}

		hdd_ipa_cleanup_iface(adapter->ipa_context);

		qdf_mutex_release(&hdd_ipa->event_lock);
		break;

	case WLAN_AP_DISCONNECT:
		qdf_mutex_acquire(&hdd_ipa->event_lock);

		if (!adapter->ipa_context) {
			qdf_mutex_release(&hdd_ipa->event_lock);
			hdd_err("%s: Evt: %d, SAP already disconnected",
				msg_ex->name, meta.msg_type);
			return -EINVAL;
		}

		if ((!hdd_ipa->num_iface) &&
			(HDD_IPA_UC_NUM_WDI_PIPE ==
				hdd_ipa->activated_fw_pipe) &&
				!hdd_ipa->ipa_pipes_down) {
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
			qdf_mutex_release(&hdd_ipa->event_lock);
			hdd_ipa_uc_offload_enable_disable(adapter,
				SIR_AP_RX_DATA_OFFLOAD, false);
			qdf_mutex_acquire(&hdd_ipa->event_lock);
			hdd_ipa->vdev_to_iface[adapter->sessionId] =
				CSR_ROAM_SESSION_MAX;
		}

		hdd_ipa_cleanup_iface(adapter->ipa_context);

		qdf_mutex_release(&hdd_ipa->event_lock);
		break;

	case WLAN_CLIENT_CONNECT_EX:
		if (!hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
				"%s: Evt: %d, IPA UC OFFLOAD NOT ENABLED",
				adapter->dev->name, type);
			return 0;
		}

		qdf_mutex_acquire(&hdd_ipa->event_lock);
		if (hdd_ipa_uc_find_add_assoc_sta(hdd_ipa,
				true, sta_id)) {
			qdf_mutex_release(&hdd_ipa->event_lock);
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				"%s: STA ID %d found, not valid",
				adapter->dev->name, sta_id);
			return 0;
		}

		/* Enable IPA UC Data PIPEs when first STA connected */
		if (hdd_ipa->sap_num_connected_sta == 0 &&
				hdd_ipa->uc_loaded == true) {
			if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx) &&
			    hdd_ipa->sta_connected) {
				qdf_mutex_release(&hdd_ipa->event_lock);
				hdd_ipa_uc_offload_enable_disable(
					hdd_get_adapter(hdd_ipa->hdd_ctx,
							QDF_STA_MODE),
					SIR_STA_RX_DATA_OFFLOAD, true);
				qdf_mutex_acquire(&hdd_ipa->event_lock);
			}

			ret = hdd_ipa_uc_handle_first_con(hdd_ipa);
			if (ret) {
				HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
					    "%s: handle 1st con ret %d",
					    adapter->dev->name, ret);

				if (hdd_ipa_uc_sta_is_enabled(
					hdd_ipa->hdd_ctx) &&
				    hdd_ipa->sta_connected) {
					qdf_mutex_release(&hdd_ipa->event_lock);
					hdd_ipa_uc_offload_enable_disable(
						hdd_get_adapter(
							hdd_ipa->hdd_ctx,
							QDF_STA_MODE),
						SIR_STA_RX_DATA_OFFLOAD, false);
				} else {
					qdf_mutex_release(&hdd_ipa->event_lock);
				}

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
			HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s: Evt: %d : %d",
				    adapter->dev->name, type, ret);
			qdf_mem_free(msg_ex);
			return ret;
		}
		hdd_ipa->stats.num_send_msg++;
		return ret;

	case WLAN_CLIENT_DISCONNECT:
		if (!hdd_ipa_uc_is_enabled(hdd_ipa->hdd_ctx)) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
					"%s: IPA UC OFFLOAD NOT ENABLED",
					msg_ex->name);
			return 0;
		}

		qdf_mutex_acquire(&hdd_ipa->event_lock);
		if (!hdd_ipa_uc_find_add_assoc_sta(hdd_ipa, false, sta_id)) {
			qdf_mutex_release(&hdd_ipa->event_lock);
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
				    "%s: STA ID %d NOT found, not valid",
				    msg_ex->name, sta_id);
			return 0;
		}
		hdd_ipa->sap_num_connected_sta--;

		/* Disable IPA UC TX PIPE when last STA disconnected */
		if (!hdd_ipa->sap_num_connected_sta &&
				hdd_ipa->uc_loaded == true) {
			if ((false == hdd_ipa->resource_unloading)
			    && (HDD_IPA_UC_NUM_WDI_PIPE ==
				hdd_ipa->activated_fw_pipe) &&
				!hdd_ipa->ipa_pipes_down) {
				hdd_ipa_uc_handle_last_discon(hdd_ipa);
			}

			qdf_mutex_release(&hdd_ipa->event_lock);

			if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx) &&
			    hdd_ipa->sta_connected)
				hdd_ipa_uc_offload_enable_disable(
					hdd_get_adapter(hdd_ipa->hdd_ctx,
							QDF_STA_MODE),
					SIR_STA_RX_DATA_OFFLOAD, false);
		} else {
			qdf_mutex_release(&hdd_ipa->event_lock);
		}
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

	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG, "%s: Evt: %d",
		    msg->name, meta.msg_type);

	ret = ipa_send_msg(&meta, msg, hdd_ipa_msg_free_fn);

	if (ret) {
		hdd_err("%s: Evt: %d fail:%d",
			msg->name, meta.msg_type, ret);
		qdf_mem_free(msg);
		return ret;
	}

	hdd_ipa->stats.num_send_msg++;

end:
	return ret;
}

/**
 * hdd_ipa_wlan_evt() - SSR wrapper for __hdd_ipa_wlan_evt
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
	int ret = 0;

	cds_ssr_protect(__func__);

	/* Data path offload only support for STA and SAP mode */
	if ((QDF_STA_MODE == adapter->device_mode) ||
	    (QDF_SAP_MODE == adapter->device_mode))
		ret = __hdd_ipa_wlan_evt(adapter, sta_id, type, mac_addr);

	cds_ssr_unprotect(__func__);

	return ret;
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
	HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
		"%s, Pending Event Count %d", __func__, pending_event_count);
	if (!pending_event_count) {
		HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
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
 * __hdd_ipa_init() - IPA initialization function
 * @hdd_ctx: HDD global context
 *
 * Allocate hdd_ipa resources, ipa pipe resource and register
 * wlan interface with IPA module.
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS __hdd_ipa_init(hdd_context_t *hdd_ctx)
{
	struct hdd_ipa_priv *hdd_ipa = NULL;
	int ret, i;
	struct hdd_ipa_iface_context *iface_context = NULL;
	struct ol_txrx_pdev_t *pdev = NULL;

	if (!hdd_ipa_is_enabled(hdd_ctx))
		return QDF_STATUS_SUCCESS;

	ENTER();

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
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
	for (i = 0; i < CSR_ROAM_SESSION_MAX; i++) {
		hdd_ipa->vdev_to_iface[i] = CSR_ROAM_SESSION_MAX;
		hdd_ipa->vdev_offload_enabled[i] = false;
	}

	INIT_WORK(&hdd_ipa->pm_work, hdd_ipa_pm_flush);
	qdf_spinlock_create(&hdd_ipa->pm_lock);
	qdf_spinlock_create(&hdd_ipa->q_lock);
	qdf_nbuf_queue_init(&hdd_ipa->pm_queue_head);
	qdf_list_create(&hdd_ipa->pending_event, 1000);
	qdf_mutex_create(&hdd_ipa->event_lock);
	qdf_mutex_create(&hdd_ipa->ipa_lock);

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
		hdd_ipa->wdi_enabled = false;
		/* Setup IPA sys_pipe for MCC */
		if (hdd_ipa_uc_sta_is_enabled(hdd_ipa->hdd_ctx)) {
			ret = hdd_ipa_setup_sys_pipe(hdd_ipa);
			if (ret)
				goto fail_create_sys_pipe;
		}
		if (hdd_ipa_uc_register_uc_ready(hdd_ipa))
			goto fail_create_sys_pipe;

		for (i = 0; i < HDD_IPA_UC_OPCODE_MAX; i++) {
			hdd_ipa_init_uc_op_work(&hdd_ipa->uc_op_work[i].work,
						hdd_ipa_uc_fw_op_event_handler);
			hdd_ipa->uc_op_work[i].msg = NULL;
		}
	} else {
		ret = hdd_ipa_setup_sys_pipe(hdd_ipa);
		if (ret)
			goto fail_create_sys_pipe;
	}

	EXIT();
	return QDF_STATUS_SUCCESS;

fail_create_sys_pipe:
	hdd_ipa_destroy_rm_resource(hdd_ipa);
fail_setup_rm:
	qdf_spinlock_destroy(&hdd_ipa->pm_lock);
	qdf_mem_free(hdd_ipa);
	hdd_ctx->hdd_ipa = NULL;
	ghdd_ipa = NULL;
fail_return:
	EXIT();
	return QDF_STATUS_E_FAILURE;
}

/**
 * hdd_ipa_init() - SSR wrapper for __hdd_ipa_init
 * @hdd_ctx: HDD global context
 *
 * Allocate hdd_ipa resources, ipa pipe resource and register
 * wlan interface with IPA module.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_ipa_init(hdd_context_t *hdd_ctx)
{
	QDF_STATUS ret;

	cds_ssr_protect(__func__);
	ret = __hdd_ipa_init(hdd_ctx);
	cds_ssr_unprotect(__func__);

	return ret;
}


/**
 * hdd_ipa_cleanup_pending_event() - Cleanup IPA pending event list
 * @hdd_ipa: pointer to HDD IPA struct
 *
 * Return: none
 */
static void hdd_ipa_cleanup_pending_event(struct hdd_ipa_priv *hdd_ipa)
{
	struct ipa_uc_pending_event *pending_event = NULL;

	while (qdf_list_remove_front(&hdd_ipa->pending_event,
		(qdf_list_node_t **)&pending_event) == QDF_STATUS_SUCCESS) {
		qdf_mem_free(pending_event);
	}

	qdf_list_destroy(&hdd_ipa->pending_event);
}

/**
 * __hdd_ipa_cleanup - IPA cleanup function
 * @hdd_ctx: HDD global context
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS __hdd_ipa_cleanup(hdd_context_t *hdd_ctx)
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

	cancel_work_sync(&hdd_ipa->pm_work);

	qdf_spin_lock_bh(&hdd_ipa->pm_lock);

	while (((skb = qdf_nbuf_queue_remove(&hdd_ipa->pm_queue_head))
								!= NULL)) {
		qdf_spin_unlock_bh(&hdd_ipa->pm_lock);

		pm_tx_cb = (struct hdd_ipa_pm_tx_cb *)skb->cb;
		if (pm_tx_cb->ipa_tx_desc)
			ipa_free_skb(pm_tx_cb->ipa_tx_desc);

		qdf_spin_lock_bh(&hdd_ipa->pm_lock);
	}
	qdf_spin_unlock_bh(&hdd_ipa->pm_lock);

	qdf_spinlock_destroy(&hdd_ipa->pm_lock);
	qdf_spinlock_destroy(&hdd_ipa->q_lock);

	/* destory the interface lock */
	for (i = 0; i < HDD_IPA_MAX_IFACE; i++) {
		iface_context = &hdd_ipa->iface_context[i];
		qdf_spinlock_destroy(&iface_context->interface_lock);
	}

	if (hdd_ipa_uc_is_enabled(hdd_ctx)) {
		if (ipa_uc_dereg_rdyCB())
			HDD_IPA_LOG(QDF_TRACE_LEVEL_ERROR,
					"UC Ready CB deregister fail");
		hdd_ipa_uc_rt_debug_deinit(hdd_ctx);
		if (true == hdd_ipa->uc_loaded) {
			HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
			    "%s: Disconnect TX PIPE tx_pipe_handle=0x%x",
			    __func__, hdd_ipa->tx_pipe_handle);
			ipa_disconnect_wdi_pipe(hdd_ipa->tx_pipe_handle);
			HDD_IPA_LOG(QDF_TRACE_LEVEL_DEBUG,
			    "%s: Disconnect RX PIPE rx_pipe_handle=0x%x",
			    __func__, hdd_ipa->rx_pipe_handle);
			ipa_disconnect_wdi_pipe(hdd_ipa->rx_pipe_handle);
		}
		qdf_mutex_destroy(&hdd_ipa->event_lock);
		qdf_mutex_destroy(&hdd_ipa->ipa_lock);
		hdd_ipa_cleanup_pending_event(hdd_ipa);

		for (i = 0; i < HDD_IPA_UC_OPCODE_MAX; i++) {
			cancel_work_sync(&hdd_ipa->uc_op_work[i].work);
			hdd_ipa->uc_op_work[i].msg = NULL;
		}
	}

	qdf_mem_free(hdd_ipa);
	hdd_ctx->hdd_ipa = NULL;

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_ipa_cleanup - SSR wrapper for __hdd_ipa_cleanup
 * @hdd_ctx: HDD global context
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_ipa_cleanup(hdd_context_t *hdd_ctx)
{
	QDF_STATUS ret;

	cds_ssr_protect(__func__);
	ret = __hdd_ipa_cleanup(hdd_ctx);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif /* IPA_OFFLOAD */
