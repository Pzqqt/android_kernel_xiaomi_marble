/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __CFG_MLME_EHT_CAPS_H
#define __CFG_MLME_EHT_CAPS_H

/*
 * <ini>
 * eht_su_beamformer - Enable SU beamformer
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable or disable SU beamformer
 *
 * Related: None
 *
 * Supported Feature: 11be
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_EHT_SU_BEAMFORMER CFG_BOOL( \
				"eht_su_beamformer", \
				0, \
				"EHT Su Beamformer")

/*
 * <ini>
 * eht_su_beamformee - Enable SU beamformee
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable or disable SU beamformee
 *
 * Related: None
 *
 * Supported Feature: 11be
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_EHT_SU_BEAMFORMEE CFG_BOOL( \
				"eht_su_beamformee", \
				0, \
				"EHT Su Beamformee")

/*
 * <ini>
 * mu_bformer_le_80mhz - Enable MU beamformer for BW <= 80
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable or disable MU beamformer for BW <= 80
 *
 * Related: None
 *
 * Supported Feature: 11be
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_EHT_MU_BFORMER_LE_80MHZ CFG_BOOL( \
				"mu_bformer_le_80mhz", \
				0, \
				"EHT MU Beamformer BW <= 80 MHz")

/*
 * <ini>
 * mu_bformer_160mhz - Enable MU beamformer for BW == 160
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable or disable MU beamformer for BW == 160
 *
 * Related: None
 *
 * Supported Feature: 11be
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_EHT_MU_BFORMER_160MHZ CFG_BOOL( \
				"mu_bformer_160mhz", \
				0, \
				"EHT MU Beamformer BW = 160 MHz")

/*
 * <ini>
 * mu_bformer_320mhz - Enable MU beamformer for BW == 320
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable or disable MU beamformer for BW == 320
 *
 * Related: None
 *
 * Supported Feature: 11be
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_EHT_MU_BFORMER_320MHZ CFG_BOOL( \
				"mu_bformer_320mhz", \
				0, \
				"EHT MU Beamformer BW = 320 MHz")

/*
 * <ini>
 * eht_bfee_ss_le_80mhz - For a PPDU bandwidth less than or equal to 80 MHz,
 *                        indicates the maximum number of spatial streams that
 *                        the STA can receive in an EHT sounding NDP.
 * @Min: 0
 * @Max: 7
 * @Default: 0
 *
 * If the SU Beamformee subfield is 1, set to the maximum number of spatial
 * streams that the STA is capable of receiving in an EHT sounding NDP minus 1.
 * The minimum value of this field is 3.
 * Reserved if the SU Beamformee field is 0.
 *
 * Related: NA
 *
 * Supported Feature: 11be
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_EHT_BFEE_SS_LE_80MHZ CFG_UINT( \
				"eht_bfee_ss_le_80mhz", \
				3, \
				7, \
				3, \
				CFG_VALUE_OR_DEFAULT, \
				"EHT Beamformee SS <= 80 MHz")

/*
 * <ini>
 * eht_bfee_ss_160mhz - For a PPDU bandwidth of 160 MHz, indicates the
 *                      maximum number of spatial streams that the STA
 *                      can receive in an EHT sounding NDP.
 * @Min: 0
 * @Max: 7
 * @Default: 0
 *
 * If the SU Beamformee subfield is 1, set to the maximum number of spatial
 * streams that the STA is capable of receiving in an EHT sounding NDP minus 1.
 * The minimum value of this field is 3.
 * Reserved if the SU Beamformee field is 0.
 *
 * Related: NA
 *
 * Supported Feature: 11be
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_EHT_BFEE_SS_160MHZ CFG_UINT( \
				"eht_bfee_ss_160mhz", \
				3, \
				7, \
				3, \
				CFG_VALUE_OR_DEFAULT, \
				"EHT Beamformee SS = 160 MHz")

/*
 * <ini>
 * eht_bfee_ss_320mhz - For a PPDU bandwidth of 320 MHz, indicates the
 *                      maximum number of spatial streams that the STA
 *                      can receive in an EHT sounding NDP.
 * @Min: 0
 * @Max: 7
 * @Default: 0
 *
 * If the SU Beamformee subfield is 1, set to the maximum number of spatial
 * streams that the STA is capable of receiving in an EHT sounding NDP minus 1.
 * The minimum value of this field is 3.
 * Reserved if the SU Beamformee field is 0.
 *
 * Related: NA
 *
 * Supported Feature: 11be
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_EHT_BFEE_SS_320MHZ CFG_UINT( \
				"eht_bfee_ss_320mhz", \
				3, \
				7, \
				3, \
				CFG_VALUE_OR_DEFAULT, \
				"EHT Beamformee SS = 320 MHz")

/*
 * <ini>
 * eht_num_sounding_dim_le_80mhz - For bandwidth less than or equal to 80 MHz,
 *                                 indicates the beamformer's capability
 *                                 indicating the maximum value of the TXVECTOR
 *                                 parameter NUM_STS for an EHT sounding NDP
 * @Min: 0
 * @Max: 7
 * @Default: 0
 *
 * If the SU Beamformer subfield is 1, set to the supported maximum
 * TXVECTOR parameter NUM_STS value minus 1.
 * Reserved if the SU Beamformer subfield is 0.
 *
 * Related: NA
 *
 * Supported Feature: 11be
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_EHT_NUM_SOUNDING_DIM_LE_80MHZ CFG_UINT( \
				"eht_num_sounding_dim_le_80mhz", \
				0, \
				7, \
				0, \
				CFG_VALUE_OR_DEFAULT, \
				"EHT Number Of Sounding Dimensions <= 80 MHz")

/*
 * <ini>
 * eht_num_sounding_dim_160mhz - For bandwidth of 160 MHz, indicates the
 *                               beamformer's capability indicating the
 *                               maximum value of the TXVECTOR parameter
 *                               NUM_STS for an EHT sounding NDP
 * @Min: 0
 * @Max: 7
 * @Default: 0
 *
 * If the SU Beamformer subfield is 1, set to the supported maximum
 * TXVECTOR parameter NUM_STS value minus 1.
 * Reserved if the SU Beamformer subfield is 0 or the Supported Channel
 * Width Set field does not indicate support for bandwidth of 160 MHz.
 *
 * Related: NA
 *
 * Supported Feature: 11be
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_EHT_NUM_SOUNDING_DIM_160MHZ CFG_UINT( \
				"eht_num_sounding_dim_160mhz", \
				0, \
				7, \
				0, \
				CFG_VALUE_OR_DEFAULT, \
				"EHT Number Of Sounding Dimensions = 160 MHz")

/*
 * <ini>
 * eht_num_sounding_dim_320mhz - For bandwidth of 320 MHz, indicates the
 *                               beamformer's capability indicating the
 *                               maximum value of the TXVECTOR parameter
 *                               NUM_STS for an EHT sounding NDP
 * @Min: 0
 * @Max: 7
 * @Default: 0
 *
 * If the SU Beamformer subfield is 1, set to the supported maximum
 * TXVECTOR parameter NUM_STS value minus 1.
 * Reserved if the SU Beamformer subfield is 0 or the Supported Channel
 * Width Set field does not indicate support for bandwidth of 320 MHz.
 *
 * Related: NA
 *
 * Supported Feature: 11be
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_EHT_NUM_SOUNDING_DIM_320MHZ CFG_UINT( \
				"eht_num_sounding_dim_320mhz", \
				0, \
				7, \
				0, \
				CFG_VALUE_OR_DEFAULT, \
				"EHT Number Of Sounding Dimensions = 320 MHz")

#define CFG_EHT_CAPS_ALL \
	CFG(CFG_EHT_SU_BEAMFORMER) \
	CFG(CFG_EHT_SU_BEAMFORMEE) \
	CFG(CFG_EHT_MU_BFORMER_LE_80MHZ) \
	CFG(CFG_EHT_MU_BFORMER_160MHZ) \
	CFG(CFG_EHT_MU_BFORMER_320MHZ) \
	CFG(CFG_EHT_BFEE_SS_LE_80MHZ) \
	CFG(CFG_EHT_BFEE_SS_160MHZ) \
	CFG(CFG_EHT_BFEE_SS_320MHZ) \
	CFG(CFG_EHT_NUM_SOUNDING_DIM_LE_80MHZ) \
	CFG(CFG_EHT_NUM_SOUNDING_DIM_160MHZ) \
	CFG(CFG_EHT_NUM_SOUNDING_DIM_320MHZ)

#endif /* __CFG_MLME_EHT_CAPS_H */

