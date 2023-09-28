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
#include <stdlib.h>
#include <cstring>

#include "HeaderInsertion.h"
#include "TestsUtils.h"

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

bool HeaderInsertion::addHeaderHpc(const string& name, uint8_t* header, const size_t headerLen, bool isPartial, enum ipa_client_type ipaClient){
	if(name.empty() || name.size() >= IPA_RESOURCE_NAME_MAX){
		return false;
	}
	int fd = open(CONFIGURATION_NODE_PATH, O_RDONLY);
	if (fd < 0) {
		cout << "failed to open " << CONFIGURATION_NODE_PATH << endl;
		return false;
	}
	struct ipa_ioc_add_hdr *iocH = static_cast<struct ipa_ioc_add_hdr*>(calloc(1, sizeof(*iocH) + sizeof(struct ipa_hdr_add)));
	if(!iocH){
		return false;
	}
	iocH->commit = 1;
	iocH->num_hdrs = 1;
	struct ipa_hdr_add *h = &iocH->hdr[0];
	strlcpy(h->name, name.c_str(), IPA_RESOURCE_NAME_MAX);
	memcpy(h->hdr, header, headerLen);
	h->hdr_len = headerLen;
	h->hdr_hdl = -1;
	h->status = -1;
	h->is_partial = isPartial;
	cout << "h->name=" << h->name << ", h->is_partial=" << h->is_partial << endl;
	int result = ioctl(fd, IPA_TEST_IOC_ADD_HDR_HPC, iocH);
	if(result || h->status){
		free(iocH);
		close(fd);
		return false;
	}
	cout << "result=" << result << ", status=" << h->status << ", ipaClient=" << ipaClient << endl;
    struct ipa_pkt_init_ex_hdr_ofst_set lookup;
    lookup.ep = ipaClient;
    strlcpy(lookup.name, name.c_str(), IPA_RESOURCE_NAME_MAX);
    result = ioctl(fd, IPA_TEST_IOC_PKT_INIT_EX_SET_HDR_OFST , &lookup);
    if (result) {
		free(iocH);
		close(fd);
		return false;
    }
	free(iocH);
	close(fd);
	return true;
}

bool HeaderInsertion::DeleteHeader(struct ipa_ioc_del_hdr *pHeaderTableToDelete)
{
	int nRetVal = 0;
	/*call the Driver ioctl in order to remove header*/
	nRetVal = ioctl(m_fd, IPA_IOC_DEL_HDR , pHeaderTableToDelete);
	LOG_IOCTL_RETURN_VALUE(nRetVal);
	return (-1 != nRetVal);
}

bool HeaderInsertion::DeleteHeader(const string& name){
	if(name.empty() || name.size() >= IPA_RESOURCE_NAME_MAX){
		return false;
	}
	int hdl = GetHeaderHandle(name);
	if(hdl == -1){
		return false;
	}
	struct ipa_ioc_del_hdr *iocD = static_cast<struct ipa_ioc_del_hdr*>(calloc(1, sizeof(*iocD) + sizeof(struct ipa_hdr_del)));
	if(!iocD){
		return false;
	}
	iocD->commit = 1;
	iocD->num_hdls = 1;
	struct ipa_hdr_del *h = &iocD->hdl[0];
	h->hdl = hdl;
	h->status = -1;
	cout << "h->hdl=" << h->hdl << endl;
	if(!DeleteHeader(iocD)){
		free(iocD);
		return false;
	}
	free(iocD);
	return true;
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

int HeaderInsertion::GetHeaderHandle(const string& name){
	if(name.empty() || name.size() >= IPA_RESOURCE_NAME_MAX){
		return false;
	}
	struct ipa_ioc_get_hdr retHeader;
	memset(&retHeader, 0, sizeof(retHeader));
	strlcpy(retHeader.name, name.c_str(), IPA_RESOURCE_NAME_MAX);
	retHeader.hdl = -1;
	printf("retHeader.name=%s\n", retHeader.name);
	if(!GetHeaderHandle(&retHeader)){
		cout << "GetHeaderHandle(&retHeader) Failed" << endl;
		return -1;
	}
	cout << "retHeader.hdl=" << retHeader.hdl << endl;
	return retHeader.hdl;
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

