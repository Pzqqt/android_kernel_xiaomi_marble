/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: qdf_ptr_hash.h
 *
 * A minimal hashtable implementation for doing fast lookups via pointer.
 *
 * qdf_ptr_hash also has the benefit of knowing its own size, allowing a pointer
 * to the hashtable to be passed around and embedded in other structs. Since
 * every hashtable is not necessarily of the same size, this allows using hash
 * tables in a lot of new places which would be impossible with the current
 * kernel hashtable implementation.
 *
 * Because the size of the hashtable varies with the number of bits used in the
 * hash, declaring a qdf_ptr_hash is a bit different. If you want to embed a
 * qdf_ptr_hash in another type, use a combination of qdf_ptr_hash_declare() and
 * qdf_ptr_hash_ptr(). If you just want to declare and use a qdf_ptr_hash, use
 * qdf_ptr_hash_declare_ptr() instead. Either method will ensure the appropriate
 * number of bytes is accounted for using an internal union, and provides the
 * consumer with a pointer to a qdf_ptr_hash type which can be used with all of
 * the other qdf_ptr_hash APIs. Alternatively, you can skip these complexities
 * by simply dynamically allocating the qdf_ptr_hash via qdf_ptr_hash_create().
 */

#ifndef __QDF_PTR_HASH_H
#define __QDF_PTR_HASH_H

#include "i_qdf_ptr_hash.h"
#include "qdf_mem.h"
#include "qdf_slist.h"
#include "qdf_types.h"
#include "qdf_util.h"

/**
 * struct qdf_ptr_hash_bucket - a type representing a hash bucket
 * @list: the list used for hash chaining
 */
struct qdf_ptr_hash_bucket {
	struct qdf_slist list;
};

/**
 * struct qdf_ptr_hash - a hash table type for doing fast lookups via pointer
 * @bits: the number of bits to use when hashing keys
 * @count: the number of buckets, always equal to 2^@bits
 * @buckets: empty bucket array for accessing a variable length array of buckets
 */
struct qdf_ptr_hash {
	int8_t bits;
	int16_t count;
	struct qdf_ptr_hash_bucket buckets[];
};

/**
 * struct qdf_ptr_hash_entry - entry type of membership in a qdf_ptr_hash
 * @key: the value used as the key for insertion/lookup
 * @node: the list node used for hash chaining
 */
struct qdf_ptr_hash_entry {
	uintptr_t key;
	struct qdf_slist_node node;
};

#define __qdf_ptr_hash_size(bits) (sizeof(struct qdf_ptr_hash) + \
	sizeof(((struct qdf_ptr_hash *)0)->buckets[0]) * (1 << bits))

/**
 * qdf_ptr_hash_declare() - declare a new qdf_ptr_hash
 * @name: the C identifier to use for the new hash table
 * @bits: The number of bits to use for hashing
 *
 * Return: None
 */
#define qdf_ptr_hash_declare(name, _bits) \
union { \
	struct qdf_ptr_hash ht; \
	uint8_t __raw[__qdf_ptr_hash_size(_bits)]; \
} __##name = { .ht = { .bits = _bits, .count = (1 << _bits) } }

/**
 * qdf_ptr_hash_ptr() - get a pointer to a declared qdf_ptr_hash
 * @name: the C identifier of the declared qdf_ptr_hash
 *
 * Return: pointer to a qdf_ptr_hash
 */
#define qdf_ptr_hash_ptr(name) &__##name.ht

/**
 * qdf_ptr_hash_declare_ptr() - declare a pointer to a new qdf_ptr_hash
 * @name: the C identifier to use for the pointer to the new qdf_ptr_hash
 * @bits: The number of bits to use for hashing
 *
 * Return: None
 */
#define qdf_ptr_hash_declare_ptr(name, bits) \
qdf_ptr_hash_declare(name, bits); \
struct qdf_ptr_hash *name = qdf_ptr_hash_ptr(name)

#define __qdf_ptr_hash_for_each_bucket(ht, bkt) \
	for ((bkt) = (ht)->buckets; \
	     (bkt) < (ht)->buckets + (ht)->count; \
	     (bkt)++)

/**
 * qdf_ptr_hash_init() - initialize a qdf_ptr_hash
 * @ht: the hash table to initialize
 *
 * Return: None
 */
