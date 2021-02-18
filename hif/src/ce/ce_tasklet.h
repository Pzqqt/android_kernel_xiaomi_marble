/*
 * Copyright (c) 2015-2016,2018,2020-2021 The Linux Foundation.
 * All rights reserved.
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

#ifndef __CE_TASKLET_H__
#define __CE_TASKLET_H__
#include "ce_main.h"
void init_tasklet_worker_by_ceid(struct hif_opaque_softc *scn, int pipe_num);
void deinit_tasklet_workers(struct hif_opaque_softc *scn);
void ce_tasklet_init(struct HIF_CE_state *hif_ce_state, uint32_t mask);
void ce_tasklet_kill(struct hif_softc *scn);
int hif_drain_tasklets(struct hif_softc *scn);
int hif_drain_fw_diag_ce(struct hif_softc *scn);
QDF_STATUS ce_register_irq(struct HIF_CE_state *hif_ce_state, uint32_t mask);
QDF_STATUS ce_unregister_irq(struct HIF_CE_state *hif_ce_state, uint32_t mask);
irqreturn_t ce_dispatch_interrupt(int irq,
				  struct ce_tasklet_entry *tasklet_entry);
void hif_display_ce_stats(struct hif_softc *hif_ctx);
void hif_clear_ce_stats(struct HIF_CE_state *hif_ce_state);
#endif /* __CE_TASKLET_H__ */
