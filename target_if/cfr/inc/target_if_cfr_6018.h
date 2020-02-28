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

#ifndef _TARGET_IF_CFR_6018_H_
#define _TARGET_IF_CFR_6018_H_

#ifdef WLAN_ENH_CFR_ENABLE
/*
 * Memory requirements :
 *
 *  1. DMA header :
 *
 * Legacy DMA header(QCA8074V2) : 2 words (length = 8 bytes)
 * Enhanced DMA header(QCA6018) : Upto 16 words depending on no. of MU users
 *                       in UL-MU-PPDU (Max length = 64 bytes)
 *
 * Fixed 4 words for whal_cfir_enhanced_hdr + freeze TLV
 *                                          + uplink_user_info TLV (MAX 4)
 *
 * mu_rx_num_users -> No. of words in CFR DMA header
 * 0 -> 12  =  4 + 7(freeze TLV) + 1(for 64-bit alignment)
 * 1 -> 12  =  4 + 7(freeze TLV) + 1(user1)
 * 2 -> 14  =  4 + 7(freeze TLV) + 2(users 1,2) + 1(for 64-bit alignment)
 * 3 -> 14  =  4 + 7(freeze TLV) + 3(users 1,2,3)
 * 4 -> 16  =  4 + 7(freeze TLV) + 4(users 1,2,3,4) + 1(for 64-bit alignment)
 *
 *
 * 2. CFR data size for max BW/Nss/Nrx
 *
 *	Cypress : Max BW = 80 MHz
 *			 NSS = 2
 *			 Nrx = 2
 *			 Size of one tone = 4 bytes
 *
 *		a. RTT-H - 2048 bytes
 *
 *		b. Debug-H (MIMO CFR) - 16016 bytes
 *
 *		c. RTT-H + CIR - 10240 bytes = 2048(RTT-H) + 8192(CIR)
 */

/* Max 4 users in MU case */
#define CYP_CFR_MU_USERS 4

#define CYP_MAX_HEADER_LENGTH_WORDS 16

/* payload_len = Max(2048, 16016, 10240) = 16064 (64-bit alignment) */
#define CYP_MAX_DATA_LENGTH_BYTES 16064

/* in ms */
#define LUT_AGE_TIMER 3000
#define LUT_AGE_THRESHOLD 3000
#define NUM_LUT_ENTRIES 136

/* Max size :
 * 16173 = 93 bytes(csi header) + 64 bytes(cfr header) + 16016 bytes(cfr
 * payload)
 */
#define STREAMFS_MAX_SUBBUF_CYP 16173

#define STREAMFS_NUM_SUBBUF_CYP 255

/*
 * @tag: ucode fills this with 0xBA
 *
 * @length: length of CFR header in words (32-bit)
 *
 * @upload_done: ucode sets this to 1 to indicate DMA completion
 *
 * @capture_type:
 *
 *			0 - None
 *			1 - RTT-H (Nss = 1, Nrx)
 *			2 - Debug-H (Nss, Nrx)
 *			3 - Reserved
 *			5 - RTT-H + CIR(Nss, Nrx)
 *
 * @preamble_type:
 *
 *			0 - Legacy
 *			1 - HT
 *			2 - VHT
 *			3 - HE
 *
 * @nss:
 *
 *			0 - 1-stream
 *			1 - 2-stream
 *			..	..
 *			7 - 8-stream
 *
 *@num_chains:
 *
 *			0 - 1-chain
 *			1 - 2-chain
 *			..  ..
 *			7 - 8-chain
 *
 *@upload_bw_pkt:
 *
 *			0 - 20 MHz
 *			1 - 40 MHz
 *			2 - 80 MHz
 *			3 - 160 MHz
 *
 * @sw_peer_id_valid: Indicates whether sw_peer_id field is valid or not,
 * sent from MAC to PHY via the MACRX_FREEZE_CAPTURE_CHANNEL TLV
 *
 * @sw_peer_id: Indicates peer id based on AST search, sent from MAC to PHY
 * via the MACRX_FREEZE_CAPTURE_CHANNEL TLV
 *
 * @phy_ppdu_id: sent from PHY to MAC, copied to MACRX_FREEZE_CAPTURE_CHANNEL
 * TLV
 *
 * @total_bytes: Total size of CFR payload (FFT bins)
 *
 * @header_version:
 *
 *			1 - HKV2/Hastings
 *			2 - Cypress
 *
 * @target_id:
 *
 *			1 - Hastings
 *			2 - Cypress
 *			3 - Hastings Prime
 *			4 - Pine
 *
 * @cfr_fmt:
 *
 *			0 - raw (32-bit format)
 *			1 - compressed (24-bit format)
 *
 * @mu_rx_data_incl: Indicates whether CFR header contains UL-MU-MIMO info
 *
 * @freeze_data_incl: Indicates whether CFR header contains
 * MACRX_FREEZE_CAPTURE_CHANNEL TLV
 *
 * @decimation_factor: FFT bins decimation
 * @mu_rx_num_users: Number of users in UL-MU-PPDU
 */
struct whal_cfir_enhanced_hdr {
	uint16_t tag              :  8,
		 length           :  6,
		 rsvd1            :  2;

	uint16_t upload_done        :  1,
		 capture_type       :  3,
		 preamble_type      :  2,
		 nss                :  3,
		 num_chains         :  3,
		 upload_pkt_bw      :  3,
		 sw_peer_id_valid   :  1;

	uint16_t sw_peer_id         : 16;

