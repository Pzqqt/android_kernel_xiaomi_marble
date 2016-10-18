/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/*
 *
 * This file contains the source code for CFG API functions.
 *
 * Author:      Kevin Nguyen
 * Date:        04/09/02
 * History:-
 * 04/09/02        Created.
 * --------------------------------------------------------------------
 */

#include "cds_api.h"
#include "cfg_priv.h"
#include "cfg_debug.h"
#include "wma_types.h"
#include "cfg_api.h"

/* --------------------------------------------------------------------- */
/* Static Variables */
/* ---------------------------------------------------------------------- */
static tCfgCtl __g_cfg_entry[CFG_PARAM_MAX_NUM];
static uint8_t __g_s_buffer[CFG_MAX_STR_LEN];
static uint32_t __g_param_list[WNI_CFG_MAX_PARAM_NUM +
			     WNI_CFG_GET_PER_STA_STAT_RSP_NUM];

static void notify(tpAniSirGlobal, uint16_t, uint32_t);

typedef enum {
	eRF_BAND_UNKNOWN = 0,
	eRF_BAND_2_4_GHZ = 1,
	eRF_BAND_5_GHZ = 2
} eRfBandMode;

extern cfgstatic_string cfg_static_string[CFG_MAX_STATIC_STRING];
extern cgstatic cfg_static[CFG_PARAM_MAX_NUM] ;

/* --------------------------------------------------------------------- */
uint32_t cfg_need_restart(tpAniSirGlobal pMac, uint16_t cfgId)
{
	if (!pMac->cfg.gCfgEntry) {
		PELOGE(cfg_log(pMac, LOGE, FL("gCfgEntry is NULL"));)
		return 0;
	}
	return !!(pMac->cfg.gCfgEntry[cfgId].control & CFG_CTL_RESTART);
}

static void cfg_get_strindex(tpAniSirGlobal pMac, uint16_t cfgId)
{
	uint16_t i = 0;

	for (i = 0; i < CFG_MAX_STATIC_STRING; i++) {
		if (cfgId == cfg_static_string[i].cfgId)
			break;
	}
	if (i == CFG_MAX_STATIC_STRING) {
		PELOGE(cfg_log(pMac, LOGE, FL("Entry not found for cfg id :%d"), cfgId);)
		cfg_static[cfgId].pStrData = NULL;
		return;
	}
	cfg_static[cfgId].pStrData = &cfg_static_string[i];
}
/* --------------------------------------------------------------------- */
uint32_t cfg_need_reload(tpAniSirGlobal pMac, uint16_t cfgId)
{
	if (!pMac->cfg.gCfgEntry) {
		PELOGE(cfg_log(pMac, LOGE, FL("gCfgEntry is NULL"));)
		return 0;
	}
	return !!(pMac->cfg.gCfgEntry[cfgId].control & CFG_CTL_RELOAD);
}

