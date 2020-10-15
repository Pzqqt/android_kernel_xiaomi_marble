// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 */

/*
 * This module is dedicated for operations that must be performed at
 * init stage later than the main IPA driver and IPA clients manager init.
 *
 * E.g.:
 * Both net_dev driver and ipa driver are initialized during
 * the subsys_initcall, and the rmnet_ipa driver probe is called
 * by the ipa_pre_init or ipa_post_init or ipa_clients_manager_init,
 * the register_netdev() call will cause a kernel bug.
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ipa.h>
#include "ipa_i.h"
#include "ipa_qmi_service.h"

static int __init ipa_late_init(void)
{
	int rc = 0;

	IPADBG("IPA late init\n");

	rc = ipa3_wwan_platform_driver_register();
	if (rc) {
		IPAERR("ipa3_wwan_platform_driver_register failed: %d\n",
			   rc);
		ipa3_wwan_cleanup();
	}

	return rc;
}
fs_initcall(ipa_late_init);

static void __exit ipa_late_exit(void)
{
	IPADBG("IPA late exit\n");
	ipa3_wwan_cleanup();
}
module_exit(ipa_late_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("IPA late init module");

