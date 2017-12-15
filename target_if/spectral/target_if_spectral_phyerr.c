/*
 * Copyright (c) 2011,2017 The Linux Foundation. All rights reserved.
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
extern int spectral_debug_level;

/* START of spectral GEN II HW specific function declarations */
static int dump_summary_report_gen2(
	SPECTRAL_PHYERR_TLV_GEN2 *ptlv,
	 int tlvlen,
	 bool is_160_format);
static int process_search_fft_report_gen2(
	SPECTRAL_PHYERR_TLV_GEN2 *ptlv,
	 int tlvlen,
	 SPECTRAL_SEARCH_FFT_INFO_GEN2 *p_fft_info);
static int dump_adc_report_gen2(SPECTRAL_PHYERR_TLV_GEN2 *ptlv, int tlvlen);
static int dump_search_fft_report_gen2(
	SPECTRAL_PHYERR_TLV_GEN2 *ptlv,
	 int tlvlen,
	 bool is_160_format);
/* END of spectral GEN II HW specific function declarations */

#if WLAN_SPECTRAL_ENABLE

/*
 * Function     : print_buf
 * Description  : Prints given buffer for given length
 * Input        : Pointer to buffer and length
 * Output       : Void
 *
 */
static void print_buf(u_int8_t *pbuf, int len)
{
	int i = 0;

	for (i = 0; i < len; i++) {
		qdf_print("%02X ", pbuf[i]);
		if (i % 32 == 31)
			qdf_print("\n");
	}
}

/*
 * Function     : target_if_spectral_dump_fft
 * Description  : Dump Spectral FFT
 * Input        : Pointer to Spectral Phyerr FFT
 * Output       : Success/Failure
 *
 */
int target_if_spectral_dump_fft(u_int8_t *pfft, int fftlen)
{
	int i = 0;

	/* TODO : Do not delete the following print
	 *        The scripts used to validate Spectral depend on this Print
	 */
	qdf_print("SPECTRAL : FFT Length is 0x%x (%d)\n", fftlen, fftlen);

	qdf_print("fft_data # ");
	for (i = 0; i < fftlen; i++) {
		qdf_print("%d ", pfft[i]);
		/* if (i % 32 == 31)
		 * qdf_print("\n");
		 */
	}
	qdf_print("\n");
	return 0;
}

/*
 * Function     : target_if_spectral_send_tlv_to_host
 * Description  : Send the TLV information to Host
 * Input        : Pointer to the TLV
 * Output       : Success/Failure
 *
 */

int target_if_spectral_send_tlv_to_host(
	struct target_if_spectral *spectral,
	 u_int8_t *data,
	 u_int32_t datalen)
{
	int status = true;

	target_if_spectral_prep_skb(spectral);
	if (spectral->spectral_skb) {
		spectral->spectral_nlh =
			(struct nlmsghdr *)spectral->spectral_skb->data;
		memcpy(NLMSG_DATA(spectral->spectral_nlh), data, datalen);
		target_if_spectral_bcast_msg(spectral);
	} else {
		status = false;
	}
	return status;
}

/*
 * Function     : dbg_print_SAMP_param
 * Description  : Print contents of SAMP struct
 * Input        : Pointer to SAMP message
 * Output       : Void
 *
 */
void target_if_dbg_print_SAMP_param(struct target_if_samp_msg_params *p)
{
	qdf_print("\nSAMP Packet : -------------------- START --------------"
		  "------\n");
	qdf_print("Freq        = %d\n", p->freq);
	qdf_print("RSSI        = %d\n", p->rssi);
	qdf_print("Bin Count   = %d\n", p->pwr_count);
	qdf_print("Timestamp   = %d\n", p->tstamp);
	qdf_print("SAMP Packet : -------------------- END ------------------"
		  "-----\n");
}

/*
 * Function     : dbg_print_SAMP_msg
 * Description  : Print contents of SAMP Message
 * Input        : Pointer to SAMP message
 * Output       : Void
 *
 */
void target_if_dbg_print_SAMP_msg(SPECTRAL_SAMP_MSG *ss_msg)
{
	int i = 0;

	SPECTRAL_SAMP_DATA *p = &ss_msg->samp_data;
	SPECTRAL_CLASSIFIER_PARAMS *pc = &p->classifier_params;
	struct INTERF_SRC_RSP *pi = &p->interf_list;

	line();
	qdf_print("Spectral Message\n");
	line();
	qdf_print("Signature   :   0x%x\n", ss_msg->signature);
	qdf_print("Freq        :   %d\n", ss_msg->freq);
	qdf_print("Freq load   :   %d\n", ss_msg->freq_loading);
	qdf_print("Intfnc type :   %d\n", ss_msg->int_type);
	line();
	qdf_print("Spectral Data info\n");
	line();
	qdf_print("data length     :   %d\n", p->spectral_data_len);
	qdf_print("rssi            :   %d\n", p->spectral_rssi);
	qdf_print("combined rssi   :   %d\n", p->spectral_combined_rssi);
	qdf_print("upper rssi      :   %d\n", p->spectral_upper_rssi);
	qdf_print("lower rssi      :   %d\n", p->spectral_lower_rssi);
	qdf_print("bw info         :   %d\n", p->spectral_bwinfo);
	qdf_print("timestamp       :   %d\n", p->spectral_tstamp);
	qdf_print("max index       :   %d\n", p->spectral_max_index);
	qdf_print("max exp         :   %d\n", p->spectral_max_exp);
	qdf_print("max mag         :   %d\n", p->spectral_max_mag);
	qdf_print("last timstamp   :   %d\n", p->spectral_last_tstamp);
	qdf_print("upper max idx   :   %d\n", p->spectral_upper_max_index);
	qdf_print("lower max idx   :   %d\n", p->spectral_lower_max_index);
	qdf_print("bin power count :   %d\n", p->bin_pwr_count);
	line();
	qdf_print("Classifier info\n");
	line();
	qdf_print("20/40 Mode      :   %d\n", pc->spectral_20_40_mode);
	qdf_print("dc index        :   %d\n", pc->spectral_dc_index);
	qdf_print("dc in MHz       :   %d\n", pc->spectral_dc_in_mhz);
	qdf_print("upper channel   :   %d\n", pc->upper_chan_in_mhz);
	qdf_print("lower channel   :   %d\n", pc->lower_chan_in_mhz);
	line();
	qdf_print("Interference info\n");
	line();
	qdf_print("inter count     :   %d\n", pi->count);

	for (i = 0; i < pi->count; i++) {
	qdf_print("inter type  :   %d\n", pi->interf[i].interf_type);
	qdf_print("min freq    :   %d\n", pi->interf[i].interf_min_freq);
	qdf_print("max freq    :   %d\n", pi->interf[i].interf_max_freq);
	}
}

/*
 * Function     : get_offset_swar_sec80
 * Description  : Get offset for SWAR according to the channel width
 * Input        : Channel width
 * Output       : Offset for SWAR algorithm
 */
uint32_t target_if_get_offset_swar_sec80(uint32_t channel_width)
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

/* START of spectral GEN II HW specific functions */

