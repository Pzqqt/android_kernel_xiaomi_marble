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
/*
 *		IOT SIM User Buf Format for Drop
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * |FrmType/subtype| Seq |category|action| drop |MacAddr|
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * |   2Characters |2char| 2chars |2chars| 1char|17chars|
 *
 */
#define USER_BUF_LEN_DROP (2 + 2 + 2 + 2 + 1 + 17)
/*
 *		IOT SIM User Buf Format for Drop
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * |FrmType/subtype| Seq |category|action| delay |MacAddr|
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * |   2Characters |2char| 2chars |2chars| 4char|17chars|
 *
 */
#define USER_BUF_LEN_DELAY (2 + 2 + 2 + 2 + 4 + 17)

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
 * @rx: TRUE if its getting called in the rx path
 *
 * Provides the simulation database index for the action frame.
 *
 * Return: QDF_STATUS_SUCCESS on success
 *         QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS iot_sim_get_index_for_action_frm(uint8_t *frm, uint8_t *cat,
					    uint8_t *act, bool rx);

/**
 * iot_sim_find_peer_from_mac - function to find the iot sim peer data
 *                              based on the mac address provided
 *
 * @isc: iot_sim pdev private object
 * @mac: mac address of the peer
 * Return: iot_sim_rule_per_peer reference if exists else NULL
 */
struct iot_sim_rule_per_peer *
iot_sim_find_peer_from_mac(struct iot_sim_context *isc,
			   struct qdf_mac_addr *mac);

/**
 * iot_sim_frame_update() - Management frame update
 * @pdev: reference to global pdev object
 * @nbuf: frame buffer
 * @tx: TRUE in case of tx
 * @rx_param: mgmt_rx_event_params
 *
 * This function updates the outgoing management frame with
 * the content stored in iot_sim_context.
 *
 * Return: QDF_STATUS_SUCCESS on success
 * QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS iot_sim_frame_update(struct wlan_objmgr_pdev *pdev,
				qdf_nbuf_t nbuf,
				struct beacon_tmpl_params *param,
				bool tx,
				struct mgmt_rx_event_params *rx_param);

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

/*
 * iot_sim_delete_rule_for_mac - function to delete content change rule
 *                               for given peer mac
 * @isc: iot sim context
 * @oper: iot sim operation
 * @seq: authentication sequence number, mostly 0 for non-authentication frame
 * @type: 802.11 frame type
 * @subtype: 802.11 frame subtype
 * @mac: peer mac address
 * @action: action frame or not
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE otherwise
 */
QDF_STATUS
iot_sim_delete_rule_for_mac(struct iot_sim_context *isc,
			    enum iot_sim_operations oper,
			    uint16_t seq, uint8_t type,
			    uint8_t subtype,
			    struct qdf_mac_addr *mac,
			    bool action);
/*
 * iot_sim_parse_user_input_content_change - function to parse user input into
 *					     predefined format for content
 *					     change operation. All arguments
 *					     passed will be filled upon success
 * @isc: iot sim context
 * @userbuf: local copy of user input
 * @count: length of userbuf
 * @t_st: address of type variable
 * @seq: address of seq variable
 * @offset: address of offset variable
 * @length: address of length variable
 * @content: double pointer to storage to store frame content after processing
 * @mac: pointer to mac address
 *
 * Return: QDF_STATUS_SUCCESS on success
 *	   QDF_STATUS_E_FAILURE otherwise
 */
QDF_STATUS
iot_sim_parse_user_input_content_change(struct iot_sim_context *isc,
					char *userbuf, ssize_t count,
					uint8_t *t_st, uint16_t *seq,
					uint16_t *offset, uint16_t *length,
					uint8_t **content,
					struct qdf_mac_addr *mac);
#endif /* _IOT_SIM_CMN_API_I_H_ */
