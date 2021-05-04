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

#define IPV4_DST_ADDR_OFFSET (16)
#define IPV4_SRC_PORT_OFFSET (20)
#define IPV4_DST_PORT_OFFSET (20+2)

#define DST_ADDR_LSB_OFFSET_IPV4 (19)
#define DST_ADDR_LSB_OFFSET_IPV6 (39)

class IpaFilteringEthernetBridgingTestFixture : public TestBase
{
public:
	virtual bool ModifyPackets() = 0;
	virtual bool AddRules() = 0;

	IpaFilteringEthernetBridgingTestFixture();

	bool Setup();

	bool Teardown();

	virtual bool LoadFiles(enum ipa_ip_type ip);

	bool ReceivePacketsAndCompare();

	// This function creates three IPv4 bypass routing entries and commits them.
	bool CreateThreeIPv4BypassRoutingTables (const char *bypass0, const char *bypass1,
		const char *bypass2);

	// This function creates three IPv6 bypass routing entries and commits them.
	bool CreateThreeIPv6BypassRoutingTables (const char *bypass0, const char *bypass1,
		const char *bypass2);

	bool Run();

	~IpaFilteringEthernetBridgingTestFixture();

	static const size_t m_BUFF_MAX_SIZE = 1024;
	static const uint8_t m_ETH2_DST_ADDR[ETH_ALEN];
	static const uint8_t m_ETH2_SRC_ADDR[ETH_ALEN];
	static const uint8_t m_MAC_ADDR_MASK_ALL[ETH_ALEN];
	static Filtering m_filtering;
	static RoutingDriverWrapper m_routing;

	InterfaceAbstraction m_producer;
	InterfaceAbstraction m_producer2; // Pipe with ETH2 header removal
	InterfaceAbstraction *m_pCurrentProducer;
	InterfaceAbstraction m_consumer;
	InterfaceAbstraction m_consumer2;
	InterfaceAbstraction m_defaultConsumer;

	Byte m_sendBuffer1[m_BUFF_MAX_SIZE];
	Byte m_sendBuffer2[m_BUFF_MAX_SIZE];
	Byte m_sendBuffer3[m_BUFF_MAX_SIZE];
	size_t m_sendSize1;
	size_t m_sendSize2;
	size_t m_sendSize3;
	enum ipa_ip_type m_IpaIPType;

private:
};

