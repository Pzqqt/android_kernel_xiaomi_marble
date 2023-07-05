/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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
#include <ol_if_athvar.h>
#include <target_if.h>
#include <wlan_lmac_if_def.h>
#include <wlan_osif_priv.h>
#include <init_deinit_lmac.h>
#include <wlan_cfr_utils_api.h>
#include <target_if_cfr_dbr.h>
#ifdef DIRECT_BUF_RX_ENABLE
#include <target_if_direct_buf_rx_api.h>
#endif

#ifdef DIRECT_BUF_RX_ENABLE
static u_int32_t end_magic = 0xBEAFDEAD;

/**
 * dump_lut() - dump all valid lut entries
 * @pdev: objmgr pdev
 *
 * return: none
 */
static int dump_lut(struct wlan_objmgr_pdev *pdev)
{
	struct pdev_cfr *pdev_cfrobj;
	struct look_up_table *lut = NULL;
	int i = 0;

	pdev_cfrobj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							    WLAN_UMAC_COMP_CFR);
	if (!pdev_cfrobj) {
		cfr_err("pdev object for CFR is null");
		return -EINVAL;
	}

	for (i = 0; i < pdev_cfrobj->lut_num; i++) {
		lut = pdev_cfrobj->lut[i];
		cfr_info("idx:%d dbrevnt: %d txevent: %d dbrppdu:0x%x txppdu:0x%x",
			 i, lut->dbr_recv, lut->tx_recv,
			 lut->dbr_ppdu_id, lut->tx_ppdu_id);
	}

	return 0;
}

/**
 * dump_dma_hdr() - Dump DMA header populated by uCode
 * @dma_hdr: pointer to the DMA header
 * @error: Indicates whether it is an error
 *
 * Return: none
 */
static void dump_dma_hdr(struct whal_cfir_dma_hdr *dma_hdr, int error)
{
	if (!error) {
		cfr_debug("Tag: 0x%02x Length: %d udone: %d ctype: %d preamble: %d",
			  dma_hdr->tag, dma_hdr->length, dma_hdr->upload_done,
			  dma_hdr->capture_type, dma_hdr->preamble_type);

		cfr_debug("Nss: %d num_chains: %d bw: %d", dma_hdr->nss,
			  dma_hdr->num_chains, dma_hdr->upload_pkt_bw);

		cfr_debug("peervalid: %d peer_id: %d ppdu_id: 0x%04x",
			  dma_hdr->sw_peer_id_valid, dma_hdr->sw_peer_id,
			  dma_hdr->phy_ppdu_id);
	} else {
		cfr_err("Tag: 0x%02x Length: %d udone: %d ctype: %d preamble: %d",
			dma_hdr->tag, dma_hdr->length, dma_hdr->upload_done,
			dma_hdr->capture_type,	dma_hdr->preamble_type);

		cfr_err("Nss: %d num_chains: %d bw: %d", dma_hdr->nss,
			dma_hdr->num_chains, dma_hdr->upload_pkt_bw);

		cfr_err("peervalid: %d peer_id: %d ppdu_id: 0x%04x",
			dma_hdr->sw_peer_id_valid, dma_hdr->sw_peer_id,
			dma_hdr->phy_ppdu_id);
	}
}

/**
 * compute_length() - Compute the number of tones based on BW
 * @dma_hdr: DMA header from uCode
 *
 * Return: Computed number of tones based on BW
 */
static int compute_length(struct whal_cfir_dma_hdr *dma_hdr)
{
	uint8_t bw = dma_hdr->upload_pkt_bw;
	uint8_t preamble = dma_hdr->preamble_type;

	switch (preamble) {
	case 0:
	case 2:
		switch (bw) {
		case 0:
			return TONES_IN_20MHZ;
		case 1: /* DUP40/VHT40 */
			return TONES_IN_40MHZ;
		case 2: /* DUP80/VHT80 */
			return TONES_IN_80MHZ;
		case 3: /* DUP160/VHT160 */
			return TONES_IN_160MHZ;
		}

	case 1:
		switch (bw) {
		case 0:
			return TONES_IN_20MHZ;
		case 1:
			return TONES_IN_40MHZ;
		}
	}

	return TONES_INVALID;
}

