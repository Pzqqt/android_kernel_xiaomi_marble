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

#include "FilteringEthernetBridgingTestFixture.h"

IpaFilteringEthernetBridgingTestFixture::IpaFilteringEthernetBridgingTestFixture():
	m_sendSize1 (m_BUFF_MAX_SIZE),
	m_sendSize2 (m_BUFF_MAX_SIZE),
	m_sendSize3 (m_BUFF_MAX_SIZE),
	m_IpaIPType(IPA_IP_v4)
{
	memset(m_sendBuffer1, 0, sizeof(m_sendBuffer1));
	memset(m_sendBuffer2, 0, sizeof(m_sendBuffer2));
	memset(m_sendBuffer3, 0, sizeof(m_sendBuffer3));
	m_testSuiteName.push_back("FilteringEth");
}

bool IpaFilteringEthernetBridgingTestFixture::Setup()
{
	ConfigureScenario(PHASE_TWO_TEST_CONFIGURATION);

	m_producer.Open(INTERFACE0_TO_IPA_DATA_PATH, INTERFACE0_FROM_IPA_DATA_PATH);
	m_producer2.Open(INTERFACE4_TO_IPA_DATA_PATH, INTERFACE4_FROM_IPA_DATA_PATH);

	m_consumer.Open(INTERFACE1_TO_IPA_DATA_PATH, INTERFACE1_FROM_IPA_DATA_PATH);
	m_consumer2.Open(INTERFACE2_TO_IPA_DATA_PATH, INTERFACE2_FROM_IPA_DATA_PATH);
	m_defaultConsumer.Open(INTERFACE3_TO_IPA_DATA_PATH, INTERFACE3_FROM_IPA_DATA_PATH);

	if (!m_routing.DeviceNodeIsOpened())
	{
		LOG_MSG_ERROR("Routing block is not ready for immediate commands!\n");
		return false;
	}

	if (!m_filtering.DeviceNodeIsOpened())
	{
		LOG_MSG_ERROR("Filtering block is not ready for immediate commands!\n");
		return false;
	}
	m_routing.Reset(IPA_IP_v4);
	m_routing.Reset(IPA_IP_v6);
	return true;
} // Setup()

bool IpaFilteringEthernetBridgingTestFixture::Teardown()
{
	m_producer.Close();
	m_producer2.Close();
	m_consumer.Close();
	m_consumer2.Close();
	m_defaultConsumer.Close();
	return true;
} // Teardown()

bool IpaFilteringEthernetBridgingTestFixture::LoadFiles(enum ipa_ip_type ip)
{
	if (!LoadDefaultEth2Packet(ip, m_sendBuffer1, m_sendSize1)) {
		LOG_MSG_ERROR("Failed default Packet\n");
		return false;
	}
	LOG_MSG_DEBUG ("Loaded %zu Bytes to Buffer 1\n",m_sendSize1);

	if (!LoadDefaultEth2Packet(ip, m_sendBuffer2, m_sendSize2)) {
		LOG_MSG_ERROR("Failed default Packet\n");
		return false;
	}
	LOG_MSG_DEBUG ("Loaded %zu Bytes to Buffer 2\n",m_sendSize2);

	if (!LoadDefaultEth2Packet(ip, m_sendBuffer3, m_sendSize3)) {
		LOG_MSG_ERROR("Failed default Packet\n");
		return false;
	}
	LOG_MSG_DEBUG ("Loaded %zu Bytes to Buffer 3\n",m_sendSize3);

	return true;
}

