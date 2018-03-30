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
#include <qdf_types.h>
#include <qdf_module.h>
#include <wlan_tgt_def_config.h>
#include <hif.h>
#include <hif_hw_version.h>
#include <wmi_unified_api.h>
#include <target_if_spectral.h>
#include <wlan_lmac_if_def.h>
#include <wlan_osif_priv.h>
#ifdef CONFIG_WIN
#include <osif_rawmode_sim.h>
#endif /*CONFIG_WIN*/
#include <reg_services_public_struct.h>
#include <target_if_direct_buf_rx_api.h>
extern int spectral_debug_level;

#ifdef WLAN_CONV_SPECTRAL_ENABLE

/**
 * target_if_print_buf() - Prints given buffer for given length
 * @pbuf: Pointer to buffer
 * @len: length
 *
 * Prints given buffer for given length
 *
 * Return: void
 */
static void
target_if_print_buf(uint8_t *pbuf, int len)
{
	int i = 0;

	for (i = 0; i < len; i++) {
		spectral_debug("%02X ", pbuf[i]);
		if (i % 32 == 31)
			spectral_debug("\n");
	}
}

int
target_if_spectral_dump_fft(uint8_t *pfft, int fftlen)
{
	int i = 0;

	/*
	 * TODO : Do not delete the following print
	 *        The scripts used to validate Spectral depend on this Print
	 */
	spectral_debug("SPECTRAL : FFT Length is 0x%x (%d)", fftlen, fftlen);

	spectral_debug("fft_data # ");
	for (i = 0; i < fftlen; i++)
		spectral_debug("%d ", pfft[i]);
	spectral_debug("\n");
	return 0;
}

/**
 * target_if_spectral_send_tlv_to_host() - Send the TLV information to Host
 * @spectral: Pointer to target_if spectral object
 * @data: Pointer to the TLV
 * @datalen: data length
 *
 * Send the TLV information to Host
 *
 * Return: Success or failure
 */
int
target_if_spectral_send_tlv_to_host(struct target_if_spectral *spectral,
				    uint8_t *data, uint32_t datalen)
{
	int status = true;
	void *nl_data = spectral->nl_cb.get_nbuff(spectral->pdev_obj);

	if (nl_data) {
		memcpy(nl_data, data, datalen);
		if (spectral->send_phy_data(spectral->pdev_obj) == 0)
			spectral->spectral_sent_msg++;
	} else {
		status = false;
	}
	return status;
}

void
target_if_dbg_print_samp_param(struct target_if_samp_msg_params *p)
{
	spectral_debug("\nSAMP Packet : -------------------- START --------------------");
	spectral_debug("Freq        = %d", p->freq);
	spectral_debug("RSSI        = %d", p->rssi);
	spectral_debug("Bin Count   = %d", p->pwr_count);
	spectral_debug("Timestamp   = %d", p->tstamp);
	spectral_debug("SAMP Packet : -------------------- END -----------------------");
}

void
target_if_dbg_print_samp_msg(struct spectral_samp_msg *ss_msg)
{
	int i = 0;

	struct spectral_samp_data *p = &ss_msg->samp_data;
	struct spectral_classifier_params *pc = &p->classifier_params;
	struct interf_src_rsp *pi = &p->interf_list;

	spectral_dbg_line();
	spectral_debug("Spectral Message");
	spectral_dbg_line();
	spectral_debug("Signature   :   0x%x", ss_msg->signature);
	spectral_debug("Freq        :   %d", ss_msg->freq);
	spectral_debug("Freq load   :   %d", ss_msg->freq_loading);
	spectral_debug("Intfnc type :   %d", ss_msg->int_type);
	spectral_dbg_line();
	spectral_debug("Spectral Data info");
	spectral_dbg_line();
	spectral_debug("data length     :   %d", p->spectral_data_len);
	spectral_debug("rssi            :   %d", p->spectral_rssi);
	spectral_debug("combined rssi   :   %d", p->spectral_combined_rssi);
	spectral_debug("upper rssi      :   %d", p->spectral_upper_rssi);
	spectral_debug("lower rssi      :   %d", p->spectral_lower_rssi);
	spectral_debug("bw info         :   %d", p->spectral_bwinfo);
	spectral_debug("timestamp       :   %d", p->spectral_tstamp);
	spectral_debug("max index       :   %d", p->spectral_max_index);
	spectral_debug("max exp         :   %d", p->spectral_max_exp);
	spectral_debug("max mag         :   %d", p->spectral_max_mag);
	spectral_debug("last timstamp   :   %d", p->spectral_last_tstamp);
	spectral_debug("upper max idx   :   %d", p->spectral_upper_max_index);
	spectral_debug("lower max idx   :   %d", p->spectral_lower_max_index);
	spectral_debug("bin power count :   %d", p->bin_pwr_count);
	spectral_dbg_line();
	spectral_debug("Classifier info");
	spectral_dbg_line();
	spectral_debug("20/40 Mode      :   %d", pc->spectral_20_40_mode);
	spectral_debug("dc index        :   %d", pc->spectral_dc_index);
	spectral_debug("dc in MHz       :   %d", pc->spectral_dc_in_mhz);
	spectral_debug("upper channel   :   %d", pc->upper_chan_in_mhz);
	spectral_debug("lower channel   :   %d", pc->lower_chan_in_mhz);
	spectral_dbg_line();
	spectral_debug("Interference info");
	spectral_dbg_line();
	spectral_debug("inter count     :   %d", pi->count);

	for (i = 0; i < pi->count; i++) {
		spectral_debug("inter type  :   %d",
			       pi->interf[i].interf_type);
		spectral_debug("min freq    :   %d",
			       pi->interf[i].interf_min_freq);
		spectral_debug("max freq    :   %d",
			       pi->interf[i].interf_max_freq);
	}
}

uint32_t
target_if_get_offset_swar_sec80(uint32_t channel_width)
{
	uint32_t offset = 0;

	switch (channel_width) {
	case CH_WIDTH_20MHZ:
		offset = OFFSET_CH_WIDTH_20;
		break;
	case CH_WIDTH_40MHZ:
		offset = OFFSET_CH_WIDTH_40;
		break;
	case CH_WIDTH_80MHZ:
		offset = OFFSET_CH_WIDTH_80;
		break;
	case CH_WIDTH_160MHZ:
		offset = OFFSET_CH_WIDTH_160;
		break;
	default:
		offset = OFFSET_CH_WIDTH_80;
		break;
	}
	return offset;
}

/**
 * target_if_dump_summary_report_gen2() - Dump Spectral Summary Report for gen2
 * @ptlv: Pointer to Spectral Phyerr TLV
 * @tlvlen: length
 * @is_160_format: Indicates whether information provided by HW is in altered
 *                 format for 802.11ac 160/80+80 MHz support (QCA9984 onwards)
 *
 * Dump Spectral Summary Report for gen2
 *
 * Return: Success/Failure
 */
