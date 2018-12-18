/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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
 * This file contains CFG functions for processing host messages.
 */
#include "cds_api.h"
#include "ani_global.h"
#include "cfg_priv.h"
#include "wma_types.h"
#include "lim_trace.h"

cgstatic cfg_static[CFG_PARAM_MAX_NUM] = {
	{WNI_CFG_STA_ID,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_RELOAD,
	0, 255, 1},
	{WNI_CFG_DOT11_MODE,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_INT | CFG_CTL_RESTART |
	CFG_CTL_NTF_LIM,
	WNI_CFG_DOT11_MODE_STAMIN,
	WNI_CFG_DOT11_MODE_STAMAX,
	WNI_CFG_DOT11_MODE_STADEF},
	{WNI_CFG_VALID_CHANNEL_LIST,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_RESTART |
	CFG_CTL_NTF_LIM,
	0, 1, 1},
	{WNI_CFG_APSD_ENABLED,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_INT,
	WNI_CFG_APSD_ENABLED_STAMIN,
	WNI_CFG_APSD_ENABLED_STAMAX,
	WNI_CFG_APSD_ENABLED_STADEF},
	{WNI_CFG_CURRENT_TX_POWER_LEVEL,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_INT,
	WNI_CFG_CURRENT_TX_POWER_LEVEL_STAMIN,
	WNI_CFG_CURRENT_TX_POWER_LEVEL_STAMAX,
	WNI_CFG_CURRENT_TX_POWER_LEVEL_STADEF},
	{WNI_CFG_COUNTRY_CODE,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_NTF_SCH,
	0, 0, 0},
	{WNI_CFG_LOCAL_POWER_CONSTRAINT,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_INT |
	CFG_CTL_RESTART,
	WNI_CFG_LOCAL_POWER_CONSTRAINT_STAMIN,
	WNI_CFG_LOCAL_POWER_CONSTRAINT_STAMAX,
	WNI_CFG_LOCAL_POWER_CONSTRAINT_STADEF},
	{WNI_CFG_SCAN_CONTROL_LIST,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_RESTART |
	CFG_CTL_NTF_LIM,
	0, 0, 0},
	{WNI_CFG_SCAN_IN_POWERSAVE,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_INT |
	CFG_CTL_NTF_LIM,
	WNI_CFG_SCAN_IN_POWERSAVE_STAMIN,
	WNI_CFG_SCAN_IN_POWERSAVE_STAMAX,
	WNI_CFG_SCAN_IN_POWERSAVE_STADEF},
	{WNI_CFG_IBSS_AUTO_BSSID,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_INT,
	WNI_CFG_IBSS_AUTO_BSSID_STAMIN,
	WNI_CFG_IBSS_AUTO_BSSID_STAMAX,
	WNI_CFG_IBSS_AUTO_BSSID_STADEF},
	{WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_INT,
	WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED_STAMIN,
	WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED_STAMAX,
	WNI_CFG_ENABLE_MCC_ADAPTIVE_SCHED_STADEF},
	{WNI_CFG_IBSS_ATIM_WIN_SIZE,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_INT,
	WNI_CFG_IBSS_ATIM_WIN_SIZE_STAMIN,
	WNI_CFG_IBSS_ATIM_WIN_SIZE_STAMAX,
	WNI_CFG_IBSS_ATIM_WIN_SIZE_STADEF},
	{WNI_CFG_PS_WOW_DATA_INACTIVITY_TIMEOUT,
	CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_INT,
	WNI_CFG_PS_WOW_DATA_INACTIVITY_TIMEOUT_STAMIN,
	WNI_CFG_PS_WOW_DATA_INACTIVITY_TIMEOUT_STAMAX,
	WNI_CFG_PS_WOW_DATA_INACTIVITY_TIMEOUT_STADEF},
	{WNI_CFG_OBSS_DETECTION_OFFLOAD,
	 CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_INT,
	 0, 1, 0},
	{WNI_CFG_OBSS_COLOR_COLLISION_OFFLOAD,
	 CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_INT,
	 0, 1, 0},
	{WNI_CFG_REMOVE_TIME_SYNC_CMD,
	 CFG_CTL_VALID | CFG_CTL_RE | CFG_CTL_WE | CFG_CTL_INT,
	 WNI_CFG_REMOVE_TIME_SYNC_CMD_STAMIN,
	 WNI_CFG_REMOVE_TIME_SYNC_CMD_STAMAX,
	 WNI_CFG_REMOVE_TIME_SYNC_CMD_STADEF},
};