/*
 * Function     : dump_summary_report_gen2
 * Description  : Dump Spectral Summary Report
 * Input        : Pointer to Spectral Phyerr TLV and Length, flag indicating
 *                whether information provided by HW is in altered format for
 *                802.11ac 160/80+80 MHz support (QCA9984 onwards).
 * Output       : Success/Failure
 *
 */
int dump_summary_report_gen2(
	SPECTRAL_PHYERR_TLV_GEN2 *ptlv,
	 int tlvlen,
	 bool is_160_format)
{
	/* For simplicity, everything is defined as uint32_t (except one).
	 * Proper code will later use the right sizes.
	 */

	/* For easy comparision between MDK team and OS team, the MDK script
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
	SPECTRAL_PHYERR_HDR_GEN2 *phdr =
		(SPECTRAL_PHYERR_HDR_GEN2 *)((u_int8_t *)ptlv +
		sizeof(SPECTRAL_PHYERR_TLV_GEN2));

	qdf_print("SPECTRAL : SPECTRAL SUMMARY REPORT\n");

	if (is_160_format) {
	if (tlvlen != 20) {
		qdf_print("SPECTRAL : Unexpected TLV length %d for Spectral "
			  "Summary Report! Hexdump follows\n", tlvlen);
		print_buf((u_int8_t *)ptlv, tlvlen + 4);
		return -EPERM;
	}

	/* Doing copy as the contents may not be aligned */
	qdf_mem_copy(&ss_summary_A, (u_int8_t *)phdr, sizeof(int));
	qdf_mem_copy(
		&ss_summary_B,
		 (u_int8_t *)((u_int8_t *)phdr + sizeof(int)),
		 sizeof(int));
	qdf_mem_copy(
		&ss_summary_C,
		 (u_int8_t *)((u_int8_t *)phdr + 2 * sizeof(int)),
		 sizeof(int));
	qdf_mem_copy(
		&ss_summary_D,
		 (u_int8_t *)((u_int8_t *)phdr + 3 * sizeof(int)),
		 sizeof(int));
	qdf_mem_copy(
		&ss_summary_E,
		 (u_int8_t *)((u_int8_t *)phdr + 4 * sizeof(int)),
		 sizeof(int));

	/*The following is adapted from MDK scripts for easier comparability */

	recent_rfsat = ((ss_summary_A >> 8) & 0x1);
	sscan_gidx = (ss_summary_A & 0xff);
	qdf_print("sscan_gidx=%d, is_recent_rfsat=%d\n",
	       sscan_gidx, recent_rfsat);

	/* First segment */
	agc_mb_gain = ((ss_summary_B >> 10) & 0x7f);
	agc_total_gain = (ss_summary_B & 0x3ff);
	nb_mask = ((ss_summary_C >> 22) & 0xff);
	ob_flag = ((ss_summary_B >> 17) & 0x1);
	peak_inx = (ss_summary_C  & 0xfff);
	if (peak_inx > 2047)
		peak_inx = peak_inx - 4096;
	peak_mag = ((ss_summary_C >> 12) & 0x3ff);

	qdf_print("agc_total_gain_segid0 = 0x%.2x, agc_mb_gain_segid0=%d\n",
	       agc_total_gain, agc_mb_gain);
	qdf_print("nb_mask_segid0 = 0x%.2x, ob_flag_segid0=%d, "
	       "peak_index_segid0=%d, peak_mag_segid0=%d\n",
	       nb_mask, ob_flag, peak_inx, peak_mag);

	/* Second segment */
	agc_mb_gain = ((ss_summary_D >> 10) & 0x7f);
	agc_total_gain = (ss_summary_D & 0x3ff);
	nb_mask = ((ss_summary_E >> 22) & 0xff);
	ob_flag = ((ss_summary_D >> 17) & 0x1);
	peak_inx = (ss_summary_E  & 0xfff);
	if (peak_inx > 2047)
		peak_inx = peak_inx - 4096;
	peak_mag = ((ss_summary_E >> 12) & 0x3ff);

	qdf_print("agc_total_gain_segid1 = 0x%.2x, agc_mb_gain_segid1=%d\n",
	       agc_total_gain, agc_mb_gain);
	qdf_print("nb_mask_segid1 = 0x%.2x, ob_flag_segid1=%d, "
	       "peak_index_segid1=%d, peak_mag_segid1=%d\n",
	       nb_mask, ob_flag, peak_inx, peak_mag);
	} else {
	if (tlvlen != 8) {
		qdf_print("SPECTRAL : Unexpected TLV length %d for Spectral "
			  "Summary Report! Hexdump follows\n", tlvlen);
		print_buf((u_int8_t *)ptlv, tlvlen + 4);
		return -EPERM;
	}

	/* Doing copy as the contents may not be aligned */
	qdf_mem_copy(&ss_summary_A, (u_int8_t *)phdr, sizeof(int));
	qdf_mem_copy(
		&ss_summary_B,
		 (u_int8_t *)((u_int8_t *)phdr + sizeof(int)),
		 sizeof(int));

	nb_mask = ((ss_summary_B >> 22) & 0xff);
	ob_flag = ((ss_summary_B >> 30) & 0x1);
	peak_inx = (ss_summary_B  & 0xfff);

	if (peak_inx > 2047)
		peak_inx = peak_inx - 4096;

	peak_mag = ((ss_summary_B >> 12) & 0x3ff);
	agc_mb_gain = ((ss_summary_A >> 24) & 0x7f);
	agc_total_gain = (ss_summary_A  & 0x3ff);
	sscan_gidx = ((ss_summary_A >> 16) & 0xff);
	recent_rfsat = ((ss_summary_B >> 31) & 0x1);

	qdf_print("nb_mask = 0x%.2x, ob_flag=%d, peak_index=%d, peak_mag=%d, "
		  "agc_mb_gain=%d, agc_total_gain=%d, sscan_gidx=%d, "
		  "recent_rfsat=%d\n",
		  nb_mask, ob_flag, peak_inx, peak_mag, agc_mb_gain,
		  agc_total_gain, sscan_gidx, recent_rfsat);
	}

	return 0;
}

/*
 * Function     : process_search_fft_report_gen2
 * Description  : Process Search FFT Report
 * Input        : Pointer to Spectral Phyerr TLV and Length and pointer to
 *                search fft info
 * Output       : Success/Failure
 *
 */
