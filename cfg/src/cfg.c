/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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

#include "cfg_all.h"
#include "cfg_define.h"
#include "cfg_dispatcher.h"
#include "cfg_ucfg_api.h"
#include "i_cfg.h"
#include "i_cfg_objmgr.h"
#include "qdf_atomic.h"
#include "qdf_list.h"
#include "qdf_mem.h"
#include "qdf_module.h"
#include "qdf_parse.h"
#include "qdf_status.h"
#include "qdf_str.h"
#include "qdf_trace.h"
#include "qdf_types.h"
#include "wlan_objmgr_psoc_obj.h"

/**
 * struct cfg_value_store - backing store for an ini file
 * @path: file path of the ini file
 * @node: internal list node for keeping track of all the allocated stores
 * @users: number of references on the store
 * @values: a values struct containing the parsed values from the ini file
 */
struct cfg_value_store {
	char *path;
	qdf_list_node_t node;
	qdf_atomic_t users;
	struct cfg_values values;
};

/* define/populate dynamic metadata lookup table */

/**
 * struct cfg_meta - configuration item metadata for dynamic lookup during parse
 * @name: name of the config item used in the ini file (i.e. "gScanDwellTime")
 * @item_handler: parsing callback based on the type of the config item
 * @min: minimum value for use in bounds checking (min_len for strings)
 * @max: maximum value for use in bounds checking (max_len for strings)
 * @fallback: the fallback behavior to use when configured values are invalid
 */
struct cfg_meta {
	const char *name;
	const uint32_t field_offset;
	void (*const item_handler)(struct cfg_value_store *store,
				   const struct cfg_meta *meta,
				   const char *value);
	const int32_t min;
	const int32_t max;
	const enum cfg_fallback_behavior fallback;
};

/* ini item handler functions */

#define cfg_value_ptr(store, meta) \
	((void *)&(store)->values + (meta)->field_offset)

static __attribute__((unused)) void
cfg_int_item_handler(struct cfg_value_store *store,
		     const struct cfg_meta *meta,
		     const char *str_value)
{
	QDF_STATUS status;
	int32_t *store_value = cfg_value_ptr(store, meta);
	int32_t value;

	status = qdf_int32_parse(str_value, &value);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg_err("%s=%s - Invalid format (status %d); Using default %d",
			meta->name, str_value, status, *store_value);
		return;
	}

	QDF_BUG(meta->min <= meta->max);
	if (meta->min > meta->max) {
		cfg_err("Invalid config item meta for %s", meta->name);
		return;
	}

	if (value >= meta->min && value <= meta->max) {
		*store_value = value;
		return;
	}

	switch (meta->fallback) {
	default:
		QDF_DEBUG_PANIC("Unknown fallback method %d for cfg item '%s'",
				meta->fallback, meta->name);
		/* fall through */
	case CFG_VALUE_OR_DEFAULT:
		/* store already contains default */
		break;
	case CFG_VALUE_OR_CLAMP:
		*store_value = __cfg_clamp(value, meta->min, meta->max);
		break;
	}

	cfg_err("%s=%d - Out of range [%d, %d]; Using %d",
		meta->name, value, meta->min, meta->max, *store_value);
}

static __attribute__((unused)) void
cfg_uint_item_handler(struct cfg_value_store *store,
		      const struct cfg_meta *meta,
		      const char *str_value)
{
	QDF_STATUS status;
	uint32_t *store_value = cfg_value_ptr(store, meta);
	uint32_t value;
	uint32_t min;
	uint32_t max;

	/**
	 * Since meta min and max are of type int32_t
	 * We need explicit type casting to avoid
	 * implicit wrap around for uint32_t type cfg data.
	*/
	min = (uint32_t)meta->min;
	max = (uint32_t)meta->max;

	status = qdf_uint32_parse(str_value, &value);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg_err("%s=%s - Invalid format (status %d); Using default %u",
			meta->name, str_value, status, *store_value);
		return;
	}

	QDF_BUG(min <= max);
	if (min > max) {
		cfg_err("Invalid config item meta for %s", meta->name);
		return;
	}

	if (value >= min && value <= max) {
		*store_value = value;
		return;
	}

	switch (meta->fallback) {
	default:
		QDF_DEBUG_PANIC("Unknown fallback method %d for cfg item '%s'",
				meta->fallback, meta->name);
		/* fall through */
	case CFG_VALUE_OR_DEFAULT:
		/* store already contains default */
		break;
	case CFG_VALUE_OR_CLAMP:
		*store_value = __cfg_clamp(value, min, max);
		break;
	}

	cfg_err("%s=%u - Out of range [%d, %d]; Using %u",
		meta->name, value, min, max, *store_value);
}

