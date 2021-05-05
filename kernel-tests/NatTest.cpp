/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
#include <cstring> // for memcpy
#include "hton.h" // for htonl
#include "InterfaceAbstraction.h"
#include "Constants.h"
#include "Logger.h"
#include "TestsUtils.h"
#include "Filtering.h"
#include "RoutingDriverWrapper.h"
#include "IPAFilteringTable.h"
extern "C" {
#include "ipa_nat_drv.h"
}

//IP offsets
#define IPV4_PROTOCOL_OFFSET (9)
#define IPV4_IP_CHECKSUM_OFFSET (10)
#define IPV4_SRC_ADDR_OFFSET (12)
#define IPV4_DST_ADDR_OFFSET (16)

//TCP offsets
#define IPV4_SRC_PORT_OFFSET (20)
#define IPV4_DST_PORT_OFFSET (20+2)
#define IPV4_TCP_FLAGS_OFFSET (33)
#define IPV4_TCP_CHECKSUM_OFFSET (36)

#define IPA_16BIT_ROUND_UP(val) \
	do { \
		if (val >> 16) { \
			val = (val & 0x0000FFFF);\
			val += 1;\
		} \
	} while(0)

extern Logger g_Logger;

class IpaNatBlockTestFixture : public TestBase
{
public:

	IpaNatBlockTestFixture() :
		m_sendSize(BUFF_MAX_SIZE),
		m_sendSize2(BUFF_MAX_SIZE),
		m_sendSize3(BUFF_MAX_SIZE),
		m_IpaIPType(IPA_IP_v4),
		m_extHdrType(NONE)
	{
		memset(m_sendBuffer, 0, sizeof(m_sendBuffer));	// First input file / IP packet
		memset(m_sendBuffer2, 0, sizeof(m_sendBuffer2));	// Second input file / IP packet
		memset(m_sendBuffer3, 0, sizeof(m_sendBuffer3));	// Third input file (default) / IP packet
		m_testSuiteName.push_back("Nat");
	}

	static int SetupKernelModule(bool en_status = 0)
	{
		int retval;
		struct ipa_channel_config from_ipa_channels[3];
		struct test_ipa_ep_cfg from_ipa_cfg[3];
		struct ipa_channel_config to_ipa_channels[2];
		struct test_ipa_ep_cfg to_ipa_cfg[2];

		struct ipa_test_config_header header = { 0 };
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

		/* To ipa configurations - 2 pipes */
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
			LOG_MSG_ERROR("Routing block is not ready for immediate commands!\n");
			return false;
		}

		if (!m_filtering.DeviceNodeIsOpened())
		{
			LOG_MSG_ERROR("Filtering block is not ready for immediate commands!\n");
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
			LOG_MSG_ERROR("Routing block is not ready for immediate commands!\n");
			return false;
		}

