/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
 * DOC: declares driver functions interfacing with linux kernel
 */


#ifndef _WLAN_CFG80211_H_
#define _WLAN_CFG80211_H_

#include <linux/version.h>
#include <linux/netdevice.h>
#include <net/netlink.h>
#include <net/cfg80211.h>
#include <qca_vendor.h>
#include <qdf_nbuf.h>
#include "qal_devcfg.h"

#define osif_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_OS_IF, params)
#define osif_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_OS_IF, params)
#define osif_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_OS_IF, params)
#define osif_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_OS_IF, params)
#define osif_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_OS_IF, params)
#define osif_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_OS_IF, params)
#define osif_rl_debug(params...) \
	QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_OS_IF, params)

#define osif_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_OS_IF, params)
#define osif_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_OS_IF, params)
#define osif_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_OS_IF, params)
#define osif_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_OS_IF, params)
#define osif_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_OS_IF, params)

#if defined(NBUF_MEMORY_DEBUG) && defined(NETLINK_BUF_TRACK)
#define wlan_cfg80211_vendor_free_skb(skb) \
	qdf_nbuf_free(skb)

#define wlan_cfg80211_vendor_event(skb, gfp) \
{ \
	qdf_nbuf_count_dec(skb); \
	qdf_net_buf_debug_release_skb(skb); \
	cfg80211_vendor_event(skb, gfp); \
}

#define wlan_cfg80211_vendor_cmd_reply(skb) \
{ \
	qdf_nbuf_count_dec(skb); \
	qdf_net_buf_debug_release_skb(skb); \
	cfg80211_vendor_cmd_reply(skb); \
}

static inline QDF_STATUS wlan_cfg80211_qal_devcfg_send_response(qdf_nbuf_t skb)
{
	qdf_nbuf_count_dec(skb);
	qdf_net_buf_debug_release_skb(skb);
	return qal_devcfg_send_response(skb);
}

static inline struct sk_buff *
__cfg80211_vendor_cmd_alloc_reply_skb(struct wiphy *wiphy, int len,
				      const char *func, uint32_t line)
{
	struct sk_buff *skb;

	skb = cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len);
	if (skb) {
		qdf_nbuf_count_inc(skb);
		qdf_net_buf_debug_acquire_skb(skb, func, line);
	}
	return skb;
}
#define wlan_cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len) \
	__cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len, __func__, __LINE__)

static inline struct sk_buff *
__cfg80211_vendor_event_alloc(struct wiphy *wiphy,
			      struct wireless_dev *wdev,
			      int approxlen,
			      int event_idx,
			      gfp_t gfp,
			      const char *func,
			      uint32_t line)
{
	struct sk_buff *skb;

	skb = cfg80211_vendor_event_alloc(wiphy, wdev,
					  approxlen,
					  event_idx,
					  gfp);
	if (skb) {
		qdf_nbuf_count_inc(skb);
		qdf_net_buf_debug_acquire_skb(skb, func, line);
	}
	return skb;
}
#define wlan_cfg80211_vendor_event_alloc(wiphy, wdev, len, idx, gfp) \
	__cfg80211_vendor_event_alloc(wiphy, wdev, len, \
				      idx, gfp, \
				      __func__, __LINE__)
#else /* NBUF_MEMORY_DEBUG && NETLINK_BUF_TRACK */
#define wlan_cfg80211_vendor_free_skb(skb) \
	kfree_skb(skb)

#define wlan_cfg80211_vendor_event(skb, gfp) \
	cfg80211_vendor_event(skb, gfp)

#define wlan_cfg80211_vendor_cmd_reply(skb) \
	cfg80211_vendor_cmd_reply(skb)

#define wlan_cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len) \
	cfg80211_vendor_cmd_alloc_reply_skb(wiphy, len)

#define wlan_cfg80211_vendor_event_alloc(wiphy, wdev, len, idx, gfp) \
	cfg80211_vendor_event_alloc(wiphy, wdev, len, idx, gfp)

static inline QDF_STATUS wlan_cfg80211_qal_devcfg_send_response( qdf_nbuf_t skb)
{
	return qal_devcfg_send_response(skb);
}
#endif /* NBUF_MEMORY_DEBUG && NETLINK_BUF_TRACK */

#undef nla_parse
#undef nla_parse_nested
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)
static inline int wlan_cfg80211_nla_parse(struct nlattr **tb,
					  int maxtype,
					  const struct nlattr *head,
					  int len,
					  const struct nla_policy *policy)
{
	return nla_parse(tb, maxtype, head, len, policy);
}

static inline int
wlan_cfg80211_nla_parse_nested(struct nlattr *tb[],
			       int maxtype,
			       const struct nlattr *nla,
			       const struct nla_policy *policy)
{
	return nla_parse_nested(tb, maxtype, nla, policy);
}
#else
static inline int wlan_cfg80211_nla_parse(struct nlattr **tb,
					  int maxtype,
					  const struct nlattr *head,
					  int len,
					  const struct nla_policy *policy)
{
	return nla_parse(tb, maxtype, head, len, policy, NULL);
}

static inline int
wlan_cfg80211_nla_parse_nested(struct nlattr *tb[],
			       int maxtype,
			       const struct nlattr *nla,
			       const struct nla_policy *policy)
{
	return nla_parse_nested(tb, maxtype, nla, policy, NULL);
}
#endif
#define nla_parse(...) (obsolete, use wlan_cfg80211_nla_parse)
#define nla_parse_nested(...) (obsolete, use wlan_cfg80211_nla_parse_nested)

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 7, 0))
static inline int
wlan_cfg80211_nla_put_u64(struct sk_buff *skb, int attrtype, u64 value)
{
	return nla_put_u64(skb, attrtype, value);
}
#else
static inline int
wlan_cfg80211_nla_put_u64(struct sk_buff *skb, int attrtype, u64 value)
{
	return nla_put_u64_64bit(skb, attrtype, value, NL80211_ATTR_PAD);
}
#endif
#endif
