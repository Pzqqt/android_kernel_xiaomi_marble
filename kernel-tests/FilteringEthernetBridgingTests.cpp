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

/*---------------------------------------------------------------------------*/
/* Test 00: Destination IP address and subnet mask match against LAN subnet  */
/*---------------------------------------------------------------------------*/
class IpaFilterEthIPv4Test00 : public IpaFilteringEthernetBridgingTestFixture
{
public:
	IpaFilterEthIPv4Test00()
	{
		m_name = "IpaFilterEthIPv4Test00";
		m_description =
		"Filtering block test 01 - Ethernet Bridge, ETH2 filters, \
		IPv4 address (EP Filtering Table, \
		Insert all rules in a single commit) \
		1. Generate and commit three routing tables. \
		Each table contains a single \"bypass\" rule \
		(all data goes to output pipe 0, 1  and 2 (accordingly)) \
		2. Generate and commit 3 ETH2 filtering rules: \
		All MAC DST == (aabbccddee11) traffic goes to routing table 0 \
		All MAC SRC == (22eeddccbbaa) traffic goes to routing table 1 \
		All (1)                       traffic goes to routing table 2";
		m_minIPAHwType = IPA_HW_v2_5;
		m_maxIPAHwType = IPA_HW_MAX;
		Register(*this);
		m_pCurrentProducer = &m_producer2;
	}

	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering");

		const char bypass0[] = "bypass0";
		const char bypass1[] = "bypass1";
		const char bypass2[] = "bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeIPv4BypassRoutingTables");
			return false;
		}

		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR(
				"m_routing.GetRoutingTable(&routing_table0=0x%p)",
				&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			LOG_MSG_ERROR(
				"m_routing.GetRoutingTable(&routing_table1=0x%p)",
				&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			LOG_MSG_ERROR(
				"m_routing.GetRoutingTable(&routing_table2=0x%p)",
				&routing_table2);
			return false;
		}

		// Create 3 filter rules
		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST2_PROD,false,3);

		// Configuring Filtering Rule 0 - ETH2 DST
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.rule.retain_hdr = 1; // retain header removed in producer pipe
		flt_rule_entry.flt_rule_hdl = -1; // return value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl; // Handle corresponding to routing table 0
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_MAC_DST_ADDR_ETHER_II; // Filter using ETH2 DST address
		memcpy(flt_rule_entry.rule.attrib.dst_mac_addr_mask,
				m_MAC_ADDR_MASK_ALL,
				sizeof(flt_rule_entry.rule.attrib.dst_mac_addr_mask)); // ETH2 DST address mask
		memcpy(flt_rule_entry.rule.attrib.dst_mac_addr,
				m_ETH2_DST_ADDR,
				sizeof(flt_rule_entry.rule.attrib.dst_mac_addr)); // ETH2 DST address

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			LOG_MSG_ERROR ("Adding RuleTable(0) to Filtering");
			return false;
		} else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x",
				FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,
				FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule 1 - ETH2 SRC
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl; // Handle corresponding to routing table 1
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_MAC_SRC_ADDR_ETHER_II; // Filter using ETH2 SRC address
		memcpy(flt_rule_entry.rule.attrib.src_mac_addr_mask,
			m_MAC_ADDR_MASK_ALL,
			sizeof(flt_rule_entry.rule.attrib.src_mac_addr_mask)); // ETH2 SRC address mask
		memcpy(flt_rule_entry.rule.attrib.src_mac_addr,
			m_ETH2_SRC_ADDR,
			sizeof(flt_rule_entry.rule.attrib.src_mac_addr)); // ETH2 SRC address
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			LOG_MSG_ERROR ("Adding RuleTable(1) to Filtering");
			return false;
		} else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x",
				FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,
				FilterTable0.ReadRuleFromTable(1)->status);
		}

		// Configuring Filtering Rule 2 - Accept all
		flt_rule_entry.rule.rt_tbl_hdl = routing_table2.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0x00000000; // Accept all
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x00000000; // Has no effect
		if (((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable()))
		{
			LOG_MSG_ERROR ("Adding RuleTable(2) to Filtering");
			return false;
		} else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x",
				FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,
				FilterTable0.ReadRuleFromTable(2)->status);
		}
		LOG_MSG_DEBUG("Leaving function\n");
		return true;

	}// AddRules()

	virtual bool ModifyPackets()
	{
		memcpy(&m_sendBuffer1[ETH2_DST_ADDR_OFFSET],
			m_ETH2_DST_ADDR, ETH_ALEN);

		memcpy(&m_sendBuffer2[ETH2_DST_ADDR_OFFSET],
			m_ETH2_DST_ADDR, ETH_ALEN);
		m_sendBuffer2[ETH2_DST_ADDR_OFFSET] =  0x00;
		memcpy(&m_sendBuffer2[ETH2_SRC_ADDR_OFFSET],
			m_ETH2_SRC_ADDR, ETH_ALEN);

		// swap destination and source addresses so that both
		// rule0 and rule1 are miss and rule2 (accept all) is hit
		memcpy(&m_sendBuffer3[ETH2_DST_ADDR_OFFSET],
			m_ETH2_SRC_ADDR, ETH_ALEN);
		memcpy(&m_sendBuffer3[ETH2_SRC_ADDR_OFFSET],
			m_ETH2_DST_ADDR, ETH_ALEN);

		return true;
	}// ModifyPacktes ()
};

