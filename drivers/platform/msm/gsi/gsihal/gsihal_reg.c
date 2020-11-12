// SPDX-License-Identifier: GPL-2.0-only
/*
* Copyright (c) 2020, The Linux Foundation. All rights reserved.
*/

#include "gsihal_i.h"
#include "gsihal_reg_i.h"
#include "gsihal_reg.h"

/*
*	GSI_CH_BIT_MAP_ARR_SIZE - The number of cells in bit map
*		registers for GSI channels.
*		This is taken from the k range
*		of each bit map register.
*/
#define GSI_CH_BIT_MAP_ARR_SIZE			(1)

/* Utility macros to use with bit map arrays*/
#define GSI_CH_BIT_MAP_CELL_NUM(num)	((num) >> 5)
#define GSI_BIT_MAP_CELL_MSK(num)		(1 << (num - (GSI_CH_BIT_MAP_CELL_NUM(num) << 5)))

#define gsi_readl(c)	(readl_relaxed(c))
#define gsi_writel(v, c)	({ __iowmb(); writel_relaxed((v), (c)); })

static const char *gsireg_name_to_str[GSI_REG_MAX] = {
	__stringify(GSI_EE_n_CNTXT_TYPE_IRQ_MSK),
	__stringify(GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK),
	__stringify(GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK),
	__stringify(GSI_EE_n_CNTXT_SRC_IEOB_IRQ_MSK),
	__stringify(GSI_EE_n_CNTXT_GLOB_IRQ_EN),
	__stringify(GSI_EE_n_CNTXT_GSI_IRQ_EN),
	__stringify(GSI_EE_n_CNTXT_TYPE_IRQ),
	__stringify(GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ),
	__stringify(GSI_EE_n_GSI_CH_k_CNTXT_0),
	__stringify(GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR),
	__stringify(GSI_EE_n_CNTXT_SRC_EV_CH_IRQ),
	__stringify(GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_0),
	__stringify(GSI_EE_n_CNTXT_GLOB_IRQ_STTS),
	__stringify(GSI_EE_n_ERROR_LOG),
	__stringify(GSI_EE_n_ERROR_LOG_CLR),
	__stringify(GSI_EE_n_CNTXT_GLOB_IRQ_CLR),
	__stringify(GSI_EE_n_EV_CH_k_DOORBELL_0),
	__stringify(GSI_EE_n_GSI_CH_k_DOORBELL_0),
	__stringify(GSI_EE_n_CNTXT_SRC_IEOB_IRQ),
	__stringify(GSI_EE_n_CNTXT_SRC_IEOB_IRQ_CLR),
	__stringify(GSI_INTER_EE_n_SRC_GSI_CH_IRQ),
	__stringify(GSI_INTER_EE_n_SRC_GSI_CH_IRQ_CLR),
	__stringify(GSI_INTER_EE_n_SRC_EV_CH_IRQ),
	__stringify(GSI_INTER_EE_n_SRC_EV_CH_IRQ_CLR),
	__stringify(GSI_EE_n_CNTXT_GSI_IRQ_STTS),
	__stringify(GSI_EE_n_CNTXT_GSI_IRQ_CLR),
	__stringify(GSI_EE_n_GSI_HW_PARAM),
	__stringify(GSI_EE_n_GSI_HW_PARAM_0),
	__stringify(GSI_EE_n_GSI_HW_PARAM_2),
	__stringify(GSI_EE_n_GSI_HW_PARAM_4),
	__stringify(GSI_EE_n_GSI_SW_VERSION),
	__stringify(GSI_EE_n_CNTXT_INTSET),
	__stringify(GSI_EE_n_CNTXT_MSI_BASE_LSB),
	__stringify(GSI_EE_n_CNTXT_MSI_BASE_MSB),
	__stringify(GSI_EE_n_GSI_STATUS),
	__stringify(GSI_EE_n_CNTXT_SCRATCH_0),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_1),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_2),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_3),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_8),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_9),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_10),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_11),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_12),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_13),
	__stringify(GSI_EE_n_EV_CH_k_DOORBELL_1),
	__stringify(GSI_EE_n_EV_CH_CMD),
	__stringify(GSI_EE_n_EV_CH_k_SCRATCH_0),
	__stringify(GSI_EE_n_EV_CH_k_SCRATCH_1),
	__stringify(GSI_EE_n_GSI_CH_k_DOORBELL_1),
	__stringify(GSI_EE_n_GSI_CH_k_QOS),
	__stringify(GSI_EE_n_GSI_CH_k_CNTXT_1),
	__stringify(GSI_EE_n_GSI_CH_k_CNTXT_2),
	__stringify(GSI_EE_n_GSI_CH_k_CNTXT_3),
	__stringify(GSI_EE_n_GSI_CH_CMD),
	__stringify(GSI_EE_n_GSI_CH_k_SCRATCH_0),
	__stringify(GSI_EE_n_GSI_CH_k_SCRATCH_1),
	__stringify(GSI_EE_n_GSI_CH_k_SCRATCH_2),
	__stringify(GSI_EE_n_GSI_CH_k_SCRATCH_3),
	__stringify(GSI_EE_n_GSI_CH_k_SCRATCH_4),
	__stringify(GSI_EE_n_GSI_CH_k_SCRATCH_5),
	__stringify(GSI_EE_n_GSI_CH_k_SCRATCH_6),
	__stringify(GSI_EE_n_GSI_CH_k_SCRATCH_7),
	__stringify(GSI_EE_n_GSI_CH_k_SCRATCH_8),
	__stringify(GSI_EE_n_GSI_CH_k_SCRATCH_9),
	__stringify(GSI_EE_n_GSI_CH_k_CNTXT_4),
	__stringify(GSI_EE_n_GSI_CH_k_CNTXT_5),
	__stringify(GSI_EE_n_GSI_CH_k_CNTXT_6),
	__stringify(GSI_EE_n_GSI_CH_k_CNTXT_7),
	__stringify(GSI_EE_n_GSI_CH_k_CNTXT_8),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_4),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_5),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_6),
	__stringify(GSI_EE_n_EV_CH_k_CNTXT_7),
	__stringify(GSI_GSI_IRAM_PTR_CH_CMD),
	__stringify(GSI_GSI_IRAM_PTR_CH_DB),
	__stringify(GSI_GSI_IRAM_PTR_CH_DIS_COMP),
	__stringify(GSI_GSI_IRAM_PTR_CH_EMPTY),
	__stringify(GSI_GSI_IRAM_PTR_EE_GENERIC_CMD),
	__stringify(GSI_GSI_IRAM_PTR_EVENT_GEN_COMP),
	__stringify(GSI_GSI_IRAM_PTR_INT_MOD_STOPPED),
	__stringify(GSI_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0),
	__stringify(GSI_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2),
	__stringify(GSI_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1),
	__stringify(GSI_GSI_IRAM_PTR_NEW_RE),
	__stringify(GSI_GSI_IRAM_PTR_READ_ENG_COMP),
	__stringify(GSI_GSI_IRAM_PTR_TIMER_EXPIRED),
	__stringify(GSI_GSI_IRAM_PTR_EV_DB),
	__stringify(GSI_GSI_IRAM_PTR_UC_GP_INT),
	__stringify(GSI_GSI_IRAM_PTR_WRITE_ENG_COMP),
	__stringify(GSI_GSI_IRAM_PTR_TLV_CH_NOT_FULL),
	__stringify(GSI_IC_DISABLE_CHNL_BCK_PRS_LSB),
	__stringify(GSI_IC_DISABLE_CHNL_BCK_PRS_MSB),
	__stringify(GSI_IC_GEN_EVNT_BCK_PRS_LSB),
	__stringify(GSI_IC_GEN_EVNT_BCK_PRS_MSB),
	__stringify(GSI_IC_GEN_INT_BCK_PRS_LSB),
	__stringify(GSI_IC_GEN_INT_BCK_PRS_MSB),
	__stringify(GSI_IC_STOP_INT_MOD_BCK_PRS_LSB),
	__stringify(GSI_IC_STOP_INT_MOD_BCK_PRS_MSB),
	__stringify(GSI_IC_PROCESS_DESC_BCK_PRS_LSB),
	__stringify(GSI_IC_PROCESS_DESC_BCK_PRS_MSB),
	__stringify(GSI_IC_TLV_STOP_BCK_PRS_LSB),
	__stringify(GSI_IC_TLV_STOP_BCK_PRS_MSB),
	__stringify(GSI_IC_TLV_RESET_BCK_PRS_LSB),
	__stringify(GSI_IC_TLV_RESET_BCK_PRS_MSB),
	__stringify(GSI_IC_RGSTR_TIMER_BCK_PRS_LSB),
	__stringify(GSI_IC_RGSTR_TIMER_BCK_PRS_MSB),
	__stringify(GSI_IC_READ_BCK_PRS_LSB),
	__stringify(GSI_IC_READ_BCK_PRS_MSB),
	__stringify(GSI_IC_WRITE_BCK_PRS_LSB),
	__stringify(GSI_IC_WRITE_BCK_PRS_MSB),
	__stringify(GSI_IC_UCONTROLLER_GPR_BCK_PRS_LSB),
	__stringify(GSI_IC_UCONTROLLER_GPR_BCK_PRS_MSB),
	__stringify(GSI_GSI_PERIPH_BASE_ADDR_MSB),
	__stringify(GSI_GSI_PERIPH_BASE_ADDR_LSB),
	__stringify(GSI_GSI_MCS_CFG),
	__stringify(GSI_GSI_CFG),
	__stringify(GSI_EE_n_GSI_EE_GENERIC_CMD),
	__stringify(GSI_MAP_EE_n_CH_k_VP_TABLE),
	__stringify(GSI_EE_n_GSI_CH_k_RE_FETCH_READ_PTR),
	__stringify(GSI_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR),
	__stringify(GSI_GSI_INST_RAM_n),
	__stringify(GSI_GSI_IRAM_PTR_MSI_DB),
	__stringify(GSI_GSI_IRAM_PTR_INT_NOTIFY_MCS),
	__stringify(GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_k),
	__stringify(GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_k),
	__stringify(GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k),
	__stringify(GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k),
	__stringify(GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k),
	__stringify(GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k),
	__stringify(GSI_EE_n_CNTXT_SRC_IEOB_IRQ_k),
	__stringify(GSI_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k),
	__stringify(GSI_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k),
	__stringify(GSI_INTER_EE_n_SRC_GSI_CH_IRQ_k),
	__stringify(GSI_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k),
	__stringify(GSI_INTER_EE_n_SRC_EV_CH_IRQ_k),
	__stringify(GSI_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k),
	__stringify(GSI_GSI_SHRAM_n),
};

/*
* gsihal_reg_name_str() - returns string that represent the register
* @reg_name: [in] register name
*/
const char *gsihal_reg_name_str(enum gsihal_reg_name reg_name)
{
	if (reg_name < 0 || reg_name >= GSI_REG_MAX) {
		GSIERR("requested name of invalid reg=%d\n", reg_name);
		return "Invalid Register";
	}

	return gsireg_name_to_str[reg_name];
}

static void gsireg_construct_dummy(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	GSIERR("No construct function for %s\n",
		gsihal_reg_name_str(reg));
	WARN(1, "invalid register operation");
}

static void gsireg_parse_dummy(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	GSIERR("No parse function for %s\n",
		gsihal_reg_name_str(reg));
	WARN(1, "invalid register operation");

}

