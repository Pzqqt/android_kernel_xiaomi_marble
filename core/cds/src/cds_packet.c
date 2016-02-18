/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

/**=========================================================================

   \file        cds_packet.c

   \brief       Connectivity driver services (CDS) network Packet APIs

   Network Protocol packet/buffer support interfaces

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <cds_packet.h>
#include <i_cds_packet.h>
#include <cdf_mc_timer.h>
#include <cdf_trace.h>
#include <wlan_hdd_main.h>
#include "cdf_nbuf.h"
#include "cdf_memory.h"

#define TX_PKT_MIN_HEADROOM          (64)

/* Protocol specific packet tracking feature */
#define CDS_PKT_TRAC_ETH_TYPE_OFFSET (12)
#define CDS_PKT_TRAC_IP_OFFSET       (14)
#define CDS_PKT_TRAC_IP_HEADER_SIZE  (20)
#define CDS_PKT_TRAC_DHCP_SRV_PORT   (67)
#define CDS_PKT_TRAC_DHCP_CLI_PORT   (68)
#define CDS_PKT_TRAC_EAPOL_ETH_TYPE  (0x888E)
#ifdef QCA_PKT_PROTO_TRACE
#define CDS_PKT_TRAC_MAX_STRING_LEN  (12)
#define CDS_PKT_TRAC_MAX_TRACE_BUF   (50)
#define CDS_PKT_TRAC_MAX_STRING_BUF  (64)

/* protocol Storage Structure */
typedef struct {
	uint32_t order;
	v_TIME_t event_time;
	char event_string[CDS_PKT_TRAC_MAX_STRING_LEN];
} cds_pkt_proto_trace_t;

cds_pkt_proto_trace_t *trace_buffer = NULL;
unsigned int trace_buffer_order = 0;
cdf_spinlock_t trace_buffer_lock;
#endif /* QCA_PKT_PROTO_TRACE */

/**
 * cds_pkt_return_packet  Free the cds Packet
 * @ cds Packet
 */
QDF_STATUS cds_pkt_return_packet(cds_pkt_t *packet)
{
	/* Validate the input parameter pointer */
	if (unlikely(packet == NULL)) {
		return QDF_STATUS_E_INVAL;
	}

	/* Free up the Adf nbuf */
	cdf_nbuf_free(packet->pkt_buf);

	packet->pkt_buf = NULL;

	/* Free up the Rx packet */
	cdf_mem_free(packet);

	return QDF_STATUS_SUCCESS;
}

/**--------------------------------------------------------------------------

   \brief cds_pkt_get_packet_length() - Get packet length for a cds Packet

   This API returns the total length of the data in a cds Packet.

   \param pPacket - the cds Packet to get the packet length from.

   \param pPacketSize - location to return the total size of the data contained
   in the cds Packet.
   \return

   \sa

   ---------------------------------------------------------------------------*/
QDF_STATUS
cds_pkt_get_packet_length(cds_pkt_t *pPacket, uint16_t *pPacketSize)
{
	/* Validate the parameter pointers */
	if (unlikely((pPacket == NULL) || (pPacketSize == NULL)) ||
	    (pPacket->pkt_buf == NULL)) {
		CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_FATAL,
			  "VPKT [%d]: NULL pointer", __LINE__);
		return QDF_STATUS_E_INVAL;
	}
	/* return the requested information */
	*pPacketSize = cdf_nbuf_len(pPacket->pkt_buf);
	return QDF_STATUS_SUCCESS;
}

/*---------------------------------------------------------------------------
* @brief cds_pkt_get_proto_type() -
      Find protoco type from packet contents

* skb Packet Pointer
* tracking_map packet type want to track
* dot11_type, type of dot11 frame
   ---------------------------------------------------------------------------*/