/**
 * release_lut_entry() - Clear all params in an LUT entry
 * @pdev: objmgr PDEV
 * @lut: pointer to LUT
 *
 * Return: status
 */
static int release_lut_entry(struct wlan_objmgr_pdev *pdev,
			     struct look_up_table *lut)
{
	lut->dbr_recv = false;
	lut->tx_recv = false;
	lut->data = NULL;
	lut->data_len = 0;
	lut->dbr_ppdu_id = 0;
	lut->tx_ppdu_id = 0;
	qdf_mem_zero(&lut->header, sizeof(struct csi_cfr_header));

	return 0;
}

/**
 * correlate_and_relay() - Correlate TXRX and DBR events and stream CFR data to
 * userspace
 * @pdev: objmgr PDEV
 * @cookie: Index into lookup table
 * @lut: pointer to lookup table
 * @module_id: ID of the event received
 *  0 - DBR event
 *  1 - TXRX event
 *
 * Return:
 *  - STATUS_ERROR
 *  - STATUS_HOLD
 *  - STATUS_STREAM_AND_RELEASE
 */
static int correlate_and_relay(struct wlan_objmgr_pdev *pdev, uint32_t cookie,
			       struct look_up_table *lut, uint8_t module_id)
{
	struct pdev_cfr *pdev_cfrobj;
	int status = STATUS_ERROR;

	if (module_id > 1) {
		cfr_err("Received request with invalid mod id. Investigate!!");
		QDF_ASSERT(0);
		return status;
	}

	pdev_cfrobj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							    WLAN_UMAC_COMP_CFR);

	if (module_id == CORRELATE_TX_EV_MODULE_ID) {
		pdev_cfrobj->tx_evt_cnt++;
		lut->tx_recv = true;
	} else if (module_id == CORRELATE_DBR_MODULE_ID) {
		pdev_cfrobj->dbr_evt_cnt++;
		lut->dbr_recv = true;
	}

	if (lut->dbr_recv && lut->tx_recv) {
		if (lut->dbr_ppdu_id == lut->tx_ppdu_id) {
			pdev_cfrobj->release_cnt++;
			status = STATUS_STREAM_AND_RELEASE;
		} else {
			/*
			 * When there is a ppdu id mismatch, discard the other
			 * older event's data and wait hold for new event
			 */
			if (module_id == CORRELATE_TX_EV_MODULE_ID) {
				cfr_debug("Received new tx event for same cookie %u",
					  cookie);
				lut->dbr_recv = false;
				lut->data = NULL;
				lut->data_len = 0;
				lut->dbr_ppdu_id = 0;
				qdf_mem_zero(&lut->dbr_address,
					     sizeof(lut->dbr_address));
			} else if (module_id == CORRELATE_DBR_MODULE_ID) {
				cfr_debug("Received new dbr event for same cookie %u",
					  cookie);
				lut->tx_recv = false;
				lut->tx_ppdu_id = 0;
			}

			/*
			 * This is condition can occur if DBR buffer did not get
			 * released, or leaked either by Host/Target.
			 * we may need to add recovery here.
			 *
			 * 1. Stop all captures
			 * 2. Flush/release DBR buffer and LUT
			 * 3. Start capture again
			 */
			if ((pdev_cfrobj->dbr_evt_cnt -
				pdev_cfrobj->release_cnt) >= MAX_LUT_ENTRIES) {
				cfr_err("cookie = %u dbr_cnt = %llu, release_cnt = %llu",
					cookie, pdev_cfrobj->dbr_evt_cnt,
					pdev_cfrobj->release_cnt);
				dump_lut(pdev);
				dump_dma_hdr(&lut->dma_hdr, 1);
				cfr_err("correlation_info1: 0x%08x correlation_info2 0x%08x",
					lut->tx_address1, lut->tx_address2);
			}
			status = STATUS_HOLD;
		}
	} else {
		status = STATUS_HOLD;
	}

	return status;
}

