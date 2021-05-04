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

#include "TestsUtils.h"
#include "RoutingDriverWrapper.h"
#include "HeaderInsertion.h"
#include "Filtering.h"
#include "IPAFilteringTable.h"
#include <string.h>

using namespace IPA;

class IPAExceptionTestFixture: public TestBase {
public:

	IPAExceptionTestFixture() :
			m_uBufferSize(0) {
		memset(m_aBuffer, 0, sizeof(m_aBuffer));
		m_testSuiteName.push_back("Exception");
	}

	virtual bool AddRules() = 0;
	virtual bool ModifyPackets() = 0;
	virtual bool TestLogic() = 0;

	bool Setup() {

		ConfigureScenario(PHASE_SEVEN_TEST_CONFIGURATION);

		m_producer.Open(INTERFACE0_TO_IPA_DATA_PATH,
				INTERFACE0_FROM_IPA_DATA_PATH);
		m_Consumer1.Open(INTERFACE1_TO_IPA_DATA_PATH,
				INTERFACE1_FROM_IPA_DATA_PATH);
		m_Consumer2.Open(INTERFACE2_TO_IPA_DATA_PATH,
				INTERFACE2_FROM_IPA_DATA_PATH);
		m_Consumer3.Open(INTERFACE3_TO_IPA_DATA_PATH,
				INTERFACE3_FROM_IPA_DATA_PATH);
		m_Exceptions.Open(INTERFACE_TO_IPA_EXCEPTION_PATH,
				INTERFACE_FROM_IPA_EXCEPTION_PATH);


		if (!m_Routing.DeviceNodeIsOpened()) {
			LOG_MSG_ERROR(
					"Routing block is not ready for immediate commands!\n");
			return false;
		}
		if (!m_Filtering.DeviceNodeIsOpened()) {
			LOG_MSG_ERROR(
					"Filtering block is not ready for immediate commands!\n");
			return false;
		}
		if (!m_HeaderInsertion.DeviceNodeIsOpened())
		{
			LOG_MSG_ERROR("Header Insertion block is not ready for immediate commands!\n");
			return false;
		}
		m_HeaderInsertion.Reset();

		return true;
	} // Setup()

	bool Run() {
		m_uBufferSize = BUFF_MAX_SIZE;
		LOG_MSG_STACK("Entering Function");

		// Configure the system by adding Routing / Filtering / HDR
		if (!AddRules()) {
			LOG_MSG_ERROR("Failed adding Routing / Filtering / HDR.");
			return false;
		}
		// Load input data (IP packet) from file
		if (!LoadDefaultPacket(m_eIP, m_aBuffer, m_uBufferSize)) {
			LOG_MSG_ERROR("Failed default Packet");
			return false;
		}
		if (!ModifyPackets()) {
			LOG_MSG_ERROR("Failed to modify packets.");
			return false;
		}
		if (!TestLogic()) {
			LOG_MSG_ERROR("Test failed, Input and expected output mismatch.");
			return false;
		}
		LOG_MSG_STACK("Leaving Function (Returning True)");
		return true;
	} // Run()

	bool Teardown() {
		m_producer.Close();
		m_Consumer1.Close();
		m_Consumer2.Close();
		m_Consumer3.Close();
		m_Exceptions.Close();
		return true;
	} // Teardown()

	~IPAExceptionTestFixture() {
	}

	static RoutingDriverWrapper m_Routing;
	static Filtering m_Filtering;
	static HeaderInsertion m_HeaderInsertion;
	InterfaceAbstraction m_producer;
	InterfaceAbstraction m_Consumer1;
	InterfaceAbstraction m_Consumer2;
	InterfaceAbstraction m_Consumer3;
	InterfaceAbstraction m_Exceptions;

protected:
	static const size_t BUFF_MAX_SIZE = 1024;
	static const uint8_t MAX_HEADER_SIZE = 64; // 64Bytes - Max Header Length
	enum ipa_ip_type m_eIP;
	uint8_t m_aBuffer[BUFF_MAX_SIZE]; // Input file \ IP packet
	size_t m_uBufferSize;

};
RoutingDriverWrapper IPAExceptionTestFixture::m_Routing;
Filtering IPAExceptionTestFixture::m_Filtering;
HeaderInsertion IPAExceptionTestFixture::m_HeaderInsertion;

