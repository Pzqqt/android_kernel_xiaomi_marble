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
#include "ipa_ipv6ct.h"
#include "ipa_ipv6cti.h"

#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#define IPA_IPV6CT_DEBUG_FILE_PATH "/sys/kernel/debug/ipa/ipv6ct"
#define IPA_UC_ACT_DEBUG_FILE_PATH "/sys/kernel/debug/ipa/uc_act_table"
#define IPA_IPV6CT_TABLE_NAME "IPA IPv6CT table"
#define IPA_MAX_DMA_ENTRIES_FOR_ADD 2
#define IPA_MAX_DMA_ENTRIES_FOR_DEL 2

static int ipa_ipv6ct_create_table(ipa_ipv6ct_table* ipv6ct_table, uint16_t number_of_entries, uint8_t table_index);
static int ipa_ipv6ct_destroy_table(ipa_ipv6ct_table* ipv6ct_table);
static void ipa_ipv6ct_create_table_dma_cmd_helpers(ipa_ipv6ct_table* ipv6ct_table, uint8_t table_indx);
static int ipa_ipv6ct_post_init_cmd(ipa_ipv6ct_table* ipv6ct_table, uint8_t tbl_index);
static int ipa_ipv6ct_post_dma_cmd(struct ipa_ioc_nat_dma_cmd* cmd);
static uint16_t ipa_ipv6ct_hash(const ipa_ipv6ct_rule* rule, uint16_t size);
static uint16_t ipa_ipv6ct_xor_segments(uint64_t num);

static int table_entry_is_valid(void* entry);
static uint16_t table_entry_get_next_index(void* entry);
static uint16_t table_entry_get_prev_index(void* entry, uint16_t entry_index, void* meta, uint16_t base_table_size);
static void table_entry_set_prev_index(void* entry, uint16_t entry_index, uint16_t prev_index,
	void* meta, uint16_t base_table_size);
static int table_entry_head_insert(void* entry, void* user_data, uint16_t* dma_command_data);
static int table_entry_tail_insert(void* entry, void* user_data);
static uint16_t table_entry_get_delete_head_dma_command_data(void* head, void* next_entry);

static ipa_ipv6ct ipv6ct;
static pthread_mutex_t ipv6ct_mutex = PTHREAD_MUTEX_INITIALIZER;

static ipa_table_entry_interface entry_interface =
{
	table_entry_is_valid,
	table_entry_get_next_index,
	table_entry_get_prev_index,
	table_entry_set_prev_index,
	table_entry_head_insert,
	table_entry_tail_insert,
	table_entry_get_delete_head_dma_command_data
};

/**
 * ipa_ipv6ct_add_tbl() - Adds a new IPv6CT table
 * @number_of_entries: [in] number of IPv6CT entries
 * @table_handle: [out] handle of new IPv6CT table
 *
 * This function creates new IPv6CT table and posts IPv6CT init command to HW
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_ipv6ct_add_tbl(uint16_t number_of_entries, uint32_t* table_handle)
{
	int ret;
	ipa_ipv6ct_table* ipv6ct_table;

	IPADBG("\n");

	if (table_handle == NULL || number_of_entries == 0)
	{
		IPAERR("Invalid parameters table_handle=%pK number_of_entries=%d\n", table_handle, number_of_entries);
		return -EINVAL;
	}

	*table_handle = 0;

	if (ipv6ct.table_cnt >= IPA_IPV6CT_MAX_TBLS)
	{
		IPAERR("Can't add addition IPv6 connection tracking table. Maximum %d tables allowed\n", IPA_IPV6CT_MAX_TBLS);
		return -EINVAL;
	}

	if (!ipv6ct.ipa_desc)
	{
		ipv6ct.ipa_desc = ipa_descriptor_open();
		if (ipv6ct.ipa_desc == NULL)
		{
			IPAERR("failed to open IPA driver file descriptor\n");
			return -EIO;
		}
	}

	if (ipv6ct.ipa_desc->ver < IPA_HW_v4_0)
	{
		IPAERR("IPv6 connection tracking isn't supported for IPA version %d\n", ipv6ct.ipa_desc->ver);
		ret = -EPERM;
		goto bail_ipa_desc;
	}

	ipv6ct_table = &ipv6ct.tables[ipv6ct.table_cnt];
	ret = ipa_ipv6ct_create_table(ipv6ct_table, number_of_entries, ipv6ct.table_cnt);
	if (ret)
	{
		IPAERR("unable to create ipv6ct table Error: %d\n", ret);
		goto bail_ipa_desc;
	}

	/* Initialize the ipa hw with ipv6ct table dimensions */
	ret = ipa_ipv6ct_post_init_cmd(ipv6ct_table, ipv6ct.table_cnt);
	if (ret)
	{
		IPAERR("unable to post ipv6ct_init command Error %d\n", ret);
		goto bail_ipv6ct_table;
	}

	/* Return table handle */
	++ipv6ct.table_cnt;
	*table_handle = ipv6ct.table_cnt;

	IPADBG("Returning table handle 0x%x\n", *table_handle);
	return 0;

