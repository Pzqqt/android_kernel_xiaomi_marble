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
 /**
  * DOC: This file provides the common definitions for object manager
  */

#ifndef _WLAN_CMN_H_
#define _WLAN_CMN_H_

/* Max no of UMAC components */
#define WLAN_UMAC_MAX_COMPONENTS 25
/* Max no. of radios, a pSoc/Device can support */
#define WLAN_UMAC_MAX_PDEVS 3
/* Max no. of VDEV per PSOC */
#define WLAN_UMAC_PSOC_MAX_VDEVS 51
/* Max no. of VDEVs, a PDEV can support */
#define WLAN_UMAC_PDEV_MAX_VDEVS 17
/* Max no. of Peers, a device can support */
#define WLAN_UMAC_PSOC_MAX_PEERS 1024

/* Max length of a SSID */
#define WLAN_SSID_MAX_LEN 32

/* Max no. of Stations can be associated to VDEV*/
#define WLAN_UMAC_MAX_AP_PEERS WLAN_UMAC_PSOC_MAX_PEERS
/* Max no. of peers for STA vap */
#define WLAN_UMAC_MAX_STA_PEERS 2

/* 802.11 cap info */
#define WLAN_CAPINFO_ESS               0x0001
#define WLAN_CAPINFO_IBSS              0x0002
#define WLAN_CAPINFO_CF_POLLABLE       0x0004
#define WLAN_CAPINFO_CF_POLLREQ        0x0008
#define WLAN_CAPINFO_PRIVACY           0x0010
#define WLAN_CAPINFO_SHORT_PREAMBLE    0x0020
#define WLAN_CAPINFO_PBCC              0x0040
#define WLAN_CAPINFO_CHNL_AGILITY      0x0080
#define WLAN_CAPINFO_SPECTRUM_MGMT     0x0100
#define WLAN_CAPINFO_QOS               0x0200
#define WLAN_CAPINFO_SHORT_SLOTTIME    0x0400
#define WLAN_CAPINFO_APSD              0x0800
#define WLAN_CAPINFO_RADIOMEAS         0x1000
#define WLAN_CAPINFO_DSSSOFDM          0x2000

/* Allowed time to wait for Object creation  */
#define WLAN_VDEV_CREATE_TIMEOUT_CNT 300
 /* 25 msec */
#define WLAN_VDEV_CREATE_TIMEOUT ((CONVERT_SEC_TO_SYSTEM_TIME(1)/40) + 1)

#define WLAN_PDEV_CREATE_TIMEOUT_CNT 300
 /* 25 msec */
#define WLAN_PDEV_CREATE_TIMEOUT ((CONVERT_SEC_TO_SYSTEM_TIME(1)/40) + 1)

#define WLAN_PSOC_CREATE_TIMEOUT_CNT 300
/* 25 msec */
#define WLAN_PSOC_CREATE_TIMEOUT ((CONVERT_SEC_TO_SYSTEM_TIME(1)/40) + 1)

/**
 * enum wlan_umac_comp_id - UMAC component id
 * @WLAN_UMAC_COMP_MLME:        MLME
 * @WLAN_UMAC_COMP_SCANMGR:     SCAN MGR
 * @WLAN_UMAC_COMP_SCANCACHE:   SCAN CACHE
 * @WLAN_UMAC_COMP_MGMT_TXRX:   MGMT Tx/Rx
 *
 * This id is static.
 * On Adding new component, new id has to be assigned
 */
enum wlan_umac_comp_id {
	WLAN_UMAC_COMP_MLME       = 0,
	WLAN_UMAC_COMP_SCANMGR    = 1,
	WLAN_UMAC_COMP_SCANCACHE  = 2,
	WLAN_UMAC_COMP_MGMT_TXRX  = 3,
};

/**
 *  enum WLAN_DEV_TYPE  - for DA or OL architecture types
 *  @WLAN_DEV_DA:       Direct attach
 *  @WLAN_DEV_OL:       Partial offload
 *  @WLAN_DEV_INVALID:  Invalid dev type
 */
typedef enum {
	WLAN_DEV_DA       = 0,
	WLAN_DEV_OL       = 1,
	WLAN_DEV_INVALID  = 3,
} WLAN_DEV_TYPE;

