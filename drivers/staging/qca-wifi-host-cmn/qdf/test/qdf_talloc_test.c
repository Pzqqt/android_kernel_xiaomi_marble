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

#include "qdf_status.h"
#include "qdf_talloc.h"
#include "qdf_talloc_test.h"
#include "qdf_trace.h"

static uint32_t qdf_talloc_test_alloc_free(void)
{
	uint32_t value;
	uint32_t *root;
	uint32_t *child;

	root = &value;

	child = qdf_talloc_type(root, child);
	QDF_BUG(child);

	qdf_tfree(child);

	return 0;
}

static uint32_t qdf_talloc_test_parent_child(void)
{
	uint32_t value;
	uint32_t *root;
	uint32_t *parent;
	uint32_t *child;

	root = &value;

	parent = qdf_talloc_type(root, parent);
	QDF_BUG(parent);

	child = qdf_talloc_type(parent, child);
	QDF_BUG(child);

	qdf_tfree(child);
	qdf_tfree(parent);

	return 0;
}

uint32_t qdf_talloc_unit_test(void)
{
	uint32_t errors = 0;

	errors += qdf_talloc_test_alloc_free();
	errors += qdf_talloc_test_parent_child();

	return errors;
}