cfgstatic_string cfg_static_string[CFG_MAX_STATIC_STRING] = {

	{WNI_CFG_STA_ID,
	WNI_CFG_STA_ID_LEN,
	6,
	{0x22, 0x22, 0x44, 0x44, 0x33, 0x33} },
	{WNI_CFG_VALID_CHANNEL_LIST,
	WNI_CFG_VALID_CHANNEL_LIST_LEN,
	55,
	{36, 40, 44, 48, 52, 56, 60, 64, 1, 6, 11, 34, 38, 42, 46, 2, 3, 4,
	 5, 7, 8, 9, 10, 12, 13, 14, 100, 104, 108, 112, 116, 120, 124, 128,
	 132, 136, 140, 149, 151, 153, 155, 157, 159, 161, 50, 54, 58, 62, 240,
	 242, 244, 246, 248, 250, 252} },
	{WNI_CFG_COUNTRY_CODE,
	WNI_CFG_COUNTRY_CODE_LEN,
	0,
	{0} },
	{WNI_CFG_SCAN_CONTROL_LIST,
	WNI_CFG_SCAN_CONTROL_LIST_LEN,
	114,
	{0x1, 0x1, 0x2, 0x1, 0x3, 0x1, 0x4, 0x1, 0x5, 0x1, 0x6, 0x1, 0x7, 0x1,
	 0x8, 0x1, 0x9, 0x1, 0xa, 0x1, 0xb, 0x1, 0xc, 0x1, 0xd, 0x1, 0xe, 0x1,
	 0x22, 0x1, 0x24, 0x1, 0x26, 0x1, 0x28, 0x1, 0x2a, 0x1, 0x2c, 0x1, 0x2e,
	 0x1, 0x30, 0x1, 0x32, 0x1, 0x34, 0x0, 0x36, 0x0, 0x38, 0x0, 0x3a, 0x0,
	 0x3c, 0x0, 0x3e, 0x0, 0x40, 0x0, 0x64, 0x0, 0x68, 0x0, 0x6c, 0x0, 0x70,
	 0x0, 0x74, 0x0, 0x78, 0x0, 0x7c, 0x0, 0x80, 0x0, 0x84, 0x0, 0x88, 0x0,
	 0x8c, 0x0, 0x90, 0x0, 0x95, 0x1, 0x97, 0x1, 0x99, 0x1, 0x9b, 0x1, 0x9d,
	 0x1, 0x9f, 0x1, 0xa1, 0x1, 0xa5, 0x1, 0xf0, 0x1, 0xf2, 0x1, 0xf4, 0x1,
	 0xf6, 0x1, 0xf8, 0x1, 0xfa, 0x1, 0xfc, 0x1} },
};

/*--------------------------------------------------------------------*/
/* Static function prototypes                                         */
/*--------------------------------------------------------------------*/
static void proc_dnld_rsp(struct mac_context *, uint16_t, uint32_t *);
static void proc_get_req(struct mac_context *, uint16_t, uint32_t *);

static uint8_t check_param(struct mac_context *, uint16_t, uint32_t, uint32_t,
			   uint32_t *);
/*--------------------------------------------------------------------*/
/* Module global variables                                            */
/*--------------------------------------------------------------------*/

/* CFG function table */
void (*g_cfg_func[])(struct mac_context *, uint16_t, uint32_t *) = {
	proc_dnld_rsp, proc_get_req
};

