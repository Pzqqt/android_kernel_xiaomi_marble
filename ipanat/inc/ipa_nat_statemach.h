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
#if !defined(_IPA_NAT_STATEMACH_H_)
# define _IPA_NAT_STATEMACH_H_

typedef uintptr_t arb_t;

#define MAKE_AS_STR_CASE(v) case v: return #v

/******************************************************************************/
/**
 * The following enum represents the states that a nati object can be
 * in.
 */
typedef enum {
	NATI_STATE_NULL       = 0,
	NATI_STATE_DDR_ONLY   = 1, /* NAT in DDR only (traditional) */
	NATI_STATE_SRAM_ONLY  = 2, /* NAT in SRAM only (new) */
	NATI_STATE_HYBRID     = 3, /* NAT simultaneously in both SRAM/DDR */
	NATI_STATE_HYBRID_DDR = 4, /* NAT transitioned from SRAM to DDR */

	NATI_STATE_LAST
} ipa_nati_state;

/* KEEP THE FOLLOWING IN SYNC WITH ABOVE. */
static inline const char* ipa_nati_state_as_str(
	ipa_nati_state s )
{
	switch ( s )
	{
		MAKE_AS_STR_CASE(NATI_STATE_NULL);
		MAKE_AS_STR_CASE(NATI_STATE_DDR_ONLY);
		MAKE_AS_STR_CASE(NATI_STATE_SRAM_ONLY);
		MAKE_AS_STR_CASE(NATI_STATE_HYBRID);
		MAKE_AS_STR_CASE(NATI_STATE_HYBRID_DDR);
		MAKE_AS_STR_CASE(NATI_STATE_LAST);

	default:
		break;
	}

	return "???";
}

# undef strcasesame
# define strcasesame(a, b) (!strcasecmp(a, b))

static inline ipa_nati_state mem_type_str_to_ipa_nati_state(
	const char* str )
{
	if ( str ) {
		if (strcasesame(str, "HYBRID" ))
			return NATI_STATE_HYBRID;
		if (strcasesame(str, "SRAM" ))
			return NATI_STATE_SRAM_ONLY;
	}
	return NATI_STATE_DDR_ONLY;
}

/******************************************************************************/
/**
 * The following enum represents the API triggers that may or may not
 * cause a nati object to transition through its various allowable
 * states defined in ipa_nati_state above.
 */
typedef enum {
	NATI_TRIG_NULL       =  0,
	NATI_TRIG_ADD_TABLE  =  1,
	NATI_TRIG_DEL_TABLE  =  2,
	NATI_TRIG_CLR_TABLE  =  3,
	NATI_TRIG_WLK_TABLE  =  4,
	NATI_TRIG_TBL_STATS  =  5,
	NATI_TRIG_ADD_RULE   =  6,
	NATI_TRIG_DEL_RULE   =  7,
	NATI_TRIG_TBL_SWITCH =  8,
	NATI_TRIG_GOTO_DDR   =  9,
	NATI_TRIG_GOTO_SRAM  = 10,
	NATI_TRIG_GET_TSTAMP = 11,

	NATI_TRIG_LAST
} ipa_nati_trigger;

/******************************************************************************/
/**
 * The following structure used to keep switch stats.
 */
typedef struct
{
	uint32_t pass;
	uint32_t fail;
} nati_switch_stats;

/******************************************************************************/
/**
 * The following structure used to direct map usage.
 *
 * Maps are needed to map rule handles..orig to new and new to orig.
 * See comments in ipa_nat_statemach.c on this topic...
 */
typedef struct
{
	uint32_t orig2new_map;
	uint32_t new2orig_map;
} nati_map_pair;

/******************************************************************************/
/**
 * The following is a nati object that will maintain state relative to
 * various API calls.
 */
typedef struct
{
	ipa_nati_state prev_state;
	ipa_nati_state curr_state;
	bool           hold_state;
	ipa_nati_state state_to_hold;
	uint32_t       ddr_tbl_hdl;
	uint32_t       sram_tbl_hdl;
	uint32_t       tot_slots_in_sram;
	uint32_t       back_to_sram_thresh;
	/*
	 * tot_rules_in_table[0] for ddr, and
	 * tot_rules_in_table[1] for sram
	 */
	uint32_t       tot_rules_in_table[2];
	/*
	 * map_pairs[0] for ddr, and
	 * map_pairs[1] for sram
	 */
	nati_map_pair  map_pairs[2];
	/*
	 * sw_stats[0] for ddr, and
	 * sw_stats[1] for sram
	 */
	nati_switch_stats sw_stats[2];
} ipa_nati_obj;

