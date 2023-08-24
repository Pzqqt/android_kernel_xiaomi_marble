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
	ipa_nat_test024.c

	@brief
	Verify the following scenario:
	1. Trigger thousands of table memory switches

*/
/*===========================================================================*/

#include "ipa_nat_test.h"

int ipa_nat_test024(
	const char* nat_mem_type,
	u32 pub_ip_add,
	int total_entries,
	u32 tbl_hdl,
	int sep,
	void* arb_data_ptr)
{
	int* tbl_hdl_ptr = (int*) arb_data_ptr;

	int i, ret;

	IPADBG("In\n");

	if ( sep )
	{
		ret = ipa_nat_add_ipv4_tbl(pub_ip_add, nat_mem_type, total_entries, &tbl_hdl);
		CHECK_ERR_TBL_STOP(ret, tbl_hdl);
	}

	for ( i = 0; i < 1000; i++ )
	{
		ret = ipa_nat_test022(
			nat_mem_type, pub_ip_add, total_entries, tbl_hdl, !sep, arb_data_ptr);
		CHECK_ERR_TBL_STOP(ret, tbl_hdl);
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
