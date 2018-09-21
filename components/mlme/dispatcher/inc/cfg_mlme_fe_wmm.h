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

#ifndef __CFG_MLME_FE_WMM_H
#define __CFG_MLME_FE_WMM_H

#define CFG_QOS_ENABLED CFG_BOOL( \
		"qos_enabled", \
		0, \
		"QOS Enabled")

#define CFG_WME_ENABLED CFG_BOOL( \
		"wme_enabled", \
		1, \
		"WME Enabled")

#define CFG_MAX_SP_LENGTH CFG_UINT( \
		"max_sp_length", \
		0, \
		3, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"MAX sp length")

#define CFG_WSM_ENABLED CFG_BOOL( \
		"wsm_enabled", \
		0, \
		"WSM Enabled")

#define CFG_EDCA_PROFILE CFG_UINT( \
		"edca_profile", \
		0, \
		4, \
		1, \
		CFG_VALUE_OR_DEFAULT, \
		"Edca Profile")

/* default TSPEC parameters for AC_VO */
/*
 * <ini>
 * InfraDirAcVo - Set TSPEC direction for VO
 * @Min: 0
 * @Max: 3
 * @Default: 3
 *
 * This ini is used to set TSPEC direction for VO
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_DIR_AC_VO CFG_INI_UINT( \
		"InfraDirAcVo", \
		0, \
		3, \
		3, \
		CFG_VALUE_OR_DEFAULT, \
		"direction for vo")

/*
 * <ini>
 * InfraNomMsduSizeAcVo - Set normal MSDU size for VO
 * @Min: 0x0
 * @Max: 0xFFFF
 * @Default: 0x80D0
 *
 * This ini is used to set normal MSDU size for VO
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_NOM_MSDU_SIZE_AC_VO CFG_INI_UINT( \
		"InfraNomMsduSizeAcVo", \
		0x0, \
		0xFFFF, \
		0x80D0, \
		CFG_VALUE_OR_DEFAULT, \
		"MSDU size for VO")

/*
 * <ini>
 * InfraMeanDataRateAcVo - Set mean data rate for VO
 * @Min: 0x0
 * @Max: 0xFFFFFFFF
 * @Default: 0x14500
 *
 * This ini is used to set mean data rate for VO
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_MEAN_DATA_RATE_AC_VO CFG_INI_UINT( \
		"InfraMeanDataRateAcVo", \
		0x0, \
		0xFFFFFFFF, \
		0x14500, \
		CFG_VALUE_OR_DEFAULT, \
		"mean data rate for VO")

/*
 * <ini>
 * InfraMinPhyRateAcVo - Set min PHY rate for VO
 * @Min: 0x0
 * @Max: 0xFFFFFFFF
 * @Default: 0x5B8D80
 *
 * This ini is used to set min PHY rate for VO
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_MIN_PHY_RATE_AC_VO CFG_INI_UINT( \
		"InfraMinPhyRateAcVo", \
		0x0, \
		0xFFFFFFFF, \
		0x5B8D80, \
		CFG_VALUE_OR_DEFAULT, \
		"min PHY rate for VO")

/*
 * <ini>
 * InfraSbaAcVo - Set surplus bandwidth allowance for VO
 * @Min: 0x2001
 * @Max: 0xFFFF
 * @Default: 0x2001
 *
 * This ini is used to set surplus bandwidth allowance for VO
 *
 * Related: None.
 *
 * Supported Feature: WMM
*
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_SBA_AC_VO CFG_INI_UINT( \
		"InfraSbaAcVo", \
		0x2001, \
		0xFFFF, \
		0x2001, \
		CFG_VALUE_OR_DEFAULT, \
		"surplus bandwidth allowance for VO")

/*
 * <ini>
 * InfraUapsdVoSrvIntv - Set Uapsd service interval for voice
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 20
 *
 * This ini is used to set Uapsd service interval for voice.
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_UAPSD_VO_SRV_INTV CFG_INI_UINT( \
		"InfraUapsdVoSrvIntv", \
		0, \
		4294967295UL, \
		20, \
		CFG_VALUE_OR_DEFAULT, \
		"Infra uapsd vo srv intv")

/*
 * <ini>
 * InfraUapsdVoSuspIntv - Set Uapsd suspension interval for voice
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 2000
 *
 * This ini is used to set Uapsd suspension interval for voice.
 *
 * Related: None.
 *
 * Supported Feature: WMM
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_QOS_WMM_UAPSD_VO_SUS_INTV CFG_INI_UINT( \
		"InfraUapsdVoSuspIntv", \
		0, \
		4294967295UL, \
		2000, \
		CFG_VALUE_OR_DEFAULT, \
		"Infra uapsd vo sus intv")

#define CFG_WMM_PARAMS_ALL \
	CFG(CFG_QOS_ENABLED) \
	CFG(CFG_WME_ENABLED) \
	CFG(CFG_MAX_SP_LENGTH) \
	CFG(CFG_WSM_ENABLED) \
	CFG(CFG_EDCA_PROFILE) \
	CFG(CFG_QOS_WMM_DIR_AC_VO) \
	CFG(CFG_QOS_WMM_NOM_MSDU_SIZE_AC_VO) \
	CFG(CFG_QOS_WMM_MEAN_DATA_RATE_AC_VO) \
	CFG(CFG_QOS_WMM_MIN_PHY_RATE_AC_VO) \
	CFG(CFG_QOS_WMM_SBA_AC_VO) \
	CFG(CFG_QOS_WMM_UAPSD_VO_SRV_INTV) \
	CFG(CFG_QOS_WMM_UAPSD_VO_SUS_INTV)

#endif /* __CFG_MLME_FE_WMM_H */