uint8_t cds_pkt_get_proto_type(struct sk_buff *skb, uint8_t tracking_map,
			       uint8_t dot11_type)
{
	uint8_t pkt_proto_type = 0;
	uint16_t ether_type;
	uint16_t SPort;
	uint16_t DPort;

	if (dot11_type) {
		if (dot11_type ==
		    (CDS_PKT_TRAC_TYPE_MGMT_ACTION & tracking_map))
			pkt_proto_type |= CDS_PKT_TRAC_TYPE_MGMT_ACTION;

		/* Protocol type map */
		return pkt_proto_type;
	}

	/* EAPOL Tracking enabled */
	if (CDS_PKT_TRAC_TYPE_EAPOL & tracking_map) {
		ether_type = (uint16_t) (*(uint16_t *)
					 (skb->data +
					  CDS_PKT_TRAC_ETH_TYPE_OFFSET));
		if (CDS_PKT_TRAC_EAPOL_ETH_TYPE == CDF_SWAP_U16(ether_type)) {
			pkt_proto_type |= CDS_PKT_TRAC_TYPE_EAPOL;
		}
	}

	/* DHCP Tracking enabled */
	if (CDS_PKT_TRAC_TYPE_DHCP & tracking_map) {
		SPort = (uint16_t) (*(uint16_t *)
				    (skb->data + CDS_PKT_TRAC_IP_OFFSET +
				     CDS_PKT_TRAC_IP_HEADER_SIZE));
		DPort = (uint16_t) (*(uint16_t *)
				    (skb->data + CDS_PKT_TRAC_IP_OFFSET +
				     CDS_PKT_TRAC_IP_HEADER_SIZE +
				     sizeof(uint16_t)));
		if (((CDS_PKT_TRAC_DHCP_SRV_PORT == CDF_SWAP_U16(SPort))
		     && (CDS_PKT_TRAC_DHCP_CLI_PORT == CDF_SWAP_U16(DPort)))
		    || ((CDS_PKT_TRAC_DHCP_CLI_PORT == CDF_SWAP_U16(SPort))
			&& (CDS_PKT_TRAC_DHCP_SRV_PORT == CDF_SWAP_U16(DPort)))) {
			pkt_proto_type |= CDS_PKT_TRAC_TYPE_DHCP;
		}
	}

	/* Protocol type map */
	return pkt_proto_type;
}

#ifdef QCA_PKT_PROTO_TRACE
/*---------------------------------------------------------------------------
* @brief cds_pkt_trace_buf_update() -
      Update storage buffer with interest event string

* event_string Event String may packet type or outstanding event
   ---------------------------------------------------------------------------*/
void cds_pkt_trace_buf_update(char *event_string)
{
	uint32_t slot;

	CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_INFO,
		  "%s %d, %s", __func__, __LINE__, event_string);
	cdf_spinlock_acquire(&trace_buffer_lock);
	slot = trace_buffer_order % CDS_PKT_TRAC_MAX_TRACE_BUF;
	trace_buffer[slot].order = trace_buffer_order;
	trace_buffer[slot].event_time = cdf_mc_timer_get_system_time();
	cdf_mem_zero(trace_buffer[slot].event_string,
		     sizeof(trace_buffer[slot].event_string));
	cdf_mem_copy(trace_buffer[slot].event_string,
		     event_string,
		     (CDS_PKT_TRAC_MAX_STRING_LEN < strlen(event_string)) ?
		     CDS_PKT_TRAC_MAX_STRING_LEN : strlen(event_string));
	trace_buffer_order++;
	cdf_spinlock_release(&trace_buffer_lock);

	return;
}

/*---------------------------------------------------------------------------
* @brief cds_pkt_trace_buf_dump() -
      Dump stored information into kernel log
   ---------------------------------------------------------------------------*/
void cds_pkt_trace_buf_dump(void)
{
	uint32_t slot, idx;

	cdf_spinlock_acquire(&trace_buffer_lock);
	CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
		  "PACKET TRACE DUMP START Current Timestamp %u",
		  (unsigned int)cdf_mc_timer_get_system_time());
	CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
		  "ORDER :        TIME : EVT");
	if (CDS_PKT_TRAC_MAX_TRACE_BUF > trace_buffer_order) {
		for (slot = 0; slot < trace_buffer_order; slot++) {
			CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
				  "%5d :%12u : %s",
				  trace_buffer[slot].order,
				  (unsigned int)trace_buffer[slot].event_time,
				  trace_buffer[slot].event_string);
		}
	} else {
		for (idx = 0; idx < CDS_PKT_TRAC_MAX_TRACE_BUF; idx++) {
			slot =
				(trace_buffer_order +
				 idx) % CDS_PKT_TRAC_MAX_TRACE_BUF;
			CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
				  "%5d :%12u : %s", trace_buffer[slot].order,
				  (unsigned int)trace_buffer[slot].event_time,
				  trace_buffer[slot].event_string);
		}
	}

	CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
		  "PACKET TRACE DUMP END");
	cdf_spinlock_release(&trace_buffer_lock);

	return;
}

