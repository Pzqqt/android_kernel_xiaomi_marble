/*
 * Copyright (c) 2011-2019 The Linux Foundation. All rights reserved.
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
#include "wma_types.h"
#include "cfg_api.h"

/* --------------------------------------------------------------------- */
/* Static Variables */
/* ---------------------------------------------------------------------- */
static tCfgCtl __g_cfg_entry[CFG_PARAM_MAX_NUM];
static uint8_t __g_s_buffer[CFG_MAX_STR_LEN];
static uint32_t __g_param_list[WNI_CFG_MAX_PARAM_NUM +
			     WNI_CFG_GET_PER_STA_STAT_RSP_NUM];

static void notify(struct mac_context *, uint16_t, uint32_t);

typedef enum {
	eRF_BAND_UNKNOWN = 0,
	eRF_BAND_2_4_GHZ = 1,
	eRF_BAND_5_GHZ = 2
} eRfBandMode;

extern cfgstatic_string cfg_static_string[CFG_MAX_STATIC_STRING];
extern cgstatic cfg_static[CFG_PARAM_MAX_NUM];

/* --------------------------------------------------------------------- */
uint32_t cfg_need_restart(struct mac_context *mac, uint16_t cfgId)
{
	if (!mac->cfg.gCfgEntry) {
		pe_err("gCfgEntry is NULL");
		return 0;
	}
	return !!(mac->cfg.gCfgEntry[cfgId].control & CFG_CTL_RESTART);
}

static void cfg_get_strindex(struct mac_context *mac, uint16_t cfgId)
{
	uint16_t i = 0;

	for (i = 0; i < CFG_MAX_STATIC_STRING; i++) {
		if (cfgId == cfg_static_string[i].cfgId)
			break;
	}
	if (i == CFG_MAX_STATIC_STRING) {
		pe_warn("Entry not found for cfg id: %d", cfgId);
		cfg_static[cfgId].pStrData = NULL;
		return;
	}
	cfg_static[cfgId].pStrData = &cfg_static_string[i];
}
/* --------------------------------------------------------------------- */
uint32_t cfg_need_reload(struct mac_context *mac, uint16_t cfgId)
{
	if (!mac->cfg.gCfgEntry) {
		pe_err("gCfgEntry is NULL");
		return 0;
	}
	return !!(mac->cfg.gCfgEntry[cfgId].control & CFG_CTL_RELOAD);
}

/* --------------------------------------------------------------------- */
QDF_STATUS cfg_init(struct mac_context *mac)
{
	uint16_t i = 0;
	uint16_t combined_buff_size = 0;
	uint32_t    max_i_count = 0;
	uint32_t    max_s_count = 0;
	cfgstatic_string *str_cfg;

	mac->cfg.gSBuffer = __g_s_buffer;
	mac->cfg.gCfgEntry = __g_cfg_entry;
	mac->cfg.gParamList = __g_param_list;

	for (i = 0; i < CFG_PARAM_MAX_NUM; i++) {
		if (!(cfg_static[i].control & CFG_CTL_INT)) {
			cfg_get_strindex(mac, i);
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
				pe_warn("pStrCfg is NULL for CfigID: %d", i);
				continue;
			}
			/* + 2 to include len field and max len field */
			max_s_count += str_cfg->maxLen + 2;
		}
	}

	mac->cfg.gCfgMaxIBufSize = max_i_count;
	mac->cfg.gCfgMaxSBufSize = max_s_count;

	/* Allocate a combined memory */
	combined_buff_size = max_s_count + (3 * sizeof(uint32_t) * max_i_count);

	pe_debug("Size of cfg I buffer: %d  S buffer: %d",
		max_i_count, max_s_count);

	pe_debug("Allocation for cfg buffers: %d bytes", combined_buff_size);

	if (combined_buff_size > 4 * PAGE_SIZE) {
		pe_err("Mem alloc request too big");
		return QDF_STATUS_E_NOMEM;
	}
	/* at this point mac->cfg.gCfgSBuf starts */
	mac->cfg.gCfgSBuf = qdf_mem_malloc(combined_buff_size);
	if (!mac->cfg.gCfgSBuf)
		return QDF_STATUS_E_NOMEM;

	/* at offset max_s_count, mac->cfg.gCfgIBuf starts */
	mac->cfg.gCfgIBuf = (uint32_t *)&mac->cfg.gCfgSBuf[max_s_count];
	/* after max_i_count integers, mac->cfg.gCfgIBufMin starts */
	mac->cfg.gCfgIBufMin = &mac->cfg.gCfgIBuf[max_i_count];
	/* after max_i_count integers, mac->cfg.gCfgIBufMax starts */
	mac->cfg.gCfgIBufMax = &mac->cfg.gCfgIBufMin[max_i_count];

	return QDF_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------- */
