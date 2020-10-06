/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
#include "qca_mscs.h"
#include "qca_mscs_if.h"

static int mscs_soc_attached_count;
extern ol_ath_soc_softc_t *ol_global_soc[GLOBAL_SOC_SIZE];

/**
 * qca_mscs_peer_lookup_n_get_priority() - Find MSCS enabled peer and priority
 * @src_mac - src mac address to be used for peer lookup
 * @nbuf - network buffer
 * @priority - priority/tid to be updated
 *
 * Return: QDF_STATUS_SUCCESS for successful peer lookup
 */
int qca_mscs_peer_lookup_n_get_priority(uint8_t *src_mac, struct sk_buff *skb)
{
	uint8_t i = 0;
	ol_ath_soc_softc_t *soc = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	ol_txrx_soc_handle soc_txrx_handle = NULL;
	qdf_nbuf_t nbuf = (qdf_nbuf_t)skb;

	/*
	 * Loop over all the attached soc for peer lookup
	 * with given src mac address
	 */
	for (i = 0; i < GLOBAL_SOC_SIZE; i++) {
		if (!ol_global_soc[i])
			continue;

		soc = ol_global_soc[i];
		soc_txrx_handle = wlan_psoc_get_dp_handle(soc->psoc_obj);
		status = cdp_mscs_peer_lookup_n_get_priority(soc_txrx_handle,
				src_mac, nbuf);
		/*
		 * wifi peer is found with this mac address.there can
		 * be 3 possiblities -
		 * 1. peer has no active MSCS session.
		 * 2. peer has active MSCS session but priority not valid.
		 * 3. peer has active MSCS session and priority is valid.
		 * return the status to ECM classifier.
		 */
		if (status >= QDF_STATUS_SUCCESS)
			return status;
		/*
		 * no wifi peer exists in this soc with given src mac address
		 * iterate over next soc
		 */
	}

	if (i == GLOBAL_SOC_SIZE)
		/*
		 * No wlan peer is found in any of attached
		 * soc with given mac address
		 * return the status to ECM classifier.
		 */
		status = QDF_STATUS_E_FAILURE;

	return status;
}

qdf_export_symbol(qca_mscs_peer_lookup_n_get_priority);

/**
 * qca_mscs_module_init() - Initialize the MSCS module
 * @soc - Pointer to soc getting attached
 *
 * Return: void
 */

void qca_mscs_module_init(ol_ath_soc_softc_t *soc)
{
	/* Check if soc max init count is reached */
	if (mscs_soc_attached_count >= GLOBAL_SOC_SIZE)
		return;

	QDF_TRACE(QDF_MODULE_ID_MSCS, QDF_TRACE_LEVEL_INFO,
			     FL("\n****QCA MSCS Initialization Done**** SoC %pK"), soc);

	mscs_soc_attached_count++;
}

qdf_export_symbol(qca_mscs_module_init);

/**
 * qca_mscs_module_deinit() - De-Initialize MSCS module
 * @soc - Pointer to soc getting detached
 *
 * Return: void
 */
void qca_mscs_module_deinit(ol_ath_soc_softc_t *soc)
{
	if (!soc)
		return;

	/*
	 * All soc are detached by now
	 */
	if (mscs_soc_attached_count < 0)
		return;

	QDF_TRACE(QDF_MODULE_ID_MSCS, QDF_TRACE_LEVEL_INFO,
			FL("\n****QCA MSCS De-Initialization Done**** SoC %pK"), soc);
	mscs_soc_attached_count--;
}

qdf_export_symbol(qca_mscs_module_deinit);
