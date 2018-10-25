/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
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

#include "qdf_lock.h"
#include "qdf_status.h"
#include "qdf_module.h"
#include "hif_main.h"

#if defined(HIF_PCI) || defined(HIF_SNOC) || defined(HIF_AHB)
#include "ce_api.h"
#include "ce_internal.h"
#endif

#ifdef WLAN_FEATURE_FASTPATH
/**
 * hif_send_fast() - API to access hif specific function
 * ce_send_fast.
 * @osc: HIF Context
 * @msdu : array of msdus to be sent
 * @num_msdus : number of msdus in an array
 * @transfer_id: transfer id
 * @download_len: download length
 *
 * Return: No. of packets that could be sent
 */
int hif_send_fast(struct hif_opaque_softc *osc, qdf_nbuf_t nbuf,
		  uint32_t transfer_id, uint32_t download_len)
{
	void *ce_tx_hdl = hif_get_ce_handle(osc, CE_HTT_TX_CE);

	return ce_send_fast((struct CE_handle *)ce_tx_hdl, nbuf,
			transfer_id, download_len);
}

qdf_export_symbol(hif_send_fast);

/**
 * hif_ce_fastpath_cb_register() - Register callback for fastpath msg handler
 * @handler: Callback funtcion
 * @context: handle for callback function
 *
 * Return: QDF_STATUS_SUCCESS on success or QDF_STATUS_E_FAILURE
 */
int hif_ce_fastpath_cb_register(struct hif_opaque_softc *hif_ctx,
				fastpath_msg_handler handler,
				void *context)
{
	struct CE_state *ce_state;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	int i;

	if (!scn) {
		HIF_ERROR("%s: scn is NULL", __func__);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	if (!scn->fastpath_mode_on) {
		HIF_WARN("%s: Fastpath mode disabled", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < scn->ce_count; i++) {
		ce_state = scn->ce_id_to_state[i];
		if (ce_state->htt_rx_data) {
			ce_state->fastpath_handler = handler;
			ce_state->context = context;
			ce_state->service = ce_per_engine_service_fast;
		}
	}

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(hif_ce_fastpath_cb_register);
#endif
