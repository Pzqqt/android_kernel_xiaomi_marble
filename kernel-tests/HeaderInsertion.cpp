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

#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdio.h>

#include "HeaderInsertion.h"

/*All interaction through the driver are
 * made through this inode.
 */
static const char* DEVICE_NAME = "/dev/ipa";

#define LOG_IOCTL_RETURN_VALUE(nRetVal) \
		printf("%s()- %s\n", __func__, \
		(-1 == nRetVal) ? "Fail" : "Success");

HeaderInsertion::HeaderInsertion()
{
	m_fd = open(DEVICE_NAME, O_RDWR);
	if (-1 == m_fd)
	{
		printf(
			"Failed to open %s in HeaderInsertion test application constructor.\n",
			DEVICE_NAME);
	}
}

HeaderInsertion::~HeaderInsertion()
{
	if (-1 != m_fd)
	{
		close(m_fd);
	}
}

bool HeaderInsertion::DeviceNodeIsOpened()
{
	return (-1 != m_fd);
}

bool HeaderInsertion::AddHeader(struct ipa_ioc_add_hdr *pHeaderTableToAdd)
{
	int nRetVal = 0;
	/*call the Driver ioctl in order to add header*/
	nRetVal = ioctl(m_fd, IPA_IOC_ADD_HDR, pHeaderTableToAdd);
	LOG_IOCTL_RETURN_VALUE(nRetVal);
	return (-1 != nRetVal);
}

bool HeaderInsertion::DeleteHeader(struct ipa_ioc_del_hdr *pHeaderTableToDelete)
{
	int nRetVal = 0;
	/*call the Driver ioctl in order to remove header*/
	nRetVal = ioctl(m_fd, IPA_IOC_DEL_HDR , pHeaderTableToDelete);
	LOG_IOCTL_RETURN_VALUE(nRetVal);
	return (-1 != nRetVal);
}

bool HeaderInsertion::AddProcCtx(struct ipa_ioc_add_hdr_proc_ctx *procCtxTable)
{
	int retval = 0;

	retval = ioctl(m_fd, IPA_IOC_ADD_HDR_PROC_CTX, procCtxTable);
	if (retval) {
		printf("%s(), failed adding ProcCtx rule table %p\n", __FUNCTION__, procCtxTable);
		return false;
	}

	printf("%s(), Added ProcCtx rule to table %p\n", __FUNCTION__, procCtxTable);
	return true;
}

bool HeaderInsertion::DeleteProcCtx(struct ipa_ioc_del_hdr_proc_ctx *procCtxTable)
{
	int retval = 0;

	retval = ioctl(m_fd, IPA_IOC_DEL_HDR_PROC_CTX, procCtxTable);
	if (retval) {
		printf("%s(), failed deleting ProcCtx rule in table %p\n", __FUNCTION__, procCtxTable);
		return false;
	}

	printf("%s(), Deleted ProcCtx rule in table %p\n", __FUNCTION__, procCtxTable);
	return true;
}

bool HeaderInsertion::Commit()
{
	int nRetVal = 0;
	nRetVal = ioctl(m_fd, IPA_IOC_COMMIT_HDR);
	LOG_IOCTL_RETURN_VALUE(nRetVal);
	return true;
}

bool HeaderInsertion::Reset()
{
	int nRetVal = 0;

	nRetVal = ioctl(m_fd, IPA_IOC_RESET_HDR);
	nRetVal |= ioctl(m_fd, IPA_IOC_COMMIT_HDR);
	LOG_IOCTL_RETURN_VALUE(nRetVal);
	return true;
}

bool HeaderInsertion::GetHeaderHandle(struct ipa_ioc_get_hdr *pHeaderStruct)
{
	int retval = 0;

	if (!DeviceNodeIsOpened())
		return false;

	retval = ioctl(m_fd, IPA_IOC_GET_HDR, pHeaderStruct);
	if (retval) {
		printf(
		"%s(), IPA_IOC_GET_HDR ioctl failed, routingTable =0x%p, retval=0x%x.\n"
		, __func__,
		pHeaderStruct,
		retval);
		return false;
	}

	printf(
	"%s(), IPA_IOC_GET_HDR ioctl issued to IPA header insertion block.\n",
	__func__);
	return true;
}

bool HeaderInsertion::CopyHeader(struct ipa_ioc_copy_hdr *pCopyHeaderStruct)
{
	int retval = 0;

	if (!DeviceNodeIsOpened())
		return false;

	retval = ioctl(m_fd, IPA_IOC_COPY_HDR, pCopyHeaderStruct);
	if (retval) {
		printf(
		"%s(), IPA_IOC_COPY_HDR ioctl failed, retval=0x%x.\n",
		__func__,
		retval);
		return false;
	}

	printf(
	"%s(), IPA_IOC_COPY_HDR ioctl issued to IPA header insertion block.\n",
	__func__);
	return true;
}

