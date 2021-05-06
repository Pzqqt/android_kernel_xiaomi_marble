/*
 * Copyright (c) 2017,2020 The Linux Foundation. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "hton.h" /* for htonl*/
#include "DataPathTestFixture.h"
#include "Constants.h"
#include "TestsUtils.h"
#include "linux/msm_ipa.h"

#define PACKET_SIZE ((10)*(4))

class IpaTxDpTest:public DataPathTestFixture {

public:
	IpaTxDpTest() {
		m_name = "IpaTxDpTest";
		m_description = "Sending one SKB via ipa_tx_dp() and checking"
				"if it was received";
		m_runInRegression = true;
	}

	bool TestLogic() {
		LOG_MSG_DEBUG("Entering %s\n", __func__);
		int ret;
		unsigned char *input, *output;
		unsigned char pkt[PACKET_SIZE] = {
			0x59, 0x61, 0x6e, 0x69,
			0x76, 0x5f, 0x48 ,0x61,
			0x73 ,0x62 ,0x61 ,0x6e,
			0x69 ,0x5f ,0x54 ,0x68,
			0x65 ,0x5f ,0x47 ,0x72,
			0x65 ,0x61 ,0x74 ,0x16,
			0x32 ,0x49 ,0x0c ,0x3f,
			0x37 ,0x23 ,0x6d ,0x15,
			0x50 ,0x10 ,0x3f ,0xbd,
			0xcc ,0xd8 ,0x00, 0x00
		};

		input = (unsigned char *)malloc(PACKET_SIZE);
		if(!input) {
			LOG_MSG_ERROR("Error in allocation\n");
			goto fail;
		}

		output = (unsigned char *)malloc(PACKET_SIZE);
		if(!output) {
			LOG_MSG_ERROR("Error in allocation\n");
			free(input);
			goto fail;
		}

		memcpy(input, pkt, PACKET_SIZE);
		LOG_MSG_DEBUG("Sending packet through ipa_tx_dp() in func %s\n", __func__);
		ret = m_IpaDriverPipe.Send(input, PACKET_SIZE);

		if (ret != PACKET_SIZE) {
			LOG_MSG_ERROR(
				"Amount of bits sent are: %d instead of %d\nExiting..\n"
				, ret
				, PACKET_SIZE);
			free(input);
			free(output);
			goto fail;
		}

		ret = m_FromIPAPipe.Receive(output, PACKET_SIZE);
		if (ret != PACKET_SIZE) {
			LOG_MSG_ERROR(
				"Amount of bits sent are: %d instead of %d\nExiting..\n",
				ret,
				PACKET_SIZE);
			free(input);
			free(output);
			goto fail;
		}

		LOG_MSG_INFO("Input buff:\n");
		print_buff(input, PACKET_SIZE);
		LOG_MSG_INFO("Output buff:\n");
		print_buff(output, PACKET_SIZE);
		if (memcmp(input,output, PACKET_SIZE)) {
			free(input);
			free(output);
			return false;
		}

		free(input);
		free(output);
		return true;
fail:
		return false;
	}
};

class IpaTxDpMultipleTest:public DataPathTestFixture {

public:
	IpaTxDpMultipleTest() {
		m_name = "IpaTxDpMultipleTest";
		m_description = "Sending multiple SKB via ipa_tx_dp() and checking"
				"if it was received";
		m_runInRegression = false;
	}