/* --------------------------------------------------------------------- */
tSirRetStatus cfg_init(tpAniSirGlobal pMac)
{
	uint16_t i = 0;
	uint16_t combined_buff_size = 0;
	uint32_t    max_i_count = 0;
	uint32_t    max_s_count = 0;
	cfgstatic_string *str_cfg;

	pMac->cfg.gSBuffer = __g_s_buffer;
	pMac->cfg.gCfgEntry = __g_cfg_entry;
	pMac->cfg.gParamList = __g_param_list;

	for (i = 0; i < CFG_PARAM_MAX_NUM; i++) {
		if (!(cfg_static[i].control & CFG_CTL_INT)) {
			cfg_get_strindex(pMac, i);
		} else {
			cfg_static[i].pStrData = NULL;
		}
	}

	for (i = 0; i < CFG_PARAM_MAX_NUM ; i++) {
		if (cfg_static[i].control & CFG_CTL_INT) {
			max_i_count++;
		} else {
			str_cfg = (cfgstatic_string *)cfg_static[i].pStrData;
			if (str_cfg == NULL) {
				cfg_log(pMac, LOGE,
					FL("pStrCfg is NULL for CfigID : %d"),
					i);
				continue;
			}
			/* + 2 to include len field and max len field */
			max_s_count += str_cfg->maxLen + 2;
		}
	}

	pMac->cfg.gCfgMaxIBufSize = max_i_count;
	pMac->cfg.gCfgMaxSBufSize = max_s_count;

	/* Allocate a combined memory */
	combined_buff_size = max_s_count + (3 * sizeof(uint32_t) * max_i_count);

	cfg_log(pMac, LOGE, FL("Size of cfg I buffer:%d  S buffer: %d"),
		max_i_count, max_s_count);

	cfg_log(pMac, LOGE, FL("Allocation for cfg buffers: %d bytes"),
		combined_buff_size);

	if (combined_buff_size > 4 * PAGE_SIZE) {
		cfg_log(pMac, LOGE, FL("Mem alloc request too big"));
		return eSIR_MEM_ALLOC_FAILED;
	}
	/* at this point pMac->cfg.gCfgSBuf starts */
	pMac->cfg.gCfgSBuf = qdf_mem_malloc(combined_buff_size);
	if (NULL == pMac->cfg.gCfgSBuf) {
		cfg_log(pMac, LOGE,
			FL("Failed to allocate memory for cfg array"));
		return eSIR_MEM_ALLOC_FAILED;
	}
	/* at offset max_s_count, pMac->cfg.gCfgIBuf starts */
	pMac->cfg.gCfgIBuf = (uint32_t *)&pMac->cfg.gCfgSBuf[max_s_count];
	/* after max_i_count integers, pMac->cfg.gCfgIBufMin starts */
	pMac->cfg.gCfgIBufMin = &pMac->cfg.gCfgIBuf[max_i_count];
	/* after max_i_count integers, pMac->cfg.gCfgIBufMax starts */
	pMac->cfg.gCfgIBufMax = &pMac->cfg.gCfgIBufMin[max_i_count];

	return eSIR_SUCCESS;
}

/* ---------------------------------------------------------------------- */
void cfg_de_init(tpAniSirGlobal pMac)
{
	qdf_mem_free(pMac->cfg.gCfgSBuf);
	pMac->cfg.gCfgIBufMin = NULL;
	pMac->cfg.gCfgIBufMax = NULL;
	pMac->cfg.gCfgIBuf = NULL;
	pMac->cfg.gCfgSBuf = NULL;
	pMac->cfg.gSBuffer = NULL;
	pMac->cfg.gCfgEntry = NULL;
	pMac->cfg.gParamList = NULL;
}

/* --------------------------------------------------------------------- */
/**
 * cfg_check_valid()
 *
 * FUNCTION:
 * This function is called to check if a parameter is valid
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param cfgId:  16-bit CFG parameter ID
 *
 * @return eSIR_SUCCESS:  request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 */
tSirRetStatus cfg_check_valid(tpAniSirGlobal pMac, uint16_t cfgId,
					uint32_t *index)
{
	uint32_t control;

	if (cfgId >= CFG_PARAM_MAX_NUM) {
		PELOGE(cfg_log(pMac, LOG3, FL("Invalid cfg id %d"), cfgId);)
		return eSIR_CFG_INVALID_ID;
	}
	if (!pMac->cfg.gCfgEntry) {
		PELOGE(cfg_log(pMac, LOGE, FL("gCfgEntry is NULL"));)
		return eSIR_CFG_INVALID_ID;
	}

	control = pMac->cfg.gCfgEntry[cfgId].control;

	/* Check if parameter is valid */
	if ((control & CFG_CTL_VALID) == 0) {
		PELOGE(cfg_log(pMac, LOGE, FL("Not valid cfg id %d"), cfgId);)
		return eSIR_CFG_INVALID_ID;
	}

	*index = control & CFG_BUF_INDX_MASK;

	if (*index >= pMac->cfg.gCfgMaxSBufSize) {
		PELOGE(cfg_log(pMac, LOGE, FL("cfg index out of bounds %d"),
					*index);)
		return eSIR_CFG_INVALID_ID;
	}

	return eSIR_SUCCESS;

} /*** end cfg_check_valid() ***/

