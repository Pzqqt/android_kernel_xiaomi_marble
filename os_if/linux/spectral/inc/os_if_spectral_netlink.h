/*
 * Copyright (c) 2011, 2017-2020 The Linux Foundation. All rights reserved.
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

#ifndef _OS_IF_SPECTRAL_NETLINK_H
#define _OS_IF_SPECTRAL_NETLINK_H

#include <net/netlink.h>
#include <wlan_objmgr_pdev_obj.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <wlan_spectral_public_structs.h>

/* NETLINK related declarations */
#if (KERNEL_VERSION(2, 6, 31) > LINUX_VERSION_CODE)
void os_if_spectral_nl_data_ready(struct sock *sk, int len);
#else
void os_if_spectral_nl_data_ready(struct sk_buff *skb);
#endif /* VERSION CHECK */

#ifndef SPECTRAL_NETLINK
#define SPECTRAL_NETLINK              (NETLINK_GENERIC + 1)
#endif
#define MAX_SPECTRAL_PAYLOAD         (2004)

/* Init's network namespace */
extern struct net init_net;

/**
 * os_if_spectral_netlink_init() - Initialize Spectral Netlink data structures
 * and register the NL handlers with Spectral target_if
 * @pdev: Pointer to pdev
 *
 * Preparing socket buffer and sending Netlink messages to application layer are
 * defined in os_if layer, they need to be registered with Spectral target_if
 *
 * Return: None
 */
#ifdef WLAN_CONV_SPECTRAL_ENABLE
void os_if_spectral_netlink_init(struct wlan_objmgr_pdev *pdev);
/**
 * os_if_spectral_prep_skb() - Prepare socket buffer
 * @pdev : Pointer to pdev
 * @smsg_type:  Spectral scan message type
 * @buf_type: Spectral report buffer type
 *
 * Prepare socket buffer to send the data to application layer
 *
 * Return: NLMSG_DATA of the created skb or NULL if no memory
 */
void *os_if_spectral_prep_skb(struct wlan_objmgr_pdev *pdev,
			      enum spectral_msg_type smsg_type,
			      enum spectral_msg_buf_type buf_type);

/**
 * os_if_spectral_netlink_deinit() - De-initialize Spectral Netlink data
 * structures and de-register the NL handlers from Spectral target_if
 * @pdev: Pointer to pdev
 *
 * Return: None
 */
void os_if_spectral_netlink_deinit(struct wlan_objmgr_pdev *pdev);
#else

static inline void os_if_spectral_netlink_init(struct wlan_objmgr_pdev *pdev)
{
}

static inline void os_if_spectral_netlink_deinit(struct wlan_objmgr_pdev *pdev)
{
}

#endif
#endif /* _OS_IF_SPECTRAL_NETLINK_H */
