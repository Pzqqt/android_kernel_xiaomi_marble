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
 * DOC: qdf_streamfs.h
 * This file provides OS abstraction for stream filesystem APIs.
 */

#ifndef _QDF_STREAMFS_H
#define _QDF_STREAMFS_H

#include <i_qdf_streamfs.h>
#include <qdf_types.h>
#include <qdf_debugfs.h>

typedef __qdf_streamfs_chan_t qdf_streamfs_chan_t;
typedef __qdf_streamfs_chan_buf_t qdf_streamfs_chan_buf_t;

#ifdef WLAN_STREAMFS
/**
 * qdf_streamfs_create_dir() - wrapper to create a debugfs directory
 * @name: name of the new directory
 * @parent: parent node. If NULL, defaults to base qdf_debugfs_root
 *
 * Return: dentry structure pointer in case of success, otherwise NULL.
 *
 */
static inline qdf_dentry_t qdf_streamfs_create_dir(
			const char *name, qdf_dentry_t parent)
{
	return qdf_debugfs_create_dir(name, parent);
}

/**
 * qdf_streamfs_remove_file() - wrapper to remove streamfs file
 * @d: streamfs node
 *
 */
static inline void qdf_streamfs_remove_file(qdf_dentry_t d)
{
	qdf_debugfs_remove_file(d);
}

/**
 * qdf_debugfs_remove_dir_recursive() - wrapper to remove directory recursively
 * @d: debugfs node
 *
 * This function will recursively remove a directory in streamfs that was
 * previously created with a call to qdf_debugfs_create_file() or it's
 * variant functions.
 */
static inline void qdf_streamfs_remove_dir_recursive(qdf_dentry_t d)
{
	qdf_debugfs_remove_dir_recursive(d);
}

/**
 * qdf_streamfs_create_file() - Create streamfs chan buffer file
 * @name: base name of file to create
 * @mode: filemode
 * @parent: dentry of parent directory, NULL for root directory
 * @buf: pointer to chan buffer
 *
 * Returns file dentry pointer if successful, NULL otherwise.
 */
qdf_dentry_t qdf_streamfs_create_file(const char *name, uint16_t mode,
				      qdf_dentry_t parent,
				      qdf_streamfs_chan_buf_t buf);

/**
 * qdf_streamfs_open() - Create streamfs channel for data trasfer
 * @base_filename: base name of files to create, %NULL for buffering only
 * @parent: dentry of parent directory, %NULL for root directory
 * @subbuf_size: size of sub-buffers
 * @n_subbufs: number of sub-buffers
 * @private_data: user-defined data
 *
 * Returns channel pointer if successful, %NULL otherwise.
 */
qdf_streamfs_chan_t qdf_streamfs_open(const char *base_filename,
				      qdf_dentry_t parent,
				      size_t subbuf_size, size_t n_subbufs,
				      void *private_data);

/**
 * qdf_streamfs_close() - Closes all channel buffers and frees the channel.
 * @chan: pointer to qdf_streamfs_chan.
 *
 * Returns NONE
 */
void qdf_streamfs_close(qdf_streamfs_chan_t chan);

/**
 * qdf_streamfs_flush() - Flushes all channel buffers.
 * @chan: pointer to qdf_streamfs_chan.
 *
 * Returns NONE
 */
void qdf_streamfs_flush(qdf_streamfs_chan_t chan);

/**
 * qdf_streamfs_reset() - Reset streamfs channel
 * @chan: pointer to qdf_streamfs_chan.
 *
 * This erases data from all channel buffers and restarting the channel
 * in its initial state. The buffers are not freed, so any mappings are
 * still in effect.
 *
 * Returns NONE
 */
void qdf_streamfs_reset(qdf_streamfs_chan_t chan);

/**
 * qdf_streamfs_subbufs_consumed() - update the buffer's sub-buffers-consumed
 * count
 * @chan: pointer to qdf_streamfs_chan.
 * @cpu: the cpu associated with the channel buffer to update
 * @subbufs_consumed: number of sub-buffers to add to current buf's count
 *
 * Returns NONE
 */
void qdf_streamfs_subbufs_consumed(qdf_streamfs_chan_t chan,
				   unsigned int cpu,
				   size_t consumed);

/**
 * qdf_streamfs_write() - write data into the channel
 * @chan: relay channel
 * @data: data to be written
 * @length: number of bytes to write
 *
 * Writes data into the current cpu's channel buffer.
 */
void qdf_streamfs_write(qdf_streamfs_chan_t chan, const void *data,
			size_t length);
#else
static inline qdf_dentry_t qdf_streamfs_create_dir(
			const char *name, qdf_dentry_t parent)
{
	return NULL;
}

static inline void qdf_streamfs_remove_file(qdf_dentry_t d)
{
}

static inline void qdf_streamfs_remove_dir_recursive(qdf_dentry_t d)
{
}

static inline
qdf_dentry_t qdf_streamfs_create_file(const char *name, uint16_t mode,
				      qdf_dentry_t parent,
				      qdf_streamfs_chan_buf_t buf)
{
	return NULL;
}

static inline
qdf_streamfs_chan_t qdf_streamfs_open(const char *base_filename,
				      qdf_dentry_t parent,
				      size_t subbuf_size, size_t n_subbufs,
				      void *private_data)
{
	return NULL;
}

static inline void qdf_streamfs_close(qdf_streamfs_chan_t chan)
{
}

static inline void qdf_streamfs_flush(qdf_streamfs_chan_t chan)
{
}

static inline void qdf_streamfs_reset(qdf_streamfs_chan_t chan)
{
}

static inline void
qdf_streamfs_subbufs_consumed(qdf_streamfs_chan_t chan,
			      unsigned int cpu, size_t consumed)
{
}

static inline void
qdf_streamfs_write(qdf_streamfs_chan_t chan, const void *data,
		   size_t length)
{
}
#endif /* WLAN_STREAMFS */
#endif /* _QDF_STREAMFS_H */