static int
target_if_dump_summary_report_gen2(struct spectral_phyerr_tlv_gen2 *ptlv,
				   int tlvlen, bool is_160_format)
{
	/*
	 * For simplicity, everything is defined as uint32_t (except one).
	 * Proper code will later use the right sizes.
	 */

	/*
	 * For easy comparision between MDK team and OS team, the MDK script
	 * variable names have been used
	 */

	uint32_t agc_mb_gain;
	uint32_t sscan_gidx;
	uint32_t agc_total_gain;
	uint32_t recent_rfsat;
	uint32_t ob_flag;
	uint32_t nb_mask;
	uint32_t peak_mag;
	int16_t peak_inx;

	uint32_t ss_summary_A = 0;
	uint32_t ss_summary_B = 0;
	uint32_t ss_summary_C = 0;
	uint32_t ss_summary_D = 0;
	uint32_t ss_summary_E = 0;
	struct spectral_phyerr_hdr_gen2 *phdr =
	    (struct spectral_phyerr_hdr_gen2 *)(
		(uint8_t *)ptlv +
		sizeof(struct spectral_phyerr_tlv_gen2));

	spectral_debug("SPECTRAL : SPECTRAL SUMMARY REPORT");

	if (is_160_format) {
		if (tlvlen != 20) {
			spectral_err("Unexpected TLV length %d for Spectral Summary Report! Hexdump follows",
				     tlvlen);
			target_if_print_buf((uint8_t *)ptlv, tlvlen + 4);
			return -EPERM;
		}

		/* Doing copy as the contents may not be aligned */
		qdf_mem_copy(&ss_summary_A, (uint8_t *)phdr, sizeof(int));
		qdf_mem_copy(&ss_summary_B,
			     (uint8_t *)((uint8_t *)phdr + sizeof(int)),
			     sizeof(int));
		qdf_mem_copy(&ss_summary_C,
			     (uint8_t *)((uint8_t *)phdr + 2 * sizeof(int)),
			     sizeof(int));
		qdf_mem_copy(&ss_summary_D,
			     (uint8_t *)((uint8_t *)phdr + 3 * sizeof(int)),
			     sizeof(int));
		qdf_mem_copy(&ss_summary_E,
			     (uint8_t *)((uint8_t *)phdr + 4 * sizeof(int)),
			     sizeof(int));

		/*
		 * The following is adapted from MDK scripts for
		 * easier comparability
		 */

		recent_rfsat = ((ss_summary_A >> 8) & 0x1);
		sscan_gidx = (ss_summary_A & 0xff);
		spectral_debug("sscan_gidx=%d, is_recent_rfsat=%d",
			       sscan_gidx, recent_rfsat);

		/* First segment */
		agc_mb_gain = ((ss_summary_B >> 10) & 0x7f);
		agc_total_gain = (ss_summary_B & 0x3ff);
		nb_mask = ((ss_summary_C >> 22) & 0xff);
		ob_flag = ((ss_summary_B >> 17) & 0x1);
		peak_inx = (ss_summary_C & 0xfff);
		if (peak_inx > 2047)
			peak_inx = peak_inx - 4096;
		peak_mag = ((ss_summary_C >> 12) & 0x3ff);

		spectral_debug("agc_total_gain_segid0 = 0x%.2x, agc_mb_gain_segid0=%d",
			       agc_total_gain, agc_mb_gain);
		spectral_debug("nb_mask_segid0 = 0x%.2x, ob_flag_segid0=%d, peak_index_segid0=%d, peak_mag_segid0=%d",
			       nb_mask, ob_flag, peak_inx, peak_mag);

		/* Second segment */
		agc_mb_gain = ((ss_summary_D >> 10) & 0x7f);
		agc_total_gain = (ss_summary_D & 0x3ff);
		nb_mask = ((ss_summary_E >> 22) & 0xff);
		ob_flag = ((ss_summary_D >> 17) & 0x1);
		peak_inx = (ss_summary_E & 0xfff);
		if (peak_inx > 2047)
			peak_inx = peak_inx - 4096;
		peak_mag = ((ss_summary_E >> 12) & 0x3ff);

		spectral_debug("agc_total_gain_segid1 = 0x%.2x, agc_mb_gain_segid1=%d",
			       agc_total_gain, agc_mb_gain);
		spectral_debug("nb_mask_segid1 = 0x%.2x, ob_flag_segid1=%d, peak_index_segid1=%d, peak_mag_segid1=%d",
			       nb_mask, ob_flag, peak_inx, peak_mag);
	} else {
		if (tlvlen != 8) {
			spectral_err("Unexpected TLV length %d for Spectral Summary Report! Hexdump follows",
				     tlvlen);
			target_if_print_buf((uint8_t *)ptlv, tlvlen + 4);
			return -EPERM;
		}

		/* Doing copy as the contents may not be aligned */
		qdf_mem_copy(&ss_summary_A, (uint8_t *)phdr, sizeof(int));
		qdf_mem_copy(&ss_summary_B,
			     (uint8_t *)((uint8_t *)phdr + sizeof(int)),
			     sizeof(int));

		nb_mask = ((ss_summary_B >> 22) & 0xff);
		ob_flag = ((ss_summary_B >> 30) & 0x1);
		peak_inx = (ss_summary_B & 0xfff);

		if (peak_inx > 2047)
			peak_inx = peak_inx - 4096;

		peak_mag = ((ss_summary_B >> 12) & 0x3ff);
		agc_mb_gain = ((ss_summary_A >> 24) & 0x7f);
		agc_total_gain = (ss_summary_A & 0x3ff);
		sscan_gidx = ((ss_summary_A >> 16) & 0xff);
		recent_rfsat = ((ss_summary_B >> 31) & 0x1);

		spectral_debug("nb_mask = 0x%.2x, ob_flag=%d, peak_index=%d, peak_mag=%d, agc_mb_gain=%d, agc_total_gain=%d, sscan_gidx=%d, recent_rfsat=%d",
			       nb_mask, ob_flag, peak_inx, peak_mag,
			       agc_mb_gain, agc_total_gain, sscan_gidx,
			       recent_rfsat);
	}

	return 0;
}

/**
 * target_if_process_sfft_report_gen2() - Process Search FFT Report
 * @ptlv: Pointer to Spectral Phyerr TLV
 * @tlvlen: length
 * @p_fft_info: Pointer to search fft info
 *
 * Dump Spectral Summary Report for gen2
 *
 * Return: Success/Failure
 */
static int
target_if_process_sfft_report_gen2(
	struct spectral_phyerr_tlv_gen2 *ptlv,
	int tlvlen,
	struct spectral_search_fft_info_gen2 *p_fft_info)
{
	/*
	 * For simplicity, everything is defined as uint32_t (except one).
	 * Proper code will later use the right sizes.
	 */
	/*
	 * For easy comparision between MDK team and OS team, the MDK script
	 * variable names have been used
	 */
	uint32_t relpwr_db;
	uint32_t num_str_bins_ib;
	uint32_t base_pwr;
	uint32_t total_gain_info;

	uint32_t fft_chn_idx;
	int16_t peak_inx;
	uint32_t avgpwr_db;
	uint32_t peak_mag;

	uint32_t fft_summary_A = 0;
	uint32_t fft_summary_B = 0;
	uint8_t *tmp = (uint8_t *)ptlv;
	struct spectral_phyerr_hdr_gen2 *phdr =
	    (struct spectral_phyerr_hdr_gen2 *)(
		tmp +
		sizeof(struct spectral_phyerr_tlv_gen2));

	/* Relook this */
	if (tlvlen < 8) {
		spectral_err("Unexpected TLV length %d for Spectral Summary Report! Hexdump follows",
			     tlvlen);
		target_if_print_buf((uint8_t *)ptlv, tlvlen + 4);
		return -EPERM;
	}

	/* Doing copy as the contents may not be aligned */
	qdf_mem_copy(&fft_summary_A, (uint8_t *)phdr, sizeof(int));
	qdf_mem_copy(&fft_summary_B,
		     (uint8_t *)((uint8_t *)phdr + sizeof(int)),
		     sizeof(int));

	relpwr_db = ((fft_summary_B >> 26) & 0x3f);
	num_str_bins_ib = fft_summary_B & 0xff;
	base_pwr = ((fft_summary_A >> 14) & 0x1ff);
	total_gain_info = ((fft_summary_A >> 23) & 0x1ff);

	fft_chn_idx = ((fft_summary_A >> 12) & 0x3);
	peak_inx = fft_summary_A & 0xfff;

