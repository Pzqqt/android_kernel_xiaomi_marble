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
#include <iostream>
#include <string>
#include <cstring>
#include "UlsoPacket.h"

using std::cout;
using std::endl;
using std::string;

static uint8_t buf[UlsoPacket<UdpHeader>::maxSize];

static uint8_t qmapIPv4UdpPacketNoSeg[] = {
        //C=0,Next Header=1,PAD=b'63,MUXID=0,Packet length=30
        0X7f, 0x00, 0x00, 0x1e,//QMAP header
        //type=3,Next Header=0,IPIDCfg=1,Zero Checksum=1,Additional header size=0,segment size=2
        0X06, 0xc0, 0x00, 0x02,//QMAP header extension
        //IPv4 header
        0x45, 0x00, 0x00, 0x1e,//version=4,IHL=5,DSCP=0,ECN=0,length=30
        0x00, 0x00, 0x00, 0x00,//id=0,flags=0,fragment offset=0
        0xff, 0x11, 0x00, 0x00,//TTL=255,Protocol=17 (TCP),checksum=
        0xc0, 0xa8, 0x02, 0x13,//IPv4 SRC Addr 192.168.2.19
        0xc0, 0xa8, 0x02, 0x68,//IPv4 DST Addr 192.168.2.104
        //UDP header
        0x04, 0x57, 0x08, 0xae,//source port=1111, destination port=2222
        0x00, 0x0a, 0x00, 0x00,//length=10,checksum=
        //payload
        0x00, 0x01
};

static uint8_t qmapIPv4UdpPacket[] = {
        //C=0,Next Header=1,PAD=b'63,MUXID=0,Packet length=30
        0X7f, 0x00, 0x00, 0x1e,//QMAP header
        //type=3,Next Header=0,IPIDCfg=1,Zero Checksum=1,Additional header size=0,segment size=1
        0X06, 0xc0, 0x00, 0x01,//QMAP header extension
        //IPv4 header
        0x45, 0x00, 0x00, 0x1e,//version=4,IHL=5,DSCP=0,ECN=0,length=30
        0x00, 0x00, 0x00, 0x00,//id=0,flags=0,fragment offset=0
        0xff, 0x11, 0x00, 0x00,//TTL=255,Protocol=17 (TCP),checksum=
        0xc0, 0xa8, 0x02, 0x13,//IPv4 SRC Addr 192.168.2.19
        0xc0, 0xa8, 0x02, 0x68,//IPv4 DST Addr 192.168.2.104
        //UDP header
        0x04, 0x57, 0x08, 0xae,//source port=1111, destination port=2222
        0x00, 0x0a, 0x00, 0x00,//length=10,checksum=
        //payload
        0x00, 0x01
};

static uint8_t qmapIPv4UdpPacket2[] = {
        //C=0,Next Header=1,PAD=b'63,MUXID=0,Packet length=30
        0X7f, 0x00, 0x00, 0x20,//QMAP header
        //type=3,Next Header=0,IPIDCfg=0,Zero Checksum=1,Additional header size=0,segment size=1
        0X06, 0x00, 0x00, 0x01,//QMAP header extension
        //IPv4 header
        0x45, 0x00, 0x00, 0x20,//version=4,IHL=5,DSCP=0,ECN=0,length=30
        0x00, 0x00, 0x00, 0x00,//id=0,flags=0,fragment offset=0
        0xff, 0x11, 0x00, 0x00,//TTL=255,Protocol=17 (TCP),checksum=
        0xc0, 0xa8, 0x02, 0x13,//IPv4 SRC Addr 192.168.2.19
        0xc0, 0xa8, 0x02, 0x68,//IPv4 DST Addr 192.168.2.104
        //UDP header
        0x04, 0x57, 0x08, 0xae,//source port=1111, destination port=2222
        0x00, 0x0c, 0x00, 0x00,//length=10,checksum=
        //payload
        0x00, 0x01, 0x02, 0x03
};

char sep[] = "---------------------------------------------"
             "---------------------------------------------"
             "---------------------------------------------"
             "---------------------------------------------"
             "---------------------------------------------";

void printDemoHeadline(const string& name) {
    string full = "###################################################"
                  "###################################################";
    string side = "############################";
    string space = "             ";
    cout << full << endl << side << space << "Demo " + name << space
    << side << endl << full << endl;
}

void demo(UlsoPacket<>& ulsoPacket, const string& name) {
    printDemoHeadline(name);
    cout << ulsoPacket << endl;
    cout << sep << endl;

    vector<UlsoPacket<>> packets = ulsoPacket.segment();
    std::for_each(packets.begin(), packets.end(), [](const auto& p){cout << p << endl;
        cout << sep << endl;});
}

