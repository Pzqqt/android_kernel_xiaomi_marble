/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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
#include <errno.h>
#include <pthread.h>

#include "ipa_nat_drv.h"
#include "ipa_nat_drvi.h"

#include "ipa_nat_map.h"

#include "ipa_nat_statemach.h"

#undef PRCNT_OF
#define PRCNT_OF(v) \
	((.25) * (v))

#undef  CHOOSE_MEM_SUB
#define CHOOSE_MEM_SUB() \
	(nati_obj.curr_state == NATI_STATE_HYBRID) ? \
	SRAM_SUB : \
	DDR_SUB

#undef  CHOOSE_MAPS
#define CHOOSE_MAPS(o2n, n2o) \
	do { \
		uint32_t sub = CHOOSE_MEM_SUB(); \
		o2n = nati_obj.map_pairs[sub].orig2new_map; \
		n2o = nati_obj.map_pairs[sub].new2orig_map; \
	} while (0)

#undef  CHOOSE_CNTR
#define CHOOSE_CNTR() \
	&(nati_obj.tot_rules_in_table[CHOOSE_MEM_SUB()])

#undef  CHOOSE_SW_STATS
#define CHOOSE_SW_STATS() \
	&(nati_obj.sw_stats[CHOOSE_MEM_SUB()])

/*
 * BACKROUND INFORMATION
 *
 * As it relates to why this file exists...
 *
 * In the past, a NAT table API was presented to upper layer
 * applications.  Said API mananged low level details of NAT table
 * creation, manipulation, and destruction.  The API
 * managed/manipulated NAT tables that lived exclusively in DDR. DDR
 * based tables are fine, but lead to uneeded bus accesses to/from DDR
 * by the IPA while doing its NAT duties. These accesses cause NAT to
 * take longer than necessary.
 *
 * If the DDR bus accesses could be eliminated by storing the table in
 * the IPA's internal memory (ie. SRAM), the IPA's IP V4 NAT could be
 * sped up. This leads us to the following description of this file's
 * intent.
 *
 * The purpose and intent of this file is to hijack the API described
 * above, but in a way that allows the tables to live in both SRAM and
 * DDR.  The details of whether SRAM or DDR is being used is hidden
 * from the application.  More specifically, the API will allow the
 * following to occur completely tranparent to the application using
 * the API.
 *
 *   (1) NAT tables can live exclusively in DDR (traditional and
 *       historically like before)
 *
 *   (2) NAT tables can live simultaneously in SRAM and DDR.  SRAM
 *       initially being used by the IPA, but both being kept in sync.
 *       When SRAM becomes too full, a switch to DDR will occur.
 *
 *   (3) The same as (2) above, but after the switch to DDR occurs,
 *       we'll have the ability to switch back to SRAM if/when DDR
 *       table entry deletions take us to a small enough entry
 *       count. An entry count that when met, allows us to switch back
 *       using SRAM again.
 *
 * As above, all of these details will just magically happen unknown
 * to the application using the API.  The implementation is done via a
 * state machine.
 */

/*
 * The following will be used to keep state machine state for and
 * between API calls...
 */
static ipa_nati_obj nati_obj = {
	.prev_state          = NATI_STATE_NULL,
	.curr_state          = NATI_STATE_NULL,
	.hold_state          = false,
	.state_to_hold       = NATI_STATE_NULL,
	.ddr_tbl_hdl         = 0,
	.sram_tbl_hdl        = 0,
	.tot_slots_in_sram   = 0,
	.back_to_sram_thresh = 0,
	/*
	 * Remember:
	 *   tot_rules_in_table[0] for ddr, and
	 *   tot_rules_in_table[1] for sram
	 */
	.tot_rules_in_table  = { 0, 0 },
	/*
	 * Remember:
	 *   map_pairs[0] for ddr, and
	 *   map_pairs[1] for sram
	 */
	.map_pairs = { {MAP_NUM_00, MAP_NUM_01}, {MAP_NUM_02, MAP_NUM_03} },
	/*
	 * Remember:
	 *   sw_stats[0] for ddr, and
	 *   sw_stats[1] for sram
	 */
	.sw_stats = { {0, 0}, {0, 0} },
};

/*
 * The following needed to protect nati_obj above, as well as a number
 * of data stuctures within the file ipa_nat_drvi.c
 */
pthread_mutex_t nat_mutex;
static bool     nat_mutex_init = false;

static inline int mutex_init(void)
{
	static pthread_mutexattr_t nat_mutex_attr;

	int ret = 0;

	IPADBG("In\n");

	ret = pthread_mutexattr_init(&nat_mutex_attr);

	if ( ret != 0 )
	{
		IPAERR("pthread_mutexattr_init() failed: ret(%d)\n", ret );
		goto bail;
	}

	ret = pthread_mutexattr_settype(
		&nat_mutex_attr, PTHREAD_MUTEX_RECURSIVE);

	if ( ret != 0 )
	{
		IPAERR("pthread_mutexattr_settype() failed: ret(%d)\n",
			   ret );
		goto bail;
	}

	ret = pthread_mutex_init(&nat_mutex, &nat_mutex_attr);

	if ( ret != 0 )
	{
		IPAERR("pthread_mutex_init() failed: ret(%d)\n",
			   ret );
		goto bail;
	}

	nat_mutex_init = true;

bail:
	IPADBG("Out\n");

	return ret;
}

/*
 * Function for taking/locking the mutex...
 */
static int take_mutex()
{
	int ret;

	if ( nat_mutex_init )
	{
again:
		ret = pthread_mutex_lock(&nat_mutex);
	}
	else
	{
		ret = mutex_init();

		if ( ret == 0 )
		{
			goto again;
		}
	}

	if ( ret != 0 )
	{
		IPAERR("Unable to lock the %s nat mutex\n",
			   (nat_mutex_init) ? "initialized" : "uninitialized");
	}

	return ret;
}

/*
 * Function for giving/unlocking the mutex...
 */
static int give_mutex()
{
	int ret = (nat_mutex_init) ? pthread_mutex_unlock(&nat_mutex) : -1;

	if ( ret != 0 )
	{
		IPAERR("Unable to unlock the %s nat mutex\n",
			   (nat_mutex_init) ? "initialized" : "uninitialized");
	}

	return ret;
}

/*
 * ****************************************************************************
 *
 * HIJACKED API FUNCTIONS START HERE
 *
 * ****************************************************************************
 */
int ipa_nati_add_ipv4_tbl(
	uint32_t    public_ip_addr,
	const char* mem_type_ptr,
	uint16_t    number_of_entries,
	uint32_t*   tbl_hdl)
{
	arb_t* args[] = {
		(arb_t*) public_ip_addr,
		(arb_t*) number_of_entries,
		(arb_t*) tbl_hdl,
		(arb_t*) mem_type_ptr,
	};

	int ret;

	IPADBG("In\n");

	ret = ipa_nati_statemach(&nati_obj, NATI_TRIG_ADD_TABLE, args);

	if ( ret == 0 )
	{
		IPADBG("tbl_hdl val(0x%08X)\n", *tbl_hdl);
	}

	IPADBG("Out\n");

	return ret;
}

int ipa_nati_del_ipv4_table(
	uint32_t tbl_hdl)
{
	arb_t* args[] = {
		(arb_t*) tbl_hdl,
	};

	int ret;

	IPADBG("In\n");

	ret = ipa_nati_statemach(&nati_obj, NATI_TRIG_DEL_TABLE, args);

	IPADBG("Out\n");

	return ret;
}

int ipa_nati_clear_ipv4_tbl(
	uint32_t tbl_hdl )
{
	arb_t* args[] = {
		(arb_t*) tbl_hdl,
	};

	int ret;

	IPADBG("In\n");

	ret = ipa_nati_statemach(&nati_obj, NATI_TRIG_CLR_TABLE, args);

	IPADBG("Out\n");

	return ret;
}

int ipa_nati_walk_ipv4_tbl(
	uint32_t          tbl_hdl,
	WhichTbl2Use      which,
	ipa_table_walk_cb walk_cb,
	void*             arb_data_ptr )
{
	arb_t* args[] = {
		(arb_t*) tbl_hdl,
		(arb_t*) which,
		(arb_t*) walk_cb,
		(arb_t*) arb_data_ptr,
	};

	int ret;

	IPADBG("In\n");

	ret = ipa_nati_statemach(&nati_obj, NATI_TRIG_WLK_TABLE, args);

	IPADBG("Out\n");

	return ret;
}