/**---------------------------------------------------------------------
 * cfg_process_mb_msg()
 *
 ***FUNCTION:
 * CFG mailbox message processing function.
 *
 ***LOGIC:
 *
 ***ASSUMPTIONS:
 * None.
 *
 ***NOTE:
 *
 * @param pMsg    Message pointer
 *
 * @return None.
 *
 */
void cfg_process_mb_msg(struct mac_context *mac, tSirMbMsg *pMsg)
{
	uint16_t index;
	uint16_t len;
	uint32_t *pParam;

	/* Use type[7:0] as index to function table */
	index = CFG_GET_FUNC_INDX(pMsg->type);

	if (index >= QDF_ARRAY_SIZE(g_cfg_func)) {
		qdf_mem_free(pMsg);
		return;
	}
	len = pMsg->msgLen - WNI_CFG_MB_HDR_LEN;
	pParam = ((uint32_t *) pMsg) + 1;

	/* Call processing function */
	g_cfg_func[index] (mac, len, pParam);

	/* Free up buffer */
	qdf_mem_free(pMsg);

} /*** end cfg_process_mb_msg() ***/

/**---------------------------------------------------------------------
 * proc_dnld_rsp()
 *
 * FUNCTION:
 * This function processes CFG_DNLD_RSP message from host.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param length:  message length
 * @param pParam:  parameter list pointer
 *
 * @return None
 *
 */
