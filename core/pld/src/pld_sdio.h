/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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

#ifndef __PLD_SDIO_H__
#define __PLD_SDIO_H__

#include "pld_common.h"

#ifndef CONFIG_CNSS
#define PLD_AR6004_VERSION_REV1_3           0x31c8088a
#define PLD_AR9888_REV2_VERSION             0x4100016c
#define PLD_AR6320_REV1_VERSION             0x5000000
#define PLD_AR6320_REV1_1_VERSION           0x5000001
#define PLD_AR6320_REV1_3_VERSION           0x5000003
#define PLD_AR6320_REV2_1_VERSION           0x5010000
#define PLD_AR6320_REV3_VERSION             0x5020000
#define PLD_AR6320_REV3_2_VERSION           0x5030000
#define PLD_AR6320_DEV_VERSION              0x1000000
#define PLD_QCA9377_REV1_1_VERSION          0x5020001

#ifdef MULTI_IF_NAME
#define PREFIX MULTI_IF_NAME
#else
#define PREFIX ""
#endif

struct pld_fw_files fw_files_qca6174_fw_1_1 = {
	PREFIX "qwlan11.bin", PREFIX  "bdwlan11.bin", PREFIX "otp11.bin",
	PREFIX  "utf11.bin", PREFIX "utfbd11.bin", PREFIX "qsetup11.bin",
	PREFIX "epping11.bin", ""};
struct pld_fw_files fw_files_qca6174_fw_2_0 = {
	PREFIX "qwlan20.bin", PREFIX "bdwlan20.bin", PREFIX "otp20.bin",
	PREFIX "utf20.bin", PREFIX "utfbd20.bin", PREFIX "qsetup20.bin",
	PREFIX "epping20.bin", ""};
struct pld_fw_files fw_files_qca6174_fw_1_3 = {
	PREFIX "qwlan13.bin", PREFIX "bdwlan13.bin", PREFIX "otp13.bin",
	PREFIX "utf13.bin", PREFIX "utfbd13.bin", PREFIX "qsetup13.bin",
	PREFIX "epping13.bin", ""};
struct pld_fw_files fw_files_qca6174_fw_3_0 = {
	PREFIX "qwlan30.bin", PREFIX "bdwlan30.bin", PREFIX "otp30.bin",
	PREFIX "utf30.bin", PREFIX "utfbd30.bin", PREFIX "qsetup30.bin",
	PREFIX "epping30.bin", PREFIX "qwlan30i.bin"};
struct pld_fw_files fw_files_default = {
	PREFIX "qwlan.bin", PREFIX "bdwlan.bin", PREFIX "otp.bin",
	PREFIX "utf.bin", PREFIX "utfbd.bin", PREFIX "qsetup.bin",
	PREFIX "epping.bin", ""};
#endif

#ifndef CONFIG_SDIO
static inline int pld_sdio_register_driver(void)
{
	return 0;
}

static inline void pld_sdio_unregister_driver(void)
{
}

static inline
int pld_sdio_get_fw_files_for_target(struct pld_fw_files *pfw_files,
				     u32 target_type, u32 target_version)
{
	return 0;
}
static inline uint8_t *pld_sdio_get_wlan_mac_address(struct device *dev,
						     uint32_t *num)
{
	*num = 0;
	return NULL;
}
#else
int pld_sdio_register_driver(void);
void pld_sdio_unregister_driver(void);
int pld_sdio_get_fw_files_for_target(struct pld_fw_files *pfw_files,
				     u32 target_type, u32 target_version);
static inline uint8_t *pld_sdio_get_wlan_mac_address(struct device *dev,
						     uint32_t *num)
{
	return cnss_common_get_wlan_mac_address(dev, num);
}
#endif

#endif