/* --------------------------------------------------------------------- */
/**
 * cfg_set_int()
 *
 * FUNCTION:
 * This function is called to update an integer parameter.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * - Range checking is performed by the calling function.  In case this
 *   function call is being triggered by a request from host, then host
 *   is responsible for performing range checking before sending the
 *   request.
 *
 * - Host RW permission checking should already be done prior to calling
 *   this function by the message processing function.
 *
 * NOTE:
 *
 * @param cfgId:     16-bit CFG parameter ID
 * @param value:     32-bit unsigned value
 *
 * @return eSIR_SUCCESS:  request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 */

tSirRetStatus cfg_set_int(tpAniSirGlobal pMac, uint16_t cfgId, uint32_t value)
{
	uint32_t index;
	uint32_t control;
	uint32_t mask;
	tSirRetStatus status;

	status = cfg_check_valid(pMac, cfgId, &index);

	if (eSIR_SUCCESS != status)
		return status;

	if ((pMac->cfg.gCfgIBufMin[index] > value) ||
			(pMac->cfg.gCfgIBufMax[index] < value)) {
		PELOGE(cfg_log (pMac, LOGE, FL(
			"Value %d out of range [%d,%d] cfg id %d"), value,
			       pMac->cfg.gCfgIBufMin[index],
			       pMac->cfg.gCfgIBufMax[index], cfgId);)
		return eSIR_CFG_INVALID_ID;
	} else {
		/* Write integer value */
		pMac->cfg.gCfgIBuf[index] = value;

		control = pMac->cfg.gCfgEntry[cfgId].control;
		/* Update hardware if necessary */
		mask = control & CFG_CTL_NTF_MASK;
		if ((mask & CFG_CTL_NTF_HW) != 0)
			PELOGE(cfg_log(pMac, LOGE, FL(
				"CFG notify HW not supported!!!"));)
			/* notify other modules if necessary */
			if ((mask & CFG_CTL_NTF_MASK) != 0)
				notify(pMac, cfgId, mask);
	}
	return status;
} /*** end cfg_set_int ***/

/* --------------------------------------------------------------------- */
/**
 * wlan_cfg_get_int()
 *
 * FUNCTION:
 * This function is called to read an integer parameter.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param cfgId:  16-bit CFG parameter ID
 * @param pVal:   address where parameter value will be written
 *
 * @return eSIR_SUCCESS:  request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 */

tSirRetStatus wlan_cfg_get_int(tpAniSirGlobal pMac, uint16_t cfgId,
				uint32_t *pValue)
{
	uint32_t index;
	tSirRetStatus status;

	status = cfg_check_valid(pMac, cfgId, &index);

	if (eSIR_SUCCESS != status)
		return status;

	/* Get integer value */
	*pValue = pMac->cfg.gCfgIBuf[index];

	return eSIR_SUCCESS;
} /*** end wlan_cfg_get_int() ***/

/* --------------------------------------------------------------------- */
/**
 * cfg_set_str()
 *
 * FUNCTION:
 * This function is called to set a string parameter.
 *
 * LOGIC:
 * This function invokes the cfg_set_str_notify function passing the notify
 * bool value set to true. This basically means that HAL needs to be
 * notified. This is true in the case of non-integrated SOC's or Libra/Volans.
 * In the case of Prima the cfg_set_str_notify is invoked with the bool value
 * set to false.
 *
 * ASSUMPTIONS:
 * - always notify has to be called
 *
 * NOTE:
 *
 * @param cfgId:     16-bit CFG parameter ID
 * @param pStr:      address of string data
 * @param len:       string length
 *
 * @return eSIR_SUCCESS:  request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 * @return eSIR_CFG_INVALID_LEN:  invalid CFG parameter length
 *
 */