static void proc_dnld_rsp(struct mac_context *mac, uint16_t length, uint32_t *pParam)
{
	int32_t i;

	uint32_t expLen, retVal, bufStart, bufEnd;
	uint32_t *pSrc, *pDst, *pDstEnd;
	uint32_t strSize, j;
	uint8_t pStr[CFG_MAX_STR_LEN];
	tpCfgBinHdr pHdr;
	struct scheduler_msg mmhMsg = {0};

	/* First Dword must contain the AP or STA magic dword */
	pe_debug("CFG size: %d bytes MAGIC dword is: 0x%x",
		       length, sir_read_u32_n((uint8_t *) pParam));

	/* if the string is not correct, return failure */
	if (*pParam == CFG_STA_MAGIC_DWORD) {
	}

	else {
		pe_warn("Invalid magic dword: 0x%x",
			       sir_read_u32_n((uint8_t *) pParam));
		retVal = WNI_CFG_INVALID_LEN;
		goto end;
	}

	pParam++;
	length -= 4;

	/* Parse the Cfg header */
	pHdr = (tpCfgBinHdr) pParam;
	pParam += (sizeof(tCfgBinHdr) >> 2);
	pe_debug("CFG hdr totParams: %d intParams: %d strBufSize: %d/%d",
		       pHdr->controlSize, pHdr->iBufSize, pHdr->sBufSize,
		       mac->cfg.gCfgMaxSBufSize);

	if (pHdr->sBufSize > (UINT_MAX -
		(((CFG_PARAM_MAX_NUM + 3 * mac->cfg.gCfgMaxIBufSize) << 2) +
		sizeof(tCfgBinHdr)))) {
		pe_warn("Invalid sBufSize coming from fw: %d", pHdr->sBufSize);
		retVal = WNI_CFG_INVALID_LEN;
		goto end;
	}
	expLen =
		((CFG_PARAM_MAX_NUM + 3 * mac->cfg.gCfgMaxIBufSize) << 2) +
		pHdr->sBufSize + sizeof(tCfgBinHdr);

	if (length != expLen) {
		pe_warn("<CFG> DNLD_RSP invalid length: %d (exp: %d)", length,
			       expLen);
		retVal = WNI_CFG_INVALID_LEN;
		goto end;
	}

	if (pHdr->controlSize != CFG_PARAM_MAX_NUM) {
		pe_warn("<CFG> Total parameter count mismatch");
		retVal = WNI_CFG_INVALID_LEN;
		goto end;
	}

	if (pHdr->iBufSize != mac->cfg.gCfgMaxIBufSize) {
		pe_warn("<CFG> Integer parameter count mismatch");
		retVal = WNI_CFG_INVALID_LEN;
		goto end;
	}
	/* Copy control array */
	pDst = (uint32_t *) mac->cfg.gCfgEntry;
	pDstEnd = pDst + CFG_PARAM_MAX_NUM;
	pSrc = pParam;
	while (pDst < pDstEnd) {
		*pDst++ = *pSrc++;
	}
	/* Copy default values */
	pDst = mac->cfg.gCfgIBuf;
	pDstEnd = pDst + mac->cfg.gCfgMaxIBufSize;
	while (pDst < pDstEnd) {
		*pDst++ = *pSrc++;
	}

	/* Copy min values */
	pDst = mac->cfg.gCfgIBufMin;
	pDstEnd = pDst + mac->cfg.gCfgMaxIBufSize;
	while (pDst < pDstEnd) {
		*pDst++ = *pSrc++;
	}

	/* Copy max values */
	pDst = mac->cfg.gCfgIBufMax;
	pDstEnd = pDst + mac->cfg.gCfgMaxIBufSize;
	while (pDst < pDstEnd) {
		*pDst++ = *pSrc++;
	}

	for (i = 0; i < mac->cfg.gCfgMaxIBufSize; i++)
		if (mac->cfg.gCfgIBuf[i] < mac->cfg.gCfgIBufMin[i] ||
		    mac->cfg.gCfgIBuf[i] > mac->cfg.gCfgIBufMax[i]) {
			pe_debug("cfg id: %d Invalid def value: %d min: %d max: %d",
					i, mac->cfg.gCfgIBuf[i],
				       mac->cfg.gCfgIBufMin[i],
				       mac->cfg.gCfgIBufMax[i]);
		}
	/* Calculate max string buffer lengths for all string parameters */
	bufEnd = mac->cfg.gCfgMaxSBufSize;
	for (i = CFG_PARAM_MAX_NUM - 1; i >= 0; i--) {
		if ((mac->cfg.gCfgEntry[i].control & CFG_CTL_INT) != 0)
			continue;

		if ((mac->cfg.gCfgEntry[i].control & CFG_CTL_VALID) == 0)
			continue;

		bufStart = mac->cfg.gCfgEntry[i].control & CFG_BUF_INDX_MASK;
		mac->cfg.gCfgSBuf[bufStart] =
			(uint8_t) (bufEnd - bufStart - 2);

		pe_debug("id: %d max: %d bufStart: %d bufEnd: %d", i,
			       mac->cfg.gCfgSBuf[bufStart], bufStart, bufEnd);
		bufEnd = bufStart;
	}

	/* Initialize string defaults */
	strSize = pHdr->sBufSize;
	while (strSize) {
		uint32_t paramId, paramLen, paramLenCeil4;

		if (strSize < 4) {
			pe_warn("parsing str defaults, rem: %d bytes",
				       strSize);
			retVal = WNI_CFG_INVALID_LEN;
			goto end;
		}
		paramId = *pSrc >> 16;
		paramLen = *pSrc & 0xff;
		pSrc++;
		strSize -= 4;

		paramLenCeil4 = ((paramLen + 3) >> 2);
		if (strSize < paramLenCeil4 << 2) {
			pe_warn("parsing str defaults, rem: %d bytes",
				       strSize);
			pe_warn("param id: %d len: %d bytes",
					paramId, paramLen);
			retVal = WNI_CFG_INVALID_LEN;
			goto end;
		}
		for (j = 0; j < paramLenCeil4; j++) {
			pStr[4 * j] = (uint8_t) (*pSrc >> 24) & 0xff;
			pStr[4 * j + 1] = (uint8_t) (*pSrc >> 16) & 0xff;
			pStr[4 * j + 2] = (uint8_t) (*pSrc >> 8) & 0xff;
			pStr[4 * j + 3] = (uint8_t) (*pSrc) & 0xff;

			pSrc++;
			strSize -= 4;
		}

		pe_debug("set str id: %d len: %d", paramId, paramLen);

		if (cfg_set_str(mac, (uint16_t) paramId, pStr, paramLen) !=
		    QDF_STATUS_SUCCESS) {
			pe_warn("setting str default param %d len %d",
				       paramId, paramLen);
			retVal = WNI_CFG_INVALID_LEN;
			goto end;
		}
	}

	/* Set status to READY */
	mac->cfg.gCfgStatus = CFG_SUCCESS;
	retVal = WNI_CFG_SUCCESS;
	pe_debug("<CFG> Completed successfully");

end:

	if (retVal != WNI_CFG_SUCCESS)
		mac->cfg.gCfgStatus = CFG_FAILURE;

	/* Send response message to host */
	mac->cfg.gParamList[WNI_CFG_DNLD_CNF_RES] = retVal;
	cfg_send_host_msg(mac, WNI_CFG_DNLD_CNF, WNI_CFG_DNLD_CNF_LEN,
			  WNI_CFG_DNLD_CNF_NUM, mac->cfg.gParamList, 0, 0);

	/* notify WMA that the config has downloaded */
	mmhMsg.type = SIR_CFG_DOWNLOAD_COMPLETE_IND;
	mmhMsg.bodyptr = NULL;
	mmhMsg.bodyval = 0;

	MTRACE(mac_trace_msg_tx(mac, NO_SESSION, mmhMsg.type));
	if (wma_post_ctrl_msg(mac, &mmhMsg) != QDF_STATUS_SUCCESS) {
		pe_err("WMAPostMsgApi failed!");
	}

} /*** end procDnldRsp() ***/

