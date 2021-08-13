/* SPDX-License-Identifier: GPL-2.0-only */
/*
* Copyright (c) 2030, The Linux Foundation. All rights reserved.
*/

#ifndef _GSIHAL_REG_H_
#define _GSIHAL_REG_H_

/*
 * Registers names
 *
 * NOTE:: Any change to this enum, need to change to gsireg_name_to_str
 *	array as well.
 */
enum gsihal_reg_name {
	GSI_EE_n_CNTXT_TYPE_IRQ_MSK,
	GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK,
	GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK,
	GSI_EE_n_CNTXT_SRC_IEOB_IRQ_MSK,
	GSI_EE_n_CNTXT_GLOB_IRQ_EN,
	GSI_EE_n_CNTXT_GSI_IRQ_EN,
	GSI_EE_n_CNTXT_TYPE_IRQ,
	GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ,
	GSI_EE_n_GSI_CH_k_CNTXT_0,
	GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR,
	GSI_EE_n_CNTXT_SRC_EV_CH_IRQ,
	GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR,
	GSI_EE_n_EV_CH_k_CNTXT_0,
	GSI_EE_n_CNTXT_GLOB_IRQ_STTS,
	GSI_EE_n_ERROR_LOG,
	GSI_EE_n_ERROR_LOG_CLR,
	GSI_EE_n_CNTXT_GLOB_IRQ_CLR,
	GSI_EE_n_EV_CH_k_DOORBELL_0,
	GSI_EE_n_GSI_CH_k_DOORBELL_0,
	GSI_EE_n_CNTXT_SRC_IEOB_IRQ,
	GSI_EE_n_CNTXT_SRC_IEOB_IRQ_CLR,
	GSI_INTER_EE_n_SRC_GSI_CH_IRQ,
	GSI_INTER_EE_n_SRC_GSI_CH_IRQ_CLR,
	GSI_INTER_EE_n_SRC_EV_CH_IRQ,
	GSI_INTER_EE_n_SRC_EV_CH_IRQ_CLR,
	GSI_EE_n_CNTXT_GSI_IRQ_STTS,
	GSI_EE_n_CNTXT_GSI_IRQ_CLR,
	GSI_EE_n_GSI_HW_PARAM,
	GSI_EE_n_GSI_HW_PARAM_0,
	GSI_EE_n_GSI_HW_PARAM_2,
	GSI_EE_n_GSI_HW_PARAM_4,
	GSI_EE_n_GSI_SW_VERSION,
	GSI_EE_n_CNTXT_INTSET,
	GSI_EE_n_CNTXT_MSI_BASE_LSB,
	GSI_EE_n_CNTXT_MSI_BASE_MSB,
	GSI_EE_n_GSI_STATUS,
	GSI_EE_n_CNTXT_SCRATCH_0,
	GSI_EE_n_EV_CH_k_CNTXT_1,
	GSI_EE_n_EV_CH_k_CNTXT_2,
	GSI_EE_n_EV_CH_k_CNTXT_3,
	GSI_EE_n_EV_CH_k_CNTXT_8,
	GSI_EE_n_EV_CH_k_CNTXT_9,
	GSI_EE_n_EV_CH_k_CNTXT_10,
	GSI_EE_n_EV_CH_k_CNTXT_11,
	GSI_EE_n_EV_CH_k_CNTXT_12,
	GSI_EE_n_EV_CH_k_CNTXT_13,
	GSI_EE_n_EV_CH_k_DOORBELL_1,
	GSI_EE_n_EV_CH_CMD,
	GSI_EE_n_EV_CH_k_SCRATCH_0,
	GSI_EE_n_EV_CH_k_SCRATCH_1,
	GSI_EE_n_GSI_CH_k_DOORBELL_1,
	GSI_EE_n_GSI_CH_k_QOS,
	GSI_EE_n_GSI_CH_k_CNTXT_1,
	GSI_EE_n_GSI_CH_k_CNTXT_2,
	GSI_EE_n_GSI_CH_k_CNTXT_3,
	GSI_EE_n_GSI_CH_CMD,
	GSI_EE_n_GSI_CH_k_SCRATCH_0,
	GSI_EE_n_GSI_CH_k_SCRATCH_1,
	GSI_EE_n_GSI_CH_k_SCRATCH_2,
	GSI_EE_n_GSI_CH_k_SCRATCH_3,
	GSI_EE_n_GSI_CH_k_SCRATCH_4,
	GSI_EE_n_GSI_CH_k_SCRATCH_5,
	GSI_EE_n_GSI_CH_k_SCRATCH_6,
	GSI_EE_n_GSI_CH_k_SCRATCH_7,
	GSI_EE_n_GSI_CH_k_SCRATCH_8,
	GSI_EE_n_GSI_CH_k_SCRATCH_9,
	GSI_EE_n_GSI_CH_k_CNTXT_4,
	GSI_EE_n_GSI_CH_k_CNTXT_5,
	GSI_EE_n_GSI_CH_k_CNTXT_6,
	GSI_EE_n_GSI_CH_k_CNTXT_7,
	GSI_EE_n_GSI_CH_k_CNTXT_8,
	GSI_EE_n_EV_CH_k_CNTXT_4,
	GSI_EE_n_EV_CH_k_CNTXT_5,
	GSI_EE_n_EV_CH_k_CNTXT_6,
	GSI_EE_n_EV_CH_k_CNTXT_7,
	GSI_GSI_IRAM_PTR_CH_CMD,
	GSI_GSI_IRAM_PTR_CH_DB,
	GSI_GSI_IRAM_PTR_CH_DIS_COMP,
	GSI_GSI_IRAM_PTR_CH_EMPTY,
	GSI_GSI_IRAM_PTR_EE_GENERIC_CMD,
	GSI_GSI_IRAM_PTR_EVENT_GEN_COMP,
	GSI_GSI_IRAM_PTR_INT_MOD_STOPPED,
	GSI_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0,
	GSI_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2,
	GSI_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1,
	GSI_GSI_IRAM_PTR_NEW_RE,
	GSI_GSI_IRAM_PTR_READ_ENG_COMP,
	GSI_GSI_IRAM_PTR_TIMER_EXPIRED,
	GSI_GSI_IRAM_PTR_EV_DB,
	GSI_GSI_IRAM_PTR_UC_GP_INT,
	GSI_GSI_IRAM_PTR_WRITE_ENG_COMP,
	GSI_GSI_IRAM_PTR_TLV_CH_NOT_FULL,
	GSI_IC_DISABLE_CHNL_BCK_PRS_LSB,
	GSI_IC_DISABLE_CHNL_BCK_PRS_MSB,
	GSI_IC_GEN_EVNT_BCK_PRS_LSB,
	GSI_IC_GEN_EVNT_BCK_PRS_MSB,
	GSI_IC_GEN_INT_BCK_PRS_LSB,
	GSI_IC_GEN_INT_BCK_PRS_MSB,
	GSI_IC_STOP_INT_MOD_BCK_PRS_LSB,
	GSI_IC_STOP_INT_MOD_BCK_PRS_MSB,
	GSI_IC_PROCESS_DESC_BCK_PRS_LSB,
	GSI_IC_PROCESS_DESC_BCK_PRS_MSB,
	GSI_IC_TLV_STOP_BCK_PRS_LSB,
	GSI_IC_TLV_STOP_BCK_PRS_MSB,
	GSI_IC_TLV_RESET_BCK_PRS_LSB,
	GSI_IC_TLV_RESET_BCK_PRS_MSB,
	GSI_IC_RGSTR_TIMER_BCK_PRS_LSB,
	GSI_IC_RGSTR_TIMER_BCK_PRS_MSB,
	GSI_IC_READ_BCK_PRS_LSB,
	GSI_IC_READ_BCK_PRS_MSB,
	GSI_IC_WRITE_BCK_PRS_LSB,
	GSI_IC_WRITE_BCK_PRS_MSB,
	GSI_IC_UCONTROLLER_GPR_BCK_PRS_LSB,
	GSI_IC_UCONTROLLER_GPR_BCK_PRS_MSB,
	GSI_GSI_PERIPH_BASE_ADDR_MSB,
	GSI_GSI_PERIPH_BASE_ADDR_LSB,
	GSI_GSI_MCS_CFG,
	GSI_GSI_CFG,
	GSI_EE_n_GSI_EE_GENERIC_CMD,
	GSI_MAP_EE_n_CH_k_VP_TABLE,
	GSI_EE_n_GSI_CH_k_RE_FETCH_READ_PTR,
	GSI_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR,
	GSI_GSI_INST_RAM_n,
	GSI_GSI_IRAM_PTR_MSI_DB,
	GSI_GSI_IRAM_PTR_INT_NOTIFY_MCS,
	GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_k,
	GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_k,
	GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k,
	GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k,
	GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k,
	GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k,
	GSI_EE_n_CNTXT_SRC_IEOB_IRQ_k,
	GSI_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k,
	GSI_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k,
	GSI_INTER_EE_n_SRC_GSI_CH_IRQ_k,
	GSI_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k,
	GSI_INTER_EE_n_SRC_EV_CH_IRQ_k,
	GSI_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k,
	GSI_GSI_SHRAM_n,
	GSI_GSI_MCS_PROFILING_BP_CNT_LSB,
	GSI_GSI_MCS_PROFILING_BP_CNT_MSB,
	GSI_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_LSB,
	GSI_GSI_MCS_PROFILING_BP_AND_PENDING_CNT_MSB,
	GSI_GSI_MCS_PROFILING_MCS_BUSY_CNT_LSB,
	GSI_GSI_MCS_PROFILING_MCS_BUSY_CNT_MSB,
	GSI_GSI_MCS_PROFILING_MCS_IDLE_CNT_LSB,
	GSI_GSI_MCS_PROFILING_MCS_IDLE_CNT_MSB,
	GSI_EE_n_CH_k_CH_ALMST_EMPTY_THRSHOLD,
	GSI_EE_n_GSI_DEBUG_PC_FOR_DEBUG,
	GSI_EE_n_GSI_DEBUG_BUSY_REG,
	GSI_REG_MAX
};

