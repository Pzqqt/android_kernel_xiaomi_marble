// SPDX-License-Identifier: GPL-2.0-only
/*
* Copyright (c) 2017, The Linux Foundation. All rights reserved.
*/

#ifndef _IPA_RM_UT_H_
#define _IPA_RM_UT_H_

/**
 * ipa_rm_ut - unit test module
 * Defines sanity test scenarios executed from debugfs
 * writer function defined in ipa_rm module
 */

#include <linux/msm_ipa.h>
#include <linux/ipa.h>

int build_rmnet_bridge_use_case_graph(
		int (*create_resource)(struct ipa_rm_create_params *create_params),
		int (*consumer_cb)(enum ipa_rm_event event,
				enum ipa_rm_resource_name resource_name));

int build_rmnet_bridge_use_case_dependencies(
		int (*add_dependency)(enum ipa_rm_resource_name dependant_name,
						enum ipa_rm_resource_name dependency_name));
int request_release_resource_sequence(
		int (*resource_request)(enum ipa_rm_resource_name resource_name),
		int (*resource_release)(enum ipa_rm_resource_name resource_name));

void clean_ut(void);

#endif /* _IPA_RM_UT_H_ */
