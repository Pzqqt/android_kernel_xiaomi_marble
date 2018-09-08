/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: This file contains centralized definitions of converged configuration.
 */

#ifndef __CFG_MLME_VHT_CAPS_H
#define __CFG_MLME_VHT_CAPS_H

#define CFG_VHT_SUPP_CHAN_WIDTH CFG_UINT( \
		"supp_chan_width", \
		0, \
		2, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT SUPPORTED CHAN WIDTH SET")

#define CFG_VHT_BEAMFORMEE_ANT_SUPP CFG_UINT( \
		"tx_bfee_ant_supp", \
		0, \
		8, \
		8, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT BEAMFORMEE ANTENNA SUPPORTED CAP")

#define CFG_VHT_NUM_SOUNDING_DIMENSIONS CFG_UINT( \
		"num_soundingdim", \
		0, \
		3, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT NUMBER OF SOUNDING DIMENSIONS")

#define CFG_VHT_HTC_VHTC CFG_BOOL( \
		"htc_vhtc", \
		0, \
		"VHT HTC VHTC")

#define CFG_VHT_LINK_ADAPTATION_CAP CFG_UINT( \
		"link_adap_cap", \
		0, \
		3, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT LINK ADAPTATION CAP")

#define CFG_VHT_RX_ANT_PATTERN CFG_BOOL( \
		"rx_antpattern", \
		1, \
		"VHT RX ANTENNA PATTERN CAP")

#define CFG_VHT_TX_ANT_PATTERN CFG_BOOL( \
		"tx_antpattern", \
		1, \
		"VHT TX ANTENNA PATTERN CAP")

#define CFG_VHT_RX_SUPP_DATA_RATE CFG_UINT( \
		"rx_supp_data_rate", \
		0, \
		780, \
		780, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT RX SUPP DATA RATE")

#define CFG_VHT_TX_SUPP_DATA_RATE CFG_UINT( \
		"tx_supp_data_rate", \
		0, \
		780, \
		780, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT TX SUPP DATA RATE")

#define CFG_VHT_ENABLE_TXBF_20MHZ CFG_BOOL( \
		"enable_txbf_20mhz", \
		0, \
		"VHT ENABLE TXBF 20MHZ")

#define CFG_VHT_LDPC_CODING_CAP CFG_INI_BOOL( \
		"ldpc_coding_cap", \
		0, \
		"VHT LDPC CODING CAP")

#define CFG_VHT_SHORT_GI_80MHZ CFG_INI_BOOL( \
		"short_gi_80mhz", \
		1, \
		"VHT SHORT GI 80MHZ")

#define CFG_VHT_SHORT_GI_160_AND_80_PLUS_80MHZ CFG_INI_BOOL( \
		"short_gi_160mhz", \
		0, \
		"VHT SHORT GI 160 AND 80 PLUS 80MHZ")

#define CFG_VHT_TXSTBC CFG_INI_BOOL( \
		"tx_stbc", \
		0, \
		"VHT Tx STBC")

#define CFG_VHT_RXSTBC CFG_INI_BOOL( \
		"rx_stbc", \
		1, \
		"VHT Rx STBC")

#define CFG_VHT_SU_BEAMFORMER_CAP CFG_INI_BOOL( \
		"su_bformer", \
		0, \
		"VHT SU BEAMFORMER CAP")

#define CFG_VHT_SU_BEAMFORMEE_CAP CFG_INI_BOOL( \
		"su_bformee", \
		1, \
		"VHT SU BEAMFORMEE CAP")

#define CFG_VHT_MU_BEAMFORMER_CAP CFG_INI_BOOL( \
		"mu_bformer", \
		0, \
		"VHT MU BEAMFORMER CAP")

#define CFG_VHT_TXOP_PS CFG_INI_BOOL( \
		"txop_ps", \
		0, \
		"VHT TXOP PS")

#define CFG_VHT_RX_MCS_MAP CFG_INI_UINT( \
		"rx_mcs_map", \
		0, \
		65535, \
		65534, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT RX MCS MAP")

#define CFG_VHT_TX_MCS_MAP CFG_INI_UINT( \
		"tx_mcs_map", \
		0, \
		65535, \
		65534, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT TX MCS MAP")

#define CFG_VHT_BASIC_MCS_SET CFG_INI_UINT( \
		"basic_mcs_set", \
		0, \
		65535, \
		65534, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT BASIC MCS SET")