/*
* struct gsihal_reg_obj - Register H/W information for specific GSI version
* @construct - CB to construct register value from abstracted structure
* @parse - CB to parse register value to abstracted structure
* @offset - register offset relative to base address
* @n_ofst - N parameterized register sub-offset
* @k_ofst - K parameterized register sub-offset
*/
struct gsihal_reg_obj {
	void(*construct)(enum gsihal_reg_name reg, const void *fields,
		u32 *val);
	void(*parse)(enum gsihal_reg_name reg, void *fields,
		u32 val);
	u32 offset;
	u32 n_ofst;
	u32 k_ofst;
};

static void gsireg_parse_ctx_type_irq(enum gsihal_reg_name reg, void *fields,
	u32 val)
{
	struct gsihal_reg_ctx_type_irq *ctx = (struct gsihal_reg_ctx_type_irq *)fields;

	ctx->ch_ctrl = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_TYPE_IRQ_CH_CTRL_SHFT,
		GSI_EE_n_CNTXT_TYPE_IRQ_CH_CTRL_BMSK);
	ctx->ev_ctrl = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_TYPE_IRQ_EV_CTRL_SHFT,
		GSI_EE_n_CNTXT_TYPE_IRQ_EV_CTRL_BMSK);
	ctx->glob_ee = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_TYPE_IRQ_GLOB_EE_SHFT,
		GSI_EE_n_CNTXT_TYPE_IRQ_GLOB_EE_BMSK);
	ctx->ieob = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_TYPE_IRQ_IEOB_SHFT,
		GSI_EE_n_CNTXT_TYPE_IRQ_IEOB_BMSK);
	ctx->inter_ee_ch_ctrl = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_TYPE_IRQ_INTER_EE_CH_CTRL_SHFT,
		GSI_EE_n_CNTXT_TYPE_IRQ_INTER_EE_CH_CTRL_BMSK);
	ctx->inter_ee_ev_ctrl = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_TYPE_IRQ_INTER_EE_EV_CTRL_SHFT,
		GSI_EE_n_CNTXT_TYPE_IRQ_INTER_EE_EV_CTRL_BMSK);
	ctx->general = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_TYPE_IRQ_GENERAL_SHFT,
		GSI_EE_n_CNTXT_TYPE_IRQ_GENERAL_BMSK);
}

static void gsireg_parse_ch_k_cntxt_0(enum gsihal_reg_name reg, void *fields,
	u32 val)
{
	struct gsihal_reg_ch_k_cntxt_0 *ch_k_ctxt =
		(struct gsihal_reg_ch_k_cntxt_0 *) fields;

	ch_k_ctxt->element_size = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_GSI_CH_k_CNTXT_0_ELEMENT_SIZE_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_ELEMENT_SIZE_BMSK);
	ch_k_ctxt->chstate = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_BMSK);
	ch_k_ctxt->erindex = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_GSI_CH_k_CNTXT_0_ERINDEX_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_ERINDEX_BMSK);
	ch_k_ctxt->chid = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHID_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHID_BMSK);
	ch_k_ctxt->ee = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_GSI_CH_k_CNTXT_0_EE_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_EE_BMSK);
	ch_k_ctxt->chtype_dir = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_DIR_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_DIR_BMSK);
	ch_k_ctxt->chtype_protocol = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_BMSK);
}

static void gsireg_parse_ch_k_cntxt_0_v2_5(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_ch_k_cntxt_0 *ch_k_ctxt =
		(struct gsihal_reg_ch_k_cntxt_0 *) fields;

	gsireg_parse_ch_k_cntxt_0(reg, fields, val);
	ch_k_ctxt->chtype_protocol_msb = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_5_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_MSB_SHFT,
		GSI_V2_5_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_MSB_BMSK);
}

static void gsireg_parse_ch_k_cntxt_0_v3_0(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_ch_k_cntxt_0 *ch_k_ctxt =
		(struct gsihal_reg_ch_k_cntxt_0 *) fields;

	ch_k_ctxt->element_size = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_GSI_CH_k_CNTXT_0_ELEMENT_SIZE_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_ELEMENT_SIZE_BMSK);
	ch_k_ctxt->chstate = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_BMSK);
	ch_k_ctxt->chid = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_CHID_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_CHID_BMSK);
	ch_k_ctxt->ee = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_EE_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_EE_BMSK);
	ch_k_ctxt->chtype_dir = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_DIR_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_DIR_BMSK);
	ch_k_ctxt->chtype_protocol = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_BMSK);
}

static void gsireg_parse_ch_k_cntxt_1_v3_0(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_ch_k_cntxt_1 *ch_k_ctxt =
		(struct gsihal_reg_ch_k_cntxt_1 *) fields;

	ch_k_ctxt->r_length = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_BMSK);
	ch_k_ctxt->erindex = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_1_ERINDEX_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_1_ERINDEX_BMSK);
}

static void gsireg_parse_ev_ch_k_cntxt_0_common(enum gsihal_reg_name reg, void *fields,
	u32 val)
{
	struct gsihal_reg_ev_ch_k_cntxt_0 *ev_ch_k_ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_0 *) fields;

	ev_ch_k_ctxt->element_size = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_EV_CH_k_CNTXT_0_ELEMENT_SIZE_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_0_ELEMENT_SIZE_BMSK);
	ev_ch_k_ctxt->chstate = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_EV_CH_k_CNTXT_0_CHSTATE_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_0_CHSTATE_BMSK);
	ev_ch_k_ctxt->evchid = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_EV_CH_k_CNTXT_0_EVCHID_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_0_EVCHID_BMSK);
}

static void gsireg_parse_ev_ch_k_cntxt_0(enum gsihal_reg_name reg, void *fields,
	u32 val)
{
	struct gsihal_reg_ev_ch_k_cntxt_0 *ev_ch_k_ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_0 *) fields;

	gsireg_parse_ev_ch_k_cntxt_0_common(reg, fields, val);

	ev_ch_k_ctxt->intype = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_EV_CH_k_CNTXT_0_INTYPE_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_0_INTYPE_BMSK);
	ev_ch_k_ctxt->ee = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_EV_CH_k_CNTXT_0_EE_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_0_EE_BMSK);
	ev_ch_k_ctxt->chtype = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_EV_CH_k_CNTXT_0_CHTYPE_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_0_CHTYPE_BMSK);
}

static void gsireg_parse_ev_ch_k_cntxt_0_v3_0(enum gsihal_reg_name reg, void *fields,
	u32 val)
{
	struct gsihal_reg_ev_ch_k_cntxt_0 *ev_ch_k_ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_0 *) fields;

	gsireg_parse_ev_ch_k_cntxt_0_common(reg, fields, val);

	ev_ch_k_ctxt->ee = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_0_EE_SHFT,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_0_EE_BMSK);
	ev_ch_k_ctxt->intype = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_0_INTYPE_SHFT,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_0_INTYPE_BMSK);
	ev_ch_k_ctxt->chtype = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_0_CHTYPE_SHFT,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_0_CHTYPE_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_0_common(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_0 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_0 *)fields;

	GSI_SETFIELD_IN_REG(*val, ctxt->element_size,
		GSI_EE_n_EV_CH_k_CNTXT_0_ELEMENT_SIZE_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_0_ELEMENT_SIZE_BMSK);
	GSI_SETFIELD_IN_REG(*val, ctxt->chstate,
		GSI_EE_n_EV_CH_k_CNTXT_0_CHSTATE_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_0_CHSTATE_BMSK);
	GSI_SETFIELD_IN_REG(*val, ctxt->evchid,
		GSI_EE_n_EV_CH_k_CNTXT_0_EVCHID_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_0_EVCHID_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_0(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_0 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_0 *)fields;

	gsireg_construct_ev_ch_k_cntxt_0_common(reg, fields, val);

	GSI_SETFIELD_IN_REG(*val, ctxt->intype,
		GSI_EE_n_EV_CH_k_CNTXT_0_INTYPE_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_0_INTYPE_BMSK);
	GSI_SETFIELD_IN_REG(*val, ctxt->ee,
		GSI_EE_n_EV_CH_k_CNTXT_0_EE_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_0_EE_BMSK);
	GSI_SETFIELD_IN_REG(*val, ctxt->chtype,
		GSI_EE_n_EV_CH_k_CNTXT_0_CHTYPE_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_0_CHTYPE_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_0_v3_0(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_0 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_0 *)fields;

	gsireg_construct_ev_ch_k_cntxt_0_common(reg, fields, val);

	GSI_SETFIELD_IN_REG(*val, ctxt->ee,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_0_EE_SHFT,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_0_EE_BMSK);
	GSI_SETFIELD_IN_REG(*val, ctxt->intype,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_0_INTYPE_SHFT,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_0_INTYPE_BMSK);
	GSI_SETFIELD_IN_REG(*val, ctxt->chtype,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_0_CHTYPE_SHFT,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_0_CHTYPE_BMSK);
}

static void gsireg_parse_cntxt_glob_irq_stts(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_cntxt_glob_irq_stts *glob_irq_stts =
		(struct gsihal_reg_cntxt_glob_irq_stts *) fields;

	glob_irq_stts->gp_int3 = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_GLOB_IRQ_STTS_GP_INT3_SHFT,
		GSI_EE_n_CNTXT_GLOB_IRQ_STTS_GP_INT3_BMSK);
	glob_irq_stts->gp_int2 = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_GLOB_IRQ_STTS_GP_INT2_SHFT,
		GSI_EE_n_CNTXT_GLOB_IRQ_STTS_GP_INT2_BMSK);
	glob_irq_stts->gp_int1 = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_GLOB_IRQ_STTS_GP_INT1_SHFT,
		GSI_EE_n_CNTXT_GLOB_IRQ_STTS_GP_INT1_BMSK);
	glob_irq_stts->error_int = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_GLOB_IRQ_STTS_ERROR_INT_SHFT,
		GSI_EE_n_CNTXT_GLOB_IRQ_STTS_ERROR_INT_BMSK);
}

static void gsireg_parse_cntxt_gsi_irq_stts(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_cntxt_gsi_irq_stts *gsi_irq_stts =
		(struct gsihal_reg_cntxt_gsi_irq_stts *) fields;

	gsi_irq_stts->gsi_mcs_stack_ovrflow = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_GSI_IRQ_STTS_GSI_MCS_STACK_OVRFLOW_SHFT,
		GSI_EE_n_CNTXT_GSI_IRQ_STTS_GSI_MCS_STACK_OVRFLOW_BMSK);
	gsi_irq_stts->gsi_cmd_fifo_ovrflow = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_GSI_IRQ_STTS_GSI_CMD_FIFO_OVRFLOW_SHFT,
		GSI_EE_n_CNTXT_GSI_IRQ_STTS_GSI_CMD_FIFO_OVRFLOW_BMSK);
	gsi_irq_stts->gsi_bus_error = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_GSI_IRQ_STTS_GSI_BUS_ERROR_SHFT,
		GSI_EE_n_CNTXT_GSI_IRQ_STTS_GSI_BUS_ERROR_BMSK);
	gsi_irq_stts->gsi_break_point = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_CNTXT_GSI_IRQ_STTS_GSI_BREAK_POINT_SHFT,
		GSI_EE_n_CNTXT_GSI_IRQ_STTS_GSI_BREAK_POINT_BMSK);
}