bool IpaFilteringEthernetBridgingTestFixture::ReceivePacketsAndCompare()
{
	size_t receivedSize = 0;
	size_t receivedSize2 = 0;
	size_t receivedSize3 = 0;
	bool isSuccess = true;

	// Receive results
	Byte *rxBuff1 = new Byte[m_BUFF_MAX_SIZE];
	Byte *rxBuff2 = new Byte[m_BUFF_MAX_SIZE];
	Byte *rxBuff3 = new Byte[m_BUFF_MAX_SIZE];

	if (NULL == rxBuff1 || NULL == rxBuff2 || NULL == rxBuff3)
	{
		printf("Memory allocation error.\n");
		return false;
	}

	receivedSize = m_consumer.ReceiveData(rxBuff1,
		m_BUFF_MAX_SIZE);
	LOG_MSG_DEBUG("Received %zu bytes on %s.\n",
		receivedSize,
		m_consumer.m_fromChannelName.c_str());

	receivedSize2 = m_consumer2.ReceiveData(rxBuff2,
		m_BUFF_MAX_SIZE);
	LOG_MSG_DEBUG("Received %zu bytes on %s.\n",
		receivedSize2,
		m_consumer2.m_fromChannelName.c_str());

	receivedSize3 = m_defaultConsumer.ReceiveData(rxBuff3,
		m_BUFF_MAX_SIZE);
	LOG_MSG_DEBUG("Received %zu bytes on %s.\n",
		receivedSize3,
		m_defaultConsumer.m_fromChannelName.c_str());

	// Compare results
	if (!CompareResultVsGolden(m_sendBuffer1,
		m_sendSize1,
		rxBuff1,
		receivedSize))
	{
		LOG_MSG_ERROR("Comparison of Buffer0 Failed!");
		isSuccess = false;
	}

	char recievedBuffer[256] = {0};
	char SentBuffer[256] = {0};

	size_t j;
	for(j = 0; j < m_sendSize1; j++)
		snprintf(&SentBuffer[3*j], sizeof(SentBuffer) - (3*j + 1), " %02X", m_sendBuffer1[j]);
	for(j = 0; j < receivedSize; j++)
		snprintf(&recievedBuffer[3*j], sizeof(recievedBuffer) - (3*j + 1), " %02X", rxBuff1[j]);
	printf("Expected Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n",
		m_sendSize1,SentBuffer,receivedSize,recievedBuffer);
	recievedBuffer[0] = 0;

	for(j = 0; j < m_sendSize2; j++)
		snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3*j + 1), " %02X", m_sendBuffer2[j]);
	for(j = 0; j < receivedSize2; j++)
		snprintf(&recievedBuffer[3*j], sizeof(recievedBuffer) - (3*j + 1), " %02X", rxBuff2[j]);
	printf("Expected Value2 (%zu)\n%s\n, Received Value2(%zu)\n%s\n",
		m_sendSize2,SentBuffer,receivedSize2,recievedBuffer);
	recievedBuffer[0] = 0;

	for(j = 0; j < m_sendSize3; j++)
		snprintf(&SentBuffer[3*j], sizeof(SentBuffer) - (3*j + 1), " %02X", m_sendBuffer3[j]);
	for(j = 0; j < receivedSize3; j++)
		snprintf(&recievedBuffer[3*j], sizeof(recievedBuffer) - (3*j + 1), " %02X", rxBuff3[j]);
	printf("Expected Value3 (%zu)\n%s\n, Received Value3(%zu)\n%s\n",
		m_sendSize3,SentBuffer,receivedSize3,recievedBuffer);
	recievedBuffer[0] = 0;

	isSuccess &= CompareResultVsGolden(m_sendBuffer2,
		m_sendSize2, rxBuff2, receivedSize2);
	isSuccess &= CompareResultVsGolden(m_sendBuffer3,
		m_sendSize3, rxBuff3, receivedSize3);

	delete[] rxBuff1;
	delete[] rxBuff2;
	delete[] rxBuff3;

	return isSuccess;
}

