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

#include "MBIMAggregationTestFixtureConf11.h"
#include "TestManager.h"

/////////////////////////////////////////////////////////////////////////////////

//define the static Pipes which will be used by all derived tests.
Pipe MBIMAggregationTestFixtureConf11::m_IpaToUsbPipeAgg(IPA_CLIENT_TEST2_CONS,
		IPA_TEST_CONFIGURATION_11);
Pipe MBIMAggregationTestFixtureConf11::m_UsbToIpaPipe(IPA_CLIENT_TEST_PROD,
		IPA_TEST_CONFIGURATION_11);
Pipe MBIMAggregationTestFixtureConf11::m_IpaToUsbPipe(IPA_CLIENT_TEST3_CONS,
		IPA_TEST_CONFIGURATION_11);
Pipe MBIMAggregationTestFixtureConf11::m_UsbToIpaPipeDeagg(IPA_CLIENT_TEST2_PROD,
		IPA_TEST_CONFIGURATION_11);
Pipe MBIMAggregationTestFixtureConf11::m_IpaToUsbPipeAggTime(IPA_CLIENT_TEST_CONS,
		IPA_TEST_CONFIGURATION_11);
Pipe MBIMAggregationTestFixtureConf11::m_IpaToUsbPipeAgg0Limits(IPA_CLIENT_TEST4_CONS,
		IPA_TEST_CONFIGURATION_11);

RoutingDriverWrapper MBIMAggregationTestFixtureConf11::m_Routing;
Filtering MBIMAggregationTestFixtureConf11::m_Filtering;
HeaderInsertion MBIMAggregationTestFixtureConf11::m_HeaderInsertion;

/////////////////////////////////////////////////////////////////////////////////

MBIMAggregationTestFixtureConf11::MBIMAggregationTestFixtureConf11(bool generic_agg)
: mGenericAgg(generic_agg)
{
	if (mGenericAgg) {
		m_testSuiteName.push_back("GenMbimAgg");
		m_minIPAHwType = IPA_HW_v4_0;
	} else {
		m_testSuiteName.push_back("Mbim16Agg");
		m_maxIPAHwType = IPA_HW_v3_5_1;
	}
	Register(*this);
}

/////////////////////////////////////////////////////////////////////////////////

