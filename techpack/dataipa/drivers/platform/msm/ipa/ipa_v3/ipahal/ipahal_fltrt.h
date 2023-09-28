/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
 */

#ifndef _IPAHAL_FLTRT_H_
#define _IPAHAL_FLTRT_H_

/*
 * struct ipahal_fltrt_alloc_imgs_params - Params for tbls imgs allocations
 *  The allocation logic will allocate DMA memory representing the header.
 *  If the bodies are local (SRAM) the allocation will allocate
 *  a DMA buffers that would contain the content of these local tables in raw
 * @ipt: IP version type
 * @tbls_num: Number of tables to represent by the header
 * @num_lcl_hash_tbls: Number of local (sram) hashable tables
 * @num_lcl_nhash_tbls: Number of local (sram) non-hashable tables
 * @total_sz_lcl_hash_tbls: Total size of local hashable tables
 * @total_sz_lcl_nhash_tbls: Total size of local non-hashable tables
 * @hash_hdr/nhash_hdr: OUT params for the header structures
 * @hash_bdy/nhash_bdy: OUT params for the local body structures
 */
struct ipahal_fltrt_alloc_imgs_params {
	enum ipa_ip_type ipt;
	u32 tbls_num;
	u32 num_lcl_hash_tbls;
	u32 num_lcl_nhash_tbls;
	u32 total_sz_lcl_hash_tbls;
	u32 total_sz_lcl_nhash_tbls;

	/* OUT PARAMS */
	struct ipa_mem_buffer hash_hdr;
	struct ipa_mem_buffer nhash_hdr;
	struct ipa_mem_buffer hash_bdy;
	struct ipa_mem_buffer nhash_bdy;
};

/*
 * enum ipahal_rt_rule_hdr_type - Header type used in rt rules
 * @IPAHAL_RT_RULE_HDR_NONE: No header is used
 * @IPAHAL_RT_RULE_HDR_RAW: Raw header is used
 * @IPAHAL_RT_RULE_HDR_PROC_CTX: Header Processing context is used
 */
enum ipahal_rt_rule_hdr_type {
	IPAHAL_RT_RULE_HDR_NONE,
	IPAHAL_RT_RULE_HDR_RAW,
	IPAHAL_RT_RULE_HDR_PROC_CTX,
};

/*
 * struct ipahal_rt_rule_gen_params - Params for generating rt rule
 * @ipt: IP family version
 * @dst_pipe_idx: Destination pipe index
 * @hdr_type: Header type to be used
 * @hdr_lcl: Does header on local or system table?
 * @hdr_ofst: Offset of the header in the header table
 * @priority: Rule priority
 * @id: Rule ID
 * @cnt_idx: Stats counter index
 * @rule: Rule info
 */
struct ipahal_rt_rule_gen_params {
	enum ipa_ip_type ipt;
	int dst_pipe_idx;
	enum ipahal_rt_rule_hdr_type hdr_type;
	bool hdr_lcl;
	u32 hdr_ofst;
	u32 priority;
	u32 id;
	u8 cnt_idx;
	const struct ipa_rt_rule_i *rule;
};

/*
 * struct ipahal_rt_rule_entry - Rt rule info parsed from H/W
 * @dst_pipe_idx: Destination pipe index
 * @hdr_lcl: Does the references header located in sram or system mem?
 * @hdr_ofst: Offset of the header in the header table
 * @hdr_type: Header type to be used
 * @priority: Rule priority
 * @retain_hdr: to retain the removed header in header removal
 * @id: Rule ID
 * @cnt_idx: stats counter index
 * @close_aggr_irq_mod: close aggregation/coalescing and close GSI
 *  interrupt moderation
 * @eq_attrib: Equations and their params in the rule
 * @rule_size: Rule size in memory
 */
