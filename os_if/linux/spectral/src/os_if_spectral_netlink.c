/*
 * Copyright (c) 2011, 2017-2019 The Linux Foundation. All rights reserved.
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

#include <os_if_spectral_netlink.h>
#include <spectral_cmn_api_i.h>
#include <spectral_defs_i.h>
#include <wlan_nlink_srv.h>
#include <wlan_nlink_common.h>
#include <qdf_module.h>
#ifdef CNSS_GENL
#include <net/cnss_nl.h>
#endif
#include <wlan_cfg80211.h>

/**
 * os_if_spectral_remove_nbuf_debug_entry() - Remove nbuf from nbuf debug table
 * @nbuf - nbuf to remove from the nbuf debug table
 *
 * Remove nbuf from the nbuf debug hash table and decrement the nbuf count
 *
 * Return: None
 */
static inline void os_if_spectral_remove_nbuf_debug_entry(qdf_nbuf_t nbuf)
{
	qdf_nbuf_count_dec(nbuf);
	qdf_net_buf_debug_release_skb(nbuf);
}

#ifndef CNSS_GENL
static struct sock *os_if_spectral_nl_sock;
static atomic_t spectral_nl_users = ATOMIC_INIT(0);
#endif

#if (KERNEL_VERSION(2, 6, 31) > LINUX_VERSION_CODE)
void
os_if_spectral_nl_data_ready(struct sock *sk, int len)
{
	spectral_debug("%d", __LINE__);
}

#else
void
os_if_spectral_nl_data_ready(struct sk_buff *skb)
{
	spectral_debug("%d", __LINE__);
}
#endif				/* VERSION */

#ifndef CNSS_GENL
/**
 * os_if_spectral_init_nl_cfg() - Initialize netlink kernel
 * configuration parameters
 * @cfg : Pointer to netlink_kernel_cfg
 *
 * Initialize netlink kernel configuration parameters required
 * for spectral module
 *
 * Return: None
 */
#if KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE
static void
os_if_spectral_init_nl_cfg(struct netlink_kernel_cfg *cfg)
{
	cfg->groups = 1;
	cfg->input = os_if_spectral_nl_data_ready;
}
#else
static void
os_if_spectral_init_nl_cfg(struct netlink_kernel_cfg *cfg)
{
}
#endif
/**
 * os_if_spectral_create_nl_sock() - Create Netlink socket
 * @cfg : Pointer to netlink_kernel_cfg
 *
 * Create Netlink socket required for spectral module
 *
 * Return: None
 */
#if KERNEL_VERSION(3, 7, 0) <= LINUX_VERSION_CODE
static void
os_if_spectral_create_nl_sock(struct netlink_kernel_cfg *cfg)
{
	os_if_spectral_nl_sock =
	    (struct sock *)netlink_kernel_create(&init_net,
						 SPECTRAL_NETLINK, cfg);
}
#elif KERNEL_VERSION(3, 6, 0) <= LINUX_VERSION_CODE
static void
os_if_spectral_create_nl_sock(struct netlink_kernel_cfg *cfg)
{
	os_if_spectral_nl_sock =
	    (struct sock *)netlink_kernel_create(&init_net,
						 SPECTRAL_NETLINK,
						 THIS_MODULE, cfg);
}
#elif (KERNEL_VERSION(2, 6, 31) > LINUX_VERSION_CODE)
static void
os_if_spectral_create_nl_sock(struct netlink_kernel_cfg *cfg)
{
	os_if_spectral_nl_sock =
	    (struct sock *)netlink_kernel_create(
		SPECTRAL_NETLINK, 1,
		&os_if_spectral_nl_data_ready,
		THIS_MODULE);
}
#else
#if (KERNEL_VERSION(3, 10, 0) <= LINUX_VERSION_CODE)
static void
os_if_spectral_create_nl_sock(struct netlink_kernel_cfg *cfg)
{
	memset(cfg, 0, sizeof(*cfg));
	cfg->groups = 1;
	cfg->input = &os_if_spectral_nl_data_ready;
	os_if_spectral_nl_sock =
	    (struct sock *)netlink_kernel_create(&init_net,
						 SPECTRAL_NETLINK, cfg);
}
#else
static void
os_if_spectral_create_nl_sock(struct netlink_kernel_cfg *cfg)
{
	os_if_spectral_nl_sock =
	    (struct sock *)netlink_kernel_create(
		&init_net,
		SPECTRAL_NETLINK, 1,
		&os_if_spectral_nl_data_ready,
		NULL, THIS_MODULE);
}
#endif
#endif

