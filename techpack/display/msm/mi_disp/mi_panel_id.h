/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2020 XiaoMi, Inc. All rights reserved.
 */

#ifndef _MI_PANEL_ID_H_
#define _MI_PANEL_ID_H_

#include <linux/types.h>
#include "dsi_panel.h"

/*
   Naming Rules,Wiki Dec .
   4Byte : Project ASCII Value .Exemple "L18" ASCII Is 004C3138
   1Byte : Prim Panel Is 'P' ASCII Value , Sec Panel Is 'S' Value
   1Byte : Panel Vendor
   1Byte : DDIC Vendor ,Samsung 0x0C Novatek 0x02
   1Byte : Production Batch Num
*/
#define L1_42_02_0A_PANEL_ID   0x00004C3100380C00
#define L2_38_0C_0A_PANEL_ID   0x00004C3200380C00
#define L2S_38_0C_0A_PANEL_ID  0x004C325300380C00
#define L2S_42_02_0B_PANEL_ID  0x004C325300420200
#define L3_42_02_0A_PANEL_ID   0x00004C3300420200
#define L3S_42_02_0A_PANEL_ID  0x004C335300420200
#define L10_42_02_0A_PANEL_ID  0x004C313000420200
#define L18_38_0C_PA_PANEL_ID  0x004C313850380C00
#define L18_38_0C_SA_PANEL_ID  0x004C313853380C00
#define L9S_42_02_0A_PANEL_ID  0x004C395300420200
#define L9S_36_02_0B_PANEL_ID  0x004C395300360200
#define L12_42_02_0A_PANEL_ID  0x004C313200420200
#define L12_36_02_0B_PANEL_ID  0x004C313200360200
#define M11A_42_02_0A_PANEL_ID 0x4D31314100420200
#define M16T_36_02_0A_PANEL_ID 0x4D31365400360200
#define M16T_36_02_0B_PANEL_ID 0x4D31365400360201
#define M80_42_02_0A_PANEL_ID  0x004D383000420200
#define N16_36_0D_0A_PANEL_ID  0x004E313600360d00
#define N16_42_02_OB_PANEL_ID  0x004e313600420201

/* PA: Primary display, First selection screen
 * PB: Primary display, Second selection screen
 * SA: Secondary display, First selection screen
 * SB: Secondary display, Second selection screen
 */
enum mi_project_panel_id {
	PANEL_ID_INVALID = 0,
	L1_PANEL_PA,
	L2_PANEL_PA,
	L2S_PANEL_PA,
	L2S_PANEL_PB,
	L3_PANEL_PA,
	L3S_PANEL_PA,
	L10_PANEL_PA,
	L18_PANEL_PA,
	L18_PANEL_SA,
	L9S_PANEL_PA,
	L9S_PANEL_PB,
	L12_PANEL_PA,
	L12_PANEL_PB,
	M11A_PANEL_PA,
	M16T_PANEL_PA,
	M16T_PANEL_PB,
	M80_PANEL_PA,
	N16_PANEL_PA,
	N16_PANEL_PB,
	PANEL_ID_MAX
};

static inline enum mi_project_panel_id mi_get_panel_id(u64 mi_panel_id)
{
	switch(mi_panel_id) {
	case L1_42_02_0A_PANEL_ID:
		return L1_PANEL_PA;
	case L2_38_0C_0A_PANEL_ID:
		return L2_PANEL_PA;
	case L2S_38_0C_0A_PANEL_ID:
		return L2S_PANEL_PA;
	case L2S_42_02_0B_PANEL_ID:
		return L2S_PANEL_PB;
	case L3_42_02_0A_PANEL_ID:
		return L3_PANEL_PA;
	case L3S_42_02_0A_PANEL_ID:
		return L3S_PANEL_PA;
	case L10_42_02_0A_PANEL_ID:
		return L10_PANEL_PA;
	case L18_38_0C_PA_PANEL_ID:
		return L18_PANEL_PA;
	case L18_38_0C_SA_PANEL_ID:
		return L18_PANEL_SA;
	case L9S_42_02_0A_PANEL_ID:
		return L9S_PANEL_PA;
	case L9S_36_02_0B_PANEL_ID:
		return L9S_PANEL_PB;
	case L12_42_02_0A_PANEL_ID:
		return L12_PANEL_PA;
	case L12_36_02_0B_PANEL_ID:
		return L12_PANEL_PB;
	case M11A_42_02_0A_PANEL_ID:
		return M11A_PANEL_PA;
	case M16T_36_02_0A_PANEL_ID:
		return M16T_PANEL_PA;
	case M16T_36_02_0B_PANEL_ID:
		return M16T_PANEL_PB;
	case M80_42_02_0A_PANEL_ID:
		return M80_PANEL_PA;
	case N16_36_0D_0A_PANEL_ID:
		return N16_PANEL_PA;
	case N16_42_02_OB_PANEL_ID:
		return N16_PANEL_PB;
	default:
		return PANEL_ID_INVALID;
	}
}

