/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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

#include "ipa_nat_drv.h"
#include "ipa_nat_drvi.h"

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <linux/msm_ipa.h>

#define IPA_NAT_DEBUG_FILE_PATH "/sys/kernel/debug/ipa/ip4_nat"
#define IPA_NAT_TABLE_NAME "IPA NAT table"
#define IPA_NAT_INDEX_TABLE_NAME "IPA NAT index table"


#undef min
#define min(a, b) ((a) < (b)) ? (a) : (b)

#undef max
#define max(a, b) ((a) > (b)) ? (a) : (b)

static struct ipa_nat_cache ipv4_nat_cache[IPA_NAT_MEM_IN_MAX];

static struct ipa_nat_cache *active_nat_cache_ptr = NULL;

#undef DDR_IS_ACTIVE
#define DDR_IS_ACTIVE() \
	(active_nat_cache_ptr) ? \
	(active_nat_cache_ptr->nmi == IPA_NAT_MEM_IN_DDR) : \
	false

#undef  SRAM_IS_ACTIVE
#define SRAM_IS_ACTIVE() \
	(active_nat_cache_ptr) ? \
	(active_nat_cache_ptr->nmi == IPA_NAT_MEM_IN_SRAM) : \
	false

extern pthread_mutex_t nat_mutex;

static ipa_nat_pdn_entry pdns[IPA_MAX_PDN_NUM];
static int num_pdns = 0;
static int Hash_token = 69;
/*
 * ----------------------------------------------------------------------------
 * Private helpers for manipulating regular tables
 * ----------------------------------------------------------------------------
 */
static int table_entry_is_valid(
	void* entry)
{
	struct ipa_nat_rule* rule = (struct ipa_nat_rule*) entry;

	IPADBG("In\n");

	IPADBG("enable(%u)\n", rule->enable);

	IPADBG("Out\n");

	return rule->enable;
}

static uint16_t table_entry_get_next_index(
	void* entry)
{
	uint16_t result;
	struct ipa_nat_rule* rule = (struct ipa_nat_rule*)entry;

	IPADBG("In\n");

	result = rule->next_index;

	IPADBG("Next entry of %pK is %u\n", entry, result);

	IPADBG("Out\n");

	return result;
}

static uint16_t table_entry_get_prev_index(
	void* entry,
	uint16_t entry_index,
	void* meta,
	uint16_t base_table_size)
{
	uint16_t result;
	struct ipa_nat_rule* rule = (struct ipa_nat_rule*)entry;

	IPADBG("In\n");

	result = rule->prev_index;

	IPADBG("Previous entry of %u is %u\n", entry_index, result);

	IPADBG("Out\n");

	return result;
}

static void table_entry_set_prev_index(
	void*    entry,
	uint16_t entry_index,
	uint16_t prev_index,
	void*    meta,
	uint16_t base_table_size)
{
	struct ipa_nat_rule* rule = (struct ipa_nat_rule*) entry;

	IPADBG("In\n");

	IPADBG("Previous entry of %u is %u\n", entry_index, prev_index);

	rule->prev_index = prev_index;

	IPADBG("Out\n");
}

/**
 * ipa_nati_calc_ip_cksum() - Calculate the source nat IP checksum diff
 * @pub_ip_addr: [in] public ip address
 * @priv_ip_addr: [in]	Private ip address
 *
 * source nat ip checksum different is calculated as
 * public_ip_addr - private_ip_addr
 * Here we are using 1's complement to represent -ve number.
 * So take 1's complement of private ip addr and add it
 * to public ip addr.
 *
 * Returns: >0 ip checksum diff
 */
static uint16_t ipa_nati_calc_ip_cksum(
	uint32_t pub_ip_addr,
	uint32_t priv_ip_addr)
{
	uint16_t ret;
	uint32_t cksum = 0;

	IPADBG("In\n");

	/* Add LSB(2 bytes) of public ip address to cksum */
	cksum += (pub_ip_addr & 0xFFFF);

	/* Add MSB(2 bytes) of public ip address to cksum
		and check for carry forward(CF), if any add it
	*/
	cksum += (pub_ip_addr>>16);
	if (cksum >> 16) {
		cksum = (cksum & 0x0000FFFF);
		cksum += 1;
	}

	/* Calculate the 1's complement of private ip address */
	priv_ip_addr = (~priv_ip_addr);

	/* Add LSB(2 bytes) of private ip address to cksum
		 and check for carry forward(CF), if any add it
	*/
	cksum += (priv_ip_addr & 0xFFFF);
	if (cksum >> 16) {
		cksum = (cksum & 0x0000FFFF);
		cksum += 1;
	}

	/* Add MSB(2 bytes) of private ip address to cksum
		 and check for carry forward(CF), if any add it
	*/
	cksum += (priv_ip_addr>>16);
	if (cksum >> 16) {
		cksum = (cksum & 0x0000FFFF);
		cksum += 1;
	}

	/* Return the LSB(2 bytes) of checksum	*/
	ret = (uint16_t)cksum;

	IPADBG("Out\n");

	return ret;
}

/**
 * ipa_nati_calc_tcp_udp_cksum() - Calculate the source nat TCP/UDP checksum diff
 * @pub_ip_addr: [in] public ip address
 * @pub_port: [in] public tcp/udp port
 * @priv_ip_addr: [in]	Private ip address
 * @priv_port: [in] Private tcp/udp prot
 *
 * source nat tcp/udp checksum is calculated as
 * (pub_ip_addr + pub_port) - (priv_ip_addr + priv_port)
 * Here we are using 1's complement to represent -ve number.
 * So take 1's complement of prviate ip addr &private port
 * and add it public ip addr & public port.
 *
 * Returns: >0 tcp/udp checksum diff
 */
static uint16_t ipa_nati_calc_tcp_udp_cksum(
	uint32_t pub_ip_addr,
	uint16_t pub_port,
	uint32_t priv_ip_addr,
	uint16_t priv_port)
{
	uint16_t ret = 0;
	uint32_t cksum = 0;

	IPADBG("In\n");

	/* Add LSB(2 bytes) of public ip address to cksum */
	cksum += (pub_ip_addr & 0xFFFF);

	/* Add MSB(2 bytes) of public ip address to cksum
		and check for carry forward(CF), if any add it
	*/
	cksum += (pub_ip_addr>>16);
	if (cksum >> 16) {
		cksum = (cksum & 0x0000FFFF);
		cksum += 1;
	}

	/* Add public port to cksum and
		 check for carry forward(CF), if any add it */
	cksum += pub_port;
	if (cksum >> 16) {
		cksum = (cksum & 0x0000FFFF);
		cksum += 1;
	}

	/* Calculate the 1's complement of private ip address */
	priv_ip_addr = (~priv_ip_addr);

	/* Add LSB(2 bytes) of private ip address to cksum
		 and check for carry forward(CF), if any add it
	*/
	cksum += (priv_ip_addr & 0xFFFF);
	if (cksum >> 16) {
		cksum = (cksum & 0x0000FFFF);
		cksum += 1;
	}

	/* Add MSB(2 bytes) of private ip address to cksum
		 and check for carry forward(CF), if any add
	*/
	cksum += (priv_ip_addr>>16);
	if (cksum >> 16) {
		cksum = (cksum & 0x0000FFFF);
		cksum += 1;
	}

	/* Calculate the 1's complement of private port */
	priv_port = (~priv_port);

	/* Add public port to cksum and
	 check for carry forward(CF), if any add it */
	cksum += priv_port;
	if (cksum >> 16) {
		cksum = (cksum & 0x0000FFFF);
		cksum += 1;
	}

	/* return the LSB(2 bytes) of checksum */
	ret = (uint16_t)cksum;

	IPADBG("Out\n");

	return ret;
}

static int table_entry_copy_from_user(
	void* entry,
	void* user_data)
{
	uint32_t pub_ip_addr;

	struct ipa_nat_rule*     nat_entry = (struct ipa_nat_rule*) entry;
	const ipa_nat_ipv4_rule* user_rule = (const ipa_nat_ipv4_rule*) user_data;

	IPADBG("In\n");

	pub_ip_addr = pdns[user_rule->pdn_index].public_ip;

	nat_entry->private_ip   = user_rule->private_ip;
	nat_entry->private_port = user_rule->private_port;
	nat_entry->protocol     = user_rule->protocol;
	nat_entry->public_port  = user_rule->public_port;
	nat_entry->target_ip    = user_rule->target_ip;
	nat_entry->target_port  = user_rule->target_port;
	nat_entry->pdn_index    = user_rule->pdn_index;
	nat_entry->uc_activation_index = user_rule->uc_activation_index;
	nat_entry->s = user_rule->s;
	nat_entry->ucp = user_rule->ucp;
	nat_entry->dst_only = user_rule->dst_only;
	nat_entry->src_only = user_rule->src_only;

	nat_entry->ip_chksum =
		ipa_nati_calc_ip_cksum(pub_ip_addr, user_rule->private_ip);

	if (IPPROTO_TCP == nat_entry->protocol ||
		IPPROTO_UDP == nat_entry->protocol) {
		nat_entry->tcp_udp_chksum = ipa_nati_calc_tcp_udp_cksum(
			pub_ip_addr,
			user_rule->public_port,
			user_rule->private_ip,
			user_rule->private_port);
	}

	IPADBG("Out\n");

	return 0;
}

static int table_entry_head_insert(
	void*      entry,
	void*      user_data,
	uint16_t*  dma_command_data)
{
	int  ret;
	struct ipa_nat_rule* nat_entry = (struct ipa_nat_rule*) entry;

	IPADBG("In\n");

	IPADBG("entry(%p) user_data(%p) dma_command_data(%p)\n",
		   entry,
		   user_data,
		   dma_command_data);

	ret = table_entry_copy_from_user(entry, user_data);

	if (ret) {
		IPAERR("unable to copy from user a new entry\n");
		goto bail;
	}

	*dma_command_data = 0;
	((ipa_nat_flags*)dma_command_data)->enable = IPA_NAT_FLAG_ENABLE_BIT;
	((ipa_nat_flags*)dma_command_data)->uc_activation_index =
		nat_entry->uc_activation_index;
	((ipa_nat_flags*)dma_command_data)->s = nat_entry->s;
;

bail:
	IPADBG("Out\n");

	return ret;
}

