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

/*----------------------------------------------------------------------------*/
/* Test00: Header insertion scenario of [RNDIS][ETH_II][IP] ->                */
/* [WLAN][ETH_II][IP]                                                         */
/*----------------------------------------------------------------------------*/
class IpaHdrProcCtxTest00 : public IpaHdrProcCtxTestFixture
{
public:
	IpaHdrProcCtxTest00()
	{
		m_name = "IpaHdrProcCtxTest00";
		m_description =
			"Processing Context test 00 - \
			of [RNDIS][ETH_II][IP] -> [WLAN][ETH_II][IP] \
			1. Generate and commit all headers for all tests. \
			2. Generate and commit all processing context rules \
			for all tests.\
			3. Generate and commit routing table 0. \
			The table contains 1 \"bypass\" rule. \
			All data goes to output pipe TEST2. \
			Routing rule will use processing context ETH2_2_WLAN_ETH2 \
			4. Generate and commit 1 filtering rule. \
			All traffic goes to routing table 0";
		m_pCurrentProducer = &m_rndisEth2Producer;
		m_currProducerClient = IPA_CLIENT_TEST_PROD;
		m_pCurrentConsumer = &m_defaultConsumer;
		m_currConsumerPipeNum = IPA_CLIENT_TEST2_CONS;
		m_procCtxHandleId = PROC_CTX_HANDLE_ID_ETH2_2_WLAN_ETH2;
		m_minIPAHwType = IPA_HW_v2_5;
		m_maxIPAHwType = IPA_HW_MAX;
		m_runInRegression = false;
		Register(*this);
	}

	virtual bool LoadPackets(enum ipa_ip_type ip)
	{
		if (!RNDISAggregationHelper::LoadRNDISPacket(
			ip,
			m_sendBuffer1,
			m_sendSize1))
		{
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}
		printf ("Loaded %zu Bytes to Packet 1\n",m_sendSize1);

		add_buff(m_sendBuffer1+RNDIS_HDR_SIZE, ETH_HLEN, 2);
		print_buff(m_sendBuffer1, m_sendSize1);

		return true;
	}

	virtual bool GenerateExpectedPackets()
	{
		m_expectedBufferSize1 = WLAN_ETH2_HDR_SIZE +
			IP4_PACKET_SIZE;

		// copy WLAN header to the beginning of the buffer
		memcpy(m_expectedBuffer1, WLAN_ETH2_HDR, WLAN_HDR_SIZE);

		// copy ETH+IP packet right after WLAN header
		memcpy(m_expectedBuffer1 + WLAN_HDR_SIZE,
			m_sendBuffer1 + RNDIS_HDR_SIZE,
			ETH_HLEN + IP4_PACKET_SIZE);

		return true;
	} // GenerateExpectedPackets()
};

/*----------------------------------------------------------------------------*/
/* Test01: Header insertion scenario of [WLAN][ETH_II][IP] ->                 */
/* [RNDIS][ETH_II][IP]                                                        */
/*----------------------------------------------------------------------------*/
class IpaHdrProcCtxTest01 : public IpaHdrProcCtxTestFixture
{
public:
	IpaHdrProcCtxTest01()
	{
		m_name = "IpaHdrProcCtxTest01";
		m_description =
			"Processing context test 01 - \
			of [WLAN][ETH_II][IP] -> [RNDIS][ETH_II][IP] \
			1. Generate and commit all headers for all tests. \
			2. Generate and commit all processing context rules \
			for all tests.\
			3. Generate and commit routing table 0. \
			The table contains 1 \"bypass\" rule. \
			All data goes to output pipe TEST3. \
			Routing rule will use processing context ETH2_2_RNDIS_ETH2 \
			4. Generate and commit 1 filtering rule. \
			All traffic goes to routing table 0";
		m_pCurrentProducer = &m_wlanEth2producer;
		m_currProducerClient = IPA_CLIENT_TEST2_PROD;
		m_pCurrentConsumer = &m_rndisEth2Consumer;
		m_currConsumerPipeNum = IPA_CLIENT_TEST3_CONS;
		m_procCtxHandleId = PROC_CTX_HANDLE_ID_ETH2_2_RNDIS_ETH2;
		m_minIPAHwType = IPA_HW_v2_5;
		m_maxIPAHwType = IPA_HW_MAX;
		m_runInRegression = false;
		Register(*this);
	}

