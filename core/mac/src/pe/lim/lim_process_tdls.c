/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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

/*===========================================================================
 * lim_process_tdls.c
 * OVERVIEW:
 *
 * DEPENDENCIES:
 *
 * Are listed for each API below.
 * ===========================================================================*/

/*===========================================================================

 *                      EDIT HISTORY FOR FILE

 *  This section contains comments describing changes made to the module.
 *  Notice that changes are listed in reverse chronological order.

 *  $Header$$DateTime$$Author$

 *  when        who     what, where, why
 *  ----------    ---    ------------------------------------------------------
 *  05/05/2010   Ashwani    Initial Creation, added TDLS action frame
 *  functionality,TDLS message exchange with SME..etc..

   ===========================================================================*/

/**
 * \file lim_process_tdls.c
 *
 * \brief Code for preparing,processing and sending 802.11z action frames
 *
 */

#ifdef FEATURE_WLAN_TDLS

#include "sir_api.h"
#include "ani_global.h"
#include "sir_mac_prot_def.h"
#include "cfg_api.h"
#include "utils_api.h"
#include "lim_types.h"
#include "lim_utils.h"
#include "lim_security_utils.h"
#include "dot11f.h"
#include "lim_sta_hash_api.h"
#include "sch_api.h"
#include "lim_send_messages.h"
#include "utils_parser.h"
#include "lim_assoc_utils.h"
#include "lim_prop_exts_utils.h"
#include "dph_hash_table.h"
#include "wma_types.h"
#include "cds_regdomain.h"
#include "cds_utils.h"
#include "wlan_reg_services_api.h"

#ifdef CONVERGED_TDLS_ENABLE
#include "wlan_tdls_tgt_api.h"
#endif

/* define NO_PAD_TDLS_MIN_8023_SIZE to NOT padding: See CR#447630
   There was IOT issue with cisco 1252 open mode, where it pads
   discovery req/teardown frame with some junk value up to min size.
   To avoid this issue, we pad QCOM_VENDOR_IE.
   If there is other IOT issue because of this bandage, define NO_PAD...
 */
#ifndef NO_PAD_TDLS_MIN_8023_SIZE
#define MIN_IEEE_8023_SIZE              46
#define MIN_VENDOR_SPECIFIC_IE_SIZE     5
#endif

static tSirRetStatus lim_tdls_setup_add_sta(tpAniSirGlobal pMac,
		tSirTdlsAddStaReq * pAddStaReq, tpPESession psessionEntry);

/*
 * TDLS data frames will go out/come in as non-qos data.
 * so, eth_890d_header will be aligned access..
 */
static const uint8_t eth_890d_header[] = {
	0xaa, 0xaa, 0x03, 0x00,
	0x00, 0x00, 0x89, 0x0d,
};

/*
 * type of links used in TDLS
 */
enum tdlsLinks {
	TDLS_LINK_AP,
	TDLS_LINK_DIRECT
} e_tdls_link;

/*
 * node status in node searching
 */
enum tdlsLinkNodeStatus {
	TDLS_NODE_NOT_FOUND,
	TDLS_NODE_FOUND
} e_tdls_link_node_status;

enum tdlsReqType {
	TDLS_INITIATOR,
	TDLS_RESPONDER
} e_tdls_req_type;

typedef enum tdlsLinkSetupStatus {
	TDLS_SETUP_STATUS_SUCCESS = 0,
	TDLS_SETUP_STATUS_FAILURE = 37
} etdlsLinkSetupStatus;

/* These maps to Kernel TDLS peer capability
 * flags and should get changed as and when necessary
 */
enum tdls_peer_capability {
	TDLS_PEER_HT_CAP = 0,
	TDLS_PEER_VHT_CAP = 1,
	TDLS_PEER_WMM_CAP = 2
} e_tdls_peer_capability;

/* some local defines */
#define LINK_IDEN_ADDR_OFFSET(x) (&x.LinkIdentifier)
#define PTI_LINK_IDEN_OFFSET     (5)
#define PTI_BUF_STATUS_OFFSET    (25)

/* TODO, Move this parameters to configuration */
#define PEER_PSM_SUPPORT          (0)
#define TDLS_SUPPORT              (1)
#define TDLS_PROHIBITED           (0)
#define TDLS_CH_SWITCH_PROHIBITED (1)
/** @brief Set bit manipulation macro */
#define SET_BIT(value, mask)       ((value) |= (1 << (mask)))
/** @brief Clear bit manipulation macro */
#define CLEAR_BIT(value, mask)     ((value) &= ~(1 << (mask)))
/** @brief Check bit manipulation macro */
#define CHECK_BIT(value, mask)    ((value) & (1 << (mask)))

#define SET_PEER_AID_BITMAP(peer_bitmap, aid) \
	do { \
	if ((aid) < (sizeof(uint32_t) << 3)) \
		SET_BIT(peer_bitmap[0], (aid));	\
	else if ((aid) < (sizeof(uint32_t) << 4)) \
		SET_BIT(peer_bitmap[1], ((aid) - (sizeof(uint32_t) << 3)));\
	} while (0);

#define CLEAR_PEER_AID_BITMAP(peer_bitmap, aid)	\
	do { \
	if ((aid) < (sizeof(uint32_t) << 3)) \
		CLEAR_BIT(peer_bitmap[0], (aid)); \
	else if ((aid) < (sizeof(uint32_t) << 4)) \
		CLEAR_BIT(peer_bitmap[1], ((aid) - (sizeof(uint32_t) << 3)));\
	} while (0);

#ifdef FEATURE_WLAN_TDLS
#define WNI_CFG_TDLS_LINK_SETUP_RSP_TIMEOUT         (800)
#define WNI_CFG_TDLS_LINK_SETUP_CNF_TIMEOUT         (200)
#endif

#define IS_QOS_ENABLED(psessionEntry) ((((psessionEntry)->limQosEnabled) && \
					SIR_MAC_GET_QOS((psessionEntry)->limCurrentBssCaps)) ||	\
				       (((psessionEntry)->limWmeEnabled) && \
					LIM_BSS_CAPS_GET(WME, (psessionEntry)->limCurrentBssQosCaps)))

#define TID_AC_VI                  4
#define TID_AC_BK                  1

static const uint8_t *lim_trace_tdls_action_string(uint8_t tdlsActionCode)
{
	switch (tdlsActionCode) {
		CASE_RETURN_STRING(SIR_MAC_TDLS_SETUP_REQ);
		CASE_RETURN_STRING(SIR_MAC_TDLS_SETUP_RSP);
		CASE_RETURN_STRING(SIR_MAC_TDLS_SETUP_CNF);
		CASE_RETURN_STRING(SIR_MAC_TDLS_TEARDOWN);
		CASE_RETURN_STRING(SIR_MAC_TDLS_PEER_TRAFFIC_IND);
		CASE_RETURN_STRING(SIR_MAC_TDLS_CH_SWITCH_REQ);
		CASE_RETURN_STRING(SIR_MAC_TDLS_CH_SWITCH_RSP);
		CASE_RETURN_STRING(SIR_MAC_TDLS_PEER_TRAFFIC_RSP);
		CASE_RETURN_STRING(SIR_MAC_TDLS_DIS_REQ);
		CASE_RETURN_STRING(SIR_MAC_TDLS_DIS_RSP);
	}
	return (const uint8_t *)"UNKNOWN";
}

/*
 * initialize TDLS setup list and related data structures.
 */
void lim_init_tdls_data(tpAniSirGlobal pMac, tpPESession pSessionEntry)
{
	lim_init_peer_idxpool(pMac, pSessionEntry);

	return;
}

static void populate_dot11f_tdls_offchannel_params(
				tpAniSirGlobal pMac,
				tpPESession psessionEntry,
				tDot11fIESuppChannels *suppChannels,
				tDot11fIESuppOperatingClasses *suppOperClasses)
{
	uint32_t numChans = WNI_CFG_VALID_CHANNEL_LIST_LEN;
	uint8_t validChan[WNI_CFG_VALID_CHANNEL_LIST_LEN];
	uint8_t i;
	uint8_t valid_count = 0;
	uint8_t chanOffset;
	uint8_t op_class;
	uint8_t numClasses;
	uint8_t classes[REG_MAX_SUPP_OPER_CLASSES];
	uint32_t band;
	uint8_t nss_2g;
	uint8_t nss_5g;

	if (wlan_cfg_get_str(pMac, WNI_CFG_VALID_CHANNEL_LIST,
			     validChan, &numChans) != eSIR_SUCCESS) {
		/**
		 * Could not get Valid channel list from CFG.
		 * Log error.
		 */
		pe_err("could not retrieve Valid channel list");
		return;
	}

	if (IS_5G_CH(psessionEntry->currentOperChannel))
		band = eCSR_BAND_5G;
	else
		band = eCSR_BAND_24;

	nss_5g = QDF_MIN(pMac->vdev_type_nss_5g.tdls,
			 pMac->user_configured_nss);
	nss_2g = QDF_MIN(pMac->vdev_type_nss_2g.tdls,
			 pMac->user_configured_nss);

	/* validating the channel list for DFS and 2G channels */
	for (i = 0U; i < numChans; i++) {
		if ((band == eCSR_BAND_5G) &&
		    (NSS_2x2_MODE == nss_5g) &&
		    (NSS_1x1_MODE == nss_2g) &&
		    (wlan_reg_is_dfs_ch(pMac->pdev, validChan[i]))) {
			pe_debug("skipping channel: %d, nss_5g: %d, nss_2g: %d",
				validChan[i], nss_5g, nss_2g);
			continue;
		} else {
			if (WLAN_REG_IS_11P_CH(validChan[i])) {
				pe_debug("skipping channel: %d from the valid channel list",
					validChan[i]);
				continue;
			}
		}

		if (valid_count >= ARRAY_SIZE(suppChannels->bands))
			 break;
		suppChannels->bands[valid_count][0] = validChan[i];
		suppChannels->bands[valid_count][1] = 1;
		valid_count++;
	}

	suppChannels->num_bands = valid_count;
	suppChannels->present = 1;

	/* find channel offset and get op class for current operating channel */
	switch (psessionEntry->htSecondaryChannelOffset) {
	case PHY_SINGLE_CHANNEL_CENTERED:
		chanOffset = BW20;
		break;

	case PHY_DOUBLE_CHANNEL_LOW_PRIMARY:
		chanOffset = BW40_LOW_PRIMARY;
		break;

	case PHY_DOUBLE_CHANNEL_HIGH_PRIMARY:
		chanOffset = BW40_HIGH_PRIMARY;
		break;

	default:
		chanOffset = BWALL;
		break;
	}

	op_class = wlan_reg_dmn_get_opclass_from_channel(
		pMac->scan.countryCodeCurrent,
		psessionEntry->currentOperChannel,
		chanOffset);

	pe_debug("countryCodeCurrent: %s, currentOperChannel: %d, htSecondaryChannelOffset: %d, chanOffset: %d op class: %d",
			pMac->scan.countryCodeCurrent,
			psessionEntry->currentOperChannel,
			psessionEntry->htSecondaryChannelOffset,
			chanOffset, op_class);
	suppOperClasses->present = 1;
	suppOperClasses->classes[0] = op_class;

	wlan_reg_dmn_get_curr_opclasses(&numClasses, &classes[0]);

	for (i = 0; i < numClasses; i++)
		suppOperClasses->classes[i + 1] = classes[i];

	/* add one for present operating class, added in the beginning */
	suppOperClasses->num_classes = numClasses + 1;

	return;
}

/*
 * FUNCTION: Populate Link Identifier element IE
 *
 */

static void populate_dot11f_link_iden(tpAniSirGlobal pMac,
				      tpPESession psessionEntry,
				      tDot11fIELinkIdentifier *linkIden,
				      struct qdf_mac_addr peer_mac,
				      uint8_t reqType)
{
	uint8_t *initStaAddr = NULL;
	uint8_t *respStaAddr = NULL;

	(reqType == TDLS_INITIATOR) ? ((initStaAddr = linkIden->InitStaAddr),
				       (respStaAddr = linkIden->RespStaAddr))
	: ((respStaAddr = linkIden->InitStaAddr),
	   (initStaAddr = linkIden->RespStaAddr));
	qdf_mem_copy((uint8_t *) linkIden->bssid,
		     (uint8_t *) psessionEntry->bssId, QDF_MAC_ADDR_SIZE);

	qdf_mem_copy((uint8_t *) initStaAddr,
		     psessionEntry->selfMacAddr, QDF_MAC_ADDR_SIZE);

	qdf_mem_copy((uint8_t *) respStaAddr, (uint8_t *) peer_mac.bytes,
		     QDF_MAC_ADDR_SIZE);

	linkIden->present = 1;
	return;

}

static void populate_dot11f_tdls_ext_capability(tpAniSirGlobal pMac,
						tpPESession psessionEntry,
						tDot11fIEExtCap *extCapability)
{
	struct s_ext_cap *p_ext_cap = (struct s_ext_cap *)extCapability->bytes;

	p_ext_cap->tdls_peer_psm_supp = PEER_PSM_SUPPORT;
	p_ext_cap->tdls_peer_uapsd_buffer_sta = pMac->lim.gLimTDLSBufStaEnabled;

	/*
	 * Set TDLS channel switching bits only if offchannel is enabled
	 * and TDLS Channel Switching is not prohibited by AP in ExtCap
	 * IE in assoc/re-assoc response.
	 */
	if ((1 == pMac->lim.gLimTDLSOffChannelEnabled) &&
	    (!psessionEntry->tdls_chan_swit_prohibited)) {
		p_ext_cap->tdls_channel_switching = 1;
		p_ext_cap->tdls_chan_swit_prohibited = 0;
	} else {
	    p_ext_cap->tdls_channel_switching = 0;
	    p_ext_cap->tdls_chan_swit_prohibited = TDLS_CH_SWITCH_PROHIBITED;
	}
	p_ext_cap->tdls_support = TDLS_SUPPORT;
	p_ext_cap->tdls_prohibited = TDLS_PROHIBITED;

	extCapability->present = 1;
	extCapability->num_bytes = lim_compute_ext_cap_ie_length(extCapability);

	return;
}

/*
 * prepare TDLS frame header, it includes
 * |             |              |                |
 * |802.11 header|RFC1042 header|TDLS_PYLOAD_TYPE|PAYLOAD
 * |             |              |                |
 */
static uint32_t lim_prepare_tdls_frame_header(tpAniSirGlobal pMac, uint8_t *pFrame,
					      tDot11fIELinkIdentifier *link_iden,
					      uint8_t tdlsLinkType, uint8_t reqType,
					      uint8_t tid,
					      tpPESession psessionEntry)
{
	tpSirMacDataHdr3a pMacHdr;
	uint32_t header_offset = 0;
	uint8_t *addr1 = NULL;
	uint8_t *addr3 = NULL;
	uint8_t toDs = (tdlsLinkType == TDLS_LINK_AP)
		       ? ANI_TXDIR_TODS : ANI_TXDIR_IBSS;
	uint8_t *peerMac = (reqType == TDLS_INITIATOR)
			   ? link_iden->RespStaAddr : link_iden->InitStaAddr;
	uint8_t *staMac = (reqType == TDLS_INITIATOR)
			  ? link_iden->InitStaAddr : link_iden->RespStaAddr;
	tpDphHashNode sta_ds;
	uint16_t aid = 0;
	uint8_t qos_mode = 0;

	pMacHdr = (tpSirMacDataHdr3a) (pFrame);

	/*
	 * if TDLS frame goes through the AP link, it follows normal address
	 * pattern, if TDLS frame goes thorugh the direct link, then
	 * A1--> Peer STA addr, A2-->Self STA address, A3--> BSSID
	 */
	(tdlsLinkType == TDLS_LINK_AP) ? ((addr1 = (link_iden->bssid)),
					  (addr3 = (peerMac)))
	: ((addr1 = (peerMac)), (addr3 = (link_iden->bssid)));
	/*
	 * prepare 802.11 header
	 */
	pMacHdr->fc.protVer = SIR_MAC_PROTOCOL_VERSION;
	pMacHdr->fc.type = SIR_MAC_DATA_FRAME;

	sta_ds = dph_lookup_hash_entry(pMac, peerMac, &aid,
					&psessionEntry->dph.dphHashTable);
	if (sta_ds)
		qos_mode = sta_ds->qosMode;

	pMacHdr->fc.subType =
		((IS_QOS_ENABLED(psessionEntry) &&
		(tdlsLinkType == TDLS_LINK_AP)) ||
		((tdlsLinkType == TDLS_LINK_DIRECT) && qos_mode))
		? SIR_MAC_DATA_QOS_DATA : SIR_MAC_DATA_DATA;

	/*
	 * TL is not setting up below fields, so we are doing it here
	 */
	pMacHdr->fc.toDS = toDs;
	pMacHdr->fc.powerMgmt = 0;
	pMacHdr->fc.wep = (psessionEntry->encryptType == eSIR_ED_NONE) ? 0 : 1;

	qdf_mem_copy((uint8_t *) pMacHdr->addr1,
		     (uint8_t *) addr1, sizeof(tSirMacAddr));
	qdf_mem_copy((uint8_t *) pMacHdr->addr2,
		     (uint8_t *) staMac, sizeof(tSirMacAddr));

	qdf_mem_copy((uint8_t *) pMacHdr->addr3,
		     (uint8_t *) (addr3), sizeof(tSirMacAddr));

	pe_debug("Preparing TDLS frame header to %s A1:"
		   MAC_ADDRESS_STR", A2:"MAC_ADDRESS_STR", A3:"
		   MAC_ADDRESS_STR,
		(tdlsLinkType == TDLS_LINK_AP) ? "AP" : "DIRECT",
		MAC_ADDR_ARRAY(pMacHdr->addr1),
		MAC_ADDR_ARRAY(pMacHdr->addr2),
		MAC_ADDR_ARRAY(pMacHdr->addr3));

	if (pMacHdr->fc.subType == SIR_MAC_DATA_QOS_DATA) {
		pMacHdr->qosControl.tid = tid;
		header_offset += sizeof(tSirMacDataHdr3a);
	} else
		header_offset += sizeof(tSirMacMgmtHdr);

	/*
	 * Now form RFC1042 header
	 */
	qdf_mem_copy((uint8_t *) (pFrame + header_offset),
		     (uint8_t *) eth_890d_header, sizeof(eth_890d_header));

	header_offset += sizeof(eth_890d_header);

	/* add payload type as TDLS */
	*(pFrame + header_offset) = PAYLOAD_TYPE_TDLS;
	header_offset += PAYLOAD_TYPE_TDLS_SIZE;
	return header_offset;
}