struct ipahal_rt_rule_entry {
	int dst_pipe_idx;
	bool hdr_lcl;
	u32 hdr_ofst;
	enum ipahal_rt_rule_hdr_type hdr_type;
	u32 priority;
	bool retain_hdr;
	u32 id;
	u8 cnt_idx;
	u8 close_aggr_irq_mod;
	struct ipa_ipfltri_rule_eq eq_attrib;
	u32 rule_size;
};

/*
 * struct ipahal_flt_rule_gen_params - Params for generating flt rule
 * @ipt: IP family version
 * @rt_tbl_idx: Routing table the rule pointing to
 * @priority: Rule priority
 * @id: Rule ID
 * @cnt_idx: Stats counter index
 * @rule: Rule info
 */
struct ipahal_flt_rule_gen_params {
	enum ipa_ip_type ipt;
	u32 rt_tbl_idx;
	u32 priority;
	u32 id;
	u8 cnt_idx;
	const struct ipa_flt_rule_i *rule;
};

/*
 * struct ipahal_flt_rule_entry - Flt rule info parsed from H/W
 * @rule: Rule info
 * @priority: Rule priority
 * @id: Rule ID
 * @cnt_idx: stats counter index
 * @rule_size: Rule size in memory
 */
struct ipahal_flt_rule_entry {
	struct ipa_flt_rule_i rule;
	u32 priority;
	u32 id;
	u8 cnt_idx;
	u32 rule_size;
};

/* Get the H/W table (flt/rt) header width */
u32 ipahal_get_hw_tbl_hdr_width(void);

/* Get the H/W local table (SRAM) address alignment
 * Tables headers references to local tables via offsets in SRAM
 * This function return the alignment of the offset that IPA expects
 */
u32 ipahal_get_lcl_tbl_addr_alignment(void);

/* Get the H/W (flt/rt) prefetch buf size */
u32 ipahal_get_hw_prefetch_buf_size(void);

/*
 * Rule priority is used to distinguish rules order
 * at the integrated table consisting from hashable and
 * non-hashable tables. Max priority are rules that once are
 * scanned by IPA, IPA will not look for further rules and use it.
 */
int ipahal_get_rule_max_priority(void);

/* Given a priority, calc and return the next lower one if it is in
 * legal range.
 */
int ipahal_rule_decrease_priority(int *prio);

/* Does the given ID represents rule miss? */
bool ipahal_is_rule_miss_id(u32 id);

/* Get rule ID with high bit only asserted
 * Used e.g. to create groups of IDs according to this bit
 */
u32 ipahal_get_rule_id_hi_bit(void);

/* Get the low value possible to be used for rule-id */
u32 ipahal_get_low_rule_id(void);

/*
 * low value possible for counter hdl id
 */
u32 ipahal_get_low_hdl_id(void);

/*
 * max counter hdl id for stats
 */
u32 ipahal_get_high_hdl_id(void);

/* used for query check and associated with rt/flt rules */
bool ipahal_is_rule_cnt_id_valid(u8 cnt_id);

/* max rule id for stats */
bool ipahal_get_max_stats_rule_id(void);

/*
 * ipahal_rt_generate_empty_img() - Generate empty route image
 *  Creates routing header buffer for the given tables number.
 * For each table, make it point to the empty table on DDR.
 * @tbls_num: Number of tables. For each will have an entry in the header
 * @hash_hdr_size: SRAM buf size of the hash tbls hdr. Used for space check
 * @nhash_hdr_size: SRAM buf size of the nhash tbls hdr. Used for space check
 * @mem: mem object that points to DMA mem representing the hdr structure
 * @atomic: should DMA allocation be executed with atomic flag
 */
int ipahal_rt_generate_empty_img(u32 tbls_num, u32 hash_hdr_size,
	u32 nhash_hdr_size, struct ipa_mem_buffer *mem, bool atomic);

