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
#include <qdf_types.h>
#include <qdf_module.h>
#include <wlan_tgt_def_config.h>
#include <hif.h>
#include <hif_hw_version.h>
#include <wmi_unified_api.h>
#include <target_if_spectral.h>
#include <wlan_lmac_if_def.h>
#include <wlan_osif_priv.h>
#include <reg_services_public_struct.h>
#include <target_if.h>
#ifdef DIRECT_BUF_RX_ENABLE
#include <target_if_direct_buf_rx_api.h>
#endif
extern int spectral_debug_level;

#ifdef WLAN_CONV_SPECTRAL_ENABLE

#define SPECTRAL_HEXDUMP_OCTET_PRINT_SIZE           (3)
#define SPECTRAL_HEXDUMP_NUM_OCTETS_PER_LINE        (16)
#define SPECTRAL_HEXDUMP_EXTRA_BUFFER_PER_LINE      (16)

/*
 * Provision for the expected hexdump line size as follows:
 *
 * Size per octet multiplied by number of octets per line
 * +
 * ASCII representation which is equivalent in print size to number of octets
 * per line
 * +
 * Some extra buffer
 */
#define SPECTRAL_HEXDUMP_LINESIZE                           \
		((SPECTRAL_HEXDUMP_OCTET_PRINT_SIZE *       \
		  SPECTRAL_HEXDUMP_NUM_OCTETS_PER_LINE) +   \
		 SPECTRAL_HEXDUMP_NUM_OCTETS_PER_LINE +     \
		 SPECTRAL_HEXDUMP_EXTRA_BUFFER_PER_LINE)

/**
 * target_if_spectral_hexdump() - Print hexdump of the given buffer
 * @_buf: Pointer to buffer
 * @_len: Length of the buffer
 *
 * Print the hexdump of buffer upto given length. Print upto
 * SPECTRAL_HEXDUMP_NUM_OCTETS_PER_LINE per line, followed by the ASCII
 * representation of these octets.
 */
static inline void target_if_spectral_hexdump(unsigned char *_buf, int _len)
{
	int i, mod;
	unsigned char ascii[SPECTRAL_HEXDUMP_NUM_OCTETS_PER_LINE + 1];
	unsigned char *pc = (_buf);
	char hexdump_line[SPECTRAL_HEXDUMP_LINESIZE + 1];
	int loc = 0;

	qdf_mem_zero(hexdump_line, sizeof(hexdump_line));

	if (_len <= 0) {
		spectral_err("buffer len is %d, too short", _len);
		return;
	}

	for (i = 0; i < _len; i++) {
		mod = i % SPECTRAL_HEXDUMP_NUM_OCTETS_PER_LINE;

		if (!mod) {
			if (i) {
				qdf_assert_always(loc < sizeof(hexdump_line));
				loc += snprintf(&hexdump_line[loc],
						sizeof(hexdump_line) - loc,
						"  %s", ascii);
				spectral_debug("%s", hexdump_line);
				qdf_mem_zero(hexdump_line,
					     sizeof(hexdump_line));
				loc = 0;
			}
		}

		qdf_assert_always(loc < sizeof(hexdump_line));
		loc += snprintf(&hexdump_line[loc], sizeof(hexdump_line) - loc,
				" %02x", pc[i]);

		if ((pc[i] < 0x20) || (pc[i] > 0x7e))
			ascii[mod] = '.';
		else
			ascii[mod] = pc[i];
		ascii[(mod) + 1] = '\0';
	}

	while ((i % SPECTRAL_HEXDUMP_NUM_OCTETS_PER_LINE) != 0) {
		qdf_assert_always(loc < sizeof(hexdump_line));
		loc += snprintf(&hexdump_line[loc], sizeof(hexdump_line) - loc,
				"   ");
		i++;
	}

	qdf_assert_always(loc < sizeof(hexdump_line));
	snprintf(&hexdump_line[loc], sizeof(hexdump_line) - loc, "  %s", ascii);
	spectral_debug("%s", hexdump_line);
}

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

