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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "hton.h" // for htonl


#include "InterfaceAbstraction.h"
#include "Constants.h"
#include "Logger.h"
#include "TestsUtils.h"
#include "linux/msm_ipa.h"
#include "RoutingDriverWrapper.h"
#include "Filtering.h"
#include "IPAFilteringTable.h"

#define TOS_FIELD_OFFSET (1)
#define DST_ADDR_LSB_OFFSET_IPV4 (19)
#define SRC_ADDR_LSB_OFFSET_IPV4 (15)
#define DST_ADDR_MSB_OFFSET_IPV6 (24)
#define DST_ADDR_LSB_OFFSET_IPV6 (39)
#define TRAFFIC_CLASS_MSB_OFFSET_IPV6 (0)
#define TRAFFIC_CLASS_LSB_OFFSET_IPV6 (1)
#define FLOW_CLASS_MSB_OFFSET_IPV6 (1)
#define FLOW_CLASS_MB_OFFSET_IPV6 (2)
#define FLOW_CLASS_LSB_OFFSET_IPV6 (3)
#define IPV4_DST_PORT_OFFSET (20+2)
#define IPV6_SRC_PORT_OFFSET (40)
#define IPV6_DST_PORT_OFFSET (40+2)
#define IPv4_TCP_FLAGS_OFFSET (20+13)
#define IPv6_TCP_FLAGS_OFFSET (40+13)

#define TCP_ACK_FLAG_MASK (0x10)

extern Logger g_Logger;

class IpaRoutingBlockTestFixture:public TestBase
{
public:
	IpaRoutingBlockTestFixture():
		m_sendSize (BUFF_MAX_SIZE),
		m_sendSize2 (BUFF_MAX_SIZE),
		m_sendSize3 (BUFF_MAX_SIZE),
		m_IpaIPType(IPA_IP_v4)
	{
		memset(m_sendBuffer, 0, sizeof(m_sendBuffer));
		memset(m_sendBuffer2, 0, sizeof(m_sendBuffer2));
		memset(m_sendBuffer3, 0, sizeof(m_sendBuffer3));
		m_testSuiteName.push_back("Routing");
	}

	static int SetupKernelModule(bool en_status = false)
	{
		int retval;
		struct ipa_channel_config from_ipa_channels[3];
		struct test_ipa_ep_cfg from_ipa_cfg[3];
		struct ipa_channel_config to_ipa_channels[1];
		struct test_ipa_ep_cfg to_ipa_cfg[1];

		struct ipa_test_config_header header = {0};
		struct ipa_channel_config *to_ipa_array[1];
		struct ipa_channel_config *from_ipa_array[3];

		/* From ipa configurations - 3 pipes */
		memset(&from_ipa_cfg[0], 0, sizeof(from_ipa_cfg[0]));
		prepare_channel_struct(&from_ipa_channels[0],
				header.from_ipa_channels_num++,
				IPA_CLIENT_TEST2_CONS,
				(void *)&from_ipa_cfg[0],
				sizeof(from_ipa_cfg[0]), en_status);
		from_ipa_array[0] = &from_ipa_channels[0];

		memset(&from_ipa_cfg[1], 0, sizeof(from_ipa_cfg[1]));
		prepare_channel_struct(&from_ipa_channels[1],
				header.from_ipa_channels_num++,
				IPA_CLIENT_TEST3_CONS,
				(void *)&from_ipa_cfg[1],
				sizeof(from_ipa_cfg[1]), en_status);
		from_ipa_array[1] = &from_ipa_channels[1];

		memset(&from_ipa_cfg[2], 0, sizeof(from_ipa_cfg[2]));
		prepare_channel_struct(&from_ipa_channels[2],
				header.from_ipa_channels_num++,
				IPA_CLIENT_TEST4_CONS,
				(void *)&from_ipa_cfg[2],
				sizeof(from_ipa_cfg[2]), en_status);
		from_ipa_array[2] = &from_ipa_channels[2];

		/* To ipa configurations - 1 pipes */
		memset(&to_ipa_cfg[0], 0, sizeof(to_ipa_cfg[0]));
		prepare_channel_struct(&to_ipa_channels[0],
				header.to_ipa_channels_num++,
				IPA_CLIENT_TEST_PROD,
				(void *)&to_ipa_cfg[0],
				sizeof(to_ipa_cfg[0]));
		to_ipa_array[0] = &to_ipa_channels[0];

		prepare_header_struct(&header, from_ipa_array, to_ipa_array);

		retval = GenericConfigureScenario(&header);

		return retval;
	}

	bool Setup(bool en_status)
	{
		bool bRetVal = true;

		bRetVal = SetupKernelModule(en_status);
		if (bRetVal != true) {
			return bRetVal;
		}
		m_producer.Open(INTERFACE0_TO_IPA_DATA_PATH, INTERFACE0_FROM_IPA_DATA_PATH);

		m_consumer.Open(INTERFACE1_TO_IPA_DATA_PATH, INTERFACE1_FROM_IPA_DATA_PATH);
		m_consumer2.Open(INTERFACE2_TO_IPA_DATA_PATH, INTERFACE2_FROM_IPA_DATA_PATH);
		m_defaultConsumer.Open(INTERFACE3_TO_IPA_DATA_PATH, INTERFACE3_FROM_IPA_DATA_PATH);

		if (!m_routing.DeviceNodeIsOpened()) {
			printf("Routing block is not ready for immediate commands!\n");
			return false;
		}

		if (!m_filtering.DeviceNodeIsOpened()) {
			printf("Filtering block is not ready for immediate commands!\n");
			return false;
		}
		m_routing.Reset(IPA_IP_v4);
		m_routing.Reset(IPA_IP_v6);

		return true;
	} /* Setup()*/

	bool Setup()
	{
		return Setup(false);
	}

	bool Teardown()
	{
		if (!m_routing.DeviceNodeIsOpened()) {
			printf("Routing block is not ready for immediate commands!\n");
			return false;
		}
		if (!m_filtering.DeviceNodeIsOpened()) {
			printf("Filtering block is not ready for immediate commands!\n");
			return false;
		}

		m_producer.Close();
		m_consumer.Close();
		m_consumer2.Close();
		m_defaultConsumer.Close();

		return true;
	} /* Teardown() */

	bool LoadFiles(enum ipa_ip_type ip)
	{
		string fileName;

		if (IPA_IP_v4 == ip) {
			fileName = "Input/IPv4_1";
		} else {
			fileName = "Input/IPv6";
		}

		if (!LoadDefaultPacket(ip, m_sendBuffer, m_sendSize)) {
			LOG_MSG_ERROR("Failed loading default Packet");
			return false;
		}

		if (!LoadDefaultPacket(ip, m_sendBuffer2, m_sendSize2)) {
			LOG_MSG_ERROR("Failed loading default Packet");
			return false;
		}

		if (!LoadDefaultPacket(ip, m_sendBuffer3, m_sendSize3)) {
			LOG_MSG_ERROR("Failed loading default Packet");
			return false;
		}

		return true;
	}

	bool ReceivePacketAndCompareFrom(InterfaceAbstraction& cons, Byte* send,
									 size_t send_sz, bool shouldBeHit)
	{
		size_t receivedSize = 0;
		bool isSuccess = true;

		/* Receive results*/
		Byte *rxBuff1 = new Byte[0x400];

		if (NULL == rxBuff1)
		{
			printf("Memory allocation error.\n");
			return false;
		}

		receivedSize = cons.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, cons.m_fromChannelName.c_str());

		// Compare results
		isSuccess &= CompareResultVsGolden_w_Status(send,  send_sz,  rxBuff1, receivedSize);

		if (shouldBeHit) {
			isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
				IsCacheHit_v5_0(send_sz, receivedSize, rxBuff1) : IsCacheHit(send_sz, receivedSize, rxBuff1);
		}
		else
		{
			isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
				IsCacheMiss_v5_0(send_sz, receivedSize, rxBuff1) : IsCacheMiss(send_sz, receivedSize, rxBuff1);
		}

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];
//		char * p = recievedBuffer;
		size_t j;
		for(j = 0; j < m_sendSize; j++)
			snprintf(&sentBuffer[3 * j], sentBufferSize - (3 * j + 1), " %02X", send[j]);
		for(j = 0; j < receivedSize; j++)
