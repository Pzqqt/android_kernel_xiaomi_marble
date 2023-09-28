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
#include "ipa_table.h"
#include "ipa_nat_utils.h"

#include <errno.h>

#define IPA_BASE_TABLE_PERCENTAGE       .8
#define IPA_EXPANSION_TABLE_PERCENTAGE  .2

#define IPA_BASE_TABLE_PCNT_4SRAM      1.00
#define IPA_EXPANSION_TABLE_PCNT_4SRAM 0.43

/*
 * The table number of entries is limited by Entry ID structure
 * above. The base table max entries is limited by index into table
 * bits number.
 *
 * The table max ents number is: (base table max ents / base table percentage)
 *
 * IPA_TABLE_MAX_ENTRIES = 2^(index into table) / IPA_BASE_TABLE_PERCENTAGE
 */

static int InsertHead(
	ipa_table*                  table,
	void*                       rec_ptr,   /* empty record in table */
	uint16_t                    rec_index, /* index of record above */
	void*                       user_data,
	struct ipa_ioc_nat_dma_cmd* cmd );

static int InsertTail(
	ipa_table*                  table,
	void*                       rec_ptr,       /* occupied record at index below */
	uint16_t*                   rec_index_ptr, /* pointer to index of record above */
	void*                       user_data,
	struct ipa_ioc_nat_dma_cmd* cmd );

static uint16_t MakeEntryHdl(
	ipa_table* tbl,
	uint16_t   tbl_entry );

static int FindExpnTblFreeEntry(
	ipa_table* table,
	void**     free_entry,
	uint16_t*  entry_index );

static int Get2PowerTightUpperBound(
	uint16_t num);

static int GetEvenTightUpperBound(
	uint16_t num);

void ipa_table_init(
	ipa_table*           table,
	const char*          table_name,
	enum ipa3_nat_mem_in nmi,
	int                  entry_size,
	void*                meta,
	int                  meta_entry_size,
	ipa_table_entry_interface* entry_interface )
{
	IPADBG("In\n");

	memset(table, 0, sizeof(ipa_table));

	strlcpy(table->name, table_name, IPA_RESOURCE_NAME_MAX);

	table->nmi             = nmi;
	table->entry_size      = entry_size;
	table->meta            = meta;
	table->meta_entry_size = meta_entry_size;
	table->entry_interface = entry_interface;

	IPADBG("Table %s with entry size %d has been initialized\n",
		   table->name, table->entry_size);

	IPADBG("Out\n");
}

int ipa_table_calculate_entries_num(
	ipa_table*           table,
	uint16_t             number_of_entries,
	enum ipa3_nat_mem_in nmi)
{
	uint16_t table_entries, expn_table_entries;
	float btp, etp;
	int result = 0;

	IPADBG("In\n");

	if (number_of_entries > IPA_TABLE_MAX_ENTRIES)
	{
		IPAERR("Required number of %s entries %d exceeds the maximum %d\n",
			table->name, number_of_entries, IPA_TABLE_MAX_ENTRIES);
		result = -EINVAL;
		goto bail;
	}

	if ( nmi == IPA_NAT_MEM_IN_SRAM )
	{
		btp = IPA_BASE_TABLE_PCNT_4SRAM;
		etp = IPA_EXPANSION_TABLE_PCNT_4SRAM;
	}
	else
	{
		btp = IPA_BASE_TABLE_PERCENTAGE;
		etp = IPA_EXPANSION_TABLE_PERCENTAGE;
	}

	table_entries      = Get2PowerTightUpperBound(number_of_entries * btp);
	expn_table_entries = GetEvenTightUpperBound(number_of_entries * etp);

	table->tot_tbl_ents = table_entries + expn_table_entries;

	if ( table->tot_tbl_ents > IPA_TABLE_MAX_ENTRIES )
	{
		IPAERR("Required number of %s entries %u "
			   "(user provided %u) exceeds the maximum %u\n",
			   table->name,
			   table->tot_tbl_ents,
			   number_of_entries,
			   IPA_TABLE_MAX_ENTRIES);
		result = -EINVAL;
		goto bail;
	}

	table->table_entries      = table_entries;
	table->expn_table_entries = expn_table_entries;

	IPADBG("Num of %s entries:%u expn entries:%u total entries:%u\n",
		   table->name,
		   table->table_entries,
		   table->expn_table_entries,
		   table->tot_tbl_ents);

bail:
	IPADBG("Out\n");