int ipa_nati_ipv4_tbl_stats(
	uint32_t            tbl_hdl,
	ipa_nati_tbl_stats* nat_stats_ptr,
	ipa_nati_tbl_stats* idx_stats_ptr )
{
	arb_t* args[] = {
		(arb_t*) tbl_hdl,
		(arb_t*) nat_stats_ptr,
		(arb_t*) idx_stats_ptr,
	};

	int ret;

	IPADBG("In\n");

	ret = ipa_nati_statemach(&nati_obj, NATI_TRIG_TBL_STATS, args);

	IPADBG("Out\n");

	return ret;
}

int ipa_nati_add_ipv4_rule(
	uint32_t                 tbl_hdl,
	const ipa_nat_ipv4_rule* clnt_rule,
	uint32_t*                rule_hdl )
{
	arb_t* args[] = {
		(arb_t*) tbl_hdl,
		(arb_t*) clnt_rule,
		(arb_t*) rule_hdl,
	};

	int ret;

	IPADBG("In\n");

	ret = ipa_nati_statemach(&nati_obj, NATI_TRIG_ADD_RULE, args);

	if ( ret == 0 )
	{
		IPADBG("rule_hdl val(%u)\n", *rule_hdl);
	}

	IPADBG("Out\n");

	return ret;
}

int ipa_nati_del_ipv4_rule(
	uint32_t tbl_hdl,
	uint32_t rule_hdl )
{
	arb_t* args[] = {
		(arb_t*) tbl_hdl,
		(arb_t*) rule_hdl,
	};

	int ret;

	IPADBG("In\n");

	ret = ipa_nati_statemach(&nati_obj, NATI_TRIG_DEL_RULE, args);

	IPADBG("Out\n");

	return ret;
}

int ipa_nati_query_timestamp(
	uint32_t  tbl_hdl,
	uint32_t  rule_hdl,
	uint32_t* time_stamp)
{
	arb_t* args[] = {
		(arb_t*) tbl_hdl,
		(arb_t*) rule_hdl,
		(arb_t*) time_stamp,
	};

	int ret;

	IPADBG("In\n");

	ret = ipa_nati_statemach(&nati_obj, NATI_TRIG_GET_TSTAMP, args);

	if ( ret == 0 )
	{
		IPADBG("time_stamp val(0x%08X)\n", *time_stamp);
	}

	IPADBG("Out\n");

	return ret;
}

int ipa_nat_switch_to(
	enum ipa3_nat_mem_in nmi,
	bool                 hold_state )
{
	int ret = -1;

	IPADBG("In - current state %s\n",
		   ipa_nati_state_as_str(nati_obj.curr_state));

	if ( ! IPA_VALID_NAT_MEM_IN(nmi) )
	{
		IPAERR("Bad nmi(%s)\n", ipa3_nat_mem_in_as_str(nmi));

		ret = -1;

		goto bail;
	}

	ret = take_mutex();

	if ( ret != 0 )
	{
		goto bail;
	}

	/*
	 * Are we here before the state machine has been started?
	 */
	if ( IN_UNSTARTED_STATE() )
	{
		nati_obj.hold_state = hold_state;

		nati_obj.state_to_hold =
			(nmi == IPA_NAT_MEM_IN_DDR) ?
			NATI_STATE_DDR_ONLY         :
			NATI_STATE_SRAM_ONLY;

		IPADBG(
			"Initial state will be %s before table init and it %s be held\n",
			ipa_nati_state_as_str(nati_obj.state_to_hold),
			(hold_state) ? "will" : "will not");

		ret = 0;

		goto unlock;
	}

	/*
	 * Are we here after we've already started in hybrid state?
	 */
	if ( IN_HYBRID_STATE() )
	{
		ret = 0;

		if ( COMPATIBLE_NMI_4SWITCH(nmi) )
		{
			ret = ipa_nati_statemach(&nati_obj, NATI_TRIG_TBL_SWITCH, 0);
		}

		if ( ret == 0 )
		{
			nati_obj.hold_state = hold_state;

			if ( hold_state )
			{
				nati_obj.state_to_hold = GEN_HOLD_STATE();
			}

			IPADBG(
				"Current state is %s and it %s be held\n",
				ipa_nati_state_as_str(nati_obj.curr_state),
				(hold_state) ? "will" : "will not");
		}

		goto unlock;
	}

	/*
	 * We've gotten here because we're not in an unstarted state, nor
	 * are we in hybrid state. This means we're either in
	 * NATI_STATE_DDR_ONLY or NATI_STATE_SRAM_ONLY
	 *
	 * Let's see what's being attempted and if it's OK...
	 */
	if ( hold_state )
	{
		if ( COMPATIBLE_NMI_4SWITCH(nmi) )
		{
			/*
			 * If we've gotten here, it means that the requested nmi,
			 * the current state, and the hold are compatible...
			 */
			nati_obj.state_to_hold = GEN_HOLD_STATE();
			nati_obj.hold_state    = hold_state;

			IPADBG(
				"Requesting to hold memory type %s at "
				"current state %s will be done\n",
				ipa3_nat_mem_in_as_str(nmi),
				ipa_nati_state_as_str(nati_obj.curr_state));

			ret = 0;

			goto unlock;
		}
		else
		{
			/*
			 * The requested nmi, the current state, and the hold are
			 * not compatible...
			 */
			IPAERR(
				"Requesting to hold memory type %s and "
				"current state %s are incompatible\n",
				ipa3_nat_mem_in_as_str(nmi),
				ipa_nati_state_as_str(nati_obj.curr_state));

			ret = -1;

			goto unlock;
		}
	}

	/*
	 * If we've gotten here, it's because the holding of state is no
	 * longer desired...
	 */
	nati_obj.state_to_hold = NATI_STATE_NULL;
	nati_obj.hold_state    = hold_state;

	IPADBG("Holding of state is no longer desired\n");

	ret = 0;

unlock:
	ret = give_mutex();

bail:
	IPADBG("Out\n");

	return ret;
}

bool ipa_nat_is_sram_supported(void)
{
	return VALID_TBL_HDL(nati_obj.sram_tbl_hdl);
}

/******************************************************************************/
/*
 * FUNCTION: migrate_rule
 *
 * PARAMS:
 *
 *   table_ptr         (IN) The table being walked
 *
 *   tbl_rule_hdl      (IN) The nat rule's handle from the source table
 *
 *   record_ptr        (IN) The nat rule record from the source table
 *
 *   record_index      (IN) The record above's index in the table being walked
 *
 *   meta_record_ptr   (IN) If meta data in table, this will be it
 *
 *   meta_record_index (IN) The record above's index in the table being walked
 *
 *   arb_data_ptr      (IN) The destination table handle
 *
 * DESCRIPTION:
 *
 *   This routine is intended to copy records from a source table to a
 *   destination table.

 *   It is used in union with the ipa_nati_copy_ipv4_tbl() API call
 *   below.
 *
 *   It is compatible with the ipa_table_walk() API.
 *
 *   In the context of the ipa_nati_copy_ipv4_tbl(), the arguments
 *   passed in are as enumerated above.
 *
 * AN IMPORTANT NOTE ON RULE HANDLES WHEN IN MYBRID MODE
 *
 *   The rule_hdl is used to find a rule in the nat table.  It is, in
 *   effect, an index into the table.  The applcation above us retains
 *   it for future manipulation of the rule in the table.
 *
 *   In hybrid mode, a rule can and will move between SRAM and DDR.
 *   Because of this, its handle will change.  The application has
 *   only the original handle and doesn't know of the new handle.  A
 *   mapping, used in hybrid mode, will maintain a relationship
 *   between the original handle and the rule's current real handle...
 *
 *   To help you get a mindset of how this is done:
 *
 *     The original handle will map (point) to the new and new handle
 *     will map (point) back to original.
 *
 * NOTE WELL: There are two sets of maps.  One for each memory type...
 *
 * RETURNS:
 *
 *   Returns 0 on success, non-zero on failure
 */