static __attribute__((unused)) void
cfg_bool_item_handler(struct cfg_value_store *store,
		      const struct cfg_meta *meta,
		      const char *str_value)
{
	QDF_STATUS status;
	bool *store_value = cfg_value_ptr(store, meta);

	status = qdf_bool_parse(str_value, store_value);
	if (QDF_IS_STATUS_SUCCESS(status))
		return;

	cfg_err("%s=%s - Invalid format (status %d); Using default '%s'",
		meta->name, str_value, status, *store_value ? "true" : "false");
}

static __attribute__((unused)) void
cfg_string_item_handler(struct cfg_value_store *store,
			const struct cfg_meta *meta,
			const char *str_value)
{
	char *store_value = cfg_value_ptr(store, meta);
	qdf_size_t len;

	QDF_BUG(meta->min >= 0);
	QDF_BUG(meta->min <= meta->max);
	if (meta->min < 0 || meta->min > meta->max) {
		cfg_err("Invalid config item meta for %s", meta->name);
		return;
	}

	/* ensure min length */
	len = qdf_str_nlen(str_value, meta->min);
	if (len < meta->min) {
		cfg_err("%s=%s - Too short; Using default '%s'",
			meta->name, str_value, store_value);
		return;
	}

	/* check max length */
	len += qdf_str_nlen(str_value + meta->min, meta->max - meta->min + 1);
	if (len > meta->max) {
		cfg_err("%s=%s - Too long; Using default '%s'",
			meta->name, str_value, store_value);
		return;
	}

	qdf_str_lcopy(store_value, str_value, meta->max + 1);
}

static __attribute__((unused)) void
cfg_mac_item_handler(struct cfg_value_store *store,
		     const struct cfg_meta *meta,
		     const char *str_value)
{
	QDF_STATUS status;
	struct qdf_mac_addr *store_value = cfg_value_ptr(store, meta);

	status = qdf_mac_parse(str_value, store_value);
	if (QDF_IS_STATUS_SUCCESS(status))
		return;

	cfg_err("%s=%s - Invalid format (status %d); Using default "
		QDF_MAC_ADDR_FMT, meta->name, str_value, status,
		QDF_MAC_ADDR_REF(store_value->bytes));
}

static __attribute__((unused)) void
cfg_ipv4_item_handler(struct cfg_value_store *store,
		      const struct cfg_meta *meta,
		      const char *str_value)
{
	QDF_STATUS status;
	struct qdf_ipv4_addr *store_value = cfg_value_ptr(store, meta);

	status = qdf_ipv4_parse(str_value, store_value);
	if (QDF_IS_STATUS_SUCCESS(status))
		return;

	cfg_err("%s=%s - Invalid format (status %d); Using default "
		QDF_IPV4_ADDR_STR, meta->name, str_value, status,
		QDF_IPV4_ADDR_ARRAY(store_value->bytes));
}

static __attribute__((unused)) void
cfg_ipv6_item_handler(struct cfg_value_store *store,
		      const struct cfg_meta *meta,
		      const char *str_value)
{
	QDF_STATUS status;
	struct qdf_ipv6_addr *store_value = cfg_value_ptr(store, meta);

	status = qdf_ipv6_parse(str_value, store_value);
	if (QDF_IS_STATUS_SUCCESS(status))
		return;

	cfg_err("%s=%s - Invalid format (status %d); Using default "
		QDF_IPV6_ADDR_STR, meta->name, str_value, status,
		QDF_IPV6_ADDR_ARRAY(store_value->bytes));
}

