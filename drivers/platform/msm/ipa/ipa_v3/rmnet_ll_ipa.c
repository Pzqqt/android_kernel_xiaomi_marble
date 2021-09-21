// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/skbuff.h>
#include <linux/workqueue.h>
#include <linux/ipa.h>
#include <uapi/linux/msm_rmnet.h>
#include "ipa_i.h"

enum ipa_rmnet_ll_state {
	IPA_RMNET_LL_NOT_REG,
	IPA_RMNET_LL_REGD, /* rmnet_ll register */
	IPA_RMNET_LL_PIPE_READY, /* sys pipe setup */
	IPA_RMNET_LL_START, /* rmnet_ll register + pipe setup */
};

#define IPA_RMNET_LL_PIPE_NOT_READY (0)
#define IPA_RMNET_LL_PIPE_TX_READY (1 << 0)
#define IPA_RMNET_LL_PIPE_RX_READY (1 << 1)
#define IPA_RMNET_LL_PIPE_READY_ALL (IPA_RMNET_LL_PIPE_TX_READY | \
	IPA_RMNET_LL_PIPE_RX_READY) /* TX Ready + RX ready */


#define IPA_WWAN_CONS_DESC_FIFO_SZ 256
#define RMNET_LL_QUEUE_MAX ((2 * IPA_WWAN_CONS_DESC_FIFO_SZ) - 1)

struct ipa3_rmnet_ll_cb_info {
	ipa_rmnet_ll_ready_cb ready_cb;
	ipa_rmnet_ll_stop_cb stop_cb;
	ipa_rmnet_ll_rx_notify_cb rx_notify_cb;
	void *ready_cb_user_data;
	void *stop_cb_user_data;
	void *rx_notify_cb_user_data;
};

struct ipa3_rmnet_ll_stats {
	atomic_t outstanding_pkts;
	u32 tx_pkt_sent;
	u32 rx_pkt_rcvd;
	u64 tx_byte_sent;
	u64 rx_byte_rcvd;
	u32 tx_pkt_dropped;
	u32 rx_pkt_dropped;
	u64 tx_byte_dropped;
	u64 rx_byte_dropped;
};

struct rmnet_ll_ipa3_debugfs_file {
	const char *name;
	umode_t mode;
	void *data;
	const struct file_operations fops;
};

struct rmnet_ll_ipa3_debugfs {
	struct dentry *dent;
};

struct rmnet_ll_ipa3_context {
	struct ipa3_rmnet_ll_stats stats;
	enum ipa_rmnet_ll_state state;
	u8 pipe_state;
	struct ipa_sys_connect_params apps_to_ipa_low_lat_data_ep_cfg;
	struct ipa_sys_connect_params ipa_to_apps_low_lat_data_ep_cfg;
	u32 apps_to_ipa3_low_lat_data_hdl;
	u32 ipa3_to_apps_low_lat_data_hdl;
	spinlock_t tx_lock;
	struct ipa3_rmnet_ll_cb_info cb_info;
	struct sk_buff_head tx_queue;
	u32 rmnet_ll_pm_hdl;
	struct rmnet_ll_ipa3_debugfs dbgfs;
	struct mutex lock;
	struct workqueue_struct *wq;
};

static struct rmnet_ll_ipa3_context *rmnet_ll_ipa3_ctx;

static void rmnet_ll_wakeup_ipa(struct work_struct *work);
static DECLARE_DELAYED_WORK(rmnet_ll_wakeup_work,
	rmnet_ll_wakeup_ipa);
static void apps_rmnet_ll_tx_complete_notify(void *priv,
	enum ipa_dp_evt_type evt, unsigned long data);
static void apps_rmnet_ll_receive_notify(void *priv,
	enum ipa_dp_evt_type evt, unsigned long data);
static int ipa3_rmnet_ll_register_pm_client(void);
static void ipa3_rmnet_ll_deregister_pm_client(void);
#ifdef CONFIG_DEBUG_FS
#define IPA_MAX_MSG_LEN 4096
static char dbg_buff[IPA_MAX_MSG_LEN + 1];

static ssize_t rmnet_ll_ipa3_read_stats(struct file *file, char __user *ubuf,
		size_t count, loff_t *ppos)
{
	int nbytes;
	int cnt = 0;

	nbytes = scnprintf(dbg_buff, IPA_MAX_MSG_LEN,
		"Queue Leng=%u\n"
		"outstanding_pkts=%u\n"
		"tx_pkt_sent=%u\n"
		"rx_pkt_rcvd=%u\n"
		"tx_byte_sent=%lu\n"
		"rx_byte_rcvd=%lu\n"
		"tx_pkt_dropped=%u\n"
		"rx_pkt_dropped=%u\n"
		"tx_byte_dropped=%lu\n"
		"rx_byte_dropped=%lu\n",
		skb_queue_len(&rmnet_ll_ipa3_ctx->tx_queue),
		atomic_read(
		&rmnet_ll_ipa3_ctx->stats.outstanding_pkts),
		rmnet_ll_ipa3_ctx->stats.tx_pkt_sent,
		rmnet_ll_ipa3_ctx->stats.rx_pkt_rcvd,
		rmnet_ll_ipa3_ctx->stats.tx_byte_sent,
		rmnet_ll_ipa3_ctx->stats.rx_byte_rcvd,
		rmnet_ll_ipa3_ctx->stats.tx_pkt_dropped,
		rmnet_ll_ipa3_ctx->stats.rx_pkt_dropped,
		rmnet_ll_ipa3_ctx->stats.tx_byte_dropped,
		rmnet_ll_ipa3_ctx->stats.rx_byte_dropped);
	cnt += nbytes;

	return simple_read_from_buffer(ubuf, count, ppos, dbg_buff, cnt);
}

