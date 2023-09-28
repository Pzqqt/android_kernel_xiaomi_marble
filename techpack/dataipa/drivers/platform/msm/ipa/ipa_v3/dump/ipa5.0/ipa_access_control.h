/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */
#if !defined(_IPA_ACCESS_CONTROL_H_)
#define _IPA_ACCESS_CONTROL_H_

#include "ipa_reg_dump.h"

/*
 * AA_COMBO - actual read, actual write
 * AN_COMBO - actual read, no-op write
 * NA_COMBO - no-op read, actual write
 * NN_COMBO - no-op read, no-op write
 */

/*
 * The following is target specific.
 */
static struct reg_mem_access_map_t mem_access_map[] = {
	/*------------------------------------------------------------*/
	/*      Range               Use when              Use when    */
	/*  Begin    End           SD_ENABLED           SD_DISABLED   */
	/*------------------------------------------------------------*/
	{ 0x04000, 0x04FFF, { &io_matrix[AN_COMBO], &io_matrix[AN_COMBO] } },
	{ 0xA8000, 0xB7FFF, { &io_matrix[AN_COMBO], &io_matrix[AN_COMBO] } },
	{ 0x05000, 0x0EFFF, { &io_matrix[AN_COMBO], &io_matrix[AN_COMBO] } },
	{ 0x0F000, 0x0FFFF, { &io_matrix[AN_COMBO], &io_matrix[AN_COMBO] } },
	{ 0x18000, 0x29FFF, { &io_matrix[AA_COMBO], &io_matrix[AA_COMBO] } },
	{ 0x2A000, 0x3BFFF, { &io_matrix[AN_COMBO], &io_matrix[AN_COMBO] } },
	{ 0x3C000, 0x4DFFF, { &io_matrix[AN_COMBO], &io_matrix[AN_COMBO] } },
	{ 0x10000, 0x10FFF, { &io_matrix[AA_COMBO], &io_matrix[AA_COMBO] } },
	{ 0x11000, 0x11FFF, { &io_matrix[NN_COMBO], &io_matrix[NN_COMBO] } },
	{ 0x12000, 0x12FFF, { &io_matrix[NN_COMBO], &io_matrix[NN_COMBO] } },
	{ 0x14C000, 0x14CFFF, { &io_matrix[AA_COMBO], &io_matrix[AA_COMBO] } },
	{ 0x14D000, 0x14DFFF, { &io_matrix[NN_COMBO], &io_matrix[NN_COMBO] } },
	{ 0x14E000, 0x14FFFF, { &io_matrix[NN_COMBO], &io_matrix[NN_COMBO] } },
	{ 0x140000, 0x147FFF, { &io_matrix[AA_COMBO], &io_matrix[AA_COMBO] } },
	{ 0x148000, 0x14BFFF, { &io_matrix[AN_COMBO], &io_matrix[AN_COMBO] } },
	{ 0x150000, 0x15FFFF, { &io_matrix[AA_COMBO], &io_matrix[AA_COMBO] } },
	{ 0x160000, 0x17FFFF, { &io_matrix[AN_COMBO], &io_matrix[NN_COMBO] } },
	{ 0x180000, 0x180FFF, { &io_matrix[NN_COMBO], &io_matrix[NN_COMBO] } },
	{ 0x181000, 0x19FFFF, { &io_matrix[AN_COMBO], &io_matrix[AN_COMBO] } },
	{ 0x1A0000, 0x1BFFFF, { &io_matrix[AN_COMBO], &io_matrix[NN_COMBO] } },
	{ 0x1C0000, 0x1C1FFF, { &io_matrix[NN_COMBO], &io_matrix[NN_COMBO] } },
	{ 0x1C2000, 0x1C3FFF, { &io_matrix[AA_COMBO], &io_matrix[AA_COMBO] } },
};

#endif /* #if !defined(_IPA_ACCESS_CONTROL_H_) */