//			recievedBuffer += sprintf(recievedBuffer, "%02X", rxBuff1[i]);
		    snprintf(&recievedBuffer[3 * j], recievedBufferSize - (3 * j + 1), " %02X", rxBuff1[j]);
		printf("Expected Value (%zu)\n%s\n, Received Value1(%zu)\n%s\n",send_sz,sentBuffer,receivedSize,recievedBuffer);

		delete[] rxBuff1;
		delete[] recievedBuffer;
		delete[] sentBuffer;

		return isSuccess;
	}

	bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		size_t receivedSize3 = 0;
		bool pkt1_cmp_succ, pkt2_cmp_succ, pkt3_cmp_succ;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];
		Byte *rxBuff3 = new Byte[0x400];

		if (NULL == rxBuff1 || NULL == rxBuff2 || NULL == rxBuff3)
		{
			printf("Memory allocation error.\n");
			return false;
		}

		memset(rxBuff1, 0, 0x400);
		memset(rxBuff2, 0, 0x400);
		memset(rxBuff3, 0, 0x400);

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		receivedSize2 = m_consumer2.ReceiveData(rxBuff2, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize2, m_consumer2.m_fromChannelName.c_str());

		receivedSize3 = m_defaultConsumer.ReceiveData(rxBuff3, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize3, m_defaultConsumer.m_fromChannelName.c_str());

		/* Compare results */
		pkt1_cmp_succ = CompareResultVsGolden(m_sendBuffer,  m_sendSize,  rxBuff1, receivedSize);
		pkt2_cmp_succ = CompareResultVsGolden(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		pkt3_cmp_succ = CompareResultVsGolden(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		size_t recievedBufferSize =
			MAX3(receivedSize, receivedSize2, receivedSize3) * 3;
		size_t sentBufferSize =
			MAX3(m_sendSize, m_sendSize2, m_sendSize3) * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		if (NULL == recievedBuffer || NULL == sentBuffer) {
			printf("Memory allocation error\n");
			return false;
		}

		size_t j;
		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);
		for(j = 0; j < m_sendSize; j++)
		    snprintf(&sentBuffer[3 * j], sentBufferSize - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for(j = 0; j < receivedSize; j++)
		    snprintf(&recievedBuffer[3 * j], recievedBufferSize - (3 * j + 1), " %02X", rxBuff1[j]);
		printf("Expected Value1(%zu)\n%s\n, Received Value1(%zu)\n%s\n-->Value1 %s\n",
			m_sendSize,sentBuffer,receivedSize,recievedBuffer,
			pkt1_cmp_succ?"Match":"no Match");

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);
		for(j = 0; j < m_sendSize2; j++)
		    snprintf(&sentBuffer[3 * j], sentBufferSize - (3 * j + 1), " %02X", m_sendBuffer2[j]);
		for(j = 0; j < receivedSize2; j++)
		    snprintf(&recievedBuffer[3 * j], recievedBufferSize - (3 * j + 1), " %02X", rxBuff2[j]);
		printf("Expected Value2 (%zu)\n%s\n, Received Value2(%zu)\n%s\n-->Value2 %s\n",
			m_sendSize2,sentBuffer,receivedSize2,recievedBuffer,
			pkt2_cmp_succ?"Match":"no Match");

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);
		for(j = 0; j < m_sendSize3; j++)
		    snprintf(&sentBuffer[3 * j], sentBufferSize - (3 * j + 1), " %02X", m_sendBuffer3[j]);
		for(j = 0; j < receivedSize3; j++)
		    snprintf(&recievedBuffer[3 * j], recievedBufferSize - (3 * j + 1), " %02X", rxBuff3[j]);
		printf("Expected Value3 (%zu)\n%s\n, Received Value3(%zu)\n%s\n-->Value3 %s\n",
			m_sendSize3,sentBuffer,receivedSize3,recievedBuffer,
			pkt3_cmp_succ?"Match":"no Match");

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return pkt1_cmp_succ && pkt2_cmp_succ && pkt3_cmp_succ;
	}

	void print_packets(size_t receivedSize, size_t m_sendSize, size_t recievedBufferSize, size_t sentBufferSize, Byte *rxBuff, Byte *m_sendBuffer,  char *recievedBuffer, char *sentBuffer)
	{
		size_t j;

		for(j = 0; j < m_sendSize; j++) {
			snprintf(&sentBuffer[3 * j], sentBufferSize - 3 * j,
				" %02X", m_sendBuffer[j]);
		}
		for(j = 0; j < receivedSize; j++) {
			snprintf(&recievedBuffer[3 * j], recievedBufferSize- 3 * j,
				" %02X", rxBuff[j]);
		}
		printf("Expected Value (%zu)\n%s\n, Received Value(%zu)\n%s\n",m_sendSize,sentBuffer,receivedSize,recievedBuffer);
	}

	~IpaRoutingBlockTestFixture()
	{
		m_sendSize = 0;
		m_sendSize2 = 0;
		m_sendSize3 = 0;
	}

	void InitFilteringBlock()
	{
		IPAFilteringTable fltTable;
		struct ipa_ioc_get_rt_tbl st_rt_tbl;
		struct ipa_flt_rule_add flt_rule_entry;

		memset(&st_rt_tbl, 0, sizeof(st_rt_tbl));
		memset(&flt_rule_entry, 0, sizeof(flt_rule_entry));
		strlcpy(st_rt_tbl.name, "LAN", sizeof(st_rt_tbl.name));
		st_rt_tbl.ip = m_IpaIPType;
		fltTable.Init(m_IpaIPType, IPA_CLIENT_TEST_PROD, false, 1);
		m_routing.GetRoutingTable(&st_rt_tbl);
		flt_rule_entry.rule.rt_tbl_hdl = st_rt_tbl.hdl;
		fltTable.AddRuleToTable(flt_rule_entry);
		m_filtering.AddFilteringRule(fltTable.GetFilteringTable());
	}

	inline bool VerifyStatusReceived(size_t SendSize, size_t RecvSize)
	{
		size_t stts_size = sizeof(struct ipa3_hw_pkt_status);

		if (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) {
			stts_size = sizeof(struct ipa3_hw_pkt_status_hw_v5_0);
		}

		if ((RecvSize <= SendSize) ||
			((RecvSize - SendSize) != stts_size)){
			printf("received buffer size does not match! sent:receive [%zu]:[%zu]\n",SendSize,RecvSize);
			return false;
		}

		return true;
	}

	inline bool IsCacheHit(size_t SendSize, size_t RecvSize, void *Buff)
	{
		struct ipa3_hw_pkt_status *pStatus = (struct ipa3_hw_pkt_status *)Buff;

		if (VerifyStatusReceived(SendSize,RecvSize) == false){
			return false;
		}

		if((bool)pStatus->route_hash){
			printf ("%s::cache hit!! \n",__FUNCTION__);
			return true;
		}

		printf ("%s::cache miss!! \n",__FUNCTION__);
		return false;

	}

	inline bool IsCacheHit_v5_0(size_t SendSize, size_t RecvSize, void *Buff)
	{
		struct ipa3_hw_pkt_status_hw_v5_0 *pStatus = (struct ipa3_hw_pkt_status_hw_v5_0 *)Buff;

		if (VerifyStatusReceived(SendSize,RecvSize) == false){
			return false;
		}

		if((bool)pStatus->route_hash){
			printf ("%s::cache hit!! \n",__FUNCTION__);
			return true;
		}

		printf ("%s::cache miss!! \n",__FUNCTION__);
		return false;

	}

	inline bool IsCacheMiss(size_t SendSize, size_t RecvSize, void *Buff)
	{
		struct ipa3_hw_pkt_status *pStatus = (struct ipa3_hw_pkt_status *)Buff;

		if (VerifyStatusReceived(SendSize,RecvSize) == false){
			return false;
		}

		if(!((bool)pStatus->route_hash)){
			printf ("%s::cache miss!! \n",__FUNCTION__);
			return true;
		}

		printf ("%s::cache hit!! \n",__FUNCTION__);
		return false;
	}

		inline bool IsCacheMiss_v5_0(size_t SendSize, size_t RecvSize, void *Buff)
	{
		struct ipa3_hw_pkt_status_hw_v5_0 *pStatus = (struct ipa3_hw_pkt_status_hw_v5_0 *)Buff;

		if (VerifyStatusReceived(SendSize,RecvSize) == false){
			return false;
		}

		if(!((bool)pStatus->route_hash)){
			printf ("%s::cache miss!! \n",__FUNCTION__);
			return true;
		}

		printf ("%s::cache hit!! \n",__FUNCTION__);
		return false;
	}

	static RoutingDriverWrapper m_routing;
	static Filtering m_filtering;

	static const size_t BUFF_MAX_SIZE = 1024;

	InterfaceAbstraction m_producer;
	InterfaceAbstraction m_consumer;
	InterfaceAbstraction m_consumer2;
	InterfaceAbstraction m_defaultConsumer;
	Byte m_sendBuffer[BUFF_MAX_SIZE];	// First input file / IP packet
	Byte m_sendBuffer2[BUFF_MAX_SIZE];	// Second input file / IP packet
	Byte m_sendBuffer3[BUFF_MAX_SIZE];	// Third input file (default) / IP packet
	size_t m_sendSize;
	size_t m_sendSize2;
	size_t m_sendSize3;
	enum ipa_ip_type m_IpaIPType;