		if (!m_filtering.DeviceNodeIsOpened())
		{
			LOG_MSG_ERROR("Filtering block is not ready for immediate commands!\n");
			return false;
		}
		m_routing.Reset(IPA_IP_v4); // This will issue a Reset command to the Filtering as well
		m_routing.Reset(IPA_IP_v6); // This will issue a Reset command to the Filtering as well
		return true;
	} // Setup()

	bool Teardown()
	{
		ipa_nat_dump_ipv4_table(m_tbl_hdl);
		ipa_nat_del_ipv4_tbl(m_tbl_hdl);

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
		}
		else {
			fileName = "Input/IPv6";
		}

		if (!LoadDefaultPacket(ip, m_extHdrType, m_sendBuffer, m_sendSize)) {
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}
		LOG_MSG_DEBUG("Loaded %zu Bytes to Buffer 1\n", m_sendSize);

		if (!LoadDefaultPacket(ip, m_extHdrType, m_sendBuffer2, m_sendSize2)) {
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}
		LOG_MSG_DEBUG("Loaded %zu Bytes to Buffer 2\n", m_sendSize2);

		if (!LoadDefaultPacket(ip, m_extHdrType, m_sendBuffer3, m_sendSize3)) {
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}
		LOG_MSG_DEBUG("Loaded %zu Bytes to Buffer 3\n", m_sendSize3);

		return true;
	}

	inline bool VerifyStatusReceived(size_t SendSize, size_t RecvSize)
	{
		size_t stts_size = sizeof(struct ipa3_hw_pkt_status);

		if (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) {
			stts_size = sizeof(struct ipa3_hw_pkt_status_hw_v5_0);
		}

		if ((RecvSize <= SendSize) ||
			((RecvSize - SendSize) != stts_size)) {
			LOG_MSG_ERROR("received buffer size does not match! sent:receive [%zu]:[%zu]\n", SendSize, RecvSize);
			return false;
		}

		return true;
	}

	inline bool IsCacheHit(size_t SendSize, size_t RecvSize, void *Buff)
	{
		struct ipa3_hw_pkt_status *pStatus = (struct ipa3_hw_pkt_status *)Buff;

		if (VerifyStatusReceived(SendSize, RecvSize) == false) {
			return false;
		}

		if ((bool)pStatus->route_hash) {
			LOG_MSG_DEBUG("cache hit!! \n");
			return true;
		}

		LOG_MSG_ERROR("cache miss!! \n");
		return false;

	}
	
	inline bool IsCacheHit_v5_0(size_t SendSize, size_t RecvSize, void *Buff)
	{
		struct ipa3_hw_pkt_status_hw_v5_0 *pStatus = (struct ipa3_hw_pkt_status_hw_v5_0 *)Buff;

		if (VerifyStatusReceived(SendSize, RecvSize) == false) {
			return false;
		}

		if ((bool)pStatus->route_hash) {
			LOG_MSG_DEBUG("cache hit!! \n");
			return true;
		}

		LOG_MSG_ERROR("cache miss!! \n");
		return false;

	}

	inline bool IsCacheMiss(size_t SendSize, size_t RecvSize, void *Buff)
	{
		struct ipa3_hw_pkt_status *pStatus = (struct ipa3_hw_pkt_status *)Buff;

		if (VerifyStatusReceived(SendSize, RecvSize) == false) {
			return false;
		}

		if (!((bool)pStatus->route_hash)) {
			LOG_MSG_DEBUG("cache miss!! \n");
			return true;
		}

		LOG_MSG_ERROR("cache hit!! \n");
		return false;
	}
	
	inline bool IsCacheMiss_v5_0(size_t SendSize, size_t RecvSize, void *Buff)
	{
		struct ipa3_hw_pkt_status_hw_v5_0 *pStatus = (struct ipa3_hw_pkt_status_hw_v5_0 *)Buff;

		if (VerifyStatusReceived(SendSize, RecvSize) == false) {
			return false;
		}

		if (!((bool)pStatus->route_hash)) {
			LOG_MSG_DEBUG("cache miss!! \n");
			return true;
		}

		LOG_MSG_ERROR("cache hit!! \n");
		return false;
	}

	bool CompareResultVsGoldenNat(Byte *goldenBuffer, unsigned int goldenSize,
		Byte *receivedBuffer, unsigned int receivedSize, int private_ip, int public_ip,
		int private_port, int public_port, bool src_nat, int IPv4_offset = 0, bool with_status = false)
	{
		bool result;
		uint32_t address;
		uint16_t port;
		uint32_t val;
		uint16_t ip_checksum_diff, tcp_checksum_diff;
		uint32_t ip_checksum, tcp_checksum;
		int recv_offset = 0;
		size_t stts_size = sizeof(struct ipa3_hw_pkt_status);

		if (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) {
			stts_size = sizeof(struct ipa3_hw_pkt_status_hw_v5_0);
		}

		if (with_status)
			recv_offset += stts_size;

		ip_checksum_diff = calc_ip_cksum_diff(public_ip, private_ip);
		tcp_checksum_diff = calc_tcp_udp_cksum_diff(public_ip, public_port, private_ip, private_port);

		//calculate new ip checksum, old checksum + 1's compliment of checksum diff
		ip_checksum = *((uint16_t *)&goldenBuffer[IPV4_IP_CHECKSUM_OFFSET + IPv4_offset]);
		ip_checksum = ntohs(ip_checksum);

		if(src_nat)
			ip_checksum += (uint16_t)(~ip_checksum_diff);
		else
			ip_checksum += (uint16_t)ip_checksum_diff;

		IPA_16BIT_ROUND_UP(ip_checksum);

		//return to network format
		ip_checksum = htons(ip_checksum);

		//calculate new tcp checksum, old checksum + 1's compliment of checksum diff
		tcp_checksum = *((uint16_t *)&goldenBuffer[IPV4_TCP_CHECKSUM_OFFSET + IPv4_offset]);
		tcp_checksum = ntohs(tcp_checksum);

		if(src_nat)
			tcp_checksum += (uint16_t)(~tcp_checksum_diff);
		else
			tcp_checksum += (uint16_t)tcp_checksum_diff;

		IPA_16BIT_ROUND_UP(tcp_checksum);

		//return to network format
		tcp_checksum = htons(tcp_checksum);

		if ((receivedSize - recv_offset) != goldenSize) {
			g_Logger.AddMessage(LOG_VERBOSE, "%s Buffers sizes are different.\n", __FUNCTION__);
			return false;
		}

		Byte *tmp_buff = new Byte[goldenSize];

		memcpy(tmp_buff, goldenBuffer, goldenSize);

		if (src_nat) {
			address = htonl(public_ip);
			port = htons(public_port);

			memcpy(&tmp_buff[IPV4_SRC_ADDR_OFFSET + IPv4_offset], &address, sizeof(address));
			memcpy(&tmp_buff[IPV4_SRC_PORT_OFFSET + IPv4_offset], &port, sizeof(port));

			val = (*(uint32_t*)(&receivedBuffer[IPV4_SRC_ADDR_OFFSET + IPv4_offset + recv_offset]));
			if (address != val)
				LOG_MSG_ERROR("received src ip 0x%X != 0x%X\n", val, address);

			val = (*(uint16_t*)(&receivedBuffer[IPV4_SRC_PORT_OFFSET + IPv4_offset + recv_offset]));
			if (port != val)
				LOG_MSG_ERROR("received src port %d != %d\n", val, port);
		}
		else {
			address = htonl(private_ip);
			port = htons(private_port);

			memcpy(&tmp_buff[IPV4_DST_ADDR_OFFSET + IPv4_offset], &address, sizeof(address));
			memcpy(&tmp_buff[IPV4_DST_PORT_OFFSET + IPv4_offset], &port, sizeof(port));

			val = (*(uint32_t*)(&receivedBuffer[IPV4_DST_ADDR_OFFSET + IPv4_offset + +recv_offset]));
			if (address != val)
				LOG_MSG_ERROR("received dst ip 0x%X != 0x%X\n", val, address);
			val = (*(uint16_t*)(&receivedBuffer[IPV4_DST_PORT_OFFSET + IPv4_offset + recv_offset]));
			if (port != val)
				LOG_MSG_ERROR("received dst port %d != %d\n", val, port);
		}

		memcpy(&tmp_buff[IPV4_IP_CHECKSUM_OFFSET + IPv4_offset], &ip_checksum, sizeof(uint16_t));
		val  = (*(uint16_t*)(&receivedBuffer[IPV4_IP_CHECKSUM_OFFSET + IPv4_offset + recv_offset]));
		if (ip_checksum != val)
			LOG_MSG_ERROR("received checksum %d != %d\n", val, ip_checksum);

		memcpy(&tmp_buff[IPV4_TCP_CHECKSUM_OFFSET + IPv4_offset], &tcp_checksum, sizeof(uint16_t));
		val = (*(uint16_t*)(&receivedBuffer[IPV4_TCP_CHECKSUM_OFFSET + IPv4_offset + recv_offset]));
		if (tcp_checksum != val)
			LOG_MSG_ERROR("received checksum %d != %d\n", val, tcp_checksum);

		size_t j;
		char tmpBuffer[512] = { 0 };

		for (j = 0; j < receivedSize; j++)
			snprintf(&tmpBuffer[3 * j], sizeof(tmpBuffer) - (3 * j + 1), " %02X", tmp_buff[j]);
		LOG_MSG_STACK("expected packet should be (%zu)\n%s\n", receivedSize, tmpBuffer);

		result = !memcmp((void*)tmp_buff, (void*)(receivedBuffer + recv_offset), goldenSize);
		if (!result)
			LOG_MSG_ERROR("buffers comparison failed!!\n");

		delete[] tmp_buff;

		return result;
	}

	bool CreateMetdataRoutingRule(const char * bypass0)
	{
		LOG_MSG_DEBUG("Entering\n");
		struct ipa_ioc_add_rt_rule *rt_rule0 = NULL;
		struct ipa_rt_rule_add *rt_rule_entry;

		rt_rule0 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
				sizeof(struct ipa_ioc_add_rt_rule) +
				1 * sizeof(struct ipa_rt_rule_add)
			);
		if (!rt_rule0) {
			LOG_MSG_ERROR("calloc failed to allocate rt_rule0\n");
			return false;
		}

		rt_rule0->num_rules = 1;
		rt_rule0->ip = IPA_IP_v4;
		rt_rule0->commit = true;
		strlcpy(rt_rule0->rt_tbl_name, bypass0, sizeof(rt_rule0->rt_tbl_name));

		rt_rule_entry = &rt_rule0->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_META_DATA;
		rt_rule_entry->rule.attrib.meta_data = m_metadata;
		rt_rule_entry->rule.attrib.meta_data_mask = 0xFFFFFFFF;// Filter exact metadata value
		if (false == m_routing.AddRoutingRule(rt_rule0))
		{
			LOG_MSG_ERROR("Routing rule addition(rt_rule0) failed!\n");
			Free(rt_rule0);
			return false;
		}

		Free(rt_rule0);
		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}

	bool CreateHashableRoutingRules(const char * bypass0)
	{
		LOG_MSG_DEBUG("Entering\n");
		struct ipa_ioc_add_rt_rule *rt_rule0 = 0, *rt_rule1 = 0;
		struct ipa_rt_rule_add *rt_rule_entry;

		rt_rule0 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
				sizeof(struct ipa_ioc_add_rt_rule) +
				2 * sizeof(struct ipa_rt_rule_add)
			);
		if (!rt_rule0) {
			LOG_MSG_ERROR("calloc failed to allocate rt_rule0\n");
			return false;
		}

		rt_rule0->num_rules = 2;
		rt_rule0->ip = IPA_IP_v4;
		rt_rule0->commit = true;
		strlcpy(rt_rule0->rt_tbl_name, bypass0, sizeof(rt_rule0->rt_tbl_name));

		rt_rule_entry = &rt_rule0->rules[0];
		rt_rule_entry->at_rear = 0;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST2_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr = m_private_ip;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;// Exact match
		rt_rule_entry->rule.hashable = 1;

		rt_rule_entry = &rt_rule0->rules[1];
		rt_rule_entry->at_rear = 1;
		rt_rule_entry->rule.dst = IPA_CLIENT_TEST3_CONS;
		rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		rt_rule_entry->rule.attrib.u.v4.dst_addr = m_private_ip2;
		rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF;// Exact match
		rt_rule_entry->rule.hashable = 1;
		if (false == m_routing.AddRoutingRule(rt_rule0))
		{
			LOG_MSG_ERROR("Routing rule addition(rt_rule0) failed!\n");
			Free(rt_rule1);
			Free(rt_rule0);
			return false;
		}

		Free(rt_rule0);
		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}

	// This function creates three IPv4 bypass routing entries and commits them.
	bool CreateThreeIPv4BypassRoutingTables(const char * bypass0, const char * bypass1, const char * bypass2)
	{
		LOG_MSG_DEBUG("Entering\n");
		struct ipa_ioc_add_rt_rule *rt_rule0 = 0, *rt_rule1 = 0, *rt_rule2 = 0;
		struct ipa_rt_rule_add *rt_rule_entry;

		rt_rule0 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
				sizeof(struct ipa_ioc_add_rt_rule) +
				1 * sizeof(struct ipa_rt_rule_add)
			);
		if (!rt_rule0) {
			LOG_MSG_ERROR("calloc failed to allocate rt_rule0\n");
			return false;
		}
		rt_rule1 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
				sizeof(struct ipa_ioc_add_rt_rule) +
				1 * sizeof(struct ipa_rt_rule_add)
			);
		if (!rt_rule1) {
			LOG_MSG_ERROR("calloc failed to allocate rt_rule1\n");
			Free(rt_rule0);
			return false;
		}
		rt_rule2 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
				sizeof(struct ipa_ioc_add_rt_rule) +
				1 * sizeof(struct ipa_rt_rule_add)
			);
		if (!rt_rule2) {
			LOG_MSG_ERROR("calloc failed to allocate rt_rule2\n");
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
			Free(rt_rule2);
			Free(rt_rule1);
			Free(rt_rule0);
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
			Free(rt_rule2);
			Free(rt_rule1);
			Free(rt_rule0);
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
			LOG_MSG_ERROR("Routing rule addition(rt_rule2) failed!\n");
			Free(rt_rule2);
			Free(rt_rule1);
			Free(rt_rule0);
			return false;
		}


		Free(rt_rule2);
		Free(rt_rule1);
		Free(rt_rule0);
		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}

	void Load8021QPacket()
	{
		m_sendSize = sizeof(m_sendBuffer);
		LoadDefault802_1Q(IPA_IP_v4, m_sendBuffer, m_sendSize);
	}

	virtual bool ModifyPackets() = 0;
	virtual bool AddRules() = 0;
	virtual bool SendPackets() = 0;
	virtual bool ReceivePacketsAndCompare() = 0;

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		LOG_MSG_DEBUG("Entering\n");

		// Add the relevant filtering rules
		res = AddRules();
		if (false == res) {
			LOG_MSG_ERROR("Failed adding filtering rules.\n");
			return false;
		}

		// Load input data (IP packet) from file
		res = LoadFiles(m_IpaIPType);
		if (false == res) {
			LOG_MSG_ERROR("Failed loading files.\n");
			return false;
		}

		res = ModifyPackets();
		if (false == res) {
			LOG_MSG_ERROR("Failed to modify packets.\n");
			return false;
		}

		res = SendPackets();
		if (res == false) {
			LOG_MSG_ERROR("failed to send packets\n");
			return false;
		}
		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		LOG_MSG_DEBUG("Returning %d\n", isSuccess);

		return isSuccess;
	} // Run()

	  /**
	  * calc_ip_cksum_diff() - Calculate the source nat
	  * IP checksum diff
	  * @pub_ip_addr: [in] public ip address
	  * @priv_ip_addr: [in]	Private ip address
	  *
	  * source nat ip checksum different is calculated as
	  * public_ip_addr - private_ip_addr
	  * Here we are using 1's complement to represent -negative number.
	  * So take 1's complement of private ip addr and add it
	  * to public ip addr.
	  *
	  * Returns: >0 ip checksum diff
	  */
	uint16_t calc_ip_cksum_diff(uint32_t pub_ip_addr,
		uint32_t priv_ip_addr)
	{
		uint16_t ret;
		uint32_t cksum = 0;

		/* Add LSB(2 bytes) of public ip address to cksum */
		cksum += (pub_ip_addr & 0xFFFF);

		/* Add MSB(2 bytes) of public ip address to cksum
		and check for carry forward(CF), if any add it
		*/
		cksum += (pub_ip_addr >> 16);
		IPA_16BIT_ROUND_UP(cksum);

		/* Calculate the 1's complement of private ip address */
		priv_ip_addr = (~priv_ip_addr);

		/* Add LSB(2 bytes) of private ip address to cksum
		and check for carry forward(CF), if any add it
		*/
		cksum += (priv_ip_addr & 0xFFFF);
		IPA_16BIT_ROUND_UP(cksum);

		/* Add MSB(2 bytes) of private ip address to cksum
		and check for carry forward(CF), if any add it
		*/
		cksum += (priv_ip_addr >> 16);
		IPA_16BIT_ROUND_UP(cksum);

		/* Return the LSB(2 bytes) of checksum	*/
		ret = (uint16_t)cksum;
		return ret;
	}

	/**
	* calc_tcp_udp_cksum() - Calculate the source nat
	* TCP/UDP checksum diff
	* @pub_ip_addr: [in] public ip address
	* @pub_port: [in] public tcp/udp port
	* @priv_ip_addr: [in]	Private ip address
	* @priv_port: [in] Private tcp/udp prot
	*
	* source nat tcp/udp checksum is calculated as
	* (pub_ip_addr + pub_port) - (priv_ip_addr + priv_port)
	* Here we are using 1's complement to represent -ve number.
	* So take 1's complement of prviate ip addr &private port
	* and add it public ip addr & public port.
	*
	* Returns: >0 tcp/udp checksum diff
	*/
	uint16_t calc_tcp_udp_cksum_diff(uint32_t pub_ip_addr,
		uint16_t pub_port,
		uint32_t priv_ip_addr,
		uint16_t priv_port)
	{
		uint16_t ret = 0;
		uint32_t cksum = 0;

		/* Add LSB(2 bytes) of public ip address to cksum */
		cksum += (pub_ip_addr & 0xFFFF);

		/* Add MSB(2 bytes) of public ip address to cksum
		and check for carry forward(CF), if any add it
		*/
		cksum += (pub_ip_addr >> 16);
		IPA_16BIT_ROUND_UP(cksum);

		/* Add public port to cksum and
		check for carry forward(CF), if any add it */
		cksum += pub_port;
		IPA_16BIT_ROUND_UP(cksum);

		/* Calculate the 1's complement of private ip address */
		priv_ip_addr = (~priv_ip_addr);

		/* Add LSB(2 bytes) of private ip address to cksum
		and check for carry forward(CF), if any add it
		*/
		cksum += (priv_ip_addr & 0xFFFF);
		IPA_16BIT_ROUND_UP(cksum);

		/* Add MSB(2 bytes) of private ip address to cksum
		and check for carry forward(CF), if any add
		*/
		cksum += (priv_ip_addr >> 16);
		IPA_16BIT_ROUND_UP(cksum);

		/* Calculate the 1's complement of private port */
		priv_port = (~priv_port);

		/* Add public port to cksum and
		check for carry forward(CF), if any add it */
		cksum += priv_port;
		IPA_16BIT_ROUND_UP(cksum);

		/* return the LSB(2 bytes) of checksum */
		ret = (uint16_t)cksum;
		return ret;
	}

	~IpaNatBlockTestFixture()
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

	Byte m_sendBuffer[BUFF_MAX_SIZE];	// First input file / IP packet
	Byte m_sendBuffer2[BUFF_MAX_SIZE];	// Second input file / IP packet
	Byte m_sendBuffer3[BUFF_MAX_SIZE];	// Third input file (default) / IP packet
	size_t m_sendSize;
	size_t m_sendSize2;
	size_t m_sendSize3;
	enum ipa_ip_type m_IpaIPType;
	enum ipv6_ext_hdr_type m_extHdrType;
	uint32_t m_tbl_hdl;
	uint32_t m_nat_rule_hdl1;
	uint32_t m_public_ip;
	uint32_t m_public_ip2;
	uint32_t m_private_ip;
	uint32_t m_private_ip2;
	uint32_t m_target_ip;
	uint16_t m_public_port;
	uint16_t m_public_port2;
	uint16_t m_private_port;
	uint16_t m_private_port2;
	uint16_t m_target_port;
	uint32_t m_metadata;
private:
};

RoutingDriverWrapper IpaNatBlockTestFixture::m_routing;
Filtering IpaNatBlockTestFixture::m_filtering;