int process_search_fft_report_gen2(
	SPECTRAL_PHYERR_TLV_GEN2 *ptlv,
	 int tlvlen,
	 SPECTRAL_SEARCH_FFT_INFO_GEN2 *p_fft_info)
{
	/* For simplicity, everything is defined as uint32_t (except one).
	 * Proper code will later use the right sizes.
	 */
	/* For easy comparision between MDK team and OS team, the MDK script
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
	u_int8_t *tmp = (u_int8_t *)ptlv;
	SPECTRAL_PHYERR_HDR_GEN2 *phdr =
	  (SPECTRAL_PHYERR_HDR_GEN2 *)(tmp + sizeof(SPECTRAL_PHYERR_TLV_GEN2));

	/* Relook this */
	if (tlvlen < 8) {
		qdf_print("SPECTRAL : Unexpected TLV length %d for Spectral "
			  "Summary Report! Hexdump follows\n", tlvlen);
		print_buf((u_int8_t *)ptlv, tlvlen + 4);
		return -EPERM;
	}

	/* Doing copy as the contents may not be aligned */
	qdf_mem_copy(&fft_summary_A, (u_int8_t *)phdr, sizeof(int));
	qdf_mem_copy(
		&fft_summary_B,
		 (u_int8_t *)((u_int8_t *)phdr + sizeof(int)),
		 sizeof(int));

	relpwr_db       = ((fft_summary_B >> 26) & 0x3f);
	num_str_bins_ib = fft_summary_B & 0xff;
	base_pwr        = ((fft_summary_A >> 14) & 0x1ff);
	total_gain_info = ((fft_summary_A >> 23) & 0x1ff);

	fft_chn_idx     = ((fft_summary_A >> 12) & 0x3);
	peak_inx        = fft_summary_A & 0xfff;

	if (peak_inx > 2047)
		peak_inx = peak_inx - 4096;

	avgpwr_db = ((fft_summary_B >> 18) & 0xff);
	peak_mag = ((fft_summary_B >> 8) & 0x3ff);

	/* Populate the Search FFT Info */
	if (p_fft_info) {
	p_fft_info->relpwr_db       = relpwr_db;
	p_fft_info->num_str_bins_ib = num_str_bins_ib;
	p_fft_info->base_pwr        = base_pwr;
	p_fft_info->total_gain_info = total_gain_info;
	p_fft_info->fft_chn_idx     = fft_chn_idx;
	p_fft_info->peak_inx        = peak_inx;
	p_fft_info->avgpwr_db       = avgpwr_db;
	p_fft_info->peak_mag        = peak_mag;
	}

	return 0;
}

/*
 * Function     : dump_adc_report_gen2
 * Description  : Dump ADC Reports
 * Input        : Pointer to Spectral Phyerr TLV and Length
 * Output       : Success/Failure
 *
 */
int dump_adc_report_gen2(SPECTRAL_PHYERR_TLV_GEN2 *ptlv, int tlvlen)
{
	int i;
	uint32_t *pdata;
	uint32_t data;

	/* For simplicity, everything is defined as uint32_t (except one).
	 * Proper code will later use the right sizes.
	 */
	uint32_t samp_fmt;
	uint32_t chn_idx;
	uint32_t recent_rfsat;
	uint32_t agc_mb_gain;
	uint32_t agc_total_gain;

	uint32_t adc_summary = 0;

	u_int8_t *ptmp = (u_int8_t *)ptlv;

	qdf_print("SPECTRAL : ADC REPORT\n");

	/* Relook this */
	if (tlvlen < 4) {
		qdf_print("Unexpected TLV length %d for ADC Report! Hexdump"
			  " follows\n", tlvlen);
		print_buf((u_int8_t *)ptlv, tlvlen + 4);
		return -EPERM;
	}

	qdf_mem_copy(&adc_summary, (u_int8_t *)(ptlv + 4), sizeof(int));

	samp_fmt = ((adc_summary >> 28) & 0x1);
	chn_idx = ((adc_summary >> 24) & 0x3);
	recent_rfsat = ((adc_summary >> 23) & 0x1);
	agc_mb_gain = ((adc_summary >> 16) & 0x7f);
	agc_total_gain = adc_summary & 0x3ff;

	qdf_print("samp_fmt= %u, chn_idx= %u, recent_rfsat= %u, agc_mb_gain=%u"
		  " agc_total_gain=%u\n", samp_fmt, chn_idx, recent_rfsat,
		  agc_mb_gain, agc_total_gain);

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

		qdf_print("SPECTRAL ADC : Interpreting capture format 1\n");
		qdf_print("adc_data_format_1 # %d %d %d\n", 2 * i, si1, sq1);
		qdf_print("adc_data_format_1 # %d %d %d\n",
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
		qdf_print("SPECTRAL ADC : Interpreting capture format 0\n");
		qdf_print("adc_data_format_2 # %d %d %d\n", i, si1, sq1);
	}
	}

	qdf_print("\n");

	return 0;
}

/*
 * Function     : dump_search_fft_report_gen2
 * Description  : Process Search FFT Report
 * Input        : Pointer to Spectral Phyerr TLV and Length, flag indicating
 *                whether information provided by HW is in altered format for
 *                802.11ac 160/80+80 MHz support (QCA9984 onwards).
 * Output       : Success/Failure
 *
 */
int dump_search_fft_report_gen2(
	SPECTRAL_PHYERR_TLV_GEN2 *ptlv,
	 int tlvlen,
	 bool is_160_format)
{
	int i;
	uint32_t fft_mag;

	/* For simplicity, everything is defined as uint32_t (except one).
	 * Proper code will later use the right sizes.
	 */
	/* For easy comparision between MDK team and OS team, the MDK script
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
	u_int8_t segid;

	uint32_t fft_summary_A = 0;
	uint32_t fft_summary_B = 0;
	uint32_t fft_summary_C = 0;
	u_int8_t *tmp = (u_int8_t *)ptlv;
	SPECTRAL_PHYERR_HDR_GEN2 *phdr =
	  (SPECTRAL_PHYERR_HDR_GEN2 *)(tmp + sizeof(SPECTRAL_PHYERR_TLV_GEN2));
	u_int32_t segid_skiplen = 0;

	if (is_160_format)
		segid_skiplen = sizeof(SPECTRAL_SEGID_INFO);

	qdf_print("SPECTRAL : SEARCH FFT REPORT\n");

	/* Relook this */
	if (tlvlen < (8 + segid_skiplen)) {
		qdf_print("SPECTRAL : Unexpected TLV length %d for Spectral "
			  "Summary Report! Hexdump follows\n", tlvlen);
		print_buf((u_int8_t *)ptlv, tlvlen + 4);
		return -EPERM;
	}

	/* Doing copy as the contents may not be aligned */
	qdf_mem_copy(&fft_summary_A, (u_int8_t *)phdr, sizeof(int));
	qdf_mem_copy(
		&fft_summary_B,
		 (u_int8_t *)((u_int8_t *)phdr + sizeof(int)),
		 sizeof(int));
	if (is_160_format)
		qdf_mem_copy(
			&fft_summary_C,
			 (u_int8_t *)((u_int8_t *)phdr + 2 * sizeof(int)),
			 sizeof(int));

	relpwr_db       = ((fft_summary_B >> 26) & 0x3f);
	num_str_bins_ib = fft_summary_B & 0xff;
	base_pwr        = ((fft_summary_A >> 14) & 0x1ff);
	total_gain_info = ((fft_summary_A >> 23) & 0x1ff);

	fft_chn_idx     = ((fft_summary_A >> 12) & 0x3);
	peak_inx        = fft_summary_A & 0xfff;

	if (peak_inx > 2047)
		peak_inx = peak_inx - 4096;

	avgpwr_db = ((fft_summary_B >> 18) & 0xff);
	peak_mag = ((fft_summary_B >> 8) & 0x3ff);

	qdf_print("Header A = 0x%x Header B = 0x%x\n",
		  phdr->hdr_a, phdr->hdr_b);
	qdf_print("Base Power= 0x%x, Total Gain= %d, relpwr_db=%d, "
		  "num_str_bins_ib=%d fft_chn_idx=%d peak_inx=%d avgpwr_db=%d "
		  "peak_mag=%d\n", base_pwr, total_gain_info, relpwr_db,
		  num_str_bins_ib, fft_chn_idx, peak_inx, avgpwr_db, peak_mag);
	if (is_160_format) {
		segid = fft_summary_C & 0x1;
		qdf_print("Segment ID: %hhu\n", segid);
	}

	qdf_print("FFT bins:\n");
	for (i = 0; i < (tlvlen - 8 - segid_skiplen); i++) {
	fft_mag = ((u_int8_t *)ptlv)[12 + segid_skiplen + i];
	qdf_print("%d %d, ", i, fft_mag);
	}

	qdf_print("\n");

	return 0;
}

