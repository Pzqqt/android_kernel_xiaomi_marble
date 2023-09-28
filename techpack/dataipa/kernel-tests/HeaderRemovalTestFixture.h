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

#ifndef _IPA_LINUX_TESTS_HR_TEST_FIXTURE_H_
#define _IPA_LINUX_TESTS_HR_TEST_FIXTURE_H_


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "Constants.h"
#include "Logger.h"
#include "linux/msm_ipa.h"
#include "TestsUtils.h"
#include "TestBase.h"
#include "Pipe.h"
#include "RoutingDriverWrapper.h"
#include "Filtering.h"

/*This class will be the base class of all HeaderRemoval tests.
 *Any method other than the test case itself can
 *be declared in this Fixture thus allowing the derived classes to
 *implement only the test case.
 *All the test of the HeaderRemovalTestFixture
 *uses one input and two output.
 */
class HeaderRemovalTestFixture:public TestBase
{
public:
	/*This Constructor will register each instance
	 * that it creates.*/
	HeaderRemovalTestFixture();

	/*This method will create and initialize two Pipe object for the USB
	 *(Ethernet) Pipes, one as input and the other as output.
	 */
	virtual bool Setup();

	/*This method will destroy the pipes.*/
	virtual bool Teardown();

	/*The client type are set from the peripheral perspective
	 *(TODO Pipe:in case the Driver will change its perspective
	 *of ipa_connect this should be changed).
	 */
	static Pipe m_A2NDUNToIpaPipe;
	/*from the test application into the IPA(DMUX header)*/
	static Pipe m_IpaToUsbPipe;
	/*from the IPA back to the test application(Ethernet header)*/
	static Pipe m_IpaToA2NDUNPipe;
	/*from the IPA back to the test application(DMUX header)*/
	static Pipe m_IpaToQ6LANPipe;

	static RoutingDriverWrapper   m_routing;
	static Filtering m_filtering;

protected:
	unsigned char *CreateA2NDUNPacket(unsigned int magicNumber,
			unsigned int ID,
			string sPayloadFileName,
			unsigned int *nTotalLength);
	bool  SetIPATablesToPassAllToSpecificClient(
			enum ipa_client_type nClientTypeSrc,
			enum ipa_client_type nClientTypeDst);
	bool  SetFilterTableToPassAllToSpecificClient(
			enum ipa_client_type nClientType);
	bool  SetRoutingTableToPassAllToSpecificClient(
			enum ipa_client_type nClientType);
	bool  SetHeaderInsertionTableAddEmptyHeaderForTheClient(
			enum ipa_client_type nClientType);
	bool  CreateBypassRoutingTablesIPv4(
			const char *bypass0,
			enum ipa_client_type nClientType
			);
	bool ConfigureFilteringBlockWithMetaDataEq(
			enum ipa_client_type nClientType,
			unsigned int nMetaData,
			unsigned int nMetaDataMask);
};

#endif