QDF_STATUS target_if_spectral_fw_hang(struct target_if_spectral *spectral)
{
	struct crash_inject param;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc;
	struct target_if_psoc_spectral *psoc_spectral;

	if (!spectral) {
		spectral_err("Spectral LMAC object is null");
		return QDF_STATUS_E_INVAL;
	}

	pdev = spectral->pdev_obj;
	if (!pdev) {
		spectral_err("pdev is null");
		return QDF_STATUS_E_FAILURE;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		spectral_err("psoc is null");
		return QDF_STATUS_E_FAILURE;
	}

	psoc_spectral = get_target_if_spectral_handle_from_psoc(psoc);
	if (!psoc_spectral) {
		spectral_err("spectral psoc object is null");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_set(&param, sizeof(param), 0);
	param.type = 1; //RECOVERY_SIM_ASSERT

	return psoc_spectral->wmi_ops.wmi_spectral_crash_inject(
		GET_WMI_HDL_FROM_PDEV(spectral->pdev_obj), &param);
}

#ifdef OPTIMIZED_SAMP_MESSAGE
void
target_if_dbg_print_samp_msg(struct spectral_samp_msg *ss_msg)
{
	int span, det;
	struct samp_detector_info *det_info;
	struct samp_freq_span_info *span_info;

	spectral_dbg_line();
	spectral_debug("Spectral Message");
	spectral_dbg_line();
	spectral_debug("Signature        :   0x%x", ss_msg->signature);
	spectral_debug("Freq             :   %u", ss_msg->pri20_freq);
	spectral_debug("sscan width      :   %d", ss_msg->sscan_bw);
	spectral_debug("sscan cfreq1     :   %u", ss_msg->sscan_cfreq1);
	spectral_debug("sscan cfreq2     :   %u", ss_msg->sscan_cfreq2);
	spectral_debug("bin power count  :   %d", ss_msg->bin_pwr_count);
	spectral_debug("Number of spans  :   %d", ss_msg->num_freq_spans);
	spectral_dbg_line();
	for (span = 0; span < ss_msg->num_freq_spans; span++) {
		span_info = &ss_msg->freq_span_info[span];
		spectral_debug("-------- Span ID : %d --------", span);
		spectral_debug("Number of detectors  :  %d",
			       span_info->num_detectors);
		spectral_dbg_line();
		for (det = 0; det < span_info->num_detectors; det++) {
			det_info = &span_info->detector_info[det];
			spectral_debug("------ Detector ID : %d ------", det);
			spectral_dbg_line();
			spectral_debug("RSSI            : %d", det_info->rssi);
			spectral_debug("Timestamp       : %u",
				       det_info->timestamp);
			spectral_debug("Start bin index : %d",
				       det_info->start_bin_idx);
			spectral_debug("End bin index   : %d",
				       det_info->end_bin_idx);
			spectral_debug("Start frequency : %d",
				       det_info->start_frequency);
			spectral_debug("End frequency   : %d",
				       det_info->end_frequency);
			spectral_dbg_line();
		}
	}
}
#else
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
#endif /* OPTIMIZED_SAMP_MESSAGE */

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
	case CH_WIDTH_80P80MHZ:
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

#ifndef OPTIMIZED_SAMP_MESSAGE
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
#endif /* OPTIMIZED_SAMP_MESSAGE */

#ifdef OPTIMIZED_SAMP_MESSAGE
/**
 * target_if_spectral_unify_cfreq_format() - Unify the cfreq representations.
 * @spectral: Pointer to target_if spectral internal structure
 * @cfreq1: cfreq1 value received in the Spectral report
 * @cfreq2: cfreq2 value received in the Spectral report
 * @pri20_freq: Primary 20MHz frequency of operation
 * @ch_width: channel width. If the center frequencies are of operating channel,
 * pass the operating channel width, else pass the sscan channel width.
 * @smode: Spectral scan mode
 *
 * This API converts the cfreq1 and cfreq2 representations as follows.
 * For a contiguous channel, cfreq1 will represent the center of the entire
 * span and cfreq2 will be 0. For a discontiguous channel like 80p80, cfreq1
 * will represent the center of primary segment whereas cfreq2 will
 * represent the center of secondary segment.
 *
 * Return: Success/Failure
 */
static QDF_STATUS
target_if_spectral_unify_cfreq_format(struct target_if_spectral *spectral,
				      uint32_t *cfreq1, uint32_t *cfreq2,
				      uint32_t pri20_freq,
				      enum phy_ch_width ch_width,
				      enum spectral_scan_mode smode)

{
	uint32_t reported_cfreq1, reported_cfreq2;
	struct wlan_objmgr_psoc *psoc;

	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!spectral->pdev_obj) {
		spectral_err_rl("Spectral PDEV is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc = wlan_pdev_get_psoc(spectral->pdev_obj);
	if (!psoc) {
		spectral_err_rl("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	reported_cfreq1 = *cfreq1;
	reported_cfreq2 = *cfreq2;

	if (ch_width == CH_WIDTH_160MHZ &&
	    spectral->rparams.fragmentation_160[smode]) {
		/* cfreq should be 0 for 160MHz as it is contiguous */
		*cfreq2 = 0;

		/**
		 * For gen3 chipsets that use fragmentation, cfreq1 is center of
		 * pri80, and cfreq2 is center of sec80. Averaging them gives
		 * the center of the 160MHz span.
		 * Whereas gen2 chipsets report the center of the 160MHz span in
		 * cfreq2 itself.
		 */
		if (spectral->spectral_gen == SPECTRAL_GEN3)
			*cfreq1 = (reported_cfreq1 + reported_cfreq2) >> 1;
		else
			*cfreq1 = reported_cfreq2;
	} else if (ch_width == CH_WIDTH_80P80MHZ &&
		   wlan_psoc_nif_fw_ext_cap_get(
		   psoc, WLAN_SOC_RESTRICTED_80P80_SUPPORT)) {
			/* In restricted 80p80 case */
			const struct bonded_channel_freq
					*bonded_chan_ptr = NULL;
			enum channel_state state;

			/* Get the 80MHz channel containing the pri20 freq */
			state = wlan_reg_get_5g_bonded_channel_and_state_for_freq
				(spectral->pdev_obj, pri20_freq, CH_WIDTH_80MHZ,
				 &bonded_chan_ptr);

			if (state == CHANNEL_STATE_DISABLE ||
			    state == CHANNEL_STATE_INVALID) {
				spectral_err_rl("Channel state is disable or invalid");
				return QDF_STATUS_E_FAILURE;
			}

			if (!bonded_chan_ptr) {
				spectral_err_rl("Bonded channel is not found");
				return QDF_STATUS_E_FAILURE;
			}

			/* cfreq1 is the center of the pri80 segment */
			*cfreq1 = (bonded_chan_ptr->start_freq +
				   bonded_chan_ptr->end_freq) >> 1;

			/**
			 * cfreq2 is 85MHz away from cfreq1. Whether it is
			 * higher or lower depends on pri20_freq's relationship
			 * with the reported center frequency.
			 */
			if (pri20_freq < reported_cfreq1)
				*cfreq2 = *cfreq1 + FREQ_OFFSET_85MHZ;
			else
				*cfreq2 = *cfreq1 - FREQ_OFFSET_85MHZ;
	} else {
		/* All other cases are reporting the cfreq properly */
		*cfreq1 = reported_cfreq1;
		*cfreq2 = reported_cfreq2;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_populate_det_start_end_freqs() - Populate the start and end
 * frequencies, on per-detector level.
 * @spectral: Pointer to target_if spectral internal structure
 * @smode: Spectral scan mode
 *
 * Populate the start and end frequencies, on per-detector level.
 *
 * Return: Success/Failure
 */
static QDF_STATUS
target_if_populate_det_start_end_freqs(struct target_if_spectral *spectral,
				       enum spectral_scan_mode smode)
{
	struct per_session_report_info *rpt_info;
	struct per_session_det_map *det_map;
	struct per_session_dest_det_info *dest_det_info;
	enum phy_ch_width ch_width;
	struct sscan_detector_list *detector_list;
	bool is_fragmentation_160;
	uint8_t det;
	uint32_t cfreq;
	uint32_t start_end_freq_arr[2];

	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("Invalid Spectral mode");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&spectral->session_report_info_lock);

	ch_width = spectral->report_info[smode].sscan_bw;
	is_fragmentation_160 = spectral->rparams.fragmentation_160[smode];

	rpt_info = &spectral->report_info[smode];

	qdf_spin_lock_bh(&spectral->detector_list_lock);
	detector_list = &spectral->detector_list[smode][ch_width];

	for (det = 0; det < detector_list->num_detectors; det++) {
		qdf_spin_lock_bh(&spectral->session_det_map_lock);
		det_map = &spectral->det_map
				[detector_list->detectors[det]];
		dest_det_info = &det_map->dest_det_info[0];

		switch (det) {
		case 0:
			if (ch_width == CH_WIDTH_160MHZ &&
			    is_fragmentation_160) {
				if (rpt_info->pri20_freq <
				    rpt_info->sscan_cfreq1)
					cfreq = rpt_info->sscan_cfreq1 -
						FREQ_OFFSET_40MHZ;
				else
					cfreq = rpt_info->sscan_cfreq1 +
						FREQ_OFFSET_40MHZ;
			} else
				cfreq = rpt_info->sscan_cfreq1;
			break;

		case 1:
			if (ch_width == CH_WIDTH_160MHZ &&
			    is_fragmentation_160) {
				if (rpt_info->pri20_freq <
				    rpt_info->sscan_cfreq1)
					cfreq = rpt_info->sscan_cfreq1 +
						FREQ_OFFSET_40MHZ;
				else
					cfreq = rpt_info->sscan_cfreq1 -
						FREQ_OFFSET_40MHZ;
			} else
				cfreq = rpt_info->sscan_cfreq2;
			break;

		default:
			qdf_spin_unlock_bh(&spectral->session_det_map_lock);
			qdf_spin_unlock_bh(&spectral->detector_list_lock);
			qdf_spin_unlock_bh(
					&spectral->session_report_info_lock);

			return QDF_STATUS_E_FAILURE;
		}

		/* Set start and end frequencies */
		target_if_spectral_set_start_end_freq(cfreq,
						      ch_width,
						      is_fragmentation_160,
						      start_end_freq_arr);
		dest_det_info->start_freq = start_end_freq_arr[0];
		dest_det_info->end_freq = start_end_freq_arr[1];

		qdf_spin_unlock_bh(&spectral->session_det_map_lock);
	}

	qdf_spin_unlock_bh(&spectral->detector_list_lock);
	qdf_spin_unlock_bh(&spectral->session_report_info_lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_populate_fft_bins_info(struct target_if_spectral *spectral,
				 enum spectral_scan_mode smode)
{
	struct per_session_det_map *det_map;
	struct per_session_dest_det_info *dest_det_info;
	enum phy_ch_width ch_width;
	struct sscan_detector_list *detector_list;
	bool is_fragmentation_160;
	uint8_t spectral_fft_size;
	uint8_t rpt_mode;
	uint32_t num_fft_bins;
	uint16_t start_bin;
	uint8_t det;

	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("Invalid Spectral mode");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&spectral->session_report_info_lock);

	ch_width = spectral->report_info[smode].sscan_bw;
	is_fragmentation_160 = spectral->rparams.fragmentation_160[smode];
	spectral_fft_size = spectral->params[smode].ss_fft_size;
	rpt_mode = spectral->params[smode].ss_rpt_mode;
	num_fft_bins =
		target_if_spectral_get_num_fft_bins(spectral_fft_size,
						    rpt_mode);
	if (num_fft_bins < 0) {
		qdf_spin_unlock_bh(&spectral->session_report_info_lock);
		spectral_err_rl("Invalid number of FFT bins %d",
				num_fft_bins);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&spectral->detector_list_lock);
	detector_list = &spectral->detector_list[smode][ch_width];

	for (det = 0; det < detector_list->num_detectors; det++) {
		uint16_t lb_extrabins_offset = 0;

		qdf_spin_lock_bh(&spectral->session_det_map_lock);
		det_map = &spectral->det_map
				[detector_list->detectors[det]];
		dest_det_info = &det_map->dest_det_info[0];
		dest_det_info->lb_extrabins_num = spectral->lb_edge_extrabins;
		dest_det_info->rb_extrabins_num = spectral->rb_edge_extrabins;
		switch (det) {
		case 0:
			if (ch_width == CH_WIDTH_160MHZ &&
			    is_fragmentation_160 &&
			    spectral->report_info[smode].pri20_freq >
			    spectral->report_info[smode].sscan_cfreq1) {
				start_bin = num_fft_bins;
				lb_extrabins_offset =
					dest_det_info->lb_extrabins_num +
					dest_det_info->rb_extrabins_num;
			} else {
				start_bin = 0;
			}
			break;
		case 1:
			if (ch_width == CH_WIDTH_160MHZ &&
			    is_fragmentation_160 &&
			    spectral->report_info[smode].pri20_freq >
			    spectral->report_info[smode].sscan_cfreq1)
				start_bin = 0;
			else {
				start_bin = num_fft_bins;
				lb_extrabins_offset =
					dest_det_info->lb_extrabins_num +
					dest_det_info->rb_extrabins_num;
			}
			break;
		default:
			qdf_spin_unlock_bh(&spectral->session_det_map_lock);
			qdf_spin_unlock_bh(&spectral->detector_list_lock);
			qdf_spin_unlock_bh(
					&spectral->session_report_info_lock);

			return QDF_STATUS_E_FAILURE;
		}
		dest_det_info->dest_start_bin_idx = start_bin;
		dest_det_info->dest_end_bin_idx =
					dest_det_info->dest_start_bin_idx +
					num_fft_bins - 1;
		if (dest_det_info->lb_extrabins_num) {
			if (is_ch_width_160_or_80p80(ch_width)) {
				dest_det_info->lb_extrabins_start_idx =
							2 * num_fft_bins +
							lb_extrabins_offset;
			} else {
				dest_det_info->lb_extrabins_start_idx =
								num_fft_bins;
			}
		}
		if (dest_det_info->rb_extrabins_num)
			dest_det_info->rb_extrabins_start_idx =
					dest_det_info->lb_extrabins_start_idx +
					dest_det_info->lb_extrabins_num;
		dest_det_info->src_start_bin_idx = 0;
		qdf_spin_unlock_bh(&spectral->session_det_map_lock);
	}
	qdf_spin_unlock_bh(&spectral->detector_list_lock);
	qdf_spin_unlock_bh(&spectral->session_report_info_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_update_session_info_from_report_ctx() - Update per-session
 * information from the consume report context. This includes populating start
 * and end bin indices, and set the start and end frequency per-detector.
 * @spectral: Pointer to target_if spectral internal structure
 * @fft_bin_size: Size of 1 FFT bin (in bytes)
 * @cfreq1: Center frequency of Detector 1
 * @cfreq2: Center frequency of Detector 2
 * @smode: Spectral scan mode
 *
 * Update per-session information from the consume report context.
 *
 * Return: Success/Failure
 */
static QDF_STATUS
target_if_update_session_info_from_report_ctx(
				struct target_if_spectral *spectral,
				uint8_t fft_bin_size,
				uint32_t cfreq1, uint32_t cfreq2,
				enum spectral_scan_mode smode)
{
	struct target_if_spectral_ops *p_sops;
	struct per_session_report_info *rpt_info;
	struct per_session_det_map *det_map;
	struct per_session_dest_det_info *dest_det_info;
	enum phy_ch_width ch_width;
	struct wlan_objmgr_psoc *psoc;
	bool is_fragmentation_160;
	uint32_t start_end_freq_arr[2];
	QDF_STATUS ret;
	bool is_session_info_expected;

	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	ret = spectral_is_session_info_expected_from_target(
				spectral->pdev_obj,
				&is_session_info_expected);
	if (QDF_IS_STATUS_ERROR(ret)) {
		spectral_err_rl("Failed to check if session info is expected");
		return ret;
	}

	/* If FW sends this information, use it, no need to get it from here */
	if (is_session_info_expected)
		return QDF_STATUS_SUCCESS;

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("Invalid Spectral mode");
		return QDF_STATUS_E_FAILURE;
	}
	if (!spectral->pdev_obj) {
		spectral_err_rl("Spectral PDEV is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	psoc = wlan_pdev_get_psoc(spectral->pdev_obj);
	if (!psoc) {
		spectral_err_rl("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	qdf_spin_lock_bh(&spectral->session_report_info_lock);

	rpt_info = &spectral->report_info[smode];
	ch_width = rpt_info->sscan_bw;
	is_fragmentation_160 = spectral->rparams.fragmentation_160[smode];

	rpt_info->pri20_freq = p_sops->get_current_channel(spectral, smode);
	rpt_info->cfreq1 = cfreq1;
	rpt_info->cfreq2 = cfreq2;

	if (spectral_debug_level & DEBUG_SPECTRAL4)
		spectral_debug("Before conversion: cfreq1: %u cfreq2: %u",
			       rpt_info->cfreq1, rpt_info->cfreq2);

	ret = target_if_spectral_unify_cfreq_format(
		spectral, &rpt_info->cfreq1, &rpt_info->cfreq2,
		rpt_info->pri20_freq, rpt_info->operating_bw, smode);

	if (QDF_IS_STATUS_ERROR(ret)) {
		qdf_spin_unlock_bh(&spectral->session_report_info_lock);
		spectral_err_rl("Unable to unify cfreq1/cfreq2");
		return QDF_STATUS_E_FAILURE;
	}

	if (spectral_debug_level & DEBUG_SPECTRAL4)
		spectral_debug("After conversion: cfreq1: %d cfreq2: %d",
			       rpt_info->cfreq1, rpt_info->cfreq2);

	/* For Agile mode, sscan_cfreq1 and sscan_cfreq2 are populated
	 * during Spectral start scan
	 */
	if (smode == SPECTRAL_SCAN_MODE_NORMAL) {
		rpt_info->sscan_cfreq1 = rpt_info->cfreq1;
		rpt_info->sscan_cfreq2 = rpt_info->cfreq2;
	}
	qdf_spin_unlock_bh(&spectral->session_report_info_lock);

	if (ch_width == CH_WIDTH_80P80MHZ && wlan_psoc_nif_fw_ext_cap_get(
	    psoc, WLAN_SOC_RESTRICTED_80P80_SUPPORT)) {
		/* Restricted 80p80 */
		struct spectral_fft_bin_markers_160_165mhz *marker;
		struct sscan_detector_list *detector_list;

		marker = &spectral->rparams.marker[smode];
		if (!marker->is_valid)
			return QDF_STATUS_E_FAILURE;

		/**
		 * Restricted 80p80 on Pine has only 1 detector for
		 * normal/agile spectral scan. So, detector_list will
		 * have only one detector
		 */
		qdf_spin_lock_bh(&spectral->detector_list_lock);
		detector_list = &spectral->detector_list[smode][ch_width];

		qdf_spin_lock_bh(&spectral->session_det_map_lock);
		det_map = &spectral->det_map[detector_list->detectors[0]];

		dest_det_info = &det_map->dest_det_info[0];
		dest_det_info->dest_start_bin_idx = marker->start_pri80;
		dest_det_info->dest_end_bin_idx =
					dest_det_info->dest_start_bin_idx +
					marker->num_pri80 - 1;
		dest_det_info->src_start_bin_idx = marker->start_pri80 *
						   fft_bin_size;
		/* Set start and end frequencies */
		qdf_spin_lock_bh(&spectral->session_report_info_lock);
		target_if_spectral_set_start_end_freq(rpt_info->sscan_cfreq1,
						      ch_width,
						      is_fragmentation_160,
						      start_end_freq_arr);
		dest_det_info->start_freq = start_end_freq_arr[0];
		dest_det_info->end_freq = start_end_freq_arr[1];


		dest_det_info = &det_map->dest_det_info[1];
		dest_det_info->dest_start_bin_idx = marker->start_sec80;
		dest_det_info->dest_end_bin_idx =
					dest_det_info->dest_start_bin_idx +
					marker->num_sec80 - 1;
		dest_det_info->src_start_bin_idx = marker->start_sec80 *
						   fft_bin_size;
		/* Set start and end frequencies */
		target_if_spectral_set_start_end_freq(rpt_info->sscan_cfreq2,
						      ch_width,
						      is_fragmentation_160,
						      start_end_freq_arr);
		dest_det_info->start_freq = start_end_freq_arr[0];
		dest_det_info->end_freq = start_end_freq_arr[1];

		dest_det_info = &det_map->dest_det_info[2];
		dest_det_info->dest_start_bin_idx = marker->start_5mhz;
		dest_det_info->dest_end_bin_idx =
					dest_det_info->dest_start_bin_idx +
					marker->num_5mhz - 1;
		dest_det_info->src_start_bin_idx = marker->start_5mhz *
						   fft_bin_size;
		/* Set start and end frequencies */
		dest_det_info->start_freq =
				min(det_map->dest_det_info[0].end_freq,
				    det_map->dest_det_info[1].end_freq);
		dest_det_info->end_freq =
				max(det_map->dest_det_info[0].start_freq,
				    det_map->dest_det_info[1].start_freq);

		qdf_spin_unlock_bh(&spectral->session_report_info_lock);
		qdf_spin_unlock_bh(&spectral->session_det_map_lock);
		qdf_spin_unlock_bh(&spectral->detector_list_lock);
	} else {
		ret = target_if_populate_fft_bins_info(spectral, smode);
		if (QDF_IS_STATUS_ERROR(ret)) {
			spectral_err_rl("Error in populating fft bins info");
			return QDF_STATUS_E_FAILURE;
		}

		ret = target_if_populate_det_start_end_freqs(spectral, smode);
		if (QDF_IS_STATUS_ERROR(ret)) {
			spectral_err_rl("Failed to populate start/end freqs");
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* OPTIMIZED_SAMP_MESSAGE */

#ifdef OPTIMIZED_SAMP_MESSAGE
/**
 * target_if_spectral_populate_samp_params_gen2() - Populate the SAMP params
 * for gen2. SAMP params are to be used for populating SAMP msg.
 * @spectral: Pointer to spectral object
 * @phyerr_info: Pointer to processed phyerr info
 * @params: Pointer to Spectral SAMP message fields to be populated
 *
 * Populate the SAMP params for gen2, which will be used to populate SAMP msg.
 *
 * Return: Success/Failure
 */
static QDF_STATUS
target_if_spectral_populate_samp_params_gen2(
			struct target_if_spectral *spectral,
			struct spectral_process_phyerr_info_gen2 *phyerr_info,
			struct target_if_samp_msg_params *params)
{
	uint8_t chn_idx_highest_enabled;
	uint8_t chn_idx_lowest_enabled;
	int8_t control_rssi;
	int8_t extension_rssi;
	struct target_if_spectral_rfqual_info *p_rfqual;
	struct spectral_search_fft_info_gen2 *p_sfft;
	struct spectral_phyerr_fft_gen2 *pfft;
	struct target_if_spectral_acs_stats *acs_stats;
	enum phy_ch_width ch_width;
	enum spectral_scan_mode smode = SPECTRAL_SCAN_MODE_NORMAL;

	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!phyerr_info) {
		spectral_err_rl("Pointer to phyerr info is null");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!params) {
		spectral_err_rl("SAMP msg params structure is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	qdf_spin_lock_bh(&spectral->session_report_info_lock);
	ch_width = spectral->report_info[smode].sscan_bw;
	qdf_spin_unlock_bh(&spectral->session_report_info_lock);

	acs_stats = phyerr_info->acs_stats;
	pfft = phyerr_info->pfft;
	p_sfft = phyerr_info->p_sfft;
	p_rfqual = phyerr_info->p_rfqual;

	params->hw_detector_id = phyerr_info->seg_id;
	params->rssi = p_rfqual->rssi_comb;
	if (spectral->is_sec80_rssi_war_required && phyerr_info->seg_id == 1)
		params->rssi = target_if_get_combrssi_sec80_seg_gen2(spectral,
								     p_sfft);

	chn_idx_highest_enabled =
		   ((spectral->params[smode].ss_chn_mask & 0x8) ? 3 :
		    (spectral->params[smode].ss_chn_mask & 0x4) ? 2 :
		    (spectral->params[smode].ss_chn_mask & 0x2) ? 1 : 0);
	chn_idx_lowest_enabled =
		   ((spectral->params[smode].ss_chn_mask & 0x1) ? 0 :
		    (spectral->params[smode].ss_chn_mask & 0x2) ? 1 :
		    (spectral->params[smode].ss_chn_mask & 0x4) ? 2 : 3);
	control_rssi =
		p_rfqual->pc_rssi_info[chn_idx_highest_enabled].rssi_pri20;
	extension_rssi =
		p_rfqual->pc_rssi_info[chn_idx_highest_enabled].rssi_sec20;

	if (spectral->upper_is_control)
		params->upper_rssi = control_rssi;
	else
		params->upper_rssi = extension_rssi;

	if (spectral->lower_is_control)
		params->lower_rssi = control_rssi;
	else
		params->lower_rssi = extension_rssi;

	if (spectral->sc_spectral_noise_pwr_cal) {
		int idx;

		for (idx = 0; idx < HOST_MAX_ANTENNA; idx++) {
			params->chain_ctl_rssi[idx] =
				p_rfqual->pc_rssi_info[idx].rssi_pri20;
			params->chain_ext_rssi[idx] =
				p_rfqual->pc_rssi_info[idx].rssi_sec20;
		}
	}
	params->timestamp = (phyerr_info->tsf64 & SPECTRAL_TSMASK);
	params->max_mag = p_sfft->peak_mag;
	params->max_index = p_sfft->peak_inx;

	/*
	 * For VHT80_80/VHT160, the noise floor for primary
	 * 80MHz segment is populated with the lowest enabled
	 * antenna chain and the noise floor for secondary 80MHz segment
	 * is populated with the highest enabled antenna chain.
	 * For modes upto VHT80, the noise floor is populated with the
	 * one corresponding to the highest enabled antenna chain.
	 */
	if (is_ch_width_160_or_80p80(ch_width) && phyerr_info->seg_id == 0)
		params->noise_floor =
				p_rfqual->noise_floor[chn_idx_lowest_enabled];
	else
		params->noise_floor =
				p_rfqual->noise_floor[chn_idx_highest_enabled];

	acs_stats->ctrl_nf = params->noise_floor;
	acs_stats->ext_nf = params->noise_floor;
	acs_stats->nfc_ctl_rssi = control_rssi;
	acs_stats->nfc_ext_rssi = extension_rssi;

	params->bin_pwr_data = (uint8_t *)pfft;

	return QDF_STATUS_SUCCESS;
}

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
	 *          1. Frequency
	 *          2. Spectral RSSI
	 *          3. Bin Power Count
	 *          4. Bin Power values
	 *          5. Spectral Timestamp
	 *          6. MAC Address
	 *
	 *       This function prepares the params structure and populates it
	 *       with relevant values, this is in turn passed to
	 *       spectral_fill_samp_msg()
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
	struct spectral_phyerr_tlv_gen2 *ptlv = NULL;
	struct spectral_phyerr_tlv_gen2 *ptlv_sec80 = NULL;
	struct spectral_phyerr_fft_gen2 *pfft = NULL;
	struct spectral_phyerr_fft_gen2 *pfft_sec80 = NULL;
	struct spectral_process_phyerr_info_gen2 process_phyerr_fields;
	struct spectral_process_phyerr_info_gen2 *phyerr_info =
						&process_phyerr_fields;
	uint8_t segid = 0;
	uint8_t segid_sec80;
	enum phy_ch_width ch_width;
	QDF_STATUS ret;
	struct target_if_spectral_ops *p_sops;

	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return -EPERM;
	}

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	/* Drop the sample if Spectral is not active */
	if (!p_sops->is_spectral_active(spectral,
					SPECTRAL_SCAN_MODE_NORMAL)) {
		spectral_info_rl("Spectral scan is not active");
		goto fail_no_print;
	}

	if (!data) {
		spectral_err_rl("Phyerror event buffer is null");
		goto fail;
	}
	if (!p_rfqual) {
		spectral_err_rl("RF quality information is null");
		goto fail;
	}
	if (!p_chaninfo) {
		spectral_err_rl("Channel information is null");
		goto fail;
	}
	if (!acs_stats) {
		spectral_err_rl("ACS stats pointer is null");
		goto fail;
	}

	qdf_spin_lock_bh(&spectral->session_report_info_lock);
	ch_width = spectral->report_info[SPECTRAL_SCAN_MODE_NORMAL].sscan_bw;
	qdf_spin_unlock_bh(&spectral->session_report_info_lock);

	ptlv = (struct spectral_phyerr_tlv_gen2 *)data;

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
	if (spectral_debug_level & (DEBUG_SPECTRAL2 | DEBUG_SPECTRAL4))
		target_if_spectral_dump_phyerr_data_gen2(
					data, datalen,
					spectral->is_160_format);

	if (ptlv->signature != SPECTRAL_PHYERR_SIGNATURE_GEN2) {
		/*
		 * EV# 118023: We tentatively disable the below print
		 * and provide stats instead.
		 */
		spectral->diag_stats.spectral_mismatch++;
		goto fail;
	}

	qdf_mem_zero(&params, sizeof(params));

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
				goto fail;
			}
		}

		target_if_process_sfft_report_gen2(ptlv, ptlv->length,
						   p_sfft);

		ret = target_if_update_session_info_from_report_ctx(
						spectral, FFT_BIN_SIZE_1BYTE,
						p_chaninfo->center_freq1,
						p_chaninfo->center_freq2,
						SPECTRAL_SCAN_MODE_NORMAL);
		if (QDF_IS_STATUS_ERROR(ret)) {
			spectral_err_rl("Failed to update per-session info");
			goto fail;
		}

		phyerr_info->p_rfqual = p_rfqual;
		phyerr_info->p_sfft = p_sfft;
		phyerr_info->pfft = pfft;
		phyerr_info->acs_stats = acs_stats;
		phyerr_info->tsf64 = tsf64;
		phyerr_info->seg_id = segid;

		ret = target_if_spectral_populate_samp_params_gen2(spectral,
								   phyerr_info,
								   &params);
		if (QDF_IS_STATUS_ERROR(ret)) {
			spectral_err_rl("Failed to populate SAMP params");
			goto fail;
		}

		ret = target_if_spectral_fill_samp_msg(spectral, &params);
		if (QDF_IS_STATUS_ERROR(ret)) {
			spectral_err_rl("Failed to fill the SAMP msg");
			goto fail;
		}

		if (spectral->is_160_format &&
		    is_ch_width_160_or_80p80(ch_width)) {
			/*
			 * We expect to see one more Search FFT report, and it
			 * should be equal in size to the current one.
			 */
			if (datalen < (
				2 * (sizeof(struct spectral_phyerr_tlv_gen2) +
				     ptlv->length))) {
				struct spectral_diag_stats *p_diag_stats =
					&spectral->diag_stats;
				p_diag_stats->spectral_sec80_sfft_insufflen++;
				goto fail;
			}

			ptlv_sec80 = (struct spectral_phyerr_tlv_gen2 *)(
				      data +
				      sizeof(struct spectral_phyerr_tlv_gen2) +
				      ptlv->length);

			if (ptlv_sec80->signature !=
			    SPECTRAL_PHYERR_SIGNATURE_GEN2) {
				spectral->diag_stats.spectral_mismatch++;
				goto fail;
			}

			if (ptlv_sec80->tag != TLV_TAG_SEARCH_FFT_REPORT_GEN2) {
				spectral->diag_stats.spectral_no_sec80_sfft++;
				goto fail;
			}

			segid_sec80 = *((SPECTRAL_SEGID_INFO *)(
				(uint8_t *)ptlv_sec80 +
				sizeof(struct spectral_phyerr_tlv_gen2) +
				sizeof(struct spectral_phyerr_hdr_gen2)));

			if (segid_sec80 != 1) {
				struct spectral_diag_stats *p_diag_stats =
					&spectral->diag_stats;
				p_diag_stats->spectral_vhtseg2id_mismatch++;
				goto fail;
			}

			target_if_process_sfft_report_gen2(ptlv_sec80,
							   ptlv_sec80->length,
							   p_sfft_sec80);

			pfft_sec80 = (struct spectral_phyerr_fft_gen2 *)(
				((uint8_t *)ptlv_sec80) +
				sizeof(struct spectral_phyerr_tlv_gen2) +
				sizeof(struct spectral_phyerr_hdr_gen2) +
				segid_skiplen);

			qdf_mem_zero(&params, sizeof(params));

			phyerr_info->p_rfqual = p_rfqual;
			phyerr_info->p_sfft = p_sfft_sec80;
			phyerr_info->pfft = pfft_sec80;
			phyerr_info->acs_stats = acs_stats;
			phyerr_info->tsf64 = tsf64;
			phyerr_info->seg_id = segid_sec80;

			ret = target_if_spectral_populate_samp_params_gen2(
							spectral, phyerr_info,
							&params);
			if (QDF_IS_STATUS_ERROR(ret)) {
				spectral_err_rl("Failed to populate SAMP params");
				goto fail;
			}
			ret = target_if_spectral_fill_samp_msg(spectral,
							       &params);
			if (QDF_IS_STATUS_ERROR(ret)) {
				spectral_err_rl("Failed to fill the SAMP msg");
				goto fail;
			}
		}
	}

	if (spectral_debug_level & DEBUG_SPECTRAL4)
		spectral_debug_level = DEBUG_SPECTRAL;

	return 0;

fail:
	spectral_err_rl("Error while processing Spectral report");

fail_no_print:
	if (spectral_debug_level & DEBUG_SPECTRAL4)
		spectral_debug_level = DEBUG_SPECTRAL;

	free_samp_msg_skb(spectral, SPECTRAL_SCAN_MODE_NORMAL);
	return -EPERM;
}

#else
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
	enum phy_ch_width ch_width =
				spectral->ch_width[SPECTRAL_SCAN_MODE_NORMAL];

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
	/* Gen 2 only supports normal Spectral scan currently */
	params.smode = SPECTRAL_SCAN_MODE_NORMAL;

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
		   ((spectral->params[params.smode].ss_chn_mask & 0x8) ? 3 :
		    (spectral->params[params.smode].ss_chn_mask & 0x4) ? 2 :
		    (spectral->params[params.smode].ss_chn_mask & 0x2) ? 1 : 0);
		chn_idx_lowest_enabled =
		   ((spectral->params[params.smode].ss_chn_mask & 0x1) ? 0 :
		    (spectral->params[params.smode].ss_chn_mask & 0x2) ? 1 :
		    (spectral->params[params.smode].ss_chn_mask & 0x4) ? 2 : 3);
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
		params.freq = p_sops->get_current_channel(spectral,
							  params.smode);
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
		    is_ch_width_160_or_80p80(ch_width)) {
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
#endif /* OPTIMIZED_SAMP_MESSAGE */

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
	offset = target_if_get_offset_swar_sec80(
			spectral->ch_width[SPECTRAL_SCAN_MODE_NORMAL]);

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

QDF_STATUS
target_if_spectral_copy_fft_bins(struct target_if_spectral *spectral,
				 const void *src_fft_buf,
				 void *dest_fft_buf,
				 uint32_t fft_bin_count,
				 uint32_t *bytes_copied,
				 uint16_t pwr_format)
{
	uint16_t idx, dword_idx, fft_bin_idx;
	uint8_t num_bins_per_dword, hw_fft_bin_width_bits;
	uint32_t num_dwords;
	uint16_t fft_bin_val;
	struct spectral_report_params *rparams;
	const uint32_t *dword_ptr;
	uint32_t dword;
	uint8_t *fft_bin_buf;

	*bytes_copied = 0;

	if (!spectral) {
		spectral_err("spectral lmac object is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!src_fft_buf) {
		spectral_err("source fft bin buffer is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!dest_fft_buf) {
		spectral_err("destination fft bin buffer is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	rparams = &spectral->rparams;
	num_bins_per_dword = SPECTRAL_DWORD_SIZE / rparams->hw_fft_bin_width;
	num_dwords = fft_bin_count / num_bins_per_dword;
	hw_fft_bin_width_bits = rparams->hw_fft_bin_width * QDF_CHAR_BIT;

	fft_bin_idx = 0;
	dword_ptr = src_fft_buf;
	fft_bin_buf = dest_fft_buf;
	for (dword_idx = 0; dword_idx < num_dwords; dword_idx++) {
		dword = *dword_ptr++; /* Read a DWORD */
		for (idx = 0; idx < num_bins_per_dword; idx++) {
			fft_bin_val = (uint16_t)QDF_GET_BITS(
					dword,
					idx * hw_fft_bin_width_bits,
					hw_fft_bin_width_bits);

			fft_bin_buf[fft_bin_idx++] =
				clamp_fft_bin_value(fft_bin_val, pwr_format);
		}
	}

	*bytes_copied = num_dwords *  SPECTRAL_DWORD_SIZE;

	return QDF_STATUS_SUCCESS;
}

#ifdef DIRECT_BUF_RX_ENABLE
/**
 * target_if_get_spectral_mode() - Get Spectral scan mode corresponding to a
 * detector id
 * @detector_id: detector id in the Spectral report
 * @rparams: pointer to report params object
 *
 * Helper API to get Spectral scan mode from the detector ID. This mapping is
 * target specific.
 *
 * Return: Spectral scan mode
 */
static enum spectral_scan_mode
target_if_get_spectral_mode(enum spectral_detector_id detector_id,
			    struct spectral_report_params *rparams)
{
	if (detector_id >= SPECTRAL_DETECTOR_ID_MAX) {
		spectral_err_rl("Invalid detector id %d", detector_id);
		return SPECTRAL_SCAN_MODE_INVALID;
	}

	return rparams->detid_mode_table[detector_id];
}

/**
 * target_if_spectral_get_bin_count_after_len_adj() - Get number of FFT bins in
 * Spectral FFT report
 * @fft_bin_len: FFT bin length reported by target
 * @rpt_mode: Spectral report mode
 * @swar: Spectral FFT bin length adjustments SWAR parameters
 * @fft_bin_size: Size of one FFT bin in bytes
 *
 * Get actual number of FFT bins in the FFT report after adjusting the length
 * by applying the SWARs for getting correct length.
 *
 * Return: FFT bin count
 */
static size_t
target_if_spectral_get_bin_count_after_len_adj(
				size_t fft_bin_len, uint8_t rpt_mode,
				struct spectral_fft_bin_len_adj_swar *swar,
				size_t *fft_bin_size)
{
	size_t fft_bin_count = fft_bin_len;

	if (rpt_mode == 1 && swar->null_fftbin_adj) {
		/*
		 * No FFT bins are expected. Explicitly set FFT bin
		 * count to 0.
		 */
		fft_bin_count = 0;
		*fft_bin_size = 0;
	} else {
		/*
		 * Divide fft bin length by appropriate factor depending
		 * on the value of fftbin_size_war.
		 */
		switch (swar->fftbin_size_war) {
		case SPECTRAL_FFTBIN_SIZE_WAR_4BYTE_TO_1BYTE:
			fft_bin_count >>= 2;
			*fft_bin_size = 4;
			break;
		case SPECTRAL_FFTBIN_SIZE_WAR_2BYTE_TO_1BYTE:
			fft_bin_count >>= 1;
			*fft_bin_size = 2;
			/* Ideally we should be dividing fft bin length
			 * by 2. Due to a HW bug, actual length is two
			 * times the expected length.
			 */
			if (swar->packmode_fftbin_size_adj)
				fft_bin_count >>= 1;
			break;
		case SPECTRAL_FFTBIN_SIZE_NO_WAR:
			*fft_bin_size = 1;
			/* No length adjustment */
			break;
		default:
			qdf_assert_always(0);
		}

		if (rpt_mode == 2 && swar->inband_fftbin_size_adj)
			fft_bin_count >>= 1;
	}

	return fft_bin_count;
}

#ifndef OPTIMIZED_SAMP_MESSAGE
/**
 * target_if_process_sfft_report_gen3() - Process Search FFT Report for gen3
 * @p_fft_report: Pointer to fft report
 * @p_sfft: Pointer to search fft report
 * @rparams: pointer to report params object
 *
 * Process Search FFT Report for gen3
 *
 * Return: Success/Failure
 */
static int
target_if_process_sfft_report_gen3(
	struct spectral_phyerr_fft_report_gen3 *p_fft_report,
	struct spectral_search_fft_info_gen3 *p_sfft,
	struct spectral_report_params *rparams)
{
	int32_t peak_sidx = 0;
	int32_t peak_mag;

	qdf_assert_always(p_fft_report);
	qdf_assert_always(p_sfft);
	qdf_assert_always(rparams);

	/*
	 * For simplicity, everything is defined as uint32_t (except one).
	 * Proper code will later use the right sizes.
	 */
	/*
	 * For easy comparision between MDK team and OS team, the MDK script
	 * variable names have been used
	 */

	/* Populate the Search FFT Info */
	p_sfft->timestamp = p_fft_report->fft_timestamp;

	p_sfft->fft_detector_id = get_bitfield(p_fft_report->hdr_a,
					       2, 0);
	p_sfft->fft_num = get_bitfield(p_fft_report->hdr_a, 3, 2);

	switch (rparams->version) {
	case SPECTRAL_REPORT_FORMAT_VERSION_1:
		p_sfft->fft_radar_check = get_bitfield(p_fft_report->hdr_a,
						       12, 5);
		peak_sidx = get_bitfield(p_fft_report->hdr_a, 11, 17);
		p_sfft->fft_chn_idx = get_bitfield(p_fft_report->hdr_a, 3, 28);
		p_sfft->fft_base_pwr_db = get_bitfield(p_fft_report->hdr_b,
						       9, 0);
		p_sfft->fft_total_gain_db = get_bitfield(p_fft_report->hdr_b,
							 8, 9);
		break;
	case SPECTRAL_REPORT_FORMAT_VERSION_2:
		p_sfft->fft_radar_check = get_bitfield(p_fft_report->hdr_a,
						       14, 5);
		peak_sidx = get_bitfield(p_fft_report->hdr_a, 11, 19);
		p_sfft->fft_chn_idx = get_bitfield(p_fft_report->hdr_b, 3, 0);
		p_sfft->fft_base_pwr_db = get_bitfield(p_fft_report->hdr_b,
						       9, 3);
		p_sfft->fft_total_gain_db = get_bitfield(p_fft_report->hdr_b,
							 8, 12);
		break;
	default:
		qdf_assert_always(0);
	}

	p_sfft->fft_peak_sidx = unsigned_to_signed(peak_sidx, 11);

	p_sfft->fft_num_str_bins_ib = get_bitfield(p_fft_report->hdr_c,
						   8, 0);
	peak_mag = get_bitfield(p_fft_report->hdr_c, 10, 8);
	p_sfft->fft_peak_mag = unsigned_to_signed(peak_mag, 10);
	p_sfft->fft_avgpwr_db = get_bitfield(p_fft_report->hdr_c,
					     7, 18);
	p_sfft->fft_relpwr_db = get_bitfield(p_fft_report->hdr_c,
					     7, 25);

	return 0;
}
#endif

/**
 * target_if_dump_fft_report_gen3() - Dump FFT Report for gen3
 * @spectral: Pointer to Spectral object
 * @smode: Spectral scan mode
 * @p_fft_report: Pointer to fft report
 * @p_sfft: Pointer to search fft report
 *
 * Dump FFT Report for gen3
 *
 * Return: void
 */
static void
target_if_dump_fft_report_gen3(struct target_if_spectral *spectral,
			enum spectral_scan_mode smode,
			struct spectral_phyerr_fft_report_gen3 *p_fft_report,
			struct spectral_search_fft_info_gen3 *p_sfft)
{
	size_t fft_hdr_length;
	size_t report_len;
	size_t fft_bin_len;
	size_t fft_bin_count;
	size_t fft_bin_size;
	size_t fft_bin_len_inband_tfer = 0;
	uint8_t tag, signature;

	qdf_assert_always(spectral);

	/* There won't be FFT report/bins in report mode 0, so return */
	if (!spectral->params[smode].ss_rpt_mode)
		return;

	fft_hdr_length = get_bitfield(
				p_fft_report->fft_hdr_lts,
				SPECTRAL_REPORT_LTS_HDR_LENGTH_SIZE_GEN3,
				SPECTRAL_REPORT_LTS_HDR_LENGTH_POS_GEN3) * 4;

	tag = get_bitfield(p_fft_report->fft_hdr_lts,
			   SPECTRAL_REPORT_LTS_TAG_SIZE_GEN3,
			   SPECTRAL_REPORT_LTS_TAG_POS_GEN3);

	signature = get_bitfield(p_fft_report->fft_hdr_lts,
				 SPECTRAL_REPORT_LTS_SIGNATURE_SIZE_GEN3,
				 SPECTRAL_REPORT_LTS_SIGNATURE_POS_GEN3);

	report_len = (fft_hdr_length + 8);
	fft_bin_len = fft_hdr_length - spectral->rparams.fft_report_hdr_len;
	fft_bin_count = target_if_spectral_get_bin_count_after_len_adj(
			fft_bin_len,
			spectral->params[smode].ss_rpt_mode,
			&spectral->len_adj_swar, &fft_bin_size);

	if ((spectral->params[smode].ss_rpt_mode == 2) &&
	    spectral->len_adj_swar.inband_fftbin_size_adj)
		fft_bin_len_inband_tfer = fft_bin_len >> 1;

	spectral_debug("Spectral FFT Report");
	spectral_debug("fft_timestamp = 0x%x", p_fft_report->fft_timestamp);
	spectral_debug("fft_hdr_length = %zu(32 bit words)",
		       fft_hdr_length >> 2);
	spectral_debug("fft_hdr_tag = 0x%x", tag);
	spectral_debug("fft_hdr_sig = 0x%x", signature);

	spectral_debug("Length field in search fft report is %zu(0x%zx) bytes",
		       fft_hdr_length, fft_hdr_length);
	spectral_debug("Total length of search fft report is %zu(0x%zx) bytes",
		       report_len, report_len);
	spectral_debug("Target reported fftbins in report is %zu(0x%zx)",
		       fft_bin_len, fft_bin_len);

	if ((spectral->params[smode].ss_rpt_mode == 1) &&
	    spectral->len_adj_swar.null_fftbin_adj)
		spectral_debug("WAR: Considering number of FFT bins as 0");
	else if ((spectral->params[smode].ss_rpt_mode == 2) &&
		 spectral->len_adj_swar.inband_fftbin_size_adj) {
		spectral_debug("FW fftbins actually transferred (in-band report mode) %zu(0x%zx)",
			       fft_bin_len_inband_tfer,
			       fft_bin_len_inband_tfer);
	}

	spectral_debug("Actual number of fftbins in report is %zu(0x%zx)",
		       fft_bin_count, fft_bin_count);

	spectral_debug("fft_detector_id = %u", p_sfft->fft_detector_id);
	spectral_debug("fft_num = %u", p_sfft->fft_num);
	spectral_debug("fft_radar_check = %u", p_sfft->fft_radar_check);
	spectral_debug("fft_peak_sidx = %d",  p_sfft->fft_peak_sidx);
	spectral_debug("fft_chn_idx = %u", p_sfft->fft_chn_idx);
	spectral_debug("fft_base_pwr_db = %u", p_sfft->fft_base_pwr_db);
	spectral_debug("fft_total_gain_db = %u", p_sfft->fft_total_gain_db);
	spectral_debug("fft_num_str_bins_ib = %u", p_sfft->fft_num_str_bins_ib);
	spectral_debug("fft_peak_mag = %d", p_sfft->fft_peak_mag);
	spectral_debug("fft_avgpwr_db = %u", p_sfft->fft_avgpwr_db);
	spectral_debug("fft_relpwr_db = %u", p_sfft->fft_relpwr_db);

	if (fft_bin_count > 0) {
		uint8_t *fft_bin_buf;
		uint32_t bytes_copied;
		QDF_STATUS status;

		fft_bin_buf = qdf_mem_malloc(fft_bin_count);
		if (!fft_bin_buf) {
			spectral_err_rl("memory allocation failed");
			return;
		}

		status = target_if_spectral_copy_fft_bins(
				spectral, &p_fft_report->buf,
				fft_bin_buf, fft_bin_count, &bytes_copied,
				spectral->params[smode].ss_pwr_format);
		if (QDF_IS_STATUS_ERROR(status)) {
			spectral_err_rl("Unable to populate FFT bins");
			qdf_mem_free(fft_bin_buf);
			return;
		}

		spectral_debug("FFT bin buffer size = %zu", fft_bin_count);
		spectral_debug("FFT bins:");
		target_if_spectral_hexdump(fft_bin_buf, fft_bin_count);
		qdf_mem_free(fft_bin_buf);
	}
}
#endif

#ifdef OPTIMIZED_SAMP_MESSAGE
QDF_STATUS
target_if_160mhz_delivery_state_change(struct target_if_spectral *spectral,
				       enum spectral_scan_mode smode,
				       uint8_t detector_id) {
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("Invalid Spectral mode %d", smode);
		return QDF_STATUS_E_INVAL;
	}

	if (!is_ch_width_160_or_80p80(spectral->report_info[smode].sscan_bw)) {
		spectral_err_rl("Scan BW %d is not 160/80p80 for mode %d",
				spectral->report_info[smode].sscan_bw, smode);
		return QDF_STATUS_E_FAILURE;
	}

	switch (spectral->state_160mhz_delivery[smode]) {
	case SPECTRAL_REPORT_WAIT_PRIMARY80:
		if (detector_id == SPECTRAL_DETECTOR_ID_0)
			spectral->state_160mhz_delivery[smode] =
				SPECTRAL_REPORT_WAIT_SECONDARY80;
		else {
			status = QDF_STATUS_E_FAILURE;
			spectral->diag_stats.spectral_vhtseg1id_mismatch++;
		}
		break;

	case SPECTRAL_REPORT_WAIT_SECONDARY80:
		if (detector_id == SPECTRAL_DETECTOR_ID_1)
			spectral->state_160mhz_delivery[smode] =
				SPECTRAL_REPORT_WAIT_PRIMARY80;
		else {
			spectral->state_160mhz_delivery[smode] =
				SPECTRAL_REPORT_WAIT_PRIMARY80;
			status = QDF_STATUS_E_FAILURE;
			spectral->diag_stats.spectral_vhtseg2id_mismatch++;
		}
		break;

	default:
		break;
	}

	return status;
}
#else
QDF_STATUS
target_if_160mhz_delivery_state_change(struct target_if_spectral *spectral,
				       enum spectral_scan_mode smode,
				       uint8_t detector_id) {
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (smode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("Invalid Spectral mode %d", smode);
		return QDF_STATUS_E_INVAL;
	}

	if (!is_ch_width_160_or_80p80(spectral->ch_width[smode])) {
		spectral_err_rl("Scan BW %d is not 160/80p80 for mode %d",
				spectral->ch_width[smode], smode);
		return QDF_STATUS_E_FAILURE;
	}

	switch (spectral->state_160mhz_delivery[smode]) {
	case SPECTRAL_REPORT_WAIT_PRIMARY80:
		if (detector_id == SPECTRAL_DETECTOR_ID_0)
			spectral->state_160mhz_delivery[smode] =
				SPECTRAL_REPORT_RX_PRIMARY80;
		else {
			status = QDF_STATUS_E_FAILURE;
			spectral->diag_stats.spectral_vhtseg1id_mismatch++;
		}
		break;

	case SPECTRAL_REPORT_WAIT_SECONDARY80:
		if (detector_id == SPECTRAL_DETECTOR_ID_1)
			spectral->state_160mhz_delivery[smode] =
				SPECTRAL_REPORT_RX_SECONDARY80;
		else {
			spectral->state_160mhz_delivery[smode] =
				SPECTRAL_REPORT_WAIT_PRIMARY80;
			status = QDF_STATUS_E_FAILURE;
			spectral->diag_stats.spectral_vhtseg2id_mismatch++;
		}
		break;

	case SPECTRAL_REPORT_RX_SECONDARY80:
		/* We don't care about detector id in this state. */
		reset_160mhz_delivery_state_machine(spectral, smode);
		break;

	case SPECTRAL_REPORT_RX_PRIMARY80:
		/* We don't care about detector id in this state */
		spectral->state_160mhz_delivery[smode] =
				SPECTRAL_REPORT_WAIT_SECONDARY80;
		break;

	default:
		break;
	}

	return status;
}
#endif /* OPTIMIZED_SAMP_MESSAGE */

#ifdef DIRECT_BUF_RX_ENABLE
/**
 * target_if_get_detector_id_sscan_summary_report_gen3() - Get Spectral detector
 * ID from Spectral summary report
 * @data: Pointer to Spectral summary report
 *
 * Return: Detector ID
 */
static uint8_t
target_if_get_detector_id_sscan_summary_report_gen3(uint8_t *data) {
	struct spectral_sscan_summary_report_gen3 *psscan_summary_report;
	uint8_t detector_id;

	qdf_assert_always(data);

	psscan_summary_report =
		(struct spectral_sscan_summary_report_gen3 *)data;

	detector_id = get_bitfield(
			psscan_summary_report->hdr_a,
			SSCAN_SUMMARY_REPORT_HDR_A_DETECTOR_ID_SIZE_GEN3,
			SSCAN_SUMMARY_REPORT_HDR_A_DETECTOR_ID_POS_GEN3);

	return detector_id;
}

#ifndef OPTIMIZED_SAMP_MESSAGE
/**
 * target_if_consume_sscan_summary_report_gen3() - Consume Spectral summary
 * report
 * @data: Pointer to Spectral summary report
 * @fields: Pointer to structure to be populated with extracted fields
 * @rparams: Pointer to structure with Spectral report params
 *
 * Consume Spectral summary report for gen3
 *
 * Return: void
 */
static void
target_if_consume_sscan_summary_report_gen3(
				uint8_t *data,
				struct sscan_report_fields_gen3 *fields,
				struct spectral_report_params *rparams) {
	struct spectral_sscan_summary_report_gen3 *psscan_summary_report;

	qdf_assert_always(data);
	qdf_assert_always(fields);
	qdf_assert_always(rparams);

	psscan_summary_report =
		(struct spectral_sscan_summary_report_gen3 *)data;

	fields->sscan_agc_total_gain = get_bitfield(
			psscan_summary_report->hdr_a,
			SSCAN_SUMMARY_REPORT_HDR_A_AGC_TOTAL_GAIN_SIZE_GEN3,
			SSCAN_SUMMARY_REPORT_HDR_A_AGC_TOTAL_GAIN_POS_GEN3);
	fields->inband_pwr_db = get_bitfield(
			psscan_summary_report->hdr_a,
			SSCAN_SUMMARY_REPORT_HDR_A_INBAND_PWR_DB_SIZE_GEN3,
			SSCAN_SUMMARY_REPORT_HDR_A_INBAND_PWR_DB_POS_GEN3);
	fields->sscan_pri80 = get_bitfield(
			psscan_summary_report->hdr_a,
			SSCAN_SUMMARY_REPORT_HDR_A_PRI80_SIZE_GEN3,
			SSCAN_SUMMARY_REPORT_HDR_A_PRI80_POS_GEN3);

	switch (rparams->version) {
	case SPECTRAL_REPORT_FORMAT_VERSION_1:
		fields->sscan_gainchange = get_bitfield(
			psscan_summary_report->hdr_b,
			SSCAN_SUMMARY_REPORT_HDR_B_GAINCHANGE_SIZE_GEN3_V1,
			SSCAN_SUMMARY_REPORT_HDR_B_GAINCHANGE_POS_GEN3_V1);
		break;
	case SPECTRAL_REPORT_FORMAT_VERSION_2:
		fields->sscan_gainchange = get_bitfield(
			psscan_summary_report->hdr_c,
			SSCAN_SUMMARY_REPORT_HDR_C_GAINCHANGE_SIZE_GEN3_V2,
			SSCAN_SUMMARY_REPORT_HDR_C_GAINCHANGE_POS_GEN3_V2);
		break;
	default:
		qdf_assert_always(0);
	}
}
#endif

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
	uint32_t lts;

	lts = *((uint32_t *)(data + SPECTRAL_PHYERR_HDR_LTS_POS));
	/* Peek into the data to figure out whether
	 *      1) Signature matches the expected value
	 *      2) What is inside the package (TAG ID is used for finding this)
	 */
	tag = get_bitfield(lts,
			   SPECTRAL_REPORT_LTS_TAG_SIZE_GEN3,
			   SPECTRAL_REPORT_LTS_TAG_POS_GEN3);

	signature = get_bitfield(lts,
				 SPECTRAL_REPORT_LTS_SIGNATURE_SIZE_GEN3,
				 SPECTRAL_REPORT_LTS_SIGNATURE_POS_GEN3);


	if (signature != SPECTRAL_PHYERR_SIGNATURE_GEN3) {
		spectral->diag_stats.spectral_mismatch++;
		return -EINVAL;
	}

	if (tag != exp_tag) {
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

#ifdef DIRECT_BUF_RX_DEBUG
static void target_if_spectral_check_buffer_poisoning(
	struct target_if_spectral *spectral,
	struct spectral_report *report,
	int num_fft_bins, enum spectral_scan_mode smode)
{
	uint32_t *data;
	size_t len;
	size_t words_to_check =
		sizeof(struct spectral_sscan_summary_report_gen3) >> 2;
	bool poisoned_words_found = false;

	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return;
	}

	if (!spectral->dbr_buff_debug)
		return;

	if (!report) {
		spectral_err_rl("Spectral report is null");
		return;
	}

	/* Add search FFT report */
	if (spectral->params[smode].ss_rpt_mode > 0)
		words_to_check +=
			sizeof(struct spectral_phyerr_fft_report_gen3) >> 2;

	/* Now add the number of FFT bins */
	if (spectral->params[smode].ss_rpt_mode > 1) {
		/* Caller should take care to pass correct number of FFT bins */
		if (spectral->len_adj_swar.fftbin_size_war ==
				SPECTRAL_FFTBIN_SIZE_WAR_4BYTE_TO_1BYTE)
			words_to_check += num_fft_bins;
		else if (spectral->len_adj_swar.fftbin_size_war ==
				SPECTRAL_FFTBIN_SIZE_WAR_2BYTE_TO_1BYTE)
			words_to_check += (num_fft_bins >> 1);
	}

	data = (uint32_t *)report->data;
	for (len = 0; len < words_to_check; ++len) {
		if (*data == MEM_POISON_SIGNATURE) {
			spectral_err("Pattern(%x) found in Spectral search FFT report at position %zu in the buffer %pK",
				     MEM_POISON_SIGNATURE,
				     (len << 2), report->data);
			poisoned_words_found = true;
			break;
		}
		++data;
	}

	/* Crash the FW even if one word is poisoned */
	if (poisoned_words_found) {
		spectral_err("Pattern(%x) found in Spectral report, Hex dump of the sfft follows",
			     MEM_POISON_SIGNATURE);
		target_if_spectral_hexdump((unsigned char *)report->data,
					   words_to_check << 2);
		spectral_err("Asserting the FW");
		target_if_spectral_fw_hang(spectral);
	}
}

#ifdef OPTIMIZED_SAMP_MESSAGE
static void target_if_spectral_verify_ts(struct target_if_spectral *spectral,
					 uint8_t *buf, uint32_t current_ts,
					 uint8_t detector_id)
{
	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return;
	}
	if (detector_id >= MAX_DETECTORS_PER_PDEV) {
		spectral_err_rl("Spectral detector_id %d exceeds range",
				detector_id);
		return;
	}

	if (!spectral->dbr_buff_debug)
		return;

	if (spectral->prev_tstamp[detector_id]) {
		if (current_ts == spectral->prev_tstamp[detector_id]) {
			spectral_err("Spectral timestamp(%u) in the current buffer(%pK) is equal to the previous timestamp, same report DMAed twice? Asserting the FW",
				     current_ts, buf);
			target_if_spectral_fw_hang(spectral);
		}
	}
	spectral->prev_tstamp[detector_id] = current_ts;
}
#else
static void target_if_spectral_verify_ts(struct target_if_spectral *spectral,
					 uint8_t *buf, uint32_t current_ts)
{
	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return;
	}

	if (!spectral->dbr_buff_debug)
		return;

	if (spectral->prev_tstamp) {
		if (current_ts == spectral->prev_tstamp) {
			spectral_err("Spectral timestamp(%u) in the current buffer(%pK) is equal to the previous timestamp, same report DMAed twice? Asserting the FW",
				     current_ts, buf);
			target_if_spectral_fw_hang(spectral);
		}
	}
	spectral->prev_tstamp = current_ts;
}
#endif /* OPTIMIZED_SAMP_MESSAGE */
#else
static void target_if_spectral_check_buffer_poisoning(
	struct target_if_spectral *spectral,
	struct spectral_report *report,
	int num_fft_bins, enum spectral_scan_mode smode)
{
}

#ifdef OPTIMIZED_SAMP_MESSAGE
static void target_if_spectral_verify_ts(struct target_if_spectral *spectral,
					 uint8_t *buf, uint32_t current_ts,
					 uint8_t detector_id)
{
}
#else
static void target_if_spectral_verify_ts(struct target_if_spectral *spectral,
					 uint8_t *buf, uint32_t current_ts)
{
}
#endif /* OPTIMIZED_SAMP_MESSAGE */
#endif

/**
 * target_if_spectral_get_adjusted_timestamp() - Adjust Spectral time
 * stamp to account for reset in time stamp due to target reset
 * @twar: Spectral time stamp WAR related information
 * @raw_timestamp: Spectral time stamp reported by target
 * @reset_delay: Reset delay at target
 * @smode: Spectral scan mode
 *
 * Correct time stamp to account for reset in time stamp due to target reset
 *
 * Return: Adjusted time stamp
 */
static uint32_t
target_if_spectral_get_adjusted_timestamp(struct spectral_timestamp_war *twar,
					  uint32_t raw_timestamp,
					  uint32_t reset_delay,
					  enum spectral_scan_mode smode) {
	qdf_assert_always(smode < SPECTRAL_SCAN_MODE_MAX);

	if (reset_delay) {
		enum spectral_scan_mode m =
					SPECTRAL_SCAN_MODE_NORMAL;

		/* Adjust the offset for all the Spectral modes.
		 * Target will be sending the non zero reset delay for
		 * the first Spectral report after reset. This delay is
		 * common for all the Spectral modes.
		 */
		for (; m < SPECTRAL_SCAN_MODE_MAX; m++)
			twar->timestamp_war_offset[m] += (reset_delay +
					twar->last_fft_timestamp[m]);
		twar->target_reset_count++;
	}
	twar->last_fft_timestamp[smode] = raw_timestamp;

	return raw_timestamp + twar->timestamp_war_offset[smode];
}

#ifdef BIG_ENDIAN_HOST
QDF_STATUS target_if_byte_swap_spectral_headers_gen3(
	 struct target_if_spectral *spectral,
	 void *data)
{
	int i;
	uint32_t *ptr32;
	size_t words32;

	qdf_assert_always(data);
	qdf_assert_always(spectral);

	ptr32 = (uint32_t *)data;

	/* Summary Report */
	words32 = sizeof(struct spectral_sscan_summary_report_gen3) >> 2;
	for (i = 0; i < words32; ++i) {
		*ptr32 = qdf_le32_to_cpu(*ptr32);
		++ptr32;
	}

	/* No need to swap the padding bytes */
	ptr32 += (spectral->rparams.ssumaary_padding_bytes >> 2);

	/* Search FFT Report */
	words32 = sizeof(struct spectral_phyerr_fft_report_gen3) >> 2;
	for (i = 0; i < words32; ++i) {
		*ptr32 = qdf_le32_to_cpu(*ptr32);
		++ptr32;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS target_if_byte_swap_spectral_fft_bins_gen3(
	const struct spectral_report_params *rparams,
	void *bin_pwr_data, size_t num_fftbins)
{
	uint16_t dword_idx, num_dwords;
	uint8_t num_bins_per_dword;
	uint32_t *dword_ptr;

	qdf_assert_always(bin_pwr_data);
	qdf_assert_always(rparams);

	num_bins_per_dword = SPECTRAL_DWORD_SIZE / rparams->hw_fft_bin_width;
	num_dwords = num_fftbins / num_bins_per_dword;
	dword_ptr = (uint32_t *)bin_pwr_data;

	for (dword_idx = 0; dword_idx < num_dwords; dword_idx++) {
		/* Read a DWORD, byteswap it, and copy it back */
		*dword_ptr = qdf_le32_to_cpu(*dword_ptr);
		++dword_ptr;
	}

	return QDF_STATUS_SUCCESS;
}
#endif /* BIG_ENDIAN_HOST */

#ifdef OPTIMIZED_SAMP_MESSAGE
/**
 * target_if_consume_sscan_summary_report_gen3() - Consume Spectral summary
 * report
 * @data: Pointer to Spectral summary report
 * @fields: Pointer to structure to be populated with extracted fields
 * @spectral: Pointer to spectral object
 *
 * Consume Spectral summary report for gen3
 *
 * Return: Success/Failure
 */
static QDF_STATUS
target_if_consume_sscan_summary_report_gen3(
				uint8_t **data,
				struct sscan_report_fields_gen3 *fields,
				struct target_if_spectral *spectral)
{
	struct spectral_sscan_summary_report_gen3 *psscan_summary_report;

	if (!data) {
		spectral_err_rl("Summary report buffer is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!fields) {
		spectral_err_rl("Invalid pointer to Summary report fields");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* Validate Spectral scan summary report */
	if (target_if_verify_sig_and_tag_gen3(
			spectral, *data,
			TLV_TAG_SPECTRAL_SUMMARY_REPORT_GEN3) != 0) {
		spectral_err_rl("Wrong tag/sig in sscan summary");
		return QDF_STATUS_E_FAILURE;
	}

	fields->sscan_detector_id =
		target_if_get_detector_id_sscan_summary_report_gen3(*data);
	if (fields->sscan_detector_id >=
	    spectral->rparams.num_spectral_detectors) {
		spectral->diag_stats.spectral_invalid_detector_id++;
		spectral_err_rl("Invalid detector id %u, expected is 0 to %u",
				fields->sscan_detector_id,
				spectral->rparams.num_spectral_detectors);
		return QDF_STATUS_E_FAILURE;
	}

	psscan_summary_report =
		(struct spectral_sscan_summary_report_gen3 *)*data;

	fields->sscan_agc_total_gain = get_bitfield(
			psscan_summary_report->hdr_a,
			SSCAN_SUMMARY_REPORT_HDR_A_AGC_TOTAL_GAIN_SIZE_GEN3,
			SSCAN_SUMMARY_REPORT_HDR_A_AGC_TOTAL_GAIN_POS_GEN3);
	fields->inband_pwr_db = get_bitfield(
			psscan_summary_report->hdr_a,
			SSCAN_SUMMARY_REPORT_HDR_A_INBAND_PWR_DB_SIZE_GEN3,
			SSCAN_SUMMARY_REPORT_HDR_A_INBAND_PWR_DB_POS_GEN3);
	fields->sscan_pri80 = get_bitfield(
			psscan_summary_report->hdr_a,
			SSCAN_SUMMARY_REPORT_HDR_A_PRI80_SIZE_GEN3,
			SSCAN_SUMMARY_REPORT_HDR_A_PRI80_POS_GEN3);

	switch (spectral->rparams.version) {
	case SPECTRAL_REPORT_FORMAT_VERSION_1:
		fields->sscan_gainchange = get_bitfield(
			psscan_summary_report->hdr_b,
			SSCAN_SUMMARY_REPORT_HDR_B_GAINCHANGE_SIZE_GEN3_V1,
			SSCAN_SUMMARY_REPORT_HDR_B_GAINCHANGE_POS_GEN3_V1);
		break;
	case SPECTRAL_REPORT_FORMAT_VERSION_2:
		fields->sscan_gainchange = get_bitfield(
			psscan_summary_report->hdr_c,
			SSCAN_SUMMARY_REPORT_HDR_C_GAINCHANGE_SIZE_GEN3_V2,
			SSCAN_SUMMARY_REPORT_HDR_C_GAINCHANGE_POS_GEN3_V2);
		break;
	default:
		qdf_assert_always(0);
	}

	/* Advance buf pointer to the search fft report */
	*data += sizeof(struct spectral_sscan_summary_report_gen3);
	*data += spectral->rparams.ssumaary_padding_bytes;

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_process_sfft_report_gen3() - Validate and Process Search
 * FFT Report for gen3
 * @data: Pointer to Spectral FFT report
 * @p_sfft: Pointer to search fft report
 * @spectral: Pointer to spectral object
 * @sscan_detector_id: Spectral detector id extracted from Summary report
 * @reset_delay: Time taken for warm reset in usec
 *
 * Validate and Process Search FFT Report for gen3
 *
 * Return: Success/Failure
 */
static QDF_STATUS
target_if_process_sfft_report_gen3(
	uint8_t *data,
	struct spectral_search_fft_info_gen3 *p_sfft,
	struct target_if_spectral *spectral,
	enum spectral_detector_id sscan_detector_id,
	uint32_t reset_delay)
{
	struct spectral_phyerr_fft_report_gen3 *p_fft_report;
	int32_t peak_sidx = 0;
	int32_t peak_mag;
	int fft_hdr_length = 0;
	struct target_if_spectral_ops *p_sops;
	enum spectral_scan_mode spectral_mode;
	QDF_STATUS ret;

	if (!data) {
		spectral_err_rl("FFT report buffer is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!p_sfft) {
		spectral_err_rl("Invalid pointer to Search FFT report info");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/*
	 * For easy comparision between MDK team and OS team, the MDK script
	 * variable names have been used
	 */

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);

	/* Validate Spectral search FFT report */
	if (target_if_verify_sig_and_tag_gen3(
			spectral, data, TLV_TAG_SEARCH_FFT_REPORT_GEN3) != 0) {
		spectral_err_rl("Unexpected tag/sig in sfft, detid= %u",
				sscan_detector_id);
		return QDF_STATUS_E_FAILURE;
	}

	p_fft_report = (struct spectral_phyerr_fft_report_gen3 *)data;

	fft_hdr_length = get_bitfield(
			p_fft_report->fft_hdr_lts,
			SPECTRAL_REPORT_LTS_HDR_LENGTH_SIZE_GEN3,
			SPECTRAL_REPORT_LTS_HDR_LENGTH_POS_GEN3) * 4;
	if (fft_hdr_length < 16) {
		spectral_err("Wrong TLV length %u, detector id = %d",
			     fft_hdr_length, sscan_detector_id);
		return QDF_STATUS_E_FAILURE;
	}

	p_sfft->fft_detector_id = get_bitfield(
					p_fft_report->hdr_a,
					FFT_REPORT_HDR_A_DETECTOR_ID_SIZE_GEN3,
					FFT_REPORT_HDR_A_DETECTOR_ID_POS_GEN3);

	/* It is expected to have same detector id for
	 * summary and fft report
	 */
	if (sscan_detector_id != p_sfft->fft_detector_id) {
		spectral_err_rl("Different detid in ssummary(%u) and sfft(%u)",
				sscan_detector_id, p_sfft->fft_detector_id);
		return QDF_STATUS_E_FAILURE;
	}

	if (p_sfft->fft_detector_id >
				spectral->rparams.num_spectral_detectors) {
		spectral->diag_stats.spectral_invalid_detector_id++;
		spectral_err("Invalid detector id %u, expected is 0 to %u",
			     p_sfft->fft_detector_id,
			     spectral->rparams.num_spectral_detectors);
		return QDF_STATUS_E_FAILURE;
	}

	spectral_mode = target_if_get_spectral_mode(p_sfft->fft_detector_id,
						    &spectral->rparams);
	if (spectral_mode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("No valid Spectral mode for detector id %u",
				p_sfft->fft_detector_id);
		return QDF_STATUS_E_FAILURE;
	}

	/* Populate the Search FFT Info */
	p_sfft->timestamp = p_fft_report->fft_timestamp;
	p_sfft->last_raw_timestamp = spectral->timestamp_war.
					last_fft_timestamp[spectral_mode];
	p_sfft->adjusted_timestamp = target_if_spectral_get_adjusted_timestamp(
						&spectral->timestamp_war,
						p_sfft->timestamp,
						reset_delay,
						spectral_mode);
	/* Timestamp verification */
	target_if_spectral_verify_ts(spectral, data,
				     p_sfft->adjusted_timestamp,
				     p_sfft->fft_detector_id);


	p_sfft->fft_num = get_bitfield(p_fft_report->hdr_a,
				       FFT_REPORT_HDR_A_FFT_NUM_SIZE_GEN3,
				       FFT_REPORT_HDR_A_FFT_NUM_POS_GEN3);

	switch (spectral->rparams.version) {
	case SPECTRAL_REPORT_FORMAT_VERSION_1:
		p_sfft->fft_radar_check = get_bitfield(p_fft_report->hdr_a,
				FFT_REPORT_HDR_A_RADAR_CHECK_SIZE_GEN3_V1,
				FFT_REPORT_HDR_A_RADAR_CHECK_POS_GEN3_V1);
		peak_sidx = get_bitfield(
				p_fft_report->hdr_a,
				FFT_REPORT_HDR_A_PEAK_INDEX_SIZE_GEN3_V1,
				FFT_REPORT_HDR_A_PEAK_INDEX_POS_GEN3_V1);
		p_sfft->fft_chn_idx = get_bitfield(p_fft_report->hdr_a,
				FFT_REPORT_HDR_A_CHAIN_INDEX_SIZE_GEN3_V1,
				FFT_REPORT_HDR_A_CHAIN_INDEX_POS_GEN3_V1);
		p_sfft->fft_base_pwr_db = get_bitfield(p_fft_report->hdr_b,
				FFT_REPORT_HDR_B_BASE_PWR_SIZE_GEN3_V1,
				FFT_REPORT_HDR_B_BASE_PWR_POS_GEN3_V1);
		p_sfft->fft_total_gain_db = get_bitfield(p_fft_report->hdr_b,
				FFT_REPORT_HDR_B_TOTAL_GAIN_SIZE_GEN3_V1,
				FFT_REPORT_HDR_B_TOTAL_GAIN_POS_GEN3_V1);
		break;
	case SPECTRAL_REPORT_FORMAT_VERSION_2:
		p_sfft->fft_radar_check = get_bitfield(p_fft_report->hdr_a,
				FFT_REPORT_HDR_A_RADAR_CHECK_SIZE_GEN3_V2,
				FFT_REPORT_HDR_A_RADAR_CHECK_POS_GEN3_V2);
		peak_sidx = get_bitfield(
				p_fft_report->hdr_a,
				FFT_REPORT_HDR_A_PEAK_INDEX_SIZE_GEN3_V2,
				FFT_REPORT_HDR_A_PEAK_INDEX_POS_GEN3_V2);
		p_sfft->fft_chn_idx = get_bitfield(p_fft_report->hdr_b,
				FFT_REPORT_HDR_B_CHAIN_INDEX_SIZE_GEN3_V2,
				FFT_REPORT_HDR_B_CHAIN_INDEX_POS_GEN3_V2);
		p_sfft->fft_base_pwr_db = get_bitfield(p_fft_report->hdr_b,
				FFT_REPORT_HDR_B_BASE_PWR_SIZE_GEN3_V2,
				FFT_REPORT_HDR_B_BASE_PWR_POS_GEN3_V2);
		p_sfft->fft_total_gain_db = get_bitfield(p_fft_report->hdr_b,
				FFT_REPORT_HDR_B_TOTAL_GAIN_SIZE_GEN3_V2,
				FFT_REPORT_HDR_B_TOTAL_GAIN_POS_GEN3_V2);
		break;
	default:
		qdf_assert_always(0);
	}

	p_sfft->fft_peak_sidx = unsigned_to_signed(peak_sidx,
				FFT_REPORT_HDR_A_PEAK_INDEX_SIZE_GEN3_V1);

	p_sfft->fft_num_str_bins_ib = get_bitfield(p_fft_report->hdr_c,
				FFT_REPORT_HDR_C_NUM_STRONG_BINS_SIZE_GEN3,
				FFT_REPORT_HDR_C_NUM_STRONG_BINS_POS_GEN3);
	peak_mag = get_bitfield(p_fft_report->hdr_c,
				FFT_REPORT_HDR_C_PEAK_MAGNITUDE_SIZE_GEN3,
				FFT_REPORT_HDR_C_PEAK_MAGNITUDE_POS_GEN3);
	p_sfft->fft_peak_mag = unsigned_to_signed(peak_mag,
				FFT_REPORT_HDR_C_PEAK_MAGNITUDE_SIZE_GEN3);
	p_sfft->fft_avgpwr_db = get_bitfield(p_fft_report->hdr_c,
				FFT_REPORT_HDR_C_AVG_PWR_SIZE_GEN3,
				FFT_REPORT_HDR_C_AVG_PWR_POS_GEN3);
	p_sfft->fft_relpwr_db = get_bitfield(p_fft_report->hdr_c,
				FFT_REPORT_HDR_C_RELATIVE_PWR_SIZE_GEN3,
				FFT_REPORT_HDR_C_RELATIVE_PWR_POS_GEN3);

	p_sfft->fft_bin_count =
		target_if_spectral_get_bin_count_after_len_adj(
			fft_hdr_length - spectral->rparams.fft_report_hdr_len,
			spectral->params[spectral_mode].ss_rpt_mode,
			&spectral->len_adj_swar,
			(size_t *)&p_sfft->fft_bin_size);

	p_sfft->bin_pwr_data = (uint8_t *)p_fft_report + SPECTRAL_FFT_BINS_POS;

	/* Apply byte-swap on the FFT bins.
	 * NOTE: Until this point, bytes of the FFT bins could be in
	 *       reverse order on a big-endian machine. If the consumers
	 *       of FFT bins expects bytes in the correct order,
	 *       they should use them only after this point.
	 */
	if (p_sops->byte_swap_fft_bins) {
		ret = p_sops->byte_swap_fft_bins(&spectral->rparams,
						 &p_sfft->bin_pwr_data,
						 p_sfft->fft_bin_count);
		if (QDF_IS_STATUS_ERROR(ret)) {
			spectral_err_rl("Byte-swap on the FFT bins failed");
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * target_if_spectral_populate_samp_params_gen3() - Populate the SAMP params
 * for gen3. SAMP params are to be used for populating SAMP msg.
 * @spectral: Pointer to spectral object
 * @p_sfft: Fields extracted from FFT report
 * @sscan_fields: Fields extracted from Summary report
 * @report: Pointer to spectral report
 * @params: Pointer to Spectral SAMP message fields to be populated
 *
 * Populate the SAMP params for gen3, which will be used to populate SAMP msg.
 *
 * Return: Success/Failure
 */
static QDF_STATUS
target_if_spectral_populate_samp_params_gen3(
		struct target_if_spectral *spectral,
		struct spectral_search_fft_info_gen3 *p_sfft,
		struct sscan_report_fields_gen3 *sscan_fields,
		struct spectral_report *report,
		struct target_if_samp_msg_params *params)
{
	enum spectral_scan_mode spectral_mode;
	uint8_t chn_idx_lowest_enabled;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_rxchainmask;

	if (!p_sfft) {
		spectral_err_rl("Invalid pointer to Search FFT report info");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!sscan_fields) {
		spectral_err_rl("Invalid pointer to Summary report fields");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!report) {
		spectral_err_rl("Spectral report is null");
		return QDF_STATUS_E_NULL_VALUE;
	}
	if (!params) {
		spectral_err_rl("SAMP msg params structure is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* RSSI is in 1/2 dBm steps, Covert it to dBm scale */
	params->rssi = (sscan_fields->inband_pwr_db) >> 1;

	params->hw_detector_id = p_sfft->fft_detector_id;
	params->raw_timestamp = p_sfft->timestamp;
	params->last_raw_timestamp = p_sfft->last_raw_timestamp;
	params->timestamp = p_sfft->adjusted_timestamp;
	params->reset_delay = report->reset_delay;

	params->max_mag = p_sfft->fft_peak_mag;

	spectral_mode = target_if_get_spectral_mode(params->hw_detector_id,
						    &spectral->rparams);
	vdev = target_if_spectral_get_vdev(spectral, spectral_mode);
	if (!vdev) {
		spectral_debug("First vdev is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	vdev_rxchainmask = wlan_vdev_mlme_get_rxchainmask(vdev);
	QDF_ASSERT(vdev_rxchainmask != 0);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

	chn_idx_lowest_enabled =
		target_if_spectral_get_lowest_chn_idx(vdev_rxchainmask);
	if (chn_idx_lowest_enabled >= DBR_MAX_CHAINS) {
		spectral_err("Invalid chain index, detector id = %u",
			     params->hw_detector_id);
		return QDF_STATUS_E_FAILURE;
	}
	params->noise_floor = report->noisefloor[chn_idx_lowest_enabled];
	params->agc_total_gain = sscan_fields->sscan_agc_total_gain;
	params->gainchange = sscan_fields->sscan_gainchange;
	params->pri80ind = sscan_fields->sscan_pri80;

	params->bin_pwr_data = p_sfft->bin_pwr_data;

	return QDF_STATUS_SUCCESS;
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
	 *          1. Frequency
	 *          2. Spectral RSSI
	 *          3. Bin Power Count
	 *          4. Bin Power values
	 *          5. Spectral Timestamp
	 *          6. MAC Address
	 *
	 *       This function processes the Spectral summary and FFT reports
	 *       and passes the processed information
	 *       target_if_spectral_fill_samp_msg()
	 *       to prepare fully formatted Spectral SAMP message
	 *
	 *       XXX : Need to verify
	 *          1. Order of FFT bin values
	 *
	 */
	struct target_if_samp_msg_params params = {0};
	struct spectral_search_fft_info_gen3 search_fft_info;
	struct spectral_search_fft_info_gen3 *p_sfft = &search_fft_info;
	struct target_if_spectral_ops *p_sops;
	struct spectral_phyerr_fft_report_gen3 *p_fft_report;
	uint8_t *data;
	struct sscan_report_fields_gen3 sscan_report_fields = {0};
	QDF_STATUS ret;
	enum spectral_scan_mode spectral_mode = SPECTRAL_SCAN_MODE_INVALID;
	bool finite_scan = false;
	int det = 0;
	struct sscan_detector_list *det_list;

	if (!spectral) {
		spectral_err_rl("Spectral LMAC object is null");
		goto fail_no_print;
	}

	if (!report) {
		spectral_err_rl("Spectral report is null");
		goto fail_no_print;
	}

	p_sops = GET_TARGET_IF_SPECTRAL_OPS(spectral);
	data = report->data;

	/* Apply byte-swap on the headers */
	if (p_sops->byte_swap_headers) {
		ret = p_sops->byte_swap_headers(spectral, data);
		if (QDF_IS_STATUS_ERROR(ret)) {
			spectral_err_rl("Byte-swap on Spectral headers failed");
			goto fail;
		}
	}

	/* Validate and Process Spectral scan summary report */
	ret = target_if_consume_sscan_summary_report_gen3(&data,
							  &sscan_report_fields,
							  spectral);
	if (QDF_IS_STATUS_ERROR(ret)) {
		spectral_err_rl("Failed to process Spectral summary report");
		goto fail;
	}

	spectral_mode = target_if_get_spectral_mode(
					sscan_report_fields.sscan_detector_id,
					&spectral->rparams);
	if (spectral_mode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("No valid Spectral mode for detector id %u",
				sscan_report_fields.sscan_detector_id);
		goto fail;
	}

	/* Drop the sample if Spectral is not active for the current mode */
	if (!p_sops->is_spectral_active(spectral, spectral_mode)) {
		spectral_info_rl("Spectral scan is not active");
		goto fail_no_print;
	}

	ret = target_if_spectral_is_finite_scan(spectral, spectral_mode,
						&finite_scan);
	if (QDF_IS_STATUS_ERROR(ret)) {
		spectral_err_rl("Failed to check scan is finite");
		goto fail;
	}

	if (finite_scan) {
		ret = target_if_spectral_finite_scan_update(spectral,
							    spectral_mode);
		if (QDF_IS_STATUS_ERROR(ret)) {
			spectral_err_rl("Failed to update scan count");
			goto fail;
		}
	}

	/* Validate and Process the search FFT report */
	ret = target_if_process_sfft_report_gen3(
					data, p_sfft,
					spectral,
					sscan_report_fields.sscan_detector_id,
					report->reset_delay);
	if (QDF_IS_STATUS_ERROR(ret)) {
		spectral_err_rl("Failed to process search FFT report");
		goto fail;
	}

	qdf_spin_lock_bh(&spectral->detector_list_lock);
	det_list = &spectral->detector_list[spectral_mode]
			[spectral->report_info[spectral_mode].sscan_bw];
	for (det = 0; det < det_list->num_detectors; det++) {
		if (p_sfft->fft_detector_id == det_list->detectors[det])
			break;
		if (det == det_list->num_detectors - 1) {
			qdf_spin_unlock_bh(&spectral->detector_list_lock);
			spectral_info("Incorrect det id %d for given scan mode and channel width",
				      p_sfft->fft_detector_id);
			goto fail_no_print;
		}
	}
	qdf_spin_unlock_bh(&spectral->detector_list_lock);

	ret = target_if_update_session_info_from_report_ctx(
						spectral,
						p_sfft->fft_bin_size,
						report->cfreq1, report->cfreq2,
						spectral_mode);
	if (QDF_IS_STATUS_ERROR(ret)) {
		spectral_err_rl("Failed to update per-session info");
		goto fail;
	}

	qdf_spin_lock_bh(&spectral->session_report_info_lock);
	/* Check FFT report are in order for 160 MHz and 80p80 */
	if (is_ch_width_160_or_80p80(
	    spectral->report_info[spectral_mode].sscan_bw) &&
	    spectral->rparams.fragmentation_160[spectral_mode]) {
		ret = target_if_160mhz_delivery_state_change(
				spectral, spectral_mode,
				p_sfft->fft_detector_id);
		if (ret != QDF_STATUS_SUCCESS) {
			qdf_spin_unlock_bh(
					&spectral->session_report_info_lock);
			goto fail;
		}
	}
	qdf_spin_unlock_bh(&spectral->session_report_info_lock);

	p_fft_report = (struct spectral_phyerr_fft_report_gen3 *)data;
	if (spectral_debug_level & (DEBUG_SPECTRAL2 | DEBUG_SPECTRAL4))
		target_if_dump_fft_report_gen3(spectral, spectral_mode,
					       p_fft_report, p_sfft);

	target_if_spectral_check_buffer_poisoning(spectral, report,
						  p_sfft->fft_bin_count,
						  spectral_mode);

	/* Populate SAMP params */
	ret = target_if_spectral_populate_samp_params_gen3(
							spectral, p_sfft,
							&sscan_report_fields,
							report, &params);
	if (QDF_IS_STATUS_ERROR(ret)) {
		spectral_err_rl("Failed to populate SAMP params");
		goto fail;
	}
	/* Fill SAMP message */
	ret = target_if_spectral_fill_samp_msg(spectral, &params);
	if (QDF_IS_STATUS_ERROR(ret)) {
		spectral_err_rl("Failed to fill the SAMP msg");
		goto fail;
	}

	return 0;
 fail:
	spectral_err_rl("Error while processing Spectral report");
fail_no_print:
	if (spectral_mode != SPECTRAL_SCAN_MODE_INVALID)
		reset_160mhz_delivery_state_machine(spectral, spectral_mode);
	return -EPERM;
}

#else
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
	struct target_if_samp_msg_params params = {0};
	struct spectral_search_fft_info_gen3 search_fft_info;
	struct spectral_search_fft_info_gen3 *p_sfft = &search_fft_info;
	int8_t chn_idx_lowest_enabled  = 0;
	int fft_hdr_length = 0;
	int report_len = 0;
	size_t fft_bin_count;
	size_t fft_bin_size;
	struct target_if_spectral_ops *p_sops =
		GET_TARGET_IF_SPECTRAL_OPS(spectral);
	struct spectral_phyerr_fft_report_gen3 *p_fft_report;
	int8_t rssi;
	uint8_t *data = report->data;
	struct wlan_objmgr_vdev *vdev;
	uint8_t vdev_rxchainmask;
	struct sscan_report_fields_gen3 sscan_report_fields = {0};
	enum spectral_detector_id detector_id;
	QDF_STATUS ret;
	enum spectral_scan_mode spectral_mode = SPECTRAL_SCAN_MODE_INVALID;
	uint8_t *temp;
	bool finite_scan = false;

	/* Apply byte-swap on the headers */
	if (p_sops->byte_swap_headers) {
		ret = p_sops->byte_swap_headers(spectral, data);
		if (QDF_IS_STATUS_ERROR(ret)) {
			spectral_err_rl("Byte-swap on Spectral headers failed");
			goto fail;
		}
	}

	/* Process Spectral scan summary report */
	if (target_if_verify_sig_and_tag_gen3(
			spectral, data,
			TLV_TAG_SPECTRAL_SUMMARY_REPORT_GEN3) != 0) {
		spectral_err_rl("Wrong tag/sig in sscan summary");
		goto fail;
	}

	detector_id = target_if_get_detector_id_sscan_summary_report_gen3(data);
	if (detector_id >= spectral->rparams.num_spectral_detectors) {
		spectral->diag_stats.spectral_invalid_detector_id++;
		spectral_err("Invalid detector id %u, expected is 0/1/2",
			     detector_id);
		goto fail;
	}

	spectral_mode = target_if_get_spectral_mode(detector_id,
						    &spectral->rparams);
	if (spectral_mode >= SPECTRAL_SCAN_MODE_MAX) {
		spectral_err_rl("No valid Spectral mode for detector id %u",
				detector_id);
		goto fail;
	}

	/* Drop the sample if Spectral is not active for the current mode */
	if (!p_sops->is_spectral_active(spectral, spectral_mode)) {
		spectral_info_rl("Spectral scan is not active");
		goto fail_no_print;
	}

	ret = target_if_spectral_is_finite_scan(spectral, spectral_mode,
						&finite_scan);
	if (QDF_IS_STATUS_ERROR(ret)) {
		spectral_err_rl("Failed to check scan is finite");
		goto fail;
	}

	if (finite_scan) {
		ret = target_if_spectral_finite_scan_update(spectral,
							    spectral_mode);
		if (QDF_IS_STATUS_ERROR(ret)) {
			spectral_err_rl("Failed to update scan count");
			goto fail;
		}
	}

	target_if_consume_sscan_summary_report_gen3(data, &sscan_report_fields,
						    &spectral->rparams);
	/* Advance buf pointer to the search fft report */
	data += sizeof(struct spectral_sscan_summary_report_gen3);
	data += spectral->rparams.ssumaary_padding_bytes;
	params.vhtop_ch_freq_seg1 = report->cfreq1;
	params.vhtop_ch_freq_seg2 = report->cfreq2;

	if (is_primaryseg_expected(spectral, spectral_mode)) {
		/* RSSI is in 1/2 dBm steps, Covert it to dBm scale */
		rssi = (sscan_report_fields.inband_pwr_db) >> 1;
		params.agc_total_gain =
			sscan_report_fields.sscan_agc_total_gain;
		params.gainchange = sscan_report_fields.sscan_gainchange;
		params.pri80ind = sscan_report_fields.sscan_pri80;

		/* Process Spectral search FFT report */
		if (target_if_verify_sig_and_tag_gen3(
				spectral, data,
				TLV_TAG_SEARCH_FFT_REPORT_GEN3) != 0) {
			spectral_err_rl("Unexpected tag/sig in sfft, detid= %u",
					detector_id);
			goto fail;
		}
		p_fft_report = (struct spectral_phyerr_fft_report_gen3 *)data;
		fft_hdr_length = get_bitfield(
				p_fft_report->fft_hdr_lts,
				SPECTRAL_REPORT_LTS_HDR_LENGTH_SIZE_GEN3,
				SPECTRAL_REPORT_LTS_HDR_LENGTH_POS_GEN3) * 4;
		if (fft_hdr_length < 16) {
			spectral_err("Wrong TLV length %u, detector id = %d",
				     fft_hdr_length, detector_id);
			goto fail;
		}

		report_len = (fft_hdr_length + 8);

		target_if_process_sfft_report_gen3(p_fft_report, p_sfft,
						   &spectral->rparams);
		/* It is expected to have same detector id for
		 * summary and fft report
		 */
		if (detector_id != p_sfft->fft_detector_id) {
			spectral_err_rl
				("Different detid in ssummary(%u) and sfft(%u)",
				 detector_id, p_sfft->fft_detector_id);
			goto fail;
		}

		if (detector_id > spectral->rparams.num_spectral_detectors) {
			spectral->diag_stats.spectral_invalid_detector_id++;
			spectral_err("Invalid detector id %u, expected is 0/2",
				     detector_id);
			goto fail;
		}
		params.smode = spectral_mode;

		fft_bin_count = target_if_spectral_get_bin_count_after_len_adj(
			fft_hdr_length - spectral->rparams.fft_report_hdr_len,
			spectral->params[spectral_mode].ss_rpt_mode,
			&spectral->len_adj_swar, &fft_bin_size);

		params.last_raw_timestamp = spectral->timestamp_war.
				last_fft_timestamp[spectral_mode];
		params.reset_delay = report->reset_delay;
		params.raw_timestamp = p_sfft->timestamp;
		params.tstamp = target_if_spectral_get_adjusted_timestamp(
					&spectral->timestamp_war,
					p_sfft->timestamp, report->reset_delay,
					spectral_mode);
		params.timestamp_war_offset = spectral->timestamp_war.
				timestamp_war_offset[spectral_mode];
		params.target_reset_count = spectral->timestamp_war.
				target_reset_count;

		/* Take care of state transitions for 160 MHz and 80p80 */
		if (is_ch_width_160_or_80p80(spectral->ch_width
		    [spectral_mode]) && spectral->rparams.
		    fragmentation_160[spectral_mode]) {
			ret = target_if_160mhz_delivery_state_change(
					spectral, spectral_mode,
					detector_id);
			if (ret != QDF_STATUS_SUCCESS)
				goto fail;
		}

		params.rssi         = rssi;

		vdev = target_if_spectral_get_vdev(spectral, spectral_mode);
		if (!vdev) {
			spectral_debug("First vdev is NULL");
			reset_160mhz_delivery_state_machine(
						spectral, spectral_mode);
			return -EPERM;
		}
		vdev_rxchainmask = wlan_vdev_mlme_get_rxchainmask(vdev);
		QDF_ASSERT(vdev_rxchainmask != 0);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

		chn_idx_lowest_enabled =
		target_if_spectral_get_lowest_chn_idx(vdev_rxchainmask);
		if (chn_idx_lowest_enabled >= DBR_MAX_CHAINS) {
			spectral_err("Invalid chain index, detector id = %u",
				     detector_id);
			goto fail;
		}

		params.max_mag  = p_sfft->fft_peak_mag;

		params.freq = p_sops->get_current_channel(spectral,
							  spectral_mode);
		params.agile_freq1 = spectral->params[SPECTRAL_SCAN_MODE_AGILE].
				     ss_frequency.cfreq1;
		params.agile_freq2 = spectral->params[SPECTRAL_SCAN_MODE_AGILE].
				     ss_frequency.cfreq2;
		params.noise_floor =
			report->noisefloor[chn_idx_lowest_enabled];
		temp = (uint8_t *)p_fft_report + SPECTRAL_FFT_BINS_POS;
		if (is_ch_width_160_or_80p80(spectral->ch_width
		    [spectral_mode]) && !spectral->rparams.
		    fragmentation_160[spectral_mode]) {
			struct wlan_objmgr_psoc *psoc;
			struct spectral_fft_bin_markers_160_165mhz *marker;

			qdf_assert_always(spectral->pdev_obj);
			psoc = wlan_pdev_get_psoc(spectral->pdev_obj);
			qdf_assert_always(psoc);

			params.agc_total_gain_sec80 =
				sscan_report_fields.sscan_agc_total_gain;
			params.gainchange_sec80 =
					sscan_report_fields.sscan_gainchange;
			params.raw_timestamp_sec80 = p_sfft->timestamp;
			params.rssi_sec80 = rssi;
			params.noise_floor_sec80    =
				report->noisefloor[chn_idx_lowest_enabled];
			params.max_mag_sec80        = p_sfft->fft_peak_mag;
			params.datalen = fft_hdr_length * 2;
			params.datalen_sec80 = fft_hdr_length * 2;

			marker = &spectral->rparams.marker[spectral_mode];
			if (!marker->is_valid) {
				/* update stats */
				goto fail_no_print;
			}
			params.bin_pwr_data = temp +
				marker->start_pri80 * fft_bin_size;
			params.pwr_count = marker->num_pri80;
			params.bin_pwr_data_sec80 = temp +
				marker->start_sec80 * fft_bin_size;
			params.pwr_count_sec80 = marker->num_sec80;
			if (spectral->ch_width[spectral_mode] ==
			    CH_WIDTH_80P80MHZ && wlan_psoc_nif_fw_ext_cap_get(
			    psoc, WLAN_SOC_RESTRICTED_80P80_SUPPORT)) {
				params.bin_pwr_data_5mhz = temp +
					marker->start_5mhz * fft_bin_size;
				params.pwr_count_5mhz = marker->num_5mhz;
			}
		} else {
			params.bin_pwr_data = temp;
			params.pwr_count = fft_bin_count;
			params.datalen = (fft_hdr_length * 4);
		}

		/* Apply byte-swap on the FFT bins.
		 * NOTE: Until this point, bytes of the FFT bins could be in
		 *       reverse order on a big-endian machine. If the consumers
		 *       of FFT bins expects bytes in the correct order,
		 *       they should use them only after this point.
		 */
		if (p_sops->byte_swap_fft_bins) {
			ret = p_sops->byte_swap_fft_bins(
						&spectral->rparams,
						temp, fft_bin_count);
			if (QDF_IS_STATUS_ERROR(ret)) {
				spectral_err_rl("Byte-swap on the FFT bins failed");
				goto fail;
			}
		}

		if (spectral_debug_level & (DEBUG_SPECTRAL2 | DEBUG_SPECTRAL4))
			target_if_dump_fft_report_gen3(spectral, spectral_mode,
						       p_fft_report, p_sfft);

		target_if_spectral_verify_ts(spectral, report->data,
					     params.tstamp);
	} else if (is_secondaryseg_expected(spectral, spectral_mode)) {
		/* RSSI is in 1/2 dBm steps, Covert it to dBm scale */
		rssi = (sscan_report_fields.inband_pwr_db) >> 1;
		params.agc_total_gain_sec80 =
			sscan_report_fields.sscan_agc_total_gain;
		params.gainchange_sec80 = sscan_report_fields.sscan_gainchange;
		params.pri80ind_sec80 = sscan_report_fields.sscan_pri80;

		/* Process Spectral search FFT report */
		if (target_if_verify_sig_and_tag_gen3(
				spectral, data,
				TLV_TAG_SEARCH_FFT_REPORT_GEN3) != 0) {
			spectral_err_rl("Unexpected tag/sig in sfft, detid= %u",
					detector_id);
			goto fail;
		}
		p_fft_report = (struct spectral_phyerr_fft_report_gen3 *)data;
		fft_hdr_length = get_bitfield(
				p_fft_report->fft_hdr_lts,
				SPECTRAL_REPORT_LTS_HDR_LENGTH_SIZE_GEN3,
				SPECTRAL_REPORT_LTS_HDR_LENGTH_POS_GEN3) * 4;
		if (fft_hdr_length < 16) {
			spectral_err("Wrong TLV length %u, detector id = %u",
				     fft_hdr_length, detector_id);
			goto fail;
		}

		report_len     = (fft_hdr_length + 8);

		target_if_process_sfft_report_gen3(p_fft_report, p_sfft,
						   &spectral->rparams);
		/* It is expected to have same detector id for
		 * summary and fft report
		 */
		if (detector_id != p_sfft->fft_detector_id) {
			spectral_err_rl
				("Different detid in ssummary(%u) and sfft(%u)",
				 detector_id, p_sfft->fft_detector_id);
			goto fail;
		}

		if (detector_id > spectral->rparams.num_spectral_detectors) {
			spectral->diag_stats.spectral_invalid_detector_id++;
			spectral_err("Invalid detector id %u, expected is 1",
				     detector_id);
			goto fail;
		}
		params.smode = spectral_mode;

		fft_bin_count = target_if_spectral_get_bin_count_after_len_adj(
			fft_hdr_length - spectral->rparams.fft_report_hdr_len,
			spectral->params[spectral_mode].ss_rpt_mode,
			&spectral->len_adj_swar, &fft_bin_size);
		params.raw_timestamp_sec80 = p_sfft->timestamp;

		/* Take care of state transitions for 160 MHz and 80p80 */
		if (is_ch_width_160_or_80p80(spectral->ch_width
		    [spectral_mode]) && spectral->rparams.
		    fragmentation_160[spectral_mode]) {
			ret = target_if_160mhz_delivery_state_change(
					spectral, spectral_mode,
					detector_id);
			if (ret != QDF_STATUS_SUCCESS)
				goto fail;
		}

		params.rssi_sec80 = rssi;

		vdev = target_if_spectral_get_vdev(spectral, spectral_mode);
		if (!vdev) {
			spectral_info("First vdev is NULL");
			reset_160mhz_delivery_state_machine
						(spectral, spectral_mode);
			return -EPERM;
		}
		vdev_rxchainmask = wlan_vdev_mlme_get_rxchainmask(vdev);
		QDF_ASSERT(vdev_rxchainmask != 0);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_SPECTRAL_ID);

		chn_idx_lowest_enabled =
		target_if_spectral_get_lowest_chn_idx(vdev_rxchainmask);
		if (chn_idx_lowest_enabled >= DBR_MAX_CHAINS) {
			spectral_err("Invalid chain index");
			goto fail;
		}

		/* Need to change this as per FW team's inputs */
		params.noise_floor_sec80    =
			report->noisefloor[chn_idx_lowest_enabled];

		params.max_mag_sec80        = p_sfft->fft_peak_mag;
		/* params.max_index_sec80      = p_sfft->peak_inx; */
		/* XXX Does this definition of datalen *still hold? */
		params.datalen_sec80        = fft_hdr_length * 4;
		params.pwr_count_sec80      = fft_bin_count;
		params.bin_pwr_data_sec80   =
			(uint8_t *)((uint8_t *)p_fft_report +
			 SPECTRAL_FFT_BINS_POS);

		/* Apply byte-swap on the FFT bins.
		 * NOTE: Until this point, bytes of the FFT bins could be in
		 *       reverse order on a big-endian machine. If the consumers
		 *       of FFT bins expects bytes in the correct order,
		 *       they should use them only after this point.
		 */
		if (p_sops->byte_swap_fft_bins) {
			ret = p_sops->byte_swap_fft_bins(
					&spectral->rparams,
					params.bin_pwr_data_sec80,
					fft_bin_count);
			if (QDF_IS_STATUS_ERROR(ret)) {
				spectral_err_rl("Byte-swap on the FFT bins failed");
				goto fail;
			}
		}

		if (spectral_debug_level & (DEBUG_SPECTRAL2 | DEBUG_SPECTRAL4))
			target_if_dump_fft_report_gen3(spectral, spectral_mode,
						       p_fft_report, p_sfft);

	} else {
		spectral_err("Spectral state machine in undefined state");
		goto fail;
	}

	target_if_spectral_check_buffer_poisoning(spectral, report,
						  fft_bin_count, spectral_mode);
	qdf_mem_copy(&params.classifier_params,
		     &spectral->classifier_params,
		     sizeof(struct spectral_classifier_params));

	target_if_spectral_log_SAMP_param(&params);
	target_if_spectral_create_samp_msg(spectral, &params);

	return 0;
 fail:
	spectral_err_rl("Error while processing Spectral report");
fail_no_print:
	if (spectral_mode != SPECTRAL_SCAN_MODE_INVALID)
		reset_160mhz_delivery_state_machine(spectral, spectral_mode);
	return -EPERM;
}
#endif /* OPTIMIZED_SAMP_MESSAGE */

int target_if_spectral_process_report_gen3(
	struct wlan_objmgr_pdev *pdev,
	void *buf)
{
	int ret = 0;
	struct direct_buf_rx_data *payload = buf;
	struct target_if_spectral *spectral;
	struct spectral_report report;
	int samp_msg_index;

	spectral = get_target_if_spectral_handle_from_pdev(pdev);
	if (!spectral) {
		spectral_err("Spectral target object is null");
		return -EINVAL;
	}

	report.data = payload->vaddr;
	if (payload->meta_data_valid) {
		qdf_mem_copy(report.noisefloor, payload->meta_data.noisefloor,
			     qdf_min(sizeof(report.noisefloor),
				     sizeof(payload->meta_data.noisefloor)));
		report.reset_delay = payload->meta_data.reset_delay;
		report.cfreq1 = payload->meta_data.cfreq1;
		report.cfreq2 = payload->meta_data.cfreq2;
		report.ch_width = payload->meta_data.ch_width;
	}

	if (spectral_debug_level & (DEBUG_SPECTRAL2 | DEBUG_SPECTRAL4)) {
		spectral_debug("Printing the spectral phyerr buffer for debug");
		spectral_debug("Datalength of buffer = 0x%zx(%zd) bufptr = 0x%pK",
			       payload->dbr_len,
			       payload->dbr_len,
			       payload->vaddr);
		target_if_spectral_hexdump((unsigned char *)payload->vaddr,
					   1024);
	}

	samp_msg_index = spectral->spectral_sent_msg;

	ret = target_if_consume_spectral_report_gen3(spectral, &report);

	/* Reset debug level when SAMP msg is sent successfully or on error */
	if ((spectral_debug_level & DEBUG_SPECTRAL4) &&
	    (ret != 0 || spectral->spectral_sent_msg == samp_msg_index + 1))
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
