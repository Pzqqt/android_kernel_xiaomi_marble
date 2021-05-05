/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

#include "TLPAggregationTestFixture.h"
#include "Constants.h"
#include "TestsUtils.h"
#include "linux/msm_ipa.h"

#define NUM_PACKETS 5
#define TIME_LIMIT_NUM_PACKETS 1
#define MAX_PACKET_SIZE 1024
#define AGGREGATION_LOOP 4

int test_num = 0;

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class TLPAggregationTest: public TLPAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	TLPAggregationTest()
	{
		m_name = "TLPAggregationTest";
		m_description = "TLP Aggregation test - sends 5 packets and receives 1 "
				"aggregated packet";
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run()
	{
		bool bTestResult = true;
		//The packets that will be sent
		Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
		//The real sizes of the packets that will be sent
		int pPacketsSizes[NUM_PACKETS];
		//Buffer for the packet that will be received
		Byte pReceivedPacket[MAX_PACKET_SIZE];
		//Total size of all sent packets (this is the max size of the aggregated packet
		//minus 2 bytes for each packet in the aggregated packet)
		int nTotalPacketsSize = MAX_PACKET_SIZE - (2 * NUM_PACKETS);
		//The expected aggregated packet
		Byte pExpectedAggregatedPacket[MAX_PACKET_SIZE] = {0};

		//initialize the packets
		//example: for NUM_PACKETS = 5 and MAX_PACKET_SIZE = 1024:
		//nTotalPacketsSize will be 1024 - 5*2 = 1014
		//packets[0] size will be 1014/5 = 202 bytes of 0
		//packets[1] size will be (1014 - 202) / 5 = 162 bytes of 1
		//packets[2] size will be (1014 - 201 - 162) / 5 = 130 bytes of 2
		//packets[3] size will be (1014 - 201 - 162 - 130) / 5 = 104 bytes of 3
		//packets[4] size will be 1014 - 201 - 162 - 130 - 104 = 416 bytes of 4
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			if (NUM_PACKETS - 1 == i)
				pPacketsSizes[i] = nTotalPacketsSize;
			else
				pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
			nTotalPacketsSize -= pPacketsSizes[i];
			for (int j = 0; j < pPacketsSizes[i]; j++)
				pPackets[i][j] = i + 16*test_num;
		}
		test_num++;

		//send the packets
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
					pPacketsSizes[i]);
			int nBytesSent = m_UsbNoAggToIpaPipeAgg.Send(pPackets[i],
					pPacketsSizes[i]);
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
		int nBytesReceived = m_IpaToUsbPipeAggr.Receive(pReceivedPacket,
				MAX_PACKET_SIZE);
		if (MAX_PACKET_SIZE != nBytesReceived)
		{
			LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d "
					"bytes) failed!\n", MAX_PACKET_SIZE);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}


		//initializing the aggregated packet
		int k = 0;
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			//the first 2 bytes are the packet length in little endian
			pExpectedAggregatedPacket[k] = pPacketsSizes[i] & 0x00FF;
			pExpectedAggregatedPacket[k+1] = pPacketsSizes[i] >> 8;
			k += 2;
			for (int j = 0; j < pPacketsSizes[i]; j++)
			{
				pExpectedAggregatedPacket[k] = pPackets[i][j];
				k++;
			}
		}

		//comparing the received packet to the aggregated packet
		LOG_MSG_DEBUG("Checking sent.vs.received packet\n");

		bTestResult &= !memcmp(pExpectedAggregatedPacket, pReceivedPacket,
				sizeof(pReceivedPacket));

		return bTestResult;
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class TLPDeaggregationTest: public TLPAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	TLPDeaggregationTest()
	{
		m_name = "TLPDeaggregationTest";
		m_description = "TLP Deaggregation test - sends an aggregated packet made from"
				"5 packets and receives 5 packets";
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run()
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
		//minus 2 bytes for each packet in the aggregated packet)
		int nTotalPacketsSize = MAX_PACKET_SIZE - (2 * NUM_PACKETS);
		//The aggregated packet that will be sent
		Byte pAggregatedPacket[MAX_PACKET_SIZE] = {0};

		//initialize the packets
		//example: for NUM_PACKETS = 5 and MAX_PACKET_SIZE = 1024:
		//nTotalPacketsSize will be 1024 - 5*2 = 1014
		//packets[0] size will be 1014/5 = 202 bytes of 0
		//packets[1] size will be (1014 - 202) / 5 = 162 bytes of 1
		//packets[2] size will be (1014 - 201 - 162) / 5 = 130 bytes of 2
		//packets[3] size will be (1014 - 201 - 162 - 130) / 5 = 104 bytes of 3
		//packets[4] size will be 1014 - 201 - 162 - 130 - 104 = 416 bytes of 4
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			if (NUM_PACKETS - 1 == i)
				pPacketsSizes[i] = nTotalPacketsSize;
			else
				pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
			nTotalPacketsSize -= pPacketsSizes[i];
			for (int j = 0; j < pPacketsSizes[i]; j++)
				pExpectedPackets[i][j] = i+ 16*test_num;
		}
		test_num++;

		//initializing the aggregated packet
		int k = 0;
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			//the first 2 bytes are the packet length in little endian
			pAggregatedPacket[k] = pPacketsSizes[i] & 0x00FF;
			pAggregatedPacket[k+1] = pPacketsSizes[i] >> 8;
			k += 2;
			for (int j = 0; j < pPacketsSizes[i]; j++)
			{
				pAggregatedPacket[k] = pExpectedPackets[i][j];
				k++;
			}
		}

		//send the aggregated packet
		LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d bytes)\n",
				sizeof(pAggregatedPacket));
		int nBytesSent = m_UsbDeaggToIpaPipeNoAgg.Send(pAggregatedPacket,
				sizeof(pAggregatedPacket));
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
			int nBytesReceived = m_IpaToUsbPipeNoAgg.Receive(pReceivedPackets[i],
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
			bTestResult &= !memcmp(pExpectedPackets[i], pReceivedPackets[i], pPacketsSizes[i]);

		return bTestResult;
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


class TLPDeaggregationAndAggregationTest: public TLPAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	TLPDeaggregationAndAggregationTest()
	{
		m_name = "TLPDeaggregationAndAggregationTest";
		m_description = "TLP Deaggregation and Aggregation test - sends an aggregated "
				"packet made from 5 packets and receives the same aggregated packet";
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run()
	{
		bool bTestResult = true;
		//The packets that the aggregated packet will be made of
		Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
		//The real sizes of the packets that the aggregated packet will be made of
		int pPacketsSizes[NUM_PACKETS];
		//Buffers for the packets that will be received
		Byte pReceivedPacket[MAX_PACKET_SIZE];
		//Total size of all the packets that the aggregated packet will be made of
		//(this is the max size of the aggregated packet
		//minus 2 bytes for each packet in the aggregated packet)
		int nTotalPacketsSize = MAX_PACKET_SIZE - (2 * NUM_PACKETS);
		//The aggregated packet that will be sent
		Byte pAggregatedPacket[MAX_PACKET_SIZE] = {0};

		//initialize the packets
		//example: for NUM_PACKETS = 5 and MAX_PACKET_SIZE = 1024:
		//nTotalPacketsSize will be 1024 - 5*2 = 1014
		//packets[0] size will be 1014/5 = 202 bytes of 0
		//packets[1] size will be (1014 - 202) / 5 = 162 bytes of 1
		//packets[2] size will be (1014 - 201 - 162) / 5 = 130 bytes of 2
		//packets[3] size will be (1014 - 201 - 162 - 130) / 5 = 104 bytes of 3
		//packets[4] size will be 1014 - 201 - 162 - 130 - 104 = 416 bytes of 4
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			if (NUM_PACKETS - 1 == i)
				pPacketsSizes[i] = nTotalPacketsSize;
			else
				pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
			nTotalPacketsSize -= pPacketsSizes[i];
			for (int j = 0; j < pPacketsSizes[i]; j++)
				pPackets[i][j] = i+ 16*test_num;
		}
		test_num++;

		//initializing the aggregated packet
		int k = 0;
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			//the first 2 bytes are the packet length in little endian
			pAggregatedPacket[k] = pPacketsSizes[i] & 0x00FF;
			pAggregatedPacket[k+1] = pPacketsSizes[i] >> 8;
			k += 2;
			for (int j = 0; j < pPacketsSizes[i]; j++)
			{
				pAggregatedPacket[k] = pPackets[i][j];
				k++;
			}
		}

		//send the aggregated packet
		LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d bytes)\n",
				sizeof(pAggregatedPacket));
		int nBytesSent = m_UsbDeaggToIpaPipeAgg.Send(pAggregatedPacket,
				sizeof(pAggregatedPacket));
		if (sizeof(pAggregatedPacket) != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d bytes)"
					" failed!\n", sizeof(pAggregatedPacket));
			return false;
		}

		//receive the aggregated packet
		LOG_MSG_DEBUG("Reading aggregated packet from the USB pipe(%d bytes "
				"should be there)\n", sizeof(pAggregatedPacket));
		int nBytesReceived = m_IpaToUsbPipeAggr.Receive(pReceivedPacket,
				sizeof(pAggregatedPacket));
		if (sizeof(pAggregatedPacket) != nBytesReceived)
		{
			LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d "
					"bytes) failed!\n", sizeof(pAggregatedPacket));
			LOG_MSG_DEBUG("Received %d bytes\n", nBytesReceived);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}
