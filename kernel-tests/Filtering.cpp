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

#include "Filtering.h"

const char* Filtering::DEVICE_NAME = "/dev/ipa";

Filtering::Filtering()
{
	fd = open(DEVICE_NAME, O_RDWR);
	if (0 == fd) {
		printf("Failed opening %s.\n", DEVICE_NAME);
	}
}

Filtering::~Filtering()
{
	close(fd);
}

bool Filtering::DeviceNodeIsOpened()
{
	return fd;
}

bool Filtering::AddFilteringRule(struct ipa_ioc_add_flt_rule const * ruleTable)
{
	int retval = 0;

	retval = ioctl(fd, IPA_IOC_ADD_FLT_RULE, ruleTable);
	if (retval) {
		printf("%s(), failed adding Filtering rule table %p\n", __FUNCTION__, ruleTable);
		return false;
	}

	printf("%s(), Added Filtering rule to table %p\n", __FUNCTION__, ruleTable);
	return true;
}

bool Filtering::AddFilteringRule(struct ipa_ioc_add_flt_rule_v2 const * ruleTable)
{
	int retval = 0;

	retval = ioctl(fd, IPA_IOC_ADD_FLT_RULE_V2, ruleTable);
	if (retval) {
		printf("%s(), failed adding Filtering rule table %p\n", __FUNCTION__, ruleTable);
		return false;
	}

	printf("%s(), Added Filtering rule to table %p\n", __FUNCTION__, ruleTable);
	return true;
}

bool Filtering::DeleteFilteringRule(struct ipa_ioc_del_flt_rule *ruleTable)
{
	int retval = 0;

	retval = ioctl(fd, IPA_IOC_DEL_FLT_RULE, ruleTable);
	if (retval) {
		printf("%s(), failed deleting Filtering rule in table %p\n", __FUNCTION__, ruleTable);
		return false;
	}

	printf("%s(), Deleted Filtering rule in table %p\n", __FUNCTION__, ruleTable);
	return true;
}

bool Filtering::Commit(enum ipa_ip_type ip)
{
	int retval = 0;

	retval = ioctl(fd, IPA_IOC_COMMIT_FLT, ip);
	if (retval) {
		printf("%s(), failed committing Filtering rules.\n", __FUNCTION__);
		return false;
	}

	printf("%s(), Committed Filtering rules to IPA HW.\n", __FUNCTION__);
	return true;
}

bool Filtering::Reset(enum ipa_ip_type ip)
{
	int retval = 0;

	retval = ioctl(fd, IPA_IOC_RESET_FLT, ip);
	retval |= ioctl(fd, IPA_IOC_COMMIT_FLT, ip);
	if (retval) {
		printf("%s(), failed resetting Filtering block.\n", __FUNCTION__);
		return false;
	}

	printf("%s(), Reset command issued to IPA Filtering block.\n", __FUNCTION__);
	return true;
}