int MBIMAggregationTestFixtureConf11::SetupKernelModule()
{
	int retval;
	struct ipa_channel_config from_ipa_channels[4];
	struct test_ipa_ep_cfg from_ipa_cfg[4];
	struct ipa_channel_config to_ipa_channels[2];
	struct test_ipa_ep_cfg to_ipa_cfg[2];

	struct ipa_test_config_header header = {0};
	struct ipa_channel_config *to_ipa_array[2];
	struct ipa_channel_config *from_ipa_array[4];

	/* From ipa configurations - 4 pipes */
	memset(&from_ipa_cfg[0], 0, sizeof(from_ipa_cfg[0]));
	from_ipa_cfg[0].aggr.aggr_en = IPA_ENABLE_AGGR;
	from_ipa_cfg[0].aggr.aggr = IPA_MBIM_16;
	from_ipa_cfg[0].aggr.aggr_byte_limit = 1;
	from_ipa_cfg[0].aggr.aggr_time_limit = 0;
	if (mGenericAgg) {
		from_ipa_cfg[0].hdr.hdr_len = 4;
		from_ipa_cfg[0].hdr_ext.hdr_pad_to_alignment = 2;
	} else {
		from_ipa_cfg[0].hdr.hdr_len = 1;
	}

	prepare_channel_struct(&from_ipa_channels[0],
			header.from_ipa_channels_num++,
			IPA_CLIENT_TEST2_CONS,
			(void *)&from_ipa_cfg[0],
			sizeof(from_ipa_cfg[0]),
			false);
	from_ipa_array[0] = &from_ipa_channels[0];

	memset(&from_ipa_cfg[1], 0, sizeof(from_ipa_cfg[1]));
	prepare_channel_struct(&from_ipa_channels[1],
			header.from_ipa_channels_num++,
			IPA_CLIENT_TEST3_CONS,
			(void *)&from_ipa_cfg[1],
			sizeof(from_ipa_cfg[1]),
			false);
	from_ipa_array[1] = &from_ipa_channels[1];

	memset(&from_ipa_cfg[2], 0, sizeof(from_ipa_cfg[2]));
	from_ipa_cfg[2].aggr.aggr_en = IPA_ENABLE_AGGR;
	from_ipa_cfg[2].aggr.aggr = IPA_MBIM_16;
	from_ipa_cfg[2].aggr.aggr_byte_limit = 1;
	from_ipa_cfg[2].aggr.aggr_time_limit = 30;
	if (TestManager::GetInstance()->GetIPAHwType() >= IPA_HW_v4_2)
		from_ipa_cfg[2].aggr.aggr_time_limit *= 1000;
	if (mGenericAgg) {
		from_ipa_cfg[2].hdr.hdr_len = 4;
		from_ipa_cfg[2].hdr_ext.hdr_pad_to_alignment = 2;
	} else {
		from_ipa_cfg[2].hdr.hdr_len = 1;
	}

	prepare_channel_struct(&from_ipa_channels[2],
			header.from_ipa_channels_num++,
			IPA_CLIENT_TEST_CONS,
			(void *)&from_ipa_cfg[2],
			sizeof(from_ipa_cfg[2]),
			false);
	from_ipa_array[2] = &from_ipa_channels[2];

	memset(&from_ipa_cfg[3], 0, sizeof(from_ipa_cfg[3]));
	from_ipa_cfg[3].aggr.aggr_en = IPA_ENABLE_AGGR;
	from_ipa_cfg[3].aggr.aggr = IPA_MBIM_16;
	from_ipa_cfg[3].aggr.aggr_byte_limit = 0;
	from_ipa_cfg[3].aggr.aggr_time_limit = 0;
	if (mGenericAgg) {
		from_ipa_cfg[3].hdr.hdr_len = 4;
		from_ipa_cfg[3].hdr_ext.hdr_pad_to_alignment = 2;
	} else {
		from_ipa_cfg[3].hdr.hdr_len = 1;
	}

	prepare_channel_struct(&from_ipa_channels[3],
			header.from_ipa_channels_num++,
			IPA_CLIENT_TEST4_CONS,
			(void *)&from_ipa_cfg[3],
			sizeof(from_ipa_cfg[3]),
			false);
	from_ipa_array[3] = &from_ipa_channels[3];

	/* To ipa configurations - 2 pipes */
	memset(&to_ipa_cfg[0], 0, sizeof(to_ipa_cfg[0]));
	prepare_channel_struct(&to_ipa_channels[0],
			header.to_ipa_channels_num++,
			IPA_CLIENT_TEST_PROD,
			(void *)&to_ipa_cfg[0],
			sizeof(to_ipa_cfg[0]));
	to_ipa_array[0] = &to_ipa_channels[0];

	memset(&to_ipa_cfg[1], 0, sizeof(to_ipa_cfg[1]));
	to_ipa_cfg[1].aggr.aggr_en = IPA_ENABLE_DEAGGR;
	to_ipa_cfg[1].aggr.aggr = IPA_MBIM_16;
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

bool MBIMAggregationTestFixtureConf11::Setup()
{
	bool bRetVal = true;

	//Set the configuration to support USB->IPA and IPA->USB pipes.
	if (SetupKernelModule() != true) {
		LOG_MSG_ERROR("fail to configure kernel module!\n");
		return false;
	}

	//Initialize the pipe for all the tests - this will open the inode which represents the pipe.
	bRetVal &= m_IpaToUsbPipeAgg.Init();
	bRetVal &= m_UsbToIpaPipe.Init();
	bRetVal &= m_IpaToUsbPipe.Init();
	bRetVal &= m_UsbToIpaPipeDeagg.Init();
	bRetVal &= m_IpaToUsbPipeAggTime.Init();
	bRetVal &= m_IpaToUsbPipeAgg0Limits.Init();

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

	return bRetVal;
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationTestFixtureConf11::Teardown()
{
	//The Destroy method will close the inode.
	m_IpaToUsbPipeAgg.Destroy();
	m_UsbToIpaPipe.Destroy();
	m_IpaToUsbPipe.Destroy();
	m_UsbToIpaPipeDeagg.Destroy();
	m_IpaToUsbPipeAggTime.Destroy();
	m_IpaToUsbPipeAgg0Limits.Destroy();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationTestFixtureConf11::Run()
{
	LOG_MSG_STACK("Entering Function");

	// Add the relevant filtering rules
	if (!AddRules()) {
		LOG_MSG_ERROR("Failed adding filtering rules.");
		return false;
	}
	if (!TestLogic()) {
		LOG_MSG_ERROR("Test failed, Input and expected output mismatch.");
		return false;
	}

	LOG_MSG_STACK("Leaving Function (Returning True)");
	return true;
} // Run()

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationTestFixtureConf11::AddRules1HeaderAggregation() {
	m_eIP = IPA_IP_v4;
	const char aBypass[20] = "Bypass1";
	uint32_t nTableHdl;
	bool bRetVal = true;
	IPAFilteringTable cFilterTable0;
	IPAFilteringTable cFilterTable1;
	struct ipa_flt_rule_add sFilterRuleEntry;
	struct ipa_ioc_get_hdr sGetHeader;
	uint8_t aHeadertoAdd[4];
	int hdrSize;

	if (mGenericAgg) {
		hdrSize = 4;
		aHeadertoAdd[0]= 0x49;
		aHeadertoAdd[1] = 0x50;
		aHeadertoAdd[2] = 0x53;
		aHeadertoAdd[3] = 0x00;
	} else {
		hdrSize = 1;
		aHeadertoAdd[0] = 0x00;
	}

	LOG_MSG_STACK("Entering Function");
	memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));
	memset(&sGetHeader, 0, sizeof(sGetHeader));

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
	// Adding Header No1.
	strlcpy(pHeaderDescriptor->hdr[0].name, "StreamId0", sizeof(pHeaderDescriptor->hdr[0].name)); // Header's Name
	memcpy(pHeaderDescriptor->hdr[0].hdr, aHeadertoAdd, hdrSize); //Header's Data
	pHeaderDescriptor->hdr[0].hdr_len = hdrSize;
	pHeaderDescriptor->hdr[0].hdr_hdl    = -1; //Return Value
	pHeaderDescriptor->hdr[0].is_partial = false;
	pHeaderDescriptor->hdr[0].status     = -1; // Return Parameter

	strlcpy(sGetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(sGetHeader.name));


	if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
	{
		LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
		bRetVal = false;
		goto bail;
	}

	if (!m_HeaderInsertion.GetHeaderHandle(&sGetHeader))
	{
		LOG_MSG_ERROR(" Failed");
		bRetVal = false;
		goto bail;
	}
	LOG_MSG_DEBUG("Received Header Handle = 0x%x", sGetHeader.hdl);


	if (!CreateBypassRoutingTable(&m_Routing, m_eIP, aBypass, IPA_CLIENT_TEST2_CONS,
			sGetHeader.hdl,&nTableHdl)) {
		LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
		bRetVal = false;
		goto bail;
	}


	LOG_MSG_INFO("Creation of bypass routing table completed successfully");

	// Creating Filtering Rules
	cFilterTable0.Init(m_eIP,IPA_CLIENT_TEST_PROD, false, 1);
	LOG_MSG_INFO("Creation of filtering table for IPA_CLIENT_TEST_PROD completed successfully");

	// Configuring Filtering Rule No.1
	cFilterTable0.GeneratePresetRule(1,sFilterRuleEntry);
	sFilterRuleEntry.at_rear = true;
	sFilterRuleEntry.flt_rule_hdl=-1; // return Value
	sFilterRuleEntry.status = -1; // return value
	sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
	sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl; //put here the handle corresponding to Routing Rule 1
	sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
	if (
			((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
			!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
			)
	{
		LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
		bRetVal = false;
		goto bail;
	} else
	{
		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable0.ReadRuleFromTable(0)->status);
	}

	memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));

	// Creating Filtering  Rule for De-aggregation PROD
	cFilterTable1.Init(m_eIP,IPA_CLIENT_TEST2_PROD, false, 1);
	LOG_MSG_INFO("Creation of filtering table for IPA_CLIENT_TEST2_PROD completed successfully");

	// Configuring Filtering Rule No.1
	cFilterTable1.GeneratePresetRule(1,sFilterRuleEntry);
	sFilterRuleEntry.at_rear = true;
	sFilterRuleEntry.flt_rule_hdl=-1; // return Value
	sFilterRuleEntry.status = -1; // return value
	sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
	sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl; //put here the handle corresponding to Routing Rule 1
	sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
	if (
			((uint8_t)-1 == cFilterTable1.AddRuleToTable(sFilterRuleEntry)) ||
			!m_Filtering.AddFilteringRule(cFilterTable1.GetFilteringTable())
			)
	{
		LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
		bRetVal = false;
		goto bail;
	} else
	{
		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable1.ReadRuleFromTable(0)->status);
	}

