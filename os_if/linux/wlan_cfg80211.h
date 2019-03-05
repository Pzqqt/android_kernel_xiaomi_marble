/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#define cfg80211_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_OS_IF, params)
#define cfg80211_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_OS_IF, params)
#define cfg80211_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_OS_IF, params)
#define cfg80211_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_OS_IF, params)
#define cfg80211_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_OS_IF, params)
#define cfg80211_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_OS_IF, params)
#define cfg80211_debug_rl(params...) \
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

#define COMMON_VENDOR_COMMANDS						\
{ 									\
	.info.vendor_id = OUI_QCA,					\
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_SET_WIFI_CONFIGURATION,\
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |				\
		 WIPHY_VENDOR_CMD_NEED_NETDEV,				\
	.doit = NULL							\
},									\
{									\
	.info.vendor_id = OUI_QCA,					\
	.info.subcmd = QCA_NL80211_VENDOR_SUBCMD_GET_WIFI_CONFIGURATION,\
	.flags = WIPHY_VENDOR_CMD_NEED_WDEV |				\
		WIPHY_VENDOR_CMD_NEED_NETDEV,				\
	.doit = NULL							\
},

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
