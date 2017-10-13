/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
 * wlan_hdd_tsf.c - WLAN Host Device Driver tsf related implementation
 */

#include "wlan_hdd_main.h"
#include "wlan_hdd_tsf.h"
#include "wma_api.h"
#include <qca_vendor.h>

static struct completion tsf_sync_get_completion_evt;
#define WLAN_TSF_SYNC_GET_TIMEOUT 2000

/**
 * enum hdd_tsf_op_result - result of tsf operation
 *
 * HDD_TSF_OP_SUCC:  succeed
 * HDD_TSF_OP_FAIL:  fail
 */
enum hdd_tsf_op_result {
	HDD_TSF_OP_SUCC,
	HDD_TSF_OP_FAIL
};

#ifdef WLAN_FEATURE_TSF_PLUS
static inline void hdd_set_th_sync_status(struct hdd_adapter *adapter,
					  bool initialized)
{
	qdf_atomic_set(&adapter->tsf_sync_ready_flag,
		       (initialized ? 1 : 0));
}

static inline bool hdd_get_th_sync_status(struct hdd_adapter *adapter)
{
	return qdf_atomic_read(&adapter->tsf_sync_ready_flag) != 0;
}

#else
static inline bool hdd_get_th_sync_status(struct hdd_adapter *adapter)
{
	return true;
}
#endif

static
enum hdd_tsf_get_state hdd_tsf_check_conn_state(struct hdd_adapter *adapter)
{
	enum hdd_tsf_get_state ret = TSF_RETURN;
	struct hdd_station_ctx *hdd_sta_ctx;

	if (adapter->device_mode == QDF_STA_MODE ||
			adapter->device_mode == QDF_P2P_CLIENT_MODE) {
		hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
		if (hdd_sta_ctx->conn_info.connState !=
				eConnectionState_Associated) {
			hdd_err("failed to cap tsf, not connect with ap");
			ret = TSF_STA_NOT_CONNECTED_NO_TSF;
		}
	} else if ((adapter->device_mode == QDF_SAP_MODE ||
				adapter->device_mode == QDF_P2P_GO_MODE) &&
			!(test_bit(SOFTAP_BSS_STARTED,
					&adapter->event_flags))) {
		hdd_err("Soft AP / P2p GO not beaconing");
		ret = TSF_SAP_NOT_STARTED_NO_TSF;
	}
	return ret;
}

static bool hdd_tsf_is_initialized(struct hdd_adapter *adapter)
{
	struct hdd_context *hddctx;

	if (!adapter) {
		hdd_err("invalid adapter");
		return false;
	}

	hddctx = WLAN_HDD_GET_CTX(adapter);
	if (!hddctx) {
		hdd_err("invalid hdd context");
		return false;
	}

	if (!qdf_atomic_read(&hddctx->tsf_ready_flag) ||
	    !hdd_get_th_sync_status(adapter)) {
		hdd_err("TSF is not initialized");
		return false;
	}

	return true;
}

/**
 * hdd_tsf_reset_gpio() - Reset TSF GPIO used for host timer sync
 * @adapter: pointer to adapter
 *
 * This function send WMI command to reset GPIO configured in FW after
 * TSF get operation.
 *
 * Return: TSF_RETURN on Success, TSF_RESET_GPIO_FAIL on failure
 */
#ifdef QCA_WIFI_3_0
static int hdd_tsf_reset_gpio(struct hdd_adapter *adapter)
{
	/* No GPIO Host timer sync for integrated WIFI Device */
	return TSF_RETURN;
}
#else
static int hdd_tsf_reset_gpio(struct hdd_adapter *adapter)
{
	int ret;

	ret = wma_cli_set_command((int)adapter->sessionId,
			(int)GEN_PARAM_RESET_TSF_GPIO, adapter->sessionId,
			GEN_CMD);

	if (ret != 0) {
		hdd_err("tsf reset GPIO fail ");
		ret = TSF_RESET_GPIO_FAIL;
	} else {
		ret = TSF_RETURN;
	}
	return ret;
}
#endif

static enum hdd_tsf_op_result hdd_capture_tsf_internal(
	struct hdd_adapter *adapter, uint32_t *buf, int len)
{
	int ret;
	struct hdd_context *hddctx;

	if (adapter == NULL || buf == NULL) {
		hdd_err("invalid pointer");
		return HDD_TSF_OP_FAIL;
	}

	if (len != 1)
		return HDD_TSF_OP_FAIL;

	hddctx = WLAN_HDD_GET_CTX(adapter);
	if (!hddctx) {
		hdd_err("invalid hdd context");
		return HDD_TSF_OP_FAIL;
	}

	if (!hdd_tsf_is_initialized(adapter)) {
		buf[0] = TSF_NOT_READY;
		return HDD_TSF_OP_SUCC;
	}

	buf[0] = hdd_tsf_check_conn_state(adapter);
	if (buf[0] != TSF_RETURN)
		return HDD_TSF_OP_SUCC;

	if (qdf_atomic_inc_return(&hddctx->cap_tsf_flag) > 1) {
		hdd_err("current in capture state");
		buf[0] = TSF_CURRENT_IN_CAP_STATE;
		return HDD_TSF_OP_SUCC;
	}

	/* record adapter for cap_tsf_irq_handler  */
	hddctx->cap_tsf_context = adapter;

	hdd_err("+ioctl issue cap tsf cmd");

	/* Reset TSF value for new capture */
	adapter->cur_target_time = 0;