tSirRetStatus cfg_set_str(tpAniSirGlobal pMac, uint16_t cfgId, uint8_t *pStr,
			  uint32_t length)
{
	return cfg_set_str_notify(pMac, cfgId, pStr, length, true);
}

/* --------------------------------------------------------------------- */
/**
 * cfg_set_str_notify()
 *
 * FUNCTION:
 * This function is called to set a string parameter.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * - No length checking will be performed.  Should be done by calling
 *   module.
 * - Host RW permission should be checked prior to calling this
 *   function.
 *
 * NOTE:
 *
 * @param cfgId:     16-bit CFG parameter ID
 * @param pStr:      address of string data
 * @param len:       string length
 * @param notifyMod. notify respective Module
 *
 * @return eSIR_SUCCESS:  request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 * @return eSIR_CFG_INVALID_LEN:  invalid CFG parameter length
 *
 */

tSirRetStatus cfg_set_str_notify(tpAniSirGlobal pMac, uint16_t cfgId,
					uint8_t *pStr, uint32_t length,
					int notifyMod)
{
	uint8_t *pDst, *pDstEnd;
	uint32_t index, paramLen, mask;
	uint32_t control;
	tSirRetStatus status;

	status = cfg_check_valid(pMac, cfgId, &index);

	if (eSIR_SUCCESS != status)
		return status;

	pDst = &pMac->cfg.gCfgSBuf[index];
	paramLen = *pDst++;
	control = pMac->cfg.gCfgEntry[cfgId].control;
	if (length > paramLen) {
		PELOGE(cfg_log(pMac, LOGE, FL(
			"Invalid length %d (>%d) cfg id %d"),
			length, paramLen, cfgId);)
			return eSIR_CFG_INVALID_LEN;
	} else {
		*pDst++ = (uint8_t) length;
		pDstEnd = pDst + length;
		while (pDst < pDstEnd) {
			*pDst++ = *pStr++;
		}
		if (notifyMod) {
			/* Update hardware if necessary */
			mask = control & CFG_CTL_NTF_MASK;
			if ((mask & CFG_CTL_NTF_HW) != 0) {
				PELOGE(cfg_log(pMac, LOGE, FL(
					"CFG notify HW not supported!"));)
			}
			/* notify other modules if necessary */
			if ((mask & CFG_CTL_NTF_MASK) != 0) {
				notify(pMac, cfgId, mask);
			}
		}
	}
	return eSIR_SUCCESS;
} /*** end cfg_set_str_notify() ***/

/* --------------------------------------------------------------------- */
/**
 * wlan_cfg_get_str()
 *
 * FUNCTION:
 * This function is called to get a string parameter.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * - Host RW permission should be checked prior to calling this
 *   function.
 *
 * NOTE:
 *
 * @param cfgId:     16-bit CFG parameter ID
 * @param pBuf:      address of string buffer
 * @param pLen:      address of max buffer length
 *                   actual length will be returned at this address
 *
 * @return eSIR_SUCCESS:  request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 * @return eSIR_CFG_INVALID_LEN:  invalid CFG parameter length
 *
 */

tSirRetStatus wlan_cfg_get_str(tpAniSirGlobal pMac, uint16_t cfgId,
				uint8_t *pBuf, uint32_t *pLength)
{
	uint8_t *pSrc, *pSrcEnd;
	uint32_t index;
	tSirRetStatus status;

	status = cfg_check_valid(pMac, cfgId, &index);

	if (eSIR_SUCCESS != status)
		return status;

	/* Get string */
	pSrc = &pMac->cfg.gCfgSBuf[index];
	pSrc++;         /* skip over max length */
	if (*pLength < *pSrc) {
		PELOGE(cfg_log(pMac, LOGE, FL(
			"Invalid length %d (<%d) cfg id %d"),
			*pLength, *pSrc, cfgId);)
			return eSIR_CFG_INVALID_LEN;
	} else {
		*pLength = *pSrc++;     /* save parameter length */
		pSrcEnd = pSrc + *pLength;
		while (pSrc < pSrcEnd)
			*pBuf++ = *pSrc++;
	}
	return eSIR_SUCCESS;
} /*** end wlan_cfg_get_str() ***/

