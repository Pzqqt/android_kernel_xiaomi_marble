/*
 * Copyright (c) 2011-2014, 2016, 2018 The Linux Foundation. All rights reserved.
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
 * This file lim_prop_exts_utils.h contains the definitions
 * used by all LIM modules to support proprietary features.
 * Author:        Chandra Modumudi
 * Date:          12/11/02
 * History:-
 * Date           Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#ifndef __LIM_PROP_EXTS_UTILS_H
#define __LIM_PROP_EXTS_UTILS_H

/* Function templates */
void limQuietBss(struct mac_context *, uint32_t);
void lim_cleanupMeasData(struct mac_context *);
void limDeleteMeasTimers(struct mac_context *);
void limStopMeasTimers(struct mac_context *mac);
void lim_cleanupMeasResources(struct mac_context *);
void limRestorePreLearnState(struct mac_context *);
void limCollectMeasurementData(struct mac_context *, uint32_t *, tpSchBeaconStruct);
void limCollectRSSI(struct mac_context *);
void limDeleteCurrentBssWdsNode(struct mac_context *);
uint32_t limComputeAvg(struct mac_context *, uint32_t, uint32_t);

/* / Function to extract AP's HCF capability from IE fields */
void lim_extract_ap_capability(struct mac_context *, uint8_t *, uint16_t, uint8_t *,
			       uint16_t *, uint8_t *, int8_t *, struct pe_session *);

ePhyChanBondState lim_get_htcb_state(ePhyChanBondState aniCBMode);

#endif /* __LIM_PROP_EXTS_UTILS_H */