	buf[0] = TSF_RETURN;
	init_completion(&tsf_sync_get_completion_evt);
	ret = wma_cli_set_command((int)adapter->sessionId,
				  (int)GEN_PARAM_CAPTURE_TSF,
				  adapter->sessionId, GEN_CMD);
	if (QDF_STATUS_SUCCESS != ret) {
		hdd_err("cap tsf fail");
		buf[0] = TSF_CAPTURE_FAIL;
		hddctx->cap_tsf_context = NULL;
		qdf_atomic_set(&hddctx->cap_tsf_flag, 0);
		return HDD_TSF_OP_SUCC;
	}
	hdd_err("-ioctl return cap tsf cmd");
	return HDD_TSF_OP_SUCC;
}

static enum hdd_tsf_op_result hdd_indicate_tsf_internal(
	struct hdd_adapter *adapter, uint32_t *buf, int len)
{
	int ret;
	struct hdd_context *hddctx;

	if (!adapter || !buf) {
		hdd_err("invalid pointer");
		return HDD_TSF_OP_FAIL;
	}

	if (len != 3)
		return HDD_TSF_OP_FAIL;

	hddctx = WLAN_HDD_GET_CTX(adapter);
	if (!hddctx) {
		hdd_err("invalid hdd context");
		return HDD_TSF_OP_FAIL;
	}

	buf[1] = 0;
	buf[2] = 0;

	if (!hdd_tsf_is_initialized(adapter)) {
		buf[0] = TSF_NOT_READY;
		return HDD_TSF_OP_SUCC;
	}

	buf[0] = hdd_tsf_check_conn_state(adapter);
	if (buf[0] != TSF_RETURN)
		return HDD_TSF_OP_SUCC;

	if (adapter->cur_target_time == 0) {
		hdd_info("TSF value not received");
		buf[0] = TSF_NOT_RETURNED_BY_FW;
		return HDD_TSF_OP_SUCC;
	}

	buf[0] = TSF_RETURN;
	buf[1] = (uint32_t)(adapter->cur_target_time & 0xffffffff);
	buf[2] = (uint32_t)((adapter->cur_target_time >> 32) &
				0xffffffff);

	if (!qdf_atomic_read(&hddctx->cap_tsf_flag)) {
		hdd_info("old: status=%u, tsf_low=%u, tsf_high=%u",
			 buf[0], buf[1], buf[2]);
		return HDD_TSF_OP_SUCC;
	}

	ret = hdd_tsf_reset_gpio(adapter);
	if (0 != ret) {
		hdd_err("reset tsf gpio fail");
		buf[0] = TSF_RESET_GPIO_FAIL;
		return HDD_TSF_OP_SUCC;
	}
	hddctx->cap_tsf_context = NULL;
	qdf_atomic_set(&hddctx->cap_tsf_flag, 0);
	hdd_info("get tsf cmd,status=%u, tsf_low=%u, tsf_high=%u",
		 buf[0], buf[1], buf[2]);

	return HDD_TSF_OP_SUCC;
}

#ifdef WLAN_FEATURE_TSF_PLUS
/* unit for target time: us;  host time: ns */
#define HOST_TO_TARGET_TIME_RATIO NSEC_PER_USEC
#define MAX_ALLOWED_DEVIATION_NS (20 * NSEC_PER_MSEC)
#define MAX_CONTINUOUS_ERROR_CNT 3

/* to distinguish 32-bit overflow case, this inverval should:
 * equal or less than (1/2 * OVERFLOW_INDICATOR32 us)
 */
#define WLAN_HDD_CAPTURE_TSF_INTERVAL_SEC 500
#define WLAN_HDD_CAPTURE_TSF_INIT_INTERVAL_MS 100
#define NORMAL_INTERVAL_TARGET \
	((int64_t)((int64_t)WLAN_HDD_CAPTURE_TSF_INTERVAL_SEC * \
		NSEC_PER_SEC / HOST_TO_TARGET_TIME_RATIO))
#define OVERFLOW_INDICATOR32 (((int64_t)0x1) << 32)

/**
 * TS_STATUS - timestamp status
 *
 * HDD_TS_STATUS_WAITING:  one of the stamp-pair
 *    is not updated
 * HDD_TS_STATUS_READY:  valid tstamp-pair
 * HDD_TS_STATUS_INVALID: invalid tstamp-pair
 */
enum hdd_ts_status {
	HDD_TS_STATUS_WAITING,
	HDD_TS_STATUS_READY,
	HDD_TS_STATUS_INVALID
};

static
enum hdd_tsf_op_result __hdd_start_tsf_sync(struct hdd_adapter *adapter)
{
	QDF_STATUS ret;

	if (!hdd_get_th_sync_status(adapter)) {
		hdd_err("Host Target sync has not initialized");
		return HDD_TSF_OP_FAIL;
	}

	ret = qdf_mc_timer_start(&adapter->host_target_sync_timer,
				 WLAN_HDD_CAPTURE_TSF_INIT_INTERVAL_MS);
	if (ret != QDF_STATUS_SUCCESS && ret != QDF_STATUS_E_ALREADY) {
		hdd_err("Failed to start timer, ret: %d", ret);
		return HDD_TSF_OP_FAIL;
	}
	return HDD_TSF_OP_SUCC;
}

static
enum hdd_tsf_op_result __hdd_stop_tsf_sync(struct hdd_adapter *adapter)
{
	QDF_STATUS ret;

