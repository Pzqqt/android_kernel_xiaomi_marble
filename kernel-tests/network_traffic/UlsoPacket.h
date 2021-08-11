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
#ifndef NETWORK_TRAFFIC_ULSOPACKET_H
#define NETWORK_TRAFFIC_ULSOPACKET_H


#include <algorithm>
#include <utility>
#include <stdexcept>
#include "QmapHeader.h"
#include "UdpHeader.h"
#include "TcpHeader.h"
#include "IPv4Header.h"
#include "IPv6Header.h"
#include "packets.h"
#include "Ethernet2Header.h"

template <typename Transport=UdpHeader, typename Internet=IPv4Header>
class UlsoPacket {

    static_assert(std::is_base_of<TransportHeader, Transport>::value,
            "Transport is not derived from TransportHeader");
    static_assert(std::is_base_of<InternetHeader, Internet>::value,
            "Internet is not derived from InternetHeader");

private:

    static constexpr uint8_t qmapIPv4UdpPacketNoSeg[] = {
            //C=0,Next Header=1,PAD=b'63,MUXID=0,Packet length=30
            0X7f, 0x00, 0x00, 0x1e,//QMAP header
            //type=3,Next Header=0,IPIDCfg=1,Zero Checksum=1,Additional header size=0,segment size=2
            0X06, 0xc0, 0x00, 0x02,//QMAP header extension
            //IPv4 header
            0x45, 0x00, 0x00, 0x1e,//version=4,IHL=5,DSCP=0,ECN=0,length=30
            0x00, 0x00, 0x00, 0x00,//id=0,flags=0,fragment offset=0
            0xff, 0x11, 0x36, 0x03,//TTL=255,Protocol=17 (TCP),checksum=
            0xc0, 0xa8, 0x02, 0x13,//IPv4 SRC Addr 192.168.2.19
            0xc0, 0xa8, 0x02, 0x68,//IPv4 DST Addr 192.168.2.104
            //UDP header
            0x04, 0x57, 0x08, 0xae,//source port=1111, destination port=2222
            0x00, 0x0a, 0x00, 0x00,//length=10,checksum=
            //payload
            0x00, 0x01
    };

    /**
     * Resembles ULSO related endpoint configurations.
     */
    unsigned int mMinId {0};
    unsigned int mMaxId {65535};

    bool mIsSegmented {false};

public:

    static constexpr int maxSize {65536};//64 KB

    QmapHeader mQmapHeader;
    Ethernet2Header mEthernetHeader;
    Internet mInternetHeader;
    bool mEthernetHeaderValid {false};
    Transport mTransportHeader;
    vector<uint8_t> mPayload {};

    UlsoPacket(unsigned int segmentSize, unsigned int payloadSize, bool ethernetHeaderValid=true):
        mEthernetHeaderValid(ethernetHeaderValid){
        bool first = true;
        uint32_t seqNum = 0;

        mQmapHeader.setmSegmentSize(segmentSize);
        if(mEthernetHeaderValid){
            mEthernetHeader.setmEtherType(mInternetHeader.getEtherType());
        }
        mPayload = vector<uint8_t>(payloadSize);
        for(unsigned int i = 0; i < payloadSize; i++){
            mPayload[i] = i % UINT8_MAX;
        }
        mInternetHeader.adjust(mTransportHeader.size() + mPayload.size(), mTransportHeader.protocolNum());
        mQmapHeader.setmPacketLength(mInternetHeader.size() + mTransportHeader.size() + mPayload.size());
        adjustHeader(mTransportHeader, seqNum, first);
    }

    UlsoPacket(unsigned int segmentSize, uint8_t* payload, unsigned int payloadSize){
        bool first = true;
        uint32_t seqNum = 0;

        mQmapHeader.setmSegmentSize(segmentSize);
        mPayload  = vector<uint8_t>{payload, payload + payloadSize};
        mInternetHeader.adjust(mTransportHeader.size() + mPayload.size(), mTransportHeader.protocolNum());
        mQmapHeader.setmPacketLength(mInternetHeader.size() + mTransportHeader.size() + mPayload.size());
        adjustHeader(mTransportHeader, seqNum, first);
    }

