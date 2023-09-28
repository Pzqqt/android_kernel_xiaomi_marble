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

#ifndef _PIPE_H_
#define _PIPE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdint.h>
#include "linux/msm_ipa.h"
#include "Constants.h"
#include "Logger.h"

using namespace std;

/*This class will be used to interact with the system pipes
 *by only referring to Client type.
 *It will allow "raw" data transfer to/from the IPA and
 *will allow a encapsulation of the header addition/
 *removal of a packets thus allowing
 *the test to deal only with IP packet.
 */
class Pipe
{

public:
	/* see Constants.h for nClientType / eConfiguration */
	Pipe(enum ipa_client_type nClientType,
		  IPATestConfiguration eConfiguration);
	/* exception pipe Ctor */
	Pipe(IPATestConfiguration eConfiguration);
	~Pipe();

	/*In this method the actual inode openning will occur.*/
	bool Init();

	/*The close of the inode*/
	void Destroy();

	/*Send the pBuffer(which is an ip[ packet)
	 *after adding the header to the packet.*/
	int  AddHeaderAndSend(
			unsigned char *pBuffer,
			size_t nIPPacketSize);

	/*Send raw data as is - no header removal
	 *- nBytesToSend bytes will be added*/
	int Send(
			unsigned char *pBuffer,
			size_t nBytesToSend);

	/*Receive data from the IPA and remove its header*/
	int  ReceiveAndRemoveHeader(
			unsigned char *pBuffer,
			size_t nIPPacketSize);

	/*Receive data from the IPA as is*/
	int  Receive(unsigned char *pBuffer, size_t nBytesToReceive);

	/*return the Client type of this pipe.*/
	enum ipa_client_type  GetClientType();

	/*Return the length of the header to be added to an
	 *IP packet before it is being sent to the pipe
	 *(This length will be determine by the Pipe's ClientType).
	 */
	int  GetHeaderLengthAdd();

	/*Return the length of the header to be removed from a
	 *packet before it is being sent to the user
	 *(thus returning only the IP packet).
	 *(This length will be determine by the Pipe's ClientType).
	 */
	int  GetHeaderLengthRemove();

	bool EnableHolb(unsigned timerValue);
	bool DisableHolb();

private:
	void SetSpecificClientParameters(
			enum ipa_client_type nClientType,
			IPATestConfiguration eConfiguration);

	bool ConfigureHolb(unsigned short enable, unsigned timerValue);

public:
	/*efault Headers(Can be changed in Derived classes).*/
	static unsigned char m_pUsbHeader[];
	static unsigned char m_pHSICHeader[];
	static unsigned char m_pA2DUNHeader[];
	static unsigned char m_pA2NDUNHeader[];
	static unsigned char m_pQ6LANHeader[];


private:
	int         m_Fd;
	/*The file descriptor which will be used to transfer data
	 * via the inode(this inode will be created by the ITAKEM)
	 */
	enum ipa_client_type  m_nClientType;
	int         m_nHeaderLengthRemove;
	/*this length will be set in the
	 * constructor in corresponds to m_nClientType
	 */
	int         m_nHeaderLengthAdd;
	/*this length will be set in the constructor
	 * in corresponds to m_nClientType
	 */
	unsigned char       *m_pHeader;
	/*this pointer will be set to the current pipe used*/
	const char *m_pInodePath;
	/*this pointer will be set to the current pipe used*/
	bool        m_bInitialized;
	IPATestConfiguration m_eConfiguration;
	/*The Pipes configuration env*/
	bool m_ExceptionPipe;
	/* Is this the exception pipe */

};

#endif