private:
};

RoutingDriverWrapper IpaRoutingBlockTestFixture::m_routing;
Filtering IpaRoutingBlockTestFixture::m_filtering;

/*---------------------------------------------------------------------------*/
/* Test1: Tests routing by destination address							     */
/*---------------------------------------------------------------------------*/
class IpaRoutingBlockTest1 : public IpaRoutingBlockTestFixture
{
public:
	IpaRoutingBlockTest1()
	{
		m_name = "IpaRoutingBlockTest1";
		m_description =" \
		Routing block test 001 - Destination address exact match\1. Generate and commit a single routing tables. \
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP == (192.169.2.170 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP == (192.168.2.255 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v4;
		Register(*this);
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("Failed memory allocation for rt_rule\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802FF;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AA;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}
};

/*---------------------------------------------------------------------------*/
/* Test2: Tests routing by destination address with a subnet (mask) */
/*---------------------------------------------------------------------------*/
class IpaRoutingBlockTest2 : IpaRoutingBlockTestFixture
{
public:
	IpaRoutingBlockTest2()
	{
		m_name = "IpaRoutingBlockTest2";
		m_description =" \
		Routing block test 002 - Destination address subnet match \
		1. Generate and commit a single routing tables. \
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP == (192.169.170.0 & 255.255.255.0)traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP == (192.168.255.0 & 255.255.255.0)traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v4;
		Register(*this);
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		printf("ENTRY: IpaRoutingBlockTest2::Run()\n");

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[18] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[18] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		printf("ENTRY: IpaRoutingBlockTest2::AddRules()\n");

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A8FF00;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFF00;

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A8AA00;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFF00;

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		printf("Before calling m_routing.AddRoutingRule()\n");
		printf("m_routing = %p\n", &m_routing);

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}
};


/*---------------------------------------------------------------------------*/
/* Test3: Tests routing by TOS (Type Of Service) */
/*---------------------------------------------------------------------------*/
class IpaRoutingBlockTest3 : IpaRoutingBlockTestFixture
{
public:
	IpaRoutingBlockTest3()
	{
		m_name = "IpaRoutingBlockTest3";
		m_description = " \
		Routing block test 003 - TOS exact match\
		1. Generate and commit a single routing tables. \
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All TOS == 0xBF traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All TOS == 0x3A traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v4;
		Register(*this);
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[TOS_FIELD_OFFSET] = 0xBF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[TOS_FIELD_OFFSET] = 0x3A;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if (!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl;
		// gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_TOS;
		rt_rule_entry->rule.attrib.u.v4.tos = 0xBF;

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_TOS;
		rt_rule_entry->rule.attrib.u.v4.tos = 0x3A;

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}
};

/*---------------------------------------------------------------------------*/
/* Test4: Destination address exact match and TOS exact match */
/*---------------------------------------------------------------------------*/
class IpaRoutingBlockTest4 : IpaRoutingBlockTestFixture
{
public:
	IpaRoutingBlockTest4()
	{
		m_name = "IpaRoutingBlockTest4";
		m_description =" \
		Routing block test 004 - Source and Destination address and TOS exact match \
		1. Generate and commit a single routing tables. \
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP == (192.169.2.255 & 255.255.255.255) and TOS == 0xFF traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP == (192.168.2.170 & 255.255.255.255) and TOS == 0xAA traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All DST_IP == (192.168.2.85 & 255.255.255.255) and SRC_IP == (192.168.2.241 & 255.255.255.255) TOS == 0x24 traffic goes to pipe IPA_CLIENT_TEST3_CONS";
		m_IpaIPType = IPA_IP_v4;
		Register(*this);
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[TOS_FIELD_OFFSET] = 0xFF;
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[TOS_FIELD_OFFSET] = 0xAA;
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		m_sendBuffer3[TOS_FIELD_OFFSET] = 0x24;
		m_sendBuffer3[DST_ADDR_LSB_OFFSET_IPV4] = 0x55;
		m_sendBuffer3[SRC_ADDR_LSB_OFFSET_IPV4] = 0xF1;
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_TOS | IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.tos = 0xFF;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802FF;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;


		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_TOS | IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.tos = 0xAA;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AA;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_TOS | IPA_FLT_DST_ADDR | IPA_FLT_SRC_ADDR;
		rt_rule_entry->rule.attrib.u.v4.tos = 0x24;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A80255;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v4.src_addr      = 0xC0A802F1;
		rt_rule_entry->rule.attrib.u.v4.src_addr_mask = 0xFFFFFFFF;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}
};

/*---------------------------------------------------------------------------*/
/* Test5: IPv6 - Tests routing by destination address */
/*---------------------------------------------------------------------------*/
class IpaRoutingBlockTest5 : public IpaRoutingBlockTestFixture
{
public:
	IpaRoutingBlockTest5()
	{
		m_name = "IpaRoutingBlockTest5";
		m_description =" \
		Routing block test 005 - IPv6 Destination address exact match \
		1. Generate and commit a single routing tables. \
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000FF \
		traffic goes to pipe IPA_CLIENT_TEST2_CONS \
		All DST_IP ==	0XFF020000 \
						0x00000000 \
						0x00000000 \
						0X000000FF \
		traffic goes to pipe IPA_CLIENT_TEST3_CONS\
		All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		Register(*this);
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X000000FF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X000000AA;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}
};

/*---------------------------------------------------------------------------*/
/* Test6: IPv6 - Tests routing by destination address */
/*---------------------------------------------------------------------------*/
class IpaRoutingBlockTest006 : public IpaRoutingBlockTestFixture
{
public:
	IpaRoutingBlockTest006()
	{
		m_name = "IpaRoutingBlockTest006";
		m_description =" \
		Routing block test 006 - IPv6 Destination address Subnet match \
		1. Generate and commit a single routing tables. \
		2. Generate and commit Three routing rules: (DST & Mask Match 0xFFFFFFFF,0xFFFFFFFF,0x00000000,0x0000000). \
			All DST_IP ==	0X11020000 \
							0x00000000 \
							0x00000000 \
							0X0000000C \
		traffic goes to pipe IPA_CLIENT_TEST2_CONS \
		All DST_IP ==	0X22020000 \
						0x00000000 \
						0x00000000 \
						0X0000000C \
		traffic goes to pipe IPA_CLIENT_TEST3_CONS\
		All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		Register(*this);
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_MSB_OFFSET_IPV6] = 0x11;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_MSB_OFFSET_IPV6] = 0x22;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // TODO: Header Insertion gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0X11020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X0000000C;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0x00000000;

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // TODO: Header Insertion gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0X22020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X0000000C;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0x00000000;

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}
};

/*---------------------------------------------------------------------------*/
/* Test7: IPv6 - Tests routing by destination address */
/*---------------------------------------------------------------------------*/
class IpaRoutingBlockTest007 : public IpaRoutingBlockTestFixture
{
public:
	IpaRoutingBlockTest007()
	{
		m_name = "IpaRoutingBlockTest007";
		m_description = " \
		Routing block test 007 - IPv6 Exact Traffic Class Match \
		1. Generate and commit a single routing tables. \
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All Traffic Class == 0xAA traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All Traffic Class == 0xBB traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		Register(*this);
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[TRAFFIC_CLASS_MSB_OFFSET_IPV6] &= 0xF0;
		m_sendBuffer[TRAFFIC_CLASS_MSB_OFFSET_IPV6] |= 0x0A;
		m_sendBuffer[TRAFFIC_CLASS_LSB_OFFSET_IPV6] &= 0x0F;
		m_sendBuffer[TRAFFIC_CLASS_LSB_OFFSET_IPV6] |= 0xA0;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[TRAFFIC_CLASS_MSB_OFFSET_IPV6] &= 0xF0;
		m_sendBuffer2[TRAFFIC_CLASS_MSB_OFFSET_IPV6] |= 0x0B;
		m_sendBuffer2[TRAFFIC_CLASS_LSB_OFFSET_IPV6] &= 0x0F;
		m_sendBuffer2[TRAFFIC_CLASS_LSB_OFFSET_IPV6] |= 0xB0;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // TODO: Header Insertion gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_TC;
		rt_rule_entry->rule.attrib.u.v6.tc = 0xAA;

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // TODO: Header Insertion gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_TC;
		rt_rule_entry->rule.attrib.u.v6.tc = 0xBB;

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}
};

/*---------------------------------------------------------------------------*/
/* Test8: IPv6 - Tests routing by destination address */
/*---------------------------------------------------------------------------*/
class IpaRoutingBlockTest008 : public IpaRoutingBlockTestFixture
{
public:
	IpaRoutingBlockTest008()
	{
		m_name = "IpaRoutingBlockTest008";
		m_description = " \
		Routing block test 008 - IPv6 Destination address exact match and Traffic Class Match \
		1. Generate and commit a single routing tables. \
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All Traffic Class == 0xAA & IPv6 DST Addr 0xFF020000...00AA traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All Traffic Class == 0xBB & IPv6 DST Addr 0xFF020000...00BB traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		Register(*this);
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[TRAFFIC_CLASS_MSB_OFFSET_IPV6] &= 0xF0;
		m_sendBuffer[TRAFFIC_CLASS_MSB_OFFSET_IPV6] |= 0x0A;
		m_sendBuffer[TRAFFIC_CLASS_LSB_OFFSET_IPV6] &= 0x0F;
		m_sendBuffer[TRAFFIC_CLASS_LSB_OFFSET_IPV6] |= 0xA0;
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[TRAFFIC_CLASS_MSB_OFFSET_IPV6] &= 0xF0;
		m_sendBuffer2[TRAFFIC_CLASS_MSB_OFFSET_IPV6] |= 0x0B;
		m_sendBuffer2[TRAFFIC_CLASS_LSB_OFFSET_IPV6] &= 0x0F;
		m_sendBuffer2[TRAFFIC_CLASS_LSB_OFFSET_IPV6] |= 0xB0;
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR | IPA_FLT_TC;
		rt_rule_entry->rule.attrib.u.v6.tc = 0xAA;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X000000FF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR | IPA_FLT_TC;
		rt_rule_entry->rule.attrib.u.v6.tc = 0xBB;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X000000AA;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}
};

/*---------------------------------------------------------------------------*/
/* Test9: IPv6 - Tests routing by destination address */
/*---------------------------------------------------------------------------*/
class IpaRoutingBlockTest009 : public IpaRoutingBlockTestFixture
{
public:
	IpaRoutingBlockTest009()
	{
		m_name = "IpaRoutingBlockTest009";
		m_description = " \
		Routing block test 009 - IPv6 Exact Flow Label Match \
		1. Generate and commit a single routing tables. \
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All Flow Label == 0xABCDE traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All Flow Label == 0x12345 traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		Register(*this);
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[FLOW_CLASS_MSB_OFFSET_IPV6] &= 0xF0;
		m_sendBuffer[FLOW_CLASS_MSB_OFFSET_IPV6] |= 0x0A;
		m_sendBuffer[FLOW_CLASS_MB_OFFSET_IPV6] = 0xBC;
		m_sendBuffer[FLOW_CLASS_LSB_OFFSET_IPV6] = 0xDE;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[FLOW_CLASS_MSB_OFFSET_IPV6] &= 0xF0;
		m_sendBuffer2[FLOW_CLASS_MSB_OFFSET_IPV6] |= 0x01;
		m_sendBuffer2[FLOW_CLASS_MB_OFFSET_IPV6] = 0x23;
		m_sendBuffer2[FLOW_CLASS_LSB_OFFSET_IPV6] = 0x45;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_FLOW_LABEL;
		rt_rule_entry->rule.attrib.u.v6.flow_label = 0xABCDE;

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_FLOW_LABEL;
		rt_rule_entry->rule.attrib.u.v6.flow_label = 0x12345;

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}
};

/*--------------------------------------------------------------------------*/
/* Test10: IPv4 - Tests routing hashable vs non hashable priorities			*/
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest010 : public IpaRoutingBlockTestFixture
{
public:

	IpaRoutingBlockTest010()
	{
		m_name = "IpaRoutingBlockTest10";
		m_description =" \
		Routing block test 010 - Destination address exact match non hashable priority higher than hashable \
		both match the packet but only non hashable should hit\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP == (192.168.2.170 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP == (192.168.2.170 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v4;
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AA; //192.168.02.170
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AA; //192.168.02.170
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		size_t receivedSize3 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];
		Byte *rxBuff3 = new Byte[0x400];

		if (NULL == rxBuff1 || NULL == rxBuff2 || NULL == rxBuff3)
		{
			printf("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		receivedSize2 = m_consumer.ReceiveData(rxBuff2, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize2, m_consumer2.m_fromChannelName.c_str());

		receivedSize3 = m_defaultConsumer.ReceiveData(rxBuff3, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize3, m_defaultConsumer.m_fromChannelName.c_str());

		/* Compare results */
		isSuccess &= CompareResultVsGolden(m_sendBuffer,  m_sendSize,  rxBuff1, receivedSize);
		isSuccess &= CompareResultVsGolden(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		isSuccess &= CompareResultVsGolden(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}
};

/*--------------------------------------------------------------------------*/
/* Test11: IPv4 - Tests routing hashable vs non hashable priorities			*/
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest011 : public IpaRoutingBlockTestFixture
{
public:

	IpaRoutingBlockTest011()
	{
		m_name = "IpaRoutingBlockTest011";
		m_description =" \
		Routing block test 011 - Destination address exact match hashable priority higher than non hashable \
		both match the packet but only hashable should hit, second packet should get cache hit\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP == (192.168.2.170 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP == (192.168.2.170 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v4;
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);
	}

	bool Setup()
	{
		return IpaRoutingBlockTestFixture:: Setup(true);
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AA; //192.168.02.170
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AA; //192.168.02.170
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		size_t receivedSize3 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];
		Byte *rxBuff3 = new Byte[0x400];

		if (NULL == rxBuff1 || NULL == rxBuff2 || NULL == rxBuff3)
		{
			printf("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		receivedSize2 = m_consumer.ReceiveData(rxBuff2, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize2, m_consumer2.m_fromChannelName.c_str());

		receivedSize3 = m_defaultConsumer.ReceiveData(rxBuff3, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize3, m_defaultConsumer.m_fromChannelName.c_str());

		/* Compare results */
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer,  m_sendSize,  rxBuff1, receivedSize);
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize,receivedSize,rxBuff1);
		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ? 
			IsCacheHit_v5_0(m_sendSize2, receivedSize2, rxBuff2) : IsCacheHit(m_sendSize2,receivedSize2,rxBuff2);
		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize3, receivedSize3, rxBuff3) : IsCacheMiss(m_sendSize3,receivedSize3,rxBuff3);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}
};

/*--------------------------------------------------------------------------*/
/* Test12: IPv4 - Tests routing hashable vs non hashable priorities			*/
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest012 : public IpaRoutingBlockTestFixture
{
public:

	IpaRoutingBlockTest012()
	{
		m_name = "IpaRoutingBlockTest012";
		m_description =" \
		Routing block test 012 - Destination address exact match hashable priority lower than non hashable \
		no match on non hashable rule (with higher priority), match on hashable rule. two packets with\
		different tuple are sent (but match the rule) cache miss expected\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP == (192.168.2.170 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP == (192.168.2.171 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v4;
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);
	}