	virtual bool LoadPackets(enum ipa_ip_type ip)
	{
		// load WLAN ETH2 IP4 packet of size 1kB + 1 byte
		// This size will trigger RNDIS aggregation later
		m_sendSize1 =
			RNDISAggregationHelper::RNDIS_AGGREGATION_BYTE_LIMIT + 1;

		if (!WlanHelper::LoadWlanEth2IP4PacketByLength(
			m_sendBuffer1,
			m_BUFF_MAX_SIZE,
			m_sendSize1,
			0x01))
			return false;

		printf ("Loaded %zu Bytes to Packet 1\n",m_sendSize1);

		add_buff(m_sendBuffer1+WLAN_HDR_SIZE, ETH_HLEN, 3);

		print_buff(m_sendBuffer1, m_sendSize1);

		return true;
	}

	virtual bool GenerateExpectedPackets()
	{
		size_t len = 0;
		size_t eth2PacketSize = m_sendSize1 - WLAN_HDR_SIZE;

		m_expectedBufferSize1 =	eth2PacketSize + RNDIS_HDR_SIZE;

		// copy RNDIS header
		if (!RNDISAggregationHelper::LoadRNDISHeader(
			m_expectedBuffer1,
			m_BUFF_MAX_SIZE,
			m_expectedBufferSize1,
			&len))
		{
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}

		// copy ETH2 packet after RNDIS header
		memcpy(m_expectedBuffer1 + len,
			m_sendBuffer1 + WLAN_HDR_SIZE,
			eth2PacketSize);

		return true;
	} // GenerateExpectedPackets()
};

/*----------------------------------------------------------------------------*/
/* Test02: Header insertion scenario of [WLAN][ETH_II][IP] ->                 */
/* [WLAN'][ETH_II][IP]                                                        */
/*----------------------------------------------------------------------------*/
class IpaHdrProcCtxTest02 : public IpaHdrProcCtxTestFixture
{
public:
	IpaHdrProcCtxTest02()
	{
		m_name = "IpaHdrProcCtxTest02";
		m_description =
			"Processing context test 02 - \
			of [WLAN][ETH_II][IP] -> [WLAN'][ETH_II][IP] \
			1. Generate and commit all headers for all tests. \
			2. Generate and commit all processing context rules \
			for all tests.\
			3. Generate and commit routing table 0. \
			The table contains 1 \"bypass\" rule. \
			All data goes to output pipe TEST2. \
			Routing rule will use processing context ETH2_2_WLAN_ETH2 \
			4. Generate and commit 1 filtering rule. \
			All traffic goes to routing table 0";
		m_pCurrentProducer = &m_wlanEth2producer;
		m_currProducerClient = IPA_CLIENT_TEST2_PROD;
		m_pCurrentConsumer = &m_defaultConsumer;
		m_currConsumerPipeNum = IPA_CLIENT_TEST2_CONS;
		m_procCtxHandleId = PROC_CTX_HANDLE_ID_ETH2_2_WLAN_ETH2;
		m_minIPAHwType = IPA_HW_v2_5;
		m_maxIPAHwType = IPA_HW_MAX;
		m_runInRegression = false;
		Register(*this);
	}

	virtual bool LoadPackets(enum ipa_ip_type ip)
	{
		// load WLAN ETH2 IP4 packet of size 1kB
		// This size will trigger RNDIS aggregation later
		if (!WlanHelper::LoadWlanEth2IP4Packet(
			m_sendBuffer1,
			m_BUFF_MAX_SIZE,
			&m_sendSize1))
			return false;

		printf ("Loaded %zu Bytes to Packet 1\n",m_sendSize1);

		add_buff(m_sendBuffer1+WLAN_HDR_SIZE, ETH_HLEN, 5);

		return true;
	}

