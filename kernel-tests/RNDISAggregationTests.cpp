/*
 * Copyright (c) 2017,2020 The Linux Foundation. All rights reserved.
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
#include <string.h>
#include <stdint.h>
#include "hton.h" /* for htonl*/
#include "RNDISAggregationTestFixture.h"
#include "Constants.h"
#include "TestsUtils.h"
#include "linux/msm_ipa.h"

#define IPV4_DST_ADDR_OFFSET (16)
#define IPV4_DST_ADDR_OFFSET_IN_ETH \
		(16 /* IP */ + 14 /* ethernet */)
#define IPV4_DST_ADDR_OFFSET_IN_RNDIS \
		(IPV4_DST_ADDR_OFFSET_IN_ETH + \
				sizeof(struct RndisHeader))

#define NUM_PACKETS (4)

class RNDISAggregationSanityTest: public RNDISAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	RNDISAggregationSanityTest()
	{
		m_name = "RNDISAggregationSanityTest";
		m_description = "RNDISAggregationSanityTest - Send one packet "
			"and expect same packet.";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRulesNoAgg();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		//The packets that will be sent
		Byte pPacket[MAX_PACKET_SIZE];
		//Buffer for the packet that will be received
		Byte pReceivedPacket[2*MAX_PACKET_SIZE];
		//Total size of all sent packets (this is the max size of the aggregated
		//packet minus the size of the header and the NDP)
		//int nTotalPacketsSize = MAX_PACKET_SIZE - (4 * NUM_PACKETS) - 24;
		uint32_t nIPv4DSTAddr;
		size_t pIpPacketsSize;

		//initialize the packets
		// Load input data (Ethernet packet) from file
		pIpPacketsSize = MAX_PACKET_SIZE;
		if (!RNDISAggregationHelper::LoadEtherPacket(m_eIP, pPacket, pIpPacketsSize))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPacket[IPV4_DST_ADDR_OFFSET_IN_ETH],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));


		//send the packet
		LOG_MSG_DEBUG("Sending packet into the A2 EMB pipe(%d bytes)\n",
				pIpPacketsSize);
		size_t nBytesSent = m_UsbToIpaPipe.Send(pPacket, pIpPacketsSize);
		if (pIpPacketsSize != nBytesSent)
		{
			LOG_MSG_ERROR("Sending packet into the A2 EMB pipe(%d bytes) "
				"failed!\n", pIpPacketsSize);
			return false;
		}

		//receive the packet
		LOG_MSG_DEBUG("Reading packet from the A2 EMB pipe(%d bytes should be there)"
			"\n", pIpPacketsSize);
		size_t nBytesReceived = m_IpaToUsbPipe.Receive(pReceivedPacket, MAX_PACKET_SIZE);
		if (pIpPacketsSize != nBytesReceived)
		{
			LOG_MSG_ERROR("Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", pIpPacketsSize);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}
		return RNDISAggregationHelper::ComparePackets(pReceivedPacket, nBytesReceived,
			pPacket, pIpPacketsSize);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