	if (peak_inx > 2047)
		peak_inx = peak_inx - 4096;

	avgpwr_db = ((fft_summary_B >> 18) & 0xff);
	peak_mag = ((fft_summary_B >> 8) & 0x3ff);

	/* Populate the Search FFT Info */
	if (p_fft_info) {
		p_fft_info->relpwr_db = relpwr_db;
		p_fft_info->num_str_bins_ib = num_str_bins_ib;
		p_fft_info->base_pwr = base_pwr;
		p_fft_info->total_gain_info = total_gain_info;
		p_fft_info->fft_chn_idx = fft_chn_idx;
		p_fft_info->peak_inx = peak_inx;
		p_fft_info->avgpwr_db = avgpwr_db;
		p_fft_info->peak_mag = peak_mag;
	}

	return 0;
}

/**
 * target_if_dump_adc_report_gen2() - Dump ADC Reports for gen2
 * @ptlv: Pointer to Spectral Phyerr TLV
 * @tlvlen: length
 *
 * Dump ADC Reports for gen2
 *
 * Return: Success/Failure
 */
static int
target_if_dump_adc_report_gen2(
	struct spectral_phyerr_tlv_gen2 *ptlv, int tlvlen)
{
	int i;
	uint32_t *pdata;
	uint32_t data;

	/*
	 * For simplicity, everything is defined as uint32_t (except one).
	 * Proper code will later use the right sizes.
	 */
	uint32_t samp_fmt;
	uint32_t chn_idx;
	uint32_t recent_rfsat;
	uint32_t agc_mb_gain;
	uint32_t agc_total_gain;

	uint32_t adc_summary = 0;

	uint8_t *ptmp = (uint8_t *)ptlv;

	spectral_debug("SPECTRAL : ADC REPORT");

	/* Relook this */
	if (tlvlen < 4) {
		spectral_err("Unexpected TLV length %d for ADC Report! Hexdump follows",
			     tlvlen);
		target_if_print_buf((uint8_t *)ptlv, tlvlen + 4);
		return -EPERM;
	}

	qdf_mem_copy(&adc_summary, (uint8_t *)(ptlv + 4), sizeof(int));

	samp_fmt = ((adc_summary >> 28) & 0x1);
	chn_idx = ((adc_summary >> 24) & 0x3);
	recent_rfsat = ((adc_summary >> 23) & 0x1);
	agc_mb_gain = ((adc_summary >> 16) & 0x7f);
	agc_total_gain = adc_summary & 0x3ff;

	spectral_debug("samp_fmt= %u, chn_idx= %u, recent_rfsat= %u, agc_mb_gain=%u agc_total_gain=%u",
		       samp_fmt, chn_idx, recent_rfsat, agc_mb_gain,
		       agc_total_gain);

	for (i = 0; i < (tlvlen / 4); i++) {
		pdata = (uint32_t *)(ptmp + 4 + i * 4);
		data = *pdata;

		/* Interpreting capture format 1 */
		if (1) {
			uint8_t i1;
			uint8_t q1;
			uint8_t i2;
			uint8_t q2;
			int8_t si1;
			int8_t sq1;
			int8_t si2;
			int8_t sq2;

			i1 = data & 0xff;
			q1 = (data >> 8) & 0xff;
			i2 = (data >> 16) & 0xff;
			q2 = (data >> 24) & 0xff;

			if (i1 > 127)
				si1 = i1 - 256;
			else
				si1 = i1;

			if (q1 > 127)
				sq1 = q1 - 256;
			else
				sq1 = q1;

			if (i2 > 127)
				si2 = i2 - 256;
			else
				si2 = i2;

			if (q2 > 127)
				sq2 = q2 - 256;
			else
				sq2 = q2;

			spectral_debug("SPECTRAL ADC : Interpreting capture format 1");
			spectral_debug("adc_data_format_1 # %d %d %d",
				       2 * i, si1, sq1);
			spectral_debug("adc_data_format_1 # %d %d %d",
				       2 * i + 1, si2, sq2);
		}

		/* Interpreting capture format 0 */
		if (1) {
			uint16_t i1;
			uint16_t q1;
			int16_t si1;
			int16_t sq1;

			i1 = data & 0xffff;
			q1 = (data >> 16) & 0xffff;
			if (i1 > 32767)
				si1 = i1 - 65536;
			else
				si1 = i1;

			if (q1 > 32767)
				sq1 = q1 - 65536;
			else
				sq1 = q1;
			spectral_debug("SPECTRAL ADC : Interpreting capture format 0");
			spectral_debug("adc_data_format_2 # %d %d %d",
				       i, si1, sq1);
		}
	}

	spectral_debug("\n");

	return 0;
}

/**
 * target_if_dump_sfft_report_gen2() - Process Search FFT Report for gen2
 * @ptlv: Pointer to Spectral Phyerr TLV
 * @tlvlen: length
 * @is_160_format: Indicates 160 format
 *
 * Process Search FFT Report for gen2
 *
 * Return: Success/Failure
 */
static int
target_if_dump_sfft_report_gen2(struct spectral_phyerr_tlv_gen2 *ptlv,
				int tlvlen, bool is_160_format)
{
	int i;
	uint32_t fft_mag;

	/*
	 * For simplicity, everything is defined as uint32_t (except one).
	 * Proper code will later use the right sizes.
	 */
	/*
	 * For easy comparision between MDK team and OS team, the MDK script
	 * variable names have been used
	 */
	uint32_t relpwr_db;
	uint32_t num_str_bins_ib;
	uint32_t base_pwr;
	uint32_t total_gain_info;

	uint32_t fft_chn_idx;
	int16_t peak_inx;
	uint32_t avgpwr_db;
	uint32_t peak_mag;
	uint8_t segid;

	uint32_t fft_summary_A = 0;
	uint32_t fft_summary_B = 0;
	uint32_t fft_summary_C = 0;
	uint8_t *tmp = (uint8_t *)ptlv;
	struct spectral_phyerr_hdr_gen2 *phdr =
	    (struct spectral_phyerr_hdr_gen2 *)(
		tmp +
		sizeof(struct spectral_phyerr_tlv_gen2));
	uint32_t segid_skiplen = 0;

	if (is_160_format)
		segid_skiplen = sizeof(SPECTRAL_SEGID_INFO);

	spectral_debug("SPECTRAL : SEARCH FFT REPORT");

	/* Relook this */
	if (tlvlen < (8 + segid_skiplen)) {
		spectral_err("Unexpected TLV length %d for Spectral Summary Report! Hexdump follows",
			     tlvlen);
		target_if_print_buf((uint8_t *)ptlv, tlvlen + 4);
		return -EPERM;
	}

	/* Doing copy as the contents may not be aligned */
	qdf_mem_copy(&fft_summary_A, (uint8_t *)phdr, sizeof(int));
	qdf_mem_copy(&fft_summary_B,
		     (uint8_t *)((uint8_t *)phdr + sizeof(int)),
		     sizeof(int));
	if (is_160_format)
		qdf_mem_copy(&fft_summary_C,
			     (uint8_t *)((uint8_t *)phdr + 2 * sizeof(int)),
			     sizeof(int));

	relpwr_db = ((fft_summary_B >> 26) & 0x3f);
	num_str_bins_ib = fft_summary_B & 0xff;
	base_pwr = ((fft_summary_A >> 14) & 0x1ff);
	total_gain_info = ((fft_summary_A >> 23) & 0x1ff);

	fft_chn_idx = ((fft_summary_A >> 12) & 0x3);
	peak_inx = fft_summary_A & 0xfff;

	if (peak_inx > 2047)
		peak_inx = peak_inx - 4096;

	avgpwr_db = ((fft_summary_B >> 18) & 0xff);
	peak_mag = ((fft_summary_B >> 8) & 0x3ff);

	spectral_debug("Header A = 0x%x Header B = 0x%x",
		       phdr->hdr_a, phdr->hdr_b);
	spectral_debug("Base Power= 0x%x, Total Gain= %d, relpwr_db=%d, num_str_bins_ib=%d fft_chn_idx=%d peak_inx=%d avgpwr_db=%d peak_mag=%d",
		       base_pwr, total_gain_info, relpwr_db, num_str_bins_ib,
		       fft_chn_idx, peak_inx, avgpwr_db, peak_mag);
	if (is_160_format) {
		segid = fft_summary_C & 0x1;
		spectral_debug("Segment ID: %hhu", segid);
	}

	spectral_debug("FFT bins:");
	for (i = 0; i < (tlvlen - 8 - segid_skiplen); i++) {
		fft_mag = ((uint8_t *)ptlv)[12 + segid_skiplen + i];
		spectral_debug("%d %d, ", i, fft_mag);
	}

	spectral_debug("\n");

	return 0;
}

