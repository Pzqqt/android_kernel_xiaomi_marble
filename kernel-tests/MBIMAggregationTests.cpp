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
#include "hton.h" // for htonl
#include "MBIMAggregationTestFixtureConf11.h"
#include "Constants.h"
#include "TestsUtils.h"
#include "linux/msm_ipa.h"

#define AGGREGATION_LOOP 4
#define IPV4_DST_ADDR_OFFSET (16)

/////////////////////////////////////////////////////////////////////////////////
//							MBIM Aggregation scenarios                         //
/////////////////////////////////////////////////////////////////////////////////

class MBIMAggregationScenarios {
public:
	//MBIM Aggregation test - sends 5 packets and receives 1 aggregated packet
	static bool MBIMAggregationTest(Pipe* input, Pipe* output, enum ipa_ip_type m_eIP);
	//MBIM Deaggregation test - sends an aggregated packet made from 5 packets
	//and receives 5 packets
	static bool MBIMDeaggregationTest(Pipe* input,
			Pipe* output, enum ipa_ip_type m_eIP);
	//MBIM Deaggregation one packet test - sends an aggregated packet made from
	//1 packet and receives 1 packet
	static bool MBIMDeaggregationOnePacketTest(Pipe* input, Pipe* output, enum ipa_ip_type m_eIP);
	//MBIM Deaggregation and Aggregation test - sends an aggregated packet made
	//from 5 packets and receives the same aggregated packet
	static bool MBIMDeaggregationAndAggregationTest(Pipe* input, Pipe* output, enum ipa_ip_type m_eIP);
	//MBIM multiple Deaggregation and Aggregation test - sends 5 aggregated
	//packets each one made of 1 packet and receives an aggregated packet made
	//of the 5 packets
	static bool MBIMMultipleDeaggregationAndAggregationTest(
			Pipe* input, Pipe* output,
			enum ipa_ip_type m_eIP);
	//MBIM Aggregation Loop test - sends 5 packets and expects to receive 1
	//aggregated packet a few times
	static bool MBIMAggregationLoopTest(Pipe* input,
			Pipe* output, enum ipa_ip_type m_eIP);
	//MBIM Aggregation time limit test - sends 1 small packet smaller than the
	//byte limit and receives 1 aggregated packet
	static bool MBIMAggregationTimeLimitTest(Pipe* input, Pipe* output, enum ipa_ip_type m_eIP);
	//MBIM Aggregation byte limit test - sends 2 packets that together are
	//larger than the byte limit
	static bool MBIMAggregationByteLimitTest(Pipe* input, Pipe* output, enum ipa_ip_type m_eIP);
	static bool MBIMAggregationByteLimitTestFC(Pipe* input, Pipe* output, enum ipa_ip_type m_eIP);
	static bool MBIMAggregationDualDpTestFC(Pipe* input, Pipe* output1, Pipe* output2, enum ipa_ip_type m_eIP);
	//MBIM Deaggregation multiple NDP test - sends an aggregated packet made
	//from 5 packets and 2 NDPs and receives 5 packets
	static bool MBIMDeaggregationMultipleNDPTest(Pipe* input, Pipe* output, enum ipa_ip_type m_eIP);
	//MBIM Aggregation 2 pipes test - sends 3 packets from one pipe and an
	//aggregated packet made of 2 packets from another pipe and receives 1
	//aggregated packet made of all 5 packets
	static bool MBIMAggregation2PipesTest(Pipe* input1, Pipe* input2, Pipe* output, enum ipa_ip_type m_eIP);
	//MBIM Aggregation time limit loop test - sends 5 small packet smaller than
	//the byte limit and receives 5 aggregated packet
	static bool MBIMAggregationTimeLimitLoopTest(Pipe* input, Pipe* output, enum ipa_ip_type m_eIP);
	//MBIM Aggregation 0 limits test - sends 5 packets and expects to get each
	//packet back aggregated (both size and time limits are 0)
	static bool MBIMAggregation0LimitsTest(Pipe* input, Pipe* output, enum ipa_ip_type m_eIP);
	//MBIM Aggregation multiple packets test - sends 9 packets with same stream
	//ID and receives 1 aggregated packet with 2 NDPs
	static bool MBIMAggregationMultiplePacketsTest(Pipe* input, Pipe* output, enum ipa_ip_type m_eIP);
	//MBIM Aggregation different stream IDs test - sends 5 packets with
	//different stream IDs and receives 1 aggregated packet made of 5 NDPs
	static bool MBIMAggregationDifferentStreamIdsTest(Pipe* input, Pipe* output, enum ipa_ip_type m_eIP);
	//MBIM Aggregation no interleaving stream IDs test - sends 5 packets with
	//interleaving stream IDs (0, 1, 0, 1, 0) and receives 1 aggregated packet
	//made of 5 NDPs
	static bool MBIMAggregationNoInterleavingStreamIdsTest(
			Pipe* input, Pipe* output,
			enum ipa_ip_type m_eIP);

private:
	//This method will deaggregate an aggregated packet and compare the packets
	//to the expected packets
	static bool DeaggragateAndComparePackets(
			Byte pAggregatedPacket[MAX_PACKET_SIZE],
			Byte pExpectedPackets[MAX_PACKETS_IN_MBIM_TESTS][MAX_PACKET_SIZE],
			int pPacketsSizes[MAX_PACKETS_IN_MBIM_TESTS], int nNumPackets,
			int nAggregatedPacketSize);
	//This method will aggregate packets
	static void AggregatePackets(
			Byte pAggregatedPacket[MAX_PACKET_SIZE]/*ouput*/,
			Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE],
			int pPacketsSizes[NUM_PACKETS], int nNumPackets,
			int nAggregatedPacketSize);
	//This method will aggregate packets and take into consideration their
	//stream id to seperate them into different NDPs
	static void AggregatePacketsWithStreamId(
			Byte pAggregatedPacket[MAX_PACKET_SIZE]/*ouput*/,
			Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE],
			int pPacketsSizes[NUM_PACKETS], int nNumPackets,
			int nAggregatedPacketSize, Byte pPacketsStreamId[NUM_PACKETS]);
	//This method will deaggregate an aggregated packet made of one packet and
	//compare the packet to the expected packet
	static bool DeaggragateAndCompareOnePacket(
			Byte pAggregatedPacket[MAX_PACKET_SIZE],
			Byte pExpectedPacket[MAX_PACKET_SIZE], int nPacketsSize,
			int nAggregatedPacketSize);
	//This method will deaggregate an aggregated packet and compare the packets
	//to the expected packets
	static bool DeaggragateAndComparePacketsWithStreamId(
			Byte pAggregatedPacket[MAX_PACKET_SIZE],
			Byte pExpectedPackets[][MAX_PACKET_SIZE], int pPacketsSizes[],
			int nNumPackets, int nAggregatedPacketSize,
			Byte pPacketsStreamId[NUM_PACKETS]);
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMAggregationTest(
		Pipe* input, Pipe* output, enum ipa_ip_type m_eIP)
{
	//The packets that will be sent
	Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//The real sizes of the packets that will be sent
	int pPacketsSizes[NUM_PACKETS];
	//Buffer for the packet that will be received
	Byte pReceivedPacket[2*MAX_PACKET_SIZE];
	//Total size of all sent packets (this is the max size of the aggregated
	//packet minus the size of the header and the NDP)
	int nTotalPacketsSize = MAX_PACKET_SIZE - (4 * NUM_PACKETS) - 24;
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[NUM_PACKETS];

	//initialize the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		if (NUM_PACKETS - 1 == i)
			pPacketsSizes[i] = nTotalPacketsSize;
		else
			pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
		while (0 != pPacketsSizes[i] % 4)
		{
			pPacketsSizes[i]++;
		}
		nTotalPacketsSize -= pPacketsSizes[i];

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}

	}

	//send the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
					pPacketsSizes[i]);
		int nBytesSent = input->Send(pPackets[i], pPacketsSizes[i]);
		if (pPacketsSizes[i] != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			return false;
		}
	}

	//receive the aggregated packet
	LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be there)"
			"\n", MAX_PACKET_SIZE);
	int nBytesReceived = output->Receive(pReceivedPacket, MAX_PACKET_SIZE);
	if (MAX_PACKET_SIZE != nBytesReceived)
	{
		LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", MAX_PACKET_SIZE);
		print_buff(pReceivedPacket, nBytesReceived);
		return false;
	}
	//deaggregating the aggregated packet
	return DeaggragateAndComparePackets(pReceivedPacket, pPackets,
			pPacketsSizes, NUM_PACKETS, nBytesReceived);
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMDeaggregationTest(
		Pipe* input, Pipe* output, enum ipa_ip_type m_eIP)
{
	bool bTestResult = true;
	//The packets that the aggregated packet will be made of
	Byte pExpectedPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//The real sizes of the packets that the aggregated packet will be made of
	int pPacketsSizes[NUM_PACKETS];
	//Buffers for the packets that will be received
	Byte pReceivedPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//Total size of all the packets that the aggregated packet will be made of
	//(this is the max size of the aggregated packet
	//minus the size of the header and the NDP)
	int nTotalPacketsSize = MAX_PACKET_SIZE - (4 * NUM_PACKETS) - 24;
	//The aggregated packet that will be sent
	Byte pAggregatedPacket[MAX_PACKET_SIZE] = {0};

	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[NUM_PACKETS];

	//initialize the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		if (NUM_PACKETS - 1 == i)
			pPacketsSizes[i] = nTotalPacketsSize;
		else
			pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
		while (0 != pPacketsSizes[i] % 4)
		{
			pPacketsSizes[i]++;
		}
		nTotalPacketsSize -= pPacketsSizes[i];
		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pExpectedPackets[i],
				pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pExpectedPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pExpectedPackets[i][size] = i;
			size++;
		}
	}

	//initializing the aggregated packet
	AggregatePackets(pAggregatedPacket, pExpectedPackets, pPacketsSizes,
			NUM_PACKETS, MAX_PACKET_SIZE);

	//send the aggregated packet
	LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d bytes)\n",
			sizeof(pAggregatedPacket));
	int nBytesSent = input->Send(pAggregatedPacket, sizeof(pAggregatedPacket));
	if (sizeof(pAggregatedPacket) != nBytesSent)
	{
		LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d bytes) "
				"failed!\n", sizeof(pAggregatedPacket));
		return false;
	}

	//receive the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		LOG_MSG_DEBUG("Reading packet %d from the USB pipe(%d bytes should be "
				"there)\n", i, pPacketsSizes[i]);
		int nBytesReceived = output->Receive(pReceivedPackets[i],
				pPacketsSizes[i]);
		if (pPacketsSizes[i] != nBytesReceived)
		{
			LOG_MSG_DEBUG("Receiving packet %d from the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			print_buff(pReceivedPackets[i], nBytesReceived);
			return false;
		}
	}

	//comparing the received packet to the aggregated packet
	LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
	for (int i = 0; i < NUM_PACKETS; i++)
		bTestResult &= !memcmp(pExpectedPackets[i], pReceivedPackets[i],
				pPacketsSizes[i]);

	return bTestResult;
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMDeaggregationOnePacketTest(
		Pipe* input, Pipe* output,
		enum ipa_ip_type m_eIP)
{
	bool bTestResult = true;
	//The packets that the aggregated packet will be made of
	Byte pExpectedPackets[1][MAX_PACKET_SIZE];
	//The real sizes of the packets that the aggregated packet will be made of
	int pPacketsSizes[1] = {100};
	//Buffers for the packets that will be received
	Byte pReceivedPackets[1][MAX_PACKET_SIZE];
	//Total size of the aggregated packet
	//(this is the max size of the aggregated packet
	//minus the size of the header and the NDP)
	int nTotalAggregatedPacketSize = 100 + 12 + 16;
	//The aggregated packet that will be sent
	Byte pAggregatedPacket[MAX_PACKET_SIZE] = {0};

	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[1];

	// Load input data (IP packet) from file
	pIpPacketsSizes[0] = 100;
	if (!LoadDefaultPacket(m_eIP, pExpectedPackets[0], pIpPacketsSizes[0]))
	{
		LOG_MSG_ERROR("Failed default Packet");
		return false;
	}
	nIPv4DSTAddr = ntohl(0x7F000001);
	memcpy (&pExpectedPackets[0][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));
	int size = pIpPacketsSizes[0];
	while (size < pPacketsSizes[0])
	{
		pExpectedPackets[0][size] = 0;
		size++;
	}


	//initializing the aggregated packet
	AggregatePackets(pAggregatedPacket, pExpectedPackets, pPacketsSizes, 1,
			nTotalAggregatedPacketSize);