	bool Setup()
	{
		return IpaRoutingBlockTestFixture:: Setup(true);
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AB; //192.168.02.171
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AA; //192.168.02.170
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;
		unsigned short port;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		port = ntohs(547);//DHCP Client Port
		memcpy (&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		port = ntohs(546);//DHCP Client Port
		memcpy (&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		size_t receivedSize3 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];
		Byte *rxBuff3 = new Byte[0x400];

		if (NULL == rxBuff1 || NULL == rxBuff2 || NULL == rxBuff3)
		{
			printf("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer2.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, m_consumer2.m_fromChannelName.c_str());

		receivedSize2 = m_consumer2.ReceiveData(rxBuff2, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize2, m_consumer2.m_fromChannelName.c_str());

		receivedSize3 = m_defaultConsumer.ReceiveData(rxBuff3, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize3, m_defaultConsumer.m_fromChannelName.c_str());

		/* Compare results */
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer,  m_sendSize,  rxBuff1, receivedSize);
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize,receivedSize,rxBuff1);
		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize2, receivedSize2, rxBuff2) : IsCacheMiss(m_sendSize2,receivedSize2,rxBuff2);
		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize3, receivedSize3, rxBuff3) : IsCacheMiss(m_sendSize3,receivedSize3,rxBuff3);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}

};