struct gsihal_reg_ctx_type_irq {
	uint32_t general;
	uint32_t inter_ee_ev_ctrl;
	uint32_t inter_ee_ch_ctrl;
	uint32_t ieob;
	uint32_t glob_ee;
	uint32_t ev_ctrl;
	uint32_t ch_ctrl;
};

struct gsihal_reg_ch_k_cntxt_0 {
	uint32_t element_size;
	uint32_t chstate;
	uint32_t erindex;
	uint32_t chtype_protocol_msb;
	uint32_t chid;
	uint32_t ee;
	uint32_t chtype_dir;
	uint32_t chtype_protocol;
};

struct gsihal_reg_cntxt_glob_irq_stts {
	uint8_t gp_int3;
	uint8_t gp_int2;
	uint8_t gp_int1;
	uint8_t error_int;
};

struct gsihal_reg_cntxt_gsi_irq_stts {
	uint8_t gsi_mcs_stack_ovrflow;
	uint8_t gsi_cmd_fifo_ovrflow;
	uint8_t gsi_bus_error;
	uint8_t gsi_break_point;
};

struct gsihal_reg_hw_param {
	uint32_t periph_sec_grp;
	uint32_t use_axi_m;
	uint32_t periph_conf_addr_bus_w;
	uint32_t num_ees;
	uint32_t gsi_ch_num;
	uint32_t gsi_ev_ch_num;
};

