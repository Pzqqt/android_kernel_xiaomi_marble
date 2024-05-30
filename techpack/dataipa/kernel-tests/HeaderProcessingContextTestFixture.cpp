/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

#include "HeaderProcessingContextTestFixture.h"
#include "TestsUtils.h"

const Byte IpaHdrProcCtxTestFixture::WLAN_ETH2_HDR[WLAN_ETH2_HDR_SIZE] =
{
	// WLAN hdr - 4 bytes
	0xa1, 0xb2, 0xc3, 0xd4,

	// ETH2 - 14 bytes
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};

const Byte IpaHdrProcCtxTestFixture::ETH2_HDR[ETH_HLEN] =
{
	// ETH2 - 14 bytes
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};

const Byte IpaHdrProcCtxTestFixture::ETH2_8021Q_HDR[ETH8021Q_HEADER_LEN] =
{
	// 802_1Q - 18 bytes
	// src and dst MAC - 6 + 6 bytes
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	// 802_1Q tag - VLAN ID 3
	0x81, 0x00, 0x00, 0x03,
	// ethertype
	0x00, 0x00
};

const Byte IpaHdrProcCtxTestFixture::WLAN_802_3_HDR[WLAN_802_3_HDR_SIZE] =
{
	// WLAN hdr - 4 bytes
	0x0a, 0x0b, 0x0c, 0x0d,

	// 802_3 - 26 bytes
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};

const Byte IpaHdrProcCtxTestFixture::EoGRE_V4_HDR[EoGRE_V4_HDR_LEN] =
{
	// IP V4 header
	0x45, 0x00, 0x00, 0x53, /* 0x00, 0x53 is the total length...who fills this */
	0x00, 0x00, 0x40, 0x00,
	0x40, 0x2f, 0x00, 0x00, // 0x2f Protocol (Generic Routing Encapsulation)
	0x00, 0x00, 0x00, 0x00, // src address here
	0x00, 0x00, 0x00, 0x00, // dest address here

	// GRE gretap header
	0x00, 0x00, 0x65, 0x58
};

const Byte IpaHdrProcCtxTestFixture::EoGRE_V6_HDR[EoGRE_V6_HDR_LEN] =
{
	// IP V6 header
	0x60, 0x00, 0x00, 0x00,
	0x00, 0x47, 0x2f, 0x01, // 0x2f Protocol (Generic Routing Encapsulation)
	0x00, 0x00, 0x00, 0x00, // src address here
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, // dest address here
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,

	// options header
	0x2f, 0x00, 0x04, 0x01,
	0x04, 0x01, 0x01, 0x00,

	// GRE gretap header
	0x00, 0x00, 0x65, 0x58
};

IpaHdrProcCtxTestFixture::IpaHdrProcCtxTestFixture():
	m_procCtxHandleId(PROC_CTX_HANDLE_ID_MAX),
	m_headerHandleId(HEADER_HANDLE_ID_MAX),
	m_pCurrentProducer(NULL),
	m_pCurrentConsumer(NULL),
	m_sendSize1 (m_BUFF_MAX_SIZE),
	m_sendSize2 (m_BUFF_MAX_SIZE),
	m_expectedBufferSize1(0),
	m_IpaIPType(IPA_IP_v4)
{
	memset(m_headerHandles, 0, sizeof(m_headerHandles));
	memset(m_procCtxHHandles, 0, sizeof(m_procCtxHHandles));
	memset(m_sendBuffer1, 0, sizeof(m_sendBuffer1));
	memset(m_sendBuffer2, 0, sizeof(m_sendBuffer2));
	memset(m_expectedBuffer1, 0, sizeof(m_expectedBuffer1));
	memset(&m_ip_addrs, 0, sizeof(m_ip_addrs));
	m_testSuiteName.push_back("HdrProcCtx");
}

#define stringit(x) #x

