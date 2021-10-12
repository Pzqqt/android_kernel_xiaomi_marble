/*
 * Copyright (c) 2011,2017-2021 The Linux Foundation. All rights reserved.
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

extern int spectral_debug_level;

#ifdef OPTIMIZED_SAMP_MESSAGE
QDF_STATUS
target_if_spectral_fill_samp_msg(struct target_if_spectral *spectral,
				 struct target_if_samp_msg_params *params)
{
	struct spectral_samp_msg *spec_samp_msg;
	struct per_session_det_map *det_map;
	enum spectral_msg_type msg_type;
	QDF_STATUS ret;
	uint16_t dest_det_idx;
	enum spectral_scan_mode spectral_mode;

	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!params) {
		spectral_err_rl("SAMP msg params structure is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (params->hw_detector_id >= SPECTRAL_DETECTOR_ID_MAX) {
		spectral_err_rl("Invalid detector ID");
		return QDF_STATUS_E_FAILURE;
	}

	spectral_mode =
		spectral->rparams.detid_mode_table[params->hw_detector_id];
	if (spectral_mode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("No valid Spectral mode for detector id %u",
				params->hw_detector_id);
		return QDF_STATUS_E_FAILURE;
	}

	ret = target_if_get_spectral_msg_type(spectral_mode,
					      &msg_type);
	if (QDF_IS_STATUS_ERROR(ret)) {
		spectral_err_rl("Invalid spectral msg type");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&spectral->session_det_map_lock);

	if (!spectral->det_map[params->hw_detector_id].det_map_valid) {
		qdf_spin_unlock_bh(&spectral->session_det_map_lock);
		spectral_info("Detector Map not valid for det id = %d",
			      params->hw_detector_id);
		return QDF_STATUS_E_FAILURE;
	}

	det_map = &spectral->det_map[params->hw_detector_id];

	spec_samp_msg = spectral->nl_cb.get_sbuff(spectral->pdev_obj,
						  msg_type,
						  det_map->buf_type);
	if (!spec_samp_msg) {
		qdf_spin_unlock_bh(&spectral->session_det_map_lock);
		spectral_err_rl("Spectral SAMP message is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	for (dest_det_idx = 0; dest_det_idx < det_map->num_dest_det_info;
	     dest_det_idx++) {
		struct per_session_dest_det_info *map_det_info;
		struct spectral_fft_bin_len_adj_swar *swar;
		struct samp_freq_span_info *span_info;
		struct samp_detector_info *detector_info;
		uint8_t dest_detector_id;
		uint8_t span_id;
		struct samp_edge_extra_bin_info *lb_edge_bins;
		struct samp_edge_extra_bin_info *rb_edge_bins;
		uint8_t *bin_pwr_data;
		size_t pwr_count;
		uint16_t num_edge_bins;
		uint16_t start_bin_index;
		uint32_t bytes_copied;

		swar = &spectral->len_adj_swar;

		map_det_info = &det_map->dest_det_info[dest_det_idx];
		span_id = map_det_info->freq_span_id;
		span_info = &spec_samp_msg->freq_span_info[span_id];
		span_info->num_detectors++;

		dest_detector_id = map_det_info->det_id;
		detector_info = &span_info->detector_info[dest_detector_id];
		lb_edge_bins = &detector_info->left_edge_bins;
		rb_edge_bins = &detector_info->right_edge_bins;

		detector_info->start_frequency = map_det_info->start_freq;
		detector_info->end_frequency = map_det_info->end_freq;
		detector_info->start_bin_idx = map_det_info->dest_start_bin_idx;
		detector_info->end_bin_idx = map_det_info->dest_end_bin_idx;
		lb_edge_bins->start_bin_idx =
					map_det_info->lb_extrabins_start_idx;
		lb_edge_bins->num_bins = map_det_info->lb_extrabins_num;
		rb_edge_bins->start_bin_idx =
					map_det_info->rb_extrabins_start_idx;
		rb_edge_bins->num_bins = map_det_info->rb_extrabins_num;
		start_bin_index = detector_info->start_bin_idx;

		detector_info->rssi = params->rssi;

		detector_info->last_raw_timestamp = params->last_raw_timestamp;
		detector_info->reset_delay = params->reset_delay;
		detector_info->raw_timestamp = params->raw_timestamp;
		detector_info->timestamp = params->timestamp;
		detector_info->timestamp_war_offset = spectral->timestamp_war.
				timestamp_war_offset[spectral_mode];

		detector_info->max_magnitude = params->max_mag;
		detector_info->max_index = params->max_index;

		detector_info->noise_floor = params->noise_floor;
		detector_info->agc_total_gain = params->agc_total_gain;
		detector_info->gainchange = params->gainchange;
		detector_info->is_sec80 = map_det_info->is_sec80;
		/* In 165MHz, Pri80 indication to be set for Span ID 0 only */
		if (span_id == SPECTRAL_FREQ_SPAN_ID_0)
			detector_info->pri80ind = params->pri80ind;

		bin_pwr_data = &params->bin_pwr_data
					[map_det_info->src_start_bin_idx];
		pwr_count = detector_info->end_bin_idx -
			    detector_info->start_bin_idx + 1;
		num_edge_bins = lb_edge_bins->num_bins +
				rb_edge_bins->num_bins;

		/* Copy left edge bins */
		if (lb_edge_bins->num_bins > 0) {
			ret = target_if_spectral_copy_fft_bins(
					spectral, bin_pwr_data,
					&spec_samp_msg->bin_pwr[
					lb_edge_bins->start_bin_idx],
					lb_edge_bins->num_bins,
					&bytes_copied);

			if (QDF_IS_STATUS_ERROR(ret)) {
				qdf_spin_unlock_bh(
					&spectral->session_det_map_lock);
				spectral_err_rl("Unable to copy left edge FFT bins");
				return QDF_STATUS_E_FAILURE;
			}

			/* Advance the fft bin pointer in the report */
			bin_pwr_data += bytes_copied;
		}

		/* Copy the in-band and out-band bins */
		ret = target_if_spectral_copy_fft_bins(
				spectral, bin_pwr_data,
				&spec_samp_msg->bin_pwr[start_bin_index],
				pwr_count,
				&bytes_copied);

		if (QDF_IS_STATUS_ERROR(ret)) {
			qdf_spin_unlock_bh(
				&spectral->session_det_map_lock);
			spectral_err_rl("Unable to copy in-band/out-band FFT bins");
			return QDF_STATUS_E_FAILURE;
		}
		/* Advance the fft bin pointer in the report */
		bin_pwr_data += bytes_copied;

		/* Copy right edge bins */
		if (rb_edge_bins->num_bins > 0) {
			ret = target_if_spectral_copy_fft_bins(
					spectral, bin_pwr_data,
					&spec_samp_msg->bin_pwr[
					rb_edge_bins->start_bin_idx],
					rb_edge_bins->num_bins,
					&bytes_copied);

			if (QDF_IS_STATUS_ERROR(ret)) {
				qdf_spin_unlock_bh(
					&spectral->session_det_map_lock);
				spectral_err_rl("Unable to copy right edge FFT bins");
				return QDF_STATUS_E_FAILURE;
			}

			/* Advance the fft bin pointer in the report */
			bin_pwr_data += bytes_copied;
		}

		spec_samp_msg->bin_pwr_count += (pwr_count + num_edge_bins);
	}

	if (det_map->send_to_upper_layers) {
		/* Fill per-report information */
		struct per_session_report_info *rpt_info;
		struct target_if_spectral_ops *p_sops;

		p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

		qdf_spin_lock_bh(&spectral->session_report_info_lock);

		rpt_info = &spectral->report_info[spectral_mode];

		if (!rpt_info->valid) {
			qdf_spin_unlock_bh(&spectral->session_report_info_lock);
			qdf_spin_unlock_bh(&spectral->session_det_map_lock);
			spectral_info("per-session report info is not valid");
			return QDF_STATUS_E_FAILURE;
		}

		spec_samp_msg->signature = SPECTRAL_SIGNATURE;
		p_sops->get_mac_address(spectral, spec_samp_msg->macaddr);
		spec_samp_msg->spectral_mode = spectral_mode;
		spec_samp_msg->target_reset_count =
				spectral->timestamp_war.target_reset_count;
		spec_samp_msg->operating_bw = spectral->nl_cb.
				convert_to_nl_ch_width(rpt_info->operating_bw);
		if (spec_samp_msg->operating_bw < 0) {
			spectral_err_rl("Invalid operating channel width %d",
					rpt_info->operating_bw);
			return QDF_STATUS_E_FAILURE;
		}
		spec_samp_msg->pri20_freq = rpt_info->pri20_freq;
		spec_samp_msg->cfreq1 = rpt_info->cfreq1;
		spec_samp_msg->cfreq2 = rpt_info->cfreq2;
		spec_samp_msg->sscan_cfreq1 = rpt_info->sscan_cfreq1;
		spec_samp_msg->sscan_cfreq2 = rpt_info->sscan_cfreq2;
		spec_samp_msg->sscan_bw = spectral->nl_cb.
				convert_to_nl_ch_width(rpt_info->sscan_bw);
		if (spec_samp_msg->sscan_bw < 0) {
			spectral_err_rl("Invalid sscan channel width %d",
					rpt_info->sscan_bw);
			return QDF_STATUS_E_FAILURE;
		}
		spec_samp_msg->fft_width = FFT_BIN_SIZE_1BYTE;
		spec_samp_msg->num_freq_spans = rpt_info->num_spans;

		qdf_spin_unlock_bh(&spectral->session_report_info_lock);

		spec_samp_msg->spectral_upper_rssi = params->upper_rssi;
		spec_samp_msg->spectral_lower_rssi = params->lower_rssi;
		qdf_mem_copy(spec_samp_msg->spectral_chain_ctl_rssi,
			     params->chain_ctl_rssi,
			     sizeof(params->chain_ctl_rssi));
		qdf_mem_copy(spec_samp_msg->spectral_chain_ext_rssi,
			     params->chain_ext_rssi,
			     sizeof(params->chain_ext_rssi));

		if (spectral_debug_level & DEBUG_SPECTRAL4)
			target_if_dbg_print_samp_msg(spec_samp_msg);

		if (spectral->send_phy_data(spectral->pdev_obj,
					    msg_type) == 0)
			spectral->spectral_sent_msg++;
		if (spectral->spectral_gen == SPECTRAL_GEN3)
			reset_160mhz_delivery_state_machine(spectral,
							    spectral_mode);
	}
	qdf_spin_unlock_bh(&spectral->session_det_map_lock);

	return QDF_STATUS_SUCCESS;
}
#endif /* OPTIMIZED_SAMP_MESSAGE */

