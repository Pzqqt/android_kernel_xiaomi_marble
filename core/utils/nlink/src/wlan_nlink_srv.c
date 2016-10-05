/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

/******************************************************************************
* wlan_nlink_srv.c
*
* This file contains the definitions specific to the wlan_nlink_srv
*
******************************************************************************/

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <wlan_nlink_srv.h>
#include <qdf_trace.h>

#if defined(CONFIG_CNSS_LOGGER)

#include <net/cnss_logger.h>

static int radio_idx = -EINVAL;
static void *wiphy_ptr;
static bool logger_initialized;

/**
 * nl_srv_init() - wrapper function to register to cnss_logger
 * @wiphy:	the pointer to the wiphy structure
 *
 * The netlink socket is no longer initialized in the driver itself, instead
 * will be initialized in the cnss_logger module, the driver should register
 * itself to cnss_logger module to get the radio_index for all the netlink
 * operation. (cfg80211 vendor command is using different netlink socket).
 *
 * The cnss_logger_device_register() use to register the driver with the
 * wiphy structure and the module name (debug purpose) and then return the
 * radio_index depending on the availibility.
 *
 * Return: radio index for success and -EINVAL for failure
 */
int nl_srv_init(void *wiphy)
{
	if (logger_initialized)
		goto initialized;

	wiphy_ptr = wiphy;
	radio_idx = cnss_logger_device_register(wiphy, THIS_MODULE->name);
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
		  "%s: radio_index: %d, wiphy_ptr: %p",
		  __func__, radio_idx, wiphy_ptr);

	if (radio_idx >= 0)
		logger_initialized = true;

initialized:
	return radio_idx;
}

/**
 * nl_srv_exit() - wrapper function to unregister from cnss_logger
 *
 * The cnss_logger_device_unregister() use to unregister the driver with
 * the radio_index assigned and wiphy structure from cnss_logger.
 *
 * Return: None
 */
void nl_srv_exit(void)
{
	if (logger_initialized) {
		cnss_logger_device_unregister(radio_idx, wiphy_ptr);
		radio_idx = -EINVAL;
		wiphy_ptr = NULL;
		logger_initialized = false;
	}
}

/**
 * nl_srv_ucast() - wrapper function to do unicast tx through cnss_logger
 * @skb:	the socket buffer to send
 * @dst_pid:	the port id
 * @flag:	the blocking or nonblocking flag
 *
 * The nl_srv_is_initialized() is used to do sanity check if the netlink
 * service is ready, e.g if the radio_index is assigned properly, if not
 * the driver should take the responsibility to free the skb.
 *
 * The cnss_logger_nl_ucast() use the same parameters to send the socket
 * buffers.
 *
 * Return: the error of the transmission status
 */
int nl_srv_ucast(struct sk_buff *skb, int dst_pid, int flag)
{
	int err = -EINVAL;

	/* sender's pid */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
	NETLINK_CB(skb).pid = 0;
#else
	NETLINK_CB(skb).portid = 0;
#endif
	/* not multicast */
	NETLINK_CB(skb).dst_group = 0;

	if (nl_srv_is_initialized() == 0)
		err = cnss_logger_nl_ucast(skb, dst_pid, flag);
	else
		dev_kfree_skb(skb);
	return err;
}

/**
 * nl_srv_bcast() - wrapper function to do broadcast tx through cnss_logger
 * @skb:	the socket buffer to send
 *
 * The cnss_logger_nl_bcast() is used to transmit the socket buffer.
 *
 * Return: status of transmission
 */
int nl_srv_bcast(struct sk_buff *skb)
{
	int err = -EINVAL;
	int flags = GFP_KERNEL;

	if (in_interrupt() || irqs_disabled() || in_atomic())
		flags = GFP_ATOMIC;

	/* sender's pid */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0))
	NETLINK_CB(skb).pid = 0;
#else
	NETLINK_CB(skb).portid = 0;
#endif
	 /* destination group */
	NETLINK_CB(skb).dst_group = WLAN_NLINK_MCAST_GRP_ID;

	if (nl_srv_is_initialized() == 0)
		err = cnss_logger_nl_bcast(skb, WLAN_NLINK_MCAST_GRP_ID, flags);
	else
		dev_kfree_skb(skb);
	return err;
}

/**
 * nl_srv_unregister() - wrapper function to unregister event to cnss_logger
 * @msg_type:		the message to unregister
 * @msg_handler:	the message handler
 *
 * The cnss_logger_event_unregister() is used to unregister the message and
 * message handler.
 *
 * Return: 0 if successfully unregister, otherwise proper error code
 */
