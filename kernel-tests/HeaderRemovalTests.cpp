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

#include "HeaderRemovalTests.h"
#include "TestsUtils.h"
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////////

static const unsigned int HEADER_REMOVAL_TEST_MAX_PACKET_BYTE_SIZE = 1024;

/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


HeaderRemovalTOSCheck::HeaderRemovalTOSCheck()
{
	m_name = "HeaderRemovalTOSCheck";
	m_description = "HeaderRemovalTOSCheck: Remove the header from the A2NDUN pipe and check the TOS field of the IP packet";
}

/////////////////////////////////////////////////////////////////////////////////

bool HeaderRemovalTOSCheck::Run()
{
	bool bTestResult = true;
	Byte pPacketReceiveBuffer[HEADER_REMOVAL_TEST_MAX_PACKET_BYTE_SIZE] = {0};//This buffer will be used in order to store the received packet.

	unsigned int nMagicNumber = 0x12345678; //arbitrary number
	unsigned int nChannelID = 0xABCD;//arbitrary number
	unsigned int nA2NDUNPacketByteSize = 0;
	unsigned int nBytesSent = 0;
	int          nBytesReceived = 0;
	Byte *pA2NDUNPacket = CreateA2NDUNPacket(nMagicNumber, nChannelID, IPV4_FILE_PATH, &nA2NDUNPacketByteSize);
	if(0 == pA2NDUNPacket) {
	  LOG_MSG_ERROR("Cannot load file to memory, exiting");
	  return false;
	}
	LOG_MSG_INFO("A2 Packet was  successfully created (%d bytes)", nA2NDUNPacketByteSize);

	if ( false == SetIPATablesToPassAllToSpecificClient(IPA_CLIENT_TEST2_PROD, IPA_CLIENT_TEST_CONS)) {
		LOG_MSG_ERROR("SetIPATablesToPassAllToSpecificClient failed, exiting test case");
		bTestResult = false;
		goto bail;
	}

	LOG_MSG_INFO("All tables were configured in order to output the packet to the correct pipe");

	LOG_MSG_INFO("Sending packet into the A2NDUN pipe(%d bytes) and the Pipe will add an header",
	             nA2NDUNPacketByteSize);

	nBytesSent = m_A2NDUNToIpaPipe.Send(pA2NDUNPacket, nA2NDUNPacketByteSize);
	if (nA2NDUNPacketByteSize != nBytesSent)
	{
		bTestResult = false;
		goto bail;
	}
	//Receive the raw IP packet(which is a 4 arbitrary bytes) without header removal by the Pipe
	LOG_MSG_INFO("Reading packet from the USB pipe");

	nBytesReceived = m_IpaToUsbPipe.Receive(pPacketReceiveBuffer, HEADER_REMOVAL_TEST_MAX_PACKET_BYTE_SIZE);
	//TODO Header Removal: at this point the success scenario is that data came to the correct pipe - change this to
	//packet memory compare after header insertion is enabled.
	if (0 == nBytesReceived)
	{
		bTestResult = false;
		goto bail;
	}

	LOG_MSG_INFO("Read buffer : ");
	//Print the output
	for (int i = 0 ; i < nBytesReceived ; i++)
	{
	  printf("0x%02x", pPacketReceiveBuffer[i]);
	}
	LOG_MSG_INFO("End of Read buffer.");

	if(0 != memcmp((const void *)pPacketReceiveBuffer,
	               (const void *)(pA2NDUNPacket + (nBytesSent - nBytesReceived)),
	               nBytesReceived)) {
	  LOG_MSG_ERROR("Memory contains don't match");
	  bTestResult = false;
	  goto bail;
	}
/* fall through */

bail:

	delete pA2NDUNPacket;
	return bTestResult;
}

HeaderRemovalMetaDataFiltering::HeaderRemovalMetaDataFiltering()
{
	m_name = "HeaderRemovalMetaDataFiltering";
	m_description =
			"HeaderRemovalMetaDataFiltering: check meta data based filtering";
}

