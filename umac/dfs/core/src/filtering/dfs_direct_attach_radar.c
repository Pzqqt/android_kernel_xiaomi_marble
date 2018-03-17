/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
 * Copyright (c) 2011, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: This file has radar table and initialization function for Beeliner
 * family of chipsets.
 */

#include "../dfs.h"
#include "wlan_dfs_mlme_api.h"
#include "wlan_dfs_utils_api.h"
#include "wlan_dfs_lmac_api.h"
#include "../dfs_internal.h"

void dfs_get_da_radars(struct wlan_dfs *dfs)
{
#define AR5212_DEVID_IBM            0x1014 /* IBM minipci ID */
#define AR5212_AR2413               0x001a /* AR2413 aka Griffin-lite */
#define AR5212_AR2413               0x001a /* AR2413 aka Griffin-lite */
#define AR5212_AR5413               0x001b /* Eagle */
#define AR5212_AR5424               0x001c /* Condor (PCI express) */
#define AR5212_DEVID_FF19           0xff19 /* PCI express */
#define AR5212_AR2417               0x001d /* Nala, PCI */
#define AR5212_DEVID                0x0013 /* Final ar5212 devid */
#define AR5212_FPGA                 0xf013 /* Emulation board */
#define AR5212_DEFAULT              0x1113 /* No eeprom HW default */

#define AR5416_DEVID_PCI            0x0023 /* AR5416 PCI (CB/MB) (Owl)*/
#define AR5416_DEVID_PCIE           0x0024 /* AR5416 PCI-E (XB) (Owl) */
#define AR5416_DEVID_AR9160_PCI     0x0027 /* AR9160 PCI (Sowl) */
#define AR5416_AR9100_DEVID         0x000b /* AR9100 (Howl)    */
#define AR5416_DEVID_AR9280_PCI     0x0029 /* PCI (Merlin) */
#define AR5416_DEVID_AR9280_PCIE    0x002a /* PCIE (Merlin) */
#define AR5416_DEVID_AR9285_PCIE    0x002b /* PCIE (Kite) */
#define AR5416_DEVID_AR9285G_PCIE   0x002c /* PCIE (Kite G only) */
#define AR5416_DEVID_AR9287_PCI     0x002d /* PCI (Kiwi) */
#define AR5416_DEVID_AR9287_PCIE    0x002e /* PCIE (Kiwi) */

#define AR9300_DEVID_AR9380_PCIE    0x0030 /* PCIE (Osprey) */
#define AR9300_DEVID_AR9340         0x0031 /* Wasp */
#define AR9300_DEVID_AR9485_PCIE    0x0032 /* Poseidon */
#define AR9300_DEVID_AR9580_PCIE    0x0033 /* Peacock */
#define AR9300_DEVID_AR1111_PCIE    0x0037 /* AR1111 */
#define AR9300_DEVID_AR946X_PCIE    0x0034 /* Jupiter: 2x2 DB + BT - AR9462 */
#define AR9300_DEVID_AR955X         0x0039 /* Scorpion */
#define AR9300_DEVID_AR953X         0x003d /* Honey Bee */
#define AR9300_DEVID_AR956X         0x003f /* Dragonfly */
#define AR9300_DEVID_AR956X_PCIE    0x0036 /* Aphrodite: 1x1 DB + BT - AR9564 */
#define AR9300_DEVID_EMU_PCIE       0xabcd

	uint16_t devid = lmac_get_ah_devid(dfs->dfs_pdev_obj);
	/* For DA */

	switch (devid) {
	case AR5212_DEVID_IBM:
	case AR5212_AR2413:
	case AR5212_AR5413:
	case AR5212_AR5424:
	case AR5212_DEVID_FF19:
		devid = AR5212_DEVID;
	case AR5212_AR2417:
	case AR5212_DEVID:
	case AR5212_FPGA:
	case AR5212_DEFAULT:
		dfs_get_radars_for_ar5212(dfs);
		break;
	case AR5416_DEVID_PCI:
	case AR5416_DEVID_PCIE:
	case AR5416_DEVID_AR9160_PCI:
	case AR5416_AR9100_DEVID:
	case AR5416_DEVID_AR9280_PCI:
	case AR5416_DEVID_AR9280_PCIE:
	case AR5416_DEVID_AR9285_PCIE:
	case AR5416_DEVID_AR9285G_PCIE:
	case AR5416_DEVID_AR9287_PCI:
	case AR5416_DEVID_AR9287_PCIE:
		dfs_get_radars_for_ar5416(dfs);
		break;
	case AR9300_DEVID_AR9380_PCIE:
	case AR9300_DEVID_AR9340:
	case AR9300_DEVID_AR9485_PCIE:
	case AR9300_DEVID_AR9580_PCIE:
	case AR9300_DEVID_AR1111_PCIE:
	case AR9300_DEVID_AR946X_PCIE:
	case AR9300_DEVID_AR955X:
	case AR9300_DEVID_AR953X:
	case AR9300_DEVID_AR956X:
	case AR9300_DEVID_AR956X_PCIE:
	case AR9300_DEVID_EMU_PCIE:
		dfs_get_radars_for_ar9300(dfs);
		break;
	}
}
