/*
 * Copyright (c) 2015-2020 The Linux Foundation. All rights reserved.
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

#include "wmi_filtered_logging.h"

static struct wmi_log_buf_t *wmi_log_buf_allocate(void)
{
	struct wmi_log_buf_t *cmd_log_buf;
	int buf_size = WMI_FILTERED_CMD_EVT_MAX_NUM_ENTRY *
			sizeof(struct wmi_command_debug);

	cmd_log_buf = qdf_mem_malloc(sizeof(struct wmi_log_buf_t));
	if (!cmd_log_buf)
		return NULL;

	cmd_log_buf->buf = qdf_mem_malloc(buf_size);
	if (!cmd_log_buf->buf) {
		qdf_mem_free(cmd_log_buf);
		return NULL;
	}
	cmd_log_buf->length = 0;
	cmd_log_buf->buf_tail_idx = 0;
	cmd_log_buf->size = WMI_FILTERED_CMD_EVT_MAX_NUM_ENTRY;
	cmd_log_buf->p_buf_tail_idx = &cmd_log_buf->buf_tail_idx;

	return cmd_log_buf;
}

void wmi_filtered_logging_init(wmi_unified_t wmi_handle)
{
	int buf_size = WMI_FILTERED_CMD_EVT_SUPPORTED * sizeof(int);

	/* alloc buffer to save user inputs, for WMI_CMD */
	wmi_handle->log_info.filtered_wmi_cmds =
					qdf_mem_malloc(buf_size);
	if (!wmi_handle->log_info.filtered_wmi_cmds)
		return;

	wmi_handle->log_info.filtered_wmi_cmds_idx = 0;

	/* alloc buffer to save user interested WMI commands */
	wmi_handle->log_info.wmi_filtered_command_log = wmi_log_buf_allocate();
	if (!wmi_handle->log_info.wmi_filtered_command_log)
		goto fail1;

	/* alloc buffer to save user inputs, for WMI_EVT */
	wmi_handle->log_info.filtered_wmi_evts =
					qdf_mem_malloc(buf_size);
	if (!wmi_handle->log_info.filtered_wmi_evts)
		goto fail2;

	wmi_handle->log_info.filtered_wmi_evts_idx = 0;

	/* alloc buffer to save user interested WMI events */
	wmi_handle->log_info.wmi_filtered_event_log = wmi_log_buf_allocate();
	if (!wmi_handle->log_info.wmi_filtered_event_log)
		goto fail3;

	return;

fail3:
	qdf_mem_free(wmi_handle->log_info.filtered_wmi_evts);
	wmi_handle->log_info.filtered_wmi_evts = NULL;
fail2:
	qdf_mem_free(wmi_handle->log_info.wmi_filtered_command_log);
	wmi_handle->log_info.wmi_filtered_command_log = NULL;
fail1:
	qdf_mem_free(wmi_handle->log_info.filtered_wmi_cmds);
	wmi_handle->log_info.filtered_wmi_cmds = NULL;
}

void wmi_filtered_logging_free(wmi_unified_t wmi_handle)
{
	if (!wmi_handle)
		return;

	qdf_mem_free(wmi_handle->log_info.filtered_wmi_cmds);
	wmi_handle->log_info.filtered_wmi_cmds = NULL;
	qdf_mem_free(wmi_handle->log_info.filtered_wmi_evts);
	wmi_handle->log_info.filtered_wmi_evts = NULL;

	if (wmi_handle->log_info.wmi_filtered_command_log) {
		qdf_mem_free(wmi_handle->log_info.
			     wmi_filtered_command_log->buf);
		wmi_handle->log_info.wmi_filtered_command_log->buf = NULL;
		qdf_mem_free(wmi_handle->log_info.wmi_filtered_command_log);
		wmi_handle->log_info.wmi_filtered_command_log = NULL;
	}
	if (wmi_handle->log_info.wmi_filtered_event_log) {
		qdf_mem_free(wmi_handle->log_info.
			     wmi_filtered_event_log->buf);
		wmi_handle->log_info.wmi_filtered_event_log->buf = NULL;
		qdf_mem_free(wmi_handle->log_info.wmi_filtered_event_log);
		wmi_handle->log_info.wmi_filtered_event_log = NULL;
	}
}

/*
 * Reset the buffer which saves user interested cmds/evts
 */