static int table_entry_tail_insert(
	void* entry,
	void* user_data)
{
	struct ipa_nat_rule* nat_entry = (struct ipa_nat_rule*) entry;

	int  ret;

	IPADBG("In\n");

	IPADBG("entry(%p) user_data(%p)\n",
		   entry,
		   user_data);

	ret = table_entry_copy_from_user(entry, user_data);

	if (ret) {
		IPAERR("unable to copy from user a new entry\n");
		goto bail;
	}

	nat_entry->enable = IPA_NAT_FLAG_ENABLE_BIT;

bail:
	IPADBG("Out\n");

	return ret;
}

static uint16_t table_entry_get_delete_head_dma_command_data(
	void* head,
	void* next_entry)
{
	IPADBG("In\n");

	IPADBG("Out\n");

	return IPA_NAT_INVALID_PROTO_FIELD_VALUE;
}

/*
 * ----------------------------------------------------------------------------
 * Private helpers for manipulating index tables
 * ----------------------------------------------------------------------------
 */
static int index_table_entry_is_valid(
	void* entry)
{
	struct ipa_nat_indx_tbl_rule* rule =
		(struct ipa_nat_indx_tbl_rule*) entry;

	int ret;

	IPADBG("In\n");

	ret = (rule->tbl_entry) ? 1 : 0;

	IPADBG("enable(%d)\n", ret);

	IPADBG("Out\n");

	return ret;
}

static uint16_t index_table_entry_get_next_index(
	void* entry)
{
	uint16_t result;
	struct ipa_nat_indx_tbl_rule* rule = (struct ipa_nat_indx_tbl_rule*)entry;

	IPADBG("In\n");

	result = rule->next_index;

	IPADBG("Next entry of %pK is %d\n", entry, result);

	IPADBG("Out\n");

	return result;
}

static uint16_t index_table_entry_get_prev_index(
	void* entry,
	uint16_t entry_index,
	void* meta,
	uint16_t base_table_size)
{
	uint16_t result = 0;
	struct ipa_nat_indx_tbl_meta_info* index_expn_table_meta =
		(struct ipa_nat_indx_tbl_meta_info*)meta;

	IPADBG("In\n");

	if (entry_index >= base_table_size)
		result = index_expn_table_meta[entry_index - base_table_size].prev_index;

	IPADBG("Previous entry of %d is %d\n", entry_index, result);

	IPADBG("Out\n");

	return result;
}

static void index_table_entry_set_prev_index(
	void*    entry,
	uint16_t entry_index,
	uint16_t prev_index,
	void*    meta,
	uint16_t base_table_size)
{
	struct ipa_nat_indx_tbl_meta_info* index_expn_table_meta =
		(struct ipa_nat_indx_tbl_meta_info*) meta;

	IPADBG("In\n");

	IPADBG("Previous entry of %u is %u\n", entry_index, prev_index);

	if ( entry_index >= base_table_size )
	{
		index_expn_table_meta[entry_index - base_table_size].prev_index = prev_index;
	}
	else if ( VALID_INDEX(prev_index) )
	{
		IPAERR("Base table entry %u can't has prev entry %u, but only %u",
			   entry_index, prev_index, IPA_TABLE_INVALID_ENTRY);
	}

	IPADBG("Out\n");
}

static int index_table_entry_head_insert(
	void*      entry,
	void*      user_data,
	uint16_t*  dma_command_data)
{
	IPADBG("In\n");

	IPADBG("entry(%p) user_data(%p) dma_command_data(%p)\n",
		   entry,
		   user_data,
		   dma_command_data);

	*dma_command_data = *((uint16_t*)user_data);

	IPADBG("Out\n");

	return 0;
}

static int index_table_entry_tail_insert(
	void* entry,
	void* user_data)
{
	struct ipa_nat_indx_tbl_rule* rule_ptr =
		(struct ipa_nat_indx_tbl_rule*) entry;

	IPADBG("In\n");

	IPADBG("entry(%p) user_data(%p)\n",
		   entry,
		   user_data);

	rule_ptr->tbl_entry = *((uint16_t*)user_data);

	IPADBG("Out\n");

	return 0;
}

static uint16_t index_table_entry_get_delete_head_dma_command_data(
	void* head,
	void* next_entry)
{
	uint16_t result;
	struct ipa_nat_indx_tbl_rule* rule =
		(struct ipa_nat_indx_tbl_rule*)next_entry;

	IPADBG("In\n");

	result = rule->tbl_entry;

	IPADBG("Out\n");

	return result;
}

/*
 * ----------------------------------------------------------------------------
 * Private data and functions used by this file's API
 * ----------------------------------------------------------------------------
 */
static ipa_table_entry_interface entry_interface = {
	table_entry_is_valid,
	table_entry_get_next_index,
	table_entry_get_prev_index,
	table_entry_set_prev_index,
	table_entry_head_insert,
	table_entry_tail_insert,
	table_entry_get_delete_head_dma_command_data
};

static ipa_table_entry_interface index_entry_interface = {
	index_table_entry_is_valid,
	index_table_entry_get_next_index,
	index_table_entry_get_prev_index,
	index_table_entry_set_prev_index,
	index_table_entry_head_insert,
	index_table_entry_tail_insert,
	index_table_entry_get_delete_head_dma_command_data
};

/**
 * ipa_nati_create_table_dma_cmd_helpers()
 *
 *   Creates dma_cmd_helpers for base and index tables in the received
 *   NAT table
 *
 * @nat_table: [in] NAT table
 * @table_indx: [in] The index of the NAT table
 *
 * A DMA command helper helps to generate the DMA command for one
 * specific field change. Each table has 3 different types of field
 * change: update_head, update_entry and delete_head. This function
 * creates the helpers for base and index tables and updates the
 * tables correspondingly.
 */
static void ipa_nati_create_table_dma_cmd_helpers(
	struct ipa_nat_ip4_table_cache* nat_table,
	uint8_t table_indx)
{
	IPADBG("In\n");

	/*
	 * Create helpers for base table
	 */
	ipa_table_dma_cmd_helper_init(
		&nat_table->table_dma_cmd_helpers[IPA_NAT_TABLE_FLAGS],
		table_indx,
		IPA_NAT_BASE_TBL,
		IPA_NAT_EXPN_TBL,
		nat_table->mem_desc.addr_offset + IPA_NAT_RULE_FLAG_FIELD_OFFSET);

	ipa_table_dma_cmd_helper_init(
		&nat_table->table_dma_cmd_helpers[IPA_NAT_TABLE_NEXT_INDEX],
		table_indx,
		IPA_NAT_BASE_TBL,
		IPA_NAT_EXPN_TBL,
		nat_table->mem_desc.addr_offset + IPA_NAT_RULE_NEXT_FIELD_OFFSET);

	ipa_table_dma_cmd_helper_init(
		&nat_table->table_dma_cmd_helpers[IPA_NAT_TABLE_PROTOCOL],
		table_indx,
		IPA_NAT_BASE_TBL,
		IPA_NAT_EXPN_TBL,
		nat_table->mem_desc.addr_offset + IPA_NAT_RULE_PROTO_FIELD_OFFSET);

	/*
	 * Create helpers for index table
	 */
	ipa_table_dma_cmd_helper_init(
		&nat_table->table_dma_cmd_helpers[IPA_NAT_INDEX_TABLE_ENTRY],
		table_indx,
		IPA_NAT_INDX_TBL,
		IPA_NAT_INDEX_EXPN_TBL,
		nat_table->mem_desc.addr_offset + IPA_NAT_INDEX_RULE_NAT_INDEX_FIELD_OFFSET);

	ipa_table_dma_cmd_helper_init(
		&nat_table->table_dma_cmd_helpers[IPA_NAT_INDEX_TABLE_NEXT_INDEX],
		table_indx,
		IPA_NAT_INDX_TBL,
		IPA_NAT_INDEX_EXPN_TBL,
		nat_table->mem_desc.addr_offset + IPA_NAT_INDEX_RULE_NEXT_FIELD_OFFSET);

	/*
	 * Init helpers for base table
	 */
	nat_table->table.dma_help[HELP_UPDATE_HEAD] =
		&nat_table->table_dma_cmd_helpers[IPA_NAT_TABLE_FLAGS];

	nat_table->table.dma_help[HELP_UPDATE_ENTRY] =
		&nat_table->table_dma_cmd_helpers[IPA_NAT_TABLE_NEXT_INDEX];

	nat_table->table.dma_help[HELP_DELETE_HEAD] =
		&nat_table->table_dma_cmd_helpers[IPA_NAT_TABLE_PROTOCOL];

	/*
	 * Init helpers for index table
	 */
	nat_table->index_table.dma_help[HELP_UPDATE_HEAD] =
		&nat_table->table_dma_cmd_helpers[IPA_NAT_INDEX_TABLE_ENTRY];

	nat_table->index_table.dma_help[HELP_UPDATE_ENTRY] =
		&nat_table->table_dma_cmd_helpers[IPA_NAT_INDEX_TABLE_NEXT_INDEX];

	nat_table->index_table.dma_help[HELP_DELETE_HEAD] =
		&nat_table->table_dma_cmd_helpers[IPA_NAT_INDEX_TABLE_ENTRY];

	IPADBG("Out\n");
}

/**
 * ipa_nati_create_table() - Creates a new IPv4 NAT table
 * @nat_table: [in] IPv4 NAT table
 * @public_ip_addr: [in] public IPv4 address
 * @number_of_entries: [in] number of NAT entries
 * @table_index: [in] the index of the IPv4 NAT table
 *
 * This function creates new IPv4 NAT table:
 * - Initializes table, index table, memory descriptor and
 *   table_dma_cmd_helpers structures
 * - Allocates the index expansion table meta data
 * - Allocates, maps and clears the memory for table and index table
 *
 * Returns:	0  On Success, negative on failure
 */