bail:
	Free(pHeaderDescriptor);
	LOG_MSG_STACK(
			"Leaving Function (Returning %s)", bRetVal?"True":"False");
	return bRetVal;
} // AddRules()

bool MBIMAggregationTestFixtureConf11::AddRules1HeaderAggregation(bool bAggForceClose) {
	m_eIP = IPA_IP_v4;
	const char aBypass[20] = "Bypass1";
	uint32_t nTableHdl;
	bool bRetVal = true;
	IPAFilteringTable_v2 cFilterTable0;
	IPAFilteringTable_v2 cFilterTable1;
	struct ipa_flt_rule_add_v2 sFilterRuleEntry;
	struct ipa_ioc_get_hdr sGetHeader;
	uint8_t aHeadertoAdd[4];
	int hdrSize;

	if (mGenericAgg) {
		hdrSize = 4;
		aHeadertoAdd[0]= 0x49;
		aHeadertoAdd[1] = 0x50;
		aHeadertoAdd[2] = 0x53;
		aHeadertoAdd[3] = 0x00;
	} else {
		hdrSize = 1;
		aHeadertoAdd[0] = 0x00;
	}

	LOG_MSG_STACK("Entering Function");
	memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));
	memset(&sGetHeader, 0, sizeof(sGetHeader));

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
	// Adding Header No1.
	strlcpy(pHeaderDescriptor->hdr[0].name, "StreamId0", sizeof(pHeaderDescriptor->hdr[0].name)); // Header's Name
	memcpy(pHeaderDescriptor->hdr[0].hdr, aHeadertoAdd, hdrSize); //Header's Data
	pHeaderDescriptor->hdr[0].hdr_len = hdrSize;
	pHeaderDescriptor->hdr[0].hdr_hdl    = -1; //Return Value
	pHeaderDescriptor->hdr[0].is_partial = false;
	pHeaderDescriptor->hdr[0].status     = -1; // Return Parameter

	strlcpy(sGetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(sGetHeader.name));


	if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
	{
		LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
		bRetVal = false;
		goto bail;
	}

	if (!m_HeaderInsertion.GetHeaderHandle(&sGetHeader))
	{
		LOG_MSG_ERROR(" Failed");
		bRetVal = false;
		goto bail;
	}
	LOG_MSG_DEBUG("Received Header Handle = 0x%x", sGetHeader.hdl);


	if (!CreateBypassRoutingTable_v2(&m_Routing, m_eIP, aBypass, IPA_CLIENT_TEST2_CONS,
			sGetHeader.hdl, &nTableHdl, 0)) {
		LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
		bRetVal = false;
		goto bail;
	}


	LOG_MSG_INFO("Creation of bypass routing table completed successfully");

	// Creating Filtering Rules
	cFilterTable0.Init(m_eIP,IPA_CLIENT_TEST_PROD, false, 1);
	LOG_MSG_INFO("Creation of filtering table for IPA_CLIENT_TEST_PROD completed successfully");

	// Configuring Filtering Rule No.1
	cFilterTable0.GeneratePresetRule(1,sFilterRuleEntry);
	sFilterRuleEntry.at_rear = true;
	sFilterRuleEntry.flt_rule_hdl=-1; // return Value
	sFilterRuleEntry.status = -1; // return value
	sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
	sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl; //put here the handle corresponding to Routing Rule 1
	sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
	sFilterRuleEntry.rule.close_aggr_irq_mod = bAggForceClose;
	if (
			((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
			!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
			)
	{
		LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
		bRetVal = false;
		goto bail;
	} else
	{
		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable0.ReadRuleFromTable(0)->status);
	}

	memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));

	// Creating Filtering  Rule for De-aggregation PROD
	cFilterTable1.Init(m_eIP,IPA_CLIENT_TEST2_PROD, false, 1);
	LOG_MSG_INFO("Creation of filtering table for IPA_CLIENT_TEST2_PROD completed successfully");

	// Configuring Filtering Rule No.1
	cFilterTable1.GeneratePresetRule(1,sFilterRuleEntry);
	sFilterRuleEntry.at_rear = true;
	sFilterRuleEntry.flt_rule_hdl=-1; // return Value
	sFilterRuleEntry.status = -1; // return value
	sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
	sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl; //put here the handle corresponding to Routing Rule 1
	sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
	sFilterRuleEntry.rule.close_aggr_irq_mod = bAggForceClose;
	if (
			((uint8_t)-1 == cFilterTable1.AddRuleToTable(sFilterRuleEntry)) ||
			!m_Filtering.AddFilteringRule(cFilterTable1.GetFilteringTable())
			)
	{
		LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
		bRetVal = false;
		goto bail;
	} else
	{
		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable1.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable1.ReadRuleFromTable(0)->status);
	}

