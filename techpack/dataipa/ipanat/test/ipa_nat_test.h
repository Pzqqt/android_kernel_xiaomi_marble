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

/*
 * ===========================================================================
 *
 * INCLUDE FILES FOR MODULE
 *
 * ===========================================================================
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h> /* for proto definitions */

#include "ipa_nat_drv.h"
#include "ipa_nat_drvi.h"

#undef array_sz
#define array_sz(a) \
	( sizeof(a)/sizeof(a[0]) )

#define u32 uint32_t
#define u16 uint16_t
#define u8  uint8_t

#define RAN_ADDR rand_ip_addr()
#define RAN_PORT rand_ip_port()

static inline u32 rand_ip_addr()
{
	static char buf[64];

	snprintf(
		buf, sizeof(buf),
		"%u.%u.%u.%u",
		(rand() % 254) + 1,
		 rand() % 255,
		 rand() % 255,
		(rand() % 254) + 1);

	return (u32) inet_addr(buf);
}

static inline u16 rand_ip_port()
{
	return (u16) ((rand() % 60535) + 5000);
}

/*============ Preconditions to run NAT Test cases =========*/
#define IPA_NAT_TEST_PRE_COND_TE  20

#define CHECK_ERR(x)							\
	if ( x ) {									\
		IPAERR("Abrupt end of %s with "			\
			   "err: %d at line: %d\n",			\
			   __FUNCTION__, x, __LINE__);		\
		return -1;								\
	}

#define CHECK_ERR_TBL_STOP(x, th)									 \
	if ( th ) {														 \
		int _ter_ = ipa_nat_validate_ipv4_table(th);				 \
		if ( _ter_ ) {												 \
			if ( sep ) {											 \
				ipa_nat_del_ipv4_tbl(th);							 \
			}														 \
			IPAERR("Abrupt end of %s with "							 \
				   "err: %d at line: %d\n",							 \
				   __FUNCTION__, _ter_, __LINE__);					 \
			return -1;												 \
		}															 \
	}																 \
	if ( x ) {														 \
		if ( th ) {													 \
			ipa_nat_dump_ipv4_table(th);							 \
			if( sep ) {												 \
				ipa_nat_del_ipv4_tbl(th);							 \
			}														 \
		}															 \
		IPAERR("Abrupt end of %s with "								 \
			   "err: %d at line: %d\n",								 \
			   __FUNCTION__, x, __LINE__);							 \
		return -1;													 \
	}

#define CHECK_ERR_TBL_ACTION(x, th, action)							 \
	if ( th ) {														 \
		int _ter_ = ipa_nat_validate_ipv4_table(th);				 \
		if ( _ter_ ) {												 \
			IPAERR("ipa_nat_validate_ipv4_table() failed "			 \
				   "in: %s at line: %d\n",							 \
				   __FUNCTION__, __LINE__);							 \
			action;													 \
		}															 \
	}																 \
	if ( x ) {														 \
		if ( th ) {													 \
			ipa_nat_dump_ipv4_table(th);							 \
		}															 \
		IPAERR("error: %d in %s at line: %d\n",						 \
			   x, __FUNCTION__, __LINE__);							 \
		action;														 \
	}

typedef int (*NatTestFunc)(
	const char*, u32, int, u32, int, void*);

typedef struct
{
	const char* func_name;
	int         num_ents_trigger;
	int         test_hold_time_in_secs;
	NatTestFunc func;
} NatTests;

#undef NAT_TEST_ENTRY
#define NAT_TEST_ENTRY(f, n, ht) \
	{#f, (n), (ht), f}

#define NAT_DEBUG
int ipa_nat_validate_ipv4_table(u32);

int ipa_nat_testREG(const char*, u32, int, u32, int, void*);

int ipa_nat_test000(const char*, u32, int, u32, int, void*);
int ipa_nat_test001(const char*, u32, int, u32, int, void*);
int ipa_nat_test002(const char*, u32, int, u32, int, void*);
int ipa_nat_test003(const char*, u32, int, u32, int, void*);
int ipa_nat_test004(const char*, u32, int, u32, int, void*);
int ipa_nat_test005(const char*, u32, int, u32, int, void*);
int ipa_nat_test006(const char*, u32, int, u32, int, void*);
int ipa_nat_test007(const char*, u32, int, u32, int, void*);
int ipa_nat_test008(const char*, u32, int, u32, int, void*);
int ipa_nat_test009(const char*, u32, int, u32, int, void*);
int ipa_nat_test010(const char*, u32, int, u32, int, void*);
int ipa_nat_test011(const char*, u32, int, u32, int, void*);
int ipa_nat_test012(const char*, u32, int, u32, int, void*);
int ipa_nat_test013(const char*, u32, int, u32, int, void*);
int ipa_nat_test014(const char*, u32, int, u32, int, void*);
int ipa_nat_test015(const char*, u32, int, u32, int, void*);
int ipa_nat_test016(const char*, u32, int, u32, int, void*);
int ipa_nat_test017(const char*, u32, int, u32, int, void*);
int ipa_nat_test018(const char*, u32, int, u32, int, void*);
int ipa_nat_test019(const char*, u32, int, u32, int, void*);
int ipa_nat_test020(const char*, u32, int, u32, int, void*);
int ipa_nat_test021(const char*, u32, int, u32, int, void*);
int ipa_nat_test022(const char*, u32, int, u32, int, void*);
int ipa_nat_test023(const char*, u32, int, u32, int, void*);
int ipa_nat_test024(const char*, u32, int, u32, int, void*);
int ipa_nat_test025(const char*, u32, int, u32, int, void*);
int ipa_nat_test999(const char*, u32, int, u32, int, void*);