static inline void qdf_ptr_hash_init(struct qdf_ptr_hash *ht)
{
	struct qdf_ptr_hash_bucket *bucket;

	__qdf_ptr_hash_for_each_bucket(ht, bucket)
		qdf_slist_init(&bucket->list);
}

/**
 * qdf_ptr_hash_deinit() - de-initialize a qdf_ptr_hash
 * @ht: the hash table to de-initialize
 *
 * Return: None
 */
static inline void qdf_ptr_hash_deinit(struct qdf_ptr_hash *ht)
{
	struct qdf_ptr_hash_bucket *bucket;

	__qdf_ptr_hash_for_each_bucket(ht, bucket)
		qdf_slist_deinit(&bucket->list);
}

/**
 * qdf_ptr_hash_create() - allocate and initialize a qdf_ptr_hash
 * @bits: the number of bits to use for hashing
 *
 * Return: qdf_ptr_hash pointer on succes, NULL on allocation failure
 */
static inline struct qdf_ptr_hash *qdf_ptr_hash_create(uint8_t bits)
{
	struct qdf_ptr_hash *ht = qdf_mem_malloc(__qdf_ptr_hash_size(bits));

	if (!ht)
		return NULL;

	ht->bits = bits;
	ht->count = 1 << bits;
	qdf_ptr_hash_init(ht);

	return ht;
}

/**
 * qdf_ptr_hash_destroy() - de-initialize and de-allocate a qdf_ptr_hash
 * @ht: the qdf_ptr_hash to destroy
 *
 * Return: None
 */
static inline void qdf_ptr_hash_destroy(struct qdf_ptr_hash *ht)
{
	qdf_ptr_hash_deinit(ht);
	qdf_mem_free(ht);
}

/**
 * qdf_ptr_hash_empty() - check if a qdf_ptr_hash has any entries
 * @ht: the qdf_ptr_hash to check
 *
 * Return: true if @ht contains no entries
 */
static inline bool qdf_ptr_hash_empty(struct qdf_ptr_hash *ht)
{
	struct qdf_ptr_hash_bucket *bucket;

	__qdf_ptr_hash_for_each_bucket(ht, bucket)
		if (!qdf_slist_empty(&bucket->list))
			return false;

	return true;
}

#ifdef ENABLE_QDF_PTR_HASH_DEBUG
/**
 * qdf_ptr_hash_dup_check_in_bucket() - check if a hash_entry is duplicated
					in hash_bucket
 * @bucket: qdf_ptr_hash_bucket pointer
 * cmp_entry: the hash_entry to be checked
 *
 * if the cmp_entry is found in bucket list, then trigger
 * assert to report duplication.
 *
 * Return: None
 */
static inline void qdf_ptr_hash_dup_check_in_bucket(
				struct qdf_ptr_hash_bucket *bucket,
				struct qdf_ptr_hash_entry *cmp_entry)
{
	struct qdf_ptr_hash_entry *tmp_entry;

	qdf_slist_for_each(&bucket->list, tmp_entry, node)
		qdf_assert_always(tmp_entry != cmp_entry);
}
#else
#define qdf_ptr_hash_dup_check_in_bucket(_bucket, _cmp_entry) /* no op */
#endif

static inline struct qdf_ptr_hash_bucket *
__qdf_ptr_hash_get_bucket(struct qdf_ptr_hash *ht, uintptr_t key)
{
	return ht->buckets + __qdf_ptr_hash_key(key, ht->bits);
}

/**
 * qdf_ptr_hash_add() - insert an entry into a qdf_ptr_hash
 * @ht: the qdf_ptr_hash to insert into
 * @key: the pointer to use as an insertion/lookup key
 * @item: a pointer to a type that contains a qdf_ptr_hash_entry
 * @entry_field: C identifier for the qdf_ptr_hash_entry field in @item
 *
 * Return: None
 */
#define qdf_ptr_hash_add(ht, key, item, entry_field) \
	__qdf_ptr_hash_add(ht, (uintptr_t)key, &(item)->entry_field)

static inline void __qdf_ptr_hash_add(struct qdf_ptr_hash *ht, uintptr_t key,
				      struct qdf_ptr_hash_entry *entry)
{
	entry->key = key;
	/* check hash_enrty exist or not before push */
	qdf_ptr_hash_dup_check_in_bucket(__qdf_ptr_hash_get_bucket(ht, key),
					 entry);
	qdf_slist_push(&__qdf_ptr_hash_get_bucket(ht, key)->list, entry, node);
}