#ifdef SPECTRAL_DEBUG_SAMP_MSG
/**
 * target_if_spectral_log_SAMP_param() - Log SAMP parameters
 * @params: Reference to target_if_samp_msg_params
 *
 * API to log spectral SAMP message parameters
 *
 * Return: None
 */
static void
target_if_spectral_log_SAMP_param(struct target_if_samp_msg_params *params)
{
	target_if_dbg_print_samp_param(params);
}

#else
static void
target_if_spectral_log_SAMP_param(struct target_if_samp_msg_params *params)
{
}
#endif

int
target_if_process_phyerr_gen2(struct target_if_spectral *spectral,
			      uint8_t *data,
			      uint32_t datalen,
			      struct target_if_spectral_rfqual_info *p_rfqual,
			      struct target_if_spectral_chan_info *p_chaninfo,
			      uint64_t tsf64,
			      struct target_if_spectral_acs_stats *acs_stats)
{
	/*
	 * XXX : The classifier do not use all the members of the SAMP
	 *       message data format.
	 *       The classifier only depends upon the following parameters
	 *
	 *          1. Frequency (freq, msg->freq)
	 *          2. Spectral RSSI (spectral_rssi,
	 *          msg->samp_data.spectral_rssi)
	 *          3. Bin Power Count (bin_pwr_count,
	 *          msg->samp_data.bin_pwr_count)
	 *          4. Bin Power values (bin_pwr, msg->samp_data.bin_pwr[0]
	 *          5. Spectral Timestamp (spectral_tstamp,
	 *          msg->samp_data.spectral_tstamp)
	 *          6. MAC Address (macaddr, msg->macaddr)
	 *
	 *       This function prepares the params structure and populates it
	 *       with
	 *       relevant values, this is in turn passed to
	 *       spectral_create_samp_msg()
	 *       to prepare fully formatted Spectral SAMP message
	 *
	 *       XXX : Need to verify
	 *          1. Order of FFT bin values
	 *
	 */

	struct target_if_samp_msg_params params;
	struct spectral_search_fft_info_gen2 search_fft_info;
	struct spectral_search_fft_info_gen2 *p_sfft = &search_fft_info;
	struct spectral_search_fft_info_gen2 search_fft_info_sec80;
	struct spectral_search_fft_info_gen2 *p_sfft_sec80 =
		&search_fft_info_sec80;
	uint32_t segid_skiplen = 0;

	int8_t rssi_up = 0;
	int8_t rssi_low = 0;

	int8_t chn_idx_highest_enabled = 0;
	int8_t chn_idx_lowest_enabled = 0;

	uint8_t control_rssi = 0;
	uint8_t extension_rssi = 0;
	uint8_t combined_rssi = 0;

	uint32_t tstamp = 0;

	struct target_if_spectral_ops *p_sops =
		GET_TARGET_IF_SPECTRAL_OPS(spectral);

	struct spectral_phyerr_tlv_gen2 *ptlv =
		(struct spectral_phyerr_tlv_gen2 *)data;
	struct spectral_phyerr_tlv_gen2 *ptlv_sec80 = NULL;
	struct spectral_phyerr_fft_gen2 *pfft = NULL;
	struct spectral_phyerr_fft_gen2 *pfft_sec80 = NULL;

	uint8_t segid = 0;
	uint8_t segid_sec80 = 0;

	if (spectral->is_160_format)
		segid_skiplen = sizeof(SPECTRAL_SEGID_INFO);

	pfft = (struct spectral_phyerr_fft_gen2 *)(
			data +
			sizeof(struct spectral_phyerr_tlv_gen2) +
			sizeof(struct spectral_phyerr_hdr_gen2) +
			segid_skiplen);

	/*
	 * XXX Extend SPECTRAL_DPRINTK() to use spectral_debug_level,
	 * and use this facility inside spectral_dump_phyerr_data()
	 * and supporting functions.
	 */
	if (spectral_debug_level & DEBUG_SPECTRAL2)
		target_if_spectral_dump_phyerr_data_gen2(
					data, datalen,
					spectral->is_160_format);

	if (spectral_debug_level & DEBUG_SPECTRAL4) {
		target_if_spectral_dump_phyerr_data_gen2(
					data, datalen,
					spectral->is_160_format);
		spectral_debug_level = DEBUG_SPECTRAL;
	}

	if (ptlv->signature != SPECTRAL_PHYERR_SIGNATURE_GEN2) {
		/*
		 * EV# 118023: We tentatively disable the below print
		 * and provide stats instead.
		 */
		spectral->diag_stats.spectral_mismatch++;
		return -EPERM;
	}

	OS_MEMZERO(&params, sizeof(params));

