/*
 * Copyright (c) 2013-2021 The Linux Foundation. All rights reserved.
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
#ifndef IPA_NAT_DRV_H
#define IPA_NAT_DRV_H

#include "ipa_nat_utils.h"

#include <stdint.h>  /* uint32_t */
#include <stdbool.h>

/**
 * ipa_nat_is_sram_supported() - Reports if sram is available for use
 */
bool ipa_nat_is_sram_supported(void);

/**
 * struct ipa_nat_ipv4_rule - To hold ipv4 nat rule
 * @target_ip: destination ip address
 * @private_ip: private ip address
 * @target_port: destination port
 * @private_port: private port
 * @protocol: protocol of rule (tcp/udp)
 * @pdn_index: PDN index in the PDN config table
 * @redirect: used internally by various API calls
 * @enable: used internally by various API calls
 * @time_stamp: used internally by various API calls
 * @uc_activation_index: index pointing to uc activation table
 * @s: bit indication to use the system or local (1 or 0) addr for above table
 * @ucp: enable uc processing
 * @dst_only: construct NAT for DL only
 * @src_only: construct NAT for UL only
 */
typedef struct {
	uint32_t target_ip;
	uint32_t private_ip;
	uint16_t target_port;
	uint16_t private_port;
	uint16_t public_port;
	uint8_t  protocol;
	uint8_t  pdn_index;
	uint8_t  redirect;
	uint8_t  enable;
	uint32_t time_stamp;
	uint16_t uc_activation_index;
	bool s;
	bool ucp;
	bool dst_only;
	bool src_only;
} ipa_nat_ipv4_rule;

static inline char* prep_nat_ipv4_rule_4print(
	ipa_nat_ipv4_rule* rule_ptr,
	char*              buf_ptr,
	uint32_t           buf_sz )
{
	if ( rule_ptr && buf_ptr && buf_sz )
	{
		snprintf(
			buf_ptr, buf_sz,
			"IPV4 RULE: "
			"protocol(0x%02X) "
			"public_port(0x%04X) "
			"target_ip(0x%08X) "
			"target_port(0x%04X) "
			"private_ip(0x%08X) "
			"private_port(0x%04X) "
			"pdn_index(0x%02X)",
			rule_ptr->protocol,
			rule_ptr->public_port,
			rule_ptr->target_ip,
			rule_ptr->target_port,
			rule_ptr->private_ip,
			rule_ptr->private_port,
			rule_ptr->pdn_index);
	}

	return buf_ptr;
}

/**
 * struct ipa_nat_pdn_entry - holds a PDN entry data
 * @public_ip: PDN's public ip address
 * @src_metadata: metadata to be used for source NAT metadata replacement
 * @dst_metadata: metadata to be used for destination NAT metadata replacement
 */
typedef struct {
	uint32_t public_ip;
	uint32_t src_metadata;
	uint32_t dst_metadata;
} ipa_nat_pdn_entry;

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
	uint32_t *table_handle);

/**
 * ipa_nat_del_ipv4_tbl() - delete ipv4 table
 * @table_handle: [in] Handle of ipv4 nat table
 *
 * To delete given ipv4 nat table
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_nat_del_ipv4_tbl(uint32_t table_handle);

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
int ipa_nat_add_ipv4_rule(uint32_t table_handle,
				const ipa_nat_ipv4_rule * rule,
				uint32_t *rule_handle);

/**
 * ipa_nat_del_ipv4_rule() - to delete ipv4 nat rule
 * @table_handle: [in] handle of ipv4 nat table
 * @rule_handle: [in] ipv4 nat rule handle
 *
 * To insert new ipv4 nat rule into ipv4 nat table
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_nat_del_ipv4_rule(uint32_t table_handle,
				uint32_t rule_handle);


/**
 * ipa_nat_query_timestamp() - to query timestamp
 * @table_handle: [in] handle of ipv4 nat table
 * @rule_handle: [in] ipv4 nat rule handle
 * @time_stamp: [out] time stamp of rule
 *
 * To retrieve the timestamp that lastly the
 * nat rule was accessed
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_nat_query_timestamp(uint32_t  table_handle,
				uint32_t  rule_handle,
				uint32_t  *time_stamp);


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
int ipa_nat_modify_pdn(uint32_t  tbl_hdl,
	uint8_t pdn_index,
	ipa_nat_pdn_entry *pdn_info);

/**
* ipa_nat_get_pdn_index() - get a PDN index for a public ip
* @public_ip : [in] IPv4 address of the PDN entry
* @pdn_index : [out] the index of the requested PDN entry
*
* Get a PDN entry
*
* Returns:	0  On Success, negative on failure
*/
int ipa_nat_get_pdn_index(uint32_t public_ip, uint8_t *pdn_index);

/**
* ipa_nat_alloc_pdn() - allocate a PDN for new WAN
* @pdn_info : [in] values for the PDN entry to be created
* @pdn_index : [out] the index of the requested PDN entry
*
* allocate a new PDN entry
*
* Returns:	0  On Success, negative on failure
*/
int ipa_nat_alloc_pdn(ipa_nat_pdn_entry *pdn_info,
	uint8_t *pdn_index);

/**
* ipa_nat_get_pdn_count() - get the number of allocated PDNs
* @pdn_cnt : [out] the number of allocated PDNs
*
* get the number of allocated PDNs
*
* Returns:	0  On Success, negative on failure
*/
int ipa_nat_get_pdn_count(uint8_t *pdn_cnt);

/**
* ipa_nat_dealloc_pdn() - deallocate a PDN entry
* @pdn_index : [in] pdn index to be deallocated
*
* deallocate a PDN in specified index - zero the PDN entry
*
* Returns:	0  On Success, negative on failure
*/
int ipa_nat_dealloc_pdn(uint8_t pdn_index);


/**
 * ipa_nat_dump_ipv4_table() - dumps IPv4 NAT table
 * @table_handle: [in] handle of IPv4 NAT table
 */
void ipa_nat_dump_ipv4_table(uint32_t tbl_hdl);

/**
 * ipa_nat_vote_clock() - used for voting clock
 * @vote_type: [in] desired vote type
 */
int ipa_nat_vote_clock(
	enum ipa_app_clock_vote_type vote_type );

/**
 * ipa_nat_switch_to() - While in HYBRID mode only, used for switching
 * from SRAM to DDR or the reverse.
 * @nmi: memory type to switch to
 * @hold_state: Will the new memory type get locked in (ie. no more
 *              oscilation between the memory types)
 */
int ipa_nat_switch_to(
	enum ipa3_nat_mem_in nmi,
	bool                 hold_state );

#endif