static int ipa_nati_create_table(
	struct ipa_nat_cache*           nat_cache_ptr,
	struct ipa_nat_ip4_table_cache* nat_table,
	uint32_t                        public_ip_addr,
	uint16_t                        number_of_entries,
	uint8_t                         table_index)
{
	int ret, size;
	void* base_addr;

#ifdef IPA_ON_R3PC
	uint32_t nat_mem_offset = 0;
#endif

	IPADBG("In\n");

	nat_table->public_addr = public_ip_addr;

	ipa_table_init(
		&nat_table->table,
		IPA_NAT_TABLE_NAME,
		nat_cache_ptr->nmi,
		sizeof(struct ipa_nat_rule),
		NULL,
		0,
		&entry_interface);

	ret = ipa_table_calculate_entries_num(
		&nat_table->table,
		number_of_entries,
		nat_cache_ptr->nmi);

	if (ret) {
		IPAERR(
			"unable to calculate number of entries in "
			"nat table %d, while required by user %d\n",
			table_index, number_of_entries);
		goto done;
	}

	/*
	 * Allocate memory for NAT index expansion table meta data
	 */
	nat_table->index_expn_table_meta = (struct ipa_nat_indx_tbl_meta_info*)
		calloc(nat_table->table.expn_table_entries,
			   sizeof(struct ipa_nat_indx_tbl_meta_info));

	if (nat_table->index_expn_table_meta == NULL) {
		IPAERR(
			"Fail to allocate ipv4 index expansion table meta with size %d\n",
			nat_table->table.expn_table_entries *
			sizeof(struct ipa_nat_indx_tbl_meta_info));
		ret = -ENOMEM;
		goto done;
	}

	ipa_table_init(
		&nat_table->index_table,
		IPA_NAT_INDEX_TABLE_NAME,
		nat_cache_ptr->nmi,
		sizeof(struct ipa_nat_indx_tbl_rule),
		nat_table->index_expn_table_meta,
		sizeof(struct ipa_nat_indx_tbl_meta_info),
		&index_entry_interface);

	nat_table->index_table.table_entries =
		nat_table->table.table_entries;

	nat_table->index_table.expn_table_entries =
		nat_table->table.expn_table_entries;

	nat_table->index_table.tot_tbl_ents =
		nat_table->table.tot_tbl_ents;

	size  = ipa_table_calculate_size(&nat_table->table);
	size += ipa_table_calculate_size(&nat_table->index_table);

	IPADBG("Nat Base and Index Table size: %d\n", size);

	ipa_mem_descriptor_init(
		&nat_table->mem_desc,
		IPA_NAT_DEV_NAME,
		size,
		table_index,
		IPA_IOC_ALLOC_NAT_TABLE,
		IPA_IOC_DEL_NAT_TABLE,
		true);  /* true here means do consider using sram */

	ret = ipa_mem_descriptor_allocate_memory(
		&nat_table->mem_desc,
		nat_cache_ptr->ipa_desc->fd);

	if (ret) {
		IPAERR("unable to allocate nat memory descriptor Error: %d\n", ret);
		goto bail_meta;
	}

	base_addr = nat_table->mem_desc.base_addr;

#ifdef IPA_ON_R3PC
	ret = ioctl(nat_cache_ptr->ipa_desc->fd,
				IPA_IOC_GET_NAT_OFFSET,
				&nat_mem_offset);
	if (ret) {
		IPAERR("unable to post ant offset cmd Error: %d IPA fd %d\n",
			   ret, nat_cache_ptr->ipa_desc->fd);
		goto bail_mem_desc;
	}
	base_addr += nat_mem_offset;
#endif

	base_addr =
		ipa_table_calculate_addresses(&nat_table->table, base_addr);
	ipa_table_calculate_addresses(&nat_table->index_table, base_addr);

	ipa_table_reset(&nat_table->table);
	ipa_table_reset(&nat_table->index_table);

	ipa_nati_create_table_dma_cmd_helpers(nat_table, table_index);

	goto done;

#ifdef IPA_ON_R3PC
bail_mem_desc:
	ipa_mem_descriptor_delete(&nat_table->mem_desc, nat_cache_ptr->ipa_desc->fd);
#endif

bail_meta:
	free(nat_table->index_expn_table_meta);
	memset(nat_table, 0, sizeof(*nat_table));

done:
	IPADBG("Out\n");

	return ret;
}

static int ipa_nati_destroy_table(
	struct ipa_nat_cache*           nat_cache_ptr,
	struct ipa_nat_ip4_table_cache* nat_table)
{
	int ret;

	IPADBG("In\n");

	ret = ipa_mem_descriptor_delete(
		&nat_table->mem_desc, nat_cache_ptr->ipa_desc->fd);

	if (ret)
		IPAERR("unable to delete NAT descriptor\n");

	free(nat_table->index_expn_table_meta);

	memset(nat_table, 0, sizeof(*nat_table));

	IPADBG("Out\n");

	return ret;
}

static int ipa_nati_post_ipv4_init_cmd(
	struct ipa_nat_cache*           nat_cache_ptr,
	struct ipa_nat_ip4_table_cache* nat_table,
	uint8_t                         tbl_index,
	bool                            focus_change )
{
	struct ipa_ioc_v4_nat_init cmd;

	char buf[1024];
	int  ret;

	IPADBG("In\n");

	IPADBG("nat_cache_ptr(%p) nat_table(%p) tbl_index(%u) focus_change(%u)\n",
		   nat_cache_ptr, nat_table, tbl_index, focus_change);

	memset(&cmd, 0, sizeof(cmd));

	cmd.tbl_index    = tbl_index;
	cmd.focus_change = focus_change;

	cmd.mem_type = nat_cache_ptr->nmi;

	cmd.ipv4_rules_offset =
		nat_table->mem_desc.addr_offset;

	cmd.expn_rules_offset =
		cmd.ipv4_rules_offset +
		(nat_table->table.table_entries * sizeof(struct ipa_nat_rule));

	cmd.index_offset =
		cmd.expn_rules_offset +
		(nat_table->table.expn_table_entries * sizeof(struct ipa_nat_rule));

	cmd.index_expn_offset =
		cmd.index_offset +
		(nat_table->index_table.table_entries * sizeof(struct ipa_nat_indx_tbl_rule));

	/*
	 * Driverr/HW expected base table size to be power^2-1 due to H/W
	 * hash calculation
	 */
	cmd.table_entries =
		nat_table->table.table_entries - 1;
	cmd.expn_table_entries =
		nat_table->table.expn_table_entries;

	cmd.ip_addr = nat_table->public_addr;

	IPADBG("%s\n", ipa_ioc_v4_nat_init_as_str(&cmd, buf, sizeof(buf)));

	ret = ioctl(nat_cache_ptr->ipa_desc->fd, IPA_IOC_V4_INIT_NAT, &cmd);

	if (ret) {
		IPAERR("unable to post init cmd Error: %d IPA fd %d\n",
			   ret, nat_cache_ptr->ipa_desc->fd);
		goto bail;
	}

	IPADBG("Posted IPA_IOC_V4_INIT_NAT to kernel successfully\n");

bail:
	IPADBG("Out\n");

	return ret;
}

static void ipa_nati_copy_second_index_entry_to_head(
	struct ipa_nat_ip4_table_cache* nat_table,
	ipa_table_iterator* index_table_iterator,
	struct ipa_ioc_nat_dma_cmd* cmd)
{
	uint16_t index;
	struct ipa_nat_rule* table;
	struct ipa_nat_indx_tbl_rule* index_table_rule =
		(struct ipa_nat_indx_tbl_rule*)index_table_iterator->next_entry;

	IPADBG("In\n");

	/*
	 * The DMA command for field tbl_entry already added by the
	 * index_table.ipa_table_create_delete_command()
	 */
	ipa_table_add_dma_cmd(
		&nat_table->index_table,
		HELP_UPDATE_ENTRY,
		index_table_iterator->curr_entry,
		index_table_iterator->curr_index,
		index_table_rule->next_index,
		cmd);

	/* Change the indx_tbl_entry field in the related table rule */
	if (index_table_rule->tbl_entry < nat_table->table.table_entries) {
		index = index_table_rule->tbl_entry;
		table = (struct ipa_nat_rule*)nat_table->table.table_addr;
	} else {
		index = index_table_rule->tbl_entry - nat_table->table.table_entries;
		table = (struct ipa_nat_rule*)nat_table->table.expn_table_addr;
	}

	table[index].indx_tbl_entry = index_table_iterator->curr_index;

	IPADBG("Out\n");
}

/**
 * dst_hash() - Find the index into ipv4 base table
 * @public_ip: [in] public_ip
 * @trgt_ip: [in] Target IP address
 * @trgt_port: [in]  Target port
 * @public_port: [in]  Public port
 * @proto: [in] Protocol (TCP/IP)
 * @size: [in] size of the ipv4 base Table
 *
 * This hash method is used to find the hash index of new nat
 * entry into ipv4 base table. In case of zero index, the
 * new entry will be stored into N-1 index where N is size of
 * ipv4 base table
 *
 * Returns: >0 index into ipv4 base table, negative on failure
 */
static uint16_t dst_hash(
	struct ipa_nat_cache* nat_cache_ptr,
	uint32_t public_ip,
	uint32_t trgt_ip,
	uint16_t trgt_port,
	uint16_t public_port,
	uint8_t  proto,
	uint16_t size)
{
	uint16_t hash =
		((uint16_t)(trgt_ip))       ^
		((uint16_t)(trgt_ip >> 16)) ^
		(trgt_port)                 ^
		(public_port)               ^
		(proto);

	IPADBG("In\n");

	IPADBG("public_ip: 0x%08X public_port: 0x%04X\n", public_ip, public_port);
	IPADBG("target_ip: 0x%08X target_port: 0x%04X\n", trgt_ip, trgt_port);
	IPADBG("proto: 0x%02X size: 0x%04X\n", proto, size);

	if (nat_cache_ptr->ipa_desc->ver >= IPA_HW_v4_0)
		hash ^=
			((uint16_t)(public_ip)) ^
			((uint16_t)(public_ip >> 16));

	/*
	 * The size passed to hash function expected be power^2-1, while
	 * the actual size is power^2, actual_size = size + 1
	 */
	hash = (hash & size);

	/*
	 * If the hash resulted to zero then set it to maximum value as
	 * zero is unused entry in nat tables
	 */
	if (hash == 0) {
		hash = size;
	}

	IPADBG("dst_hash returning value: %d\n", hash);

	IPADBG("Out\n");

	return hash;
}

/**
 * src_hash() - Find the index into ipv4 index base table
 * @priv_ip: [in] Private IP address
 * @priv_port: [in]  Private port
 * @trgt_ip: [in]  Target IP address
 * @trgt_port: [in] Target Port
 * @proto: [in]  Protocol (TCP/IP)
 * @size: [in] size of the ipv4 index base Table
 *
 * This hash method is used to find the hash index of new nat
 * entry into ipv4 index base table. In case of zero index, the
 * new entry will be stored into N-1 index where N is size of
 * ipv4 index base table
 *
 * Returns: >0 index into ipv4 index base table, negative on failure
 */
