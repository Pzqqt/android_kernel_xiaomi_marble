/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
 * DOC: i_qdf_perf
 * This file provides OS dependent perf API's.
 */

#ifndef _I_QDF_PERF_H
#define _I_QDF_PERF_H

#ifdef QCA_PERF_PROFILING

#if (QCA_MIPS74K_PERF_PROFILING || QCA_MIPS24KK_PERF_PROFILING)
#include <qdf_mips_perf_pvt.h>
#endif

/* #defines required for structures */
#define MAX_SAMPLES_SHIFT   5   /* change this only*/
#define MAX_SAMPLES         (1 << MAX_SAMPLES_SHIFT)
#define INC_SAMPLES(x)      ((x + 1) & (MAX_SAMPLES - 1))
#define MAX_SAMPLE_SZ       (sizeof(uint32_t) * MAX_SAMPLES)
#define PER_SAMPLE_SZ       sizeof(uint32_t)

/**
 * typedef qdf_perf_entry_t - performance entry
 * @list: pointer to next
 * @child: pointer tochild
 * @parent: pointer to top
 * @type: perf cntr
 * @name: string
 * @proc: pointer to proc entry
 * @start_tsc: array at start tsc
 * @end_tsc: array at ent tsc
 * @samples: array of samples
 * @sample_idx: sample index
 * @lock_irq: lock irq
 */
typedef struct qdf_os_perf_entry {
	struct list_head        list;
	struct list_head        child;

	struct qdf_perf_entry   *parent;

	qdf_perf_cntr_t type;
	uint8_t *name;

	struct proc_dir_entry   *proc;

	uint64_t        start_tsc[MAX_SAMPLES];
	uint64_t        end_tsc[MAX_SAMPLES];

	uint32_t        samples[MAX_SAMPLES];
	uint32_t        sample_idx;

	spinlock_t      lock_irq;

} qdf_perf_entry_t;

/* typedefs */
typedef void *__qdf_perf_id_t;

#endif /* QCA_PERF_PROFILING */
#endif /* _I_QDF_PERF_H */