class RNDISAggregationDeaggregation1PacketTest: public RNDISAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	RNDISAggregationDeaggregation1PacketTest()
	{
		m_name = "RNDISAggregationDeaggregation1PacketTest";
		m_description = "RNDISAggregationDeaggregation1PacketTest - Send 1 RNDIS packet "
			"and expect Ethernet packet.";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRulesDeAggEther();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		//The packets that will be sent
		Byte pPacket[MAX_PACKET_SIZE];
		//Buffer for the packet that will be received
		Byte pReceivedPacket[2*MAX_PACKET_SIZE];
		//Total size of all sent packets (this is the max size of the aggregated
		//packet minus the size of the header and the NDP)
		//int nTotalPacketsSize = MAX_PACKET_SIZE - (4 * NUM_PACKETS) - 24;
		uint32_t nIPv4DSTAddr;
		size_t pIpPacketsSize;

		//initialize the packets
		// Load input data (IP packet) from file
		pIpPacketsSize = MAX_PACKET_SIZE;
		if (!RNDISAggregationHelper::LoadRNDISPacket(m_eIP, pPacket, pIpPacketsSize))
		{
			LOG_MSG_ERROR("Failed to load RNDIS Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPacket[IPV4_DST_ADDR_OFFSET_IN_RNDIS],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));

		//send the packet
		LOG_MSG_DEBUG("Sending packet into the A2 TETH pipe(%d bytes)\n",
				pIpPacketsSize);
		size_t nBytesSent = m_UsbToIpaPipeDeagg.Send(pPacket, pIpPacketsSize);
		if (pIpPacketsSize != nBytesSent)
		{
			LOG_MSG_ERROR("Sending packet into the A2 EMB pipe(%d bytes) "
				"failed!\n", pIpPacketsSize);
			return false;
		}

		//receive the packet
		LOG_MSG_DEBUG("Reading packet from the A2 TETH pipe(%d bytes should be there)"
			"\n", pIpPacketsSize);
		size_t nBytesReceived = m_IpaToUsbPipe.Receive(pReceivedPacket, MAX_PACKET_SIZE);
		if (pIpPacketsSize - sizeof(struct RndisHeader) != nBytesReceived)
		{
			LOG_MSG_ERROR("Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", pIpPacketsSize);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}
		return RNDISAggregationHelper::CompareEthervsRNDISPacket(pReceivedPacket, nBytesReceived,
			pPacket, pIpPacketsSize);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

class RNDISAggregation1PacketTest: public RNDISAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	RNDISAggregation1PacketTest()
	{
		m_name = "RNDISAggregation1PacketTest";
		m_description = "RNDISAggregation1PacketTest - Send 1 IP packet "
			"and expect RNDIS packet.";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRulesAggTimeLimit();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		//The packets that will be sent
		Byte pPacket[MAX_PACKET_SIZE];
		//Buffer for the packet that will be received
		Byte pReceivedPacket[2*MAX_PACKET_SIZE];
		//Total size of all sent packets (this is the max size of the aggregated
		//packet minus the size of the header and the NDP)
		//int nTotalPacketsSize = MAX_PACKET_SIZE - (4 * NUM_PACKETS) - 24;
		uint32_t nIPv4DSTAddr;
		size_t pIpPacketsSize;

		//initialize the packets
		// Load input data (IP packet) from file
		pIpPacketsSize = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPacket, pIpPacketsSize))
		{
			LOG_MSG_ERROR("Failed to load Ethernet Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPacket[IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));

		//send the packet
		LOG_MSG_DEBUG("Sending packet into the USB pipe(%d bytes)\n",
				pIpPacketsSize);
		size_t nBytesSent = m_HsicToIpaPipe.Send(pPacket, pIpPacketsSize);
		if (pIpPacketsSize != nBytesSent)
		{
			LOG_MSG_ERROR("Sending packet into the USB pipe(%d bytes) "
				"failed!\n", pIpPacketsSize);
			return false;
		}

		//receive the packet
		LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be there)"
			"\n", pIpPacketsSize);
		size_t nBytesReceived = m_IpaToUsbPipeAggTime.Receive(pReceivedPacket, MAX_PACKET_SIZE);
		if (pIpPacketsSize != nBytesReceived - sizeof(struct RndisEtherHeader))
		{
			LOG_MSG_ERROR("Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", pIpPacketsSize);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}


		return RNDISAggregationHelper::CompareIPvsRNDISPacket(pPacket, pIpPacketsSize,
			pReceivedPacket, nBytesReceived);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

class RNDISAggregationSuspendWaTest: public RNDISAggregationTestFixture {
public:
	/////////////////////////////////////////////////////////////////////////////////

	RNDISAggregationSuspendWaTest()
	{
		m_name = "RNDISAggregationSuspendWaTest";
		m_description = "RNDISAggregationSuspendWaTest - Send 3 IP packet instead 4, suspend the pipe"
			" and expect aggregation to be closed.";
		m_minIPAHwType = IPA_HW_v3_0;

		// WA not needed in IPA_3_5
		m_maxIPAHwType = IPA_HW_v3_1;
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRulesAggByteLimit();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool Setup()
	{
		bool bRetVal = true;

		bRetVal = RNDISAggregationTestFixture::Setup();
		if (bRetVal == false) {
			return bRetVal;
		}

		/* register test framework suspend handler to from_ipa_devs */
		bRetVal = RegSuspendHandler(false, true, 0);

		return bRetVal;
	}

	bool TestLogic()
	{
		/*The packets that will be sent*/

		Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
		/*Buffer for the packet that will be received*/
		Byte pReceivedPacket[2*MAX_PACKET_SIZE];
		/*
		 *Total size of all sent packets
		 * (this is the max size of the aggregated
		 *packet minus the size of the header and the NDP)
		 */
		uint32_t nIPv4DSTAddr;
		size_t pIpPacketsSizes[NUM_PACKETS];
		size_t ExpectedPacketSize = (NUM_PACKETS - 1) * sizeof(struct RndisEtherHeader);

		struct ipa_test_ep_ctrl ep_ctrl;

		/* send one packet less than aggregation size in order to see the force close */
		for(int i = 0; i < (NUM_PACKETS - 1); i++) {
			/*
			 *initialize the packets
			 *Load input data (IP packet) from file
			 */
			pIpPacketsSizes[i] = MAX_PACKET_SIZE;
			if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
			{
				LOG_MSG_ERROR("Failed to load Ethernet Packet");
				return false;
			}
			nIPv4DSTAddr = ntohl(0x7F000001);
			memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
			for(int j = pIpPacketsSizes[i]; j < MAX_PACKET_SIZE / NUM_PACKETS + 1; j++) {
				pPackets[i][j] = j & 0xFF;
			}
			pIpPacketsSizes[i] = MAX_PACKET_SIZE / NUM_PACKETS + 1;

			/* send the packet */
			LOG_MSG_DEBUG("Sending packet into the A2 TETH pipe(%d bytes)\n",
					pIpPacketsSizes[i]);
			size_t nBytesSent = m_HsicToIpaPipe.Send(pPackets[i], pIpPacketsSizes[i]);
			if (pIpPacketsSizes[i] != nBytesSent)
			{
				LOG_MSG_ERROR("Sending packet into the USB pipe(%d bytes) "
					"failed!\n", pIpPacketsSizes[i]);
				return false;
			}
			ExpectedPacketSize += pIpPacketsSizes[i];
		}

		/* suspend the pipe */
		ep_ctrl.from_dev_num = 0;
		ep_ctrl.ipa_ep_delay = false;
		ep_ctrl.ipa_ep_suspend = true;
		configure_ep_ctrl(&ep_ctrl);

		/* receive the packet */
		LOG_MSG_DEBUG(
			"Reading packet from the USB pipe(%d bytes should be there)"
			"\n", ExpectedPacketSize);
		size_t nBytesReceived = m_IpaToUsbPipeAgg
				.Receive(pReceivedPacket, MAX_PACKET_SIZE);
		if (ExpectedPacketSize != nBytesReceived)
		{
			LOG_MSG_ERROR(
				"Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", nBytesReceived);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}


		for(int i = 0; i < (NUM_PACKETS - 1); i++) {
			if (!RNDISAggregationHelper::
					CompareIPvsRNDISPacket(pPackets[i], pIpPacketsSizes[i],
				pReceivedPacket + (i * ExpectedPacketSize / (NUM_PACKETS - 1)),
				ExpectedPacketSize / (NUM_PACKETS - 1)))
				return false;
		}

		return true;
	}

	bool Teardown()
	{
		bool bRetVal = true;

		bRetVal = RNDISAggregationTestFixture::Teardown();
		if (bRetVal == false) {
			return bRetVal;
		}

		/* unregister the test framework suspend handler */
		bRetVal = RegSuspendHandler(false, false, 0);

		return bRetVal;
	}
};

class RNDISAggregationByteLimitTest: public RNDISAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	RNDISAggregationByteLimitTest()
	{
		m_name = "RNDISAggregationByteLimitTest";
		m_description = "RNDISAggregationByteLimitTest - Send 2 IP packet "
			"and expect aggregated RNDIS packet.";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRulesAggByteLimit();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		/*The packets that will be sent*/

		Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
		/*Buffer for the packet that will be received*/
		Byte pReceivedPacket[2*MAX_PACKET_SIZE];
		/*Total size of all sent packets
		 * (this is the max size of the aggregated
		 *packet minus the size of the header and the NDP)
		 */
		uint32_t nIPv4DSTAddr;
		size_t pIpPacketsSizes[NUM_PACKETS];
		size_t ExpectedPacketSize = NUM_PACKETS * sizeof(struct RndisEtherHeader);

		for(int i = 0; i < NUM_PACKETS; i++) {
			/*
			 *initialize the packets
			 *Load input data (IP packet) from file
			 */
			pIpPacketsSizes[i] = MAX_PACKET_SIZE;
			if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
			{
				LOG_MSG_ERROR("Failed to load Ethernet Packet");
				return false;
			}
			nIPv4DSTAddr = ntohl(0x7F000001);
			memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
			for(int j = pIpPacketsSizes[i]; j < MAX_PACKET_SIZE / NUM_PACKETS + 1; j++) {
				pPackets[i][j] = j & 0xFF;
			}
			pIpPacketsSizes[i] = MAX_PACKET_SIZE / NUM_PACKETS + 1;

			/* send the packet */
			LOG_MSG_DEBUG("Sending packet into the A2 TETH pipe(%d bytes)\n",
					pIpPacketsSizes[i]);
			size_t nBytesSent = m_HsicToIpaPipe.Send(pPackets[i], pIpPacketsSizes[i]);
			if (pIpPacketsSizes[i] != nBytesSent)
			{
				LOG_MSG_ERROR("Sending packet into the USB pipe(%d bytes) "
					"failed!\n", pIpPacketsSizes[i]);
				return false;
			}
			ExpectedPacketSize += pIpPacketsSizes[i];
		}

		/* receive the packet */
		LOG_MSG_DEBUG(
			"Reading packet from the USB pipe(%d bytes should be there)"
			"\n", ExpectedPacketSize);
		size_t nBytesReceived = m_IpaToUsbPipeAgg
				.Receive(pReceivedPacket, MAX_PACKET_SIZE);
		if (ExpectedPacketSize != nBytesReceived)
		{
			LOG_MSG_ERROR(
				"Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", nBytesReceived);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}


		for(int i = 0; i < NUM_PACKETS; i++) {
			if (!RNDISAggregationHelper::
					CompareIPvsRNDISPacket(pPackets[i], pIpPacketsSizes[i],
				pReceivedPacket + (i * ExpectedPacketSize / NUM_PACKETS),
				ExpectedPacketSize / NUM_PACKETS))
				return false;
		}

		return true;
	}
};

class RNDISAggregationByteLimitTestFC : public RNDISAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	RNDISAggregationByteLimitTestFC() {
		m_name = "RNDISAggregationByteLimitTestFC";
		m_description = "RNDISAggregationByteLimitTestFC - Send 4 IP packet with FC"
			"and expect 4 aggregated RNDIS packets.";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules() {
		return AddRulesAggByteLimit(true);
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic() {
		/*The packets that will be sent*/

		Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
		/*Buffer for the packets that will be received*/
		Byte pReceivedPacket[NUM_PACKETS][MAX_PACKET_SIZE];
		/*Total size of all sent packets
		 * (this is the max size of the aggregated
		 *packet minus the size of the header and the NDP)
		 */
		uint32_t nIPv4DSTAddr;
		size_t pIpPacketsSizes[NUM_PACKETS];
		size_t ExpectedPacketSize =
			sizeof(struct RndisEtherHeader) + MAX_PACKET_SIZE / NUM_PACKETS + 1;

		for (int i = 0; i < NUM_PACKETS; i++) {
			/*
			 *initialize the packets
			 *Load input data (IP packet) from file
			 */
			pIpPacketsSizes[i] = MAX_PACKET_SIZE;
			if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i])) {
				LOG_MSG_ERROR("Failed to load Ethernet Packet");
				return false;
			}
			nIPv4DSTAddr = ntohl(0x7F000001);
			memcpy(&pPackets[i][IPV4_DST_ADDR_OFFSET], &nIPv4DSTAddr,
				   sizeof(nIPv4DSTAddr));
			for (int j = pIpPacketsSizes[i]; j < MAX_PACKET_SIZE / NUM_PACKETS + 1; j++) {
				pPackets[i][j] = j & 0xFF;
			}
			pIpPacketsSizes[i] = MAX_PACKET_SIZE / NUM_PACKETS + 1;

			/* send the packet */
			LOG_MSG_DEBUG("Sending packet into the A2 TETH pipe(%d bytes)\n",
						  pIpPacketsSizes[i]);
			size_t nBytesSent = m_HsicToIpaPipe.Send(pPackets[i], pIpPacketsSizes[i]);
			if (pIpPacketsSizes[i] != nBytesSent) {
				LOG_MSG_ERROR("Sending packet into the USB pipe(%d bytes) "
							  "failed!\n", pIpPacketsSizes[i]);
				return false;
			}
		}

		/* receive the packet */
		LOG_MSG_DEBUG(
			"Reading packets from the USB pipe(%d bytes for each)"
			"\n", ExpectedPacketSize);
		for (int i = 0; i < NUM_PACKETS; i++) {
			size_t nBytesReceived = m_IpaToUsbPipeAgg
				.Receive(pReceivedPacket[i], MAX_PACKET_SIZE);
			if (ExpectedPacketSize != nBytesReceived) {
				LOG_MSG_ERROR(
					"Receiving aggregated packet from the USB pipe(%d bytes) "
					"failed!\n", nBytesReceived);
				print_buff(pReceivedPacket[i], nBytesReceived);
				return false;
			}
			print_buff(pReceivedPacket, nBytesReceived);
		}


		for (int i = 0; i < NUM_PACKETS; i++) {
			if (!RNDISAggregationHelper::CompareIPvsRNDISPacket(
					pPackets[i],
					pIpPacketsSizes[i],
					pReceivedPacket[i],
					ExpectedPacketSize)) return false;
		}

		return true;
	}
};