#ifndef OPTIMIZED_SAMP_MESSAGE
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
	uint16_t pwr_16;
	int idx = 0;
	struct spectral_samp_data *samp_data;
	static int samp_msg_index;
	size_t pwr_count = 0;
	size_t pwr_count_sec80 = 0;
	size_t pwr_count_5mhz = 0;
	enum spectral_msg_type msg_type;
	QDF_STATUS ret;
	struct spectral_fft_bin_len_adj_swar *swar = &spectral->len_adj_swar;

	ret = target_if_get_spectral_msg_type(params->smode, &msg_type);
	if (QDF_IS_STATUS_ERROR(ret))
		return;

	if (is_primaryseg_rx_inprog(spectral, params->smode)) {
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
		spec_samp_msg->agile_freq1 = params->agile_freq1;
		spec_samp_msg->agile_freq2 = params->agile_freq2;
		spec_samp_msg->freq_loading = params->freq_loading;
		spec_samp_msg->vhtop_ch_freq_seg1 = params->vhtop_ch_freq_seg1;
		spec_samp_msg->vhtop_ch_freq_seg2 = params->vhtop_ch_freq_seg2;
		samp_data->spectral_mode = params->smode;
		samp_data->spectral_data_len = params->datalen;
		samp_data->spectral_rssi = params->rssi;
		samp_data->ch_width =
				spectral->ch_width[SPECTRAL_SCAN_MODE_NORMAL];
		samp_data->agile_ch_width =
				spectral->ch_width[SPECTRAL_SCAN_MODE_AGILE];
		samp_data->spectral_agc_total_gain = params->agc_total_gain;
		samp_data->spectral_gainchange = params->gainchange;
		samp_data->spectral_pri80ind = params->pri80ind;
		samp_data->last_raw_timestamp = params->last_raw_timestamp;
		samp_data->timestamp_war_offset = params->timestamp_war_offset;
		samp_data->raw_timestamp = params->raw_timestamp;
		samp_data->reset_delay = params->reset_delay;
		samp_data->target_reset_count = params->target_reset_count;

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

		/*
		 * To check whether FFT bin values exceed 8 bits, we add a
		 * check before copying values to samp_data->bin_pwr.
		 * If it crosses 8 bits, we cap the values to maximum value
		 * supported by 8 bits ie. 255. This needs to be done as the
		 * destination array in SAMP message is 8 bits. This is a
		 * temporary solution till an array of 16 bits is used for
		 * SAMP message.
		 */
		if (swar->fftbin_size_war ==
				SPECTRAL_FFTBIN_SIZE_WAR_4BYTE_TO_1BYTE) {
			binptr_32 = (uint32_t *)bin_pwr_data;
			for (idx = 0; idx < pwr_count; idx++) {
				/* Read only the first 2 bytes of the DWORD */
				pwr_16 = *((uint16_t *)binptr_32++);
				if (qdf_unlikely(pwr_16 > MAX_FFTBIN_VALUE))
					pwr_16 = MAX_FFTBIN_VALUE;
				samp_data->bin_pwr[idx] = pwr_16;
			}
		} else if (swar->fftbin_size_war ==
				SPECTRAL_FFTBIN_SIZE_WAR_2BYTE_TO_1BYTE) {
			binptr_16 = (uint16_t *)bin_pwr_data;
			for (idx = 0; idx < pwr_count; idx++) {
				pwr_16 = *(binptr_16++);
				if (qdf_unlikely(pwr_16 > MAX_FFTBIN_VALUE))
					pwr_16 = MAX_FFTBIN_VALUE;
				samp_data->bin_pwr[idx] = pwr_16;
			}
		} else {
			SPECTRAL_MESSAGE_COPY_CHAR_ARRAY(
					&samp_data->bin_pwr[0], bin_pwr_data,
					pwr_count);
		}

		p_sops->get_mac_address(spectral, spec_samp_msg->macaddr);
	}

	if (is_secondaryseg_rx_inprog(spectral, params->smode)) {
		spec_samp_msg  = (struct spectral_samp_msg *)
		      spectral->nl_cb.get_sbuff(spectral->pdev_obj,
						msg_type,
						SPECTRAL_MSG_BUF_SAVED);

		if (!spec_samp_msg) {
			spectral_err("Spectral SAMP message is NULL");
			return;
		}

		samp_data = &spec_samp_msg->samp_data;
		samp_data->spectral_rssi_sec80 =
		    params->rssi_sec80;
		samp_data->noise_floor_sec80 =
		    params->noise_floor_sec80;
		spec_samp_msg->samp_data.spectral_agc_total_gain_sec80 =
			params->agc_total_gain_sec80;
		spec_samp_msg->samp_data.spectral_gainchange_sec80 =
			params->gainchange_sec80;
		spec_samp_msg->samp_data.spectral_pri80ind_sec80 =
			params->pri80ind_sec80;

		samp_data->spectral_data_len_sec80 =
		    params->datalen_sec80;
		samp_data->spectral_max_index_sec80 =
		    params->max_index_sec80;
		samp_data->spectral_max_mag_sec80 =
		    params->max_mag_sec80;

		samp_data->raw_timestamp_sec80 = params->raw_timestamp_sec80;

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
		pwr_count_5mhz = qdf_min((size_t)params->pwr_count_5mhz,
					 sizeof(samp_data->bin_pwr_5mhz));

		samp_data->bin_pwr_count_sec80 = pwr_count_sec80;
		samp_data->bin_pwr_count_5mhz = pwr_count_5mhz;

		bin_pwr_data = params->bin_pwr_data_sec80;

		/*
		 * To check whether FFT bin values exceed 8 bits, we add a
		 * check before copying values to samp_data->bin_pwr_sec80.
		 * If it crosses 8 bits, we cap the values to maximum value
		 * supported by 8 bits ie. 255. This needs to be done as the
		 * destination array in SAMP message is 8 bits. This is a
		 * temporary solution till an array of 16 bits is used for
		 * SAMP message.
		 */
		if (swar->fftbin_size_war ==
				SPECTRAL_FFTBIN_SIZE_WAR_4BYTE_TO_1BYTE) {
			binptr_32 = (uint32_t *)bin_pwr_data;
			for (idx = 0; idx < pwr_count_sec80; idx++) {
				/* Read only the first 2 bytes of the DWORD */
				pwr_16 = *((uint16_t *)binptr_32++);
				if (qdf_unlikely(pwr_16 > MAX_FFTBIN_VALUE))
					pwr_16 = MAX_FFTBIN_VALUE;
				samp_data->bin_pwr_sec80[idx] = pwr_16;
			}
		} else if (swar->fftbin_size_war ==
				SPECTRAL_FFTBIN_SIZE_WAR_2BYTE_TO_1BYTE) {
			binptr_16 = (uint16_t *)bin_pwr_data;
			for (idx = 0; idx < pwr_count_sec80; idx++) {
				pwr_16 = *(binptr_16++);
				if (qdf_unlikely(pwr_16 > MAX_FFTBIN_VALUE))
					pwr_16 = MAX_FFTBIN_VALUE;
				samp_data->bin_pwr_sec80[idx] = pwr_16;
			}

			binptr_16 = (uint16_t *)params->bin_pwr_data_5mhz;
			for (idx = 0; idx < pwr_count_5mhz; idx++) {
				pwr_16 = *(binptr_16++);
				if (qdf_unlikely(pwr_16 > MAX_FFTBIN_VALUE))
					pwr_16 = MAX_FFTBIN_VALUE;
				samp_data->bin_pwr_5mhz[idx] = pwr_16;
			}
		} else {
			SPECTRAL_MESSAGE_COPY_CHAR_ARRAY(
					&samp_data->bin_pwr_sec80[0],
					params->bin_pwr_data_sec80,
					pwr_count_sec80);
		}
	}

	if (!is_ch_width_160_or_80p80(spectral->ch_width[params->smode]) ||
	    is_secondaryseg_rx_inprog(spectral, params->smode)) {
		if (spectral->send_phy_data(spectral->pdev_obj,
					    msg_type) == 0)
			spectral->spectral_sent_msg++;
		samp_msg_index++;
	}

	/* Take care of state transitions for 160MHz/ 80p80 */
	if (spectral->spectral_gen == SPECTRAL_GEN3 &&
	    is_ch_width_160_or_80p80(spectral->ch_width[params->smode]) &&
	    spectral->rparams.fragmentation_160[params->smode])
		target_if_160mhz_delivery_state_change(
				spectral, params->smode,
				SPECTRAL_DETECTOR_ID_INVALID);
}
#endif