/*---------------------------------------------------------------------------*/
/* Test001: Single PDN src NAT test					     */
/* NOTE: other classes are derived from this class - change carefully        */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest001 : public IpaNatBlockTestFixture
{
public:
	IpaNatBlockTest001()
	{
		m_name = "IpaNatBlockTest001";
		m_description =
			"NAT block test 001 - single PDN src NAT test\
		1. Generate and commit three routing tables (only one is used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			action go to src NAT \
			All DST_IP == (193.23.22.1 & 0.255.255.255)traffic goes to NAT block \
		3. generate and commit one NAT rule:\
			private ip 194.23.22.1 --> public ip 192.23.22.1";
		m_private_ip = 0xC2171601; /* 194.23.22.1 */
		m_private_port = 5678;
		m_public_ip = 0xC0171601;   /* "192.23.22.1" */
		m_public_port = 9050;
		m_target_ip = 0xC1171601; /* 193.23.22.1 */
		m_target_port = 1234;
		Register(*this);
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0;

		if (!CreateThreeIPv4BypassRoutingTables(bypass0, bypass1, bypass2))
		{
			LOG_MSG_ERROR("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		LOG_MSG_DEBUG("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST_PROD, false, 1);
		LOG_MSG_DEBUG("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_SRC_NAT;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0x00FFFFFF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = m_target_ip; // Filter DST_IP == 193.23.22.1
		flt_rule_entry.rule.pdn_idx = 0;
		flt_rule_entry.rule.set_metadata = 0;
		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR("Error Adding Rule to Filter Table, aborting...\n");
			return false;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
		}

		//NAT table and rules creation
		int total_entries = 20;
		int ret;
		ipa_nat_ipv4_rule ipv4_rule;

		ret = ipa_nat_add_ipv4_tbl(m_public_ip, m_mem_type, total_entries, &m_tbl_hdl);
		if (ret) {
			LOG_MSG_DEBUG("failed creating NAT table\n");
			return false;
		}
		LOG_MSG_DEBUG("nat table added, hdl %d, public ip 0x%X\n", m_tbl_hdl,
			m_public_ip);

		ipv4_rule.target_ip = m_target_ip;
		ipv4_rule.target_port = m_target_port;
		ipv4_rule.private_ip = m_private_ip;
		ipv4_rule.private_port = m_private_port;
		ipv4_rule.protocol = IPPROTO_TCP;
		ipv4_rule.public_port = m_public_port;
		ipv4_rule.pdn_index = 0;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("failed adding NAT rule 0\n");
			return false;
		}
		LOG_MSG_DEBUG("NAT rule added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port);

		LOG_MSG_DEBUG("Leaving");
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint32_t address;
		uint16_t port;
		char flags = 0x18;

		address = htonl(m_target_ip);//193.23.22.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = htonl(m_private_ip);/* 194.23.22.1 */
		memcpy(&m_sendBuffer[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_private_port);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer[IPV4_TCP_FLAGS_OFFSET],&flags , sizeof(flags));
		return true;
	}// ModifyPacktes ()

	virtual bool SendPackets()
	{
		bool isSuccess = false;

		// Send first packet
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		LOG_MSG_DEBUG("sent successfully one packet\n");
		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];

		if (NULL == rxBuff1)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGoldenNat(
			m_sendBuffer, m_sendSize,
			rxBuff1, receivedSize,
			m_private_ip, m_public_ip,
			m_private_port, m_public_port,
			true))
		{
			LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		char recievedBuffer[256] = { 0 };
		char SentBuffer[256] = { 0 };
		size_t j;

		for (j = 0; j < m_sendSize; j++)
			snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for (j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);

		delete[] rxBuff1;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test002: Single PDN dst NAT test					     */
/* NOTE: other classes are derived from this class - change carefully        */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest002 : public IpaNatBlockTestFixture
{
public:
	IpaNatBlockTest002()
	{
		m_name = "IpaNatBlockTest002";
		m_description =
			"NAT block test 002 - single PDN dst NAT test\
		1. Generate and commit three routing tables (only one is used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			action go to dst NAT \
			All DST_IP == (192.23.22.1 & 0.255.255.255)traffic goes to NAT block (public IP filtering) \
		3. generate and commit one NAT rule:\
			public ip 192.23.22.1 --> private ip 194.23.22.1  ";
		m_private_ip = 0xC2171601; /* 194.23.22.1 */
		m_private_port = 5678;
		m_public_ip = 0xC0171601;   /* "192.23.22.1" */
		m_public_port = 9050;
		m_target_ip = 0xC1171601; /* 193.23.22.1 */
		m_target_port = 1234;
		Register(*this);
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0;

		if (!CreateThreeIPv4BypassRoutingTables(bypass0, bypass1, bypass2))
		{
			LOG_MSG_ERROR("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		LOG_MSG_DEBUG("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST_PROD, false, 3);
		LOG_MSG_DEBUG("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_DST_NAT;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0x00FFFFFF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = m_public_ip; // Filter DST_IP == 192.23.22.1
		flt_rule_entry.rule.pdn_idx = 0;
		flt_rule_entry.rule.set_metadata = 0;
		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR("Error Adding Rule to Filter Table, aborting...\n");
			return false;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
		}

		//NAT table and rules creation
		int total_entries = 20;
		int ret;
		ipa_nat_ipv4_rule ipv4_rule;
		uint32_t pub_ip_add = m_public_ip;

		ret = ipa_nat_add_ipv4_tbl(pub_ip_add, m_mem_type, total_entries, &m_tbl_hdl);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed creating NAT table\n");
			return false;
		}

		LOG_MSG_DEBUG("nat table added, hdl %d, public ip 0x%X\n", m_tbl_hdl,
			pub_ip_add);

		ipv4_rule.target_ip = m_target_ip;
		ipv4_rule.target_port = m_target_port;
		ipv4_rule.private_ip = m_private_ip;
		ipv4_rule.private_port = m_private_port;
		ipv4_rule.protocol = IPPROTO_TCP;
		ipv4_rule.public_port = m_public_port;
		ipv4_rule.pdn_index = 0;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 0\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port);

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint32_t address;
		uint16_t port;
		char flags = 0x18;

		address = htonl(m_public_ip);//192.23.22.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_public_port);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = htonl(m_target_ip);/* 193.23.22.1 */
		memcpy(&m_sendBuffer[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));

		return true;
	}// ModifyPacktes ()

	virtual bool SendPackets()
	{
		bool isSuccess = false;

		// Send first packet
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		LOG_MSG_DEBUG("sent successfully one packet\n");
		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];

		if (NULL == rxBuff1)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGoldenNat(
			m_sendBuffer, m_sendSize,
			rxBuff1, receivedSize,
			m_private_ip, m_public_ip,
			m_private_port, m_public_port,
			false))
		{
			LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		char recievedBuffer[256] = { 0 };
		char SentBuffer[256] = { 0 };
		size_t j;

		for (j = 0; j < m_sendSize; j++)
			snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for (j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);

		delete[] rxBuff1;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test003: Multi PDN src NAT test					     */
/* NOTE: other classes are derived from this class - change carefully        */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest003 : public IpaNatBlockTestFixture
{
public:
	IpaNatBlockTest003()
	{
		m_name = "IpaNatBlockTest003";
		m_description =
			"NAT block test 003 - Multi PDN src NAT test\
		1. Generate and commit three routing tables (two are used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit two filtering rule: (DST & Mask Match). \
			- action go to src NAT \
			  All SRC_IP == (194.23.22.1 & 0.255.255.255)traffic goes to NAT block \
			  All SRC_IP == (197.23.22.1 & 0.255.255.255)traffic goes to NAT block \
		3. generate and commit two NAT rules:\
			private ip 194.23.22.1 --> public ip 192.23.22.1 \
			private ip 197.23.22.1 --> public ip 195.23.22.1";
		m_private_ip = 0xC2171601; /* 194.23.22.1 */
		m_private_port = 5678;
		m_private_ip2 = 0xC5171601; /* 197.23.22.1 */
		m_private_port2 = 5679;
		m_public_ip = 0xC0171601;   /* "192.23.22.1" */
		m_public_port = 9050;
		m_public_ip2 = 0xC3171601;   /* "195.23.22.1" */
		m_public_port2 = 9051;
		m_target_ip = 0xC1171601; /* 193.23.22.1 */
		m_target_port = 1234;
		m_minIPAHwType = IPA_HW_v4_0;
		Register(*this);
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0, routing_table1;

		if (!CreateThreeIPv4BypassRoutingTables(bypass0, bypass1, bypass2))
		{
			LOG_MSG_ERROR("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		LOG_MSG_DEBUG("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n", &routing_table1);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass1, routing_table1.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST_PROD, false, 2);
		LOG_MSG_DEBUG("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_SRC_NAT;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_SRC_ADDR;
		flt_rule_entry.rule.attrib.u.v4.src_addr_mask = 0xFFFFFFFF; // Mask
		flt_rule_entry.rule.attrib.u.v4.src_addr = m_private_ip; // Filter SRC_IP == 194.23.22.1
		flt_rule_entry.rule.pdn_idx = 0;
		flt_rule_entry.rule.set_metadata = 0;
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			LOG_MSG_ERROR("Error Adding Rule to Filter Table, aborting...\n");
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl = routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.src_addr = m_private_ip2; // Filter SRC_IP == 197.23.22.1
		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR("Error Adding Rule to Filter Table, aborting...\n");
			return false;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
			LOG_MSG_DEBUG("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(1)->status);
		}

		//NAT table and rules creation
		int total_entries = 20;
		int ret;
		ipa_nat_ipv4_rule ipv4_rule;
		uint32_t pub_ip_add = m_public_ip;
		ipa_nat_pdn_entry pdn_info;

		// first create the NAT table
		ret = ipa_nat_add_ipv4_tbl(pub_ip_add, m_mem_type, total_entries, &m_tbl_hdl);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed creating NAT table\n");
			return false;
		}

		LOG_MSG_DEBUG("nat table added, hdl %d, public ip 0x%X\n", m_tbl_hdl,
			pub_ip_add);

		// modify the PDN entries that will be pointed by the NAT rules
		pdn_info.public_ip = m_public_ip;
		pdn_info.src_metadata = 0;
		pdn_info.dst_metadata = 0;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 0, &pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed Modifying PDN entry 0 \n");
			return false;
		}

		pdn_info.public_ip = m_public_ip2;
		pdn_info.src_metadata = 0;
		pdn_info.dst_metadata = 0;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 1, &pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed Modifying PDN entry 1 \n");
			return false;
		}

		ipv4_rule.target_ip = m_target_ip;
		ipv4_rule.target_port = m_target_port;
		ipv4_rule.private_ip = m_private_ip;
		ipv4_rule.private_port = m_private_port;
		ipv4_rule.protocol = IPPROTO_TCP;
		ipv4_rule.public_port = m_public_port;
		ipv4_rule.pdn_index = 0;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 0\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port);

		ipv4_rule.private_ip = m_private_ip2;
		ipv4_rule.private_port = m_private_port2;
		ipv4_rule.public_port = m_public_port2;
		ipv4_rule.pdn_index = 1;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 1\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule 2 added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port);

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint32_t address;
		uint16_t port;
		char flags = 0x18;

		//first packet private ip 194.23.22.1 --> public ip 192.23.22.1
		address = htonl(m_target_ip);//193.23.22.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = htonl(m_private_ip);/* 194.23.22.1 */
		memcpy(&m_sendBuffer[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_private_port);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));

		// second packet private ip 197.23.22.1 --> public ip 195.23.22.1
		address = htonl(m_target_ip);//193.23.22.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = htonl(m_private_ip2);/* 197.23.22.1 */
		memcpy(&m_sendBuffer2[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_private_port2);
		memcpy(&m_sendBuffer2[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer2[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));

		return true;
	}// ModifyPacktes ()

	virtual bool SendPackets()
	{
		bool isSuccess = false;

		// Send first packet
		LOG_MSG_DEBUG("sending first packet\n");
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		// Send second packet
		LOG_MSG_DEBUG("sending second packet\n");
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		LOG_MSG_DEBUG("sent successfully two packets\n");
		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];

		if (rxBuff1 == NULL)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			if (rxBuff2)
				delete[] rxBuff2;
			return false;
		}

		if (rxBuff2 == NULL)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			delete[] rxBuff1;
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		receivedSize2 = m_consumer2.ReceiveData(rxBuff2, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize2, m_consumer2.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGoldenNat(
			m_sendBuffer, m_sendSize,
			rxBuff1, receivedSize,
			m_private_ip, m_public_ip,
			m_private_port, m_public_port,
			true))
		{
			LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		char recievedBuffer[256] = { 0 };
		char SentBuffer[256] = { 0 };
		char recievedBuffer2[256] = { 0 };
		char SentBuffer2[256] = { 0 };
		size_t j;

		for (j = 0; j < m_sendSize; j++)
			snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for (j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);

		delete[] rxBuff1;

		isSuccess &= CompareResultVsGoldenNat(
			m_sendBuffer2, m_sendSize2,
			rxBuff2, receivedSize2,
			m_private_ip2, m_public_ip2,
			m_private_port2, m_public_port2,
			true);

		for (j = 0; j < m_sendSize2; j++)
			snprintf(&SentBuffer2[3 * j], sizeof(SentBuffer2) - (3 * j + 1), " %02X", m_sendBuffer2[j]);
		for (j = 0; j < receivedSize2; j++)
			snprintf(&recievedBuffer2[3 * j], sizeof(recievedBuffer2) - (3 * j + 1), " %02X", rxBuff2[j]);
		LOG_MSG_STACK("sent Value2 (%zu)\n%s\n, Received Value2(%zu)\n%s\n", m_sendSize2, SentBuffer2, receivedSize2, recievedBuffer2);

		delete[] rxBuff2;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test004: Multi PDN dst NAT test					     */
/* NOTE: other classes are derived from this class - change carefully        */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest004 : public IpaNatBlockTestFixture
{
public:
	IpaNatBlockTest004()
	{
		m_name = "IpaNatBlockTest004";
		m_description =
			"NAT block test 004 - Multi PDN dst NAT test\
		1. Generate and commit three routing tables (two are used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit two filtering rule: (DST & Mask Match). \
			- action go to dst NAT \
			  All DST_IP == (192.23.22.1 & 0.255.255.255)traffic goes to NAT block \
			  All DST_IP == (195.23.22.1 & 0.255.255.255)traffic goes to NAT block \
		3. generate and commit two NAT rules:\
			private ip 194.23.22.1 --> public ip 192.23.22.1 \
			private ip 197.23.22.1 --> public ip 195.23.22.1";
		m_private_ip = 0xC2171601; /* 194.23.22.1 */
		m_private_port = 5678;
		m_private_ip2 = 0xC5171601; /* 197.23.22.1 */
		m_private_port2 = 5679;
		m_public_ip = 0xC0171601;   /* "192.23.22.1" */
		m_public_port = 9050;
		m_public_ip2 = 0xC3171601;   /* "195.23.22.1" */
		m_public_port2 = 9051;
		m_target_ip = 0xC1171601; /* 193.23.22.1 */
		m_target_port = 1234;
		m_minIPAHwType = IPA_HW_v4_0;
		Register(*this);
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0, routing_table1;

		if (!CreateThreeIPv4BypassRoutingTables(bypass0, bypass1, bypass2))
		{
			LOG_MSG_ERROR("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		LOG_MSG_DEBUG("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n", &routing_table1);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass1, routing_table1.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST_PROD, false, 2);
		LOG_MSG_DEBUG("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_DST_NAT;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = m_public_ip; // Filter DST_IP == 192.23.22.1
		flt_rule_entry.rule.pdn_idx = 0;
		flt_rule_entry.rule.set_metadata = 0;
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			LOG_MSG_ERROR("Error Adding Rule to Filter Table, aborting...\n");
			return false;
		}

		// Configuring Filtering Rule No.1
		flt_rule_entry.rule.rt_tbl_hdl = routing_table1.hdl; //put here the handle corresponding to Routing Rule 2
		flt_rule_entry.rule.attrib.u.v4.dst_addr = m_public_ip2; // Filter DST_IP == 195.23.22.1
		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR("Error Adding Rule to Filter Table, aborting...\n");
			return false;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
			LOG_MSG_DEBUG("flt rule hdl1=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(1)->status);
		}

		//NAT table and rules creation
		int total_entries = 20;
		int ret;
		ipa_nat_ipv4_rule ipv4_rule;
		uint32_t pub_ip_add = m_public_ip;
		ipa_nat_pdn_entry pdn_info;

		// first create the NAT table
		ret = ipa_nat_add_ipv4_tbl(pub_ip_add, m_mem_type, total_entries, &m_tbl_hdl);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed creating NAT table\n");
			return false;
		}

		LOG_MSG_DEBUG("nat table added, hdl %d, public ip 0x%X\n", m_tbl_hdl,
			pub_ip_add);

		// modify the PDN entries that will be pointed by the NAT rules
		pdn_info.public_ip = m_public_ip;
		pdn_info.src_metadata = 0;
		pdn_info.dst_metadata = 0;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 0,&pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed Modifying PDN entry 0 \n");
			return false;
		}

		pdn_info.public_ip = m_public_ip2;
		pdn_info.src_metadata = 0;
		pdn_info.dst_metadata = 0;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 1, &pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed Modifying PDN entry 1 \n");
			return false;
		}

		ipv4_rule.target_ip = m_target_ip;
		ipv4_rule.target_port = m_target_port;
		ipv4_rule.private_ip = m_private_ip;
		ipv4_rule.private_port = m_private_port;
		ipv4_rule.protocol = IPPROTO_TCP;
		ipv4_rule.public_port = m_public_port;
		ipv4_rule.pdn_index = 0;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 0\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port);

		ipv4_rule.private_ip = m_private_ip2;
		ipv4_rule.private_port = m_private_port2;
		ipv4_rule.public_port = m_public_port2;
		ipv4_rule.pdn_index = 1;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 0\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule 2 added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port);

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint32_t address;
		uint16_t port;
		char flags = 0x18;

		//first packet private ip public ip 192.23.22.1 --> 194.23.22.1
		address = htonl(m_public_ip);//192.23.22.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_public_port);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = htonl(m_target_ip);/* 194.23.22.1 */
		memcpy(&m_sendBuffer[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));

		// second packet public ip 195.23.22.1--> private ip 197.23.22.1
		address = htonl(m_public_ip2);//193.23.22.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_public_port2);
		memcpy(&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = htonl(m_target_ip);/* 197.23.22.1 */
		memcpy(&m_sendBuffer2[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer2[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer2[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));

		return true;
	}// ModifyPacktes ()

	virtual bool SendPackets()
	{
		bool isSuccess = false;

		// Send first packet
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		// Send second packet
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		LOG_MSG_DEBUG("sent successfully two packets\n");
		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];

		if (rxBuff1 == NULL)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			if (rxBuff2)
				delete[] rxBuff2;
			return false;
		}

		if (rxBuff2 == NULL)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			delete[] rxBuff1;
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		receivedSize2 = m_consumer2.ReceiveData(rxBuff2, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize2, m_consumer2.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGoldenNat(
			m_sendBuffer, m_sendSize,
			rxBuff1, receivedSize,
			m_private_ip, m_public_ip,
			m_private_port, m_public_port,
			false))
		{
			LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		char recievedBuffer[256] = { 0 };
		char SentBuffer[256] = { 0 };
		char recievedBuffer2[256] = { 0 };
		char SentBuffer2[256] = { 0 };
		size_t j;

		for (j = 0; j < m_sendSize; j++)
			snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for (j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);

		delete[] rxBuff1;

		isSuccess &= CompareResultVsGoldenNat(
			m_sendBuffer2, m_sendSize2,
			rxBuff2, receivedSize2,
			m_private_ip2, m_public_ip2,
			m_private_port2, m_public_port2,
			false);

		for (j = 0; j < m_sendSize2; j++)
			snprintf(&SentBuffer2[3 * j], sizeof(SentBuffer2) - (3 * j + 1), " %02X", m_sendBuffer2[j]);
		for (j = 0; j < receivedSize2; j++)
			snprintf(&recievedBuffer2[3 * j], sizeof(recievedBuffer2) - (3 * j + 1), " %02X", rxBuff2[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize2, SentBuffer2, receivedSize2, recievedBuffer2);

		delete[] rxBuff2;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test005: Single PDN src metadata replacement NAT test		     */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest005 : public IpaNatBlockTestFixture
{
public:
	IpaNatBlockTest005()
	{
		m_name = "IpaNatBlockTest005";
		m_description =
			"NAT block test 005 - single PDN src metadata replacement NAT test\
			source metadata will be replaced and the routing rule equation will be done upon replaced value\
		1. Generate and commit two routing tables (only one is used). \
			the routing table will catch packets with metadata value 0x34567890 (different from original value)\
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			action go to src NAT \
			All DST_IP == (193.23.22.1 & 0.255.255.255)traffic goes to NAT block \
			action parameters metadata replacement = true\
		3. generate and commit one NAT rule:\
			private ip 194.23.22.1 --> public ip 192.23.22.1\
			source metadata value shall be replaced to 0x34567890 (caught by the routing rule)";
		m_private_ip = 0xC2171601; /* 194.23.22.1 */
		m_private_port = 5678;
		m_public_ip = 0xC0171601;   /* "192.23.22.1" */
		m_public_port = 9050;
		m_target_ip = 0xC1171601; /* 193.23.22.1 */
		m_target_port = 1234;
		m_metadata = 0x34567890;
		m_minIPAHwType = IPA_HW_v4_0;
		Register(*this);
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		const char bypass0[20] = "Bypass0";
		struct ipa_ioc_get_rt_tbl routing_table0;

		if (!CreateMetdataRoutingRule(bypass0))
		{
			LOG_MSG_ERROR("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		LOG_MSG_DEBUG("CreateMetdataRoutingRule completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST2_PROD, false, 1);
		LOG_MSG_DEBUG("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_SRC_NAT;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = m_target_ip; // Filter DST_IP == 193.23.22.1
		flt_rule_entry.rule.pdn_idx = 0;
		flt_rule_entry.rule.set_metadata = 1;
		flt_rule_entry.rule.retain_hdr = 1;
		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR("Error Adding Rule to Filter Table, aborting...\n");
			return false;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
		}

		//NAT table and rules creation
		int total_entries = 20;
		int ret;
		ipa_nat_ipv4_rule ipv4_rule;
		ipa_nat_pdn_entry pdn_info;

		ret = ipa_nat_add_ipv4_tbl(m_public_ip, m_mem_type, total_entries, &m_tbl_hdl);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed creating NAT table\n");
			return false;
		}
		LOG_MSG_DEBUG("nat table added, hdl %d, public ip 0x%X\n", m_tbl_hdl,
			m_public_ip);

		ipv4_rule.target_ip = m_target_ip;
		ipv4_rule.target_port = m_target_port;
		ipv4_rule.private_ip = m_private_ip;
		ipv4_rule.private_port = m_private_port;
		ipv4_rule.protocol = IPPROTO_TCP;
		ipv4_rule.public_port = m_public_port;
		ipv4_rule.pdn_index = 0;

		// modify the PDN entries that will be pointed by the NAT rules
		pdn_info.public_ip = m_public_ip;
		pdn_info.src_metadata = m_metadata;
		pdn_info.dst_metadata = 0;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 0, &pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed modifying PDN index 0\n");
			return false;
		}
		LOG_MSG_DEBUG("PDN 0 was modified to hold ip 0x%X, src_metadata 0x%X\n", m_public_ip, m_metadata);

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 0\n");
			return false;
		}
		LOG_MSG_DEBUG("NAT rule added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port);

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint32_t address;
		uint16_t port;
		char flags = 0x18;

		Load8021QPacket();

		address = htonl(m_target_ip);//193.23.22.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET + ETH8021Q_HEADER_LEN], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET + ETH8021Q_HEADER_LEN], &port, sizeof(port));

		address = htonl(m_private_ip);/* 194.23.22.1 */
		memcpy(&m_sendBuffer[IPV4_SRC_ADDR_OFFSET + ETH8021Q_HEADER_LEN], &address, sizeof(address));
		port = htons(m_private_port);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET + ETH8021Q_HEADER_LEN], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer[IPV4_TCP_FLAGS_OFFSET + ETH8021Q_HEADER_LEN], &flags, sizeof(flags));
		return true;
	}// ModifyPacktes ()

	virtual bool SendPackets()
	{
		bool isSuccess = false;

		// Send first packet
		isSuccess = m_producer2.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		LOG_MSG_DEBUG("sent successfully one packet\n");
		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];

		if (NULL == rxBuff1)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGoldenNat(
			m_sendBuffer, m_sendSize,
			rxBuff1, receivedSize,
			m_private_ip, m_public_ip,
			m_private_port, m_public_port,
			true, ETH8021Q_HEADER_LEN))
		{
			LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		char recievedBuffer[256] = { 0 };
		char SentBuffer[256] = { 0 };
		size_t j;

		for (j = 0; j < m_sendSize; j++)
			snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for (j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);

		delete[] rxBuff1;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test006: Single PDN dst metadata replacement NAT test		     */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest006 : public IpaNatBlockTestFixture
{
public:
	IpaNatBlockTest006()
	{
		m_name = "IpaNatBlockTest006";
		m_description =
			"NAT block test 006 - single PDN dst metadata replacement NAT test\
			destination metadata will be replaced and the routing rule equation will be done upon replaced value\
		1. Generate and commit two routing tables (only one is used). \
			the routing table will catch packets with metadata value 0x34567890 (different from original value)\
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			action go to dst NAT \
			All DST_IP == (192.23.22.1 & 0.255.255.255)traffic goes to NAT block \
			action parameters metadata replacement = true\
		3. generate and commit one NAT rule:\
			public ip 192.23.22.1 --> private ip 194.23.22.1 \
			destination metadata value shall be replaced to 0x34567890 (caught by the routing rule)";
		m_private_ip = 0xC2171601; /* 194.23.22.1 */
		m_private_port = 5678;
		m_public_ip = 0xC0171601;   /* "192.23.22.1" */
		m_public_port = 9050;
		m_target_ip = 0xC1171601; /* 193.23.22.1 */
		m_target_port = 1234;
		m_metadata = 0x34567890;
		m_minIPAHwType = IPA_HW_v4_0;
		Register(*this);
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		const char bypass0[20] = "Bypass0";
		struct ipa_ioc_get_rt_tbl routing_table0;

		if (!CreateMetdataRoutingRule(bypass0))
		{
			LOG_MSG_ERROR("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		LOG_MSG_DEBUG("CreateMetdataRoutingRule completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST2_PROD, false, 1);
		LOG_MSG_DEBUG("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_DST_NAT;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0xFFFFFFFF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = m_public_ip; // Filter DST_IP == 193.23.22.1
		flt_rule_entry.rule.pdn_idx = 0;
		flt_rule_entry.rule.set_metadata = 1;
		flt_rule_entry.rule.retain_hdr = 1;
		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR("Error Adding Rule to Filter Table, aborting...\n");
			return false;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
		}

		//NAT table and rules creation
		int total_entries = 20;
		int ret;
		ipa_nat_ipv4_rule ipv4_rule;
		ipa_nat_pdn_entry pdn_info;

		ret = ipa_nat_add_ipv4_tbl(m_public_ip, m_mem_type, total_entries, &m_tbl_hdl);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed creating NAT table\n");
			return false;
		}
		LOG_MSG_DEBUG("nat table added, hdl %d, public ip 0x%X\n", m_tbl_hdl,
			m_public_ip);

		ipv4_rule.target_ip = m_target_ip;
		ipv4_rule.target_port = m_target_port;
		ipv4_rule.private_ip = m_private_ip;
		ipv4_rule.private_port = m_private_port;
		ipv4_rule.protocol = IPPROTO_TCP;
		ipv4_rule.public_port = m_public_port;
		ipv4_rule.pdn_index = 0;

		// modify the PDN entries that will be pointed by the NAT rules
		pdn_info.public_ip = m_public_ip;
		pdn_info.src_metadata = 0;
		pdn_info.dst_metadata = m_metadata;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 0, &pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed modifying PDN index 0\n");
			return false;
		}
		LOG_MSG_DEBUG("PDN 0 was modified to hold ip 0x%X, dst_metadata 0x%X\n", m_public_ip, m_metadata);

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 0\n");
			return false;
		}
		LOG_MSG_ERROR("NAT rule added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port);

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint32_t address;
		uint16_t port;
		char flags = 0x18;

		Load8021QPacket();

		address = htonl(m_public_ip);//193.23.22.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET + ETH8021Q_HEADER_LEN], &address, sizeof(address));
		port = htons(m_public_port);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET + ETH8021Q_HEADER_LEN], &port, sizeof(port));

		address = htonl(m_target_ip);/* 194.23.22.1 */
		memcpy(&m_sendBuffer[IPV4_SRC_ADDR_OFFSET + ETH8021Q_HEADER_LEN], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET + ETH8021Q_HEADER_LEN], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer[IPV4_TCP_FLAGS_OFFSET + ETH8021Q_HEADER_LEN], &flags, sizeof(flags));
		return true;
	}// ModifyPacktes ()

	virtual bool SendPackets()
	{
		bool isSuccess = false;

		// Send first packet
		isSuccess = m_producer2.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		LOG_MSG_DEBUG("sent successfully one packet\n");
		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];

		if (NULL == rxBuff1)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGoldenNat(
			m_sendBuffer, m_sendSize,
			rxBuff1, receivedSize,
			m_private_ip, m_public_ip,
			m_private_port, m_public_port,
			false, ETH8021Q_HEADER_LEN))
		{
			LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		char recievedBuffer[256] = { 0 };
		char SentBuffer[256] = { 0 };
		size_t j;

		for (j = 0; j < m_sendSize; j++)
			snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for (j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);

		delete[] rxBuff1;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test007: Hashable routing rule with dst NAT test                          */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest007 : public IpaNatBlockTestFixture
{
public:
	IpaNatBlockTest007()
	{
		m_name = "IpaNatBlockTest007";
		m_description =
			"NAT block test 007 - single PDN dst NAT with hashable routing rule test\
			test if routing block hash mechanism tests NATed values or pre NAT values\
		1. Generate and commit routing table with two hashable rules. \
			first routing rule will send packets with ip == 192.168.9.119 to first pipe \
			second routing rule will send packets with ip == 192.168.9.120 to second pipe\
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			action go to dst NAT \
			All DST_IP == (192.168.9.1 & 255.255.255.0)traffic goes to NAT block \
		3. generate and commit two NAT rules with target ip 211.1.1.4:\
			1. public ip 5.5.6.120 --> private ip 192.168.9.119 \
			   public port 4501 --> private port 4500 \
			2.  public ip 5.5.6.120 --> private ip 192.168.9.119 \
			    public port 4502 --> private port 4500";
		m_private_ip = 0xC0A80977; /* 192.168.9.119 */
		m_private_port = 4500;
		m_private_ip2 = 0xC0A80978; /* 192.168.9.120 */
		m_private_port2 = 4500;
		m_public_ip = 0x05050678;   /* 5.5.6.120 */
		m_public_port = 4501;
		m_public_port2 = 4502;
		m_target_ip = 0xD3010104; /* 211.1.1.4 */
		m_target_port = 1234;
		m_minIPAHwType = IPA_HW_v4_0;
		Register(*this);
	}

	bool Setup()
	{
		return IpaNatBlockTestFixture::Setup(true);
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		const char bypass0[20] = "Bypass0";
		struct ipa_ioc_get_rt_tbl routing_table0;

		if (!CreateHashableRoutingRules(bypass0))
		{
			LOG_MSG_ERROR("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		LOG_MSG_DEBUG("CreateHashableRoutingRules completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST_PROD, false, 1);
		LOG_MSG_DEBUG("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_DST_NAT;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl; //put here the handle corresponding to Routing table 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0x00000000; // Mask - catch all
		flt_rule_entry.rule.attrib.u.v4.dst_addr = m_public_ip; // Filter DST_IP == 5.5.6.120
		flt_rule_entry.rule.pdn_idx = 0;
		flt_rule_entry.rule.set_metadata = 0;
		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR("Error Adding Rule to Filter Table, aborting...\n");
			return false;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
		}

		//NAT table and rules creation
		int total_entries = 20;
		int ret;
		ipa_nat_ipv4_rule ipv4_rule;

		ret = ipa_nat_add_ipv4_tbl(m_public_ip, m_mem_type, total_entries, &m_tbl_hdl);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed creating NAT table\n");
			return false;
		}
		LOG_MSG_DEBUG("nat table added, hdl %d, public ip 0x%X\n", m_tbl_hdl,
			m_public_ip);

		ipv4_rule.target_ip = m_target_ip;
		ipv4_rule.target_port = m_target_port;
		ipv4_rule.private_ip = m_private_ip;
		ipv4_rule.private_port = m_private_port;
		ipv4_rule.protocol = IPPROTO_TCP;
		ipv4_rule.public_port = m_public_port;
		ipv4_rule.pdn_index = 0;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 0\n");
			return false;
		}
		LOG_MSG_DEBUG("NAT rule 1 added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port);

		ipv4_rule.target_ip = m_target_ip;
		ipv4_rule.target_port = m_target_port;
		ipv4_rule.private_ip = m_private_ip2;
		ipv4_rule.private_port = m_private_port2;
		ipv4_rule.protocol = IPPROTO_TCP;
		ipv4_rule.public_port = m_public_port2;
		ipv4_rule.pdn_index = 0;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 0\n");
			return false;
		}
		LOG_MSG_DEBUG("NAT rule 2 added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port);

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint32_t address;
		uint16_t port;
		char flags = 0x18;

		address = ntohl(m_public_ip);//5.5.6.120
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = ntohs(m_public_port); // 4501
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = ntohl(m_target_ip);/* 211.1.1.4 */
		memcpy(&m_sendBuffer[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = ntohs(m_target_port); // 4500
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));

		// second packet
		address = ntohl(m_public_ip);//5.5.6.120
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = ntohs(m_public_port2); // 4502
		memcpy(&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = ntohl(m_target_ip);/* 211.1.1.4 */
		memcpy(&m_sendBuffer2[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = ntohs(m_target_port); // 4500
		memcpy(&m_sendBuffer2[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer2[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));
		return true;
	}// ModifyPacktes ()

	virtual bool SendPackets()
	{
		bool isSuccess = false;

		// Send first packet
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		LOG_MSG_DEBUG("sent successfully the first packet\n");

		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		LOG_MSG_DEBUG("sent successfully two packet\n");
		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];

		if (rxBuff1 == NULL || rxBuff2 == NULL)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		receivedSize2 = m_consumer2.ReceiveData(rxBuff2, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize2, m_consumer2.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGoldenNat(
			m_sendBuffer, m_sendSize,
			rxBuff1, receivedSize,
			m_private_ip, m_public_ip,
			m_private_port, m_public_port,
			false, 0, true))
		{
			LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize, receivedSize, rxBuff1) : IsCacheMiss(m_sendSize, receivedSize, rxBuff1);

		char recievedBuffer[0x400] = { 0 };
		char SentBuffer[0x400] = { 0 };
		char recievedBuffer2[0x400] = { 0 };
		char SentBuffer2[0x400] = { 0 };
		size_t j;

		for (j = 0; j < m_sendSize; j++)
			snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for (j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);

		delete[] rxBuff1;

		isSuccess &= CompareResultVsGoldenNat(
			m_sendBuffer2, m_sendSize2,
			rxBuff2, receivedSize2,
			m_private_ip2, m_public_ip,
			m_private_port2, m_public_port2,
			false, 0, true);

		isSuccess &= (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v5_0) ?
			IsCacheMiss_v5_0(m_sendSize2, receivedSize2, rxBuff2) : IsCacheMiss(m_sendSize2, receivedSize2, rxBuff2);

		for (j = 0; j < m_sendSize2; j++)
			snprintf(&SentBuffer2[3 * j], sizeof(SentBuffer2) - (3 * j + 1), " %02X", m_sendBuffer2[j]);
		for (j = 0; j < receivedSize2; j++)
			snprintf(&recievedBuffer2[3 * j], sizeof(recievedBuffer2) - (3 * j + 1), " %02X", rxBuff2[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize2, SentBuffer2, receivedSize2, recievedBuffer2);

		delete[] rxBuff2;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test008: Multi PDN src NAT test match PDN by input from filtering block   */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest008 : public IpaNatBlockTestFixture
{
public:
	IpaNatBlockTest008()
	{
		m_name = "IpaNatBlockTest008";
		m_description =
			"NAT block test 008 - Multi PDN src NAT test match PDN by input from filtering block\
		1. Generate and commit three routing tables (two are used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			- action go to src NAT + PDN index 2\
			  All SRC_IP == (194.23.22.1 & 0.255.255.255)traffic goes to NAT block \
		3. generate and commit two NAT rules:\
			private ip 194.23.22.1 --> public ip 192.23.22.1 PDN index 1\
			private ip 194.23.22.1 --> public ip 195.23.22.1 PDN index 2";
		m_private_ip = 0xC2171601; /* 194.23.22.1 */
		m_private_port = 5678;
		m_public_ip = 0xC0171601;   /* "192.23.22.1" */
		m_public_port = 9050;
		m_public_ip2 = 0xC3171601;   /* "195.23.22.1" */
		m_target_ip = 0xC1171601; /* 193.23.22.1 */
		m_target_port = 1234;
		m_minIPAHwType = IPA_HW_v4_0;
		Register(*this);
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0, routing_table1;

		if (!CreateThreeIPv4BypassRoutingTables(bypass0, bypass1, bypass2))
		{
			LOG_MSG_ERROR("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		LOG_MSG_DEBUG("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n", &routing_table1);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass1, routing_table1.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST_PROD, false, 2);
		LOG_MSG_DEBUG("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_SRC_NAT;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_SRC_ADDR;
		flt_rule_entry.rule.attrib.u.v4.src_addr_mask = 0xFFFFFFFF; // Mask
		flt_rule_entry.rule.attrib.u.v4.src_addr = m_private_ip; // Filter SRC_IP == 194.23.22.1
		flt_rule_entry.rule.pdn_idx = 2;
		flt_rule_entry.rule.set_metadata = 0;
		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR("Error Adding Rule to Filter Table, aborting...\n");
			return false;
		}
		else
		{
			LOG_MSG_ERROR("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
		}

		//NAT table and rules creation
		int total_entries = 20;
		int ret;
		ipa_nat_ipv4_rule ipv4_rule;
		uint32_t pub_ip_add = m_public_ip;
		ipa_nat_pdn_entry pdn_info;

		// first create the NAT table
		ret = ipa_nat_add_ipv4_tbl(pub_ip_add, m_mem_type, total_entries, &m_tbl_hdl);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed creating NAT table\n");
			return false;
		}

		LOG_MSG_DEBUG("nat table added, hdl %d, public ip 0x%X\n", m_tbl_hdl,
			pub_ip_add);

		// modify the PDN entries that will be pointed by the NAT rules
		pdn_info.public_ip = m_public_ip;
		pdn_info.src_metadata = 0;
		pdn_info.dst_metadata = 0;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 1, &pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed Modifying PDN entry 0 \n");
			return false;
		}

		pdn_info.public_ip = m_public_ip2;
		pdn_info.src_metadata = 0;
		pdn_info.dst_metadata = 0;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 2, &pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed Modifying PDN entry 1 \n");
			return false;
		}

		ipv4_rule.target_ip = m_target_ip;
		ipv4_rule.target_port = m_target_port;
		ipv4_rule.private_ip = m_private_ip;
		ipv4_rule.private_port = m_private_port;
		ipv4_rule.protocol = IPPROTO_TCP;
		ipv4_rule.public_port = m_public_port;
		ipv4_rule.pdn_index = 1;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 0\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port);

		// the second rule shall be identical to the first on all parameters except PDN index so the filtering
		// block action parameter will provide the PDN index.
		ipv4_rule.pdn_index = 2;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 1\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule 2 added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port);

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint32_t address;
		uint16_t port;
		char flags = 0x18;

		//first packet private ip 194.23.22.1 --> public ip 195.23.22.1
		address = htonl(m_target_ip);//193.23.22.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = htonl(m_private_ip);/* 194.23.22.1 */
		memcpy(&m_sendBuffer[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_private_port);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));

		return true;
	}// ModifyPacktes ()

	virtual bool SendPackets()
	{
		bool isSuccess = false;

		// Send first packet
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}
		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];

		if (rxBuff1 == NULL)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		// Compare results - verify that the ip of PDN 2 was selected (m_public_ip2)
		if (!CompareResultVsGoldenNat(
			m_sendBuffer, m_sendSize,
			rxBuff1, receivedSize,
			m_private_ip, m_public_ip2,
			m_private_port, m_public_port,
			true))
		{
			LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		char recievedBuffer[256] = { 0 };
		char SentBuffer[256] = { 0 };
		size_t j;

		for (j = 0; j < m_sendSize; j++)
			snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for (j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);

		delete[] rxBuff1;
		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test009: Single PDN src NAT delete rule test                              */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest009 : public IpaNatBlockTest001
{
public:
	IpaNatBlockTest009()
	{
		m_name = "IpaNatBlockTest009";
		m_description =
			"NAT block test 009 - single PDN src NAT rule deletion test\
		1. Generate and commit three routing tables (only one is used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			action go to src NAT \
			All DST_IP == (193.23.22.1 & 0.255.255.255)traffic goes to NAT block \
		3. generate and commit one NAT rule:\
			private ip 194.23.22.1 --> public ip 192.23.22.1\
		4. delete the NAT rule and expect NAT miss";
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		int ret = IpaNatBlockTest001::AddRules();
		if (!ret) {
			LOG_MSG_ERROR("Leaving, failed Adding test 001 rules 0\n");
			return false;
		}

		ret = ipa_nat_del_ipv4_rule(m_tbl_hdl, m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed deleting NAT rule 0\n");
			return false;
		}
		LOG_MSG_DEBUG("NAT rule deleted\n");

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];

		if (NULL == rxBuff1)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		if (receivedSize) {
			LOG_MSG_ERROR("Data received - test failed!\n");
			isSuccess = false;

			// Compare results
			if (!CompareResultVsGoldenNat(
				m_sendBuffer, m_sendSize,
				rxBuff1, receivedSize,
				m_private_ip, m_public_ip,
				m_private_port, m_public_port,
				true))
			{
				LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			} else {
				LOG_MSG_ERROR("Comparison of Buffer0 succeeded - NAT rule was hit despite deletion!\n");
			}

			char recievedBuffer[256] = { 0 };
			char SentBuffer[256] = { 0 };
			size_t j;

			for (j = 0; j < m_sendSize; j++)
				snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
			for (j = 0; j < receivedSize; j++)
				snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
			LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);
		}

		delete[] rxBuff1;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test010: Single PDN dst NAT rule deletion test                            */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest010 : public IpaNatBlockTest002
{
public:
	IpaNatBlockTest010()
	{
		m_name = "IpaNatBlockTest010";
		m_description =
			"NAT block test 010 - single PDN dst NAT rule deletion test\
		1. Generate and commit three routing tables (only one is used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			action go to dst NAT \
			All DST_IP == (192.23.22.1 & 0.255.255.255)traffic goes to NAT block (public IP filtering) \
		3. generate and commit one NAT rule:\
			public ip 192.23.22.1 --> private ip 194.23.22.1  \
			delete rule and verrify NAT miss";
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		int ret = IpaNatBlockTest002::AddRules();
		if (!ret) {
			LOG_MSG_ERROR("Leaving, failed Adding test 002 rules 0\n");
			return false;
		}

		ret = ipa_nat_del_ipv4_rule(m_tbl_hdl, m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed deleting NAT rule 0\n");
			return false;
		}
		LOG_MSG_DEBUG("NAT rule deleted\n");

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];

		if (NULL == rxBuff1)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		if (receivedSize) {
			LOG_MSG_ERROR("Data received - test failed!\n");
			isSuccess = false;

			// Compare results
			if (!CompareResultVsGoldenNat(
				m_sendBuffer, m_sendSize,
				rxBuff1, receivedSize,
				m_private_ip, m_public_ip,
				m_private_port, m_public_port,
				false))
			{
				LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			}
			else {
				LOG_MSG_ERROR("Comparison of Buffer0 succeeded - NAT rule was hit despite deletion!\n");
			}

			char recievedBuffer[256] = { 0 };
			char SentBuffer[256] = { 0 };
			size_t j;

			for (j = 0; j < m_sendSize; j++)
				snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
			for (j = 0; j < receivedSize; j++)
				snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
			LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);
		}

		delete[] rxBuff1;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test011: Multi PDN src NAT - MAX number of PDNs test                      */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest011 : public IpaNatBlockTestFixture
{
	uint32_t m_public_ip3;
	uint32_t m_public_ip4;
	uint32_t m_private_ip3;
	uint32_t m_private_ip4;
	uint16_t m_public_port3;
	uint16_t m_public_port4;
	uint16_t m_private_port3;
	uint16_t m_private_port4;
	Byte m_sendBuffer4[BUFF_MAX_SIZE];
	size_t m_sendSize4;

public:
	IpaNatBlockTest011()
	{
		m_name = "IpaNatBlockTest011";
		m_description =
			"NAT block test 011 - Multi PDN src NAT test\
		1. Generate and commit three routing tables (one is used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			- action go to src NAT \
			  All SRC_IP == (192.23.22.1 & 0.255.255.255)traffic goes to NAT block \
			  All SRC_IP == (194.23.22.1 & 0.255.255.255)traffic goes to NAT block \
			  All SRC_IP == (196.23.22.1 & 0.255.255.255)traffic goes to NAT block \
			  All SRC_IP == (198.23.22.1 & 0.255.255.255)traffic goes to NAT block \
		3. generate and commit four NAT rules:\
			private ip 192.23.22.1 --> public ip 193.23.22.1 \
			private ip 194.23.22.1 --> public ip 195.23.22.1 \
			private ip 196.23.22.1 --> public ip 197.23.22.1 \
			private ip 198.23.22.1 --> public ip 199.23.22.1";
		m_private_ip = 0xC0171601; /* 192.23.22.1 */
		m_private_port = 5678;
		m_public_ip = 0xC1171601;   /* "193.23.22.1" */
		m_public_port = 9050;

		m_private_ip2 = 0xC2171601; /* 194.23.22.1 */
		m_private_port2 = 5679;
		m_public_ip2 = 0xC3171601;   /* "195.23.22.1" */
		m_public_port2 = 9051;

		m_private_ip3 = 0xC4171601; /* 196.23.22.1 */
		m_private_port3 = 5680;
		m_public_ip3 = 0xC5171601;   /* "197.23.22.1" */
		m_public_port3 = 9052;

		m_private_ip4 = 0xC6171601; /* 198.23.22.1 */
		m_private_port4 = 5681;
		m_public_ip4 = 0xC7171601;   /* "199.23.22.1" */
		m_public_port4 = 9053;

		m_target_ip = 0xBF171601; /* 191.23.22.1 */
		m_target_port = 1234;

		m_sendSize4 = BUFF_MAX_SIZE;
		m_minIPAHwType = IPA_HW_v4_0;
		Register(*this);
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0, routing_table1;

		if (!CreateThreeIPv4BypassRoutingTables(bypass0, bypass1, bypass2))
		{
			LOG_MSG_ERROR("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		LOG_MSG_DEBUG("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table1=0x%p) Failed.\n", &routing_table1);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass1, routing_table1.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST_PROD, false, 1);
		LOG_MSG_DEBUG("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_SRC_NAT;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_SRC_ADDR;
		flt_rule_entry.rule.attrib.u.v4.src_addr_mask = 0x00FFFFFF; // Mask - catch all private IPs
		flt_rule_entry.rule.attrib.u.v4.src_addr = m_private_ip; // Filter SRC_IP == 192.23.22.1
		flt_rule_entry.rule.pdn_idx = 0;
		flt_rule_entry.rule.set_metadata = 0;
		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR("Error Adding Rule to Filter Table, aborting...\n");
			return false;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
		}

		//NAT table and rules creation
		int total_entries = 20;
		int ret;
		ipa_nat_ipv4_rule ipv4_rule;
		ipa_nat_pdn_entry pdn_info;

		// first create the NAT table
		ret = ipa_nat_add_ipv4_tbl(m_public_ip, m_mem_type, total_entries, &m_tbl_hdl);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed creating NAT table\n");
			return false;
		}

		LOG_MSG_DEBUG("nat table added, hdl %d, public ip 0x%X\n", m_tbl_hdl,
			m_public_ip);

		// modify the PDN entries that will be pointed by the NAT rules
		pdn_info.public_ip = m_public_ip;
		pdn_info.src_metadata = 0;
		pdn_info.dst_metadata = 0;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 0, &pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed Modifying PDN entry 0 \n");
			return false;
		}

		pdn_info.public_ip = m_public_ip2;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 1, &pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed Modifying PDN entry 1 \n");
			return false;
		}

		pdn_info.public_ip = m_public_ip3;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 2, &pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed Modifying PDN entry 2 \n");
			return false;
		}

		pdn_info.public_ip = m_public_ip4;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 3, &pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed Modifying PDN entry 3 \n");
			return false;
		}

		LOG_MSG_DEBUG("Added 4 PDNs successfully: 0x%X, 0x%X, 0x%X, 0x%X\n",
			m_public_ip, m_public_ip2, m_public_ip3, m_public_ip4);

		ipv4_rule.target_ip = m_target_ip;
		ipv4_rule.target_port = m_target_port;
		ipv4_rule.private_ip = m_private_ip;
		ipv4_rule.private_port = m_private_port;
		ipv4_rule.protocol = IPPROTO_TCP;
		ipv4_rule.public_port = m_public_port;
		ipv4_rule.pdn_index = 0;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 0\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d, $d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port, ipv4_rule.pdn_index);

		ipv4_rule.private_ip = m_private_ip2;
		ipv4_rule.private_port = m_private_port2;
		ipv4_rule.public_port = m_public_port2;
		ipv4_rule.pdn_index = 1;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 1\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule 2 added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d, $d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port, ipv4_rule.pdn_index);

		ipv4_rule.private_ip = m_private_ip3;
		ipv4_rule.private_port = m_private_port3;
		ipv4_rule.public_port = m_public_port3;
		ipv4_rule.pdn_index = 2;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 2\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule 3 added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d, $d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port, ipv4_rule.pdn_index);

		ipv4_rule.private_ip = m_private_ip4;
		ipv4_rule.private_port = m_private_port4;
		ipv4_rule.public_port = m_public_port4;
		ipv4_rule.pdn_index = 3;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 3\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule 4 added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d, $d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port, ipv4_rule.pdn_index);

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint32_t address;
		uint16_t port;
		char flags = 0x18;

		if (!LoadDefaultPacket(IPA_IP_v4, m_extHdrType, m_sendBuffer4, m_sendSize4)) {
			LOG_MSG_ERROR("Failed default Packet buffer 4\n");
			return false;
		}
		LOG_MSG_DEBUG("Loaded %zu Bytes to Buffer 4\n", m_sendSize4);

		//first packet private ip 192.23.22.1 --> public ip 193.23.22.1
		address = htonl(m_target_ip);//191.23.22.1
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = htonl(m_private_ip);/* 192.23.22.1 */
		memcpy(&m_sendBuffer[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_private_port);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));

		// second packet private ip 194.23.22.1 --> public ip 195.23.22.1
		address = htonl(m_target_ip);//191.23.22.1
		memcpy(&m_sendBuffer2[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer2[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = htonl(m_private_ip2);/* 194.23.22.1 */
		memcpy(&m_sendBuffer2[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_private_port2);
		memcpy(&m_sendBuffer2[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer2[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));

		// third packet private ip 196.23.22.1 --> public ip 197.23.22.1
		address = htonl(m_target_ip);//191.23.22.1
		memcpy(&m_sendBuffer3[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer3[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = htonl(m_private_ip3);/* 196.23.22.1 */
		memcpy(&m_sendBuffer3[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_private_port3);
		memcpy(&m_sendBuffer3[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer3[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));

		// third packet private ip 198.23.22.1 --> public ip 199.23.22.1
		address = htonl(m_target_ip);//191.23.22.1
		memcpy(&m_sendBuffer4[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_target_port);
		memcpy(&m_sendBuffer4[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = htonl(m_private_ip4);/* 198.23.22.1 */
		memcpy(&m_sendBuffer4[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_private_port4);
		memcpy(&m_sendBuffer4[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer4[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));

		return true;
	}// ModifyPacktes ()

	virtual bool SendPackets()
	{
		bool isSuccess = false;

		// Send first packet
		LOG_MSG_DEBUG("sending first packet\n");
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		// Send second packet
		LOG_MSG_DEBUG("sending second packet\n");
		isSuccess = m_producer.SendData(m_sendBuffer2, m_sendSize2);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		// Send third packet
		LOG_MSG_DEBUG("sending third packet\n");
		isSuccess = m_producer.SendData(m_sendBuffer3, m_sendSize3);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		// Send fourth packet
		LOG_MSG_DEBUG("sending fourth packet\n");
		isSuccess = m_producer.SendData(m_sendBuffer4, m_sendSize4);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		LOG_MSG_DEBUG("sent successfully four packets\n");
		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		size_t receivedSize3 = 0;
		size_t receivedSize4 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];
		Byte *rxBuff3 = new Byte[0x400];
		Byte *rxBuff4 = new Byte[0x400];

		if (rxBuff1 == NULL)
		{
			LOG_MSG_ERROR("Memory allocation error 1.\n");
			if (rxBuff2)
				delete[] rxBuff2;
			if (rxBuff3)
				delete[] rxBuff3;
			if (rxBuff4)
				delete[] rxBuff4;
			return false;
		}

		if (rxBuff2 == NULL)
		{
			LOG_MSG_ERROR("Memory allocation error 2.\n");
			delete[] rxBuff1;
			if (rxBuff3)
				delete[] rxBuff3;
			if (rxBuff4)
				delete[] rxBuff4;
			return false;
		}

		if (rxBuff3 == NULL)
		{
			LOG_MSG_ERROR("Memory allocation error 3.\n");
			delete[] rxBuff1;
			delete[] rxBuff2;
			if (rxBuff4)
				delete[] rxBuff4;
			return false;
		}

		if (rxBuff4 == NULL)
		{
			LOG_MSG_ERROR("Memory allocation error 4.\n");
			delete[] rxBuff1;
			delete[] rxBuff2;
			delete[] rxBuff3;
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		receivedSize2 = m_consumer.ReceiveData(rxBuff2, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize2, m_consumer.m_fromChannelName.c_str());

		receivedSize3 = m_consumer.ReceiveData(rxBuff3, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize3, m_consumer.m_fromChannelName.c_str());

		receivedSize4 = m_consumer.ReceiveData(rxBuff4, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize4, m_consumer.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGoldenNat(
			m_sendBuffer, m_sendSize,
			rxBuff1, receivedSize,
			m_private_ip, m_public_ip,
			m_private_port, m_public_port,
			true))
		{
			LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		char recievedBuffer[256] = { 0 };
		char SentBuffer[256] = { 0 };
		char recievedBuffer2[256] = { 0 };
		char SentBuffer2[256] = { 0 };
		char recievedBuffer3[256] = { 0 };
		char SentBuffer3[256] = { 0 };
		char recievedBuffer4[256] = { 0 };
		char SentBuffer4[256] = { 0 };
		size_t j;

		for (j = 0; j < m_sendSize; j++)
			snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for (j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);

		delete[] rxBuff1;

		isSuccess &= CompareResultVsGoldenNat(
			m_sendBuffer2, m_sendSize2,
			rxBuff2, receivedSize2,
			m_private_ip2, m_public_ip2,
			m_private_port2, m_public_port2,
			true);

		for (j = 0; j < m_sendSize2; j++)
			snprintf(&SentBuffer2[3 * j], sizeof(SentBuffer2) - (3 * j + 1), " %02X", m_sendBuffer2[j]);
		for (j = 0; j < receivedSize2; j++)
			snprintf(&recievedBuffer2[3 * j], sizeof(recievedBuffer2) - (3 * j + 1), " %02X", rxBuff2[j]);
		LOG_MSG_STACK("sent Value2 (%zu)\n%s\n, Received Value2(%zu)\n%s\n", m_sendSize2, SentBuffer2, receivedSize2, recievedBuffer2);

		delete[] rxBuff2;

		isSuccess &= CompareResultVsGoldenNat(
			m_sendBuffer3, m_sendSize3,
			rxBuff3, receivedSize3,
			m_private_ip3, m_public_ip3,
			m_private_port3, m_public_port3,
			true);

		for (j = 0; j < m_sendSize3; j++)
			snprintf(&SentBuffer3[3 * j], sizeof(SentBuffer3) - (3 * j + 1), " %02X", m_sendBuffer3[j]);
		for (j = 0; j < receivedSize3; j++)
			snprintf(&recievedBuffer3[3 * j], sizeof(recievedBuffer3) - (3 * j + 1), " %02X", rxBuff3[j]);
		LOG_MSG_STACK("sent Value3 (%zu)\n%s\n, Received Value3(%zu)\n%s\n", m_sendSize3, SentBuffer3, receivedSize3, recievedBuffer3);

		delete[] rxBuff3;

		isSuccess &= CompareResultVsGoldenNat(
			m_sendBuffer4, m_sendSize4,
			rxBuff4, receivedSize4,
			m_private_ip4, m_public_ip4,
			m_private_port4, m_public_port4,
			true);

		for (j = 0; j < m_sendSize4; j++)
			snprintf(&SentBuffer4[3 * j], sizeof(SentBuffer4) - (3 * j + 1), " %02X", m_sendBuffer4[j]);
		for (j = 0; j < receivedSize4; j++)
			snprintf(&recievedBuffer4[3 * j], sizeof(recievedBuffer4) - (3 * j + 1), " %02X", rxBuff4[j]);
		LOG_MSG_STACK("sent Value4 (%zu)\n%s\n, Received Value4(%zu)\n%s\n", m_sendSize4, SentBuffer4, receivedSize4, recievedBuffer4);

		delete[] rxBuff4;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test012: Single PDN dst NAT test expansion table usage                    */
/* NOTE: other classes are derived from this class - change carefully        */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest012 : public IpaNatBlockTestFixture
{
	uint32_t m_target_ip2;
	uint16_t m_target_port2;
public:
	IpaNatBlockTest012()
	{
		m_name = "IpaNatBlockTest012";
		m_description =
			"NAT block test 012 - single PDN dst NAT test - expansion table usage\
		1. Generate and commit three routing tables (only one is used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			action go to dst NAT \
			All DST_IP == (192.23.22.1 & 0.255.255.255)traffic goes to NAT block (public IP filtering) \
		3. generate and commit two NAT rules so second one is located in expansion table:\
		   since we use a single public ip this test should work also on pre IPAv4 targets\
			public ip 192.23.22.1 --> private ip 194.23.22.1  ";
		m_private_ip = 0xC2171601; /* 194.23.22.1 */
		m_private_port = 5678;
		m_private_ip2 = 0xC5171601; /* 197.23.22.1 */
		m_private_port2 = 5679;
		m_public_ip = 0xC0171601;   /* "192.23.22.1" */
		m_public_port = 9050;
		m_target_ip = 0xC1171601; /* 193.23.22.1 */
		m_target_port = 1234;
		m_target_ip2 = 0x1601C117; /* swap m_target_ip to get same hash*/
		m_target_port2 = m_target_port;
		Register(*this);
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0;

		if (!CreateThreeIPv4BypassRoutingTables(bypass0, bypass1, bypass2))
		{
			LOG_MSG_ERROR("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		LOG_MSG_DEBUG("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table0=0x%p) Failed.\n", &routing_table0);
			return false;
		}
		LOG_MSG_DEBUG("%s route table handle = %u\n", bypass0, routing_table0.hdl);

		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST_PROD, false, 3);
		LOG_MSG_DEBUG("FilterTable*.Init Completed Successfully..\n");

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_DST_NAT;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0x00FFFFFF; // Mask
		flt_rule_entry.rule.attrib.u.v4.dst_addr = m_public_ip; // Filter DST_IP == 192.23.22.1
		flt_rule_entry.rule.pdn_idx = 0;
		flt_rule_entry.rule.set_metadata = 0;
		if (
			((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable())
			)
		{
			LOG_MSG_ERROR("Error Adding Rule to Filter Table, aborting...\n");
			return false;
		}
		else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
		}

		//NAT table and rules creation
		int total_entries = 20;
		int ret;
		ipa_nat_ipv4_rule ipv4_rule;
		uint32_t pub_ip_add = m_public_ip;

		ret = ipa_nat_add_ipv4_tbl(pub_ip_add, m_mem_type, total_entries, &m_tbl_hdl);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed creating NAT table\n");
			return false;
		}

		LOG_MSG_DEBUG("nat table added, hdl %d, public ip 0x%X\n", m_tbl_hdl,
			pub_ip_add);

		ipv4_rule.target_ip = m_target_ip;
		ipv4_rule.target_port = m_target_port;
		ipv4_rule.private_ip = m_private_ip;
		ipv4_rule.private_port = m_private_port;
		ipv4_rule.protocol = IPPROTO_TCP;
		ipv4_rule.public_port = m_public_port;
		ipv4_rule.pdn_index = 0;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 0\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d, $d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port, ipv4_rule.pdn_index);

		ipv4_rule.target_ip = m_target_ip2;
		ipv4_rule.target_port = m_target_port2;

		// private IPs are not part of the dst NAT entry hash calculation
		ipv4_rule.private_ip = m_private_ip2;
		ipv4_rule.private_port = m_private_port2;

		ret = ipa_nat_add_ipv4_rule(m_tbl_hdl, &ipv4_rule, &m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed adding NAT rule 0\n");
			return false;
		}

		LOG_MSG_DEBUG("NAT rule 2 added, hdl %d, data: 0x%X, %d, 0x%X, %d, %d, %d, $d\n",
			m_nat_rule_hdl1, ipv4_rule.target_ip, ipv4_rule.target_port,
			ipv4_rule.private_ip, ipv4_rule.private_port,
			ipv4_rule.protocol, ipv4_rule.public_port, ipv4_rule.pdn_index);

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint32_t address;
		uint16_t port;
		char flags = 0x18;

		address = htonl(m_public_ip);
		memcpy(&m_sendBuffer[IPV4_DST_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_public_port);
		memcpy(&m_sendBuffer[IPV4_DST_PORT_OFFSET], &port, sizeof(port));

		address = htonl(m_target_ip2);
		memcpy(&m_sendBuffer[IPV4_SRC_ADDR_OFFSET], &address, sizeof(address));
		port = htons(m_target_port2);
		memcpy(&m_sendBuffer[IPV4_SRC_PORT_OFFSET], &port, sizeof(port));

		//make sure the FIN flag is not set, otherwise we will get a NAT miss
		memcpy(&m_sendBuffer[IPV4_TCP_FLAGS_OFFSET], &flags, sizeof(flags));

		return true;
	}// ModifyPacktes ()

	virtual bool SendPackets()
	{
		bool isSuccess = false;

		// Send first packet
		isSuccess = m_producer.SendData(m_sendBuffer, m_sendSize);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR("SendData failure.\n");
			return false;
		}

		LOG_MSG_DEBUG("sent successfully one packet\n");
		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];

		if (NULL == rxBuff1)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGoldenNat(
			m_sendBuffer, m_sendSize,
			rxBuff1, receivedSize,
			m_private_ip2, m_public_ip,
			m_private_port2, m_public_port,
			false))
		{
			LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		char recievedBuffer[256] = { 0 };
		char SentBuffer[256] = { 0 };
		size_t j;

		for (j = 0; j < m_sendSize; j++)
			snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for (j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);

		delete[] rxBuff1;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test013: Single PDN dst NAT expansion rule deletion test                 */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest013 : public IpaNatBlockTest012
{
public:
	IpaNatBlockTest013()
	{
		m_name = "IpaNatBlockTest013";
		m_description =
			"NAT block test 013 - single PDN dst NAT test - expansion table rule deletion\
		1. Generate and commit three routing tables (only one is used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			action go to dst NAT \
			All DST_IP == (192.23.22.1 & 0.255.255.255)traffic goes to NAT block (public IP filtering) \
		3. generate and commit two NAT rules so second one is located in expansion table:\
		   since we use a single public ip this test should work also on pre IPAv4 targets\
			public ip 192.23.22.1 --> private ip 194.23.22.1\
		4. delete NAT rule and expect NAT miss";
	}


	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering\n");

		int ret = IpaNatBlockTest012::AddRules();
		if (!ret) {
			LOG_MSG_ERROR("Leaving, failed Adding test 012 rules 0\n");
			return false;
		}

		ret = ipa_nat_del_ipv4_rule(m_tbl_hdl, m_nat_rule_hdl1);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed deleting NAT rule 1\n");
			return false;
		}
		LOG_MSG_DEBUG("NAT rule deleted\n");

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];

		if (NULL == rxBuff1)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		if (receivedSize) {
			LOG_MSG_ERROR("Data received - test failed!\n");
			isSuccess = false;

			// Compare results for debug in the case of failure only
			if (!CompareResultVsGoldenNat(
				m_sendBuffer, m_sendSize,
				rxBuff1, receivedSize,
				m_private_ip2, m_public_ip,
				m_private_port2, m_public_port,
				false))
			{
				LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			}
			else {
				LOG_MSG_ERROR("Comparison of Buffer0 succeeded - NAT rule was hit despite deletion!\n");
			}

			char recievedBuffer[256] = { 0 };
			char SentBuffer[256] = { 0 };
			size_t j;

			for (j = 0; j < m_sendSize; j++)
				snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
			for (j = 0; j < receivedSize; j++)
				snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
			LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);
		}

		delete[] rxBuff1;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test014: Single PDN src NAT zero PDN test                                 */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest014 : public IpaNatBlockTest001
{
public:
	IpaNatBlockTest014()
	{
		m_name = "IpaNatBlockTest014";
		m_description =
			"NAT block test 014 - single PDN src NAT PDN zeroing test\
		1. Generate and commit three routing tables (only one is used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			action go to src NAT \
			All DST_IP == (193.23.22.1 & 0.255.255.255)traffic goes to NAT block \
		3. generate and commit one NAT rule:\
			private ip 194.23.22.1 --> public ip 192.23.22.1\
		4. modify PDN entry 0 and expect NAT miss";
		m_minIPAHwType = IPA_HW_v4_0;
	}


	virtual bool AddRules()
	{
		ipa_nat_pdn_entry pdn_info;

		LOG_MSG_DEBUG("Entering\n");

		int ret = IpaNatBlockTest001::AddRules();
		if (!ret) {
			LOG_MSG_ERROR("Leaving, failed Adding test 001 rules 0\n");
			return false;
		}

		// modify PDN entry 0 so the NAT rule will get a NAT miss
		pdn_info.public_ip = 0;
		pdn_info.src_metadata = 0;
		pdn_info.dst_metadata = 0;
		ret = ipa_nat_modify_pdn(m_tbl_hdl, 0, &pdn_info);
		if (ret) {
			LOG_MSG_ERROR("Leaving, failed Modifying PDN entry 0 \n");
			return false;
		}

		LOG_MSG_DEBUG("PDN modified\n");

		LOG_MSG_DEBUG("Leaving\n");
		return true;
	}// AddRules()

	virtual bool ReceivePacketsAndCompare()
	{
		size_t receivedSize = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];

		if (NULL == rxBuff1)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		if (receivedSize) {
			LOG_MSG_ERROR("Data received - test failed!\n");
			isSuccess = false;

			// Compare results
			if (!CompareResultVsGoldenNat(
				m_sendBuffer, m_sendSize,
				rxBuff1, receivedSize,
				m_private_ip, m_public_ip,
				m_private_port, m_public_port,
				true))
			{
				LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			}
			else {
				LOG_MSG_ERROR("Comparison of Buffer0 succeeded - NAT rule was hit despite deletion!\n");
			}

			char recievedBuffer[256] = { 0 };
			char SentBuffer[256] = { 0 };
			size_t j;

			for (j = 0; j < m_sendSize; j++)
				snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
			for (j = 0; j < receivedSize; j++)
				snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
			LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);
		}

		delete[] rxBuff1;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test015: Single PDN src NAT send two packets that will hit the same rule  */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest015 : public IpaNatBlockTest001
{
public:
	IpaNatBlockTest015()
	{
		m_name = "IpaNatBlockTest015";
		m_description =
			"NAT block test 015 - single PDN src NAT hit the same rule twice\
		1. Generate and commit three routing tables (only one is used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			action go to src NAT \
			All DST_IP == (193.23.22.1 & 0.255.255.255)traffic goes to NAT block \
		3. generate and commit one NAT rule:\
			private ip 194.23.22.1 --> public ip 192.23.22.1\
		4. send two pakcets and verify NATing";
	}

	virtual bool SendPackets()
	{
		bool ret = IpaNatBlockTest001::SendPackets();
		if (!ret)
		{
			LOG_MSG_ERROR("failed sending first pakcket.\n");
			return false;
		}

		LOG_MSG_DEBUG("first packet sent succesfully.\n");

		ret = IpaNatBlockTest001::SendPackets();
		if (!ret)
		{
			LOG_MSG_ERROR("failed sending second pakcket.\n");
			return false;
		}

		LOG_MSG_DEBUG("second packet sent succesfully.\n");

		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		bool isSuccess = true;

		isSuccess &= IpaNatBlockTest001::ReceivePacketsAndCompare();
		if (!isSuccess)
		{
			LOG_MSG_ERROR("failed to receive\\compare first packet.\n");
		}

		isSuccess &= IpaNatBlockTest001::ReceivePacketsAndCompare();
		if (!isSuccess)
		{
			LOG_MSG_ERROR("failed to receive\\compare second packet.\n");
		}

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------*/
/* Test016: Single PDN dst NAT send two packets that will hit the same rule  */
/*---------------------------------------------------------------------------*/
class IpaNatBlockTest016 : public IpaNatBlockTest002
{
public:
	IpaNatBlockTest016()
	{
		m_name = "IpaNatBlockTest016";
		m_description =
			"NAT block test 016 - single PDN dst NAT hit the same rule twice\
		1. Generate and commit three routing tables (only one is used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit one filtering rule: (DST & Mask Match). \
			action go to dst NAT \
			All DST_IP == (192.23.22.1 & 0.255.255.255)traffic goes to NAT block \
		3. generate and commit one NAT rule:\
			public ip 192.23.22.1 --> private ip 194.23.22.1\
		4. send two pakcets and verify NATing";
	}

	virtual bool SendPackets()
	{
		bool ret = IpaNatBlockTest002::SendPackets();
		if (!ret)
		{
			LOG_MSG_ERROR("failed sending first pakcket.\n");
			return false;
		}

		LOG_MSG_DEBUG("first packet sent succesfully.\n");

		ret = IpaNatBlockTest002::SendPackets();
		if (!ret)
		{
			LOG_MSG_ERROR("failed sending second pakcket.\n");
			return false;
		}

		LOG_MSG_DEBUG("second packet sent succesfully.\n");

		return true;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		bool isSuccess = true;

		isSuccess &= IpaNatBlockTest002::ReceivePacketsAndCompare();
		if (!isSuccess)
		{
			LOG_MSG_ERROR("failed to receive\\compare first packet.\n");
		}

		isSuccess &= IpaNatBlockTest002::ReceivePacketsAndCompare();
		if (!isSuccess)
		{
			LOG_MSG_ERROR("failed to receive\\compare second packet.\n");
		}

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------------*/
/* Test017: Multi PDN src NAT test with identical private IPs and different ports  */
/*---------------------------------------------------------------------------------*/
class IpaNatBlockTest017 : public IpaNatBlockTest003
{
public:
	IpaNatBlockTest017()
	{
		m_name = "IpaNatBlockTest017";
		m_description =
		"NAT block test 017 - Multi PDN src NAT test with identical private IPs\
		1. Generate and commit three routing tables (two are used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit two filtering rule: (DST & Mask Match). \
			- action go to src NAT \
			  All SRC_IP == (194.23.22.1 & 0.255.255.255)traffic goes to NAT block \
			  All SRC_IP == (197.23.22.1 & 0.255.255.255)traffic goes to NAT block - not relevant for this test \
		3. generate and commit two NAT rules:\
			private ip 194.23.22.1 && port 5678--> public ip 192.23.22.1 \
			private ip 194.23.22.1 && port 5679--> public ip 195.23.22.1";
		m_private_ip2 = m_private_ip;
	}

	virtual bool ReceivePacketsAndCompare()
	{
		// we cannot just use test 003 ReceivePacketsAndCompare since the filtering rules send the
		// packets to two different pipes, but since the private IPs are now equal the second filtering rule
		// won't be hit so we need to recive the second packet on the first pipe

		size_t receivedSize = 0;
		size_t receivedSize2 = 0;
		bool isSuccess = true;

		// Receive results
		Byte *rxBuff1 = new Byte[0x400];
		Byte *rxBuff2 = new Byte[0x400];

		if (rxBuff1 == NULL)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			if (rxBuff2)
				delete[] rxBuff2;
			return false;
		}

		if (rxBuff2 == NULL)
		{
			LOG_MSG_ERROR("Memory allocation error.\n");
			delete[] rxBuff1;
			return false;
		}

		receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		receivedSize2 = m_consumer.ReceiveData(rxBuff2, 0x400);
		LOG_MSG_DEBUG("Received %zu bytes on %s.\n", receivedSize2, m_consumer.m_fromChannelName.c_str());

		// Compare results
		if (!CompareResultVsGoldenNat(
			m_sendBuffer, m_sendSize,
			rxBuff1, receivedSize,
			m_private_ip, m_public_ip,
			m_private_port, m_public_port,
			true))
		{
			LOG_MSG_ERROR("Comparison of Buffer0 Failed!\n");
			isSuccess = false;
		}

		char recievedBuffer[256] = { 0 };
		char SentBuffer[256] = { 0 };
		char recievedBuffer2[256] = { 0 };
		char SentBuffer2[256] = { 0 };
		size_t j;

		for (j = 0; j < m_sendSize; j++)
			snprintf(&SentBuffer[3 * j], sizeof(SentBuffer) - (3 * j + 1), " %02X", m_sendBuffer[j]);
		for (j = 0; j < receivedSize; j++)
			snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer) - (3 * j + 1), " %02X", rxBuff1[j]);
		LOG_MSG_STACK("sent Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n", m_sendSize, SentBuffer, receivedSize, recievedBuffer);

		delete[] rxBuff1;

		isSuccess &= CompareResultVsGoldenNat(
			m_sendBuffer2, m_sendSize2,
			rxBuff2, receivedSize2,
			m_private_ip2, m_public_ip2,
			m_private_port2, m_public_port2,
			true);

		for (j = 0; j < m_sendSize2; j++)
			snprintf(&SentBuffer2[3 * j], sizeof(SentBuffer2) - (3 * j + 1), " %02X", m_sendBuffer2[j]);
		for (j = 0; j < receivedSize2; j++)
			snprintf(&recievedBuffer2[3 * j], sizeof(recievedBuffer2) - (3 * j + 1), " %02X", rxBuff2[j]);
		LOG_MSG_STACK("sent Value2 (%zu)\n%s\n, Received Value2(%zu)\n%s\n", m_sendSize2, SentBuffer2, receivedSize2, recievedBuffer2);

		delete[] rxBuff2;

		return isSuccess;
	}
};

/*---------------------------------------------------------------------------------*/
/* Test018: Multi PDN dst NAT test with identical private IPs and different ports  */
/*---------------------------------------------------------------------------------*/
class IpaNatBlockTest018 : public IpaNatBlockTest004
{
public:
	IpaNatBlockTest018()
	{
		m_name = "IpaNatBlockTest018";
		m_description =
		"NAT block test 018 - Multi PDN dst NAT test with identical private IPs\
		1. Generate and commit three routing tables (two are used). \
			Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit two filtering rule: (DST & Mask Match). \
			- action go to dst NAT \
			  All DST_IP == (192.23.22.1 & 0.255.255.255)traffic goes to NAT block \
			  All DST_IP == (195.23.22.1 & 0.255.255.255)traffic goes to NAT block - not releveant for this test\
		3. generate and commit two NAT rules:\
			private ip 194.23.22.1 --> public ip 192.23.22.1 && port 9050\
			private ip 194.23.22.1 --> public ip 192.23.22.1 && port 9051";
		m_private_ip2 = m_private_ip;
	}
};

static class IpaNatBlockTest001 IpaNatBlockTest001;//single PDN src NAT test
static class IpaNatBlockTest002 IpaNatBlockTest002;//single PDN dst NAT test
static class IpaNatBlockTest003 IpaNatBlockTest003;//multi PDN (tuple) src NAT test
static class IpaNatBlockTest004 IpaNatBlockTest004;//multi PDN (tuple) dst NAT test
static class IpaNatBlockTest005 IpaNatBlockTest005;//single PDN src NAT test - src metadata replacement
static class IpaNatBlockTest006 IpaNatBlockTest006;//single PDN dst NAT test - dst metadata replacement
static class IpaNatBlockTest007 IpaNatBlockTest007;//hashable routing rule with dst NAT test
static class IpaNatBlockTest008 IpaNatBlockTest008;//Multi PDN src NAT test match PDN by input from filtering block
static class IpaNatBlockTest009 IpaNatBlockTest009;//single PDN src NAT rule deletion test
static class IpaNatBlockTest010 IpaNatBlockTest010;//single PDN dst NAT rule deletion test
static class IpaNatBlockTest011 IpaNatBlockTest011;//Multi PDN src NAT - MAX number of PDNs test
static class IpaNatBlockTest012 IpaNatBlockTest012;//Single PDN dst NAT test expansion table usage
static class IpaNatBlockTest013 IpaNatBlockTest013;//Single PDN dst NAT test expansion table delete test
static class IpaNatBlockTest014 IpaNatBlockTest014;//single PDN src NAT zero PDN test
static class IpaNatBlockTest015 IpaNatBlockTest015;//single PDN src NAT test - send two packets that will hit the same rule
static class IpaNatBlockTest016 IpaNatBlockTest016;//single PDN dst NAT test - send two packets that will hit the same rule
static class IpaNatBlockTest017 IpaNatBlockTest017;//multi PDN (tuple) src NAT test - identical private IPs different ports
static class IpaNatBlockTest018 IpaNatBlockTest018;//multi PDN (tuple) dst NAT test - identical private IPs different ports

