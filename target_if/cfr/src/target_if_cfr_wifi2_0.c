/*
 * Copyright (c) 2019, 2021 The Linux Foundation. All rights reserved.
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

#include <target_if_cfr.h>
#include <wlan_tgt_def_config.h>
#include <target_type.h>
#include <ol_if_athvar.h>
#include <target_if.h>
#include <wlan_lmac_if_def.h>
#include <wlan_mlme_dispatcher.h>
#include <init_deinit_lmac.h>
#include <wlan_cfr_utils_api.h>

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <qdf_types.h>
#include <htt.h>

/* hardcoded for CFR specific Shared memory  */
#define CFR_CAPTURE_HOST_MEM_REQ_ID                 9
#define CFR_CAPTURE_HOST_MEM_DEFAULT_READ_OFFSET    8

/* max subbuf size and number for 2 chains  */
#define MAX_SUBBUF_2S                               1100
#define NUM_SUBBUF_2S                               2200

/* max subbuf size and number for 4 chains */
#define MAX_SUBBUF_4S                               2200
#define NUM_SUBBUF_4S                               1100
#define MAX_CFR_CLIENTS_LEGACY                      10

static u_int8_t *
target_if_mac_addr_deswizzle(u_int8_t *tgt_mac_addr, u_int8_t *buffer)
{
#if BIG_ENDIAN_HOST
    /*
     * The host endianness is opposite of the target endianness.
     * To make u_int32_t elements come out correctly, the target->host
     * upload has swizzled the bytes in each u_int32_t element of the
     * message.
     * For byte-array message fields like the MAC address, this
     * upload swizzling puts the bytes in the wrong order, and needs
     * to be undone.
     */
	buffer[0] = tgt_mac_addr[3];
	buffer[1] = tgt_mac_addr[2];
	buffer[2] = tgt_mac_addr[1];
	buffer[3] = tgt_mac_addr[0];
	buffer[4] = tgt_mac_addr[7];
	buffer[5] = tgt_mac_addr[6];
	return buffer;
#else
    /*
     * The host endianness matches the target endianness -
     * we can use the mac addr directly from the message buffer.
     */
	return tgt_mac_addr;
#endif
}

QDF_STATUS ol_txrx_htt_cfr_rx_ind_handler(void *pdev_ptr,
					  uint32_t *msg_word,
					  size_t msg_len)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;
	struct pdev_cfr *pa;
	u_int32_t *vaddr;
	u_int32_t msg_info, cfr_msg_type;
	u_int8_t  cfr_cap_status, cfr_vdev_id;
	u_int32_t cfr_dump_len, cfr_dump_index;
	uint32_t status, target_type;
	void *prindex = NULL;
	struct csi_cfr_header cfr_header = {0};
	u_int32_t end_magic = 0xBEAFDEAD;

	pdev = (struct wlan_objmgr_pdev *)pdev_ptr;

	status = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_CFR_ID);

	if (QDF_IS_STATUS_ERROR(status)) {
		cfr_err("unable to get reference");
		return status;
	}

	if (wlan_cfr_is_feature_disabled(pdev)) {
		cfr_err("cfr is disabled");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);
		return QDF_STATUS_E_NOSUPPORT;
	}

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (!pa) {
		cfr_err("pdev_cfr is NULL\n");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);
		return QDF_STATUS_E_FAILURE;
	}

	if (!pa->is_cfr_capable) {
		cfr_err("CFR capture not enabled/supported in Host SW\n");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);
		return QDF_STATUS_E_FAILURE;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	target_type = target_if_cfr_get_target_type(psoc);

    /* This CFR msg info (bit 8:31) should be checked for any new element
     * added in this capture type. Ex:Rssi. TODO: pass msg info,capture type
     * and msgword+3 to CFR file, to avoid additional copy?
     */

	msg_info        = *msg_word;
	cfr_msg_type    = *(msg_word + 1);

	target_if_cfr_fill_header(&cfr_header, true, target_type, false);

	switch (cfr_msg_type) {
	u_int8_t *peeraddr;

	case HTT_PEER_CFR_CAPTURE_MSG_TYPE_1:
		cfr_header.u.meta_legacy.status =
			HTT_T2H_CFR_DUMP_TYPE1_STATUS_GET(*(msg_word + 2));
		cfr_cap_status = cfr_header.u.meta_legacy.status;
		cfr_header.u.meta_legacy.capture_bw =
			HTT_T2H_CFR_DUMP_TYPE1_CAP_BW_GET(*(msg_word + 2));
		cfr_header.u.meta_legacy.capture_mode =
			HTT_T2H_CFR_DUMP_TYPE1_MODE_GET(*(msg_word + 2));
		cfr_header.u.meta_legacy.sts_count =
			HTT_T2H_CFR_DUMP_TYPE1_STS_GET(*(msg_word + 2));
		cfr_header.u.meta_legacy.channel_bw =
			HTT_T2H_CFR_DUMP_TYPE1_CHAN_BW_GET(*(msg_word + 2));
		cfr_header.u.meta_legacy.capture_type =
			HTT_T2H_CFR_DUMP_TYPE1_CAP_TYPE_GET(*(msg_word + 2));
		cfr_vdev_id = HTT_T2H_CFR_DUMP_TYPE1_VDEV_ID_GET
				(*(msg_word + 2));

		vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, cfr_vdev_id,
							    WLAN_CFR_ID);
		if (!vdev) {
			cfr_header.u.meta_legacy.num_rx_chain = 0;
		} else {
			cfr_header.u.meta_legacy.num_rx_chain =
				wlan_vdev_mlme_get_rxchainmask(vdev);
			wlan_objmgr_vdev_release_ref(vdev, WLAN_CFR_ID);
		}

		peeraddr = target_if_mac_addr_deswizzle
				((u_int8_t *)(msg_word + 3),
				 cfr_header.u.meta_legacy.peer_addr);

		memcpy(cfr_header.u.meta_legacy.peer_addr, peeraddr, 6);

		cfr_dump_index = *(msg_word + 5);
		cfr_header.u.meta_legacy.length = *(msg_word + 6);
		cfr_dump_len = cfr_header.u.meta_legacy.length;
		cfr_header.u.meta_legacy.timestamp = *(msg_word + 7);
		cfr_header.u.meta_legacy.prim20_chan = *(msg_word + 9);
		cfr_header.u.meta_legacy.center_freq1 = *(msg_word + 10);
		cfr_header.u.meta_legacy.center_freq2 = *(msg_word + 11);
		cfr_header.u.meta_legacy.phy_mode = *(msg_word + 12);
		break;

	default:
		cfr_err("Unsupported CFR capture type:%d\n", cfr_msg_type);
		wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);
		return QDF_STATUS_E_NOSUPPORT;
	}

	vaddr = pa->cfr_mem_chunk.vaddr;

	if (cfr_cap_status == 1) {
		prindex = (void *)((u_int8_t *)vaddr + cfr_dump_index);

		target_if_cfr_info_send(pdev, &cfr_header,
					sizeof(struct csi_cfr_header),
					prindex, cfr_dump_len, &end_magic, 4);

		*vaddr =  cfr_dump_index + cfr_dump_len;

	} else if (cfr_cap_status == 0) {
		target_if_cfr_info_send(pdev, &cfr_header,
					sizeof(struct csi_cfr_header),
					prindex, cfr_dump_len, &end_magic, 4);
	}

	cfr_debug("CFR: status=%d rindex=0x%pK dump_len=%d\n",
		  cfr_cap_status, prindex, cfr_dump_len);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(ol_txrx_htt_cfr_rx_ind_handler);