/*
 * Function     : spectral_process_phyerr_gen2
 * Description  : Process PHY Error
 * Input        : Pointer to buffer
 * Output       : Success/Failure
 *
 */
int spectral_process_phyerr_gen2(
	struct target_if_spectral *spectral,
	u_int8_t *data,
	u_int32_t datalen, struct target_if_spectral_rfqual_info *p_rfqual,
	struct target_if_spectral_chan_info *p_chaninfo,
	u_int64_t tsf64,
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
	SPECTRAL_SEARCH_FFT_INFO_GEN2 search_fft_info;
	SPECTRAL_SEARCH_FFT_INFO_GEN2 *p_sfft = &search_fft_info;
	SPECTRAL_SEARCH_FFT_INFO_GEN2 search_fft_info_sec80;
	SPECTRAL_SEARCH_FFT_INFO_GEN2 *p_sfft_sec80 = &search_fft_info_sec80;
	u_int32_t segid_skiplen = 0;

	int8_t rssi_up  = 0;
	int8_t rssi_low = 0;

	int8_t chn_idx_highest_enabled = 0;
	int8_t chn_idx_lowest_enabled  = 0;

	u_int8_t control_rssi   = 0;
	u_int8_t extension_rssi = 0;
	u_int8_t combined_rssi  = 0;

	u_int32_t tstamp    = 0;

	struct target_if_spectral_ops *p_sops = GET_TIF_SPECTRAL_OPS(spectral);

	SPECTRAL_PHYERR_TLV_GEN2 *ptlv = (SPECTRAL_PHYERR_TLV_GEN2 *)data;
	SPECTRAL_PHYERR_TLV_GEN2 *ptlv_sec80 = NULL;
	SPECTRAL_PHYERR_FFT_GEN2 *pfft = NULL;
	SPECTRAL_PHYERR_FFT_GEN2 *pfft_sec80 = NULL;

	u_int8_t segid = 0;
	u_int8_t segid_sec80 = 0;

	if (spectral->is_160_format)
	segid_skiplen = sizeof(SPECTRAL_SEGID_INFO);

	pfft = (SPECTRAL_PHYERR_FFT_GEN2 *)(data +
	  sizeof(SPECTRAL_PHYERR_TLV_GEN2) +
	  sizeof(SPECTRAL_PHYERR_HDR_GEN2) + segid_skiplen);

	/* XXX Extend SPECTRAL_DPRINTK() to use spectral_debug_level,
	 * and use this facility inside spectral_dump_phyerr_data()
	 * and supporting functions.
	 */
	if (spectral_debug_level & ATH_DEBUG_SPECTRAL2)
	spectral_dump_phyerr_data_gen2(data, datalen, spectral->is_160_format);

	if (spectral_debug_level & ATH_DEBUG_SPECTRAL4) {
	spectral_dump_phyerr_data_gen2(data, datalen, spectral->is_160_format);
	spectral_debug_level = ATH_DEBUG_SPECTRAL;
	}

	if (ptlv->signature != SPECTRAL_PHYERR_SIGNATURE_GEN2) {
	/* EV# 118023: We tentatively disable the below print
	 * and provide stats instead.
	 */
	/* qdf_print("SPECTRAL : Mismatch\n"); */
	spectral->diag_stats.spectral_mismatch++;
	return -EPERM;
	}

	OS_MEMZERO(&params, sizeof(params));

	if (ptlv->tag == TLV_TAG_SEARCH_FFT_REPORT_GEN2) {
	if (spectral->is_160_format) {
		segid = *((SPECTRAL_SEGID_INFO *)(
				(u_int8_t *)ptlv +
				sizeof(SPECTRAL_PHYERR_TLV_GEN2) +
				sizeof(SPECTRAL_PHYERR_HDR_GEN2)));

		if (segid != 0) {
		spectral->diag_stats.spectral_vhtseg1id_mismatch++;
		return -EPERM;
		}
	}

	process_search_fft_report_gen2(ptlv, ptlv->length, &search_fft_info);

	tstamp = p_sops->get_tsf64(spectral) & SPECTRAL_TSMASK;

	combined_rssi   = p_rfqual->rssi_comb;

	if (spectral->upper_is_control)
		rssi_up = control_rssi;
	else
		rssi_up = extension_rssi;

	if (spectral->lower_is_control)
		rssi_low = control_rssi;
	else
		rssi_low = extension_rssi;

	params.rssi         = p_rfqual->rssi_comb;
	params.lower_rssi   = rssi_low;
	params.upper_rssi   = rssi_up;

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
	 *       There are three chains in Peregrine and 4 chains in Beeliner &
	 *       Cascade
	 *       This value decides the per-chain enable mask to select
	 *       the input ADC for search FTT.
	 *       For modes upto VHT80, if more than one chain is enabled, the
	 *       max valid chain
	 *       is used. LSB corresponds to chain zero.
	 *       For VHT80_80 and VHT160, the lowest enabled chain is used for
	 *       primary
	 *       detection and highest enabled chain is used for secondary
	 *       detection.
	 *
	 *  XXX: The current algorithm do not use these control and extension
	 *       channel
	 *       Instead, it just relies on the combined RSSI values only.
	 *       For fool-proof detection algorithm, we should take these RSSI
	 *       values
	 *       in to account. This is marked for future enhancements.
	 */
	chn_idx_highest_enabled = ((spectral->params.ss_chn_mask & 0x8) ? 3 :
			(spectral->params.ss_chn_mask & 0x4) ? 2 :
			(spectral->params.ss_chn_mask & 0x2) ? 1 : 0);
	chn_idx_lowest_enabled  = ((spectral->params.ss_chn_mask & 0x1) ? 0 :
			(spectral->params.ss_chn_mask & 0x2) ? 1 :
			(spectral->params.ss_chn_mask & 0x4) ? 2 : 3);
	control_rssi    = (u_int8_t)
		p_rfqual->pc_rssi_info[chn_idx_highest_enabled].rssi_pri20;
	extension_rssi  = (u_int8_t)
		p_rfqual->pc_rssi_info[chn_idx_highest_enabled].rssi_sec20;

	params.bwinfo   = 0;
	params.tstamp   = 0;
	params.max_mag  = p_sfft->peak_mag;

	params.max_index    = p_sfft->peak_inx;
	params.max_exp      = 0;
	params.peak         = 0;
	params.bin_pwr_data = (u_int8_t *)pfft;
	params.freq         = p_sops->get_current_channel(spectral);
	params.freq_loading = 0;

	params.interf_list.count = 0;
	params.max_lower_index   = 0;
	params.max_upper_index   = 0;
	params.nb_lower          = 0;
	params.nb_upper          = 0;
	/*
	 * For modes upto VHT80, the noise floor is populated with the one
	 * corresponding
	 * to the highest enabled antenna chain
	 */
	params.noise_floor       =
		p_rfqual->noise_floor[chn_idx_highest_enabled];
	params.datalen           = ptlv->length;
	params.pwr_count         = ptlv->length -
		sizeof(SPECTRAL_PHYERR_HDR_GEN2) - segid_skiplen;
	params.tstamp            = (tsf64 & SPECTRAL_TSMASK);

	acs_stats->ctrl_nf             = params.noise_floor;
	acs_stats->ext_nf              = params.noise_floor;
	acs_stats->nfc_ctl_rssi        = control_rssi;
	acs_stats->nfc_ext_rssi        = extension_rssi;

	if (spectral->is_160_format &&
	    spectral->ch_width == CH_WIDTH_160MHZ) {
		/* We expect to see one more Search FFT report, and it should
		 * be equal in size to the current one.
		 */
		if (datalen < (2 * (sizeof(SPECTRAL_PHYERR_TLV_GEN2) +
			ptlv->length))) {
			spectral->diag_stats.spectral_sec80_sfft_insufflen++;
			return -EPERM;
		}

		ptlv_sec80 = (SPECTRAL_PHYERR_TLV_GEN2 *)(
					data +
					sizeof(SPECTRAL_PHYERR_TLV_GEN2) +
					ptlv->length);

		if (ptlv_sec80->signature != SPECTRAL_PHYERR_SIGNATURE_GEN2) {
			spectral->diag_stats.spectral_mismatch++;
			return -EPERM;
		}

		if (ptlv_sec80->tag != TLV_TAG_SEARCH_FFT_REPORT_GEN2) {
			spectral->diag_stats.spectral_no_sec80_sfft++;
			return -EPERM;
		}

		segid_sec80 = *((SPECTRAL_SEGID_INFO *)(
					(u_int8_t *)ptlv_sec80 +
					sizeof(SPECTRAL_PHYERR_TLV_GEN2) +
					sizeof(SPECTRAL_PHYERR_HDR_GEN2)));

		if (segid_sec80 != 1) {
		spectral->diag_stats.spectral_vhtseg2id_mismatch++;
		return -EPERM;
		}

		params.vhtop_ch_freq_seg1  = p_chaninfo->center_freq1;
		params.vhtop_ch_freq_seg2  = p_chaninfo->center_freq2;

		process_search_fft_report_gen2(
			ptlv_sec80,
			ptlv_sec80->length, &search_fft_info_sec80);

		pfft_sec80 = (SPECTRAL_PHYERR_FFT_GEN2 *)(
				((u_int8_t *)ptlv_sec80) +
				sizeof(SPECTRAL_PHYERR_TLV_GEN2) +
				sizeof(SPECTRAL_PHYERR_HDR_GEN2) +
				segid_skiplen);

		/* XXX: Confirm. TBD at SoD. */
		params.rssi_sec80           = p_rfqual->rssi_comb;
		if (spectral->is_sec80_rssi_war_required)
			params.rssi_sec80 =
				get_combined_rssi_sec80_segment_gen2(
					spectral,
					&search_fft_info_sec80);
		/* XXX: Determine dynamically. TBD at SoD. */
		/*
		 * For VHT80_80/VHT160, the noise floor for primary 80MHz
		 * segment is populated with the
		 * lowest enabled antenna chain and the noise floor for
		 * secondary 80MHz segment is populated
		 * with the highest enabled antenna chain
		 */
		params.noise_floor_sec80    =
			p_rfqual->noise_floor[chn_idx_highest_enabled];
		params.noise_floor          =
			p_rfqual->noise_floor[chn_idx_lowest_enabled];

		params.max_mag_sec80        = p_sfft_sec80->peak_mag;
		params.max_index_sec80      = p_sfft_sec80->peak_inx;
		/* XXX Does this definition of datalen *still hold? */
		params.datalen_sec80        = ptlv_sec80->length;
		params.pwr_count_sec80      =
			ptlv_sec80->length -
			sizeof(SPECTRAL_PHYERR_HDR_GEN2) -
			segid_skiplen;
		params.bin_pwr_data_sec80   = (u_int8_t *)pfft_sec80;
	}
	qdf_mem_copy(
		&params.classifier_params,
		 &spectral->classifier_params,
		 sizeof(struct spectral_classifier_params));

#ifdef SPECTRAL_DEBUG_SAMP_MSG
	target_if_dbg_print_SAMP_param(&params);
#endif
	target_if_spectral_create_samp_msg(spectral, &params);
	}

	return 0;
}