class RNDISAggregationDualDpTestFC : public RNDISAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	RNDISAggregationDualDpTestFC() {
		m_name = "RNDISAggregationDualDpTestFC";
		m_description = "RNDISAggregationDualDpTestFC - Send IP packets "
			"on two datapathes: one with FC and one without. "
			"Expect 2 aggregated RNDIS packets on pipe with FC. "
			"Expect one aggregated RNDIS packet on pipe without FC. ";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules() {
		return AddRulesAggDualFC();
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic() {
		int i;
		/* The packets that will be sent */
		Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
		/* Buffer for the packets that will be received */
		Byte pReceivedPacket[(NUM_PACKETS / 2) * MAX_PACKET_SIZE];
		Byte pReceivedPacketFC[NUM_PACKETS / 2][MAX_PACKET_SIZE];
		/*
		 * Total size of all sent packets
		 * (this is the max size of the aggregated
		 * packet minus the size of the header and the NDP)
		 */
		uint32_t nIPv4DSTAddr;
		size_t nBytesReceived;
		size_t pIpPacketsSizes[NUM_PACKETS];
		size_t ExpectedPacketSize =
			sizeof(struct RndisEtherHeader) + MAX_PACKET_SIZE / NUM_PACKETS + 1;

		for (i = 0; i < NUM_PACKETS; i++) {
			/*
			 * Initialize the packets
			 * Load input data (IP packet) from file
			 */
			pIpPacketsSizes[i] = MAX_PACKET_SIZE;
			if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i])) {
				LOG_MSG_ERROR("Failed to load Ethernet Packet");
				return false;
			}
			/*
			 * Half of the packets will go to 127.0.0.1
			 * and the other half to 127.0.0.2
			 */
			nIPv4DSTAddr = ntohl(0x7F000001 + (i & 0x1));
			memcpy(&pPackets[i][IPV4_DST_ADDR_OFFSET], &nIPv4DSTAddr,
				   sizeof(nIPv4DSTAddr));
			for (int j = pIpPacketsSizes[i]; j < MAX_PACKET_SIZE / NUM_PACKETS + 1; j++) {
				pPackets[i][j] = j & 0xFF;
			}
			pIpPacketsSizes[i] = MAX_PACKET_SIZE / NUM_PACKETS + 1;

