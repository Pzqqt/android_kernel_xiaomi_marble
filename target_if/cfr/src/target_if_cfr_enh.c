/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
#include <hif_hw_version.h>
#include <target_if.h>
#include <wlan_lmac_if_def.h>
#include <wlan_osif_priv.h>
#include <init_deinit_lmac.h>
#include <wlan_cfr_utils_api.h>
#ifdef DIRECT_BUF_RX_ENABLE
#include <target_if_direct_buf_rx_api.h>
#endif
#include <target_if_cfr_enh.h>
#include "cdp_txrx_ctrl.h"

#define CMN_NOISE_FLOOR       (-96)
#define NUM_CHAINS_FW_TO_HOST(n) ((1 << ((n) + 1)) - 1)

#define CFR_INVALID_SNR 0x80

static u_int32_t end_magic = 0xBEAFDEAD;

/**
 * snr_to_signal_strength() - Convert SNR(dB) to signal strength(dBm)
 * @snr: SNR in dB
 *
 * Return: signal strength in dBm
 */
static inline
u_int32_t snr_to_signal_strength(uint8_t snr)
{
	/* SNR value 0x80 indicates -128dB which is not a valid value */
	return (snr != CFR_INVALID_SNR) ?
		(((int8_t)snr) + CMN_NOISE_FLOOR) :
		((int8_t)snr);
}

/**
 * get_lut_entry() - Retrieve LUT entry using cookie number
 * @pcfr: PDEV CFR object
 * @offset: cookie number
 *
 * Return: look up table entry
 */
static struct look_up_table *get_lut_entry(struct pdev_cfr *pcfr,
					   int offset)
{
	if (offset >= pcfr->lut_num) {
		cfr_err("Invalid offset %d, lut_num %d",
			offset, pcfr->lut_num);
		return NULL;
	}

	return pcfr->lut[offset];
}

/**
 * release_lut_entry_enh() - Clear all params in an LUT entry
 * @pdev: objmgr PDEV
 * @lut: pointer to LUT
 *
 * Return: status
 */
static int release_lut_entry_enh(struct wlan_objmgr_pdev *pdev,
				 struct look_up_table *lut)
{
	lut->dbr_recv = false;
	lut->tx_recv = false;
	lut->data = NULL;
	lut->data_len = 0;
	lut->dbr_ppdu_id = 0;
	lut->tx_ppdu_id = 0;
	lut->dbr_tstamp = 0;
	lut->txrx_tstamp = 0;
	lut->tx_address1 = 0;
	lut->tx_address2 = 0;
	lut->dbr_address = 0;
	qdf_mem_zero(&lut->header, sizeof(struct csi_cfr_header));

	return 0;
}

/**
 * target_if_cfr_dump_lut_enh() - dump all valid lut entries
 * @pdev: objmgr pdev
 *
 * return: none
 */
void target_if_cfr_dump_lut_enh(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pcfr;
	struct look_up_table *lut = NULL;
	int i = 0;
	uint64_t diff;
	QDF_STATUS retval = 0;

	retval = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_CFR_ID);
	if (retval != QDF_STATUS_SUCCESS) {
		cfr_err("failed to get pdev reference");
		return;
	}

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						     WLAN_UMAC_COMP_CFR);
	if (!pcfr) {
		cfr_err("pdev object for CFR is null");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);
		return;
	}

	qdf_spin_lock_bh(&pcfr->lut_lock);

	for (i = 0; i < pcfr->lut_num; i++) {
		lut = get_lut_entry(pcfr, i);
		if (!lut)
			continue;
		if (lut->dbr_recv ^ lut->tx_recv) {
			diff = (lut->dbr_tstamp > lut->txrx_tstamp) ?
				(lut->dbr_tstamp - lut->txrx_tstamp) :
				(lut->txrx_tstamp - lut->dbr_tstamp);
		}
	}

	qdf_spin_unlock_bh(&pcfr->lut_lock);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);
}

/**
 * cfr_free_pending_dbr_events() - Flush all pending DBR events. This is useful
 * in cases where for RXTLV drops in host monitor status ring is huge.
 * @pdev: objmgr pdev
 *
 * return: none
 */
static void cfr_free_pending_dbr_events(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pcfr;
	struct look_up_table *lut = NULL;
	int i = 0;
	QDF_STATUS retval = 0;

	retval = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_CFR_ID);
	if (retval != QDF_STATUS_SUCCESS) {
		cfr_err("failed to get pdev reference");
		return;
	}

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						     WLAN_UMAC_COMP_CFR);
	if (!pcfr) {
		cfr_err("pdev object for CFR is null");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);
		return;
	}

	for (i = 0; i < pcfr->lut_num; i++) {
		lut = get_lut_entry(pcfr, i);
		if (!lut)
			continue;

		if (lut->dbr_recv && !lut->tx_recv &&
		    (lut->dbr_tstamp < pcfr->last_success_tstamp)) {
			target_if_dbr_buf_release(pdev, DBR_MODULE_CFR,
						  lut->dbr_address,
						  i, 0);
			pcfr->flush_dbr_cnt++;
			release_lut_entry_enh(pdev, lut);
		}
	}
	wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);
}

/**
 * dump_freeze_tlv() - Dump freeze TLV sent in enhanced DMA header
 * @freeze_tlv: Freeze TLV sent from MAC to PHY
 * @cookie: Index into lookup table
 *
 * Return: none
 */
static void dump_freeze_tlv(void *freeze_tlv, uint32_t cookie)
{
	struct macrx_freeze_capture_channel *freeze =
		(struct macrx_freeze_capture_channel *)freeze_tlv;

	cfr_debug("<DBRCOMP><FREEZE><%u>\n"
		  "freeze: %d capture_reason: %d packet_type: 0x%x\n"
		  "packet_subtype: 0x%x sw_peer_id_valid: %d sw_peer_id: %d\n"
		  "phy_ppdu_id: 0x%04x packet_ta_upper_16: 0x%04x\n"
		  "packet_ta_mid_16: 0x%04x packet_ta_lower_16: 0x%04x\n"
		  "packet_ra_upper_16: 0x%04x packet_ra_mid_16: 0x%04x\n"
		  "packet_ra_lower_16: 0x%04x tsf_timestamp_63_48: 0x%04x\n"
		  "tsf_timestamp_47_32: 0x%04x tsf_timestamp_31_16: 0x%04x\n"
		  "tsf_timestamp_15_0: 0x%04x user_index_or_user_mask_5_0: %d\n"
		  "directed: %d\n",
		  cookie,
		  freeze->freeze,
		  freeze->capture_reason,
		  freeze->packet_type,
		  freeze->packet_sub_type,
		  freeze->sw_peer_id_valid,
		  freeze->sw_peer_id,
		  freeze->phy_ppdu_id,
		  freeze->packet_ta_upper_16,
		  freeze->packet_ta_mid_16,
		  freeze->packet_ta_lower_16,
		  freeze->packet_ra_upper_16,
		  freeze->packet_ra_mid_16,
		  freeze->packet_ra_lower_16,
		  freeze->tsf_timestamp_63_48,
		  freeze->tsf_timestamp_47_32,
		  freeze->tsf_timestamp_31_16,
		  freeze->tsf_timestamp_15_0,
		  freeze->user_index_or_user_mask_5_0,
		  freeze->directed);
}

/**
 * dump_freeze_tlv_v3() - Dump freeze TLV v2 sent in enhanced DMA header
 * @freeze_tlv: Freeze TLV sent from MAC to PHY
 * @cookie: Index into lookup table
 *
 * Return: none
 */