static int migrate_rule(
	ipa_table*      table_ptr,
	uint32_t        tbl_rule_hdl,
	void*           record_ptr,
	uint16_t        record_index,
	void*           meta_record_ptr,
	uint16_t        meta_record_index,
	void*           arb_data_ptr )
{
	struct ipa_nat_rule* nat_rule_ptr = (struct ipa_nat_rule*) record_ptr;
	uint32_t             dst_tbl_hdl  = (uint32_t) arb_data_ptr;

	ipa_nat_ipv4_rule    v4_rule;

	uint32_t             orig_rule_hdl;
	uint32_t             new_rule_hdl;

	uint32_t             src_orig2new_map, src_new2orig_map;
	uint32_t             dst_orig2new_map, dst_new2orig_map;
	uint32_t*            cnt_ptr;

	const char*          mig_dir_ptr;

	char                 buf[1024];
	int                  ret;

	IPADBG("In\n");

	IPADBG("tbl_mem_type(%s) tbl_rule_hdl(%u) -> %s\n",
		   ipa3_nat_mem_in_as_str(table_ptr->nmi),
		   tbl_rule_hdl,
		   prep_nat_rule_4print(nat_rule_ptr, buf, sizeof(buf)));

	IPADBG("dst_tbl_hdl(0x%08X)\n", dst_tbl_hdl);

	/*
	 * What is the type of the source table?
	 */
	if ( table_ptr->nmi == IPA_NAT_MEM_IN_SRAM )
	{
		mig_dir_ptr = "SRAM -> DDR";

		src_orig2new_map = nati_obj.map_pairs[SRAM_SUB].orig2new_map;
		src_new2orig_map = nati_obj.map_pairs[SRAM_SUB].new2orig_map;

		dst_orig2new_map = nati_obj.map_pairs[DDR_SUB].orig2new_map;
		dst_new2orig_map = nati_obj.map_pairs[DDR_SUB].new2orig_map;

		cnt_ptr          = &(nati_obj.tot_rules_in_table[DDR_SUB]);
	}
	else
	{
		mig_dir_ptr = "DDR -> SRAM";

		src_orig2new_map = nati_obj.map_pairs[DDR_SUB].orig2new_map;
		src_new2orig_map = nati_obj.map_pairs[DDR_SUB].new2orig_map;

		dst_orig2new_map = nati_obj.map_pairs[SRAM_SUB].orig2new_map;
		dst_new2orig_map = nati_obj.map_pairs[SRAM_SUB].new2orig_map;

		cnt_ptr          = &(nati_obj.tot_rules_in_table[SRAM_SUB]);
	}

	if ( nat_rule_ptr->protocol == IPA_NAT_INVALID_PROTO_FIELD_VALUE_IN_RULE )
	{
		IPADBG("%s: Special \"first rule in list\" case. "
			   "Rule's enabled bit on, but protocol implies deleted\n",
			   mig_dir_ptr);
		ret = 0;
		goto bail;
	}

	ret = ipa_nat_map_find(src_new2orig_map, tbl_rule_hdl, &orig_rule_hdl);

	if ( ret != 0 )
	{
		IPAERR("%s: ipa_nat_map_find(src_new2orig_map) fail\n", mig_dir_ptr);
		goto bail;
	}

	memset(&v4_rule, 0, sizeof(v4_rule));

	v4_rule.private_ip   = nat_rule_ptr->private_ip;
	v4_rule.private_port = nat_rule_ptr->private_port;
	v4_rule.protocol     = nat_rule_ptr->protocol;
	v4_rule.public_port  = nat_rule_ptr->public_port;
	v4_rule.target_ip    = nat_rule_ptr->target_ip;
	v4_rule.target_port  = nat_rule_ptr->target_port;
	v4_rule.pdn_index    = nat_rule_ptr->pdn_index;
	v4_rule.redirect     = nat_rule_ptr->redirect;
	v4_rule.enable       = nat_rule_ptr->enable;
	v4_rule.time_stamp   = nat_rule_ptr->time_stamp;
	v4_rule.uc_activation_index = nat_rule_ptr->uc_activation_index;
	v4_rule.s = nat_rule_ptr->s;
	v4_rule.ucp = nat_rule_ptr->ucp;
	v4_rule.dst_only = nat_rule_ptr->dst_only;
	v4_rule.src_only = nat_rule_ptr->src_only;

	ret = ipa_NATI_add_ipv4_rule(dst_tbl_hdl, &v4_rule, &new_rule_hdl);

	if ( ret != 0 )
	{
		IPAERR("%s: ipa_NATI_add_ipv4_rule() fail\n", mig_dir_ptr);
		goto bail;
	}

	(*cnt_ptr)++;

	/*
	 * The following is needed to maintain the original handle and
	 * have it point to the new handle.
	 *
	 * Remember, original handle points to new and the new handle
	 * points back to original.
	 */
	ret = ipa_nat_map_add(dst_orig2new_map, orig_rule_hdl, new_rule_hdl);

	if ( ret != 0 )
	{
		IPAERR("%s: ipa_nat_map_add(dst_orig2new_map) fail\n", mig_dir_ptr);
		goto bail;
	}

	ret = ipa_nat_map_add(dst_new2orig_map, new_rule_hdl, orig_rule_hdl);

	if ( ret != 0 )
	{
		IPAERR("%s: ipa_nat_map_add(dst_new2orig_map) fail\n", mig_dir_ptr);
		goto bail;
	}

	IPADBG("orig_rule_hdl(0x%08X) new_rule_hdl(0x%08X)\n",
		   orig_rule_hdl, new_rule_hdl);

bail:
	IPADBG("Out\n");

	return ret;
}

/*
 * ****************************************************************************
 *
 * STATE MACHINE CODE BEGINS HERE
 *
 * ****************************************************************************
 */
static int _smUndef(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr ); /* forward declaration */

