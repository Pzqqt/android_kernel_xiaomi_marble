/*
 * Copyright (c) 2014, 2018-2019 The Linux Foundation. All rights reserved.
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
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "ipa_nat_test.h"
#include "ipa_nat_map.h"

#undef strcasesame
#define strcasesame(x, y) \
	(! strcasecmp((x), (y)))

static inline const char* legal_mem_type(
	const char* mt )
{
	if ( strcasesame(mt, "DDR") )    return "DDR";
	if ( strcasesame(mt, "SRAM") )   return "SRAM";
	if ( strcasesame(mt, "HYBRID") ) return "HYBRID";
	return NULL;
}

static int nat_rule_loop_check(
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
	uint32_t             tbl_hdl = (uint32_t) arb_data_ptr;

	struct ipa_nat_rule* rule_ptr =
		(struct ipa_nat_rule*) record_ptr;

	BREAK_RULE_HDL(table_ptr, rule_hdl, nmi, is_expn_tbl, rule_index);

	/*
	 * By virtue of this function being called back by the walk, this
	 * record_index is valid.  Denote it as such in the map...
	 */
	if ( ipa_nat_map_add(MAP_NUM_99, record_index, 1) )
	{
		IPAERR("ipa_nat_map_add(index(%u)) failed\n", record_index);
		return -EINVAL;
	}

	if ( rule_ptr->next_index == record_index )
	{
		IPAERR("Infinite loop detected in IPv4 %s table, entry %u\n",
			   (is_expn_tbl) ? "expansion" : "base",
			   record_index);

		ipa_nat_dump_ipv4_table(tbl_hdl);

		return -EINVAL;
	}

	return 0;
}

static int nat_rule_validity_check(
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
	uint16_t             index;

	struct ipa_nat_rule* rule_ptr =
		(struct ipa_nat_rule*) record_ptr;

	BREAK_RULE_HDL(table_ptr, rule_hdl, nmi, is_expn_tbl, rule_index);

	index = rule_ptr->next_index;

	if ( index && ipa_nat_map_find(MAP_NUM_99, index, NULL) )
	{
		IPAERR("Invalid next index %u found in IPv4 %s table entry %u\n",
			   index,
			   (is_expn_tbl) ? "expansion" : "base",
			   rule_index);

		return -EINVAL;
	}

	if ( is_expn_tbl )
	{
		index = rule_ptr->prev_index;

		if ( index && ipa_nat_map_find(MAP_NUM_99, index, NULL) )
		{
			IPAERR("Invalid previous index %u found in IPv4 %s table entry %u\n",
				   index,
				   "expansion",
				   rule_index);

			return -EINVAL;
		}
	}

	return 0;
}

static int index_loop_check(
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
	uint32_t             tbl_hdl = (uint32_t) arb_data_ptr;

	struct ipa_nat_indx_tbl_rule* itr_ptr =
		(struct ipa_nat_indx_tbl_rule*) record_ptr;

	BREAK_RULE_HDL(table_ptr, rule_hdl, nmi, is_expn_tbl, rule_index);

	/*
	 * By virtue of this function being called back by the walk, this
	 * record_index is valid.  Denote it as such in the map...
	 */
	if ( ipa_nat_map_add(MAP_NUM_99, record_index, 1) )
	{
		IPAERR("ipa_nat_map_add(index(%u)) failed\n", record_index);
		return -EINVAL;
	}

	if ( itr_ptr->next_index == record_index )
	{
		IPAERR("Infinite loop detected in IPv4 index %s table, entry %u\n",
			   (is_expn_tbl) ? "expansion" : "base",
			   record_index);

		ipa_nat_dump_ipv4_table(tbl_hdl);

		return -EINVAL;
	}

	return 0;
}