	if (ptlv->tag == TLV_TAG_SEARCH_FFT_REPORT_GEN2) {
		if (spectral->is_160_format) {
			segid = *((SPECTRAL_SEGID_INFO *)(
				  (uint8_t *)ptlv +
				  sizeof(struct spectral_phyerr_tlv_gen2) +
				  sizeof(struct spectral_phyerr_hdr_gen2)));

			if (segid != 0) {
				struct spectral_diag_stats *p_diag_stats =
					&spectral->diag_stats;
				p_diag_stats->spectral_vhtseg1id_mismatch++;
				return -EPERM;
			}
		}

		target_if_process_sfft_report_gen2(ptlv, ptlv->length,
						   &search_fft_info);

		tstamp = p_sops->get_tsf64(spectral) & SPECTRAL_TSMASK;

		combined_rssi = p_rfqual->rssi_comb;

		if (spectral->upper_is_control)
			rssi_up = control_rssi;
		else
			rssi_up = extension_rssi;

		if (spectral->lower_is_control)
			rssi_low = control_rssi;
		else
			rssi_low = extension_rssi;

		params.rssi = p_rfqual->rssi_comb;
		params.lower_rssi = rssi_low;
		params.upper_rssi = rssi_up;

		if (spectral->sc_spectral_noise_pwr_cal) {
			params.chain_ctl_rssi[0] =
			    p_rfqual->pc_rssi_info[0].rssi_pri20;
			params.chain_ctl_rssi[1] =
			    p_rfqual->pc_rssi_info[1].rssi_pri20;
			params.chain_ctl_rssi[2] =
			    p_rfqual->pc_rssi_info[2].rssi_pri20;
			params.chain_ext_rssi[0] =
			    p_rfqual->pc_rssi_info[0].rssi_sec20;
			params.chain_ext_rssi[1] =
			    p_rfqual->pc_rssi_info[1].rssi_sec20;
			params.chain_ext_rssi[2] =
			    p_rfqual->pc_rssi_info[2].rssi_sec20;
		}

		/*
		 * XXX : This actually depends on the programmed chain mask
		 *       This value decides the per-chain enable mask to select
		 *       the input ADC for search FTT.
		 *       For modes upto VHT80, if more than one chain is
		 *       enabled, the max valid chain
		 *       is used. LSB corresponds to chain zero.
		 *       For VHT80_80 and VHT160, the lowest enabled chain is
		 *       used for primary
		 *       detection and highest enabled chain is used for
		 *       secondary detection.
		 *
		 * XXX : The current algorithm do not use these control and
		 *       extension channel
		 *       Instead, it just relies on the combined RSSI values
		 *       only.
		 *       For fool-proof detection algorithm, we should take
		 *       these RSSI values in to account.
		 *       This is marked for future enhancements.
		 */
		chn_idx_highest_enabled =
		    ((spectral->params.ss_chn_mask & 0x8) ? 3 :
		     (spectral->params.ss_chn_mask & 0x4) ? 2 :
		     (spectral->params.ss_chn_mask & 0x2) ? 1 : 0);
		chn_idx_lowest_enabled =
		    ((spectral->params.ss_chn_mask & 0x1) ? 0 :
		     (spectral->params.ss_chn_mask & 0x2) ? 1 :
		     (spectral->params.ss_chn_mask & 0x4) ? 2 : 3);
		control_rssi = (uint8_t)
		    p_rfqual->pc_rssi_info[chn_idx_highest_enabled].rssi_pri20;
		extension_rssi = (uint8_t)
		    p_rfqual->pc_rssi_info[chn_idx_highest_enabled].rssi_sec20;

		params.bwinfo = 0;
		params.tstamp = 0;
		params.max_mag = p_sfft->peak_mag;

		params.max_index = p_sfft->peak_inx;
		params.max_exp = 0;
		params.peak = 0;
		params.bin_pwr_data = (uint8_t *)pfft;
		params.freq = p_sops->get_current_channel(spectral);
		params.freq_loading = 0;

		params.interf_list.count = 0;
		params.max_lower_index = 0;
		params.max_upper_index = 0;
		params.nb_lower = 0;
		params.nb_upper = 0;
		/*
		 * For modes upto VHT80, the noise floor is populated with the
		 * one corresponding
		 * to the highest enabled antenna chain
		 */
		params.noise_floor =
		    p_rfqual->noise_floor[chn_idx_highest_enabled];
		params.datalen = ptlv->length;
		params.pwr_count = ptlv->length -
		    sizeof(struct spectral_phyerr_hdr_gen2) - segid_skiplen;
		params.tstamp = (tsf64 & SPECTRAL_TSMASK);

		acs_stats->ctrl_nf = params.noise_floor;
		acs_stats->ext_nf = params.noise_floor;
		acs_stats->nfc_ctl_rssi = control_rssi;
		acs_stats->nfc_ext_rssi = extension_rssi;

		if (spectral->is_160_format &&
		    spectral->ch_width == CH_WIDTH_160MHZ) {
			/*
			 * We expect to see one more Search FFT report, and it
			 * should be equal in size to the current one.
			 */
			if (datalen < (
				2 * (
				sizeof(struct spectral_phyerr_tlv_gen2) +
				ptlv->length))) {
				struct spectral_diag_stats *p_diag_stats =
					&spectral->diag_stats;
				p_diag_stats->spectral_sec80_sfft_insufflen++;
				return -EPERM;
			}

			ptlv_sec80 = (struct spectral_phyerr_tlv_gen2 *)(
				      data +
				      sizeof(struct spectral_phyerr_tlv_gen2) +
				      ptlv->length);

			if (ptlv_sec80->signature !=
			    SPECTRAL_PHYERR_SIGNATURE_GEN2) {
				spectral->diag_stats.spectral_mismatch++;
				return -EPERM;
			}

			if (ptlv_sec80->tag != TLV_TAG_SEARCH_FFT_REPORT_GEN2) {
				spectral->diag_stats.spectral_no_sec80_sfft++;
				return -EPERM;
			}

			segid_sec80 = *((SPECTRAL_SEGID_INFO *)(
				(uint8_t *)ptlv_sec80 +
				sizeof(struct spectral_phyerr_tlv_gen2) +
				sizeof(struct spectral_phyerr_hdr_gen2)));

			if (segid_sec80 != 1) {
				struct spectral_diag_stats *p_diag_stats =
					&spectral->diag_stats;
				p_diag_stats->spectral_vhtseg2id_mismatch++;
				return -EPERM;
			}

			params.vhtop_ch_freq_seg1 = p_chaninfo->center_freq1;
			params.vhtop_ch_freq_seg2 = p_chaninfo->center_freq2;

			target_if_process_sfft_report_gen2(
				ptlv_sec80,
				ptlv_sec80->length,
				&search_fft_info_sec80);

			pfft_sec80 = (struct spectral_phyerr_fft_gen2 *)(
				((uint8_t *)ptlv_sec80) +
				sizeof(struct spectral_phyerr_tlv_gen2) +
				sizeof(struct spectral_phyerr_hdr_gen2) +
				segid_skiplen);

			/* XXX: Confirm. TBD at SoD. */
			params.rssi_sec80 = p_rfqual->rssi_comb;
			if (spectral->is_sec80_rssi_war_required)
				params.rssi_sec80 =
				    target_if_get_combrssi_sec80_seg_gen2
				    (spectral, &search_fft_info_sec80);
			/* XXX: Determine dynamically. TBD at SoD. */
			/*
			 * For VHT80_80/VHT160, the noise floor for primary
			 * 80MHz segment is populated with the
			 * lowest enabled antenna chain and the noise floor for
			 * secondary 80MHz segment is populated
			 * with the highest enabled antenna chain
			 */
			params.noise_floor_sec80 =
			    p_rfqual->noise_floor[chn_idx_highest_enabled];
			params.noise_floor =
			    p_rfqual->noise_floor[chn_idx_lowest_enabled];

			params.max_mag_sec80 = p_sfft_sec80->peak_mag;
			params.max_index_sec80 = p_sfft_sec80->peak_inx;
			/* XXX Does this definition of datalen *still hold? */
			params.datalen_sec80 = ptlv_sec80->length;
			params.pwr_count_sec80 =
			    ptlv_sec80->length -
			    sizeof(struct spectral_phyerr_hdr_gen2) -
			    segid_skiplen;
			params.bin_pwr_data_sec80 = (uint8_t *)pfft_sec80;
		}
		qdf_mem_copy(&params.classifier_params,
			     &spectral->classifier_params,
			     sizeof(struct spectral_classifier_params));

		target_if_spectral_log_SAMP_param(&params);
		target_if_spectral_create_samp_msg(spectral, &params);
	}

	return 0;
}

int
target_if_spectral_dump_hdr_gen2(struct spectral_phyerr_hdr_gen2 *phdr)
{
	uint32_t a = 0;
	uint32_t b = 0;

	qdf_mem_copy(&a, (uint8_t *)phdr, sizeof(int));
	qdf_mem_copy(&b,
		     (uint8_t *)((uint8_t *)phdr + sizeof(int)),
		     sizeof(int));

	spectral_debug("SPECTRAL : HEADER A 0x%x (%d)", a, a);
	spectral_debug("SPECTRAL : HEADER B 0x%x (%d)", b, b);
	return 0;
}

int8_t
target_if_get_combrssi_sec80_seg_gen2(
	struct target_if_spectral *spectral,
	struct spectral_search_fft_info_gen2 *p_sfft_sec80)
{
	uint32_t avgpwr_db = 0;
	uint32_t total_gain_db = 0;
	uint32_t offset = 0;
	int8_t comb_rssi = 0;