/**
 * lim_mgmt_tdls_tx_complete - callback to indicate Tx completion
 * @context: pointer to mac structure
 * @buf: buffer
 * @tx_complete: indicates tx sucess/failure
 * @params: tx completion params
 *
 * function will be invoked on receiving tx completion indication
 *
 * return: success: eHAL_STATUS_SUCCESS failure: eHAL_STATUS_FAILURE
 */
static QDF_STATUS lim_mgmt_tdls_tx_complete(void *context,
					    qdf_nbuf_t buf,
					    uint32_t tx_complete,
					    void *params)
{
	tpAniSirGlobal mac_ctx = (tpAniSirGlobal)context;

	pe_debug("tdls_frm_session_id: %x tx_complete: %x",
		mac_ctx->lim.tdls_frm_session_id, tx_complete);

	if (NO_SESSION != mac_ctx->lim.tdls_frm_session_id) {
		lim_send_sme_mgmt_tx_completion(mac_ctx,
				mac_ctx->lim.tdls_frm_session_id,
				tx_complete);
		mac_ctx->lim.tdls_frm_session_id = NO_SESSION;
	}

	if (buf)
		qdf_nbuf_free(buf);

	return QDF_STATUS_SUCCESS;
}

/*
 * This function can be used for bacst or unicast discovery request
 * We are not differentiating it here, it will all depnds on peer MAC address,
 */
static tSirRetStatus lim_send_tdls_dis_req_frame(tpAniSirGlobal pMac,
						 struct qdf_mac_addr peer_mac,
						 uint8_t dialog,
						 tpPESession psessionEntry)
{
	tDot11fTDLSDisReq tdlsDisReq;
	uint32_t status = 0;
	uint32_t nPayload = 0;
	uint32_t size = 0;
	uint32_t nBytes = 0;
	uint32_t header_offset = 0;
	uint8_t *pFrame;
	void *pPacket;
	QDF_STATUS qdf_status;
#ifndef NO_PAD_TDLS_MIN_8023_SIZE
	uint32_t padLen = 0;
#endif
	uint8_t smeSessionId = 0;

	if (NULL == psessionEntry) {
		pe_err("psessionEntry is NULL");
		return eSIR_FAILURE;
	}
	smeSessionId = psessionEntry->smeSessionId;
	/*
	 * The scheme here is to fill out a 'tDot11fProbeRequest' structure
	 * and then hand it off to 'dot11f_pack_probe_request' (for
	 * serialization).  We start by zero-initializing the structure:
	 */
	qdf_mem_set((uint8_t *) &tdlsDisReq, sizeof(tDot11fTDLSDisReq), 0);

	/*
	 * setup Fixed fields,
	 */
	tdlsDisReq.Category.category = SIR_MAC_ACTION_TDLS;
	tdlsDisReq.Action.action = SIR_MAC_TDLS_DIS_REQ;
	tdlsDisReq.DialogToken.token = dialog;

	size = sizeof(tSirMacAddr);

	populate_dot11f_link_iden(pMac, psessionEntry, &tdlsDisReq.LinkIdentifier,
				  peer_mac, TDLS_INITIATOR);

	/*
	 * now we pack it.  First, how much space are we going to need?
	 */
	status = dot11f_get_packed_tdls_dis_req_size(pMac, &tdlsDisReq, &nPayload);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to calculate the packed size for a discovery Request (0x%08x)",
			status);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fTDLSDisReq);
	} else if (DOT11F_WARNED(status)) {
		pe_warn("There were warnings while calculating the packed size for a discovery Request (0x%08x)",
			status);
	}

	/*
	 * This frame is going out from PE as data frames with special ethertype
	 * 89-0d.
	 * 8 bytes of RFC 1042 header
	 */

	nBytes = nPayload + ((IS_QOS_ENABLED(psessionEntry))
			     ? sizeof(tSirMacDataHdr3a) :
			     sizeof(tSirMacMgmtHdr))
		 + sizeof(eth_890d_header)
		 + PAYLOAD_TYPE_TDLS_SIZE;

#ifndef NO_PAD_TDLS_MIN_8023_SIZE
	/* IOT issue with some AP : some AP doesn't like the data packet size < minimum 802.3 frame length (64)
	   Hence AP itself padding some bytes, which caused teardown packet is dropped at
	   receiver side. To avoid such IOT issue, we added some extra bytes to meet data frame size >= 64
	 */
	if (nPayload + PAYLOAD_TYPE_TDLS_SIZE < MIN_IEEE_8023_SIZE) {
		padLen =
			MIN_IEEE_8023_SIZE - (nPayload + PAYLOAD_TYPE_TDLS_SIZE);

		/* if padLen is less than minimum vendorSpecific (5), pad up to 5 */
		if (padLen < MIN_VENDOR_SPECIFIC_IE_SIZE)
			padLen = MIN_VENDOR_SPECIFIC_IE_SIZE;

		nBytes += padLen;
	}
#endif

	/* Ok-- try to allocate memory from MGMT PKT pool */

	qdf_status = cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				      (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pe_err("Failed to allocate: %d bytes for a TDLS Discovery Request",
			nBytes);
		return eSIR_MEM_ALLOC_FAILED;
	}

	/* zero out the memory */
	qdf_mem_set(pFrame, nBytes, 0);

	/*
	 * IE formation, memory allocation is completed, Now form TDLS discovery
	 * request frame
	 */

	/* fill out the buffer descriptor */

	header_offset = lim_prepare_tdls_frame_header(pMac, pFrame,
						      LINK_IDEN_ADDR_OFFSET
							      (tdlsDisReq), TDLS_LINK_AP,
						      TDLS_INITIATOR, TID_AC_VI,
						      psessionEntry);

	status = dot11f_pack_tdls_dis_req(pMac, &tdlsDisReq, pFrame
					  + header_offset, nPayload, &nPayload);

	if (DOT11F_FAILED(status)) {
		pe_err("Failed to pack a TDLS discovery req (0x%08x)",
			status);
		cds_packet_free((void *)pPacket);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_warn("There were warnings while packing TDLS Discovery Request (0x%08x)",
			status);
	}

#ifndef NO_PAD_TDLS_MIN_8023_SIZE
	if (padLen != 0) {
		/* QCOM VENDOR OUI = { 0x00, 0xA0, 0xC6, type = 0x0000 }; */
		uint8_t *padVendorSpecific = pFrame + header_offset + nPayload;
		/* make QCOM_VENDOR_OUI, and type = 0x0000, and all the payload to be zero */
		padVendorSpecific[0] = 221;
		padVendorSpecific[1] = padLen - 2;
		padVendorSpecific[2] = 0x00;
		padVendorSpecific[3] = 0xA0;
		padVendorSpecific[4] = 0xC6;

		pe_debug("Padding Vendor Specific Ie Len: %d", padLen);

		/* padding zero if more than 5 bytes are required */
		if (padLen > MIN_VENDOR_SPECIFIC_IE_SIZE)
			qdf_mem_set(pFrame + header_offset + nPayload +
				    MIN_VENDOR_SPECIFIC_IE_SIZE,
				    padLen - MIN_VENDOR_SPECIFIC_IE_SIZE, 0);
	}
#endif

	pe_debug("[TDLS] action: %d (%s) -AP-> OTA peer="MAC_ADDRESS_STR,
		SIR_MAC_TDLS_DIS_REQ,
		lim_trace_tdls_action_string(SIR_MAC_TDLS_DIS_REQ),
		MAC_ADDR_ARRAY(peer_mac.bytes));

	pMac->lim.tdls_frm_session_id = psessionEntry->smeSessionId;
	qdf_status = wma_tx_frameWithTxComplete(pMac, pPacket,
					(uint16_t) nBytes,
					TXRX_FRM_802_11_DATA,
					ANI_TXDIR_TODS,
					TID_AC_VI,
					lim_tx_complete, pFrame,
					lim_mgmt_tdls_tx_complete,
					HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME |
					HAL_USE_PEER_STA_REQUESTED_MASK,
					smeSessionId, false, 0);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pMac->lim.tdls_frm_session_id = NO_SESSION;
		pe_err("could not send TDLS Discovery Request frame");
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;

}

/*
 * This static function is consistent with any kind of TDLS management
 * frames we are sending. Currently it is being used by lim_send_tdls_dis_rsp_frame,
 * lim_send_tdls_link_setup_req_frame and lim_send_tdls_setup_rsp_frame
 */
static void populate_dot11f_tdls_ht_vht_cap(tpAniSirGlobal pMac,
					    uint32_t selfDot11Mode,
					    tDot11fIEHTCaps *htCap,
					    tDot11fIEVHTCaps *vhtCap,
					    tpPESession psessionEntry)
{
	uint8_t nss;
	uint32_t val;

	if (IS_5G_CH(psessionEntry->currentOperChannel))
		nss = pMac->vdev_type_nss_5g.tdls;
	else
		nss = pMac->vdev_type_nss_2g.tdls;

	nss = QDF_MIN(nss, pMac->user_configured_nss);
	if (IS_DOT11_MODE_HT(selfDot11Mode)) {
		/* Include HT Capability IE */
		populate_dot11f_ht_caps(pMac, NULL, htCap);
		val = SIZE_OF_SUPPORTED_MCS_SET;
		wlan_cfg_get_str(pMac, WNI_CFG_SUPPORTED_MCS_SET,
				&htCap->supportedMCSSet[0], &val);
		if (NSS_1x1_MODE == nss)
			htCap->supportedMCSSet[1] = 0;
		/*
		 * Advertise ht capability and max supported channel bandwidth
		 * when populating HT IE in TDLS Setup Request/Setup Response/
		 * Setup Confirmation frames.
		 * 11.21.6.2 Setting up a 40 MHz direct link: A 40 MHz
		 * off-channel direct link may be started if both TDLS peer STAs
		 * indicated 40 MHz support in the Supported Channel Width Set
		 * field of the HT Capabilities element (which is included in
		 * the TDLS Setup Request frame and the TDLS Setup Response
		 * frame). Switching to a 40 MHz off-channel direct link is
		 * achieved by including the following information in the TDLS
		 * Channel Switch Request
		 * 11.21.1 General: The channel width of the TDLS direct link on
		 * the base channel shall not exceed the channel width of the
		 * BSS to which the TDLS peer STAs are associated.
		 */
		htCap->supportedChannelWidthSet = 1;
	} else {
		htCap->present = 0;
	}
	pe_debug("HT present: %hu, Chan Width: %hu",
		htCap->present, htCap->supportedChannelWidthSet);
	if (((psessionEntry->currentOperChannel <= SIR_11B_CHANNEL_END) &&
	     pMac->roam.configParam.enableVhtFor24GHz) ||
	    (psessionEntry->currentOperChannel >= SIR_11B_CHANNEL_END)) {
		if (IS_DOT11_MODE_VHT(selfDot11Mode) &&
		    IS_FEATURE_SUPPORTED_BY_FW(DOT11AC)) {
			/* Include VHT Capability IE */
			populate_dot11f_vht_caps(pMac, psessionEntry, vhtCap);

			/*
			 * Set to 0 if the TDLS STA does not support either 160
			 * or 80+80 MHz.
			 * Set to 1 if the TDLS STA supports 160 MHz.
			 * Set to 2 if the TDLS STA supports 160 MHz and
			 * 80+80 MHz.
			 * The value 3 is reserved
			 */
			vhtCap->supportedChannelWidthSet = 0;

			vhtCap->suBeamformeeCap = 0;
			vhtCap->suBeamFormerCap = 0;
			vhtCap->muBeamformeeCap = 0;
			vhtCap->muBeamformerCap = 0;

			wlan_cfg_get_int(pMac, WNI_CFG_VHT_RX_MCS_MAP, &val);
			vhtCap->rxMCSMap = val;
			wlan_cfg_get_int(pMac,
				WNI_CFG_VHT_RX_HIGHEST_SUPPORTED_DATA_RATE,
				&val);
			vhtCap->rxHighSupDataRate = val;
			wlan_cfg_get_int(pMac, WNI_CFG_VHT_TX_MCS_MAP, &val);
			vhtCap->txMCSMap = val;
			wlan_cfg_get_int(pMac,
				WNI_CFG_VHT_TX_HIGHEST_SUPPORTED_DATA_RATE,
				&val);
			vhtCap->txSupDataRate = val;
			if (nss == NSS_1x1_MODE) {
				vhtCap->txMCSMap |= DISABLE_NSS2_MCS;
				vhtCap->rxMCSMap |= DISABLE_NSS2_MCS;
				vhtCap->txSupDataRate =
					VHT_TX_HIGHEST_SUPPORTED_DATA_RATE_1_1;
				vhtCap->rxHighSupDataRate =
					VHT_RX_HIGHEST_SUPPORTED_DATA_RATE_1_1;
			}
		} else {
			vhtCap->present = 0;
		}
	} else {
		/* Vht Disable from ini in 2.4 GHz */
		vhtCap->present = 0;
	}
	pe_debug("VHT present: %hu, Chan Width: %hu",
		vhtCap->present, vhtCap->supportedChannelWidthSet);
}

/*
 * Send TDLS discovery response frame on direct link.
 */

static tSirRetStatus lim_send_tdls_dis_rsp_frame(tpAniSirGlobal pMac,
						 struct qdf_mac_addr peer_mac,
						 uint8_t dialog,
						 tpPESession psessionEntry,
						 uint8_t *addIe,
						 uint16_t addIeLen)
{
	tDot11fTDLSDisRsp tdlsDisRsp;
	uint16_t caps = 0;
	uint32_t status = 0;
	uint32_t nPayload = 0;
	uint32_t nBytes = 0;
	uint8_t *pFrame;
	void *pPacket;
	QDF_STATUS qdf_status;
	uint32_t selfDot11Mode;
/*  Placeholder to support different channel bonding mode of TDLS than AP. */
/*  Today, WNI_CFG_CHANNEL_BONDING_MODE will be overwritten when connecting to AP */
/*  To support this feature, we need to introduce WNI_CFG_TDLS_CHANNEL_BONDING_MODE */
/*  As of now, we hardcoded to max channel bonding of dot11Mode (i.e HT80 for 11ac/HT40 for 11n) */
/*  uint32_t tdlsChannelBondingMode; */
	uint8_t smeSessionId = 0;

	if (NULL == psessionEntry) {
		pe_err("psessionEntry is NULL");
		return eSIR_FAILURE;
	}
	smeSessionId = psessionEntry->smeSessionId;

	/*
	 * The scheme here is to fill out a 'tDot11fProbeRequest' structure
	 * and then hand it off to 'dot11f_pack_probe_request' (for
	 * serialization).  We start by zero-initializing the structure:
	 */
	qdf_mem_set((uint8_t *) &tdlsDisRsp, sizeof(tDot11fTDLSDisRsp), 0);

	/*
	 * setup Fixed fields,
	 */
	tdlsDisRsp.Category.category = SIR_MAC_ACTION_PUBLIC_USAGE;
	tdlsDisRsp.Action.action = SIR_MAC_TDLS_DIS_RSP;
	tdlsDisRsp.DialogToken.token = dialog;

	populate_dot11f_link_iden(pMac, psessionEntry,
				  &tdlsDisRsp.LinkIdentifier,
				  peer_mac, TDLS_RESPONDER);

	if (cfg_get_capability_info(pMac, &caps, psessionEntry)
		!= eSIR_SUCCESS) {
		/*
		 * Could not get Capabilities value
		 * from CFG. Log error.
		 */
		pe_err("could not retrieve Capabilities value");
	}
	swap_bit_field16(caps, (uint16_t *) &tdlsDisRsp.Capabilities);

	/* populate supported rate and ext supported rate IE */
	if (eSIR_FAILURE == populate_dot11f_rates_tdls(pMac,
					&tdlsDisRsp.SuppRates,
					&tdlsDisRsp.ExtSuppRates,
					psessionEntry->currentOperChannel))
		pe_err("could not populate supported data rates");

	/* populate extended capability IE */
	populate_dot11f_tdls_ext_capability(pMac,
					    psessionEntry,
					    &tdlsDisRsp.ExtCap);

	wlan_cfg_get_int(pMac, WNI_CFG_DOT11_MODE, &selfDot11Mode);

	/* Populate HT/VHT Capabilities */
	populate_dot11f_tdls_ht_vht_cap(pMac, selfDot11Mode, &tdlsDisRsp.HTCaps,
					&tdlsDisRsp.VHTCaps, psessionEntry);

	/* Populate TDLS offchannel param only if offchannel is enabled
	 * and TDLS Channel Switching is not prohibited by AP in ExtCap
	 * IE in assoc/re-assoc response.
	 */
	if ((1 == pMac->lim.gLimTDLSOffChannelEnabled) &&
	    (!psessionEntry->tdls_chan_swit_prohibited)) {
		populate_dot11f_tdls_offchannel_params(pMac, psessionEntry,
						       &tdlsDisRsp.SuppChannels,
						       &tdlsDisRsp.
						       SuppOperatingClasses);
		if (pMac->roam.configParam.bandCapability != eCSR_BAND_24) {
			tdlsDisRsp.ht2040_bss_coexistence.present = 1;
			tdlsDisRsp.ht2040_bss_coexistence.info_request = 1;
		}
	} else {
		pe_debug("TDLS offchan not enabled, or channel switch prohibited by AP, gLimTDLSOffChannelEnabled: %d tdls_chan_swit_prohibited: %d",
			pMac->lim.gLimTDLSOffChannelEnabled,
			psessionEntry->tdls_chan_swit_prohibited);
	}
	/*
	 * now we pack it.  First, how much space are we going to need?
	 */
	status = dot11f_get_packed_tdls_dis_rsp_size(pMac, &tdlsDisRsp, &nPayload);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to calculate the packed size for a Discovery Response (0x%08x)",
			status);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fProbeRequest);
	} else if (DOT11F_WARNED(status)) {
		pe_warn("There were warnings while calculating the packed size for a Discovery Response (0x%08x)",
			status);
	}

	/*
	 * This frame is going out from PE as data frames with special ethertype
	 * 89-0d.
	 * 8 bytes of RFC 1042 header
	 */

	nBytes = nPayload + sizeof(tSirMacMgmtHdr) + addIeLen;

	/* Ok-- try to allocate memory from MGMT PKT pool */
	qdf_status = cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				(void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pe_err("Failed to allocate %d bytes for a TDLS Discovery Request",
			nBytes);
		return eSIR_MEM_ALLOC_FAILED;
	}

	/* zero out the memory */
	qdf_mem_set(pFrame, nBytes, 0);

	/*
	 * IE formation, memory allocation is completed, Now form TDLS discovery
	 * response frame
	 */

	/* Make public Action Frame */

	lim_populate_mac_header(pMac, pFrame, SIR_MAC_MGMT_FRAME,
				SIR_MAC_MGMT_ACTION, peer_mac.bytes,
				psessionEntry->selfMacAddr);

	{
		tpSirMacMgmtHdr pMacHdr;
		pMacHdr = (tpSirMacMgmtHdr) pFrame;
		pMacHdr->fc.toDS = ANI_TXDIR_IBSS;
		pMacHdr->fc.powerMgmt = 0;
		sir_copy_mac_addr(pMacHdr->bssId, psessionEntry->bssId);
	}

	status = dot11f_pack_tdls_dis_rsp(pMac, &tdlsDisRsp, pFrame +
					  sizeof(tSirMacMgmtHdr),
					  nPayload, &nPayload);

	if (DOT11F_FAILED(status)) {
		pe_err("Failed to pack a TDLS discovery response (0x%08x)",
			status);
		cds_packet_free((void *)pPacket);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_warn("There were warnings while packing TDLS Discovery Response (0x%08x)",
			status);
	}

	if (0 != addIeLen) {
		pe_debug("Copy Additional Ie Len: %d", addIeLen);
		qdf_mem_copy(pFrame + sizeof(tSirMacMgmtHdr) + nPayload, addIe,
			     addIeLen);
	}
	pe_debug("[TDLS] action: %d (%s) -DIRECT-> OTA peer="MAC_ADDRESS_STR,
		SIR_MAC_TDLS_DIS_RSP,
		lim_trace_tdls_action_string(SIR_MAC_TDLS_DIS_RSP),
		MAC_ADDR_ARRAY(peer_mac.bytes));

	pMac->lim.tdls_frm_session_id = psessionEntry->smeSessionId;
	/*
	 * Transmit Discovery response and watch if this is delivered to
	 * peer STA.
	 */
	/* In CLD 2.0, pass Discovery Response as mgmt frame so that
	 * wma does not do header conversion to 802.3 before calling tx/rx
	 * routine and subsequenly target also sends frame as is OTA
	 */
	qdf_status = wma_tx_frameWithTxComplete(pMac, pPacket, (uint16_t) nBytes,
					      TXRX_FRM_802_11_MGMT,
					      ANI_TXDIR_IBSS,
					      0,
					      lim_tx_complete, pFrame,
					      lim_mgmt_tdls_tx_complete,
					      HAL_USE_SELF_STA_REQUESTED_MASK,
					      smeSessionId, false, 0);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pMac->lim.tdls_frm_session_id = NO_SESSION;
		pe_err("could not send TDLS Discovery Response frame!");
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;

}

