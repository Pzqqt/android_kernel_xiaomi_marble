/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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

#include "dp_types.h"
#include "cdp_txrx_cmn_reg.h"

void dp_configure_arch_ops(struct dp_soc *soc);
qdf_size_t dp_get_soc_context_size(uint16_t device_id);

#ifdef CONFIG_LITHIUM
void dp_initialize_arch_ops_li(struct dp_arch_ops *arch_ops);
qdf_size_t dp_get_soc_context_size_li(void);
#endif

#ifdef CONFIG_BERYLLIUM
void dp_initialize_arch_ops_be(struct dp_arch_ops *arch_ops);
qdf_size_t dp_get_soc_context_size_be(void);
#endif

static void dp_initialize_default_arch_ops(struct dp_arch_ops *arch_ops)
{
/* assign dummy functions for arch_ops which are architecture specific */
}

qdf_size_t dp_get_soc_context_size(uint16_t device_id)

{
	switch (cdp_get_arch_type_from_devid(device_id)) {
#ifdef CONFIG_LITHIUM
	case CDP_ARCH_TYPE_LI:
		return dp_get_soc_context_size_li();
#endif

#ifdef CONFIG_BERYLLIUM
	case CDP_ARCH_TYPE_BE:
		return dp_get_soc_context_size_be();
	break;
#endif
	default:
		QDF_BUG(0);
	}

	return 0;
}

void dp_configure_arch_ops(struct dp_soc *soc)
{
	dp_initialize_default_arch_ops(&soc->arch_ops);

	switch (cdp_get_arch_type_from_devid(soc->device_id)) {
#ifdef CONFIG_LITHIUM
	case CDP_ARCH_TYPE_LI:
		dp_initialize_arch_ops_li(&soc->arch_ops);
	break;
#endif

#ifdef CONFIG_BERYLLIUM
	case CDP_ARCH_TYPE_BE:
		dp_initialize_arch_ops_be(&soc->arch_ops);
	break;
#endif
	default:
		QDF_BUG(0);
	}
}
