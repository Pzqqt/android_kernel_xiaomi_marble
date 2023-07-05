/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#include "qdf_ptr_hash.h"
#include "qdf_ptr_hash_test.h"
#include "qdf_trace.h"

#define qdf_ptr_hash_bits 4 /* 16 buckets */
#define qdf_ptr_hash_entry_count 10

struct qdf_ptr_hash_test_item {
	uint32_t id;
	struct qdf_ptr_hash_entry entry;
};

static uint32_t __qdf_ptr_hash_test_empty(struct qdf_ptr_hash *ht)
{
	struct qdf_ptr_hash_test_item *item;

	/* a new ptr_hash should ... */

	/* ... be empty */
	QDF_BUG(qdf_ptr_hash_empty(ht));

	/* ... return NULL with get()'d */
	QDF_BUG(!qdf_ptr_hash_get(ht, NULL, item, entry));

	return 0;
}

static uint32_t qdf_ptr_hash_test_empty(void)
{
	qdf_ptr_hash_declare_ptr(ht, qdf_ptr_hash_bits);
	int errors;

	qdf_ptr_hash_init(ht);
	errors = __qdf_ptr_hash_test_empty(ht);
	qdf_ptr_hash_deinit(ht);

	return errors;
}

static uint32_t __qdf_ptr_hash_test_add_remove(struct qdf_ptr_hash *ht)
{
	struct qdf_ptr_hash_test_item items[qdf_ptr_hash_entry_count];
	struct qdf_ptr_hash_test_item *item;
	int i;

	/* a ptr_hash with items should ... */
	for (i = 0; i < qdf_ptr_hash_entry_count; i++) {
		items[i].id = i;
		qdf_ptr_hash_add(ht, &items[i], &items[i], entry);
	}

	/* ... not be empty */
	QDF_BUG(!qdf_ptr_hash_empty(ht));

	/* ... be able to get() all items previously add()'d */
	for (i = 0; i < qdf_ptr_hash_entry_count; i++) {
		QDF_BUG(qdf_ptr_hash_get(ht, &items[i], item, entry));
		QDF_BUG(item->id == items[i].id);
	}

	/* ... be able to remove() all items previously add()'d */
	for (i = 0; i < qdf_ptr_hash_entry_count; i++) {
		QDF_BUG(qdf_ptr_hash_remove(ht, &items[i], item, entry));
		QDF_BUG(item->id == items[i].id);
	}

	/* ... be empty after remove()'ing all items */
	QDF_BUG(qdf_ptr_hash_empty(ht));

	return 0;
}

static uint32_t qdf_ptr_hash_test_add_remove(void)
{
	qdf_ptr_hash_declare_ptr(ht, qdf_ptr_hash_bits);
	int errors;

	qdf_ptr_hash_init(ht);
	errors = __qdf_ptr_hash_test_add_remove(ht);
	qdf_ptr_hash_deinit(ht);

	return errors;
}

static uint32_t __qdf_ptr_hash_test_for_each(struct qdf_ptr_hash *ht)
{
	struct qdf_ptr_hash_bucket *bucket;
	struct qdf_ptr_hash_test_item items[qdf_ptr_hash_entry_count];
	struct qdf_ptr_hash_test_item *item;
	int i;
	int count;

	/* a ptr_hash with items should ... */
	for (i = 0; i < qdf_ptr_hash_entry_count; i++) {
		items[i].id = i;
		qdf_ptr_hash_add(ht, i, &items[i], entry);
	}

	/* ... be able to iterate over each item */
	count = 0;
	qdf_ptr_hash_for_each(ht, bucket, item, entry) {
		QDF_BUG(item->id == items[item->id].id);
		count++;
	}
	QDF_BUG(count == qdf_ptr_hash_entry_count);

	/* ... be able to interate by hash value */
	count = 0;
	for (i = 0; i < qdf_ptr_hash_entry_count; i++) {
		qdf_ptr_hash_for_each_by_hash(ht, i, item, entry) {
			QDF_BUG(item->id == items[item->id].id);
			count++;
		}
	}
	QDF_BUG(count >= qdf_ptr_hash_entry_count);

	/* ... be able to interate by key value */
	for (i = 0; i < qdf_ptr_hash_entry_count; i++) {
		count = 0;
		qdf_ptr_hash_for_each_by_key(ht, i, item, entry) {
			QDF_BUG(item->id == items[i].id);
			count++;
		}
		QDF_BUG(count == 1);
	}

	/* ... be able to remove each item */
	for (i = 0; i < qdf_ptr_hash_entry_count; i++) {
		qdf_ptr_hash_remove(ht, i, item, entry);
		QDF_BUG(item);
		QDF_BUG(item->id == items[i].id);
	}

	/* ... be empty after all items are removed */
	QDF_BUG(qdf_ptr_hash_empty(ht));

	return 0;
}

static uint32_t qdf_ptr_hash_test_for_each(void)
{
	qdf_ptr_hash_declare_ptr(ht, qdf_ptr_hash_bits);
	int errors;

	qdf_ptr_hash_init(ht);
	errors = __qdf_ptr_hash_test_for_each(ht);
	qdf_ptr_hash_deinit(ht);

	return errors;
}

static uint32_t qdf_ptr_hash_test_create_destroy(void)
{
	struct qdf_ptr_hash *ht = qdf_ptr_hash_create(qdf_ptr_hash_bits);
	uint32_t errors = 0;

	QDF_BUG(ht);
	errors += __qdf_ptr_hash_test_empty(ht);
	errors += __qdf_ptr_hash_test_add_remove(ht);
	errors += __qdf_ptr_hash_test_for_each(ht);

	qdf_ptr_hash_destroy(ht);

	return errors;
}

uint32_t qdf_ptr_hash_unit_test(void)
{
	uint32_t errors = 0;

	errors += qdf_ptr_hash_test_empty();
	errors += qdf_ptr_hash_test_add_remove();
	errors += qdf_ptr_hash_test_for_each();
	errors += qdf_ptr_hash_test_create_destroy();

	return errors;
}