static void dump_freeze_tlv_v3(void *freeze_tlv, uint32_t cookie)
{
	struct macrx_freeze_capture_channel_v3 *freeze =
		(struct macrx_freeze_capture_channel_v3 *)freeze_tlv;

	cfr_debug("<DBRCOMP><FREEZE><%u>\n"
		  "freeze: %d capture_reason: %d packet_type: 0x%x\n"
		  "packet_subtype: 0x%x sw_peer_id_valid: %d sw_peer_id: %d\n"
		  "phy_ppdu_id: 0x%04x packet_ta_upper_16: 0x%04x\n"
		  "packet_ta_mid_16: 0x%04x packet_ta_lower_16: 0x%04x\n"
		  "packet_ra_upper_16: 0x%04x packet_ra_mid_16: 0x%04x\n"
		  "packet_ra_lower_16: 0x%04x\n"
		  "tsf_63_48_or_user_mask_36_32: 0x%04x\n"
		  "tsf_timestamp_47_32: 0x%04x\n"
		  "tsf_timestamp_31_16: 0x%04x\n"
		  "tsf_timestamp_15_0: 0x%04x\n"
		  "user_index_or_user_mask_15_0: 0x%04x\n"
		  "user_mask_31_16: 0x%04x\n"
		  "directed: %d\n",
		  cookie,
		  freeze->freeze,
		  freeze->capture_reason,
		  freeze->packet_type,
		  freeze->packet_sub_type,
		  freeze->sw_peer_id_valid,
		  freeze->sw_peer_id,
		  freeze->phy_ppdu_id,
		  freeze->packet_ta_upper_16,
		  freeze->packet_ta_mid_16,
		  freeze->packet_ta_lower_16,
		  freeze->packet_ra_upper_16,
		  freeze->packet_ra_mid_16,
		  freeze->packet_ra_lower_16,
		  freeze->tsf_63_48_or_user_mask_36_32,
		  freeze->tsf_timestamp_47_32,
		  freeze->tsf_timestamp_31_16,
		  freeze->tsf_timestamp_15_0,
		  freeze->user_index_or_user_mask_15_0,
		  freeze->user_mask_31_16,
		  freeze->directed);
}

/**
 * dump_mu_rx_info() - Dump MU info in enhanced DMA header
 * @mu_rx_user_info: MU info sent by ucode
 * @mu_rx_num_users: Number of MU users in UL-MU-PPDU
 * @cookie: Index into lookup table
 *
 * Return: none
 */
static void dump_mu_rx_info(void *mu_rx_user_info,
			    uint8_t mu_rx_num_users,
			    uint32_t cookie)
{
	uint8_t i;
	struct uplink_user_setup_info *ul_mu_user_info =
		(struct uplink_user_setup_info *)mu_rx_user_info;

	for (i = 0 ; i < mu_rx_num_users; i++) {
		cfr_debug("<DBRCOMP><MU><%u>\n"
			  "<user_id:%d>\n"
			  "bw_info_valid = %d\n"
			  "uplink_receive_type = %d\n"
			  "uplink_11ax_mcs = %d\n"
			  "ru_width = %d\n"
			  "nss = %d\n"
			  "stream_offset = %d\n"
			  "sta_dcm = %d\n"
			  "sta_coding = %d\n"
			  "ru_start_index = %d\n",
			  cookie,
			  i,
			  ul_mu_user_info->bw_info_valid,
			  ul_mu_user_info->uplink_receive_type,
			  ul_mu_user_info->uplink_11ax_mcs,
			  ul_mu_user_info->ru_width,
			  ul_mu_user_info->nss,
			  ul_mu_user_info->stream_offset,
			  ul_mu_user_info->sta_dcm,
			  ul_mu_user_info->sta_coding,
			  ul_mu_user_info->ru_start_index);
		ul_mu_user_info += sizeof(struct uplink_user_setup_info);
	}
}

static void dump_metadata(struct csi_cfr_header *header, uint32_t cookie)
{
	uint8_t user_id, chain_id;
	struct cfr_metadata_version_3 *meta = &header->u.meta_v3;
	uint8_t *usermac = NULL;

	cfr_debug("<METADATA><%u>\n"
		  "start_magic_num = 0x%x\n"
		  "vendorid = 0x%x\n"
		  "cfr_metadata_version = %d\n"
		  "cfr_data_version = %d\n"
		  "chip_type = %d\n"
		  "platform_type = %d\n"
		  "status = %d\n"
		  "capture_bw = %d\n"
		  "channel_bw = %d\n"
		  "phy_mode = %d\n"
		  "prim20_chan = %d\n"
		  "center_freq1 = %d\n"
		  "center_freq2 = %d\n"
		  "ack_capture_mode = %d\n"
		  "cfr_capture_type = %d\n"
		  "sts_count = %d\n"
		  "num_rx_chain = %d\n"
		  "timestamp = %llu\n"
		  "length = %d\n"
		  "is_mu_ppdu = %d\n"
		  "num_users = %d\n",
		cookie,
		header->start_magic_num,
		header->vendorid,
		header->cfr_metadata_version,
		header->cfr_data_version,
		header->chip_type,
		header->pltform_type,
		meta->status,
		meta->capture_bw,
		meta->channel_bw,
		meta->phy_mode,
		meta->prim20_chan,
		meta->center_freq1,
		meta->center_freq2,
		meta->capture_mode,
		meta->capture_type,
		meta->sts_count,
		meta->num_rx_chain,
		meta->timestamp,
		meta->length,
		meta->is_mu_ppdu,
		meta->num_mu_users);

	if (meta->is_mu_ppdu) {
		for (user_id = 0; user_id < meta->num_mu_users; user_id++) {
			usermac = meta->peer_addr.mu_peer_addr[user_id];
			cfr_debug("peermac[%d]: " QDF_MAC_ADDR_FMT,
				  user_id, QDF_MAC_ADDR_REF(usermac));
		}
	} else {
		cfr_debug("peermac: " QDF_MAC_ADDR_FMT,
			  QDF_MAC_ADDR_REF(meta->peer_addr.su_peer_addr));
	}

	for (chain_id = 0; chain_id < HOST_MAX_CHAINS; chain_id++) {
		cfr_debug("chain_rssi[%d] = %d\n",
			  chain_id,
			  meta->chain_rssi[chain_id]);
	}

	for (chain_id = 0; chain_id < HOST_MAX_CHAINS; chain_id++) {
		cfr_debug("chain_phase[%d] = %d\n",
			  chain_id,
			  meta->chain_phase[chain_id]);
	}
}

/**
 * dump_enh_dma_hdr() - Dump enhanced DMA header populated by ucode
 * @dma_hdr: pointer to enhanced DMA header
 * @freeze_tlv: pointer to MACRX_FREEZE_CAPTURE_CHANNEL TLV
 * @mu_rx_user_info: UPLINK_USER_SETUP_INFO TLV
 * @header: pointer to metadata passed to userspace
 * @error: Indicates whether it is an error
 * @cookie: Index into lookup table
 *
 * Return: none
 */
static void dump_enh_dma_hdr(struct whal_cfir_enhanced_hdr *dma_hdr,
			     void *freeze_tlv, void *mu_rx_user_info,
			     struct csi_cfr_header *header, int error,
			     uint32_t cookie)
{
	if (!error) {
		cfr_debug("<DBRCOMP><%u>\n"
			  "Tag: 0x%02x Length: %d udone: %d\n"
			  "ctype: %d preamble: %d Nss: %d\n"
			  "num_chains: %d bw: %d peervalid: %d\n"
			  "peer_id: %d ppdu_id: 0x%04x total_bytes: %d\n"
			  "header_version: %d target_id: %d cfr_fmt: %d\n"
			  "mu_rx_data_incl: %d freeze_data_incl: %d\n"
			  "mu_rx_num_users: %d decimation_factor: %d\n"
			  "freeze_tlv_version: %d\n",
			  cookie,
			  dma_hdr->tag,
			  dma_hdr->length,
			  dma_hdr->upload_done,
			  dma_hdr->capture_type,
			  dma_hdr->preamble_type,
			  dma_hdr->nss,
			  dma_hdr->num_chains,
			  dma_hdr->upload_pkt_bw,
			  dma_hdr->sw_peer_id_valid,
			  dma_hdr->sw_peer_id,
			  dma_hdr->phy_ppdu_id,
			  dma_hdr->total_bytes,
			  dma_hdr->header_version,
			  dma_hdr->target_id,
			  dma_hdr->cfr_fmt,
			  dma_hdr->mu_rx_data_incl,
			  dma_hdr->freeze_data_incl,
			  dma_hdr->mu_rx_num_users,
			  dma_hdr->decimation_factor,
			  dma_hdr->freeze_tlv_version);

		if (dma_hdr->freeze_data_incl) {
			if (dma_hdr->freeze_tlv_version ==
					MACRX_FREEZE_TLV_VERSION_3)
				dump_freeze_tlv_v3(freeze_tlv, cookie);
			else
				dump_freeze_tlv(freeze_tlv, cookie);
		}

		if (dma_hdr->mu_rx_data_incl)
			dump_mu_rx_info(mu_rx_user_info,
					dma_hdr->mu_rx_num_users,
					cookie);
	} else {
		cfr_err("<DBRCOMP><%u>\n"
			"Tag: 0x%02x Length: %d udone: %d\n"
			"ctype: %d preamble: %d Nss: %d\n"
			"num_chains: %d bw: %d peervalid: %d\n"
			"peer_id: %d ppdu_id: 0x%04x total_bytes: %d\n"
			"header_version: %d target_id: %d cfr_fmt: %d\n"
			"mu_rx_data_incl: %d freeze_data_incl: %d\n"
			"mu_rx_num_users: %d decimation_factor: %d\n"
			"freeze_tlv_version: %d\n",
			cookie,
			dma_hdr->tag,
			dma_hdr->length,
			dma_hdr->upload_done,
			dma_hdr->capture_type,
			dma_hdr->preamble_type,
			dma_hdr->nss,
			dma_hdr->num_chains,
			dma_hdr->upload_pkt_bw,
			dma_hdr->sw_peer_id_valid,
			dma_hdr->sw_peer_id,
			dma_hdr->phy_ppdu_id,
			dma_hdr->total_bytes,
			dma_hdr->header_version,
			dma_hdr->target_id,
			dma_hdr->cfr_fmt,
			dma_hdr->mu_rx_data_incl,
			dma_hdr->freeze_data_incl,
			dma_hdr->mu_rx_num_users,
			dma_hdr->decimation_factor,
			dma_hdr->freeze_tlv_version);
	}
}