/**
 * cfr_dbr_event_handler() - Process DBR event for CFR data DMA completion
 * @pdev: PDEV object
 * @payload: pointer to CFR data
 *
 * Return: status
 */
static bool cfr_dbr_event_handler(struct wlan_objmgr_pdev *pdev,
				  struct direct_buf_rx_data *payload)
{
	uint8_t *data = NULL;
	uint32_t cookie = 0;
	struct whal_cfir_dma_hdr dma_hdr = {0};
	int  length = 8, tones = 0, status = 0;
	struct wlan_objmgr_psoc *psoc;
	struct pdev_cfr *pdev_cfrobj;
	struct look_up_table *lut = NULL;
	struct csi_cfr_header *header = NULL;
	struct wlan_lmac_if_rx_ops *rx_ops;
	bool ret = true;

	if ((!pdev) || (!payload)) {
		cfr_err("pdev or payload is null");
		return true;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		cfr_err("psoc is null");
		return true;
	}

	pdev_cfrobj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							    WLAN_UMAC_COMP_CFR);
	if (!pdev_cfrobj) {
		cfr_err("pdev object for CFR is null");
		return true;
	}

	data = payload->vaddr;
	cookie = payload->cookie;

	cfr_debug("bufferaddr: 0x%pK cookie: %u",
		  (void *)((uintptr_t)payload->paddr), cookie);
	qdf_mem_copy(&dma_hdr, &data[0], sizeof(struct whal_cfir_dma_hdr));

	dump_dma_hdr(&dma_hdr, 0);
	tones = compute_length(&dma_hdr);
	if (tones == TONES_INVALID) {
		cfr_err("Number of tones received is invalid. Investigate!");
		return true;
	}

	length += tones * (dma_hdr.num_chains + 1);

	lut = pdev_cfrobj->lut[cookie];
	lut->data = data;
	lut->data_len = length;
	lut->dbr_ppdu_id = dma_hdr.phy_ppdu_id;
	lut->dbr_address = payload->paddr;
	qdf_mem_copy(&lut->dma_hdr, &dma_hdr, sizeof(struct whal_cfir_dma_hdr));

	header = &lut->header;
	header->u.meta_dbr.channel_bw = dma_hdr.upload_pkt_bw;
	header->u.meta_dbr.length = length;
	status = correlate_and_relay(pdev, cookie, lut,
				     CORRELATE_DBR_MODULE_ID);
	if (status == STATUS_STREAM_AND_RELEASE) {
		/*
		 * Message format
		 *  Meta data Header + actual payload + trailer
		 */
		rx_ops = wlan_psoc_get_lmac_if_rxops(psoc);
		if (!rx_ops) {
			cfr_err("rx_ops is NULL");
			return true;
		}

		status = rx_ops->cfr_rx_ops.cfr_info_send
				(pdev, &lut->header,
				 sizeof(struct csi_cfr_header),
				 lut->data, lut->data_len,
				 &end_magic, 4);
		release_lut_entry(pdev, lut);
		cfr_debug("Data sent to upper layers, released look up table");
		ret = true;
	} else if (status == STATUS_HOLD) {
		cfr_debug("Tx event not received yet. Buffer is not released");
		ret = false;
	} else {
		cfr_err("Correlation returned invalid status!!");
		ret = true;
	}

	return ret;
}

/**
 * dump_cfr_peer_tx_event() - Dump TX completion event
 * @event: ptr to WMI TX completion event for QOS frames sent during
 * one-shot capture.
 *
 * Return: none
 */