//----------------------------------------------------------------------------------------------------------------------------------------/
// Test001: Test that when a packet with (IPVer != 4) && (IPVer Ver != 6) , an exception packet is created and received & exception_pipe  /
//----------------------------------------------------------------------------------------------------------------------------------------/
class IPAExceptionPacketTest001: public IPAExceptionTestFixture {
public:
	IPAExceptionPacketTest001() {
		m_name = "IPAExceptionPacketTest001";
		m_description = "\
			IPA Exception Test 001 - Test that when a packet with (IPVer != 4) && (IPVer Ver != 6) , an exception packet is created and received & exception_pipe \
			Test Generates send NUM_OF_EXCEPTION_PKTS packets with IP Version changing from  0 to 9.\
			First IP Version == 4, hence it is not considered as exception (same goes for IP Ver == 6) \
			";
		m_eIP = IPA_IP_v4;
		Register(*this);
	}

	virtual bool AddRules() {
		// Clear All Rules
		bool bRetVal = true;
		LOG_MSG_STACK("Entering Function");

		const char bypass0[20] = "Bypass0";
		struct ipa_ioc_get_rt_tbl sRoutingTable;
		IPAFilteringTable cFilterTable;
		struct ipa_flt_rule_add sFilterRuleEntry;
		uint32_t nRTTableHdl=0;
		memset(&sRoutingTable, 0, sizeof(sRoutingTable));

		LOG_MSG_STACK("Entering Function");
		if (!CreateBypassRoutingTable(&m_Routing, m_eIP, bypass0, IPA_CLIENT_TEST2_CONS,
				0,&nRTTableHdl)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			bRetVal = false;
			goto bail;
		}
		LOG_MSG_INFO("CreateBypassRoutingTable completed successfully");
		sRoutingTable.ip = m_eIP;
		strlcpy(sRoutingTable.name, bypass0, sizeof(sRoutingTable.name));
		if (!m_Routing.GetRoutingTable(&sRoutingTable)) {
			LOG_MSG_ERROR(
					"m_routing.GetRoutingTable(&sRoutingTable=0x%p) Failed.", &sRoutingTable);
			bRetVal = false;
			goto bail;
		}
		// Creating Filtering Rules
		cFilterTable.Init(m_eIP,IPA_CLIENT_TEST_PROD,true,1);
		LOG_MSG_INFO("Creation of filtering table completed successfully");

		// Configuring Filtering Rule No.1
		cFilterTable.GeneratePresetRule(0,sFilterRuleEntry);
		sFilterRuleEntry.at_rear = true;
		sFilterRuleEntry.flt_rule_hdl = -1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.action = IPA_PASS_TO_ROUTING;
		sFilterRuleEntry.rule.rt_tbl_hdl = nRTTableHdl;
		if (
				((uint8_t)-1 == cFilterTable.AddRuleToTable(sFilterRuleEntry)) ||
				!m_Filtering.AddFilteringRule(cFilterTable.GetFilteringTable())
				)
		{
			LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
			bRetVal = false;
			goto bail;
		} else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable.ReadRuleFromTable(0)->status);
		}

	bail:
		LOG_MSG_STACK(
				"Leaving Function (Returning %s)", bRetVal?"True":"False");
		return bRetVal;
	} // AddRules()

	virtual bool ModifyPackets() {
		m_eIP = IPA_IP_v6;

		AddRules(); // Need to add Routing / Filtering rules for IPv6 as well.
		return true;
	} // ModifyPacktes ()

	virtual bool TestLogic() {
		int i = 0, nIPVer = 0;;
		memset(m_aExpectedBuffer, 0, sizeof(m_aExpectedBuffer));
		m_aExpectedBuffer[2] = 0x0b;
		m_aExpectedBuffer[3] = 0x80;

		memcpy(m_aExpectedBuffer+8, m_aBuffer, m_uBufferSize);
		m_aExpectedBufSize = m_uBufferSize+8;

		for (i=0;i<NUM_OF_EXCEPTION_PKTS;i++)
		{
		LOG_MSG_INFO("Packet %d\n",i);
			nIPVer = i+4 % 10;
			m_aBuffer[0] = (m_aBuffer[0] & 0x0F)+0x10*nIPVer;// Change to Invalid IP version
			m_aExpectedBuffer[8] = (m_aExpectedBuffer[8] & 0x0F)+0x10*nIPVer;
			if (4 == nIPVer || 6 == nIPVer)
			{
				if (!SendReceiveAndCompare(&m_producer, m_aBuffer, m_uBufferSize,
						&m_Consumer1, m_aExpectedBuffer+8, m_aExpectedBufSize-8))
				{
					LOG_MSG_ERROR("SendReceiveAndCompare failed. IPVer = %d",nIPVer);
					return false;
				}
			} else
			{
				if (!SendReceiveAndCompare(&m_producer, m_aBuffer, m_uBufferSize,
						&m_Exceptions, m_aExpectedBuffer, m_aExpectedBufSize))
				{
				LOG_MSG_ERROR("SendReceiveAndCompare failed. IPVer = %d",nIPVer);
				return false;
				}
			}
		}
		return true;
	}