    size_t size() const {
        return (!isSegmented() * mQmapHeader.size()) + (mEthernetHeaderValid * mEthernetHeader.size())
        + mInternetHeader.size() + mTransportHeader.size() + mPayload.size();
    }

    UlsoPacket(const QmapHeader& qmapHeader, const Internet& iPv4Header, const Transport& udpHeader,
               const vector<uint8_t>& payload) :
            mQmapHeader(qmapHeader),
            mInternetHeader(iPv4Header),
            mTransportHeader(udpHeader),
            mPayload(payload) {}

    explicit UlsoPacket(size_t bufLen, uint8_t *buf=const_cast<uint8_t *>(qmapIPv4UdpPacketNoSeg)) {
        size_t curIndex = 0;

        mQmapHeader = QmapHeader(buf + curIndex);
        curIndex += mQmapHeader.size();
        mInternetHeader = Internet(buf + curIndex);
        curIndex += mInternetHeader.size();
        mTransportHeader = Transport(buf + curIndex);
        curIndex += mTransportHeader.size();
        mPayload = vector<uint8_t>();
        while(curIndex < bufLen){
            mPayload.emplace_back(buf[curIndex]);
            curIndex++;
        }
    }

    UlsoPacket(){
        mQmapHeader.setmPacketLength(mInternetHeader.size() + mTransportHeader.size() + mPayload.size());
        mInternetHeader.adjust(mTransportHeader.size() + mPayload.size(), mTransportHeader.protocolNum());
        adjustHeader(mTransportHeader);
    }

    vector<bool> asVector() const {
        vector<bool> outVec;
        auto concatenateFunc = [](vector<bool>& v1, const vector<bool>& v2){
            v1.insert(v1.end(), v2.begin(), v2.end());
        };

        if(!mIsSegmented){
            concatenateFunc(outVec, mQmapHeader.asVector());
        }
        if(mEthernetHeaderValid){
            concatenateFunc(outVec, mEthernetHeader.asVector());
        }
        concatenateFunc(outVec, mInternetHeader.asVector());
        concatenateFunc(outVec, mTransportHeader.asVector());
        std::for_each(mPayload.cbegin(), mPayload.cend(), [&outVec](char c){
            for (int i = SIZE_OF_BITS(c) - 1; i >= 0; i--)
                outVec.emplace_back((c & ( 1 << i )) >> i); // NOLINT(hicpp-signed-bitwise)
        });
        return outVec;
    }

    uint8_t* asArray() const {
        vector<bool> vec = asVector();
        size_t resSize = vec.size() / CHAR_BIT + ((vec.size() % CHAR_BIT) > 0);
        auto *outArr = new uint8_t[resSize];

        asArray(outArr);
        return outArr;
    }

    size_t asArray(uint8_t* buf) const {
        vector<bool> vec = asVector();
        size_t bufSize = vec.size() / CHAR_BIT + ((vec.size() % CHAR_BIT) > 0);

        memset(buf, 0, bufSize);
        if(!isSegmented()){
            buf += mQmapHeader.asArray(buf);
        }
        if(mEthernetHeaderValid){
            buf += mEthernetHeader.asArray(buf);
        }
        buf += mInternetHeader.asArray(buf);
        buf += mTransportHeader.asArray(buf);
        for(auto val: mPayload){
            *buf++ = val;
        }
        return bufSize;
    }

    vector<UlsoPacket> segment() const {
        bool first = true;
        uint32_t seqNum = 0;

        if(isSegmented()){
            throw std::logic_error("A segmented packet cannot be segmented again!");
        }
        unsigned int segmentSize = mQmapHeader.mSegmentSize.to_ulong();
        vector<vector<uint8_t>> payloads = segmentPayload(segmentSize, mPayload);
        UlsoPacket ulsoCopy(*this);
        fixFlags(ulsoCopy.mTransportHeader);
        vector<UlsoPacket> outVec = vector<UlsoPacket>(payloads.size(), ulsoCopy);
        for(size_t i = 0; i < outVec.size(); i++){
            UlsoPacket& p = outVec[i];
            p.mPayload = payloads[i];
        }
        if(!outVec.empty()){
            fixLastSegmentFlags(outVec[outVec.size() - 1].mTransportHeader);
        }
        if(mQmapHeader.mIpIdCfg == 0){
            fixIpId(outVec, mMinId, mMaxId);
        }
        for(UlsoPacket& p: outVec){
            p.mInternetHeader.adjust(p.mTransportHeader.size() + p.mPayload.size(), p.mTransportHeader.protocolNum());
            p.adjustHeader(p.mTransportHeader, seqNum, first);
            p.mIsSegmented = true;
        }
        return outVec;
    }