	if (!hdd_get_th_sync_status(adapter)) {
		hdd_err("Host Target sync has not initialized");
		return HDD_TSF_OP_SUCC;
	}

	ret = qdf_mc_timer_stop(&adapter->host_target_sync_timer);
	if (ret != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to stop timer, ret: %d", ret);
		return HDD_TSF_OP_FAIL;
	}
	return HDD_TSF_OP_SUCC;
}

static inline void hdd_reset_timestamps(struct hdd_adapter *adapter)
{
	qdf_spin_lock_bh(&adapter->host_target_sync_lock);
	adapter->cur_host_time = 0;
	adapter->cur_target_time = 0;
	adapter->last_host_time = 0;
	adapter->last_target_time = 0;
	qdf_spin_unlock_bh(&adapter->host_target_sync_lock);
}

/**
 * hdd_check_timestamp_status() - return the tstamp status
 *
 * @last_target_time: the last saved target time
 * @last_host_time: the last saved host time
 * @cur_target_time : new target time
 * @cur_host_time : new host time
 *
 * This function check the new timstamp-pair(cur_host_time/cur_target_time)
 *
 * Return:
 * HDD_TS_STATUS_WAITING: cur_host_time or cur_host_time is 0
 * HDD_TS_STATUS_READY: cur_target_time/cur_host_time is a valid pair,
 *    and can be saved
 * HDD_TS_STATUS_INVALID: cur_target_time/cur_host_time is a invalid pair,
 *    should be discard
 */
static
enum hdd_ts_status hdd_check_timestamp_status(
		uint64_t last_target_time,
		uint64_t last_host_time,
		uint64_t cur_target_time,
		uint64_t cur_host_time)
{
	uint64_t delta_ns, delta_target_time, delta_host_time;

	/* one or more are not updated, need to wait */
	if (cur_target_time == 0 || cur_host_time == 0)
		return HDD_TS_STATUS_WAITING;

	/* init value, it's the first time to update the pair */
	if (last_target_time == 0 && last_host_time == 0)
		return HDD_TS_STATUS_READY;

	/* the new values should be greater than the saved values */
	if ((cur_target_time <= last_target_time) ||
	    (cur_host_time <= last_host_time)) {
		hdd_err("Invalid timestamps!last_target_time: %llu;"
			"last_host_time: %llu; cur_target_time: %llu;"
			"cur_host_time: %llu",
			last_target_time, last_host_time,
			cur_target_time, cur_host_time);
		return HDD_TS_STATUS_INVALID;
	}

	delta_target_time = (cur_target_time - last_target_time) *
		HOST_TO_TARGET_TIME_RATIO;
	delta_host_time = cur_host_time - last_host_time;

	/*
	 * DO NOT use abs64() , a big uint64 value might be turned to
	 * a small int64 value
	 */
	delta_ns = ((delta_target_time > delta_host_time) ?
			(delta_target_time - delta_host_time) :
			(delta_host_time - delta_target_time));

	/* the deviation should be smaller than a threshold */
	if (delta_ns > MAX_ALLOWED_DEVIATION_NS) {
		hdd_info("Invalid timestamps - delta: %llu ns", delta_ns);
		return HDD_TS_STATUS_INVALID;
	}
	return HDD_TS_STATUS_READY;
}

static void hdd_update_timestamp(struct hdd_adapter *adapter,
				 uint64_t target_time, uint64_t host_time)
{
	int interval = 0;
	enum hdd_ts_status sync_status;

	if (!adapter)
		return;

	/* host time is updated in IRQ context, it's always before target time,
	 * and so no need to try update last_host_time at present;
	 * since the interval of capturing TSF
	 * (WLAN_HDD_CAPTURE_TSF_INTERVAL_SEC) is long enough, host and target
	 * time are updated in pairs, and one by one, we can return here to
	 * avoid requiring spin lock, and to speed up the IRQ processing.
	 */
	if (host_time > 0) {
		adapter->cur_host_time = host_time;
		return;
	}

	qdf_spin_lock_bh(&adapter->host_target_sync_lock);
	if (target_time > 0)
		adapter->cur_target_time = target_time;

	sync_status = hdd_check_timestamp_status(adapter->last_target_time,
						 adapter->last_host_time,
						 adapter->cur_target_time,
						 adapter->cur_host_time);
	switch (sync_status) {
	case HDD_TS_STATUS_INVALID:
		if (++adapter->continuous_error_count <
		    MAX_CONTINUOUS_ERROR_CNT) {
			interval =
				WLAN_HDD_CAPTURE_TSF_INIT_INTERVAL_MS;
			adapter->cur_target_time = 0;
			adapter->cur_host_time = 0;
			break;
		}
		hdd_info("Reach the max continuous error count");
		/*
		 * fall through:
		 * If reach MAX_CONTINUOUS_ERROR_CNT, treat it as a
		 * valid pair
		 */
	case HDD_TS_STATUS_READY:
		adapter->last_target_time = adapter->cur_target_time;
		adapter->last_host_time = adapter->cur_host_time;
		adapter->cur_target_time = 0;
		adapter->cur_host_time = 0;
		hdd_info("ts-pair updated: target: %llu; host: %llu",
			 adapter->last_target_time,
			 adapter->last_host_time);
		interval = WLAN_HDD_CAPTURE_TSF_INTERVAL_SEC *
			MSEC_PER_SEC;
		adapter->continuous_error_count = 0;
		break;
	case HDD_TS_STATUS_WAITING:
		interval = 0;
		break;
	}
	qdf_spin_unlock_bh(&adapter->host_target_sync_lock);

	if (interval > 0)
		qdf_mc_timer_start(&adapter->host_target_sync_timer, interval);
}