static void dump_cfr_peer_tx_event(wmi_cfr_peer_tx_event_param *event)
{
	cfr_debug("CFR capture method: %u vdev_id: %u mac: " QDF_MAC_ADDR_FMT,
		  event->capture_method, event->vdev_id,
		  QDF_MAC_ADDR_REF(&event->peer_mac_addr.bytes[0]));

	cfr_debug("Chan: %u bw: %u phymode: %u cfreq1: %u cfrq2: %u nss: %u",
		  event->primary_20mhz_chan, event->bandwidth,
		  event->phy_mode, event->band_center_freq1,
		  event->band_center_freq2, event->spatial_streams);

	cfr_debug("Correlation_info1: 0x%08x Correlation_info2: 0x%08x",
		  event->correlation_info_1, event->correlation_info_2);

	cfr_debug("status: 0x%x ts: %u counter: %u rssi0: 0x%08x",
		  event->status, event->timestamp_us, event->counter,
		  event->chain_rssi[0]);

	cfr_debug("phase0: 0x%04x phase1: 0x%04x phase2: 0x%04x phase3: 0x%04x\n"
		  "phase4: 0x%04x phase5: 0x%04x phase6: 0x%04x phase7: 0x%04x",
		  event->chain_phase[0], event->chain_phase[1],
		  event->chain_phase[2], event->chain_phase[3],
		  event->chain_phase[4], event->chain_phase[5],
		  event->chain_phase[6], event->chain_phase[7]);

	cfr_debug("rtt_cfo_measurement: %d\n", event->cfo_measurement);

	cfr_debug("rx_start_ts: %u\n", event->rx_start_ts);

	cfr_debug("mcs_rate: %u\n", event->mcs_rate);

	cfr_debug("gi_type: %u\n", event->gi_type);

	cfr_debug("agc_gain0: %u agc_gain1: %u agc_gain2: %u agc_gain3: %u\n"
		  "agc_gain4: %u agc_gain5: %u agc_gain6: %u agc_gain7: %u\n",
		  event->agc_gain[0], event->agc_gain[1],
		  event->agc_gain[2], event->agc_gain[3],
		  event->agc_gain[4], event->agc_gain[5],
		  event->agc_gain[6], event->agc_gain[7]);
	cfr_debug("gain_tbl_idx0: %u gain_tbl_idx1: %u gain_tbl_idx2: %u\n"
		  "gain_tbl_idx3: %u gain_tbl_idx4: %u gain_tbl_idx5: %u\n"
		  "gain_tbl_idx6: %u gain_tbl_idx7: %u\n",
		  event->agc_gain_tbl_index[0], event->agc_gain_tbl_index[1],
		  event->agc_gain_tbl_index[2], event->agc_gain_tbl_index[3],
		  event->agc_gain_tbl_index[4], event->agc_gain_tbl_index[5],
		  event->agc_gain_tbl_index[6], event->agc_gain_tbl_index[7]);
}

/**
 * prepare_cfr_header_txstatus() - Prepare CFR metadata for TX failures
 * @tx_evt_param: ptr to WMI TX completion event
 * @header: pointer to metadata
 *
 * Return: none
 */