/*
 * <ini>
 * gVhtChannelWidth - Channel width capability for 11ac
 * @Min: 0
 * @Max: 4
 * @Default: 3
 *
 * This ini is  used to set channel width capability for 11AC.
 * eHT_CHANNEL_WIDTH_20MHZ = 0,
 * eHT_CHANNEL_WIDTH_40MHZ = 1,
 * eHT_CHANNEL_WIDTH_80MHZ = 2,
 * eHT_CHANNEL_WIDTH_160MHZ = 3,
 * eHT_CHANNEL_WIDTH_80P80MHZ = 4,
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_VHT_CHANNEL_WIDTH CFG_INI_UINT( \
		"gVhtChannelWidth", \
		0, \
		4, \
		2, \
		CFG_VALUE_OR_DEFAULT, \
		"Channel width capability for 11ac")

/*
 * <ini>
 * gVhtRxMCS - VHT Rx MCS capability for 1x1 mode
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is  used to set VHT Rx MCS capability for 1x1 mode.
 * 0, MCS0-7
 * 1, MCS0-8
 * 2, MCS0-9
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_VHT_ENABLE_RX_MCS_8_9 CFG_INI_UINT( \
		"gVhtRxMCS", \
		0, \
		2, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT Rx MCS")

/*
 * <ini>
 * gVhtTxMCS - VHT Tx MCS capability for 1x1 mode
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is  used to set VHT Tx MCS capability for 1x1 mode.
 * 0, MCS0-7
 * 1, MCS0-8
 * 2, MCS0-9
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_VHT_ENABLE_TX_MCS_8_9 CFG_INI_UINT( \
		"gVhtTxMCS", \
		0, \
		2, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT Tx MCS")

/*
 * <ini>
 * gVhtRxMCS2x2 - VHT Rx MCS capability for 2x2 mode
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is  used to set VHT Rx MCS capability for 2x2 mode.
 * 0, MCS0-7
 * 1, MCS0-8
 * 2, MCS0-9
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_VHT_ENABLE_RX_MCS2x2_8_9 CFG_INI_UINT( \
		"gVhtRxMCS2x2", \
		0, \
		2, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT Rx MCS 2x2")

/*
 * <ini>
 * gVhtTxMCS2x2 - VHT Tx MCS capability for 2x2 mode
 * @Min: 0
 * @Max: 2
 * @Default: 0
 *
 * This ini is  used to set VHT Tx MCS capability for 2x2 mode.
 * 0, MCS0-7
 * 1, MCS0-8
 * 2, MCS0-9
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_VHT_ENABLE_TX_MCS2x2_8_9 CFG_INI_UINT( \
		"gVhtTxMCS2x2", \
		0, \
		2, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT Tx MCS 2x2")

/*
 * <ini>
 * enable_vht20_mcs9 - Enables VHT MCS9 in 20M BW operation
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_ENABLE_VHT20_MCS9 CFG_INI_BOOL( \
		"enable_vht20_mcs9", \
		1, \
		"Enables VHT MCS9 in 20M BW")

/*
 * <ini>
 * gEnable2x2 - Enables/disables VHT Tx/Rx MCS values for 2x2
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini disables/enables 2x2 mode. If this is zero then DUT operates as 1x1
 *
 * 0, Disable
 * 1, Enable
 *
 * Related: NA
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_VHT_ENABLE_2x2_CAP_FEATURE CFG_INI_BOOL( \
		"gEnable2x2", \
		0, \
		"VHT Enable 2x2")

/*
 * <ini>
 * gEnableMuBformee - Enables/disables multi-user (MU) beam formee capability
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini enables/disables multi-user (MU) beam formee
 * capability
 *
 * Change MU Bformee only when  gTxBFEnable is enabled.
 * When gTxBFEnable and gEnableMuBformee are set, MU beam formee capability is
 * enabled.
 * Related:  gTxBFEnable
 *
 * Supported Feature: 11AC
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_VHT_ENABLE_MU_BFORMEE_CAP_FEATURE CFG_INI_BOOL( \
		"gEnableMuBformee", \
		0, \
		"VHT Enable MU Beamformee")

#define CFG_VHT_ENABLE_PAID_FEATURE CFG_INI_BOOL( \
		"gEnablePAID", \
		0, \
		"VHT Enable PAID")

#define CFG_VHT_ENABLE_GID_FEATURE CFG_INI_BOOL( \
		"gEnableGID", \
		0, \
		"VHT Enable GID")

/* Parameter to control VHT support in 2.4 GHz band */
#define CFG_ENABLE_VHT_FOR_24GHZ CFG_INI_BOOL( \
		"gEnableVhtFor24GHzBand", \
		0, \
		"VHT Enable for 24GHz")