/**
 *  enum wlan_phymode - phy mode
 *  @WLAN_PHYMODE_AUTO:           autoselect
 *  @WLAN_PHYMODE_11A:            5GHz, OFDM
 *  @WLAN_PHYMODE_11B:            2GHz, CCK
 *  @WLAN_PHYMODE_11G:            2GHz, OFDM
 *  @WLAN_PHYMODE_11NA_HT20:      5Ghz, HT20
 *  @WLAN_PHYMODE_11NG_HT20:      2Ghz, HT20
 *  @WLAN_PHYMODE_11NA_HT40PLUS:  5Ghz, HT40 (ext ch +1)
 *  @WLAN_PHYMODE_11NA_HT40MINUS: 5Ghz, HT40 (ext ch -1)
 *  @WLAN_PHYMODE_11NG_HT40PLUS:  2Ghz, HT40 (ext ch +1)
 *  @WLAN_PHYMODE_11NG_HT40MINUS: 2Ghz, HT40 (ext ch -1)
 *  @WLAN_PHYMODE_11NG_HT40:      2Ghz, Auto HT40
 *  @WLAN_PHYMODE_11NA_HT40:      5Ghz, Auto HT40
 *  @WLAN_PHYMODE_11AC_VHT20:     5Ghz, VHT20
 *  @WLAN_PHYMODE_11AC_VHT40PLUS: 5Ghz, VHT40 (Ext ch +1)
 *  @WLAN_PHYMODE_11AC_VHT40MINUS:5Ghz  VHT40 (Ext ch -1)
 *  @WLAN_PHYMODE_11AC_VHT40:     5Ghz, VHT40
 *  @WLAN_PHYMODE_11AC_VHT80:     5Ghz, VHT80
 *  @WLAN_PHYMODE_11AC_VHT160:    5Ghz, VHT160
 *  @WLAN_PHYMODE_11AC_VHT80_80:  5Ghz, VHT80_80
 */
enum wlan_phymode {
	WLAN_PHYMODE_AUTO             = 0,
	WLAN_PHYMODE_11A              = 1,
	WLAN_PHYMODE_11B              = 2,
	WLAN_PHYMODE_11G              = 3,
	WLAN_PHYMODE_11NA_HT20        = 4,
	WLAN_PHYMODE_11NG_HT20        = 5,
	WLAN_PHYMODE_11NA_HT40PLUS    = 6,
	WLAN_PHYMODE_11NA_HT40MINUS   = 7,
	WLAN_PHYMODE_11NG_HT40PLUS    = 8,
	WLAN_PHYMODE_11NG_HT40MINUS   = 9,
	WLAN_PHYMODE_11NG_HT40        = 10,
	WLAN_PHYMODE_11NA_HT40        = 11,
	WLAN_PHYMODE_11AC_VHT20       = 12,
	WLAN_PHYMODE_11AC_VHT40PLUS   = 13,
	WLAN_PHYMODE_11AC_VHT40MINUS  = 14,
	WLAN_PHYMODE_11AC_VHT40       = 15,
	WLAN_PHYMODE_11AC_VHT80       = 16,
	WLAN_PHYMODE_11AC_VHT160      = 17,
	WLAN_PHYMODE_11AC_VHT80_80    = 18,
};

#define WLAN_PHYMODE_MAX      (WLAN_PHYMODE_11AC_VHT80_80 + 1)

/**
 * enum wlan_peer_type  - peer type
 * @WLAN_PEER_SELF:     for AP mode, SELF PEER or AP PEER are same
 * @WLAN_PEER_AP:       BSS peer for STA mode, Self peer for AP mode
 * @WLAN_PEER_STA:      Self Peer for STA mode, STA peer for AP mode
 * @WLAN_PEER_TDLS:     TDLS Peer
 * @WLAN_PEER_NAWDS:    NAWDS Peer
 * @WLAN_PEER_STA_TEMP: STA Peer Temp (its host only node)
 */
enum wlan_peer_type {
	WLAN_PEER_SELF     = 1,
	WLAN_PEER_AP       = 2,
	WLAN_PEER_STA      = 3,
	WLAN_PEER_TDLS     = 4,
	WLAN_PEER_NAWDS    = 5,
	WLAN_PEER_STA_TEMP = 6,
};

/* MAC address length */
#define WLAN_MACADDR_LEN 6
/* Util API to copy the MAC address */
#define WLAN_ADDR_COPY(dst, src)    qdf_mem_copy(dst, src, WLAN_MACADDR_LEN)
/* Util API to compare the MAC address */
#define WLAN_ADDR_EQ(a1, a2)   qdf_mem_cmp(a1, a2, WLAN_MACADDR_LEN)

#endif /* _WLAN_OBJMGR_CMN_H_*/