static
void prepare_cfr_header_txstatus(wmi_cfr_peer_tx_event_param *tx_evt_param,
				 struct csi_cfr_header *header,
				 uint32_t target_type)
{
	target_if_cfr_fill_header(header, false, target_type, false);
	header->u.meta_dbr.status = 0; /* failure */
	header->u.meta_dbr.length = 0;
	qdf_mem_copy(&header->u.meta_dbr.peer_addr[0],
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
	struct pdev_cfr *pdev_cfrobj;
	struct look_up_table *lut = NULL;
	struct csi_cfr_header *header = NULL;
	struct csi_cfr_header header_error = {0};
	wmi_cfr_peer_tx_event_param tx_evt_param = {0};
	qdf_dma_addr_t buf_addr = 0, buf_addr_temp = 0;
	int status;
	struct wlan_lmac_if_rx_ops *rx_ops;
	uint32_t target_type;

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

	dump_cfr_peer_tx_event(&tx_evt_param);

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

	pdev_cfrobj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							    WLAN_UMAC_COMP_CFR);
	if (!pdev_cfrobj) {
		cfr_err("pdev object for CFR is NULL");
		status = -EINVAL;
		goto done;
	}

	target_type = target_if_cfr_get_target_type(psoc);

	if (tx_evt_param.status & PEER_CFR_CAPTURE_EVT_PS_STATUS_MASK) {
		cfr_debug("CFR capture failed as peer is in powersave : " QDF_MAC_ADDR_FMT,
			  QDF_MAC_ADDR_REF(&tx_evt_param.peer_mac_addr.bytes[0]));
		status = -EINVAL;
		goto relay_failure;
	}

	if ((tx_evt_param.status & PEER_CFR_CAPTURE_EVT_STATUS_MASK) == 0) {
		cfr_debug("CFR capture failed for peer : " QDF_MAC_ADDR_FMT,
			  QDF_MAC_ADDR_REF(&tx_evt_param.peer_mac_addr.bytes[0]));
		status = -EINVAL;
		pdev_cfrobj->tx_peer_status_cfr_fail++;
		goto relay_failure;
	}

	if (tx_evt_param.status & CFR_TX_EVT_STATUS_MASK) {
		cfr_debug("TX packet returned status %d for peer: " QDF_MAC_ADDR_FMT,
			  tx_evt_param.status & CFR_TX_EVT_STATUS_MASK,
			  QDF_MAC_ADDR_REF(&tx_evt_param.peer_mac_addr.bytes[0]));
		status = -EINVAL;
		pdev_cfrobj->tx_evt_status_cfr_fail++;
		goto relay_failure;
	}

	buf_addr_temp = (tx_evt_param.correlation_info_2 & 0x0f);
	buf_addr = (tx_evt_param.correlation_info_1 |
		    ((uint64_t)buf_addr_temp << 32));

	if (target_if_dbr_cookie_lookup(pdev, DBR_MODULE_CFR, buf_addr,
					&cookie, 0)) {
		cfr_debug("Cookie lookup failure for addr: 0x%pK status: 0x%x",
			  (void *)((uintptr_t)buf_addr), tx_evt_param.status);
		status = -EINVAL;
		pdev_cfrobj->tx_dbr_cookie_lookup_fail++;
		goto done;
	}

	cfr_debug("buffer address: 0x%pK cookie: %u",
		  (void *)((uintptr_t)buf_addr), cookie);

	lut = pdev_cfrobj->lut[cookie];
	lut->tx_ppdu_id = (tx_evt_param.correlation_info_2 >> 16);
	lut->tx_address1 = tx_evt_param.correlation_info_1;
	lut->tx_address2 = tx_evt_param.correlation_info_2;
	header = &lut->header;

	target_if_cfr_fill_header(header, false, target_type, false);

	header->u.meta_dbr.status        = (tx_evt_param.status &
					    PEER_CFR_CAPTURE_EVT_STATUS_MASK) ?
					    1 : 0;
	header->u.meta_dbr.capture_bw    = tx_evt_param.bandwidth;
	header->u.meta_dbr.phy_mode      = tx_evt_param.phy_mode;
	header->u.meta_dbr.prim20_chan   = tx_evt_param.primary_20mhz_chan;
	header->u.meta_dbr.center_freq1  = tx_evt_param.band_center_freq1;
	header->u.meta_dbr.center_freq2  = tx_evt_param.band_center_freq2;
	/* Currently CFR data is captured on ACK of a Qos NULL frame.
	 * For 20 MHz, ACK is Legacy and for 40/80/160, ACK is DUP Legacy.
	 */
	header->u.meta_dbr.capture_mode  = tx_evt_param.bandwidth ?
					   CFR_DUP_LEGACY_ACK : CFR_LEGACY_ACK;
	header->u.meta_dbr.capture_type  = tx_evt_param.capture_method;
	header->u.meta_dbr.num_rx_chain  = wlan_vdev_mlme_get_rxchainmask(vdev);
	header->u.meta_dbr.sts_count     = tx_evt_param.spatial_streams;
	header->u.meta_dbr.timestamp     = tx_evt_param.timestamp_us;
	header->u.meta_dbr.rx_start_ts   = tx_evt_param.rx_start_ts;
	header->u.meta_dbr.rtt_cfo_measurement = tx_evt_param.cfo_measurement;
	header->u.meta_dbr.mcs_rate      = tx_evt_param.mcs_rate;
	header->u.meta_dbr.gi_type       = tx_evt_param.gi_type;

	qdf_mem_copy(&header->u.meta_dbr.agc_gain[0],
		     &tx_evt_param.agc_gain[0],
		     HOST_MAX_CHAINS * sizeof(tx_evt_param.agc_gain[0]));
	qdf_mem_copy(&header->u.meta_dbr.peer_addr[0],
		     &tx_evt_param.peer_mac_addr.bytes[0], QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(&header->u.meta_dbr.chain_rssi[0],
		     &tx_evt_param.chain_rssi[0],
		     HOST_MAX_CHAINS * sizeof(tx_evt_param.chain_rssi[0]));
	qdf_mem_copy(&header->u.meta_dbr.chain_phase[0],
		     &tx_evt_param.chain_phase[0],
		     HOST_MAX_CHAINS * sizeof(tx_evt_param.chain_phase[0]));
	qdf_mem_copy(&header->u.meta_dbr.agc_gain_tbl_index[0],
		     &tx_evt_param.agc_gain_tbl_index[0],
		     (HOST_MAX_CHAINS *
		      sizeof(tx_evt_param.agc_gain_tbl_index[0])));

	status = correlate_and_relay(pdev, cookie, lut,
				     CORRELATE_TX_EV_MODULE_ID);
	if (status == STATUS_STREAM_AND_RELEASE) {
		status = rx_ops->cfr_rx_ops.cfr_info_send(pdev, &lut->header,
				sizeof(struct csi_cfr_header),
				lut->data, lut->data_len, &end_magic, 4);
		release_lut_entry(pdev, lut);
		target_if_dbr_buf_release(pdev, DBR_MODULE_CFR, buf_addr,
					  cookie, 0);
		cfr_debug("Data sent to upper layers, releasing look up table");
	} else if (status == STATUS_HOLD) {
		cfr_debug("HOLD for buffer address: 0x%pK cookie: %u",
			  (void *)((uintptr_t)buf_addr), cookie);
	} else {
		cfr_err("Correlation returned invalid status!!");
		status = -EINVAL;
		goto done;
	}

	status = 0;
	goto done;

relay_failure:
	prepare_cfr_header_txstatus(&tx_evt_param, &header_error, target_type);
	rx_ops->cfr_rx_ops.cfr_info_send(pdev, &header_error,
					 sizeof(struct csi_cfr_header),
					 NULL, 0, &end_magic, 4);
done:
	wlan_objmgr_psoc_release_ref(psoc, WLAN_CFR_ID);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_CFR_ID);
	wlan_objmgr_pdev_release_ref(pdev, WLAN_CFR_ID);
	return status;
}
#else
static int
target_if_peer_capture_event(ol_scn_t sc, uint8_t *data, uint32_t datalen)
{
	return 0;
}
#endif