#define READ_ONLY_MODE  0444
static const struct rmnet_ll_ipa3_debugfs_file debugfs_files[] = {
	{
		"stats", READ_ONLY_MODE, NULL, {
			.read = rmnet_ll_ipa3_read_stats
		}
	},
};

static void rmnet_ll_ipa3_debugfs_remove(void)
{
	if (IS_ERR(rmnet_ll_ipa3_ctx->dbgfs.dent))
		return;

	debugfs_remove_recursive(rmnet_ll_ipa3_ctx->dbgfs.dent);
	memset(&rmnet_ll_ipa3_ctx->dbgfs, 0,
		sizeof(struct rmnet_ll_ipa3_debugfs));
}

static void rmnet_ll_ipa3_debugfs_init(void)
{
	struct rmnet_ll_ipa3_debugfs *dbgfs = &rmnet_ll_ipa3_ctx->dbgfs;
	struct dentry *file;
	const size_t debugfs_files_num =
		sizeof(debugfs_files) / sizeof(struct rmnet_ll_ipa3_debugfs_file);
	size_t i;

	dbgfs->dent = debugfs_create_dir("rmnet_ll_ipa", 0);
	if (IS_ERR(dbgfs->dent)) {
		pr_err("fail to create folder in debug_fs\n");
		return;
	}

	for (i = 0; i < debugfs_files_num; ++i) {
		const struct rmnet_ll_ipa3_debugfs_file *curr = &debugfs_files[i];

		file = debugfs_create_file(curr->name, curr->mode, dbgfs->dent,
			curr->data, &curr->fops);
		if (!file || IS_ERR(file)) {
			IPAERR("fail to create file for debug_fs %s\n",
				curr->name);
			goto fail;
		}
	}

	return;

fail:
	rmnet_ll_ipa3_debugfs_remove();
}
#else /* CONFIG_DEBUG_FS */
static void rmnet_ll_ipa3_debugfs_init(void){}
static void rmnet_ll_ipa3_debugfs_remove(void){}
#endif /* CONFIG_DEBUG_FS */

int ipa3_rmnet_ll_init(void)
{
	char buff[IPA_RESOURCE_NAME_MAX];

	if (!ipa3_ctx) {
		IPAERR("ipa3_ctx was not initialized\n");
		return -EINVAL;
	}

	if (ipa3_get_ep_mapping(IPA_CLIENT_APPS_WAN_LOW_LAT_DATA_PROD) == -1 ||
		ipa3_get_ep_mapping(IPA_CLIENT_APPS_WAN_LOW_LAT_DATA_CONS) == -1)
	{
		IPAERR("invalid low lat data endpoints\n");
		return -EINVAL;
	}

	rmnet_ll_ipa3_ctx = kzalloc(sizeof(*rmnet_ll_ipa3_ctx),
			GFP_KERNEL);

	if (!rmnet_ll_ipa3_ctx)
		return -ENOMEM;

	snprintf(buff, IPA_RESOURCE_NAME_MAX, "rmnet_llwq");
	rmnet_ll_ipa3_ctx->wq = alloc_workqueue(buff,
		WQ_MEM_RECLAIM | WQ_UNBOUND | WQ_SYSFS, 1);
	if (!rmnet_ll_ipa3_ctx->wq) {
		kfree(rmnet_ll_ipa3_ctx);
		rmnet_ll_ipa3_ctx = NULL;
		return -ENOMEM;
	}
	memset(&rmnet_ll_ipa3_ctx->apps_to_ipa_low_lat_data_ep_cfg, 0,
		sizeof(struct ipa_sys_connect_params));
	memset(&rmnet_ll_ipa3_ctx->ipa_to_apps_low_lat_data_ep_cfg, 0,
		sizeof(struct ipa_sys_connect_params));
	skb_queue_head_init(&rmnet_ll_ipa3_ctx->tx_queue);
	rmnet_ll_ipa3_ctx->state = IPA_RMNET_LL_NOT_REG;
	mutex_init(&rmnet_ll_ipa3_ctx->lock);
	spin_lock_init(&rmnet_ll_ipa3_ctx->tx_lock);
	rmnet_ll_ipa3_ctx->pipe_state = IPA_RMNET_LL_PIPE_NOT_READY;
	rmnet_ll_ipa3_debugfs_init();
	return 0;
}