	return result;
}

int ipa_table_calculate_size(ipa_table* table)
{
	int size = table->entry_size * (table->table_entries + table->expn_table_entries);

	IPADBG("In\n");

	IPADBG("%s size: %d\n", table->name, size);

	IPADBG("Out\n");

	return size;
}

uint8_t* ipa_table_calculate_addresses(
	ipa_table* table,
	uint8_t*   base_addr)
{
	uint8_t* result = NULL;

	IPADBG("In\n");

	table->table_addr = base_addr;
	table->expn_table_addr =
		table->table_addr + table->entry_size * table->table_entries;

	IPADBG("Table %s addresses: table_addr %pK expn_table_addr %pK\n",
		   table->name, table->table_addr, table->expn_table_addr);

	result = table->expn_table_addr + table->entry_size * table->expn_table_entries;

	IPADBG("Out\n");

	return result;
}

void ipa_table_reset(
	ipa_table* table)
{
	uint32_t i,tot;

	IPADBG("In\n");

	IPADBG("memset %s table to 0, %pK\n", table->name, table->table_addr);
	tot = table->entry_size * table->table_entries;
	for (i = 0; i < tot; i++)
		table->table_addr[i] = '\0';

	IPADBG("memset %s expn table to 0, %pK\n", table->name, table->expn_table_addr);
	tot = table->entry_size * table->expn_table_entries;
	for (i = 0; i < tot; i++)
		table->expn_table_addr[i] = '\0';

	IPADBG("Out\n");
}

int ipa_table_add_entry(
	ipa_table* table,
	void*      user_data,
	uint16_t*  rec_index_ptr,
	uint32_t*  rule_hdl,
	struct ipa_ioc_nat_dma_cmd* cmd )
{
	void* rec_ptr;
	int ret = 0, occupied;

	IPADBG("In\n");

	rec_ptr = GOTO_REC(table, *rec_index_ptr);

	/*
	 * Check whether there is any collision
	 */
	occupied = table->entry_interface->entry_is_valid(rec_ptr);

	if ( ! occupied )
	{
		IPADBG("Collision free (in %s) ... found open slot\n", table->name);
		ret = InsertHead(table, rec_ptr, *rec_index_ptr, user_data, cmd);
	}
	else
	{
		IPADBG("Collision (in %s) ... will probe for open slot\n", table->name);
		ret = InsertTail(table, rec_ptr, rec_index_ptr, user_data, cmd);
	}

	if (ret)
		goto bail;

	IPADBG("New Entry Index %u in %s\n", *rec_index_ptr, table->name);

	if ( rule_hdl ) {
		*rule_hdl = MakeEntryHdl(table, *rec_index_ptr);
		IPADBG("rule_hdl value(%u)\n", *rule_hdl);
	}

bail:
	IPADBG("Out\n");

	return ret;
}

void ipa_table_create_delete_command(
	ipa_table* table,
	struct ipa_ioc_nat_dma_cmd* cmd,
	ipa_table_iterator* iterator)
{
	IPADBG("In\n");

	IPADBG("Delete rule at index(0x%04X) in %s\n",
		   iterator->curr_index,
		   table->name);

	if ( ! VALID_INDEX(iterator->prev_index) )
	{
		/*
		 * The following two assigns (ie. the defaults), will cause
		 * the enabled bit in the record to be set to 0.
		 */
		uint16_t      data = 0;
		dma_help_type ht   = HELP_UPDATE_HEAD;

		if ( VALID_INDEX(iterator->next_index) )
		{
			/*
			 * NOTE WELL HERE:
			 *
			 * This record is the first in a chain/list of
			 * records. Delete means something different in this
			 * context.
			 *
			 * The code below will cause the change of the protocol
			 * field in the rule record to 0xFF.  It does not set the
			 * enable bit in the record to 0.  This is done in special
			 * cases when the record being deleted is the first in a
			 * list of records.
			 *
			 * What does this mean?  It means that the record is
			 * functionally deleted, but not really deleted.  Why?
			 * Because the IPA will no longer use it because of the
			 * bad protocol (ie. functionally deleted), but these
			 * higher level APIs still see it as "enabled."
			 *
			 * This all means that deleted really means two things: 1)
			 * Not enabled, and 2) Not a valid record.  APIs that walk
			 * the table...looking for enabled records (ie. the
			 * enabled bit)....now have to be a bit smarter to see the
			 * bad protocol as well.
			 */
			data = table->entry_interface->
				entry_get_delete_head_dma_command_data(
					iterator->curr_entry, iterator->next_entry);

			ht = HELP_DELETE_HEAD;
		}

		ipa_table_add_dma_cmd(table,
							  ht,
							  iterator->curr_entry,
							  iterator->curr_index,
							  data,
							  cmd);
	}
	else
	{
		ipa_table_add_dma_cmd(table,
							  HELP_UPDATE_ENTRY,
							  iterator->prev_entry,
							  iterator->prev_index,
							  iterator->next_index,
							  cmd);
	}

	IPADBG("Out\n");
}

