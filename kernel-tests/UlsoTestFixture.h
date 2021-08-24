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
#ifndef ULSOTESTFIXTURE_H_
#define ULSOTESTFIXTURE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <linux/if_ether.h>

#include "Constants.h"
#include "Logger.h"
#include "linux/msm_ipa.h"
#include "TestsUtils.h"
#include "TestBase.h"
#include "network_traffic/UlsoPacket.h"

using std::cout;
using std::endl;
using std::string;

class UlsoTestFixture: public TestBase {

public:

	UlsoTestFixture(){
		m_testSuiteName.push_back("ULSO");
		memset(m_sendBuf, 0, sizeof(m_sendBuf));
		memset(m_receiveBuf, 0, sizeof(m_receiveBuf));
		memset(m_segmentBuf, 0, sizeof(m_segmentBuf));
	}

	virtual bool Setup() {
		if(!setupKernelModule()){
			return false;
		}
		m_producer.Open(INTERFACE0_TO_IPA_DATA_PATH, INTERFACE0_FROM_IPA_DATA_PATH);
		m_consumer.Open(INTERFACE1_TO_IPA_DATA_PATH, INTERFACE1_FROM_IPA_DATA_PATH);
		return true;
	}

	virtual bool Teardown(){
		m_producer.Close();
		m_consumer.Close();
		return true;
	}

	virtual bool Run() = 0;

protected:

	virtual void configFromEp(struct test_ipa_ep_cfg *ep_cfg){
		return;
	}

	size_t clearPipe(){
		cout << "In clearPipe" << endl;
		if(m_consumer.setReadNoBlock() == -1){
			cout << "Error: setReadNoBlock returned -1" << endl;
			return 0;
		}
		size_t recievedBytes = m_consumer.ReceiveSingleDataChunk(m_receiveBuf, UlsoPacket<>::maxSize);
		size_t totalReceivedBytes = recievedBytes;
		if(recievedBytes > 0){
			unsigned count = 1;
			while(recievedBytes){
				cout << "Receive #" << count << endl;
				printBuf(m_receiveBuf, recievedBytes, string("Rceived ")
					+ std::to_string(recievedBytes) + string(" Bytes:"));
				recievedBytes = m_consumer.ReceiveSingleDataChunk(m_receiveBuf, UlsoPacket<>::maxSize);
				totalReceivedBytes += recievedBytes;
				count ++;
			}
		} else {
			cout << "There were no bytes left in the pipe" << endl;
		}
		m_consumer.clearReadNoBlock();
		return totalReceivedBytes;
	}

	bool fail(size_t sendSize=0, size_t totalSegmentsSize=0, size_t recievedBytes=0){
		printBuf(m_sendBuf, sendSize, "Sent:");
		printBuf(m_receiveBuf, recievedBytes, string("Rceived ") + std::to_string(recievedBytes) + string(" Bytes:"));
		printBuf(m_segmentBuf, totalSegmentsSize, string("Expected to receive ") + std::to_string(totalSegmentsSize) + string(" Bytes:"));
		clearPipe();
		return false;
	}

	virtual int setupKernelModule(bool en_status = 0){
		struct ipa_channel_config from_ipa_channels[1];
		struct test_ipa_ep_cfg from_ipa_cfg[1];
		struct ipa_channel_config to_ipa_channels[1];
		struct test_ipa_ep_cfg to_ipa_cfg[1];

		struct ipa_test_config_header header = {0};
		struct ipa_channel_config *to_ipa_array[1];
		struct ipa_channel_config *from_ipa_array[1];

		/* From ipa configurations - 1 pipe */
		memset(&from_ipa_cfg[0], 0, sizeof(from_ipa_cfg[0]));
		from_ipa_cfg[0].ulso.is_ulso_pipe = true;
		configFromEp(&from_ipa_cfg[0]);

		prepare_channel_struct(&from_ipa_channels[0],
				header.from_ipa_channels_num++,
				IPA_CLIENT_TEST_CONS,
				(void *)&from_ipa_cfg[0],
				sizeof(from_ipa_cfg[0]),
				en_status);
		from_ipa_array[0] = &from_ipa_channels[0];

		/* To ipa configurations - 1 pipe */
		memset(&to_ipa_cfg[0], 0, sizeof(to_ipa_cfg[0]));
		to_ipa_cfg[0].ulso.ipid_min_max_idx = 0;
		to_ipa_cfg[0].ulso.is_ulso_pipe = true;
		prepare_channel_struct(&to_ipa_channels[0],
				header.to_ipa_channels_num++,
				IPA_CLIENT_TEST_PROD,
				(void *)&to_ipa_cfg[0],
				sizeof(to_ipa_cfg[0]));
		to_ipa_array[0] = &to_ipa_channels[0];

		prepare_header_struct(&header, from_ipa_array, to_ipa_array);

		return GenericConfigureScenario(&header, true);
	}

protected:

	static void printBuf(uint8_t* buf, size_t bufSize, string title=""){
		if(bufSize == 0){
			return;
		}
		cout << title << endl << std::hex;
		for (size_t i = 0; i < bufSize-1; i++)
			cout << std::setfill('0') << std::setw(2) << static_cast<int>(buf[i]) << " ";
		cout << std::setfill('0') << std::setw(2) << static_cast<int>(buf[bufSize-1]) << std::dec << endl;
	}

public:

	InterfaceAbstraction m_producer;
	InterfaceAbstraction m_consumer;
	uint8_t m_sendBuf[UlsoPacket<>::maxSize];
	uint8_t m_receiveBuf[UlsoPacket<>::maxSize];
	uint8_t m_segmentBuf[UlsoPacket<>::maxSize];
};
#endif /* ULSOTESTFIXTURE_H_ */