/**
 * target_if_register_tx_completion_event_handler()
 * register TX completion handler
 * @pdev: pointer to pdev object
 *
 * Return: Status
 */
static QDF_STATUS
target_if_register_tx_completion_event_handler(struct wlan_objmgr_psoc *psoc)
{
	/* Register completion handler here */
	wmi_unified_t wmi_hdl;
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	wmi_hdl = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_hdl) {
		cfr_err("Unable to get wmi handle");
		return QDF_STATUS_E_NULL_VALUE;
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
 * target_if_unregister_tx_completion_event_handler
 * unregister TX completion handler
 * @pdev: pointer to pdev object
 *
 * Return: Status
 */
static QDF_STATUS
target_if_unregister_tx_completion_event_handler(struct wlan_objmgr_psoc *psoc)
{
	/* Unregister completion handler here */
	wmi_unified_t wmi_hdl;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	wmi_hdl = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_hdl) {
		cfr_err("Unable to get wmi handle");
		return QDF_STATUS_E_NULL_VALUE;
	}

	status = wmi_unified_unregister_event(wmi_hdl,
					      wmi_peer_cfr_capture_event_id);
	return status;
}

#ifdef DIRECT_BUF_RX_ENABLE
/**
 * target_if_register_to_dbr() - Register to Direct DMA handler
 * @pdev: pointer to pdev object
 *
 * Return: Status
 */