/**
 * os_if_spectral_init_nl() - Initialize netlink data structures for
 * spectral module
 * @pdev : Pointer to pdev
 *
 * Return: 0 on success else failure
 */
static int
os_if_spectral_init_nl(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_spectral *ps = NULL;
	struct netlink_kernel_cfg cfg;

	memset(&cfg, 0, sizeof(cfg));
	if (!pdev) {
		osif_err("PDEV is NULL!");
		return -EINVAL;
	}
	ps = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						   WLAN_UMAC_COMP_SPECTRAL);

	if (!ps) {
		osif_err("PDEV SPECTRAL object is NULL!");
		return -EINVAL;
	}
	os_if_spectral_init_nl_cfg(&cfg);

	if (!os_if_spectral_nl_sock) {
		os_if_spectral_create_nl_sock(&cfg);

		if (!os_if_spectral_nl_sock) {
			osif_err("NETLINK_KERNEL_CREATE FAILED");
			return -ENODEV;
		}
	}
	ps->spectral_sock = os_if_spectral_nl_sock;

	if (!ps->spectral_sock) {
		osif_err("ps->spectral_sock is NULL");
		return -ENODEV;
	}
	atomic_inc(&spectral_nl_users);

	return 0;
}

/**
 * os_if_spectral_destroy_netlink() - De-initialize netlink data structures for
 * spectral module
 * @pdev : Pointer to pdev
 *
 * Return: Success/Failure
 */
static int
os_if_spectral_destroy_netlink(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_spectral *ps = NULL;

	if (!pdev) {
		osif_err("PDEV is NULL!");
		return -EINVAL;
	}
	ps = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						   WLAN_UMAC_COMP_SPECTRAL);

	if (!ps) {
		osif_err("PDEV SPECTRAL object is NULL!");
		return -EINVAL;
	}
	ps->spectral_sock = NULL;
	if (atomic_dec_and_test(&spectral_nl_users)) {
		sock_release(os_if_spectral_nl_sock->sk_socket);
		os_if_spectral_nl_sock = NULL;
	}
	return 0;
}
#else

static int
os_if_spectral_init_nl(struct wlan_objmgr_pdev *pdev)
{
	return 0;
}

static int
os_if_spectral_destroy_netlink(struct wlan_objmgr_pdev *pdev)
{
	return 0;
}
#endif

void *
os_if_spectral_prep_skb(struct wlan_objmgr_pdev *pdev,
			enum spectral_msg_type smsg_type,
			enum spectral_msg_buf_type buf_type)
{
	struct pdev_spectral *ps = NULL;
	struct nlmsghdr *spectral_nlh = NULL;
	void *buf = NULL;

	if (!pdev) {
		osif_err("PDEV is NULL!");
		return NULL;
	}

	if (smsg_type >= SPECTRAL_MSG_TYPE_MAX) {
		osif_err("Invalid Spectral message type %u", smsg_type);
		return NULL;
	}

	if (buf_type >= SPECTRAL_MSG_BUF_TYPE_MAX) {
		osif_err("Invalid Spectral message buffer type %u",
			 buf_type);
		return NULL;
	}