void cfg_de_init(struct mac_context *mac)
{
	qdf_mem_free(mac->cfg.gCfgSBuf);
	mac->cfg.gCfgIBufMin = NULL;
	mac->cfg.gCfgIBufMax = NULL;
	mac->cfg.gCfgIBuf = NULL;
	mac->cfg.gCfgSBuf = NULL;
	mac->cfg.gSBuffer = NULL;
	mac->cfg.gCfgEntry = NULL;
	mac->cfg.gParamList = NULL;
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
 * @return QDF_STATUS_SUCCESS:  request completed successfully
 * @return QDF_STATUS_E_INVAL:  invalid CFG parameter ID
 */
QDF_STATUS cfg_check_valid(struct mac_context *mac, uint16_t cfgId,
			   uint32_t *index)
{
	uint32_t control;

	if (cfgId >= CFG_PARAM_MAX_NUM) {
		pe_warn("Invalid cfg id: %d", cfgId);
		return QDF_STATUS_E_INVAL;
	}
	if (!mac->cfg.gCfgEntry) {
		pe_warn("gCfgEntry is NULL");
		return QDF_STATUS_E_INVAL;
	}

	control = mac->cfg.gCfgEntry[cfgId].control;

	/* Check if parameter is valid */
	if ((control & CFG_CTL_VALID) == 0) {
		pe_warn("Not valid cfg id: %d", cfgId);
		return QDF_STATUS_E_INVAL;
	}

	*index = control & CFG_BUF_INDX_MASK;

	if (*index >= mac->cfg.gCfgMaxSBufSize) {
		pe_warn("cfg index out of bounds: %d", *index);
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;

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
 * @return QDF_STATUS_SUCCESS:  request completed successfully
 * @return QDF_STATUS_E_INVAL:  invalid CFG parameter ID
 */

QDF_STATUS cfg_set_int(struct mac_context *mac, uint16_t cfgId, uint32_t value)
{
	uint32_t index;
	uint32_t control;
	uint32_t mask;
	QDF_STATUS status;

	status = cfg_check_valid(mac, cfgId, &index);

	if (QDF_STATUS_SUCCESS != status)
		return status;

	if ((mac->cfg.gCfgIBufMin[index] > value) ||
			(mac->cfg.gCfgIBufMax[index] < value)) {
		pe_warn("Value: %d out of range: [%d,%d] cfg id: %d, %s",
				value, mac->cfg.gCfgIBufMin[index],
				mac->cfg.gCfgIBufMax[index], cfgId,
				cfg_get_string(cfgId));
		return QDF_STATUS_E_INVAL;
	} else {
		/* Write integer value */
		mac->cfg.gCfgIBuf[index] = value;

		control = mac->cfg.gCfgEntry[cfgId].control;
		/* Update hardware if necessary */
		mask = control & CFG_CTL_NTF_MASK;
		if ((mask & CFG_CTL_NTF_HW) != 0)
			pe_debug("CFG notify HW not supported!!!");
			/* notify other modules if necessary */
			if ((mask & CFG_CTL_NTF_MASK) != 0)
				notify(mac, cfgId, mask);
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
 * @return QDF_STATUS_SUCCESS:  request completed successfully
 * @return QDF_STATUS_E_INVAL:  invalid CFG parameter ID
 */

QDF_STATUS wlan_cfg_get_int(struct mac_context *mac, uint16_t cfgId,
			    uint32_t *pValue)
{
	uint32_t index;
	QDF_STATUS status;

	status = cfg_check_valid(mac, cfgId, &index);

	if (QDF_STATUS_SUCCESS != status)
		return status;

	/* Get integer value */
	*pValue = mac->cfg.gCfgIBuf[index];

	return QDF_STATUS_SUCCESS;
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
 * @return QDF_STATUS_SUCCESS:  request completed successfully
 * @return QDF_STATUS_E_INVAL:  invalid CFG parameter ID
 * @return QDF_STATUS_E_INVAL:  invalid CFG parameter length
 *
 */

QDF_STATUS cfg_set_str(struct mac_context *mac, uint16_t cfgId, uint8_t *pStr,
		       uint32_t length)
{
	return cfg_set_str_notify(mac, cfgId, pStr, length, true);
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
 * @return QDF_STATUS_SUCCESS:  request completed successfully
 * @return QDF_STATUS_E_INVAL:  invalid CFG parameter ID
 * @return QDF_STATUS_E_INVAL:  invalid CFG parameter length
 *
 */

QDF_STATUS cfg_set_str_notify(struct mac_context *mac, uint16_t cfgId,
			      uint8_t *pStr, uint32_t length,
			      int notifyMod)
{
	uint8_t *pDst, *pDstEnd;
	uint32_t index, paramLen, mask;
	uint32_t control;
	QDF_STATUS status;

	status = cfg_check_valid(mac, cfgId, &index);

	if (QDF_STATUS_SUCCESS != status)
		return status;

	pDst = &mac->cfg.gCfgSBuf[index];
	paramLen = *pDst++;
	control = mac->cfg.gCfgEntry[cfgId].control;
	if (length > paramLen) {
		pe_warn("Invalid length: %d (>%d) cfg id: %d",
			length, paramLen, cfgId);
			return QDF_STATUS_E_INVAL;
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
				pe_debug("CFG notify HW not supported!");
			}
			/* notify other modules if necessary */
			if ((mask & CFG_CTL_NTF_MASK) != 0) {
				notify(mac, cfgId, mask);
			}
		}
	}
	return QDF_STATUS_SUCCESS;
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
 * @return QDF_STATUS_SUCCESS:  request completed successfully
 * @return QDF_STATUS_E_INVAL:  invalid CFG parameter ID
 * @return QDF_STATUS_E_INVAL:  invalid CFG parameter length
 *
 */

QDF_STATUS wlan_cfg_get_str(struct mac_context *mac, uint16_t cfgId,
			    uint8_t *pBuf, uint32_t *pLength)
{
	uint8_t *pSrc, *pSrcEnd;
	uint32_t index;
	QDF_STATUS status;

	status = cfg_check_valid(mac, cfgId, &index);

	if (QDF_STATUS_SUCCESS != status)
		return status;

	/* Get string */
	pSrc = &mac->cfg.gCfgSBuf[index];
	pSrc++;         /* skip over max length */
	if (*pLength < *pSrc) {
		pe_warn("Invalid length: %d (<%d) cfg id: %d",
			*pLength, *pSrc, cfgId);
			return QDF_STATUS_E_INVAL;
	} else {
		*pLength = *pSrc++;     /* save parameter length */
		pSrcEnd = pSrc + *pLength;
		while (pSrc < pSrcEnd)
			*pBuf++ = *pSrc++;
	}
	return QDF_STATUS_SUCCESS;
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
 * @return QDF_STATUS_SUCCESS:  request completed successfully
 * @return QDF_STATUS_E_INVAL:  invalid CFG parameter ID
 *
 */

QDF_STATUS wlan_cfg_get_str_max_len(struct mac_context *mac, uint16_t cfgId,
				    uint32_t *pLength)
{
	uint32_t index;
	QDF_STATUS status;

	status = cfg_check_valid(mac, cfgId, &index);

	if (QDF_STATUS_SUCCESS != status)
		return status;

	*pLength = mac->cfg.gCfgSBuf[index];

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
 * @return QDF_STATUS_SUCCESS:         request completed successfully
 * @return QDF_STATUS_E_INVAL:  invalid CFG parameter ID
 *
 */

QDF_STATUS wlan_cfg_get_str_len(struct mac_context *mac, uint16_t cfgId,
				uint32_t *pLength)
{
	uint32_t index;
	QDF_STATUS status;

	status = cfg_check_valid(mac, cfgId, &index);

	if (QDF_STATUS_SUCCESS != status)
		return status;

	*pLength = mac->cfg.gCfgSBuf[index + 1];

	return status;

} /*** end wlan_cfg_get_str_len() ***/

/**
 * cfg_get_dot11d_transmit_power() - regulatory max transmit power
 * @pMac: pointer to mac data
 * @channel: channel number
 *
 * Return:  int8_t - power
 */
static int8_t
cfg_get_dot11d_transmit_power(struct mac_context *pMac, uint8_t channel)
{
	uint32_t cfg_length = 0;
	int8_t max_tx_pwr = 0;
	uint8_t *country_info = NULL;
	uint8_t count = 0;
	uint8_t first_channel;
	uint8_t maxChannels;

	if (WLAN_REG_IS_5GHZ_CH(channel))
		cfg_length = pMac->mlme_cfg->power.max_tx_power_5.len;
	else if (WLAN_REG_IS_24GHZ_CH(channel))
		cfg_length = pMac->mlme_cfg->power.max_tx_power_24.len;
	else
		return max_tx_pwr;

	country_info = qdf_mem_malloc(cfg_length);
	if (!country_info)
		goto error;

	if (WLAN_REG_IS_5GHZ_CH(channel)) {
		qdf_mem_copy(country_info,
			     pMac->mlme_cfg->power.max_tx_power_5.data,
			     cfg_length);
	} else if (WLAN_REG_IS_24GHZ_CH(channel)) {
		qdf_mem_copy(country_info,
			     pMac->mlme_cfg->power.max_tx_power_24.data,
			     cfg_length);
	}

	/* Identify the channel and maxtxpower */
	while (count <= (cfg_length - (sizeof(tSirMacChanInfo)))) {
		first_channel = country_info[count++];
		maxChannels = country_info[count++];
		max_tx_pwr = country_info[count++];

		if ((channel >= first_channel) &&
		    (channel < (first_channel + maxChannels))) {
			break;
		}
	}

error:
	if (country_info)
		qdf_mem_free(country_info);

	return max_tx_pwr;
}

/**----------------------------------------------------------------------
   \fn     cfg_get_regulatory_max_transmit_power

   \brief  Gets regulatory tx power on the current channel.

   \param  mac
   \param  channel
   \param  rfBand
   -----------------------------------------------------------------------*/
int8_t cfg_get_regulatory_max_transmit_power(struct mac_context *mac,
					     uint8_t channel)
{
	return cfg_get_dot11d_transmit_power(mac, channel);
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

QDF_STATUS cfg_get_capability_info(struct mac_context *mac, uint16_t *pCap,
				   struct pe_session *pe_session)
{
	uint32_t val = 0;
	tpSirMacCapabilityInfo pCapInfo;

	*pCap = 0;
	pCapInfo = (tpSirMacCapabilityInfo) pCap;

	if (LIM_IS_IBSS_ROLE(pe_session))
		pCapInfo->ibss = 1;     /* IBSS bit */
	else if (LIM_IS_AP_ROLE(pe_session) ||
		LIM_IS_STA_ROLE(pe_session))
		pCapInfo->ess = 1;      /* ESS bit */
	else if (LIM_IS_P2P_DEVICE_ROLE(pe_session) ||
		LIM_IS_NDI_ROLE(pe_session)) {
		pCapInfo->ess = 0;
		pCapInfo->ibss = 0;
	} else
		pe_warn("can't get capability, role is UNKNOWN!!");

	if (LIM_IS_AP_ROLE(pe_session)) {
		val = pe_session->privacy;
	} else {
		/* PRIVACY bit */
		val = mac->mlme_cfg->wep_params.is_privacy_enabled;
	}
	if (val)
		pCapInfo->privacy = 1;

	/* Short preamble bit */
	if (mac->mlme_cfg->ht_caps.short_preamble)
		pCapInfo->shortPreamble =
			mac->mlme_cfg->ht_caps.short_preamble;

	/* PBCC bit */
	pCapInfo->pbcc = 0;

	/* Channel agility bit */
	pCapInfo->channelAgility = 0;
	/* If STA/AP operating in 11B mode, don't set rest of the
	 * capability info bits.
	 */
	if (pe_session->dot11mode == WNI_CFG_DOT11_MODE_11B)
		return QDF_STATUS_SUCCESS;

	/* Short slot time bit */
	if (LIM_IS_AP_ROLE(pe_session)) {
		pCapInfo->shortSlotTime = pe_session->shortSlotTimeSupported;
	} else {
		/* When in STA mode, we need to check if short slot is
		 * enabled as well as check if the current operating
		 * mode is short slot time and then decide whether to
		 * enable short slot or not. It is safe to check both
		 * cfg values to determine short slot value in this
		 * funcn since this funcn is always used after assoc
		 * when these cfg values are already set based on
		 * peer's capability. Even in case of IBSS, its value
		 * is set to correct value either in delBSS as part of
		 * deleting the previous IBSS or in start BSS as part
		 * of coalescing
		 */
		if (mac->mlme_cfg->feature_flags.enable_short_slot_time_11g) {
			pCapInfo->shortSlotTime =
				pe_session->shortSlotTimeSupported;
		}
	}

	/* Spectrum Management bit */
	if (!LIM_IS_IBSS_ROLE(pe_session) && pe_session->lim11hEnable) {
		if (mac->mlme_cfg->gen.enabled_11h)
			pCapInfo->spectrumMgt = 1;
	}
	/* QoS bit */
	if (mac->mlme_cfg->wmm_params.qos_enabled)
		pCapInfo->qos = 1;

	/* APSD bit */
	if (mac->mlme_cfg->scoring.apsd_enabled)
		pCapInfo->apsd = 1;

	pCapInfo->rrm = mac->rrm.rrmSmeContext.rrmConfig.rrm_enabled;
	pe_debug("RRM: %d", pCapInfo->rrm);
	/* DSSS-OFDM */
	/* FIXME : no config defined yet. */

	/* Block ack bit */
	val = mac->mlme_cfg->feature_flags.enable_block_ack;
	pCapInfo->delayedBA =
		(uint16_t) ((val >> WNI_CFG_BLOCK_ACK_ENABLED_DELAYED) & 1);
	pCapInfo->immediateBA =
		(uint16_t) ((val >> WNI_CFG_BLOCK_ACK_ENABLED_IMMEDIATE) & 1);

	return QDF_STATUS_SUCCESS;
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
 * @param  mac   Pointer to global MAC structure
 * @param  caps   16-bit Capability Info field
 * @return None
 */

void cfg_set_capability_info(struct mac_context *mac, uint16_t caps)
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

void cfg_cleanup(struct mac_context *mac)
{
	/* Set status to not-ready */
	mac->cfg.gCfgStatus = CFG_INCOMPLETE;

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

static void notify(struct mac_context *mac, uint16_t cfgId, uint32_t ntfMask)
{

	struct scheduler_msg mmhMsg = {0};

	mmhMsg.type = SIR_CFG_PARAM_UPDATE_IND;
	mmhMsg.bodyval = (uint32_t) cfgId;
	mmhMsg.bodyptr = NULL;

	if ((ntfMask & CFG_CTL_NTF_SCH) != 0)
		sch_post_message(mac, &mmhMsg);

	if ((ntfMask & CFG_CTL_NTF_LIM) != 0)
		lim_post_msg_api(mac, &mmhMsg);

	if ((ntfMask & CFG_CTL_NTF_TARGET) != 0)
		wma_post_ctrl_msg(mac, &mmhMsg);

	/* notify ARQ */

} /*** end notify() ***/