/*
 * This static function is currently used by lim_send_tdls_link_setup_req_frame and
 * lim_send_tdls_setup_rsp_frame to populate the AID if device is 11ac capable.
 */
static void populate_dotf_tdls_vht_aid(tpAniSirGlobal pMac, uint32_t selfDot11Mode,
				       struct qdf_mac_addr peerMac,
				       tDot11fIEAID *Aid,
				       tpPESession psessionEntry)
{
	if (((psessionEntry->currentOperChannel <= SIR_11B_CHANNEL_END) &&
	     pMac->roam.configParam.enableVhtFor24GHz) ||
	    (psessionEntry->currentOperChannel >= SIR_11B_CHANNEL_END)) {
		if (IS_DOT11_MODE_VHT(selfDot11Mode) &&
		    IS_FEATURE_SUPPORTED_BY_FW(DOT11AC)) {

			uint16_t aid;
			tpDphHashNode pStaDs;

			pStaDs =
				dph_lookup_hash_entry(pMac, peerMac.bytes, &aid,
						      &psessionEntry->dph.
						      dphHashTable);
			if (NULL != pStaDs) {
				Aid->present = 1;
				Aid->assocId = aid | LIM_AID_MASK;      /* set bit 14 and 15 1's */
			} else {
				Aid->present = 0;
				pe_err("pStaDs is NULL for "
					   MAC_ADDRESS_STR,
					MAC_ADDR_ARRAY(peerMac.bytes));
			}
		}
	} else {
		Aid->present = 0;
		pe_warn("Vht not enable from ini for 2.4GHz");
	}
}

#ifdef CONFIG_HL_SUPPORT

/**
 * wma_tx_frame_with_tx_complete_send() - Send tx frames on Direct link or AP link
 *				       depend on reason code
 * @pMac: pointer to MAC Sirius parameter structure
 * @pPacket: pointer to mgmt packet
 * @nBytes: number of bytes to send
 * @tid:tid value for AC
 * @pFrame: pointer to tdls frame
 * @smeSessionId:session id
 * @flag: tdls flag
 *
 * Send TDLS Teardown frame on Direct link or AP link, depends on reason code.
 *
 * Return: None
 */
static inline QDF_STATUS
wma_tx_frame_with_tx_complete_send(tpAniSirGlobal pMac, void *pPacket,
				uint16_t nBytes,
				uint8_t tid,
				uint8_t *pFrame,
				uint8_t smeSessionId, bool flag)
{
	return wma_tx_frameWithTxComplete(pMac, pPacket,
					  (uint16_t) nBytes,
					  TXRX_FRM_802_11_DATA,
					  ANI_TXDIR_TODS,
					  tid,
					  lim_tx_complete, pFrame,
					  lim_mgmt_tdls_tx_complete,
					  HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME
					  | HAL_USE_PEER_STA_REQUESTED_MASK,
					  smeSessionId, flag, 0);
}
#else

static inline QDF_STATUS
wma_tx_frame_with_tx_complete_send(tpAniSirGlobal pMac, void *pPacket,
				uint16_t nBytes,
				uint8_t tid,
				uint8_t *pFrame,
				uint8_t smeSessionId, bool flag)
{
	return wma_tx_frameWithTxComplete(pMac, pPacket,
					  (uint16_t) nBytes,
					  TXRX_FRM_802_11_DATA,
					  ANI_TXDIR_TODS,
					  tid,
					  lim_tx_complete, pFrame,
					  lim_mgmt_tdls_tx_complete,
					  HAL_USE_BD_RATE2_FOR_MANAGEMENT_FRAME
					  | HAL_USE_PEER_STA_REQUESTED_MASK,
					  smeSessionId, false, 0);
}
#endif

void lim_set_tdls_flags(roam_offload_synch_ind *roam_sync_ind_ptr,
		   tpPESession ft_session_ptr)
{
	roam_sync_ind_ptr->join_rsp->tdls_prohibited =
		ft_session_ptr->tdls_prohibited;
	roam_sync_ind_ptr->join_rsp->tdls_chan_swit_prohibited =
		ft_session_ptr->tdls_chan_swit_prohibited;
}

/*
 * TDLS setup Request frame on AP link
 */
static
tSirRetStatus lim_send_tdls_link_setup_req_frame(tpAniSirGlobal pMac,
						 struct qdf_mac_addr peer_mac,
						 uint8_t dialog,
						 tpPESession psessionEntry,
						 uint8_t *addIe,
						 uint16_t addIeLen)
{
	tDot11fTDLSSetupReq tdlsSetupReq;
	uint16_t caps = 0;
	uint32_t status = 0;
	uint32_t nPayload = 0;
	uint32_t nBytes = 0;
	uint32_t header_offset = 0;
	uint8_t *pFrame;
	void *pPacket;
	QDF_STATUS qdf_status;
	uint32_t selfDot11Mode;
	uint8_t smeSessionId = 0;
/*  Placeholder to support different channel bonding mode of TDLS than AP. */
/*  Today, WNI_CFG_CHANNEL_BONDING_MODE will be overwritten when connecting to AP */
/*  To support this feature, we need to introduce WNI_CFG_TDLS_CHANNEL_BONDING_MODE */
/*  As of now, we hardcoded to max channel bonding of dot11Mode (i.e HT80 for 11ac/HT40 for 11n) */
/*  uint32_t tdlsChannelBondingMode; */

	/*
	 * The scheme here is to fill out a 'tDot11fProbeRequest' structure
	 * and then hand it off to 'dot11f_pack_probe_request' (for
	 * serialization).  We start by zero-initializing the structure:
	 */
	smeSessionId = psessionEntry->smeSessionId;

	qdf_mem_set((uint8_t *) &tdlsSetupReq, sizeof(tDot11fTDLSSetupReq), 0);
	tdlsSetupReq.Category.category = SIR_MAC_ACTION_TDLS;
	tdlsSetupReq.Action.action = SIR_MAC_TDLS_SETUP_REQ;
	tdlsSetupReq.DialogToken.token = dialog;

	populate_dot11f_link_iden(pMac, psessionEntry,
				  &tdlsSetupReq.LinkIdentifier, peer_mac,
				  TDLS_INITIATOR);

	if (cfg_get_capability_info(pMac, &caps, psessionEntry) != eSIR_SUCCESS) {
		/*
		 * Could not get Capabilities value
		 * from CFG. Log error.
		 */
		pe_err("could not retrieve Capabilities value");
	}
	swap_bit_field16(caps, (uint16_t *) &tdlsSetupReq.Capabilities);

	/* populate supported rate and ext supported rate IE */
	if (eSIR_FAILURE == populate_dot11f_rates_tdls(pMac,
					&tdlsSetupReq.SuppRates,
					&tdlsSetupReq.ExtSuppRates,
					psessionEntry->currentOperChannel))
		pe_err("could not populate supported data rates");

	/* Populate extended capability IE */
	populate_dot11f_tdls_ext_capability(pMac,
					    psessionEntry,
					    &tdlsSetupReq.ExtCap);

	if (1 == pMac->lim.gLimTDLSWmmMode) {
		uint32_t val = 0;

		pe_debug("populate WMM IE in Setup Request Frame");
		/* include WMM IE */
		tdlsSetupReq.WMMInfoStation.version = SIR_MAC_OUI_VERSION_1;
		tdlsSetupReq.WMMInfoStation.acvo_uapsd =
			(pMac->lim.gLimTDLSUapsdMask & 0x01);
		tdlsSetupReq.WMMInfoStation.acvi_uapsd =
			((pMac->lim.gLimTDLSUapsdMask & 0x02) >> 1);
		tdlsSetupReq.WMMInfoStation.acbk_uapsd =
			((pMac->lim.gLimTDLSUapsdMask & 0x04) >> 2);
		tdlsSetupReq.WMMInfoStation.acbe_uapsd =
			((pMac->lim.gLimTDLSUapsdMask & 0x08) >> 3);

		if (wlan_cfg_get_int(pMac, WNI_CFG_MAX_SP_LENGTH, &val) !=
		    eSIR_SUCCESS)
			pe_warn("could not retrieve Max SP Length");

		tdlsSetupReq.WMMInfoStation.max_sp_length = (uint8_t) val;
		tdlsSetupReq.WMMInfoStation.present = 1;
	} else {
		/*
		 * TODO: we need to see if we have to support conditions where
		 * we have EDCA parameter info element is needed a) if we need
		 * different QOS parameters for off channel operations or QOS
		 * is not supported on AP link and we wanted to QOS on direct
		 * link.
		 */

		/* Populate QOS info, needed for Peer U-APSD session */

		/*
		 * TODO: Now hardcoded, since populate_dot11f_qos_caps_station()
		 * depends on AP's capability, and TDLS doesn't want to depend
		 * on AP's capability
		 */

		pe_debug("populate QOS IE in Setup Request Frame");
		tdlsSetupReq.QOSCapsStation.present = 1;
		tdlsSetupReq.QOSCapsStation.max_sp_length = 0;
		tdlsSetupReq.QOSCapsStation.qack = 0;
		tdlsSetupReq.QOSCapsStation.acbe_uapsd =
			((pMac->lim.gLimTDLSUapsdMask & 0x08) >> 3);
		tdlsSetupReq.QOSCapsStation.acbk_uapsd =
			((pMac->lim.gLimTDLSUapsdMask & 0x04) >> 2);
		tdlsSetupReq.QOSCapsStation.acvi_uapsd =
			((pMac->lim.gLimTDLSUapsdMask & 0x02) >> 1);
		tdlsSetupReq.QOSCapsStation.acvo_uapsd =
			(pMac->lim.gLimTDLSUapsdMask & 0x01);
	}

	/*
	 * we will always try to init TDLS link with 11n capabilities
	 * let TDLS setup response to come, and we will set our caps based
	 * of peer caps
	 */

	wlan_cfg_get_int(pMac, WNI_CFG_DOT11_MODE, &selfDot11Mode);

	/* Populate HT/VHT Capabilities */
	populate_dot11f_tdls_ht_vht_cap(pMac, selfDot11Mode, &tdlsSetupReq.HTCaps,
					&tdlsSetupReq.VHTCaps, psessionEntry);

	/* Populate AID */
	populate_dotf_tdls_vht_aid(pMac, selfDot11Mode, peer_mac,
				   &tdlsSetupReq.AID, psessionEntry);

	/* Populate TDLS offchannel param only if offchannel is enabled
	 * and TDLS Channel Switching is not prohibited by AP in ExtCap
	 * IE in assoc/re-assoc response.
	 */
	if ((1 == pMac->lim.gLimTDLSOffChannelEnabled) &&
	    (!psessionEntry->tdls_chan_swit_prohibited)) {
		populate_dot11f_tdls_offchannel_params(pMac, psessionEntry,
						     &tdlsSetupReq.SuppChannels,
						     &tdlsSetupReq.
						     SuppOperatingClasses);
		if (pMac->roam.configParam.bandCapability != eCSR_BAND_24) {
			tdlsSetupReq.ht2040_bss_coexistence.present = 1;
			tdlsSetupReq.ht2040_bss_coexistence.info_request = 1;
		}
	} else {
		pe_debug("TDLS offchan not enabled, or channel switch prohibited by AP, gLimTDLSOffChannelEnabled: %d tdls_chan_swit_prohibited: %d",
			pMac->lim.gLimTDLSOffChannelEnabled,
			psessionEntry->tdls_chan_swit_prohibited);
	}
	/*
	 * now we pack it.  First, how much space are we going to need?
	 */
	status = dot11f_get_packed_tdls_setup_req_size(pMac, &tdlsSetupReq,
						       &nPayload);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to calculate the packed size for a Setup Request (0x%08x)",
			status);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fProbeRequest);
	} else if (DOT11F_WARNED(status)) {
		pe_warn("There were warnings while calculating the packed size for a Setup Request (0x%08x)",
			status);
	}

	/*
	 * This frame is going out from PE as data frames with special ethertype
	 * 89-0d.
	 * 8 bytes of RFC 1042 header
	 */

	nBytes = nPayload + ((IS_QOS_ENABLED(psessionEntry))
			     ? sizeof(tSirMacDataHdr3a) :
			     sizeof(tSirMacMgmtHdr))
		 + sizeof(eth_890d_header)
		 + PAYLOAD_TYPE_TDLS_SIZE + addIeLen;

	/* Ok-- try to allocate memory from MGMT PKT pool */
	qdf_status = cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
			(void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pe_err("Failed to allocate %d bytes for a TDLS Setup Request",
			nBytes);
		return eSIR_MEM_ALLOC_FAILED;
	}

	/* zero out the memory */
	qdf_mem_set(pFrame, nBytes, 0);

	/*
	 * IE formation, memory allocation is completed, Now form TDLS discovery
	 * request frame
	 */

	/* fill out the buffer descriptor */

	header_offset = lim_prepare_tdls_frame_header(pMac, pFrame,
						      LINK_IDEN_ADDR_OFFSET
							      (tdlsSetupReq), TDLS_LINK_AP,
						      TDLS_INITIATOR, TID_AC_VI,
						      psessionEntry);

	pe_debug("SupportedChnlWidth: %x rxMCSMap: %x rxMCSMap: %x txSupDataRate: %x",
		tdlsSetupReq.VHTCaps.supportedChannelWidthSet,
		tdlsSetupReq.VHTCaps.rxMCSMap,
		tdlsSetupReq.VHTCaps.txMCSMap,
		tdlsSetupReq.VHTCaps.txSupDataRate);

	status = dot11f_pack_tdls_setup_req(pMac, &tdlsSetupReq, pFrame
					    + header_offset, nPayload, &nPayload);

	if (DOT11F_FAILED(status)) {
		pe_err("Failed to pack a TDLS Setup request (0x%08x)",
			status);
		cds_packet_free((void *)pPacket);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_warn("There were warnings while packing TDLS Setup Request (0x%08x)",
			status);
	}

	/* Copy the additional IE. */
	/* TODO : addIe is added at the end of the frame. This means it doesnt */
	/* follow the order. This should be ok, but we should consider changing this */
	/* if there is any IOT issue. */
	if (addIeLen != 0) {
		pe_debug("Copy Additional Ie Len = %d", addIeLen);
		qdf_mem_copy(pFrame + header_offset + nPayload, addIe,
			     addIeLen);
	}

	pe_debug("[TDLS] action: %d (%s) -AP-> OTA peer="MAC_ADDRESS_STR,
		SIR_MAC_TDLS_SETUP_REQ,
		lim_trace_tdls_action_string(SIR_MAC_TDLS_SETUP_REQ),
		MAC_ADDR_ARRAY(peer_mac.bytes));

	pMac->lim.tdls_frm_session_id = psessionEntry->smeSessionId;

	qdf_status = wma_tx_frame_with_tx_complete_send(pMac, pPacket,
						     (uint16_t) nBytes,
						     TID_AC_VI,
						     pFrame,
						     smeSessionId, true);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pMac->lim.tdls_frm_session_id = NO_SESSION;
		pe_err("could not send TDLS Setup Request frame!");
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;

}

/*
 * Send TDLS Teardown frame on Direct link or AP link, depends on reason code.
 */