/*
 * ipahal_flt_generate_empty_img() - Generate empty filter image
 *  Creates filter header buffer for the given tables number.
 *  For each table, make it point to the empty table on DDR.
 * @tbls_num: Number of tables. For each will have an entry in the header
 * @hash_hdr_size: SRAM buf size of the hash tbls hdr. Used for space check
 * @nhash_hdr_size: SRAM buf size of the nhash tbls hdr. Used for space check
 * @ep_bitmap: Bitmap representing the EP that has flt tables. The format
 *  should be: bit0->EP0, bit1->EP1
 * @mem: mem object that points to DMA mem representing the hdr structure
 * @atomic: should DMA allocation be executed with atomic flag
 */
int ipahal_flt_generate_empty_img(u32 tbls_num, u32 hash_hdr_size,
	u32 nhash_hdr_size, u64 ep_bitmap, struct ipa_mem_buffer *mem,
	bool atomic);

/*
 * ipahal_fltrt_allocate_hw_tbl_imgs() - Allocate tbl images DMA structures
 *  Used usually during commit.
 *  Allocates header structures and init them to point to empty DDR table
 *  Allocate body strucutres for local bodies tables
 * @params: Parameters for IN and OUT regard the allocation.
 */
int ipahal_fltrt_allocate_hw_tbl_imgs(
	struct ipahal_fltrt_alloc_imgs_params *params);

/*
 * ipahal_fltrt_allocate_hw_sys_tbl() - Allocate DMA mem for H/W flt/rt sys tbl
 * @tbl_mem: IN/OUT param. size for effective table size. Pointer, for the
 *  allocated memory.
 *
 * The size is adapted for needed alignments/borders.
 */
int ipahal_fltrt_allocate_hw_sys_tbl(struct ipa_mem_buffer *tbl_mem);

/*
 * ipahal_fltrt_write_addr_to_hdr() - Fill table header with table address
 *  Given table addr/offset, adapt it to IPA H/W format and write it
 *  to given header index.
 * @addr: Address or offset to be used
 * @hdr_base: base address of header structure to write the address
 * @hdr_idx: index of the address in the header structure
 * @is_sys: Is it system address or local offset
 */
int ipahal_fltrt_write_addr_to_hdr(u64 addr, void *hdr_base, u32 hdr_idx,
	bool is_sys);

/*
 * ipahal_fltrt_read_addr_from_hdr() - Given sram address, read it's
 *  content (physical address or offset) and parse it.
 * @hdr_base: base sram address of the header structure.
 * @hdr_idx: index of the header entry line in the header structure.
 * @addr: The parsed address - Out parameter
 * @is_sys: Is this system or local address - Out parameter
 */
int ipahal_fltrt_read_addr_from_hdr(void *hdr_base, u32 hdr_idx, u64 *addr,
	bool *is_sys);

/*
 * ipahal_rt_generate_hw_rule() - generates the routing hardware rule.
 * @params: Params for the rule creation.
 * @hw_len: Size of the H/W rule to be returned
 * @buf: Buffer to build the rule in. If buf is NULL, then the rule will
 *  be built in internal temp buf. This is used e.g. to get the rule size
 *  only.
 */
int ipahal_rt_generate_hw_rule(struct ipahal_rt_rule_gen_params *params,
	u32 *hw_len, u8 *buf);

/*
 * ipahal_flt_generate_hw_rule() - generates the filtering hardware rule.
 * @params: Params for the rule creation.
 * @hw_len: Size of the H/W rule to be returned
 * @buf: Buffer to build the rule in. If buf is NULL, then the rule will
 *  be built in internal temp buf. This is used e.g. to get the rule size
 *  only.
 */
int ipahal_flt_generate_hw_rule(struct ipahal_flt_rule_gen_params *params,
	u32 *hw_len, u8 *buf);

/*
 * ipahal_flt_generate_equation() - generate flt rule in equation form
 *  Will build equation form flt rule from given info.
 * @ipt: IP family
 * @attrib: Rule attribute to be generated
 * @eq_atrb: Equation form generated rule
 * Note: Usage example: Pass the generated form to other sub-systems
 *  for inter-subsystems rules exchange.
 */