void ipa_table_delete_entry(
	ipa_table*          table,
	ipa_table_iterator* iterator,
	uint8_t             is_prev_empty)
{
	IPADBG("In\n");

	if ( VALID_INDEX(iterator->next_index) )
	{
		/*
		 * Update the next entry's prev_index field with current
		 * entry's prev_index
		 */
		table->entry_interface->entry_set_prev_index(
			iterator->next_entry,
			iterator->next_index,
			iterator->prev_index,
			table->meta,
			table->table_entries);
	}
	else if (is_prev_empty)
	{
		if (iterator->prev_entry == NULL)
		{
			IPAERR("failed to delete of an empty head %d while delete the next entry %d in %s",
				   iterator->prev_index, iterator->curr_index, table->name);
		}
		else
		{
			/*
			 * Delete an empty head rule after the whole tail was deleted
			 */
			IPADBG("deleting the dead node %d for %s\n",
				   iterator->prev_index, table->name);

			memset(iterator->prev_entry, 0, table->entry_size);

			--table->cur_tbl_cnt;
		}
	}

	ipa_table_erase_entry(table, iterator->curr_index);

	IPADBG("Out\n");
}

void ipa_table_erase_entry(
	ipa_table* table,
	uint16_t   index)
{
	void* entry = GOTO_REC(table, index);

	IPADBG("In\n");

	IPADBG("table(%p) index(%u)\n", table, index);

	memset(entry, 0, table->entry_size);

	if ( index < table->table_entries )
	{
		--table->cur_tbl_cnt;
	}
	else
	{
		--table->cur_expn_tbl_cnt;
	}

	IPADBG("Out\n");
}

/**
 * ipa_table_get_entry() - returns a table entry according to the received entry handle
 * @table: [in] the table
 * @entry_handle: [in] entry handle
 * @entry: [out] the retrieved entry
 * @entry_index: [out] absolute index of the retrieved entry
 *
 * Parse the entry handle to retrieve the entry and its index
 *
 * Returns: 0 on success, negative on failure
 */