	virtual bool GenerateExpectedPackets()
	{
		m_expectedBufferSize1 = m_sendSize1;
		memcpy(m_expectedBuffer1, m_sendBuffer1, m_expectedBufferSize1);
		memcpy(m_expectedBuffer1, WLAN_ETH2_HDR, WLAN_HDR_SIZE);

		return true;
	} // GenerateExpectedPackets()
};

/*----------------------------------------------------------------------------*/
/* Test03: Header insertion scenario of [WLAN][ETH_II][IP] ->                 */
/* [RNDIS][ETH_II][IP] with RNDIS aggregation                                 */
/*----------------------------------------------------------------------------*/
class IpaHdrProcCtxTest03 : public IpaHdrProcCtxTestFixture
{
public:
	IpaHdrProcCtxTest03()
	{
		m_name = "IpaHdrProcCtxTest03";
		m_description =
			"Processing Context test 03 - \
			of [RNDIS][ETH_II][IP] -> [WLAN][ETH_II][IP] \
			with RNDIS aggregation \
			1. Generate and commit all headers for all tests. \
			2. Generate and commit all processing context rules \
			for all tests.\
			3. Generate and commit routing table 0. \
			The table contains 1 \"bypass\" rule. \
			All data goes to output pipe TEST3. \
			Routing rule will use processing context ETH2_2_RNDIS_ETH2 \
			4. Generate and commit 1 filtering rule. \
			All traffic goes to routing table 0";
		m_pCurrentProducer = &m_wlanEth2producer;
		m_currProducerClient = IPA_CLIENT_TEST2_PROD;
		m_pCurrentConsumer = &m_rndisEth2Consumer;
		m_currConsumerPipeNum = IPA_CLIENT_TEST3_CONS;
		m_procCtxHandleId = PROC_CTX_HANDLE_ID_ETH2_2_RNDIS_ETH2;
		m_minIPAHwType = IPA_HW_v2_5;
		m_maxIPAHwType = IPA_HW_MAX;
		m_runInRegression = false;
		Register(*this);
	}

	virtual bool LoadPackets(enum ipa_ip_type ip)
	{
		// choose this size so that 2 such buffers would be aggregated
		m_sendSize1 = RNDISAggregationHelper::
			RNDIS_AGGREGATION_BYTE_LIMIT /	2 + 200;

		if (!WlanHelper::LoadWlanEth2IP4PacketByLength(
			m_sendBuffer1,
			m_BUFF_MAX_SIZE,
			m_sendSize1,
			1))
			return false;

		printf ("Loaded %zu Bytes to Packet 1\n", m_sendSize1);

		add_buff(m_sendBuffer1+WLAN_HDR_SIZE, ETH_HLEN, 7);

		// choose this size so that 2 such buffers would be aggregated
		m_sendSize2 = RNDISAggregationHelper::
			RNDIS_AGGREGATION_BYTE_LIMIT /	2 + 200;

		if (!WlanHelper::LoadWlanEth2IP4PacketByLength(
			m_sendBuffer2,
			m_BUFF_MAX_SIZE,
			m_sendSize2,
			2))
			return false;

		printf ("Loaded %zu Bytes to Packet 2\n", m_sendSize2);

		add_buff(m_sendBuffer2+WLAN_HDR_SIZE, ETH_HLEN, 11);

		return true;
	}

	virtual bool SendPackets()
	{
		bool isSuccess = false;

		// Send packet 1
		isSuccess = m_pCurrentProducer->SendData(
			m_sendBuffer1,
			m_sendSize1);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR(
				"SendData Buffer 1 failed on producer %d\n", m_currProducerClient);
			return false;
		}

		// Send packet 2
		isSuccess = m_pCurrentProducer->SendData(
			m_sendBuffer2,
			m_sendSize2);
		if (false == isSuccess)
		{
			LOG_MSG_ERROR(
				"SendData Buffer 2 failed on producer %d\n", m_currProducerClient);
			return false;
		}

