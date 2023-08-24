/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *  * Neither the name of The Linux Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include "TestsUtils.h"
#include "InterfaceAbstraction.h"
#include "Constants.h"
#include "Pipe.h"

using namespace std;
///////////////////////////////////////////////////////////////////////////////

extern Logger g_Logger;

static uint8_t IPv4Packet[IP4_PACKET_SIZE] = {
		0x45, 0x00, 0x00, 0x2e,
		0x00, 0x00, 0x40, 0x00,
		0xff, 0x06, 0xf5, 0xfd,// Protocol = 06 (TCP)
		0xc0, 0xa8, 0x02, 0x13,// IPv4 SRC Addr 192.168.2.19
		0xc0, 0xa8, 0x02, 0x68,// IPv4 DST Addr 192.168.2.104
		0x04, 0x57, 0x08, 0xae,
		0x00, 0x00, 0x30, 0x34,
		0x00, 0x00, 0x00, 0x50,
		0x50, 0xc1, 0x40, 0x00,
		0xab, 0xc9, 0x00, 0x00,
		0x00, 0xaa, 0xaa, 0xaa,
		0xbb, 0xbb, 0xbb, 0xbb,
		0xbb, 0xbb, 0xbb, 0xbb,
		0xbb, 0xbb, 0xbb, 0xbb,
		0xbb, 0xbb, 0xbb, 0xbb,
		0xbb, 0xbb, 0xbb, 0xbb,
		0xbb, 0xbb, 0xbb, 0xbb,
		0xbb, 0xbb
};

static uint8_t IPv6Packet[] = {
		0x60, 0x00, 0x00, 0x00,
		0x00, 0x1c, 0x06, 0x01, // Protocol = 6 (TCP)
		0xfe, 0x80, 0x00, 0x00, // source address (16B)
		0x00, 0x00, 0x00, 0x00,
		0xd9, 0xf9, 0xce, 0x5e,
		0x02, 0xec, 0x32, 0x99,
		0xff, 0x02, 0x00, 0x00, // dst address (16B)
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x0c,
		0x12, 0x34, 0x12, 0x34, // port src = 0x1234 dest = 0x1234
		0x00, 0x14, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x60, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, // options
		0xda, 0x7a, 0xda, 0x7a // payload
};

static uint8_t IPv6PacketFragExtHdr[] = {
		0x60, 0x00, 0x00, 0x00,
		0x00, 0x0c, 0x2C, 0x01, // Next header = FRAGMENT HEADER(44)
		0xfe, 0x80, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0xd9, 0xf9, 0xce, 0x5e,
		0x02, 0xec, 0x32, 0x99,
		0xff, 0x02, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x0c,
		0x06, 0x00, 0x00, 0x00, // fragment header, Protocol = 6 (TCP)
		0x00, 0x00, 0x00, 0x00,
		0x12, 0x34, 0x12, 0x34, // port src = 0x1234 dest = 0x1234
		0x00, 0x14, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x60, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0xda, 0x7a, 0xda, 0x7a  // payload
};

static const uint8_t Eth2IPv4Packet[] =
{
	// ETH2 - 14 bytes
	0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x11, // ETH2 DST
	0x22, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, // ETH2 SRC
	0x08, 0x00,		// ETH2 TYPE IPv4 - ETH_P_IP 0x0800

	// IPv4
	0x45, 0x00, 0x00, 0x2e,
	0x00, 0x00, 0x40, 0x00,
	0xff, 0x06, 0xf5, 0xfd, // Protocol = 06 (TCP)
	0xc0, 0xa8, 0x02, 0x13, // IPv4 SRC Addr 192.168.2.19
	0xc0, 0xa8, 0x02, 0x68, // IPv4 DST Addr 192.168.2.104
	0x04, 0x57, 0x08, 0xae,
	0x00, 0x00, 0x30, 0x34,
	0x00, 0x00, 0x00, 0x50,
	0x50, 0xc1, 0x40, 0x00,
	0xab, 0xc9, 0x00, 0x00,
	0x00
};

static const uint8_t Eth2IPv6Packet[] =
{
	// ETH2 - 14 bytes
	0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x11, // ETH2 DST
	0x22, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, // ETH2 SRC
	0x86, 0xdd,		// ETH2 TYPE IPv6 - ETH_P_IPV6 x86DD

	// IPv6
	0x60, 0x00, 0x00, 0x00,
	0x02, 0x12, 0x11, 0x01,
	0xfe, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0xd9, 0xf9, 0xce, 0x5e,
	0x02, 0xec, 0x32, 0x99,
	0xff, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x0c
};

static const uint8_t WLANEth2IPv4Packet[] =
{
	// WLAN
	0xa1, 0xb2, 0xc3, 0xd4,			// WLAN hdr

	// ETH2 - 14 bytes
	0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x11,	// ETH2 DST
	0x22, 0xee, 0xdd, 0xcc, 0xbb, 0xaa,	// ETH2 SRC
	0x08, 0x00,			// ETH2 TYPE IPv4 - ETH_P_IP 0x0800

	// IPv4
	0x45, 0x00, 0x00, 0x2e,
	0x00, 0x00, 0x40, 0x00,
	0xff, 0x06, 0xf5, 0xfd,	// Protocol = 06 (TCP)
	0xc0, 0xa8, 0x02, 0x13,	// IPv4 SRC Addr 192.168.2.19
	0xc0, 0xa8, 0x02, 0x68,	// IPv4 DST Addr 192.168.2.104
	0x04, 0x57, 0x08, 0xae,
	0x00, 0x00, 0x30, 0x34,
	0x00, 0x00, 0x00, 0x50,
	0x50, 0xc1, 0x40, 0x00,
	0xab, 0xc9, 0x00, 0x00,
	0x00
};

static const uint8_t WLAN802_3IPv4Packet[] =
{
	// WLAN
	0xa1, 0xb2, 0xc3, 0xd4,			// WLAN hdr

	// 802_3 - 26 bytes
	0xa5, 0xb6, 0xc7, 0xd8, // ROME proprietary header
	0xa0, 0xb1, 0xc2, 0xd3, 0xe4, 0x33,	// 802_3 DST
	0x44, 0xb2, 0xc3, 0xd4, 0xe5, 0xf6,	// 802_3 SRC
	0x00, IP4_PACKET_SIZE,			// 802_3 length
	0x04, 0x15, 0x26, 0x37, 0x48, 0x59,	// LLC/SNAP
	0x08, 0x00,				// Ethrtype - 0x0800

	// IPv4
	0x45, 0x00, 0x00, 0x2e,
	0x00, 0x00, 0x40, 0x00,
	0xff, 0x06, 0xf5, 0xfd,	// Protocol = 06 (TCP)
	0xc0, 0xa8, 0x02, 0x13,	// IPv4 SRC Addr 192.168.2.19
	0xc0, 0xa8, 0x02, 0x68,	// IPv4 DST Addr 192.168.2.104
	0x04, 0x57, 0x08, 0xae,
	0x00, 0x00, 0x30, 0x34,
	0x00, 0x00, 0x00, 0x50,
	0x50, 0xc1, 0x40, 0x00,
	0xab, 0xc9, 0x00, 0x00,
	0x00
};

static uint8_t IPv4_TCP_No_Payload_Packet[] = {
		0x45, 0x00, 0x00, 0x28, // Total length is 40
		0x00, 0x00, 0x40, 0x00,
		0xff, 0x06, 0xf5, 0xfd,	// Protocol = 06 (TCP)
		0xc0, 0xa8, 0x02, 0x13,	// IPv4 SRC Addr 192.168.2.19
		0xc0, 0xa8, 0x02, 0xc8,	// IPv4 DST Addr 192.168.2.200
		0x04, 0x57, 0x08, 0xae,
		0x00, 0x00, 0x30, 0x34,
		0x00, 0x00, 0x00, 0x50,
		0x50, 0xc1, 0x40, 0x00,
		0xab, 0xc9, 0x00, 0x00,
};

static uint8_t IPv6_TCP_No_Payload_Packet[] = {
		0x60, 0x00, 0x00, 0x00,
		0x00, 0x14, 0x06, 0x01, // Payload is 20 Bytes (TCP header only)
		0xfe, 0x80, 0x00, 0x00, // SRC Addr
		0x00, 0x00, 0x00, 0x00,
		0xd9, 0xf9, 0xce, 0x5e,
		0x02, 0xec, 0x32, 0x99,
		0xff, 0x02, 0x00, 0x00, // DST Addr
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x4b,
		0x04, 0x57, 0x08, 0xae, // TCP header
		0x00, 0x00, 0x30, 0x34,
		0x00, 0x00, 0x00, 0x50,
		0x50, 0xc1, 0x40, 0x00,
		0xab, 0xc9, 0x00, 0x00,
};