/**
 * extract_peer_mac_from_freeze_tlv() - extract macaddr from freeze tlv
 * @freeze_tlv: Freeze TLV sent from MAC to PHY
 * @peermac: macaddr of the peer
 *
 * Return: none
 */
static void
extract_peer_mac_from_freeze_tlv(void *freeze_tlv, uint8_t *peermac)
{
	/*
	 * Packet_ta fields position is common between freeze tlv v1
	 * and v2, hence typecasting to v1 is also fine
	 */
	struct macrx_freeze_capture_channel *freeze =
		(struct macrx_freeze_capture_channel *)freeze_tlv;

	peermac[0] = freeze->packet_ta_lower_16 & 0x00FF;
	peermac[1] = (freeze->packet_ta_lower_16 & 0xFF00) >> 8;
	peermac[2] = freeze->packet_ta_mid_16 & 0x00FF;
	peermac[3] = (freeze->packet_ta_mid_16 & 0xFF00) >> 8;
	peermac[4] = freeze->packet_ta_upper_16 & 0x00FF;
	peermac[5] = (freeze->packet_ta_upper_16 & 0xFF00) >> 8;
}

/**
 * check_dma_length() - Sanity check DMA header and payload length
 * @dma_hdr: pointer to enhanced DMA header
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS check_dma_length(struct look_up_table *lut,
				   uint32_t target_type)
{
	if (target_type == TARGET_TYPE_QCN9000) {
		if (lut->header_length <= PINE_MAX_HEADER_LENGTH_WORDS &&
		    lut->payload_length <= PINE_MAX_DATA_LENGTH_BYTES) {
			return QDF_STATUS_SUCCESS;
		}
	} else {
		if (lut->header_length <= CYP_MAX_HEADER_LENGTH_WORDS &&
		    lut->payload_length <= CYP_MAX_DATA_LENGTH_BYTES) {
			return QDF_STATUS_SUCCESS;
		}
	}
	return QDF_STATUS_E_FAILURE;
}

/**
 * correlate_and_relay_enh() - Correlate TXRX and DBR events and stream CFR
 * data to userspace
 * @pdev: objmgr PDEV
 * @cookie: Index into lookup table
 * @lut: pointer to lookup table
 * @module_id: ID of the event received
 *  0 - DBR event
 *  1 - TXRX event
 *
 * Return:
 *	- STATUS_ERROR
 *	- STATUS_HOLD
 *	- STATUS_STREAM_AND_RELEASE
 */
static int correlate_and_relay_enh(struct wlan_objmgr_pdev *pdev,
				   uint32_t cookie,
				   struct look_up_table *lut,
				   uint8_t module_id)
{
	struct pdev_cfr *pcfr;
	uint64_t diff;
	int status = STATUS_ERROR;
	struct wlan_objmgr_psoc *psoc;
	uint32_t target_type;

	if (module_id > 1) {
		cfr_err("Received request with invalid mod id. Investigate!!");
		QDF_ASSERT(0);
		status = STATUS_ERROR;
		goto done;
	}

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						     WLAN_UMAC_COMP_CFR);

	psoc = wlan_pdev_get_psoc(pdev);
	if (qdf_unlikely(!psoc)) {
		cfr_err("psoc is null\n");
		status = STATUS_ERROR;
		goto done;
	}

	target_type = target_if_cfr_get_target_type(psoc);

	if (module_id == CORRELATE_TX_EV_MODULE_ID) {
		if (lut->tx_recv)
			pcfr->cfr_dma_aborts++;
		lut->tx_recv = true;
	} else if (module_id == CORRELATE_DBR_MODULE_ID) {
		pcfr->dbr_evt_cnt++;
		lut->dbr_recv = true;
	}

	if ((lut->dbr_recv) && (lut->tx_recv)) {
		if (lut->dbr_ppdu_id == lut->tx_ppdu_id) {
			pcfr->last_success_tstamp = lut->dbr_tstamp;
			if (lut->dbr_tstamp > lut->txrx_tstamp) {
				diff = lut->dbr_tstamp - lut->txrx_tstamp;
				cfr_debug("<CORRELATE><%u>: "
					  "TXRX evt -> DBR evt"
					  "(delay = %llu ms)\n", cookie, diff);
			} else if (lut->txrx_tstamp > lut->dbr_tstamp) {
				diff = lut->txrx_tstamp - lut->dbr_tstamp;
				cfr_debug("<CORRELATE><%u>: "
					  "DBR evt -> TXRX evt"
					  "(delay = %llu ms)\n", cookie, diff);
			}

			/*
			 * Flush pending dbr events, if newer PPDU TLV is
			 * received
			 */
			cfr_free_pending_dbr_events(pdev);

			if (check_dma_length(lut, target_type) ==
					QDF_STATUS_SUCCESS) {
				pcfr->release_cnt++;
				cfr_debug("<CORRELATE><%u>:Stream and release "
					  "CFR data for "
					  "ppdu_id:0x%04x\n", cookie,
					  lut->tx_ppdu_id);
				status = STATUS_STREAM_AND_RELEASE;
				goto done;
			} else {
				pcfr->invalid_dma_length_cnt++;
				cfr_err("<CORRELATE><%u>:CFR buffers "
					"received with invalid length "
					"header_length_words = %d "
					"cfr_payload_length_bytes = %d "
					"ppdu_id:0x%04x\n",
					cookie,
					lut->header_length,
					lut->payload_length,
					lut->tx_ppdu_id);
				/*
				 * Assert here as length exceeding the allowed
				 * limit would anyway manifest as random crash
				 */
				QDF_ASSERT(0);
				status = STATUS_ERROR;
				goto done;
			}
		} else {
			/*
			 * When there is a ppdu id mismatch, discard the TXRX
			 * event since multiple PPDUs are likely to have same
			 * dma addr, due to ucode aborts
			 */
			cfr_debug("Received new dbr event for same "
				  "cookie %u",
				  cookie);
			lut->tx_recv = false;
			lut->tx_ppdu_id = 0;
			pcfr->clear_txrx_event++;
			pcfr->cfr_dma_aborts++;
			status = STATUS_HOLD;
		}
	} else {
		status = STATUS_HOLD;
	}
done:
	return status;
}

/**
 * target_if_cfr_rx_tlv_process() - Process PPDU status TLVs and store info in
 * lookup table
 * @pdev_obj: PDEV object
 * @nbuf: ppdu info
 *
 * Return: none
 */