    bool isSegmented() const {
        return mIsSegmented;
    }

    void setIpId(const uint16_t id){
        changeIpId(mInternetHeader, mTransportHeader);
    }

    void changeIpId(IPv4Header& iPv4Header, UdpHeader& udpHeader){
        bool first = true;
        uint32_t seqNum = 0;

        mInternetHeader.adjust(mTransportHeader.size() + mPayload.size(), mTransportHeader.protocolNum());
        adjustHeader(mTransportHeader, seqNum, first);
    }

    void changeIpId(IPv4Header& iPv4Header, TcpHeader& tcpHeader){
        bool first = true;
        uint32_t seqNum = 0;

        mInternetHeader.adjust(mTransportHeader.size() + mPayload.size(), mTransportHeader.protocolNum());
        adjustHeader(mTransportHeader, seqNum, first);
    }

private:

    static void fixFlags(TcpHeader& tcpHeader){
        tcpHeader.setmFIN(0);
        tcpHeader.setmPSH(0);
        tcpHeader.setmRST(0);
        tcpHeader.setmCWR(0);
    }

    static void fixFlags(UdpHeader& udpHeader){}

    void fixLastSegmentFlags(TcpHeader& tcpHeader) const {
        TcpHeader::flags flags = mTransportHeader.getFlags();

        tcpHeader.setmFIN(flags.fin);
        tcpHeader.setmPSH(flags.psh);
        tcpHeader.setmRST(flags.rst);
        tcpHeader.setmCWR(flags.cwr);
    }

    void fixLastSegmentFlags(UdpHeader& udpHeader) const {}

    static vector<vector<uint8_t>> segmentPayload(unsigned long segmentSize, const vector<uint8_t>& payload) {
        vector<vector<uint8_t>> outVec;

        for(size_t i = 0; i < payload.size(); i += segmentSize) {
            auto last = std::min<size_t>(static_cast<size_t>(payload.size()), i + segmentSize);
            auto index = i / segmentSize;

            outVec.emplace_back(vector<uint8_t>());
            auto& vec = outVec[index];
            vec.reserve(last - i);
            move(payload.begin() + i, payload.begin() + last, back_inserter(vec));
        }
        return outVec;
    }

    void adjustHeader(TcpHeader& tcpHeader, uint32_t& seqNum, bool& first){
        tcpHeader.zeroChecksum();
        if(first){
            seqNum = tcpHeader.getSeqNum();
            first = false;
        }
        tcpHeader.setmSequenceNumber(seqNum);
        seqNum += mPayload.size();
        size_t checksumBufSize = mInternetHeader.l3ChecksumPseudoHeaderSize() + mTransportHeader.size() + mPayload.size();
        uint8_t checksumBuf[checksumBufSize];
        memset(checksumBuf, 0, checksumBufSize);
        uint8_t *checksumBufPtr = checksumBuf;

        size_t ulsoBufSize = size();
        uint8_t ulsoBuf[ulsoBufSize];
        memset(ulsoBuf, 0, ulsoBufSize);
        asArray(ulsoBuf);
        size_t ipOffset = mQmapHeader.size() + mEthernetHeaderValid * mEthernetHeader.size();
        mInternetHeader.tcpChecksumPseudoHeader(checksumBuf, ulsoBuf + ipOffset);
        checksumBufPtr += mInternetHeader.l3ChecksumPseudoHeaderSize();
        checksumBufPtr += tcpHeader.asArray(checksumBufPtr);
        for(auto val: mPayload){
            *checksumBufPtr++ = val;
        }
        mTransportHeader.adjust(checksumBuf, checksumBufSize);
    }