static
tSirRetStatus lim_send_tdls_teardown_frame(tpAniSirGlobal pMac,
					   struct qdf_mac_addr peer_mac,
					   uint16_t reason,
					   uint8_t responder,
					   tpPESession psessionEntry,
					   uint8_t *addIe, uint16_t addIeLen)
{
	tDot11fTDLSTeardown teardown;
	uint32_t status = 0;
	uint32_t nPayload = 0;
	uint32_t nBytes = 0;
	uint32_t header_offset = 0;
	uint8_t *pFrame;
	void *pPacket;
	QDF_STATUS qdf_status;
#ifndef NO_PAD_TDLS_MIN_8023_SIZE
	uint32_t padLen = 0;
#endif
	uint8_t smeSessionId = 0;
	tpDphHashNode sta_ds;
	uint16_t aid = 0;
	uint8_t qos_mode = 0;
	uint8_t tdls_link_type;

	if (NULL == psessionEntry) {
		pe_err("psessionEntry is NULL");
		return eSIR_FAILURE;
	}
	smeSessionId = psessionEntry->smeSessionId;
	/*
	 * The scheme here is to fill out a 'tDot11fProbeRequest' structure
	 * and then hand it off to 'dot11f_pack_probe_request' (for
	 * serialization).  We start by zero-initializing the structure:
	 */
	qdf_mem_set((uint8_t *) &teardown, sizeof(tDot11fTDLSTeardown), 0);
	teardown.Category.category = SIR_MAC_ACTION_TDLS;
	teardown.Action.action = SIR_MAC_TDLS_TEARDOWN;
	teardown.Reason.code = reason;

	populate_dot11f_link_iden(pMac, psessionEntry, &teardown.LinkIdentifier,
				  peer_mac,
				  (responder ==
				   true) ? TDLS_RESPONDER : TDLS_INITIATOR);

	/*
	 * now we pack it.  First, how much space are we going to need?
	 */
	status = dot11f_get_packed_tdls_teardown_size(pMac, &teardown, &nPayload);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to calculate the packed size for a discovery Request (0x%08x)",
			status);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fProbeRequest);
	} else if (DOT11F_WARNED(status)) {
		pe_warn("There were warnings while calculating the packed size for a discovery Request (0x%08x)",
			status);
	}

	/*
	 * This frame is going out from PE as data frames with special ethertype
	 * 89-0d.
	 * 8 bytes of RFC 1042 header
	 */
	sta_ds = dph_lookup_hash_entry(pMac, psessionEntry->bssId, &aid,
					&psessionEntry->dph.dphHashTable);
	if (sta_ds)
		qos_mode = sta_ds->qosMode;
	tdls_link_type = (reason == eSIR_MAC_TDLS_TEARDOWN_PEER_UNREACHABLE)
				? TDLS_LINK_AP : TDLS_LINK_DIRECT;
	nBytes = nPayload + (((IS_QOS_ENABLED(psessionEntry) &&
			     (tdls_link_type == TDLS_LINK_AP)) ||
			     ((tdls_link_type == TDLS_LINK_DIRECT) && qos_mode))
			     ? sizeof(tSirMacDataHdr3a) :
			     sizeof(tSirMacMgmtHdr))
		 + sizeof(eth_890d_header)
		 + PAYLOAD_TYPE_TDLS_SIZE + addIeLen;

#ifndef NO_PAD_TDLS_MIN_8023_SIZE
	/* IOT issue with some AP : some AP doesn't like the data packet size < minimum 802.3 frame length (64)
	   Hence AP itself padding some bytes, which caused teardown packet is dropped at
	   receiver side. To avoid such IOT issue, we added some extra bytes to meet data frame size >= 64
	 */
	if (nPayload + PAYLOAD_TYPE_TDLS_SIZE < MIN_IEEE_8023_SIZE) {
		padLen =
			MIN_IEEE_8023_SIZE - (nPayload + PAYLOAD_TYPE_TDLS_SIZE);

		/* if padLen is less than minimum vendorSpecific (5), pad up to 5 */
		if (padLen < MIN_VENDOR_SPECIFIC_IE_SIZE)
			padLen = MIN_VENDOR_SPECIFIC_IE_SIZE;

		nBytes += padLen;
	}
#endif

	/* Ok-- try to allocate memory from MGMT PKT pool */
	qdf_status = cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
			(void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pe_err("Failed to allocate %d bytes for a TDLS Teardown Frame.",
			nBytes);
		return eSIR_MEM_ALLOC_FAILED;
	}

	/* zero out the memory */
	qdf_mem_set(pFrame, nBytes, 0);

	/*
	 * IE formation, memory allocation is completed, Now form TDLS discovery
	 * request frame
	 */

	/* fill out the buffer descriptor */
	pe_debug("Reason of TDLS Teardown: %d", reason);
	header_offset = lim_prepare_tdls_frame_header(pMac, pFrame,
						      LINK_IDEN_ADDR_OFFSET
							      (teardown),
						      (reason ==
						       eSIR_MAC_TDLS_TEARDOWN_PEER_UNREACHABLE)
						      ? TDLS_LINK_AP :
						      TDLS_LINK_DIRECT,
						      (responder ==
						       true) ? TDLS_RESPONDER :
						      TDLS_INITIATOR, TID_AC_VI,
						      psessionEntry);

	status = dot11f_pack_tdls_teardown(pMac, &teardown, pFrame
					   + header_offset, nPayload, &nPayload);

	if (DOT11F_FAILED(status)) {
		pe_err("Failed to pack a TDLS Teardown frame (0x%08x)",
			status);
		cds_packet_free((void *)pPacket);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_warn("There were warnings while packing TDLS Teardown frame (0x%08x)",
			status);
	}

	if (addIeLen != 0) {
		pe_debug("Copy Additional Ie Len = %d", addIeLen);
		qdf_mem_copy(pFrame + header_offset + nPayload, addIe,
			     addIeLen);
	}
#ifndef NO_PAD_TDLS_MIN_8023_SIZE
	if (padLen != 0) {
		/* QCOM VENDOR OUI = { 0x00, 0xA0, 0xC6, type = 0x0000 }; */
		uint8_t *padVendorSpecific =
			pFrame + header_offset + nPayload + addIeLen;
		/* make QCOM_VENDOR_OUI, and type = 0x0000, and all the payload to be zero */
		padVendorSpecific[0] = 221;
		padVendorSpecific[1] = padLen - 2;
		padVendorSpecific[2] = 0x00;
		padVendorSpecific[3] = 0xA0;
		padVendorSpecific[4] = 0xC6;

		pe_debug("Padding Vendor Specific Ie Len = %d", padLen);

		/* padding zero if more than 5 bytes are required */
		if (padLen > MIN_VENDOR_SPECIFIC_IE_SIZE)
			qdf_mem_set(pFrame + header_offset + nPayload +
				    addIeLen + MIN_VENDOR_SPECIFIC_IE_SIZE,
				    padLen - MIN_VENDOR_SPECIFIC_IE_SIZE, 0);
	}
#endif
	pe_debug("[TDLS] action: %d (%s) -%s-> OTA peer="MAC_ADDRESS_STR,
		SIR_MAC_TDLS_TEARDOWN,
		lim_trace_tdls_action_string(SIR_MAC_TDLS_TEARDOWN),
		((reason == eSIR_MAC_TDLS_TEARDOWN_PEER_UNREACHABLE) ? "AP" :
		    "DIRECT"),
		MAC_ADDR_ARRAY(peer_mac.bytes));

	pMac->lim.tdls_frm_session_id = psessionEntry->smeSessionId;

	qdf_status = wma_tx_frame_with_tx_complete_send(pMac, pPacket,
						     (uint16_t) nBytes,
						     TID_AC_VI,
						     pFrame,
						     smeSessionId,
						     (reason == eSIR_MAC_TDLS_TEARDOWN_PEER_UNREACHABLE)
						     ? true : false);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pMac->lim.tdls_frm_session_id = NO_SESSION;
		pe_err("could not send TDLS Teardown frame");
		return eSIR_FAILURE;

	}
	return eSIR_SUCCESS;
}

/*
 * Send Setup RSP frame on AP link.
 */
static tSirRetStatus lim_send_tdls_setup_rsp_frame(tpAniSirGlobal pMac,
						   struct qdf_mac_addr peer_mac,
						   uint8_t dialog,
						   tpPESession psessionEntry,
						   etdlsLinkSetupStatus setupStatus,
						   uint8_t *addIe,
						   uint16_t addIeLen)
{
	tDot11fTDLSSetupRsp tdlsSetupRsp;
	uint32_t status = 0;
	uint16_t caps = 0;
	uint32_t nPayload = 0;
	uint32_t header_offset = 0;
	uint32_t nBytes = 0;
	uint8_t *pFrame;
	void *pPacket;
	QDF_STATUS qdf_status;
	uint32_t selfDot11Mode;
/*  Placeholder to support different channel bonding mode of TDLS than AP. */
/*  Today, WNI_CFG_CHANNEL_BONDING_MODE will be overwritten when connecting to AP */
/*  To support this feature, we need to introduce WNI_CFG_TDLS_CHANNEL_BONDING_MODE */
/*  As of now, we hardcoded to max channel bonding of dot11Mode (i.e HT80 for 11ac/HT40 for 11n) */
/*  uint32_t tdlsChannelBondingMode; */
	uint8_t smeSessionId = 0;

	if (NULL == psessionEntry) {
		pe_err("psessionEntry is NULL");
		return eSIR_FAILURE;
	}
	smeSessionId = psessionEntry->smeSessionId;

	/*
	 * The scheme here is to fill out a 'tDot11fProbeRequest' structure
	 * and then hand it off to 'dot11f_pack_probe_request' (for
	 * serialization).  We start by zero-initializing the structure:
	 */
	qdf_mem_set((uint8_t *) &tdlsSetupRsp, sizeof(tDot11fTDLSSetupRsp), 0);

	/*
	 * setup Fixed fields,
	 */
	tdlsSetupRsp.Category.category = SIR_MAC_ACTION_TDLS;
	tdlsSetupRsp.Action.action = SIR_MAC_TDLS_SETUP_RSP;
	tdlsSetupRsp.DialogToken.token = dialog;

	populate_dot11f_link_iden(pMac, psessionEntry,
				  &tdlsSetupRsp.LinkIdentifier, peer_mac,
				  TDLS_RESPONDER);

	if (cfg_get_capability_info(pMac, &caps, psessionEntry) != eSIR_SUCCESS) {
		/*
		 * Could not get Capabilities value
		 * from CFG. Log error.
		 */
		pe_err("could not retrieve Capabilities value");
	}
	swap_bit_field16(caps, (uint16_t *) &tdlsSetupRsp.Capabilities);

	/* populate supported rate and ext supported rate IE */
	if (eSIR_FAILURE == populate_dot11f_rates_tdls(pMac,
					&tdlsSetupRsp.SuppRates,
					&tdlsSetupRsp.ExtSuppRates,
					psessionEntry->currentOperChannel))
		pe_err("could not populate supported data rates");

	/* Populate extended capability IE */
	populate_dot11f_tdls_ext_capability(pMac,
					    psessionEntry,
					    &tdlsSetupRsp.ExtCap);

	if (1 == pMac->lim.gLimTDLSWmmMode) {
		uint32_t val = 0;

		pe_debug("populate WMM IE in Setup Response frame");
		/* include WMM IE */
		tdlsSetupRsp.WMMInfoStation.version = SIR_MAC_OUI_VERSION_1;
		tdlsSetupRsp.WMMInfoStation.acvo_uapsd =
			(pMac->lim.gLimTDLSUapsdMask & 0x01);
		tdlsSetupRsp.WMMInfoStation.acvi_uapsd =
			((pMac->lim.gLimTDLSUapsdMask & 0x02) >> 1);
		tdlsSetupRsp.WMMInfoStation.acbk_uapsd =
			((pMac->lim.gLimTDLSUapsdMask & 0x04) >> 2);
		tdlsSetupRsp.WMMInfoStation.acbe_uapsd =
			((pMac->lim.gLimTDLSUapsdMask & 0x08) >> 3);
		if (wlan_cfg_get_int(pMac, WNI_CFG_MAX_SP_LENGTH, &val) !=
		    eSIR_SUCCESS)
			pe_warn("could not retrieve Max SP Length");
			tdlsSetupRsp.WMMInfoStation.max_sp_length = (uint8_t) val;
		tdlsSetupRsp.WMMInfoStation.present = 1;
	} else {
		/*
		 * TODO: we need to see if we have to support conditions where
		 * we have EDCA parameter info element is needed a) if we need
		 * different QOS parameters for off channel operations or QOS
		 * is not supported on AP link and we wanted to QOS on direct
		 * link.
		 */
		/* Populate QOS info, needed for Peer U-APSD session */
		/*
		 * TODO: Now hardcoded, because
		 * populate_dot11f_qos_caps_station() depends on AP's
		 * capability, and TDLS doesn't want to depend on AP's
		 * capability
		 */
		pe_debug("populate QOS IE in Setup Response frame");
		tdlsSetupRsp.QOSCapsStation.present = 1;
		tdlsSetupRsp.QOSCapsStation.max_sp_length = 0;
		tdlsSetupRsp.QOSCapsStation.qack = 0;
		tdlsSetupRsp.QOSCapsStation.acbe_uapsd =
			((pMac->lim.gLimTDLSUapsdMask & 0x08) >> 3);
		tdlsSetupRsp.QOSCapsStation.acbk_uapsd =
			((pMac->lim.gLimTDLSUapsdMask & 0x04) >> 2);
		tdlsSetupRsp.QOSCapsStation.acvi_uapsd =
			((pMac->lim.gLimTDLSUapsdMask & 0x02) >> 1);
		tdlsSetupRsp.QOSCapsStation.acvo_uapsd =
			(pMac->lim.gLimTDLSUapsdMask & 0x01);
	}

	wlan_cfg_get_int(pMac, WNI_CFG_DOT11_MODE, &selfDot11Mode);

	/* Populate HT/VHT Capabilities */
	populate_dot11f_tdls_ht_vht_cap(pMac, selfDot11Mode, &tdlsSetupRsp.HTCaps,
					&tdlsSetupRsp.VHTCaps, psessionEntry);

	/* Populate AID */
	populate_dotf_tdls_vht_aid(pMac, selfDot11Mode, peer_mac,
				   &tdlsSetupRsp.AID, psessionEntry);

	/* Populate TDLS offchannel param only if offchannel is enabled
	 * and TDLS Channel Switching is not prohibited by AP in ExtCap
	 * IE in assoc/re-assoc response.
	 */
	if ((1 == pMac->lim.gLimTDLSOffChannelEnabled) &&
	    (!psessionEntry->tdls_chan_swit_prohibited)) {
		populate_dot11f_tdls_offchannel_params(pMac, psessionEntry,
						    &tdlsSetupRsp.SuppChannels,
						    &tdlsSetupRsp.
						    SuppOperatingClasses);
		if (pMac->roam.configParam.bandCapability != eCSR_BAND_24) {
			tdlsSetupRsp.ht2040_bss_coexistence.present = 1;
			tdlsSetupRsp.ht2040_bss_coexistence.info_request = 1;
		}
	} else {
		pe_debug("TDLS offchan not enabled, or channel switch prohibited by AP, gLimTDLSOffChannelEnabled: %d tdls_chan_swit_prohibited: %d",
			pMac->lim.gLimTDLSOffChannelEnabled,
			psessionEntry->tdls_chan_swit_prohibited);
	}
	tdlsSetupRsp.Status.status = setupStatus;
	/*
	 * now we pack it.  First, how much space are we going to need?
	 */
	status = dot11f_get_packed_tdls_setup_rsp_size(pMac, &tdlsSetupRsp,
						       &nPayload);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to calculate the packed size for a Setup Response (0x%08x)",
			status);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fProbeRequest);
	} else if (DOT11F_WARNED(status)) {
		pe_warn("There were warnings while calculating the packed size for Setup Response (0x%08x)",
			status);
	}

	/*
	 * This frame is going out from PE as data frames with special ethertype
	 * 89-0d.
	 * 8 bytes of RFC 1042 header
	 */

	nBytes = nPayload + ((IS_QOS_ENABLED(psessionEntry))
			     ? sizeof(tSirMacDataHdr3a) :
			     sizeof(tSirMacMgmtHdr))
		 + sizeof(eth_890d_header)
		 + PAYLOAD_TYPE_TDLS_SIZE + addIeLen;

	/* Ok-- try to allocate memory from MGMT PKT pool */
	qdf_status = cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				      (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pe_err("Failed to allocate %d bytes for a TDLS Setup Response",
			nBytes);
		return eSIR_MEM_ALLOC_FAILED;
	}

	/* zero out the memory */
	qdf_mem_set(pFrame, nBytes, 0);

	/*
	 * IE formation, memory allocation is completed, Now form TDLS discovery
	 * request frame
	 */

	/* fill out the buffer descriptor */

	header_offset = lim_prepare_tdls_frame_header(pMac, pFrame,
						      LINK_IDEN_ADDR_OFFSET
							      (tdlsSetupRsp), TDLS_LINK_AP,
						      TDLS_RESPONDER, TID_AC_VI,
						      psessionEntry);

	pe_debug("SupportedChnlWidth: %x rxMCSMap: %x rxMCSMap: %x txSupDataRate: %x",
		tdlsSetupRsp.VHTCaps.supportedChannelWidthSet,
		tdlsSetupRsp.VHTCaps.rxMCSMap,
		tdlsSetupRsp.VHTCaps.txMCSMap,
		tdlsSetupRsp.VHTCaps.txSupDataRate);
	status = dot11f_pack_tdls_setup_rsp(pMac, &tdlsSetupRsp,
					    pFrame + header_offset,
					    nPayload, &nPayload);

	if (DOT11F_FAILED(status)) {
		pe_err("Failed to pack a TDLS Setup Response (0x%08x)",
			status);
		cds_packet_free((void *)pPacket);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_warn("There were warnings while packing TDLS Setup Response (0x%08x)",
			status);
	}

	/* Copy the additional IE. */
	/* TODO : addIe is added at the end of the frame. This means it doesnt */
	/* follow the order. This should be ok, but we should consider changing this */
	/* if there is any IOT issue. */
	if (addIeLen != 0) {
		qdf_mem_copy(pFrame + header_offset + nPayload, addIe,
			     addIeLen);
	}

	pe_debug("[TDLS] action: %d (%s) -AP-> OTA peer="MAC_ADDRESS_STR,
		SIR_MAC_TDLS_SETUP_RSP,
		lim_trace_tdls_action_string(SIR_MAC_TDLS_SETUP_RSP),
		MAC_ADDR_ARRAY(peer_mac.bytes));

	pMac->lim.tdls_frm_session_id = psessionEntry->smeSessionId;

	qdf_status = wma_tx_frame_with_tx_complete_send(pMac, pPacket,
						     (uint16_t) nBytes,
						     TID_AC_VI,
						     pFrame,
						     smeSessionId, true);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pMac->lim.tdls_frm_session_id = NO_SESSION;
		pe_err("could not send TDLS Dis Request frame!");
		return eSIR_FAILURE;
	}

	return eSIR_SUCCESS;
}