/////////////////////////////////////////////////////////////////////////////////

bool HeaderRemovalMetaDataFiltering::Run()
{
	bool bTestResult = true;
	Byte pPacketReceiveBuffer[HEADER_REMOVAL_TEST_MAX_PACKET_BYTE_SIZE] = {0};//This buffer will be used in order to store the received packet.

	unsigned int nMagicNumber          = 0x12345678; //arbitrary number
	unsigned int nChannelID            = 0xABCD;//arbitrary number
	unsigned int nA2NDUNPacketByteSize = 0;
	unsigned int nMetaData             = 0;
	unsigned int nMetaDataMask         = 0xFFFF;
	unsigned int nBytesSent            = 0;
	int          nBytesReceived        = 0;


	Byte *pA2NDUNPacket = CreateA2NDUNPacket(nMagicNumber, nChannelID, IPV4_FILE_PATH, &nA2NDUNPacketByteSize);
	if(0 == pA2NDUNPacket) {
		LOG_MSG_ERROR("Cannot load file to memory, exiting");
		return false;
	}

	nMetaData             = (nChannelID << 16) | (0xFFFF & nA2NDUNPacketByteSize);
	LOG_MSG_INFO("*************nMetaData ==  (0x%x)", nMetaData);

	LOG_MSG_INFO("A2 Packet was  successfully created (%d bytes)", nA2NDUNPacketByteSize);

	SetRoutingTableToPassAllToSpecificClient(IPA_CLIENT_TEST_CONS);
	SetHeaderInsertionTableAddEmptyHeaderForTheClient(IPA_CLIENT_TEST_CONS);

	LOG_MSG_INFO("Configuring Filtering module...");

	if (false ==
			ConfigureFilteringBlockWithMetaDataEq(
					IPA_CLIENT_TEST_CONS,
					nMetaData,
					nMetaDataMask)) {
		bTestResult = false;
		goto bail;
	}

	LOG_MSG_INFO("Sending packet into the A2NDUN pipe(%d bytes) and the Pipe will add an header",
	             nA2NDUNPacketByteSize);
	nBytesSent = m_A2NDUNToIpaPipe.Send(pA2NDUNPacket, nA2NDUNPacketByteSize);
	if (nA2NDUNPacketByteSize != nBytesSent) {
		bTestResult = false;
		goto bail;
	}

	//Receive the raw IP packet(which is a 4 arbitrary bytes) without header removal by the Pipe
	LOG_MSG_INFO("Reading packet from the USB pipe");
	nBytesReceived = m_IpaToUsbPipe.Receive(pPacketReceiveBuffer, HEADER_REMOVAL_TEST_MAX_PACKET_BYTE_SIZE);
	//TODO Header Removal: at this point the success scenario is that data came to the correct pipe - change this to
	//packet memory compare after header insertion is enabled.
	if (0 == nBytesReceived) {
		bTestResult = false;
		goto bail;
	}

	LOG_MSG_INFO("Read buffer : ");
	//Print the output
	for (int i = 0 ; i < nBytesReceived ; i++) {
	  printf("0x%02x", pPacketReceiveBuffer[i]);
	}
	LOG_MSG_INFO("End of Read buffer.");

	if(0 != memcmp((const void *)pPacketReceiveBuffer,
	               (const void *)(pA2NDUNPacket + (nBytesSent - nBytesReceived)),
	               nBytesReceived)) {
	  LOG_MSG_ERROR("Memory contains don't match");
	  bTestResult = false;
	  goto bail;
	}

/* fall through */

bail:

	delete pA2NDUNPacket;
	return bTestResult;
}

static HeaderRemovalTOSCheck          headerRemovalTOSCheck;
static HeaderRemovalMetaDataFiltering headerRemovalMetaDataFiltering;

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