struct gsihal_reg_hw_param2 {
	uint32_t gsi_use_inter_ee;
	uint32_t gsi_use_rd_wr_eng;
	uint32_t gsi_sdma_n_iovec;
	uint32_t gsi_sdma_max_burst;
	uint32_t gsi_sdma_n_int;
	uint32_t gsi_use_sdma;
	uint32_t gsi_ch_full_logic;
	uint32_t gsi_ch_pend_translate;
	uint32_t gsi_num_ev_per_ee;
	uint32_t gsi_num_ch_per_ee;
	uint32_t gsi_iram_size;
};

struct gsihal_reg_hw_param4 {
	uint32_t gsi_iram_protcol_cnt;
	uint32_t gsi_num_ev_per_ee;
};

struct gsihal_reg_gsi_status {
	uint8_t enabled;
};

struct gsihal_reg_ev_ch_k_cntxt_0 {
	uint32_t element_size;
	uint32_t chstate;
	uint32_t intype;
	uint32_t evchid;
	uint32_t ee;
	uint32_t chtype;

};
struct gsihal_reg_ev_ch_k_cntxt_1 {
	uint32_t r_length;
};

struct gsihal_reg_ev_ch_k_cntxt_2 {
	uint32_t r_base_addr_lsbs;
};

struct gsihal_reg_ev_ch_k_cntxt_3 {
	uint32_t r_base_addr_msbs;
};

