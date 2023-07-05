// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2016-2017, 2020 The Linux Foundation. All rights reserved.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include "audio_pdr.h"

struct audio_pdr_service {
	void *pdr_handle;
	char service_name[SERVREG_NAME_LENGTH + 1];
	char service_path[SERVREG_NAME_LENGTH + 1];
};

static struct audio_pdr_service audio_pdr_services[AUDIO_PDR_DOMAIN_MAX] = {
	{	/* AUDIO_PDR_DOMAIN_ADSP */
		.service_name = "avs/audio",
		.service_path = "msm/adsp/audio_pd",
	}
};

void *audio_pdr_service_register(int domain_id, void (*cb)(int, char *, void *))
{
	if ((domain_id < 0) ||
	    (domain_id >= AUDIO_PDR_DOMAIN_MAX)) {
		pr_err("%s: Invalid service ID %d\n", __func__, domain_id);
		return ERR_PTR(-EINVAL);
	}

	audio_pdr_services[domain_id].pdr_handle = pdr_handle_alloc(cb, NULL);

	return pdr_add_lookup(audio_pdr_services[domain_id].pdr_handle,
			      audio_pdr_services[domain_id].service_name,
			      audio_pdr_services[domain_id].service_path);
}
EXPORT_SYMBOL(audio_pdr_service_register);

int audio_pdr_service_deregister(int domain_id)
{
	if ((domain_id < 0) ||
	    (domain_id >= AUDIO_PDR_DOMAIN_MAX)) {
		pr_err("%s: Invalid service ID %d\n", __func__, domain_id);
		return -EINVAL;
	}
	pdr_handle_release(audio_pdr_services[domain_id].pdr_handle);

	return 0;
}
EXPORT_SYMBOL(audio_pdr_service_deregister);

static int __init audio_pdr_late_init(void)
{
	return 0;
}
module_init(audio_pdr_late_init);

static void __exit audio_pdr_late_exit(void)
{

}
module_exit(audio_pdr_late_exit);

MODULE_DESCRIPTION("PDR framework driver");
MODULE_LICENSE("GPL v2");