void target_if_cfr_rx_tlv_process(struct wlan_objmgr_pdev *pdev, void *nbuf)
{
	struct cdp_rx_indication_ppdu *cdp_rx_ppdu;
	struct cdp_rx_stats_ppdu_user *rx_stats_peruser;
	struct cdp_rx_ppdu_cfr_info *cfr_info;
	qdf_dma_addr_t buf_addr = 0, buf_addr_extn = 0;
	struct pdev_cfr *pcfr;
	struct look_up_table *lut = NULL;
	struct csi_cfr_header *header = NULL;
	uint32_t cookie;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_channel *bss_chan;
	enum wlan_phymode ch_phymode;
	uint16_t ch_freq;
	uint32_t ch_cfreq1;
	uint32_t ch_cfreq2;
	struct wlan_objmgr_vdev *vdev = NULL;
	int i, status = 0;
	QDF_STATUS retval = 0;
	struct wlan_lmac_if_cfr_rx_ops *cfr_rx_ops = NULL;
	struct cfr_metadata_version_3 *meta = NULL;
	uint8_t srng_id = 0;
	struct wlan_lmac_if_rx_ops *rx_ops;

	if (qdf_unlikely(!pdev)) {
		cfr_err("pdev is null\n");
		qdf_nbuf_free(nbuf);
		return;
	}

	retval = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_CFR_ID);
	if (qdf_unlikely(retval != QDF_STATUS_SUCCESS)) {
		cfr_err("failed to get pdev reference");
		qdf_nbuf_free(nbuf);
		return;
	}

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						     WLAN_UMAC_COMP_CFR);
	if (qdf_unlikely(!pcfr)) {
		cfr_err("pdev object for CFR is NULL");
		goto relref;
	}

	cdp_rx_ppdu = (struct cdp_rx_indication_ppdu *)qdf_nbuf_data(nbuf);
	cfr_info = &cdp_rx_ppdu->cfr_info;

	if (!cfr_info->bb_captured_channel)
		goto relref;

	psoc = wlan_pdev_get_psoc(pdev);
	if (qdf_unlikely(!psoc)) {
		cfr_err("psoc is null\n");
		goto relref;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		cfr_err("rx_ops is NULL");
		goto relref;
	}
	cfr_rx_ops = &rx_ops->cfr_rx_ops;
	buf_addr_extn = cfr_info->rtt_che_buffer_pointer_high8 & 0xF;
	buf_addr = (cfr_info->rtt_che_buffer_pointer_low32 |
		    ((uint64_t)buf_addr_extn << 32));

	srng_id = pcfr->rcc_param.srng_id;
	if (target_if_dbr_cookie_lookup(pdev, DBR_MODULE_CFR, buf_addr,
					&cookie, srng_id)) {
		cfr_debug("Cookie lookup failure for addr: 0x%pK",
			  (void *)((uintptr_t)buf_addr));
		goto relref;
	}

	cfr_debug("<RXTLV><%u>:buffer address: 0x%pK\n"
		  "<WIFIRX_PPDU_START_E> ppdu_id: 0x%04x\n"
		  "<WIFIRXPCU_PPDU_END_INFO_E> BB_CAPTURED_CHANNEL = %d\n"
		  "<WIFIPHYRX_PKT_END_E> RX_LOCATION_INFO_VALID = %d\n"
		  "<WIFIPHYRX_PKT_END_E> RTT_CHE_BUFFER_POINTER_LOW32 = %x\n"
		  "<WIFIPHYRX_PKT_END_E> RTT_CHE_BUFFER_POINTER_HIGH8 = %x\n"
		  "<WIFIPHYRX_PKT_END_E> CHAN_CAPTURE_STATUS = %d\n",
		  cookie,
		  (void *)((uintptr_t)buf_addr),
		  cdp_rx_ppdu->ppdu_id,
		  cfr_info->bb_captured_channel,
		  cfr_info->rx_location_info_valid,
		  cfr_info->rtt_che_buffer_pointer_low32,
		  cfr_info->rtt_che_buffer_pointer_high8,
		  cfr_info->chan_capture_status);

	qdf_spin_lock_bh(&pcfr->lut_lock);

	lut = get_lut_entry(pcfr, cookie);
	if (qdf_unlikely(!lut)) {
		cfr_err("lut is NULL");
		goto unlock;
	}

	vdev = wlan_objmgr_pdev_get_first_vdev(pdev, WLAN_CFR_ID);
	if (qdf_unlikely(!vdev)) {
		cfr_debug("vdev is null\n");
		goto unlock;
	}

	bss_chan = wlan_vdev_mlme_get_bss_chan(vdev);
	ch_freq = bss_chan->ch_freq;
	ch_cfreq1 = bss_chan->ch_cfreq1;
	ch_cfreq2 = bss_chan->ch_cfreq2;
	ch_phymode = bss_chan->ch_phymode;
	wlan_objmgr_vdev_release_ref(vdev, WLAN_CFR_ID);

	pcfr->rx_tlv_evt_cnt++;
	lut->tx_ppdu_id = cdp_rx_ppdu->ppdu_id;
	lut->tx_address1 = cfr_info->rtt_che_buffer_pointer_low32;
	lut->tx_address2 = cfr_info->rtt_che_buffer_pointer_high8;
	lut->txrx_tstamp = qdf_ktime_to_ms(qdf_ktime_get());
	header = &lut->header;
	meta = &header->u.meta_v3;

	header->start_magic_num        = 0xDEADBEAF;
	header->vendorid               = 0x8cfdf0;
	header->cfr_metadata_version   = CFR_META_VERSION_3;
	header->cfr_data_version       = CFR_DATA_VERSION_1;
	header->chip_type              = pcfr->chip_type;
	header->pltform_type           = CFR_PLATFORM_TYPE_ARM;
	header->Reserved               = 0;

	meta->status       = 1;
	meta->phy_mode = ch_phymode;
	meta->prim20_chan  = ch_freq;
	meta->center_freq1 = ch_cfreq1;
	meta->center_freq2 = ch_cfreq2;
	meta->capture_mode = 0;

	meta->timestamp = cdp_rx_ppdu->timestamp;
	meta->is_mu_ppdu = (cdp_rx_ppdu->u.ppdu_type == CDP_RX_TYPE_SU) ? 0 : 1;
	meta->num_mu_users = (meta->is_mu_ppdu) ? (cdp_rx_ppdu->num_users) : 0;

	if (meta->num_mu_users > pcfr->max_mu_users)
		meta->num_mu_users = pcfr->max_mu_users;

	for (i = 0; i < MAX_CHAIN; i++)
		meta->chain_rssi[i] =
			snr_to_signal_strength(cdp_rx_ppdu->per_chain_rssi[i]);

	if (cdp_rx_ppdu->u.ppdu_type != CDP_RX_TYPE_SU) {
		for (i = 0 ; i < meta->num_mu_users; i++) {
			rx_stats_peruser = &cdp_rx_ppdu->user[i];
			qdf_mem_copy(meta->peer_addr.mu_peer_addr[i],
				     rx_stats_peruser->mac_addr,
				     QDF_MAC_ADDR_SIZE);
		}
	}
	status = correlate_and_relay_enh(pdev, cookie, lut,
					 CORRELATE_TX_EV_MODULE_ID);
	if (status == STATUS_STREAM_AND_RELEASE) {
		if (cfr_rx_ops->cfr_info_send)
			status = cfr_rx_ops->cfr_info_send(pdev,
							   &lut->header,
							   sizeof(struct
							   csi_cfr_header),
							   lut->data,
							   lut->data_len,
							   &end_magic, 4);
		dump_metadata(header, cookie);
		release_lut_entry_enh(pdev, lut);
		target_if_dbr_buf_release(pdev, DBR_MODULE_CFR, buf_addr,
					  cookie, srng_id);
	}

unlock:
	qdf_spin_unlock_bh(&pcfr->lut_lock);
relref:
	qdf_nbuf_free(nbuf);
	wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);
}

/**
 * freeze_reason_to_capture_type() - Convert capture type enum in freeze tlv
 * to the cfr type enum shared with userspace
 * @freeze_tlv: pointer to MACRX_FREEZE_CAPTURE_CHANNEL TLV
 *
 * Return: cfr type enum
 */
static uint8_t freeze_reason_to_capture_type(void *freeze_tlv)
{
	/*
	 * Capture_reason field position is common between freeze_tlv v1
	 * and v2, hence typcasting to any one is fine
	 */
	struct macrx_freeze_capture_channel *freeze =
		(struct macrx_freeze_capture_channel *)freeze_tlv;

	switch (freeze->capture_reason) {
	case FREEZE_REASON_TM:
		return CFR_TYPE_METHOD_TM;
	case FREEZE_REASON_FTM:
		return CFR_TYPE_METHOD_FTM;
	case FREEZE_REASON_TA_RA_TYPE_FILTER:
		return CFR_TYPE_METHOD_TA_RA_TYPE_FILTER;
	case FREEZE_REASON_NDPA_NDP:
		return CFR_TYPE_METHOD_NDPA_NDP;
	case FREEZE_REASON_ALL_PACKET:
		return CFR_TYPE_METHOD_ALL_PACKET;
	case FREEZE_REASON_ACK_RESP_TO_TM_FTM:
		return CFR_TYPE_METHOD_ACK_RESP_TO_TM_FTM;
	default:
		return CFR_TYPE_METHOD_AUTO;
	}
	return CFR_TYPE_METHOD_AUTO;
}

