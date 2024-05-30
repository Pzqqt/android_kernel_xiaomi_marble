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
#ifndef NETWORK_TRAFFIC_QMAPHEADER_H
#define NETWORK_TRAFFIC_QMAPHEADER_H

#include "Header.h"

using std::bitset;

class QmapHeader: public Header {

public:

    const static size_t mSize {8};

    DECLARE_BITSET(Pad, 6, 0);
    DECLARE_BITSET(NextHdr, 1, 1);
    DECLARE_BITSET(Cd, 1, 0);
    DECLARE_BITSET(MuxId, 8, 0);
    DECLARE_BITSET(PacketLength, 16, 0);
    DECLARE_BITSET(ExtensionNextHeader, 1, 0);
    DECLARE_BITSET(HeaderType, 7, 3);
    DECLARE_BITSET(AdditionalHdrSize, 5, 0);
    DECLARE_BITSET(Res, 1, 0);
    DECLARE_BITSET(ZeroChecksum, 1, 0);
    DECLARE_BITSET(IpIdCfg, 1, 0);
    DECLARE_BITSET(SegmentSize, 16, 1);

    explicit QmapHeader(const uint8_t *start) {
        unsigned int bufIndex = 0;
        setBitsFromArray<uint8_t, 6>(mPad, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mNextHdr, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mCd, start, bufIndex);
        setBitsFromArray<uint8_t, 8>(mMuxId, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mPacketLength, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mExtensionNextHeader, start, bufIndex);
        setBitsFromArray<uint8_t, 7>(mHeaderType, start, bufIndex);
        setBitsFromArray<uint8_t, 5>(mAdditionalHdrSize, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mRes, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mZeroChecksum, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mIpIdCfg, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mSegmentSize, start, bufIndex);
    }

    QmapHeader() = default;

    vector<bool> asVector() const override {
        vector<bool> outVec;
        auto inserter = [](vector<bool>& vec, auto val){
            vector<bool> valAsVector = bitsetToVector<val.size()>(val);
            vec.insert(vec.end(), valAsVector.begin(), valAsVector.end());};

        inserter(outVec, mPad);
        inserter(outVec, mNextHdr);
        inserter(outVec, mCd);
        inserter(outVec, mMuxId);
        vector<bool> v = bitsetToVector<16>(mPacketLength);
        toggleLsbMsb(v, v.size());
        toggleEndianness(v, v.size());
        outVec.insert(outVec.end(), v.begin(), v.end());
        inserter(outVec, mExtensionNextHeader);
        v = bitsetToVector<7>(mHeaderType);
        toggleLsbMsb(v, v.size());
        outVec.insert(outVec.end(), v.begin(), v.end());
        inserter(outVec, mAdditionalHdrSize);
        inserter(outVec, mRes);
        inserter(outVec, mZeroChecksum);
        inserter(outVec, mIpIdCfg);
        v = bitsetToVector<16>(mSegmentSize);
        toggleLsbMsb(v, v.size());
        toggleEndianness(v, v.size());
        outVec.insert(outVec.end(), v.begin(), v.end());
        return outVec;
    }

    size_t size() const override {
        return mSize;
    }

    string name() const override {
        return string("QMAP");
    }

    void streamFields(std::ostream &out) const override {
        out << "Padding: " << mPad.to_ulong() << ", "
            << "Next Header: " << mNextHdr.to_ulong() << ", "
            << "C\\D: " << mCd.to_ulong() << ", "
            << "Mux ID: " << mMuxId.to_ulong() << ", "
            << "Packet Length With Padding: " << mPacketLength.to_ulong() << ", "
            << "Extension Next Header: " << mExtensionNextHeader.to_ulong() << ", "
            << "Header Type: " << mHeaderType.to_ulong() << ", "
            << "Additional Header Size: " << mAdditionalHdrSize.to_ulong() << ", "
            << "Res: " << mRes.to_ulong() << ", "
            << "Zero Checksum: " << mZeroChecksum.to_ulong() << ", "
            << "IPID Cfg: " << mIpIdCfg.to_ulong() << ", "
            << "Segment Size: " << mSegmentSize.to_ulong() << "\n";
    }
};

#endif //NETWORK_TRAFFIC_QMAPHEADER_H