static void gsireg_parse_hw_param(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_hw_param *hw_param =
		(struct gsihal_reg_hw_param *) fields;

	hw_param->periph_sec_grp = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_0_EE_n_GSI_HW_PARAM_PERIPH_SEC_GRP_SHFT,
		GSI_V1_0_EE_n_GSI_HW_PARAM_PERIPH_SEC_GRP_BMSK);
	hw_param->use_axi_m = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_0_EE_n_GSI_HW_PARAM_USE_AXI_M_SHFT,
		GSI_V1_0_EE_n_GSI_HW_PARAM_USE_AXI_M_BMSK);
	hw_param->periph_conf_addr_bus_w = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_0_EE_n_GSI_HW_PARAM_PERIPH_CONF_ADDR_BUS_W_SHFT,
		GSI_V1_0_EE_n_GSI_HW_PARAM_PERIPH_CONF_ADDR_BUS_W_BMSK);
	hw_param->num_ees = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_0_EE_n_GSI_HW_PARAM_NUM_EES_SHFT,
		GSI_V1_0_EE_n_GSI_HW_PARAM_NUM_EES_BMSK);
	hw_param->gsi_ch_num = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_0_EE_n_GSI_HW_PARAM_GSI_CH_NUM_SHFT,
		GSI_V1_0_EE_n_GSI_HW_PARAM_GSI_CH_NUM_BMSK);
	hw_param->gsi_ev_ch_num = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_0_EE_n_GSI_HW_PARAM_GSI_EV_CH_NUM_SHFT,
		GSI_V1_0_EE_n_GSI_HW_PARAM_GSI_EV_CH_NUM_BMSK);
}

static void gsireg_parse_hw_param0(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_hw_param *hw_param =
		(struct gsihal_reg_hw_param *) fields;

	hw_param->periph_sec_grp = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_2_EE_n_GSI_HW_PARAM_0_PERIPH_SEC_GRP_SHFT,
		GSI_V1_2_EE_n_GSI_HW_PARAM_0_PERIPH_SEC_GRP_BMSK);
	hw_param->use_axi_m = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_2_EE_n_GSI_HW_PARAM_0_USE_AXI_M_SHFT,
		GSI_V1_2_EE_n_GSI_HW_PARAM_0_USE_AXI_M_BMSK);
	hw_param->periph_conf_addr_bus_w = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_2_EE_n_GSI_HW_PARAM_0_PERIPH_CONF_ADDR_BUS_W_SHFT,
		GSI_V1_2_EE_n_GSI_HW_PARAM_0_PERIPH_CONF_ADDR_BUS_W_BMSK);
	hw_param->num_ees = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_2_EE_n_GSI_HW_PARAM_0_NUM_EES_SHFT,
		GSI_V1_2_EE_n_GSI_HW_PARAM_0_NUM_EES_BMSK);
	hw_param->gsi_ch_num = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_2_EE_n_GSI_HW_PARAM_0_GSI_CH_NUM_SHFT,
		GSI_V1_2_EE_n_GSI_HW_PARAM_0_GSI_CH_NUM_BMSK);
	hw_param->gsi_ev_ch_num = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_2_EE_n_GSI_HW_PARAM_0_GSI_EV_CH_NUM_SHFT,
		GSI_V1_2_EE_n_GSI_HW_PARAM_0_GSI_EV_CH_NUM_BMSK);
}

static void gsireg_parse_hw_param2(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_hw_param2 *hw_param =
		(struct gsihal_reg_hw_param2 *) fields;

	hw_param->gsi_ch_full_logic = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_CH_FULL_LOGIC_SHFT,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_CH_FULL_LOGIC_BMSK);
	hw_param->gsi_ch_pend_translate = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_CH_PEND_TRANSLATE_SHFT,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_CH_PEND_TRANSLATE_BMSK);
	hw_param->gsi_num_ev_per_ee = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_NUM_EV_PER_EE_SHFT,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_NUM_EV_PER_EE_BMSK);
	hw_param->gsi_num_ch_per_ee = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_NUM_CH_PER_EE_SHFT,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_NUM_CH_PER_EE_BMSK);
	hw_param->gsi_iram_size = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_IRAM_SIZE_SHFT,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_IRAM_SIZE_BMSK);
}

static void gsireg_parse_hw_param2_v2_0(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_hw_param2 *hw_param =
		(struct gsihal_reg_hw_param2 *) fields;

	gsireg_parse_hw_param2(reg, fields, val);

	hw_param->gsi_sdma_n_iovec = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_SDMA_N_IOVEC_SHFT,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_SDMA_N_IOVEC_BMSK);
	hw_param->gsi_sdma_max_burst = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_SDMA_MAX_BURST_SHFT,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_SDMA_MAX_BURST_BMSK);
	hw_param->gsi_sdma_n_int = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_SDMA_N_INT_SHFT,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_SDMA_N_INT_BMSK);
	hw_param->gsi_use_sdma = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_USE_SDMA_SHFT,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_USE_SDMA_BMSK);

}

static void gsireg_parse_hw_param2_v2_2(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_hw_param2 *hw_param =
		(struct gsihal_reg_hw_param2 *) fields;

	gsireg_parse_hw_param2_v2_0(reg, fields, val);

	hw_param->gsi_use_inter_ee = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_2_EE_n_GSI_HW_PARAM_2_GSI_USE_INTER_EE_SHFT,
		GSI_V2_2_EE_n_GSI_HW_PARAM_2_GSI_USE_INTER_EE_BMSK);
	hw_param->gsi_use_rd_wr_eng = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_2_EE_n_GSI_HW_PARAM_2_GSI_USE_RD_WR_ENG_SHFT,
		GSI_V2_2_EE_n_GSI_HW_PARAM_2_GSI_USE_RD_WR_ENG_BMSK);
}

static void gsireg_parse_hw_param2_v3_0(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_hw_param2 *hw_param =
		(struct gsihal_reg_hw_param2 *) fields;

	hw_param->gsi_ch_full_logic = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_CH_FULL_LOGIC_SHFT,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_CH_FULL_LOGIC_BMSK);
	hw_param->gsi_ch_pend_translate = GSI_GETFIELD_FROM_REG(val,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_CH_PEND_TRANSLATE_SHFT,
		GSI_V1_3_EE_n_GSI_HW_PARAM_2_GSI_CH_PEND_TRANSLATE_BMSK);
	hw_param->gsi_num_ch_per_ee = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_GSI_HW_PARAM_2_GSI_NUM_CH_PER_EE_SHFT,
		GSI_V3_0_EE_n_GSI_HW_PARAM_2_GSI_NUM_CH_PER_EE_BMSK);
	hw_param->gsi_iram_size = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_GSI_HW_PARAM_2_GSI_IRAM_SIZE_SHFT,
		GSI_V3_0_EE_n_GSI_HW_PARAM_2_GSI_IRAM_SIZE_BMSK);
	hw_param->gsi_sdma_n_iovec = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_SDMA_N_IOVEC_SHFT,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_SDMA_N_IOVEC_BMSK);
	hw_param->gsi_sdma_max_burst = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_SDMA_MAX_BURST_SHFT,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_SDMA_MAX_BURST_BMSK);
	hw_param->gsi_sdma_n_int = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_SDMA_N_INT_SHFT,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_SDMA_N_INT_BMSK);
	hw_param->gsi_use_sdma = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_USE_SDMA_SHFT,
		GSI_V2_0_EE_n_GSI_HW_PARAM_2_GSI_USE_SDMA_BMSK);
	hw_param->gsi_use_inter_ee = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_2_EE_n_GSI_HW_PARAM_2_GSI_USE_INTER_EE_SHFT,
		GSI_V2_2_EE_n_GSI_HW_PARAM_2_GSI_USE_INTER_EE_BMSK);
	hw_param->gsi_use_rd_wr_eng = GSI_GETFIELD_FROM_REG(val,
		GSI_V2_2_EE_n_GSI_HW_PARAM_2_GSI_USE_RD_WR_ENG_SHFT,
		GSI_V2_2_EE_n_GSI_HW_PARAM_2_GSI_USE_RD_WR_ENG_BMSK);
}

static void gsireg_parse_hw_param4_v3_0(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_hw_param4 *hw_param =
		(struct gsihal_reg_hw_param4 *) fields;

	hw_param->gsi_iram_protcol_cnt = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_GSI_HW_PARAM_4_GSI_IRAM_PROTCOL_CNT_SHFT,
		GSI_V3_0_EE_n_GSI_HW_PARAM_4_GSI_IRAM_PROTCOL_CNT_BMSK);
	hw_param->gsi_num_ev_per_ee = GSI_GETFIELD_FROM_REG(val,
		GSI_V3_0_EE_n_GSI_HW_PARAM_4_GSI_NUM_EV_PER_EE_SHFT,
		GSI_V3_0_EE_n_GSI_HW_PARAM_4_GSI_NUM_EV_PER_EE_BMSK);
}

