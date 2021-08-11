/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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
#include "UlsoTestFixture.h"
#include "HeaderInsertion.h"

#define ARRAY_SIZE(A) (sizeof(ArraySizeHelper(A)))

template <typename T, size_t N>
char (&ArraySizeHelper(T (&a)[N]))[N];

using std::cout;
using std::endl;
using std::string;

using I4 = IPv4Header;
using I6 = IPv6Header;
using UDPH = UdpHeader;
using TCPH = TcpHeader;

/* Packet modification function objects */
template<typename PacketType> class PacketModifier {public:virtual void operator()(PacketType& p) const = 0;};
template<typename PacketType> class NullPacketModifier: public PacketModifier<PacketType> {public:void operator()(PacketType& p) const override {}};
template<typename PacketType> class ZeroChecksumPacketModifier: public PacketModifier<PacketType>
	{public:void operator()(PacketType& p) const override {p.mQmapHeader.setmZeroChecksum(1);}};
template<typename PacketType> class OutOfBoundsPacketModifier: public PacketModifier<PacketType>
	{public:void operator()(PacketType& p) const override {p.mQmapHeader.setmIpIdCfg(0);p.setIpId(65530);}};

template<typename Transport, typename Internet, const size_t* SegmentSizesArr, size_t SegmentSizesArrSize, const float* SegmentsNumsArr,
	size_t SegmentsNumsArrSize, typename Modifier = NullPacketModifier<UlsoPacket<Transport, Internet>>>
class PacketsGeneratorClass {

protected:

	using PacketType = UlsoPacket<Transport, Internet>;

public:

	vector<PacketType> operator()(){
		vector<PacketType> outVec;
		Modifier m;
		for(size_t i=0; i<SegmentSizesArrSize; i++){
			for(size_t j=0; j<SegmentsNumsArrSize; j++){
				PacketType p(SegmentSizesArr[i], static_cast<size_t>(SegmentSizesArr[i] * SegmentsNumsArr[j]), true);
				m(p);
				outVec.emplace_back(p);
			}
		}
		return outVec;
	}
};

template<typename Transport, typename Internet, typename PacketsGenerator>
class UlsoTest: public UlsoTestFixture {

private:

	using PacketType = UlsoPacket<Transport, Internet>;

	bool singlePacketRun(PacketType& p){
		memset(m_sendBuf, 0, sizeof(m_sendBuf));
		size_t packetSize = p.asArray(m_sendBuf);
		size_t numSent = m_producer.SendData(m_sendBuf, packetSize);

		if(numSent == 0){
			return false;
		}
		vector<PacketType> segmentedPacketsVec = p.segment();
		for(auto& segmentedPacket: segmentedPacketsVec){
			memset(m_receiveBuf, 0, sizeof(m_receiveBuf));
			memset(m_segmentBuf, 0, sizeof(m_segmentBuf));
			packetSize = segmentedPacket.asArray(m_segmentBuf);
			size_t recievedBytes = m_consumer.ReceiveSingleDataChunk(m_receiveBuf, packetSize);
			if(packetSize != recievedBytes || memcmp(m_segmentBuf, m_receiveBuf, packetSize)){
				return fail(numSent, packetSize, recievedBytes);
			}
		}
		return clearPipe() == 0;
	}

public:

	UlsoTest(const char* name){
		m_name = name;
		string title = string("ULSO Test");
		string packetStructure = string("Structure: ") + string ("QMAP + Ethernet 2 + ")
			+ string(Internet().name()) + string(" ") + string(Transport().name());
		string testProcess = string(
			"1. Config IPA->APPS test pipe\n"
			"2. Generate a vector of ULSO packets\n"
			"3. For each packet in the packets vector\n"
			"	a. Send the packet over the APPS->IPA pipe using ipa_tx_dp\n"
			"	b. Segment the packet using the software simulation and store it in a segments vector\n"
			"	c. For each segment in the segments vector in order\n"
			"		# Receive a segment over IPA->APPS test pipe\n"
			"		# Compare the received segment to the software simulated segment\n"
			"4. Clear the IPA->USB pipe and verify there were no bytes left in the pipe");
		m_description = string(title + "\n" + packetStructure + "\n" + testProcess + "\n").c_str();
		m_minIPAHwType = IPA_HW_v5_0;
		Register(*this);
	}

