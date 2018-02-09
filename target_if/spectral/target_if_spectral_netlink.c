/*
 * Copyright (c) 2011,2017-2018 The Linux Foundation. All rights reserved.
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

static void
target_if_spectral_process_noise_pwr_report(
	struct target_if_spectral *spectral,
	const struct spectral_samp_msg *spec_samp_msg)
{
	int i, done;

	qdf_spin_lock(&spectral->noise_pwr_reports_lock);

	if (!spectral->noise_pwr_reports_reqd) {
		qdf_spin_unlock(&spectral->noise_pwr_reports_lock);
		return;
	}

	if (spectral->noise_pwr_reports_recv <
	    spectral->noise_pwr_reports_reqd) {
		spectral->noise_pwr_reports_recv++;

		for (i = 0; i < HOST_MAX_ANTENNA; i++) {
			uint32_t index;

			if (spectral->noise_pwr_chain_ctl[i]) {
				index =
				    spectral->noise_pwr_chain_ctl[i]->
				    rptcount++;
				spectral->noise_pwr_chain_ctl[i]->pwr[index] =
				    spec_samp_msg->samp_data.
				    spectral_chain_ctl_rssi[i];
			}
			if (spectral->noise_pwr_chain_ext[i]) {
				index =
				    spectral->noise_pwr_chain_ext[i]->
				    rptcount++;
				spectral->noise_pwr_chain_ext[i]->pwr[index] =
				    spec_samp_msg->samp_data.
				    spectral_chain_ext_rssi[i];
			}
		}
	}

	done = (spectral->noise_pwr_reports_recv >=
		spectral->noise_pwr_reports_reqd);

	qdf_spin_unlock(&spectral->noise_pwr_reports_lock);

	if (done) {
		qdf_spin_lock(&spectral->spectral_lock);
		target_if_stop_spectral_scan(spectral->pdev_obj);
		spectral->sc_spectral_scan = 0;
		qdf_spin_unlock(&spectral->spectral_lock);
	}
}

/*
 * Function     : spectral_create_samp_msg
 * Description  : create SAMP message and send it host
 * Input        :
 * Output       :
 *
 */

#ifdef SPECTRAL_CLASSIFIER_IN_KERNEL
static void
target_if_spectral_init_interf_list(
	struct spectral_samp_data *data,
	struct target_if_samp_msg_params *params)
{
	if (params->interf_list.count)
		OS_MEMCPY(&data->interf_list,
			  &params->interf_list, sizeof(struct interf_src_rsp));
	else
		data->interf_list.count = 0;
}
#else
static void
target_if_spectral_init_interf_list(
	struct spectral_samp_data *data,
	struct target_if_samp_msg_params *params)
{
	data->interf_list.count = 0;
}
#endif

void
target_if_spectral_create_samp_msg(struct target_if_spectral *spectral,
				   struct target_if_samp_msg_params *params)
{
	/*
	 * XXX : Non-Rentrant. Will be an issue with dual concurrent
	 *       operation on multi-processor system
	 */

	int temp_samp_msg_len = 0;

	struct spectral_samp_msg *spec_samp_msg;

	struct spectral_samp_data *data = NULL;
	uint8_t *bin_pwr_data = NULL;
	struct spectral_classifier_params *cp = NULL;
	struct spectral_classifier_params *pcp = NULL;
	struct target_if_spectral_ops *p_sops = NULL;
	uint32_t *binptr = NULL;
	int idx = 0;

	static int samp_msg_index;

	spec_samp_msg  = (struct spectral_samp_msg *)spectral->nl_cb.get_nbuff(
				spectral->pdev_obj);
	if (!spec_samp_msg)
		return;

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	temp_samp_msg_len = sizeof(struct spectral_samp_msg) -
	    (MAX_NUM_BINS * sizeof(uint8_t));
	temp_samp_msg_len += (params->pwr_count * sizeof(uint8_t));
	if (spectral->ch_width == CH_WIDTH_160MHZ)
		temp_samp_msg_len +=
		    (params->pwr_count_sec80 * sizeof(uint8_t));
	bin_pwr_data = params->bin_pwr_data;

	data = &spec_samp_msg->samp_data;

	spec_samp_msg->signature = SPECTRAL_SIGNATURE;
	spec_samp_msg->freq = params->freq;
	spec_samp_msg->freq_loading = params->freq_loading;
	spec_samp_msg->samp_data.spectral_data_len = params->datalen;
	spec_samp_msg->samp_data.spectral_rssi = params->rssi;
	spec_samp_msg->samp_data.ch_width = spectral->ch_width;