static const uint8_t Eth2IPv4Packet802_1Q_tag[] =
{
	// ETH2 + 8021Q - 14 + 4 bytes
	0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x11, // ETH2 DST
	0x22, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, // ETH2 SRC
	0x81, 0x00, 0x20, 0x03,             // 802.1Q header with PCP = 1 VLAN ID = 3
	0x08, 0x00,                         // ETH2 TYPE IPv4 - ETH_P_IP 0x0800
	// IPv4 payload starts here
	0x45, 0x00, 0x00, 0x29,
	0x00, 0x00, 0x40, 0x00,
	0xff, 0x06, 0xf5, 0xfd,   // Protocol = 06 (TCP)
	0xc0, 0xa8, 0x02, 0x13,   // IPv4 SRC Addr 192.168.2.19
	0xc0, 0xa8, 0x02, 0x68,   // IPv4 DST Addr 192.168.2.104
	0x04, 0x57, 0x08, 0xae,
	0x00, 0x00, 0x30, 0x34,
	0x00, 0x00, 0x00, 0x50,
	0x50, 0xc1, 0x40, 0x00,
	0xab, 0xc9, 0x00, 0x00,
	0x00
};

static const uint8_t Eth2IPv6Packet802_1Q_tag[] =
{
	// ETH2 + 8021Q - 14 + 4 bytes
	0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x11, // ETH2 DST
	0x22, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, // ETH2 SRC
	0x81, 0x00, 0x20, 0x03,             // 802.1Q header with PCP = 1 VLAN ID = 3
	0x86, 0xdd,                         // ETH2 TYPE IPv6 - ETH_P_IP 0x08DD

	// IPv6 payload starts here...
	0x60, 0x0a, 0x41, 0xd5,
	0x00, 0x00, 0x06, 0x40,
	0xfd, 0x53, 0x7c, 0xb8, //SRC Addr: fd53:7cb8:383:4::124
	0x03, 0x83, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x24,
	0xfd, 0x53, 0x7c, 0xb8, //DST Addr: fd53:7cb8:383:4::123
	0x03, 0x83, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x23,
};

static struct ipa_test_config_header *current_configuration = NULL;

///////////////////////////////////////////////////////////////////////////////
/**
*@brief Function loads a default IPv4 / IPv6 Packet
*
*@param [in] eIP - Type of Packet to load (IPA_IP_v4 / IPA_IP_v6)
*@param [in] pBuffer - pointer to the destination buffer
*@param [in,out] nMaxSize - The size of the buffer.*Upon function return,
*	the total number of bytes copied will be stored in this parameter.
*@return boolean indicating whether the operation completed successfully or not.
*@details Function loads a default IPv4 / IPv6 packet into pBuffer.
*/
bool LoadDefaultPacket(
	enum ipa_ip_type eIP,
	enum ipv6_ext_hdr_type extHdrType,
	uint8_t *pBuffer,
	size_t &nMaxSize)
{
	if (IPA_IP_v4 == eIP) {
		if (nMaxSize < sizeof(IPv4Packet))
		{
			LOG_MSG_ERROR("Buffer is smaller than %d, no Data was copied.",sizeof(IPv4Packet));
			return false;
		}
		memcpy(pBuffer,IPv4Packet, sizeof(IPv4Packet));
		nMaxSize = sizeof(IPv4Packet);
		return true;
	} else {
		if (extHdrType == FRAGMENT)
		{
			if (nMaxSize < sizeof(IPv6PacketFragExtHdr))
			{
				LOG_MSG_ERROR("Buffer is smaller than %d, no Data was copied.",sizeof(IPv6PacketFragExtHdr));
				return false;
			}
			memcpy(pBuffer,IPv6PacketFragExtHdr, sizeof(IPv6PacketFragExtHdr));
			nMaxSize = sizeof(IPv6PacketFragExtHdr);

		}
		else
		{
			if (nMaxSize < sizeof(IPv6Packet))
			{
				LOG_MSG_ERROR("Buffer is smaller than %d, no Data was copied.",sizeof(IPv6Packet));
				return false;
			}
			memcpy(pBuffer,IPv6Packet, sizeof(IPv6Packet));
			nMaxSize = sizeof(IPv6Packet);
		}
		return true;
	}
}

bool LoadDefaultPacket(enum ipa_ip_type eIP, uint8_t *pBuffer, size_t &nMaxSize)
{
	return LoadDefaultPacket(eIP, NONE, pBuffer, nMaxSize);
}

bool LoadNoPayloadPacket(enum ipa_ip_type eIP, uint8_t *pBuffer, size_t &nMaxSize)
{
	if (IPA_IP_v4 == eIP) {
		if (nMaxSize < sizeof(IPv4_TCP_No_Payload_Packet))
		{
			LOG_MSG_ERROR("Buffer is smaller than %zu, no Data was copied.",
				sizeof(IPv4_TCP_No_Payload_Packet));
			return false;
		}
		memcpy(pBuffer, IPv4_TCP_No_Payload_Packet, sizeof(IPv4_TCP_No_Payload_Packet));
		nMaxSize = sizeof(IPv4_TCP_No_Payload_Packet);
	} else {
		if (nMaxSize < sizeof(IPv6_TCP_No_Payload_Packet))
		{
			LOG_MSG_ERROR("Buffer is smaller than %zu, no Data was copied.",
				sizeof(IPv6_TCP_No_Payload_Packet));
			return false;
		}
		memcpy(pBuffer, IPv6_TCP_No_Payload_Packet, sizeof(IPv6_TCP_No_Payload_Packet));
		nMaxSize = sizeof(IPv6_TCP_No_Payload_Packet);
	}

	return true;
}

bool LoadDefaultEth2Packet(
	enum ipa_ip_type eIP,
	uint8_t *pBuffer,
	size_t &nMaxSize)
{
	if (IPA_IP_v4 == eIP) {
		if (nMaxSize < sizeof(Eth2IPv4Packet))
		{
			LOG_MSG_ERROR(
				"Buffer is smaller than %d, "
				"no data was copied.",
				sizeof(Eth2IPv4Packet));
			return false;
		}
		memcpy(pBuffer,Eth2IPv4Packet,
			sizeof(Eth2IPv4Packet));
		nMaxSize = sizeof(Eth2IPv4Packet);
	}
	else
	{
		if (nMaxSize < sizeof(Eth2IPv6Packet))
		{
			LOG_MSG_ERROR(
				"Buffer is smaller than %d, "
				"no data was copied.",
				sizeof(Eth2IPv6Packet));
			return false;
		}
		memcpy(pBuffer,Eth2IPv6Packet,
			sizeof(Eth2IPv6Packet));
		nMaxSize = sizeof(Eth2IPv6Packet);
	}

	return true;
}

bool LoadDefaultWLANEth2Packet(
	enum ipa_ip_type eIP,
	uint8_t *pBuffer,
	size_t &nMaxSize)
{
	if (IPA_IP_v4 == eIP) {
		if (nMaxSize < sizeof(WLANEth2IPv4Packet))
		{
			LOG_MSG_ERROR(
				"Buffer is smaller than %d, "
				"no data was copied.",
				sizeof(WLANEth2IPv4Packet));
			return false;
		}
		memcpy(pBuffer,WLANEth2IPv4Packet,
			sizeof(WLANEth2IPv4Packet));
		nMaxSize = sizeof(WLANEth2IPv4Packet);
	}
	else
	{
		LOG_MSG_ERROR("%s isn't implemented "
			"for IPv6 - do it yourself :-)",
			__FUNCTION__);
		return false;
	}

	return true;
}

bool LoadDefaultWLAN802_32Packet(
	enum ipa_ip_type eIP,
	uint8_t *pBuffer,
	size_t &nMaxSize)
{
	if (IPA_IP_v4 == eIP) {
		if (nMaxSize < sizeof(WLAN802_3IPv4Packet))
		{
			LOG_MSG_ERROR(
				"Buffer is smaller than %d, "
				"no data was copied.",
				sizeof(WLAN802_3IPv4Packet));
			return false;
		}
		memcpy(pBuffer,WLAN802_3IPv4Packet,
			sizeof(WLAN802_3IPv4Packet));
		nMaxSize = sizeof(WLAN802_3IPv4Packet);
	}
	else
	{
		LOG_MSG_ERROR("%s isn't implemented"
			" for IPv6 - do it yourself :-)",
			__FUNCTION__);
		return false;
	}

	return true;
}