bool IpaHdrProcCtxTestFixture::Setup()
{
	ConfigureScenario(PHASE_TWENTY_TEST_CONFIGURATION);

	// init producers
	m_rndisEth2Producer.Open(INTERFACE0_TO_IPA_DATA_PATH,
		INTERFACE0_FROM_IPA_DATA_PATH,
		stringit(m_rndisEth2Producer));

	m_wlanEth2producer.Open(INTERFACE4_TO_IPA_DATA_PATH,
		INTERFACE4_FROM_IPA_DATA_PATH,
		stringit(m_wlanEth2producer));

	m_eth2Producer.Open(INTERFACE5_TO_IPA_DATA_PATH,
		INTERFACE5_FROM_IPA_DATA_PATH,
		stringit(m_eth2Producer));

	// init consumers
	m_defaultConsumer.Open(INTERFACE1_TO_IPA_DATA_PATH,
		INTERFACE1_FROM_IPA_DATA_PATH,
		stringit(m_defaultConsumer));

	m_rndisEth2Consumer.Open(INTERFACE2_TO_IPA_DATA_PATH,
		INTERFACE2_FROM_IPA_DATA_PATH,
		stringit(m_rndisEth2Consumer));

	if (!m_headerInsertion.DeviceNodeIsOpened())
	{
		LOG_MSG_ERROR("HeaderInsertion block is not ready "
			"for immediate commands!\n");
		return false;
	}

	if (!m_routing.DeviceNodeIsOpened())
	{
		LOG_MSG_ERROR("Routing block is not ready "
			"for immediate commands!\n");
		return false;
	}

	if (!m_filtering.DeviceNodeIsOpened())
	{
		LOG_MSG_ERROR("Filtering block is not ready "
			"for immediate commands!\n");
		return false;
	}

	// resetting this component will reset
	// both Routing and Filtering tables
	m_headerInsertion.Reset();

	return true;
} // Setup()

bool IpaHdrProcCtxTestFixture::Teardown()
{
	m_rndisEth2Producer.Close();
	m_wlanEth2producer.Close();
	m_eth2Producer.Close();
	m_defaultConsumer.Close();
	m_rndisEth2Consumer.Close();
	return true;
} // Teardown()

void IpaHdrProcCtxTestFixture::AddAllHeaders()
{
	for (int i = 0; i < HEADER_HANDLE_ID_MAX; i++) {
		AddHeader(static_cast<HeaderHandleId>(i));
	}
}