	ps = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						   WLAN_UMAC_COMP_SPECTRAL);

	if (!ps) {
		osif_err("PDEV SPECTRAL object is NULL!");
		return NULL;
	}

	if (buf_type == SPECTRAL_MSG_BUF_NEW) {
		QDF_ASSERT(!ps->skb[smsg_type]);
		ps->skb[smsg_type] =
				qdf_nbuf_alloc(NULL, MAX_SPECTRAL_PAYLOAD,
					       0, 0, false);

		if (!ps->skb[smsg_type]) {
			osif_err("alloc skb (len=%u, msg_type=%u) failed",
				 MAX_SPECTRAL_PAYLOAD, smsg_type);
			return NULL;
		}

		qdf_nbuf_put_tail(ps->skb[smsg_type], MAX_SPECTRAL_PAYLOAD);
		spectral_nlh = (struct nlmsghdr *)ps->skb[smsg_type]->data;

		qdf_mem_zero(spectral_nlh, sizeof(*spectral_nlh));

		/*
		 * Possible bug that size of  struct spectral_samp_msg and
		 * SPECTRAL_MSG differ by 3 bytes  so we miss 3 bytes
		 */

		spectral_nlh->nlmsg_len =
				NLMSG_SPACE(sizeof(struct spectral_samp_msg));
		spectral_nlh->nlmsg_pid = 0;
		spectral_nlh->nlmsg_flags = 0;
		spectral_nlh->nlmsg_type = WLAN_NL_MSG_SPECTRAL_SCAN;

		qdf_mem_zero(NLMSG_DATA(spectral_nlh),
			     sizeof(struct spectral_samp_msg));
		buf = NLMSG_DATA(spectral_nlh);
	} else if (buf_type == SPECTRAL_MSG_BUF_SAVED) {
		QDF_ASSERT(ps->skb[smsg_type]);
		spectral_nlh = (struct nlmsghdr *)ps->skb[smsg_type]->data;
		buf = NLMSG_DATA(spectral_nlh);
	} else {
		osif_err("Failed to get spectral report buffer");
		buf = NULL;
	}

	return buf;
}

#if (KERNEL_VERSION(2, 6, 31) > LINUX_VERSION_CODE)
static inline void
os_if_init_spectral_skb_dst_pid(
	struct sk_buff *skb,
	struct pdev_spectral *ps)
{
	NETLINK_CB(skb).dst_pid =
	    ps->spectral_pid;
}
#else
static inline void
os_if_init_spectral_skb_dst_pid(
	struct sk_buff *skb,
	struct pdev_spectral *ps)
{
}
#endif			/* VERSION - field deprecated by newer kernels */

#if KERNEL_VERSION(3, 7, 0) > LINUX_VERSION_CODE
static inline void
os_if_init_spectral_skb_pid_portid(struct sk_buff *skb)
{
	NETLINK_CB(skb).pid = 0;  /* from kernel */
}
#else
static inline void
os_if_init_spectral_skb_pid_portid(struct sk_buff *skb)
{
	NETLINK_CB(skb).portid = 0;  /* from kernel */
}
#endif


/**
 * os_if_spectral_nl_unicast_msg() - Sends unicast Spectral message to user
 * space
 * @pdev : Pointer to pdev
 * @smsg_type: Spectral message type
 *
 * Return: void
 */
#ifndef CNSS_GENL
static int
os_if_spectral_nl_unicast_msg(struct wlan_objmgr_pdev *pdev,
			      enum spectral_msg_type smsg_type)
{
	struct pdev_spectral *ps = NULL;
	int status;

	if (!pdev) {
		osif_err("PDEV is NULL!");
		return -EINVAL;
	}

	if (smsg_type >= SPECTRAL_MSG_TYPE_MAX) {
		osif_err("Invalid Spectral message type %u", smsg_type);
		return -EINVAL;
	}