	/* Obtain required parameters for algorithm from search FFT report */
	avgpwr_db = p_sfft_sec80->avgpwr_db;
	total_gain_db = p_sfft_sec80->total_gain_info;

	/* Calculate offset */
	offset = target_if_get_offset_swar_sec80(spectral->ch_width);

	/* Calculate RSSI */
	comb_rssi = ((avgpwr_db - total_gain_db) + offset);

	return comb_rssi;
}

int
target_if_spectral_dump_tlv_gen2(
	struct spectral_phyerr_tlv_gen2 *ptlv, bool is_160_format)
{
	int ret = 0;

	/*
	 * TODO : Do not delete the following print
	 *        The scripts used to validate Spectral depend on this Print
	 */
	spectral_debug("SPECTRAL : TLV Length is 0x%x (%d)",
		       ptlv->length, ptlv->length);

	switch (ptlv->tag) {
	case TLV_TAG_SPECTRAL_SUMMARY_REPORT_GEN2:
		ret =
		    target_if_dump_summary_report_gen2(
			ptlv, ptlv->length, is_160_format);
		break;

	case TLV_TAG_SEARCH_FFT_REPORT_GEN2:
		ret =
		    target_if_dump_sfft_report_gen2(ptlv, ptlv->length,
						    is_160_format);
		break;

	case TLV_TAG_ADC_REPORT_GEN2:
		ret = target_if_dump_adc_report_gen2(ptlv, ptlv->length);
		break;

	default:
		spectral_warn("INVALID TLV");
		ret = -1;
		break;
	}

	return ret;
}

int
target_if_spectral_dump_phyerr_data_gen2(uint8_t *data, uint32_t datalen,
					 bool is_160_format)
{
	struct spectral_phyerr_tlv_gen2 *ptlv = NULL;
	uint32_t bytes_processed = 0;
	uint32_t bytes_remaining = datalen;
	uint32_t curr_tlv_complete_size = 0;

	if (datalen < sizeof(struct spectral_phyerr_tlv_gen2)) {
		spectral_err("Total PHY error data length %u too short to contain any TLVs",
			     datalen);
		return -EPERM;
	}

	while (bytes_processed < datalen) {
		if (bytes_remaining < sizeof(struct spectral_phyerr_tlv_gen2)) {
			spectral_err("Remaining PHY error data length %u too short to contain a TLV",
				     bytes_remaining);
			return -EPERM;
		}

		ptlv = (struct spectral_phyerr_tlv_gen2 *)(data +
							   bytes_processed);

		if (ptlv->signature != SPECTRAL_PHYERR_SIGNATURE_GEN2) {
			spectral_err("Invalid signature 0x%x!",
				     ptlv->signature);
			return -EPERM;
		}

		curr_tlv_complete_size =
			sizeof(struct spectral_phyerr_tlv_gen2) +
			ptlv->length;

		if (curr_tlv_complete_size > bytes_remaining) {
			spectral_err("TLV size %d greater than number of bytes remaining %d",
				     curr_tlv_complete_size, bytes_remaining);
			return -EPERM;
		}

		if (target_if_spectral_dump_tlv_gen2(ptlv, is_160_format) == -1)
			return -EPERM;

		bytes_processed += curr_tlv_complete_size;
		bytes_remaining = datalen - bytes_processed;
	}

	return 0;
}

int
target_if_process_sfft_report_gen3(
	struct spectral_phyerr_fft_report_gen3 *p_fft_report,
	struct spectral_search_fft_info_gen3 *p_sfft)
{
	/*
	 * For simplicity, everything is defined as uint32_t (except one).
	 * Proper code will later use the right sizes.
	 */
	/*
	 * For easy comparision between MDK team and OS team, the MDK script
	 * variable names have been used
	 */
	int32_t peak_sidx;
	int32_t peak_mag;

	/* Populate the Search FFT Info */
	if (p_sfft) {
		p_sfft->timestamp = p_fft_report->fft_timestamp;

		p_sfft->fft_detector_id = get_bitfield(p_fft_report->hdr_a,
						       2, 0);
		p_sfft->fft_num = get_bitfield(p_fft_report->hdr_a, 3, 2);
		p_sfft->fft_radar_check = get_bitfield(p_fft_report->hdr_a,
						       12, 5);

		peak_sidx = get_bitfield(p_fft_report->hdr_a, 11, 17);
		p_sfft->fft_peak_sidx = unsigned_to_signed(peak_sidx, 11);
		p_sfft->fft_chn_idx = get_bitfield(p_fft_report->hdr_a, 3, 28);

		p_sfft->fft_base_pwr_db = get_bitfield(p_fft_report->hdr_b,
						       9, 0);
		p_sfft->fft_total_gain_db = get_bitfield(p_fft_report->hdr_b,
							 8, 9);

		p_sfft->fft_num_str_bins_ib = get_bitfield(p_fft_report->hdr_c,
							   8, 0);
		peak_mag = get_bitfield(p_fft_report->hdr_c, 10, 8);
		p_sfft->fft_peak_mag = unsigned_to_signed(peak_mag, 10);
		p_sfft->fft_avgpwr_db = get_bitfield(p_fft_report->hdr_c,
						     7, 18);
		p_sfft->fft_relpwr_db = get_bitfield(p_fft_report->hdr_c,
						     7, 25);
	}

	return 0;
}

int
target_if_dump_fft_report_gen3(struct target_if_spectral *spectral,
			struct spectral_phyerr_fft_report_gen3 *p_fft_report,
			struct spectral_search_fft_info_gen3 *p_sfft)
{
	int i = 0;
	int fft_mag = 0;
	int fft_hdr_length = (p_fft_report->fft_hdr_length * 4);
	int report_len = (fft_hdr_length + 8);
	int fft_bin_len = (fft_hdr_length - 16);
	int fft_bin_len_adj = fft_bin_len >> 2;
	int fft_bin_len_inband_tfer = 0;
	int fft_bin_len_to_dump = fft_bin_len;

	if ((spectral->params.ss_rpt_mode == 2) &&
			spectral->inband_fftbin_size_adj) {
		fft_bin_len_adj >>= 1;
		fft_bin_len_inband_tfer = fft_bin_len >> 1;
		fft_bin_len_to_dump = fft_bin_len_inband_tfer;
	}

	spectral_debug("#############################################################");
	spectral_debug("Spectral search fft_report");
	spectral_debug("fft_timestamp  = 0x%x\nfft_hdr_length = %d(32 bit words)\nfft_hdr_tag    = 0x%x\nfft_hdr_sig    = 0x%x",
		       p_fft_report->fft_timestamp,
		       p_fft_report->fft_hdr_length,
		       p_fft_report->fft_hdr_tag, p_fft_report->fft_hdr_sig);

	spectral_debug("Length field in search fft report is %d(0x%x) bytes",
		       fft_hdr_length, fft_hdr_length);
	spectral_debug("Total length of search fft report is %d(0x%x) bytes",
		       report_len, report_len);
	spectral_debug("FW reported fftbins in report is %d(0x%x)", fft_bin_len,
		       fft_bin_len);
	if ((spectral->params.ss_rpt_mode == 2) &&
			spectral->inband_fftbin_size_adj) {
		spectral_debug("FW fftbins actually transferred (in-band report mode) "
					"%d(0x%x)",
					fft_bin_len_inband_tfer, fft_bin_len_inband_tfer);
	}
	spectral_debug("Actual number of fftbins in report is %d(0x%x)\n",
			fft_bin_len_adj, fft_bin_len_adj);