static void gsireg_parse_gsi_status(enum gsihal_reg_name reg,
	void *fields, u32 val)
{
	struct gsihal_reg_gsi_status *gsi_status =
		(struct gsihal_reg_gsi_status *) fields;

	gsi_status->enabled = GSI_GETFIELD_FROM_REG(val,
		GSI_EE_n_GSI_STATUS_ENABLED_SHFT,
		GSI_EE_n_GSI_STATUS_ENABLED_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_1(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_1 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_1 *)fields;

	GSI_SETFIELD_IN_REG(*val, ctxt->r_length,
		GSI_EE_n_EV_CH_k_CNTXT_1_R_LENGTH_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_1_R_LENGTH_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_1_v2_9(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_1 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_1 *)fields;

	GSI_SETFIELD_IN_REG(*val, ctxt->r_length,
		GSI_V2_9_EE_n_EV_CH_k_CNTXT_1_R_LENGTH_SHFT,
		GSI_V2_9_EE_n_EV_CH_k_CNTXT_1_R_LENGTH_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_1_v3_0(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_1 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_1 *)fields;

	GSI_SETFIELD_IN_REG(*val, ctxt->r_length,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_1_R_LENGTH_SHFT,
		GSI_V3_0_EE_n_EV_CH_k_CNTXT_1_R_LENGTH_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_2(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_2 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_2 *)fields;

	GSI_SETFIELD_IN_REG(*val, ctxt->r_base_addr_lsbs,
		GSI_EE_n_EV_CH_k_CNTXT_2_R_BASE_ADDR_LSBS_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_2_R_BASE_ADDR_LSBS_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_3(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_3 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_3 *)fields;

	GSI_SETFIELD_IN_REG(*val, ctxt->r_base_addr_msbs,
		GSI_EE_n_EV_CH_k_CNTXT_3_R_BASE_ADDR_MSBS_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_3_R_BASE_ADDR_MSBS_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_8(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_8 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_8 *)fields;

	GSI_SETFIELD_IN_REG(*val, ctxt->int_mod_cnt,
		GSI_EE_n_EV_CH_k_CNTXT_8_INT_MOD_CNT_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_8_INT_MOD_CNT_BMSK);
	GSI_SETFIELD_IN_REG(*val, ctxt->int_modc,
		GSI_EE_n_EV_CH_k_CNTXT_8_INT_MODC_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_8_INT_MODC_BMSK);
	GSI_SETFIELD_IN_REG(*val, ctxt->int_modt,
		GSI_EE_n_EV_CH_k_CNTXT_8_INT_MODT_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_8_INT_MODT_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_9(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_9 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_9 *)fields;

	GSI_SETFIELD_IN_REG(*val, ctxt->intvec,
		GSI_EE_n_EV_CH_k_CNTXT_9_INTVEC_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_9_INTVEC_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_10(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_10 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_10 *)fields;

	GSI_SETFIELD_IN_REG(*val, ctxt->msi_addr_lsb,
		GSI_EE_n_EV_CH_k_CNTXT_10_MSI_ADDR_LSB_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_10_MSI_ADDR_LSB_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_11(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_11 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_11 *)fields;

	GSI_SETFIELD_IN_REG(*val, ctxt->msi_addr_msb,
		GSI_EE_n_EV_CH_k_CNTXT_11_MSI_ADDR_MSB_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_11_MSI_ADDR_MSB_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_12(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_12 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_12 *)fields;

	GSI_SETFIELD_IN_REG(*val, ctxt->rp_update_addr_lsb,
		GSI_EE_n_EV_CH_k_CNTXT_12_RP_UPDATE_ADDR_LSB_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_12_RP_UPDATE_ADDR_LSB_BMSK);
}

static void gsireg_construct_ev_ch_k_cntxt_13(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ev_ch_k_cntxt_13 *ctxt =
		(struct gsihal_reg_ev_ch_k_cntxt_13 *)fields;

	GSI_SETFIELD_IN_REG(*val, ctxt->rp_update_addr_msb,
		GSI_EE_n_EV_CH_k_CNTXT_13_RP_UPDATE_ADDR_MSB_SHFT,
		GSI_EE_n_EV_CH_k_CNTXT_13_RP_UPDATE_ADDR_MSB_BMSK);
}

static void gsireg_construct_ev_ch_k_doorbell_1(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_gsi_ee_n_ev_ch_k_doorbell_1 *db =
		(struct gsihal_reg_gsi_ee_n_ev_ch_k_doorbell_1 *)fields;

	GSI_SETFIELD_IN_REG(*val, db->write_ptr_msb,
		GSI_EE_n_EV_CH_k_DOORBELL_1_WRITE_PTR_MSB_SHFT,
		GSI_EE_n_EV_CH_k_DOORBELL_1_WRITE_PTR_MSB_BMSK);
}

static void gsireg_construct_ee_n_ev_ch_cmd(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ee_n_ev_ch_cmd *ev_ch_cmd =
		(struct gsihal_reg_ee_n_ev_ch_cmd *)fields;

	GSI_SETFIELD_IN_REG(*val, ev_ch_cmd->opcode,
		GSI_EE_n_EV_CH_CMD_OPCODE_SHFT,
		GSI_EE_n_EV_CH_CMD_OPCODE_BMSK);
	GSI_SETFIELD_IN_REG(*val, ev_ch_cmd->chid,
		GSI_EE_n_EV_CH_CMD_CHID_SHFT,
		GSI_EE_n_EV_CH_CMD_CHID_BMSK);
}

static void gsireg_construct_ee_n_gsi_ch_k_qos(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_gsi_ee_n_gsi_ch_k_qos *ch_qos =
		(struct gsihal_reg_gsi_ee_n_gsi_ch_k_qos *)fields;

	GSI_SETFIELD_IN_REG(*val, !!ch_qos->use_db_eng,
		GSI_EE_n_GSI_CH_k_QOS_USE_DB_ENG_SHFT,
		GSI_EE_n_GSI_CH_k_QOS_USE_DB_ENG_BMSK);
	GSI_SETFIELD_IN_REG(*val, !!ch_qos->max_prefetch,
		GSI_EE_n_GSI_CH_k_QOS_MAX_PREFETCH_SHFT,
		GSI_EE_n_GSI_CH_k_QOS_MAX_PREFETCH_BMSK);
	GSI_SETFIELD_IN_REG(*val, ch_qos->wrr_weight,
		GSI_EE_n_GSI_CH_k_QOS_WRR_WEIGHT_SHFT,
		GSI_EE_n_GSI_CH_k_QOS_WRR_WEIGHT_BMSK);
}

static void gsireg_construct_ee_n_gsi_ch_k_qos_v1_2(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_gsi_ee_n_gsi_ch_k_qos *ch_qos =
		(struct gsihal_reg_gsi_ee_n_gsi_ch_k_qos *)fields;

	gsireg_construct_ee_n_gsi_ch_k_qos(reg, fields, val);

	GSI_SETFIELD_IN_REG(*val, !!ch_qos->use_escape_buf_only,
		GSI_V2_0_EE_n_GSI_CH_k_QOS_USE_ESCAPE_BUF_ONLY_SHFT,
		GSI_V2_0_EE_n_GSI_CH_k_QOS_USE_ESCAPE_BUF_ONLY_BMSK);
}

static void gsireg_construct_ee_n_gsi_ch_k_qos_v2_5(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_gsi_ee_n_gsi_ch_k_qos *ch_qos =
		(struct gsihal_reg_gsi_ee_n_gsi_ch_k_qos *)fields;

	gsireg_construct_ee_n_gsi_ch_k_qos(reg, fields, val);

	GSI_SETFIELD_IN_REG(*val, ch_qos->empty_lvl_thrshold,
		GSI_V2_5_EE_n_GSI_CH_k_QOS_EMPTY_LVL_THRSHOLD_SHFT,
		GSI_V2_9_EE_n_GSI_CH_k_QOS_EMPTY_LVL_THRSHOLD_BMSK);

	GSI_SETFIELD_IN_REG(*val, ch_qos->prefetch_mode,
		GSI_V2_5_EE_n_GSI_CH_k_QOS_PREFETCH_MODE_SHFT,
		GSI_V2_5_EE_n_GSI_CH_k_QOS_PREFETCH_MODE_BMSK);
}

static void gsireg_construct_ee_n_gsi_ch_k_qos_v2_9(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_gsi_ee_n_gsi_ch_k_qos *ch_qos =
		(struct gsihal_reg_gsi_ee_n_gsi_ch_k_qos *)fields;

	gsireg_construct_ee_n_gsi_ch_k_qos_v2_5(reg, fields, val);

	GSI_SETFIELD_IN_REG(*val, !!ch_qos->db_in_bytes,
		GSI_V2_9_EE_n_GSI_CH_k_QOS_DB_IN_BYTES_SHFT,
		GSI_V2_9_EE_n_GSI_CH_k_QOS_DB_IN_BYTES_BMSK);
}

static void gsireg_construct_ee_n_gsi_ch_k_qos_v3_0(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_gsi_ee_n_gsi_ch_k_qos *ch_qos =
		(struct gsihal_reg_gsi_ee_n_gsi_ch_k_qos *)fields;

	gsireg_construct_ee_n_gsi_ch_k_qos_v2_9(reg, fields, val);

	GSI_SETFIELD_IN_REG(*val, !!ch_qos->low_latency_en,
		GSI_V3_0_EE_n_GSI_CH_k_QOS_LOW_LATENCY_EN_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_QOS_LOW_LATENCY_EN_BMSK);
}

static void gsireg_construct_ch_k_cntxt_0_common(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ch_k_cntxt_0 *ch_cntxt =
		(struct gsihal_reg_ch_k_cntxt_0 *)fields;

	GSI_SETFIELD_IN_REG(*val, ch_cntxt->element_size,
		GSI_EE_n_GSI_CH_k_CNTXT_0_ELEMENT_SIZE_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_ELEMENT_SIZE_BMSK);
	GSI_SETFIELD_IN_REG(*val, ch_cntxt->chstate,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHSTATE_BMSK);
}

static void gsireg_construct_ch_k_cntxt_0(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ch_k_cntxt_0 *ch_cntxt =
		(struct gsihal_reg_ch_k_cntxt_0 *)fields;

	gsireg_construct_ch_k_cntxt_0_common(reg, fields, val);

	GSI_SETFIELD_IN_REG(*val, ch_cntxt->erindex,
		GSI_EE_n_GSI_CH_k_CNTXT_0_ERINDEX_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_ERINDEX_BMSK);
	GSI_SETFIELD_IN_REG(*val, ch_cntxt->chid,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHID_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHID_BMSK);
	GSI_SETFIELD_IN_REG(*val, ch_cntxt->ee,
		GSI_EE_n_GSI_CH_k_CNTXT_0_EE_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_EE_BMSK);
	GSI_SETFIELD_IN_REG(*val, !!ch_cntxt->chtype_dir,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_DIR_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_DIR_BMSK);
	GSI_SETFIELD_IN_REG(*val, ch_cntxt->chtype_protocol,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_BMSK);
}

static void gsireg_construct_ch_k_cntxt_0_v2_5(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ch_k_cntxt_0 *ch_cntxt =
		(struct gsihal_reg_ch_k_cntxt_0 *)fields;

	gsireg_construct_ch_k_cntxt_0(reg, fields, val);

	GSI_SETFIELD_IN_REG(*val, !!ch_cntxt->chtype_protocol_msb,
		GSI_V2_5_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_MSB_SHFT,
		GSI_V2_5_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_MSB_BMSK);
}

static void gsireg_construct_ch_k_cntxt_0_v3_0(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ch_k_cntxt_0 *ch_cntxt =
		(struct gsihal_reg_ch_k_cntxt_0 *)fields;

	gsireg_construct_ch_k_cntxt_0_common(reg, fields, val);

	GSI_SETFIELD_IN_REG(*val, ch_cntxt->chid,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_CHID_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_CHID_BMSK);
	GSI_SETFIELD_IN_REG(*val, ch_cntxt->ee,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_EE_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_EE_BMSK);
	GSI_SETFIELD_IN_REG(*val, !!ch_cntxt->chtype_dir,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_DIR_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_DIR_BMSK);
	GSI_SETFIELD_IN_REG(*val, ch_cntxt->chtype_protocol,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_0_CHTYPE_PROTOCOL_BMSK);
}

static void gsireg_construct_ch_k_cntxt_1(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ch_k_cntxt_1 *ch_cntxt =
		(struct gsihal_reg_ch_k_cntxt_1 *)fields;

	GSI_SETFIELD_IN_REG(*val, ch_cntxt->r_length,
		GSI_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_SHFT,
		GSI_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_BMSK);
}

static void gsireg_construct_ch_k_cntxt_1_v2_9(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ch_k_cntxt_1 *ch_cntxt =
		(struct gsihal_reg_ch_k_cntxt_1 *)fields;

	GSI_SETFIELD_IN_REG(*val, ch_cntxt->r_length,
		GSI_V2_9_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_SHFT,
		GSI_V2_9_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_BMSK);
}

static void gsireg_construct_ch_k_cntxt_1_v3_0(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ch_k_cntxt_1 *ch_cntxt =
		(struct gsihal_reg_ch_k_cntxt_1 *)fields;

	GSI_SETFIELD_IN_REG(*val, ch_cntxt->r_length,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_BMSK);
	GSI_SETFIELD_IN_REG(*val, ch_cntxt->erindex,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_1_ERINDEX_SHFT,
		GSI_V3_0_EE_n_GSI_CH_k_CNTXT_1_ERINDEX_BMSK);
}

static void gsireg_construct_ee_n_gsi_ch_cmd(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_ee_n_gsi_ch_cmd *ev_ch_cmd =
		(struct gsihal_reg_ee_n_gsi_ch_cmd *)fields;

	GSI_SETFIELD_IN_REG(*val, ev_ch_cmd->opcode,
		GSI_EE_n_GSI_CH_CMD_OPCODE_SHFT,
		GSI_EE_n_GSI_CH_CMD_OPCODE_BMSK);
	GSI_SETFIELD_IN_REG(*val, ev_ch_cmd->chid,
		GSI_EE_n_GSI_CH_CMD_CHID_SHFT,
		GSI_EE_n_GSI_CH_CMD_CHID_BMSK);
}

static void gsireg_construct_gsi_cfg(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_gsi_cfg *gsi_cfg =
		(struct gsihal_reg_gsi_cfg *)fields;

	GSI_SETFIELD_IN_REG(*val, !!gsi_cfg->bp_mtrix_disable,
		GSI_GSI_CFG_BP_MTRIX_DISABLE_SHFT,
		GSI_GSI_CFG_BP_MTRIX_DISABLE_BMSK);
	GSI_SETFIELD_IN_REG(*val, !!gsi_cfg->gsi_pwr_clps,
		GSI_GSI_CFG_GSI_PWR_CLPS_SHFT,
		GSI_GSI_CFG_GSI_PWR_CLPS_BMSK);
	GSI_SETFIELD_IN_REG(*val, !!gsi_cfg->uc_is_mcs,
		GSI_GSI_CFG_UC_IS_MCS_SHFT,
		GSI_GSI_CFG_UC_IS_MCS_BMSK);
	GSI_SETFIELD_IN_REG(*val, !!gsi_cfg->double_mcs_clk_freq,
		GSI_GSI_CFG_DOUBLE_MCS_CLK_FREQ_SHFT,
		GSI_GSI_CFG_DOUBLE_MCS_CLK_FREQ_BMSK);
	GSI_SETFIELD_IN_REG(*val, !!gsi_cfg->mcs_enable,
		GSI_GSI_CFG_MCS_ENABLE_SHFT,
		GSI_GSI_CFG_MCS_ENABLE_BMSK);
	GSI_SETFIELD_IN_REG(*val, !!gsi_cfg->gsi_enable,
		GSI_GSI_CFG_GSI_ENABLE_SHFT,
		GSI_GSI_CFG_GSI_ENABLE_BMSK);
}

static void gsireg_construct_gsi_cfg_v2_5(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_gsi_cfg *gsi_cfg =
		(struct gsihal_reg_gsi_cfg *)fields;

	gsireg_construct_gsi_cfg(reg, fields, val);
	GSI_SETFIELD_IN_REG(*val, gsi_cfg->sleep_clk_div,
		GSI_V2_5_GSI_CFG_SLEEP_CLK_DIV_SHFT,
		GSI_V2_5_GSI_CFG_SLEEP_CLK_DIV_BMSK);
}

static void gsireg_construct_gsi_ee_generic_cmd(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_gsi_ee_generic_cmd *cmd =
		(struct gsihal_reg_gsi_ee_generic_cmd *)fields;

	GSI_SETFIELD_IN_REG(*val, cmd->opcode,
		GSI_EE_n_GSI_EE_GENERIC_CMD_OPCODE_SHFT,
		GSI_EE_n_GSI_EE_GENERIC_CMD_OPCODE_BMSK);
	GSI_SETFIELD_IN_REG(*val, cmd->virt_chan_idx,
		GSI_EE_n_GSI_EE_GENERIC_CMD_VIRT_CHAN_IDX_SHFT,
		GSI_EE_n_GSI_EE_GENERIC_CMD_VIRT_CHAN_IDX_BMSK);
	GSI_SETFIELD_IN_REG(*val, cmd->ee,
		GSI_EE_n_GSI_EE_GENERIC_CMD_EE_SHFT,
		GSI_EE_n_GSI_EE_GENERIC_CMD_EE_BMSK);
}

static void gsireg_construct_gsi_ee_generic_cmd_v3_0(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_gsi_ee_generic_cmd *cmd =
		(struct gsihal_reg_gsi_ee_generic_cmd *)fields;

	GSI_SETFIELD_IN_REG(*val, cmd->opcode,
		GSI_EE_n_GSI_EE_GENERIC_CMD_OPCODE_SHFT,
		GSI_EE_n_GSI_EE_GENERIC_CMD_OPCODE_BMSK);
	GSI_SETFIELD_IN_REG(*val, cmd->virt_chan_idx,
		GSI_V3_0_EE_n_GSI_EE_GENERIC_CMD_VIRT_CHAN_IDX_SHFT,
		GSI_V3_0_EE_n_GSI_EE_GENERIC_CMD_VIRT_CHAN_IDX_BMSK);
	GSI_SETFIELD_IN_REG(*val, cmd->ee,
		GSI_V3_0_EE_n_GSI_EE_GENERIC_CMD_EE_SHFT,
		GSI_V3_0_EE_n_GSI_EE_GENERIC_CMD_EE_BMSK);
}

static void gsireg_construct_cntxt_gsi_irq_en(enum gsihal_reg_name reg,
	const void *fields, u32 *val)
{
	struct gsihal_reg_gsi_ee_n_cntxt_gsi_irq *irq =
		(struct gsihal_reg_gsi_ee_n_cntxt_gsi_irq *)fields;

	GSI_SETFIELD_IN_REG(*val, !!irq->gsi_mcs_stack_ovrflow,
		GSI_EE_n_CNTXT_GSI_IRQ_EN_GSI_MCS_STACK_OVRFLOW_SHFT,
		GSI_EE_n_CNTXT_GSI_IRQ_EN_GSI_MCS_STACK_OVRFLOW_BMSK);
	GSI_SETFIELD_IN_REG(*val, !!irq->gsi_cmd_fifo_ovrflow,
		GSI_EE_n_CNTXT_GSI_IRQ_EN_GSI_CMD_FIFO_OVRFLOW_SHFT,
		GSI_EE_n_CNTXT_GSI_IRQ_EN_GSI_CMD_FIFO_OVRFLOW_BMSK);
	GSI_SETFIELD_IN_REG(*val, !!irq->gsi_bus_error,
		GSI_EE_n_CNTXT_GSI_IRQ_EN_GSI_BUS_ERROR_SHFT,
		GSI_EE_n_CNTXT_GSI_IRQ_EN_GSI_BUS_ERROR_BMSK);
	GSI_SETFIELD_IN_REG(*val, !!irq->gsi_break_point,
		GSI_EE_n_CNTXT_GSI_IRQ_EN_GSI_BREAK_POINT_SHFT,
		GSI_EE_n_CNTXT_GSI_IRQ_EN_GSI_BREAK_POINT_BMSK);
}

/*
* This table contains the info regarding each register for GSI1.0 and later.
* Information like: offset and construct/parse functions.
* All the information on the register on GSI are statically defined below.
* If information is missing regarding some register on some GSI version,
*  the init function will fill it with the information from the previous
*  GSI version.
* Information is considered missing if all of the fields are 0.
* If offset is -1, this means that the register is removed on the
*  specific version.
*/
static struct gsihal_reg_obj gsihal_reg_objs[GSI_VER_MAX][GSI_REG_MAX] = {
	/* GSIv1_0 */
	[GSI_VER_1_0][GSI_EE_n_CNTXT_TYPE_IRQ_MSK] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f088, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f098, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f09c, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_SRC_IEOB_IRQ_MSK] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f0b8, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_GLOB_IRQ_EN] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f108, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_GSI_IRQ_EN] = {
	gsireg_construct_cntxt_gsi_irq_en, gsireg_parse_dummy,
	0x0001f120, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_TYPE_IRQ] = {
	gsireg_construct_dummy, gsireg_parse_ctx_type_irq,
	0x0001f080, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f090, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_CNTXT_0] = {
	gsireg_construct_ch_k_cntxt_0, gsireg_parse_ch_k_cntxt_0,
	0x0001c000, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f0a0, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_SRC_EV_CH_IRQ] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f094, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f0a4, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_0] = {
	gsireg_construct_ev_ch_k_cntxt_0, gsireg_parse_ev_ch_k_cntxt_0,
	0x0001d000, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_GLOB_IRQ_STTS] = {
	gsireg_construct_dummy, gsireg_parse_cntxt_glob_irq_stts,
	0x0001f100, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_ERROR_LOG] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f200, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_ERROR_LOG_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f210, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_GLOB_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f110, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_DOORBELL_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001e100, 0x4000, 0x8},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_DOORBELL_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001e000, 0x4000, 0x8 },
	[GSI_VER_1_0][GSI_EE_n_CNTXT_SRC_IEOB_IRQ] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f0b0, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_SRC_IEOB_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f0c0, 0x4000, 0},
	[GSI_VER_1_0][GSI_INTER_EE_n_SRC_GSI_CH_IRQ] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000c018, 0x1000, 0},
	[GSI_VER_1_0][GSI_INTER_EE_n_SRC_GSI_CH_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000c028, 0x1000, 0},
	[GSI_VER_1_0][GSI_INTER_EE_n_SRC_EV_CH_IRQ] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000c01c, 0x1000, 0},
	[GSI_VER_1_0][GSI_INTER_EE_n_SRC_EV_CH_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000c02c, 0x1000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_GSI_IRQ_STTS] = {
	gsireg_construct_dummy, gsireg_parse_cntxt_gsi_irq_stts,
	0x0001f118, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_GSI_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f128, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_GSI_HW_PARAM] = {
	gsireg_construct_dummy, gsireg_parse_hw_param,
	0x0001f040, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_GSI_SW_VERSION] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f044, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_INTSET] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f180, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_MSI_BASE_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f180, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_MSI_BASE_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f180, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_GSI_STATUS] = {
	gsireg_construct_dummy, gsireg_parse_gsi_status,
	0x0001f000, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_CNTXT_SCRATCH_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001f400, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_1] = {
	gsireg_construct_ev_ch_k_cntxt_1, gsireg_parse_dummy,
	0x0001d004, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_2] = {
	gsireg_construct_ev_ch_k_cntxt_2, gsireg_parse_dummy,
	0x0001d008, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_3] = {
	gsireg_construct_ev_ch_k_cntxt_3, gsireg_parse_dummy,
	0x0001d00c, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_8] = {
	gsireg_construct_ev_ch_k_cntxt_8, gsireg_parse_dummy,
	0x0001d020, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_9] = {
	gsireg_construct_ev_ch_k_cntxt_9, gsireg_parse_dummy,
	0x0001d024, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_10] = {
	gsireg_construct_ev_ch_k_cntxt_10, gsireg_parse_dummy,
	0x0001d028, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_11] = {
	gsireg_construct_ev_ch_k_cntxt_11, gsireg_parse_dummy,
	0x0001d02c, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_12] = {
	gsireg_construct_ev_ch_k_cntxt_12, gsireg_parse_dummy,
	0x0001d030, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_13] = {
	gsireg_construct_ev_ch_k_cntxt_13, gsireg_parse_dummy,
	0x0001d034, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_DOORBELL_1] = {
	gsireg_construct_ev_ch_k_doorbell_1, gsireg_parse_dummy,
	0x0001e104, 0x4000, 0x8},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_CMD] = {
	gsireg_construct_ee_n_ev_ch_cmd, gsireg_parse_dummy,
	0x0001f010, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_SCRATCH_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001d048, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_SCRATCH_1] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001d04c, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_DOORBELL_1] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001e004, 0x4000, 0x8},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_QOS] = {
	gsireg_construct_ee_n_gsi_ch_k_qos, gsireg_parse_dummy,
	0x0001c05c, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_CNTXT_1] = {
	gsireg_construct_ch_k_cntxt_1, gsireg_parse_dummy,
	0x0001c004, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_CNTXT_2] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c008, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_CNTXT_3] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c00c, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_CMD] = {
	gsireg_construct_ee_n_gsi_ch_cmd, gsireg_parse_dummy,
	0x0001f008, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_SCRATCH_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c060, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_SCRATCH_1] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c064, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_SCRATCH_2] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c068, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_SCRATCH_3] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c06c, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_CNTXT_4] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c010, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_CNTXT_5] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c014, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_CNTXT_6] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c018, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_CNTXT_7] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c01c, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_4] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001d010, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_5] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001d014, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_6] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001d018, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_EV_CH_k_CNTXT_7] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001d01c, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_CH_CMD] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000400, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_CH_DB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000418, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_CH_DIS_COMP] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000424, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_CH_EMPTY] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000428, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_EE_GENERIC_CMD] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000404, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_EVENT_GEN_COMP] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000042c, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_INT_MOD_STOPPED] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000044c, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000430, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_2] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000434, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_PERIPH_IF_TLV_IN_1] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000438, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_NEW_RE] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000420, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_READ_ENG_COMP] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000444, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_TIMER_EXPIRED] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000043c, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_EV_DB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000041c, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_UC_GP_INT] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000448, 0, 0},
	[GSI_VER_1_0][GSI_GSI_IRAM_PTR_WRITE_ENG_COMP] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000440, 0, 0},
	[GSI_VER_1_0][GSI_IC_DISABLE_CHNL_BCK_PRS_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000a0, 0, 0},
	[GSI_VER_1_0][GSI_IC_DISABLE_CHNL_BCK_PRS_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000a4, 0, 0},
	[GSI_VER_1_0][GSI_IC_GEN_EVNT_BCK_PRS_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000a8, 0, 0},
	[GSI_VER_1_0][GSI_IC_GEN_EVNT_BCK_PRS_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000ac, 0, 0},
	[GSI_VER_1_0][GSI_IC_GEN_INT_BCK_PRS_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000b0, 0, 0},
	[GSI_VER_1_0][GSI_IC_GEN_INT_BCK_PRS_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000b4, 0, 0},
	[GSI_VER_1_0][GSI_IC_STOP_INT_MOD_BCK_PRS_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000b8, 0, 0 },
	[GSI_VER_1_0][GSI_IC_STOP_INT_MOD_BCK_PRS_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000bc, 0, 0},
	[GSI_VER_1_0][GSI_IC_PROCESS_DESC_BCK_PRS_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000c0, 0, 0},
	[GSI_VER_1_0][GSI_IC_PROCESS_DESC_BCK_PRS_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000c4, 0, 0},
	[GSI_VER_1_0][GSI_IC_TLV_STOP_BCK_PRS_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000c8, 0, 0},
	[GSI_VER_1_0][GSI_IC_TLV_STOP_BCK_PRS_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000cc, 0, 0},
	[GSI_VER_1_0][GSI_IC_TLV_RESET_BCK_PRS_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000d0, 0, 0},
	[GSI_VER_1_0][GSI_IC_TLV_RESET_BCK_PRS_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000d4, 0, 0},
	[GSI_VER_1_0][GSI_IC_RGSTR_TIMER_BCK_PRS_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000d8, 0, 0},
	[GSI_VER_1_0][GSI_IC_RGSTR_TIMER_BCK_PRS_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000dc, 0, 0},
	[GSI_VER_1_0][GSI_IC_READ_BCK_PRS_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000e0, 0, 0},
	[GSI_VER_1_0][GSI_IC_READ_BCK_PRS_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000e4, 0, 0},
	[GSI_VER_1_0][GSI_IC_WRITE_BCK_PRS_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000e8, 0, 0},
	[GSI_VER_1_0][GSI_IC_WRITE_BCK_PRS_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000ec, 0, 0},
	[GSI_VER_1_0][GSI_IC_UCONTROLLER_GPR_BCK_PRS_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000f0, 0, 0},
	[GSI_VER_1_0][GSI_IC_UCONTROLLER_GPR_BCK_PRS_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000000f4, 0, 0},
	[GSI_VER_1_0][GSI_GSI_PERIPH_BASE_ADDR_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000001c, 0, 0},
	[GSI_VER_1_0][GSI_GSI_PERIPH_BASE_ADDR_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000018, 0, 0},
	[GSI_VER_1_0][GSI_GSI_CFG] = {
	gsireg_construct_gsi_cfg, gsireg_parse_dummy,
	0x00000000, 0, 0},
	[GSI_VER_1_0][GSI_EE_n_GSI_EE_GENERIC_CMD] = {
	gsireg_construct_gsi_ee_generic_cmd, gsireg_parse_dummy,
	0x0001f018, 0x4000, 0},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_RE_FETCH_READ_PTR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c054, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c058, 0x4000, 0x80},
	[GSI_VER_1_0][GSI_GSI_INST_RAM_n] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00004000, GSI_GSI_INST_RAM_n_WORD_SZ, 0},

	/* GSIv1_2 */
	[GSI_VER_1_2][GSI_EE_n_GSI_HW_PARAM] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	-1, 0, 0},
	[GSI_VER_1_2][GSI_EE_n_GSI_HW_PARAM_0] = {
	gsireg_construct_dummy, gsireg_parse_hw_param0,
	0x0001f038, 0x4000, 0},
	[GSI_VER_1_2][GSI_EE_n_GSI_CH_k_QOS] = {
	gsireg_construct_ee_n_gsi_ch_k_qos_v1_2, gsireg_parse_dummy,
	0x0001c05c, 0x4000, 0x80},
	[GSI_VER_1_2][GSI_GSI_MCS_CFG] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000B000, 0, 0},

	/* GSIv1_3 */
	[GSI_VER_1_3][GSI_EE_n_GSI_HW_PARAM_2] = {
	gsireg_construct_dummy, gsireg_parse_hw_param2,
	0x0001f040, 0x4000, 0},

	/* GSIv2_0 */
	[GSI_VER_2_0][GSI_EE_n_GSI_HW_PARAM_2] = {
	gsireg_construct_dummy, gsireg_parse_hw_param2_v2_0,
	0x0001f040, 0x4000, 0},

	/* GSIv2_2 */
	[GSI_VER_2_2][GSI_EE_n_GSI_HW_PARAM_2] = {
	gsireg_construct_dummy, gsireg_parse_hw_param2_v2_2,
	0x0001f040, 0x4000, 0 },
	
	/* GSIv2_5 */
	[GSI_VER_2_5][GSI_EE_n_CNTXT_TYPE_IRQ_MSK] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012088, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012098, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001209c, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_SRC_IEOB_IRQ_MSK] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000120b8, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_GLOB_IRQ_EN] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012108, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_GSI_IRQ_EN] = {
	gsireg_construct_cntxt_gsi_irq_en, gsireg_parse_dummy,
	0x00012120, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_TYPE_IRQ] = {
	gsireg_construct_dummy, gsireg_parse_ctx_type_irq,
	0x00012080, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012090, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_CNTXT_0] = {
	gsireg_construct_ch_k_cntxt_0_v2_5, gsireg_parse_ch_k_cntxt_0_v2_5,
	0x0000f000, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000120a0, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_SRC_EV_CH_IRQ] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012094, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000120a4, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_0] = {
	gsireg_construct_ev_ch_k_cntxt_0, gsireg_parse_ev_ch_k_cntxt_0,
	0x00010000, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_GLOB_IRQ_STTS] = {
	gsireg_construct_dummy, gsireg_parse_cntxt_glob_irq_stts,
	0x00012100, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_ERROR_LOG] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012200, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_ERROR_LOG_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012210, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_GLOB_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012110, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_DOORBELL_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00011100, 0x4000, 0x8},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_DOORBELL_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00011000, 0x4000, 0x8},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_SRC_IEOB_IRQ] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000120b0, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_SRC_IEOB_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000120c0, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_GSI_IRQ_STTS] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012118, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_GSI_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012128, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_GSI_HW_PARAM_2] = {
	gsireg_construct_dummy, gsireg_parse_hw_param2_v2_2,
	0x00012040, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_GSI_SW_VERSION] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012044, 0x4000, 0 },
	[GSI_VER_2_5][GSI_EE_n_CNTXT_INTSET] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012180, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_MSI_BASE_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012188, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_MSI_BASE_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001218c, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_GSI_STATUS] = {
	gsireg_construct_dummy, gsireg_parse_gsi_status,
	0x00012000, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_CNTXT_SCRATCH_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00012400, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_1] = {
	gsireg_construct_ev_ch_k_cntxt_1, gsireg_parse_dummy,
	0x00010004, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_2] = {
	gsireg_construct_ev_ch_k_cntxt_2, gsireg_parse_dummy,
	0x00010008, 0x4000, 0x80 },
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_3] = {
	gsireg_construct_ev_ch_k_cntxt_3, gsireg_parse_dummy,
	0x0001000c, 0x4000, 0x80 },
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_8] = {
	gsireg_construct_ev_ch_k_cntxt_8, gsireg_parse_dummy,
	0x00010020, 0x4000, 0x80 },
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_9] = {
	gsireg_construct_ev_ch_k_cntxt_9, gsireg_parse_dummy,
	0x00010024, 0x4000, 0x80 },
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_10] = {
	gsireg_construct_ev_ch_k_cntxt_10, gsireg_parse_dummy,
	0x00010028, 0x4000, 0x80 },
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_11] = {
	gsireg_construct_ev_ch_k_cntxt_11, gsireg_parse_dummy,
	0x0001002c, 0x4000, 0x80 },
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_12] = {
	gsireg_construct_ev_ch_k_cntxt_12, gsireg_parse_dummy,
	0x00010030, 0x4000, 0x80 },
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_13] = {
	gsireg_construct_ev_ch_k_cntxt_13, gsireg_parse_dummy,
	0x00010034, 0x4000, 0x80 },
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_DOORBELL_1] = {
	gsireg_construct_ev_ch_k_doorbell_1, gsireg_parse_dummy,
	0x00011104, 0x4000, 0x8},
	[GSI_VER_2_5][GSI_EE_n_EV_CH_CMD] = {
	gsireg_construct_ee_n_ev_ch_cmd, gsireg_parse_dummy,
	0x00012010, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_SCRATCH_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00010048, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_SCRATCH_1] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001004c, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_DOORBELL_1] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00011004, 0x4000, 0x8},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_QOS] = {
	gsireg_construct_ee_n_gsi_ch_k_qos_v2_5, gsireg_parse_dummy,
	0x0000f05c, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_CNTXT_1] = {
	gsireg_construct_ch_k_cntxt_1, gsireg_parse_dummy,
	0x0000f004, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_CNTXT_2] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000f008, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_CNTXT_3] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000f00c, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_CMD] = {
	gsireg_construct_ee_n_gsi_ch_cmd, gsireg_parse_dummy,
	0x00012008, 0x4000, 0},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_SCRATCH_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000f060, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_SCRATCH_1] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000f064, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_SCRATCH_2] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000f068, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_SCRATCH_3] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000f06c, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_CNTXT_4] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000f010, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_CNTXT_5] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000f014, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_CNTXT_6] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000f018, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_GSI_CH_k_CNTXT_7] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000f01c, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_4] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00010010, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_5] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00010014, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_6] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00010018, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_EE_n_EV_CH_k_CNTXT_7] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001001c, 0x4000, 0x80},
	[GSI_VER_2_5][GSI_GSI_IRAM_PTR_TLV_CH_NOT_FULL] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000408, 0, 0},
	[GSI_VER_2_5][GSI_GSI_CFG] = {
	gsireg_construct_gsi_cfg_v2_5, gsireg_parse_dummy,
	0x00000000, 0, 0},
	[GSI_VER_2_5][GSI_EE_n_GSI_EE_GENERIC_CMD] = {
	gsireg_construct_gsi_ee_generic_cmd, gsireg_parse_dummy,
	0x00012018, 0x4000, 0},
	[GSI_VER_2_5][GSI_MAP_EE_n_CH_k_VP_TABLE] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00003800, 0x80, 0x4 },
	[GSI_VER_2_5][GSI_GSI_INST_RAM_n] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001b000, GSI_GSI_INST_RAM_n_WORD_SZ, 0},
	[GSI_VER_2_5][GSI_GSI_SHRAM_n] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00002000, GSI_GSI_SHRAM_n_WORD_SZ, 0 },

	/* GSIv2_9 */
	[GSI_VER_2_9][GSI_EE_n_EV_CH_k_CNTXT_1] = {
	gsireg_construct_ev_ch_k_cntxt_1_v2_9, gsireg_parse_dummy,
	0x00010004, 0x4000, 0x80},
	[GSI_VER_2_9][GSI_EE_n_GSI_CH_k_QOS] = {
	gsireg_construct_ee_n_gsi_ch_k_qos_v2_9, gsireg_parse_dummy,
	0x0000f05c, 0x4000, 0x80},
	[GSI_VER_2_9][GSI_EE_n_GSI_CH_k_CNTXT_1] = {
	gsireg_construct_ch_k_cntxt_1_v2_9, gsireg_parse_dummy,
	0x0000f004, 0x4000, 0x80},

	/* GSIv2_11 */
	[GSI_VER_2_11][GSI_GSI_IRAM_PTR_MSI_DB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000414, 0, 0 },

	/* GSIv3_0 */
	[GSI_VER_3_0][GSI_GSI_IRAM_PTR_INT_NOTIFY_MCS] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00000470, 0, 0 },
	[GSI_VER_3_0][GSI_GSI_INST_RAM_n] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000a4000, GSI_GSI_INST_RAM_n_WORD_SZ, 0},
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_CNTXT_0] = {
	gsireg_construct_ch_k_cntxt_0_v3_0, gsireg_parse_ch_k_cntxt_0_v3_0,
	0x00014000, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_CNTXT_1] = {
	gsireg_construct_ch_k_cntxt_1_v3_0, gsireg_parse_ch_k_cntxt_1_v3_0,
	0x00014004, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_CNTXT_2] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014008, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_CNTXT_3] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001400c, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_CNTXT_4] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014010, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_CNTXT_5] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014014, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_CNTXT_6] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014018, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_CNTXT_7] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001401c, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_CNTXT_8] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014020 , 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_RE_FETCH_READ_PTR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014040, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_RE_FETCH_WRITE_PTR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014044, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_QOS] = {
	gsireg_construct_ee_n_gsi_ch_k_qos_v3_0, gsireg_parse_dummy,
	0x00014048, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_SCRATCH_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001404c, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_SCRATCH_1] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014050, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_SCRATCH_2] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014054, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_SCRATCH_3] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014058, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_SCRATCH_4] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001405c, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_SCRATCH_5] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014060, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_SCRATCH_6] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014064, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_SCRATCH_7] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014068, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_SCRATCH_8] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001406c, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_SCRATCH_9] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00014070, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_0] = {
	gsireg_construct_ev_ch_k_cntxt_0_v3_0, gsireg_parse_ev_ch_k_cntxt_0_v3_0,
	0x0001c000, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_1] = {
	gsireg_construct_ev_ch_k_cntxt_1_v3_0, gsireg_parse_dummy,
	0x0001c004, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_2] = {
	gsireg_construct_ev_ch_k_cntxt_2, gsireg_parse_dummy,
	0x0001c008, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_3] = {
	gsireg_construct_ev_ch_k_cntxt_3, gsireg_parse_dummy,
	0x0001c00c, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_4] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c010, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_5] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c014, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_6] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c018, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_7] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c01c, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_8] = {
	gsireg_construct_ev_ch_k_cntxt_8, gsireg_parse_dummy,
	0x0001c020, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_9] = {
	gsireg_construct_ev_ch_k_cntxt_9, gsireg_parse_dummy,
	0x0001c024, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_10] = {
	gsireg_construct_ev_ch_k_cntxt_10, gsireg_parse_dummy,
	0x0001c028, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_11] = {
	gsireg_construct_ev_ch_k_cntxt_11, gsireg_parse_dummy,
	0x0001c02c, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_12] = {
	gsireg_construct_ev_ch_k_cntxt_12, gsireg_parse_dummy,
	0x0001c030, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_CNTXT_13] = {
	gsireg_construct_ev_ch_k_cntxt_13, gsireg_parse_dummy,
	0x0001c034, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_SCRATCH_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c048, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_SCRATCH_1] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0001c04c, 0x12000, 0x80 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_DOORBELL_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00024000, 0x12000, 0x8 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_k_DOORBELL_1] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00024004, 0x12000, 0x8 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_DOORBELL_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00024800, 0x12000, 0x8 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_k_DOORBELL_1] = {
	gsireg_construct_ev_ch_k_doorbell_1, gsireg_parse_dummy,
	0x00024804, 0x12000, 0x8 },
	[GSI_VER_3_0][GSI_EE_n_GSI_STATUS] = {
	gsireg_construct_dummy, gsireg_parse_gsi_status,
	0x00025000, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_GSI_CH_CMD] = {
	gsireg_construct_ee_n_gsi_ch_cmd, gsireg_parse_dummy,
	0x00025008, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_EV_CH_CMD] = {
	gsireg_construct_ee_n_ev_ch_cmd, gsireg_parse_dummy,
	0x00025010, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_GSI_EE_GENERIC_CMD] = {
	gsireg_construct_gsi_ee_generic_cmd_v3_0, gsireg_parse_dummy,
	0x00025018, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_GSI_HW_PARAM_2] = {
	gsireg_construct_dummy, gsireg_parse_hw_param2_v3_0,
	0x00025040, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_GSI_HW_PARAM_4] = {
	gsireg_construct_dummy, gsireg_parse_hw_param4_v3_0,
	0x00025050, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_GSI_SW_VERSION] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025044, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_TYPE_IRQ] = {
	gsireg_construct_dummy, gsireg_parse_ctx_type_irq,
	0x00025080, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_TYPE_IRQ_MSK] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025088, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025090, 0x12000, 0x24 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_MSK_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025094, 0x12000, 0x24 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_SRC_GSI_CH_IRQ_CLR_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025098, 0x12000, 0x24 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0002509c, 0x12000, 0x24 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_MSK_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000250a0, 0x12000, 0x24 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_SRC_EV_CH_IRQ_CLR_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000250a4, 0x12000, 0x24 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_SRC_IEOB_IRQ_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000250a8, 0x12000, 0x24 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_SRC_IEOB_IRQ_MSK_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000250ac, 0x12000, 0x24 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_SRC_IEOB_IRQ_CLR_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x000250b0, 0x12000, 0x24 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_GLOB_IRQ_STTS] = {
	gsireg_construct_dummy, gsireg_parse_cntxt_glob_irq_stts,
	0x00025200, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_GLOB_IRQ_EN] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025204, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_GLOB_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025208, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_GSI_IRQ_STTS] = {
	gsireg_construct_dummy, gsireg_parse_cntxt_gsi_irq_stts,
	0x0002520c, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_GSI_IRQ_EN] = {
	gsireg_construct_cntxt_gsi_irq_en, gsireg_parse_dummy,
	0x00025210, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_GSI_IRQ_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025214, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_MSI_BASE_LSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025230, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_MSI_BASE_MSB] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025234, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_INTSET] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025220, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_ERROR_LOG] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025240, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_ERROR_LOG_CLR] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00025244, 0x12000, 0 },
	[GSI_VER_3_0][GSI_EE_n_CNTXT_SCRATCH_0] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00024500, 0x12000, 0},
	[GSI_VER_3_0][GSI_INTER_EE_n_SRC_GSI_CH_IRQ_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000c018, 0x1000, 0x18 },
	[GSI_VER_3_0][GSI_INTER_EE_n_SRC_GSI_CH_IRQ_CLR_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000c020, 0x1000, 0x18 },
	[GSI_VER_3_0][GSI_INTER_EE_n_SRC_EV_CH_IRQ_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000c024, 0x1000, 0x18 },
	[GSI_VER_3_0][GSI_INTER_EE_n_SRC_EV_CH_IRQ_CLR_k] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x0000c02c, 0x1000, 0x18 },
	[GSI_VER_3_0][GSI_MAP_EE_n_CH_k_VP_TABLE] = {
	gsireg_construct_dummy, gsireg_parse_dummy,
	0x00009000, 0x400, 0x4 },
};