int ipa3_register_rmnet_ll_cb(
	void (*ipa_rmnet_ll_ready_cb)(void *user_data1),
	void *user_data1,
	void (*ipa_rmnet_ll_stop_cb)(void *user_data2),
	void *user_data2,
	void (*ipa_rmnet_ll_rx_notify_cb)(
	void *user_data3, void *rx_data),
	void *user_data3)
{
	/* check ipa3_ctx existed or not */
	if (!ipa3_ctx) {
		IPADBG("rmnet_ll_ctx haven't initialized\n");
		return -EAGAIN;
	}

	if (!ipa3_ctx->rmnet_ll_enable) {
		IPAERR("low lat data pipes are not supported");
		return -ENXIO;
	}

	if (!rmnet_ll_ipa3_ctx) {
		IPADBG("rmnet_ll_ctx haven't initialized\n");
		return -EAGAIN;
	}

	mutex_lock(&rmnet_ll_ipa3_ctx->lock);
	if (rmnet_ll_ipa3_ctx->state != IPA_RMNET_LL_NOT_REG &&
		rmnet_ll_ipa3_ctx->state != IPA_RMNET_LL_PIPE_READY) {
		IPADBG("rmnet_ll registered already\n", __func__);
		mutex_unlock(&rmnet_ll_ipa3_ctx->lock);
		return -EEXIST;
	}
	rmnet_ll_ipa3_ctx->cb_info.ready_cb = ipa_rmnet_ll_ready_cb;
	rmnet_ll_ipa3_ctx->cb_info.ready_cb_user_data = user_data1;
	rmnet_ll_ipa3_ctx->cb_info.stop_cb = ipa_rmnet_ll_stop_cb;
	rmnet_ll_ipa3_ctx->cb_info.stop_cb_user_data = user_data2;
	rmnet_ll_ipa3_ctx->cb_info.rx_notify_cb = ipa_rmnet_ll_rx_notify_cb;
	rmnet_ll_ipa3_ctx->cb_info.rx_notify_cb_user_data = user_data3;
	if (rmnet_ll_ipa3_ctx->state == IPA_RMNET_LL_NOT_REG) {
		rmnet_ll_ipa3_ctx->state = IPA_RMNET_LL_REGD;
	} else {
		(*ipa_rmnet_ll_ready_cb)(user_data1);
		rmnet_ll_ipa3_ctx->state = IPA_RMNET_LL_START;
	}
	ipa3_rmnet_ll_register_pm_client();
	mutex_unlock(&rmnet_ll_ipa3_ctx->lock);
	IPADBG("rmnet_ll registered successfually\n");
	return 0;
}

int ipa3_unregister_rmnet_ll_cb(void)
{
	/* check ipa3_ctx existed or not */
	if (!ipa3_ctx) {
		IPADBG("IPA driver haven't initialized\n");
		return -EAGAIN;
	}

	if (!ipa3_ctx->rmnet_ll_enable) {
		IPAERR("low lat data pipe is disabled");
		return -ENXIO;
	}

	if (!rmnet_ll_ipa3_ctx) {
		IPADBG("rmnet_ll_ctx haven't initialized\n");
		return -EAGAIN;
	}

	mutex_lock(&rmnet_ll_ipa3_ctx->lock);
	if (rmnet_ll_ipa3_ctx->state != IPA_RMNET_LL_REGD &&
		rmnet_ll_ipa3_ctx->state != IPA_RMNET_LL_START) {
		IPADBG("rmnet_ll unregistered already\n", __func__);
		mutex_unlock(&rmnet_ll_ipa3_ctx->lock);
		return 0;
	}
	rmnet_ll_ipa3_ctx->cb_info.ready_cb = NULL;
	rmnet_ll_ipa3_ctx->cb_info.ready_cb_user_data = NULL;
	rmnet_ll_ipa3_ctx->cb_info.stop_cb = NULL;
	rmnet_ll_ipa3_ctx->cb_info.stop_cb_user_data = NULL;
	rmnet_ll_ipa3_ctx->cb_info.rx_notify_cb = NULL;
	rmnet_ll_ipa3_ctx->cb_info.rx_notify_cb_user_data = NULL;
	if (rmnet_ll_ipa3_ctx->state == IPA_RMNET_LL_REGD)
		rmnet_ll_ipa3_ctx->state = IPA_RMNET_LL_NOT_REG;
	else
		rmnet_ll_ipa3_ctx->state = IPA_RMNET_LL_PIPE_READY;

	ipa3_rmnet_ll_deregister_pm_client();
	mutex_unlock(&rmnet_ll_ipa3_ctx->lock);

	IPADBG("rmnet_ll unregistered successfually\n");
	return 0;
}

int ipa3_setup_apps_low_lat_data_cons_pipe(
	struct rmnet_ingress_param *ingress_param,
	struct net_device *dev)
{
	struct ipa_sys_connect_params *ipa_low_lat_data_ep_cfg;
	int ret = 0;
	int ep_idx;