bail_ipv6ct_table:
	ipa_ipv6ct_destroy_table(ipv6ct_table);
bail_ipa_desc:
	if (!ipv6ct.table_cnt) {
		ipa_descriptor_close(ipv6ct.ipa_desc);
		ipv6ct.ipa_desc = NULL;
	}
	return ret;
}

int ipa_ipv6ct_del_tbl(uint32_t table_handle)
{
	ipa_ipv6ct_table* ipv6ct_table;
	int ret;

	IPADBG("\n");

	if (ipv6ct.ipa_desc->ver < IPA_HW_v4_0)
	{
		IPAERR("IPv6 connection tracking isn't supported for IPA version %d\n", ipv6ct.ipa_desc->ver);
		return -EINVAL;
	}

	if (table_handle == IPA_TABLE_INVALID_ENTRY || table_handle > IPA_IPV6CT_MAX_TBLS)
	{
		IPAERR("invalid table handle %d passed\n", table_handle);
		return -EINVAL;
	}
	IPADBG("Passed Table Handle: 0x%x\n", table_handle);

	if (pthread_mutex_lock(&ipv6ct_mutex))
	{
		IPAERR("unable to lock the ipv6ct mutex\n");
		return -EINVAL;
	}

	ipv6ct_table = &ipv6ct.tables[table_handle - 1];
	if (!ipv6ct_table->mem_desc.valid)
	{
		IPAERR("invalid table handle %d\n", table_handle);
		ret = -EINVAL;
		goto unlock;
	}

	ret = ipa_ipv6ct_destroy_table(ipv6ct_table);
	if (ret)
	{
		IPAERR("unable to delete IPV6CT table with handle %d\n", table_handle);
		goto unlock;
	}

	if (!--ipv6ct.table_cnt) {
		ipa_descriptor_close(ipv6ct.ipa_desc);
		ipv6ct.ipa_desc = NULL;
	}

unlock:
	if (pthread_mutex_unlock(&ipv6ct_mutex))
	{
		IPAERR("unable to unlock the ipv6ct mutex\n");
		return (ret) ? ret : -EPERM;
	}

	IPADBG("return\n");
	return ret;
}

int ipa_ipv6ct_add_rule(uint32_t table_handle, const ipa_ipv6ct_rule* user_rule, uint32_t* rule_handle)
{
	int ret;
	ipa_ipv6ct_table* ipv6ct_table;
	uint16_t new_entry_index;
	uint32_t new_entry_handle;
	uint32_t cmd_sz = sizeof(struct ipa_ioc_nat_dma_cmd) +
		(IPA_MAX_DMA_ENTRIES_FOR_ADD * sizeof(struct ipa_ioc_nat_dma_one));
	char cmd_buf[cmd_sz];
	struct ipa_ioc_nat_dma_cmd* cmd;

	IPADBG("\n");

	if (ipv6ct.ipa_desc->ver < IPA_HW_v4_0)
	{
		IPAERR("IPv6 connection tracking isn't supported for IPA version %d\n", ipv6ct.ipa_desc->ver);
		return -EINVAL;
	}

	if (table_handle == IPA_TABLE_INVALID_ENTRY || table_handle > IPA_IPV6CT_MAX_TBLS ||
		rule_handle == NULL || user_rule == NULL)
	{
		IPAERR("Invalid parameters table_handle=%d rule_handle=%pK user_rule=%pK\n",
			table_handle, rule_handle, user_rule);
		return -EINVAL;
	}
	IPADBG("Passed Table handle: 0x%x\n", table_handle);

	if (user_rule->protocol == IPA_IPV6CT_INVALID_PROTO_FIELD_CMP)
	{
		IPAERR("invalid parameter protocol=%d\n", user_rule->protocol);
		return -EINVAL;
	}

	if (pthread_mutex_lock(&ipv6ct_mutex))
	{
		IPAERR("unable to lock the ipv6ct mutex\n");
		return -EINVAL;
	}

	ipv6ct_table = &ipv6ct.tables[table_handle - 1];
	if (!ipv6ct_table->mem_desc.valid)
	{
		IPAERR("invalid table handle %d\n", table_handle);
		ret = -EINVAL;
		goto unlock;
	}

	memset(cmd_buf, 0, sizeof(cmd_buf));
	cmd = (struct ipa_ioc_nat_dma_cmd*) cmd_buf;
	cmd->entries = 0;
	new_entry_index = ipa_ipv6ct_hash(user_rule, ipv6ct_table->table.table_entries - 1);

	ret = ipa_table_add_entry(&ipv6ct_table->table, (void*)user_rule, &new_entry_index, &new_entry_handle, cmd);
	if (ret)
	{
		IPAERR("failed to add a new IPV6CT entry\n");
		goto unlock;
	}

	ret = ipa_ipv6ct_post_dma_cmd(cmd);
	if (ret)
	{
		IPAERR("unable to post dma command\n");
		goto bail;
	}

	if (pthread_mutex_unlock(&ipv6ct_mutex))
	{
		IPAERR("unable to unlock the ipv6ct mutex\n");
		return -EPERM;
	}

	*rule_handle = new_entry_handle;

	IPADBG("return\n");
	return 0;

bail:
	ipa_table_erase_entry(&ipv6ct_table->table, new_entry_index);
unlock:
	if (pthread_mutex_unlock(&ipv6ct_mutex))
		IPAERR("unable to unlock the ipv6ct mutex\n");
	return ret;
}

int ipa_ipv6ct_del_rule(uint32_t table_handle, uint32_t rule_handle)
{
	ipa_ipv6ct_table* ipv6ct_table;
	ipa_table_iterator table_iterator;
	ipa_ipv6ct_hw_entry* entry;
	uint32_t cmd_sz = sizeof(struct ipa_ioc_nat_dma_cmd) +
		(IPA_MAX_DMA_ENTRIES_FOR_DEL * sizeof(struct ipa_ioc_nat_dma_one));
	char cmd_buf[cmd_sz];
	struct ipa_ioc_nat_dma_cmd* cmd;
	uint16_t index;
	int ret;

	IPADBG("\n");

	if (ipv6ct.ipa_desc->ver < IPA_HW_v4_0)
	{
		IPAERR("IPv6 connection tracking isn't supported for IPA version %d\n", ipv6ct.ipa_desc->ver);
		return -EINVAL;
	}

	if (table_handle == IPA_TABLE_INVALID_ENTRY || table_handle > IPA_IPV6CT_MAX_TBLS ||
		rule_handle == IPA_TABLE_INVALID_ENTRY)
	{
		IPAERR("Invalid parameters table_handle=%d rule_handle=%d\n", table_handle, rule_handle);
		return -EINVAL;
	}
	IPADBG("Passed Table: 0x%x and rule handle 0x%x\n", table_handle, rule_handle);

	if (pthread_mutex_lock(&ipv6ct_mutex))
	{
		IPAERR("unable to lock the ipv6ct mutex\n");
		return -EINVAL;
	}

	ipv6ct_table = &ipv6ct.tables[table_handle - 1];
	if (!ipv6ct_table->mem_desc.valid)
	{
		IPAERR("invalid table handle %d\n", table_handle);
		ret = -EINVAL;
		goto unlock;
	}

	ret = ipa_table_get_entry(&ipv6ct_table->table, rule_handle, (void**)&entry, &index);
	if (ret)
	{
		IPAERR("unable to retrive the entry with handle=%d in IPV6CT table with handle=%d\n",
			rule_handle, table_handle);
		goto unlock;
	}

	ret = ipa_table_iterator_init(&table_iterator, &ipv6ct_table->table, entry, index);
	if (ret)
	{
		IPAERR("unable to create iterator which points to the entry index=%d in IPV6CT table with handle=%d\n",
			index, table_handle);
		goto unlock;
	}

	memset(cmd_buf, 0, sizeof(cmd_buf));
	cmd = (struct ipa_ioc_nat_dma_cmd*) cmd_buf;
	cmd->entries = 0;

	ipa_table_create_delete_command(&ipv6ct_table->table, cmd, &table_iterator);

	ret = ipa_ipv6ct_post_dma_cmd(cmd);
	if (ret)
	{
		IPAERR("unable to post dma command\n");
		goto unlock;
	}

	if (!ipa_table_iterator_is_head_with_tail(&table_iterator))
	{
		/* The entry can be deleted */
		uint8_t is_prev_empty = (table_iterator.prev_entry != NULL &&
			((ipa_ipv6ct_hw_entry*)table_iterator.prev_entry)->protocol == IPA_IPV6CT_INVALID_PROTO_FIELD_CMP);
		ipa_table_delete_entry(&ipv6ct_table->table, &table_iterator, is_prev_empty);
	}

unlock:
	if (pthread_mutex_unlock(&ipv6ct_mutex))
	{
		IPAERR("unable to unlock the ipv6ct mutex\n");
		return (ret) ? ret : -EPERM;
	}

	IPADBG("return\n");
	return ret;
}

int ipa_ipv6ct_query_timestamp(uint32_t table_handle, uint32_t rule_handle, uint32_t* time_stamp)
{
	int ret;
	ipa_ipv6ct_table* ipv6ct_table;
	ipa_ipv6ct_hw_entry *entry;

	IPADBG("\n");

	if (ipv6ct.ipa_desc->ver < IPA_HW_v4_0)
	{
		IPAERR("IPv6 connection tracking isn't supported for IPA version %d\n", ipv6ct.ipa_desc->ver);
		return -EINVAL;
	}

	if (table_handle == IPA_TABLE_INVALID_ENTRY || table_handle > IPA_IPV6CT_MAX_TBLS ||
		rule_handle == IPA_TABLE_INVALID_ENTRY || time_stamp == NULL)
	{
		IPAERR("invalid parameters passed table_handle=%d rule_handle=%d time_stamp=%pK\n",
			table_handle, rule_handle, time_stamp);
		return -EINVAL;
	}
	IPADBG("Passed Table: %d and rule handle %d\n", table_handle, rule_handle);

	if (pthread_mutex_lock(&ipv6ct_mutex))
	{
		IPAERR("unable to lock the ipv6ct mutex\n");
		return -EINVAL;
	}

	ipv6ct_table = &ipv6ct.tables[table_handle - 1];
	if (!ipv6ct_table->mem_desc.valid)
	{
		IPAERR("invalid table handle %d\n", table_handle);
		ret = -EINVAL;
		goto unlock;
	}

	ret = ipa_table_get_entry(&ipv6ct_table->table, rule_handle, (void**)&entry, NULL);
	if (ret)
	{
		IPAERR("unable to retrive the entry with handle=%d in IPV6CT table with handle=%d\n",
			rule_handle, table_handle);
		goto unlock;
	}

	*time_stamp = entry->time_stamp;

unlock:
	if (pthread_mutex_unlock(&ipv6ct_mutex))
	{
		IPAERR("unable to unlock the ipv6ct mutex\n");
		return (ret) ? ret : -EPERM;
	}

	IPADBG("return\n");
	return ret;
}

/**
* ipv6ct_hash() - Find the index into ipv6ct table
* @rule: [in] an IPv6CT rule
* @size: [in] size of the IPv6CT table
*
* This hash method is used to find the hash index of an entry into IPv6CT table.
* In case of result zero, N-1 will be returned, where N is size of IPv6CT table.
*
* Returns: >0 index into IPv6CT table, negative on failure
*/
static uint16_t ipa_ipv6ct_hash(const ipa_ipv6ct_rule* rule, uint16_t size)
{
	uint16_t hash = 0;

	IPADBG("src_ipv6_lsb 0x%llx\n", rule->src_ipv6_lsb);
	IPADBG("src_ipv6_msb 0x%llx\n", rule->src_ipv6_msb);
	IPADBG("dest_ipv6_lsb 0x%llx\n", rule->dest_ipv6_lsb);
	IPADBG("dest_ipv6_msb 0x%llx\n", rule->dest_ipv6_msb);
	IPADBG("src_port: 0x%x dest_port: 0x%x\n", rule->src_port, rule->dest_port);
	IPADBG("protocol: 0x%x size: 0x%x\n", rule->protocol, size);

	hash ^= ipa_ipv6ct_xor_segments(rule->src_ipv6_lsb);
	hash ^= ipa_ipv6ct_xor_segments(rule->src_ipv6_msb);
	hash ^= ipa_ipv6ct_xor_segments(rule->dest_ipv6_lsb);
	hash ^= ipa_ipv6ct_xor_segments(rule->dest_ipv6_msb);

	hash ^= rule->src_port;
	hash ^= rule->dest_port;
	hash ^= rule->protocol;

	/*
	 * The size passed to hash function expected be power^2-1, while the actual size is power^2,
	 * actual_size = size + 1
	 */
	hash &= size;

	/* If the hash resulted to zero then set it to maximum value as zero is unused entry in ipv6ct table */
	if (hash == 0)
	{
		hash = size;
	}

	IPADBG("ipa_ipv6ct_hash returning value: %d\n", hash);
	return hash;
}

static uint16_t ipa_ipv6ct_xor_segments(uint64_t num)
{
	const uint64_t mask = 0xffff;
	const size_t bits_in_two_byte = 16;
	uint16_t ret = 0;

	IPADBG("\n");

	while (num)
	{
		ret ^= (uint16_t)(num & mask);
		num >>= bits_in_two_byte;
	}

	IPADBG("return\n");
	return ret;
}

static int table_entry_is_valid(void* entry)
{
	ipa_ipv6ct_hw_entry* ipv6ct_entry = (ipa_ipv6ct_hw_entry*)entry;

	IPADBG("\n");

	return ipv6ct_entry->enable;
}

static uint16_t table_entry_get_next_index(void* entry)
{
	uint16_t result;
	ipa_ipv6ct_hw_entry* ipv6ct_entry = (ipa_ipv6ct_hw_entry*)entry;

	IPADBG("\n");

	result = ipv6ct_entry->next_index;

	IPADBG("Next entry of %pK is %d\n", entry, result);
	return result;
}

static uint16_t table_entry_get_prev_index(void* entry, uint16_t entry_index, void* meta, uint16_t base_table_size)
{
	uint16_t result;
	ipa_ipv6ct_hw_entry* ipv6ct_entry = (ipa_ipv6ct_hw_entry*)entry;

	IPADBG("\n");

	result = ipv6ct_entry->prev_index;

	IPADBG("Previous entry of %d is %d\n", entry_index, result);
	return result;
}

static void table_entry_set_prev_index(void* entry, uint16_t entry_index, uint16_t prev_index,
	void* meta, uint16_t base_table_size)
{
	ipa_ipv6ct_hw_entry* ipv6ct_entry = (ipa_ipv6ct_hw_entry*)entry;

	IPADBG("Previous entry of %d is %d\n", entry_index, prev_index);

	ipv6ct_entry->prev_index = prev_index;

	IPADBG("return\n");
}

static int table_entry_copy_from_user(void* entry, void* user_data)
{
	ipa_ipv6ct_hw_entry* ipv6ct_entry = (ipa_ipv6ct_hw_entry*)entry;
	const ipa_ipv6ct_rule* user_rule = (const ipa_ipv6ct_rule*)user_data;

	IPADBG("\n");

	ipv6ct_entry->src_ipv6_lsb = user_rule->src_ipv6_lsb;
	ipv6ct_entry->src_ipv6_msb = user_rule->src_ipv6_msb;
	ipv6ct_entry->dest_ipv6_lsb = user_rule->dest_ipv6_lsb;
	ipv6ct_entry->dest_ipv6_msb = user_rule->dest_ipv6_msb;
	ipv6ct_entry->protocol = user_rule->protocol;
	ipv6ct_entry->src_port = user_rule->src_port;
	ipv6ct_entry->dest_port = user_rule->dest_port;
	ipv6ct_entry->ucp = user_rule->ucp;
	ipv6ct_entry->uc_activation_index = user_rule->uc_activation_index;
	ipv6ct_entry->s = user_rule->s;

	switch (user_rule->direction_settings)
	{
	case IPA_IPV6CT_DIRECTION_DENY_ALL:
		break;
	case IPA_IPV6CT_DIRECTION_ALLOW_OUT:
		ipv6ct_entry->out_allowed = IPA_IPV6CT_DIRECTION_ALLOW_BIT;
		break;
	case IPA_IPV6CT_DIRECTION_ALLOW_IN:
		ipv6ct_entry->in_allowed = IPA_IPV6CT_DIRECTION_ALLOW_BIT;
		break;
	case IPA_IPV6CT_DIRECTION_ALLOW_ALL:
		ipv6ct_entry->out_allowed = IPA_IPV6CT_DIRECTION_ALLOW_BIT;
		ipv6ct_entry->in_allowed = IPA_IPV6CT_DIRECTION_ALLOW_BIT;
		break;
	default:
		IPAERR("wrong value for IPv6CT direction setting parameter %d\n", user_rule->direction_settings);
		return -EINVAL;
	}

	IPADBG("return\n");
	return 0;
}

static int table_entry_head_insert(void* entry, void* user_data, uint16_t* dma_command_data)
{
	int ret;

	IPADBG("\n");

	ret = table_entry_copy_from_user(entry, user_data);
	if (ret)
	{
		IPAERR("unable to copy from user a new entry\n");
		return ret;
	}

	*dma_command_data = 0;
	((ipa_ipv6ct_flags*)dma_command_data)->enable = IPA_IPV6CT_FLAG_ENABLE_BIT;

	IPADBG("return\n");
	return 0;
}

static int table_entry_tail_insert(void* entry, void* user_data)
{
	int ret;

	IPADBG("\n");

	ret = table_entry_copy_from_user(entry, user_data);
	if (ret)
	{
		IPAERR("unable to copy from user a new entry\n");
		return ret;
	}

	((ipa_ipv6ct_hw_entry*)entry)->enable = IPA_IPV6CT_FLAG_ENABLE_BIT;

	IPADBG("return\n");
	return 0;
}

static uint16_t table_entry_get_delete_head_dma_command_data(void* head, void* next_entry)
{
	IPADBG("\n");
	return IPA_IPV6CT_INVALID_PROTO_FIELD_VALUE;
}

/**
 * ipa_ipv6ct_create_table() - Creates a new IPv6CT table
 * @ipv6ct_table: [in] IPv6CT table
 * @number_of_entries: [in] number of IPv6CT entries
 * @table_index: [in] the index of the IPv6CT table
 *
 * This function creates new IPv6CT table:
 * - Initializes table, memory descriptor and table_dma_cmd_helpers structures
 * - Allocates, maps and clears the memory for table
 *
 * Returns:	0  On Success, negative on failure
 */
static int ipa_ipv6ct_create_table(ipa_ipv6ct_table* ipv6ct_table, uint16_t number_of_entries, uint8_t table_index)
{
	int ret, size;

	IPADBG("\n");

	ipa_table_init(
		&ipv6ct_table->table, IPA_IPV6CT_TABLE_NAME, IPA_NAT_MEM_IN_DDR,
		sizeof(ipa_ipv6ct_hw_entry), NULL, 0, &entry_interface);

	ret = ipa_table_calculate_entries_num(
		&ipv6ct_table->table, number_of_entries, IPA_NAT_MEM_IN_DDR);

	if (ret)
	{
		IPAERR("unable to calculate number of entries in ipv6ct table %d, while required by user %d\n",
			table_index, number_of_entries);
		return ret;
	}

	size = ipa_table_calculate_size(&ipv6ct_table->table);
	IPADBG("IPv6CT table size: %d\n", size);

	ipa_mem_descriptor_init(
		&ipv6ct_table->mem_desc,
		IPA_IPV6CT_DEV_NAME,
		size,
		table_index,
		IPA_IOC_ALLOC_IPV6CT_TABLE,
		IPA_IOC_DEL_IPV6CT_TABLE,
		false); /* false here means don't consider using sram */

	ret = ipa_mem_descriptor_allocate_memory(
		&ipv6ct_table->mem_desc,
		ipv6ct.ipa_desc->fd);

	if (ret)
	{
		IPAERR("unable to allocate ipv6ct memory descriptor Error: %d\n", ret);
		goto bail;
	}

	ipa_table_calculate_addresses(&ipv6ct_table->table, ipv6ct_table->mem_desc.base_addr);

	ipa_table_reset(&ipv6ct_table->table);

	ipa_ipv6ct_create_table_dma_cmd_helpers(ipv6ct_table, table_index);

	IPADBG("return\n");
	return 0;

bail:
	memset(ipv6ct_table, 0, sizeof(*ipv6ct_table));
	return ret;
}

static int ipa_ipv6ct_destroy_table(ipa_ipv6ct_table* ipv6ct_table)
{
	int ret;

	IPADBG("\n");

	ret = ipa_mem_descriptor_delete(&ipv6ct_table->mem_desc, ipv6ct.ipa_desc->fd);
	if (ret)
		IPAERR("unable to delete IPV6CT descriptor\n");

	memset(ipv6ct_table, 0, sizeof(*ipv6ct_table));

	IPADBG("return\n");
	return ret;
}

/**
 * ipa_ipv6ct_create_table_dma_cmd_helpers() -
 *   Creates dma_cmd_helpers for base table in the received IPv6CT table
 * @ipv6ct_table: [in] IPv6CT table
 * @table_indx: [in] The index of the IPv6CT table
 *
 * A DMA command helper helps to generate the DMA command for one
 * specific field change. Each table has 3 different types of field
 * change: update_head, update_entry and delete_head. This function
 * creates the helpers and updates the base table correspondingly.
 */
static void ipa_ipv6ct_create_table_dma_cmd_helpers(
	ipa_ipv6ct_table* ipv6ct_table,
	uint8_t table_indx )
{
	IPADBG("\n");

	ipa_table_dma_cmd_helper_init(
		&ipv6ct_table->table_dma_cmd_helpers[IPA_IPV6CT_TABLE_FLAGS],
		table_indx,
		IPA_IPV6CT_BASE_TBL,
		IPA_IPV6CT_EXPN_TBL,
		ipv6ct_table->mem_desc.addr_offset + IPA_IPV6CT_RULE_FLAG_FIELD_OFFSET);

	ipa_table_dma_cmd_helper_init(
		&ipv6ct_table->table_dma_cmd_helpers[IPA_IPV6CT_TABLE_NEXT_INDEX],
		table_indx,
		IPA_IPV6CT_BASE_TBL,
		IPA_IPV6CT_EXPN_TBL,
		ipv6ct_table->mem_desc.addr_offset + IPA_IPV6CT_RULE_NEXT_FIELD_OFFSET);

	ipa_table_dma_cmd_helper_init(
		&ipv6ct_table->table_dma_cmd_helpers[IPA_IPV6CT_TABLE_PROTOCOL],
		table_indx,
		IPA_IPV6CT_BASE_TBL,
		IPA_IPV6CT_EXPN_TBL,
		ipv6ct_table->mem_desc.addr_offset + IPA_IPV6CT_RULE_PROTO_FIELD_OFFSET);

	ipv6ct_table->table.dma_help[HELP_UPDATE_HEAD] =
		&ipv6ct_table->table_dma_cmd_helpers[IPA_IPV6CT_TABLE_FLAGS];
	ipv6ct_table->table.dma_help[HELP_UPDATE_ENTRY] =
		&ipv6ct_table->table_dma_cmd_helpers[IPA_IPV6CT_TABLE_NEXT_INDEX];
	ipv6ct_table->table.dma_help[HELP_DELETE_HEAD] =
		&ipv6ct_table->table_dma_cmd_helpers[IPA_IPV6CT_TABLE_PROTOCOL];

	IPADBG("return\n");
}

