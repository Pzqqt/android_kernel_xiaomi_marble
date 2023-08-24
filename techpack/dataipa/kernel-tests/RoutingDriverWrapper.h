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

#ifndef ROUTING_H_
#define ROUTING_H_

#include <stdint.h>
#include "linux/msm_ipa.h"

using namespace std;

class RoutingDriverWrapper
{
public:
	RoutingDriverWrapper();
	~RoutingDriverWrapper();

	bool AddRoutingRule(struct ipa_ioc_add_rt_rule *ruleTable);
	bool AddRoutingRule(ipa_ioc_add_rt_rule_v2 *ruleTable_v2);
	bool DeleteRoutingRule(struct ipa_ioc_del_rt_rule *ruleTable);

	bool Commit(enum ipa_ip_type ip);
	bool Reset(enum ipa_ip_type ip);

	bool GetRoutingTable(struct ipa_ioc_get_rt_tbl *routingTable);
	bool PutRoutingTable(uint32_t routingTableHandle);

	bool DeviceNodeIsOpened();

private:
	static const char *DEVICE_NAME;
	/* File descriptor of the IPA device node /dev/ipa*/
	int m_fd;
};

#endif