private:
	static const int NUM_OF_EXCEPTION_PKTS = 9;
	uint8_t m_aExpectedBuffer[BUFF_MAX_SIZE];
	size_t m_aExpectedBufSize;
};

//------------------------------------------------------------------------------------------------------------------------------------------/
// Test003: Test that when Filtering Routes the Packet to the Exception Pipe, an exception packet is created and received & exception_pipe  /
//------------------------------------------------------------------------------------------------------------------------------------------/
class IPAExceptionPacketTest003: public IPAExceptionTestFixture {
public:
	IPAExceptionPacketTest003() {
		m_name = "IPAExceptionPacketTest003";
		m_description = "\
			IPA Exception Test 003 - Test that when Filtering Routes the Packet to the Exception Pipe, an exception packet is created and received & exception_pipe \
			Test Generates a Filtering Table that routes all packets to the Exception Pipe. \
			and verify that the packet is recieved @ the Exception Pipe. \
			";
		m_eIP = IPA_IP_v4;
		Register(*this);
	}

	virtual bool AddRules() {
		// Clear All Rules
		bool bRetVal = true;
		LOG_MSG_STACK("Entering Function");

		const char bypass0[20] = "Bypass0";
		struct ipa_ioc_get_rt_tbl sRoutingTable;
		IPAFilteringTable cFilterTable;
		struct ipa_flt_rule_add sFilterRuleEntry;
		uint32_t nRTTableHdl=0;
		memset(&sRoutingTable, 0, sizeof(sRoutingTable));

		LOG_MSG_STACK("Entering Function");
		if (!CreateBypassRoutingTable(&m_Routing, m_eIP, bypass0, IPA_CLIENT_TEST2_CONS,
				0,&nRTTableHdl)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			bRetVal = false;
			goto bail;
		}
		LOG_MSG_INFO("CreateBypassRoutingTable completed successfully");
		sRoutingTable.ip = m_eIP;
		strlcpy(sRoutingTable.name, bypass0, sizeof(sRoutingTable.name));
		if (!m_Routing.GetRoutingTable(&sRoutingTable)) {
			LOG_MSG_ERROR(
					"m_routing.GetRoutingTable(&sRoutingTable=0x%p) Failed.", &sRoutingTable);
			bRetVal = false;
			goto bail;
		}
		// Creating Filtering Rules
		cFilterTable.Init(m_eIP,IPA_CLIENT_TEST_PROD,true,1);
		LOG_MSG_INFO("Creation of filtering table completed successfully");

		// Configuring Filtering Rule No.1
		cFilterTable.GeneratePresetRule(0,sFilterRuleEntry);
		sFilterRuleEntry.at_rear = true;
		sFilterRuleEntry.flt_rule_hdl = -1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.action = IPA_PASS_TO_EXCEPTION;
		sFilterRuleEntry.rule.rt_tbl_hdl = nRTTableHdl;
		if (
				((uint8_t)-1 == cFilterTable.AddRuleToTable(sFilterRuleEntry)) ||
				!m_Filtering.AddFilteringRule(cFilterTable.GetFilteringTable())
				)
		{
			LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
			bRetVal = false;
			goto bail;
		} else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable.ReadRuleFromTable(0)->status);
		}

	bail:
		LOG_MSG_STACK(
				"Leaving Function (Returning %s)", bRetVal?"True":"False");
		return bRetVal;
	} // AddRules()

	virtual bool ModifyPackets() {
		return true;
	} // ModifyPacktes ()

	virtual bool TestLogic() {
		memset(m_aExpectedBuffer, 0, sizeof(m_aExpectedBuffer));
		m_aExpectedBuffer[2] = 0x0b;
		m_aExpectedBuffer[3] = 0x20;

		memcpy(m_aExpectedBuffer+8, m_aBuffer, m_uBufferSize);
		m_aExpectedBufSize = m_uBufferSize+8;

		if (!SendReceiveAndCompare(&m_producer, m_aBuffer, m_uBufferSize,
			&m_Exceptions, m_aExpectedBuffer, m_aExpectedBufSize))
			{
				LOG_MSG_ERROR("SendReceiveAndCompare failed.");
				return false;
			}
		return true;
		}
private:
	uint8_t m_aExpectedBuffer[BUFF_MAX_SIZE];
	size_t m_aExpectedBufSize;
};