			/* send the packet */
			LOG_MSG_DEBUG("Sending packet into the m_HsicToIpaPipe pipe (%d bytes)\n",
						  pIpPacketsSizes[i]);
			print_buff(pPackets[i], pIpPacketsSizes[i]);
			size_t nBytesSent = m_HsicToIpaPipe.Send(pPackets[i], pIpPacketsSizes[i]);
			if (pIpPacketsSizes[i] != nBytesSent) {
				LOG_MSG_ERROR("Sending packet into the m_HsicToIpaPipe pipe (%d bytes) "
							  "failed!\n", pIpPacketsSizes[i]);
				return false;
			}
		}

		/* receive the packets from FC pipe */
		LOG_MSG_DEBUG(
			"Reading packets from the m_IpaToUsbPipeAgg pipe (%d bytes for each)"
			"\n", ExpectedPacketSize);
		for (i = 0; i < NUM_PACKETS / 2; i++) {
			nBytesReceived = m_IpaToUsbPipeAgg
				.Receive(pReceivedPacketFC[i], MAX_PACKET_SIZE);
			if (ExpectedPacketSize != nBytesReceived) {
				LOG_MSG_ERROR(
					"Receiving aggregated packet from the m_IpaToUsbPipeAgg pipe (%d bytes) "
					"failed!\n", nBytesReceived);
				print_buff(pReceivedPacketFC[i], nBytesReceived);
				return false;
			}
		}

		for (i = 0; i < NUM_PACKETS / 2; i++) {
			if (!RNDISAggregationHelper::CompareIPvsRNDISPacket(
					pPackets[i * 2],
					pIpPacketsSizes[i * 2],
					pReceivedPacketFC[i],
					ExpectedPacketSize)) return false;
		}

		/* receive the packet from non-FC pipe */
		LOG_MSG_DEBUG(
			"Reading packet from the m_IpaToUsbPipeAggPktLimit pipe (%d bytes)"
			"\n", ExpectedPacketSize * 2);
		nBytesReceived = m_IpaToUsbPipeAggPktLimit
			.Receive(pReceivedPacket, MAX_PACKET_SIZE);
		if (ExpectedPacketSize * 2 != nBytesReceived) {
			LOG_MSG_ERROR(
				"Receiving aggregated packets from the m_IpaToUsbPipeAggPktLimit pipe (%d bytes) "
				"failed!\n", nBytesReceived);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}

		for (int i = 0; i < NUM_PACKETS; i += 2) {
			if (!RNDISAggregationHelper::CompareIPvsRNDISPacket(
					pPackets[i + 1], // Odd packets are in the second pipe
					pIpPacketsSizes[i + 1],
					pReceivedPacket + (i * ExpectedPacketSize / 2),
					ExpectedPacketSize))
				return false;
		}

		return true;
	}
};