	uint16_t phy_ppdu_id        : 16;

	uint16_t total_bytes;

	uint16_t header_version  :4,
		 target_id       :4,
		 cfr_fmt         :1,
		 rsvd2           :1,
		 mu_rx_data_incl :1,
		 freeze_data_incl:1,
		 rsvd3           :4;

	uint16_t mu_rx_num_users   :8,
		 decimation_factor :4,
		 rsvd4		   :4;

	uint16_t rsvd5;
};

struct macrx_freeze_capture_channel {
	uint16_t freeze                          :  1, //[0]
		 capture_reason                  :  3, //[3:1]
		 packet_type                     :  2, //[5:4]
		 packet_sub_type                 :  4, //[9:6]
		 reserved                        :  5, //[14:10]
		 sw_peer_id_valid                :  1; //[15]
	uint16_t sw_peer_id                      : 16; //[15:0]
	uint16_t phy_ppdu_id                     : 16; //[15:0]
	uint16_t packet_ta_lower_16              : 16; //[15:0]
	uint16_t packet_ta_mid_16                : 16; //[15:0]
	uint16_t packet_ta_upper_16              : 16; //[15:0]
	uint16_t packet_ra_lower_16              : 16; //[15:0]
	uint16_t packet_ra_mid_16                : 16; //[15:0]
	uint16_t packet_ra_upper_16              : 16; //[15:0]
	uint16_t tsf_timestamp_15_0              : 16; //[15:0]
	uint16_t tsf_timestamp_31_16             : 16; //[15:0]
	uint16_t tsf_timestamp_47_32             : 16; //[15:0]
	uint16_t tsf_timestamp_63_48             : 16; //[15:0]
	uint16_t user_index                      :  6, //[5:0]
		 directed                        :  1, //[6]
		 reserved_13                     :  9; //[15:7]
};

struct uplink_user_setup_info {
	uint32_t bw_info_valid                   :  1, //[0]
		 uplink_receive_type             :  2, //[2:1]
		 reserved_0a                     :  1, //[3]
		 uplink_11ax_mcs                 :  4, //[7:4]
		 ru_width                        :  7, //[14:8]
		 reserved_0b                     :  1, //[15]
		 nss                             :  3, //[18:16]
		 stream_offset                   :  3, //[21:19]
		 sta_dcm                         :  1, //[22]
		 sta_coding                      :  1, //[23]
		 ru_start_index                  :  7, //[30:24]
		 reserved_0c                     :  1; //[31]
};

/**
 * cfr_6018_init_pdev() - Inits cfr pdev and registers necessary handlers.
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: Registration status for necessary handlers
 */
QDF_STATUS cfr_6018_init_pdev(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev);

/**
 * cfr_6018_deinit_pdev() - De-inits corresponding pdev and handlers.
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: De-registration status for necessary handlers
 */
QDF_STATUS cfr_6018_deinit_pdev(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev);

/**
 * target_if_cfr_start_lut_age_timer() - Start timer to flush aged-out LUT
 * entries
 * @pdev: pointer to pdev object
 *
 * Return: None
 */
void target_if_cfr_start_lut_age_timer(struct wlan_objmgr_pdev *pdev);

/**
 * target_if_cfr_stop_lut_age_timer() - Stop timer to flush aged-out LUT
 * entries
 * @pdev: pointer to pdev object
 *
 * Return: None
 */
void target_if_cfr_stop_lut_age_timer(struct wlan_objmgr_pdev *pdev);

/**
 * target_if_cfr_dump_lut_enh() - Dump all valid LUT entries
 * @pdev: objmgr PDEV
 *
 * Return: none
 */
void target_if_cfr_dump_lut_enh(struct wlan_objmgr_pdev *pdev);

/**
 * target_if_cfr_config_rcc() - Start repetitive channel capture
 * @pdev: pointer to pdev object
 * @rcc_param: rcc configurations
 *
 * Return: Success/Failure status
 */
QDF_STATUS target_if_cfr_config_rcc(struct wlan_objmgr_pdev *pdev,
				    struct cfr_rcc_param *rcc_param);

/**
 *  target_if_cfr_default_ta_ra_config() - Configure default values to all
 *  params(BW/NSS/TA/RA) in TA_RA mode
 * @rcc_param: rcc configurations
 * @allvalid: Indicates whether all TA_RA params are valid or not.
 *            It could be either 0 or 1.
 *            1: should be sent to FW during CFR initialization
 *            0: should be set, after a successful commit session.
 * @reset_cfg: This bitmap is being used to determine which groups'
 *            parameters are needed to be reset to its default state.
 */
void target_if_cfr_default_ta_ra_config(struct cfr_rcc_param *rcc_param,
					bool allvalid, uint16_t reset_cfg);

/**
 * target_if_cfr_rx_tlv_process() - Process PPDU status TLVs and store info in
 * lookup table
 * @pdev: PDEV object
 * @nbuf: ppdu info
 *
 * Return: none
 */
void target_if_cfr_rx_tlv_process(struct wlan_objmgr_pdev *pdev, void *nbuf);

/**
 * target_if_cfr_update_global_cfg() - Update global config after a successful
 * commit
 * @pdev: pointer to pdev object
 *
 * Return: None
 */
void target_if_cfr_update_global_cfg(struct wlan_objmgr_pdev *pdev);
#else
static QDF_STATUS cfr_6018_init_pdev(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS cfr_6018_deinit_pdev(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif
