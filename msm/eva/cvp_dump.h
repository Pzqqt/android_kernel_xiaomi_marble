/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#ifndef __H_CVP_MINIDUMP_H__
#define __H_CVP_MINIDUMP_H__

#include <linux/notifier.h>
#include <linux/kernel.h>
#include "cvp_comm_def.h"

enum cvp_dump_type {
	CVP_QUEUE_DUMP,
	CVP_DBG_DUMP,
	CVP_STATIC_DUMP,
	CVP_MAX_DUMP,
};

#define MAX_REGION_NAME_LEN 32
#define EVAFW_IMAGE_SIZE 7*1024*1024

#ifdef CVP_MINIDUMP_ENABLED
#include <soc/qcom/minidump.h>

/*
 * wrapper for static minidump register

 * @name: Dump will be collected with this name
 * @virt: Virtual address of the buffer which needs to be dumped
 * @phys: Physical address of the buffer which needs to be dumped
 * @size: Size of the buffer which needs to be dumped
*/
int md_eva_static_dump_register(const char *name, u64 virt, u64 phys, u64 size);

/*
 * wrapper for static minidump unregister
*/
void md_eva_static_dump_unregister(void);

/*
 * Fucntion to add dump region to queue

 * @type: Type of the list node which needs to be updated
 * @buff_va: Virtual address of the buffer which needs to be dumped
 * @buff_size: Size of the buffer which needs to be dumped
 * @region_name: Dump will be collected with this name
 * @copy: Flag to indicate if the buffer data needs to be copied
 *		to the intermidiate buffer allocated by kzmalloc.
*/
void add_va_node_to_list(enum cvp_dump_type type, void *buff_va,
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
struct eva_va_md_queue {
	struct list_head list;
	void *va_md_buff;
	u32 va_md_buff_size;
	char region_name[MAX_REGION_NAME_LEN];
	bool copy;
};

/*
 * Node structure for STATIC_MD Linked List

 * @list: linux kernel list implementation
 * @.md_entry: md_region to pass to minidump driver
*/
struct eva_static_md {
	struct list_head list;
	struct md_region md_entry;
};
#else
static inline int md_eva_static_dump_register(const char *name, u64 virt, u64 phys, u64 size)
{
	return 0;
}

static inline void md_eva_static_dump_unregister(void)
{
}

static inline void add_va_node_to_list(enum cvp_dump_type type, void *buff_va,
                        u32 buff_size, const char *region_name, bool copy)
{
}

static inline void cvp_va_md_register(char *name, void* notf_blk_ptr)
{
}

static inline void cvp_register_va_md_region(void)
{
}

static inline void cvp_free_va_md_list(void)
{
}

static inline void add_hfi_queue_to_va_md_list(void *device)
{
}

static inline void add_queue_header_to_va_md_list(void *device)
{
}

#endif	/* End of CVP_MINIDUMP_ENABLED */
#endif