/*---------------------------------------------------------------------------
* @brief cds_pkt_proto_trace_init() -
      Initialize protocol trace functionality, allocate required resource
   ---------------------------------------------------------------------------*/
void cds_pkt_proto_trace_init(void)
{
	/* Init spin lock to protect global memory */
	cdf_spinlock_init(&trace_buffer_lock);
	trace_buffer_order = 0;

	trace_buffer = cdf_mem_malloc(CDS_PKT_TRAC_MAX_TRACE_BUF *
				      sizeof(cds_pkt_proto_trace_t));

	/* Register callback function to NBUF
	 * Lower layer event also will be reported to here */
	cdf_nbuf_reg_trace_cb(cds_pkt_trace_buf_update);
	return;
}

/*---------------------------------------------------------------------------
* @brief cds_pkt_proto_trace_close() -
      Free required resource
   ---------------------------------------------------------------------------*/
void cds_pkt_proto_trace_close(void)
{
	CDF_TRACE(CDF_MODULE_ID_CDF, CDF_TRACE_LEVEL_ERROR,
		  "%s %d", __func__, __LINE__);
	cdf_mem_free(trace_buffer);
	cdf_spinlock_destroy(&trace_buffer_lock);

	return;
}
#endif /* QCA_PKT_PROTO_TRACE */

#ifdef MEMORY_DEBUG
/*---------------------------------------------------------------------------
* @brief cds_packet_alloc_debug() -
      Allocate a network buffer for TX
   ---------------------------------------------------------------------------*/
QDF_STATUS cds_packet_alloc_debug(uint16_t size, void **data, void **ppPacket,
				  uint8_t *file_name, uint32_t line_num)
{
	QDF_STATUS cdf_ret_status = QDF_STATUS_E_FAILURE;
	cdf_nbuf_t nbuf;

	nbuf =
		cdf_nbuf_alloc_debug(NULL, roundup(size + TX_PKT_MIN_HEADROOM, 4),
				     TX_PKT_MIN_HEADROOM, sizeof(uint32_t), false,
				     file_name, line_num);

	if (nbuf != NULL) {
		cdf_nbuf_put_tail(nbuf, size);
		cdf_nbuf_set_protocol(nbuf, ETH_P_CONTROL);
		*ppPacket = nbuf;
		*data = cdf_nbuf_data(nbuf);
		cdf_ret_status = QDF_STATUS_SUCCESS;
	}

	return cdf_ret_status;
}
#else
/*---------------------------------------------------------------------------
* @brief cds_packet_alloc() -
      Allocate a network buffer for TX
   ---------------------------------------------------------------------------*/
QDF_STATUS cds_packet_alloc(uint16_t size, void **data, void **ppPacket)
{
	QDF_STATUS cdf_ret_status = QDF_STATUS_E_FAILURE;
	cdf_nbuf_t nbuf;

	nbuf = cdf_nbuf_alloc(NULL, roundup(size + TX_PKT_MIN_HEADROOM, 4),
			      TX_PKT_MIN_HEADROOM, sizeof(uint32_t), false);

	if (nbuf != NULL) {
		cdf_nbuf_put_tail(nbuf, size);
		cdf_nbuf_set_protocol(nbuf, ETH_P_CONTROL);
		*ppPacket = nbuf;
		*data = cdf_nbuf_data(nbuf);
		cdf_ret_status = QDF_STATUS_SUCCESS;
	}

	return cdf_ret_status;
}

#endif
/*---------------------------------------------------------------------------
* @brief cds_packet_free() -
      Free input network buffer
   ---------------------------------------------------------------------------*/
void cds_packet_free(void *pPacket)
{
	cdf_nbuf_free((cdf_nbuf_t) pPacket);
}
