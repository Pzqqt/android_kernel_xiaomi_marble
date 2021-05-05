/*
 * Copyright (c) 2018, 2020 The Linux Foundation. All rights reserved.
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
#ifndef IPA_IPV6CT_H
#define IPA_IPV6CT_H

#include <stdint.h>
#include <stdbool.h>
#include <linux/msm_ipa.h>

/**
 * enum ipa_ipv6_ct_direction_settings_type - direction filter settings
 *
 * IPA_IPV6CT_DIRECTION_DENY_ALL  - deny inbound and outbound
 * IPA_IPV6CT_DIRECTION_ALLOW_OUT - allow outbound and deny inbound
 * IPA_IPV6CT_DIRECTION_ALLOW_IN  - allow inbound and deny outbound
 * IPA_IPV6CT_DIRECTION_ALLOW_ALL - allow inbound and outbound
 */
typedef enum
{
	IPA_IPV6CT_DIRECTION_DENY_ALL  = 0,
	IPA_IPV6CT_DIRECTION_ALLOW_OUT = 1,
	IPA_IPV6CT_DIRECTION_ALLOW_IN  = 2,
	IPA_IPV6CT_DIRECTION_ALLOW_ALL = 3
} ipa_ipv6_ct_direction_settings_type;

/**
 * struct ipa_ipv6ct_rule - To hold IPv6CT rule
 * @src_ipv6_lsb: source IPv6 address LSB
 * @src_ipv6_msb: source IPv6 address MSB
 * @dest_ipv6_lsb: destination IPv6 address LSB
 * @dest_ipv6_msb: destination IPv6 address MSB
 * @direction_settings: direction filter settings (inbound/outbound) (see ipa_ipv6_ct_direction_settings_type)
 * @src_port: source port
 * @dest_port: destination port
 * @protocol: protocol of rule (tcp/udp)
 * @uc_activation_index: index pointing to uc activation table
 * @s: bit indication to use the system or local (1 or 0) addr for above table
 * @ucp: enable uc processing
 */
typedef struct {
	uint64_t src_ipv6_lsb;
	uint64_t src_ipv6_msb;
	uint64_t dest_ipv6_lsb;
	uint64_t dest_ipv6_msb;
	ipa_ipv6_ct_direction_settings_type  direction_settings;
	bool  ucp;
	bool s;
	uint16_t uc_activation_index;
	uint16_t src_port;
	uint16_t dest_port;
	uint8_t  protocol;
} ipa_ipv6ct_rule;

/**
 * ipa_ipv6ct_add_tbl() - create IPv6CT table
 * @number_of_entries: [in] number of IPv6CT entries
 * @table_handle: [out] handle of new IPv6CT table
 *
 * To create new IPv6CT table
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_ipv6ct_add_tbl(uint16_t number_of_entries, uint32_t* table_handle);

/**
 * ipa_ipv6ct_del_tbl() - delete IPv6CT table
 * @table_handle: [in] Handle of IPv6CT table
 *
 * To delete given IPv6CT table
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_ipv6ct_del_tbl(uint32_t table_handle);

/**
 * ipa_ipv6ct_add_rule() - to insert new IPv6CT rule
 * @table_handle: [in] handle of IPv6CT table
 * @user_rule: [in] Pointer to new rule
 * @rule_handle: [out] Return the handle to rule
 *
 * To insert new rule into a IPv6CT table
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_ipv6ct_add_rule(uint32_t table_handle, const ipa_ipv6ct_rule* user_rule, uint32_t* rule_handle);

/**
 * ipa_ipv6ct_del_rule() - to delete IPv6CT rule
 * @table_handle: [in] handle of IPv6CT table
 * @rule_handle: [in] IPv6CT rule handle
 *
 * To delete a rule from a IPv6CT table
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_ipv6ct_del_rule(uint32_t table_handle, uint32_t rule_handle);

/**
 * ipa_ipv6ct_query_timestamp() - to query timestamp
 * @table_handle: [in] handle of IPv6CT table
 * @rule_handle: [in] IPv6CT rule handle
 * @time_stamp: [out] time stamp of rule
 *
 * To retrieve the timestamp that lastly the IPv6CT rule was accessed
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_ipv6ct_query_timestamp(uint32_t table_handle, uint32_t rule_handle, uint32_t* time_stamp);

/**
 * ipa_ipv6ct_dump_table() - dumps IPv6CT table
 * @table_handle: [in] handle of IPv6CT table
 */
void ipa_ipv6ct_dump_table(uint32_t tbl_hdl);

/**
 * ipa_ipv6ct_add_uc_act_entry() - add uc activation entry
 * @u: [in] structure specifying the uC activation entry
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_ipv6ct_add_uc_act_entry(union ipa_ioc_uc_activation_entry *u);

/**
 * ipa_ipv6ct_del_uc_act_entry() - del uc activation entry
 * @index: [in] index of the uc activation entry to be removed
 *
 * Returns:	0  On Success, negative on failure
 */
int ipa_ipv6ct_del_uc_act_entry(uint16_t index);

#endif

