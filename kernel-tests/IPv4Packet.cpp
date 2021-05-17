/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

#include "TestsUtils.h"
#include "IPv4Packet.h"
#include "memory.h"

using namespace IPA;

static const unsigned char TCP_IP_PACKET_DUMP[] = { 0x45, // IPv4, IHL = 5
		0x00, // ToS = 0
		0x00, 0x28, // Total length
		0x11, 0xc2, // ID
		0x40, 0x00, //ID + Fragment Offset
		0x80, // TTL
		0x06, // Protocol = TCP
		0x70, 0x3a, //Checksum
		0x0a, 0x05, 0x07, 0x46, // Source IP 10.5.7.70
		0x81, 0x2e, 0xe6, 0x5a, // Destination IP 129.46.230.90
		0xf3, 0xa2, // Source Port  62370
		0x01, 0xbd, // Destination Port 445
		0x26, 0x26, 0x1d, 0x7d, // Seq Number
		0x15, 0xaa, 0xbc, 0xdb, // Ack Num
		0x50, 0x10, 0x80, 0xd4, // TCP Params
		0xaa, 0xa3, // TCP Checksum
		0x00, 0x00 // Urgent PTR
		};

static const unsigned char UDP_IP_PACKET_DUMP[] = {
		0x45, // IPv4, IHL = 5
		0x00, // ToS = 0
		0x00,
		0x34, // Total Length
		0x12,
		0xa2, // ID
		0x00,
		0x00, //ID + fragment offset
		0x80, // TTL
		0x11, // Protocol = UDP
		0xe4,
		0x92, // Checksum
		0x0a, 0x05, 0x07,
		0x46, // Source IP 10.5.7.70
		0x0a, 0x2b, 0x28,
		0x0f, // Destination IP 10.43.40.15
		0x03,
		0x4a, // Source port 842
		0x1b,
		0x4f, // Destination Port 6991
		0x00,
		0x20, // UDP length
		0x36,
		0xac, // UDP checksum
		0x00, 0x05, 0x20,
		0x6d, // Data
		0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x01, 0x13, 0x05, 0x20, 0x6c };
static unsigned char ICMP_IP_PACKET_DUMP[] = {
		//IP
		0x45, 0x00, 0x00, 0xdc, 0x03, 0xfe, 0x00, 0x00, 0x80, 0x01, 0x00, 0x00,
		0x81, 0x2e, 0xe4, 0xf6, 0x81, 0x2e, 0xe6, 0xd4,
		//ICMP
		0x00, 0x00, 0xa9, 0xcd, 0x28, 0xa3, 0x01, 0x00,
		//DATA
		0xee, 0x7c, 0xf7, 0x90, 0x39, 0x06, 0xd4, 0x41, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51,
		0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51, 0x51 };

static void LittleToBigEndianUint32(unsigned char *pBigEndianBuffer,
		unsigned int nUint32LittleEndianValue) {
	unsigned char nLsb = nUint32LittleEndianValue & 0xff;
	unsigned char nLsbMsbLow = (nUint32LittleEndianValue >> 8) & 0xff;
	unsigned char nLsbMsbHigh = (nUint32LittleEndianValue >> 16) & 0xff;
	unsigned char nMsb = (nUint32LittleEndianValue >> 24) & 0xff;

	pBigEndianBuffer[0] = nMsb;
	pBigEndianBuffer[1] = nLsbMsbHigh;
	pBigEndianBuffer[2] = nLsbMsbLow;
	pBigEndianBuffer[3] = nLsb;
}

static unsigned short BigToLittleEndianUint16(unsigned char *pBigEndianStart) {
	unsigned char nMsb = pBigEndianStart[0];
	unsigned char nLsb = pBigEndianStart[1];

	return (nMsb << 8 | nLsb << 0);
}

static unsigned int BigToLittleEndianUint32(unsigned char *pBigEndianStart) {
	unsigned char nMsb = pBigEndianStart[0];
	unsigned char nMsbLsbHigh = pBigEndianStart[1];
	unsigned char nMsbLsbLow = pBigEndianStart[2];
	unsigned char nLsb = pBigEndianStart[3];

	return (nMsb << 24 | nMsbLsbHigh << 16 | nMsbLsbLow << 8 | nLsb << 0);
}

