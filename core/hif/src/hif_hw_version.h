/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */


#ifndef HIF_HW_VERSION_H
#define HIF_HW_VERSION_H

#define AR6320_REV1_VERSION             0x5000000
#define AR6320_REV1_1_VERSION           0x5000001
#define AR6320_REV1_3_VERSION           0x5000003
#define AR6320_REV2_1_VERSION           0x5010000
#define AR6320_REV3_VERSION             0x5020000
#define AR6320_REV3_2_VERSION           0x5030000

struct qwlan_hw {
	u32 id;
	u32 subid;
	const char *name;
};

static const struct qwlan_hw qwlan_hw_list[] = {
	{
		.id = AR6320_REV1_VERSION,
		.subid = 0,
		.name = "QCA6174_REV1",
	},
	{
		.id = AR6320_REV1_1_VERSION,
		.subid = 0x1,
		.name = "QCA6174_REV1_1",
	},
	{
		.id = AR6320_REV1_3_VERSION,
		.subid = 0x2,
		.name = "QCA6174_REV1_3",
	},
	{
		.id = AR6320_REV2_1_VERSION,
		.subid = 0x4,
		.name = "QCA6174_REV2_1",
	},
	{
		.id = AR6320_REV2_1_VERSION,
		.subid = 0x5,
		.name = "QCA6174_REV2_2",
	},
	{
		.id = AR6320_REV3_VERSION,
		.subid = 0x6,
		.name = "QCA6174_REV2.3",
	},
	{
		.id = AR6320_REV3_VERSION,
		.subid = 0x8,
		.name = "QCA6174_REV3",
	},
	{
		.id = AR6320_REV3_VERSION,
		.subid = 0x9,
		.name = "QCA6174_REV3_1",
	},
	{
		.id = AR6320_REV3_2_VERSION,
		.subid = 0xA,
		.name = "AR6320_REV3_2_VERSION",
	}
};

#endif /* HIF_HW_VERSION_H */
