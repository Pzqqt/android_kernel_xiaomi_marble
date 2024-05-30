/*
 * Copyright (c) 2017-2018,2020 The Linux Foundation. All rights reserved.
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

#include "Pipe.h"
#include "TestsUtils.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//Do not change those default values due to the fact that some test may relay on those default values.
//In case you need a change of the field do this in a derived class.

//Dest MAC(6 bytes) Src MAC(6 Bytes) EtherType(2 Bytes)
unsigned char Pipe::m_pUsbHeader[] = { 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xB1, 0xB2,
		0xB3, 0xB4, 0xB5, 0xB6, 0xC1, 0xC2 };
unsigned char Pipe::m_pA2NDUNHeader[] =
		{ 0xA1, 0xA2, 0xA3, 0xA4, 0xB1, 0xB2, 0xC1, 0xC2 };
//unsigned char Pipe::m_pA2DUNHeader[]  = {};
//unsigned char Pipe::m_pQ6LANHeader[]  = {};

//////////////////////////////////////////////////////////////////////////////////////////////////////////

Pipe::Pipe(enum ipa_client_type nClientType,
		IPATestConfiguration eConfiguration) :
		m_Fd(-1), m_nHeaderLengthRemove(0),
		m_nHeaderLengthAdd(0), m_pHeader(NULL), m_pInodePath(NULL),
		m_bInitialized(false), m_ExceptionPipe(false) {
	m_nClientType = nClientType;
	m_eConfiguration = eConfiguration;
}

Pipe::Pipe(IPATestConfiguration eConfiguration) :
	m_Fd(-1), m_nHeaderLengthRemove(0),
	m_nHeaderLengthAdd(0), m_pHeader(NULL), m_pInodePath(NULL),
	m_bInitialized(false), m_ExceptionPipe(true) {
	m_eConfiguration = eConfiguration;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

Pipe::~Pipe() {
	//Nothing to be done at this point...
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Pipe::Init() {
	int tries_cnt = 1;
	SetSpecificClientParameters(m_nClientType, m_eConfiguration);
	//By examining the Client type we will map the inode device name
	while (tries_cnt <= 10000) {
		m_Fd = open(m_pInodePath, O_RDWR);
		if (-1 != m_Fd)
			break;

		// Sleep for 5 msec
		usleep(5000);
		++tries_cnt;
	}
	LOG_MSG_DEBUG("open retries_cnt=%d\n", tries_cnt);
	if (-1 == m_Fd) {
		LOG_MSG_ERROR("Failed to open %s", m_pInodePath);
		return false;
	}
	m_bInitialized = true;
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void Pipe::Destroy() {
	if (false == m_bInitialized) {
		LOG_MSG_ERROR("Pipe is being used without being initialized!");
		return;
	}
	close(m_Fd);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

int Pipe::Send(unsigned char * pBuffer, size_t nBytesToSend) {
	if (false == m_bInitialized) {
		LOG_MSG_ERROR("Pipe is being used without being initialized!");
		return 0;
	}
	size_t nBytesWritten = 0;
	nBytesWritten = write(m_Fd, pBuffer, nBytesToSend);
	return nBytesWritten;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

int Pipe::AddHeaderAndSend(unsigned char * pIpPacket, size_t nIpPacketSize) {
	int retval;

	if (false == m_bInitialized) {
		LOG_MSG_ERROR("Pipe is being used without being initialized!");
		return 0;
	}
	size_t nBytesToWrite = nIpPacketSize + m_nHeaderLengthAdd;
	//Allocate new buffer for the Header and IP packet:
	unsigned char *pLinkLayerAndIpPacket = new unsigned char[nBytesToWrite];
	if (!pLinkLayerAndIpPacket) {
		LOG_MSG_ERROR("Memory allocation failure.");
		return 0;
	}

	//put the header first:
	memcpy(pLinkLayerAndIpPacket, m_pHeader, m_nHeaderLengthAdd);
	//Then add the IP packet:
	memcpy(pLinkLayerAndIpPacket + m_nHeaderLengthAdd, pIpPacket,
			nIpPacketSize);
	//Call the Send method which will send the new created buffer(which contains the IP packet with the Header):
	retval = Send(pLinkLayerAndIpPacket, nBytesToWrite);
	delete[] pLinkLayerAndIpPacket;

	return retval;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

int Pipe::Receive(unsigned char *pBuffer, size_t nBytesToReceive) {
	if (false == m_bInitialized) {
		LOG_MSG_ERROR("Pipe is being used without being initialized!");
		return 0;
	}
	size_t nBytesRead = 0;
	nBytesRead = read(m_Fd, (void*) pBuffer, nBytesToReceive);
	return nBytesRead;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

int Pipe::ReceiveAndRemoveHeader(unsigned char *pIpPacket, size_t nIpPacketSize) {
	if (false == m_bInitialized) {
		LOG_MSG_ERROR("Pipe is being used without being initialized!");
		return 0;
	}
	size_t nBytesToRead = nIpPacketSize + m_nHeaderLengthRemove;
	unsigned char *pPacket = new unsigned char[nBytesToRead];
	if (!pPacket) {
		LOG_MSG_ERROR("Memory allocation failure.");
		return 0;
	}
	size_t nReceivedBytes = Receive(pPacket, nBytesToRead);
	if (nReceivedBytes != nBytesToRead) {
		LOG_MSG_ERROR("Pipe was asked to receive an IP packet "
			      "of size %d, however only %d bytes were read "
			      "while the header size is %d",
			      nIpPacketSize,
			      nReceivedBytes,
			      m_nHeaderLengthRemove);
		delete[] pPacket;
		return nReceivedBytes - m_nHeaderLengthRemove;
	}

	memcpy(pIpPacket, pPacket + m_nHeaderLengthRemove, nIpPacketSize);
	delete[] pPacket;

	return (nReceivedBytes - m_nHeaderLengthRemove);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

enum ipa_client_type Pipe::GetClientType() {
	if (false == m_bInitialized) {
		LOG_MSG_ERROR("Pipe is being used without being initialized!");
		return IPA_CLIENT_HSIC1_PROD;
	}
	return m_nClientType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

void Pipe::SetSpecificClientParameters(
		enum ipa_client_type nClientType,
		IPATestConfiguration eConfiguration) {
	switch (eConfiguration) {
	case IPA_TEST_CONFIFURATION_0:
		break;
	case IPA_TEST_CONFIFURATION_1:
		switch (nClientType) {
		case (IPA_CLIENT_TEST_PROD):
			m_pInodePath = CONFIG_1_FROM_USB1_TO_IPA_DMA;
			m_nHeaderLengthAdd = sizeof(m_pUsbHeader);
			m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
			m_pHeader = m_pUsbHeader;
			LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_PROD ");
			break;
		case (IPA_CLIENT_TEST_CONS):
			m_pInodePath = CONFIG_1_FROM_IPA_TO_USB1_DMA;
			m_nHeaderLengthAdd = sizeof(m_pUsbHeader);
			m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
			m_pHeader = m_pUsbHeader;
			LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_CONS");
			break;
		default:
			LOG_MSG_ERROR("IPA_TEST_CONFIFURATION_1 switch in default "
			"nClientType = %d is not supported ", nClientType);
			break;
		}
		break;
	case IPA_TEST_CONFIFURATION_2:
		switch (nClientType) {
		case (IPA_CLIENT_TEST_PROD):
			m_pInodePath = CONFIG_2_FROM_USB_TO_IPA;
			m_nHeaderLengthAdd = sizeof(m_pUsbHeader);
			m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
			m_pHeader = m_pUsbHeader;
			LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_PROD");
			break;
		case (IPA_CLIENT_TEST2_CONS):
			m_pInodePath = CONFIG_2_FROM_IPA_TO_A2_NDUN;
			m_nHeaderLengthAdd = sizeof(m_pA2NDUNHeader);
			m_nHeaderLengthRemove = sizeof(m_pA2NDUNHeader);
			m_pHeader = m_pA2NDUNHeader;
			LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_CONS");
			break;
		default:
			LOG_MSG_ERROR("IPA_TEST_CONFIFURATION_2 switch in default "
			"nClientType = %d is not supported ", nClientType);
			break;
		}
		break;
	case IPA_TEST_CONFIFURATION_3:
		switch (nClientType) {
		case IPA_CLIENT_TEST2_PROD:
			m_pInodePath = CONFIG_3_FROM_A2_NDUN_TO_IPA;
			m_nHeaderLengthAdd = sizeof(m_pA2NDUNHeader);
			m_nHeaderLengthRemove = sizeof(m_pA2NDUNHeader);
			m_pHeader = m_pA2NDUNHeader;
			LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_PROD");
			break;
		case IPA_CLIENT_TEST_CONS:
			m_pInodePath = CONFIG_3_FROM_IPA_TO_USB1;
			m_nHeaderLengthAdd = sizeof(m_pUsbHeader);
			m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
			m_pHeader = m_pUsbHeader;
			LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_CONS");
			break;
		case IPA_CLIENT_TEST2_CONS:
			m_pInodePath = CONFIG_3_FROM_IPA_TO_A2_NDUN;
			m_nHeaderLengthAdd = sizeof(m_pA2NDUNHeader);
			m_nHeaderLengthRemove = sizeof(m_pA2NDUNHeader);
			m_pHeader = m_pA2NDUNHeader;
			LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_CONS");
			break;
		case IPA_CLIENT_TEST4_CONS:
			//TODO add when applicable
			m_pInodePath = CONFIG_3_FROM_IPA_TO_Q6_LAN;
			m_nHeaderLengthAdd = 0;
			m_nHeaderLengthRemove = 0;
			m_pHeader = 0;
			LOG_MSG_INFO("IPA_CLIENT_TEST4_CONS is not supported yet");
			break;
		default:
			LOG_MSG_INFO("IPA_TEST_CONFIFURATION_3 switch in default "
			"nClientType = %d is not supported ", nClientType);
			break;
		}
		break;
	case IPA_TEST_CONFIFURATION_7:
		if (m_ExceptionPipe) {
			m_pInodePath = CONFIG_7_FROM_IPA_TO_A5_EXCEPTION;
			m_nHeaderLengthAdd = 0; //No send
			m_nHeaderLengthRemove = 8; //A5Mux header size without retained source header
			m_pHeader = NULL; //No header to send
			LOG_MSG_INFO("Setting parameters for A5_Exception ");
			break;
		}

		if (nClientType == IPA_CLIENT_TEST_PROD) {
			m_pInodePath = CONFIG_7_FROM_USB1_TO_IPA;
			m_nHeaderLengthAdd = 0;
			m_nHeaderLengthRemove = 0;
			m_pHeader = NULL;
			LOG_MSG_INFO(
					"Setting parameters for FROM_USB1_TO_IPA - no header addition/removal");
		} else {
			LOG_MSG_INFO("IPA_TEST_CONFIFURATION_7 switch in default "
			"nClientType = %d is not supported ", nClientType);
		}
		break;
	case IPA_TEST_CONFIGURATION_8:
	    switch(nClientType)
	    {
	    case (IPA_CLIENT_TEST_PROD):
	      m_pInodePath          = CONFIG_8_DEAGG_TO_IPA_NO_AGG;
	      m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
	      m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
	      m_pHeader             = m_pUsbHeader;
	      LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_PROD ");
	      break;
	    case (IPA_CLIENT_TEST_CONS):
	      m_pInodePath          = CONFIG_8_FROM_IPA_AGG;
	      m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
	      m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
	      m_pHeader             = m_pUsbHeader;
	      LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_CONS");
	      break;
	    case (IPA_CLIENT_TEST3_PROD):
	      m_pInodePath          = CONFIG_8_NO_AGG_TO_IPA_AGG;
	      m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
	      m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
	      m_pHeader             = m_pUsbHeader;
	      LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST3_PROD ");
	      break;
	    case (IPA_CLIENT_TEST3_CONS):
	      m_pInodePath          = CONFIG_8_FROM_IPA_NO_AGG;
	      m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
	      m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
	      m_pHeader             = m_pUsbHeader;
	      LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST3_CONS");
	      break;
	    case (IPA_CLIENT_TEST2_PROD):
	      m_pInodePath          = CONFIG_8_DEAGG_TO_IPA_AGG;
	      m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
	      m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
	      m_pHeader             = m_pUsbHeader;
	      LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_PROD");
	      break;
	    case (IPA_CLIENT_TEST2_CONS):
	      m_pInodePath          = CONFIG_8_DEAGG_FROM_IPA_AGG;
	      m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
	      m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
	      m_pHeader             = m_pUsbHeader;
	      LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_CONS");
	      break;
	    case (IPA_CLIENT_TEST4_PROD):
	      m_pInodePath          = CONFIG_8_NO_AGG_TO_IPA_AGG_TIME;
	      m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
	      m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
	      m_pHeader             = m_pUsbHeader;
	      LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST4_PROD");
	      break;
	    default:
	      LOG_MSG_ERROR("IPA_TEST_CONFIFURATION_8 switch in default "
	                    "nClientType = %d is not supported ",
	                    nClientType);
	      break;
	    }
	    break;
	case IPA_TEST_CONFIGURATION_9:
		switch(nClientType)
		{
		case (IPA_CLIENT_TEST_PROD):
		  m_pInodePath          = CONFIG_9_DEAGG_TO_IPA_NO_AGG;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_PROD ");
		  break;
		case (IPA_CLIENT_TEST_CONS):
		  m_pInodePath          = CONFIG_9_FROM_IPA_AGG;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_CONS");
		  break;
		case (IPA_CLIENT_TEST3_PROD):
		  m_pInodePath          = CONFIG_9_NO_AGG_TO_IPA_AGG;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST3_PROD ");
		  break;
		case (IPA_CLIENT_TEST3_CONS):
		  m_pInodePath          = CONFIG_9_FROM_IPA_NO_AGG;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST3_CONS");
		  break;
		case (IPA_CLIENT_TEST2_PROD):
		  m_pInodePath          = CONFIG_9_DEAGG_TO_IPA_AGG;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_PROD");
		  break;
		case (IPA_CLIENT_TEST2_CONS):
		  m_pInodePath          = CONFIG_9_DEAGG_FROM_IPA_AGG;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_CONS");
		  break;
		case (IPA_CLIENT_TEST4_PROD):
		  m_pInodePath          = CONFIG_9_NO_AGG_TO_IPA_AGG_TIME;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST4_PROD");
		  break;
		default:
		  LOG_MSG_ERROR("IPA_TEST_CONFIFURATION_9 switch in default "
				  	  	  "nClientType = %d is not supported ",
				  	  	  nClientType);
		  break;
		}
		break;
	case IPA_TEST_CONFIGURATION_10:
	  	switch(nClientType)
	  	{
	  	case (IPA_CLIENT_TEST_PROD):
	  	  m_pInodePath          = CONFIG_10_TO_IPA_AGG_ZERO_LIMITS;
	  	  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
	  	  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
	  	  m_pHeader             = m_pUsbHeader;
	  	  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_PROD");
	  	  break;
	  	case (IPA_CLIENT_TEST_CONS):
		  m_pInodePath          = CONFIG_10_FROM_IPA_AGG_ZERO_LIMITS;
	  	  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
	  	  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
	  	  m_pHeader             = m_pUsbHeader;
	  	  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_CONS");
	  	  break;
	  	default:
	  	  LOG_MSG_ERROR("IPA_TEST_CONFIFURATION_10 switch in default "
	  			  	  	  "nClientType = %d is not supported ",
	  			  	  	  	  nClientType);
	  	  break;
	  	}
	  	break;
	case IPA_TEST_CONFIGURATION_11:
		switch(nClientType)
		{
		case (IPA_CLIENT_TEST_PROD):
		  m_pInodePath          = CONFIG_11_TO_IPA;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_PROD");
		  break;
		case (IPA_CLIENT_TEST2_CONS):
		  m_pInodePath          = CONFIG_11_FROM_IPA_AGG;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_CONS");
		  break;
		case (IPA_CLIENT_TEST2_PROD):
		  m_pInodePath          = CONFIG_11_TO_IPA_DEAGG;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_PROD");
		  break;
		case (IPA_CLIENT_TEST3_CONS):
		  m_pInodePath          = CONFIG_11_FROM_IPA;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST3_CONS");
		  break;
		case (IPA_CLIENT_TEST_CONS):
		  m_pInodePath          = CONFIG_11_FROM_IPA_AGG_TIME;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_CONS");
		  break;
		case (IPA_CLIENT_TEST4_CONS):
		  m_pInodePath          = CONFIG_11_FROM_IPA_ZERO_LIMITS;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST4_CONS");
		  break;
		default:
		  LOG_MSG_ERROR("IPA_TEST_CONFIFURATION_11 switch in default "
				  	  	  "nClientType = %d is not supported ",
				  	  	  	  nClientType);
		  break;
		}
		break;
	case IPA_TEST_CONFIGURATION_12:
		switch(nClientType)
		{
		case (IPA_CLIENT_TEST_PROD):
		  m_pInodePath          = CONFIG_12_TO_IPA;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_PROD");
		  break;
		case (IPA_CLIENT_TEST2_CONS):
		  m_pInodePath          = CONFIG_12_FROM_IPA_AGG;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_CONS");
		  break;
		case (IPA_CLIENT_TEST2_PROD):
		  m_pInodePath          = CONFIG_12_TO_IPA_DEAGG;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_PROD");
		  break;
		case (IPA_CLIENT_TEST3_CONS):
		  m_pInodePath          = CONFIG_12_FROM_IPA;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST3_CONS");
		  break;
		case (IPA_CLIENT_TEST_CONS):
		  m_pInodePath          = CONFIG_12_FROM_IPA_AGG_TIME;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_CONS");
		  break;
		case (IPA_CLIENT_TEST4_CONS):
		  m_pInodePath          = CONFIG_12_FROM_IPA_ZERO_LIMITS;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST4_CONS");
		  break;
		default:
		  LOG_MSG_ERROR("IPA_TEST_CONFIFURATION_12 switch in default "
				  	  	  "nClientType = %d is not supported ",
				  	  	  	  nClientType);
		  break;
		}
		break;
	case IPA_TEST_CONFIGURATION_17:
		switch(nClientType)
		{
		case (IPA_CLIENT_TEST_PROD):
		  m_pInodePath          = CONFIG_17_TO_IPA;
		  m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		  m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		  m_pHeader             = m_pUsbHeader;
		  LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_PROD ");
		  break;
	   case (IPA_CLIENT_TEST3_PROD):
		   m_pInodePath          = CONFIG_17_TO_IPA_NO_HDR;
		   m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		   m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		   m_pHeader             = m_pUsbHeader;
		   LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST3_PROD ");
		   break;
	   case (IPA_CLIENT_TEST2_CONS):
		   m_pInodePath          = CONFIG_17_FROM_IPA_AGG;
		   m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		   m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		   m_pHeader             = m_pUsbHeader;
		   LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_CONS");
		   break;
	   case (IPA_CLIENT_TEST2_PROD):
		   m_pInodePath          = CONFIG_17_TO_IPA_DEAGG;
		   m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		   m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		   m_pHeader             = m_pUsbHeader;
		   LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_PROD ");
		   break;
	   case (IPA_CLIENT_TEST3_CONS):
		   m_pInodePath          = CONFIG_17_FROM_IPA;
		   m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		   m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		   m_pHeader             = m_pUsbHeader;
		   LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST3_CONS");
		   break;
	   case (IPA_CLIENT_TEST_CONS):
		   m_pInodePath          = CONFIG_17_FROM_IPA_AGG_TIME;
		   m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		   m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		   m_pHeader             = m_pUsbHeader;
		   LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_CONS");
		   break;
	   case (IPA_CLIENT_TEST4_CONS):
		   m_pInodePath          = CONFIG_17_FROM_IPA_ZERO_LIMITS;
		   m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
		   m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
		   m_pHeader             = m_pUsbHeader;
		   LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST4_CONS");
		   break;
	   default:
		   LOG_MSG_ERROR("IPA_TEST_CONFIFURATION_17 switch in default "
			   "nClientType = %d is not supported ",
			   nClientType);
		   break;
	   }
	   break;
	case IPA_TEST_CONFIGURATION_18:
		switch (nClientType)
		{
		case (IPA_CLIENT_TEST_PROD):
			m_pInodePath          = CONFIG_18_TO_IPA;
			m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
			m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
			m_pHeader             = m_pUsbHeader;
			LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_PROD ");
			break;
		case (IPA_CLIENT_TEST2_PROD):
			m_pInodePath          = CONFIG_18_DUMMY_ENDPOINT;
			m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
			m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
			m_pHeader             = m_pUsbHeader;
			LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST2_PROD ");
			break;
		case (IPA_CLIENT_TEST_CONS):
			m_pInodePath          = CONFIG_18_FROM_IPA;
			m_nHeaderLengthAdd    = sizeof(m_pUsbHeader);
			m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
			m_pHeader             = m_pUsbHeader;
			LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_CONS");
			break;
		default:
			LOG_MSG_ERROR(
				"IPA_TEST_CONFIFURATION_18 switch in default "
				"nClientType = %d is not supported ",
				nClientType);
			break;
		}
		break;
	case IPA_TEST_CONFIGURATION_19:
		switch (nClientType)
		{
		case (IPA_CLIENT_TEST_PROD):
			m_pInodePath = CONFIG_19_FROM_USB_TO_IPA_DMA;
			m_nHeaderLengthAdd = sizeof(m_pUsbHeader);
			m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
			m_pHeader = m_pUsbHeader;
			LOG_MSG_INFO("Setting parameters for IPA_CLIENT_TEST_PROD ");
			break;
		case (IPA_CLIENT_TEST_CONS):
			m_pInodePath = CONFIG_19_FROM_IPA_TO_USB_DMA;
			m_nHeaderLengthAdd = sizeof(m_pUsbHeader);
			m_nHeaderLengthRemove = sizeof(m_pUsbHeader);
			m_pHeader = m_pUsbHeader;
			LOG_MSG_INFO("Setting parameters for TEST_CONS");
			break;
		default:
			LOG_MSG_ERROR("IPA_TEST_CONFIFURATION_19 switch in default "
			"nClientType = %d is not supported ", nClientType);
			break;
		}
		break;
	default:
	LOG_MSG_ERROR("Pipe::SetSpecificClientParameters "
	"switch in default eConfiguration = %d ", eConfiguration);
	break;
	}
}/* Pipe::SetSpecificClientParameters() */