print_buff(pReceivedPacket, nBytesReceived);

		//comparing the received packet to the aggregated packet
		LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
		bTestResult &= !memcmp(pAggregatedPacket, pReceivedPacket,
				sizeof(pReceivedPacket));


		return bTestResult;
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class TLPAggregationLoopTest: public TLPAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	TLPAggregationLoopTest()
	{
		m_name = "TLPAggregationLoopTest";
		m_description = "TLP Aggregation Loop test - sends 5 packets and expects to"
				"receives 1 aggregated packet a few times";
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run()
	{
		//The packets that will be sent
		Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
		//The real sizes of the packets that will be sent
		int pPacketsSizes[NUM_PACKETS];
		//Buffer for the packet that will be received
		Byte pReceivedPacket[MAX_PACKET_SIZE];
		//Total size of all sent packets (this is the max size of the aggregated packet
		//minus 2 bytes for each packet in the aggregated packet)
		int nTotalPacketsSize = MAX_PACKET_SIZE - (2 * NUM_PACKETS);
		//The expected aggregated packet
		Byte pExpectedAggregatedPacket[MAX_PACKET_SIZE] = {0};

		//initialize the packets
		//example: for NUM_PACKETS = 5 and MAX_PACKET_SIZE = 1024:
		//nTotalPacketsSize will be 1024 - 5*2 = 1014
		//packets[0] size will be 1014/5 = 202 bytes of 0
		//packets[1] size will be (1014 - 202) / 5 = 162 bytes of 1
		//packets[2] size will be (1014 - 201 - 162) / 5 = 130 bytes of 2
		//packets[3] size will be (1014 - 201 - 162 - 130) / 5 = 104 bytes of 3
		//packets[4] size will be 1014 - 201 - 162 - 130 - 104 = 416 bytes of 4
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			if (NUM_PACKETS - 1 == i)
				pPacketsSizes[i] = nTotalPacketsSize;
			else
				pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
			nTotalPacketsSize -= pPacketsSizes[i];
			for (int j = 0; j < pPacketsSizes[i]; j++)
				pPackets[i][j] = i+ 16*test_num;
		}
		test_num++;

		//initializing the aggregated packet
		int k = 0;
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			//the first 2 bytes are the packet length in little endian
			pExpectedAggregatedPacket[k] = pPacketsSizes[i] & 0x00FF;
			pExpectedAggregatedPacket[k+1] = pPacketsSizes[i] >> 8;
			k += 2;
			for (int j = 0; j < pPacketsSizes[i]; j++)
			{
				pExpectedAggregatedPacket[k] = pPackets[i][j];
				k++;
			}
		}

		for (int j = 0; j < AGGREGATION_LOOP; j++)
		{
			//send the packets
			for (int i = 0; i < NUM_PACKETS; i++)
			{
				LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n",
						i, pPacketsSizes[i]);
				int nBytesSent = m_UsbNoAggToIpaPipeAgg.Send(pPackets[i],
						pPacketsSizes[i]);
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
			int nBytesReceived = m_IpaToUsbPipeAggr.Receive(pReceivedPacket,
					MAX_PACKET_SIZE);
			if (MAX_PACKET_SIZE != nBytesReceived)
			{
				LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d "
						"bytes) failed!\n", MAX_PACKET_SIZE);
				return false;
			}

			//comparing the received packet to the aggregated packet
			LOG_MSG_DEBUG("Checking sent.vs.received packet\n");

			if (0 != memcmp(pExpectedAggregatedPacket, pReceivedPacket,
					sizeof(pReceivedPacket)))
			{
				LOG_MSG_DEBUG("Comparison of packet %d failed!\n", j);
				return false;
			}
		}

		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class TLPAggregationTimeLimitTest: public TLPAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	TLPAggregationTimeLimitTest()
	{
		m_name = "TLPAggregationTimeLimitTest";
		m_description = "TLP Aggregation time limit test - sends 1 packet "
				"smaller than the byte limit and receives 1 aggregated packet";
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run()
	{
		bool bTestResult = true;
		//The packets that will be sent
		Byte pPackets[TIME_LIMIT_NUM_PACKETS][MAX_PACKET_SIZE];
		//The real sizes of the packets that will be sent
		int pPacketsSizes[TIME_LIMIT_NUM_PACKETS];
		//Buffer for the packet that will be received
		Byte pReceivedPacket[MAX_PACKET_SIZE] = {0};
		//The expected aggregated packet
		Byte pExpectedAggregatedPacket[MAX_PACKET_SIZE] = {0};
		//Size of aggregated packet
		int nTotalPacketsSize = 0;

		//initialize the packets
		for (int i = 0; i < TIME_LIMIT_NUM_PACKETS; i++)
		{
			pPacketsSizes[i] = i + 1;
			nTotalPacketsSize += pPacketsSizes[i] + 2; //size of the packet + 2 bytes for length
			for (int j = 0; j < pPacketsSizes[i]; j++)
				pPackets[i][j] = i+ 16*test_num;
		}
		test_num++;

		//send the packets
		for (int i = 0; i < TIME_LIMIT_NUM_PACKETS; i++)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
					pPacketsSizes[i]);
			int nBytesSent = m_UsbNoAggToIpaPipeAggTime.Send(pPackets[i],
					pPacketsSizes[i]);
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
		int nBytesReceived = m_IpaToUsbPipeAggTime.Receive(pReceivedPacket,
				nTotalPacketsSize);
		if (nTotalPacketsSize != nBytesReceived)
		{
			LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d "
					"bytes) failed!\n", nTotalPacketsSize);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}

		//initializing the aggregated packet
		int k = 0;
		for (int i = 0; i < TIME_LIMIT_NUM_PACKETS; i++)
		{
			//the first 2 bytes are the packet length in little endian
			pExpectedAggregatedPacket[k] = pPacketsSizes[i] & 0x00FF;
			pExpectedAggregatedPacket[k+1] = pPacketsSizes[i] >> 8;
			k += 2;
			for (int j = 0; j < pPacketsSizes[i]; j++)
			{
				pExpectedAggregatedPacket[k] = pPackets[i][j];
				k++;
			}
		}

		//comparing the received packet to the aggregated packet
		LOG_MSG_DEBUG("Checking sent.vs.received packet\n");

		bTestResult &= !memcmp(pExpectedAggregatedPacket, pReceivedPacket,
				sizeof(pReceivedPacket));

		return bTestResult;
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class TLPAggregationByteLimitTest: public TLPAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	TLPAggregationByteLimitTest()
	{
		m_name = "TLPAggregationByteLimitTest";
		m_description = "TLP Aggregation byte limit test - sends 2 packets that together "
				"are larger than the byte limit ";
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run()
	{
		bool bTestResult = true;
		//The packets that will be sent
		Byte pPackets[2][MAX_PACKET_SIZE];
		//The real sizes of the packets that will be sent
		int pPacketsSizes[2];
		//Buffer for the packet that will be received
		Byte pReceivedPacket[2*MAX_PACKET_SIZE] = {0};
		//The expected aggregated packet
		Byte pExpectedAggregatedPacket[2*MAX_PACKET_SIZE] = {0};
		//Size of aggregated packet
		int nTotalPacketsSize = 0;

		//initialize the packets
		for (int i = 0; i < 2; i++)
		{
			pPacketsSizes[i] = (MAX_PACKET_SIZE / 2) + 10;
			nTotalPacketsSize += pPacketsSizes[i] + 2;
			for (int j = 0; j < pPacketsSizes[i]; j++)
				pPackets[i][j] = i+ 16*test_num;
		}
		test_num++;

		//send the packets
		for (int i = 0; i < 2; i++)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
					pPacketsSizes[i]);
			int nBytesSent = m_UsbNoAggToIpaPipeAgg.Send(pPackets[i],
					pPacketsSizes[i]);
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
		int nBytesReceived = m_IpaToUsbPipeAggr.Receive(pReceivedPacket,
				nTotalPacketsSize);
		if (nTotalPacketsSize != nBytesReceived)
		{
			LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d"
					" bytes) failed!\n", nTotalPacketsSize);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}


		//initializing the aggregated packet
		int k = 0;
		for (int i = 0; i < 2; i++)
		{
			//the first 2 bytes are the packet length in little endian
			pExpectedAggregatedPacket[k] = pPacketsSizes[i] & 0x00FF;
			pExpectedAggregatedPacket[k+1] = pPacketsSizes[i] >> 8;
			k += 2;
			for (int j = 0; j < pPacketsSizes[i]; j++)
			{
				pExpectedAggregatedPacket[k] = pPackets[i][j];
				k++;
			}
		}

		//comparing the received packet to the aggregated packet
		LOG_MSG_DEBUG("Checking sent.vs.received packet\n");

		bTestResult &= !memcmp(pExpectedAggregatedPacket, pReceivedPacket,
				sizeof(pReceivedPacket));

		return bTestResult;
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class TLPAggregation2PipesTest: public TLPAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	TLPAggregation2PipesTest()
	{
		m_name = "TLPAggregation2PipesTest";
		m_description = "TLP Aggregation 2 pipes test - sends 3 packets from one pipe"
				"and an aggregated packet made of 2 packets from another pipe and "
				"receives 1 aggregated packet made of all 5 packets";
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run()
	{
		bool bTestResult = true;
		//The packets that will be sent
		Byte pPackets[NUM_PACKETS][MAX_PACKET_SIZE];
		//The real sizes of the packets that will be sent
		int pPacketsSizes[NUM_PACKETS];
		//Buffer for the packet that will be received
		Byte pReceivedPacket[MAX_PACKET_SIZE];
		//Total size of all sent packets (this is the max size of the aggregated packet
		//minus 2 bytes for each packet in the aggregated packet)
		int nTotalPacketsSize = MAX_PACKET_SIZE - (2 * NUM_PACKETS);
		//The expected aggregated packet
		Byte pExpectedAggregatedPacket[MAX_PACKET_SIZE] = {0};
		//The aggregated packet that will be sent
		Byte pAggregatedPacket[MAX_PACKET_SIZE] = {0};
		//The size of the sent aggregated packet
		int nAggregatedPacketSize = 0;

		//initialize the packets
		//example: for NUM_PACKETS = 5 and MAX_PACKET_SIZE = 1024:
		//nTotalPacketsSize will be 1024 - 5*2 = 1014
		//packets[0] size will be 1014/5 = 202 bytes of 0
		//packets[1] size will be (1014 - 202) / 5 = 162 bytes of 1
		//packets[2] size will be (1014 - 201 - 162) / 5 = 130 bytes of 2
		//packets[3] size will be (1014 - 201 - 162 - 130) / 5 = 104 bytes of 3
		//packets[4] size will be 1014 - 201 - 162 - 130 - 104 = 416 bytes of 4
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			if (NUM_PACKETS - 1 == i)
				pPacketsSizes[i] = nTotalPacketsSize;
			else
				pPacketsSizes[i] = nTotalPacketsSize / NUM_PACKETS;
			nTotalPacketsSize -= pPacketsSizes[i];
			for (int j = 0; j < pPacketsSizes[i]; j++)
				pPackets[i][j] = i+ 16*test_num;
		}
		test_num++;

		//initializing the aggregated packet
		int k = 0;
		for (int i = 0; i < 2; i++)
		{
			nAggregatedPacketSize += pPacketsSizes[i] + 2;
			//the first 2 bytes are the packet length in little endian
			pAggregatedPacket[k] = pPacketsSizes[i] & 0x00FF;
			pAggregatedPacket[k+1] = pPacketsSizes[i] >> 8;
			k += 2;
			for (int j = 0; j < pPacketsSizes[i]; j++)
			{
				pAggregatedPacket[k] = pPackets[i][j];
				k++;
			}
		}

		//send the aggregated packet
		LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d "
				"bytes)\n", nAggregatedPacketSize);
		int nBytesSent = m_UsbDeaggToIpaPipeAgg.Send(pAggregatedPacket,
				nAggregatedPacketSize);
		if (nAggregatedPacketSize != nBytesSent)
		{
			LOG_MSG_DEBUG("Sending aggregated packet into the USB pipe(%d "
					"bytes) failed!\n", nAggregatedPacketSize);
			return false;
		}

		//send the packets
		for (int i = 2; i < NUM_PACKETS; i++)
		{
			LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n", i,
					pPacketsSizes[i]);
			int nBytesSent = m_UsbNoAggToIpaPipeAgg.Send(pPackets[i],
					pPacketsSizes[i]);
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
		int nBytesReceived = m_IpaToUsbPipeAggr.Receive(pReceivedPacket,
				MAX_PACKET_SIZE);
		if (MAX_PACKET_SIZE != nBytesReceived)
		{
			LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d "
					"bytes) failed!\n", MAX_PACKET_SIZE);
			print_buff(pReceivedPacket, nBytesReceived);
			return false;
		}


		//initializing the aggregated packet
		k = 0;
		for (int i = 0; i < NUM_PACKETS; i++)
		{
			//the first 2 bytes are the packet length in little endian
			pExpectedAggregatedPacket[k] = pPacketsSizes[i] & 0x00FF;
			pExpectedAggregatedPacket[k+1] = pPacketsSizes[i] >> 8;
			k += 2;
			for (int j = 0; j < pPacketsSizes[i]; j++)
			{
				pExpectedAggregatedPacket[k] = pPackets[i][j];
				k++;
			}
		}

		//comparing the received packet to the aggregated packet
		LOG_MSG_DEBUG("Checking sent.vs.received packet\n");

		bTestResult &= !memcmp(pExpectedAggregatedPacket, pReceivedPacket,
				sizeof(pReceivedPacket));

		return bTestResult;
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class TLPAggregationTimeLimitLoopTest: public TLPAggregationTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	TLPAggregationTimeLimitLoopTest()
	{
		m_name = "TLPAggregationTimeLimitLoopTest";
		m_description = "TLP Aggregation time limit loop test - sends 1 packet "
				"smaller than the byte limit and receives 1 aggregated packets";
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run()
	{
		//The packets that will be sent
		Byte pPackets[TIME_LIMIT_NUM_PACKETS][MAX_PACKET_SIZE];
		//The real sizes of the packets that will be sent
		int pPacketsSizes[TIME_LIMIT_NUM_PACKETS];
		//Buffer for the packet that will be received
		Byte pReceivedPacket[MAX_PACKET_SIZE] = {0};
		//The expected aggregated packet
		Byte pExpectedAggregatedPacket[MAX_PACKET_SIZE] = {0};
		//Size of aggregated packet
		int nTotalPacketsSize = 0;

		//initialize the packets
		for (int i = 0; i < TIME_LIMIT_NUM_PACKETS; i++)
		{
			pPacketsSizes[i] = i + 1;
			nTotalPacketsSize += pPacketsSizes[i] + 2; //size of the packet + 2 bytes for length
			for (int j = 0; j < pPacketsSizes[i]; j++)
				pPackets[i][j] = i+ 16*test_num;
		}
		test_num++;

		for (int n = 0; n < NUM_PACKETS; n++)
		{
			//send the packets
			for (int i = 0; i < TIME_LIMIT_NUM_PACKETS; i++)
			{
				LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d bytes)\n",
						i, pPacketsSizes[i]);
				int nBytesSent = m_UsbNoAggToIpaPipeAggTime.Send(pPackets[i],
						pPacketsSizes[i]);
				if (pPacketsSizes[i] != nBytesSent)
				{
					LOG_MSG_DEBUG("Sending packet %d into the USB pipe(%d "
							"bytes) failed!\n", i, pPacketsSizes[i]);
					return false;
				}
			}

			//receive the aggregated packet
			LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be "
					"there)\n", nTotalPacketsSize);
			int nBytesReceived = m_IpaToUsbPipeAggTime.Receive(pReceivedPacket,
					nTotalPacketsSize);
			if (nTotalPacketsSize != nBytesReceived)
			{
				LOG_MSG_DEBUG("Receiving aggregated packet from the USB pipe(%d "
						"bytes) failed!\n", nTotalPacketsSize);
				print_buff(pReceivedPacket, nBytesReceived);
				return false;
			}

			//initializing the aggregated packet
			int k = 0;
			for (int i = 0; i < TIME_LIMIT_NUM_PACKETS; i++)
			{
				//the first 2 bytes are the packet length in little endian
				pExpectedAggregatedPacket[k] = pPacketsSizes[i] & 0x00FF;
				pExpectedAggregatedPacket[k+1] = pPacketsSizes[i] >> 8;
				k += 2;
				for (int j = 0; j < pPacketsSizes[i]; j++)
				{
					pExpectedAggregatedPacket[k] = pPackets[i][j];
					k++;
				}
			}

			//comparing the received packet to the aggregated packet
			LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
			if (0 != memcmp(pExpectedAggregatedPacket, pReceivedPacket,
					sizeof(pReceivedPacket)))
			{
				LOG_MSG_DEBUG("Comparison of packet %d failed!\n", n);
				return false;
			}
		}

		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////
};


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//Those tests should be run with configuration number 8.
//Please look at the Fixture for more configurations update.
static TLPAggregationTest tlpAggregationTest;
static TLPDeaggregationTest tlpDeaggregationTest;
static TLPAggregationLoopTest tlpAggregationLoopTest;
static TLPAggregationTimeLimitTest tlpAggregationTimeLimitTest;
static TLPAggregationByteLimitTest tlpAggregationByteLimitTest;
static TLPAggregation2PipesTest tlpAggregation2PipesTest;
static TLPAggregationTimeLimitLoopTest tlpAggregationTimeLimitLoopTest;
static TLPDeaggregationAndAggregationTest tlpDeaggregationAndAggregationTest;

/////////////////////////////////////////////////////////////////////////////////
//                                  EOF                                      ////
/////////////////////////////////////////////////////////////////////////////////