static int wmi_reset_filtered_buffers(wmi_unified_t wmi_handle,
				      struct wmi_log_buf_t *cmd_log_buf)
{
	int buf_size = WMI_FILTERED_CMD_EVT_MAX_NUM_ENTRY *
			sizeof(struct wmi_command_debug);

	if (!cmd_log_buf)
		return 0;

	cmd_log_buf->length = 0;
	cmd_log_buf->buf_tail_idx = 0;
	cmd_log_buf->size = WMI_FILTERED_CMD_EVT_MAX_NUM_ENTRY;
	cmd_log_buf->p_buf_tail_idx = &cmd_log_buf->buf_tail_idx;
	qdf_mem_zero(cmd_log_buf->buf, buf_size);
	return 0;
}

/*
 * Check if id is in id list,
 * return true if found.
 */
static bool wmi_id_in_list(uint32_t *id_list, uint32_t id)
{
	int i;

	if (!id_list)
		return false;

	for (i = 0; i < WMI_FILTERED_CMD_EVT_SUPPORTED; i++) {
		if (id == id_list[i]) {
			/* id already in target list */
			return true;
		}
	}
	return false;
}

/*
 * Add command or event ids to list to be recorded
 */
static int wmi_add_to_record_list(wmi_unified_t wmi_handle,
				  uint32_t id,
				  enum WMI_RECORD_TYPE record_type)
{
	uint32_t *target_list;

	if (record_type == WMI_CMD) {
		target_list = wmi_handle->log_info.filtered_wmi_cmds;
		/* check if id already in target list */
		if (wmi_id_in_list(target_list, id))
			return 0;
		if (wmi_handle->log_info.filtered_wmi_cmds_idx >=
		    WMI_FILTERED_CMD_EVT_SUPPORTED) {
			wmi_handle->log_info.filtered_wmi_cmds_idx = 0;
		}
		target_list[wmi_handle->log_info.filtered_wmi_cmds_idx] = id;
		wmi_handle->log_info.filtered_wmi_cmds_idx++;
	} else if (record_type == WMI_EVT) {
		target_list = wmi_handle->log_info.filtered_wmi_evts;
		/* check if id already in target list */
		if (wmi_id_in_list(target_list, id))
			return 0;
		if (wmi_handle->log_info.filtered_wmi_evts_idx >=
		    WMI_FILTERED_CMD_EVT_SUPPORTED) {
			wmi_handle->log_info.filtered_wmi_evts_idx = 0;
		}
		target_list[wmi_handle->log_info.filtered_wmi_evts_idx] = id;
		wmi_handle->log_info.filtered_wmi_evts_idx++;
	} else {
		return -EINVAL;
	}
	return 0;
}

static void wmi_specific_cmd_evt_record(uint32_t id, uint8_t *buf,
					struct wmi_log_buf_t *log_buffer)
{
	int idx;
	struct wmi_command_debug *tmpbuf =
		(struct wmi_command_debug *)log_buffer->buf;

	if (*log_buffer->p_buf_tail_idx >= WMI_FILTERED_CMD_EVT_MAX_NUM_ENTRY)
		*log_buffer->p_buf_tail_idx = 0;

	idx = *log_buffer->p_buf_tail_idx;
	tmpbuf[idx].command = id;
	qdf_mem_copy(tmpbuf[idx].data, buf,
		     WMI_DEBUG_ENTRY_MAX_LENGTH);
	tmpbuf[idx].time = qdf_get_log_timestamp();
	(*log_buffer->p_buf_tail_idx)++;
	log_buffer->length++;
}

void wmi_specific_cmd_record(wmi_unified_t wmi_handle,
			     uint32_t id, uint8_t *buf)
{
	uint32_t *target_list;
	struct wmi_log_buf_t *log_buffer;

	target_list = wmi_handle->log_info.filtered_wmi_cmds;
	if (!target_list)
		return;

	log_buffer = wmi_handle->log_info.wmi_filtered_command_log;
	if (!log_buffer)
		return;

	if (wmi_id_in_list(target_list, id)) {
		/* id in target list, need to be recorded */
		wmi_specific_cmd_evt_record(id, buf, log_buffer);
	}
}

void wmi_specific_evt_record(wmi_unified_t wmi_handle,
			     uint32_t id, uint8_t *buf)
{
	uint32_t *target_list;
	struct wmi_log_buf_t *log_buffer;

	target_list = wmi_handle->log_info.filtered_wmi_evts;
	if (!target_list)
		return;

	log_buffer = wmi_handle->log_info.wmi_filtered_event_log;
	if (!log_buffer)
		return;

	if (wmi_id_in_list(target_list, id)) {
		/* id in target list, need to be recorded */
		wmi_specific_cmd_evt_record(id, buf, log_buffer);
	}
}