		return true;
	}

	virtual bool GenerateExpectedPackets()
	{
		size_t len = 0;
		size_t eth2PacketSize1 = m_sendSize1 - WLAN_HDR_SIZE;
		size_t rndisPacketSize1 = eth2PacketSize1 + RNDIS_HDR_SIZE;
		size_t eth2PacketSize2 = m_sendSize2 - WLAN_HDR_SIZE;
		size_t rndisPacketSize2 = eth2PacketSize2 + RNDIS_HDR_SIZE;
		Byte *currBuffLocation = NULL;

		m_expectedBufferSize1 = rndisPacketSize1 + rndisPacketSize2;

		currBuffLocation = m_expectedBuffer1;

		// copy first RNDIS header
		if (!RNDISAggregationHelper::LoadRNDISHeader(
			currBuffLocation,
			m_BUFF_MAX_SIZE,
			rndisPacketSize1,
			&len))
			return false;

		// copy ETH2 packet 1 after RNDIS header
		currBuffLocation += len;
		memcpy(currBuffLocation,
			m_sendBuffer1 + WLAN_HDR_SIZE,
			eth2PacketSize1);

		// copy second RNDIS header
		currBuffLocation += eth2PacketSize1;
		if (!RNDISAggregationHelper::LoadRNDISHeader(
			currBuffLocation,
			m_BUFF_MAX_SIZE - rndisPacketSize1,
			rndisPacketSize2,
			&len))
			return false;

		// copy ETH2 packet 2 after RNDIS header
		currBuffLocation += len;
		memcpy(currBuffLocation,
			m_sendBuffer2 + WLAN_HDR_SIZE,
			eth2PacketSize2);

		return true;
	} // GenerateExpectedPackets()
};


/*----------------------------------------------------------------------------*/
/* Test04: Header insertion scenario when adding total header sizes > 2048    */
/*----------------------------------------------------------------------------*/
class IpaHdrProcCtxTest04 : public IpaHdrProcCtxTestFixture
{
public:
	IpaHdrProcCtxTest04()
	{
		m_name = "IpaHdrProcCtxTest04";
		m_description =
			"Processing context test 04 - \
			Header insertion scenario when adding \
			total header sizes > 2048 \
			1. Generate and commit all headers for all tests. \
			2. Generate and commit all processing context rules \
			for all tests.\
			3. Generate and commit routing table 0. \
			The table contains 1 \"bypass\" rule. \
			All data goes to output pipe TEST2. \
			Routing rule will use header WLAN_ETH2 \
			4. Generate and commit 1 filtering rule. \
			All traffic goes to routing table 0";
		m_pCurrentProducer = &m_wlanEth2producer;
		m_currProducerClient = IPA_CLIENT_TEST2_PROD;
		m_pCurrentConsumer = &m_defaultConsumer;
		m_currConsumerPipeNum = IPA_CLIENT_TEST2_CONS;
		m_minIPAHwType = IPA_HW_v2_5;
		m_maxIPAHwType = IPA_HW_MAX;
		m_runInRegression = false;
		Register(*this);
	}

	virtual void AddAllHeaders()
	{
		int cnt = 0;
		int allHeadersSize = 0;

		while (allHeadersSize <= m_ALL_HEADER_SIZE_LIMIT)
		{
			AddHeader(HEADER_HANDLE_ID_ETH2);
			/* header bins are power of 2 */
			allHeadersSize += ETH_HLEN + 2;
			cnt++;
		}

		AddHeader(HEADER_HANDLE_ID_WLAN_ETH2);
	}

	virtual bool AddRules()
	{
		printf("Entering %s, %s()\n",__FUNCTION__, __FILE__);

		AddAllHeaders();

		AddRtBypassRule(m_headerHandles[HEADER_HANDLE_ID_WLAN_ETH2], 0);

		AddFltBypassRule();

		printf("Leaving %s, %s()\n",__FUNCTION__, __FILE__);
		return true;

	}// AddRules()

	virtual bool LoadPackets(enum ipa_ip_type ip)
	{
		// load WLAN ETH2 IP4 packet of size 1kB
		// This size will trigger RNDIS aggregation later
		if (!WlanHelper::LoadWlanEth2IP4Packet(
			m_sendBuffer1,
			m_BUFF_MAX_SIZE,
			&m_sendSize1))
			return false;

		printf ("Loaded %zu Bytes to Packet 1\n",m_sendSize1);

		add_buff(m_sendBuffer1+WLAN_HDR_SIZE, ETH_HLEN, 13);

		return true;
	}

