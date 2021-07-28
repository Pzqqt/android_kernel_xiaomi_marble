/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef __WMA_EHT_H
#define __WMA_EHT_H

#include "wma.h"

#ifdef WLAN_FEATURE_11BE
/*
 * wma_eht_update_tgt_services() - update tgt cfg to indicate 11be support
 * @wmi_handle: pointer to WMI handle
 * @cfg: pointer to WMA target services
 *
 * Based on WMI SERVICES information, enable 11be support and set DOT11BE
 * bit in feature caps bitmap.
 *
 * Return: None
 */
void wma_eht_update_tgt_services(struct wmi_unified *wmi_handle,
				 struct wma_tgt_services *cfg);
/**
 * wma_update_target_ext_eht_cap() - Update EHT caps with given extended cap
 * @tgt_hdl: target psoc information
 * @tgt_cfg: Target config
 *
 * This function loop through each hardware mode and for each hardware mode
 * again it loop through each MAC/PHY and pull the caps 2G and 5G specific
 * EHT caps and derives the final cap.
 *
 * Return: None
 */
void wma_update_target_ext_eht_cap(struct target_psoc_info *tgt_hdl,
				   struct wma_tgt_cfg *tgt_cfg);

void wma_update_vdev_eht_ops(uint32_t *eht_ops, tDot11fIEeht_op *eht_op);

/**
 * wma_print_eht_cap() - Print EHT capabilities
 * @eht_cap: pointer to EHT Capability
 *
 * Received EHT capabilities are converted into dot11f structure.
 * This function will print all the EHT capabilities as stored
 * in the dot11f structure.
 *
 * Return: None
 */
void wma_print_eht_cap(tDot11fIEeht_cap *eht_cap);

/**
 * wma_print_eht_phy_cap() - Print EHT PHY Capability
 * @phy_cap: pointer to PHY Capability
 *
 * This function prints EHT PHY Capability received from FW.
 *
 * Return: none
 */
void wma_print_eht_phy_cap(uint32_t *phy_cap);

/**
 * wma_print_eht_mac_cap() - Print EHT MAC Capability
 * @mac_cap: pointer to MAC Capability
 *
 * This function prints EHT MAC Capability received from FW.
 *
 * Return: none
 */
void wma_print_eht_mac_cap(uint32_t *mac_cap);

/**
 * wma_print_eht_op() - Print EHT Operation
 * @eht_cap: pointer to EHT Operation
 *
 * Print EHT operation stored as dot11f structure
 *
 * Return: None
 */
void wma_print_eht_op(tDot11fIEeht_op *eht_ops);

/**
 * wma_populate_peer_eht_cap() - populate peer EHT capabilities in
 *                               peer assoc cmd
 * @peer: pointer to peer assoc params
 * @params: pointer to ADD STA params
 *
 * Return: None
 */
void wma_populate_peer_eht_cap(struct peer_assoc_params *peer,
			       tpAddStaParams params);

/**
 * wma_vdev_set_eht_bss_params() - set EHT OPs in vdev start
 * @wma: pointer to wma handle
 * @vdev_id: VDEV id
 * @eht_info: pointer to eht info
 *
 * Return: None
 */
void wma_vdev_set_eht_bss_params(tp_wma_handle wma, uint8_t vdev_id,
				 struct vdev_mlme_eht_ops_info *eht_info);

/**
 * wma_is_peer_eht_capable() - whether peer is eht capable or not
 * @params: add sta params
 *
 * Return: true if eht capable is present
 */
static inline bool wma_is_peer_eht_capable(tpAddStaParams params)
{
	return params->eht_capable;
}

/**
 * wma_get_eht_capabilities() - Get EHT capabilities from WMA
 * @eht_cap: Pointer to EHT capabilities
 *
 * Currently EHT capabilities are not updated in wma_handle. This
 * is an interface for upper layer to query capabilities from WMA.
 * When the real use case arise, update wma_handle with EHT capabilities
 * as required.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wma_get_eht_capabilities(struct eht_capability *eht_cap);

/**
 * wma_set_peer_assoc_params_bw_320() - Set bw_320 based on ch_width
 * @params: pointer to peer assoc params
 * @ch_width: enum phy_ch_width
 *
 * If ch_width is CH_WIDTH_320MHZ, set params->bw_320 to 1
 *
 * Return: None
 */
void wma_set_peer_assoc_params_bw_320(struct peer_assoc_params *params,
				      enum phy_ch_width ch_width);
#else
static inline void wma_eht_update_tgt_services(struct wmi_unified *wmi_handle,
					       struct wma_tgt_services *cfg)
{
		cfg->en_11be = false;
		return;
}

static inline
void wma_update_target_ext_eht_cap(struct target_psoc_info *tgt_hdl,
				   struct wma_tgt_cfg *tgt_cfg)
{
}

static inline
void wma_update_vdev_eht_ops(uint32_t *eht_ops, tDot11fIEeht_op *eht_op)
{
}

static inline
void wma_print_eht_cap(tDot11fIEeht_cap *eht_cap)
{
}

static inline
void wma_print_eht_phy_cap(uint32_t *phy_cap)
{
}

static inline
void wma_print_eht_mac_cap(uint32_t *mac_cap)
{
}

static inline
void wma_print_eht_op(tDot11fIEeht_op *eht_ops)
{
}

static inline
void wma_populate_peer_eht_cap(struct peer_assoc_params *peer,
			       tpAddStaParams params)
{
}

static inline bool wma_is_peer_eht_capable(tpAddStaParams params)
{
	return false;
}

static inline
void wma_set_peer_assoc_params_bw_320(struct peer_assoc_params *params,
				      enum phy_ch_width ch_width)
{
}
#endif
#endif
