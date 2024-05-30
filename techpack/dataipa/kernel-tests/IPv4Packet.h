/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

#ifndef __IPA_TESTS_IPV4_PACKET__H__
#define __IPA_TESTS_IPV4_PACKET__H__

namespace IPA
{

#define IPV4_FLAGS_BYTE_OFFSET 6

/**
  @brief
  IPv4Packet abstract class.

  @details
  new setters should call to RecalculateChecksum();
  */
class IPv4Packet
{
public:

	IPv4Packet(unsigned int size);

	virtual ~IPv4Packet(void) = 0;

	void ToNetworkByteStream(unsigned char *addr);

	unsigned int GetSrcAddr(void);

	void SetSrcAddr(unsigned int addr);

	unsigned int GetDstAddr(void);

	void SetDstAddr(unsigned int addr);

	unsigned char GetProtocol(void);

	unsigned int GetSize(void)
	{
		return m_PacketSize;
	}

	virtual unsigned short GetSrcPort(void);

	virtual unsigned short GetDstPort(void);

	virtual void SetDstPort(unsigned short port);

	virtual void SetSrcPort(unsigned short port);

	void SetMF(bool bValue);

protected:

	virtual void RecalculateTCPChecksum(void)  {}

	virtual void RecalculateUDPChecksum(void)  {}

	virtual void RecalculateICMPChecksum(void) {}

	unsigned char *m_Packet;

private:

	unsigned int  m_PacketSize;

	void RecalculateChecksum(void);

	void RecalculateIPChecksum(void);
};

/**
  @brief
  TCPPacket implementation.

  @details
  new setters should call to RecalculateChecksum();
  */
class TCPPacket:public IPv4Packet
{
public:

	TCPPacket(void);

	~TCPPacket(void) {}

protected:

	virtual void RecalculateTCPChecksum(void);
};

/**
  @brief
  UDPPacket implementation.

  @details
  new setters should call to RecalculateChecksum();
  */
class UDPPacket:public IPv4Packet
{
public:
	UDPPacket(void);

	~UDPPacket(void) {}

protected:

	virtual void RecalculateUDPChecksum(void);
};

/**
  @brief
  ICMPPacket implementation.

  @details
  new setters should call to RecalculateChecksum();
  */
class ICMPPacket:public IPv4Packet
{
public:
	ICMPPacket(void);

	~ICMPPacket(void) {}

	virtual unsigned short GetSrcPort(void);

	virtual unsigned short GetDstPort(void);

	virtual void SetDstPort(unsigned short port);

	virtual void SetSrcPort(unsigned short port);

protected:

	/**
	@brief
	RecalculateICMPChecksum method.

	@details
	ICMP checksum recalculation is not needed now
	and by so is not implemented yet
	TODO: implement if needed
   */
	virtual void RecalculateICMPChecksum(void)
	{
		return;
	}
};

} /* namespace IPA */

#endif