#ifdef DIRECT_BUF_RX_ENABLE
/**
 * enh_cfr_dbr_event_handler() - Process DBR event for CFR data DMA completion
 * @pdev: PDEV object
 * @payload: pointer to CFR data
 *
 * Return: status
 */
static bool enh_cfr_dbr_event_handler(struct wlan_objmgr_pdev *pdev,
				      struct direct_buf_rx_data *payload)
{
	uint8_t *data = NULL;
	uint32_t cookie = 0;
	struct whal_cfir_enhanced_hdr dma_hdr = {0};
	int  length, status = 0;
	struct wlan_objmgr_psoc *psoc;
	struct pdev_cfr *pcfr;
	struct look_up_table *lut = NULL;
	struct csi_cfr_header *header = NULL;
	void *mu_rx_user_info = NULL, *freeze_tlv = NULL;
	uint8_t capture_type = CFR_TYPE_METHOD_AUTO;
	uint8_t *peer_macaddr = NULL;
	struct wlan_lmac_if_cfr_rx_ops *cfr_rx_ops = NULL;
	struct cfr_metadata_version_3 *meta = NULL;
	struct wlan_lmac_if_rx_ops *rx_ops;

	if ((!pdev) || (!payload)) {
		cfr_err("pdev or payload is null");
		return true;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		cfr_err("psoc is null");
		return true;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		cfr_err("rx_ops is NULL");
		return true;
	}
	cfr_rx_ops = &rx_ops->cfr_rx_ops;

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						     WLAN_UMAC_COMP_CFR);
	if (!pcfr) {
		cfr_err("pdev object for CFR is null");
		return true;
	}

	data = payload->vaddr;
	cookie = payload->cookie;

	cfr_debug("<DBRCOMP><%u>:bufferaddr: 0x%pK cookie: %u\n", cookie,
		  (void *)((uintptr_t)payload->paddr), cookie);

	qdf_mem_copy(&dma_hdr, &data[0],
		     sizeof(struct whal_cfir_enhanced_hdr));

	if (dma_hdr.freeze_data_incl) {
		freeze_tlv = data + sizeof(struct whal_cfir_enhanced_hdr);
		capture_type = freeze_reason_to_capture_type(freeze_tlv);
	}

	if (dma_hdr.mu_rx_data_incl) {
		uint8_t freeze_tlv_len;

		if (dma_hdr.freeze_tlv_version == MACRX_FREEZE_TLV_VERSION_3) {
			freeze_tlv_len =
				sizeof(struct macrx_freeze_capture_channel_v3);
		} else {
			freeze_tlv_len =
				sizeof(struct macrx_freeze_capture_channel);
		}
		mu_rx_user_info = data +
			sizeof(struct whal_cfir_enhanced_hdr) +
			(dma_hdr.freeze_data_incl ? freeze_tlv_len : 0);
	}

	length  = dma_hdr.length * 4;
	length += dma_hdr.total_bytes; /* size of cfr data */

	qdf_spin_lock_bh(&pcfr->lut_lock);

	lut = get_lut_entry(pcfr, cookie);
	if (!lut) {
		cfr_err("lut is NULL");
		qdf_spin_unlock_bh(&pcfr->lut_lock);
		return true;
	}

	lut->data = data;
	lut->data_len = length;
	lut->dbr_ppdu_id = dma_hdr.phy_ppdu_id;
	lut->dbr_address = payload->paddr;
	lut->dbr_tstamp = qdf_ktime_to_ms(qdf_ktime_get());
	lut->header_length = dma_hdr.length;
	lut->payload_length = dma_hdr.total_bytes;
	qdf_mem_copy(&lut->dma_hdr, &dma_hdr,
		     sizeof(struct whal_cfir_dma_hdr));

	header = &lut->header;
	header->chip_type = pcfr->chip_type;
	meta = &header->u.meta_v3;
	meta->channel_bw = dma_hdr.upload_pkt_bw;
	meta->num_rx_chain = NUM_CHAINS_FW_TO_HOST(dma_hdr.num_chains);
	meta->length = length;
	/* For Tx based captures, capture type is sent from FW */
	if (capture_type != CFR_TYPE_METHOD_ACK_RESP_TO_TM_FTM) {
		meta->capture_type = capture_type;
		meta->sts_count = (dma_hdr.nss + 1);
		if (!dma_hdr.mu_rx_data_incl) {
			/* extract peer addr from freeze tlv */
			peer_macaddr = meta->peer_addr.su_peer_addr;
			if (dma_hdr.freeze_data_incl) {
				extract_peer_mac_from_freeze_tlv(freeze_tlv,
								 peer_macaddr);
			}
		}
	}

	if (dma_hdr.freeze_data_incl) {
		dump_enh_dma_hdr(&dma_hdr, freeze_tlv, mu_rx_user_info,
				 header, 0, cookie);
	}

	status = correlate_and_relay_enh(pdev, cookie, lut,
					 CORRELATE_DBR_MODULE_ID);
	if (status == STATUS_STREAM_AND_RELEASE) {
		/*
		 * Message format
		 *  Meta data Header + actual payload + trailer
		 */
		if (cfr_rx_ops->cfr_info_send)
			status = cfr_rx_ops->cfr_info_send(pdev,
							   &lut->header,
							   sizeof(struct
							   csi_cfr_header),
							   lut->data,
							   lut->data_len,
							   &end_magic, 4);
		dump_metadata(header, cookie);
		release_lut_entry_enh(pdev, lut);
		status = true;
	} else if (status == STATUS_HOLD) {
		status = false;
	} else {
		status = true;
	}

	qdf_spin_unlock_bh(&pcfr->lut_lock);
	return status;
}

/**
 * target_if_register_to_dbr_enh() - Initialize DBR ring and register callback
 * for DBR events
 * @pdev: PDEV object
 *
 * Return: status
 */
