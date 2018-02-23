/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: UCFG APIs for the configuration component.
 *
 * Logically, configuration exists at the psoc level. This means, each psoc can
 * have its own custom configuration, and calls to lookup configuration take a
 * psoc parameter for reference. E.g.
 *
 *	int32_t value = cfg_get(psoc, WLAN_SOME_INTEGER_CONFIG_ID);
 *
 * Configuration is cascading, and lookups happen in this order:
 *
 *	1) use psoc value, if configured
 *	2) use global value, if configured
 *	3) fallback to the default value for the configuration item
 *
 * This means a psoc configuration is a specialization of the global
 * configuration, and does not need to explicitly set the same values if they
 * would match the global config.
 *
 * In order to load and parse the global config, call cfg_parse(). In order to
 * load and parse psoc configs, call cfg_psoc_parse(). cfg_parse() MUST be
 * called before cfg_psoc_parse(), as global configuration will be consulted
 * during the psoc parsing process.
 *
 * There are two basic lifecycles supported:
 *
 * 1) The type and number of psocs is *not* known at load time
 *
 *	// driver is loading
 *	cfg_parse("/path/to/config");
 *
 *	...
 *
 *	// a psoc has just been created
 *	cfg_psoc_parse(psoc, "/path/to/psoc/config");
 *
 *	...
 *
 *	// driver is unloading
 *	cfg_release();
 *
 * 2) The type and number of psocs *is* known at load time
 *
 *	// driver is loading
 *	cfg_parse("/path/to/config");
 *
 *	...
 *
 *	// for each psoc
 *		cfg_psoc_parse(psoc, "/path/to/psoc/config");
 *
 *	// no further psocs will be created after this point
 *	cfg_release();
 *
 *	...
 *
 *	// driver is unloaded later
 *
 * Each configuration store is reference counted to reduce memory footprint, and
 * the configuration component itself will hold one ref count on the global
 * config store. All psocs for which psoc-specific configurations have *not*
 * been provided will reference the global config store. Psocs for which psoc-
 * specific configurations *have* been provded will check for existings stores
 * with a matching path to use, before parsing the specified configuration file.
 *
 * If, at some point in time, it is known that no further psocs will ever be
 * created, a call to cfg_release() will release the global ref count held by
 * the configuration component. For systems which specify psoc-specific configs
 * for all psocs, this will release the unnecessary memory used by the global
 * config store. Otherwise, calling cfg_release() at unload time will ensure
 * the global config store is properly freed.
 */

#ifndef __CFG_UCFG_H
#define __CFG_UCFG_H

#include "cfg_all.h"
#include "cfg_define.h"
#include "i_cfg.h"
#include "qdf_status.h"
#include "qdf_str.h"
#include "qdf_types.h"
#include "wlan_objmgr_psoc_obj.h"

/**
 * cfg_parse() - parse an ini file, and populate the global config storei
 * @path: The full file path of the ini file to parse
 *
 * Note: A matching cfg_release() call is required to release allocated
 * resources.
 *
 * The *.ini file format is a simple format consiting of a list of key/value
 * pairs, separated by an '=' character. e.g.
 *
 *	gConfigItem1=some string value
 *	gConfigItem2=0xabc
 *
 * Comments are also supported, initiated with the '#' character:
 *
 *	# This is a comment. It will be ignored by the *.ini parser
 *	gConfigItem3=aa:bb:cc:dd:ee:ff # this is also a comment
 *
 * Several datatypes are natively supported:
 *
 *	gInt=-123 # bin (0b), octal (0o), hex (0x), and decimal supported
 *	gUint=123 # a non-negative integer value
 *	gBool=y # (1, Y, y) -> true; (0, N, n) -> false
 *	gString=any string # strings are useful for representing complex types
 *	gMacAddr=aa:bb:cc:dd:ee:ff # colons are optional, upper and lower case
 *	gIpv4Addr=127.0.0.1 # uses typical dot-decimal notation
 *	gIpv6Addr=::1 # typical notation, supporting zero-compression
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cfg_parse(const char *path);

/**
 * cfg_release() - release the global configuration store
 *
 * This API releases the configuration component's reference to the global
 * config store.
 *
 * See also: this file's DOC section.
 *
 * Return: None
 */