bool LoadDefault802_1Q(
	enum ipa_ip_type eIP,
	uint8_t *pBuffer,
	size_t &nMaxSize)
{
	const uint8_t* frame_ptr =
		(eIP == IPA_IP_v4) ? Eth2IPv4Packet802_1Q_tag : Eth2IPv6Packet802_1Q_tag;

	size_t frame_len =
		(eIP == IPA_IP_v4) ? sizeof(Eth2IPv4Packet802_1Q_tag) : sizeof(Eth2IPv6Packet802_1Q_tag);

	if (nMaxSize < frame_len) {
		LOG_MSG_ERROR(
			"Buffer is smaller than %d, "
			"no data was copied.",
			frame_len);
		return false;
	}

	memcpy(pBuffer, frame_ptr, frame_len);

	nMaxSize = frame_len;

	return true;
}

bool SendReceiveAndCompare(InterfaceAbstraction *pSink, uint8_t* pSendBuffer, size_t nSendBuffSize,
		InterfaceAbstraction *pSource, uint8_t* pExpectedBuffer, size_t nExpectedBuffSize)
{
	LOG_MSG_STACK("Entering Function");
	bool bRetVal = true;
	uint8_t * pRxBuff = new uint8_t[2*(nExpectedBuffSize+1)];
	size_t nReceivedBuffSize = 0;
	size_t j;

	// Send buffer to pSink
	bRetVal = pSink->SendData((Byte *) pSendBuffer, nSendBuffSize);
	if (!bRetVal)
	{
		LOG_MSG_ERROR("SendData (pOutputBuffer=0x%p) failed",pSendBuffer);
		goto bail;
	}

	// Receive buffer from pSource
	if (NULL == pRxBuff)
	{
		LOG_MSG_ERROR("Failed to allocated pRxBuff[%d]",2*(nExpectedBuffSize+1));
		goto bail;
	}
	nReceivedBuffSize = pSource->ReceiveData(pRxBuff, 2*(nExpectedBuffSize+1)); // We cannot overflow here.
	LOG_MSG_INFO("Received %d bytes on %s.", nReceivedBuffSize, pSource->m_fromChannelName.c_str());
	if (0 > nReceivedBuffSize)
	{
		bRetVal = false;
		goto bail;
	}

	{// Logging Expected and Received buffers
		char aExpectedBufferStr[3*nExpectedBuffSize+1];
		char aRecievedBufferStr[3*nReceivedBuffSize+1];
		memset(aExpectedBufferStr,0,3*nExpectedBuffSize+1);
		memset(aRecievedBufferStr,0,3*nReceivedBuffSize+1);

		for(j = 0; j < nExpectedBuffSize; j++)
			snprintf(&aExpectedBufferStr[3*j], 3*nExpectedBuffSize+1 - (3*j + 1), " %02X", pExpectedBuffer[j]);
		for(j = 0; j < nReceivedBuffSize; j++)
			snprintf(&aRecievedBufferStr[3*j], 3*nReceivedBuffSize+1 - (3*j + 1), " %02X", pRxBuff[j]);
		LOG_MSG_INFO("\nExpected Value (%d)\n%s\n, Received Value(%d)\n%s\n",nExpectedBuffSize,aExpectedBufferStr,nReceivedBuffSize,aRecievedBufferStr);
	}

	//Comparing Expected and received sizes
	if (nExpectedBuffSize != nReceivedBuffSize)
	{
		LOG_MSG_INFO("Buffers' Size differ: expected(%d), Received(%d)",nExpectedBuffSize,nReceivedBuffSize);
		bRetVal = false;
		goto bail;
	}

	bRetVal = !memcmp((void*)pRxBuff, (void*)pExpectedBuffer, nExpectedBuffSize);
	LOG_MSG_INFO("Buffers %s ",bRetVal?"MATCH":"MISMATCH");


	LOG_MSG_INFO("Verify that pipe is Empty");
	nReceivedBuffSize = pSource->ReceiveData(pRxBuff, 2*(nExpectedBuffSize+1)); // We cannot overflow here.
	while (nReceivedBuffSize){
		char aRecievedBufferStr[3*nReceivedBuffSize+1];
		bRetVal = false;
		LOG_MSG_ERROR("More Data in Pipe!\nReceived %d bytes on %s.", nReceivedBuffSize, pSource->m_fromChannelName.c_str());
		memset(aRecievedBufferStr,0,3*nReceivedBuffSize+1);
		for(j = 0; j < nReceivedBuffSize; j++) {
			snprintf(&aRecievedBufferStr[3*j], 3*nReceivedBuffSize+1 - (3*j + 1), " %02X", pRxBuff[j]);
		}
		LOG_MSG_ERROR("\nReceived Value(%d)\n%s\n",nReceivedBuffSize,aRecievedBufferStr);
		nReceivedBuffSize = pSource->ReceiveData(pRxBuff, 2*(nExpectedBuffSize+1)); // We cannot overflow here.
	}

bail:
	delete (pRxBuff);
	LOG_MSG_STACK("Leaving Function (Returning %s)",bRetVal?"True":"False");
	return bRetVal;
}

bool CreateBypassRoutingTable (RoutingDriverWrapper * pRouting,enum ipa_ip_type eIP,
		const char * pTableName, enum ipa_client_type eRuleDestination,
		uint32_t uHeaderHandle, uint32_t * pTableHdl)
{
	bool bRetVal = true;
	struct ipa_ioc_add_rt_rule *pRoutingRule = NULL;
	struct ipa_rt_rule_add *pRoutingRuleEntry = NULL;
	struct ipa_ioc_get_rt_tbl sRoutingTable;

	LOG_MSG_STACK("Entering Function");
	memset(&sRoutingTable,0,sizeof(sRoutingTable));
	pRoutingRule = (struct ipa_ioc_add_rt_rule *)
		calloc(1,
				sizeof(struct ipa_ioc_add_rt_rule) +
		       1*sizeof(struct ipa_rt_rule_add)
			);
	if(!pRoutingRule) {
		LOG_MSG_ERROR("calloc failed to allocate pRoutingRule");
		bRetVal = false;
		goto bail;
	}

	pRoutingRule->num_rules = 1;
	pRoutingRule->ip = ((IPA_IP_v4 == eIP)? IPA_IP_v4 : IPA_IP_v6);
	pRoutingRule->commit = true;
	strlcpy(pRoutingRule->rt_tbl_name, pTableName, sizeof(pRoutingRule->rt_tbl_name));

	pRoutingRuleEntry = &(pRoutingRule->rules[0]);
	pRoutingRuleEntry->at_rear = 1;
	pRoutingRuleEntry->rule.dst = eRuleDestination;// Setting Rule's Destination Pipe
	pRoutingRuleEntry->rule.hdr_hdl = uHeaderHandle; // Header handle
	pRoutingRuleEntry->rule.attrib.attrib_mask = 0;// All Packets will get a "Hit"
	if (false == pRouting->AddRoutingRule(pRoutingRule))
	{
		LOG_MSG_ERROR("Routing rule addition(pRoutingRule) failed!");
		bRetVal = false;
		goto bail;
	}
	if (!pRoutingRuleEntry->rt_rule_hdl)
	{
		LOG_MSG_ERROR("pRoutingRuleEntry->rt_rule_hdl == 0, Routing rule addition(pRoutingRule) failed!");
		bRetVal = false;
		goto bail;
	}
	LOG_MSG_INFO("pRoutingRuleEntry->rt_rule_hdl == 0x%x added to Table %s",pRoutingRuleEntry->rt_rule_hdl,pTableName);
	sRoutingTable.ip = eIP;
	strlcpy(sRoutingTable.name, pTableName, sizeof(sRoutingTable.name));
	if (!pRouting->GetRoutingTable(&sRoutingTable)) {
		LOG_MSG_ERROR(
				"m_routing.GetRoutingTable(&sRoutingTable=0x%p) Failed.", &sRoutingTable);
		bRetVal = false;
		goto bail;
	}
	if(NULL != pTableHdl){
		(* pTableHdl ) = sRoutingTable.hdl;
		LOG_MSG_DEBUG("Table Handle =0x%x will be returned.",(*pTableHdl));
	}

bail:
	Free (pRoutingRule);
	LOG_MSG_STACK("Leaving Function (Returning %s)",bRetVal?"True":"False");
	return bRetVal;
}

