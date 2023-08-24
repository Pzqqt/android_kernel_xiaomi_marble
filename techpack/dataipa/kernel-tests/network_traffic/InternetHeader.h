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
#ifndef NETWORK_TRAFFIC_INTERNETHEADER_H
#define NETWORK_TRAFFIC_INTERNETHEADER_H


#include "Header.h"

using std::string;

class InternetHeader: public Header {
/**
 * class InternetHeader is an abstract class that inherits from class Header. It provides identification for all
 * internet layer protocol headers and also some common functionality. Every internet layer protocol header must inherit
 * from it directly or indirectly.
 */
protected:

    string expandIpv4(unsigned int ip) const {
        string ipString = std::to_string((ip >> 24u) & 0xffu);
        ipString += "." + std::to_string((ip >> 16u) & 0xfu);
        ipString += "." + std::to_string((ip >> 8u) & 0xffu);
        ipString += "." + std::to_string(ip & 0xffu);
        return ipString;
    }

    string expandIpv6(const uint8_t* buf) const {
        char str[40];

        snprintf(str, sizeof(str), "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                static_cast<int>(buf[0]), static_cast<int>(buf[1]),
                static_cast<int>(buf[2]), static_cast<int>(buf[3]),
                static_cast<int>(buf[4]), static_cast<int>(buf[5]),
                static_cast<int>(buf[6]), static_cast<int>(buf[7]),
                static_cast<int>(buf[8]), static_cast<int>(buf[9]),
                static_cast<int>(buf[10]), static_cast<int>(buf[11]),
                static_cast<int>(buf[12]), static_cast<int>(buf[13]),
                static_cast<int>(buf[14]), static_cast<int>(buf[15]));
        return string(str);
    }
};


#endif //NETWORK_TRAFFIC_INTERNETHEADER_H