bail:
	Free(pHeaderDescriptor);
	LOG_MSG_STACK(
			"Leaving Function (Returning %s)", bRetVal?"True":"False");
	return bRetVal;
} // AddRules()


/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationTestFixtureConf11::AddRulesDeaggregation() {
	m_eIP = IPA_IP_v4;
	const char aBypass[20] = "Bypass1";
	uint32_t nTableHdl;
	bool bRetVal = true;
	IPAFilteringTable cFilterTable0;
	struct ipa_flt_rule_add sFilterRuleEntry;
	struct ipa_ioc_get_hdr sGetHeader;
	uint8_t aHeadertoAdd[4];
	int hdrSize;

	if (mGenericAgg) {
		hdrSize = 4;
		aHeadertoAdd[0] = 0x49;
		aHeadertoAdd[1] = 0x50;
		aHeadertoAdd[2] = 0x53;
		aHeadertoAdd[3] = 0x00;
	}
	else {
		hdrSize = 1;
		aHeadertoAdd[0] = 0x00;
	}

	LOG_MSG_STACK("Entering Function");
	memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));
	memset(&sGetHeader, 0, sizeof(sGetHeader));

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
	// Adding Header No1.
	strlcpy(pHeaderDescriptor->hdr[0].name, "StreamId0", sizeof(pHeaderDescriptor->hdr[0].name)); // Header's Name
	memcpy(pHeaderDescriptor->hdr[0].hdr, (void*)&aHeadertoAdd,
			hdrSize); //Header's Data
	pHeaderDescriptor->hdr[0].hdr_len    = hdrSize;
	pHeaderDescriptor->hdr[0].hdr_hdl    = -1; //Return Value
	pHeaderDescriptor->hdr[0].is_partial = false;
	pHeaderDescriptor->hdr[0].status     = -1; // Return Parameter

	strlcpy(sGetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(sGetHeader.name));


	if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
	{
		LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
		bRetVal = false;
		goto bail;
	}

	if (!m_HeaderInsertion.GetHeaderHandle(&sGetHeader))
	{
		LOG_MSG_ERROR(" Failed");
		bRetVal = false;
		goto bail;
	}
	LOG_MSG_DEBUG("Received Header Handle = 0x%x", sGetHeader.hdl);


	if (!CreateBypassRoutingTable(&m_Routing, m_eIP, aBypass, IPA_CLIENT_TEST3_CONS,
			sGetHeader.hdl,&nTableHdl)) {
		LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
		bRetVal = false;
		goto bail;
	}


	LOG_MSG_INFO("Creation of bypass routing table completed successfully");

	// Creating Filtering Rules
	cFilterTable0.Init(m_eIP,IPA_CLIENT_TEST2_PROD, false, 1);
	LOG_MSG_INFO("Creation of filtering table completed successfully");

	// Configuring Filtering Rule No.1
	cFilterTable0.GeneratePresetRule(1,sFilterRuleEntry);
	sFilterRuleEntry.at_rear = true;
	sFilterRuleEntry.flt_rule_hdl=-1; // return Value
	sFilterRuleEntry.status = -1; // return value
	sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
	sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl; //put here the handle corresponding to Routing Rule 1
	sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
	if (
			((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
			!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
			)
	{
		LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
		bRetVal = false;
		goto bail;
	} else
	{
		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable0.ReadRuleFromTable(0)->status);
	}

bail:
	Free(pHeaderDescriptor);
	LOG_MSG_STACK(
			"Leaving Function (Returning %s)", bRetVal?"True":"False");
	return bRetVal;
} // AddRules()

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationTestFixtureConf11::AddRules1HeaderAggregationTime() {
	m_eIP = IPA_IP_v4;
	const char aBypass[20] = "Bypass1";
	uint32_t nTableHdl;
	bool bRetVal = true;
	IPAFilteringTable cFilterTable0;
	struct ipa_flt_rule_add sFilterRuleEntry;
	struct ipa_ioc_get_hdr sGetHeader;
	uint8_t aHeadertoAdd[4];
	int hdrSize;

	if (mGenericAgg) {
		hdrSize = 4;
		aHeadertoAdd[0] = 0x49;
		aHeadertoAdd[1] = 0x50;
		aHeadertoAdd[2] = 0x53;
		aHeadertoAdd[3] = 0x00;
	}
	else {
		hdrSize = 1;
		aHeadertoAdd[0] = 0x00;
	}

	LOG_MSG_STACK("Entering Function");
	memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));
	memset(&sGetHeader, 0, sizeof(sGetHeader));

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
	// Adding Header No1.
	strlcpy(pHeaderDescriptor->hdr[0].name, "StreamId0", sizeof(pHeaderDescriptor->hdr[0].name)); // Header's Name
	memcpy(pHeaderDescriptor->hdr[0].hdr, (void*)&aHeadertoAdd,
			hdrSize); //Header's Data
	pHeaderDescriptor->hdr[0].hdr_len = hdrSize;
	pHeaderDescriptor->hdr[0].hdr_hdl    = -1; //Return Value
	pHeaderDescriptor->hdr[0].is_partial = false;
	pHeaderDescriptor->hdr[0].status     = -1; // Return Parameter

	strlcpy(sGetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(sGetHeader.name));


	if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
	{
		LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
		bRetVal = false;
		goto bail;
	}

	if (!m_HeaderInsertion.GetHeaderHandle(&sGetHeader))
	{
		LOG_MSG_ERROR(" Failed");
		bRetVal = false;
		goto bail;
	}
	LOG_MSG_DEBUG("Received Header Handle = 0x%x", sGetHeader.hdl);


	if (!CreateBypassRoutingTable(&m_Routing, m_eIP, aBypass, IPA_CLIENT_TEST_CONS,
			sGetHeader.hdl,&nTableHdl)) {
		LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
		bRetVal = false;
		goto bail;
	}


	LOG_MSG_INFO("Creation of bypass routing table completed successfully");

	// Creating Filtering Rules
	cFilterTable0.Init(m_eIP,IPA_CLIENT_TEST_PROD, false, 1);
	LOG_MSG_INFO("Creation of filtering table completed successfully");

	// Configuring Filtering Rule No.1
	cFilterTable0.GeneratePresetRule(1,sFilterRuleEntry);
	sFilterRuleEntry.at_rear = true;
	sFilterRuleEntry.flt_rule_hdl=-1; // return Value
	sFilterRuleEntry.status = -1; // return value
	sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
	sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl; //put here the handle corresponding to Routing Rule 1
	sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
	if (
			((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
			!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
			)
	{
		LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
		bRetVal = false;
		goto bail;
	} else
	{
		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable0.ReadRuleFromTable(0)->status);
	}

bail:
	Free(pHeaderDescriptor);
	LOG_MSG_STACK(
			"Leaving Function (Returning %s)", bRetVal?"True":"False");
	return bRetVal;
} // AddRules()