void cfg_release(void);

/**
 * cfg_psoc_parse() - specialize the config store for @psoc by parsing @path
 * @psoc: The psoc whose config store should be specialized
 * @path: The full file path of the ini file to parse
 *
 * See also: cfg_parse(), and this file's DOC section.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cfg_psoc_parse(struct wlan_objmgr_psoc *psoc, const char *path);

/**
 * cfg_get() - lookup the configured value for @id from @psoc
 * @psoc: The psoc from which to lookup the configured value
 * @id: The id of the configured value to lookup
 *
 * E.g.
 *
 *	int32_t value = cfg_get(psoc, WLAN_SOME_INTEGER_CONFIG_ID);
 *
 * Return: The configured value
 */
#define cfg_get(psoc, id) __cfg_get(psoc, __##id)

/* Configuration Access APIs */
#define __do_call(op, args...) op(args)
#define do_call(op, args) __do_call(op, rm_parens args)

#define cfg_id(id) #id

#define __cfg_mtype(mtype, ctype, name, min, max, fallback, desc, def...) mtype
#define cfg_mtype(id) do_call(__cfg_mtype, id)

#define __cfg_type(mtype, ctype, name, min, max, fallback, desc, def...) ctype
#define cfg_type(id) do_call(__cfg_type, id)

#define __cfg_name(mtype, ctype, name, min, max, fallback, desc, def...) name
#define cfg_name(id) do_call(__cfg_name, id)

#define __cfg_min(mtype, ctype, name, min, max, fallback, desc, def...) min
#define cfg_min(id) do_call(__cfg_min, id)

#define __cfg_max(mtype, ctype, name, min, max, fallback, desc, def...) max
#define cfg_max(id) do_call(__cfg_max, id)

#define __cfg_fb(mtype, ctype, name, min, max, fallback, desc, def...) fallback
#define cfg_fallback(id) do_call(__cfg_fb, id)

#define __cfg_desc(mtype, ctype, name, min, max, fallback, desc, def...) desc
#define cfg_description(id) do_call(__cfg_desc, id)

#define __cfg_def(mtype, ctype, name, min, max, fallback, desc, def...) def
#define cfg_default(id) do_call(__cfg_def, id)

#define __cfg_str(id...) #id
#define cfg_str(id) #id __cfg_str(id)

/* validate APIs */
static inline bool
cfg_string_in_range(const char *value, qdf_size_t min_len, qdf_size_t max_len)
{
	qdf_size_t len = qdf_str_len(value);

	return len >= min_len && len <= max_len;
}

#define __cfg_int_in_range(value, min, max) (value >= min && value <= max)
#define __cfg_uint_in_range(value, min, max) (value >= min && value <= max)
#define __cfg_string_in_range(value, min_len, max_len) \
	cfg_string_in_range(value, min_len, max_len)

#define __cfg_in_range(id, value, mtype) \
	__cfg_ ## mtype ## _in_range(value, cfg_min(id), cfg_max(id))

/* this may look redundant, but forces @mtype to be expanded */
#define __cfg_in_range_type(id, value, mtype) \
	__cfg_is_range(id, value, mtype)

#define cfg_in_range(id, value) __cfg_in_range_type(id, value, cfg_mtype(id))

/* Value-or-Default APIs */
#define __cfg_value_or_default(id, value, def) \
	(cfg_in_range(id, value) ? value : def)

#define cfg_value_or_default(id, value) \
	__cfg_value_or_default(id, value, cfg_default(id))

/* Value-or-Clamped APIs */
#define __cfg_clamp(val, min, max) (val < min ? min : (val > max ? max : val))
#define cfg_clamp(id, value) __cfg_clamp(value, cfg_min(id), cfg_max(id))

#endif /* __CFG_UCFG_H */