	virtual bool Run() override {
		vector<PacketType> packetsVec = PacketsGenerator()();
		for(auto& p: packetsVec){
			if(!singlePacketRun(p)){
				cout << "Failed With the following packet:" << endl;
				cout << p << endl;
				return false;
			}
		}
		return true;
	}
};

template<typename Transport, typename Internet, typename PacketsGenerator>
class UlsoHPCTest: public UlsoTestFixture {

private:

	using PacketType = UlsoPacket<Transport, Internet>;
	static constexpr size_t rndisHdrLen {44};
	HeaderInsertion m_HeaderInsertion;
	uint8_t mRndisHeader[rndisHdrLen] = {0};

	bool fail(size_t sendSize=0, size_t totalSegmentsSize=0, size_t recievedBytes=0){
		printBuf(m_sendBuf, sendSize, "Sent:");
		printBuf(m_receiveBuf, recievedBytes, string("Rceived ")
				+ std::to_string(recievedBytes) + string(" Bytes:"));
		printBuf(m_segmentBuf, totalSegmentsSize, string("Expected to receive ")
			+ std::to_string(totalSegmentsSize) + string(" Bytes:"));
		clearPipe();
		return false;
	}

	bool singlePacketRun(PacketType& p){
		cout << p << endl;
		memset(m_sendBuf, 0, sizeof(m_sendBuf));
		size_t sendSize = p.asArray(m_sendBuf);
		if(!m_producer.SendData(m_sendBuf, sendSize)){
			return fail(sendSize);
		}
		vector<PacketType> segmentedPacketsVec = p.segment();
		memset(m_segmentBuf, 0, sizeof(m_segmentBuf));
		uint8_t *segmentBufPtr = m_segmentBuf;
		size_t totalSegmentsSize = 0;
		vector<size_t> comparisionIntervalsSizesVec;
		for(auto& segmentedPacket: segmentedPacketsVec){
			memcpy(segmentBufPtr, mRndisHeader, sizeof(mRndisHeader));
			segmentBufPtr += sizeof(mRndisHeader);
			comparisionIntervalsSizesVec.emplace_back(sizeof(mRndisHeader));
			totalSegmentsSize += sizeof(mRndisHeader);
			size_t n = segmentedPacket.asArray(segmentBufPtr);
			segmentBufPtr += n;
			totalSegmentsSize += n;
			comparisionIntervalsSizesVec.emplace_back(n);
		}
		memset(m_receiveBuf, 0, sizeof(m_receiveBuf));
		size_t recievedBytes = m_consumer.ReceiveSingleDataChunk(m_receiveBuf, totalSegmentsSize);
		if(totalSegmentsSize != recievedBytes){
			return fail(sendSize, totalSegmentsSize, recievedBytes);
		}
		segmentBufPtr = m_segmentBuf;
		uint8_t *recieveBufPtr = m_receiveBuf;
		while(!comparisionIntervalsSizesVec.empty()){
			size_t skipSize = comparisionIntervalsSizesVec.front();
			recieveBufPtr += skipSize;
			segmentBufPtr += skipSize;
			comparisionIntervalsSizesVec.erase(comparisionIntervalsSizesVec.begin());
			if(comparisionIntervalsSizesVec.empty()){
				return fail(sendSize, totalSegmentsSize, recievedBytes);
			}
			size_t compareSize = comparisionIntervalsSizesVec.front();
			if(memcmp(segmentBufPtr, recieveBufPtr, compareSize)){
				return fail(sendSize, totalSegmentsSize, recievedBytes);
			}
			segmentBufPtr += compareSize;
			recieveBufPtr += compareSize;
			comparisionIntervalsSizesVec.erase(comparisionIntervalsSizesVec.begin());
		}
		if(clearPipe()){
			return fail(sendSize, totalSegmentsSize, recievedBytes);
		}
		return true;
	}

protected:

	virtual void configFromEp(struct test_ipa_ep_cfg *ep_cfg){
		ep_cfg->hdr.hdr_len = ETH_HLEN + rndisHdrLen;
		ep_cfg->hdr.hdr_additional_const_len = ETH_HLEN;
		ep_cfg->hdr.hdr_ofst_pkt_size_valid = true;
		ep_cfg->hdr.hdr_ofst_pkt_size = 3 * sizeof(uint32_t);
		ep_cfg->hdr_ext.hdr_total_len_or_pad_offset = sizeof(uint32_t);
		ep_cfg->hdr_ext.hdr_total_len_or_pad = IPA_HDR_TOTAL_LEN;
		ep_cfg->hdr_ext.hdr_total_len_or_pad_valid = true;
		ep_cfg->hdr_ext.hdr_little_endian = true;
		ep_cfg->aggr.aggr_en = IPA_ENABLE_AGGR;
		ep_cfg->aggr.aggr = IPA_GENERIC;
		ep_cfg->aggr.aggr_byte_limit = 4;
		ep_cfg->aggr.aggr_time_limit = 1000;
	}

public:

	UlsoHPCTest(const char* name, const char* description){
		m_name = name;
		m_description = description;
		m_minIPAHwType = IPA_HW_v5_0;
		for(size_t i=0; i<rndisHdrLen; i++){
			mRndisHeader[i] = i;
		}
		Register(*this);
	}

	virtual bool Run() override {
		string headerName("rndis");
		if(!m_HeaderInsertion.addHeaderHpc(headerName, mRndisHeader, rndisHdrLen, false, IPA_CLIENT_TEST_CONS)){
			LOG_MSG_ERROR("!m_HeaderInsertion.addHeaderHpc(headerName, mRndisHeader, 44, false, true) Failed.");
			return false;
		}
		vector<PacketType> packetsVec = PacketsGenerator()();
		for(auto& p: packetsVec){
			if(!singlePacketRun(p)){
				return false;
			}
		}
		if(!m_HeaderInsertion.DeleteHeader(headerName)){
			LOG_MSG_ERROR("Delete rndis header failed");
			return false;
		}
		return true;
	}
};

/* Tests Macros */
#define PACKETS_GEN_MODIFY(T, I, a, b, m) PacketsGeneratorClass<T, I, a, ARRAY_SIZE(a), b, ARRAY_SIZE(b), m<UlsoPacket<T, I>>>
#define PACKETS_GEN(T, I, a, b) PACKETS_GEN_MODIFY(T, I, a, b, NullPacketModifier)//todo: change macro parameters to meaningfull names

////////////////////////////////////////////////////////////////////////////////
///////////            Simple Single Packet Tests                 //////////////
////////////////////////////////////////////////////////////////////////////////
/*
 * Send a single packet and compare the received segments to the software simulation
 */
constexpr size_t segmentSizes1[] = {20};
constexpr float segmentsNum1[] = {5};
static UlsoTest<UDPH, I4, PACKETS_GEN(UDPH, I4, segmentSizes1, segmentsNum1)> ulsoTest0 {"Single Packet: IPV4 UDP"};
static UlsoTest<TCPH, I4, PACKETS_GEN(TCPH, I4, segmentSizes1, segmentsNum1)> ulsoTest1 {"Single Packet: IPV4 TCP"};
static UlsoTest<UDPH, I6, PACKETS_GEN(UDPH, I6, segmentSizes1, segmentsNum1)> ulsoTest2 {"Single Packet: IPV6 UDP"};
static UlsoTest<TCPH, I6, PACKETS_GEN(TCPH, I6, segmentSizes1, segmentsNum1)> ulsoTest3 {"Single Packet: IPV6 TCP"};

////////////////////////////////////////////////////////////////////////////////
///////////         Segmentation & Non-Segmentation mix           //////////////
////////////////////////////////////////////////////////////////////////////////
/*
 * Send a sequence of [large, small, large, small, ...] packets and compare the received segments to the software simulation
 */
constexpr size_t segmentSizes2[] = {10, 50, 100, 500, 1460};
constexpr float segmentsNum2[] = {1, 4};
static UlsoTest<UDPH, I4, PACKETS_GEN(UDPH, I4, segmentSizes2, segmentsNum2)> ulsoTest10 {"Segmentation No Segmentation IPV4 UDP"};
static UlsoTest<TCPH, I4, PACKETS_GEN(TCPH, I4, segmentSizes2, segmentsNum2)> ulsoTest11 {"Segmentation No Segmentation IPV4 TCP"};
static UlsoTest<UDPH, I6, PACKETS_GEN(UDPH, I6, segmentSizes2, segmentsNum2)> ulsoTest12 {"Segmentation No Segmentation IPV6 UDP"};
static UlsoTest<TCPH, I6, PACKETS_GEN(TCPH, I6, segmentSizes2, segmentsNum2)> ulsoTest13 {"Segmentation No Segmentation IPV6 TCP"};

////////////////////////////////////////////////////////////////////////////////
////////////////////             Zero Checksum              ////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
 * Send a sequence of large packets with zero checksum=1 and compare the received segments to the software simulation
 */