	virtual bool GenerateExpectedPackets()
	{
		m_expectedBufferSize1 = m_sendSize1;
		memcpy(m_expectedBuffer1, m_sendBuffer1, m_expectedBufferSize1);
		memcpy(m_expectedBuffer1, WLAN_ETH2_HDR, WLAN_ETH2_HDR_SIZE);

		return true;
	} // GenerateExpectedPackets()
};

/*----------------------------------------------------------------------------*/
/* Test05: Header insertion scenario of [ETH_II_802_1Q][IP] ->                */
/* [ETH_II_802_1Q][IP]                                                        */
/*----------------------------------------------------------------------------*/
class IpaHdrProcCtxTest05 : public IpaHdrProcCtxTestFixture
{
public:
	IpaHdrProcCtxTest05()
	{
		m_name = "IpaHdrProcCtxTest05";
		m_description =
			"Processing Context test 05 - \
			of [ETH_II_802_1Q][IP] -> [ETH_II_802_1Q][IP] \
			1. Generate and commit all headers for all tests. \
			2. Generate and commit all processing context rules \
			for all tests.\
			3. Generate and commit routing table 0. \
			The table contains 1 \"bypass\" rule. \
			All data goes to output pipe TEST2. \
			Routing rule will use processing context 802_1Q_2_802_1Q \
			4. Generate and commit 1 filtering rule. \
			All traffic goes to routing table 0";
		/*
		 * NOTE: we use the wlan + ETH header prod pipe since the header
		 * length shall be equal to 8021Q ETH_II length
		 */
		m_pCurrentProducer = &m_wlanEth2producer;
		m_currProducerClient = IPA_CLIENT_TEST2_PROD;
		m_pCurrentConsumer = &m_defaultConsumer;
		m_currConsumerPipeNum = IPA_CLIENT_TEST2_CONS;
		m_procCtxHandleId = PROC_CTX_HANDLE_ID_802_1Q_2_802_1Q;
		m_minIPAHwType = IPA_HW_v4_0;
		m_maxIPAHwType = IPA_HW_MAX;
		m_runInRegression = false;
		Register(*this);
	}

	virtual bool LoadPackets(enum ipa_ip_type ip)
	{
		if (!LoadDefault802_1Q(ip,
			m_sendBuffer1,
			m_sendSize1)) {
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}
		printf("Loaded %zu Bytes to Packet 1\n", m_sendSize1);

		// modify the MAC addresses only
		add_buff(m_sendBuffer1, 12, 14);

		//change vlan ID to 9
		m_sendBuffer1[15] = 0x9;

		print_buff(m_sendBuffer1, m_sendSize1);

		return true;
	}

	virtual bool GenerateExpectedPackets()
	{
		m_expectedBufferSize1 = m_sendSize1;

		// we actually expect the same packet to come out (but after uCP)
		memcpy(m_expectedBuffer1, m_sendBuffer1, m_expectedBufferSize1);

		return true;
	} // GenerateExpectedPackets()
};

/*----------------------------------------------------------------------------*/
/* Test06: Header insertion scenario of [ETH_II][IP] ->                       */
/* [ETH_II_802_1Q][IP]                                                        */
/*----------------------------------------------------------------------------*/
class IpaHdrProcCtxTest06 : public IpaHdrProcCtxTestFixture
{
public:
	IpaHdrProcCtxTest06()
	{
		m_name = "IpaHdrProcCtxTest06";
		m_description =
			"Processing Context test 06 - \
			of [ETH_II][IP] -> [ETH_II_802_1Q][IP] \
			1. Generate and commit all headers for all tests. \
			2. Generate and commit all processing context rules \
			for all tests.\
			3. Generate and commit routing table 0. \
			The table contains 1 \"bypass\" rule. \
			All data goes to output pipe TEST2. \
			Routing rule will use processing context ETH2_2_802_1Q \
			4. Generate and commit 1 filtering rule. \
			All traffic goes to routing table 0";

		m_pCurrentProducer = &m_eth2Producer;
		m_currProducerClient = IPA_CLIENT_TEST3_PROD;
		m_pCurrentConsumer = &m_defaultConsumer;
		m_currConsumerPipeNum = IPA_CLIENT_TEST2_CONS;
		m_procCtxHandleId = PROC_CTX_HANDLE_ID_ETH2_2_802_1Q;
		m_minIPAHwType = IPA_HW_v4_0;
		m_maxIPAHwType = IPA_HW_MAX;
		m_runInRegression = false;
		Register(*this);
	}

