/*
 * Copyright (c) 2012-2018, 2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef HIF_HW_VERSION_H
#define HIF_HW_VERSION_H

#define AR6004_VERSION_REV1_3           0x31c8088a
#define AR9888_REV2_VERSION             0x4100016c
#define AR9887_REV1_VERSION             0x4100016d
#define AR6320_REV1_VERSION             0x5000000
#define AR6320_REV1_1_VERSION           0x5000001
#define AR6320_REV1_3_VERSION           0x5000003
#define AR6320_REV2_1_VERSION           0x5010000
#define AR6320_REV3_VERSION             0x5020000
#define AR6320_REV3_2_VERSION           0x5030000
#define QCA9379_REV1_VERSION            0x5040000
#define AR6320_DEV_VERSION              0x1000000
#define QCA9377_REV1_1_VERSION          0x5020001
#define QCA6390_V1                      0x50040000
#define QCA6490_V1                      0x50060000
#define WCN3990_v1                      0x40000000
#define WCN3990_v2                      0x40010000
#define WCN3990_v2_1                    0x40010002
#define WCN3998                         0x40030001
#define AR900B_REV_2                    0x1
#define KIWI_V1                         0x40170100
#define KIWI_V2                         0x40170200
#define QCA6490_v2_1                    0x400c1211
#define QCA6490_v2                      0x400c0200
#define WCN3990_v2_2                    0x400f0000
#define WCN6750_V1                      0x40140110
#define WCN3990_TALOS                   0x40670000
#define WCN3990_MOOREA                  0x40090000
#define WCN3990_SAIPAN                  0x400B0000
#define WCN3990_RENNELL                 0x400C0000
#define WCN3990_BITRA                   0x400D0000
#define WCN3990_DIVAR                   0x40690000
#define WCN3990_ATHERTON                0x40680000
#define WCN3990_STRAIT                  0x400E0000
#define WCN3990_NETRANI                 0x400F0000
#define WCN3990_CLARENCE                0x406A0000

struct qwlan_hw {
	u32 id;
	u32 subid;
	const char *name;
};

#endif /* HIF_HW_VERSION_H */
