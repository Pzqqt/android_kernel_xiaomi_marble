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
#ifndef NETWORK_TRAFFIC_TCPHEADER_H
#define NETWORK_TRAFFIC_TCPHEADER_H


#include "TransportHeader.h"

class TcpHeader: public TransportHeader {

public:

    const static unsigned int mSizeBits {160};
    const static unsigned int mSize {20};

    DECLARE_BITSET(SourcePort, 16, 56789);
    DECLARE_BITSET(DestPort, 16, 64886);
    DECLARE_BITSET(SequenceNumber, 32, 1077280711);
    DECLARE_BITSET(AckNumber, 32, 3090542765);
    DECLARE_BITSET(DataOffset, 4, 5);
    DECLARE_BITSET(Reserved, 3, 0);
    DECLARE_BITSET(NS, 1, 0);
    DECLARE_BITSET(CWR, 1, 0);
    DECLARE_BITSET(ECE, 1, 0);
    DECLARE_BITSET(URG, 1, 0);
    DECLARE_BITSET(ACK, 1, 1);
    DECLARE_BITSET(PSH, 1, 1);
    DECLARE_BITSET(RST, 1, 0);
    DECLARE_BITSET(SYN, 1, 0);
    DECLARE_BITSET(FIN, 1, 0);
    DECLARE_BITSET(WindowSize, 16, 473);
    DECLARE_BITSET(Checksum, 16, 0);
    DECLARE_BITSET(UrgentPtr, 16, 0);

    TcpHeader() = default;

    explicit TcpHeader(const uint8_t *start) {
        unsigned int bufIndex = 0;

        setBitsFromArray<uint8_t, 16>(mSourcePort, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mDestPort, start, bufIndex);
        setBitsFromArray<uint8_t, 32>(mSequenceNumber, start, bufIndex);
        setBitsFromArray<uint8_t, 32>(mAckNumber, start, bufIndex);
        setBitsFromArray<uint8_t, 4>(mDataOffset, start, bufIndex);
        setBitsFromArray<uint8_t, 3>(mReserved, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mNS, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mCWR, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mECE, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mURG, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mACK, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mPSH, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mRST, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mSYN, start, bufIndex);
        setBitsFromArray<uint8_t, 1>(mFIN, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mWindowSize, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mChecksum, start, bufIndex);
        setBitsFromArray<uint8_t, 16>(mUrgentPtr, start, bufIndex);
    }
    vector<bool> asVector() const override {
        vector<bool> outVec;
        auto inserter = [](vector<bool>& vec, auto val){
            vector<bool> valAsVector = bitsetToVector<val.size()>(val);
            vec.insert(vec.end(), valAsVector.begin(), valAsVector.end());};
        auto inserter2 = [](vector<bool>& vec, auto val){
            vector<bool> valAsVector = bitsetToVector<val.size()>(val);
            toggleLsbMsb(valAsVector, CHAR_BIT);
            vec.insert(vec.end(), valAsVector.begin(), valAsVector.end());};

        inserter2(outVec, mSourcePort);
        inserter2(outVec, mDestPort);
        inserter2(outVec, mSequenceNumber);
        inserter2(outVec, mAckNumber);
        vector<bool> v = bitsetToVector<4>(mDataOffset);
        vector<bool> v2 = bitsetToVector<3>(mReserved);
        v.insert(v.end(), v2.begin(), v2.end());
        v2 = bitsetToVector<1>(mNS);
        v.insert(v.end(), v2.begin(), v2.end());
        outVec.insert(outVec.end(), v.rbegin(), v.rend());
        v = bitsetToVector<1>(mCWR);
        v2 = bitsetToVector<1>(mECE);
        v.insert(v.end(), v2.begin(), v2.end());
        v2 = bitsetToVector<1>(mURG);
        v.insert(v.end(), v2.begin(), v2.end());
        v2 = bitsetToVector<1>(mACK);
        v.insert(v.end(), v2.begin(), v2.end());
        v2 = bitsetToVector<1>(mPSH);
        v.insert(v.end(), v2.begin(), v2.end());
        v2 = bitsetToVector<1>(mRST);
        v.insert(v.end(), v2.begin(), v2.end());
        v2 = bitsetToVector<1>(mSYN);
        v.insert(v.end(), v2.begin(), v2.end());
        v2 = bitsetToVector<1>(mFIN);
        v.insert(v.end(), v2.begin(), v2.end());
        outVec.insert(outVec.end(), v.rbegin(), v.rend());
        inserter2(outVec, mWindowSize);
        inserter2(outVec, mChecksum);
        inserter(outVec, mUrgentPtr);
        return outVec;
    }

    uint32_t getSeqNum() const {
        return static_cast<uint32_t>(mSequenceNumber.to_ulong());
    }

    size_t size() const override {
        return TcpHeader::mSize;
    }

    static uint8_t protocolNum(){
        return 6;
    }

    void adjust(uint8_t *checksumBuf, size_t count){
        fixChecksum(checksumBuf, count);
    }

    void zeroChecksum(){
        mChecksum = 0;
    }

    string name() const override {
        return string("TCP");
    }

    void streamFields(std::ostream &out) const override {
        out << "Source Port: " << mSourcePort.to_ulong() << ", "
            << "Dest Port: " << mDestPort.to_ulong() << ", "
            << "Sequence Number: " << mSequenceNumber.to_ulong() << ", "
            << "Ack Number: " << mAckNumber.to_ulong() << ", "
            << "Data Offset: " << mDataOffset.to_ulong() << ", "
            << "Reserved: " << mReserved.to_ulong() << ", "
            << "NS: " << mNS.to_ulong() << ", "
            << "CWR: " << mCWR.to_ulong() << ", "
            << "URG: " << mURG.to_ulong() << ", "
            << "ACK: " << mACK.to_ulong() << ", "
            << "PSH: " << mPSH.to_ulong() << ", "
            << "RST: " << mRST.to_ulong() << ", "
            << "SYN: " << mSYN.to_ulong() << ", "
            << "FIN: " << mFIN.to_ulong() << ", "
            << "Window Size: " << mWindowSize.to_ulong() << ", "
            << "Checksum: " << mChecksum.to_ulong() << ", "
            << "Urgent Ptr: " << mUrgentPtr << std::endl;
    }

    struct flags {
        bool ns;
        bool cwr;
        bool ece;
        bool urg;
        bool ack;
        bool psh;
        bool rst;
        bool syn;
        bool fin;
    };

    flags getFlags() const {
        return flags {
            .ns = mNS.test(0),
            .cwr = mCWR.test(0),
            .ece = mECE.test(0),
            .urg = mURG.test(0),
            .ack = mACK.test(0),
            .psh = mPSH.test(0),
            .rst = mRST.test(0),
            .syn = mSYN.test(0),
            .fin = mFIN.test(0)
        };
    }

private:

    void fixChecksum(uint8_t *checksumBuf, size_t count){
        mChecksum = 0;
        mChecksum = computeChecksum(reinterpret_cast<uint16_t*>(checksumBuf), count);
    }

};


#endif //NETWORK_TRAFFIC_TCPHEADER_H