static inline const char *mi_get_panel_id_name(u64 mi_panel_id)
{
	switch (mi_get_panel_id(mi_panel_id)) {
	case L1_PANEL_PA:
		return "L1_PANEL_PA";
	case L2_PANEL_PA:
		return "L2_PANEL_PA";
	case L2S_PANEL_PA:
		return "L2S_PANEL_PA";
	case L2S_PANEL_PB:
		return "L2S_PANEL_PB";
	case L3_PANEL_PA:
		return "L3_PANEL_PA";
	case L3S_PANEL_PA:
		return "L3S_PANEL_PA";
	case L10_PANEL_PA:
		return "L10_PANEL_PA";
	case L18_PANEL_PA:
		return "L18_PANEL_PA";
	case L18_PANEL_SA:
		return "L18_PANEL_SA";
	case L9S_PANEL_PA:
		return "L9S_PANEL_PA";
	case L9S_PANEL_PB:
		return "L9S_PANEL_PB";
	case M11A_PANEL_PA:
		return "M11A_PANEL_PA";
	case M16T_PANEL_PA:
		return "M16T_PANEL_PA";
	case M16T_PANEL_PB:
		return "M16T_PANEL_PB";
	case M80_PANEL_PA:
		return "M80_PANEL_PA";
	case N16_PANEL_PA:
		return "N16_PANEL_PA";
	case N16_PANEL_PB:
		return "N16_PANEL_PB";
	default:
		return "unknown";
	}
}

static inline bool is_use_nvt_dsc_config(u64 mi_panel_id)
{
	switch(mi_panel_id) {
	case L3_42_02_0A_PANEL_ID:
	case L3S_42_02_0A_PANEL_ID:
	case L10_42_02_0A_PANEL_ID:
	case L9S_42_02_0A_PANEL_ID:
	case L9S_36_02_0B_PANEL_ID:
	case L12_42_02_0A_PANEL_ID:
	case L12_36_02_0B_PANEL_ID:
	case M11A_42_02_0A_PANEL_ID:
	case M16T_36_02_0A_PANEL_ID:
	case M16T_36_02_0B_PANEL_ID:
	case M80_42_02_0A_PANEL_ID:
	case N16_42_02_OB_PANEL_ID:
		return true;
	default:
		return false;
	}
}

static inline bool is_use_nt37705_dsc_config(u64 mi_panel_id)
{
	switch(mi_panel_id) {
	case L12_36_02_0B_PANEL_ID:
		return true;
	default:
		return false;
	}
}

static inline bool is_use_nt36532_dsc_config(u64 mi_panel_id)
{
	switch(mi_panel_id) {
	case M80_42_02_0A_PANEL_ID:
		return true;
	default:
		return false;
	}
}

static inline bool is_use_nt37801_dsc_config(u64 mi_panel_id)
{
	switch(mi_panel_id) {
	case M11A_42_02_0A_PANEL_ID:
		return true;
	default:
		return false;
	}
}

static inline bool is_use_nt37703_dsc_config(u64 mi_panel_id)
{
	switch(mi_panel_id) {
	case M16T_36_02_0A_PANEL_ID:
	case M16T_36_02_0B_PANEL_ID:
	case M80_42_02_0A_PANEL_ID:
		return true;
	default:
		return false;
	}
}

static inline bool is_use_nt37706_dsc_config(u64 mi_panel_id)
{
	switch(mi_panel_id) {
	case N16_42_02_OB_PANEL_ID:
		return true;
	default:
		return false;
	}
}

enum mi_project_panel_id mi_get_panel_id_by_dsi_panel(struct dsi_panel *panel);
enum mi_project_panel_id mi_get_panel_id_by_disp_id(int disp_id);

#endif /* _MI_PANEL_ID_H_ */
