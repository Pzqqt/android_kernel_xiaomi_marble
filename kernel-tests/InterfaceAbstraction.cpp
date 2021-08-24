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
#include <fcntl.h>
#include <errno.h>
#include <iostream>
#include "InterfaceAbstraction.h"

#define MAX_OPEN_RETRY 10000

using std::cout;
using std::endl;

bool InterfaceAbstraction::Open(const char * toIPAPath, const char * fromIPAPath)
{
	int tries_cnt = MAX_OPEN_RETRY;
	if (NULL == toIPAPath && NULL == fromIPAPath)
	{
		printf("InterfaceAbstraction constructor got 2 null arguments.\n");
		exit(0);
	}

	if (NULL != toIPAPath) {
		while (tries_cnt > 0) {
			printf("trying to open %s %d/%d\n", toIPAPath, MAX_OPEN_RETRY - tries_cnt, MAX_OPEN_RETRY);
			// Sleep for 5 msec
			usleep(5000);
			m_toIPADescriptor = open(toIPAPath, O_WRONLY);
			if (-1 != m_toIPADescriptor) {
				printf("Success!\n");
				break;
			}
			tries_cnt--;
		}
		printf("open retries_cnt=%d\n", MAX_OPEN_RETRY - tries_cnt);
		if (-1 == m_toIPADescriptor) {
			printf("InterfaceAbstraction failed while opening %s.\n", toIPAPath);
			exit(0);
		}
		m_toChannelName = toIPAPath;
		printf("%s device node opened, fd = %d.\n", toIPAPath, m_toIPADescriptor);
	}
	tries_cnt = MAX_OPEN_RETRY;
	if (NULL != fromIPAPath) {
		while (tries_cnt > 0) {
			printf("trying to open %s %d/%d\n", fromIPAPath, MAX_OPEN_RETRY - tries_cnt, MAX_OPEN_RETRY);
			// Sleep for 5 msec
			usleep(5000);
			m_fromIPADescriptor = open(fromIPAPath, O_RDONLY);
			if (-1 != m_fromIPADescriptor) {
				printf("Success!\n");
				break;
			}
			tries_cnt--;
		}
		printf("open retries_cnt=%d\n", MAX_OPEN_RETRY - tries_cnt);
		if (-1 == m_fromIPADescriptor)
		{
			printf("InterfaceAbstraction failed on opening %s.\n", fromIPAPath);
			exit(0);
		}
		m_fromChannelName = fromIPAPath;
		printf("%s device node opened, fd = %d.\n", fromIPAPath, m_fromIPADescriptor);
	}

	return true;
}/*Ctor*/

void InterfaceAbstraction::Close()
{
	close(m_toIPADescriptor);
	close(m_fromIPADescriptor);
}

long InterfaceAbstraction::SendData(unsigned char *buf, size_t size)
{
	long bytesWritten = 0;

	printf("Trying to write %zu bytes to %d.\n", size, m_toIPADescriptor);

	bytesWritten = write(m_toIPADescriptor, buf, size);
	if (-1 == bytesWritten)
	{
		int err = errno;
		printf(
		"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
		printf(
		"Failed to execute command\n write(m_toIPADescriptor, buf, size);\n "
		"m_toIPADescriptor=%d, buf=0x%p, size=%zu",m_toIPADescriptor,
		buf,
		size);
		printf("Error on write execution, errno=%d, Quitting!\n", err);
		printf(
		"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
		exit(-1);
	}

	cout << "bytesWritten = " << bytesWritten << endl;

	return bytesWritten;
}

int InterfaceAbstraction::ReceiveData(unsigned char *buf, size_t size)
{
	size_t bytesRead = 0;
	size_t totalBytesRead = 0;
	bool continueRead = false;

	do
	{
		printf("Trying to read %zu bytes from %d.\n", size, m_fromIPADescriptor);

		bytesRead = read(m_fromIPADescriptor, (void*)buf, size);
		printf("Read %zu bytes.\n", bytesRead);
		totalBytesRead += bytesRead;
		if (bytesRead == size)
			continueRead = true;
		else
			continueRead = false;
	} while (continueRead);

	return totalBytesRead;
}

int InterfaceAbstraction::ReceiveSingleDataChunk(unsigned char *buf, size_t size){
	size_t bytesRead = 0;
	printf("Trying to read %zu bytes from %d.\n", size, m_fromIPADescriptor);
	bytesRead = read(m_fromIPADescriptor, (void*)buf, size);
	printf("Read %zu bytes.\n", bytesRead);
	return bytesRead;
}

int InterfaceAbstraction::setReadNoBlock(){
	int flags = fcntl(m_fromIPADescriptor, F_GETFL, 0);
	if(flags == -1){
		return -1;
	}
	return fcntl(m_fromIPADescriptor, F_SETFL, flags | O_NONBLOCK);
}

int InterfaceAbstraction::clearReadNoBlock(){
	int flags = fcntl(m_fromIPADescriptor, F_GETFL, 0);
	if(flags == -1){
		return -1;
	}
	return fcntl(m_fromIPADescriptor, F_SETFL, flags & ~O_NONBLOCK);
}

InterfaceAbstraction::~InterfaceAbstraction()
{
	close(m_fromIPADescriptor);
	m_fromChannelName = "";

	close(m_toIPADescriptor);
	m_toChannelName = "";
}
