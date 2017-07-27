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
 * DOC: qdf_debugfs.h
 * Linux specific implementation for debug filesystem APIs.
 */


#ifndef _I_QDF_DEBUGFS_H
#define _I_QDF_DEBUGFS_H

#include <linux/fs.h>
#include <linux/debugfs.h>

typedef struct dentry *__qdf_dentry_t;
typedef struct seq_file *__qdf_debugfs_file_t;

#ifdef WLAN_DEBUGFS

/**
 * qdf_debugfs_get_root() - get debugfs root
 *
 * Return: dentry * or NULL in case of failure
 */
struct dentry *qdf_debugfs_get_root(void);

/**
 * qdf_debugfs_get_filemode() - get Linux specific file mode
 * @mode: This is a bitmap of file modes,
 *		QDF_FILE_USR_READ
 *		QDF_FILE_USR_WRITE
 *		QDF_FILE_OTH_READ
 *		QDF_FILE_OTH_WRITE
 *		QDF_FILE_GRP_READ
 *		QDF_FILE_GRP_WRITE
 *
 * Return: Linux specific file mode
 */
umode_t qdf_debugfs_get_filemode(uint16_t mode);

#endif /* WLAN_DEBUGFS */
#endif /* _I_QDF_DEBUGFS_H */