/*
 * Function     : spectral_dump_header_gen2
 * Description  : Dump Spectral header
 * Input        : Pointer to Spectral Phyerr Header
 * Output       : Success/Failure
 *
 */
int spectral_dump_header_gen2(SPECTRAL_PHYERR_HDR_GEN2 *phdr)
{
	u_int32_t a = 0;
	u_int32_t b = 0;

	qdf_mem_copy(&a, (u_int8_t *)phdr, sizeof(int));
	qdf_mem_copy(
		&b,
		(u_int8_t *)((u_int8_t *)phdr + sizeof(int)), sizeof(int));

	qdf_print("SPECTRAL : HEADER A 0x%x (%d)\n", a, a);
	qdf_print("SPECTRAL : HEADER B 0x%x (%d)\n", b, b);
	return 0;
}

/*
 * Function     : get_combined_rssi_sec80_segment_gen2
 * Description  : Get approximate combined RSSI for Secondary 80 segment
 * Input        : Pointer to spectral and  pointer to search fft info
 *                of secondary 80 segment
 * Output       : Combined RSSI for secondary 80Mhz segment
 *
 */
int8_t get_combined_rssi_sec80_segment_gen2(
	struct target_if_spectral *spectral,
	 SPECTRAL_SEARCH_FFT_INFO_GEN2 *p_sfft_sec80)
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

/*
 * Function     : spectral_dump_tlv_gen2
 * Description  : Dump Spectral TLV
 * Input        : Pointer to Spectral Phyerr TLV, flag indicating whether
 *                information provided by HW is in altered format for 802.11ac
 *                160/80+80 MHz support (QCA9984 onwards).
 * Output       : Success/Failure
 *
 */
int spectral_dump_tlv_gen2(SPECTRAL_PHYERR_TLV_GEN2 *ptlv, bool is_160_format)
{
	int ret = 0;

	/* TODO : Do not delete the following print
	 *        The scripts used to validate Spectral depend on this Print
	 */
	qdf_print("SPECTRAL : TLV Length is 0x%x (%d)\n",
		  ptlv->length, ptlv->length);

	switch (ptlv->tag) {
	case TLV_TAG_SPECTRAL_SUMMARY_REPORT_GEN2:
		ret = dump_summary_report_gen2(
			ptlv, ptlv->length, is_160_format);
		break;

	case TLV_TAG_SEARCH_FFT_REPORT_GEN2:
		ret = dump_search_fft_report_gen2(
			ptlv, ptlv->length, is_160_format);
		break;

	case TLV_TAG_ADC_REPORT_GEN2:
		ret = dump_adc_report_gen2(
			ptlv, ptlv->length);
		break;

	default:
		qdf_print("SPECTRAL : INVALID TLV\n");
		ret = -1;
		break;
	}

	return ret;
}

