/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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

#include "qdf_module.h"
#include "qdf_trace.h"
#include "qdf_platform.h"

/**
 * The following callbacks should be defined static to make sure they are
 * initialized to NULL
 */
static qdf_self_recovery_callback	self_recovery_cb;
static qdf_is_fw_down_callback		is_fw_down_cb;
static qdf_is_driver_unloading_callback is_driver_unloading_cb;
static qdf_is_recovering_callback	is_recovering_cb;
static qdf_is_drv_connected_callback    is_drv_connected_cb;
static qdf_wmi_send_over_qmi_callback _wmi_send_recv_qmi_cb;
static qdf_is_drv_supported_callback    is_drv_supported_cb;

void qdf_register_fw_down_callback(qdf_is_fw_down_callback is_fw_down)
{
	is_fw_down_cb = is_fw_down;
}

qdf_export_symbol(qdf_register_fw_down_callback);

bool qdf_is_fw_down(void)
{
	if (!is_fw_down_cb) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			"fw down callback is not registered");
			return false;
	}

	return is_fw_down_cb();
}
qdf_export_symbol(qdf_is_fw_down);

void qdf_register_wmi_send_recv_qmi_callback(qdf_wmi_send_over_qmi_callback
					     wmi_send_recv_qmi_cb)
{
	_wmi_send_recv_qmi_cb = wmi_send_recv_qmi_cb;
}

qdf_export_symbol(qdf_register_wmi_send_recv_qmi_callback);

QDF_STATUS qdf_wmi_send_recv_qmi(void *buf, uint32_t len, void *cb_ctx,
				 qdf_wmi_recv_qmi_cb wmi_recv_qmi_cb)
{
	if (!_wmi_send_recv_qmi_cb) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "Platform callback for WMI over QMI not registered");
			return QDF_STATUS_E_INVAL;
	}

	return _wmi_send_recv_qmi_cb(buf, len, cb_ctx, wmi_recv_qmi_cb);
}

qdf_export_symbol(qdf_wmi_send_recv_qmi);

void qdf_register_is_driver_unloading_callback(
				qdf_is_driver_unloading_callback callback)
{
	is_driver_unloading_cb = callback;
}

qdf_export_symbol(qdf_register_is_driver_unloading_callback);

void qdf_register_self_recovery_callback(qdf_self_recovery_callback callback)
{
	self_recovery_cb = callback;
}

qdf_export_symbol(qdf_register_self_recovery_callback);

void __qdf_trigger_self_recovery(void *psoc, enum qdf_hang_reason reason,
				 const char *func, const uint32_t line)
{
	if (self_recovery_cb)
		self_recovery_cb(psoc, reason, func, line);
	else
		QDF_DEBUG_PANIC_FL(func, line, "");
}

qdf_export_symbol(__qdf_trigger_self_recovery);

void qdf_register_recovering_state_query_callback(
			qdf_is_recovering_callback is_recovering)
{
	is_recovering_cb = is_recovering;
}

bool qdf_is_driver_unloading(void)
{
	if (is_driver_unloading_cb)
		return is_driver_unloading_cb();
	return false;
}

qdf_export_symbol(qdf_is_driver_unloading);

bool qdf_is_recovering(void)
{
	if (is_recovering_cb)
		return is_recovering_cb();
	return false;
}

qdf_export_symbol(qdf_is_recovering);

static qdf_op_protect_cb __on_op_protect;
static qdf_op_unprotect_cb __on_op_unprotect;

void qdf_op_callbacks_register(qdf_op_protect_cb on_protect,
			       qdf_op_unprotect_cb on_unprotect)
{
	__on_op_protect = on_protect;
	__on_op_unprotect = on_unprotect;
}
qdf_export_symbol(qdf_op_callbacks_register);

int __qdf_op_protect(struct qdf_op_sync **out_sync, const char *func)
{
	if (!__on_op_protect)
		return 0;

	return __on_op_protect((void **)out_sync, func);
}
qdf_export_symbol(__qdf_op_protect);

void __qdf_op_unprotect(struct qdf_op_sync *sync, const char *func)
{
	if (__on_op_unprotect)
		__on_op_unprotect(sync, func);
}
qdf_export_symbol(__qdf_op_unprotect);

void qdf_register_drv_connected_callback(qdf_is_drv_connected_callback
					 is_drv_connected)
{
	is_drv_connected_cb = is_drv_connected;
}
qdf_export_symbol(qdf_register_drv_connected_callback);

bool qdf_is_drv_connected(void)
{
	if (!is_drv_connected_cb) {
		qdf_err("drv connected callback is not registered");
		return false;
	}

	return is_drv_connected_cb();
}
qdf_export_symbol(qdf_is_drv_connected);

void qdf_check_state_before_panic(void)
{
	if (!qdf_is_recovering() && !qdf_is_fw_down())
		QDF_BUG(0);
}

qdf_export_symbol(qdf_check_state_before_panic);

void qdf_register_drv_supported_callback(qdf_is_drv_supported_callback
					 is_drv_supported)
{
	is_drv_supported_cb = is_drv_supported;
}

qdf_export_symbol(qdf_register_drv_supported_callback);

bool qdf_is_drv_supported(void)
{
	if (!is_drv_supported_cb) {
		qdf_err("drv supported callback is not registered");
		return false;
	}

	return is_drv_supported_cb();
}

qdf_export_symbol(qdf_is_drv_supported);