constexpr size_t segmentSizes3[] = {10, 50, 100, 500, 1460};
constexpr float numSegments3[] = {4};
static UlsoTest<UDPH, I4, PACKETS_GEN_MODIFY(UDPH, I4, segmentSizes3, numSegments3, ZeroChecksumPacketModifier)>
	ulsoTest20 {"Zero Checksum IPV4 UDP"};
static UlsoTest<TCPH, I4, PACKETS_GEN_MODIFY(TCPH, I4, segmentSizes3, numSegments3, ZeroChecksumPacketModifier)>
	ulsoTest21 {"Zero Checksum IPV4 TCP"};
static UlsoTest<UDPH, I6, PACKETS_GEN_MODIFY(UDPH, I6, segmentSizes3, numSegments3, ZeroChecksumPacketModifier)>
	ulsoTest22 {"Zero Checksum IPV6 UDP"};
static UlsoTest<TCPH, I6, PACKETS_GEN_MODIFY(TCPH, I6, segmentSizes3, numSegments3, ZeroChecksumPacketModifier)>
	ulsoTest23 {"Zero Checksum IPV6 TCP"};

////////////////////////////////////////////////////////////////////////////////
/////////        Segment Size Greater Than Payload Size        /////////////////
////////////////////////////////////////////////////////////////////////////////
/*
 * Send a single packet with payload size and MSS matching an edge case and edge case and compare the received segments to the software simulation.
 * Edge cases:
 * 1. payload size < MSS ==> No segmentation
 * 2. payload size == MSS - epsilon ==> No segmentation
 * 3. payload size == MSS ==> Segmentation
 */
/* Segment Size = 100 Payload Size = 50 */
constexpr size_t segmentSizes4[] = {100};
constexpr float numSegments4[] = {0.5};
static UlsoTest<UDPH, I4, PACKETS_GEN(UDPH, I4, segmentSizes4, numSegments4)> ulsoTest30 {"Payload Smaller Than MSS IPV4 UDP"};
static UlsoTest<TCPH, I4, PACKETS_GEN(TCPH, I4, segmentSizes4, numSegments4)> ulsoTest31 {"Payload Smaller Than MSS IPV4 TCP"};
static UlsoTest<UDPH, I6, PACKETS_GEN(UDPH, I6, segmentSizes4, numSegments4)> ulsoTest32 {"Payload Smaller Than MSS IPV6 UDP"};
static UlsoTest<TCPH, I6, PACKETS_GEN(TCPH, I6, segmentSizes4, numSegments4)> ulsoTest33 {"Payload Smaller Than MSS IPV6 TCP"};

/* Segment Size = 100 Payload Size = 99 */
constexpr size_t segmentSizes5[] = {100};
constexpr float numSegments5[] = {0.99};
static UlsoTest<UDPH, I4, PACKETS_GEN(UDPH, I4, segmentSizes5, numSegments5)> ulsoTest40 {"Payload slightly Smaller Than MSS IPV4 UDP"};
static UlsoTest<TCPH, I4, PACKETS_GEN(TCPH, I4, segmentSizes5, numSegments5)> ulsoTest41 {"Payload slightly Smaller Than MSS IPV4 TCP"};
static UlsoTest<UDPH, I6, PACKETS_GEN(UDPH, I6, segmentSizes5, numSegments5)> ulsoTest42 {"Payload slightly Smaller Than MSS IPV6 UDP"};
static UlsoTest<TCPH, I6, PACKETS_GEN(TCPH, I6, segmentSizes5, numSegments5)> ulsoTest43 {"Payload slightly Smaller Than MSS IPV6 TCP"};

/* Segment Size = 20 Payload Size = 20 */
constexpr size_t segmentSizes6[] = {100};
constexpr float numSegments6[] = {1};
static UlsoTest<UDPH, I4, PACKETS_GEN(UDPH, I4, segmentSizes6, numSegments6)> ulsoTest50 {"Payload Equals MSS IPV4 UDP"};
static UlsoTest<TCPH, I4, PACKETS_GEN(TCPH, I4, segmentSizes6, numSegments6)> ulsoTest51 {"Payload Equals MSS IPV4 TCP"};
static UlsoTest<UDPH, I6, PACKETS_GEN(UDPH, I6, segmentSizes6, numSegments6)> ulsoTest52 {"Payload Equals MSS IPV6 UDP"};
static UlsoTest<TCPH, I6, PACKETS_GEN(TCPH, I6, segmentSizes6, numSegments6)> ulsoTest53 {"Payload Equals MSS IPV6 TCP"};