/*---------------------------------------------------------------------------*/
/* Test 01: Destination IP address and subnet mask match against LAN subnet  */
/*---------------------------------------------------------------------------*/
class IpaFilterEthIPv4Test01 : public IpaFilteringEthernetBridgingTestFixture
{
public:
	IpaFilterEthIPv4Test01()
	{
		m_name = "IpaFilterEthIPv4Test01";
		m_description =
			"Filtering block test 01 - Ethernet Bridge, ETH2 filters, \
			IPv4 address (EP Filtering Table, \
			Insert all rules in a single commit) \
			1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule \
			(all data goes to output pipe 0, 1  and 2 (accordingly)) \
			2. Generate and commit 3 ETH2 filtering rules: \
			All MAC DST == (aabbccddee11) traffic goes to routing table 0 \
			All MAC ETH TYPE ==    (0800) traffic goes to routing table 1 \
			All (1)                       traffic goes to routing table 2";
		m_minIPAHwType = IPA_HW_v2_5;
		m_maxIPAHwType = IPA_HW_MAX;
		Register(*this);
		m_pCurrentProducer = &m_producer2;
	}

	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering");

		const char bypass0[] = "bypass0";
		const char bypass1[] = "bypass1";
		const char bypass2[] = "bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeIPv4BypassRoutingTables");
			return false;
		}

		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR(
				"m_routing.GetRoutingTable(&routing_table0=0x%p)",
				&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table1=0x%p)",
				&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			LOG_MSG_ERROR(
				"m_routing.GetRoutingTable(&routing_table2=0x%p)",
				&routing_table2);
			return false;
		}

		// Create 3 filter rules
		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST2_PROD,false,3);

		// Configuring Filtering Rule 0 - ETH2 DST
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.rule.retain_hdr = 1; // retain header removed in producer pipe
		flt_rule_entry.flt_rule_hdl = -1; // return value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl;

		// DST
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_MAC_DST_ADDR_ETHER_II;
		memcpy(flt_rule_entry.rule.attrib.dst_mac_addr_mask,
			m_MAC_ADDR_MASK_ALL,
			sizeof(flt_rule_entry.rule.attrib.dst_mac_addr_mask));
		memcpy(flt_rule_entry.rule.attrib.dst_mac_addr,
			m_ETH2_DST_ADDR,
			sizeof(flt_rule_entry.rule.attrib.dst_mac_addr));
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			LOG_MSG_ERROR ("Adding RuleTable(0) to Filtering");
			return false;
		} else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x",
				FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,
				FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule 1 - ETH2 type
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_MAC_ETHER_TYPE;
		flt_rule_entry.rule.attrib.ether_type = ETH_P_IP;
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			LOG_MSG_ERROR ("Adding RuleTable(1) to Filtering");
			return false;
		} else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x",
				FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,
				FilterTable0.ReadRuleFromTable(1)->status);
		}

		// Configuring Filtering Rule 2 - Accept all
		flt_rule_entry.rule.rt_tbl_hdl = routing_table2.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0x00000000; // Accept all
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x00000000; // Has no effect
		if (((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable()))
		{
			LOG_MSG_ERROR ("Adding RuleTable(2) to Filtering");
			return false;
		} else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x",
				FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,
				FilterTable0.ReadRuleFromTable(2)->status);
		}
		LOG_MSG_DEBUG("Leaving function\n");
		return true;

	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint16_t ether_type = ETH_P_IP;
		uint16_t wrong_ether_type = 0x1234;

		// DST && SRC correct
		memcpy(&m_sendBuffer1[ETH2_DST_ADDR_OFFSET],
			m_ETH2_DST_ADDR, ETH_ALEN);
		memcpy(&m_sendBuffer1[ETH2_SRC_ADDR_OFFSET],
			m_ETH2_SRC_ADDR, ETH_ALEN);

		// DST is wrong, ETH2 type is correct
		memcpy(&m_sendBuffer2[ETH2_DST_ADDR_OFFSET],
			m_ETH2_DST_ADDR, ETH_ALEN);
		m_sendBuffer2[ETH2_DST_ADDR_OFFSET] = 0x00;
		memcpy(&m_sendBuffer2[ETH2_ETH_TYPE_OFFSET],
			&ether_type, sizeof(ether_type));

		// DST is wrong, ETH2 type is wrong
		memcpy(&m_sendBuffer3[ETH2_DST_ADDR_OFFSET],
			m_ETH2_DST_ADDR, ETH_ALEN);
		m_sendBuffer3[ETH2_DST_ADDR_OFFSET] = 0x00;
		memcpy(&m_sendBuffer3[ETH2_ETH_TYPE_OFFSET],
			&wrong_ether_type, sizeof(wrong_ether_type));

		return true;
	}// ModifyPacktes ()
};