// This function creates three IPv4 bypass routing entries and commits them.
bool IpaFilteringEthernetBridgingTestFixture::CreateThreeIPv4BypassRoutingTables(const char *bypass0, const char *bypass1,
	const char *bypass2)
{
	LOG_MSG_DEBUG("Entering");
	struct ipa_ioc_add_rt_rule *rt_rule0 = 0, *rt_rule1 = 0,*rt_rule2 = 0;
	struct ipa_rt_rule_add *rt_rule_entry;

	rt_rule0 = (struct ipa_ioc_add_rt_rule *)
		calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			1*sizeof(struct ipa_rt_rule_add));
	if(!rt_rule0) {
		LOG_MSG_ERROR("calloc failed to allocate rt_rule0");
		return false;
	}
	rt_rule1 = (struct ipa_ioc_add_rt_rule *)
		calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			1*sizeof(struct ipa_rt_rule_add));
	if(!rt_rule1) {
		LOG_MSG_ERROR("calloc failed to allocate rt_rule1");
		Free(rt_rule0);
		return false;
	}
	rt_rule2 = (struct ipa_ioc_add_rt_rule *)
		calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			1*sizeof(struct ipa_rt_rule_add));
	if(!rt_rule2) {
		LOG_MSG_ERROR("calloc failed to allocate rt_rule2");
		Free(rt_rule0);
		Free(rt_rule1);
		return false;
	}

	rt_rule0->num_rules = 1;
	rt_rule0->ip = IPA_IP_v4;
	rt_rule0->commit = true;
	strlcpy(rt_rule0->rt_tbl_name, bypass0, sizeof(rt_rule0->rt_tbl_name));

	rt_rule_entry = &rt_rule0->rules[0];
	rt_rule_entry->at_rear = 0;
	rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;

	rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
	rt_rule_entry->rule.attrib.u.v4.dst_addr = 0xaabbccdd;
	rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0x00000000;// All Packets will get a "Hit"
	if (false == m_routing.AddRoutingRule(rt_rule0))
	{
		LOG_MSG_ERROR("Routing rule addition(rt_rule0) failed!\n");
		Free (rt_rule2);
		Free (rt_rule1);
		Free (rt_rule0);
		return false;
	}

	rt_rule1->num_rules = 1;
	rt_rule1->ip = IPA_IP_v4;
	rt_rule1->commit = true;
	strlcpy(rt_rule1->rt_tbl_name, bypass1, sizeof(rt_rule1->rt_tbl_name));
	rt_rule_entry = &rt_rule1->rules[0];
	rt_rule_entry->at_rear = 0;
	rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
	rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
	rt_rule_entry->rule.attrib.u.v4.dst_addr = 0xaabbccdd;
	rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0x00000000;// All Packets will get a "Hit"
	if (false == m_routing.AddRoutingRule(rt_rule1))
	{
		LOG_MSG_ERROR("Routing rule addition(rt_rule1) failed!\n");
		Free (rt_rule2);
		Free (rt_rule1);
		Free (rt_rule0);
		return false;
	}


	rt_rule2->num_rules = 1;
	rt_rule2->ip = IPA_IP_v4;
	rt_rule2->commit = true;
	strlcpy(rt_rule2->rt_tbl_name, bypass2, sizeof(rt_rule2->rt_tbl_name));
	rt_rule_entry = &rt_rule2->rules[0];
	rt_rule_entry->at_rear = 0;
	rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;
	rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
	rt_rule_entry->rule.attrib.u.v4.dst_addr = 0xaabbccdd;
	rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0x00000000;// All Packets will get a "Hit"
	if (false == m_routing.AddRoutingRule(rt_rule2))
	{
		LOG_MSG_ERROR("Routing rule addition(rt_rule2)\n");
		Free (rt_rule2);
		Free (rt_rule1);
		Free (rt_rule0);
		return false;
	}

	Free (rt_rule2);
	Free (rt_rule1);
	Free (rt_rule0);
	LOG_MSG_DEBUG("Leaving");
	return true;
}