/**---------------------------------------------------------------------
 * proc_get_req()
 *
 * FUNCTION:
 * This function processes CFG_GET_REQ message from host.
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 * For every parameter ID specified on the list, CFG will send a separate
 * CFG_GET_RSP back to host.
 *
 * @param length:  message length
 * @param pParam:  parameter list pointer
 *
 * @return None
 *
 */
static void proc_get_req(struct mac_context *mac, uint16_t length, uint32_t *pParam)
{
	uint16_t cfgId, i;
	uint32_t value, valueLen, result;
	uint32_t *pValue;

	pe_debug("Rcvd cfg get request %d bytes", length);
	for (i = 0; i < length / 4; i++)
		pe_debug("[%2d] 0x%08x", i, pParam[i]);

		if (!mac->cfg.gCfgStatus) {
			cfgId = (uint16_t) sir_read_u32_n((uint8_t *) pParam);
			pe_debug("CFG not ready, param %d", cfgId);
			mac->cfg.gParamList[WNI_CFG_GET_RSP_RES] =
				WNI_CFG_NOT_READY;
			mac->cfg.gParamList[WNI_CFG_GET_RSP_PID] = cfgId;
			mac->cfg.gParamList[WNI_CFG_GET_RSP_PLEN] = 0;
			cfg_send_host_msg(mac, WNI_CFG_GET_RSP,
					  WNI_CFG_GET_RSP_PARTIAL_LEN, WNI_CFG_GET_RSP_NUM,
					  mac->cfg.gParamList, 0, 0);
		} else {
			/* Process all parameter ID's on the list */
			while (length >= sizeof(uint32_t)) {
				cfgId = (uint16_t) *pParam++;
				pValue = 0;
				valueLen = 0;

				pe_debug("Cfg get param %d", cfgId);
				/* Check for valid parameter ID, etc... */
				if (check_param
					    (mac, cfgId, CFG_CTL_RE, WNI_CFG_WO_PARAM,
					    &result)) {
					if ((mac->cfg.gCfgEntry[cfgId].
					     control & CFG_CTL_INT) != 0) {
						/* Get integer parameter */
						result =
							(wlan_cfg_get_int(mac, cfgId, &value)
							 ==
							 QDF_STATUS_SUCCESS ? WNI_CFG_SUCCESS :
							 WNI_CFG_OTHER_ERROR);
						pValue = &value;
						valueLen = sizeof(uint32_t);
					} else {
						/* Get string parameter */
						valueLen = sizeof(mac->cfg.gSBuffer);
						result =
							(wlan_cfg_get_str
								 (mac, cfgId, mac->cfg.gSBuffer,
								 &valueLen)
							 == QDF_STATUS_SUCCESS ? WNI_CFG_SUCCESS :
							 WNI_CFG_OTHER_ERROR);
						pValue =
							(uint32_t *) mac->cfg.gSBuffer;
					}
				} else {
					pe_warn("Check param failed, param %d",
						       cfgId);
					result = WNI_CFG_INVALID_LEN;
				}

				/* Send response message to host */
				mac->cfg.gParamList[WNI_CFG_GET_RSP_RES] = result;
				mac->cfg.gParamList[WNI_CFG_GET_RSP_PID] = cfgId;
				mac->cfg.gParamList[WNI_CFG_GET_RSP_PLEN] = valueLen;

				/* We need to round up buffer length to word-increment */
				valueLen = (((valueLen + 3) >> 2) << 2);
				cfg_send_host_msg(mac, WNI_CFG_GET_RSP,
						  WNI_CFG_GET_RSP_PARTIAL_LEN + valueLen,
						  WNI_CFG_GET_RSP_NUM,
						  mac->cfg.gParamList, valueLen, pValue);

				/* Decrement length */
				length -= sizeof(uint32_t);
			}
		}

} /*** end procGetReq() ***/

