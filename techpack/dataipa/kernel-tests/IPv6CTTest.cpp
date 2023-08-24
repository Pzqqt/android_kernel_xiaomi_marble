/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
#include "ipa_ipv6ct.h"
}

#define IPV6_SRC_PORT_OFFSET (40)
#define IPV6_SRC_ADDRESS_MSB_OFFSET (8)
#define IPV6_SRC_ADDRESS_LSB_OFFSET (16)
#define IPV6_DST_ADDRESS_MSB_OFFSET (24)
#define IPV6_DST_ADDRESS_LSB_OFFSET (32)
#define IPV6_DST_PORT_OFFSET (40+2)

#define IPV6_LOW_32_MASK (0xFFFFFFFF)
#define IPV6_HIGH_32_MASK (0xFFFFFFFF00000000)

#define IPV6_BITS_IN_BYTE 8

inline uint32_t GetHigh32(uint64_t in)
{
	return static_cast<uint32_t>((in & IPV6_HIGH_32_MASK) >> 32);
}

inline uint32_t GetLow32(uint64_t in)
{
	return static_cast<uint32_t>(in & IPV6_LOW_32_MASK);
}

template <typename T>
T HostToNetwork(T in)
{
	printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
	if (1 == htons(1))
	{
		return in;
	}

	static const T mask = 0xff;
	T ret;
	uint8_t* p = reinterpret_cast<uint8_t*>(&ret + 1);
	while (in)
	{
		*--p = static_cast<uint8_t>(in & mask);
		in >>= IPV6_BITS_IN_BYTE;
	}
	printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
	return ret;
}

extern Logger g_Logger;

class IpaIPv6CTBlockTestFixture : public TestBase
{
public:

	IpaIPv6CTBlockTestFixture() :
		m_sendSize(BUFF_MAX_SIZE),
		m_sendSize2(BUFF_MAX_SIZE),
		m_sendSize3(BUFF_MAX_SIZE),
		m_outbound_dst_addr_msb(0XFF02000000000000),
		m_outbound_dst_addr_lsb(0x11223344556677AA),
		m_outbound_dst_port(1000),
		m_outbound_src_addr_msb(m_outbound_dst_addr_msb),
		m_outbound_src_addr_lsb(0x11223344556677CC),
		m_outbound_src_port(1001),
		m_direction_settings(IPA_IPV6CT_DIRECTION_ALLOW_ALL),
		m_tableHandle(0)
	{
		memset(m_sendBuffer, 0, sizeof(m_sendBuffer));	// First input file / IP packet
		memset(m_sendBuffer2, 0, sizeof(m_sendBuffer2));	// Second input file / IP packet
		memset(m_sendBuffer3, 0, sizeof(m_sendBuffer3));	// Third input file (default) / IP packet
		m_minIPAHwType = IPA_HW_v4_0;
		m_testSuiteName.push_back("IPv6CT");
	}

	static int SetupKernelModule(bool en_status = false)
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		int retval;
		struct ipa_channel_config from_ipa_channels[3];
		struct test_ipa_ep_cfg from_ipa_cfg[3];
		struct ipa_channel_config to_ipa_channels[1];
		struct test_ipa_ep_cfg to_ipa_cfg[1];

		struct ipa_test_config_header header = { 0 };
		struct ipa_channel_config *to_ipa_array[1];
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

		prepare_header_struct(&header, from_ipa_array, to_ipa_array);

