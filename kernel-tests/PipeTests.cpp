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

#include "PipeTestFixture.h"
#include "Constants.h"
#include "TestsUtils.h"
#include "linux/msm_ipa.h"

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

class PipeTestRawTransfer: public PipeTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	PipeTestRawTransfer() {
		m_name = "PipeTestRawTransfer";
		m_description = "This test will be using the Pipe raw transfer ability";
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run() {
		bool bTestResult = true;
		Byte pIpPacket[] = { 0x01, 0x02, 0x03, 0x04 }; //This packet will be sent(It can be replaced by a real IP packet).
		Byte pIpPacketReceive[sizeof(pIpPacket)] = { 0 }; //This buffer will be used in order to store the received packet.

		//Send the raw IP packet(which is a 4 arbitrary bytes) without header addition by the Pipe
		LOG_MSG_DEBUG(
				"Sending packet into the USB pipe(%d bytes)\n", sizeof(pIpPacket));
		int nBytesSent = m_UsbToIpaPipe.Send(pIpPacket, sizeof(pIpPacket));
		if (sizeof(pIpPacket) != nBytesSent) {
			return false;
		}

		//Receive the raw IP packet(which is a 4 arbitrary bytes) without header removal by the Pipe
		LOG_MSG_DEBUG(
				"Reading packet from the USB pipe(%d bytes should be there)\n", sizeof(pIpPacketReceive));
		int nBytesReceived = m_IpaToUsbPipe.Receive(pIpPacketReceive,
				sizeof(pIpPacketReceive));
		if (sizeof(pIpPacketReceive) != nBytesReceived) {
			return false;
		}
		for (int i = 0; i < nBytesReceived; i++) {
			LOG_MSG_DEBUG("0x%02x\n", pIpPacketReceive[i]);
		}

		//Check that the sent IP packet is equal to the received IP packet.
		LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
		bTestResult &= !memcmp(pIpPacket, pIpPacketReceive, sizeof(pIpPacket));

		return bTestResult;
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//This test will be using the Pipe Add-Header transfer ability(and not header insertion
class PipeTestAddHeader: public PipeTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	PipeTestAddHeader() {
		m_name = "PipeTestAddHeader";
		m_description = "Add header to USB pipe and receive it without removing the header";
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run() {
		Byte pIpPacketSend[4] = { 0x01, 0x02, 0x03, 0x04 };
		int nReceivedPacketSize = sizeof(pIpPacketSend)
				+ m_IpaToUsbPipe.GetHeaderLengthAdd();
		Byte *pPacketReceive = new Byte[nReceivedPacketSize];

		LOG_MSG_DEBUG("Sending packet into the USB pipe(%d bytes - no header size)\n", sizeof(pIpPacketSend));
		int nRetValSend = m_UsbToIpaPipe.AddHeaderAndSend(pIpPacketSend,
				sizeof(pIpPacketSend));
		LOG_MSG_DEBUG("Result of AddHeaderAndSend = %d\n", nRetValSend);

		LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes - including header)\n", nReceivedPacketSize);
		int nRetValReceive = m_IpaToUsbPipe.Receive(pPacketReceive,
				nReceivedPacketSize);
		LOG_MSG_DEBUG("Result of Receive = %d\n", nRetValReceive);

		if (nReceivedPacketSize != nRetValReceive) {
			delete[] pPacketReceive;
			LOG_MSG_ERROR(
					"Size of received packet is not as expected - %d\n", nRetValReceive);
			return false;
		}

		bool bHeaderCmp = !memcmp(pPacketReceive, Pipe::m_pUsbHeader,
				m_IpaToUsbPipe.GetHeaderLengthAdd());
		LOG_MSG_DEBUG("bHeaderCmp - %s\n", bHeaderCmp ? "True" : "False");

		bool bIpCmp = !memcmp(pPacketReceive + m_IpaToUsbPipe.GetHeaderLengthAdd(),
				pIpPacketSend, sizeof(pIpPacketSend));
		LOG_MSG_DEBUG("bIpCmp - %s\n", bIpCmp ? "True" : "False");

		delete[] pPacketReceive;
		return bHeaderCmp && bIpCmp;
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//This test will be using the Pipe Remove-Header transfer ability(and header addition)
class PipeTestAddAndRemoveHeader: public PipeTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	PipeTestAddAndRemoveHeader() {
		m_name = "PipeTestAddAndRemoveHeader";
		m_description = "This test will be using the Pipe Remove-Header transfer ability(and header addition)";
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run() {
		Byte pIpPacketSend[4] = { 0x01, 0x02, 0x03, 0x04 };
		int nToBeReceivedPacketSize = sizeof(pIpPacketSend)
				+ m_IpaToUsbPipe.GetHeaderLengthAdd();
		Byte *pPacketReceive = new Byte[nToBeReceivedPacketSize];

		LOG_MSG_DEBUG("Sending packet into the USB pipe(%d bytes - no header size)\n", sizeof(pIpPacketSend));
		int nBytesSent = m_UsbToIpaPipe.AddHeaderAndSend(pIpPacketSend,
				sizeof(pIpPacketSend));
		LOG_MSG_DEBUG("nBytesSent of AddHeaderAndSend = %d\n", nBytesSent);

		LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes - including header)\n", nToBeReceivedPacketSize);
		int nBytesReceived = m_IpaToUsbPipe.Receive(pPacketReceive,
				nToBeReceivedPacketSize);
		LOG_MSG_DEBUG("nBytesReceived = %d\n", nBytesReceived);

		if (nToBeReceivedPacketSize != nBytesReceived) {
			delete[] pPacketReceive;
			LOG_MSG_ERROR("Size of received packet is not as expected - %d\n", nBytesReceived);
			return false;
		}

		bool bHeaderCmp = !memcmp(pPacketReceive, Pipe::m_pUsbHeader,
				m_IpaToUsbPipe.GetHeaderLengthAdd());
		LOG_MSG_DEBUG("bHeaderCmp - %s\n", bHeaderCmp ? "True" : "False");

		bool bIpCmp = !memcmp(pPacketReceive + m_IpaToUsbPipe.GetHeaderLengthAdd(),
				pIpPacketSend, sizeof(pIpPacketSend));
		LOG_MSG_DEBUG("bIpCmp - %s\n", bIpCmp ? "True" : "False");

		delete[] pPacketReceive;
		return bHeaderCmp && bIpCmp;
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//This test will try to send big chuck of data and check if the Data FIFO is working correct
class PipeTestDataFifoOverflow: public PipeTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	PipeTestDataFifoOverflow() {
		m_name = "PipeTestDataFifoOverflow";
		m_description = "Send many big packets over the IPA. there will be toggle of send/receive";
		m_runInRegression = false; // Test is very long thus makes a problem in regression testing
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run() {
		bool bTestResult = true;
		int nPacketByteSize;
		int nTotalDataSent = 0;
		int nTestsMaxDataSend = 3 * 0x6400;
		int nIterationNumber = 0;
		Byte *pIpPacket;
		Byte *pIpPacketReceive;
		srand(123); //set some constant seed value in order to be able to reproduce problems.

		//send and receive many packets(nIterations)
		while (nTotalDataSent < nTestsMaxDataSend) {
			//get random values for this packet.
			nPacketByteSize = (rand() % 400) + 200;
			pIpPacket = new Byte[nPacketByteSize];

			for (int j = 0; j < nPacketByteSize; j++) {
				pIpPacket[j] = rand() % 0x100;
			}

			//Send the raw IP packet(which is a 4 arbitrary bytes) without header addition by the Pipe
			LOG_MSG_DEBUG(
					"Iteration number %d(0x%08x/0x%08x data):\n", nIterationNumber++, nTotalDataSent, nTestsMaxDataSend);
			LOG_MSG_DEBUG(
					"Sending packet into the USB pipe(%d bytes)\n", nPacketByteSize);
			int nBytesSent = m_UsbToIpaPipe.Send(pIpPacket, nPacketByteSize);
			if (nPacketByteSize != nBytesSent) {
				delete[] pIpPacket;
				LOG_MSG_ERROR("Could not send the whole packet - nTotalDataSent = 0x%08x\n", nTotalDataSent);
				return false;
			}

			//Receive the raw IP packet(which is a 4 arbitrary bytes) without header removal by the Pipe
			pIpPacketReceive = new Byte[nPacketByteSize];
			LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be there)\n", nPacketByteSize);
			int nBytesReceived = m_IpaToUsbPipe.Receive(pIpPacketReceive,
					nPacketByteSize);
			if (nPacketByteSize != nBytesReceived) {
				delete[] pIpPacket;
				delete[] pIpPacketReceive;
				LOG_MSG_ERROR("Could not read the whole packet - nTotalDataSent = 0x%08x\n", nTotalDataSent);
				return false;
			}
			for (int j = 0; j < nBytesReceived; j++) {
				LOG_MSG_DEBUG("0x%02x\n", pIpPacketReceive[j]);
			}

			//Check that the sent IP packet is equal to the received IP packet.
			LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
			bTestResult &= !memcmp(pIpPacket, pIpPacketReceive,
					nPacketByteSize);
			if (true != bTestResult) {
				delete[] pIpPacketReceive;
				delete[] pIpPacket;
				LOG_MSG_ERROR("Send != Received  - nTotalDataSent = 0x%08x\n", nTotalDataSent);
				return false;
			}
			nTotalDataSent += nPacketByteSize;
			delete[] pIpPacket;
			delete[] pIpPacketReceive;
		}
		LOG_MSG_DEBUG("Great success - nTotalDataSent = 0x%08x\n", nTotalDataSent);
		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//This test will try to many little chuck of data and check if the Descriptor FIFO is working correct
class PipeTestDescriptorFifoOverflow: public PipeTestFixture {
public:

	/////////////////////////////////////////////////////////////////////////////////

	PipeTestDescriptorFifoOverflow() {
		m_name = "PipeTestDescriptorFifoOverflow";
		m_description = "Send many small packets over the IPA. there will be toggle of send/receive";
		m_runInRegression = false; // Test is very long thus makes a problem in regression testing
	}

	/////////////////////////////////////////////////////////////////////////////////

	bool Run() {
		bool bTestResult = true;
		int nPacketByteSize;
		int nTotalPacketSent = 0;
		int nTestsMaxDescriptors = 3 * 0x1000;
		int nIterationNumber = 0;
		Byte *pIpPacket;
		Byte *pIpPacketReceive;
		srand(123); //set some constant seed value in order to be able to reproduce problems.

		//send and receive many packets(nIterations)
		while (nTotalPacketSent < nTestsMaxDescriptors) {
			//get random values for this packet.
			nPacketByteSize = (rand() % 10) + 1;
			pIpPacket = new Byte[nPacketByteSize];

			for (int j = 0; j < nPacketByteSize; j++) {
				pIpPacket[j] = rand() % 0x100;
			}

			//Send the raw IP packet(which is a 4 arbitrary bytes) without header addition by the Pipe
			LOG_MSG_DEBUG("Iteration number %d(0x%08x/0x%08x packets):\n", nIterationNumber++, nTotalPacketSent, nTestsMaxDescriptors);
			LOG_MSG_DEBUG("Sending packet into the USB pipe(%d bytes)\n", nPacketByteSize);
			int nBytesSent = m_UsbToIpaPipe.Send(pIpPacket, nPacketByteSize);
			if (nPacketByteSize != nBytesSent) {
				delete[] pIpPacket;
				LOG_MSG_ERROR("Could not send the whole packet - nTotalPacketSent = 0x%08x\n", nTotalPacketSent);
				return false;
			}

			//Receive the raw IP packet(which is a 4 arbitrary bytes) without header removal by the Pipe
			pIpPacketReceive = new Byte[nPacketByteSize];
			LOG_MSG_DEBUG("Reading packet from the USB pipe(%d bytes should be there)\n", nPacketByteSize);
			int nBytesReceived = m_IpaToUsbPipe.Receive(pIpPacketReceive,
					nPacketByteSize);
			if (nPacketByteSize != nBytesReceived) {
				delete[] pIpPacketReceive;
				LOG_MSG_ERROR("Could not read the whole packet - nTotalPacketSent = 0x%08x\n", nTotalPacketSent);
				return false;
			}
			for (int j = 0; j < nBytesReceived; j++) {
				LOG_MSG_DEBUG("0x%02x\n", pIpPacketReceive[j]);
			}

			//Check that the sent IP packet is equal to the received IP packet.
			LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
			bTestResult &= !memcmp(pIpPacket, pIpPacketReceive,
					nPacketByteSize);
			if (true != bTestResult) {
				delete[] pIpPacketReceive;
				delete[] pIpPacket;
				LOG_MSG_ERROR("Send != Received  - nTotalPacketSent = 0x%08x\n", nTotalPacketSent);
				return false;
			}
			nTotalPacketSent++;
			delete[] pIpPacket;
			delete[] pIpPacketReceive;
		}
		LOG_MSG_DEBUG("Great success - nTotalPacketSent = 0x%08x\n", nTotalPacketSent);
		return true;
	}

	/////////////////////////////////////////////////////////////////////////////////

};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

#define HOLB_TEST_PACKETS_MAX_NUM 50
class PipeTestHolb: public PipeTestFixture {

public:

	PipeTestHolb() {
		m_name = "PipeTestHolb";
		m_description = "This test will check the HOLB function";
	}

	bool Run() {
		int nPacketsToSend = HOLB_TEST_PACKETS_MAX_NUM;
		int nBytesReceived;
		int nBytesSentInLastPacket;
		int i;
		Byte pIpPacket[] = { 0x01, 0x02, 0x03, 0x04 }; //This packet will be sent(It can be replaced by a real IP packet).
		Byte pIpPacketReceive[sizeof(pIpPacket) * HOLB_TEST_PACKETS_MAX_NUM] = { 0 }; //This buffer will be used in order to store the received packet.

		m_IpaToUsbPipe.DisableHolb();

		//Send the IP packets
		LOG_MSG_DEBUG("Sending %d packets of %d bytes into the USB pipe\n",
					  nPacketsToSend, sizeof(pIpPacket));
		for (i = 0; i < nPacketsToSend; i++) {
			nBytesSentInLastPacket = m_UsbToIpaPipe.Send(pIpPacket, sizeof(pIpPacket));
			if (sizeof(pIpPacket) != nBytesSentInLastPacket) {
				LOG_MSG_ERROR("Failed sending the packet %d to m_UsbToIpaPipe", i);
				return false;
			}
		}

		//Receive all the raw IP packets (which are a 4 arbitrary bytes)
		LOG_MSG_DEBUG(
				"Reading packets from the USB pipe (%dx%d bytes should be there)\n",
				sizeof(pIpPacket), nPacketsToSend);
		for (i = 0; i < nPacketsToSend; i++) {
			nBytesReceived = m_IpaToUsbPipe.Receive(pIpPacketReceive, sizeof(pIpPacketReceive));
			if ((int)sizeof(pIpPacket) != nBytesReceived) {
				LOG_MSG_ERROR("Failed with HOLB disabled! Packet #%d: Expected %d Bytes, got %d Bytes",
							  i, sizeof(pIpPacket), nBytesReceived);
				return false;
			}
		}

		// Enable HOLB
		m_IpaToUsbPipe.EnableHolb(0);

		//Send the IP packets
		LOG_MSG_DEBUG("Sending %d packets of %d bytes into the USB pipe\n",
					  nPacketsToSend, sizeof(pIpPacket));
		for (i = 0; i < nPacketsToSend; i++) {
			nBytesSentInLastPacket = m_UsbToIpaPipe.Send(pIpPacket, sizeof(pIpPacket));
			if (sizeof(pIpPacket) != nBytesSentInLastPacket) {
				LOG_MSG_ERROR("Failed sending the packet %d to m_UsbToIpaPipe", i);
				return false;
			}
		}

		// Receive the raw IP packets (which are a 4 arbitrary bytes)
		// that fit into the FIFO before the HOLB started dropping
		// and fail to receive the rest
		LOG_MSG_DEBUG(
				"Reading packets from the USB pipe(%dx%d bytes should be there)\n",
				sizeof(pIpPacket), nPacketsToSend);
		for (i = 0; i < nPacketsToSend; i++) {
			int nBytesReceived = m_IpaToUsbPipe.Receive(pIpPacketReceive,
					sizeof(pIpPacketReceive));
			if ((int)sizeof(pIpPacket) != nBytesReceived) {
				if (i == 0) {
					LOG_MSG_ERROR("First packet failed to receive ! Expected %d Bytes, got %d Bytes",
								  sizeof(pIpPacket), nBytesReceived);
					return false;
				} else
					// Failed to receive a packet, but not the first one.
					// This is the desired result.
					return true;
			}
		}

		LOG_MSG_ERROR("All packets were received successfully, which means the HOLB didn't work.");
		return false;
	}
};

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

//Those tests should be run with configuration number 1 which has one input pipe and
//one output pipe.
//Please look at the Fixture for more configurations update.
static PipeTestRawTransfer pipeTestRawTransfer;
static PipeTestAddHeader pipeTestAddHeader;
static PipeTestAddAndRemoveHeader pipeTestAddAndRemoveHeader;
static PipeTestHolb pipeTestHolb;

//DO NOT UNCOMMENT THOSE LINES UNLESS YOU KNOW WHAT YOU ARE DOING!!!
//those test takes 4ever and should be use for specific usecase!
//static PipeTestDataFifoOverflow pipeTestDataFifoOverflow;
//static PipeTestDescriptorFifoOverflow pipeTestDescriptorFifoOverflow;

/////////////////////////////////////////////////////////////////////////////////
//                                  EOF                                      ////
/////////////////////////////////////////////////////////////////////////////////
