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
#include <cstring>

#include "Filtering.h"
#include "Constants.h"

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

bool Filtering::AddEoGREMap(
	enum ipa_ip_type     iptype,
	union ipa_ip_params& ipaddrs )
{
	struct ipa_ioc_eogre_info einfo;

	int retval = 0;

	memset(&einfo, 0, sizeof(einfo));

	einfo.ipgre_info.iptype = iptype;

	if ( einfo.ipgre_info.iptype == IPA_IP_v4  )
	{
		einfo.ipgre_info.ipv4_src = ipaddrs.v4.src_addr;
		einfo.ipgre_info.ipv4_dst = ipaddrs.v4.dst_addr;
	}
	else
	{
		memcpy(
			&einfo.ipgre_info.ipv6_src,
			&ipaddrs.v6.src_addr,
			sizeof(einfo.ipgre_info.ipv6_src));
		memcpy(
			&einfo.ipgre_info.ipv6_dst,
			&ipaddrs.v6.dst_addr,
			sizeof(einfo.ipgre_info.ipv6_dst));
	}

	/*
	 * VLAN_ID_VAL and PCP_VAL match what's embedded in
	 * Eth2IPv4Packet802_1Q_tag in TestsUtils.cpp
	 */
	einfo.map_info.vlan[0]          = VLAN_ID_VAL;
	einfo.map_info.dscp[0][PCP_VAL] = DSCP_VAL;
	einfo.map_info.num_vlan         = 1;

	retval = ioctl(fd, IPA_IOC_ADD_EoGRE_MAPPING, &einfo);

	if (retval) {
		printf("%s(), Failed adding eogre mapping data\n", __FUNCTION__);
		return false;
	}

	printf("%s(), Success adding eogre mapping data\n", __FUNCTION__);

	return true;
}

bool Filtering::ClrEoGREMap()
{
	int retval = 0;

	retval = ioctl(fd, IPA_IOC_DEL_EoGRE_MAPPING, 0);

	if (retval) {
		printf("%s(), Failed clearing eogre mapping data\n", __FUNCTION__);
		return false;
	}

	printf("%s(), Success clearing eogre mapping data\n", __FUNCTION__);

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