	//send the aggregated packet
	LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d bytes)\n",
			nTotalAggregatedPacketSize);
	int nBytesSent = input->Send(pAggregatedPacket, nTotalAggregatedPacketSize);
	if (nTotalAggregatedPacketSize != nBytesSent)
	{
		LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d bytes) "
				"failed!\n", nTotalAggregatedPacketSize);
		return false;
	}

	//receive the packet
	for (int i = 0; i < 1; i++)
	{
		LOG_MSG_DEBUG("Reading packet %d from the USB pipe(%d bytes should be "
				"there)\n", i, pPacketsSizes[i]);
		int nBytesReceived = output->Receive(pReceivedPackets[i],
				pPacketsSizes[i]);
		if (pPacketsSizes[i] != nBytesReceived)
		{
			LOG_MSG_DEBUG("Receiving packet %d from the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			print_buff(pReceivedPackets[i], nBytesReceived);
			return false;
		}
	}

	//comparing the received packet to the aggregated packet
	LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
	for (int i = 0; i < 1; i++)
		bTestResult &= !memcmp(pExpectedPackets[i], pReceivedPackets[i],
				pPacketsSizes[i]);

	return bTestResult;
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMDeaggregationAndAggregationTest(
		Pipe* input, Pipe* output,
		enum ipa_ip_type m_eIP)
{
	//The packets that the aggregated packet will be made of
	Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//The real sizes of the packets that the aggregated packet will be made of
	int pPacketsSizes[NUM_PACKETS];
	//Buffers for the packets that will be received
	Byte pReceivedPacket[MAX_PACKET_SIZE];
	//Total size of all the packets that the aggregated packet will be made of
	//(this is the max size of the aggregated packet
	//minus the size of the header and the NDP)
	int nTotalPacketsSize = MAX_PACKET_SIZE - (4 * NUM_PACKETS) - 24;
	//The aggregated packet that will be sent
	Byte pAggregatedPacket[MAX_PACKET_SIZE] = {0};
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[NUM_PACKETS];

	//initialize the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		if (NUM_PACKETS - 1 == i)
			pPacketsSizes[i] = nTotalPacketsSize;
		else
			pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
		while (0 != pPacketsSizes[i] % 4)
			pPacketsSizes[i]++;
		nTotalPacketsSize -= pPacketsSizes[i];

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}
	}

	//initializing the aggregated packet
	AggregatePackets(pAggregatedPacket, pPackets, pPacketsSizes, NUM_PACKETS,
			MAX_PACKET_SIZE);

	//send the aggregated packet
	LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d bytes)\n",
			MAX_PACKET_SIZE);
	int nBytesSent = input->Send(pAggregatedPacket, MAX_PACKET_SIZE);
	if (MAX_PACKET_SIZE != nBytesSent)
	{
		LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d bytes) "
				"failed!\n", MAX_PACKET_SIZE);
		return false;
	}

	//receive the aggregated packet
	LOG_MSG_DEBUG("Reading aggregated packet from the USB pipe(%d bytes should "
			"be there)\n", MAX_PACKET_SIZE);
	int nBytesReceived = output->Receive(pReceivedPacket, MAX_PACKET_SIZE);
	if (MAX_PACKET_SIZE != nBytesReceived)
	{
		LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", MAX_PACKET_SIZE);
		LOG_MSG_DEBUG("Received %d bytes\n", nBytesReceived);
		print_buff(pReceivedPacket, nBytesReceived);
		return false;
	}


	//comparing the received packet to the aggregated packet
	LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
	return DeaggragateAndComparePackets(pReceivedPacket, pPackets, pPacketsSizes,
			NUM_PACKETS, nBytesReceived);
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMMultipleDeaggregationAndAggregationTest(
		Pipe* input, Pipe* output,
		enum ipa_ip_type m_eIP)
{
	//The packets that the aggregated packets will be made of
	Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//The real sizes of the packets that the aggregated packet will be made of
	int pPacketsSizes[NUM_PACKETS];
	//Buffers for the packets that will be received
	Byte pReceivedPacket[MAX_PACKET_SIZE];
	//Total size of all the packets that the aggregated packet will be made of
	//(this is the max size of the aggregated packet
	//minus the size of the header and the NDP)
	int nTotalPacketsSize = MAX_PACKET_SIZE - (4 * NUM_PACKETS) - 24;
	//The aggregated packet that will be sent
	Byte pAggregatedPacket[NUM_PACKETS][MAX_PACKET_SIZE];
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[NUM_PACKETS];

	//initialize the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		if (NUM_PACKETS - 1 == i)
			pPacketsSizes[i] = nTotalPacketsSize;
		else
			pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
		while (0 != pPacketsSizes[i] % 4)
			pPacketsSizes[i]++;
		nTotalPacketsSize -= pPacketsSizes[i];

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}

	}

	//initializing the aggregated packets
	for (int i = 0; i < NUM_PACKETS; i++)
		AggregatePackets(pAggregatedPacket[i], &pPackets[i], &pPacketsSizes[i],
				1, pPacketsSizes[i] + 12 + 16);

	//send the aggregated packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		LOG_MSG_DEBUG("Sending aggregated packet %d into the USB pipe(%d "
				"bytes)\n", i, pPacketsSizes[i] + 12 + 16);
		int nBytesSent = input->Send(pAggregatedPacket[i],
				pPacketsSizes[i] + 12 + 16);
		if (pPacketsSizes[i] + 12 + 16 != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending aggregated packet %d into the USB pipe(%d "
					"bytes) failed!\n", i, pPacketsSizes[i] + 12 + 16);
			return false;
		}
	}

	//receive the aggregated packet
	LOG_MSG_DEBUG("Reading aggregated packet from the USB pipe(%d bytes should "
			"be there)\n", MAX_PACKET_SIZE);
	int nBytesReceived = output->Receive(pReceivedPacket, MAX_PACKET_SIZE);
	if (MAX_PACKET_SIZE != nBytesReceived)
	{
		LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", MAX_PACKET_SIZE);
		LOG_MSG_DEBUG("Received %d bytes\n", nBytesReceived);
		print_buff(pReceivedPacket, nBytesReceived);
		return false;
	}


	//comparing the received packet to the aggregated packet
	LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
	return DeaggragateAndComparePackets(pReceivedPacket, pPackets,
			pPacketsSizes, NUM_PACKETS, nBytesReceived);
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMAggregationLoopTest(
		Pipe* input, Pipe* output, enum ipa_ip_type m_eIP)
{
	//The packets that will be sent
	Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//The real sizes of the packets that will be sent
	int pPacketsSizes[NUM_PACKETS];
	//Buffer for the packet that will be received
	Byte pReceivedPacket[MAX_PACKET_SIZE];
	//Total size of all sent packets (this is the max size of the aggregated
	//packet minus the size of the header and the NDP)
	int nTotalPacketsSize = MAX_PACKET_SIZE - (4 * NUM_PACKETS) - 24;
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[NUM_PACKETS];

	//initialize the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		if (NUM_PACKETS - 1 == i)
			pPacketsSizes[i] = nTotalPacketsSize;
		else
			pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
		while (0 != pPacketsSizes[i] % 4)
			pPacketsSizes[i]++;
		nTotalPacketsSize -= pPacketsSizes[i];

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}
	}

	int num_iters = AGGREGATION_LOOP - 1;
	for (int j = 0; j < num_iters; j++)
	{
		//send the packets
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
					pPacketsSizes[i]);
			int nBytesSent = input->Send(pPackets[i], pPacketsSizes[i]);
			if (pPacketsSizes[i] != nBytesSent)
			{
				LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes) "
						"failed!\n", i, pPacketsSizes[i]);
				return false;
			}
		}

		memset(pReceivedPacket, 0, sizeof(pReceivedPacket));
		//receive the aggregated packet
		LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be "
				"there)\n", MAX_PACKET_SIZE);
		int nBytesReceived = output->Receive(pReceivedPacket, MAX_PACKET_SIZE);
		if (MAX_PACKET_SIZE != nBytesReceived)
		{
			LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d "
					"bytes) failed!\n", MAX_PACKET_SIZE);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}

		LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
		if (false == DeaggragateAndComparePackets(pReceivedPacket, pPackets,
				pPacketsSizes, NUM_PACKETS, nBytesReceived))
		{
			LOG_MSG_DEBUG("Comparing aggregated packet failed!\n");
			return false;
		}

	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMAggregationTimeLimitTest(
		Pipe* input, Pipe* output,
		enum ipa_ip_type m_eIP)
{
	//The packets that will be sent
	Byte pPackets[1][MAX_PACKET_SIZE];
	//The real sizes of the packets that will be sent
	int pPacketsSizes[1] = {0};
	//Buffer for the packet that will be received
	Byte pReceivedPacket[MAX_PACKET_SIZE] = {0};
	//Size of aggregated packet
	int nTotalPacketsSize = 24;
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[1];

