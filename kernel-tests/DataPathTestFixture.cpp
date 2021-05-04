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

#include "DataPathTestFixture.h"

Pipe DataPathTestFixture::m_FromIPAPipe(IPA_CLIENT_TEST_CONS,
					IPA_TEST_CONFIGURATION_18);
Pipe DataPathTestFixture::m_ToIpaPipe(IPA_CLIENT_TEST_PROD,
					IPA_TEST_CONFIGURATION_18);
Pipe DataPathTestFixture::m_IpaDriverPipe(IPA_CLIENT_TEST2_PROD,
					IPA_TEST_CONFIGURATION_18);


RoutingDriverWrapper DataPathTestFixture::m_Routing;
Filtering DataPathTestFixture::m_Filtering;
HeaderInsertion DataPathTestFixture::m_HeaderInsertion;

DataPathTestFixture::DataPathTestFixture()
{
	m_testSuiteName.push_back("DataPath");
	Register(*this);
}

bool DataPathTestFixture::Setup()
{
	bool bRetVal = true;

	/*Set the configuration to support USB->IPA and IPA->USB pipes.*/
	ConfigureScenario(IPA_TEST_CONFIGURATION_18);

	bRetVal &= m_ToIpaPipe.Init();
	bRetVal &= m_FromIPAPipe.Init();
	bRetVal &= m_IpaDriverPipe.Init();

	if (!m_Routing.DeviceNodeIsOpened()) {
		LOG_MSG_ERROR(
			"Routing block is not ready for immediate commands!\n");
		return false;
	}
	if (!m_Filtering.DeviceNodeIsOpened()) {
		LOG_MSG_ERROR(
			"Filtering block is not ready for immediate commands!\n");
		return false;
	}
	if (!m_HeaderInsertion.DeviceNodeIsOpened())
	{
		LOG_MSG_ERROR("Header Insertion block is not ready for immediate commands!\n");
		return false;
	}\
	/*resetting this component will reset both Routing and Filtering tables*/
	m_HeaderInsertion.Reset();

	return bRetVal;
}

bool DataPathTestFixture::Teardown()
{
	/*The Destroy method will close the inode.*/
	m_FromIPAPipe.Destroy();
	m_ToIpaPipe.Destroy();
	m_IpaDriverPipe.Destroy();
	return true;
}

bool DataPathTestFixture::Run()
{
	LOG_MSG_DEBUG("Entering Function");

	if (!TestLogic()) {
		LOG_MSG_ERROR(
			"Test failed, Input and expected output mismatch.");
		return false;
	}

	LOG_MSG_DEBUG("Leaving Function (Returning True)");
	return true;
}