// Insert a single header
void IpaHdrProcCtxTestFixture::AddHeader(HeaderHandleId handleId)
{
	static const int NUM_OF_HEADERS = 1;

	uint32_t length =
		sizeof(struct ipa_ioc_add_hdr) +
		(NUM_OF_HEADERS * sizeof(struct ipa_hdr_add));

	uint8_t buf[length];

	struct ipa_ioc_add_hdr *hdrTable =
		(struct ipa_ioc_add_hdr *) buf;

	struct ipa_hdr_add *hdr;

	memset(buf, 0, sizeof(buf));

	// init hdr table
	hdrTable->commit   = true;
	hdrTable->num_hdrs = NUM_OF_HEADERS;

	// init the hdr common fields
	hdr             = &hdrTable->hdr[0];
	hdr->is_partial = false;
	hdr->hdr_hdl    = -1; // Return Value
	hdr->status     = -1; // Return Parameter

	// init hdr specific fields
	switch (handleId)
	{
	case HEADER_HANDLE_ID_WLAN_ETH2:
		memcpy(hdr->hdr, WLAN_ETH2_HDR, WLAN_ETH2_HDR_SIZE);
		hdr->hdr_len = WLAN_ETH2_HDR_SIZE;

		strlcpy(hdr->name, "WLAN_ETH2", sizeof(hdr->name));
		hdr->type = IPA_HDR_L2_ETHERNET_II;
		break;

	case HEADER_HANDLE_ID_RNDIS_ETH2:
		if (!RNDISAggregationHelper::LoadRNDISEth2IP4Header(
			hdr->hdr,
			IPA_HDR_MAX_SIZE,
			0,
			(size_t*)&hdr->hdr_len))
			return;

		strlcpy(hdr->name, "RNDIS_ETH2", sizeof(hdr->name));
		hdr->type = IPA_HDR_L2_ETHERNET_II;
		break;

	case HEADER_HANDLE_ID_ETH2:
		strlcpy(hdr->name, "ETH2", sizeof(hdr->name));
		memcpy(hdr->hdr, ETH2_HDR, ETH_HLEN);
		hdr->type = IPA_HDR_L2_ETHERNET_II;
		hdr->hdr_len = ETH_HLEN;

		break;

	case HEADER_HANDLE_ID_WLAN_802_3:
		strlcpy(hdr->name, "WLAN_802_3", sizeof(hdr->name));
		memcpy(hdr->hdr, WLAN_802_3_HDR, WLAN_802_3_HDR_SIZE);
		hdr->type = IPA_HDR_L2_802_3;
		hdr->hdr_len = WLAN_802_3_HDR_SIZE;

		LOG_MSG_DEBUG(
			"HEADER_HANDLE_ID_WLAN_802_3 NOT supported for now");
		return;

		break;

	case HEADER_HANDLE_ID_VLAN_802_1Q:
		strlcpy(hdr->name, "VLAN_8021Q", sizeof(hdr->name));
		memcpy(hdr->hdr, ETH2_8021Q_HDR, ETH8021Q_HEADER_LEN);
		hdr->type = IPA_HDR_L2_802_1Q;
		hdr->hdr_len = ETH8021Q_HEADER_LEN;
		break;

	case HEADER_HANDLE_ID_EoGRE_V4:
		strlcpy(hdr->name,
				"EoGRE in v4",
				sizeof(hdr->name));
		memcpy(hdr->hdr,
			   EoGRE_V4_HDR,
			   EoGRE_V4_HDR_LEN);
		hdr->hdr_len = EoGRE_V4_HDR_LEN;
		hdr->type    = IPA_HDR_L2_802_1Q;
		break;

	case HEADER_HANDLE_ID_EoGRE_V6:
		strlcpy(hdr->name,
				"EoGRE in v6",
				sizeof(hdr->name));
		memcpy(hdr->hdr,
			   EoGRE_V6_HDR,
			   EoGRE_V6_HDR_LEN);
		hdr->hdr_len = EoGRE_V6_HDR_LEN;
		hdr->type    = IPA_HDR_L2_802_1Q;
		break;

	default:
		LOG_MSG_ERROR("header handleId not supported.");
		return;
	}

	// commit header to HW
	if (!m_headerInsertion.AddHeader(hdrTable))
	{
		LOG_MSG_ERROR("m_headerInsertion.AddHeader() failed.");
		return;
	}

	// save header handle
	m_headerHandles[handleId] = hdr->hdr_hdl;
}

void IpaHdrProcCtxTestFixture::AddAllProcCtx()
{
	for (int i = 0; i <PROC_CTX_HANDLE_ID_MAX; i++)
	{
		AddProcCtx(static_cast<ProcCtxHandleId>(i));
	}
}