/**---------------------------------------------------------------------
 * check_param()
 *
 * FUNCTION:
 * This function is called to perform various check on a parameter.
 *
 * LOGIC:
 * -  If cfgId is out of bound or parameter is not valid, result
 *    WNI_CFG_INVALID_PID is returned at address specified in pResult.
 *
 * -  If specified 'flag' is not set in the parameter control entry,
 *    'failedResult' is returned at address specified in pResult.
 *
 * ASSUMPTIONS:
 * Since this function is used internally, 'pResult' is always valid.
 *
 * NOTE:
 *
 * @param None
 *
 * @return true:   Parameter is valid and matches checked condition \n
 * @return false:  Parameter either is not valid or does not match
 *                 checked condition.
 *
 */
static uint8_t
check_param(struct mac_context *mac, uint16_t cfgId, uint32_t flag,
	    uint32_t failedResult, uint32_t *pResult)
{
	/* Check if parameter ID is out of bound */
	if (cfgId >= CFG_PARAM_MAX_NUM) {
		pe_warn("Invalid param id: %d", cfgId);
		*pResult = WNI_CFG_INVALID_PID;
	} else {
		/* Check if parameter is valid */
		if ((mac->cfg.gCfgEntry[cfgId].control & CFG_CTL_VALID) == 0) {
			pe_warn("Param id: %d not valid", cfgId);
			*pResult = WNI_CFG_INVALID_PID;
		} else {
			/* Check control field against flag */
			if ((mac->cfg.gCfgEntry[cfgId].control & flag) == 0) {
				pe_debug("Param id: %d wrong permissions: %x",
					       cfgId,
					       mac->cfg.gCfgEntry[cfgId].control);
				*pResult = failedResult;
			} else
				return true;
		}
	}
	return false;

} /*** cfgParamCheck() ***/

/**---------------------------------------------------------------------
 * process_cfg_download_req()
 *
 * FUNCTION: This function does the Cfg Download and is invoked
 *           only in the case of Prima or the Integrated SOC
 *           solutions. Not applicable to Volans or Libra
 *
 * LOGIC:
 *
 * ASSUMPTIONS:
 *
 * NOTE:
 *
 * @param mac: Pointer to Mac Structure
 *
 * @return None
 *
 */