/*---------------------------------------------------------------------------*/
/* Test 02: Destination IP address and subnet mask match against LAN subnet  */
/*---------------------------------------------------------------------------*/
class IpaFilterEthIPv4Test02 : public IpaFilteringEthernetBridgingTestFixture
{
public:
	IpaFilterEthIPv4Test02()
	{
		m_name = "IpaFilterEthIPv4Test02";
		m_description =
			"Filtering block test 02 - Ethernet Bridge, ETH2 filters, \
			IPv4 address (EP Filtering Table, \
			Insert all rules in a single commit) \
			1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule \
			(all data goes to output pipe 0, 1  and 2 (accordingly)) \
			2. Generate and commit 3 ETH2 filtering rules: \
			All MAC SRC == (22eeddccbbaa) traffic goes to routing table 0 \
			All MAC ETH TYPE ==    (0801) traffic goes to routing table 1 \
			All (1)                       traffic goes to routing table 2";
		m_minIPAHwType = IPA_HW_v2_5;
		m_maxIPAHwType = IPA_HW_MAX;
		Register(*this);
		m_pCurrentProducer = &m_producer2;
	}

	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering");

		const char bypass0[] = "bypass0";
		const char bypass1[] = "bypass1";
		const char bypass2[] = "bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeIPv4BypassRoutingTables");
			return false;
		}

		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR(
				"m_routing.GetRoutingTable(&routing_table0=0x%p)",
				&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table1=0x%p)",
				&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			LOG_MSG_ERROR(
				"m_routing.GetRoutingTable(&routing_table2=0x%p)",
				&routing_table2);
			return false;
		}

		// Create 3 filter rules
		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST2_PROD,false,3);

		// Configuring Filtering Rule 0 - ETH2 SRC
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.rule.retain_hdr = 1; // retain header removed in producer pipe
		flt_rule_entry.flt_rule_hdl = -1; // return value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_MAC_SRC_ADDR_ETHER_II;
		memcpy(flt_rule_entry.rule.attrib.src_mac_addr_mask,
			m_MAC_ADDR_MASK_ALL,
			sizeof(flt_rule_entry.rule.attrib.src_mac_addr_mask));
		memcpy(flt_rule_entry.rule.attrib.src_mac_addr,
			m_ETH2_SRC_ADDR,
			sizeof(flt_rule_entry.rule.attrib.src_mac_addr));

		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			LOG_MSG_ERROR ("Adding RuleTable(0) to Filtering");
			return false;
		} else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x",
				FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,
				FilterTable0.ReadRuleFromTable(0)->status);
		}

		// Configuring Filtering Rule 1 - ETH2 type
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_MAC_ETHER_TYPE;
		flt_rule_entry.rule.attrib.ether_type = ETH_P_IP + 1;
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			LOG_MSG_ERROR ("Adding RuleTable(1) to Filtering");
			return false;
		} else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x",
				FilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,
				FilterTable0.ReadRuleFromTable(1)->status);
		}

		// Configuring Filtering Rule 2 - Accept all
		flt_rule_entry.rule.rt_tbl_hdl = routing_table2.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0x00000000; // Accept all
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x00000000; // Has no effect
		if (((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable()))
		{
			LOG_MSG_ERROR ("Adding RuleTable(2) to Filtering");
			return false;
		} else
		{
			LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x",
				FilterTable0.ReadRuleFromTable(2)->flt_rule_hdl,
				FilterTable0.ReadRuleFromTable(2)->status);
		}
		LOG_MSG_DEBUG("Leaving function\n");
		return true;

	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint16_t ether_type = ETH_P_IP;
		uint16_t wrong_ether_type = 0x1234;

		memcpy(&m_sendBuffer1[ETH2_SRC_ADDR_OFFSET],
			m_ETH2_SRC_ADDR, ETH_ALEN);
		memcpy(&m_sendBuffer1[ETH2_ETH_TYPE_OFFSET],
			&ether_type, sizeof(ether_type));

		memcpy(&m_sendBuffer2[ETH2_SRC_ADDR_OFFSET],
			m_ETH2_SRC_ADDR, ETH_ALEN);
		m_sendBuffer2[ETH2_SRC_ADDR_OFFSET] = 0x00;
		memcpy(&m_sendBuffer2[ETH2_DST_ADDR_OFFSET],
			m_ETH2_DST_ADDR, ETH_ALEN);
		ether_type++;
		memcpy(&m_sendBuffer2[ETH2_ETH_TYPE_OFFSET],
			&ether_type, sizeof(ether_type));

		memcpy(&m_sendBuffer3[ETH2_DST_ADDR_OFFSET],
			m_ETH2_SRC_ADDR, ETH_ALEN);
		memcpy(&m_sendBuffer3[ETH2_SRC_ADDR_OFFSET],
			m_ETH2_DST_ADDR, ETH_ALEN);
		memcpy(&m_sendBuffer3[ETH2_ETH_TYPE_OFFSET],
			&wrong_ether_type, sizeof(wrong_ether_type));

		return true;
	}// ModifyPacktes ()
};

