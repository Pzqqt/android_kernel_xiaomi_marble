/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_debugfs
 * This file provides QDF debug file system APIs
 */

#include <qdf_debugfs.h>
#include <i_qdf_debugfs.h>

/* entry for root debugfs directory*/
static struct dentry *qdf_debugfs_root;

QDF_STATUS qdf_debugfs_init(void)
{
	qdf_debugfs_root = debugfs_create_dir(KBUILD_MODNAME"_qdf", NULL);

	if (!qdf_debugfs_root)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_debugfs_init);

QDF_STATUS qdf_debugfs_exit(void)
{
	debugfs_remove_recursive(qdf_debugfs_root);
	qdf_debugfs_root = NULL;

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(qdf_debugfs_exit);

struct dentry *qdf_debugfs_get_root(void)
{
	return qdf_debugfs_root;
}