		retval = GenericConfigureScenario(&header);

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return retval;
	}

	bool Setup()
	{
		bool bRetVal = true;

		if (SetupKernelModule() != true)
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
		ipa_ipv6ct_dump_table(m_tableHandle);
		ipa_ipv6ct_del_tbl(m_tableHandle);

		m_producer.Close();
		m_consumer.Close();
		m_consumer2.Close();
		m_defaultConsumer.Close();

		return true;
	} // Teardown()

	bool LoadFiles()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		if (!LoadDefaultPacket(IPA_IP_v6, m_extHdrType, m_sendBuffer, m_sendSize))
		{
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}
		printf("Loaded %zu Bytes to Buffer 1\n", m_sendSize);

		if (!LoadDefaultPacket(IPA_IP_v6, m_extHdrType, m_sendBuffer2, m_sendSize2))
		{
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}
		printf("Loaded %zu Bytes to Buffer 2\n", m_sendSize2);

		if (!LoadDefaultPacket(IPA_IP_v6, m_extHdrType, m_sendBuffer3, m_sendSize3))
		{
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}
		printf("Loaded %zu Bytes to Buffer 3\n", m_sendSize3);

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}

	// This function creates three IPv6 bypass routing entries and commits them.
	bool CreateThreeIPv6BypassRoutingTables(const char * bypass0, const char * bypass1, const char * bypass2)
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		struct ipa_ioc_add_rt_rule *rt_rule0 = 0, *rt_rule1 = 0, *rt_rule2 = 0;
		struct ipa_rt_rule_add *rt_rule_entry;

		rt_rule0 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
				sizeof(struct ipa_ioc_add_rt_rule) +
				1 * sizeof(struct ipa_rt_rule_add)
			);
		if (!rt_rule0) {
			printf("calloc failed to allocate rt_rule0 in %s\n", __FUNCTION__);
			return false;
		}
		rt_rule1 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
				sizeof(struct ipa_ioc_add_rt_rule) +
				1 * sizeof(struct ipa_rt_rule_add)
			);
		if (!rt_rule1) {
			printf("calloc failed to allocate rt_rule1 in %s\n", __FUNCTION__);
			Free(rt_rule0);
			return false;
		}
		rt_rule2 = (struct ipa_ioc_add_rt_rule *)
			calloc(1,
				sizeof(struct ipa_ioc_add_rt_rule) +
				1 * sizeof(struct ipa_rt_rule_add)
			);
		if (!rt_rule2) {
			printf("calloc failed to allocate rt_rule2 in %s\n", __FUNCTION__);
			Free(rt_rule0);
			Free(rt_rule1);
			return false;
		}

		rt_rule0->num_rules = 1;
		rt_rule0->ip = IPA_IP_v6;
		rt_rule0->commit = true;
		strlcpy(rt_rule0->rt_tbl_name, bypass0, sizeof(rt_rule0->rt_tbl_name));

		rt_rule_entry = &rt_rule0->rules[0];
		rt_rule_entry->at_rear = false;
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
			printf("Routing rule addition(rt_rule0) failed!\n");
			Free(rt_rule2);
			Free(rt_rule1);
			Free(rt_rule0);
			return false;
		}


		rt_rule1->num_rules = 1;
		rt_rule1->ip = IPA_IP_v6;
		rt_rule1->commit = true;
		strlcpy(rt_rule1->rt_tbl_name, bypass1, sizeof(rt_rule1->rt_tbl_name));
		rt_rule_entry = &rt_rule1->rules[0];
		rt_rule_entry->at_rear = false;
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
			printf("Routing rule addition(rt_rule1) failed!\n");
			Free(rt_rule2);
			Free(rt_rule1);
			Free(rt_rule0);
			return false;
		}


		rt_rule2->num_rules = 1;
		rt_rule2->ip = IPA_IP_v6;
		rt_rule2->commit = true;
		strlcpy(rt_rule2->rt_tbl_name, bypass2, sizeof(rt_rule2->rt_tbl_name));
		rt_rule_entry = &rt_rule2->rules[0];
		rt_rule_entry->at_rear = false;
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
			printf("Routing rule addition(rt_rule2) failed!\n");
			Free(rt_rule2);
			Free(rt_rule1);
			Free(rt_rule0);
			return false;
		}


		Free(rt_rule2);
		Free(rt_rule1);
		Free(rt_rule0);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}

	bool GetThreeIPv6BypassRoutingTables(uint32_t *Hndl0, uint32_t *Hndl1, uint32_t *Hndl2)
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0, routing_table1, routing_table2;

		if (!CreateThreeIPv6BypassRoutingTables(bypass0, bypass1, bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}

		printf("CreateThreeBypassRoutingTables completed successfully\n");
		routing_table0.ip = IPA_IP_v6;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%pK) Failed.\n", &routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v6;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%pK) Failed.\n", &routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v6;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			printf("m_routing.GetRoutingTable(&routing_table2=0x%pK) Failed.\n", &routing_table2);
			return false;
		}

		*Hndl0 = routing_table0.hdl;
		*Hndl1 = routing_table1.hdl;
		*Hndl2 = routing_table2.hdl;

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}

	bool AddIpv6ctTable()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		const int total_entries = 20;

		int result = ipa_ipv6ct_add_tbl(total_entries, &m_tableHandle);
		if (result)
		{
			printf("Leaving %s, %s(), failed creating IPvC6T table with result %d\n", __FUNCTION__, __FILE__, result);
			return false;
		}

		printf("IPv6CT table added, hdl %d\n", m_tableHandle);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}

	bool AddIpv6ctRule(ipa_ipv6ct_rule& rule, uint32_t& rule_hdl) const
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		int result = ipa_ipv6ct_add_rule(m_tableHandle, &rule, &rule_hdl);
		if (result)
		{
			printf("Leaving %s, %s(), failed creating IPvC6T rule with result %d\n", __FUNCTION__, __FILE__, result);
			return false;
		}
		printf("IPv6CT rule added:\ndest lsb %llX, dest msb %llX, dest port %d\ndir %d, proto %d\nsrc lsb 0x%llX, src msb 0x%llX, src port %d\n",
			(long long unsigned int)rule.dest_ipv6_lsb, (long long unsigned int)rule.dest_ipv6_msb,
			rule.dest_port, rule.direction_settings,
			rule.protocol, (long long unsigned int)rule.src_ipv6_lsb, (long long unsigned int)rule.src_ipv6_msb,
			rule.src_port);

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}

	void InitIpv6ctRule(ipa_ipv6ct_rule& rule, uint64_t change_bit) const
	{
		rule.dest_ipv6_lsb = m_outbound_dst_addr_lsb ^ change_bit;
		rule.dest_ipv6_msb = m_outbound_dst_addr_msb;
		rule.dest_port = m_outbound_dst_port;
		rule.direction_settings = m_direction_settings;
		rule.protocol = IPPROTO_TCP;
		rule.src_ipv6_lsb = m_outbound_src_addr_lsb ^ change_bit;
		rule.src_ipv6_msb = m_outbound_src_addr_msb;
		rule.src_port = m_outbound_src_port;
	}

	virtual bool AddIpv6ctRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		ipa_ipv6ct_rule rule;
		InitIpv6ctRule(rule, 0);

		uint32_t rule_hdl;
		bool result = AddIpv6ctRule(rule, rule_hdl);

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}

	virtual bool ModifyPackets() = 0;
	virtual bool AddRoutingFilteringRules() = 0;
	virtual bool ReceivePacketsAndCompare() = 0;

	bool Run()
	{
		bool res = false;
		bool isSuccess = false;

		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		res = AddRoutingFilteringRules();
		if (false == res) {
			printf("Failed adding routing and filtering rules.\n");
			return false;
		}

		res = AddIpv6ctTable();
		if (false == res)
		{
			printf("Failed adding IPv6 connection tracking table.\n");
			return false;
		}

		res = AddIpv6ctRules();
		if (false == res)
		{
			printf("Failed adding IPv6 connection tracking rules.\n");
			return false;
		}

		res = LoadFiles();
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

		// Receive packets from the channels and compare results
		isSuccess = ReceivePacketsAndCompare();

		printf("Leaving %s, %s(), Returning %d\n", __FUNCTION__, __FILE__, isSuccess);

		return isSuccess;
	} // Run()

	void ModifyPackets(uint64_t dstAddrLsb, uint64_t dstAddrMsb, uint16_t dstPort,
		uint64_t srcAddrLsb, uint64_t srcAddrMsb, uint16_t srcPort)
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		// destination
		uint64_t address = HostToNetwork(dstAddrLsb);
		memcpy(&m_sendBuffer[IPV6_DST_ADDRESS_LSB_OFFSET], &address, sizeof(address));

		address = HostToNetwork(dstAddrMsb);
		memcpy(&m_sendBuffer[IPV6_DST_ADDRESS_MSB_OFFSET], &address, sizeof(address));

		uint16_t port = ntohs(dstPort);
		memcpy(&m_sendBuffer[IPV6_DST_PORT_OFFSET], &port, sizeof(port));

		// source
		address = HostToNetwork(srcAddrLsb);
		memcpy(&m_sendBuffer[IPV6_SRC_ADDRESS_LSB_OFFSET], &address, sizeof(address));

		address = HostToNetwork(srcAddrMsb);
		memcpy(&m_sendBuffer[IPV6_SRC_ADDRESS_MSB_OFFSET], &address, sizeof(address));

		port = ntohs(srcPort);
		memcpy(&m_sendBuffer[IPV6_SRC_PORT_OFFSET], &port, sizeof(port));

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
	}// ModifyPacktes ()

	virtual bool AddRoutingFilteringRules(enum ipa_flt_action flt_action, uint64_t dst_addr_msb, uint64_t dst_addr_lsb)
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		const char bypass0[20] = "Bypass0";
		const char bypass1[20] = "Bypass1";
		const char bypass2[20] = "Bypass2";

		if (!CreateThreeIPv6BypassRoutingTables(bypass0, bypass1, bypass2))
		{
			printf("CreateThreeBypassRoutingTables Failed\n");
			return false;
		}
		printf("CreateThreeBypassRoutingTables completed successfully\n");

		ipa_ioc_get_rt_tbl routing_table0;
		routing_table0.ip = IPA_IP_v6;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			printf("m_routing.GetRoutingTable(&routing_table0=0x%pK) Failed.\n", &routing_table0);
			return false;
		}

		ipa_ioc_get_rt_tbl routing_table1;
		routing_table1.ip = IPA_IP_v6;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			printf("m_routing.GetRoutingTable(&routing_table1=0x%pK) Failed.\n", &routing_table1);
			return false;
		}

		IPAFilteringTable FilterTable0;
		ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v6, IPA_CLIENT_TEST_PROD, false, 1);

		// Configuring Filtering Rule No.0
		FilterTable0.GeneratePresetRule(1, flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.flt_rule_hdl = -1; // return Value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = flt_action;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;

		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[0] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[1] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[2] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr_mask[3] = 0xFFFFFFFF;// Exact Match
		flt_rule_entry.rule.attrib.u.v6.dst_addr[0] = GetHigh32(dst_addr_msb); // Filter DST_IP
		flt_rule_entry.rule.attrib.u.v6.dst_addr[1] = GetLow32(dst_addr_msb);
		flt_rule_entry.rule.attrib.u.v6.dst_addr[2] = GetHigh32(dst_addr_lsb);
		flt_rule_entry.rule.attrib.u.v6.dst_addr[3] = GetLow32(dst_addr_lsb);

		printf("flt_rule_entry was set successfully, preparing for insertion....\n");

		if (((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable()))
		{
			printf("%s::Error Adding Rule to Filter Table, aborting...\n", __FUNCTION__);
			return false;
		}
		else
		{
			printf("flt rule hdl0=0x%x, status=0x%x\n",
				FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl, FilterTable0.ReadRuleFromTable(0)->status);
		}

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// AddRoutingFilteringRules()

	virtual bool ReceivePacketsAndCompare(bool packetPassExpected)
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		// Receive results
		Byte rxBuff1[0x400];
		size_t receivedSize = m_consumer.ReceiveData(rxBuff1, 0x400);
		printf("Received %zu bytes on %s.\n", receivedSize, m_consumer.m_fromChannelName.c_str());

		bool isSuccess = true;
		if (packetPassExpected)
		{
			// Compare results
			if (!CompareResultVsGolden(m_sendBuffer, m_sendSize, rxBuff1, receivedSize))
			{
				printf("Comparison of Buffer0 Failed!\n");
				isSuccess = false;
			}
		}
		else
		{
			if (receivedSize)
			{
				isSuccess = false;
				printf("got data while expected packet to be blocked, failing\n");
			}
		}

		char recievedBuffer[256] = {0};
		char SentBuffer[256] = {0};
		size_t j;

		for (j = 0; j < m_sendSize; j++)
		{
			snprintf(&SentBuffer[3 * j], sizeof(SentBuffer)-(3 * j + 1), " %02X", m_sendBuffer[j]);
		}

		for (j = 0; j < receivedSize; j++)
		{
			snprintf(&recievedBuffer[3 * j], sizeof(recievedBuffer)-(3 * j + 1), " %02X", rxBuff1[j]);
		}
		printf("Expected Value1 (%zu)\n%s\n, Received Value1(%zu)\n%s\n",
			m_sendSize, SentBuffer, receivedSize, recievedBuffer);

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return isSuccess;
	}

protected:

	static Filtering m_filtering;
	static RoutingDriverWrapper m_routing;
	InterfaceAbstraction m_producer;
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
	static const ipv6_ext_hdr_type m_extHdrType = NONE;

	uint64_t m_outbound_dst_addr_msb;
	uint64_t m_outbound_dst_addr_lsb;
	uint16_t m_outbound_dst_port;
	uint64_t m_outbound_src_addr_msb;
	uint64_t m_outbound_src_addr_lsb;
	uint16_t m_outbound_src_port;
	ipa_ipv6_ct_direction_settings_type m_direction_settings;

	uint32_t m_tableHandle;
};