	//initialize the packets
	for (int i = 0; i < 1 ; i++)
	{
		pPacketsSizes[i] = 52 + 4*i;
		nTotalPacketsSize += pPacketsSizes[i] + 4; //size of the packet + 4 bytes for index and length

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}
	}
	int nAllPacketsSizes = 0;
	for (int i = 0; i < 1; i++)
		nAllPacketsSizes += pPacketsSizes[i];
	while (0 != nAllPacketsSizes % 4)
	{
		nAllPacketsSizes++;
		nTotalPacketsSize++;  //zero padding for NDP offset to be 4x
	}

	//send the packets
	for (int i = 0; i < 1; i++)
	{
		LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
				pPacketsSizes[i]);
		int nBytesSent = input->Send(pPackets[i], pPacketsSizes[i]);
		if (pPacketsSizes[i] != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			return false;
		}
	}

	//receive the aggregated packet
	LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be "
			"there)\n", nTotalPacketsSize);
	int nBytesReceived = output->Receive(pReceivedPacket, nTotalPacketsSize);
	// IPA HW may add padding to the packets to align to 4B
	if (nTotalPacketsSize > nBytesReceived)
	{
		LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", nTotalPacketsSize);
		print_buff(pReceivedPacket, nBytesReceived);
		return false;
	}

	//comparing the received packet to the aggregated packet
	LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
	if (false == DeaggragateAndComparePackets(pReceivedPacket, pPackets,
			pPacketsSizes, 1, nBytesReceived))
	{
		LOG_MSG_DEBUG("Comparing aggregated packet failed!\n");
		print_buff(pReceivedPacket, nBytesReceived);
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMAggregationByteLimitTest(
		Pipe* input, Pipe* output,
		enum ipa_ip_type m_eIP)
{
	//The packets that will be sent
	Byte pPackets[2][MAX_PACKET_SIZE];
	//The real sizes of the packets that will be sent
	int pPacketsSizes[2];
	//Buffer for the packet that will be received
	Byte pReceivedPacket[2*MAX_PACKET_SIZE] = {0};
	//Size of aggregated packet
	int nTotalPacketsSize = 24;
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[2];

	//initialize the packets
	for (int i = 0; i < 2; i++)
	{
		pPacketsSizes[i] = (MAX_PACKET_SIZE / 2) + 10;
		nTotalPacketsSize += pPacketsSizes[i] + 4;

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}
	}


	//send the packets
	for (int i = 0; i < 2; i++)
	{
		LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
				pPacketsSizes[i]);
		int nBytesSent = input->Send(pPackets[i], pPacketsSizes[i]);
		if (pPacketsSizes[i] != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			return false;
		}
	}

	//receive the aggregated packet
	LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be "
			"there)\n", nTotalPacketsSize);
	int nBytesReceived = output->Receive(pReceivedPacket, nTotalPacketsSize);
	// IPA HW may add padding to the packets to align to 4B
	if (nTotalPacketsSize > nBytesReceived)
	{
		LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", nTotalPacketsSize);
		print_buff(pReceivedPacket, nBytesReceived);
		return false;
	}

	//comparing the received packet to the aggregated packet
	LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
	if (false == DeaggragateAndComparePackets(pReceivedPacket, pPackets,
			pPacketsSizes, 2, nBytesReceived))
	{
		LOG_MSG_DEBUG("Comparing aggregated packet failed!\n");
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMAggregationByteLimitTestFC(
	Pipe *input, Pipe *output,
	enum ipa_ip_type m_eIP)
{
	//The packets that will be sent
	Byte pPackets[2][MAX_PACKET_SIZE];
	//The real sizes of the packets that will be sent
	int pPacketsSizes[2];
	//Buffer for the packet that will be received
	Byte pReceivedPacket[2][MAX_PACKET_SIZE] = {0};
	//Size of aggregated packet
	int nTotalPacketsSize = 24 + (MAX_PACKET_SIZE / 2) + 8 + 4;
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[2];
	int nBytesReceived;

	for (int i = 0; i < 2; i++)
	{
		pPacketsSizes[i] = (MAX_PACKET_SIZE / 2) + 8;

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}
	}


	//send the packets
	for (int i = 0; i < 2; i++)
	{
		LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
				pPacketsSizes[i]);
		int nBytesSent = input->Send(pPackets[i], pPacketsSizes[i]);
		if (pPacketsSizes[i] != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			return false;
		}
	}

	/* receive the packet */
	LOG_MSG_DEBUG(
		"Reading packets from the USB pipe(%d bytes for each)"
		"\n", nTotalPacketsSize);
	for (int i = 0; i < 2; i++)
	{
		nBytesReceived = output->Receive(pReceivedPacket[i], MAX_PACKET_SIZE);
		if (nTotalPacketsSize != nBytesReceived)
		{
			LOG_MSG_ERROR(
				"Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", nBytesReceived);
			print_buff(pReceivedPacket[i], nBytesReceived);
			return false;
		}
	}

	//comparing the received packets to the aggregated packets
	LOG_MSG_DEBUG("Checking sent.vs.received packets\n");
	for (int i = 0; i < 2; i++)
	{
		if (false == DeaggragateAndComparePackets(pReceivedPacket[i],
							  &pPackets[i],
							  (int *)&pPacketsSizes[i],
							  1,
							  nBytesReceived))
		{
			LOG_MSG_DEBUG("Comparing aggregated packet failed!\n");
			return false;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////
#define DUAL_FC_IP_PACKET_L ((MAX_PACKET_SIZE / 2) + 8)
#define DUAL_FC_1_AGG_PACKET_L (12 + DUAL_FC_IP_PACKET_L + 12 + 4)
#define DUAL_FC_2_AGG_PACKET_L (12 + DUAL_FC_IP_PACKET_L + DUAL_FC_IP_PACKET_L + 12 + 4 + 4)
bool MBIMAggregationScenarios::MBIMAggregationDualDpTestFC(
	Pipe *input, Pipe *output1, Pipe *output2,
	enum ipa_ip_type m_eIP)
{
	int i;
	//The packets that will be sent
	Byte pPackets[4][MAX_PACKET_SIZE];
	//The real sizes of the packets that will be sent
	int pPacketsSizes[4];
	//Buffer for the packet that will be received
	Byte pReceivedPacket[2 * MAX_PACKET_SIZE] = {0};
	Byte pReceivedPacketFC[2][MAX_PACKET_SIZE] = {0};
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[4];
	int nBytesReceived;

	for (i = 0; i < 4; i++)
	{
		pPacketsSizes[i] = DUAL_FC_IP_PACKET_L;

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001 + (i & 0x1));
		memcpy(&pPackets[i][IPV4_DST_ADDR_OFFSET], &nIPv4DSTAddr,
		       sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = 0xAA;
			size++;
		}
	}

	//send the packets
	for (int i = 0; i < 4; i++)
	{
		LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
				pPacketsSizes[i]);
		int nBytesSent = input->Send(pPackets[i], pPacketsSizes[i]);
		if (pPacketsSizes[i] != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			return false;
		}
	}

	/* receive the packets from FC pipe */
	LOG_MSG_DEBUG(
		"Reading packets from the FC pipe (%d bytes for each)"
		"\n", DUAL_FC_1_AGG_PACKET_L);
	for (i = 0; i < 2; i++)
	{
		nBytesReceived = output1->Receive(pReceivedPacketFC[i], MAX_PACKET_SIZE);
		if (DUAL_FC_1_AGG_PACKET_L != nBytesReceived)
		{
			LOG_MSG_ERROR(
				"Receiving aggregated packet from the USB pipe (%d bytes) "
				"failed!\n", nBytesReceived);
			print_buff(pReceivedPacketFC[i], nBytesReceived);
			return false;
		}
	}

	for (i = 0; i < 2; i++)
	{
		if (false == DeaggragateAndComparePackets(pReceivedPacketFC[i],
							  &pPackets[i * 2],
							  (int *)&pPacketsSizes[i * 2],
							  1,
							  nBytesReceived))
		{
			LOG_MSG_DEBUG("Comparing aggregated packet failed!\n");
			return false;
		}
	}

	/* receive the packet from non-FC pipe */
	LOG_MSG_DEBUG(
		"Reading packet from the non-FC pipe (%d bytes)"
		"\n", DUAL_FC_2_AGG_PACKET_L);
	nBytesReceived = output2->Receive(pReceivedPacket, MAX_PACKET_SIZE);
	if (DUAL_FC_2_AGG_PACKET_L != nBytesReceived)
	{
		LOG_MSG_ERROR(
			"Receiving aggregated packet from the USB pipe (%d bytes) "
			"failed!\n", nBytesReceived);
		print_buff(pReceivedPacket, nBytesReceived);
		return false;
	}

	// Setting all source packets IP to 127.0.0.2 for comparison
	nIPv4DSTAddr = ntohl(0x7F000002);
	memcpy(&pPackets[0][IPV4_DST_ADDR_OFFSET], &nIPv4DSTAddr, sizeof(nIPv4DSTAddr));
	memcpy(&pPackets[2][IPV4_DST_ADDR_OFFSET], &nIPv4DSTAddr, sizeof(nIPv4DSTAddr));

	if (false == DeaggragateAndComparePackets(&pReceivedPacket[0], pPackets,
			(int *)&pPacketsSizes, 2, nBytesReceived))
	{
		LOG_MSG_DEBUG("Comparing aggregated packet failed!\n");
		print_buff(pReceivedPacket, nBytesReceived);
		return false;
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMDeaggregationMultipleNDPTest(
		Pipe* input, Pipe* output,
		enum ipa_ip_type m_eIP)
{
	bool bTestResult = true;
	//The packets that the aggregated packet will be made of
	Byte pExpectedPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//The real sizes of the packets that the aggregated packet will be made of
	int pPacketsSizes[NUM_PACKETS];
	//Buffers for the packets that will be received
	Byte pReceivedPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//Total size of all the packets that the aggregated packet will be made of
	//(this is the max size of the aggregated packet
	//minus the size of the header and the 2 NDPs)
	int nTotalPacketsSize = MAX_PACKET_SIZE - (4 * NUM_PACKETS) - 36;
	//The aggregated packet that will be sent
	Byte pAggregatedPacket[MAX_PACKET_SIZE] = {0};
	//The stream Id byte for every packet - this will determine on which NDP the
	//packet will appear
	Byte pPacketsStreamId[NUM_PACKETS] = {0};
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[NUM_PACKETS];

	//initialize the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		if (NUM_PACKETS - 1 == i)
			pPacketsSizes[i] = nTotalPacketsSize;
		else {
			pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
			pPacketsSizes[i] += (pPacketsSizes[i] % 4 == 0 ? 0 :
				4 - pPacketsSizes[i] % 4);
		}
		nTotalPacketsSize -= pPacketsSizes[i];
		pPacketsStreamId[i] = i < 3 ? 0 : 1;

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pExpectedPackets[i],
				pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pExpectedPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pExpectedPackets[i][size] = i;
			size++;
		}
	}

	//initializing the aggregated packet
	AggregatePacketsWithStreamId(pAggregatedPacket, pExpectedPackets,
			pPacketsSizes, NUM_PACKETS, MAX_PACKET_SIZE, pPacketsStreamId);

	//send the aggregated packet
	LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d bytes)\n",
			sizeof(pAggregatedPacket));
	int nBytesSent = input->Send(pAggregatedPacket, sizeof(pAggregatedPacket));
	if (sizeof(pAggregatedPacket) != nBytesSent)
	{
		LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d bytes) "
				"failed!\n", sizeof(pAggregatedPacket));
		return false;
	}

	//receive the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		LOG_MSG_DEBUG("Reading packet %d from the USB pipe(%d bytes should be "
				"there)\n", i, pPacketsSizes[i]);
		int nBytesReceived = output->Receive(pReceivedPackets[i],
				pPacketsSizes[i]);
		if (pPacketsSizes[i] != nBytesReceived)
		{
			LOG_MSG_DEBUG("Receiving packet %d from the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			print_buff(pReceivedPackets[i], nBytesReceived);
			return false;
		}
	}

	//comparing the received packet to the aggregated packet
	LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
	for (int i = 0; i < NUM_PACKETS; i++)
		bTestResult &= !memcmp(pExpectedPackets[i], pReceivedPackets[i],
				pPacketsSizes[i]);

	return bTestResult;
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMAggregation2PipesTest(
		Pipe* input1, Pipe* input2, Pipe* output, enum ipa_ip_type m_eIP)
{
	//The packets that will be sent
	Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//The real sizes of the packets that will be sent
	int pPacketsSizes[NUM_PACKETS];
	//Buffer for the packet that will be received
	Byte pReceivedPacket[2*MAX_PACKET_SIZE];
	//Total size of all sent packets (this is the max size of the aggregated
	//packet minus the size of the header and the NDP)
	int nTotalPacketsSize = MAX_PACKET_SIZE - (4 * NUM_PACKETS) - 24;
	//The aggregated packet that will be sent
	Byte pAggregatedPacket[2][MAX_PACKET_SIZE];
	//The size of the sent aggregated packet
	int nAggregatedPacketSize[2] = {0};
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[NUM_PACKETS];

	//initialize the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		if (NUM_PACKETS - 1 == i)
			pPacketsSizes[i] = nTotalPacketsSize;
		else
			pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
		while (0 != pPacketsSizes[i] % 4)
			pPacketsSizes[i]++;
		nTotalPacketsSize -= pPacketsSizes[i];

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}
	}


	nAggregatedPacketSize[0] += pPacketsSizes[0] + pPacketsSizes[1]; //adding the packets
	nAggregatedPacketSize[0] += 12;  //adding the header
	nAggregatedPacketSize[0] += 12 + 4*2; //adding the NDP
	//initializing the aggregated packet
	AggregatePackets(pAggregatedPacket[0], pPackets, pPacketsSizes, 2,
			nAggregatedPacketSize[0]);

	//send the aggregated packet

	LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d "
		"bytes)\n", nAggregatedPacketSize[0]);
	int nBytesSent = input1->Send(pAggregatedPacket[0],
		nAggregatedPacketSize[0]);
	if (nAggregatedPacketSize[0] != nBytesSent)
	{
		LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d bytes) "
			"failed!\n", nAggregatedPacketSize[0]);
		return false;
	}

	//send the packets
	for (int i = 2; i < NUM_PACKETS; i++)
	{
		LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
				pPacketsSizes[i]);
		int nBytesSent = input2->Send(pPackets[i], pPacketsSizes[i]);
		if (pPacketsSizes[i] != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			return false;
		}
	}

	//receive the aggregated packet
	LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be "
			"there)\n", MAX_PACKET_SIZE);
	int nBytesReceived = output->Receive(pReceivedPacket, MAX_PACKET_SIZE);
	if (MAX_PACKET_SIZE != nBytesReceived)
	{
		LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", MAX_PACKET_SIZE);
		print_buff(pReceivedPacket, nBytesReceived);
		return false;
	}

	//deaggregating the aggregated packet
	return DeaggragateAndComparePackets(pReceivedPacket, pPackets, pPacketsSizes, NUM_PACKETS, nBytesReceived);
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMAggregationTimeLimitLoopTest(
		Pipe* input, Pipe* output,
		enum ipa_ip_type m_eIP)
{
	//The packets that will be sent
	Byte pPackets[1][MAX_PACKET_SIZE];
	//The real sizes of the packets that will be sent
	int pPacketsSizes[1] = {0};
	//Buffer for the packet that will be received
	Byte pReceivedPacket[MAX_PACKET_SIZE] = {0};
	//Size of aggregated packet
	int nTotalPacketsSize = 24;
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[NUM_PACKETS];

