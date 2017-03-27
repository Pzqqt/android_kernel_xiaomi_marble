/*
 * Copyright (c) 2011-2012, 2015-2017 The Linux Foundation. All rights reserved.
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
tSirRetStatus cfg_set_int(tpAniSirGlobal, uint16_t, uint32_t);

/* / Check if the parameter is valid */
tSirRetStatus cfg_check_valid(tpAniSirGlobal, uint16_t, uint32_t *);

/* / Get integer parameter value */
tSirRetStatus wlan_cfg_get_int(tpAniSirGlobal, uint16_t, uint32_t *);

/* / Set string parameter value */
tSirRetStatus cfg_set_str(tpAniSirGlobal, uint16_t, uint8_t *, uint32_t);

tSirRetStatus cfg_set_str_notify(tpAniSirGlobal, uint16_t, uint8_t *, uint32_t,
				 int);

/* Cfg Download function for Prima or Integrated solutions. */
void process_cfg_download_req(tpAniSirGlobal);

/* / Get string parameter value */
tSirRetStatus wlan_cfg_get_str(tpAniSirGlobal, uint16_t, uint8_t *, uint32_t *);

/* / Get string parameter maximum length */
tSirRetStatus wlan_cfg_get_str_max_len(tpAniSirGlobal, uint16_t, uint32_t *);

/* / Get string parameter maximum length */
tSirRetStatus wlan_cfg_get_str_len(tpAniSirGlobal, uint16_t, uint32_t *);

/* / Get the regulatory tx power on given channel */
int8_t cfg_get_regulatory_max_transmit_power(tpAniSirGlobal pMac,
					     uint8_t channel);

/* / Dump CFG data to memory */
void cfgDump(uint32_t *);

/* / Save parameters with P flag set */
void cfgSave(void);

/* / Get capability info */
extern tSirRetStatus cfg_get_capability_info(tpAniSirGlobal pMac, uint16_t *pCap,
					     tpPESession psessionEntry);

/* / Set capability info */
extern void cfg_set_capability_info(tpAniSirGlobal, uint16_t);

/* / Cleanup CFG module */
void cfg_cleanup(tpAniSirGlobal pMac);

extern uint8_t *g_cfg_param_name[];

uint8_t *cfg_get_vendor_ie_ptr_from_oui(tpAniSirGlobal mac_ctx,
					uint8_t *oui,
					uint8_t oui_size,
					uint8_t *ie,
					uint16_t ie_len);

#endif /* __CFGAPI_H */
