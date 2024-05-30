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
#ifndef IPA_TABLE_H
#define IPA_TABLE_H

#include <stdint.h>
#include <stdbool.h>
#include <linux/msm_ipa.h>

#define IPA_TABLE_MAX_ENTRIES 5120

#define IPA_TABLE_INVALID_ENTRY 0x0

#undef  VALID_INDEX
#define VALID_INDEX(idx) \
	( (idx) != IPA_TABLE_INVALID_ENTRY )

#undef  VALID_RULE_HDL
#define VALID_RULE_HDL(hdl) \
	( (hdl) != IPA_TABLE_INVALID_ENTRY )

#undef GOTO_REC
#define GOTO_REC(tbl, rec_idx) \
	( (tbl)->table_addr + ((rec_idx) * (tbl)->entry_size) )

typedef enum
{
	IPA_NAT_BASE_TBL       = 0,
	IPA_NAT_EXPN_TBL       = 1,
	IPA_NAT_INDX_TBL       = 2,
	IPA_NAT_INDEX_EXPN_TBL = 3,
	IPA_IPV6CT_BASE_TBL    = 4,
	IPA_IPV6CT_EXPN_TBL    = 5,
} ipa_table_dma_type;

#define VALID_IPA_TABLE_DMA_TYPE(t) \
	( (t) >= IPA_NAT_BASE_TBL && (t) <= IPA_IPV6CT_EXPN_TBL )

/*
 *    --------- NAT Rule Handle Entry ID structure ---------
 *
 * +-----------+-----------+------------------+----------------+
 * |   1 bit   |  2 bits   |    12 bits       |     1 bit      |
 * +-----------+-----------+------------------+----------------+
 * | 0 - DDR   | reserved  | index into table |  0 - base      |
 * | 1 - SRAM  |           |                  |  1 - expansion |
 * +-----------+-----------+------------------+----------------+
 */
#define IPA_TABLE_TYPE_BITS      0x00000001
#define IPA_TABLE_TYPE_MASK      0x00000001
#define IPA_TABLE_INDX_MASK      0x00000FFF
#define IPA_TABLE_TYPE_MEM_SHIFT 15

#undef BREAK_RULE_HDL
#define BREAK_RULE_HDL(tbl, hdl, mt, iet, indx) \
	do { \
		mt    = ((hdl) >> IPA_TABLE_TYPE_MEM_SHIFT) & IPA_TABLE_TYPE_MASK; \
		iet   =  (hdl)                              & IPA_TABLE_TYPE_MASK; \
		indx  = ((hdl) >> IPA_TABLE_TYPE_BITS)      & IPA_TABLE_INDX_MASK; \
		indx += (iet) ? tbl->table_entries : 0; \
		/*IPADBG("hdl(%u) -> mt(%u) iet(%u) indx(%u)\n", hdl, mt, iet, indx);*/ \
	} while ( 0 )

typedef int (*entry_validity_checker)(
	void* entry);

typedef uint16_t (*entry_next_index_getter)(
	void* entry);

typedef uint16_t (*entry_prev_index_getter)(
	void*    entry,
	uint16_t entry_index,
	void*    meta,
	uint16_t base_table_size);

typedef void (*entry_prev_index_setter)(
	void*    entry,
	uint16_t entry_index,
	uint16_t prev_index,
	void*    meta,
	uint16_t base_table_size);

typedef int (*entry_head_inserter)(
	void*     entry,
	void*     user_data,
	uint16_t* dma_command_data);

typedef int (*entry_tail_inserter)(
	void* entry,
	void* user_data);

typedef uint16_t (*entry_delete_head_dma_command_data_getter)(
	void* head,
	void* next_entry);

typedef struct
{
	entry_validity_checker  entry_is_valid;
	entry_next_index_getter entry_get_next_index;
	entry_prev_index_getter entry_get_prev_index;
	entry_prev_index_setter entry_set_prev_index;
	entry_head_inserter     entry_head_insert;
	entry_tail_inserter     entry_tail_insert;
	entry_delete_head_dma_command_data_getter
	  entry_get_delete_head_dma_command_data;
} ipa_table_entry_interface;

typedef enum
{
	HELP_UPDATE_HEAD  = 0,
	HELP_UPDATE_ENTRY = 1,
	HELP_DELETE_HEAD  = 2,

	HELP_UPDATE_MAX,
} dma_help_type;

#undef VALID_DMA_HELP_TYPE
#define VALID_DMA_HELP_TYPE(t) \
	( (t) >=  HELP_UPDATE_HEAD && (t) < HELP_UPDATE_MAX )

typedef struct
{
	uint32_t           offset;
	ipa_table_dma_type table_type;
	ipa_table_dma_type expn_table_type;
	uint8_t            table_indx;
} ipa_table_dma_cmd_helper;