bool CreateBypassRoutingTable_v2 (RoutingDriverWrapper * pRouting,enum ipa_ip_type eIP,
		const char * pTableName, enum ipa_client_type eRuleDestination,
		uint32_t uHeaderHandle, uint32_t * pTableHdl, uint8_t uClsAggrIrqMod)
{
	bool bRetVal = true;
	struct ipa_ioc_add_rt_rule_v2 *pRoutingRule = NULL;
	struct ipa_rt_rule_add_v2 *pRoutingRuleEntry = NULL;
	struct ipa_ioc_get_rt_tbl sRoutingTable;

	LOG_MSG_STACK("Entering Function");
	memset(&sRoutingTable,0,sizeof(sRoutingTable));
	pRoutingRule = (struct ipa_ioc_add_rt_rule_v2 *)
		calloc(1, sizeof(struct ipa_ioc_add_rt_rule_v2));
	if(!pRoutingRule) {
		LOG_MSG_ERROR("calloc failed to allocate pRoutingRule");
		bRetVal = false;
		goto bail;
	}
	pRoutingRule->rules = (uint64_t)calloc(1, sizeof(struct ipa_rt_rule_add_v2));
	if (!pRoutingRule->rules) {
		LOG_MSG_ERROR("calloc failed to allocate pRoutingRule->rules");
		bRetVal = false;
		goto bail;
	}

	pRoutingRule->num_rules = 1;
	pRoutingRule->rule_add_size = sizeof(struct ipa_rt_rule_add_v2);
	printf("%s(), Nadav: Adding rule_add_size\n", __FUNCTION__);
	pRoutingRule->ip = ((IPA_IP_v4 == eIP)? IPA_IP_v4 : IPA_IP_v6);
	pRoutingRule->commit = true;
	strlcpy(pRoutingRule->rt_tbl_name, pTableName, sizeof(pRoutingRule->rt_tbl_name));

	pRoutingRuleEntry = &(((struct ipa_rt_rule_add_v2 *)pRoutingRule->rules)[0]);
	pRoutingRuleEntry->at_rear = 1;
	pRoutingRuleEntry->rule.dst = eRuleDestination;// Setting Rule's Destination Pipe
	printf("%s(), Nadav: Destination = %d\n", __FUNCTION__, pRoutingRuleEntry->rule.dst);
	pRoutingRuleEntry->rule.hdr_hdl = uHeaderHandle; // Header handle
	pRoutingRuleEntry->rule.attrib.attrib_mask = 0;// All Packets will get a "Hit"
	pRoutingRuleEntry->rule.close_aggr_irq_mod = uClsAggrIrqMod;
	if (false == pRouting->AddRoutingRule(pRoutingRule))
	{
		LOG_MSG_ERROR("Routing rule addition(pRoutingRule) failed!");
		bRetVal = false;
		goto bail;
	}
	if (!pRoutingRuleEntry->rt_rule_hdl)
	{
		LOG_MSG_ERROR("pRoutingRuleEntry->rt_rule_hdl == 0, Routing rule addition(pRoutingRule) failed!");
		bRetVal = false;
		goto bail;
	}
	LOG_MSG_INFO("pRoutingRuleEntry->rt_rule_hdl == 0x%x added to Table %s",pRoutingRuleEntry->rt_rule_hdl,pTableName);
	sRoutingTable.ip = eIP;
	strlcpy(sRoutingTable.name, pTableName, sizeof(sRoutingTable.name));
	if (!pRouting->GetRoutingTable(&sRoutingTable)) {
		LOG_MSG_ERROR(
				"m_routing.GetRoutingTable(&sRoutingTable=0x%p) Failed.", &sRoutingTable);
		bRetVal = false;
		goto bail;
	}
	if(NULL != pTableHdl){
		(* pTableHdl ) = sRoutingTable.hdl;
		LOG_MSG_DEBUG("Table Handle =0x%x will be returned.",(*pTableHdl));
	}

bail:
	Free (pRoutingRule);
	LOG_MSG_STACK("Leaving Function (Returning %s)",bRetVal?"True":"False");
	return bRetVal;
}

//Don't use these methods directly. use MACROs instead
void __log_msg(enum msgType logType, const char* filename, int line, const char* function, const char* format, ... )
{
	va_list args;
	switch (logType) {
	case ERROR:
		fprintf( stderr, "ERROR!");
		break;
	case DEBUG:
		fprintf( stderr, "DEBUG:");
		break;
	case INFO:
		fprintf( stderr, "INFO :");
		break;
	case STACK:
		fprintf( stderr, "STACK:");
		break;
	default:
		fprintf( stderr, "BUG!!!");
		break;
	}
	fprintf( stderr, " [%s:%d, %s()] ",filename,line,function);
	va_start( args, format );
	vfprintf( stderr, format, args );
	va_end( args );
	fprintf( stderr, "\n" );
}

bool file_exists(const char* filename)
{
	return (access(filename, F_OK) == 0);
}

int ConfigureSystem(int testConfiguration, int fd)
{
	return ConfigureSystem(testConfiguration, fd, NULL);
}

int ConfigureSystem(int testConfiguration, int fd, const char* params)
{
	char testConfigurationStr[10];
	int ret;
	char *pSendBuffer;
	char str[10];
	int iter_cnt = 2000;

	if(params != NULL)
		pSendBuffer = new char[strlen(params) + 10];
	else
		pSendBuffer = new char[10];

	if (NULL == pSendBuffer)
	{
		LOG_MSG_ERROR("Failed to allocated pSendBuffer");
		return -1;
	}

	if(params != NULL)
		snprintf(pSendBuffer, strlen(params) + 10, "%d %s", testConfiguration, params);
	else
		snprintf(pSendBuffer, 10, "%d", testConfiguration);

	ret = write(fd, pSendBuffer, sizeof(pSendBuffer) );
	if (ret < 0) {
		g_Logger.AddMessage(LOG_ERROR, "%s Write operation failed.\n", __FUNCTION__);
		goto bail;
	}

	// Wait until the system is fully configured

	// Convert testConfiguration to string
	snprintf(testConfigurationStr, sizeof(testConfigurationStr), "%d", testConfiguration);

	// Read the configuration index from the device node
	ret = read(fd, str, sizeof(str));
	if (ret < 0) {
		g_Logger.AddMessage(LOG_ERROR, "%s Read operation failed.\n", __FUNCTION__);
		goto bail;
	}

	while ( strcmp(str, testConfigurationStr) ) {
		// Sleep for 5 msec
		struct timespec time;
		time.tv_sec = 0;
		time.tv_nsec = 50e6;
		nanosleep(&time, NULL);
		ret = read(fd, str, sizeof(str));
		if (ret < 0) {
			g_Logger.AddMessage(LOG_ERROR, "%s Read operation failed.\n", __FUNCTION__);
			goto bail;
		}
		if (!--iter_cnt) {
			g_Logger.AddMessage(LOG_ERROR, "%s timeout waiting for test driver.\n", __FUNCTION__);
			ret = -1;
			goto bail;
		}
	}
bail:
	delete[] pSendBuffer;
	return ret;
}

void ConfigureScenario(int testConfiguration)
{
	ConfigureScenario(testConfiguration, NULL);
}

void ConfigureScenario(int testConfiguration, const char* params)
{
	int fd, ret;
	char str[10];
	int currentConf;

	// Open /dev/ipa_test device node. This will allow to configure the system
	// and read its current configuration.
	fd = open(CONFIGURATION_NODE_PATH, O_RDWR);
	if (fd < 0) {
		g_Logger.AddMessage(LOG_ERROR, "%s Could not open configuration device node.\n", __FUNCTION__);
		exit(0);
	}

	// Read the current configuration.
	ret = read(fd, str, sizeof(str));
	if (ret < 0) {
		g_Logger.AddMessage(LOG_ERROR, "%s Read operation failed.\n", __FUNCTION__);
		return;
	}
	currentConf = atoi(str);

	// Do not reconfigure to the same configuration
	if (currentConf == testConfiguration) {
		g_Logger.AddMessage(LOG_DEVELOPMENT,"%s System was already configured as required(%d)\n",
			__FUNCTION__, currentConf);
		return;
	}

	/* in case the system is not "clean"*/
	if (-1 != currentConf) {
		g_Logger.AddMessage(LOG_DEVELOPMENT,"%s System has other configuration (%d) - cleanup\n", __FUNCTION__, currentConf);
		ret = ConfigureSystem(-1, fd);
		if (ret < 0) {
			g_Logger.AddMessage(LOG_ERROR, "%s Configure operation failed.\n",
					__FUNCTION__);
			return;
		}
	}

	// Start system configuration.
	g_Logger.AddMessage(LOG_DEVELOPMENT,"%s Setting system to the required configuration (%d)\n", __FUNCTION__, testConfiguration);

	ret = ConfigureSystem(testConfiguration, fd, params);
	if (ret < 0) {
		g_Logger.AddMessage(LOG_ERROR, "%s configure operation failed.\n",
				__FUNCTION__);
		return;
	}

	ret = system("mdev -s");
	if (ret < 0) {
		g_Logger.AddMessage(LOG_ERROR, "%s system(\"mdev -s\") returned %d\n",
				__FUNCTION__, ret);
	}

	close(fd);
}//func

