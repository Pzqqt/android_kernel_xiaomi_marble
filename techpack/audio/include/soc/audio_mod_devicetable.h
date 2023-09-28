/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
 */

#ifndef AUDIO_MOD_DEVICETABLE_H
#define AUDIO_MOD_DEVICETABLE_H

#include <linux/mod_devicetable.h>

/* soundwire */

#define SOUNDWIRE_NAME_SIZE     32
#define SOUNDWIRE_MODULE_PREFIX "swr:"

struct swr_device_id {
	char name[SOUNDWIRE_NAME_SIZE];
	kernel_ulong_t driver_data;     /* Data private to the driver */
};

/* gpr */
#define GPR_NAME_SIZE	32
#define GPR_MODULE_PREFIX "gpr:"

struct gpr_device_id {
	char name[GPR_NAME_SIZE];
	__u32 domain_id;
	__u32 svc_id;
	__u32 svc_version;
	kernel_ulong_t driver_data;	/* Data private to the driver */
};

#endif /* AUDIO_MOD_DEVICETABLE_H */

