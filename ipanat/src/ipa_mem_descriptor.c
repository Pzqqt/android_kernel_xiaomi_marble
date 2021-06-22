/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
#include "ipa_mem_descriptor.h"
#include "ipa_nat_utils.h"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>

#define IPA_DEV_DIR "/dev/"

#ifdef IPA_ON_R3PC
#define IPA_DEVICE_MMAP_MEM_SIZE (2 * 1024UL * 1024UL - 1)
#endif

static int AllocateMemory(
	ipa_mem_descriptor* desc,
	int ipa_fd)
{
	struct ipa_ioc_nat_ipv6ct_table_alloc cmd;
	int ret = 0;

	IPADBG("In\n");

#ifndef IPA_ON_R3PC
	/*
	 * If/when the number of NAT table entries requested yields a byte
	 * count that will fit in SRAM, SRAM will be used to hold the NAT
	 * table. When SRAM is used, some odd things can happen, relative
	 * to mmap'ing's virtual memory scheme, that require us to make
	 * some adjustments.
	 *
	 * To be more specific, the real physical SRAM location for the
	 * table and the table's size may not play well with Linux's
	 * mmap'ing virtual memory scheme....which likes everything to be
	 * PAGE_SIZE aligned and sized in multiples of PAGE_SIZE.
	 *
	 * Given the above, if the NAT table's (in SRAM) physical address
	 * in not on a PAGE_SIZE boundary, it will be offset into the
	 * mmap'd virtual memory, hence we need to know that offset in
	 * order to get to the table.  If said offset plus the table's
	 * size takes it across a PAGE_SIZE boundary, we need to allocate
	 * more space to ensure that the table is completely within the
	 * mmap'd virtual memory.
	 */
	desc->sram_available = desc->sram_to_be_used = false;

	memset(&desc->nat_sram_info, 0, sizeof(desc->nat_sram_info));

	ret = ioctl(
		ipa_fd,
		IPA_IOC_GET_NAT_IN_SRAM_INFO,
		&desc->nat_sram_info);

	if ( ret == 0 )
	{
		IPADBG("sram_mem_available_for_nat(0x%08x) "
			   "nat_table_offset_into_mmap(0x%08x) "
			   "best_nat_in_sram_size_rqst(0x%08x)\n",
			   desc->nat_sram_info.sram_mem_available_for_nat,
			   desc->nat_sram_info.nat_table_offset_into_mmap,
			   desc->nat_sram_info.best_nat_in_sram_size_rqst);

		desc->sram_available = true;

		if ( desc->consider_using_sram )
		{
			if (desc->orig_rqst_size <=
				desc->nat_sram_info.sram_mem_available_for_nat)
			{
				desc->sram_to_be_used = true;
			}
		}
	}
#endif

	/*
	 * Now do the actual allocation...
	 */
	memset(&cmd, 0, sizeof(cmd));

	cmd.size = desc->orig_rqst_size;

	ret = ioctl(ipa_fd, desc->allocate_ioctl_num, &cmd);

	if (ret)
	{
		IPAERR("Unable to post %s allocate table command. Error %d IPA fd %d\n",
			   desc->name, ret, ipa_fd);
		goto bail;
	}

	desc->addr_offset = cmd.offset;

	IPADBG("The memory desc for %s allocated successfully\n", desc->name);

bail:
	IPADBG("Out\n");

	return ret;
}