typedef struct
{
	char                       name[IPA_RESOURCE_NAME_MAX];

	enum ipa3_nat_mem_in       nmi;

	int                        entry_size;

	uint16_t                   table_entries;
	uint16_t                   expn_table_entries;
	uint32_t                   tot_tbl_ents;

	uint8_t*                   table_addr;
	uint8_t*                   expn_table_addr;

	uint16_t                   cur_tbl_cnt;
	uint16_t                   cur_expn_tbl_cnt;

	ipa_table_entry_interface* entry_interface;

	ipa_table_dma_cmd_helper*  dma_help[HELP_UPDATE_MAX];

	void*                      meta;
	int                        meta_entry_size;
} ipa_table;

typedef struct
{
	uint16_t prev_index;
	void*    prev_entry;

	uint16_t curr_index;
	void*    curr_entry;

	uint16_t next_index;
	void*    next_entry;
} ipa_table_iterator;


void ipa_table_init(
	ipa_table*                 table,
	const char*                table_name,
	enum ipa3_nat_mem_in       nmi,
	int                        entry_size,
	void*                      meta,
	int                        meta_entry_size,
	ipa_table_entry_interface* entry_interface);

int ipa_table_calculate_entries_num(
	ipa_table*           table,
	uint16_t             number_of_entries,
	enum ipa3_nat_mem_in nmi);

int ipa_table_calculate_size(
	ipa_table* table);

uint8_t* ipa_table_calculate_addresses(
	ipa_table* table,
	uint8_t*   base_addr);

void ipa_table_reset(
	ipa_table* table);

int ipa_table_add_entry(
	ipa_table*                  table,
	void*                       user_data,
	uint16_t*                   index,
	uint32_t*                   rule_hdl,
	struct ipa_ioc_nat_dma_cmd* cmd);

void ipa_table_create_delete_command(
	ipa_table*                  table,
	struct ipa_ioc_nat_dma_cmd* cmd,
	ipa_table_iterator*         iterator);

void ipa_table_delete_entry(
	ipa_table*          table,
	ipa_table_iterator* iterator,
	uint8_t             is_prev_empty);

void ipa_table_erase_entry(
	ipa_table* table,
	uint16_t   index);

int ipa_table_get_entry(
	ipa_table* table,
	uint32_t   entry_handle,
	void**     entry,
	uint16_t*  entry_index);

void* ipa_table_get_entry_by_index(
	ipa_table* table,
	uint16_t   index);

void ipa_table_dma_cmd_helper_init(
	ipa_table_dma_cmd_helper* dma_cmd_helper,
	uint8_t                   table_indx,
	ipa_table_dma_type        table_type,
	ipa_table_dma_type        expn_table_type,
	uint32_t                  offset);

void ipa_table_dma_cmd_generate(
	ipa_table_dma_cmd_helper*   dma_cmd_helper,
	uint8_t                     is_expn,
	uint32_t                    entry_offset,
	uint16_t                    data,
	struct ipa_ioc_nat_dma_cmd* cmd);

int ipa_table_iterator_init(
	ipa_table_iterator* iterator,
	ipa_table*          table,
	void*               curr_entry,
	uint16_t            curr_index);

int ipa_table_iterator_next(
	ipa_table_iterator* iterator,
	ipa_table*          table);

int ipa_table_iterator_end(
	ipa_table_iterator* iterator,
	ipa_table*          table,
	uint16_t            head_index,
	void*               head);

int ipa_table_iterator_is_head_with_tail(
	ipa_table_iterator* iterator);

int ipa_calc_num_sram_table_entries(
	uint32_t  sram_size,
	uint32_t  table1_ent_size,
	uint32_t  table2_ent_size,
	uint16_t* num_entries_ptr);

typedef int (*ipa_table_walk_cb)(
	ipa_table*      table_ptr,
	uint32_t        rule_hdl,
	void*           record_ptr,
	uint16_t        record_index,
	void*           meta_record_ptr,
	uint16_t        meta_record_index,
	void*           arb_data_ptr );

typedef enum
{
	WHEN_SLOT_EMPTY  = 0,
	WHEN_SLOT_FILLED = 1,

	WHEN_SLOT_MAX
} When2Callback;

#define VALID_WHEN2CALLBACK(w) \
	( (w) >= WHEN_SLOT_EMPTY && (w) < WHEN_SLOT_MAX )

int ipa_table_walk(
	ipa_table*        table,
	uint16_t          start_index,
	When2Callback     when,
	ipa_table_walk_cb walk_cb,
	void*             arb_data_ptr );

int ipa_table_add_dma_cmd(
	ipa_table*                  tbl_ptr,
	dma_help_type               help_type,
	void*                       rec_ptr,
	uint16_t                    rec_index,
	uint16_t                    data_for_entry,
	struct ipa_ioc_nat_dma_cmd* cmd_ptr );

#endif