void clean_old_stashed_config()
{
	if (current_configuration == NULL)
		return;

	for (int i = 0 ; i < current_configuration->from_ipa_channels_num ; i++) {
		delete((struct test_ipa_ep_cfg*)
				current_configuration->from_ipa_channel_config[i]->cfg);
		delete((struct test_ipa_ep_cfg*)
				current_configuration->from_ipa_channel_config[i]);
	}

	for (int i = 0 ; i < current_configuration->to_ipa_channels_num ; i++) {
		delete((struct test_ipa_ep_cfg*)
				current_configuration->to_ipa_channel_config[i]->cfg);
		delete((struct test_ipa_ep_cfg*)
				current_configuration->to_ipa_channel_config[i]);
	}
	delete(current_configuration);
	current_configuration = NULL;
}

void stash_new_configuration(struct ipa_test_config_header *header)
{
	clean_old_stashed_config();

        /*
	 * We will start by shallow copying each level, and afterwards,
	 * override the pointers
	 */
		current_configuration = new ipa_test_config_header();
		*current_configuration = *header;

		current_configuration->from_ipa_channel_config =
			new ipa_channel_config*[header->from_ipa_channels_num]();

		current_configuration->to_ipa_channel_config =
			new ipa_channel_config*[header->to_ipa_channels_num]();

		for (int i = 0 ; i < current_configuration->from_ipa_channels_num ; i++) {
			current_configuration->from_ipa_channel_config[i] =
				new ipa_channel_config;
			*current_configuration->from_ipa_channel_config[i] =
				*header->from_ipa_channel_config[i];
			current_configuration->from_ipa_channel_config[i]->cfg =
				new test_ipa_ep_cfg();
			memcpy(current_configuration->from_ipa_channel_config[i]->cfg,
				header->from_ipa_channel_config[i]->cfg,
				header->from_ipa_channel_config[i]->config_size);
		}

		for (int i = 0 ; i < current_configuration->to_ipa_channels_num ; i++) {
			current_configuration->to_ipa_channel_config[i] =
				new ipa_channel_config;
			*current_configuration->to_ipa_channel_config[i] =
				*header->to_ipa_channel_config[i];
			current_configuration->to_ipa_channel_config[i]->cfg = new test_ipa_ep_cfg();
			memcpy(current_configuration->to_ipa_channel_config[i]->cfg,
				header->to_ipa_channel_config[i]->cfg,
				header->to_ipa_channel_config[i]->config_size);
		}
}

bool is_prev_configuration_generic()
{
	int fd;
	char str[10] = {0};
	int retval;
	int current_conf_num;

	fd = open(CONFIGURATION_NODE_PATH, O_RDWR);
	if (fd < 0) {
		g_Logger.AddMessage(LOG_ERROR ,"%s Could not open configuration device node.\n", __FUNCTION__);
		exit(0);
	}

	retval = read(fd, str, sizeof(str));
	close(fd);
	if (retval < 0) {
		g_Logger.AddMessage(LOG_ERROR ,"%s Read operation failed.\n", __FUNCTION__);
		return true;
	}
	current_conf_num = atoi(str);

	if (current_conf_num == GENERIC_TEST_CONFIGURATION_IDX)
		return true;

	return false;
}


static bool is_reconfigure_required(struct ipa_test_config_header *header)
{
	// Is reconfiguration not required flag (current conf is cool)
	bool flag = true;

	if (is_prev_configuration_generic() == false) {
		g_Logger.AddMessage(LOG_DEVELOPMENT ,
			"prev configuration didn't use generic configuration\n");
		return true;
	}

	if (current_configuration == NULL) {
		g_Logger.AddMessage(LOG_DEVELOPMENT ,
			"no prev generic configuration found in the test app data-base\n");
		return true;
	}

	flag &= (header->from_ipa_channels_num ==
			current_configuration->from_ipa_channels_num);
	flag &= (header->to_ipa_channels_num ==
			current_configuration->to_ipa_channels_num);

	if (flag == false) {
		g_Logger.AddMessage(LOG_DEVELOPMENT ,
			"not same number of pipes\n");
		return true;
	}

	for (int i = 0 ; i < header->from_ipa_channels_num ; i++) {
		flag &= (header->from_ipa_channel_config[i]->client ==
				current_configuration->from_ipa_channel_config[i]->client);
		flag &= (header->from_ipa_channel_config[i]->index ==
				current_configuration->from_ipa_channel_config[i]->index);
		flag &= !memcmp(header->from_ipa_channel_config[i]->cfg,
				current_configuration->from_ipa_channel_config[i]->cfg,
				header->from_ipa_channel_config[i]->config_size);
		flag &= (header->from_ipa_channel_config[i]->en_status ==
				current_configuration->from_ipa_channel_config[i]->en_status);
	}

	if (flag == false) {
		g_Logger.AddMessage(LOG_DEVELOPMENT ,
			"\"from\" endpoint configuration is different from prev\n");
		return true;
	}

	for (int i = 0 ; i < header->to_ipa_channels_num ; i++) {
		flag &= (header->to_ipa_channel_config[i]->client ==
				current_configuration->to_ipa_channel_config[i]->client);
		flag &= (header->to_ipa_channel_config[i]->index ==
				current_configuration->to_ipa_channel_config[i]->index);
		flag &= !memcmp(header->to_ipa_channel_config[i]->cfg,
				current_configuration->to_ipa_channel_config[i]->cfg,
				header->to_ipa_channel_config[i]->config_size);
		flag &= (header->to_ipa_channel_config[i]->en_status ==
				current_configuration->to_ipa_channel_config[i]->en_status);
	}

	if (flag == false) {
		g_Logger.AddMessage(LOG_DEVELOPMENT ,
			"\"to\" endpoint configuration is different from prev\n");
		return true;
	}

	return false;
}