/**
 * qdf_ptr_hash_remove() - remove an entry from a qdf_ptr_hash
 * @ht: the qdf_ptr_hash to remove from
 * @key: the pointer to use as a lookup key
 * @cursor: a pointer to a type that contains a qdf_ptr_hash_entry
 * @entry_field: C identifier for the qdf_ptr_hash_entry field in @cursor
 *
 * Return: removed item of type @cursor on success, NULL otherwise
 */
#define qdf_ptr_hash_remove(ht, key, cursor, entry_field) ({ \
	struct qdf_ptr_hash_entry *_e = \
		__qdf_ptr_hash_remove(ht, (uintptr_t)key); \
	cursor = _e ? qdf_container_of(_e, typeof(*(cursor)), \
				       entry_field) : NULL; \
	cursor; })

static inline struct qdf_ptr_hash_entry *
__qdf_ptr_hash_remove(struct qdf_ptr_hash *ht, uintptr_t key)
{
	struct qdf_ptr_hash_bucket *bucket = __qdf_ptr_hash_get_bucket(ht, key);
	struct qdf_ptr_hash_entry *prev;
	struct qdf_ptr_hash_entry *entry;

	qdf_slist_for_each_del(&bucket->list, prev, entry, node) {
		if (entry->key == key) {
			qdf_slist_remove(&bucket->list, prev, node);
			/* check hash_enrty exist or not after remove */
			qdf_ptr_hash_dup_check_in_bucket(bucket, entry);
			entry->key = 0;
			return entry;
		}
	}

	return NULL;
}

#define __qdf_ptr_hash_for_each_in_bucket(bucket, cursor, entry_field) \
	qdf_slist_for_each(&(bucket)->list, cursor, entry_field.node)

/**
 * qdf_ptr_hash_for_each() - qdf_ptr_hash item iterator for all items
 * @ht: the qdf_ptr_hash to iterate over
 * @bucket: qdf_ptr_hash_bucket cursor pointer
 * @cursor: a pointer to a type that contains a qdf_ptr_hash_entry
 * @entry_field: C identifier for the qdf_ptr_hash_entry field in @cursor
 */
#define qdf_ptr_hash_for_each(ht, bucket, cursor, entry_field) \
	__qdf_ptr_hash_for_each_bucket(ht, bucket) \
		__qdf_ptr_hash_for_each_in_bucket(bucket, cursor, entry_field)

/**
 * qdf_ptr_hash_for_each_by_hash() - qdf_ptr_hash item iterator for items which
 *	hash to the same value as @key
 * @ht: the qdf_ptr_hash to iterate over
 * @key: the pointer to use as a lookup key
 * @cursor: a pointer to a type that contains a qdf_ptr_hash_entry
 * @entry_field: C identifier for the qdf_ptr_hash_entry field in @cursor
 */
#define qdf_ptr_hash_for_each_by_hash(ht, key, cursor, entry_field) \
	__qdf_ptr_hash_for_each_in_bucket( \
		__qdf_ptr_hash_get_bucket(ht, (uintptr_t)key), \
		cursor, entry_field)

/**
 * qdf_ptr_hash_for_each_by_key() - qdf_ptr_hash item iterator for items whose
 *	keys equal @key
 * @ht: the qdf_ptr_hash to iterate over
 * @key: the pointer to use as a lookup key
 * @cursor: a pointer to a type that contains a qdf_ptr_hash_entry
 * @entry_field: C identifier for the qdf_ptr_hash_entry field in @cursor
 */
#define qdf_ptr_hash_for_each_by_key(ht, _key, cursor, entry_field) \
	qdf_ptr_hash_for_each_by_hash(ht, _key, cursor, entry_field) \
		if ((cursor)->entry_field.key == (uintptr_t)_key)

/**
 * qdf_ptr_hash_get() - get the first item whose key matches @key
 * @ht: the qdf_ptr_hash to look in
 * @key: the pointer to use as a lookup key
 * @cursor: a pointer to a type that contains a qdf_ptr_hash_entry
 * @entry_field: C identifier for the qdf_ptr_hash_entry field in @cursor
 *
 * Return: first item matching @key of type @cursor on success, NULL otherwise
 */
#define qdf_ptr_hash_get(ht, key, cursor, entry_field) ({ \
	cursor = NULL; \
	qdf_ptr_hash_for_each_by_key(ht, key, cursor, entry_field) \
		break; \
	cursor; })

#endif /* __QDF_PTR_HASH_H */

