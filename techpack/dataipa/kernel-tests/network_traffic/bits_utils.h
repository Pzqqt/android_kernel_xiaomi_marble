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
#ifndef NETWORK_TRAFFIC_BITS_UTILS_H
#define NETWORK_TRAFFIC_BITS_UTILS_H


#include <vector>
#include <bitset>


#define SIZE_OF_BITS(x) (sizeof(x) * CHAR_BIT)


using std::vector;
using std::bitset;

/**
 * bitwise get the n-th bit of val.
 * @tparam T - some unsigned integer type
 * @param num - number to get the bit from
 * @param n - index of bit to return
 * @return 1 if the n-th bit of val is 1, 0 otherwise
 */
template<typename T>
bool getUintNthBit(T num, unsigned int n) {
    assert(n >= 0 && n < (SIZE_OF_BITS(num)));
    return (num & ( 1 << n )) >> n; // NOLINT(hicpp-signed-bitwise)
}

/**
 * Transforms a vector<bool> from a bitset<N>.
 * @tparam N - Number of bits.
 * @param bits - Bits to transform.
 * @return vector<bool> that represents the bitset
 */
template<int N>
static vector<bool> bitsetToVector(const bitset<N>& bits){
    vector<bool> outVec;

    for(int i = 0; i < N; i++){
        outVec.insert(outVec.begin(), bits[i]);
    }
    return outVec;
}

template<typename T>
static vector<bool> uintToVector(T n) {
    vector<bool> outVec;

    for (int i = 0; i < SIZE_OF_BITS(n); i++) {
        outVec.insert(outVec.begin(), getUintNthBit(n, i));
    }
    return outVec;
}

/**
 * Copies a range of bits in an array of integer-like elements to a bitset.
 * The range is [bufIndex, N-1]
 * @tparam T - The type of the elements in the array.
 * @tparam N - The size of the range.
 * @param bits - the bits object to copy into.
 * @param buf - Input array.
 * @param bufIndex - starting bit.
 */
template<typename T, unsigned int N>
static void setBitsFromArray(bitset<N>& bits, const T* buf, unsigned int& bufIndex) {
    for (int i = N-1; i >= 0; i--) {
        unsigned int bitIndex = SIZE_OF_BITS(T) - 1 - (bufIndex % SIZE_OF_BITS(T));
        bool bit = getUintNthBit(*(buf + (bufIndex / 8)), bitIndex);
        bits.set(i, bit);
        bufIndex++;
    }
}

template<typename T, unsigned int N>
static void setBitsFromArray(bitset<N>& bits, const T* buf) {
    unsigned int idx = 0;

    for (int i = N - 1; i >= 0; i--) {
        unsigned int bitIndex = SIZE_OF_BITS(T) - 1 - (idx % SIZE_OF_BITS(T));
        bool bit = getUintNthBit(*(buf + (idx / 8)), bitIndex);
        bits.set(i, bit);
        idx++;
    }
}

template<typename T>
void setNthBit(T& num, unsigned int n){
    assert(n < SIZE_OF_BITS(num));
    num |= 1UL << n;
}

/**
 * Clears the n-th bit of num
 * @tparam T - the type of num
 * @param num - the number to clear a bit on
 * @param n - the index of the bit to clear
 */
template<typename T>
void clearNthBit(T& num, unsigned int n){
    assert(n < SIZE_OF_BITS(num));
    num &= ~(1UL << n);
}

template<typename T>
void changeNthBit(T& num, unsigned int n, bool bitVal){
    assert(n < SIZE_OF_BITS(num));
    if(bitVal){
        setNthBit(num, n);
    } else {
        clearNthBit(num, n);
    }
}

void toggleLsbMsb(vector<bool>& v, unsigned int intervalSize){
    if(v.size() % intervalSize != 0){
        return;
    }
    for(size_t i = 0; i < v.size(); i += intervalSize){
        vector<bool> tmp(intervalSize);
        for(unsigned int j = 0; j < intervalSize; j++){
            tmp[j] = v[i + intervalSize - 1 - j];
        }
        for(unsigned int j = 0; j < intervalSize; j++){
            v[i + j] = tmp[j];
        }
    }
}

void toggleEndianness(vector<bool>& v, unsigned int wordSize){
    if(wordSize % CHAR_BIT != 0 || v.size() % wordSize != 0){
        return;
    }
    for(size_t i = 0; i < v.size(); i += wordSize){
        vector<bool> tmp(wordSize);
        for(size_t j = 0; j < wordSize; j += CHAR_BIT){
            for(size_t k = 0; k < CHAR_BIT; k++){
                unsigned int readIdx = i + wordSize - j - CHAR_BIT + k;
                unsigned int writeIdx = j + k;
                tmp[writeIdx] = v[readIdx];
            }
        }
        for(unsigned int j = 0; j < wordSize; j++){
            v[i + j] = tmp[j];
        }
    }
}

template<typename T, typename W>
W bitWiseConcatenate(T left, T right){
    W wide = (static_cast<W>(left) << CHAR_BIT * (sizeof(W) - sizeof(T))) | right;
    return wide;
}

template<typename IntType>
void toArray(vector<bool>& v, IntType* buf){
    for(unsigned int i = 0; i < v.size(); i++){
        if(v[i]){
            setNthBit(buf[i / SIZE_OF_BITS(*buf)], i % SIZE_OF_BITS(*buf));
        }
    }
}

#endif //NETWORK_TRAFFIC_BITS_UTILS_H