/*---------------------------------------------------------------------------*/
/* Test 03: Destination IP address and subnet mask match against LAN subnet  */
/*---------------------------------------------------------------------------*/
class IpaFilterEthIPv4Test03 : public IpaFilteringEthernetBridgingTestFixture
{
public:
	IpaFilterEthIPv4Test03()
	{
		m_name = "IpaFilterEthIPv4Test03";
		m_description =
			"Filtering block test 03 - Ethernet Bridge, ETH2 filters, \
			IPv4 address (EP Filtering Table, \
			Insert all rules in a single commit) \
			1. Generate and commit three routing tables. \
			Each table contains a single \"bypass\" rule \
			(all data goes to output pipe 0, 1  and 2 (accordingly)) \
			2. Generate and commit 3 ETH2 filtering rules: \
			All MAC SRC == (22eeddccbbaa) traffic goes to routing table 0 \
			All MAC ETH TYPE ==    (0800) traffic goes to routing table 1 \
			All (1)                       traffic goes to routing table 2";
		m_minIPAHwType = IPA_HW_v2_5;
		m_maxIPAHwType = IPA_HW_MAX;
		Register(*this);
		m_pCurrentProducer = &m_producer2;
	}

	virtual bool AddRules()
	{
		LOG_MSG_DEBUG("Entering");

		const char bypass0[] = "bypass0";
		const char bypass1[] = "bypass1";
		const char bypass2[] = "bypass2";
		struct ipa_ioc_get_rt_tbl routing_table0,routing_table1,routing_table2;

		if (!CreateThreeIPv4BypassRoutingTables (bypass0,bypass1,bypass2))
		{
			printf("CreateThreeIPv4BypassRoutingTables");
			return false;
		}

		routing_table0.ip = IPA_IP_v4;
		strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
		if (!m_routing.GetRoutingTable(&routing_table0))
		{
			LOG_MSG_ERROR(
				"m_routing.GetRoutingTable(&routing_table0=0x%p)",
				&routing_table0);
			return false;
		}
		routing_table1.ip = IPA_IP_v4;
		strlcpy(routing_table1.name, bypass1, sizeof(routing_table1.name));
		if (!m_routing.GetRoutingTable(&routing_table1))
		{
			LOG_MSG_ERROR("m_routing.GetRoutingTable(&routing_table1=0x%p)",
				&routing_table1);
			return false;
		}

		routing_table2.ip = IPA_IP_v4;
		strlcpy(routing_table2.name, bypass2, sizeof(routing_table2.name));
		if (!m_routing.GetRoutingTable(&routing_table2))
		{
			LOG_MSG_ERROR(
				"m_routing.GetRoutingTable(&routing_table2=0x%p)",
				&routing_table2);
			return false;
		}

		// Create 3 filter rules
		IPAFilteringTable FilterTable0;
		struct ipa_flt_rule_add flt_rule_entry;
		FilterTable0.Init(IPA_IP_v4,IPA_CLIENT_TEST2_PROD,false,3);

		// Configuring Filtering Rule 0 - ETH2 DST
		FilterTable0.GeneratePresetRule(1,flt_rule_entry);
		flt_rule_entry.at_rear = true;
		flt_rule_entry.rule.retain_hdr = 1; // retain header removed in producer pipe
		flt_rule_entry.flt_rule_hdl = -1; // return value
		flt_rule_entry.status = -1; // return value
		flt_rule_entry.rule.action = IPA_PASS_TO_ROUTING;
		flt_rule_entry.rule.rt_tbl_hdl = routing_table0.hdl;

		// SRC
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_MAC_SRC_ADDR_ETHER_II;
		memcpy(flt_rule_entry.rule.attrib.src_mac_addr_mask,
			m_MAC_ADDR_MASK_ALL,
			sizeof(flt_rule_entry.rule.attrib.src_mac_addr_mask));
		memcpy(flt_rule_entry.rule.attrib.src_mac_addr,
			m_ETH2_SRC_ADDR,
			sizeof(flt_rule_entry.rule.attrib.src_mac_addr));
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			LOG_MSG_ERROR ("Adding RuleTable(0) to Filtering");
			return false;
		}