static QDF_STATUS
target_if_register_to_dbr_enh(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_direct_buf_rx_tx_ops *dbr_tx_ops = NULL;
	struct dbr_module_config dbr_config;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		cfr_err("tx_ops is NULL");
		return QDF_STATUS_SUCCESS;
	}
	dbr_tx_ops = &tx_ops->dbr_tx_ops;
	dbr_config.num_resp_per_event = DBR_NUM_RESP_PER_EVENT_CFR;
	dbr_config.event_timeout_in_ms = DBR_EVENT_TIMEOUT_IN_MS_CFR;
	if (dbr_tx_ops->direct_buf_rx_module_register) {
		return dbr_tx_ops->direct_buf_rx_module_register
			(pdev, DBR_MODULE_CFR, &dbr_config,
			 enh_cfr_dbr_event_handler);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_unregister_to_dbr_enh() - Unregister callback for DBR events
 * @pdev: PDEV object
 *
 * Return: status
 */
static QDF_STATUS
target_if_unregister_to_dbr_enh(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_direct_buf_rx_tx_ops *dbr_tx_ops = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		cfr_err("tx_ops is NULL");
		return QDF_STATUS_SUCCESS;
	}
	dbr_tx_ops = &tx_ops->dbr_tx_ops;
	if (dbr_tx_ops->direct_buf_rx_module_unregister) {
		return dbr_tx_ops->direct_buf_rx_module_unregister
			(pdev, DBR_MODULE_CFR);
	}

	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * dump_cfr_peer_tx_event_enh() - Dump TX completion event
 * @event: ptr to WMI TX completion event for QOS frames sent during
 * one-shot capture
 * @cookie: Index into lookup table
 *
 * Return: none
 */
static void dump_cfr_peer_tx_event_enh(wmi_cfr_peer_tx_event_param *event,
				       uint32_t cookie)
{
	cfr_debug("<TXCOMP><%u>CFR capture method: %d vdev_id: %d mac: "
		  QDF_MAC_ADDR_FMT, cookie,
		  event->capture_method, event->vdev_id,
		  QDF_MAC_ADDR_REF(event->peer_mac_addr.bytes));

	cfr_debug("<TXCOMP><%u>Chan: %d bw: %d phymode: %d cfreq1: %d cfrq2: %d "
		  "nss: %d\n",
		  cookie,
		  event->primary_20mhz_chan, event->bandwidth,
		  event->phy_mode, event->band_center_freq1,
		  event->band_center_freq2, event->spatial_streams);

	cfr_debug("<TXCOMP><%u>Correlation_info1: 0x%08x "
		  "Correlation_info2: 0x%08x\n",
		  cookie,
		  event->correlation_info_1, event->correlation_info_2);

	cfr_debug("<TXCOMP><%u>status: 0x%x ts: %d counter: %d rssi0: 0x%08x\n",
		  cookie,
		  event->status, event->timestamp_us, event->counter,
		  event->chain_rssi[0]);
}

#ifdef DIRECT_BUF_RX_ENABLE
/**
 * enh_prepare_cfr_header_txstatus() - Prepare CFR metadata for TX failures
 * @tx_evt_param: ptr to WMI TX completion event
 * @header: pointer to metadata
 *
 * Return: none
 */
static void enh_prepare_cfr_header_txstatus(wmi_cfr_peer_tx_event_param
					    *tx_evt_param,
					    struct csi_cfr_header *header,
					    uint32_t target_type)
{
	header->start_magic_num        = 0xDEADBEAF;
	header->vendorid               = 0x8cfdf0;
	header->cfr_metadata_version   = CFR_META_VERSION_3;
	header->cfr_data_version       = CFR_DATA_VERSION_1;

	if (target_type == TARGET_TYPE_QCN9000)
		header->chip_type      = CFR_CAPTURE_RADIO_PINE;
	else if (target_type == TARGET_TYPE_QCA5018)
		header->chip_type      = CFR_CAPTURE_RADIO_MAPLE;
	else
		header->chip_type      = CFR_CAPTURE_RADIO_CYP;

	header->pltform_type           = CFR_PLATFORM_TYPE_ARM;
	header->Reserved               = 0;
	header->u.meta_v3.status       = 0; /* failure */
	header->u.meta_v3.length       = 0;

	qdf_mem_copy(&header->u.meta_v3.peer_addr.su_peer_addr[0],
		     &tx_evt_param->peer_mac_addr.bytes[0], QDF_MAC_ADDR_SIZE);
}

/**
 * target_if_peer_capture_event() - WMI TX completion event for one-shot
 * capture
 * @sc: pointer to offload soc object
 * @data: WMI TX completion event buffer
 * @datalen: WMI Tx completion event buffer length
 *
 * Return: status
 */
static int
target_if_peer_capture_event(ol_scn_t sc, uint8_t *data, uint32_t datalen)
{
	QDF_STATUS retval = 0;
	struct wmi_unified *wmi_handle;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_vdev *vdev;
	uint32_t cookie;
	struct pdev_cfr *pcfr;
	struct look_up_table *lut = NULL;
	struct csi_cfr_header *header = NULL;
	struct csi_cfr_header header_error = {0};
	wmi_cfr_peer_tx_event_param tx_evt_param = {0};
	qdf_dma_addr_t buf_addr = 0, buf_addr_temp = 0;
	int status;
	struct wlan_channel *bss_chan;
	struct wlan_lmac_if_cfr_rx_ops *cfr_rx_ops = NULL;
	struct wlan_lmac_if_rx_ops *rx_ops;
	uint32_t target_type;

	if (!sc || !data) {
		cfr_err("sc or data is null");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(sc);
	if (!psoc) {
		cfr_err("psoc is null");
		return -EINVAL;
	}

	rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
	if (!rx_ops) {
		cfr_err("rx_ops is NULL");
		return -EINVAL;
	}
	cfr_rx_ops = &rx_ops->cfr_rx_ops;

	retval = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_CFR_ID);
	if (QDF_IS_STATUS_ERROR(retval)) {
		cfr_err("unable to get psoc reference");
		return -EINVAL;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		cfr_err("wmi_handle is null");
		wlan_objmgr_psoc_release_ref(psoc, WLAN_CFR_ID);
		return -EINVAL;
	}

	retval = wmi_extract_cfr_peer_tx_event_param(wmi_handle, data,
						     &tx_evt_param);

	if (retval != QDF_STATUS_SUCCESS) {
		cfr_err("Failed to extract cfr tx event param");
		wlan_objmgr_psoc_release_ref(psoc, WLAN_CFR_ID);
		return -EINVAL;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, tx_evt_param.vdev_id,
						    WLAN_CFR_ID);
	if (!vdev) {
		cfr_err("vdev is null");
		wlan_objmgr_psoc_release_ref(psoc, WLAN_CFR_ID);
		return -EINVAL;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		cfr_err("pdev is null");
		wlan_objmgr_psoc_release_ref(psoc, WLAN_CFR_ID);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_CFR_ID);
		return -EINVAL;
	}

	retval = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_CFR_ID);
	if (retval != QDF_STATUS_SUCCESS) {
		cfr_err("failed to get pdev reference");
		wlan_objmgr_psoc_release_ref(psoc, WLAN_CFR_ID);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_CFR_ID);
		return -EINVAL;
	}

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						     WLAN_UMAC_COMP_CFR);
	if (!pcfr) {
		cfr_err("pdev object for CFR is NULL");
		retval = -EINVAL;
		goto relref;
	}

	target_type = target_if_cfr_get_target_type(psoc);

	if (tx_evt_param.status & PEER_CFR_CAPTURE_EVT_PS_STATUS_MASK) {
		cfr_err("CFR capture failed as peer is in powersave: "
			QDF_MAC_ADDR_FMT,
			QDF_MAC_ADDR_REF(tx_evt_param.peer_mac_addr.bytes));

		enh_prepare_cfr_header_txstatus(&tx_evt_param,
						&header_error,
						target_type);
		if (cfr_rx_ops->cfr_info_send)
			cfr_rx_ops->cfr_info_send(pdev,
						  &header_error,
						  sizeof(struct
							 csi_cfr_header),
						  NULL, 0, &end_magic, 4);

		retval = -EINVAL;
		goto relref;
	}

	if ((tx_evt_param.status & PEER_CFR_CAPTURE_EVT_STATUS_MASK) == 0) {
		cfr_debug("CFR capture failed for peer: " QDF_MAC_ADDR_FMT,
			  QDF_MAC_ADDR_REF(tx_evt_param.peer_mac_addr.bytes));
		pcfr->tx_peer_status_cfr_fail++;
		retval = -EINVAL;
		goto relref;
	}

	if (tx_evt_param.status & CFR_TX_EVT_STATUS_MASK) {
		cfr_debug("TX packet returned status %d for peer: "
			  QDF_MAC_ADDR_FMT,
			  tx_evt_param.status & CFR_TX_EVT_STATUS_MASK,
			  QDF_MAC_ADDR_REF(tx_evt_param.peer_mac_addr.bytes));
		pcfr->tx_evt_status_cfr_fail++;
		retval = -EINVAL;
		goto relref;
	}

	buf_addr_temp = (tx_evt_param.correlation_info_2 & 0x0f);
	buf_addr = (tx_evt_param.correlation_info_1 |
		    ((uint64_t)buf_addr_temp << 32));

	if (target_if_dbr_cookie_lookup(pdev, DBR_MODULE_CFR, buf_addr,
					&cookie, 0)) {
		cfr_debug("Cookie lookup failure for addr: 0x%pK status: 0x%x",
			  (void *)((uintptr_t)buf_addr), tx_evt_param.status);
		pcfr->tx_dbr_cookie_lookup_fail++;
		retval = -EINVAL;
		goto relref;
	}

	cfr_debug("buffer address: 0x%pK cookie: %u",
		  (void *)((uintptr_t)buf_addr), cookie);

	dump_cfr_peer_tx_event_enh(&tx_evt_param, cookie);

	qdf_spin_lock_bh(&pcfr->lut_lock);

	lut = get_lut_entry(pcfr, cookie);
	if (!lut) {
		cfr_err("lut is NULL\n");
		retval = -EINVAL;
		goto unlock;
	}

	pcfr->tx_evt_cnt++;
	pcfr->total_tx_evt_cnt++;

	lut->tx_ppdu_id = (tx_evt_param.correlation_info_2 >> 16);
	lut->tx_address1 = tx_evt_param.correlation_info_1;
	lut->tx_address2 = tx_evt_param.correlation_info_2;
	lut->txrx_tstamp = qdf_ktime_to_ms(qdf_ktime_get());

	header = &lut->header;
	header->start_magic_num        = 0xDEADBEAF;
	header->vendorid               = 0x8cfdf0;
	header->cfr_metadata_version   = CFR_META_VERSION_3;
	header->cfr_data_version       = CFR_DATA_VERSION_1;

	if (target_type == TARGET_TYPE_QCN9000)
		header->chip_type      = CFR_CAPTURE_RADIO_PINE;
	else if (target_type == TARGET_TYPE_QCA5018)
		header->chip_type      = CFR_CAPTURE_RADIO_MAPLE;
	else
		header->chip_type      = CFR_CAPTURE_RADIO_CYP;

	header->pltform_type           = CFR_PLATFORM_TYPE_ARM;
	header->Reserved               = 0;
	header->u.meta_v3.status       = (tx_evt_param.status &
					  PEER_CFR_CAPTURE_EVT_STATUS_MASK) ?
					  1 : 0;
	header->u.meta_v3.capture_bw   = tx_evt_param.bandwidth;

	bss_chan = wlan_vdev_mlme_get_bss_chan(vdev);
	header->u.meta_v3.phy_mode     = bss_chan->ch_phymode;

	header->u.meta_v3.prim20_chan  = tx_evt_param.primary_20mhz_chan;
	header->u.meta_v3.center_freq1 = tx_evt_param.band_center_freq1;
	header->u.meta_v3.center_freq2 = tx_evt_param.band_center_freq2;

	/* Currently CFR data is captured on ACK of a Qos NULL frame.
	 * For 20 MHz, ACK is Legacy and for 40/80/160, ACK is DUP Legacy.
	 */
	header->u.meta_v3.capture_mode = tx_evt_param.bandwidth ?
		CFR_DUP_LEGACY_ACK : CFR_LEGACY_ACK;
	header->u.meta_v3.capture_type = tx_evt_param.capture_method;
	header->u.meta_v3.num_rx_chain = wlan_vdev_mlme_get_rxchainmask(vdev);
	header->u.meta_v3.sts_count    = tx_evt_param.spatial_streams;
	header->u.meta_v3.timestamp    = tx_evt_param.timestamp_us;

	qdf_mem_copy(&header->u.meta_v3.peer_addr.su_peer_addr[0],
		     &tx_evt_param.peer_mac_addr.bytes[0], QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(&header->u.meta_v3.chain_rssi[0],
		     &tx_evt_param.chain_rssi[0],
		     HOST_MAX_CHAINS * sizeof(tx_evt_param.chain_rssi[0]));
	qdf_mem_copy(&header->u.meta_v3.chain_phase[0],
		     &tx_evt_param.chain_phase[0],
		     HOST_MAX_CHAINS * sizeof(tx_evt_param.chain_phase[0]));

	status = correlate_and_relay_enh(pdev, cookie, lut,
					 CORRELATE_TX_EV_MODULE_ID);
	if (status == STATUS_STREAM_AND_RELEASE) {
		if (cfr_rx_ops->cfr_info_send)
			status = cfr_rx_ops->cfr_info_send(pdev,
							   &lut->header,
							   sizeof(
							   struct
							   csi_cfr_header),
							   lut->data,
							   lut->data_len,
							   &end_magic, 4);
		dump_metadata(header, cookie);
		release_lut_entry_enh(pdev, lut);
		target_if_dbr_buf_release(pdev, DBR_MODULE_CFR, buf_addr,
					  cookie, 0);
	} else {
		retval = -EINVAL;
	}

unlock:
	qdf_spin_unlock_bh(&pcfr->lut_lock);
relref:

	wlan_objmgr_psoc_release_ref(psoc, WLAN_CFR_ID);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_CFR_ID);
	wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);

	return retval;
}
#else
static int
target_if_peer_capture_event(ol_scn_t sc, uint8_t *data, uint32_t datalen)
{
	return 0;
}
#endif