static int MapMemory(
	ipa_mem_descriptor* desc,
	int ipa_fd)
{
	char device_full_path[IPA_RESOURCE_NAME_MAX];
	size_t ipa_dev_dir_path_len;
	int device_fd;
	int ret = 0;

	IPADBG("In\n");

	ipa_dev_dir_path_len =
		strlcpy(device_full_path, IPA_DEV_DIR, IPA_RESOURCE_NAME_MAX);

	if (ipa_dev_dir_path_len >= IPA_RESOURCE_NAME_MAX)
	{
		IPAERR("Unable to copy a string with size %d to buffer with size %d\n",
			   (int)ipa_dev_dir_path_len, IPA_RESOURCE_NAME_MAX);
		ret = -EINVAL;
		goto bail;
	}

	strlcpy(device_full_path + ipa_dev_dir_path_len,
			desc->name, IPA_RESOURCE_NAME_MAX - ipa_dev_dir_path_len);

	device_fd = open(device_full_path, O_RDWR);

	if (device_fd < 0)
	{
		IPAERR("unable to open the desc %s in path %s. Error:%d\n",
			   desc->name, device_full_path, device_fd);
		ret = -EIO;
		goto bail;
	}

#ifndef IPA_ON_R3PC
	/*
	 * If/when the number of NAT table entries requested yields a byte
	 * count that will fit in SRAM, SRAM will be used to hold the NAT
	 * table. When SRAM is used, some odd things can happen, relative
	 * to mmap'ing's virtual memory scheme, that require us to make
	 * some adjustments.
	 *
	 * To be more specific, the real physical SRAM location for the
	 * table and the table's size may not play well with Linux's
	 * mmap'ing virtual memory scheme....which likes everything to be
	 * PAGE_SIZE aligned and sized in multiples of PAGE_SIZE.
	 *
	 * Given the above, if the NAT table's (in SRAM) physical address
	 * in not on a PAGE_SIZE boundary, it will be offset into the
	 * mmap'd virtual memory, hence we need to know that offset in
	 * order to get to the table.  If said offset plus the table's
	 * size takes it across a PAGE_SIZE boundary, we need to allocate
	 * more space to ensure that the table is completely within the
	 * mmap'd virtual memory.
	 */
	desc->mmap_size =
		( desc->sram_to_be_used )                      ?
		desc->nat_sram_info.best_nat_in_sram_size_rqst :
		desc->orig_rqst_size;

	desc->mmap_addr = desc->base_addr =
		(void* )mmap(
			NULL,
			desc->mmap_size,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			device_fd,
			0);
#else
	IPADBG("user space r3pc\n");
	desc->mmap_addr = desc->base_addr =
		(void *) mmap(
			(caddr_t)0,
			IPA_DEVICE_MMAP_MEM_SIZE,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			device_fd,
			0);
#endif

	if (desc->base_addr == MAP_FAILED)
	{
		IPAERR("Unable to mmap the memory for %s\n", desc->name);
		ret = -EINVAL;
		goto close;
	}

	if ( desc->sram_to_be_used )
	{
		desc->base_addr =
			(uint8_t*) (desc->base_addr) +
			desc->nat_sram_info.nat_table_offset_into_mmap;
	}

	IPADBG("mmap for %s return value 0x%lx -> 0x%lx\n",
		   desc->name,
		   (long unsigned int) desc->mmap_addr,
		   (long unsigned int) desc->base_addr);

close:
	if (close(device_fd))
	{
		IPAERR("unable to close the file descriptor for %s\n", desc->name);
		ret = -EINVAL;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

static int DeallocateMemory(
	ipa_mem_descriptor* desc,
	int ipa_fd)
{
	struct ipa_ioc_nat_ipv6ct_table_del cmd;
	int ret = 0;

	IPADBG("In\n");

	memset(&cmd, 0, sizeof(cmd));

	cmd.table_index = desc->table_index;

	cmd.mem_type =
		( desc->sram_to_be_used ) ?
		IPA_NAT_MEM_IN_SRAM       :
		IPA_NAT_MEM_IN_DDR;

	ret = ioctl(ipa_fd, desc->delete_ioctl_num, &cmd);

	if (ret)
	{
		IPAERR("unable to post table delete command for %s Error: %d IPA fd %d\n",
			   desc->name, ret, ipa_fd);
		goto bail;
	}

	IPADBG("posted delete command for %s to kernel successfully\n", desc->name);

bail:
	IPADBG("Out\n");

	return ret;
}

void ipa_mem_descriptor_init(
	ipa_mem_descriptor* desc,
	const char* device_name,
	int size,
	uint8_t table_index,
	unsigned long allocate_ioctl_num,
	unsigned long delete_ioctl_num,
	bool consider_using_sram )
{
	IPADBG("In\n");

	strlcpy(desc->name, device_name, IPA_RESOURCE_NAME_MAX);

	desc->orig_rqst_size      = desc->mmap_size = size;
	desc->table_index         = table_index;
	desc->allocate_ioctl_num  = allocate_ioctl_num;
	desc->delete_ioctl_num    = delete_ioctl_num;
	desc->consider_using_sram = consider_using_sram;

	IPADBG("Out\n");
}

int ipa_mem_descriptor_allocate_memory(
	ipa_mem_descriptor* desc,
	int ipa_fd)
{
	int ret;

	IPADBG("In\n");

	ret = AllocateMemory(desc, ipa_fd);

	if (ret)
	{
		IPAERR("unable to allocate %s\n", desc->name);
		goto bail;
	}

	ret = MapMemory(desc, ipa_fd);

	if (ret)
	{
		IPAERR("unable to map %s\n", desc->name);
		DeallocateMemory(desc, ipa_fd);
		goto bail;
	}

	desc->valid = TRUE;

bail:
	IPADBG("Out\n");

	return ret;
}

int ipa_mem_descriptor_delete(
	ipa_mem_descriptor* desc,
	int ipa_fd)
{
	int ret = 0;

	IPADBG("In\n");

	if (! desc->valid)
	{
		IPAERR("invalid desc handle passed\n");
		ret = -EINVAL;
		goto bail;
	}

	desc->valid = FALSE;

#ifndef IPA_ON_R3PC
	munmap(desc->mmap_addr, desc->mmap_size);
#else
	munmap(desc->mmap_addr, IPA_DEVICE_MMAP_MEM_SIZE);
#endif

	ret = DeallocateMemory(desc, ipa_fd);

bail:
	IPADBG("Out\n");

	return ret;
}
