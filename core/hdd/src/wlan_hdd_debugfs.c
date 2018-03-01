/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_debugfs.c
 *
 * This driver currently supports the following debugfs files:
 * wlan_wcnss/wow_enable to enable/disable WoWL.
 * wlan_wcnss/wow_pattern to configure WoWL patterns.
 * wlan_wcnss/pattern_gen to configure periodic TX patterns.
 */

#ifdef WLAN_OPEN_SOURCE
#include <wlan_hdd_includes.h>
#include <wlan_hdd_debugfs.h>
#include <wlan_hdd_request_manager.h>
#include <wlan_hdd_wowl.h>
#include <cds_sched.h>
#include <wlan_hdd_debugfs_llstat.h>

#define MAX_USER_COMMAND_SIZE_WOWL_ENABLE 8
#define MAX_USER_COMMAND_SIZE_WOWL_PATTERN 512
#define MAX_USER_COMMAND_SIZE_FRAME 4096

#ifdef WLAN_POWER_DEBUGFS
#define POWER_DEBUGFS_BUFFER_MAX_LEN 4096
#endif

/**
 * __wcnss_wowpattern_write() - wow_pattern debugfs handler
 * @file: debugfs file handle
 * @buf: text being written to the debugfs
 * @count: size of @buf
 * @ppos: (unused) offset into the virtual file system
 *
 * Return: number of bytes processed
 */
