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

#include "HeaderRemovalTestFixture.h"

#include "Constants.h"
#include "Logger.h"
#include "IPAFilteringTable.h"

#define IPA_TEST_DMUX_HEADER_LENGTH           8
#define IPA_TEST_META_DATA_IS_VALID           1
#define IPA_TEST_DMUX_HEADER_META_DATA_OFFSET 4

extern Logger g_Logger;

/////////////////////////////////////////////////////////////////////////////////

//define the static Pipes which will be used by all derived tests.
Pipe HeaderRemovalTestFixture::m_A2NDUNToIpaPipe(IPA_CLIENT_TEST2_PROD, IPA_TEST_CONFIFURATION_3);
Pipe HeaderRemovalTestFixture::m_IpaToUsbPipe(IPA_CLIENT_TEST_CONS, IPA_TEST_CONFIFURATION_3);
Pipe HeaderRemovalTestFixture::m_IpaToA2NDUNPipe(IPA_CLIENT_TEST2_CONS, IPA_TEST_CONFIFURATION_3);
Pipe HeaderRemovalTestFixture::m_IpaToQ6LANPipe(IPA_CLIENT_TEST4_CONS, IPA_TEST_CONFIFURATION_3);
RoutingDriverWrapper   HeaderRemovalTestFixture::m_routing;
Filtering HeaderRemovalTestFixture::m_filtering;
const char HeaderRemovalTestFixture_bypass0[20] = "Bypass0";
const char HeaderRemovalTestFixture_bypassIPv60[20] = "BypassIPv60";

/////////////////////////////////////////////////////////////////////////////////

HeaderRemovalTestFixture::HeaderRemovalTestFixture()
{
	m_testSuiteName.push_back("Removal");
	Register(*this);
}

static int SetupKernelModule(void)
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
			IPA_CLIENT_TEST_CONS,
			(void *)&from_ipa_cfg[0],
			sizeof(from_ipa_cfg[0]));
	from_ipa_array[0] = &from_ipa_channels[0];

	memset(&from_ipa_cfg[1], 0, sizeof(from_ipa_cfg[1]));
	prepare_channel_struct(&from_ipa_channels[1],
			header.from_ipa_channels_num++,
			IPA_CLIENT_TEST2_CONS,
			(void *)&from_ipa_cfg[1],
			sizeof(from_ipa_cfg[1]));
	from_ipa_array[1] = &from_ipa_channels[1];

	memset(&from_ipa_cfg[2], 0, sizeof(from_ipa_cfg[2]));
	prepare_channel_struct(&from_ipa_channels[2],
			header.from_ipa_channels_num++,
			IPA_CLIENT_TEST4_CONS,
			(void *)&from_ipa_cfg[2],
			sizeof(from_ipa_cfg[2]));
	from_ipa_array[2] = &from_ipa_channels[2];

	/* To ipa configurations - 1 pipes */
	memset(&to_ipa_cfg[0], 0, sizeof(to_ipa_cfg[0]));
	to_ipa_cfg[0].hdr.hdr_len = IPA_TEST_DMUX_HEADER_LENGTH;
	to_ipa_cfg[0].hdr.hdr_ofst_metadata_valid = IPA_TEST_META_DATA_IS_VALID;
	to_ipa_cfg[0].hdr.hdr_ofst_metadata =
		IPA_TEST_DMUX_HEADER_META_DATA_OFFSET;
	prepare_channel_struct(&to_ipa_channels[0],
			header.to_ipa_channels_num++,
			IPA_CLIENT_TEST2_PROD,
			(void *)&to_ipa_cfg[0],
			sizeof(to_ipa_cfg[0]));
	to_ipa_array[0] = &to_ipa_channels[0];

	header.head_marker = IPA_TEST_CONFIG_MARKER;
	header.tail_marker = IPA_TEST_CONFIG_MARKER;

	prepare_header_struct(&header, from_ipa_array, to_ipa_array);

	retval = GenericConfigureScenario(&header);

	return retval;
}