static inline bool hdd_tsf_is_in_cap(struct hdd_adapter *adapter)
{
	struct hdd_context *hddctx;

	hddctx = WLAN_HDD_GET_CTX(adapter);
	if (!hddctx)
		return false;

	return qdf_atomic_read(&hddctx->cap_tsf_flag) > 0;
}

/* define 64bit plus/minus to deal with overflow */
static inline int hdd_64bit_plus(uint64_t x, int64_t y, uint64_t *ret)
{
	if ((y < 0 && (-y) > x) ||
	    (y > 0 && (y > U64_MAX - x))) {
		*ret = 0;
		return -EINVAL;
	}

	*ret = x + y;
	return 0;
}

static inline int hdd_uint64_plus(uint64_t x, uint64_t y, uint64_t *ret)
{
	if (!ret)
		return -EINVAL;

	if (x > (U64_MAX - y)) {
		*ret = 0;
		return -EINVAL;
	}

	*ret = x + y;
	return 0;
}

static inline int hdd_uint64_minus(uint64_t x, uint64_t y, uint64_t *ret)
{
	if (!ret)
		return -EINVAL;

	if (x < y) {
		*ret = 0;
		return -EINVAL;
	}

	*ret = x - y;
	return 0;
}

static inline int32_t hdd_get_hosttime_from_targettime(
	struct hdd_adapter *adapter, uint64_t target_time,
	uint64_t *host_time)
{
	int32_t ret = -EINVAL;
	int64_t delta32_target;
	bool in_cap_state;

	in_cap_state = hdd_tsf_is_in_cap(adapter);

	/*
	 * To avoid check the lock when it's not capturing tsf
	 * (the tstamp-pair won't be changed)
	 */
	if (in_cap_state)
		qdf_spin_lock_bh(&adapter->host_target_sync_lock);

	/* at present, target_time is only 32bit in fact */
	delta32_target = (int64_t)((target_time & U32_MAX) -
			(adapter->last_target_time & U32_MAX));

	if (delta32_target <
			(NORMAL_INTERVAL_TARGET - OVERFLOW_INDICATOR32))
		delta32_target += OVERFLOW_INDICATOR32;
	else if (delta32_target >
			(OVERFLOW_INDICATOR32 - NORMAL_INTERVAL_TARGET))
		delta32_target -= OVERFLOW_INDICATOR32;

	ret = hdd_64bit_plus(adapter->last_host_time,
			     HOST_TO_TARGET_TIME_RATIO * delta32_target,
			     host_time);

	if (in_cap_state)
		qdf_spin_unlock_bh(&adapter->host_target_sync_lock);

	return ret;
}

static inline int32_t hdd_get_targettime_from_hosttime(
	struct hdd_adapter *adapter, uint64_t host_time,
	uint64_t *target_time)
{
	int32_t ret = -EINVAL;
	bool in_cap_state;

	if (!adapter || host_time == 0)
		return ret;

	in_cap_state = hdd_tsf_is_in_cap(adapter);
	if (in_cap_state)
		qdf_spin_lock_bh(&adapter->host_target_sync_lock);

	if (host_time < adapter->last_host_time)
		ret = hdd_uint64_minus(adapter->last_target_time,
				       qdf_do_div(adapter->last_host_time -
						  host_time,
						  HOST_TO_TARGET_TIME_RATIO),
				       target_time);
	else
		ret = hdd_uint64_plus(adapter->last_target_time,
				      qdf_do_div(host_time -
						 adapter->last_host_time,
						 HOST_TO_TARGET_TIME_RATIO),
				      target_time);

	if (in_cap_state)
		qdf_spin_unlock_bh(&adapter->host_target_sync_lock);

	return ret;
}

static inline
uint64_t hdd_get_monotonic_host_time(struct hdd_context *hdd_ctx)
{
	return HDD_TSF_IS_RAW_SET(hdd_ctx) ?
		ktime_get_ns() : ktime_get_real_ns();
}

static ssize_t __hdd_wlan_tsf_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct hdd_station_ctx *hdd_sta_ctx;
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx;
	ssize_t size;
	uint64_t host_time, target_time;

	struct net_device *net_dev = container_of(dev, struct net_device, dev);

	adapter = (struct hdd_adapter *)(netdev_priv(net_dev));
	if (adapter->magic != WLAN_HDD_ADAPTER_MAGIC)
		return scnprintf(buf, PAGE_SIZE, "Invalid device\n");

	if (!hdd_get_th_sync_status(adapter))
		return scnprintf(buf, PAGE_SIZE,
				 "TSF sync is not initialized\n");

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (eConnectionState_Associated != hdd_sta_ctx->conn_info.connState)
		return scnprintf(buf, PAGE_SIZE, "NOT connected\n");

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (!hdd_ctx)
		return scnprintf(buf, PAGE_SIZE, "Invalid HDD context\n");

	host_time = hdd_get_monotonic_host_time(hdd_ctx);
	if (hdd_get_targettime_from_hosttime(adapter, host_time,
					     &target_time))
		size = scnprintf(buf, PAGE_SIZE, "Invalid timestamp\n");
	else
		size = scnprintf(buf, PAGE_SIZE, "%s%llu %llu %pM\n",
				 buf, target_time, host_time,
				 hdd_sta_ctx->conn_info.bssId.bytes);
	return size;
}