	if (!ipa3_ctx->rmnet_ll_enable) {
		IPAERR("low lat data pipe is disabled");
		return 0;
	}
	ep_idx = ipa_get_ep_mapping(
		IPA_CLIENT_APPS_WAN_LOW_LAT_DATA_CONS);
	if (ep_idx == IPA_EP_NOT_ALLOCATED) {
		IPADBG("Low lat datapath not supported\n");
		return -ENXIO;
	}
	if (rmnet_ll_ipa3_ctx->state != IPA_RMNET_LL_NOT_REG &&
		rmnet_ll_ipa3_ctx->state != IPA_RMNET_LL_REGD) {
		IPADBG("rmnet_ll in bad state %d\n",
			rmnet_ll_ipa3_ctx->state);
		return -ENXIO;
	}
	ipa_low_lat_data_ep_cfg =
		&rmnet_ll_ipa3_ctx->ipa_to_apps_low_lat_data_ep_cfg;
	/*
	 * Removing enable aggr from assign_policy
	 * and placing it here for future enablement
	 */
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.aggr.aggr_en = IPA_ENABLE_AGGR;
	if (ingress_param) {
		/* Open for future cs offload disablement on low lat pipe */
		if (ingress_param->cs_offload_en) {
			ipa_low_lat_data_ep_cfg->ipa_ep_cfg.cfg.cs_offload_en =
				IPA_ENABLE_CS_DL_QMAP;
		} else {
			ipa_low_lat_data_ep_cfg->ipa_ep_cfg.cfg.cs_offload_en =
				IPA_DISABLE_CS_OFFLOAD;
		}
		ipa_low_lat_data_ep_cfg->ext_ioctl_v2 = true;
		ipa_low_lat_data_ep_cfg->int_modt = ingress_param->int_modt;
		ipa_low_lat_data_ep_cfg->int_modc = ingress_param->int_modc;
		ipa_low_lat_data_ep_cfg->buff_size = ingress_param->buff_size;
		ipa_low_lat_data_ep_cfg->ipa_ep_cfg.aggr.aggr_byte_limit =
			ingress_param->agg_byte_limit;
		ipa_low_lat_data_ep_cfg->ipa_ep_cfg.aggr.aggr_pkt_limit =
			ingress_param->agg_pkt_limit;
		ipa_low_lat_data_ep_cfg->ipa_ep_cfg.aggr.aggr_time_limit =
			ingress_param->agg_time_limit;
	} else {
		ipa_low_lat_data_ep_cfg->ext_ioctl_v2 = false;
		ipa_low_lat_data_ep_cfg->ipa_ep_cfg.cfg.cs_offload_en =
			IPA_ENABLE_CS_DL_QMAP;
		ipa_low_lat_data_ep_cfg->ipa_ep_cfg.aggr.aggr_byte_limit =
			0;
		ipa_low_lat_data_ep_cfg->ipa_ep_cfg.aggr.aggr_pkt_limit =
			0;
	}

	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr.hdr_len = 8;
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr.hdr_ofst_metadata_valid
		= 1;
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr.hdr_ofst_metadata
		= 1;
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid
		= 1;
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr.hdr_ofst_pkt_size
		= 2;
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr_ext.hdr_total_len_or_pad_valid
		= true;
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr_ext.hdr_total_len_or_pad
		= 0;
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr_ext.hdr_payload_len_inc_padding
		= true;
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr_ext.hdr_total_len_or_pad_offset
		= 0;
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr_ext.hdr_little_endian
		= 0;
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.metadata_mask.metadata_mask
		= 0xFF000000;
	ipa_low_lat_data_ep_cfg->client = IPA_CLIENT_APPS_WAN_LOW_LAT_DATA_CONS;
	ipa_low_lat_data_ep_cfg->notify = apps_rmnet_ll_receive_notify;
	ipa_low_lat_data_ep_cfg->priv = dev;
	ipa_low_lat_data_ep_cfg->desc_fifo_sz =
		IPA_WWAN_CONS_DESC_FIFO_SZ * IPA_FIFO_ELEMENT_SIZE;
	ipa_low_lat_data_ep_cfg->priv = dev;
	ret = ipa_setup_sys_pipe(
		&rmnet_ll_ipa3_ctx->ipa_to_apps_low_lat_data_ep_cfg,
		&rmnet_ll_ipa3_ctx->ipa3_to_apps_low_lat_data_hdl);
	if (ret) {
		IPADBG("Low lat data pipe setup fails\n");
		return ret;
	}
	rmnet_ll_ipa3_ctx->pipe_state |= IPA_RMNET_LL_PIPE_RX_READY;
	if (rmnet_ll_ipa3_ctx->cb_info.ready_cb) {
		(*(rmnet_ll_ipa3_ctx->cb_info.ready_cb))
			(rmnet_ll_ipa3_ctx->cb_info.ready_cb_user_data);
	}
	/*
	 * if no ready_cb yet, which means rmnet_ll not
	 * register to IPA, we will move state to pipe
	 * ready and will wait for register event
	 * coming and move to start state.
	 * The ready_cb will called from regsiter itself.
	 */
	mutex_lock(&rmnet_ll_ipa3_ctx->lock);
	if (rmnet_ll_ipa3_ctx->state == IPA_RMNET_LL_NOT_REG)
		rmnet_ll_ipa3_ctx->state = IPA_RMNET_LL_PIPE_READY;
	else
		rmnet_ll_ipa3_ctx->state = IPA_RMNET_LL_START;
	mutex_unlock(&rmnet_ll_ipa3_ctx->lock);