/* --------------------------------------------------------------------- */
/**
 * wlan_cfg_get_str_max_len()
 *
 * FUNCTION:
 * This function is called to get a string maximum length.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * - Host RW permission should be checked prior to calling this
 *   function.
 *
 * NOTE:
 *
 * @param cfgId:     16-bit CFG parameter ID
 * @param pLen:      maximum length will be returned at this address
 *
 * @return eSIR_SUCCESS:  request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 *
 */

tSirRetStatus wlan_cfg_get_str_max_len(tpAniSirGlobal pMac, uint16_t cfgId,
					uint32_t *pLength)
{
	uint32_t index;
	tSirRetStatus status;

	status = cfg_check_valid(pMac, cfgId, &index);

	if (eSIR_SUCCESS != status)
		return status;

	*pLength = pMac->cfg.gCfgSBuf[index];

	return status;
} /*** end wlan_cfg_get_str_max_len() ***/

/* --------------------------------------------------------------------- */
/**
 * wlan_cfg_get_str_len()
 *
 * FUNCTION:
 * This function is called to get a string length.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * - Host RW permission should be checked prior to calling this
 *   function.
 *
 * NOTE:
 *
 * @param cfgId:     16-bit CFG parameter ID
 * @param pLen:      current length will be returned at this address
 *
 * @return eSIR_SUCCESS:         request completed successfully
 * @return eSIR_CFG_INVALID_ID:  invalid CFG parameter ID
 *
 */

tSirRetStatus wlan_cfg_get_str_len(tpAniSirGlobal pMac, uint16_t cfgId,
		uint32_t *pLength)
{
	uint32_t index;
	tSirRetStatus status;

	status = cfg_check_valid(pMac, cfgId, &index);

	if (eSIR_SUCCESS != status)
		return status;

	*pLength = pMac->cfg.gCfgSBuf[index + 1];

	return status;

} /*** end wlan_cfg_get_str_len() ***/

/**
 * cfg_get_dot11d_transmit_power() - regulatory max transmit power
 * @pMac: pointer to mac data
 * @cfgId: configuration ID
 * @cfgLength: configuration length
 * @channel: channel number
 *
 * Return:  int8_t - power
 */
static int8_t
cfg_get_dot11d_transmit_power(tpAniSirGlobal pMac, uint16_t cfgId,
			      uint32_t cfgLength, uint8_t channel)
{
	uint8_t *pCountryInfo = NULL;
	uint8_t count = 0;
	int8_t maxTxPwr = WMA_MAX_TXPOWER_INVALID;

	/* At least one element is present */
	if (cfgLength < sizeof(tSirMacChanInfo)) {
		PELOGE(cfg_log
			       (pMac, LOGE,
			       FL("Invalid CFGLENGTH %d while getting 11d txpower"),
			       cfgLength);
		       )
		goto error;
	}

	pCountryInfo = qdf_mem_malloc(cfgLength);
	if (NULL == pCountryInfo) {
		cfg_log(pMac, LOGP, FL(" failed to allocate memory"));
		goto error;
	}
	/* The CSR will always update this CFG. The contents will be from country IE if regulatory domain
	 * is enabled on AP else will contain EEPROM contents
	 */
	if (wlan_cfg_get_str(pMac, cfgId, pCountryInfo, &cfgLength) !=
	    eSIR_SUCCESS) {
		qdf_mem_free(pCountryInfo);
		pCountryInfo = NULL;

		cfg_log(pMac, LOGP,
			FL
				("Failed to retrieve 11d configuration parameters while retrieving 11d tuples"));
		goto error;
	}
	/* Identify the channel and maxtxpower */
	while (count <= (cfgLength - (sizeof(tSirMacChanInfo)))) {
		uint8_t firstChannel, maxChannels;

		firstChannel = pCountryInfo[count++];
		maxChannels = pCountryInfo[count++];
		maxTxPwr = pCountryInfo[count++];

		if ((channel >= firstChannel) &&
		    (channel < (firstChannel + maxChannels))) {
			break;
		}
	}

error:
	if (NULL != pCountryInfo)
		qdf_mem_free(pCountryInfo);

	return maxTxPwr;
}