		// Configuring Filtering Rule 1 - ETH2 type
		flt_rule_entry.rule.rt_tbl_hdl=routing_table1.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_MAC_ETHER_TYPE;
		flt_rule_entry.rule.attrib.ether_type = ETH_P_IP;
		if ((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry))
		{
			LOG_MSG_ERROR ("Adding RuleTable(1) to Filtering");
			return false;
		}

		// Configuring Filtering Rule 2 - Accept all
		flt_rule_entry.rule.rt_tbl_hdl = routing_table2.hdl;
		flt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
		flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0x00000000; // Accept all
		flt_rule_entry.rule.attrib.u.v4.dst_addr = 0x00000000; // Has no effect
		if (((uint8_t)-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable()))
		{
			LOG_MSG_ERROR ("Adding RuleTable(2) to Filtering");
			return false;
		}

		LOG_MSG_DEBUG("flt rule status: s0=0x%x, s1=0x%x s2=0x%x",
			FilterTable0.ReadRuleFromTable(0)->status,
			FilterTable0.ReadRuleFromTable(1)->status,
			FilterTable0.ReadRuleFromTable(2)->status);

		LOG_MSG_DEBUG("Leaving function\n");
		return true;

	}// AddRules()

	virtual bool ModifyPackets()
	{
		uint16_t ether_type = ETH_P_IP;
		uint16_t wrong_ether_type = 0x1234;

		// SRC correct, DST wrong, ETH type wrong
		memcpy(&m_sendBuffer1[ETH2_SRC_ADDR_OFFSET],
			m_ETH2_SRC_ADDR, ETH_ALEN);
		memcpy(&m_sendBuffer1[ETH2_DST_ADDR_OFFSET],
			m_ETH2_DST_ADDR, ETH_ALEN);
		m_sendBuffer1[ETH2_DST_ADDR_OFFSET] = 0x00;
		memcpy(&m_sendBuffer1[ETH2_ETH_TYPE_OFFSET],
			&wrong_ether_type, sizeof(wrong_ether_type));

		// SRC wrong, DST wrong, ETH type correct
		memcpy(&m_sendBuffer2[ETH2_SRC_ADDR_OFFSET],
			m_ETH2_SRC_ADDR, ETH_ALEN);
		m_sendBuffer2[ETH2_SRC_ADDR_OFFSET] = 0x00;
		memcpy(&m_sendBuffer2[ETH2_DST_ADDR_OFFSET],
			m_ETH2_DST_ADDR, ETH_ALEN);
		m_sendBuffer2[ETH2_DST_ADDR_OFFSET] = 0x00;
		memcpy(&m_sendBuffer2[ETH2_ETH_TYPE_OFFSET],
			&ether_type, sizeof(ether_type));

		// SRC wrong, DST correct, ETH type wrong
		memcpy(&m_sendBuffer3[ETH2_SRC_ADDR_OFFSET],
			m_ETH2_SRC_ADDR, ETH_ALEN);
		m_sendBuffer3[ETH2_SRC_ADDR_OFFSET] = 0x00;
		memcpy(&m_sendBuffer3[ETH2_DST_ADDR_OFFSET],
			m_ETH2_DST_ADDR, ETH_ALEN);
		memcpy(&m_sendBuffer3[ETH2_ETH_TYPE_OFFSET],
			&wrong_ether_type, sizeof(wrong_ether_type));

		return true;
	}// ModifyPacktes ()
};

static IpaFilterEthIPv4Test00 ipaFilterEthIPv4Test00;
static IpaFilterEthIPv4Test01 ipaFilterEthIPv4Test01;
static IpaFilterEthIPv4Test02 ipaFilterEthIPv4Test02;
static IpaFilterEthIPv4Test03 ipaFilterEthIPv4Test03;