class RNDISAggregationDualDpTestFcRoutingBased : public RNDISAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	RNDISAggregationDualDpTestFcRoutingBased() {
		m_name = "RNDISAggregationDualDpTestFcRoutingBased";
		m_description = "RNDISAggregationDualDpTestFcRoutingBased - Send IP packets "
			"on two datapathes: one with RT based FC and one without. "
			"Expect 2 aggregated RNDIS packets on pipe with RT based FC. "
			"Expect one aggregated RNDIS packet on pipe without RT based FC. ";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules() {
		return AddRulesAggDualFcRoutingBased();
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic() {
		int i;
		/* The packets that will be sent */
		Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
		/* Buffer for the packets that will be received */
		Byte pReceivedPacket[(NUM_PACKETS / 2) * MAX_PACKET_SIZE];
		Byte pReceivedPacketFC[NUM_PACKETS / 2][MAX_PACKET_SIZE];
		/*
		 * Total size of all sent packets
		 * (this is the max size of the aggregated
		 * packet minus the size of the header and the NDP)
		 */
		uint32_t nIPv4DSTAddr;
		size_t nBytesReceived;
		size_t pIpPacketsSizes[NUM_PACKETS];
		size_t ExpectedPacketSize =
			sizeof(struct RndisEtherHeader) + MAX_PACKET_SIZE / NUM_PACKETS + 1;

		for (i = 0; i < NUM_PACKETS; i++) {
			/*
			 * Initialize the packets
			 * Load input data (IP packet) from file
			 */
			pIpPacketsSizes[i] = MAX_PACKET_SIZE;
			if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i])) {
				LOG_MSG_ERROR("Failed to load Ethernet Packet");
				return false;
			}
			/*
			 * Half of the packets will go to 127.0.0.1
			 * and the other half to 127.0.0.2
			 */
			nIPv4DSTAddr = ntohl(0x7F000001 + (i & 0x1));
			memcpy(&pPackets[i][IPV4_DST_ADDR_OFFSET], &nIPv4DSTAddr,
				   sizeof(nIPv4DSTAddr));
			for (int j = pIpPacketsSizes[i]; j < MAX_PACKET_SIZE / NUM_PACKETS + 1; j++) {
				pPackets[i][j] = j & 0xFF;
			}
			pIpPacketsSizes[i] = MAX_PACKET_SIZE / NUM_PACKETS + 1;

			/* send the packet */
			LOG_MSG_DEBUG("Sending packet into the m_HsicToIpaPipe pipe (%d bytes)\n",
						  pIpPacketsSizes[i]);
			print_buff(pPackets[i], pIpPacketsSizes[i]);
			size_t nBytesSent = m_HsicToIpaPipe.Send(pPackets[i], pIpPacketsSizes[i]);
			if (pIpPacketsSizes[i] != nBytesSent) {
				LOG_MSG_ERROR("Sending packet into the m_HsicToIpaPipe pipe (%d bytes) "
							  "failed!\n", pIpPacketsSizes[i]);
				return false;
			}
		}

		/* receive the packets from FC pipe */
		LOG_MSG_DEBUG(
			"Reading packets from the m_IpaToUsbPipeAgg pipe (%d bytes for each)"
			"\n", ExpectedPacketSize);
		for (i = 0; i < NUM_PACKETS / 2; i++) {
			nBytesReceived = m_IpaToUsbPipeAgg
				.Receive(pReceivedPacketFC[i], MAX_PACKET_SIZE);
			if (ExpectedPacketSize != nBytesReceived) {
				LOG_MSG_ERROR(
					"Receiving aggregated packet from the m_IpaToUsbPipeAgg pipe (%d bytes) "
					"failed!\n", nBytesReceived);
				print_buff(pReceivedPacketFC[i], nBytesReceived);
				return false;
			}
		}

		for (i = 0; i < NUM_PACKETS / 2; i++) {
			if (!RNDISAggregationHelper::CompareIPvsRNDISPacket(
					pPackets[i * 2],
					pIpPacketsSizes[i * 2],
					pReceivedPacketFC[i],
					ExpectedPacketSize)) return false;
		}

		/* receive the packet from non-FC pipe */
		LOG_MSG_DEBUG(
			"Reading packet from the m_IpaToUsbPipeAggPktLimit pipe (%d bytes)"
			"\n", ExpectedPacketSize * 2);
		nBytesReceived = m_IpaToUsbPipeAggPktLimit
			.Receive(pReceivedPacket, MAX_PACKET_SIZE);
		if (ExpectedPacketSize * 2 != nBytesReceived) {
			LOG_MSG_ERROR(
				"Receiving aggregated packets from the m_IpaToUsbPipeAggPktLimit pipe (%d bytes) "
				"failed!\n", nBytesReceived);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}

		for (int i = 0; i < NUM_PACKETS; i += 2) {
			if (!RNDISAggregationHelper::CompareIPvsRNDISPacket(
					pPackets[i + 1], // Odd packets are in the second pipe
					pIpPacketsSizes[i + 1],
					pReceivedPacket + (i * ExpectedPacketSize / 2),
					ExpectedPacketSize))
				return false;
		}

		return true;
	}
};

