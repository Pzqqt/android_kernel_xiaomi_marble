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
 * DOC: qdf_hashtable.h - Public APIs for a hashtable data structure
 */

#ifndef __QDF_HASHTABLE_H
#define __QDF_HASHTABLE_H

#include "i_qdf_hashtable.h"

/**
 * struct qdf_ht - opaque hashtable data type
 */
#define qdf_ht __qdf_ht

/**
 * struct qdf_ht_entry - opaque hashtable entry for membership in a qdf_ht
 */
#define qdf_ht_entry __qdf_ht_entry

/**
 * qdf_ht_declare() - declare a new qdf_ht
 * @name: variable name of the hashtable to declare
 * @bits: number of hash bits to use; buckets=2^bits; Needs to be a compile
 *        time constant
 *
 */
#define qdf_ht_declare(name, bits) __qdf_ht_declare(name, bits)

/**
 * qdf_ht_init() - initialize a qdf_ht instance
 * @table: a non-pointer qdf_ht instance to initialize
 *
 * Return: none
 */
#define qdf_ht_init(table) __qdf_ht_init(table)

/**
 * qdf_ht_deinit() - de-initialize a qdf_ht instance
 * @table: a non-pointer qdf_ht instance to de-initialize
 *
 * Return: none
 */
#define qdf_ht_deinit(table) __qdf_ht_deinit(table)

/**
 * qdf_ht_empty() - check if a qdf_ht has any entries
 * @table: a non-pointer qdf_ht instance to check
 *
 * Return: true if the hashtable is empty
 */
#define qdf_ht_empty(table) __qdf_ht_empty(table)

/**
 * qdf_ht_add() - add an entry to a qdf_ht instance
 * @table: a non-pointer qdf_ht instance to add an entry to
 * @entry: pinter to a qdf_ht_entry instance to add to @table
 * @key: the key to use for entry insertion and lookup
 *
 * Return: none
 */
#define qdf_ht_add(table, entry, key) __qdf_ht_add(table, entry, key)

/**
 * qdf_ht_remove() - remove and entry from a qdf_ht instance
 * @entry: pointer to a qdf_ht_entry instance to remove
 *
 * Return: none
 */
#define qdf_ht_remove(entry) __qdf_ht_remove(entry)

/**
 * qdf_ht_for_each() - iterate all entries in @table
 * @table: a non-pointer qdf_ht instance to iterate
 * @i: int type cursor populated with the bucket index
 * @cursor: container struct pointer populated with each iteration
 * @entry_field: name of the entry field in the entry container struct
 */
#define qdf_ht_for_each(table, i, cursor, entry_field) \
	__qdf_ht_for_each(table, i, cursor, entry_field)

/**
 * qdf_ht_for_each_in_bucket() - iterate entries in the bucket for @key
 * @table: a non-pointer qdf_ht instance to iterate
 * @cursor: container struct pointer populated with each iteration
 * @entry_field: name of the entry field in the entry container struct
 * @key: key used to lookup the hashtable bucket
 */
#define qdf_ht_for_each_in_bucket(table, cursor, entry_field, key) \
	__qdf_ht_for_each_in_bucket(table, cursor, entry_field, key)

/**
 * qdf_ht_for_each_match() - iterates through each entry matching @key
 * @table: a non-pointer qdf_ht instance to iterate
 * @cursor: container struct pointer populated with each iteration
 * @entry_field: name of the entry field in the entry container struct
 * @key: key used to lookup the entries
 * @key_field: name of the key field in the entry container struct
 */
#define qdf_ht_for_each_match(table, cursor, entry_field, key, key_field) \
	__qdf_ht_for_each_match(table, cursor, entry_field, key, key_field)

/**
 * qdf_ht_get() - get the first entry with a key matching @key
 * @table: a non-pointer qdf_ht instance to look in
 * @cursor: container struct pointer populated with each iteration
 * @entry_field: name of the entry field in the entry container struct
 * @key: key used to lookup the entry
 * @key_field: name of the key field in the entry container struct
 */
#define qdf_ht_get(table, cursor, entry_field, key, key_field) \
	__qdf_ht_get(table, cursor, entry_field, key, key_field)

#endif /* __QDF_HASHTABLE_H */