int nl_srv_unregister(tWlanNlModTypes msg_type, nl_srv_msg_callback msg_handler)
{
	int ret = -EINVAL;

	if (nl_srv_is_initialized() != 0)
		return ret;

	if ((msg_type >= WLAN_NL_MSG_BASE) && (msg_type < WLAN_NL_MSG_MAX) &&
	    msg_handler != NULL) {
		ret = cnss_logger_event_unregister(radio_idx, msg_type,
						   msg_handler);
	} else {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "NLINK: nl_srv_unregister failed for msg_type %d",
			  msg_type);
		ret = -EINVAL;
	}

	return ret;
}

/**
 * nl_srv_register() - wrapper function to register event to cnss_logger
 * @msg_type:		the message to register
 * @msg_handler:	the message handler
 *
 * The cnss_logger_event_register() is used to register the message and
 * message handler.
 *
 * Return: 0 if successfully register, otherwise proper error code
 */
int nl_srv_register(tWlanNlModTypes msg_type, nl_srv_msg_callback msg_handler)
{
	int ret = -EINVAL;

	if (nl_srv_is_initialized() != 0)
		return ret;

	if ((msg_type >= WLAN_NL_MSG_BASE) && (msg_type < WLAN_NL_MSG_MAX) &&
	    msg_handler != NULL) {
		ret = cnss_logger_event_register(radio_idx, msg_type,
						 msg_handler);
	} else {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "NLINK: nl_srv_register failed for msg_type %d",
			  msg_type);
		ret = -EINVAL;
	}

	return ret;
}

/**
 * nl_srv_is_initialized() - check if netlink service is initialized
 *
 * Return: 0 if it is initialized, otherwise error code
 */
inline int nl_srv_is_initialized(void)
{
	if (logger_initialized)
		return 0;
	else
		return -EPERM;
}

#else


/* Global variables */
static DEFINE_MUTEX(nl_srv_sem);
static struct sock *nl_srv_sock;
static nl_srv_msg_callback nl_srv_msg_handler[NLINK_MAX_CALLBACKS];

/* Forward declaration */
static void nl_srv_rcv(struct sk_buff *sk);
static void nl_srv_rcv_skb(struct sk_buff *skb);
static void nl_srv_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh);

/*
 * Initialize the netlink service.
 * Netlink service is usable after this.
 */
int nl_srv_init(void *wiphy)
{
	int retcode = 0;
	struct netlink_kernel_cfg cfg = {
		.groups = WLAN_NLINK_MCAST_GRP_ID,
		.input = nl_srv_rcv
	};

	nl_srv_sock = netlink_kernel_create(&init_net, WLAN_NLINK_PROTO_FAMILY,
					    &cfg);

	if (nl_srv_sock != NULL) {
		memset(nl_srv_msg_handler, 0, sizeof(nl_srv_msg_handler));
	} else {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "NLINK: netlink_kernel_create failed");
		retcode = -ECONNREFUSED;
	}
	return retcode;
}

/*
 * Deinit the netlink service.
 * Netlink service is unusable after this.
 */
void nl_srv_exit(void)
{
	if (nl_srv_is_initialized() == 0)
		netlink_kernel_release(nl_srv_sock);

	nl_srv_sock = NULL;
}

/*
 * Register a message handler for a specified module.
 * Each module (e.g. WLAN_NL_MSG_BTC )will register a
 * handler to handle messages addressed to it.
 */
int nl_srv_register(tWlanNlModTypes msg_type, nl_srv_msg_callback msg_handler)
{
	int retcode = 0;

	if ((msg_type >= WLAN_NL_MSG_BASE) && (msg_type < WLAN_NL_MSG_MAX) &&
	    msg_handler != NULL) {
		nl_srv_msg_handler[msg_type - WLAN_NL_MSG_BASE] = msg_handler;
	} else {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  "NLINK: nl_srv_register failed for msg_type %d",
			  msg_type);
		retcode = -EINVAL;
	}

	return retcode;
}

/*
 * Unregister the message handler for a specified module.
 */
int nl_srv_unregister(tWlanNlModTypes msg_type, nl_srv_msg_callback msg_handler)
{
	int retcode = 0;

	if ((msg_type >= WLAN_NL_MSG_BASE) && (msg_type < WLAN_NL_MSG_MAX) &&
	    (nl_srv_msg_handler[msg_type - WLAN_NL_MSG_BASE] == msg_handler)) {
		nl_srv_msg_handler[msg_type - WLAN_NL_MSG_BASE] = NULL;
	} else {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  "NLINK: nl_srv_unregister failed for msg_type %d",
			  msg_type);
		retcode = -EINVAL;
	}

	return retcode;
}