class RNDISAggregationDeaggregationNumPacketsTest:
	public RNDISAggregationTestFixture {
public:

	RNDISAggregationDeaggregationNumPacketsTest()
	{
		m_name = "RNDISAggregationDeaggregationNumPacketsTest";
		m_description = "RNDISAggregationByteLimitTest - Send on IP packet "
			"and expect aggregated RNDIS packet.";
	}

	virtual bool AddRules()
	{
		return AddRulesDeAggEther();
	} /* AddRules()*/

	bool TestLogic()
	{
		/*the packets that will be sent*/
		Byte pPacket[MAX_PACKET_SIZE];
		//Buffer for the packet that will be received
		Byte pReceivedPacket[2*MAX_PACKET_SIZE];
		//Total size of all sent packets (this is the max size of the aggregated
		//packet minus the size of the header and the NDP)
		uint32_t nIPv4DSTAddr;
		size_t pIpPacketsSize;
		size_t pAggrPacketsSize = 0;

		for(int i = 0; i < NUM_PACKETS; i++) {
			//initialize the packets
			// Load input data (RNDIS packet) from file
			pIpPacketsSize = MAX_PACKET_SIZE;
			if (!RNDISAggregationHelper::LoadRNDISPacket(m_eIP, pPacket + pAggrPacketsSize, pIpPacketsSize))
			{
				LOG_MSG_ERROR("Failed to load Ethernet Packet");
				return false;
			}
			pAggrPacketsSize += pIpPacketsSize;
			nIPv4DSTAddr = ntohl(0x7F000001);
			memcpy (&((pPacket + i * pIpPacketsSize)[IPV4_DST_ADDR_OFFSET_IN_RNDIS]),&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		}
		print_buff(pPacket, pAggrPacketsSize);

		/*send the packet*/
		LOG_MSG_DEBUG("Sending packet into the A2 TETH pipe(%d bytes)\n",
					pIpPacketsSize * NUM_PACKETS);
		size_t nBytesSent = m_UsbToIpaPipeDeagg.Send(pPacket, pAggrPacketsSize);
		if (pAggrPacketsSize != nBytesSent)
		{
			LOG_MSG_ERROR("Sending packet into the USB pipe(%d bytes) "
				"failed!\n", pIpPacketsSize * NUM_PACKETS);
			return false;
		}
		for(int i = 0; i < NUM_PACKETS; i++) {
			//receive the packet, one by one
			LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be there)"
				"\n", pIpPacketsSize - sizeof(struct RndisHeader));
			size_t nBytesReceived = m_IpaToUsbPipe.Receive(pReceivedPacket, MAX_PACKET_SIZE);
			if (pIpPacketsSize - sizeof(struct RndisHeader) != nBytesReceived)
			{
				LOG_MSG_ERROR("Receiving aggregated packet from the USB pipe(%d bytes) "
					"failed!\n", nBytesReceived);
				print_buff(pReceivedPacket, nBytesReceived);
				return false;
			}
			if (!RNDISAggregationHelper::CompareEthervsRNDISPacket(pReceivedPacket,
										nBytesReceived,
										pPacket + i * pIpPacketsSize,
									       pIpPacketsSize))
				return false;
		}

		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////
};