/*
 * Function     : spectral_dump_phyerr_data_gen2
 * Description  : Dump Spectral related PHY Error TLVs
 * Input        : Pointer to buffer, flag indicating whether information
 *                provided by HW is in altered format for 802.11ac 160/80+80 MHz
 *                support (QCA9984 onwards).
 * Output       : Success/Failure
 *
 */
int spectral_dump_phyerr_data_gen2(u_int8_t *data, u_int32_t datalen,
				   bool is_160_format)
{
	SPECTRAL_PHYERR_TLV_GEN2 *ptlv = NULL;
	u_int32_t bytes_processed = 0;
	u_int32_t bytes_remaining = datalen;
	u_int32_t curr_tlv_complete_size = 0;

	if (datalen < sizeof(SPECTRAL_PHYERR_TLV_GEN2)) {
	qdf_print("DRIVER: Total PHY error data length %u too short to contain"
		  " any TLVs\n", datalen);
	return -EPERM;
	}

	while (bytes_processed < datalen) {
	if (bytes_remaining < sizeof(SPECTRAL_PHYERR_TLV_GEN2)) {
		qdf_print("DRIVER: Remaining PHY error data length %u too "
			  "short to contain a TLV\n", bytes_remaining);
		return -EPERM;
	}

	ptlv = (SPECTRAL_PHYERR_TLV_GEN2 *)(data + bytes_processed);

	if (ptlv->signature != SPECTRAL_PHYERR_SIGNATURE_GEN2) {
		qdf_print("DRIVER : Invalid signature 0x%x!\n",
			  ptlv->signature);
		return -EPERM;
	}

	curr_tlv_complete_size = sizeof(SPECTRAL_PHYERR_TLV_GEN2) +
		ptlv->length;

	if (curr_tlv_complete_size > bytes_remaining) {
		qdf_print("DRIVER : Current indicated complete TLV size %u "
			  "greater than number of bytes remaining to be "
			  "processed %u",
			  curr_tlv_complete_size,
			  bytes_remaining);
		return -EPERM;
	}

	if (spectral_dump_tlv_gen2(ptlv, is_160_format) == -1)
		return -EPERM;

	bytes_processed += curr_tlv_complete_size;
	bytes_remaining = datalen - bytes_processed;
	}

	return 0;
}

/* END of spectral GEN II HW specific functions */

/* START of spectral GEN III HW specific functions */

/*
 * Function     : process_search_fft_report_gen3
 * Description  : Process Search FFT Report
 * Input        : Pointer to Spectral fft report and pointer to search fft info
 * Output       : Success/Failure
 *
 */
int process_search_fft_report_gen3(
	struct spectral_phyerr_fft_report_gen3 *p_fft_report,
	 struct spectral_search_fft_info_gen3 *p_sfft)
{
	/* For simplicity, everything is defined as uint32_t (except one).
	 * Proper code will later use the right sizes.
	 */
	/* For easy comparision between MDK team and OS team, the MDK script
	 * variable names have been used
	 */
	int32_t peak_sidx;
	int32_t peak_mag;

	/* Populate the Search FFT Info */
	if (p_sfft) {
		p_sfft->timestamp       = p_fft_report->fft_timestamp;

		p_sfft->fft_detector_id = get_bitfield(
			p_fft_report->hdr_a,
			 2,
			 0);
		p_sfft->fft_num         = get_bitfield(
			p_fft_report->hdr_a,
			 3,
			 2);
		p_sfft->fft_radar_check = get_bitfield(
			p_fft_report->hdr_a,
			 12,
			 5);

		peak_sidx              = get_bitfield(
			p_fft_report->hdr_a,
			 11,
			 17);
		p_sfft->fft_peak_sidx  = unsigned_to_signed(peak_sidx, 11);
		p_sfft->fft_chn_idx    = get_bitfield(
			p_fft_report->hdr_a,
			 3,
			 28);

		p_sfft->fft_base_pwr_db   = get_bitfield(
			p_fft_report->hdr_b,
			 9,
			 0);
		p_sfft->fft_total_gain_db = get_bitfield(
			p_fft_report->hdr_b,
			 8,
			 9);

		p_sfft->fft_num_str_bins_ib = get_bitfield(
			p_fft_report->hdr_c,
			 8,
			 0);
		peak_mag                    = get_bitfield(
			p_fft_report->hdr_c,
			 10,
			 8);
		p_sfft->fft_peak_mag        = unsigned_to_signed(peak_mag, 10);
		p_sfft->fft_avgpwr_db       = get_bitfield(
			p_fft_report->hdr_c,
			 7,
			 18);
		p_sfft->fft_relpwr_db       = get_bitfield(
			p_fft_report->hdr_c,
			 7,
			 25);
	}

	return 0;
}

/*
 * Function     : spectral_dump_fft_report_gen3
 * Description  : Process Search FFT Report
 * Input        : Pointer to Spectral fft report and pointer to search fft info
 * Output       : Success/Failure
 *
 */
int spectral_dump_fft_report_gen3(
	struct spectral_phyerr_fft_report_gen3 *p_fft_report,
				  struct spectral_search_fft_info_gen3 *p_sfft)
{
	int i = 0;
	int fft_mag = 0;
	int fft_hdr_length = (p_fft_report->fft_hdr_length * 4);
	int report_len     = (fft_hdr_length + 8);
	int fft_bin_len    = (fft_hdr_length - 16);

	qdf_print("##############################################"
		  "###############\n");
	qdf_print("Spectral search fft_report\n");
	qdf_print("fft_timestamp  = 0x%x\n"
			"fft_hdr_length = %d(32 bit words)\n"
			"fft_hdr_tag    = 0x%x\n"
			"fft_hdr_sig    = 0x%x\n",
			p_fft_report->fft_timestamp,
			p_fft_report->fft_hdr_length,
			p_fft_report->fft_hdr_tag,
			p_fft_report->fft_hdr_sig);

	qdf_print("Length field in search fft report is %d(0x%x) bytes\n",
		  fft_hdr_length, fft_hdr_length);
	qdf_print("Total length of search fft report is %d(0x%x) bytes\n",
		  report_len, report_len);
	qdf_print("Number of fftbins in report is %d(0x%x)\n", fft_bin_len,
		  fft_bin_len);

	qdf_print("fft_detector_id     = %u\n"
			"fft_num             = %u\n"
			"fft_radar_check     = %u\n"
			"fft_peak_sidx       = %d\n"
			"fft_chn_idx         = %u\n"
			"fft_base_pwr_db     = %u\n"
			"fft_total_gain_db   = %u\n"
			"fft_num_str_bins_ib = %u\n"
			"fft_peak_mag        = %d\n"
			"fft_avgpwr_db       = %u\n"
			"fft_relpwr_db       = %u\n",
			p_sfft->fft_detector_id,
			p_sfft->fft_num,
			p_sfft->fft_radar_check,
			p_sfft->fft_peak_sidx,
			p_sfft->fft_chn_idx,
			p_sfft->fft_base_pwr_db,
			p_sfft->fft_total_gain_db,
			p_sfft->fft_num_str_bins_ib,
			p_sfft->fft_peak_mag,
			p_sfft->fft_avgpwr_db,
			p_sfft->fft_relpwr_db);

