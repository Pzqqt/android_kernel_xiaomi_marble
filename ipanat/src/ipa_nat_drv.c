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

#include <errno.h>

/**
 * ipa_nat_add_ipv4_tbl() - create ipv4 nat table
 * @public_ip_addr: [in] public ipv4 address
 * @mem_type_ptr: [in] type of memory table is to reside in
 * @number_of_entries: [in]  number of nat entries
 * @table_handle: [out] Handle of new ipv4 nat table
 *
 * To create new ipv4 nat table
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_nat_add_ipv4_tbl(
	uint32_t public_ip_addr,
	const char *mem_type_ptr,
	uint16_t number_of_entries,
	uint32_t *tbl_hdl)
{
	int ret;

	if (tbl_hdl == NULL || mem_type_ptr == NULL || number_of_entries == 0) {
		IPAERR(
			"Invalid parameters tbl_hdl=%pK mem_type_ptr=%p number_of_entries=%d\n",
			tbl_hdl,
			mem_type_ptr,
			number_of_entries);
		return -EINVAL;
	}

	*tbl_hdl = 0;

	ret = ipa_nati_add_ipv4_tbl(
		public_ip_addr, mem_type_ptr, number_of_entries, tbl_hdl);

	if (ret) {
		IPAERR("unable to add NAT table\n");
		return ret;
	}

	IPADBG("Returning table handle 0x%x\n", *tbl_hdl);

	return ret;
} /* __ipa_nat_add_ipv4_tbl() */

/**
 * ipa_nat_del_ipv4_tbl() - delete ipv4 table
 * @table_handle: [in] Handle of ipv4 nat table
 *
 * To delete given ipv4 nat table
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_nat_del_ipv4_tbl(
	uint32_t tbl_hdl)
{
	if ( ! VALID_TBL_HDL(tbl_hdl) ) {
		IPAERR("Invalid table handle passed 0x%08X\n", tbl_hdl);
		return -EINVAL;
	}

	IPADBG("Passed Table Handle: 0x%08X\n", tbl_hdl);

	return ipa_nati_del_ipv4_table(tbl_hdl);
}

/**
 * ipa_nat_add_ipv4_rule() - to insert new ipv4 rule
 * @table_handle: [in] handle of ipv4 nat table
 * @rule: [in]  Pointer to new rule
 * @rule_handle: [out] Return the handle to rule
 *
 * To insert new ipv4 nat rule into ipv4 nat table
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_nat_add_ipv4_rule(
	uint32_t tbl_hdl,
	const ipa_nat_ipv4_rule *clnt_rule,
	uint32_t *rule_hdl)
{
	int result = -EINVAL;

	if ( ! VALID_TBL_HDL(tbl_hdl) ||
		 rule_hdl == NULL ||
		 clnt_rule == NULL ) {
		IPAERR(
			"Invalid parameters tbl_hdl=%d clnt_rule=%pK rule_hdl=%pK\n",
			tbl_hdl, clnt_rule, rule_hdl);
		return result;
	}

	IPADBG("Passed Table handle: 0x%x\n", tbl_hdl);

	if (ipa_nati_add_ipv4_rule(tbl_hdl, clnt_rule, rule_hdl)) {
		return result;
	}

	IPADBG("Returning rule handle %u\n", *rule_hdl);

	return 0;
}

/**
 * ipa_nat_del_ipv4_rule() - to delete ipv4 nat rule
 * @table_handle: [in] handle of ipv4 nat table
 * @rule_handle: [in] ipv4 nat rule handle
 *
 * To insert new ipv4 nat rule into ipv4 nat table
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_nat_del_ipv4_rule(
	uint32_t tbl_hdl,
	uint32_t rule_hdl)
{
	int result = -EINVAL;

	if ( ! VALID_TBL_HDL(tbl_hdl) || ! VALID_RULE_HDL(rule_hdl) )
	{
		IPAERR("Invalid parameters tbl_hdl=0x%08X rule_hdl=0x%08X\n",
			   tbl_hdl, rule_hdl);
		return result;
	}

	IPADBG("Passed Table: 0x%08X and rule handle 0x%08X\n", tbl_hdl, rule_hdl);

	result = ipa_nati_del_ipv4_rule(tbl_hdl, rule_hdl);
	if (result) {
		IPAERR(
			"Unable to delete rule with handle 0x%08X "
			"from hw for NAT table with handle 0x%08X\n",
			rule_hdl, tbl_hdl);
		return result;
	}

	return 0;
}

/**
 * ipa_nat_query_timestamp() - to query timestamp
 * @table_handle: [in] handle of ipv4 nat table
 * @rule_handle: [in] ipv4 nat rule handle
 * @time_stamp: [out] time stamp of rule
 *
 * To retrieve the timestamp that lastly the
 * nat rule was accessed
 *
 * Returns:     0  On Success, negative on failure
 */
int ipa_nat_query_timestamp(
	uint32_t tbl_hdl,
	uint32_t rule_hdl,
	uint32_t *time_stamp)
{
	int redirect = 0;

	if ( ! VALID_TBL_HDL(tbl_hdl) ||
		! VALID_RULE_HDL(rule_hdl) ||
		time_stamp == NULL)
	{
		IPAERR("Invalid parameters passed tbl_hdl=0x%x rule_hdl=%u time_stamp=%pK\n",
				tbl_hdl, rule_hdl, time_stamp);
		return -EINVAL;
	}

	IPADBG("Passed Table 0x%x and rule handle %u\n", tbl_hdl, rule_hdl);

	return ipa_nati_query_timestamp_redirect(tbl_hdl, rule_hdl, time_stamp, &redirect);
}

