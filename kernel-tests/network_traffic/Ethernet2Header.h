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
#ifndef NETWORK_TRAFFIC_ETHERNET2HEADER_H
#define NETWORK_TRAFFIC_ETHERNET2HEADER_H


#include "Header.h"

#define ETHER_TYPE 16

class Ethernet2Header: public Header {

public:

    const static unsigned int mSize {14};

    DECLARE_BITSET(DestMac, 48, 0x54E1ADB47F9F);
    DECLARE_BITSET(SourceMac, 48, 0xA0E0AF89A93F);
    DECLARE_BITSET(EtherType, ETHER_TYPE, 0x0800);

    vector<bool> asVector() const override {
        vector<bool> outVec;
        auto inserter = [](vector<bool>& vec, auto val){
            vector<bool> valAsVector = bitsetToVector<val.size()>(val);
            toggleLsbMsb(valAsVector, CHAR_BIT);
            vec.insert(vec.end(), valAsVector.begin(), valAsVector.end());};

        inserter(outVec, mDestMac);
        inserter(outVec, mSourceMac);
        inserter(outVec, mEtherType);
        return outVec;
    }

    size_t size() const override {
        return mSize;
    }

    string name() const override {
        return string("Ethernet 2");
    }

    void streamFields(std::ostream &out) const override {
        out << "mDestAddress: " << mDestMac.to_ullong() << ", "
            << "SourceMac: " << mSourceMac.to_ullong() << ", "
            << "mEtherType: " << mEtherType.to_ulong() << "\n";
    }
};


#endif //NETWORK_TRAFFIC_ETHERNET2HEADER_H