/* populate metadata lookup table */
#undef __CFG_INI
#define __CFG_INI(_id, _mtype, _ctype, _name, _min, _max, _fallback, ...) \
{ \
	.name = _name, \
	.field_offset = qdf_offsetof(struct cfg_values, _id##_internal), \
	.item_handler = cfg_ ## _mtype ## _item_handler, \
	.min = _min, \
	.max = _max, \
	.fallback = _fallback, \
},

#define cfg_INT_item_handler cfg_int_item_handler
#define cfg_UINT_item_handler cfg_uint_item_handler
#define cfg_BOOL_item_handler cfg_bool_item_handler
#define cfg_STRING_item_handler cfg_string_item_handler
#define cfg_MAC_item_handler cfg_mac_item_handler
#define cfg_IPV4_item_handler cfg_ipv4_item_handler
#define cfg_IPV6_item_handler cfg_ipv6_item_handler

static const struct cfg_meta cfg_meta_lookup_table[] = {
	CFG_ALL
};

/* default store initializer */

static void cfg_store_set_defaults(struct cfg_value_store *store)
{
#undef __CFG_INI
#define __CFG_INI(id, mtype, ctype, name, min, max, fallback, desc, def...) \
	ctype id = def;

	CFG_ALL

#undef __CFG_INI_STRING
#define __CFG_INI_STRING(id, mtype, ctype, name, min_len, max_len, ...) \
	qdf_str_lcopy((char *)&store->values.id##_internal, id, (max_len) + 1);

#undef __CFG_INI
#define __CFG_INI(id, mtype, ctype, name, min, max, fallback, desc, def...) \
	*(ctype *)&store->values.id##_internal = id;

	CFG_ALL
}

static const struct cfg_meta *cfg_lookup_meta(const char *name)
{
	int i;

	QDF_BUG(name);
	if (!name)
		return NULL;

	/* linear search for now; optimize in the future if needed */
	for (i = 0; i < QDF_ARRAY_SIZE(cfg_meta_lookup_table); i++) {
		const struct cfg_meta *meta = &cfg_meta_lookup_table[i];

		if (qdf_str_eq(name, meta->name))
			return meta;
	}

	return NULL;
}

static QDF_STATUS
cfg_ini_item_handler(void *context, const char *key, const char *value)
{
	struct cfg_value_store *store = context;
	const struct cfg_meta *meta;

	meta = cfg_lookup_meta(key);
	if (!meta) {
		/* TODO: promote to 'err' or 'warn' once legacy is ported */
		cfg_debug("Unknown config item '%s'", key);
		return QDF_STATUS_SUCCESS;
	}

	QDF_BUG(meta->item_handler);
	if (!meta->item_handler)
		return QDF_STATUS_SUCCESS;

	meta->item_handler(store, meta, value);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS cfg_ini_section_handler(void *context, const char *name)
{
	cfg_err("Unexpected section '%s'. Sections are not supported.", name);

	return QDF_STATUS_SUCCESS;
}

#define cfg_assert_success(expr) \
do { \
	QDF_STATUS __assert_status = (expr); \
	QDF_BUG(QDF_IS_STATUS_SUCCESS(__assert_status)); \
} while (0)

static bool __cfg_is_init;
static struct cfg_value_store *__cfg_global_store;
static qdf_list_t __cfg_stores_list;
static qdf_spinlock_t __cfg_stores_lock;

struct cfg_psoc_ctx {
	struct cfg_value_store *store;
};

static QDF_STATUS
cfg_store_alloc(const char *path, struct cfg_value_store **out_store)
{
	QDF_STATUS status;
	struct cfg_value_store *store;

	cfg_enter();

	store = qdf_mem_malloc(sizeof(*store));
	if (!store)
		return QDF_STATUS_E_NOMEM;

	status = qdf_str_dup(&store->path, path);
	if (QDF_IS_STATUS_ERROR(status))
		goto free_store;

	status = qdf_atomic_init(&store->users);
	if (QDF_IS_STATUS_ERROR(status))
		goto free_path;
	qdf_atomic_inc(&store->users);

	qdf_spin_lock_bh(&__cfg_stores_lock);
	status = qdf_list_insert_back(&__cfg_stores_list, &store->node);
	qdf_spin_unlock_bh(&__cfg_stores_lock);
	if (QDF_IS_STATUS_ERROR(status))
		goto free_path;

	*out_store = store;

	return QDF_STATUS_SUCCESS;

free_path:
	qdf_mem_free(store->path);

free_store:
	qdf_mem_free(store);

	return status;
}

static void cfg_store_free(struct cfg_value_store *store)
{
	QDF_STATUS status;

	cfg_enter();

	qdf_spin_lock_bh(&__cfg_stores_lock);
	status = qdf_list_remove_node(&__cfg_stores_list, &store->node);
	qdf_spin_unlock_bh(&__cfg_stores_lock);
	if (QDF_IS_STATUS_ERROR(status))
		QDF_DEBUG_PANIC("Failed config store list removal; status:%d",
				status);

	qdf_mem_free(store->path);
	qdf_mem_free(store);
}

static QDF_STATUS
cfg_store_get(const char *path, struct cfg_value_store **out_store)
{
	QDF_STATUS status;
	qdf_list_node_t *node;

	*out_store = NULL;

	qdf_spin_lock_bh(&__cfg_stores_lock);
	status = qdf_list_peek_front(&__cfg_stores_list, &node);
	while (QDF_IS_STATUS_SUCCESS(status)) {
		struct cfg_value_store *store =
			qdf_container_of(node, struct cfg_value_store, node);

		if (qdf_str_eq(path, store->path)) {
			qdf_atomic_inc(&store->users);
			*out_store = store;
			break;
		}

		status = qdf_list_peek_next(&__cfg_stores_list, node, &node);
	}
	qdf_spin_unlock_bh(&__cfg_stores_lock);

	return status;
}

static void cfg_store_put(struct cfg_value_store *store)
{
	if (qdf_atomic_dec_and_test(&store->users))
		cfg_store_free(store);
}

static struct cfg_psoc_ctx *cfg_psoc_get_ctx(struct wlan_objmgr_psoc *psoc)
{
	struct cfg_psoc_ctx *psoc_ctx;

	psoc_ctx = cfg_psoc_get_priv(psoc);
	QDF_BUG(psoc_ctx);

	return psoc_ctx;
}

struct cfg_values *cfg_psoc_get_values(struct wlan_objmgr_psoc *psoc)
{
	return &cfg_psoc_get_ctx(psoc)->store->values;
}
qdf_export_symbol(cfg_psoc_get_values);

static QDF_STATUS
cfg_ini_parse_to_store(const char *path, struct cfg_value_store *store)
{
	QDF_STATUS status;

	status = qdf_ini_parse(path, store, cfg_ini_item_handler,
			       cfg_ini_section_handler);
	if (QDF_IS_STATUS_ERROR(status))
		cfg_err("Failed to parse *.ini file @ %s; status:%d",
			path, status);

	return status;
}

QDF_STATUS cfg_parse_to_psoc_store(struct wlan_objmgr_psoc *psoc,
				   const char *path)
{
	return cfg_ini_parse_to_store(path, cfg_psoc_get_ctx(psoc)->store);
}

qdf_export_symbol(cfg_parse_to_psoc_store);

QDF_STATUS cfg_parse_to_global_store(const char *path)
{
	if (!__cfg_global_store) {
		cfg_err("Global INI store is not valid");
		return QDF_STATUS_E_NOMEM;
	}

	return cfg_ini_parse_to_store(path, __cfg_global_store);
}

qdf_export_symbol(cfg_parse_to_global_store);


static QDF_STATUS
cfg_store_print(struct wlan_objmgr_psoc *psoc)
{
	struct cfg_value_store *store;
	struct cfg_psoc_ctx *psoc_ctx;

	cfg_enter();

	psoc_ctx = cfg_psoc_get_ctx(psoc);
	if (!psoc_ctx)
		return QDF_STATUS_E_FAILURE;

	store = psoc_ctx->store;
	if (!store)
		return QDF_STATUS_E_FAILURE;

#undef __CFG_INI_MAC
#define __CFG_INI_MAC(id, mtype, ctype, name, desc, def...) \
	cfg_nofl_debug("%s "QDF_MAC_ADDR_FMT, name, \
	QDF_MAC_ADDR_REF((&store->values.id##_internal)->bytes));

#undef __CFG_INI_IPV4
#define __CFG_INI_IPV4(id, mtype, ctype, name, desc, def...) \
	cfg_nofl_debug("%s %pI4", name, (&store->values.id##_internal)->bytes);

#undef __CFG_INI_IPV6
#define __CFG_INI_IPV6(id, mtype, ctype, name, desc, def...) \
	cfg_nofl_debug("%s %pI6c", name, (&store->values.id##_internal)->bytes);

#undef __CFG_INI
#define __CFG_INI(id, mtype, ctype, name, min, max, fallback, desc, def...) \
	cfg_nofl_debug("%s %u", name, *(ctype *)&store->values.id##_internal);

#undef __CFG_INI_STRING
#define __CFG_INI_STRING(id, mtype, ctype, name, min_len, max_len, ...) \
	cfg_nofl_debug("%s %s", name, (char *)&store->values.id##_internal);

	CFG_ALL

#undef __CFG_INI_MAC
#undef __CFG_INI_IPV4
#undef __CFG_INI_IPV6
#undef __CFG_INI
#undef __CFG_INI_STRING

	cfg_exit();
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
cfg_ini_config_print(struct wlan_objmgr_psoc *psoc, uint8_t *buf,
		     ssize_t *plen, ssize_t buflen)
{
	struct cfg_value_store *store;
	struct cfg_psoc_ctx *psoc_ctx;
	ssize_t len;
	ssize_t total_len = buflen;

	cfg_enter();

	psoc_ctx = cfg_psoc_get_ctx(psoc);
	if (!psoc_ctx)
		return QDF_STATUS_E_FAILURE;

	store = psoc_ctx->store;
	if (!store)
		return QDF_STATUS_E_FAILURE;

#undef __CFG_INI_MAC
#define __CFG_INI_MAC(id, mtype, ctype, name, desc, def...) \
	do { \
		len = qdf_scnprintf(buf, buflen, "%s "QDF_MAC_ADDR_FMT"\n", \
			name, \
			QDF_MAC_ADDR_REF((&store->values.id##_internal)->bytes)); \
		buf += len; \
		buflen -= len; \
	} while (0);

#undef __CFG_INI_IPV4
#define __CFG_INI_IPV4(id, mtype, ctype, name, desc, def...) \
	do { \
		len = qdf_scnprintf(buf, buflen, "%s %pI4\n", name, \
				    (&store->values.id##_internal)->bytes); \
		buf += len; \
		buflen -= len; \
	} while (0);

#undef __CFG_INI_IPV6
#define __CFG_INI_IPV6(id, mtype, ctype, name, desc, def...) \
	do { \
		len = qdf_scnprintf(buf, buflen, "%s %pI6c\n", name, \
				    (&store->values.id##_internal)->bytes); \
		buf += len; \
		buflen -= len; \
	} while (0);

#undef __CFG_INI
#define __CFG_INI(id, mtype, ctype, name, min, max, fallback, desc, def...) \
	do { \
		len = qdf_scnprintf(buf, buflen, "%s %u\n", name, \
				    *(ctype *)&store->values.id##_internal); \
		buf += len; \
		buflen -= len; \
	} while (0);

#undef __CFG_INI_STRING
#define __CFG_INI_STRING(id, mtype, ctype, name, min_len, max_len, ...) \
	do { \
		len = qdf_scnprintf(buf, buflen, "%s %s\n", name, \
				    (char *)&store->values.id##_internal); \
		buf += len; \
		buflen -= len; \
	} while (0);

	CFG_ALL

#undef __CFG_INI_MAC
#undef __CFG_INI_IPV4
#undef __CFG_INI_IPV6
#undef __CFG_INI
#undef __CFG_INI_STRING

	*plen = total_len - buflen;
	cfg_exit();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ucfg_cfg_store_print(struct wlan_objmgr_psoc *psoc)
{
	return cfg_store_print(psoc);
}

QDF_STATUS ucfg_cfg_ini_config_print(struct wlan_objmgr_psoc *psoc,
				     uint8_t *buf, ssize_t *plen,
				     ssize_t buflen)
{
	return cfg_ini_config_print(psoc, buf, plen, buflen);
}

static QDF_STATUS
cfg_on_psoc_create(struct wlan_objmgr_psoc *psoc, void *context)
{
	QDF_STATUS status;
	struct cfg_psoc_ctx *psoc_ctx;

	cfg_enter();

	QDF_BUG(__cfg_global_store);
	if (!__cfg_global_store)
		return QDF_STATUS_E_FAILURE;

	psoc_ctx = qdf_mem_malloc(sizeof(*psoc_ctx));
	if (!psoc_ctx)
		return QDF_STATUS_E_NOMEM;

	qdf_atomic_inc(&__cfg_global_store->users);
	psoc_ctx->store = __cfg_global_store;

	status = cfg_psoc_set_priv(psoc, psoc_ctx);
	if (QDF_IS_STATUS_ERROR(status))
		goto put_store;

	return QDF_STATUS_SUCCESS;

put_store:
	cfg_store_put(__cfg_global_store);
	qdf_mem_free(psoc_ctx);

	return status;
}

static QDF_STATUS
cfg_on_psoc_destroy(struct wlan_objmgr_psoc *psoc, void *context)
{
	QDF_STATUS status;
	struct cfg_psoc_ctx *psoc_ctx;

	cfg_enter();

	psoc_ctx = cfg_psoc_get_ctx(psoc);
	status = cfg_psoc_unset_priv(psoc, psoc_ctx);

	cfg_store_put(psoc_ctx->store);
	qdf_mem_free(psoc_ctx);

	return status;
}

QDF_STATUS cfg_dispatcher_init(void)
{
	QDF_STATUS status;

	cfg_enter();

	QDF_BUG(!__cfg_is_init);
	if (__cfg_is_init)
		return QDF_STATUS_E_INVAL;

	qdf_list_create(&__cfg_stores_list, 0);
	qdf_spinlock_create(&__cfg_stores_lock);

	status = cfg_psoc_register_create(cfg_on_psoc_create);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	status = cfg_psoc_register_destroy(cfg_on_psoc_destroy);
	if (QDF_IS_STATUS_ERROR(status))
		goto unreg_create;

	__cfg_is_init = true;

	return QDF_STATUS_SUCCESS;

unreg_create:
	cfg_assert_success(cfg_psoc_unregister_create(cfg_on_psoc_create));

	return status;
}

QDF_STATUS cfg_dispatcher_deinit(void)
{
	cfg_enter();

	QDF_BUG(__cfg_is_init);
	if (!__cfg_is_init)
		return QDF_STATUS_E_INVAL;

	__cfg_is_init = false;

	cfg_assert_success(cfg_psoc_unregister_create(cfg_on_psoc_create));
	cfg_assert_success(cfg_psoc_unregister_destroy(cfg_on_psoc_destroy));

	qdf_spin_lock_bh(&__cfg_stores_lock);
	QDF_BUG(qdf_list_empty(&__cfg_stores_list));
	qdf_spin_unlock_bh(&__cfg_stores_lock);

	qdf_spinlock_destroy(&__cfg_stores_lock);
	qdf_list_destroy(&__cfg_stores_list);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS cfg_parse(const char *path)
{
	QDF_STATUS status;
	struct cfg_value_store *store;

	cfg_enter();

	QDF_BUG(!__cfg_global_store);
	if (__cfg_global_store)
		return QDF_STATUS_E_INVAL;

	status = cfg_store_alloc(path, &store);
	if (QDF_IS_STATUS_ERROR(status))
		return status;

	cfg_store_set_defaults(store);

	status = cfg_ini_parse_to_store(path, store);
	if (QDF_IS_STATUS_ERROR(status))
		goto free_store;

	__cfg_global_store = store;

	return QDF_STATUS_SUCCESS;

free_store:
	cfg_store_free(store);

	return status;
}

void cfg_release(void)
{
	cfg_enter();

	QDF_BUG(__cfg_global_store);
	if (!__cfg_global_store)
		return;

	cfg_store_put(__cfg_global_store);
	__cfg_global_store = NULL;
}

QDF_STATUS cfg_psoc_parse(struct wlan_objmgr_psoc *psoc, const char *path)
{
	QDF_STATUS status;
	struct cfg_value_store *store;
	struct cfg_psoc_ctx *psoc_ctx;

	cfg_enter();

	QDF_BUG(__cfg_global_store);
	if (!__cfg_global_store)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(__cfg_is_init);
	if (!__cfg_is_init)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(psoc);
	if (!psoc)
		return QDF_STATUS_E_INVAL;

	QDF_BUG(path);
	if (!path)
		return QDF_STATUS_E_INVAL;

	psoc_ctx = cfg_psoc_get_ctx(psoc);

	QDF_BUG(psoc_ctx->store == __cfg_global_store);
	if (psoc_ctx->store != __cfg_global_store)
		return QDF_STATUS_SUCCESS;

	/* check if @path has been parsed before */
	status = cfg_store_get(path, &store);
	if (QDF_IS_STATUS_ERROR(status)) {
		status = cfg_store_alloc(path, &store);
		if (QDF_IS_STATUS_ERROR(status))
			return status;

		/* inherit global configuration */
		qdf_mem_copy(&store->values, &__cfg_global_store->values,
			     sizeof(store->values));

		status = cfg_ini_parse_to_store(path, store);
		if (QDF_IS_STATUS_ERROR(status))
			goto put_store;
	}

	psoc_ctx->store = store;
	cfg_store_put(__cfg_global_store);

	return QDF_STATUS_SUCCESS;

put_store:
	cfg_store_put(store);

	return status;
}

qdf_export_symbol(cfg_psoc_parse);

