/*
 * Copyright (c) 2013, 2018-2019 The Linux Foundation. All rights reserved.
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
#include "ipa_nat_utils.h"
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define IPA_MAX_MSG_LEN 4096

static char dbg_buff[IPA_MAX_MSG_LEN];

#if !defined(MSM_IPA_TESTS) && !defined(USE_GLIB) && !defined(FEATURE_IPA_ANDROID)
size_t strlcpy(char* dst, const char* src, size_t size)
{
	size_t i;

	if (size == 0)
		return strlen(src);

	for (i = 0; i < (size - 1) && src[i] != '\0'; ++i)
		dst[i] = src[i];

	dst[i] = '\0';

	return i + strlen(src + i);
}
#endif

ipa_descriptor* ipa_descriptor_open(void)
{
	ipa_descriptor* desc_ptr;
	int res = 0;

	IPADBG("In\n");

	desc_ptr = calloc(1, sizeof(ipa_descriptor));

	if ( desc_ptr == NULL )
	{
		IPAERR("Unable to allocate ipa_descriptor\n");
		goto bail;
	}

	desc_ptr->fd = open(IPA_DEV_NAME, O_RDONLY);

	if (desc_ptr->fd < 0)
	{
		IPAERR("Unable to open ipa device\n");
		goto free;
	}

	res = ioctl(desc_ptr->fd, IPA_IOC_GET_HW_VERSION, &desc_ptr->ver);

	if (res == 0)
	{
		IPADBG("IPA version is %d\n", desc_ptr->ver);
	}
	else
	{
		IPAERR("Unable to get IPA version. Error %d\n", res);
		desc_ptr->ver = IPA_HW_None;
	}

	goto bail;

free:
	free(desc_ptr);
	desc_ptr = NULL;

bail:
	IPADBG("Out\n");

	return desc_ptr;
}

void ipa_descriptor_close(
	ipa_descriptor* desc_ptr)
{
	IPADBG("In\n");

	if ( desc_ptr )
	{
		if ( desc_ptr->fd >= 0)
		{
			close(desc_ptr->fd);
		}
		free(desc_ptr);
	}

	IPADBG("Out\n");
}

void ipa_read_debug_info(
	const char* debug_file_path)
{
	size_t result;
	FILE* debug_file;

	debug_file = fopen(debug_file_path, "r");
	if (debug_file == NULL)
	{
		printf("Failed to open %s\n", debug_file_path);
		return;
	}

	for (;;)
	{
		result = fread(dbg_buff, sizeof(char), IPA_MAX_MSG_LEN, debug_file);
		if (!result)
			break;

		if (result < IPA_MAX_MSG_LEN)
		{
			if (ferror(debug_file))
			{
				printf("Failed to read from %s\n", debug_file_path);
				break;
			}

			dbg_buff[result] = '\0';
		}
		else
		{
			dbg_buff[IPA_MAX_MSG_LEN - 1] = '\0';
		}


		printf("%s", dbg_buff);

		if (feof(debug_file))
			break;
	}
	fclose(debug_file);
}

void log_nat_message(char *msg)
{
	 return;
}

int currTimeAs(
	TimeAs_t  timeAs,
	uint64_t* valPtr )
{
	struct timespec timeSpec;

	int ret = 0;

	if ( ! VALID_TIMEAS(timeAs) || ! valPtr )
	{
		IPAERR("Bad arg: timeAs (%u) and/or valPtr (%p)\n",
			   timeAs, valPtr );
		ret = -1;
		goto bail;
	}

	memset(&timeSpec, 0, sizeof(timeSpec));

	if ( clock_gettime(CLOCK_MONOTONIC, &timeSpec) != 0 )
	{
		IPAERR("Can't get system clock time\n" );
		ret = -1;
		goto bail;
	}

	switch( timeAs )
	{
	case TimeAsNanSecs:
		*valPtr =
			(uint64_t) (SECS2NanSECS((uint64_t) timeSpec.tv_sec) +
						((uint64_t) timeSpec.tv_nsec));
		break;
	case TimeAsMicSecs:
		*valPtr =
			(uint64_t) (SECS2MicSECS((uint64_t) timeSpec.tv_sec) +
						((uint64_t) timeSpec.tv_nsec / 1000));
		break;
	case TimeAsMilSecs:
		*valPtr =
			(uint64_t) (SECS2MilSECS((uint64_t) timeSpec.tv_sec) +
						((uint64_t) timeSpec.tv_nsec / 1000000));
		break;
	}

bail:
	return ret;
}