/*
 * Send TDLS setup CNF frame on AP link
 */
static
tSirRetStatus lim_send_tdls_link_setup_cnf_frame(tpAniSirGlobal pMac,
						 struct qdf_mac_addr peer_mac,
						 uint8_t dialog,
						 uint32_t peerCapability,
						 tpPESession psessionEntry,
						 uint8_t *addIe, uint16_t addIeLen)
{
	tDot11fTDLSSetupCnf tdlsSetupCnf;
	uint32_t status = 0;
	uint32_t nPayload = 0;
	uint32_t nBytes = 0;
	uint32_t header_offset = 0;
	uint8_t *pFrame;
	void *pPacket;
	QDF_STATUS qdf_status;
#ifndef NO_PAD_TDLS_MIN_8023_SIZE
	uint32_t padLen = 0;
#endif
	uint8_t smeSessionId = 0;

	/*
	 * The scheme here is to fill out a 'tDot11fProbeRequest' structure
	 * and then hand it off to 'dot11f_pack_probe_request' (for
	 * serialization).  We start by zero-initializing the structure:
	 */
	smeSessionId = psessionEntry->smeSessionId;

	qdf_mem_set((uint8_t *) &tdlsSetupCnf, sizeof(tDot11fTDLSSetupCnf), 0);

	/*
	 * setup Fixed fields,
	 */
	tdlsSetupCnf.Category.category = SIR_MAC_ACTION_TDLS;
	tdlsSetupCnf.Action.action = SIR_MAC_TDLS_SETUP_CNF;
	tdlsSetupCnf.DialogToken.token = dialog;

	populate_dot11f_link_iden(pMac, psessionEntry,
				  &tdlsSetupCnf.LinkIdentifier, peer_mac,
				  TDLS_INITIATOR);
	/*
	 * TODO: we need to see if we have to support conditions where we have
	 * EDCA parameter info element is needed a) if we need different QOS
	 * parameters for off channel operations or QOS is not supported on
	 * AP link and we wanted to QOS on direct link.
	 */

	/* Check self and peer WMM capable */
	if ((1 == pMac->lim.gLimTDLSWmmMode) &&
	    (CHECK_BIT(peerCapability, TDLS_PEER_WMM_CAP))) {
		pe_debug("populate WMM praram in Setup Confirm");
		populate_dot11f_wmm_params(pMac, &tdlsSetupCnf.WMMParams,
					   psessionEntry);
	}

	/* Check peer is VHT capable */
	if (CHECK_BIT(peerCapability, TDLS_PEER_VHT_CAP)) {
		populate_dot11f_vht_operation(pMac,
					      psessionEntry,
					      &tdlsSetupCnf.VHTOperation);
		populate_dot11f_ht_info(pMac, &tdlsSetupCnf.HTInfo, psessionEntry);
	} else if (CHECK_BIT(peerCapability, TDLS_PEER_HT_CAP)) {       /* Check peer is HT capable */
		populate_dot11f_ht_info(pMac, &tdlsSetupCnf.HTInfo, psessionEntry);
	}

	/*
	 * now we pack it.  First, how much space are we going to need?
	 */
	status = dot11f_get_packed_tdls_setup_cnf_size(pMac, &tdlsSetupCnf,
						       &nPayload);
	if (DOT11F_FAILED(status)) {
		pe_err("Failed to calculate the packed size for a Setup Confirm (0x%08x)",
			status);
		/* We'll fall back on the worst case scenario: */
		nPayload = sizeof(tDot11fProbeRequest);
	} else if (DOT11F_WARNED(status)) {
		pe_warn("There were warnings while calculating the packed size for Setup Confirm (0x%08x)",
			status);
	}

	/*
	 * This frame is going out from PE as data frames with special ethertype
	 * 89-0d.
	 * 8 bytes of RFC 1042 header
	 */

	nBytes = nPayload + ((IS_QOS_ENABLED(psessionEntry))
			     ? sizeof(tSirMacDataHdr3a) :
			     sizeof(tSirMacMgmtHdr))
		 + sizeof(eth_890d_header)
		 + PAYLOAD_TYPE_TDLS_SIZE + addIeLen;

#ifndef NO_PAD_TDLS_MIN_8023_SIZE
	/* IOT issue with some AP : some AP doesn't like the data packet size < minimum 802.3 frame length (64)
	   Hence AP itself padding some bytes, which caused teardown packet is dropped at
	   receiver side. To avoid such IOT issue, we added some extra bytes to meet data frame size >= 64
	 */
	if (nPayload + PAYLOAD_TYPE_TDLS_SIZE < MIN_IEEE_8023_SIZE) {
		padLen =
			MIN_IEEE_8023_SIZE - (nPayload + PAYLOAD_TYPE_TDLS_SIZE);

		/* if padLen is less than minimum vendorSpecific (5), pad up to 5 */
		if (padLen < MIN_VENDOR_SPECIFIC_IE_SIZE)
			padLen = MIN_VENDOR_SPECIFIC_IE_SIZE;

		nBytes += padLen;
	}
#endif

	/* Ok-- try to allocate memory from MGMT PKT pool */
	qdf_status = cds_packet_alloc((uint16_t) nBytes, (void **)&pFrame,
				      (void **)&pPacket);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pe_err("Failed to allocate %d bytes for a TDLS Setup Confirm",
			nBytes);
		return eSIR_MEM_ALLOC_FAILED;
	}

	/* zero out the memory */
	qdf_mem_set(pFrame, nBytes, 0);

	/*
	 * IE formation, memory allocation is completed, Now form TDLS discovery
	 * request frame
	 */

	/* fill out the buffer descriptor */

	header_offset = lim_prepare_tdls_frame_header(pMac, pFrame,
						      LINK_IDEN_ADDR_OFFSET
							      (tdlsSetupCnf), TDLS_LINK_AP,
						      TDLS_INITIATOR, TID_AC_VI,
						      psessionEntry);

	status = dot11f_pack_tdls_setup_cnf(pMac, &tdlsSetupCnf, pFrame
					    + header_offset, nPayload, &nPayload);

	if (DOT11F_FAILED(status)) {
		pe_err("Failed to pack a TDLS discovery req (0x%08x)", status);
		cds_packet_free((void *)pPacket);
		return eSIR_FAILURE;
	} else if (DOT11F_WARNED(status)) {
		pe_warn("There were warnings while packing TDLS Discovery Request (0x%08x)",
			status);
	}

	/* Copy the additional IE. */
	/* TODO : addIe is added at the end of the frame. This means it doesnt */
	/* follow the order. This should be ok, but we should consider changing this */
	/* if there is any IOT issue. */
	if (addIeLen != 0) {
		qdf_mem_copy(pFrame + header_offset + nPayload, addIe,
			     addIeLen);
	}
#ifndef NO_PAD_TDLS_MIN_8023_SIZE
	if (padLen != 0) {
		/* QCOM VENDOR OUI = { 0x00, 0xA0, 0xC6, type = 0x0000 }; */
		uint8_t *padVendorSpecific =
			pFrame + header_offset + nPayload + addIeLen;
		/* make QCOM_VENDOR_OUI, and type = 0x0000, and all the payload to be zero */
		padVendorSpecific[0] = 221;
		padVendorSpecific[1] = padLen - 2;
		padVendorSpecific[2] = 0x00;
		padVendorSpecific[3] = 0xA0;
		padVendorSpecific[4] = 0xC6;

		pe_debug("Padding Vendor Specific Ie Len: %d", padLen);

		/* padding zero if more than 5 bytes are required */
		if (padLen > MIN_VENDOR_SPECIFIC_IE_SIZE)
			qdf_mem_set(pFrame + header_offset + nPayload +
				    addIeLen + MIN_VENDOR_SPECIFIC_IE_SIZE,
				    padLen - MIN_VENDOR_SPECIFIC_IE_SIZE, 0);
	}
#endif

	pe_debug("[TDLS] action: %d (%s) -AP-> OTA peer="MAC_ADDRESS_STR,
		SIR_MAC_TDLS_SETUP_CNF,
		lim_trace_tdls_action_string(SIR_MAC_TDLS_SETUP_CNF),
	       MAC_ADDR_ARRAY(peer_mac.bytes));

	pMac->lim.tdls_frm_session_id = psessionEntry->smeSessionId;

	qdf_status = wma_tx_frame_with_tx_complete_send(pMac, pPacket,
						     (uint16_t) nBytes,
						     TID_AC_VI,
						     pFrame,
						     smeSessionId, true);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		pMac->lim.tdls_frm_session_id = NO_SESSION;
		pe_err("could not send TDLS Setup Confirm frame");
		return eSIR_FAILURE;

	}

	return eSIR_SUCCESS;
}

/* This Function is similar to populate_dot11f_ht_caps, except that the HT Capabilities
 * are considered from the AddStaReq rather from the cfg.dat as in populate_dot11f_ht_caps
 */
static tSirRetStatus lim_tdls_populate_dot11f_ht_caps(tpAniSirGlobal pMac,
						      tpPESession psessionEntry,
						      tSirTdlsAddStaReq *
						      pTdlsAddStaReq,
						      tDot11fIEHTCaps *pDot11f)
{
	uint32_t nCfgValue;
	uint8_t nCfgValue8;
	tSirMacHTParametersInfo *pHTParametersInfo;
	union {
		uint16_t nCfgValue16;
		tSirMacHTCapabilityInfo htCapInfo;
		tSirMacExtendedHTCapabilityInfo extHtCapInfo;
	} uHTCapabilityInfo;

	tSirMacTxBFCapabilityInfo *pTxBFCapabilityInfo;
	tSirMacASCapabilityInfo *pASCapabilityInfo;

	nCfgValue = pTdlsAddStaReq->htCap.capInfo;

	uHTCapabilityInfo.nCfgValue16 = nCfgValue & 0xFFFF;

	pDot11f->advCodingCap = uHTCapabilityInfo.htCapInfo.advCodingCap;
	pDot11f->mimoPowerSave = uHTCapabilityInfo.htCapInfo.mimoPowerSave;
	pDot11f->greenField = uHTCapabilityInfo.htCapInfo.greenField;
	pDot11f->shortGI20MHz = uHTCapabilityInfo.htCapInfo.shortGI20MHz;
	pDot11f->shortGI40MHz = uHTCapabilityInfo.htCapInfo.shortGI40MHz;
	pDot11f->txSTBC = uHTCapabilityInfo.htCapInfo.txSTBC;
	pDot11f->rxSTBC = uHTCapabilityInfo.htCapInfo.rxSTBC;
	pDot11f->delayedBA = uHTCapabilityInfo.htCapInfo.delayedBA;
	pDot11f->maximalAMSDUsize =
		uHTCapabilityInfo.htCapInfo.maximalAMSDUsize;
	pDot11f->dsssCckMode40MHz =
		uHTCapabilityInfo.htCapInfo.dsssCckMode40MHz;
	pDot11f->psmp = uHTCapabilityInfo.htCapInfo.psmp;
	pDot11f->stbcControlFrame =
		uHTCapabilityInfo.htCapInfo.stbcControlFrame;
	pDot11f->lsigTXOPProtection =
		uHTCapabilityInfo.htCapInfo.lsigTXOPProtection;

	/*
	 * All sessionized entries will need the check below
	 * Only in case of NO session
	 */
	if (psessionEntry == NULL) {
		pDot11f->supportedChannelWidthSet =
			uHTCapabilityInfo.htCapInfo.supportedChannelWidthSet;
	} else {
		pDot11f->supportedChannelWidthSet =
			psessionEntry->htSupportedChannelWidthSet;
	}

	/* Ensure that shortGI40MHz is Disabled if supportedChannelWidthSet is
	   eHT_CHANNEL_WIDTH_20MHZ */
	if (pDot11f->supportedChannelWidthSet == eHT_CHANNEL_WIDTH_20MHZ) {
		pDot11f->shortGI40MHz = 0;
	}

	pe_debug("SupportedChnlWidth: %d, mimoPS: %d, GF: %d, shortGI20:%d, shortGI40: %d, dsssCck: %d",
		pDot11f->supportedChannelWidthSet,
		pDot11f->mimoPowerSave,
		pDot11f->greenField,
		pDot11f->shortGI20MHz,
		pDot11f->shortGI40MHz,
		pDot11f->dsssCckMode40MHz);

	nCfgValue = pTdlsAddStaReq->htCap.ampduParamsInfo;

	nCfgValue8 = (uint8_t) nCfgValue;
	pHTParametersInfo = (tSirMacHTParametersInfo *) &nCfgValue8;

	pDot11f->maxRxAMPDUFactor = pHTParametersInfo->maxRxAMPDUFactor;
	pDot11f->mpduDensity = pHTParametersInfo->mpduDensity;
	pDot11f->reserved1 = pHTParametersInfo->reserved;

	pe_debug("AMPDU Param: %x", nCfgValue);

	qdf_mem_copy(pDot11f->supportedMCSSet, pTdlsAddStaReq->htCap.suppMcsSet,
		     SIZE_OF_SUPPORTED_MCS_SET);

	nCfgValue = pTdlsAddStaReq->htCap.extendedHtCapInfo;

	uHTCapabilityInfo.nCfgValue16 = nCfgValue & 0xFFFF;

	pDot11f->pco = uHTCapabilityInfo.extHtCapInfo.pco;
	pDot11f->transitionTime = uHTCapabilityInfo.extHtCapInfo.transitionTime;
	pDot11f->mcsFeedback = uHTCapabilityInfo.extHtCapInfo.mcsFeedback;

	nCfgValue = pTdlsAddStaReq->htCap.txBFCapInfo;

	pTxBFCapabilityInfo = (tSirMacTxBFCapabilityInfo *) &nCfgValue;
	pDot11f->txBF = pTxBFCapabilityInfo->txBF;
	pDot11f->rxStaggeredSounding = pTxBFCapabilityInfo->rxStaggeredSounding;
	pDot11f->txStaggeredSounding = pTxBFCapabilityInfo->txStaggeredSounding;
	pDot11f->rxZLF = pTxBFCapabilityInfo->rxZLF;
	pDot11f->txZLF = pTxBFCapabilityInfo->txZLF;
	pDot11f->implicitTxBF = pTxBFCapabilityInfo->implicitTxBF;
	pDot11f->calibration = pTxBFCapabilityInfo->calibration;
	pDot11f->explicitCSITxBF = pTxBFCapabilityInfo->explicitCSITxBF;
	pDot11f->explicitUncompressedSteeringMatrix =
		pTxBFCapabilityInfo->explicitUncompressedSteeringMatrix;
	pDot11f->explicitBFCSIFeedback =
		pTxBFCapabilityInfo->explicitBFCSIFeedback;
	pDot11f->explicitUncompressedSteeringMatrixFeedback =
		pTxBFCapabilityInfo->explicitUncompressedSteeringMatrixFeedback;
	pDot11f->explicitCompressedSteeringMatrixFeedback =
		pTxBFCapabilityInfo->explicitCompressedSteeringMatrixFeedback;
	pDot11f->csiNumBFAntennae = pTxBFCapabilityInfo->csiNumBFAntennae;
	pDot11f->uncompressedSteeringMatrixBFAntennae =
		pTxBFCapabilityInfo->uncompressedSteeringMatrixBFAntennae;
	pDot11f->compressedSteeringMatrixBFAntennae =
		pTxBFCapabilityInfo->compressedSteeringMatrixBFAntennae;

	nCfgValue = pTdlsAddStaReq->htCap.antennaSelectionInfo;

	nCfgValue8 = (uint8_t) nCfgValue;

	pASCapabilityInfo = (tSirMacASCapabilityInfo *) &nCfgValue8;
	pDot11f->antennaSelection = pASCapabilityInfo->antennaSelection;
	pDot11f->explicitCSIFeedbackTx =
		pASCapabilityInfo->explicitCSIFeedbackTx;
	pDot11f->antennaIndicesFeedbackTx =
		pASCapabilityInfo->antennaIndicesFeedbackTx;
	pDot11f->explicitCSIFeedback = pASCapabilityInfo->explicitCSIFeedback;
	pDot11f->antennaIndicesFeedback =
		pASCapabilityInfo->antennaIndicesFeedback;
	pDot11f->rxAS = pASCapabilityInfo->rxAS;
	pDot11f->txSoundingPPDUs = pASCapabilityInfo->txSoundingPPDUs;

	pDot11f->present = pTdlsAddStaReq->htcap_present;

	return eSIR_SUCCESS;

}