int GenericConfigureScenario(struct ipa_test_config_header *header, bool isUlso)
{
	int fd;
	int retval;

	if (is_reconfigure_required(header) == false) {
		g_Logger.AddMessage(LOG_DEVELOPMENT , "No need to reconfigure, we are all good :)\n");
		return true;
	} else {
		g_Logger.AddMessage(LOG_DEVELOPMENT , "Need to run configuration again\n");
	}
	g_Logger.AddMessage(LOG_DEVELOPMENT, "configuration has started, parameters:\n");
	g_Logger.AddMessage(LOG_DEVELOPMENT, "header->head_marker=0x%x\n", header->head_marker);
	g_Logger.AddMessage(LOG_DEVELOPMENT, "header->from_ipa_channels_num=%d\n", header->from_ipa_channels_num);
	g_Logger.AddMessage(LOG_DEVELOPMENT, "header->to_ipa_channels_num=%d\n", header->to_ipa_channels_num);
	g_Logger.AddMessage(LOG_DEVELOPMENT, "header->tail_marker=0x%x\n", header->tail_marker);

	for (int i = 0 ; i < header->from_ipa_channels_num ; i++) {
		g_Logger.AddMessage(LOG_DEVELOPMENT,
				"header->from_ipa_channel_config[%d]->head_marker=0x%x\n", i,
				header->from_ipa_channel_config[i]->head_marker);
		g_Logger.AddMessage(LOG_DEVELOPMENT,
				"header->from_ipa_channel_config[%d]->index=%d\n", i,
				header->from_ipa_channel_config[i]->index);
		g_Logger.AddMessage(LOG_DEVELOPMENT,
				"header->from_ipa_channel_config[%d]->client=%d\n", i,
				header->from_ipa_channel_config[i]->client);
		g_Logger.AddMessage(LOG_DEVELOPMENT,
				"header->from_ipa_channel_config[%d]->config_size=%d\n", i,
				header->from_ipa_channel_config[i]->config_size);
		g_Logger.AddMessage(LOG_DEVELOPMENT,
				"header->from_ipa_channel_config[%d]->en_status=%d\n", i,
				header->from_ipa_channel_config[i]->en_status);
	}

	for (int i = 0 ; i < header->to_ipa_channels_num ; i++) {
		g_Logger.AddMessage(LOG_DEVELOPMENT,
			"header->to_ipa_channel_config[%d]->head_marker=0x%x\n", i,
			header->to_ipa_channel_config[i]->head_marker);
		g_Logger.AddMessage(LOG_DEVELOPMENT,
			"header->to_ipa_channel_config[%d]->index=%d\n", i,
			header->to_ipa_channel_config[i]->index);
		g_Logger.AddMessage(LOG_DEVELOPMENT,
			"header->to_ipa_channel_config[%d]->client=%d\n", i,
			header->to_ipa_channel_config[i]->client);
		g_Logger.AddMessage(LOG_DEVELOPMENT,
			"header->to_ipa_channel_config[%d]->config_size=%d\n", i,
			header->to_ipa_channel_config[i]->config_size);
		g_Logger.AddMessage(LOG_DEVELOPMENT,
				"header->to_ipa_channel_config[%d]->en_status=%d\n", i,
				header->to_ipa_channel_config[i]->en_status);
	}

	fd = open(CONFIGURATION_NODE_PATH, O_RDWR);
	if (fd == -1) {
		g_Logger.AddMessage(LOG_ERROR,
				"%s - open %s failed (fd=%d,errno=%s)\n",
				__FUNCTION__, CONFIGURATION_NODE_PATH, fd, strerror(errno));
		return false;
	}

	if(isUlso){
		retval = ioctl(fd, IPA_TEST_IOC_ULSO_CONFIGURE, header);
	} else {
		retval = ioctl(fd, IPA_TEST_IOC_CONFIGURE, header);
	}
	if (retval) {
		g_Logger.AddMessage(LOG_ERROR, "fail to configure the system (%d)\n", retval);
		close(fd);
		return false;
	} else {
		g_Logger.AddMessage(LOG_DEVELOPMENT, "system was successfully configured\n");
	}

	retval = close(fd);
	if (retval) {
		g_Logger.AddMessage(LOG_ERROR,
				"%s - fail to close the fd (path=%s,retval=%d,fd=%d,errno=%s)\n",
				__FUNCTION__, CONFIGURATION_NODE_PATH, retval, fd, strerror(errno));
		return false;
	}

	g_Logger.AddMessage(LOG_DEVELOPMENT ,"stashing new configuration\n");
	stash_new_configuration(header);

	g_Logger.AddMessage(LOG_DEVELOPMENT,
			"Running mdev in order to create the device nodes.\n");

	retval = system("mdev -s");
	if (retval < 0) {
		g_Logger.AddMessage(LOG_ERROR, "%s system(\"mdev -s\") returned %d\n",
				__FUNCTION__, retval);
	}

	return true;
}

int GenericConfigureScenarioDestory(void)
{
	int fd;
	int retval;

	g_Logger.AddMessage(LOG_DEVELOPMENT, "cleanup started\n");

	fd = open(CONFIGURATION_NODE_PATH,  O_RDWR);
	if (fd == -1) {
		g_Logger.AddMessage(LOG_ERROR,
			"%s - open %s failed (retval=%d,fd=%d,errno=%s)\n",
			__FUNCTION__, CONFIGURATION_NODE_PATH, fd, strerror(errno));
		return false;
	}

	retval = ioctl(fd, IPA_TEST_IOC_CLEAN);
	if (retval)
		g_Logger.AddMessage(LOG_ERROR, "fail to clean the system (%d)\n", retval);
	else
		g_Logger.AddMessage(LOG_DEVELOPMENT, "system was successfully cleaned\n");

	retval = close(fd);
	if (retval) {
		g_Logger.AddMessage(LOG_ERROR, "fail to close the fd - %d\n", retval);
		return false;
	}

	return true;
}

bool configure_ep_ctrl(struct ipa_test_ep_ctrl *ep_ctrl)
{
	int fd;
	int retval = 0;

	g_Logger.AddMessage(LOG_DEVELOPMENT, "ep ctrl started \n");

	fd = open(CONFIGURATION_NODE_PATH,  O_RDWR);
	if (fd == -1) {
		g_Logger.AddMessage(LOG_ERROR,
			"%s - open %s failed (retval=%d,fd=%d,errno=%s)\n",
			__FUNCTION__, CONFIGURATION_NODE_PATH, fd, strerror(errno));
		return false;
	}

	retval = ioctl(fd, IPA_TEST_IOC_EP_CTRL, ep_ctrl);
	if (retval)
		g_Logger.AddMessage(LOG_ERROR, "fail to perform ep ctrl (%d)\n", retval);
	else
		g_Logger.AddMessage(LOG_DEVELOPMENT, "ep ctrl was successfully executed\n");

	retval = close(fd);
	if (retval) {
		g_Logger.AddMessage(LOG_ERROR, "fail to close the fd - %d\n", retval);
	}

	return true;
}

bool configure_holb(struct ipa_test_holb_config *test_holb_config)
{
	int fd;
	int retval = 0;

	g_Logger.AddMessage(LOG_DEVELOPMENT, "holb config started \n");

	if (!test_holb_config) {
		g_Logger.AddMessage(LOG_ERROR, "Null pointer argument!");
		return false;
	}

	fd = open(CONFIGURATION_NODE_PATH,  O_RDWR);
	if (fd == -1) {
		g_Logger.AddMessage(LOG_ERROR,
			"%s - open %s failed (retval=%d,fd=%d,errno=%s)\n",
			__FUNCTION__, CONFIGURATION_NODE_PATH, fd, strerror(errno));
		return false;
	}

	retval = ioctl(fd, IPA_TEST_IOC_HOLB_CONFIG, test_holb_config);
	if (retval)
		g_Logger.AddMessage(LOG_ERROR,
							"fail to perform holb config (%d)\n",
							retval);
	else
		g_Logger.AddMessage(LOG_DEVELOPMENT,
							"holb config was successfully executed\n");

	retval = close(fd);
	if (retval) {
		g_Logger.AddMessage(LOG_ERROR, "fail to close the fd - %d\n", retval);
	}

	return true;
}

void prepare_channel_struct(struct ipa_channel_config *channel,
		int index,
		enum ipa_client_type client,
		void *cfg,
		size_t config_size,
		bool en_status)
{
	channel->head_marker = IPA_TEST_CHANNEL_CONFIG_MARKER;
	channel->index = index;
	channel->client = client;
	channel->cfg = (char*)cfg;
	channel->config_size = config_size;
	channel->tail_marker = IPA_TEST_CHANNEL_CONFIG_MARKER;
	channel->en_status = en_status;
}

void prepare_header_struct(struct ipa_test_config_header *header,
		struct ipa_channel_config **from,
		struct ipa_channel_config **to)
{
	header->head_marker = IPA_TEST_CONFIG_MARKER;
	header->from_ipa_channel_config = from;
	header->to_ipa_channel_config = to;
	header->tail_marker = IPA_TEST_CONFIG_MARKER;
}

bool CompareResultVsGolden(Byte *goldenBuffer,   unsigned int goldenSize,
			   Byte *receivedBuffer, unsigned int receivedSize)
{
	if (receivedSize != goldenSize) {
		g_Logger.AddMessage(LOG_VERBOSE,  "%s File sizes are different.\n", __FUNCTION__);
		return false;
	}
	return !memcmp((void*)receivedBuffer, (void*)goldenBuffer, goldenSize);
}

bool CompareResultVsGolden_w_Status(Byte *goldenBuffer,   unsigned int goldenSize,
			   Byte *receivedBuffer, unsigned int receivedSize)
{
	size_t stts_size = sizeof(struct ipa3_hw_pkt_status);

	if (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) {
		stts_size = sizeof(struct ipa3_hw_pkt_status_hw_v5_0);
	}

	if ((receivedSize - stts_size) != goldenSize) {
		g_Logger.AddMessage(LOG_VERBOSE,  "%s Buffer sizes are different.\n", __FUNCTION__);
		return false;
	}

	printf("comparison is made considering %zu status bytes!\n", stts_size);

	return !memcmp((void*)((unsigned char *)receivedBuffer +
		stts_size), (void*)goldenBuffer, goldenSize);
}


