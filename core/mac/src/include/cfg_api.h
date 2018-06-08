/*
 * Copyright (c) 2011-2012, 2015-2018 The Linux Foundation. All rights reserved.
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
 * Author:      Kevin Nguyen
 * Date:        04/09/02
 * History:-
 * 04/09/02        Created.
 * --------------------------------------------------------------------
 *
 */

#ifndef __CFGAPI_H
#define __CFGAPI_H

#include <sir_common.h>
#include <sir_params.h>
#include <sir_mac_prot_def.h>
#include <wni_api.h>
#include <ani_global.h>

/*---------------------------------------------------------------------*/
/* CFG definitions                                                     */
/*---------------------------------------------------------------------*/

/* CFG status */
typedef enum eCfgStatusTypes {
	CFG_INCOMPLETE,
	CFG_SUCCESS,
	CFG_FAILURE
} tCfgStatusTypes;

/* WEP key mapping table row structure */
typedef struct {
	uint8_t keyMappingAddr[QDF_MAC_ADDR_SIZE];
	uint32_t wepOn;
	uint8_t key[SIR_MAC_KEY_LENGTH];
	uint32_t status;
} tCfgWepKeyEntry;

/*---------------------------------------------------------------------*/
/* CFG function prototypes                                             */
/*---------------------------------------------------------------------*/

uint32_t cfg_need_restart(tpAniSirGlobal pMac, uint16_t cfgId);
uint32_t cfg_need_reload(tpAniSirGlobal pMac, uint16_t cfgId);

/* / Process host message */
void cfg_process_mb_msg(tpAniSirGlobal, tSirMbMsg *);

/* / Set integer parameter value */
QDF_STATUS cfg_set_int(tpAniSirGlobal, uint16_t, uint32_t);

/* / Check if the parameter is valid */
QDF_STATUS cfg_check_valid(tpAniSirGlobal, uint16_t, uint32_t *);

/* / Get integer parameter value */
QDF_STATUS wlan_cfg_get_int(tpAniSirGlobal, uint16_t, uint32_t *);

/* / Set string parameter value */
QDF_STATUS cfg_set_str(tpAniSirGlobal, uint16_t, uint8_t *, uint32_t);

QDF_STATUS cfg_set_str_notify(tpAniSirGlobal, uint16_t, uint8_t *, uint32_t,
			      int);

/* Cfg Download function for Prima or Integrated solutions. */
void process_cfg_download_req(tpAniSirGlobal);

/* / Get string parameter value */
QDF_STATUS wlan_cfg_get_str(tpAniSirGlobal, uint16_t, uint8_t *, uint32_t *);

/* / Get string parameter maximum length */
QDF_STATUS wlan_cfg_get_str_max_len(tpAniSirGlobal, uint16_t, uint32_t *);

/* / Get string parameter maximum length */
QDF_STATUS wlan_cfg_get_str_len(tpAniSirGlobal, uint16_t, uint32_t *);

/* / Get the regulatory tx power on given channel */
int8_t cfg_get_regulatory_max_transmit_power(tpAniSirGlobal pMac,
					     uint8_t channel);

/* / Get capability info */
QDF_STATUS cfg_get_capability_info(tpAniSirGlobal pMac, uint16_t *pCap,
				   tpPESession psessionEntry);

/* / Set capability info */
void cfg_set_capability_info(tpAniSirGlobal, uint16_t);

/* / Cleanup CFG module */
void cfg_cleanup(tpAniSirGlobal pMac);

const char *cfg_get_string(uint16_t cfg_id);

#endif /* __CFGAPI_H */