/**----------------------------------------------------------------------
   \fn     cfg_get_regulatory_max_transmit_power

   \brief  Gets regulatory tx power on the current channel.

   \param  pMac
   \param  channel
   \param  rfBand
   -----------------------------------------------------------------------*/
int8_t cfg_get_regulatory_max_transmit_power(tpAniSirGlobal pMac,
					     uint8_t channel)
{
	uint32_t cfgLength = 0;
	uint16_t cfgId = 0;
	int8_t maxTxPwr;
	eRfBandMode rfBand = eRF_BAND_UNKNOWN;

	if ((channel >= SIR_11A_CHANNEL_BEGIN) &&
	    (channel <= SIR_11A_CHANNEL_END))
		rfBand = eRF_BAND_5_GHZ;
	else
		rfBand = eRF_BAND_2_4_GHZ;

	/* Get the max transmit power for current channel for the current regulatory domain */
	switch (rfBand) {
	case eRF_BAND_2_4_GHZ:
		cfgId = WNI_CFG_MAX_TX_POWER_2_4;
		cfgLength = WNI_CFG_MAX_TX_POWER_2_4_LEN;
		PELOG2(cfg_log
			       (pMac, LOG2,
			       FL
				       ("HAL: Reading CFG for 2.4 GHz channels to get regulatory max tx power"));
		       )
		break;

	case eRF_BAND_5_GHZ:
		cfgId = WNI_CFG_MAX_TX_POWER_5;
		cfgLength = WNI_CFG_MAX_TX_POWER_5_LEN;
		PELOG2(cfg_log
			       (pMac, LOG2,
			       FL
				       ("HAL: Reading CFG for 5.0 GHz channels to get regulatory max tx power"));
		       )
		break;

	case eRF_BAND_UNKNOWN:
	default:
		PELOG2(cfg_log
			       (pMac, LOG2,
			       FL("HAL: Invalid current working band for the device"));
		       )
		return WMA_MAX_TXPOWER_INVALID;         /* Its return, not break. */
	}

	maxTxPwr = cfg_get_dot11d_transmit_power(pMac, cfgId, cfgLength, channel);

	return maxTxPwr;
}

/* --------------------------------------------------------------------- */
/**
 * cfg_get_capability_info
 *
 * FUNCTION:
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param None
 * @return None
 */