Byte *LoadFileToMemory(const string &name, unsigned int *sizeLoaded)
{
	FILE *file;
	Byte *buffer;
	size_t fileLen;

	// Open file
	file = fopen(name.c_str(), "rb");
	if (!file) {
		g_Logger.AddMessage(LOG_ERROR,  "Unable to open file %s\n", name.c_str());
		return NULL;
	}

	// Get file length
	fseek(file, 0, SEEK_END);
	fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Allocate memory
	buffer=(Byte*)malloc(fileLen+1);
	if (!buffer) {
		fprintf(stderr, "Memory error!\n");
		fclose(file);
		return NULL;
	}

	// Read file contents into buffer
	*sizeLoaded = fread(buffer, 1, fileLen, file);
	fclose(file);

	return buffer;
}

void print_buff(void *data, size_t size)
{
	uint8_t bytes_in_line = 16;
	uint i, j, num_lines;
	char str[1024], tmp[4];

	num_lines = size / bytes_in_line;
	if (size % bytes_in_line > 0)
		num_lines++;

	printf("Printing buffer at address 0x%p, size = %zu: \n", data, size);
	for (i = 0 ; i < num_lines; i++) {
		str[0] = '\0';
		for (j = 0; (j < bytes_in_line) && ((i * bytes_in_line + j) < size); j++) {
			snprintf(tmp, sizeof(tmp), "%02x ",
				 ((unsigned char*)data)[i * bytes_in_line + j]);
			strlcpy(str + strlen(str), tmp, sizeof(str) - strlen(str));
		}
		printf("%s\n", str);
	}
}

void add_buff(uint8_t *data, size_t size, uint8_t val)
{
	for (int i = 0; i < static_cast<int>(size); i++)
		data[i]+=val;
}

bool RegSuspendHandler(bool deferred_flag, bool reg, int DevNum)
{
	int fd = 0;
	int retval = 0;
	struct ipa_test_reg_suspend_handler RegData;

	fd = open(CONFIGURATION_NODE_PATH,  O_RDWR);
	if (fd == -1) {
		g_Logger.AddMessage(LOG_ERROR,
				"%s - open %s failed (fd=%d,errno=%s)\n",
				__FUNCTION__, CONFIGURATION_NODE_PATH, fd, strerror(errno));
		return false;
	}

	RegData.DevNum = DevNum;
	RegData.reg = reg;
	RegData.deferred_flag = deferred_flag;

	retval = ioctl(fd, IPA_TEST_IOC_REG_SUSPEND_HNDL, &RegData);
	if (retval) {
		g_Logger.AddMessage(LOG_ERROR, "fail to reg suspend handler (%d)\n", retval);
		close(fd);
		return false;
	} else {
		g_Logger.AddMessage(LOG_DEVELOPMENT, "suspend handler was successfully configured\n");
	}

	close(fd);

	return true;
}

const Byte Eth2Helper::m_ETH2_IP4_HDR[ETH_HLEN] =
{
	0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,	// ETH2 DST
	0xA7, 0xA8, 0xA9, 0xB0, 0xB1, 0xB2,	// ETH2 SRC
	0x08, 0x00,	// ETH2 TYPE IPv4 - ETH_P_IP 0x0800
};

bool Eth2Helper::LoadEth2IP4Header(
	uint8_t *pBuffer,
	size_t bufferSize,
	size_t *pLen)
{
	if (bufferSize < ETH_HLEN)
	{
		LOG_MSG_ERROR("Buffer too small\n");
		return false;
	}

	memcpy(pBuffer, m_ETH2_IP4_HDR, ETH_HLEN);

	*pLen = ETH_HLEN;

	return true;
}

bool Eth2Helper::LoadEth2IP6Header(
	uint8_t *pBuffer,
	size_t bufferSize,
	size_t *pLen)
{
	if (bufferSize < ETH_HLEN)
	{
		LOG_MSG_ERROR("Buffer too small\n");
		return false;
	}

	// copy eth2 ip4 header
	memcpy(pBuffer, m_ETH2_IP4_HDR, ETH_HLEN);

	// change ethtype to ip6
	pBuffer[ETH2_ETH_TYPE_OFFSET] = 0x86;
	pBuffer[ETH2_ETH_TYPE_OFFSET+1] = 0xdd;

	*pLen = ETH_HLEN;

	return true;
}

bool Eth2Helper::LoadEth2IP4Packet(
	uint8_t *pBuffer,
	size_t bufferSize,
	size_t *pLen)
{
	size_t cnt = 0;
	size_t len = 0;

	if (!LoadEth2IP4Header(pBuffer, bufferSize, &cnt))
		return false;

	len = bufferSize - cnt;

	if (!LoadDefaultPacket(IPA_IP_v4, pBuffer + cnt, len))
		return false;

	*pLen = len + cnt;

	return true;
}

bool Eth2Helper::LoadEth2IP6Packet(
	uint8_t *pBuffer,
	size_t bufferSize,
	size_t *pLen)
{
	size_t cnt = 0;
	size_t len = 0;

	if (!LoadEth2IP6Header(pBuffer, bufferSize, &cnt))
		return false;

	len = bufferSize - cnt;

	if (!LoadDefaultPacket(IPA_IP_v6, pBuffer + cnt, len))
		return false;

	*pLen = len + cnt;

	return true;
}

const Byte WlanHelper::m_WLAN_HDR[WLAN_HDR_SIZE] =
{
	// WLAN hdr - 4 bytes
	0x01, 0x02, 0x03, 0x04
};

bool WlanHelper::LoadWlanHeader(
	uint8_t *pBuffer,
	size_t bufferSize,
	size_t *pLen)
{
	if (bufferSize < WLAN_HDR_SIZE)
	{
		LOG_MSG_ERROR("Buffer too small\n");
		return false;
	}

	memcpy(pBuffer, m_WLAN_HDR, WLAN_HDR_SIZE);

	*pLen = WLAN_HDR_SIZE;

	return true;
}

bool WlanHelper::LoadWlanEth2IP4Header(
	uint8_t *pBuffer,
	size_t bufferSize,
	size_t *pLen)
{
	size_t cnt = 0;
	size_t len = 0;

	if (!LoadWlanHeader(pBuffer, bufferSize, &cnt))
		return false;

	if (!Eth2Helper::LoadEth2IP4Header(
		pBuffer + cnt,
		bufferSize - cnt,
		&len))
		return false;

	*pLen = len + cnt;

	return true;
}

bool WlanHelper::LoadWlanEth2IP6Header(
	uint8_t *pBuffer,
	size_t bufferSize,
	size_t *pLen)
{
	size_t cnt = 0;
	size_t len = 0;

	if (!LoadWlanHeader(pBuffer, bufferSize, &cnt))
		return false;

	if (!Eth2Helper::LoadEth2IP6Header(
		pBuffer + cnt,
		bufferSize - cnt,
		&len))
		return false;

	*pLen = len + cnt;

	return true;
}

bool WlanHelper::LoadWlanEth2IP4Packet(
	uint8_t *pBuffer,
	size_t bufferSize,
	size_t *pLen)
{
	size_t cnt = 0;
	size_t len = 0;

	if (!LoadWlanHeader(pBuffer, bufferSize, &cnt))
		return false;

	if (!Eth2Helper::LoadEth2IP4Packet(
		pBuffer + cnt,
		bufferSize - cnt,
		&len))
		return false;

	*pLen = len + cnt;

	return true;
}

bool PadByLength(
	uint8_t *pBuffer,
	size_t bufferSize,
	size_t len,
	uint8_t padValue)
{
	if (bufferSize < len)
	{
		LOG_MSG_ERROR("bufferSize < len.\n");
		return false;
	}

	memset(pBuffer, padValue, len);

	return true;
}

bool WlanHelper::LoadWlanEth2IP4PacketByLength(
	uint8_t *pBuffer,
	size_t bufferSize,
	size_t len,
	uint8_t padValue)
{
	size_t cnt = 0;

	if (!LoadWlanEth2IP4Packet(pBuffer, bufferSize, &cnt))
		return false;

	if (!PadByLength(pBuffer + cnt, bufferSize - cnt, len - cnt, padValue))
		return false;

	return true;
}

