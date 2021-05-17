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

#ifndef IPA_NAT_DRVI_H
#define IPA_NAT_DRVI_H

#include "ipa_table.h"
#include "ipa_mem_descriptor.h"
#include "ipa_nat_utils.h"

#undef MAKE_TBL_HDL
#define MAKE_TBL_HDL(hdl, mt) \
	((mt) << 31 | (hdl))

#undef BREAK_TBL_HDL
#define BREAK_TBL_HDL(hdl_in, mt, hdl_out) \
	do { \
		mt      = (hdl_in) >> 31 & 0x0000000001; \
		hdl_out = (hdl_in)       & 0x00000000FF; \
	} while ( 0 )

#undef VALID_TBL_HDL
#define VALID_TBL_HDL(h) \
	(((h) & 0x00000000FF) == IPA_NAT_MAX_IP4_TBLS)

/*======= IMPLEMENTATION related data structures and functions ======= */

#define IPA_NAT_MAX_IP4_TBLS   1

#define IPA_NAT_RULE_FLAG_FIELD_OFFSET        18
#define IPA_NAT_RULE_NEXT_FIELD_OFFSET        8
#define IPA_NAT_RULE_PROTO_FIELD_OFFSET       22

#define IPA_NAT_INDEX_RULE_NEXT_FIELD_OFFSET       2
#define IPA_NAT_INDEX_RULE_NAT_INDEX_FIELD_OFFSET  0

#define IPA_NAT_FLAG_ENABLE_BIT  1

#define IPA_NAT_INVALID_PROTO_FIELD_VALUE 0xFF00
/*
 * IPA_NAT_INVALID_PROTO_FIELD_VALUE above is what's passed to the IPA
 * in a DMA command.  It is written into the NAT's rule, by the
 * IPA. After being written, It minifests in the rule in the form
 * below, hence it will be used when perusing the "struct
 * ipa_nat_rule".
 */
#define IPA_NAT_INVALID_PROTO_FIELD_VALUE_IN_RULE  0xFF

typedef enum {
	IPA_NAT_TABLE_FLAGS,
	IPA_NAT_TABLE_NEXT_INDEX,
	IPA_NAT_TABLE_PROTOCOL,
	IPA_NAT_INDEX_TABLE_ENTRY,
	IPA_NAT_INDEX_TABLE_NEXT_INDEX,
	IPA_NAT_TABLE_DMA_CMD_MAX
} ipa_nat_table_dma_cmd_type;

/*
 * ------------------------  NAT Table Entry  -----------------------------------------
 *
 * ------------------------------------------------------------------------------------
 * |   7    |    6    |   5    |    4    |     3        |  2   |    1    |    0       |
 * ------------------------------------------------------------------------------------
 * |             Target IP(4B)           |             Private IP(4B)                 |
 * ------------------------------------------------------------------------------------
 * |Target Port(2B)   | Private Port(2B) | Public Port(2B)     | Next Index(2B)       |
 * ------------------------------------------------------------------------------------
 * | Proto   |      TimeStamp(3B)        |       Flags(2B)     | IP check sum Diff(2B)|
 * | (1B)    |                           |EN|Redirect|Resv     |                      |
 * ------------------------------------------------------------------------------------
 * | TCP/UDP checksum |PDN info|Reserved |    SW Specific Parameters(4B)              |
 * |    diff (2B)     |  (1B)  |  (1B)   |                                            |
 * ------------------------------------------------------------------------------------
 *
 * Dont change below structure definition.
 *
 * It should be same as above(little endian order)
 *
 * -------------------------------------------------------------------------------
 */
struct ipa_nat_rule {
	uint64_t private_ip:32;
	uint64_t target_ip:32;

	uint64_t next_index:16;
	uint64_t public_port:16;
	uint64_t private_port:16;
	uint64_t target_port:16;

	uint64_t ip_chksum:16;

	/*--------------------------------------------------
	IPA NAT Flag is interpreted as follows
	---------------------------------------------------
	|  EN   |FIN/RST|  S   | IPv4 uC activation index |
	| [15]  | [14]  | [13] |          [12:0]          |
	---------------------------------------------------
	--------------------------------------------------*/
	uint64_t uc_activation_index:13;
	uint64_t s:1;
	uint64_t redirect:1;
	uint64_t enable:1;

	uint64_t time_stamp:24;
	uint64_t protocol:8;