/*
 * gsihal_read_reg_nk() - Get nk parameterized reg value
 */
u32 gsihal_read_reg_nk(enum gsihal_reg_name reg, u32 n, u32 k)
{
	u32 offset;

	if (reg >= GSI_REG_MAX) {
		GSIERR("Invalid register reg=%u\n", reg);
		WARN_ON(1);
		return -EINVAL;
	}

	GSIDBG_LOW("read %s k=%u n=%u\n",
		gsihal_reg_name_str(reg), k, n);
	offset = gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].offset;
	if (offset == -1) {
		GSIERR("Read access to obsolete reg=%s\n",
			gsihal_reg_name_str(reg));
		WARN_ON_ONCE(1);
		return -EPERM;
	}

	offset += gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].k_ofst * k;
	offset += gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].n_ofst * n;
	return gsi_readl(gsihal_ctx->base + offset);
}

/*
* gsihal_write_reg_nk() - Write to n/k parameterized reg a raw value
*/
void gsihal_write_reg_nk(enum gsihal_reg_name reg, u32 n, u32 k, u32 val)
{
	u32 offset;

	if (reg >= GSI_REG_MAX) {
		GSIERR("Invalid register reg=%u\n", reg);
		WARN_ON(1);
		return;
	}

	GSIDBG_LOW("write to %s k=%u n=%u val=%u\n",
		gsihal_reg_name_str(reg), k, n, val);
	offset = gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].offset;
	if (offset == -1) {
		GSIERR("Write access to obsolete reg=%s\n",
			gsihal_reg_name_str(reg));
		WARN_ON(1);
		return;
	}

	offset += gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].k_ofst * k;
	offset += gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].n_ofst * n;
	gsi_writel(val, gsihal_ctx->base + offset);
}

