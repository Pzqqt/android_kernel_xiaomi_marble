/*
 * Copyright (c) 2011-2012,2014-2015,2018 The Linux Foundation. All rights reserved.
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
 * This file lim_sme_req_utils.h contains the utility definitions
 * LIM uses while processing SME request messsages.
 * Author:        Chandra Modumudi
 * Date:          02/13/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 */
#ifndef __LIM_SME_REQ_UTILS_H
#define __LIM_SME_REQ_UTILS_H

#include "sir_api.h"
#include "lim_types.h"

/* LIM SME request messages related utility functions */
uint8_t lim_is_sme_start_bss_req_valid(tpAniSirGlobal, tpSirSmeStartBssReq);
uint8_t lim_set_rs_nie_wp_aiefrom_sme_start_bss_req_message(tpAniSirGlobal,
							    tpSirRSNie, tpPESession);
uint8_t lim_is_sme_join_req_valid(tpAniSirGlobal, tpSirSmeJoinReq);
uint8_t lim_is_sme_disassoc_req_valid(tpAniSirGlobal, tpSirSmeDisassocReq,
				      tpPESession);
uint8_t lim_is_sme_deauth_req_valid(tpAniSirGlobal, tpSirSmeDeauthReq, tpPESession);
uint8_t lim_is_sme_set_context_req_valid(tpAniSirGlobal, tpSirSmeSetContextReq);
uint8_t lim_is_sme_stop_bss_req_valid(uint32_t *);
uint8_t *lim_get_bss_id_from_sme_join_req_msg(uint8_t *);
uint8_t lim_is_sme_disassoc_cnf_valid(tpAniSirGlobal, tpSirSmeDisassocCnf,
				      tpPESession);

#endif /* __LIM_SME_REQ_UTILS_H */