static uint16_t src_hash(
	uint32_t priv_ip,
	uint16_t priv_port,
	uint32_t trgt_ip,
	uint16_t trgt_port,
	uint8_t  proto,
	uint16_t size)
{
	uint16_t hash =
		((uint16_t)(priv_ip))       ^
		((uint16_t)(priv_ip >> 16)) ^
		(priv_port)                 ^
		((uint16_t)(trgt_ip))       ^
		((uint16_t)(trgt_ip >> 16)) ^
		(trgt_port)                 ^
		(proto);

	IPADBG("In\n");

	IPADBG("private_ip: 0x%08X private_port: 0x%04X\n", priv_ip, priv_port);
	IPADBG(" target_ip: 0x%08X  target_port: 0x%04X\n", trgt_ip, trgt_port);
	IPADBG("proto: 0x%02X size: 0x%04X\n", proto, size);

	/*
	 * The size passed to hash function expected be power^2-1, while
	 * the actual size is power^2, actual_size = size + 1
	 */
	hash = (hash & size);

	/*
	 * If the hash resulted to zero then set it to maximum value as
	 * zero is unused entry in nat tables
	 */
	if (hash == 0) {
		hash = size;
	}

	IPADBG("src_hash returning value: %d\n", hash);

	IPADBG("Out\n");

	return hash;
}

static int ipa_nati_post_ipv4_dma_cmd(
	struct ipa_nat_cache*       nat_cache_ptr,
	struct ipa_ioc_nat_dma_cmd* cmd)
{
	char buf[4096];
	int  ret = 0;

	IPADBG("In\n");

	cmd->mem_type = nat_cache_ptr->nmi;

	IPADBG("%s\n", prep_ioc_nat_dma_cmd_4print(cmd, buf, sizeof(buf)));

	if (ioctl(nat_cache_ptr->ipa_desc->fd, IPA_IOC_TABLE_DMA_CMD, cmd)) {
		IPAERR("ioctl (IPA_IOC_TABLE_DMA_CMD) on fd %d has failed\n",
			   nat_cache_ptr->ipa_desc->fd);
		ret = -EIO;
		goto bail;
	}

	IPADBG("Posted IPA_IOC_TABLE_DMA_CMD to kernel successfully\n");

bail:
	IPADBG("Out\n");

	return ret;
}

/*
 * ----------------------------------------------------------------------------
 * API functions exposed to the upper layers
 * ----------------------------------------------------------------------------
 */
int ipa_nati_modify_pdn(
	struct ipa_ioc_nat_pdn_entry *entry)
{
	struct ipa_nat_cache* nat_cache_ptr;
	int ret = 0;

	IPADBG("In\n");

	nat_cache_ptr =
		(ipv4_nat_cache[IPA_NAT_MEM_IN_DDR].ipa_desc) ?
		&ipv4_nat_cache[IPA_NAT_MEM_IN_DDR]           :
		&ipv4_nat_cache[IPA_NAT_MEM_IN_SRAM];

	if ( nat_cache_ptr->ipa_desc == NULL )
	{
		IPAERR("Uninitialized cache file descriptor\n");
		ret = -EIO;
		goto done;
	}

	if (entry->public_ip == 0)
		IPADBG("PDN %d public ip will be set  to 0\n", entry->pdn_index);

	ret = ioctl(nat_cache_ptr->ipa_desc->fd, IPA_IOC_NAT_MODIFY_PDN, entry);

	if ( ret ) {
		IPAERR("unable to call modify pdn icotl\nindex %d, ip 0x%X, src_metdata 0x%X, dst_metadata 0x%X IPA fd %d\n",
			   entry->pdn_index,
			   entry->public_ip,
			   entry->src_metadata,
			   entry->dst_metadata,
			   nat_cache_ptr->ipa_desc->fd);
		goto done;
	}

	pdns[entry->pdn_index].public_ip    = entry->public_ip;
	pdns[entry->pdn_index].dst_metadata = entry->dst_metadata;
	pdns[entry->pdn_index].src_metadata = entry->src_metadata;

	IPADBG("posted IPA_IOC_NAT_MODIFY_PDN to kernel successfully and stored in cache\n index %d, ip 0x%X, src_metdata 0x%X, dst_metadata 0x%X\n",
		   entry->pdn_index,
		   entry->public_ip,
		   entry->src_metadata,
		   entry->dst_metadata);
done:
	IPADBG("Out\n");

	return ret;
}

int ipa_nati_get_pdn_index(
	uint32_t public_ip,
	uint8_t *pdn_index)
{
	int i = 0;

	for(i = 0; i < (IPA_MAX_PDN_NUM - 1); i++) {
		if(pdns[i].public_ip == public_ip) {
			IPADBG("ip 0x%X matches PDN index %d\n", public_ip, i);
			*pdn_index = i;
			return 0;
		}
	}

	IPAERR("ip 0x%X does not match any PDN\n", public_ip);

	return -EIO;
}

int ipa_nati_alloc_pdn(
	ipa_nat_pdn_entry *pdn_info,
	uint8_t *pdn_index)
{
	ipa_nat_pdn_entry zero_test;
	struct ipa_ioc_nat_pdn_entry pdn_data;
	int i, ret;

	IPADBG("alloc PDN  for ip 0x%x\n", pdn_info->public_ip);

	memset(&zero_test, 0, sizeof(zero_test));

	if(num_pdns >= (IPA_MAX_PDN_NUM - 1)) {
		IPAERR("exceeded max num of PDNs, num_pdns %d\n", num_pdns);
		return -EIO;
	}

	for(i = 0; i < (IPA_MAX_PDN_NUM - 1); i++) {
		if(pdns[i].public_ip == pdn_info->public_ip)
		{
			IPADBG("found the same pdn in index %d\n", i);
			*pdn_index = i;
			if((pdns[i].src_metadata != pdn_info->src_metadata) ||
			   (pdns[i].dst_metadata != pdn_info->dst_metadata))
			{
				IPAERR("WARNING: metadata values don't match! [%d, %d], [%d, %d]\n\n",
					   pdns[i].src_metadata, pdn_info->src_metadata,
					   pdns[i].dst_metadata, pdn_info->dst_metadata);
			}
			return 0;
		}

		if(!memcmp((pdns + i), &zero_test, sizeof(ipa_nat_pdn_entry)))
		{
			IPADBG("found an empty pdn in index %d\n", i);
			break;
		}
	}

	if(i >= (IPA_MAX_PDN_NUM - 1))
	{
		IPAERR("couldn't find an empty entry while num is %d\n",
			   num_pdns);
		return -EIO;
	}

	pdn_data.pdn_index    = i;
	pdn_data.public_ip    = pdn_info->public_ip;
	pdn_data.src_metadata = pdn_info->src_metadata;
	pdn_data.dst_metadata = pdn_info->dst_metadata;

	ret = ipa_nati_modify_pdn(&pdn_data);
	if(!ret)
	{
		num_pdns++;
		*pdn_index = i;
		IPADBG("modify num_pdns (%d)\n", num_pdns);
	}

	return ret;
}

int ipa_nati_get_pdn_cnt(void)
{
	return num_pdns;
}

int ipa_nati_dealloc_pdn(
	uint8_t pdn_index)
{
	ipa_nat_pdn_entry zero_test;
	struct ipa_ioc_nat_pdn_entry pdn_data;
	int ret;

	IPADBG(" trying to deallocate PDN index %d\n", pdn_index);

	if(!num_pdns)
	{
		IPAERR("pdn table is already empty\n");
		return -EIO;
	}

	memset(&zero_test, 0, sizeof(zero_test));

	if(!memcmp((pdns + pdn_index), &zero_test, sizeof(ipa_nat_pdn_entry)))
	{
		IPAERR("pdn entry is a zero entry\n");
		return -EIO;
	}

	IPADBG("PDN in index %d has ip 0x%X\n", pdn_index, pdns[pdn_index].public_ip);

	pdn_data.pdn_index    = pdn_index;
	pdn_data.src_metadata = 0;
	pdn_data.dst_metadata = 0;
	pdn_data.public_ip    = 0;

	ret = ipa_nati_modify_pdn(&pdn_data);
	if(ret)
	{
		IPAERR("failed modifying PDN\n");
		return -EIO;
	}

	memset((pdns + pdn_index), 0, sizeof(ipa_nat_pdn_entry));

	num_pdns--;

	IPADBG("successfully removed pdn from index %d num_pdns %d\n", pdn_index, num_pdns);

	return 0;
}

/*
 * ----------------------------------------------------------------------------
 * Previously public API functions, but have been hijacked (in
 * ipa_nat_statemach.c).  The new definitions that replaced these, now
 * call the functions below.
 * ----------------------------------------------------------------------------
 */