	bool TestLogic() {
		int packet_to_send = 10;
		int loop_size = 100;
		int i;
		int j;

		LOG_MSG_DEBUG("Entering %s\n", __func__);
		int ret;
		unsigned char *input, *output;
		unsigned char pkt[PACKET_SIZE] = {
			0x59, 0x61, 0x6e, 0x69,
			0x76, 0x5f, 0x48 ,0x61,
			0x73 ,0x62 ,0x61 ,0x6e,
			0x69 ,0x5f ,0x54 ,0x68,
			0x65 ,0x5f ,0x47 ,0x72,
			0x65 ,0x61 ,0x74 ,0x16,
			0x32 ,0x49 ,0x0c ,0x3f,
			0x37 ,0x23 ,0x6d ,0x15,
			0x50 ,0x10 ,0x3f ,0xbd,
			0xcc ,0xd8 ,0x00, 0x00
		};

		input = (unsigned char *)malloc(PACKET_SIZE);
		if(!input) {
			LOG_MSG_ERROR("Error in allocation\n");
			goto fail;
		}

		output = (unsigned char *)malloc(PACKET_SIZE);
		if(!output) {
			LOG_MSG_ERROR("Error in allocation\n");
			free(input);
			goto fail;
		}

		memcpy(input, pkt, PACKET_SIZE);
		for (i = 0; i < loop_size; i++) {
			for (j = 0; j < packet_to_send; j++) {
				input[0] = i;
				input[1] = j;
				LOG_MSG_DEBUG("Sending packet through ipa_tx_dp() in func %s\n", __func__);
				ret = m_IpaDriverPipe.Send(input, PACKET_SIZE);
				if (ret != PACKET_SIZE) {
					LOG_MSG_ERROR(
						"Amount of bits sent are: %d instead of %d\nExiting..\n"
						, ret
						, PACKET_SIZE);
					free(input);
					free(output);
					goto fail;
				}
			}

			for (j = 0; j < packet_to_send; j++) {
				ret = m_FromIPAPipe.Receive(output, PACKET_SIZE);
				if (ret != PACKET_SIZE) {
					LOG_MSG_ERROR(
						"Amount of bits sent are: %d instead of %d\nExiting..\n",
						ret,
						PACKET_SIZE);
					free(input);
					free(output);
					goto fail;
				}
				input[0] = i;
				input[1] = j;
				LOG_MSG_INFO("Input buff:\n");
				print_buff(input, PACKET_SIZE);
				LOG_MSG_INFO("Output buff:\n");
				print_buff(output, PACKET_SIZE);
				if (memcmp(input,output, PACKET_SIZE)) {
					free(input);
					free(output);
					LOG_MSG_ERROR("Failed in buffers comparison");
					return false;
				}
			}
		}

		free(input);
		free(output);
		return true;
fail:
		return false;
	}
};

class IPAToAppsTest:public DataPathTestFixture {

public:
	IPAToAppsTest() {
		m_name = "IPAToApps";
		m_description = "Sending one SKB via USB_PROD pipe and checking"
				"if it was received";
		m_runInRegression = true;
	}

	bool TestLogic() {
		int ret;
		unsigned char *input, *output;
		unsigned char pkt[PACKET_SIZE] = {
			0x59, 0x61, 0x6e, 0x69,
			0x76, 0x5f, 0x48 ,0x61,
			0x73 ,0x62 ,0x61 ,0x6e,
			0x69 ,0x5f ,0x54 ,0x68,
			0x65 ,0x5f ,0x47 ,0x72,
			0x65 ,0x61 ,0x74 ,0x16,
			0x32 ,0x49 ,0x0c ,0x3f,
			0x37 ,0x23 ,0x6d ,0x15,
			0x50 ,0x10 ,0x3f ,0xbd,
			0xcc ,0xd8 ,0x00, 0x00
		};

		input = (unsigned char *)malloc(PACKET_SIZE);
		if(!input) {
			LOG_MSG_ERROR("Error in allocation\n");
			goto fail;
		}
		output = (unsigned char *)malloc(PACKET_SIZE);
		if(!output) {
			LOG_MSG_ERROR("Error in allocation\n");
			free(input);
			goto fail;
		}
		memcpy(input, pkt, PACKET_SIZE);

		LOG_MSG_DEBUG("Sending packet through USB_PROD pipe in func %s\n", __func__);
		ret = m_ToIpaPipe.Send(input, PACKET_SIZE);
		if (ret != PACKET_SIZE) {
			LOG_MSG_ERROR(
				"Amount of bits sent are: %d instead of %d\nExiting..\n",
				ret,
				PACKET_SIZE);
			goto fail;
		}
		LOG_MSG_DEBUG("Reading packet through Dummy Endpoint pipe in func %s\n", __func__);
		ret = m_IpaDriverPipe.Receive(output, PACKET_SIZE);
		if (ret != 0) {
			LOG_MSG_ERROR("Failed in reading buffer. %d error", ret);
			free(input);
			free(output);
			goto fail;
		}
		LOG_MSG_DEBUG("SKB original packet:\n");
		print_buff(input, PACKET_SIZE);
		LOG_MSG_DEBUG("SKB received packet:\n");
		print_buff(output, PACKET_SIZE);
		if (memcmp(input,output, PACKET_SIZE)) {
			free(input);
			free(output);
			return false;
		}
		free(input);
		free(output);
		return true;
fail:
		return false;
	}
};

