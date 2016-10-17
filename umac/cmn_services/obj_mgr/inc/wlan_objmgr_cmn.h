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
  * DOC: This file provides the common defintions for object manager
  */

#ifndef _WLAN_OBJMGR_CMN_H_
#define _WLAN_OBJMGR_CMN_H_

#include "qdf_lock.h"
#include "qdf_list.h"
#include "qdf_status.h"

/* No. of PSOCs can be supported */
#define WLAN_OBJMGR_MAX_DEVICES 2
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
 * enum WLAN_OBJ_STATE - State of Object
 * @WLAN_OBJ_STATE_CREATED:             All component objects are created
 * @WLAN_OBJ_STATE_DELETED:             All component objects are deleted
 * @WLAN_OBJ_STATE_PARTIALLY_CREATED:   Few/All component objects creation is
 *                                      in progress
 * @WLAN_OBJ_STATE_PARTIALLY_DELETED:   Few/All component objects yet to be
 *                                      deleted
 * @WLAN_OBJ_STATE_COMP_DEL_PROGRESS:   If a component is disabled run time,
 *                                      and this state is used to represent the
 *                                      deletion in progress after that
 *                                      component object is deleted, object
 *                                      state would be moved to CREATED state
 * @WLAN_OBJ_STATE_CREATION_FAILED:     any component object is failed to be
 *                                      created
 * @WLAN_OBJ_STATE_DELETION_FAILED:     any component object is failed to be
 *                                      deleted
 */
typedef enum {
	WLAN_OBJ_STATE_CREATED            = 0,
	WLAN_OBJ_STATE_DELETED            = 1,
	WLAN_OBJ_STATE_PARTIALLY_CREATED  = 2,
	WLAN_OBJ_STATE_PARTIALLY_DELETED  = 3,
	WLAN_OBJ_STATE_COMP_DEL_PROGRESS  = 4,
	WLAN_OBJ_STATE_CREATION_FAILED    = 5,
	WLAN_OBJ_STATE_DELETION_FAILED    = 6,
} WLAN_OBJ_STATE;

/* Object type is assigned with value */
enum wlan_objmgr_obj_type {
	WLAN_PSOC_OP  = 0,
	WLAN_PDEV_OP  = 1,
	WLAN_VDEV_OP  = 2,
	WLAN_PEER_OP  = 3,
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

/* size of Hash */
#define WLAN_PEER_HASHSIZE 64
/* simple hash is enough for variation of macaddr */
#define WLAN_PEER_HASH(addr)   \
(((const uint8_t *)(addr))[WLAN_MACADDR_LEN - 1] % WLAN_PEER_HASHSIZE)

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
 * struct wlan_peer_list {
 * @peer_hash[]:    peer sub lists
 * @peer_list_lock: List lock, this has to be acquired on
 *		    accessing/updating the list
 *
 *  Peer list, it maintains sublists based on the MAC address as hash
 *  Note: For DA WDS similar peer list has to be maintained
 *  This peer list will not have WDS nodes
 */
struct wlan_peer_list {
	qdf_list_t peer_hash[WLAN_PEER_HASHSIZE];
	qdf_spinlock_t peer_list_lock;
};

struct wlan_objmgr_psoc;
struct wlan_objmgr_pdev;
struct wlan_objmgr_vdev;
struct wlan_objmgr_peer;

/* Create handler would return the following status
	QDF_STATUS_SUCCESS--
		For synchronous handler:- this is returned on successful
	component object creation

	QDF_STATUS_COMP_DISABLED--
		For synchronous handler:- this is returned on if component
	doesn't want to allocate

	QDF_STATUS_COMP_ASYNC--
		For asynchronous handler:- this is returned on if component
	needs a context break

	QDF_STATUS_E_NOMEM--
		For synchronous handler:- this is returned on if component
	can't allocate
	QDF_STATUS_E_FAILURE--
		For synchronous handler:- If it is failed,
		For asynchronous handler:- If it is failed to post message
	(means, not required)/feature is not supported
*/
typedef QDF_STATUS (*wlan_objmgr_psoc_create_handler)(
				struct wlan_objmgr_psoc *psoc, void *arg);
typedef QDF_STATUS (*wlan_objmgr_psoc_delete_handler)(
				struct wlan_objmgr_psoc *psoc, void *arg);
typedef void (*wlan_objmgr_psoc_status_handler)(struct wlan_objmgr_psoc *psoc,
					 void *arg, QDF_STATUS status);

typedef QDF_STATUS (*wlan_objmgr_pdev_create_handler)(
				struct wlan_objmgr_pdev *pdev, void *arg);
typedef QDF_STATUS (*wlan_objmgr_pdev_delete_handler)(
				struct wlan_objmgr_pdev *pdev, void *arg);
typedef void (*wlan_objmgr_pdev_status_handler)(
				struct wlan_objmgr_pdev *pdev, void *arg,
						QDF_STATUS status);

typedef QDF_STATUS (*wlan_objmgr_vdev_create_handler)(
				struct wlan_objmgr_vdev *vdev, void *arg);
typedef QDF_STATUS (*wlan_objmgr_vdev_delete_handler)(
				struct wlan_objmgr_vdev *vdev, void *arg);
typedef void (*wlan_objmgr_vdev_status_handler)(
				struct wlan_objmgr_vdev *vdev, void *arg,
						QDF_STATUS status);

typedef QDF_STATUS (*wlan_objmgr_peer_create_handler)(
				struct wlan_objmgr_peer *peer, void *arg);
typedef QDF_STATUS (*wlan_objmgr_peer_delete_handler)(
				struct wlan_objmgr_peer *peer, void *arg);
typedef void (*wlan_objmgr_peer_status_handler)(
				struct wlan_objmgr_peer *peer, void *arg,
						QDF_STATUS status);

#endif /* _WLAN_OBJMGR_CMN_H_*/