// Insert a single proc_ctx
void IpaHdrProcCtxTestFixture::AddProcCtx(ProcCtxHandleId handleId)
{
	static const int NUM_OF_PROC_CTX = 1;

	uint32_t length =
		sizeof(struct ipa_ioc_add_hdr_proc_ctx) +
		(NUM_OF_PROC_CTX * sizeof(struct ipa_hdr_proc_ctx_add));

	uint8_t buf[length];

	struct ipa_ioc_add_hdr_proc_ctx *procCtxTable =
		(struct ipa_ioc_add_hdr_proc_ctx *) buf;

	struct ipa_hdr_proc_ctx_add *procCtx;

	memset(buf, 0, sizeof(buf));

	// init proc ctx table
	procCtxTable->commit        = true;
	procCtxTable->num_proc_ctxs = NUM_OF_PROC_CTX;

	// init proc_ctx common fields
	procCtx               = &procCtxTable->proc_ctx[0];
	procCtx->proc_ctx_hdl = -1; // return value
	procCtx->status       = -1; // Return parameter

	// init proc_ctx specific fields
	switch (handleId)
	{
	case PROC_CTX_HANDLE_ID_ETH2_2_WLAN_ETH2:
		procCtx->type = IPA_HDR_PROC_ETHII_TO_ETHII;
		procCtx->hdr_hdl =
			m_headerHandles[HEADER_HANDLE_ID_WLAN_ETH2];
		break;

	case PROC_CTX_HANDLE_ID_ETH2_2_RNDIS_ETH2:
		procCtx->type = IPA_HDR_PROC_ETHII_TO_ETHII;
		procCtx->hdr_hdl =
			m_headerHandles[HEADER_HANDLE_ID_RNDIS_ETH2];
		break;

	case PROC_CTX_HANDLE_ID_ETH2_ETH2_2_ETH2:
		procCtx->type = IPA_HDR_PROC_ETHII_TO_ETHII;
		procCtx->hdr_hdl =
			m_headerHandles[HEADER_HANDLE_ID_ETH2];
		break;

	case PROC_CTX_HANDLE_ID_WLAN_ETH2_2_802_3:
		procCtx->type = IPA_HDR_PROC_ETHII_TO_802_3;
		procCtx->hdr_hdl =
			m_headerHandles[HEADER_HANDLE_ID_WLAN_802_3];
		break;

	case PROC_CTX_HANDLE_ID_RNDIS_802_3_2_ETH2:
		procCtx->type = IPA_HDR_PROC_802_3_TO_ETHII;
		procCtx->hdr_hdl =
			m_headerHandles[HEADER_HANDLE_ID_RNDIS_ETH2];
		break;

	case PROC_CTX_HANDLE_ID_WLAN_802_3_2_ETH2:
		procCtx->type = IPA_HDR_PROC_802_3_TO_802_3;
		procCtx->hdr_hdl =
			m_headerHandles[HEADER_HANDLE_ID_WLAN_802_3];
		break;
	case PROC_CTX_HANDLE_ID_802_1Q_2_802_1Q:
		procCtx->type = IPA_HDR_PROC_ETHII_TO_ETHII_EX;
		procCtx->hdr_hdl =
			m_headerHandles[HEADER_HANDLE_ID_VLAN_802_1Q];
		procCtx->generic_params.input_ethhdr_negative_offset = 18;
		procCtx->generic_params.output_ethhdr_negative_offset = 18;
		break;
	case PROC_CTX_HANDLE_ID_802_1Q_2_ETH2:
		procCtx->type = IPA_HDR_PROC_ETHII_TO_ETHII_EX;
		procCtx->hdr_hdl =
			m_headerHandles[HEADER_HANDLE_ID_ETH2];
		procCtx->generic_params.input_ethhdr_negative_offset = 18;
		procCtx->generic_params.output_ethhdr_negative_offset = 14;
		break;
	case PROC_CTX_HANDLE_ID_ETH2_2_802_1Q:
		procCtx->type = IPA_HDR_PROC_ETHII_TO_ETHII_EX;
		procCtx->hdr_hdl =
			m_headerHandles[HEADER_HANDLE_ID_VLAN_802_1Q];
		procCtx->generic_params.input_ethhdr_negative_offset = 14;
		procCtx->generic_params.output_ethhdr_negative_offset = 18;
		break;
	case PROC_CTX_HANDLE_ID_ETH2_ETH2_2_ETH2_EX:
		procCtx->type = IPA_HDR_PROC_ETHII_TO_ETHII_EX;
		procCtx->hdr_hdl =
			m_headerHandles[HEADER_HANDLE_ID_ETH2];
		procCtx->generic_params.input_ethhdr_negative_offset = 14;
		procCtx->generic_params.output_ethhdr_negative_offset = 14;
		break;
	case PROC_CTX_HANDLE_ID_EoGRE_HDR_ADD:
		procCtx->type = IPA_HDR_PROC_EoGRE_HEADER_ADD;
		procCtx->hdr_hdl = m_headerHandles[m_headerHandleId];
		procCtx->eogre_params.hdr_add_param.eth_hdr_retained = 1;
		procCtx->eogre_params.hdr_add_param.output_ip_version =
			(m_headerHandleId == HEADER_HANDLE_ID_EoGRE_V4) ? 0 : 1;
		procCtx->eogre_params.hdr_add_param.second_pass = 0; // use 1 in ipacm
		break;
	case PROC_CTX_HANDLE_ID_EoGRE_HDR_REMOVE:
		procCtx->type = IPA_HDR_PROC_EoGRE_HEADER_REMOVE;
		procCtx->hdr_hdl = m_headerHandles[m_headerHandleId];
		procCtx->eogre_params.hdr_remove_param.hdr_len_remove =
			(m_headerHandleId == HEADER_HANDLE_ID_EoGRE_V4) ?
			EoGRE_V4_HDR_LEN :
			EoGRE_V6_HDR_LEN;
		break;
	default:
		LOG_MSG_ERROR("proc ctx handleId %d not supported.", handleId);
		return;
	}

	if (!m_headerInsertion.AddProcCtx(procCtxTable))
	{
		LOG_MSG_ERROR("m_headerInsertion.AddProcCtx(procCtxTable) failed.");
		return;
	}

	// save proc_ctx handle
	m_procCtxHHandles[handleId] = procCtx->proc_ctx_hdl;
}

