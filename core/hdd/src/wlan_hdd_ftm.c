/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: wlan_hdd_ftm.c
 *
 * This file contains the WLAN factory test mode implementation
 */

/* denote that this file does not allow legacy hddLog */
#define HDD_DISALLOW_LEGACY_HDDLOG 1

#include <cds_mq.h>
#include "cds_sched.h"
#include <cds_api.h>
#include "sir_types.h"
#include "qdf_types.h"
#include "sir_api.h"
#include "sir_mac_prot_def.h"
#include "sme_api.h"
#include "mac_init_api.h"
#include "wlan_qct_sys.h"
#include "wlan_hdd_misc.h"
#include "i_cds_packet.h"
#include "cds_reg_service.h"
#include "wlan_hdd_main.h"
#include "qwlan_version.h"
#include "wma_types.h"
#include "cfg_api.h"

#if  defined(QCA_WIFI_FTM)
#include "bmi.h"
#include "ol_fw.h"
#include "wlan_hdd_cfg80211.h"
#include "wlan_hdd_main.h"
#include "hif.h"
#endif

#define HDD_FTM_WMA_PRE_START_TIMEOUT (30000) /* 30 seconds */

#if  defined(QCA_WIFI_FTM)
#if defined(LINUX_QCMBR)
#define ATH_XIOCTL_UNIFIED_UTF_CMD  0x1000
#define ATH_XIOCTL_UNIFIED_UTF_RSP  0x1001
#define MAX_UTF_LENGTH              1024
typedef struct qcmbr_data_s {
	unsigned int cmd;
	unsigned int length;
	unsigned char buf[MAX_UTF_LENGTH + 4];
	unsigned int copy_to_user;
} qcmbr_data_t;
typedef struct qcmbr_queue_s {
	unsigned char utf_buf[MAX_UTF_LENGTH + 4];
	struct list_head list;
} qcmbr_queue_t;
LIST_HEAD(qcmbr_queue_head);
DEFINE_SPINLOCK(qcmbr_queue_lock);
static void wlanqcmbr_mc_process_msg(void *message);
#endif
#endif

/**
 * wlan_ftm_postmsg() - Post FTM message
 * @cmd_ptr: Pointer to FTM command buffer
 * @cmd_len: Length of command in @cmd_ptr
 *
 * This function is used to send FTM commands to firmware
 *
 * Return: 0 for success, non zero for failure
 */