	//initialize the packets
	for (int i = 0; i < 1 ; i++)
	{
		pPacketsSizes[i] = 52 + 4*i;
		nTotalPacketsSize += pPacketsSizes[i] + 4; //size of the packet + 4 bytes for index and length

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}
	}
	int nAllPacketsSizes = 0;
	for (int i = 0; i < 1; i++)
		nAllPacketsSizes += pPacketsSizes[i];
	while (0 != nAllPacketsSizes % 4)
	{
		nAllPacketsSizes++;
		nTotalPacketsSize++;  //zero padding for NDP offset to be 4x
	}

	for (int k = 0; k < AGGREGATION_LOOP; k++)
	{
		//send the packets
		for (int i = 0; i < 1; i++)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
					pPacketsSizes[i]);
			int nBytesSent = input->Send(pPackets[i], pPacketsSizes[i]);
			if (pPacketsSizes[i] != nBytesSent)
			{
				LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes) "
						"failed!\n", i, pPacketsSizes[i]);
				return false;
			}
		}

		//receive the aggregated packet
		LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be "
				"there)\n", nTotalPacketsSize);
		int nBytesReceived = output->Receive(pReceivedPacket,
				nTotalPacketsSize);
		// IPA HW may add padding to the packets to align to 4B
		if (nTotalPacketsSize > nBytesReceived)
		{
			LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d "
					"bytes) failed!\n", nTotalPacketsSize);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}

		//comparing the received packet to the aggregated packet
		LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
		if (false == DeaggragateAndComparePackets(pReceivedPacket, pPackets,
				pPacketsSizes, 1, nBytesReceived))
		{
			LOG_MSG_DEBUG("Comparing aggregated packet failed!\n");
			return false;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMAggregation0LimitsTest(
		Pipe* input, Pipe* output,
		enum ipa_ip_type m_eIP)
{
	//The packets that will be sent
	Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//The real sizes of the packets that will be sent
	int pPacketsSizes[NUM_PACKETS];
	//Buffer for the packet that will be received
	Byte pReceivedPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//The expected aggregated packets sizes
	int pAggragatedPacketsSizes[NUM_PACKETS] = {0};
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[NUM_PACKETS];

	//initialize the packets
	for (int i = 0; i < NUM_PACKETS ; i++)
	{
		pPacketsSizes[i] = 52 + 4*i;

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}
	}

	//calculate aggregated packets sizes
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		pAggragatedPacketsSizes[i] += pPacketsSizes[i];
		while (0 != pAggragatedPacketsSizes[i] % 4)
			pAggragatedPacketsSizes[i]++;  //zero padding for NDP offset to be 4x
		pAggragatedPacketsSizes[i] += 28;  //header + NDP
	}

	//send the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
				pPacketsSizes[i]);
		int nBytesSent = input->Send(pPackets[i], pPacketsSizes[i]);
		if (pPacketsSizes[i] != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			return false;
		}
	}

	//receive the aggregated packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		LOG_MSG_DEBUG("Reading packet %d from the USB pipe(%d bytes should be "
				"there)\n", i, pAggragatedPacketsSizes[i]);
		int nBytesReceived = output->Receive(pReceivedPackets[i],
				pAggragatedPacketsSizes[i]);
		// IPA HW may add padding to the packets to align to 4B
		if (pAggragatedPacketsSizes[i] > nBytesReceived)
		{
			LOG_MSG_DEBUG("Receiving aggregated packet %d from the USB pipe(%d "
					"bytes) failed!\n", i, pAggragatedPacketsSizes[i]);
			print_buff(pReceivedPackets[i], nBytesReceived);
			return false;
		}
		pAggragatedPacketsSizes[i] = nBytesReceived;
	}


	//comparing the received packet to the aggregated packet
	LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		if (false == DeaggragateAndCompareOnePacket(pReceivedPackets[i],
				pPackets[i], pPacketsSizes[i], pAggragatedPacketsSizes[i]))
		{
			LOG_MSG_DEBUG("Comparing aggregated packet %d failed!\n", i);
			return false;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMAggregationMultiplePacketsTest(
		Pipe* input, Pipe* output,
		enum ipa_ip_type m_eIP)
{
	//The packets that will be sent
	Byte pPackets[MAX_PACKETS_IN_NDP + 1][MAX_PACKET_SIZE];
	//The real sizes of the packets that will be sent
	int pPacketsSizes[MAX_PACKETS_IN_NDP + 1];
	//Buffer for the packet that will be received
	Byte pReceivedPacket[2*MAX_PACKET_SIZE];
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[MAX_PACKETS_IN_NDP + 1];
	//Total size of all sent packets (this is the max size of the aggregated packet
	//minus the size of the header and the 2 NDPs)
	int nTotalPacketsSize = MAX_PACKET_SIZE - (4 * (MAX_PACKETS_IN_NDP + 1)) - 24;

	//initialize the packets
	for (int i = 0; i < MAX_PACKETS_IN_NDP + 1; i++)
	{
		if (MAX_PACKETS_IN_NDP == i)
			pPacketsSizes[i] = nTotalPacketsSize;
		else
		{
			pPacketsSizes[i] = nTotalPacketsSize / (MAX_PACKETS_IN_NDP + 1);
			pPacketsSizes[i] += (pPacketsSizes[i] % 4 == 0 ? 0 :
				4 - pPacketsSizes[i] % 4);
		}
		nTotalPacketsSize -= pPacketsSizes[i];

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&pPackets[i][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
				sizeof(nIPv4DSTAddr));
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}
	}

	//send the packets
	for (int i = 0; i < MAX_PACKETS_IN_NDP + 1; i++)
	{
		LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
					pPacketsSizes[i]);
		int nBytesSent = input->Send(pPackets[i], pPacketsSizes[i]);

		if (pPacketsSizes[i] != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			return false;
		}
	}

	//receive the aggregated packet
	LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be "
			"there)\n", MAX_PACKET_SIZE);
	int nBytesReceived = output->Receive(pReceivedPacket, MAX_PACKET_SIZE);
	if (MAX_PACKET_SIZE != nBytesReceived)
	{
		LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", MAX_PACKET_SIZE);
		print_buff(pReceivedPacket, nBytesReceived);
		return false;
	}

	//deaggregating the aggregated packet
	return DeaggragateAndComparePackets(pReceivedPacket, pPackets,
			pPacketsSizes, MAX_PACKETS_IN_NDP+1, nBytesReceived);
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMAggregationDifferentStreamIdsTest(
		Pipe* input, Pipe* output,
		enum ipa_ip_type m_eIP)
{
	//The packets that will be sent
	Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//The real sizes of the packets that will be sent
	int pPacketsSizes[NUM_PACKETS];
	//Buffer for the packet that will be received
	Byte pReceivedPacket[2*MAX_PACKET_SIZE];
	//Total size of all sent packets (this is the max size of the aggregated
	//packet minus the size of the header and the NDPs)
	int nTotalPacketsSize = MAX_PACKET_SIZE - (16 * NUM_PACKETS) - 12;
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[NUM_PACKETS];
	Byte pPacketsStreamId[NUM_PACKETS];

	//initialize the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		pPacketsStreamId[i] = i;
		if (NUM_PACKETS - 1 == i)
			pPacketsSizes[i] = nTotalPacketsSize + 12;
		else
			pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
		while (0 != pPacketsSizes[i] % 4)
			pPacketsSizes[i]++;
		nTotalPacketsSize -= pPacketsSizes[i];

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}
	}

	nIPv4DSTAddr = ntohl(0x7F000001);
	memcpy (&pPackets[0][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));
	nIPv4DSTAddr = ntohl(0xC0A80101);
	memcpy (&pPackets[1][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));
	nIPv4DSTAddr = ntohl(0xC0A80102);
	memcpy (&pPackets[2][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));
	nIPv4DSTAddr = ntohl(0xC0A80103);
	memcpy (&pPackets[3][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));
	nIPv4DSTAddr = ntohl(0xC0A80104);
	memcpy (&pPackets[4][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));

	//send the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
				pPacketsSizes[i]);
		int nBytesSent = input->Send(pPackets[i], pPacketsSizes[i]);
		if (pPacketsSizes[i] != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			return false;
		}
	}

	//receive the aggregated packet
	LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be "
			"there)\n", MAX_PACKET_SIZE + 12);
	int nBytesReceived = output->Receive(pReceivedPacket, MAX_PACKET_SIZE + 12);
	if (MAX_PACKET_SIZE + 12 != nBytesReceived)
	{
		LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d bytes) "
				"failed!\n", MAX_PACKET_SIZE + 12);
		print_buff(pReceivedPacket, nBytesReceived + 12);
		return false;
	}

	//deaggregating the aggregated packet
	return DeaggragateAndComparePacketsWithStreamId(pReceivedPacket, pPackets,
			pPacketsSizes, NUM_PACKETS, nBytesReceived, pPacketsStreamId);
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::MBIMAggregationNoInterleavingStreamIdsTest(
		Pipe* input, Pipe* output,
		enum ipa_ip_type m_eIP)
{
	//The packets that will be sent
	Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
	//The real sizes of the packets that will be sent
	int pPacketsSizes[NUM_PACKETS];
	//Buffer for the packet that will be received
	Byte pReceivedPacket[2*MAX_PACKET_SIZE];
	//Total size of all sent packets (this is the max size of the aggregated packet
	//minus the size of the header and the NDPs)
	int nTotalPacketsSize = MAX_PACKET_SIZE - (16 * NUM_PACKETS) - 12;
	uint32_t nIPv4DSTAddr;
	size_t pIpPacketsSizes[NUM_PACKETS];
	Byte pPacketsStreamId[NUM_PACKETS];

	//initialize the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		pPacketsStreamId[i] = i % 2;
		if (NUM_PACKETS - 1 == i)
			pPacketsSizes[i] = nTotalPacketsSize + 12;
		else
			pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
		while (0 != pPacketsSizes[i] % 4)
			pPacketsSizes[i]++;
		nTotalPacketsSize -= pPacketsSizes[i];

		// Load input data (IP packet) from file
		pIpPacketsSizes[i] = MAX_PACKET_SIZE;
		if (!LoadDefaultPacket(m_eIP, pPackets[i], pIpPacketsSizes[i]))
		{
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		int size = pIpPacketsSizes[i];
		while (size < pPacketsSizes[i])
		{
			pPackets[i][size] = i;
			size++;
		}
	}