	/*--------------------------------------------------
	32 bit sw_spec_params is interpreted as follows
	------------------------------------
	|     16 bits     |     16 bits    |
	------------------------------------
	|  index table    |  prev index    |
	|     entry       |                |
	------------------------------------
	--------------------------------------------------*/
	uint64_t prev_index:16;
	uint64_t indx_tbl_entry:16;
	uint64_t rsvd2:8;
	/*-----------------------------------------
	8 bit PDN info is interpreted as following
	-----------------------------------------------------
	|     4 bits      |     1 bit      |     3 bits     |
	-----------------------------------------------------
	|  PDN index      |  uC processing |  src dst Rsrv3 |
	|      [7:4]      |       [3]      |  [2] [1]  [0]  |
	-----------------------------------------------------
	-------------------------------------------*/
	uint64_t rsvd3:1;
	uint64_t dst_only:1;
	uint64_t src_only:1;
	uint64_t ucp:1;
	uint64_t pdn_index:4;

	uint64_t tcp_udp_chksum:16;
};

static inline char* prep_nat_rule_4print(
	struct ipa_nat_rule* rule_ptr,
	char*                buf_ptr,
	uint32_t             buf_sz )
{
	if ( rule_ptr && buf_ptr && buf_sz )
	{
		snprintf(
			buf_ptr, buf_sz,
			"NAT RULE: "
			"protocol(0x%02X) "
			"public_port(0x%04X) "
			"target_ip(0x%08X) "
			"target_port(0x%04X) "
			"private_ip(0x%08X) "
			"private_port(0x%04X) "
			"pdn_index(0x%02X) "
			"ip_chksum(0x%04X) "
			"tcp_udp_chksum(0x%04X) "
			"redirect(0x%02X) "
			"enable(0x%02X) "
			"time_stamp(0x%08X) "
			"indx_tbl_entry(0x%04X) "
			"prev_index(0x%04X) "
			"next_index(0x%04X)",
			rule_ptr->protocol,
			rule_ptr->public_port,
			rule_ptr->target_ip,
			rule_ptr->target_port,
			rule_ptr->private_ip,
			rule_ptr->private_port,
			rule_ptr->pdn_index,
			rule_ptr->ip_chksum,
			rule_ptr->tcp_udp_chksum,
			rule_ptr->redirect,
			rule_ptr->enable,
			rule_ptr->time_stamp,
			rule_ptr->indx_tbl_entry,
			rule_ptr->prev_index,
			rule_ptr->next_index);
	}

	return buf_ptr;
}

static inline const char *ipa3_nat_mem_in_as_str(
	enum ipa3_nat_mem_in nmi)
{
	switch (nmi) {
	case IPA_NAT_MEM_IN_DDR:
		return "IPA_NAT_MEM_IN_DDR";
	case IPA_NAT_MEM_IN_SRAM:
		return "IPA_NAT_MEM_IN_SRAM";
	default:
		break;
	}
	return "???";
}

static inline char *ipa_ioc_v4_nat_init_as_str(
	struct ipa_ioc_v4_nat_init *ptr,
	char                       *buf,
	uint32_t                    buf_sz)
{
	if (ptr && buf && buf_sz) {
		snprintf(
			buf, buf_sz,
			"V4 NAT INIT: tbl_index(0x%02X) ipv4_rules_offset(0x%08X) expn_rules_offset(0x%08X) index_offset(0x%08X) index_expn_offset(0x%08X) table_entries(0x%04X) expn_table_entries(0x%04X) ip_addr(0x%08X)",
			ptr->tbl_index,
			ptr->ipv4_rules_offset,
			ptr->expn_rules_offset,
			ptr->index_offset,
			ptr->index_expn_offset,
			ptr->table_entries,
			ptr->expn_table_entries,
			ptr->ip_addr);
	}
	return buf;
}

/*
	IPA NAT Flag is interpreted as follows
	---------------------------------------------------
	|  EN   |FIN/RST|  S   | IPv4 uC activation index |
	| [15]  | [14]  | [13] |          [12:0]          |
	---------------------------------------------------
*/
typedef struct {
	uint32_t uc_activation_index:13;
	uint32_t s:1;
	uint32_t redirect:1;
	uint32_t enable:1;
} ipa_nat_flags;

struct ipa_nat_indx_tbl_rule {
	uint16_t tbl_entry;
	uint16_t next_index;
};

struct ipa_nat_indx_tbl_meta_info {
	uint16_t prev_index;
};

struct ipa_nat_ip4_table_cache {
	uint32_t public_addr;
	ipa_mem_descriptor mem_desc;
	ipa_table table;
	ipa_table index_table;
	struct ipa_nat_indx_tbl_meta_info *index_expn_table_meta;
	ipa_table_dma_cmd_helper table_dma_cmd_helpers[IPA_NAT_TABLE_DMA_CMD_MAX];
};