static tSirRetStatus
lim_tdls_populate_dot11f_vht_caps(tpAniSirGlobal pMac,
				  tSirTdlsAddStaReq *pTdlsAddStaReq,
				  tDot11fIEVHTCaps *pDot11f)
{
	uint32_t nCfgValue = 0;
	union {
		uint32_t nCfgValue32;
		tSirMacVHTCapabilityInfo vhtCapInfo;
	} uVHTCapabilityInfo;
	union {
		uint16_t nCfgValue16;
		tSirMacVHTTxSupDataRateInfo vhtTxSupDataRateInfo;
		tSirMacVHTRxSupDataRateInfo vhtRxsupDataRateInfo;
	} uVHTSupDataRateInfo;

	pDot11f->present = pTdlsAddStaReq->vhtcap_present;

	nCfgValue = pTdlsAddStaReq->vhtCap.vhtCapInfo;
	uVHTCapabilityInfo.nCfgValue32 = nCfgValue;

	pDot11f->maxMPDULen = uVHTCapabilityInfo.vhtCapInfo.maxMPDULen;
	pDot11f->supportedChannelWidthSet =
		uVHTCapabilityInfo.vhtCapInfo.supportedChannelWidthSet;
	pDot11f->ldpcCodingCap = uVHTCapabilityInfo.vhtCapInfo.ldpcCodingCap;
	pDot11f->shortGI80MHz = uVHTCapabilityInfo.vhtCapInfo.shortGI80MHz;
	pDot11f->shortGI160and80plus80MHz =
		uVHTCapabilityInfo.vhtCapInfo.shortGI160and80plus80MHz;
	pDot11f->txSTBC = uVHTCapabilityInfo.vhtCapInfo.txSTBC;
	pDot11f->rxSTBC = uVHTCapabilityInfo.vhtCapInfo.rxSTBC;
	pDot11f->suBeamFormerCap = 0;
	pDot11f->suBeamformeeCap = 0;
	pDot11f->csnofBeamformerAntSup =
		uVHTCapabilityInfo.vhtCapInfo.csnofBeamformerAntSup;
	pDot11f->numSoundingDim = uVHTCapabilityInfo.vhtCapInfo.numSoundingDim;
	pDot11f->muBeamformerCap = 0;
	pDot11f->muBeamformeeCap = 0;
	pDot11f->vhtTXOPPS = uVHTCapabilityInfo.vhtCapInfo.vhtTXOPPS;
	pDot11f->htcVHTCap = uVHTCapabilityInfo.vhtCapInfo.htcVHTCap;
	pDot11f->maxAMPDULenExp = uVHTCapabilityInfo.vhtCapInfo.maxAMPDULenExp;
	pDot11f->vhtLinkAdaptCap =
		uVHTCapabilityInfo.vhtCapInfo.vhtLinkAdaptCap;
	pDot11f->rxAntPattern = uVHTCapabilityInfo.vhtCapInfo.rxAntPattern;
	pDot11f->txAntPattern = uVHTCapabilityInfo.vhtCapInfo.txAntPattern;
	pDot11f->reserved1 = uVHTCapabilityInfo.vhtCapInfo.reserved1;

	pDot11f->rxMCSMap = pTdlsAddStaReq->vhtCap.suppMcs.rxMcsMap;

	nCfgValue = pTdlsAddStaReq->vhtCap.suppMcs.rxHighest;
	uVHTSupDataRateInfo.nCfgValue16 = nCfgValue & 0xffff;
	pDot11f->rxHighSupDataRate =
		uVHTSupDataRateInfo.vhtRxsupDataRateInfo.rxSupDataRate;

	pDot11f->txMCSMap = pTdlsAddStaReq->vhtCap.suppMcs.txMcsMap;

	nCfgValue = pTdlsAddStaReq->vhtCap.suppMcs.txHighest;
	uVHTSupDataRateInfo.nCfgValue16 = nCfgValue & 0xffff;
	pDot11f->txSupDataRate =
		uVHTSupDataRateInfo.vhtTxSupDataRateInfo.txSupDataRate;

	pDot11f->reserved3 = uVHTSupDataRateInfo.vhtTxSupDataRateInfo.reserved;

	lim_log_vht_cap(pMac, pDot11f);

	return eSIR_SUCCESS;

}

/**
 * lim_tdls_populate_matching_rate_set() - populate matching rate set
 *
 * @mac_ctx  - global MAC context
 * @stads - station hash entry
 * @supp_rate_set - pointer to supported rate set
 * @supp_rates_len - length of the supported rates
 * @supp_mcs_set - pointer to supported MSC set
 * @session_entry - pointer to PE session entry
 * @vht_caps - pointer to VHT capability
 *
 *
 * This function gets set of available rates from the config and compare them
 * against the set of received supported rates. After the comparison station
 * entry's rates is populated with 11A rates and 11B rates.
 *
 * Return: eSIR_SUCCESS on success, eSIR_FAILURE on failure.
 */
static tSirRetStatus
lim_tdls_populate_matching_rate_set(tpAniSirGlobal mac_ctx, tpDphHashNode stads,
				    uint8_t *supp_rate_set,
				    uint8_t supp_rates_len,
				    uint8_t *supp_mcs_set,
				    tpPESession session_entry,
				    tDot11fIEVHTCaps *vht_caps)
{
	tSirMacRateSet temp_rate_set;
	uint32_t i, j, val, min, is_a_rate;
	tSirMacRateSet temp_rate_set2;
	uint32_t phymode;
	uint8_t mcsSet[SIZE_OF_SUPPORTED_MCS_SET];
	tpSirSupportedRates rates;
	uint8_t a_rateindex = 0;
	uint8_t b_rateindex = 0;
	uint8_t nss;
	is_a_rate = 0;
	temp_rate_set2.numRates = 0;

	lim_get_phy_mode(mac_ctx, &phymode, NULL);

	/* get own rate set */
	val = WNI_CFG_OPERATIONAL_RATE_SET_LEN;
	if (wlan_cfg_get_str(mac_ctx, WNI_CFG_OPERATIONAL_RATE_SET,
			     (uint8_t *) &temp_rate_set.rate,
			     &val) != eSIR_SUCCESS) {
		/* Could not get rateset from CFG. Log error. */
		pe_err("could not retrieve rateset");
		val = 0;
	}
	temp_rate_set.numRates = val;

	if (phymode == WNI_CFG_PHY_MODE_11G) {
		/* get own extended rate set */
		val = WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET_LEN;
		if (wlan_cfg_get_str(mac_ctx,
				     WNI_CFG_EXTENDED_OPERATIONAL_RATE_SET,
				     (uint8_t *) &temp_rate_set2.rate,
				     &val) != eSIR_SUCCESS)
			temp_rate_set2.numRates = val;
	}

	if ((temp_rate_set.numRates + temp_rate_set2.numRates) > 12) {
		pe_err("more than 12 rates in CFG");
		return eSIR_FAILURE;
	}

	/**
	 * Handling of the rate set IEs is the following:
	 * - keep only rates that we support and that the station supports
	 * - sort and the rates into the pSta->rate array
	 */

	/* Copy all rates in temp_rate_set, there are 12 rates max */
	for (i = 0; i < temp_rate_set2.numRates; i++)
		temp_rate_set.rate[i + temp_rate_set.numRates] =
			temp_rate_set2.rate[i];

	temp_rate_set.numRates += temp_rate_set2.numRates;

	/**
	 * Sort rates in temp_rate_set (they are likely to be already sorted)
	 * put the result in temp_rate_set2
	 */
	temp_rate_set2.numRates = 0;

	for (i = 0; i < temp_rate_set.numRates; i++) {
		min = 0;
		val = 0xff;

		for (j = 0; j < temp_rate_set.numRates; j++)
			if ((uint32_t) (temp_rate_set.rate[j] & 0x7f) < val) {
				val = temp_rate_set.rate[j] & 0x7f;
				min = j;
			}

		temp_rate_set2.rate[temp_rate_set2.numRates++] =
			temp_rate_set.rate[min];
		temp_rate_set.rate[min] = 0xff;
	}

	/**
	 * Copy received rates in temp_rate_set, the parser has ensured
	 * unicity of the rates so there cannot be more than 12 .
	 */
	if (supp_rates_len > SIR_MAC_RATESET_EID_MAX) {
		pe_warn("Supported rates length: %d more than the Max limit, reset to Max",
			supp_rates_len);
		supp_rates_len = SIR_MAC_RATESET_EID_MAX;
	}

	for (i = 0; i < supp_rates_len; i++)
		temp_rate_set.rate[i] = supp_rate_set[i];

	temp_rate_set.numRates = supp_rates_len;

	rates = &stads->supportedRates;
	qdf_mem_set((uint8_t *) rates, sizeof(tSirSupportedRates), 0);

	for (i = 0; i < temp_rate_set2.numRates; i++) {
		for (j = 0; j < temp_rate_set.numRates; j++) {
			if ((temp_rate_set2.rate[i] & 0x7F) !=
				(temp_rate_set.rate[j] & 0x7F))
				continue;

			if ((b_rateindex > SIR_NUM_11B_RATES) ||
			    (a_rateindex > SIR_NUM_11A_RATES)) {
				pe_warn("Invalid number of rates (11b->%d, 11a->%d)",
					b_rateindex, a_rateindex);
				return eSIR_FAILURE;
			}
			if (sirIsArate(temp_rate_set2.rate[i] & 0x7f)) {
				is_a_rate = 1;
				if (a_rateindex < SIR_NUM_11A_RATES)
					rates->llaRates[a_rateindex++] = temp_rate_set2.rate[i];
			} else {
				if (b_rateindex < SIR_NUM_11B_RATES)
					rates->llbRates[b_rateindex++] = temp_rate_set2.rate[i];
			}
			break;
		}
	}

	if (IS_5G_CH(session_entry->currentOperChannel))
		nss = mac_ctx->vdev_type_nss_5g.tdls;
	else
		nss = mac_ctx->vdev_type_nss_2g.tdls;

	nss = QDF_MIN(nss, mac_ctx->user_configured_nss);

	/* compute the matching MCS rate set, if peer is 11n capable and self mode is 11n */
#ifdef FEATURE_WLAN_TDLS
	if (stads->mlmStaContext.htCapability)
#else
	if (IS_DOT11_MODE_HT(session_entry->dot11mode) &&
	    (stads->mlmStaContext.htCapability))
#endif
	{
		val = SIZE_OF_SUPPORTED_MCS_SET;
		if (wlan_cfg_get_str(mac_ctx, WNI_CFG_SUPPORTED_MCS_SET,
				     mcsSet, &val) != eSIR_SUCCESS) {
			/* Could not get rateset from CFG. Log error. */
			pe_err("could not retrieve supportedMCSSet");
			return eSIR_FAILURE;
		}

		if (NSS_1x1_MODE == nss)
			mcsSet[1] = 0;
		for (i = 0; i < val; i++)
			stads->supportedRates.supportedMCSSet[i] =
				mcsSet[i] & supp_mcs_set[i];

		pe_debug("MCS Rate Set Bitmap from CFG and DPH");
		for (i = 0; i < SIR_MAC_MAX_SUPPORTED_MCS_SET; i++) {
			pe_debug("%x %x", mcsSet[i],
				stads->supportedRates.supportedMCSSet[i]);
		}
	}
	lim_populate_vht_mcs_set(mac_ctx, &stads->supportedRates, vht_caps,
				 session_entry, nss);
	/**
	 * Set the erpEnabled bit if the phy is in G mode and at least
	 * one A rate is supported
	 */
	if ((phymode == WNI_CFG_PHY_MODE_11G) && is_a_rate)
		stads->erpEnabled = eHAL_SET;

	return eSIR_SUCCESS;
}

/*
 * update HASH node entry info
 */
static void lim_tdls_update_hash_node_info(tpAniSirGlobal pMac,
					   tDphHashNode *pStaDs,
					   tSirTdlsAddStaReq *pTdlsAddStaReq,
					   tpPESession psessionEntry)
{
	tDot11fIEHTCaps htCap = {0,};
	tDot11fIEHTCaps *htCaps;
	tDot11fIEVHTCaps *pVhtCaps = NULL;
	tDot11fIEVHTCaps *pVhtCaps_txbf = NULL;
	tDot11fIEVHTCaps vhtCap;
	uint8_t cbMode;
	tpDphHashNode pSessStaDs = NULL;
	uint16_t aid;

	if (pTdlsAddStaReq->tdlsAddOper == TDLS_OPER_ADD) {
		populate_dot11f_ht_caps(pMac, psessionEntry, &htCap);
	} else if (pTdlsAddStaReq->tdlsAddOper == TDLS_OPER_UPDATE) {
		lim_tdls_populate_dot11f_ht_caps(pMac, NULL, pTdlsAddStaReq, &htCap);
	}
	htCaps = &htCap;
	if (htCaps->present) {
		pStaDs->mlmStaContext.htCapability = 1;
		pStaDs->htGreenfield = htCaps->greenField;
		/*
		 * pStaDs->htSupportedChannelWidthSet should have the base
		 * channel capability. The htSupportedChannelWidthSet of the
		 * TDLS link on base channel should be less than or equal to
		 * channel width of STA-AP link. So take this setting from the
		 * psessionEntry.
		 */
		pe_debug("supportedChannelWidthSet: 0x%x htSupportedChannelWidthSet: 0x%x",
				htCaps->supportedChannelWidthSet,
				psessionEntry->htSupportedChannelWidthSet);
		pStaDs->htSupportedChannelWidthSet =
				(htCaps->supportedChannelWidthSet <
				 psessionEntry->htSupportedChannelWidthSet) ?
				htCaps->supportedChannelWidthSet :
				psessionEntry->htSupportedChannelWidthSet;
		pe_debug("pStaDs->htSupportedChannelWidthSet: 0x%x",
				pStaDs->htSupportedChannelWidthSet);

		pStaDs->htMIMOPSState = htCaps->mimoPowerSave;
		pStaDs->htMaxAmsduLength = htCaps->maximalAMSDUsize;
		pStaDs->htAMpduDensity = htCaps->mpduDensity;
		pStaDs->htDsssCckRate40MHzSupport = htCaps->dsssCckMode40MHz;
		pStaDs->htShortGI20Mhz = htCaps->shortGI20MHz;
		pStaDs->htShortGI40Mhz = htCaps->shortGI40MHz;
		pStaDs->htMaxRxAMpduFactor = htCaps->maxRxAMPDUFactor;
		lim_fill_rx_highest_supported_rate(pMac,
						   &pStaDs->supportedRates.
						   rxHighestDataRate,
						   htCaps->supportedMCSSet);
		pStaDs->baPolicyFlag = 0xFF;
		pMac->lim.gLimTdlsLinkMode = TDLS_LINK_MODE_N;
		pStaDs->ht_caps = pTdlsAddStaReq->htCap.capInfo;
	} else {
		pStaDs->mlmStaContext.htCapability = 0;
		pMac->lim.gLimTdlsLinkMode = TDLS_LINK_MODE_BG;
	}
	lim_tdls_populate_dot11f_vht_caps(pMac, pTdlsAddStaReq, &vhtCap);
	pVhtCaps = &vhtCap;
	if (pVhtCaps->present) {
		pStaDs->mlmStaContext.vhtCapability = 1;

		/*
		 * 11.21.1 General: The channel width of the TDLS direct
		 * link on the base channel shall not exceed the channel
		 * width of the BSS to which the TDLS peer STAs are
		 * associated.
		 */
		if (psessionEntry->ch_width)
			pStaDs->vhtSupportedChannelWidthSet =
					psessionEntry->ch_width - 1;
		else
			pStaDs->vhtSupportedChannelWidthSet =
					psessionEntry->ch_width;

		pe_debug("vhtSupportedChannelWidthSet: %hu htSupportedChannelWidthSet: %hu",
			pStaDs->vhtSupportedChannelWidthSet,
			pStaDs->htSupportedChannelWidthSet);

		pStaDs->vhtLdpcCapable = pVhtCaps->ldpcCodingCap;
		pStaDs->vhtBeamFormerCapable = 0;
		pMac->lim.gLimTdlsLinkMode = TDLS_LINK_MODE_AC;
		pVhtCaps_txbf = (tDot11fIEVHTCaps *) (&pTdlsAddStaReq->vhtCap);
		pVhtCaps_txbf->suBeamformeeCap = 0;
		pVhtCaps_txbf->suBeamFormerCap = 0;
		pVhtCaps_txbf->muBeamformerCap = 0;
		pVhtCaps_txbf->muBeamformeeCap = 0;
		pStaDs->vht_caps = pTdlsAddStaReq->vhtCap.vhtCapInfo;
	} else {
		pStaDs->mlmStaContext.vhtCapability = 0;
		pStaDs->vhtSupportedChannelWidthSet =
			WNI_CFG_VHT_CHANNEL_WIDTH_20_40MHZ;
	}

	/*
	 * Calculate the Secondary Coannel Offset if our
	 * own channel bonding state is enabled
	 */
	if (psessionEntry->htSupportedChannelWidthSet) {
		cbMode = lim_select_cb_mode(pStaDs, psessionEntry,
				    psessionEntry->currentOperChannel,
				    pStaDs->vhtSupportedChannelWidthSet);

		if (pStaDs->mlmStaContext.vhtCapability)
			pStaDs->htSecondaryChannelOffset =
					lim_get_htcb_state(cbMode);
		else
			pStaDs->htSecondaryChannelOffset = cbMode;
	}
	pSessStaDs = dph_lookup_hash_entry(pMac, psessionEntry->bssId, &aid,
					   &psessionEntry->dph.dphHashTable);
	/* Lets enable QOS parameter */
	pStaDs->qosMode = (pTdlsAddStaReq->capability & CAPABILITIES_QOS_OFFSET)
				|| pTdlsAddStaReq->htcap_present;
	pStaDs->wmeEnabled = 1;
	pStaDs->lleEnabled = 0;
	/*  TDLS Dummy AddSTA does not have qosInfo , is it OK ??
	 */
	pStaDs->qos.capability.qosInfo =
		(*(tSirMacQosInfoStation *) &pTdlsAddStaReq->uapsd_queues);

	/* populate matching rate set */

	/* TDLS Dummy AddSTA does not have HTCap,VHTCap,Rates info , is it OK ??
	 */
	lim_tdls_populate_matching_rate_set(pMac, pStaDs,
					    pTdlsAddStaReq->supported_rates,
					    pTdlsAddStaReq->supported_rates_length,
					    (uint8_t *) pTdlsAddStaReq->htCap.
					    suppMcsSet, psessionEntry, pVhtCaps);

	/*  TDLS Dummy AddSTA does not have right capability , is it OK ??
	 */
	pStaDs->mlmStaContext.capabilityInfo =
		(*(tSirMacCapabilityInfo *) &pTdlsAddStaReq->capability);

	return;
}

/*
 * Add STA for TDLS setup procedure
 */
