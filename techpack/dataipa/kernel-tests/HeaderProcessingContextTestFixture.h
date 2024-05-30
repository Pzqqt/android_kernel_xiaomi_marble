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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <cstring> // for memcpy
#include <linux/msm_ipa.h>
#include "hton.h" // for htonl
#include "InterfaceAbstraction.h"
#include "Constants.h"
#include "Logger.h"
#include "TestsUtils.h"
#include "Filtering.h"
#include "HeaderInsertion.h"
#include "RoutingDriverWrapper.h"
#include "IPAFilteringTable.h"

/*
Processing context test design:

1.	1 Producer with RNDIS de-aggregation and ETH2 header removal.
2.	1 Producer with WLAN and ETH2 header removal.
3.	1 default consumer.
4.	1 RNDIS aggregation consumer.
5.	1 FLT rule  accept all - points to 1 RT rule.
6.	1 RT rule  accept all - points to specific test relevant proc_ctx.
7.	All tests add all proc_ctx  (for all tests).
8.	Proc_ctx to be added: 1 for each test  3 altogether.
	Proc_ctx of test 01 and 03 are the same.
9.	Each test will send 1 packet and check that the packet is good
	except test 03.
10.	Test 03  the same as Test 01  but will send multiple packets
	and expect 1 (RNDIS aggregation test).

List of tests:
00.	Header insertion scenario of [RNDIS][ETH_II][IP] -> [WLAN][ETH_II][IP]
01.	Header insertion scenario of [WLAN][ETH_II][IP] -> [RNDIS][ETH_II][IP]
02.	Header insertion scenario of [WLAN][ETH_II][IP] -> [WLAN][ETH_II][IP]
03.	Header insertion of [WLAN][ETH_II][IP] -> [RNDIS][ETH_II][IP]
	with RNDIS aggregation.
04.	Header insertion scenario when adding total header sizes > 2048
05.	Header insertion scenario of [ETH_II][IP] -> [WLAN][ETH_II][IP]
06.	Header insertion scenario of [WLAN][ETH_II][IP] -> [ETH_II][IP
07.	Header insertion scenario of [ETH_II][IP] -> [ETH_II][IP]
08.	Header insertion scenario of [RNDIS][ETH_II][IP] -> [WLAN][802.3][IP]
09.	Header insertion scenario of [WLAN][802.3][IP] -> [RNDIS][ETH_II][IP]
10.	Header insertion scenario of [ETH_II][IP] -> [WLAN][802.3][IP]
11.	Header insertion scenario of [WLAN][802.3][IP] -> [WLAN][802.3][IP]
*/
class IpaHdrProcCtxTestFixture : public TestBase
{
public:
	enum HeaderHandleId
	{
		HEADER_HANDLE_ID_WLAN_ETH2,
		HEADER_HANDLE_ID_RNDIS_ETH2,
		HEADER_HANDLE_ID_ETH2,
		HEADER_HANDLE_ID_WLAN_802_3,
		HEADER_HANDLE_ID_VLAN_802_1Q,
		HEADER_HANDLE_ID_EoGRE_V4,
		HEADER_HANDLE_ID_EoGRE_V6,
		HEADER_HANDLE_ID_MAX
	};

	enum ProcCtxHandleId
	{
		PROC_CTX_HANDLE_ID_ETH2_2_WLAN_ETH2,
		PROC_CTX_HANDLE_ID_ETH2_2_RNDIS_ETH2,
		PROC_CTX_HANDLE_ID_ETH2_ETH2_2_ETH2,
		PROC_CTX_HANDLE_ID_WLAN_ETH2_2_802_3,
		PROC_CTX_HANDLE_ID_RNDIS_802_3_2_ETH2,
		PROC_CTX_HANDLE_ID_WLAN_802_3_2_ETH2,
		PROC_CTX_HANDLE_ID_802_1Q_2_802_1Q,
		PROC_CTX_HANDLE_ID_ETH2_2_802_1Q,
		PROC_CTX_HANDLE_ID_802_1Q_2_ETH2,
		PROC_CTX_HANDLE_ID_ETH2_ETH2_2_ETH2_EX,
		PROC_CTX_HANDLE_ID_EoGRE_HDR_ADD,
		PROC_CTX_HANDLE_ID_EoGRE_HDR_REMOVE,
		PROC_CTX_HANDLE_ID_MAX
	};

	// header table consist of 9 bits and 4B units -> 2048
	static const int m_ALL_HEADER_SIZE_LIMIT = 2048;

	static const size_t m_BUFF_MAX_SIZE =
		2 * RNDISAggregationHelper::RNDIS_AGGREGATION_BYTE_LIMIT;