int ipahal_flt_generate_equation(enum ipa_ip_type ipt,
		const struct ipa_rule_attrib *attrib,
		struct ipa_ipfltri_rule_eq *eq_atrb);

/*
 * ipahal_rt_parse_hw_rule() - Parse H/W formated rt rule
 *  Given the rule address, read the rule info from H/W and parse it.
 * @rule_addr: Rule address (virtual memory)
 * @rule: Out parameter for parsed rule info
 */
int ipahal_rt_parse_hw_rule(u8 *rule_addr,
	struct ipahal_rt_rule_entry *rule);

/*
 * ipahal_flt_parse_hw_rule() - Parse H/W formated flt rule
 *  Given the rule address, read the rule info from H/W and parse it.
 * @rule_addr: Rule address (virtual memory)
 * @rule: Out parameter for parsed rule info
 */
int ipahal_flt_parse_hw_rule(u8 *rule_addr,
	struct ipahal_flt_rule_entry *rule);

/*
 * ipa_fltrt_get_aligned_lcl_bdy_size() - Calculate real SRAM block aligned size
 *  required for flt table bodies
 * @num_lcl_tbls: [in] Number of the tables
 * @total_sz_lcl_tbls: [in] The size in driver cashe
 */
u32 ipa_fltrt_get_aligned_lcl_bdy_size(u32 num_lcl_tbls, u32 total_sz_lcl_tbls);

#ifdef IPA_FLT_EXT_MPLS_GRE_GENERAL
/*
 * *****************************************************************************
 * MLPS OVER GRE EQUATION GENERATION CONSTANTS
 * *****************************************************************************
 */
#define ETH_HDR_LEN            (sizeof(struct ethhdr))
#define ETH_HDR_W_TAGS_LEN     (sizeof(struct ethhdr) + S_C_TAG_LEN)
#define ETH_TYPE_LEN           (ETH_TLEN)
#define MAC_ADDR_LEN           (ETH_ALEN)
#define MAC_ADDRS_LEN          (MAC_ADDR_LEN*2)
#define VLAN_TAG_LEN           (sizeof(uint32_t))
#define S_C_TAG_LEN            (VLAN_TAG_LEN * 2)

#define GRE_HDR_LEN            (sizeof(uint32_t))
#define MPLS_HDR_LEN           (sizeof(uint32_t))

#define V4_HDR_LEN             (sizeof(struct iphdr))
#define V6_HDR_LEN             (sizeof(struct ipv6hdr))
#define V6_EXT_HDR_LEN         (sizeof(uint32_t)*2)

#define ONE_BYTE               (sizeof(uint8_t))
#define TWO_BYTE               (sizeof(uint16_t))
#define FOUR_BYTE              (sizeof(uint32_t))

#define OFFSET_TO_V4_PROTO     offsetof(struct iphdr,   protocol)
#define OFFSET_TO_V6_PROTO     offsetof(struct ipv6hdr, nexthdr)

#define OFFSET_TO_TCP_SRC_PORT offsetof(struct tcphdr,  source)
#define OFFSET_TO_TCP_DST_PORT offsetof(struct tcphdr,  dest)
#define OFFSET_TO_UDP_SRC_PORT offsetof(struct udphdr,  source)
#define OFFSET_TO_UDP_DST_PORT offsetof(struct udphdr,  dest)
#define OFFSET_TO_ETHER_TYPE   offsetof(struct ethhdr,  h_proto)

/*
 * The following are the outer and inner packet types combinations
 * we'll see for either uplink (UL below) or downlink (DL below) data
 * flows:
 *
 *   out 4/in 4 (O4_I4 below)
 *   out 6/in 6 (O6_I6 below)
 *
 *   out 4/in 6 (O4_I6 below)
 *   out 6/in 4 (O6_I4 below)
 *
 *   out eth/in 4 (OETH_I4 below)
 *   out eth/in 6 (OETH_I6 below)
 *
 * To follow are:
 *
 *    DOWNLINK PACKET OFFSET CONSTANTS
 */

