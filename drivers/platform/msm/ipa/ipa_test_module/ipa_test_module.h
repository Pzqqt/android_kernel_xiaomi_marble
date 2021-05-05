// SPDX-License-Identifier: GPL-2.0-only
/*
* Copyright (c) 2017-2018,2020, The Linux Foundation. All rights reserved.
*/

#ifndef _IPA_TEST_MODULE_H_
#define _IPA_TEST_MODULE_H_

#include <stddef.h>
#include <linux/msm_ipa.h>
#include <linux/ioctl.h>
#ifdef _KERNEL_
#include <linux/ipa.h>
#endif

#define IPA_TEST_IOC_MAGIC 0xA5
enum {
	IPA_TEST_IOCTL_GET_HW_TYPE = 1,
	IPA_TEST_IOCTL_CONFIGURE,
	IPA_TEST_IOCTL_CLEAN,
	IPA_TEST_IOCTL_EP_CTRL,
	IPA_TEST_IOCTL_REG_SUSPEND_HNDL,
	IPA_TEST_IOCTL_HOLB_CONFIG,
	IPA_TEST_IOCTL_NUM,
};

#define IPA_TEST_IOC_GET_HW_TYPE _IO(IPA_TEST_IOC_MAGIC, \
		IPA_TEST_IOCTL_GET_HW_TYPE)
#define IPA_TEST_IOC_CONFIGURE _IOWR(IPA_TEST_IOC_MAGIC, \
		IPA_TEST_IOCTL_CONFIGURE, \
		struct ipa_test_config_header *)
#define IPA_TEST_IOC_CLEAN _IO(IPA_TEST_IOC_MAGIC, \
		IPA_TEST_IOCTL_CLEAN)
#define IPA_TEST_IOC_EP_CTRL _IOWR(IPA_TEST_IOC_MAGIC, \
		IPA_TEST_IOCTL_EP_CTRL, \
		struct ipa_test_ep_ctrl *)
#define IPA_TEST_IOC_REG_SUSPEND_HNDL _IOWR(IPA_TEST_IOC_MAGIC, \
		IPA_TEST_IOCTL_REG_SUSPEND_HNDL, \
		struct ipa_test_reg_suspend_handler *)
#define IPA_TEST_IOC_HOLB_CONFIG _IOWR(IPA_TEST_IOC_MAGIC, \
		IPA_TEST_IOCTL_HOLB_CONFIG, \
		struct handle_holb_config_ioctl *)

#define IPA_TEST_CONFIG_MARKER 0x57
#define IPA_TEST_CHANNEL_CONFIG_MARKER 0x83

/*
 * This is the configuration number that is saved for Generic configuration
 * we need it in order to allow coexistence of Generic
 * configured tests with old fashion tests
 */
#define GENERIC_TEST_CONFIGURATION_IDX 37788239

struct ipa_test_config_header
{
	unsigned char head_marker; /* IPA_TEST_CONFIG_MARKER */
	int to_ipa_channels_num;
	struct ipa_channel_config **to_ipa_channel_config;
	int from_ipa_channels_num;
	struct ipa_channel_config **from_ipa_channel_config;
	unsigned char tail_marker; /* IPA_TEST_CONFIG_MARKER */
};

struct ipa_test_en_status
{
	int num_clients;
	enum ipa_client_type *clients;
};

struct ipa_test_ep_ctrl
{
	bool ipa_ep_suspend;
	bool ipa_ep_delay;
	int from_dev_num;
};

struct ipa_test_reg_suspend_handler
{
	int DevNum;
	bool reg;
	bool deferred_flag;
};

struct ipa_channel_config
{
	unsigned char head_marker; /* IPA_TEST_CHANNEL_CONFIG_MARKER */
	enum ipa_client_type client;
	int index; /* shall be used for to_ipa_x or from_ipa_x */
	size_t config_size;
	void *cfg;
	bool en_status;
	unsigned char tail_marker; /* IPA_TEST_CHANNEL_CONFIG_MARKER */
};

struct ipa_test_holb_config
{
	enum ipa_client_type client;
	unsigned tmr_val;
	unsigned short en;
};
#endif /* _IPA_TEST_MODULE_H_ */
