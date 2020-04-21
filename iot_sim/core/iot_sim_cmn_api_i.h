/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _IOT_SIM_CMN_API_I_H_
#define _IOT_SIM_CMN_API_I_H_

#include "iot_sim_defs_i.h"
#include <qdf_net_types.h>

#define MAX_BUFFER_SIZE 2048
/*
 *                   IOT SIM User Buf Format
 *
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * | FrmType/subtype |  Seq  | Offset | Length | content | Mac Addr |
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * |     1Byte       | 2Byte | 2Bytes | 2Bytes | Length  | 6 Bytes  |
 *
 */
#define USER_BUF_LEN (1 + 2 + 2 + 2 + MAX_BUFFER_SIZE + 6)
#define IOT_SIM_SET_OP_BIT(bitmap, oper) ((bitmap) |= 1 << (oper))
#define IOT_SIM_CLEAR_OP_BIT(bitmap, oper) (((bitmap) &= ~(1 << (oper))) == 0)

/**
 * wlan_iot_sim_pdev_obj_create_handler() - handler for pdev object create
 * @pdev: reference to global pdev object
 * @arg:  reference to argument provided during registration of handler
 *
 * This is a handler to indicate pdev object created. Hence iot_sim_context
 * object can be created and attached to pdev component list.
 *
 * Return: QDF_STATUS_SUCCESS on success
 *         QDF_STATUS_E_FAILURE if pdev is null
 *         QDF_STATUS_E_NOMEM on failure of iot_sim object allocation
 */
QDF_STATUS wlan_iot_sim_pdev_obj_create_handler(struct wlan_objmgr_pdev *pdev,
						void *arg);

/**
 * wlan_iot_sim_pdev_obj_destroy_handler() - handler for pdev object delete
 * @pdev: reference to global pdev object
 * @arg:  reference to argument provided during registration of handler
 *
 * This is a handler to indicate pdev object going to be deleted.
 * Hence iot_sim_context object can be detached from pdev component list.
 * Then iot_sim_context object can be deleted.
 *
 * Return: QDF_STATUS_SUCCESS on success
 *         QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS wlan_iot_sim_pdev_obj_destroy_handler(struct wlan_objmgr_pdev *pdev,
						 void *arg);

/**
 * iot_sim_get_index_for_action_frm - Provides the action frame index
 * @frm: action frame
 * @cat: action frame category
 * @act: action frame details
 *
 * Provides the simulation database index for the action frame.
 *
 * Return: QDF_STATUS_SUCCESS on success
 *         QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS iot_sim_get_index_for_action_frm(uint8_t *frm, uint8_t *cat,
					    uint8_t *act);

/**
 * iot_sim_frame_update() - Management frame update
 * @pdev: reference to global pdev object
 * @nbuf: frame buffer
 *
 * This function updates the outgoing management frame with
 * the content stored in iot_sim_context.
 *
 * Return: QDF_STATUS_SUCCESS on success
 * QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS iot_sim_frame_update(struct wlan_objmgr_pdev *pdev, qdf_nbuf_t nbuf);

/*
 * iot_sim_get_ctx_from_pdev() - API to get iot_sim context object
 *                               from pdev
 * @pdev : Reference to psoc global object
 *
 * This API used to get iot sim context object from global psoc reference.
 * Null check should be done before invoking this inline function.
 *
 * Return : Reference to iot_sim_context object
 *
 */
static inline struct iot_sim_context *
iot_sim_get_ctx_from_pdev(struct wlan_objmgr_pdev *pdev)
{
	struct iot_sim_context *isc = NULL;

	if (pdev) {
		isc = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							    WLAN_IOT_SIM_COMP);
	}

	return isc;
}

QDF_STATUS
iot_sim_delete_rule_for_mac(struct iot_sim_context *isc,
			    enum iot_sim_operations oper,
			    unsigned short seq,
			    unsigned char type,
			    unsigned char subtype,
			    struct qdf_mac_addr *mac,
			    uint8_t cat, uint8_t act, bool action);

QDF_STATUS
iot_sim_parse_user_input_content_change(struct iot_sim_context *isc,
					char *userbuf, ssize_t count,
					uint8_t *t_st, uint16_t *seq,
					uint16_t *offset, uint16_t *length,
					uint8_t **content,
					struct qdf_mac_addr *mac);
#endif /* _IOT_SIM_CMN_API_I_H_ */