static uint32_t wlan_ftm_postmsg(uint8_t *cmd_ptr, uint16_t cmd_len)
{
	cds_msg_t ftmMsg;

	ENTER();

	ftmMsg.type = WMA_FTM_CMD_REQ;
	ftmMsg.reserved = 0;
	ftmMsg.bodyptr = (uint8_t *) cmd_ptr;
	ftmMsg.bodyval = 0;

	if (QDF_STATUS_SUCCESS != cds_mq_post_message(QDF_MODULE_ID_WMA,
						      &ftmMsg)) {
		hdd_err("Failed to post Msg to HAL");

		return QDF_STATUS_E_FAILURE;
	}

	EXIT();
	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_LPSS
static inline void hdd_is_lpass_supported(struct cds_config_info *cds_cfg,
						hdd_context_t *hdd_ctx)
{
	cds_cfg->is_lpass_enabled = hdd_ctx->config->enable_lpass_support;
}
#else
static inline void hdd_is_lpass_supported(struct cds_config_info *cds_cfg,
						hdd_context_t *hdd_ctx)
{ }
#endif

/**
 * hdd_update_cds_config_ftm() - API to update cds configuration parameters
 * for FTM mode.
 * @hdd_ctx: HDD Context
 *
 * Return: 0 on success; errno on failure
 */

int hdd_update_cds_config_ftm(hdd_context_t *hdd_ctx)
{
	struct cds_config_info *cds_cfg;

	cds_cfg = qdf_mem_malloc(sizeof(struct cds_config_info));
	if (!cds_cfg) {
		hdd_err("failed to allocate cds config");
		return -ENOMEM;
	}
	qdf_mem_zero(cds_cfg, sizeof(struct cds_config_info));

	cds_cfg->driver_type = eDRIVER_TYPE_MFG;
	cds_cfg->powersave_offload_enabled =
			hdd_ctx->config->enablePowersaveOffload;
	hdd_is_lpass_supported(cds_cfg, hdd_ctx);
	/* UMA is supported in hardware for performing the
	 * frame translation 802.11 <-> 802.3
	 */
	cds_cfg->frame_xln_reqd = 1;
	cds_cfg->sub_20_channel_width = WLAN_SUB_20_CH_WIDTH_NONE;
	cds_init_ini_config(cds_cfg);

	return 0;
}

/**
 * hdd_ftm_mc_process_msg() - Process FTM mailbox message
 * @message: FTM response message
 *
 * Process FTM mailbox message
 *
 * Return: void
 */
void hdd_ftm_mc_process_msg(void *message)
{
	void *data;
	uint32_t data_len;

	if (!message) {
		hdd_err("Message is NULL, nothing to process.");
		return;
	}

	data_len = *((uint32_t *) message);
	data = (uint32_t *) message + 1;

#if defined(LINUX_QCMBR)
	wlanqcmbr_mc_process_msg(message);
#else
#ifdef CONFIG_NL80211_TESTMODE
	wlan_hdd_testmode_rx_event(data, (size_t) data_len);
#endif
#endif
	return;
}

#if  defined(QCA_WIFI_FTM)
#if defined(LINUX_QCMBR)
/**
 * wlan_hdd_qcmbr_command() - QCMBR command handler
 * @adapter: adapter upon which the command was received
 * @pqcmbr_data: QCMBR command
 *
 * Return: 0 on success, non-zero on error
 */
static int wlan_hdd_qcmbr_command(hdd_adapter_t *adapter,
				  qcmbr_data_t *pqcmbr_data)
{
	int ret = 0;
	qcmbr_queue_t *qcmbr_buf = NULL;

	switch (pqcmbr_data->cmd) {
	case ATH_XIOCTL_UNIFIED_UTF_CMD: {
		pqcmbr_data->copy_to_user = 0;
		if (pqcmbr_data->length) {
			if (wlan_hdd_ftm_testmode_cmd(pqcmbr_data->buf,
						      pqcmbr_data->
						      length)
			    != QDF_STATUS_SUCCESS) {
				ret = -EBUSY;
			} else {
				ret = 0;
			}
		}
	}
	break;

	case ATH_XIOCTL_UNIFIED_UTF_RSP: {
		pqcmbr_data->copy_to_user = 1;
		if (!list_empty(&qcmbr_queue_head)) {
			spin_lock_bh(&qcmbr_queue_lock);
			qcmbr_buf = list_first_entry(&qcmbr_queue_head,
						     qcmbr_queue_t,
						     list);
			list_del(&qcmbr_buf->list);
			spin_unlock_bh(&qcmbr_queue_lock);
			ret = 0;
		} else {
			ret = -1;
		}

		if (!ret) {
			memcpy(pqcmbr_data->buf, qcmbr_buf->utf_buf,
			       (MAX_UTF_LENGTH + 4));
			kfree(qcmbr_buf);
		} else {
			ret = -EAGAIN;
		}
	}
	break;
	}

	return ret;
}

#ifdef CONFIG_COMPAT
/**
 * wlan_hdd_qcmbr_ioctl() - Compatability-mode QCMBR ioctl handler
 * @adapter: adapter upon which the ioctl was received
 * @ifr: the ioctl request
 *
 * Return: 0 on success, non-zero on error
 */
static int wlan_hdd_qcmbr_compat_ioctl(hdd_adapter_t *adapter,
				       struct ifreq *ifr)
{
	qcmbr_data_t *qcmbr_data;
	int ret = 0;

	qcmbr_data = kzalloc(sizeof(qcmbr_data_t), GFP_KERNEL);
	if (qcmbr_data == NULL)
		return -ENOMEM;

	if (copy_from_user(qcmbr_data, ifr->ifr_data, sizeof(*qcmbr_data))) {
		ret = -EFAULT;
		goto exit;
	}

	ret = wlan_hdd_qcmbr_command(adapter, qcmbr_data);
	if (qcmbr_data->copy_to_user) {
		ret = copy_to_user(ifr->ifr_data, qcmbr_data->buf,
				   (MAX_UTF_LENGTH + 4));
	}

exit:
	kfree(qcmbr_data);
	return ret;
}
#else                           /* CONFIG_COMPAT */
static int wlan_hdd_qcmbr_compat_ioctl(hdd_adapter_t *adapter,
				       struct ifreq *ifr)
{
	return 0;
}
#endif /* CONFIG_COMPAT */

/**
 * wlan_hdd_qcmbr_ioctl() - Standard QCMBR ioctl handler
 * @adapter: adapter upon which the ioctl was received
 * @ifr: the ioctl request
 *
 * Return: 0 on success, non-zero on error
 */
static int wlan_hdd_qcmbr_ioctl(hdd_adapter_t *adapter, struct ifreq *ifr)
{
	qcmbr_data_t *qcmbr_data;
	int ret = 0;

	qcmbr_data = kzalloc(sizeof(qcmbr_data_t), GFP_KERNEL);
	if (qcmbr_data == NULL)
		return -ENOMEM;

	if (copy_from_user(qcmbr_data, ifr->ifr_data, sizeof(*qcmbr_data))) {
		ret = -EFAULT;
		goto exit;
	}

	ret = wlan_hdd_qcmbr_command(adapter, qcmbr_data);
	if (qcmbr_data->copy_to_user) {
		ret = copy_to_user(ifr->ifr_data, qcmbr_data->buf,
				   (MAX_UTF_LENGTH + 4));
	}

exit:
	kfree(qcmbr_data);
	return ret;
}

/**
 * wlan_hdd_qcmbr_unified_ioctl() - Unified QCMBR ioctl handler
 * @adapter: adapter upon which the ioctl was received
 * @ifr: the ioctl request
 *
 * Return: 0 on success, non-zero on error
 */
int wlan_hdd_qcmbr_unified_ioctl(hdd_adapter_t *adapter, struct ifreq *ifr)
{
	int ret = 0;

	if (is_compat_task()) {
		ret = wlan_hdd_qcmbr_compat_ioctl(adapter, ifr);
	} else {
		ret = wlan_hdd_qcmbr_ioctl(adapter, ifr);
	}

	return ret;
}

/**
 * wlanqcmbr_mc_process_msg() - Process QCMBR response message
 * @message: QCMBR message
 *
 * Return: None
 */
static void wlanqcmbr_mc_process_msg(void *message)
{
	qcmbr_queue_t *qcmbr_buf = NULL;
	uint32_t data_len;

	data_len = *((uint32_t *) message) + sizeof(uint32_t);
	qcmbr_buf = kzalloc(sizeof(qcmbr_queue_t), GFP_KERNEL);
	if (qcmbr_buf != NULL) {
		memcpy(qcmbr_buf->utf_buf, message, data_len);
		spin_lock_bh(&qcmbr_queue_lock);
		list_add_tail(&(qcmbr_buf->list), &qcmbr_queue_head);
		spin_unlock_bh(&qcmbr_queue_lock);
	}
}
#endif /*LINUX_QCMBR */

/**
 * wlan_hdd_ftm_testmode_cmd() - Process FTM testmode command
 * @data: FTM testmode command
 * @len: length of @data
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_hdd_ftm_testmode_cmd(void *data, int len)
{
	struct ar6k_testmode_cmd_data *cmd_data;

	cmd_data = (struct ar6k_testmode_cmd_data *)
		   qdf_mem_malloc(sizeof(*cmd_data));

	if (!cmd_data) {
		hdd_err("Failed to allocate FTM command data");
		return QDF_STATUS_E_NOMEM;
	}

	cmd_data->data = qdf_mem_malloc(len);

	if (!cmd_data->data) {
		hdd_err("Failed to allocate FTM command data buffer");
		qdf_mem_free(cmd_data);
		return QDF_STATUS_E_NOMEM;
	}

	cmd_data->len = len;
	qdf_mem_copy(cmd_data->data, data, len);

	if (wlan_ftm_postmsg((uint8_t *) cmd_data, sizeof(*cmd_data))) {
		qdf_mem_free(cmd_data->data);
		qdf_mem_free(cmd_data);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif /*QCA_WIFI_FTM */
