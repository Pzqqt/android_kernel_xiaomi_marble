/*
 * Copyright (c) 2013-2014, 2017-2019, 2021 The Linux Foundation. All rights reserved.
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
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#ifndef __SCH_GLOBAL_H__
#define __SCH_GLOBAL_H__

#include "sir_mac_prop_exts.h"
#include "lim_global.h"

#include "parser_api.h"

#define TIM_IE_SIZE 0xB

/* ----------------------- Beacon processing ------------------------ */

/* / Beacon structure */
#define tSchBeaconStruct tSirProbeRespBeacon
#define tpSchBeaconStruct struct sSirProbeRespBeacon *

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * struct ml_sch_partner_info - Partner link information
 * @vdev_id: Vdev id
 * @beacon_interval: Beacon interval
 * @bcn_csa_cnt_ofst: CSA swith count offset in beacon frame
 * @bcn_ext_csa_cnt_ofst: ECSA switch count offset in beacon frame
 * @link_info_sta_prof_ofst: offset sta profile in link info.
 *                           If per sta profile exists, this value is non zero
 * @prb_csa_cnt_ofst: CSA swith count offset in probe frame
 * @prb_ext_csa_cnt_ofst: ECSA switch count offset in probe frame
 * @csa_ext_csa_exist: csa or ext csa exists
 */
struct ml_sch_partner_info {
	uint32_t vdev_id;
	uint32_t beacon_interval;
	uint32_t bcn_csa_cnt_ofst;
	uint32_t bcn_ext_csa_cnt_ofst;
	uint16_t link_info_sta_prof_ofst;
	uint32_t prb_csa_cnt_ofst;
	uint32_t prb_ext_csa_cnt_ofst;
	bool csa_ext_csa_exist;
};

/**
 * struct mlo_sch_partner_links - ML partner links
 * @num_links: Number of links
 * @mlo_ie_link_info_ofst: offset of link info in mlo IE
 * @partner_info: Partner link info
 */
struct mlo_sch_partner_links {
	uint8_t num_links;
	uint16_t mlo_ie_link_info_ofst;
	struct ml_sch_partner_info partner_info[WLAN_UMAC_MLO_MAX_VDEVS];
};
#endif

/**
 * struct sch_context - SCH global context
 * @beacon_interval: global beacon interval
 * @beacon_changed: flag to indicate that beacon template has been updated
 * @p2p_ie_offset: P2P IE offset
 * @csa_count_offset: CSA Switch Count Offset to be sent to FW
 * @ecsa_count_offset: ECSA Switch Count Offset to be sent to FW
 */
struct sch_context {
	uint16_t beacon_interval;
	uint8_t beacon_changed;
	uint16_t p2p_ie_offset;
	uint32_t csa_count_offset;
	uint32_t ecsa_count_offset;
#ifdef WLAN_FEATURE_11BE_MLO
	struct mlo_sch_partner_links sch_mlo_partner;
#endif
};

#endif