struct gsihal_reg_ev_ch_k_cntxt_8 {
	uint32_t int_mod_cnt;
	uint32_t int_modc;
	uint32_t int_modt;
};

struct gsihal_reg_ev_ch_k_cntxt_9 {
	uint32_t intvec;
};

struct gsihal_reg_ev_ch_k_cntxt_10 {
	uint32_t msi_addr_lsb;
};

struct gsihal_reg_ev_ch_k_cntxt_11 {
	uint32_t msi_addr_msb;
};

struct gsihal_reg_ev_ch_k_cntxt_12 {
	uint32_t rp_update_addr_lsb;
};

struct gsihal_reg_ev_ch_k_cntxt_13 {
	uint32_t rp_update_addr_msb;
};

struct gsihal_reg_gsi_ee_n_ev_ch_k_doorbell_1 {
	uint32_t write_ptr_msb;
};

struct gsihal_reg_ee_n_ev_ch_cmd {
	uint32_t opcode;
	uint32_t chid;
};

struct gsihal_reg_ee_n_gsi_ch_cmd {
	uint32_t opcode;
	uint32_t chid;
};

struct gsihal_reg_gsi_ee_n_gsi_ch_k_qos {
	uint32_t low_latency_en; //3.0
	uint32_t db_in_bytes; //2.9
	uint32_t empty_lvl_thrshold;
	uint32_t prefetch_mode;
	uint32_t use_escape_buf_only; //stringray
	uint32_t use_db_eng; //mclaren
	uint32_t max_prefetch;
	uint32_t wrr_weight;
};

struct gsihal_reg_ch_k_cntxt_1 {
	uint32_t r_length;
	uint32_t erindex;
};

struct gsihal_reg_gsi_cfg {
	uint32_t sleep_clk_div;
	uint32_t bp_mtrix_disable;
	uint32_t gsi_pwr_clps;
	uint32_t uc_is_mcs;
	uint32_t double_mcs_clk_freq;
	uint32_t mcs_enable;
	uint32_t gsi_enable;
};

struct gsihal_reg_gsi_ee_generic_cmd {
	uint32_t opcode;
	uint32_t virt_chan_idx;
	uint32_t ee;
	bool prmy_scnd_fc;
};

struct gsihal_reg_gsi_ee_n_cntxt_gsi_irq {
	uint8_t gsi_mcs_stack_ovrflow;
	uint8_t gsi_cmd_fifo_ovrflow;
	uint8_t gsi_bus_error;
	uint8_t gsi_break_point;
};

/*
 * gsihal_reg_init() - intialize gsihal regsiters module
 */
int gsihal_reg_init(enum gsi_ver gsi_ver);

/*
 * gsihal_read_reg_nk() - Get nk parameterized reg value
 */
u32 gsihal_read_reg_nk(enum gsihal_reg_name reg, u32 n, u32 k);

/*
 * gsihal_read_reg_n() - Get n parameterized reg value
 */
static inline u32 gsihal_read_reg_n(enum gsihal_reg_name reg, u32 n)
{
	return gsihal_read_reg_nk(reg, n, 0);
}

/*
 * gsihal_read_reg() - Get reg value
 */
static inline u32 gsihal_read_reg(enum gsihal_reg_name reg)
{
	return gsihal_read_reg_nk(reg, 0, 0);
}

/*
 * gsihal_write_reg_nk() - Write to n/k parameterized reg a raw value
 */
void gsihal_write_reg_nk(enum gsihal_reg_name reg, u32 n, u32 k, u32 val);

/*
 * gsihal_write_reg_n() - Write to n parameterized reg a raw value
 */
static inline void gsihal_write_reg_n(enum gsihal_reg_name reg, u32 n, u32 val)
{
	gsihal_write_reg_nk(reg, n, 0, val);
}

/*
 * gsihal_write_reg() - Write to reg a raw value
 */
static inline void gsihal_write_reg(enum gsihal_reg_name reg, u32 val)
{
	gsihal_write_reg_nk(reg, 0, 0, val);
}

