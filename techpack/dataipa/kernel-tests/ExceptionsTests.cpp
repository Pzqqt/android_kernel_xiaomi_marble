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

#include "RoutingDriverWrapper.h"
#include "HeaderInsertion.h"
#include "Filtering.h"
#include "IPAFilteringTable.h"
#include "TestsUtils.h"
#include "ExceptionsTestFixture.h"
#include "IPv4Packet.h"
#include <string.h>

#define MAX_SENT_BUFFER_SIZE 1500
#define MAX_RECEIVE_BUFFER_SIZE 1500
#define VALIDATE_WITH_MSG_AND_RETVAL(bRetVal,msg) \
	if (false == bRetVal){  \
		LOG_MSG_ERROR(msg); \
		return false;	    \
	}

using namespace IPA;

///////////////////////////////////////////////////////////////////////////////

class ExceptionsTestNonIpPacket: public ExceptionsTestFixture {
public:
	//The packet size to be sent
	size_t m_nPacketSize;
	//A buffer to hold the non-IP(V4/V6) packet
	Byte *m_pSendBuffer;

	///////////////////////////////////////////////////////////////////////////

	//Set the tests name and description
	ExceptionsTestNonIpPacket() :
			m_nPacketSize(0), m_pSendBuffer(NULL) {
		m_name = "ExceptionsTestNonIpPacket";
		m_description =
				"Create a non-IP packet(version!=4 && version !=6) and \
				expect exception from Filter block";
	}

	///////////////////////////////////////////////////////////////////////////

