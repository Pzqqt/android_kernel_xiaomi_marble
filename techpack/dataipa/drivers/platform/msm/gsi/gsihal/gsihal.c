// SPDX-License-Identifier: GPL-2.0-only
/*
* Copyright (c) 2020, The Linux Foundation. All rights reserved.
*/

#include "gsihal_i.h"
#include "gsihal_reg.h"

struct gsihal_context *gsihal_ctx;

int gsihal_init(enum gsi_ver gsi_ver, void __iomem *base)
{
	int result = 0;

	GSIDBG("initializing GSI HAL, GSI ver %d, base = %pK\n",
		gsi_ver, base);

	if (gsihal_ctx) {
		GSIDBG("gsihal already initialized\n");
		if (base != gsihal_ctx->base) {
			GSIERR(
				"base address of early init is differnet (%pK - %pK)\n"
			);
			WARN_ON(1);
		}
		result = -EEXIST;
		goto bail_err_exit;
	}

	if (gsi_ver < GSI_VER_1_0 || gsi_ver >= GSI_VER_MAX) {
		GSIERR("invalid GSI version %d\n", gsi_ver);
		result = -EINVAL;
		goto bail_err_exit;
	}

	if (!base) {
		GSIERR("invalid memory io mapping addr\n");
		result = -EINVAL;
		goto bail_err_exit;
	}

	gsihal_ctx = kzalloc(sizeof(*gsihal_ctx), GFP_KERNEL);
	if (!gsihal_ctx) {
		GSIERR("kzalloc err for gsihal_ctx\n");
		result = -ENOMEM;
		goto bail_err_exit;
	}

	gsihal_ctx->gsi_ver = gsi_ver;
	gsihal_ctx->base = base;

	if (gsihal_reg_init(gsi_ver)) {
		GSIERR("failed to initialize gsihal regs\n");
		result = -EINVAL;
		goto bail_free_ctx;
	}

	return 0;
bail_free_ctx:
	kfree(gsihal_ctx);
	gsihal_ctx = NULL;
bail_err_exit:
	return result;
}

void gsihal_destroy(void)
{
	GSIDBG("Entry\n");
	if (!gsihal_ctx) {
		GSIERR("gsihal_ctx not allocated.\n");
		return;
	}
	kfree(gsihal_ctx);
	gsihal_ctx = NULL;
}