/**
 * target_if_register_tx_completion_enh_event_handler() - Register callback for
 * WMI TX completion event
 * @psoc: PSOC object
 *
 * Return: Success/Failure status
 */
static int
target_if_register_tx_completion_enh_event_handler(struct wlan_objmgr_psoc
						   *psoc)
{
	/* Register completion handler here */
	wmi_unified_t wmi_hdl;
	int ret = 0;

	wmi_hdl = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_hdl) {
		cfr_err("Unable to get wmi handle");
		return -EINVAL;
	}

	ret = wmi_unified_register_event_handler(wmi_hdl,
						 wmi_peer_cfr_capture_event_id,
						 target_if_peer_capture_event,
						 WMI_RX_UMAC_CTX);
	/*
	 * Event registration is called per pdev
	 * Ignore erorr if event is alreday registred.
	 */
	if (ret == QDF_STATUS_E_FAILURE)
		ret = QDF_STATUS_SUCCESS;

	return ret;
}

/**
 * target_if_unregister_tx_completion_enh_event_handler() - Unregister callback
 * for WMI TX completion event
 * @psoc: PSOC object
 *
 * Return: Success/Failure status
 */
static int
target_if_unregister_tx_completion_enh_event_handler(struct wlan_objmgr_psoc
						     *psoc)
{
	/* Unregister completion handler here */
	wmi_unified_t wmi_hdl;
	int status = 0;

	wmi_hdl = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_hdl) {
		cfr_err("Unable to get wmi handle");
		return -EINVAL;
	}

	status = wmi_unified_unregister_event(wmi_hdl,
					      wmi_peer_cfr_capture_event_id);
	return status;
}

/**
 * lut_ageout_timer_task() - Timer to flush pending TXRX/DBR events
 *
 * Return: none
 */
static os_timer_func(lut_ageout_timer_task)
{
	int i = 0;
	struct pdev_cfr *pcfr = NULL;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct look_up_table *lut = NULL;
	uint64_t diff, cur_tstamp;
	uint8_t srng_id = 0;

	OS_GET_TIMER_ARG(pcfr, struct pdev_cfr*);

	if (!pcfr) {
		cfr_err("pdev object for CFR is null");
		return;
	}

	pdev = pcfr->pdev_obj;
	if (!pdev) {
		cfr_err("pdev is null");
		return;
	}

	srng_id = pcfr->rcc_param.srng_id;
	if (wlan_objmgr_pdev_try_get_ref(pdev, WLAN_CFR_ID)
	    != QDF_STATUS_SUCCESS) {
		cfr_err("failed to get pdev reference");
		return;
	}

	cur_tstamp = qdf_ktime_to_ms(qdf_ktime_get());

	qdf_spin_lock_bh(&pcfr->lut_lock);

	for (i = 0; i < pcfr->lut_num; i++) {
		lut = get_lut_entry(pcfr, i);
		if (!lut)
			continue;

		if (lut->dbr_recv && !lut->tx_recv) {
			diff = cur_tstamp - lut->dbr_tstamp;
			if (diff > LUT_AGE_THRESHOLD) {
				target_if_dbr_buf_release(pdev, DBR_MODULE_CFR,
							  lut->dbr_address,
							  i, srng_id);
				pcfr->flush_timeout_dbr_cnt++;
				release_lut_entry_enh(pdev, lut);
			}
		}
	}

	qdf_spin_unlock_bh(&pcfr->lut_lock);

	if (pcfr->lut_timer_init)
		qdf_timer_mod(&pcfr->lut_age_timer, LUT_AGE_TIMER);
	wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);
}

/**
 * target_if_cfr_start_lut_age_timer() - Start timer to flush aged-out LUT
 * entries
 * @pdev: pointer to pdev object
 *
 * Return: None
 */
void target_if_cfr_start_lut_age_timer(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pcfr;

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						     WLAN_UMAC_COMP_CFR);
	if (pcfr->lut_timer_init)
		qdf_timer_mod(&pcfr->lut_age_timer, LUT_AGE_TIMER);
}

/**
 * target_if_cfr_stop_lut_age_timer() - Stop timer to flush aged-out LUT
 * entries
 * @pdev: pointer to pdev object
 *
 * Return: None
 */
void target_if_cfr_stop_lut_age_timer(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pcfr;

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						     WLAN_UMAC_COMP_CFR);
	if (pcfr->lut_timer_init)
		qdf_timer_stop(&pcfr->lut_age_timer);
}

/**
 * target_if_cfr_update_global_cfg() - Update global config after a successful
 * commit
 * @pdev: pointer to pdev object
 *
 * Return: None
 */