void IpaHdrProcCtxTestFixture::AddRtBypassRule(uint32_t hdrHdl, uint32_t procCtxHdl)
{
	static const char bypass0[] = "bypass0";
	struct ipa_ioc_get_rt_tbl routing_table0;

	if (!CreateIPv4BypassRoutingTable (
		bypass0,
		hdrHdl,
		procCtxHdl))
	{
		LOG_MSG_ERROR("CreateIPv4BypassRoutingTable Failed\n");
		return;
	}

	routing_table0.ip = IPA_IP_v4;
	strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
	if (!m_routing.GetRoutingTable(&routing_table0))
	{
		LOG_MSG_ERROR("m_routing.GetRoutingTable() Failed.");
		return;
	}

	m_routingTableHdl = routing_table0.hdl;
}

void IpaHdrProcCtxTestFixture::AddFltBypassRule()
{
	IPAFilteringTable       FilterTable0;
	struct ipa_flt_rule_add flt_rule_entry;

	FilterTable0.Init(IPA_IP_v4, m_currProducerClient, false, 1);

	printf("FilterTable*.Init Completed Successfully..\n");

	/*
	 * Configuring Filtering Rule No.0
	 */
	FilterTable0.GeneratePresetRule(1, flt_rule_entry);

	flt_rule_entry.at_rear                        = true;
	flt_rule_entry.flt_rule_hdl                   = -1; // return Value
	flt_rule_entry.status                         = -1; // return value
	flt_rule_entry.rule.action                    = IPA_PASS_TO_ROUTING;
	flt_rule_entry.rule.rt_tbl_hdl                = m_routingTableHdl;
	flt_rule_entry.rule.attrib.attrib_mask        = IPA_FLT_DST_ADDR;
	flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0x00000000; // Mask - Bypass rule
	flt_rule_entry.rule.attrib.u.v4.dst_addr      = 0x12345678; // Filter is irrelevant.

	if ( (FilterTable0.AddRuleToTable(flt_rule_entry) == (uint8_t) -1) ||
		 m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable()) == false )
	{
		LOG_MSG_ERROR(
			"%s::m_filtering.AddFilteringRule() failed",
			__FUNCTION__);
		return;
	}
	else
	{
		printf("flt rule hdl0=0x%x, status=0x%x\n",
			FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,
			FilterTable0.ReadRuleFromTable(0)->status);
	}
}