static ssize_t __wcnss_wowpattern_write(struct file *file,
				      const char __user *buf, size_t count,
				      loff_t *ppos)
{
	struct hdd_adapter *adapter = (struct hdd_adapter *) file->private_data;
	struct hdd_context *hdd_ctx;
	char cmd[MAX_USER_COMMAND_SIZE_WOWL_PATTERN + 1];
	char *sptr, *token;
	uint8_t pattern_idx = 0;
	uint8_t pattern_offset = 0;
	char *pattern_buf;
	char *pattern_mask;
	int ret;

	ENTER();

	if ((NULL == adapter) || (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		hdd_err("Invalid adapter or adapter has invalid magic");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (!wlan_hdd_validate_modules_state(hdd_ctx))
		return -EINVAL;

	if (!sme_is_feature_supported_by_fw(WOW)) {
		hdd_err("Wake-on-Wireless feature is not supported in firmware!");
		return -EINVAL;
	}

	if (count > MAX_USER_COMMAND_SIZE_WOWL_PATTERN) {
		hdd_err("Command length is larger than %d bytes",
			MAX_USER_COMMAND_SIZE_WOWL_PATTERN);
		return -EINVAL;
	}

	/* Get command from user */
	if (copy_from_user(cmd, buf, count))
		return -EFAULT;
	cmd[count] = '\0';
	sptr = cmd;

	/* Get pattern idx */
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;

	if (kstrtou8(token, 0, &pattern_idx))
		return -EINVAL;

	/* Get pattern offset */
	token = strsep(&sptr, " ");

	/* Delete pattern if no further argument */
	if (!token) {
		hdd_del_wowl_ptrn_debugfs(adapter, pattern_idx);

		return count;
	}

	if (kstrtou8(token, 0, &pattern_offset))
		return -EINVAL;

	/* Get pattern */
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;

	pattern_buf = token;

	/* Get pattern mask */
	token = strsep(&sptr, " ");
	if (!token)
		return -EINVAL;

	pattern_mask = token;
	pattern_mask[strlen(pattern_mask) - 1] = '\0';

	hdd_add_wowl_ptrn_debugfs(adapter, pattern_idx, pattern_offset,
				  pattern_buf, pattern_mask);
	EXIT();
	return count;
}

/**
 * wcnss_wowpattern_write() - SSR wrapper for __wcnss_wowpattern_write
 * @file: file pointer
 * @buf: buffer
 * @count: count
 * @ppos: position pointer
 *
 * Return: 0 on success, error number otherwise
 */
static ssize_t wcnss_wowpattern_write(struct file *file,
				      const char __user *buf,
				      size_t count, loff_t *ppos)
{
	ssize_t ret;

	cds_ssr_protect(__func__);
	ret = __wcnss_wowpattern_write(file, buf, count, ppos);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wcnss_patterngen_write() - pattern_gen debugfs handler
 * @file: debugfs file handle
 * @buf: text being written to the debugfs
 * @count: size of @buf
 * @ppos: (unused) offset into the virtual file system
 *
 * Return: number of bytes processed
 */
static ssize_t __wcnss_patterngen_write(struct file *file,
				      const char __user *buf, size_t count,
				      loff_t *ppos)
{
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx;
	tSirAddPeriodicTxPtrn *addPeriodicTxPtrnParams;
	tSirDelPeriodicTxPtrn *delPeriodicTxPtrnParams;

	char *cmd, *sptr, *token;
	uint8_t pattern_idx = 0;
	uint8_t pattern_duration = 0;
	char *pattern_buf;
	uint16_t pattern_len = 0;
	uint16_t i = 0;
	QDF_STATUS status;
	int ret;

	ENTER();

	adapter = (struct hdd_adapter *)file->private_data;
	if ((NULL == adapter) || (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		hdd_err("Invalid adapter or adapter has invalid magic");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (!wlan_hdd_validate_modules_state(hdd_ctx))
		return -EINVAL;

	if (!sme_is_feature_supported_by_fw(WLAN_PERIODIC_TX_PTRN)) {
		hdd_err("Periodic Tx Pattern Offload feature is not supported in firmware!");
		return -EINVAL;
	}

	/* Get command from user */
	if (count <= MAX_USER_COMMAND_SIZE_FRAME)
		cmd = qdf_mem_malloc(count + 1);
	else {
		hdd_err("Command length is larger than %d bytes",
			MAX_USER_COMMAND_SIZE_FRAME);

		return -EINVAL;
	}

	if (!cmd) {
		hdd_err("Memory allocation for cmd failed!");
		return -ENOMEM;
	}

	if (copy_from_user(cmd, buf, count)) {
		qdf_mem_free(cmd);
		return -EFAULT;
	}
	cmd[count] = '\0';
	sptr = cmd;

	/* Get pattern idx */
	token = strsep(&sptr, " ");
	if (!token)
		goto failure;
	if (kstrtou8(token, 0, &pattern_idx))
		goto failure;

	if (pattern_idx > (MAXNUM_PERIODIC_TX_PTRNS - 1)) {
		hdd_err("Pattern index: %d is not in the range (0 ~ %d)",
			pattern_idx, MAXNUM_PERIODIC_TX_PTRNS - 1);

		goto failure;
	}

	/* Get pattern duration */
	token = strsep(&sptr, " ");
	if (!token)
		goto failure;
	if (kstrtou8(token, 0, &pattern_duration))
		goto failure;

	/* Delete pattern using index if duration is 0 */
	if (!pattern_duration) {
		delPeriodicTxPtrnParams =
			qdf_mem_malloc(sizeof(tSirDelPeriodicTxPtrn));
		if (!delPeriodicTxPtrnParams) {
			hdd_err("Memory allocation failed!");
			qdf_mem_free(cmd);
			return -ENOMEM;
		}
		delPeriodicTxPtrnParams->ucPtrnId = pattern_idx;
		delPeriodicTxPtrnParams->ucPatternIdBitmap = 1 << pattern_idx;
		qdf_copy_macaddr(&delPeriodicTxPtrnParams->mac_address,
				 &adapter->mac_addr);

		/* Delete pattern */
		status = sme_del_periodic_tx_ptrn(hdd_ctx->hHal,
						  delPeriodicTxPtrnParams);
		if (QDF_STATUS_SUCCESS != status) {
			hdd_err("sme_del_periodic_tx_ptrn() failed!");

			qdf_mem_free(delPeriodicTxPtrnParams);
			goto failure;
		}
		qdf_mem_free(cmd);
		qdf_mem_free(delPeriodicTxPtrnParams);
		return count;
	}

	/*
	 * In SAP mode allow configuration without any connection check
	 * In STA mode check if it's in connected state before adding
	 * patterns
	 */
	hdd_debug("device mode %d", adapter->device_mode);
	if ((QDF_STA_MODE == adapter->device_mode) &&
	    (!hdd_conn_is_connected(WLAN_HDD_GET_STATION_CTX_PTR(adapter)))) {
		hdd_err("Not in Connected state!");
		goto failure;
	}

	/* Get pattern */
	token = strsep(&sptr, " ");
	if (!token)
		goto failure;

	pattern_buf = token;
	pattern_buf[strlen(pattern_buf) - 1] = '\0';
	pattern_len = strlen(pattern_buf);

	/* Since the pattern is a hex string, 2 characters represent 1 byte. */
	if (pattern_len % 2) {
		hdd_err("Malformed pattern!");

		goto failure;
	} else
		pattern_len >>= 1;

	if (pattern_len < 14 || pattern_len > PERIODIC_TX_PTRN_MAX_SIZE) {
		hdd_err("Not an 802.3 frame!");

		goto failure;
	}

	addPeriodicTxPtrnParams = qdf_mem_malloc(sizeof(tSirAddPeriodicTxPtrn));
	if (!addPeriodicTxPtrnParams) {
		hdd_err("Memory allocation failed!");
		qdf_mem_free(cmd);
		return -ENOMEM;
	}

	addPeriodicTxPtrnParams->ucPtrnId = pattern_idx;
	addPeriodicTxPtrnParams->usPtrnIntervalMs = pattern_duration * 500;
	addPeriodicTxPtrnParams->ucPtrnSize = pattern_len;
	qdf_copy_macaddr(&addPeriodicTxPtrnParams->mac_address,
			 &adapter->mac_addr);

	/* Extract the pattern */
	for (i = 0; i < addPeriodicTxPtrnParams->ucPtrnSize; i++) {
		addPeriodicTxPtrnParams->ucPattern[i] =
			(hex_to_bin(pattern_buf[0]) << 4) +
			hex_to_bin(pattern_buf[1]);

		/* Skip to next byte */
		pattern_buf += 2;
	}

	/* Add pattern */
	status = sme_add_periodic_tx_ptrn(hdd_ctx->hHal,
					  addPeriodicTxPtrnParams);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("sme_add_periodic_tx_ptrn() failed!");

		qdf_mem_free(addPeriodicTxPtrnParams);
		goto failure;
	}
	qdf_mem_free(cmd);
	qdf_mem_free(addPeriodicTxPtrnParams);
	EXIT();
	return count;

failure:
	hdd_err("Invalid input. Input format is: ptrn_idx duration pattern");
	qdf_mem_free(cmd);
	return -EINVAL;
}

/**
 * wcnss_patterngen_write() - SSR wrapper for __wcnss_patterngen_write
 * @file: file pointer
 * @buf: buffer
 * @count: count
 * @ppos: position pointer
 *
 * Return: 0 on success, error number otherwise
 */
static ssize_t wcnss_patterngen_write(struct file *file,
				      const char __user *buf,
				      size_t count, loff_t *ppos)
{
	ssize_t ret;

	cds_ssr_protect(__func__);
	ret = __wcnss_patterngen_write(file, buf, count, ppos);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef WLAN_POWER_DEBUGFS
struct power_stats_priv {
	struct power_stats_response power_stats;
};

static void hdd_power_debugstats_dealloc(void *priv)
{
	struct power_stats_priv *stats = priv;

	qdf_mem_free(stats->power_stats.debug_registers);
}

static void hdd_power_debugstats_cb(struct power_stats_response *response,
				    void *context)
{
	struct hdd_request *request;
	struct power_stats_priv *priv;
	uint32_t *debug_registers;
	uint32_t debug_registers_len;

	ENTER();

	request = hdd_request_get(context);
	if (!request) {
		hdd_err("Obsolete request");
		return;
	}

	priv = hdd_request_priv(request);

	/* copy fixed-sized data */
	priv->power_stats = *response;

	/* copy variable-size data */
	if (response->num_debug_register) {
		debug_registers_len = (sizeof(response->debug_registers[0]) *
				       response->num_debug_register);
		debug_registers = qdf_mem_malloc(debug_registers_len);
		priv->power_stats.debug_registers = debug_registers;
		if (debug_registers) {
			qdf_mem_copy(debug_registers,
				     response->debug_registers,
				     debug_registers_len);
		} else {
			hdd_err("Power stats memory alloc fails!");
			priv->power_stats.num_debug_register = 0;
		}
	}
	hdd_request_complete(request);
	hdd_request_put(request);
	EXIT();
}

/**
 * __wlan_hdd_read_power_debugfs() - API to collect Chip power stats from FW
 * @file: file pointer
 * @buf: buffer
 * @count: count
 * @pos: position pointer
 *
 * Return: Number of bytes read on success, error number otherwise
 */
static ssize_t __wlan_hdd_read_power_debugfs(struct file *file,
		char __user *buf,
		size_t count, loff_t *pos)
{
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx;
	QDF_STATUS status;
	struct power_stats_response *chip_power_stats;
	ssize_t ret_cnt = 0;
	int j;
	unsigned int len = 0;
	char *power_debugfs_buf = NULL;
	void *cookie;
	struct hdd_request *request;
	struct power_stats_priv	*priv;
	static const struct hdd_request_params params = {
		.priv_size = sizeof(*priv),
		.timeout_ms = WLAN_WAIT_TIME_STATS,
		.dealloc = hdd_power_debugstats_dealloc,
	};

	ENTER();
	adapter = (struct hdd_adapter *)file->private_data;
	if ((!adapter) || (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		hdd_err("Invalid adapter or adapter has invalid magic");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret_cnt = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret_cnt)
		return ret_cnt;

	if (!wlan_hdd_validate_modules_state(hdd_ctx))
		return -EINVAL;

	request = hdd_request_alloc(&params);
	if (!request) {
		hdd_err("Request allocation failure");
		return -ENOMEM;
	}
	cookie = hdd_request_cookie(request);

	status = sme_power_debug_stats_req(hdd_ctx->hHal,
					   hdd_power_debugstats_cb,
					   cookie);
	if (QDF_IS_STATUS_ERROR(status)) {
		hdd_err("chip power stats request failed");
		ret_cnt = -EINVAL;
		goto cleanup;
	}

	ret_cnt = hdd_request_wait_for_response(request);
	if (ret_cnt) {
		hdd_err("Target response timed out Power stats");
		ret_cnt = -ETIMEDOUT;
		goto cleanup;
	}

	priv = hdd_request_priv(request);
	chip_power_stats = &priv->power_stats;

	power_debugfs_buf = qdf_mem_malloc(POWER_DEBUGFS_BUFFER_MAX_LEN);
	if (!power_debugfs_buf) {
		hdd_err("Power stats buffer alloc fails!");
		ret_cnt = -EINVAL;
		goto cleanup;
	}

	len += scnprintf(power_debugfs_buf, POWER_DEBUGFS_BUFFER_MAX_LEN,
			"POWER DEBUG STATS\n=================\n"
			"cumulative_sleep_time_ms: %d\n"
			"cumulative_total_on_time_ms: %d\n"
			"deep_sleep_enter_counter: %d\n"
			"last_deep_sleep_enter_tstamp_ms: %d\n"
			"debug_register_fmt: %d\n"
			"num_debug_register: %d\n",
			chip_power_stats->cumulative_sleep_time_ms,
			chip_power_stats->cumulative_total_on_time_ms,
			chip_power_stats->deep_sleep_enter_counter,
			chip_power_stats->last_deep_sleep_enter_tstamp_ms,
			chip_power_stats->debug_register_fmt,
			chip_power_stats->num_debug_register);

	for (j = 0; j < chip_power_stats->num_debug_register; j++) {
		if ((POWER_DEBUGFS_BUFFER_MAX_LEN - len) > 0)
			len += scnprintf(power_debugfs_buf + len,
					POWER_DEBUGFS_BUFFER_MAX_LEN - len,
					"debug_registers[%d]: 0x%x\n", j,
					chip_power_stats->debug_registers[j]);
		else
			j = chip_power_stats->num_debug_register;
	}

	ret_cnt = simple_read_from_buffer(buf, count, pos,
					  power_debugfs_buf, len);

 cleanup:
	qdf_mem_free(power_debugfs_buf);
	hdd_request_put(request);

	return ret_cnt;
}

/**
 * wlan_hdd_read_power_debugfs() - SSR wrapper function to read power debugfs
 * @file: file pointer
 * @buf: buffer
 * @count: count
 * @pos: position pointer
 *
 * Return: Number of bytes read on success, error number otherwise
 */
static ssize_t wlan_hdd_read_power_debugfs(struct file *file,
		char __user *buf,
		size_t count, loff_t *pos)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_read_power_debugfs(file, buf, count, pos);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * __wlan_hdd_open_power_debugfs() - Function to save private on open
 * @inode: Pointer to inode structure
 * @file: file pointer
 *
 * Return: zero
 */
static int __wlan_hdd_open_power_debugfs(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}


/**
 * wlan_hdd_open_power_debugfs() - SSR wrapper function to save private on open
 * @inode: Pointer to inode structure
 * @file: file pointer
 *
 * Return: zero
 */
static int wlan_hdd_open_power_debugfs(struct inode *inode, struct file *file)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_open_power_debugfs(inode, file);
	cds_ssr_unprotect(__func__);

	return ret;
}
#endif

/**
 * __wcnss_debugfs_open() - Generic debugfs open() handler
 * @inode: inode of the debugfs file
 * @file: file handle of the debugfs file
 *
 * Return: 0
 */
static int __wcnss_debugfs_open(struct inode *inode, struct file *file)
{
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx;
	int ret;

	ENTER();

	if (inode->i_private)
		file->private_data = inode->i_private;

	adapter = (struct hdd_adapter *)file->private_data;
	if ((NULL == adapter) || (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		hdd_err("Invalid adapter or adapter has invalid magic");
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;
	EXIT();
	return 0;
}

/**
 * wcnss_debugfs_open() - SSR wrapper for __wcnss_debugfs_open
 * @inode: inode pointer
 * @file: file pointer
 *
 * Return: 0 on success, error number otherwise
 */
static int wcnss_debugfs_open(struct inode *inode, struct file *file)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wcnss_debugfs_open(inode, file);
	cds_ssr_unprotect(__func__);

	return ret;
}

static const struct file_operations fops_wowpattern = {
	.write = wcnss_wowpattern_write,
	.open = wcnss_debugfs_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

static const struct file_operations fops_patterngen = {
	.write = wcnss_patterngen_write,
	.open = wcnss_debugfs_open,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

#ifdef WLAN_POWER_DEBUGFS
static const struct file_operations fops_powerdebugs = {
	.read = wlan_hdd_read_power_debugfs,
	.open = wlan_hdd_open_power_debugfs,
	.owner = THIS_MODULE,
	.llseek = default_llseek,
};

/**
 * wlan_hdd_create_power_stats_file() - API to create power stats file
 * @adapter: interface adapter pointer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wlan_hdd_create_power_stats_file(struct hdd_adapter *adapter)
{
	if (!debugfs_create_file("power_stats", 00400 | 00040 | 00004,
				adapter->debugfs_phy, adapter,
				&fops_powerdebugs))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

#else
static QDF_STATUS wlan_hdd_create_power_stats_file(struct hdd_adapter *adapter)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * hdd_debugfs_init() - Initialize debugfs interface
 * @adapter: interface adapter pointer
 *
 * Register support for the debugfs files supported by the driver.
 *
 * NB: The current implementation only supports debugfs operations
 * on the primary interface, i.e. wlan0
 *
 * Return: QDF_STATUS_SUCCESS if all files registered,
 *	   QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS hdd_debugfs_init(struct hdd_adapter *adapter)
{
	struct net_device *dev = adapter->dev;

	adapter->debugfs_phy = debugfs_create_dir(dev->name, 0);

	if (NULL == adapter->debugfs_phy)
		return QDF_STATUS_E_FAILURE;

	if (NULL == debugfs_create_file("wow_pattern", 00400 | 00200,
					adapter->debugfs_phy, adapter,
					&fops_wowpattern))
		return QDF_STATUS_E_FAILURE;

	if (NULL == debugfs_create_file("pattern_gen", 00400 | 00200,
					adapter->debugfs_phy, adapter,
					&fops_patterngen))
		return QDF_STATUS_E_FAILURE;

	if (QDF_STATUS_SUCCESS != wlan_hdd_create_power_stats_file(adapter))
		return QDF_STATUS_E_FAILURE;

	if (0 != wlan_hdd_create_ll_stats_file(adapter))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

/**
 * hdd_debugfs_exit() - Shutdown debugfs interface
 * @adapter: interface adapter pointer
 *
 * Unregister support for the debugfs files supported by the driver.
 *
 * Return: None
 */
void hdd_debugfs_exit(struct hdd_adapter *adapter)
{
	debugfs_remove_recursive(adapter->debugfs_phy);
}
#endif /* #ifdef WLAN_OPEN_SOURCE */