/*
 * gsihal_write_reg_fields() - Write to reg a prased value
 */
void gsihal_write_reg_fields(enum gsihal_reg_name reg, const void *fields)
{
	u32 val = 0;
	u32 offset;

	if (!fields) {
		GSIERR("Input error fields=%pK\n", fields);
		WARN_ON(1);
		return;
	}

	if (reg >= GSI_REG_MAX) {
		GSIERR("Invalid register reg=%u\n", reg);
		WARN_ON(1);
		return;
	}

	GSIDBG_LOW("write to %s after constructing it\n",
		gsihal_reg_name_str(reg));
	offset = gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].offset;
	if (offset == -1) {
		GSIERR("Write access to obsolete reg=%s\n",
			gsihal_reg_name_str(reg));
		WARN_ON(1);
		return;
	}
	gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].construct(reg, fields, &val);

	gsi_writel(val, gsihal_ctx->base + offset);
}

/*
 * gsihal_read_reg_fields() - Get the parsed value of reg
 */
u32 gsihal_read_reg_fields(enum gsihal_reg_name reg, void *fields)
{
	u32 val = 0;
	u32 offset;

	if (!fields) {
		GSIERR("Input error fields\n");
		WARN_ON(1);
		return -EINVAL;
	}

	if (reg >= GSI_REG_MAX) {
		GSIERR("Invalid register reg=%u\n", reg);
		WARN_ON(1);
		return -EINVAL;
	}

	GSIDBG_LOW("read from %s and parse it\n",
		gsihal_reg_name_str(reg));
	offset = gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].offset;
	if (offset == -1) {
		GSIERR("Read access to obsolete reg=%s\n",
			gsihal_reg_name_str(reg));
		WARN_ON(1);
		return -EPERM;
	}
	val = gsi_readl(gsihal_ctx->base + offset);
	gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].parse(reg, fields, val);

	return val;
}