    void adjustHeader(UdpHeader& udpHeader, uint32_t seqNum, bool first){
        udpHeader.zeroChecksum();
        if(mQmapHeader.mZeroChecksum.test(0)){
            mTransportHeader.adjust(mPayload.size());
        } else{
            udpHeader.setmLength(udpHeader.size() + mPayload.size());
            size_t checksumBufSize = mInternetHeader.l3ChecksumPseudoHeaderSize() + mTransportHeader.size() + mPayload.size();
            uint8_t checksumBuf[checksumBufSize];
            memset(checksumBuf, 0, checksumBufSize);
            uint8_t *checksumBufPtr = checksumBuf;

            size_t ulsoBufSize = size();
            uint8_t ulsoBuf[ulsoBufSize];
            memset(ulsoBuf, 0, ulsoBufSize);
            asArray(ulsoBuf);
            size_t ipOffset = mQmapHeader.size() + mEthernetHeaderValid * mEthernetHeader.size();
            mInternetHeader.udpChecksumPseudoHeader(checksumBuf, ulsoBuf + ipOffset);
            checksumBufPtr += mInternetHeader.l3ChecksumPseudoHeaderSize();
            checksumBufPtr += udpHeader.asArray(checksumBufPtr);
            for(auto val: mPayload){
                *checksumBufPtr++ = val;
            }
            mTransportHeader.adjust(checksumBuf, checksumBufSize, mPayload.size());
        }
    }

    template <typename T, typename I>
    friend std::ostream& operator<< (std::ostream &out, UlsoPacket<Transport, Internet> const& packet);
};

template <typename Transport, typename Internet>
constexpr uint8_t UlsoPacket<Transport, Internet>::qmapIPv4UdpPacketNoSeg[];

template <typename Transport, typename Internet>
inline std::ostream& operator << (std::ostream &out, UlsoPacket<Transport, Internet> const& packet) {
    out << "ULSO Packet\n" << "#Bytes=" << packet.size() << std::endl;
    if(!packet.isSegmented()){
        out << packet.mQmapHeader << std::endl;
    } else {
        out << "QMAP header removed in segmentation\n";
    }
    if(packet.mEthernetHeaderValid){
        out << packet.mEthernetHeader << std::endl;
    }
    out << packet.mInternetHeader << std::endl;
    out << packet.mTransportHeader << std::endl;
    out << "Payload\n" << packet.mPayload;
    return out;
}

template<typename Transport, typename Internet>
void fixIpId(vector<UlsoPacket<Transport, Internet>>& v, unsigned int minId, unsigned int maxId) {
    return;
}

template<>
void fixIpId<UdpHeader, IPv4Header>(vector<UlsoPacket<UdpHeader, IPv4Header>>& v, unsigned int minId, unsigned int maxId) {
    unsigned int curId = 0;

    if(!v.empty()){
        curId = std::max(static_cast<unsigned int>(v[0].mInternetHeader.mId.to_ulong()), minId) % (maxId + 1);
    }
    for (auto &p: v) {
        p.mInternetHeader.setmId(curId);
        curId++;
        if (curId == (maxId + 1)) curId = minId;
    }
}

template<>
void fixIpId<TcpHeader, IPv4Header>(vector<UlsoPacket<TcpHeader, IPv4Header>>& v, unsigned int minId, unsigned int maxId) {
    unsigned int curId = 0;

    if(!v.empty()){
        curId = std::max(static_cast<unsigned int>(v[0].mInternetHeader.mId.to_ulong()), minId) % (maxId + 1);
    }
    for (auto &p: v) {
        p.mInternetHeader.setmId(curId);
        curId++;
        if (curId == (maxId + 1)) curId = minId;
    }
}

template<typename Internet, typename Transport>
bool changeIpId(Internet& ipHeader, uint16_t id){
    return false;
}

template<>
bool changeIpId<IPv4Header, UdpHeader>(IPv4Header& iPv4Header, uint16_t id){
    iPv4Header.setmId(id);
    return true;
}

#endif //NETWORK_TRAFFIC_ULSOPACKET_H
