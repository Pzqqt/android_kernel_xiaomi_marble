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

#include "qdf_hashtable.h"
#include "qdf_hashtable_test.h"
#include "qdf_trace.h"

/* 16 buckets */
#define QDF_HT_HASH_BITS 4

struct qdf_ht_test_item {
	struct qdf_ht_entry entry;
	uintptr_t key;
};

static uint32_t qdf_ht_test_single(void)
{
	const int bits = QDF_HT_HASH_BITS;
	struct qdf_ht_test_item item = { .key = (uintptr_t)&bits };
	struct qdf_ht_test_item *cursor;
	int i, count;

	qdf_ht_declare(ht, QDF_HT_HASH_BITS);

	qdf_ht_init(ht);
	qdf_ht_add(ht, &item.entry, item.key);

	qdf_ht_get(ht, cursor, entry, item.key, key);
	QDF_BUG(cursor);
	QDF_BUG(cursor->key == item.key);

	count = 0;
	qdf_ht_for_each(ht, i, cursor, entry) {
		QDF_BUG(cursor->key == item.key);
		count++;
	}
	QDF_BUG(count == 1);

	count = 0;
	qdf_ht_for_each_in_bucket(ht, cursor, entry, item.key) {
		QDF_BUG(cursor->key == item.key);
		count++;
	}
	QDF_BUG(count == 1);

	count = 0;
	qdf_ht_for_each_match(ht, cursor, entry, item.key, key) {
		QDF_BUG(cursor->key == item.key);
		count++;
	}
	QDF_BUG(count == 1);

	qdf_ht_remove(&item.entry);

	QDF_BUG(qdf_ht_empty(ht));

	qdf_ht_deinit(ht);

	return 0;
}

uint32_t qdf_ht_unit_test(void)
{
	uint32_t errors = 0;

	errors += qdf_ht_test_single();

	return errors;
}

