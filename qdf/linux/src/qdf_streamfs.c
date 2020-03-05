/*
 * Copyright (c) 2018, 2020 The Linux Foundation. All rights reserved.
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
 * DOC: qdf_streamfs
 * This file provides QDF stream file system APIs
 */

#include <i_qdf_streamfs.h>
#include <qdf_trace.h>
#include <qdf_streamfs.h>
#include <qdf_module.h>

/**
 * qdf_create_buf_file_handler() - Create streamfs buffer file
 * @filename: base name of files to create, NULL for buffering only
 * @parent: dentry of parent directory, NULL for root directory
 * @mode: filemode
 * @buf: streamfs channel buf
 * @is_global: pointer to set whether this buf file is global or not.
 *
 *  Returns dentry if successful, NULL otherwise.
 */
static qdf_dentry_t
qdf_create_buf_file_handler(const char *filename, qdf_dentry_t parent,
			    uint16_t mode, qdf_streamfs_chan_buf_t buf,
			    int32_t *is_global)
{
	qdf_dentry_t buf_file;
	*is_global = 1;
	buf_file = qdf_streamfs_create_file(filename, mode, parent, buf);

	if (!buf_file)
		return NULL;

	return buf_file;
}

/**
 * qdf_remove_buf_file_handler() - Remove streamfs buffer file
 *  @dentry:dentry
 */
static int qdf_remove_buf_file_handler(qdf_dentry_t dentry)
{
	qdf_streamfs_remove_file(dentry);

	return 0;
}

static struct rchan_callbacks g_qdf_streamfs_cb = {
	.create_buf_file = qdf_create_buf_file_handler,
	.remove_buf_file = qdf_remove_buf_file_handler,
};

qdf_dentry_t
qdf_streamfs_create_file(const char *name, uint16_t mode,
			 qdf_dentry_t parent,
			 qdf_streamfs_chan_buf_t buf)
{
	qdf_dentry_t file = NULL;

	if (!name)
		return NULL;

	file = debugfs_create_file(name, mode,
				   (struct dentry *)parent,
				   buf, &relay_file_operations);

	return file;
}

qdf_export_symbol(qdf_streamfs_create_file);

qdf_streamfs_chan_t
qdf_streamfs_open(const char *base_filename, qdf_dentry_t parent,
		  size_t subbuf_size, size_t n_subbufs,
		  void *private_data)
{
	qdf_streamfs_chan_t channel_ptr = NULL;

	channel_ptr = relay_open(base_filename,
				 (struct dentry *)parent,
				 subbuf_size, n_subbufs,
				 &g_qdf_streamfs_cb,
				 private_data);

	return channel_ptr;
}

qdf_export_symbol(qdf_streamfs_open);

void qdf_streamfs_close(qdf_streamfs_chan_t chan)
{
	if (chan)
		relay_close(chan);
}

qdf_export_symbol(qdf_streamfs_close);

void qdf_streamfs_flush(qdf_streamfs_chan_t chan)
{
	if (chan)
		relay_flush(chan);
}

qdf_export_symbol(qdf_streamfs_flush);

void qdf_streamfs_reset(qdf_streamfs_chan_t chan)
{
	if (chan)
		relay_reset(chan);
}

qdf_export_symbol(qdf_streamfs_reset);

void qdf_streamfs_subbufs_consumed(qdf_streamfs_chan_t chan,
				   unsigned int cpu,
				   size_t consumed)
{
	if (chan)
		relay_subbufs_consumed(chan, cpu, consumed);
}

qdf_export_symbol(qdf_streamfs_subbufs_consumed);

void qdf_streamfs_write(qdf_streamfs_chan_t chan,
			const void *data,
			size_t length)
{
	if (chan)
		relay_write(chan, data, length);
}

qdf_export_symbol(qdf_streamfs_write);