/******************************************************************************/
/*
 * FUNCTION: _smDelTbl
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the destruction of the DDR based NAT
 *   table.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smDelTbl(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t**  args = arb_data_ptr;

	uint32_t tbl_hdl = (uint32_t) args[0];

	int ret;

	IPADBG("In\n");

	IPADBG("tbl_hdl(0x%08X)\n", tbl_hdl);

	ret = ipa_NATI_del_ipv4_table(tbl_hdl);

	if ( ret == 0 && ! IN_HYBRID_STATE() )
	{
		/*
		 * The following will create the preferred "initial state" for
		 * restart...
		 */
		BACK2_UNSTARTED_STATE();
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smFirstTbl
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the creation of the very first NAT table(s)
 *   before any others have ever been created...
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smFirstTbl(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t**   args = arb_data_ptr;

	uint32_t    public_ip_addr    = (uint32_t)    args[0];
	uint16_t    number_of_entries = (uint16_t)    args[1];
	uint32_t*   tbl_hdl_ptr       = (uint32_t*)   args[2];
	const char* mem_type_ptr      = (const char*) args[3];

	int ret;

	IPADBG("In\n");

	/*
	 * This is the first time in here.  Let the ipacm's XML config (or
	 * state_to_hold) drive initial state...
	 */
	SET_NATIOBJ_STATE(
		nati_obj_ptr,
		(nati_obj_ptr->hold_state && nati_obj_ptr->state_to_hold) ?
		nati_obj_ptr->state_to_hold                               :
		mem_type_str_to_ipa_nati_state(mem_type_ptr));

	ret = ipa_nati_statemach(nati_obj_ptr, NATI_TRIG_ADD_TABLE, args);

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smAddDdrTbl
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the creation of a NAT table in DDR.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smAddDdrTbl(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t**   args = arb_data_ptr;

	uint32_t  public_ip_addr    = (uint32_t)  args[0];
	uint16_t  number_of_entries = (uint16_t)  args[1];
	uint32_t* tbl_hdl_ptr       = (uint32_t*) args[2];

	int ret;

	IPADBG("In\n");

	IPADBG("public_ip_addr(0x%08X) number_of_entries(%u) tbl_hdl_ptr(%p)\n",
		   public_ip_addr, number_of_entries, tbl_hdl_ptr);

	ret = ipa_NATI_add_ipv4_tbl(
		IPA_NAT_MEM_IN_DDR,
		public_ip_addr,
		number_of_entries,
		&nati_obj_ptr->ddr_tbl_hdl);

	if ( ret == 0 )
	{
		*tbl_hdl_ptr = nati_obj_ptr->ddr_tbl_hdl;

		IPADBG("DDR table creation successful: tbl_hdl(0x%08X)\n",
			   *tbl_hdl_ptr);
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smAddSramTbl
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the creation of a NAT table in SRAM.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smAddSramTbl(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t**   args = arb_data_ptr;

	uint32_t  public_ip_addr    = (uint32_t)  args[0];
	uint16_t  number_of_entries = (uint16_t)  args[1];
	uint32_t* tbl_hdl_ptr       = (uint32_t*) args[2];

	uint32_t  sram_size = 0;

	int ret;

	IPADBG("In\n");

	IPADBG("public_ip_addr(0x%08X) tbl_hdl_ptr(%p)\n",
		   public_ip_addr, tbl_hdl_ptr);

	ret = ipa_nati_get_sram_size(&sram_size);

	if ( ret == 0 )
	{
		ret = ipa_calc_num_sram_table_entries(
			sram_size,
			sizeof(struct ipa_nat_rule),
			sizeof(struct ipa_nat_indx_tbl_rule),
			&nati_obj_ptr->tot_slots_in_sram);

		if ( ret == 0 )
		{
			nati_obj_ptr->back_to_sram_thresh =
				PRCNT_OF(nati_obj_ptr->tot_slots_in_sram);

			IPADBG("sram_size(%u or 0x%x) tot_slots_in_sram(%u) back_to_sram_thresh(%u)\n",
				   sram_size,
				   sram_size,
				   nati_obj_ptr->tot_slots_in_sram,
				   nati_obj_ptr->back_to_sram_thresh);

			IPADBG("Voting clock on for sram table creation\n");

			if ( (ret = ipa_nat_vote_clock(IPA_APP_CLK_VOTE)) != 0 )
			{
				IPAERR("Voting clock on failed\n");
				goto done;
			}

			ret = ipa_NATI_add_ipv4_tbl(
				IPA_NAT_MEM_IN_SRAM,
				public_ip_addr,
				nati_obj_ptr->tot_slots_in_sram,
				&nati_obj_ptr->sram_tbl_hdl);

			if ( ipa_nat_vote_clock(IPA_APP_CLK_DEVOTE) != 0 )
			{
				IPAWARN("Voting clock off failed\n");
			}

			if ( ret == 0 )
			{
				*tbl_hdl_ptr = nati_obj_ptr->sram_tbl_hdl;

				IPADBG("SRAM table creation successful: tbl_hdl(0x%08X)\n",
					   *tbl_hdl_ptr);
			}
		}
	}

done:
	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smAddSramAndDdrTbl
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the creation of NAT tables in both DDR
 *   and in SRAM.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smAddSramAndDdrTbl(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t**   args = arb_data_ptr;

	uint32_t  public_ip_addr    = (uint32_t)  args[0];
	uint16_t  number_of_entries = (uint16_t)  args[1];
	uint32_t* tbl_hdl_ptr       = (uint32_t*) args[2];

	uint32_t tbl_hdl;

	int ret;

	IPADBG("In\n");

	nati_obj_ptr->tot_rules_in_table[SRAM_SUB] = 0;
	nati_obj_ptr->tot_rules_in_table[DDR_SUB]  = 0;

	ipa_nat_map_clear(nati_obj_ptr->map_pairs[SRAM_SUB].orig2new_map);
	ipa_nat_map_clear(nati_obj_ptr->map_pairs[SRAM_SUB].new2orig_map);
	ipa_nat_map_clear(nati_obj_ptr->map_pairs[DDR_SUB].orig2new_map);
	ipa_nat_map_clear(nati_obj_ptr->map_pairs[DDR_SUB].new2orig_map);

	ret = _smAddSramTbl(nati_obj_ptr, trigger, arb_data_ptr);

	if ( ret == 0 )
	{
		if ( nati_obj_ptr->tot_slots_in_sram >= number_of_entries )
		{
			/*
			 * The number of slots in SRAM can accommodate what was
			 * being requested for DDR, hence no need to use DDR and
			 * we will continue by using SRAM only...
			 */
			SET_NATIOBJ_STATE(nati_obj_ptr, NATI_STATE_SRAM_ONLY);
		}
		else
		{
			/*
			 * SRAM not big enough. Let's create secondary DDR based
			 * table...
			 */
			arb_t*   new_args[] = {
				(arb_t*) public_ip_addr,
				(arb_t*) number_of_entries,
				(arb_t*) &tbl_hdl,  /* to protect app's table handle above */
			};

			ret = _smAddDdrTbl(nati_obj_ptr, trigger, new_args);

			if ( ret == 0 )
			{
				/*
				 * The following will tell the IPA to change focus to
				 * SRAM...
				 */
				ret = ipa_nati_statemach(nati_obj_ptr, NATI_TRIG_GOTO_SRAM, 0);
			}
		}
	}
	else
	{
		/*
		 * SRAM table creation in HYBRID mode failed.  Can we fall
		 * back to DDR only?  We need to try and see what happens...
		 */
		ret = _smAddDdrTbl(nati_obj_ptr, trigger, arb_data_ptr);

		if ( ret == 0 )
		{
			SET_NATIOBJ_STATE(nati_obj_ptr, NATI_STATE_DDR_ONLY);
		}
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smDelSramAndDdrTbl
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the destruction of the SRAM, then DDR
 *   based NAT tables.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smDelSramAndDdrTbl(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	int ret;

	IPADBG("In\n");

	nati_obj_ptr->tot_rules_in_table[SRAM_SUB] = 0;
	nati_obj_ptr->tot_rules_in_table[DDR_SUB]  = 0;

	ipa_nat_map_clear(nati_obj_ptr->map_pairs[SRAM_SUB].orig2new_map);
	ipa_nat_map_clear(nati_obj_ptr->map_pairs[SRAM_SUB].new2orig_map);
	ipa_nat_map_clear(nati_obj_ptr->map_pairs[DDR_SUB].orig2new_map);
	ipa_nat_map_clear(nati_obj_ptr->map_pairs[DDR_SUB].new2orig_map);

	ret = _smDelTbl(nati_obj_ptr, trigger, arb_data_ptr);

	if ( ret == 0 )
	{
		arb_t* new_args[] = {
			(arb_t*) nati_obj_ptr->ddr_tbl_hdl,
		};

		ret = _smDelTbl(nati_obj_ptr, trigger, new_args);
	}

	if ( ret == 0 )
	{
		/*
		 * The following will create the preferred "initial state" for
		 * restart...
		 */
		BACK2_UNSTARTED_STATE();
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smClrTbl
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the clearing of a table.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smClrTbl(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t**  args = arb_data_ptr;

	uint32_t tbl_hdl = (uint32_t) args[0];

	enum ipa3_nat_mem_in nmi;
	uint32_t             unused_hdl, sub;

	int ret;

	IPADBG("In\n");

	IPADBG("tbl_hdl(0x%08X)\n", tbl_hdl);

	BREAK_TBL_HDL(tbl_hdl, nmi, unused_hdl);

	if ( ! IPA_VALID_NAT_MEM_IN(nmi) ) {
		IPAERR("Bad cache type\n");
		ret = -EINVAL;
		goto bail;
	}

	sub = (nmi == IPA_NAT_MEM_IN_SRAM) ? SRAM_SUB : DDR_SUB;

	nati_obj_ptr->tot_rules_in_table[sub] = 0;

	ipa_nat_map_clear(nati_obj.map_pairs[sub].orig2new_map);
	ipa_nat_map_clear(nati_obj.map_pairs[sub].new2orig_map);

	ret = ipa_NATI_clear_ipv4_tbl(tbl_hdl);

bail:
	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smClrTblHybrid
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the clearing of the appropriate hybrid
 *   table.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smClrTblHybrid(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t**  args = arb_data_ptr;

	uint32_t tbl_hdl = (uint32_t) args[0];

	arb_t*   new_args[] = {
		(arb_t*) (nati_obj_ptr->curr_state == NATI_STATE_HYBRID) ?
		         tbl_hdl :
		         nati_obj_ptr->ddr_tbl_hdl,
	};

	int ret;

	IPADBG("In\n");

	ret = _smClrTbl(nati_obj_ptr, trigger, new_args);

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smWalkTbl
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the walk of a table.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smWalkTbl(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t** args = arb_data_ptr;

	uint32_t          tbl_hdl = (uint32_t)          args[0];
	WhichTbl2Use      which   = (WhichTbl2Use)      args[1];
	ipa_table_walk_cb walk_cb = (ipa_table_walk_cb) args[2];
	arb_t*            wadp    = (arb_t*)            args[3];

	int ret;

	IPADBG("In\n");

	IPADBG("tbl_hdl(0x%08X)\n", tbl_hdl);

	ret = ipa_NATI_walk_ipv4_tbl(tbl_hdl, which, walk_cb, wadp);

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smWalkTblHybrid
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the walk of the appropriate hybrid
 *   table.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smWalkTblHybrid(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t** args = arb_data_ptr;

	uint32_t          tbl_hdl = (uint32_t)          args[0];
	WhichTbl2Use      which   = (WhichTbl2Use)      args[1];
	ipa_table_walk_cb walk_cb = (ipa_table_walk_cb) args[2];
	arb_t*            wadp    = (arb_t*)            args[3];

	arb_t* new_args[] = {
		(arb_t*) (nati_obj_ptr->curr_state == NATI_STATE_HYBRID) ?
		         tbl_hdl :
		         nati_obj_ptr->ddr_tbl_hdl,
		(arb_t*) which,
		(arb_t*) walk_cb,
		(arb_t*) wadp,
	};

	int ret;

	IPADBG("In\n");

	ret = _smWalkTbl(nati_obj_ptr, trigger, new_args);

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smStatTbl
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will get size/usage stats for a table.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smStatTbl(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t** args = arb_data_ptr;

	uint32_t            tbl_hdl       = (uint32_t)            args[0];
	ipa_nati_tbl_stats* nat_stats_ptr = (ipa_nati_tbl_stats*) args[1];
	ipa_nati_tbl_stats* idx_stats_ptr = (ipa_nati_tbl_stats*) args[2];

	int ret;

	IPADBG("In\n");

	IPADBG("tbl_hdl(0x%08X)\n", tbl_hdl);

	ret = ipa_NATI_ipv4_tbl_stats(tbl_hdl, nat_stats_ptr, idx_stats_ptr);

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smStatTblHybrid
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the retrieval of table size/usage stats
 *   for the appropriate hybrid table.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smStatTblHybrid(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t** args = arb_data_ptr;

	uint32_t            tbl_hdl       = (uint32_t)            args[0];
	ipa_nati_tbl_stats* nat_stats_ptr = (ipa_nati_tbl_stats*) args[1];
	ipa_nati_tbl_stats* idx_stats_ptr = (ipa_nati_tbl_stats*) args[2];

	arb_t* new_args[] = {
		(arb_t*) (nati_obj_ptr->curr_state == NATI_STATE_HYBRID) ?
		         tbl_hdl :
		         nati_obj_ptr->ddr_tbl_hdl,
		(arb_t*) nat_stats_ptr,
		(arb_t*) idx_stats_ptr,
	};

	int ret;

	IPADBG("In\n");

	ret = _smStatTbl(nati_obj_ptr, trigger, new_args);

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smAddRuleToTbl
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the addtion of a NAT rule into the DDR
 *   based table.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smAddRuleToTbl(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t** args = arb_data_ptr;

	uint32_t           tbl_hdl   = (uint32_t)           args[0];
	ipa_nat_ipv4_rule* clnt_rule = (ipa_nat_ipv4_rule*) args[1];
	uint32_t*          rule_hdl  = (uint32_t*)          args[2];

	char buf[1024];

	int ret;

	IPADBG("In\n");

	IPADBG("tbl_hdl(0x%08X) clnt_rule_ptr(%p) rule_hdl_ptr(%p) %s\n",
		   tbl_hdl, clnt_rule, rule_hdl,
		   prep_nat_ipv4_rule_4print(clnt_rule, buf, sizeof(buf)));

	clnt_rule->redirect = clnt_rule->enable = clnt_rule->time_stamp = 0;

	ret = ipa_NATI_add_ipv4_rule(tbl_hdl, clnt_rule, rule_hdl);

	if ( ret == 0 )
	{
		uint32_t* cnt_ptr = CHOOSE_CNTR();

		(*cnt_ptr)++;

		IPADBG("rule_hdl value(%u or 0x%08X)\n",
			   *rule_hdl, *rule_hdl);
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smDelRuleFromTbl
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the deletion of a NAT rule from the DDR
 *   based table.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smDelRuleFromTbl(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t**  args = arb_data_ptr;

	uint32_t tbl_hdl  = (uint32_t) args[0];
	uint32_t rule_hdl = (uint32_t) args[1];

	int ret;

	IPADBG("In\n");

	IPADBG("tbl_hdl(0x%08X) rule_hdl(%u)\n", tbl_hdl, rule_hdl);

	ret = ipa_NATI_del_ipv4_rule(tbl_hdl, rule_hdl);

	if ( ret == 0 )
	{
		uint32_t* cnt_ptr = CHOOSE_CNTR();

		(*cnt_ptr)--;
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smAddRuleHybrid
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the addition of a NAT rule into either
 *   the SRAM or DDR based table.
 *
 *   *** !!! HOWEVER *** REMEMBER !!! ***
 *
 *   We're here because we're in a HYBRID state...with the potential
 *   moving between SRAM and DDR.  THIS HAS IMLICATIONS AS IT RELATES
 *   TO RULE MAPPING.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smAddRuleHybrid(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t** args = arb_data_ptr;

	uint32_t           tbl_hdl   = (uint32_t)           args[0];
	ipa_nat_ipv4_rule* clnt_rule = (ipa_nat_ipv4_rule*) args[1];
	uint32_t*          rule_hdl  = (uint32_t*)          args[2];

	arb_t*             new_args[] = {
		(arb_t*) (nati_obj_ptr->curr_state == NATI_STATE_HYBRID) ?
		         tbl_hdl :
		         nati_obj_ptr->ddr_tbl_hdl,
		(arb_t*) clnt_rule,
		(arb_t*) rule_hdl,
	};

	uint32_t orig2new_map, new2orig_map;

	int ret;

	IPADBG("In\n");

	ret = _smAddRuleToTbl(nati_obj_ptr, trigger, new_args);

	if ( ret == 0 )
	{
		/*
		 * The rule_hdl is used to find a rule in the nat table.  It
		 * is, in effect, an index into the table.  The applcation
		 * above us retains it for future manipulation of the rule in
		 * the table.
		 *
		 * In hybrid mode, a rule can and will move between SRAM and
		 * DDR.  Because of this, its handle will change.  The
		 * application has only the original handle and doesn't know
		 * of the new handle.  A mapping, used in hybrid mode, will
		 * maintain a relationship between the original handle and the
		 * rule's current real handle...
		 *
		 * To help you get a mindset of how this is done:
		 *
		 *   The original handle will map (point) to the new and new
		 *   handle will map (point) back to original.
		 *
		 * NOTE WELL: There are two sets of maps.  One for each memory
		 *            type...
		 */
		CHOOSE_MAPS(orig2new_map, new2orig_map);

		ret = ipa_nat_map_add(orig2new_map, *rule_hdl, *rule_hdl);

		if ( ret == 0 )
		{
			ret = ipa_nat_map_add(new2orig_map, *rule_hdl, *rule_hdl);
		}
	}
	else
	{
		if ( nati_obj_ptr->curr_state == NATI_STATE_HYBRID
			 &&
			 ! nati_obj_ptr->hold_state )
		{
			/*
			 * In hybrid mode, we always start in SRAM...hence
			 * NATI_STATE_HYBRID implies SRAM.  The rule addition
			 * above did not work, meaning the SRAM table is full,
			 * hence let's jump to DDR...
			 *
			 * The following will focus us on DDR and cause the copy
			 * of data from SRAM to DDR.
			 */
			IPAINFO("Add of rule failed...attempting table switch\n");

			ret = ipa_nati_statemach(nati_obj_ptr, NATI_TRIG_TBL_SWITCH, 0);

			if ( ret == 0 )
			{
				SET_NATIOBJ_STATE(nati_obj_ptr, NATI_STATE_HYBRID_DDR);

				/*
				 * Now add the rule to DDR...
				 */
				ret = ipa_nati_statemach(nati_obj_ptr, trigger, arb_data_ptr);
			}
		}
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smDelRuleHybrid
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the deletion of a NAT rule from either
 *   the SRAM or DDR based table.
 *
 *   *** !!! HOWEVER *** REMEMBER !!! ***
 *
 *   We're here because we're in a HYBRID state...with the potential
 *   moving between SRAM and DDR.  THIS HAS IMLICATIONS AS IT RELATES
 *   TO RULE MAPPING.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smDelRuleHybrid(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t**  args = arb_data_ptr;

	uint32_t tbl_hdl       = (uint32_t) args[0];
	uint32_t orig_rule_hdl = (uint32_t) args[1];

	uint32_t new_rule_hdl;

	uint32_t orig2new_map,  new2orig_map;

	int      ret;

	IPADBG("In\n");

	CHOOSE_MAPS(orig2new_map, new2orig_map);

	/*
	 * The rule_hdl is used to find a rule in the nat table.  It is,
	 * in effect, an index into the table.  The applcation above us
	 * retains it for future manipulation of the rule in the table.
	 *
	 * In hybrid mode, a rule can and will move between SRAM and DDR.
	 * Because of this, its handle will change.  The application has
	 * only the original handle and doesn't know of the new handle.  A
	 * mapping, used in hybrid mode, will maintain a relationship
	 * between the original handle and the rule's current real
	 * handle...
	 *
	 * To help you get a mindset of how this is done:
	 *
	 *   The original handle will map (point) to the new and new
	 *   handle will map (point) back to original.
	 *
	 * NOTE WELL: There are two sets of maps.  One for each memory
	 *            type...
	 */
	ret = ipa_nat_map_del(orig2new_map, orig_rule_hdl, &new_rule_hdl);

	if ( ret == 0 )
	{
		arb_t* new_args[]  = {
			(arb_t*) (nati_obj_ptr->curr_state == NATI_STATE_HYBRID) ?
			        tbl_hdl :
			        nati_obj_ptr->ddr_tbl_hdl,
			(arb_t*) new_rule_hdl,
		};

		IPADBG("orig_rule_hdl(0x%08X) -> new_rule_hdl(0x%08X)\n",
			   orig_rule_hdl, new_rule_hdl);

		ipa_nat_map_del(new2orig_map, new_rule_hdl, NULL);

		ret = _smDelRuleFromTbl(nati_obj_ptr, trigger, new_args);

		if ( ret == 0 && nati_obj_ptr->curr_state == NATI_STATE_HYBRID_DDR )
		{
			/*
			 * We need to check when/if we can go back to SRAM.
			 *
			 * How/why can we go back?
			 *
			 *   Given enough deletions, and when we get to a user
			 *   defined threshold (ie. a percentage of what SRAM can
			 *   hold), we can pop back to using SRAM.
			 */
			uint32_t* cnt_ptr = CHOOSE_CNTR();

			if ( *cnt_ptr <= nati_obj_ptr->back_to_sram_thresh
				 &&
				 ! nati_obj_ptr->hold_state )
			{
				/*
				 * The following will focus us on SRAM and cause the copy
				 * of data from DDR to SRAM.
				 */
				IPAINFO("Switch back to SRAM threshold has been reached -> "
						"Total rules in DDR(%u) <= SRAM THRESH(%u)\n",
						*cnt_ptr,
						nati_obj_ptr->back_to_sram_thresh);

				ret = ipa_nati_statemach(nati_obj_ptr, NATI_TRIG_TBL_SWITCH, 0);

				if ( ret == 0 )
				{
					SET_NATIOBJ_STATE(nati_obj_ptr, NATI_STATE_HYBRID);
				}
				else
				{
					/*
					 * The following will force us stay in DDR for
					 * now, but the next delete will trigger the
					 * switch logic above to run again...perhaps it
					 * will work then.
					 */
					ret = 0;
				}
			}
		}
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smGoToDdr
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the IPA to use the DDR based NAT
 *   table...
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smGoToDdr(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	int ret;

	IPADBG("In\n");

	ret = ipa_NATI_post_ipv4_init_cmd(nati_obj_ptr->ddr_tbl_hdl);

	if ( ret == 0 )
	{
		SET_NATIOBJ_STATE(nati_obj_ptr, NATI_STATE_HYBRID_DDR);
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smGoToSram
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause the IPA to use the SRAM based NAT
 *   table...
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smGoToSram(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	int ret;

	IPADBG("In\n");

	ret = ipa_NATI_post_ipv4_init_cmd(nati_obj_ptr->sram_tbl_hdl);

	if ( ret == 0 )
	{
		SET_NATIOBJ_STATE(nati_obj_ptr, NATI_STATE_HYBRID);
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smSwitchFromDdrToSram
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause a copy of the DDR table to SRAM and then
 *   will make the IPA use the SRAM...
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smSwitchFromDdrToSram(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	nati_switch_stats* sw_stats_ptr = CHOOSE_SW_STATS();

	uint32_t*          cnt_ptr      = CHOOSE_CNTR();

	ipa_nati_tbl_stats nat_stats, idx_stats;

	const char*        mem_type;

	uint64_t           start, stop;

	int                stats_ret, ret;

	bool               collect_stats = (bool) arb_data_ptr;

	IPADBG("In\n");

	stats_ret = (collect_stats) ?
		ipa_NATI_ipv4_tbl_stats(
			nati_obj_ptr->ddr_tbl_hdl, &nat_stats, &idx_stats) :
		-1;

	currTimeAs(TimeAsNanSecs, &start);

	/*
	 * First, switch focus to SRAM...
	 */
	ret = ipa_nati_statemach(nati_obj_ptr, NATI_TRIG_GOTO_SRAM, 0);

	if ( ret == 0 )
	{
		/*
		 * Clear destination counter...
		 */
		nati_obj_ptr->tot_rules_in_table[SRAM_SUB] = 0;

		/*
		 * Clear destination SRAM maps...
		 */
		ipa_nat_map_clear(nati_obj.map_pairs[SRAM_SUB].orig2new_map);
		ipa_nat_map_clear(nati_obj.map_pairs[SRAM_SUB].new2orig_map);

		/*
		 * Now copy DDR's content to SRAM...
		 */
		ret = ipa_nati_copy_ipv4_tbl(
			nati_obj_ptr->ddr_tbl_hdl,
			nati_obj_ptr->sram_tbl_hdl,
			migrate_rule);

		currTimeAs(TimeAsNanSecs, &stop);

		if ( ret == 0 )
		{
			sw_stats_ptr->pass += 1;

			IPADBG("Transistion from DDR to SRAM took %f microseconds\n",
				   (float) (stop - start) / 1000.0);
		}
		else
		{
			sw_stats_ptr->fail += 1;
		}

		IPADBG("Transistion pass/fail counts (DDR to SRAM) PASS: %u FAIL: %u\n",
			   sw_stats_ptr->pass,
			   sw_stats_ptr->fail);

		if ( stats_ret == 0 )
		{
			mem_type = ipa3_nat_mem_in_as_str(nat_stats.nmi);

			/*
			 * NAT table stats...
			 */
			IPADBG("Able to add (%u) records to %s "
				   "NAT table of size (%u) or (%f) percent\n",
				   *cnt_ptr,
				   mem_type,
				   nat_stats.tot_ents,
				   ((float) *cnt_ptr / (float) nat_stats.tot_ents) * 100.0);

			IPADBG("Able to add (%u) records to %s "
				   "NAT BASE table of size (%u) or (%f) percent\n",
				   nat_stats.tot_base_ents_filled,
				   mem_type,
				   nat_stats.tot_base_ents,
				   ((float) nat_stats.tot_base_ents_filled /
					(float) nat_stats.tot_base_ents) * 100.0);

			IPADBG("Able to add (%u) records to %s "
				   "NAT EXPN table of size (%u) or (%f) percent\n",
				   nat_stats.tot_expn_ents_filled,
				   mem_type,
				   nat_stats.tot_expn_ents,
				   ((float) nat_stats.tot_expn_ents_filled /
					(float) nat_stats.tot_expn_ents) * 100.0);

			IPADBG("%s NAT table chains: tot_chains(%u) min_len(%u) max_len(%u) avg_len(%f)\n",
				   mem_type,
				   nat_stats.tot_chains,
				   nat_stats.min_chain_len,
				   nat_stats.max_chain_len,
				   nat_stats.avg_chain_len);

			/*
			 * INDEX table stats...
			 */
			IPADBG("Able to add (%u) records to %s "
				   "IDX table of size (%u) or (%f) percent\n",
				   *cnt_ptr,
				   mem_type,
				   idx_stats.tot_ents,
				   ((float) *cnt_ptr / (float) idx_stats.tot_ents) * 100.0);

			IPADBG("Able to add (%u) records to %s "
				   "IDX BASE table of size (%u) or (%f) percent\n",
				   idx_stats.tot_base_ents_filled,
				   mem_type,
				   idx_stats.tot_base_ents,
				   ((float) idx_stats.tot_base_ents_filled /
					(float) idx_stats.tot_base_ents) * 100.0);

			IPADBG("Able to add (%u) records to %s "
				   "IDX EXPN table of size (%u) or (%f) percent\n",
				   idx_stats.tot_expn_ents_filled,
				   mem_type,
				   idx_stats.tot_expn_ents,
				   ((float) idx_stats.tot_expn_ents_filled /
					(float) idx_stats.tot_expn_ents) * 100.0);

			IPADBG("%s IDX table chains: tot_chains(%u) min_len(%u) max_len(%u) avg_len(%f)\n",
				   mem_type,
				   idx_stats.tot_chains,
				   idx_stats.min_chain_len,
				   idx_stats.max_chain_len,
				   idx_stats.avg_chain_len);
		}
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smSwitchFromSramToDdr
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following will cause a copy of the SRAM table to DDR and then
 *   will make the IPA use the DDR...
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smSwitchFromSramToDdr(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	nati_switch_stats* sw_stats_ptr = CHOOSE_SW_STATS();

	uint32_t*          cnt_ptr      = CHOOSE_CNTR();

	ipa_nati_tbl_stats nat_stats, idx_stats;

	const char*        mem_type;

	uint64_t           start, stop;

	int                stats_ret, ret;

	bool               collect_stats = (bool) arb_data_ptr;

	IPADBG("In\n");

	stats_ret = (collect_stats) ?
		ipa_NATI_ipv4_tbl_stats(
			nati_obj_ptr->sram_tbl_hdl, &nat_stats, &idx_stats) :
		-1;

	currTimeAs(TimeAsNanSecs, &start);

	/*
	 * First, switch focus to DDR...
	 */
	ret = ipa_nati_statemach(nati_obj_ptr, NATI_TRIG_GOTO_DDR, 0);

	if ( ret == 0 )
	{
		/*
		 * Clear destination counter...
		 */
		nati_obj_ptr->tot_rules_in_table[DDR_SUB] = 0;

		/*
		 * Clear destination DDR maps...
		 */
		ipa_nat_map_clear(nati_obj.map_pairs[DDR_SUB].orig2new_map);
		ipa_nat_map_clear(nati_obj.map_pairs[DDR_SUB].new2orig_map);

		/*
		 * Now copy SRAM's content to DDR...
		 */
		ret = ipa_nati_copy_ipv4_tbl(
			nati_obj_ptr->sram_tbl_hdl,
			nati_obj_ptr->ddr_tbl_hdl,
			migrate_rule);

		currTimeAs(TimeAsNanSecs, &stop);

		if ( ret == 0 )
		{
			sw_stats_ptr->pass += 1;

			IPADBG("Transistion from SRAM to DDR took %f microseconds\n",
				   (float) (stop - start) / 1000.0);
		}
		else
		{
			sw_stats_ptr->fail += 1;
		}

		IPADBG("Transistion pass/fail counts (SRAM to DDR) PASS: %u FAIL: %u\n",
			   sw_stats_ptr->pass,
			   sw_stats_ptr->fail);

		if ( stats_ret == 0 )
		{
			mem_type = ipa3_nat_mem_in_as_str(nat_stats.nmi);

			/*
			 * NAT table stats...
			 */
			IPADBG("Able to add (%u) records to %s "
				   "NAT table of size (%u) or (%f) percent\n",
				   *cnt_ptr,
				   mem_type,
				   nat_stats.tot_ents,
				   ((float) *cnt_ptr / (float) nat_stats.tot_ents) * 100.0);

			IPADBG("Able to add (%u) records to %s "
				   "NAT BASE table of size (%u) or (%f) percent\n",
				   nat_stats.tot_base_ents_filled,
				   mem_type,
				   nat_stats.tot_base_ents,
				   ((float) nat_stats.tot_base_ents_filled /
					(float) nat_stats.tot_base_ents) * 100.0);

			IPADBG("Able to add (%u) records to %s "
				   "NAT EXPN table of size (%u) or (%f) percent\n",
				   nat_stats.tot_expn_ents_filled,
				   mem_type,
				   nat_stats.tot_expn_ents,
				   ((float) nat_stats.tot_expn_ents_filled /
					(float) nat_stats.tot_expn_ents) * 100.0);

			IPADBG("%s NAT table chains: tot_chains(%u) min_len(%u) max_len(%u) avg_len(%f)\n",
				   mem_type,
				   nat_stats.tot_chains,
				   nat_stats.min_chain_len,
				   nat_stats.max_chain_len,
				   nat_stats.avg_chain_len);

			/*
			 * INDEX table stats...
			 */
			IPADBG("Able to add (%u) records to %s "
				   "IDX table of size (%u) or (%f) percent\n",
				   *cnt_ptr,
				   mem_type,
				   idx_stats.tot_ents,
				   ((float) *cnt_ptr / (float) idx_stats.tot_ents) * 100.0);

			IPADBG("Able to add (%u) records to %s "
				   "IDX BASE table of size (%u) or (%f) percent\n",
				   idx_stats.tot_base_ents_filled,
				   mem_type,
				   idx_stats.tot_base_ents,
				   ((float) idx_stats.tot_base_ents_filled /
					(float) idx_stats.tot_base_ents) * 100.0);

			IPADBG("Able to add (%u) records to %s "
				   "IDX EXPN table of size (%u) or (%f) percent\n",
				   idx_stats.tot_expn_ents_filled,
				   mem_type,
				   idx_stats.tot_expn_ents,
				   ((float) idx_stats.tot_expn_ents_filled /
					(float) idx_stats.tot_expn_ents) * 100.0);

			IPADBG("%s IDX table chains: tot_chains(%u) min_len(%u) max_len(%u) avg_len(%f)\n",
				   mem_type,
				   idx_stats.tot_chains,
				   idx_stats.min_chain_len,
				   idx_stats.max_chain_len,
				   idx_stats.avg_chain_len);
		}
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smGetTmStmp
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   Retrieve rule's timestamp from NAT table.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smGetTmStmp(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t** args = arb_data_ptr;

	uint32_t  tbl_hdl    = (uint32_t)  args[0];
	uint32_t  rule_hdl   = (uint32_t)  args[1];
	uint32_t* time_stamp = (uint32_t*) args[2];

	int ret;

	IPADBG("In\n");

	IPADBG("tbl_hdl(0x%08X) rule_hdl(%u) time_stamp_ptr(%p)\n",
		   tbl_hdl, rule_hdl, time_stamp);

	ret = ipa_NATI_query_timestamp(tbl_hdl, rule_hdl, time_stamp);

	if ( ret == 0 )
	{
		IPADBG("time_stamp(0x%08X)\n", *time_stamp);
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * FUNCTION: _smGetTmStmpHybrid
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   Retrieve rule's timestamp from the state approriate NAT table.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smGetTmStmpHybrid(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	arb_t** args = arb_data_ptr;

	uint32_t  tbl_hdl       = (uint32_t)  args[0];
	uint32_t  orig_rule_hdl = (uint32_t)  args[1];
	uint32_t* time_stamp    = (uint32_t*) args[2];

	uint32_t  new_rule_hdl;

	uint32_t  orig2new_map, new2orig_map;

	int       ret;

	IPADBG("In\n");

	CHOOSE_MAPS(orig2new_map, new2orig_map);

	ret = ipa_nat_map_find(orig2new_map, orig_rule_hdl, &new_rule_hdl);

	if ( ret == 0 )
	{
		arb_t* new_args[] = {
			(arb_t*) (nati_obj_ptr->curr_state == NATI_STATE_HYBRID) ?
			         tbl_hdl :
			         nati_obj_ptr->ddr_tbl_hdl,
			(arb_t*) new_rule_hdl,
			(arb_t*) time_stamp,
		};

		ret = _smGetTmStmp(nati_obj_ptr, trigger, new_args);
	}

	IPADBG("Out\n");

	return ret;
}

/******************************************************************************/
/*
 * The following table relates a nati object's state and a transition
 * trigger to a callback...
 */
static nati_statemach_tuple
_state_mach_tbl[NATI_STATE_LAST+1][NATI_TRIG_LAST+1] =
{
	{
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_NULL,       _smUndef ),
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_ADD_TABLE,  _smFirstTbl ),
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_DEL_TABLE,  _smUndef ),
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_CLR_TABLE,  _smUndef ),
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_WLK_TABLE,  _smUndef ),
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_TBL_STATS,  _smUndef ),
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_ADD_RULE,   _smUndef ),
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_DEL_RULE,   _smUndef ),
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_TBL_SWITCH, _smUndef ),
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_GOTO_DDR,   _smUndef ),
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_GOTO_SRAM,  _smUndef ),
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_GET_TSTAMP, _smUndef ),
		SM_ROW( NATI_STATE_NULL,       NATI_TRIG_LAST,       _smUndef ),
	},

	{
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_NULL,       _smUndef ),
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_ADD_TABLE,  _smAddDdrTbl ),
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_DEL_TABLE,  _smDelTbl ),
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_CLR_TABLE,  _smClrTbl ),
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_WLK_TABLE,  _smWalkTbl ),
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_TBL_STATS,  _smStatTbl ),
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_ADD_RULE,   _smAddRuleToTbl ),
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_DEL_RULE,   _smDelRuleFromTbl ),
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_TBL_SWITCH, _smUndef ),
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_GOTO_DDR,   _smUndef ),
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_GOTO_SRAM,  _smUndef ),
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_GET_TSTAMP, _smGetTmStmp ),
		SM_ROW( NATI_STATE_DDR_ONLY,   NATI_TRIG_LAST,       _smUndef ),
	},

	{
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_NULL,       _smUndef ),
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_ADD_TABLE,  _smAddSramTbl ),
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_DEL_TABLE,  _smDelTbl ),
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_CLR_TABLE,  _smClrTbl ),
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_WLK_TABLE,  _smWalkTbl ),
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_TBL_STATS,  _smStatTbl ),
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_ADD_RULE,   _smAddRuleToTbl ),
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_DEL_RULE,   _smDelRuleFromTbl ),
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_TBL_SWITCH, _smUndef ),
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_GOTO_DDR,   _smUndef ),
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_GOTO_SRAM,  _smUndef ),
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_GET_TSTAMP, _smGetTmStmp ),
		SM_ROW( NATI_STATE_SRAM_ONLY,  NATI_TRIG_LAST,       _smUndef ),
	},

	{
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_NULL,       _smUndef ),
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_ADD_TABLE,  _smAddSramAndDdrTbl ),
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_DEL_TABLE,  _smDelSramAndDdrTbl ),
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_CLR_TABLE,  _smClrTblHybrid ),
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_WLK_TABLE,  _smWalkTblHybrid ),
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_TBL_STATS,  _smStatTblHybrid ),
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_ADD_RULE,   _smAddRuleHybrid ),
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_DEL_RULE,   _smDelRuleHybrid ),
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_TBL_SWITCH, _smSwitchFromSramToDdr ),
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_GOTO_DDR,   _smGoToDdr ),
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_GOTO_SRAM,  _smGoToSram ),
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_GET_TSTAMP, _smGetTmStmpHybrid ),
		SM_ROW( NATI_STATE_HYBRID,     NATI_TRIG_LAST,       _smUndef ),
	},

	{
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_NULL,       _smUndef ),
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_ADD_TABLE,  _smUndef ),
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_DEL_TABLE,  _smDelSramAndDdrTbl ),
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_CLR_TABLE,  _smClrTblHybrid ),
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_WLK_TABLE,  _smWalkTblHybrid ),
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_TBL_STATS,  _smStatTblHybrid ),
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_ADD_RULE,   _smAddRuleHybrid ),
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_DEL_RULE,   _smDelRuleHybrid ),
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_TBL_SWITCH, _smSwitchFromDdrToSram ),
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_GOTO_DDR,   _smGoToDdr ),
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_GOTO_SRAM,  _smGoToSram ),
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_GET_TSTAMP, _smGetTmStmpHybrid ),
		SM_ROW( NATI_STATE_HYBRID_DDR, NATI_TRIG_LAST,       _smUndef ),
	},

	{
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_NULL,       _smUndef ),
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_ADD_TABLE,  _smUndef ),
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_DEL_TABLE,  _smUndef ),
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_CLR_TABLE,  _smUndef ),
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_WLK_TABLE,  _smUndef ),
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_TBL_STATS,  _smUndef ),
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_ADD_RULE,   _smUndef ),
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_DEL_RULE,   _smUndef ),
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_TBL_SWITCH, _smUndef ),
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_GOTO_DDR,   _smUndef ),
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_GOTO_SRAM,  _smUndef ),
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_GET_TSTAMP, _smUndef ),
		SM_ROW( NATI_STATE_LAST,       NATI_TRIG_LAST,       _smUndef ),
	},
};