	ps = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						   WLAN_UMAC_COMP_SPECTRAL);
	if (!ps) {
		osif_err("PDEV SPECTRAL object is NULL!");
		return -EINVAL;
	}

	if (!ps->skb[smsg_type]) {
		osif_err("Socket buffer is null, msg_type= %u", smsg_type);
		return -EINVAL;
	}

	if (!ps->spectral_sock) {
		osif_err("Spectral Socket is invalid, msg_type= %u",
			 smsg_type);
		qdf_nbuf_free(ps->skb[smsg_type]);
		ps->skb[smsg_type] = NULL;

		return -EINVAL;
	}

	os_if_init_spectral_skb_dst_pid(ps->skb[smsg_type], ps);

	os_if_init_spectral_skb_pid_portid(ps->skb[smsg_type]);

	/* to mcast group 1<<0 */
	NETLINK_CB(ps->skb[smsg_type]).dst_group = 0;

	os_if_spectral_remove_nbuf_debug_entry(ps->skb[smsg_type]);
	status = netlink_unicast(ps->spectral_sock,
				 ps->skb[smsg_type],
				 ps->spectral_pid, MSG_DONTWAIT);

	/* clear the local copy, free would be done by netlink layer */
	ps->skb[smsg_type] = NULL;

	return status;
}
#else

static int
os_if_spectral_nl_unicast_msg(struct wlan_objmgr_pdev *pdev,
			      enum spectral_msg_type smsg_type)
{
	struct pdev_spectral *ps = NULL;
	int status;

	if (!pdev) {
		osif_err("PDEV is NULL!");
		return -EINVAL;
	}

	if (smsg_type >= SPECTRAL_MSG_TYPE_MAX) {
		osif_err("Invalid Spectral message type %u", smsg_type);
		return -EINVAL;
	}

	ps = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						   WLAN_UMAC_COMP_SPECTRAL);
	if (!ps) {
		osif_err("PDEV SPECTRAL object is NULL!");
		return -EINVAL;
	}

	if (!ps->skb[smsg_type]) {
		osif_err("Socket buffer is null, msg_type= %u", smsg_type);
		return -EINVAL;
	}

	os_if_init_spectral_skb_pid_portid(ps->skb[smsg_type]);

	os_if_spectral_remove_nbuf_debug_entry(ps->skb[smsg_type]);
	status = nl_srv_ucast(ps->skb[smsg_type], ps->spectral_pid,
			      MSG_DONTWAIT, WLAN_NL_MSG_SPECTRAL_SCAN,
			      CLD80211_MCGRP_OEM_MSGS);
	if (status < 0)
		osif_err("failed to send to spectral scan app");

	/* clear the local copy, free would be done by netlink layer */
	ps->skb[smsg_type] = NULL;

	return status;
}

#endif

/**
 * os_if_spectral_nl_bcast_msg() - Sends broadcast Spectral message to user
 * space
 * @pdev : Pointer to pdev
 * @smsg_type: Spectral message type
 *
 * Return: void
 */
static int
os_if_spectral_nl_bcast_msg(struct wlan_objmgr_pdev *pdev,
			    enum spectral_msg_type smsg_type)
{
#if (KERNEL_VERSION(2, 6, 31) >= LINUX_VERSION_CODE)
	fd_set write_set;
#endif
	int status;
	struct pdev_spectral *ps = NULL;

#if (KERNEL_VERSION(2, 6, 31) >= LINUX_VERSION_CODE)
	FD_ZERO(&write_set);
#endif

	if (!pdev) {
		osif_err("PDEV is NULL!");
		return -EINVAL;
	}

	if (smsg_type >= SPECTRAL_MSG_TYPE_MAX) {
		osif_err("Invalid Spectral message type %u", smsg_type);
		return -EINVAL;
	}

