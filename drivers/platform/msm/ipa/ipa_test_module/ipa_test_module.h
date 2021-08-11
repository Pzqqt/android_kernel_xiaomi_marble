// SPDX-License-Identifier: GPL-2.0-only
/*
* Copyright (c) 2017-2018,2020-2021, The Linux Foundation. All rights reserved.
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
	IPA_TEST_IOCTL_FLT_TBL_IN_SRAM,
	IPA_TEST_IOCTL_GET_MEM_PART,
	IPA_TEST_IOCTL_ULSO_CONFIGURE,
	IPA_TEST_IOCTL_ADD_HDR_HPC,
	IPA_TEST_IOCTL_PKT_INIT_EX_SET_HDR_OFST,
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
#define IPA_TEST_IOC_IS_TEST_PROD_FLT_IN_SRAM _IO(IPA_TEST_IOC_MAGIC, \
		IPA_TEST_IOCTL_FLT_TBL_IN_SRAM)
#define IPA_TEST_IOC_GET_MEM_PART _IOWR(IPA_TEST_IOC_MAGIC, \
		IPA_TEST_IOCTL_GET_MEM_PART, \
		struct ipa_test_mem_partition *)
#define IPA_TEST_IOC_ULSO_CONFIGURE _IOWR(IPA_TEST_IOC_MAGIC, \
		IPA_TEST_IOCTL_ULSO_CONFIGURE, \
		struct ipa_test_config_header *)
#define IPA_TEST_IOC_ADD_HDR_HPC _IOWR(IPA_TEST_IOC_MAGIC, \
		IPA_TEST_IOCTL_ADD_HDR_HPC, \
		struct ipa_ioc_add_hdr *)
#define IPA_TEST_IOC_PKT_INIT_EX_SET_HDR_OFST _IOWR(IPA_TEST_IOC_MAGIC, \
		IPA_TEST_IOCTL_PKT_INIT_EX_SET_HDR_OFST, \
		struct ipa_ioc_set_pkt_init_ex_hdr_ofst *)

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

struct ipa_test_mem_partition {
	unsigned ofst_start;
	unsigned v4_flt_hash_ofst;
	unsigned v4_flt_hash_size;
	unsigned v4_flt_hash_size_ddr;
	unsigned v4_flt_nhash_ofst;
	unsigned v4_flt_nhash_size;
	unsigned v4_flt_nhash_size_ddr;
	unsigned v6_flt_hash_ofst;
	unsigned v6_flt_hash_size;
	unsigned v6_flt_hash_size_ddr;
	unsigned v6_flt_nhash_ofst;
	unsigned v6_flt_nhash_size;
	unsigned v6_flt_nhash_size_ddr;
	unsigned v4_rt_num_index;
	unsigned v4_modem_rt_index_lo;
	unsigned v4_modem_rt_index_hi;
	unsigned v4_apps_rt_index_lo;
	unsigned v4_apps_rt_index_hi;
	unsigned v4_rt_hash_ofst;
	unsigned v4_rt_hash_size;
	unsigned v4_rt_hash_size_ddr;
	unsigned v4_rt_nhash_ofst;
	unsigned v4_rt_nhash_size;
	unsigned v4_rt_nhash_size_ddr;
	unsigned v6_rt_num_index;
	unsigned v6_modem_rt_index_lo;
	unsigned v6_modem_rt_index_hi;
	unsigned v6_apps_rt_index_lo;
	unsigned v6_apps_rt_index_hi;
	unsigned v6_rt_hash_ofst;
	unsigned v6_rt_hash_size;
	unsigned v6_rt_hash_size_ddr;
	unsigned v6_rt_nhash_ofst;
	unsigned v6_rt_nhash_size;
	unsigned v6_rt_nhash_size_ddr;
	unsigned modem_hdr_ofst;
	unsigned modem_hdr_size;
	unsigned apps_hdr_ofst;
	unsigned apps_hdr_size;
	unsigned apps_hdr_size_ddr;
	unsigned modem_hdr_proc_ctx_ofst;
	unsigned modem_hdr_proc_ctx_size;
	unsigned apps_hdr_proc_ctx_ofst;
	unsigned apps_hdr_proc_ctx_size;
	unsigned apps_hdr_proc_ctx_size_ddr;
	unsigned nat_tbl_ofst;
	unsigned nat_tbl_size;
	unsigned modem_comp_decomp_ofst;
	unsigned modem_comp_decomp_size;
	unsigned modem_ofst;
	unsigned modem_size;
	unsigned apps_v4_flt_hash_ofst;
	unsigned apps_v4_flt_hash_size;
	unsigned apps_v4_flt_nhash_ofst;
	unsigned apps_v4_flt_nhash_size;
	unsigned apps_v6_flt_hash_ofst;
	unsigned apps_v6_flt_hash_size;
	unsigned apps_v6_flt_nhash_ofst;
	unsigned apps_v6_flt_nhash_size;
	unsigned uc_info_ofst;
	unsigned uc_info_size;
	unsigned end_ofst;
	unsigned apps_v4_rt_hash_ofst;
	unsigned apps_v4_rt_hash_size;
	unsigned apps_v4_rt_nhash_ofst;
	unsigned apps_v4_rt_nhash_size;
	unsigned apps_v6_rt_hash_ofst;
	unsigned apps_v6_rt_hash_size;
	unsigned apps_v6_rt_nhash_ofst;
	unsigned apps_v6_rt_nhash_size;
	unsigned uc_descriptor_ram_ofst;
	unsigned uc_descriptor_ram_size;
	unsigned pdn_config_ofst;
	unsigned pdn_config_size;
	unsigned stats_quota_q6_ofst;
	unsigned stats_quota_q6_size;
	unsigned stats_quota_ap_ofst;
	unsigned stats_quota_ap_size;
	unsigned stats_tethering_ofst;
	unsigned stats_tethering_size;
	unsigned stats_fnr_ofst;
	unsigned stats_fnr_size;
	unsigned uc_ofst;
	unsigned uc_size;

	/* Irrelevant starting IPA4.5 */
	unsigned stats_flt_v4_ofst;
	unsigned stats_flt_v4_size;
	unsigned stats_flt_v6_ofst;
	unsigned stats_flt_v6_size;
	unsigned stats_rt_v4_ofst;
	unsigned stats_rt_v4_size;
	unsigned stats_rt_v6_ofst;
	unsigned stats_rt_v6_size;

	unsigned stats_drop_ofst;
	unsigned stats_drop_size;
};
#endif /* _IPA_TEST_MODULE_H_ */
