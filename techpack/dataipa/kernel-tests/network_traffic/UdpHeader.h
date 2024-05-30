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
#ifndef NETWORK_TRAFFIC_UDPHEADER_H
#define NETWORK_TRAFFIC_UDPHEADER_H

#include "TransportHeader.h"

class UdpHeader: public TransportHeader {

public:

    const static unsigned int mSize {8};

    DECLARE_BITSET(SourcePort, 16, 1985);
    DECLARE_BITSET(DestPort, 16, 1985);
    DECLARE_BITSET(Length, 16, 0);
    DECLARE_BITSET(Checksum, 16, 0);

    explicit UdpHeader(const uint8_t *start) {
        unsigned int bufIndex = 0;

        setBitsFromArray<uint8_t, 16>(mSourcePort, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mDestPort, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mLength, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mChecksum, start, bufIndex);
    }

    UdpHeader(uint16_t sourcePort, uint16_t destPort, uint16_t length, uint16_t checksum) :
            mSourcePort(sourcePort),
            mDestPort(destPort),
            mLength(length),
            mChecksum(checksum) {}

    UdpHeader(const UdpHeader& udpHeader) = default;

    UdpHeader() = default;

    vector<bool> asVector() const override {
        vector<bool> outVec;
        auto inserter2 = [&outVec](bitset<16>  val){
            vector<bool> valAsVector = bitsetToVector<16>(val);
            toggleLsbMsb(valAsVector, CHAR_BIT);
            outVec.insert(outVec.end(), valAsVector.begin(), valAsVector.end());};

        inserter2(mSourcePort);
        inserter2(mDestPort);
        inserter2(mLength);
        inserter2(mChecksum);
        return outVec;
    }

    size_t size() const override {
        return mSize;
    }

    static uint8_t protocolNum(){
        return 17;
    }

    void adjust(const size_t payloadSize){
        mLength = mSize + payloadSize;
    }

    void adjust(uint8_t *checksumBuf, size_t count, size_t payloadSize){
        mLength = mSize + payloadSize;
        fixChecksum(checksumBuf, count);
    }

    void zeroChecksum(){
        mChecksum = 0;
    }

    string name() const override {
        return string("UDP");
    }

    void streamFields(std::ostream &out) const override {
        out << "Source port: " << mSourcePort.to_ulong() << ", "
            << "Destination port: " << mDestPort.to_ulong() << ", "
            << "Length: " << mLength.to_ulong() << ", "
            << "Checksum: " << mChecksum.to_ulong() << "\n";
    }

private:

    void fixChecksum(uint8_t *checksumBuf, size_t count){
        mChecksum = 0;
        mChecksum = computeChecksum(reinterpret_cast<uint16_t*>(checksumBuf), count);
    }

};

#endif //NETWORK_TRAFFIC_UDPHEADER_H
