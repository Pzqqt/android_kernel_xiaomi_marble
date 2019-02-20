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

#include "qdf_tracker.h"
#include "qdf_tracker_test.h"
#include "qdf_trace.h"
#include "qdf_types.h"

#define qdf_ut_tracker_bits 4 /* 16 buckets */
#define qdf_ut_tracker_item_count 3
#define qdf_ut_tracker_declare(name) \
	qdf_tracker_declare(name, qdf_ut_tracker_bits, "unit-test leak", \
			    "unit-test alloc", "unit-test free")

static uint32_t qdf_tracker_test_empty(void)
{
	qdf_ut_tracker_declare(tracker);
	char func[QDF_TRACKER_FUNC_SIZE];
	uint32_t line;

	/* a new tracker should ... */
	qdf_tracker_init(&tracker);

	/* ... be empty */
	qdf_tracker_check_for_leaks(&tracker);

	/* ... not contain an arbitrary pointer */
	QDF_BUG(!qdf_tracker_lookup(&tracker, &tracker, &func, &line));

	qdf_tracker_deinit(&tracker);

	return 0;
}

static uint32_t qdf_tracker_test_add_remove(void)
{
	qdf_ut_tracker_declare(tracker);
	bool items[qdf_ut_tracker_item_count];
	QDF_STATUS status;
	int i;

	qdf_tracker_init(&tracker);

	/* an empty tracker should track items */
	for (i = 0; i < qdf_ut_tracker_item_count; i++) {
		status = qdf_tracker_track(&tracker, items + i,
					   __func__, __LINE__);
		items[i] = QDF_IS_STATUS_SUCCESS(status);
	}

	/* a non-empty tracker should find previously added items */
	for (i = 0; i < qdf_ut_tracker_item_count; i++) {
		char func[QDF_TRACKER_FUNC_SIZE];
		uint32_t line;

		if (!items[i])
			continue;

		QDF_BUG(qdf_tracker_lookup(&tracker, items + i, &func, &line));
		qdf_tracker_untrack(&tracker, items + i, __func__, __LINE__);
	}

	/* a tracker should be empty after all items are untracked*/
	qdf_tracker_check_for_leaks(&tracker);

	qdf_tracker_deinit(&tracker);

	return 0;
}

uint32_t qdf_tracker_unit_test(void)
{
	uint32_t errors = 0;

	errors += qdf_tracker_test_empty();
	errors += qdf_tracker_test_add_remove();

	return errors;
}