/*
 * gEnableVendorVhtFor24GHzBand:Parameter to control VHT support
 * based on vendor ie in 2.4 GHz band
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This parameter will enable SAP to read VHT capability in vendor ie in Assoc
 * Req and send VHT caps in Resp to establish connection in VHT Mode.
 * Supported Feature: SAP
 *
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_VENDOR_VHT_FOR_24GHZ CFG_INI_BOOL( \
		"gEnableVendorVhtFor24GHzBand", \
		1, \
		"VHT Enable Vendor for 24GHz")

/* Macro to handle maximum receive AMPDU size configuration */
#define CFG_VHT_AMPDU_LEN_EXPONENT CFG_INI_UINT( \
		"gVhtAmpduLenExponent", \
		0, \
		7, \
		3, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT AMPDU Len in Exponent")

#define CFG_VHT_MPDU_LEN CFG_INI_UINT( \
		"gVhtMpduLen", \
		0, \
		2, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"VHT MPDU Length")

#define CFG_VHT_CAPS_ALL \
	CFG(CFG_VHT_SUPP_CHAN_WIDTH) \
	CFG(CFG_VHT_LDPC_CODING_CAP) \
	CFG(CFG_VHT_SHORT_GI_80MHZ) \
	CFG(CFG_VHT_SHORT_GI_160_AND_80_PLUS_80MHZ) \
	CFG(CFG_VHT_TXSTBC) \
	CFG(CFG_VHT_RXSTBC) \
	CFG(CFG_VHT_SU_BEAMFORMER_CAP) \
	CFG(CFG_VHT_SU_BEAMFORMEE_CAP) \
	CFG(CFG_VHT_BEAMFORMEE_ANT_SUPP) \
	CFG(CFG_VHT_NUM_SOUNDING_DIMENSIONS) \
	CFG(CFG_VHT_MU_BEAMFORMER_CAP) \
	CFG(CFG_VHT_TXOP_PS) \
	CFG(CFG_VHT_HTC_VHTC) \
	CFG(CFG_VHT_LINK_ADAPTATION_CAP) \
	CFG(CFG_VHT_RX_ANT_PATTERN) \
	CFG(CFG_VHT_TX_ANT_PATTERN) \
	CFG(CFG_VHT_RX_MCS_MAP) \
	CFG(CFG_VHT_TX_MCS_MAP) \
	CFG(CFG_VHT_RX_SUPP_DATA_RATE) \
	CFG(CFG_VHT_TX_SUPP_DATA_RATE) \
	CFG(CFG_VHT_BASIC_MCS_SET) \
	CFG(CFG_VHT_ENABLE_TXBF_20MHZ) \
	CFG(CFG_VHT_CHANNEL_WIDTH) \
	CFG(CFG_VHT_ENABLE_RX_MCS_8_9) \
	CFG(CFG_VHT_ENABLE_TX_MCS_8_9) \
	CFG(CFG_VHT_ENABLE_RX_MCS2x2_8_9) \
	CFG(CFG_VHT_ENABLE_TX_MCS2x2_8_9) \
	CFG(CFG_ENABLE_VHT20_MCS9) \
	CFG(CFG_VHT_ENABLE_2x2_CAP_FEATURE) \
	CFG(CFG_VHT_ENABLE_MU_BFORMEE_CAP_FEATURE) \
	CFG(CFG_VHT_ENABLE_PAID_FEATURE) \
	CFG(CFG_VHT_ENABLE_GID_FEATURE) \
	CFG(CFG_ENABLE_VHT_FOR_24GHZ) \
	CFG(CFG_ENABLE_VENDOR_VHT_FOR_24GHZ) \
	CFG(CFG_VHT_AMPDU_LEN_EXPONENT) \
	CFG(CFG_VHT_MPDU_LEN)

#endif /* __CFG_MLME_VHT_CAPS_H */