static ssize_t hdd_wlan_tsf_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	ssize_t ret;

	cds_ssr_protect(__func__);
	ret = __hdd_wlan_tsf_show(dev, attr, buf);
	cds_ssr_unprotect(__func__);

	return ret;
}

static DEVICE_ATTR(tsf, 0400, hdd_wlan_tsf_show, NULL);

static void hdd_capture_tsf_timer_expired_handler(void *arg)
{
	uint32_t tsf_op_resp;
	struct hdd_adapter *adapter;

	if (!arg)
		return;

	adapter = (struct hdd_adapter *)arg;
	hdd_capture_tsf_internal(adapter, &tsf_op_resp, 1);
}

static irqreturn_t hdd_tsf_captured_irq_handler(int irq, void *arg)
{
	struct hdd_adapter *adapter;
	struct hdd_context *hdd_ctx;
	uint64_t host_time;
	char *name = NULL;

	if (!arg)
		return IRQ_NONE;

	hdd_ctx = (struct hdd_context *)arg;
	host_time = hdd_get_monotonic_host_time(hdd_ctx);

	adapter = hdd_ctx->cap_tsf_context;
	if (!adapter)
		return IRQ_HANDLED;

	if (!hdd_tsf_is_initialized(adapter)) {
		hdd_err("tsf is not init, ignore irq");
		return IRQ_HANDLED;
	}

	hdd_update_timestamp(adapter, 0, host_time);
	if (adapter->dev)
		name = adapter->dev->name;

	hdd_info("irq: %d - iface: %s - host_time: %llu",
		 irq, (!name ? "none" : name), host_time);

	return IRQ_HANDLED;
}

static enum hdd_tsf_op_result hdd_tsf_sync_init(struct hdd_adapter *adapter)
{
	QDF_STATUS ret;
	struct hdd_context *hddctx;
	struct net_device *net_dev;

	if (!adapter)
		return HDD_TSF_OP_FAIL;

	hddctx = WLAN_HDD_GET_CTX(adapter);
	if (!hddctx) {
		hdd_err("invalid hdd context");
		return HDD_TSF_OP_FAIL;
	}

	if (!qdf_atomic_read(&hddctx->tsf_ready_flag)) {
		hdd_err("TSF feature has NOT been initialized");
		return HDD_TSF_OP_FAIL;
	}

	if (hdd_get_th_sync_status(adapter)) {
		hdd_err("Host Target sync has been initialized!!");
		return HDD_TSF_OP_SUCC;
	}

	qdf_spinlock_create(&adapter->host_target_sync_lock);

	hdd_reset_timestamps(adapter);

	ret = qdf_mc_timer_init(&adapter->host_target_sync_timer,
				QDF_TIMER_TYPE_SW,
				hdd_capture_tsf_timer_expired_handler,
				(void *)adapter);
	if (ret != QDF_STATUS_SUCCESS) {
		hdd_err("Failed to init timer, ret: %d", ret);
		goto fail;
	}

	net_dev = adapter->dev;
	if (net_dev && HDD_TSF_IS_DBG_FS_SET(hddctx))
		device_create_file(&net_dev->dev, &dev_attr_tsf);
	hdd_set_th_sync_status(adapter, true);

	return HDD_TSF_OP_SUCC;
fail:
	hdd_set_th_sync_status(adapter, false);
	return HDD_TSF_OP_FAIL;
}

static enum hdd_tsf_op_result hdd_tsf_sync_deinit(struct hdd_adapter *adapter)
{
	QDF_STATUS ret;
	struct hdd_context *hddctx;
	struct net_device *net_dev;

	if (!adapter)
		return HDD_TSF_OP_FAIL;

	if (!hdd_get_th_sync_status(adapter)) {
		hdd_err("Host Target sync has not been initialized!!");
		return HDD_TSF_OP_SUCC;
	}

	hdd_set_th_sync_status(adapter, false);

	ret = qdf_mc_timer_destroy(&adapter->host_target_sync_timer);
	if (ret != QDF_STATUS_SUCCESS)
		hdd_err("Failed to destroy timer, ret: %d", ret);

	hddctx = WLAN_HDD_GET_CTX(adapter);

	/* reset the cap_tsf flag and gpio if needed */
	if (hddctx && qdf_atomic_read(&hddctx->cap_tsf_flag) &&
	    hddctx->cap_tsf_context == adapter) {
		int reset_ret = hdd_tsf_reset_gpio(adapter);

		if (reset_ret)
			hdd_err("Failed to reset tsf gpio, ret:%d",
				reset_ret);
		hddctx->cap_tsf_context = NULL;
		qdf_atomic_set(&hddctx->cap_tsf_flag, 0);
	}

	hdd_reset_timestamps(adapter);

	net_dev = adapter->dev;
	if (net_dev && HDD_TSF_IS_DBG_FS_SET(hddctx)) {
		struct device *dev = &net_dev->dev;

		device_remove_file(dev, &dev_attr_tsf);
	}
	return HDD_TSF_OP_SUCC;
}

static inline void hdd_update_tsf(struct hdd_adapter *adapter, uint64_t tsf)
{
	uint32_t tsf_op_resp[3];

	hdd_indicate_tsf_internal(adapter, tsf_op_resp, 3);
	hdd_update_timestamp(adapter, tsf, 0);
}

static inline
enum hdd_tsf_op_result hdd_netbuf_timestamp(qdf_nbuf_t netbuf,
					    uint64_t target_time)
{
	struct hdd_adapter *adapter;
	struct net_device *net_dev = netbuf->dev;

	if (!net_dev)
		return HDD_TSF_OP_FAIL;

	adapter = (struct hdd_adapter *)(netdev_priv(net_dev));
	if (adapter && adapter->magic == WLAN_HDD_ADAPTER_MAGIC &&
	    hdd_get_th_sync_status(adapter)) {
		uint64_t host_time;
		int32_t ret = hdd_get_hosttime_from_targettime(adapter,
				target_time, &host_time);
		if (!ret) {
			netbuf->tstamp = ns_to_ktime(host_time);
			return HDD_TSF_OP_SUCC;
		}
	}

	return HDD_TSF_OP_FAIL;
}

int hdd_start_tsf_sync(struct hdd_adapter *adapter)
{
	enum hdd_tsf_op_result ret;

	if (!adapter)
		return -EINVAL;

	ret = hdd_tsf_sync_init(adapter);
	if (ret != HDD_TSF_OP_SUCC) {
		hdd_err("Failed to init tsf sync, ret: %d", ret);
		return -EINVAL;
	}

	return (__hdd_start_tsf_sync(adapter) ==
		HDD_TSF_OP_SUCC) ? 0 : -EINVAL;
}

int hdd_stop_tsf_sync(struct hdd_adapter *adapter)
{
	enum hdd_tsf_op_result ret;

	if (!adapter)
		return -EINVAL;

	ret = __hdd_stop_tsf_sync(adapter);
	if (ret != HDD_TSF_OP_SUCC)
		return -EINVAL;

	ret = hdd_tsf_sync_deinit(adapter);
	if (ret != HDD_TSF_OP_SUCC) {
		hdd_err("Failed to deinit tsf sync, ret: %d", ret);
		return -EINVAL;
	}
	return 0;
}

int hdd_tx_timestamp(qdf_nbuf_t netbuf, uint64_t target_time)
{
	struct sock *sk = netbuf->sk;

	if (!sk)
		return -EINVAL;

	if ((skb_shinfo(netbuf)->tx_flags & SKBTX_SW_TSTAMP) &&
	    !(skb_shinfo(netbuf)->tx_flags & SKBTX_IN_PROGRESS)) {
		struct sock_exterr_skb *serr;
		qdf_nbuf_t new_netbuf;
		int err;

		if (hdd_netbuf_timestamp(netbuf, target_time) !=
		    HDD_TSF_OP_SUCC)
			return -EINVAL;

		new_netbuf = qdf_nbuf_clone(netbuf);
		if (!new_netbuf)
			return -ENOMEM;

		serr = SKB_EXT_ERR(new_netbuf);
		memset(serr, 0, sizeof(*serr));
		serr->ee.ee_errno = ENOMSG;
		serr->ee.ee_origin = SO_EE_ORIGIN_TIMESTAMPING;

		err = sock_queue_err_skb(sk, new_netbuf);
		if (err) {
			qdf_nbuf_free(new_netbuf);
			return err;
		}

		return 0;
	}
	return -EINVAL;
}

int hdd_rx_timestamp(qdf_nbuf_t netbuf, uint64_t target_time)
{
	if (hdd_netbuf_timestamp(netbuf, target_time) ==
		HDD_TSF_OP_SUCC)
		return 0;

	/* reset tstamp when failed */
	netbuf->tstamp = ns_to_ktime(0);
	return -EINVAL;
}

static inline int __hdd_capture_tsf(struct hdd_adapter *adapter,
				    uint32_t *buf, int len)
{
	if (!adapter || !buf) {
		hdd_err("invalid pointer");
		return -EINVAL;
	}

	if (len != 1)
		return -EINVAL;

	buf[0] = TSF_DISABLED_BY_TSFPLUS;

	return 0;
}

static inline int __hdd_indicate_tsf(struct hdd_adapter *adapter,
				     uint32_t *buf, int len)
{
	if (!adapter || !buf) {
		hdd_err("invalid pointer");
		return -EINVAL;
	}

	if (len != 3)
		return -EINVAL;

	buf[0] = TSF_DISABLED_BY_TSFPLUS;
	buf[1] = 0;
	buf[2] = 0;

	return 0;
}

static inline
enum hdd_tsf_op_result wlan_hdd_tsf_plus_init(struct hdd_context *hdd_ctx)
{
	int ret;

	if (!HDD_TSF_IS_PTP_ENABLED(hdd_ctx)) {
		hdd_info("To enable TSF_PLUS, set gtsf_ptp_options in ini");
		return HDD_TSF_OP_FAIL;
	}

	ret = cnss_common_register_tsf_captured_handler(
			hdd_ctx->parent_dev,
			hdd_tsf_captured_irq_handler,
			(void *)hdd_ctx);
	if (ret != 0) {
		hdd_err("Failed to register irq handler: %d", ret);
		return HDD_TSF_OP_FAIL;
	}

	if (HDD_TSF_IS_TX_SET(hdd_ctx))
		ol_register_timestamp_callback(hdd_tx_timestamp);
	return HDD_TSF_OP_SUCC;
}

static inline
enum hdd_tsf_op_result wlan_hdd_tsf_plus_deinit(struct hdd_context *hdd_ctx)
{
	int ret;

	if (!HDD_TSF_IS_PTP_ENABLED(hdd_ctx))
		return HDD_TSF_OP_SUCC;