	qdf_print("FFT bins:\n");
	for (i = 0; i < (fft_hdr_length - 16); i++) {
	fft_mag = ((u_int8_t *)p_fft_report)[SPECTRAL_FFT_BINS_POS + i];
	qdf_print("%d: %d, ", i, fft_mag);
	if (i % 16 == 0)
		qdf_print("\n");
	}
	qdf_print("\n");
	qdf_print("###########################################################"
		  "##\n");

	return 0;
}

static int consume_searchfft_report_gen3(
	struct target_if_spectral *spectral,
	 struct phyerr_info *pinfo)
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

	/* Unpack the arguments */
	u_int32_t datalen = pinfo->datalen;
	struct target_if_spectral_rfqual_info *p_rfqual = pinfo->p_rfqual;
	struct target_if_spectral_chan_info *p_chaninfo = pinfo->p_chaninfo;
	u_int64_t tsf64 = pinfo->tsf64;
	struct target_if_spectral_acs_stats *acs_stats = pinfo->acs_stats;

	struct target_if_samp_msg_params params;
	struct spectral_search_fft_info_gen3 search_fft_info;
	struct spectral_search_fft_info_gen3 *p_sfft = &search_fft_info;
	int8_t rssi_up = 0;
	int8_t rssi_low = 0;
	int8_t chn_idx_highest_enabled = 0;
	int8_t chn_idx_lowest_enabled  = 0;
	u_int8_t control_rssi   = 0;
	u_int8_t extension_rssi = 0;
	u_int32_t tstamp    = 0;
	int fft_hdr_length = 0;
	int report_len  = 0;
	int fft_bin_len = 0;
	struct target_if_spectral_ops *p_sops = GET_TIF_SPECTRAL_OPS(spectral);
	struct spectral_phyerr_fft_report_gen3 *p_fft_report =
		(struct spectral_phyerr_fft_report_gen3 *)(pinfo->data);

	OS_MEMZERO(&params, sizeof(params));

	fft_hdr_length = p_fft_report->fft_hdr_length * 4;
	if (fft_hdr_length < 16) {
	qdf_print("SPECTRAL : Unexpected TLV length %u for FFT Report! Hexdump"
		  " follows\n", fft_hdr_length);
	goto fail;
	}

	report_len     = (fft_hdr_length + 8);
	fft_bin_len    = (fft_hdr_length - 16);
	if (datalen < report_len) {
		qdf_print("DRIVER: Total PHY error data length %u too short to"
			  " contain the search fft report of length %u\n",
			  datalen, report_len);
		goto fail;
	}

	process_search_fft_report_gen3(p_fft_report, p_sfft);

	if (p_sfft->fft_detector_id != 0) {
		qdf_print("Expected segid is 0 but we got %d\n",
			  p_sfft->fft_detector_id);
		spectral->diag_stats.spectral_vhtseg1id_mismatch++;
		goto fail;
	}

	if (spectral_debug_level & (ATH_DEBUG_SPECTRAL2 | ATH_DEBUG_SPECTRAL4))
		spectral_dump_fft_report_gen3(p_fft_report, p_sfft);

	tstamp = p_sops->get_tsf64(spectral) & SPECTRAL_TSMASK;

	if (spectral->upper_is_control)
		rssi_up = control_rssi;
	else
		rssi_up = extension_rssi;

	if (spectral->lower_is_control)
		rssi_low = control_rssi;
	else
		rssi_low = extension_rssi;

	params.rssi         = p_rfqual->rssi_comb;
	params.lower_rssi   = rssi_low;
	params.upper_rssi   = rssi_up;

	if (spectral->sc_spectral_noise_pwr_cal) {
	/* qdf_print("SPECTRAL : Needs to verified with FW\n"); */
	params.chain_ctl_rssi[0] = p_rfqual->pc_rssi_info[0].rssi_pri20;
	params.chain_ctl_rssi[1] = p_rfqual->pc_rssi_info[1].rssi_pri20;
	params.chain_ctl_rssi[2] = p_rfqual->pc_rssi_info[2].rssi_pri20;
	params.chain_ext_rssi[0] = p_rfqual->pc_rssi_info[0].rssi_sec20;
	params.chain_ext_rssi[1] = p_rfqual->pc_rssi_info[1].rssi_sec20;
	params.chain_ext_rssi[2] = p_rfqual->pc_rssi_info[2].rssi_sec20;
	}

	/*
	 * XXX : This actually depends on the programmed chain mask
	 *       There are three chains in Peregrine and 4 chains in Beeliner &
	 *       Cascade
	 *       This value decides the per-chain enable mask to select
	 *       the input ADC for search FTT.
	 *       For modes upto VHT80, if more than one chain is enabled, the
	 *       max valid chain
	 *       is used. LSB corresponds to chain zero.
	 *       For VHT80_80 and VHT160, the lowest enabled chain is used for
	 *       primary
	 *       detection and highest enabled chain is used for secondary
	 *       detection.
	 *
	 *  XXX: The current algorithm do not use these control and extension
	 *       channel
	 *       Instead, it just relies on the combined RSSI values only.
	 *       For fool-proof detection algorithm, we should take these RSSI
	 *       values
	 *       in to account. This is marked for future enhancements.
	 */
	/* qdf_print("SPECTRAL : TBD (Chainmask changes for 8x8)\n"); */
	chn_idx_highest_enabled = ((spectral->params.ss_chn_mask & 0x8) ? 3 :
		(spectral->params.ss_chn_mask & 0x4) ? 2 :
		(spectral->params.ss_chn_mask & 0x2) ? 1 : 0);
	chn_idx_lowest_enabled  = ((spectral->params.ss_chn_mask & 0x1) ? 0 :
		(spectral->params.ss_chn_mask & 0x2) ? 1 :
		(spectral->params.ss_chn_mask & 0x4) ? 2 : 3);
	control_rssi    = (u_int8_t)
		p_rfqual->pc_rssi_info[chn_idx_highest_enabled].rssi_pri20;
	extension_rssi  = (u_int8_t)
		p_rfqual->pc_rssi_info[chn_idx_highest_enabled].rssi_sec20;

	params.bwinfo   = 0;
	params.tstamp   = 0;
	params.max_mag  = p_sfft->fft_peak_mag;

	/* params.max_index    = p_sfft->peak_inx; */
	params.max_exp      = 0;
	params.peak         = 0;
	params.bin_pwr_data = (u_int8_t *)((uint8_t *)p_fft_report +
		SPECTRAL_FFT_BINS_POS);
	params.freq         = p_sops->get_current_channel(spectral);
	params.freq_loading = 0;

	params.interf_list.count = 0;
	params.max_lower_index   = 0;
	params.max_upper_index   = 0;
	params.nb_lower          = 0;
	params.nb_upper          = 0;
	/*
	 * For modes upto VHT80, the noise floor is populated with the one
	 * corresponding
	 * to the highest enabled antenna chain
	 */
	params.noise_floor       =
		p_rfqual->noise_floor[chn_idx_highest_enabled];
	params.datalen           = (fft_hdr_length * 4);
	params.pwr_count         = fft_bin_len;
	params.tstamp            = (tsf64 & SPECTRAL_TSMASK);

	acs_stats->ctrl_nf             = params.noise_floor;
	acs_stats->ext_nf              = params.noise_floor;
	acs_stats->nfc_ctl_rssi        = control_rssi;
	acs_stats->nfc_ext_rssi        = extension_rssi;

	if (spectral->ch_width == CH_WIDTH_160MHZ) {
	/* We expect to see one more Search FFT report, and it should be
	 * equal in size to the current one.
	 */
	if (datalen != (2 * report_len)) {
		spectral->diag_stats.spectral_sec80_sfft_insufflen++;
		goto fail;
	}

	p_fft_report = (struct spectral_phyerr_fft_report_gen3 *)(
			(uint8_t *)p_fft_report + report_len);

	if (p_fft_report->fft_hdr_sig != SPECTRAL_PHYERR_SIGNATURE_GEN3) {
		spectral->diag_stats.spectral_mismatch++;
		goto fail;
	}

	if (p_fft_report->fft_hdr_tag != TLV_TAG_SEARCH_FFT_REPORT_GEN3) {
		spectral->diag_stats.spectral_no_sec80_sfft++;
		goto fail;
	}

	fft_hdr_length = p_fft_report->fft_hdr_length * 4;
	report_len     = (fft_hdr_length + 8);
	fft_bin_len    = (fft_hdr_length - 16);

	process_search_fft_report_gen3(p_fft_report,  p_sfft);

	if (p_sfft->fft_detector_id != 1) {
		qdf_print("Expected segid is 1 but we got %d\n",
			  p_sfft->fft_detector_id);
		spectral->diag_stats.spectral_vhtseg2id_mismatch++;
		goto fail;
	}

	if (spectral_debug_level & (ATH_DEBUG_SPECTRAL2 | ATH_DEBUG_SPECTRAL4))
		spectral_dump_fft_report_gen3(p_fft_report, p_sfft);

	params.vhtop_ch_freq_seg1  = p_chaninfo->center_freq1;
	params.vhtop_ch_freq_seg2  = p_chaninfo->center_freq2;

	/* XXX: Confirm. TBD at SoD. */
	params.rssi_sec80           = p_rfqual->rssi_comb;

	/* if (spectral->is_sec80_rssi_war_required) { */
	/* params.rssi_sec80 = get_combined_rssi_sec80_segment_gen3(spectral,
	 * &search_fft_info_sec80);
	 */
	/* } */
	/* XXX: Determine dynamically. TBD at SoD. */

	/*
	 * For VHT80_80/VHT160, the noise floor for primary 80MHz segment is
	 * populated with the
	 * lowest enabled antenna chain and the noise floor for secondary 80MHz
	 * segment is populated
	 * with the highest enabled antenna chain
	 */
	params.noise_floor_sec80    =
		p_rfqual->noise_floor[chn_idx_highest_enabled];
	params.noise_floor          =
		p_rfqual->noise_floor[chn_idx_lowest_enabled];

	params.max_mag_sec80        = p_sfft->fft_peak_mag;
	/* params.max_index_sec80      = p_sfft->peak_inx; */
	/* XXX Does this definition of datalen *still hold? */
	params.datalen_sec80        = fft_hdr_length;
	params.pwr_count_sec80      = fft_bin_len;
	params.bin_pwr_data_sec80   = (u_int8_t *)(
		(uint8_t *)p_fft_report + SPECTRAL_FFT_BINS_POS);
	}

	qdf_mem_copy(
		&params.classifier_params,
		 &spectral->classifier_params,
		 sizeof(struct spectral_classifier_params));