	ps = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						   WLAN_UMAC_COMP_SPECTRAL);

	if (!ps) {
		osif_err("PDEV SPECTRAL object is NULL!");
		return -EINVAL;
	}

	if (!ps->skb[smsg_type]) {
		osif_err("Socket buffer is null, msg_type= %u", smsg_type);
		return -EINVAL;
	}

	if (!ps->spectral_sock) {
		qdf_nbuf_free(ps->skb[smsg_type]);
		ps->skb[smsg_type] = NULL;

		return -EINVAL;
	}

	os_if_spectral_remove_nbuf_debug_entry(ps->skb[smsg_type]);
	status = netlink_broadcast(ps->spectral_sock,
				   ps->skb[smsg_type],
				   0, 1, GFP_ATOMIC);

	/* clear the local copy, free would be done by netlink layer */
	ps->skb[smsg_type] = NULL;

	return status;
}

/**
 * os_if_spectral_free_skb() - Free spectral SAMP message skb
 *
 * @pdev : Pointer to pdev
 * @smsg_type: Spectral message type
 *
 * Return: void
 */
static void
os_if_spectral_free_skb(struct wlan_objmgr_pdev *pdev,
			enum spectral_msg_type smsg_type)
{
	struct pdev_spectral *ps = NULL;

	if (!pdev) {
		osif_err("PDEV is NULL!");
		return;
	}

	if (smsg_type >= SPECTRAL_MSG_TYPE_MAX) {
		osif_err("Invalid Spectral message type %u", smsg_type);
		return;
	}

	ps = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						   WLAN_UMAC_COMP_SPECTRAL);

	if (!ps) {
		osif_err("PDEV SPECTRAL object is NULL!");
		return;
	}

	if (!ps->skb[smsg_type]) {
		osif_info("Socket buffer is null, msg_type= %u", smsg_type);
		return;
	}

	/* Free buffer */
	qdf_nbuf_free(ps->skb[smsg_type]);

	/* clear the local copy */
	ps->skb[smsg_type] = NULL;
}

qdf_export_symbol(os_if_spectral_free_skb);

void
os_if_spectral_netlink_init(struct wlan_objmgr_pdev *pdev)
{
	struct spectral_nl_cb nl_cb = {0};
	struct spectral_context *sptrl_ctx;

	if (!pdev) {
		osif_err("PDEV is NULL!");
		return;
	}

	sptrl_ctx = spectral_get_spectral_ctx_from_pdev(pdev);

	if (!sptrl_ctx) {
		osif_err("Spectral context is NULL!");
		return;
	}

	os_if_spectral_init_nl(pdev);

	/* Register Netlink handlers */
	nl_cb.get_sbuff = os_if_spectral_prep_skb;
	nl_cb.send_nl_bcast = os_if_spectral_nl_bcast_msg;
	nl_cb.send_nl_unicast = os_if_spectral_nl_unicast_msg;
	nl_cb.free_sbuff = os_if_spectral_free_skb;

	if (sptrl_ctx->sptrlc_register_netlink_cb)
		sptrl_ctx->sptrlc_register_netlink_cb(pdev, &nl_cb);
}
qdf_export_symbol(os_if_spectral_netlink_init);

void os_if_spectral_netlink_deinit(struct wlan_objmgr_pdev *pdev)
{
	struct spectral_context *sptrl_ctx;
	enum spectral_msg_type msg_type = SPECTRAL_MSG_NORMAL_MODE;

	if (!pdev) {
		osif_err("PDEV is NULL!");
		return;
	}

	sptrl_ctx = spectral_get_spectral_ctx_from_pdev(pdev);

	if (!sptrl_ctx) {
		osif_err("Spectral context is NULL!");
		return;
	}

	for (; msg_type < SPECTRAL_MSG_TYPE_MAX; msg_type++)
		os_if_spectral_free_skb(pdev, msg_type);

	if (sptrl_ctx->sptrlc_deregister_netlink_cb)
		sptrl_ctx->sptrlc_deregister_netlink_cb(pdev);

	os_if_spectral_destroy_netlink(pdev);
}
qdf_export_symbol(os_if_spectral_netlink_deinit);