/*
 * out 4/in 4 -> O4_I4
 */
#define MPLS_DL_O4_I4_OFFSET_TO_ETH \
    (GRE_HDR_LEN + MPLS_HDR_LEN)

#define MPLS_DL_O4_I4_OFFSET_TO_V4_HDR \
    (MPLS_DL_O4_I4_OFFSET_TO_ETH + ETH_HDR_LEN)

#define MPLS_DL_O4_I4_IP_PROTOCOL_OFFSET \
    (MPLS_DL_O4_I4_OFFSET_TO_V4_HDR + OFFSET_TO_V4_PROTO)
#define MPLS_DL_O4_I4_TCP_SRC_PORT_OFFSET \
    (MPLS_DL_O4_I4_OFFSET_TO_V4_HDR + V4_HDR_LEN + OFFSET_TO_TCP_SRC_PORT)
#define MPLS_DL_O4_I4_TCP_DST_PORT_OFFSET \
    (MPLS_DL_O4_I4_OFFSET_TO_V4_HDR + V4_HDR_LEN + OFFSET_TO_TCP_DST_PORT)
#define MPLS_DL_O4_I4_UDP_SRC_PORT_OFFSET \
    (MPLS_DL_O4_I4_OFFSET_TO_V4_HDR + V4_HDR_LEN + OFFSET_TO_UDP_SRC_PORT)
#define MPLS_DL_O4_I4_UDP_DST_PORT_OFFSET \
    (MPLS_DL_O4_I4_OFFSET_TO_V4_HDR + V4_HDR_LEN + OFFSET_TO_UDP_DST_PORT)

#define MPLS_DL_O4_I4_ETHER_TYPE_OFFSET \
    (MPLS_DL_O4_I4_OFFSET_TO_ETH + OFFSET_TO_ETHER_TYPE)

/*
 * out 6/in 6 -> O6_I6
 */
#define MPLS_DL_O6_I6_OFFSET_TO_ETH \
    (V6_EXT_HDR_LEN + GRE_HDR_LEN + MPLS_HDR_LEN)

#define MPLS_DL_O6_I6_OFFSET_TO_V6_HDR \
    (MPLS_DL_O6_I6_OFFSET_TO_ETH + ETH_HDR_LEN)

#define MPLS_DL_O6_I6_IP_PROTOCOL_OFFSET \
    (MPLS_DL_O6_I6_OFFSET_TO_V6_HDR + OFFSET_TO_V6_PROTO)
#define MPLS_DL_O6_I6_TCP_SRC_PORT_OFFSET \
    (MPLS_DL_O6_I6_OFFSET_TO_V6_HDR + V6_HDR_LEN + OFFSET_TO_TCP_SRC_PORT)
#define MPLS_DL_O6_I6_TCP_DST_PORT_OFFSET \
    (MPLS_DL_O6_I6_OFFSET_TO_V6_HDR + V6_HDR_LEN + OFFSET_TO_TCP_DST_PORT)
#define MPLS_DL_O6_I6_UDP_SRC_PORT_OFFSET \
    (MPLS_DL_O6_I6_OFFSET_TO_V6_HDR + V6_HDR_LEN + OFFSET_TO_UDP_SRC_PORT)
#define MPLS_DL_O6_I6_UDP_DST_PORT_OFFSET \
    (MPLS_DL_O6_I6_OFFSET_TO_V6_HDR + V6_HDR_LEN + OFFSET_TO_UDP_DST_PORT)

#define MPLS_DL_O6_I6_ETHER_TYPE_OFFSET \
    (MPLS_DL_O6_I6_OFFSET_TO_ETH + OFFSET_TO_ETHER_TYPE)

/*
 * out 4/in 6 -> O4_I6
 */
#define MPLS_DL_O4_I6_OFFSET_TO_ETH \
    (GRE_HDR_LEN + MPLS_HDR_LEN)

