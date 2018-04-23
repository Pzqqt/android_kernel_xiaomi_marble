/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

#if !defined(__CDS_PKT_H)
#define __CDS_PKT_H

/**=========================================================================

   \file        cds_packet.h

   \brief       Connectivity driver services (CDS) network Packet APIs

   Network Protocol packet/buffer support interfaces

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <qdf_types.h>
#include <qdf_status.h>

/*--------------------------------------------------------------------------
   Preprocessor definitions and constants
   ------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
   Type declarations
   ------------------------------------------------------------------------*/
struct cds_pkt_t;
typedef struct cds_pkt_t cds_pkt_t;

#include "qdf_nbuf.h"

/**
 * cds_pkt_return_packet  Free the cds Packet
 * @ cds Packet
 */
QDF_STATUS cds_pkt_return_packet(cds_pkt_t *packet);

/**
 * cds_pkt_get_packet_length  Returns the packet length
 * @ cds Packet
 */
QDF_STATUS cds_pkt_get_packet_length(cds_pkt_t *pPacket,
				     uint16_t *pPacketSize);

/*
 * TODO: Remove later
 * All the below difinitions are not
 * required for Host Driver 2.0
 * once corresponding references are removed
 * from HDD and other layers
 * below code will be removed
 */
/* The size of AMSDU frame per spec can be a max of 3839 bytes
   in BD/PDUs that means 30 (one BD = 128 bytes)
   we must add the size of the 802.11 header to that */
#define CDS_PKT_SIZE_BUFFER  ((30 * 128) + 32)

/* cds Packet Types */
typedef enum {
	/* cds Packet is used to transmit 802.11 Management frames. */
	CDS_PKT_TYPE_TX_802_11_MGMT,

	/* cds Packet is used to transmit 802.11 Data frames. */
	CDS_PKT_TYPE_TX_802_11_DATA,

	/* cds Packet is used to transmit 802.3 Data frames. */
	CDS_PKT_TYPE_TX_802_3_DATA,

	/* cds Packet contains Received data of an unknown frame type */
	CDS_PKT_TYPE_RX_RAW,

	/* Invalid sentinel value */
	CDS_PKT_TYPE_MAXIMUM
} CDS_PKT_TYPE;

/* user IDs.   These IDs are needed on the cds_pkt_get/set_user_data_ptr()
   to identify the user area in the cds Packet. */
typedef enum {
	CDS_PKT_USER_DATA_ID_TL = 0,
	CDS_PKT_USER_DATA_ID_BAL,
	CDS_PKT_USER_DATA_ID_WMA,
	CDS_PKT_USER_DATA_ID_HDD,
	CDS_PKT_USER_DATA_ID_BSL,

	CDS_PKT_USER_DATA_ID_MAX
} CDS_PKT_USER_DATA_ID;

#ifdef MEMORY_DEBUG
#define cds_packet_alloc(s, d, p)	\
	cds_packet_alloc_debug(s, d, p, __FILE__, __LINE__)

QDF_STATUS cds_packet_alloc_debug(uint16_t size, void **data, void **ppPacket,
				  uint8_t *file_name, uint32_t line_num);
#else
QDF_STATUS cds_packet_alloc(uint16_t size, void **data, void **ppPacket);
#endif

void cds_packet_free(void *pPacket);

typedef QDF_STATUS (*cds_pkt_get_packet_callback)(cds_pkt_t *pPacket,
						  void *userData);

#endif /* !defined( __CDS_PKT_H ) */