//-----------------------------------------------------------------------------------------------------------------------------------------/
// Test006: Test that when a packet with Internet Header Length < 5 Arrives, an exception packet is created and received & exception_pipe  /
//-----------------------------------------------------------------------------------------------------------------------------------------/
class IPAExceptionPacketTest006: public IPAExceptionTestFixture {
public:
	IPAExceptionPacketTest006() {
		m_name = "IPAExceptionPacketTest006"
		m_description = "\
			IPA Exception Test 006 - Test that when a packet with Internet Header Length < 5 Arrives, an exception packet is created and received & exception_pipe \
			Test Generates a Packet with Internet Header Length (IHL == 4). \
			and verifies that the packet is recieved @ the Exception Pipe. \
			";
		m_eIP = IPA_IP_v4;
		Register(*this);
	}

	virtual bool AddRules() {
		// Clear All Rules
		bool bRetVal = true;
		LOG_MSG_STACK("Entering Function");

		const char bypass0[20] = "Bypass0";
		struct ipa_ioc_get_rt_tbl sRoutingTable;
		IPAFilteringTable cFilterTable;
		struct ipa_flt_rule_add sFilterRuleEntry;
		uint32_t nRTTableHdl=0;
		memset(&sRoutingTable, 0, sizeof(sRoutingTable));

		LOG_MSG_STACK("Entering Function");
		if (!CreateBypassRoutingTable(&m_Routing, m_eIP, bypass0, IPA_CLIENT_TEST2_CONS,
				0,&nRTTableHdl)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			bRetVal = false;
			goto bail;
		}
		LOG_MSG_INFO("CreateBypassRoutingTable completed successfully");
		sRoutingTable.ip = m_eIP;
		strlcpy(sRoutingTable.name, bypass0, sizeof(sRoutingTable.name));
		if (!m_Routing.GetRoutingTable(&sRoutingTable)) {
			LOG_MSG_ERROR(
					"m_routing.GetRoutingTable(&sRoutingTable=0x%p) Failed.", &sRoutingTable);
			bRetVal = false;
			goto bail;
		}
		// Creating Filtering Rules
		cFilterTable.Init(m_eIP,IPA_CLIENT_TEST_PROD,true,1);
		LOG_MSG_INFO("Creation of filtering table completed successfully");

		// Configuring Filtering Rule No.1
		cFilterTable.GeneratePresetRule(0,sFilterRuleEntry);
		sFilterRuleEntry.at_rear = true;
		sFilterRuleEntry.flt_rule_hdl = -1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.action = IPA_PASS_TO_ROUTING;
		sFilterRuleEntry.rule.rt_tbl_hdl = nRTTableHdl;
		if (
				((uint8_t)-1 == cFilterTable.AddRuleToTable(sFilterRuleEntry)) ||
				!m_Filtering.AddFilteringRule(cFilterTable.GetFilteringTable())
				)
		{
			LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
			bRetVal = false;
			goto bail;
		} else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable.ReadRuleFromTable(0)->status);
		}

	bail:
		LOG_MSG_STACK(
				"Leaving Function (Returning %s)", bRetVal?"True":"False");
		return bRetVal;
	} // AddRules()

	virtual bool ModifyPackets() {
		m_aBuffer[0] =(m_aBuffer[0] & 0xF0)+0x04;// Change the IHL to 4
		return true;
	} // ModifyPacktes ()

	virtual bool TestLogic() {
		memset(m_aExpectedBuffer, 0, sizeof(m_aExpectedBuffer));
		m_aExpectedBuffer[2] = 0x0b;
		m_aExpectedBuffer[3] = 0x04;

		memcpy(m_aExpectedBuffer+8, m_aBuffer, m_uBufferSize);
		m_aExpectedBufSize = m_uBufferSize+8;

		if (!SendReceiveAndCompare(&m_producer, m_aBuffer, m_uBufferSize,
			&m_Exceptions, m_aExpectedBuffer, m_aExpectedBufSize))
			{
				LOG_MSG_ERROR("SendReceiveAndCompare failed.");
				return false;
			}
		return true;
		}
private:
	uint8_t m_aExpectedBuffer[BUFF_MAX_SIZE];
	size_t m_aExpectedBufSize;
};

static IPAExceptionPacketTest001 ipaExceptionPacketTest001;
static IPAExceptionPacketTest002 ipaExceptionPacketTest002;
static IPAExceptionPacketTest003 ipaExceptionPacketTest003;
static IPAExceptionPacketTest006 ipaExceptionPacketTest006;