// This function creates three IPv6 bypass routing entries and commits them.
bool IpaFilteringEthernetBridgingTestFixture::CreateThreeIPv6BypassRoutingTables (const char *bypass0, const char *bypass1,
	const char *bypass2)
{
	LOG_MSG_DEBUG("Entering");
	struct ipa_ioc_add_rt_rule *rt_rule0 = 0, *rt_rule1 = 0,
		*rt_rule2 = 0;
	struct ipa_rt_rule_add *rt_rule_entry;

	rt_rule0 = (struct ipa_ioc_add_rt_rule *)
		calloc(1,sizeof(struct ipa_ioc_add_rt_rule) +
			1*sizeof(struct ipa_rt_rule_add));
	if(!rt_rule0) {
		LOG_MSG_ERROR("calloc failed to allocate rt_rule0\n");
		return false;
	}
	rt_rule1 = (struct ipa_ioc_add_rt_rule *)
		calloc(1,sizeof(struct ipa_ioc_add_rt_rule) +
			1*sizeof(struct ipa_rt_rule_add));
	if(!rt_rule1) {
		LOG_MSG_ERROR("calloc failed to allocate rt_rule1\n");
		Free(rt_rule0);
		return false;
	}
	rt_rule2 = (struct ipa_ioc_add_rt_rule *)
		calloc(1,sizeof(struct ipa_ioc_add_rt_rule) +
			1*sizeof(struct ipa_rt_rule_add));
	if(!rt_rule2) {
		LOG_MSG_ERROR("calloc failed to allocate rt_rule2\n");
		Free(rt_rule0);
		Free(rt_rule1);
		return false;
	}

	rt_rule0->num_rules = 1;
	rt_rule0->ip = IPA_IP_v6;
	rt_rule0->commit = true;
	strlcpy(rt_rule0->rt_tbl_name, bypass0, sizeof(rt_rule0->rt_tbl_name));

	rt_rule_entry = &rt_rule0->rules[0];
	rt_rule_entry->at_rear = 0;
	rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
	rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
	rt_rule_entry->rule.attrib.u.v6.dst_addr[0] = 0xaabbccdd;
	rt_rule_entry->rule.attrib.u.v6.dst_addr[1] = 0xeeff0011;
	rt_rule_entry->rule.attrib.u.v6.dst_addr[2] = 0x22334455;
	rt_rule_entry->rule.attrib.u.v6.dst_addr[3] = 0x66778899;
	rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0x00000000;// All Packets will get a "Hit"
	rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0x00000000;
	rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;
	rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0x00000000;
	if (false == m_routing.AddRoutingRule(rt_rule0))
	{
		LOG_MSG_ERROR("Routing rule addition(rt_rule0)");
		Free (rt_rule2);
		Free (rt_rule1);
		Free (rt_rule0);
		return false;
	}

	rt_rule1->num_rules = 1;
	rt_rule1->ip = IPA_IP_v6;
	rt_rule1->commit = true;
	strlcpy(rt_rule1->rt_tbl_name, bypass1, sizeof(rt_rule1->rt_tbl_name));
	rt_rule_entry = &rt_rule1->rules[0];
	rt_rule_entry->at_rear = 0;
	rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
	rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
	rt_rule_entry->rule.attrib.u.v6.dst_addr[0] = 0xaabbccdd;
	rt_rule_entry->rule.attrib.u.v6.dst_addr[1] = 0xeeff0011;
	rt_rule_entry->rule.attrib.u.v6.dst_addr[2] = 0x22334455;
	rt_rule_entry->rule.attrib.u.v6.dst_addr[3] = 0x66778899;
	rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0x00000000;// All Packets will get a "Hit"
	rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0x00000000;
	rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;
	rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0x00000000;
	if (false == m_routing.AddRoutingRule(rt_rule1))
	{
		LOG_MSG_ERROR("Routing rule addition(rt_rule1)");
		Free (rt_rule2);
		Free (rt_rule1);
		Free (rt_rule0);
		return false;
	}

	rt_rule2->num_rules = 1;
	rt_rule2->ip = IPA_IP_v6;
	rt_rule2->commit = true;
	strlcpy(rt_rule2->rt_tbl_name, bypass2, sizeof(rt_rule2->rt_tbl_name));
	rt_rule_entry = &rt_rule2->rules[0];
	rt_rule_entry->at_rear = 0;
	rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;
	rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
	rt_rule_entry->rule.attrib.u.v6.dst_addr[0] = 0xaabbccdd;
	rt_rule_entry->rule.attrib.u.v6.dst_addr[1] = 0xeeff0011;
	rt_rule_entry->rule.attrib.u.v6.dst_addr[2] = 0x22334455;
	rt_rule_entry->rule.attrib.u.v6.dst_addr[3] = 0x66778899;
	rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0x00000000;// All Packets will get a "Hit"
	rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0x00000000;
	rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;
	rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0x00000000;
	if (false == m_routing.AddRoutingRule(rt_rule2))
	{
		LOG_MSG_ERROR("Routing rule addition(rt_rule2)");
		Free (rt_rule2);
		Free (rt_rule1);
		Free (rt_rule0);
		return false;
	}

	Free (rt_rule2);
	Free (rt_rule1);
	Free (rt_rule0);
	LOG_MSG_DEBUG("Leaving function\n");
	return true;
}