void testIpv4Tcp(){
    using L2Type = IPv4Header;
    using L3Type = TcpHeader;
    using PacketType = UlsoPacket<L3Type, L2Type>;
    size_t payloadSize = 91, segmentSize = 32;
    size_t packetSize = QmapHeader::mSize + L2Type::mSize + L3Type::mSize + payloadSize;

    PacketType p1(segmentSize, payloadSize);
    cout << p1 << endl;
    uint8_t ipv4HeaderBuf[packetSize];
    p1.asArray(ipv4HeaderBuf);
    uint8_t *ipv4HeaderPtr = ipv4HeaderBuf;
    uint8_t *goldBuf = QmapIpv4TcpPacket.l2Packet();
    for(unsigned int i=0; i<QmapIpv4TcpPacket.l2PacketSize(); i++){
        if(ipv4HeaderPtr[i] != goldBuf[i]){
            cout << "ipv4HeaderBuf[" << i << "]=" << std::hex << std::setfill('0') << std::setw(2) <<
                 static_cast<int>(ipv4HeaderPtr[i]) << std::dec << ", goldBuf[" << i << "]=" << std::setfill('0') <<
                 std::setw(2) << std::hex << static_cast<int>(goldBuf[i]) << std::dec << endl;
        }
    }
}

void testIpv4Udp(){
    using L2Type = IPv4Header;
    using L3Type = UdpHeader;
    using PacketType = UlsoPacket<L3Type, L2Type>;
    size_t payloadSize = 80, segmentSize = 32;
    size_t packetSize = QmapHeader::mSize + L2Type::mSize + L3Type::mSize + payloadSize;

    PacketType p1(segmentSize, payloadSize);
    cout << p1 << endl;
    uint8_t ipv4UdpHeaderBuf[packetSize];
    p1.asArray(ipv4UdpHeaderBuf);
    uint8_t *udpHeaderPtr = ipv4UdpHeaderBuf + QmapHeader::mSize + L2Type::mSize;
    uint8_t *goldBuf = udpPacket.l3Packet();
    for(unsigned int i=0; i<udpPacket.l3PacketSize(); i++){
        if(udpHeaderPtr[i] != goldBuf[i]){
            cout << "udpPayload[" << i << "]=" << std::hex << std::setfill('0') << std::setw(2) <<
                 static_cast<int>(udpHeaderPtr[i]) << std::dec << ", goldBuf[" << i << "]=" << std::setfill('0') <<
                 std::setw(2) << std::hex << static_cast<int>(goldBuf[i]) << std::dec << endl;
        }
    }
}

template<typename L3Type, typename L2Type>
size_t packetTestOffset(const struct Packet& p){
    return QmapHeader::mSize + (p.l2Size == 0) * L2Type::mSize + (p.l3Size == 0) * L3Type::mSize;
}

template<typename L3Type, typename L2Type>
bool testPacketConstruction(struct Packet packet, size_t segmentSize=20){
    auto testOffsetFunc = packetTestOffset<L3Type, L2Type>;
    using PacketType = UlsoPacket<L3Type, L2Type>;
    size_t ipPacketSize = packet.l2PacketSize();
    PacketType ulsoPacket(segmentSize, packet.payload(), packet.payloadSize);
    if(ulsoPacket.size() - testOffsetFunc(packet) != ipPacketSize){
        cout << "Error: ULSO packet size = " << ulsoPacket.size() << " expected size = " << ipPacketSize << endl;
        return false;
    }
    uint8_t ulsoPacketBuf[ulsoPacket.size()];
    ulsoPacket.asArray(ulsoPacketBuf);
    uint8_t *ulsoCmpBuf = ulsoPacketBuf + testOffsetFunc(packet);
    uint8_t *goldBuf = packet.l2Packet();
    for(unsigned int i=0; i<ipPacketSize; i++){
        if(ulsoCmpBuf[i] != goldBuf[i]){
            cout << "afterQmapPtr[" << i << "]=" << std::hex << std::setfill('0') << std::setw(2) <<
                 static_cast<int>(ulsoCmpBuf[i]) << std::dec << ", goldBuf[" << i << "]=" << std::setfill('0') <<
                 std::setw(2) << std::hex << static_cast<int>(goldBuf[i]) << std::dec << endl;
        }
    }
    return true;
}

int main() {

    uint8_t arr[UlsoPacket<>::maxSize] = {0};
    UlsoPacket<UdpHeader, IPv4Header> p(19, 100, false);
    p.mQmapHeader.setmIpIdCfg(0);
    cout << p << endl;
    auto v = p.segment();
    for(auto& pSeg: v){
        memset(arr, 0, UlsoPacket<>::maxSize);
        cout << pSeg << endl;
    }

    return 0;
}