tSirRetStatus cfg_get_capability_info(tpAniSirGlobal pMac, uint16_t *pCap,
			tpPESession sessionEntry)
{
	uint32_t val = 0;
	tpSirMacCapabilityInfo pCapInfo;

	*pCap = 0;
	pCapInfo = (tpSirMacCapabilityInfo) pCap;

	if (LIM_IS_IBSS_ROLE(sessionEntry))
		pCapInfo->ibss = 1;     /* IBSS bit */
	else if (LIM_IS_AP_ROLE(sessionEntry) ||
		LIM_IS_STA_ROLE(sessionEntry))
		pCapInfo->ess = 1;      /* ESS bit */
	else if (LIM_IS_P2P_DEVICE_ROLE(sessionEntry) ||
		LIM_IS_NDI_ROLE(sessionEntry)) {
		pCapInfo->ess = 0;
		pCapInfo->ibss = 0;
	} else
		cfg_log(pMac, LOGP,
			FL("can't get capability, role is UNKNOWN!!"));

	if (LIM_IS_AP_ROLE(sessionEntry)) {
		val = sessionEntry->privacy;
	} else {
		/* PRIVACY bit */
		if (wlan_cfg_get_int(pMac, WNI_CFG_PRIVACY_ENABLED, &val) !=
		    eSIR_SUCCESS) {
			cfg_log(pMac, LOGP,
				FL("cfg get WNI_CFG_PRIVACY_ENABLED failed"));
			return eSIR_FAILURE;
		}
	}
	if (val)
		pCapInfo->privacy = 1;

	/* Short preamble bit */
	if (wlan_cfg_get_int(pMac, WNI_CFG_SHORT_PREAMBLE, &val) != eSIR_SUCCESS) {
		cfg_log(pMac, LOGP, FL("cfg get WNI_CFG_SHORT_PREAMBLE failed"));
		return eSIR_FAILURE;
	}
	if (val)
		pCapInfo->shortPreamble = 1;

	/* PBCC bit */
	pCapInfo->pbcc = 0;

	/* Channel agility bit */
	pCapInfo->channelAgility = 0;
	/* If STA/AP operating in 11B mode, don't set rest of the capability info bits. */
	if (sessionEntry->dot11mode == WNI_CFG_DOT11_MODE_11B)
		return eSIR_SUCCESS;

	/* Short slot time bit */
	if (LIM_IS_AP_ROLE(sessionEntry)) {
		pCapInfo->shortSlotTime = sessionEntry->shortSlotTimeSupported;
	} else {
		if (wlan_cfg_get_int
			    (pMac, WNI_CFG_11G_SHORT_SLOT_TIME_ENABLED, &val)
		    != eSIR_SUCCESS) {
			cfg_log(pMac, LOGP,
				FL
					("cfg get WNI_CFG_11G_SHORT_SLOT_TIME failed"));
			return eSIR_FAILURE;
		}
		/* When in STA mode, we need to check if short slot is enabled as well as check if the current operating
		 * mode is short slot time and then decide whether to enable short slot or not. It is safe to check both
		 * cfg values to determine short slot value in this funcn since this funcn is always used after assoc when
		 * these cfg values are already set based on peer's capability. Even in case of IBSS, its value is set to
		 * correct value either in delBSS as part of deleting the previous IBSS or in start BSS as part of coalescing
		 */
		if (val) {
			pCapInfo->shortSlotTime =
				sessionEntry->shortSlotTimeSupported;
		}
	}

	/* Spectrum Management bit */
	if (!LIM_IS_IBSS_ROLE(sessionEntry) && sessionEntry->lim11hEnable) {
		if (wlan_cfg_get_int(pMac, WNI_CFG_11H_ENABLED, &val) !=
		    eSIR_SUCCESS) {
			cfg_log(pMac, LOGP,
				FL("cfg get WNI_CFG_11H_ENABLED failed"));
			return eSIR_FAILURE;
		}
		if (val)
			pCapInfo->spectrumMgt = 1;
	}
	/* QoS bit */
	if (wlan_cfg_get_int(pMac, WNI_CFG_QOS_ENABLED, &val) != eSIR_SUCCESS) {
		cfg_log(pMac, LOGP, FL("cfg get WNI_CFG_QOS_ENABLED failed"));
		return eSIR_FAILURE;
	}
	if (val)
		pCapInfo->qos = 1;

	/* APSD bit */
	if (wlan_cfg_get_int(pMac, WNI_CFG_APSD_ENABLED, &val) != eSIR_SUCCESS) {
		cfg_log(pMac, LOGP, FL("cfg get WNI_CFG_APSD_ENABLED failed"));
		return eSIR_FAILURE;
	}
	if (val)
		pCapInfo->apsd = 1;

	pCapInfo->rrm = pMac->rrm.rrmSmeContext.rrmConfig.rrm_enabled;
	cfg_log(pMac, LOG1, FL("RRM = %d"), pCapInfo->rrm);
	/* DSSS-OFDM */
	/* FIXME : no config defined yet. */

	/* Block ack bit */
	if (wlan_cfg_get_int(pMac, WNI_CFG_BLOCK_ACK_ENABLED, &val) !=
	    eSIR_SUCCESS) {
		cfg_log(pMac, LOGP,
			FL("cfg get WNI_CFG_BLOCK_ACK_ENABLED failed"));
		return eSIR_FAILURE;
	}
	pCapInfo->delayedBA =
		(uint16_t) ((val >> WNI_CFG_BLOCK_ACK_ENABLED_DELAYED) & 1);
	pCapInfo->immediateBA =
		(uint16_t) ((val >> WNI_CFG_BLOCK_ACK_ENABLED_IMMEDIATE) & 1);

	return eSIR_SUCCESS;
}

