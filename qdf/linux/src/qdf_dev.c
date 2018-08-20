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
 * DOC: qdf_dev
 * This file provides OS dependent device related APIs
 */

#include "qdf_dev.h"
#include "qdf_mem.h"
#include "qdf_util.h"
#include "qdf_module.h"
#include <linux/irq.h>

QDF_STATUS
qdf_dev_alloc_mem(struct qdf_dev *qdfdev, struct qdf_devm **mrptr,
		  uint32_t reqsize, uint32_t mask)
{
	struct qdf_devm *mptr;

	if (!qdfdev)
		return QDF_STATUS_E_INVAL;

	mptr = devm_kzalloc((struct device *)qdfdev, reqsize, mask);

	if (!mrptr)
		return QDF_STATUS_E_NOMEM;

	*mrptr = mptr;

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qdf_dev_alloc_mem);

QDF_STATUS
qdf_dev_release_mem(struct qdf_dev *qdfdev, struct qdf_devm *mrptr)
{
	if (!mrptr)
		return QDF_STATUS_E_INVAL;

	devm_kfree((struct device *)qdfdev, mrptr);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qdf_dev_release_mem);

QDF_STATUS
qdf_dev_modify_irq_status(uint32_t irnum, unsigned long cmask,
			  unsigned long smask)
{
	if (irnum <= 0)
		return QDF_STATUS_E_INVAL;

	irq_modify_status(irnum, cmask, smask);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(qdf_dev_modify_irq_status);

QDF_STATUS
qdf_dev_set_irq_affinity(uint32_t irnum, struct qdf_cpu_mask *cpmask)
{
	int ret;

	if (irnum <= 0)
		return QDF_STATUS_E_INVAL;

	ret = irq_set_affinity_hint(irnum, (struct cpumask *)cpmask);

	return qdf_status_from_os_return(ret);
}

qdf_export_symbol(qdf_dev_set_irq_affinity);