int ipa_NATI_post_ipv4_init_cmd(
	uint32_t tbl_hdl )
{
	enum ipa3_nat_mem_in            nmi;
	struct ipa_nat_cache*           nat_cache_ptr;
	struct ipa_nat_ip4_table_cache* nat_table;
	int ret;

	IPADBG("In\n");

	BREAK_TBL_HDL(tbl_hdl, nmi, tbl_hdl);

	if ( ! IPA_VALID_NAT_MEM_IN(nmi) ) {
		IPAERR("Bad cache type argument passed\n");
		ret = -EINVAL;
		goto bail;
	}

	IPADBG("nmi(%s)\n", ipa3_nat_mem_in_as_str(nmi));

	nat_cache_ptr = &ipv4_nat_cache[nmi];

	if (pthread_mutex_lock(&nat_mutex)) {
		IPAERR("unable to lock the nat mutex\n");
		ret = -EINVAL;
		goto bail;
	}

	if ( ! nat_cache_ptr->table_cnt ) {
		IPAERR("No initialized table in NAT cache\n");
		ret = -EINVAL;
		goto unlock;
	}

	nat_table = &nat_cache_ptr->ip4_tbl[tbl_hdl - 1];

	ret = ipa_nati_post_ipv4_init_cmd(
		nat_cache_ptr,
		nat_table,
		tbl_hdl - 1,
		true);

	if (ret) {
		IPAERR("unable to post nat_init command Error %d\n", ret);
		goto unlock;
	}

	active_nat_cache_ptr = nat_cache_ptr;

unlock:
	if (pthread_mutex_unlock(&nat_mutex)) {
		IPAERR("unable to unlock the nat mutex\n");
		ret = (ret) ? ret : -EPERM;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

/**
 * ipa_NATI_add_ipv4_tbl() - Adds a new IPv4 NAT table
 * @ct: [in] the desired cache type to use
 * @public_ip_addr: [in] public IPv4 address
 * @number_of_entries: [in] number of NAT entries
 * @table_handle: [out] handle of new IPv4 NAT table
 *
 * This function creates new IPv4 NAT table and posts IPv4 NAT init command to HW
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_NATI_add_ipv4_tbl(
	enum ipa3_nat_mem_in nmi,
	uint32_t             public_ip_addr,
	uint16_t             number_of_entries,
	uint32_t*            tbl_hdl )
{
	struct ipa_nat_cache*           nat_cache_ptr;
	struct ipa_nat_ip4_table_cache* nat_table;
	int ret = 0;

	IPADBG("In\n");

	*tbl_hdl = 0;

	if ( ! IPA_VALID_NAT_MEM_IN(nmi) ) {
		IPAERR("Bad cache type argument passed\n");
		ret = -EINVAL;
		goto bail;
	}

	IPADBG("nmi(%s)\n", ipa3_nat_mem_in_as_str(nmi));

	nat_cache_ptr = &ipv4_nat_cache[nmi];

	if (pthread_mutex_lock(&nat_mutex)) {
		IPAERR("unable to lock the nat mutex\n");
		ret = -EINVAL;
		goto bail;
	}

	nat_cache_ptr->nmi = nmi;

	if (nat_cache_ptr->table_cnt >= IPA_NAT_MAX_IP4_TBLS) {
		IPAERR(
			"Can't add addition NAT table. Maximum %d tables allowed\n",
			IPA_NAT_MAX_IP4_TBLS);
		ret = -EINVAL;
		goto unlock;
	}

	if ( ! nat_cache_ptr->ipa_desc ) {
		nat_cache_ptr->ipa_desc = ipa_descriptor_open();
		if ( nat_cache_ptr->ipa_desc == NULL ) {
			IPAERR("failed to open IPA driver file descriptor\n");
			ret = -EIO;
			goto unlock;
		}
	}

	nat_table = &nat_cache_ptr->ip4_tbl[nat_cache_ptr->table_cnt];

	ret = ipa_nati_create_table(
		nat_cache_ptr,
		nat_table,
		public_ip_addr,
		number_of_entries,
		nat_cache_ptr->table_cnt);

	if (ret) {
		IPAERR("unable to create nat table Error: %d\n", ret);
		goto failed_create_table;
	}

	/*
	 * Initialize the ipa hw with nat table dimensions
	 */
	ret = ipa_nati_post_ipv4_init_cmd(
		nat_cache_ptr,
		nat_table,
		nat_cache_ptr->table_cnt,
		false);

	if (ret) {
		IPAERR("unable to post nat_init command Error %d\n", ret);
		goto failed_post_init_cmd;
	}

	active_nat_cache_ptr = nat_cache_ptr;

	/*
	 * Store the initial public ip address in the cached pdn table
	 * this is backward compatible for pre IPAv4 versions, we will
	 * always use this ip as the single PDN address
	 */
	pdns[0].public_ip = public_ip_addr;
	num_pdns = 1;

	nat_cache_ptr->table_cnt++;

	/*
	 * Return table handle
	 */
	*tbl_hdl = MAKE_TBL_HDL(nat_cache_ptr->table_cnt, nmi);

	IPADBG("tbl_hdl value(0x%08X) num_pdns (%d)\n", *tbl_hdl, num_pdns);

	goto unlock;

failed_post_init_cmd:
	ipa_nati_destroy_table(nat_cache_ptr, nat_table);

failed_create_table:
	if (!nat_cache_ptr->table_cnt) {
		ipa_descriptor_close(nat_cache_ptr->ipa_desc);
		nat_cache_ptr->ipa_desc = NULL;
	}

unlock:
	if (pthread_mutex_unlock(&nat_mutex)) {
		IPAERR("unable to unlock the nat mutex\n");
		ret = -EPERM;
		goto bail;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

int ipa_NATI_del_ipv4_table(
	uint32_t tbl_hdl )
{
	enum ipa3_nat_mem_in            nmi;
	struct ipa_nat_cache*           nat_cache_ptr;
	struct ipa_nat_ip4_table_cache* nat_table;

	int ret;

	IPADBG("In\n");

	BREAK_TBL_HDL(tbl_hdl, nmi, tbl_hdl);

	if ( ! IPA_VALID_NAT_MEM_IN(nmi) ) {
		IPAERR("Bad cache type argument passed\n");
		ret = -EINVAL;
		goto bail;
	}

	IPADBG("nmi(%s)\n", ipa3_nat_mem_in_as_str(nmi));

	nat_cache_ptr = &ipv4_nat_cache[nmi];

	nat_table = &nat_cache_ptr->ip4_tbl[tbl_hdl - 1];

	if (pthread_mutex_lock(&nat_mutex)) {
		IPAERR("unable to lock the nat mutex\n");
		ret = -EINVAL;
		goto bail;
	}

	if (! nat_table->mem_desc.valid) {
		IPAERR("invalid table handle %d\n", tbl_hdl);
		ret = -EINVAL;
		goto unlock;
	}

	ret = ipa_nati_destroy_table(nat_cache_ptr, nat_table);
	if (ret) {
		IPAERR("unable to delete NAT table with handle %d\n", tbl_hdl);
		goto unlock;
	}

	if (! --nat_cache_ptr->table_cnt) {
		ipa_descriptor_close(nat_cache_ptr->ipa_desc);
		nat_cache_ptr->ipa_desc = NULL;
	}

unlock:
	if (pthread_mutex_unlock(&nat_mutex)) {
		IPAERR("unable to unlock the nat mutex\n");
		ret = (ret) ? ret : -EPERM;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

int ipa_NATI_query_timestamp(
	uint32_t  tbl_hdl,
	uint32_t  rule_hdl,
	uint32_t* time_stamp )
{
	enum ipa3_nat_mem_in            nmi;
	struct ipa_nat_cache*           nat_cache_ptr;
	struct ipa_nat_ip4_table_cache* nat_table;
	struct ipa_nat_rule*            rule_ptr;

	char buf[1024];
	int  ret;

	IPADBG("In\n");

	BREAK_TBL_HDL(tbl_hdl, nmi, tbl_hdl);

	if ( ! IPA_VALID_NAT_MEM_IN(nmi) ) {
		IPAERR("Bad cache type argument passed\n");
		ret = -EINVAL;
		goto bail;
	}

	IPADBG("nmi(%s)\n", ipa3_nat_mem_in_as_str(nmi));

	nat_cache_ptr = &ipv4_nat_cache[nmi];

	nat_table = &nat_cache_ptr->ip4_tbl[tbl_hdl - 1];

	if (pthread_mutex_lock(&nat_mutex)) {
		IPAERR("unable to lock the nat mutex\n");
		ret = -EINVAL;
		goto bail;
	}

	if ( ! nat_table->mem_desc.valid ) {
		IPAERR("invalid table handle %d\n", tbl_hdl);
		ret = -EINVAL;
		goto unlock;
	}

	ret = ipa_table_get_entry(
		&nat_table->table,
		rule_hdl,
		(void**) &rule_ptr,
		NULL);

	if (ret) {
		IPAERR("Unable to retrive the entry with "
			   "handle=%u in NAT table with handle=0x%08X\n",
			   rule_hdl, tbl_hdl);
		goto unlock;
	}

	IPADBG("rule_hdl(0x%08X) -> %s\n",
		   rule_hdl,
		   prep_nat_rule_4print(rule_ptr, buf, sizeof(buf)));

	*time_stamp = rule_ptr->time_stamp;

unlock:
	if (pthread_mutex_unlock(&nat_mutex)) {
		IPAERR("unable to unlock the nat mutex\n");
		ret = (ret) ? ret : -EPERM;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

int ipa_NATI_add_ipv4_rule(
	uint32_t                 tbl_hdl,
	const ipa_nat_ipv4_rule* clnt_rule,
	uint32_t*                rule_hdl)
{
	uint32_t cmd_sz =
		sizeof(struct ipa_ioc_nat_dma_cmd) +
		(MAX_DMA_ENTRIES_FOR_ADD * sizeof(struct ipa_ioc_nat_dma_one));
	char cmd_buf[cmd_sz];
	struct ipa_ioc_nat_dma_cmd* cmd =
		(struct ipa_ioc_nat_dma_cmd*) cmd_buf;

	enum ipa3_nat_mem_in            nmi;
	struct ipa_nat_cache*           nat_cache_ptr;
	struct ipa_nat_ip4_table_cache* nat_table;
	struct ipa_nat_rule*            rule;

	uint16_t new_entry_index;
	uint16_t new_index_tbl_entry_index;
	uint32_t new_entry_handle;
	char     buf[1024];

	int ret = 0;

	IPADBG("In\n");

	memset(cmd_buf, 0, sizeof(cmd_buf));

	if ( ! VALID_TBL_HDL(tbl_hdl) ||
		 ! clnt_rule ||
		 ! rule_hdl )
	{
		IPAERR("Bad arg: tbl_hdl(0x%08X) and/or clnt_rule(%p) and/or rule_hdl(%p)\n",
			   tbl_hdl, clnt_rule, rule_hdl);
		ret = -EINVAL;
		goto done;
	}

	*rule_hdl = 0;

	IPADBG("tbl_hdl(0x%08X)\n", tbl_hdl);

	BREAK_TBL_HDL(tbl_hdl, nmi, tbl_hdl);

	if ( ! IPA_VALID_NAT_MEM_IN(nmi) ) {
		IPAERR("Bad cache type argument passed\n");
		ret = -EINVAL;
		goto done;
	}

	IPADBG("tbl_hdl(0x%08X) nmi(%s) %s\n",
		   tbl_hdl,
		   ipa3_nat_mem_in_as_str(nmi),
		   prep_nat_ipv4_rule_4print(clnt_rule, buf, sizeof(buf)));

	nat_cache_ptr = &ipv4_nat_cache[nmi];

	nat_table = &nat_cache_ptr->ip4_tbl[tbl_hdl - 1];

	if (clnt_rule->protocol == IPAHAL_NAT_INVALID_PROTOCOL) {
		IPAERR("invalid parameter protocol=%d\n", clnt_rule->protocol);
		ret = -EINVAL;
		goto done;
	}

	/*
	 * Verify that the rule's PDN is valid
	 */
	if (clnt_rule->pdn_index >= IPA_MAX_PDN_NUM ||
		pdns[clnt_rule->pdn_index].public_ip == 0) {
		IPAERR("invalid parameters, pdn index %d, public ip = 0x%X\n",
			   clnt_rule->pdn_index, pdns[clnt_rule->pdn_index].public_ip);
		ret = -EINVAL;
		goto done;
	}

	if (pthread_mutex_lock(&nat_mutex)) {
		IPAERR("unable to lock the nat mutex\n");
		ret = -EINVAL;
		goto done;
	}

	if (! nat_table->mem_desc.valid) {
		IPAERR("invalid table handle %d\n", tbl_hdl);
		ret = -EINVAL;
		goto unlock;
	}

	/* src_only */
	if (clnt_rule->src_only) {
		new_entry_index = dst_hash(
			nat_cache_ptr,
			pdns[clnt_rule->pdn_index].public_ip,
			clnt_rule->target_ip,
			clnt_rule->target_port,
			clnt_rule->public_port,
			clnt_rule->protocol,
			nat_table->table.table_entries - 1) + Hash_token;
		new_entry_index = (new_entry_index & (nat_table->table.table_entries - 1));
		if (new_entry_index == 0) {
			new_entry_index = nat_table->table.table_entries - 1;
		}
		Hash_token++;
	} else {
	new_entry_index = dst_hash(
		nat_cache_ptr,
		pdns[clnt_rule->pdn_index].public_ip,
		clnt_rule->target_ip,
		clnt_rule->target_port,
		clnt_rule->public_port,
		clnt_rule->protocol,
		nat_table->table.table_entries - 1);
	}

	ret = ipa_table_add_entry(
		&nat_table->table,
		(void*) clnt_rule,
		&new_entry_index,
		&new_entry_handle,
		cmd);

	if (ret) {
		IPAERR("Failed to add a new NAT entry\n");
		goto unlock;
	}

	/* dst_only */
	if (clnt_rule->dst_only) {
		new_index_tbl_entry_index =
			src_hash(clnt_rule->private_ip,
				 clnt_rule->private_port,
				 clnt_rule->target_ip,
				 clnt_rule->target_port,
				 clnt_rule->protocol,
				 nat_table->table.table_entries - 1) + Hash_token;
		new_index_tbl_entry_index = (new_index_tbl_entry_index & (nat_table->table.table_entries - 1));
		if (new_index_tbl_entry_index == 0) {
			new_index_tbl_entry_index = nat_table->table.table_entries - 1;
		}
		Hash_token++;
	} else {
	new_index_tbl_entry_index =
		src_hash(clnt_rule->private_ip,
				 clnt_rule->private_port,
				 clnt_rule->target_ip,
				 clnt_rule->target_port,
				 clnt_rule->protocol,
				 nat_table->table.table_entries - 1);
	}
	ret = ipa_table_add_entry(
		&nat_table->index_table,
		(void*) &new_entry_index,
		&new_index_tbl_entry_index,
		NULL,
		cmd);

	if (ret) {
		IPAERR("failed to add a new NAT index entry\n");
		goto fail_add_index_entry;
	}

	rule = ipa_table_get_entry_by_index(
		&nat_table->table,
		new_entry_index);

	if (rule == NULL) {
		IPAERR("Failed to retrieve the entry in index %d for NAT table with handle=%d\n",
			   new_entry_index, tbl_hdl);
		ret = -EPERM;
		goto bail;
	}

	rule->indx_tbl_entry = new_index_tbl_entry_index;

	rule->redirect   = clnt_rule->redirect;
	rule->enable     = clnt_rule->enable;
	rule->time_stamp = clnt_rule->time_stamp;

	IPADBG("new entry:%d, new index entry: %d\n",
		   new_entry_index, new_index_tbl_entry_index);

	IPADBG("rule_hdl(0x%08X) -> %s\n",
		   new_entry_handle,
		   prep_nat_rule_4print(rule, buf, sizeof(buf)));

	ret = ipa_nati_post_ipv4_dma_cmd(nat_cache_ptr, cmd);

	if (ret) {
		IPAERR("unable to post dma command\n");
		goto bail;
	}

	if (pthread_mutex_unlock(&nat_mutex)) {
		IPAERR("unable to unlock the nat mutex\n");
		ret = -EPERM;
		goto done;
	}

	*rule_hdl = new_entry_handle;

	IPADBG("rule_hdl value(%u)\n", *rule_hdl);

	goto done;

bail:
	ipa_table_erase_entry(&nat_table->index_table, new_index_tbl_entry_index);

fail_add_index_entry:
	ipa_table_erase_entry(&nat_table->table, new_entry_index);

unlock:
	if (pthread_mutex_unlock(&nat_mutex))
		IPAERR("unable to unlock the nat mutex\n");
done:
	IPADBG("Out\n");

	return ret;
}

int ipa_NATI_del_ipv4_rule(
	uint32_t tbl_hdl,
	uint32_t rule_hdl )
{
	uint32_t cmd_sz =
		sizeof(struct ipa_ioc_nat_dma_cmd) +
		(MAX_DMA_ENTRIES_FOR_DEL * sizeof(struct ipa_ioc_nat_dma_one));
	char cmd_buf[cmd_sz];
	struct ipa_ioc_nat_dma_cmd* cmd =
		(struct ipa_ioc_nat_dma_cmd*) cmd_buf;

	enum ipa3_nat_mem_in            nmi;
	struct ipa_nat_cache*           nat_cache_ptr;
	struct ipa_nat_ip4_table_cache* nat_table;
	struct ipa_nat_rule*            table_rule;
	struct ipa_nat_indx_tbl_rule*   index_table_rule;

	ipa_table_iterator table_iterator;
	ipa_table_iterator index_table_iterator;

	uint16_t index;
	char     buf[1024];
	int      ret = 0;

	IPADBG("In\n");

	memset(cmd_buf, 0, sizeof(cmd_buf));

	IPADBG("tbl_hdl(0x%08X) rule_hdl(%u)\n", tbl_hdl, rule_hdl);

	BREAK_TBL_HDL(tbl_hdl, nmi, tbl_hdl);

	if ( ! IPA_VALID_NAT_MEM_IN(nmi) ) {
		IPAERR("Bad cache type argument passed\n");
		ret = -EINVAL;
		goto done;
	}

	IPADBG("nmi(%s)\n", ipa3_nat_mem_in_as_str(nmi));

	nat_cache_ptr = &ipv4_nat_cache[nmi];

	nat_table = &nat_cache_ptr->ip4_tbl[tbl_hdl - 1];

	if (pthread_mutex_lock(&nat_mutex)) {
		IPAERR("Unable to lock the nat mutex\n");
		ret = -EINVAL;
		goto done;
	}

	if (! nat_table->mem_desc.valid) {
		IPAERR("Invalid table handle 0x%08X\n", tbl_hdl);
		ret = -EINVAL;
		goto unlock;
	}

	ret = ipa_table_get_entry(
		&nat_table->table,
		rule_hdl,
		(void**) &table_rule,
		&index);

	if (ret) {
		IPAERR("Unable to retrive the entry with rule_hdl=%u\n", rule_hdl);
		goto unlock;
	}

	IPADBG("rule_hdl(0x%08X) -> %s\n",
		   rule_hdl,
		   prep_nat_rule_4print(table_rule, buf, sizeof(buf)));

	ret = ipa_table_iterator_init(
		&table_iterator,
		&nat_table->table,
		table_rule,
		index);

	if (ret) {
		IPAERR("Unable to create iterator which points to the "
			   "entry %u in NAT table with handle=0x%08X\n",
			   index, tbl_hdl);
		goto unlock;
	}

	index = table_rule->indx_tbl_entry;

	index_table_rule = (struct ipa_nat_indx_tbl_rule*)
		ipa_table_get_entry_by_index(&nat_table->index_table, index);

	if (index_table_rule == NULL) {
		IPAERR("Unable to retrieve the entry in index %u "
			   "in NAT index table with handle=0x%08X\n",
			   index, tbl_hdl);
		ret = -EPERM;
		goto unlock;
	}

	ret = ipa_table_iterator_init(
		&index_table_iterator,
		&nat_table->index_table,
		index_table_rule,
		index);

	if (ret) {
		IPAERR("Unable to create iterator which points to the "
			   "entry %u in NAT index table with handle=0x%08X\n",
			   index, tbl_hdl);
		goto unlock;
	}

	ipa_table_create_delete_command(
		&nat_table->index_table,
		cmd,
		&index_table_iterator);

	if (ipa_table_iterator_is_head_with_tail(&index_table_iterator)) {

		ipa_nati_copy_second_index_entry_to_head(
			nat_table, &index_table_iterator, cmd);
		/*
		 * Iterate to the next entry which should be deleted
		 */
		ret = ipa_table_iterator_next(
			&index_table_iterator, &nat_table->index_table);

		if (ret) {
			IPAERR("Unable to move the iterator to the next entry "
				   "(points to the entry %u in NAT index table)\n",
				   index);
			goto unlock;
		}
	}

	ipa_table_create_delete_command(
		&nat_table->table,
		cmd,
		&table_iterator);

	ret = ipa_nati_post_ipv4_dma_cmd(nat_cache_ptr, cmd);

	if (ret) {
		IPAERR("Unable to post dma command\n");
		goto unlock;
	}

	if (! ipa_table_iterator_is_head_with_tail(&table_iterator)) {
		/* The entry can be deleted */
		uint8_t is_prev_empty =
			(table_iterator.prev_entry != NULL &&
			 ((struct ipa_nat_rule*)table_iterator.prev_entry)->protocol ==
			 IPAHAL_NAT_INVALID_PROTOCOL);

		ipa_table_delete_entry(
			&nat_table->table, &table_iterator, is_prev_empty);
	}

	ipa_table_delete_entry(
		&nat_table->index_table,
		&index_table_iterator,
		FALSE);

	if (index_table_iterator.curr_index >= nat_table->index_table.table_entries)
		nat_table->index_expn_table_meta[
			index_table_iterator.curr_index - nat_table->index_table.table_entries].
			prev_index = IPA_TABLE_INVALID_ENTRY;

unlock:
	if (pthread_mutex_unlock(&nat_mutex)) {
		IPAERR("Unable to unlock the nat mutex\n");
		ret = (ret) ? ret : -EPERM;
	}

done:
	IPADBG("Out\n");

	return ret;
}

/*
 * ----------------------------------------------------------------------------
 * New function to get sram size.
 * ----------------------------------------------------------------------------
 */
int ipa_nati_get_sram_size(
	uint32_t* size_ptr)
{
	struct ipa_nat_cache* nat_cache_ptr =
		&ipv4_nat_cache[IPA_NAT_MEM_IN_SRAM];
	struct ipa_nat_in_sram_info nat_sram_info;
	int ret;

	IPADBG("In\n");

	if (pthread_mutex_lock(&nat_mutex)) {
		IPAERR("unable to lock the nat mutex\n");
		ret = -EINVAL;
		goto bail;
	}

	if ( ! nat_cache_ptr->ipa_desc ) {
		nat_cache_ptr->ipa_desc = ipa_descriptor_open();
		if ( nat_cache_ptr->ipa_desc == NULL ) {
			IPAERR("failed to open IPA driver file descriptor\n");
			ret = -EIO;
			goto unlock;
		}
	}

	memset(&nat_sram_info, 0, sizeof(nat_sram_info));

	ret = ioctl(nat_cache_ptr->ipa_desc->fd,
				IPA_IOC_GET_NAT_IN_SRAM_INFO,
				&nat_sram_info);

	if (ret) {
		IPAERR("NAT_IN_SRAM_INFO ioctl failure %d on IPA fd %d\n",
			   ret, nat_cache_ptr->ipa_desc->fd);
		goto unlock;
	}

	if ( (*size_ptr = nat_sram_info.sram_mem_available_for_nat) == 0 )
	{
		IPAERR("sram_mem_available_for_nat is zero\n");
		ret = -EINVAL;
		goto unlock;
	}

unlock:
	if (pthread_mutex_unlock(&nat_mutex)) {
		IPAERR("unable to unlock the nat mutex\n");
		ret = (ret) ? ret : -EPERM;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

/*
 * ----------------------------------------------------------------------------
 * Utility functions
 * ----------------------------------------------------------------------------
 */
static int print_nat_rule(
	ipa_table*      table_ptr,
	uint32_t        rule_hdl,
	void*           record_ptr,
	uint16_t        record_index,
	void*           meta_record_ptr,
	uint16_t        meta_record_index,
	void*           arb_data_ptr )
{
	enum ipa3_nat_mem_in nmi;
	uint8_t              is_expn_tbl;
	uint16_t             rule_index;

	char buf[1024];

	struct ipa_nat_rule* rule_ptr =
		(struct ipa_nat_rule*) record_ptr;

	if ( rule_ptr->protocol == IPA_NAT_INVALID_PROTO_FIELD_VALUE_IN_RULE )
	{
		goto bail;
	}

	BREAK_RULE_HDL(table_ptr, rule_hdl, nmi, is_expn_tbl, rule_index);

	printf("  %s %s (0x%04X) (0x%08X) -> %s\n",
		   (table_ptr->nmi == IPA_NAT_MEM_IN_DDR) ? "DDR" : "SRAM",
		   (is_expn_tbl) ? "EXP " : "BASE",
		   record_index,
		   rule_hdl,
		   prep_nat_rule_4print(rule_ptr, buf, sizeof(buf)));

	fflush(stdout);

	*((bool*) arb_data_ptr) = false;

bail:
	return 0;
}

static int print_meta_data(
	ipa_table*      table_ptr,
	uint32_t        rule_hdl,
	void*           record_ptr,
	uint16_t        record_index,
	void*           meta_record_ptr,
	uint16_t        meta_record_index,
	void*           arb_data_ptr )
{
	struct ipa_nat_indx_tbl_rule* index_entry =
		(struct ipa_nat_indx_tbl_rule *) record_ptr;

	struct ipa_nat_indx_tbl_meta_info* mi_ptr =
		(struct ipa_nat_indx_tbl_meta_info*) meta_record_ptr;

	enum ipa3_nat_mem_in nmi;
	uint8_t              is_expn_tbl;
	uint16_t             rule_index;

	BREAK_RULE_HDL(table_ptr, rule_hdl, nmi, is_expn_tbl, rule_index);

	if ( mi_ptr )
	{
		printf("  %s %s Entry_Index=0x%04X Table_Entry=0x%04X -> "
			   "Prev_Index=0x%04X Next_Index=0x%04X\n",
			   (table_ptr->nmi == IPA_NAT_MEM_IN_DDR) ? "DDR" : "SRAM",
			   (is_expn_tbl) ? "EXP " : "BASE",
			   record_index,
			   index_entry->tbl_entry,
			   mi_ptr->prev_index,
			   index_entry->next_index);
	}
	else
	{
		printf("  %s %s Entry_Index=0x%04X Table_Entry=0x%04X -> "
			   "Prev_Index=0xXXXX Next_Index=0x%04X\n",
			   (table_ptr->nmi == IPA_NAT_MEM_IN_DDR) ? "DDR" : "SRAM",
			   (is_expn_tbl) ? "EXP " : "BASE",
			   record_index,
			   index_entry->tbl_entry,
			   index_entry->next_index);
	}

	fflush(stdout);

	*((bool*) arb_data_ptr) = false;

	return 0;
}

void ipa_nat_dump_ipv4_table(
	uint32_t tbl_hdl )
{
	bool empty;

	if (pthread_mutex_lock(&nat_mutex)) {
		IPAERR("unable to lock the nat mutex\n");
		return;
	}

	printf("\nIPv4 active rules:\n");

	empty = true;

	ipa_nati_walk_ipv4_tbl(tbl_hdl, USE_NAT_TABLE, print_nat_rule, &empty);

	if ( empty )
	{
		printf("  Empty\n");
	}

	printf("\nExpansion Index Table Meta Data:\n");

	empty = true;

	ipa_nati_walk_ipv4_tbl(tbl_hdl, USE_INDEX_TABLE, print_meta_data, &empty);

	if ( empty )
	{
		printf("  Empty\n");
	}

	printf("\n");

	if (pthread_mutex_unlock(&nat_mutex)) {
		IPAERR("unable to unlock the nat mutex\n");
	}
}

int ipa_NATI_clear_ipv4_tbl(
	uint32_t tbl_hdl )
{
	enum ipa3_nat_mem_in            nmi;
	struct ipa_nat_cache*           nat_cache_ptr;
	struct ipa_nat_ip4_table_cache* nat_table;
	int ret = 0;

	IPADBG("In\n");

	BREAK_TBL_HDL(tbl_hdl, nmi, tbl_hdl);

	if ( ! IPA_VALID_NAT_MEM_IN(nmi) ) {
		IPAERR("Bad cache type argument passed\n");
		ret = -EINVAL;
		goto bail;
	}

	IPADBG("nmi(%s)\n", ipa3_nat_mem_in_as_str(nmi));

	nat_cache_ptr = &ipv4_nat_cache[nmi];

	if (pthread_mutex_lock(&nat_mutex)) {
		IPAERR("unable to lock the nat mutex\n");
		ret = -EINVAL;
		goto bail;
	}

	if ( ! nat_cache_ptr->table_cnt ) {
		IPAERR("No initialized table in NAT cache\n");
		ret = -EINVAL;
		goto unlock;
	}

	nat_table = &nat_cache_ptr->ip4_tbl[tbl_hdl - 1];

	ipa_table_reset(&nat_table->table);
	nat_table->table.cur_tbl_cnt =
		nat_table->table.cur_expn_tbl_cnt = 0;

	ipa_table_reset(&nat_table->index_table);
	nat_table->index_table.cur_tbl_cnt =
		nat_table->index_table.cur_expn_tbl_cnt = 0;

unlock:
	if (pthread_mutex_unlock(&nat_mutex)) {
		IPAERR("unable to unlock the nat mutex\n");
		ret = (ret) ? ret : -EPERM;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

int ipa_nati_copy_ipv4_tbl(
	uint32_t          src_tbl_hdl,
	uint32_t          dst_tbl_hdl,
	ipa_table_walk_cb copy_cb )
{
	int ret = 0;

	IPADBG("In\n");

	if ( ! copy_cb )
	{
		IPAERR("copy_cb is null\n");
		ret = -EINVAL;
		goto bail;
	}

	if (pthread_mutex_lock(&nat_mutex))
	{
		IPAERR("unable to lock the nat mutex\n");
		ret = -EINVAL;
		goto bail;
	}

	/*
	 * Clear the destination table...
	 */
	ret = ipa_NATI_clear_ipv4_tbl(dst_tbl_hdl);

	if ( ret == 0 )
	{
		/*
		 * Now walk the source table and pass the valid records to the
		 * user's copy callback...
		 */
		ret = ipa_NATI_walk_ipv4_tbl(
			src_tbl_hdl, USE_NAT_TABLE, copy_cb, dst_tbl_hdl);

		if ( ret != 0 )
		{
			IPAERR("ipa_table_walk returned non-zero (%d)\n", ret);
			goto unlock;
		}
	}

unlock:
	if (pthread_mutex_unlock(&nat_mutex))
	{
		IPAERR("unable to unlock the nat mutex\n");
		ret = (ret) ? ret : -EPERM;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

int ipa_NATI_walk_ipv4_tbl(
	uint32_t          tbl_hdl,
	WhichTbl2Use      which,
	ipa_table_walk_cb walk_cb,
	void*             arb_data_ptr )
{
	enum ipa3_nat_mem_in            nmi;
	uint32_t                        broken_tbl_hdl;
	struct ipa_nat_cache*           nat_cache_ptr;
	struct ipa_nat_ip4_table_cache* nat_table;
	ipa_table*                      ipa_tbl_ptr;

	int ret = 0;

	IPADBG("In\n");

	if ( ! VALID_TBL_HDL(tbl_hdl) ||
		 ! VALID_WHICHTBL2USE(which) ||
		 ! walk_cb )
	{
		IPAERR("Bad arg: tbl_hdl(0x%08X) and/or WhichTbl2Use(%u) and/or walk_cb(%p)\n",
			   tbl_hdl, which, walk_cb);
		ret = -EINVAL;
		goto bail;
	}

	if ( pthread_mutex_lock(&nat_mutex) )
	{
		IPAERR("unable to lock the nat mutex\n");
		ret = -EINVAL;
		goto bail;
	}

	/*
	 * Now walk the table and pass the valid records to the user's
	 * walk callback...
	 */
	BREAK_TBL_HDL(tbl_hdl, nmi, broken_tbl_hdl);

	if ( ! IPA_VALID_NAT_MEM_IN(nmi) )
	{
		IPAERR("Bad cache type argument passed\n");
		ret = -EINVAL;
		goto unlock;
	}

	nat_cache_ptr = &ipv4_nat_cache[nmi];

	if ( ! nat_cache_ptr->table_cnt )
	{
		IPAERR("No initialized table in NAT cache\n");
		ret = -EINVAL;
		goto unlock;
	}

	nat_table = &nat_cache_ptr->ip4_tbl[broken_tbl_hdl - 1];

	ipa_tbl_ptr =
		(which == USE_NAT_TABLE) ?
		&nat_table->table     :
		&nat_table->index_table;

	ret = ipa_table_walk(ipa_tbl_ptr, 0, WHEN_SLOT_FILLED, walk_cb, arb_data_ptr);

	if ( ret != 0 )
	{
		IPAERR("ipa_table_walk returned non-zero (%d)\n", ret);
		goto unlock;
	}

unlock:
	if ( pthread_mutex_unlock(&nat_mutex) )
	{
		IPAERR("unable to unlock the nat mutex\n");
		ret = (ret) ? ret : -EPERM;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

typedef struct
{
	WhichTbl2Use        which;
	uint32_t            tot_for_avg;
	ipa_nati_tbl_stats* stats_ptr;
} chain_stat_help;

static int gen_chain_stats(
	ipa_table*      table_ptr,
	uint32_t        rule_hdl,
	void*           record_ptr,
	uint16_t        record_index,
	void*           meta_record_ptr,
	uint16_t        meta_record_index,
	void*           arb_data_ptr )
{
	chain_stat_help* csh_ptr = (chain_stat_help*) arb_data_ptr;

	enum ipa3_nat_mem_in nmi;
	uint8_t              is_expn_tbl;
	uint16_t             rule_index;

	uint32_t             chain_len = 0;

	BREAK_RULE_HDL(table_ptr, rule_hdl, nmi, is_expn_tbl, rule_index);

	if ( is_expn_tbl )
	{
		return 1;
	}

	if ( csh_ptr->which == USE_NAT_TABLE )
	{
		struct ipa_nat_rule* list_elem_ptr =
			(struct ipa_nat_rule*) record_ptr;

		if ( list_elem_ptr->next_index )
		{
			chain_len = 1;

			while ( list_elem_ptr->next_index )
			{
				chain_len++;

				list_elem_ptr = GOTO_REC(table_ptr, list_elem_ptr->next_index);
			}
		}
	}
	else
	{
		struct ipa_nat_indx_tbl_rule* list_elem_ptr =
			(struct ipa_nat_indx_tbl_rule*) record_ptr;

		if ( list_elem_ptr->next_index )
		{
			chain_len = 1;

			while ( list_elem_ptr->next_index )
			{
				chain_len++;

				list_elem_ptr = GOTO_REC(table_ptr, list_elem_ptr->next_index);
			}
		}
	}

	if ( chain_len )
	{
		csh_ptr->stats_ptr->tot_chains += 1;

		csh_ptr->tot_for_avg += chain_len;

		if ( csh_ptr->stats_ptr->min_chain_len == 0 )
		{
			csh_ptr->stats_ptr->min_chain_len = chain_len;
		}
		else
		{
			csh_ptr->stats_ptr->min_chain_len =
				min(csh_ptr->stats_ptr->min_chain_len, chain_len);
		}

		csh_ptr->stats_ptr->max_chain_len =
			max(csh_ptr->stats_ptr->max_chain_len, chain_len);
	}

	return 0;
}

int ipa_NATI_ipv4_tbl_stats(
	uint32_t            tbl_hdl,
	ipa_nati_tbl_stats* nat_stats_ptr,
	ipa_nati_tbl_stats* idx_stats_ptr )
{
	enum ipa3_nat_mem_in            nmi;
	uint32_t                        broken_tbl_hdl;
	struct ipa_nat_cache*           nat_cache_ptr;
	struct ipa_nat_ip4_table_cache* nat_table;
	ipa_table*                      ipa_tbl_ptr;

	chain_stat_help                 csh;

	int ret = 0;

	IPADBG("In\n");

	if ( ! VALID_TBL_HDL(tbl_hdl) ||
		 ! nat_stats_ptr ||
		 ! idx_stats_ptr )
	{
		IPAERR("Bad arg: "
			   "tbl_hdl(0x%08X) and/or "
			   "nat_stats_ptr(%p) and/or "
			   "idx_stats_ptr(%p)\n",
			   tbl_hdl,
			   nat_stats_ptr,
			   idx_stats_ptr );
		ret = -EINVAL;
		goto bail;
	}

	if ( pthread_mutex_lock(&nat_mutex) )
	{
		IPAERR("unable to lock the nat mutex\n");
		ret = -EINVAL;
		goto bail;
	}

	memset(nat_stats_ptr, 0, sizeof(ipa_nati_tbl_stats));
	memset(idx_stats_ptr, 0, sizeof(ipa_nati_tbl_stats));

	BREAK_TBL_HDL(tbl_hdl, nmi, broken_tbl_hdl);

	if ( ! IPA_VALID_NAT_MEM_IN(nmi) )
	{
		IPAERR("Bad cache type argument passed\n");
		ret = -EINVAL;
		goto unlock;
	}

	nat_cache_ptr = &ipv4_nat_cache[nmi];

	if ( ! nat_cache_ptr->table_cnt )
	{
		IPAERR("No initialized table in NAT cache\n");
		ret = -EINVAL;
		goto unlock;
	}

	nat_table = &nat_cache_ptr->ip4_tbl[broken_tbl_hdl - 1];

	/*
	 * Gather NAT table stats...
	 */
	ipa_tbl_ptr = &nat_table->table;

	nat_stats_ptr->nmi                  = nmi;

	nat_stats_ptr->tot_base_ents        = ipa_tbl_ptr->table_entries;
	nat_stats_ptr->tot_expn_ents        = ipa_tbl_ptr->expn_table_entries;
	nat_stats_ptr->tot_ents             =
		nat_stats_ptr->tot_base_ents + nat_stats_ptr->tot_expn_ents;

	nat_stats_ptr->tot_base_ents_filled = ipa_tbl_ptr->cur_tbl_cnt;
	nat_stats_ptr->tot_expn_ents_filled = ipa_tbl_ptr->cur_expn_tbl_cnt;

	memset(&csh, 0, sizeof(chain_stat_help));

	csh.which     = USE_NAT_TABLE;
	csh.stats_ptr = nat_stats_ptr;

	ret = ipa_table_walk(
		ipa_tbl_ptr, 0, WHEN_SLOT_FILLED, gen_chain_stats, &csh);

	if ( ret < 0 )
	{
		IPAERR("Error gathering chain stats\n");
		ret = -EINVAL;
		goto unlock;
	}

	if ( csh.tot_for_avg && nat_stats_ptr->tot_chains )
	{
		nat_stats_ptr->avg_chain_len =
			(float) csh.tot_for_avg / (float) nat_stats_ptr->tot_chains;
	}

	/*
	 * Now lets gather index table stats...
	 */
	ipa_tbl_ptr = &nat_table->index_table;

	idx_stats_ptr->nmi                  = nmi;

	idx_stats_ptr->tot_base_ents        = ipa_tbl_ptr->table_entries;
	idx_stats_ptr->tot_expn_ents        = ipa_tbl_ptr->expn_table_entries;
	idx_stats_ptr->tot_ents             =
		idx_stats_ptr->tot_base_ents + idx_stats_ptr->tot_expn_ents;

	idx_stats_ptr->tot_base_ents_filled = ipa_tbl_ptr->cur_tbl_cnt;
	idx_stats_ptr->tot_expn_ents_filled = ipa_tbl_ptr->cur_expn_tbl_cnt;

	memset(&csh, 0, sizeof(chain_stat_help));

	csh.which     = USE_INDEX_TABLE;
	csh.stats_ptr = idx_stats_ptr;

	ret = ipa_table_walk(
		ipa_tbl_ptr, 0, WHEN_SLOT_FILLED, gen_chain_stats, &csh);

	if ( ret < 0 )
	{
		IPAERR("Error gathering chain stats\n");
		ret = -EINVAL;
		goto unlock;
	}

	if ( csh.tot_for_avg && idx_stats_ptr->tot_chains )
	{
		idx_stats_ptr->avg_chain_len =
			(float) csh.tot_for_avg / (float) idx_stats_ptr->tot_chains;
	}

	ret = 0;

unlock:
	if ( pthread_mutex_unlock(&nat_mutex) )
	{
		IPAERR("unable to unlock the nat mutex\n");
		ret = (ret) ? ret : -EPERM;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

int ipa_nati_vote_clock(
    enum ipa_app_clock_vote_type vote_type )
{
	struct ipa_nat_cache* nat_cache_ptr =
		&ipv4_nat_cache[IPA_NAT_MEM_IN_SRAM];

	int ret = 0;

	IPADBG("In\n");

	if ( ! nat_cache_ptr->ipa_desc ) {
		nat_cache_ptr->ipa_desc = ipa_descriptor_open();
		if ( nat_cache_ptr->ipa_desc == NULL ) {
			IPAERR("failed to open IPA driver file descriptor\n");
			ret = -EIO;
			goto bail;
		}
	}

	ret = ioctl(nat_cache_ptr->ipa_desc->fd,
				IPA_IOC_APP_CLOCK_VOTE,
				vote_type);

	if (ret) {
		IPAERR("APP_CLOCK_VOTE ioctl failure %d on IPA fd %d\n",
			   ret, nat_cache_ptr->ipa_desc->fd);
		goto bail;
	}

bail:
	IPADBG("Out\n");

	return ret;
}
