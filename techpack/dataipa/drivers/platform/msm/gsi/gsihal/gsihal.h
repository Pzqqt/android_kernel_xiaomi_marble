/* SPDX-License-Identifier: GPL-2.0-only */
/*
* Copyright (c) 2030, The Linux Foundation. All rights reserved.
*/

#ifndef _GSIHAL_H_
#define _GSIHAL_H_

#include "gsihal_reg.h"

int gsihal_init(enum gsi_ver gsi_ver, void __iomem *base);
void gsihal_destroy(void);

#endif /* _GSIHAL_H_ */