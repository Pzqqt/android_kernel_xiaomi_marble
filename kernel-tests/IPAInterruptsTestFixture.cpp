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

#include "IPAInterruptsTestFixture.h"

/*define the static Pipes which will be used by all derived tests.*/
Pipe IPAInterruptsTestFixture::m_IpaToUsbPipe(IPA_CLIENT_TEST_CONS, IPA_TEST_CONFIGURATION_19);
Pipe IPAInterruptsTestFixture::m_UsbToIpaPipe(IPA_CLIENT_TEST_PROD, IPA_TEST_CONFIGURATION_19);

IPAInterruptsTestFixture::IPAInterruptsTestFixture()
{
	m_testSuiteName.push_back("Interrupts");
}

bool IPAInterruptsTestFixture::Setup()
{
	return true;
}

bool IPAInterruptsTestFixture::Run()
{
	bool bTestResult = true;
	Byte pIpPacket[] = { 0x01, 0x02, 0x03, 0x04 }; //This packet will be sent(It can be replaced by a real IP packet).
	Byte pIpPacketReceive[sizeof(pIpPacket)] = { 0 }; //This buffer will be used in order to store the received packet.

	//Send the raw IP packet(which is a 4 arbitrary bytes) without header addition by the Pipe
	LOG_MSG_DEBUG(
			"Sending packet into the USB pipe(%d bytes)\n", sizeof(pIpPacket));
	int nBytesSent = m_UsbToIpaPipe.Send(pIpPacket, sizeof(pIpPacket));
	if (sizeof(pIpPacket) != nBytesSent) {
		return false;
	}

	//Receive the raw IP packet(which is a 4 arbitrary bytes) without header removal by the Pipe
	LOG_MSG_DEBUG(
			"Reading packet from the USB pipe(%d bytes should be there)\n", sizeof(pIpPacketReceive));
	int nBytesReceived = m_IpaToUsbPipe.Receive(pIpPacketReceive,
			sizeof(pIpPacketReceive));
	if (sizeof(pIpPacketReceive) != nBytesReceived) {
		LOG_MSG_DEBUG("sizes mismatch\n");
		for (int i = 0; i < nBytesReceived && i < (int)sizeof(pIpPacketReceive) ; i++) {
			LOG_MSG_DEBUG("0x%02x\n", pIpPacketReceive[i]);
		}
		return false;
	}
	for (int i = 0; i < nBytesReceived; i++) {
		LOG_MSG_DEBUG("0x%02x\n", pIpPacketReceive[i]);
	}

	//Check that the sent IP packet is equal to the received IP packet.
	LOG_MSG_DEBUG("Checking sent.vs.received packet\n");
	bTestResult &= !memcmp(pIpPacket, pIpPacketReceive, sizeof(pIpPacket));

	return bTestResult;
}

bool IPAInterruptsTestFixture::Teardown()
{
	/* unregister the test framework suspend handler */
	RegSuspendHandler(false, false, 0);

	/*The Destroy method will close the inode.*/
	m_IpaToUsbPipe.Destroy();
	m_UsbToIpaPipe.Destroy();
	ConfigureScenario(-1);
	return true;
}