/* -------------------------------------------------------------------- */
/**
 * cfg_set_capability_info
 *
 * FUNCTION:
 * This function is called on BP based on the capabilities
 * received in SME_JOIN/REASSOC_REQ message.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE: 1. ESS/IBSS capabilities are based on system role.
 *       2. Since PBCC, Channel agility and Extended capabilities
 *          are not supported, they're not set at CFG
 *
 * @param  pMac   Pointer to global MAC structure
 * @param  caps   16-bit Capability Info field
 * @return None
 */

void cfg_set_capability_info(tpAniSirGlobal pMac, uint16_t caps)
{
}

/* --------------------------------------------------------------------- */
/**
 * cfg_cleanup()
 *
 * FUNCTION:
 * CFG cleanup function.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 * None.
 *
 * NOTE:
 * This function must be called during system shutdown.
 *
 * @param None
 *
 * @return None.
 *
 */

void cfg_cleanup(tpAniSirGlobal pMac)
{
	/* Set status to not-ready */
	pMac->cfg.gCfgStatus = CFG_INCOMPLETE;

} /*** end CfgCleanup() ***/

/* --------------------------------------------------------------------- */
/**
 * notify()
 *
 * FUNCTION:
 * This function is called to notify other modules of parameter update.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param cfgId:    configuration parameter ID
 * @param mask:     notification mask
 *
 * @return None.
 *
 */

static void notify(tpAniSirGlobal pMac, uint16_t cfgId, uint32_t ntfMask)
{

	tSirMsgQ mmhMsg;

	mmhMsg.type = SIR_CFG_PARAM_UPDATE_IND;
	mmhMsg.bodyval = (uint32_t) cfgId;
	mmhMsg.bodyptr = NULL;

	if ((ntfMask & CFG_CTL_NTF_SCH) != 0)
		sch_post_message(pMac, &mmhMsg);

	if ((ntfMask & CFG_CTL_NTF_LIM) != 0)
		lim_post_msg_api(pMac, &mmhMsg);

	if ((ntfMask & CFG_CTL_NTF_HAL) != 0)
		wma_post_ctrl_msg(pMac, &mmhMsg);

	/* notify ARQ */

} /*** end notify() ***/

/**
 * cfg_get_vendor_ie_ptr_from_oui() - returns IE pointer in IE buffer given its
 * OUI and OUI size
 * @mac_ctx:    mac context.
 * @oui:        OUI string.
 * @oui_size:   length of OUI string
 *              One can provide multiple line descriptions
 *              for arguments.
 * @ie:         ie buffer
 * @ie_len:     length of ie buffer
 *
 * This function parses the IE buffer and finds the given OUI and returns its
 * pointer
 *
 * Return: pointer of given OUI IE else NULL
 */
uint8_t *cfg_get_vendor_ie_ptr_from_oui(tpAniSirGlobal mac_ctx,
					uint8_t *oui,
					uint8_t oui_size,
					uint8_t *ie,
					uint16_t ie_len)
{
	int32_t left = ie_len;
	uint8_t *ptr = ie;
	uint8_t elem_id, elem_len;

	while (left >= 2) {
		elem_id  = ptr[0];
		elem_len = ptr[1];
		left -= 2;
		if (elem_len > left) {
			cfg_log(mac_ctx, LOGE,
				FL("Invalid IEs eid = %d elem_len=%d left=%d"),
				elem_id, elem_len, left);
			return NULL;
		}
		if (SIR_MAC_EID_VENDOR == elem_id) {
			if (memcmp(&ptr[2], oui, oui_size) == 0)
				return ptr;
		}

		left -= elem_len;
		ptr += (elem_len + 2);
	}
	return NULL;
}
/* --------------------------------------------------------------------- */