	virtual bool LoadPackets(enum ipa_ip_type ip)
	{
		if (!Eth2Helper::LoadEth2IP4Packet(
			m_sendBuffer1,
			m_BUFF_MAX_SIZE,
			&m_sendSize1)) {
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}

		printf("Loaded %zu Bytes to Packet 1\n", m_sendSize1);

		// modify the MAC addresses only
		add_buff(m_sendBuffer1, 12, 15);

		print_buff(m_sendBuffer1, m_sendSize1);

		return true;
	}

	virtual bool GenerateExpectedPackets()
	{
		size_t len;

		m_expectedBufferSize1 = ETH8021Q_HEADER_LEN +
			IP4_PACKET_SIZE;
		// copy the VLAN header to expected buffer
		memcpy(m_expectedBuffer1, ETH2_8021Q_HDR, ETH8021Q_HEADER_LEN);

		// fill src and dst mac and ethertype
		memcpy(m_expectedBuffer1, m_sendBuffer1, 2 * ETH_ALEN);
		memcpy(m_expectedBuffer1 + ETH8021Q_ETH_TYPE_OFFSET,
			m_sendBuffer1 + ETH2_ETH_TYPE_OFFSET, ETH2_ETH_TYPE_LEN);

		len = m_BUFF_MAX_SIZE - ETH8021Q_HEADER_LEN;
		if (!LoadDefaultPacket(IPA_IP_v4,
			m_expectedBuffer1 + ETH8021Q_HEADER_LEN,
			len)) {
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}

		return true;
	} // GenerateExpectedPackets()
};

/*----------------------------------------------------------------------------*/
/* Test07: Header insertion scenario of [ETH_II_802_1Q][IP] ->                */
/* [ETH_II][IP]                                                               */
/*----------------------------------------------------------------------------*/
class IpaHdrProcCtxTest07 : public IpaHdrProcCtxTestFixture
{
public:
	IpaHdrProcCtxTest07()
	{
		m_name = "IpaHdrProcCtxTest07";
		m_description =
			"Processing Context test 07 - \
			of [ETH_II_802_1Q][IP] -> [ETH_II][IP] \
			1. Generate and commit all headers for all tests. \
			2. Generate and commit all processing context rules \
			for all tests.\
			3. Generate and commit routing table 0. \
			The table contains 1 \"bypass\" rule. \
			All data goes to output pipe TEST2. \
			Routing rule will use processing context 802_1Q_2_ETH2 \
			4. Generate and commit 1 filtering rule. \
			All traffic goes to routing table 0";

		m_pCurrentProducer = &m_wlanEth2producer;
		m_currProducerClient = IPA_CLIENT_TEST2_PROD;
		m_pCurrentConsumer = &m_defaultConsumer;
		m_currConsumerPipeNum = IPA_CLIENT_TEST2_CONS;
		m_procCtxHandleId = PROC_CTX_HANDLE_ID_802_1Q_2_ETH2;
		m_minIPAHwType = IPA_HW_v4_0;
		m_maxIPAHwType = IPA_HW_MAX;
		m_runInRegression = false;
		Register(*this);
	}

	virtual bool LoadPackets(enum ipa_ip_type ip)
	{
		if (!LoadDefault802_1Q(ip,
			m_sendBuffer1,
			m_sendSize1)) {
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}

		printf("Loaded %zu Bytes to Packet 1\n", m_sendSize1);

		// modify the MAC addresses only
		add_buff(m_sendBuffer1, ETH8021Q_METADATA_OFFSET, 16);

		print_buff(m_sendBuffer1, m_sendSize1);

		return true;
	}