int ipa_table_get_entry(
	ipa_table* table,
	uint32_t   entry_handle,
	void**     entry,
	uint16_t*  entry_index )
{
	enum ipa3_nat_mem_in nmi;
	uint8_t              is_expn_tbl;
	uint16_t             rec_index;

	int ret = 0;

	IPADBG("In\n");

	IPADBG("table(%p) entry_handle(%u) entry(%p) entry_index(%p)\n",
		   table, entry_handle, entry, entry_index);

	/*
	 * Retrieve the memory and table type as well as the index
	 */
	BREAK_RULE_HDL(table, entry_handle, nmi, is_expn_tbl, rec_index);

	if ( is_expn_tbl )
	{
		IPADBG("Retrieving entry from expansion table\n");
	}
	else
	{
		IPADBG("Retrieving entry from base (non-expansion) table\n");
	}

	if ( rec_index >= table->tot_tbl_ents )
	{
		IPAERR("The entry handle's record index (%u) exceeds table size (%u)\n",
			   rec_index, table->tot_tbl_ents);
		ret = -EINVAL;
		goto bail;
	}

	*entry = GOTO_REC(table, rec_index);

	if ( entry_index )
	{
		*entry_index = rec_index;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

void* ipa_table_get_entry_by_index(
	ipa_table* table,
	uint16_t   rec_index )
{
	void*    result = NULL;

	IPADBG("In\n");

	IPADBG("table(%p) rec_index(%u)\n",
		   table,
		   rec_index);

	if ( ! rec_index || rec_index >= table->tot_tbl_ents )
	{
		IPAERR("Invalid record index (%u): It's "
			   "either zero or exceeds table size (%u)\n",
			   rec_index, table->tot_tbl_ents);
		goto bail;
	}

	result = GOTO_REC(table, rec_index);

bail:
	IPADBG("Out\n");

	return result;
}

void ipa_table_dma_cmd_helper_init(
	ipa_table_dma_cmd_helper* dma_cmd_helper,
	uint8_t table_indx,
	ipa_table_dma_type table_type,
	ipa_table_dma_type expn_table_type,
	uint32_t offset)
{
	IPADBG("In\n");

	dma_cmd_helper->offset = offset;
	dma_cmd_helper->table_indx = table_indx;
	dma_cmd_helper->table_type = table_type;
	dma_cmd_helper->expn_table_type = expn_table_type;

	IPADBG("Out\n");
}

void ipa_table_dma_cmd_generate(
	ipa_table_dma_cmd_helper* dma_cmd_helper,
	uint8_t is_expn,
	uint32_t entry_offset,
	uint16_t data,
	struct ipa_ioc_nat_dma_cmd* cmd)
{
	struct ipa_ioc_nat_dma_one* dma = &cmd->dma[cmd->entries];

	IPADBG("In\n");

	IPADBG("is_expn(0x%02X) entry_offset(0x%08X) data(0x%04X)\n",
		   is_expn, entry_offset, data);

	dma->table_index = dma_cmd_helper->table_indx;

	/*
	 * DMA parameter base_addr is the table type (see the IPA
	 * architecture document)
	 */
	dma->base_addr =
		(is_expn) ?
		dma_cmd_helper->expn_table_type :
		dma_cmd_helper->table_type;

	dma->offset = dma_cmd_helper->offset + entry_offset;

	dma->data = data;

	IPADBG("dma_entry[%u](table_index(0x%02X) "
		   "base_addr(0x%02X) data(0x%04X) offset(0x%08X))\n",
		   cmd->entries,
		   dma->table_index,
		   dma->base_addr,
		   dma->data,
		   dma->offset);

	cmd->entries++;

	IPADBG("Out\n");
}

int ipa_table_iterator_init(
	ipa_table_iterator* iterator,
	ipa_table*          table,
	void*               curr_entry,
	uint16_t            curr_index)
{
	int occupied;

	int ret = 0;

	IPADBG("In\n");

	memset(iterator, 0, sizeof(ipa_table_iterator));

	occupied = table->entry_interface->entry_is_valid(curr_entry);

	if ( ! occupied )
	{
		IPAERR("Invalid (not enabled) rule %u in %s\n", curr_index, table->name);
		ret = -EINVAL;
		goto bail;
	}

	iterator->curr_entry = curr_entry;
	iterator->curr_index = curr_index;

	iterator->prev_index = table->entry_interface->entry_get_prev_index(
		curr_entry,
		curr_index,
		table->meta,
		table->table_entries);

	iterator->next_index = table->entry_interface->entry_get_next_index(
		curr_entry);

	if ( VALID_INDEX(iterator->prev_index) )
	{
		iterator->prev_entry = ipa_table_get_entry_by_index(
			table,
			iterator->prev_index);

		if ( iterator->prev_entry == NULL )
		{
			IPAERR("Failed to retrieve the entry at index 0x%04X for %s\n",
				   iterator->prev_index, table->name);
			ret = -EPERM;
			goto bail;
		}
	}

	if ( VALID_INDEX(iterator->next_index) )
	{
		iterator->next_entry = ipa_table_get_entry_by_index(
			table,
			iterator->next_index);

		if ( iterator->next_entry == NULL )
		{
			IPAERR("Failed to retrieve the entry at index 0x%04X for %s\n",
				   iterator->next_index, table->name);
			ret = -EPERM;
			goto bail;
		}
	}

	IPADBG("[index/entry] for "
		   "prev:[0x%04X/%p] "
		   "curr:[0x%04X/%p] "
		   "next:[0x%04X/%p] "
		   "\"%s\"\n",
		   iterator->prev_index,
		   iterator->prev_entry,
		   iterator->curr_index,
		   iterator->curr_entry,
		   iterator->next_index,
		   iterator->next_entry,
		   table->name);

bail:
	IPADBG("Out\n");

	return ret;
}

int ipa_table_iterator_next(
	ipa_table_iterator* iterator,
	ipa_table*          table)
{
	int ret = 0;

	IPADBG("In\n");

	iterator->prev_entry = iterator->curr_entry;
	iterator->prev_index = iterator->curr_index;
	iterator->curr_entry = iterator->next_entry;
	iterator->curr_index = iterator->next_index;

	iterator->next_index = table->entry_interface->entry_get_next_index(
		iterator->curr_entry);

	if ( ! VALID_INDEX(iterator->next_index) )
	{
		iterator->next_entry = NULL;
	}
	else
	{
		iterator->next_entry = ipa_table_get_entry_by_index(
			table, iterator->next_index);

		if (iterator->next_entry == NULL)
		{
			IPAERR("Failed to retrieve the entry at index %d for %s\n",
				   iterator->next_index, table->name);
			ret = -EPERM;
			goto bail;
		}
	}

	IPADBG("Iterator moved to: prev_index=%d curr_index=%d next_index=%d\n",
		   iterator->prev_index, iterator->curr_index, iterator->next_index);

	IPADBG("                   prev_entry=%pK curr_entry=%pK next_entry=%pK\n",
		   iterator->prev_entry, iterator->curr_entry, iterator->next_entry);

bail:
	IPADBG("Out\n");

	return ret;
}

int ipa_table_iterator_end(
	ipa_table_iterator* iterator,
	ipa_table*          table_ptr,
	uint16_t            rec_index,  /* a table slot relative to hash */
	void*               rec_ptr )   /* occupant record at index above */
{
	bool found_end = false;

	int ret;

	IPADBG("In\n");

	if ( ! iterator || ! table_ptr || ! rec_ptr )
	{
		IPAERR("Bad arg: iterator(%p) and/or table_ptr (%p) and/or rec_ptr(%p)\n",
			   iterator, table_ptr, rec_ptr);
		ret = -1;
		goto bail;
	}

	memset(iterator, 0, sizeof(ipa_table_iterator));

	iterator->prev_index = rec_index;
	iterator->prev_entry = rec_ptr;

	while ( 1 )
	{
		uint16_t next_index =
			table_ptr->entry_interface->entry_get_next_index(iterator->prev_entry);

		if ( ! VALID_INDEX(next_index) )
		{
			found_end = true;
			break;
		}

		if ( next_index == iterator->prev_index )
		{
			IPAERR("next_index(%u) and prev_index(%u) shouldn't be equal in %s\n",
				   next_index,
				   iterator->prev_index,
				   table_ptr->name);
			break;
		}

		iterator->prev_index = next_index;
		iterator->prev_entry = GOTO_REC(table_ptr, next_index);
	}

	if ( found_end )
	{
		IPADBG("Iterator found end of list record\n");
		ret = 0;
	}
	else
	{
		IPAERR("Iterator can't find end of list record\n");
		ret = -1;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

int ipa_table_iterator_is_head_with_tail(
	ipa_table_iterator* iterator)
{
	int ret = 0;

	IPADBG("In\n");

	ret = VALID_INDEX(iterator->next_index) && ! VALID_INDEX(iterator->prev_index);

	IPADBG("Out\n");

	return ret;
}

static int InsertHead(
	ipa_table*                  table,
	void*                       rec_ptr,   /* empty record in table */
	uint16_t                    rec_index, /* index of record above */
	void*                       user_data,
	struct ipa_ioc_nat_dma_cmd* cmd )
{
	uint16_t enable_data = 0;

	int ret = 0;

	IPADBG("In\n");

	ret = table->entry_interface->entry_head_insert(
		rec_ptr,
		user_data,
		&enable_data);

	if (ret)
	{
		IPAERR("unable to insert a new entry to the head in %s\n", table->name);
		goto bail;
	}

	ipa_table_add_dma_cmd(
		table,
		HELP_UPDATE_HEAD,
		rec_ptr,
		rec_index,
		enable_data,
		cmd);

	++table->cur_tbl_cnt;

bail:
	IPADBG("Out\n");

	return ret;
}

static int InsertTail(
	ipa_table*                  table,
	void*                       rec_ptr,       /* occupied record at index below */
	uint16_t*                   rec_index_ptr, /* pointer to index of record above */
	void*                       user_data,
	struct ipa_ioc_nat_dma_cmd* cmd )
{
	bool is_index_tbl = (table->meta) ? true : false;

	ipa_table_iterator iterator;

	uint16_t enable_data = 0;

	int ret = 0;

	IPADBG("In\n");

	/*
	 * The most important side effect of the following is to set the
	 * iterator's prev_index and prev_entry...which will be the last
	 * valid entry on the end of the list.
	 */
	ret = ipa_table_iterator_end(&iterator, table, *rec_index_ptr, rec_ptr);

	if ( ret )
	{
		IPAERR("Failed to reach the end of list following rec_index(%u) in %s\n",
			   *rec_index_ptr, table->name);
		goto bail;
	}

	/*
	 * The most important side effect of the following is to set the
	 * iterator's curr_index and curr_entry with the next available
	 * expansion table open slot.
	 */
	ret = FindExpnTblFreeEntry(table, &iterator.curr_entry, &iterator.curr_index);

	if ( ret )
	{
		IPAERR("FindExpnTblFreeEntry of %s failed\n", table->name);
		goto bail;
	}

	/*
	 * Copy data into curr_entry (ie. open slot).
	 */
	if ( is_index_tbl )
	{
		ret = table->entry_interface->entry_tail_insert(
			iterator.curr_entry,
			user_data);
	}
	else
	{
		/*
		 * We need enable bit when not index table, hence...
		 */
		ret = table->entry_interface->entry_head_insert(
			iterator.curr_entry,
			user_data,
			&enable_data);
	}

	if (ret)
	{
		IPAERR("Unable to insert a new entry to the tail in %s\n", table->name);
		goto bail;
	}

	/*
	 * Update curr_entry's prev_index field with iterator.prev_index
	 */
	table->entry_interface->entry_set_prev_index(
		iterator.curr_entry, /* set by FindExpnTblFreeEntry above */
		iterator.curr_index, /* set by FindExpnTblFreeEntry above */
		iterator.prev_index, /* set by ipa_table_iterator_end above */
		table->meta,
		table->table_entries);

	if ( ! is_index_tbl )
	{
		/*
		 * Generate dma command to have the IPA update the
		 * curr_entry's enable field when not the index table...
		 */
		ipa_table_add_dma_cmd(
			table,
			HELP_UPDATE_HEAD,
			iterator.curr_entry,
			iterator.curr_index,
			enable_data,
			cmd);
	}

	/*
	 * Generate a dma command to have the IPA update the prev_entry's
	 * next_index with iterator.curr_index.
	 */
	ipa_table_add_dma_cmd(
		table,
		HELP_UPDATE_ENTRY,
		iterator.prev_entry,
		iterator.prev_index,
		iterator.curr_index,
		cmd);

	++table->cur_expn_tbl_cnt;

	*rec_index_ptr = iterator.curr_index;

bail:
	IPADBG("Out\n");

	return ret;
}

/**
 * MakeEntryHdl() - makes an entry handle
 * @tbl_hdl: [in] tbl - the table
 * @tbl_entry: [in] tbl_entry - table entry
 *
 * Calculate the entry handle which will be returned to client
 *
 * Returns: >0 table entry handle
 */
static uint16_t MakeEntryHdl(
	ipa_table* tbl,
	uint16_t   tbl_entry )
{
	uint16_t entry_hdl = 0;

	IPADBG("In\n");

	if (tbl_entry >= tbl->table_entries)
	{
		/*
		 * Update the index into table
		 */
		entry_hdl = tbl_entry - tbl->table_entries;
		entry_hdl = (entry_hdl << IPA_TABLE_TYPE_BITS);
		/*
		 * Update the expansion table type bit
		 */
		entry_hdl = (entry_hdl | IPA_TABLE_TYPE_MASK);
	}
	else
	{
		entry_hdl = tbl_entry;
		entry_hdl = (entry_hdl << IPA_TABLE_TYPE_BITS);
	}

	/*
	 * Set memory type bit.
	 */
	entry_hdl = entry_hdl | (tbl->nmi << IPA_TABLE_TYPE_MEM_SHIFT);

	IPADBG("In: tbl_entry(%u) Out: entry_hdl(%u)\n", tbl_entry, entry_hdl);

	IPADBG("Out\n");

	return entry_hdl;
}

static int mt_slot(
	ipa_table*      table_ptr,
	uint32_t        rule_hdl,
	void*           record_ptr,
	uint16_t        record_index,
	void*           meta_record_ptr,
	uint16_t        meta_record_index,
	void*           arb_data_ptr )
{
	IPADBG("%s: Empty expansion slot: (%u) in table of size: (%u)\n",
		   table_ptr->name,
		   record_index,
		   table_ptr->tot_tbl_ents);

	return record_index;
}

/*
 * returns expn table entry absolute index
 */
static int FindExpnTblFreeEntry(
	ipa_table* table,
	void**     free_entry,
	uint16_t*  entry_index )
{
	int ret;

	IPADBG("In\n");

	if ( ! table || ! free_entry || ! entry_index )
	{
		IPAERR("Bad arg: table(%p) and/or "
			   "free_entry(%p) and/or entry_index(%p)\n",
			   table, free_entry, entry_index);
		ret = -1;
		goto bail;
	}

	*entry_index = 0;
	*free_entry  = NULL;

	/*
	 * The following will start walk at expansion slots
	 * (ie. just after table->table_entries)...
	 */
	ret = ipa_table_walk(table, table->table_entries, WHEN_SLOT_EMPTY, mt_slot, 0);

	if ( ret > 0 )
	{
		*entry_index = (uint16_t) ret;

		*free_entry = GOTO_REC(table, *entry_index);

		IPADBG("%s: entry_index val (%u) free_entry val (%p)\n",
			   table->name,
			   *entry_index,
			   *free_entry);

		ret = 0;
	}
	else
	{
		if ( ret < 0 )
		{
			IPAERR("%s: While searching table for emtpy slot\n",
				   table->name);
		}
		else
		{
			IPADBG("%s: No empty slots (ie. expansion table full): "
				   "BASE (avail/used): (%u/%u) EXPN (avail/used): (%u/%u)\n",
				   table->name,
				   table->table_entries,
				   table->cur_tbl_cnt,
				   table->expn_table_entries,
				   table->cur_expn_tbl_cnt);
		}

		ret = -1;
	}

bail:
	IPADBG("Out\n");

	return ret;
}

/**
 * Get2PowerTightUpperBound() - Returns the tight upper bound which is a power of 2
 * @num: [in] given number
 *
 * Returns the tight upper bound for a given number which is power of 2
 *
 * Returns: the tight upper bound which is power of 2
 */
static int Get2PowerTightUpperBound(uint16_t num)
{
	uint16_t tmp = num, prev = 0, curr = 2;

	if (num == 0)
		return 2;

	while (tmp != 1)
	{
		prev = curr;
		curr <<= 1;
		tmp >>= 1;
	}

	return (num == prev) ? prev : curr;
}

/**
 * GetEvenTightUpperBound() - Returns the tight upper bound which is an even number
 * @num: [in] given number
 *
 * Returns the tight upper bound for a given number which is an even number
 *
 * Returns: the tight upper bound which is an even number
 */
static int GetEvenTightUpperBound(uint16_t num)
{
	if (num == 0)
		return 2;

	return (num % 2) ? num + 1 : num;
}

int ipa_calc_num_sram_table_entries(
	uint32_t  sram_size,
	uint32_t  table1_ent_size,
	uint32_t  table2_ent_size,
	uint16_t* num_entries_ptr)
{
	ipa_table nat_table;
	ipa_table index_table;
	int       size = 0;
	uint16_t  tot;

	IPADBG("In\n");

	IPADBG("sram_size(%x or %u)\n", sram_size, sram_size);

	*num_entries_ptr = 0;

	tot = 1;

	while ( 1 )
	{
		IPADBG("Trying %u entries\n", tot);

		ipa_table_init(&nat_table,
					   "tmp_sram_table1",
					   IPA_NAT_MEM_IN_DDR,
					   table1_ent_size,
					   NULL,
					   0,
					   NULL);

		ipa_table_init(&index_table,
					   "tmp_sram_table1",
					   IPA_NAT_MEM_IN_DDR,
					   table2_ent_size,
					   NULL,
					   0,
					   NULL);

		nat_table.table_entries = index_table.table_entries =
			Get2PowerTightUpperBound(tot * IPA_BASE_TABLE_PCNT_4SRAM);
		nat_table.expn_table_entries = index_table.expn_table_entries =
			GetEvenTightUpperBound(tot * IPA_EXPANSION_TABLE_PCNT_4SRAM);

		size  = ipa_table_calculate_size(&nat_table);
		size += ipa_table_calculate_size(&index_table);

		IPADBG("%u entries consumes size(0x%x or %u)\n", tot, size, size);

		if ( size > sram_size )
			break;

		*num_entries_ptr = tot;

		++tot;
	}

	IPADBG("Optimal number of entries: %u\n", *num_entries_ptr);

	IPADBG("Out\n");

	return (*num_entries_ptr) ? 0 : -1;
}

int ipa_table_walk(
	ipa_table*        ipa_tbl_ptr,
	uint16_t          start_index,
	When2Callback     when2cb,
	ipa_table_walk_cb walk_cb,
	void*             arb_data_ptr )
{
	uint16_t i;
	uint32_t tot;
	uint8_t* rec_ptr;
	void*    meta_record_ptr;
	uint16_t meta_record_index;

	int ret = 0;

	IPADBG("In\n");

	if ( ! ipa_tbl_ptr ||
		 ! VALID_WHEN2CALLBACK(when2cb) ||
		 ! walk_cb )
	{
		IPAERR("Bad arg: ipa_tbl_ptr(%p) and/or "
			   "when2cb(%u) and/or walk_cb(%p)\n",
			   ipa_tbl_ptr,
			   when2cb,
			   walk_cb);
		ret = -EINVAL;
		goto bail;
	}

	tot =
		ipa_tbl_ptr->table_entries +
		ipa_tbl_ptr->expn_table_entries;

	if ( start_index >= tot )
	{
		IPAERR("Bad arg: start_index(%u)\n", start_index);
		ret = -EINVAL;
		goto bail;
	}

	/*
	 * Go through table...
	 */
	for ( i = start_index, rec_ptr = GOTO_REC(ipa_tbl_ptr, start_index);
		  i < tot;
		  i++,             rec_ptr += ipa_tbl_ptr->entry_size )
	{
		bool call_back;

		if ( ipa_tbl_ptr->entry_interface->entry_is_valid(rec_ptr) )
		{
			call_back = (when2cb == WHEN_SLOT_FILLED) ? true : false;
		}
		else
		{
			call_back = (when2cb == WHEN_SLOT_EMPTY)  ? true : false;
		}

		if ( call_back )
		{
			uint32_t rule_hdl = MakeEntryHdl(ipa_tbl_ptr, i);

			meta_record_ptr   = NULL;
			meta_record_index = 0;

			if ( i >= ipa_tbl_ptr->table_entries && ipa_tbl_ptr->meta )
			{
				meta_record_index = i - ipa_tbl_ptr->table_entries;

				meta_record_ptr = (uint8_t*) ipa_tbl_ptr->meta +
					(meta_record_index * ipa_tbl_ptr->meta_entry_size);
			}

			ret = walk_cb(
				ipa_tbl_ptr,
				rule_hdl,
				rec_ptr,
				i,
				meta_record_ptr,
				meta_record_index,
				arb_data_ptr);

			if ( ret != 0 )
			{
				if ( ret < 0 )
				{
					IPAERR("walk_cb returned non-zero (%d)\n", ret);
				}
				else
				{
					IPADBG("walk_cb returned non-zero (%d)\n", ret);
				}
				goto bail;
			}
		}
	}

bail:
	IPADBG("Out\n");

	return ret;
}

int ipa_table_add_dma_cmd(
	ipa_table*                  tbl_ptr,
	dma_help_type               help_type,
	void*                       rec_ptr,
	uint16_t                    rec_index,
	uint16_t                    data_for_entry,
	struct ipa_ioc_nat_dma_cmd* cmd_ptr )
{
	ipa_table_dma_cmd_helper* help_ptr;

	uint32_t tab_sz, entry_offset;

	uint8_t is_expn;

	int ret = 0;

	IPADBG("In\n");

	if ( ! tbl_ptr ||
		 ! VALID_DMA_HELP_TYPE(help_type) ||
		 ! rec_ptr ||
		 ! cmd_ptr )
	{
		IPAERR("Bad arg: tbl_ptr(%p) and/or help_type(%u) "
			   "and/or rec_ptr(%p) and/or cmd_ptr(%p)\n",
			   tbl_ptr,
			   help_type,
			   rec_ptr,
			   cmd_ptr);
		ret = -EINVAL;
		goto bail;
	}

	tab_sz =
		tbl_ptr->table_entries +
		tbl_ptr->expn_table_entries;

	if ( rec_index >= tab_sz )
	{
		IPAERR("Bad arg: rec_index(%u)\n", rec_index);
		ret = -EINVAL;
		goto bail;
	}

	is_expn = (rec_index >= tbl_ptr->table_entries);

	entry_offset = (uint8_t*) rec_ptr -
		((is_expn) ? tbl_ptr->expn_table_addr : tbl_ptr->table_addr);

	ipa_table_dma_cmd_generate(
		tbl_ptr->dma_help[help_type],
		is_expn,
		entry_offset,
		data_for_entry,
		cmd_ptr);

bail:
	IPADBG("Out\n");

	return ret;
}
