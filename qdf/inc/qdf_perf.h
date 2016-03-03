/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: qdf_perf
 * This file provides OS abstraction perf API's.
 */

#ifndef _QDF_PERF_H
#define _QDF_PERF_H

/* headers */
#include <i_qdf_perf.h>

#ifdef QCA_PERF_PROFILING

/* Typedefs */
typedef __qdf_perf_id_t  qdf_perf_id_t;

typedef int (*proc_read_t)(char *page, char **start, off_t off, int count,
		int *eof, void *data);
typedef int (*proc_write_t)(struct file *file, const char *buf,
		unsigned long count, void *data);
typedef void (*perf_sample_t)(struct qdf_perf_entry  *entry,
		uint8_t  done);

typedef void (*perf_init_t)(struct qdf_perf_entry *entry, uint32_t def_val);

/**
 * typedef proc_api_tbl_t - contains functions to read, write to proc FS
 * @proc_read: function pointer to read function
 * @proc_write: function pointer to write function
 * @sample: function pointer to sample function
 * @init: function pointer to init function
 * @def_val: int contains default value
 */
typedef struct proc_api_tbl {
	proc_read_t     proc_read;
	proc_write_t    proc_write;
	perf_sample_t   sample;
	perf_init_t     init;
	uint32_t        def_val;
} proc_api_tbl_t;

proc_api_tbl_t          api_tbl[];

/* Macros */
#define INIT_API(name, val)    {   \
	.proc_read  = read_##name,     \
	.proc_write = write_##name,    \
	.sample     = sample_event,    \
	.init       = init_##name,     \
	.def_val    = val,             \
}

#define PERF_ENTRY(hdl) ((qdf_perf_entry_t *)hdl)

#define qdf_perf_init(_parent, _id, _ctr_type)   \
	__qdf_perf_init((_parent), (_id), (_ctr_type))

#define qdf_perf_destroy(_id) __qdf_perf_destroy((_id))

#define qdf_perf_start(_id) __qdf_perf_start((_id))

#define qdf_perf_end(_id)  __qdf_perf_end((_id))

/* Extern declarations */
extern __qdf_perf_id_t
	__qdf_perf_init(qdf_perf_id_t parent,
			uint8_t *id_name,
			qdf_perf_cntr_t type)(__qdf_perf_id_t parent,
						uint8_t *id_name,
						uint32_t type);

extern bool __qdf_perf_destroy(qdf_perf_id_t id)(__qdf_perf_id_t     id);

extern void __qdf_perf_start(qdf_perf_id_t id)(__qdf_perf_id_t       id);
extern void __qdf_perf_end(qdf_perf_id_t id)(__qdf_perf_id_t         id);

extern int
qdf_perfmod_init(void);
extern void
qdf_perfmod_exit(void);

#else /* !QCA_PERF_PROFILING */

#define qdf_perfmod_init()
#define qdf_perfmod_exit()
#define DECLARE_N_EXPORT_PERF_CNTR(id)
#define START_PERF_CNTR(_id, _name)
#define END_PERF_CNTR(_id)

#endif /* QCA_PERF_PROFILING */

#endif /* end of _QDF_PERF_H */
