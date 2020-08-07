/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
 */

#ifndef __H_HFI_DEFINITION_H__
#define __H_HFI_DEFINITION_H__

typedef u8 HFI_U8;
typedef s8 HFI_S8;
typedef u16 HFI_U16;
typedef s16 HFI_S16;
typedef u32 HFI_U32;
typedef s32 HFI_S32;
typedef u64 HFI_U64;

typedef HFI_U32 HFI_BOOL;
#define  HFI_FALSE      0
#define  HFI_TRUE       (!HFI_FALSE)

typedef HFI_U32 HFI_STATUS;
#define  HFI_FAIL       0
#define  HFI_SUCCESS    1

#define HFI_ALIGN(a, b) (((b) & ((b) - 1)) ? \
	(((a) + (b) - 1) / (b) * (b)) : \
	(((a) + (b) - 1) & (~((b) - 1))))

#endif // __H_HFI_DEFINITION_H__