/*--------------------------------------------------------------------------*/
/* Test13: IPv4 - Tests routing hashable vs non hashable priorities			*/
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest013 : public IpaRoutingBlockTestFixture
{
public:

	IpaRoutingBlockTest013()
	{
		m_name = "IpaRoutingBlockTest013";
		m_description =" \
		Routing block test 013 - Destination address exact match \
		no match on non hashable rule (with lower priority), match on hashable rule. two packets with\
		different tuple are sent (but match the rule) cache miss expected\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP == (192.168.2.170 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP == (192.168.2.171 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v4;
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);
	}

	bool Setup()
	{
		return IpaRoutingBlockTestFixture:: Setup(true);
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AA; //192.168.02.170
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AB; //192.168.02.171
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;
		unsigned short port;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		port = ntohs(547);//DHCP Client Port
		memcpy (&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		port = ntohs(546);//DHCP Client Port
		memcpy (&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		size_t receivedSize3 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];
		Byte *rxBuff3 = new Byte[0x400];

		if (NULL == rxBuff1 || NULL == rxBuff2 || NULL == rxBuff3)
		{
			printf("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		receivedSize2 = m_consumer.ReceiveData(rxBuff2, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize2, m_consumer.m_fromChannelName.c_str());

		receivedSize3 = m_defaultConsumer.ReceiveData(rxBuff3, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize3, m_defaultConsumer.m_fromChannelName.c_str());

		/* Compare results */
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer,  m_sendSize,  rxBuff1, receivedSize);
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize,receivedSize,rxBuff1);
		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize2, receivedSize2, rxBuff2) : IsCacheMiss(m_sendSize2,receivedSize2,rxBuff2);
		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize3, receivedSize3, rxBuff3) : IsCacheMiss(m_sendSize3,receivedSize3,rxBuff3);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}

};

/*--------------------------------------------------------------------------*/
/* Test14: IPv4 - Tests routing hashable vs non hashable priorities			*/
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest014 : public IpaRoutingBlockTestFixture
{
public:

	IpaRoutingBlockTest014()
	{
		m_name = "IpaRoutingBlockTest014";
		m_description =" \
		Routing block test 014 - Destination address exact match \
		no match on non hashable rule(with higher priority) , match on hashable rule. two identical\
		packets are sent cache hit expected on the second one\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP == (192.168.2.171 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP == (192.168.2.170 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v4;
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);
	}

	bool Setup()
	{
		return IpaRoutingBlockTestFixture:: Setup(true);
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("Failed memory allocation for rt_rule\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AB; //192.168.02.171
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AA; //192.168.02.170
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			free(rt_rule);
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		size_t receivedSize3 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];
		Byte *rxBuff3 = new Byte[0x400];

		if (NULL == rxBuff1 || NULL == rxBuff2 || NULL == rxBuff3)
		{
			printf("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer2.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, m_consumer2.m_fromChannelName.c_str());

		receivedSize2 = m_consumer2.ReceiveData(rxBuff2, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize2, m_consumer2.m_fromChannelName.c_str());

		receivedSize3 = m_defaultConsumer.ReceiveData(rxBuff3, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize3, m_defaultConsumer.m_fromChannelName.c_str());

		/* Compare results */
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer,  m_sendSize,  rxBuff1, receivedSize);
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize,receivedSize,rxBuff1);
		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ? 
			IsCacheHit_v5_0(m_sendSize2, receivedSize2, rxBuff2) : IsCacheHit(m_sendSize2,receivedSize2,rxBuff2);
		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize3, receivedSize3, rxBuff3) : IsCacheMiss(m_sendSize3,receivedSize3,rxBuff3);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}
};


/*--------------------------------------------------------------------------*/
/* Test15: IPv4 - Tests routing hashable vs non hashable priorities			*/
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest015 : public IpaRoutingBlockTestFixture
{
public:

	IpaRoutingBlockTest015()
	{
		m_name = "IpaRoutingBlockTest015";
		m_description =" \
		Routing block test 015 - Destination address exact match \
		no match on non hashable rule(with lower priority) , match on hashable rule. two identical\
		packets are sent cache hit expected on the second one\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP == (192.168.2.170 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP == (192.168.2.171 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v4;
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);
	}

	bool Setup()
	{
		return IpaRoutingBlockTestFixture:: Setup(true);
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("Failed memory allocation for rt_rule\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AA; //192.168.02.170
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AB; //192.168.02.171
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		for (int i = 0; i < rt_rule->num_rules; i++) {
			uRtRuleHdl[i] = rt_rule->rules[i].rt_rule_hdl;
		}
		uNumRtRules = rt_rule->num_rules;

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool RemoveLastRule(enum ipa_ip_type ipType)
	{
		struct ipa_ioc_del_rt_rule *ruleTable;

		ruleTable = (struct ipa_ioc_del_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_del_rt_rule) +
				   sizeof(struct ipa_rt_rule_del));

		ruleTable->commit = 1;
		ruleTable->ip = ipType;
		ruleTable->num_hdls = 1;
		ruleTable->hdl[0].hdl = uRtRuleHdl[uNumRtRules - 1];
		ruleTable->hdl[0].status = 0;

		if (false == m_routing.DeleteRoutingRule(ruleTable))
		{
			printf("Routing rule deletion failed!\n");
			return false;
		}

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		size_t receivedSize3 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];
		Byte *rxBuff3 = new Byte[0x400];

		if (NULL == rxBuff1 || NULL == rxBuff2 || NULL == rxBuff3)
		{
			printf("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		receivedSize2 = m_consumer.ReceiveData(rxBuff2, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize2, m_consumer.m_fromChannelName.c_str());

		receivedSize3 = m_defaultConsumer.ReceiveData(rxBuff3, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize3, m_defaultConsumer.m_fromChannelName.c_str());

		/* Compare results */
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer,  m_sendSize,  rxBuff1, receivedSize);
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize,receivedSize,rxBuff1);
		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ? 
			IsCacheHit_v5_0(m_sendSize2, receivedSize2, rxBuff2) : IsCacheHit(m_sendSize2,receivedSize2,rxBuff2);
		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize3, receivedSize3, rxBuff3) : IsCacheMiss(m_sendSize3,receivedSize3,rxBuff3);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}

	bool ReceivePacketsAndCompareSpecial()
	{
		size_t receivedSize = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];

		if (NULL == rxBuff1)
		{
			printf("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		/* Compare results */
		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer,  m_sendSize,  rxBuff1, receivedSize);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize,receivedSize,rxBuff1);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];
		size_t j;
		for(j = 0; j < m_sendSize; j++)
		    snprintf(&sentBuffer[3 * j], sentBufferSize - 3 * j,
				" %02X", m_sendBuffer[j]);
		for(j = 0; j < receivedSize; j++)
		    snprintf(&recievedBuffer[3 * j], recievedBufferSize - 3 * j,
				" %02X", rxBuff1[j]);
		printf("Expected Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n",m_sendSize,sentBuffer,receivedSize,recievedBuffer);

		delete[] rxBuff1;

		return isSuccess;
	}

protected:
	uint32_t uRtRuleHdl[3];
	uint8_t uNumRtRules;
};