bool IpaHdrProcCtxTestFixture::ReceivePacketsAndCompare()
{
	size_t receivedBufferSize1 = 0;
	bool isSuccess = true;

	// Receive results
	Byte *receivedBuffer1 = new Byte[m_BUFF_MAX_SIZE];

	if (NULL == receivedBuffer1)
	{
		printf("Memory allocation error.\n");
		return false;
	}

	receivedBufferSize1 = m_pCurrentConsumer->ReceiveData(
		receivedBuffer1,
		m_BUFF_MAX_SIZE);
	printf("Received %zu bytes on %s.\n",
		receivedBufferSize1,
		m_pCurrentConsumer->m_fromChannelName.c_str());

	// Compare results
	if (!CompareResultVsGolden(
		m_expectedBuffer1,
		m_expectedBufferSize1,
		receivedBuffer1,
		receivedBufferSize1))
	{
		printf("Comparison of Buffer Failed!\n");
		isSuccess = false;
	}

	printf("Expected buffer 1 - %zu bytes\n", m_expectedBufferSize1);
	print_buff(m_expectedBuffer1, m_expectedBufferSize1);

	printf("Received buffer 1 - %zu bytes\n", receivedBufferSize1);
	print_buff(receivedBuffer1, receivedBufferSize1);

	delete[] receivedBuffer1;

	return isSuccess;
}

bool IpaHdrProcCtxTestFixture::ReceivePacketsAndCompare(
	Byte*  receivedBuffer,
	size_t receivedBufferSize )
{
	bool isSuccess = true;

	// Compare results
	if ( ! CompareResultVsGolden(
			 m_expectedBuffer1,
			 m_expectedBufferSize1,
			 receivedBuffer,
			 receivedBufferSize))
	{
		printf("Comparison of Buffer Failed!\n");
		isSuccess = false;
	}

	print_buffer(m_expectedBuffer1, m_expectedBufferSize1, "Expected buffer");
	print_buffer(receivedBuffer,    receivedBufferSize,    "Received buffer");

	return isSuccess;
}

// Create 1 IPv4 bypass routing entry and commits it
bool IpaHdrProcCtxTestFixture::CreateIPv4BypassRoutingTable (
	const char *name,
	uint32_t hdrHdl,
	uint32_t procCtxHdl)
{
	printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);
	struct ipa_ioc_add_rt_rule *rt_table = 0;
	struct ipa_rt_rule_add *rt_rule_entry = NULL;

	// Verify that only one is nonzero
	if ((hdrHdl == 0 && procCtxHdl == 0) ||
		(hdrHdl != 0 && procCtxHdl != 0))
	{
		LOG_MSG_ERROR("Error: hdrHdl = %u, procCtxHdl = %u\n");
		return false;
	}

	rt_table = (struct ipa_ioc_add_rt_rule *)
		calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
		1*sizeof(struct ipa_rt_rule_add));
	if (!rt_table) {
		LOG_MSG_ERROR("calloc failed to allocate rt_table\n");
		return false;
	}

	rt_table->num_rules = 1;
	rt_table->ip = IPA_IP_v4;
	rt_table->commit = true;
	strlcpy(rt_table->rt_tbl_name, name, sizeof(rt_table->rt_tbl_name));

	rt_rule_entry = &rt_table->rules[0];
	rt_rule_entry->at_rear = 0;
	rt_rule_entry->rule.dst = m_currConsumerPipeNum;
	rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
	rt_rule_entry->rule.attrib.u.v4.dst_addr = 0xaabbccdd;

	// All Packets will get a "Hit"
	rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0x00000000;
	rt_rule_entry->rule.hdr_hdl = hdrHdl;
	rt_rule_entry->rule.hdr_proc_ctx_hdl = procCtxHdl;
	if (false == m_routing.AddRoutingRule(rt_table))
	{
		printf("Routing rule addition(rt_table) failed!\n");
		Free (rt_table);
		return false;
	}

	Free (rt_table);
	printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
	return true;
}