bool RNDISAggregationHelper::LoadRNDISHeader(
	uint8_t *pBuffer,
	size_t bufferSize,
	uint32_t messageLength,
	size_t *pLen)
{
	if (bufferSize < RNDIS_HDR_SIZE)
	{
		LOG_MSG_ERROR("Buffer too small\n");
		return false;
	}

	struct RndisHeader *pRndisHeader =
		reinterpret_cast<struct RndisHeader*>(pBuffer);

	memset(pRndisHeader, 0, sizeof(struct RndisHeader));
	pRndisHeader->MessageType = 0x01;
	pRndisHeader->DataOffset = 0x24;

	if (messageLength > RNDIS_HDR_SIZE)
	{
		pRndisHeader->MessageLength = messageLength;
		pRndisHeader->DataLength = messageLength - RNDIS_HDR_SIZE;
	}
	else
	{
		// This handles a case where we use the header
		// in IPA headers table
		// IPA needs to set these values
		pRndisHeader->MessageLength = 0;
		pRndisHeader->DataLength = 0;
	}

	*pLen = RNDIS_HDR_SIZE;

	return true;
}

bool RNDISAggregationHelper::LoadRNDISEth2IP4Header(
	uint8_t *pBuffer,
	size_t bufferSize,
	uint32_t messageLength,
	size_t *pLen)
{
	size_t cnt = 0;
	size_t len = 0;

	if (!LoadRNDISHeader(pBuffer, bufferSize, messageLength, &cnt))
		return 0;

	if (!Eth2Helper::LoadEth2IP4Header(pBuffer + cnt, bufferSize - cnt, &len))
		return false;

	*pLen = cnt + len;

	return true;
}

bool RNDISAggregationHelper::LoadRNDISPacket(
	enum ipa_ip_type eIP,
	uint8_t *pBuffer,
	size_t &nMaxSize)
{
	if (nMaxSize < sizeof(struct RndisHeader))
	{
		LOG_MSG_ERROR("Buffer too small\n");
		return false;
	}

	size_t nMaxSizeForDefaultPacket = nMaxSize - sizeof(struct RndisHeader);

	if (!LoadEtherPacket(eIP, pBuffer + sizeof(struct RndisHeader),
		nMaxSizeForDefaultPacket))
	{
		LOG_MSG_ERROR("LoadEtherPacket() failed\n");
		return false;
	}

	nMaxSize = nMaxSizeForDefaultPacket + sizeof(struct RndisHeader);
	struct RndisHeader *pRndisHeader = (struct RndisHeader*)pBuffer;

	memset(pRndisHeader, 0, sizeof(struct RndisHeader));
	pRndisHeader->MessageType = 0x01;
	pRndisHeader->MessageLength = nMaxSize;
	pRndisHeader->DataOffset = 0x24;
	pRndisHeader->DataLength = nMaxSizeForDefaultPacket;
	return true;
}

bool RNDISAggregationHelper::LoadEtherPacket(
	enum ipa_ip_type eIP,
	uint8_t *pBuffer,
	size_t &nMaxSize)
{
	if (nMaxSize < sizeof(struct ethhdr))
	{
		LOG_MSG_ERROR("Buffer too small\n");
		return false;
	}

	size_t nMaxSizeForDefaultPacket = nMaxSize - sizeof(struct ethhdr);

	if (!LoadDefaultPacket(eIP, pBuffer + sizeof(struct ethhdr),
		nMaxSizeForDefaultPacket))
	{
		LOG_MSG_ERROR("LoadDefaultPacket() failed\n");
		return false;
	}

	nMaxSize = nMaxSizeForDefaultPacket + sizeof(struct ethhdr);
	struct ethhdr *pEtherHeader = (struct ethhdr*)pBuffer;

	memcpy(pEtherHeader, Eth2Helper::m_ETH2_IP4_HDR, sizeof(struct ethhdr));

	print_buff(pBuffer, nMaxSize);
	return true;


}

bool RNDISAggregationHelper::CompareIPvsRNDISPacket(
	Byte *pIPPacket,
	int ipPacketSize,
	Byte *pRNDISPacket,
	size_t rndisPacketSize)
{
	struct RndisHeader *pRndisHeader = (struct RndisHeader*)pRNDISPacket;

	if (pRndisHeader->MessageType != 0x01)
	{
		LOG_MSG_ERROR("Wrong  MessageType 0x%8x\n",
			pRndisHeader->MessageType);
		return false;
	}

	if (pRndisHeader->MessageLength != rndisPacketSize)
	{
		LOG_MSG_ERROR(
			"Packet sizes do not match 0x%8x expected 0x%8x\n",
			pRndisHeader->MessageLength, rndisPacketSize);
		return false;
	}

	// Create Ethernet packet from the IP packet and compare it to RNDIS payload
	size_t EtherPacketSize = ipPacketSize + sizeof(struct ethhdr);
	Byte* pEtherPacket = (Byte *) malloc(EtherPacketSize);
	if (pEtherPacket == NULL) {
		LOG_MSG_ERROR("Memory allocation failure.\n");
		return false;
	}

	memcpy(pEtherPacket, Eth2Helper::m_ETH2_IP4_HDR, sizeof(struct ethhdr));
	memcpy(pEtherPacket + sizeof(struct ethhdr), pIPPacket, ipPacketSize);

	if (pRndisHeader->DataLength != EtherPacketSize)
	{
		LOG_MSG_ERROR(
			"Packet sizes do not match 0x%8x expected 0x%8x\n",
			pRndisHeader->DataLength, EtherPacketSize);
		Free(pEtherPacket);
		return false;
	}

	if(!ComparePackets(
		(Byte*)&pRndisHeader->DataOffset + pRndisHeader->DataOffset,
		EtherPacketSize, pEtherPacket, EtherPacketSize))
	{
		LOG_MSG_ERROR("Packets do not match\n");
		Free(pEtherPacket);
		return false;
	}

	Free(pEtherPacket);
	return true;
}

bool RNDISAggregationHelper::CompareEthervsRNDISPacket(
	Byte *pIPPacket,
	size_t ipPacketSize,
	Byte *pRNDISPacket,
	size_t rndisPacketSize)
{
	struct RndisHeader *pRndisHeader = (struct RndisHeader*)pRNDISPacket;

	if (pRndisHeader->MessageType != 0x01)
	{
		LOG_MSG_ERROR("Wrong  MessageType 0x%8x\n",
			pRndisHeader->MessageType);
		return false;
	}

	if (pRndisHeader->MessageLength != rndisPacketSize)
	{
		LOG_MSG_ERROR(
			"Packet sizes do not match 0x%8x expected 0x%8x\n",
			pRndisHeader->MessageLength, rndisPacketSize);
		return false;
	}

	if (pRndisHeader->DataLength != ipPacketSize)
	{
		LOG_MSG_ERROR(
			"Packet sizes do not match 0x%8x expected 0x%8x\n",
			pRndisHeader->DataLength, ipPacketSize);
		return false;
	}

	return ComparePackets(
		(Byte*)&pRndisHeader->DataOffset + pRndisHeader->DataOffset,
		ipPacketSize, pIPPacket, ipPacketSize);
}

bool RNDISAggregationHelper::ComparePackets(
	Byte *pPacket,
	int packetSize,
	Byte *pExpectedPacket,
	int expectedPacketSize)
{
	bool res = true;

	if (packetSize != expectedPacketSize)
	{
		LOG_MSG_ERROR("Packet sizes do not match\n");
		res = false;
	}

	for (int i = 0; i < packetSize; i++)
	{
		if (pPacket[i] != pExpectedPacket[i])
		{
			LOG_MSG_ERROR(
				"Byte %d not match 0x%2x != 0x%2x\n",
				i, pPacket[i], pExpectedPacket[i]);
			res = false;
		}
	}

	if (!res)
	{
		LOG_MSG_ERROR("Packet:\n");
		print_buff(pPacket, packetSize);
		LOG_MSG_ERROR("Expected Packet:\n");
		print_buff(pExpectedPacket, expectedPacketSize);
	}

	return res;
}


#if !defined(MSM_IPA_TESTS) && !defined(USE_GLIB) && !defined(FEATURE_IPA_ANDROID)
size_t strlcpy(char* dst, const char* src, size_t size)
{
	size_t i;

	if (size == 0)
		return strlen(src);

	for (i = 0; i < (size - 1) && src[i] != '\0'; ++i)
		dst[i] = src[i];

	dst[i] = '\0';

	return i + strlen(src + i);
}
#endif