struct ipa_nat_cache {
	ipa_descriptor* ipa_desc;
	struct ipa_nat_ip4_table_cache ip4_tbl[IPA_NAT_MAX_IP4_TBLS];
	uint8_t table_cnt;
	enum ipa3_nat_mem_in nmi;
};

int ipa_nati_add_ipv4_tbl(
	uint32_t    public_ip_addr,
	const char *mem_type_ptr,
	uint16_t    number_of_entries,
	uint32_t   *table_hanle);

int ipa_nati_del_ipv4_table(uint32_t tbl_hdl);

int ipa_nati_query_timestamp(uint32_t  tbl_hdl,
				uint32_t  rule_hdl,
				uint32_t  *time_stamp);

int ipa_nati_modify_pdn(struct ipa_ioc_nat_pdn_entry *entry);

int ipa_nati_get_pdn_index(uint32_t public_ip, uint8_t *pdn_index);

int ipa_nati_alloc_pdn(ipa_nat_pdn_entry *pdn_info, uint8_t *pdn_index);

int ipa_nati_get_pdn_cnt(void);

int ipa_nati_dealloc_pdn(uint8_t pdn_index);

int ipa_nati_add_ipv4_rule(uint32_t tbl_hdl,
				const ipa_nat_ipv4_rule *clnt_rule,
				uint32_t *rule_hdl);

int ipa_nati_del_ipv4_rule(uint32_t tbl_hdl,
				uint32_t rule_hdl);

int ipa_nati_get_sram_size(
	uint32_t* size_ptr);

int ipa_nati_clear_ipv4_tbl(
	uint32_t tbl_hdl );

int ipa_nati_copy_ipv4_tbl(
	uint32_t          src_tbl_hdl,
	uint32_t          dst_tbl_hdl,
	ipa_table_walk_cb copy_cb );

typedef enum
{
	USE_NAT_TABLE   = 0,
	USE_INDEX_TABLE = 1,

	USE_MAX
} WhichTbl2Use;

#define VALID_WHICHTBL2USE(w) \
	( (w) >= USE_NAT_TABLE && (w) < USE_MAX )

int ipa_nati_walk_ipv4_tbl(
	uint32_t          tbl_hdl,
	WhichTbl2Use      which,
	ipa_table_walk_cb walk_cb,
	void*             arb_data_ptr );

/*
 * The following used for retrieving table stats.
 */
typedef struct
{
	enum ipa3_nat_mem_in nmi;
	uint32_t tot_ents;
	uint32_t tot_base_ents;
	uint32_t tot_base_ents_filled;
	uint32_t tot_expn_ents;
	uint32_t tot_expn_ents_filled;
	uint32_t tot_chains;
	uint32_t min_chain_len;
	uint32_t max_chain_len;
	float    avg_chain_len;
} ipa_nati_tbl_stats;

int ipa_nati_ipv4_tbl_stats(
	uint32_t            tbl_hdl,
	ipa_nati_tbl_stats* nat_stats_ptr,
	ipa_nati_tbl_stats* idx_stats_ptr );

int ipa_nati_vote_clock(
	enum ipa_app_clock_vote_type vote_type );

int ipa_NATI_add_ipv4_tbl(
	enum ipa3_nat_mem_in nmi,
	uint32_t             public_ip_addr,
	uint16_t             number_of_entries,
	uint32_t*            tbl_hdl);

int ipa_NATI_del_ipv4_table(
	uint32_t tbl_hdl);

int ipa_NATI_clear_ipv4_tbl(
	uint32_t tbl_hdl );

int ipa_NATI_walk_ipv4_tbl(
	uint32_t          tbl_hdl,
	WhichTbl2Use      which,
	ipa_table_walk_cb walk_cb,
	void*             arb_data_ptr );

int ipa_NATI_ipv4_tbl_stats(
	uint32_t            tbl_hdl,
	ipa_nati_tbl_stats* nat_stats_ptr,
	ipa_nati_tbl_stats* idx_stats_ptr );

int ipa_NATI_query_timestamp(
	uint32_t  tbl_hdl,
	uint32_t  rule_hdl,
	uint32_t* time_stamp);

int ipa_NATI_add_ipv4_rule(
	uint32_t                 tbl_hdl,
	const ipa_nat_ipv4_rule* clnt_rule,
	uint32_t*                rule_hdl);

int ipa_NATI_del_ipv4_rule(
	uint32_t tbl_hdl,
	uint32_t rule_hdl);

int ipa_NATI_post_ipv4_init_cmd(
	uint32_t tbl_hdl );

#endif /* #ifndef IPA_NAT_DRVI_H */