/*
 * Debugfs read/write functions
 */
static int wmi_filtered_seq_printf(qdf_debugfs_file_t m, const char *f, ...)
{
	va_list args;

	va_start(args, f);
	seq_vprintf(m, f, args);
	va_end(args);

	return 0;
}

/*
 * debugfs show/read for filtered_wmi_cmds
 */
int debug_filtered_wmi_cmds_show(qdf_debugfs_file_t m, void *v)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)m->private;
	int i;
	int *target_list;

	target_list = wmi_handle->log_info.filtered_wmi_cmds;
	if (!target_list)
		return 0;

	for (i = 0; i < WMI_FILTERED_CMD_EVT_SUPPORTED; i++) {
		if (target_list[i] != 0) {
			wmi_filtered_seq_printf(m, "0x%x ",
						target_list[i]);
		}
	}
	wmi_filtered_seq_printf(m, "\n");

	return 0;
}

int debug_filtered_wmi_evts_show(qdf_debugfs_file_t m, void *v)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)m->private;
	int i;
	int *target_list;

	target_list = wmi_handle->log_info.filtered_wmi_evts;
	if (!target_list)
		return 0;
	for (i = 0; i < WMI_FILTERED_CMD_EVT_SUPPORTED; i++) {
		if (target_list[i] != 0) {
			wmi_filtered_seq_printf(m, "0x%x ",
						target_list[i]);
		}
	}
	wmi_filtered_seq_printf(m, "\n");

	return 0;
}

static int wmi_log_show(wmi_unified_t wmi_handle, void *buf,
			qdf_debugfs_file_t m)
{
	struct wmi_log_buf_t *wmi_log = (struct wmi_log_buf_t *)buf;
	int pos, nread, outlen;
	int i;
	uint64_t secs, usecs;
	int wmi_ring_size = 100;

	qdf_spin_lock_bh(&wmi_handle->log_info.wmi_record_lock);
	if (!wmi_log->length) {
		qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);
		return wmi_filtered_seq_printf(m,
					       "Nothing to read!\n");
	}
	if (wmi_log->length <= wmi_ring_size)
		nread = wmi_log->length;
	else
		nread = wmi_ring_size;

	if (*wmi_log->p_buf_tail_idx == 0)
		/* tail can be 0 after wrap-around */
		pos = wmi_ring_size - 1;
	else
		pos = *wmi_log->p_buf_tail_idx - 1;

	outlen = wmi_filtered_seq_printf(m, "Length = %d\n", wmi_log->length);
	qdf_spin_unlock_bh(&wmi_handle->log_info.wmi_record_lock);
	while (nread--) {
		struct wmi_event_debug *wmi_record;

		wmi_record = &(((struct wmi_event_debug *)wmi_log->buf)[pos]);
		qdf_log_timestamp_to_secs(wmi_record->time, &secs,
					  &usecs);
		outlen += wmi_filtered_seq_printf(m, "Event ID = %x\n",
						  (wmi_record->event));
		outlen +=
			wmi_filtered_seq_printf(m,
						"Event TIME = [%llu.%06llu]\n",
						secs, usecs);
		outlen += wmi_filtered_seq_printf(m, "CMD = ");
		for (i = 0; i < (WMI_DEBUG_ENTRY_MAX_LENGTH /
				sizeof(uint32_t)); i++)
			outlen += wmi_filtered_seq_printf(m, "%x ",
							  wmi_record->data[i]);
		outlen += wmi_filtered_seq_printf(m, "\n");
		if (pos == 0)
			pos = wmi_ring_size - 1;
		else
			pos--;
	}
	return outlen;
}

int debug_wmi_filtered_command_log_show(qdf_debugfs_file_t m, void *v)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)m->private;
	struct wmi_log_buf_t *wmi_log =
		wmi_handle->log_info.wmi_filtered_command_log;

	if (!wmi_log)
		return 0;
	return wmi_log_show(wmi_handle, wmi_log, m);
}

int debug_wmi_filtered_event_log_show(qdf_debugfs_file_t m, void *v)
{
	wmi_unified_t wmi_handle = (wmi_unified_t)m->private;
	struct wmi_log_buf_t *wmi_log =
		wmi_handle->log_info.wmi_filtered_event_log;

	if (!wmi_log)
		return 0;
	return wmi_log_show(wmi_handle, wmi_log, m);
}