	virtual bool Run() {
		bool bRetVal = true;
		Byte *pReceiveBuffer = new Byte[MAX_RECEIVE_BUFFER_SIZE];
		//Send the non-IPV4/IPV6 packet to the IPA
		LOG_MSG_DEBUG("Send the non-IPV4/IPV6 packet to the IPA");
		size_t nBytesSent = m_USB1ToIpaPipe.Send(m_pSendBuffer, m_nPacketSize);
		if (nBytesSent != m_nPacketSize) {
			LOG_MSG_ERROR("Not all data was sent into the IPA");
			return false;
		}

		//Read from the exception pipe(from IPA to A5) - try to read as much as we can
		size_t nBytesRead = m_IpaToA5ExceptionPipe.Receive(pReceiveBuffer,
				MAX_RECEIVE_BUFFER_SIZE);
		if (nBytesRead != nBytesSent) {
			LOG_MSG_ERROR("Not all data was read:");
			print_buff(pReceiveBuffer, nBytesRead);
			return false;
		}

		//check the exception packet against the one that we sent
		bRetVal = !memcmp(m_pSendBuffer, pReceiveBuffer, nBytesSent);
		if (false == bRetVal) {
			LOG_MSG_ERROR("Received packet is not equal, Received:");
			print_buff(pReceiveBuffer, nBytesRead);
			LOG_MSG_ERROR("Received packet is not equal, Sent:");
			print_buff(m_pSendBuffer, m_nPacketSize);
			return false;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////

	//build the non-IP packet
	virtual bool Setup() {
		bool bRetVal = true;
		m_pSendBuffer = new Byte[MAX_SENT_BUFFER_SIZE];
		//Load some default IPV4 packet and save its size
		m_nPacketSize = MAX_SENT_BUFFER_SIZE; //This parameter is In/Out
		bRetVal = LoadDefaultPacket(IPA_IP_v4, m_pSendBuffer, m_nPacketSize);
		VALIDATE_WITH_MSG_AND_RETVAL(bRetVal, "Load failed");
		//Set the version field to non-IPV4/IPV6(version = 5)
		m_pSendBuffer[0] &= 0x0F;
		m_pSendBuffer[0] |= 0x50;

		//initialize Pipes
		bRetVal = m_USB1ToIpaPipe.Init();
		VALIDATE_WITH_MSG_AND_RETVAL(bRetVal, "Pipe Initialization failed");
		bRetVal = m_IpaToA5ExceptionPipe.Init();
		VALIDATE_WITH_MSG_AND_RETVAL(bRetVal, "Pipe Initialization failed");
		return true;
	}

	///////////////////////////////////////////////////////////////////////////

	virtual bool Teardown() {
		bool bRetVal = true;
		delete[] m_pSendBuffer;
		m_USB1ToIpaPipe.Destroy();
		m_IpaToA5ExceptionPipe.Destroy();
		return bRetVal;
	}

	///////////////////////////////////////////////////////////////////////////

};
//ExceptionTestNoneIpPacket

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

class ExceptionsTestFragmentedException: public ExceptionsTestFixture {
public:
	//The packet size to be sent
	size_t m_nPacketSize;
	//A buffer to hold the non-IP(V4/V6) packet
	Byte *m_pSendBuffer;
	Byte *m_pReceiveBuffer;

	///////////////////////////////////////////////////////////////////////////

	//Set the tests name and description
	ExceptionsTestFragmentedException():m_nPacketSize(0), m_pSendBuffer(NULL),
			m_pReceiveBuffer(NULL){
		m_name = "ExceptionsTestFragmentedException";
		m_description =
				"Send IP packet with MF set, create global Filter rule \
				that will hit it as Exception";
	}

	///////////////////////////////////////////////////////////////////////////

	virtual bool Run() {
		bool bRetVal = true;
		//configuring the Filter block to catch the fragmented packet:
		ConfigureFilterGlobalRuleForMF();
		//Send the non-IPV4/IPV6 packet to the IPA
		LOG_MSG_DEBUG("Send the IP packet with the MF bit set(size = %d)", m_nPacketSize);
		size_t nBytesSent = m_USB1ToIpaPipe.Send(m_pSendBuffer, m_nPacketSize);
		if (nBytesSent != m_nPacketSize) {
			LOG_MSG_ERROR("Not all data was sent into the IPA(only %d)", nBytesSent);
			return false;
		}
		//Read from the exception pipe(from IPA to A5) - try to read as much as we can
		size_t nBytesRead = m_IpaToA5ExceptionPipe.Receive(m_pReceiveBuffer,
				MAX_RECEIVE_BUFFER_SIZE);
		if (nBytesRead != nBytesSent) {
			LOG_MSG_ERROR("Not all data was read:");
			print_buff(m_pReceiveBuffer, nBytesRead);
			return false;
		}
		//check the exception packet against the one that we sent
		bRetVal = !memcmp(m_pSendBuffer, m_pReceiveBuffer, nBytesSent);
		if (false == bRetVal) {
			LOG_MSG_ERROR("Received packet is not equal, Received:");
			print_buff(m_pReceiveBuffer, nBytesRead);
			LOG_MSG_ERROR("Received packet is not equal, Sent:");
			print_buff(m_pSendBuffer, m_nPacketSize);
			return false;
		}
		return true;
	}

	///////////////////////////////////////////////////////////////////////////

	//build the non-IP packet
	virtual bool Setup() {
		bool bRetVal = true;
		m_pReceiveBuffer = new Byte[MAX_RECEIVE_BUFFER_SIZE];
		m_pSendBuffer = new Byte[MAX_RECEIVE_BUFFER_SIZE];
		//Load some default TCP packet
		TCPPacket tcpPacket;
		//Set the MF bit
		tcpPacket.SetMF(true);
		//copy the packet to the send buffer
		m_nPacketSize = tcpPacket.GetSize();
		tcpPacket.ToNetworkByteStream(m_pSendBuffer);
		//initialize Pipes
		bRetVal = m_USB1ToIpaPipe.Init();
		VALIDATE_WITH_MSG_AND_RETVAL(bRetVal, "Pipe Initialization failed");
		bRetVal = m_IpaToA5ExceptionPipe.Init();
		VALIDATE_WITH_MSG_AND_RETVAL(bRetVal, "Pipe Initialization failed");
		return true;
	}

	///////////////////////////////////////////////////////////////////////////

	virtual bool Teardown() {
		bool bRetVal = true;
		delete[] m_pSendBuffer;
		m_USB1ToIpaPipe.Destroy();
		m_IpaToA5ExceptionPipe.Destroy();
		return bRetVal;
	}



	///////////////////////////////////////////////////////////////////////////

	void ConfigureFilterGlobalRuleForMF(){
		//struct ipa_ioc_add_flt_rule *pRuleTable;
		//Allocate memory for a table with one rule.

		//Instruct the Driver to write this table(with its one rule) to the HW

		//Continue from here - build the rule to catch the fragmented packet
	}

	///////////////////////////////////////////////////////////////////////////


};
//ExceptionsTestFragmentedException

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class ExceptionsTestNonTCPUDP: public ExceptionsTestFixture {

};

///////////////////////////////////////////////////////////////////////////////
//Classes instances:
static ExceptionsTestNonIpPacket exceptionsTestNonIpPacket;
static ExceptionsTestFragmentedException exceptionsTestFragmentedException;

///////////////////////////////////////////////////////////////////////////////
//////////////                         EOF                             ////////
///////////////////////////////////////////////////////////////////////////////
