// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 */

#include <linux/clk.h>
#include <linux/compat.h>
#include <linux/device.h>
#include <linux/dmapool.h>
#include <linux/fs.h>
#include <linux/genalloc.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/rbtree.h>
#include <linux/uaccess.h>
#include <linux/interrupt.h>
#include <linux/interconnect.h>
#include <linux/netdevice.h>
#include <linux/delay.h>
#include <linux/msm_gsi.h>
#include <linux/time.h>
#include <linux/hashtable.h>
#include <linux/jhash.h>
#include <linux/pci.h>
#include <soc/qcom/subsystem_restart.h>
#include <linux/soc/qcom/smem.h>
#include <linux/qcom_scm.h>
#include <asm/cacheflush.h>
#include <linux/soc/qcom/smem_state.h>
#include <linux/of_irq.h>
#include <linux/ctype.h>
#include <linux/of_address.h>
#include <linux/qcom_scm.h>
#include <linux/soc/qcom/mdt_loader.h>
#include "gsi.h"
#include "ipa_stats.h"
#include <linux/suspend.h>

#ifdef CONFIG_ARM64

/* Outer caches unsupported on ARM64 platforms */
#define outer_flush_range(x, y)
#define __cpuc_flush_dcache_area __flush_dcache_area

#endif

#define DRV_NAME "ipa"
#define DELAY_BEFORE_FW_LOAD 500
#define IPA_SUBSYSTEM_NAME "ipa_fws"
#define IPA_UC_SUBSYSTEM_NAME "ipa_uc"

#include "ipa_i.h"
#include "ipa_rm_i.h"
#include "ipahal.h"
#include "ipahal_fltrt.h"

#define CREATE_TRACE_POINTS
#include "ipa_trace.h"
#include "ipa_odl.h"

#define IPA_SUSPEND_BUSY_TIMEOUT (msecs_to_jiffies(10))

#define DEFAULT_MPM_RING_SIZE_UL 6
#define DEFAULT_MPM_RING_SIZE_DL 16
#define DEFAULT_MPM_TETH_AGGR_SIZE 24
#define DEFAULT_MPM_UC_THRESH_SIZE 4

RAW_NOTIFIER_HEAD(ipa_rmnet_notifier_list);

/*
 * The following for adding code (ie. for EMULATION) not found on x86.
 */
#if defined(CONFIG_IPA_EMULATION)
# include "ipa_emulation_stubs.h"
#endif

#ifdef CONFIG_COMPAT
/**
 * struct ipa3_ioc_nat_alloc_mem32 - nat table memory allocation
 * properties
 * @dev_name: input parameter, the name of table
 * @size: input parameter, size of table in bytes
 * @offset: output parameter, offset into page in case of system memory
 */
struct ipa3_ioc_nat_alloc_mem32 {
	char dev_name[IPA_RESOURCE_NAME_MAX];
	compat_size_t size;
	compat_off_t offset;
};

/**
 * struct ipa_ioc_nat_ipv6ct_table_alloc32 - table memory allocation
 * properties
 * @size: input parameter, size of table in bytes
 * @offset: output parameter, offset into page in case of system memory
 */
struct ipa_ioc_nat_ipv6ct_table_alloc32 {
	compat_size_t size;
	compat_off_t offset;
};
#endif /* #ifdef CONFIG_COMPAT */

#define IPA_TZ_UNLOCK_ATTRIBUTE 0x0C0311

struct tz_smmu_ipa_protect_region_iovec_s {
	u64 input_addr;
	u64 output_addr;
	u64 size;
	u32 attr;
} __packed;

struct tz_smmu_ipa_protect_region_s {
	phys_addr_t iovec_buf;
	u32 size_bytes;
} __packed;

static void ipa3_start_tag_process(struct work_struct *work);
static DECLARE_WORK(ipa3_tag_work, ipa3_start_tag_process);

static void ipa3_transport_release_resource(struct work_struct *work);
static DECLARE_DELAYED_WORK(ipa3_transport_release_resource_work,
	ipa3_transport_release_resource);
static void ipa_gsi_notify_cb(struct gsi_per_notify *notify);

static int ipa3_attach_to_smmu(void);
static int ipa3_alloc_pkt_init(void);
static int ipa_alloc_pkt_init_ex(void);
static void ipa3_free_pkt_init(void);
static void ipa3_free_pkt_init_ex(void);

#ifdef CONFIG_DEEPSLEEP
static void ipa3_deepsleep_resume(void);
static void ipa3_deepsleep_suspend(void);
#endif

static void ipa3_load_ipa_fw(struct work_struct *work);
static DECLARE_WORK(ipa3_fw_loading_work, ipa3_load_ipa_fw);
static DECLARE_DELAYED_WORK(ipa3_fw_load_failure_handle, ipa3_load_ipa_fw);

static void ipa_dec_clients_disable_clks_on_wq(struct work_struct *work);
static DECLARE_DELAYED_WORK(ipa_dec_clients_disable_clks_on_wq_work,
	ipa_dec_clients_disable_clks_on_wq);

static DECLARE_DELAYED_WORK(ipa_dec_clients_disable_clks_on_suspend_irq_wq_work,
	ipa_dec_clients_disable_clks_on_wq);
static void ipa_inc_clients_enable_clks_on_wq(struct work_struct *work);
static DECLARE_WORK(ipa_inc_clients_enable_clks_on_wq_work,
	ipa_inc_clients_enable_clks_on_wq);

static int ipa3_ioctl_add_rt_rule_v2(unsigned long arg);
static int ipa3_ioctl_add_rt_rule_ext_v2(unsigned long arg);
static int ipa3_ioctl_add_rt_rule_after_v2(unsigned long arg);
static int ipa3_ioctl_mdfy_rt_rule_v2(unsigned long arg);
static int ipa3_ioctl_add_flt_rule_v2(unsigned long arg);
static int ipa3_ioctl_add_flt_rule_after_v2(unsigned long arg);
static int ipa3_ioctl_mdfy_flt_rule_v2(unsigned long arg);
static int ipa3_ioctl_fnr_counter_alloc(unsigned long arg);
static int ipa3_ioctl_fnr_counter_query(unsigned long arg);
static int ipa3_ioctl_fnr_counter_set(unsigned long arg);

static struct ipa3_plat_drv_res ipa3_res = {0, };

static struct clk *ipa3_clk;

struct ipa3_context *ipa3_ctx = NULL;
/* ipa_i.h is included in ipa_client modules and ipa3_ctx is
 * declared as extern in ipa_i.h. So export ipa3_ctx variable
 * to be visible to ipa_client module.
*/
EXPORT_SYMBOL(ipa3_ctx);

int ipa3_plat_drv_probe(struct platform_device *pdev_p);
int ipa3_pci_drv_probe(struct pci_dev *pci_dev,
	const struct pci_device_id *ent);

/**
 * ipa_get_hw_type() - Return IPA HW version
 *
 * Return value: enum ipa_hw_type
 */
enum ipa_hw_type ipa_get_hw_type_internal(void)
{
	if (ipa3_ctx == NULL)
		return IPA_HW_None;

	return ipa3_ctx->ipa_hw_type;
}

/**
 * ipa_is_test_prod_flt_in_sram_internal() - Return true if test prod FLT tbl is in SRAM
 *
 * Return value:bool
 */
bool ipa_is_test_prod_flt_in_sram_internal(enum ipa_ip_type ip)
{
	struct ipa3_flt_tbl *flt_tbl;
	const struct ipa_gsi_ep_config *gsi_ep_info_cfg;

	if (ipa3_ctx == NULL)
		return false;

	gsi_ep_info_cfg = ipa3_get_gsi_ep_info(IPA_CLIENT_TEST_PROD);
	if(gsi_ep_info_cfg == NULL)
		return false;

	flt_tbl = &ipa3_ctx->flt_tbl[gsi_ep_info_cfg->ipa_ep_num][ip];

	return !flt_tbl->force_sys[IPA_RULE_NON_HASHABLE] &&
		!flt_tbl->in_sys[IPA_RULE_NON_HASHABLE];
}
EXPORT_SYMBOL(ipa_is_test_prod_flt_in_sram_internal);

/**
 * ipa_assert() - general function for assertion
 */
void ipa_assert(void)
{
	pr_err("IPA: unrecoverable error has occurred, asserting\n");
	BUG();
}
EXPORT_SYMBOL(ipa_assert);

#if defined(CONFIG_IPA_EMULATION)
static bool running_emulation = true;
#else
static bool running_emulation;
#endif

static const struct of_device_id ipa_pci_drv_match[] = {
	{ .compatible = "qcom,ipa", },
	{}
};

#define LOCAL_VENDOR 0x17CB
#define LOCAL_DEVICE 0x00ff

static const char ipa_pci_driver_name[] = "qcipav3";

static const struct pci_device_id ipa_pci_tbl[] = {
	{ PCI_DEVICE(LOCAL_VENDOR, LOCAL_DEVICE) },
	{ 0, 0, 0, 0, 0, 0, 0 }
};

MODULE_DEVICE_TABLE(pci, ipa_pci_tbl);

static void ipa_pci_remove(struct pci_dev *pci_dev)
{
}

static void ipa_pci_shutdown(struct pci_dev *pci_dev)
{
}

static pci_ers_result_t ipa_pci_io_error_detected(struct pci_dev *pci_dev,
	pci_channel_state_t state)
{
	return 0;
}

static pci_ers_result_t ipa_pci_io_slot_reset(struct pci_dev *pci_dev)
{
	return 0;
}

static void ipa_pci_io_resume(struct pci_dev *pci_dev)
{
}

/* PCI Error Recovery */
static const struct pci_error_handlers ipa_pci_err_handler = {
	.error_detected = ipa_pci_io_error_detected,
	.slot_reset = ipa_pci_io_slot_reset,
	.resume = ipa_pci_io_resume,
};

static struct pci_driver ipa_pci_driver = {
	.name = ipa_pci_driver_name,
	.id_table = ipa_pci_tbl,
	.probe    = ipa3_pci_drv_probe,
	.remove   = ipa_pci_remove,
	.shutdown = ipa_pci_shutdown,
	.err_handler = &ipa_pci_err_handler
};

static const struct of_device_id ipa_plat_drv_match[] = {
	{ .compatible = "qcom,ipa", },
	{ .compatible = "qcom,ipa-smmu-ap-cb", },
	{ .compatible = "qcom,ipa-smmu-wlan-cb", },
	{ .compatible = "qcom,ipa-smmu-uc-cb", },
	{ .compatible = "qcom,ipa-smmu-11ad-cb", },
	{ .compatible = "qcom,ipa-smmu-eth-cb", },
	{ .compatible = "qcom,ipa-smmu-eth1-cb", },
	{ .compatible = "qcom,ipa-smmu-wlan1-cb", },
	{ .compatible = "qcom,smp2p-map-ipa-1-in", },
	{ .compatible = "qcom,smp2p-map-ipa-1-out", },
	{}
};

/**
 * ipa_write_64() - convert 64 bit value to byte array
 * @w: 64 bit integer
 * @dest: byte array
 *
 * Return value: converted value
 */
u8 *ipa_write_64(u64 w, u8 *dest)
{
	if (unlikely(dest == NULL)) {
		pr_err("%s: NULL address\n", __func__);
		return dest;
	}
	*dest++ = (u8)((w) & 0xFF);
	*dest++ = (u8)((w >> 8) & 0xFF);
	*dest++ = (u8)((w >> 16) & 0xFF);
	*dest++ = (u8)((w >> 24) & 0xFF);
	*dest++ = (u8)((w >> 32) & 0xFF);
	*dest++ = (u8)((w >> 40) & 0xFF);
	*dest++ = (u8)((w >> 48) & 0xFF);
	*dest++ = (u8)((w >> 56) & 0xFF);

	return dest;
}

/**
 * ipa_write_32() - convert 32 bit value to byte array
 * @w: 32 bit integer
 * @dest: byte array
 *
 * Return value: converted value
 */
u8 *ipa_write_32(u32 w, u8 *dest)
{
	if (unlikely(dest == NULL)) {
		pr_err("%s: NULL address\n", __func__);
		return dest;
	}
	*dest++ = (u8)((w) & 0xFF);
	*dest++ = (u8)((w >> 8) & 0xFF);
	*dest++ = (u8)((w >> 16) & 0xFF);
	*dest++ = (u8)((w >> 24) & 0xFF);

	return dest;
}

/**
 * ipa_write_16() - convert 16 bit value to byte array
 * @hw: 16 bit integer
 * @dest: byte array
 *
 * Return value: converted value
 */
u8 *ipa_write_16(u16 hw, u8 *dest)
{
	if (unlikely(dest == NULL)) {
		pr_err("%s: NULL address\n", __func__);
		return dest;
	}
	*dest++ = (u8)((hw) & 0xFF);
	*dest++ = (u8)((hw >> 8) & 0xFF);

	return dest;
}

/**
 * ipa_write_8() - convert 8 bit value to byte array
 * @hw: 8 bit integer
 * @dest: byte array
 *
 * Return value: converted value
 */
u8 *ipa_write_8(u8 b, u8 *dest)
{
	if (unlikely(dest == NULL)) {
		WARN(1, "%s: NULL address\n", __func__);
		return dest;
	}
	*dest++ = (b) & 0xFF;

	return dest;
}

/**
 * ipa_pad_to_64() - pad byte array to 64 bit value
 * @dest: byte array
 *
 * Return value: padded value
 */
u8 *ipa_pad_to_64(u8 *dest)
{
	int i;
	int j;

	if (unlikely(dest == NULL)) {
		WARN(1, "%s: NULL address\n", __func__);
		return dest;
	}

	i = (long)dest & 0x7;

	if (i)
		for (j = 0; j < (8 - i); j++)
			*dest++ = 0;

	return dest;
}

/**
 * ipa_pad_to_32() - pad byte array to 32 bit value
 * @dest: byte array
 *
 * Return value: padded value
 */
u8 *ipa_pad_to_32(u8 *dest)
{
	int i;
	int j;

	if (unlikely(dest == NULL)) {
		WARN(1, "%s: NULL address\n", __func__);
		return dest;
	}

	i = (long)dest & 0x7;

	if (i)
		for (j = 0; j < (4 - i); j++)
			*dest++ = 0;

	return dest;
}

int ipa_smmu_store_sgt(struct sg_table **out_ch_ptr,
	struct sg_table *in_sgt_ptr)
{
	unsigned int nents;
	int i;
	struct scatterlist *in_sg, *out_sg;

	if (in_sgt_ptr != NULL) {
		*out_ch_ptr = kzalloc(sizeof(struct sg_table), GFP_KERNEL);
		if (*out_ch_ptr == NULL)
			return -ENOMEM;

		nents = in_sgt_ptr->nents;

		(*out_ch_ptr)->sgl =
			kcalloc(nents, sizeof(struct scatterlist),
				GFP_KERNEL);
		if ((*out_ch_ptr)->sgl == NULL) {
			kfree(*out_ch_ptr);
			*out_ch_ptr = NULL;
			return -ENOMEM;
		}

		out_sg = (*out_ch_ptr)->sgl;
		for_each_sg(in_sgt_ptr->sgl, in_sg, in_sgt_ptr->nents, i) {
			memcpy(out_sg, in_sg, sizeof(struct scatterlist));
			out_sg++;
		}

		(*out_ch_ptr)->nents = nents;
		(*out_ch_ptr)->orig_nents = in_sgt_ptr->orig_nents;
	}
	return 0;
}
EXPORT_SYMBOL(ipa_smmu_store_sgt);

int ipa_smmu_free_sgt(struct sg_table **out_sgt_ptr)
{
	if (*out_sgt_ptr != NULL) {
		kfree((*out_sgt_ptr)->sgl);
		(*out_sgt_ptr)->sgl = NULL;
		kfree(*out_sgt_ptr);
		*out_sgt_ptr = NULL;
	}
	return 0;
}
EXPORT_SYMBOL(ipa_smmu_free_sgt);

/**
 * ipa_pm_notify() - PM notify to listen suspend events
 *
 * This callback will be invoked by the pm framework to suspend
 * operation is invoked.
 *
 * Returns NOTIFY_DONE to pm framework completed operation.
 */

static int ipa_pm_notify(struct notifier_block *b, unsigned long event, void *p)
{
	IPAERR("Entry\n");
	switch (event) {
		case PM_POST_SUSPEND:
#ifdef CONFIG_DEEPSLEEP
			if (mem_sleep_current == PM_SUSPEND_MEM && ipa3_ctx->deepsleep) {
				IPADBG("Enter deepsleep resume\n");
				ipa3_deepsleep_resume();
				IPADBG("Exit deepsleep resume\n");
			}
#endif
			break;
	}
	IPAERR("Exit\n");
	return NOTIFY_DONE;
}


static struct notifier_block ipa_pm_notifier = {
	.notifier_call = ipa_pm_notify,
};

static const struct dev_pm_ops ipa_pm_ops = {
	.suspend_late = ipa3_ap_suspend,
	.resume_early = ipa3_ap_resume,
};

static struct platform_driver ipa_plat_drv = {
	.probe = ipa3_plat_drv_probe,
	.driver = {
		.name = DRV_NAME,
		.pm = &ipa_pm_ops,
		.of_match_table = ipa_plat_drv_match,
	},
};

static struct {
	bool present[IPA_SMMU_CB_MAX];
	bool arm_smmu;
	bool use_64_bit_dma_mask;
	u32 ipa_base;
	u32 ipa_size;
} smmu_info;

static char *active_clients_table_buf;

int ipa3_active_clients_log_print_buffer(char *buf, int size)
{
	int i;
	int nbytes;
	int cnt = 0;
	int start_idx;
	int end_idx;
	unsigned long flags;

	spin_lock_irqsave(&ipa3_ctx->ipa3_active_clients_logging.lock, flags);
	start_idx = (ipa3_ctx->ipa3_active_clients_logging.log_tail + 1) %
			IPA3_ACTIVE_CLIENTS_LOG_BUFFER_SIZE_LINES;
	end_idx = ipa3_ctx->ipa3_active_clients_logging.log_head;
	for (i = start_idx; i != end_idx;
		i = (i + 1) % IPA3_ACTIVE_CLIENTS_LOG_BUFFER_SIZE_LINES) {
		nbytes = scnprintf(buf + cnt, size - cnt, "%s\n",
				ipa3_ctx->ipa3_active_clients_logging
				.log_buffer[i]);
		cnt += nbytes;
	}
	spin_unlock_irqrestore(&ipa3_ctx->ipa3_active_clients_logging.lock,
		flags);

	return cnt;
}

int ipa3_active_clients_log_print_table(char *buf, int size)
{
	int i;
	struct ipa3_active_client_htable_entry *iterator;
	int cnt = 0;
	unsigned long flags;

	spin_lock_irqsave(&ipa3_ctx->ipa3_active_clients_logging.lock, flags);
	cnt = scnprintf(buf, size, "\n---- Active Clients Table ----\n");
	hash_for_each(ipa3_ctx->ipa3_active_clients_logging.htable, i,
			iterator, list) {
		switch (iterator->type) {
		case IPA3_ACTIVE_CLIENT_LOG_TYPE_EP:
			cnt += scnprintf(buf + cnt, size - cnt,
					"%-40s %-3d ENDPOINT\n",
					iterator->id_string, iterator->count);
			break;
		case IPA3_ACTIVE_CLIENT_LOG_TYPE_SIMPLE:
			cnt += scnprintf(buf + cnt, size - cnt,
					"%-40s %-3d SIMPLE\n",
					iterator->id_string, iterator->count);
			break;
		case IPA3_ACTIVE_CLIENT_LOG_TYPE_RESOURCE:
			cnt += scnprintf(buf + cnt, size - cnt,
					"%-40s %-3d RESOURCE\n",
					iterator->id_string, iterator->count);
			break;
		case IPA3_ACTIVE_CLIENT_LOG_TYPE_SPECIAL:
			cnt += scnprintf(buf + cnt, size - cnt,
					"%-40s %-3d SPECIAL\n",
					iterator->id_string, iterator->count);
			break;
		default:
			IPAERR("Trying to print illegal active_clients type");
			break;
		}
	}
	cnt += scnprintf(buf + cnt, size - cnt,
			"\nTotal active clients count: %d\n",
			atomic_read(&ipa3_ctx->ipa3_active_clients.cnt));

	if (ipa3_is_mhip_offload_enabled())
		cnt += ipa_mpm_panic_handler(buf + cnt, size - cnt);

	spin_unlock_irqrestore(&ipa3_ctx->ipa3_active_clients_logging.lock,
		flags);

	return cnt;
}

static int ipa3_clean_modem_rule(void)
{
	struct ipa_install_fltr_rule_req_msg_v01 *req;
	struct ipa_install_fltr_rule_req_ex_msg_v01 *req_ex;
	int val = 0;

	if (ipa3_ctx->ipa_hw_type < IPA_HW_v3_0) {
		req = kzalloc(
			sizeof(struct ipa_install_fltr_rule_req_msg_v01),
			GFP_KERNEL);
		if (!req) {
			IPAERR("mem allocated failed!\n");
			return -ENOMEM;
		}
		req->filter_spec_list_valid = false;
		req->filter_spec_list_len = 0;
		req->source_pipe_index_valid = 0;
		val = ipa3_qmi_filter_request_send(req);
		kfree(req);
	} else {
		req_ex = kzalloc(
			sizeof(struct ipa_install_fltr_rule_req_ex_msg_v01),
			GFP_KERNEL);
		if (!req_ex) {
			IPAERR("mem allocated failed!\n");
			return -ENOMEM;
		}
		req_ex->filter_spec_ex_list_valid = false;
		req_ex->filter_spec_ex_list_len = 0;
		req_ex->source_pipe_index_valid = 0;
		val = ipa3_qmi_filter_request_ex_send(req_ex);
		kfree(req_ex);
	}

	return val;
}

static int ipa3_clean_mhip_dl_rule(void)
{
	struct ipa_remove_offload_connection_req_msg_v01 req;

	memset(&req, 0, sizeof(struct
		ipa_remove_offload_connection_req_msg_v01));

	req.clean_all_rules_valid = true;
	req.clean_all_rules = true;

	if (ipa3_qmi_rmv_offload_request_send(&req)) {
		IPAWANDBG("clean dl rule cache failed\n");
		return -EFAULT;
	}

	return 0;
}

#ifdef CONFIG_IPA_DEBUG
static int ipa3_active_clients_log_insert(const char *string)
{
	int head;
	int tail;

	if (!ipa3_ctx->ipa3_active_clients_logging.log_rdy)
		return -EPERM;

	head = ipa3_ctx->ipa3_active_clients_logging.log_head;
	tail = ipa3_ctx->ipa3_active_clients_logging.log_tail;

	memset(ipa3_ctx->ipa3_active_clients_logging.log_buffer[head], '_',
			IPA3_ACTIVE_CLIENTS_LOG_LINE_LEN);
	strlcpy(ipa3_ctx->ipa3_active_clients_logging.log_buffer[head], string,
			(size_t)IPA3_ACTIVE_CLIENTS_LOG_LINE_LEN);
	head = (head + 1) % IPA3_ACTIVE_CLIENTS_LOG_BUFFER_SIZE_LINES;
	if (tail == head)
		tail = (tail + 1) % IPA3_ACTIVE_CLIENTS_LOG_BUFFER_SIZE_LINES;

	ipa3_ctx->ipa3_active_clients_logging.log_tail = tail;
	ipa3_ctx->ipa3_active_clients_logging.log_head = head;

	return 0;
}
#endif

static int ipa3_active_clients_log_init(void)
{
	int i;

	spin_lock_init(&ipa3_ctx->ipa3_active_clients_logging.lock);
	ipa3_ctx->ipa3_active_clients_logging.log_buffer[0] = kcalloc(
			IPA3_ACTIVE_CLIENTS_LOG_BUFFER_SIZE_LINES,
			sizeof(char[IPA3_ACTIVE_CLIENTS_LOG_LINE_LEN]),
			GFP_KERNEL);
	active_clients_table_buf = kzalloc(sizeof(
			char[IPA3_ACTIVE_CLIENTS_TABLE_BUF_SIZE]), GFP_KERNEL);
	if (ipa3_ctx->ipa3_active_clients_logging.log_buffer[0] == NULL) {
		pr_err("Active Clients Logging memory allocation failed\n");
		goto bail;
	}
	for (i = 0; i < IPA3_ACTIVE_CLIENTS_LOG_BUFFER_SIZE_LINES; i++) {
		ipa3_ctx->ipa3_active_clients_logging.log_buffer[i] =
			ipa3_ctx->ipa3_active_clients_logging.log_buffer[0] +
			(IPA3_ACTIVE_CLIENTS_LOG_LINE_LEN * i);
	}
	ipa3_ctx->ipa3_active_clients_logging.log_head = 0;
	ipa3_ctx->ipa3_active_clients_logging.log_tail =
			IPA3_ACTIVE_CLIENTS_LOG_BUFFER_SIZE_LINES - 1;
	hash_init(ipa3_ctx->ipa3_active_clients_logging.htable);
	ipa3_ctx->ipa3_active_clients_logging.log_rdy = true;

	return 0;

bail:
	return -ENOMEM;
}

void ipa3_active_clients_log_clear(void)
{
	unsigned long flags;

	spin_lock_irqsave(&ipa3_ctx->ipa3_active_clients_logging.lock, flags);
	ipa3_ctx->ipa3_active_clients_logging.log_head = 0;
	ipa3_ctx->ipa3_active_clients_logging.log_tail =
			IPA3_ACTIVE_CLIENTS_LOG_BUFFER_SIZE_LINES - 1;
	spin_unlock_irqrestore(&ipa3_ctx->ipa3_active_clients_logging.lock,
		flags);
}

static void ipa3_active_clients_log_destroy(void)
{
	unsigned long flags;

	spin_lock_irqsave(&ipa3_ctx->ipa3_active_clients_logging.lock, flags);
	ipa3_ctx->ipa3_active_clients_logging.log_rdy = false;
	kfree(active_clients_table_buf);
	active_clients_table_buf = NULL;
	kfree(ipa3_ctx->ipa3_active_clients_logging.log_buffer[0]);
	ipa3_ctx->ipa3_active_clients_logging.log_buffer[0] = NULL;
	ipa3_ctx->ipa3_active_clients_logging.log_head = 0;
	ipa3_ctx->ipa3_active_clients_logging.log_tail =
			IPA3_ACTIVE_CLIENTS_LOG_BUFFER_SIZE_LINES - 1;
	spin_unlock_irqrestore(&ipa3_ctx->ipa3_active_clients_logging.lock,
		flags);
}

static struct ipa_smmu_cb_ctx smmu_cb[IPA_SMMU_CB_MAX];

struct iommu_domain *ipa3_get_smmu_domain_by_type(enum ipa_smmu_cb_type cb_type)
{
	if (VALID_IPA_SMMU_CB_TYPE(cb_type) && smmu_cb[cb_type].valid)
		return smmu_cb[cb_type].iommu_domain;

	IPAERR("cb_type(%d) not valid\n", cb_type);

	return NULL;
}

struct iommu_domain *ipa3_get_smmu_domain(void)
{
	return ipa3_get_smmu_domain_by_type(IPA_SMMU_CB_AP);
}

struct iommu_domain *ipa3_get_uc_smmu_domain(void)
{
	return ipa3_get_smmu_domain_by_type(IPA_SMMU_CB_UC);
}

struct iommu_domain *ipa3_get_wlan_smmu_domain(void)
{
	return ipa3_get_smmu_domain_by_type(IPA_SMMU_CB_WLAN);
}

struct iommu_domain *ipa3_get_wlan1_smmu_domain(void)
{
	return ipa3_get_smmu_domain_by_type(IPA_SMMU_CB_WLAN1);
}

struct iommu_domain *ipa3_get_eth_smmu_domain(void)
{
	return ipa3_get_smmu_domain_by_type(IPA_SMMU_CB_ETH);
}

struct iommu_domain *ipa3_get_eth1_smmu_domain(void)
{
	return ipa3_get_smmu_domain_by_type(IPA_SMMU_CB_ETH1);
}

struct iommu_domain *ipa3_get_11ad_smmu_domain(void)
{
	return ipa3_get_smmu_domain_by_type(IPA_SMMU_CB_11AD);
}

struct device *ipa3_get_wlan_device(void)
{
	if (ipa3_ctx->ipa_hw_type == IPA_HW_v4_5 &&
		ipa_get_wdi_version() == IPA_WDI_1) {
		if (smmu_cb[IPA_SMMU_CB_WLAN].valid)
			return smmu_cb[IPA_SMMU_CB_WLAN].dev;
		IPAERR("CB not valid\n");
		return NULL;
	}
	return ipa3_ctx->pdev;
}

struct device *ipa3_get_dma_dev(void)
{
	return ipa3_ctx->pdev;
}
EXPORT_SYMBOL(ipa3_get_dma_dev);


/**
 * ipa3_get_smmu_ctx()- Return smmu context for the given cb_type
 *
 * Return value: pointer to smmu context address
 */
struct ipa_smmu_cb_ctx *ipa3_get_smmu_ctx(enum ipa_smmu_cb_type cb_type)
{
	return &smmu_cb[cb_type];
}
EXPORT_SYMBOL(ipa3_get_smmu_ctx);

static int ipa3_open(struct inode *inode, struct file *filp)
{
	IPADBG_LOW("ENTER\n");
	filp->private_data = ipa3_ctx;

	return 0;
}

static void ipa3_wan_msg_free_cb(void *buff, u32 len, u32 type)
{
	if (!buff) {
		IPAERR("Null buffer\n");
		return;
	}

	if (type != WAN_UPSTREAM_ROUTE_ADD &&
	    type != WAN_UPSTREAM_ROUTE_DEL &&
	    type != WAN_EMBMS_CONNECT) {
		IPAERR("Wrong type given. buff %pK type %d\n", buff, type);
		return;
	}

	kfree(buff);
}

static int ipa3_send_wan_msg(unsigned long usr_param,
	uint8_t msg_type, bool is_cache)
{
	int retval;
	struct ipa_wan_msg *wan_msg;
	struct ipa_msg_meta msg_meta;
	struct ipa_wan_msg cache_wan_msg;

	wan_msg = kzalloc(sizeof(*wan_msg), GFP_KERNEL);
	if (!wan_msg)
		return -ENOMEM;

	if (copy_from_user(wan_msg, (const void __user *)usr_param,
		sizeof(struct ipa_wan_msg))) {
		kfree(wan_msg);
		return -EFAULT;
	}

	memcpy(&cache_wan_msg, wan_msg, sizeof(cache_wan_msg));

	memset(&msg_meta, 0, sizeof(struct ipa_msg_meta));
	msg_meta.msg_type = msg_type;
	msg_meta.msg_len = sizeof(struct ipa_wan_msg);
	retval = ipa3_send_msg(&msg_meta, wan_msg, ipa3_wan_msg_free_cb);
	if (retval) {
		IPAERR_RL("ipa3_send_msg failed: %d\n", retval);
		kfree(wan_msg);
		return retval;
	}

	if (is_cache) {
		mutex_lock(&ipa3_ctx->ipa_cne_evt_lock);

		/* cache the cne event */
		memcpy(&ipa3_ctx->ipa_cne_evt_req_cache[
			ipa3_ctx->num_ipa_cne_evt_req].wan_msg,
			&cache_wan_msg,
			sizeof(cache_wan_msg));

		memcpy(&ipa3_ctx->ipa_cne_evt_req_cache[
			ipa3_ctx->num_ipa_cne_evt_req].msg_meta,
			&msg_meta,
			sizeof(struct ipa_msg_meta));

		ipa3_ctx->num_ipa_cne_evt_req++;
		ipa3_ctx->num_ipa_cne_evt_req %= IPA_MAX_NUM_REQ_CACHE;
		mutex_unlock(&ipa3_ctx->ipa_cne_evt_lock);
	}

	return 0;
}

static void ipa3_vlan_l2tp_msg_free_cb(void *buff, u32 len, u32 type)
{
	if (!buff) {
		IPAERR("Null buffer\n");
		return;
	}

	switch (type) {
	case ADD_VLAN_IFACE:
	case DEL_VLAN_IFACE:
	case ADD_L2TP_VLAN_MAPPING:
	case DEL_L2TP_VLAN_MAPPING:
	case ADD_BRIDGE_VLAN_MAPPING:
	case DEL_BRIDGE_VLAN_MAPPING:
		break;
	default:
		IPAERR("Wrong type given. buff %pK type %d\n", buff, type);
		return;
	}

	kfree(buff);
}

static void ipa3_pdn_config_msg_free_cb(void *buff, u32 len, u32 type)
{
	if (!buff) {
		IPAERR("Null buffer\n");
		return;
	}

	kfree(buff);
}

static int ipa3_send_pdn_config_msg(unsigned long usr_param)
{
	int retval;
	struct ipa_ioc_pdn_config *pdn_info;
	struct ipa_msg_meta msg_meta;
	void *buff;

	memset(&msg_meta, 0, sizeof(msg_meta));

	pdn_info = kzalloc(sizeof(struct ipa_ioc_pdn_config),
		GFP_KERNEL);
	if (!pdn_info)
		return -ENOMEM;

	if (copy_from_user((u8 *)pdn_info, (void __user *)usr_param,
		sizeof(struct ipa_ioc_pdn_config))) {
		kfree(pdn_info);
		return -EFAULT;
	}

	msg_meta.msg_len = sizeof(struct ipa_ioc_pdn_config);
	buff = pdn_info;

	msg_meta.msg_type = pdn_info->pdn_cfg_type;
	/* null terminate the string */
	pdn_info->dev_name[IPA_RESOURCE_NAME_MAX - 1] = '\0';
	if ((pdn_info->pdn_cfg_type < IPA_PDN_DEFAULT_MODE_CONFIG) ||
			(pdn_info->pdn_cfg_type >= IPA_PDN_CONFIG_EVENT_MAX)) {
		IPAERR_RL("invalid pdn_cfg_type =%d", pdn_info->pdn_cfg_type);
		kfree(pdn_info);
		return -EINVAL;
	}

	IPADBG("type %d, interface name: %s, enable:%d\n", msg_meta.msg_type,
		pdn_info->dev_name, pdn_info->enable);

	if (pdn_info->pdn_cfg_type == IPA_PDN_IP_PASSTHROUGH_MODE_CONFIG) {
		IPADBG("Client MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
			pdn_info->u.passthrough_cfg.client_mac_addr[0],
			pdn_info->u.passthrough_cfg.client_mac_addr[1],
			pdn_info->u.passthrough_cfg.client_mac_addr[2],
			pdn_info->u.passthrough_cfg.client_mac_addr[3],
			pdn_info->u.passthrough_cfg.client_mac_addr[4],
			pdn_info->u.passthrough_cfg.client_mac_addr[5]);
	}

	retval = ipa3_send_msg(&msg_meta, buff,
		ipa3_pdn_config_msg_free_cb);
	if (retval) {
		IPAERR("ipa3_send_msg failed: %d, msg_type %d\n",
			retval,
			msg_meta.msg_type);
		kfree(buff);
		return retval;
	}
	IPADBG("exit\n");

	return 0;
}

static int ipa3_send_vlan_l2tp_msg(unsigned long usr_param, uint8_t msg_type)
{
	int retval;
	struct ipa_ioc_vlan_iface_info *vlan_info;
	struct ipa_ioc_l2tp_vlan_mapping_info *mapping_info;
	struct ipa_ioc_bridge_vlan_mapping_info *bridge_vlan_info;
	struct ipa_msg_meta msg_meta;
	void *buff;

	IPADBG("type %d\n", msg_type);

	memset(&msg_meta, 0, sizeof(msg_meta));
	msg_meta.msg_type = msg_type;

	if ((msg_type == ADD_VLAN_IFACE) ||
		(msg_type == DEL_VLAN_IFACE)) {
		vlan_info = kzalloc(sizeof(struct ipa_ioc_vlan_iface_info),
			GFP_KERNEL);
		if (!vlan_info)
			return -ENOMEM;

		if (copy_from_user((u8 *)vlan_info, (void __user *)usr_param,
			sizeof(struct ipa_ioc_vlan_iface_info))) {
			kfree(vlan_info);
			return -EFAULT;
		}

		msg_meta.msg_len = sizeof(struct ipa_ioc_vlan_iface_info);
		buff = vlan_info;
	} else if ((msg_type == ADD_L2TP_VLAN_MAPPING) ||
		(msg_type == DEL_L2TP_VLAN_MAPPING)) {
		mapping_info = kzalloc(sizeof(struct
			ipa_ioc_l2tp_vlan_mapping_info), GFP_KERNEL);
		if (!mapping_info)
			return -ENOMEM;

		if (copy_from_user((u8 *)mapping_info,
			(void __user *)usr_param,
			sizeof(struct ipa_ioc_l2tp_vlan_mapping_info))) {
			kfree(mapping_info);
			return -EFAULT;
		}

		msg_meta.msg_len = sizeof(struct
			ipa_ioc_l2tp_vlan_mapping_info);
		buff = mapping_info;
	} else if ((msg_type == ADD_BRIDGE_VLAN_MAPPING) ||
		(msg_type == DEL_BRIDGE_VLAN_MAPPING)) {
		bridge_vlan_info = kzalloc(
			sizeof(struct ipa_ioc_bridge_vlan_mapping_info),
			GFP_KERNEL);
		if (!bridge_vlan_info)
			return -ENOMEM;

		if (copy_from_user((u8 *)bridge_vlan_info,
			(void __user *)usr_param,
			sizeof(struct ipa_ioc_bridge_vlan_mapping_info))) {
			kfree(bridge_vlan_info);
			IPAERR("copy from user failed\n");
			return -EFAULT;
		}

		msg_meta.msg_len = sizeof(struct
			ipa_ioc_bridge_vlan_mapping_info);
		buff = bridge_vlan_info;
	} else {
		IPAERR("Unexpected event\n");
		return -EFAULT;
	}

	retval = ipa3_send_msg(&msg_meta, buff,
		ipa3_vlan_l2tp_msg_free_cb);
	if (retval) {
		IPAERR("ipa3_send_msg failed: %d, msg_type %d\n",
			retval,
			msg_type);
		kfree(buff);
		return retval;
	}
	IPADBG("exit\n");

	return 0;
}

static void ipa3_gsb_msg_free_cb(void *buff, u32 len, u32 type)
{
	if (!buff) {
		IPAERR("Null buffer\n");
		return;
	}

	switch (type) {
	case IPA_GSB_CONNECT:
	case IPA_GSB_DISCONNECT:
		break;
	default:
		IPAERR("Wrong type given. buff %pK type %d\n", buff, type);
		return;
	}

	kfree(buff);
}

static void ipa3_get_usb_ep_info(
		struct ipa_ioc_get_ep_info *ep_info,
		struct ipa_ep_pair_info *pair_info
		)
{
	int ep_index = -1, i;
	int pair_id = 0;

	for (i = 0; i < ep_info->max_ep_pairs; i++) {
		pair_info[i].consumer_pipe_num = -1;
		pair_info[i].producer_pipe_num = -1;
		pair_info[i].ep_id = -1;
	}

	if ((!ep_info->teth_prot_valid) || (ep_info->teth_prot_valid &&
		ep_info->teth_prot == IPA_PROT_RMNET_CV2X)) {
		ep_index = ipa3_get_ep_mapping(IPA_CLIENT_USB2_PROD);

		if ((ep_index != -1) && ipa3_ctx->ep[ep_index].valid) {
			pair_info[pair_id].consumer_pipe_num = ep_index;
			ep_index = ipa3_get_ep_mapping(IPA_CLIENT_USB2_CONS);

			if ((ep_index != -1) &&
				(ipa3_ctx->ep[ep_index].valid)) {
				pair_info[pair_id].producer_pipe_num = ep_index;
				pair_info[pair_id].ep_id = IPA_USB1_EP_ID;

				IPADBG("ep_pair_info consumer_pipe_num %d",
					pair_info[pair_id].consumer_pipe_num);
				IPADBG(" producer_pipe_num %d ep_id %d\n",
					pair_info[pair_id].producer_pipe_num,
					pair_info[pair_id].ep_id);
				pair_id++;
			} else {
				pair_info[pair_id].consumer_pipe_num = -1;
				IPADBG("ep_pair_info consumer_pipe_num %d",
					pair_info[pair_id].consumer_pipe_num);
				IPADBG(" producer_pipe_num %d ep_id %d\n",
					pair_info[pair_id].producer_pipe_num,
					pair_info[pair_id].ep_id);
			}
		}
	}


	if ((!ep_info->teth_prot_valid) || (ep_info->teth_prot_valid &&
		ep_info->teth_prot == IPA_PROT_RMNET)) {
		ep_index = ipa3_get_ep_mapping(IPA_CLIENT_USB_PROD);

		if ((ep_index != -1) && ipa3_ctx->ep[ep_index].valid) {
			pair_info[pair_id].consumer_pipe_num = ep_index;
			ep_index = ipa3_get_ep_mapping(IPA_CLIENT_USB_CONS);

			if ((ep_index != -1) &&
				(ipa3_ctx->ep[ep_index].valid)) {
				pair_info[pair_id].producer_pipe_num = ep_index;
				pair_info[pair_id].ep_id = IPA_USB0_EP_ID;

				IPADBG("ep_pair_info consumer_pipe_num %d",
					pair_info[pair_id].consumer_pipe_num);
				IPADBG(" producer_pipe_num %d ep_id %d\n",
					pair_info[pair_id].producer_pipe_num,
					pair_info[pair_id].ep_id);
				pair_id++;
			} else {
				pair_info[pair_id].consumer_pipe_num = -1;
				IPADBG("ep_pair_info consumer_pipe_num %d",
					pair_info[pair_id].consumer_pipe_num);
				IPADBG(" producer_pipe_num %d ep_id %d\n",
					pair_info[pair_id].producer_pipe_num,
					pair_info[pair_id].ep_id);
			}
		}
	}
	ep_info->num_ep_pairs = pair_id;

}

static void ipa3_get_pcie_ep_info(
			struct ipa_ioc_get_ep_info *ep_info,
			struct ipa_ep_pair_info *pair_info
			)
{
	int ep_index = -1, i;

	ep_info->num_ep_pairs = 0;
	for (i = 0; i < ep_info->max_ep_pairs; i++) {
		pair_info[i].consumer_pipe_num = -1;
		pair_info[i].producer_pipe_num = -1;
		pair_info[i].ep_id = -1;
	}

	/*
	 * Legacy codes for ipa4.X version
	 */
	if (ipa3_ctx->ipa_hw_type < IPA_HW_v5_0) {
		ep_index = ipa3_get_ep_mapping(IPA_CLIENT_MHI2_PROD);

		if ((ep_index != -1) && ipa3_ctx->ep[ep_index].valid) {
			pair_info[ep_info->num_ep_pairs].consumer_pipe_num = ep_index;
			ep_index = ipa3_get_ep_mapping(IPA_CLIENT_MHI2_CONS);
			if ((ep_index != -1) && (ipa3_ctx->ep[ep_index].valid)) {
				pair_info[ep_info->num_ep_pairs].producer_pipe_num =
				ep_index;
				pair_info[ep_info->num_ep_pairs].ep_id =
				IPA_v4_PCIE1_EP_ID;

				IPADBG("ep_pair_info consumer_pipe_num %d",
					pair_info[ep_info->num_ep_pairs].consumer_pipe_num);
				IPADBG(" producer_pipe_num %d ep_id %d\n",
					pair_info[ep_info->num_ep_pairs].producer_pipe_num,
					pair_info[ep_info->num_ep_pairs].ep_id);
				ep_info->num_ep_pairs++;
			} else {
				pair_info[ep_info->num_ep_pairs].consumer_pipe_num = -1;
				IPADBG("ep_pair_info consumer_pipe_num %d",
					pair_info[ep_info->num_ep_pairs].consumer_pipe_num);
				IPADBG(" producer_pipe_num %d ep_id %d\n",
				pair_info[ep_info->num_ep_pairs].producer_pipe_num,
					pair_info[ep_info->num_ep_pairs].ep_id);
			}
		}
	}

	ep_index = ipa3_get_ep_mapping(IPA_CLIENT_MHI_PROD);

	if ((ep_index != -1) && ipa3_ctx->ep[ep_index].valid) {
		pair_info[ep_info->num_ep_pairs].consumer_pipe_num = ep_index;
		ep_index = ipa3_get_ep_mapping(IPA_CLIENT_MHI_CONS);
		if ((ep_index != -1) && (ipa3_ctx->ep[ep_index].valid)) {
			pair_info[ep_info->num_ep_pairs].producer_pipe_num =
				ep_index;
			if (ipa3_ctx->ipa_hw_type < IPA_HW_v5_0)
				pair_info[ep_info->num_ep_pairs].ep_id =
					IPA_v4_PCIE0_EP_ID;
			else
				pair_info[ep_info->num_ep_pairs].ep_id =
					IPA_v5_PCIE0_EP_ID;

			IPADBG("ep_pair_info consumer_pipe_num %d",
			pair_info[ep_info->num_ep_pairs].consumer_pipe_num);
			IPADBG(" producer_pipe_num %d ep_id %d\n",
			pair_info[ep_info->num_ep_pairs].producer_pipe_num,
				pair_info[ep_info->num_ep_pairs].ep_id);
			ep_info->num_ep_pairs++;
		} else {
			pair_info[ep_info->num_ep_pairs].consumer_pipe_num = -1;
			IPADBG("ep_pair_info consumer_pipe_num %d",
			pair_info[ep_info->num_ep_pairs].consumer_pipe_num);
			IPADBG(" producer_pipe_num %d ep_id %d\n",
			pair_info[ep_info->num_ep_pairs].producer_pipe_num,
				pair_info[ep_info->num_ep_pairs].ep_id);
		}
	}
}

static void ipa3_get_eth_ep_info(
	struct ipa_ioc_get_ep_info *ep_info,
	struct ipa_ep_pair_info *pair_info
	)
{
	int ep_index = -1, i;

	ep_info->num_ep_pairs = 0;
	for (i = 0; i < ep_info->max_ep_pairs; i++) {
		pair_info[i].consumer_pipe_num = -1;
		pair_info[i].producer_pipe_num = -1;
		pair_info[i].ep_id = -1;
	}

	ep_index = ipa3_get_ep_mapping(IPA_CLIENT_ETHERNET2_PROD);

	if ((ep_index != -1) && ipa3_ctx->ep[ep_index].valid) {
		pair_info[ep_info->num_ep_pairs].consumer_pipe_num = ep_index;
		ep_index = ipa3_get_ep_mapping(IPA_CLIENT_ETHERNET2_CONS);
		if ((ep_index != -1) && (ipa3_ctx->ep[ep_index].valid)) {
			pair_info[ep_info->num_ep_pairs].producer_pipe_num =
				ep_index;
			pair_info[ep_info->num_ep_pairs].ep_id =
				IPA_ETH1_EP_ID;
		IPADBG("ep_pair_info consumer_pipe_num %d",
			pair_info[ep_info->num_ep_pairs].consumer_pipe_num);
			IPADBG(" producer_pipe_num %d ep_id %d\n",
			pair_info[ep_info->num_ep_pairs].producer_pipe_num,
				pair_info[ep_info->num_ep_pairs].ep_id);
			ep_info->num_ep_pairs++;
		} else {
			pair_info[ep_info->num_ep_pairs].consumer_pipe_num = -1;
			IPADBG("ep_pair_info consumer_pipe_num %d",
			pair_info[ep_info->num_ep_pairs].consumer_pipe_num);
			IPADBG(" producer_pipe_num %d ep_id %d\n",
			pair_info[ep_info->num_ep_pairs].producer_pipe_num,
				pair_info[ep_info->num_ep_pairs].ep_id);
		}
	}

	ep_index = ipa3_get_ep_mapping(IPA_CLIENT_ETHERNET_PROD);

	if ((ep_index != -1) && ipa3_ctx->ep[ep_index].valid) {
		pair_info[ep_info->num_ep_pairs].consumer_pipe_num = ep_index;
		ep_index = ipa3_get_ep_mapping(IPA_CLIENT_ETHERNET_CONS);
		if ((ep_index != -1) && (ipa3_ctx->ep[ep_index].valid)) {
			pair_info[ep_info->num_ep_pairs].producer_pipe_num =
				ep_index;
			pair_info[ep_info->num_ep_pairs].ep_id =
				IPA_ETH0_EP_ID;

			IPADBG("ep_pair_info consumer_pipe_num %d",
			pair_info[ep_info->num_ep_pairs].consumer_pipe_num);
			IPADBG(" producer_pipe_num %d ep_id %d\n",
			pair_info[ep_info->num_ep_pairs].producer_pipe_num,
				pair_info[ep_info->num_ep_pairs].ep_id);
			ep_info->num_ep_pairs++;
		} else {
			pair_info[ep_info->num_ep_pairs].consumer_pipe_num = -1;
			IPADBG("ep_pair_info consumer_pipe_num %d",
			pair_info[ep_info->num_ep_pairs].consumer_pipe_num);
			IPADBG(" producer_pipe_num %d ep_id %d\n",
			pair_info[ep_info->num_ep_pairs].producer_pipe_num,
				pair_info[ep_info->num_ep_pairs].ep_id);
		}
	}
}

static int ipa3_get_ep_info(struct ipa_ioc_get_ep_info *ep_info,
							u8 *param)
{
	int ret = 0;
	struct ipa_ep_pair_info *pair_info = (struct ipa_ep_pair_info *)param;

	switch (ep_info->ep_type) {
	case IPA_DATA_EP_TYP_HSUSB:
		ipa3_get_usb_ep_info(ep_info, pair_info);
		break;

	case IPA_DATA_EP_TYP_PCIE:
		ipa3_get_pcie_ep_info(ep_info, pair_info);
		break;

	case IPA_DATA_EP_TYP_ETH:
		ipa3_get_eth_ep_info(ep_info, pair_info);
		break;

	default:
		IPAERR_RL("Undefined ep_type %d\n", ep_info->ep_type);
		ret = -EFAULT;
		break;
	}

	return ret;
}

static int ipa3_send_gsb_msg(unsigned long usr_param, uint8_t msg_type)
{
	int retval;
	struct ipa_ioc_gsb_info *gsb_info;
	struct ipa_msg_meta msg_meta;
	void *buff;

	IPADBG("type %d\n", msg_type);

	memset(&msg_meta, 0, sizeof(msg_meta));
	msg_meta.msg_type = msg_type;

	if ((msg_type == IPA_GSB_CONNECT) ||
		(msg_type == IPA_GSB_DISCONNECT)) {
		gsb_info = kzalloc(sizeof(struct ipa_ioc_gsb_info),
			GFP_KERNEL);
		if (!gsb_info) {
			IPAERR("no memory\n");
			return -ENOMEM;
		}

		if (copy_from_user((u8 *)gsb_info, (void __user *)usr_param,
			sizeof(struct ipa_ioc_gsb_info))) {
			kfree(gsb_info);
			return -EFAULT;
		}

		msg_meta.msg_len = sizeof(struct ipa_ioc_gsb_info);
		buff = gsb_info;
	} else {
		IPAERR("Unexpected event\n");
		return -EFAULT;
	}

	retval = ipa3_send_msg(&msg_meta, buff,
		ipa3_gsb_msg_free_cb);
	if (retval) {
		IPAERR("ipa3_send_msg failed: %d, msg_type %d\n",
			retval,
			msg_type);
		kfree(buff);
		return retval;
	}
	IPADBG("exit\n");

	return 0;
}

static int ipa3_ioctl_add_rt_rule_v2(unsigned long arg)
{
	int retval = 0;
	int i;
	u8 header[128] = { 0 };
	int pre_entry;
	u32 usr_pyld_sz;
	u32 pyld_sz;
	u64 uptr = 0;
	u8 *param = NULL;
	u8 *param2 = NULL;
	u8 *kptr = NULL;

	if (copy_from_user(header, (const void __user *)arg,
		sizeof(struct ipa_ioc_add_rt_rule_v2))) {
		IPAERR_RL("copy_from_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}
	pre_entry =
		((struct ipa_ioc_add_rt_rule_v2 *)header)->num_rules;
	if (unlikely(((struct ipa_ioc_add_rt_rule_v2 *)
		header)->rule_add_size >
		sizeof(struct ipa_rt_rule_add_i))) {
		IPAERR_RL("unexpected rule_add_size %d\n",
		((struct ipa_ioc_add_rt_rule_v2 *)
		header)->rule_add_size);
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* user payload size */
	usr_pyld_sz = ((struct ipa_ioc_add_rt_rule_v2 *)
		header)->rule_add_size * pre_entry;
	/* actual payload structure size in kernel */
	pyld_sz = sizeof(struct ipa_rt_rule_add_i) * pre_entry;
	uptr = ((struct ipa_ioc_add_rt_rule_v2 *)
		header)->rules;
	if (unlikely(!uptr)) {
		IPAERR_RL("unexpected NULL rules\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* alloc param with same payload size as user payload */
	param = memdup_user((const void __user *)uptr,
		usr_pyld_sz);
	if (IS_ERR(param)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}

	param2 = memdup_user((const void __user *)arg,
		sizeof(struct ipa_ioc_add_rt_rule_v2));
	if (IS_ERR(param2)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}


	/* add check in case user-space module compromised */
	if (unlikely(((struct ipa_ioc_add_rt_rule_v2 *)param2)->num_rules
		!= pre_entry)) {
		IPAERR_RL("current %d pre %d\n",
			((struct ipa_ioc_add_rt_rule_v2 *)param2)->
				num_rules, pre_entry);
			retval = -EFAULT;
			goto free_param_kptr;
	}
	/* alloc kernel pointer with actual payload size */
	kptr = kzalloc(pyld_sz, GFP_KERNEL);
	if (!kptr) {
		retval = -ENOMEM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy(kptr + i * sizeof(struct ipa_rt_rule_add_i),
			(void *)param + i *
			((struct ipa_ioc_add_rt_rule_v2 *)
			header)->rule_add_size,
			((struct ipa_ioc_add_rt_rule_v2 *)
			header)->rule_add_size);
	/* modify the rule pointer to the kernel pointer */
	((struct ipa_ioc_add_rt_rule_v2 *)header)->rules =
		(u64)kptr;
	if (ipa3_add_rt_rule_usr_v2(
		(struct ipa_ioc_add_rt_rule_v2 *)header, true)) {
		IPAERR_RL("ipa3_add_rt_rule_usr_v2 fails\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy((void *)param + i *
			((struct ipa_ioc_add_rt_rule_v2 *)
			header)->rule_add_size,
			kptr + i * sizeof(struct ipa_rt_rule_add_i),
			((struct ipa_ioc_add_rt_rule_v2 *)
			header)->rule_add_size);
	if (copy_to_user((void __user *)uptr, param,
		usr_pyld_sz)) {
		IPAERR_RL("copy_to_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}

free_param_kptr:
	if (!IS_ERR(param))
		kfree(param);
	if (!IS_ERR(param2))
		kfree(param2);
	kfree(kptr);

	return retval;
}

static int ipa3_ioctl_add_rt_rule_ext_v2(unsigned long arg)
{
	int retval = 0;
	int i;
	u8 header[128] = { 0 };
	int pre_entry;
	u32 usr_pyld_sz;
	u32 pyld_sz;
	u64 uptr = 0;
	u8 *param = NULL;
	u8 *param2 = NULL;
	u8 *kptr = NULL;

	if (copy_from_user(header,
			(const void __user *)arg,
			sizeof(struct ipa_ioc_add_rt_rule_ext_v2))) {
		IPAERR_RL("copy_from_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}
	pre_entry =
		((struct ipa_ioc_add_rt_rule_ext_v2 *)
		header)->num_rules;
	if (unlikely(((struct ipa_ioc_add_rt_rule_ext_v2 *)
		header)->rule_add_ext_size >
		sizeof(struct ipa_rt_rule_add_ext_i))) {
		IPAERR_RL("unexpected rule_add_size %d\n",
		((struct ipa_ioc_add_rt_rule_ext_v2 *)
		header)->rule_add_ext_size);
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* user payload size */
	usr_pyld_sz = ((struct ipa_ioc_add_rt_rule_ext_v2 *)
		header)->rule_add_ext_size * pre_entry;
	/* actual payload structure size in kernel */
	pyld_sz = sizeof(struct ipa_rt_rule_add_ext_i)
		* pre_entry;
	uptr = ((struct ipa_ioc_add_rt_rule_ext_v2 *)
		header)->rules;
	if (unlikely(!uptr)) {
		IPAERR_RL("unexpected NULL rules\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* alloc param with same payload size as user payload */
	param = memdup_user((const void __user *)uptr,
		usr_pyld_sz);
	if (IS_ERR(param)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}

	param2 = memdup_user((const void __user *)arg,
		sizeof(struct ipa_ioc_add_rt_rule_ext_v2));
	if (IS_ERR(param2)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}


	/* add check in case user-space module compromised */
	if (unlikely(((struct ipa_ioc_add_rt_rule_ext_v2 *)param2)->num_rules
		!= pre_entry)) {
		IPAERR_RL("current %d pre %d\n",
			((struct ipa_ioc_add_rt_rule_ext_v2 *)param2)->
				num_rules, pre_entry);
			retval = -EFAULT;
			goto free_param_kptr;
	}
	/* alloc kernel pointer with actual payload size */
	kptr = kzalloc(pyld_sz, GFP_KERNEL);
	if (!kptr) {
		retval = -ENOMEM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy(kptr + i *
			sizeof(struct ipa_rt_rule_add_ext_i),
			(void *)param + i *
			((struct ipa_ioc_add_rt_rule_ext_v2 *)
			header)->rule_add_ext_size,
			((struct ipa_ioc_add_rt_rule_ext_v2 *)
			header)->rule_add_ext_size);
	/* modify the rule pointer to the kernel pointer */
	((struct ipa_ioc_add_rt_rule_ext_v2 *)header)->rules =
		(u64)kptr;
	if (ipa3_add_rt_rule_ext_v2(
		(struct ipa_ioc_add_rt_rule_ext_v2 *)header, true)) {
		IPAERR_RL("ipa3_add_rt_rule_ext_v2 fails\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy((void *)param + i *
			((struct ipa_ioc_add_rt_rule_ext_v2 *)
			header)->rule_add_ext_size,
			kptr + i *
			sizeof(struct ipa_rt_rule_add_ext_i),
			((struct ipa_ioc_add_rt_rule_ext_v2 *)
			header)->rule_add_ext_size);
	if (copy_to_user((void __user *)uptr, param,
		usr_pyld_sz)) {
		IPAERR_RL("copy_to_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}

free_param_kptr:
	if (!IS_ERR(param))
		kfree(param);
	if (!IS_ERR(param2))
		kfree(param2);
	kfree(kptr);

	return retval;
}

static int ipa3_ioctl_add_rt_rule_after_v2(unsigned long arg)
{
	int retval = 0;
	int i;
	u8 header[128] = { 0 };
	int pre_entry;
	u32 usr_pyld_sz;
	u32 pyld_sz;
	u64 uptr = 0;
	u8 *param = NULL;
	u8 *param2 = NULL;
	u8 *kptr = NULL;

	if (copy_from_user(header, (const void __user *)arg,
		sizeof(struct ipa_ioc_add_rt_rule_after_v2))) {
		IPAERR_RL("copy_from_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}
	pre_entry =
		((struct ipa_ioc_add_rt_rule_after_v2 *)
		header)->num_rules;
	if (unlikely(((struct ipa_ioc_add_rt_rule_after_v2 *)
		header)->rule_add_size >
		sizeof(struct ipa_rt_rule_add_i))) {
		IPAERR_RL("unexpected rule_add_size %d\n",
		((struct ipa_ioc_add_rt_rule_after_v2 *)
		header)->rule_add_size);
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* user payload size */
	usr_pyld_sz = ((struct ipa_ioc_add_rt_rule_after_v2 *)
		header)->rule_add_size * pre_entry;
	/* actual payload structure size in kernel */
	pyld_sz = sizeof(struct ipa_rt_rule_add_i)
		* pre_entry;
	uptr = ((struct ipa_ioc_add_rt_rule_after_v2 *)
		header)->rules;
	if (unlikely(!uptr)) {
		IPAERR_RL("unexpected NULL rules\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* alloc param with same payload size as user payload */
	param = memdup_user((const void __user *)uptr,
		usr_pyld_sz);
	if (IS_ERR(param)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}

	param2 = memdup_user((const void __user *)arg,
		sizeof(struct ipa_ioc_add_rt_rule_after_v2));
	if (IS_ERR(param2)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}

	/* add check in case user-space module compromised */
	if (unlikely(((struct ipa_ioc_add_rt_rule_after_v2 *)param2)->num_rules
		!= pre_entry)) {
		IPAERR_RL("current %d pre %d\n",
			((struct ipa_ioc_add_rt_rule_after_v2 *)param2)->
				num_rules, pre_entry);
			retval = -EFAULT;
			goto free_param_kptr;
	}
	/* alloc kernel pointer with actual payload size */
	kptr = kzalloc(pyld_sz, GFP_KERNEL);
	if (!kptr) {
		retval = -ENOMEM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy(kptr + i * sizeof(struct ipa_rt_rule_add_i),
			(void *)param + i *
			((struct ipa_ioc_add_rt_rule_after_v2 *)
			header)->rule_add_size,
			((struct ipa_ioc_add_rt_rule_after_v2 *)
			header)->rule_add_size);
	/* modify the rule pointer to the kernel pointer */
	((struct ipa_ioc_add_rt_rule_after_v2 *)header)->rules =
		(u64)kptr;
	if (ipa3_add_rt_rule_after_v2(
		(struct ipa_ioc_add_rt_rule_after_v2 *)header)) {
		IPAERR_RL("ipa3_add_rt_rule_after_v2 fails\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy((void *)param + i *
			((struct ipa_ioc_add_rt_rule_after_v2 *)
			header)->rule_add_size,
			kptr + i * sizeof(struct ipa_rt_rule_add_i),
			((struct ipa_ioc_add_rt_rule_after_v2 *)
			header)->rule_add_size);
	if (copy_to_user((void __user *)uptr, param,
		usr_pyld_sz)) {
		IPAERR_RL("copy_to_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}

free_param_kptr:
	if (!IS_ERR(param))
		kfree(param);
	if (!IS_ERR(param2))
		kfree(param2);
	kfree(kptr);

	return retval;
}

static int ipa3_ioctl_mdfy_rt_rule_v2(unsigned long arg)
{
	int retval = 0;
	int i;
	u8 header[128] = { 0 };
	int pre_entry;
	u32 usr_pyld_sz;
	u32 pyld_sz;
	u64 uptr = 0;
	u8 *param = NULL;
	u8 *param2 = NULL;
	u8 *kptr = NULL;

	if (copy_from_user(header, (const void __user *)arg,
		sizeof(struct ipa_ioc_mdfy_rt_rule_v2))) {
		IPAERR_RL("copy_from_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}
	pre_entry =
		((struct ipa_ioc_mdfy_rt_rule_v2 *)
		header)->num_rules;
	if (unlikely(((struct ipa_ioc_mdfy_rt_rule_v2 *)
		header)->rule_mdfy_size >
		sizeof(struct ipa_rt_rule_mdfy_i))) {
		IPAERR_RL("unexpected rule_add_size %d\n",
		((struct ipa_ioc_mdfy_rt_rule_v2 *)
		header)->rule_mdfy_size);
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* user payload size */
	usr_pyld_sz = ((struct ipa_ioc_mdfy_rt_rule_v2 *)
		header)->rule_mdfy_size * pre_entry;
	/* actual payload structure size in kernel */
	pyld_sz = sizeof(struct ipa_rt_rule_mdfy_i)
		* pre_entry;
	uptr = ((struct ipa_ioc_mdfy_rt_rule_v2 *)
		header)->rules;
	if (unlikely(!uptr)) {
		IPAERR_RL("unexpected NULL rules\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* alloc param with same payload size as user payload */
	param = memdup_user((const void __user *)uptr,
		usr_pyld_sz);
	if (IS_ERR(param)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}

	param2 = memdup_user((const void __user *)arg,
		sizeof(struct ipa_ioc_mdfy_rt_rule_v2));
	if (IS_ERR(param2)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}

	/* add check in case user-space module compromised */
	if (unlikely(((struct ipa_ioc_mdfy_rt_rule_v2 *)param2)->num_rules
		!= pre_entry)) {
		IPAERR_RL("current %d pre %d\n",
			((struct ipa_ioc_mdfy_rt_rule_v2 *)param2)->
				num_rules, pre_entry);
			retval = -EFAULT;
			goto free_param_kptr;
	}
	/* alloc kernel pointer with actual payload size */
	kptr = kzalloc(pyld_sz, GFP_KERNEL);
	if (!kptr) {
		retval = -ENOMEM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy(kptr + i * sizeof(struct ipa_rt_rule_mdfy_i),
			(void *)param + i *
			((struct ipa_ioc_mdfy_rt_rule_v2 *)
			header)->rule_mdfy_size,
			((struct ipa_ioc_mdfy_rt_rule_v2 *)
			header)->rule_mdfy_size);
	/* modify the rule pointer to the kernel pointer */
	((struct ipa_ioc_mdfy_rt_rule_v2 *)header)->rules =
		(u64)kptr;
	if (ipa3_mdfy_rt_rule_v2((struct ipa_ioc_mdfy_rt_rule_v2 *)
		header)) {
		IPAERR_RL("ipa3_mdfy_rt_rule_v2 fails\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy((void *)param + i *
			((struct ipa_ioc_mdfy_rt_rule_v2 *)
			header)->rule_mdfy_size,
			kptr + i * sizeof(struct ipa_rt_rule_mdfy_i),
			((struct ipa_ioc_mdfy_rt_rule_v2 *)
			header)->rule_mdfy_size);
	if (copy_to_user((void __user *)uptr, param,
		usr_pyld_sz)) {
		IPAERR_RL("copy_to_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}

free_param_kptr:
	if (!IS_ERR(param))
		kfree(param);
	if (!IS_ERR(param2))
		kfree(param2);
	kfree(kptr);

	return retval;
}

static int ipa3_ioctl_add_flt_rule_v2(unsigned long arg)
{
	int retval = 0;
	int i;
	u8 header[128] = { 0 };
	int pre_entry;
	u32 usr_pyld_sz;
	u32 pyld_sz;
	u64 uptr = 0;
	u8 *param = NULL;
	u8 *param2 = NULL;
	u8 *kptr = NULL;

	if (copy_from_user(header, (const void __user *)arg,
		sizeof(struct ipa_ioc_add_flt_rule_v2))) {
		IPAERR_RL("copy_from_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}
	pre_entry =
		((struct ipa_ioc_add_flt_rule_v2 *)header)->num_rules;
	if (unlikely(((struct ipa_ioc_add_flt_rule_v2 *)
		header)->flt_rule_size >
		sizeof(struct ipa_flt_rule_add_i))) {
		IPAERR_RL("unexpected rule_add_size %d\n",
		((struct ipa_ioc_add_flt_rule_v2 *)
		header)->flt_rule_size);
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* user payload size */
	usr_pyld_sz = ((struct ipa_ioc_add_flt_rule_v2 *)
		header)->flt_rule_size * pre_entry;
	/* actual payload structure size in kernel */
	pyld_sz = sizeof(struct ipa_flt_rule_add_i)
		* pre_entry;
	uptr = ((struct ipa_ioc_add_flt_rule_v2 *)
		header)->rules;
	if (unlikely(!uptr)) {
		IPAERR_RL("unexpected NULL rules\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* alloc param with same payload size as user payload */
	param = memdup_user((const void __user *)uptr,
		usr_pyld_sz);
	if (IS_ERR(param)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}

	param2 = memdup_user((const void __user *)arg,
		sizeof(struct ipa_ioc_add_flt_rule_v2));
	if (IS_ERR(param2)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}

	/* add check in case user-space module compromised */
	if (unlikely(((struct ipa_ioc_add_flt_rule_v2 *)param2)->num_rules
		!= pre_entry)) {
		IPAERR_RL("current %d pre %d\n",
			((struct ipa_ioc_add_flt_rule_v2 *)param2)->
				num_rules, pre_entry);
			retval = -EFAULT;
			goto free_param_kptr;
	}
	/* alloc kernel pointer with actual payload size */
	kptr = kzalloc(pyld_sz, GFP_KERNEL);
	if (!kptr) {
		retval = -ENOMEM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy(kptr + i * sizeof(struct ipa_flt_rule_add_i),
			(void *)param + i *
			((struct ipa_ioc_add_flt_rule_v2 *)
			header)->flt_rule_size,
			((struct ipa_ioc_add_flt_rule_v2 *)
			header)->flt_rule_size);
	/* modify the rule pointer to the kernel pointer */
	((struct ipa_ioc_add_flt_rule_v2 *)header)->rules =
		(u64)kptr;
	if (ipa3_add_flt_rule_usr_v2((struct ipa_ioc_add_flt_rule_v2 *)
			header, true)) {
		IPAERR_RL("ipa3_add_flt_rule_usr_v2 fails\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy((void *)param + i *
			((struct ipa_ioc_add_flt_rule_v2 *)
			header)->flt_rule_size,
			kptr + i * sizeof(struct ipa_flt_rule_add_i),
			((struct ipa_ioc_add_flt_rule_v2 *)
			header)->flt_rule_size);
	if (copy_to_user((void __user *)uptr, param,
		usr_pyld_sz)) {
		IPAERR_RL("copy_to_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}
free_param_kptr:
	if (!IS_ERR(param))
		kfree(param);
	if (!IS_ERR(param2))
		kfree(param2);
	kfree(kptr);

	return retval;
}

static int ipa3_ioctl_add_flt_rule_after_v2(unsigned long arg)
{
	int retval = 0;
	int i;
	u8 header[128] = { 0 };
	int pre_entry;
	u32 usr_pyld_sz;
	u32 pyld_sz;
	u64 uptr = 0;
	u8 *param = NULL;
	u8 *param2 = NULL;
	u8 *kptr = NULL;

	if (copy_from_user(header, (const void __user *)arg,
		sizeof(struct ipa_ioc_add_flt_rule_after_v2))) {
		IPAERR_RL("copy_from_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}
	pre_entry =
		((struct ipa_ioc_add_flt_rule_after_v2 *)
		 header)->num_rules;
	if (unlikely(((struct ipa_ioc_add_flt_rule_after_v2 *)
		header)->flt_rule_size >
		sizeof(struct ipa_flt_rule_add_i))) {
		IPAERR_RL("unexpected rule_add_size %d\n",
		((struct ipa_ioc_add_flt_rule_after_v2 *)
		header)->flt_rule_size);
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* user payload size */
	usr_pyld_sz = ((struct ipa_ioc_add_flt_rule_after_v2 *)
		header)->flt_rule_size * pre_entry;
	/* actual payload structure size in kernel */
	pyld_sz = sizeof(struct ipa_flt_rule_add_i)
		* pre_entry;
	uptr = ((struct ipa_ioc_add_flt_rule_after_v2 *)
		header)->rules;
	if (unlikely(!uptr)) {
		IPAERR_RL("unexpected NULL rules\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* alloc param with same payload size as user payload */
	param = memdup_user((const void __user *)uptr,
		usr_pyld_sz);
	if (IS_ERR(param)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}

	param2 = memdup_user((const void __user *)arg,
		sizeof(struct ipa_ioc_add_flt_rule_after_v2));
	if (IS_ERR(param2)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}

	/* add check in case user-space module compromised */
	if (unlikely(((struct ipa_ioc_add_flt_rule_after_v2 *)param2)->num_rules
		!= pre_entry)) {
		IPAERR_RL("current %d pre %d\n",
			((struct ipa_ioc_add_flt_rule_after_v2 *)param2)->
				num_rules, pre_entry);
			retval = -EFAULT;
			goto free_param_kptr;
	}
	/* alloc kernel pointer with actual payload size */
	kptr = kzalloc(pyld_sz, GFP_KERNEL);
	if (!kptr) {
		retval = -ENOMEM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy(kptr + i * sizeof(struct ipa_flt_rule_add_i),
			(void *)param + i *
			((struct ipa_ioc_add_flt_rule_after_v2 *)
			header)->flt_rule_size,
			((struct ipa_ioc_add_flt_rule_after_v2 *)
			header)->flt_rule_size);
	/* modify the rule pointer to the kernel pointer */
	((struct ipa_ioc_add_flt_rule_after_v2 *)header)->rules =
		(u64)kptr;
	if (ipa3_add_flt_rule_after_v2(
		(struct ipa_ioc_add_flt_rule_after_v2 *)header)) {
		IPAERR_RL("ipa3_add_flt_rule_after_v2 fails\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy((void *)param + i *
			((struct ipa_ioc_add_flt_rule_after_v2 *)
			header)->flt_rule_size,
			kptr + i * sizeof(struct ipa_flt_rule_add_i),
			((struct ipa_ioc_add_flt_rule_after_v2 *)
			header)->flt_rule_size);
	if (copy_to_user((void __user *)uptr, param,
		usr_pyld_sz)) {
		IPAERR_RL("copy_to_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}

free_param_kptr:
	if (!IS_ERR(param))
		kfree(param);
	if (!IS_ERR(param2))
		kfree(param2);
	kfree(kptr);

	return retval;
}

static int ipa3_ioctl_mdfy_flt_rule_v2(unsigned long arg)
{
	int retval = 0;
	int i;
	u8 header[128] = { 0 };
	int pre_entry;
	u32 usr_pyld_sz;
	u32 pyld_sz;
	u64 uptr = 0;
	u8 *param = NULL;
	u8 *param2 = NULL;
	u8 *kptr = NULL;

	if (copy_from_user(header, (const void __user *)arg,
		sizeof(struct ipa_ioc_mdfy_flt_rule_v2))) {
		IPAERR_RL("copy_from_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}
	pre_entry =
		((struct ipa_ioc_mdfy_flt_rule_v2 *)
		 header)->num_rules;
	if (unlikely(((struct ipa_ioc_mdfy_flt_rule_v2 *)
		header)->rule_mdfy_size >
		sizeof(struct ipa_flt_rule_mdfy_i))) {
		IPAERR_RL("unexpected rule_add_size %d\n",
		((struct ipa_ioc_mdfy_flt_rule_v2 *)
		header)->rule_mdfy_size);
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* user payload size */
	usr_pyld_sz = ((struct ipa_ioc_mdfy_flt_rule_v2 *)
		header)->rule_mdfy_size * pre_entry;
	/* actual payload structure size in kernel */
	pyld_sz = sizeof(struct ipa_flt_rule_mdfy_i)
		* pre_entry;
	uptr = ((struct ipa_ioc_mdfy_flt_rule_v2 *)
		header)->rules;
	if (unlikely(!uptr)) {
		IPAERR_RL("unexpected NULL rules\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* alloc param with same payload size as user payload */
	param = memdup_user((const void __user *)uptr,
		usr_pyld_sz);
	if (IS_ERR(param)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}

	param2 = memdup_user((const void __user *)arg,
		sizeof(struct ipa_ioc_mdfy_flt_rule_v2));
	if (IS_ERR(param2)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}

	/* add check in case user-space module compromised */
	if (unlikely(((struct ipa_ioc_mdfy_flt_rule_v2 *)param2)->num_rules
		!= pre_entry)) {
		IPAERR_RL("current %d pre %d\n",
			((struct ipa_ioc_mdfy_flt_rule_v2 *)param2)->
				num_rules, pre_entry);
			retval = -EFAULT;
			goto free_param_kptr;
	}
	/* alloc kernel pointer with actual payload size */
	kptr = kzalloc(pyld_sz, GFP_KERNEL);
	if (!kptr) {
		retval = -ENOMEM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy(kptr + i * sizeof(struct ipa_flt_rule_mdfy_i),
			(void *)param + i *
			((struct ipa_ioc_mdfy_flt_rule_v2 *)
			header)->rule_mdfy_size,
			((struct ipa_ioc_mdfy_flt_rule_v2 *)
			header)->rule_mdfy_size);
	/* modify the rule pointer to the kernel pointer */
	((struct ipa_ioc_mdfy_flt_rule_v2 *)header)->rules =
		(u64)kptr;
	if (ipa3_mdfy_flt_rule_v2
		((struct ipa_ioc_mdfy_flt_rule_v2 *)header)) {
		IPAERR_RL("ipa3_mdfy_flt_rule_v2 fails\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy((void *)param + i *
			((struct ipa_ioc_mdfy_flt_rule_v2 *)
			header)->rule_mdfy_size,
			kptr + i * sizeof(struct ipa_flt_rule_mdfy_i),
			((struct ipa_ioc_mdfy_flt_rule_v2 *)
			header)->rule_mdfy_size);
	if (copy_to_user((void __user *)uptr, param,
		usr_pyld_sz)) {
		IPAERR_RL("copy_to_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}

free_param_kptr:
	if (!IS_ERR(param))
		kfree(param);
	if (!IS_ERR(param2))
		kfree(param2);
	kfree(kptr);

	return retval;
}

static int ipa3_ioctl_fnr_counter_alloc(unsigned long arg)
{
	int retval = 0;
	u8 header[128] = { 0 };

	if (copy_from_user(header, (const void __user *)arg,
		sizeof(struct ipa_ioc_flt_rt_counter_alloc))) {
		IPAERR("copy_from_user fails\n");
		return -EFAULT;
	}
	if (((struct ipa_ioc_flt_rt_counter_alloc *)
		header)->hw_counter.num_counters >
		IPA_FLT_RT_HW_COUNTER ||
		((struct ipa_ioc_flt_rt_counter_alloc *)
		header)->sw_counter.num_counters >
		IPA_FLT_RT_SW_COUNTER) {
		IPAERR("failed: wrong sw/hw num_counters\n");
		return -EPERM;
	}
	if (((struct ipa_ioc_flt_rt_counter_alloc *)
		header)->hw_counter.num_counters == 0 &&
		((struct ipa_ioc_flt_rt_counter_alloc *)
		header)->sw_counter.num_counters == 0) {
		IPAERR("failed: both sw/hw num_counters 0\n");
		return -EPERM;
	}
	retval = ipa3_alloc_counter_id
		((struct ipa_ioc_flt_rt_counter_alloc *)header);
	if (retval < 0) {
		IPAERR("ipa3_alloc_counter_id failed\n");
		return retval;
	}
	if (copy_to_user((void __user *)arg, header,
		sizeof(struct ipa_ioc_flt_rt_counter_alloc))) {
		IPAERR("copy_to_user fails\n");
		ipa3_counter_remove_hdl(
		((struct ipa_ioc_flt_rt_counter_alloc *)
		header)->hdl);
		return -EFAULT;
	}
	return 0;
}

static int ipa3_ioctl_fnr_counter_query(unsigned long arg)
{
	int retval = 0;
	int i;
	u8 header[128] = { 0 };
	int pre_entry;
	u32 usr_pyld_sz;
	u32 pyld_sz;
	u64 uptr = 0;
	u8 *param = NULL;
	u8 *kptr = NULL;

	if (copy_from_user(header, (const void __user *)arg,
		sizeof(struct ipa_ioc_flt_rt_query))) {
		IPAERR_RL("copy_from_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}
	pre_entry =
		((struct ipa_ioc_flt_rt_query *)
		header)->end_id - ((struct ipa_ioc_flt_rt_query *)
		header)->start_id + 1;
	if (pre_entry <= 0 || pre_entry > IPA_MAX_FLT_RT_CNT_INDEX) {
		IPAERR("IPA_IOC_FNR_COUNTER_QUERY failed: num %d\n",
			pre_entry);
		retval = -EPERM;
		goto free_param_kptr;
	}
	if (((struct ipa_ioc_flt_rt_query *)header)->stats_size
		> sizeof(struct ipa_flt_rt_stats)) {
		IPAERR_RL("unexpected stats_size %d\n",
		((struct ipa_ioc_flt_rt_query *)header)->stats_size);
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* user payload size */
	usr_pyld_sz = ((struct ipa_ioc_flt_rt_query *)
		header)->stats_size * pre_entry;
	/* actual payload structure size in kernel */
	pyld_sz = sizeof(struct ipa_flt_rt_stats) * pre_entry;
	uptr = ((struct ipa_ioc_flt_rt_query *)
		header)->stats;
	if (unlikely(!uptr)) {
		IPAERR_RL("unexpected NULL rules\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	/* alloc param with same payload size as user payload */
	param = memdup_user((const void __user *)uptr,
		usr_pyld_sz);
	if (IS_ERR(param)) {
		retval = -EFAULT;
		goto free_param_kptr;
	}
	/* alloc kernel pointer with actual payload size */
	kptr = kzalloc(pyld_sz, GFP_KERNEL);
	if (!kptr) {
		retval = -ENOMEM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy(kptr + i * sizeof(struct ipa_flt_rt_stats),
			(void *)param + i *
			((struct ipa_ioc_flt_rt_query *)
			header)->stats_size,
			((struct ipa_ioc_flt_rt_query *)
			header)->stats_size);
	/* modify the rule pointer to the kernel pointer */
	((struct ipa_ioc_flt_rt_query *)
		header)->stats = (u64)kptr;
	retval = ipa_get_flt_rt_stats
		((struct ipa_ioc_flt_rt_query *)header);
	if (retval < 0) {
		IPAERR("ipa_get_flt_rt_stats failed\n");
		retval = -EPERM;
		goto free_param_kptr;
	}
	for (i = 0; i < pre_entry; i++)
		memcpy((void *)param + i *
			((struct ipa_ioc_flt_rt_query *)
			header)->stats_size,
			kptr + i * sizeof(struct ipa_flt_rt_stats),
			((struct ipa_ioc_flt_rt_query *)
			header)->stats_size);
	if (copy_to_user((void __user *)uptr, param,
		usr_pyld_sz)) {
		IPAERR_RL("copy_to_user fails\n");
		retval = -EFAULT;
		goto free_param_kptr;
	}

free_param_kptr:
	if (!IS_ERR(param))
		kfree(param);
	kfree(kptr);

	return retval;
}

static int ipa3_ioctl_fnr_counter_set(unsigned long arg)
{
	u8 header[128] = { 0 };
	uint8_t value;

	if (copy_from_user(header, (const void __user *)arg,
		sizeof(struct ipa_ioc_fnr_index_info))) {
		IPAERR_RL("copy_from_user fails\n");
		return -EFAULT;
	}

	value = ((struct ipa_ioc_fnr_index_info *)
		header)->hw_counter_offset;
	if (value <= 0 || value > IPA_MAX_FLT_RT_CNT_INDEX) {
		IPAERR("hw_counter_offset failed: num %d\n",
			value);
		return -EPERM;
	}

	ipa3_ctx->fnr_info.hw_counter_offset = value;

	value = ((struct ipa_ioc_fnr_index_info *)
		header)->sw_counter_offset;
	if (value <= 0 || value > IPA_MAX_FLT_RT_CNT_INDEX) {
		IPAERR("sw_counter_offset failed: num %d\n",
			value);
		return -EPERM;
	}
	ipa3_ctx->fnr_info.sw_counter_offset = value;
	/* reset when ipacm-cleanup */
	ipa3_ctx->fnr_info.valid = true;
	IPADBG("fnr_info hw=%d, hw=%d\n",
		ipa3_ctx->fnr_info.hw_counter_offset,
		ipa3_ctx->fnr_info.sw_counter_offset);
	return 0;
}

static int proc_sram_info_rqst(
	unsigned long arg)
{
	struct ipa_nat_in_sram_info sram_info = { 0 };

	if (ipa3_nat_get_sram_info(&sram_info))
		return  -EFAULT;

	if (copy_to_user(
		(void __user *) arg,
		&sram_info,
		sizeof(struct ipa_nat_in_sram_info)))
		return -EFAULT;

	return 0;
}

static void ipa3_general_free_cb(void *buff, u32 len, u32 type)
{
	if (!buff) {
		IPAERR("Null buffer\n");
		return;
	}
	kfree(buff);
}

static int ipa3_send_mac_flt_list(unsigned long usr_param)
{
	int retval;
	struct ipa_msg_meta msg_meta;
	void *buff;

	buff = kzalloc(sizeof(struct ipa_ioc_mac_client_list_type),
				GFP_KERNEL);
	if (!buff)
		return -ENOMEM;

	if (copy_from_user(buff, (const void __user *)usr_param,
		sizeof(struct ipa_ioc_mac_client_list_type))) {
		kfree(buff);
		return -EFAULT;
	}
	memset(&msg_meta, 0, sizeof(struct ipa_msg_meta));
	msg_meta.msg_type = IPA_MAC_FLT_EVENT;
	msg_meta.msg_len = sizeof(struct ipa_ioc_mac_client_list_type);

	IPADBG("No of clients: %d, flt state: %d\n",
		((struct ipa_ioc_mac_client_list_type *)buff)->num_of_clients,
		((struct ipa_ioc_mac_client_list_type *)buff)->flt_state);

	retval = ipa3_send_msg(&msg_meta, buff,
		ipa3_general_free_cb);
	if (retval) {
		IPAERR("ipa3_send_msg failed: %d, msg_type %d\n",
		retval,
		msg_meta.msg_type);
		kfree(buff);
		return retval;
	}
	return 0;
}

static int ipa3_send_pkt_threshold(unsigned long usr_param)
{
	int retval;
	struct ipa_msg_meta msg_meta;
	void *buff1, *buff2;

	buff1 = kzalloc(sizeof(struct ipa_ioc_set_pkt_threshold),
		GFP_KERNEL);
	if (!buff1)
		return -ENOMEM;

	if (copy_from_user(buff1, (const void __user *)usr_param,
		sizeof(struct ipa_ioc_set_pkt_threshold))) {
		kfree(buff1);
		return -EFAULT;
	}

	if (((struct ipa_ioc_set_pkt_threshold *)buff1)->ioctl_data_size !=
		sizeof(struct ipa_set_pkt_threshold)) {
		IPAERR("IPA_IOC_SET_PKT_THRESHOLD size not match(%d,%d)!\n",
		((struct ipa_ioc_set_pkt_threshold *)buff1)->ioctl_data_size,
		sizeof(struct ipa_set_pkt_threshold));
		kfree(buff1);
		return -EFAULT;
	}

	buff2 = kzalloc(sizeof(struct ipa_set_pkt_threshold),
		GFP_KERNEL);
	if (!buff2) {
		IPAERR("ipa_set_pkt_threshold buff2 allocate failure\n");
		kfree(buff1);
		return -ENOMEM;
	}

	if (copy_from_user(buff2, u64_to_user_ptr(
		((struct ipa_ioc_set_pkt_threshold *)buff1)->ioctl_ptr),
		sizeof(struct ipa_set_pkt_threshold))) {
		IPAERR("Failed to copy ipa_set_pkt_threshold\n");
		kfree(buff1);
		kfree(buff2);
		return -EFAULT;
	}


	memset(&msg_meta, 0, sizeof(struct ipa_msg_meta));
	msg_meta.msg_type = IPA_PKT_THRESHOLD_EVENT;
	msg_meta.msg_len = sizeof(struct ipa_set_pkt_threshold);

	IPADBG("pkt thr enable: %d, pkt_threshold: %d\n",
		((struct ipa_set_pkt_threshold *)buff2)->pkt_threshold_enable,
		((struct ipa_set_pkt_threshold *)buff2)->pkt_threshold);

	retval = ipa3_send_msg(&msg_meta, buff2,
		ipa3_general_free_cb);
	if (retval) {
		IPAERR("ipa3_send_msg failed: %d, msg_type %d\n",
		retval,
		msg_meta.msg_type);
		kfree(buff1);
		kfree(buff2);
		return retval;
	}
	return 0;
}

static int ipa3_send_sw_flt_list(unsigned long usr_param)
{
	int retval;
	struct ipa_msg_meta msg_meta;
	struct ipa_ioc_sw_flt_list_type sw_flt_list;
	void *buff;

	if (copy_from_user(&sw_flt_list, (const void __user *)usr_param,
		sizeof(struct ipa_ioc_sw_flt_list_type))) {
		IPAERR("Copy ipa_ioc_sw_flt_list_type failure\n");
		return -EFAULT;
	}

	if (sw_flt_list.ioctl_data_size !=
		sizeof(struct ipa_sw_flt_list_type)) {
		IPAERR("IPA_IOC_SET_SW_FLT size not match(%d,%d)!\n",
		sw_flt_list.ioctl_data_size,
		sizeof(struct ipa_sw_flt_list_type));
		return -EFAULT;
	}

	buff = kzalloc(sizeof(struct ipa_sw_flt_list_type),
				GFP_KERNEL);
	if (!buff) {
		IPAERR("ipa_sw_flt_list_type mem-allocate failure\n");
		return -ENOMEM;
	}

	if (copy_from_user(buff, u64_to_user_ptr(sw_flt_list.ioctl_ptr),
		sizeof(struct ipa_sw_flt_list_type))) {
		IPAERR("Failed to copy ipa_sw_flt_list_type\n");
		kfree(buff);
		return -EFAULT;
	}
	memset(&msg_meta, 0, sizeof(struct ipa_msg_meta));
	msg_meta.msg_type = IPA_SW_FLT_EVENT;
	msg_meta.msg_len = sizeof(struct ipa_sw_flt_list_type);

	IPADBG("No of clients: %d, mac-flt enable: %d\n",
		((struct ipa_sw_flt_list_type *)buff)->num_of_mac,
		((struct ipa_sw_flt_list_type *)buff)->mac_enable);

	IPADBG("No of segs: %d, ipv4-seg-flt enable: %d v6-offload %d\n",
		((struct ipa_sw_flt_list_type *)buff)->num_of_ipv4_segs,
		((struct ipa_sw_flt_list_type *)buff)->ipv4_segs_enable,
		((struct ipa_sw_flt_list_type *)buff)->ipv4_segs_ipv6_offload);

	IPADBG("No of ifaces: %d, iface-flt enable: %d\n",
		((struct ipa_sw_flt_list_type *)buff)->num_of_iface,
		((struct ipa_sw_flt_list_type *)buff)->iface_enable);

	retval = ipa3_send_msg(&msg_meta, buff,
		ipa3_general_free_cb);
	if (retval) {
		IPAERR("ipa3_send_msg failed: %d, msg_type %d\n",
		retval,
		msg_meta.msg_type);
		kfree(buff);
		return retval;
	}
	return 0;
}

static int ipa3_send_ippt_sw_flt_list(unsigned long usr_param)
{
	int retval;
	struct ipa_msg_meta msg_meta;
	struct ipa_ioc_sw_flt_list_type sw_flt_list;
	void *buff;

	if (copy_from_user(&sw_flt_list, (const void __user *)usr_param,
		sizeof(struct ipa_ioc_sw_flt_list_type))) {
		IPAERR("Copy ipa_ioc_sw_flt_list_type failure\n");
		return -EFAULT;
	}

	/* Expect ipa_ippt_sw_flt_list_type struct*/
	if (sw_flt_list.ioctl_data_size !=
		sizeof(struct ipa_ippt_sw_flt_list_type)) {
		IPAERR("IPA_IOC_SET_IPPT_SW_FLT size not match(%d,%d)!\n",
		sw_flt_list.ioctl_data_size,
		sizeof(struct ipa_ippt_sw_flt_list_type));
		return -EFAULT;
	}

	buff = kzalloc(sizeof(struct ipa_ippt_sw_flt_list_type),
				GFP_KERNEL);
	if (!buff) {
		IPAERR("ipa_ippt_sw_flt_list_type mem-allocate failure\n");
		return -ENOMEM;
	}

	if (copy_from_user(buff, u64_to_user_ptr(sw_flt_list.ioctl_ptr),
		sizeof(struct ipa_ippt_sw_flt_list_type))) {
		IPAERR("Failed to copy ipa_ippt_sw_flt_list_type\n");
		kfree(buff);
		return -EFAULT;
	}
	memset(&msg_meta, 0, sizeof(struct ipa_msg_meta));
	msg_meta.msg_type = IPA_IPPT_SW_FLT_EVENT;
	msg_meta.msg_len = sizeof(struct ipa_ippt_sw_flt_list_type);

	IPADBG("Num of ipv4: %d, ipv4 enable: %d \n",
		((struct ipa_ippt_sw_flt_list_type *)buff)->num_of_ipv4,
		((struct ipa_ippt_sw_flt_list_type *)buff)->ipv4_enable);

	IPADBG("Num of ports: %d, port enable: %d\n",
		((struct ipa_ippt_sw_flt_list_type *)buff)->num_of_port,
		((struct ipa_ippt_sw_flt_list_type *)buff)->port_enable);

	retval = ipa3_send_msg(&msg_meta, buff,
		ipa3_general_free_cb);
	if (retval) {
		IPAERR("ipa3_send_msg failed: %d, msg_type %d\n",
		retval,
		msg_meta.msg_type);
		kfree(buff);
		return retval;
	}
	return 0;
}

/**
 * ipa3_send_macsec_info() - Pass macsec mapping to the IPACM
 * @event_type: Type of the event - UP or DOWN
 * @map: pointer to macsec to eth mapping structure
 *
 * Returns: 0 on success, negative on failure
 */
int ipa3_send_macsec_info(enum ipa_macsec_event event_type, struct ipa_macsec_map *map)
{
	struct ipa_msg_meta msg_meta;
	int res = 0;

	if (!map) {
		IPAERR("Bad arg: info is NULL\n");
		res = -EIO;
		goto done;
	}

	/*
	 * Prep and send msg to ipacm
	 */
	memset(&msg_meta, 0, sizeof(struct ipa_msg_meta));
	msg_meta.msg_type = event_type;
	msg_meta.msg_len  = sizeof(struct ipa_macsec_map);

	/*
	 * Post event to ipacm
	 */
	res = ipa3_send_msg(&msg_meta, map, ipa3_general_free_cb);

	if (res) {
		IPAERR_RL("ipa3_send_msg failed: %d\n", res);
		kfree(map);
		goto done;
	}

done:
	return res;
}

/**
 * ipa3_send_ext_router_info() - Pass ext_router_info to the IPACM
 * @info: pointer to the ext router info
 *
 * Returns: 0 on success, negative on failure
 */
int ipa3_send_ext_router_info(struct ipa_ioc_ext_router_info *info)
{
	struct ipa_msg_meta msg_meta;
	int res = 0;

	if (!info) {
		IPAERR("Bad arg: info is NULL\n");
		res = -EIO;
		goto done;
	}

	/*
	 * Prep and send msg to ipacm
	 */
	memset(&msg_meta, 0, sizeof(struct ipa_msg_meta));
	msg_meta.msg_type = IPA_SET_EXT_ROUTER_MODE_EVENT;
	msg_meta.msg_len  = sizeof(struct ipa_ioc_ext_router_info);

	IPADBG("Setting IPA to Ext Router mode %d\n", info->mode);

	/*
	 * Post event to ipacm
	 */
	res = ipa3_send_msg(&msg_meta, info, ipa3_general_free_cb);

	if (res) {
		IPAERR_RL("ipa3_send_msg failed: %d\n", res);
		kfree(info);
		goto done;
	}

done:
	return res;
}

static long ipa3_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	u32 pyld_sz;
	u8 header[256] = { 0 };
	u8 *param = NULL;
	bool is_vlan_mode;
	struct ipa_ioc_nat_alloc_mem nat_mem;
	struct ipa_ioc_nat_ipv6ct_table_alloc table_alloc;
	struct ipa_ioc_v4_nat_init nat_init;
	struct ipa_ioc_ipv6ct_init ipv6ct_init;
	struct ipa_ioc_v4_nat_del nat_del;
	struct ipa_ioc_nat_ipv6ct_table_del table_del;
	struct ipa_ioc_nat_pdn_entry mdfy_pdn;
	struct ipa_ioc_nat_dma_cmd *table_dma_cmd;
	struct ipa_ioc_get_vlan_mode vlan_mode;
	struct ipa_ioc_wigig_fst_switch fst_switch;
	struct ipa_ioc_eogre_info eogre_info;
	struct ipa_ioc_macsec_info macsec_info;
	struct ipa_macsec_map *macsec_map;
	struct ipa_ioc_ext_router_info *ext_router_info;
	bool send2uC, send2ipacm;
	size_t sz;
	int pre_entry;
	int hdl;
	unsigned long uptr = 0;
	struct ipa_ioc_get_ep_info ep_info;

	IPADBG("cmd=%x nr=%d\n", cmd, _IOC_NR(cmd));

	if (_IOC_TYPE(cmd) != IPA_IOC_MAGIC)
		return -ENOTTY;

	if (!ipa3_is_ready()) {
		IPAERR("IPA not ready, waiting for init completion\n");
		wait_for_completion(&ipa3_ctx->init_completion_obj);
	}

	IPA_ACTIVE_CLIENTS_INC_SIMPLE();

	switch (cmd) {
	case IPA_IOC_ALLOC_NAT_MEM:
		if (copy_from_user(&nat_mem, (const void __user *)arg,
			sizeof(struct ipa_ioc_nat_alloc_mem))) {
			retval = -EFAULT;
			break;
		}
		/* null terminate the string */
		nat_mem.dev_name[IPA_RESOURCE_NAME_MAX - 1] = '\0';

		if (ipa3_allocate_nat_device(&nat_mem)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, &nat_mem,
			sizeof(struct ipa_ioc_nat_alloc_mem))) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_ALLOC_NAT_TABLE:
		if (copy_from_user(&table_alloc, (const void __user *)arg,
			sizeof(struct ipa_ioc_nat_ipv6ct_table_alloc))) {
			retval = -EFAULT;
			break;
		}

		if (ipa3_allocate_nat_table(&table_alloc)) {
			retval = -EFAULT;
			break;
		}
		if (table_alloc.offset &&
			copy_to_user((void __user *)arg, &table_alloc, sizeof(
				struct ipa_ioc_nat_ipv6ct_table_alloc))) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_ALLOC_IPV6CT_TABLE:
		if (copy_from_user(&table_alloc, (const void __user *)arg,
			sizeof(struct ipa_ioc_nat_ipv6ct_table_alloc))) {
			retval = -EFAULT;
			break;
		}

		if (ipa3_allocate_ipv6ct_table(&table_alloc)) {
			retval = -EFAULT;
			break;
		}
		if (table_alloc.offset &&
			copy_to_user((void __user *)arg, &table_alloc, sizeof(
				struct ipa_ioc_nat_ipv6ct_table_alloc))) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_V4_INIT_NAT:
		if (copy_from_user(&nat_init, (const void __user *)arg,
			sizeof(struct ipa_ioc_v4_nat_init))) {
			retval = -EFAULT;
			break;
		}

		if (ipa3_nat_init_cmd(&nat_init)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_INIT_IPV6CT_TABLE:
		if (copy_from_user(&ipv6ct_init, (const void __user *)arg,
			sizeof(struct ipa_ioc_ipv6ct_init))) {
			retval = -EFAULT;
			break;
		}
		if (ipa3_ipv6ct_init_cmd(&ipv6ct_init)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_TABLE_DMA_CMD:
		table_dma_cmd = (struct ipa_ioc_nat_dma_cmd *)header;
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_nat_dma_cmd))) {
			retval = -EFAULT;
			break;
		}
		pre_entry = table_dma_cmd->entries;
		pyld_sz = sizeof(struct ipa_ioc_nat_dma_cmd) +
			pre_entry * sizeof(struct ipa_ioc_nat_dma_one);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		table_dma_cmd = (struct ipa_ioc_nat_dma_cmd *)param;
		/* add check in case user-space module compromised */
		if (unlikely(table_dma_cmd->entries != pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				table_dma_cmd->entries, pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_table_dma_cmd(table_dma_cmd)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_V4_DEL_NAT:
		if (copy_from_user(&nat_del, (const void __user *)arg,
			sizeof(struct ipa_ioc_v4_nat_del))) {
			retval = -EFAULT;
			break;
		}
		if (ipa3_nat_del_cmd(&nat_del)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_DEL_NAT_TABLE:
		if (copy_from_user(&table_del, (const void __user *)arg,
			sizeof(struct ipa_ioc_nat_ipv6ct_table_del))) {
			retval = -EFAULT;
			break;
		}

		if (ipa3_del_nat_table(&table_del)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_DEL_IPV6CT_TABLE:
		if (copy_from_user(&table_del, (const void __user *)arg,
			sizeof(struct ipa_ioc_nat_ipv6ct_table_del))) {
			retval = -EFAULT;
			break;
		}

		if (ipa3_del_ipv6ct_table(&table_del)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_NAT_MODIFY_PDN:
		if (copy_from_user(&mdfy_pdn, (const void __user *)arg,
			sizeof(struct ipa_ioc_nat_pdn_entry))) {
			retval = -EFAULT;
			break;
		}
		if (ipa3_nat_mdfy_pdn(&mdfy_pdn)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_ADD_HDR:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_add_hdr))) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_add_hdr *)header)->num_hdrs;
		pyld_sz =
		   sizeof(struct ipa_ioc_add_hdr) +
		   pre_entry * sizeof(struct ipa_hdr_add);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_add_hdr *)param)->num_hdrs
			!= pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_add_hdr *)param)->num_hdrs,
				pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_add_hdr_usr((struct ipa_ioc_add_hdr *)param,
			true)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_DEL_HDR:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_del_hdr))) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_del_hdr *)header)->num_hdls;
		pyld_sz =
		   sizeof(struct ipa_ioc_del_hdr) +
		   pre_entry * sizeof(struct ipa_hdr_del);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_del_hdr *)param)->num_hdls
			!= pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_del_hdr *)param)->num_hdls,
				pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_del_hdr_by_user((struct ipa_ioc_del_hdr *)param,
			true)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_ADD_RT_RULE:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_add_rt_rule))) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_add_rt_rule *)header)->num_rules;
		pyld_sz =
		   sizeof(struct ipa_ioc_add_rt_rule) +
		   pre_entry * sizeof(struct ipa_rt_rule_add);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_add_rt_rule *)param)->num_rules
			!= pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_add_rt_rule *)param)->
				num_rules,
				pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_add_rt_rule_usr((struct ipa_ioc_add_rt_rule *)param,
				true)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_ADD_RT_RULE_EXT:
		if (copy_from_user(header,
				(const void __user *)arg,
				sizeof(struct ipa_ioc_add_rt_rule_ext))) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_add_rt_rule_ext *)header)->num_rules;
		pyld_sz =
		   sizeof(struct ipa_ioc_add_rt_rule_ext) +
		   pre_entry * sizeof(struct ipa_rt_rule_add_ext);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(
			((struct ipa_ioc_add_rt_rule_ext *)param)->num_rules
			!= pre_entry)) {
			IPAERR(" prevent memory corruption(%d not match %d)\n",
				((struct ipa_ioc_add_rt_rule_ext *)param)->
				num_rules,
				pre_entry);
			retval = -EINVAL;
			break;
		}
		if (ipa3_add_rt_rule_ext(
			(struct ipa_ioc_add_rt_rule_ext *)param)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_ADD_RT_RULE_AFTER:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_add_rt_rule_after))) {

			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_add_rt_rule_after *)header)->num_rules;
		pyld_sz =
		   sizeof(struct ipa_ioc_add_rt_rule_after) +
		   pre_entry * sizeof(struct ipa_rt_rule_add);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_add_rt_rule_after *)param)->
			num_rules != pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_add_rt_rule_after *)param)->
				num_rules,
				pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_add_rt_rule_after(
			(struct ipa_ioc_add_rt_rule_after *)param)) {

			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_MDFY_RT_RULE:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_mdfy_rt_rule))) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_mdfy_rt_rule *)header)->num_rules;
		pyld_sz =
		   sizeof(struct ipa_ioc_mdfy_rt_rule) +
		   pre_entry * sizeof(struct ipa_rt_rule_mdfy);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_mdfy_rt_rule *)param)->num_rules
			!= pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_mdfy_rt_rule *)param)->
				num_rules,
				pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_mdfy_rt_rule((struct ipa_ioc_mdfy_rt_rule *)param)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_DEL_RT_RULE:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_del_rt_rule))) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_del_rt_rule *)header)->num_hdls;
		pyld_sz =
		   sizeof(struct ipa_ioc_del_rt_rule) +
		   pre_entry * sizeof(struct ipa_rt_rule_del);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_del_rt_rule *)param)->num_hdls
			!= pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_del_rt_rule *)param)->num_hdls,
				pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_del_rt_rule((struct ipa_ioc_del_rt_rule *)param)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_ADD_FLT_RULE:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_add_flt_rule))) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_add_flt_rule *)header)->num_rules;
		pyld_sz =
		   sizeof(struct ipa_ioc_add_flt_rule) +
		   pre_entry * sizeof(struct ipa_flt_rule_add);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_add_flt_rule *)param)->num_rules
			!= pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_add_flt_rule *)param)->
				num_rules,
				pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_add_flt_rule_usr((struct ipa_ioc_add_flt_rule *)param,
				true)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_ADD_FLT_RULE_AFTER:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_add_flt_rule_after))) {

			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_add_flt_rule_after *)header)->
			num_rules;
		pyld_sz =
		   sizeof(struct ipa_ioc_add_flt_rule_after) +
		   pre_entry * sizeof(struct ipa_flt_rule_add);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_add_flt_rule_after *)param)->
			num_rules != pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_add_flt_rule_after *)param)->
				num_rules,
				pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_add_flt_rule_after(
				(struct ipa_ioc_add_flt_rule_after *)param)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_DEL_FLT_RULE:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_del_flt_rule))) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_del_flt_rule *)header)->num_hdls;
		pyld_sz =
		   sizeof(struct ipa_ioc_del_flt_rule) +
		   pre_entry * sizeof(struct ipa_flt_rule_del);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_del_flt_rule *)param)->num_hdls
			!= pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_del_flt_rule *)param)->
				num_hdls,
				pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_del_flt_rule((struct ipa_ioc_del_flt_rule *)param)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_MDFY_FLT_RULE:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_mdfy_flt_rule))) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_mdfy_flt_rule *)header)->num_rules;
		pyld_sz =
		   sizeof(struct ipa_ioc_mdfy_flt_rule) +
		   pre_entry * sizeof(struct ipa_flt_rule_mdfy);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_mdfy_flt_rule *)param)->num_rules
			!= pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_mdfy_flt_rule *)param)->
				num_rules,
				pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_mdfy_flt_rule((struct ipa_ioc_mdfy_flt_rule *)param)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_COMMIT_HDR:
		retval = ipa3_commit_hdr();
		break;
	case IPA_IOC_RESET_HDR:
		retval = ipa3_reset_hdr(false);
		break;
	case IPA_IOC_COMMIT_RT:
		retval = ipa3_commit_rt(arg);
		break;
	case IPA_IOC_RESET_RT:
		retval = ipa3_reset_rt(arg, false);
		break;
	case IPA_IOC_COMMIT_FLT:
		retval = ipa3_commit_flt(arg);
		break;
	case IPA_IOC_RESET_FLT:
		retval = ipa3_reset_flt(arg, false);
		break;
	case IPA_IOC_GET_RT_TBL:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_get_rt_tbl))) {
			retval = -EFAULT;
			break;
		}
		if (ipa3_get_rt_tbl((struct ipa_ioc_get_rt_tbl *)header)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, header,
					sizeof(struct ipa_ioc_get_rt_tbl))) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_PUT_RT_TBL:
		retval = ipa3_put_rt_tbl(arg);
		break;
	case IPA_IOC_GET_HDR:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_get_hdr))) {
			retval = -EFAULT;
			break;
		}
		if (ipa3_get_hdr((struct ipa_ioc_get_hdr *)header)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, header,
			sizeof(struct ipa_ioc_get_hdr))) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_PUT_HDR:
		retval = ipa3_put_hdr(arg);
		break;
	case IPA_IOC_SET_FLT:
		retval = ipa3_cfg_filter(arg);
		break;
	case IPA_IOC_COPY_HDR:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_copy_hdr))) {
			retval = -EFAULT;
			break;
		}
		if (ipa3_copy_hdr((struct ipa_ioc_copy_hdr *)header)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, header,
			sizeof(struct ipa_ioc_copy_hdr))) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_QUERY_INTF:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_query_intf))) {
			retval = -EFAULT;
			break;
		}
		if (ipa3_query_intf((struct ipa_ioc_query_intf *)header)) {
			retval = -1;
			break;
		}
		if (copy_to_user((void __user *)arg, header,
			sizeof(struct ipa_ioc_query_intf))) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_QUERY_INTF_TX_PROPS:
		sz = sizeof(struct ipa_ioc_query_intf_tx_props);
		if (copy_from_user(header, (const void __user *)arg, sz)) {
			retval = -EFAULT;
			break;
		}

		if (((struct ipa_ioc_query_intf_tx_props *)header)->num_tx_props
			> IPA_NUM_PROPS_MAX) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_query_intf_tx_props *)
			header)->num_tx_props;
		pyld_sz = sz + pre_entry *
			sizeof(struct ipa_ioc_tx_intf_prop);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_query_intf_tx_props *)
			param)->num_tx_props
			!= pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_query_intf_tx_props *)
				param)->num_tx_props, pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_query_intf_tx_props(
			(struct ipa_ioc_query_intf_tx_props *)param)) {
			retval = -1;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_QUERY_INTF_RX_PROPS:
		sz = sizeof(struct ipa_ioc_query_intf_rx_props);
		if (copy_from_user(header, (const void __user *)arg, sz)) {
			retval = -EFAULT;
			break;
		}

		if (((struct ipa_ioc_query_intf_rx_props *)header)->num_rx_props
			> IPA_NUM_PROPS_MAX) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_query_intf_rx_props *)
			header)->num_rx_props;
		pyld_sz = sz + pre_entry *
			sizeof(struct ipa_ioc_rx_intf_prop);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_query_intf_rx_props *)
			param)->num_rx_props != pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_query_intf_rx_props *)
				param)->num_rx_props, pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_query_intf_rx_props(
			(struct ipa_ioc_query_intf_rx_props *)param)) {
			retval = -1;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_QUERY_INTF_EXT_PROPS:
		sz = sizeof(struct ipa_ioc_query_intf_ext_props);
		if (copy_from_user(header, (const void __user *)arg, sz)) {
			retval = -EFAULT;
			break;
		}

		if (((struct ipa_ioc_query_intf_ext_props *)
			header)->num_ext_props > IPA_NUM_PROPS_MAX) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_query_intf_ext_props *)
			header)->num_ext_props;
		pyld_sz = sz + pre_entry *
			sizeof(struct ipa_ioc_ext_intf_prop);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_query_intf_ext_props *)
			param)->num_ext_props != pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_query_intf_ext_props *)
				param)->num_ext_props, pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_query_intf_ext_props(
			(struct ipa_ioc_query_intf_ext_props *)param)) {
			retval = -1;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_PULL_MSG:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_msg_meta))) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
		   ((struct ipa_msg_meta *)header)->msg_len;
		pyld_sz = sizeof(struct ipa_msg_meta) +
		   pre_entry;
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_msg_meta *)param)->msg_len
			!= pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_msg_meta *)param)->msg_len,
				pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_pull_msg((struct ipa_msg_meta *)param,
			(char *)param + sizeof(struct ipa_msg_meta),
			((struct ipa_msg_meta *)param)->msg_len) !=
			((struct ipa_msg_meta *)param)->msg_len) {
			retval = -1;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_RM_ADD_DEPENDENCY:
		/* IPA RM is deprecate because IPA PM is used */
		IPAERR("using obselete command: IPA_IOC_RM_ADD_DEPENDENCY");
		retval = -EINVAL;
		break;

	case IPA_IOC_RM_DEL_DEPENDENCY:
		/* IPA RM is deprecate because IPA PM is used */
		IPAERR("using obselete command: IPA_IOC_RM_DEL_DEPENDENCY");
		retval = -EINVAL;
		break;

	case IPA_IOC_GENERATE_FLT_EQ:
		{
			struct ipa_ioc_generate_flt_eq flt_eq;

			if (copy_from_user(&flt_eq, (const void __user *)arg,
				sizeof(struct ipa_ioc_generate_flt_eq))) {
				retval = -EFAULT;
				break;
			}
			if (ipahal_flt_generate_equation(flt_eq.ip,
				&flt_eq.attrib, &flt_eq.eq_attrib)) {
				retval = -EFAULT;
				break;
			}
			if (copy_to_user((void __user *)arg, &flt_eq,
				sizeof(struct ipa_ioc_generate_flt_eq))) {
				retval = -EFAULT;
				break;
			}
			break;
		}
	case IPA_IOC_QUERY_EP_MAPPING:
		{
			retval = ipa3_get_ep_mapping(arg);
			break;
		}
	case IPA_IOC_QUERY_RT_TBL_INDEX:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_get_rt_tbl_indx))) {
			retval = -EFAULT;
			break;
		}
		if (ipa3_query_rt_index(
			(struct ipa_ioc_get_rt_tbl_indx *)header)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, header,
			sizeof(struct ipa_ioc_get_rt_tbl_indx))) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_WRITE_QMAPID:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_write_qmapid))) {
			retval = -EFAULT;
			break;
		}
		if (ipa3_write_qmap_id((struct ipa_ioc_write_qmapid *)header)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, header,
			sizeof(struct ipa_ioc_write_qmapid))) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_NOTIFY_WAN_UPSTREAM_ROUTE_ADD:
		retval = ipa3_send_wan_msg(arg, WAN_UPSTREAM_ROUTE_ADD, true);
		if (retval) {
			IPAERR("ipa3_send_wan_msg failed: %d\n", retval);
			break;
		}
		break;
	case IPA_IOC_NOTIFY_WAN_UPSTREAM_ROUTE_DEL:
		retval = ipa3_send_wan_msg(arg, WAN_UPSTREAM_ROUTE_DEL, true);
		if (retval) {
			IPAERR("ipa3_send_wan_msg failed: %d\n", retval);
			break;
		}
		break;
	case IPA_IOC_NOTIFY_WAN_EMBMS_CONNECTED:
		retval = ipa3_send_wan_msg(arg, WAN_EMBMS_CONNECT, false);
		if (retval) {
			IPAERR("ipa3_send_wan_msg failed: %d\n", retval);
			break;
		}
		break;
	case IPA_IOC_ADD_HDR_PROC_CTX:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_add_hdr_proc_ctx))) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_add_hdr_proc_ctx *)
			header)->num_proc_ctxs;
		pyld_sz =
		   sizeof(struct ipa_ioc_add_hdr_proc_ctx) +
		   pre_entry * sizeof(struct ipa_hdr_proc_ctx_add);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_add_hdr_proc_ctx *)
			param)->num_proc_ctxs != pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_add_hdr_proc_ctx *)
				param)->num_proc_ctxs, pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_add_hdr_proc_ctx(
			(struct ipa_ioc_add_hdr_proc_ctx *)param, true)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_DEL_HDR_PROC_CTX:
		if (copy_from_user(header, (const void __user *)arg,
			sizeof(struct ipa_ioc_del_hdr_proc_ctx))) {
			retval = -EFAULT;
			break;
		}
		pre_entry =
			((struct ipa_ioc_del_hdr_proc_ctx *)header)->num_hdls;
		pyld_sz =
		   sizeof(struct ipa_ioc_del_hdr_proc_ctx) +
		   pre_entry * sizeof(struct ipa_hdr_proc_ctx_del);
		param = memdup_user((const void __user *)arg, pyld_sz);
		if (IS_ERR(param)) {
			retval = PTR_ERR(param);
			break;
		}
		/* add check in case user-space module compromised */
		if (unlikely(((struct ipa_ioc_del_hdr_proc_ctx *)
			param)->num_hdls != pre_entry)) {
			IPAERR_RL("current %d pre %d\n",
				((struct ipa_ioc_del_hdr_proc_ctx *)param)->
				num_hdls,
				pre_entry);
			retval = -EFAULT;
			break;
		}
		if (ipa3_del_hdr_proc_ctx_by_user(
			(struct ipa_ioc_del_hdr_proc_ctx *)param, true)) {
			retval = -EFAULT;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_GET_HW_VERSION:
		pyld_sz = sizeof(enum ipa_hw_type);
		param = kmemdup(&ipa3_ctx->ipa_hw_type, pyld_sz, GFP_KERNEL);
		if (!param) {
			retval = -ENOMEM;
			break;
		}
		if (copy_to_user((void __user *)arg, param, pyld_sz)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_GET_VLAN_MODE:
		if (copy_from_user(&vlan_mode, (const void __user *)arg,
			sizeof(struct ipa_ioc_get_vlan_mode))) {
			retval = -EFAULT;
			break;
		}
		retval = ipa3_is_vlan_mode(
			vlan_mode.iface,
			&is_vlan_mode);
		if (retval)
			break;

		vlan_mode.is_vlan_mode = is_vlan_mode;

		if (copy_to_user((void __user *)arg,
			&vlan_mode,
			sizeof(struct ipa_ioc_get_vlan_mode))) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_ADD_VLAN_IFACE:
		if (ipa3_send_vlan_l2tp_msg(arg, ADD_VLAN_IFACE)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_DEL_VLAN_IFACE:
		if (ipa3_send_vlan_l2tp_msg(arg, DEL_VLAN_IFACE)) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_ADD_BRIDGE_VLAN_MAPPING:
		if (ipa3_send_vlan_l2tp_msg(arg, ADD_BRIDGE_VLAN_MAPPING)) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_DEL_BRIDGE_VLAN_MAPPING:
		if (ipa3_send_vlan_l2tp_msg(arg, DEL_BRIDGE_VLAN_MAPPING)) {
			retval = -EFAULT;
			break;
		}
		break;
	case IPA_IOC_ADD_L2TP_VLAN_MAPPING:
		if (ipa3_send_vlan_l2tp_msg(arg, ADD_L2TP_VLAN_MAPPING)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_DEL_L2TP_VLAN_MAPPING:
		if (ipa3_send_vlan_l2tp_msg(arg, DEL_L2TP_VLAN_MAPPING)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_CLEANUP:
		/*Route and filter rules will also be clean*/
		IPADBG("Got IPA_IOC_CLEANUP\n");
		retval = ipa3_reset_hdr(true);
		IPA_ACTIVE_CLIENTS_INC_SPECIAL("SRAM");
		retval = ipa3_nat_cleanup_cmd();
		IPA_ACTIVE_CLIENTS_DEC_SPECIAL("SRAM");
		if (ipa3_ctx->platform_type == IPA_PLAT_TYPE_APQ)
			retval = ipa3_clean_mhip_dl_rule();
		else
			retval = ipa3_clean_modem_rule();
		ipa3_counter_id_remove_all();
		break;

	case IPA_IOC_QUERY_WLAN_CLIENT:
		IPADBG("Got IPA_IOC_QUERY_WLAN_CLIENT\n");
		retval = ipa3_resend_wlan_msg();
		break;

	case IPA_IOC_QUERY_CACHED_DRIVER_MSG:
		IPADBG("Got IPA_IOC_QUERY_CACHED_DRIVER_MSG\n");
		retval = ipa3_resend_driver_msg();
		break;

	case IPA_IOC_GSB_CONNECT:
		IPADBG("Got IPA_IOC_GSB_CONNECT\n");
		if (ipa3_send_gsb_msg(arg, IPA_GSB_CONNECT)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_GSB_DISCONNECT:
		IPADBG("Got IPA_IOC_GSB_DISCONNECT\n");
		if (ipa3_send_gsb_msg(arg, IPA_GSB_DISCONNECT)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_ADD_RT_RULE_V2:
		retval = ipa3_ioctl_add_rt_rule_v2(arg);
		break;

	case IPA_IOC_ADD_RT_RULE_EXT_V2:
		retval = ipa3_ioctl_add_rt_rule_ext_v2(arg);
		break;

	case IPA_IOC_ADD_RT_RULE_AFTER_V2:
		retval = ipa3_ioctl_add_rt_rule_after_v2(arg);
		break;

	case IPA_IOC_MDFY_RT_RULE_V2:
		retval = ipa3_ioctl_mdfy_rt_rule_v2(arg);
		break;

	case IPA_IOC_ADD_FLT_RULE_V2:
		retval = ipa3_ioctl_add_flt_rule_v2(arg);
		break;

	case IPA_IOC_ADD_FLT_RULE_AFTER_V2:
		retval = ipa3_ioctl_add_flt_rule_after_v2(arg);
		break;

	case IPA_IOC_MDFY_FLT_RULE_V2:
		retval = ipa3_ioctl_mdfy_flt_rule_v2(arg);
		break;

	case IPA_IOC_FNR_COUNTER_ALLOC:
		if (ipa3_ctx->ipa_hw_type < IPA_HW_v4_5) {
			IPAERR("FNR stats not supported on IPA ver %d",
				ipa3_ctx->ipa_hw_type);
			retval = -EFAULT;
			break;
		}
		retval = ipa3_ioctl_fnr_counter_alloc(arg);
		break;

	case IPA_IOC_FNR_COUNTER_DEALLOC:
		if (ipa3_ctx->ipa_hw_type < IPA_HW_v4_5) {
			IPAERR("FNR stats not supported on IPA ver %d",
				 ipa3_ctx->ipa_hw_type);
			retval = -EFAULT;
			break;
		}
		hdl = (int)arg;
		if (hdl < 0) {
			IPAERR("IPA_FNR_COUNTER_DEALLOC failed: hdl %d\n",
				hdl);
			retval = -EPERM;
			break;
		}
		ipa3_counter_remove_hdl(hdl);
		break;

	case IPA_IOC_FNR_COUNTER_QUERY:
		if (ipa3_ctx->ipa_hw_type < IPA_HW_v4_5) {
			IPAERR("FNR stats not supported on IPA ver %d",
				ipa3_ctx->ipa_hw_type);
			retval = -EFAULT;
			break;
		}
		retval = ipa3_ioctl_fnr_counter_query(arg);
		break;

	case IPA_IOC_SET_FNR_COUNTER_INFO:
		if (ipa3_ctx->ipa_hw_type < IPA_HW_v4_5) {
			IPAERR("FNR stats not supported on IPA ver %d",
				ipa3_ctx->ipa_hw_type);
			retval = -EFAULT;
			break;
		}
		retval = ipa3_ioctl_fnr_counter_set(arg);
		break;

	case IPA_IOC_WIGIG_FST_SWITCH:
		IPADBG("Got IPA_IOCTL_WIGIG_FST_SWITCH\n");
		if (copy_from_user(&fst_switch, (const void __user *)arg,
			sizeof(struct ipa_ioc_wigig_fst_switch))) {
			retval = -EFAULT;
			break;
		}

		/* null terminate the string */
		fst_switch.netdev_name[IPA_RESOURCE_NAME_MAX - 1] = '\0';

		retval = ipa_wigig_send_msg(WIGIG_FST_SWITCH,
			fst_switch.netdev_name,
			fst_switch.client_mac_addr,
			IPA_CLIENT_MAX,
			fst_switch.to_wigig);
		break;

	case IPA_IOC_GET_NAT_IN_SRAM_INFO:
		retval = proc_sram_info_rqst(arg);
		break;

	case IPA_IOC_APP_CLOCK_VOTE:
		retval = ipa3_app_clk_vote(
			(enum ipa_app_clock_vote_type) arg);
		break;

	case IPA_IOC_PDN_CONFIG:
		if (ipa3_send_pdn_config_msg(arg)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_SET_MAC_FLT:
		IPADBG("Got IPA_IOC_SET_MAC_FLT\n");
		if (ipa3_send_mac_flt_list(arg)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_SET_SW_FLT:
		IPADBG("Got IPA_IOC_SET_SW_FLT\n");
		if (ipa3_send_sw_flt_list(arg)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_SET_IPPT_SW_FLT:
		IPADBG("Got IPA_IOC_SET_IPPT_SW_FLT\n");
		if (ipa3_send_ippt_sw_flt_list(arg)) {
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_GET_PHERIPHERAL_EP_INFO:
		IPADBG("Got IPA_IOC_GET_EP_INFO\n");
		/* used in IPA4.X AUTO and IPA5.0 MDM onwards */
		if (copy_from_user(&ep_info, (const void __user *)arg,
			sizeof(struct ipa_ioc_get_ep_info))) {
			IPAERR_RL("copy_from_user fails\n");
			retval = -EFAULT;
			break;
		}

		if (ep_info.max_ep_pairs != QUERY_MAX_EP_PAIRS) {
			IPAERR_RL("unexpected max_ep_pairs %d\n",
			ep_info.max_ep_pairs);
			retval = -EFAULT;
			break;
		}

		if (ep_info.ep_pair_size !=
			(QUERY_MAX_EP_PAIRS * sizeof(struct ipa_ep_pair_info))) {
			IPAERR_RL("unexpected ep_pair_size %d\n",
			ep_info.max_ep_pairs);
			retval = -EFAULT;
			break;
		}

		uptr = ep_info.info;
		if (unlikely(!uptr)) {
			IPAERR_RL("unexpected NULL info\n");
			retval = -EFAULT;
			break;
		}

		param = kzalloc(ep_info.ep_pair_size, GFP_KERNEL);
		if (!param) {
			IPAERR_RL("kzalloc fails\n");
			retval = -ENOMEM;
			break;
		}

		retval = ipa3_get_ep_info(&ep_info, param);
		if (retval < 0) {
			IPAERR("ipa3_get_ep_info failed\n");
			retval = -EFAULT;
			break;
		}

		if (copy_to_user((void __user *)uptr, param,
			ep_info.ep_pair_size)) {
			IPAERR_RL("copy_to_user fails\n");
			retval = -EFAULT;
			break;
		}

		if (copy_to_user((void __user *)arg, &ep_info,
			sizeof(struct ipa_ioc_get_ep_info))) {
			IPAERR_RL("copy_to_user fails\n");
			retval = -EFAULT;
			break;
		}
		break;

	case IPA_IOC_SET_PKT_THRESHOLD:
		IPADBG("Got IPA_IOC_SET_PKT_THRESHOLD\n");
		if (ipa3_send_pkt_threshold(arg))
			retval = -EFAULT;
		break;

	case IPA_IOC_ADD_EoGRE_MAPPING:
		IPADBG("Got IPA_IOC_ADD_EoGRE_MAPPING\n");
		if (copy_from_user(
				&eogre_info,
				(const void __user *) arg,
				sizeof(struct ipa_ioc_eogre_info))) {
			IPAERR_RL("copy_from_user fails\n");
			retval = -EFAULT;
			break;
		}

		retval = ipa3_check_eogre(&eogre_info, &send2uC, &send2ipacm);
		if (retval == -EIO)
		{
			IPADBG("no work needs to be done but return success to caller");
			retval = 0;
			break;
		}

		ipa3_ctx->eogre_enabled = (retval == 0);

		if (retval == 0 && send2uC == true) {
			/*
			 * Send map to uC...
			 */
			retval = ipa3_add_dscp_vlan_pcp_map(
				&eogre_info.map_info);
		}

		if (retval == 0 && send2ipacm == true) {
			/*
			 * Send ip addrs to ipacm...
			 */
			retval = ipa3_send_eogre_info(IPA_EoGRE_UP_EVENT, &eogre_info);
		}

		if (retval != 0) {
			ipa3_ctx->eogre_enabled = false;
		}

		break;

	case IPA_IOC_DEL_EoGRE_MAPPING:
		IPADBG("Got IPA_IOC_DEL_EoGRE_MAPPING\n");

		memset(&eogre_info, 0, sizeof(eogre_info));

		retval = ipa3_check_eogre(&eogre_info, &send2uC, &send2ipacm);
		if (retval == -EIO)
		{
			IPADBG("no work needs to be done but return success to caller");
			retval = 0;
			break;
		}

		if (retval == 0 && send2uC == true) {
			/*
			 * Send map clear to uC...
			 */
			retval = ipa3_add_dscp_vlan_pcp_map(
				&eogre_info.map_info);
		}

		if (retval == 0 && send2ipacm == true) {
			/*
			 * Send null ip addrs to ipacm...
			 */
			retval = ipa3_send_eogre_info(IPA_EoGRE_DOWN_EVENT, &eogre_info);
		}

		if (retval == 0) {
			ipa3_ctx->eogre_enabled = false;
		}

		break;
#ifdef IPA_IOC_FLT_MEM_PERIPHERAL_SET_PRIO_HIGH
	case IPA_IOC_FLT_MEM_PERIPHERAL_SET_PRIO_HIGH:
		retval = ipa_flt_sram_set_client_prio_high((enum ipa_client_type) arg);
		if (retval)
			IPAERR("ipa_flt_sram_set_client_prio_high failed! retval=%d\n", retval);
		break;
#endif

	case IPA_IOC_ADD_MACSEC_MAPPING:
	case IPA_IOC_DEL_MACSEC_MAPPING:
		IPADBG("Got %s\n", cmd == IPA_IOC_ADD_MACSEC_MAPPING ?
			"IPA_IOC_ADD_MACSEC_MAPPING" : "IPA_IOC_DEL_MACSEC_MAPPING");
		if (copy_from_user(&macsec_info, (const void __user *) arg,
			sizeof(struct ipa_ioc_macsec_info))) {
			IPAERR_RL("copy_from_user for ipa_ioc_macsec_info fails\n");
			retval = -EFAULT;
			break;
		}

		/* Validate the input */
		if (macsec_info.ioctl_data_size != sizeof(struct ipa_macsec_map)) {
			IPAERR_RL("data size missmatch\n");
			retval = -EFAULT;
			break;
		}

		macsec_map = kzalloc(sizeof(struct ipa_macsec_map), GFP_KERNEL);
		if (!macsec_map) {
			IPAERR("macsec_map memory allocation failed !\n");
			retval = -ENOMEM;
			break;
		}

		if (copy_from_user(macsec_map, (const void __user *)(macsec_info.ioctl_ptr),
			sizeof(struct ipa_macsec_map))) {
			IPAERR_RL("copy_from_user for ipa_macsec_map fails\n");
			retval = -EFAULT;
			kfree(macsec_map);
			break;
		}

		/* Send message to the IPACM */
		ipa3_send_macsec_info(
			(cmd == IPA_IOC_ADD_MACSEC_MAPPING) ?
			IPA_MACSEC_ADD_EVENT : IPA_MACSEC_DEL_EVENT,
			macsec_map);
		break;

	case IPA_IOC_SET_EXT_ROUTER_MODE:
		IPADBG("Got IPA_IOC_SET_EXT_ROUTER_MODE\n");

		ext_router_info = kzalloc(sizeof(struct ipa_ioc_ext_router_info), GFP_KERNEL);
		if (!ext_router_info) {
			IPAERR("ext_router_info memory allocation failed !\n");
			retval = -ENOMEM;
			break;
		}

		if (copy_from_user(ext_router_info, (const void __user *) arg,
				sizeof(struct ipa_ioc_ext_router_info))) {
			IPAERR_RL("copy_from_user fails\n");
			retval = -EFAULT;
			kfree(ext_router_info);
			break;
		}

		if (ipa3_send_ext_router_info(ext_router_info)) {
			IPAERR("failed to send ext_router_info!\n");
			retval = -EFAULT;
		}
		break;

	default:
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		return -ENOTTY;
	}
	if (!IS_ERR(param))
		kfree(param);
	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();

	return retval;
}

/**
 * ipa3_setup_dflt_rt_tables() - Setup default routing tables
 *
 * Return codes:
 * 0: success
 * -ENOMEM: failed to allocate memory
 * -EPERM: failed to add the tables
 */
int ipa3_setup_dflt_rt_tables(void)
{
	struct ipa_ioc_add_rt_rule *rt_rule;
	struct ipa_rt_rule_add *rt_rule_entry;

	rt_rule =
		kzalloc(sizeof(struct ipa_ioc_add_rt_rule) + 1 *
			sizeof(struct ipa_rt_rule_add), GFP_KERNEL);
	if (!rt_rule)
		return -ENOMEM;

	/* setup a default v4 route to point to Apps */
	rt_rule->num_rules = 1;
	rt_rule->commit = 1;
	rt_rule->ip = IPA_IP_v4;
	strlcpy(rt_rule->rt_tbl_name, IPA_DFLT_RT_TBL_NAME,
		IPA_RESOURCE_NAME_MAX);

	rt_rule_entry = &rt_rule->rules[0];
	rt_rule_entry->at_rear = 1;
	rt_rule_entry->rule.dst = IPA_CLIENT_APPS_LAN_CONS;
	rt_rule_entry->rule.hdr_hdl = ipa3_ctx->excp_hdr_hdl;
	rt_rule_entry->rule.retain_hdr = 1;

	if (ipa3_add_rt_rule(rt_rule)) {
		IPAERR("fail to add dflt v4 rule\n");
		kfree(rt_rule);
		return -EPERM;
	}
	IPADBG("dflt v4 rt rule hdl=%x\n", rt_rule_entry->rt_rule_hdl);
	ipa3_ctx->dflt_v4_rt_rule_hdl = rt_rule_entry->rt_rule_hdl;

	/* setup a default v6 route to point to A5 */
	rt_rule->ip = IPA_IP_v6;
	if (ipa3_add_rt_rule(rt_rule)) {
		IPAERR("fail to add dflt v6 rule\n");
		kfree(rt_rule);
		return -EPERM;
	}
	IPADBG("dflt v6 rt rule hdl=%x\n", rt_rule_entry->rt_rule_hdl);
	ipa3_ctx->dflt_v6_rt_rule_hdl = rt_rule_entry->rt_rule_hdl;

	/*
	 * because these tables are the very first to be added, they will both
	 * have the same index (0) which is essential for programming the
	 * "route" end-point config
	 */

	kfree(rt_rule);

	return 0;
}

static int ipa3_setup_exception_path(void)
{
	struct ipa_ioc_add_hdr *hdr;
	struct ipa_hdr_add *hdr_entry;
	struct ipahal_reg_route route = { 0 };
	struct ipa3_hdr_entry *hdr_entry_internal;
	int ret;

	/* install the basic exception header */
	hdr = kzalloc(sizeof(struct ipa_ioc_add_hdr) + 1 *
		sizeof(struct ipa_hdr_add), GFP_KERNEL);
	if (!hdr)
		return -ENOMEM;

	hdr->num_hdrs = 1;
	hdr->commit = 1;
	hdr_entry = &hdr->hdr[0];

	strlcpy(hdr_entry->name, IPA_LAN_RX_HDR_NAME, IPA_RESOURCE_NAME_MAX);
	hdr_entry->hdr_len = IPA_LAN_RX_HEADER_LENGTH;

	if (ipa3_add_hdr(hdr)) {
		IPAERR("fail to add exception hdr\n");
		ret = -EPERM;
		goto bail;
	}

	if (hdr_entry->status) {
		IPAERR("fail to add exception hdr\n");
		ret = -EPERM;
		goto bail;
	}

	hdr_entry_internal = ipa3_id_find(hdr_entry->hdr_hdl);
	if (unlikely(!hdr_entry_internal)) {
		IPAERR("fail to find internal hdr structure\n");
		ret = -EPERM;
		goto bail;
	}

	ipa3_ctx->excp_hdr_hdl = hdr_entry->hdr_hdl;

	/* set the route register to pass exception packets to Apps */
	route.route_def_pipe = ipa3_get_ep_mapping(IPA_CLIENT_APPS_LAN_CONS);
	route.route_frag_def_pipe = ipa3_get_ep_mapping(
		IPA_CLIENT_APPS_LAN_CONS);
	route.route_def_hdr_table = !hdr_entry_internal->is_lcl;
	route.route_def_retain_hdr = 1;

	if (ipa3_cfg_route(&route)) {
		IPAERR("fail to add exception hdr\n");
		ret = -EPERM;
		goto bail;
	}

	ret = 0;
bail:
	kfree(hdr);
	return ret;
}

static int ipa3_init_smem_region(int memory_region_size,
				int memory_region_offset)
{
	struct ipahal_imm_cmd_dma_shared_mem cmd;
	struct ipahal_imm_cmd_pyld *cmd_pyld;
	struct ipa3_desc desc;
	struct ipa_mem_buffer mem;
	int rc;

	if (memory_region_size == 0)
		return 0;

	memset(&desc, 0, sizeof(desc));
	memset(&cmd, 0, sizeof(cmd));
	memset(&mem, 0, sizeof(mem));

	mem.size = memory_region_size;
	mem.base = dma_alloc_coherent(ipa3_ctx->pdev, mem.size,
		&mem.phys_base, GFP_KERNEL);
	if (!mem.base) {
		IPAERR("failed to alloc DMA buff of size %d\n", mem.size);
		return -ENOMEM;
	}

	cmd.is_read = false;
	cmd.skip_pipeline_clear = false;
	cmd.pipeline_clear_options = IPAHAL_HPS_CLEAR;
	cmd.size = mem.size;
	cmd.system_addr = mem.phys_base;
	cmd.local_addr = ipa3_ctx->smem_restricted_bytes +
		memory_region_offset;
	cmd_pyld = ipahal_construct_imm_cmd(
		IPA_IMM_CMD_DMA_SHARED_MEM, &cmd, false);
	if (!cmd_pyld) {
		IPAERR("failed to construct dma_shared_mem imm cmd\n");
		return -ENOMEM;
	}
	ipa3_init_imm_cmd_desc(&desc, cmd_pyld);

	rc = ipa3_send_cmd(1, &desc);
	if (rc) {
		IPAERR("failed to send immediate command (error %d)\n", rc);
		rc = -EFAULT;
	}

	ipahal_destroy_imm_cmd(cmd_pyld);
	dma_free_coherent(ipa3_ctx->pdev, mem.size, mem.base,
		mem.phys_base);

	return rc;
}

/**
 * ipa3_init_q6_smem() - Initialize Q6 general memory and
 *	header memory regions in IPA.
 *
 * Return codes:
 * 0: success
 * -ENOMEM: failed to allocate dma memory
 * -EFAULT: failed to send IPA command to initialize the memory
 */
int ipa3_init_q6_smem(void)
{
	int rc;

	IPA_ACTIVE_CLIENTS_INC_SIMPLE();

	rc = ipa3_init_smem_region(IPA_MEM_PART(modem_size),
		IPA_MEM_PART(modem_ofst));
	if (rc) {
		IPAERR("failed to initialize Modem RAM memory\n");
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		return rc;
	}

	rc = ipa3_init_smem_region(IPA_MEM_PART(modem_hdr_size),
		IPA_MEM_PART(modem_hdr_ofst));
	if (rc) {
		IPAERR("failed to initialize Modem HDRs RAM memory\n");
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		return rc;
	}

	rc = ipa3_init_smem_region(IPA_MEM_PART(modem_hdr_proc_ctx_size),
		IPA_MEM_PART(modem_hdr_proc_ctx_ofst));
	if (rc) {
		IPAERR("failed to initialize Modem proc ctx RAM memory\n");
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		return rc;
	}

	rc = ipa3_init_smem_region(IPA_MEM_PART(modem_comp_decomp_size),
		IPA_MEM_PART(modem_comp_decomp_ofst));
	if (rc) {
		IPAERR("failed to initialize Modem Comp/Decomp RAM memory\n");
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		return rc;
	}
	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();

	return rc;
}

static void ipa3_destroy_imm(void *user1, int user2)
{
	ipahal_destroy_imm_cmd(user1);
}

static void ipa3_q6_pipe_flow_control(bool delay)
{
	int ep_idx;
	int client_idx;
	int code = 0, result;
	const struct ipa_gsi_ep_config *gsi_ep_cfg;

	for (client_idx = 0; client_idx < IPA_CLIENT_MAX; client_idx++) {
		if (IPA_CLIENT_IS_Q6_PROD(client_idx)) {
			ep_idx = ipa3_get_ep_mapping(client_idx);
			if (ep_idx == -1)
				continue;
			gsi_ep_cfg = ipa3_get_gsi_ep_info(client_idx);
			if (!gsi_ep_cfg) {
				IPAERR("failed to get GSI config\n");
				ipa_assert();
				return;
			}
			IPADBG("pipe setting V2 flow control\n");
			/* Configurig primary flow control on Q6 pipes*/
			result = gsi_flow_control_ee(
					gsi_ep_cfg->ipa_gsi_chan_num, ep_idx,
					gsi_ep_cfg->ee, delay, false, &code);
			if (result == GSI_STATUS_SUCCESS) {
				IPADBG("sussess gsi ch %d with code %d\n",
					gsi_ep_cfg->ipa_gsi_chan_num, code);
			} else {
				IPADBG("failed  gsi ch %d code %d\n",
					gsi_ep_cfg->ipa_gsi_chan_num, code);
			}
		}
	}
}

static void ipa3_q6_pipe_delay(bool delay)
{
	int client_idx;
	int ep_idx;
	struct ipa_ep_cfg_ctrl ep_ctrl;

	memset(&ep_ctrl, 0, sizeof(struct ipa_ep_cfg_ctrl));
	ep_ctrl.ipa_ep_delay = delay;

	for (client_idx = 0; client_idx < IPA_CLIENT_MAX; client_idx++) {
		if (IPA_CLIENT_IS_Q6_PROD(client_idx)) {
			ep_idx = ipa3_get_ep_mapping(client_idx);
			if (ep_idx == -1)
				continue;

			ipahal_write_reg_n_fields(IPA_ENDP_INIT_CTRL_n,
				ep_idx, &ep_ctrl);
		}
	}
}

static void ipa3_q6_avoid_holb(void)
{
	int ep_idx;
	int client_idx;
	struct ipa_ep_cfg_ctrl ep_suspend;
	struct ipa_ep_cfg_holb ep_holb;

	memset(&ep_suspend, 0, sizeof(ep_suspend));
	memset(&ep_holb, 0, sizeof(ep_holb));

	ep_suspend.ipa_ep_suspend = true;
	ep_holb.tmr_val = 0;
	ep_holb.en = 1;

	if (ipa3_ctx->ipa_hw_type == IPA_HW_v4_2)
		ipa3_cal_ep_holb_scale_base_val(ep_holb.tmr_val, &ep_holb);

	for (client_idx = 0; client_idx < IPA_CLIENT_MAX; client_idx++) {
		if (IPA_CLIENT_IS_Q6_CONS(client_idx)) {
			ep_idx = ipa3_get_ep_mapping(client_idx);
			if (ep_idx == -1)
				continue;

			/* from IPA 4.0 pipe suspend is not supported */
			if (ipa3_ctx->ipa_hw_type < IPA_HW_v4_0)
				ipahal_write_reg_n_fields(
				IPA_ENDP_INIT_CTRL_n,
				ep_idx, &ep_suspend);

			/*
			 * ipa3_cfg_ep_holb is not used here because we are
			 * setting HOLB on Q6 pipes, and from APPS perspective
			 * they are not valid, therefore, the above function
			 * will fail.
			 * Also don't reset the HOLB timer to 0 for Q6 pipes.
			 */



			ipahal_write_reg_n_fields(
				IPA_ENDP_INIT_HOL_BLOCK_EN_n,
				ep_idx, &ep_holb);

			/* For targets > IPA_4.0 issue requires HOLB_EN to
			 * be written twice.
			 */
			if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_0)
				ipahal_write_reg_n_fields(
					IPA_ENDP_INIT_HOL_BLOCK_EN_n,
					ep_idx, &ep_holb);
		}
	}
}

static void ipa3_halt_q6_gsi_channels(bool prod)
{
	int ep_idx;
	int client_idx;
	const struct ipa_gsi_ep_config *gsi_ep_cfg;
	int i;
	int ret;
	int code = 0;

	/* if prod flag is true, then we halt the producer channels also */
	for (client_idx = 0; client_idx < IPA_CLIENT_MAX; client_idx++) {
		if (IPA_CLIENT_IS_Q6_CONS(client_idx)
			|| (IPA_CLIENT_IS_Q6_PROD(client_idx) && prod)) {
			ep_idx = ipa3_get_ep_mapping(client_idx);
			if (ep_idx == -1)
				continue;

			gsi_ep_cfg = ipa3_get_gsi_ep_info(client_idx);
			if (!gsi_ep_cfg) {
				IPAERR("failed to get GSI config\n");
				ipa_assert();
				return;
			}

			ret = gsi_halt_channel_ee(
				gsi_ep_cfg->ipa_gsi_chan_num, gsi_ep_cfg->ee,
				&code);
			for (i = 0; i < IPA_GSI_CHANNEL_STOP_MAX_RETRY &&
				ret == -GSI_STATUS_AGAIN; i++) {
				IPADBG(
				"ch %d ee %d with code %d\n is busy try again",
					gsi_ep_cfg->ipa_gsi_chan_num,
					gsi_ep_cfg->ee,
					code);
				usleep_range(IPA_GSI_CHANNEL_HALT_MIN_SLEEP,
					IPA_GSI_CHANNEL_HALT_MAX_SLEEP);
				ret = gsi_halt_channel_ee(
					gsi_ep_cfg->ipa_gsi_chan_num,
					gsi_ep_cfg->ee, &code);
			}
			if (ret == GSI_STATUS_SUCCESS)
				IPADBG("halted gsi ch %d ee %d with code %d\n",
				gsi_ep_cfg->ipa_gsi_chan_num,
				gsi_ep_cfg->ee,
				code);
			else
				IPAERR("failed to halt ch %d ee %d code %d\n",
				gsi_ep_cfg->ipa_gsi_chan_num,
				gsi_ep_cfg->ee,
				code);
		}
	}
}

static int ipa3_q6_clean_q6_flt_tbls(enum ipa_ip_type ip,
	enum ipa_rule_type rlt)
{
	struct ipa3_desc *desc;
	struct ipahal_imm_cmd_dma_shared_mem cmd = {0};
	struct ipahal_imm_cmd_pyld **cmd_pyld;
	int retval = 0;
	int pipe_idx;
	int flt_idx = 0;
	int num_cmds = 0, count = 0;
	int index;
	u32 lcl_addr_mem_part;
	u32 lcl_hdr_sz;
	struct ipa_mem_buffer mem;
	struct ipahal_reg_valmask valmask;
	struct ipahal_imm_cmd_register_write reg_write_coal_close;
	int coal_ep = IPA_EP_NOT_ALLOCATED;

	IPADBG("Entry\n");

	if ((ip >= IPA_IP_MAX) || (rlt >= IPA_RULE_TYPE_MAX)) {
		IPAERR("Input Err: ip=%d ; rlt=%d\n", ip, rlt);
		return -EINVAL;
	}

	/*
	 * SRAM memory not allocated to hash tables. Cleaning the of hash table
	 * operation not supported.
	 */
	if (rlt == IPA_RULE_HASHABLE && ipa3_ctx->ipa_fltrt_not_hashable) {
		IPADBG("Clean hashable rules not supported\n");
		return retval;
	}

	/* Up to filtering pipes we have filtering tables + 1 for coal close */
	desc = kcalloc(ipa3_ctx->ep_flt_num + 1, sizeof(struct ipa3_desc),
		GFP_KERNEL);
	if (!desc)
		return -ENOMEM;

	cmd_pyld = kcalloc(ipa3_ctx->ep_flt_num + 1,
		sizeof(struct ipahal_imm_cmd_pyld *), GFP_KERNEL);
	if (!cmd_pyld) {
		retval = -ENOMEM;
		goto free_desc;
	}

	if (ip == IPA_IP_v4) {
		if (rlt == IPA_RULE_HASHABLE) {
			lcl_addr_mem_part = IPA_MEM_PART(v4_flt_hash_ofst);
			lcl_hdr_sz = IPA_MEM_PART(v4_flt_hash_size);
		} else {
			lcl_addr_mem_part = IPA_MEM_PART(v4_flt_nhash_ofst);
			lcl_hdr_sz = IPA_MEM_PART(v4_flt_nhash_size);
		}
	} else {
		if (rlt == IPA_RULE_HASHABLE) {
			lcl_addr_mem_part = IPA_MEM_PART(v6_flt_hash_ofst);
			lcl_hdr_sz = IPA_MEM_PART(v6_flt_hash_size);
		} else {
			lcl_addr_mem_part = IPA_MEM_PART(v6_flt_nhash_ofst);
			lcl_hdr_sz = IPA_MEM_PART(v6_flt_nhash_size);
		}
	}

	retval = ipahal_flt_generate_empty_img(1, lcl_hdr_sz, lcl_hdr_sz,
		0, &mem, true);
	if (retval) {
		IPAERR("failed to generate flt single tbl empty img\n");
		goto free_cmd_pyld;
	}

	coal_ep = ipa3_get_ep_mapping(IPA_CLIENT_APPS_WAN_COAL_CONS);
	/* IC to close the coal frame before HPS Clear if coal is enabled */
	if (coal_ep != IPA_EP_NOT_ALLOCATED && !ipa3_ctx->ulso_wa) {
		u32 offset = 0;

		reg_write_coal_close.skip_pipeline_clear = false;
		reg_write_coal_close.pipeline_clear_options = IPAHAL_HPS_CLEAR;
		if (ipa3_ctx->ipa_hw_type < IPA_HW_v5_0)
			offset = ipahal_get_reg_ofst(
				IPA_AGGR_FORCE_CLOSE);
		else
			offset = ipahal_get_ep_reg_offset(
				IPA_AGGR_FORCE_CLOSE_n, coal_ep);
		reg_write_coal_close.offset = offset;
		ipahal_get_aggr_force_close_valmask(coal_ep, &valmask);
		reg_write_coal_close.value = valmask.val;
		reg_write_coal_close.value_mask = valmask.mask;
		cmd_pyld[num_cmds] = ipahal_construct_imm_cmd(
			IPA_IMM_CMD_REGISTER_WRITE,
			&reg_write_coal_close, false);
		if (!cmd_pyld[num_cmds]) {
			IPAERR("failed to construct coal close IC\n");
			retval = -ENOMEM;
			goto free_empty_img;
		}
		ipa3_init_imm_cmd_desc(&desc[num_cmds], cmd_pyld[num_cmds]);
		++num_cmds;
	}

	for (pipe_idx = 0; pipe_idx < ipa3_ctx->ipa_num_pipes; pipe_idx++) {
		if (!ipa_is_ep_support_flt(pipe_idx))
			continue;

		/*
		 * Iterating over all the filtering pipes which are either
		 * invalid but connected or connected but not configured by AP.
		 */
		if (!ipa3_ctx->ep[pipe_idx].valid ||
		    ipa3_ctx->ep[pipe_idx].skip_ep_cfg) {

			/*
			 * When coal pipe is valid send close coalescing frame
			 * command and increment the ep_flt_num accordingly.
			 */
			count = (coal_ep != IPA_EP_NOT_ALLOCATED) ? 1 : 0;
			if (num_cmds >= (ipa3_ctx->ep_flt_num + count)) {
				IPAERR("number of commands is out of range\n");
				retval = -ENOBUFS;
				goto free_empty_img;
			}

			cmd.is_read = false;
			cmd.skip_pipeline_clear = false;
			cmd.pipeline_clear_options = IPAHAL_HPS_CLEAR;
			cmd.size = mem.size;
			cmd.system_addr = mem.phys_base;
			cmd.local_addr =
				ipa3_ctx->smem_restricted_bytes +
				lcl_addr_mem_part +
				ipahal_get_hw_tbl_hdr_width() +
				flt_idx * ipahal_get_hw_tbl_hdr_width();
			cmd_pyld[num_cmds] = ipahal_construct_imm_cmd(
				IPA_IMM_CMD_DMA_SHARED_MEM, &cmd, false);
			if (!cmd_pyld[num_cmds]) {
				IPAERR("fail construct dma_shared_mem cmd\n");
				retval = -ENOMEM;
				goto free_empty_img;
			}
			ipa3_init_imm_cmd_desc(&desc[num_cmds],
				cmd_pyld[num_cmds]);
			++num_cmds;
		}

		++flt_idx;
	}

	IPADBG("Sending %d descriptors for flt tbl clearing\n", num_cmds);
	retval = ipa3_send_cmd(num_cmds, desc);
	if (retval) {
		IPAERR("failed to send immediate command (err %d)\n", retval);
		retval = -EFAULT;
	}

free_empty_img:
	ipahal_free_dma_mem(&mem);
free_cmd_pyld:
	for (index = 0; index < num_cmds; index++)
		ipahal_destroy_imm_cmd(cmd_pyld[index]);
	kfree(cmd_pyld);
free_desc:
	kfree(desc);
	return retval;
}

static int ipa3_q6_clean_q6_rt_tbls(enum ipa_ip_type ip,
	enum ipa_rule_type rlt)
{
	struct ipa3_desc *desc;
	struct ipahal_imm_cmd_dma_shared_mem cmd = {0};
	struct ipahal_imm_cmd_pyld **cmd_pyld;
	int retval = 0;
	int num_cmds = 0;
	u32 modem_rt_index_lo;
	u32 modem_rt_index_hi;
	u32 lcl_addr_mem_part;
	u32 lcl_hdr_sz;
	struct ipa_mem_buffer mem;
	struct ipahal_reg_valmask valmask;
	struct ipahal_imm_cmd_register_write reg_write_coal_close;
	int i;

	IPADBG("Entry\n");

	if ((ip >= IPA_IP_MAX) || (rlt >= IPA_RULE_TYPE_MAX)) {
		IPAERR("Input Err: ip=%d ; rlt=%d\n", ip, rlt);
		return -EINVAL;
	}

	/*
	 * SRAM memory not allocated to hash tables. Cleaning the of hash table
	 * operation not supported.
	 */
	if (rlt == IPA_RULE_HASHABLE && ipa3_ctx->ipa_fltrt_not_hashable) {
		IPADBG("Clean hashable rules not supported\n");
		return retval;
	}

	if (ip == IPA_IP_v4) {
		modem_rt_index_lo = IPA_MEM_PART(v4_modem_rt_index_lo);
		modem_rt_index_hi = IPA_MEM_PART(v4_modem_rt_index_hi);
		if (rlt == IPA_RULE_HASHABLE) {
			lcl_addr_mem_part = IPA_MEM_PART(v4_rt_hash_ofst);
			lcl_hdr_sz =  IPA_MEM_PART(v4_rt_hash_size);
		} else {
			lcl_addr_mem_part = IPA_MEM_PART(v4_rt_nhash_ofst);
			lcl_hdr_sz = IPA_MEM_PART(v4_rt_nhash_size);
		}
	} else {
		modem_rt_index_lo = IPA_MEM_PART(v6_modem_rt_index_lo);
		modem_rt_index_hi = IPA_MEM_PART(v6_modem_rt_index_hi);
		if (rlt == IPA_RULE_HASHABLE) {
			lcl_addr_mem_part = IPA_MEM_PART(v6_rt_hash_ofst);
			lcl_hdr_sz =  IPA_MEM_PART(v6_rt_hash_size);
		} else {
			lcl_addr_mem_part = IPA_MEM_PART(v6_rt_nhash_ofst);
			lcl_hdr_sz = IPA_MEM_PART(v6_rt_nhash_size);
		}
	}

	retval = ipahal_rt_generate_empty_img(
		modem_rt_index_hi - modem_rt_index_lo + 1,
		lcl_hdr_sz, lcl_hdr_sz, &mem, true);
	if (retval) {
		IPAERR("fail generate empty rt img, size %d\n", lcl_hdr_sz);
		return -ENOMEM;
	}

	desc = kcalloc(2, sizeof(struct ipa3_desc), GFP_KERNEL);
	if (!desc) {
		retval = -ENOMEM;
		goto free_empty_img;
	}

	cmd_pyld = kcalloc(2, sizeof(struct ipahal_imm_cmd_pyld *), GFP_KERNEL);
	if (!cmd_pyld) {
		retval = -ENOMEM;
		goto free_desc;
	}

	/* IC to close the coal frame before HPS Clear if coal is enabled */
	if (ipa3_get_ep_mapping(IPA_CLIENT_APPS_WAN_COAL_CONS) != -1
		&& !ipa3_ctx->ulso_wa) {
		u32 offset = 0;

		i = ipa3_get_ep_mapping(IPA_CLIENT_APPS_WAN_COAL_CONS);
		reg_write_coal_close.skip_pipeline_clear = false;
		reg_write_coal_close.pipeline_clear_options = IPAHAL_HPS_CLEAR;
		if (ipa3_ctx->ipa_hw_type < IPA_HW_v5_0)
			offset = ipahal_get_reg_ofst(
				IPA_AGGR_FORCE_CLOSE);
		else
			offset = ipahal_get_ep_reg_offset(
				IPA_AGGR_FORCE_CLOSE_n, i);
		reg_write_coal_close.offset = offset;
		ipahal_get_aggr_force_close_valmask(i, &valmask);
		reg_write_coal_close.value = valmask.val;
		reg_write_coal_close.value_mask = valmask.mask;
		cmd_pyld[num_cmds] = ipahal_construct_imm_cmd(
			IPA_IMM_CMD_REGISTER_WRITE,
			&reg_write_coal_close, false);
		if (!cmd_pyld[num_cmds]) {
			IPAERR("failed to construct coal close IC\n");
			retval = -ENOMEM;
			goto free_cmd_pyld;
		}
		ipa3_init_imm_cmd_desc(&desc[num_cmds], cmd_pyld[num_cmds]);
		++num_cmds;
	}

	cmd.is_read = false;
	cmd.skip_pipeline_clear = false;
	cmd.pipeline_clear_options = IPAHAL_HPS_CLEAR;
	cmd.size = mem.size;
	cmd.system_addr =  mem.phys_base;
	cmd.local_addr = ipa3_ctx->smem_restricted_bytes +
		lcl_addr_mem_part +
		modem_rt_index_lo * ipahal_get_hw_tbl_hdr_width();
	cmd_pyld[num_cmds] = ipahal_construct_imm_cmd(
			IPA_IMM_CMD_DMA_SHARED_MEM, &cmd, false);
	if (!cmd_pyld[num_cmds]) {
		IPAERR("failed to construct dma_shared_mem imm cmd\n");
		retval = -ENOMEM;
		goto free_cmd_pyld;
	}
	ipa3_init_imm_cmd_desc(&desc[num_cmds], cmd_pyld[num_cmds]);
	++num_cmds;

	IPADBG("Sending 1 descriptor for rt tbl clearing\n");
	retval = ipa3_send_cmd(num_cmds, desc);
	if (retval) {
		IPAERR("failed to send immediate command (err %d)\n", retval);
		retval = -EFAULT;
	}

free_cmd_pyld:
	for (i = 0; i < num_cmds; i++)
		ipahal_destroy_imm_cmd(cmd_pyld[i]);
	kfree(cmd_pyld);
free_desc:
	kfree(desc);
free_empty_img:
	ipahal_free_dma_mem(&mem);
	return retval;
}

static int ipa3_q6_clean_q6_tables(void)
{
	struct ipa3_desc *desc;
	struct ipahal_imm_cmd_pyld **cmd_pyld;
	struct ipahal_imm_cmd_register_write reg_write_cmd = {0};
	int retval = 0;
	int num_cmds = 0;
	struct ipahal_reg_valmask valmask;
	struct ipahal_imm_cmd_register_write reg_write_coal_close;
	int i;

	IPADBG("Entry\n");


	if (ipa3_q6_clean_q6_flt_tbls(IPA_IP_v4, IPA_RULE_HASHABLE)) {
		IPAERR("failed to clean q6 flt tbls (v4/hashable)\n");
		return -EFAULT;
	}
	if (ipa3_q6_clean_q6_flt_tbls(IPA_IP_v6, IPA_RULE_HASHABLE)) {
		IPAERR("failed to clean q6 flt tbls (v6/hashable)\n");
		return -EFAULT;
	}
	if (ipa3_q6_clean_q6_flt_tbls(IPA_IP_v4, IPA_RULE_NON_HASHABLE)) {
		IPAERR("failed to clean q6 flt tbls (v4/non-hashable)\n");
		return -EFAULT;
	}
	if (ipa3_q6_clean_q6_flt_tbls(IPA_IP_v6, IPA_RULE_NON_HASHABLE)) {
		IPAERR("failed to clean q6 flt tbls (v6/non-hashable)\n");
		return -EFAULT;
	}

	if (ipa3_q6_clean_q6_rt_tbls(IPA_IP_v4, IPA_RULE_HASHABLE)) {
		IPAERR("failed to clean q6 rt tbls (v4/hashable)\n");
		return -EFAULT;
	}
	if (ipa3_q6_clean_q6_rt_tbls(IPA_IP_v6, IPA_RULE_HASHABLE)) {
		IPAERR("failed to clean q6 rt tbls (v6/hashable)\n");
		return -EFAULT;
	}
	if (ipa3_q6_clean_q6_rt_tbls(IPA_IP_v4, IPA_RULE_NON_HASHABLE)) {
		IPAERR("failed to clean q6 rt tbls (v4/non-hashable)\n");
		return -EFAULT;
	}
	if (ipa3_q6_clean_q6_rt_tbls(IPA_IP_v6, IPA_RULE_NON_HASHABLE)) {
		IPAERR("failed to clean q6 rt tbls (v6/non-hashable)\n");
		return -EFAULT;
	}

	/*
	 * SRAM memory not allocated to hash tables. Cleaning the of hash table
	 * operation not supported.
	 */
	if (ipa3_ctx->ipa_fltrt_not_hashable)
		return retval;
	/* Flush rules cache */
	desc = kcalloc(2, sizeof(struct ipa3_desc), GFP_KERNEL);
	if (!desc)
		return -ENOMEM;

	cmd_pyld = kcalloc(2, sizeof(struct ipahal_imm_cmd_pyld *), GFP_KERNEL);
	if (!cmd_pyld) {
		retval = -ENOMEM;
		goto bail_desc;
	}

	/* IC to close the coal frame before HPS Clear if coal is enabled */
	if (ipa3_get_ep_mapping(IPA_CLIENT_APPS_WAN_COAL_CONS) != -1
		&& !ipa3_ctx->ulso_wa) {
		u32 offset = 0;

		i = ipa3_get_ep_mapping(IPA_CLIENT_APPS_WAN_COAL_CONS);
		reg_write_coal_close.skip_pipeline_clear = false;
		reg_write_coal_close.pipeline_clear_options = IPAHAL_HPS_CLEAR;
		if (ipa3_ctx->ipa_hw_type < IPA_HW_v5_0)
			offset = ipahal_get_reg_ofst(
				IPA_AGGR_FORCE_CLOSE);
		else
			offset = ipahal_get_ep_reg_offset(
				IPA_AGGR_FORCE_CLOSE_n, i);
		reg_write_coal_close.offset = offset;
		ipahal_get_aggr_force_close_valmask(i, &valmask);
		reg_write_coal_close.value = valmask.val;
		reg_write_coal_close.value_mask = valmask.mask;
		cmd_pyld[num_cmds] = ipahal_construct_imm_cmd(
			IPA_IMM_CMD_REGISTER_WRITE,
			&reg_write_coal_close, false);
		if (!cmd_pyld[num_cmds]) {
			IPAERR("failed to construct coal close IC\n");
			retval = -ENOMEM;
			goto free_cmd_pyld;
		}
		ipa3_init_imm_cmd_desc(&desc[num_cmds], cmd_pyld[num_cmds]);
		++num_cmds;
	}

	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v5_0) {
		struct ipahal_reg_fltrt_cache_flush flush_cache;

		memset(&flush_cache, 0, sizeof(flush_cache));
		flush_cache.flt = true;
		flush_cache.rt = true;
		ipahal_get_fltrt_cache_flush_valmask(
			&flush_cache, &valmask);
		reg_write_cmd.offset = ipahal_get_reg_ofst(
			IPA_FILT_ROUT_CACHE_FLUSH);
	} else {
		struct ipahal_reg_fltrt_hash_flush flush_hash;

		flush_hash.v4_flt = true;
		flush_hash.v4_rt = true;
		flush_hash.v6_flt = true;
		flush_hash.v6_rt = true;
		ipahal_get_fltrt_hash_flush_valmask(&flush_hash, &valmask);
		reg_write_cmd.offset = ipahal_get_reg_ofst(
			IPA_FILT_ROUT_HASH_FLUSH);
	}
	reg_write_cmd.skip_pipeline_clear = false;
	reg_write_cmd.pipeline_clear_options = IPAHAL_HPS_CLEAR;
	reg_write_cmd.value = valmask.val;
	reg_write_cmd.value_mask = valmask.mask;
	cmd_pyld[num_cmds] = ipahal_construct_imm_cmd(
		IPA_IMM_CMD_REGISTER_WRITE, &reg_write_cmd, false);
	if (!cmd_pyld[num_cmds]) {
		IPAERR("fail construct register_write imm cmd\n");
		retval = -EFAULT;
		goto free_cmd_pyld;
	}
	ipa3_init_imm_cmd_desc(&desc[num_cmds], cmd_pyld[num_cmds]);
	++num_cmds;

	IPADBG("Sending 1 descriptor for tbls flush\n");
	retval = ipa3_send_cmd(num_cmds, desc);
	if (retval) {
		IPAERR("failed to send immediate command (err %d)\n", retval);
		retval = -EFAULT;
	}

free_cmd_pyld:
	for (i = 0; i < num_cmds; i++)
		ipahal_destroy_imm_cmd(cmd_pyld[i]);
	kfree(cmd_pyld);
bail_desc:
	kfree(desc);
	IPADBG("Done - retval = %d\n", retval);
	return retval;
}

static int ipa3_q6_set_ex_path_to_apps(void)
{
	int ep_idx;
	int client_idx;
	struct ipa3_desc *desc;
	int num_descs = 0;
	int index;
	struct ipahal_imm_cmd_register_write reg_write;
	struct ipahal_imm_cmd_pyld *cmd_pyld;
	int retval;
	struct ipahal_reg_valmask valmask;
	struct ipahal_imm_cmd_register_write reg_write_coal_close;
	int i;

	desc = kcalloc(ipa3_ctx->ipa_num_pipes + 1, sizeof(struct ipa3_desc),
			GFP_KERNEL);
	if (!desc)
		return -ENOMEM;

	/* IC to close the coal frame before HPS Clear if coal is enabled */
	if (ipa3_get_ep_mapping(IPA_CLIENT_APPS_WAN_COAL_CONS) != -1
		&& !ipa3_ctx->ulso_wa) {
		u32 offset = 0;

		i = ipa3_get_ep_mapping(IPA_CLIENT_APPS_WAN_COAL_CONS);
		reg_write_coal_close.skip_pipeline_clear = false;
		reg_write_coal_close.pipeline_clear_options = IPAHAL_HPS_CLEAR;
		if (ipa3_ctx->ipa_hw_type < IPA_HW_v5_0)
			offset = ipahal_get_reg_ofst(
				IPA_AGGR_FORCE_CLOSE);
		else
			offset = ipahal_get_ep_reg_offset(
				IPA_AGGR_FORCE_CLOSE_n, i);
		reg_write_coal_close.offset = offset;
		ipahal_get_aggr_force_close_valmask(i, &valmask);
		reg_write_coal_close.value = valmask.val;
		reg_write_coal_close.value_mask = valmask.mask;
		cmd_pyld = ipahal_construct_imm_cmd(
			IPA_IMM_CMD_REGISTER_WRITE,
			&reg_write_coal_close, false);
		if (!cmd_pyld) {
			IPAERR("failed to construct coal close IC\n");
			ipa_assert();
			return -ENOMEM;
		}
		ipa3_init_imm_cmd_desc(&desc[num_descs], cmd_pyld);
		desc[num_descs].callback = ipa3_destroy_imm;
		desc[num_descs].user1 = cmd_pyld;
		++num_descs;
	}

	/* Set the exception path to AP */
	for (client_idx = 0; client_idx < IPA_CLIENT_MAX; client_idx++) {
		ep_idx = ipa3_get_ep_mapping(client_idx);
		if (ep_idx == -1 || (ep_idx >= ipa3_get_max_num_pipes()))
			continue;

		/* disable statuses for all modem controlled prod pipes */
		if (!IPA_CLIENT_IS_TEST(client_idx) &&
			(IPA_CLIENT_IS_Q6_PROD(client_idx) ||
			(IPA_CLIENT_IS_PROD(client_idx) &&
			ipa3_ctx->ep[ep_idx].valid &&
			ipa3_ctx->ep[ep_idx].skip_ep_cfg) ||
			(ipa3_ctx->ep[ep_idx].client == IPA_CLIENT_APPS_WAN_PROD
			&& ipa3_ctx->modem_cfg_emb_pipe_flt))) {
			ipa_assert_on(num_descs >= ipa3_ctx->ipa_num_pipes);

			ipa3_ctx->ep[ep_idx].status.status_en = false;
			reg_write.skip_pipeline_clear = false;
			reg_write.pipeline_clear_options =
				IPAHAL_HPS_CLEAR;
			reg_write.offset =
				ipahal_get_reg_n_ofst(IPA_ENDP_STATUS_n,
					ep_idx);
			reg_write.value = 0;
			reg_write.value_mask = ~0;
			cmd_pyld = ipahal_construct_imm_cmd(
				IPA_IMM_CMD_REGISTER_WRITE, &reg_write, false);
			if (!cmd_pyld) {
				IPAERR("fail construct register_write cmd\n");
				ipa_assert();
				return -ENOMEM;
			}

			ipa3_init_imm_cmd_desc(&desc[num_descs], cmd_pyld);
			desc[num_descs].callback = ipa3_destroy_imm;
			desc[num_descs].user1 = cmd_pyld;
			++num_descs;
		}
	}

	/* Will wait 500msecs for IPA tag process completion */
	retval = ipa3_tag_process(desc, num_descs, CLEANUP_TAG_PROCESS_TIMEOUT);
	if (retval) {
		IPAERR("TAG process failed! (error %d)\n", retval);
		/* For timeout error ipa3_destroy_imm cb will destroy user1 */
		if (retval != -ETIME) {
			for (index = 0; index < num_descs; index++)
				if (desc[index].callback)
					desc[index].callback(desc[index].user1,
						desc[index].user2);
			retval = -EINVAL;
		}
	}

	kfree(desc);

	return retval;
}

/*
 * ipa3_update_ssr_state() - updating current SSR state
 * @is_ssr:	[in] Current SSR state
 */

void ipa3_update_ssr_state(bool is_ssr)
{
	if (is_ssr)
		atomic_set(&ipa3_ctx->is_ssr, 1);
	else
		atomic_set(&ipa3_ctx->is_ssr, 0);
}

/**
 * ipa3_q6_pre_shutdown_cleanup() - A cleanup for all Q6 related configuration
 *	in IPA HW. This is performed in case of SSR.
 *
 * This is a mandatory procedure, in case one of the steps fails, the
 * AP needs to restart.
 */
void ipa3_q6_pre_shutdown_cleanup(void)
{
	IPADBG_LOW("ENTER\n");

	IPA_ACTIVE_CLIENTS_INC_SIMPLE();

	ipa3_update_ssr_state(true);

	if (ipa3_ctx->ipa_endp_delay_wa_v2)
		ipa3_q6_pipe_flow_control(true);
	else if (!ipa3_ctx->ipa_endp_delay_wa)
		ipa3_q6_pipe_delay(true);

	ipa3_q6_avoid_holb();
	if (ipa3_ctx->ipa_config_is_mhi) {
		ipa3_set_reset_client_cons_pipe_sus_holb(true,
		IPA_CLIENT_MHI_CONS);
		if (ipa3_ctx->ipa_config_is_auto)
			ipa3_set_reset_client_cons_pipe_sus_holb(true,
				IPA_CLIENT_MHI2_CONS);
	}

	if (ipa3_q6_clean_q6_tables()) {
		IPAERR("Failed to clean Q6 tables\n");
		/*
		 * Indicates IPA hardware is stalled, unexpected
		 * hardware state.
		 */
		ipa_assert();
	}
	if (ipa3_q6_set_ex_path_to_apps()) {
		IPAERR("Failed to redirect exceptions to APPS\n");
		/*
		 * Indicates IPA hardware is stalled, unexpected
		 * hardware state.
		 */
		ipa_assert();
	}

	if(ipa3_ctx->ipa_hw_type != IPA_HW_v5_2) {
		/* Remove delay from Q6 PRODs to avoid pending descriptors
	 	* on pipe reset procedure
	 	*/

		if (ipa3_ctx->ipa_endp_delay_wa_v2) {
			ipa3_q6_pipe_flow_control(false);
			ipa3_set_reset_client_prod_pipe_delay(true,
				IPA_CLIENT_USB_PROD);
		} else if (!ipa3_ctx->ipa_endp_delay_wa) {
			ipa3_q6_pipe_delay(false);
			ipa3_set_reset_client_prod_pipe_delay(true,
				IPA_CLIENT_USB_PROD);
			if (ipa3_ctx->ipa_config_is_auto)
				ipa3_set_reset_client_prod_pipe_delay(true,
					IPA_CLIENT_USB2_PROD);
		} else {
			ipa3_start_stop_client_prod_gsi_chnl(IPA_CLIENT_USB_PROD,
						false);
			if (ipa3_ctx->ipa_config_is_auto)
				ipa3_start_stop_client_prod_gsi_chnl(
					IPA_CLIENT_USB2_PROD, false);
		}
	}

	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
	IPADBG_LOW("Exit with success\n");
}

/*
 * ipa3_q6_post_shutdown_cleanup() - As part of this cleanup
 * check if GSI channel related to Q6 producer client is empty.
 *
 * Q6 GSI channel emptiness is needed to garantee no descriptors with invalid
 *  info are injected into IPA RX from IPA_IF, while modem is restarting.
 */
void ipa3_q6_post_shutdown_cleanup(void)
{
	int client_idx;
	int ep_idx;
	bool prod = false;

	IPADBG_LOW("ENTER\n");

	IPA_ACTIVE_CLIENTS_INC_SIMPLE();

	/* Handle the issue where SUSPEND was removed for some reason */
	ipa3_q6_avoid_holb();

	/* halt both prod and cons channels starting at IPAv4 */
	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_0) {
		prod = true;
		ipa3_halt_q6_gsi_channels(prod);
		if (ipa3_ctx->ipa_hw_type == IPA_HW_v5_2) {
			if (ipa3_ctx->ipa_endp_delay_wa_v2) {
				ipa3_q6_pipe_flow_control(false);
				ipa3_set_reset_client_prod_pipe_delay(true,
					IPA_CLIENT_USB_PROD);
			} else if (!ipa3_ctx->ipa_endp_delay_wa) {
				ipa3_q6_pipe_delay(false);
				ipa3_set_reset_client_prod_pipe_delay(true,
				IPA_CLIENT_USB_PROD);
				if (ipa3_ctx->ipa_config_is_auto)
					ipa3_set_reset_client_prod_pipe_delay(true,
						IPA_CLIENT_USB2_PROD);
			} else {
				ipa3_start_stop_client_prod_gsi_chnl(IPA_CLIENT_USB_PROD,
							false);
				if (ipa3_ctx->ipa_config_is_auto)
					ipa3_start_stop_client_prod_gsi_chnl(
						IPA_CLIENT_USB2_PROD, false);
			}
		}
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		IPADBG("Exit without consumer check\n");
		return;
	}

	ipa3_halt_q6_gsi_channels(prod);

	if (!ipa3_ctx->uc_ctx.uc_loaded) {
		IPAERR("uC is not loaded. Skipping\n");
		return;
	}

	for (client_idx = 0; client_idx < IPA_CLIENT_MAX; client_idx++)
		if (IPA_CLIENT_IS_Q6_PROD(client_idx)) {
			ep_idx = ipa3_get_ep_mapping(client_idx);
			if (ep_idx == -1)
				continue;

			if (ipa3_uc_is_gsi_channel_empty(client_idx)) {
				IPAERR("fail to validate Q6 ch emptiness %d\n",
					client_idx);
				/*
				 * Indicates GSI hardware is stalled, unexpected
				 * hardware state.
				 * Remove bug for adb reboot issue.
				 */
			}
		}

	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
	IPADBG_LOW("Exit with success\n");
}

/**
 * ipa3_q6_pre_powerup_cleanup() - A cleanup routine for pheripheral
 * configuration in IPA HW. This is performed in case of SSR.
 *
 * This is a mandatory procedure, in case one of the steps fails, the
 * AP needs to restart.
 */
void ipa3_q6_pre_powerup_cleanup(void)
{
	IPADBG_LOW("ENTER\n");

	IPA_ACTIVE_CLIENTS_INC_SIMPLE();

	if (ipa3_ctx->ipa_config_is_mhi) {
		if (!ipa3_ctx->ipa_endp_delay_wa) {
			ipa3_set_reset_client_prod_pipe_delay(true,
				IPA_CLIENT_MHI_PROD);
			if (ipa3_ctx->ipa_config_is_auto)
				ipa3_set_reset_client_prod_pipe_delay(true,
					IPA_CLIENT_MHI2_PROD);
		}
	}

	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
	IPADBG_LOW("Exit with success\n");
}

/*
 * ipa3_client_prod_post_shutdown_cleanup () - As part of this function
 * set end point delay client producer pipes and starting corresponding
 * gsi channels
 */

void ipa3_client_prod_post_shutdown_cleanup(void)
{
	IPADBG_LOW("ENTER\n");

	IPA_ACTIVE_CLIENTS_INC_SIMPLE();

	ipa3_set_reset_client_prod_pipe_delay(true,
				IPA_CLIENT_USB_PROD);
	ipa3_start_stop_client_prod_gsi_chnl(IPA_CLIENT_USB_PROD, true);

	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
	IPADBG_LOW("Exit with success\n");
}

static inline void ipa3_sram_set_canary(u32 *sram_mmio, int offset)
{
	/* Set 4 bytes of CANARY before the offset */
	sram_mmio[(offset - 4) / 4] = IPA_MEM_CANARY_VAL;
}

/**
 * _ipa_init_sram_v3() - Initialize IPA local SRAM.
 *
 * Return codes: 0 for success, negative value for failure
 */
int _ipa_init_sram_v3(void)
{
	u32 *ipa_sram_mmio;
	unsigned long phys_addr;

	IPADBG(
	    "ipa_wrapper_base(0x%08X) ipa_reg_base_ofst(0x%08X) IPA_SW_AREA_RAM_DIRECT_ACCESS_n(0x%08X) smem_restricted_bytes(0x%08X) smem_sz(0x%08X)\n",
	    ipa3_ctx->ipa_wrapper_base,
	    ipa3_ctx->ctrl->ipa_reg_base_ofst,
	    ipahal_get_reg_n_ofst(
		IPA_SW_AREA_RAM_DIRECT_ACCESS_n,
		ipa3_ctx->smem_restricted_bytes / 4),
	    ipa3_ctx->smem_restricted_bytes,
	    ipa3_ctx->smem_sz);

	phys_addr = ipa3_ctx->ipa_wrapper_base +
		ipa3_ctx->ctrl->ipa_reg_base_ofst +
		ipahal_get_reg_n_ofst(IPA_SW_AREA_RAM_DIRECT_ACCESS_n,
			ipa3_ctx->smem_restricted_bytes / 4);

	ipa_sram_mmio = ioremap(phys_addr, ipa3_ctx->smem_sz);
	if (!ipa_sram_mmio) {
		IPAERR("fail to ioremap IPA SRAM\n");
		return -ENOMEM;
	}

	/* Consult with ipa_i.h on the location of the CANARY values */
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v4_flt_hash_ofst) - 4);
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v4_flt_hash_ofst));
	ipa3_sram_set_canary(ipa_sram_mmio,
		IPA_MEM_PART(v4_flt_nhash_ofst) - 4);
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v4_flt_nhash_ofst));
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v6_flt_hash_ofst) - 4);
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v6_flt_hash_ofst));
	ipa3_sram_set_canary(ipa_sram_mmio,
		IPA_MEM_PART(v6_flt_nhash_ofst) - 4);
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v6_flt_nhash_ofst));
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v4_rt_hash_ofst) - 4);
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v4_rt_hash_ofst));
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v4_rt_nhash_ofst) - 4);
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v4_rt_nhash_ofst));
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v6_rt_hash_ofst) - 4);
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v6_rt_hash_ofst));
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v6_rt_nhash_ofst) - 4);
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(v6_rt_nhash_ofst));
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(modem_hdr_ofst) - 4);
	ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(modem_hdr_ofst));
	ipa3_sram_set_canary(ipa_sram_mmio,
		IPA_MEM_PART(modem_hdr_proc_ctx_ofst) - 4);
	ipa3_sram_set_canary(ipa_sram_mmio,
		IPA_MEM_PART(modem_hdr_proc_ctx_ofst));
	if (ipa_get_hw_type_internal() >= IPA_HW_v4_5
		&& ipa_get_hw_type_internal() < IPA_HW_v5_0) {
		/* 4.5, 4.7, 4.9, 4.11 */
		ipa3_sram_set_canary(ipa_sram_mmio,
			IPA_MEM_PART(nat_tbl_ofst) - 12);
	}

	if (ipa_get_hw_type_internal() >= IPA_HW_v4_0) {
		if (ipa_get_hw_type_internal() < IPA_HW_v4_5) {
			/* 4.0, 4.1, 4.2 */
			ipa3_sram_set_canary(ipa_sram_mmio,
				IPA_MEM_PART(pdn_config_ofst) - 4);
			ipa3_sram_set_canary(ipa_sram_mmio,
				IPA_MEM_PART(pdn_config_ofst));
			ipa3_sram_set_canary(ipa_sram_mmio,
				IPA_MEM_PART(stats_quota_q6_ofst) - 4);
			ipa3_sram_set_canary(ipa_sram_mmio,
				IPA_MEM_PART(stats_quota_q6_ofst));
		} else if (ipa_get_hw_type_internal() < IPA_HW_v5_0) {
			/* 4.5, 4.7, 4.11 */
			ipa3_sram_set_canary(ipa_sram_mmio,
				IPA_MEM_PART(stats_quota_q6_ofst) - 12);
		} else {
			/* 5.0 and above */
			ipa3_sram_set_canary(ipa_sram_mmio,
				IPA_MEM_PART(stats_quota_q6_ofst) - 4);
			ipa3_sram_set_canary(ipa_sram_mmio,
				IPA_MEM_PART(stats_quota_q6_ofst));
		}
	}

	/* all excluding 3.5.1, 4.0, 4.1, 4.2 */
	if (ipa_get_hw_type_internal() <= IPA_HW_v3_5 ||
		ipa_get_hw_type_internal() >= IPA_HW_v4_5) {
		ipa3_sram_set_canary(ipa_sram_mmio,
			IPA_MEM_PART(modem_ofst) - 4);
		ipa3_sram_set_canary(ipa_sram_mmio, IPA_MEM_PART(modem_ofst));
	}

	if (ipa_get_hw_type_internal() == IPA_HW_v5_0) {
		ipa3_sram_set_canary(ipa_sram_mmio,
			IPA_MEM_PART(apps_v4_flt_nhash_ofst) - 4);
		ipa3_sram_set_canary(ipa_sram_mmio,
			IPA_MEM_PART(apps_v4_flt_nhash_ofst));
		ipa3_sram_set_canary(ipa_sram_mmio,
			IPA_MEM_PART(stats_fnr_ofst) - 4);
		ipa3_sram_set_canary(ipa_sram_mmio,
			IPA_MEM_PART(stats_fnr_ofst));
	}

	if (ipa_get_hw_type_internal() >= IPA_HW_v5_0) {
		ipa3_sram_set_canary(ipa_sram_mmio,
			IPA_MEM_PART(pdn_config_ofst - 4));
		ipa3_sram_set_canary(ipa_sram_mmio,
			IPA_MEM_PART(pdn_config_ofst));
	} else {
		ipa3_sram_set_canary(ipa_sram_mmio,
			(ipa_get_hw_type_internal() >= IPA_HW_v3_5) ?
			IPA_MEM_PART(uc_descriptor_ram_ofst) :
			IPA_MEM_PART(end_ofst));
	}

	iounmap(ipa_sram_mmio);

	return 0;
}

/**
 * _ipa_init_hdr_v3_0() - Initialize IPA header block.
 *
 * Return codes: 0 for success, negative value for failure
 */
int _ipa_init_hdr_v3_0(void)
{
	struct ipa3_desc hdr_init_desc;
	struct ipa3_desc dma_cmd_desc[2];
	struct ipa_mem_buffer mem;
	struct ipahal_imm_cmd_hdr_init_local cmd = {0};
	struct ipahal_imm_cmd_pyld *hdr_init_cmd_payload;
	struct ipahal_imm_cmd_pyld *cmd_pyld[2];
	struct ipahal_imm_cmd_dma_shared_mem dma_cmd = { 0 };
	struct ipahal_reg_valmask valmask;
	struct ipahal_imm_cmd_register_write reg_write_coal_close;
	int num_cmds = 0;
	int i;

	mem.size = IPA_MEM_PART(modem_hdr_size) + IPA_MEM_PART(apps_hdr_size);
	mem.base = dma_alloc_coherent(ipa3_ctx->pdev, mem.size, &mem.phys_base,
		GFP_KERNEL);
	if (!mem.base) {
		IPAERR("fail to alloc DMA buff of size %d\n", mem.size);
		return -ENOMEM;
	}

	cmd.hdr_table_addr = mem.phys_base;
	cmd.size_hdr_table = mem.size;
	cmd.hdr_addr = ipa3_ctx->smem_restricted_bytes +
		IPA_MEM_PART(modem_hdr_ofst);
	hdr_init_cmd_payload = ipahal_construct_imm_cmd(
		IPA_IMM_CMD_HDR_INIT_LOCAL, &cmd, false);
	if (!hdr_init_cmd_payload) {
		IPAERR("fail to construct hdr_init_local imm cmd\n");
		dma_free_coherent(ipa3_ctx->pdev,
			mem.size, mem.base,
			mem.phys_base);
		return -EFAULT;
	}
	ipa3_init_imm_cmd_desc(&hdr_init_desc, hdr_init_cmd_payload);
	IPA_DUMP_BUFF(mem.base, mem.phys_base, mem.size);

	if (ipa3_send_cmd(1, &hdr_init_desc)) {
		IPAERR("fail to send immediate command\n");
		ipahal_destroy_imm_cmd(hdr_init_cmd_payload);
		dma_free_coherent(ipa3_ctx->pdev,
			mem.size, mem.base,
			mem.phys_base);
		return -EFAULT;
	}

	ipahal_destroy_imm_cmd(hdr_init_cmd_payload);
	dma_free_coherent(ipa3_ctx->pdev, mem.size, mem.base, mem.phys_base);

	/* IC to close the coal frame before HPS Clear if coal is enabled */
	if (ipa3_get_ep_mapping(IPA_CLIENT_APPS_WAN_COAL_CONS) != -1
		&& !ipa3_ctx->ulso_wa) {
		u32 offset = 0;

		i = ipa3_get_ep_mapping(IPA_CLIENT_APPS_WAN_COAL_CONS);
		reg_write_coal_close.skip_pipeline_clear = false;
		reg_write_coal_close.pipeline_clear_options = IPAHAL_HPS_CLEAR;
		if (ipa3_ctx->ipa_hw_type < IPA_HW_v5_0)
			offset = ipahal_get_reg_ofst(
				IPA_AGGR_FORCE_CLOSE);
		else
			offset = ipahal_get_ep_reg_offset(
				IPA_AGGR_FORCE_CLOSE_n, i);
		reg_write_coal_close.offset = offset;
		ipahal_get_aggr_force_close_valmask(i, &valmask);
		reg_write_coal_close.value = valmask.val;
		reg_write_coal_close.value_mask = valmask.mask;
		cmd_pyld[num_cmds] = ipahal_construct_imm_cmd(
			IPA_IMM_CMD_REGISTER_WRITE,
			&reg_write_coal_close, false);
		if (!cmd_pyld[num_cmds]) {
			IPAERR("failed to construct coal close IC\n");
			return -ENOMEM;
		}
		ipa3_init_imm_cmd_desc(&dma_cmd_desc[num_cmds],
			cmd_pyld[num_cmds]);
		++num_cmds;
	}

	mem.size = IPA_MEM_PART(modem_hdr_proc_ctx_size) +
		IPA_MEM_PART(apps_hdr_proc_ctx_size);
	mem.base = dma_alloc_coherent(ipa3_ctx->pdev, mem.size, &mem.phys_base,
		GFP_KERNEL);
	if (!mem.base) {
		IPAERR("fail to alloc DMA buff of size %d\n", mem.size);
		return -ENOMEM;
	}

	dma_cmd.is_read = false;
	dma_cmd.skip_pipeline_clear = false;
	dma_cmd.pipeline_clear_options = IPAHAL_HPS_CLEAR;
	dma_cmd.system_addr = mem.phys_base;
	dma_cmd.local_addr = ipa3_ctx->smem_restricted_bytes +
		IPA_MEM_PART(modem_hdr_proc_ctx_ofst);
	dma_cmd.size = mem.size;
	cmd_pyld[num_cmds] = ipahal_construct_imm_cmd(
		IPA_IMM_CMD_DMA_SHARED_MEM, &dma_cmd, false);
	if (!cmd_pyld[num_cmds]) {
		IPAERR("fail to construct dma_shared_mem imm\n");
		dma_free_coherent(ipa3_ctx->pdev,
			mem.size, mem.base,
			mem.phys_base);
		return -ENOMEM;
	}
	ipa3_init_imm_cmd_desc(&dma_cmd_desc[num_cmds], cmd_pyld[num_cmds]);
	++num_cmds;
	IPA_DUMP_BUFF(mem.base, mem.phys_base, mem.size);

	if (ipa3_send_cmd(num_cmds, dma_cmd_desc)) {
		IPAERR("fail to send immediate command\n");
		for (i = 0; i < num_cmds; i++)
			ipahal_destroy_imm_cmd(cmd_pyld[i]);
		dma_free_coherent(ipa3_ctx->pdev,
			mem.size,
			mem.base,
			mem.phys_base);
		return -EBUSY;
	}
	for (i = 0; i < num_cmds; i++)
		ipahal_destroy_imm_cmd(cmd_pyld[i]);

	ipahal_write_reg(IPA_LOCAL_PKT_PROC_CNTXT_BASE, dma_cmd.local_addr);

	dma_free_coherent(ipa3_ctx->pdev, mem.size, mem.base, mem.phys_base);

	return 0;
}

/**
 * _ipa_init_rt4_v3() - Initialize IPA routing block for IPv4.
 *
 * Return codes: 0 for success, negative value for failure
 */
int _ipa_init_rt4_v3(void)
{
	struct ipa3_desc desc;
	struct ipa_mem_buffer mem;
	struct ipahal_imm_cmd_ip_v4_routing_init v4_cmd;
	struct ipahal_imm_cmd_pyld *cmd_pyld;
	int i;
	int rc = 0;

	for (i = IPA_MEM_PART(v4_modem_rt_index_lo);
		i <= IPA_MEM_PART(v4_modem_rt_index_hi);
		i++)
		ipa3_ctx->rt_idx_bitmap[IPA_IP_v4] |= (1 << i);
	IPADBG("v4 rt bitmap 0x%lx\n", ipa3_ctx->rt_idx_bitmap[IPA_IP_v4]);

	rc = ipahal_rt_generate_empty_img(IPA_MEM_PART(v4_rt_num_index),
		IPA_MEM_PART(v4_rt_hash_size), IPA_MEM_PART(v4_rt_nhash_size),
		&mem, false);
	if (rc) {
		IPAERR("fail generate empty v4 rt img\n");
		return rc;
	}

	/*
	 * SRAM memory not allocated to hash tables. Initializing/Sending
	 * command to hash tables(filer/routing) operation not supported.
	 */
	if (ipa3_ctx->ipa_fltrt_not_hashable) {
		v4_cmd.hash_rules_addr = 0;
		v4_cmd.hash_rules_size = 0;
		v4_cmd.hash_local_addr = 0;
	} else {
		v4_cmd.hash_rules_addr = mem.phys_base;
		v4_cmd.hash_rules_size = mem.size;
		v4_cmd.hash_local_addr = ipa3_ctx->smem_restricted_bytes +
			IPA_MEM_PART(v4_rt_hash_ofst);
	}

	v4_cmd.nhash_rules_addr = mem.phys_base;
	v4_cmd.nhash_rules_size = mem.size;
	v4_cmd.nhash_local_addr = ipa3_ctx->smem_restricted_bytes +
		IPA_MEM_PART(v4_rt_nhash_ofst);
	IPADBG("putting hashable routing IPv4 rules to phys 0x%x\n",
				v4_cmd.hash_local_addr);
	IPADBG("putting non-hashable routing IPv4 rules to phys 0x%x\n",
				v4_cmd.nhash_local_addr);
	cmd_pyld = ipahal_construct_imm_cmd(
		IPA_IMM_CMD_IP_V4_ROUTING_INIT, &v4_cmd, false);
	if (!cmd_pyld) {
		IPAERR("fail construct ip_v4_rt_init imm cmd\n");
		rc = -EPERM;
		goto free_mem;
	}

	ipa3_init_imm_cmd_desc(&desc, cmd_pyld);
	IPA_DUMP_BUFF(mem.base, mem.phys_base, mem.size);

	if (ipa3_send_cmd(1, &desc)) {
		IPAERR("fail to send immediate command\n");
		rc = -EFAULT;
	}

	ipahal_destroy_imm_cmd(cmd_pyld);

free_mem:
	ipahal_free_dma_mem(&mem);
	return rc;
}

/**
 * _ipa_init_rt6_v3() - Initialize IPA routing block for IPv6.
 *
 * Return codes: 0 for success, negative value for failure
 */
int _ipa_init_rt6_v3(void)
{
	struct ipa3_desc desc;
	struct ipa_mem_buffer mem;
	struct ipahal_imm_cmd_ip_v6_routing_init v6_cmd;
	struct ipahal_imm_cmd_pyld *cmd_pyld;
	int i;
	int rc = 0;

	for (i = IPA_MEM_PART(v6_modem_rt_index_lo);
		i <= IPA_MEM_PART(v6_modem_rt_index_hi);
		i++)
		ipa3_ctx->rt_idx_bitmap[IPA_IP_v6] |= (1 << i);
	IPADBG("v6 rt bitmap 0x%lx\n", ipa3_ctx->rt_idx_bitmap[IPA_IP_v6]);

	rc = ipahal_rt_generate_empty_img(IPA_MEM_PART(v6_rt_num_index),
		IPA_MEM_PART(v6_rt_hash_size), IPA_MEM_PART(v6_rt_nhash_size),
		&mem, false);
	if (rc) {
		IPAERR("fail generate empty v6 rt img\n");
		return rc;
	}

	/*
	 * SRAM memory not allocated to hash tables. Initializing/Sending
	 * command to hash tables(filer/routing) operation not supported.
	 */
	if (ipa3_ctx->ipa_fltrt_not_hashable) {
		v6_cmd.hash_rules_addr = 0;
		v6_cmd.hash_rules_size = 0;
		v6_cmd.hash_local_addr = 0;
	} else {
		v6_cmd.hash_rules_addr = mem.phys_base;
		v6_cmd.hash_rules_size = mem.size;
		v6_cmd.hash_local_addr = ipa3_ctx->smem_restricted_bytes +
			IPA_MEM_PART(v6_rt_hash_ofst);
	}

	v6_cmd.nhash_rules_addr = mem.phys_base;
	v6_cmd.nhash_rules_size = mem.size;
	v6_cmd.nhash_local_addr = ipa3_ctx->smem_restricted_bytes +
		IPA_MEM_PART(v6_rt_nhash_ofst);
	IPADBG("putting hashable routing IPv6 rules to phys 0x%x\n",
				v6_cmd.hash_local_addr);
	IPADBG("putting non-hashable routing IPv6 rules to phys 0x%x\n",
				v6_cmd.nhash_local_addr);
	cmd_pyld = ipahal_construct_imm_cmd(
		IPA_IMM_CMD_IP_V6_ROUTING_INIT, &v6_cmd, false);
	if (!cmd_pyld) {
		IPAERR("fail construct ip_v6_rt_init imm cmd\n");
		rc = -EPERM;
		goto free_mem;
	}

	ipa3_init_imm_cmd_desc(&desc, cmd_pyld);
	IPA_DUMP_BUFF(mem.base, mem.phys_base, mem.size);

	if (ipa3_send_cmd(1, &desc)) {
		IPAERR("fail to send immediate command\n");
		rc = -EFAULT;
	}

	ipahal_destroy_imm_cmd(cmd_pyld);

free_mem:
	ipahal_free_dma_mem(&mem);
	return rc;
}

/**
 * _ipa_init_flt4_v3() - Initialize IPA filtering block for IPv4.
 *
 * Return codes: 0 for success, negative value for failure
 */
int _ipa_init_flt4_v3(void)
{
	struct ipa3_desc desc;
	struct ipa_mem_buffer mem;
	struct ipahal_imm_cmd_ip_v4_filter_init v4_cmd;
	struct ipahal_imm_cmd_pyld *cmd_pyld;
	int rc;

	rc = ipahal_flt_generate_empty_img(ipa3_ctx->ep_flt_num,
		IPA_MEM_PART(v4_flt_hash_size),
		IPA_MEM_PART(v4_flt_nhash_size), ipa3_ctx->ep_flt_bitmap,
		&mem, false);
	if (rc) {
		IPAERR("fail generate empty v4 flt img\n");
		return rc;
	}

	/*
	 * SRAM memory not allocated to hash tables. Initializing/Sending
	 * command to hash tables(filer/routing) operation not supported.
	 */
	if (ipa3_ctx->ipa_fltrt_not_hashable) {
		v4_cmd.hash_rules_addr = 0;
		v4_cmd.hash_rules_size = 0;
		v4_cmd.hash_local_addr = 0;
	} else {
		v4_cmd.hash_rules_addr = mem.phys_base;
		v4_cmd.hash_rules_size = mem.size;
		v4_cmd.hash_local_addr = ipa3_ctx->smem_restricted_bytes +
			IPA_MEM_PART(v4_flt_hash_ofst);
	}

	v4_cmd.nhash_rules_addr = mem.phys_base;
	v4_cmd.nhash_rules_size = mem.size;
	v4_cmd.nhash_local_addr = ipa3_ctx->smem_restricted_bytes +
		IPA_MEM_PART(v4_flt_nhash_ofst);
	IPADBG("putting hashable filtering IPv4 rules to phys 0x%x\n",
				v4_cmd.hash_local_addr);
	IPADBG("putting non-hashable filtering IPv4 rules to phys 0x%x\n",
				v4_cmd.nhash_local_addr);
	cmd_pyld = ipahal_construct_imm_cmd(
		IPA_IMM_CMD_IP_V4_FILTER_INIT, &v4_cmd, false);
	if (!cmd_pyld) {
		IPAERR("fail construct ip_v4_flt_init imm cmd\n");
		rc = -EPERM;
		goto free_mem;
	}

	ipa3_init_imm_cmd_desc(&desc, cmd_pyld);
	IPA_DUMP_BUFF(mem.base, mem.phys_base, mem.size);

	if (ipa3_send_cmd(1, &desc)) {
		IPAERR("fail to send immediate command\n");
		rc = -EFAULT;
	}

	ipahal_destroy_imm_cmd(cmd_pyld);

free_mem:
	ipahal_free_dma_mem(&mem);
	return rc;
}

/**
 * _ipa_init_flt6_v3() - Initialize IPA filtering block for IPv6.
 *
 * Return codes: 0 for success, negative value for failure
 */
int _ipa_init_flt6_v3(void)
{
	struct ipa3_desc desc;
	struct ipa_mem_buffer mem;
	struct ipahal_imm_cmd_ip_v6_filter_init v6_cmd;
	struct ipahal_imm_cmd_pyld *cmd_pyld;
	int rc;

	rc = ipahal_flt_generate_empty_img(ipa3_ctx->ep_flt_num,
		IPA_MEM_PART(v6_flt_hash_size),
		IPA_MEM_PART(v6_flt_nhash_size), ipa3_ctx->ep_flt_bitmap,
		&mem, false);
	if (rc) {
		IPAERR("fail generate empty v6 flt img\n");
		return rc;
	}

	/*
	 * SRAM memory not allocated to hash tables. Initializing/Sending
	 * command to hash tables(filer/routing) operation not supported.
	 */
	if (ipa3_ctx->ipa_fltrt_not_hashable) {
		v6_cmd.hash_rules_addr = 0;
		v6_cmd.hash_rules_size = 0;
		v6_cmd.hash_local_addr = 0;
	} else {
		v6_cmd.hash_rules_addr = mem.phys_base;
		v6_cmd.hash_rules_size = mem.size;
		v6_cmd.hash_local_addr = ipa3_ctx->smem_restricted_bytes +
			IPA_MEM_PART(v6_flt_hash_ofst);
	}

	v6_cmd.nhash_rules_addr = mem.phys_base;
	v6_cmd.nhash_rules_size = mem.size;
	v6_cmd.nhash_local_addr = ipa3_ctx->smem_restricted_bytes +
		IPA_MEM_PART(v6_flt_nhash_ofst);
	IPADBG("putting hashable filtering IPv6 rules to phys 0x%x\n",
				v6_cmd.hash_local_addr);
	IPADBG("putting non-hashable filtering IPv6 rules to phys 0x%x\n",
				v6_cmd.nhash_local_addr);

	cmd_pyld = ipahal_construct_imm_cmd(
		IPA_IMM_CMD_IP_V6_FILTER_INIT, &v6_cmd, false);
	if (!cmd_pyld) {
		IPAERR("fail construct ip_v6_flt_init imm cmd\n");
		rc = -EPERM;
		goto free_mem;
	}

	ipa3_init_imm_cmd_desc(&desc, cmd_pyld);
	IPA_DUMP_BUFF(mem.base, mem.phys_base, mem.size);

	if (ipa3_send_cmd(1, &desc)) {
		IPAERR("fail to send immediate command\n");
		rc = -EFAULT;
	}

	ipahal_destroy_imm_cmd(cmd_pyld);

free_mem:
	ipahal_free_dma_mem(&mem);
	return rc;
}

static int ipa3_setup_flt_hash_tuple(void)
{
	int pipe_idx;
	struct ipahal_reg_hash_tuple tuple;

	memset(&tuple, 0, sizeof(struct ipahal_reg_hash_tuple));

	for (pipe_idx = 0; pipe_idx < ipa3_ctx->ipa_num_pipes ; pipe_idx++) {
		if (!ipa_is_ep_support_flt(pipe_idx))
			continue;

		if (ipa_is_modem_pipe(pipe_idx))
			continue;

		if (ipa3_set_flt_tuple_mask(pipe_idx, &tuple)) {
			IPAERR("failed to setup pipe %d flt tuple\n", pipe_idx);
			return -EFAULT;
		}
	}

	return 0;
}

static int ipa3_setup_rt_hash_tuple(void)
{
	int tbl_idx;
	struct ipahal_reg_hash_tuple tuple;

	memset(&tuple, 0, sizeof(struct ipahal_reg_hash_tuple));

	for (tbl_idx = 0;
		tbl_idx < max(IPA_MEM_PART(v6_rt_num_index),
		IPA_MEM_PART(v4_rt_num_index));
		tbl_idx++) {

		if (tbl_idx >= IPA_MEM_PART(v4_modem_rt_index_lo) &&
			tbl_idx <= IPA_MEM_PART(v4_modem_rt_index_hi))
			continue;

		if (tbl_idx >= IPA_MEM_PART(v6_modem_rt_index_lo) &&
			tbl_idx <= IPA_MEM_PART(v6_modem_rt_index_hi))
			continue;

		if (ipa3_set_rt_tuple_mask(tbl_idx, &tuple)) {
			IPAERR("failed to setup tbl %d rt tuple\n", tbl_idx);
			return -EFAULT;
		}
	}

	return 0;
}

static int ipa3_setup_apps_pipes(void)
{
	struct ipa_sys_connect_params sys_in;
	int result = 0;

	if (ipa3_ctx->gsi_ch20_wa) {
		IPADBG("Allocating GSI physical channel 20\n");
		result = ipa_gsi_ch20_wa();
		if (result) {
			IPAERR("ipa_gsi_ch20_wa failed %d\n", result);
			goto fail_ch20_wa;
		}
	}

	/* allocate the common PROD event ring */
	if (ipa3_alloc_common_event_ring()) {
		IPAERR("ipa3_alloc_common_event_ring failed.\n");
		result = -EPERM;
		goto fail_ch20_wa;
	}

	/* CMD OUT (AP->IPA) */
	memset(&sys_in, 0, sizeof(struct ipa_sys_connect_params));
	sys_in.client = IPA_CLIENT_APPS_CMD_PROD;
	sys_in.desc_fifo_sz = IPA_SYS_DESC_FIFO_SZ;
	sys_in.ipa_ep_cfg.mode.mode = IPA_DMA;
	sys_in.ipa_ep_cfg.mode.dst = IPA_CLIENT_APPS_LAN_CONS;
	if (ipa3_setup_sys_pipe(&sys_in, &ipa3_ctx->clnt_hdl_cmd)) {
		IPAERR(":setup sys pipe (APPS_CMD_PROD) failed.\n");
		result = -EPERM;
		goto fail_ch20_wa;
	}
	IPADBG("Apps to IPA cmd pipe is connected\n");

	IPADBG("Will initialize SRAM\n");
	ipa3_ctx->ctrl->ipa_init_sram();
	IPADBG("SRAM initialized\n");

	IPADBG("Will initialize HDR\n");
	ipa3_ctx->ctrl->ipa_init_hdr();
	IPADBG("HDR initialized\n");

	IPADBG("Will initialize V4 RT\n");
	ipa3_ctx->ctrl->ipa_init_rt4();
	IPADBG("V4 RT initialized\n");

	IPADBG("Will initialize V6 RT\n");
	ipa3_ctx->ctrl->ipa_init_rt6();
	IPADBG("V6 RT initialized\n");

	IPADBG("Will initialize V4 FLT\n");
	ipa3_ctx->ctrl->ipa_init_flt4();
	IPADBG("V4 FLT initialized\n");

	IPADBG("Will initialize V6 FLT\n");
	ipa3_ctx->ctrl->ipa_init_flt6();
	IPADBG("V6 FLT initialized\n");

	if (!ipa3_ctx->ipa_fltrt_not_hashable) {
		if (ipa3_setup_flt_hash_tuple()) {
			IPAERR(":fail to configure flt hash tuple\n");
			result = -EPERM;
			goto fail_flt_hash_tuple;
		}
		IPADBG("flt hash tuple is configured\n");

		if (ipa3_setup_rt_hash_tuple()) {
			IPAERR(":fail to configure rt hash tuple\n");
			result = -EPERM;
			goto fail_flt_hash_tuple;
		}
		IPADBG("rt hash tuple is configured\n");
	}
	if (ipa3_setup_exception_path()) {
		IPAERR(":fail to setup excp path\n");
		result = -EPERM;
		goto fail_flt_hash_tuple;
	}
	IPADBG("Exception path was successfully set");

	if (ipa3_setup_dflt_rt_tables()) {
		IPAERR(":fail to setup dflt routes\n");
		result = -EPERM;
		goto fail_flt_hash_tuple;
	}
	IPADBG("default routing was set\n");

	/* LAN IN (IPA->AP) */
	memset(&sys_in, 0, sizeof(struct ipa_sys_connect_params));
	sys_in.client = IPA_CLIENT_APPS_LAN_CONS;
	sys_in.desc_fifo_sz = IPA_SYS_DESC_FIFO_SZ;
	sys_in.notify = ipa3_lan_rx_cb;
	sys_in.priv = NULL;
	if (ipa3_ctx->lan_rx_napi_enable)
		sys_in.napi_obj = &ipa3_ctx->napi_lan_rx;
	sys_in.ipa_ep_cfg.hdr.hdr_len = IPA_LAN_RX_HEADER_LENGTH;
	sys_in.ipa_ep_cfg.hdr_ext.hdr_little_endian = false;
	sys_in.ipa_ep_cfg.hdr_ext.hdr_total_len_or_pad_valid = true;
	sys_in.ipa_ep_cfg.hdr_ext.hdr_total_len_or_pad = IPA_HDR_PAD;
	sys_in.ipa_ep_cfg.hdr_ext.hdr_payload_len_inc_padding = false;
	sys_in.ipa_ep_cfg.hdr_ext.hdr_total_len_or_pad_offset = 0;
	sys_in.ipa_ep_cfg.hdr_ext.hdr_pad_to_alignment = 2;
	sys_in.ipa_ep_cfg.cfg.cs_offload_en = IPA_DISABLE_CS_OFFLOAD;

	/**
	 * ipa_lan_rx_cb() intended to notify the source EP about packet
	 * being received on the LAN_CONS via calling the source EP call-back.
	 * There could be a race condition with calling this call-back. Other
	 * thread may nullify it - e.g. on EP disconnect.
	 * This lock intended to protect the access to the source EP call-back
	 */
	spin_lock_init(&ipa3_ctx->disconnect_lock);
	if (ipa3_setup_sys_pipe(&sys_in, &ipa3_ctx->clnt_hdl_data_in)) {
		IPAERR(":setup sys pipe (LAN_CONS) failed.\n");
		result = -EPERM;
		goto fail_flt_hash_tuple;
	}

	/* LAN OUT (AP->IPA) */
	if (!ipa3_ctx->ipa_config_is_mhi) {
		memset(&sys_in, 0, sizeof(struct ipa_sys_connect_params));
		sys_in.client = IPA_CLIENT_APPS_LAN_PROD;
		sys_in.desc_fifo_sz = IPA_SYS_TX_DATA_DESC_FIFO_SZ;
		sys_in.ipa_ep_cfg.mode.mode = IPA_BASIC;
		if (ipa3_ctx->ulso_supported) {
			sys_in.ipa_ep_cfg.ulso.ipid_min_max_idx =
				ENDP_INIT_ULSO_CFG_IP_ID_MIN_MAX_VAL_IDX_LINUX;
			sys_in.ipa_ep_cfg.ulso.is_ulso_pipe = true;
			sys_in.ipa_ep_cfg.cfg.cs_offload_en = IPA_ENABLE_CS_OFFLOAD_UL;
			sys_in.ipa_ep_cfg.hdr.hdr_len = QMAP_HDR_LEN + ETH_HLEN;
			sys_in.ipa_ep_cfg.hdr_ext.hdr_bytes_to_remove_valid = true;
			sys_in.ipa_ep_cfg.hdr_ext.hdr_bytes_to_remove = QMAP_HDR_LEN;
		}
		if (ipa3_setup_sys_pipe(&sys_in,
			&ipa3_ctx->clnt_hdl_data_out)) {
			IPAERR(":setup sys pipe (LAN_PROD) failed.\n");
			result = -EPERM;
			goto fail_lan_data_out;
		}
	}

	return 0;

fail_lan_data_out:
	ipa3_teardown_sys_pipe(ipa3_ctx->clnt_hdl_data_in);
fail_flt_hash_tuple:
	if (ipa3_ctx->dflt_v6_rt_rule_hdl)
		__ipa3_del_rt_rule(ipa3_ctx->dflt_v6_rt_rule_hdl);
	if (ipa3_ctx->dflt_v4_rt_rule_hdl)
		__ipa3_del_rt_rule(ipa3_ctx->dflt_v4_rt_rule_hdl);
	if (ipa3_ctx->excp_hdr_hdl)
		__ipa3_del_hdr(ipa3_ctx->excp_hdr_hdl, false);
	ipa3_teardown_sys_pipe(ipa3_ctx->clnt_hdl_cmd);
fail_ch20_wa:
	return result;
}

static void ipa3_teardown_apps_pipes(void)
{
	if (!ipa3_ctx->ipa_config_is_mhi)
		ipa3_teardown_sys_pipe(ipa3_ctx->clnt_hdl_data_out);
	ipa3_teardown_sys_pipe(ipa3_ctx->clnt_hdl_data_in);
	__ipa3_del_rt_rule(ipa3_ctx->dflt_v6_rt_rule_hdl);
	__ipa3_del_rt_rule(ipa3_ctx->dflt_v4_rt_rule_hdl);
	__ipa3_del_hdr(ipa3_ctx->excp_hdr_hdl, false);
	ipa3_teardown_sys_pipe(ipa3_ctx->clnt_hdl_cmd);
	ipa3_dealloc_common_event_ring();
}

#ifdef CONFIG_COMPAT

static long compat_ipa3_nat_ipv6ct_alloc_table(unsigned long arg,
	int (alloc_func)(struct ipa_ioc_nat_ipv6ct_table_alloc *))
{
	long retval;
	struct ipa_ioc_nat_ipv6ct_table_alloc32 table_alloc32;
	struct ipa_ioc_nat_ipv6ct_table_alloc table_alloc;

	retval = copy_from_user(&table_alloc32, (const void __user *)arg,
		sizeof(struct ipa_ioc_nat_ipv6ct_table_alloc32));
	if (retval)
		return retval;

	table_alloc.size = (size_t)table_alloc32.size;
	table_alloc.offset = (off_t)table_alloc32.offset;

	retval = alloc_func(&table_alloc);
	if (retval)
		return retval;

	if (table_alloc.offset) {
		table_alloc32.offset = (compat_off_t)table_alloc.offset;
		retval = copy_to_user((void __user *)arg, &table_alloc32,
			sizeof(struct ipa_ioc_nat_ipv6ct_table_alloc32));
	}

	return retval;
}

long compat_ipa3_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long retval = 0;
	struct ipa3_ioc_nat_alloc_mem32 nat_mem32;
	struct ipa_ioc_nat_alloc_mem nat_mem;

	switch (cmd) {
	case IPA_IOC_ADD_HDR32:
		cmd = IPA_IOC_ADD_HDR;
		break;
	case IPA_IOC_DEL_HDR32:
		cmd = IPA_IOC_DEL_HDR;
		break;
	case IPA_IOC_ADD_RT_RULE32:
		cmd = IPA_IOC_ADD_RT_RULE;
		break;
	case IPA_IOC_DEL_RT_RULE32:
		cmd = IPA_IOC_DEL_RT_RULE;
		break;
	case IPA_IOC_ADD_FLT_RULE32:
		cmd = IPA_IOC_ADD_FLT_RULE;
		break;
	case IPA_IOC_DEL_FLT_RULE32:
		cmd = IPA_IOC_DEL_FLT_RULE;
		break;
	case IPA_IOC_GET_RT_TBL32:
		cmd = IPA_IOC_GET_RT_TBL;
		break;
	case IPA_IOC_COPY_HDR32:
		cmd = IPA_IOC_COPY_HDR;
		break;
	case IPA_IOC_QUERY_INTF32:
		cmd = IPA_IOC_QUERY_INTF;
		break;
	case IPA_IOC_QUERY_INTF_TX_PROPS32:
		cmd = IPA_IOC_QUERY_INTF_TX_PROPS;
		break;
	case IPA_IOC_QUERY_INTF_RX_PROPS32:
		cmd = IPA_IOC_QUERY_INTF_RX_PROPS;
		break;
	case IPA_IOC_QUERY_INTF_EXT_PROPS32:
		cmd = IPA_IOC_QUERY_INTF_EXT_PROPS;
		break;
	case IPA_IOC_GET_HDR32:
		cmd = IPA_IOC_GET_HDR;
		break;
	case IPA_IOC_ALLOC_NAT_MEM32:
		retval = copy_from_user(&nat_mem32, (const void __user *)arg,
			sizeof(struct ipa3_ioc_nat_alloc_mem32));
		if (retval)
			return retval;
		memcpy(nat_mem.dev_name, nat_mem32.dev_name,
				IPA_RESOURCE_NAME_MAX);
		nat_mem.size = (size_t)nat_mem32.size;
		nat_mem.offset = (off_t)nat_mem32.offset;

		/* null terminate the string */
		nat_mem.dev_name[IPA_RESOURCE_NAME_MAX - 1] = '\0';

		retval = ipa3_allocate_nat_device(&nat_mem);
		if (retval)
			return retval;
		nat_mem32.offset = (compat_off_t)nat_mem.offset;
		retval = copy_to_user((void __user *)arg, &nat_mem32,
			sizeof(struct ipa3_ioc_nat_alloc_mem32));
		return retval;
	case IPA_IOC_ALLOC_NAT_TABLE32:
		return compat_ipa3_nat_ipv6ct_alloc_table(arg,
			ipa3_allocate_nat_table);
	case IPA_IOC_ALLOC_IPV6CT_TABLE32:
		return compat_ipa3_nat_ipv6ct_alloc_table(arg,
			ipa3_allocate_ipv6ct_table);
	case IPA_IOC_V4_INIT_NAT32:
		cmd = IPA_IOC_V4_INIT_NAT;
		break;
	case IPA_IOC_INIT_IPV6CT_TABLE32:
		cmd = IPA_IOC_INIT_IPV6CT_TABLE;
		break;
	case IPA_IOC_TABLE_DMA_CMD32:
		cmd = IPA_IOC_TABLE_DMA_CMD;
		break;
	case IPA_IOC_V4_DEL_NAT32:
		cmd = IPA_IOC_V4_DEL_NAT;
		break;
	case IPA_IOC_DEL_NAT_TABLE32:
		cmd = IPA_IOC_DEL_NAT_TABLE;
		break;
	case IPA_IOC_DEL_IPV6CT_TABLE32:
		cmd = IPA_IOC_DEL_IPV6CT_TABLE;
		break;
	case IPA_IOC_NAT_MODIFY_PDN32:
		cmd = IPA_IOC_NAT_MODIFY_PDN;
		break;
	case IPA_IOC_GET_NAT_OFFSET32:
		cmd = IPA_IOC_GET_NAT_OFFSET;
		break;
	case IPA_IOC_PULL_MSG32:
		cmd = IPA_IOC_PULL_MSG;
		break;
	case IPA_IOC_RM_ADD_DEPENDENCY32:
		cmd = IPA_IOC_RM_ADD_DEPENDENCY;
		break;
	case IPA_IOC_RM_DEL_DEPENDENCY32:
		cmd = IPA_IOC_RM_DEL_DEPENDENCY;
		break;
	case IPA_IOC_GENERATE_FLT_EQ32:
		cmd = IPA_IOC_GENERATE_FLT_EQ;
		break;
	case IPA_IOC_QUERY_RT_TBL_INDEX32:
		cmd = IPA_IOC_QUERY_RT_TBL_INDEX;
		break;
	case IPA_IOC_WRITE_QMAPID32:
		cmd = IPA_IOC_WRITE_QMAPID;
		break;
	case IPA_IOC_MDFY_FLT_RULE32:
		cmd = IPA_IOC_MDFY_FLT_RULE;
		break;
	case IPA_IOC_NOTIFY_WAN_UPSTREAM_ROUTE_ADD32:
		cmd = IPA_IOC_NOTIFY_WAN_UPSTREAM_ROUTE_ADD;
		break;
	case IPA_IOC_NOTIFY_WAN_UPSTREAM_ROUTE_DEL32:
		cmd = IPA_IOC_NOTIFY_WAN_UPSTREAM_ROUTE_DEL;
		break;
	case IPA_IOC_NOTIFY_WAN_EMBMS_CONNECTED32:
		cmd = IPA_IOC_NOTIFY_WAN_EMBMS_CONNECTED;
		break;
	case IPA_IOC_MDFY_RT_RULE32:
		cmd = IPA_IOC_MDFY_RT_RULE;
		break;
	case IPA_IOC_GET_NAT_IN_SRAM_INFO32:
		cmd = IPA_IOC_GET_NAT_IN_SRAM_INFO;
		break;
	case IPA_IOC_APP_CLOCK_VOTE32:
		cmd = IPA_IOC_APP_CLOCK_VOTE;
		break;
	case IPA_IOC_ADD_EoGRE_MAPPING32:
		cmd = IPA_IOC_ADD_EoGRE_MAPPING;
		break;
	case IPA_IOC_DEL_EoGRE_MAPPING32:
		cmd = IPA_IOC_DEL_EoGRE_MAPPING;
		break;
	case IPA_IOC_COMMIT_HDR:
	case IPA_IOC_RESET_HDR:
	case IPA_IOC_COMMIT_RT:
	case IPA_IOC_RESET_RT:
	case IPA_IOC_COMMIT_FLT:
	case IPA_IOC_RESET_FLT:
	case IPA_IOC_DUMP:
	case IPA_IOC_PUT_RT_TBL:
	case IPA_IOC_PUT_HDR:
	case IPA_IOC_SET_FLT:
	case IPA_IOC_QUERY_EP_MAPPING:
		break;
	default:
		return -ENOIOCTLCMD;
	}
	return ipa3_ioctl(file, cmd, (unsigned long) compat_ptr(arg));
}
#endif

static ssize_t ipa3_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos);

static const struct file_operations ipa3_drv_fops = {
	.owner = THIS_MODULE,
	.open = ipa3_open,
	.read = ipa3_read,
	.write = ipa3_write,
	.unlocked_ioctl = ipa3_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = compat_ipa3_ioctl,
#endif
};

static int ipa3_get_clks(struct device *dev)
{
	if (!IPA_IS_REGULAR_CLK_MODE(ipa3_ctx->ipa3_hw_mode)) {
		IPADBG("not supported in this HW mode\n");
		ipa3_clk = NULL;
		return 0;
	}

	if (ipa3_res.use_bw_vote) {
		IPADBG("Vote IPA clock by bw voting via bus scaling driver\n");
		ipa3_clk = NULL;
		return 0;
	}

	ipa3_clk = clk_get(dev, "core_clk");
	if (IS_ERR(ipa3_clk)) {
		if (ipa3_clk != ERR_PTR(-EPROBE_DEFER))
			IPAERR("fail to get ipa clk\n");
		return PTR_ERR(ipa3_clk);
	}
	return 0;
}

/**
 * _ipa_enable_clks_v3_0() - Enable IPA clocks.
 */
void _ipa_enable_clks_v3_0(void)
{
	IPADBG_LOW("curr_ipa_clk_rate=%d", ipa3_ctx->curr_ipa_clk_rate);
	if (ipa3_clk) {
		IPADBG_LOW("enabling gcc_ipa_clk\n");
		clk_prepare(ipa3_clk);
		clk_enable(ipa3_clk);
		clk_set_rate(ipa3_clk, ipa3_ctx->curr_ipa_clk_rate);
	}

	ipa3_uc_notify_clk_state(true);
}

static unsigned int ipa3_get_bus_vote(void)
{
	unsigned int idx = 1;

	if (ipa3_ctx->curr_ipa_clk_rate == ipa3_ctx->ctrl->ipa_clk_rate_svs2) {
		idx = 1;
	} else if (ipa3_ctx->curr_ipa_clk_rate ==
		ipa3_ctx->ctrl->ipa_clk_rate_svs) {
		idx = 2;
	} else if (ipa3_ctx->curr_ipa_clk_rate ==
		ipa3_ctx->ctrl->ipa_clk_rate_nominal) {
		idx = 3;
	} else if (ipa3_ctx->curr_ipa_clk_rate ==
			ipa3_ctx->ctrl->ipa_clk_rate_turbo) {
		idx = 4;
	} else {
		WARN(1, "unexpected clock rate");
	}
	IPADBG_LOW("curr %d idx %d\n", ipa3_ctx->curr_ipa_clk_rate, idx);

	return idx;
}

/**
 * ipa3_enable_clks() - Turn on IPA clocks
 *
 * Return codes:
 * None
 */
void ipa3_enable_clks(void)
{
	int idx;
	int i;

	if (!IPA_IS_REGULAR_CLK_MODE(ipa3_ctx->ipa3_hw_mode)) {
		IPAERR("not supported in this mode\n");
		return;
	}

	IPADBG("enabling IPA clocks and bus voting\n");

	idx = ipa3_get_bus_vote();

	IPADBG_CLK("IPA ICC Voting for BW Started\n");
	for (i = 0; i < ipa3_ctx->icc_num_paths; i++) {
		if (ipa3_ctx->ctrl->icc_path[i] &&
			icc_set_bw(
			ipa3_ctx->ctrl->icc_path[i],
			ipa3_ctx->icc_clk[idx][i][IPA_ICC_AB],
			ipa3_ctx->icc_clk[idx][i][IPA_ICC_IB]))
			WARN(1, "path %d bus scaling failed", i);
			IPADBG_CLK("IPA ICC Voting for BW %d Path Completed\n", i);
	}
	IPADBG_CLK("IPA ICC Voting for BW Finished\n");

	IPADBG_CLK("Enabling IPA Clocks Started\n");
	ipa3_ctx->ctrl->ipa3_enable_clks();
	IPADBG_CLK("Enabling IPA Clocks Finished\n");
	atomic_set(&ipa3_ctx->ipa_clk_vote, 1);
}


/**
 * _ipa_disable_clks_v3_0() - Disable IPA clocks.
 */
void _ipa_disable_clks_v3_0(void)
{
	ipa3_uc_notify_clk_state(false);
	if (ipa3_clk) {
		IPADBG_LOW("disabling gcc_ipa_clk\n");
		clk_disable_unprepare(ipa3_clk);
	}
}

/**
 * ipa3_disable_clks() - Turn off IPA clocks
 *
 * Return codes:
 * None
 */
void ipa3_disable_clks(void)
{
	int i;
	int type;

	if (!IPA_IS_REGULAR_CLK_MODE(ipa3_ctx->ipa3_hw_mode)) {
		IPAERR("not supported in this mode\n");
		return;
	}

	IPADBG("Disabling IPA clocks and bus voting\n");

	if (atomic_read(&ipa3_ctx->ipa_clk_vote) == 0)
	{
		IPAERR("IPA clock already disabled\n");
		return;
	}

	/*
	 * We see a NoC error on GSI on this flag sequence.
	 * Need to set this flag first before clock off.
	 */
	atomic_set(&ipa3_ctx->ipa_clk_vote, 0);

	/*
	 * If there is still pending gsi irq, this indicate
	 * issue on GSI FW side. We need to capture before
	 * turn off the ipa clock.
	 */
	if (!ipa3_ctx->ipa_config_is_mhi) {
		type = gsi_pending_irq_type();
		if (type) {
			IPAERR("unexpected gsi irq type: %d\n", type);
			/* increase ipa3_active_clients for smp2p response */
			atomic_inc(&ipa3_ctx->ipa3_active_clients.cnt);
			ipa_assert();
		}
	}

	IPADBG_CLK("Disabling IPA Clocks Started\n");
	ipa3_ctx->ctrl->ipa3_disable_clks();
	IPADBG_CLK("Disabling IPA Clocks Finished\n");

	ipa_pm_set_clock_index(0);

	IPADBG_CLK("IPA ICC Voting for BW Started\n");
	for (i = 0; i < ipa3_ctx->icc_num_paths; i++) {
		if (ipa3_ctx->ctrl->icc_path[i] &&
			icc_set_bw(
			ipa3_ctx->ctrl->icc_path[i],
			ipa3_ctx->icc_clk[IPA_ICC_NONE][i][IPA_ICC_AB],
			ipa3_ctx->icc_clk[IPA_ICC_NONE][i][IPA_ICC_IB]))
			WARN(1, "path %d bus off failed", i);
			IPADBG_CLK("IPA ICC Voting for BW %d Path Completed\n", i);
	}
	IPADBG_CLK("IPA ICC Voting for BW Finished\n");
	atomic_set(&ipa3_ctx->ipa_clk_vote, 0);
}

/**
 * ipa3_start_tag_process() - Send TAG packet and wait for it to come back
 *
 * This function is called prior to clock gating when active client counter
 * is 1. TAG process ensures that there are no packets inside IPA HW that
 * were not submitted to the IPA client via the transport. During TAG process
 * all aggregation frames are (force) closed.
 *
 * Return codes:
 * None
 */
static void ipa3_start_tag_process(struct work_struct *work)
{
	int res;

	IPADBG("starting TAG process\n");
	/* close aggregation frames on all pipes */
	res = ipa3_tag_aggr_force_close(-1);
	if (res)
		IPAERR("ipa3_tag_aggr_force_close failed %d\n", res);
	IPA_ACTIVE_CLIENTS_DEC_SPECIAL("TAG_PROCESS");

	IPADBG("TAG process done\n");
}

/**
 * ipa3_active_clients_log_mod() - Log a modification in the active clients
 * reference count
 *
 * This method logs any modification in the active clients reference count:
 * It logs the modification in the circular history buffer
 * It logs the modification in the hash table - looking for an entry,
 * creating one if needed and deleting one if needed.
 *
 * @id: ipa3_active client logging info struct to hold the log information
 * @inc: a boolean variable to indicate whether the modification is an increase
 * or decrease
 * @int_ctx: a boolean variable to indicate whether this call is being made from
 * an interrupt context and therefore should allocate GFP_ATOMIC memory
 *
 * Method process:
 * - Hash the unique identifier string
 * - Find the hash in the table
 *    1)If found, increase or decrease the reference count
 *    2)If not found, allocate a new hash table entry struct and initialize it
 * - Remove and deallocate unneeded data structure
 * - Log the call in the circular history buffer (unless it is a simple call)
 */
#ifdef CONFIG_IPA_DEBUG
static void ipa3_active_clients_log_mod(
		struct ipa_active_client_logging_info *id,
		bool inc, bool int_ctx)
{
	char temp_str[IPA3_ACTIVE_CLIENTS_LOG_LINE_LEN];
	unsigned long long t;
	unsigned long nanosec_rem;
	struct ipa3_active_client_htable_entry *hentry;
	struct ipa3_active_client_htable_entry *hfound;
	u32 hkey;
	char str_to_hash[IPA3_ACTIVE_CLIENTS_LOG_NAME_LEN];
	unsigned long flags;

	spin_lock_irqsave(&ipa3_ctx->ipa3_active_clients_logging.lock, flags);
	int_ctx = true;
	hfound = NULL;
	memset(str_to_hash, 0, IPA3_ACTIVE_CLIENTS_LOG_NAME_LEN);
	strlcpy(str_to_hash, id->id_string, IPA3_ACTIVE_CLIENTS_LOG_NAME_LEN);
	hkey = jhash(str_to_hash, IPA3_ACTIVE_CLIENTS_LOG_NAME_LEN,
			0);
	hash_for_each_possible(ipa3_ctx->ipa3_active_clients_logging.htable,
			hentry, list, hkey) {
		if (!strcmp(hentry->id_string, id->id_string)) {
			hentry->count = hentry->count + (inc ? 1 : -1);
			hfound = hentry;
		}
	}
	if (hfound == NULL) {
		hentry = NULL;
		hentry = kzalloc(sizeof(
				struct ipa3_active_client_htable_entry),
				int_ctx ? GFP_ATOMIC : GFP_KERNEL);
		if (hentry == NULL) {
			spin_unlock_irqrestore(
				&ipa3_ctx->ipa3_active_clients_logging.lock,
				flags);
			return;
		}
		hentry->type = id->type;
		strlcpy(hentry->id_string, id->id_string,
				IPA3_ACTIVE_CLIENTS_LOG_NAME_LEN);
		INIT_HLIST_NODE(&hentry->list);
		hentry->count = inc ? 1 : -1;
		hash_add(ipa3_ctx->ipa3_active_clients_logging.htable,
				&hentry->list, hkey);
	} else if (hfound->count == 0) {
		hash_del(&hfound->list);
		kfree(hfound);
	}

	if (id->type != SIMPLE) {
		t = local_clock();
		nanosec_rem = do_div(t, 1000000000) / 1000;
		snprintf(temp_str, IPA3_ACTIVE_CLIENTS_LOG_LINE_LEN,
				inc ? "[%5lu.%06lu] ^ %s, %s: %d" :
						"[%5lu.%06lu] v %s, %s: %d",
				(unsigned long)t, nanosec_rem,
				id->id_string, id->file, id->line);
		ipa3_active_clients_log_insert(temp_str);
	}
	spin_unlock_irqrestore(&ipa3_ctx->ipa3_active_clients_logging.lock,
		flags);
}
#else
static void ipa3_active_clients_log_mod(
		struct ipa_active_client_logging_info *id,
		bool inc, bool int_ctx)
{
}
#endif

void ipa3_active_clients_log_dec(struct ipa_active_client_logging_info *id,
		bool int_ctx)
{
	ipa3_active_clients_log_mod(id, false, int_ctx);
}

void ipa3_active_clients_log_inc(struct ipa_active_client_logging_info *id,
		bool int_ctx)
{
	ipa3_active_clients_log_mod(id, true, int_ctx);
}

/**
 * ipa3_inc_client_enable_clks() - Increase active clients counter, and
 * enable ipa clocks if necessary
 *
 * Return codes:
 * None
 */
void ipa3_inc_client_enable_clks(struct ipa_active_client_logging_info *id)
{
	int ret;

	ipa3_active_clients_log_inc(id, false);
	ret = atomic_inc_not_zero(&ipa3_ctx->ipa3_active_clients.cnt);
	if (ret) {
		IPADBG_LOW("active clients = %d\n",
			atomic_read(&ipa3_ctx->ipa3_active_clients.cnt));
		return;
	}

	mutex_lock(&ipa3_ctx->ipa3_active_clients.mutex);

	/* somebody might voted to clocks meanwhile */
	ret = atomic_inc_not_zero(&ipa3_ctx->ipa3_active_clients.cnt);
	if (ret) {
		mutex_unlock(&ipa3_ctx->ipa3_active_clients.mutex);
		IPADBG_LOW("active clients = %d\n",
			atomic_read(&ipa3_ctx->ipa3_active_clients.cnt));
		return;
	}

	ipa3_enable_clks();
	ipa3_suspend_apps_pipes(false);
	atomic_inc(&ipa3_ctx->ipa3_active_clients.cnt);
	IPADBG_LOW("active clients = %d\n",
		atomic_read(&ipa3_ctx->ipa3_active_clients.cnt));
	mutex_unlock(&ipa3_ctx->ipa3_active_clients.mutex);
}
EXPORT_SYMBOL(ipa3_inc_client_enable_clks);

static void ipa3_handle_gsi_differ_irq(void)
{
	queue_work(ipa3_ctx->power_mgmt_wq,
		&ipa_inc_clients_enable_clks_on_wq_work);
}

/**
 * ipa3_active_clks_status() - update the current msm bus clock vote
 * status
 */
int ipa3_active_clks_status(void)
{
	return atomic_read(&ipa3_ctx->ipa_clk_vote);
}

/**
 * ipa3_inc_client_enable_clks_no_block() - Only increment the number of active
 * clients if no asynchronous actions should be done. Asynchronous actions are
 * locking a mutex and waking up IPA HW.
 *
 * Return codes: 0 for success
 *		-EPERM if an asynchronous action should have been done
 */
int ipa3_inc_client_enable_clks_no_block(struct ipa_active_client_logging_info
		*id)
{
	int ret;

	ret = atomic_inc_not_zero(&ipa3_ctx->ipa3_active_clients.cnt);
	if (ret) {
		ipa3_active_clients_log_inc(id, true);
		IPADBG_LOW("active clients = %d\n",
			atomic_read(&ipa3_ctx->ipa3_active_clients.cnt));
		return 0;
	}

	return -EPERM;
}
EXPORT_SYMBOL(ipa3_inc_client_enable_clks_no_block);

static void __ipa3_dec_client_disable_clks(void)
{
	int ret;

	if (!atomic_read(&ipa3_ctx->ipa3_active_clients.cnt)) {
		IPAERR("trying to disable clocks with refcnt is 0\n");
		ipa_assert();
		return;
	}

	ret = atomic_add_unless(&ipa3_ctx->ipa3_active_clients.cnt, -1, 1);
	if (ret)
		goto bail;

	/* Send force close coalsecing frame command in LPM mode before taking
	 * mutex lock and otherwise observing race condition.
	 */
	if (atomic_read(&ipa3_ctx->ipa3_active_clients.cnt) == 1 &&
		!ipa3_ctx->tag_process_before_gating) {
		ipa3_force_close_coal();
		/* While sending force close command setting
		 * tag process as true to make configure to
		 * original state
		 */
		ipa3_ctx->tag_process_before_gating = false;
	}
	/* seems like this is the only client holding the clocks */
	mutex_lock(&ipa3_ctx->ipa3_active_clients.mutex);
	if (atomic_read(&ipa3_ctx->ipa3_active_clients.cnt) == 1 &&
	    ipa3_ctx->tag_process_before_gating) {
		ipa3_ctx->tag_process_before_gating = false;
		/*
		 * When TAG process ends, active clients will be
		 * decreased
		 */
		queue_work(ipa3_ctx->power_mgmt_wq, &ipa3_tag_work);
		goto unlock_mutex;
	}

	/* a different context might increase the clock reference meanwhile */
	ret = atomic_sub_return(1, &ipa3_ctx->ipa3_active_clients.cnt);
	if (ret > 0)
		goto unlock_mutex;
	ret = ipa3_suspend_apps_pipes(true);
	if (ret) {
		/* HW is busy, retry after some time */
		atomic_inc(&ipa3_ctx->ipa3_active_clients.cnt);
		queue_delayed_work(ipa3_ctx->power_mgmt_wq,
			&ipa_dec_clients_disable_clks_on_wq_work,
			IPA_SUSPEND_BUSY_TIMEOUT);
	} else {
		ipa3_disable_clks();
	}

unlock_mutex:
	mutex_unlock(&ipa3_ctx->ipa3_active_clients.mutex);
bail:
	IPADBG_LOW("active clients = %d\n",
		atomic_read(&ipa3_ctx->ipa3_active_clients.cnt));
}

/**
 * ipa3_dec_client_disable_clks() - Decrease active clients counter
 *
 * In case that there are no active clients this function also starts
 * TAG process. When TAG progress ends ipa clocks will be gated.
 * start_tag_process_again flag is set during this function to signal TAG
 * process to start again as there was another client that may send data to ipa
 *
 * Return codes:
 * None
 */
void ipa3_dec_client_disable_clks(struct ipa_active_client_logging_info *id)
{
	ipa3_active_clients_log_dec(id, false);
	__ipa3_dec_client_disable_clks();
}
EXPORT_SYMBOL(ipa3_dec_client_disable_clks);

static void ipa_dec_clients_disable_clks_on_wq(struct work_struct *work)
{
	__ipa3_dec_client_disable_clks();
}

static void ipa_inc_clients_enable_clks_on_wq(struct work_struct *work)
{
	int type;
	ipa3_enable_clks();
	IPAERR("unexpected clk access, clock on IPA to save reg");
	type = gsi_pending_irq_type();
	if (type) {
		IPAERR("unexpected gsi irq type: %d\n", type);
        }
	ipa_assert();
}

/**
 * ipa3_dec_client_disable_clks_no_block() - Decrease active clients counter
 * if possible without blocking. If this is the last client then the desrease
 * will happen from work queue context.
 *
 * Return codes:
 * None
 */
void ipa3_dec_client_disable_clks_no_block(
	struct ipa_active_client_logging_info *id)
{
	int ret;

	ipa3_active_clients_log_dec(id, true);
	ret = atomic_add_unless(&ipa3_ctx->ipa3_active_clients.cnt, -1, 1);
	if (ret) {
		IPADBG_LOW("active clients = %d\n",
			atomic_read(&ipa3_ctx->ipa3_active_clients.cnt));
		return;
	}

	/* seems like this is the only client holding the clocks */
	queue_delayed_work(ipa3_ctx->power_mgmt_wq,
		&ipa_dec_clients_disable_clks_on_wq_work, 0);
}

/**
 * ipa3_dec_client_disable_clks_delay_wq() - Decrease active clients counter
 * in delayed workqueue.
 *
 * Return codes:
 * None
 */
void ipa3_dec_client_disable_clks_delay_wq(
	struct ipa_active_client_logging_info *id, unsigned long delay)
{
	ipa3_active_clients_log_dec(id, true);

	if (!queue_delayed_work(ipa3_ctx->power_mgmt_wq,
		&ipa_dec_clients_disable_clks_on_suspend_irq_wq_work, delay))
		IPAERR("Scheduling delayed work failed\n");
}
/**
 * ipa3_inc_acquire_wakelock() - Increase active clients counter, and
 * acquire wakelock if necessary
 *
 * Return codes:
 * None
 */
void ipa3_inc_acquire_wakelock(void)
{
	unsigned long flags;

	spin_lock_irqsave(&ipa3_ctx->wakelock_ref_cnt.spinlock, flags);
	ipa3_ctx->wakelock_ref_cnt.cnt++;
	if (ipa3_ctx->wakelock_ref_cnt.cnt == 1)
		__pm_stay_awake(ipa3_ctx->w_lock);
	IPADBG_LOW("active wakelock ref cnt = %d\n",
		ipa3_ctx->wakelock_ref_cnt.cnt);
	spin_unlock_irqrestore(&ipa3_ctx->wakelock_ref_cnt.spinlock, flags);
}

/**
 * ipa3_dec_release_wakelock() - Decrease active clients counter
 *
 * In case if the ref count is 0, release the wakelock.
 *
 * Return codes:
 * None
 */
void ipa3_dec_release_wakelock(void)
{
	unsigned long flags;

	spin_lock_irqsave(&ipa3_ctx->wakelock_ref_cnt.spinlock, flags);
	ipa3_ctx->wakelock_ref_cnt.cnt--;
	IPADBG_LOW("active wakelock ref cnt = %d\n",
		ipa3_ctx->wakelock_ref_cnt.cnt);
	if (ipa3_ctx->wakelock_ref_cnt.cnt == 0)
		__pm_relax(ipa3_ctx->w_lock);
	spin_unlock_irqrestore(&ipa3_ctx->wakelock_ref_cnt.spinlock, flags);
}

int ipa3_set_clock_plan_from_pm(int idx)
{
	u32 clk_rate;
	int i;

	IPADBG_LOW("idx = %d\n", idx);

	if (!ipa3_ctx->enable_clock_scaling) {
		ipa3_ctx->ipa3_active_clients.bus_vote_idx = idx;
		return 0;
	}

	if (!IPA_IS_REGULAR_CLK_MODE(ipa3_ctx->ipa3_hw_mode)) {
		IPAERR("not supported in this mode\n");
		return 0;
	}

	if (idx <= 0 || idx >= 5) {
		IPAERR("bad voltage\n");
		return -EINVAL;
	}

	if (idx == 1)
		clk_rate = ipa3_ctx->ctrl->ipa_clk_rate_svs2;
	else if (idx == 2)
		clk_rate = ipa3_ctx->ctrl->ipa_clk_rate_svs;
	else if (idx == 3)
		clk_rate = ipa3_ctx->ctrl->ipa_clk_rate_nominal;
	else if (idx == 4)
		clk_rate = ipa3_ctx->ctrl->ipa_clk_rate_turbo;
	else {
		IPAERR("bad voltage\n");
		WARN_ON(1);
		return -EFAULT;
	}

	if (clk_rate == ipa3_ctx->curr_ipa_clk_rate) {
		IPADBG_LOW("Same voltage\n");
		return 0;
	}

	mutex_lock(&ipa3_ctx->ipa3_active_clients.mutex);
	ipa3_ctx->curr_ipa_clk_rate = clk_rate;
	ipa3_ctx->ipa3_active_clients.bus_vote_idx = idx;
	IPADBG_LOW("setting clock rate to %u\n", ipa3_ctx->curr_ipa_clk_rate);
	if (atomic_read(&ipa3_ctx->ipa3_active_clients.cnt) > 0) {
		if (ipa3_clk)
			clk_set_rate(ipa3_clk, ipa3_ctx->curr_ipa_clk_rate);
		idx = ipa3_get_bus_vote();
		for (i = 0; i < ipa3_ctx->icc_num_paths; i++) {
			if (ipa3_ctx->ctrl->icc_path[i] &&
			    icc_set_bw(
			    ipa3_ctx->ctrl->icc_path[i],
			    ipa3_ctx->icc_clk[idx][i][IPA_ICC_AB],
			    ipa3_ctx->icc_clk[idx][i][IPA_ICC_IB])) {
				WARN(1, "path %d bus scaling failed",
					i);
			}
		}
	} else {
		IPADBG_LOW("clocks are gated, not setting rate\n");
	}
	mutex_unlock(&ipa3_ctx->ipa3_active_clients.mutex);
	IPADBG_LOW("Done\n");

	return 0;
}

int ipa3_set_required_perf_profile(enum ipa_voltage_level floor_voltage,
				  u32 bandwidth_mbps)
{
	enum ipa_voltage_level needed_voltage;
	u32 clk_rate;
	int i;
	int idx;

	if (!IPA_IS_REGULAR_CLK_MODE(ipa3_ctx->ipa3_hw_mode)) {
		IPAERR("not supported in this mode\n");
		return 0;
	}

	IPADBG_LOW("floor_voltage=%d, bandwidth_mbps=%u",
					floor_voltage, bandwidth_mbps);

	if (floor_voltage < IPA_VOLTAGE_UNSPECIFIED ||
		floor_voltage >= IPA_VOLTAGE_MAX) {
		IPAERR("bad voltage\n");
		return -EINVAL;
	}

	if (ipa3_ctx->enable_clock_scaling) {
		IPADBG_LOW("Clock scaling is enabled\n");
		if (bandwidth_mbps >=
			ipa3_ctx->ctrl->clock_scaling_bw_threshold_turbo)
			needed_voltage = IPA_VOLTAGE_TURBO;
		else if (bandwidth_mbps >=
			ipa3_ctx->ctrl->clock_scaling_bw_threshold_nominal)
			needed_voltage = IPA_VOLTAGE_NOMINAL;
		else if (bandwidth_mbps >=
			ipa3_ctx->ctrl->clock_scaling_bw_threshold_svs)
			needed_voltage = IPA_VOLTAGE_SVS;
		else
			needed_voltage = IPA_VOLTAGE_SVS2;
	} else {
		IPADBG_LOW("Clock scaling is disabled\n");
		needed_voltage = IPA_VOLTAGE_NOMINAL;
	}

	needed_voltage = max(needed_voltage, floor_voltage);
	switch (needed_voltage) {
	case IPA_VOLTAGE_SVS2:
		clk_rate = ipa3_ctx->ctrl->ipa_clk_rate_svs2;
		break;
	case IPA_VOLTAGE_SVS:
		clk_rate = ipa3_ctx->ctrl->ipa_clk_rate_svs;
		break;
	case IPA_VOLTAGE_NOMINAL:
		clk_rate = ipa3_ctx->ctrl->ipa_clk_rate_nominal;
		break;
	case IPA_VOLTAGE_TURBO:
		clk_rate = ipa3_ctx->ctrl->ipa_clk_rate_turbo;
		break;
	default:
		IPAERR("bad voltage\n");
		WARN_ON(1);
		return -EFAULT;
	}

	if (clk_rate == ipa3_ctx->curr_ipa_clk_rate) {
		IPADBG_LOW("Same voltage\n");
		return 0;
	}

	/* Hold the mutex to avoid race conditions with ipa3_enable_clocks() */
	mutex_lock(&ipa3_ctx->ipa3_active_clients.mutex);
	ipa3_ctx->curr_ipa_clk_rate = clk_rate;
	IPADBG_LOW("setting clock rate to %u\n", ipa3_ctx->curr_ipa_clk_rate);
	if (atomic_read(&ipa3_ctx->ipa3_active_clients.cnt) > 0) {
		if (ipa3_clk)
			clk_set_rate(ipa3_clk, ipa3_ctx->curr_ipa_clk_rate);
		idx = ipa3_get_bus_vote();
		for (i = 0; i < ipa3_ctx->icc_num_paths; i++) {
			if (ipa3_ctx->ctrl->icc_path[i] &&
				icc_set_bw(
				ipa3_ctx->ctrl->icc_path[i],
				ipa3_ctx->icc_clk[idx][i][IPA_ICC_AB],
				ipa3_ctx->icc_clk[idx][i][IPA_ICC_IB]))
				WARN(1, "path %d bus scaling failed", i);
		}
	} else {
		IPADBG_LOW("clocks are gated, not setting rate\n");
	}
	mutex_unlock(&ipa3_ctx->ipa3_active_clients.mutex);
	IPADBG_LOW("Done\n");

	return 0;
}
EXPORT_SYMBOL(ipa3_set_required_perf_profile);

static void ipa3_process_irq_schedule_rel(void)
{
	queue_delayed_work(ipa3_ctx->transport_power_mgmt_wq,
		&ipa3_transport_release_resource_work,
		msecs_to_jiffies(IPA_TRANSPORT_PROD_TIMEOUT_MSEC));
}

/**
 * ipa3_suspend_handler() - Handles the suspend interrupt:
 * wakes up the suspended peripheral by requesting its consumer
 * @interrupt:		Interrupt type
 * @private_data:	The client's private data
 * @interrupt_data:	Interrupt specific information data
 */
void ipa3_suspend_handler(enum ipa_irq_type interrupt,
				void *private_data,
				void *interrupt_data)
{
	u32 *suspend_data =
		((struct ipa_tx_suspend_irq_data *)interrupt_data)->endpoints;
	u32 bmsk = 1;
	u32 i = 0, j = 0, ep_arr_size, ep_per_reg;
	int res;
	u32 pipe_bitmask = 0;

	IPADBG("interrupt=%d\n", interrupt);

	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v5_0) {
		ep_arr_size = IPA_EP_ARR_SIZE;
		ep_per_reg = IPA_EP_PER_REG;
	} else {
		ep_arr_size = 1;
		ep_per_reg = ipa3_ctx->ipa_num_pipes;
	}

	j = 0;
	for (i = 0; i < ipa3_ctx->ipa_num_pipes && j < ep_arr_size; i++) {
		if ((suspend_data[j] & bmsk) && (ipa3_ctx->ep[i].valid))
			pipe_bitmask |= bmsk;
		bmsk = bmsk << 1;

		if ((i % IPA_EP_PER_REG) == (ep_per_reg - 1)
			|| (i == ipa3_ctx->ipa_num_pipes - 1)) {
			IPADBG("interrupt data: %u\n", suspend_data[j]);
			res = ipa_pm_handle_suspend(pipe_bitmask, j);
			if (res) {
				IPAERR(
				"ipa_pm_handle_suspend failed %d\n", res);
				return;
			}
			pipe_bitmask = 0;
			bmsk = 1;
			j++;
		}
	}
}

/**
 * ipa3_restore_suspend_handler() - restores the original suspend IRQ handler
 * as it was registered in the IPA init sequence.
 * Return codes:
 * 0: success
 * -EPERM: failed to remove current handler or failed to add original handler
 */
int ipa3_restore_suspend_handler(void)
{
	int result = 0;

	result  = ipa3_remove_interrupt_handler(IPA_TX_SUSPEND_IRQ);
	if (result) {
		IPAERR("remove handler for suspend interrupt failed\n");
		return -EPERM;
	}

	result = ipa3_add_interrupt_handler(IPA_TX_SUSPEND_IRQ,
			ipa3_suspend_handler, false, NULL);
	if (result) {
		IPAERR("register handler for suspend interrupt failed\n");
		result = -EPERM;
	}

	IPADBG("suspend handler successfully restored\n");

	return result;
}

static void ipa3_transport_release_resource(struct work_struct *work)
{
	mutex_lock(&ipa3_ctx->transport_pm.transport_pm_mutex);
	/* check whether still need to decrease client usage */
	if (atomic_read(&ipa3_ctx->transport_pm.dec_clients)) {
		if (atomic_read(&ipa3_ctx->transport_pm.eot_activity)) {
			IPADBG("EOT pending Re-scheduling\n");
			ipa3_process_irq_schedule_rel();
		} else {
			atomic_set(&ipa3_ctx->transport_pm.dec_clients, 0);
			ipa3_dec_release_wakelock();
			IPA_ACTIVE_CLIENTS_DEC_SPECIAL("TRANSPORT_RESOURCE");
		}
	}
	atomic_set(&ipa3_ctx->transport_pm.eot_activity, 0);
	mutex_unlock(&ipa3_ctx->transport_pm.transport_pm_mutex);
}

/**
 * ipa3_init_interrupts() - Register to IPA IRQs
 *
 * Return codes: 0 in success, negative in failure
 *
 */
int ipa3_init_interrupts(void)
{
	int result;

	/*register IPA IRQ handler*/
	result = ipa3_interrupts_init(ipa3_res.ipa_irq, 0,
			&ipa3_ctx->master_pdev->dev);
	if (result) {
		IPAERR("ipa interrupts initialization failed\n");
		return -ENODEV;
	}

	/*add handler for suspend interrupt*/
	result = ipa3_add_interrupt_handler(IPA_TX_SUSPEND_IRQ,
			ipa3_suspend_handler, false, NULL);
	if (result) {
		IPAERR("register handler for suspend interrupt failed\n");
		result = -ENODEV;
		goto fail_add_interrupt_handler;
	}

	return 0;

fail_add_interrupt_handler:
	ipa3_interrupts_destroy(ipa3_res.ipa_irq, &ipa3_ctx->master_pdev->dev);
	return result;
}

/**
 * ipa3_destroy_flt_tbl_idrs() - destroy the idr structure for flt tables
 *  The idr strcuture per filtering table is intended for rule id generation
 *  per filtering rule.
 */
static void ipa3_destroy_flt_tbl_idrs(void)
{
	int i;
	struct ipa3_flt_tbl *flt_tbl;

	idr_destroy(&ipa3_ctx->flt_rule_ids[IPA_IP_v4]);
	idr_destroy(&ipa3_ctx->flt_rule_ids[IPA_IP_v6]);

	for (i = 0; i < ipa3_ctx->ipa_num_pipes; i++) {
		if (!ipa_is_ep_support_flt(i))
			continue;

		flt_tbl = &ipa3_ctx->flt_tbl[i][IPA_IP_v4];
		flt_tbl->rule_ids = NULL;
		flt_tbl = &ipa3_ctx->flt_tbl[i][IPA_IP_v6];
		flt_tbl->rule_ids = NULL;
	}
}

static void ipa3_freeze_clock_vote_and_notify_modem(void)
{
	int res;
	struct ipa_active_client_logging_info log_info;

	if (ipa3_ctx->platform_type == IPA_PLAT_TYPE_APQ) {
		IPADBG("Ignore smp2p on APQ platform\n");
		return;
	}

	if (ipa3_ctx->smp2p_info.res_sent)
		return;

	if (IS_ERR(ipa3_ctx->smp2p_info.smem_state)) {
		IPAERR("fail to get smp2p clk resp bit %ld\n",
			PTR_ERR(ipa3_ctx->smp2p_info.smem_state));
		return;
	}

	IPA_ACTIVE_CLIENTS_PREP_SPECIAL(log_info, "FREEZE_VOTE");
	res = ipa3_inc_client_enable_clks_no_block(&log_info);
	if (res)
		ipa3_ctx->smp2p_info.ipa_clk_on = false;
	else
		ipa3_ctx->smp2p_info.ipa_clk_on = true;

	qcom_smem_state_update_bits(ipa3_ctx->smp2p_info.smem_state,
			IPA_SMP2P_SMEM_STATE_MASK,
			((ipa3_ctx->smp2p_info.ipa_clk_on <<
			IPA_SMP2P_OUT_CLK_VOTE_IDX) |
			(1 << IPA_SMP2P_OUT_CLK_RSP_CMPLT_IDX)));

	ipa3_ctx->smp2p_info.res_sent = true;
	IPADBG("IPA clocks are %s\n",
		ipa3_ctx->smp2p_info.ipa_clk_on ? "ON" : "OFF");
}

void ipa3_reset_freeze_vote(void)
{
	if (!ipa3_ctx->smp2p_info.res_sent)
		return;

	if (ipa3_ctx->smp2p_info.ipa_clk_on)
		IPA_ACTIVE_CLIENTS_DEC_SPECIAL("FREEZE_VOTE");

	qcom_smem_state_update_bits(ipa3_ctx->smp2p_info.smem_state,
			IPA_SMP2P_SMEM_STATE_MASK,
			((0 <<
			IPA_SMP2P_OUT_CLK_VOTE_IDX) |
			(0 << IPA_SMP2P_OUT_CLK_RSP_CMPLT_IDX)));

	ipa3_ctx->smp2p_info.res_sent = false;
	ipa3_ctx->smp2p_info.ipa_clk_on = false;
}

static int ipa3_panic_notifier(struct notifier_block *this,
	unsigned long event, void *ptr)
{
	int res;
	struct ipa_active_client_logging_info log_info;

	if (ipa3_ctx != NULL)
	{
		if (ipa3_ctx->is_device_crashed)
			return NOTIFY_DONE;
		ipa3_ctx->is_device_crashed = true;
	}

	ipa3_freeze_clock_vote_and_notify_modem();

	IPADBG("Calling uC panic handler\n");
	res = ipa3_uc_panic_notifier(this, event, ptr);
	if (res)
		IPAERR("uC panic handler failed %d\n", res);

	/* Make sure IPA clock voted when collecting the reg dump */
	IPA_ACTIVE_CLIENTS_PREP_SPECIAL(log_info, "PANIC_VOTE");
	res = ipa3_inc_client_enable_clks_no_block(&log_info);
	if (!ipa3_active_clks_status()) {
		IPAERR("IPA clk off not saving the IPA registers\n");
	} else {
		/*make sure clock won't disable in middle of save reg*/
		if (res) {
			IPADBG("IPA resume in progress increment clinet cnt\n");
			atomic_inc(&ipa3_ctx->ipa3_active_clients.cnt);
		}
		ipa_save_registers();
		ipahal_print_all_regs(false);
		ipa_wigig_save_regs();
		if (res) {
			IPADBG("IPA resume in progress decrement clinet cnt\n");
			atomic_dec(&ipa3_ctx->ipa3_active_clients.cnt);
		}
	}

	ipa3_active_clients_log_print_table(active_clients_table_buf,
			IPA3_ACTIVE_CLIENTS_TABLE_BUF_SIZE);
	IPAERR("%s\n", active_clients_table_buf);

	return NOTIFY_DONE;
}

static struct notifier_block ipa3_panic_blk = {
	.notifier_call = ipa3_panic_notifier,
	/* IPA panic handler needs to run before modem shuts down */
	.priority = INT_MAX,
};

static void ipa3_register_panic_hdlr(void)
{
	atomic_notifier_chain_register(&panic_notifier_list,
		&ipa3_panic_blk);
}

static void ipa3_uc_is_loaded(void)
{
	IPADBG("\n");
	complete_all(&ipa3_ctx->uc_loaded_completion_obj);
}

static enum gsi_ver ipa3_get_gsi_ver(enum ipa_hw_type ipa_hw_type)
{
	enum gsi_ver gsi_ver;

	switch (ipa_hw_type) {
	case IPA_HW_v3_0:
	case IPA_HW_v3_1:
		gsi_ver = GSI_VER_1_0;
		break;
	case IPA_HW_v3_5:
		gsi_ver = GSI_VER_1_2;
		break;
	case IPA_HW_v3_5_1:
		gsi_ver = GSI_VER_1_3;
		break;
	case IPA_HW_v4_0:
	case IPA_HW_v4_1:
		gsi_ver = GSI_VER_2_0;
		break;
	case IPA_HW_v4_2:
		gsi_ver = GSI_VER_2_2;
		break;
	case IPA_HW_v4_5:
		gsi_ver = GSI_VER_2_5;
		break;
	case IPA_HW_v4_7:
		gsi_ver = GSI_VER_2_7;
		break;
	case IPA_HW_v4_9:
		gsi_ver = GSI_VER_2_9;
		break;
	case IPA_HW_v4_11:
		gsi_ver = GSI_VER_2_11;
		break;
	case IPA_HW_v5_0:
	case IPA_HW_v5_1:
		gsi_ver = GSI_VER_3_0;
		break;
	case IPA_HW_v5_2:
		gsi_ver = GSI_VER_5_2;
		break;
	default:
		IPAERR("No GSI version for ipa type %d\n", ipa_hw_type);
		WARN_ON(1);
		gsi_ver = GSI_VER_ERR;
	}

	IPADBG("GSI version %d\n", gsi_ver);

	return gsi_ver;
}

static int ipa3_gsi_pre_fw_load_init(void)
{
	int result;

	result = gsi_configure_regs(
		ipa3_res.ipa_mem_base,
		ipa3_ctx->gsi_ver);

	if (result) {
		IPAERR("Failed to configure GSI registers\n");
		return -EINVAL;
	}

	return 0;
}

static int ipa3_alloc_gsi_channel(void)
{
	const struct ipa_gsi_ep_config *gsi_ep_cfg;
	enum ipa_client_type type;
	int code = 0;
	int ret = 0;
	int i;

	for (i = 0; i < ipa3_ctx->ipa_num_pipes; i++) {
		type = ipa3_get_client_by_pipe(i);
		gsi_ep_cfg = ipa3_get_gsi_ep_info(type);
		IPADBG("for ep %d client is %d\n", i, type);
		if (!gsi_ep_cfg)
			continue;

		ret = gsi_alloc_channel_ee(gsi_ep_cfg->ipa_gsi_chan_num,
					gsi_ep_cfg->ee, &code);
		if (ret == GSI_STATUS_SUCCESS) {
			IPADBG("alloc gsi ch %d ee %d with code %d\n",
					gsi_ep_cfg->ipa_gsi_chan_num,
					gsi_ep_cfg->ee,
					code);
		} else {
			IPAERR("failed to alloc ch %d ee %d code %d\n",
					gsi_ep_cfg->ipa_gsi_chan_num,
					gsi_ep_cfg->ee,
					code);
			return ret;
		}
	}
	return ret;
}

static inline void ipa3_enable_napi_lan_rx(void)
{
	if (ipa3_ctx->lan_rx_napi_enable)
		napi_enable(&ipa3_ctx->napi_lan_rx);
}

static inline void ipa3_disable_napi_lan_rx(void)
{
	if (ipa3_ctx->lan_rx_napi_enable)
		napi_disable(&ipa3_ctx->napi_lan_rx);
}

static inline void ipa3_register_to_fmwk(void)
{
	struct ipa_core_data data;

	data.ipa_tx_dp = ipa3_tx_dp;
	data.ipa_get_hw_type = ipa_get_hw_type_internal;
	data.ipa_is_vlan_mode = ipa3_is_vlan_mode;
	data.ipa_get_smmu_params = ipa3_get_smmu_params;
	data.ipa_get_lan_rx_napi = ipa3_get_lan_rx_napi;
	data.ipa_dma_init = ipa3_dma_init;
	data.ipa_dma_enable = ipa3_dma_enable;
	data.ipa_dma_disable = ipa3_dma_disable;
	data.ipa_dma_sync_memcpy = ipa3_dma_sync_memcpy;
	data.ipa_dma_async_memcpy = ipa3_dma_async_memcpy;
	data.ipa_dma_destroy = ipa3_dma_destroy;
	data.ipa_get_ep_mapping = ipa3_get_ep_mapping;
	data.ipa_send_msg = ipa3_send_msg;
	data.ipa_free_skb = ipa3_free_skb;
	data.ipa_setup_sys_pipe = ipa3_setup_sys_pipe;
	data.ipa_teardown_sys_pipe = ipa3_teardown_sys_pipe;
	data.ipa_get_wdi_stats = ipa3_get_wdi_stats;
	data.ipa_uc_bw_monitor = ipa3_uc_bw_monitor;
	data.ipa_broadcast_wdi_quota_reach_ind =
		ipa3_broadcast_wdi_quota_reach_ind;
	data.ipa_uc_wdi_get_dbpa = ipa3_uc_wdi_get_dbpa;
	data.ipa_cfg_ep_ctrl = ipa3_cfg_ep_ctrl;
	data.ipa_add_rt_rule = ipa3_add_rt_rule;
	data.ipa_put_rt_tbl = ipa3_put_rt_tbl;
	data.ipa_register_intf = ipa3_register_intf;
	data.ipa_deregister_intf = ipa3_deregister_intf;
	data.ipa_add_hdr = ipa3_add_hdr;
	data.ipa_get_hdr = ipa3_get_hdr;
	data.ipa_del_hdr = ipa3_del_hdr;
	data.ipa_set_aggr_mode = ipa3_set_aggr_mode;
	data.ipa_set_qcncm_ndp_sig = ipa3_set_qcncm_ndp_sig;
	data.ipa_set_single_ndp_per_mbim = ipa3_set_single_ndp_per_mbim;
	data.ipa_add_interrupt_handler = ipa3_add_interrupt_handler;
	data.ipa_restore_suspend_handler = ipa3_restore_suspend_handler;
	data.ipa_get_gsi_ep_info = ipa3_get_gsi_ep_info;
	data.ipa_stop_gsi_channel = ipa3_stop_gsi_channel;
	data.ipa_rmnet_ctl_xmit = ipa3_rmnet_ctl_xmit;
	data.ipa_register_rmnet_ctl_cb = ipa3_register_rmnet_ctl_cb;
	data.ipa_unregister_rmnet_ctl_cb = ipa3_unregister_rmnet_ctl_cb;
	if (ipa3_ctx->use_pm_wrapper) {
		data.ipa_enable_wdi_pipe = ipa_pm_wrapper_enable_wdi_pipe;
		data.ipa_disable_wdi_pipe = ipa_pm_wrapper_disable_pipe;
		data.ipa_connect_wdi_pipe = ipa_pm_wrapper_connect_wdi_pipe;
		data.ipa_disconnect_wdi_pipe = ipa_pm_wrapper_disconnect_wdi_pipe;
	}
	else {
		data.ipa_enable_wdi_pipe = ipa3_enable_wdi_pipe;
		data.ipa_disable_wdi_pipe = ipa3_disable_wdi_pipe;
		data.ipa_connect_wdi_pipe = ipa3_connect_wdi_pipe;
		data.ipa_disconnect_wdi_pipe = ipa3_disconnect_wdi_pipe;
	}
	data.ipa_resume_wdi_pipe = ipa3_resume_wdi_pipe;
	data.ipa_suspend_wdi_pipe = ipa3_suspend_wdi_pipe;
	data.ipa_uc_reg_rdyCB = ipa3_uc_reg_rdyCB;
	data.ipa_uc_dereg_rdyCB = ipa3_uc_dereg_rdyCB;
	data.ipa_rmnet_ll_xmit = ipa3_rmnet_ll_xmit;
	data.ipa_register_rmnet_ll_cb = ipa3_register_rmnet_ll_cb;
	data.ipa_unregister_rmnet_ll_cb = ipa3_unregister_rmnet_ll_cb;
	data.ipa_register_notifier =
		ipa3_register_notifier;
	data.ipa_unregister_notifier =
		ipa3_unregister_notifier;
	data.ipa_add_socksv5_conn = ipa3_add_socksv5_conn;
	data.ipa_del_socksv5_conn = ipa3_del_socksv5_conn;

	if (ipa_fmwk_register_ipa(&data)) {
		IPAERR("couldn't register to IPA framework\n");
	}
}

void ipa3_notify_clients_registered(void)
{
	bool reg = false;

	mutex_lock(&ipa3_ctx->lock);
	if (ipa3_ctx->ipa_initialization_complete)
		reg = true;
	ipa3_ctx->clients_registered = true;
	mutex_unlock(&ipa3_ctx->lock);

	if (reg) {
		IPADBG("register to fmwk\n");
		ipa3_register_to_fmwk();
	}
}
EXPORT_SYMBOL(ipa3_notify_clients_registered);

static void ipa_gsi_map_unmap_gsi_msi_addr(bool map)
{
	struct ipa_smmu_cb_ctx *cb;
	u64 rounddown_addr;
	int res;
	int prot = IOMMU_READ | IOMMU_WRITE | IOMMU_MMIO;

	cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_AP);
	rounddown_addr = rounddown(ipa3_ctx->gsi_msi_addr, PAGE_SIZE);
	if (map) {
		res = ipa3_iommu_map(cb->iommu_domain,
			rounddown_addr, rounddown_addr, PAGE_SIZE, prot);
		if (res) {
			IPAERR("iommu mapping failed for gsi_msi_addr\n");
			ipa_assert();
		}
	} else {
		res = iommu_unmap(cb->iommu_domain, rounddown_addr, PAGE_SIZE);
		if (res)
			IPAERR("smmu unmap for gsi_msi_addr failed %d\n", res);
	}
}


/**
 * ipa3_post_init() - Initialize the IPA Driver (Part II).
 * This part contains all initialization which requires interaction with
 * IPA HW (via GSI).
 *
 * @resource_p:	contain platform specific values from DST file
 * @pdev:	The platform device structure representing the IPA driver
 *
 * Function initialization process:
 * - Initialize endpoints bitmaps
 * - Initialize resource groups min and max values
 * - Initialize filtering lists heads and idr
 * - Initialize interrupts
 * - Register GSI
 * - Setup APPS pipes
 * - Initialize tethering bridge
 * - Initialize IPA debugfs
 * - Initialize IPA uC interface
 * - Initialize WDI interface
 * - Initialize USB interface
 * - Register for panic handler
 * - Trigger IPA ready callbacks (to all subscribers)
 * - Trigger IPA completion object (to all who wait on it)
 */
static int ipa3_post_init(const struct ipa3_plat_drv_res *resource_p,
			  struct device *ipa_dev)
{
	int result;
	struct gsi_per_props gsi_props;
	struct ipa3_uc_hdlrs uc_hdlrs = { 0 };
	struct ipa3_flt_tbl *flt_tbl;
	struct ipa3_flt_tbl_nhash_lcl *lcl_tbl;
	int i;
	struct idr *idr;
	bool reg = false;
	enum ipa_ip_type ip;
#if IS_ENABLED(CONFIG_QCOM_VA_MINIDUMP)
	struct ipa_minidump_data *mini_dump;
#endif

	if (ipa3_ctx == NULL) {
		IPADBG("IPA driver haven't initialized\n");
		return -ENXIO;
	}

	/* Prevent consequent calls from trying to load the FW again. */
	if (ipa3_ctx->ipa_initialization_complete)
		return 0;

	IPADBG("active clients = %d\n",
			atomic_read(&ipa3_ctx->ipa3_active_clients.cnt));
	/* move proxy vote for modem on ipa3_post_init */
	if (ipa3_ctx->ipa_hw_type != IPA_HW_v4_0)
		ipa3_proxy_clk_vote(false);

	/* The following will retrieve and save the gsi fw version */
	ipa_save_gsi_ver();

	/*
	 * IPA version 3.0 IPAHAL initialized at pre_init as there is no SMMU.
	 * In normal mode need to wait until SMMU is attached and
	 * thus initialization done here
	 */
	if (ipa3_ctx->ipa_hw_type != IPA_HW_v3_0) {
		if (ipahal_init(ipa3_ctx->ipa_hw_type, ipa3_ctx->mmio,
				ipa3_ctx->ipa_cfg_offset, ipa3_ctx->pdev)) {
			IPAERR("fail to init ipahal\n");
			result = -EFAULT;
			goto fail_ipahal;
		}
	}

	result = ipa3_init_hw();
	if (result) {
		IPAERR(":error initializing HW\n");
		result = -ENODEV;
		goto fail_init_hw;
	}
	IPADBG("IPA HW initialization sequence completed");

	ipa3_ctx->ipa_num_pipes = ipa3_get_num_pipes();
	IPADBG("IPA Pipes num %u\n", ipa3_ctx->ipa_num_pipes);
	if (ipa3_ctx->ipa_num_pipes > IPA5_MAX_NUM_PIPES) {
		IPAERR("IPA has more pipes then supported has %d, max %d\n",
			ipa3_ctx->ipa_num_pipes, IPA5_MAX_NUM_PIPES);
		result = -ENODEV;
		goto fail_init_hw;
	}

	ipa3_ctx->ctrl->ipa_sram_read_settings();
	IPADBG("SRAM, size: 0x%x, restricted bytes: 0x%x\n",
		ipa3_ctx->smem_sz, ipa3_ctx->smem_restricted_bytes);

	IPADBG("ip4_rt_hash=%u ip4_rt_nonhash=%u\n",
		ipa3_ctx->rt_tbl_hash_lcl[IPA_IP_v4], ipa3_ctx->rt_tbl_nhash_lcl[IPA_IP_v4]);

	IPADBG("ip6_rt_hash=%u ip6_rt_nonhash=%u\n",
		ipa3_ctx->rt_tbl_hash_lcl[IPA_IP_v6], ipa3_ctx->rt_tbl_nhash_lcl[IPA_IP_v6]);

	IPADBG("ip4_flt_hash=%u ip4_flt_nonhash=%u\n",
		ipa3_ctx->flt_tbl_hash_lcl[IPA_IP_v4],
		ipa3_ctx->flt_tbl_nhash_lcl[IPA_IP_v4]);

	IPADBG("ip6_flt_hash=%u ip6_flt_nonhash=%u\n",
		ipa3_ctx->flt_tbl_hash_lcl[IPA_IP_v6],
		ipa3_ctx->flt_tbl_nhash_lcl[IPA_IP_v6]);

	if (ipa3_ctx->smem_reqd_sz > ipa3_ctx->smem_sz) {
		IPAERR("SW expect more core memory, needed %d, avail %d\n",
			ipa3_ctx->smem_reqd_sz, ipa3_ctx->smem_sz);
		result = -ENOMEM;
		goto fail_init_hw;
	}

	result = ipa3_allocate_dma_task_for_gsi();
	if (result) {
		IPAERR("failed to allocate dma task\n");
		goto fail_dma_task;
	}

	result = ipa3_allocate_coal_close_frame();
	if (result) {
		IPAERR("failed to allocate coal frame cmd\n");
		goto fail_coal_frame;
	}

	if (ipa3_nat_ipv6ct_init_devices()) {
		IPAERR("unable to init NAT and IPv6CT devices\n");
		result = -ENODEV;
		goto fail_nat_ipv6ct_init_dev;
	}

	result = ipa3_alloc_pkt_init();
	if (result) {
		IPAERR("Failed to alloc pkt_init payload\n");
		result = -ENODEV;
		goto fail_alloc_pkt_init;
	}

	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v5_0) {
		result = ipa_alloc_pkt_init_ex();
		if (result) {
			IPAERR("Failed to alloc pkt_init_ex payload\n");
			result = -ENODEV;
			goto fail_alloc_pkt_init_ex;
		}
	}

	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v3_5)
		ipa3_enable_dcd();

	/*
	 * indication whether working in MHI config or non MHI config is given
	 * in ipa3_write which is launched before ipa3_post_init. i.e. from
	 * this point it is safe to use ipa3_ep_mapping array and the correct
	 * entry will be returned from ipa3_get_hw_type_index()
	 */
	ipa_init_ep_flt_bitmap();
	IPADBG("EP with flt support bitmap 0x%llx (%u pipes)\n",
		ipa3_ctx->ep_flt_bitmap, ipa3_ctx->ep_flt_num);

	/* Assign resource limitation to each group */
	ipa3_set_resorce_groups_min_max_limits();

	/* Initialize general resource group parameters */
	ipa3_set_resorce_groups_config();

	idr = &(ipa3_ctx->flt_rule_ids[IPA_IP_v4]);
	idr_init(idr);
	idr = &(ipa3_ctx->flt_rule_ids[IPA_IP_v6]);
	idr_init(idr);

	INIT_LIST_HEAD(&ipa3_ctx->flt_tbl_nhash_lcl_list[IPA_IP_v4]);
	INIT_LIST_HEAD(&ipa3_ctx->flt_tbl_nhash_lcl_list[IPA_IP_v6]);

	for (i = 0; i < ipa3_ctx->ipa_num_pipes; i++) {
		if (!ipa_is_ep_support_flt(i))
			continue;

		for (ip = IPA_IP_v4; ip < IPA_IP_MAX; ip++) {
			flt_tbl = &ipa3_ctx->flt_tbl[i][ip];
			INIT_LIST_HEAD(&flt_tbl->head_flt_rule_list);
			flt_tbl->in_sys[IPA_RULE_HASHABLE] = !ipa3_ctx->flt_tbl_hash_lcl[ip];

			/*	For ETH client place Non-Hash FLT table in SRAM if allowed, for
				all other EPs always place the table in DDR */
			if (ipa3_ctx->flt_tbl_nhash_lcl[ip] &&
			    (IPA_CLIENT_IS_ETH_PROD(i) ||
			     ((ipa3_ctx->ipa3_hw_mode == IPA_HW_MODE_TEST) &&
			      (i == ipa3_get_ep_mapping(IPA_CLIENT_TEST_PROD))))) {
				flt_tbl->in_sys[IPA_RULE_NON_HASHABLE] = false;
				lcl_tbl = kcalloc(1, sizeof(struct ipa3_flt_tbl_nhash_lcl),
						  GFP_KERNEL);
				WARN_ON(lcl_tbl);
				if (likely(lcl_tbl)) {
					lcl_tbl->tbl = flt_tbl;
					/* Add to the head of the list, to be pulled first */
					list_add(&lcl_tbl->link,
						 &ipa3_ctx->flt_tbl_nhash_lcl_list[ip]);
				}
			} else
				flt_tbl->in_sys[IPA_RULE_NON_HASHABLE] = true;

			/* Init force sys to false */
			flt_tbl->force_sys[IPA_RULE_HASHABLE] = false;
			flt_tbl->force_sys[IPA_RULE_NON_HASHABLE] = false;

			flt_tbl->rule_ids = &ipa3_ctx->flt_rule_ids[ip];
		}
	}

	if (!ipa3_ctx->apply_rg10_wa) {
		result = ipa3_init_interrupts();
		if (result) {
			IPAERR("ipa initialization of interrupts failed\n");
			result = -ENODEV;
			goto fail_init_interrupts;
		}
	} else {
		IPADBG("Initialization of ipa interrupts skipped\n");
	}

	/*
	 * Disable prefetch for USB or MHI at IPAv3.5/IPA.3.5.1
	 * This is to allow MBIM to work.
	 */
	if ((ipa3_ctx->ipa_hw_type >= IPA_HW_v3_5
		&& ipa3_ctx->ipa_hw_type < IPA_HW_v4_0) &&
		(!ipa3_ctx->ipa_config_is_mhi))
		ipa3_disable_prefetch(IPA_CLIENT_USB_CONS);

	if ((ipa3_ctx->ipa_hw_type >= IPA_HW_v3_5
		&& ipa3_ctx->ipa_hw_type < IPA_HW_v4_0) &&
		(ipa3_ctx->ipa_config_is_mhi))
		ipa3_disable_prefetch(IPA_CLIENT_MHI_CONS);

	memset(&gsi_props, 0, sizeof(gsi_props));
	gsi_props.ver = ipa3_ctx->gsi_ver;
	gsi_props.ee = resource_p->ee;
	gsi_props.intr = GSI_INTR_IRQ;
	gsi_props.phys_addr = resource_p->transport_mem_base;
	gsi_props.size = resource_p->transport_mem_size;
	if (ipa3_ctx->ipa3_hw_mode == IPA_HW_MODE_EMULATION) {
		gsi_props.irq = resource_p->emulator_irq;
		gsi_props.emulator_intcntrlr_client_isr = ipa3_get_isr();
		gsi_props.emulator_intcntrlr_addr =
		    resource_p->emulator_intcntrlr_mem_base;
		gsi_props.emulator_intcntrlr_size =
		    resource_p->emulator_intcntrlr_mem_size;
	} else {
		gsi_props.irq = resource_p->transport_irq;
	}
	gsi_props.notify_cb = ipa_gsi_notify_cb;
	gsi_props.req_clk_cb = NULL;
	gsi_props.rel_clk_cb = NULL;
	gsi_props.clk_status_cb = ipa3_active_clks_status;
	gsi_props.enable_clk_bug_on = ipa3_handle_gsi_differ_irq;

	if (ipa3_ctx->ipa_config_is_mhi) {
		gsi_props.mhi_er_id_limits_valid = true;
		gsi_props.mhi_er_id_limits[0] = resource_p->mhi_evid_limits[0];
		gsi_props.mhi_er_id_limits[1] = resource_p->mhi_evid_limits[1];
	}
	gsi_props.skip_ieob_mask_wa = resource_p->skip_ieob_mask_wa;
	gsi_props.tx_poll = resource_p->tx_poll;

	result = gsi_register_device(&gsi_props,
		&ipa3_ctx->gsi_dev_hdl);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR(":gsi register error - %d\n", result);
		result = -ENODEV;
		goto fail_register_device;
	}
	IPADBG("IPA gsi is registered\n");
	/* GSI 2.2 requires to allocate all EE GSI channel
	 * during device bootup.
	 */
	if (gsi_props.ver == GSI_VER_2_2) {
		result = ipa3_alloc_gsi_channel();
		if (result) {
			IPAERR("Failed to alloc the GSI channels\n");
			result = -ENODEV;
			goto fail_alloc_gsi_channel;
		}
	}

	/* setup the AP-IPA pipes */
	if (ipa3_setup_apps_pipes()) {
		IPAERR(":failed to setup IPA-Apps pipes\n");
		result = -ENODEV;
		goto fail_setup_apps_pipes;
	}
	IPADBG("IPA GPI pipes were connected\n");

	if (ipa3_ctx->use_ipa_teth_bridge) {
		/* Initialize the tethering bridge driver */
		result = ipa3_teth_bridge_driver_init();
		if (result) {
			IPAERR(":teth_bridge init failed (%d)\n", -result);
			result = -ENODEV;
			goto fail_teth_bridge_driver_init;
		}
		IPADBG("teth_bridge initialized");
	}

	result = ipa3_uc_interface_init();
	if (result)
		IPAERR(":ipa Uc interface init failed (%d)\n", -result);
	else
		IPADBG(":ipa Uc interface init ok\n");
	uc_hdlrs.ipa_uc_loaded_hdlr = ipa3_uc_is_loaded;
	uc_hdlrs.ipa_uc_holb_enabled_hdlr = ipa3_uc_holb_client_handler;
	ipa3_uc_register_handlers(IPA_HW_FEATURE_COMMON, &uc_hdlrs);

	if (ipa3_ctx->use_tput_est_ep) {
		result = ipa3_setup_tput_pipe();
		if (result)
			IPAERR(":Failed configuring throughput moniter ep\n");
		else
			IPADBG(":Throughput moniter ep configured\n");
	}

	result = ipa3_wdi_init();
	if (result)
		IPAERR(":wdi init failed (%d)\n", -result);
	else
		IPADBG(":wdi init ok\n");

	result = ipa3_wigig_init_i();
	if (result)
		IPAERR(":wigig init failed (%d)\n", -result);
	else
		IPADBG(":wigig init ok\n");

	result = ipa3_ntn_init();
	if (result)
		IPAERR(":ntn init failed (%d)\n", -result);
	else
		IPADBG(":ntn init ok\n");

	result = ipa_hw_stats_init();
	if (result)
		IPAERR("fail to init stats %d\n", result);
	else
		IPADBG(":stats init ok\n");

	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_5) {
		result = ipa_init_flt_rt_stats();
		if (result)
			IPAERR("fail to init FnR stats %d\n", result);
		else
			IPADBG(":FnR stats init ok\n");
	}

	result = ipa_drop_stats_init();
	if (result)
		IPAERR("fail to init stats %d\n", result);
	else
		IPADBG(":stats init ok\n");

	/* 1st ipa3_panic_notifier*/
	ipa3_register_panic_hdlr();

	ipa3_debugfs_init();

	result = ipa_mpm_init();
	if (result)
		IPAERR("fail to init mpm %d\n", result);
	else
		IPADBG(":mpm init init ok\n");

	mutex_lock(&ipa3_ctx->lock);
	ipa3_ctx->ipa_initialization_complete = true;
	if (ipa3_ctx->clients_registered)
		reg = true;
	mutex_unlock(&ipa3_ctx->lock);
	ipa3_enable_napi_lan_rx();
	if (reg) {
		IPADBG("register to fmwk\n");
		ipa3_register_to_fmwk();
	}

	/* init uc-activation tbl*/
	ipa3_setup_uc_act_tbl();

#ifdef CONFIG_DEEPSLEEP
	if (!ipa3_is_ready())
		ipa_fmwk_deepsleep_exit_ipa();
#endif
	complete_all(&ipa3_ctx->init_completion_obj);

	ipa_ut_module_init();

	/* Query MSI address. */
	gsi_query_device_msi_addr(&ipa3_ctx->gsi_msi_addr);
	/* Map the MSI addresses for the GSI to access, for LL and QMAP FC pipe */
	if (ipa3_ctx->gsi_msi_addr)
		ipa_gsi_map_unmap_gsi_msi_addr(true);

	if(!ipa_spearhead_stats_init())
		IPADBG("Fail to init spearhead ipa lnx module");

	pr_info("IPA driver initialization was successful.\n");
#if IS_ENABLED(CONFIG_QCOM_VA_MINIDUMP)
	/*Adding ipa3_ctx pointer to minidump list*/
	mini_dump = (struct ipa_minidump_data *)kzalloc(sizeof(struct ipa_minidump_data), GFP_KERNEL);
	if (mini_dump != NULL) {
		strlcpy(mini_dump->data.owner, "ipa3_ctx", sizeof(mini_dump->data.owner));
		mini_dump->data.vaddr = (unsigned long)(ipa3_ctx);
		mini_dump->data.size = sizeof(struct ipa3_context);
		list_add(&mini_dump->entry, &ipa3_ctx->minidump_list_head);
	}
#endif
	return 0;

fail_teth_bridge_driver_init:
	ipa3_teardown_apps_pipes();
fail_alloc_gsi_channel:
fail_setup_apps_pipes:
	gsi_deregister_device(ipa3_ctx->gsi_dev_hdl, false);
fail_register_device:
	ipa3_destroy_flt_tbl_idrs();
fail_init_interrupts:
	ipa3_remove_interrupt_handler(IPA_TX_SUSPEND_IRQ);
	ipa3_interrupts_destroy(ipa3_res.ipa_irq, &ipa3_ctx->master_pdev->dev);
	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v5_0)
		ipa3_free_pkt_init_ex();
fail_alloc_pkt_init_ex:
	ipa3_free_pkt_init();
fail_alloc_pkt_init:
	ipa3_nat_ipv6ct_destroy_devices();
fail_nat_ipv6ct_init_dev:
	ipa3_free_coal_close_frame();
fail_coal_frame:
	ipa3_free_dma_task_for_gsi();
fail_dma_task:
fail_init_hw:
	ipahal_destroy();
fail_ipahal:
	ipa3_proxy_clk_unvote();

	return result;
}

static int ipa3_manual_load_ipa_fws(void)
{
	int result;
	const struct firmware *fw;
	const char *path = IPA_FWS_PATH;
	enum gsi_ver gsi_ver = ipa3_ctx->gsi_ver;

	if (ipa3_ctx->ipa3_hw_mode == IPA_HW_MODE_EMULATION) {
		switch (ipa3_get_emulation_type()) {
		case IPA_HW_v3_5_1:
			path = IPA_FWS_PATH_3_5_1;
			break;
		case IPA_HW_v4_0:
			path = IPA_FWS_PATH_4_0;
			break;
		case IPA_HW_v4_5:
			path = IPA_FWS_PATH_4_5;
			break;
		default:
			break;
		}
	}

	IPADBG("Manual FW loading (%s) process initiated\n", path);

	result = request_firmware(&fw, path, ipa3_ctx->cdev.dev);
	if (result < 0) {
		IPAERR("request_firmware failed, error %d\n", result);
		return result;
	}

	IPADBG("FWs are available for loading\n");

	if (ipa3_ctx->ipa3_hw_mode == IPA_HW_MODE_EMULATION) {
		result = emulator_load_fws(fw,
			ipa3_res.transport_mem_base,
			ipa3_res.transport_mem_size,
			gsi_ver);
	} else {
		result = ipa3_load_fws(fw, ipa3_res.transport_mem_base,
			gsi_ver);
	}

	if (result) {
		IPAERR("Manual IPA FWs loading has failed\n");
		release_firmware(fw);
		return result;
	}

	result = gsi_enable_fw(ipa3_res.transport_mem_base,
				ipa3_res.transport_mem_size,
				gsi_ver);
	if (result) {
		IPAERR("Failed to enable GSI FW\n");
		release_firmware(fw);
		return result;
	}

	release_firmware(fw);

	IPADBG("Manual FW loading process is complete\n");

	return 0;
}

#if IS_ENABLED(CONFIG_QCOM_MDT_LOADER)
static int ipa_firmware_load(const char *sub_sys)
{
	const struct firmware *fw;
	char fw_name[32];
	struct device_node *node;
	struct resource res;
	phys_addr_t phys;
	ssize_t size;
	void *virt;
	int ret, index, pas_id;
	struct device *dev = &ipa3_ctx->master_pdev->dev;

	index = of_property_match_string(dev->of_node, "firmware-names",
					 sub_sys);
	if (index < 0) {
		pr_err("#####Not able to match firmware names prorperty\n");
		return -EINVAL;
	}

	node = of_parse_phandle(dev->of_node, "memory-regions", index);
	if (!node) {
		dev_err(dev, "DT error getting \"memory-region\" property\n");
		return -EINVAL;
	}

	ret = of_address_to_resource(node, 0, &res);
	if (ret) {
		dev_err(dev, "error %d getting \"memory-region\" resource\n",
			ret);
		return ret;
	}

	scnprintf(fw_name, ARRAY_SIZE(fw_name), "%s.mdt", sub_sys);
	ret = of_property_read_u32_index(dev->of_node, "pas-ids", index,
					  &pas_id);

	ret = request_firmware(&fw, fw_name, dev);
	if (ret) {
		dev_err(dev, "error %d requesting \"%s\"\n", ret, fw_name);
		return ret;
	}

	phys = res.start;
	size = (size_t)resource_size(&res);
	virt = memremap(phys, size, MEMREMAP_WC);
	if (!virt) {
		dev_err(dev, "unable to remap firmware memory\n");
		ret = -ENOMEM;
		goto out_release_firmware;
	}

	ret = qcom_mdt_load(dev, fw, fw_name, pas_id, virt, phys, size, NULL);
	if (ret)
		dev_err(dev, "error %d loading \"%s\"\n", ret, fw_name);
	else if ((ret = qcom_scm_pas_auth_and_reset(pas_id)))
		dev_err(dev, "error %d authenticating \"%s\"\n", ret, fw_name);

	memunmap(virt);

out_release_firmware:
	release_firmware(fw);

	return ret;
}

static int ipa3_mdt_load_ipa_fws(const char *sub_sys)
{
	int ret;

	IPADBG("MDT FW loading process initiated sub_sys=%s\n",
		sub_sys);

	ret = ipa_firmware_load(sub_sys);
	if (ret < 0) {
		IPAERR("Unable to MDT load FW for sub_sys=%s\n", sub_sys);
		return -EINVAL;
	}

	IPADBG("MDT FW loading process is complete sub_sys=%s\n", sub_sys);
	return 0;
}
#else /* IS_ENABLED(CONFIG_QCOM_MDT_LOADER) */

static int ipa3_pil_load_ipa_fws(const char *sub_sys)
{

	IPADBG("PIL FW loading process initiated sub_sys=%s\n",
		sub_sys);

	ipa3_ctx->subsystem_get_retval = subsystem_get(sub_sys);
	if (IS_ERR_OR_NULL(ipa3_ctx->subsystem_get_retval)) {
		IPAERR("Unable to PIL load FW for sub_sys=%s\n", sub_sys);
		return -EINVAL;
	}

	IPADBG("PIL FW loading process is complete sub_sys=%s\n", sub_sys);
	return 0;
}
#endif /* IS_ENABLED(CONFIG_QCOM_MDT_LOADER) */

#ifdef CONFIG_DEEPSLEEP
static int ipa3_pil_unload_ipa_fws(void)
{

	IPADBG("PIL FW unloading process initiated sub_sys\n");

	if (ipa3_ctx->subsystem_get_retval)
		subsystem_put(ipa3_ctx->subsystem_get_retval);

	IPADBG("PIL FW unloading process is complete sub_sys\n");
	return 0;
}
#endif

static void ipa3_load_ipa_fw(struct work_struct *work)
{
	int result;

	IPADBG("Entry\n");

	IPA_ACTIVE_CLIENTS_INC_SIMPLE();
	
	result = ipa3_attach_to_smmu();
	if (result) {
		IPAERR("IPA attach to smmu failed %d\n", result);
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		queue_delayed_work(ipa3_ctx->transport_power_mgmt_wq,
			&ipa3_fw_load_failure_handle,
			msecs_to_jiffies(DELAY_BEFORE_FW_LOAD));
		return;
	}

	if (ipa3_ctx->ipa3_hw_mode != IPA_HW_MODE_EMULATION &&
	    ((ipa3_ctx->platform_type != IPA_PLAT_TYPE_MDM) ||
	    (ipa3_ctx->ipa_hw_type >= IPA_HW_v3_5))) {
		/* some targets sharing same lunch option but
		 * using different signing images, adding support to
		 * load specific FW image to based on dt entry.
		 */
#if IS_ENABLED(CONFIG_QCOM_MDT_LOADER)
		if (ipa3_ctx->gsi_fw_file_name)
			result = ipa3_mdt_load_ipa_fws(
						ipa3_ctx->gsi_fw_file_name);
		else
			result = ipa3_mdt_load_ipa_fws(IPA_SUBSYSTEM_NAME);
#else /* IS_ENABLED(CONFIG_QCOM_MDT_LOADER) */
		if (ipa3_ctx->gsi_fw_file_name)
			result = ipa3_pil_load_ipa_fws(
						ipa3_ctx->gsi_fw_file_name);
		else
			result = ipa3_pil_load_ipa_fws(IPA_SUBSYSTEM_NAME);
#endif /* IS_ENABLED(CONFIG_QCOM_MDT_LOADER) */
	} else {
		result = ipa3_manual_load_ipa_fws();
	}


	if (result) {

		ipa3_ctx->ipa_pil_load++;
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		IPADBG("IPA firmware loading deffered to a work queue\n");
		queue_delayed_work(ipa3_ctx->transport_power_mgmt_wq,
			&ipa3_fw_load_failure_handle,
			msecs_to_jiffies(DELAY_BEFORE_FW_LOAD));
		return;
	}
	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
	mutex_lock(&ipa3_ctx->fw_load_data.lock);
	ipa3_ctx->fw_load_data.state = IPA_FW_LOAD_STATE_LOADED;
	mutex_unlock(&ipa3_ctx->fw_load_data.lock);
	pr_info("IPA FW loaded successfully\n");

	result = ipa3_post_init(&ipa3_res, ipa3_ctx->cdev.dev);
	if (result) {
		IPAERR("IPA post init failed %d\n", result);
		return;
	}

	if (ipa3_ctx->platform_type == IPA_PLAT_TYPE_APQ &&
		ipa3_ctx->ipa3_hw_mode != IPA_HW_MODE_VIRTUAL &&
		ipa3_ctx->ipa3_hw_mode != IPA_HW_MODE_EMULATION) {

		IPADBG("Loading IPA uC via PIL\n");

		/* Unvoting will happen when uC loaded event received. */
		ipa3_proxy_clk_vote(false);

#if IS_ENABLED(CONFIG_QCOM_MDT_LOADER)
		if (ipa3_ctx->uc_fw_file_name)
			result = ipa3_mdt_load_ipa_fws(
						ipa3_ctx->uc_fw_file_name);
		else
			result = ipa3_mdt_load_ipa_fws(IPA_UC_SUBSYSTEM_NAME);
#else /* IS_ENABLED(CONFIG_QCOM_MDT_LOADER) */
		if (ipa3_ctx->uc_fw_file_name)
			result = ipa3_pil_load_ipa_fws(
						ipa3_ctx->uc_fw_file_name);
		else
			result = ipa3_pil_load_ipa_fws(IPA_UC_SUBSYSTEM_NAME);
#endif /* IS_ENABLED(CONFIG_QCOM_MDT_LOADER) */
		if (result) {
			IPAERR("IPA uC loading process has failed result=%d\n",
				result);
			ipa3_proxy_clk_unvote();
			return;
		}
		IPADBG("IPA uC loading succeeded\n");
	}
}

static void ipa_fw_load_sm_handle_event(enum ipa_fw_load_event ev)
{
	mutex_lock(&ipa3_ctx->fw_load_data.lock);

	IPADBG("state=%d event=%d\n", ipa3_ctx->fw_load_data.state, ev);

	if (ev == IPA_FW_LOAD_EVNT_FWFILE_READY) {
		if (ipa3_ctx->fw_load_data.state == IPA_FW_LOAD_STATE_INIT) {
			ipa3_ctx->fw_load_data.state =
				IPA_FW_LOAD_STATE_FWFILE_READY;
			goto out;
		}
		if (ipa3_ctx->fw_load_data.state ==
			IPA_FW_LOAD_STATE_SMMU_DONE) {
			ipa3_ctx->fw_load_data.state =
				IPA_FW_LOAD_STATE_LOAD_READY;
			goto sched_fw_load;
		}
		IPAERR("ignore multiple requests to load FW\n");
		goto out;
	}
	if (ev == IPA_FW_LOAD_EVNT_SMMU_DONE) {
		if (ipa3_ctx->fw_load_data.state == IPA_FW_LOAD_STATE_INIT) {
			ipa3_ctx->fw_load_data.state =
				IPA_FW_LOAD_STATE_SMMU_DONE;
			goto sched_fw_load;
		}
		if (ipa3_ctx->fw_load_data.state ==
			IPA_FW_LOAD_STATE_FWFILE_READY) {
			ipa3_ctx->fw_load_data.state =
				IPA_FW_LOAD_STATE_LOAD_READY;
			goto sched_fw_load;
		}
		IPAERR("ignore multiple smmu done events\n");
		goto out;
	}
	IPAERR("invalid event ev=%d\n", ev);
	mutex_unlock(&ipa3_ctx->fw_load_data.lock);
	ipa_assert();
	return;

out:
	mutex_unlock(&ipa3_ctx->fw_load_data.lock);
	return;

sched_fw_load:
	IPADBG("Scheduled a work to load IPA FW\n");
	mutex_unlock(&ipa3_ctx->fw_load_data.lock);
	queue_work(ipa3_ctx->transport_power_mgmt_wq,
		&ipa3_fw_loading_work);
}

static ssize_t ipa3_write(struct file *file, const char __user *buf,
			  size_t count, loff_t *ppos)
{
	unsigned long missing;

	char dbg_buff[32] = { 0 };
	int i = 0;

	if (count >= sizeof(dbg_buff))
		return -EFAULT;

	missing = copy_from_user(dbg_buff, buf, count);

	if (missing) {
		IPAERR("Unable to copy data from user\n");
		return -EFAULT;
	}

	if (count > 0)
		dbg_buff[count] = '\0';

	IPADBG("user input string %s\n", dbg_buff);

	/*Ignore empty ipa_config file*/
	for (i = 0 ; i < count ; ++i) {
		if (!isspace(dbg_buff[i]))
			break;
	}

	if (i == count) {
		IPADBG("Empty ipa_config file\n");
		return count;
	}

	/* Check MHI configuration on MDM devices */
	if (ipa3_ctx->platform_type == IPA_PLAT_TYPE_MDM) {

		/* todo in future: change vlan_mode_iface from bool to enum
		 * and support double vlan for all ifaces
		 */
		if (strnstr(dbg_buff, "double-vlan", strlen(dbg_buff)))
			ipa3_ctx->is_eth_double_vlan_mode = true;

		if (strnstr(dbg_buff, "vlan", strlen(dbg_buff))) {
			if (strnstr(dbg_buff, STR_ETH_IFACE, strlen(dbg_buff)))
				ipa3_ctx->vlan_mode_iface[IPA_VLAN_IF_EMAC] = true;
#if IPA_ETH_API_VER >= 2
			/* In Dual NIC mode we get "vlan: eth [eth0|eth1] [eth0|eth1]?" while device name is
			   "eth0" in legacy so, we set it to false to diffrentiate Dual NIC from legacy */
			if (strnstr(dbg_buff, STR_ETH0_IFACE, strlen(dbg_buff))) {
				ipa3_ctx->vlan_mode_iface[IPA_VLAN_IF_ETH0] = true;
				ipa3_ctx->vlan_mode_iface[IPA_VLAN_IF_EMAC] = false;
			}
			if (strnstr(dbg_buff, STR_ETH1_IFACE, strlen(dbg_buff))){
				ipa3_ctx->vlan_mode_iface[IPA_VLAN_IF_ETH1] = true;
				ipa3_ctx->vlan_mode_iface[IPA_VLAN_IF_EMAC] = false;
			}
#endif
			if (strnstr(dbg_buff, STR_RNDIS_IFACE, strlen(dbg_buff)))
				ipa3_ctx->vlan_mode_iface[IPA_VLAN_IF_RNDIS] = true;
			if (strnstr(dbg_buff, STR_ECM_IFACE, strlen(dbg_buff)))
				ipa3_ctx->vlan_mode_iface[IPA_VLAN_IF_ECM] = true;

			/*
			 * when vlan mode is passed to our dev we expect
			 * another write
			 */
			return count;
		}

		/* trim ending newline character if any */
		if (count && (dbg_buff[count - 1] == '\n'))
			dbg_buff[count - 1] = '\0';

		/*
		 * This logic enforeces MHI mode based on userspace input.
		 * Note that MHI mode could be already determined due
		 *  to previous logic.
		 */
		if (!strcasecmp(dbg_buff, "MHI")) {
			ipa3_ctx->ipa_config_is_mhi = true;
		} else if(!strcmp(dbg_buff, "DBS")) {
			ipa3_ctx->is_wdi3_tx1_needed = true;
		} else if (strcmp(dbg_buff, "1")) {
			IPAERR("got invalid string %s not loading FW\n",
				dbg_buff);
			return count;
		}
		pr_info("IPA is loading with %sMHI configuration\n",
			ipa3_ctx->ipa_config_is_mhi ? "" : "non ");
	}

	/* Prevent consequent calls from trying to load the FW again. */
	if (ipa3_is_ready())
		return count;

	ipa_fw_load_sm_handle_event(IPA_FW_LOAD_EVNT_FWFILE_READY);

	return count;
}

/**
 * ipa3_tz_unlock_reg - Unlocks memory regions so that they become accessible
 *	from AP.
 * @reg_info - Pointer to array of memory regions to unlock
 * @num_regs - Number of elements in the array
 *
 * Converts the input array of regions to a struct that TZ understands and
 * issues an SCM call.
 * Also flushes the memory cache to DDR in order to make sure that TZ sees the
 * correct data structure.
 *
 * Returns: 0 on success, negative on failure
 */
int ipa3_tz_unlock_reg(struct ipa_tz_unlock_reg_info *reg_info, u16 num_regs)
{
	int i, ret;
	compat_size_t size;
	struct tz_smmu_ipa_protect_region_iovec_s *ipa_tz_unlock_vec;
	struct tz_smmu_ipa_protect_region_s cmd_buf;

	if (reg_info ==  NULL || num_regs == 0) {
		IPAERR("Bad parameters\n");
		return -EFAULT;
	}

	size = num_regs * sizeof(struct tz_smmu_ipa_protect_region_iovec_s);
	ipa_tz_unlock_vec = kzalloc(PAGE_ALIGN(size), GFP_KERNEL);
	if (ipa_tz_unlock_vec == NULL)
		return -ENOMEM;

	for (i = 0; i < num_regs; i++) {
		ipa_tz_unlock_vec[i].input_addr = reg_info[i].reg_addr ^
			(reg_info[i].reg_addr & 0xFFF);
		ipa_tz_unlock_vec[i].output_addr = reg_info[i].reg_addr ^
			(reg_info[i].reg_addr & 0xFFF);
		ipa_tz_unlock_vec[i].size = reg_info[i].size;
		ipa_tz_unlock_vec[i].attr = IPA_TZ_UNLOCK_ATTRIBUTE;
	}

	/* pass physical address of command buffer */
	cmd_buf.iovec_buf = virt_to_phys((void *)ipa_tz_unlock_vec);
	cmd_buf.size_bytes = size;

	ret = qcom_scm_mem_protect_region_id(
			virt_to_phys((void *)ipa_tz_unlock_vec),
			size);

	if (ret) {
		IPAERR("scm call SCM_SVC_MP failed: %d\n", ret);
		kfree(ipa_tz_unlock_vec);
		return -EFAULT;
	}
	kfree(ipa_tz_unlock_vec);
	return 0;
}

static int ipa3_alloc_pkt_init(void)
{
	struct ipa_mem_buffer *mem = &ipa3_ctx->pkt_init_mem;
	struct ipahal_imm_cmd_pyld *cmd_pyld;
	struct ipahal_imm_cmd_ip_packet_init cmd = {0};
	int i;

	cmd_pyld = ipahal_construct_imm_cmd(IPA_IMM_CMD_IP_PACKET_INIT,
		&cmd, false);
	if (!cmd_pyld) {
		IPAERR("failed to construct IMM cmd\n");
		return -ENOMEM;
	}
	ipa3_ctx->pkt_init_imm_opcode = cmd_pyld->opcode;

	mem->size = cmd_pyld->len * ipa3_ctx->ipa_num_pipes;
	ipahal_destroy_imm_cmd(cmd_pyld);
	mem->base = dma_alloc_coherent(ipa3_ctx->pdev, mem->size,
		&mem->phys_base, GFP_KERNEL);
	if (!mem->base) {
		IPAERR("failed to alloc DMA buff of size %d\n", mem->size);
		return -ENOMEM;
	}

	memset(mem->base, 0, mem->size);
	for (i = 0; i < ipa3_ctx->ipa_num_pipes; i++) {
		cmd.destination_pipe_index = i;
		cmd_pyld = ipahal_construct_imm_cmd(IPA_IMM_CMD_IP_PACKET_INIT,
			&cmd, false);
		if (!cmd_pyld) {
			IPAERR("failed to construct IMM cmd\n");
			dma_free_coherent(ipa3_ctx->pdev,
				mem->size,
				mem->base,
				mem->phys_base);
			return -ENOMEM;
		}
		memcpy(mem->base + i * cmd_pyld->len, cmd_pyld->data,
			cmd_pyld->len);
		ipa3_ctx->pkt_init_imm[i] = mem->phys_base + i * cmd_pyld->len;
		ipahal_destroy_imm_cmd(cmd_pyld);
	}

	return 0;
}

static void ipa3_free_pkt_init(void)
{
	dma_free_coherent(ipa3_ctx->pdev, ipa3_ctx->pkt_init_mem.size,
		ipa3_ctx->pkt_init_mem.base,
		ipa3_ctx->pkt_init_mem.phys_base);
}

static void ipa3_free_pkt_init_ex(void)
{
	dma_free_coherent(ipa3_ctx->pdev, ipa3_ctx->pkt_init_ex_mem.size,
		ipa3_ctx->pkt_init_ex_mem.base,
		ipa3_ctx->pkt_init_ex_mem.phys_base);
}

static int ipa_alloc_pkt_init_ex(void)
{
	struct ipa_mem_buffer *mem = &ipa3_ctx->pkt_init_ex_mem;
	struct ipahal_imm_cmd_pyld *cmd_pyld;
	struct ipahal_imm_cmd_ip_packet_init_ex cmd = {0};
	struct ipahal_imm_cmd_ip_packet_init_ex cmd_mask = {0};
	int result = 0;

	cmd_pyld = ipahal_construct_imm_cmd(IPA_IMM_CMD_IP_PACKET_INIT_EX,
		&cmd, false);
	if (!cmd_pyld) {
		IPAERR("failed to construct IMM cmd\n");
		return -ENOMEM;
	}
	ipa3_ctx->pkt_init_ex_imm_opcode = cmd_pyld->opcode;

	/* one cmd for each pipe for ULSO + one common for ICMP */
	mem->size = cmd_pyld->len * (ipa3_ctx->ipa_num_pipes + 1);
	mem->base = dma_alloc_coherent(ipa3_ctx->pdev, mem->size,
		&mem->phys_base, GFP_KERNEL);
	if (!mem->base) {
		IPAERR("failed to alloc DMA buff of size %d\n", mem->size);
		result = -ENOMEM;
		goto free_imm;
	}

	memset(mem->base, 0, mem->size);
	cmd.frag_disable = true;
	cmd_mask.frag_disable = true;
	cmd.nat_disable = true;
	cmd_mask.nat_disable = true;
	cmd.filter_disable = true;
	cmd_mask.filter_disable = true;
	cmd.route_disable = true;
	cmd_mask.route_disable = true;
	cmd.hdr_removal_insertion_disable = false;
	cmd_mask.hdr_removal_insertion_disable = true;
	cmd.cs_disable = false;
	cmd_mask.cs_disable = true;
	cmd.flt_retain_hdr = true;
	cmd_mask.flt_retain_hdr = true;
	cmd.rt_retain_hdr = true;
	cmd_mask.rt_retain_hdr = true;
	cmd_mask.rt_pipe_dest_idx = true;
	for (cmd.rt_pipe_dest_idx = 0;
		cmd.rt_pipe_dest_idx < ipa3_ctx->ipa_num_pipes;
		cmd.rt_pipe_dest_idx++) {
		result = ipahal_modify_imm_cmd(IPA_IMM_CMD_IP_PACKET_INIT_EX,
			cmd_pyld->data, &cmd, &cmd_mask);
		if (unlikely(result != 0)) {
			IPAERR("failed to modify IMM cmd\n");
			goto free_dma;
		}
		memcpy(mem->base + cmd.rt_pipe_dest_idx * cmd_pyld->len,
			cmd_pyld->data, cmd_pyld->len);
		ipa3_ctx->pkt_init_ex_imm[cmd.rt_pipe_dest_idx].phys_base =
			mem->phys_base + cmd.rt_pipe_dest_idx * cmd_pyld->len;
		ipa3_ctx->pkt_init_ex_imm[cmd.rt_pipe_dest_idx].base =
			mem->base + cmd.rt_pipe_dest_idx * cmd_pyld->len;
		ipa3_ctx->pkt_init_ex_imm[cmd.rt_pipe_dest_idx].size =
			cmd_pyld->len;
	}

	cmd.hdr_removal_insertion_disable = true;
	cmd.cs_disable = true;
	cmd.flt_retain_hdr = false;
	cmd.rt_retain_hdr = false;
	cmd.flt_close_aggr_irq_mod = true;
	cmd_mask.flt_close_aggr_irq_mod = true;
	cmd.rt_close_aggr_irq_mod = true;
	cmd_mask.rt_close_aggr_irq_mod = true;
	/* Just a placeholder. Will be assigned in the DP, before sending. */
	cmd.rt_pipe_dest_idx = ipa3_ctx->ipa_num_pipes;
	result = ipahal_modify_imm_cmd(IPA_IMM_CMD_IP_PACKET_INIT_EX,
		cmd_pyld->data, &cmd, &cmd_mask);
	if (unlikely(result != 0)) {
		IPAERR("failed to modify IMM cmd\n");
		goto free_dma;
	}
	memcpy(mem->base + ipa3_ctx->ipa_num_pipes * cmd_pyld->len,
		cmd_pyld->data,
		cmd_pyld->len);
	ipa3_ctx->pkt_init_ex_imm[ipa3_ctx->ipa_num_pipes].phys_base =
		mem->phys_base + ipa3_ctx->ipa_num_pipes * cmd_pyld->len;
	ipa3_ctx->pkt_init_ex_imm[ipa3_ctx->ipa_num_pipes].base =
		mem->base + ipa3_ctx->ipa_num_pipes * cmd_pyld->len;
	ipa3_ctx->pkt_init_ex_imm[ipa3_ctx->ipa_num_pipes].size = cmd_pyld->len;

	goto free_imm;

free_dma:
	dma_free_coherent(ipa3_ctx->pdev,
		mem->size,
		mem->base,
		mem->phys_base);
free_imm:
	ipahal_destroy_imm_cmd(cmd_pyld);
	return result;
}

/**
 * ipa_set_pkt_init_ex_hdr_ofst() - Set pkt_init_ex header offset for the ep
 * @lookup: header and ep identifying parameters
 *
 * Returns 0 on success
 */
int ipa_set_pkt_init_ex_hdr_ofst(struct ipa_pkt_init_ex_hdr_ofst_set
	*lookup, bool proc_ctx)
{
	struct ipahal_imm_cmd_pyld *cmd_pyld;
	struct ipahal_imm_cmd_ip_packet_init_ex cmd = {0};
	u32 offset;
	int res = 0;
	int dst_ep_idx;

	if (!lookup)
		return -EINVAL;

	dst_ep_idx = ipa3_get_ep_mapping(lookup->ep);
	IPADBG("dst_ep_idx=%d\n", dst_ep_idx);
	if (-1 == dst_ep_idx) {
		IPAERR("Client %u is not mapped\n", lookup->ep);
		return -EINVAL;
	}
	if (proc_ctx) {
		res = ipa3_get_hdr_proc_ctx_offset(lookup->name, &offset);
	} else {
		res = ipa3_get_hdr_offset(lookup->name ,&offset);
	}
	if (res != 0)
		return res;

	cmd.rt_hdr_offset = offset;
	IPADBG("cmd.rt_hdr_offset=%d\n", cmd.rt_hdr_offset);
	cmd.frag_disable = true;
	cmd.nat_disable = true;
	cmd.filter_disable = true;
	cmd.route_disable = true;
	cmd.hdr_removal_insertion_disable = false;
	cmd.cs_disable = false;
	cmd.flt_retain_hdr = true;
	cmd.rt_retain_hdr = true;
	cmd.rt_pipe_dest_idx = dst_ep_idx;
	cmd.rt_proc_ctx = proc_ctx;
	cmd_pyld = ipahal_construct_imm_cmd(IPA_IMM_CMD_IP_PACKET_INIT_EX,
		&cmd, false);
	if (!cmd_pyld) {
		IPAERR("failed to construct IMM cmd\n");
		return -ENOMEM;
	}
	memcpy(ipa3_ctx->pkt_init_ex_mem.base + dst_ep_idx * cmd_pyld->len,
		cmd_pyld->data, cmd_pyld->len);
	ipahal_destroy_imm_cmd(cmd_pyld);
	return 0;
}
EXPORT_SYMBOL(ipa_set_pkt_init_ex_hdr_ofst);

/*
 * SCM call to check if secure dump is allowed.
 *
 * Returns true in secure dump allowed.
 * Return false when secure dump not allowed.
 */
static bool ipa_is_mem_dump_allowed(void)
{
	int ret;
	u32 dump_state;

	ret = qcom_scm_get_sec_dump_state(&dump_state);

	if (ret) {
		IPAERR("SCM DUMP_STATE call failed\n");
		return false;
	}

	return (dump_state == 1);
}

static int ipa3_lan_poll(struct napi_struct *napi, int budget)
{
	int rcvd_pkts = 0;

	rcvd_pkts = ipa3_lan_rx_poll(ipa3_ctx->clnt_hdl_data_in,
							NAPI_WEIGHT);
	return rcvd_pkts;
}

static inline void ipa3_enable_napi_netdev(void)
{
	if (ipa3_ctx->lan_rx_napi_enable || ipa3_ctx->tx_napi_enable) {
		init_dummy_netdev(&ipa3_ctx->generic_ndev);
		if(ipa3_ctx->lan_rx_napi_enable) {
			netif_napi_add(&ipa3_ctx->generic_ndev, &ipa3_ctx->napi_lan_rx,
					ipa3_lan_poll, NAPI_WEIGHT);
		}
	}
}

static inline void ipa3_disable_napi_netdev(void)
{
	if (ipa3_ctx->lan_rx_napi_enable)
		netif_napi_del(&ipa3_ctx->napi_lan_rx);
}

static u32 get_tx_wrapper_cache_size(u32 cache_size)
{
	if (cache_size <= IPA_TX_WRAPPER_CACHE_MAX_THRESHOLD)
		return cache_size;
	return IPA_TX_WRAPPER_CACHE_MAX_THRESHOLD;
}

static u32 get_ipa_gen_rx_cmn_page_pool_size(u32 rx_cmn_page_pool_size)
{
        if (!rx_cmn_page_pool_size)
                return IPA_GENERIC_RX_CMN_PAGE_POOL_SZ_FACTOR;
        if (rx_cmn_page_pool_size <= IPA_GENERIC_RX_CMN_PAGE_POOL_SZ_FACTOR)
                return rx_cmn_page_pool_size;
        return IPA_GENERIC_RX_CMN_PAGE_POOL_SZ_FACTOR;
}


static u32 get_ipa_gen_rx_cmn_temp_pool_size(u32 rx_cmn_temp_pool_size)
{
        if (!rx_cmn_temp_pool_size)
                return IPA_GENERIC_RX_CMN_TEMP_POOL_SZ_FACTOR;
        if (rx_cmn_temp_pool_size <= IPA_GENERIC_RX_CMN_TEMP_POOL_SZ_FACTOR)
                return rx_cmn_temp_pool_size;
        return IPA_GENERIC_RX_CMN_TEMP_POOL_SZ_FACTOR;
}

#if IS_ENABLED(CONFIG_QCOM_VA_MINIDUMP)
static int qcom_va_md_ipa_notif_handler(struct notifier_block *this,
					unsigned long event, void *ptr)
{
	struct ipa_minidump_data *testptr;
	struct ipa_minidump_data *ptr_next;

	list_for_each_entry_safe(testptr, ptr_next, &ipa3_ctx->minidump_list_head, entry) {
		qcom_va_md_add_region(&testptr->data);
	}

	return NOTIFY_OK;
}

static struct notifier_block qcom_va_md_ipa_notif_blk = {
	.notifier_call = qcom_va_md_ipa_notif_handler,
	.priority = INT_MAX,
};
#endif

/**
 * ipa3_pre_init() - Initialize the IPA Driver.
 * This part contains all initialization which doesn't require IPA HW, such
 * as structure allocations and initializations, register writes, etc.
 *
 * @resource_p:	contain platform specific values from DST file
 * @pdev:	The platform device structure representing the IPA driver
 *
 * Function initialization process:
 * Allocate memory for the driver context data struct
 * Initializing the ipa3_ctx with :
 *    1)parsed values from the dts file
 *    2)parameters passed to the module initialization
 *    3)read HW values(such as core memory size)
 * Map IPA core registers to CPU memory
 * Restart IPA core(HW reset)
 * Initialize the look-aside caches(kmem_cache/slab) for filter,
 *   routing and IPA-tree
 * Create memory pool with 4 objects for DMA operations(each object
 *   is 512Bytes long), this object will be use for tx(A5->IPA)
 * Initialize lists head(routing, hdr, system pipes)
 * Initialize mutexes (for ipa_ctx and NAT memory mutexes)
 * Initialize spinlocks (for list related to A5<->IPA pipes)
 * Initialize 2 single-threaded work-queue named "ipa rx wq" and "ipa tx wq"
 * Initialize Red-Black-Tree(s) for handles of header,routing rule,
 *  routing table ,filtering rule
 * Initialize the filter block by committing IPV4 and IPV6 default rules
 * Create empty routing table in system memory(no committing)
 * Create a char-device for IPA
 * Initialize IPA PM (power manager)
 * Configure GSI registers (in GSI case)
 */
static int ipa3_pre_init(const struct ipa3_plat_drv_res *resource_p,
		struct platform_device *ipa_pdev)
{
	int result = 0;
	int i, j;
	struct ipa3_rt_tbl_set *rset;
	struct ipa_active_client_logging_info log_info;
	struct cdev *cdev;
	enum hdr_tbl_storage hdr_tbl;

	IPADBG("IPA Driver initialization started\n");

	if (!ipa3_ctx) {
		result = -ENOMEM;
		goto fail_mem_ctx;
	}
	/* If SMMU not support fw load state will be updated
	 * in probe function. Avoid overwriting in pre-init function */
	if (ipa3_ctx->fw_load_data.state != IPA_FW_LOAD_STATE_SMMU_DONE)
		ipa3_ctx->fw_load_data.state = IPA_FW_LOAD_STATE_INIT;
	mutex_init(&ipa3_ctx->fw_load_data.lock);

	ipa3_ctx->logbuf = ipc_log_context_create(IPA_IPC_LOG_PAGES, "ipa", MINIDUMP_MASK);
	if (ipa3_ctx->logbuf == NULL)
		IPADBG("failed to create IPC log, continue...\n");

	ipa3_ctx->logbuf_clk = ipc_log_context_create(IPA_IPC_LOG_PAGES, "ipa_clk", MINIDUMP_MASK);
	if (ipa3_ctx->logbuf_clk == NULL)
		IPADBG("failed to create IPC ipa_clk log, continue...\n");

	/* ipa3_ctx->pdev and ipa3_ctx->uc_pdev will be set in the smmu probes*/
	ipa3_ctx->master_pdev = ipa_pdev;
	for (i = 0; i < IPA_SMMU_CB_MAX; i++)
		ipa3_ctx->s1_bypass_arr[i] = true;

	/* initialize the gsi protocol info for uC debug stats */
	for (i = 0; i < IPA_HW_PROTOCOL_MAX; i++) {
		ipa3_ctx->gsi_info[i].protocol = i;
		/* initialize all to be not started */
		for (j = 0; j < IPA_MAX_CH_STATS_SUPPORTED; j++)
			ipa3_ctx->gsi_info[i].ch_id_info[j].ch_id =
				0xFF;
	}

	ipa3_ctx->gfp_no_retry = resource_p->gfp_no_retry;
	ipa3_ctx->ipa_wrapper_base = resource_p->ipa_mem_base;
	ipa3_ctx->ipa_wrapper_size = resource_p->ipa_mem_size;
	ipa3_ctx->ipa_cfg_offset = resource_p->ipa_cfg_offset;
	ipa3_ctx->ipa_hw_type = resource_p->ipa_hw_type;
	ipa3_ctx->ipa_config_is_mhi = resource_p->ipa_mhi_dynamic_config;
	ipa3_ctx->ipa3_hw_mode = resource_p->ipa3_hw_mode;
	ipa3_ctx->platform_type = resource_p->platform_type;
	ipa3_ctx->use_ipa_teth_bridge = resource_p->use_ipa_teth_bridge;
	ipa3_ctx->modem_cfg_emb_pipe_flt = resource_p->modem_cfg_emb_pipe_flt;
	ipa3_ctx->ipa_wdi2 = resource_p->ipa_wdi2;
	ipa3_ctx->ipa_wdi2_over_gsi = resource_p->ipa_wdi2_over_gsi;
	ipa3_ctx->ipa_wdi3_over_gsi = resource_p->ipa_wdi3_over_gsi;
	ipa3_ctx->ipa_fltrt_not_hashable = resource_p->ipa_fltrt_not_hashable;
	ipa3_ctx->use_xbl_boot = resource_p->use_xbl_boot;
	ipa3_ctx->use_64_bit_dma_mask = resource_p->use_64_bit_dma_mask;
	ipa3_ctx->wan_rx_ring_size = resource_p->wan_rx_ring_size;
	ipa3_ctx->lan_rx_ring_size = resource_p->lan_rx_ring_size;
	ipa3_ctx->ipa_wan_skb_page = resource_p->ipa_wan_skb_page;
	ipa3_ctx->uc_ctx.ipa_use_uc_holb_monitor =
		resource_p->ipa_use_uc_holb_monitor;
	ipa3_ctx->uc_ctx.holb_monitor.poll_period =
		resource_p->ipa_holb_monitor_poll_period;
	ipa3_ctx->uc_ctx.holb_monitor.max_cnt_wlan =
		resource_p->ipa_holb_monitor_max_cnt_wlan;
	ipa3_ctx->uc_ctx.holb_monitor.max_cnt_usb =
		resource_p->ipa_holb_monitor_max_cnt_usb;
	ipa3_ctx->uc_ctx.holb_monitor.max_cnt_11ad =
		resource_p->ipa_holb_monitor_max_cnt_11ad;
	ipa3_ctx->ipa_wan_aggr_pkt_cnt = resource_p->ipa_wan_aggr_pkt_cnt;
	ipa3_ctx->stats.page_recycle_stats[0].total_replenished = 0;
	ipa3_ctx->stats.page_recycle_stats[0].tmp_alloc = 0;
	ipa3_ctx->stats.page_recycle_stats[1].total_replenished = 0;
	ipa3_ctx->stats.page_recycle_stats[1].tmp_alloc = 0;
	memset(ipa3_ctx->stats.page_recycle_cnt, 0,
		sizeof(ipa3_ctx->stats.page_recycle_cnt));
	ipa3_ctx->stats.num_sort_tasklet_sched[0] = 0;
	ipa3_ctx->stats.num_sort_tasklet_sched[1] = 0;
	ipa3_ctx->stats.num_sort_tasklet_sched[2] = 0;
	ipa3_ctx->stats.num_of_times_wq_reschd = 0;
	ipa3_ctx->stats.page_recycle_cnt_in_tasklet = 0;
	ipa3_ctx->skip_uc_pipe_reset = resource_p->skip_uc_pipe_reset;
	ipa3_ctx->tethered_flow_control = resource_p->tethered_flow_control;
	ipa3_ctx->ee = resource_p->ee;
	ipa3_ctx->apply_rg10_wa = resource_p->apply_rg10_wa;
	ipa3_ctx->gsi_ch20_wa = resource_p->gsi_ch20_wa;
	ipa3_ctx->wdi_over_pcie = resource_p->wdi_over_pcie;
	ipa3_ctx->ipa3_active_clients_logging.log_rdy = false;
	ipa3_ctx->is_device_crashed = false;
	ipa3_ctx->mhi_evid_limits[0] = resource_p->mhi_evid_limits[0];
	ipa3_ctx->mhi_evid_limits[1] = resource_p->mhi_evid_limits[1];
	ipa3_ctx->entire_ipa_block_size = resource_p->entire_ipa_block_size;
	ipa3_ctx->do_register_collection_on_crash =
	    resource_p->do_register_collection_on_crash;
	ipa3_ctx->do_testbus_collection_on_crash =
	    resource_p->do_testbus_collection_on_crash;
	ipa3_ctx->do_non_tn_collection_on_crash =
	    resource_p->do_non_tn_collection_on_crash;
	ipa3_ctx->secure_debug_check_action =
		resource_p->secure_debug_check_action;
	ipa3_ctx->do_ram_collection_on_crash =
		resource_p->do_ram_collection_on_crash;
	ipa3_ctx->lan_rx_napi_enable = resource_p->lan_rx_napi_enable;
	ipa3_ctx->tx_napi_enable = resource_p->tx_napi_enable;
	ipa3_ctx->tx_poll = resource_p->tx_poll;
	ipa3_ctx->ipa_gpi_event_rp_ddr = resource_p->ipa_gpi_event_rp_ddr;
	ipa3_ctx->rmnet_ctl_enable = resource_p->rmnet_ctl_enable;
	ipa3_ctx->rmnet_ll_enable = resource_p->rmnet_ll_enable;
	ipa3_ctx->tx_wrapper_cache_max_size = get_tx_wrapper_cache_size(
			resource_p->tx_wrapper_cache_max_size);
	ipa3_ctx->ipa_gen_rx_cmn_page_pool_sz_factor = get_ipa_gen_rx_cmn_page_pool_size(
                        resource_p->ipa_gen_rx_cmn_page_pool_sz_factor);
        ipa3_ctx->ipa_gen_rx_cmn_temp_pool_sz_factor = get_ipa_gen_rx_cmn_temp_pool_size(
                        resource_p->ipa_gen_rx_cmn_temp_pool_sz_factor);
	ipa3_ctx->ipa_config_is_auto = resource_p->ipa_config_is_auto;
	ipa3_ctx->ipa_mhi_proxy = resource_p->ipa_mhi_proxy;
	ipa3_ctx->max_num_smmu_cb = resource_p->max_num_smmu_cb;
	ipa3_ctx->hw_type_index = ipa3_get_hw_type_index();
	ipa3_ctx->ipa_wdi3_2g_holb_timeout =
		resource_p->ipa_wdi3_2g_holb_timeout;
	ipa3_ctx->ipa_wdi3_5g_holb_timeout =
		resource_p->ipa_wdi3_5g_holb_timeout;
	ipa3_ctx->is_wdi3_tx1_needed = false;
	ipa3_ctx->ulso_supported = resource_p->ulso_supported;
	ipa3_ctx->ulso_ip_id_min = resource_p->ulso_ip_id_min;
	ipa3_ctx->ulso_ip_id_max = resource_p->ulso_ip_id_max;
	ipa3_ctx->use_pm_wrapper = resource_p->use_pm_wrapper;
	ipa3_ctx->use_tput_est_ep = resource_p->use_tput_est_ep;
	ipa3_ctx->mpm_ring_size_ul_cache = DEFAULT_MPM_RING_SIZE_UL;
	ipa3_ctx->mpm_ring_size_ul = DEFAULT_MPM_RING_SIZE_UL;
	ipa3_ctx->mpm_ring_size_dl_cache = DEFAULT_MPM_RING_SIZE_DL;
	ipa3_ctx->mpm_ring_size_dl = DEFAULT_MPM_RING_SIZE_DL;
	ipa3_ctx->mpm_teth_aggr_size = DEFAULT_MPM_TETH_AGGR_SIZE;
	ipa3_ctx->mpm_uc_thresh = DEFAULT_MPM_UC_THRESH_SIZE;
	ipa3_ctx->uc_act_tbl_valid = false;
	ipa3_ctx->uc_act_tbl_total = 0;
	ipa3_ctx->uc_act_tbl_next_index = 0;
	ipa3_ctx->is_dual_pine_config = resource_p->is_dual_pine_config;

	if (resource_p->gsi_fw_file_name) {
		ipa3_ctx->gsi_fw_file_name =
			kzalloc(((strlen(resource_p->gsi_fw_file_name)+1) *
				sizeof(const char)), GFP_KERNEL);
		if (ipa3_ctx->gsi_fw_file_name == NULL) {
			IPAERR_RL("Failed to alloc GSI FW file name\n");
			result = -ENOMEM;
			goto fail_gsi_file_alloc;
		}
		memcpy(ipa3_ctx->gsi_fw_file_name,
				(void const *)resource_p->gsi_fw_file_name,
				strlen(resource_p->gsi_fw_file_name));
	}

	if (resource_p->uc_fw_file_name) {
		ipa3_ctx->uc_fw_file_name =
			kzalloc(((strlen(resource_p->uc_fw_file_name)+1) *
				sizeof(const char)), GFP_KERNEL);
		if (ipa3_ctx->uc_fw_file_name == NULL) {
			IPAERR_RL("Failed to alloc uC FW file name\n");
			result = -ENOMEM;
			goto fail_uc_file_alloc;
		}
		memcpy(ipa3_ctx->uc_fw_file_name,
			(void const *)resource_p->uc_fw_file_name,
			strlen(resource_p->uc_fw_file_name));
	}

	if (ipa3_ctx->secure_debug_check_action == USE_SCM) {
		if (ipa_is_mem_dump_allowed())
			ipa3_ctx->sd_state = SD_ENABLED;
		else
			ipa3_ctx->sd_state = SD_DISABLED;
	} else {
		if (ipa3_ctx->secure_debug_check_action == OVERRIDE_SCM_TRUE)
			ipa3_ctx->sd_state = SD_ENABLED;
		else
			/* secure_debug_check_action == OVERRIDE_SCM_FALSE */
			ipa3_ctx->sd_state = SD_DISABLED;
	}

	if (ipa3_ctx->sd_state == SD_ENABLED) {
		/* secure debug is enabled. */
		IPADBG("secure debug enabled\n");
	} else {
		/* secure debug is disabled. */
		IPADBG("secure debug disabled\n");
		ipa3_ctx->do_testbus_collection_on_crash = false;
	}
	ipa3_ctx->ipa_endp_delay_wa = resource_p->ipa_endp_delay_wa;
	ipa3_ctx->ipa_endp_delay_wa_v2 = resource_p->ipa_endp_delay_wa_v2;
	ipa3_ctx->ulso_wa = resource_p->ulso_wa;

	WARN(!IPA_IS_REGULAR_CLK_MODE(ipa3_ctx->ipa3_hw_mode),
		"Non NORMAL IPA HW mode, is this emulation platform ?");

	if (resource_p->ipa_tz_unlock_reg) {
		ipa3_ctx->ipa_tz_unlock_reg_num =
			resource_p->ipa_tz_unlock_reg_num;
		ipa3_ctx->ipa_tz_unlock_reg = kcalloc(
			ipa3_ctx->ipa_tz_unlock_reg_num,
			sizeof(*ipa3_ctx->ipa_tz_unlock_reg),
			GFP_KERNEL);
		if (ipa3_ctx->ipa_tz_unlock_reg == NULL) {
			result = -ENOMEM;
			goto fail_tz_unlock_reg;
		}
		for (i = 0; i < ipa3_ctx->ipa_tz_unlock_reg_num; i++) {
			ipa3_ctx->ipa_tz_unlock_reg[i].reg_addr =
				resource_p->ipa_tz_unlock_reg[i].reg_addr;
			ipa3_ctx->ipa_tz_unlock_reg[i].size =
				resource_p->ipa_tz_unlock_reg[i].size;
		}

		/* unlock registers for uc */
		result = ipa3_tz_unlock_reg(ipa3_ctx->ipa_tz_unlock_reg,
					    ipa3_ctx->ipa_tz_unlock_reg_num);
		if (result)
			IPAERR("Failed to unlock memory region using TZ\n");
	}

	/* default aggregation parameters */
	ipa3_ctx->aggregation_type = IPA_MBIM_16;
	ipa3_ctx->aggregation_byte_limit = 1;
	ipa3_ctx->aggregation_time_limit = 0;

	/* configure interconnect parameters */
	ipa3_ctx->icc_num_cases = resource_p->icc_num_cases;
	ipa3_ctx->icc_num_paths = resource_p->icc_num_paths;
	for (i = 0; i < ipa3_ctx->icc_num_cases; i++) {
		for (j = 0; j < ipa3_ctx->icc_num_paths; j++) {
			ipa3_ctx->icc_clk[i][j][IPA_ICC_AB] =
			    resource_p->icc_clk_val[i][j*IPA_ICC_TYPE_MAX];
			ipa3_ctx->icc_clk[i][j][IPA_ICC_IB] =
			    resource_p->icc_clk_val[i][j*IPA_ICC_TYPE_MAX+1];
		}
	}

	ipa3_ctx->ctrl = kzalloc(sizeof(*ipa3_ctx->ctrl), GFP_KERNEL);
	if (!ipa3_ctx->ctrl) {
		result = -ENOMEM;
		goto fail_mem_ctrl;
	}
	result = ipa3_controller_static_bind(ipa3_ctx->ctrl,
			ipa3_ctx->ipa_hw_type, ipa3_ctx->ipa_cfg_offset);
	if (result) {
		IPAERR("fail to static bind IPA ctrl\n");
		result = -EFAULT;
		goto fail_bind;
	}

	result = ipa3_init_mem_partition(ipa3_ctx->ipa_hw_type);
	if (result) {
		IPAERR(":ipa3_init_mem_partition failed\n");
		result = -ENODEV;
		goto fail_init_mem_partition;
	}

	if (ipa3_ctx->ipa3_hw_mode != IPA_HW_MODE_VIRTUAL &&
	    ipa3_ctx->ipa3_hw_mode != IPA_HW_MODE_EMULATION) {
		/* get BUS handle */
		for (i = 0; i < ipa3_ctx->icc_num_paths; i++) {
			ipa3_ctx->ctrl->icc_path[i] = of_icc_get(
				&ipa3_ctx->master_pdev->dev,
				resource_p->icc_path_name[i]);
			if (IS_ERR(ipa3_ctx->ctrl->icc_path[i])) {
				IPAERR("fail to register with bus mgr!\n");
				result = PTR_ERR(ipa3_ctx->ctrl->icc_path[i]);
				if (result != -EPROBE_DEFER) {
					IPAERR("Failed to get path %s\n",
						ipa3_ctx->master_pdev->name);
				}
				goto fail_bus_reg;
			}
		}
	}

	/* get IPA clocks */
	result = ipa3_get_clks(&ipa3_ctx->master_pdev->dev);
	if (result)
		goto fail_bus_reg;

	/* init active_clients_log after getting ipa-clk */
	result = ipa3_active_clients_log_init();
	if (result)
		goto fail_init_active_client;

	/* Enable ipa3_ctx->enable_clock_scaling */
	ipa3_ctx->enable_clock_scaling = 1;
	/* vote for svs2 on bootup */
	ipa3_ctx->curr_ipa_clk_rate = ipa3_ctx->ctrl->ipa_clk_rate_svs2;

	/* Enable ipa3_ctx->enable_napi_chain */
	ipa3_ctx->enable_napi_chain = 1;

	/* Initialize Page poll threshold. */
	ipa3_ctx->page_poll_threshold = IPA_PAGE_POLL_DEFAULT_THRESHOLD;

	/*Initialize number napi without prealloc buff*/
	ipa3_ctx->ipa_max_napi_sort_page_thrshld = IPA_MAX_NAPI_SORT_PAGE_THRSHLD;
	ipa3_ctx->page_wq_reschd_time = IPA_MAX_PAGE_WQ_RESCHED_TIME;

	/* Use common page pool for Def/Coal pipe. */
	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v5_1)
		ipa3_ctx->wan_common_page_pool = true;

	/* assume clock is on in virtual/emulation mode */
	if (ipa3_ctx->ipa3_hw_mode == IPA_HW_MODE_VIRTUAL ||
	    ipa3_ctx->ipa3_hw_mode == IPA_HW_MODE_EMULATION)
		atomic_set(&ipa3_ctx->ipa_clk_vote, 1);

	/* enable IPA clocks explicitly to allow the initialization */
	ipa3_enable_clks();

	/* setup IPA register access */
	IPADBG("Mapping 0x%x\n", resource_p->ipa_mem_base +
		ipa3_ctx->ctrl->ipa_reg_base_ofst);
	ipa3_ctx->mmio = ioremap(resource_p->ipa_mem_base +
			ipa3_ctx->ctrl->ipa_reg_base_ofst,
			resource_p->ipa_mem_size);
	if (!ipa3_ctx->mmio) {
		IPAERR(":ipa-base ioremap err\n");
		result = -EFAULT;
		goto fail_remap;
	}

	IPADBG(
	    "base(0x%x)+offset(0x%x)=(0x%x) mapped to (%0x%x) with len (0x%x)\n",
	    resource_p->ipa_mem_base,
	    ipa3_ctx->ctrl->ipa_reg_base_ofst,
	    resource_p->ipa_mem_base + ipa3_ctx->ctrl->ipa_reg_base_ofst,
	    ipa3_ctx->mmio,
	    resource_p->ipa_mem_size);

	/* IPA version 3.0 IPAHAL used to load the firmwares and
	 * there is no SMMU so IPAHAL is initialized here.*/
	if (ipa3_ctx->ipa_hw_type == IPA_HW_v3_0) {
		if (ipahal_init(ipa3_ctx->ipa_hw_type, ipa3_ctx->mmio,
				ipa3_ctx->ipa_cfg_offset, &ipa3_ctx->master_pdev->dev)) {
			IPAERR("fail to init ipahal\n");
			result = -EFAULT;
			goto fail_remap;
		}
	}

	INIT_LIST_HEAD(&ipa3_ctx->minidump_list_head);

	/*
	 * Setup access for register collection/dump on crash
	 */
	if (ipa_reg_save_init(IPA_MEM_INIT_VAL) != 0) {
		result = -EFAULT;
		goto fail_gsi_map;
	}

	mutex_init(&ipa3_ctx->ipa3_active_clients.mutex);

	IPA_ACTIVE_CLIENTS_PREP_SPECIAL(log_info, "PROXY_CLK_VOTE");
	ipa3_active_clients_log_inc(&log_info, false);
	ipa3_ctx->q6_proxy_clk_vote_valid = true;
	ipa3_ctx->q6_proxy_clk_vote_cnt = 1;

	/*Updating the proxy vote cnt 1 */
	atomic_set(&ipa3_ctx->ipa3_active_clients.cnt, 1);

	/* Create workqueues for power management */
	ipa3_ctx->power_mgmt_wq = alloc_workqueue("ipa_power_mgmt",
			WQ_MEM_RECLAIM | WQ_UNBOUND | WQ_SYSFS | WQ_HIGHPRI, 1);
	if (!ipa3_ctx->power_mgmt_wq) {
		IPAERR("failed to create power mgmt wq\n");
		result = -ENOMEM;
		goto fail_gsi_map;
	}
	mutex_init(&ipa3_ctx->recycle_stats_collection_lock);
	memset(&ipa3_ctx->recycle_stats, 0, sizeof(struct ipa_lnx_pipe_page_recycling_stats));
	memset(&ipa3_ctx->prev_coal_recycle_stats, 0, sizeof(struct ipa3_page_recycle_stats));
	memset(&ipa3_ctx->prev_default_recycle_stats, 0, sizeof(struct ipa3_page_recycle_stats));
	memset(&ipa3_ctx->prev_low_lat_data_recycle_stats, 0,
		sizeof(struct ipa3_page_recycle_stats));

	ipa3_ctx->transport_power_mgmt_wq =
		create_singlethread_workqueue("transport_power_mgmt");
	if (!ipa3_ctx->transport_power_mgmt_wq) {
		IPAERR("failed to create transport power mgmt wq\n");
		result = -ENOMEM;
		goto fail_create_transport_wq;
	}

	/* Create workqueue for recycle stats collection */
	ipa3_ctx->collect_recycle_stats_wq =
			create_singlethread_workqueue("page_recycle_stats_collection");
	if (!ipa3_ctx->collect_recycle_stats_wq) {
		IPAERR("failed to create page recycling stats collection wq\n");
		result = -ENOMEM;
		goto fail_create_recycle_stats_wq;
	}
	memset(&ipa3_ctx->recycle_stats, 0,
		   sizeof(ipa3_ctx->recycle_stats));

	mutex_init(&ipa3_ctx->transport_pm.transport_pm_mutex);

	/* init the lookaside cache */
	ipa3_ctx->flt_rule_cache = kmem_cache_create("IPA_FLT",
			sizeof(struct ipa3_flt_entry), 0, 0, NULL);
	if (!ipa3_ctx->flt_rule_cache) {
		IPAERR(":ipa flt cache create failed\n");
		result = -ENOMEM;
		goto fail_flt_rule_cache;
	}
	ipa3_ctx->rt_rule_cache = kmem_cache_create("IPA_RT",
			sizeof(struct ipa3_rt_entry), 0, 0, NULL);
	if (!ipa3_ctx->rt_rule_cache) {
		IPAERR(":ipa rt cache create failed\n");
		result = -ENOMEM;
		goto fail_rt_rule_cache;
	}
	ipa3_ctx->hdr_cache = kmem_cache_create("IPA_HDR",
			sizeof(struct ipa3_hdr_entry), 0, 0, NULL);
	if (!ipa3_ctx->hdr_cache) {
		IPAERR(":ipa hdr cache create failed\n");
		result = -ENOMEM;
		goto fail_hdr_cache;
	}
	ipa3_ctx->hdr_offset_cache =
	   kmem_cache_create("IPA_HDR_OFFSET",
			   sizeof(struct ipa_hdr_offset_entry), 0, 0, NULL);
	if (!ipa3_ctx->hdr_offset_cache) {
		IPAERR(":ipa hdr off cache create failed\n");
		result = -ENOMEM;
		goto fail_hdr_offset_cache;
	}
	ipa3_ctx->fnr_stats_cache = kmem_cache_create("IPA_FNR_STATS",
		sizeof(struct ipa_ioc_flt_rt_counter_alloc), 0, 0, NULL);
	if (!ipa3_ctx->fnr_stats_cache) {
		IPAERR(":ipa fnr stats cache create failed\n");
		result = -ENOMEM;
		goto fail_fnr_stats_cache;
	}
	ipa3_ctx->hdr_proc_ctx_cache = kmem_cache_create("IPA_HDR_PROC_CTX",
		sizeof(struct ipa3_hdr_proc_ctx_entry), 0, 0, NULL);
	if (!ipa3_ctx->hdr_proc_ctx_cache) {
		IPAERR(":ipa hdr proc ctx cache create failed\n");
		result = -ENOMEM;
		goto fail_hdr_proc_ctx_cache;
	}
	ipa3_ctx->hdr_proc_ctx_offset_cache =
		kmem_cache_create("IPA_HDR_PROC_CTX_OFFSET",
		sizeof(struct ipa3_hdr_proc_ctx_offset_entry), 0, 0, NULL);
	if (!ipa3_ctx->hdr_proc_ctx_offset_cache) {
		IPAERR(":ipa hdr proc ctx off cache create failed\n");
		result = -ENOMEM;
		goto fail_hdr_proc_ctx_offset_cache;
	}
	ipa3_ctx->rt_tbl_cache = kmem_cache_create("IPA_RT_TBL",
			sizeof(struct ipa3_rt_tbl), 0, 0, NULL);
	if (!ipa3_ctx->rt_tbl_cache) {
		IPAERR(":ipa rt tbl cache create failed\n");
		result = -ENOMEM;
		goto fail_rt_tbl_cache;
	}
	ipa3_ctx->tx_pkt_wrapper_cache =
	   kmem_cache_create("IPA_TX_PKT_WRAPPER",
			   sizeof(struct ipa3_tx_pkt_wrapper), 0, 0, NULL);
	if (!ipa3_ctx->tx_pkt_wrapper_cache) {
		IPAERR(":ipa tx pkt wrapper cache create failed\n");
		result = -ENOMEM;
		goto fail_tx_pkt_wrapper_cache;
	}
	ipa3_ctx->rx_pkt_wrapper_cache =
	   kmem_cache_create("IPA_RX_PKT_WRAPPER",
			   sizeof(struct ipa3_rx_pkt_wrapper), 0, 0, NULL);
	if (!ipa3_ctx->rx_pkt_wrapper_cache) {
		IPAERR(":ipa rx pkt wrapper cache create failed\n");
		result = -ENOMEM;
		goto fail_rx_pkt_wrapper_cache;
	}

	/* Init the various list heads for both SRAM/DDR */
	for (hdr_tbl = HDR_TBL_LCL; hdr_tbl < HDR_TBLS_TOTAL; hdr_tbl++) {
		INIT_LIST_HEAD(&ipa3_ctx->hdr_tbl[hdr_tbl].head_hdr_entry_list);
		for (i = 0; i < IPA_HDR_BIN_MAX; i++) {
			INIT_LIST_HEAD(&ipa3_ctx->hdr_tbl[hdr_tbl].head_offset_list[i]);
			INIT_LIST_HEAD(&ipa3_ctx->hdr_tbl[hdr_tbl].head_free_offset_list[i]);
		}
	}
	INIT_LIST_HEAD(&ipa3_ctx->hdr_proc_ctx_tbl.head_proc_ctx_entry_list);
	for (i = 0; i < IPA_HDR_PROC_CTX_BIN_MAX; i++) {
		INIT_LIST_HEAD(
			&ipa3_ctx->hdr_proc_ctx_tbl.head_offset_list[i]);
		INIT_LIST_HEAD(
			&ipa3_ctx->hdr_proc_ctx_tbl.head_free_offset_list[i]);
	}
	INIT_LIST_HEAD(&ipa3_ctx->rt_tbl_set[IPA_IP_v4].head_rt_tbl_list);
	idr_init(&ipa3_ctx->rt_tbl_set[IPA_IP_v4].rule_ids);
	INIT_LIST_HEAD(&ipa3_ctx->rt_tbl_set[IPA_IP_v6].head_rt_tbl_list);
	idr_init(&ipa3_ctx->rt_tbl_set[IPA_IP_v6].rule_ids);

	rset = &ipa3_ctx->reap_rt_tbl_set[IPA_IP_v4];
	INIT_LIST_HEAD(&rset->head_rt_tbl_list);
	idr_init(&rset->rule_ids);
	rset = &ipa3_ctx->reap_rt_tbl_set[IPA_IP_v6];
	INIT_LIST_HEAD(&rset->head_rt_tbl_list);
	idr_init(&rset->rule_ids);
	idr_init(&ipa3_ctx->flt_rt_counters.hdl);
	spin_lock_init(&ipa3_ctx->flt_rt_counters.hdl_lock);
	memset(&ipa3_ctx->flt_rt_counters.used_hw, 0,
		   sizeof(ipa3_ctx->flt_rt_counters.used_hw));
	memset(&ipa3_ctx->flt_rt_counters.used_sw, 0,
		   sizeof(ipa3_ctx->flt_rt_counters.used_sw));

	INIT_LIST_HEAD(&ipa3_ctx->intf_list);
	INIT_LIST_HEAD(&ipa3_ctx->msg_list);
	INIT_LIST_HEAD(&ipa3_ctx->pull_msg_list);
	init_waitqueue_head(&ipa3_ctx->msg_waitq);
	mutex_init(&ipa3_ctx->msg_lock);

	/* store wlan client-connect-msg-list */
	INIT_LIST_HEAD(&ipa3_ctx->msg_wlan_client_list);
	mutex_init(&ipa3_ctx->msg_wlan_client_lock);

	/* store  ecm-connect-msg-list */
	INIT_LIST_HEAD(&ipa3_ctx->msg_lan_list);
	mutex_init(&ipa3_ctx->msg_lan_lock);

	mutex_init(&ipa3_ctx->q6_proxy_clk_vote_mutex);
	mutex_init(&ipa3_ctx->ipa_cne_evt_lock);
	mutex_init(&ipa3_ctx->act_tbl_lock);
	mutex_init(&ipa3_ctx->mhi_lock);

	idr_init(&ipa3_ctx->ipa_idr);
	spin_lock_init(&ipa3_ctx->idr_lock);

	/* wlan related member */
	memset(&ipa3_ctx->wc_memb, 0, sizeof(ipa3_ctx->wc_memb));
	spin_lock_init(&ipa3_ctx->wc_memb.wlan_spinlock);
	spin_lock_init(&ipa3_ctx->wc_memb.ipa_tx_mul_spinlock);
	INIT_LIST_HEAD(&ipa3_ctx->wc_memb.wlan_comm_desc_list);

	ipa3_ctx->cdev.class = class_create(THIS_MODULE, DRV_NAME);

	result = alloc_chrdev_region(&ipa3_ctx->cdev.dev_num, 0, 1, DRV_NAME);
	if (result) {
		IPAERR("alloc_chrdev_region err\n");
		result = -ENODEV;
		goto fail_alloc_chrdev_region;
	}

	ipa3_ctx->cdev.dev = device_create(ipa3_ctx->cdev.class, NULL,
		 ipa3_ctx->cdev.dev_num, ipa3_ctx, DRV_NAME);
	if (IS_ERR(ipa3_ctx->cdev.dev)) {
		IPAERR(":device_create err.\n");
		result = -ENODEV;
		goto fail_device_create;
	}

	/* Register a wakeup source. */
	ipa3_ctx->w_lock =
		wakeup_source_register(&ipa_pdev->dev, "IPA_WS");
	if (!ipa3_ctx->w_lock) {
		IPAERR("IPA wakeup source register failed\n");
		result = -ENOMEM;
		goto fail_w_source_register;
	}
	spin_lock_init(&ipa3_ctx->wakelock_ref_cnt.spinlock);

	/* Initialize Power Management framework */
	result = ipa_pm_init(&ipa3_res.pm_init);
	if (result) {
		IPAERR("IPA PM initialization failed (%d)\n", -result);
		result = -ENODEV;
		goto fail_ipa_pm_init;
	}
	IPADBG("IPA power manager initialized\n");

	INIT_LIST_HEAD(&ipa3_ctx->ipa_ready_cb_list);

	init_completion(&ipa3_ctx->init_completion_obj);
	init_completion(&ipa3_ctx->uc_loaded_completion_obj);

	result = ipa3_dma_setup();
	if (result) {
		IPAERR("Failed to setup IPA DMA\n");
		result = -ENODEV;
		goto fail_ipa_dma_setup;
	}

	/*
	 * We can't register the GSI driver yet, as it expects
	 * the GSI FW to be up and running before the registration.
	 *
	 * For IPA3.0 and the emulation system, the GSI configuration
	 * is done by the GSI driver.
	 *
	 * For IPA3.1 (and on), the GSI configuration is done by TZ.
	 */
	if (ipa3_ctx->ipa_hw_type == IPA_HW_v3_0 ||
	    ipa3_ctx->ipa3_hw_mode == IPA_HW_MODE_EMULATION) {
		result = ipa3_gsi_pre_fw_load_init();
		if (result) {
			IPAERR("gsi pre FW loading config failed\n");
			result = -ENODEV;
			goto fail_gsi_pre_fw_load_init;
		}
	}

	cdev = &ipa3_ctx->cdev.cdev;
	cdev_init(cdev, &ipa3_drv_fops);
	cdev->owner = THIS_MODULE;
	cdev->ops = &ipa3_drv_fops;  /* from LDD3 */

	result = cdev_add(cdev, ipa3_ctx->cdev.dev_num, 1);
	if (result) {
		IPAERR(":cdev_add err=%d\n", -result);
		result = -ENODEV;
		goto fail_cdev_add;
	}
	IPADBG("ipa cdev added successful. major:%d minor:%d\n",
			MAJOR(ipa3_ctx->cdev.dev_num),
			MINOR(ipa3_ctx->cdev.dev_num));

	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_1) {
		result = ipa_odl_init();
		if (result) {
			IPADBG("Error: ODL init fialed\n");
			result = -ENODEV;
			goto fail_odl_init;
		}
	}

	/*
	 * for IPA 4.0 offline charge is not needed and we need to prevent
	 * power collapse until IPA uC is loaded.
	 */

	/* proxy vote for modem is added in ipa3_post_init() phase */
	if (ipa3_ctx->ipa_hw_type != IPA_HW_v4_0)
		ipa3_proxy_clk_unvote();

	/* Create the dummy netdev for LAN RX NAPI*/
	ipa3_enable_napi_netdev();

	result = ipa3_wwan_init();
	if (result) {
		IPAERR(":ipa3_wwan_init err=%d\n", -result);
		result = -ENODEV;
		goto fail_wwan_init;
	}

	if (ipa3_ctx->rmnet_ctl_enable) {
		result = ipa3_rmnet_ctl_init();
		if (result) {
			IPAERR(":ipa3_rmnet_ctl_init err=%d\n", -result);
			result = -ENODEV;
			goto fail_rmnet_ctl_init;
		}
	}

	if (ipa3_ctx->rmnet_ll_enable) {
		result = ipa3_rmnet_ll_init();
		if (result) {
			IPAERR(":ipa3_rmnet_ll_init err=%d\n", -result);
			result = -ENODEV;
			goto fail_rmnet_ll_init;
		}
	}
	ipa3_ctx->ipa_rmnet_notifier_list_internal = &ipa_rmnet_notifier_list;
	spin_lock_init(&ipa3_ctx->notifier_lock);
	ipa3_ctx->buff_above_thresh_for_def_pipe_notified = false;
	ipa3_ctx->buff_above_thresh_for_coal_pipe_notified = false;
	ipa3_ctx->buff_below_thresh_for_def_pipe_notified = false;
	ipa3_ctx->buff_below_thresh_for_coal_pipe_notified = false;

	mutex_init(&ipa3_ctx->app_clock_vote.mutex);
	mutex_init(&ipa3_ctx->ssr_lock);
	ipa3_ctx->is_modem_up = false;
	ipa3_ctx->mhi_ctrl_state = IPA_MHI_CTRL_NOT_SETUP;
	ipa3_ctx->is_mhi_coal_set = false;

#if IS_ENABLED(CONFIG_QCOM_VA_MINIDUMP)
	result = qcom_va_md_register("ipa_mini", &qcom_va_md_ipa_notif_blk);

	if(result)
		IPAERR("ipa mini qcom_va_md_register failed = %d\n", result);
	else
		IPADBG("ipa mini qcom_va_md_register success\n");
#endif
	return 0;

fail_rmnet_ll_init:
fail_rmnet_ctl_init:
	ipa3_wwan_cleanup();
fail_wwan_init:
	ipa3_disable_napi_netdev();
	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_1)
		ipa_odl_cleanup();
fail_odl_init:
	cdev_del(cdev);
fail_cdev_add:
fail_gsi_pre_fw_load_init:
	ipa3_dma_shutdown();
fail_ipa_dma_setup:
	ipa_pm_destroy();
fail_w_source_register:
	device_destroy(ipa3_ctx->cdev.class, ipa3_ctx->cdev.dev_num);
fail_ipa_pm_init:
	wakeup_source_unregister(ipa3_ctx->w_lock);
	ipa3_ctx->w_lock = NULL;
fail_device_create:
	unregister_chrdev_region(ipa3_ctx->cdev.dev_num, 1);
fail_alloc_chrdev_region:
	idr_destroy(&ipa3_ctx->ipa_idr);
	rset = &ipa3_ctx->reap_rt_tbl_set[IPA_IP_v6];
	idr_destroy(&rset->rule_ids);
	rset = &ipa3_ctx->reap_rt_tbl_set[IPA_IP_v4];
	idr_destroy(&rset->rule_ids);
	idr_destroy(&ipa3_ctx->rt_tbl_set[IPA_IP_v6].rule_ids);
	idr_destroy(&ipa3_ctx->rt_tbl_set[IPA_IP_v4].rule_ids);
	kmem_cache_destroy(ipa3_ctx->rx_pkt_wrapper_cache);
fail_rx_pkt_wrapper_cache:
	kmem_cache_destroy(ipa3_ctx->tx_pkt_wrapper_cache);
fail_tx_pkt_wrapper_cache:
	kmem_cache_destroy(ipa3_ctx->rt_tbl_cache);
fail_rt_tbl_cache:
	kmem_cache_destroy(ipa3_ctx->hdr_proc_ctx_offset_cache);
fail_hdr_proc_ctx_offset_cache:
	kmem_cache_destroy(ipa3_ctx->hdr_proc_ctx_cache);
fail_hdr_proc_ctx_cache:
	kmem_cache_destroy(ipa3_ctx->fnr_stats_cache);
fail_fnr_stats_cache:
	kmem_cache_destroy(ipa3_ctx->hdr_offset_cache);
fail_hdr_offset_cache:
	kmem_cache_destroy(ipa3_ctx->hdr_cache);
fail_hdr_cache:
	kmem_cache_destroy(ipa3_ctx->rt_rule_cache);
fail_rt_rule_cache:
	kmem_cache_destroy(ipa3_ctx->flt_rule_cache);
fail_flt_rule_cache:
	destroy_workqueue(ipa3_ctx->collect_recycle_stats_wq);
fail_create_recycle_stats_wq:
	destroy_workqueue(ipa3_ctx->transport_power_mgmt_wq);
fail_create_transport_wq:
	destroy_workqueue(ipa3_ctx->power_mgmt_wq);
fail_gsi_map:
	if (ipa3_ctx->reg_collection_base)
		iounmap(ipa3_ctx->reg_collection_base);
	iounmap(ipa3_ctx->mmio);
fail_remap:
	ipa3_disable_clks();
	ipa3_active_clients_log_destroy();
	gsi_unmap_base();
fail_init_active_client:
	if (ipa3_clk)
		clk_put(ipa3_clk);
	ipa3_clk = NULL;
fail_bus_reg:
	for (i = 0; i < ipa3_ctx->icc_num_paths; i++)
		if (IS_ERR_OR_NULL(ipa3_ctx->ctrl->icc_path[i])) {
			ipa3_ctx->ctrl->icc_path[i] = NULL;
			break;
		} else {
			icc_put(ipa3_ctx->ctrl->icc_path[i]);
			ipa3_ctx->ctrl->icc_path[i] = NULL;
		}
fail_init_mem_partition:
fail_bind:
	kfree(ipa3_ctx->ctrl);
	ipa3_ctx->ctrl = NULL;
fail_mem_ctrl:
	kfree(ipa3_ctx->ipa_tz_unlock_reg);
	ipa3_ctx->ipa_tz_unlock_reg = NULL;
fail_tz_unlock_reg:
	if (ipa3_ctx->logbuf) {
		ipc_log_context_destroy(ipa3_ctx->logbuf);
		ipa3_ctx->logbuf = NULL;
	}
fail_uc_file_alloc:
	kfree(ipa3_ctx->gsi_fw_file_name);
	ipa3_ctx->gsi_fw_file_name = NULL;
fail_gsi_file_alloc:
fail_mem_ctx:
	return result;
}

static int get_ipa_dts_pm_info(struct platform_device *pdev,
	struct ipa3_plat_drv_res *ipa_drv_res)
{
	int result;
	int i, j;

	/* this interconnects entry must be presented */
	if (!of_find_property(pdev->dev.of_node,
			"interconnects", NULL)) {
		IPAERR("No interconnect info\n");
		return -EFAULT;
	}

	result = of_property_read_u32(pdev->dev.of_node,
		"qcom,interconnect,num-cases",
		&ipa_drv_res->icc_num_cases);
	/* No vote is ignored */
	ipa_drv_res->pm_init.threshold_size =
		ipa_drv_res->icc_num_cases - 2;
	if (result || ipa_drv_res->pm_init.threshold_size >
		IPA_PM_THRESHOLD_MAX) {
		IPAERR("invalid qcom,interconnect,num-cases %d\n",
			ipa_drv_res->pm_init.threshold_size);
		return -EFAULT;
	}

	result = of_property_read_u32(pdev->dev.of_node,
		"qcom,interconnect,num-paths",
		&ipa_drv_res->icc_num_paths);
	if (result || ipa_drv_res->icc_num_paths >
		IPA_ICC_PATH_MAX) {
		IPAERR("invalid qcom,interconnect,num-paths %d\n",
			ipa_drv_res->icc_num_paths);
		return -EFAULT;
	}

	for (i = 0; i < ipa_drv_res->icc_num_paths; i++) {
		result = of_property_read_string_index(pdev->dev.of_node,
			"interconnect-names",
			i,
			&ipa_drv_res->icc_path_name[i]);
		if (result) {
			IPAERR("invalid interconnect-names %d\n", i);
			return -EFAULT;
		}
	}
	/* read no-vote AB IB value */
	result = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,no-vote",
			ipa_drv_res->icc_clk_val[IPA_ICC_NONE],
			ipa_drv_res->icc_num_paths *
			IPA_ICC_TYPE_MAX);
	if (result) {
		IPAERR("invalid property qcom,no-vote\n");
		return -EFAULT;
	}

	/* read svs2 AB IB value */
	result = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,svs2",
			ipa_drv_res->icc_clk_val[IPA_ICC_SVS2],
			ipa_drv_res->icc_num_paths *
			IPA_ICC_TYPE_MAX);
	if (result) {
		IPAERR("invalid property qcom,svs2\n");
		return -EFAULT;
	}

	/* read svs AB IB value */
	result = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,svs",
			ipa_drv_res->icc_clk_val[IPA_ICC_SVS],
			ipa_drv_res->icc_num_paths *
			IPA_ICC_TYPE_MAX);
	if (result) {
		IPAERR("invalid property qcom,svs\n");
		return -EFAULT;
	}

	/* read nominal AB IB value */
	result = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,nominal",
			ipa_drv_res->icc_clk_val[IPA_ICC_NOMINAL],
			ipa_drv_res->icc_num_paths *
			IPA_ICC_TYPE_MAX);
	if (result) {
		IPAERR("invalid property qcom,nominal\n");
		return -EFAULT;
	}

	/* read turbo AB IB value */
	result = of_property_read_u32_array(pdev->dev.of_node,
			"qcom,turbo",
			ipa_drv_res->icc_clk_val[IPA_ICC_TURBO],
			ipa_drv_res->icc_num_paths *
			IPA_ICC_TYPE_MAX);
	if (result) {
		IPAERR("invalid property qcom,turbo\n");
		return -EFAULT;
	}

	result = of_property_read_u32_array(pdev->dev.of_node,
		"qcom,throughput-threshold",
		ipa_drv_res->pm_init.default_threshold,
		ipa_drv_res->pm_init.threshold_size);
	if (result) {
		IPAERR("failed to read qcom,throughput-thresholds\n");
		return -EFAULT;
	}

	result = of_property_count_strings(pdev->dev.of_node,
		"qcom,scaling-exceptions");
	if (result < 0) {
		IPADBG("no exception list for ipa pm\n");
		result = 0;
	}

	if (result % (ipa_drv_res->pm_init.threshold_size + 1)) {
		IPAERR("failed to read qcom,scaling-exceptions\n");
		return -EFAULT;
	}

	ipa_drv_res->pm_init.exception_size = result /
		(ipa_drv_res->pm_init.threshold_size + 1);
	if (ipa_drv_res->pm_init.exception_size >=
		IPA_PM_EXCEPTION_MAX) {
		IPAERR("exception list larger then max %d\n",
			ipa_drv_res->pm_init.exception_size);
		return -EFAULT;
	}

	for (i = 0; i < ipa_drv_res->pm_init.exception_size; i++) {
		struct ipa_pm_exception *ex = ipa_drv_res->pm_init.exceptions;

		result = of_property_read_string_index(pdev->dev.of_node,
			"qcom,scaling-exceptions",
			i * (ipa_drv_res->pm_init.threshold_size + 1),
			&ex[i].usecase);
		if (result) {
			IPAERR("failed to read qcom,scaling-exceptions");
			return -EFAULT;
		}

		for (j = 0; j < ipa_drv_res->pm_init.threshold_size; j++) {
			const char *str;

			result = of_property_read_string_index(
				pdev->dev.of_node,
				"qcom,scaling-exceptions",
				i * (ipa_drv_res->pm_init.threshold_size + 1)
				+ j + 1,
				&str);
			if (result) {
				IPAERR("failed to read qcom,scaling-exceptions"
					);
				return -EFAULT;
			}

			if (kstrtou32(str, 0, &ex[i].threshold[j])) {
				IPAERR("error str=%s\n", str);
				return -EFAULT;
			}
		}
	}

	return 0;
}

static void get_dts_tx_wrapper_cache_size(struct platform_device *pdev,
		struct ipa3_plat_drv_res *ipa_drv_res)
{
	int result;

	result = of_property_read_u32 (
		pdev->dev.of_node,
		"qcom,tx-wrapper-cache-max-size",
		&ipa_drv_res->tx_wrapper_cache_max_size);
	if (result)
		ipa_drv_res->tx_wrapper_cache_max_size = 0;

	IPADBG("tx_wrapper_cache_max_size is set to %d",
		ipa_drv_res->tx_wrapper_cache_max_size);
}


static void get_dts_ipa_gen_rx_cmn_page_pool_sz_factor(struct platform_device *pdev,
                struct ipa3_plat_drv_res *ipa_drv_res)
{
        int result;

        result = of_property_read_u32 (
                pdev->dev.of_node,
                "qcom,ipa-gen-rx-cmn-page-pool-sz-factor",
                &ipa_drv_res->ipa_gen_rx_cmn_page_pool_sz_factor);
        if (result)
                ipa_drv_res->ipa_gen_rx_cmn_page_pool_sz_factor = 0;

        IPADBG("ipa_gen_rx_cmn_page_pool_sz_factor is set to %d",
                ipa_drv_res->ipa_gen_rx_cmn_page_pool_sz_factor);
}


static void get_dts_ipa_gen_rx_cmn_temp_pool_sz_factor(struct platform_device *pdev,
                struct ipa3_plat_drv_res *ipa_drv_res)
{
        int result;

        result = of_property_read_u32 (
                pdev->dev.of_node,
                "qcom,ipa-gen-rx-cmn-temp-pool-sz-factor",
                &ipa_drv_res->ipa_gen_rx_cmn_temp_pool_sz_factor);
        if (result)
                ipa_drv_res->ipa_gen_rx_cmn_temp_pool_sz_factor = 0;

        IPADBG("ipa_gen_rx_cmn_temp_pool_sz_factor is set to %d",
                ipa_drv_res->ipa_gen_rx_cmn_temp_pool_sz_factor);
}

static void ipa_dts_get_ulso_data(struct platform_device *pdev,
		struct ipa3_plat_drv_res *ipa_drv_res)
{
	int result;
	u32 tmp;

	ipa_drv_res->ulso_supported = of_property_read_bool(pdev->dev.of_node,
		"qcom,ulso-supported");
	IPADBG(": ulso_supported = %d", ipa_drv_res->ulso_supported);
	if (!ipa_drv_res->ulso_supported)
		return;

	result = of_property_read_u32(
		pdev->dev.of_node,
		"qcom,ulso-ip-id-min-linux-val",
		&tmp);
	if (result) {
		ipa_drv_res->ulso_ip_id_min = 0;
	} else {
		ipa_drv_res->ulso_ip_id_min = tmp;
	}
	IPADBG("ulso_ip_id_min is set to %d",
		ipa_drv_res->ulso_ip_id_min);

	result = of_property_read_u32(
		pdev->dev.of_node,
		"qcom,ulso-ip-id-max-linux-val",
		&tmp);
	if (result) {
		ipa_drv_res->ulso_ip_id_max = 0xffff;
	} else {
		ipa_drv_res->ulso_ip_id_max = tmp;
	}
	IPADBG("ulso_ip_id_max is set to %d",
		ipa_drv_res->ulso_ip_id_max);
}

static int get_ipa_dts_configuration(struct platform_device *pdev,
		struct ipa3_plat_drv_res *ipa_drv_res)
{
	int i, result, pos;
	struct resource *resource;
	u32 *ipa_tz_unlock_reg;
	int elem_num;
	u32 mhi_evid_limits[2];
	u32 ipa_holb_monitor_poll_period;
	u32 ipa_holb_monitor_max_cnt_wlan;
	u32 ipa_holb_monitor_max_cnt_usb;
	u32 ipa_holb_monitor_max_cnt_11ad;
	u32 ipa_wan_aggr_pkt_cnt;

	/* initialize ipa3_res */
	ipa_drv_res->ipa_wdi3_2g_holb_timeout = 0;
	ipa_drv_res->ipa_wdi3_5g_holb_timeout = 0;
	ipa_drv_res->ipa_pipe_mem_start_ofst = IPA_PIPE_MEM_START_OFST;
	ipa_drv_res->ipa_pipe_mem_size = IPA_PIPE_MEM_SIZE;
	ipa_drv_res->ipa_hw_type = 0;
	ipa_drv_res->ipa3_hw_mode = 0;
	ipa_drv_res->platform_type = 0;
	ipa_drv_res->modem_cfg_emb_pipe_flt = false;
	ipa_drv_res->ipa_wdi2 = false;
	ipa_drv_res->ipa_wan_skb_page = false;
	ipa_drv_res->ipa_use_uc_holb_monitor = false;
	ipa_drv_res->ipa_wdi2_over_gsi = false;
	ipa_drv_res->ipa_wdi3_over_gsi = false;
	ipa_drv_res->use_xbl_boot = false;
	ipa_drv_res->ipa_mhi_dynamic_config = false;
	ipa_drv_res->use_64_bit_dma_mask = false;
	ipa_drv_res->use_bw_vote = false;
	ipa_drv_res->wan_rx_ring_size = IPA_GENERIC_RX_POOL_SZ_WAN;
	ipa_drv_res->lan_rx_ring_size = IPA_GENERIC_RX_POOL_SZ;
	ipa_drv_res->apply_rg10_wa = false;
	ipa_drv_res->gsi_ch20_wa = false;
	ipa_drv_res->ipa_tz_unlock_reg_num = 0;
	ipa_drv_res->ipa_tz_unlock_reg = NULL;
	ipa_drv_res->mhi_evid_limits[0] = IPA_MHI_GSI_EVENT_RING_ID_START;
	ipa_drv_res->mhi_evid_limits[1] = IPA_MHI_GSI_EVENT_RING_ID_END;
	ipa_drv_res->ipa_fltrt_not_hashable = false;
	ipa_drv_res->ipa_endp_delay_wa = false;
	ipa_drv_res->skip_ieob_mask_wa = false;
	ipa_drv_res->ipa_gpi_event_rp_ddr = false;
	ipa_drv_res->ipa_config_is_auto = false;
	ipa_drv_res->max_num_smmu_cb = IPA_SMMU_CB_MAX;
	ipa_drv_res->ipa_endp_delay_wa_v2 = false;
	ipa_drv_res->use_tput_est_ep = false;
	ipa_drv_res->rmnet_ctl_enable = 0;
	ipa_drv_res->rmnet_ll_enable = 0;
	ipa_drv_res->ulso_wa = false;
	ipa_drv_res->is_dual_pine_config = false;
	ipa_drv_res->gfp_no_retry = false;

	/* Get IPA HW Version */
	result = of_property_read_u32(pdev->dev.of_node, "qcom,ipa-hw-ver",
					&ipa_drv_res->ipa_hw_type);
	if ((result) || (ipa_drv_res->ipa_hw_type == IPA_HW_None)) {
		IPAERR(":get resource failed for ipa-hw-ver\n");
		return -ENODEV;
	}
	IPADBG(": ipa_hw_type = %d", ipa_drv_res->ipa_hw_type);

	if (ipa_drv_res->ipa_hw_type < IPA_HW_v3_0) {
		IPAERR(":IPA version below 3.0 not supported\n");
		return -ENODEV;
	}

	if (ipa_drv_res->ipa_hw_type >= IPA_HW_MAX) {
		IPAERR(":IPA version is greater than the MAX\n");
		return -ENODEV;
	}

	/* Get IPA HW mode */
	result = of_property_read_u32(pdev->dev.of_node, "qcom,ipa-hw-mode",
			&ipa_drv_res->ipa3_hw_mode);
	if (result)
		IPADBG("using default (IPA_MODE_NORMAL) for ipa-hw-mode\n");
	else
		IPADBG(": found ipa_drv_res->ipa3_hw_mode = %d",
				ipa_drv_res->ipa3_hw_mode);

	/* Get Platform Type */
	result = of_property_read_u32(pdev->dev.of_node, "qcom,platform-type",
			&ipa_drv_res->platform_type);
	if (result)
		IPADBG("using default (IPA_PLAT_TYPE_MDM) for platform-type\n");
	else
		IPADBG(": found ipa_drv_res->platform_type = %d",
				ipa_drv_res->platform_type);

	/* Get IPA WAN / LAN RX pool size */
	result = of_property_read_u32(pdev->dev.of_node,
			"qcom,wan-rx-ring-size",
			&ipa_drv_res->wan_rx_ring_size);
	if (result)
		IPADBG("using default for wan-rx-ring-size = %u\n",
				ipa_drv_res->wan_rx_ring_size);
	else
		IPADBG(": found ipa_drv_res->wan-rx-ring-size = %u",
				ipa_drv_res->wan_rx_ring_size);

	result = of_property_read_u32(pdev->dev.of_node,
			"qcom,lan-rx-ring-size",
			&ipa_drv_res->lan_rx_ring_size);
	if (result)
		IPADBG("using default for lan-rx-ring-size = %u\n",
			ipa_drv_res->lan_rx_ring_size);
	else
		IPADBG(": found ipa_drv_res->lan-rx-ring-size = %u",
			ipa_drv_res->lan_rx_ring_size);

	ipa_drv_res->use_ipa_teth_bridge =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,use-ipa-tethering-bridge");
	IPADBG(": using ipa teth bridge = %s",
		ipa_drv_res->use_ipa_teth_bridge
		? "True" : "False");

	ipa_drv_res->ipa_mhi_dynamic_config =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,use-ipa-in-mhi-mode");
	IPADBG(": ipa_mhi_dynamic_config (%s)\n",
		ipa_drv_res->ipa_mhi_dynamic_config
		? "True" : "False");

	ipa_drv_res->modem_cfg_emb_pipe_flt =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,modem-cfg-emb-pipe-flt");
	IPADBG(": modem configure embedded pipe filtering = %s\n",
			ipa_drv_res->modem_cfg_emb_pipe_flt
			? "True" : "False");
	ipa_drv_res->ipa_wdi2_over_gsi =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,ipa-wdi2_over_gsi");
	IPADBG(": WDI-2.0 over gsi= %s\n",
			ipa_drv_res->ipa_wdi2_over_gsi
			? "True" : "False");

	ipa_drv_res->ipa_endp_delay_wa =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,ipa-endp-delay-wa");
	IPADBG(": endppoint delay wa = %s\n",
		ipa_drv_res->ipa_endp_delay_wa
		? "True" : "False");

	ipa_drv_res->ipa_endp_delay_wa_v2 =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,ipa-endp-delay-wa-v2");
	IPADBG(": endppoint delay wa v2 = %s\n",
		ipa_drv_res->ipa_endp_delay_wa_v2
		? "True" : "False");

	/**
	 * Overwrite end point delay workaround for
	 * APQ target as device tree is same
	 * for MSM and APQ
	 */
	if (ipa_drv_res->platform_type == IPA_PLAT_TYPE_APQ) {
		ipa_drv_res->ipa_endp_delay_wa = true;
		ipa_drv_res->ipa_endp_delay_wa_v2 = false;
	}

	ipa_drv_res->gfp_no_retry = of_property_read_bool(pdev->dev.of_node,
			"qcom,gfp-no-retry");
	IPADBG(": gfp-no-retry = %s\n",
			ipa_drv_res->gfp_no_retry
			? "True" : "False");

	ipa_drv_res->ulso_wa = of_property_read_bool(pdev->dev.of_node,
			"qcom,ipa-ulso-wa");
	IPADBG(": ipa-ulso wa = %s\n",
			ipa_drv_res->ulso_wa
			? "True" : "False");

	ipa_drv_res->ipa_wdi3_over_gsi =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,ipa-wdi3-over-gsi");
	IPADBG(": WDI-3.0 over gsi= %s\n",
			ipa_drv_res->ipa_wdi3_over_gsi
			? "True" : "False");

	ipa_drv_res->ipa_wdi2 =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,ipa-wdi2");
	IPADBG(": WDI-2.0 = %s\n",
			ipa_drv_res->ipa_wdi2
			? "True" : "False");

	ipa_drv_res->ipa_config_is_auto =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,ipa-config-is-auto");
	IPADBG(": ipa-config-is-auto = %s\n",
		ipa_drv_res->ipa_config_is_auto
		? "True" : "False");

	ipa_drv_res->ipa_wan_skb_page =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,wan-use-skb-page");
	IPADBG(": Use skb page = %s\n",
			ipa_drv_res->ipa_wan_skb_page
			? "True" : "False");

	ipa_drv_res->ipa_use_uc_holb_monitor =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,ipa-uc-holb-monitor");
	IPADBG(": uC HOLB monitor = %s\n",
			ipa_drv_res->ipa_use_uc_holb_monitor
			? "True" : "False");

	/* Get HOLB Monitor Polling Period */
	result = of_property_read_u32(pdev->dev.of_node,
			"qcom,ipa-holb-monitor-poll-period",
			&ipa_holb_monitor_poll_period);
	if (result) {
		IPADBG("ipa holb monitor poll period = %u\n",
			IPA_HOLB_POLLING_PERIOD_MS);
		ipa_holb_monitor_poll_period = IPA_HOLB_POLLING_PERIOD_MS;
	} else
		IPADBG("ipa holb monitor poll period = %u\n",
			ipa_holb_monitor_poll_period);

	ipa_drv_res->ipa_holb_monitor_poll_period =
			ipa_holb_monitor_poll_period;

	/* Get HOLB Monitor Max Stuck Cnt Values */
	result = of_property_read_u32(pdev->dev.of_node,
			"qcom,ipa-holb-monitor-max-cnt-wlan",
			&ipa_holb_monitor_max_cnt_wlan);
	if (result) {
		IPADBG("ipa holb monitor max count wlan = %u\n",
			IPA_HOLB_MONITOR_MAX_STUCK_COUNT);
		ipa_holb_monitor_max_cnt_wlan =
				IPA_HOLB_MONITOR_MAX_STUCK_COUNT;
	} else
		IPADBG("ipa holb monitor max count wlan = %u\n",
			ipa_holb_monitor_max_cnt_wlan);

	ipa_drv_res->ipa_holb_monitor_max_cnt_wlan =
			ipa_holb_monitor_max_cnt_wlan;

	result = of_property_read_u32(pdev->dev.of_node,
			"qcom,ipa-holb-monitor-max-cnt-usb",
			&ipa_holb_monitor_max_cnt_usb);
	if (result) {
		IPADBG("ipa holb monitor max count usb = %u\n",
			IPA_HOLB_MONITOR_MAX_STUCK_COUNT);
		ipa_holb_monitor_max_cnt_usb =
				IPA_HOLB_MONITOR_MAX_STUCK_COUNT;
	} else
		IPADBG("ipa holb monitor max count usb = %u\n",
			ipa_holb_monitor_max_cnt_usb);

	ipa_drv_res->ipa_holb_monitor_max_cnt_usb =
			ipa_holb_monitor_max_cnt_usb;

	result = of_property_read_u32(pdev->dev.of_node,
			"qcom,ipa-holb-monitor-max-cnt-11ad",
			&ipa_holb_monitor_max_cnt_11ad);
	if (result) {
		IPADBG("ipa holb monitor max count 11ad = %u\n",
			IPA_HOLB_MONITOR_MAX_STUCK_COUNT);
		ipa_holb_monitor_max_cnt_11ad =
			IPA_HOLB_MONITOR_MAX_STUCK_COUNT;
	} else
		IPADBG("ipa holb monitor max count 11ad = %u\n",
			ipa_holb_monitor_max_cnt_11ad);

	ipa_drv_res->ipa_holb_monitor_max_cnt_11ad =
			ipa_holb_monitor_max_cnt_11ad;

	ipa_drv_res->ipa_fltrt_not_hashable =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,ipa-fltrt-not-hashable");
	IPADBG(": IPA filter/route rule hashable = %s\n",
			ipa_drv_res->ipa_fltrt_not_hashable
			? "True" : "False");

	ipa_drv_res->use_xbl_boot =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,use-xbl-boot");
	IPADBG("Is xbl loading used ? (%s)\n",
			ipa_drv_res->use_xbl_boot
			? "Yes":"No");

	ipa_drv_res->use_64_bit_dma_mask =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,use-64-bit-dma-mask");
	IPADBG(": use_64_bit_dma_mask = %s\n",
			ipa_drv_res->use_64_bit_dma_mask
			? "True" : "False");

	ipa_drv_res->use_bw_vote =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,bandwidth-vote-for-ipa");
	IPADBG(": use_bw_vote = %s\n",
			ipa_drv_res->use_bw_vote
			? "True" : "False");
	ipa_drv_res->skip_ieob_mask_wa =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,skip-ieob-mask-wa");
	IPADBG(": skip ieob mask wa = %s\n",
			ipa_drv_res->skip_ieob_mask_wa
			? "True" : "False");

	ipa_drv_res->skip_uc_pipe_reset =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,skip-uc-pipe-reset");
	IPADBG(": skip uC pipe reset = %s\n",
		ipa_drv_res->skip_uc_pipe_reset
		? "True" : "False");

	ipa_drv_res->tethered_flow_control =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,tethered-flow-control");
	IPADBG(": Use apps based flow control = %s\n",
		ipa_drv_res->tethered_flow_control
		? "True" : "False");

	ipa_drv_res->lan_rx_napi_enable =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,lan-rx-napi");
	IPADBG(": Enable LAN rx NAPI = %s\n",
		ipa_drv_res->lan_rx_napi_enable
		? "True" : "False");

	ipa_drv_res->ipa_gpi_event_rp_ddr =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,ipa-gpi-event-rp-ddr");
	IPADBG(": Read GPI or GCI Event RP from DDR = %s\n",
		ipa_drv_res->ipa_gpi_event_rp_ddr ? "True" : "False");

	ipa_drv_res->tx_napi_enable =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,tx-napi");
	IPADBG(": Enable tx NAPI = %s\n",
		ipa_drv_res->tx_napi_enable
		? "True" : "False");

	ipa_drv_res->tx_poll = of_property_read_bool(pdev->dev.of_node,
		"qcom,tx-poll");
	IPADBG(": Enable tx polling = %s\n", ipa_drv_res->tx_poll
		? "True" : "False");

	if (ipa_drv_res->platform_type != IPA_PLAT_TYPE_APQ) {
		ipa_drv_res->rmnet_ctl_enable =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,rmnet-ctl-enable");
		IPADBG(": Enable rmnet ctl = %s\n",
			ipa_drv_res->rmnet_ctl_enable
			? "True" : "False");

		ipa_drv_res->rmnet_ll_enable =
			of_property_read_bool(pdev->dev.of_node,
			"qcom,rmnet-ll-enable");
		IPADBG(": Enable rmnet ll = %s\n",
			ipa_drv_res->rmnet_ll_enable
			? "True" : "False");
	}

	result = of_property_read_string(pdev->dev.of_node,
			"qcom,use-gsi-ipa-fw", &ipa_drv_res->gsi_fw_file_name);
	if (!result)
		IPADBG("GSI IPA FW name %s\n", ipa_drv_res->gsi_fw_file_name);
	else
		IPADBG("GSI IPA FW file not defined. Using default one\n");
	result = of_property_read_string(pdev->dev.of_node,
			"qcom,use-uc-ipa-fw", &ipa_drv_res->uc_fw_file_name);
	if (!result)
		IPADBG("uC IPA FW name = %s\n", ipa_drv_res->uc_fw_file_name);
	else
		IPADBG("uC IPA FW file not defined. Using default one\n");

	ipa_drv_res->ipa_mhi_proxy =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,ipa-mhi-proxy");
	IPADBG(": Use mhi proxy = %s\n",
		ipa_drv_res->ipa_mhi_proxy
		? "True" : "False");

	/* Get IPA wrapper address */
	result = of_property_read_u32(pdev->dev.of_node, "qcom,ipa-cfg-offset",
		&ipa_drv_res->ipa_cfg_offset);
	if (!result) {
		IPADBG(": Read offset of IPA_CFG from IPA_WRAPPER_BASE = 0x%x\n",
			ipa_drv_res->ipa_cfg_offset);
	} else {
		ipa_drv_res->ipa_cfg_offset = 0;
		IPADBG("IPA_CFG_OFFSET not defined. Using default one\n");
	}

	resource = platform_get_resource_byname(pdev, IORESOURCE_MEM,
			"ipa-base");
	if (!resource) {
		IPAERR(":get resource failed for ipa-base!\n");
		return -ENODEV;
	}
	ipa_drv_res->ipa_mem_base = resource->start;
	ipa_drv_res->ipa_mem_size = resource_size(resource);
	IPADBG(": ipa-base = 0x%x, size = 0x%x\n",
			ipa_drv_res->ipa_mem_base,
			ipa_drv_res->ipa_mem_size);

	smmu_info.ipa_base = ipa_drv_res->ipa_mem_base;
	smmu_info.ipa_size = ipa_drv_res->ipa_mem_size;

	/* Get IPA GSI address */
	resource = platform_get_resource_byname(pdev, IORESOURCE_MEM,
			"gsi-base");
	if (!resource) {
		IPAERR(":get resource failed for gsi-base\n");
		return -ENODEV;
	}
	ipa_drv_res->transport_mem_base = resource->start;
	ipa_drv_res->transport_mem_size = resource_size(resource);
	IPADBG(": gsi-base = 0x%x, size = 0x%x\n",
			ipa_drv_res->transport_mem_base,
			ipa_drv_res->transport_mem_size);

	/* Get IPA GSI IRQ number */
	resource = platform_get_resource_byname(pdev, IORESOURCE_IRQ,
			"gsi-irq");
	if (!resource) {
		IPAERR(":get resource failed for gsi-irq\n");
		return -ENODEV;
	}
	ipa_drv_res->transport_irq = resource->start;
	IPADBG(": gsi-irq = %d\n", ipa_drv_res->transport_irq);

	/* Get IPA pipe mem start ofst */
	resource = platform_get_resource_byname(pdev, IORESOURCE_MEM,
			"ipa-pipe-mem");
	if (!resource) {
		IPADBG(":not using pipe memory - resource nonexisting\n");
	} else {
		ipa_drv_res->ipa_pipe_mem_start_ofst = resource->start;
		ipa_drv_res->ipa_pipe_mem_size = resource_size(resource);
		IPADBG(":using pipe memory - at 0x%x of size 0x%x\n",
				ipa_drv_res->ipa_pipe_mem_start_ofst,
				ipa_drv_res->ipa_pipe_mem_size);
	}

	/* Get IPA IRQ number */
	resource = platform_get_resource_byname(pdev, IORESOURCE_IRQ,
			"ipa-irq");
	if (!resource) {
		IPAERR(":get resource failed for ipa-irq\n");
		return -ENODEV;
	}
	ipa_drv_res->ipa_irq = resource->start;
	IPADBG(":ipa-irq = %d\n", ipa_drv_res->ipa_irq);

	result = of_property_read_u32(pdev->dev.of_node, "qcom,ee",
			&ipa_drv_res->ee);
	if (result)
		ipa_drv_res->ee = 0;
	IPADBG(":ee = %u\n", ipa_drv_res->ee);

	ipa_drv_res->apply_rg10_wa =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,use-rg10-limitation-mitigation");
	IPADBG(": Use Register Group 10 limitation mitigation = %s\n",
		ipa_drv_res->apply_rg10_wa
		? "True" : "False");

	ipa_drv_res->gsi_ch20_wa =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,do-not-use-ch-gsi-20");
	IPADBG(": GSI CH 20 WA is = %s\n",
		ipa_drv_res->gsi_ch20_wa
		? "Needed" : "Not needed");

	ipa_drv_res->use_pm_wrapper = false;
	ipa_drv_res->use_pm_wrapper =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,use-wrapper-pm-support");
	IPADBG(": Use PM wrapper Support = %s\n",
		ipa_drv_res->use_pm_wrapper
		? "Needed" : "Not needed");

	ipa_drv_res->use_tput_est_ep =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,use-tput-estmation-pipe");
	IPADBG(": Use Tput estimation ep = %s\n",
		ipa_drv_res->use_tput_est_ep
		? "Needed" : "Not needed");

	elem_num = of_property_count_elems_of_size(pdev->dev.of_node,
		"qcom,mhi-event-ring-id-limits", sizeof(u32));

	if (elem_num == 2) {
		if (of_property_read_u32_array(pdev->dev.of_node,
			"qcom,mhi-event-ring-id-limits", mhi_evid_limits, 2)) {
			IPAERR("failed to read mhi event ring id limits\n");
			return -EFAULT;
		}
		if (mhi_evid_limits[0] > mhi_evid_limits[1]) {
			IPAERR("mhi event ring id low limit > high limit\n");
			return -EFAULT;
		}
		ipa_drv_res->mhi_evid_limits[0] = mhi_evid_limits[0];
		ipa_drv_res->mhi_evid_limits[1] = mhi_evid_limits[1];
		IPADBG(": mhi-event-ring-id-limits start=%u end=%u\n",
			mhi_evid_limits[0], mhi_evid_limits[1]);
	} else {
		if (elem_num > 0) {
			IPAERR("Invalid mhi event ring id limits number %d\n",
				elem_num);
			return -EINVAL;
		}
		IPADBG("use default mhi evt ring id limits start=%u end=%u\n",
			ipa_drv_res->mhi_evid_limits[0],
			ipa_drv_res->mhi_evid_limits[1]);
	}

	elem_num = of_property_count_elems_of_size(pdev->dev.of_node,
		"qcom,ipa-tz-unlock-reg", sizeof(u32));

	if (elem_num > 0 && elem_num % 2 == 0) {
		ipa_drv_res->ipa_tz_unlock_reg_num = elem_num / 2;

		ipa_tz_unlock_reg = kcalloc(elem_num, sizeof(u32), GFP_KERNEL);
		if (ipa_tz_unlock_reg == NULL)
			return -ENOMEM;

		ipa_drv_res->ipa_tz_unlock_reg = kcalloc(
			ipa_drv_res->ipa_tz_unlock_reg_num,
			sizeof(*ipa_drv_res->ipa_tz_unlock_reg),
			GFP_KERNEL);
		if (ipa_drv_res->ipa_tz_unlock_reg == NULL) {
			kfree(ipa_tz_unlock_reg);
			return -ENOMEM;
		}

		if (of_property_read_u32_array(pdev->dev.of_node,
			"qcom,ipa-tz-unlock-reg", ipa_tz_unlock_reg,
			elem_num)) {
			IPAERR("failed to read register addresses\n");
			kfree(ipa_tz_unlock_reg);
			kfree(ipa_drv_res->ipa_tz_unlock_reg);
			ipa_drv_res->ipa_tz_unlock_reg = NULL;
			return -EFAULT;
		}

		pos = 0;
		for (i = 0; i < ipa_drv_res->ipa_tz_unlock_reg_num; i++) {
			ipa_drv_res->ipa_tz_unlock_reg[i].reg_addr =
				ipa_tz_unlock_reg[pos++];
			ipa_drv_res->ipa_tz_unlock_reg[i].size =
				ipa_tz_unlock_reg[pos++];
			IPADBG("tz unlock reg %d: addr 0x%pa size %llu\n", i,
				&ipa_drv_res->ipa_tz_unlock_reg[i].reg_addr,
				ipa_drv_res->ipa_tz_unlock_reg[i].size);
		}
		kfree(ipa_tz_unlock_reg);
	}

	/* get HOLB_TO numbers for wdi3 tx pipe */
	result = of_property_read_u32(pdev->dev.of_node,
			"qcom,ipa-wdi3-holb-2g",
			&ipa_drv_res->ipa_wdi3_2g_holb_timeout);
	if (result)
		IPADBG("Not able to get the holb for 2g pipe = %u\n",
			ipa_drv_res->ipa_wdi3_2g_holb_timeout);
	else
		IPADBG(": found ipa_drv_res->ipa_wdi3_2g_holb_timeout = %u",
			ipa_drv_res->ipa_wdi3_2g_holb_timeout);

	/* get HOLB_TO numbers for wdi3 tx1 pipe */
	result = of_property_read_u32(pdev->dev.of_node,
			"qcom,ipa-wdi3-holb-5g",
			&ipa_drv_res->ipa_wdi3_5g_holb_timeout);
	if (result)
		IPADBG("Not able to get the holb for 5g pipe = %u\n",
			ipa_drv_res->ipa_wdi3_5g_holb_timeout);
	else
		IPADBG(": found ipa_drv_res->ipa_wdi3_2g_holb_timeout = %u",
			ipa_drv_res->ipa_wdi3_2g_holb_timeout);

	/* get IPA PM related information */
	result = get_ipa_dts_pm_info(pdev, ipa_drv_res);
	if (result) {
		IPAERR("failed to get pm info from dts %d\n", result);
		return result;
	}

	ipa_drv_res->wdi_over_pcie =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,wlan-ce-db-over-pcie");
	IPADBG("Is wdi_over_pcie ? (%s)\n",
		ipa_drv_res->wdi_over_pcie ? "Yes":"No");

	/*
	 * If we're on emulator, get its interrupt controller's mem
	 * start and size
	 */
	if (ipa_drv_res->ipa3_hw_mode == IPA_HW_MODE_EMULATION) {
		resource = platform_get_resource_byname(
		    pdev, IORESOURCE_MEM, "intctrl-base");
		if (!resource) {
			IPAERR(":Can't find intctrl-base resource\n");
			return -ENODEV;
		}
		ipa_drv_res->emulator_intcntrlr_mem_base =
		    resource->start;
		ipa_drv_res->emulator_intcntrlr_mem_size =
		    resource_size(resource);
		IPADBG(":using intctrl-base at 0x%x of size 0x%x\n",
			ipa_drv_res->emulator_intcntrlr_mem_base,
			ipa_drv_res->emulator_intcntrlr_mem_size);
	}

	ipa_drv_res->entire_ipa_block_size = 0x100000;
	result = of_property_read_u32(pdev->dev.of_node,
		"qcom,entire-ipa-block-size",
		&ipa_drv_res->entire_ipa_block_size);
	IPADBG(": entire_ipa_block_size = %d\n",
		ipa_drv_res->entire_ipa_block_size);

	/*
	 * We'll read register-collection-on-crash here, but log it
	 * later below because its value may change based on other
	 * subsequent dtsi reads......
	 */
	ipa_drv_res->do_register_collection_on_crash =
	    of_property_read_bool(pdev->dev.of_node,
				  "qcom,register-collection-on-crash");
	/*
	 * We'll read testbus-collection-on-crash here...
	 */
	ipa_drv_res->do_testbus_collection_on_crash =
	    of_property_read_bool(pdev->dev.of_node,
				  "qcom,testbus-collection-on-crash");
	IPADBG(": doing testbus collection on crash = %u\n",
		ipa_drv_res->do_testbus_collection_on_crash);

	/*
	 * We'll read non-tn-collection-on-crash here...
	 */
	ipa_drv_res->do_non_tn_collection_on_crash =
	    of_property_read_bool(pdev->dev.of_node,
				  "qcom,non-tn-collection-on-crash");
	IPADBG(": doing non-tn collection on crash = %u\n",
		ipa_drv_res->do_non_tn_collection_on_crash);

	/*
	 * We'll read ram-collection-on-crash here...
	 */
	ipa_drv_res->do_ram_collection_on_crash =
		of_property_read_bool(
			pdev->dev.of_node,
			"qcom,ram-collection-on-crash");
	IPADBG(": doing ram collection on crash = %u\n",
		   ipa_drv_res->do_ram_collection_on_crash);

	if (ipa_drv_res->do_testbus_collection_on_crash ||
		ipa_drv_res->do_non_tn_collection_on_crash ||
		ipa_drv_res->do_ram_collection_on_crash)
		ipa_drv_res->do_register_collection_on_crash = true;

	IPADBG(": doing register collection on crash = %u\n",
		ipa_drv_res->do_register_collection_on_crash);

	result = of_property_read_u32(
		pdev->dev.of_node,
		"qcom,secure-debug-check-action",
		&ipa_drv_res->secure_debug_check_action);
	if (result ||
		(ipa_drv_res->secure_debug_check_action != 0 &&
		 ipa_drv_res->secure_debug_check_action != 1 &&
		 ipa_drv_res->secure_debug_check_action != 2))
		ipa_drv_res->secure_debug_check_action = USE_SCM;

	IPADBG(": secure-debug-check-action = %d\n",
		   ipa_drv_res->secure_debug_check_action);


	result = of_property_read_u32(
		pdev->dev.of_node,
		"qcom,ipa-wan-aggr-pkt-cnt",
		&ipa_wan_aggr_pkt_cnt);
	if (result) {
		ipa_wan_aggr_pkt_cnt = IPA_WAN_AGGR_PKT_CNT;
		IPADBG("ipa wan aggr pkt cnt = %u\n", ipa_wan_aggr_pkt_cnt);
	} else
		IPADBG("ipa wan aggr pkt cnt = %u\n", ipa_wan_aggr_pkt_cnt);

	ipa_drv_res->ipa_wan_aggr_pkt_cnt = ipa_wan_aggr_pkt_cnt;

	get_dts_tx_wrapper_cache_size(pdev, ipa_drv_res);

	get_dts_ipa_gen_rx_cmn_page_pool_sz_factor(pdev, ipa_drv_res);

        get_dts_ipa_gen_rx_cmn_temp_pool_sz_factor(pdev, ipa_drv_res);

	ipa_dts_get_ulso_data(pdev, ipa_drv_res);

	result = of_property_read_u32(pdev->dev.of_node,
		"qcom,max_num_smmu_cb",
		&ipa_drv_res->max_num_smmu_cb);
	if (result)
		IPADBG(": using default max number of cb = %d\n",
			ipa_drv_res->max_num_smmu_cb);
	else
		IPADBG(": found ipa_drv_res->max_num_smmu_cb = %d\n",
			ipa_drv_res->max_num_smmu_cb);

	ipa_drv_res->is_dual_pine_config =
		of_property_read_bool(pdev->dev.of_node,
		"qcom,use-dual-pine-config");
	IPADBG(": Use dual pine config = %s\n",
		ipa_drv_res->is_dual_pine_config
		? "True" : "False");

	return 0;
}

static int ipa_smmu_perph_cb_probe(struct device *dev,
	enum ipa_smmu_cb_type cb_type)
{
	struct ipa_smmu_cb_ctx *cb = ipa3_get_smmu_ctx(cb_type);
	int fast = 0;
	int bypass = 0;
	u32 add_map_size;
	const u32 *add_map;
	int i;
	u32 iova_ap_mapping[2];

	IPADBG("CB %d PROBE dev=%pK\n", cb_type, dev);

	if (!smmu_info.present[cb_type]) {
		IPAERR("cb %d is disabled\n", cb_type);
		return 0;
	}

	IPADBG("CB %d PROBE dev=%pK retrieving IOMMU mapping\n", cb_type, dev);

	cb->iommu_domain = iommu_get_domain_for_dev(dev);
	if (IS_ERR_OR_NULL(cb->iommu_domain)) {
		IPAERR("could not get iommu domain\n");
		return -EINVAL;
	}

	IPADBG("CB %d PROBE mapping retrieved\n", cb_type);
	cb->is_cache_coherent = of_property_read_bool(dev->of_node,
		"dma-coherent");
	cb->dev   = dev;
	cb->valid = true;

	cb->va_start = cb->va_end  = cb->va_size = 0;
	if (of_property_read_u32_array(
			dev->of_node, "qcom,iommu-dma-addr-pool",
			iova_ap_mapping, 2) == 0) {
		cb->va_start = iova_ap_mapping[0];
		cb->va_size  = iova_ap_mapping[1];
		cb->va_end   = cb->va_start + cb->va_size;
	}

	IPADBG("CB %d PROBE dev=%pK va_start=0x%x va_size=0x%x\n",
		   cb_type, dev, cb->va_start, cb->va_size);

	/*
	 * Prior to these calls to iommu_domain_get_attr(), these
	 * attributes were set in this function relative to dtsi values
	 * defined for this driver.  In other words, if corresponding ipa
	 * driver owned values were found in the dtsi, they were read and
	 * set here.
	 *
	 * In this new world, the developer will use iommu owned dtsi
	 * settings to set them there.  This new logic below, simply
	 * checks to see if they've been set in dtsi.  If so, the logic
	 * further below acts accordingly...
	 */
	iommu_domain_get_attr(cb->iommu_domain, DOMAIN_ATTR_S1_BYPASS, &bypass);
	iommu_domain_get_attr(cb->iommu_domain, DOMAIN_ATTR_FAST, &fast);

	IPADBG(
	  "CB %d PROBE dev=%pK DOMAIN ATTRS bypass=%d fast=%d\n",
	  cb_type, dev, bypass, fast);

	ipa3_ctx->s1_bypass_arr[cb_type] = (bypass != 0);

	if (of_property_read_bool(dev->of_node, "qcom,shared-cb")) {
		IPADBG("CB %d using shared CB\n", cb_type);
		cb->shared = true;
	}

	/* MAP ipa-uc ram */
	add_map = of_get_property(dev->of_node,
		"qcom,additional-mapping", &add_map_size);
	if (add_map) {
		/* mapping size is an array of 3-tuple of u32 */
		if (add_map_size % (3 * sizeof(u32))) {
			IPAERR("wrong additional mapping format\n");
			cb->valid = false;
			return -EFAULT;
		}

		/* iterate of each entry of the additional mapping array */
		for (i = 0; i < add_map_size / sizeof(u32); i += 3) {
			u32 iova = be32_to_cpu(add_map[i]);
			u32 pa = be32_to_cpu(add_map[i + 1]);
			u32 size = be32_to_cpu(add_map[i + 2]);
			unsigned long iova_p;
			phys_addr_t pa_p;
			u32 size_p;

			IPA_SMMU_ROUND_TO_PAGE(iova, pa, size,
				iova_p, pa_p, size_p);
			IPADBG_LOW("mapping 0x%lx to 0x%pa size %d\n",
				iova_p, &pa_p, size_p);
			ipa3_iommu_map(cb->iommu_domain,
				iova_p, pa_p, size_p,
				IOMMU_READ | IOMMU_WRITE | IOMMU_MMIO);
		}
	}

	cb->done = true;
	return 0;
}

static int ipa_smmu_uc_cb_probe(struct device *dev)
{
	struct ipa_smmu_cb_ctx *cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_UC);
	int bypass = 0;
	int fast = 0;
	u32 iova_ap_mapping[2];

	IPADBG("UC CB PROBE dev=%pK\n", dev);

	if (!smmu_info.present[IPA_SMMU_CB_UC]) {
		IPAERR("UC SMMU is disabled\n");
		return 0;
	}

	if (smmu_info.use_64_bit_dma_mask) {
		if (dma_set_mask(dev, DMA_BIT_MASK(64)) ||
			dma_set_coherent_mask(dev, DMA_BIT_MASK(64))) {
			IPAERR("DMA set 64bit mask failed\n");
			return -EOPNOTSUPP;
		}
	} else {
		if (dma_set_mask(dev, DMA_BIT_MASK(32)) ||
			dma_set_coherent_mask(dev, DMA_BIT_MASK(32))) {
			IPAERR("DMA set 32bit mask failed\n");
			return -EOPNOTSUPP;
		}
	}

	IPADBG("UC CB PROBE dev=%pK retrieving IOMMU mapping\n", dev);

	cb->iommu_domain = iommu_get_domain_for_dev(dev);
	if (IS_ERR_OR_NULL(cb->iommu_domain)) {
		IPAERR("could not get iommu domain\n");
		return -EINVAL;
	}

	IPADBG("UC CB PROBE mapping retrieved\n");

	cb->is_cache_coherent = of_property_read_bool(dev->of_node,
						"dma-coherent");
	cb->dev   = dev;
	cb->valid = true;

	cb->va_start = cb->va_end  = cb->va_size = 0;
	if (of_property_read_u32_array(
			dev->of_node, "qcom,iommu-dma-addr-pool",
			iova_ap_mapping, 2) == 0) {
		cb->va_start = iova_ap_mapping[0];
		cb->va_size  = iova_ap_mapping[1];
		cb->va_end   = cb->va_start + cb->va_size;
	}

	IPADBG("UC CB PROBE dev=%pK va_start=0x%x va_size=0x%x\n",
		   dev, cb->va_start, cb->va_size);

	/*
	 * Prior to these calls to iommu_domain_get_attr(), these
	 * attributes were set in this function relative to dtsi values
	 * defined for this driver.  In other words, if corresponding ipa
	 * driver owned values were found in the dtsi, they were read and
	 * set here.
	 *
	 * In this new world, the developer will use iommu owned dtsi
	 * settings to set them there.  This new logic below, simply
	 * checks to see if they've been set in dtsi.  If so, the logic
	 * further below acts accordingly...
	 */
	iommu_domain_get_attr(cb->iommu_domain, DOMAIN_ATTR_S1_BYPASS, &bypass);
	iommu_domain_get_attr(cb->iommu_domain, DOMAIN_ATTR_FAST, &fast);

	IPADBG("UC CB PROBE dev=%pK DOMAIN ATTRS bypass=%d fast=%d\n",
		   dev, bypass, fast);

	ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_UC] = (bypass != 0);

	ipa3_ctx->uc_pdev = dev;
	cb->done = true;
	return 0;
}

static void ipa3_ap_iommu_unmap(struct ipa_smmu_cb_ctx *cb, const u32 *add_map, u32 add_map_size) {

	int i, res;

	/* iterate of each entry of the additional mapping array */
	for (i = 0; i < add_map_size / sizeof(u32); i += 3) {
		u32 iova = be32_to_cpu(add_map[i]);
		u32 pa = be32_to_cpu(add_map[i + 1]);
		u32 size = be32_to_cpu(add_map[i + 2]);
		unsigned long iova_p;
		phys_addr_t pa_p;
		u32 size_p;

		IPA_SMMU_ROUND_TO_PAGE(iova, pa, size,
			iova_p, pa_p, size_p);
		IPADBG_LOW("unmapping 0x%lx to 0x%pa size %d\n",
				iova_p, &pa_p, size_p);

		res = iommu_unmap(cb->iommu_domain,iova_p, size_p);
		if(res != size_p) {
			pr_err("iommu unmap failed for AP cb\n");
			ipa_assert();
		}
	}
}
static int ipa_smmu_ap_cb_probe(struct device *dev)
{
	struct ipa_smmu_cb_ctx *cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_AP);
	int fast = 0;
	int bypass = 0;
	u32 add_map_size;
	const u32 *add_map;
	void *smem_addr;
	size_t smem_size;
	u32 ipa_smem_size = 0;
	int ret;
	int i;
	unsigned long iova_p;
	phys_addr_t pa_p;
	u32 size_p;
	phys_addr_t iova;
	phys_addr_t pa;
	u32 iova_ap_mapping[2];

	IPADBG("AP CB PROBE dev=%pK\n", dev);

	if (!smmu_info.present[IPA_SMMU_CB_AP]) {
		IPAERR("AP SMMU is disabled");
		return 0;
	}

	if (smmu_info.use_64_bit_dma_mask) {
		if (dma_set_mask(dev, DMA_BIT_MASK(64)) ||
			dma_set_coherent_mask(dev, DMA_BIT_MASK(64))) {
			IPAERR("DMA set 64bit mask failed\n");
			return -EOPNOTSUPP;
		}
	} else {
		if (dma_set_mask(dev, DMA_BIT_MASK(32)) ||
			dma_set_coherent_mask(dev, DMA_BIT_MASK(32))) {
			IPAERR("DMA set 32bit mask failed\n");
			return -EOPNOTSUPP;
		}
	}

	IPADBG("AP CB PROBE dev=%pK retrieving IOMMU mapping\n", dev);

	cb->iommu_domain = iommu_get_domain_for_dev(dev);
	if (IS_ERR_OR_NULL(cb->iommu_domain)) {
		IPAERR("could not get iommu domain\n");
		return -EINVAL;
	}

	IPADBG("AP CB PROBE mapping retrieved\n");

	cb->is_cache_coherent = of_property_read_bool(dev->of_node,
						"dma-coherent");
	cb->dev   = dev;
	cb->valid = true;

	cb->va_start = cb->va_end  = cb->va_size = 0;
	if (of_property_read_u32_array(
			dev->of_node, "qcom,iommu-dma-addr-pool",
			iova_ap_mapping, 2) == 0) {
		cb->va_start = iova_ap_mapping[0];
		cb->va_size  = iova_ap_mapping[1];
		cb->va_end   = cb->va_start + cb->va_size;
	}

	IPADBG("AP CB PROBE dev=%pK va_start=0x%x va_size=0x%x\n",
		   dev, cb->va_start, cb->va_size);

	/*
	 * Prior to these calls to iommu_domain_get_attr(), these
	 * attributes were set in this function relative to dtsi values
	 * defined for this driver.  In other words, if corresponding ipa
	 * driver owned values were found in the dtsi, they were read and
	 * set here.
	 *
	 * In this new world, the developer will use iommu owned dtsi
	 * settings to set them there.  This new logic below, simply
	 * checks to see if they've been set in dtsi.  If so, the logic
	 * further below acts accordingly...
	 */
	iommu_domain_get_attr(cb->iommu_domain, DOMAIN_ATTR_S1_BYPASS, &bypass);
	iommu_domain_get_attr(cb->iommu_domain, DOMAIN_ATTR_FAST, &fast);

	IPADBG("AP CB PROBE dev=%pK DOMAIN ATTRS bypass=%d fast=%d\n",
		   dev, bypass, fast);

	ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_AP] = (bypass != 0);

	add_map = of_get_property(dev->of_node,
		"qcom,additional-mapping", &add_map_size);
	if (add_map) {
		/* mapping size is an array of 3-tuple of u32 */
		if (add_map_size % (3 * sizeof(u32))) {
			IPAERR("wrong additional mapping format\n");
			cb->valid = false;
			return -EFAULT;
		}

		/* iterate of each entry of the additional mapping array */
		for (i = 0; i < add_map_size / sizeof(u32); i += 3) {
			u32 iova = be32_to_cpu(add_map[i]);
			u32 pa = be32_to_cpu(add_map[i + 1]);
			u32 size = be32_to_cpu(add_map[i + 2]);
			unsigned long iova_p;
			phys_addr_t pa_p;
			u32 size_p;

			IPA_SMMU_ROUND_TO_PAGE(iova, pa, size,
				iova_p, pa_p, size_p);
			IPADBG_LOW("mapping 0x%lx to 0x%pa size %d\n",
				iova_p, &pa_p, size_p);
			ipa3_iommu_map(cb->iommu_domain,
				iova_p, pa_p, size_p,
				IOMMU_READ | IOMMU_WRITE | IOMMU_MMIO);
		}
	}

	ret = of_property_read_u32(dev->of_node, "qcom,ipa-q6-smem-size",
					&ipa_smem_size);
	if (ret) {
		IPADBG("ipa q6 smem size (default) = %u\n", IPA_SMEM_SIZE);
		ipa_smem_size = IPA_SMEM_SIZE;
	} else {
		IPADBG("ipa q6 smem size = %u\n", ipa_smem_size);
	}

	if (ipa3_ctx->platform_type != IPA_PLAT_TYPE_APQ) {
		/* map SMEM memory for IPA table accesses */
		ret = qcom_smem_alloc(SMEM_MODEM,
			SMEM_IPA_FILTER_TABLE,
			ipa_smem_size);

		if (ret < 0 && ret != -EEXIST) {
			IPAERR("unable to allocate smem MODEM entry\n");
			cb->valid = false;
			if(add_map)
				ipa3_ap_iommu_unmap(cb, add_map, add_map_size);
			return -EFAULT;
		}
		smem_addr = qcom_smem_get(SMEM_MODEM,
			SMEM_IPA_FILTER_TABLE,
			&smem_size);
		if (IS_ERR(smem_addr)) {
			IPAERR("unable to acquire smem MODEM entry\n");
			cb->valid = false;
			if(add_map)
				ipa3_ap_iommu_unmap(cb, add_map, add_map_size);
			return -EFAULT;
		}
		if (smem_size != ipa_smem_size)
			IPAERR("unexpected read q6 smem size %zu %u\n",
				smem_size, ipa_smem_size);

		iova = qcom_smem_virt_to_phys(smem_addr);
		pa = iova;

		IPA_SMMU_ROUND_TO_PAGE(iova, pa, ipa_smem_size,
				iova_p, pa_p, size_p);
		IPADBG("mapping 0x%lx to 0x%pa size %d\n",
				iova_p, &pa_p, size_p);
		ipa3_iommu_map(cb->iommu_domain,
				iova_p, pa_p, size_p,
				IOMMU_READ | IOMMU_WRITE);

		ipa3_ctx->per_stats_smem_pa = iova;
		ipa3_ctx->per_stats_smem_va = smem_addr;

		/**
		 * Force type casting to perpheral stats structure.
		 * First 2kB of the FILTER_TABLE SMEM is allocated for
		 * Peripheral stats design. If there is a need to
		 * use rest of FILTER_TABLE_SMEM it should be used from
		 * smem_addr + 2KB offset
		 */
		ret = ipa3_peripheral_stats_init((union ipa_peripheral_stats *) smem_addr);
		if(ret)	IPAERR("IPA Peripheral stats init failure = %d ", ret);
	}

	smmu_info.present[IPA_SMMU_CB_AP] = true;

	cb->done = true;
	ipa3_ctx->pdev = dev;
	cb->next_addr = cb->va_end;

	return 0;
}

static int ipa_smmu_11ad_cb_probe(struct device *dev)
{
	int bypass = 0;
	struct ipa_smmu_cb_ctx *cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_11AD);
	u32 iova_ap_mapping[2];

	IPADBG("11AD CB probe: dev=%pK\n", dev);

	if (!smmu_info.present[IPA_SMMU_CB_11AD]) {
		IPAERR("11AD SMMU is disabled");
		return 0;
	}

	cb->iommu_domain = iommu_get_domain_for_dev(dev);
	if (IS_ERR_OR_NULL(cb->iommu_domain)) {
		IPAERR("could not get iommu domain\n");
		return -EINVAL;
	}
	cb->is_cache_coherent = of_property_read_bool(dev->of_node,
							"dma-coherent");
	cb->dev   = dev;
	cb->valid = true;

	cb->va_start = cb->va_end  = cb->va_size = 0;
	if (of_property_read_u32_array(
			dev->of_node, "qcom,iommu-dma-addr-pool",
			iova_ap_mapping, 2) == 0) {
		cb->va_start = iova_ap_mapping[0];
		cb->va_size  = iova_ap_mapping[1];
		cb->va_end   = cb->va_start + cb->va_size;
	}

	IPADBG("11AD CB PROBE dev=%pK va_start=0x%x va_size=0x%x\n",
		   dev, cb->va_start, cb->va_size);

	iommu_domain_get_attr(cb->iommu_domain, DOMAIN_ATTR_S1_BYPASS, &bypass);

	IPADBG("11AD CB PROBE dev=%pK DOMAIN ATTRS bypass=%d\n",
		   dev, bypass);

	ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_11AD] = (bypass != 0);

	if (of_property_read_bool(dev->of_node, "qcom,shared-cb")) {
		IPADBG("11AD using shared CB\n");
		cb->shared = true;
	}
	cb->done = true;
	return 0;
}

static int ipa_smmu_cb_probe(struct device *dev, enum ipa_smmu_cb_type cb_type)
{
	struct ipa_smmu_cb_ctx *cb = ipa3_get_smmu_ctx(cb_type);

	if((cb != NULL) && (cb->done == true)) {
		IPADBG("SMMU CB type %d already initialized\n", cb_type);
		return 0;
	}
	switch (cb_type) {
	case IPA_SMMU_CB_AP:
		ipa3_ctx->pdev = &ipa3_ctx->master_pdev->dev;
		return ipa_smmu_ap_cb_probe(dev);
	case IPA_SMMU_CB_WLAN:
	case IPA_SMMU_CB_WLAN1:
	case IPA_SMMU_CB_ETH:
	case IPA_SMMU_CB_ETH1:
		return ipa_smmu_perph_cb_probe(dev, cb_type);
	case IPA_SMMU_CB_UC:
		ipa3_ctx->uc_pdev = &ipa3_ctx->master_pdev->dev;
		return ipa_smmu_uc_cb_probe(dev);
	case IPA_SMMU_CB_11AD:
		return ipa_smmu_11ad_cb_probe(dev);
	case IPA_SMMU_CB_MAX:
		IPAERR("Invalid cb_type\n");
	}
	return 0;
}

static int ipa3_attach_to_smmu(void)
{
	struct ipa_smmu_cb_ctx *cb;
	int i, result;

	if (smmu_info.arm_smmu) {
		IPADBG("smmu is enabled\n");
		for (i = 0; i < IPA_SMMU_CB_MAX; i++) {
			cb = ipa3_get_smmu_ctx(i);
			result = ipa_smmu_cb_probe(cb->dev, i);
			if (result) {
				IPAERR("probe failed for cb %d\n", i);
				return result;
			}
		}
	} else {
		ipa3_ctx->pdev = &ipa3_ctx->master_pdev->dev;
		ipa3_ctx->uc_pdev = &ipa3_ctx->master_pdev->dev;
		IPADBG("smmu is disabled\n");
	}
	return 0;
}

static irqreturn_t ipa3_smp2p_modem_clk_query_isr(int irq, void *ctxt)
{
	ipa3_freeze_clock_vote_and_notify_modem();

	return IRQ_HANDLED;
}

static int ipa3_smp2p_probe(struct device *dev)
{
	struct device_node *node = dev->of_node;
	int res;
	int irq = 0;

	if (ipa3_ctx == NULL) {
		IPAERR("ipa3_ctx was not initialized\n");
		return -EPROBE_DEFER;
	}
	IPADBG("node->name=%s\n", node->name);
	if (ipa3_ctx->platform_type == IPA_PLAT_TYPE_APQ) {
		IPADBG("Ignore smp2p on APQ platform\n");
		return 0;
	}

	if (strcmp("qcom,smp2p_map_ipa_1_out", node->name) == 0) {
		if (of_find_property(node, "qcom,smem-states", NULL)) {
			ipa3_ctx->smp2p_info.smem_state =
			qcom_smem_state_get(dev, "ipa-smp2p-out",
			&ipa3_ctx->smp2p_info.smem_bit);
			if (IS_ERR(ipa3_ctx->smp2p_info.smem_state)) {
				IPAERR("fail to get smp2p clk resp bit %ld\n",
				PTR_ERR(ipa3_ctx->smp2p_info.smem_state));
				return PTR_ERR(ipa3_ctx->smp2p_info.smem_state);
			}
			IPADBG("smem_bit=%d\n", ipa3_ctx->smp2p_info.smem_bit);
		}
	} else if (strcmp("qcom,smp2p_map_ipa_1_in", node->name) == 0) {
		res = irq = of_irq_get_byname(node, "ipa-smp2p-in");
		if (res < 0) {
			IPADBG("of_irq_get_byname returned %d\n", irq);
			return res;
		}

		ipa3_ctx->smp2p_info.in_base_id = irq;
		IPADBG("smp2p irq#=%d\n", irq);
		res = devm_request_threaded_irq(dev, irq, NULL,
			(irq_handler_t)ipa3_smp2p_modem_clk_query_isr,
			IRQF_TRIGGER_RISING | IRQF_ONESHOT,
			"ipa_smp2p_clk_vote", dev);
		if (res) {
			IPAERR("fail to register smp2p irq=%d\n", irq);
			return -ENODEV;
		}
	}
	return 0;
}

static int ipa_smmu_update_fw_loader(void)
{
	int i, result;
	int cnt = 0;

	if (smmu_info.arm_smmu) {
		IPADBG("smmu is enabled\n");
		for (i = 0; i < IPA_SMMU_CB_MAX; i++) {
			if (!smmu_info.present[i]) {
				IPADBG("CB %d not probed yet\n", i);
			} else {
				cnt++;
				IPADBG("CB %d probed\n", i);
			}
		}
		if (cnt == IPA_SMMU_CB_MAX ||
			ipa3_ctx->num_smmu_cb_probed ==
			ipa3_ctx->max_num_smmu_cb) {
			IPADBG("All %d CBs probed\n", IPA_SMMU_CB_MAX);

			if (ipa3_ctx->use_xbl_boot) {
				IPAERR("Using XBL boot load for IPA FW\n");
				mutex_lock(&ipa3_ctx->fw_load_data.lock);
				ipa3_ctx->fw_load_data.state = IPA_FW_LOAD_STATE_LOADED;
				mutex_unlock(&ipa3_ctx->fw_load_data.lock);

				result = ipa3_attach_to_smmu();
				if (result) {
					IPAERR("IPA attach to smmu failed %d\n",
						result);
					return result;
				}

				result = ipa3_post_init(&ipa3_res, ipa3_ctx->cdev.dev);
				if (result) {
					IPAERR("IPA post init failed %d\n", result);
					return result;
				}
			} else {

				ipa_fw_load_sm_handle_event(IPA_FW_LOAD_EVNT_SMMU_DONE);
			}
		}
	} else {
		IPADBG("smmu is disabled\n");
	}

	return 0;
}

int ipa3_plat_drv_probe(struct platform_device *pdev_p)
{
	int result;
	struct device *dev = &pdev_p->dev;
	struct ipa_smmu_cb_ctx *cb;

	/*
	 * IPA probe function can be called for multiple times as the same probe
	 * function handles multiple compatibilities
	 */
	pr_debug("ipa: IPA driver probing started for %s\n",
		pdev_p->dev.of_node->name);

	if (ipa3_ctx == NULL) {
		IPAERR("ipa3_ctx was not initialized\n");
		return -EPROBE_DEFER;
	}

	if (ipa3_ctx->ipa_hw_type == 0) {

		/* Get IPA HW Version */
		result = of_property_read_u32(pdev_p->dev.of_node,
			"qcom,ipa-hw-ver", &ipa3_ctx->ipa_hw_type);
		if ((result) || (ipa3_ctx->ipa_hw_type == IPA_HW_None)) {
			pr_err("ipa: get resource failed for ipa-hw-ver!\n");
			return -ENODEV;
		}
		pr_debug("ipa: ipa_hw_type = %d\n", ipa3_ctx->ipa_hw_type);
	}

	if (ipa3_ctx->ipa_hw_type < IPA_HW_v3_0) {
		pr_err(":IPA version below 3.0 not supported\n");
		return -ENODEV;
	}

	if (ipa3_ctx->ipa_hw_type >= IPA_HW_MAX) {
		pr_err(":IPA version is greater than the MAX\n");
		return -ENODEV;
	}

	IPADBG("IPA driver probing started\n");
	IPADBG("dev->of_node->name = %s\n", dev->of_node->name);

	if (of_device_is_compatible(dev->of_node, "qcom,ipa-smmu-ap-cb")) {
		if (ipa3_ctx == NULL) {
			IPAERR("ipa3_ctx was not initialized\n");
			return -EPROBE_DEFER;
		}
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_AP);
		cb->dev = dev;
		smmu_info.present[IPA_SMMU_CB_AP] = true;
		ipa3_ctx->num_smmu_cb_probed++;
		return ipa_smmu_update_fw_loader();
	}

	if (of_device_is_compatible(dev->of_node, "qcom,ipa-smmu-wlan-cb")) {
		if (ipa3_ctx == NULL) {
			IPAERR("ipa3_ctx was not initialized\n");
			return -EPROBE_DEFER;
		}
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_WLAN);
		cb->dev = dev;
		smmu_info.present[IPA_SMMU_CB_WLAN] = true;
		ipa3_ctx->num_smmu_cb_probed++;
		return ipa_smmu_update_fw_loader();
	}

	if (of_device_is_compatible(dev->of_node, "qcom,ipa-smmu-wlan1-cb")) {
		if (ipa3_ctx == NULL) {
			IPAERR("ipa3_ctx was not initialized\n");
			return -EPROBE_DEFER;
		}
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_WLAN1);
		cb->dev = dev;
		smmu_info.present[IPA_SMMU_CB_WLAN1] = true;
		ipa3_ctx->num_smmu_cb_probed++;
		return ipa_smmu_update_fw_loader();
	}

	if (of_device_is_compatible(dev->of_node, "qcom,ipa-smmu-eth-cb")) {
		if (ipa3_ctx == NULL) {
			IPAERR("ipa3_ctx was not initialized\n");
			return -EPROBE_DEFER;
		}
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_ETH);
		cb->dev = dev;
		smmu_info.present[IPA_SMMU_CB_ETH] = true;
		ipa3_ctx->num_smmu_cb_probed++;
		return ipa_smmu_update_fw_loader();
	}

	if (of_device_is_compatible(dev->of_node, "qcom,ipa-smmu-eth1-cb")) {
		if (ipa3_ctx == NULL) {
			IPAERR("ipa3_ctx was not initialized\n");
			return -EPROBE_DEFER;
		}
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_ETH1);
		cb->dev = dev;
		smmu_info.present[IPA_SMMU_CB_ETH1] = true;
		ipa3_ctx->num_smmu_cb_probed++;
		return ipa_smmu_update_fw_loader();
	}

	if (of_device_is_compatible(dev->of_node, "qcom,ipa-smmu-uc-cb")) {
		if (ipa3_ctx == NULL) {
			IPAERR("ipa3_ctx was not initialized\n");
			return -EPROBE_DEFER;
		}
		cb =  ipa3_get_smmu_ctx(IPA_SMMU_CB_UC);
		cb->dev = dev;
		smmu_info.present[IPA_SMMU_CB_UC] = true;
		ipa3_ctx->num_smmu_cb_probed++;
		return ipa_smmu_update_fw_loader();
	}

	if (of_device_is_compatible(dev->of_node, "qcom,ipa-smmu-11ad-cb")) {
		if (ipa3_ctx == NULL) {
			IPAERR("ipa3_ctx was not initialized\n");
			return -EPROBE_DEFER;
		}
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_11AD);
		cb->dev = dev;
		smmu_info.present[IPA_SMMU_CB_11AD] = true;
		ipa3_ctx->num_smmu_cb_probed++;
		return ipa_smmu_update_fw_loader();
	}

	if (of_device_is_compatible(dev->of_node,
	    "qcom,smp2p-map-ipa-1-out"))
		return ipa3_smp2p_probe(dev);
	if (of_device_is_compatible(dev->of_node,
	    "qcom,smp2p-map-ipa-1-in"))
		return ipa3_smp2p_probe(dev);

	result = get_ipa_dts_configuration(pdev_p, &ipa3_res);
	if (result) {
		IPAERR("IPA dts parsing failed\n");
		return result;
	}

	/*
	* Since we now know where the transport's registers live,
	* let's set up access to them.  This is done since subsequent
	* functions, that deal with the transport, require the
	* access.
	*/
	if (gsi_map_base(
		ipa3_res.transport_mem_base,
		ipa3_res.transport_mem_size,
		ipa3_get_gsi_ver(ipa3_res.ipa_hw_type)) != 0) {
		IPAERR("Allocation of gsi base failed\n");
		return -EFAULT;
	}

	/* Get GSI version */
	ipa3_ctx->gsi_ver = ipa3_get_gsi_ver(ipa3_res.ipa_hw_type);

	if (of_property_read_bool(pdev_p->dev.of_node, "qcom,arm-smmu")) {
		if (of_property_read_bool(pdev_p->dev.of_node,
			"qcom,use-64-bit-dma-mask")) {
			smmu_info.use_64_bit_dma_mask = true;
			if (dma_set_mask_and_coherent(&pdev_p->dev, DMA_BIT_MASK(64))) {
				IPAERR("DMA set 64bit mask failed\n");
				return -EOPNOTSUPP;
			}
		}
		smmu_info.arm_smmu = true;
	} else {
		if (of_property_read_bool(pdev_p->dev.of_node,
			"qcom,use-64-bit-dma-mask")) {
			if (dma_set_mask_and_coherent(&pdev_p->dev, DMA_BIT_MASK(64))) {
				IPAERR("DMA set 64bit mask failed\n");
				return -EOPNOTSUPP;
			}
		} else {
			if (dma_set_mask_and_coherent(&pdev_p->dev, DMA_BIT_MASK(32))) {
				IPAERR("DMA set 32bit mask failed\n");
				return -EOPNOTSUPP;
			}
		}
		/* Below update of pre init for non smmu device, As
		 * existing flow initialzies only for smmu
		 * enabled node.*/

		result = ipa3_pre_init(&ipa3_res, pdev_p);
		if (result) {
			IPAERR("ipa3_init failed\n");
			goto err_check;
		}
		ipa_fw_load_sm_handle_event(IPA_FW_LOAD_EVNT_SMMU_DONE);
		goto skip_repeat_pre_init;
	}

	/* Proceed to real initialization */
	result = ipa3_pre_init(&ipa3_res, pdev_p);
	if (result) {
		IPAERR("ipa3_init failed\n");
		goto err_check;
	}

skip_repeat_pre_init:
	result = of_platform_populate(pdev_p->dev.of_node,
		ipa_plat_drv_match, NULL, &pdev_p->dev);
	if (result) {
		IPAERR("failed to populate platform\n");
		return result;
	}

err_check:
	if (result) {
		if (result != -EPROBE_DEFER) {
			IPAERR("ipa: ipa_plat_drv_probe failed\n");
		} else {
			gsi_unmap_base();
		}
	}

	return result;
}

/**
 * ipa3_ap_suspend() - suspend callback for runtime_pm
 * @dev: pointer to device
 *
 * This callback will be invoked by the runtime_pm framework when an AP suspend
 * operation is invoked, usually by pressing a suspend button.
 *
 * Returns -EAGAIN to runtime_pm framework in case IPA is in use by AP.
 * This will postpone the suspend operation until IPA is no longer used by AP.
 */
int ipa3_ap_suspend(struct device *dev)
{
	int i;

	IPADBG("Enter...\n");
	if (!of_device_is_compatible(dev->of_node,"qcom,ipa"))
		return 0;
	/* In case there is a tx/rx handler in polling mode fail to suspend */
	for (i = 0; i < ipa3_ctx->ipa_num_pipes; i++) {
		if (ipa3_ctx->ep[i].sys &&
			atomic_read(&ipa3_ctx->ep[i].sys->curr_polling_state)) {
			IPAERR("EP %d is in polling state, do not suspend\n",
				i);
			return -EAGAIN;
		}
	}

#ifdef CONFIG_DEEPSLEEP
	if (mem_sleep_current == PM_SUSPEND_MEM) {
		IPADBG("Enter deepsleep suspend\n");
		ipa3_deepsleep_suspend();
		IPADBG("Exit deepsleep suspend\n");
	}
#endif
	ipa_pm_deactivate_all_deferred();

	IPADBG("Exit\n");

	return 0;
}

/**
 * ipa3_ap_resume() - resume callback for runtime_pm
 * @dev: pointer to device
 *
 * This callback will be invoked by the runtime_pm framework when an AP resume
 * operation is invoked.
 *
 * Always returns 0 since resume should always succeed.
 */
int ipa3_ap_resume(struct device *dev)
{
	return 0;
}

struct ipa3_context *ipa3_get_ctx(void)
{
	return ipa3_ctx;
}
EXPORT_SYMBOL(ipa3_get_ctx);

bool ipa3_get_lan_rx_napi(void)
{
	return ipa3_ctx->lan_rx_napi_enable;
}


#ifdef CONFIG_DEEPSLEEP
static void ipa3_deepsleep_suspend(void)
{
	IPADBG("Entry\n");
	IPA_ACTIVE_CLIENTS_INC_SIMPLE();

	/* To allow default routing table delection using this flag */
	ipa3_ctx->deepsleep = true;
	/*Disabling the LAN NAPI*/
	ipa3_disable_napi_lan_rx();
	/*NOt allow uC related operations until uC load again*/
	ipa3_ctx->uc_ctx.uc_loaded = false;
	/*Disconnecting LAN PROD/LAN CONS/CMD PROD apps pipes*/
	ipa3_teardown_apps_pipes();
	/*Deregistering the GSI driver*/
	gsi_deregister_device(ipa3_ctx->gsi_dev_hdl, false);
	/*Destroying filter table ids*/
	ipa3_destroy_flt_tbl_idrs();
	/*Disabling IPA interrupt*/
	ipa3_remove_interrupt_handler(IPA_TX_SUSPEND_IRQ);
	ipa3_interrupts_destroy(ipa3_res.ipa_irq, &ipa3_ctx->master_pdev->dev);
	ipa3_uc_interface_destroy();
	/*Destroy the NAT device*/
	ipa3_nat_ipv6ct_destroy_devices();
	/*Freeing memory allocated for coalesing and dma task*/
	ipa3_free_coal_close_frame();
	ipa3_free_dma_task_for_gsi();
	/*Destroying ipa hal module*/
	ipahal_destroy();
	ipa3_ctx->ipa_initialization_complete = false;
	ipa3_debugfs_remove();
	/*Unloading IPA FW to allow FW load in resume*/
	ipa3_pil_unload_ipa_fws();
	/*Calling framework API to reset IPA ready flag to false*/
	ipa_fmwk_deepsleep_entry_ipa();
	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
	IPADBG("Exit\n");
}

static void ipa3_deepsleep_resume(void)
{

	IPADBG("Entry\n");
	/*After deeplseep exit we shouldn't allow delete the default routing table*/
	ipa3_ctx->deepsleep = false;
	/*Scheduling WQ to load IPA FW*/
	queue_work(ipa3_ctx->transport_power_mgmt_wq,
		&ipa3_fw_loading_work);
	IPADBG("Exit\n");
}
#endif

static void ipa_gsi_notify_cb(struct gsi_per_notify *notify)
{
	/*
	 * These values are reported by hardware. Any error indicates
	 * hardware unexpected state.
	 */
	switch (notify->evt_id) {
	case GSI_PER_EVT_GLOB_ERROR:
		IPAERR("Got GSI_PER_EVT_GLOB_ERROR\n");
		IPAERR("Err_desc = 0x%04x\n", notify->data.err_desc);
		break;
	case GSI_PER_EVT_GLOB_GP1:
		IPAERR("Got GSI_PER_EVT_GLOB_GP1\n");
		ipa_assert();
		break;
	case GSI_PER_EVT_GLOB_GP2:
		IPAERR("Got GSI_PER_EVT_GLOB_GP2\n");
		ipa_assert();
		break;
	case GSI_PER_EVT_GLOB_GP3:
		IPAERR("Got GSI_PER_EVT_GLOB_GP3\n");
		ipa_assert();
		break;
	case GSI_PER_EVT_GENERAL_BREAK_POINT:
		IPAERR("Got GSI_PER_EVT_GENERAL_BREAK_POINT\n");
		break;
	case GSI_PER_EVT_GENERAL_BUS_ERROR:
		IPAERR("Got GSI_PER_EVT_GENERAL_BUS_ERROR\n");
		ipa_assert();
		break;
	case GSI_PER_EVT_GENERAL_CMD_FIFO_OVERFLOW:
		IPAERR("Got GSI_PER_EVT_GENERAL_CMD_FIFO_OVERFLOW\n");
		ipa_assert();
		break;
	case GSI_PER_EVT_GENERAL_MCS_STACK_OVERFLOW:
		IPAERR("Got GSI_PER_EVT_GENERAL_MCS_STACK_OVERFLOW\n");
		ipa_assert();
		break;
	default:
		IPAERR("Received unexpected evt: %d\n",
			notify->evt_id);
		ipa_assert();
	}
}

int ipa3_iommu_map(struct iommu_domain *domain,
	unsigned long iova, phys_addr_t paddr, size_t size, int prot)
{
	struct ipa_smmu_cb_ctx *cb = NULL;

	IPADBG_LOW("domain =0x%pK iova 0x%lx\n", domain, iova);
	IPADBG_LOW("paddr =0x%pa size 0x%x\n", &paddr, (u32)size);

	/* make sure no overlapping */
	if (domain == ipa3_get_smmu_domain()) {
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_AP);
		if (iova >= cb->va_start && iova < cb->va_end) {
			IPAERR("iommu AP overlap addr 0x%lx\n", iova);
			ipa_assert();
			return -EFAULT;
		}
	} else if (domain == ipa3_get_wlan_smmu_domain()) {
		/* wlan is one time map */
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_WLAN);
	} else if (domain == ipa3_get_wlan1_smmu_domain()) {
		/* wlan1 is one time map */
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_WLAN1);
	} else if (domain == ipa3_get_eth_smmu_domain()) {
		/* eth is one time map */
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_ETH);
	} else if (domain == ipa3_get_eth1_smmu_domain()) {
		/* eth1 is one time map */
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_ETH1);
	} else if (domain == ipa3_get_11ad_smmu_domain()) {
		/* 11ad is one time map */
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_11AD);
	} else if (domain == ipa3_get_uc_smmu_domain()) {
		cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_UC);
		if (iova >= cb->va_start && iova < cb->va_end) {
			IPAERR("iommu uC overlap addr 0x%lx\n", iova);
			ipa_assert();
			return -EFAULT;
		}
	} else {
		IPAERR("Unexpected domain 0x%pK\n", domain);
		ipa_assert();
		return -EFAULT;
	}

	if (cb == NULL) {
		IPAERR("Unexpected cb turning NULL for domain 0x%pK\n", domain);
		ipa_assert();
	}

	/*
	 * IOMMU_CACHE is needed to make the entries cachable
	 * if cache coherency is enabled in dtsi.
	 */
	if (cb->is_cache_coherent)
		prot |= IOMMU_CACHE;

	return iommu_map(domain, iova, paddr, size, prot);
}
EXPORT_SYMBOL(ipa3_iommu_map);

/**
 * ipa3_get_smmu_params()- Return the ipa3 smmu related params.
 */
int ipa3_get_smmu_params(struct ipa_smmu_in_params *in,
	struct ipa_smmu_out_params *out)
{
	bool is_smmu_enable = false;

	if (out == NULL || in == NULL) {
		IPAERR("bad parms for Client SMMU out params\n");
		return -EINVAL;
	}

	if (!ipa3_ctx) {
		IPAERR("IPA not yet initialized\n");
		return -EINVAL;
	}

	out->shared_cb = false;

	switch (in->smmu_client) {
	case IPA_SMMU_WLAN_CLIENT:
		if (ipa_get_wdi_version() == IPA_WDI_3 ||
			IPA_WDI2_OVER_GSI())
			is_smmu_enable =
				!(ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_AP] ||
				ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_WLAN]);
		else
			is_smmu_enable =
			!(ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_UC] ||
			ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_WLAN]);
		break;
	case IPA_SMMU_WLAN1_CLIENT:
		is_smmu_enable =
			!(ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_AP] ||
			ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_WLAN1]);
		break;
	case IPA_SMMU_ETH_CLIENT:
		is_smmu_enable =
			!(ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_AP] ||
			ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_ETH]);
		break;
	case IPA_SMMU_ETH1_CLIENT:
		is_smmu_enable =
			!(ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_AP] ||
			ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_ETH1]);
		break;
	case IPA_SMMU_WIGIG_CLIENT:
		is_smmu_enable = !(ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_UC] ||
			ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_11AD] ||
			ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_AP]);
		if (is_smmu_enable) {
			if (ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_UC] ||
				ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_11AD] ||
				ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_AP]) {
				IPAERR("11AD SMMU Discrepancy (%d %d %d)\n",
				ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_UC],
				ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_AP],
				ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_11AD]);
				WARN_ON(1);
				return -EINVAL;
			}
		} else {
			if (!ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_UC] ||
				!ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_11AD] ||
				!ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_AP]) {
				IPAERR("11AD SMMU Discrepancy (%d %d %d)\n",
				ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_UC],
				ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_AP],
				ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_11AD]);
				WARN_ON(1);
				return -EINVAL;
			}
		}
		out->shared_cb = (ipa3_get_smmu_ctx(IPA_SMMU_CB_11AD))->shared;
		break;
	case IPA_SMMU_AP_CLIENT:
		is_smmu_enable =
			!(ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_AP]);
		break;
	default:
		is_smmu_enable = false;
		IPAERR("Trying to get illegal clients SMMU status");
		return -EINVAL;
	}

	out->smmu_enable = is_smmu_enable;

	return 0;
}
EXPORT_SYMBOL(ipa3_get_smmu_params);

#define MAX_LEN 96

void ipa_pc_qmp_enable(void)
{
	char buf[MAX_LEN] = "{class: bcm, res: ipa_pc, val: 1}";
	struct qmp_pkt pkt;
	int ret = 0;
	struct ipa3_pc_mbox_data *mbox_data = &ipa3_ctx->pc_mbox;

	IPADBG("Enter\n");

	/* prepare the mailbox struct */
	mbox_data->mbox_client.dev = &ipa3_ctx->master_pdev->dev;
	mbox_data->mbox_client.tx_block = true;
	mbox_data->mbox_client.tx_tout = MBOX_TOUT_MS;
	mbox_data->mbox_client.knows_txdone = false;

	mbox_data->mbox = mbox_request_channel(&mbox_data->mbox_client, 0);
	if (IS_ERR(mbox_data->mbox)) {
		ret = PTR_ERR(mbox_data->mbox);
		if (ret != -EPROBE_DEFER)
			IPAERR("mailbox channel request failed, ret=%d\n", ret);

		return;
	}

	/* prepare the QMP packet to send */
	pkt.size = MAX_LEN;
	pkt.data = buf;

	/* send the QMP packet to AOP */
	ret = mbox_send_message(mbox_data->mbox, &pkt);
	if (ret < 0)
		IPAERR("qmp message send failed, ret=%d\n", ret);

	if (mbox_data->mbox) {
		mbox_free_channel(mbox_data->mbox);
		mbox_data->mbox = NULL;
	}
}

/**************************************************************
 *            PCIe Version
 *************************************************************/

int ipa3_pci_drv_probe(struct pci_dev *pci_dev, const struct pci_device_id *ent)
{
	int result;
	struct ipa3_plat_drv_res *ipa_drv_res;
	u32 bar0_offset;
	u32 mem_start;
	u32 mem_end;
	uint32_t bits;
	uint32_t ipa_start, gsi_start, intctrl_start;
	struct device *dev;
	static struct platform_device platform_dev;

	if (!pci_dev || !ent) {
		pr_err(
			"Bad arg: pci_dev (%pK) and/or ent (%pK)\n",
			pci_dev, ent);
		return -EOPNOTSUPP;
	}

	if (ipa3_ctx == NULL) {
		IPAERR("ipa3_ctx was not initialized\n");
		return -EPROBE_DEFER;
	}

	if (ipa3_ctx->ipa_hw_type == 0) {
		/* Get IPA HW Version */
		result = of_property_read_u32(NULL,
			"qcom,ipa-hw-ver", &ipa3_ctx->ipa_hw_type);
		if (result || ipa3_ctx->ipa_hw_type == IPA_HW_None) {
			pr_err("ipa: get resource failed for ipa-hw-ver!\n");
			return -ENODEV;
		}
		pr_debug("ipa: ipa_hw_type = %d\n", ipa3_ctx->ipa_hw_type);
	}

	dev = &(pci_dev->dev);

	IPADBG("IPA PCI driver probing started\n");

	/*
	 * Follow PCI driver flow here.
	 * pci_enable_device:  Enables device and assigns resources
	 * pci_request_region:  Makes BAR0 address region usable
	 */
	result = pci_enable_device(pci_dev);
	if (result < 0) {
		IPAERR("pci_enable_device() failed\n");
		return -EOPNOTSUPP;
	}

	result = pci_request_region(pci_dev, 0, "IPA Memory");
	if (result < 0) {
		IPAERR("pci_request_region() failed\n");
		pci_disable_device(pci_dev);
		return -EOPNOTSUPP;
	}

	/*
	 * When in the PCI/emulation environment, &platform_dev is
	 * passed to get_ipa_dts_configuration(), but is unused, since
	 * all usages of it in the function are replaced by CPP
	 * relative to definitions in ipa_emulation_stubs.h.  Passing
	 * &platform_dev makes code validity tools happy.
	 */
	if (get_ipa_dts_configuration(&platform_dev, &ipa3_res) != 0) {
		IPAERR("get_ipa_dts_configuration() failed\n");
		pci_release_region(pci_dev, 0);
		pci_disable_device(pci_dev);
		return -EOPNOTSUPP;
	}

	ipa_drv_res = &ipa3_res;

	result =
		of_property_read_u32(NULL, "emulator-bar0-offset",
			&bar0_offset);
	if (result) {
		IPAERR(":get resource failed for emulator-bar0-offset!\n");
		pci_release_region(pci_dev, 0);
		pci_disable_device(pci_dev);
		return -ENODEV;
	}
	IPADBG(":using emulator-bar0-offset 0x%08X\n", bar0_offset);

	ipa_start = ipa_drv_res->ipa_mem_base;
	gsi_start = ipa_drv_res->transport_mem_base;
	intctrl_start = ipa_drv_res->emulator_intcntrlr_mem_base;

	/*
	 * Where will we be inerrupted at?
	 */
	ipa_drv_res->emulator_irq = pci_dev->irq;
	IPADBG(
	    "EMULATION PCI_INTERRUPT_PIN(%u)\n",
	    ipa_drv_res->emulator_irq);

	/*
	 * Set the ipa_mem_base to the PCI base address of BAR0
	 */
	mem_start = pci_resource_start(pci_dev, 0);
	mem_end   = pci_resource_end(pci_dev, 0);

	IPADBG("PCI START = 0x%x\n", mem_start);
	IPADBG("PCI END = 0x%x\n", mem_end);

	ipa_drv_res->ipa_mem_base = mem_start + bar0_offset;

	smmu_info.ipa_base = ipa_drv_res->ipa_mem_base;
	smmu_info.ipa_size = ipa_drv_res->ipa_mem_size;

	ipa_drv_res->transport_mem_base =
	    ipa_drv_res->ipa_mem_base + (gsi_start - ipa_start);

	ipa_drv_res->emulator_intcntrlr_mem_base =
	    ipa_drv_res->ipa_mem_base + (intctrl_start - ipa_start);

	IPADBG("ipa_mem_base = 0x%x\n",
		ipa_drv_res->ipa_mem_base);
	IPADBG("ipa_mem_size = 0x%x\n",
		ipa_drv_res->ipa_mem_size);

	IPADBG("transport_mem_base = 0x%x\n",
		ipa_drv_res->transport_mem_base);
	IPADBG("transport_mem_size = 0x%x\n",
		ipa_drv_res->transport_mem_size);

	IPADBG("emulator_intcntrlr_mem_base = 0x%x\n",
		ipa_drv_res->emulator_intcntrlr_mem_base);
	IPADBG("emulator_intcntrlr_mem_size = 0x%x\n",
		ipa_drv_res->emulator_intcntrlr_mem_size);

	bits = (ipa_drv_res->use_64_bit_dma_mask) ? 64 : 32;

	if (dma_set_mask(dev, DMA_BIT_MASK(bits)) != 0) {
		IPAERR("dma_set_mask(%pK, %u) failed\n", dev, bits);
		pci_release_region(pci_dev, 0);
		pci_disable_device(pci_dev);
		return -EOPNOTSUPP;
	}

	if (dma_set_coherent_mask(dev, DMA_BIT_MASK(bits)) != 0) {
		IPAERR("dma_set_coherent_mask(%pK, %u) failed\n", dev, bits);
		pci_release_region(pci_dev, 0);
		pci_disable_device(pci_dev);
		return -EOPNOTSUPP;
	}

	pci_set_master(pci_dev);

	memset(&platform_dev, 0, sizeof(platform_dev));
	platform_dev.dev = *dev;

	/* Proceed to real initialization */
	result = ipa3_pre_init(&ipa3_res, &platform_dev);
	if (result) {
		IPAERR("ipa3_init failed\n");
		pci_clear_master(pci_dev);
		pci_release_region(pci_dev, 0);
		pci_disable_device(pci_dev);
		return result;
	}

	return result;
}

/*
 * The following returns transport register memory location and
 * size...
 */
int ipa3_get_transport_info(
	phys_addr_t *phys_addr_ptr,
	unsigned long *size_ptr)
{
	if (!phys_addr_ptr || !size_ptr) {
		IPAERR("Bad arg: phys_addr_ptr(%pK) and/or size_ptr(%pK)\n",
			phys_addr_ptr, size_ptr);
		return -EINVAL;
	}

	*phys_addr_ptr = ipa3_res.transport_mem_base;
	*size_ptr = ipa3_res.transport_mem_size;

	return 0;
}
EXPORT_SYMBOL(ipa3_get_transport_info);

static uint emulation_type = IPA_HW_v4_0;

/*
 * The following returns emulation type...
 */
uint ipa3_get_emulation_type(void)
{
	return emulation_type;
}

static int __init ipa_module_init(void)
{
	pr_debug("IPA module init\n");

	ipa3_ctx = kzalloc(sizeof(*ipa3_ctx), GFP_KERNEL);
	if (!ipa3_ctx) {
		return -ENOMEM;
	}
	mutex_init(&ipa3_ctx->lock);

	if (running_emulation) {
		/* Register as a PCI device driver */
		return pci_register_driver(&ipa_pci_driver);
	}

	register_pm_notifier(&ipa_pm_notifier);
	/* Register as a platform device driver */
	return platform_driver_register(&ipa_plat_drv);
}
subsys_initcall(ipa_module_init);

static void __exit ipa_module_exit(void)
{
	if (running_emulation)
		pci_unregister_driver(&ipa_pci_driver);
	platform_driver_unregister(&ipa_plat_drv);
	if(ipa3_ctx->hw_stats) {
		kfree(ipa3_ctx->hw_stats);
		ipa3_ctx->hw_stats = NULL;
	}
	unregister_pm_notifier(&ipa_pm_notifier);
	kfree(ipa3_ctx);
	ipa3_ctx = NULL;
}
module_exit(ipa_module_exit);

MODULE_SOFTDEP("pre: subsys-pil-tz");
MODULE_SOFTDEP("pre: qcom-arm-smmu-mod");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("IPA HW device driver");

/*
 * Module parameter. Invoke as follows:
 *     insmod ipat.ko emulation_type=[13|14|17|...|N]
 * Examples:
 *   insmod ipat.ko emulation_type=13 # for IPA 3.5.1
 *   insmod ipat.ko emulation_type=14 # for IPA 4.0
 *   insmod ipat.ko emulation_type=17 # for IPA 4.5
 *
 * NOTE: The emulation_type values need to come from: enum ipa_hw_type
 *
 */

module_param(emulation_type, uint, 0000);
MODULE_PARM_DESC(
	emulation_type,
	"emulation_type=N N can be 13 for IPA 3.5.1, 14 for IPA 4.0, 17 for IPA 4.5");