	if (HDD_TSF_IS_TX_SET(hdd_ctx))
		ol_deregister_timestamp_callback();

	ret = cnss_common_unregister_tsf_captured_handler(
				hdd_ctx->parent_dev,
				(void *)hdd_ctx);
	if (ret != 0) {
		hdd_err("Failed to unregister irq handler, ret:%d",
			ret);
		ret = HDD_TSF_OP_FAIL;
	}

	return HDD_TSF_OP_SUCC;
}

void hdd_tsf_notify_wlan_state_change(struct hdd_adapter *adapter,
				      eConnectionState old_state,
				      eConnectionState new_state)
{
	if (!adapter)
		return;

	if (old_state != eConnectionState_Associated &&
	    new_state == eConnectionState_Associated)
		hdd_start_tsf_sync(adapter);
	else if (old_state == eConnectionState_Associated &&
		 new_state != eConnectionState_Associated)
		hdd_stop_tsf_sync(adapter);
}
#else
static inline void hdd_update_tsf(struct hdd_adapter *adapter, uint64_t tsf)
{
}

static inline int __hdd_indicate_tsf(struct hdd_adapter *adapter,
				     uint32_t *buf, int len)
{
	return (hdd_indicate_tsf_internal(adapter, buf, len) ==
		HDD_TSF_OP_SUCC) ? 0 : -EINVAL;
}

static inline int __hdd_capture_tsf(struct hdd_adapter *adapter,
				    uint32_t *buf, int len)
{
	return (hdd_capture_tsf_internal(adapter, buf, len) ==
		HDD_TSF_OP_SUCC) ? 0 : -EINVAL;
}

static inline
enum hdd_tsf_op_result wlan_hdd_tsf_plus_init(struct hdd_context *hdd_ctx)
{
	return HDD_TSF_OP_SUCC;
}

static inline
enum hdd_tsf_op_result wlan_hdd_tsf_plus_deinit(struct hdd_context *hdd_ctx)
{
	return HDD_TSF_OP_SUCC;
}
#endif /* WLAN_FEATURE_TSF_PLUS */

int hdd_capture_tsf(struct hdd_adapter *adapter, uint32_t *buf, int len)
{
	return __hdd_capture_tsf(adapter, buf, len);
}

int hdd_indicate_tsf(struct hdd_adapter *adapter, uint32_t *buf, int len)
{
	return __hdd_indicate_tsf(adapter, buf, len);
}

/**
 * hdd_get_tsf_cb() - handle tsf callback
 * @pcb_cxt: pointer to the hdd_contex
 * @ptsf: pointer to struct stsf
 *
 * This function handle the event that reported by firmware at first.
 * The event contains the vdev_id, current tsf value of this vdev,
 * tsf value is 64bits, discripted in two varaible tsf_low and tsf_high.
 * These two values each is uint32.
 *
 * Return: 0 for success or non-zero negative failure code
 */
int hdd_get_tsf_cb(void *pcb_cxt, struct stsf *ptsf)
{
	struct hdd_context *hddctx;
	struct hdd_adapter *adapter;
	int status;

	if (pcb_cxt == NULL || ptsf == NULL) {
		hdd_err("HDD context is not valid");
			return -EINVAL;
	}

	hddctx = (struct hdd_context *)pcb_cxt;
	status = wlan_hdd_validate_context(hddctx);
	if (0 != status)
		return -EINVAL;

	adapter = hdd_get_adapter_by_vdev(hddctx, ptsf->vdev_id);

	if (NULL == adapter) {
		hdd_err("failed to find adapter");
		return -EINVAL;
	}

	if (!hdd_tsf_is_initialized(adapter)) {
		hdd_err("tsf is not init, ignore tsf event");
		return -EINVAL;
	}

	hdd_info("tsf cb handle event, device_mode is %d",
		adapter->device_mode);

	adapter->cur_target_time = ((uint64_t)ptsf->tsf_high << 32 |
			 ptsf->tsf_low);
	adapter->tsf_sync_soc_timer = ((uint64_t) ptsf->soc_timer_high << 32 |
						  ptsf->soc_timer_low);

	complete(&tsf_sync_get_completion_evt);
	hdd_update_tsf(adapter, adapter->cur_target_time);
	hdd_info("Vdev=%u, tsf_low=%u, tsf_high=%u soc_timer=%llu",
		ptsf->vdev_id, ptsf->tsf_low, ptsf->tsf_high,
		adapter->tsf_sync_soc_timer);
	return 0;
}

static const struct nla_policy tsf_policy[QCA_WLAN_VENDOR_ATTR_TSF_MAX + 1] = {
	[QCA_WLAN_VENDOR_ATTR_TSF_CMD] = {.type = NLA_U32},
};

/**
 * __wlan_hdd_cfg80211_handle_tsf_cmd(): Setup TSF operations
 * @wiphy: Pointer to wireless phy
 * @wdev: Pointer to wireless device
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Handle TSF SET / GET operation from userspace
 *
 * Return: 0 on success, negative errno on failure
 */