/**
 * ipa_nat_query_timestamp_redirect() - to query timestamp and redirect flag
 * @table_handle: [in] handle of ipv4 nat table
 * @rule_handle: [in] ipv4 nat rule handle
 * @time_stamp: [out] time stamp of rule
 * @redirect: [out] redirect flag of rule
 *
 * To retrieve the timestamp that lastly the
 * nat rule was accessed and nat entry redirect flag
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_nat_query_timestamp_redirect(
	uint32_t tbl_hdl,
	uint32_t rule_hdl,
	uint32_t *time_stamp,
	uint32_t *redirect)
{
	if ( ! VALID_TBL_HDL(tbl_hdl) ||
		 ! VALID_RULE_HDL(rule_hdl) ||
		 time_stamp == NULL || redirect == NULL)
	{
		IPAERR("Invalid parameters passed tbl_hdl=0x%x rule_hdl=%u time_stamp=%pK redirect=%pK\n",
			   tbl_hdl, rule_hdl, time_stamp, redirect);
		return -EINVAL;
	}

	IPADBG("Passed Table 0x%x and rule handle %u\n", tbl_hdl, rule_hdl);

	return ipa_nati_query_timestamp_redirect(tbl_hdl, rule_hdl, time_stamp, redirect);
}

/**
* ipa_nat_modify_pdn() - modify single PDN entry in the PDN config table
* @table_handle: [in] handle of ipv4 nat table
* @pdn_index : [in] the index of the entry to be modified
* @pdn_info : [in] values for the PDN entry to be changed
*
* Modify a PDN entry
*
* Returns:	0  On Success, negative on failure
*/
int ipa_nat_modify_pdn(
	uint32_t tbl_hdl,
	uint8_t pdn_index,
	ipa_nat_pdn_entry *pdn_info)
{
	struct ipa_ioc_nat_pdn_entry pdn_data;

	if ( ! VALID_TBL_HDL(tbl_hdl) ||
		 pdn_info == NULL) {
		IPAERR(
			"invalid parameters passed tbl_hdl=%d pdn_info=%pK\n",
			tbl_hdl, pdn_info);
		return -EINVAL;
	}

	if (pdn_index > IPA_MAX_PDN_NUM) {
		IPAERR(
			"PDN index %d is out of range maximum %d",
			pdn_index, IPA_MAX_PDN_NUM);
		return -EINVAL;
	}

	pdn_data.pdn_index = pdn_index;
	pdn_data.public_ip = pdn_info->public_ip;
	pdn_data.src_metadata = pdn_info->src_metadata;
	pdn_data.dst_metadata = pdn_info->dst_metadata;

	return ipa_nati_modify_pdn(&pdn_data);
}

/**
* ipa_nat_get_pdn_index() - get a PDN index for a public ip
* @public_ip : [in] IPv4 address of the PDN entry
* @pdn_index : [out] the index of the requested PDN entry
*
* Get a PDN entry
*
* Returns:	0  On Success, negative on failure
*/
int ipa_nat_get_pdn_index(
	uint32_t public_ip,
	uint8_t *pdn_index)
{
	if(!pdn_index)
	{
		IPAERR("NULL PDN index\n");
		return -EINVAL;
	}

	return ipa_nati_get_pdn_index(public_ip, pdn_index);
}

/**
* ipa_nat_alloc_pdn() - allocate a PDN for new WAN
* @pdn_info : [in] values for the PDN entry to be created
* @pdn_index : [out] the index of the requested PDN entry
*
* allocate a new PDN entry
*
* Returns:	0  On Success, negative on failure
*/
int ipa_nat_alloc_pdn(
	ipa_nat_pdn_entry *pdn_info,
	uint8_t *pdn_index)
{
	if(!pdn_info)
	{
		IPAERR("NULL PDN info\n");
		return -EINVAL;
	}

	if(!pdn_index)
	{
		IPAERR("NULL PDN index\n");
		return -EINVAL;
	}

	return ipa_nati_alloc_pdn(pdn_info, pdn_index);
}

/**
* ipa_nat_get_pdn_count() - get the number of allocated PDNs
* @pdn_cnt : [out] the number of allocated PDNs
*
* get the number of allocated PDNs
*
* Returns:	0  On Success, negative on failure
*/
int ipa_nat_get_pdn_count(
	uint8_t *pdn_cnt)
{
	if(!pdn_cnt)
	{
		IPAERR("NULL PDN count\n");
		return -EINVAL;
	}

	*pdn_cnt = ipa_nati_get_pdn_cnt();

	return 0;
}

/**
* ipa_nat_dealloc_pdn() - deallocate a PDN entry
* @pdn_index : [in] pdn index to be deallocated
*
* deallocate a PDN in specified index - zero the PDN entry
*
* Returns:	0  On Success, negative on failure
*/
int ipa_nat_dealloc_pdn(
	uint8_t pdn_index)
{
	if(pdn_index > IPA_MAX_PDN_NUM) {
		IPAERR("PDN index is out of range %d", pdn_index);
		return -EINVAL;
	}

	return ipa_nati_dealloc_pdn(pdn_index);
}

/**
 * ipa_nat_vote_clock() - used for voting clock
 * @vote_type: [in] desired vote type
 */
int ipa_nat_vote_clock(
	enum ipa_app_clock_vote_type vote_type )
{
	if ( ! (vote_type >= IPA_APP_CLK_DEVOTE &&
			vote_type <= IPA_APP_CLK_RESET_VOTE) )
	{
		IPAERR("Bad vote_type(%u) parameter\n", vote_type);
		return -EINVAL;
	}

	return ipa_nati_vote_clock(vote_type);
}