/*
 * gsihal_write_reg_n_fields() - Write to n parameterized reg a prased value
 */
void gsihal_write_reg_n_fields(enum gsihal_reg_name reg, u32 n,
	const void *fields)
{
	u32 val = 0;
	u32 offset;

	if (!fields) {
		GSIERR("Input error fields=%pK\n", fields);
		WARN_ON(1);
		return;
	}

	if (reg >= GSI_REG_MAX) {
		GSIERR("Invalid register reg=%u\n", reg);
		WARN_ON(1);
		return;
	}

	GSIDBG_LOW("write to %s n=%u after constructing it\n",
		gsihal_reg_name_str(reg), n);
	offset = gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].offset;
	if (offset == -1) {
		GSIERR("Write access to obsolete reg=%s\n",
			gsihal_reg_name_str(reg));
		WARN_ON(1);
		return;
	}
	offset += gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].n_ofst * n;
	gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].construct(reg, fields, &val);

	gsi_writel(val, gsihal_ctx->base + offset);
}

/*
 * gsihal_read_reg_n_fields() - Get the parsed value of n parameterized reg
 */
u32 gsihal_read_reg_n_fields(enum gsihal_reg_name reg, u32 n, void *fields)
{
	u32 val = 0;
	u32 offset;

	if (!fields) {
		GSIERR("Input error fields\n");
		WARN_ON(1);
		return -EINVAL;
	}

	if (reg >= GSI_REG_MAX) {
		GSIERR("Invalid register reg=%u\n", reg);
		WARN_ON(1);
		return -EINVAL;
	}

	GSIDBG_LOW("read from %s n=%u and parse it\n",
		gsihal_reg_name_str(reg), n);
	offset = gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].offset;
	if (offset == -1) {
		GSIERR("Read access to obsolete reg=%s\n",
			gsihal_reg_name_str(reg));
		WARN_ON(1);
		return -EPERM;
	}
	offset += gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].n_ofst * n;
	val = gsi_readl(gsihal_ctx->base + offset);
	gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].parse(reg, fields, val);

	return val;
}

