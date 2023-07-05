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

#ifndef WMI_FILTERED_LOGGING_H
#define WMI_FILTERED_LOGGING_H

#include <qdf_debugfs.h>
#include "wmi_unified_priv.h"

#ifdef WMI_INTERFACE_FILTERED_EVENT_LOGGING
/**
 * wmi_specific_cmd_record() - Record user specified command
 * @wmi_handle: handle to WMI
 * @id: cmd id
 * @buf: buf containing cmd details
 *
 * Check if the command id is in target list,
 * if found, record it.
 *
 * Context: the function will not sleep, caller is expected to hold
 * proper locking.
 *
 * Return: none
 */
void wmi_specific_cmd_record(wmi_unified_t wmi_handle,
			     uint32_t id, uint8_t *buf);

/**
 * wmi_specific_evt_record() - Record user specified event
 * @wmi_handle: handle to WMI
 * @id: cmd id
 * @buf: buf containing event details
 *
 * Check if the event id is in target list,
 * if found, record it.
 *
 * Context: the function will not sleep, caller is expected to hold
 * proper locking.
 *
 * Return: none
 */
void wmi_specific_evt_record(wmi_unified_t wmi_handle,
			     uint32_t id, uint8_t *buf);

/**
 * wmi_filtered_logging_init() - initialize filtered logging
 * @wmi_handle: handle to WMI
 *
 * Context: the function will not sleep, no lock needed
 *
 * Return: none
 */
void wmi_filtered_logging_init(wmi_unified_t wmi_handle);

/**
 * wmi_filtered_logging_free() - free the buffers for filtered logging
 * @wmi_handle: handle to WMI
 *
 * Context: the function will not sleep, no lock needed
 *
 * Return: none
 */
void wmi_filtered_logging_free(wmi_unified_t wmi_handle);

/*
 * Debugfs read/write functions
 */
/**
 * debug_filtered_wmi_cmds_show() - debugfs read function for filtered_wmi_cmds
 * @m: seq_file handle
 * @v: not used, offset of read
 * Return: number of bytes read
 */
int debug_filtered_wmi_cmds_show(qdf_debugfs_file_t m, void *v);

/**
 * debug_filtered_wmi_evts_show() - debugfs read function for filtered_wmi_evts
 * @m: seq_file handle
 * @v: not used, offset of read
 * Return: number of bytes read
 */
int debug_filtered_wmi_evts_show(qdf_debugfs_file_t m, void *v);

/**
 * debug_wmi_filtered_command_log_show() - debugfs read function for
 * wmi_filtered_command_log
 * @m: seq_file handle
 * @v: not used, offset of read
 * Return: number of bytes read
 */
int debug_wmi_filtered_command_log_show(qdf_debugfs_file_t m, void *v);

/**
 * debug_wmi_filtered_event_log_show() - debugfs read function for
 * wmi_filtered_event_log
 * @m: seq_file handle
 * @v: not used, offset of read
 * Return: number of bytes read
 */
int debug_wmi_filtered_event_log_show(qdf_debugfs_file_t m, void *v);

/**
 * debug_wmi_filtered_wmi_cmds_write() - debugfs write for filtered_wmi_cmds
 *
 * @file: file handler to access wmi_handle
 * @buf: received data buffer
 * @count: length of received buffer
 * @ppos: Not used
 *
 * Return: count
 */
ssize_t debug_filtered_wmi_cmds_write(struct file *file,
				      const char __user *buf,
				      size_t count, loff_t *ppos);

/**
 * debug_wmi_filtered_wmi_evts_write() - debugfs write for filtered_wmi_evts
 *
 * @file: file handler to access wmi_handle
 * @buf: received data buffer
 * @count: length of received buffer
 * @ppos: Not used
 *
 * Return: count
 */
ssize_t debug_filtered_wmi_evts_write(struct file *file,
				      const char __user *buf,
				      size_t count, loff_t *ppos);

/**
 * debug_wmi_filtered_command_log_write() - debugfs write for
 * filtered_command_log
 *
 * @file: file handler to access wmi_handle
 * @buf: received data buffer
 * @count: length of received buffer
 * @ppos: Not used
 *
 * Return: count
 */
ssize_t debug_wmi_filtered_command_log_write(struct file *file,
					     const char __user *buf,
					     size_t count, loff_t *ppos);

/**
 * debug_wmi_filtered_event_log_write() - debugfs write for filtered_event_log
 *
 * @file: file handler to access wmi_handle
 * @buf: received data buffer
 * @count: length of received buffer
 * @ppos: Not used
 *
 * Return: count
 */
ssize_t debug_wmi_filtered_event_log_write(struct file *file,
					   const char __user *buf,
					   size_t count, loff_t *ppos);

#else /* WMI_INTERFACE_FILTERED_EVENT_LOGGING */

static inline void wmi_specific_cmd_record(wmi_unified_t wmi_handle,
					   uint32_t id, uint8_t *buf)
{
	/* do nothing */
}

static inline void wmi_specific_evt_record(wmi_unified_t wmi_handle,
					   uint32_t id, uint8_t *buf)
{
	/* do nothing */
}

static inline void wmi_filtered_logging_init(wmi_unified_t wmi_handle)
{
	/* do nothing */
}

static inline void wmi_filtered_logging_free(wmi_unified_t wmi_handle)
{
	/* do nothing */
}
#endif /* end of WMI_INTERFACE_FILTERED_EVENT_LOGGING */

#endif /*WMI_FILTERED_LOGGING_H*/
