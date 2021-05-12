/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
#include <fcntl.h>
#include <cstring> // for memcpy
#include <sys/ioctl.h> //for ioctl
#include "hton.h" // for htonl
#include "InterfaceAbstraction.h"
#include "Constants.h"
#include "Logger.h"
#include "TestsUtils.h"
#include "Filtering.h"
#include "RoutingDriverWrapper.h"
#include "IPAFilteringTable.h"

//TODO Add Enum for IP/TCP/UDP Fields

#define IP4_TOS_FIELD_OFFSET (1)
#define IPV4_PROTOCOL_OFFSET (9)
#define IPV6_NEXT_HDR_OFFSET (6)
#define IPV4_SRC_ADDR_OFFSET (12)
#define IPV4_DST_ADDR_OFFSET (16)
#define IPV4_SRC_PORT_OFFSET (20)
#define IPV4_DST_PORT_OFFSET (20+2)
#define IPV6_SRC_PORT_OFFSET (40)
#define IPV6_DST_PORT_OFFSET (40+2)

#define DST_ADDR_LSB_OFFSET_IPV4 (19)
#define DST_ADDR_LSB_OFFSET_IPV6 (39)

#define IPV4_FRAGMENT_FLAGS_OFFSET (6)
#define IPV6_FRAGMENT_FLAGS_OFFSET (42)
#define IPV6_FRAGMENT_NEXT_HDR_OFFSET (40)

#define IPv4_TCP_FLAGS_OFFSET (20+13)
#define TCP_ACK_FLAG_MASK (0x10)

#define TAG_802_1Q_OFFSET (12)

extern Logger g_Logger;

class IpaFilteringBlockTestFixture : public TestBase
{
public:

	IpaFilteringBlockTestFixture():
		m_sendSize (BUFF_MAX_SIZE),
		m_sendSize2 (BUFF_MAX_SIZE),
		m_sendSize3 (BUFF_MAX_SIZE),
		m_IpaIPType(IPA_IP_v4),
		m_extHdrType(NONE)
	{
		memset(m_sendBuffer, 0, sizeof(m_sendBuffer));	// First input file / IP packet
		memset(m_sendBuffer2, 0, sizeof(m_sendBuffer2));	// Second input file / IP packet
		memset(m_sendBuffer3, 0, sizeof(m_sendBuffer3));	// Third input file (default) / IP packet
		m_testSuiteName.push_back("Filtering");
	}

	static int SetupKernelModule(bool en_status = 0)
	{
		int retval;
		struct ipa_channel_config from_ipa_channels[3];
		struct test_ipa_ep_cfg from_ipa_cfg[3];
		struct ipa_channel_config to_ipa_channels[2];
		struct test_ipa_ep_cfg to_ipa_cfg[2];

		struct ipa_test_config_header header = {0};
		struct ipa_channel_config *to_ipa_array[2];
		struct ipa_channel_config *from_ipa_array[3];

		/* From ipa configurations - 3 pipes */
		memset(&from_ipa_cfg[0], 0, sizeof(from_ipa_cfg[0]));
		prepare_channel_struct(&from_ipa_channels[0],
				header.from_ipa_channels_num++,
				IPA_CLIENT_TEST2_CONS,
				(void *)&from_ipa_cfg[0],
				sizeof(from_ipa_cfg[0]),
				en_status);
		from_ipa_array[0] = &from_ipa_channels[0];

		memset(&from_ipa_cfg[1], 0, sizeof(from_ipa_cfg[1]));
		prepare_channel_struct(&from_ipa_channels[1],
				header.from_ipa_channels_num++,
				IPA_CLIENT_TEST3_CONS,
				(void *)&from_ipa_cfg[1],
				sizeof(from_ipa_cfg[1]),
				en_status);
		from_ipa_array[1] = &from_ipa_channels[1];

		memset(&from_ipa_cfg[2], 0, sizeof(from_ipa_cfg[2]));
		prepare_channel_struct(&from_ipa_channels[2],
				header.from_ipa_channels_num++,
				IPA_CLIENT_TEST4_CONS,
				(void *)&from_ipa_cfg[2],
				sizeof(from_ipa_cfg[2]),
				en_status);
		from_ipa_array[2] = &from_ipa_channels[2];

		/* To ipa configurations - 1 pipes */
		memset(&to_ipa_cfg[0], 0, sizeof(to_ipa_cfg[0]));
		prepare_channel_struct(&to_ipa_channels[0],
				header.to_ipa_channels_num++,
				IPA_CLIENT_TEST_PROD,
				(void *)&to_ipa_cfg[0],
				sizeof(to_ipa_cfg[0]));
		to_ipa_array[0] = &to_ipa_channels[0];

		/* header removal for Ethernet header + 8021Q header */
		memset(&to_ipa_cfg[1], 0, sizeof(to_ipa_cfg[1]));
		to_ipa_cfg[1].hdr.hdr_len = ETH8021Q_HEADER_LEN;
		to_ipa_cfg[1].hdr.hdr_ofst_metadata_valid = 1;
		to_ipa_cfg[1].hdr.hdr_ofst_metadata =
			ETH8021Q_METADATA_OFFSET;
		prepare_channel_struct(&to_ipa_channels[1],
			header.to_ipa_channels_num++,
			IPA_CLIENT_TEST2_PROD,
			(void *)&to_ipa_cfg[1],
			sizeof(to_ipa_cfg[1]));
		to_ipa_array[1] = &to_ipa_channels[1];

		prepare_header_struct(&header, from_ipa_array, to_ipa_array);

		retval = GenericConfigureScenario(&header);

		return retval;
	}

	bool Setup()
	{
		bool bRetVal = true;

		if (SetupKernelModule() != true)
			return bRetVal;

		m_producer.Open(INTERFACE0_TO_IPA_DATA_PATH, INTERFACE0_FROM_IPA_DATA_PATH);
		m_producer2.Open(INTERFACE4_TO_IPA_DATA_PATH, INTERFACE4_FROM_IPA_DATA_PATH);

		m_consumer.Open(INTERFACE1_TO_IPA_DATA_PATH, INTERFACE1_FROM_IPA_DATA_PATH);
		m_consumer2.Open(INTERFACE2_TO_IPA_DATA_PATH, INTERFACE2_FROM_IPA_DATA_PATH);
		m_defaultConsumer.Open(INTERFACE3_TO_IPA_DATA_PATH, INTERFACE3_FROM_IPA_DATA_PATH);

		if (!m_routing.DeviceNodeIsOpened())
		{
			printf("Routing block is not ready for immediate commands!\n");
			return false;
		}

		if (!m_filtering.DeviceNodeIsOpened())
		{
			printf("Filtering block is not ready for immediate commands!\n");
			return false;
		}
		m_routing.Reset(IPA_IP_v4); // This will issue a Reset command to the Filtering as well
		m_routing.Reset(IPA_IP_v6); // This will issue a Reset command to the Filtering as well
		return true;
	} // Setup()

	bool Setup(bool en_status = false)
	{
		bool bRetVal = true;

		if (SetupKernelModule(en_status) != true)
			return bRetVal;

		m_producer.Open(INTERFACE0_TO_IPA_DATA_PATH, INTERFACE0_FROM_IPA_DATA_PATH);

		m_consumer.Open(INTERFACE1_TO_IPA_DATA_PATH, INTERFACE1_FROM_IPA_DATA_PATH);
		m_consumer2.Open(INTERFACE2_TO_IPA_DATA_PATH, INTERFACE2_FROM_IPA_DATA_PATH);
		m_defaultConsumer.Open(INTERFACE3_TO_IPA_DATA_PATH, INTERFACE3_FROM_IPA_DATA_PATH);

		if (!m_routing.DeviceNodeIsOpened())
		{
			printf("Routing block is not ready for immediate commands!\n");
			return false;
		}

		if (!m_filtering.DeviceNodeIsOpened())
		{
			printf("Filtering block is not ready for immediate commands!\n");
			return false;
		}
		m_routing.Reset(IPA_IP_v4); // This will issue a Reset command to the Filtering as well
		m_routing.Reset(IPA_IP_v6); // This will issue a Reset command to the Filtering as well
		return true;
	} // Setup()

	bool Teardown()
	{
		m_producer.Close();
		m_producer2.Close();
		m_consumer.Close();
		m_consumer2.Close();
		m_defaultConsumer.Close();
		return true;
	} // Teardown()

	virtual bool LoadFiles(enum ipa_ip_type ip)
	{
		string fileName;

		if (IPA_IP_v4 == ip) {
			fileName = "Input/IPv4_1";
		} else {
			fileName = "Input/IPv6";
		}

		if (!LoadDefaultPacket(ip, m_extHdrType, m_sendBuffer, m_sendSize)) {
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}
		printf ("Loaded %zu Bytes to Buffer 1\n",m_sendSize);

		if (!LoadDefaultPacket(ip, m_extHdrType, m_sendBuffer2, m_sendSize2)) {
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}
		printf ("Loaded %zu Bytes to Buffer 2\n",m_sendSize2);

		if (!LoadDefaultPacket(ip, m_extHdrType, m_sendBuffer3, m_sendSize3)) {
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}
		printf ("Loaded %zu Bytes to Buffer 3\n",m_sendSize3);

		return true;
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

		if((bool)pStatus->filt_hash){
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

		if((bool)pStatus->filt_hash){
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

		if(!((bool)pStatus->filt_hash)){
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

		if(!((bool)pStatus->filt_hash)){
			printf ("%s::cache miss!! \n",__FUNCTION__);
			return true;
		}

		printf ("%s::cache hit!! \n",__FUNCTION__);
		return false;
	}


	bool ReceivePacketAndCompareFrom(InterfaceAbstraction& cons, Byte* send,
									 size_t send_sz, bool shouldBeHit)
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

		receivedSize = cons.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, cons.m_fromChannelName.c_str());

		// Compare results
		isSuccess &= CompareResultVsGolden_w_Status(send, send_sz, rxBuff1, receivedSize);

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
		size_t j;

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		for(j = 0; j < m_sendSize; j++)
			snprintf(&sentBuffer[3 * j], sentBufferSize - (3 * j + 1), " %02X", send[j]);
		for(j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], recievedBufferSize - (3 * j + 1), " %02X", rxBuff1[j]);
		printf("Expected Value (%zu)\n%s\n, Received Value1(%zu)\n%s\n",send_sz,sentBuffer,receivedSize,recievedBuffer);

		delete[] rxBuff1;
		delete[] recievedBuffer;
		delete[] sentBuffer;

		//receivedSize = excp_cons.ReceiveData(rxBuff1, 0x400);
		//printf("Received %zu bytes on %s.\n", receivedSize, excp_cons.m_fromChannelName.c_str());

		return isSuccess;
	}

	virtual bool ReceivePacketsAndCompare()
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

		// Compare results
		pkt1_cmp_succ = CompareResultVsGolden(m_sendBuffer, m_sendSize, rxBuff1, receivedSize);
		pkt2_cmp_succ = CompareResultVsGolden(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		pkt3_cmp_succ = CompareResultVsGolden(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		size_t recievedBufferSize =
			MAX3(receivedSize, receivedSize2, receivedSize3) * 3;
		size_t sentBufferSize =
			MAX3(m_sendSize, m_sendSize2, m_sendSize3) * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];
		size_t j;

		if (NULL == recievedBuffer || NULL == sentBuffer) {
			printf("Memory allocation error\n");
			return false;
		}

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);
		for(j = 0; j < m_sendSize; j++)
			snprintf(&sentBuffer[3 * j], sentBufferSize - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for(j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], recievedBufferSize - (3 * j + 1), " %02X", rxBuff1[j]);
		printf("Expected Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n-->Value1 %s\n",
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

	// This function creates three IPv4 bypass routing entries and commits them.
	bool CreateThreeIPv4BypassRoutingTables (const char * bypass0, const char * bypass1, const char * bypass2)
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		struct ipa_ioc_add_rt_rule *rt_rule0 = 0, *rt_rule1 = 0,*rt_rule2 = 0;
		struct ipa_rt_rule_add *rt_rule_entry;

		rt_rule0 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
					sizeof(struct ipa_ioc_add_rt_rule) +
						1*sizeof(struct ipa_rt_rule_add)
				);
		if(!rt_rule0) {
			printf("calloc failed to allocate rt_rule0 in %s\n",__FUNCTION__);
			return false;
		}
		rt_rule1 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
					sizeof(struct ipa_ioc_add_rt_rule) +
						1*sizeof(struct ipa_rt_rule_add)
				);
		if(!rt_rule1) {
			printf("calloc failed to allocate rt_rule1 in %s\n",__FUNCTION__);
			Free(rt_rule0);
			return false;
		}
		rt_rule2 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
					sizeof(struct ipa_ioc_add_rt_rule) +
						1*sizeof(struct ipa_rt_rule_add)
				);
		if(!rt_rule2) {
			printf("calloc failed to allocate rt_rule2 in %s\n",__FUNCTION__);
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
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;//Setting
		//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // TODO Header Inserion - gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr = 0xaabbccdd;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0x00000000;// All Packets will get a "Hit"
		if (false == m_routing.AddRoutingRule(rt_rule0))
		{
			printf("Routing rule addition(rt_rule0) failed!\n");
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
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // TODO Header Inserion - gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr = 0xaabbccdd;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0x00000000;// All Packets will get a "Hit"
		if (false == m_routing.AddRoutingRule(rt_rule1))
		{
			printf("Routing rule addition(rt_rule1) failed!\n");
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
		//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // TODO Header Inserion - gidons, there is no support for header insertion / removal yet.
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr = 0xaabbccdd;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0x00000000;// All Packets will get a "Hit"
		if (false == m_routing.AddRoutingRule(rt_rule2))
		{
			printf("Routing rule addition(rt_rule2) failed!\n");
			Free (rt_rule2);
			Free (rt_rule1);
			Free (rt_rule0);
			return false;
		}


		Free (rt_rule2);
		Free (rt_rule1);
		Free (rt_rule0);
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	// This function creates three IPv6 bypass routing entries and commits them.
	bool CreateThreeIPv6BypassRoutingTables (const char * bypass0, const char * bypass1, const char * bypass2)
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		struct ipa_ioc_add_rt_rule *rt_rule0 = 0, *rt_rule1 = 0,*rt_rule2 = 0;
		struct ipa_rt_rule_add *rt_rule_entry;

		rt_rule0 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
					sizeof(struct ipa_ioc_add_rt_rule) +
						1*sizeof(struct ipa_rt_rule_add)
				);
		if(!rt_rule0) {
			printf("calloc failed to allocate rt_rule0 in %s\n",__FUNCTION__);
			return false;
		}
		rt_rule1 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
					sizeof(struct ipa_ioc_add_rt_rule) +
						1*sizeof(struct ipa_rt_rule_add)
				);
		if(!rt_rule1) {
			printf("calloc failed to allocate rt_rule1 in %s\n",__FUNCTION__);
			Free(rt_rule0);
			return false;
		}
		rt_rule2 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
					sizeof(struct ipa_ioc_add_rt_rule) +
						1*sizeof(struct ipa_rt_rule_add)
				);
		if(!rt_rule2) {
			printf("calloc failed to allocate rt_rule2 in %s\n",__FUNCTION__);
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
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;//Setting
		//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // TODO Header Inserion - gidons, there is no support for header insertion / removal yet.
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
			printf("Routing rule addition(rt_rule0) failed!\n");
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
//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // TODO Header Inserion - gidons, there is no support for header insertion / removal yet.
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
			printf("Routing rule addition(rt_rule1) failed!\n");
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
		//		rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // TODO Header Inserion - gidons, there is no support for header insertion / removal yet.
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
			printf("Routing rule addition(rt_rule2) failed!\n");
			Free (rt_rule2);
			Free (rt_rule1);
			Free (rt_rule0);
			return false;
		}


		Free (rt_rule2);
		Free (rt_rule1);
		Free (rt_rule0);
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool GetThreeIPv6BypassRoutingTables(uint32_t *Hndl0, uint32_t *Hndl1, uint32_t *Hndl2)
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv6BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v6;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v6;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v6;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		*Hndl0 = routing_table0.hdl;
		*Hndl1 = routing_table1.hdl;
		*Hndl2 = routing_table2.hdl;

		return true;

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

	virtual bool ModifyPackets() = 0;
	virtual bool AddRules() = 0;

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		// Add the relevant filtering rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding filtering rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(m_IpaIPType);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		res = ModifyPackets();
		if (false == res) {
			printf("Failed to modify packets.\n");
			return false;
		}

		// Send first packet
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
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

		printf("Leaving %s, %s(), Returning %d\n",__FUNCTION__, __FILE__,isSuccess);

		return isSuccess;
	} // Run()


	~IpaFilteringBlockTestFixture()
	{
		m_sendSize = 0;
		m_sendSize2 = 0;
		m_sendSize3 = 0;
	}

	static Filtering m_filtering;
	static RoutingDriverWrapper m_routing;
	InterfaceAbstraction m_producer;
	InterfaceAbstraction m_producer2;
	InterfaceAbstraction m_consumer;
	InterfaceAbstraction m_consumer2;
	InterfaceAbstraction m_defaultConsumer;

	static const size_t BUFF_MAX_SIZE = 1024;
	static const uint32_t MAX_RULES_NUM = 250;
	static const uint32_t MIN_RULES_NUM = 0;

	Byte m_sendBuffer[BUFF_MAX_SIZE];	// First input file / IP packet
	Byte m_sendBuffer2[BUFF_MAX_SIZE];	// Second input file / IP packet
	Byte m_sendBuffer3[BUFF_MAX_SIZE];	// Third input file (default) / IP packet
	size_t m_sendSize;
	size_t m_sendSize2;
	size_t m_sendSize3;
	enum ipa_ip_type m_IpaIPType;
	enum ipv6_ext_hdr_type m_extHdrType;