	spec_samp_msg->samp_data.spectral_combined_rssi =
	    (uint8_t)params->rssi;
	spec_samp_msg->samp_data.spectral_upper_rssi = params->upper_rssi;
	spec_samp_msg->samp_data.spectral_lower_rssi = params->lower_rssi;

	OS_MEMCPY(spec_samp_msg->samp_data.spectral_chain_ctl_rssi,
		  params->chain_ctl_rssi, sizeof(params->chain_ctl_rssi));
	OS_MEMCPY(spec_samp_msg->samp_data.spectral_chain_ext_rssi,
		  params->chain_ext_rssi, sizeof(params->chain_ext_rssi));

	spec_samp_msg->samp_data.spectral_bwinfo = params->bwinfo;
	spec_samp_msg->samp_data.spectral_tstamp = params->tstamp;
	spec_samp_msg->samp_data.spectral_max_index = params->max_index;

	/* Classifier in user space needs access to these */
	spec_samp_msg->samp_data.spectral_lower_max_index =
	    params->max_lower_index;
	spec_samp_msg->samp_data.spectral_upper_max_index =
	    params->max_upper_index;
	spec_samp_msg->samp_data.spectral_nb_lower = params->nb_lower;
	spec_samp_msg->samp_data.spectral_nb_upper = params->nb_upper;
	spec_samp_msg->samp_data.spectral_last_tstamp = params->last_tstamp;
	spec_samp_msg->samp_data.spectral_max_mag = params->max_mag;
	spec_samp_msg->samp_data.bin_pwr_count = params->pwr_count;
	spec_samp_msg->samp_data.lb_edge_extrabins =
		spectral->lb_edge_extrabins;
	spec_samp_msg->samp_data.rb_edge_extrabins =
		spectral->rb_edge_extrabins;
	spec_samp_msg->samp_data.spectral_combined_rssi = params->rssi;
	spec_samp_msg->samp_data.spectral_max_scale = params->max_exp;

	/*
	 * This is a dirty hack to get the Windows build pass.
	 * Currently Windows and Linux builds source spectral_data.h
	 * form two different place. The windows version do not
	 * have noise_floor member in it.
	 *
	 * As a temp workaround this variable is set under the
	 * SPECTRAL_USE_NETLINK_SOCKETS as this is called only
	 * under the linux build and this saves the day
	 *
	 * The plan to sync of header files in under the way
	 *
	 */

	spec_samp_msg->samp_data.noise_floor = params->noise_floor;

	/* Classifier in user space needs access to these */
	cp = &spec_samp_msg->samp_data.classifier_params;
	pcp = &params->classifier_params;

	OS_MEMCPY(cp, pcp, sizeof(struct spectral_classifier_params));

	if (spectral->fftbin_size_war) {
		binptr = (uint32_t *)bin_pwr_data;
		for (idx = 0; idx < params->pwr_count; idx++)
			data->bin_pwr[idx] = *(binptr++);
	} else {
		SPECTRAL_MESSAGE_COPY_CHAR_ARRAY(&data->bin_pwr[0],
					 bin_pwr_data, params->pwr_count);
	}

	spec_samp_msg->vhtop_ch_freq_seg1 = params->vhtop_ch_freq_seg1;
	spec_samp_msg->vhtop_ch_freq_seg2 = params->vhtop_ch_freq_seg2;

	if (spectral->ch_width == CH_WIDTH_160MHZ) {
		spec_samp_msg->samp_data.spectral_rssi_sec80 =
		    params->rssi_sec80;
		spec_samp_msg->samp_data.noise_floor_sec80 =
		    params->noise_floor_sec80;

		spec_samp_msg->samp_data.spectral_data_len_sec80 =
		    params->datalen_sec80;
		spec_samp_msg->samp_data.spectral_max_index_sec80 =
		    params->max_index_sec80;
		spec_samp_msg->samp_data.spectral_max_mag_sec80 =
		    params->max_mag_sec80;
		spec_samp_msg->samp_data.bin_pwr_count_sec80 =
		    params->pwr_count_sec80;
		SPECTRAL_MESSAGE_COPY_CHAR_ARRAY(&data->bin_pwr_sec80[0],
						 (params->bin_pwr_data_sec80),
						 params->pwr_count_sec80);

		/*
		 * Note: REVERSE_ORDER is not a known use case for
		 * secondary 80 data at this point.
		 */
	}

	target_if_spectral_init_interf_list(data, params);
	p_sops->get_mac_address(spectral, spec_samp_msg->macaddr);

	if (spectral->sc_spectral_noise_pwr_cal)
		target_if_spectral_process_noise_pwr_report(
			spectral, spec_samp_msg);

	if (spectral->send_phy_data(spectral->pdev_obj) == 0)
		spectral->spectral_sent_msg++;

	samp_msg_index++;
}