	spectral_debug("fft_detector_id = %u\nfft_num = %u\nfft_radar_check = %u\nfft_peak_sidx = %d\nfft_chn_idx = %u\nfft_base_pwr_db = %u\nfft_total_gain_db   = %u\nfft_num_str_bins_ib = %u\nfft_peak_mag   = %d\nfft_avgpwr_db  = %u\nfft_relpwr_db  = %u",
		       p_sfft->fft_detector_id,
		       p_sfft->fft_num,
		       p_sfft->fft_radar_check,
		       p_sfft->fft_peak_sidx,
		       p_sfft->fft_chn_idx,
		       p_sfft->fft_base_pwr_db,
		       p_sfft->fft_total_gain_db,
		       p_sfft->fft_num_str_bins_ib,
		       p_sfft->fft_peak_mag,
		       p_sfft->fft_avgpwr_db, p_sfft->fft_relpwr_db);

	spectral_debug("FFT bins:");
	for (i = 0; i < fft_bin_len_to_dump; i++) {
		if (i % 16 == 0)
			spectral_debug("\n%d :", i);
		fft_mag =
		    ((uint8_t *)p_fft_report)[SPECTRAL_FFT_BINS_POS + i];
		spectral_debug("%d ", fft_mag);
	}
	spectral_debug("\n");
	spectral_debug("#############################################################");

	return 0;
}

#ifdef DIRECT_BUF_RX_ENABLE
/**
 * target_if_consume_sscan_report_gen3() - Consume spectral summary report
 * @spectral: Pointer to spectral object
 * @data: Pointer to spectral summary
 *
 * Consume spectral summary report for gen3
 *
 * Return: rssi
 */
static int
target_if_consume_sscan_report_gen3(struct target_if_spectral *spectral,
				    uint8_t *data) {
	int rssi;
	struct spectral_sscan_report_gen3 *psscan_report;

	psscan_report = (struct spectral_sscan_report_gen3 *)data;
	/* RSSI is in 1/2 dBm steps, Covert it to dBm scale */
	rssi = (get_bitfield(psscan_report->hdr_a, 10, 18)) >> 1;

	return rssi;
}

/**
 * target_if_verify_sig_and_tag_gen3() - Verify tag and signature
 *                                       of spectral report
 * @spectral: Pointer to spectral object
 * @data: Pointer to spectral summary report
 * @exp_tag: iexpected tag value
 *
 * Process fft report for gen3
 *
 * Return: SUCCESS/FAILURE
 */
static int
target_if_verify_sig_and_tag_gen3(struct target_if_spectral *spectral,
				  uint8_t *data, uint8_t exp_tag)
{
	uint8_t tag = 0;
	uint8_t signature = 0;

	/* Peek into the data to figure out whether
	 *      1) Signature matches the expected value
	 *      2) What is inside the package (TAG ID is used for finding this)
	 */
	tag = *(data + PHYERR_HDR_TAG_POS);
	signature = *(data + PHYERR_HDR_SIG_POS);

	if (signature != SPECTRAL_PHYERR_SIGNATURE_GEN3) {
		if (spectral_debug_level & DEBUG_SPECTRAL4)
			spectral_err("Unexpected sig %x in spectral phyerror",
				  signature);
			spectral_err("Expected sig is %x\n",
				  SPECTRAL_PHYERR_SIGNATURE_GEN3);
		spectral->diag_stats.spectral_mismatch++;
		return -EINVAL;
	}

	if (tag != exp_tag) {
		if (spectral_debug_level & DEBUG_SPECTRAL4)
			spectral_err("Unexpected tag %x in spectral phyerror",
				  tag);
			spectral_err("Expected tag is %x\n", exp_tag);
		spectral->diag_stats.spectral_mismatch++;
		return -EINVAL;
	}

	return 0;
}

static uint8_t
target_if_spectral_get_lowest_chn_idx(uint8_t chainmask)
{
	uint8_t idx;

	for (idx = 0; idx < DBR_MAX_CHAINS; idx++) {
		if (chainmask & 0x1)
			break;
		chainmask >>= 1;
	}
	return idx;
}

int
target_if_consume_spectral_report_gen3(
	 struct target_if_spectral *spectral,
	 struct spectral_report *report)
{
	/*
	 * XXX : The classifier do not use all the members of the SAMP
	 *       message data format.
	 *       The classifier only depends upon the following parameters
	 *
	 *          1. Frequency (freq, msg->freq)
	 *          2. Spectral RSSI (spectral_rssi,
	 *          msg->samp_data.spectral_rssi)
	 *          3. Bin Power Count (bin_pwr_count,
	 *          msg->samp_data.bin_pwr_count)
	 *          4. Bin Power values (bin_pwr, msg->samp_data.bin_pwr[0]
	 *          5. Spectral Timestamp (spectral_tstamp,
	 *          msg->samp_data.spectral_tstamp)
	 *          6. MAC Address (macaddr, msg->macaddr)
	 *
	 *       This function prepares the params structure and populates it
	 *       with
	 *       relevant values, this is in turn passed to
	 *       spectral_create_samp_msg()
	 *       to prepare fully formatted Spectral SAMP message
	 *
	 *       XXX : Need to verify
	 *          1. Order of FFT bin values
	 *
	 */
	uint64_t tsf64 = 0;
	struct target_if_samp_msg_params params;
	struct spectral_search_fft_info_gen3 search_fft_info;
	struct spectral_search_fft_info_gen3 *p_sfft = &search_fft_info;
	int8_t rssi_up = 0;
	int8_t rssi_low = 0;
	int8_t chn_idx_lowest_enabled  = 0;
	uint8_t control_rssi   = 0;
	uint8_t extension_rssi = 0;
	int fft_hdr_length = 0;
	int report_len = 0;
	int fft_bin_len = 0;
	struct target_if_spectral_ops *p_sops =
		GET_TARGET_IF_SPECTRAL_OPS(spectral);
	struct spectral_phyerr_fft_report_gen3 *p_fft_report;
	int8_t rssi;
	uint8_t *data = report->data;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_rxchainmask;

	OS_MEMZERO(&params, sizeof(params));

	if (target_if_verify_sig_and_tag_gen3(
			spectral, data,
			TLV_TAG_SPECTRAL_SUMMARY_REPORT_GEN3) != 0)
		goto fail;
	rssi = target_if_consume_sscan_report_gen3(spectral, data);
	/* Advance buf pointer to the search fft report */
	data += sizeof(struct spectral_sscan_report_gen3);

	if (target_if_verify_sig_and_tag_gen3(
			spectral, data,
			TLV_TAG_SEARCH_FFT_REPORT_GEN3) != 0)
		goto fail;
	p_fft_report = (struct spectral_phyerr_fft_report_gen3 *)data;
	fft_hdr_length = p_fft_report->fft_hdr_length * 4;
	if (fft_hdr_length < 16) {
		spectral_err("Unexpected TLV length %u for FFT Report! Hexdump follows",
			     fft_hdr_length);
		goto fail;
	}

	report_len = (fft_hdr_length + 8);

	fft_bin_len = (fft_hdr_length - 16);
	fft_bin_len >>= 2;
	if ((spectral->params.ss_rpt_mode == 2) &&
			spectral->inband_fftbin_size_adj) {
		fft_bin_len >>= 1;
	}

	tsf64 = p_fft_report->fft_timestamp;

	target_if_process_sfft_report_gen3(p_fft_report, p_sfft);

	if (p_sfft->fft_detector_id != 0) {
		spectral_err("Expected segid is 0 but we got %d",
			     p_sfft->fft_detector_id);
		spectral->diag_stats.spectral_vhtseg1id_mismatch++;
		goto fail;
	}

	if (spectral_debug_level & (DEBUG_SPECTRAL2 | DEBUG_SPECTRAL4))
		target_if_dump_fft_report_gen3(spectral, p_fft_report, p_sfft);

