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
#ifndef NETWORK_TRAFFIC_IPV6HEADER_H
#define NETWORK_TRAFFIC_IPV6HEADER_H


#include <netinet/in.h>
#include <arpa/inet.h>
#include "InternetHeader.h"

class IPv6Header: public InternetHeader {

public:

    const static unsigned int mSize {40};

    explicit IPv6Header(const uint8_t *start) {
        unsigned int bufIndex = 0;
        setBitsFromArray<uint8_t, 4>(mVersion, start, bufIndex);
        setBitsFromArray<uint8_t, 8>(mTrafficClass, start, bufIndex);
        setBitsFromArray<uint8_t, 20>(mFlowLabel, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mPayloadLength, start, bufIndex);
        setBitsFromArray<uint8_t, 8>(mNextHeader, start, bufIndex);
        setBitsFromArray<uint8_t, 8>(mHopLimit, start, bufIndex);
        setBitsFromArray<uint8_t, 128>(mSourceIpAddress, start, bufIndex);
        setBitsFromArray<uint8_t, 128>(mDestIpAddress, start, bufIndex);
    }

    DECLARE_BITSET(Version, 4, 6);
    DECLARE_BITSET(TrafficClass, 8, 0);
    DECLARE_BITSET(FlowLabel, 20, 377341);
    DECLARE_BITSET(PayloadLength, 16, 0);
    DECLARE_BITSET(NextHeader, 8, 0);
    DECLARE_BITSET(HopLimit, 8, 1);
    DECLARE_BITSET(SourceIpAddress, 128, 3232236051);
    DECLARE_BITSET(DestIpAddress, 128, 3232236136);

    IPv6Header() = default;

    vector<bool> asVector() const override {
        vector<bool> outVec;
        auto inserter = [](vector<bool>& vec, auto val){
            vector<bool> valAsVector = bitsetToVector<val.size()>(val);
            toggleLsbMsb(valAsVector, CHAR_BIT);
            vec.insert(vec.end(), valAsVector.begin(), valAsVector.end());};

        vector<bool> v = bitsetToVector<4>(mVersion);
        vector<bool> v2 = bitsetToVector<8>(mTrafficClass);
        v.insert(v.end(), v2.begin(), v2.end());
        v2 = bitsetToVector<20>(mFlowLabel);
        v.insert(v.end(), v2.begin(), v2.end());
        toggleLsbMsb(v, CHAR_BIT);
        outVec.insert(outVec.end(), v.begin(), v.end());
        inserter(outVec, mPayloadLength);
        inserter(outVec, mNextHeader);
        inserter(outVec, mHopLimit);
        inserter(outVec, mSourceIpAddress);
        inserter(outVec, mDestIpAddress);
        return outVec;
    }

    size_t size() const override {
        return mSize;
    }

    void adjust(size_t payloadSize, uint8_t protocol){
        mPayloadLength = payloadSize;
        mNextHeader = protocol;
    }

    void tcpChecksumPseudoHeader(uint8_t* pseudoHeaderBuf, uint8_t* ipHeader) const {
        memcpy(pseudoHeaderBuf, ipHeader + 8, 32);
        *reinterpret_cast<uint32_t*>(pseudoHeaderBuf + 32) =
                htons(static_cast<uint16_t>(mPayloadLength.to_ulong()));
        *reinterpret_cast<uint32_t*>(pseudoHeaderBuf + 36) = 0;
        pseudoHeaderBuf[39] = 6;
    }

    void udpChecksumPseudoHeader(uint8_t* pseudoHeaderBuf, const uint8_t* ipHeader) const {
        memcpy(pseudoHeaderBuf, ipHeader + 8, 32);
        *reinterpret_cast<uint32_t*>(pseudoHeaderBuf + 32) = htons(static_cast<uint16_t>(mPayloadLength.to_ulong()));
        *reinterpret_cast<uint32_t*>(pseudoHeaderBuf + 36) = 0;
        pseudoHeaderBuf[39] = 17;
    }

    static size_t l3ChecksumPseudoHeaderSize(){
        return 40;
    }

    static size_t getEtherType(){
        return 0x86dd;
    }

    string name() const override {
        return string("IPV6");
    }

    void streamFields(std::ostream &out) const override {
        uint8_t buf[32];
        vector<bool> v = asVector();

        v = vector<bool>(v.begin() + 8, v.end());
        toArray(v, buf);
        out << "Version: " << mVersion.to_ulong() << ", "
            << "IHL: " << mTrafficClass.to_ulong() << ", "
            << "DSCP: " << mFlowLabel.to_ulong() << ", "
            << "ECN: " << mPayloadLength.to_ulong() << ", "
            << "Total Length: " << mNextHeader.to_ulong() << ", "
            << "ID: " << mHopLimit.to_ulong() << ", "
            << "Source IP Address: " << expandIpv6(buf) << ", "
            << "Destination IP Address: " << expandIpv6(buf) << "\n";
    }

};

#endif //NETWORK_TRAFFIC_IPV6HEADER_H