void
process_cfg_download_req(struct mac_context *mac)
{
	int32_t i;
	uint32_t    index;
	uint8_t    *pDstTest, *pSrcTest;
	uint8_t     len;
	cfgstatic_string * pStrCfg;
	uint32_t    bufStart, bufEnd;
	uint32_t    retVal;
	uint32_t    iCount = 0;
	uint32_t    sCount = 0;

	for (i = 0; i < CFG_PARAM_MAX_NUM ; i++) {
		if ((cfg_static[i].control & CFG_CTL_VALID) != 0) {
			if (!(cfg_static[i].control & CFG_CTL_INT)) {
				pStrCfg = (cfgstatic_string*)cfg_static[i].
								pStrData;
				if (pStrCfg == NULL) {
					pe_err("pStrCfg is NULL for CfigID : %d",
					  i);
					continue;
				}
				index = sCount & CFG_BUF_INDX_MASK;
				sCount += pStrCfg->maxLen + 1 + 1;
			} else {
				index = iCount & CFG_BUF_INDX_MASK;
				iCount++;
			}
		} else {
			index = 0;
		}
		mac->cfg.gCfgEntry[i].control = cfg_static[i].control | index;
	}

	/*Fill the SBUF wih maxLength*/
	bufEnd = mac->cfg.gCfgMaxSBufSize;
	for (i = CFG_PARAM_MAX_NUM - 1; i >= 0; i--) {
		if ((mac->cfg.gCfgEntry[i].control & CFG_CTL_INT) != 0)
			continue;

		if ((mac->cfg.gCfgEntry[i].control & CFG_CTL_VALID) == 0)
			continue;

		bufStart = mac->cfg.gCfgEntry[i].control & CFG_BUF_INDX_MASK;
		mac->cfg.gCfgSBuf[bufStart] = (uint8_t)(bufEnd - bufStart - 2);

		pe_debug("id: %d max: %d bufStart: %d bufEnd: %d",
					i, mac->cfg.gCfgSBuf[bufStart],
						bufStart, bufEnd);
			bufEnd = bufStart;
	}

	for (i = 0; i < CFG_PARAM_MAX_NUM ; i++) {
		index = mac->cfg.gCfgEntry[i].control & CFG_BUF_INDX_MASK;

		if ((mac->cfg.gCfgEntry[i].control & CFG_CTL_INT) != 0) {
			mac->cfg.gCfgIBufMin[index] = cfg_static[i].cfgIMin;
			mac->cfg.gCfgIBufMax[index] = cfg_static[i].cfgIMax;
			mac->cfg.gCfgIBuf[index]    = cfg_static[i].cfgIVal;
		} else {
			uint8_t maxSavedLen;

			if ((mac->cfg.gCfgEntry[i].control & CFG_CTL_VALID) == 0)
				continue;
			if (index >= mac->cfg.gCfgMaxSBufSize) {
				pe_debug("No space id:%d BufSize:%d index:%d",
					 i, mac->cfg.gCfgMaxSBufSize, index);
				continue;
			}

			pDstTest = &mac->cfg.gCfgSBuf[index];
			pStrCfg = (cfgstatic_string*)cfg_static[i].pStrData;
			pSrcTest = pStrCfg->data;
			if ((pDstTest == NULL) || (pStrCfg == NULL) ||
							(pSrcTest == NULL))
				continue;
			maxSavedLen = *pDstTest;
			len = pStrCfg->length;
			if (len > maxSavedLen)
				continue;
			*pDstTest++ = pStrCfg->maxLen;
			*pDstTest++ = len;
			while (len) {
				*pDstTest++ = *pSrcTest++;
				len--;
			}
		}
	}

	/* Set status to READY */
	mac->cfg.gCfgStatus = CFG_SUCCESS;
	retVal = WNI_CFG_SUCCESS;
	pe_debug("<CFG> Completed successfully");

	mac->cfg.gParamList[WNI_CFG_DNLD_CNF_RES] = retVal;

} /*** end ProcessDownloadReq() ***/