void target_if_cfr_update_global_cfg(struct wlan_objmgr_pdev *pdev)
{
	int grp_id;
	struct pdev_cfr *pcfr;
	struct ta_ra_cfr_cfg *curr_cfg = NULL;
	struct ta_ra_cfr_cfg *glbl_cfg = NULL;

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						     WLAN_UMAC_COMP_CFR);

	for (grp_id = 0; grp_id < MAX_TA_RA_ENTRIES; grp_id++) {
		if (qdf_test_bit(grp_id,
				 &pcfr->rcc_param.modified_in_curr_session)) {
			/* Populating global config based on user's input */
			glbl_cfg = &pcfr->global[grp_id];
			curr_cfg = &pcfr->rcc_param.curr[grp_id];

			if (curr_cfg->valid_ta)
				qdf_mem_copy(glbl_cfg->tx_addr,
					     curr_cfg->tx_addr,
					     QDF_MAC_ADDR_SIZE);

			if (curr_cfg->valid_ra)
				qdf_mem_copy(glbl_cfg->rx_addr,
					     curr_cfg->rx_addr,
					     QDF_MAC_ADDR_SIZE);

			if (curr_cfg->valid_ta_mask)
				qdf_mem_copy(glbl_cfg->tx_addr_mask,
					     curr_cfg->tx_addr_mask,
					     QDF_MAC_ADDR_SIZE);

			if (curr_cfg->valid_ra_mask)
				qdf_mem_copy(glbl_cfg->rx_addr_mask,
					     curr_cfg->rx_addr_mask,
					     QDF_MAC_ADDR_SIZE);

			if (curr_cfg->valid_bw_mask)
				glbl_cfg->bw = curr_cfg->bw;

			if (curr_cfg->valid_nss_mask)
				glbl_cfg->nss = curr_cfg->nss;

			if (curr_cfg->valid_mgmt_subtype)
				glbl_cfg->mgmt_subtype_filter =
					curr_cfg->mgmt_subtype_filter;

			if (curr_cfg->valid_ctrl_subtype)
				glbl_cfg->ctrl_subtype_filter =
					curr_cfg->ctrl_subtype_filter;

			if (curr_cfg->valid_data_subtype)
				glbl_cfg->data_subtype_filter =
					curr_cfg->data_subtype_filter;
		}
	}
}

/**
 * cfr_enh_init_pdev() - Inits cfr pdev and registers necessary handlers.
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: Registration status for necessary handlers
 */
QDF_STATUS cfr_enh_init_pdev(struct wlan_objmgr_psoc *psoc,
			     struct wlan_objmgr_pdev *pdev)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct pdev_cfr *pcfr;
	uint32_t target_type;
	struct psoc_cfr *cfr_sc;

	if (!pdev) {
		cfr_err("PDEV is NULL!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!psoc) {
		cfr_err("PSOC is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						     WLAN_UMAC_COMP_CFR);
	if (!pcfr) {
		cfr_err("pcfr is NULL!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	cfr_sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						       WLAN_UMAC_COMP_CFR);

	if (!cfr_sc) {
		cfr_err("cfr_sc is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	target_type = target_if_cfr_get_target_type(psoc);

#if DIRECT_BUF_RX_ENABLE
	status = target_if_register_to_dbr_enh(pdev);
	if (status != QDF_STATUS_SUCCESS) {
		cfr_err("Failed to register with dbr");
		return status;
	}
#endif

	status = target_if_register_tx_completion_enh_event_handler(psoc);
	if (status != QDF_STATUS_SUCCESS) {
		cfr_err("Failed to register with tx event handler");
		return status;
	}

	pcfr->is_cfr_rcc_capable = 1;
	pcfr->rcc_param.pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	pcfr->rcc_param.modified_in_curr_session = MAX_RESET_CFG_ENTRY;
	pcfr->rcc_param.num_grp_tlvs = MAX_TA_RA_ENTRIES;
	pcfr->rcc_param.vdev_id = CFR_INVALID_VDEV_ID;
	pcfr->rcc_param.srng_id = DEFAULT_SRNGID_CFR;
	pcfr->is_cap_interval_mode_sel_support =
				cfr_sc->is_cap_interval_mode_sel_support;
	pcfr->is_mo_marking_support = cfr_sc->is_mo_marking_support;

	target_if_cfr_default_ta_ra_config(&pcfr->rcc_param,
					   true, MAX_RESET_CFG_ENTRY);

	status = target_if_cfr_config_rcc(pdev, &pcfr->rcc_param);
	if (status == QDF_STATUS_SUCCESS) {
		/* Update global configuration */
		target_if_cfr_update_global_cfg(pdev);
	} else {
		cfr_err("Sending WMI to configure default has failed");
		return status;
	}

	pcfr->rcc_param.modified_in_curr_session = 0;

	pcfr->cfr_max_sta_count = MAX_CFR_ENABLED_CLIENTS;

	if (target_type == TARGET_TYPE_QCN9000) {
		pcfr->subbuf_size = STREAMFS_MAX_SUBBUF_PINE;
		pcfr->num_subbufs = STREAMFS_NUM_SUBBUF_PINE;
		pcfr->chip_type = CFR_CAPTURE_RADIO_PINE;
		pcfr->max_mu_users = PINE_CFR_MU_USERS;
	} else if (target_type == TARGET_TYPE_QCA5018) {
		pcfr->subbuf_size = STREAMFS_MAX_SUBBUF_MAPLE;
		pcfr->num_subbufs = STREAMFS_NUM_SUBBUF_MAPLE;
		pcfr->chip_type = CFR_CAPTURE_RADIO_MAPLE;
		pcfr->max_mu_users = MAPLE_CFR_MU_USERS;
	} else {
		pcfr->subbuf_size = STREAMFS_MAX_SUBBUF_CYP;
		pcfr->num_subbufs = STREAMFS_NUM_SUBBUF_CYP;
		pcfr->chip_type = CFR_CAPTURE_RADIO_CYP;
		pcfr->max_mu_users = CYP_CFR_MU_USERS;
	}

	if (!pcfr->lut_timer_init) {
		qdf_timer_init(NULL,
			       &(pcfr->lut_age_timer),
			       lut_ageout_timer_task, (void *)pcfr,
			       QDF_TIMER_TYPE_WAKE_APPS);
		pcfr->lut_timer_init = 1;
	}

	qdf_spinlock_create(&pcfr->lut_lock);

	return status;
}

/**
 * cfr_enh_deinit_pdev() - De-inits corresponding pdev and handlers.
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: De-registration status for necessary handlers
 */
QDF_STATUS cfr_enh_deinit_pdev(struct wlan_objmgr_psoc *psoc,
			       struct wlan_objmgr_pdev *pdev)
{
	int status;
	struct pdev_cfr *pcfr;

	pcfr = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						     WLAN_UMAC_COMP_CFR);
	if (!pcfr) {
		cfr_err("pcfr is NULL");
		return -EINVAL;
	}

	if (pcfr->lut_timer_init) {
		qdf_timer_stop(&pcfr->lut_age_timer);
		qdf_timer_free(&(pcfr->lut_age_timer));
		pcfr->lut_timer_init = 0;
	}

	pcfr->tx_evt_cnt = 0;
	pcfr->dbr_evt_cnt = 0;
	pcfr->release_cnt = 0;
	pcfr->total_tx_evt_cnt = 0;
	pcfr->rx_tlv_evt_cnt = 0;
	pcfr->flush_dbr_cnt = 0;
	pcfr->flush_timeout_dbr_cnt = 0;
	pcfr->invalid_dma_length_cnt = 0;
	pcfr->clear_txrx_event = 0;
	pcfr->cfr_dma_aborts = 0;
	pcfr->tx_peer_status_cfr_fail = 0;
	pcfr->tx_evt_status_cfr_fail = 0;
	pcfr->tx_dbr_cookie_lookup_fail = 0;
	qdf_mem_zero(&pcfr->rcc_param, sizeof(struct cfr_rcc_param));
	qdf_mem_zero(&pcfr->global, (sizeof(struct ta_ra_cfr_cfg) *
				     MAX_TA_RA_ENTRIES));
	pcfr->cfr_timer_enable = 0;

#ifdef DIRECT_BUF_RX_ENABLE
	status = target_if_unregister_to_dbr_enh(pdev);
	if (status != QDF_STATUS_SUCCESS)
		cfr_err("Failed to register with dbr");
#endif

	status = target_if_unregister_tx_completion_enh_event_handler(psoc);
	if (status != QDF_STATUS_SUCCESS)
		cfr_err("Failed to register with dbr");

	qdf_spinlock_destroy(&pcfr->lut_lock);

	return status;
}
