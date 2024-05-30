/* SPDX-License-Identifier: GPL-2.0-only */
/*
* Copyright (c) 2020, The Linux Foundation. All rights reserved.
*/

#ifndef _GSIHAL_I_H_
#define _GSIHAL_I_H_

#include "../gsi.h"
#include <linux/slab.h>
#include <linux/io.h>

struct gsihal_context {
	enum gsi_ver gsi_ver;
	void __iomem *base;
};

extern struct gsihal_context *gsihal_ctx;
#endif /* _GSIHAL_I_H_ */