class RNDISAggregationDeaggregationExceptionPacketsTest:
	public RNDISAggregationTestFixture {
public:

	RNDISAggregationDeaggregationExceptionPacketsTest()
	{
		m_name = "RNDISAggregationDeaggregationExceptionPacketsTest";
		m_description = "RNDISAggregationDeaggregationExceptionPacketsTest - Send 5 frames "
			"of size 43 bytes, 1025 bytes, 43 bytes, 981 bytes, and 1024 bytes "
			"and expect aggregated RNDIS packet.";
	}

	virtual bool AddRules()
	{
		return AddRulesDeAggEther();
	} /* AddRules()*/

	bool TestLogic()
	{
		/*the packets that will be sent*/
		Byte pPacket[MAX_PACKET_SIZE];
		Byte pPacket1[MAX_PACKET_SIZE +1];
		Byte pPacket2[MAX_PACKET_SIZE];
		Byte pPacket3[MAX_PACKET_SIZE];

		//Buffer for the packet that will be received
		Byte pReceivedPacket[2*MAX_PACKET_SIZE];
		//Total size of all sent packets (this is the max size of the aggregated
		//packet minus the size of the header and the NDP)
		uint32_t nIPv4DSTAddr;
		size_t pIpPacketsSize;
		size_t pAggrPacketsSize = 0;
		size_t nBytesSent;
		size_t nBytesReceived;

		/* Create the frame of size 43 bytes which is one less byte than RNDIS header */
		pAggrPacketsSize  = sizeof(struct RndisHeader) - 1;
		struct RndisHeader *pRndisHeader = (struct RndisHeader*)pPacket;
		memset(pRndisHeader, 0, (sizeof(struct RndisHeader) - 1));
		pRndisHeader->MessageType = 0x01;
		pRndisHeader->MessageLength = pAggrPacketsSize;
		pRndisHeader->DataOffset = 0x24;
		pRndisHeader->DataLength = 0;

		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&((pPacket)[IPV4_DST_ADDR_OFFSET_IN_RNDIS]),&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));
		print_buff(pPacket, pAggrPacketsSize);

		/* Send the first frame */
		LOG_MSG_DEBUG("Sending packet into the A2 TETH pipe(%d bytes)\n",
					pAggrPacketsSize);
		nBytesSent = m_UsbToIpaPipeDeagg.Send(pPacket, pAggrPacketsSize);
		if (pAggrPacketsSize != nBytesSent)
		{
			LOG_MSG_ERROR("Sending packet into the USB pipe(%d bytes) "
					"failed!\n", pAggrPacketsSize);
			return false;
		}
		/* This is deaggregation exception packet, this packet should not arrive at this pipe */
		LOG_MSG_DEBUG("Reading packet from the USB pipe(0 bytes should be there)\n");
		nBytesReceived = m_IpaToUsbPipe.Receive(pReceivedPacket, MAX_PACKET_SIZE);
		if (0 != nBytesReceived)
		{
			LOG_MSG_ERROR("Receiving aggregated packet from the USB pipe(%d bytes) "
					"failed!\n", nBytesReceived);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}

		/* Create a frame of size 1025 bytes */
		pAggrPacketsSize = 0;
		for(int i = 0; i < 8; i++) {
			//initialize the packets
			// Load input data (RNDIS packet) from file
			pIpPacketsSize = MAX_PACKET_SIZE;
			if (!RNDISAggregationHelper::LoadRNDISPacket(m_eIP, pPacket1 + pAggrPacketsSize, pIpPacketsSize))
			{
				LOG_MSG_ERROR("Failed to load Ethernet Packet");
				return false;
			}
			pAggrPacketsSize += pIpPacketsSize;
			nIPv4DSTAddr = ntohl(0x7F000001);
			memcpy (&((pPacket1 + i * pIpPacketsSize)[IPV4_DST_ADDR_OFFSET_IN_RNDIS]),&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		}

		pPacket1[pAggrPacketsSize] = 0xdd;
		pAggrPacketsSize = pAggrPacketsSize + 1;

                print_buff(pPacket1, pAggrPacketsSize);

                /* Send the 2nd frame */
		LOG_MSG_DEBUG("Sending packet into the A2 TETH pipe(%d bytes)\n",
							pAggrPacketsSize);
		nBytesSent = m_UsbToIpaPipeDeagg.Send(pPacket1, pAggrPacketsSize);
		if (pAggrPacketsSize  != nBytesSent)
		{
			LOG_MSG_ERROR("Sending packet into the USB pipe(%d bytes) "
					"failed!\n", pAggrPacketsSize);
			return false;
		}

		for(int i = 0; i < 8; i++) {
			//Receive the packet, one by one
			LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be there)"
				"\n", pIpPacketsSize - sizeof(struct RndisHeader));
			size_t nBytesReceived = m_IpaToUsbPipe.Receive(pReceivedPacket, MAX_PACKET_SIZE);
			if (pIpPacketsSize - sizeof(struct RndisHeader) != nBytesReceived)
			{
				LOG_MSG_ERROR("Receiving aggregated packet from the USB pipe(%d bytes) "
					"failed!\n", nBytesReceived);
				print_buff(pReceivedPacket, nBytesReceived);
				return false;
			}
			if (!RNDISAggregationHelper::CompareEthervsRNDISPacket(pReceivedPacket,
										nBytesReceived,
										pPacket1 + i * pIpPacketsSize,
										pIpPacketsSize))
				return false;
		}

		/* Create a frame of size 1024 bytes and send as 2 frames */
		pAggrPacketsSize = 0;
		for(int i = 0; i < 8; i++) {
			//initialize the packets
			// Load input data (RNDIS packet) from file
			pIpPacketsSize = MAX_PACKET_SIZE;
			if (!RNDISAggregationHelper::LoadRNDISPacket(m_eIP, pPacket2 + pAggrPacketsSize, pIpPacketsSize))
			{
				LOG_MSG_ERROR("Failed to load Ethernet Packet");
				return false;
			}
			pAggrPacketsSize += pIpPacketsSize;
			nIPv4DSTAddr = ntohl(0x7F000001);
			memcpy (&((pPacket2 + i * pIpPacketsSize)[IPV4_DST_ADDR_OFFSET_IN_RNDIS]),&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		}
		print_buff(pPacket2, pAggrPacketsSize);

		/* Send the 3rd frame */
		LOG_MSG_DEBUG("Sending packet into the A2 TETH pipe(%d bytes)\n", 43);
		nBytesSent = m_UsbToIpaPipeDeagg.Send(pPacket2, 43);
		if (43 != nBytesSent)
		{
			LOG_MSG_ERROR("Sending packet into the USB pipe(%d bytes) "
				"failed!\n", 43);

			return false;
		}
		/* This is deaggregation exception packet, this packet should not arrive at this pipe */
		LOG_MSG_DEBUG("Reading packet from the USB pipe(0 bytes should be there)\n");
		nBytesReceived = m_IpaToUsbPipe.Receive(pReceivedPacket, MAX_PACKET_SIZE);
		if (0 != nBytesReceived)
		{
			LOG_MSG_ERROR("Receiving aggregated packet from the USB pipe(%d bytes) "
					"failed!\n", nBytesReceived);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}
		/* Send the 4rd frame */
		LOG_MSG_DEBUG("Sending packet into the A2 TETH pipe(%d bytes)\n",
						pAggrPacketsSize - 43 );
		nBytesSent = m_UsbToIpaPipeDeagg.Send((pPacket2 + 43), pAggrPacketsSize - 43);
		if ((pAggrPacketsSize - 43) != nBytesSent)
		{
			LOG_MSG_ERROR("Sending packet into the USB pipe(%d bytes) "
					"failed!\n", pAggrPacketsSize - 43);
			return false;
		}
		/* This is deaggregation exception packet, this packet should not arrive at this pipe */
		LOG_MSG_DEBUG("Reading packet from the USB pipe(0 bytes should be there)\n");
		nBytesReceived = m_IpaToUsbPipe.Receive(pReceivedPacket, MAX_PACKET_SIZE);
		if (0 != nBytesReceived)
		{
			LOG_MSG_ERROR("Receiving aggregated packet from the USB pipe(%d bytes) "
					"failed!\n", nBytesReceived);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}

		/* Create a frame of size 1024 bytes */
		pAggrPacketsSize = 0;
		for(int i = 0; i < 8; i++) {
			//initialize the packets
			//Load input data (RNDIS packet) from file
			pIpPacketsSize = MAX_PACKET_SIZE;
			if (!RNDISAggregationHelper::LoadRNDISPacket(m_eIP, pPacket3 + pAggrPacketsSize, pIpPacketsSize))
			{
				LOG_MSG_ERROR("Failed to load Ethernet Packet");
				return false;
			}
			pAggrPacketsSize += pIpPacketsSize;
			nIPv4DSTAddr = ntohl(0x7F000001);
			memcpy (&((pPacket3 + i * pIpPacketsSize)[IPV4_DST_ADDR_OFFSET_IN_RNDIS]),&nIPv4DSTAddr,
					sizeof(nIPv4DSTAddr));
		}
		print_buff(pPacket3, pAggrPacketsSize);

		/* Send the 5th frame */
		LOG_MSG_ERROR("blend-3 Sending packet into the A2 TETH pipe(%d bytes)\n",
						pAggrPacketsSize);
		nBytesSent = m_UsbToIpaPipeDeagg.Send(pPacket3, pAggrPacketsSize);
		if (pAggrPacketsSize  != nBytesSent)
		{
			LOG_MSG_ERROR("Sending packet into the USB pipe(%d bytes) "
				"failed!\n", pAggrPacketsSize);
			return false;
		}

		for(int i = 0; i < 8; i++) {
			//Receive the packet, one by one
			LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be there)"
					"\n", pIpPacketsSize - sizeof(struct RndisHeader));
			size_t nBytesReceived = m_IpaToUsbPipe.Receive(pReceivedPacket, MAX_PACKET_SIZE);
			if (pIpPacketsSize - sizeof(struct RndisHeader) != nBytesReceived)
			{
				LOG_MSG_ERROR("Receiving aggregated packet from the USB pipe(%d bytes) "
						"failed!\n", nBytesReceived);
				print_buff(pReceivedPacket, nBytesReceived);
				return false;
			}
			if (!RNDISAggregationHelper::CompareEthervsRNDISPacket(pReceivedPacket,
										nBytesReceived,
										pPacket3 + i * pIpPacketsSize,
										pIpPacketsSize))
				return false;
		}

		return true;
	}

};