class IPAToAppsMultipleTest:public DataPathTestFixture {

public:
	IPAToAppsMultipleTest() {
		m_name = "IPAToAppsMultipleTest";
		m_description = "Sending multiple SKB via USB_PROD pipe and checking"
				"if they was received";
		m_runInRegression = false;
	}

	bool TestLogic() {
		int packet_to_send = 10;
		int loop_size = 100;
		int i;
		int j;
		int ret;
		unsigned char *input, *output;
		unsigned char pkt[PACKET_SIZE] = {
			0x59, 0x61, 0x6e, 0x69,
			0x76, 0x5f, 0x48 ,0x61,
			0x73 ,0x62 ,0x61 ,0x6e,
			0x69 ,0x5f ,0x54 ,0x68,
			0x65 ,0x5f ,0x47 ,0x72,
			0x65 ,0x61 ,0x74 ,0x16,
			0x32 ,0x49 ,0x0c ,0x3f,
			0x37 ,0x23 ,0x6d ,0x15,
			0x50 ,0x10 ,0x3f ,0xbd,
			0xcc ,0xd8 ,0x00, 0x00
		};

		input = (unsigned char *)malloc(PACKET_SIZE);
		if(!input) {
			LOG_MSG_ERROR("Error in allocation\n");
			goto fail;
		}
		output = (unsigned char *)malloc(PACKET_SIZE);
		if(!output) {
			LOG_MSG_ERROR("Error in allocation\n");
			free(input);
			goto fail;
		}
		memcpy(input, pkt, PACKET_SIZE);
		for (i = 0; i < loop_size; i++) {
			for (j = 0; j < packet_to_send; j++) {
				input[0] = i;
				input[1] = j;

				LOG_MSG_DEBUG("Sending packet through USB_PROD pipe in func %s\n", __func__);
				ret = m_ToIpaPipe.Send(input, PACKET_SIZE);
				if (ret != PACKET_SIZE) {
					LOG_MSG_ERROR(
						"Amount of bits sent are: %d instead of %d\nExiting..\n",
						ret,
						PACKET_SIZE);
					free(input);
					free(output);
					goto fail;
				}
			}

			for (j = 0; j < packet_to_send; j++) {
				input[0] = i;
				input[1] = j;
				LOG_MSG_DEBUG("Reading packet through Dummy Endpoint pipe in func %s\n", __func__);
				ret = m_IpaDriverPipe.Receive(output, PACKET_SIZE);
				if (ret != 0) {
					LOG_MSG_ERROR("Failed in reading buffer. %d error", ret);
					free(input);
					free(output);
					goto fail;
				}
				LOG_MSG_DEBUG("SKB original packet:\n");
				print_buff(input, PACKET_SIZE);
				LOG_MSG_DEBUG("SKB received packet:\n");
				print_buff(output, PACKET_SIZE);
				if (memcmp(input,output, PACKET_SIZE)) {
					free(input);
					free(output);
					LOG_MSG_ERROR("Failed in buffers comparison");
					return false;
				}
			}
		}
		free(input);
		free(output);
		return true;
fail:
		return false;
	}
};


static IpaTxDpTest ipaTxDpTest;
static IpaTxDpMultipleTest ipaTxDpMultipleTest;
static IPAToAppsTest ipaToApps;
static IPAToAppsMultipleTest iPAToAppsMultipleTestApps;