/*
 * For use with the arrays above..in ipa_nati_obj...
 */
#undef DDR_SUB
#undef SRAM_SUB

#define DDR_SUB  0
#define SRAM_SUB 1

#undef BACK2_UNSTARTED_STATE
#define BACK2_UNSTARTED_STATE() \
	nati_obj.prev_state = nati_obj.curr_state = NATI_STATE_NULL;

#undef IN_UNSTARTED_STATE
#define IN_UNSTARTED_STATE() \
	( nati_obj.prev_state == NATI_STATE_NULL )

#undef IN_HYBRID_STATE
#define IN_HYBRID_STATE() \
	( nati_obj.curr_state == NATI_STATE_HYBRID || \
	  nati_obj.curr_state == NATI_STATE_HYBRID_DDR )

#undef COMPATIBLE_NMI_4SWITCH
#define COMPATIBLE_NMI_4SWITCH(n) \
	( (n) == IPA_NAT_MEM_IN_SRAM && nati_obj.curr_state == NATI_STATE_HYBRID_DDR ) || \
	( (n) == IPA_NAT_MEM_IN_DDR  && nati_obj.curr_state == NATI_STATE_HYBRID ) || \
	( (n) == IPA_NAT_MEM_IN_DDR  && nati_obj.curr_state == NATI_STATE_DDR_ONLY ) || \
	( (n) == IPA_NAT_MEM_IN_SRAM && nati_obj.curr_state == NATI_STATE_SRAM_ONLY )

#undef GEN_HOLD_STATE
#define GEN_HOLD_STATE() \
	( ! IN_HYBRID_STATE() ) ? nati_obj.curr_state : \
	(nati_obj.curr_state == NATI_STATE_HYBRID) ? NATI_STATE_SRAM_ONLY : \
	NATI_STATE_DDR_ONLY

#undef  SRAM_CURRENTLY_ACTIVE
#define SRAM_CURRENTLY_ACTIVE() \
	( nati_obj.curr_state == NATI_STATE_SRAM_ONLY || \
	  nati_obj.curr_state == NATI_STATE_HYBRID )

#define SRAM_TO_BE_ACCESSED(t) \
	( SRAM_CURRENTLY_ACTIVE() || \
	  (t) == NATI_TRIG_GOTO_SRAM || \
	  (t) == NATI_TRIG_TBL_SWITCH )

/*
 * NOTE: The exclusion of timestamp retrieval and table creation
 *       below.
 *
 * Why?
 *
 *  In re timestamp:
 *
 *   Because timestamp retrieval institutes too many repetitive
 *   accesses, hence would lead to too many successive votes. Instead,
 *   it will be handled differently and in the app layer above.
 *
 *  In re table creation:
 *
 *    Because it can't be known, apriori, whether or not sram is
 *    really available for use. Instead, we'll move table creation
 *    voting to a place where we know sram is available.
 */
#undef  VOTE_REQUIRED
#define VOTE_REQUIRED(t) \
	( SRAM_TO_BE_ACCESSED(t) && \
	  (t) != NATI_TRIG_GET_TSTAMP && \
	  (t) != NATI_TRIG_ADD_TABLE )

/******************************************************************************/
/**
 * A helper macro for changing a nati object's state...
 */
# undef SET_NATIOBJ_STATE
# define SET_NATIOBJ_STATE(x, s)  {        \
		(x)->prev_state = (x)->curr_state; \
		(x)->curr_state = s;               \
	}

/******************************************************************************/
/**
 * A function signature for a state/trigger callback function...
 */
typedef int (*nati_statemach_cb)(
	ipa_nati_obj*    nati_obj_ptr,
	ipa_nati_trigger trigger,
	arb_t*           arb_data_ptr );

/******************************************************************************/
/**
 * A structure for relating state to trigger callbacks.
 */
typedef struct
{
	ipa_nati_state    state;
	ipa_nati_trigger  trigger;
	nati_statemach_cb sm_cb;
	const char*       state_as_str;
	const char*       trigger_as_str;
	const char*       sm_cb_as_str;
} nati_statemach_tuple;

#undef SM_ROW
#define SM_ROW(s, t, f) \
	{ s, t, f, #s, #t, #f }

/******************************************************************************/
/**
 * FUNCTION: ipa_nati_statemach
 *
 * PARAMS:
 *
 *   @nati_obj_ptr (IN) A pointer to an initialized nati object
 *
 *   @trigger      (IN) The trigger to run through the state machine
 *
 *   @arb_data_ptr (IN) Anything you like.  Will be passed, untouched,
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
	arb_t*           arb_data_ptr );

#endif /* #if !defined(_IPA_NAT_STATEMACH_H_) */
