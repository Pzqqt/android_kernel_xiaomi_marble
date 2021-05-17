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
#ifndef IPA_MEM_DESCRIPTOR_H
#define IPA_MEM_DESCRIPTOR_H

#include <stdint.h>
#include <stdbool.h>
#include <linux/msm_ipa.h>

typedef struct
{
	int orig_rqst_size;
	int mmap_size;
	void* base_addr;
	void* mmap_addr;
	uint32_t addr_offset;
	unsigned long allocate_ioctl_num;
	unsigned long delete_ioctl_num;
	char name[IPA_RESOURCE_NAME_MAX];
	uint8_t table_index;
	uint8_t valid;
	bool consider_using_sram;
	bool sram_available;
	bool sram_to_be_used;
	struct ipa_nat_in_sram_info nat_sram_info;
} ipa_mem_descriptor;

void ipa_mem_descriptor_init(
	ipa_mem_descriptor* desc,
	const char* device_name,
	int size,
	uint8_t table_index,
	unsigned long allocate_ioctl_num,
	unsigned long delete_ioctl_num,
	bool consider_using_sram );

int ipa_mem_descriptor_allocate_memory(
	ipa_mem_descriptor* desc,
	int ipa_fd);

int ipa_mem_descriptor_delete(
	ipa_mem_descriptor* desc,
	int ipa_fd);

#endif