	if (spectral->upper_is_control)
		rssi_up = control_rssi;
	else
		rssi_up = extension_rssi;

	if (spectral->lower_is_control)
		rssi_low = control_rssi;
	else
		rssi_low = extension_rssi;

	params.rssi         = rssi;
	params.lower_rssi   = rssi_low;
	params.upper_rssi   = rssi_up;

	if (spectral->sc_spectral_noise_pwr_cal) {
	/* Fill 0's till FW provides them */
		params.chain_ctl_rssi[0] = 0;
		params.chain_ctl_rssi[1] = 0;
		params.chain_ctl_rssi[2] = 0;
		params.chain_ext_rssi[0] = 0;
		params.chain_ext_rssi[1] = 0;
		params.chain_ext_rssi[2] = 0;
	}

	vdev = target_if_spectral_get_vdev(spectral);
	if (!vdev)
		return -ENOENT;

	vdev_rxchainmask =
	    wlan_vdev_mlme_get_rxchainmask(vdev);
	QDF_ASSERT(vdev_rxchainmask != 0);
	wlan_objmgr_vdev_release_ref(vdev,
				     WLAN_SPECTRAL_ID);

	chn_idx_lowest_enabled =
	target_if_spectral_get_lowest_chn_idx(vdev_rxchainmask);
	if (chn_idx_lowest_enabled >= DBR_MAX_CHAINS)
		return -EINVAL;

	control_rssi    = 0;
	extension_rssi  = 0;

	params.bwinfo   = 0;
	params.tstamp   = 0;
	params.max_mag  = p_sfft->fft_peak_mag;

	/* params.max_index    = p_sfft->peak_inx; */
	params.max_exp = 0;
	params.peak = 0;
	params.bin_pwr_data = (uint8_t *)((uint8_t *)p_fft_report +
					   SPECTRAL_FFT_BINS_POS);
	params.freq = p_sops->get_current_channel(spectral);
	params.freq_loading = 0;

	params.interf_list.count = 0;
	params.max_lower_index = 0;
	params.max_upper_index = 0;
	params.nb_lower = 0;
	params.nb_upper = 0;
	/*
	 * For modes upto VHT80, the noise floor is populated with the one
	 * corresponding
	 * to the highest enabled antenna chain
	 */
	/* TODO:  Fill proper values once FW provides them*/
	params.noise_floor       = report->noisefloor[chn_idx_lowest_enabled];
	params.datalen           = (fft_hdr_length * 4);
	params.pwr_count         = fft_bin_len;
	params.tstamp            = (tsf64 & SPECTRAL_TSMASK);

	if (spectral->ch_width == CH_WIDTH_160MHZ) {
		/* We expect to see one more Search FFT report, and it should be
		 * equal in size to the current one.
		 */
		/* Advance to the secondary 80 Mhz spectral report */
		data += report_len;

		if (target_if_verify_sig_and_tag_gen3(
				spectral, data,
				TLV_TAG_SPECTRAL_SUMMARY_REPORT_GEN3) != 0)
			goto fail;
		rssi = target_if_consume_sscan_report_gen3(spectral, data);
		/* Advance buf pointer to the search fft report */
		data += sizeof(struct spectral_sscan_report_gen3);

		if (target_if_verify_sig_and_tag_gen3(
				spectral, data,
				TLV_TAG_SEARCH_FFT_REPORT_GEN3) != 0)
			goto fail;
		p_fft_report = (struct spectral_phyerr_fft_report_gen3 *)(data);
		fft_hdr_length = p_fft_report->fft_hdr_length * 4;
		report_len     = (fft_hdr_length + 8);

		fft_bin_len    = (fft_hdr_length - 16);
		fft_bin_len >>= 2;
		if ((spectral->params.ss_rpt_mode == 2) &&
				spectral->inband_fftbin_size_adj) {
			fft_bin_len >>= 1;
		}

		target_if_process_sfft_report_gen3(p_fft_report, p_sfft);

		if (p_sfft->fft_detector_id != 1) {
			spectral_err("Expected segid is 1 but we got %d",
				     p_sfft->fft_detector_id);
			spectral->diag_stats.spectral_vhtseg2id_mismatch++;
			goto fail;
		}

		if (spectral_debug_level &
		    (DEBUG_SPECTRAL2 | DEBUG_SPECTRAL4))
			target_if_dump_fft_report_gen3(spectral, p_fft_report, p_sfft);

		params.vhtop_ch_freq_seg1 = 0;
		params.vhtop_ch_freq_seg2 = 0;

		/* XXX: Confirm. TBD at SoD. */
		params.rssi_sec80 = rssi;

		/* XXX: Determine dynamically. TBD at SoD. */

		/*
		 * For VHT80_80/VHT160,the noise floor for primary 80MHz segment
		 * populated with the
		 * lowest enabled antenna chain and the nf for secondary 80MHz
		 * segment is populated
		 * with the highest enabled antenna chain
		 */
		/* TODO:  Fill proper values once FW provides them*/
		params.noise_floor_sec80    = DUMMY_NF_VALUE;
		params.noise_floor          = DUMMY_NF_VALUE;

		params.max_mag_sec80        = p_sfft->fft_peak_mag;
		/* params.max_index_sec80      = p_sfft->peak_inx; */
		/* XXX Does this definition of datalen *still hold? */
		params.datalen_sec80        = fft_hdr_length;
		params.pwr_count_sec80      = fft_bin_len;
		params.bin_pwr_data_sec80   = (u_int8_t *)(
			(uint8_t *)p_fft_report + SPECTRAL_FFT_BINS_POS);
	}

	qdf_mem_copy(&params.classifier_params,
		     &spectral->classifier_params,
		     sizeof(struct spectral_classifier_params));

	target_if_spectral_log_SAMP_param(&params);
	target_if_spectral_create_samp_msg(spectral, &params);

	return 0;

 fail:
	spectral_err("Error in function while processing search fft report");
	return -EPERM;
}

int target_if_spectral_process_report_gen3(
	struct wlan_objmgr_pdev *pdev,
	void *buf)
{
	int ret = 0;
	struct direct_buf_rx_data *payload = buf;
	struct target_if_spectral *spectral;
	struct spectral_report report;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (spectral == NULL) {
		spectral_err("Spectral target object is null");
		return -EINVAL;
	}

	report.data = payload->vaddr;
	if (payload->meta_data_valid)
		qdf_mem_copy(report.noisefloor, &payload->meta_data,
			     sizeof(payload->meta_data));

	if (spectral_debug_level & (DEBUG_SPECTRAL2 | DEBUG_SPECTRAL4)) {
		spectral_debug("Printing the spectral phyerr buffer for debug");
		spectral_debug("Datalength of buffer = 0x%x(%d) bufptr = 0x%pK",
			       payload->dbr_len, payload->dbr_len,
			       payload->vaddr);
#ifdef CONFIG_WIN
		RAWSIM_PKT_HEXDUMP((unsigned char *)payload->vaddr, 1024);
#endif
	}

	ret = target_if_consume_spectral_report_gen3(spectral, &report);

	if (spectral_debug_level & DEBUG_SPECTRAL4)
		spectral_debug_level = DEBUG_SPECTRAL;

	return ret;
}
#else
int target_if_spectral_process_report_gen3(
	struct wlan_objmgr_pdev *pdev,
	void *buf)
{
	spectral_err("Direct dma support is not enabled");
	return -EINVAL;
}
#endif
qdf_export_symbol(target_if_spectral_process_report_gen3);
/* END of spectral GEN III HW specific functions */

#endif  /* WLAN_CONV_SPECTRAL_ENABLE */