	return 0;
}

int ipa3_setup_apps_low_lat_data_prod_pipe(
	struct rmnet_egress_param *egress_param,
	struct net_device *dev)
{
	struct ipa_sys_connect_params *ipa_low_lat_data_ep_cfg;
	int ret = 0;
	int ep_idx;

	if (!ipa3_ctx->rmnet_ll_enable) {
		IPAERR("Low lat pipe is disabled");
		return 0;
	}
	ep_idx = ipa_get_ep_mapping(
		IPA_CLIENT_APPS_WAN_LOW_LAT_DATA_PROD);
	if (ep_idx == IPA_EP_NOT_ALLOCATED) {
		IPAERR("low lat data pipe not supported\n");
		return -EFAULT;
	}
	ipa_low_lat_data_ep_cfg =
		&rmnet_ll_ipa3_ctx->apps_to_ipa_low_lat_data_ep_cfg;
	if (egress_param) {
		/* Open for future cs offload disablement on low lat pipe */
		IPAERR("Configuring low lat data prod with rmnet config\n");
		ipa_low_lat_data_ep_cfg->ext_ioctl_v2 = true;
		ipa_low_lat_data_ep_cfg->int_modt = egress_param->int_modt;
		ipa_low_lat_data_ep_cfg->int_modc = egress_param->int_modc;
		if (egress_param->cs_offload_en) {
			ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr.hdr_len = 8;
			ipa_low_lat_data_ep_cfg->ipa_ep_cfg.cfg.cs_offload_en =
				IPA_ENABLE_CS_OFFLOAD_UL;
			ipa_low_lat_data_ep_cfg->ipa_ep_cfg.cfg.cs_metadata_hdr_offset
				= 1;
			ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr.hdr_ofst_metadata_valid
				= 1;
			/* modem want offset at 0! */
			ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr.hdr_ofst_metadata = 0;
		} else {
			ipa_low_lat_data_ep_cfg->ipa_ep_cfg.cfg.cs_offload_en =
				IPA_DISABLE_CS_OFFLOAD;
		}

		/* Open for future deaggr enablement on low lat pipe */
		if (egress_param->aggr_en) {
			IPAERR("Enabling deaggr on low_lat_prod\n");
			ipa_low_lat_data_ep_cfg->ipa_ep_cfg.aggr.aggr_en =
				IPA_ENABLE_DEAGGR;
			ipa_low_lat_data_ep_cfg->ipa_ep_cfg.aggr.aggr = IPA_QCMAP;
			ipa_low_lat_data_ep_cfg->
				ipa_ep_cfg.deaggr.packet_offset_valid = false;
		} else {
			IPAERR("Not enabling deaggr on low_lat_prod\n");
			ipa_low_lat_data_ep_cfg->ipa_ep_cfg.aggr.aggr_en =
				IPA_BYPASS_AGGR;
		}
	} else {
		IPAERR("Configuring low lat data prod without rmnet config\n");
		ipa_low_lat_data_ep_cfg->ext_ioctl_v2 = false;
		ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr.hdr_len = 8;
		ipa_low_lat_data_ep_cfg->ipa_ep_cfg.cfg.cs_offload_en =
			IPA_ENABLE_CS_OFFLOAD_UL;
		ipa_low_lat_data_ep_cfg->ipa_ep_cfg.aggr.aggr_en =
			IPA_BYPASS_AGGR;
		ipa_low_lat_data_ep_cfg->ipa_ep_cfg.cfg.cs_metadata_hdr_offset
			= 1;
		ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr.hdr_ofst_metadata_valid
			= 1;
		/* modem want offset at 0 */
		ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr.hdr_ofst_metadata = 0;
	}
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.metadata_mask.metadata_mask
			= 0;
	/* modem want offset at 0! */
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.hdr.hdr_ofst_metadata = 0x00010000;
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.deaggr.syspipe_err_detection = true;
	ipa_low_lat_data_ep_cfg->ipa_ep_cfg.mode.dst =
		IPA_CLIENT_APPS_WAN_LOW_LAT_DATA_PROD;
	ipa_low_lat_data_ep_cfg->client =
		IPA_CLIENT_APPS_WAN_LOW_LAT_DATA_PROD;
	ipa_low_lat_data_ep_cfg->notify =
		apps_rmnet_ll_tx_complete_notify;
	ipa_low_lat_data_ep_cfg->priv = dev;
	ipa_low_lat_data_ep_cfg->desc_fifo_sz =
		IPA_SYS_TX_DATA_DESC_FIFO_SZ_8K;

	ret = ipa_setup_sys_pipe(ipa_low_lat_data_ep_cfg,
		&rmnet_ll_ipa3_ctx->apps_to_ipa3_low_lat_data_hdl);
	if (ret) {
		IPAERR("failed to config apps low lat dtaa prod pipe\n");
		return ret;
	}
	rmnet_ll_ipa3_ctx->pipe_state |= IPA_RMNET_LL_PIPE_TX_READY;
	return 0;
}