ssize_t debug_filtered_wmi_cmds_write(struct file *file,
				      const char __user *buf,
				      size_t count, loff_t *ppos)
{
	wmi_unified_t wmi_handle =
		((struct seq_file *)file->private_data)->private;
	int k, ret;
	char locbuf[12] = {0};
	int buf_size = WMI_FILTERED_CMD_EVT_SUPPORTED * sizeof(int);

	if ((!buf) || (count > 8 || count <= 0))
		return -EFAULT;

	if (!wmi_handle->log_info.filtered_wmi_cmds)
		return -EFAULT;

	if (copy_from_user(locbuf, buf, count))
		return -EFAULT;

	ret = qdf_kstrtoint(locbuf, 16, &k);
	if (ret)
		return -EINVAL;

	if (k == 0xffff) {
		qdf_mem_zero(wmi_handle->log_info.filtered_wmi_cmds, buf_size);
		wmi_handle->log_info.filtered_wmi_cmds_idx = 0;
		return count;
	}

	if (wmi_add_to_record_list(wmi_handle, k, WMI_CMD)) {
		wmi_err("Add cmd %d to WMI_CMD list failed", k);
		return 0;
	}

	return count;
}

ssize_t debug_filtered_wmi_evts_write(struct file *file,
				      const char __user *buf,
				      size_t count, loff_t *ppos)
{
	wmi_unified_t wmi_handle =
		((struct seq_file *)file->private_data)->private;
	int k, ret;
	char locbuf[12] = {0};
	int buf_size = WMI_FILTERED_CMD_EVT_SUPPORTED * sizeof(int);

	if ((!buf) || (count > 8 || count <= 0))
		return -EFAULT;

	if (!wmi_handle->log_info.filtered_wmi_evts)
		return -EFAULT;

	if (copy_from_user(locbuf, buf, count))
		return -EFAULT;

	ret = qdf_kstrtoint(locbuf, 16, &k);
	if (ret)
		return -EINVAL;

	if (k == 0xffff) {
		qdf_mem_zero(wmi_handle->log_info.filtered_wmi_evts, buf_size);
		wmi_handle->log_info.filtered_wmi_evts_idx = 0;
		return count;
	}

	if (wmi_add_to_record_list(wmi_handle, k, WMI_EVT)) {
		wmi_err("Add cmd %d to WMI_EVT list failed", k);
		return 0;
	}

	return count;
}

ssize_t debug_wmi_filtered_command_log_write(struct file *file,
					     const char __user *buf,
					     size_t count, loff_t *ppos)
{
	wmi_unified_t wmi_handle =
		((struct seq_file *)file->private_data)->private;
	int k, ret;
	char locbuf[12] = {0};
	struct wmi_log_buf_t *cmd_log_buf;

	if ((!buf) || (count > 8 || count <= 0))
		return -EFAULT;

	if (copy_from_user(locbuf, buf, count))
		return -EFAULT;

	ret = qdf_kstrtoint(locbuf, 16, &k);
	if (ret)
		return -EINVAL;

	if (k != 0xffff)
		return -EINVAL;

	cmd_log_buf = wmi_handle->log_info.wmi_filtered_command_log;
	if (wmi_reset_filtered_buffers(wmi_handle, cmd_log_buf))
		wmi_err("reset WMI CMD filtered_buffers failed");
	return count;
}

ssize_t debug_wmi_filtered_event_log_write(struct file *file,
					   const char __user *buf,
					   size_t count, loff_t *ppos)
{
	wmi_unified_t wmi_handle =
		((struct seq_file *)file->private_data)->private;
	int k, ret;
	char locbuf[12] = {0};
	struct wmi_log_buf_t *cmd_log_buf;

	if ((!buf) || (count > 8 || count <= 0))
		return -EFAULT;

	if (copy_from_user(locbuf, buf, count))
		return -EFAULT;

	ret = qdf_kstrtoint(locbuf, 16, &k);
	if (ret)
		return -EINVAL;

	if (k != 0xffff)
		return -EINVAL;

	cmd_log_buf = wmi_handle->log_info.wmi_filtered_event_log;
	if (wmi_reset_filtered_buffers(wmi_handle, cmd_log_buf))
		wmi_err("reset WMI EVT filtered_buffers failed");
	return count;
}
