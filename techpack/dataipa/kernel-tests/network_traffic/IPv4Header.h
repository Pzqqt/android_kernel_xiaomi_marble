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
#ifndef NETWORK_TRAFFIC_IPV4HEADER_H
#define NETWORK_TRAFFIC_IPV4HEADER_H

#include "InternetHeader.h"

using std::bitset;

class IPv4Header: public InternetHeader {

public:

    const static unsigned int mSize {20};

    explicit IPv4Header(const uint8_t *start) {
        unsigned int bufIndex = 0;

        setBitsFromArray<uint8_t, 4>(mVersion, start, bufIndex);
        setBitsFromArray<uint8_t, 4>(mIhl, start, bufIndex);
        setBitsFromArray<uint8_t, 6>(mDscp, start, bufIndex);
        setBitsFromArray<uint8_t, 2>(mEcn, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mTotalLength, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mId, start, bufIndex);
        setBitsFromArray<uint8_t, 3>(mFlags, start, bufIndex);
        setBitsFromArray<uint8_t, 13>(mFragmentOffset, start, bufIndex);
        setBitsFromArray<uint8_t, 8>(mTimeToLive, start, bufIndex);
        setBitsFromArray<uint8_t, 8>(mProtocol, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mHeaderChecksum, start, bufIndex);
        setBitsFromArray<uint8_t, 32>(mSourceIpAddress, start, bufIndex);
        setBitsFromArray<uint8_t, 32>(mDestIpAddress, start, bufIndex);
    }

    DECLARE_BITSET(Version, 4, 4);
    DECLARE_BITSET(Ihl, 4, 5);
    DECLARE_BITSET(Dscp, 6, 0);
    DECLARE_BITSET(Ecn, 2, 0);
    DECLARE_BITSET(TotalLength, 16, mSize);
    DECLARE_BITSET(Id, 16, 65530);
    DECLARE_BITSET(Flags, 3, 2);
    DECLARE_BITSET(FragmentOffset, 13, 0);
    DECLARE_BITSET(TimeToLive, 8, 64);
    DECLARE_BITSET(Protocol, 8, 0);
    DECLARE_BITSET(HeaderChecksum, 16, 0);
    DECLARE_BITSET(SourceIpAddress, 32, 3232235793);
    DECLARE_BITSET(DestIpAddress, 32, 3232235816);

    IPv4Header() = default;

    IPv4Header(const IPv4Header& iPv4Header) = default;

    vector<bool> asVector() const override {
        vector<bool> outVec;
        auto inserter = [](vector<bool>& vec, auto val){
            vector<bool> valAsVector = bitsetToVector<val.size()>(val);
            vec.insert(vec.end(), valAsVector.begin(), valAsVector.end());};

        inserter(outVec, mVersion);
        inserter(outVec, mIhl);
        inserter(outVec, mDscp);
        inserter(outVec, mEcn);
        inserter(outVec, mTotalLength);
        inserter(outVec, mId);
        inserter(outVec, mFlags);
        inserter(outVec, mFragmentOffset);
        inserter(outVec, mTimeToLive);
        inserter(outVec, mProtocol);
        inserter(outVec, mHeaderChecksum);
        inserter(outVec, mSourceIpAddress);
        inserter(outVec, mDestIpAddress);
        toggleLsbMsb(outVec, 8);
        return outVec;
    }

    size_t size() const override {
        return mSize;
    }

    void adjust(size_t payloadSize, uint8_t protocol){
        mTotalLength = size() + payloadSize;
        mProtocol = protocol;
        fixChecksum();
    }

    static size_t getEtherType(){
        return 0x0800;
    }

    string name() const override {
        return string("IPV4");
    }

    void streamFields(std::ostream &out) const override {
        out << "Version: " << mVersion.to_ulong() << ", "
            << "IHL: " << mIhl.to_ulong() << ", "
            << "DSCP: " << mDscp.to_ulong() << ", "
            << "ECN: " << mEcn.to_ulong() << ", "
            << "Total Length: " << mTotalLength.to_ulong() << ", "
            << "ID: " << mId.to_ulong() << ", "
            << "Flags: " << mFlags.to_ulong() << ", "
            << "Fragment Offset: " << mFragmentOffset.to_ulong() << ", "
            << "TTL: " << mTimeToLive.to_ulong() << ", "
            << "Protocol: " << mProtocol.to_ulong() << ", "
            << "Checksum: " << mHeaderChecksum.to_ulong() << ", "
            << "Source IP Address: " << expandIpv4(mSourceIpAddress.to_ulong()) << ", "
            << "Destination IP Address: " << expandIpv4(mDestIpAddress.to_ulong()) << "\n";
    }

    void tcpChecksumPseudoHeader(uint8_t *pseudoHeaderBuf, const uint8_t *ipHeader) const {
        memcpy(pseudoHeaderBuf, ipHeader + 12, 8);
        pseudoHeaderBuf[8] = 0;
        pseudoHeaderBuf[9] = 6;
        *reinterpret_cast<uint16_t*>(pseudoHeaderBuf + 10) =
                htons(static_cast<uint16_t>(mTotalLength.to_ulong() - (mIhl.to_ulong() << 2u)));
    }

    static size_t l3ChecksumPseudoHeaderSize(){
        return 12;
    }

    void udpChecksumPseudoHeader(uint8_t *pseudoHeaderBuf, const uint8_t *ipHeader) const {
        memcpy(pseudoHeaderBuf, ipHeader + 12, 8);
        pseudoHeaderBuf[8] = 0;
        pseudoHeaderBuf[9] = 17;
        *reinterpret_cast<uint16_t*>(pseudoHeaderBuf + 10) =
                htons(static_cast<uint16_t>(mTotalLength.to_ulong() - (mIhl.to_ulong() << 2u)));
    }

private:

    void fixChecksum(){
        mHeaderChecksum = 0;
        uint8_t arr[mSize];

        asArray(arr);
        mHeaderChecksum = computeChecksum(reinterpret_cast<uint16_t*>(arr), mSize);
    }

};


#endif //NETWORK_TRAFFIC_IPV4HEADER_H