/******************************************************************************/
/*
 * FUNCTION: _smUndef
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Whatever you like
 *
 * DESCRIPTION:
 *
 *   The following does nothing, except report an undefined action for
 *   a particular state/trigger combo...
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
static int _smUndef(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	IPAERR("CB(%s): undefined action for STATE(%s) with TRIGGER(%s)\n",
		   _state_mach_tbl[nati_obj_ptr->curr_state][trigger].sm_cb_as_str,
		   _state_mach_tbl[nati_obj_ptr->curr_state][trigger].state_as_str,
		   _state_mach_tbl[nati_obj_ptr->curr_state][trigger].trigger_as_str);

	return -1;
}

/******************************************************************************/
/*
 * FUNCTION: ipa_nati_statemach
 *
 * PARAMS:
 *
 *   nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   trigger      (IN) The trigger to run through the state machine
 *
 *   arb_data_ptr (IN) Anything you like.  Will be passed, untouched,
 *                     to the state/trigger callback function.
 *
 * DESCRIPTION:
 *
 *   This function allows a nati object and a trigger to be run
 *   through the state machine.
 *
 * RETURNS:
 *
 *   zero on success, otherwise non-zero
 */
int ipa_nati_statemach(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr )
{
	const char* ss_ptr  = _state_mach_tbl[nati_obj_ptr->curr_state][trigger].state_as_str;
	const char* ts_ptr  = _state_mach_tbl[nati_obj_ptr->curr_state][trigger].trigger_as_str;
	const char* cbs_ptr = _state_mach_tbl[nati_obj_ptr->curr_state][trigger].sm_cb_as_str;

	bool vote = false;

	int ret;

	IPADBG("In\n");

	ret = take_mutex();

	if ( ret != 0 )
	{
		goto bail;
	}

	IPADBG("STATE(%s) TRIGGER(%s) CB(%s)\n", ss_ptr, ts_ptr, cbs_ptr);

	vote = VOTE_REQUIRED(trigger);

	if ( vote )
	{
		IPADBG("Voting clock on STATE(%s) TRIGGER(%s)\n",
			   ss_ptr, ts_ptr);

		if ( ipa_nat_vote_clock(IPA_APP_CLK_VOTE) != 0 )
		{
			IPAERR("Voting failed STATE(%s) TRIGGER(%s)\n", ss_ptr, ts_ptr);
			ret = -EINVAL;
			goto unlock;
		}
	}

	ret = _state_mach_tbl[nati_obj_ptr->curr_state][trigger].sm_cb(
		nati_obj_ptr, trigger, arb_data_ptr);

	if ( vote )
	{
		IPADBG("Voting clock off STATE(%s) TRIGGER(%s)\n",
			   ss_ptr, ts_ptr);

		if ( ipa_nat_vote_clock(IPA_APP_CLK_DEVOTE) != 0 )
		{
			IPAERR("Voting failed STATE(%s) TRIGGER(%s)\n", ss_ptr, ts_ptr);
		}
	}

unlock:
	ret = give_mutex();

bail:
	IPADBG("Out\n");

	return ret;
}