bool IpaHdrProcCtxTestFixture::AddRoutingRule(
	const char*          name,
	uint32_t             hdrHdl,
	uint32_t             procCtxHdl,
	ipa_ip_type          iptype,
	union ipa_ip_params  ip_addrs,
	enum ipa_client_type dst )
{
	static const int NUM_RT_RULE = 1;

	struct ipa_rt_rule_add rt_rule_entry;

	uint8_t buf[
		sizeof(struct ipa_ioc_add_rt_rule) +
		(NUM_RT_RULE * sizeof(struct ipa_rt_rule_add)) ];

	struct ipa_ioc_add_rt_rule* rt_table =
		(struct ipa_ioc_add_rt_rule*) buf;

	memset(buf, 0, sizeof(buf));

	printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

	/*
	 * Only one or the other can and has to be set, hence...
	 */
	if ((hdrHdl == 0 && procCtxHdl == 0) ||
		(hdrHdl != 0 && procCtxHdl != 0))
	{
		LOG_MSG_ERROR("Error: hdrHdl = %u, procCtxHdl = %u\n");
		return false;
	}

	strlcpy(rt_table->rt_tbl_name, name, sizeof(rt_table->rt_tbl_name));

	rt_table->num_rules = NUM_RT_RULE;
	rt_table->ip        = iptype;
	rt_table->commit    = true;

	/*
	 * Add first rule
	 */
	memset(&rt_rule_entry, 0, sizeof(rt_rule_entry));

	rt_rule_entry.at_rear                 = 0;

	rt_rule_entry.rule.dst                = dst;
	rt_rule_entry.rule.hdr_hdl            = hdrHdl;
	rt_rule_entry.rule.hdr_proc_ctx_hdl   = procCtxHdl;
	rt_rule_entry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
	rt_rule_entry.rule.hashable           = 1;
	rt_rule_entry.rule.retain_hdr         = 1;

	if ( iptype == IPA_IP_v4 )
	{
		memcpy(
			&rt_rule_entry.rule.attrib.u.v4,
			&ip_addrs.v4,
			sizeof(rt_rule_entry.rule.attrib.u.v4));
	}
	else
	{
		memcpy(
			&rt_rule_entry.rule.attrib.u.v6,
			&ip_addrs.v6,
			sizeof(rt_rule_entry.rule.attrib.u.v6));
	}

	memcpy(
		&(rt_table->rules[0]),
		&rt_rule_entry,
		sizeof(struct ipa_rt_rule_add));

	if ( m_routing.AddRoutingRule(rt_table) == false )
	{
		printf("Routing rule addition(rt_table) failed!\n");
		return false;
	}

	printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);

	return true;
}

