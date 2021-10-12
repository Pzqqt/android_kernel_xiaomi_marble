// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

#include <linux/msm_ipa.h>
#include "../ipa_common_i.h"
#include "../ipa_v3/ipa_pm.h"
#include "../ipa_v3/ipa_i.h"
#include <linux/ipa_eth.h>

#define OFFLOAD_DRV_NAME "ipa_eth"
#define IPA_ETH_DBG(fmt, args...) \
	do { \
		pr_debug(OFFLOAD_DRV_NAME " %s:%d " fmt, \
			__func__, __LINE__, ## args); \
		IPA_IPC_LOGGING(ipa3_get_ipc_logbuf(), \
			OFFLOAD_DRV_NAME " %s:%d " fmt, ## args); \
		IPA_IPC_LOGGING(ipa3_get_ipc_logbuf_low(), \
			OFFLOAD_DRV_NAME " %s:%d " fmt, ## args); \
	} while (0)

#define IPA_ETH_DBG_LOW(fmt, args...) \
	do { \
		pr_debug(OFFLOAD_DRV_NAME " %s:%d " fmt, \
			__func__, __LINE__, ## args); \
		IPA_IPC_LOGGING(ipa3_get_ipc_logbuf_low(), \
			OFFLOAD_DRV_NAME " %s:%d " fmt, ## args); \
	} while (0)

#define IPA_ETH_ERR(fmt, args...) \
	do { \
		pr_err(OFFLOAD_DRV_NAME " %s:%d " fmt, \
			__func__, __LINE__, ## args); \
		IPA_IPC_LOGGING(ipa3_get_ipc_logbuf(), \
			OFFLOAD_DRV_NAME " %s:%d " fmt, ## args); \
		IPA_IPC_LOGGING(ipa3_get_ipc_logbuf_low(), \
			OFFLOAD_DRV_NAME " %s:%d " fmt, ## args); \
	} while (0)

#define IPA_ETH_PIPES_NO 8

struct ipa_eth_ready_cb_wrapper {
	struct list_head link;
	struct ipa_eth_ready *info;
};

struct ipa_eth_per_client_info {
	u32 pm_hdl;
	atomic_t ref_cnt;
	bool existed;
};

struct ipa_eth_intf {
	struct list_head link;
	char netdev_name[IPA_RESOURCE_NAME_MAX];
	u8 hdr_len;
	u32 partial_hdr_hdl[IPA_IP_MAX];
};

struct ipa_eth_context {
	struct list_head ready_cb_list;
	struct completion completion;
	struct ipa_eth_per_client_info
		client[IPA_ETH_CLIENT_MAX][IPA_ETH_INST_ID_MAX];
	struct mutex lock;
	struct workqueue_struct *wq;
	bool is_eth_ready;
	struct idr idr;
	spinlock_t idr_lock;
	struct list_head head_intf_list;
	void *client_priv;
};

static struct ipa_eth_context *ipa_eth_ctx;

static int ipa_eth_uc_rdy_cb(struct notifier_block *nb,
	unsigned long action, void *data);

static void ipa_eth_ready_notify_work(struct work_struct *work);

static struct notifier_block uc_rdy_cb = {
	.notifier_call = ipa_eth_uc_rdy_cb,
};

static DECLARE_WORK(ipa_eth_ready_notify, ipa_eth_ready_notify_work);

static bool pipe_connected[IPA_ETH_PIPES_NO];

static u8 client_to_pipe_index(enum ipa_client_type client_type)
{
	switch (client_type) {
	case IPA_CLIENT_ETHERNET_CONS:
		return 0;
		break;
	case IPA_CLIENT_ETHERNET_PROD:
		return 1;
		break;
	case IPA_CLIENT_RTK_ETHERNET_CONS:
		return 2;
		break;
	case IPA_CLIENT_RTK_ETHERNET_PROD:
		return 3;
		break;
	case IPA_CLIENT_AQC_ETHERNET_CONS:
		return 4;
		break;
	case IPA_CLIENT_AQC_ETHERNET_PROD:
		return 5;
		break;
	case IPA_CLIENT_ETHERNET2_CONS:
		return 6;
		break;
	case IPA_CLIENT_ETHERNET2_PROD:
		return 7;
		break;
	default:
		IPAERR("invalid eth client_type\n");
		ipa_assert();
	}
	return 0;
}

static int ipa_eth_init_internal(void)
{
	char buff[IPA_RESOURCE_NAME_MAX];
	int i, j;

	/* already initialized */
	if (ipa_eth_ctx)
		return 0;

	ipa_eth_ctx = kzalloc(sizeof(*ipa_eth_ctx), GFP_KERNEL);
	if (ipa_eth_ctx == NULL) {
		IPA_ETH_ERR("fail to alloc eth ctx\n");
		return -ENOMEM;
	}

	snprintf(buff, IPA_RESOURCE_NAME_MAX, "ipa_eth_wq");
	ipa_eth_ctx->wq = alloc_workqueue(buff,
		WQ_MEM_RECLAIM | WQ_UNBOUND | WQ_SYSFS, 1);
	if (!ipa_eth_ctx->wq) {
		goto wq_err;
	}
	mutex_init(&ipa_eth_ctx->lock);
	INIT_LIST_HEAD(&ipa_eth_ctx->ready_cb_list);
	ipa_eth_ctx->is_eth_ready = false;
	for (i = 0; i < IPA_ETH_CLIENT_MAX; i++) {
		for (j = 0; j < IPA_ETH_INST_ID_MAX; j++) {
			ipa_eth_ctx->client[i][j].pm_hdl = 0;
			ipa_eth_ctx->client[i][j].existed = false;
			atomic_set(&ipa_eth_ctx->client[i][j].ref_cnt, 0);
		}
	}
	idr_init(&ipa_eth_ctx->idr);
	spin_lock_init(&ipa_eth_ctx->idr_lock);
	INIT_LIST_HEAD(&ipa_eth_ctx->head_intf_list);
	ipa_eth_ctx->client_priv = NULL;
	ipa3_eth_debugfs_init();
	return 0;

wq_err:
	kfree(ipa_eth_ctx);
	ipa_eth_ctx = NULL;
	return -ENOMEM;
}

static int ipa_eth_cleanup_internal(void)
{
	struct ipa_eth_intf *entry;
	struct ipa_eth_intf *next;

	/* already deinitialized */
	if (!ipa_eth_ctx)
		return 0;
	/* clear interface list */
	list_for_each_entry_safe(entry, next,
		&ipa_eth_ctx->head_intf_list, link) {
		list_del(&entry->link);
		kfree(entry);
	}
	mutex_destroy(&ipa_eth_ctx->lock);
	destroy_workqueue(ipa_eth_ctx->wq);
	kfree(ipa_eth_ctx);
	ipa_eth_ctx = NULL;
	return 0;
}

static int ipa_eth_uc_rdy_cb(struct notifier_block *nb,
	unsigned long action, void *data)
{
	IPA_ETH_DBG("IPA uC is ready for eth");
	queue_work(ipa_eth_ctx->wq, &ipa_eth_ready_notify);
	return NOTIFY_OK;
}

static void ipa_eth_ready_notify_work(struct work_struct *work)
{
	struct ipa_eth_ready_cb_wrapper *entry;
	struct ipa_eth_ready_cb_wrapper *next;

	IPA_ETH_DBG("ipa_eth ready notify\n");
	mutex_lock(&ipa_eth_ctx->lock);
	ipa_eth_ctx->is_eth_ready = true;
	list_for_each_entry_safe(entry, next,
		&ipa_eth_ctx->ready_cb_list, link) {
		if (entry->info && entry->info->notify)
			entry->info->notify(entry->info->userdata);
		/* remove from list once notify is done */
		list_del(&entry->link);
		kfree(entry);
	}
	mutex_unlock(&ipa_eth_ctx->lock);
}

static int ipa_eth_register_ready_cb_internal(struct ipa_eth_ready *ready_info)
{
	int rc;
	struct ipa_eth_ready_cb_wrapper *ready_cb;

	/* validate user input */
	if (!ready_info) {
		IPA_ETH_ERR("null ready_info");
		return -EFAULT;
	}

	if (!ipa_eth_ctx) {
		rc = ipa_eth_init_internal();
		if (rc) {
			/* it is not normal to fail here */
			IPA_ETH_ERR("initialization failure\n");
			return rc;
		}
		IPA_ETH_DBG("ipa_eth register ready cb\n");
		mutex_lock(&ipa_eth_ctx->lock);
		ready_cb = kmalloc(sizeof(struct ipa_eth_ready_cb_wrapper),
			GFP_KERNEL);
		if (!ready_cb) {
			mutex_unlock(&ipa_eth_ctx->lock);
			ipa_eth_cleanup_internal();
			return -ENOMEM;
		}
		ready_cb->info = ready_info;
		list_add_tail(&ready_cb->link, &ipa_eth_ctx->ready_cb_list);
		mutex_unlock(&ipa_eth_ctx->lock);
		/* rely on uC ready callback, only register once */
		rc = ipa3_uc_register_ready_cb(&uc_rdy_cb);
		if (rc) {
			IPA_ETH_ERR("Failed to register ready cb\n");
			goto err_uc;
		}
	} else {
		/* assume only IOSS could register for cb */
		IPA_ETH_ERR("multiple eth register happens\n");
		mutex_lock(&ipa_eth_ctx->lock);
		ready_cb = kmalloc(sizeof(struct ipa_eth_ready_cb_wrapper),
			GFP_KERNEL);
		if (!ready_cb) {
			mutex_unlock(&ipa_eth_ctx->lock);
			return -ENOMEM;
		}
		ready_cb->info = ready_info;
		list_add_tail(&ready_cb->link, &ipa_eth_ctx->ready_cb_list);
		/* if already ready, directly callback from wq */
		if (ipa3_uc_loaded_check())
			queue_work(ipa_eth_ctx->wq, &ipa_eth_ready_notify);
		mutex_unlock(&ipa_eth_ctx->lock);
	}

	/* if uc is already ready, set the output param to true */
	if (ipa3_uc_loaded_check())
		ready_info->is_eth_ready = true;

	return 0;

err_uc:
	list_del(&ready_cb->link);
	ipa_eth_cleanup_internal();
	return rc;
}

static int ipa_eth_unregister_ready_cb_internal(struct ipa_eth_ready *ready_info)
{
	struct ipa_eth_ready_cb_wrapper *entry;
	bool find_ready_info = false;

	/* validate user input */
	if (!ready_info) {
		IPA_ETH_ERR("null ready_info");
		return -EFAULT;
	}

	if (!ipa_eth_ctx) {
		IPA_ETH_ERR("unregister called before register\n");
		return -EFAULT;
	}

	IPA_ETH_DBG("ipa_eth unregister ready cb\n");
	mutex_lock(&ipa_eth_ctx->lock);
	list_for_each_entry(entry, &ipa_eth_ctx->ready_cb_list,
		link) {
		if (!entry)
			break;
		if (entry->info == ready_info) {
			list_del(&entry->link);
			find_ready_info = true;
			break;
		}
	}
	if (!find_ready_info) {
		IPA_ETH_ERR("unable to unregsiter, no ready_info\n");
		mutex_unlock(&ipa_eth_ctx->lock);
		return -EFAULT;
	}
	if (list_empty(&ipa_eth_ctx->ready_cb_list)) {
		mutex_unlock(&ipa_eth_ctx->lock);
		ipa_eth_cleanup_internal();
		return 0;
	}

	ready_info->is_eth_ready = false;
	mutex_unlock(&ipa_eth_ctx->lock);
	return 0;
}

static u32 ipa_eth_pipe_hdl_alloc(void *ptr)
{
	ipa_eth_hdl_t hdl;

	idr_preload(GFP_KERNEL);
	spin_lock(&ipa_eth_ctx->idr_lock);
	hdl = idr_alloc(&ipa_eth_ctx->idr, ptr, 0, 0, GFP_NOWAIT);
	spin_unlock(&ipa_eth_ctx->idr_lock);
	idr_preload_end();

	return hdl;
}

static void ipa_eth_pipe_hdl_remove(ipa_eth_hdl_t hdl)
{
	spin_lock(&ipa_eth_ctx->idr_lock);
	idr_remove(&ipa_eth_ctx->idr, hdl);
	spin_unlock(&ipa_eth_ctx->idr_lock);
}

static enum ipa_client_type
	ipa_eth_get_ipa_client_type_from_pipe(
	struct ipa_eth_client_pipe_info *pipe)
{
	struct ipa_eth_client *client;
	int ipa_client_type = IPA_CLIENT_MAX;

	if (!pipe) {
		IPA_ETH_ERR("invalid pipe\n");
		return ipa_client_type;
	}

	client = pipe->client_info;
	if (!client) {
		IPA_ETH_ERR("invalid client\n");
		return ipa_client_type;
	}
	switch (client->client_type) {
	case IPA_ETH_CLIENT_AQC107:
	case IPA_ETH_CLIENT_AQC113:
		if (client->traffic_type ==
			IPA_ETH_PIPE_BEST_EFFORT &&
			client->inst_id == 0) {
			if (pipe->dir == IPA_ETH_PIPE_DIR_TX) {
				ipa_client_type =
					IPA_CLIENT_AQC_ETHERNET_CONS;
			} else {
				ipa_client_type =
					IPA_CLIENT_AQC_ETHERNET_PROD;
			}
		}
		break;
	case IPA_ETH_CLIENT_RTK8111K:
	case IPA_ETH_CLIENT_RTK8125B:
		if (client->traffic_type ==
			IPA_ETH_PIPE_BEST_EFFORT &&
			client->inst_id == 0) {
			if (pipe->dir == IPA_ETH_PIPE_DIR_TX) {
				ipa_client_type =
					IPA_CLIENT_RTK_ETHERNET_CONS;
			} else {
				ipa_client_type =
					IPA_CLIENT_RTK_ETHERNET_PROD;
			}
		}
		break;
	case IPA_ETH_CLIENT_NTN:
	case IPA_ETH_CLIENT_EMAC:
		if (client->traffic_type ==
			IPA_ETH_PIPE_BEST_EFFORT &&
			client->inst_id == 0) {
			if (pipe->dir == IPA_ETH_PIPE_DIR_TX) {
				ipa_client_type =
					IPA_CLIENT_ETHERNET_CONS;
			} else {
				ipa_client_type =
					IPA_CLIENT_ETHERNET_PROD;
			}
		}
		break;
#if IPA_ETH_API_VER >= 2
	case IPA_ETH_CLIENT_NTN3:
		if (client->traffic_type == IPA_ETH_PIPE_BEST_EFFORT) {
			if (client->inst_id == 0) {
				if (pipe->dir == IPA_ETH_PIPE_DIR_TX) {
					ipa_client_type = IPA_CLIENT_ETHERNET_CONS;
				} else {
					ipa_client_type = IPA_CLIENT_ETHERNET_PROD;
				}
			} else if (client->inst_id == 1) {
				if (pipe->dir == IPA_ETH_PIPE_DIR_TX) {
					ipa_client_type = IPA_CLIENT_ETHERNET2_CONS;
				} else {
					ipa_client_type = IPA_CLIENT_ETHERNET2_PROD;
				}
			}
		}
		break;
#endif
	default:
		IPA_ETH_ERR("invalid client type%d\n",
			client->client_type);
	}
	return ipa_client_type;
}

static struct ipa_eth_client_pipe_info
	*ipa_eth_get_pipe_from_hdl(ipa_eth_hdl_t hdl)
{
	struct ipa_eth_client_pipe_info *pipe;

	spin_lock(&ipa_eth_ctx->idr_lock);
	pipe = idr_find(&ipa_eth_ctx->idr, hdl);
	spin_unlock(&ipa_eth_ctx->idr_lock);

	return pipe;
}


static int ipa_eth_client_connect_pipe(
	struct ipa_eth_client_pipe_info *pipe)
{
	enum ipa_client_type client_type;
	struct ipa_eth_client *client;
	int ret;

	if (!pipe) {
		IPA_ETH_ERR("invalid pipe\n");
		return -EFAULT;
	}
	client = pipe->client_info;
	if (!client) {
		IPA_ETH_ERR("invalid client\n");
		return -EFAULT;
	}
	client_type =
		ipa_eth_get_ipa_client_type_from_pipe(pipe);
	if (client_type == IPA_CLIENT_MAX) {
		IPA_ETH_ERR("invalid client type\n");
		return -EFAULT;
	}

	if (pipe_connected[client_to_pipe_index(client_type)]) {
		IPA_ETH_ERR("client already connected\n");
		return -EFAULT;
	}

	pipe->pipe_hdl = ipa_eth_pipe_hdl_alloc((void *)pipe);

	ret = ipa3_eth_connect(pipe, client_type);
	if (!ret) {
		pipe_connected[client_to_pipe_index(client_type)] = true;
	}

	return ret;
}

static int ipa_eth_client_disconnect_pipe(
	struct ipa_eth_client_pipe_info *pipe)
{
	enum ipa_client_type client_type;
	struct ipa_eth_client *client;
	int result;

	if (!pipe) {
		IPA_ETH_ERR("invalid pipe\n");
		return -EFAULT;
	}

	client = pipe->client_info;
	if (!client) {
		IPA_ETH_ERR("invalid client\n");
		return -EFAULT;
	}
	client_type =
		ipa_eth_get_ipa_client_type_from_pipe(pipe);
	if (client_type == IPA_CLIENT_MAX) {
		IPA_ETH_ERR("invalid client type\n");
		return -EFAULT;
	}

	if (!pipe_connected[client_to_pipe_index(client_type)]) {
		IPA_ETH_ERR("client not connected\n");
		return -EFAULT;
	}

	result = ipa3_eth_disconnect(pipe, client_type);
	if (result)
		return result;

	pipe_connected[client_to_pipe_index(client_type)] = false;

	ipa_eth_pipe_hdl_remove(pipe->pipe_hdl);
	return 0;
}


static int ipa_eth_commit_partial_hdr(
	struct ipa_ioc_add_hdr *hdr,
	const char *netdev_name,
	struct ipa_eth_hdr_info *hdr_info)
{
	int i;

	if (!hdr || !hdr_info || !netdev_name) {
		IPA_ETH_ERR("Invalid input\n");
		return -EINVAL;
	}

	hdr->commit = 0;
	hdr->num_hdrs = 2;

	snprintf(hdr->hdr[0].name, sizeof(hdr->hdr[0].name),
			 "%s_ipv4", netdev_name);
	snprintf(hdr->hdr[1].name, sizeof(hdr->hdr[1].name),
			 "%s_ipv6", netdev_name);
	for (i = IPA_IP_v4; i < IPA_IP_MAX; i++) {
		hdr->hdr[i].hdr_len = hdr_info[i].hdr_len;
		memcpy(hdr->hdr[i].hdr, hdr_info[i].hdr, hdr->hdr[i].hdr_len);
		hdr->hdr[i].type = hdr_info[i].hdr_type;
		hdr->hdr[i].is_partial = 1;
		hdr->hdr[i].is_eth2_ofst_valid = 1;
		hdr->hdr[i].eth2_ofst = hdr_info[i].dst_mac_addr_offset;
	}

	if (ipa3_add_hdr(hdr)) {
		IPA_ETH_ERR("fail to add partial headers\n");
		return -EFAULT;
	}

	return 0;
}


static void ipa_eth_pm_cb(void *p, enum ipa_pm_cb_event event)
{
	IPA_ETH_ERR("received pm event %d\n", event);
}

static int ipa_eth_pm_register(struct ipa_eth_client *client)
{
	struct ipa_pm_register_params pm_params;
	int client_type, inst_id;
	char name[IPA_RESOURCE_NAME_MAX];
	struct ipa_eth_client_pipe_info *pipe;
	int rc;

	/* validate user input */
	if (!client || (client->client_type >= IPA_ETH_CLIENT_MAX)) {
		IPA_ETH_ERR("null client or eth client doesn't exist");
		return -EFAULT;
	}
	client_type = client->client_type;
	inst_id = client->inst_id;

	if (atomic_read(
		&ipa_eth_ctx->client[client_type][inst_id].ref_cnt))
		goto add_pipe_list;

	memset(&pm_params, 0, sizeof(pm_params));
	snprintf(name, IPA_RESOURCE_NAME_MAX,
		"ipa_eth_%d_%d", client_type, inst_id);
	pm_params.name = name;
	pm_params.callback = ipa_eth_pm_cb;
	pm_params.user_data = NULL;
	pm_params.group = IPA_PM_GROUP_DEFAULT;
	if (ipa_pm_register(&pm_params,
		&ipa_eth_ctx->client[client_type][inst_id].pm_hdl)) {
		IPA_ETH_ERR("fail to register ipa pm\n");
		return -EFAULT;
	}
	/* vote IPA clock on */
	rc = ipa_pm_activate_sync(
		ipa_eth_ctx->client[client_type][inst_id].pm_hdl);
	if (rc) {
		IPA_ETH_ERR("fail to activate ipa pm\n");
		return -EFAULT;
	}
add_pipe_list:
	list_for_each_entry(pipe, &client->pipe_list,
		link) {
		if (pipe->dir == IPA_ETH_PIPE_DIR_RX)
			continue;
		rc = ipa_pm_associate_ipa_cons_to_client(
			ipa_eth_ctx->client[client_type][inst_id].pm_hdl,
			ipa_eth_get_ipa_client_type_from_pipe(pipe));
		if (rc) {
			IPA_ETH_ERR("fail to associate cons with PM %d\n", rc);
			ipa_pm_deregister(
			ipa_eth_ctx->client[client_type][inst_id].pm_hdl);
			ipa_eth_ctx->client[client_type][inst_id].pm_hdl = 0;
			ipa_assert();
			return rc;
		}
	}
	atomic_inc(
		&ipa_eth_ctx->client[client_type][inst_id].ref_cnt);
	return 0;
}

static int ipa_eth_pm_deregister(struct ipa_eth_client *client)
{
	int rc;
	int client_type, inst_id;

	/* validate user input */
	if (!client || (client->client_type >= IPA_ETH_CLIENT_MAX)) {
		IPA_ETH_ERR("null client or client type not defined");
		return -EFAULT;
	}
	client_type = client->client_type;
	inst_id = client->inst_id;
	if (atomic_read(
		&ipa_eth_ctx->client[client_type][inst_id].ref_cnt)
		== 1) {
		rc = ipa_pm_deactivate_sync(
			ipa_eth_ctx->client[client_type][inst_id].pm_hdl);
		if (rc) {
			IPA_ETH_ERR("fail to deactivate ipa pm\n");
			return -EFAULT;
		}
		if (ipa_pm_deregister(
			ipa_eth_ctx->client[client_type][inst_id].pm_hdl)) {
			IPA_ETH_ERR("fail to deregister ipa pm\n");
			return -EFAULT;
		}
	}
	atomic_dec(&ipa_eth_ctx->client[client_type][inst_id].ref_cnt);
	return 0;
}

static int ipa_eth_client_conn_pipes_internal(struct ipa_eth_client *client)
{
	struct ipa_eth_client_pipe_info *pipe;
	int rc;
	int client_type, inst_id, traff_type;

	/* validate user input */
	if (!client || (client->client_type >= IPA_ETH_CLIENT_MAX)) {
		IPA_ETH_ERR("null client or client type not defined");
		return -EFAULT;
	}
	if (!ipa_eth_ctx) {
		IPA_ETH_ERR("connect called before register readiness\n");
		return -EFAULT;
	}

	if (!ipa_eth_ctx->is_eth_ready) {
		IPA_ETH_ERR("conn called before IPA eth ready\n");
		return -EFAULT;
	}
	ipa_eth_ctx->client_priv = client->priv;
	client_type = client->client_type;
	inst_id = client->inst_id;
	traff_type = client->traffic_type;
	IPA_ETH_DBG("ipa_eth conn client %d inst %d, traffic %d\n",
		client_type, inst_id, traff_type);
	mutex_lock(&ipa_eth_ctx->lock);
	rc = ipa_eth_pm_register(client);
	if (rc) {
		IPA_ETH_ERR("pm register failed\n");
		mutex_unlock(&ipa_eth_ctx->lock);
		return -EFAULT;
	}
	list_for_each_entry(pipe, &client->pipe_list,
		link) {
		rc = ipa_eth_client_connect_pipe(pipe);
		if (rc) {
			IPA_ETH_ERR("pipe connect fails\n");
			ipa_assert();
		}
	}
	if (!ipa_eth_ctx->client[client_type][inst_id].existed) {
		ipa3_eth_debugfs_add_node(client);
		ipa_eth_ctx->client[client_type][inst_id].existed = true;
	}
	mutex_unlock(&ipa_eth_ctx->lock);
	return 0;
}

static int ipa_eth_client_disconn_pipes_internal(struct ipa_eth_client *client)
{
	int rc;
	struct ipa_eth_client_pipe_info *pipe;
	struct ipa_ep_cfg_holb holb;

	/* validate user input */
	if (!client) {
		IPA_ETH_ERR("null client");
		return -EFAULT;
	}

	if (!ipa_eth_ctx) {
		IPA_ETH_ERR("disconn called before register readiness\n");
		return -EFAULT;
	}

	if (!ipa_eth_ctx->is_eth_ready) {
		IPA_ETH_ERR("disconn called before IPA eth ready\n");
		return -EFAULT;
	}
	IPA_ETH_DBG("ipa_eth disconn client %d inst %d, traffic %d\n",
		client->client_type, client->inst_id,
		client->traffic_type);
	mutex_lock(&ipa_eth_ctx->lock);

	/* set holb on tx pipes first */
	list_for_each_entry(pipe, &client->pipe_list,
		link) {
		if (pipe->dir == IPA_ETH_PIPE_DIR_TX)
		{
			IPA_ETH_DBG("Set holb on pipe = %d, pipe->dir = %d \n",
				ipa_get_ep_mapping(ipa_eth_get_ipa_client_type_from_pipe(pipe)),
				pipe->dir);
			holb.en = 1;
			holb.tmr_val = 0;
			ipa3_cfg_ep_holb(ipa_get_ep_mapping(
				ipa_eth_get_ipa_client_type_from_pipe(pipe)), &holb);
		}
	}

	list_for_each_entry(pipe, &client->pipe_list,
		link) {
		rc = ipa_eth_client_disconnect_pipe(pipe);
		if (rc) {
			IPA_ETH_ERR("pipe connect fails\n");
			ipa_assert();
		}
	}
	if (ipa_eth_pm_deregister(client)) {
		IPA_ETH_ERR("pm deregister failed\n");
		mutex_unlock(&ipa_eth_ctx->lock);
		return -EFAULT;
	}
	mutex_unlock(&ipa_eth_ctx->lock);
	return 0;
}

static void ipa_eth_msg_free_cb(void *buff, u32 len, u32 type)
{
	kfree(buff);
}

static int ipa_eth_client_conn_evt_internal(struct ipa_ecm_msg *msg)
{
	struct ipa_msg_meta msg_meta;
	struct ipa_ecm_msg *eth_msg;
	int ret;

	IPA_ETH_DBG("enter\n");

	eth_msg = kzalloc(sizeof(*eth_msg), GFP_KERNEL);
	if (eth_msg == NULL)
		return -ENOMEM;
	memcpy(eth_msg, msg, sizeof(struct ipa_ecm_msg));
	memset(&msg_meta, 0, sizeof(struct ipa_msg_meta));
	msg_meta.msg_len = sizeof(struct ipa_ecm_msg);
	msg_meta.msg_type = IPA_PERIPHERAL_CONNECT;

	IPA_ETH_DBG("send IPA_PERIPHERAL_CONNECT, len:%d, buff %pK", msg_meta.msg_len, eth_msg);
	ret = ipa_send_msg(&msg_meta, eth_msg, ipa_eth_msg_free_cb);

	IPA_ETH_DBG("exit\n");

	return ret;
}

static int ipa_eth_client_disconn_evt_internal(struct ipa_ecm_msg *msg)
{
	struct ipa_msg_meta msg_meta;
	struct ipa_ecm_msg *eth_msg;
	int ret;

	IPA_ETH_DBG("enter\n");

	eth_msg = kzalloc(sizeof(*eth_msg), GFP_KERNEL);
	if (eth_msg == NULL)
		return -ENOMEM;
	memcpy(eth_msg, msg, sizeof(struct ipa_ecm_msg));
	memset(&msg_meta, 0, sizeof(struct ipa_msg_meta));
	msg_meta.msg_len = sizeof(struct ipa_ecm_msg);
	msg_meta.msg_type = IPA_PERIPHERAL_DISCONNECT;

	IPA_ETH_DBG("send PERIPHERAL_DISCONNECT, len:%d, buff %pK", msg_meta.msg_len, eth_msg);
	ret = ipa_send_msg(&msg_meta, eth_msg, ipa_eth_msg_free_cb);

	IPA_ETH_DBG("exit\n");

	return ret;
}

static int ipa_eth_client_reg_intf_internal(struct ipa_eth_intf_info *intf)
{
	struct ipa_eth_intf *new_intf;
	struct ipa_eth_intf *entry;
	struct ipa_ioc_add_hdr *hdr;
	struct ipa_tx_intf tx;
	struct ipa_rx_intf rx;
	enum ipa_client_type tx_client[IPA_CLIENT_MAX] = {0};
	enum ipa_client_type rx_client[IPA_CLIENT_MAX] = {0};
	struct ipa_ioc_tx_intf_prop *tx_prop =  NULL;
	struct ipa_ioc_rx_intf_prop *rx_prop = NULL;
	struct ipa_eth_client_pipe_info *pipe;
	u32 len;
	int ret = 0, i;
#if IPA_ETH_API_VER >= 2
	struct ipa_ecm_msg msg;
	bool vlan_mode = false;
	struct ipa_eth_hdr_info intf_hdr[IPA_IP_MAX];
	struct ethhdr l_ethhdr[IPA_IP_MAX] = { 0 };
	struct vlan_ethhdr l_vlan_ethhdr[IPA_IP_MAX] = { 0 };
#endif

	if (intf == NULL) {
		IPA_ETH_ERR("invalid params intf=%pK\n", intf);
		return -EINVAL;
	}
	if (!ipa_eth_ctx) {
		IPA_ETH_ERR("disconn called before register readiness\n");
		return -EFAULT;
	}
#if IPA_ETH_API_VER >= 2
	if (!intf->client) {
		IPA_ETH_ERR("invalid intf->client\n");
		return -EFAULT;
	}
	if (!intf->client->net_dev) {
		IPA_ETH_ERR("invalid netdev\n");
		return -EFAULT;
	}
	if (!intf->net_dev)
		intf->net_dev = intf->client->net_dev;

	IPA_ETH_DBG("register interface for netdev %s\n", intf->net_dev->name);
	/* multiple attach support */
	if (strnstr(intf->net_dev->name, STR_ETH0_IFACE, strlen(intf->net_dev->name))) {
		ret = ipa3_is_vlan_mode(IPA_VLAN_IF_ETH0, &vlan_mode);
		if (ret) {
			IPA_ETH_ERR("Could not determine IPA VLAN mode\n");
			return ret;
		}
	} else if (strnstr(intf->net_dev->name, STR_ETH1_IFACE, strlen(intf->net_dev->name))) {
		ret = ipa3_is_vlan_mode(IPA_VLAN_IF_ETH1, &vlan_mode);
		if (ret) {
			IPA_ETH_ERR("Could not determine IPA VLAN mode\n");
			return ret;
		}
	} else {
		ret = ipa3_is_vlan_mode(IPA_VLAN_IF_ETH, &vlan_mode);
		if (ret) {
			IPA_ETH_ERR("Could not determine IPA VLAN mode\n");
			return ret;
		}
	}
#else
	IPA_ETH_DBG("register interface for netdev %s\n",
		intf->netdev_name);
#endif
	mutex_lock(&ipa_eth_ctx->lock);
	list_for_each_entry(entry, &ipa_eth_ctx->head_intf_list, link)
#if IPA_ETH_API_VER >= 2
		if (strcmp(entry->netdev_name, intf->net_dev->name) == 0) {
#else
		if (strcmp(entry->netdev_name, intf->netdev_name) == 0) {
#endif
			IPA_ETH_DBG("intf was added before.\n");
			mutex_unlock(&ipa_eth_ctx->lock);
			return 0;
		}
#if IPA_ETH_API_VER >= 2
	memset(intf_hdr, 0, sizeof(intf_hdr));
	if (!vlan_mode) {
		struct ethhdr *eth_h;

		intf_hdr[0].hdr = (u8 *)&l_ethhdr[0];
		eth_h = (struct ethhdr *) intf_hdr[0].hdr;
		memcpy(&eth_h->h_source, intf->net_dev->dev_addr, ETH_ALEN);
		eth_h->h_proto = htons(ETH_P_IP);
		intf_hdr[0].hdr_len = ETH_HLEN;
		intf_hdr[0].hdr_type = IPA_HDR_L2_ETHERNET_II;

		intf_hdr[1].hdr = (u8 *)&l_ethhdr[1];
		eth_h = (struct ethhdr *) intf_hdr[1].hdr;
		memcpy(&eth_h->h_source, intf->net_dev->dev_addr, ETH_ALEN);
		eth_h->h_proto = htons(ETH_P_IPV6);
		intf_hdr[1].hdr_len = ETH_HLEN;
		intf_hdr[1].hdr_type = IPA_HDR_L2_ETHERNET_II;
	} else {
		struct vlan_ethhdr *vlan_eth_h;

		intf_hdr[0].hdr = (u8 *)&l_vlan_ethhdr[0];
		vlan_eth_h = (struct vlan_ethhdr *) intf_hdr[0].hdr;
		memcpy(&vlan_eth_h->h_source, intf->net_dev->dev_addr, ETH_ALEN);
		vlan_eth_h->h_vlan_proto = htons(ETH_P_8021Q);
		vlan_eth_h->h_vlan_encapsulated_proto = htons(ETH_P_IP);
		intf_hdr[0].hdr_len = VLAN_ETH_HLEN;
		intf_hdr[0].hdr_type = IPA_HDR_L2_802_1Q;

		intf_hdr[1].hdr = (u8 *)&l_vlan_ethhdr[1];
		vlan_eth_h = (struct vlan_ethhdr *) intf_hdr[1].hdr;
		memcpy(&vlan_eth_h->h_source, intf->net_dev->dev_addr, ETH_ALEN);
		vlan_eth_h->h_vlan_proto = htons(ETH_P_8021Q);
		vlan_eth_h->h_vlan_encapsulated_proto = htons(ETH_P_IPV6);
		intf_hdr[1].hdr_len = VLAN_ETH_HLEN;
		intf_hdr[1].hdr_type = IPA_HDR_L2_802_1Q;;
	}
#endif
	new_intf = kzalloc(sizeof(*new_intf), GFP_KERNEL);
	if (new_intf == NULL) {
		IPA_ETH_ERR("fail to alloc new intf\n");
		mutex_unlock(&ipa_eth_ctx->lock);
		return -ENOMEM;
	}
	INIT_LIST_HEAD(&new_intf->link);
#if IPA_ETH_API_VER >= 2
	strlcpy(new_intf->netdev_name, intf->net_dev->name, sizeof(new_intf->netdev_name));
	new_intf->hdr_len = intf_hdr[0].hdr_len;
#else
	strlcpy(new_intf->netdev_name, intf->netdev_name,
		sizeof(new_intf->netdev_name));
	new_intf->hdr_len = intf->hdr[0].hdr_len;
#endif
	/* add partial header */
	len = sizeof(struct ipa_ioc_add_hdr) + 2 * sizeof(struct ipa_hdr_add);
	hdr = kzalloc(len, GFP_KERNEL);
	if (hdr == NULL) {
		IPA_ETH_ERR("fail to alloc %d bytes\n", len);
		ret = -EFAULT;
		goto fail_alloc_hdr;
	}
#if IPA_ETH_API_VER >= 2
	if (ipa_eth_commit_partial_hdr(hdr, intf->net_dev->name, (struct ipa_eth_hdr_info *)intf_hdr)) {
#else
	if (ipa_eth_commit_partial_hdr(hdr,
		intf->netdev_name, intf->hdr)) {
#endif
		IPA_ETH_ERR("fail to commit partial headers\n");
		ret = -EFAULT;
		goto fail_commit_hdr;
	}

	new_intf->partial_hdr_hdl[IPA_IP_v4] = hdr->hdr[IPA_IP_v4].hdr_hdl;
	new_intf->partial_hdr_hdl[IPA_IP_v6] = hdr->hdr[IPA_IP_v6].hdr_hdl;
	IPA_ETH_DBG("IPv4 hdr hdl: %d IPv6 hdr hdl: %d\n",
		hdr->hdr[IPA_IP_v4].hdr_hdl, hdr->hdr[IPA_IP_v6].hdr_hdl);

	memset(&tx, 0, sizeof(struct ipa_tx_intf));
	memset(&rx, 0, sizeof(struct ipa_rx_intf));
#if IPA_ETH_API_VER >= 2
	list_for_each_entry(pipe, &intf->client->pipe_list, link) {
#else
	for (i = 0; i < intf->pipe_hdl_list_size; i++) {
		pipe = ipa_eth_get_pipe_from_hdl(intf->pipe_hdl_list[i]);
#endif
		if (pipe->dir == IPA_ETH_PIPE_DIR_TX) {
			tx_client[tx.num_props] =
				ipa_eth_get_ipa_client_type_from_pipe(pipe);
			tx.num_props++;
		} else {
			rx_client[rx.num_props] =
				ipa_eth_get_ipa_client_type_from_pipe(pipe);
			rx.num_props++;
		}
	}
	/* populate tx prop */
	if (tx.num_props) {
		tx_prop = kmalloc(
			sizeof(*tx_prop) * tx.num_props *
			IPA_IP_MAX, GFP_KERNEL);
		if (!tx_prop) {
			IPAERR("failed to allocate memory\n");
			ret = -ENOMEM;
			goto fail_commit_hdr;
		}
		memset(tx_prop, 0, sizeof(*tx_prop) *
			tx.num_props * IPA_IP_MAX);
		tx.prop = tx_prop;
		for (i = 0; i < tx.num_props; i++) {
			tx_prop[i].ip = IPA_IP_v4;
			tx_prop[i].dst_pipe = tx_client[i];
#if IPA_ETH_API_VER >= 2
			tx_prop[i].hdr_l2_type = intf_hdr[0].hdr_type;
#else
			tx_prop[i].hdr_l2_type = intf->hdr[0].hdr_type;
#endif
			strlcpy(tx_prop[i].hdr_name, hdr->hdr[IPA_IP_v4].name,
				sizeof(tx_prop[i].hdr_name));

			tx_prop[i+1].ip = IPA_IP_v6;
			tx_prop[i+1].dst_pipe = tx_client[i];
#if IPA_ETH_API_VER >= 2
			tx_prop[i+1].hdr_l2_type = intf_hdr[1].hdr_type;
#else
			tx_prop[i+1].hdr_l2_type = intf->hdr[1].hdr_type;
#endif
			strlcpy(tx_prop[i+1].hdr_name, hdr->hdr[IPA_IP_v6].name,
				sizeof(tx_prop[i+1].hdr_name));
		}
	}
	/* populate rx prop */
	if (rx.num_props) {
		rx_prop = kmalloc(
			sizeof(*rx_prop) * rx.num_props *
			IPA_IP_MAX, GFP_KERNEL);
		if (!rx_prop) {
			IPAERR("failed to allocate memory\n");
			ret = -ENOMEM;
			goto fail_commit_hdr;
		}
		memset(rx_prop, 0, sizeof(*rx_prop) *
			rx.num_props * IPA_IP_MAX);
		rx.prop = rx_prop;
		for (i = 0; i < rx.num_props; i++) {
			rx_prop[i].ip = IPA_IP_v4;
			rx_prop[i].src_pipe = rx_client[i];
#if IPA_ETH_API_VER >= 2
			rx_prop[i].hdr_l2_type = intf_hdr[0].hdr_type;
#else
			rx_prop[i].hdr_l2_type = intf->hdr[0].hdr_type;
#endif
			rx_prop[i+1].ip = IPA_IP_v6;
			rx_prop[i+1].src_pipe = rx_client[i];
#if IPA_ETH_API_VER >= 2
			rx_prop[i+1].hdr_l2_type = intf_hdr[1].hdr_type;
#else
			rx_prop[i+1].hdr_l2_type = intf->hdr[1].hdr_type;
#endif
		}
		tx.num_props *= IPA_IP_MAX;
		rx.num_props *= IPA_IP_MAX;
	}
#if IPA_ETH_API_VER >= 2
	if (ipa_register_intf(intf->net_dev->name, &tx, &rx)) {
#else
	if (ipa_register_intf(intf->netdev_name, &tx, &rx)) {
#endif
		IPA_ETH_ERR("fail to add interface prop\n");
		ret = -EFAULT;
		goto fail_commit_hdr;
	}

	list_add(&new_intf->link, &ipa_eth_ctx->head_intf_list);

	kfree(hdr);
	kfree(tx_prop);
	kfree(rx_prop);
	mutex_unlock(&ipa_eth_ctx->lock);

#if IPA_ETH_API_VER >= 2
	if (intf->is_conn_evt) {
		strlcpy(msg.name, intf->net_dev->name, sizeof(msg.name));
		msg.ifindex = intf->net_dev->ifindex;
		ipa_eth_client_conn_evt_internal(&msg);
	}
#endif
	return 0;
fail_commit_hdr:
	kfree(hdr);
	kfree(tx_prop);
	kfree(rx_prop);
fail_alloc_hdr:
	kfree(new_intf);
	mutex_unlock(&ipa_eth_ctx->lock);
	return ret;
}

static int ipa_eth_client_unreg_intf_internal(struct ipa_eth_intf_info *intf)
{
	int len, ret = 0;
	struct ipa_ioc_del_hdr *hdr = NULL;
	struct ipa_eth_intf *entry;
	struct ipa_eth_intf *next;
#if IPA_ETH_API_VER >= 2
	struct ipa_ecm_msg msg;
#endif

	if (intf == NULL) {
		IPA_ETH_ERR("invalid params intf=%pK\n", intf);
		return -EINVAL;
	}
	if (!ipa_eth_ctx) {
		IPA_ETH_ERR("disconn called before register readiness\n");
		return -EFAULT;
	}
#if IPA_ETH_API_VER >= 2
	if (!intf->net_dev) {
		IPA_ETH_ERR("invalid netdev\n");
		return -EFAULT;
	}
	IPA_ETH_DBG("unregister interface for netdev %s\n", intf->net_dev->name);
#else
	IPA_ETH_DBG("unregister interface for netdev %s\n",
		intf->netdev_name);
#endif
	mutex_lock(&ipa_eth_ctx->lock);
	list_for_each_entry_safe(entry, next, &ipa_eth_ctx->head_intf_list,
		link)
#if IPA_ETH_API_VER >= 2
		if (strcmp(entry->netdev_name, intf->net_dev->name) == 0) {
#else
		if (strcmp(entry->netdev_name, intf->netdev_name) == 0) {
#endif
			len = sizeof(struct ipa_ioc_del_hdr) +
				IPA_IP_MAX * sizeof(struct ipa_hdr_del);
			hdr = kzalloc(len, GFP_KERNEL);
			if (hdr == NULL) {
				IPA_ETH_ERR("fail to alloc %d bytes\n", len);
				mutex_unlock(&ipa_eth_ctx->lock);
				return -ENOMEM;
			}

			hdr->commit = 1;
			hdr->num_hdls = 2;
			hdr->hdl[0].hdl = entry->partial_hdr_hdl[0];
			hdr->hdl[1].hdl = entry->partial_hdr_hdl[1];
			IPA_ETH_DBG("IPv4 hdr hdl: %d IPv6 hdr hdl: %d\n",
				hdr->hdl[0].hdl, hdr->hdl[1].hdl);

			if (ipa3_del_hdr(hdr)) {
				IPA_ETH_ERR("fail to delete partial header\n");
				ret = -EFAULT;
				goto fail;
			}

			if (ipa3_deregister_intf(entry->netdev_name)) {
				IPA_ETH_ERR("fail to del interface props\n");
				ret = -EFAULT;
				goto fail;
			}

			list_del(&entry->link);
			kfree(entry);

			break;
		}
fail:
	kfree(hdr);
	mutex_unlock(&ipa_eth_ctx->lock);
#if IPA_ETH_API_VER >= 2
	if (intf->is_conn_evt) {
		strlcpy(msg.name, intf->net_dev->name, sizeof(msg.name));
		msg.ifindex = intf->net_dev->ifindex;
		ipa_eth_client_disconn_evt_internal(&msg);
	}
#endif
	return ret;

}

static int ipa_eth_client_set_perf_profile_internal(struct ipa_eth_client *client,
	struct ipa_eth_perf_profile *profile)
{
	int client_type, inst_id;

	if ((!profile) || (!client) || (client->client_type >= IPA_ETH_CLIENT_MAX)) {
		IPA_ETH_ERR("Invalid input\n");
		return -EINVAL;
	}

	client_type = client->client_type;
	inst_id = client->inst_id;

	if (ipa_pm_set_throughput(
		ipa_eth_ctx->client[client_type][inst_id].pm_hdl,
		profile->max_supported_bw_mbps)) {
		IPA_ETH_ERR("fail to set pm throughput\n");
		return -EFAULT;
	}

	return 0;
}

enum ipa_client_type ipa_eth_get_ipa_client_type_from_eth_type_internal(
	enum ipa_eth_client_type eth_client_type, enum ipa_eth_pipe_direction dir)
{
	int ipa_client_type = IPA_CLIENT_MAX;

	switch (eth_client_type) {
	case IPA_ETH_CLIENT_AQC107:
	case IPA_ETH_CLIENT_AQC113:
		if (dir == IPA_ETH_PIPE_DIR_TX) {
			ipa_client_type =
				IPA_CLIENT_AQC_ETHERNET_CONS;
		} else {
			ipa_client_type =
				IPA_CLIENT_AQC_ETHERNET_PROD;
		}
		break;
	case IPA_ETH_CLIENT_RTK8111K:
	case IPA_ETH_CLIENT_RTK8125B:
			if (dir == IPA_ETH_PIPE_DIR_TX) {
				ipa_client_type =
					IPA_CLIENT_RTK_ETHERNET_CONS;
			} else {
				ipa_client_type =
					IPA_CLIENT_RTK_ETHERNET_PROD;
			}
		break;
	case IPA_ETH_CLIENT_NTN:
	case IPA_ETH_CLIENT_EMAC:
			if (dir == IPA_ETH_PIPE_DIR_TX) {
				ipa_client_type =
					IPA_CLIENT_ETHERNET_CONS;
			} else {
				ipa_client_type =
					IPA_CLIENT_ETHERNET_PROD;
			}
		break;
	default:
		IPA_ETH_ERR("invalid client type%d\n",
			eth_client_type);
	}
	return ipa_client_type;
}

bool ipa_eth_client_exist_internal(enum ipa_eth_client_type eth_client_type, int inst_id)
{
	if (ipa_eth_ctx)
		return ipa_eth_ctx->client[eth_client_type][inst_id].existed;
	else return false;
}

void ipa_eth_register(void)
{
	struct ipa_eth_data funcs;

	funcs.ipa_eth_register_ready_cb = ipa_eth_register_ready_cb_internal;
	funcs.ipa_eth_unregister_ready_cb =
		ipa_eth_unregister_ready_cb_internal;
	funcs.ipa_eth_client_conn_pipes = ipa_eth_client_conn_pipes_internal;
	funcs.ipa_eth_client_disconn_pipes =
		ipa_eth_client_disconn_pipes_internal;
	funcs.ipa_eth_client_reg_intf = ipa_eth_client_reg_intf_internal;
	funcs.ipa_eth_client_unreg_intf = ipa_eth_client_unreg_intf_internal;
	funcs.ipa_eth_client_set_perf_profile =
		ipa_eth_client_set_perf_profile_internal;
#if IPA_ETH_API_VER < 2
	funcs.ipa_eth_client_conn_evt = ipa_eth_client_conn_evt_internal;
	funcs.ipa_eth_client_disconn_evt = ipa_eth_client_disconn_evt_internal;
#endif
	funcs.ipa_eth_get_ipa_client_type_from_eth_type =
		ipa_eth_get_ipa_client_type_from_eth_type_internal;
	funcs.ipa_eth_client_exist = ipa_eth_client_exist_internal;

	if (ipa_fmwk_register_ipa_eth(&funcs))
		pr_err("failed to register ipa_eth APIs\n");
}
