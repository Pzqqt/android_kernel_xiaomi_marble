/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

/*=========================================================================*/
/*!
	@file
	ipa_nat_test025.c

	@brief
	Note: Verify the following scenario:
	1. Similare to test022, but with random deletes during adds
*/
/*=========================================================================*/

#include "ipa_nat_test.h"

#undef  VALID_RULE
#define VALID_RULE(r) ((r) != 0 && (r) != 0xFFFFFFFF)

#undef GET_MAX
#define GET_MAX(ram, rdm) \
	do { \
		while ( (ram = rand() % 20) < 4); \
		while ( (rdm = rand() % 10) >= ram || rdm == 0 ); \
		IPADBG("rand_adds_max(%u) rand_dels_max(%u)\n", ram, rdm); \
	} while (0)

int ipa_nat_test025(
	const char* nat_mem_type,
	u32 pub_ip_add,
	int total_entries,
	u32 tbl_hdl,
	int sep,
	void* arb_data_ptr)
{
	int* tbl_hdl_ptr = (int*) arb_data_ptr;

	ipa_nat_ipv4_rule  ipv4_rule;
	u32                rule_hdls[1024];

	ipa_nati_tbl_stats nstats, last_nstats;
	ipa_nati_tbl_stats istats, last_istats;

	u32                i;
	u32                rand_adds_max, rand_dels_max;
	u32                tot, tot_added, tot_deleted;

	bool               switched = false;

	const char*        mem_type;

	int ret;

	IPADBG("In\n");

	if ( sep )
	{
		ret = ipa_nat_add_ipv4_tbl(pub_ip_add, nat_mem_type, total_entries, &tbl_hdl);
		CHECK_ERR_TBL_STOP(ret, tbl_hdl);
	}

	ret = ipa_nati_clear_ipv4_tbl(tbl_hdl);
	CHECK_ERR_TBL_STOP(ret, tbl_hdl);

	ret = ipa_nati_ipv4_tbl_stats(tbl_hdl, &nstats, &istats);
	CHECK_ERR_TBL_STOP(ret, tbl_hdl);

	IPAINFO("Attempting rule adds to %s table of size: (%u)\n",
			ipa3_nat_mem_in_as_str(nstats.nmi),
			nstats.tot_ents);

	last_nstats = nstats;
	last_istats = istats;

	memset(rule_hdls, 0, sizeof(rule_hdls));

	GET_MAX(rand_adds_max, rand_dels_max);

	tot = tot_added = tot_deleted = 0;

	for ( i = 0; i < array_sz(rule_hdls); i++ )
	{
		IPADBG("Trying %u ipa_nat_add_ipv4_rule()\n", i);

		memset(&ipv4_rule, 0, sizeof(ipv4_rule));

		ipv4_rule.protocol     = IPPROTO_TCP;
		ipv4_rule.public_port  = RAN_PORT;
		ipv4_rule.target_ip    = RAN_ADDR;
		ipv4_rule.target_port  = RAN_PORT;
		ipv4_rule.private_ip   = RAN_ADDR;
		ipv4_rule.private_port = RAN_PORT;

		ret = ipa_nat_add_ipv4_rule(tbl_hdl, &ipv4_rule, &rule_hdls[i]);
		CHECK_ERR_TBL_ACTION(ret, tbl_hdl, break);

		IPADBG("Success %u ipa_nat_add_ipv4_rule() -> rule_hdl(0x%08X)\n",
			   i, rule_hdls[i]);

		ret = ipa_nati_ipv4_tbl_stats(tbl_hdl, &nstats, &istats);
		CHECK_ERR_TBL_ACTION(ret, tbl_hdl, break);

		/*
		 * Are we in hybrid mode and have we switched memory type?
		 * Check for it and print the appropriate stats.
		 */
		if ( nstats.nmi != last_nstats.nmi )
		{
			mem_type = ipa3_nat_mem_in_as_str(last_nstats.nmi);

			switched = true;

			/*
			 * NAT table stats...
			 */
			IPAINFO("Able to add (%u) records to %s "
					"NAT table of size (%u) or (%f) percent\n",
					tot,
					mem_type,
					last_nstats.tot_ents,
					((float) tot / (float) last_nstats.tot_ents) * 100.0);

			IPAINFO("Able to add (%u) records to %s "
					"NAT BASE table of size (%u) or (%f) percent\n",
					last_nstats.tot_base_ents_filled,
					mem_type,
					last_nstats.tot_base_ents,
					((float) last_nstats.tot_base_ents_filled /
					 (float) last_nstats.tot_base_ents) * 100.0);

			IPAINFO("Able to add (%u) records to %s "
					"NAT EXPN table of size (%u) or (%f) percent\n",
					last_nstats.tot_expn_ents_filled,
					mem_type,
					last_nstats.tot_expn_ents,
					((float) last_nstats.tot_expn_ents_filled /
					 (float) last_nstats.tot_expn_ents) * 100.0);

			IPAINFO("%s NAT table chains: tot_chains(%u) min_len(%u) max_len(%u) avg_len(%f)\n",
					mem_type,
					last_nstats.tot_chains,
					last_nstats.min_chain_len,
					last_nstats.max_chain_len,
					last_nstats.avg_chain_len);

			/*
			 * INDEX table stats...
			 */
			IPAINFO("Able to add (%u) records to %s "
					"IDX table of size (%u) or (%f) percent\n",
					tot,
					mem_type,
					last_istats.tot_ents,
					((float) tot / (float) last_istats.tot_ents) * 100.0);

			IPAINFO("Able to add (%u) records to %s "
					"IDX BASE table of size (%u) or (%f) percent\n",
					last_istats.tot_base_ents_filled,
					mem_type,
					last_istats.tot_base_ents,
					((float) last_istats.tot_base_ents_filled /
					 (float) last_istats.tot_base_ents) * 100.0);

			IPAINFO("Able to add (%u) records to %s "
					"IDX EXPN table of size (%u) or (%f) percent\n",
					last_istats.tot_expn_ents_filled,
					mem_type,
					last_istats.tot_expn_ents,
					((float) last_istats.tot_expn_ents_filled /
					 (float) last_istats.tot_expn_ents) * 100.0);

			IPAINFO("%s IDX table chains: tot_chains(%u) min_len(%u) max_len(%u) avg_len(%f)\n",
					mem_type,
					last_istats.tot_chains,
					last_istats.min_chain_len,
					last_istats.max_chain_len,
					last_istats.avg_chain_len);
		}

		last_nstats = nstats;
		last_istats = istats;

		tot++;

		if ( ++tot_added == rand_adds_max )
		{
			u32  j, k;
			u32* hdl_ptr[tot];

			for ( j = k = 0; j < array_sz(rule_hdls); j++ )
			{
				if ( VALID_RULE(rule_hdls[j]) )
				{
					hdl_ptr[k] = &(rule_hdls[j]);

					if ( ++k == tot )
					{
						break;
					}
				}
			}

			IPADBG("About to delete %u rules\n", rand_dels_max);

			while ( k )
			{
				while ( j = rand() % k, ! VALID_RULE(*(hdl_ptr[j])) );

				IPADBG("Trying ipa_nat_del_ipv4_rule(0x%08X)\n",
					   *(hdl_ptr[j]));

				ret = ipa_nat_del_ipv4_rule(tbl_hdl, *(hdl_ptr[j]));
				CHECK_ERR_TBL_STOP(ret, tbl_hdl);
				IPADBG("Success ipa_nat_del_ipv4_rule(0x%08X)\n", *(hdl_ptr[j]));

				*(hdl_ptr[j]) = 0xFFFFFFFF;

				--tot;

				if ( ++tot_deleted == rand_dels_max )
				{
					break;
				}
			}

			GET_MAX(rand_adds_max, rand_dels_max);

			tot_added = tot_deleted = 0;
		}

		if ( switched )
		{
			switched = false;

			IPAINFO("Continuing rule adds to %s table of size: (%u)\n",
					ipa3_nat_mem_in_as_str(nstats.nmi),
					nstats.tot_ents);
		}
	}

	ret = ipa_nati_ipv4_tbl_stats(tbl_hdl, &nstats, &istats);
	CHECK_ERR_TBL_STOP(ret, tbl_hdl);

	mem_type = ipa3_nat_mem_in_as_str(nstats.nmi);

	/*
	 * NAT table stats...
	 */
	IPAINFO("Able to add (%u) records to %s "
			"NAT table of size (%u) or (%f) percent\n",
			tot,
			mem_type,
			nstats.tot_ents,
			((float) tot / (float) nstats.tot_ents) * 100.0);

	IPAINFO("Able to add (%u) records to %s "
			"NAT BASE table of size (%u) or (%f) percent\n",
			nstats.tot_base_ents_filled,
			mem_type,
			nstats.tot_base_ents,
			((float) nstats.tot_base_ents_filled /
			 (float) nstats.tot_base_ents) * 100.0);

	IPAINFO("Able to add (%u) records to %s "
			"NAT EXPN table of size (%u) or (%f) percent\n",
			nstats.tot_expn_ents_filled,
			mem_type,
			nstats.tot_expn_ents,
			((float) nstats.tot_expn_ents_filled /
			 (float) nstats.tot_expn_ents) * 100.0);

	IPAINFO("%s NAT table chains: tot_chains(%u) min_len(%u) max_len(%u) avg_len(%f)\n",
			mem_type,
			nstats.tot_chains,
			nstats.min_chain_len,
			nstats.max_chain_len,
			nstats.avg_chain_len);

	/*
	 * INDEX table stats...
	 */
	IPAINFO("Able to add (%u) records to %s "
			"IDX table of size (%u) or (%f) percent\n",
			tot,
			mem_type,
			istats.tot_ents,
			((float) tot / (float) istats.tot_ents) * 100.0);

	IPAINFO("Able to add (%u) records to %s "
			"IDX BASE table of size (%u) or (%f) percent\n",
			istats.tot_base_ents_filled,
			mem_type,
			istats.tot_base_ents,
			((float) istats.tot_base_ents_filled /
			 (float) istats.tot_base_ents) * 100.0);

	IPAINFO("Able to add (%u) records to %s "
			"IDX EXPN table of size (%u) or (%f) percent\n",
			istats.tot_expn_ents_filled,
			mem_type,
			istats.tot_expn_ents,
			((float) istats.tot_expn_ents_filled /
			 (float) istats.tot_expn_ents) * 100.0);

	IPAINFO("%s IDX table chains: tot_chains(%u) min_len(%u) max_len(%u) avg_len(%f)\n",
			mem_type,
			istats.tot_chains,
			istats.min_chain_len,
			istats.max_chain_len,
			istats.avg_chain_len);

	IPAINFO("Deleting remaining rules\n");

	for ( i = 0; i < array_sz(rule_hdls); i++ )
	{
		if ( VALID_RULE(rule_hdls[i]) )
		{
			IPADBG("Trying ipa_nat_del_ipv4_rule(0x%08X)\n",
				   rule_hdls[i]);
			ret = ipa_nat_del_ipv4_rule(tbl_hdl, rule_hdls[i]);
			CHECK_ERR_TBL_STOP(ret, tbl_hdl);
			IPADBG("Success ipa_nat_del_ipv4_rule(%u)\n", rule_hdls[i]);
		}
	}

	if ( sep )
	{
		ret = ipa_nat_del_ipv4_tbl(tbl_hdl);
		*tbl_hdl_ptr = 0;
		CHECK_ERR(ret);
	}

	IPADBG("Out\n");

	return 0;
}