RoutingDriverWrapper IpaIPv6CTBlockTestFixture::m_routing;
Filtering IpaIPv6CTBlockTestFixture::m_filtering;

/*---------------------------------------------------------------------------------------------*/
/* Test001: IPv6CT send outbound packet  */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest001 : public IpaIPv6CTBlockTestFixture
{
public:

	IpaIPV6CTBlockTest001()
	{
		m_name = "IpaIPV6CTBlockTest001";
		m_description =
			"IPv6CT block test 001 - IPv6CT passes successfully one packet in outbound direction\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one outbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add IPv6CT rule for the packet\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_SRC_NAT,
			m_outbound_dst_addr_msb, m_outbound_dst_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port,
			m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(true);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test002: IPv6CT send inbound packet  */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest002 : public IpaIPv6CTBlockTestFixture
{
public:

	IpaIPV6CTBlockTest002()
	{
		m_name = "IpaIPV6CTBlockTest002";
		m_description =
			"IPv6CT block test 002 - IPv6CT passes successfully one packet in inbound direction\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one inbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add IPv6CT rule for the packet\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_DST_NAT,
			m_outbound_src_addr_msb, m_outbound_src_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port,
			m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(true);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test003: IPv6CT send outbound packet - without IPV6CT rule */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest003 : public IpaIPv6CTBlockTestFixture
{
public:

	IpaIPV6CTBlockTest003()
	{
		m_name = "IpaIPV6CTBlockTest003";
		m_description =
			"IPv6CT block test 003 - IPv6CT blocks one packet in outbound direction due to rule absence\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one outbound filtering rule: Destination IP Exactly Match.\n";
		Register(*this);
	}

	virtual bool AddIpv6ctRules()
	{
		printf("not adding IPv6CT rule for packet - blocking expected %s %s\n", __FUNCTION__, __FILE__);
		return true;
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_SRC_NAT,
			m_outbound_dst_addr_msb, m_outbound_dst_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port,
			m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(false);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test004: IPv6CT send inbound packet - without IPV6CT rule */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest004 : public IpaIPv6CTBlockTestFixture
{
public:

	IpaIPV6CTBlockTest004()
	{
		m_name = "IpaIPV6CTBlockTest004";
		m_description =
			"IPv6CT block test 004 - IPv6CT blocks one packet in inbound direction due to rule absence\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one inbound filtering rule: Destination IP Exactly Match.\n";
		Register(*this);
	}

	virtual bool AddIpv6ctRules()
	{
		printf("not adding IPv6CT rule for packet - blocking expected %s %s\n", __FUNCTION__, __FILE__);
		return true;
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_DST_NAT,
			m_outbound_src_addr_msb, m_outbound_src_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port,
			m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(false);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test005: IPv6CT send outbound packet with inbound filtering rule */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest005 : public IpaIPv6CTBlockTestFixture
{
public:

	IpaIPV6CTBlockTest005()
	{
		m_name = "IpaIPV6CTBlockTest005";
		m_description =
			"IPv6CT block test 005 - IPv6CT blocks one packet in outbound direction, because the filtering rule\n"
			"                        action is inbound\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one inbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add IPv6CT rule for the packet\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_DST_NAT,
			m_outbound_dst_addr_msb, m_outbound_dst_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port,
			m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(false);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test006: IPv6CT send inbound packet with outbound filtering rule */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest006 : public IpaIPv6CTBlockTestFixture
{
public:

	IpaIPV6CTBlockTest006()
	{
		m_name = "IpaIPV6CTBlockTest006";
		m_description =
			"IPv6CT block test 006 - IPv6CT blocks one packet in inbound direction, because the filtering rule\n"
			"                        action is outbound\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one outbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add IPv6CT rule for the packet\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_SRC_NAT,
			m_outbound_src_addr_msb, m_outbound_src_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port,
			m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(false);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test007: IPv6CT block outbound packet while disabled outbound direction                     */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest007 : public IpaIPv6CTBlockTestFixture
{
public:

	IpaIPV6CTBlockTest007()
	{
		m_name = "IpaIPV6CTBlockTest007";
		m_description =
			"IPv6CT block test 007 - IPv6CT blocks one packet in outbound direction, because the outbound direction\n"
			"                        is disabled\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one outbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add IPv6CT rule for the packet with disabled outbound direction\n";
		m_direction_settings = IPA_IPV6CT_DIRECTION_ALLOW_IN;
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_SRC_NAT,
			m_outbound_dst_addr_msb, m_outbound_dst_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port,
			m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(false);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test008: IPv6CT block inbound packet with disabled inbound direction                        */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest008 : public IpaIPv6CTBlockTestFixture
{
public:

	IpaIPV6CTBlockTest008()
	{
		m_name = "IpaIPV6CTBlockTest008";
		m_description =
			"IPv6CT block test 008 - IPv6CT blocks one packet in inbound direction, because the inbound direction\n"
			"                        is disabled\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one inbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add IPv6CT rule for the packet with disabled inbound direction\n";
		m_direction_settings = IPA_IPV6CT_DIRECTION_ALLOW_OUT;
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_DST_NAT,
			m_outbound_src_addr_msb, m_outbound_src_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port,
			m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(false);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

class IpaIPv6CTBlockExpansionTableTestFixture : public IpaIPv6CTBlockTestFixture
{
public:

	virtual bool AddIpv6ctRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		ipa_ipv6ct_rule rule;
		InitIpv6ctRule(rule, 8);

		uint32_t rule_hdl;
		if (!AddIpv6ctRule(rule, rule_hdl))
		{
			return false;
		}

		bool result = IpaIPv6CTBlockTestFixture::AddIpv6ctRules();

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test009: IPv6CT send outbound packet with rule in expansion table */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest009 : public IpaIPv6CTBlockExpansionTableTestFixture
{
public:

	IpaIPV6CTBlockTest009()
	{
		m_name = "IpaIPV6CTBlockTest009";
		m_description =
			"IPv6CT block test 009 - IPv6CT passes successfully one packet in outbound direction with rule in\n"
			"                        expansion table\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one outbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add an IPv6CT rule to occupy base table. This rule is not supposed to match a packet\n"
			"4. Add IPv6CT rule for the packet to the expansion table\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_SRC_NAT,
			m_outbound_dst_addr_msb, m_outbound_dst_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port,
			m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(true);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test010: IPv6CT send inbound packet with rule in expansion table */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest010 : public IpaIPv6CTBlockExpansionTableTestFixture
{
public:

	IpaIPV6CTBlockTest010()
	{
		m_name = "IpaIPV6CTBlockTest010";
		m_description =
			"IPv6CT block test 010 - IPv6CT passes successfully one packet in inbound direction with rule in\n"
			"                        expansion table\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one inbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add an IPv6CT rule to occupy base table. This rule is not supposed to match a packet\n"
			"4. Add IPv6CT rule for the packet to the expansion table\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_DST_NAT,
			m_outbound_src_addr_msb, m_outbound_src_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port,
			m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(true);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

class IpaIPv6CTBlockRuleDeleteTestFixture : public IpaIPv6CTBlockTestFixture
{
public:

	virtual bool AddIpv6ctRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		ipa_ipv6ct_rule rule;
		InitIpv6ctRule(rule, 0);

		uint32_t rule_hdl;
		if (!AddIpv6ctRule(rule, rule_hdl))
		{
			return false;
		}

		int result = ipa_ipv6ct_del_rule(m_tableHandle, rule_hdl);
		if (result)
		{
			printf("Leaving %s, %s(), failed delete IPvC6T rule %d with result %d\n", __FUNCTION__, __FILE__,
				rule_hdl, result);
			return false;
		}

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test011: IPv6CT block outbound packet while the rule was deleted                            */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest011 : public IpaIPv6CTBlockRuleDeleteTestFixture
{
public:

	IpaIPV6CTBlockTest011()
	{
		m_name = "IpaIPV6CTBlockTest011";
		m_description =
			"IPv6CT block test 011 - IPv6CT blocks one packet in outbound direction due to the rule deletion\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one outbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add IPv6CT rule for the packet\n"
			"4. Delete IPv6CT rule for the packet\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_SRC_NAT,
			m_outbound_dst_addr_msb, m_outbound_dst_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port,
			m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(false);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test012: IPv6CT block inbound packet while the rule was deleted                             */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest012 : public IpaIPv6CTBlockRuleDeleteTestFixture
{
public:

	IpaIPV6CTBlockTest012()
	{
		m_name = "IpaIPV6CTBlockTest012";
		m_description =
			"IPv6CT block test 012 - IPv6CT blocks one packet in inbound direction due to the rule deletion\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one inbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add IPv6CT rule for the packet\n"
			"4. Delete IPv6CT rule for the packet\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_DST_NAT,
			m_outbound_src_addr_msb, m_outbound_src_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port,
			m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(false);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

class IpaIPv6CTBlockRuleDeleteExpansionTableTestFixture : public IpaIPv6CTBlockRuleDeleteTestFixture
{
public:

	virtual bool AddIpv6ctRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		ipa_ipv6ct_rule rule;
		InitIpv6ctRule(rule, 8);

		uint32_t rule_hdl;
		if (!AddIpv6ctRule(rule, rule_hdl))
		{
			return false;
		}

		bool result = IpaIPv6CTBlockRuleDeleteTestFixture::AddIpv6ctRules();

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test013: IPv6CT block outbound packet while the rule in expansion table was deleted         */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest013 : public IpaIPv6CTBlockRuleDeleteExpansionTableTestFixture
{
public:

	IpaIPV6CTBlockTest013()
	{
		m_name = "IpaIPV6CTBlockTest013";
		m_description =
			"IPv6CT block test 013 - IPv6CT blocks one packet in outbound direction due to the rule deletion from\n"
			"                        the expansion table\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one outbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add an IPv6CT rule to occupy base table. This rule is not supposed to match a packet\n"
			"4. Add IPv6CT rule for the packet to the expansion table\n"
			"5. Delete IPv6CT rule for the packet from the expansion table\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_SRC_NAT,
			m_outbound_dst_addr_msb, m_outbound_dst_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port,
			m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(false);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------*/
/* Test014: IPv6CT block inbound packet while the rule in expansion table was deleted          */
/*---------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest014 : public IpaIPv6CTBlockRuleDeleteExpansionTableTestFixture
{
public:

	IpaIPV6CTBlockTest014()
	{
		m_name = "IpaIPV6CTBlockTest014";
		m_description =
			"IPv6CT block test 014 - IPv6CT blocks one packet in inbound direction due to the rule deletion from\n"
			"                        the expansion table\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one inbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add an IPv6CT rule to occupy base table. This rule is not supposed to match a packet\n"
			"4. Add IPv6CT rule for the packet to the expansion table\n"
			"5. Delete IPv6CT rule for the packet from the expansion table\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_DST_NAT,
			m_outbound_src_addr_msb, m_outbound_src_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port,
			m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(false);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

class IpaIPv6CTBlockHeadRuleDeleteTestFixture : public IpaIPv6CTBlockTestFixture
{
public:

	virtual bool AddIpv6ctRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		ipa_ipv6ct_rule rule;
		InitIpv6ctRule(rule, 8);

		uint32_t rule_hdl;
		if (!AddIpv6ctRule(rule, rule_hdl))
		{
			return false;
		}

		if (!IpaIPv6CTBlockTestFixture::AddIpv6ctRules())
		{
			return false;
		}

		int result = ipa_ipv6ct_del_rule(m_tableHandle, rule_hdl);
		if (result)
		{
			printf("Leaving %s, %s(), failed delete IPvC6T rule %d with result %d\n", __FUNCTION__, __FILE__,
				rule_hdl, result);
			return false;
		}

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}
};

/*---------------------------------------------------------------------------------------------------------------*/
/* Test015: IPv6CT send outbound packet with rule in expansion table while the rule in the list head was deleted */
/*---------------------------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest015 : public IpaIPv6CTBlockHeadRuleDeleteTestFixture
{
public:

	IpaIPV6CTBlockTest015()
	{
		m_name = "IpaIPV6CTBlockTest015";
		m_description =
			"IPv6CT block test 015 - IPv6CT passes successfully one packet in outbound direction with rule in\n"
			"                        expansion table, while the list head was deleted\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one outbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add an IPv6CT rule to occupy base table. This rule is not supposed to match a packet\n"
			"4. Add IPv6CT rule for the packet to the expansion table\n"
			"5. Delete IPv6CT rule in the list head\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_SRC_NAT,
			m_outbound_dst_addr_msb, m_outbound_dst_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port,
			m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(true);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*---------------------------------------------------------------------------------------------------------------*/
/* Test016: IPv6CT send inbound packet with rule in expansion table while the rule in the list head was deleted  */
/*---------------------------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest016 : public IpaIPv6CTBlockHeadRuleDeleteTestFixture
{
public:

	IpaIPV6CTBlockTest016()
	{
		m_name = "IpaIPV6CTBlockTest016";
		m_description =
			"IPv6CT block test 016 - IPv6CT passes successfully one packet in inbound direction with rule in\n"
			"                        expansion table, while the list head was deleted\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one inbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add an IPv6CT rule to occupy base table. This rule is not supposed to match a packet\n"
			"4. Add IPv6CT rule for the packet to the expansion table\n"
			"5. Delete IPv6CT rule in the list head\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_DST_NAT,
			m_outbound_src_addr_msb, m_outbound_src_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port,
			m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(true);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

class IpaIPv6CTBlockMiddleRuleDeleteTestFixture : public IpaIPv6CTBlockHeadRuleDeleteTestFixture
{
public:

	virtual bool AddIpv6ctRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);

		ipa_ipv6ct_rule rule;
		InitIpv6ctRule(rule, 1);

		uint32_t rule_hdl;
		if (!AddIpv6ctRule(rule, rule_hdl))
		{
			return false;
		}

		bool result = IpaIPv6CTBlockHeadRuleDeleteTestFixture::AddIpv6ctRules();

		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*------------------------------------------------------------------------------------------------------------*/
/* Test017: IPv6CT send outbound packet with rule in expansion table while the rule in the middle of the list */
/*          was deleted                                                                                       */
/*------------------------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest017 : public IpaIPv6CTBlockMiddleRuleDeleteTestFixture
{
public:

	IpaIPV6CTBlockTest017()
	{
		m_name = "IpaIPV6CTBlockTest017";
		m_description =
			"IPv6CT block test 017 - IPv6CT passes successfully one packet in outbound direction with rule in\n"
			"                        expansion table, while the rule in the middle of the list was deleted\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one outbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add two IPv6CT rules: one to base table and other to expansion table. These rules are not supposed\n"
			"   to match a packet\n"
			"4. Add IPv6CT rule for the packet to the expansion table\n"
			"5. Delete IPv6CT rule in the middle of the list\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_SRC_NAT,
			m_outbound_dst_addr_msb, m_outbound_dst_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port,
			m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(true);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

/*------------------------------------------------------------------------------------------------------------*/
/* Test018: IPv6CT send inbound packet with rule in expansion table while the rule in the middle of the list  */
/*          was deleted                                                                                       */
/*------------------------------------------------------------------------------------------------------------*/
class IpaIPV6CTBlockTest018 : public IpaIPv6CTBlockMiddleRuleDeleteTestFixture
{
public:

	IpaIPV6CTBlockTest018()
	{
		m_name = "IpaIPV6CTBlockTest018";
		m_description =
			"IPv6CT block test 018 - IPv6CT passes successfully one packet in inbound direction with rule in\n"
			"                        expansion table, while the rule in the middle of the list was deleted\n"
			"1. Generate and commit three routing tables.\n"
			"   Each table contains a single \"bypass\" rule (all data goes to output pipe 0, 1 and 2 (accordingly))\n"
			"2. Generate and commit one inbound filtering rule: Destination IP Exactly Match.\n"
			"3. Add two IPv6CT rules to occupy base table and the middle of the list. These rules are not supposed\n"
			"   to match a packet\n"
			"4. Add IPv6CT rule for the packet to the expansion table\n"
			"5. Delete IPv6CT rule in the middle of the list\n";
		Register(*this);
	}

	virtual bool AddRoutingFilteringRules()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::AddRoutingFilteringRules(IPA_PASS_TO_DST_NAT,
			m_outbound_src_addr_msb, m_outbound_src_addr_lsb);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}// AddRoutingFilteringRules()

	virtual bool ModifyPackets()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		IpaIPv6CTBlockTestFixture::ModifyPackets(m_outbound_src_addr_lsb, m_outbound_src_addr_msb, m_outbound_src_port,
			m_outbound_dst_addr_lsb, m_outbound_dst_addr_msb, m_outbound_dst_port);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return true;
	}// ModifyPackets()

	virtual bool ReceivePacketsAndCompare()
	{
		printf("Entering %s, %s()\n", __FUNCTION__, __FILE__);
		bool result = IpaIPv6CTBlockTestFixture::ReceivePacketsAndCompare(true);
		printf("Leaving %s, %s()\n", __FUNCTION__, __FILE__);
		return result;
	}
};

// IPv6CT outbound packet test
static class IpaIPV6CTBlockTest001 IpaIPV6CTBlockTest001;

// IPv6CT inbound packet test
static class IpaIPV6CTBlockTest002 IpaIPV6CTBlockTest002;

// IPv6CT block outbound packet test
static class IpaIPV6CTBlockTest003 IpaIPV6CTBlockTest003;

// IPv6CT block inbound packet test
static class IpaIPV6CTBlockTest004 IpaIPV6CTBlockTest004;

// IPv6CT block outbound packet on inbound filtering rule test
static class IpaIPV6CTBlockTest005 IpaIPV6CTBlockTest005;

// IPv6CT block inbound packet on outbound filtering rule test
static class IpaIPV6CTBlockTest006 IpaIPV6CTBlockTest006;

// IPv6CT block outbound packet while disabled outbound direction
static class IpaIPV6CTBlockTest007 IpaIPV6CTBlockTest007;

// IPv6CT block inbound packet with disabled inbound direction
static class IpaIPV6CTBlockTest008 IpaIPV6CTBlockTest008;

// IPv6CT send outbound packet with rule in expansion table
static class IpaIPV6CTBlockTest009 IpaIPV6CTBlockTest009;

// IPv6CT send inbound packet with rule in expansion table
static class IpaIPV6CTBlockTest010 IpaIPV6CTBlockTest010;

// IPv6CT block outbound packet while the rule was deleted
static class IpaIPV6CTBlockTest011 IpaIPV6CTBlockTest011;

// IPv6CT block inbound packet while the rule was deleted
static class IpaIPV6CTBlockTest012 IpaIPV6CTBlockTest012;

// IPv6CT block outbound packet while the rule in expansion table was deleted
static class IpaIPV6CTBlockTest013 IpaIPV6CTBlockTest013;

// IPv6CT block inbound packet while the rule in expansion table was deleted
static class IpaIPV6CTBlockTest014 IpaIPV6CTBlockTest014;

// IPv6CT send outbound packet with rule in expansion table while the rule in list head was deleted
static class IpaIPV6CTBlockTest015 IpaIPV6CTBlockTest015;

// IPv6CT send inbound packet with rule in expansion table while the rule in list head was deleted
static class IpaIPV6CTBlockTest016 IpaIPV6CTBlockTest016;

// IPv6CT send outbound packet with rule in expansion table while the rule in the middle of the list was deleted
static class IpaIPV6CTBlockTest017 IpaIPV6CTBlockTest017;

// IPv6CT send inbound packet with rule in expansion table while the rule in the middle of the list was deleted
static class IpaIPV6CTBlockTest018 IpaIPV6CTBlockTest018;

