cmake_minimum_required(VERSION 3.17)
project(network_traffic)

set(CMAKE_CXX_STANDARD 14)

add_executable(network_traffic main.cpp Header.h UdpHeader.h IPv4Header.h QmapHeader.h UlsoPacket.h bits_utils.h
        TransportHeader.h InternetHeader.h IPv6Header.h TcpHeader.h packets.h Ethernet2Header.h)