#define MPLS_DL_O4_I6_OFFSET_TO_V6_HDR \
    (MPLS_DL_O4_I6_OFFSET_TO_ETH + ETH_HDR_LEN)

#define MPLS_DL_O4_I6_IP_PROTOCOL_OFFSET \
    (MPLS_DL_O4_I6_OFFSET_TO_V6_HDR + OFFSET_TO_V6_PROTO)
#define MPLS_DL_O4_I6_TCP_SRC_PORT_OFFSET \
    (MPLS_DL_O4_I6_OFFSET_TO_V6_HDR + V6_HDR_LEN + OFFSET_TO_TCP_SRC_PORT)
#define MPLS_DL_O4_I6_TCP_DST_PORT_OFFSET \
    (MPLS_DL_O4_I6_OFFSET_TO_V6_HDR + V6_HDR_LEN + OFFSET_TO_TCP_DST_PORT)
#define MPLS_DL_O4_I6_UDP_SRC_PORT_OFFSET \
    (MPLS_DL_O4_I6_OFFSET_TO_V6_HDR + V6_HDR_LEN + OFFSET_TO_UDP_SRC_PORT)
#define MPLS_DL_O4_I6_UDP_DST_PORT_OFFSET \
    (MPLS_DL_O4_I6_OFFSET_TO_V6_HDR + V6_HDR_LEN + OFFSET_TO_UDP_DST_PORT)

#define MPLS_DL_O4_I6_ETHER_TYPE_OFFSET \
    (MPLS_DL_O4_I6_OFFSET_TO_ETH + OFFSET_TO_ETHER_TYPE)

/*
 * out 6/in 4 -> O6_I4
 */
#define MPLS_DL_O6_I4_OFFSET_TO_ETH \
    (V6_EXT_HDR_LEN + GRE_HDR_LEN + MPLS_HDR_LEN)

#define MPLS_DL_O6_I4_OFFSET_TO_V4_HDR \
    (MPLS_DL_O6_I4_OFFSET_TO_ETH + ETH_HDR_LEN)

#define MPLS_DL_O6_I4_IP_PROTOCOL_OFFSET \
    (MPLS_DL_O6_I4_OFFSET_TO_V4_HDR + OFFSET_TO_V4_PROTO)
#define MPLS_DL_O6_I4_TCP_SRC_PORT_OFFSET \
    (MPLS_DL_O6_I4_OFFSET_TO_V4_HDR + V4_HDR_LEN + OFFSET_TO_TCP_SRC_PORT)
#define MPLS_DL_O6_I4_TCP_DST_PORT_OFFSET \
    (MPLS_DL_O6_I4_OFFSET_TO_V4_HDR + V4_HDR_LEN + OFFSET_TO_TCP_DST_PORT)
#define MPLS_DL_O6_I4_UDP_SRC_PORT_OFFSET \
    (MPLS_DL_O6_I4_OFFSET_TO_V4_HDR + V4_HDR_LEN + OFFSET_TO_UDP_SRC_PORT)
#define MPLS_DL_O6_I4_UDP_DST_PORT_OFFSET \
    (MPLS_DL_O6_I4_OFFSET_TO_V4_HDR + V4_HDR_LEN + OFFSET_TO_UDP_DST_PORT)

#define MPLS_DL_O6_I4_ETHER_TYPE_OFFSET \
    (MPLS_DL_O6_I4_OFFSET_TO_ETH + OFFSET_TO_ETHER_TYPE)

/*
 * UPLINK PACKET OFFSET CONSTANTS
 */

/*
 * out eth/in 4 -> OETH_I4
 */
#define MPLS_UL_OETH_I4_OFFSET_TO_ETH \
    (-(V4_HDR_LEN + ETH_HDR_W_TAGS_LEN))

#define MPLS_UL_OETH_I4_OFFSET_TO_V4_HDR \
    (-V4_HDR_LEN)

