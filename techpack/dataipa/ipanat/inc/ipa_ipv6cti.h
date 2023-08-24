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
#ifndef IPA_IPV6CTI_H
#define IPA_IPV6CTI_H

#include "ipa_table.h"
#include "ipa_mem_descriptor.h"
#include "ipa_nat_utils.h"

#define IPA_IPV6CT_MAX_TBLS   1

#define IPA_IPV6CT_RULE_FLAG_FIELD_OFFSET        34
#define IPA_IPV6CT_RULE_NEXT_FIELD_OFFSET        40
#define IPA_IPV6CT_RULE_PROTO_FIELD_OFFSET       38

#define IPA_IPV6CT_FLAG_ENABLE_BIT  1

#define IPA_IPV6CT_DIRECTION_ALLOW_BIT  1
#define IPA_IPV6CT_DIRECTION_DISALLOW_BIT 0

#define IPA_IPV6CT_INVALID_PROTO_FIELD_VALUE 0xFF00
#define IPA_IPV6CT_INVALID_PROTO_FIELD_CMP   0xFF

typedef enum
{
	IPA_IPV6CT_TABLE_FLAGS,
	IPA_IPV6CT_TABLE_NEXT_INDEX,
	IPA_IPV6CT_TABLE_PROTOCOL,
	IPA_IPV6CT_TABLE_DMA_CMD_MAX
} ipa_ipv6ct_table_dma_cmd_type;

/*------------------------  IPV6CT Table Entry  ---------------------------------------------------

  -------------------------------------------------------------------------------------------------
  |     7     |      6      |     5     |     4     |     3     |     2     |     1     |     0     |
  ---------------------------------------------------------------------------------------------------
  |                              Outbound Src IPv6 Address (8 LSB Bytes)                            |
  ---------------------------------------------------------------------------------------------------
  |                              Outbound Src IPv6 Address (8 MSB Bytes)                            |
  ---------------------------------------------------------------------------------------------------
  |                              Outbound Dest IPv6 Address (8 LSB Bytes)                           |
  ---------------------------------------------------------------------------------------------------
  |                              Outbound Dest IPv6 Address (8 MSB Bytes)                           |
  ---------------------------------------------------------------------------------------------------
  | Protocol  |           TimeStamp (3B)            |       Flags (2B)      |Rsvd   |S |uC activatio|
  |    (1B)   |                                     |Enable|Redirect|Resv   |[15:14]|13|Index [12:0]|
  ---------------------------------------------------------------------------------------------------
  |Reserved   |Settings     |     Src Port (2B)     |     Dest Port (2B)    |    Next Index (2B)    |
  |  (1B)     |  (1B)       |                       |                       |                       |
  ---------------------------------------------------------------------------------------------------
  |           SW Specific Parameters(4B)            |                   Reserved (4B)               |
  |     Prev Index (2B)     |    Reserved (2B)      |                                               |
  ---------------------------------------------------------------------------------------------------
  |                                             Reserved (8B)                                       |
  ---------------------------------------------------------------------------------------------------

  Settings(1B)
 -----------------------------------------------
 |IN Allowed|OUT Allowed|Reserved|uC processing|
 |[7:7]     |[6:6]      |[5:1]   |[0:0]        |
 -----------------------------------------------

  Dont change below structure definition.
  It should be same as above(little endian order)
  -------------------------------------------------------------------------------------------------*/
typedef struct
{
	uint64_t src_ipv6_lsb : 64;
	uint64_t src_ipv6_msb : 64;
	uint64_t dest_ipv6_lsb : 64;
	uint64_t dest_ipv6_msb : 64;

	uint64_t uc_activation_index : 13;
	uint64_t s : 1;
	uint64_t rsvd1 : 16;
	uint64_t redirect : 1;
	uint64_t enable : 1;
	uint64_t time_stamp : 24;
	uint64_t protocol : 8;

	uint64_t next_index : 16;
	uint64_t dest_port : 16;
	uint64_t src_port : 16;
	uint64_t ucp : 1;
	uint64_t rsvd2 : 5;
	uint64_t out_allowed : 1;
	uint64_t in_allowed : 1;
	uint64_t rsvd3 : 8;

	uint64_t rsvd4 : 48;
	uint64_t prev_index : 16;

	uint64_t rsvd5 : 64;
} ipa_ipv6ct_hw_entry;

/*
	----------------------
	|    1    |    0     |
	----------------------
	|     Flags(2B)      |
	|Enable|Redirect|Resv|
	----------------------
*/
typedef struct
{
	uint16_t rsvd1 : 14;
	uint16_t redirect : 1;
	uint16_t enable : 1;
} ipa_ipv6ct_flags;

typedef struct
{
	ipa_mem_descriptor mem_desc;
	ipa_table table;
	ipa_table_dma_cmd_helper table_dma_cmd_helpers[IPA_IPV6CT_TABLE_DMA_CMD_MAX];
}  ipa_ipv6ct_table;

typedef struct
{
	ipa_descriptor* ipa_desc;
	ipa_ipv6ct_table tables[IPA_IPV6CT_MAX_TBLS];
	uint8_t table_cnt;
} ipa_ipv6ct;

#endif
