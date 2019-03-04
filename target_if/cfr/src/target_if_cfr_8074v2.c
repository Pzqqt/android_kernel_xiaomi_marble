/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
#include <wlan_mlme_dispatcher.h>
#include <init_deinit_lmac.h>
#include <wlan_cfr_utils_api.h>
#include <target_if_cfr_8074v2.h>
#ifdef DIRECT_BUF_RX_ENABLE
#include <target_if_direct_buf_rx_api.h>
#endif

#ifdef DIRECT_BUF_RX_ENABLE
void dump_dma_hdr(struct whal_cfir_dma_hdr *dma_hdr)
{
	cfr_info("Tag: 0x%02x Length: %d udone: %d ctype: %d preamble: %d\n",
		 dma_hdr->tag, dma_hdr->length, dma_hdr->upload_done,
		 dma_hdr->capture_type,	dma_hdr->preamble_type);
	cfr_info("Nss: %d num_chains: %d bw: %d\n", dma_hdr->nss,
		 dma_hdr->num_chains, dma_hdr->upload_pkt_bw);
	cfr_info("peervalid: %d peer_id: %d ppdu_id: 0x%04x\n",
		 dma_hdr->sw_peer_id_valid, dma_hdr->sw_peer_id,
		 dma_hdr->phy_ppdu_id);
}

bool cfr_dbr_event_handler(struct wlan_objmgr_pdev *pdev,
			  struct direct_buf_rx_data *payload)
{
	uint8_t *data = payload->vaddr;
	struct whal_cfir_dma_hdr dma_hdr = {0};

	if ((!pdev) || (!payload)) {
		cfr_err("%s Error!! pdev or payload is null\n", __func__);
		return true;
	}

	qdf_mem_copy(&dma_hdr, &data[0], sizeof(struct whal_cfir_dma_hdr));
	dump_dma_hdr(&dma_hdr);

	return true;
}
#endif

static int
target_if_peer_capture_event(ol_scn_t sc, u_int8_t *data, u_int32_t datalen)
{
	return 0;
}

int
target_if_register_tx_completion_event_handler(struct wlan_objmgr_psoc *psoc)
{
	/* Register completion handler here */
	return 0;
}

int
target_if_unregister_tx_completion_event_handler(struct wlan_objmgr_psoc *psoc)
{
	/* UnRegister completion hoandler here */
	return 0;
}

#ifdef DIRECT_BUF_RX_ENABLE
QDF_STATUS
target_if_register_to_dbr(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_direct_buf_rx_tx_ops *dbr_tx_ops = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	dbr_tx_ops = &psoc->soc_cb.tx_ops.dbr_tx_ops;
	if (dbr_tx_ops->direct_buf_rx_module_register) {
		return dbr_tx_ops->direct_buf_rx_module_register
			(pdev, 1,
			 cfr_dbr_event_handler);
	}

	return QDF_STATUS_SUCCESS;
}
#else
QDF_STATUS
target_if_cfr_register_to_dbr(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif

int cfr_8074v2_init_pdev(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev)
{
	int status;

	status = target_if_register_to_dbr(pdev);
	status = target_if_register_tx_completion_event_handler(psoc);

	return status;
}

int cfr_8074v2_deinit_pdev(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev)
{
	int status;

	status = target_if_unregister_tx_completion_event_handler(psoc);
	return status;
}

#ifdef DIRECT_BUF_RX_ENABLE
struct module_ring_params *
target_if_dbr_get_ring_params(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_direct_buf_rx_tx_ops *dbr_tx_ops = NULL;
	struct module_ring_params *param = {0};

	psoc = wlan_pdev_get_psoc(pdev);
	dbr_tx_ops = &psoc->soc_cb.tx_ops.dbr_tx_ops;

	if(dbr_tx_ops->direct_buf_rx_get_ring_params)
		dbr_tx_ops->direct_buf_rx_get_ring_params(pdev, param, 1);
	return param;
}
#else
struct module_ring_params *
target_if_dbr_get_ring_params(struct wlan_objmgr_pdev *pdev)
{
	return NULL;
}
#endif