#ifdef SPECTRAL_DEBUG_SAMP_MSG
	target_if_dbg_print_SAMP_param(&params);
#endif
	target_if_spectral_create_samp_msg(spectral, &params);

	return 0;

fail:
	qdf_print("Error in function %s while processing search fft report\n",
		  __func__);
	print_buf((uint8_t *)p_fft_report, fft_hdr_length + 8);
	return -EPERM;
}

/*
 * Function     : spectral_process_phyerr_gen3
 * Descrip_tion  : Process PHY Error
 * Input        : Pointer to buffer
 * Output       : Success/Failure
 *
 */
int spectral_process_phyerr_gen3(
	struct target_if_spectral *spectral,
	u_int8_t *data,
	u_int32_t datalen, struct target_if_spectral_rfqual_info *p_rfqual,
	struct target_if_spectral_chan_info *p_chaninfo,
	u_int64_t tsf64,
	struct target_if_spectral_acs_stats *acs_stats)
{
	uint8_t tag = 0;
	uint8_t signature = 0;
	struct phyerr_info pinfo;
	int ret = 0;

	/* pack arguments in a structure as there are
	 * more than 3 arguments
	 */
	pinfo.data = data;
	pinfo.datalen = datalen;
	pinfo.p_rfqual = p_rfqual;
	pinfo.p_chaninfo = p_chaninfo;
	pinfo.tsf64 = tsf64;
	pinfo.acs_stats = acs_stats;

	if (spectral_debug_level &
	    (ATH_DEBUG_SPECTRAL2 | ATH_DEBUG_SPECTRAL4)) {
		qdf_print("Printing the spectral phyerr buffer for debug "
			  "purpose\n");
		qdf_print("Dalalength of buffer = 0x%x(%d)\n",
			  datalen, datalen);
#ifdef CONFIG_WIN
		RAWSIM_PKT_HEXDUMP(data, datalen);
#endif
	}

	/* Peek into the data to figure out whether
	 *      1) Signature matches the expected value
	 *      2) What is inside the package (TAG ID is used for finding this)
	 */
	tag = *(data + PHYERR_HDR_TAG_POS);
	signature = *(data + PHYERR_HDR_SIG_POS);

	if (signature != SPECTRAL_PHYERR_SIGNATURE_GEN3) {
		qdf_print("Unexpected signature %x in spectral phyerror "
			  "event\n", signature);
		spectral->diag_stats.spectral_mismatch++;
		ret = -1;
		goto end;
	}

	switch (tag) {
	case TLV_TAG_SPECTRAL_SUMMARY_REPORT_GEN3:
		/* Place holder
		 * We don't use spectral scan report as of now
		 */
		break;

	case TLV_TAG_SEARCH_FFT_REPORT_GEN3:
		ret = consume_searchfft_report_gen3(spectral, &pinfo);
		break;

	default:
		qdf_print("Unknown tag %x in spectral phyerror event\n", tag);
		break;
	}

end:
	if (spectral_debug_level & ATH_DEBUG_SPECTRAL4)
		spectral_debug_level = ATH_DEBUG_SPECTRAL;
	return ret;
}

/* END of spectral GEN III HW specific functions */

#endif  /* WLAN_SPECTRAL_ENABLE */
