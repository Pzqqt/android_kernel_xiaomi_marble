/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#define cfg80211_log(level, args...) \
	QDF_TRACE(QDF_MODULE_ID_OS_IF, level, ## args)
#define cfg80211_logfl(level, format, args...) \
	cfg80211_log(level, FL(format), ## args)

#define cfg80211_alert(format, args...) \
	cfg80211_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define cfg80211_err(format, args...) \
	cfg80211_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define cfg80211_warn(format, args...) \
	cfg80211_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define cfg80211_notice(format, args...) \
	cfg80211_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define cfg80211_info(format, args...) \
	cfg80211_logfl(QDF_TRACE_LEVEL_INFO_HIGH, format, ## args)
#define cfg80211_debug(format, args...) \
	cfg80211_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

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