/*
 * Unicast the message to the process in user space identfied
 * by the dst-pid
 */
int nl_srv_ucast(struct sk_buff *skb, int dst_pid, int flag)
{
	int err = 0;

	NETLINK_CB(skb).portid = 0;     /* sender's pid */
	NETLINK_CB(skb).dst_group = 0;  /* not multicast */

	if (nl_srv_sock)
		err = netlink_unicast(nl_srv_sock, skb, dst_pid, flag);

	if (err < 0)
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  "NLINK: netlink_unicast to pid[%d] failed, ret[%d]",
			  dst_pid, err);

	return err;
}

/*
 *  Broadcast the message. Broadcast will return an error if
 *  there are no listeners
 */
int nl_srv_bcast(struct sk_buff *skb)
{
	int err = 0;
	int flags = GFP_KERNEL;

	if (in_interrupt() || irqs_disabled() || in_atomic())
		flags = GFP_ATOMIC;

	NETLINK_CB(skb).portid = 0;     /* sender's pid */
	NETLINK_CB(skb).dst_group = WLAN_NLINK_MCAST_GRP_ID;    /* destination group */

	if (nl_srv_sock)
		err = netlink_broadcast(nl_srv_sock, skb, 0,
					WLAN_NLINK_MCAST_GRP_ID, flags);

	if (err < 0) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  "NLINK: netlink_broadcast failed err = %d", err);
	}
	return err;
}

/*
 *  Processes the Netlink socket input queue.
 *  Dequeue skb's from the socket input queue and process
 *  all the netlink messages in that skb, before moving
 *  to the next skb.
 */
static void nl_srv_rcv(struct sk_buff *sk)
{
	mutex_lock(&nl_srv_sem);
	nl_srv_rcv_skb(sk);
	mutex_unlock(&nl_srv_sem);
}

/*
 * Each skb could contain multiple Netlink messages. Process all the
 * messages in one skb and discard malformed skb's silently.
 */
static void nl_srv_rcv_skb(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;

	while (skb->len >= NLMSG_SPACE(0)) {
		u32 rlen;

		nlh = (struct nlmsghdr *)skb->data;

		if (nlh->nlmsg_len < sizeof(*nlh) || skb->len < nlh->nlmsg_len) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
				  "NLINK: Invalid "
				  "Netlink message: skb[%p], len[%d], nlhdr[%p], nlmsg_len[%d]",
				  skb, skb->len, nlh, nlh->nlmsg_len);
			return;
		}

		rlen = NLMSG_ALIGN(nlh->nlmsg_len);
		if (rlen > skb->len)
			rlen = skb->len;
		nl_srv_rcv_msg(skb, nlh);
		skb_pull(skb, rlen);
	}
}

/*
 * Process a netlink message.
 * Each netlink message will have a message of type tAniMsgHdr inside.
 */
static void nl_srv_rcv_msg(struct sk_buff *skb, struct nlmsghdr *nlh)
{
	int type;

	/* Only requests are handled by kernel now */
	if (!(nlh->nlmsg_flags & NLM_F_REQUEST)) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  "NLINK: Received Invalid NL Req type [%x]",
			  nlh->nlmsg_flags);
		return;
	}

	type = nlh->nlmsg_type;

	/* Unknown message */
	if (type < WLAN_NL_MSG_BASE || type >= WLAN_NL_MSG_MAX) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  "NLINK: Received Invalid NL Msg type [%x]", type);
		return;
	}

	/*
	 * All the messages must at least carry the tAniMsgHdr
	 * Drop any message with invalid length
	 */
	if (nlh->nlmsg_len < NLMSG_LENGTH(sizeof(tAniMsgHdr))) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  "NLINK: Received NL Msg with invalid len[%x]",
			  nlh->nlmsg_len);
		return;
	}

	/* turn type into dispatch table offset */
	type -= WLAN_NL_MSG_BASE;

	/* dispatch to handler */
	if (nl_srv_msg_handler[type] != NULL) {
		(nl_srv_msg_handler[type])(skb);
	} else {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_WARN,
			  "NLINK: No handler for Netlink Msg [0x%X]", type);
	}
}

/**
 * nl_srv_is_initialized() - This function is used check if the netlink
 * service is initialized
 *
 * This function is used check if the netlink service is initialized
 *
 * Return: Return -EPERM if the service is not initialized
 *
 */
int nl_srv_is_initialized(void)
{
	if (nl_srv_sock)
		return 0;

	return -EPERM;
}

#endif
