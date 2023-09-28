
/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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
/**
 * DOC: Implement API's specific to NAN component.
 */

#ifndef _WMI_UNIFIED_NAN_API_H_
#define _WMI_UNIFIED_NAN_API_H_

#include <nan_public_structs.h>

/**
 * wmi_unified_nan_req_cmd() - to send nan request to target
 * @wmi_handle: wmi handle
 * @nan_req: request data which will be non-null
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_nan_req_cmd(wmi_unified_t wmi_handle,
				   struct nan_msg_params *nan_req);

/**
 * wmi_unified_nan_disable_req_cmd() - to send nan disable request to target
 * @wmi_handle: wmi handle
 * @nan_req: pointer to NAN Disable request structure
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF_STATUS_E_FAILURE for failure
 */
QDF_STATUS wmi_unified_nan_disable_req_cmd(wmi_unified_t wmi_handle,
					   struct nan_disable_req *nan_req);

/**
 * wmi_unified_ndp_initiator_req_cmd_send - api to send initiator request to FW
 * @wmi_handle: wmi handle
 * @req: pointer to request buffer
 *
 * Return: status of operation
 */
QDF_STATUS
wmi_unified_ndp_initiator_req_cmd_send(wmi_unified_t wmi_handle,
				       struct nan_datapath_initiator_req *req);

/**
 * wmi_unified_ndp_responder_req_cmd_send - api to send responder request to FW
 * @wmi_handle: wmi handle
 * @req: pointer to request buffer
 *
 * Return: status of operation
 */
QDF_STATUS
wmi_unified_ndp_responder_req_cmd_send(wmi_unified_t wmi_handle,
				       struct nan_datapath_responder_req *req);

/**
 * wmi_unified_ndp_end_req_cmd_send - api to send end request to FW
 * @wmi_handle: wmi handle
 * @req: pointer to request buffer
 *
 * Return: status of operation
 */
QDF_STATUS wmi_unified_ndp_end_req_cmd_send(wmi_unified_t wmi_handle,
					    struct nan_datapath_end_req *req);

/**
 * wmi_unified_terminate_all_ndps_req_cmd - api to request Firmware for
 * termination of all NDP's associated with the given vdev id.
 * @wmi_handle: wmi handle
 * @vdev_id: vdev id
 *
 * Return: status of operation
 */
QDF_STATUS wmi_unified_terminate_all_ndps_req_cmd(wmi_unified_t wmi_handle,
						  uint32_t vdev_id);

/**
 * wmi_extract_ndp_initiator_rsp - api to extract initiator rsp from even buffer
 * @wmi_hdl: wmi handle
 * @data: event buffer
 * @rsp: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS
wmi_extract_ndp_initiator_rsp(wmi_unified_t wmi_handle, uint8_t *data,
			      struct nan_datapath_initiator_rsp *rsp);

/**
 * wmi_extract_ndp_ind - api to extract ndp indication struct from even buffer
 * @wmi_hdl: wmi handle
 * @data: event buffer
 * @ind: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_ndp_ind(wmi_unified_t wmi_handle, uint8_t *data,
			       struct nan_datapath_indication_event *ind);

/**
 * wmi_extract_nan_msg - api to extract ndp dmesg buffer to print logs
 * @data: event buffer
 * @msg: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_nan_msg(wmi_unified_t wmi_handle, uint8_t *data,
			       struct nan_dump_msg *msg);

/**
 * wmi_extract_ndp_confirm - api to extract ndp confim struct from even buffer
 * @wmi_hdl: wmi handle
 * @data: event buffer
 * @ev: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_ndp_confirm(wmi_unified_t wmi_handle, uint8_t *data,
				   struct nan_datapath_confirm_event *ev);

/**
 * wmi_extract_ndp_responder_rsp - api to extract responder rsp from even buffer
 * @wmi_hdl: wmi handle
 * @data: event buffer
 * @rsp: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS
wmi_extract_ndp_responder_rsp(wmi_unified_t wmi_handle, uint8_t *data,
			      struct nan_datapath_responder_rsp *rsp);

/**
 * wmi_extract_ndp_end_rsp - api to extract ndp end rsp from even buffer
 * @wmi_hdl: wmi handle
 * @data: event buffer
 * @rsp: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_ndp_end_rsp(wmi_unified_t wmi_handle, uint8_t *data,
				   struct nan_datapath_end_rsp_event *rsp);

/**
 * wmi_extract_ndp_end_ind - api to extract ndp end indication from even buffer
 * @wmi_hdl: wmi handle
 * @data: event buffer
 * @ind: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS
wmi_extract_ndp_end_ind(wmi_unified_t wmi_handle, uint8_t *data,
			struct nan_datapath_end_indication_event **ind);

/**
 * wmi_extract_ndp_sch_update - api to extract ndp sch update from event buffer
 * @wmi_hdl: wmi handle
 * @data: event buffer
 * @ind: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS
wmi_extract_ndp_sch_update(wmi_unified_t wmi_handle, uint8_t *data,
			   struct nan_datapath_sch_update_event *ind);

/**
 * wmi_extract_nan_event_rsp - api to extract nan event into event parameters
 * @wmi_hdl: wmi handle
 * @wvt_buf: pointer to the event buffer
 * @temp_evt_params: Pointer to a temporary parameters structure to populate
 * @nan_msg_buf: Pointer to the NAN Message buffer encapsulated in the event
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_nan_event_rsp(wmi_unified_t wmi_handle, void *evt_buf,
				     struct nan_event_params *temp_evt_params,
				     uint8_t **nan_msg_buf);

/**
 * wmi_extract_ndp_host_event - api to extract ndp event from event buffer
 * @wmi_hdl: wmi handle
 * @data: event buffer
 * @evt: event buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS wmi_extract_ndp_host_event(wmi_unified_t wmi_handle, uint8_t *data,
				      struct nan_datapath_host_event *evt);
#endif /* _WMI_UNIFIED_NAN_API_H_ */
