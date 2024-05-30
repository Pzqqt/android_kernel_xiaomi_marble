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

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>

#include "RoutingDriverWrapper.h"
#include "TestsUtils.h"

const char* RoutingDriverWrapper::DEVICE_NAME = "/dev/ipa";

RoutingDriverWrapper::RoutingDriverWrapper()
{
	m_fd = open(DEVICE_NAME, O_RDWR);
	if (0 == m_fd) {
		printf("Failed opening %s.\n", DEVICE_NAME);
	}
}

RoutingDriverWrapper::~RoutingDriverWrapper()
{
	close(m_fd);
}

bool RoutingDriverWrapper::DeviceNodeIsOpened()
{
	int res = fcntl(m_fd, F_GETFL);

	if (m_fd > 0 && res >=0)
		return true;
	else
		return false;

}

bool RoutingDriverWrapper::AddRoutingRule(struct ipa_ioc_add_rt_rule *ruleTable)
{
	int retval = 0;

	if (!DeviceNodeIsOpened())
		return false;

	retval = ioctl(m_fd, IPA_IOC_ADD_RT_RULE, ruleTable);
	if (retval) {
		printf("%s(), failed adding routing rule table %p\n", __FUNCTION__, ruleTable);
		return false;
	}

	printf("%s(), Added routing rule table %p\n", __FUNCTION__, ruleTable);
	return true;
}

bool RoutingDriverWrapper::AddRoutingRule(struct ipa_ioc_add_rt_rule_v2 *ruleTable_v2)
{
	int retval = 0;

	if (!DeviceNodeIsOpened())
		return false;

	retval = ioctl(m_fd, IPA_IOC_ADD_RT_RULE_V2, ruleTable_v2);
	if (retval) {
		printf("%s(), failed adding routing rule table %p\n", __FUNCTION__, ruleTable_v2);
		return false;
	}

	printf("%s(), Added routing rule table %p\n", __FUNCTION__, ruleTable_v2);
	return true;
}

bool RoutingDriverWrapper::DeleteRoutingRule(struct ipa_ioc_del_rt_rule *ruleTable)
{
	int retval = 0;

	if (!DeviceNodeIsOpened())
		return false;

	retval = ioctl(m_fd, IPA_IOC_DEL_RT_RULE, ruleTable);
	if (retval) {
		printf("%s(), failed deleting routing rule table %p\n", __FUNCTION__, ruleTable);
		return false;
	}

	printf("%s(), Deleted routing rule table %p\n", __FUNCTION__, ruleTable);
	return true;
}

bool RoutingDriverWrapper::Commit(enum ipa_ip_type ip)
{
	int retval = 0;

	if (!DeviceNodeIsOpened())
		return false;

	retval = ioctl(m_fd, IPA_IOC_COMMIT_RT, ip);
	if (retval) {
		printf("%s(), failed commiting routing rules.\n", __FUNCTION__);
		return false;
	}

	printf("%s(), Commited routing rules to IPA HW.\n", __FUNCTION__);
	return true;
}

bool RoutingDriverWrapper::Reset(enum ipa_ip_type ip)
{
	int retval = 0;

	if (!DeviceNodeIsOpened())
		return false;

	retval = ioctl(m_fd, IPA_IOC_RESET_RT, ip);
	retval |= ioctl(m_fd, IPA_IOC_COMMIT_RT, ip);
	if (retval) {
		printf("%s(), failed reseting routing block.\n", __FUNCTION__);
		return false;
	}

	printf("%s(), Reset command issued to IPA routing block.\n", __FUNCTION__);
	return true;
}

bool RoutingDriverWrapper::GetRoutingTable(struct ipa_ioc_get_rt_tbl *routingTable)
{
	int retval = 0;

	if (!DeviceNodeIsOpened())
		return false;

	retval = ioctl(m_fd, IPA_IOC_GET_RT_TBL, routingTable);
	if (retval) {
		printf("%s(), IPA_IOCTL_GET_RT_TBL ioctl failed, routingTable =0x%p, retval=0x%x.\n", __FUNCTION__, routingTable, retval);
		return false;
	}

	printf("%s(), IPA_IOCTL_GET_RT_TBL ioctl issued to IPA routing block.\n", __FUNCTION__);
	return true;
}

bool RoutingDriverWrapper::PutRoutingTable(uint32_t routingTableHandle)
{
	int retval = 0;

	if (!DeviceNodeIsOpened())
		return false;

	retval = ioctl(m_fd, IPA_IOC_PUT_RT_TBL, routingTableHandle);
	if (retval) {
		printf("%s(), IPA_IOCTL_PUT_RT_TBL ioctl failed.\n", __FUNCTION__);
		return false;
	}

	printf("%s(), IPA_IOCTL_PUT_RT_TBL ioctl issued to IPA routing block.\n", __FUNCTION__);
	return true;
}