QDF_STATUS
cfr_wifi2_0_init_pdev(struct wlan_objmgr_psoc *psoc,
		      struct wlan_objmgr_pdev *pdev)
{
	int idx;
	u_int32_t  *read_offset;
	struct pdev_cfr *pa;
	struct target_psoc_info *tgt_hdl;
	struct tgt_info *info;
	uint32_t target_type;

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);

	if (!pa)
		return QDF_STATUS_E_FAILURE;

	target_type = target_if_cfr_get_target_type(psoc);
	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);

	if (tgt_hdl)
		info = (&tgt_hdl->info);
	else
		return QDF_STATUS_E_FAILURE;

	if (pa->is_cfr_capable) {
		for (idx = 0; idx < info->num_mem_chunks; ++idx) {
			if (info->mem_chunks[idx].req_id ==
					    CFR_CAPTURE_HOST_MEM_REQ_ID) {
				pa->cfr_mem_chunk.req_id =
					    info->mem_chunks[idx].req_id;
				pa->cfr_mem_chunk.paddr =
					    info->mem_chunks[idx].paddr;
				pa->cfr_mem_chunk.vaddr =
					    info->mem_chunks[idx].vaddr;
				pa->cfr_mem_chunk.len =
					    info->mem_chunks[idx].len;
				read_offset = pa->cfr_mem_chunk.vaddr;
				(*read_offset) =
				    CFR_CAPTURE_HOST_MEM_DEFAULT_READ_OFFSET;
				cfr_debug("CFR: reqid=%d len=%d\n",
					  pa->cfr_mem_chunk.req_id,
					  pa->cfr_mem_chunk.len);
			}

			if (idx >= info->num_mem_chunks) {
				pa->is_cfr_capable = 0;
				cfr_err("CFR Shared memory not allocated\n");
				return QDF_STATUS_E_NOMEM;
			}
		}

    /* TODO: These values need to be fine-tuned for better optimisation,
     * LOW_MEM_SUPPORT ?
     */

		if ((target_type == TARGET_TYPE_IPQ4019) ||
		    (target_type == TARGET_TYPE_QCA9888)) {
			pa->cfr_max_sta_count = MAX_CFR_CLIENTS_LEGACY;
			pa->subbuf_size = MAX_SUBBUF_2S;
			pa->num_subbufs = NUM_SUBBUF_2S;
		} else if (target_type == TARGET_TYPE_QCA9984) {
			pa->cfr_max_sta_count = MAX_CFR_CLIENTS_LEGACY;
			pa->subbuf_size = MAX_SUBBUF_4S;
			pa->num_subbufs = NUM_SUBBUF_4S;
		} else {
			return QDF_STATUS_E_NOSUPPORT;
		}

	} else {
		return QDF_STATUS_E_NOSUPPORT;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
cfr_wifi2_0_deinit_pdev(struct wlan_objmgr_psoc *psoc,
			struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pa;

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (!pa)
		return QDF_STATUS_E_FAILURE;

	pa->cfr_timer_enable = 0;

	return QDF_STATUS_SUCCESS;
}