static tSirRetStatus lim_tdls_setup_add_sta(tpAniSirGlobal pMac,
					    tSirTdlsAddStaReq *pAddStaReq,
					    tpPESession psessionEntry)
{
	tpDphHashNode pStaDs = NULL;
	tSirRetStatus status = eSIR_SUCCESS;
	uint16_t aid = 0;

	pStaDs = dph_lookup_hash_entry(pMac, pAddStaReq->peermac.bytes, &aid,
				       &psessionEntry->dph.dphHashTable);
	if (NULL == pStaDs) {
		aid = lim_assign_peer_idx(pMac, psessionEntry);

		if (!aid) {
			pe_err("No more free AID for peer: "MAC_ADDRESS_STR,
				MAC_ADDR_ARRAY(pAddStaReq->peermac.bytes));
			return eSIR_FAILURE;
		}

		/* Set the aid in peerAIDBitmap as it has been assigned to TDLS peer */
		SET_PEER_AID_BITMAP(psessionEntry->peerAIDBitmap, aid);

		pe_debug("Aid: %d, for peer: " MAC_ADDRESS_STR,
			aid, MAC_ADDR_ARRAY(pAddStaReq->peermac.bytes));
		pStaDs =
			dph_get_hash_entry(pMac, aid,
					   &psessionEntry->dph.dphHashTable);

		if (pStaDs) {
			(void)lim_del_sta(pMac, pStaDs, false /*asynchronous */,
					  psessionEntry);
			lim_delete_dph_hash_entry(pMac, pStaDs->staAddr, aid,
						  psessionEntry);
		}

		pStaDs = dph_add_hash_entry(pMac, pAddStaReq->peermac.bytes,
					 aid, &psessionEntry->dph.dphHashTable);

		if (NULL == pStaDs) {
			pe_err("add hash entry failed");
			QDF_ASSERT(0);
			return eSIR_FAILURE;
		}
	}

	lim_tdls_update_hash_node_info(pMac, pStaDs, pAddStaReq, psessionEntry);

	pStaDs->staType = STA_ENTRY_TDLS_PEER;

	status =
		lim_add_sta(pMac, pStaDs,
			    (pAddStaReq->tdlsAddOper ==
			     TDLS_OPER_UPDATE) ? true : false, psessionEntry);

	if (eSIR_SUCCESS != status) {
		/* should not fail */
		QDF_ASSERT(0);
	}
	return status;
}

/*
 * Del STA, after Link is teardown or discovery response sent on direct link
 */
static tpDphHashNode lim_tdls_del_sta(tpAniSirGlobal pMac,
				      struct qdf_mac_addr peerMac,
				      tpPESession psessionEntry,
				      bool resp_reqd)
{
	tSirRetStatus status = eSIR_SUCCESS;
	uint16_t peerIdx = 0;
	tpDphHashNode pStaDs = NULL;

	pStaDs = dph_lookup_hash_entry(pMac, peerMac.bytes, &peerIdx,
				       &psessionEntry->dph.dphHashTable);

	if (pStaDs) {

		pe_debug("DEL STA peer MAC: "MAC_ADDRESS_STR,
		       MAC_ADDR_ARRAY(pStaDs->staAddr));

		pe_debug("STA type: %x, sta idx: %x resp_reqd: %d",
		       pStaDs->staType,
		       pStaDs->staIndex,
		       resp_reqd);

		status = lim_del_sta(pMac, pStaDs, resp_reqd, psessionEntry);
	}

	return pStaDs;
}

/*
 * Once Link is setup with PEER, send Add STA ind to SME
 */
static QDF_STATUS lim_send_sme_tdls_add_sta_rsp(tpAniSirGlobal pMac,
						uint8_t sessionId,
						tSirMacAddr peerMac,
						uint8_t updateSta,
						tDphHashNode *pStaDs, uint8_t status)
{
	struct scheduler_msg mmhMsg = { 0 };
	tSirTdlsAddStaRsp *addStaRsp = NULL;
	mmhMsg.type = eWNI_SME_TDLS_ADD_STA_RSP;

	addStaRsp = qdf_mem_malloc(sizeof(tSirTdlsAddStaRsp));
	if (NULL == addStaRsp) {
		pe_err("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}

	addStaRsp->sessionId = sessionId;
	addStaRsp->statusCode = status;
	if (pStaDs) {
		addStaRsp->staId = pStaDs->staIndex;
		addStaRsp->ucastSig = pStaDs->ucUcastSig;
		addStaRsp->bcastSig = pStaDs->ucBcastSig;
	}
	if (peerMac) {
		qdf_mem_copy(addStaRsp->peermac.bytes,
			     (uint8_t *) peerMac, QDF_MAC_ADDR_SIZE);
	}
	if (updateSta)
		addStaRsp->tdlsAddOper = TDLS_OPER_UPDATE;
	else
		addStaRsp->tdlsAddOper = TDLS_OPER_ADD;

	addStaRsp->length = sizeof(tSirTdlsAddStaRsp);
	addStaRsp->messageType = eWNI_SME_TDLS_ADD_STA_RSP;
#ifdef CONVERGED_TDLS_ENABLE
	addStaRsp->psoc = pMac->psoc;
	mmhMsg.bodyptr = addStaRsp;
	mmhMsg.callback = tgt_tdls_add_peer_rsp;

	return scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &mmhMsg);
#else
	mmhMsg.bodyptr = addStaRsp;
	mmhMsg.bodyval = 0;
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
#endif
	return QDF_STATUS_SUCCESS;
}

/*
 * STA RSP received from HAL
 */
QDF_STATUS lim_process_tdls_add_sta_rsp(tpAniSirGlobal pMac, void *msg,
					tpPESession psessionEntry)
{
	tAddStaParams *pAddStaParams = (tAddStaParams *) msg;
	uint8_t status = eSIR_SUCCESS;
	tDphHashNode *pStaDs = NULL;
	uint16_t aid = 0;

	SET_LIM_PROCESS_DEFD_MESGS(pMac, true);
	pe_debug("staIdx: %d, staMac: "MAC_ADDRESS_STR,
	       pAddStaParams->staIdx,
	       MAC_ADDR_ARRAY(pAddStaParams->staMac));

	if (pAddStaParams->status != QDF_STATUS_SUCCESS) {
		QDF_ASSERT(0);
		pe_err("Add sta failed ");
		status = eSIR_FAILURE;
		goto add_sta_error;
	}

	pStaDs = dph_lookup_hash_entry(pMac, pAddStaParams->staMac, &aid,
				       &psessionEntry->dph.dphHashTable);
	if (NULL == pStaDs) {
		pe_err("pStaDs is NULL ");
		status = eSIR_FAILURE;
		goto add_sta_error;
	}

	pStaDs->bssId = pAddStaParams->bssIdx;
	pStaDs->staIndex = pAddStaParams->staIdx;
	pStaDs->ucUcastSig = pAddStaParams->ucUcastSig;
	pStaDs->ucBcastSig = pAddStaParams->ucBcastSig;
	pStaDs->mlmStaContext.mlmState = eLIM_MLM_LINK_ESTABLISHED_STATE;
	pStaDs->valid = 1;
add_sta_error:
	status = lim_send_sme_tdls_add_sta_rsp(pMac, psessionEntry->smeSessionId,
					       pAddStaParams->staMac,
					       pAddStaParams->updateSta, pStaDs,
					       status);
	qdf_mem_free(pAddStaParams);
	return status;
}

#ifdef CONVERGED_TDLS_ENABLE
/**
 * lim_send_tdls_comp_mgmt_rsp() - Send Response to upper layers
 * @mac_ctx:          Pointer to Global MAC structure
 * @msg_type:         Indicates message type
 * @result_code:       Indicates the result of previously issued
 *                    eWNI_SME_msg_type_REQ message
 *
 * This function is called by lim_process_sme_req_messages() to send
 * eWNI_SME_START_RSP, eWNI_SME_STOP_BSS_RSP
 * or eWNI_SME_SWITCH_CHL_RSP messages to applications above MAC
 * Software.
 *
 * Return: None
 */

static void
lim_send_tdls_comp_mgmt_rsp(tpAniSirGlobal mac_ctx, uint16_t msg_type,
	 tSirResultCodes result_code, uint8_t sme_session_id,
	 uint16_t sme_transaction_id)
{
	struct scheduler_msg msg = {0};
	tSirSmeRsp *sme_rsp;

	pe_debug("Sending message %s with reasonCode %s",
		lim_msg_str(msg_type), lim_result_code_str(result_code));

	sme_rsp = qdf_mem_malloc(sizeof(tSirSmeRsp));
	if (NULL == sme_rsp) {
		/* Buffer not available. Log error */
		QDF_TRACE(QDF_MODULE_ID_PE, LOGP,
			FL("call to AllocateMemory failed for eWNI_SME_*_RSP"));
		return;
	}

	sme_rsp->messageType = msg_type;
	sme_rsp->length = sizeof(tSirSmeRsp);
	sme_rsp->statusCode = result_code;

	sme_rsp->sessionId = sme_session_id;
	sme_rsp->transactionId = sme_transaction_id;

	msg.type = msg_type;
	sme_rsp->psoc = mac_ctx->psoc;
	msg.bodyptr = sme_rsp;
	msg.callback = tgt_tdls_send_mgmt_rsp;
	scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);

}
#endif

/**
 * lim_process_sme_tdls_mgmt_send_req() - send out tdls management frames
 *
 * @mac_ctx - global mac context
 * @msg - message buffer received from SME.
 *
 * Process Send Mgmt Request from SME and transmit to AP.
 *
 * Return: eSIR_SUCCESS on success, error code otherwise
 */
tSirRetStatus lim_process_sme_tdls_mgmt_send_req(tpAniSirGlobal mac_ctx,
						 uint32_t *msg)
{
	/* get all discovery request parameters */
	tSirTdlsSendMgmtReq *send_req = (tSirTdlsSendMgmtReq *) msg;
	tpPESession session_entry;
	uint8_t session_id;
	tSirResultCodes result_code = eSIR_SME_INVALID_PARAMETERS;

	pe_debug("Send Mgmt Recieved");
	session_entry = pe_find_session_by_bssid(mac_ctx,
					 send_req->bssid.bytes, &session_id);
	if (NULL == session_entry) {
		pe_err("PE Session does not exist for given sme session_id %d",
			send_req->sessionId);
		goto lim_tdls_send_mgmt_error;
	}

	/* check if we are in proper state to work as TDLS client */
	if (!LIM_IS_STA_ROLE(session_entry)) {
		pe_err("send mgmt received in wrong system Role: %d",
			  GET_LIM_SYSTEM_ROLE(session_entry));
		goto lim_tdls_send_mgmt_error;
	}

	if (lim_is_roam_synch_in_progress(session_entry)) {
		pe_err("roaming in progress, reject mgmt! for session %d",
		       send_req->sessionId);
		result_code = eSIR_SME_REFUSED;
		goto lim_tdls_send_mgmt_error;
	}

	/*
	 * if we are still good, go ahead and check if we are in proper state to
	 * do TDLS discovery req/rsp/....frames.
	 */
	if ((session_entry->limSmeState != eLIM_SME_ASSOCIATED_STATE) &&
	    (session_entry->limSmeState != eLIM_SME_LINK_EST_STATE)) {
		pe_err("send mgmt received in invalid LIMsme state: %d",
			session_entry->limSmeState);
		goto lim_tdls_send_mgmt_error;
	}

	cds_tdls_tx_rx_mgmt_event(SIR_MAC_ACTION_TDLS,
		SIR_MAC_ACTION_TX, SIR_MAC_MGMT_ACTION,
		send_req->reqType, send_req->peer_mac.bytes);

	switch (send_req->reqType) {
	case SIR_MAC_TDLS_DIS_REQ:
		pe_debug("Transmit Discovery Request Frame");
		/* format TDLS discovery request frame and transmit it */
		lim_send_tdls_dis_req_frame(mac_ctx, send_req->peer_mac,
					    send_req->dialog, session_entry);
		result_code = eSIR_SME_SUCCESS;
		break;
	case SIR_MAC_TDLS_DIS_RSP:
		pe_debug("Transmit Discovery Response Frame");
		/* Send a response mgmt action frame */
		lim_send_tdls_dis_rsp_frame(mac_ctx, send_req->peer_mac,
			send_req->dialog, session_entry, &send_req->addIe[0],
			(send_req->length - sizeof(tSirTdlsSendMgmtReq)));
		result_code = eSIR_SME_SUCCESS;
		break;
	case SIR_MAC_TDLS_SETUP_REQ:
		pe_debug("Transmit Setup Request Frame");
		lim_send_tdls_link_setup_req_frame(mac_ctx,
			send_req->peer_mac, send_req->dialog, session_entry,
			&send_req->addIe[0],
			(send_req->length - sizeof(tSirTdlsSendMgmtReq)));
		result_code = eSIR_SME_SUCCESS;
		break;
	case SIR_MAC_TDLS_SETUP_RSP:
		pe_debug("Transmit Setup Response Frame");
		lim_send_tdls_setup_rsp_frame(mac_ctx,
			send_req->peer_mac, send_req->dialog, session_entry,
			send_req->statusCode, &send_req->addIe[0],
			(send_req->length - sizeof(tSirTdlsSendMgmtReq)));
		result_code = eSIR_SME_SUCCESS;
		break;
	case SIR_MAC_TDLS_SETUP_CNF:
		pe_debug("Transmit Setup Confirm Frame");
		lim_send_tdls_link_setup_cnf_frame(mac_ctx,
			send_req->peer_mac, send_req->dialog,
			send_req->peerCapability, session_entry,
			&send_req->addIe[0],
			(send_req->length - sizeof(tSirTdlsSendMgmtReq)));
		result_code = eSIR_SME_SUCCESS;
		break;
	case SIR_MAC_TDLS_TEARDOWN:
		pe_debug("Transmit Teardown Frame");
		lim_send_tdls_teardown_frame(mac_ctx,
			send_req->peer_mac, send_req->statusCode,
			send_req->responder, session_entry,
			&send_req->addIe[0],
			(send_req->length - sizeof(tSirTdlsSendMgmtReq)));
		result_code = eSIR_SME_SUCCESS;
		break;
	case SIR_MAC_TDLS_PEER_TRAFFIC_IND:
		break;
	case SIR_MAC_TDLS_CH_SWITCH_REQ:
		break;
	case SIR_MAC_TDLS_CH_SWITCH_RSP:
		break;
	case SIR_MAC_TDLS_PEER_TRAFFIC_RSP:
		break;
	default:
		break;
	}

lim_tdls_send_mgmt_error:
#ifdef CONVERGED_TDLS_ENABLE
	lim_send_tdls_comp_mgmt_rsp(mac_ctx, eWNI_SME_TDLS_SEND_MGMT_RSP,
		 result_code, send_req->sessionId,
		 send_req->transactionId);
#else
	lim_send_sme_rsp(mac_ctx, eWNI_SME_TDLS_SEND_MGMT_RSP,
		 result_code, send_req->sessionId,
		 send_req->transactionId);
#endif

	return eSIR_SUCCESS;
}

/*
 * Send Response to Link Establish Request to SME
 */
void lim_send_sme_tdls_link_establish_req_rsp(tpAniSirGlobal pMac,
					      uint8_t sessionId,
					      struct qdf_mac_addr *peermac,
					      tDphHashNode *pStaDs, uint8_t status)
{
	struct scheduler_msg mmhMsg = { 0 };

	tSirTdlsLinkEstablishReqRsp *pTdlsLinkEstablishReqRsp = NULL;

	pTdlsLinkEstablishReqRsp =
		qdf_mem_malloc(sizeof(tSirTdlsLinkEstablishReqRsp));
	if (NULL == pTdlsLinkEstablishReqRsp) {
		pe_err("Failed to allocate memory");
		return;
	}
	pe_debug("Send Resp to TDL Link Establish Req to SME");
	pTdlsLinkEstablishReqRsp->statusCode = status;
	if (peermac)
		qdf_copy_macaddr(&pTdlsLinkEstablishReqRsp->peermac, peermac);

	pTdlsLinkEstablishReqRsp->sessionId = sessionId;
	mmhMsg.type = eWNI_SME_TDLS_LINK_ESTABLISH_RSP;
	mmhMsg.bodyptr = pTdlsLinkEstablishReqRsp;
	mmhMsg.bodyval = 0;
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
	return;

}

/*
 * Once link is teardown, send Del Peer Ind to SME
 */
static QDF_STATUS lim_send_sme_tdls_del_sta_rsp(tpAniSirGlobal pMac,
						uint8_t sessionId,
						struct qdf_mac_addr peerMac,
						tDphHashNode *pStaDs, uint8_t status)
{
	struct scheduler_msg mmhMsg = { 0 };
	tSirTdlsDelStaRsp *pDelSta = NULL;
	mmhMsg.type = eWNI_SME_TDLS_DEL_STA_RSP;

	pDelSta = qdf_mem_malloc(sizeof(tSirTdlsDelStaRsp));
	if (NULL == pDelSta) {
		pe_err("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}

	pDelSta->sessionId = sessionId;
	pDelSta->statusCode = status;
	if (pStaDs) {
		pDelSta->staId = pStaDs->staIndex;
	} else
		pDelSta->staId = STA_INVALID_IDX;

	qdf_copy_macaddr(&pDelSta->peermac, &peerMac);

	pDelSta->length = sizeof(tSirTdlsDelStaRsp);
	pDelSta->messageType = eWNI_SME_TDLS_DEL_STA_RSP;
#ifdef CONVERGED_TDLS_ENABLE
	pDelSta->psoc = pMac->psoc;
	mmhMsg.bodyptr = pDelSta;
	mmhMsg.callback = tgt_tdls_del_peer_rsp;
	return scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &mmhMsg);
#else
	mmhMsg.bodyptr = pDelSta;

	mmhMsg.bodyval = 0;
#endif
	lim_sys_process_mmh_msg_api(pMac, &mmhMsg, ePROT);
	return QDF_STATUS_SUCCESS;

}

/*
 * Process Send Mgmt Request from SME and transmit to AP.
 */
tSirRetStatus lim_process_sme_tdls_add_sta_req(tpAniSirGlobal pMac,
					       uint32_t *pMsgBuf)
{
	/* get all discovery request parameters */
	tSirTdlsAddStaReq *pAddStaReq = (tSirTdlsAddStaReq *) pMsgBuf;
	tpPESession psessionEntry;
	uint8_t sessionId;

	pe_debug("TDLS Add STA Request Recieved");
	psessionEntry =
		pe_find_session_by_bssid(pMac, pAddStaReq->bssid.bytes,
					 &sessionId);
	if (psessionEntry == NULL) {
		pe_err("PE Session does not exist for given sme sessionId: %d",
			pAddStaReq->sessionId);
		goto lim_tdls_add_sta_error;
	}

	/* check if we are in proper state to work as TDLS client */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		pe_err("send mgmt received in wrong system Role: %d",
			  GET_LIM_SYSTEM_ROLE(psessionEntry));
		goto lim_tdls_add_sta_error;
	}

	if (lim_is_roam_synch_in_progress(psessionEntry)) {
		pe_err("roaming in progress, reject add sta! for session %d",
		       pAddStaReq->sessionId);
		goto lim_tdls_add_sta_error;
	}

	/*
	 * if we are still good, go ahead and check if we are in proper state to
	 * do TDLS discovery req/rsp/....frames.
	 */
	if ((psessionEntry->limSmeState != eLIM_SME_ASSOCIATED_STATE) &&
	    (psessionEntry->limSmeState != eLIM_SME_LINK_EST_STATE)) {
		pe_err("send mgmt received in invalid LIMsme state: %d",
			psessionEntry->limSmeState);
		goto lim_tdls_add_sta_error;
	}

	pMac->lim.gLimAddStaTdls = true;

	/* To start with, send add STA request to HAL */
	if (eSIR_FAILURE == lim_tdls_setup_add_sta(pMac, pAddStaReq, psessionEntry)) {
		pe_err("Add TDLS Station request failed");
		goto lim_tdls_add_sta_error;
	}
	return eSIR_SUCCESS;
lim_tdls_add_sta_error:
	lim_send_sme_tdls_add_sta_rsp(pMac,
				      pAddStaReq->sessionId,
				      pAddStaReq->peermac.bytes,
				      (pAddStaReq->tdlsAddOper == TDLS_OPER_UPDATE),
				      NULL, eSIR_FAILURE);

	return eSIR_SUCCESS;
}