/*--------------------------------------------------------------------------*/
/* Test16: IPv4 - Tests routing hashable vs non hashable priorities			*/
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest016 : public IpaRoutingBlockTestFixture
{
public:

	IpaRoutingBlockTest016()
	{
		m_name = "IpaRoutingBlockTest016";
		m_description =" \
		Routing block test 016 - Destination address exact match max priority for non hashable \
		match on both rule, non hashable rule should win because max priority\
		packets are sent. No cache hit is expected\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP == (192.168.2.170 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP == (192.168.2.170 & 255.255.255.255)traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v4;
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("Failed memory allocation for rt_rule\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AA; //192.168.02.170
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802AA; //192.168.02.170
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable
		rt_rule_entry->rule.max_prio =  1; // max priority

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		size_t receivedSize3 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];
		Byte *rxBuff3 = new Byte[0x400];

		if (NULL == rxBuff1 || NULL == rxBuff2 || NULL == rxBuff3)
		{
			printf("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer2.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		receivedSize2 = m_consumer2.ReceiveData(rxBuff2, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize2, m_consumer.m_fromChannelName.c_str());

		receivedSize3 = m_defaultConsumer.ReceiveData(rxBuff3, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize3, m_defaultConsumer.m_fromChannelName.c_str());

		/* Compare results */
		isSuccess &= CompareResultVsGolden(m_sendBuffer,  m_sendSize,  rxBuff1, receivedSize);
		isSuccess &= CompareResultVsGolden(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		isSuccess &= CompareResultVsGolden(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}

};

/*--------------------------------------------------------------------------*/
/* Test17: IPv4 - Tests routing hashable, non hashable,                     */
/* cache/hash invalidation test on rule addition                            */
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest017 : public IpaRoutingBlockTest015
{
public:

	IpaRoutingBlockTest017()
	{
		m_name = "IpaRoutingBlockTest017";
		m_description =" \
		Routing block test 017 - this test perform test 015 and then commits another rule\
		another identical packet is sent: DST_IP == 192.168.02.170 and expected to get cache miss";
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();
		if (false == isSuccess)
		{
			printf("ReceivePacketsAndCompare failure.\n");
			return false;
		}

		// until here test 15 was run, now we test hash invalidation

		// commit the rules again to clear the cache
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// send the packet again
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// validate we got cache miss
		isSuccess = ReceivePacketsAndCompareSpecial();
		if (false == isSuccess)
		{
			printf("ReceivePacketsAndCompareSpecial failure.\n");
		}
		return isSuccess;
	} // Run()
};

/*--------------------------------------------------------------------------*/
/* Test18: IPv4 - Tests routing hashable, non hashable,                     */
/* cache/hash invalidation test on rule delition                            */
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest018 : public IpaRoutingBlockTest015
{
public:

	IpaRoutingBlockTest018()
	{
		m_name = "IpaRoutingBlockTest018";
		m_description =" \
		Routing block test 018 - this test perform test 015 and then removes last rule\
		another identical packet is sent: DST_IP == 192.168.02.170 and expected to get cache miss";
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV4] = 0xAA;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();
		if (false == isSuccess)
		{
			printf("ReceivePacketsAndCompare failure.\n");
			return false;
		}

		// until here test 15 was run, now we test hash invalidation

		// delete the last rule, this should clear the cache
		res = RemoveLastRule(IPA_IP_v4);
		if (false == res) {
			printf("Failed removing filtering rules.\n");
			return false;
		}

		// send the packet again
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// validate we got cache miss
		isSuccess = ReceivePacketsAndCompareSpecial();
		if (false == isSuccess)
		{
			printf("ReceivePacketsAndCompareSpecial failure.\n");
		}
		return isSuccess;
	} // Run()
};

/*--------------------------------------------------------------------------*/
/* Test20: IPv6 - Tests routing hashable vs non hashable priorities	    */
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest020 : public IpaRoutingBlockTest010
{
public:

	IpaRoutingBlockTest020()
	{
		m_name = "IpaRoutingBlockTest20";
		m_description =" \
		Routing block test 020 - Destination address exact match non hashable priority higher than hashable \
		both match the packet but only non hashable should hit\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000FF \
			traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000FF \
			traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		m_minIPAHwType = IPA_HW_v3_0;
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("Failed memory allocation for rt_rule\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X000000FF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X000000FF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;
		rt_rule_entry->rule.hashable = 0; // non hashable

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			free(rt_rule);
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()
};


/*--------------------------------------------------------------------------*/
/* Test21: IPv6 - Tests routing hashable vs non hashable priorities	    */
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest021 : public IpaRoutingBlockTest011
{
public:

	IpaRoutingBlockTest021()
	{
		m_name = "IpaRoutingBlockTest021";
		m_description =" \
		Routing block test 021 - Destination address exact match hashable priority higher than non hashable \
		both match the packet but only hashable should hit, second packet should get cache hit\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000FF  - hashable\
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000FF - non hahsable \
			traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		m_minIPAHwType = IPA_HW_v3_0;
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("Failed memory allocation for rt_rule\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X000000FF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X000000FF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;
		rt_rule_entry->rule.hashable = 0; // non hashable

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			free(rt_rule);
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()
};

/*--------------------------------------------------------------------------*/
/* Test22: IPv6 - Tests routing hashable vs non hashable priorities         */
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest022 : public IpaRoutingBlockTest012
{
public:

	IpaRoutingBlockTest022()
	{
		m_name = "IpaRoutingBlockTest022";
		m_description =" \
		Routing block test 022 - Destination address exact match hashable priority higher than non hashable \
		no match on non hashable rule (with higher priority), match on hashable rule. two packets with\
		different tuple are sent (but match the rule) cache miss expected\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000AA  - non hashable\
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000FF -  hahsable \
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		m_minIPAHwType = IPA_HW_v3_0;
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("Failed memory allocation for rt_rule\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X000000AA;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X000000FF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;
		rt_rule_entry->rule.hashable = 0; // non hashable

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			free(rt_rule);
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;
		unsigned short port;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		port = ntohs(546);//DHCP Client Port
		memcpy (&m_sendBuffer[IPV6_DST_PORT_OFFSET], &port, sizeof(port));
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		port = ntohs(547);//DHCP Client Port
		memcpy (&m_sendBuffer2[IPV6_DST_PORT_OFFSET], &port, sizeof(port));
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()
};

/*--------------------------------------------------------------------------*/
/* Test23: IPv6 - Tests routing hashable vs non hashable priorities	    */
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest023 : public IpaRoutingBlockTest013
{
public:

	IpaRoutingBlockTest023()
	{
		m_name = "IpaRoutingBlockTest023";
		m_description =" \
		Routing block test 023 - Destination address exact match \
		no match on non hashable rule (with lower priority), match on hashable rule. two packets with\
		different tuple are sent (but match the rule) cache miss expected\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000FF  - hashable\
			traffic goes to pipe IPA_CLIENT_TEST2_CONS\
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000AA -  non hahsable \
			traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		m_minIPAHwType = IPA_HW_v3_0;
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
				   NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("Failed memory allocation for rt_rule\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0] 	 = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3] 	 = 0X000000FF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0] 	 = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3] 	 = 0X000000AA;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;
		rt_rule_entry->rule.hashable = 0; // non hashable

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			free(rt_rule);
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;
		unsigned short port;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		port = ntohs(546);//DHCP Client Port
		memcpy (&m_sendBuffer[IPV6_DST_PORT_OFFSET], &port, sizeof(port));
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		port = ntohs(547);//DHCP Client Port
		memcpy (&m_sendBuffer2[IPV6_DST_PORT_OFFSET], &port, sizeof(port));
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

};

/*--------------------------------------------------------------------------*/
/* Test24: IPv6 - Tests routing hashable vs non hashable priorities	    */
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest024 : public IpaRoutingBlockTest014
{
public:

	IpaRoutingBlockTest024()
	{
		m_name = "IpaRoutingBlockTest024";
		m_description =" \
		Routing block test 024 - Destination address exact match \
		no match on non hashable rule(with higher priority) , match on hashable rule. two identical\
		packets are sent cache hit expected on the second one\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000AA  - non hashable\
			traffic goes to pipe IPA_CLIENT_TEST2_CONS\
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000FF -  hahsable \
			traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		m_minIPAHwType = IPA_HW_v3_0;
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
				   NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("Failed memory allocation for rt_rule\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0] 	 = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3] 	 = 0X000000AA;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0] 	 = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3] 	 = 0X000000FF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;
		rt_rule_entry->rule.hashable = 0; // non hashable

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			free(rt_rule);
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

};

/*--------------------------------------------------------------------------*/
/* Test25: IPv6 - Tests routing hashable vs non hashable priorities	    */
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest025 : public IpaRoutingBlockTest015
{
public:
	IpaRoutingBlockTest025()
	{
		m_name = "IpaRoutingBlockTest025";
		m_description =" \
		Routing block test 025 - Destination address exact match \
		no match on non hashable rule(with lower priority) , match on hashable rule. two identical\
		packets are sent cache hit expected on the second one\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000FF  - hashable\
			traffic goes to pipe IPA_CLIENT_TEST2_CONS\
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000AA -  non hahsable \
			traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		m_minIPAHwType = IPA_HW_v3_0;
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
				   NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("Failed memory allocation for rt_rule\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0] 	 = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3] 	 = 0X000000FF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0] 	 = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3] 	 = 0X000000AA;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;
		rt_rule_entry->rule.hashable = 0; // non hashable

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			free(rt_rule);
			return false;
		}

		for (int i = 0; i < rt_rule->num_rules; i++) {
			uRtRuleHdl[i] = rt_rule->rules[i].rt_rule_hdl;
		}
		uNumRtRules = rt_rule->num_rules;

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()
};


/*--------------------------------------------------------------------------*/
/* Test26: IPv6 - Tests routing hashable vs non hashable priorities	    */
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest026 : public IpaRoutingBlockTest016
{
public:

	IpaRoutingBlockTest026()
	{
		m_name = "IpaRoutingBlockTest026";
		m_description =" \
		Routing block test 026 - Destination address exact match max priority for non hashable \
		match on both rule, non hashable rule should win because max priority\
		packets are sent cache hit expected on the second one\
		2. Generate and commit Three routing rules: (DST & Mask Match). \
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000FF  - hashable\
			traffic goes to pipe IPA_CLIENT_TEST2_CONS\
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000FF -  non hahsable max prio \
			traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		m_minIPAHwType = IPA_HW_v3_0;
	}

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
				   NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("Failed memory allocation for rt_rule\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0] 	 = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3] 	 = 0X000000FF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 1; // hashable

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0] 	 = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2] 	 = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3] 	 = 0X000000FF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;
		rt_rule_entry->rule.hashable = 0; // non hashable
		rt_rule_entry->rule.max_prio = 1; // max prio

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;
		rt_rule_entry->rule.hashable = 0; // non hashable

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			free(rt_rule);
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()
};

/*--------------------------------------------------------------------------*/
/* Test27: IPv6 - Tests routing hashable, non hashable,                     */
/* cache/hash invalidation test on rule addition                            */
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest027 : public IpaRoutingBlockTest025
{
public:

	IpaRoutingBlockTest027()
	{
		m_name = "IpaRoutingBlockTest027";
		m_description =" \
		Routing block test 027 - this test perform test 025 and then commits another rule\
		another identical packet is sent: DST_IP == 192.168.02.170 and expected to get cache miss";
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();
		if (false == isSuccess)
		{
			printf("ReceivePacketsAndCompare failure.\n");
			return false;
		}

		// until here test 25 was run, now we test hash invalidation

		// commit the rules again to clear the cache
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// send the packet again
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// validate we got cache miss
		isSuccess = ReceivePacketsAndCompareSpecial();
		if (false == isSuccess)
		{
			printf("ReceivePacketsAndCompareSpecial failure.\n");
		}
		return isSuccess;
	} // Run()
};

/*--------------------------------------------------------------------------*/
/* Test28: IPv6 - Tests routing hashable, non hashable,                     */
/* cache/hash invalidation test on rule delition                            */
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest028 : public IpaRoutingBlockTest025
{
public:

	IpaRoutingBlockTest028()
	{
		m_name = "IpaRoutingBlockTest028";
		m_description =" \
		Routing block test 028 - this test perform test 025 and then deletes last rule\
		another identical packet is sent: DST_IP == 192.168.02.170 and expected to get cache miss";
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v6);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xFF;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();
		if (false == isSuccess)
		{
			printf("ReceivePacketsAndCompare failure.\n");
			return false;
		}

		// until here test 25 was run, now we test hash invalidation

		// delete the last rule, this should clear the cache
		res = RemoveLastRule(IPA_IP_v6);
		if (false == res) {
			printf("Failed removing filtering rules.\n");
			return false;
		}

		// send the packet again
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// validate we got cache miss
		isSuccess = ReceivePacketsAndCompareSpecial();
		if (false == isSuccess)
		{
			printf("ReceivePacketsAndCompareSpecial failure.\n");
		}
		return isSuccess;
	} // Run()
};

/*--------------------------------------------------------------------------*/
/* Test30: Pure ack packet matching											*/
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest030 : IpaRoutingBlockTestFixture
{
public:
	IpaRoutingBlockTest030()
	{
		m_name = "IpaRoutingBlockTest030";
		m_description =" \
		Routing block test 030 - Pure Ack packet matching \
		1. Generate and commit a single routing table. \
		2. Generate and commit Three routing rules: \
			All TCP pure ack traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP == (192.168.2.200 & 255.255.255.255) traffic goes to pipe IPA_CLIENT_TEST3_CONS\
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v4;
		m_minIPAHwType = IPA_HW_v4_5;
		Register(*this);
	}

	bool LoadPackets()
	{
		if (!LoadNoPayloadPacket(m_IpaIPType, m_sendBuffer, m_sendSize)) {
			LOG_MSG_ERROR("Failed loading No Payload Packet");
			return false;
		}

		if (!LoadDefaultPacket(m_IpaIPType, m_sendBuffer2, m_sendSize2)) {
			LOG_MSG_ERROR("Failed loading default Packet");
			return false;
		}

		if (!LoadDefaultPacket(m_IpaIPType, m_sendBuffer3, m_sendSize3)) {
			LOG_MSG_ERROR("Failed loading default Packet");
			return false;
		}

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load IP packets
		res = LoadPackets();
		if (false == res) {
			printf("Failed loading packets\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[IPv4_TCP_FLAGS_OFFSET] |= TCP_ACK_FLAG_MASK;
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0xC8;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[IPv4_TCP_FLAGS_OFFSET] |= TCP_ACK_FLAG_MASK;
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV4] = 0xC8;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		m_sendBuffer3[IPv4_TCP_FLAGS_OFFSET] |= TCP_ACK_FLAG_MASK;
		m_sendBuffer3[DST_ADDR_LSB_OFFSET_IPV4] = 0x64;
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_IS_PURE_ACK;

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr      = 0xC0A802C8;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}
};

/*--------------------------------------------------------------------------*/
/* Test31: IPv6 Pure ack packet matching		 							*/
/*--------------------------------------------------------------------------*/
class IpaRoutingBlockTest031 : public IpaRoutingBlockTestFixture
{
public:
	IpaRoutingBlockTest031()
	{
		m_name = "IpaRoutingBlockTest031";
		m_description =" \
		Routing block test 031 - IPv6 Pure Ack packet matching \
		1. Generate and commit a single routing table. \
		2. Generate and commit Three routing rules: \
			All TCP pure ack traffic goes to pipe IPA_CLIENT_TEST2_CONS \
			All DST_IP ==	0XFF020000 \
							0x00000000 \
							0x00000000 \
							0X000000F5 \
			traffic goes to pipe IPA_CLIENT_TEST3_CONS \
			All other traffic goes to pipe IPA_CLIENT_TEST4_CONS";
		m_IpaIPType = IPA_IP_v6;
		m_minIPAHwType = IPA_HW_v4_5;
		Register(*this);
	}

	bool LoadPackets()
	{
		if (!LoadNoPayloadPacket(m_IpaIPType, m_sendBuffer, m_sendSize)) {
			LOG_MSG_ERROR("Failed loading No Payload Packet");
			return false;
		}

		if (!LoadNoPayloadPacket(m_IpaIPType, m_sendBuffer2, m_sendSize2)) {
			LOG_MSG_ERROR("Failed loading default Packet");
			return false;
		}

		if (!LoadNoPayloadPacket(m_IpaIPType, m_sendBuffer3, m_sendSize3)) {
			LOG_MSG_ERROR("Failed loading default Packet");
			return false;
		}

		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		// Add the relevant routing rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding routing rules.\n");
			return false;
		}

		// Load input data (IP packet)
		res = LoadPackets();
		if (false == res) {
			printf("Failed loading packets\n");
			return false;
		}

		// Send first packet
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xF5;
		m_sendBuffer[IPv6_TCP_FLAGS_OFFSET] |= TCP_ACK_FLAG_MASK;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xF5;
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		return isSuccess;
	} // Run()

	bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;
		const int NUM_RULES = 3;

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       NUM_RULES*sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("fail\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = NUM_RULES;
		rt_rule->ip = IPA_IP_v6;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		rt_rule_entry = &rt_rule->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_IS_PURE_ACK;

		rt_rule_entry = &rt_rule->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[0]      = 0XFF020000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[2]      = 0x00000000;
		rt_rule_entry->rule.attrib.u.v6.dst_addr[3]      = 0X000000F5;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;
		rt_rule_entry->rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;

		rt_rule_entry = &rt_rule->rules[2];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST4_CONS;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		free(rt_rule);

		InitFilteringBlock();

		return true;
	}
};

/*---------------------------------------------------------------------------*/
/* Test100: Cache LRU behavior test  */
/*---------------------------------------------------------------------------*/
#define CHACHE_ENTRIES 64
#define CHACHE_PLUS_ONE (CHACHE_ENTRIES +1)
class IpaRoutingBlockTest040 : public IpaRoutingBlockTestFixture
{
public:
	IpaRoutingBlockTest040()
	{
		m_name = "IpaRoutingBlockTest040";
		m_description = " \
		Routing block test 40 - Cache LRU behavior test \
		1. Preload the cache by sending 64 packets for different connections \
		2. Send another packet for 65th connection \
		3. Send packets for first 64 connections \
		4. Verify that 1st connections entry was reclaimed";
		m_IpaIPType = IPA_IP_v4;
		m_minIPAHwType = IPA_HW_v4_0;
		Register(*this);
	}

	bool Setup()
	{
		return IpaRoutingBlockTestFixture:: Setup(true);
	}

	virtual bool AddRules()
	{
		struct ipa_ioc_add_rt_rule *rt_rule;
		struct ipa_rt_rule_add *rt_rule_entry;

		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		rt_rule = (struct ipa_ioc_add_rt_rule *)
			calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
			       CHACHE_PLUS_ONE * sizeof(struct ipa_rt_rule_add));

		if(!rt_rule) {
			printf("Failed memory allocation for rt_rule\n");
			return false;
		}

		rt_rule->commit = 1;
		rt_rule->num_rules = CHACHE_PLUS_ONE;
		rt_rule->ip = IPA_IP_v4;
		strlcpy(rt_rule->rt_tbl_name, "LAN", sizeof(rt_rule->rt_tbl_name));

		for (int i = 0; i < CHACHE_PLUS_ONE; i++) {
			rt_rule_entry = &rt_rule->rules[i];
			rt_rule_entry->at_rear = 1;
			rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
			rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
			rt_rule_entry->rule.attrib.u.v4.dst_addr = 0xC0A80101 + i; // DST_IP == 192.168.1.(1+i)
			rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;
			rt_rule_entry->rule.hashable = 1;
		}

		// The last rule has to be catch all, otherwize no rule will work
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0x0;

		if (false == m_routing.AddRoutingRule(rt_rule))
		{
			printf("Routing rule addition failed!\n");
			return false;
		}

		printf("rt rule hdl1=%x\n", rt_rule_entry->rt_rule_hdl);

		free(rt_rule);

		InitFilteringBlock();

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		// Add the relevant filtering rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding filtering rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(IPA_IP_v4);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send the first CHACHE_ENTRIES packets
		// Receive packets from the channels and compare results
		// All rules should be cache miss
		for (int i = 0; i < CHACHE_ENTRIES; i++) {
			m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0x1 + i;
			isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
			if (false == isSuccess)
			{
				printf("SendData failure.\n");
				return false;
			}

			isSuccess = ReceivePacketAndCompareFrom(m_consumer, m_sendBuffer, m_sendSize, false);
			if (false == isSuccess)	{
				printf("%s:%d: ReceivePacketAndCompareFrom failure.\n", __FUNCTION__, __LINE__);
				return false;
			}
		}

		// Send again the first CHACHE_ENTRIES packets
		// Receive packets from the channels and compare results
		// All rules should be cache hit
		for (int i = 0; i < CHACHE_ENTRIES; i++) {
			m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0x1 + i;
			isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
			if (false == isSuccess)
			{
				printf("SendData failure.\n");
				return false;
			}

			isSuccess = ReceivePacketAndCompareFrom(m_consumer, m_sendBuffer, m_sendSize, true);
			if (false == isSuccess)	{
				printf("%s:%d: ReceivePacketAndCompareFrom failure.\n", __FUNCTION__, __LINE__);
				return false;
			}
		}

		// Send a packet to a new filter entry, this should trigger the LRU clear
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0x1 + CHACHE_ENTRIES;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)	{
			printf("SendData failure.\n");
			return false;
		}

		// receive and verify that cache was missed
		isSuccess = ReceivePacketAndCompareFrom(m_consumer, m_sendBuffer, m_sendSize, false);
		if (false == isSuccess)	{
			printf("%s:%d: ReceivePacketAndCompareFrom failure.\n", __FUNCTION__, __LINE__);
			return false;
		}

		// send the first packet again
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV4] = 0x1;
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)	{
			printf("SendData failure.\n");
			return false;
		}

		// receive and verify that cache was missed
		isSuccess = ReceivePacketAndCompareFrom(m_consumer, m_sendBuffer, m_sendSize, false);
		if (false == isSuccess)	{
			printf("%s:%d: ReceivePacketAndCompareFrom failure.\n", __FUNCTION__, __LINE__);
			return false;
		}

		printf("Leaving %s, %s(), Returning %d\n",__FUNCTION__, __FILE__, isSuccess);

		return isSuccess;
	} // Run()
};

static class IpaRoutingBlockTest1 ipaRoutingBlockTest1;
static class IpaRoutingBlockTest2 ipaRoutingBlockTest2;
static class IpaRoutingBlockTest3 ipaRoutingBlockTest3;
static class IpaRoutingBlockTest4 ipaRoutingBlockTest4;
static class IpaRoutingBlockTest5 ipaRoutingBlockTest5;
static class IpaRoutingBlockTest006 ipaRoutingBlockTest006;
static class IpaRoutingBlockTest007 ipaRoutingBlockTest007;
static class IpaRoutingBlockTest008 ipaRoutingBlockTest008;
static class IpaRoutingBlockTest009 ipaRoutingBlockTest009;

static class IpaRoutingBlockTest010 ipaRoutingBlockTest010;
static class IpaRoutingBlockTest011 ipaRoutingBlockTest011;
static class IpaRoutingBlockTest012 ipaRoutingBlockTest012;
static class IpaRoutingBlockTest013 ipaRoutingBlockTest013;
static class IpaRoutingBlockTest014 ipaRoutingBlockTest014;
static class IpaRoutingBlockTest015 ipaRoutingBlockTest015;
static class IpaRoutingBlockTest016 ipaRoutingBlockTest016;
static class IpaRoutingBlockTest017 ipaRoutingBlockTest017;
static class IpaRoutingBlockTest018 ipaRoutingBlockTest018;

static class IpaRoutingBlockTest020 ipaRoutingBlockTest020;
static class IpaRoutingBlockTest021 ipaRoutingBlockTest021;
static class IpaRoutingBlockTest022 ipaRoutingBlockTest022;
static class IpaRoutingBlockTest023 ipaRoutingBlockTest023;
static class IpaRoutingBlockTest024 ipaRoutingBlockTest024;
static class IpaRoutingBlockTest025 ipaRoutingBlockTest025;
static class IpaRoutingBlockTest026 ipaRoutingBlockTest026;
static class IpaRoutingBlockTest027 ipaRoutingBlockTest027;
static class IpaRoutingBlockTest028 ipaRoutingBlockTest028;

static class IpaRoutingBlockTest030 ipaRoutingBlockTest030;
static class IpaRoutingBlockTest031 ipaRoutingBlockTest031;

static class IpaRoutingBlockTest040 ipaRoutingBlockTest040;