static int ipa_ipv6ct_post_init_cmd(ipa_ipv6ct_table* ipv6ct_table, uint8_t tbl_index)
{
	struct ipa_ioc_ipv6ct_init cmd;
	int ret;

	IPADBG("\n");

	cmd.tbl_index = tbl_index;

	cmd.base_table_offset = ipv6ct_table->mem_desc.addr_offset;
	cmd.expn_table_offset = cmd.base_table_offset + (ipv6ct_table->table.table_entries * sizeof(ipa_ipv6ct_hw_entry));

	/* Driverr/HW expected base table size to be power^2-1 due to H/W hash calculation */
	cmd.table_entries = ipv6ct_table->table.table_entries - 1;
	cmd.expn_table_entries = ipv6ct_table->table.expn_table_entries;

	ret = ioctl(ipv6ct.ipa_desc->fd, IPA_IOC_INIT_IPV6CT_TABLE, &cmd);
	if (ret)
	{
		IPAERR("unable to post init cmd Error: %d IPA fd %d\n", ret, ipv6ct.ipa_desc->fd);
		return ret;
	}

	IPADBG("Posted IPA_IOC_INIT_IPV6CT_TABLE to kernel successfully\n");
	return 0;
}

static int ipa_ipv6ct_post_dma_cmd(struct ipa_ioc_nat_dma_cmd* cmd)
{
	IPADBG("\n");

	cmd->mem_type = IPA_NAT_MEM_IN_DDR;

	if (ioctl(ipv6ct.ipa_desc->fd, IPA_IOC_TABLE_DMA_CMD, cmd))
	{
		IPAERR("ioctl (IPA_IOC_TABLE_DMA_CMD) on fd %d has failed\n",
			   ipv6ct.ipa_desc->fd);
		return -EIO;
	}
	IPADBG("posted IPA_IOC_TABLE_DMA_CMD to kernel successfully\n");
	return 0;
}

void ipa_ipv6ct_dump_table(uint32_t table_handle)
{
	ipa_ipv6ct_table* ipv6ct_table;

	if (ipv6ct.ipa_desc->ver < IPA_HW_v4_0)
	{
		IPAERR("IPv6 connection tracking isn't supported for IPA version %d\n", ipv6ct.ipa_desc->ver);
		return;
	}

	if (table_handle == IPA_TABLE_INVALID_ENTRY || table_handle > IPA_IPV6CT_MAX_TBLS)
	{
		IPAERR("invalid parameters passed %d\n", table_handle);
		return;
	}

	if (pthread_mutex_lock(&ipv6ct_mutex))
	{
		IPAERR("unable to lock the ipv6ct mutex\n");
		return;
	}

	ipv6ct_table = &ipv6ct.tables[table_handle - 1];
	if (!ipv6ct_table->mem_desc.valid)
	{
		IPAERR("invalid table handle %d\n", table_handle);
		goto unlock;
	}

	/* Prevents interleaving with later kernel printouts. Flush doesn't help. */
	sleep(1);
	ipa_read_debug_info(IPA_IPV6CT_DEBUG_FILE_PATH);
	ipa_read_debug_info(IPA_UC_ACT_DEBUG_FILE_PATH);
	sleep(1);

unlock:
	if (pthread_mutex_unlock(&ipv6ct_mutex))
		IPAERR("unable to unlock the ipv6ct mutex\n");
}

/**
 * ipa_ipv6ct_add_uc_act_entry() - add uc activation entry
 * @u: [in] structure specifying the uC activation entry
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_ipv6ct_add_uc_act_entry(union ipa_ioc_uc_activation_entry *u)
{
	IPADBG("\n");

	if(ioctl(ipv6ct.ipa_desc->fd, IPA_IOC_ADD_UC_ACT_ENTRY, u))
	{
		IPAERR("ioctl (IPA_IOC_ADD_UC_ACT_ENTRY) on fd %d has failed\n",
			ipv6ct.ipa_desc->fd);
		return -EIO;
	}
	IPADBG("posted IPA_IOC_ADD_UC_ACT_ENTRY to kernel successfully, index %d\n",
		u->ipv6_nat.index);
	return 0;
}

/**
 * ipa_ipv6ct_del_uc_act_entry() - del uc activation entry
 * @index: [in] index of the uc activation entry to be removed
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_ipv6ct_del_uc_act_entry(uint16_t index)
{
	IPADBG("\n");

	if(ioctl(ipv6ct.ipa_desc->fd, IPA_IOC_DEL_UC_ACT_ENTRY, index))
	{
		IPAERR("ioctl (IPA_IOC_DEL_UC_ACT_ENTRY) on fd %d has failed\n",
			ipv6ct.ipa_desc->fd);
		return -EIO;
	}
	IPADBG("posted IPA_IOC_DEL_UC_ACT_ENTRY to kernel successfully, index %d\n",
		index);
	return 0;
}