static void LittleToBigEndianUint16(unsigned char *pBigEndianBuffer,
		unsigned int nUint16LittleEndianValue) {
	unsigned char nLsb = nUint16LittleEndianValue & 0xff;
	unsigned char nMsb = (nUint16LittleEndianValue >> 8) & 0xff;

	pBigEndianBuffer[0] = nMsb;
	pBigEndianBuffer[1] = nLsb;
}

static unsigned short Get2BBIGEndian(const unsigned char *pBuff, int offset) {
	unsigned char upperByte = 0;
	unsigned char lowerByte = 0;

	memcpy(&upperByte, pBuff + offset, 1);
	memcpy(&lowerByte, pBuff + offset + 1, 1);

	return (upperByte << 8 | lowerByte);
}

IPv4Packet::IPv4Packet(unsigned int size) :
		m_PacketSize(size) {
}

IPv4Packet::~IPv4Packet(void) {
	if (0 != m_Packet) {
		delete[] m_Packet;
		m_Packet = 0;
	}
}

void IPv4Packet::ToNetworkByteStream(unsigned char *buffer) {
	if (0 == buffer) {
		LOG_MSG_ERROR("IPv4Packet::ToNetworkByteStream : NULL arguments");
		return;
	}

	memcpy(buffer, m_Packet, GetSize());
}

unsigned int IPv4Packet::GetSrcAddr(void) {
	return BigToLittleEndianUint32(m_Packet + 12);
}

void IPv4Packet::SetSrcAddr(unsigned int addr) {
	LittleToBigEndianUint32(m_Packet + 12, addr);
	RecalculateChecksum();
}

unsigned int IPv4Packet::GetDstAddr(void) {

	return BigToLittleEndianUint32(m_Packet + 16);
}

void IPv4Packet::SetDstAddr(unsigned int addr) {
	LittleToBigEndianUint32(m_Packet + 16, addr);
	RecalculateChecksum();
}

unsigned char IPv4Packet::GetProtocol(void) {
	unsigned char retVal = 0;
	memcpy(&retVal, m_Packet + 9, sizeof(unsigned char));
	return retVal;
}

unsigned short IPv4Packet::GetSrcPort(void) {
	return BigToLittleEndianUint16(m_Packet + 20);
}

unsigned short IPv4Packet::GetDstPort(void) {
	return BigToLittleEndianUint16(m_Packet + 22);
}

void IPv4Packet::SetDstPort(unsigned short port) {

	LittleToBigEndianUint16(m_Packet + 22, port);
	RecalculateChecksum();
}

void IPv4Packet::SetSrcPort(unsigned short port) {
	LittleToBigEndianUint16(m_Packet + 20, port);
	RecalculateChecksum();
}

///////////////////////////////////////////////////////////////////////////////

//Set the third MSB bit of the IPV4_FLAGS_BYTE_OFFSET's byte
void IPv4Packet::SetMF(bool bValue) {

	Byte * pFlags = m_Packet + IPV4_FLAGS_BYTE_OFFSET;
	//clear the bit
	if (true == bValue)	{
		*pFlags |= (0x20);
	} else {
		*pFlags &= (~0x20);
	}
}

///////////////////////////////////////////////////////////////////////////////

void IPv4Packet::RecalculateChecksum(void) {
	RecalculateIPChecksum();
	RecalculateTCPChecksum();
	RecalculateUDPChecksum();
}

void IPv4Packet::RecalculateIPChecksum(void) {
	unsigned short pUint16[100];
	int headerLen = (m_Packet[0] & 0x0F) * 2;
	int checksum = 0;
	unsigned short result = 0;

	memset(&pUint16, 0, 100 * sizeof(unsigned short));

	//clear the IP checksum field first
	memset(m_Packet + 10, 0, sizeof(unsigned short));

	memcpy(&pUint16, m_Packet, headerLen * sizeof(unsigned short));

	for (int i = 0; i < headerLen; i++) {
		checksum += pUint16[i];
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	}

	result = (~checksum & 0xFFFF);

	memcpy(m_Packet + 10, &result, sizeof(unsigned short));

	return;
}