int ipa3_teardown_apps_low_lat_data_pipes(void)
{
	int ret = 0;

	if (rmnet_ll_ipa3_ctx->state != IPA_RMNET_LL_PIPE_READY &&
		rmnet_ll_ipa3_ctx->state != IPA_RMNET_LL_START &&
		rmnet_ll_ipa3_ctx->pipe_state == IPA_RMNET_LL_PIPE_NOT_READY) {
		IPAERR("rmnet_ll in bad state %d\n",
			rmnet_ll_ipa3_ctx->state);
		return -EFAULT;
	}
	if (rmnet_ll_ipa3_ctx->pipe_state == IPA_RMNET_LL_PIPE_READY ||
		rmnet_ll_ipa3_ctx->state == IPA_RMNET_LL_START) {
		if (rmnet_ll_ipa3_ctx->cb_info.stop_cb) {
			(*(rmnet_ll_ipa3_ctx->cb_info.stop_cb))
				(rmnet_ll_ipa3_ctx->cb_info.stop_cb_user_data);
		} else {
			IPAERR("Invalid stop_cb\n");
			return -EFAULT;
		}
		if (rmnet_ll_ipa3_ctx->state == IPA_RMNET_LL_PIPE_READY)
			rmnet_ll_ipa3_ctx->state = IPA_RMNET_LL_NOT_REG;
		else
			rmnet_ll_ipa3_ctx->state = IPA_RMNET_LL_REGD;
	}
	if (rmnet_ll_ipa3_ctx->pipe_state & IPA_RMNET_LL_PIPE_RX_READY) {
		ret = ipa3_teardown_sys_pipe(
			rmnet_ll_ipa3_ctx->ipa3_to_apps_low_lat_data_hdl);
		if (ret < 0) {
			IPAERR("Failed to teardown APPS->IPA low lat data pipe\n");
			return ret;
		}
		rmnet_ll_ipa3_ctx->ipa3_to_apps_low_lat_data_hdl = -1;
		rmnet_ll_ipa3_ctx->pipe_state &= ~IPA_RMNET_LL_PIPE_RX_READY;
	}

	if (rmnet_ll_ipa3_ctx->pipe_state & IPA_RMNET_LL_PIPE_TX_READY) {
		ret = ipa3_teardown_sys_pipe(
			rmnet_ll_ipa3_ctx->apps_to_ipa3_low_lat_data_hdl);
		if (ret < 0) {
			return ret;
			IPAERR("Failed to teardown APPS->IPA low lat data pipe\n");
		}
		rmnet_ll_ipa3_ctx->apps_to_ipa3_low_lat_data_hdl = -1;
		rmnet_ll_ipa3_ctx->pipe_state &= ~IPA_RMNET_LL_PIPE_TX_READY;
	}
	return ret;
}

