/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: APIs and macros for defining configuration.
 */

#ifndef __CFG_DEFINE_H
#define __CFG_DEFINE_H

enum cfg_fallback_behavior {
	CFG_VALUE_OR_CLAMP,
	CFG_VALUE_OR_DEFAULT,
};

#define rm_parens(...) __VA_ARGS__
#define __CFG(id, is_ini, mtype, args...) \
	__CFG_##is_ini##_##mtype(id, mtype, args)
#define _CFG(id, args) __CFG(id, args)
#define CFG(id) _CFG(__##id, rm_parens id)

#define __CFG_INI_INT(args...) __CFG_INI(args)
#define __CFG_INI_UINT(args...) __CFG_INI(args)
#define __CFG_INI_BOOL(args...) __CFG_INI(args)
#define __CFG_INI_STRING(args...) __CFG_INI(args)
#define __CFG_INI_MAC(args...) __CFG_INI(args)
#define __CFG_INI_IPV4(args...) __CFG_INI(args)
#define __CFG_INI_IPV6(args...) __CFG_INI(args)
#define __CFG_INI(args...) (args)

#define __CFG_NON_INI_INT(args...) __CFG_NON_INI(args)
#define __CFG_NON_INI_UINT(args...) __CFG_NON_INI(args)
#define __CFG_NON_INI_BOOL(args...) __CFG_NON_INI(args)
#define __CFG_NON_INI_STRING(args...) __CFG_NON_INI(args)
#define __CFG_NON_INI_MAC(args...) __CFG_NON_INI(args)
#define __CFG_NON_INI_IPV4(args...) __CFG_NON_INI(args)
#define __CFG_NON_INI_IPV6(args...) __CFG_NON_INI(args)
#define __CFG_NON_INI(args...)

/* configuration available in ini */
#define CFG_INI_INT(name, min, max, def, fallback, desc) \
	(INI, INT, int32_t, name, min, max, fallback, desc, def)
#define CFG_INI_UINT(name, min, max, def, fallback, desc) \
	(INI, UINT, uint32_t, name, min, max, fallback, desc, def)
#define CFG_INI_BOOL(name, def, desc) \
	(INI, BOOL, bool, name, false, true, -1, desc, def)
#define CFG_INI_STRING(name, min_len, max_len, def, desc) \
	(INI, STRING, char *, name, min_len, max_len, -1, desc, def)
#define CFG_INI_MAC(name, def, desc) \
	(INI, MAC, struct qdf_mac_addr, name, -1, -1, -1, desc, def)
#define CFG_INI_IPV4(name, def, desc) \
	(INI, IPV4, struct qdf_ipv4_addr, name, -1, -1, -1, desc, def)
#define CFG_INI_IPV6(name, def, desc) \
	(INI, IPV6, struct qdf_ipv6_addr, name, -1, -1, -1, desc, def)

/* configuration *not* available in ini */
#define CFG_INT(name, min, max, def, fallback, desc) \
	(NON_INI, INT, int32_t, name, min, max, fallback, desc, def)
#define CFG_UINT(name, min, max, def, fallback, desc) \
	(NON_INI, UINT, uint32_t, name, min, max, fallback, desc, def)
#define CFG_BOOL(name, def, desc) \
	(NON_INI, BOOL, bool, name, false, true, false, desc, def)
#define CFG_STRING(name, min_len, max_len, def, desc) \
	(NON_INI, STRING, char *, name, min_len, max_len, -1, desc, def)
#define CFG_MAC(name, def, desc) \
	(NON_INI, MAC, struct qdf_mac_addr, name, -1, -1, -1, desc, def)
#define CFG_IPV4(name, def, desc) \
	(NON_INI, IPV4, struct qdf_ipv4_addr, name, -1, -1, -1, desc, def)
#define CFG_IPV6(name, def, desc) \
	(NON_INI, IPV6, struct qdf_ipv6_addr, name, -1, -1, -1, desc, def)

/* utility macros/functions */
#ifdef CONFIG_AP_PLATFORM
#define PLATFORM_VALUE(non_ap_value, ap_value) ap_value
#else
#define PLATFORM_VALUE(non_ap_value, ap_value) non_ap_value
#endif

#endif /* __CFG_DEFINE_H */

