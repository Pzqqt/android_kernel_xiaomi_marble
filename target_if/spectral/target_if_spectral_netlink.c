/*
 * Copyright (c) 2011,2017-2019 The Linux Foundation. All rights reserved.
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

#include <osdep.h>
#include <wlan_tgt_def_config.h>
#include <hif.h>
#include <hif_hw_version.h>
#include <wmi_unified_api.h>
#include <target_if_spectral.h>
#include <wlan_lmac_if_def.h>
#include <wlan_osif_priv.h>
#include <reg_services_public_struct.h>

void
target_if_spectral_create_samp_msg(struct target_if_spectral *spectral,
				   struct target_if_samp_msg_params *params)
{
	/*
	 * XXX : Non-Rentrant. Will be an issue with dual concurrent
	 *       operation on multi-processor system
	 */

	struct spectral_samp_msg *spec_samp_msg = NULL;

	uint8_t *bin_pwr_data = NULL;
	struct spectral_classifier_params *cp = NULL;
	struct spectral_classifier_params *pcp = NULL;
	struct target_if_spectral_ops *p_sops = NULL;
	uint32_t *binptr_32 = NULL;
	uint16_t *binptr_16 = NULL;
	int idx = 0;
	struct spectral_samp_data *samp_data;
	static int samp_msg_index;
	size_t pwr_count = 0;
	size_t pwr_count_sec80 = 0;
	enum spectral_msg_type msg_type;
	QDF_STATUS ret;

	ret = target_if_get_spectral_msg_type(params->smode, &msg_type);
	if (QDF_IS_STATUS_ERROR(ret))
		return;

	if ((params->smode == SPECTRAL_SCAN_MODE_AGILE) ||
	    is_primaryseg_rx_inprog(spectral)) {
		spec_samp_msg  = (struct spectral_samp_msg *)
		      spectral->nl_cb.get_sbuff(spectral->pdev_obj,
						msg_type,
						SPECTRAL_MSG_BUF_NEW);

		if (!spec_samp_msg)
			return;

		samp_data = &spec_samp_msg->samp_data;
		p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
		bin_pwr_data = params->bin_pwr_data;

		spec_samp_msg->signature = SPECTRAL_SIGNATURE;
		spec_samp_msg->freq = params->freq;
		spec_samp_msg->freq_loading = params->freq_loading;
		samp_data->spectral_mode = params->smode;
		samp_data->spectral_data_len = params->datalen;
		samp_data->spectral_rssi = params->rssi;
		samp_data->ch_width = spectral->ch_width;
		samp_data->spectral_agc_total_gain = params->agc_total_gain;
		samp_data->spectral_gainchange = params->gainchange;

		samp_data->spectral_combined_rssi =
		    (uint8_t)params->rssi;
		samp_data->spectral_upper_rssi = params->upper_rssi;
		samp_data->spectral_lower_rssi = params->lower_rssi;

		qdf_mem_copy(samp_data->spectral_chain_ctl_rssi,
			     params->chain_ctl_rssi,
			     sizeof(params->chain_ctl_rssi));
		qdf_mem_copy(samp_data->spectral_chain_ext_rssi,
			     params->chain_ext_rssi,
			     sizeof(params->chain_ext_rssi));

		samp_data->spectral_bwinfo = params->bwinfo;
		samp_data->spectral_tstamp = params->tstamp;
		samp_data->spectral_max_index = params->max_index;

		/* Classifier in user space needs access to these */
		samp_data->spectral_lower_max_index =
		    params->max_lower_index;
		samp_data->spectral_upper_max_index =
		    params->max_upper_index;
		samp_data->spectral_nb_lower = params->nb_lower;
		samp_data->spectral_nb_upper = params->nb_upper;
		samp_data->spectral_last_tstamp = params->last_tstamp;
		samp_data->spectral_max_mag = params->max_mag;

		/*
		 * Currently, we compute pwr_count considering the size of the
		 * samp_data->bin_pwr array rather than the number of elements
		 * in this array. The reasons are that
		 * SPECTRAL_MESSAGE_COPY_CHAR_ARRAY() where pwr_count will be
		 * used maps directly to OS_MEMCPY() on little endian platforms,
		 * and that samp_data->bin_pwr is an array of u_int8_t elements
		 * due to which the number of elements in the array == the size
		 * of the array. In case FFT bin size is increased from 8 bits
		 * in the future, this code would have to be changed along with
		 * rest of framework on which it depends.
		 */
		pwr_count = qdf_min((size_t)params->pwr_count,
				    sizeof(samp_data->bin_pwr));

		samp_data->bin_pwr_count = pwr_count;
		samp_data->lb_edge_extrabins =
			spectral->lb_edge_extrabins;
		samp_data->rb_edge_extrabins =
			spectral->rb_edge_extrabins;
		samp_data->spectral_combined_rssi = params->rssi;
		samp_data->spectral_max_scale = params->max_exp;

		samp_data->noise_floor = params->noise_floor;

		/* Classifier in user space needs access to these */
		cp = &samp_data->classifier_params;
		pcp = &params->classifier_params;

		qdf_mem_copy(cp, pcp,
			     sizeof(struct spectral_classifier_params));

		if (spectral->fftbin_size_war ==
				SPECTRAL_FFTBIN_SIZE_WAR_4BYTE_TO_1BYTE) {
			binptr_32 = (uint32_t *)bin_pwr_data;
			for (idx = 0; idx < pwr_count; idx++)
				samp_data->bin_pwr[idx] = *(binptr_32++);
		} else if (spectral->fftbin_size_war ==
				SPECTRAL_FFTBIN_SIZE_WAR_2BYTE_TO_1BYTE) {
			binptr_16 = (uint16_t *)bin_pwr_data;
			for (idx = 0; idx < pwr_count; idx++)
				samp_data->bin_pwr[idx] = *(binptr_16++);
		} else {
			SPECTRAL_MESSAGE_COPY_CHAR_ARRAY(
					&samp_data->bin_pwr[0], bin_pwr_data,
					pwr_count);
		}

		p_sops->get_mac_address(spectral, spec_samp_msg->macaddr);
	}

	if (is_secondaryseg_rx_inprog(spectral)) {
		spec_samp_msg  = (struct spectral_samp_msg *)
		      spectral->nl_cb.get_sbuff(spectral->pdev_obj,
						msg_type,
						SPECTRAL_MSG_BUF_SAVED);

		if (!spec_samp_msg) {
			spectral_err("Spectral SAMP message is NULL");
			return;
		}

		samp_data = &spec_samp_msg->samp_data;
		spec_samp_msg->vhtop_ch_freq_seg1 = params->vhtop_ch_freq_seg1;
		spec_samp_msg->vhtop_ch_freq_seg2 = params->vhtop_ch_freq_seg2;
		samp_data->spectral_rssi_sec80 =
		    params->rssi_sec80;
		samp_data->noise_floor_sec80 =
		    params->noise_floor_sec80;
		spec_samp_msg->samp_data.spectral_agc_total_gain_sec80 =
			params->agc_total_gain_sec80;
		spec_samp_msg->samp_data.spectral_gainchange_sec80 =
			params->gainchange_sec80;

		samp_data->spectral_data_len_sec80 =
		    params->datalen_sec80;
		samp_data->spectral_max_index_sec80 =
		    params->max_index_sec80;
		samp_data->spectral_max_mag_sec80 =
		    params->max_mag_sec80;

		/*
		 * Currently, we compute pwr_count_sec80 considering the size of
		 * the samp_data->bin_pwr_sec80 array rather than the number of
		 * elements in this array. The reasons are that
		 * SPECTRAL_MESSAGE_COPY_CHAR_ARRAY() where pwr_count_sec80 will
		 * be used maps directly to OS_MEMCPY() on little endian
		 * platforms, and that samp_data->bin_pwr_sec80 is an array of
		 * u_int8_t elements due to which the number of elements in the
		 * array == the size of the array. In case FFT bin size is
		 * increased from 8 bits in the future, this code would have to
		 * be changed along with rest of framework on which it depends.
		 */
		pwr_count_sec80 = qdf_min((size_t)params->pwr_count_sec80,
					  sizeof(samp_data->bin_pwr_sec80));

		samp_data->bin_pwr_count_sec80 = pwr_count_sec80;

		bin_pwr_data = params->bin_pwr_data_sec80;
		if (spectral->fftbin_size_war ==
				SPECTRAL_FFTBIN_SIZE_WAR_4BYTE_TO_1BYTE) {
			binptr_32 = (uint32_t *)bin_pwr_data;
			for (idx = 0; idx < pwr_count_sec80; idx++)
				samp_data->bin_pwr_sec80[idx] = *(binptr_32++);
		} else if (spectral->fftbin_size_war ==
				SPECTRAL_FFTBIN_SIZE_WAR_2BYTE_TO_1BYTE) {
			binptr_16 = (uint16_t *)bin_pwr_data;
			for (idx = 0; idx < pwr_count_sec80; idx++)
				samp_data->bin_pwr_sec80[idx] = *(binptr_16++);
		} else {
			SPECTRAL_MESSAGE_COPY_CHAR_ARRAY(
					&samp_data->bin_pwr_sec80[0],
					params->bin_pwr_data_sec80,
					pwr_count_sec80);
		}
	}

	if ((spectral->ch_width != CH_WIDTH_160MHZ) ||
	    (params->smode == SPECTRAL_SCAN_MODE_AGILE) ||
	    is_secondaryseg_rx_inprog(spectral)) {
		if (spectral->send_phy_data(spectral->pdev_obj,
					    msg_type) == 0)
			spectral->spectral_sent_msg++;
		samp_msg_index++;
	}

	/* Take care of state transitions for 160MHz/ 80p80 */
	if ((spectral->spectral_gen == SPECTRAL_GEN3) &&
	    (params->smode != SPECTRAL_SCAN_MODE_AGILE))
		target_if_160mhz_delivery_state_change(
				spectral,
				SPECTRAL_DETECTOR_INVALID);
}