//////////////////////////////////////////////////////////////////////////////////////////////////////////

int Pipe::GetHeaderLengthAdd() {
	if (false == m_bInitialized) {
		LOG_MSG_ERROR("Pipe is being used without being initialized!");
		return 0;
	}
	return m_nHeaderLengthAdd;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

int Pipe::GetHeaderLengthRemove() {
	if (false == m_bInitialized) {
		LOG_MSG_ERROR("Pipe is being used without being initialized!");
		return 0;
	}
	return m_nHeaderLengthRemove;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Pipe::ConfigureHolb(unsigned short enable, unsigned timerValue) {
	if (false == m_bInitialized) {
		LOG_MSG_ERROR("Pipe is being used without being initialized!");
		return false;
	}

	if (IPA_CLIENT_IS_PROD(m_nClientType)) {
		LOG_MSG_ERROR("Can't configure HOLB on a producer pipe!");
		return false;
	}

	struct ipa_test_holb_config test_holb_config;

	test_holb_config.client = m_nClientType;
	test_holb_config.tmr_val = timerValue;
	test_holb_config.en = enable;

	LOG_MSG_DEBUG("Sending: client=%d tmr_val=%d en=%d",
				  test_holb_config.client,
				  test_holb_config.tmr_val,
				  test_holb_config.en);

	return configure_holb(&test_holb_config);
}

bool Pipe::EnableHolb(unsigned timerValue) {
	return ConfigureHolb(1, timerValue);
}

bool Pipe::DisableHolb() {
	return ConfigureHolb(0, 0);
}

