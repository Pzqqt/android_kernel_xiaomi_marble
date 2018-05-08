/*
 * Copyright (c) 2015-2018 The Linux Foundation. All rights reserved.
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

#ifndef __HIF_IRQ_AFFINITY_H__
#define __HIF_IRQ_AFFINITY_H__

#ifdef HIF_IRQ_AFFINITY
#ifndef FEATURE_NAPI
#error /*HIF_IRQ_AFFINITY currently relies on FEATURE_NAPI*/
#endif
#endif

/* CLD headers */
#include <hif.h> /* struct hif_opaque_softc; */
#include <hif_napi.h>
struct hif_opaque_softc;
enum qca_blacklist_op;

int hif_exec_cpu_migrate(struct qca_napi_data *napid, int cpu, int action);

int hif_exec_cpu_blacklist(struct qca_napi_data *napid,
			   enum qca_blacklist_op op);

#ifdef HIF_IRQ_AFFINITY
int hif_exec_event(struct hif_opaque_softc     *hif,
		   enum  qca_napi_event event,
		   void                *data);


/* hif_irq_affinity_remove() - remove affinity before freeing the irq
 * @os_irq: irq number to remove affinity from
 */
static inline void hif_irq_affinity_remove(int os_irq)
{
	irq_set_affinity_hint(os_irq, NULL);
}
#else
static inline void hif_irq_affinity_remove(int os_irq)
{
}

static inline int hif_exec_event(struct hif_opaque_softc     *hif,
		   enum  qca_napi_event event,
		   void                *data)
{
	return 0;
}
#endif
#endif