	// [WLAN][ETH2] header
	static const Byte WLAN_ETH2_HDR[WLAN_ETH2_HDR_SIZE];

	// [ETH2] header
	static const Byte ETH2_HDR[ETH_HLEN];

	// [ETH2_802_1Q] vlan header
	static const Byte ETH2_8021Q_HDR[ETH8021Q_HEADER_LEN];

	// [WLAN][802.3] header
	static const Byte WLAN_802_3_HDR[WLAN_802_3_HDR_SIZE];

	// EoGRE V4 header
	static const Byte EoGRE_V4_HDR[EoGRE_V4_HDR_LEN];

	// EoGRE V6 header
	static const Byte EoGRE_V6_HDR[EoGRE_V6_HDR_LEN];

	static Filtering m_filtering;
	static RoutingDriverWrapper m_routing;
	static HeaderInsertion m_headerInsertion;

	// For each header type the handle is saved
	// to be used by the processing context
	uint32_t m_headerHandles[HEADER_HANDLE_ID_MAX];

	// For each prco_ctx type the handle is saved
	// to be used by the routing rule
	uint32_t m_procCtxHHandles[PROC_CTX_HANDLE_ID_MAX];

	// proc_ctx handle ID
	ProcCtxHandleId m_procCtxHandleId;

	HeaderHandleId m_headerHandleId;

	// routing table handle
	uint32_t m_routingTableHdl;

	// Pipe with RNDIS and ETH2 header removal
	InterfaceAbstraction m_rndisEth2Producer;

	// Pipe with WLAN and ETH2 header removal
	InterfaceAbstraction m_wlanEth2producer;

	// Pipe with ETH2 header removal
	InterfaceAbstraction m_eth2Producer;

	// TODO: Pipe with WLAN and 802.3 header removal
	InterfaceAbstraction m_wlan802_3producer;

	// Pointer to current producer pipe used in the test
	InterfaceAbstraction *m_pCurrentProducer;
	ipa_client_type m_currProducerClient;

	// Pipe of the WLAN ETH2 consumer
	InterfaceAbstraction m_defaultConsumer;

	// Pipe of the RNDIS ETH2 consumer
	InterfaceAbstraction m_rndisEth2Consumer;

	// Pointer to current consumer pipe used in the test
	InterfaceAbstraction *m_pCurrentConsumer;
	ipa_client_type m_currConsumerPipeNum;

	// First input packet
	Byte m_sendBuffer1[m_BUFF_MAX_SIZE];
	size_t m_sendSize1;

	// Second input packet
	Byte m_sendBuffer2[m_BUFF_MAX_SIZE];
	size_t m_sendSize2;

	// First expected packet
	Byte m_expectedBuffer1[m_BUFF_MAX_SIZE];
	size_t m_expectedBufferSize1;

	enum ipa_ip_type m_IpaIPType;

	union ipa_ip_params m_ip_addrs;

	IpaHdrProcCtxTestFixture();

	virtual bool Setup();
	virtual bool Teardown();
	virtual void AddAllHeaders();

	// Insert a single header
	virtual void AddHeader(HeaderHandleId handleId);
	virtual void AddAllProcCtx();

	// Insert a single proc_ctx
	virtual void AddProcCtx(ProcCtxHandleId handleId);
	virtual void AddRtBypassRule(uint32_t hdrHdl, uint32_t procCtxHdl);
	virtual void AddFltBypassRule();
	virtual bool LoadPackets(enum ipa_ip_type ip) = 0;

	virtual bool ReceivePacketsAndCompare();
	virtual bool ReceivePacketsAndCompare(
		Byte*  receivedBuffer,
		size_t receivedBufferSize );

	// Create 1 IPv4 bypass routing entry and commits it
	virtual bool CreateIPv4BypassRoutingTable (
		const char *name,
		uint32_t hdrHdl,
		uint32_t procCtxHdl);

	// Add routing rule
	virtual bool AddRoutingRule(
		const char*          name,
		uint32_t             hdrHdl,
		uint32_t             procCtxHdl,
		ipa_ip_type          iptype,
		union ipa_ip_params  ip_addrs,
		enum ipa_client_type dst );

	// Add filtering rule
	virtual bool AddFilteringRule(
		ipa_ip_type          iptype,
		union ipa_ip_params  ip_addrs,
		enum ipa_client_type src,
		uint32_t             rt_tbl_hdl );

	virtual bool GenerateExpectedPackets() = 0;

	virtual bool AddRules();

	virtual bool SendPackets();

	virtual bool Run();

	~IpaHdrProcCtxTestFixture();

private:
};