bool IpaFilteringEthernetBridgingTestFixture::Run()
{
	bool res = false;
	bool isSuccess = false;

	LOG_MSG_DEBUG("Entering");

	// Add the relevant filtering rules
	res = AddRules();
	if (false == res) {
		LOG_MSG_ERROR("Failed adding filtering rules");
		return false;
	}

	// Load input data (IP packet) from file
	res = LoadFiles(m_IpaIPType);
	if (false == res) {
		LOG_MSG_ERROR("Failed loading files");
		return false;
	}

	res = ModifyPackets();
	if (false == res) {
		LOG_MSG_ERROR("Failed to modify packets");
		return false;
	}

	// Send first packet
	isSuccess = m_pCurrentProducer->SendData(m_sendBuffer1,
		m_sendSize1);
	if (false == isSuccess)
	{
		LOG_MSG_ERROR("SendData failure");
		return false;
	}

	// Send second packet
	isSuccess = m_pCurrentProducer->SendData(m_sendBuffer2,
		m_sendSize2);
	if (false == isSuccess)
	{
		LOG_MSG_ERROR("SendData failure");
		return false;
	}

	// Send third packet
	isSuccess = m_pCurrentProducer->SendData(m_sendBuffer3,
		m_sendSize3);
	if (false == isSuccess)
	{
		LOG_MSG_ERROR("SendData failure");
		return false;
	}

	// Receive packets from the channels and compare results
	isSuccess = ReceivePacketsAndCompare();

	LOG_MSG_DEBUG("Leaving function returning %d", isSuccess);

	return isSuccess;
} // Run()


IpaFilteringEthernetBridgingTestFixture::~IpaFilteringEthernetBridgingTestFixture()
{
	m_sendSize1 = 0;
	m_sendSize2 = 0;
	m_sendSize3 = 0;
}

static const size_t m_BUFF_MAX_SIZE = 1024;
static Filtering m_filtering;
static RoutingDriverWrapper m_routing;

InterfaceAbstraction m_producer;
InterfaceAbstraction m_producer2; // Pipe with ETH2 header removal
InterfaceAbstraction *m_pCurrentProducer;
InterfaceAbstraction m_consumer;
InterfaceAbstraction m_consumer2;
InterfaceAbstraction m_defaultConsumer;

Byte m_sendBuffer1[m_BUFF_MAX_SIZE];
Byte m_sendBuffer2[m_BUFF_MAX_SIZE];
Byte m_sendBuffer3[m_BUFF_MAX_SIZE];
size_t m_sendSize1;
size_t m_sendSize2;
size_t m_sendSize3;
enum ipa_ip_type m_IpaIPType;

RoutingDriverWrapper IpaFilteringEthernetBridgingTestFixture::m_routing;
Filtering IpaFilteringEthernetBridgingTestFixture::m_filtering;

const uint8_t IpaFilteringEthernetBridgingTestFixture::m_ETH2_DST_ADDR[ETH_ALEN] =
{
	0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0x11
};

const uint8_t IpaFilteringEthernetBridgingTestFixture::m_ETH2_SRC_ADDR[ETH_ALEN] =
{
	0x22, 0xee, 0xdd, 0xcc, 0xbb, 0xaa
};

const uint8_t IpaFilteringEthernetBridgingTestFixture::m_MAC_ADDR_MASK_ALL[ETH_ALEN] =
{
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