/*
 * gsihal_read_reg_nk_fields() - Get the parsed value of nk parameterized reg
 */
u32 gsihal_read_reg_nk_fields(enum gsihal_reg_name reg,
	u32 n, u32 k, void *fields);

/*
 * gsihal_write_reg_nk_fields() - Write to nk parameterized reg a prased value
 */
void gsihal_write_reg_nk_fields(enum gsihal_reg_name reg, u32 n, u32 k,
	const void *fields);

/*
* gsihal_read_reg_n_fields() - Get the parsed value of n parameterized reg
*/
u32 gsihal_read_reg_n_fields(enum gsihal_reg_name reg, u32 n, void *fields);

/*
 * gsihal_write_reg_n_fields() - Write to n parameterized reg a prased value
 */
void gsihal_write_reg_n_fields(enum gsihal_reg_name reg, u32 n,
	const void *fields);

/*
 * gsihal_write_reg_fields() - Write to reg a prased value
 */
void gsihal_write_reg_fields(enum gsihal_reg_name reg, const void *fields);

/*
 * gsihal_read_reg_fields() - Get the parsed value of reg
 */
u32 gsihal_read_reg_fields(enum gsihal_reg_name reg, void *fields);

/*
* gsihal_get_bit_map_array_size() - Get the size of the bit map
*	array size according to the
*	GSI version.
*/
u32 gsihal_get_bit_map_array_size(void);

/*
* gsihal_read_ch_reg() - Get the raw value of a ch reg
*/
u32 gsihal_read_ch_reg(enum gsihal_reg_name reg, u32 ch_num);

/*
 * gsihal_test_ch_bit() - return true if a ch bit is set
 */
bool gsihal_test_ch_bit(u32 reg_val, u32 ch_num);

/*
 * gsihal_get_ch_bit() - get ch bit set in the right offset
 */
u32 gsihal_get_ch_bit(u32 ch_num);

/*
 * gsihal_get_ch_reg_idx() - get ch reg index according to ch num
 */
u32 gsihal_get_ch_reg_idx(u32 ch_num);

/*
 * gsihal_get_ch_reg_mask() - get ch reg mask according to ch num
 */
u32 gsihal_get_ch_reg_mask(u32 ch_num);

/*
 * gsihal_get_ch_reg_offset() - Get the offset of a ch register according to
 *	ch index
 */
u32 gsihal_get_ch_reg_offset(enum gsihal_reg_name reg, u32 ch_num);

/*
 * gsihal_get_ch_reg_n_offset() - Get the offset of a ch n register according
 *	to ch index and n
 */
u32 gsihal_get_ch_reg_n_offset(enum gsihal_reg_name reg, u32 n, u32 ch_num);

/*
 * gsihal_write_ch_bit_map_reg_n() - Write mask to ch reg a raw value
 */
void gsihal_write_ch_bit_map_reg_n(enum gsihal_reg_name reg, u32 n, u32 ch_num,
	u32 mask);

/*
 * gsihal_write_set_ch_bit_map_reg_n() - Set ch bit in reg a raw value
 */
void gsihal_write_set_ch_bit_map_reg_n(enum gsihal_reg_name reg, u32 n,
	u32 ch_num);

/*
 * Get the offset of a nk parameterized register
 */
u32 gsihal_get_reg_nk_ofst(enum gsihal_reg_name reg, u32 n, u32 k);

/*
 * Check that ring length is valid
 */
bool gsihal_check_ring_length_valid(u32 r_len, u32 elem_size);

/*
 * Get the offset of a n parameterized register
 */
static inline u32 gsihal_get_reg_n_ofst(enum gsihal_reg_name reg, u32 n)
{
	return gsihal_get_reg_nk_ofst(reg, n, 0);
}

/*
 * Get the offset of a register
 */
static inline u32 gsihal_get_reg_ofst(enum gsihal_reg_name reg)
{
	return gsihal_get_reg_nk_ofst(reg, 0, 0);
}

/*
 * Get GSI instruction ram MAX size
 */
unsigned long gsihal_get_inst_ram_size(void);

/*
 * Get mask for GP_int1
 */
u32 gsihal_get_glob_irq_en_gp_int1_mask(void);

#endif /* _GSIHAL_REG_H_ */