/////////////////////////////////////////////////////////////////////////////////

bool MBIMAggregationTestFixtureConf11::AddRules1HeaderAggregation0Limits() {
	m_eIP = IPA_IP_v4;
	const char aBypass[20] = "Bypass1";
	uint32_t nTableHdl;
	bool bRetVal = true;
	IPAFilteringTable cFilterTable0;
	struct ipa_flt_rule_add sFilterRuleEntry;
	struct ipa_ioc_get_hdr sGetHeader;
	uint8_t aHeadertoAdd[4];
	int hdrSize;

	if (mGenericAgg) {
		hdrSize = 4;
		aHeadertoAdd[0] = 0x49;
		aHeadertoAdd[1] = 0x50;
		aHeadertoAdd[2] = 0x53;
		aHeadertoAdd[3] = 0x00;
	}
	else {
		hdrSize = 1;
		aHeadertoAdd[0] = 0x00;
	}

	LOG_MSG_STACK("Entering Function");
	memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));
	memset(&sGetHeader, 0, sizeof(sGetHeader));

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
	// Adding Header No1.
	strlcpy(pHeaderDescriptor->hdr[0].name, "StreamId0", sizeof(pHeaderDescriptor->hdr[0].name)); // Header's Name
	memcpy(pHeaderDescriptor->hdr[0].hdr, (void*)&aHeadertoAdd,
			hdrSize); //Header's Data
	pHeaderDescriptor->hdr[0].hdr_len = hdrSize;
	pHeaderDescriptor->hdr[0].hdr_hdl    = -1; //Return Value
	pHeaderDescriptor->hdr[0].is_partial = false;
	pHeaderDescriptor->hdr[0].status     = -1; // Return Parameter

	strlcpy(sGetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(sGetHeader.name));


	if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
	{
		LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
		bRetVal = false;
		goto bail;
	}

	if (!m_HeaderInsertion.GetHeaderHandle(&sGetHeader))
	{
		LOG_MSG_ERROR(" Failed");
		bRetVal = false;
		goto bail;
	}
	LOG_MSG_DEBUG("Received Header Handle = 0x%x", sGetHeader.hdl);


	if (!CreateBypassRoutingTable(&m_Routing, m_eIP, aBypass, IPA_CLIENT_TEST4_CONS,
			sGetHeader.hdl,&nTableHdl)) {
		LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
		bRetVal = false;
		goto bail;
	}


	LOG_MSG_INFO("Creation of bypass routing table completed successfully");

	// Creating Filtering Rules
	cFilterTable0.Init(m_eIP,IPA_CLIENT_TEST_PROD, false, 1);
	LOG_MSG_INFO("Creation of filtering table completed successfully");

	// Configuring Filtering Rule No.1
	cFilterTable0.GeneratePresetRule(1,sFilterRuleEntry);
	sFilterRuleEntry.at_rear = true;
	sFilterRuleEntry.flt_rule_hdl=-1; // return Value
	sFilterRuleEntry.status = -1; // return value
	sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
	sFilterRuleEntry.rule.rt_tbl_hdl=nTableHdl; //put here the handle corresponding to Routing Rule 1
	sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
	if (
			((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
			!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
			)
	{
		LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
		bRetVal = false;
		goto bail;
	} else
	{
		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable0.ReadRuleFromTable(0)->status);
	}

bail:
	Free(pHeaderDescriptor);
	LOG_MSG_STACK(
			"Leaving Function (Returning %s)", bRetVal?"True":"False");
	return bRetVal;
} // AddRules()

