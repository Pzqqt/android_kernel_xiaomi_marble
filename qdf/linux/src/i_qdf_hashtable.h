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

#ifndef __I_QDF_HASHTABLE_H
#define __I_QDF_HASHTABLE_H

#include "linux/hashtable.h"

#define __qdf_ht hlist_head
#define __qdf_ht_entry hlist_node
#define __qdf_ht_declare(name, bits) DECLARE_HASHTABLE(name, bits)
#define __qdf_ht_init(table) hash_init(table)
#define __qdf_ht_deinit(table) do { } while (false)
#define __qdf_ht_empty(table) hash_empty(table)
#define __qdf_ht_add(table, entry, key) hash_add(table, entry, key)
#define __qdf_ht_remove(entry) hash_del(entry)

#define __qdf_ht_for_each(table, i, cursor, entry_field) \
	hash_for_each(table, i, cursor, entry_field)

#define __qdf_ht_for_each_in_bucket(table, cursor, entry_field, key) \
	hash_for_each_possible(table, cursor, entry_field, key)

#define __qdf_ht_for_each_match(table, cursor, entry_field, key, key_field) \
	hash_for_each_possible(table, (cursor), entry_field, (key)) \
		if ((cursor)->key_field == (key))

#define __qdf_ht_get(table, cursor, entry_field, key, key_field) \
do { \
	cursor = NULL; \
	__qdf_ht_for_each_match(table, cursor, entry_field, key, key_field) \
		break; \
} while (false)

#endif /* __I_QDF_HASHTABLE_H */