	nIPv4DSTAddr = ntohl(0x7F000001);
	memcpy (&pPackets[0][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));
	nIPv4DSTAddr = ntohl(0xC0A80101);
	memcpy (&pPackets[1][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));
	nIPv4DSTAddr = ntohl(0x7F000001);
	memcpy (&pPackets[2][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));
	nIPv4DSTAddr = ntohl(0xC0A80101);
	memcpy (&pPackets[3][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));
	nIPv4DSTAddr = ntohl(0x7F000001);
	memcpy (&pPackets[4][IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,
			sizeof(nIPv4DSTAddr));

	//send the packets
	for (int i = 0; i < NUM_PACKETS; i++)
	{
		LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
				pPacketsSizes[i]);
		int nBytesSent = input->Send(pPackets[i], pPacketsSizes[i]);
		if (pPacketsSizes[i] != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes) "
					"failed!\n", i, pPacketsSizes[i]);
			return false;
		}
	}

	//receive the aggregated packet
	LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be "
			"there)\n", MAX_PACKET_SIZE + 12);
	int nBytesReceived = output->Receive(pReceivedPacket,
			MAX_PACKET_SIZE + 12);
	if (MAX_PACKET_SIZE + 12 != nBytesReceived)
	{
		LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d bytes)"
				" failed!\n", MAX_PACKET_SIZE + 12);
		print_buff(pReceivedPacket, nBytesReceived + 12);
		return false;
	}

	//deaggregating the aggregated packet
	return DeaggragateAndComparePacketsWithStreamId(pReceivedPacket, pPackets,
			pPacketsSizes, NUM_PACKETS, nBytesReceived, pPacketsStreamId);
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::DeaggragateAndComparePackets(
		Byte pAggregatedPacket[MAX_PACKET_SIZE],
		Byte pExpectedPackets[MAX_PACKETS_IN_MBIM_TESTS][MAX_PACKET_SIZE],
		int pPacketsSizes[MAX_PACKETS_IN_MBIM_TESTS], int nNumPackets, int nAggregatedPacketSize)
{
	int nPacketNum = 0;
	int i = 0;
	int nNdpStart = 0;
	Byte pNdpIndex[2] = {0};
	Byte pNdpLen[2] = {0};
	if (0x4e != pAggregatedPacket[i] || 0x43 != pAggregatedPacket[i+1] ||
			0x4d != pAggregatedPacket[i+2]|| 0x48 != pAggregatedPacket[i+3])
	{
		LOG_MSG_DEBUG("Error: Wrong NTH16 signature: 0x%02x 0x%02x 0x%02x "
				"0x%02x(should be 0x4e, 0x43, 0x4d, 0x48)\n",
				pAggregatedPacket[i], pAggregatedPacket[i+1],
				pAggregatedPacket[i+2], pAggregatedPacket[i+3]);
		return false;
	}
	i += 4;
	if (0x0c != pAggregatedPacket[i] || 0x00 != pAggregatedPacket[i+1])
	{
		LOG_MSG_DEBUG("Error: Wrong header length: 0x%02x 0x%02x(should be 0x0c, "
				"0x00)\n",
				pAggregatedPacket[i], pAggregatedPacket[i+1]);
		return false;
	}
	i += 4;  //ignoring sequence number
	if ((nAggregatedPacketSize & 0x00FF) != pAggregatedPacket[i] ||
			(nAggregatedPacketSize >> 8) != pAggregatedPacket[i+1])
	{
		LOG_MSG_DEBUG("Error: Wrong aggregated packet length: 0x%02x 0x%02x"
				"(should be 0x%02x, 0x%02x)\n",
				pAggregatedPacket[i], pAggregatedPacket[i+1],
				nAggregatedPacketSize & 0x00FF, nAggregatedPacketSize >> 8);
		return false;
	}
	i += 2;
	pNdpIndex[0] = pAggregatedPacket[i];  //least significant byte
	pNdpIndex[1] = pAggregatedPacket[i+1];  //most significant byte
	//reading the NDP
	while (0x00 != pNdpIndex[0] || 0x00 != pNdpIndex[1])
	{
		i = pNdpIndex[0] + 256*pNdpIndex[1];  //NDP should begin here
		nNdpStart = i;

		if (0x49 != pAggregatedPacket[i] || 0x50 != pAggregatedPacket[i + 1] ||
			0x53 != pAggregatedPacket[i + 2] || 0x00 != pAggregatedPacket[i + 3])
		{
			LOG_MSG_DEBUG("Error: Wrong NDP16 signature: 0x%02x 0x%02x "
				"0x%02x 0x%02x(should be 0x49, 0x50, 0x53, 0x00)\n",
				pAggregatedPacket[i], pAggregatedPacket[i + 1],
				pAggregatedPacket[i + 2], pAggregatedPacket[i + 3]);
			return false;
		}
		i += 4;
		pNdpLen[0] = pAggregatedPacket[i];  //least significant byte
		pNdpLen[1] = pAggregatedPacket[i+1];  //most significant byte
		if (0x00 != pAggregatedPacket[nNdpStart + pNdpLen[0] + 256*pNdpLen[1] - 2] ||
				0x00 != pAggregatedPacket[nNdpStart + pNdpLen[0] + 256*pNdpLen[1] -1])
		{
			LOG_MSG_DEBUG("Error: Wrong end of NDP: 0x%02x 0x%02x(should be 0x00,"
					" 0x00)\n",
					pAggregatedPacket[nNdpStart + pNdpLen[0] + 256*pNdpLen[1] - 2],
					pAggregatedPacket[nNdpStart + pNdpLen[0] + 256*pNdpLen[1] - 1]);
			return false;
		}
		i += 2;
		pNdpIndex[0] = pAggregatedPacket[i];  //least significant byte
		pNdpIndex[1] = pAggregatedPacket[i+1];  //most significant byte
		i += 2;
		while (i <= nNdpStart + pNdpLen[0] + 256*pNdpLen[1] - 2)
		{ //going over all the datagrams in this NDP
			Byte pDatagramIndex[2] = {0};
			Byte pDatagramLen[2] = {0};
			int packetIndex = 0;
			pDatagramIndex[0] = pAggregatedPacket[i];  //least significant byte
			pDatagramIndex[1] = pAggregatedPacket[i+1];  //most significant byte
			i += 2;
			if (0x00 == pDatagramIndex[0] && 0x00 == pDatagramIndex[1])
				break;  //zero padding after all datagrams
			if (nPacketNum >= nNumPackets)
			{
				LOG_MSG_DEBUG("Error: wrong number of packets: %d(should be %d)\n",
						nPacketNum, nNumPackets);
				return false;
			}
			pDatagramLen[0] = pAggregatedPacket[i];  //least significant byte
			pDatagramLen[1] = pAggregatedPacket[i+1];  //most significant byte
			i += 2;
			packetIndex = pDatagramIndex[0] + 256*pDatagramIndex[1];
			if (pDatagramLen[0] + 256*pDatagramLen[1] != pPacketsSizes[nPacketNum])
			{
				LOG_MSG_DEBUG("Error: Wrong packet %d length: 0x%02x 0x%02x"
						"(should be %d)\n", nPacketNum, pDatagramLen[0],
						pDatagramLen[1], pPacketsSizes[nPacketNum]);
				return false;
			}
			if (0 != memcmp(pExpectedPackets[nPacketNum],
					&pAggregatedPacket[packetIndex], pPacketsSizes[nPacketNum]))
			{
				LOG_MSG_DEBUG("Error: Comparison of packet %d failed!\n",
						nPacketNum);

				return false;
			}
			nPacketNum++;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////

void MBIMAggregationScenarios::AggregatePackets(
		Byte pAggregatedPacket[MAX_PACKET_SIZE]/*ouput*/,
		Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE],
		int pPacketsSizes[NUM_PACKETS], int nNumPackets,
		int nAggregatedPacketSize)
{
	int i = 0;
	int pDatagramIndexes[NUM_PACKETS] = {0};
	int nNdpIndex = 0;
	int nNdpLen = 0;
	//NTH16 signature
	pAggregatedPacket[i] = 0x4e;
	pAggregatedPacket[i+1] = 0x43;
	pAggregatedPacket[i+2] = 0x4d;
	pAggregatedPacket[i+3] = 0x48;
	i += 4;
	//header length
	pAggregatedPacket[i] = 0x0c;
	pAggregatedPacket[i+1] = 0x00;
	i += 2;
	//sequence number
	pAggregatedPacket[i] = 0x00;
	pAggregatedPacket[i+1] = 0x00;
	i += 2;
	//aggregated packet length
	pAggregatedPacket[i] = nAggregatedPacketSize & 0x00FF;
	pAggregatedPacket[i+1] = nAggregatedPacketSize >> 8;
	i += 2;
	//NDP index
	for (int j = 0; j < nNumPackets; j++)
		nNdpIndex += pPacketsSizes[j];
	nNdpIndex += i + 2;
	while (0 != nNdpIndex % 4)
		nNdpIndex++;
	pAggregatedPacket[i] = nNdpIndex & 0x00FF;
	pAggregatedPacket[i+1] = nNdpIndex >> 8;
	i += 2;
	//packets
	for (int j = 0; j < nNumPackets; j++)
	{
		pDatagramIndexes[j] = i;
		for (int k = 0; k < pPacketsSizes[j]; k++)
		{
			pAggregatedPacket[i] = pPackets[j][k];
			i++;
		}
	}
	while (i < nNdpIndex)
	{
		pAggregatedPacket[i] = 0x00;
		i++;
	}

	//NDP16 signature
	pAggregatedPacket[i] = 0x49;
	pAggregatedPacket[i+1] = 0x50;
	pAggregatedPacket[i+2] = 0x53;
	pAggregatedPacket[i+3] = 0x00;
	i += 4;
	//NDP length
	nNdpLen = 4*nNumPackets + 8 + 2;
	while (nNdpLen % 4 != 0)
		nNdpLen += 2;
	pAggregatedPacket[i] = nNdpLen & 0x00FF;
	pAggregatedPacket[i+1] = nNdpLen >> 8;
	i += 2;
	//next NDP
	pAggregatedPacket[i] = 0x00;
	pAggregatedPacket[i+1] = 0x00;
	i += 2;
	for (int j = 0; j < nNumPackets; j++)
	{
		//datagram index
		pAggregatedPacket[i] = pDatagramIndexes[j] & 0x00FF;
		pAggregatedPacket[i+1] = pDatagramIndexes[j] >> 8;
		i += 2;
		//datagram length
		pAggregatedPacket[i] = pPacketsSizes[j] & 0x00FF;
		pAggregatedPacket[i+1] = pPacketsSizes[j] >> 8;
		i += 2;
	}
	//zeros in the end of NDP
	while (i < nAggregatedPacketSize)
	{
		pAggregatedPacket[i] = 0x00;
		i++;
	}
}

/////////////////////////////////////////////////////////////////////////////////

void MBIMAggregationScenarios::AggregatePacketsWithStreamId(
		Byte pAggregatedPacket[MAX_PACKET_SIZE]/*ouput*/,
		Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE],
		int pPacketsSizes[NUM_PACKETS], int nNumPackets, int nAggregatedPacketSize,
		Byte pPacketsStreamId[NUM_PACKETS])
{
	int i = 0;
	int n = 0;
	int pDatagramIndexes[NUM_PACKETS] = {0};
	int nNdpIndex[NUM_PACKETS] = {0};
	int nNdpLen = 0;
	Byte currStreamId = pPacketsStreamId[0];
	int nNdpFirstPacket[NUM_PACKETS] = {0};
	int nNdpAfterLastPacket[NUM_PACKETS] = {0};
	int nNumNDPs = 0;
	for (n = 0; n < nNumPackets; n++)
	{
		if (currStreamId != pPacketsStreamId[n])
		{
			nNdpAfterLastPacket[nNumNDPs] = n;
			nNumNDPs++;
			nNdpFirstPacket[nNumNDPs] = n;
			currStreamId = pPacketsStreamId[n];
		}
	}
	nNdpAfterLastPacket[nNumNDPs] = n;
	nNumNDPs++;
	//calculate NDP indexes
	nNdpIndex[0] += 12;  //adding the header
	for (int j = 0; j < nNumNDPs; j++)
	{
		for (n = nNdpFirstPacket[j]; n < nNdpAfterLastPacket[j]; n++)
			nNdpIndex[j] += pPacketsSizes[n];  //adding the packets
		while (0 != nNdpIndex[j] % 4)
			nNdpIndex[j]++;
		if (j < nNumNDPs - 1)
			nNdpIndex[j+1] += nNdpIndex[j] + 12 + 4*(nNdpAfterLastPacket[j] -
					nNdpFirstPacket[j]);  //adding the location after the current NDP to the next NDP
	}
	//start building the aggregated packet
	//NTH16 signature
	pAggregatedPacket[i] = 0x4e;
	pAggregatedPacket[i+1] = 0x43;
	pAggregatedPacket[i+2] = 0x4d;
	pAggregatedPacket[i+3] = 0x48;
	i += 4;
	//header length
	pAggregatedPacket[i] = 0x0c;
	pAggregatedPacket[i+1] = 0x00;
	i += 2;
	//sequence number
	pAggregatedPacket[i] = 0x00;
	pAggregatedPacket[i+1] = 0x00;
	i += 2;
	//aggregated packet length
	pAggregatedPacket[i] = nAggregatedPacketSize & 0x00FF;
	pAggregatedPacket[i+1] = nAggregatedPacketSize >> 8;;
	i += 2;
	//first NDP index
	pAggregatedPacket[i] = nNdpIndex[0] & 0x00FF;
	pAggregatedPacket[i+1] = nNdpIndex[0] >> 8;
	i += 2;
	for (n = 0; n < nNumNDPs; n++)
	{
		//packets
		for (int j = nNdpFirstPacket[n]; j < nNdpAfterLastPacket[n]; j++)
		{
			pDatagramIndexes[j] = i;
			for (int k = 0; k < pPacketsSizes[j]; k++)
			{
				pAggregatedPacket[i] = pPackets[j][k];
				i++;
			}
		}
		while (i < nNdpIndex[n])
		{
			pAggregatedPacket[i] = 0x00;
			i++;
		}
		//NDP signature
		pAggregatedPacket[i] = 0x49;
		pAggregatedPacket[i+1] = 0x50;
		pAggregatedPacket[i+2] = 0x53;
		pAggregatedPacket[i+3] = pPacketsStreamId[nNdpFirstPacket[n]];
		i += 4;
		//NDP length
		nNdpLen = 4*(nNdpAfterLastPacket[n] - nNdpFirstPacket[n]) + 8 + 2;
		while (nNdpLen % 4 != 0)
			nNdpLen += 2;
		pAggregatedPacket[i] = nNdpLen & 0x00FF;
		pAggregatedPacket[i+1] = nNdpLen >> 8;
		i += 2;
		//next NDP
		pAggregatedPacket[i] = nNdpIndex[n+1] & 0x00FF;
		pAggregatedPacket[i+1] = nNdpIndex[n+1] >> 8;
		i += 2;
		for (int j = nNdpFirstPacket[n]; j < nNdpAfterLastPacket[n]; j++)
		{
			//datagram index
			pAggregatedPacket[i] = pDatagramIndexes[j] & 0x00FF;
			pAggregatedPacket[i+1] = pDatagramIndexes[j] >> 8;
			i += 2;
			//datagram length
			pAggregatedPacket[i] = pPacketsSizes[j] & 0x00FF;
			pAggregatedPacket[i+1] = pPacketsSizes[j] >> 8;
			i += 2;
		}
		//zeros in the end of NDP
		while (i < nNdpIndex[n] + nNdpLen)
		{
			pAggregatedPacket[i] = 0x00;
			i++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::DeaggragateAndCompareOnePacket(
		Byte pAggregatedPacket[MAX_PACKET_SIZE],
		Byte pExpectedPacket[MAX_PACKET_SIZE], int nPacketsSize,
		int nAggregatedPacketSize)
{
	int nPacketNum = 0;
	int i = 0;
	int nNdpStart = 0;
	Byte pNdpIndex[2] = {0};
	Byte pNdpLen[2] = {0};
	if (0x4e != pAggregatedPacket[i] || 0x43 != pAggregatedPacket[i+1] ||
			0x4d != pAggregatedPacket[i+2]|| 0x48 != pAggregatedPacket[i+3])
	{
		LOG_MSG_DEBUG("Error: Wrong NTH16 signature: 0x%02x 0x%02x 0x%02x "
				"0x%02x(should be 0x4e, 0x43, 0x4d, 0x48)\n",
				pAggregatedPacket[i], pAggregatedPacket[i+1],
				pAggregatedPacket[i+2], pAggregatedPacket[i+3]);
		return false;
	}
	i += 4;
	if (0x0c != pAggregatedPacket[i] || 0x00 != pAggregatedPacket[i+1])
	{
		LOG_MSG_DEBUG("Error: Wrong header length: 0x%02x 0x%02x(should be 0x0c,"
				" 0x00)\n", pAggregatedPacket[i], pAggregatedPacket[i+1]);
		return false;
	}
	i += 4;  //ignoring sequence number
	if ((nAggregatedPacketSize & 0x00FF) != pAggregatedPacket[i] ||
			(nAggregatedPacketSize >> 8) != pAggregatedPacket[i+1])
	{
		LOG_MSG_DEBUG("Error: Wrong aggregated packet length: 0x%02x 0x%02x"
				"(should be 0x%02x, 0x%02x)\n",
				pAggregatedPacket[i], pAggregatedPacket[i+1],
				nAggregatedPacketSize & 0x00FF, nAggregatedPacketSize >> 8);
		return false;
	}
	i += 2;
	pNdpIndex[0] = pAggregatedPacket[i];  //least significant byte
	pNdpIndex[1] = pAggregatedPacket[i+1];  //most significant byte
	//reading the NDP
	while (0x00 != pNdpIndex[0] || 0x00 != pNdpIndex[1])
	{
		i = pNdpIndex[0] + 256*pNdpIndex[1];  //NDP should begin here
		nNdpStart = i;

		if (0x49 != pAggregatedPacket[i] || 0x50 != pAggregatedPacket[i+1] ||
				0x53 != pAggregatedPacket[i+2] || 0x00 != pAggregatedPacket[i+3])
		{
			LOG_MSG_DEBUG("Error: Wrong NDP16 signature: 0x%02x 0x%02x "
					"0x%02x 0x%02x(should be 0x49, 0x50, 0x53, 0x00)\n",
					pAggregatedPacket[i], pAggregatedPacket[i+1],
					pAggregatedPacket[i+2], pAggregatedPacket[i+3]);
			return false;
		}
		i += 4;
		pNdpLen[0] = pAggregatedPacket[i];  //least significant byte
		pNdpLen[1] = pAggregatedPacket[i+1];  //most significant byte
		if (0x00 != pAggregatedPacket[nNdpStart + pNdpLen[0] + 256*pNdpLen[1] - 2] ||
				0x00 != pAggregatedPacket[nNdpStart + pNdpLen[0] + 256*pNdpLen[1] -1])
		{
			LOG_MSG_DEBUG("Error: Wrong end of NDP: 0x%02x 0x%02x(should be "
					"0x00, 0x00)\n",
					pAggregatedPacket[nNdpStart + pNdpLen[0] + 256*pNdpLen[1] - 2],
					pAggregatedPacket[nNdpStart + pNdpLen[0] + 256*pNdpLen[1] - 1]);
			return false;
		}
		i += 2;
		pNdpIndex[0] = pAggregatedPacket[i];  //least significant byte
		pNdpIndex[1] = pAggregatedPacket[i+1];  //most significant byte
		i += 2;
		while (i <= nNdpStart + pNdpLen[0] + 256*pNdpLen[1] - 2)
		{ //going over all the datagrams in this NDP
			Byte pDatagramIndex[2] = {0};
			Byte pDatagramLen[2] = {0};
			int packetIndex = 0;
			pDatagramIndex[0] = pAggregatedPacket[i];  //least significant byte
			pDatagramIndex[1] = pAggregatedPacket[i+1];  //most significant byte
			i += 2;
			if (0x00 == pDatagramIndex[0] && 0x00 == pDatagramIndex[1])
				break;  //zero padding after all datagrams
			if (nPacketNum > 1)
			{
				LOG_MSG_DEBUG("Error: wrong number of packets: %d(should be %d)\n",
						nPacketNum, 1);
				return false;
			}
			pDatagramLen[0] = pAggregatedPacket[i];  //least significant byte
			pDatagramLen[1] = pAggregatedPacket[i+1];  //most significant byte
			i += 2;
			packetIndex = pDatagramIndex[0] + 256*pDatagramIndex[1];
			if (pDatagramLen[0] + 256*pDatagramLen[1] != nPacketsSize)
			{
				LOG_MSG_DEBUG("Error: Wrong packet %d length: 0x%02x 0x%02x"
						"(should be %d)\n", nPacketNum, pDatagramLen[0],
						pDatagramLen[1], nPacketsSize);
				return false;
			}
			if (0 != memcmp(pExpectedPacket, &pAggregatedPacket[packetIndex],
					nPacketsSize))
			{
				LOG_MSG_DEBUG("Error: Comparison of packet %d failed!\n",
						nPacketNum);
				return false;
			}
			nPacketNum++;
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationScenarios::DeaggragateAndComparePacketsWithStreamId(
		Byte pAggregatedPacket[MAX_PACKET_SIZE],
		Byte pExpectedPackets[][MAX_PACKET_SIZE], int pPacketsSizes[],
		int nNumPackets, int nAggregatedPacketSize,
		Byte pPacketsStreamId[NUM_PACKETS])
{
	int nPacketNum = 0;
	int i = 0;
	int nNdpStart = 0;
	Byte pNdpIndex[2] = {0};
	Byte pNdpLen[2] = {0};
	if (0x4e != pAggregatedPacket[i] || 0x43 != pAggregatedPacket[i+1] ||
			0x4d != pAggregatedPacket[i+2]|| 0x48 != pAggregatedPacket[i+3])
	{
		LOG_MSG_DEBUG("Error: Wrong NTH16 signature: 0x%02x 0x%02x 0x%02x "
				"0x%02x(should be 0x4e, 0x43, 0x4d, 0x48)\n",
				pAggregatedPacket[i], pAggregatedPacket[i+1],
				pAggregatedPacket[i+2], pAggregatedPacket[i+3]);
		return false;
	}
	i += 4;
	if (0x0c != pAggregatedPacket[i] || 0x00 != pAggregatedPacket[i+1])
	{
		LOG_MSG_DEBUG("Error: Wrong header length: 0x%02x 0x%02x(should be "
				"0x0c, 0x00)\n",pAggregatedPacket[i], pAggregatedPacket[i+1]);
		return false;
	}
	i += 4;  //ignoring sequence number
	if ((nAggregatedPacketSize & 0x00FF) != pAggregatedPacket[i] ||
			(nAggregatedPacketSize >> 8) != pAggregatedPacket[i+1])
	{
		LOG_MSG_DEBUG("Error: Wrong aggregated packet length: 0x%02x 0x%02x"
				"(should be 0x%02x, 0x%02x)\n", pAggregatedPacket[i],
				pAggregatedPacket[i+1], nAggregatedPacketSize & 0x00FF,
				nAggregatedPacketSize >> 8);
		return false;
	}
	i += 2;
	pNdpIndex[0] = pAggregatedPacket[i];  //least significant byte
	pNdpIndex[1] = pAggregatedPacket[i+1];  //most significant byte
	//reading the NDP
	while (0x00 != pNdpIndex[0] || 0x00 != pNdpIndex[1])
	{
		i = pNdpIndex[0] + 256*pNdpIndex[1];  //NDP should begin here
		nNdpStart = i;
		if (0x49 != pAggregatedPacket[i] || 0x50 != pAggregatedPacket[i+1] ||
				0x53 != pAggregatedPacket[i+2])
		{
			LOG_MSG_DEBUG("Error: Wrong NDP16 signature: 0x%02x 0x%02x 0x%02x"
					"(should be 0x49, 0x50, 0x53)\n", pAggregatedPacket[i],
					pAggregatedPacket[i+1], pAggregatedPacket[i+2]);
			return false;
		}
		if (pPacketsStreamId[nPacketNum] != pAggregatedPacket[i+3])
		{
			LOG_MSG_DEBUG("Error: Wrong NDP stream id: 0x%02x(should be 0x%02x)\n",
					pAggregatedPacket[i+3], pPacketsStreamId[nPacketNum]);
			return false;
		}
		i += 4;
		pNdpLen[0] = pAggregatedPacket[i];  //least significant byte
		pNdpLen[1] = pAggregatedPacket[i+1];  //most significant byte
		if (0x00 != pAggregatedPacket[nNdpStart + pNdpLen[0] + 256*pNdpLen[1] - 2] ||
				0x00 != pAggregatedPacket[nNdpStart + pNdpLen[0] + 256*pNdpLen[1] -1])
		{
			LOG_MSG_DEBUG("Error: Wrong end of NDP: 0x%02x 0x%02x(should be 0x00, "
					"0x00)\n",
					pAggregatedPacket[nNdpStart + pNdpLen[0] + 256*pNdpLen[1] - 2],
					pAggregatedPacket[nNdpStart + pNdpLen[0] + 256*pNdpLen[1] - 1]);
			return false;
		}
		i += 2;
		pNdpIndex[0] = pAggregatedPacket[i];  //least significant byte
		pNdpIndex[1] = pAggregatedPacket[i+1];  //most significant byte
		i += 2;
		while (i <= nNdpStart + pNdpLen[0] + 256*pNdpLen[1] - 2)
		{ //going over all the datagrams in this NDP
			Byte pDatagramIndex[2] = {0};
			Byte pDatagramLen[2] = {0};
			int packetIndex = 0;
			pDatagramIndex[0] = pAggregatedPacket[i];  //least significant byte
			pDatagramIndex[1] = pAggregatedPacket[i+1];  //most significant byte
			i += 2;
			if (0x00 == pDatagramIndex[0] && 0x00 == pDatagramIndex[1])
				break;  //zero padding after all datagrams
			if (nPacketNum >= nNumPackets)
			{
				LOG_MSG_DEBUG("Error: wrong number of packets: %d(should be %d)\n",
						nPacketNum, nNumPackets);
				return false;
			}
			pDatagramLen[0] = pAggregatedPacket[i];  //least significant byte
			pDatagramLen[1] = pAggregatedPacket[i+1];  //most significant byte
			i += 2;
			packetIndex = pDatagramIndex[0] + 256*pDatagramIndex[1];
			if (pDatagramLen[0] + 256*pDatagramLen[1] != (int)pPacketsSizes[nPacketNum])
			{
				LOG_MSG_DEBUG("Error: Wrong packet %d length: 0x%02x 0x%02x"
						"(should be %d)\n", nPacketNum, pDatagramLen[0],
						pDatagramLen[1], pPacketsSizes[nPacketNum]);
				return false;
			}
			if (0 != memcmp(pExpectedPackets[nPacketNum],
					&pAggregatedPacket[packetIndex], pPacketsSizes[nPacketNum]))
			{
				LOG_MSG_DEBUG("Error: Comparison of packet %d failed!\n",
						nPacketNum);
				return false;
			}
			nPacketNum++;
		}
	}

	return true;
}


class MBIMAggregationTest: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregationTest(bool generic_agg) :
		MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMAggregationTest";
		else
			m_name = "MBIMAggregationTest";
		m_description = "MBIM Aggregation test - sends 5 packets and receives 1 "
				"aggregated packet";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRules1HeaderAggregation();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregationTest(&m_UsbToIpaPipe,
				&m_IpaToUsbPipeAgg, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class MBIMDeaggregationTest: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMDeaggregationTest(bool generic_agg) :
		MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMDeaggregationTest";
		else
			m_name = "MBIMDeaggregationTest";
		m_description = "MBIM Deaggregation test - sends an aggregated packet made from"
				"5 packets and receives 5 packets";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRulesDeaggregation();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMDeaggregationTest(&m_UsbToIpaPipeDeagg, &m_IpaToUsbPipe, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

class MBIMDeaggregationOnePacketTest: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMDeaggregationOnePacketTest(bool generic_agg) :
		MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMDeaggregationOnePacketTest";
		else
			m_name = "MBIMDeaggregationOnePacketTest";
		m_description = "MBIM Deaggregation one packet test - sends an aggregated packet made"
				"of 1 packet and receives 1 packet";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRulesDeaggregation();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMDeaggregationOnePacketTest(&m_UsbToIpaPipeDeagg, &m_IpaToUsbPipe, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


class MBIMDeaggregationAndAggregationTest: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMDeaggregationAndAggregationTest(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMDeaggregationAndAggregationTest";
		else
			m_name = "MBIMDeaggregationAndAggregationTest";
		m_description = "MBIM Deaggregation and Aggregation test - sends an aggregated "
				"packet made from 5 packets and receives the same aggregated packet";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRules1HeaderAggregation();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMDeaggregationAndAggregationTest(
				&m_UsbToIpaPipeDeagg, &m_IpaToUsbPipeAgg, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////

};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


class MBIMMultipleDeaggregationAndAggregationTest:
	public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMMultipleDeaggregationAndAggregationTest(bool generic_agg) :
		MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMMultipleDeaggregationAndAggregationTest";
		else
			m_name = "MBIMMultipleDeaggregationAndAggregationTest";
		m_description = "MBIM Multiple Deaggregation and Aggregation test - sends 5 aggregated "
				"packets each one made of 1 packet and receives an aggregated packet made of the"
				"5 packets";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRules1HeaderAggregation();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMMultipleDeaggregationAndAggregationTest(
				&m_UsbToIpaPipeDeagg, &m_IpaToUsbPipeAgg, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////

};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class MBIMAggregationLoopTest: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregationLoopTest(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMggregationLoopTest";
		else
			m_name = "MBIMggregationLoopTest";
		m_description = "MBIM Aggregation Loop test - sends 5 packets and expects to"
				"receives 1 aggregated packet a few times";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRules1HeaderAggregation();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregationLoopTest(
				&m_UsbToIpaPipe, &m_IpaToUsbPipeAgg, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class MBIMAggregationTimeLimitTest: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregationTimeLimitTest(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMAggregationTimeLimitTest";
		else
			m_name = "MBIMAggregationTimeLimitTest";
		m_description = "MBIM Aggregation time limit test - sends 1 small packet "
				"smaller than the byte limit and receives 1 aggregated packet";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRules1HeaderAggregationTime();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregationTimeLimitTest(
				&m_UsbToIpaPipe, &m_IpaToUsbPipeAggTime, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class MBIMAggregationByteLimitTest: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregationByteLimitTest(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMAggregationByteLimitTest";
		else
			m_name = "MBIMAggregationByteLimitTest";
		m_description = "MBIM Aggregation byte limit test - sends 2 packets that together "
				"are larger than the byte limit ";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRules1HeaderAggregation();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregationByteLimitTest(
				&m_UsbToIpaPipe, &m_IpaToUsbPipeAgg, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class MBIMAggregationByteLimitTestFC: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregationByteLimitTestFC(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg) {
		if (generic_agg)
			m_name = "GenMBIMAggregationByteLimitTestFC";
		else
			m_name = "MBIMAggregationByteLimitTestFC";
		m_description = "MBIMAggregationByteLimitTestFC - Send 4 IP packet with FC"
			"and expect 4 aggregated MBIM packets.";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules() {
		return AddRules1HeaderAggregation(true);
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregationByteLimitTestFC(
				&m_UsbToIpaPipe, &m_IpaToUsbPipeAgg, m_eIP);
	}
};

class MBIMAggregationDualDpTestFC : public MBIMAggregationTestFixtureConf11
{
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregationDualDpTestFC(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg) m_name = "GenMBIMAggregationDualDpTestFC";
		else m_name = "MBIMAggregationDualDpTestFC";
		m_description = "MBIMAggregationDualDpTestFC - Send IP packets "
			"on two datapathes: one with FC and one without. "
			"Expect 2 aggregated MBIM packets on pipe with FC. "
			"Expect one aggregated MBIM packet on pipe without FC. ";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRulesAggDualFC(&m_UsbToIpaPipe,
					 &m_IpaToUsbPipeAggTime,
					 &m_IpaToUsbPipeAgg);
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregationDualDpTestFC(
			&m_UsbToIpaPipe, &m_IpaToUsbPipeAggTime, &m_IpaToUsbPipeAgg, m_eIP);
	}
};

class MBIMAggregationDualDpTestFcRoutingBased : public MBIMAggregationTestFixtureConf11
{
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregationDualDpTestFcRoutingBased(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg) m_name = "GenMBIMAggregationDualDpTestFcRoutingBased";
		else m_name = "MBIMAggregationDualDpTestFcRoutingBased";
		m_description = "MBIMAggregationDualDpTestFcRoutingBased - Send IP packets "
			"on two datapathes: one with RT based FC and one without. "
			"Expect 2 aggregated MBIM packets on pipe with RT based FC. "
			"Expect one aggregated MBIM packet on pipe without RT based FC. ";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRulesAggDualFcRoutingBased(&m_UsbToIpaPipe,
					 &m_IpaToUsbPipeAggTime,
					 &m_IpaToUsbPipeAgg);
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregationDualDpTestFC(
			&m_UsbToIpaPipe, &m_IpaToUsbPipeAggTime, &m_IpaToUsbPipeAgg, m_eIP);
	}
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class MBIMDeaggregationMultipleNDPTest: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMDeaggregationMultipleNDPTest(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMDeaggregationMultipleNDPTest";
		else
			m_name = "MBIMDeaggregationMultipleNDPTest";
		m_description = "MBIM Deaggregation multiple NDP test - sends an aggregated"
				"packet made from 5 packets and 2 NDPs and receives 5 packets";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRulesDeaggregation();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMDeaggregationMultipleNDPTest(
				&m_UsbToIpaPipeDeagg, &m_IpaToUsbPipe, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class MBIMAggregation2PipesTest: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregation2PipesTest(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMAggregation2PipesTest";
		else
			m_name = "MBIMAggregation2PipesTest";
		m_description = "MBIM Aggregation 2 pipes test - sends 3 packets from one pipe"
				"and an aggregated packet made of 2 packets from another pipe and "
				"receives 1 aggregated packet made of all 5 packets";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRules1HeaderAggregation();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregation2PipesTest(
				&m_UsbToIpaPipeDeagg, &m_UsbToIpaPipe, &m_IpaToUsbPipeAgg, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class MBIMAggregationTimeLimitLoopTest: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregationTimeLimitLoopTest(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMAggregationTimeLimitLoopTest";
		else
			m_name = "MBIMAggregationTimeLimitLoopTest";
		m_description = "MBIM Aggregation time limit loop test - sends 5 small packet "
				"smaller than the byte limit and receives 5 aggregated packet";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRules1HeaderAggregationTime();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregationTimeLimitLoopTest(
				&m_UsbToIpaPipe, &m_IpaToUsbPipeAggTime, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class MBIMAggregationMultiplePacketsTest: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregationMultiplePacketsTest(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMAggregationMultiplePacketsTest";
		else
			m_name = "MBIMAggregationMultiplePacketsTest";
		m_description = "MBIM Aggregation multiple packets test - sends 9 packets "
				"with same stream ID and receives 1 aggregated packet with 2 NDPs";
		this->m_runInRegression = false;
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRules1HeaderAggregation();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregationMultiplePacketsTest(
				&m_UsbToIpaPipe, &m_IpaToUsbPipeAgg, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class MBIMAggregation0LimitsTest: public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregation0LimitsTest(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMAggregation0LimitsTest";
		else
			m_name = "MBIMAggregation0LimitsTest";
		m_description = "MBIM Aggregation 0 limits test - sends 5 packets and expects"
				"to get each packet back aggregated (both size and time limits are 0)";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		return AddRules1HeaderAggregation0Limits();
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregation0LimitsTest(
				&m_UsbToIpaPipe, &m_IpaToUsbPipeAgg0Limits, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class MBIMAggregationDifferentStreamIdsTest:
	public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregationDifferentStreamIdsTest(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMAggregationDifferentStreamIdsTest";
		else
			m_name = "MBIMAggregationDifferentStreamIdsTest";
		m_description = "MBIM Aggregation different stream IDs test - sends 5 packets"
				"with different stream IDs and receives 1 aggregated packet made of 5"
				"NDPs";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		m_eIP = IPA_IP_v4;
		const char aBypass[NUM_PACKETS][20] = {{"Bypass1"}, {"Bypass2"}, {"Bypass3"},
				{"Bypass4"}, {"Bypass5"}};
		uint32_t nTableHdl[NUM_PACKETS];
		bool bRetVal = true;
		IPAFilteringTable cFilterTable0;
		struct ipa_flt_rule_add sFilterRuleEntry;
		struct ipa_ioc_get_hdr sGetHeader[NUM_PACKETS];
		uint8_t aHeadertoAdd[NUM_PACKETS][4];
		int hdrSize;

		for (int i = 0; i < NUM_PACKETS; i++) {
			if (mGenericAgg) {
				hdrSize = 4;
				aHeadertoAdd[i][0] = 0x49;
				aHeadertoAdd[i][1] = 0x50;
				aHeadertoAdd[i][2] = 0x53;
				aHeadertoAdd[i][3] = (uint8_t)i;
			} else {
				hdrSize = 1;
				aHeadertoAdd[i][0] = (uint8_t)i;
			}
		}

		LOG_MSG_STACK("Entering Function");
		memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));
		for (int i = 0; i < NUM_PACKETS; i++)
			memset(&sGetHeader[i], 0, sizeof(sGetHeader[i]));
		// Create Header:
		// Allocate Memory, populate it, and add in to the Header Insertion.
		struct ipa_ioc_add_hdr * pHeaderDescriptor = NULL;
		pHeaderDescriptor = (struct ipa_ioc_add_hdr *) calloc(1,
				sizeof(struct ipa_ioc_add_hdr)
						+ NUM_PACKETS * sizeof(struct ipa_hdr_add));
		if (!pHeaderDescriptor)
		{
			LOG_MSG_ERROR("calloc failed to allocate pHeaderDescriptor");
			bRetVal = false;
			goto bail;
		}

		pHeaderDescriptor->commit = true;
		pHeaderDescriptor->num_hdrs = NUM_PACKETS;
		// Adding Header No1.
		strlcpy(pHeaderDescriptor->hdr[0].name, "StreamId0", sizeof(pHeaderDescriptor->hdr[0].name)); // Header's Name
		memcpy(pHeaderDescriptor->hdr[0].hdr, (void*)&aHeadertoAdd[0],
				hdrSize); //Header's Data
		pHeaderDescriptor->hdr[0].hdr_len = hdrSize;
		pHeaderDescriptor->hdr[0].hdr_hdl    = -1; //Return Value
		pHeaderDescriptor->hdr[0].is_partial = false;
		pHeaderDescriptor->hdr[0].status     = -1; // Return Parameter

		// Adding Header No2.
		strlcpy(pHeaderDescriptor->hdr[1].name, "StreamId1", sizeof(pHeaderDescriptor->hdr[1].name)); // Header's Name
		memcpy(pHeaderDescriptor->hdr[1].hdr, (void*)&aHeadertoAdd[1],
			hdrSize); //Header's Data
		pHeaderDescriptor->hdr[1].hdr_len = hdrSize;
		pHeaderDescriptor->hdr[1].hdr_hdl    = -1; //Return Value
		pHeaderDescriptor->hdr[1].is_partial = false;
		pHeaderDescriptor->hdr[1].status     = -1; // Return Parameter

		// Adding Header No3.
		strlcpy(pHeaderDescriptor->hdr[2].name, "StreamId2", sizeof(pHeaderDescriptor->hdr[2].name)); // Header's Name
		memcpy(pHeaderDescriptor->hdr[2].hdr, (void*)&aHeadertoAdd[2],
			hdrSize); //Header's Data
		pHeaderDescriptor->hdr[2].hdr_len = hdrSize;
		pHeaderDescriptor->hdr[2].hdr_hdl    = -1; //Return Value
		pHeaderDescriptor->hdr[2].is_partial = false;
		pHeaderDescriptor->hdr[2].status     = -1; // Return Parameter

		// Adding Header No4.
		strlcpy(pHeaderDescriptor->hdr[3].name, "StreamId3", sizeof(pHeaderDescriptor->hdr[3].name)); // Header's Name
		memcpy(pHeaderDescriptor->hdr[3].hdr, (void*)&aHeadertoAdd[3],
			hdrSize); //Header's Data
		pHeaderDescriptor->hdr[3].hdr_len = hdrSize;
		pHeaderDescriptor->hdr[3].hdr_hdl    = -1; //Return Value
		pHeaderDescriptor->hdr[3].is_partial = false;
		pHeaderDescriptor->hdr[3].status     = -1; // Return Parameter

		// Adding Header No5.
		strlcpy(pHeaderDescriptor->hdr[4].name, "StreamId4", sizeof(pHeaderDescriptor->hdr[4].name)); // Header's Name
		memcpy(pHeaderDescriptor->hdr[4].hdr, (void*)&aHeadertoAdd[4],
			hdrSize); //Header's Data
		pHeaderDescriptor->hdr[4].hdr_len = hdrSize;
		pHeaderDescriptor->hdr[4].hdr_hdl    = -1; //Return Value
		pHeaderDescriptor->hdr[4].is_partial = false;
		pHeaderDescriptor->hdr[4].status     = -1; // Return Parameter

		for (int i = 0; i < NUM_PACKETS; i++)
			strlcpy(sGetHeader[i].name, pHeaderDescriptor->hdr[i].name, sizeof(sGetHeader[i].name));


		if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
		{
			LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
			bRetVal = false;
			goto bail;
		}
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			if (!m_HeaderInsertion.GetHeaderHandle(&sGetHeader[i]))
			{
				LOG_MSG_ERROR(" Failed");
				bRetVal = false;
				goto bail;
			}
			LOG_MSG_DEBUG("Received Header %d Handle = 0x%x", i, sGetHeader[i].hdl);
		}

		for (int i = 0; i < NUM_PACKETS; i++)
		{
			if (!CreateBypassRoutingTable(&m_Routing, m_eIP, aBypass[i],
					IPA_CLIENT_TEST2_CONS, sGetHeader[i].hdl,&nTableHdl[i]))
			{
				LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
				bRetVal = false;
				goto bail;
			}
		}

		LOG_MSG_INFO("Creation of 5 bypass routing tables completed successfully");

		// Creating Filtering Rules
		cFilterTable0.Init(m_eIP,IPA_CLIENT_TEST_PROD, false, NUM_PACKETS);
		LOG_MSG_INFO("Creation of filtering table completed successfully");

		// Configuring Filtering Rule No.1
		cFilterTable0.GeneratePresetRule(1,sFilterRuleEntry);
		sFilterRuleEntry.at_rear = true;
		sFilterRuleEntry.flt_rule_hdl=-1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
		sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl[0]; //put here the handle corresponding to Routing Rule 1
		sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		if (
				((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
				!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
				)
		{
			LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
			bRetVal = false;
			goto bail;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n",
					cFilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,
					cFilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2
		sFilterRuleEntry.flt_rule_hdl=-1; // return Value
		sFilterRuleEntry.status = -1; // return Value
		sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl[1]; //put here the handle corresponding to Routing Rule 2
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1.
		if (
				((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
				!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR ("Adding Rule(1) to Filtering block Failed.");
			bRetVal = false;
			goto bail;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n",
					cFilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,
					cFilterTable0.ReadRuleFromTable(1)->status);
		}

		// Configuring Filtering Rule No.3
		sFilterRuleEntry.flt_rule_hdl=-1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl[2]; //put here the handle corresponding to Routing Rule 2
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
				!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR ("Adding Rule(2) to Filtering block Failed.");
			bRetVal = false;
			goto bail;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n",
					cFilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,
					cFilterTable0.ReadRuleFromTable(2)->status);
		}

		// Configuring Filtering Rule No.4
		sFilterRuleEntry.flt_rule_hdl=-1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl[3]; //put here the handle corresponding to Routing Rule 2
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0xC0A80103; // Filter DST_IP == 192.168.1.3.

		if (
				((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
				!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR ("Adding Rule(3) to Filtering block Failed.");
			bRetVal = false;
			goto bail;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n",
					cFilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,
					cFilterTable0.ReadRuleFromTable(2)->status);
		}

		// Configuring Filtering Rule No.5
		sFilterRuleEntry.flt_rule_hdl=-1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl[4]; //put here the handle corresponding to Routing Rule 2
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0xC0A80104; // Filter DST_IP == 192.168.1.4.

		if (
				((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
				!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR ("Adding Rule(4) to Filtering block Failed.");
			bRetVal = false;
			goto bail;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n",
					cFilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,
					cFilterTable0.ReadRuleFromTable(2)->status);
		}

	bail:
		Free(pHeaderDescriptor);
		LOG_MSG_STACK(
				"Leaving Function (Returning %s)", bRetVal?"True":"False");
		return bRetVal;
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregationDifferentStreamIdsTest(
				&m_UsbToIpaPipe, &m_IpaToUsbPipeAgg, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class MBIMAggregationNoInterleavingStreamIdsTest:
	public MBIMAggregationTestFixtureConf11 {
public:

	/////////////////////////////////////////////////////////////////////////////////

	MBIMAggregationNoInterleavingStreamIdsTest(bool generic_agg)
		: MBIMAggregationTestFixtureConf11(generic_agg)
	{
		if (generic_agg)
			m_name = "GenMBIMAggregationNoInterleavingStreamIdsTest";
		else
			m_name = "MBIMAggregationNoInterleavingStreamIdsTest";
		m_description = "MBIM Aggregation no interleaving stream IDs test - sends 5 packets"
				"with interleaving stream IDs (0, 1, 0, 1, 0) and receives 1 aggregated "
				"packet made of 5 NDPs";
	}

	/////////////////////////////////////////////////////////////////////////////////

	virtual bool AddRules()
	{
		m_eIP = IPA_IP_v4;
		const char aBypass[2][20] = {{"Bypass1"}, {"Bypass2"}};
		uint32_t nTableHdl[2];
		bool bRetVal = true;
		IPAFilteringTable cFilterTable0;
		struct ipa_flt_rule_add sFilterRuleEntry;
		struct ipa_ioc_get_hdr sGetHeader[2];
		uint8_t aHeadertoAdd[2][4];
		int hdrSize;

		for (int i = 0; i < 2; i++) {
			if (mGenericAgg) {
				hdrSize = 4;
				aHeadertoAdd[i][0] = 0x49;
				aHeadertoAdd[i][1] = 0x50;
				aHeadertoAdd[i][2] = 0x53;
				aHeadertoAdd[i][3] = (uint8_t)i;
			}
			else {
				hdrSize = 1;
				aHeadertoAdd[i][0] = (uint8_t)i;
			}
		}

		LOG_MSG_STACK("Entering Function");
		memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));
		for (int i = 0; i < 2; i++)
			memset(&sGetHeader[i], 0, sizeof(sGetHeader[i]));
		// Create Header:
		// Allocate Memory, populate it, and add in to the Header Insertion.
		struct ipa_ioc_add_hdr * pHeaderDescriptor = NULL;
		pHeaderDescriptor = (struct ipa_ioc_add_hdr *) calloc(1,
				sizeof(struct ipa_ioc_add_hdr)
						+ 2 * sizeof(struct ipa_hdr_add));
		if (!pHeaderDescriptor)
		{
			LOG_MSG_ERROR("calloc failed to allocate pHeaderDescriptor");
			bRetVal = false;
			goto bail;
		}

		pHeaderDescriptor->commit = true;
		pHeaderDescriptor->num_hdrs = 2;
		// Adding Header No1.
		strlcpy(pHeaderDescriptor->hdr[0].name, "StreamId0", sizeof(pHeaderDescriptor->hdr[0].name)); // Header's Name
		memcpy(pHeaderDescriptor->hdr[0].hdr, (void*)&aHeadertoAdd[0],
			hdrSize); //Header's Data
		pHeaderDescriptor->hdr[0].hdr_len = hdrSize;
		pHeaderDescriptor->hdr[0].hdr_hdl    = -1; //Return Value
		pHeaderDescriptor->hdr[0].is_partial = false;
		pHeaderDescriptor->hdr[0].status     = -1; // Return Parameter

		// Adding Header No2.
		strlcpy(pHeaderDescriptor->hdr[1].name, "StreamId1", sizeof(pHeaderDescriptor->hdr[1].name)); // Header's Name
		memcpy(pHeaderDescriptor->hdr[1].hdr, (void*)&aHeadertoAdd[1],
			hdrSize); //Header's Data
		pHeaderDescriptor->hdr[1].hdr_len = hdrSize;
		pHeaderDescriptor->hdr[1].hdr_hdl    = -1; //Return Value
		pHeaderDescriptor->hdr[1].is_partial = false;
		pHeaderDescriptor->hdr[1].status     = -1; // Return Parameter

		for (int i = 0; i < 2; i++)
			strlcpy(sGetHeader[i].name, pHeaderDescriptor->hdr[i].name, sizeof(sGetHeader[i].name));


		if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
		{
			LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
			bRetVal = false;
			goto bail;
		}
		for (int i = 0; i < 2; i++)
		{
			if (!m_HeaderInsertion.GetHeaderHandle(&sGetHeader[i]))
			{
				LOG_MSG_ERROR(" Failed");
				bRetVal = false;
				goto bail;
			}
			LOG_MSG_DEBUG("Received Header %d Handle = 0x%x", i, sGetHeader[i].hdl);
		}

		for (int i = 0; i < 2; i++)
		{
			if (!CreateBypassRoutingTable(&m_Routing, m_eIP, aBypass[i],
					IPA_CLIENT_TEST2_CONS, sGetHeader[i].hdl,&nTableHdl[i]))
			{
				LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
				bRetVal = false;
				goto bail;
			}
		}

		LOG_MSG_INFO("Creation of 2 bypass routing tables completed successfully");

		// Creating Filtering Rules
		cFilterTable0.Init(m_eIP,IPA_CLIENT_TEST_PROD, false, 2);
		LOG_MSG_INFO("Creation of filtering table completed successfully");

		// Configuring Filtering Rule No.1
		cFilterTable0.GeneratePresetRule(1,sFilterRuleEntry);
		sFilterRuleEntry.at_rear = true;
		sFilterRuleEntry.flt_rule_hdl=-1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
		sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl[0]; //put here the handle corresponding to Routing Rule 1
		sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		if (
				((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
				!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
				)
		{
			LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
			bRetVal = false;
			goto bail;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n",
					cFilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,
					cFilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2
		sFilterRuleEntry.flt_rule_hdl=-1; // return Value
		sFilterRuleEntry.status = -1; // return Value
		sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl[1]; //put here the handle corresponding to Routing Rule 2
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1.
		if (
				((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
				!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR ("Adding Rule(1) to Filtering block Failed.");
			bRetVal = false;
			goto bail;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n",
					cFilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,
					cFilterTable0.ReadRuleFromTable(1)->status);
		}

	bail:
		Free(pHeaderDescriptor);
		LOG_MSG_STACK(
				"Leaving Function (Returning %s)", bRetVal?"True":"False");
		return bRetVal;
	} // AddRules()

	/////////////////////////////////////////////////////////////////////////////////

	bool TestLogic()
	{
		return MBIMAggregationScenarios::MBIMAggregationNoInterleavingStreamIdsTest(
				&m_UsbToIpaPipe, &m_IpaToUsbPipeAgg, m_eIP);
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/* Legacy MBIM tests */

static MBIMAggregationTest mbimAggregationTest(false);
static MBIMDeaggregationTest mbimDeaggregationTest(false);
static MBIMDeaggregationOnePacketTest mbimDeaggregationOnePacketTest(false);
static MBIMDeaggregationAndAggregationTest mbimDeaggregationAndAggregationTest(false);
static MBIMMultipleDeaggregationAndAggregationTest
		mbimMultipleDeaggregationAndAggregationTest(false);
static MBIMAggregationLoopTest mbimAggregationLoopTest(false);
static MBIMDeaggregationMultipleNDPTest mbimDeaggregationMultipleNDPTest(false);
static MBIMAggregationMultiplePacketsTest mbimAggregationMultiplePacketsTest(false);
static MBIMAggregation2PipesTest mbimAggregation2PipesTest(false);
static MBIMAggregationNoInterleavingStreamIdsTest
		mbimAggregationNoInterleavingStreamIdsTest(false);
static MBIMAggregationDifferentStreamIdsTest mbimAggregationDifferentStreamIdsTest(false);
static MBIMAggregationTimeLimitTest mbimAggregationTimeLimitTest(false);
static MBIMAggregationByteLimitTest mbimAggregationByteLimitTest(false);
static MBIMAggregationTimeLimitLoopTest mbimAggregationTimeLimitLoopTest(false);
static MBIMAggregation0LimitsTest mbimAggregation0LimitsTest(false);

/* Generic Aggregation MBIM tests */

static MBIMAggregationTest genMbimAggregationTest(true);
static MBIMDeaggregationTest genMbimDeaggregationTest(true);
static MBIMDeaggregationOnePacketTest genMbimDeaggregationOnePacketTest(true);
static MBIMDeaggregationAndAggregationTest genMbimDeaggregationAndAggregationTest(true);
static MBIMMultipleDeaggregationAndAggregationTest genMbimMultipleDeaggregationAndAggregationTest(true);
static MBIMAggregationLoopTest genMbimAggregationLoopTest(true);
static MBIMDeaggregationMultipleNDPTest genMbimDeaggregationMultipleNDPTest(true);
static MBIMAggregationMultiplePacketsTest genMbimAggregationMultiplePacketsTest(true);
static MBIMAggregation2PipesTest genMbimAggregation2PipesTest(true);
static MBIMAggregationNoInterleavingStreamIdsTest genMbimAggregationNoInterleavingStreamIdsTest(true);
static MBIMAggregationDifferentStreamIdsTest genMbimAggregationDifferentStreamIdsTest(true);
static MBIMAggregationTimeLimitTest genMbimAggregationTimeLimitTest(true);
static MBIMAggregationByteLimitTest genMbimAggregationByteLimitTest(true);
static MBIMAggregationByteLimitTestFC genMbimAggregationByteLimitTestFC(true);
static MBIMAggregationDualDpTestFC genMBIMAggregationDualDpTestFC(true);
static MBIMAggregationDualDpTestFcRoutingBased genMBIMAggregationDualDpTestFcRoutingBased(true);
static MBIMAggregationTimeLimitLoopTest genMbimAggregationTimeLimitLoopTest(true);
static MBIMAggregation0LimitsTest genMbimAggregation0LimitsTest(true);

/////////////////////////////////////////////////////////////////////////////////
//                                  EOF                                      ////
/////////////////////////////////////////////////////////////////////////////////
