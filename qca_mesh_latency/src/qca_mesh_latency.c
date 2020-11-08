/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
#include <qdf_module.h>
#include <qdf_nbuf.h>
#include <qdf_trace.h>
#include <ol_if_athvar.h>
#include "qca_mscs_if.h"

extern ol_ath_soc_softc_t *ol_global_soc[GLOBAL_SOC_SIZE];

/**
 * qca_mesh_latency_update_peer_parameter() - Update peer mesh latency parameter
 * @dest_mac - destination mac address
 * @service_interval - Service interval associated with the tid
 * @burst_size - Burst size corresponding to the tid
 * @priority - priority/tid associated with the peer
 *
 * Return: QDF_STATUS_SUCCESS for successful peer lookup
 */
int qca_mesh_latency_update_peer_parameter(uint8_t *dest_mac,
				    uint32_t service_interval, uint32_t burst_size,
					uint16_t priority, uint8_t add_or_sub)
{
	uint8_t i = 0;
	ol_ath_soc_softc_t *soc = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	ol_txrx_soc_handle soc_txrx_handle = NULL;

	/*
	 * Loop over all the attached soc for peer lookup
	 * with given mac address
	 */
	for (i = 0; i < GLOBAL_SOC_SIZE; i++) {
		if (!ol_global_soc[i])
			continue;

		soc = ol_global_soc[i];
		soc_txrx_handle = wlan_psoc_get_dp_handle(soc->psoc_obj);
		status = cdp_mesh_latency_update_peer_parameter(soc_txrx_handle,
				dest_mac, service_interval, burst_size, priority, add_or_sub);

		if (status == QDF_STATUS_SUCCESS)
			return status;
		/*
		 * no wifi peer exists in this soc with given dest taddress
		 * iterate over next soc
		 */
	}

	/*
	 * No wlan peer is found in any of attached
	 * soc with given mac address
	 */
	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(qca_mesh_latency_update_peer_parameter);