int ipa3_rmnet_ll_xmit(struct sk_buff *skb)
{
	int ret;
	int len;
	unsigned long flags;

	if (!ipa3_ctx->rmnet_ll_enable) {
		IPAERR("low lat data pipe not supported\n");
		kfree_skb(skb);
		return 0;
	}

	spin_lock_irqsave(&rmnet_ll_ipa3_ctx->tx_lock, flags);
	/* we cannot infinitely queue the packet */
	if ((atomic_read(
		&rmnet_ll_ipa3_ctx->stats.outstanding_pkts)
		>= RMNET_LL_QUEUE_MAX)) {
		IPAERR_RL("IPA LL TX queue full\n");
		rmnet_ll_ipa3_ctx->stats.tx_pkt_dropped++;
		rmnet_ll_ipa3_ctx->stats.tx_byte_dropped +=
			skb->len;
		spin_unlock_irqrestore(&rmnet_ll_ipa3_ctx->tx_lock,
			flags);
		kfree_skb(skb);
		return -EAGAIN;
	}

	if (rmnet_ll_ipa3_ctx->state != IPA_RMNET_LL_START) {
		IPAERR("bad rmnet_ll state %d\n",
			rmnet_ll_ipa3_ctx->state);
		rmnet_ll_ipa3_ctx->stats.tx_pkt_dropped++;
		rmnet_ll_ipa3_ctx->stats.tx_byte_dropped +=
			skb->len;
		spin_unlock_irqrestore(&rmnet_ll_ipa3_ctx->tx_lock,
			flags);
		kfree_skb(skb);
		return 0;
	}

	/* if queue is not empty, means we still have pending wq */
	if (skb_queue_len(&rmnet_ll_ipa3_ctx->tx_queue) != 0) {
		skb_queue_tail(&rmnet_ll_ipa3_ctx->tx_queue, skb);
		spin_unlock_irqrestore(&rmnet_ll_ipa3_ctx->tx_lock,
			flags);
		return 0;
	}

	/* rmnet_ll is calling from atomic context */
	ret = ipa_pm_activate(rmnet_ll_ipa3_ctx->rmnet_ll_pm_hdl);
	if (ret == -EINPROGRESS) {
		skb_queue_tail(&rmnet_ll_ipa3_ctx->tx_queue, skb);
		/*
		 * delayed work is required here since we need to
		 * reschedule in the same workqueue context on error
		 */
		queue_delayed_work(rmnet_ll_ipa3_ctx->wq,
			&rmnet_ll_wakeup_work, 0);
		spin_unlock_irqrestore(&rmnet_ll_ipa3_ctx->tx_lock,
			flags);
		return 0;
	} else if (ret) {
		IPAERR("[%s] fatal: ipa pm activate failed %d\n",
			__func__, ret);
		rmnet_ll_ipa3_ctx->stats.tx_pkt_dropped++;
		rmnet_ll_ipa3_ctx->stats.tx_byte_dropped +=
			skb->len;
		spin_unlock_irqrestore(&rmnet_ll_ipa3_ctx->tx_lock,
			flags);
		kfree_skb(skb);
		return 0;
	}
	spin_unlock_irqrestore(&rmnet_ll_ipa3_ctx->tx_lock, flags);

	len = skb->len;
	/*
	 * both data packets and command will be routed to
	 * IPA_CLIENT_Q6_WAN_CONS based on DMA settings
	 */
	ret = ipa3_tx_dp(IPA_CLIENT_APPS_WAN_LOW_LAT_DATA_PROD, skb, NULL);
	if (ret) {
		if (ret == -EPIPE) {
			IPAERR("Low lat data fatal: pipe is not valid\n");
			spin_lock_irqsave(&rmnet_ll_ipa3_ctx->tx_lock,
				flags);
			rmnet_ll_ipa3_ctx->stats.tx_pkt_dropped++;
			rmnet_ll_ipa3_ctx->stats.tx_byte_dropped +=
				skb->len;
			spin_unlock_irqrestore(&rmnet_ll_ipa3_ctx->tx_lock,
				flags);
			kfree_skb(skb);
			return 0;
		}
		spin_lock_irqsave(&rmnet_ll_ipa3_ctx->tx_lock, flags);
		skb_queue_head(&rmnet_ll_ipa3_ctx->tx_queue, skb);
		queue_delayed_work(rmnet_ll_ipa3_ctx->wq,
			&rmnet_ll_wakeup_work, 0);
		ret = 0;
		goto out;
	}

	spin_lock_irqsave(&rmnet_ll_ipa3_ctx->tx_lock, flags);
	atomic_inc(&rmnet_ll_ipa3_ctx->stats.outstanding_pkts);
	rmnet_ll_ipa3_ctx->stats.tx_pkt_sent++;
	rmnet_ll_ipa3_ctx->stats.tx_byte_sent += len;
	ret = 0;

out:
	if (atomic_read(
		&rmnet_ll_ipa3_ctx->stats.outstanding_pkts)
		== 0)
		ipa_pm_deferred_deactivate(rmnet_ll_ipa3_ctx->rmnet_ll_pm_hdl);
	spin_unlock_irqrestore(&rmnet_ll_ipa3_ctx->tx_lock, flags);
	return ret;
}

static void rmnet_ll_wakeup_ipa(struct work_struct *work)
{
	int ret;
	unsigned long flags;
	struct sk_buff *skb;
	int len = 0;

	/* calling from WQ */
	ret = ipa_pm_activate_sync(rmnet_ll_ipa3_ctx->rmnet_ll_pm_hdl);
	if (ret) {
		IPAERR("[%s] fatal: ipa pm activate failed %d\n",
			__func__, ret);
		queue_delayed_work(rmnet_ll_ipa3_ctx->wq,
			&rmnet_ll_wakeup_work,
			msecs_to_jiffies(1));
		return;
	}

	spin_lock_irqsave(&rmnet_ll_ipa3_ctx->tx_lock, flags);
	/* dequeue the skb */
	while (skb_queue_len(&rmnet_ll_ipa3_ctx->tx_queue) > 0) {
		skb = skb_dequeue(&rmnet_ll_ipa3_ctx->tx_queue);
		if (skb == NULL)
			continue;
		len = skb->len;
		spin_unlock_irqrestore(&rmnet_ll_ipa3_ctx->tx_lock, flags);
		/*
		 * both data packets and command will be routed to
		 * IPA_CLIENT_Q6_WAN_CONS based on DMA settings
		 */
		ret = ipa3_tx_dp(IPA_CLIENT_APPS_WAN_LOW_LAT_DATA_PROD, skb, NULL);
		if (ret) {
			if (ret == -EPIPE) {
				/* try to drain skb from queue if pipe teardown */
				IPAERR_RL("Low lat data fatal: pipe is not valid\n");
				spin_lock_irqsave(&rmnet_ll_ipa3_ctx->tx_lock,
					flags);
				rmnet_ll_ipa3_ctx->stats.tx_pkt_dropped++;
				rmnet_ll_ipa3_ctx->stats.tx_byte_dropped +=
					skb->len;
				kfree_skb(skb);
				continue;
			}
			spin_lock_irqsave(&rmnet_ll_ipa3_ctx->tx_lock, flags);
			skb_queue_head(&rmnet_ll_ipa3_ctx->tx_queue, skb);
			spin_unlock_irqrestore(&rmnet_ll_ipa3_ctx->tx_lock, flags);
			goto delayed_work;
		}

		atomic_inc(&rmnet_ll_ipa3_ctx->stats.outstanding_pkts);
		spin_lock_irqsave(&rmnet_ll_ipa3_ctx->tx_lock, flags);
		rmnet_ll_ipa3_ctx->stats.tx_pkt_sent++;
		rmnet_ll_ipa3_ctx->stats.tx_byte_sent += len;
	}
	spin_unlock_irqrestore(&rmnet_ll_ipa3_ctx->tx_lock, flags);
	goto out;

delayed_work:
	queue_delayed_work(rmnet_ll_ipa3_ctx->wq,
		&rmnet_ll_wakeup_work,
		msecs_to_jiffies(1));
out:
	if (atomic_read(
		&rmnet_ll_ipa3_ctx->stats.outstanding_pkts)
		== 0) {
		ipa_pm_deferred_deactivate(rmnet_ll_ipa3_ctx->rmnet_ll_pm_hdl);
	}

}

