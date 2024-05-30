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
#ifndef NETWORK_TRAFFIC_HEADER_H
#define NETWORK_TRAFFIC_HEADER_H


#include <vector>
#include <climits>
#include <cassert>
#include <ostream>
#include <bitset>
#include <iostream>
#include <iomanip>
#include <netinet/in.h>
#include "bits_utils.h"

using std::vector;
using std::bitset;
using std::string;

// make a variable name fit for member variable name convention.
#define MEMBERIZE_VAR(name) m##name
// declare a bitset variable with any number of bits and also an integer setter method.
#define DECLARE_BITSET(name, numBits, defaultVal)                               \
bitset<numBits> MEMBERIZE_VAR(name) {defaultVal};                               \
void setm##name(unsigned int name) {                                            \
    for(int i = 0; i < numBits; i++){                                           \
    MEMBERIZE_VAR(name)[i] = getUintNthBit(name, i);                            \
    }                                                                           \
}                                                                               \
static_assert(true, "")

class Header {
/**
 * class Header is an abstract class that provides the interface that is needed for every protocol header.
 * It also provides implementation of common operations for all protocol headers.
 * Every protocol header class must inherit from this class directly or indirectly.
 */
public:

    virtual vector<bool> asVector() const = 0;

    virtual void streamFields(std::ostream &out) const = 0;

    virtual size_t size() const = 0;

    virtual string name() const = 0;

    virtual size_t asArray(uint8_t* buf) const {
        vector<bool> vec = asVector();
        size_t resSize = vec.size() / CHAR_BIT + ((vec.size() % CHAR_BIT) > 0);

        for(size_t i = 0; i < vec.size(); i++){
            changeNthBit(buf[i/8], (i%8), vec[i]);
        }
        return resSize;
    }

    static uint16_t computeChecksum(uint16_t *buf, size_t count){
        uint32_t sum = 0;

        while(count > 1){
            sum += *buf++;
            count -= 2;
        }
        if(count > 0){
            sum += ((*buf)&htons(0xFF00));
        }
        while(sum >> 16u){
            sum = (sum & 0xffffu) + (sum >> 16u);
        }
        sum = ~sum;
        return htons(static_cast<uint16_t>(sum));
    }

    virtual ~Header() = default;

};

template<typename T>
inline std::ostream& operator<< (std::ostream &out, vector<T>const& v) {
    out << "#Bytes=" << v.size() << std::endl;
    for (size_t i = 0; i < v.size(); i++)
        out << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(v[i]) << " ";
    out << std::dec;
    return out;
}

inline std::ostream& operator<< (std::ostream &out, Header const& h) {
    vector<bool> headerAsVec = h.asVector();
    size_t bufSize = headerAsVec.size() / CHAR_BIT + ((headerAsVec.size() % CHAR_BIT) > 0);
    uint8_t buf[bufSize];

    out << h.name() + " Header" << std::endl;
    out << "#Bytes=" << h.size() << std::endl;
    h.streamFields(out);
    memset(buf, 0, bufSize);
    h.asArray(buf);
    for (size_t i = 0; i < bufSize; i++)
        out << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(buf[i]) << " ";
    out << std::dec;
    return out;
}

#endif //NETWORK_TRAFFIC_HEADER_H