////////////////////////////////////////////////////////////////////////////////
//////////////             Valid Segment Sizes             /////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
 * Send a sequence of packets with different valid sizes and compare the received segments to the software simulation
 */
constexpr size_t segmentSizes7[] = {1460, 1220, 512, 1};
static UlsoTest<UDPH, I4, PACKETS_GEN(UDPH, I4, segmentSizes7, segmentsNum1)> ulsoTest60 {"Valid Segment Sizes IPV4 UDP"};
static UlsoTest<TCPH, I4, PACKETS_GEN(TCPH, I4, segmentSizes7, segmentsNum1)> ulsoTest61 {"Valid Segment Sizes IPV4 TCP"};
static UlsoTest<UDPH, I6, PACKETS_GEN(UDPH, I6, segmentSizes7, segmentsNum1)> ulsoTest62 {"Valid Segment Sizes IPV6 UDP"};
static UlsoTest<TCPH, I6, PACKETS_GEN(TCPH, I6, segmentSizes7, segmentsNum1)> ulsoTest63 {"Valid Segment Sizes IPV6 TCP"};

////////////////////////////////////////////////////////////////////////////////
////////////////             Big Segment Sizes             /////////////////////
////////////////////////////////////////////////////////////////////////////////
/*
 * Send a sequence of very large packets and compare the received segments to the software simulation
 */
constexpr size_t segmentSizes8[] = {2000, 3000, 4000, 5000, 6000, 10000};
static UlsoTest<UDPH, I4, PACKETS_GEN(UDPH, I4, segmentSizes8, segmentsNum1)> ulsoTest70 {"Big Segment Sizes IPV4 UDP"};
static UlsoTest<TCPH, I4, PACKETS_GEN(TCPH, I4, segmentSizes8, segmentsNum1)> ulsoTest71 {"Big Segment Sizes IPV4 TCP"};
static UlsoTest<UDPH, I6, PACKETS_GEN(UDPH, I6, segmentSizes8, segmentsNum1)> ulsoTest72 {"Big Segment Sizes IPV6 UDP"};
static UlsoTest<TCPH, I6, PACKETS_GEN(TCPH, I6, segmentSizes8, segmentsNum1)> ulsoTest73 {"Big Segment Sizes IPV6 TCP"};

////////////////////////////////////////////////////////////////////////////////
////////////////        IP ID wrapp around min/max bounds        ///////////////
////////////////////////////////////////////////////////////////////////////////
/*
 * Send a single packet such that:
 * IPID + #segments < MAX IPID
 * and compare the received segments to the software simulation
 */
constexpr size_t segmentSizes9[] = {2000};
constexpr float numSegments9[] = {10};
static UlsoTest<UDPH, I4, PACKETS_GEN_MODIFY(UDPH, I4, segmentSizes9, numSegments9, OutOfBoundsPacketModifier)> ulsoTest80 {"IPID CFG IPV4 UDP"};
static UlsoTest<TCPH, I4, PACKETS_GEN_MODIFY(TCPH, I4, segmentSizes9, numSegments9, OutOfBoundsPacketModifier)> ulsoTest81 {"IPID CFG IPV4 UDP"};
////////////////////////////////////////////////////////////////////////////////
////////////////        HPC RNDIS Header Insertion        //////////////////////
////////////////////////////////////////////////////////////////////////////////

static UlsoHPCTest<UDPH, I4, PACKETS_GEN(UDPH, I4, segmentSizes1, segmentsNum1)> Ipv4UdpHpcRndisTest {"Ipv4UdpHpcRndisTest", "IPv4 + UDP"};
static UlsoHPCTest<TCPH, I4, PACKETS_GEN(TCPH, I4, segmentSizes1, segmentsNum1)> Ipv4TcpHpcRndisTest {"Ipv4TcpHpcRndisTest", "IPv4 + TCP"};
static UlsoHPCTest<UDPH, I6, PACKETS_GEN(UDPH, I6, segmentSizes1, segmentsNum1)> Ipv6UdpHpcRndisTest {"Ipv6UdpHpcRndisTest", "IPv6 + UDP"};
static UlsoHPCTest<TCPH, I6, PACKETS_GEN(TCPH, I6, segmentSizes1, segmentsNum1)> Ipv6TcpHpcRndisTest {"Ipv6TcpHpcRndisTest", "IPv6 + TCP"};