class RNDISAggregationPacketLimitTest: public RNDISAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	RNDISAggregationPacketLimitTest()
	{
		m_name = "RNDISAggregationPacketLimitTest";
		m_description = "RNDISAggregationPacketLimitTest - Send 2 IP packet "
			"and expect aggregated RNDIS packet.";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRulesAggPacketLimit();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		//The packets that will be sent

		Byte pPackets[2][MAX_PACKET_SIZE];
		//Buffer for the packet that will be received
		Byte pReceivedPacket[2*MAX_PACKET_SIZE];
		//Total size of all sent packets (this is the max size of the aggregated
		//packet minus the size of the header and the NDP)
		uint32_t nIPv4DSTAddr;
		size_t pIpPacketsSizes[2];
		size_t ExpectedPacketSize = 2 * sizeof(struct RndisEtherHeader);

		for(int i = 0; i < 2; i++) {
			//initialize the packets
			// Load input data (IP packet) from file
			pIpPacketsSizes[i] = MAX_PACKET_SIZE;
			if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
			{
				LOG_MSG_ERROR("Failed to load Ethernet Packet");
				return false;
			}
			nIPv4DSTAddr = ntohl(0x7F000001);
			memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));

			//send the packet
			LOG_MSG_DEBUG("Sending packet into the A2 TETH pipe(%d bytes)\n",
				pIpPacketsSizes[i]);
			size_t nBytesSent = m_HsicToIpaPipe.Send(pPackets[i], pIpPacketsSizes[i]);
			if (pIpPacketsSizes[i] != nBytesSent)
			{
				LOG_MSG_ERROR("Sending packet into the USB pipe(%d bytes) "
					"failed!\n", pIpPacketsSizes[i]);
				return false;
			}
			ExpectedPacketSize += pIpPacketsSizes[i];
		}

		//receive the packet
		LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be there)"
			"\n", ExpectedPacketSize);
		size_t nBytesReceived = m_IpaToUsbPipeAggPktLimit.Receive(pReceivedPacket, MAX_PACKET_SIZE);
		if (ExpectedPacketSize != nBytesReceived)
		{
			LOG_MSG_ERROR("Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", nBytesReceived);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}


		for(int i = 0; i < 2; i++) {
			if (!RNDISAggregationHelper::CompareIPvsRNDISPacket(pPackets[i], pIpPacketsSizes[i],
				pReceivedPacket + (i * ExpectedPacketSize / 2), ExpectedPacketSize / 2))
				return false;
		}

		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////
};



static RNDISAggregationSanityTest aRNDISAggregationSanityTest;
static RNDISAggregationDeaggregation1PacketTest aRNDISAggregationDeaggregation1PacketTest;
static RNDISAggregation1PacketTest aRNDISAggregation1PacketTest;
static RNDISAggregationSuspendWaTest aRNDISAggregationSuspendWaTest;
static RNDISAggregationByteLimitTest aRNDISAggregationByteLimitTest;
static RNDISAggregationByteLimitTestFC aRNDISAggregationByteLimitTestFC;
static RNDISAggregationDualDpTestFC aRNDISAggregationDualDpTestFC;
static RNDISAggregationDualDpTestFcRoutingBased aRNDISAggregationDualDpTestFcRoutingBased;
static RNDISAggregationDeaggregationNumPacketsTest aRNDISAggregationDeaggregationNumPacketsTest;
static RNDISAggregationDeaggregationExceptionPacketsTest aRNDISAggregationDeaggregationExceptionPacketsTest;
static RNDISAggregationPacketLimitTest aRNDISAggregationPacketLimitTest;

/////////////////////////////////////////////////////////////////////////////////
//                                  EOF                                      ////
/////////////////////////////////////////////////////////////////////////////////
