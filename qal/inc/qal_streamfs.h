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

/**
 * DOC: qal_streamfs
 * stream file system APIs
 */

#if !defined(__QAL_STREAMFS_H)
#define _QAL_STREAMFS_H

/* Include Files */
#include <qdf_types.h>

struct qal_streamfs_chan;
struct qal_dentry_t;
struct qal_streamfs_chan_callbacks;
struct qal_streamfs_chan_buf;

/**
 * qal_streamfs_create_dir() - Create streamfs directory
 *  @name: name of directory to create
 *  @parent: dentry of parent directory, NULL for root directory
 *
 *  Returns directory dentry pointer if successful, NULL otherwise.
 */
struct qal_dentry_t *
qal_streamfs_create_dir(const char *name, struct qal_dentry_t *parent);

/**
 * qal_streamfs_create_file() - Create streamfs chan buffer file
 *  @name: base name of file to create
 *  @mode: filemode
 *  @parent: dentry of parent directory, NULL for root directory
 *  @buf: pointer to chan buffer
 *
 *  Returns file dentry pointer if successful, NULL otherwise.
 */
struct qal_dentry_t *
qal_streamfs_create_file(const char *name, uint16_t mode,
			 struct qal_dentry_t *parent,
			 struct qal_streamfs_chan_buf *buf);

/**
 * qal_streamfs_remove_dir_recursive() - Remove streamfs directory recursive
 *  @d: dentry of the directory
 */
void qal_streamfs_remove_dir_recursive(struct qal_dentry_t *d);

/**
 * qal_streamfs_remove_dir() - Remove streamfs directory
 *  @d: dentry of the directory
 */
void qal_streamfs_remove_dir(struct qal_dentry_t *d);

/**
 * qal_streamfs_remove_file() - Remove streamfs chan buffer file
 *  @d: dentry of the buffer file
 */
void qal_streamfs_remove_file(struct qal_dentry_t *d);

/**
 * qal_streamfs_open() - Create streamfs channel for data trasfer
 *  @base_filename: base name of files to create, %NULL for buffering only
 *  @parent: dentry of parent directory, %NULL for root directory
 *  @subbuf_size: size of sub-buffers
 *  @n_subbufs: number of sub-buffers
 *  @cb: streamfs channel callback functions
 *  @private_data: user-defined data
 *
 *  Returns channel pointer if successful, %NULL otherwise.
 */
struct qal_streamfs_chan *
qal_streamfs_open(const char *base_filename,
				  struct qal_dentry_t *parent,
				  size_t subbuf_size, size_t n_subbufs,
				  struct qal_streamfs_chan_callbacks *cb,
				  void *private_data);

/**
 * qal_streamfs_close() - Closes all channel buffers and frees the channel.
 *  @chan: pointer to qal_streamfs_chan.
 *
 *  Returns NONE
 */
void qal_streamfs_close(struct qal_streamfs_chan *chan);

/**
 * qal_streamfs_flush() - Flushes all channel buffers.
 *  @chan: pointer to qal_streamfs_chan.
 *
 *  Returns NONE
 */
void qal_streamfs_flush(struct qal_streamfs_chan *chan);

/**
 *  qal_streamfs_reset - reset streamfs channel
 *  This erases data from all channel buffers and restarting the channel
 *  in its initial state.
 *  The buffers are not freed, so any mappings are still in effect.
 *  @chan: pointer to qal_streamfs_chan.
 *
 *  Returns NONE
 */
void qal_streamfs_reset(struct qal_streamfs_chan *chan);

/**
 *  qal_streamfs_subbufs_consumed - update the buffer's sub-buffers-consumed
 *  count
 *  @chan: pointer to qal_streamfs_chan.
 *  @cpu: the cpu associated with the channel buffer to update
 *  @subbufs_consumed: number of sub-buffers to add to current buf's count
 *
 *  Returns NONE
 */
void qal_streamfs_subbufs_consumed(struct qal_streamfs_chan *chan,
								   unsigned int cpu,
								   size_t consumed);

/**
 *  qal_streamfs_write - write data into the channel
 *  @chan: relay channel
 *  @data: data to be written
 *  @length: number of bytes to write
 *
 *  Writes data into the current cpu's channel buffer.
 */
void qal_streamfs_write(struct qal_streamfs_chan *chan, const void *data,
			size_t length);

/**
 *  qal_streamfs_buf_full - boolean, is the channel buffer full?
 *  @buf: channel buffer
 *
 *  Returns 1 if the buffer is full, 0 otherwise.
 */
int qal_streamfs_buf_full(struct qal_streamfs_chan_buf *buf);

#endif