/*
 * Process Del Sta Request from SME .
 */
tSirRetStatus lim_process_sme_tdls_del_sta_req(tpAniSirGlobal pMac,
					       uint32_t *pMsgBuf)
{
	/* get all discovery request parameters */
	tSirTdlsDelStaReq *pDelStaReq = (tSirTdlsDelStaReq *) pMsgBuf;
	tpPESession psessionEntry;
	uint8_t sessionId;

	pe_debug("TDLS Delete STA Request Recieved");
	psessionEntry =
		pe_find_session_by_bssid(pMac, pDelStaReq->bssid.bytes,
					 &sessionId);
	if (psessionEntry == NULL) {
		pe_err("PE Session does not exist for given sme sessionId: %d",
			pDelStaReq->sessionId);
		lim_send_sme_tdls_del_sta_rsp(pMac, pDelStaReq->sessionId,
					      pDelStaReq->peermac, NULL,
					      eSIR_FAILURE);
		return eSIR_FAILURE;
	}

	/* check if we are in proper state to work as TDLS client */
	if (!LIM_IS_STA_ROLE(psessionEntry)) {
		pe_err("Del sta received in wrong system Role %d",
			  GET_LIM_SYSTEM_ROLE(psessionEntry));
		goto lim_tdls_del_sta_error;
	}

	if (lim_is_roam_synch_in_progress(psessionEntry)) {
		pe_err("roaming in progress, reject del sta! for session %d",
		       pDelStaReq->sessionId);
		lim_send_sme_tdls_del_sta_rsp(pMac, pDelStaReq->sessionId,
					      pDelStaReq->peermac, NULL,
					      eSIR_FAILURE);
		return eSIR_FAILURE;
	}

	/*
	 * if we are still good, go ahead and check if we are in proper state to
	 * do TDLS discovery req/rsp/....frames.
	 */
	if ((psessionEntry->limSmeState != eLIM_SME_ASSOCIATED_STATE) &&
	    (psessionEntry->limSmeState != eLIM_SME_LINK_EST_STATE)) {

		pe_err("Del Sta received in invalid LIMsme state: (%d",
			psessionEntry->limSmeState);
		goto lim_tdls_del_sta_error;
	}

	lim_tdls_del_sta(pMac, pDelStaReq->peermac, psessionEntry, true);
	return eSIR_SUCCESS;

lim_tdls_del_sta_error:
	lim_send_sme_tdls_del_sta_rsp(pMac, psessionEntry->smeSessionId,
				      pDelStaReq->peermac, NULL, eSIR_FAILURE);

	return eSIR_SUCCESS;
}

/* Intersects the two input arrays and outputs an array */
/* For now the array length of uint8_t suffices */
static void lim_tdls_get_intersection(uint8_t *input_array1,
				      uint8_t input1_length,
				      uint8_t *input_array2,
				      uint8_t input2_length,
				      uint8_t *output_array,
				      uint8_t *output_length)
{
	uint8_t i, j, k = 0, flag = 0;
	for (i = 0; i < input1_length; i++) {
		flag = 0;
		for (j = 0; j < input2_length; j++) {
			if (input_array1[i] == input_array2[j]) {
				flag = 1;
				break;
			}
		}
		if (flag == 1) {
			output_array[k] = input_array1[i];
			k++;
		}
	}
	*output_length = k;
}

/**
 * lim_process_sme_tdls_link_establish_req() - process tdls link establishment
 *                                            request
 *
 * @mac_ctx - global MAC context
 * @msg_buf - message buffer from SME
 *
 * Process Link Establishment Request from SME
 *
 * Return: eSIR_SUCCESS on success, failure code otherwise.
 */
tSirRetStatus lim_process_sme_tdls_link_establish_req(tpAniSirGlobal mac_ctx,
						     uint32_t *msg_buf)
{
	/* get all discovery request parameters */
	tSirTdlsLinkEstablishReq *tdls_req =
		(tSirTdlsLinkEstablishReq *) msg_buf;
	tpPESession session_entry;
	uint8_t session_id;
	tpTdlsLinkEstablishParams tdls_req_params;
	struct scheduler_msg msg = {0};
	uint16_t peer_idx = 0;
	tpDphHashNode stads = NULL;
	uint32_t self_num_chan = WNI_CFG_VALID_CHANNEL_LIST_LEN;
	uint8_t self_supp_chan[WNI_CFG_VALID_CHANNEL_LIST_LEN];

	pe_debug("Process TDLS Link Establishment Request from SME");

	session_entry = pe_find_session_by_bssid(mac_ctx, tdls_req->bssid.bytes,
						 &session_id);
	if (NULL == session_entry) {
		pe_err("PE Session does not exist for sme session_id: %d",
			  tdls_req->sessionId);
		lim_send_sme_tdls_link_establish_req_rsp(mac_ctx,
			tdls_req->sessionId, &tdls_req->peermac, NULL,
			eSIR_FAILURE);
		return eSIR_FAILURE;
	}

	/* check if we are in proper state to work as TDLS client */
	if (!LIM_IS_STA_ROLE(session_entry)) {
		pe_err("TDLS Link Establish Request received in wrong system Role: %d",
			  GET_LIM_SYSTEM_ROLE(session_entry));
		goto lim_tdls_link_establish_error;
	}

	/*
	 * if we are still good, go ahead and check if we are in proper state to
	 * do TDLS discovery req/rsp/....frames.
	 */
	if ((session_entry->limSmeState != eLIM_SME_ASSOCIATED_STATE) &&
	    (session_entry->limSmeState != eLIM_SME_LINK_EST_STATE)) {
		pe_err("TDLS Link Establish Request received in invalid LIMsme state: %d",
			session_entry->limSmeState);
		goto lim_tdls_link_establish_error;
	}

	stads = dph_lookup_hash_entry(mac_ctx, tdls_req->peermac.bytes,
				      &peer_idx,
				      &session_entry->dph.dphHashTable);
	if (NULL == stads) {
		pe_err("stads is NULL");
		goto lim_tdls_link_establish_error;
	}
	tdls_req_params = qdf_mem_malloc(sizeof(tTdlsLinkEstablishParams));
	if (NULL == tdls_req_params) {
		pe_err("Unable to allocate memory TDLS Link Establish Request");
		return eSIR_MEM_ALLOC_FAILED;
	}

	tdls_req_params->staIdx = stads->staIndex;
	tdls_req_params->isResponder = tdls_req->isResponder;
	tdls_req_params->uapsdQueues = tdls_req->uapsdQueues;
	tdls_req_params->maxSp = tdls_req->maxSp;
	tdls_req_params->isBufsta = tdls_req->isBufSta;
	tdls_req_params->isOffChannelSupported =
		tdls_req->isOffChannelSupported;

	if (0 == tdls_req->supportedChannelsLen)
		goto send_tdls_establish_request;

	if (wlan_cfg_get_str(mac_ctx, WNI_CFG_VALID_CHANNEL_LIST,
			     self_supp_chan,
			     &self_num_chan) != eSIR_SUCCESS) {
		/**
		 * Could not get Valid channel list from CFG.
		 * Log error.
		 */
		pe_warn("could not retrieve Valid channel list");
	}

	if (self_num_chan > WNI_CFG_VALID_CHANNEL_LIST_LEN) {
		pe_warn("Channel List more than Valid Channel list");
		self_num_chan = WNI_CFG_VALID_CHANNEL_LIST_LEN;
	}

	if (tdls_req->supportedChannelsLen > SIR_MAC_MAX_SUPP_CHANNELS) {
		pe_warn("Channel List is more than the supported Channel list");
		tdls_req->supportedChannelsLen = SIR_MAC_MAX_SUPP_CHANNELS;
	}

	lim_tdls_get_intersection(self_supp_chan, self_num_chan,
		tdls_req->supportedChannels, tdls_req->supportedChannelsLen,
		tdls_req_params->validChannels,
		&tdls_req_params->validChannelsLen);

send_tdls_establish_request:
	qdf_mem_copy(tdls_req_params->validOperClasses,
		     tdls_req->supportedOperClasses,
		     tdls_req->supportedOperClassesLen);
	tdls_req_params->validOperClassesLen =
			tdls_req->supportedOperClassesLen;

	msg.type = WMA_SET_TDLS_LINK_ESTABLISH_REQ;
	msg.reserved = 0;
	msg.bodyptr = tdls_req_params;
	msg.bodyval = 0;
	if (eSIR_SUCCESS != wma_post_ctrl_msg(mac_ctx, &msg)) {
		pe_err("halPostMsgApi failed");
		goto lim_tdls_link_establish_error;
	}
	return eSIR_SUCCESS;

lim_tdls_link_establish_error:
	lim_send_sme_tdls_link_establish_req_rsp(mac_ctx,
		session_entry->smeSessionId, &tdls_req->peermac, NULL,
		eSIR_FAILURE);

	return eSIR_SUCCESS;
}

/**
 * lim_check_aid_and_delete_peer() - Funtion to check aid and delete peer
 * @p_mac: pointer to mac context
 * @session_entry: pointer to PE session
 *
 * This function verifies aid and delete's peer with that aid from hash table
 *
 * Return: None
 */
static void lim_check_aid_and_delete_peer(tpAniSirGlobal p_mac,
					  tpPESession session_entry)
{
	tpDphHashNode stads = NULL;
	int i, aid;
	size_t aid_bitmap_size = sizeof(session_entry->peerAIDBitmap);
	struct qdf_mac_addr mac_addr;

	/*
	 * Check all the set bit in peerAIDBitmap and delete the peer
	 * (with that aid) entry from the hash table and add the aid
	 * in free pool
	 */
	pe_debug("Delete all the TDLS peer connected");
	for (i = 0; i < aid_bitmap_size / sizeof(uint32_t); i++) {
		for (aid = 0; aid < (sizeof(uint32_t) << 3); aid++) {
			if (!CHECK_BIT(session_entry->peerAIDBitmap[i], aid))
				continue;
			stads = dph_get_hash_entry(p_mac,
					(aid + i * (sizeof(uint32_t) << 3)),
					&session_entry->dph.dphHashTable);

			if (NULL == stads)
				goto skip;

			pe_debug("Deleting "MAC_ADDRESS_STR,
				MAC_ADDR_ARRAY(stads->staAddr));

			if (!lim_is_roam_synch_in_progress(session_entry)) {
				lim_send_deauth_mgmt_frame(p_mac,
					eSIR_MAC_DEAUTH_LEAVING_BSS_REASON,
					stads->staAddr, session_entry, false);

				/* Delete TDLS peer */
				qdf_mem_copy(mac_addr.bytes, stads->staAddr,
					     QDF_MAC_ADDR_SIZE);

				lim_tdls_del_sta(p_mac, mac_addr,
						 session_entry, false);
			}

			dph_delete_hash_entry(p_mac,
				stads->staAddr, stads->assocId,
				&session_entry->dph.dphHashTable);
skip:
			lim_release_peer_idx(p_mac,
				(aid + i * (sizeof(uint32_t) << 3)),
				session_entry);
			CLEAR_BIT(session_entry->peerAIDBitmap[i], aid);
		}
	}
}

#ifndef CONVERGED_TDLS_ENABLE
/**
 * lim_delete_tdls_peers() - delete tdls peers
 *
 * @mac_ctx - global MAC context
 * @session_entry - PE session entry
 *
 * Delete all the TDLS peer connected before leaving the BSS
 *
 * Return: eSIR_SUCCESS on success, error code otherwise
 */
tSirRetStatus lim_delete_tdls_peers(tpAniSirGlobal mac_ctx,
				    tpPESession session_entry)
{
	cds_msg_t msg;
	struct sir_tdls_notify_set_state_disable *tdls_state_disable;

	pe_debug("Enter");

	if (NULL == session_entry) {
		pe_err("NULL session_entry");
		return eSIR_FAILURE;
	}

	lim_check_aid_and_delete_peer(mac_ctx, session_entry);
	if (lim_is_roam_synch_in_progress(session_entry))
		return eSIR_SUCCESS;

	if (mac_ctx->lim.sme_msg_callback) {
		tdls_state_disable = qdf_mem_malloc(
						sizeof(*tdls_state_disable));
		if (NULL == tdls_state_disable) {
			pe_err("memory allocation failed");
			return eSIR_FAILURE;
		}
		tdls_state_disable->session_id = session_entry->smeSessionId;
		msg.type = eWNI_SME_TDLS_NOTIFY_SET_STATE_DISABLE;
		msg.bodyptr = tdls_state_disable;
		msg.bodyval = 0;
		mac_ctx->lim.sme_msg_callback(mac_ctx, &msg);
	}

	lim_send_sme_tdls_delete_all_peer_ind(mac_ctx, session_entry);
	pe_debug("Exit");
	return eSIR_SUCCESS;
}
#else
/**
 * lim_delete_tdls_peers() - delete tdls peers
 *
 * @mac_ctx - global MAC context
 * @session_entry - PE session entry
 *
 * Delete all the TDLS peer connected before leaving the BSS
 *
 * Return: eSIR_SUCCESS on success, error code otherwise
 */
tSirRetStatus lim_delete_tdls_peers(tpAniSirGlobal mac_ctx,
				    tpPESession session_entry)
{
	pe_debug("Enter");

	if (NULL == session_entry) {
		pe_err("NULL session_entry");
		return eSIR_FAILURE;
	}

	lim_check_aid_and_delete_peer(mac_ctx, session_entry);

	if (lim_is_roam_synch_in_progress(session_entry))
		return eSIR_SUCCESS;

	tgt_tdls_peers_deleted_notification(mac_ctx->psoc,
					    session_entry->smeSessionId);

	pe_debug("Exit");
	return eSIR_SUCCESS;
}
#endif

/**
 * lim_process_sme_del_all_tdls_peers(): process delete tdls peers
 * @p_mac: pointer to mac context
 * @msg_buf: message buffer
 *
 * This function processes request to delete tdls peers
 *
 * Return: Success: eSIR_SUCCESS Failure: Error value
 */
tSirRetStatus lim_process_sme_del_all_tdls_peers(tpAniSirGlobal p_mac,
						 uint32_t *msg_buf)
{
	struct sir_del_all_tdls_peers *msg;
	tpPESession session_entry;
	uint8_t session_id;

	msg = (struct sir_del_all_tdls_peers *)msg_buf;
	if (msg == NULL) {
		pe_err("NULL msg");
		return eSIR_FAILURE;
	}

	session_entry = pe_find_session_by_bssid(p_mac,
						 msg->bssid.bytes, &session_id);
	if (NULL == session_entry) {
		pe_err("NULL psessionEntry");
		return eSIR_FAILURE;
	}

	lim_check_aid_and_delete_peer(p_mac, session_entry);

	return eSIR_SUCCESS;
}

/**
 * lim_process_tdls_del_sta_rsp() - Handle WDA_DELETE_STA_RSP for TDLS
 * @mac_ctx: Global MAC context
 * @lim_msg: LIM message
 * @pe_session: PE session
 *
 * Return: None
 */
void lim_process_tdls_del_sta_rsp(tpAniSirGlobal mac_ctx,
				  struct scheduler_msg *lim_msg,
				  tpPESession session_entry)
{
	tpDeleteStaParams del_sta_params = (tpDeleteStaParams) lim_msg->bodyptr;
	tpDphHashNode sta_ds;
	uint16_t peer_idx = 0;
	struct qdf_mac_addr peer_mac;

	if (!del_sta_params) {
		pe_err("del_sta_params is NULL");
		return;
	}

	sta_ds = dph_lookup_hash_entry(mac_ctx, del_sta_params->staMac,
			&peer_idx, &session_entry->dph.dphHashTable);
	if (!sta_ds) {
		pe_err("DPH Entry for STA: %X is missing",
			DPH_STA_HASH_INDEX_PEER);
		goto skip_event;
	}

	qdf_mem_copy(peer_mac.bytes,
			del_sta_params->staMac, QDF_MAC_ADDR_SIZE);

	if (QDF_STATUS_SUCCESS != del_sta_params->status) {
		pe_err("DEL STA failed!");
		lim_send_sme_tdls_del_sta_rsp(mac_ctx,
				      session_entry->smeSessionId,
				      peer_mac, NULL, eSIR_FAILURE);
		goto skip_event;
	}

	pe_debug("DEL STA success");

	/* now send indication to SME-->HDD->TL to remove STA from TL */

	lim_send_sme_tdls_del_sta_rsp(mac_ctx, session_entry->smeSessionId,
				      peer_mac, sta_ds,
				      eSIR_SUCCESS);
	lim_release_peer_idx(mac_ctx, sta_ds->assocId, session_entry);

	/* Clear the aid in peerAIDBitmap as this aid is now in freepool */
	CLEAR_PEER_AID_BITMAP(session_entry->peerAIDBitmap,
			      sta_ds->assocId);
	lim_delete_dph_hash_entry(mac_ctx, sta_ds->staAddr, sta_ds->assocId,
				  session_entry);

skip_event:
	qdf_mem_free(del_sta_params);
	lim_msg->bodyptr = NULL;
}


#endif