static QDF_STATUS
target_if_register_to_dbr(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_direct_buf_rx_tx_ops *dbr_tx_ops = NULL;
	struct dbr_module_config dbr_config;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		cfr_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_tx_ops = &tx_ops->dbr_tx_ops;
	dbr_config.num_resp_per_event = DBR_NUM_RESP_PER_EVENT_CFR;
	dbr_config.event_timeout_in_ms = DBR_EVENT_TIMEOUT_IN_MS_CFR;
	if (dbr_tx_ops->direct_buf_rx_module_register) {
		return dbr_tx_ops->direct_buf_rx_module_register
			(pdev, DBR_MODULE_CFR, &dbr_config,
			 cfr_dbr_event_handler);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_unregister_to_dbr() - Unregister callback for DBR events
 * @pdev: PDEV object
 *
 * Return: status
 */
static QDF_STATUS
target_if_unregister_to_dbr(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_direct_buf_rx_tx_ops *dbr_tx_ops = NULL;
	struct wlan_lmac_if_tx_ops *tx_ops;

	psoc = wlan_pdev_get_psoc(pdev);
	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		cfr_err("tx_ops is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	dbr_tx_ops = &tx_ops->dbr_tx_ops;
	if (dbr_tx_ops->direct_buf_rx_module_unregister) {
		return dbr_tx_ops->direct_buf_rx_module_unregister
			(pdev, DBR_MODULE_CFR);
	}

	return QDF_STATUS_SUCCESS;
}

#else
static QDF_STATUS
target_if_cfr_register_to_dbr(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
target_if_unregister_to_dbr(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

#endif

QDF_STATUS cfr_dbr_init_pdev(struct wlan_objmgr_psoc *psoc,
			     struct wlan_objmgr_pdev *pdev)
{
	QDF_STATUS status;
	struct pdev_cfr *pdev_cfrobj;

	pdev_cfrobj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							    WLAN_UMAC_COMP_CFR);
	if (!pdev_cfrobj)
		return QDF_STATUS_E_NULL_VALUE;

#if DIRECT_BUF_RX_ENABLE
	status = target_if_register_to_dbr(pdev);
	if (QDF_STATUS_SUCCESS != status) {
		cfr_err("Failed to register with dbr");
		return QDF_STATUS_E_FAILURE;
	}
#endif

	status = target_if_register_tx_completion_event_handler(psoc);
	if (QDF_STATUS_SUCCESS != status) {
		cfr_err("Failed to register with tx event handler");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_cfrobj->cfr_max_sta_count = MAX_CFR_ENABLED_CLIENTS;
	pdev_cfrobj->subbuf_size = STREAMFS_MAX_SUBBUF_8S;
	pdev_cfrobj->num_subbufs = STREAMFS_NUM_SUBBUF_8S;

	return status;
}

QDF_STATUS cfr_dbr_deinit_pdev(struct wlan_objmgr_psoc *psoc,
			       struct wlan_objmgr_pdev *pdev)
{
	QDF_STATUS status;
	struct pdev_cfr *pdev_cfrobj;

	pdev_cfrobj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							    WLAN_UMAC_COMP_CFR);
	if (!pdev_cfrobj)
		return QDF_STATUS_E_NULL_VALUE;

	pdev_cfrobj->cfr_timer_enable = 0;

	status = target_if_unregister_to_dbr(pdev);
	if (QDF_STATUS_SUCCESS != status)
		cfr_err("Failed to register with dbr");

	status = target_if_unregister_tx_completion_event_handler(psoc);
	return status;
}
