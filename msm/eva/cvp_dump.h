/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#ifndef __H_CVP_MINIDUMP_H__
#define __H_CVP_MINIDUMP_H__

#include <linux/notifier.h>
#include <linux/kernel.h>
#include <soc/qcom/minidump.h>

#define MAX_REGION_NAME_LEN 32
#define EVAFW_IMAGE_SIZE 7*1024*1024

extern struct list_head head_node_hfi_queue;
extern struct list_head head_node_dbg_struct;
extern struct notifier_block eva_hfiq_list_notif_blk;
extern struct notifier_block eva_struct_list_notif_blk;

/* notifier handler function for list of eva hfi queues */
int eva_hfiq_list_notif_handler(struct notifier_block *this,
					unsigned long event, void *ptr);

/* notifier handler function for list of eva global structures */
int eva_struct_list_notif_handler(struct notifier_block *this,
					unsigned long event, void *ptr);

/*
 * wrapper for static minidump

 * @name: Dump will be collected with this name
 * @virt: Virtual address of the buffer which needs to be dumped
 * @phys: Physical address of the buffer which needs to be dumped
 * @size: Size of the buffer which needs to be dumped
*/
int md_eva_dump(const char* name, u64 virt, u64 phys, u64 size);

/*
 * Fucntion to add dump region to queue

 * @list_head_node: Head node of the list which needs to be updated
 * @buff_va: Virtual address of the buffer which needs to be dumped
 * @buff_size: Size of the buffer which needs to be dumped
 * @region_name: Dump will be collected with this name
 * @copy: Flag to indicate if the buffer data needs to be copied
 *		to the intermidiate buffer allocated by kzmalloc.
*/
void add_va_node_to_list(void *list_head_node, void *buff_va,
			u32 buff_size, const char *region_name, bool copy);

/*
 * Registers subsystem to minidump driver

 * @name: Subsytem name which will get registered
 * @notf_blk_ptr: notifier block pointer.
 *		notifier_call mentioned in this block will be triggered by
 *		minidump driver in case of crash
*/
void cvp_va_md_register(char *name, void* notf_blk_ptr);

/* One function where we will register all the regions */
void cvp_register_va_md_region(void);

/*
 * Free up the memory allocated for different va_md_list
 * Do not forget to add code for any new list in this function
*/
void cvp_free_va_md_list(void);

/* Adds the HFI queues(both for CPU and DSP) to the global hfi list head*/
void add_hfi_queue_to_va_md_list(void *device);

/*Add queue header structures(both for CPU and DSP)
to the global struct list head*/
void add_queue_header_to_va_md_list(void *device);

/*
 * Node structure for VA_MD Linked List

 * @list: linux kernel list implementation
 * @va_md_buff: Virtual address of the buffer which needs to be dumped
 * @va_md_buff_size: Size of the buffer which needs to be dumped
 * @region_name: Dump will be collected with this name
 * @copy: Flag to indicate if the buffer data needs to be copied
 *		to the intermidiate buffer allocated by kzmalloc.
*/
struct eva_va_md_queue
{
	struct list_head list;
	void *va_md_buff;
	u32 va_md_buff_size;
	char region_name[MAX_REGION_NAME_LEN];
	bool copy;
};

#endif