void TCPPacket::RecalculateTCPChecksum(void) {
	unsigned short *pUint16 = new unsigned short[100];
	int checksum = 0;
	int headerLen = 0;
	unsigned short *pTemp = 0;
	unsigned short result = 0;

	headerLen = Get2BBIGEndian(m_Packet, 2) - (m_Packet[0] & 0x0F) * 4;

	memset(pUint16, 0, 100);

	//clear the TCP checksum field first
	memset(m_Packet + 36, 0, sizeof(unsigned short));

	memcpy(pUint16, m_Packet, headerLen * sizeof(unsigned short));

	pTemp = pUint16;

	// Pseudo Header
	pUint16 += 6; // Source IP
	for (int i = 0; i < 4; i++) {
		checksum += pUint16[i];
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	}

	checksum += 0x0600; // TCP Protocol
	checksum += Get2BBIGEndian((unsigned char*) &headerLen, 0);

	pUint16 = pTemp + (m_Packet[0] & 0x0F) * 2;
	headerLen /= 2;
	for (int i = 0; i < headerLen; i++) {
		checksum += pUint16[i];
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	}

	result = (~checksum & 0xFFFF);

	memcpy(m_Packet + 36, &result, sizeof(unsigned short));

	delete[] pTemp;

	return;
}

void UDPPacket::RecalculateUDPChecksum(void) {
	unsigned short *pUint16 = new unsigned short[100];
	int checksum = 0;
	int headerLen = 0;
	unsigned short *pTemp = 0;
	unsigned short result = 0;

	headerLen = Get2BBIGEndian(m_Packet, (m_Packet[0] & 0x0F) * 4 + 4);

	memset(pUint16, 0, 100);

	//clear the UDP checksum field first
	memset(m_Packet + 26, 0, sizeof(unsigned short));

	memcpy(pUint16, m_Packet, headerLen * sizeof(unsigned short));

	pTemp = pUint16;

	// Pseudo Header
	pUint16 += 6; // Source IP
	for (int i = 0; i < 4; i++) {
		checksum += pUint16[i];
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	}

	checksum += 0x1100; // UDP Protocol
	checksum += Get2BBIGEndian((unsigned char*) &headerLen, 0);

	pUint16 = pTemp + (m_Packet[0] & 0x0F) * 2;
	headerLen /= 2;
	for (int i = 0; i < headerLen; i++) {
		checksum += pUint16[i];
		checksum = (checksum & 0xFFFF) + (checksum >> 16);
	}

	result = (~checksum & 0xFFFF);

	memcpy(m_Packet + 26, &result, sizeof(unsigned short));

	delete[] pTemp;
	return;
}

TCPPacket::TCPPacket(void) :
		IPv4Packet(sizeof(TCP_IP_PACKET_DUMP)) {
	size_t length = GetSize();

	m_Packet = new unsigned char[length];
	if (0 == m_Packet) {
		LOG_MSG_ERROR("TCPPacket : packet allocation failed");
		return;
	}

	memcpy(m_Packet, TCP_IP_PACKET_DUMP, length);
}

UDPPacket::UDPPacket(void) :
		IPv4Packet(sizeof(UDP_IP_PACKET_DUMP)) {
	size_t length = GetSize();

	m_Packet = new unsigned char[length];
	if (0 == m_Packet) {
		LOG_MSG_ERROR("UDPPacket : packet allocation failed");
		return;
	}

	memcpy(m_Packet, UDP_IP_PACKET_DUMP, length);
}

ICMPPacket::ICMPPacket(void) :
		IPv4Packet(sizeof(ICMP_IP_PACKET_DUMP)) {
	size_t length = GetSize();

	m_Packet = new unsigned char[length];
	if (0 == m_Packet) {
		LOG_MSG_ERROR("ICMPPacket : packet allocation failed");
		return;
	}

	memcpy(m_Packet, ICMP_IP_PACKET_DUMP, length);
}

unsigned short ICMPPacket::GetSrcPort(void) {
	return 0;
}

unsigned short ICMPPacket::GetDstPort(void) {
	return 0;
}

void ICMPPacket::SetDstPort(unsigned short port) {
	(void) port;
	return;
}

void ICMPPacket::SetSrcPort(unsigned short port) {
	(void) port;
	return;
}
