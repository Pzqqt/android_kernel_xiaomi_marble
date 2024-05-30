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

#include "Constants.h"
#include "Logger.h"
#include "linux/msm_ipa.h"
#include "TestsUtils.h"
#include "TestBase.h"
#include "Pipe.h"
#include "RoutingDriverWrapper.h"
#include "HeaderInsertion.h"
#include "Filtering.h"
#include "IPAFilteringTable.h"


#define NUM_PACKETS 5
#define NUM_PACKETS_FC 4
#define MAX_PACKET_SIZE 1024
#define MAX_PACKETS_IN_MBIM_TESTS 10
#define MAX_PACKETS_IN_NDP 8
#define MAX_NDPS_IN_PACKET 8

/*This class will be the base class of MBIM Aggregation tests.
 *Any method other than the test case itself
 *can be declared in this Fixture thus allowing the derived classes to
 *implement only the test case.
 *All the test of the Aggregation uses one input and one output in DMA mode.
 */
class MBIMAggregationTestFixtureConf11:public TestBase
{
public:
	/*This Constructor will register each instance that it creates.*/
	MBIMAggregationTestFixtureConf11(bool generic_agg);

	virtual int SetupKernelModule();

	/*This method will create and initialize two Pipe object
	 *for the USB (Ethernet) Pipes, one
	 *for as input and the other as output.
	 */
	virtual bool Setup();

	/*This method will destroy the pipes.*/
	virtual bool Teardown();

	virtual bool Run();

	virtual bool AddRules() = 0;

	virtual bool TestLogic() = 0;

	bool AddRules1HeaderAggregation();

	bool AddRules1HeaderAggregation(bool bAggForceClose);

	bool AddRulesDeaggregation();

	bool AddRules1HeaderAggregationTime();

	bool AddRules1HeaderAggregation0Limits();

	bool AddRulesAggDualFC(Pipe *input, Pipe *output1, Pipe *output2);

	bool AddRulesAggDualFcRoutingBased(Pipe *input, Pipe *output1, Pipe *output2);

	/*The client type are set from the
	 * peripheral perspective
	 */
	static Pipe m_IpaToUsbPipeAgg;
	static Pipe m_UsbToIpaPipe;
	static Pipe m_IpaToUsbPipe;
	static Pipe m_UsbToIpaPipeDeagg;
	static Pipe m_IpaToUsbPipeAggTime;
	static Pipe m_IpaToUsbPipeAgg0Limits;

	static RoutingDriverWrapper m_Routing;
	static Filtering m_Filtering;
	static HeaderInsertion m_HeaderInsertion;

protected:
	enum ipa_ip_type m_eIP;
	bool mGenericAgg;
};