static int index_validity_check(
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
	uint16_t             index;

	struct ipa_nat_indx_tbl_rule* itr_ptr =
		(struct ipa_nat_indx_tbl_rule*) record_ptr;

	BREAK_RULE_HDL(table_ptr, rule_hdl, nmi, is_expn_tbl, rule_index);

	index = itr_ptr->next_index;

	if ( index && ipa_nat_map_find(MAP_NUM_99, index, NULL) )
	{
		IPAERR("Invalid next index %u found in IPv4 index %s table entry %u\n",
			   index,
			   (is_expn_tbl) ? "expansion" : "base",
			   rule_index);

		return -EINVAL;
	}

	if ( is_expn_tbl )
	{
		struct ipa_nat_indx_tbl_meta_info* mi_ptr = meta_record_ptr;

		if ( ! mi_ptr )
		{
			IPAERR("Missing meta pointer for IPv4 index %s table entry %u\n",
				   "expansion",
				   rule_index);

			return -EINVAL;
		}

		index = mi_ptr->prev_index;

		if ( index && ipa_nat_map_find(MAP_NUM_99, index, NULL) )
		{
			IPAERR("Invalid previous index %u found in IPv4 index %s table entry %u\n",
				   index,
				   "expansion",
				   rule_index);

			return -EINVAL;
		}
	}

	return 0;
}

int ipa_nat_validate_ipv4_table(
	u32 tbl_hdl )
{
	int ret;

	/*
	 * Map MAP_NUM_99 will be used to keep, and to check for,
	 * record validity.
	 *
	 * The first walk will fill it. The second walk will use it...
	 */
	ipa_nat_map_clear(MAP_NUM_99);

	IPADBG("Checking IPv4 active rules:\n");

	ret = ipa_nati_walk_ipv4_tbl(tbl_hdl, USE_NAT_TABLE, nat_rule_loop_check, tbl_hdl);

	if ( ret != 0 )
	{
		return ret;
	}

	ret = ipa_nati_walk_ipv4_tbl(tbl_hdl, USE_NAT_TABLE, nat_rule_validity_check, 0);

	if ( ret != 0 )
	{
		return ret;
	}

	/*
	 * Map MAP_NUM_99 will be used to keep, and to check for,
	 * record validity.
	 *
	 * The first walk will fill it. The second walk will use it...
	 */
	ipa_nat_map_clear(MAP_NUM_99);

	IPADBG("Checking IPv4 index active rules:\n");

	ret = ipa_nati_walk_ipv4_tbl(tbl_hdl, USE_INDEX_TABLE, index_loop_check, tbl_hdl);

	if ( ret != 0 )
	{
		return ret;
	}

	ret = ipa_nati_walk_ipv4_tbl(tbl_hdl, USE_INDEX_TABLE, index_validity_check, 0);

	if ( ret != 0 )
	{
		return ret;
	}

	return 0;
}

static void
_dispUsage(
	const char* progNamePtr )
{
	printf(
		"Usage: %s [-d -r N -i N -e N -m mt]\n"
		"Where:\n"
		"  -d     Each test is discrete (create table, add rules, destroy table)\n"
		"         If not specified, only one table create and destroy for all tests\n"
		"  -r N   Where N is the number of times to run the inotify regression test\n"
		"  -i N   Where N is the number of times (iterations) to run test\n"
		"  -e N   Where N is the number of entries in the NAT\n"
		"  -m mt  Where mt is the type of memory to use for the NAT\n"
		"         Legal mt's: DDR, SRAM, or HYBRID (ie. use SRAM and DDR)\n"
		"  -g M-N Run tests M through N only\n",
		progNamePtr);

	fflush(stdout);
}

static NatTests nt_array[] = {
	NAT_TEST_ENTRY(ipa_nat_test000, 1, 0),
	NAT_TEST_ENTRY(ipa_nat_test001, 1, 0),
	NAT_TEST_ENTRY(ipa_nat_test002, 1, 0),
	NAT_TEST_ENTRY(ipa_nat_test003, 1, 0),
	NAT_TEST_ENTRY(ipa_nat_test004, 1, 0),
	NAT_TEST_ENTRY(ipa_nat_test005, 1, 0),
	NAT_TEST_ENTRY(ipa_nat_test006, 1, 0),
	NAT_TEST_ENTRY(ipa_nat_test007, 1, 0),
	NAT_TEST_ENTRY(ipa_nat_test008, 1, 0),
	NAT_TEST_ENTRY(ipa_nat_test009, 1, 0),
	NAT_TEST_ENTRY(ipa_nat_test010, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test011, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test012, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test013, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test014, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test015, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test016, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test017, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test018, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test019, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test020, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test021, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test022, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test023, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test024, IPA_NAT_TEST_PRE_COND_TE, 0),
	NAT_TEST_ENTRY(ipa_nat_test025, IPA_NAT_TEST_PRE_COND_TE, 0),
	/*
	 * Add new tests just above this comment. Keep the following two
	 * at the end...
	 */
	NAT_TEST_ENTRY(ipa_nat_test999, 1, 0),
	NAT_TEST_ENTRY(ipa_nat_testREG, 1, 0),
};

