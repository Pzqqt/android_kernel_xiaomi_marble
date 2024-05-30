/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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

#ifndef _HAL_9224_RX_H_
#define _HAL_9224_RX_H_
#include "qdf_util.h"
#include "qdf_types.h"
#include "qdf_lock.h"
#include "qdf_mem.h"
#include "qdf_nbuf.h"
#include "tcl_data_cmd.h"
#include "phyrx_rssi_legacy.h"
#include "rx_msdu_start.h"
#include "tlv_tag_def.h"
#include "hal_hw_headers.h"
#include "hal_internal.h"
#include "cdp_txrx_mon_struct.h"
#include "qdf_trace.h"
#include "hal_rx.h"
#include "hal_tx.h"
#include "dp_types.h"
#include "hal_api_mon.h"
#include "phyrx_other_receive_info_ru_details.h"

#define HAL_RX_MSDU0_BUFFER_ADDR_LSB(link_desc_va)	\
	(uint8_t *)(link_desc_va) +			\
	RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_OFFSET

#define HAL_RX_MSDU_DESC_INFO_PTR_GET(msdu0)			\
	(uint8_t *)(msdu0) +				\
	RX_MSDU_DETAILS_RX_MSDU_DESC_INFO_DETAILS_FIRST_MSDU_IN_MPDU_FLAG_OFFSET

#define HAL_ENT_MPDU_DESC_INFO(ent_ring_desc)		\
	(uint8_t *)(ent_ring_desc) +			\
	RX_MPDU_DETAILS_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET

#define HAL_DST_MPDU_DESC_INFO(dst_ring_desc)		\
	(uint8_t *)(dst_ring_desc) +			\
	REO_DESTINATION_RING_RX_MPDU_DESC_INFO_DETAILS_MSDU_COUNT_OFFSET

#define HAL_RX_GET_MAC_ADDR1_VALID(rx_mpdu_start) \
	HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO, MAC_ADDR_AD1_VALID)

#define HAL_RX_GET_SW_FRAME_GROUP_ID(rx_mpdu_start)	\
	HAL_RX_GET(rx_mpdu_start, RX_MPDU_INFO, SW_FRAME_GROUP_ID)

#define HAL_REO_R0_CONFIG(soc, reg_val, reo_params)		\
	do {							\
		reg_val &=					\
			~(HWIO_REO_R0_GENERAL_ENABLE_AGING_LIST_ENABLE_BMSK |\
			HWIO_REO_R0_GENERAL_ENABLE_AGING_FLUSH_ENABLE_BMSK); \
		reg_val |=					\
			HAL_SM(HWIO_REO_R0_GENERAL_ENABLE,	\
			       AGING_LIST_ENABLE, 1) |		\
			HAL_SM(HWIO_REO_R0_GENERAL_ENABLE,	\
			       AGING_FLUSH_ENABLE, 1);		\
		HAL_REG_WRITE(soc,				\
			      HWIO_REO_R0_GENERAL_ENABLE_ADDR(	\
				REO_REG_REG_BASE),		\
			      reg_val);				\
		reg_val = HAL_REG_READ(soc,			\
				       HWIO_REO_R0_MISC_CTL_ADDR(	\
					       REO_REG_REG_BASE));	\
		reg_val &= ~(HWIO_REO_R0_MISC_CTL_FRAGMENT_DEST_RING_BMSK); \
		reg_val |= HAL_SM(HWIO_REO_R0_MISC_CTL,		\
				  FRAGMENT_DEST_RING,		\
				  (reo_params)->frag_dst_ring); \
		reg_val &= (~HWIO_REO_R0_MISC_CTL_BAR_DEST_RING_BMSK | \
			    (REO_REMAP_TCL <<			\
			     HWIO_REO_R0_MISC_CTL_BAR_DEST_RING_SHFT)); \
		HAL_REG_WRITE(soc,				\
			      HWIO_REO_R0_MISC_CTL_ADDR(REO_REG_REG_BASE), \
			      reg_val); \
	} while (0)

#define HAL_RX_MSDU_DESC_INFO_GET(msdu_details_ptr) \
	((struct rx_msdu_desc_info *) \
	_OFFSET_TO_BYTE_PTR(msdu_details_ptr, \
RX_MSDU_DETAILS_RX_MSDU_DESC_INFO_DETAILS_RESERVED_0A_OFFSET))

#define HAL_RX_LINK_DESC_MSDU0_PTR(link_desc)   \
	((struct rx_msdu_details *) \
	 _OFFSET_TO_BYTE_PTR((link_desc),\
	RX_MSDU_LINK_MSDU_0_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_OFFSET))

#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_CHAN_CAPTURE_STATUS_BMASK 0x00000006
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_CHAN_CAPTURE_STATUS_LSB 1
#define PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_CHAN_CAPTURE_STATUS_MSB 2

#define HAL_GET_RX_LOCATION_INFO_CHAN_CAPTURE_STATUS(rx_tlv) \
	((HAL_RX_GET((rx_tlv), \
		     PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS, \
		     RTT_CFR_STATUS) & \
	  PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_CHAN_CAPTURE_STATUS_BMASK) >> \
	 PHYRX_LOCATION_RX_LOCATION_INFO_DETAILS_CHAN_CAPTURE_STATUS_LSB)
#endif
#endif