static int __wlan_hdd_cfg80211_handle_tsf_cmd(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx = wiphy_priv(wiphy);
	struct nlattr *tb_vendor[QCA_WLAN_VENDOR_ATTR_TSF_MAX + 1];
	int status, ret;
	struct sk_buff *reply_skb;
	uint32_t tsf_op_resp[3], tsf_cmd;

	ENTER_DEV(wdev->netdev);

	status = wlan_hdd_validate_context(hdd_ctx);
	if (0 != status)
		return -EINVAL;

	if (hdd_nla_parse(tb_vendor, QCA_WLAN_VENDOR_ATTR_TSF_MAX, data,
			  data_len, tsf_policy)) {
		hdd_err("Invalid TSF cmd");
		return -EINVAL;
	}

	if (!tb_vendor[QCA_WLAN_VENDOR_ATTR_TSF_CMD]) {
		hdd_err("Invalid TSF cmd");
		return -EINVAL;
	}
	tsf_cmd = nla_get_u32(tb_vendor[QCA_WLAN_VENDOR_ATTR_TSF_CMD]);

	if (tsf_cmd == QCA_TSF_CAPTURE || tsf_cmd == QCA_TSF_SYNC_GET) {
		hdd_capture_tsf(adapter, tsf_op_resp, 1);
		switch (tsf_op_resp[0]) {
		case TSF_RETURN:
			status = 0;
			break;
		case TSF_CURRENT_IN_CAP_STATE:
			status = -EALREADY;
			break;
		case TSF_STA_NOT_CONNECTED_NO_TSF:
		case TSF_SAP_NOT_STARTED_NO_TSF:
			status = -EPERM;
			break;
		default:
		case TSF_CAPTURE_FAIL:
			status = -EINVAL;
			break;
		}
	}
	if (status < 0)
		goto end;

	if (tsf_cmd == QCA_TSF_SYNC_GET) {
		ret = wait_for_completion_timeout(&tsf_sync_get_completion_evt,
			msecs_to_jiffies(WLAN_TSF_SYNC_GET_TIMEOUT));
		if (ret == 0) {
			status = -ETIMEDOUT;
			goto end;
		}
	}

	if (tsf_cmd == QCA_TSF_GET || tsf_cmd == QCA_TSF_SYNC_GET) {
		hdd_indicate_tsf(adapter, tsf_op_resp, 3);
		switch (tsf_op_resp[0]) {
		case TSF_RETURN:
			status = 0;
			break;
		case TSF_NOT_RETURNED_BY_FW:
			status = -EINPROGRESS;
			break;
		case TSF_STA_NOT_CONNECTED_NO_TSF:
		case TSF_SAP_NOT_STARTED_NO_TSF:
			status = -EPERM;
			break;
		default:
			status = -EINVAL;
			break;
		}
		if (status != 0)
			goto end;

		reply_skb = cfg80211_vendor_event_alloc(hdd_ctx->wiphy, NULL,
					sizeof(uint64_t) * 2 + NLMSG_HDRLEN,
					QCA_NL80211_VENDOR_SUBCMD_TSF_INDEX,
					GFP_KERNEL);
		if (!reply_skb) {
			hdd_err("cfg80211_vendor_cmd_alloc_reply_skb failed");
			status = -ENOMEM;
			goto end;
		}
		if (hdd_wlan_nla_put_u64(reply_skb,
				QCA_WLAN_VENDOR_ATTR_TSF_TIMER_VALUE,
				adapter->cur_target_time) ||
		    hdd_wlan_nla_put_u64(reply_skb,
				QCA_WLAN_VENDOR_ATTR_TSF_SOC_TIMER_VALUE,
				adapter->tsf_sync_soc_timer)) {
			hdd_err("nla put fail");
			kfree_skb(reply_skb);
			status = -EINVAL;
			goto end;
		}
		status = cfg80211_vendor_cmd_reply(reply_skb);
	}

end:
	hdd_info("TSF operation %d Status: %d", tsf_cmd, status);
	return status;
}

int wlan_hdd_cfg80211_handle_tsf_cmd(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_handle_tsf_cmd(wiphy, wdev, data, data_len);
	cds_ssr_unprotect(__func__);

	return ret;
}

/**
 * wlan_hdd_tsf_init() - set callback to handle tsf value.
 * @hdd_ctx: pointer to the struct hdd_context
 *
 * This function set the callback to sme module, the callback will be
 * called when a tsf event is reported by firmware
 *
 * Return: none
 */
void wlan_hdd_tsf_init(struct hdd_context *hdd_ctx)
{
	QDF_STATUS hal_status;

	if (!hdd_ctx)
		return;

	if (qdf_atomic_inc_return(&hdd_ctx->tsf_ready_flag) > 1)
		return;

	qdf_atomic_init(&hdd_ctx->cap_tsf_flag);

	if (hdd_ctx->config->tsf_gpio_pin == TSF_GPIO_PIN_INVALID)
		goto fail;

	hal_status = sme_set_tsf_gpio(hdd_ctx->hHal,
				      hdd_ctx->config->tsf_gpio_pin);
	if (QDF_STATUS_SUCCESS != hal_status) {
		hdd_err("set tsf GPIO failed, status: %d", hal_status);
		goto fail;
	}

	if (wlan_hdd_tsf_plus_init(hdd_ctx) != HDD_TSF_OP_SUCC)
		goto fail;

	return;

fail:
	qdf_atomic_set(&hdd_ctx->tsf_ready_flag, 0);
}

void wlan_hdd_tsf_deinit(struct hdd_context *hdd_ctx)
{
	if (!hdd_ctx)
		return;

	if (!qdf_atomic_read(&hdd_ctx->tsf_ready_flag))
		return;

	wlan_hdd_tsf_plus_deinit(hdd_ctx);
	qdf_atomic_set(&hdd_ctx->tsf_ready_flag, 0);
	qdf_atomic_set(&hdd_ctx->cap_tsf_flag, 0);
}