int main(
	int   argc,
	char* argv[] )
{
	int      sep        = 0;
	int      ireg       = 0;
	uint32_t nt         = 1;
	int      total_ents = 100;
	uint32_t ht         = 0;
	uint32_t start = 0, end = 0;

	char* nat_mem_type = "DDR";

	uint32_t tbl_hdl    = 0;

	uint32_t pub_ip_addr;

	uint32_t i, ub, cnt, exec, pass;

	void*    adp;

	time_t   t;

	int      c, ret;

	IPADBG("Testing user space nat driver\n");

	while ( (c = getopt(argc, argv, "dr:i:e:m:h:g:?")) != -1 )
	{
		switch (c)
		{
		case 'd':
			sep = 1;
			break;
		case 'r':
			ireg = atoi(optarg);
			break;
		case 'i':
			nt = atoi(optarg);
			break;
		case 'e':
			total_ents = atoi(optarg);
			break;
		case 'm':
			if ( ! (nat_mem_type = legal_mem_type(optarg)) )
			{
				fprintf(stderr, "Illegal: -m %s\n", optarg);
				_dispUsage(basename(argv[0]));
				exit(0);
			}
			break;
		case 'h':
			ht = atoi(optarg);
			break;
		case 'g':
			if ( sscanf(optarg, "%u-%u", &start, &end) != 2
				 ||
				 ( start >= end || end >= array_sz(nt_array) - 1 ) )
			{
				fprintf(stderr, "Illegal: -f %s\n", optarg);
				_dispUsage(basename(argv[0]));
				exit(0);
			}
			break;
		case '?':
		default:
			_dispUsage(basename(argv[0]));
			exit(0);
			break;
		}
	}

	srand(time(&t));

	pub_ip_addr = RAN_ADDR;

	exec = pass = 0;

	for ( cnt = ret = 0; cnt < nt && ret == 0; cnt++ )
	{
		IPADBG("ITERATION [%u] OF TESING\n", cnt + 1);

		if ( ireg )
		{
			adp = &ireg;
			i   = array_sz(nt_array) - 1;
			ub  = array_sz(nt_array);
		}
		else
		{
			adp = &tbl_hdl;
			i   = ( end ) ? start : 0;
			ub  = ( end ) ? end   : array_sz(nt_array) - 1;

			if ( i != 0 && ! sep )
			{
				ipa_nat_test000(
					nat_mem_type, pub_ip_addr, total_ents, tbl_hdl, 0, adp);
			}
		}

		for ( ; i < ub && ret == 0; i++ )
		{
			if ( total_ents >= nt_array[i].num_ents_trigger )
			{
				IPADBG("+------------------------------------------------+\n");
				IPADBG("|        Executing test: %s         |\n", nt_array[i].func_name);
				IPADBG("+------------------------------------------------+\n");

				ret = nt_array[i].func(
					nat_mem_type, pub_ip_addr, total_ents, tbl_hdl, sep, adp);

				exec++;

				if ( ret == 0 )
				{
					IPADBG("<<<<< Test %s SUCCEEDED >>>>>\n", nt_array[i].func_name);

					pass++;

					if ( ht || nt_array[i].test_hold_time_in_secs )
					{
						ht = (ht) ? ht : nt_array[i].test_hold_time_in_secs;

						sleep(ht);
					}
				}
				else
				{
					IPAERR("<<<<< Test %s FAILED >>>>>\n", nt_array[i].func_name);
				}
			}
		}
	}

	if ( ret && tbl_hdl )
	{
		ipa_nat_test999(
			nat_mem_type, pub_ip_addr, total_ents, tbl_hdl, 0, &tbl_hdl);
	}

	IPADBG("Total NAT Tests Run:%u, Pass:%u, Fail:%u\n",
		   exec, pass, exec - pass);

	return 0;
}