bool MBIMAggregationTestFixtureConf11::AddRulesAggDualFC(Pipe *input, Pipe *output1, Pipe *output2) {
	m_eIP = IPA_IP_v4;
	const char aBypass[2][20] = {"BypassTest2", "BypassTest4"};
	uint32_t nTableHdl[2];
	bool bRetVal = true;
	IPAFilteringTable_v2 cFilterTable0;
	struct ipa_flt_rule_add_v2 sFilterRuleEntry;
	struct ipa_ioc_get_hdr sGetHeader;
	uint8_t aHeadertoAdd[4];
	int hdrSize;

	if (mGenericAgg) {
		hdrSize = 4;
		aHeadertoAdd[0]= 0x49;
		aHeadertoAdd[1] = 0x50;
		aHeadertoAdd[2] = 0x53;
		aHeadertoAdd[3] = 0x00;
	} else {
		hdrSize = 1;
		aHeadertoAdd[0] = 0x00;
	}

	LOG_MSG_STACK("Entering Function");
	memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));
	memset(&sGetHeader, 0, sizeof(sGetHeader));

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
	// Adding Header No1.
	strlcpy(pHeaderDescriptor->hdr[0].name, "StreamId0", sizeof(pHeaderDescriptor->hdr[0].name)); // Header's Name
	memcpy(pHeaderDescriptor->hdr[0].hdr, aHeadertoAdd, hdrSize); //Header's Data
	pHeaderDescriptor->hdr[0].hdr_len = hdrSize;
	pHeaderDescriptor->hdr[0].hdr_hdl    = -1; //Return Value
	pHeaderDescriptor->hdr[0].is_partial = false;
	pHeaderDescriptor->hdr[0].status     = -1; // Return Parameter

	strlcpy(sGetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(sGetHeader.name));


	if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
	{
		LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
		bRetVal = false;
		goto bail;
	}

	if (!m_HeaderInsertion.GetHeaderHandle(&sGetHeader))
	{
		LOG_MSG_ERROR(" Failed");
		bRetVal = false;
		goto bail;
	}
	LOG_MSG_DEBUG("Received Header Handle = 0x%x", sGetHeader.hdl);

	if (!CreateBypassRoutingTable_v2(&m_Routing, m_eIP, aBypass[0],
								 output1->GetClientType(),
								 sGetHeader.hdl, &nTableHdl[0], 0)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			bRetVal = false;
			goto bail;
	}

	if (!CreateBypassRoutingTable_v2(&m_Routing, m_eIP, aBypass[1],
								 output2->GetClientType(),
								 sGetHeader.hdl, &nTableHdl[1], 0)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			bRetVal = false;
			goto bail;
	}

	LOG_MSG_INFO("Creation of bypass routing tables completed successfully");

	// Creating Filtering Rules
	cFilterTable0.Init(m_eIP,input->GetClientType(), false, 2);
	LOG_MSG_INFO("Creation of filtering table completed successfully");

	// Configuring Filtering Rule No.1
	cFilterTable0.GeneratePresetRule(1,sFilterRuleEntry);
	sFilterRuleEntry.at_rear = true;
	sFilterRuleEntry.flt_rule_hdl=-1; // return Value
	sFilterRuleEntry.status = -1; // return value
	sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
	sFilterRuleEntry.rule.rt_tbl_hdl = nTableHdl[0]; //put here the handle corresponding to Routing Rule 1
	sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
	sFilterRuleEntry.rule.close_aggr_irq_mod = 1;
	if (
		((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
		!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
		)
	{
		LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
		bRetVal = false;
		goto bail;
	} else
	{
		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable0.ReadRuleFromTable(0)->status);
	}

	// Configuring Filtering Rule No.2
	cFilterTable0.GeneratePresetRule(1,sFilterRuleEntry);
	sFilterRuleEntry.at_rear = true;
	sFilterRuleEntry.flt_rule_hdl=-1; // return Value
	sFilterRuleEntry.status = -1; // return value
	sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
	sFilterRuleEntry.rule.rt_tbl_hdl = nTableHdl[1]; //put here the handle corresponding to Routing Rule 1
	sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000002; // Filter DST_IP == 127.0.0.1.
	sFilterRuleEntry.rule.close_aggr_irq_mod = 0;
	if (
		((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
		!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
		)
	{
		LOG_MSG_ERROR ("Adding Rule (1) to Filtering block Failed.");
		bRetVal = false;
		goto bail;
	} else
	{
		LOG_MSG_DEBUG("flt rule hdl1=0x%x, status=0x%x\n", cFilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,cFilterTable0.ReadRuleFromTable(1)->status);
	}

bail:
	Free(pHeaderDescriptor);
	LOG_MSG_STACK(
		"Leaving Function (Returning %s)", bRetVal?"True":"False");
	return bRetVal;
} // AddRulesAggDualFC()

bool MBIMAggregationTestFixtureConf11::AddRulesAggDualFcRoutingBased(Pipe *input, Pipe *output1, Pipe *output2) {
	m_eIP = IPA_IP_v4;
	const char aBypass[2][20] = {"BypassTest2", "BypassTest4"};
	uint32_t nTableHdl[2];
	bool bRetVal = true;
	IPAFilteringTable_v2 cFilterTable0;
	struct ipa_flt_rule_add_v2 sFilterRuleEntry;
	struct ipa_ioc_get_hdr sGetHeader;
	uint8_t aHeadertoAdd[4];
	int hdrSize;

	if (mGenericAgg) {
		hdrSize = 4;
		aHeadertoAdd[0]= 0x49;
		aHeadertoAdd[1] = 0x50;
		aHeadertoAdd[2] = 0x53;
		aHeadertoAdd[3] = 0x00;
	} else {
		hdrSize = 1;
		aHeadertoAdd[0] = 0x00;
	}

	LOG_MSG_STACK("Entering Function");
	memset(&sFilterRuleEntry, 0, sizeof(sFilterRuleEntry));
	memset(&sGetHeader, 0, sizeof(sGetHeader));

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
	// Adding Header No1.
	strlcpy(pHeaderDescriptor->hdr[0].name, "StreamId0", sizeof(pHeaderDescriptor->hdr[0].name)); // Header's Name
	memcpy(pHeaderDescriptor->hdr[0].hdr, aHeadertoAdd, hdrSize); //Header's Data
	pHeaderDescriptor->hdr[0].hdr_len = hdrSize;
	pHeaderDescriptor->hdr[0].hdr_hdl    = -1; //Return Value
	pHeaderDescriptor->hdr[0].is_partial = false;
	pHeaderDescriptor->hdr[0].status     = -1; // Return Parameter

	strlcpy(sGetHeader.name, pHeaderDescriptor->hdr[0].name, sizeof(sGetHeader.name));


	if (!m_HeaderInsertion.AddHeader(pHeaderDescriptor))
	{
		LOG_MSG_ERROR("m_HeaderInsertion.AddHeader(pHeaderDescriptor) Failed.");
		bRetVal = false;
		goto bail;
	}

	if (!m_HeaderInsertion.GetHeaderHandle(&sGetHeader))
	{
		LOG_MSG_ERROR(" Failed");
		bRetVal = false;
		goto bail;
	}
	LOG_MSG_DEBUG("Received Header Handle = 0x%x", sGetHeader.hdl);

	if (!CreateBypassRoutingTable_v2(&m_Routing, m_eIP, aBypass[0],
								 output1->GetClientType(),
								 sGetHeader.hdl, &nTableHdl[0], 1)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			bRetVal = false;
			goto bail;
	}

	if (!CreateBypassRoutingTable_v2(&m_Routing, m_eIP, aBypass[1],
								 output2->GetClientType(),
								 sGetHeader.hdl, &nTableHdl[1], 0)) {
			LOG_MSG_ERROR("CreateBypassRoutingTable Failed\n");
			bRetVal = false;
			goto bail;
	}

	LOG_MSG_INFO("Creation of bypass routing tables completed successfully");

	// Creating Filtering Rules
	cFilterTable0.Init(m_eIP,input->GetClientType(), false, 2);
	LOG_MSG_INFO("Creation of filtering table completed successfully");

	// Configuring Filtering Rule No.1
	cFilterTable0.GeneratePresetRule(1,sFilterRuleEntry);
	sFilterRuleEntry.at_rear = true;
	sFilterRuleEntry.flt_rule_hdl=-1; // return Value
	sFilterRuleEntry.status = -1; // return value
	sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
	sFilterRuleEntry.rule.rt_tbl_hdl = nTableHdl[0]; //put here the handle corresponding to Routing Rule 1
	sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000001; // Filter DST_IP == 127.0.0.1.
	sFilterRuleEntry.rule.close_aggr_irq_mod = 0;
	if (
		((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
		!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
		)
	{
		LOG_MSG_ERROR ("Adding Rule (0) to Filtering block Failed.");
		bRetVal = false;
		goto bail;
	} else
	{
		LOG_MSG_DEBUG("flt rule hdl0=0x%x, status=0x%x\n", cFilterTable0.ReadRuleFromTable(0)->flt_rule_hdl,cFilterTable0.ReadRuleFromTable(0)->status);
	}

	// Configuring Filtering Rule No.2
	cFilterTable0.GeneratePresetRule(1,sFilterRuleEntry);
	sFilterRuleEntry.at_rear = true;
	sFilterRuleEntry.flt_rule_hdl=-1; // return Value
	sFilterRuleEntry.status = -1; // return value
	sFilterRuleEntry.rule.action=IPA_PASS_TO_ROUTING;
	sFilterRuleEntry.rule.rt_tbl_hdl = nTableHdl[1]; //put here the handle corresponding to Routing Rule 1
	sFilterRuleEntry.rule.attrib.attrib_mask = IPA_FLT_DST_ADDR; // Destination IP Based Filtering
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr_mask = 0xFF0000FF; // Mask
	sFilterRuleEntry.rule.attrib.u.v4.dst_addr = 0x7F000002; // Filter DST_IP == 127.0.0.1.
	sFilterRuleEntry.rule.close_aggr_irq_mod = 0;
	if (
		((uint8_t)-1 == cFilterTable0.AddRuleToTable(sFilterRuleEntry)) ||
		!m_Filtering.AddFilteringRule(cFilterTable0.GetFilteringTable())
		)
	{
		LOG_MSG_ERROR ("Adding Rule (1) to Filtering block Failed.");
		bRetVal = false;
		goto bail;
	} else
	{
		LOG_MSG_DEBUG("flt rule hdl1=0x%x, status=0x%x\n", cFilterTable0.ReadRuleFromTable(1)->flt_rule_hdl,cFilterTable0.ReadRuleFromTable(1)->status);
	}

bail:
	Free(pHeaderDescriptor);
	LOG_MSG_STACK(
		"Leaving Function (Returning %s)", bRetVal?"True":"False");
	return bRetVal;
} // AddRulesAggDualFC()