/**
 * apps_rmnet_ll_tx_complete_notify() - Rx notify
 *
 * @priv: driver context
 * @evt: event type
 * @data: data provided with event
 *
 * Check that the packet is the one we sent and release it
 * This function will be called in defered context in IPA wq.
 */
static void apps_rmnet_ll_tx_complete_notify(void *priv,
	enum ipa_dp_evt_type evt, unsigned long data)
{
	struct sk_buff *skb = (struct sk_buff *)data;
	unsigned long flags;

	if (evt != IPA_WRITE_DONE) {
		IPAERR("unsupported evt on Tx callback, Drop the packet\n");
		spin_lock_irqsave(&rmnet_ll_ipa3_ctx->tx_lock,
			flags);
		rmnet_ll_ipa3_ctx->stats.tx_pkt_dropped++;
		rmnet_ll_ipa3_ctx->stats.tx_byte_dropped +=
			skb->len;
		spin_unlock_irqrestore(&rmnet_ll_ipa3_ctx->tx_lock,
			flags);
		dev_kfree_skb_any(skb);
		return;
	}

	atomic_dec(&rmnet_ll_ipa3_ctx->stats.outstanding_pkts);

	if (atomic_read(
		&rmnet_ll_ipa3_ctx->stats.outstanding_pkts) == 0)
		ipa_pm_deferred_deactivate(rmnet_ll_ipa3_ctx->rmnet_ll_pm_hdl);

	dev_kfree_skb_any(skb);
}

/**
 * apps_rmnet_ll_receive_notify() - Rmnet_ll RX notify
 *
 * @priv: driver context
 * @evt: event type
 * @data: data provided with event
 *
 * IPA will pass a packet to the Linux network stack with skb->data
 */
static void apps_rmnet_ll_receive_notify(void *priv,
	enum ipa_dp_evt_type evt, unsigned long data)
{
	void *rx_notify_cb_rx_data;
	struct sk_buff *low_lat_data;
	int len;

	low_lat_data = (struct sk_buff *)data;
	if (low_lat_data == NULL) {
		IPAERR("Rx packet is invalid");
		return;
	}
	len = low_lat_data->len;
	if (evt == IPA_RECEIVE) {
		IPADBG_LOW("Rx packet was received");
		rx_notify_cb_rx_data = (void *)data;
		if (rmnet_ll_ipa3_ctx->cb_info.rx_notify_cb) {
			(*(rmnet_ll_ipa3_ctx->cb_info.rx_notify_cb))(
			rmnet_ll_ipa3_ctx->cb_info.rx_notify_cb_user_data,
			rx_notify_cb_rx_data);
		} else
			goto fail;
		rmnet_ll_ipa3_ctx->stats.rx_pkt_rcvd++;
		rmnet_ll_ipa3_ctx->stats.rx_byte_rcvd +=
			len;
	} else {
		IPAERR("Invalid evt %d received in rmnet_ll\n", evt);
		goto fail;
	}
	return;

fail:
	kfree_skb(low_lat_data);
	rmnet_ll_ipa3_ctx->stats.rx_pkt_dropped++;
}


static int ipa3_rmnet_ll_register_pm_client(void)
{
	int result;
	struct ipa_pm_register_params pm_reg;

	memset(&pm_reg, 0, sizeof(pm_reg));
	pm_reg.name = "rmnet_ll";
	pm_reg.group = IPA_PM_GROUP_APPS;
	result = ipa_pm_register(&pm_reg, &rmnet_ll_ipa3_ctx->rmnet_ll_pm_hdl);
	if (result) {
		IPAERR("failed to create IPA PM client %d\n", result);
		return result;
	}

	IPAERR("%s register done\n", pm_reg.name);

	return 0;
}

static void ipa3_rmnet_ll_deregister_pm_client(void)
{
	ipa_pm_deactivate_sync(rmnet_ll_ipa3_ctx->rmnet_ll_pm_hdl);
	ipa_pm_deregister(rmnet_ll_ipa3_ctx->rmnet_ll_pm_hdl);
}