/*
 * gsihal_read_reg_nk_fields() - Get the parsed value of nk parameterized reg
 */
u32 gsihal_read_reg_nk_fields(enum gsihal_reg_name reg,
	u32 n, u32 k, void *fields)
{
	u32 val = 0;
	u32 offset;

	if (!fields) {
		GSIERR("Input error fields\n");
		WARN_ON(1);
		return -EINVAL;
	}

	if (reg >= GSI_REG_MAX) {
		GSIERR("Invalid register reg=%u\n", reg);
		WARN_ON(1);
		return -EINVAL;
	}

	GSIDBG_LOW("read from %s n=%u k= %u and parse it\n",
		gsihal_reg_name_str(reg), n, k);
	offset = gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].offset;
	if (offset == -1) {
		GSIERR("Read access to obsolete reg=%s\n",
			gsihal_reg_name_str(reg));
		WARN_ON(1);
		return -EPERM;
	}
	offset += gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].n_ofst * n;
	offset += gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].k_ofst * k;
	val = gsi_readl(gsihal_ctx->base + offset);
	gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].parse(reg, fields, val);

	return val;
}

/*
 * gsihal_write_reg_nk_fields() - Write to nk parameterized reg a prased value
 */
void gsihal_write_reg_nk_fields(enum gsihal_reg_name reg, u32 n, u32 k,
	const void *fields)
{
	u32 val = 0;
	u32 offset;

	if (!fields) {
		GSIERR("Input error fields=%pK\n", fields);
		WARN_ON(1);
		return;
	}

	if (reg >= GSI_REG_MAX) {
		GSIERR("Invalid register reg=%u\n", reg);
		WARN_ON(1);
		return;
	}

	GSIDBG_LOW("write to %s n=%u after constructing it\n",
		gsihal_reg_name_str(reg), n);
	offset = gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].offset;
	if (offset == -1) {
		GSIERR("Write access to obsolete reg=%s\n",
			gsihal_reg_name_str(reg));
		WARN_ON(1);
		return;
	}
	offset += gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].n_ofst * n;
	offset += gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].k_ofst * k;
	gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].construct(reg, fields, &val);

	gsi_writel(val, gsihal_ctx->base + offset);
}

/*
 * Get the offset of a nk parameterized register
 */
u32 gsihal_get_reg_nk_ofst(enum gsihal_reg_name reg, u32 n, u32 k)
{
	u32 offset;

	if (reg >= GSI_REG_MAX) {
		GSIERR("Invalid register reg=%u\n", reg);
		WARN_ON(1);
		return -EINVAL;
	}

	GSIDBG_LOW("get offset of %s k=%u n=%u\n",
		gsihal_reg_name_str(reg), k, n);
	offset = gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].offset;
	if (offset == -1) {
		GSIERR("Access to obsolete reg=%s\n",
			gsihal_reg_name_str(reg));
		WARN_ON(1);
		return -EPERM;
	}

	offset += gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].n_ofst * n;
	offset += gsihal_reg_objs[gsihal_ctx->gsi_ver][reg].k_ofst * k;

	return offset;
}

/*
* gsihal_get_bit_map_array_size() - Get the size of the bit map
*	array size according to the
*	GSI version.
*/
u32 gsihal_get_bit_map_array_size()
{
	return GSI_CH_BIT_MAP_ARR_SIZE;
}

/*
* gsihal_read_ch_reg() - Get the raw value of a ch reg
*/
u32 gsihal_read_ch_reg(enum gsihal_reg_name reg, u32 ch_num)
{
	return gsihal_read_reg_n(reg, GSI_CH_BIT_MAP_CELL_NUM(ch_num));
}

/*
 * gsihal_test_ch_bit() - return true if a ch bit is set
 */
bool gsihal_test_ch_bit(u32 reg_val, u32 ch_num)
{
	return !!(reg_val & GSI_BIT_MAP_CELL_MSK(ch_num));
}

/*
 * gsihal_get_ch_bit() - get ch bit set in the right offset
 */
u32 gsihal_get_ch_bit(u32 ch_num)
{
	return GSI_BIT_MAP_CELL_MSK(ch_num);
}

/*
 * gsihal_get_ch_reg_idx() - get ch reg index according to ch num
 */
u32 gsihal_get_ch_reg_idx(u32 ch_num)
{
	return GSI_CH_BIT_MAP_CELL_NUM(ch_num);
}

/*
 * gsihal_get_ch_reg_mask() - get ch reg mask according to ch num
 */
u32 gsihal_get_ch_reg_mask(u32 ch_num)
{
	return GSI_BIT_MAP_CELL_MSK(ch_num);
}

/*
 * Get the offset of a ch register according to ch index
 */
u32 gsihal_get_ch_reg_offset(enum gsihal_reg_name reg, u32 ch_num)
{
	return gsihal_get_reg_nk_ofst(reg, 0, GSI_CH_BIT_MAP_CELL_NUM(ch_num));
}

/*
* Get the offset of a ch n register according to ch index and n
*/
u32 gsihal_get_ch_reg_n_offset(enum gsihal_reg_name reg, u32 n, u32 ch_num)
{
	return gsihal_get_reg_nk_ofst(reg, GSI_CH_BIT_MAP_CELL_NUM(ch_num), n);
}

/*
 * gsihal_write_ch_bit_map_reg_n() - Write mask to ch reg a raw value
 */
void gsihal_write_ch_bit_map_reg_n(enum gsihal_reg_name reg, u32 n, u32 ch_num,
	u32 mask)
{
	gsihal_write_reg_nk(reg, n, GSI_CH_BIT_MAP_CELL_NUM(ch_num), mask);
}

/*
 * gsihal_write_set_ch_bit_map_reg_n() - Set ch bit in reg a raw value
 */
void gsihal_write_set_ch_bit_map_reg_n(enum gsihal_reg_name reg, u32 n,
	u32 ch_num)
{
	gsihal_write_reg_nk(reg, n, GSI_CH_BIT_MAP_CELL_NUM(ch_num),
		GSI_BIT_MAP_CELL_MSK(ch_num));
}

/*
 * Get GSI instruction ram MAX size
 */
unsigned long gsihal_get_inst_ram_size(void)
{
	unsigned long maxn;
	unsigned long size;

	switch (gsihal_ctx->gsi_ver) {
	case GSI_VER_1_0:
	case GSI_VER_1_2:
	case GSI_VER_1_3:
		maxn = GSI_GSI_INST_RAM_n_MAXn;
		break;
	case GSI_VER_2_0:
		maxn = GSI_V2_0_GSI_INST_RAM_n_MAXn;
		break;
	case GSI_VER_2_2:
		maxn = GSI_V2_2_GSI_INST_RAM_n_MAXn;
		break;
	case GSI_VER_2_5:
		maxn = GSI_V2_5_GSI_INST_RAM_n_MAXn;
		break;
	case GSI_VER_2_7:
		maxn = GSI_V2_7_GSI_INST_RAM_n_MAXn;
		break;
	case GSI_VER_2_9:
		maxn = GSI_V2_9_GSI_INST_RAM_n_MAXn;
		break;
	case GSI_VER_3_0:
		maxn = GSI_V3_0_GSI_INST_RAM_n_MAXn;
		break;
	case GSI_VER_ERR:
	case GSI_VER_MAX:
	default:
		GSIERR("GSI version is not supported %d\n",
			gsihal_ctx->gsi_ver);
		WARN_ON(1);
		return 0;
	}
	size = GSI_GSI_INST_RAM_n_WORD_SZ * (maxn + 1);

	return size;
}

int gsihal_reg_init(enum gsi_ver gsi_ver)
{
	int i;
	int j;
	struct gsihal_reg_obj zero_obj;

	GSIDBG_LOW("Entry - GSI ver = %d\n", gsi_ver);

	if ((gsi_ver < GSI_VER_1_0) || (gsi_ver >= GSI_VER_MAX)) {
		GSIERR("invalid GSI HW type (%d)\n", gsi_ver);
		return -EINVAL;
	}

	memset(&zero_obj, 0, sizeof(zero_obj));
	for (i = GSI_VER_1_0; i < gsi_ver; i++) {
		for (j = 0; j < GSI_REG_MAX; j++) {
			if (!memcmp(&gsihal_reg_objs[i + 1][j], &zero_obj,
				sizeof(struct gsihal_reg_obj))) {
				memcpy(&gsihal_reg_objs[i + 1][j],
					&gsihal_reg_objs[i][j],
					sizeof(struct gsihal_reg_obj));
			} else {
				/*
				* explicitly overridden register.
				* Check validity
				*/
				if (j != GSI_GSI_CFG && !gsihal_reg_objs[i + 1][j].offset) {
					GSIERR(
						"reg=%s with zero offset gsi_ver=%d\n",
						gsihal_reg_name_str(j), i + 1);
					WARN_ON(1);
				}
				if (!gsihal_reg_objs[i + 1][j].construct) {
					GSIERR(
						"reg=%s with NULL construct func gsi_ver=%d\n",
						gsihal_reg_name_str(j), i + 1);
					WARN_ON(1);
				}
				if (!gsihal_reg_objs[i + 1][j].parse) {
					GSIERR(
						"reg=%s with NULL parse func gsi_ver=%d\n",
						gsihal_reg_name_str(j), i + 1);
					WARN_ON(1);
				}
			}
		}
	}
	return 0;
}

/*
 * Check that ring length is valid
 */
bool gsihal_check_ring_length_valid(u32 r_len, u32 elem_size)
{
	if (gsihal_ctx->gsi_ver >= GSI_VER_3_0) {
		if (r_len & ~GSI_V3_0_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_BMSK) {
			GSIERR("bad params ring_len %u is out of bounds\n", r_len);
			return false;
		}
		if (r_len / elem_size >= GSI_V3_0_MAX_ELEMENTS_PER_RING) {
			GSIERR("bad params ring_len %u / re_size %u > 64k elements \n",
				r_len, elem_size);
			return false;
		}
	} else if (gsihal_ctx->gsi_ver >= GSI_VER_2_9) {
		if (r_len & ~GSI_V2_9_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_BMSK) {
			GSIERR("bad params ring_len %u is out of bounds\n", r_len);
			return false;
		}
	} else {
		if (r_len & ~GSI_EE_n_GSI_CH_k_CNTXT_1_R_LENGTH_BMSK) {
			GSIERR("bad params ring_len %u is out of bounds\n", r_len);
			return false;
		}
	}

	return true;
}

/*
 * Get mask for GP_int1
 */
u32 gsihal_get_glob_irq_en_gp_int1_mask()
{
	return GSI_EE_n_CNTXT_GLOB_IRQ_EN_GP_INT1_BMSK;
}