private:
};

RoutingDriverWrapper IpaFilteringBlockTestFixture::m_routing;
Filtering IpaFilteringBlockTestFixture::m_filtering;


/*---------------------------------------------------------------------------*/
/* Test001: Destination IP address and subnet mask match against LAN subnet  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest001 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest001()
	{
		m_name = "IpaFilteringBlockTest001";
		m_description =
		"Filtering block test 001 - Destination IP address and subnet mask match against LAN subnet (Global Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules: (DST & Mask Match). \
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 0 \
			All DST_IP == (192.169.1.1 & 255.0.0.255)traffic goes to routing table 1 \
			All DST_IP == (192.169.1.2 & 255.0.0.255)traffic goes to routing table 2";
		m_maxIPAHwType = IPA_HW_v2_6L;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,3);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		// TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1 // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		// TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1.
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
		}

		// Configuring Filtering Rule No.2 // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		// TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;

	}// AddRules()

	virtual bool ModifyPackets()
	{
		int address;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80101);//192.168.1.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;
	}// ModifyPacktes ()
};

/*---------------------------------------------------------------------------*/
/* Test002: Destination IP address exact match against broadcast IP address  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest002 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest002()
	{
		m_name = "IpaFilteringBlockTest002";
		m_description =
		"Filtering block test 002 - Destination IP address exact match against broadcast IP address (Global Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules (MASK = 0xFF..FF). \
			All DST_IP == 127.0.0.1 traffic goes to routing table 0 \
			All DST_IP == 192.169.1.1 traffic goes to routing table 1 \
			All DST_IP == 192.169.1.2 traffic goes to routing table 2";
		m_maxIPAHwType = IPA_HW_v2_6L;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF; // Exact Match
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1.
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		int address;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80101);//192.168.1.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;
	}// ModifyPacktes ()
};



/*---------------------------------------------------------------------------*/
/* Test003: Destination UDP port exact match against DHCP port  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest003 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest003()
	{
		m_name = "IpaFilteringBlockTest003";
		m_description =
			"Filtering block test 003 - Destination UDP port exact match against DHCP port (Global Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules . \
			All DST_UDP_PORT == 546 (DHCP Client)traffic goes to routing table 0 \
			All DST_UDP_PORT == 547 (DHCP Server) traffic goes to routing table 1 \
			All DST_UDP_PORT == 500 (Non DHCP) traffic goes to routing table 2";
		m_maxIPAHwType = IPA_HW_v2_6L;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_PORT;
		flt_rule_entry.rule.attrib.dst_port = 546; // DHCP Client Port No 546

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.dst_port = 547; // DHCP Server Port No 547
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.dst_port = 500; // Non-DHCP Port

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		unsigned short port;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Port should be switched to Network Mode.
		port = ntohs(546);//DHCP Client Port
		memcpy (&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = ntohs(547);//DHCP Server Port
		memcpy (&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = ntohs(500);//Non - DHCP Port
		memcpy (&m_sendBuffer3[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		return true;
	}// ModifyPacktes ()
};

/*------------------------------------------------------------------------------*/
/* Test004: Firewall filtering rules based on source and destination port ranges  */
/*------------------------------------------------------------------------------*/
class IpaFilteringBlockTest004 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest004()
	{
		m_name = "IpaFilteringBlockTest004";
		m_description =
		"Filtering block test 004 - Firewall filtering rules based on source and destination port ranges (Global Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules . \
			All (5 >= SRC_PORT_RANGE >= 15) & (50 >= DST_PORT_RANGE >= 150) traffic goes to routing table 0 \
			All (15 >= SRC_PORT_RANGE >= 25) & (150 >= DST_PORT_RANGE >= 250) traffic goes to routing table 1 \
			All (25 >= SRC_PORT_RANGE >= 35) & (250 >= DST_PORT_RANGE >= 350) traffic goes to routing table 2";
		m_maxIPAHwType = IPA_HW_v2_6L;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_SRC_PORT_RANGE | IPA_FLT_DST_PORT_RANGE;
		//TODO: Fix from here.....
		flt_rule_entry.rule.attrib.src_port_lo =5;
		flt_rule_entry.rule.attrib.src_port_hi =15;
		flt_rule_entry.rule.attrib.dst_port_lo =50;
		flt_rule_entry.rule.attrib.dst_port_hi =150;

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.src_port_lo = 15;
		flt_rule_entry.rule.attrib.src_port_hi = 25;
		flt_rule_entry.rule.attrib.dst_port_lo = 150;
		flt_rule_entry.rule.attrib.dst_port_hi = 250;

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.src_port_lo = 25;
		flt_rule_entry.rule.attrib.src_port_hi = 35;
		flt_rule_entry.rule.attrib.dst_port_lo = 250;
		flt_rule_entry.rule.attrib.dst_port_hi = 350;

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		unsigned short port;

		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		port = htons(10);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(100);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = htons(20);
		memcpy(&m_sendBuffer2[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(200);
		memcpy(&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = htons(30);
		memcpy(&m_sendBuffer3[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(300);
		memcpy(&m_sendBuffer3[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		return true;
	}// ModifyPacktes ()
};
/*---------------------------------------------------------------------------*/
/* Test005: Destination IP address exact match against broadcast IP address  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest005 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest005()
	{
		m_name = "IpaFilteringBlockTest005";
		m_description =
		"Filtering block test 005 - Filtering Based on Protocol type (TCP/UDP/ICMP) (Global Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All UDP traffic goes to routing table 0 \
			All TCP traffic goes to routing table 1 \
			All ICMP traffic goes to routing table 2";
		m_maxIPAHwType = IPA_HW_v2_6L;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
//		Test Description:
//		1. Generate and commit two routing tables.
//			Each table will contain a single "bypass" rule (all data goes to output pipe 0 and 1 (accordingly))
//		2. Generate and commit two filtering rules.
//			All UDP traffic goes to routing table 1
//			All TCP traffic goes to routing table 2
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name,bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_PROTOCOL;
		flt_rule_entry.rule.attrib.u.v4.protocol = 17; // Filter only UDP Packets.
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.protocol = 6; // Filter only TCP Packets.
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.protocol = 1; // Filter only ICMP Packets.

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		m_sendBuffer[IPV4_PROTOCOL_OFFSET] = 0x11;// UDP 0x11 = 17
		m_sendBuffer2[IPV4_PROTOCOL_OFFSET] = 0x06;// TCP 0x06 = 6
		m_sendBuffer3[IPV4_PROTOCOL_OFFSET] = 0x01;// ICMP 0x01 = 1
		return true;
	}// ModifyPacktes ()
};


/*---------------------------------------------------------------------------*/
/* Test006: Destination IP address and subnet mask match against LAN subnet  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest006 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest006()
	{
		m_name = "IpaFilteringBlockTest006";
		m_description =
		"Filtering block test 006 - Destination IP address and subnet mask match against LAN subnet (Global Filtering Table, each rule is added in a Insert using a dedicated single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules: (DST & Mask Match). \
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 0 \
			All DST_IP == (192.169.1.1 & 255.0.0.255)traffic goes to routing table 1 \
			All DST_IP == (192.169.1.2 & 255.0.0.255)traffic goes to routing table 2";
		m_maxIPAHwType = IPA_HW_v2_6L;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0,FilterTable1,FilterTable2;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);
		FilterTable1.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);
		FilterTable2.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		// TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1 // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		// TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1.
		if (
				((uint8_t)-1 == FilterTable1.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable1.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable1.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2 // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		// TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable2.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable2.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable2.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable2.ReadRuleFromTable(0)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		int address;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80101);//192.168.1.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;
		return true;
	}// ModifyPacktes ()
};

/*---------------------------------------------------------------------------*/
/* Test007: Destination IP address exact match against broadcast IP address  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest007 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest007()
	{
		m_name = "IpaFilteringBlockTest007";
		m_description =
		"Filtering block test 007 - Destination IP address exact match against broadcast IP address (Global Filtering Table, each rule is added in a Insert using a dedicated single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules (MASK = 0xFF..FF). \
			All DST_IP == 127.0.0.1 traffic goes to routing table 0 \
			All DST_IP == 192.169.1.1 traffic goes to routing table 1 \
			All DST_IP == 192.169.1.2 traffic goes to routing table 2";
		m_maxIPAHwType = IPA_HW_v2_6L;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		//		Test Description:
		//		1. Generate and commit three routing tables.
		//			Each table will contain a single "bypass" rule (all data goes to output pipe 0, 1 and 2(accordingly))
		//		2. Generate and commit three filtering rules (each in different Filtering Table)
		//			All Filter DST_IP == 127.0.0.1 traffic goes to routing table 1
		//			All Filter DST_IP == 192.168.1.1 traffic goes to routing table 2
		//			All Filter DST_IP == 192.168.1.2 traffic goes to routing table 3

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2,sizeof(routing_table2.name) );
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0,FilterTable1,FilterTable2;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);
		FilterTable1.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);
		FilterTable2.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF; // Exact Match
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1.
		if (
				((uint8_t)-1 == FilterTable1.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable1.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable1.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable2.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable2.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable2.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable2.ReadRuleFromTable(0)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		int address;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80101);//192.168.1.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;
	}// ModifyPacktes ()
};



/*---------------------------------------------------------------------------*/
/* Test008: Destination UDP port exact match against DHCP port  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest008 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest008()
	{
		m_name = "IpaFilteringBlockTest008";
		m_description =
		"Filtering block test 008 - Destination UDP port exact match against DHCP port (Global Filtering Table, each rule is added in a Insert using a dedicated single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules . \
			All DST_UDP_PORT == 546 (DHCP Client)traffic goes to routing table 0 \
			All DST_UDP_PORT == 547 (DHCP Server) traffic goes to routing table 1 \
			All DST_UDP_PORT == 500 (Non DHCP) traffic goes to routing table 2";
		m_maxIPAHwType = IPA_HW_v2_6L;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
//		Test Description:
//		1. Generate and commit three routing tables.
//			Each table will contain a single "bypass" rule (all data goes to output pipe 0, 1  and 2 (accordingly))
//		2. Generate and commit Three filtering rules.
//			All DEST_IP == 127.0.0.1 traffic goes to routing table 0
//			All DEST_IP == 192.169.1.1 traffic goes to routing table 1
//			Non Matching traffic goes to routing table 3

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0,FilterTable1,FilterTable2;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);
		FilterTable1.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);
		FilterTable2.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_PORT;
		flt_rule_entry.rule.attrib.dst_port = 546; // DHCP Client Port No 546

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.dst_port = 547; // DHCP Server Port No 547
		if (
				((uint8_t)-1 == FilterTable1.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable1.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable1.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.dst_port = 500; // Non-DHCP Port

		if (
				((uint8_t)-1 == FilterTable2.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable2.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable2.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable2.ReadRuleFromTable(0)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		unsigned short port;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Port should be switched to Network Mode.
		port = ntohs(546);//DHCP Client Port
		memcpy (&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = ntohs(547);//DHCP Server Port
		memcpy (&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = ntohs(500);//Non - DHCP Port
		memcpy (&m_sendBuffer3[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		return true;
	}// ModifyPacktes ()
};

/*------------------------------------------------------------------------------*/
/* Test009: Firewall filtering rules based on source and destination port ranges  */
/*------------------------------------------------------------------------------*/
class IpaFilteringBlockTest009 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest009()
	{
		m_name = "IpaFilteringBlockTest009";
		m_description =
		"Filtering block test 009 - Firewall filtering rules based on source and destination port ranges (Global Filtering Table, each rule is added in a Insert using a dedicated single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules . \
			All (5 >= SRC_PORT_RANGE >= 15) & (50 >= DST_PORT_RANGE >= 150) traffic goes to routing table 0 \
			All (15 >= SRC_PORT_RANGE >= 25) & (150 >= DST_PORT_RANGE >= 250) traffic goes to routing table 1 \
			All (25 >= SRC_PORT_RANGE >= 35) & (250 >= DST_PORT_RANGE >= 350) traffic goes to routing table 2";
		m_maxIPAHwType = IPA_HW_v2_6L;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
//		Test Description:
//		1. Generate and commit two routing tables.
//			Each table will contain a single "bypass" rule (all data goes to output pipe 0 and 1 (accordingly))
//		2. Generate and commit two filtering rules.
//			All UDP traffic goes to routing table 1
//			All TCP traffic goes to routing table 2
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0,FilterTable1,FilterTable2;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);
		FilterTable1.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);
		FilterTable2.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_SRC_PORT_RANGE | IPA_FLT_DST_PORT_RANGE;
		//TODO: Fix from here.....
		flt_rule_entry.rule.attrib.src_port_lo =5;
		flt_rule_entry.rule.attrib.src_port_hi =15;
		flt_rule_entry.rule.attrib.dst_port_lo =50;
		flt_rule_entry.rule.attrib.dst_port_hi =150;

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.src_port_lo = 15;
		flt_rule_entry.rule.attrib.src_port_hi = 25;
		flt_rule_entry.rule.attrib.dst_port_lo = 150;
		flt_rule_entry.rule.attrib.dst_port_hi = 250;

		if (
				((uint8_t)-1 == FilterTable1.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable1.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable1.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.src_port_lo = 25;
		flt_rule_entry.rule.attrib.src_port_hi = 35;
		flt_rule_entry.rule.attrib.dst_port_lo = 250;
		flt_rule_entry.rule.attrib.dst_port_hi = 350;

		if (
				((uint8_t)-1 == FilterTable2.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable2.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable2.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable2.ReadRuleFromTable(0)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		unsigned short port;

		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		port = htons(10);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(100);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = htons(20);
		memcpy(&m_sendBuffer2[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(200);
		memcpy(&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = htons(30);
		memcpy(&m_sendBuffer3[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(300);
		memcpy(&m_sendBuffer3[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		return true;
	}// ModifyPacktes ()
};
/*---------------------------------------------------------------------------*/
/* Test010: Destination IP address exact match against broadcast IP address  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest010 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest010()
	{
		m_name = "IpaFilteringBlockTest010";
		m_description =
		"Filtering block test 010 - Filtering Based on Protocol type (TCP/UDP/ICMP) (Global Filtering Table, each rule is added in a Insert using a dedicated single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All UDP traffic goes to routing table 0 \
			All TCP traffic goes to routing table 1 \
			All ICMP traffic goes to routing table 2";
		m_maxIPAHwType = IPA_HW_v2_6L;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
//		Test Description:
//		1. Generate and commit two routing tables.
//			Each table will contain a single "bypass" rule (all data goes to output pipe 0 and 1 (accordingly))
//		2. Generate and commit two filtering rules.
//			All UDP traffic goes to routing table 1
//			All TCP traffic goes to routing table 2
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0,FilterTable1,FilterTable2;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);
		FilterTable1.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);
		FilterTable2.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,true,1);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_PROTOCOL;
		flt_rule_entry.rule.attrib.u.v4.protocol = 17; // Filter only UDP Packets.
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.protocol = 6; // Filter only TCP Packets.
		if (
				((uint8_t)-1 == FilterTable1.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable1.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable1.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.protocol = 1; // Filter only ICMP Packets.

		if (
				((uint8_t)-1 == FilterTable2.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable2.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable2.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable2.ReadRuleFromTable(0)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		m_sendBuffer[IPV4_PROTOCOL_OFFSET] = 0x11;// UDP 0x11 = 17
		m_sendBuffer2[IPV4_PROTOCOL_OFFSET] = 0x06;// TCP 0x06 = 6
		m_sendBuffer3[IPV4_PROTOCOL_OFFSET] = 0x01;// ICMP 0x01 = 1
		return true;
	}// ModifyPacktes ()
};


/*---------------------------------------------------------------------------*/
/* Test021: Destination IP address and subnet mask match against LAN subnet  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest021 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest021()
	{
		m_name = "IpaFilteringBlockTest021";
		m_description =
		"Filtering block test 021 - Destination IP address and subnet mask match against LAN subnet (End-Point specific Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules: (DST & Mask Match). \
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 0 \
			All DST_IP == (192.169.1.1 & 255.0.0.255)traffic goes to routing table 1 \
			All DST_IP == (192.169.1.2 & 255.0.0.255)traffic goes to routing table 2";
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		printf("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}
		printf("%s route table handle = %u\n", bypass1, routing_table1.hdl);

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}
		printf("%s route table handle = %u\n", bypass2, routing_table2.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		// TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1 // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		// TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1.
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2 // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		// TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl2=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		int address;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80101);//192.168.1.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;
	}// ModifyPacktes ()
};

/*---------------------------------------------------------------------------*/
/* Test022: Destination IP address exact match against broadcast IP address  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest022 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest022()
	{
		m_name = "IpaFilteringBlockTest022";
		m_description =
		"Filtering block test 022 - Destination IP address exact match against broadcast IP address (End-Point specific Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules (MASK = 0xFF..FF). \
			All DST_IP == 127.0.0.1 traffic goes to routing table 0 \
			All DST_IP == 192.169.1.1 traffic goes to routing table 1 \
			All DST_IP == 192.169.1.2 traffic goes to routing table 2";
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF; // Exact Match
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1.
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else {
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl2=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		int address;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80101);//192.168.1.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;
	}// ModifyPacktes ()
};



/*---------------------------------------------------------------------------*/
/* Test023: Destination UDP port exact match against DHCP port 				 */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest023 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest023()
	{
		m_name = "IpaFilteringBlockTest023";
		m_description =
		"Filtering block test 023 - Destination UDP port exact match against DHCP port (End-Point specific Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules . \
			All DST_UDP_PORT == 546 (DHCP Client)traffic goes to routing table 0 \
			All DST_UDP_PORT == 547 (DHCP Server) traffic goes to routing table 1 \
			All DST_UDP_PORT == 500 (Non DHCP) traffic goes to routing table 2";
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_PORT;
		flt_rule_entry.rule.attrib.dst_port = 546; // DHCP Client Port No 546

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.dst_port = 547; // DHCP Server Port No 547
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.dst_port = 500; // Non-DHCP Port

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else {
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl2=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		unsigned short port;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Port should be switched to Network Mode.
		port = ntohs(546);//DHCP Client Port
		memcpy (&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = ntohs(547);//DHCP Server Port
		memcpy (&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = ntohs(500);//Non - DHCP Port
		memcpy (&m_sendBuffer3[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		return true;
	}// ModifyPacktes ()
};

/*------------------------------------------------------------------------------*/
/* Test004: Firewall filtering rules based on source and destination port ranges*/
/*------------------------------------------------------------------------------*/
class IpaFilteringBlockTest024 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest024()
	{
		m_name = "IpaFilteringBlockTest024";
		m_description =
		"Filtering block test 024 - Firewall filtering rules based on source and destination port ranges (End-Point specific Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules . \
			All (5 >= SRC_PORT_RANGE >= 15) & (50 >= DST_PORT_RANGE >= 150) traffic goes to routing table 0 \
			All (15 >= SRC_PORT_RANGE >= 25) & (150 >= DST_PORT_RANGE >= 250) traffic goes to routing table 1 \
			All (25 >= SRC_PORT_RANGE >= 35) & (250 >= DST_PORT_RANGE >= 350) traffic goes to routing table 2";
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_SRC_PORT_RANGE | IPA_FLT_DST_PORT_RANGE;
		//TODO: Fix from here.....
		flt_rule_entry.rule.attrib.src_port_lo =5;
		flt_rule_entry.rule.attrib.src_port_hi =15;
		flt_rule_entry.rule.attrib.dst_port_lo =50;
		flt_rule_entry.rule.attrib.dst_port_hi =150;

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.src_port_lo = 15;
		flt_rule_entry.rule.attrib.src_port_hi = 25;
		flt_rule_entry.rule.attrib.dst_port_lo = 150;
		flt_rule_entry.rule.attrib.dst_port_hi = 250;

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.src_port_lo = 25;
		flt_rule_entry.rule.attrib.src_port_hi = 35;
		flt_rule_entry.rule.attrib.dst_port_lo = 250;
		flt_rule_entry.rule.attrib.dst_port_hi = 350;

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl2=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		unsigned short port;

		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		port = htons(10);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(100);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = htons(20);
		memcpy(&m_sendBuffer2[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(200);
		memcpy(&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = htons(30);
		memcpy(&m_sendBuffer3[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(300);
		memcpy(&m_sendBuffer3[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		return true;
	}// ModifyPacktes ()
};
/*---------------------------------------------------------------------------*/
/* Test005: Destination IP address exact match against broadcast IP address  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest025 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest025()
	{
		m_name = "IpaFilteringBlockTest025";
		m_description =
				"Filtering block test 025 - Filtering Based on Protocol type (TCP/UDP/ICMP) (End-Point specific Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All UDP traffic goes to routing table 0 \
			All TCP traffic goes to routing table 1 \
			All ICMP traffic goes to routing table 2";
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
//		Test Description:
//		1. Generate and commit two routing tables.
//			Each table will contain a single "bypass" rule (all data goes to output pipe 0 and 1 (accordingly))
//		2. Generate and commit two filtering rules.
//			All UDP traffic goes to routing table 1
//			All TCP traffic goes to routing table 2
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_PROTOCOL;
		flt_rule_entry.rule.attrib.u.v4.protocol = 17; // Filter only UDP Packets.
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.protocol = 6; // Filter only TCP Packets.
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.protocol = 1; // Filter only ICMP Packets.

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl2=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		m_sendBuffer[IPV4_PROTOCOL_OFFSET] = 0x11;// UDP 0x11 = 17
		m_sendBuffer2[IPV4_PROTOCOL_OFFSET] = 0x06;// TCP 0x06 = 6
		m_sendBuffer3[IPV4_PROTOCOL_OFFSET] = 0x01;// ICMP 0x01 = 1
		return true;
	}// ModifyPacktes ()
};


/*---------------------------------------------------------------------------*/
/* Test006: Destination IP address and subnet mask match against LAN subnet  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest026 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest026()
	{
		m_name = "IpaFilteringBlockTest026";
		m_description =
				"Filtering block test 026 - Destination IP address and subnet mask match against LAN subnet (End-Point specific Filtering Table, each rule is added in a Insert using a dedicated single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules: (DST & Mask Match). \
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 0 \
			All DST_IP == (192.169.1.1 & 255.0.0.255)traffic goes to routing table 1 \
			All DST_IP == (192.169.1.2 & 255.0.0.255)traffic goes to routing table 2";
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0,FilterTable1,FilterTable2;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);
		FilterTable1.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);
		FilterTable2.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		// TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1 // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		// TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1.
		if (
				((uint8_t)-1 == FilterTable1.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable1.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable1.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2 // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		// TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable2.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable2.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable2.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable2.ReadRuleFromTable(0)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		int address;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80101);//192.168.1.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;
	}// ModifyPacktes ()
};

/*---------------------------------------------------------------------------*/
/* Test007: Destination IP address exact match against broadcast IP address  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest027 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest027()
	{
		m_name = "IpaFilteringBlockTest027";
		m_description =
		"Filtering block test 027 - Destination IP address exact match against broadcast IP address (End-Point specific Filtering Table, each rule is added in a Insert using a dedicated single commit) \
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules (MASK = 0xFF..FF). \
			All DST_IP == 127.0.0.1 traffic goes to routing table 0 \
			All DST_IP == 192.169.1.1 traffic goes to routing table 1 \
			All DST_IP == 192.169.1.2 traffic goes to routing table 2";
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		//		Test Description:
		//		1. Generate and commit three routing tables.
		//			Each table will contain a single "bypass" rule (all data goes to output pipe 0, 1 and 2(accordingly))
		//		2. Generate and commit three filtering rules (each in different Filtering Table)
		//			All Filter DST_IP == 127.0.0.1 traffic goes to routing table 1
		//			All Filter DST_IP == 192.168.1.1 traffic goes to routing table 2
		//			All Filter DST_IP == 192.168.1.2 traffic goes to routing table 3

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0,FilterTable1,FilterTable2;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);
		FilterTable1.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);
		FilterTable2.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // TODO: Fix this, doesn't match the Rule's Requirements
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF; // Exact Match
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1.
		if (
				((uint8_t)-1 == FilterTable1.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable1.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable1.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable2.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable2.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable2.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable2.ReadRuleFromTable(0)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		int address;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80101);//192.168.1.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;
	}// ModifyPacktes ()
};



/*---------------------------------------------------------------------------*/
/* Test008: Destination UDP port exact match against DHCP port  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest028 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest028()
	{
		m_name = "IpaFilteringBlockTest028";
		m_description =
		"Filtering block test 028 - Destination UDP port exact match against DHCP port (End-Point specific Filtering Table, each rule is added in a Insert using a dedicated single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules . \
			All DST_UDP_PORT == 546 (DHCP Client)traffic goes to routing table 0 \
			All DST_UDP_PORT == 547 (DHCP Server) traffic goes to routing table 1 \
			All DST_UDP_PORT == 500 (Non DHCP) traffic goes to routing table 2";
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
//		Test Description:
//		1. Generate and commit three routing tables.
//			Each table will contain a single "bypass" rule (all data goes to output pipe 0, 1  and 2 (accordingly))
//		2. Generate and commit Three filtering rules.
//			All DEST_IP == 127.0.0.1 traffic goes to routing table 0
//			All DEST_IP == 192.169.1.1 traffic goes to routing table 1
//			Non Matching traffic goes to routing table 3

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0,FilterTable1,FilterTable2;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);
		FilterTable1.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);
		FilterTable2.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_PORT;
		flt_rule_entry.rule.attrib.dst_port = 546; // DHCP Client Port No 546

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.dst_port = 547; // DHCP Server Port No 547
		if (
				((uint8_t)-1 == FilterTable1.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable1.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable1.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.dst_port = 500; // Non-DHCP Port

		if (
				((uint8_t)-1 == FilterTable2.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable2.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable2.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable2.ReadRuleFromTable(0)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		unsigned short port;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Port should be switched to Network Mode.
		port = ntohs(546);//DHCP Client Port
		memcpy (&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = ntohs(547);//DHCP Server Port
		memcpy (&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = ntohs(500);//Non - DHCP Port
		memcpy (&m_sendBuffer3[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		return true;
	}// ModifyPacktes ()
};

/*------------------------------------------------------------------------------*/
/* Test009: Firewall filtering rules based on source and destination port ranges  */
/*------------------------------------------------------------------------------*/
class IpaFilteringBlockTest029 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest029()
	{
		m_name = "IpaFilteringBlockTest029";
		m_description =
		"Filtering block test 029 - Firewall filtering rules based on source and destination port ranges (End-Point specific Filtering Table, each rule is added in a Insert using a dedicated single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules . \
			All (5 >= SRC_PORT_RANGE >= 15) & (50 >= DST_PORT_RANGE >= 150) traffic goes to routing table 0 \
			All (15 >= SRC_PORT_RANGE >= 25) & (150 >= DST_PORT_RANGE >= 250) traffic goes to routing table 1 \
			All (25 >= SRC_PORT_RANGE >= 35) & (250 >= DST_PORT_RANGE >= 350) traffic goes to routing table 2";
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
//		Test Description:
//		1. Generate and commit two routing tables.
//			Each table will contain a single "bypass" rule (all data goes to output pipe 0 and 1 (accordingly))
//		2. Generate and commit two filtering rules.
//			All UDP traffic goes to routing table 1
//			All TCP traffic goes to routing table 2
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0,FilterTable1,FilterTable2;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);
		FilterTable1.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);
		FilterTable2.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_SRC_PORT_RANGE | IPA_FLT_DST_PORT_RANGE;
		//TODO: Fix from here.....
		flt_rule_entry.rule.attrib.src_port_lo =5;
		flt_rule_entry.rule.attrib.src_port_hi =15;
		flt_rule_entry.rule.attrib.dst_port_lo =50;
		flt_rule_entry.rule.attrib.dst_port_hi =150;

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.src_port_lo = 15;
		flt_rule_entry.rule.attrib.src_port_hi = 25;
		flt_rule_entry.rule.attrib.dst_port_lo = 150;
		flt_rule_entry.rule.attrib.dst_port_hi = 250;

		if (
				((uint8_t)-1 == FilterTable1.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable1.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable1.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.src_port_lo = 25;
		flt_rule_entry.rule.attrib.src_port_hi = 35;
		flt_rule_entry.rule.attrib.dst_port_lo = 250;
		flt_rule_entry.rule.attrib.dst_port_hi = 350;

		if (
				((uint8_t)-1 == FilterTable2.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable2.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable2.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable2.ReadRuleFromTable(0)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		unsigned short port;

		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		port = htons(10);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(100);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = htons(20);
		memcpy(&m_sendBuffer2[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(200);
		memcpy(&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));
		port = htons(30);
		memcpy(&m_sendBuffer3[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(300);
		memcpy(&m_sendBuffer3[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		return true;
	}// ModifyPacktes ()
};
/*---------------------------------------------------------------------------*/
/* Test010: Destination IP address exact match against broadcast IP address  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest030 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest030()
	{
		m_name = "IpaFilteringBlockTest030";
		m_description =
		"Filtering block test 030 - Filtering Based on Protocol type (TCP/UDP/ICMP) (End-Point specific Filtering Table, each rule is added in a Insert using a dedicated single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All UDP traffic goes to routing table 0 \
			All TCP traffic goes to routing table 1 \
			All ICMP traffic goes to routing table 2";
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
//		Test Description:
//		1. Generate and commit two routing tables.
//			Each table will contain a single "bypass" rule (all data goes to output pipe 0 and 1 (accordingly))
//		2. Generate and commit two filtering rules.
//			All UDP traffic goes to routing table 1
//			All TCP traffic goes to routing table 2
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0,FilterTable1,FilterTable2;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);
		FilterTable1.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);
		FilterTable2.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,1);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_PROTOCOL;
		flt_rule_entry.rule.attrib.u.v4.protocol = 17; // Filter only UDP Packets.
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.protocol = 6; // Filter only TCP Packets.
		if (
				((uint8_t)-1 == FilterTable1.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable1.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable1.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.protocol = 1; // Filter only ICMP Packets.

		if (
				((uint8_t)-1 == FilterTable2.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable2.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable2.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable2.ReadRuleFromTable(0)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		m_sendBuffer[IPV4_PROTOCOL_OFFSET] = 0x11;// UDP 0x11 = 17
		m_sendBuffer2[IPV4_PROTOCOL_OFFSET] = 0x06;// TCP 0x06 = 6
		m_sendBuffer3[IPV4_PROTOCOL_OFFSET] = 0x01;// ICMP 0x01 = 1
		return true;
	}// ModifyPacktes ()
};
/*-------------------------------------------------------------------------------------*/
/* Test031: Filtering Based on fragment extension, End-Point specific Filtering Table  */
/*-------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest031 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest031()
	{
		m_name = "IpaFilteringBlockTest031";
		m_description =
				"Filtering block test 031 - Filtering Based on fragment extension(End-Point specific Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit 2 filtering rules: \
			All fragmented packets goes to routing table 0: \
			Packets with MF flag set & \
			Packets with MF flag set to zero and fragment offset field nonzero \
			All other packets(non fragmented) goes to routing table 1: \
			Packets with MF flag set to zero and fragment offset field zero goes to routing table 1";
		Register(*this);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables(bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_FRAGMENT;
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xaabbccdd;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0x00000000;// All Packets will get a "Hit"

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		m_sendBuffer[IPV4_FRAGMENT_FLAGS_OFFSET] = 0x20;//MF=1
		m_sendBuffer[IPV4_FRAGMENT_FLAGS_OFFSET+1] = 0x0;//MF=1
		m_sendBuffer2[IPV4_FRAGMENT_FLAGS_OFFSET] = 0x0;//MF=0 && frag_off =126
		m_sendBuffer2[IPV4_FRAGMENT_FLAGS_OFFSET+1] = 0x7E;//MF=0 && frag_off =126
		m_sendBuffer3[IPV4_FRAGMENT_FLAGS_OFFSET] = 0x0;// MF=0 && frag_off =0
		m_sendBuffer3[IPV4_FRAGMENT_FLAGS_OFFSET+1] = 0x0;// MF=0 && frag_off =0
		return true;
	}// ModifyPacktes ()

	virtual bool ReceivePacketsAndCompare()
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

		receivedSize3 = m_consumer2.ReceiveData(rxBuff3, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize3, m_consumer2.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGolden(m_sendBuffer, m_sendSize, rxBuff1, receivedSize))
		{
			printf("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize - 1, sentBufferSize - 1, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		recievedBuffer[0] = '\0';
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize - 1, sentBufferSize - 1, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		recievedBuffer[0] = '\0';
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize - 1, sentBufferSize - 1, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		isSuccess &= CompareResultVsGolden(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		isSuccess &= CompareResultVsGolden(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test050: Destination IPv6 address and Subnet Mask exact match against broadcast IP address  */
/*---------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest050 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest050()
	{
		m_name = "IpaFilteringBlockTest050";
		m_description =
		"Filtering block test 050 - Destination IPv6 address and Mask exact match against broadcast IP address (Global Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IPv6 == 0x...AA traffic goes to routing table 1 \
			All DST_IPv6 == 0x...BB traffic goes to routing table 2 \
			All DST_IPv6 == 0x...CC traffic goes to routing table 3";
		m_IpaIPType = IPA_IP_v6;
		m_maxIPAHwType = IPA_HW_v2_6L;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv6BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v6;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v6;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v6;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,true,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // TODO: Fix this, doesn't match the Rule's Requirements

		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0x000000FF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr[0]		 = 0XFF020000; // Filter DST_IP
		flt_rule_entry.rule.attrib.u.v6.dst_addr[1]		 = 0x00000000;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[2]      = 0x11223344;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677AA;

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677BB;
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677CC;

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xBB;
		m_sendBuffer3[DST_ADDR_LSB_OFFSET_IPV6] = 0xCC;
		return true;
	}// ModifyPacktes ()
};


/*---------------------------------------------------------------------------------------------*/
/* Test051: Destination IPv6 address and Subnet Mask exact match against broadcast IP address  */
/*---------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest051 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest051()
	{
		m_name = "IpaFilteringBlockTest051";
		m_description =
		"Filtering block test 051 - Destination IPv6 address and Mask exact match against broadcast IP address (End-Point specific Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IPv6 == 0x...AA traffic goes to routing table 1 \
			All DST_IPv6 == 0x...BB traffic goes to routing table 2 \
			All DST_IPv6 == 0x...CC traffic goes to routing table 3";
		m_IpaIPType = IPA_IP_v6;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv6BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v6;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v6;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v6;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // TODO: Fix this, doesn't match the Rule's Requirements

		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0x000000FF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr[0]      = 0XFF020000; // Filter DST_IP
		flt_rule_entry.rule.attrib.u.v6.dst_addr[1]      = 0x00000000;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[2]      = 0x11223344;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677AA;

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}
		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677BB;
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677CC;

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl2=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xBB;
		m_sendBuffer3[DST_ADDR_LSB_OFFSET_IPV6] = 0xCC;
		return true;
	}// ModifyPacktes ()
};

/*---------------------------------------------------------------------------*/
/* Test052: IPv6 Filtering Based on Protocol type (TCP/UDP/ICMP)  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest052 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest052()
	{
		m_name = "IpaFilteringBlockTest052";
		m_description =
		"Filtering block test 052 - Filtering Based on Protocol type (TCP/UDP/ICMP) (Global Filtering Table, each rule is added in a Insert using a dedicated single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All UDP traffic goes to routing table 0 \
			All TCP traffic goes to routing table 1 \
			All ICMP traffic goes to routing table 2";
		m_IpaIPType = IPA_IP_v6;
		m_extHdrType = FRAGMENT;
		m_minIPAHwType = IPA_HW_v2_5;
		m_maxIPAHwType = IPA_HW_v2_6L;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv6BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v6;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v6;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v6;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0,FilterTable1,FilterTable2;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,true,1);
		FilterTable1.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,true,1);
		FilterTable2.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,true,1);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_NEXT_HDR;
		flt_rule_entry.rule.attrib.u.v6.next_hdr = 17; // Filter only UDP Packets.
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
		{
			printf ("%s::Error Adding RuleTable(0) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v6.next_hdr = 6; // Filter only TCP Packets.
		if (
				((uint8_t)-1 == FilterTable1.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable1.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(1) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable1.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v6.next_hdr = 1; // Filter only ICMP Packets.

		if (
				((uint8_t)-1 == FilterTable2.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable2.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable2.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable2.ReadRuleFromTable(0)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		m_sendBuffer[IPV6_NEXT_HDR_OFFSET] = 0x2C;//FRAGMENT HEADER(44)
		m_sendBuffer[IPV6_FRAGMENT_NEXT_HDR_OFFSET] = 0x11;// UDP 0x11 = 17
		m_sendBuffer2[IPV6_FRAGMENT_NEXT_HDR_OFFSET] = 0x06;// TCP 0x06 = 6
		m_sendBuffer3[IPV6_FRAGMENT_NEXT_HDR_OFFSET] = 0x01;// ICMP 0x01 = 1
		return true;
	}// ModifyPacktes ()
};
/*-------------------------------------------------------------------------------------*/
/* Test053: Filtering Based on fragment extension, End-Point specific Filtering Table  */
/*-------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest053 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest053()
	{
		m_name = "IpaFilteringBlockTest053";
		m_description =
				"Filtering block test 053 - Filtering Based on fragment extension(End-Point specific Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit 2 filtering rules: \
			All fragmented packets goes to routing table 0: \
			Packets with MF flag set & \
			Packets with MF flag set to zero and fragment offset field nonzero \
			All other packets(non fragmented) goes to routing table 1: \
			Packets with MF flag set to zero and fragment offset field zero goes to routing table 1";
		m_IpaIPType = IPA_IP_v6;
		m_extHdrType = FRAGMENT;
		m_minIPAHwType = IPA_HW_v2_5;
		Register(*this);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv6BypassRoutingTables(bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v6;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v6;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v6;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,false,2);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_FRAGMENT;
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[0] = 0xaabbccdd;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[1] = 0xeeff0011;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[2] = 0x22334455;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3] = 0x66778899;
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0x00000000;// All Packets will get a "Hit"
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0x00000000;
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0x00000000;
		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		m_sendBuffer[IPV6_NEXT_HDR_OFFSET] = 0x2C;//FRAGMENT HEADER(44)
		m_sendBuffer[IPV6_FRAGMENT_FLAGS_OFFSET] = 0x00;//MF=1
		m_sendBuffer[IPV6_FRAGMENT_FLAGS_OFFSET+1] = 0x1;//MF=1
		m_sendBuffer2[IPV6_FRAGMENT_FLAGS_OFFSET] = 0x3;//MF=0 && frag_off =126
		m_sendBuffer2[IPV6_FRAGMENT_FLAGS_OFFSET+1] = 0xF0;//MF=0 && frag_off =126
		m_sendBuffer3[IPV6_FRAGMENT_FLAGS_OFFSET] = 0x0;// MF=0 && frag_off =0
		m_sendBuffer3[IPV6_FRAGMENT_FLAGS_OFFSET+1] = 0x0;// MF=0 && frag_off =0
		return true;
	}// ModifyPacktes ()

	virtual bool ReceivePacketsAndCompare()
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

		receivedSize3 = m_consumer2.ReceiveData(rxBuff3, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize3, m_consumer2.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGolden(m_sendBuffer, m_sendSize, rxBuff1, receivedSize))
		{
			printf("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize - 1, sentBufferSize - 1, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		recievedBuffer[0] = '\0';
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize - 1, sentBufferSize - 1, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		recievedBuffer[0] = '\0';
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize - 1, sentBufferSize - 1, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		isSuccess &= CompareResultVsGolden(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		isSuccess &= CompareResultVsGolden(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}
};

/*----------------------------------------------------------------------------------------------*/
/* Test054: IPV6 filtering based on based on source and destination port  			*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest054 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest054()
	{
		m_name = "IpaFilteringBlockTest054";
		m_description =
		"Filtering block test 054 - IPV6 filtering rules based on source and destination port (End-Point specific Filtering Table, Insert all rules in a single commit)\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules:\
			All (SRC_PORT = 1000) traffic goes to routing table 0 \
			All (DST_PORT = 100) traffic goes to routing table 1 \
			All (5 >= SRC_PORT_RANGE >= 15) traffic goes to routing table 2";
		m_IpaIPType = IPA_IP_v6;
		m_minIPAHwType = IPA_HW_v2_5;
		Register(*this);
	}


	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv6BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v6;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v6;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v6;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_SRC_PORT;
		flt_rule_entry.rule.attrib.src_port = 1000;

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_PORT;
		flt_rule_entry.rule.attrib.dst_port = 100;

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_SRC_PORT_RANGE;
		flt_rule_entry.rule.attrib.src_port_lo = 5;
		flt_rule_entry.rule.attrib.src_port_hi = 15;

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl2=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		unsigned short port;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}

		port = htons(1000);
		memcpy(&m_sendBuffer[IPV6_SRC_PORT_OFFSET], &port, sizeof(port));
		port = htons(100);
		memcpy(&m_sendBuffer2[IPV6_DST_PORT_OFFSET], &port, sizeof(port));
		port = htons(10);
		memcpy(&m_sendBuffer3[IPV6_SRC_PORT_OFFSET], &port, sizeof(port));

		return true;
	}// ModifyPacktes ()
};

/*----------------------------------------------------------------------------------------------*/
/* Test060: IPV4 filtering test, non hashed priority higher than hashed priority				*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest060 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest060()
	{
		m_name = "IpaFilteringBlockTest060";
		m_description =
		"Filtering block test 060 - Rules prioritization hashable vs non-hashable rule, both rules match the same packet but non hashable has higher priority\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 0 - non hashable\
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 1 - hashable\
			All DST_IP == (192.169.1.2 & 255.0.0.255)traffic goes to routing table 2 - don't care for this specific test";
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);

	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass1, routing_table1.hdl);

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass2, routing_table2.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; //
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		flt_rule_entry.rule.hashable = 0; // non hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1 on lower priority (second in list)
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		flt_rule_entry.rule.hashable = 1; // hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.hashable = 0; // non hashed
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;

	}

	virtual bool ModifyPackets()
	{
		int address;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;

	}

	virtual bool ReceivePacketsAndCompare()
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

		// Compare results
		if (!CompareResultVsGolden(m_sendBuffer, m_sendSize, rxBuff1, receivedSize))
		{
			printf("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize - 1, sentBufferSize - 1, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		recievedBuffer[0] = '\0';
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize - 1, sentBufferSize - 1, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		recievedBuffer[0] = '\0';
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize - 1, sentBufferSize - 1, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		isSuccess &= CompareResultVsGolden(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		isSuccess &= CompareResultVsGolden(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}

};

/*----------------------------------------------------------------------------------------------*/
/* Test061: IPV4 filtering test, hashed priority higher than non hashed priority + cache hit	*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest061 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest061()
	{
		m_name = "IpaFilteringBlockTest061";
		m_description =
		"Filtering block test 061 - Rules prioritization hashable vs non-hashable rule, both rules match the same packet but hashable has higher priority\
		two identical packets are sent and should be catched by the hashable rule, second one shuld be hit the cache\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 0 - hashable\
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 1 - non hashable\
			All DST_IP == (192.169.1.2 & 255.0.0.255)traffic goes to routing table 2 - don't care for this specific test";
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);

	}

	bool Setup()
	{
		/* we want statuses on this test */
		return IpaFilteringBlockTestFixture::Setup(true);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass1, routing_table1.hdl);

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass2, routing_table2.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; //
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		flt_rule_entry.rule.hashable = 1; // hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1 on lower priority (second in list)
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		flt_rule_entry.rule.hashable = 0; // non hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 3
		flt_rule_entry.rule.hashable = 0; // non hashed
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets()
	{
		int address;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;

	}

	virtual bool ReceivePacketsAndCompare()
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

		// Compare results
		if (!CompareResultVsGolden_w_Status(m_sendBuffer, m_sendSize, rxBuff1, receivedSize))
		{
			printf("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize,receivedSize,rxBuff1);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);


		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ? 
			IsCacheHit_v5_0(m_sendSize2, receivedSize2, rxBuff2) : IsCacheHit(m_sendSize2,receivedSize2,rxBuff2);

		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize3, receivedSize3, rxBuff3) : IsCacheMiss(m_sendSize3,receivedSize3,rxBuff3);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}

};

/*----------------------------------------------------------------------------------------------*/
/* Test062: IPV4 filtering test, hashed rule match, non hash doesn't match expect cache miss	*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest062 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest062()
	{
		m_name = "IpaFilteringBlockTest062";
		m_description =
		"Filtering block test 062 - Rules prioritization hashable vs non-hashable rule, only hashable matches the packets\
		two packets with different tuple are sent and should match the hashable rule, no cache hit expected\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IP == (127.0.0.2 & 255.0.0.255)traffic goes to routing table 0 - non hashable\
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 1 - hashable\
			All DST_IP == (192.169.1.2 & 255.0.0.255)traffic goes to routing table 2 - don't care for this specific test\
		3. send three packets:\
			DST_IP == 127.0.0.1 port 546\
			DST_IP == 127.0.0.1 port 547\
			DST_IP == 192.168.1.2";
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);
	}

	bool Setup()
	{
		/* we want statuses on this test */
		return IpaFilteringBlockTestFixture::Setup(true);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass1, routing_table1.hdl);

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass2, routing_table2.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; //
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000002; // Filter DST_IP == 127.0.0.2.
		flt_rule_entry.rule.hashable = 0; // non hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1 on lower priority (second in list)
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		flt_rule_entry.rule.hashable = 1; // hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 3
		flt_rule_entry.rule.hashable = 0; // non hashed
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;

	}

	virtual bool ModifyPackets()
	{
		int address;
		unsigned short port;

		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = ntohs(546);//DHCP Client Port
		memcpy (&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = ntohs(547);//DHCP Server Port
		memcpy (&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;

	}

	virtual bool ReceivePacketsAndCompare()
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

		// Compare results
		if (!CompareResultVsGolden_w_Status(m_sendBuffer, m_sendSize, rxBuff1, receivedSize))
		{
			printf("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize,receivedSize,rxBuff1);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize2, receivedSize2, rxBuff2) : IsCacheMiss(m_sendSize2,receivedSize2,rxBuff2);

		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize3, receivedSize3, rxBuff3) : IsCacheMiss(m_sendSize3,receivedSize3,rxBuff3);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}
};

/*----------------------------------------------------------------------------------------------*/
/* Test063: IPV4 filtering test, hashed rule match, non hash doesn't match expect cache miss 	*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest063 : public IpaFilteringBlockTestFixture
{
public:

	IpaFilteringBlockTest063()
	{
		m_name = "IpaFilteringBlockTest063";
		m_description =
		"Filtering block test 063 - Rules prioritization hashable vs non-hashable rule, only hashable matches the packets\
		two packets with different tuple are sent and should match the hashable rule, no cache hit expected\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 0 - hashable\
			All DST_IP == (127.0.0.2 & 255.0.0.255)traffic goes to routing table 1 - non hashable\
			All DST_IP == (192.169.1.2 & 255.0.0.255)traffic goes to routing table 2 - don't care for this specific test\
		3. send three packets:\
			DST_IP == 127.0.0.1 port 546\
			DST_IP == 127.0.0.1 port 547\
			DST_IP == 192.168.1.2";
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);
	}

	bool Setup()
	{
		/* we want statuses on this test */
		return IpaFilteringBlockTestFixture::Setup(true);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass1, routing_table1.hdl);

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass2, routing_table2.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; //
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		flt_rule_entry.rule.hashable = 1; // hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1 on lower priority (second in list)
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000002; // Filter DST_IP == 127.0.0.2.
		flt_rule_entry.rule.hashable = 0; // non hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 3
		flt_rule_entry.rule.hashable = 0; // non hashed
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets()
	{
		int address;
		unsigned short port;

		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = ntohs(546);//DHCP Client Port
		memcpy (&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = ntohs(547);//DHCP Server Port
		memcpy (&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;

	}

	virtual bool ReceivePacketsAndCompare()
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

		// Compare results
		if (!CompareResultVsGolden_w_Status(m_sendBuffer, m_sendSize, rxBuff1, receivedSize))
		{
			printf("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize,receivedSize,rxBuff1);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);


		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize2, receivedSize2, rxBuff2) : IsCacheMiss(m_sendSize2,receivedSize2,rxBuff2);

		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize3, receivedSize3, rxBuff3) : IsCacheMiss(m_sendSize3,receivedSize3,rxBuff3);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}
};


/*----------------------------------------------------------------------------------------------*/
/* Test064: IPV4 filtering test, hashed rule match, non hash doesn't match expect cache hit 	*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest064 : public IpaFilteringBlockTestFixture
{
public:

	IpaFilteringBlockTest064()
	{
		m_name = "IpaFilteringBlockTest064";
		m_description =
		"Filtering block test 064 - Rules prioritization hashable vs non-hashable rule, only hashable matches the packets\
		two identical packets are sent and should match the hashable rule, cache hit expected\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IP == (127.0.0.2 & 255.0.0.255)traffic goes to routing table 0 - non hashable\
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 1 - hashable\
			All DST_IP == (192.169.1.2 & 255.0.0.255)traffic goes to routing table 2 - don't care for this specific test\
		3. send three packets:\
			DST_IP == 127.0.0.1 \
			DST_IP == 127.0.0.1 \
			DST_IP == 192.168.1.2";
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);
	}

	bool Setup()
	{
		/* we want statuses on this test */
		return IpaFilteringBlockTestFixture::Setup(true);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass1, routing_table1.hdl);

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass2, routing_table2.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; //
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000002; // Filter DST_IP == 127.0.0.2.
		flt_rule_entry.rule.hashable = 0; // non hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1 on lower priority (second in list)
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		flt_rule_entry.rule.hashable = 1; // hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 3
		flt_rule_entry.rule.hashable = 0; // non hashed
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets()
	{
		int address;

		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;

	}

	virtual bool ReceivePacketsAndCompare()
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

		// Compare results
		if (!CompareResultVsGolden_w_Status(m_sendBuffer, m_sendSize, rxBuff1, receivedSize))
		{
			printf("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize,receivedSize,rxBuff1);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ? 
			IsCacheHit_v5_0(m_sendSize2, receivedSize2, rxBuff2) : IsCacheHit(m_sendSize2,receivedSize2,rxBuff2);

		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize3, receivedSize3, rxBuff3) : IsCacheMiss(m_sendSize3,receivedSize3,rxBuff3);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}

};

/*----------------------------------------------------------------------------------------------*/
/* Test065: IPV4 filtering test, non hashable rule match with max priority vs hashable 		 	*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest065 : public IpaFilteringBlockTestFixture
{
public:

	IpaFilteringBlockTest065()
	{
		m_name = "IpaFilteringBlockTest065";
		m_description =
		"Filtering block test 065 - Rules prioritization hashable vs non-hashable rule, both rules match the packets\
		two identical packets are sent, non hashed with max priority should catch both\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 0 - hashable\
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 1 - non hashable max prio\
			All DST_IP == (192.169.1.2 & 255.0.0.255)traffic goes to routing table 2 - don't care for this specific test\
		3. send three packets:\
			DST_IP == 127.0.0.1 \
			DST_IP == 127.0.0.1 \
			DST_IP == 192.168.1.2";
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass1, routing_table1.hdl);

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass2, routing_table2.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; //
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		flt_rule_entry.rule.hashable = 1; // hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1 on lower priority (second in list)
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		flt_rule_entry.rule.hashable = 0; // non hashed
		flt_rule_entry.rule.max_prio = 1; // max prioirty, should overcome all other rules
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 3
		flt_rule_entry.rule.hashable = 0; // non hashed
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets()
	{
		int address;

		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;

	}

	virtual bool ReceivePacketsAndCompare()
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

		// Compare results
		if (!CompareResultVsGolden(m_sendBuffer, m_sendSize, rxBuff1, receivedSize))
		{
			printf("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		isSuccess &= CompareResultVsGolden(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);

		isSuccess &= CompareResultVsGolden(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}

};

/*----------------------------------------------------------------------------------------------*/
/* Test066: IPV4 filtering test, hashed rule match, non hash doesn't match expect cache hit 	*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest066 : public IpaFilteringBlockTestFixture
{
public:
	bool IsSecondTime;

	IpaFilteringBlockTest066(): IsSecondTime(false)
	{
		m_name = "IpaFilteringBlockTest066";
		m_description =
		"Filtering block test 066 - Rules prioritization hashable vs non-hashable rule, only hashable matches the packets\
		two identical packets are sent and should match the hashable rule, cache hit expected\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 0 - hashable\
			All DST_IP == (127.0.0.2 & 255.0.0.255)traffic goes to routing table 1 - non hashable\
			All DST_IP == (192.169.1.2 & 255.0.0.255)traffic goes to routing table 2 - don't care for this specific test\
		3. send three packets:\
			DST_IP == 127.0.0.1 \
			DST_IP == 127.0.0.1 \
			DST_IP == 192.168.1.2";
		m_minIPAHwType = IPA_HW_v3_0;
		Register(*this);
	}

	bool Setup()
	{
		/* we want statuses on this test */
		return IpaFilteringBlockTestFixture::Setup(true);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass1, routing_table1.hdl);

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass2, routing_table2.hdl);

		struct ipa_flt_rule_add flt_rule_entry;
		if (!IsSecondTime) {
			FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST_PROD, false, 3);
			printf("FilterTable*.Init Completed Successfully..\n");
			IsSecondTime = true;


			// Configuring Filtering Rule No.0
			FilterTable0.GeneratePresetRule(1,flt_rule_entry);
			flt_rule_entry.at_rear = true;
			flt_rule_entry.flt_rule_hdl=-1; // return Value
			flt_rule_entry.status = -1; // return value
			flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
			flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
			flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; //
			flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
			flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.2.
			flt_rule_entry.rule.hashable = 1; // hashed
			if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
			{
				printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
				return false;
			}

			// Configuring Filtering Rule No.1 on lower priority (second in list)
			flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
			flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
			flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000002; // Filter DST_IP == 127.0.0.1.
			flt_rule_entry.rule.hashable = 0; // non hashed
			if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
			{
				printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
				return false;
			}

			// Configuring Filtering Rule No.2
			flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl; //put here the handle corresponding to Routing Rule 3
			flt_rule_entry.rule.hashable = 0; // non hashed
			flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

			if (
					((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
					!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				)
			{
				printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
				return false;
			} else
			{
				printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
				printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
				printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
			}
		} else {
			printf("in the second time, just commit again\n");
			if (!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())) {
				printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
				return false;
			}
		}

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	bool RemoveLastRule()
	{
		struct ipa_ioc_del_flt_rule *pDeleteRule = (struct ipa_ioc_del_flt_rule *)
			calloc(1, sizeof(struct ipa_ioc_del_flt_rule) + sizeof(struct ipa_flt_rule_del));

		pDeleteRule->commit = 1;
		pDeleteRule->ip = IPA_IP_v4;
		pDeleteRule->num_hdls = 1;
		pDeleteRule->hdl[0].hdl = FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl;
		pDeleteRule->hdl[0].status = FilterTable0.ReadRuleFromTable(2)->status;

		if (!m_filtering.DeleteFilteringRule(pDeleteRule))
		{
			printf ("%s::Error Deleting Rule from Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		return true;
	}

	virtual bool ModifyPackets()
	{
		int address;

		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;

	}

	virtual bool ReceivePacketsAndCompare()
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

		// Compare results
		if (!CompareResultVsGolden_w_Status(m_sendBuffer, m_sendSize, rxBuff1, receivedSize))
		{
			printf("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize,receivedSize,rxBuff1);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize, sentBufferSize, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize, sentBufferSize, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize, sentBufferSize, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ? 
			IsCacheHit_v5_0(m_sendSize2, receivedSize2, rxBuff2) : IsCacheHit(m_sendSize2,receivedSize2,rxBuff2);

		isSuccess &= CompareResultVsGolden_w_Status(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize3, receivedSize3, rxBuff3) : IsCacheMiss(m_sendSize3,receivedSize3,rxBuff3);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}

	bool ReceiveAndCompareSpecial()
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

		// Compare results
		if (!CompareResultVsGolden_w_Status(m_sendBuffer, m_sendSize, rxBuff1, receivedSize))
		{
			printf("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize,receivedSize,rxBuff1);

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];
		size_t j;

		if ((3 * m_sendSize) > sentBufferSize)
			printf("Failed to stringify sent packet. Buffer too small\n");
		else
			for(j = 0; j < m_sendSize; j++)
				snprintf(&sentBuffer[3 * j], sentBufferSize - 3 * j,
					" %02X", m_sendBuffer[j]);
		if ((3 * receivedSize) > recievedBufferSize)
			printf("Failed to stringify recieved packet. Buffer too small\n");
		else
			for(j = 0; j < receivedSize; j++)
				snprintf(&recievedBuffer[3 * j], recievedBufferSize - 3 * j,
					" %02X", rxBuff1[j]);
		printf("Expected Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n",m_sendSize,sentBuffer,receivedSize,recievedBuffer);

		delete[] recievedBuffer;
		delete[] sentBuffer;
		delete[] rxBuff1;
		return isSuccess;

	}

private:
	IPAFilteringTable FilterTable0;
};

/*----------------------------------------------------------------------------------------------*/
/* Test067: IPV4 filtering test, hash/cache invalidation on add test 										*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest067 : public IpaFilteringBlockTest066
{
public:
	IpaFilteringBlockTest067()
	{
		m_name = "IpaFilteringBlockTest067";
		m_description =
		"Filtering block test 067 - this test first perfroms test 066 and then commits another rule\
		another identical packet is sent: DST_IP == 127.0.0.1 and expected to get cache miss";
		m_minIPAHwType = IPA_HW_v3_0;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		// Add the relevant filtering rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding filtering rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(m_IpaIPType);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		res = ModifyPackets();
		if (false == res) {
			printf("Failed to modify packets.\n");
			return false;
		}

		// Send first packet
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
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

		// untill here test 066 was run, now let's test the invalidation

		// commit the same rules again, this should clear the cache
		res = AddRules();
		if (false == res) {
			printf("Failed adding filtering rules.\n");
			return false;
		}

		// send packet again
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)	{
			printf("SendData failure.\n");
			return false;
		}

		// receive and verify that cache was missed
		isSuccess = ReceiveAndCompareSpecial();

		printf("Leaving %s, %s(), Returning %d\n",__FUNCTION__, __FILE__,isSuccess);

		return isSuccess;
	} // Run()

};

/*----------------------------------------------------------------------------------------------*/
/* Test068: IPV4 filtering test, hash/cache invalidation on delete test 										*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest068 : public IpaFilteringBlockTest066
{
public:
	IpaFilteringBlockTest068()
	{
		m_name = "IpaFilteringBlockTest068";
		m_description =
		"Filtering block test 068 - this test first perfroms test 066 and then removes last rule\
		another identical packet is sent: DST_IP == 127.0.0.1 and expected to get cache miss";
		m_minIPAHwType = IPA_HW_v3_0;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		// Add the relevant filtering rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding filtering rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(m_IpaIPType);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		res = ModifyPackets();
		if (false == res) {
			printf("Failed to modify packets.\n");
			return false;
		}

		// Send first packet
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
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

		// until here test 066 was run, now let's test the invalidation

		// delete the last rule, this should clear the cache
		res = RemoveLastRule();
		if (false == res) {
			printf("Failed removing filtering rules.\n");
			return false;
		}

		// send packet again
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)	{
			printf("SendData failure.\n");
			return false;
		}

		// receive and verify that cache was missed
		isSuccess = ReceiveAndCompareSpecial();

		printf("Leaving %s, %s(), Returning %d\n",__FUNCTION__, __FILE__,isSuccess);

		return isSuccess;
	} // Run()

};

/*----------------------------------------------------------------------------------------------*/
/* Test070: IPV6 filtering test, non hashed priority higher than hashed priority				*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest070 : public IpaFilteringBlockTest060
{
public:

	IpaFilteringBlockTest070()
	{
		m_name = "IpaFilteringBlockTest070";
		m_description =
		"Filtering block test 070 - Rules prioritization hashable vs non-hashable rule, both rules match the same packet but non hashable has higher priority\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IPv6 == 0x...AA traffic goes to routing table 0 - non hashable\
			All DST_IPv6 == 0x...AA traffic goes to routing table 1 - hashable\
			All DST_IPv6 == 0x...CC traffic goes to routing table 2 - non hashable - don't care for this specific test";
		m_minIPAHwType = IPA_HW_v3_0;
		m_IpaIPType = IPA_IP_v6;
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		uint32_t Hndl0, Hndl1, Hndl2;

		if(!GetThreeIPv6BypassRoutingTables(&Hndl0,&Hndl1,&Hndl2)){
			printf("failed to get three IPV6 routing tables!\n");
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=Hndl0; //put here the handle corresponding to Routing Rule 0
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;

		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0x000000FF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr[0]		 = 0XFF020000; // Filter DST_IP
		flt_rule_entry.rule.attrib.u.v6.dst_addr[1]		 = 0x00000000;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[2]      = 0x11223344;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677AA;
		flt_rule_entry.rule.hashable = 0; // non hashable

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=Hndl1; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677AA;
		flt_rule_entry.rule.hashable = 1; // hashable

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=Hndl2; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677CC;
		flt_rule_entry.rule.hashable = 0; // non hashable

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer3[DST_ADDR_LSB_OFFSET_IPV6] = 0xCC;
		return true;
	}// ModifyPacktes ()

};

/*----------------------------------------------------------------------------------------------*/
/* Test071: IPV6 filtering test, hashed priority higher than non hashed priority + cache hit	*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest071: public IpaFilteringBlockTest061
{
public:

	IpaFilteringBlockTest071()
	{
		m_name = "IpaFilteringBlockTest071";
		m_description =
		"Filtering block test 071 - Rules prioritization hashable vs non-hashable rule, both rules match the same packet but hashable has higher priority\
		two identical packets are sent and should be catched by the hashable rule, second one should be hit the cache\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IPv6 == 0x...AA traffic goes to routing table 0 - hashable\
			All DST_IPv6 == 0x...AA traffic goes to routing table 1 - non hashable\
			All DST_IPv6 == 0x...CC traffic goes to routing table 2 - don't care for this specific test";
		m_minIPAHwType = IPA_HW_v3_0;
		m_IpaIPType = IPA_IP_v6;
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		uint32_t Hndl0, Hndl1, Hndl2;

		if(!GetThreeIPv6BypassRoutingTables(&Hndl0,&Hndl1,&Hndl2)){
			printf("failed to get three IPV6 routing tables!\n");
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=Hndl0; //put here the handle corresponding to Routing Rule 0
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;

		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0x000000FF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr[0]		 = 0XFF020000; // Filter DST_IP
		flt_rule_entry.rule.attrib.u.v6.dst_addr[1]		 = 0x00000000;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[2]      = 0x11223344;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677AA;
		flt_rule_entry.rule.hashable = 1; // hashable

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=Hndl1; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677AA;
		flt_rule_entry.rule.hashable = 0; // non hashable

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=Hndl2; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677CC;
		flt_rule_entry.rule.hashable = 0; // non hashable

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		// TODO: Add verification that we access only allocated addresses
		// TODO: Fix this, doesn't match the Rule's Requirements
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer3[DST_ADDR_LSB_OFFSET_IPV6] = 0xCC;
		return true;
	}// ModifyPacktes ()

};

/*----------------------------------------------------------------------------------------------*/
/* Test072: IPV6 filtering test, hashed rule match, non hash doesn't match expect cache miss	*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest072 : public IpaFilteringBlockTest062
{
public:

	IpaFilteringBlockTest072()
	{
		m_name = "IpaFilteringBlockTest072";
		m_description =
		"Filtering block test 072 - Rules prioritization hashable vs non-hashable rule, only hashable matches the packets\
		two packets with different tuple are sent and should match the hashable rule, no cache hit expected\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IPv6 == 0x...BB traffic goes to routing table 0 - non hashable\
			All DST_IPv6 == 0x...AA traffic goes to routing table 1 - hashable\
			All DST_IPv6 == 0x...CC traffic goes to routing table 2 - don't care for this specific test\
		3. send three packets:\
			DST_IP == 0x...AA port 546\
			DST_IP == 0x...AA port 547\
			DST_IP == 0x...CC";
		m_minIPAHwType = IPA_HW_v3_0;
		m_IpaIPType = IPA_IP_v6;

	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		uint32_t Hndl0, Hndl1, Hndl2;

		if(!GetThreeIPv6BypassRoutingTables(&Hndl0,&Hndl1,&Hndl2)){
			printf("failed to get three IPV6 routing tables!\n");
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=Hndl0; //put here the handle corresponding to Routing Rule 0
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;

		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0x000000FF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr[0]		 = 0XFF020000; // Filter DST_IP
		flt_rule_entry.rule.attrib.u.v6.dst_addr[1]		 = 0x00000000;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[2]      = 0x11223344;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677BB;
		flt_rule_entry.rule.hashable = 0; // non hashable

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=Hndl1; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677AA;
		flt_rule_entry.rule.hashable = 1; // hashable

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=Hndl2; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677CC;
		flt_rule_entry.rule.hashable = 0; // non hashable

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets()
	{

		unsigned short port;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}

		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		port = ntohs(546);//DHCP Client Port
		memcpy (&m_sendBuffer[IPV6_DST_PORT_OFFSET], &port, sizeof(port));

		port = ntohs(547);//DHCP Client Port
		memcpy (&m_sendBuffer2[IPV6_DST_PORT_OFFSET], &port, sizeof(port));
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;

		m_sendBuffer3[DST_ADDR_LSB_OFFSET_IPV6] = 0xCC;
		return true;
	}// ModifyPacktes ()

};

/*----------------------------------------------------------------------------------------------*/
/* Test073: IPV4 filtering test, hashed rule match, non hash doesn't match expect cache miss 	*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest073 : public IpaFilteringBlockTest063
{
public:

	IpaFilteringBlockTest073()
	{
		m_name = "IpaFilteringBlockTest073";
		m_description =
		"Filtering block test 073 - Rules prioritization hashable vs non-hashable rule, only hashable matches the packets\
		two packets with different tuple are sent and should match the hashable rule, no cache hit expected\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IPv6 == 0x...AA traffic goes to routing table 0 - hashable\
			All DST_IPv6 == 0x...BB traffic goes to routing table 1 - non hashable\
			All DST_IPv6 == 0x...CC traffic goes to routing table 2 - don't care for this specific test\
		3. send three packets:\
			DST_IP == 0x...AA port 546\
			DST_IP == 0x...AA port 547\
			DST_IP == 0x...CC";

		m_minIPAHwType = IPA_HW_v3_0;
		m_IpaIPType = IPA_IP_v6;
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		uint32_t Hndl0, Hndl1, Hndl2;

		if(!GetThreeIPv6BypassRoutingTables(&Hndl0,&Hndl1,&Hndl2)){
			printf("failed to get three IPV6 routing tables!\n");
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=Hndl0; //put here the handle corresponding to Routing Rule 0
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;

		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0x000000FF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr[0]		 = 0XFF020000; // Filter DST_IP
		flt_rule_entry.rule.attrib.u.v6.dst_addr[1]		 = 0x00000000;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[2]      = 0x11223344;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677AA;
		flt_rule_entry.rule.hashable = 1; // hashable

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=Hndl1; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677BB;
		flt_rule_entry.rule.hashable = 0; // non hashable

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=Hndl2; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677CC;
		flt_rule_entry.rule.hashable = 0; // non hashable

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets()
	{

		unsigned short port;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}

		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		port = ntohs(546);//DHCP Client Port
		memcpy (&m_sendBuffer[IPV6_DST_PORT_OFFSET], &port, sizeof(port));

		port = ntohs(547);//DHCP Client Port
		memcpy (&m_sendBuffer2[IPV6_DST_PORT_OFFSET], &port, sizeof(port));
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;

		m_sendBuffer3[DST_ADDR_LSB_OFFSET_IPV6] = 0xCC;
		return true;
	}// ModifyPacktes ()
};

/*----------------------------------------------------------------------------------------------*/
/* Test074: IPV6 filtering test, hashed rule match, non hash doesn't match expect cache hit 	*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest074 : public IpaFilteringBlockTest064
{
public:

	IpaFilteringBlockTest074()
	{
		m_name = "IpaFilteringBlockTest074";
		m_description =
		"Filtering block test 074 - Rules prioritization hashable vs non-hashable rule, only hashable matches the packets\
		two identical packets are sent and should match the hashable rule, cache hit expected\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IPv6 == 0x...BB traffic goes to routing table 0 - non hashable\
			All DST_IPv6 == 0x...AA traffic goes to routing table 1 - hashable\
			All DST_IPv6 == 0x...CC traffic goes to routing table 2 - don't care for this specific test\
		3. send three packets:\
			DST_IP == 0x...AA \
			DST_IP == 0x...AA \
			DST_IP == 0x...CC";
		m_minIPAHwType = IPA_HW_v3_0;
		m_IpaIPType = IPA_IP_v6;
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		uint32_t Hndl0, Hndl1, Hndl2;

		if(!GetThreeIPv6BypassRoutingTables(&Hndl0,&Hndl1,&Hndl2)){
			printf("failed to get three IPV6 routing tables!\n");
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=Hndl0; //put here the handle corresponding to Routing Rule 0
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;

		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0x000000FF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr[0]		 = 0XFF020000; // Filter DST_IP
		flt_rule_entry.rule.attrib.u.v6.dst_addr[1]		 = 0x00000000;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[2]      = 0x11223344;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677BB;
		flt_rule_entry.rule.hashable = 0; // non hashable

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=Hndl1; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677AA;
		flt_rule_entry.rule.hashable = 1; // hashable

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=Hndl2; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677CC;
		flt_rule_entry.rule.hashable = 0; // non hashable

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}

		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer3[DST_ADDR_LSB_OFFSET_IPV6] = 0xCC;
		return true;
	}// ModifyPacktes ()

};

/*----------------------------------------------------------------------------------------------*/
/* Test075: IPV6 filtering test, non hashable rule match with max priority vs hashable 		 	*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest075 : public IpaFilteringBlockTest065
{
public:

	IpaFilteringBlockTest075()
	{
		m_name = "IpaFilteringBlockTest075";
		m_description =
		"Filtering block test 075 - Rules prioritization hashable vs non-hashable rule, both rules match the packets\
		two identical packets are sent, non hashed with max priority should catch both\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IPv6 == 0x...AA traffic goes to routing table 0 - hashable\
			All DST_IPv6 == 0x...AA traffic goes to routing table 1 - non hashable max prio\
			All DST_IPv6 == 0x...CC traffic goes to routing table 2 - don't care for this specific test\
		3. send three packets:\
			DST_IP == 0x...AA \
			DST_IP == 0x...AA \
			DST_IP == 0x...CC";
		m_minIPAHwType = IPA_HW_v3_0;
		m_IpaIPType = IPA_IP_v6;
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		uint32_t Hndl0, Hndl1, Hndl2;

		if(!GetThreeIPv6BypassRoutingTables(&Hndl0,&Hndl1,&Hndl2)){
			printf("failed to get three IPV6 routing tables!\n");
			return false;
		}

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6,IPA_CLIENT_TEST_PROD,false,3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=Hndl0; //put here the handle corresponding to Routing Rule 0
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;

		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0x000000FF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr[0]		 = 0XFF020000; // Filter DST_IP
		flt_rule_entry.rule.attrib.u.v6.dst_addr[1]		 = 0x00000000;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[2]      = 0x11223344;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677AA;
		flt_rule_entry.rule.hashable = 1; // hashable

		printf ("flt_rule_entry was set successfully, preparing for insertion....\n");

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=Hndl1; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677AA;
		flt_rule_entry.rule.hashable = 0; // non hashable
		flt_rule_entry.rule.max_prio = 1; // max priority

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=Hndl2; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3]      = 0X556677CC;
		flt_rule_entry.rule.hashable = 0; // non hashable

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding RuleTable(2) to Filtering, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}

		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer3[DST_ADDR_LSB_OFFSET_IPV6] = 0xCC;
		return true;
	}// ModifyPacktes ()
};

/*----------------------------------------------------------------------------------------------*/
/* Test076: IPV6 filtering test, hashed rule match, non hash doesn't match expect cache hit 	*/
/*----------------------------------------------------------------------------------------------*/

class IpaFilteringBlockTest076 : public IpaFilteringBlockTest066
{
public:

	bool IsSecondTime;

	IpaFilteringBlockTest076() :IsSecondTime(false)
	{
		m_name = "IpaFilteringBlockTest076";
		m_description =
		"Filtering block test 076 - Rules prioritization hashable vs non-hashable rule, only hashable matches the packets\
		two identical packets are sent and should match the hashable rule, cache hit expected\
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IPv6 == 0x...AA traffic goes to routing table 0 - hashable\
			All DST_IPv6 == 0x...BB traffic goes to routing table 1 - non hashable\
			All DST_IPv6 == 0x...CC traffic goes to routing table 2 - don't care for this specific test\
		3. send three packets:\
			DST_IP == 0x...AA \
			DST_IP == 0x...AA \
			DST_IP == 0x...CC";
		m_minIPAHwType = IPA_HW_v3_0;
		m_IpaIPType = IPA_IP_v6;
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		uint32_t Hndl0, Hndl1, Hndl2;

		if(!GetThreeIPv6BypassRoutingTables(&Hndl0,&Hndl1,&Hndl2)){
			printf("failed to get three IPV6 routing tables!\n");
			return false;
		}

		struct ipa_flt_rule_add flt_rule_entry;
		if (!IsSecondTime) {
			FilterTable0.Init(IPA_IP_v6, IPA_CLIENT_TEST_PROD, false, 3);
			IsSecondTime = true;


			// Configuring Filtering Rule No.0
			FilterTable0.GeneratePresetRule(1, flt_rule_entry);
			flt_rule_entry.at_rear = true;
			flt_rule_entry.flt_rule_hdl = -1; // return Value
			flt_rule_entry.status = -1; // return value
			flt_rule_entry.rule.action = IPA_PASS_TO_ROUTING;
			flt_rule_entry.rule.rt_tbl_hdl = Hndl0; //put here the handle corresponding to Routing Rule 0
			flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;

			flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;// Exact Match
			flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;// Exact Match
			flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;// Exact Match
			flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0x000000FF;// Exact Match
			flt_rule_entry.rule.attrib.u.v6.dst_addr[0] = 0XFF020000; // Filter DST_IP
			flt_rule_entry.rule.attrib.u.v6.dst_addr[1] = 0x00000000;
			flt_rule_entry.rule.attrib.u.v6.dst_addr[2] = 0x11223344;
			flt_rule_entry.rule.attrib.u.v6.dst_addr[3] = 0X556677AA;
			flt_rule_entry.rule.hashable = 1; // hashable

			printf("flt_rule_entry was set successfully, preparing for insertion....\n");

			if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) {
				printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
				return false;
			}

			// Configuring Filtering Rule No.1
			flt_rule_entry.rule.rt_tbl_hdl = Hndl1; //put here the handle corresponding to Routing Rule 1
			flt_rule_entry.rule.attrib.u.v6.dst_addr[3] = 0X556677BB;
			flt_rule_entry.rule.hashable = 0; // non hashable

			if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) {
				printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
				return false;
			}

			// Configuring Filtering Rule No.2
			flt_rule_entry.rule.rt_tbl_hdl = Hndl2; //put here the handle corresponding to Routing Rule 2
			flt_rule_entry.rule.attrib.u.v6.dst_addr[3] = 0X556677CC;
			flt_rule_entry.rule.hashable = 0; // non hashable

			if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
				) {
				printf("%s::Error Adding RuleTable(2) to Filtering, aborting...\n", __FUNCTION__);
				return false;
			} else {
				printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(2)->status);
			}
		} else {
			printf("in the second time, just commit again\n");
			if (!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())) {
				printf("%s::Error commiting rules second time, aborting...\n", __FUNCTION__);
				return false;
			}
		}
		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	bool RemoveLastRule()
	{
		struct ipa_ioc_del_flt_rule *pDeleteRule = (struct ipa_ioc_del_flt_rule *)
			calloc(1, sizeof(struct ipa_ioc_del_flt_rule) + sizeof(struct ipa_flt_rule_del));

		pDeleteRule->commit = 1;
		pDeleteRule->ip = IPA_IP_v6;
		pDeleteRule->num_hdls = 1;
		pDeleteRule->hdl[0].hdl = FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl;
		pDeleteRule->hdl[0].status = FilterTable0.ReadRuleFromTable(2)->status;

		if (!m_filtering.DeleteFilteringRule(pDeleteRule))
		{
			printf ("%s::Error Deleting Rule from Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		return true;
	}

	virtual bool ModifyPackets()
	{
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}

		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer3[DST_ADDR_LSB_OFFSET_IPV6] = 0xCC;
		return true;
	}// ModifyPacktes ()

private:
	IPAFilteringTable FilterTable0;
};

/*----------------------------------------------------------------------------------------------*/
/* Test077: IPV6 filtering test, hash/cache invalidation on add test 										*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest077 : public IpaFilteringBlockTest076
{
public:
	IpaFilteringBlockTest077()
	{
		m_name = "IpaFilteringBlockTest077";
		m_description =
		"Filtering block test 077 - this test first perfroms test 076 and then commits another rule\
		another identical packet is sent: DST_IP == 127.0.0.1 and expected to get cache miss";
		m_minIPAHwType = IPA_HW_v3_0;
		m_IpaIPType = IPA_IP_v6;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		// Add the relevant filtering rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding filtering rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(m_IpaIPType);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		res = ModifyPackets();
		if (false == res) {
			printf("Failed to modify packets.\n");
			return false;
		}

		// Send first packet
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
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

		// until here test 076 was run, now let's test the invalidation

		// commit the same rules again, this should clear the cache
		res = AddRules();
		if (false == res) {
			printf("Failed adding filtering rules.\n");
			return false;
		}

		// send packet again
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)	{
			printf("SendData failure.\n");
			return false;
		}

		// receive and verify that cache was missed
		isSuccess = ReceiveAndCompareSpecial();

		printf("Leaving %s, %s(), Returning %d\n",__FUNCTION__, __FILE__,isSuccess);

		return isSuccess;
	} // Run()

};

/*----------------------------------------------------------------------------------------------*/
/* Test078: IPV6 filtering test, hash/cache invalidation on add test 										*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest078 : public IpaFilteringBlockTest076
{
public:
	IpaFilteringBlockTest078()
	{
		m_name = "IpaFilteringBlockTest078";
		m_description =
		"Filtering block test 078 - this test first perfroms test 076 and then removes last rule\
		another identical packet is sent: DST_IP == 127.0.0.1 and expected to get cache miss";
		m_minIPAHwType = IPA_HW_v3_0;
		m_IpaIPType = IPA_IP_v6;
	}

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		// Add the relevant filtering rules
		res = AddRules();
		if (false == res) {
			printf("Failed adding filtering rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(m_IpaIPType);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		res = ModifyPackets();
		if (false == res) {
			printf("Failed to modify packets.\n");
			return false;
		}

		// Send first packet
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
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

		// until here test 076 was run, now let's test the invalidation

		// delete the last rule, this should clear the cache
		res = RemoveLastRule();
		if (false == res) {
			printf("Failed removing filtering rules.\n");
			return false;
		}

		// send packet again
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)	{
			printf("SendData failure.\n");
			return false;
		}

		// receive and verify that cache was missed
		isSuccess = ReceiveAndCompareSpecial();

		printf("Leaving %s, %s(), Returning %d\n",__FUNCTION__, __FILE__,isSuccess);

		return isSuccess;
	} // Run()

};

/*---------------------------------------------------------------------------*/
/* Test081: Type-of-service IP header field match  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest081 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest081()
	{
		m_name = "IpaFilteringBlockTest081";
		m_description = " \
		Filtering block test 081 - Source and Destination address and TOS exact match (End-Point specific Filtering Table, Insert all rules in a single commit) \
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules: \
			All DST_IP == (127.0.0.1 & 255.0.0.255) and TOS == 0xFB traffic goes to routing table 0 \
			All DST_IP == (192.168.1.1 & 255.0.0.255) and TOS == 0x10 traffic goes to routing table 1 \
			All DST_IP == (192.168.1.2 & 255.0.0.255) and SRC_IP == (192.168.1.FF & 255.0.0.255) TOS == 0x25 traffic goes to routing table 2";
		m_IpaIPType = IPA_IP_v4;
		Register(*this);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		printf("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}
		printf("%s route table handle = %u\n", bypass1, routing_table1.hdl);

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}
		printf("%s route table handle = %u\n", bypass2, routing_table2.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // out param
		flt_rule_entry.status = -1; // out param
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR | IPA_FLT_TOS;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1
		flt_rule_entry.rule.attrib.u.v4.tos = 0xFB;
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl;
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80101; // Filter DST_IP == 192.168.1.1
		flt_rule_entry.rule.attrib.u.v4.tos = 0x10;
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl;
		flt_rule_entry.rule.attrib.attrib_mask |= IPA_FLT_SRC_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2
		flt_rule_entry.rule.attrib.u.v4.src_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.src_addr = 0xC0A801FF; // Filter DST_IP == 192.168.1.255
		flt_rule_entry.rule.attrib.u.v4.tos = 0x25;

		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl2=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		int address;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		m_sendBuffer[IP4_TOS_FIELD_OFFSET] = 0xFB;

		address = ntohl(0xC0A80101);//192.168.1.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		m_sendBuffer2[IP4_TOS_FIELD_OFFSET] = 0x10;

		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A801FF);//192.168.1.255
		memcpy(&m_sendBuffer3[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		m_sendBuffer3[IP4_TOS_FIELD_OFFSET] = 0x25;

		return true;
	}// ModifyPacktes ()
};

/*---------------------------------------------------------------------------*/
/* Test082: Pure Ack packet match  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest082 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest082()
	{
		m_name = "IpaFilteringBlockTest082";
		m_description = " \
		Filtering block test 082 - Pure Ack packet match \
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules: \
			All IS_PURE_ACK traffic goes to routing table 0 \
			All DST_IP == (192.168.1.5 & 255.0.0.255) goes to routing table 1\
			All other traffic goes to routing table 2";
		m_IpaIPType = IPA_IP_v4;
		m_minIPAHwType = IPA_HW_v4_5;
		Register(*this);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}
		printf("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n",&routing_table1);
			return false;
		}
		printf("%s route table handle = %u\n", bypass1, routing_table1.hdl);

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n",&routing_table2);
			return false;
		}
		printf("%s route table handle = %u\n", bypass2, routing_table2.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST_PROD,false,3);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // out param
		flt_rule_entry.status = -1; // out param
		flt_rule_entry.rule.action=IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl=routing_table0.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_IS_PURE_ACK;
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80105; // Filter DST_IP == 192.168.1.5
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl=routing_table2.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = 0;
		if (
				((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
				!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			printf ("%s::Error Adding Rule to Filter Table, aborting...\n",__FUNCTION__);
			return false;
		} else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl2=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(2)->status);
		}

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool LoadFiles(enum ipa_ip_type ip)
	{
		string fileName;

		if (!LoadNoPayloadPacket(ip, m_sendBuffer, m_sendSize)) {
			LOG_MSG_ERROR("Failed loading no payload Packet\n");
			return false;
		}
		printf ("Loaded %zu Bytes to m_sendBuffer\n",m_sendSize);

		if (!LoadNoPayloadPacket(ip, m_sendBuffer2, m_sendSize2)) {
			LOG_MSG_ERROR("Failed loading no payload Packet\n");
			return false;
		}
		printf ("Loaded %zu Bytes to m_sendBuffer2\n",m_sendSize2);

		if (!LoadDefaultPacket(ip, m_extHdrType, m_sendBuffer3, m_sendSize3)) {
			LOG_MSG_ERROR("Failed loading default Packet\n");
			return false;
		}
		printf ("Loaded %zu Bytes to m_sendBuffer3\n",m_sendSize3);

		return true;
	}

	virtual bool ModifyPackets()
	{
		int address;
		if (
				(NULL == m_sendBuffer) ||
				(NULL == m_sendBuffer2) ||
				(NULL == m_sendBuffer3)
			)
		{
			printf ("Error : %s was called with NULL Buffers\n",__FUNCTION__);
			return false;
		}

		m_sendBuffer[IPv4_TCP_FLAGS_OFFSET] |= TCP_ACK_FLAG_MASK;
		address = ntohl(0xC0A80105);//192.168.1.5
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		address = ntohl(0xC0A80105);//192.168.1.5
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		m_sendBuffer3[IPv4_TCP_FLAGS_OFFSET] |= TCP_ACK_FLAG_MASK;
		return true;
	}// ModifyPacktes ()
};

/*---------------------------------------------------------------------------*/
/* Test090: VLAN ID filtering - IPv4  */
/*---------------------------------------------------------------------------*/
class IpaFilteringBlockTest090 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest090()
	{
		m_name = "IpaFilteringBlockTest090";
		m_description = " \
		Filtering block test 090 - VLAN ID packet match - IPv4 \
		1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit Three filtering rules: \
			All VLAN ID == 5 goes to routing table 0 \
			All VLAN ID == 6 goes to routing table 1\
			All other traffic goes to routing table 2";
		m_IpaIPType = IPA_IP_v4;
		m_minIPAHwType = IPA_HW_v4_0;
		Register(*this);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0, routing_table1, routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables(bypass0, bypass1, bypass2)) {
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0)) {
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		printf("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1)) {
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n", &routing_table1);
			return false;
		}
		printf("%s route table handle = %u\n", bypass1, routing_table1.hdl);

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2)) {
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n", &routing_table2);
			return false;
		}
		printf("%s route table handle = %u\n", bypass2, routing_table2.hdl);

		// note here we are using the VLAN pipe TEST2_PROD
		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST2_PROD, false, 3);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // out param
		flt_rule_entry.status = -1; // out param
		flt_rule_entry.rule.action = IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.retain_hdr = 1; // retain header so we can compare the VID
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_VLAN_ID;
		flt_rule_entry.rule.attrib.vlan_id = 5; //filter all packets with vlan id == 5
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) {
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl = routing_table1.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_VLAN_ID;
		flt_rule_entry.rule.attrib.vlan_id = 6; //filter all packets with vlan id == 6
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) {
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl = routing_table2.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = 0;
		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			) {
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		} else {
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl2=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(2)->status);
		}

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}

	virtual bool LoadFiles(enum ipa_ip_type ip)
	{
		string fileName;
		if (!LoadDefault802_1Q(ip, m_sendBuffer, m_sendSize)) {
			LOG_MSG_ERROR("Failed loading first VLAN Packet\n");
			return false;
		}
		printf("Loaded %zu Bytes to m_sendBuffer\n", m_sendSize);

		if (!LoadDefault802_1Q(ip, m_sendBuffer2, m_sendSize2)) {
			LOG_MSG_ERROR("Failed loading second VLAN Packet\n");
			return false;
		}
		printf("Loaded %zu Bytes to m_sendBuffer2\n", m_sendSize2);

		if (!LoadDefault802_1Q(ip, m_sendBuffer3, m_sendSize3)) {
			LOG_MSG_ERROR("Failed loading default Packet\n");
			return false;
		}
		printf("Loaded %zu Bytes to m_sendBuffer3\n", m_sendSize3);

		return true;
	}

	virtual bool ModifyPackets()
	{
		uint32_t vlan_802_1Q_tag;
		if (
			(NULL == m_sendBuffer) ||
			(NULL == m_sendBuffer2) ||
			(NULL == m_sendBuffer3)
			) {
			printf("Error : %s was called with NULL Buffers\n", __FUNCTION__);
			return false;
		}

		vlan_802_1Q_tag = ntohl(0x81002005); //VLAN ID == 5
		memcpy(&m_sendBuffer[TAG_802_1Q_OFFSET], &vlan_802_1Q_tag, sizeof(vlan_802_1Q_tag));

		vlan_802_1Q_tag = ntohl(0x81002006); //VLAN ID == 6
		memcpy(&m_sendBuffer2[TAG_802_1Q_OFFSET], &vlan_802_1Q_tag, sizeof(vlan_802_1Q_tag));

		// default packet has vlan id == 3, assign it anyway
		vlan_802_1Q_tag = ntohl(0x81002003); //VLAN ID == 3
		memcpy(&m_sendBuffer3[TAG_802_1Q_OFFSET], &vlan_802_1Q_tag, sizeof(vlan_802_1Q_tag));
		return true;
	}// ModifyPacktes ()

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
		res = LoadFiles(m_IpaIPType);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		res = ModifyPackets();
		if (false == res) {
			printf("Failed to modify packets.\n");
			return false;
		}

		// Send first packet
		isSuccess = m_producer2.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess) {
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		isSuccess = m_producer2.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess) {
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer2.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess) {
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		printf("Leaving %s, %s(), Returning %d\n", __FUNCTION__, __FILE__, isSuccess);

		return isSuccess;
	} // Run()
};

/*---------------------------------------------------------------------------*/
/* Test100: Cache LRU behavior test  */
/*---------------------------------------------------------------------------*/
#define CHACHE_ENTRIES 64
#define CHACHE_PLUS_ONE (CHACHE_ENTRIES +1)
class IpaFilteringBlockTest100 : public IpaFilteringBlockTestFixture
{
public:
	IpaFilteringBlockTest100()
	{
		m_name = "IpaFilteringBlockTest100";
		m_description = " \
		Filtering block test 100 - Cache LRU behavior test \
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
		/* we want statuses on this test */
		return IpaFilteringBlockTestFixture::Setup(true);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		int i;
		struct ipa_ioc_get_rt_tbl routing_table0;
		const char bypass0[20] = "Bypass0";

		if (!CreateBypassRoutingTable(&m_routing,
									  m_IpaIPType,
									  bypass0,
									  IPA_CLIENT_TEST2_CONS,
									  0,
									  &routing_table0.hdl)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			return false;
		}

		routing_table0.ip = m_IpaIPType;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n",&routing_table0);
			return false;
		}

		printf("Creating Bypass Routing Table completed successfully\n");

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST_PROD, false, CHACHE_PLUS_ONE);
		printf("FilterTable*.Init Completed Successfully..\n");

		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl=-1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.hashable = 1;

		for (i = 0; i < CHACHE_PLUS_ONE; i++) {
			// Configuring Filtering Rule No.i
			flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80101 + i; // Filter DST_IP == 192.168.1.(1+i).
			if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) {
				printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
				return false;
			}
		}

		if (!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable()))
		{
			printf ("%s::Error Adding RuleTable(%d) to Filtering, aborting...\n", __FUNCTION__, i);
			return false;
		} else {
			for (i = 0; i < CHACHE_PLUS_ONE; i++) {
				printf("flt rule hdl=0x%x, status=0x%x\n",
					   FilterTable0.ReadRuleFromTable(i)->flt_rule_hdl,
					   FilterTable0.ReadRuleFromTable(i)->status);
			}
		}

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets() {
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
		res = LoadFiles(m_IpaIPType);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		// Send the first CHACHE_ENTRIES packets
		// Receive packets and compare results
		// All rules should be cache miss
		for (int i = 0; i < CHACHE_ENTRIES; i++) {
			res = __ModifyPackets(i);
			if (false == res) {
				printf("Failed to modify packets.\n");
				return false;
			}

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
		// Receive packets and compare results
		// All rules should be cache hit
		for (int i = 0; i < CHACHE_ENTRIES; i++) {
			res = __ModifyPackets(i);
			if (false == res) {
				printf("Failed to modify packets.\n");
				return false;
			}

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
		res = __ModifyPackets(CHACHE_ENTRIES);
		if (false == res) {
			printf("Failed to modify packets.\n");
			return false;
		}
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
		res = __ModifyPackets(0);
		if (false == res) {
			printf("Failed to modify packets.\n");
			return false;
		}
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

private:
	bool __ModifyPackets(int i)
	{
		int address;
		if (NULL == m_sendBuffer)
		{
			printf ("Error : %s was called with NULL Buffer\n", __FUNCTION__);
			return false;
		}

		address = ntohl(0xC0A80101 + i); // 192.168.1.(1+i)
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;
	}

};

/*----------------------------------------------------------------------------------------------*/
/* Test101: IPV4 filtering test - non hashed table SRAM <-> DDR dynamic move					*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest101 : public IpaFilteringBlockTestFixture {
public:
	IpaFilteringBlockTest101()
	{
		m_name = "IpaFilteringBlockTest101";
		m_description =
			"Filtering block test 101 - Non-Hashable table should start in SRAM, after adding enough rules table should move to DDR\
			then after removing enough rules the table should return to SRAM\
			1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 0 - non hashable\
			All DST_IP == (127.0.0.1 & 255.0.0.255)traffic goes to routing table 1 - hashable\
			All DST_IP == (192.169.1.2 & 255.0.0.255)traffic goes to routing table 2 - don't care for this specific test";
		m_minIPAHwType = IPA_HW_v5_0;
		m_IpaIPType = IPA_IP_v4;
		Register(*this);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		m_routing.Reset(IPA_IP_v4); // This will issue a Reset command to the Routing as well
		m_routing.Reset(IPA_IP_v6); // This will issue a Reset command to the Routing as well
		m_filtering.Reset(IPA_IP_v4); // This will issue a Reset command to the Filtering as well
		m_filtering.Reset(IPA_IP_v6); // This will issue a Reset command to the Filtering as well

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";

		if (!CreateThreeIPv4BypassRoutingTables(bypass0, bypass1, bypass2)) {
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0)) {
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1)) {
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n", &routing_table1);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass1, routing_table1.hdl);

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2)) {
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n", &routing_table2);
			return false;
		}
		printf("route table %s has the handle %u\n", bypass2, routing_table2.hdl);

		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST_PROD, false, 3);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; //
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		flt_rule_entry.rule.hashable = 0; // non hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) {
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1 on lower priority (second in list)
		flt_rule_entry.rule.rt_tbl_hdl = routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		flt_rule_entry.rule.hashable = 1; // hashed
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) {
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl = routing_table2.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.hashable = 0; // non hashed
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0xC0A80102; // Filter DST_IP == 192.168.1.2.

		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			) {
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		} else {
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(1)->status);
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(2)->status);
		}

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets()
	{
		int address;
		if (
			(NULL == m_sendBuffer) ||
			(NULL == m_sendBuffer2) ||
			(NULL == m_sendBuffer3)
			) {
			printf("Error : %s was called with NULL Buffers\n", __FUNCTION__);
			return false;
		}

		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0x7F000001);//127.0.0.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		address = ntohl(0xC0A80102);//192.168.1.2
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));

		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		size_t receivedSize3 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];
		Byte *rxBuff3 = new Byte[0x400];

		if (NULL == rxBuff1 || NULL == rxBuff2 || NULL == rxBuff3) {
			printf("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		receivedSize2 = m_consumer.ReceiveData(rxBuff2, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize2, m_consumer.m_fromChannelName.c_str());

		receivedSize3 = m_defaultConsumer.ReceiveData(rxBuff3, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize3, m_defaultConsumer.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGolden(m_sendBuffer, m_sendSize, rxBuff1, receivedSize)) {
			printf("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		size_t recievedBufferSize = receivedSize * 3;
		size_t sentBufferSize = m_sendSize * 3;
		char *recievedBuffer = new char[recievedBufferSize];
		char *sentBuffer = new char[sentBufferSize];

		memset(recievedBuffer, 0, recievedBufferSize);
		memset(sentBuffer, 0, sentBufferSize);

		print_packets(receivedSize, m_sendSize, recievedBufferSize - 1, sentBufferSize - 1, rxBuff1, m_sendBuffer, recievedBuffer, sentBuffer);
		recievedBuffer[0] = '\0';
		print_packets(receivedSize2, m_sendSize2, recievedBufferSize - 1, sentBufferSize - 1, rxBuff2, m_sendBuffer2, recievedBuffer, sentBuffer);
		recievedBuffer[0] = '\0';
		print_packets(receivedSize3, m_sendSize3, recievedBufferSize - 1, sentBufferSize - 1, rxBuff3, m_sendBuffer3, recievedBuffer, sentBuffer);

		isSuccess &= CompareResultVsGolden(m_sendBuffer2, m_sendSize2, rxBuff2, receivedSize2);
		isSuccess &= CompareResultVsGolden(m_sendBuffer3, m_sendSize3, rxBuff3, receivedSize3);

		delete[] recievedBuffer;
		delete[] sentBuffer;

		delete[] rxBuff1;
		delete[] rxBuff2;
		delete[] rxBuff3;

		return isSuccess;
	}

	bool SendAndVerifyPackets()
	{
		bool isSuccess = false;

		// Send first packet
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess) {
			printf("SendData failure.\n");
			return false;
		}

		// Send second packet
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess) {
			printf("SendData failure.\n");
			return false;
		}

		// Send third packet
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess) {
			printf("SendData failure.\n");
			return false;
		}

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();
		if (false == isSuccess) {
			printf("ReceivePacketsAndCompare failure.\n");
			return false;
		}

		return true;
	}

	bool AddRuleToEnd()
	{
		struct ipa_flt_rule_add flt_rule_entry;
		IPAFilteringTable tempFltTable;

		tempFltTable.Init(m_IpaIPType, IPA_CLIENT_TEST_PROD, false, 1);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		tempFltTable.GeneratePresetRule(0, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; //
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
		flt_rule_entry.rule.hashable = 0; // non hashed

		if (
			((uint8_t)-1 == tempFltTable.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(tempFltTable.GetFilteringTable())
			) {
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		} else {
			printf("flt rule hdl0=0x%x, status=0x%x\n", tempFltTable.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
		}

		if (!FilterTable0.WriteRuleToEndOfTable(tempFltTable.ReadRuleFromTable(0))){
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		}

		return true;
	}

	bool AddRulesUntilTableMovesToDDR()
	{
		int fd;

		// Open ipa_test device node
		fd = open("/dev/ipa_test", O_RDONLY);
		if (fd < 0) {
			printf("Failed opening %s. errno %d: %s\n", "/dev/ipa_test", errno, strerror(errno));
			return false;
		}

		printf("%s(), fd is %d\n", __FUNCTION__, fd);

		while (FilterTable0.size() < MAX_RULES_NUM &&
			ioctl(fd, IPA_TEST_IOC_IS_TEST_PROD_FLT_IN_SRAM, m_IpaIPType)) {
			if (!AddRuleToEnd())
				return false;
			printf("%s, %s() Added rule #%d sucessfully \n", __FUNCTION__, __FILE__, FilterTable0.size() - 1);
		}
		close(fd);

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}

	bool RemoveLastRule()
	{
		struct ipa_ioc_del_flt_rule *pDeleteRule = (struct ipa_ioc_del_flt_rule *)
			calloc(1, sizeof(struct ipa_ioc_del_flt_rule) + sizeof(struct ipa_flt_rule_del));

		pDeleteRule->commit = 1;
		pDeleteRule->ip = m_IpaIPType;
		pDeleteRule->num_hdls = 1;
		pDeleteRule->hdl[0].hdl = FilterTable0.ReadRuleFromTable(FilterTable0.size()-1)->flt_rule_hdl;
		pDeleteRule->hdl[0].status = -1;

		if (!m_filtering.DeleteFilteringRule(pDeleteRule)) {
			printf("%s::Error Deleting Rule from Filter Table, aborting...\n", __FUNCTION__);
			return false;
		}

		return true;
	}

	bool RemoveRulesUntilTableMovesToSRAM()
	{
		int fd;

		// Open ipa_test device node
		fd = open("/dev/ipa_test", O_RDONLY);
		if (fd < 0) {
			printf("Failed opening %s. errno %d: %s\n", "/dev/ipa_test", errno, strerror(errno));
			return false;
		}

		printf("%s(), fd is %d\n", __FUNCTION__, fd);
		while (FilterTable0.size() > MIN_RULES_NUM &&
			!ioctl(fd, IPA_TEST_IOC_IS_TEST_PROD_FLT_IN_SRAM, m_IpaIPType))
			if (!RemoveLastRule())
				return false;
		close(fd);

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
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
		res = LoadFiles(m_IpaIPType);
		if (false == res) {
			printf("Failed loading files.\n");
			return false;
		}

		res = ModifyPackets();
		if (false == res) {
			printf("Failed to modify packets.\n");
			return false;
		}

		/* Send packets with table in SRAM */
		isSuccess = SendAndVerifyPackets();
		if (!isSuccess) {
			printf("Leaving %s, %s(), Returning %d\n", __FUNCTION__, __FILE__, isSuccess);
			return false;
		}

		/* Add rules until table moves to DDR and send packets again */
		isSuccess = AddRulesUntilTableMovesToDDR();
		if (!isSuccess) {
			printf("Leaving %s, %s(), Returning %d\n", __FUNCTION__, __FILE__, isSuccess);
			return false;
		}

		isSuccess = SendAndVerifyPackets();
		if (!isSuccess) {
			printf("Leaving %s, %s(), Returning %d\n", __FUNCTION__, __FILE__, isSuccess);
			return false;
		}

		/* Remove rules until table moves back to SRAM and send packets again */
		isSuccess = RemoveRulesUntilTableMovesToSRAM();
		if (!isSuccess) {
			printf("Leaving %s, %s(), Returning %d\n", __FUNCTION__, __FILE__, isSuccess);
			return false;
		}

		isSuccess = SendAndVerifyPackets();

		printf("Leaving %s, %s(), Returning %d\n", __FUNCTION__, __FILE__, isSuccess);

		return isSuccess;
	} // Run()

protected:
	IPAFilteringTable FilterTable0;
	struct ipa_ioc_get_rt_tbl routing_table0, routing_table1, routing_table2;
};

/*----------------------------------------------------------------------------------------------*/
/* Test102: IPV6 filtering test - non hashed table SRAM <-> DDR dynamic move					*/
/*----------------------------------------------------------------------------------------------*/
class IpaFilteringBlockTest102 : public IpaFilteringBlockTest101 {
public:
	IpaFilteringBlockTest102()
	{
		m_name = "IpaFilteringBlockTest102";
		m_description =
			"Filtering block test 102 - Non-Hashable table should start in SRAM, after adding enough rules table should move to DDR\
			then after removing enough rules the table should return to SRAM\
			1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit three filtering rules: (DST & Mask Match). \
			All DST_IPv6 == 0x...AA traffic goes to routing table 0 - non hashable\
			All DST_IPv6 == 0x...AA traffic goes to routing table 1 - hashable\
			All DST_IPv6 == 0x...CC traffic goes to routing table 2 - non hashable - don't care for this specific test";
		m_minIPAHwType = IPA_HW_v5_0;
		m_IpaIPType = IPA_IP_v6;
	}

	virtual bool GetThreeIPv6BypassRoutingTables(uint32_t *Hndl0, uint32_t *Hndl1, uint32_t *Hndl2)
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";

		if (!CreateThreeIPv6BypassRoutingTables(bypass0, bypass1, bypass2)) {
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v6;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0)) {
			printf("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v6;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1)) {
			printf("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n", &routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v6;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2)) {
			printf("m_routing.GetRoutingTable(&routing_table2=0x%p) Failed.\n", &routing_table2);
			return false;
		}

		*Hndl0 = routing_table0.hdl;
		*Hndl1 = routing_table1.hdl;
		*Hndl2 = routing_table2.hdl;

		return true;
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		if (!GetThreeIPv6BypassRoutingTables(&Hndl0, &Hndl1, &Hndl2)) {
			printf("failed to get three IPV6 routing tables!\n");
			return false;
		}

		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6, IPA_CLIENT_TEST_PROD, false, 3);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl = Hndl0; //put here the handle corresponding to Routing Rule 0
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;

		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0x000000FF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr[0] = 0XFF020000; // Filter DST_IP
		flt_rule_entry.rule.attrib.u.v6.dst_addr[1] = 0x00000000;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[2] = 0x11223344;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3] = 0X556677AA;
		flt_rule_entry.rule.hashable = 0; // non hashable

		printf("flt_rule_entry was set successfully, preparing for insertion....\n");

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) {
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl = Hndl1; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3] = 0X556677AA;
		flt_rule_entry.rule.hashable = 1; // hashable

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) {
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		}

		// Configuring Filtering Rule No.2
		flt_rule_entry.rule.rt_tbl_hdl = Hndl2; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3] = 0X556677CC;
		flt_rule_entry.rule.hashable = 0; // non hashable

		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			) {
			printf("%s::Error Adding RuleTable(2) to Filtering, aborting...\n", __FUNCTION__);
			return false;
		} else {
			printf("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(2)->status);
		}
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}

	virtual bool ModifyPackets()
	{
		if (
			(NULL == m_sendBuffer) ||
			(NULL == m_sendBuffer2) ||
			(NULL == m_sendBuffer3)
			) {
			printf("Error : %s was called with NULL Buffers\n", __FUNCTION__);
			return false;
		}
		m_sendBuffer[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer2[DST_ADDR_LSB_OFFSET_IPV6] = 0xAA;
		m_sendBuffer3[DST_ADDR_LSB_OFFSET_IPV6] = 0xCC;
		return true;
	}// ModifyPacktes ()

	bool AddRuleToEnd()
	{
		struct ipa_flt_rule_add flt_rule_entry;
		IPAFilteringTable tempFltTable;

		tempFltTable.Init(m_IpaIPType, IPA_CLIENT_TEST_PROD, false, 1);
		printf("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		tempFltTable.GeneratePresetRule(0, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl = Hndl0; //put here the handle corresponding to Routing Rule 0
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;

		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0x00000000;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0x000000FF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr[0] = 0XFF020000; // Filter DST_IP
		flt_rule_entry.rule.attrib.u.v6.dst_addr[1] = 0x00000000;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[2] = 0x11223344;
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3] = 0X556677AA;
		flt_rule_entry.rule.hashable = 0; // non hashable

		if (
			((uint8_t)-1 == tempFltTable.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(tempFltTable.GetFilteringTable())
			) {
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		} else {
			printf("flt rule hdl0=0x%x, status=0x%x\n", tempFltTable.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
		}

		if (!FilterTable0.WriteRuleToEndOfTable(tempFltTable.ReadRuleFromTable(0))) {
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		}

		return true;
	}

protected:
	uint32_t Hndl0, Hndl1, Hndl2;
};

static class IpaFilteringBlockTest001 ipaFilteringBlockTest001;//Global Filtering Test
static class IpaFilteringBlockTest002 ipaFilteringBlockTest002;//Global Filtering Test
static class IpaFilteringBlockTest003 ipaFilteringBlockTest003;//Global Filtering Test
static class IpaFilteringBlockTest004 ipaFilteringBlockTest004;//Global Filtering Test
static class IpaFilteringBlockTest005 ipaFilteringBlockTest005;//Global Filtering Test
static class IpaFilteringBlockTest006 ipaFilteringBlockTest006;//Global Filtering Test
static class IpaFilteringBlockTest007 ipaFilteringBlockTest007;//Global Filtering Test
static class IpaFilteringBlockTest008 ipaFilteringBlockTest008;//Global Filtering Test
static class IpaFilteringBlockTest009 ipaFilteringBlockTest009;//Global Filtering Test
static class IpaFilteringBlockTest010 ipaFilteringBlockTest010;//Global Filtering Test

static class IpaFilteringBlockTest021 ipaFilteringBlockTest021;//End point Specific Filtering Table
static class IpaFilteringBlockTest022 ipaFilteringBlockTest022;//End point Specific Filtering Table
static class IpaFilteringBlockTest023 ipaFilteringBlockTest023;//End point Specific Filtering Table
static class IpaFilteringBlockTest024 ipaFilteringBlockTest024;//End point Specific Filtering Table
static class IpaFilteringBlockTest025 ipaFilteringBlockTest025;//End point Specific Filtering Table
static class IpaFilteringBlockTest026 ipaFilteringBlockTest026;//End point Specific Filtering Table
static class IpaFilteringBlockTest027 ipaFilteringBlockTest027;//End point Specific Filtering Table
static class IpaFilteringBlockTest028 ipaFilteringBlockTest028;//End point Specific Filtering Table
static class IpaFilteringBlockTest029 ipaFilteringBlockTest029;//End point Specific Filtering Table
static class IpaFilteringBlockTest030 ipaFilteringBlockTest030;//End point Specific Filtering Table
static class IpaFilteringBlockTest031 ipaFilteringBlockTest031;//End point Specific Filtering Table

static class IpaFilteringBlockTest050 ipaFilteringBlockTest050;// IPv6 Test, Global Filtering Table
static class IpaFilteringBlockTest051 ipaFilteringBlockTest051;// IPv6 Test, End point Specific Filtering Table
static class IpaFilteringBlockTest052 ipaFilteringBlockTest052;// IPv6 Test, Global Filtering Table
static class IpaFilteringBlockTest053 ipaFilteringBlockTest053;// IPv6 Test, End point Specific Filtering Table
static class IpaFilteringBlockTest054 ipaFilteringBlockTest054;// IPv6 Test, End point Specific Filtering Table

static class IpaFilteringBlockTest060 ipaFilteringBlockTest060; // Hashed non hashed tests IPv4
static class IpaFilteringBlockTest061 ipaFilteringBlockTest061; // Hashed non hashed tests IPv4
static class IpaFilteringBlockTest062 ipaFilteringBlockTest062; // Hashed non hashed tests IPv4
static class IpaFilteringBlockTest063 ipaFilteringBlockTest063; // Hashed non hashed tests IPv4
static class IpaFilteringBlockTest064 ipaFilteringBlockTest064; // Hashed non hashed tests IPv4
static class IpaFilteringBlockTest065 ipaFilteringBlockTest065; // Hashed non hashed tests IPv4
static class IpaFilteringBlockTest066 ipaFilteringBlockTest066; // Hashed non hashed tests IPv4
static class IpaFilteringBlockTest067 ipaFilteringBlockTest067; // Cache/Hash invalidation on add test
static class IpaFilteringBlockTest068 ipaFilteringBlockTest068; // Cache/Hash invalidation on delete test

static class IpaFilteringBlockTest070 ipaFilteringBlockTest070; // Hashed non hashed tests IPV6
static class IpaFilteringBlockTest071 ipaFilteringBlockTest071; // Hashed non hashed tests IPV6
static class IpaFilteringBlockTest072 ipaFilteringBlockTest072; // Hashed non hashed tests IPV6
static class IpaFilteringBlockTest073 ipaFilteringBlockTest073; // Hashed non hashed tests IPV6
static class IpaFilteringBlockTest074 ipaFilteringBlockTest074; // Hashed non hashed tests IPV6
static class IpaFilteringBlockTest075 ipaFilteringBlockTest075; // Hashed non hashed tests IPV6
static class IpaFilteringBlockTest076 ipaFilteringBlockTest076; // Hashed non hashed tests IPV6
static class IpaFilteringBlockTest077 ipaFilteringBlockTest077; // Cache/Hash invalidation on add test
static class IpaFilteringBlockTest078 ipaFilteringBlockTest078; // Cache/Hash invalidation on delete test

static class IpaFilteringBlockTest081 ipaFilteringBlockTest081; // Type-of-service Filter rule test
static class IpaFilteringBlockTest082 ipaFilteringBlockTest082; // IPv4 Pure Ack Filter rule test

static class IpaFilteringBlockTest090 ipaFilteringBlockTest090; // VLAN filtering test IPv4

static class IpaFilteringBlockTest100 ipaFilteringBlockTest100; // Cache LRU behavior test

static class IpaFilteringBlockTest101 ipaFilteringBlockTest101; // Non hashed table SRAM <->DDR dynamic move
static class IpaFilteringBlockTest102 ipaFilteringBlockTest102; // Non hashed table SRAM <->DDR dynamic move