/////////////////////////////////////////////////////////////////////////////////

bool HeaderRemovalTestFixture::Setup()
{
	bool bRetVal = true;

	//Set the configuration to support USB->IPA and IPA->USB pipes.
	//ConfigureScenario(PHASE_THREE_TEST_CONFIGURATION);

	bRetVal = SetupKernelModule();
	if (bRetVal != true) {
		return bRetVal;
	}

	//Initialize the pipe for all the tests - this will open the inode which represents the pipe.
	bRetVal &= m_A2NDUNToIpaPipe.Init();
	bRetVal &= m_IpaToUsbPipe.Init();
	bRetVal &= m_IpaToA2NDUNPipe.Init();
	bRetVal &= m_IpaToQ6LANPipe.Init();

	// remove default "LAN" routing table (as we want to pass to USB pipe)
	m_routing.Reset(IPA_IP_v4);
	m_routing.Reset(IPA_IP_v6);

	return bRetVal;
}

/////////////////////////////////////////////////////////////////////////////////

bool HeaderRemovalTestFixture::Teardown()
{
	//The Destroy method will close the inode.
	m_A2NDUNToIpaPipe.Destroy();
	m_IpaToUsbPipe.Destroy();
	m_IpaToA2NDUNPipe.Destroy();
	m_IpaToQ6LANPipe.Destroy();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////

Byte* HeaderRemovalTestFixture::CreateA2NDUNPacket(
		unsigned int magicNumber,
		unsigned int nID,
		string sPayloadFileName,
		unsigned int *nTotalLength)
{
	size_t nIpv4ByteSize = 1024;
	bool bRetVal = false;
	Byte *pA2NDUNPacket = 0;
	unsigned int nA2NDUNPacketByteSize = 0;
	Byte *pIpv4Packet = (Byte*) malloc(1024);

	if(0 == pIpv4Packet)
	{
		LOG_MSG_ERROR("Cannot allocate the memory for IPv4 packet");
		return 0;
	}

	bRetVal = LoadDefaultPacket(IPA_IP_v4, pIpv4Packet, nIpv4ByteSize);
	if(false == bRetVal)
	{
		LOG_MSG_ERROR("Cannot load the packet");
		pA2NDUNPacket = 0;
		goto bail;
	}
	//Magic Number(4 Bytes) Logical Channel ID(2 Bytes)  Length(2 Bytes)
	nA2NDUNPacketByteSize = m_A2NDUNToIpaPipe.GetHeaderLengthAdd() +  nIpv4ByteSize;

	pA2NDUNPacket = new Byte[ nA2NDUNPacketByteSize ];

	//htobe32 for the magic number:
	pA2NDUNPacket[0] = (magicNumber & 0xFF000000) >> 24;//MSB
	pA2NDUNPacket[1] = (magicNumber & 0x00FF0000) >> 16;
	pA2NDUNPacket[2] = (magicNumber & 0x0000FF00) >>  8;
	pA2NDUNPacket[3] = (magicNumber & 0x000000FF) >>  0;//LSB

	//htobe16 for the Logical Channel ID:
	pA2NDUNPacket[4] = (nID & 0xFF00) >>  8;//MSB
	pA2NDUNPacket[5] = (nID & 0x00FF) >>  0;//LSB

	//htobe16 for the Length of the packet:
	pA2NDUNPacket[6] = (nA2NDUNPacketByteSize & 0xFF00) >> 8;//MSB
	pA2NDUNPacket[7] = (nA2NDUNPacketByteSize & 0x00FF) >> 0;//LSB

	//add the payload to the A2NDUN packet
	memcpy(&pA2NDUNPacket[8], pIpv4Packet, nIpv4ByteSize);

	*nTotalLength = nA2NDUNPacketByteSize;

/* fall through */

bail:

	Free(pIpv4Packet);

	return pA2NDUNPacket;
}

/////////////////////////////////////////////////////////////////////////////////

bool HeaderRemovalTestFixture::SetIPATablesToPassAllToSpecificClient(
		enum ipa_client_type  nClientTypeSrc,
		enum ipa_client_type nClientTypeDst)
{
	bool bRetVal = true;

	bRetVal = SetRoutingTableToPassAllToSpecificClient(nClientTypeDst);
	if(false == bRetVal)
		goto bail;
	bRetVal = SetFilterTableToPassAllToSpecificClient(nClientTypeSrc);
	if(false == bRetVal)
		goto bail;
	bRetVal = SetHeaderInsertionTableAddEmptyHeaderForTheClient(nClientTypeSrc);
	if(false == bRetVal)
		goto bail;
/* fall through */

bail:
	return bRetVal;
}

/////////////////////////////////////////////////////////////////////////////////

bool  HeaderRemovalTestFixture::SetFilterTableToPassAllToSpecificClient(
		enum ipa_client_type  nClientType)
{
	IPAFilteringTable         FilterTable;
	struct ipa_flt_rule_add   flt_rule_entry;
	struct ipa_ioc_get_rt_tbl sRoutingTable;

	sRoutingTable.ip = IPA_IP_v4;
	strlcpy(sRoutingTable.name, "Bypass0", sizeof(sRoutingTable.name));

	if (false == m_routing.GetRoutingTable(&sRoutingTable)) {
	  LOG_MSG_ERROR("Configure the routing block first");
	  return false;
	}

	FilterTable.Init(IPA_IP_v4, nClientType, false, 1);
	FilterTable.GeneratePresetRule(0, flt_rule_entry);
	flt_rule_entry.at_rear                        = true;
	flt_rule_entry.flt_rule_hdl                   = -1;
	flt_rule_entry.status                         = -1;
	flt_rule_entry.rule.action                    = IPA_PASS_TO_ROUTING;
	flt_rule_entry.rule.rt_tbl_hdl                = sRoutingTable.hdl;
	flt_rule_entry.rule.attrib.attrib_mask        = IPA_FLT_DST_ADDR;
	flt_rule_entry.rule.attrib.u.v4.dst_addr_mask = 0x00000000;
	flt_rule_entry.rule.attrib.u.v4.dst_addr      = 0x00000000;
	if ((-1 == FilterTable.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable.GetFilteringTable())) {
		LOG_MSG_INFO ("%s::Error Adding RuleTable(0) to Filtering, aborting...");
		return false;
	} else {
		LOG_MSG_INFO( "flt rule hdl0=0x%x, status=0x%x",
				FilterTable.ReadRuleFromTable(0)->flt_rule_hdl,
					FilterTable.ReadRuleFromTable(0)->status);
	}
	LOG_MSG_INFO("Leaving ");

	return true;
}
/////////////////////////////////////////////////////////////////////////////////

bool  HeaderRemovalTestFixture::SetRoutingTableToPassAllToSpecificClient(
		enum ipa_client_type  nClientType)
{
	if (!CreateBypassRoutingTablesIPv4(
			HeaderRemovalTestFixture_bypass0,
			nClientType)) {
		LOG_MSG_INFO("CreateThreeBypassRoutingTables Failed");
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool  HeaderRemovalTestFixture::SetHeaderInsertionTableAddEmptyHeaderForTheClient(
		enum ipa_client_type  nClientType)
{
	//TODO Header Removal: add header insertion data
	return true;
}

/////////////////////////////////////////////////////////////////////////////////
// This function creates IPv4 bypass routing entry and commits it.
bool HeaderRemovalTestFixture::CreateBypassRoutingTablesIPv4(
		const char * bypass0,
		enum ipa_client_type  nClientType)
{
	struct ipa_ioc_add_rt_rule *rt_rule0 = 0;
	struct ipa_rt_rule_add *rt_rule_entry;

	LOG_MSG_INFO("Entering");

	rt_rule0 = (struct ipa_ioc_add_rt_rule *)
		calloc(1, sizeof(struct ipa_ioc_add_rt_rule) +
				1*sizeof(struct ipa_rt_rule_add));
	if (!rt_rule0) {
		LOG_MSG_INFO("calloc failed to allocate rt_rule0");
		return false;
	}

	rt_rule0->num_rules = 1;
	rt_rule0->ip = IPA_IP_v4;
	rt_rule0->commit = true;
	strlcpy(rt_rule0->rt_tbl_name, bypass0, sizeof(rt_rule0->rt_tbl_name));

	rt_rule_entry = &rt_rule0->rules[0];
	rt_rule_entry->at_rear = 0;
	rt_rule_entry->rule.dst = nClientType;
//    rt_rule_entry->rule.hdr_hdl = hdr_entry->hdr_hdl; // gidons, there is no support for header insertion / removal yet.
	rt_rule_entry->rule.attrib.attrib_mask = IPA_FLT_DST_ADDR;
	rt_rule_entry->rule.attrib.u.v4.dst_addr = 0xaabbccdd;
	rt_rule_entry->rule.attrib.u.v4.dst_addr_mask = 0x00000000;// All Packets will get a "Hit"
	if (false == m_routing.AddRoutingRule(rt_rule0)) {
		LOG_MSG_INFO("Routing rule addition(rt_rule0) failed!");
		Free (rt_rule0);
		return false;
	}

	Free (rt_rule0);
	LOG_MSG_INFO("Leaving ");
	return true;
}

bool HeaderRemovalTestFixture::ConfigureFilteringBlockWithMetaDataEq(
		enum ipa_client_type  nClientType,
		unsigned int nMetaData,
		unsigned int nMetaDataMask)
{
	const char bypass0[20] = "Bypass0";
	struct ipa_ioc_get_rt_tbl routing_table0;
	IPAFilteringTable FilterTable0;
	struct ipa_flt_rule_add flt_rule_entry;

	LOG_MSG_INFO("Entering ");

	if (!CreateBypassRoutingTablesIPv4(
			HeaderRemovalTestFixture_bypass0,
				nClientType)) {
		LOG_MSG_INFO("CreateBypassRoutingTablesIPv4 Failed");
		return false;
	}

	LOG_MSG_INFO("CreateBypassRoutingTablesIPv4 completed successfully");
	routing_table0.ip = IPA_IP_v4;
	strlcpy(routing_table0.name, bypass0, sizeof(routing_table0.name));
	if (!m_routing.GetRoutingTable(&routing_table0)) {
		LOG_MSG_INFO(
				"m_routing.GetRoutingTable(&routing_table0=0x%p) Failed."
					,&routing_table0);
		return false;
	}

	FilterTable0.Init(IPA_IP_v4, IPA_CLIENT_TEST2_PROD, false, 1);

	LOG_MSG_INFO("FilterTable*.Init Completed Successfully..");

	// Configuring Filtering Rule No.0
	FilterTable0.GeneratePresetRule(1,flt_rule_entry);
	flt_rule_entry.at_rear                        = true;
	flt_rule_entry.flt_rule_hdl                   = -1; // return Value
	flt_rule_entry.status                         = -1; // return value
	flt_rule_entry.rule.action                    = IPA_PASS_TO_ROUTING;
	flt_rule_entry.rule.rt_tbl_hdl                = routing_table0.hdl; //put here the handle corresponding to Routing Rule 1
	flt_rule_entry.rule.attrib.attrib_mask        = IPA_FLT_META_DATA;
	flt_rule_entry.rule.attrib.meta_data          = nMetaData;
	flt_rule_entry.rule.attrib.meta_data_mask     = nMetaDataMask;
	if ( (-1 == FilterTable0.AddRuleToTable(flt_rule_entry)) ||
			!m_filtering.AddFilteringRule(FilterTable0.GetFilteringTable()))
	{
		LOG_MSG_INFO ("%s::Error Adding RuleTable(0) to Filtering, aborting...");
		return false;
	} else {
		LOG_MSG_INFO("flt rule hdl0=0x%x, status=0x%x", FilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,FilterTable0.ReadRuleFromTable(0)->status);
	}

	LOG_MSG_INFO("Leaving ");

	return true;
}