	virtual bool GenerateExpectedPackets()
	{
		size_t len;

		m_expectedBufferSize1 = m_sendSize1 - ETH8021Q_8021Q_TAG_LEN;

		// copy the ETH2 header to expected buffer
		memcpy(m_expectedBuffer1, ETH2_HDR, ETH_HLEN);

		// fill src and dst mac and ethertype
		memcpy(m_expectedBuffer1, m_sendBuffer1, 2 * ETH_ALEN);
		memcpy(m_expectedBuffer1 + ETH2_ETH_TYPE_OFFSET,
			m_sendBuffer1 + ETH8021Q_ETH_TYPE_OFFSET,
			ETH2_ETH_TYPE_LEN);

		len = m_BUFF_MAX_SIZE - ETH_HLEN;
		if (!LoadDefaultPacket(IPA_IP_v4,
			m_expectedBuffer1 + ETH_HLEN,
			len)) {
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}

		return true;
	} // GenerateExpectedPackets()
};

/*----------------------------------------------------------------------------*/
/* Test08: Header insertion scenario of [ETH_II][IP] ->                       */
/* [ETH_II][IP] with generic ucp command                                      */
/*----------------------------------------------------------------------------*/
class IpaHdrProcCtxTest08 : public IpaHdrProcCtxTestFixture
{
public:
	IpaHdrProcCtxTest08()
	{
		m_name = "IpaHdrProcCtxTest08";
		m_description =
			"Processing Context test 08 - \
			of [ETH_II][IP] -> [ETH_II][IP] with generic ucp \
			1. Generate and commit all headers for all tests. \
			2. Generate and commit all processing context rules \
			for all tests.\
			3. Generate and commit routing table 0. \
			The table contains 1 \"bypass\" rule. \
			All data goes to output pipe TEST2. \
			Routing rule will use processing context ETH2_2_ETH2_EX \
			4. Generate and commit 1 filtering rule. \
			All traffic goes to routing table 0";

		m_pCurrentProducer = &m_eth2Producer;
		m_currProducerClient = IPA_CLIENT_TEST3_PROD;
		m_pCurrentConsumer = &m_defaultConsumer;
		m_currConsumerPipeNum = IPA_CLIENT_TEST2_CONS;
		m_procCtxHandleId = PROC_CTX_HANDLE_ID_ETH2_ETH2_2_ETH2_EX;
		m_minIPAHwType = IPA_HW_v4_0;
		m_maxIPAHwType = IPA_HW_MAX;
		m_runInRegression = false;
		Register(*this);
	}

	virtual bool LoadPackets(enum ipa_ip_type ip)
	{
		if (!Eth2Helper::LoadEth2IP4Packet(
			m_sendBuffer1,
			m_BUFF_MAX_SIZE,
			&m_sendSize1)) {
			LOG_MSG_ERROR("Failed default Packet\n");
			return false;
		}

		printf("Loaded %zu Bytes to Packet 1\n", m_sendSize1);

		// modify the MAC addresses only
		add_buff(m_sendBuffer1, 12, 17);

		print_buff(m_sendBuffer1, m_sendSize1);

		return true;
	}

	virtual bool GenerateExpectedPackets()
	{
		m_expectedBufferSize1 = m_sendSize1;

		// we actually expect the same packet to come out (but after uCP)
		memcpy(m_expectedBuffer1, m_sendBuffer1, m_expectedBufferSize1);

		return true;
	} // GenerateExpectedPackets()
};

static IpaHdrProcCtxTest00 ipaHdrProcCtxTest00;
static IpaHdrProcCtxTest01 ipaHdrProcCtxTest01;
static IpaHdrProcCtxTest02 ipaHdrProcCtxTest02;
static IpaHdrProcCtxTest03 ipaHdrProcCtxTest03;
static IpaHdrProcCtxTest04 ipaHdrProcCtxTest04;
static IpaHdrProcCtxTest05 ipaHdrProcCtxTest05;
static IpaHdrProcCtxTest06 ipaHdrProcCtxTest06;
static IpaHdrProcCtxTest07 ipaHdrProcCtxTest07;
static IpaHdrProcCtxTest08 ipaHdrProcCtxTest08;
