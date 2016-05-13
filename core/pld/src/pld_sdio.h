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

struct pld_fw_files fw_files_qca6174_fw_1_1 = {
	"qwlan11.bin", "bdwlan11.bin", "otp11.bin", "utf11.bin",
	"utfbd11.bin", "qsetup11.bin", "epping11.bin"};
struct pld_fw_files fw_files_qca6174_fw_2_0 = {
	"qwlan20.bin", "bdwlan20.bin", "otp20.bin", "utf20.bin",
	"utfbd20.bin", "qsetup20.bin", "epping20.bin"};
struct pld_fw_files fw_files_qca6174_fw_1_3 = {
	"qwlan13.bin", "bdwlan13.bin", "otp13.bin", "utf13.bin",
	"utfbd13.bin", "qsetup13.bin", "epping13.bin"};
struct pld_fw_files fw_files_qca6174_fw_3_0 = {
	"qwlan30.bin", "bdwlan30.bin", "otp30.bin", "utf30.bin",
	"utfbd30.bin", "qsetup30.bin", "epping30.bin"};
struct pld_fw_files fw_files_default = {
	"qwlan.bin", "bdwlan.bin", "otp.bin", "utf.bin",
	"utfbd.bin", "qsetup.bin", "epping.bin"};
#endif

#ifndef CONFIG_SDIO
static inline int pld_sdio_register_driver(void)
{
	return 0;
}

static inline void pld_sdio_unregister_driver(void)
{
}

int pld_sdio_get_fw_files_for_target(struct pld_fw_files *pfw_files,
				     u32 target_type, u32 target_version)
{
	return 0;
}
#else
int pld_sdio_register_driver(void);
void pld_sdio_unregister_driver(void);
int pld_sdio_get_fw_files_for_target(struct pld_fw_files *pfw_files,
				     u32 target_type, u32 target_version);
#endif

#endif
