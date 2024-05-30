/*
 * Copyright (c) 2017-2018,2021 The Linux Foundation. All rights reserved.
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
#include "hton.h" // for htonl
#include "TestsUtils.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define IPV4_DST_ADDR_OFFSET (16)

class IPAHeaderInsertionTestFixture: public TestBase {
public:

	IPAHeaderInsertionTestFixture() : m_uBufferSize(0)
	{
		memset(m_aBuffer, 0, sizeof(m_aBuffer));
		m_testSuiteName.push_back("Insertion");
	}

	virtual bool AddRules() = 0;
	virtual bool ModifyPackets() = 0;
	virtual bool TestLogic() = 0;

	bool Setup()
	{

		ConfigureScenario(PHASE_FIVE_TEST_CONFIGURATION);

		m_producer.Open(INTERFACE0_TO_IPA_DATA_PATH,
				INTERFACE0_FROM_IPA_DATA_PATH);
		m_Consumer1.Open(INTERFACE1_TO_IPA_DATA_PATH,
				INTERFACE1_FROM_IPA_DATA_PATH);
		m_Consumer2.Open(INTERFACE2_TO_IPA_DATA_PATH,
				INTERFACE2_FROM_IPA_DATA_PATH);
		m_Consumer3.Open(INTERFACE3_TO_IPA_DATA_PATH,
				INTERFACE3_FROM_IPA_DATA_PATH);

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
		m_HeaderInsertion.Reset();// resetting this component will reset both Routing and Filtering tables.

		return true;
	} // Setup()

	bool Run()
	{
		m_uBufferSize = BUFF_MAX_SIZE;
		LOG_MSG_STACK("Entering Function");

		// Add the relevant filtering rules
		if (!AddRules()) {
			LOG_MSG_ERROR("Failed adding filtering rules.");
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

	bool Teardown()
	{
		m_HeaderInsertion.Reset();// resetting this component will reset both Routing and Filtering tables.
		m_producer.Close();
		m_Consumer1.Close();
		m_Consumer2.Close();
		m_Consumer3.Close();
		return true;
	} // Teardown()

	unsigned GetHdrSramSize()
	{
		int fd;
		struct ipa_test_mem_partition mem_part;

		fd = open("/dev/ipa_test", O_RDONLY);
		if (fd < 0) {
			printf("Failed opening %s. errno %d: %s\n", "/dev/ipa_test",
				errno, strerror(errno));
			return 0;
		}

		if (ioctl(fd, IPA_TEST_IOC_GET_MEM_PART, &mem_part) < 0) {
			printf("Failed ioctl IPA_TEST_IOC_GET_MEM_PART. errno %d: %s\n",
				errno, strerror(errno));
			close(fd);
			return 0;
		}

		close(fd);

		return mem_part.apps_hdr_size;
	}

	~IPAHeaderInsertionTestFixture() {}

	static RoutingDriverWrapper m_Routing;
	static Filtering m_Filtering;
	static HeaderInsertion m_HeaderInsertion;
	InterfaceAbstraction m_producer;
	InterfaceAbstraction m_Consumer1;
	InterfaceAbstraction m_Consumer2;
	InterfaceAbstraction m_Consumer3;

protected:
	static const size_t BUFF_MAX_SIZE = 1024;
	static const uint8_t MAX_HEADER_SIZE = 64; // 64Bytes - Max Header Length
	enum ipa_ip_type m_eIP;
	uint8_t m_aBuffer[BUFF_MAX_SIZE]; // Input file \ IP packet
	size_t m_uBufferSize;

};
RoutingDriverWrapper IPAHeaderInsertionTestFixture::m_Routing;
Filtering IPAHeaderInsertionTestFixture::m_Filtering;
HeaderInsertion IPAHeaderInsertionTestFixture::m_HeaderInsertion;

//---------------------------------------------------------------------------/
// Test002: Test that 802.3 header was inserted Correctly                        /
//---------------------------------------------------------------------------/
class IPAHeaderInsertionTest001: public IPAHeaderInsertionTestFixture {
public:
	IPAHeaderInsertionTest001() {
		m_name = "IPAHeaderInsertionTest001";
		m_description =
		"Header Insertion Test 001 - Test RMNet Header Insertion\
		1. Generate and commit RMNet.3 header Insertion \
		2. Generate and commit routing table containing bypass rule. \
		3. Generate and commit bypass filtering rule. \
		4. Send a packet, and verify that the RMNet.3 Header was inserted correctly.";
		Register(*this);
		uint8_t aRMNetHeader[6] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
		m_nHeadertoAddSize = sizeof(aRMNetHeader);
		memcpy(m_aHeadertoAdd, aRMNetHeader, m_nHeadertoAddSize);
	}

	// Test Description:
	// 1. Generate and commit single bypass routing table.
	virtual bool AddRules() {
		m_eIP = IPA_IP_v4;
		const char bypass0[20] = "Bypass0";
		struct ipa_ioc_get_rt_tbl sRoutingTable;
		bool bRetVal = true;
		struct ipa_ioc_get_hdr sRetHeader;
		IPAFilteringTable cFilterTable;
		struct ipa_flt_rule_add sFilterRuleEntry;
		uint32_t nRTTableHdl=0;
		memset(&sRoutingTable, 0, sizeof(sRoutingTable));
		memset(&sRetHeader, 0, sizeof(sRetHeader));
		strlcpy(sRetHeader.name, "IEEE802_3", sizeof(sRetHeader.name));

		LOG_MSG_STACK("Entering Function");
		// Create Header:
		// Allocate Memory, populate it, and add in to the Header Insertion.
		struct ipa_ioc_add_hdr * pHeaderDescriptor = NULL;
		pHeaderDescriptor = (struct ipa_ioc_add_hdr *) calloc(1,
				sizeof(struct ipa_ioc_add_hdr)
						+ 1 * sizeof(struct ipa_hdr_add));
		if (!pHeaderDescriptor) {
			LOG_MSG_ERROR("calloc failed to allocate pHeaderDescriptor");
			bRetVal = false;
			goto bail;
		}
		pHeaderDescriptor->commit = true;
		pHeaderDescriptor->num_hdrs = 1;
		strlcpy(pHeaderDescriptor->hdr[0].name, sRetHeader.name, sizeof(pHeaderDescriptor->hdr[0].name));
		memcpy(pHeaderDescriptor->hdr[0].hdr, m_aHeadertoAdd,
				m_nHeadertoAddSize); //Header's Data
		pHeaderDescriptor->hdr[0].hdr_len = m_nHeadertoAddSize;
		pHeaderDescriptor->hdr[0].hdr_hdl = -1; //Return Value
		pHeaderDescriptor->hdr[0].is_partial = false;
		pHeaderDescriptor->hdr[0].status = -1; // Return Parameter
		strlcpy(sRetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(sRetHeader.name));

		if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
		{
			LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
			bRetVal = false;
			goto bail;
		}
		if (!m_HeaderInsertion.GetHeaderHandle(&sRetHeader))
		{
			LOG_MSG_ERROR(" Failed");
			bRetVal = false;
			goto bail;
		}
		if (!CreateBypassRoutingTable(&m_Routing, m_eIP, bypass0, IPA_CLIENT_TEST2_CONS,
				sRetHeader.hdl,&nRTTableHdl)) {
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
		cFilterTable.Init(m_eIP,IPA_CLIENT_TEST_PROD,false,1);
		LOG_MSG_INFO("Creation of filtering table completed successfully");

		// Configuring Filtering Rule No.1
		cFilterTable.GeneratePresetRule(0,sFilterRuleEntry);
		sFilterRuleEntry.at_rear = true;
		sFilterRuleEntry.flt_rule_hdl = -1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.action = IPA_PASS_TO_ROUTING;
		sFilterRuleEntry.rule.rt_tbl_hdl = nRTTableHdl; //put here the handle corresponding to Routing Rule 1
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
		Free(pHeaderDescriptor);
		LOG_MSG_STACK(
				"Leaving Function (Returning %s)", bRetVal?"True":"False");
		return bRetVal;
	} // AddRules()

	virtual bool ModifyPackets() {
		// This test doesn't modify the original IP Packet.
		return true;
	} // ModifyPacktes ()

	virtual bool TestLogic() {
		memset(m_aExpectedBuffer, 0, sizeof(m_aExpectedBuffer));
		m_aExpectedBufSize = 0;

		memcpy(m_aExpectedBuffer, m_aHeadertoAdd, m_nHeadertoAddSize);
		memcpy(m_aExpectedBuffer+m_nHeadertoAddSize,m_aBuffer,m_uBufferSize);
		m_aExpectedBufSize = m_nHeadertoAddSize + m_uBufferSize;
		if (!SendReceiveAndCompare(&m_producer, m_aBuffer, m_uBufferSize,
				&m_Consumer1, m_aExpectedBuffer, m_aExpectedBufSize)) {
			LOG_MSG_ERROR("SendReceiveAndCompare failed.");
			return false;
		}
		return true;
	}
private:
	uint8_t m_aExpectedBuffer[BUFF_MAX_SIZE]; // Input file / IP packet
	size_t m_aExpectedBufSize;
	uint8_t m_aHeadertoAdd[MAX_HEADER_SIZE];
	size_t m_nHeadertoAddSize;
};

//---------------------------------------------------------------------------/
// Test002: Test that 802.3 header was inserted Correctly                        /
//---------------------------------------------------------------------------/
class IPAHeaderInsertionTest002: public IPAHeaderInsertionTestFixture {
public:
	IPAHeaderInsertionTest002() {
		m_name = "IPAHeaderInsertionTest002";
		m_description =
		"Header Insertion Test 002 - Test IEEE802.3 Header Insertion\
		1. Generate and commit IEEE802.3 header Insertion \
		2. Generate and commit routing table containing bypass rule. \
		3. Generate and commit bypass filtering rule. \
		4. Send a packet, and verify that the IEEE802.3 Header was inserted correctly \
		   and that the header Length was updated as well";
		Register(*this);
		uint8_t aIEEE802_3Header[22] = { 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
				0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0x00, 0x46, 0xAE, 0xAF, 0xB0,// the correct size (00 46) is inserted here.
				0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6 };
		m_nHeadertoAddSize = sizeof(aIEEE802_3Header);
		memcpy(m_aHeadertoAdd, aIEEE802_3Header, m_nHeadertoAddSize);
	}

	// Test Description:
	// 1. Generate and commit single bypass routing table.
	virtual bool AddRules() {
		m_eIP = IPA_IP_v4;
		const char bypass0[20] = "Bypass0";
		struct ipa_ioc_get_rt_tbl sRoutingTable;
		bool bRetVal = true;
		struct ipa_ioc_get_hdr sRetHeader;
		IPAFilteringTable cFilterTable;
		struct ipa_flt_rule_add sFilterRuleEntry;
		uint32_t nRTTableHdl=0;
		memset(&sRoutingTable, 0, sizeof(sRoutingTable));
		memset(&sRetHeader, 0, sizeof(sRetHeader));
		strlcpy(sRetHeader.name, "IEEE802_3", sizeof(sRetHeader.name));

		LOG_MSG_STACK("Entering Function");
		// Create Header:
		// Allocate Memory, populate it, and add in to the Header Insertion.
		struct ipa_ioc_add_hdr * pHeaderDescriptor = NULL;
		pHeaderDescriptor = (struct ipa_ioc_add_hdr *) calloc(1,
				sizeof(struct ipa_ioc_add_hdr)
						+ 1 * sizeof(struct ipa_hdr_add));
		if (!pHeaderDescriptor) {
			LOG_MSG_ERROR("calloc failed to allocate pHeaderDescriptor");
			bRetVal = false;
			goto bail;
		}
		pHeaderDescriptor->commit = true;
		pHeaderDescriptor->num_hdrs = 1;
		strlcpy(pHeaderDescriptor->hdr[0].name, sRetHeader.name, sizeof(pHeaderDescriptor->hdr[0].name)); // Header's Name
		memcpy(pHeaderDescriptor->hdr[0].hdr, m_aHeadertoAdd,
				m_nHeadertoAddSize); //Header's Data
    pHeaderDescriptor->hdr[0].hdr[12] = 0x00; //set length to zero, to confirm if ipa updates or not
		pHeaderDescriptor->hdr[0].hdr_len = m_nHeadertoAddSize;
		pHeaderDescriptor->hdr[0].hdr_hdl = -1; //Return Value
		pHeaderDescriptor->hdr[0].is_partial = false;
		pHeaderDescriptor->hdr[0].status = -1; // Return Parameter
		strlcpy(sRetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(sRetHeader.name));

		if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
		{
			LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
			bRetVal = false;
			goto bail;
		}
		if (!m_HeaderInsertion.GetHeaderHandle(&sRetHeader))
		{
			LOG_MSG_ERROR(" Failed");
			bRetVal = false;
			goto bail;
		}
		if (!CreateBypassRoutingTable(&m_Routing, m_eIP, bypass0, IPA_CLIENT_TEST3_CONS,
				sRetHeader.hdl,&nRTTableHdl)) {
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
		cFilterTable.Init(m_eIP,IPA_CLIENT_TEST_PROD,false,1);
		LOG_MSG_INFO("Creation of filtering table completed successfully");

		// Configuring Filtering Rule No.1
		cFilterTable.GeneratePresetRule(0,sFilterRuleEntry);
		sFilterRuleEntry.at_rear = true;
		sFilterRuleEntry.flt_rule_hdl = -1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.action = IPA_PASS_TO_ROUTING;
		sFilterRuleEntry.rule.rt_tbl_hdl = nRTTableHdl; //put here the handle corresponding to Routing Rule 1
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
		Free(pHeaderDescriptor);
		LOG_MSG_STACK(
				"Leaving Function (Returning %s)", bRetVal?"True":"False");
		return bRetVal;
	} // AddRules()

	virtual bool ModifyPackets() {
		// This test doesn't modify the original IP Packet.
		return true;
	} // ModifyPacktes ()

	virtual bool TestLogic() {
		memset(m_aExpectedBuffer, 0, sizeof(m_aExpectedBuffer));
		m_aExpectedBufSize = 0;

		memcpy(m_aExpectedBuffer, m_aHeadertoAdd, m_nHeadertoAddSize);
		memcpy(m_aExpectedBuffer+m_nHeadertoAddSize,m_aBuffer,m_uBufferSize);
		m_aExpectedBufSize = m_nHeadertoAddSize + m_uBufferSize;
		if (!SendReceiveAndCompare(&m_producer, m_aBuffer, m_uBufferSize,
				&m_Consumer2, m_aExpectedBuffer, m_aExpectedBufSize)) {
			LOG_MSG_ERROR("SendReceiveAndCompare failed.");
			return false;
		}
		return true;
	}
private:
	uint8_t m_aExpectedBuffer[BUFF_MAX_SIZE]; // Input file / IP packet
	size_t m_aExpectedBufSize;
	uint8_t m_aHeadertoAdd[MAX_HEADER_SIZE];
	size_t m_nHeadertoAddSize;
};

//---------------------------------------------------------------------------/
// Test003: Test Three Different Header Insertions                           /
//---------------------------------------------------------------------------/
class IPAHeaderInsertionTest003: public IPAHeaderInsertionTestFixture {
public:
	IPAHeaderInsertionTest003() :
	m_aExpectedBufSize(BUFF_MAX_SIZE),
	m_nHeadertoAddSize1(0),
	m_nHeadertoAddSize2(0),
	m_nHeadertoAddSize3(0)
	{
		m_name = "IPAHeaderInsertionTest003";
		m_description =
		"Header Insertion Test 003 - Test RmNet,IEEE802.3 and IEEE802.3 with const (1) addition to the length field\
		1. Generate and commit two types of header Insertion RmNet and IEE802.3 \
		2. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
			Routing table 1 is used to add RmNet Header \
			Routing table 2 is used to add IEEE802.3 Header (requires update of the Length field) \
			Routing table 3 is used to add IEEE802.3 Header with additional const (1) to the length field \
		3. Generate and commit Three filtering rules (MASK = 0xFF..FF). \
			All DST_IP == 127.0.0.1 traffic goes to routing table 1 \
			All DST_IP == 192.169.1.1 traffic goes to routing table 2 \
			All DST_IP == 192.169.1.2 traffic goes to routing table 3";

		Register(*this);
		uint8_t aRMNetHeader[6] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
		uint8_t aIEEE802_3Header1[22] = { 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
				0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0x00, 0x46, 0xAE, 0xAF, 0xB0,
				0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6 };
		uint8_t aIEEE802_3Header2[22] = { 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
				0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0x00, 0x47, 0xAE, 0xAF, 0xB0,
				0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6 };
		m_nHeadertoAddSize1 = sizeof(aRMNetHeader);
		memcpy(m_aHeadertoAdd1, aRMNetHeader, m_nHeadertoAddSize1);
		m_nHeadertoAddSize2 = sizeof(aIEEE802_3Header1);
		memcpy(m_aHeadertoAdd2, aIEEE802_3Header1, m_nHeadertoAddSize2);
		m_nHeadertoAddSize3 = sizeof(aIEEE802_3Header2);
		memcpy(m_aHeadertoAdd3, aIEEE802_3Header2, m_nHeadertoAddSize3);
	}

	// Test Description:
	// 1. Generate and commit single bypass routing table.
	virtual bool AddRules() {
		m_eIP = IPA_IP_v4;
		const char aBypass1[20] = "Bypass1";
		const char aBypass2[20] = "Bypass2";
		const char aBypass3[20] = "Bypass3";
		uint32_t nTableHdl01, nTableHdl02, nTableHdl03;
		bool bRetVal = true;
		IPAFilteringTable cFilterTable0;
		struct ipa_flt_rule_add sFilterRuleEntry;
		struct ipa_ioc_get_hdr sGetHeader1,sGetHeader2;

		LOG_MSG_STACK("Entering Function");
		memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));
		memset(&sGetHeader1, 0, sizeof(sGetHeader1));
		memset(&sGetHeader2, 0, sizeof(sGetHeader2));
		// Create Header:
		// Allocate Memory, populate it, and add in to the Header Insertion.
		struct ipa_ioc_add_hdr * pHeaderDescriptor = NULL;
		pHeaderDescriptor = (struct ipa_ioc_add_hdr *) calloc(1,
				sizeof(struct ipa_ioc_add_hdr)
						+ 2 * sizeof(struct ipa_hdr_add));
		if (!pHeaderDescriptor) {
			LOG_MSG_ERROR("calloc failed to allocate pHeaderDescriptor");
			bRetVal = false;
			goto bail;
		}

		pHeaderDescriptor->commit = true;
		pHeaderDescriptor->num_hdrs = 2;
		// Adding Header No1.
		strlcpy(pHeaderDescriptor->hdr[0].name, "RMNet", sizeof(pHeaderDescriptor->hdr[0].name)); // Header's Name
		memcpy(pHeaderDescriptor->hdr[0].hdr, m_aHeadertoAdd1,
				m_nHeadertoAddSize1); //Header's Data
		pHeaderDescriptor->hdr[0].hdr_len    = m_nHeadertoAddSize1;
		pHeaderDescriptor->hdr[0].hdr_hdl    = -1; //Return Value
		pHeaderDescriptor->hdr[0].is_partial = false;
		pHeaderDescriptor->hdr[0].status     = -1; // Return Parameter

		// Adding Header No2.
		strlcpy(pHeaderDescriptor->hdr[1].name, "IEEE_802_3", sizeof(pHeaderDescriptor->hdr[1].name)); // Header's Name
		memcpy(pHeaderDescriptor->hdr[1].hdr, m_aHeadertoAdd2,
				m_nHeadertoAddSize2); //Header's Data
		pHeaderDescriptor->hdr[1].hdr_len    = m_nHeadertoAddSize2;
		pHeaderDescriptor->hdr[1].hdr_hdl    = -1; //Return Value
		pHeaderDescriptor->hdr[1].is_partial = false;
		pHeaderDescriptor->hdr[1].status     = -1; // Return Parameter

		strlcpy(sGetHeader1.name, pHeaderDescriptor->hdr[0].name, sizeof(sGetHeader1.name));
		strlcpy(sGetHeader2.name, pHeaderDescriptor->hdr[1].name, sizeof(sGetHeader2.name));

		if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
		{
			LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
			bRetVal = false;
			goto bail;
		}
		if (!m_HeaderInsertion.GetHeaderHandle(&sGetHeader1))
		{
			LOG_MSG_ERROR(" Failed");
			bRetVal = false;
			goto bail;
		}
		LOG_MSG_DEBUG("Received Header1 Handle = 0x%x",sGetHeader1.hdl);
		if (!m_HeaderInsertion.GetHeaderHandle(&sGetHeader2))
		{
			LOG_MSG_ERROR(" Failed");
			bRetVal = false;
			goto bail;
		}
		LOG_MSG_DEBUG("Received Header2 Handle = 0x%x",sGetHeader2.hdl);
		if (!CreateBypassRoutingTable(&m_Routing, m_eIP, aBypass1, IPA_CLIENT_TEST2_CONS,
				sGetHeader1.hdl,&nTableHdl01)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			bRetVal = false;
			goto bail;
		}
		if (!CreateBypassRoutingTable(&m_Routing, m_eIP, aBypass2, IPA_CLIENT_TEST3_CONS,
				sGetHeader2.hdl,&nTableHdl02)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			bRetVal = false;
			goto bail;
		}
		if (!CreateBypassRoutingTable(&m_Routing, m_eIP, aBypass3, IPA_CLIENT_TEST4_CONS,
				sGetHeader2.hdl,&nTableHdl03)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			bRetVal = false;
			goto bail;
		}
		LOG_MSG_INFO("Creation of three bypass routing tables completed successfully TblHdl1=0x%x, TblHdl2=0x%x, TblHdl3=0x%x",
				nTableHdl01,nTableHdl02,nTableHdl03);

		// Creating Filtering Rules
		cFilterTable0.Init(m_eIP,IPA_CLIENT_TEST_PROD,false,3);
		LOG_MSG_INFO("Creation of filtering table completed successfully");

		// Configuring Filtering Rule No.1
		cFilterTable0.GeneratePresetRule(1,sFilterRuleEntry);
		sFilterRuleEntry.at_rear = true;
		sFilterRuleEntry.flt_rule_hdl=-1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
		sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl01; //put here the handle corresponding to Routing Rule 1
		sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		if ((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry))
		{
			LOG_MSG_ERROR ("Adding Rule (0) to Filtering table Failed.");
			bRetVal = false;
			goto bail;
		}

		// Configuring Filtering Rule No.2
		sFilterRuleEntry.flt_rule_hdl=-1; // return Value
		sFilterRuleEntry.status = -1; // return Value
		sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl02; //put here the handle corresponding to Routing Rule 2
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1.
		if ((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry))
		{
			LOG_MSG_ERROR ("Adding Rule(1) to Filtering table Failed.");
			bRetVal = false;
			goto bail;
		}

		// Configuring Filtering Rule No.3
		sFilterRuleEntry.flt_rule_hdl=-1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl03; //put here the handle corresponding to Routing Rule 2
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.
		if ((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry))
		{
			LOG_MSG_ERROR ("Adding Rule(2) to Filtering table Failed.");
			bRetVal = false;
			goto bail;
		}

		if (!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())) {
			LOG_MSG_ERROR ("Failed to commit Filtering rules");
			bRetVal = false;
			goto bail;
		}

		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable0.ReadRuleFromTable(0)->status);
		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,cFilterTable0.ReadRuleFromTable(1)->status);
		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,cFilterTable0.ReadRuleFromTable(2)->status);

	bail:
		Free(pHeaderDescriptor);
		LOG_MSG_STACK(
				"Leaving Function (Returning %s)", bRetVal?"True":"False");
		return bRetVal;
	} // AddRules()

	virtual bool ModifyPackets() {
		// This test doesn't modify the original IP Packet.
		return true;
	} // ModifyPacktes ()

	virtual bool TestLogic() {
		bool bRetVal = true;
		m_aExpectedBufSize = 0;
		uint32_t nIPv4DSTAddr;

		LOG_MSG_STACK("Entering Function");

		//Packet No. 1
		memset(m_aExpectedBuffer, 0, sizeof(m_aExpectedBuffer));
		nIPv4DSTAddr = ntohl(0x7F000001);
		memcpy (&m_aBuffer[IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,sizeof(nIPv4DSTAddr));
		memcpy(m_aExpectedBuffer, m_aHeadertoAdd1, m_nHeadertoAddSize1);
		memcpy(m_aExpectedBuffer+m_nHeadertoAddSize1,m_aBuffer,m_uBufferSize);
		m_aExpectedBufSize = m_nHeadertoAddSize1 + m_uBufferSize;
		if (!SendReceiveAndCompare(&m_producer, m_aBuffer, m_uBufferSize,
				&m_Consumer1, m_aExpectedBuffer, m_aExpectedBufSize))
		{
			LOG_MSG_ERROR("SendReceiveAndCompare failed.");
			bRetVal=false;
		}

		//Packet No. 2
		memset(m_aExpectedBuffer, 0, sizeof(m_aExpectedBuffer));
		nIPv4DSTAddr = ntohl(0xC0A80101);//192.168.1.1
		memcpy (&m_aBuffer[IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,sizeof(nIPv4DSTAddr));
		memcpy(m_aExpectedBuffer, m_aHeadertoAdd2, m_nHeadertoAddSize2);
		memcpy(m_aExpectedBuffer+m_nHeadertoAddSize2,m_aBuffer,m_uBufferSize);
		m_aExpectedBufSize = m_nHeadertoAddSize2 + m_uBufferSize;
		if (!SendReceiveAndCompare(&m_producer, m_aBuffer, m_uBufferSize,
				&m_Consumer2, m_aExpectedBuffer, m_aExpectedBufSize))
		{
			LOG_MSG_ERROR("SendReceiveAndCompare failed.");
			bRetVal=false;
		}

		//Packet No. 3
		nIPv4DSTAddr = ntohl(0xC0A80102);//192.168.1.2
		memcpy (&m_aBuffer[IPV4_DST_ADDR_OFFSET],&nIPv4DSTAddr,sizeof(nIPv4DSTAddr));
		memcpy(m_aExpectedBuffer, m_aHeadertoAdd3, m_nHeadertoAddSize3);
		memcpy(m_aExpectedBuffer+m_nHeadertoAddSize3,m_aBuffer,m_uBufferSize);
		m_aExpectedBufSize = m_nHeadertoAddSize3 + m_uBufferSize;
		if (!SendReceiveAndCompare(&m_producer, m_aBuffer, m_uBufferSize,
				&m_Consumer3, m_aExpectedBuffer, m_aExpectedBufSize))
		{
			LOG_MSG_ERROR("SendReceiveAndCompare failed.");
			bRetVal=false;
		}

		LOG_MSG_STACK("Leaving Function (Returning %s)",bRetVal?"True":"False");
		return bRetVal;
	}
private:
	uint8_t m_aExpectedBuffer[BUFF_MAX_SIZE]; // Input file / IP packet
	size_t m_aExpectedBufSize;
	uint8_t m_aHeadertoAdd1[MAX_HEADER_SIZE],m_aHeadertoAdd2[MAX_HEADER_SIZE],m_aHeadertoAdd3[MAX_HEADER_SIZE];
	size_t m_nHeadertoAddSize1,m_nHeadertoAddSize2,m_nHeadertoAddSize3;
};

class IPAHeaderInsertionTest004: public IPAHeaderInsertionTestFixture {
public:
	IPAHeaderInsertionTest004() {
		m_name = "IPAHeaderInsertionTest004";
		m_description =
		"Header Insertion Test 004 - Test header insertion with bad len values.";
		Register(*this);
		uint8_t aRMNetHeader[6] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
		m_nHeadertoAddSize = sizeof(aRMNetHeader);
		memcpy(m_aHeadertoAdd, aRMNetHeader, m_nHeadertoAddSize);
	}

	virtual bool AddRules() {
		// Not adding any rules here.
		return true;
	} // AddRules()

	virtual bool ModifyPackets() {
		// This test doesn't modify the original IP Packet.
		return true;
	} // ModifyPacktes ()

	bool AddSingleHeaderAndCheck(uint8_t len) {
		m_eIP = IPA_IP_v4;
		bool bRetVal = true;
		struct ipa_ioc_get_hdr sRetHeader;
		memset(&sRetHeader, 0, sizeof(sRetHeader));
		strlcpy(sRetHeader.name, "Generic", sizeof(sRetHeader.name));

		LOG_MSG_STACK("Entering Function");
		// Create Header:
		// Allocate Memory, populate it, and add in to the Header Insertion.
		struct ipa_ioc_add_hdr * pHeaderDescriptor = NULL;
		pHeaderDescriptor = (struct ipa_ioc_add_hdr *) calloc(1,
				sizeof(struct ipa_ioc_add_hdr)
						+ 1 * sizeof(struct ipa_hdr_add));
		if (!pHeaderDescriptor) {
			LOG_MSG_ERROR("calloc failed to allocate pHeaderDescriptor");
			bRetVal = false;
			goto bail;
		}
		pHeaderDescriptor->commit = true;
		pHeaderDescriptor->num_hdrs = 1;
		strlcpy(pHeaderDescriptor->hdr[0].name, sRetHeader.name, sizeof(pHeaderDescriptor->hdr[0].name));
		memcpy(pHeaderDescriptor->hdr[0].hdr, m_aHeadertoAdd,
				m_nHeadertoAddSize); //Header's Data
		pHeaderDescriptor->hdr[0].hdr_len = len;
		pHeaderDescriptor->hdr[0].hdr_hdl = -1; //Return Value
		pHeaderDescriptor->hdr[0].is_partial = false;
		pHeaderDescriptor->hdr[0].status = -1; // Return Parameter
		strlcpy(sRetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(sRetHeader.name));

		if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
		{
			LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
			bRetVal = false;
			goto bail;
		}

		if (!m_HeaderInsertion.GetHeaderHandle(&sRetHeader))
		{
			LOG_MSG_ERROR(" Failed");
			bRetVal = false;
			goto bail;
		}

	bail:
		Free(pHeaderDescriptor);
		LOG_MSG_STACK(
				"Leaving Function (Returning %s)", bRetVal?"True":"False");
		return bRetVal;
	} // AddSingleHeaderAndCheck()

	virtual bool TestLogic() {

		// Try to add headers with invalid values.
		// Valid values are between 1 to IPA_HDR_MAX_SIZE (64).
		// We expect the below functions to fail.
		if (AddSingleHeaderAndCheck(0)) {
			LOG_MSG_ERROR("This is unexpected, this can't succeed");
			return false;
		}

		if (AddSingleHeaderAndCheck(MAX_HEADER_SIZE + 1)) {
			LOG_MSG_ERROR("This is unexpected, this can't succeed");
			return false;
		}

		// Add one header which is OK
		if (!AddSingleHeaderAndCheck(m_nHeadertoAddSize)) {
			LOG_MSG_ERROR("This is unexpected, this can't succeed");
			return false;
		}
		return true;
	}

private:
	uint8_t m_aHeadertoAdd[MAX_HEADER_SIZE];
	size_t m_nHeadertoAddSize;
};

class IPAHeaderInsertionTest005: public IPAHeaderInsertionTestFixture {
public:
	IPAHeaderInsertionTest005() {
		m_name = "IPAHeaderInsertionTest005";
		m_description =
		"Header Insertion Test 005 - Test Multiple RMNet Header Insertion\
		- Stress test - Generate and commit multiple header Insertion";
		this->m_runInRegression = false;
		Register(*this);
		uint8_t aRMNetHeader[6] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
		m_nHeadertoAddSize = sizeof(aRMNetHeader);
		memcpy(m_aHeadertoAdd, aRMNetHeader, m_nHeadertoAddSize);
	}

	// Test Description:
	// 1. Generate and commit single bypass routing table.
	virtual bool AddRules() {
		m_eIP = IPA_IP_v4;
		bool bRetVal = true;
		struct ipa_ioc_get_hdr sRetHeader;
		char Name[] = "IEEE802_3\0";

		memset(&sRetHeader, 0, sizeof(sRetHeader));
		strlcpy (sRetHeader.name, Name, sizeof(sRetHeader.name));
		LOG_MSG_STACK("Entering Function");
		// Create Header:
		// Allocate Memory, populate it, and add in to the Header Insertion.
		struct ipa_ioc_add_hdr * pHeaderDescriptor = NULL;
		pHeaderDescriptor = (struct ipa_ioc_add_hdr *) calloc(1,
				sizeof(struct ipa_ioc_add_hdr)
						+ 1 * sizeof(struct ipa_hdr_add));
		if (!pHeaderDescriptor) {
			LOG_MSG_ERROR("calloc failed to allocate pHeaderDescriptor");
			bRetVal = false;
			goto bail;
		}
		pHeaderDescriptor->commit = true;
		pHeaderDescriptor->num_hdrs = 1;
		strlcpy(pHeaderDescriptor->hdr[0].name, sRetHeader.name, sizeof(pHeaderDescriptor->hdr[0].name));
		memcpy(pHeaderDescriptor->hdr[0].hdr, m_aHeadertoAdd,
				m_nHeadertoAddSize); //Header's Data
		pHeaderDescriptor->hdr[0].hdr_len = m_nHeadertoAddSize;
		pHeaderDescriptor->hdr[0].hdr_hdl = -1; //Return Value
		pHeaderDescriptor->hdr[0].is_partial = false;
		pHeaderDescriptor->hdr[0].status = -1; // Return Parameter
		strlcpy(sRetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(sRetHeader.name));

		// stress test to check if the target crashes, failure is expected before reaching 500
		for (int i = 0; i < 500; i++) {
			LOG_MSG_DEBUG("IPAHeaderInsertionTest005::AddRules iter=%d\n",i);
			if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
			{
				LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed on %d iteration.\n",i);
				goto bail;
			}
		}

	bail:
		Free(pHeaderDescriptor);
		LOG_MSG_STACK(
				"Leaving Function (Returning %s)", bRetVal?"True":"False");
		return bRetVal;
	} // AddRules()

	virtual bool ModifyPackets() {
		// This test doesn't modify the original IP Packet.
		return true;
	} // ModifyPacktes ()

	virtual bool TestLogic() {
		return true;
	}
private:
	uint8_t m_aHeadertoAdd[MAX_HEADER_SIZE];
	size_t m_nHeadertoAddSize;
};

class IPAHeaderInsertionTest006: public IPAHeaderInsertionTestFixture {
public:
	IPAHeaderInsertionTest006() {
		m_name = "IPAHeaderInsertionTest006";
		m_description =
		"Header Insertion Test 006 - Test header distriburion between SRAM and DDR\
			- fill SRAM and some DDR, use DDR header";
		this->m_runInRegression = true;
		Register(*this);
		uint8_t aRMNetHeader[6] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
		m_nHeadertoAddSize = sizeof(aRMNetHeader);
		memcpy(m_aHeadertoAdd, aRMNetHeader, m_nHeadertoAddSize);
		m_minIPAHwType = IPA_HW_v5_0;

		// The bin size is 8
		// We are going to add number of headers to occupy twice the size of the SRAM buffer
		m_InitialHeadersNum = GetHdrSramSize() / 8 * 2;
		m_HeadersNumToDelete = 0;
		m_HeadersNumToAddAgain = 0;
	}

	virtual bool AddRules() {
		bool bRetVal = true;
		m_eIP = IPA_IP_v4;
		struct ipa_ioc_add_hdr *pHeaderDescriptor = NULL;
		struct ipa_ioc_del_hdr *pDelHeaderDescriptor = NULL;

		memset(&m_RetHeader, 0, sizeof(m_RetHeader));
		LOG_MSG_STACK("Entering Function");

		if (m_InitialHeadersNum <= 0)
		{
			LOG_MSG_ERROR("Initial headers number is set to 0!\n");
			bRetVal = false;
			goto bail;
		}

		pHeaderDescriptor = (struct ipa_ioc_add_hdr *) calloc(1,
			sizeof(struct ipa_ioc_add_hdr) + 1 * sizeof(struct ipa_hdr_add));
		if (m_HeadersNumToDelete > 0)
			pDelHeaderDescriptor = (struct ipa_ioc_del_hdr *)calloc(1,
				sizeof(struct ipa_ioc_del_hdr) + m_HeadersNumToDelete * sizeof(struct ipa_hdr_del));
		if (!pHeaderDescriptor || (m_HeadersNumToDelete > 0 && !pDelHeaderDescriptor))
		{
			LOG_MSG_ERROR("calloc failed to allocate ipa_ioc_add_hdr or ipa_ioc_del_hdr");
			bRetVal = false;
			goto bail;
		}

		// Add bunch of headers to SRAM and DDR
		pHeaderDescriptor->commit = true;
		pHeaderDescriptor->num_hdrs = 1;
		memcpy(pHeaderDescriptor->hdr[0].hdr, m_aHeadertoAdd, m_nHeadertoAddSize);
		pHeaderDescriptor->hdr[0].hdr_len = m_nHeadertoAddSize;
		pHeaderDescriptor->hdr[0].hdr_hdl = -1; //Return Value
		pHeaderDescriptor->hdr[0].is_partial = false;
		pHeaderDescriptor->hdr[0].status = -1; // Return Parameter

		fflush(stderr);
		fflush(stdout);
		ret = system("cat /sys/kernel/debug/ipa/hdr");

		for (int i = 0; i < m_InitialHeadersNum; i++)
		{
			LOG_MSG_DEBUG("%s::%s iter=%d\n", typeid(this).name(), __func__, i);
			snprintf(pHeaderDescriptor->hdr[0].name, sizeof(pHeaderDescriptor->hdr[0].name), "IEEE802_3_%03d", i);
			if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
			{
				LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed on %d iteration.\n", i);
				bRetVal = false;
				goto bail;
			}
			// Store header descriptors to delete
			if (m_HeadersNumToDelete > 0 && i < m_HeadersNumToDelete) {
				pDelHeaderDescriptor->hdl[i].hdl = pHeaderDescriptor->hdr[0].hdr_hdl;
			}
		}
		strlcpy(m_RetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(m_RetHeader.name));

		fflush(stderr);
		fflush(stdout);
		ret = system("cat /sys/kernel/debug/ipa/hdr");


		if (m_HeadersNumToDelete > 0)
		{
			// Delete few headers from SRAM
			pDelHeaderDescriptor->commit = true;
			pDelHeaderDescriptor->num_hdls = m_HeadersNumToDelete;
			for (int i = 0; i < m_HeadersNumToDelete; i++)
				pDelHeaderDescriptor->hdl[i].status = -1; // Return Parameter
			if (!m_HeaderInsertion.DeleteHeader(pDelHeaderDescriptor))
			{
				LOG_MSG_ERROR("m_HeaderInsertion.DeleteHeader(pDelHeaderDescriptor) Failed");
				bRetVal = false;
				goto bail;
			}

			fflush(stderr);
			fflush(stdout);
			ret = system("cat /sys/kernel/debug/ipa/hdr");
		}

		if (m_HeadersNumToAddAgain > 0)
		{
			// Add few new headers to SRAM
			pHeaderDescriptor->commit = true;
			pHeaderDescriptor->num_hdrs = 1;
			memcpy(pHeaderDescriptor->hdr[0].hdr, m_aHeadertoAdd, m_nHeadertoAddSize);
			pHeaderDescriptor->hdr[0].hdr_len = m_nHeadertoAddSize;
			pHeaderDescriptor->hdr[0].hdr_hdl = -1; //Return Value
			pHeaderDescriptor->hdr[0].is_partial = false;
			pHeaderDescriptor->hdr[0].status = -1; // Return Parameter

			for (int i = 0; i < m_HeadersNumToAddAgain; i++) {
				LOG_MSG_DEBUG("%s::%s iter=%d\n", typeid(this).name(), __func__, i);
				snprintf(pHeaderDescriptor->hdr[0].name, sizeof(pHeaderDescriptor->hdr[0].name), "IEEE802_3_%03d_2", i);
				if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
				{
					LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed on %d iteration.\n", i);
					bRetVal = false;
					goto bail;
				}
			}
			strlcpy(m_RetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(m_RetHeader.name));

			fflush(stderr);
			fflush(stdout);
			ret = system("cat /sys/kernel/debug/ipa/hdr");
		}

		// Use last added header for traffic
		if (!m_HeaderInsertion.GetHeaderHandle(&m_RetHeader))
		{
			LOG_MSG_ERROR(" Failed");
			bRetVal = false;
			goto bail;
		}

		bRetVal = CreateFilteringAndRouting();
	bail:
		Free(pHeaderDescriptor);
		if (pDelHeaderDescriptor)
			Free(pDelHeaderDescriptor);
		LOG_MSG_STACK("Leaving %s (Returning %s)", __func__, bRetVal ? "True" : "False");
		return bRetVal;
	} // AddRules()

	virtual bool CreateFilteringAndRouting() {
		uint32_t nRTTableHdl=0;
		const char bypass0[20] = "Bypass0";
		struct ipa_ioc_get_rt_tbl sRoutingTable;
		struct ipa_flt_rule_add sFilterRuleEntry;
		IPAFilteringTable cFilterTable;

		memset(&sRoutingTable, 0, sizeof(sRoutingTable));

		// Create RT table
		if (!CreateBypassRoutingTable(&m_Routing, m_eIP, bypass0, IPA_CLIENT_TEST2_CONS,
				m_RetHeader.hdl,&nRTTableHdl)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			return false;
		}
		LOG_MSG_INFO("CreateBypassRoutingTable completed successfully");
		sRoutingTable.ip = m_eIP;
		strlcpy(sRoutingTable.name, bypass0, sizeof(sRoutingTable.name));
		if (!m_Routing.GetRoutingTable(&sRoutingTable)) {
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&sRoutingTable=0x%p) Failed.",
				&sRoutingTable);
			return false;
		}

		// Creating Filtering Rules
		cFilterTable.Init(m_eIP, IPA_CLIENT_TEST_PROD, false, 1);
		LOG_MSG_INFO("Creation of filtering table completed successfully");

		// Configuring Filtering Rule No.1
		cFilterTable.GeneratePresetRule(0, sFilterRuleEntry);
		sFilterRuleEntry.at_rear = true;
		sFilterRuleEntry.flt_rule_hdl = -1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.action = IPA_PASS_TO_ROUTING;
		sFilterRuleEntry.rule.rt_tbl_hdl = nRTTableHdl; //put here the handle corresponding to Routing Rule 1
		if (((uint8_t)-1 == cFilterTable.AddRuleToTable(sFilterRuleEntry)) ||
		    !m_Filtering.AddFilteringRule(cFilterTable.GetFilteringTable()))
		{
			LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
			return false;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n",
				cFilterTable.ReadRuleFromTable(0)->flt_rule_hdl,
				cFilterTable.ReadRuleFromTable(0)->status);
		}
		return true;
	}

	virtual bool ModifyPackets() {
		return true;
	}

	virtual bool TestLogic() {
		memset(m_aExpectedBuffer, 0, sizeof(m_aExpectedBuffer));
		m_aExpectedBufSize = 0;

		memcpy(m_aExpectedBuffer, m_aHeadertoAdd, m_nHeadertoAddSize);
		memcpy(m_aExpectedBuffer+m_nHeadertoAddSize,m_aBuffer,m_uBufferSize);
		m_aExpectedBufSize = m_nHeadertoAddSize + m_uBufferSize;
		if (!SendReceiveAndCompare(&m_producer, m_aBuffer, m_uBufferSize,
				&m_Consumer1, m_aExpectedBuffer, m_aExpectedBufSize)) {
			LOG_MSG_ERROR("SendReceiveAndCompare failed.");
			return false;
		}
		return true;
	}

protected:
	struct ipa_ioc_get_hdr m_RetHeader;
	int m_InitialHeadersNum;
	int m_HeadersNumToDelete;
	int m_HeadersNumToAddAgain;

private:
	uint8_t m_aExpectedBuffer[BUFF_MAX_SIZE]; // Input file / IP packet
	size_t m_aExpectedBufSize;
	uint8_t m_aHeadertoAdd[MAX_HEADER_SIZE];
	size_t m_nHeadertoAddSize;
	int ret;
};

class IPAHeaderInsertionTest007: public IPAHeaderInsertionTest006 {
public:
	IPAHeaderInsertionTest007() {
		m_name = "IPAHeaderInsertionTest007";
		m_description =
		"Header Insertion Test 007 - Test header distriburion between SRAM and DDR\
			- fill SRAM and some DDR, free some SRAM, use DDR header";
		// We will delete half of the headers in the SRAM,
		// which is quarter of the total initial headers number
		m_HeadersNumToDelete = m_InitialHeadersNum / 4;
		m_HeadersNumToAddAgain = 0;
	}
};

class IPAHeaderInsertionTest008: public IPAHeaderInsertionTest006 {
public:
	IPAHeaderInsertionTest008() {
		m_name = "IPAHeaderInsertionTest008";
		m_description =
		"Header Insertion Test 008 - Test header distriburion between SRAM and DDR\
			- fill SRAM and some DDR, free some SRAM, add few new SRAM headers, \
			use last SRAM header";
		// We will delete half of the headers in the SRAM,
		// which is quarter of the total initial headers number
		m_HeadersNumToDelete = m_InitialHeadersNum / 4;
		// We will add again half of the number of headers we deleted
		m_HeadersNumToAddAgain = m_HeadersNumToDelete / 2;
	}
};

class IPAHeaderInsertionTest009: public IPAHeaderInsertionTest006 {
public:
	IPAHeaderInsertionTest009() {
		m_name = "IPAHeaderInsertionTest009";
		m_description =
		"Header Insertion Test 009 - Test header distriburion between SRAM and DDR \
			- fill SRAM and some DDR, free some SRAM and DDR, \
			add new SRAM and DDR headers, \
			use last added DDR header";
		// We will delete all the headers in SRAM and half of the headers in the DDR,
		// which is 3/4 of the total initial headers number
		m_HeadersNumToDelete = m_InitialHeadersNum - (m_InitialHeadersNum / 4);
		// We will add again one less header than deleted
		m_HeadersNumToAddAgain = m_InitialHeadersNum - m_HeadersNumToDelete - 1;
	}
};

class IPAHeaderInsertionTest010: public IPAHeaderInsertionTestFixture {
public:
	IPAHeaderInsertionTest010() :
	m_aExpectedBufSize(BUFF_MAX_SIZE),
	m_nHeadertoAddSize1(0),
	m_nHeadertoAddSize2(0)
	{
		m_name = "IPAHeaderInsertionTest010";
		m_description =
		"Header Insertion Test 010 - Test header distriburion between SRAM and DDR\
			- fill SRAM and some DDR, use one SRAM and one DDR header";
		m_minIPAHwType = IPA_HW_v5_0;

		Register(*this);
		uint8_t aIEEE802_3Header1[22] = { 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
				0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0x00, 0x46, 0xAE, 0xAF, 0xB0,
				0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6 };
		uint8_t aIEEE802_3Header2[22] = { 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6,
				0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0x00, 0x47, 0xAE, 0xAF, 0xB0,
				0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6 };
		m_nHeadertoAddSize1 = sizeof(aIEEE802_3Header1);
		memcpy(m_aHeadertoAdd1, aIEEE802_3Header1, m_nHeadertoAddSize1);
		m_nHeadertoAddSize2 = sizeof(aIEEE802_3Header2);
		memcpy(m_aHeadertoAdd2, aIEEE802_3Header2, m_nHeadertoAddSize2);

		// The packet size is 22, therefore the bin size is 24
		// We are going to add number of headers to occupy twice the size of the SRAM buffer
		m_InitialHeadersNum = GetHdrSramSize() / 24 * 2;
	}

	virtual bool AddRules() {
		m_eIP = IPA_IP_v4;
		const char aBypass1[20] = "Bypass1";
		const char aBypass2[20] = "Bypass2";
		uint32_t nTableHdl01, nTableHdl02;
		bool bRetVal = true;
		IPAFilteringTable cFilterTable0;
		struct ipa_flt_rule_add sFilterRuleEntry;
		struct ipa_ioc_add_hdr *pHeaderDescriptor = NULL;

		LOG_MSG_STACK("Entering Function");

		if (m_InitialHeadersNum <= 0)
		{
			LOG_MSG_ERROR("Initial headers number is set to 0!\n");
			bRetVal = false;
			goto bail;
		}

		memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));
		memset(&m_RetHeader1, 0, sizeof(m_RetHeader1));
		memset(&m_RetHeader2, 0, sizeof(m_RetHeader2));

		pHeaderDescriptor = (struct ipa_ioc_add_hdr *) calloc(1,
			sizeof(struct ipa_ioc_add_hdr) + 1 * sizeof(struct ipa_hdr_add));
		if (!pHeaderDescriptor)
		{
			LOG_MSG_ERROR("calloc failed to allocate ipa_ioc_add_hdr");
			bRetVal = false;
			goto bail;
		}

		fflush(stderr);
		fflush(stdout);
		ret = system("cat /sys/kernel/debug/ipa/hdr");

		// Add one header to SRAM
		pHeaderDescriptor->commit = true;
		pHeaderDescriptor->num_hdrs = 1;
		pHeaderDescriptor->hdr[0].status = -1; // Return Parameter
		pHeaderDescriptor->hdr[0].hdr_hdl = -1; //Return Value
		pHeaderDescriptor->hdr[0].is_partial = false;

		memcpy(pHeaderDescriptor->hdr[0].hdr, m_aHeadertoAdd1, m_nHeadertoAddSize1);
		pHeaderDescriptor->hdr[0].hdr_len = m_nHeadertoAddSize1;
		strlcpy(pHeaderDescriptor->hdr[0].name, "IEEE802_3_SRAM", sizeof(pHeaderDescriptor->hdr[0].name));
		strlcpy(m_RetHeader1.name, pHeaderDescriptor->hdr[0].name, sizeof(m_RetHeader1.name));
		if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
		{
			LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed\n");
			bRetVal = false;
			goto bail;
		}

		// Add bunch of headers to SRAM and DDR
		for (int i = 1; i < m_InitialHeadersNum; i++)
		{
			LOG_MSG_DEBUG("%s::%s iter=%d\n", typeid(this).name(), __func__, i);
			memcpy(pHeaderDescriptor->hdr[0].hdr, m_aHeadertoAdd2, m_nHeadertoAddSize2);
			pHeaderDescriptor->hdr[0].hdr_len = m_nHeadertoAddSize2;
			snprintf(pHeaderDescriptor->hdr[0].name, sizeof(pHeaderDescriptor->hdr[0].name),
				"IEEE802_3_%03d", i);
			if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
			{
				LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed on %d iteration.\n", i);
				bRetVal = false;
				goto bail;
			}
		}
		strlcpy(m_RetHeader2.name, pHeaderDescriptor->hdr[0].name, sizeof(m_RetHeader2.name));

		fflush(stderr);
		fflush(stdout);
		ret = system("cat /sys/kernel/debug/ipa/hdr");

		if (!m_HeaderInsertion.GetHeaderHandle(&m_RetHeader1))
		{
			LOG_MSG_ERROR(" Failed");
			bRetVal = false;
			goto bail;
		}
		LOG_MSG_DEBUG("Received Header1 Handle = 0x%x", m_RetHeader1.hdl);

		if (!m_HeaderInsertion.GetHeaderHandle(&m_RetHeader2))
		{
			LOG_MSG_ERROR(" Failed");
			bRetVal = false;
			goto bail;
		}
		LOG_MSG_DEBUG("Received Header2 Handle = 0x%x", m_RetHeader2.hdl);

		if (!CreateBypassRoutingTable(&m_Routing, m_eIP, aBypass1, IPA_CLIENT_TEST3_CONS,
				m_RetHeader1.hdl, &nTableHdl01)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			bRetVal = false;
			goto bail;
		}
		if (!CreateBypassRoutingTable(&m_Routing, m_eIP, aBypass2, IPA_CLIENT_TEST4_CONS,
				m_RetHeader2.hdl, &nTableHdl02)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			bRetVal = false;
			goto bail;
		}
		LOG_MSG_INFO("Creation of two bypass routing tables completed successfully TblHdl1=0x%x, TblHdl2=0x%x",
				nTableHdl01, nTableHdl02);

		// Creating Filtering Rules
		cFilterTable0.Init(m_eIP, IPA_CLIENT_TEST_PROD, false, 2);
		LOG_MSG_INFO("Creation of filtering table completed successfully");

		// Configuring common Filtering fields
		cFilterTable0.GeneratePresetRule(1, sFilterRuleEntry);
		sFilterRuleEntry.at_rear = true;
		sFilterRuleEntry.rule.action = IPA_PASS_TO_ROUTING;
		sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask

		// Configuring Filtering Rule No.1
		sFilterRuleEntry.flt_rule_hdl = -1; // return Value
		sFilterRuleEntry.status = -1; // return Value
		sFilterRuleEntry.rule.rt_tbl_hdl = nTableHdl01; //put here the handle corresponding to Routing Rule 1
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1.
		if ((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry))
		{
			LOG_MSG_ERROR ("Adding Rule(1) to Filtering table Failed.");
			bRetVal = false;
			goto bail;
		}

		// Configuring Filtering Rule No.2
		sFilterRuleEntry.flt_rule_hdl = -1; // return Value
		sFilterRuleEntry.status = -1; // return value
		sFilterRuleEntry.rule.rt_tbl_hdl = nTableHdl02; //put here the handle corresponding to Routing Rule 2
		sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.
		if ((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry))
		{
			LOG_MSG_ERROR ("Adding Rule(2) to Filtering table Failed.");
			bRetVal = false;
			goto bail;
		}

		if (!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())) {
			LOG_MSG_ERROR ("Failed to commit Filtering rules");
			bRetVal = false;
			goto bail;
		}

		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n",
			cFilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,
			cFilterTable0.ReadRuleFromTable(0)->status);
		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n",
			cFilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,
			cFilterTable0.ReadRuleFromTable(1)->status);

	bail:
		Free(pHeaderDescriptor);
		LOG_MSG_STACK("Leaving Function (Returning %s)", bRetVal ? "True" : "False");
		return bRetVal;
	} // AddRules()

	virtual bool ModifyPackets() {
		// This test doesn't modify the original IP Packet.
		return true;
	} // ModifyPacktes ()

	virtual bool TestLogic() {
		bool bRetVal = true;
		m_aExpectedBufSize = 0;
		uint32_t nIPv4DSTAddr;

		LOG_MSG_STACK("Entering Function");

		//Packet No. 1
		memset(m_aExpectedBuffer, 0, sizeof(m_aExpectedBuffer));
		nIPv4DSTAddr = ntohl(0xC0A80101); //192.168.1.1
		memcpy(&m_aBuffer[IPV4_DST_ADDR_OFFSET], &nIPv4DSTAddr, sizeof(nIPv4DSTAddr));
		memcpy(m_aExpectedBuffer, m_aHeadertoAdd1, m_nHeadertoAddSize1);
		memcpy(m_aExpectedBuffer+m_nHeadertoAddSize1,m_aBuffer,m_uBufferSize);
		m_aExpectedBufSize = m_nHeadertoAddSize1 + m_uBufferSize;
		if (!SendReceiveAndCompare(&m_producer, m_aBuffer, m_uBufferSize,
				&m_Consumer2, m_aExpectedBuffer, m_aExpectedBufSize))
		{
			LOG_MSG_ERROR("SendReceiveAndCompare failed.");
			bRetVal=false;
		}

		//Packet No. 2
		memset(m_aExpectedBuffer, 0, sizeof(m_aExpectedBuffer));
		nIPv4DSTAddr = ntohl(0xC0A80102); //192.168.1.2
		memcpy (&m_aBuffer[IPV4_DST_ADDR_OFFSET], &nIPv4DSTAddr, sizeof(nIPv4DSTAddr));
		memcpy(m_aExpectedBuffer, m_aHeadertoAdd2, m_nHeadertoAddSize2);
		memcpy(m_aExpectedBuffer+m_nHeadertoAddSize2, m_aBuffer, m_uBufferSize);
		m_aExpectedBufSize = m_nHeadertoAddSize2 + m_uBufferSize;
		if (!SendReceiveAndCompare(&m_producer, m_aBuffer, m_uBufferSize,
				&m_Consumer3, m_aExpectedBuffer, m_aExpectedBufSize))
		{
			LOG_MSG_ERROR("SendReceiveAndCompare failed.");
			bRetVal=false;
		}

		LOG_MSG_STACK("Leaving Function (Returning %s)",bRetVal?"True":"False");
		return bRetVal;
	}

protected:
	struct ipa_ioc_get_hdr m_RetHeader1, m_RetHeader2;
	int m_InitialHeadersNum;

private:
	uint8_t m_aExpectedBuffer[BUFF_MAX_SIZE]; // Input file / IP packet
	size_t m_aExpectedBufSize;
	uint8_t m_aHeadertoAdd1[MAX_HEADER_SIZE], m_aHeadertoAdd2[MAX_HEADER_SIZE];
	size_t m_nHeadertoAddSize1, m_nHeadertoAddSize2;
	int ret;
};

static IPAHeaderInsertionTest001 ipaHeaderInsertionTest001;
static IPAHeaderInsertionTest002 ipaHeaderInsertionTest002;
static IPAHeaderInsertionTest003 ipaHeaderInsertionTest003;
static IPAHeaderInsertionTest004 ipaHeaderInsertionTest004;
static IPAHeaderInsertionTest005 ipaHeaderInsertionTest005;
static IPAHeaderInsertionTest006 ipaHeaderInsertionTest006;
static IPAHeaderInsertionTest007 ipaHeaderInsertionTest007;
static IPAHeaderInsertionTest008 ipaHeaderInsertionTest008;
static IPAHeaderInsertionTest009 ipaHeaderInsertionTest009;
static IPAHeaderInsertionTest010 ipaHeaderInsertionTest010;