#define MPLS_UL_OETH_I4_IP_PROTOCOL_OFFSET \
    (MPLS_UL_OETH_I4_OFFSET_TO_V4_HDR + OFFSET_TO_V4_PROTO)
#define MPLS_UL_OETH_I4_TCP_SRC_PORT_OFFSET \
    (MPLS_UL_OETH_I4_OFFSET_TO_V4_HDR + V4_HDR_LEN + OFFSET_TO_TCP_SRC_PORT)
#define MPLS_UL_OETH_I4_TCP_DST_PORT_OFFSET \
    (MPLS_UL_OETH_I4_OFFSET_TO_V4_HDR + V4_HDR_LEN + OFFSET_TO_TCP_DST_PORT)
#define MPLS_UL_OETH_I4_UDP_SRC_PORT_OFFSET \
    (MPLS_UL_OETH_I4_OFFSET_TO_V4_HDR + V4_HDR_LEN + OFFSET_TO_UDP_SRC_PORT)
#define MPLS_UL_OETH_I4_UDP_DST_PORT_OFFSET \
    (MPLS_UL_OETH_I4_OFFSET_TO_V4_HDR + V4_HDR_LEN + OFFSET_TO_UDP_DST_PORT)

#define MPLS_UL_OETH_I4_ETHER_TYPE_OFFSET \
    (MPLS_UL_OETH_I4_OFFSET_TO_ETH + OFFSET_TO_ETHER_TYPE + S_C_TAG_LEN)

/*
 * out eth/in 6 -> OETH_I6
 */
#define MPLS_UL_OETH_I6_OFFSET_TO_ETH \
    (-(V6_HDR_LEN + ETH_HDR_W_TAGS_LEN))

#define MPLS_UL_OETH_I6_OFFSET_TO_V6_HDR \
    (-V6_HDR_LEN)

#define MPLS_UL_OETH_I6_IP_PROTOCOL_OFFSET \
    (MPLS_UL_OETH_I6_OFFSET_TO_V6_HDR + OFFSET_TO_V6_PROTO)
#define MPLS_UL_OETH_I6_TCP_SRC_PORT_OFFSET \
    (MPLS_UL_OETH_I6_OFFSET_TO_V6_HDR + V6_HDR_LEN + OFFSET_TO_TCP_SRC_PORT)
#define MPLS_UL_OETH_I6_TCP_DST_PORT_OFFSET \
    (MPLS_UL_OETH_I6_OFFSET_TO_V6_HDR + V6_HDR_LEN + OFFSET_TO_TCP_DST_PORT)
#define MPLS_UL_OETH_I6_UDP_SRC_PORT_OFFSET \
    (MPLS_UL_OETH_I6_OFFSET_TO_V6_HDR + V6_HDR_LEN + OFFSET_TO_UDP_SRC_PORT)
#define MPLS_UL_OETH_I6_UDP_DST_PORT_OFFSET \
    (MPLS_UL_OETH_I6_OFFSET_TO_V6_HDR + V6_HDR_LEN + OFFSET_TO_UDP_DST_PORT)

#define MPLS_UL_OETH_I6_ETHER_TYPE_OFFSET \
    (MPLS_UL_OETH_I6_OFFSET_TO_ETH + OFFSET_TO_ETHER_TYPE + S_C_TAG_LEN)

/*
 * The following structure definition is used to define width and
 * offset tables. These tables are used for mpls over gre equation
 * generation.
 */
typedef struct {
    uint8_t width;
    int8_t  offset;
} ipa_fld_wid_off_t;

ipa_fld_wid_off_t* get_mpls_v4_outer(enum ipa_data_flow_type, enum ipa_ip_type, enum ipa_exception_type);

ipa_fld_wid_off_t* get_mpls_v6_outer(enum ipa_data_flow_type, enum ipa_ip_type, enum ipa_exception_type);

#endif /* IPA_FLT_EXT_MPLS_GRE_GENERAL */
#endif /* _IPAHAL_FLTRT_H_ */