bool IpaHdrProcCtxTestFixture::AddFilteringRule(
	ipa_ip_type          iptype,
	union ipa_ip_params  ip_addrs,
	enum ipa_client_type src,
	uint32_t             rt_tbl_hdl )
{
	static const int NUM_FLT_RULE = 1;

	struct ipa_flt_rule_add flt_rule_entry;

	uint8_t buf[
		sizeof(struct ipa_ioc_add_flt_rule) +
		(NUM_FLT_RULE * sizeof(struct ipa_flt_rule_add)) ];

	struct ipa_ioc_add_flt_rule* flt_rule =
		(struct ipa_ioc_add_flt_rule*) buf;

	memset(buf, 0, sizeof(buf));

	printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

	flt_rule->commit    = 1;
	flt_rule->ep        = src;
	flt_rule->global    = false;
	flt_rule->ip        = iptype;
	flt_rule->num_rules = NUM_FLT_RULE;

	/*
	 * Add first rule
	 */
	memset(&flt_rule_entry, 0, sizeof(flt_rule_entry));

	flt_rule_entry.at_rear                  = true; // FIXME FINDME correct?
	flt_rule_entry.flt_rule_hdl             = -1;
	flt_rule_entry.status                   = -1;

	flt_rule_entry.rule.retain_hdr          = 1; // FIXME FINDME correct?
	flt_rule_entry.rule.to_uc               = 1; // FIXME FINDME correct?
	flt_rule_entry.rule.action              = IPA_PASS_TO_ROUTING;
	flt_rule_entry.rule.rt_tbl_hdl          = rt_tbl_hdl;
	flt_rule_entry.rule.attrib.attrib_mask |= IPA_FLT_DST_ADDR;

	if ( iptype == IPA_IP_v4 )
	{
		memcpy(
			&flt_rule_entry.rule.attrib.u.v4,
			&ip_addrs.v4,
			sizeof(flt_rule_entry.rule.attrib.u.v4));
	}
	else
	{
		memcpy(
			&flt_rule_entry.rule.attrib.u.v6,
			&ip_addrs.v6,
			sizeof(flt_rule_entry.rule.attrib.u.v6));
	}

	memcpy(
		&(flt_rule->rules[0]),
		&flt_rule_entry,
		sizeof(struct ipa_flt_rule_add));

	if ( m_filtering.AddFilteringRule(flt_rule) == false )
	{
		printf("Error Adding Filtering rule, aborting...\n");
		return false;
	}

	printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);

	return true;
}

bool IpaHdrProcCtxTestFixture::AddRules()
{
	printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

	if (m_procCtxHandleId == PROC_CTX_HANDLE_ID_MAX)
	{
		LOG_MSG_ERROR("Test developer didn't implement "
			"AddRules() or didn't set m_procCtxHandleId");
		return false;
	}

	AddAllHeaders();

	AddAllProcCtx();

	AddRtBypassRule(0, m_procCtxHHandles[m_procCtxHandleId]);

	AddFltBypassRule();

	printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
	return true;

}// AddRules()

bool IpaHdrProcCtxTestFixture::SendPackets()
{

	bool isSuccess = false;

	// Send first packet
	isSuccess = m_pCurrentProducer->SendData(
		m_sendBuffer1,
		m_sendSize1);
	if (false == isSuccess)
	{
		LOG_MSG_ERROR("SendPackets Buffer1 failed on client %d\n", m_currProducerClient);
		return false;
	}

	return true;
}

bool IpaHdrProcCtxTestFixture::Run()
{
	bool res = false;
	bool isSuccess = false;

	printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

	res = AddRules();
	if (false == res) {
		printf("Failed adding filtering rules.\n");
		return false;
	}

	// Load input data - IP packets
	res = LoadPackets(m_IpaIPType);
	if (false == res) {
		printf("Failed loading packets.\n");
		return false;
	}

	res = GenerateExpectedPackets();
	if (false == res) {
		printf("GenerateExpectedPackets failed\n");
		return false;
	}

	res = SendPackets();
	if (false == res) {
		printf("SendPackets failed\n");
		return false;
	}

	// Receive packets from the channels and compare results
	isSuccess = ReceivePacketsAndCompare();

	printf("Leaving %s, %s(), Returning %d\n",
		__FUNCTION__,
		__FILE__,
		isSuccess);

	return isSuccess;
} // Run()

IpaHdrProcCtxTestFixture::~IpaHdrProcCtxTestFixture()
{
	m_sendSize1 = 0;
}

RoutingDriverWrapper IpaHdrProcCtxTestFixture::m_routing;
Filtering IpaHdrProcCtxTestFixture::m_filtering;
HeaderInsertion IpaHdrProcCtxTestFixture::m_headerInsertion;

