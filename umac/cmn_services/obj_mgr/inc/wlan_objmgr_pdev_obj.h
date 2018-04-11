/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
 * DOC: Define the pdev data structure of UMAC
 * Public APIs to perform operations on Global objects
 */

#ifndef _WLAN_OBJMGR_PDEV_OBJ_H_
#define _WLAN_OBJMGR_PDEV_OBJ_H_

#include <wlan_objmgr_cmn.h>
#include "wlan_objmgr_psoc_obj.h"

/* STATUS: scanning */
#define WLAN_PDEV_F_SCAN                    0x00000001
/* STATUS: use short slot time*/
#define WLAN_PDEV_F_SHSLOT                  0x00000002
  /* STATUS: channel switch event pending after DFS RADAR */
#define WLAN_PDEV_F_DFS_CHANSWITCH_PENDING  0x00000004
  /* TX Power: fixed rate */
#define WLAN_PDEV_F_TXPOW_FIXED             0x00000008
  /* STATUS: use short preamble */
#define WLAN_PDEV_F_SHPREAMBLE              0x00000010
  /* CONF: do alignment pad */
#define WLAN_PDEV_F_DATAPAD                 0x00000020
  /* STATUS: protection enabled */
#define WLAN_PDEV_F_USEPROT                 0x00000040
  /* STATUS: use barker preamble*/
#define WLAN_PDEV_F_USEBARKER               0x00000080
  /* CONF: DISABLE 2040 coexistence */
#define WLAN_PDEV_F_COEXT_DISABLE           0x00000100
  /* STATE: scan pending */
#define WLAN_PDEV_F_SCAN_PENDING            0x00000200
  /* CONF: send regclassids in country ie */
#define WLAN_PDEV_F_REGCLASS                0x00000400
  /* CONF: block the use of DFS channels */
#define WLAN_PDEV_F_BLKDFSCHAN              0x00000800
  /* STATUS: 11D in used */
#define WLAN_PDEV_F_DOT11D                  0x00001000
  /* STATUS: 11D channel-switch detected */
#define WLAN_PDEV_F_RADAR                   0x00002000
  /* CONF: A-MPDU supported */
#define WLAN_PDEV_F_AMPDU                   0x00004000
  /* CONF: A-MSDU supported */
#define WLAN_PDEV_F_AMSDU                   0x00008000
  /* CONF: HT traffic protected */
#define WLAN_PDEV_F_HTPROT                  0x00010000
  /* CONF: Reset once */
#define WLAN_PDEV_F_RESET                   0x00020000
  /* CONF: ignore 11d beacon */
#define WLAN_PDEV_F_IGNORE_11D_BEACON       0x00040000
  /* HT CAP IE present */
#define WLAN_PDEV_F_HTVIE                   0x00080000
 /* radio in middle of CSA */
#define WLAN_PDEV_F_CSA_WAIT                0x00100000
 /* wnm support flag */
#define WLAN_PDEV_F_WNM                     0x00200000
#define WLAN_PDEV_F_2G_CSA                  0x00400000
  /* enhanced independent repeater  */
#define WLAN_PDEV_F_ENH_REP_IND             0x00800000
 /* Disable Tx AMSDU for station vap */
#define WLAN_PDEV_F_STA_AMPDU_DIS           0x01000000
/* do not send probe request in passive channel */
#define WLAN_PDEV_F_STRICT_PSCAN_EN         0x02000000
  /* dupie (ANA,pre ANA ) */
/*#define WLAN_PDEV_F_DUPIE                 0x00200000*/
 /* QWRAP enable flag */
#define WLAN_PDEV_F_WRAP_EN                 0x04000000

/* PDEV op flags */
   /* Enable htrate for wep and tkip */
#define WLAN_PDEV_OP_WEP_TKIP_HTRATE    0x00000001
  /* non HT AP found flag */
#define WLAN_PDEV_OP_NON_HT_AP          0x00000002
  /* block the use of DFS channels flag */
#define WLAN_PDEV_OP_BLK_DFS_CHAN       0x00000004
  /* 11.h flag */
#define WLAN_PDEV_OP_DOTH               0x00000008
  /* Off-channel support enabled */
#define WLAN_PDEV_OP_OFFCHAN            0x00000010
#define WLAN_PDEV_OP_HT20ADHOC          0x00000020
#define WLAN_PDEV_OP_HT40ADHOC          0x00000040
#define WLAN_PDEV_OP_HTADHOC_AGGR       0x00000080
   /* disallow CC change when assoc completes */
#define WLAN_PDEV_OP_DISALLOW_AUTO_CC   0x00000100
   /* Is P2P Enabled? */
#define WLAN_PDEV_OP_P2P                0x00000200
   /* disallowed  */
#define WLAN_PDEV_OP_IGNORE_DYNHALT     0x00000400
   /* overwrite probe response IE with beacon IE */
#define WLAN_PDEV_OP_OVERRIDE_PROBERESP 0x00000800
#define WLAN_PDEV_OP_DROPSTA_QUERY      0x00001000
#define WLAN_PDEV_OP_BLK_REPORT_FLOOD   0x00002000
   /* Offchan scan */
#define WLAN_PDEV_OP_OFFCHAN_SCAN       0x00004000
   /*Consider OBSS non-erp to change to long slot*/
#define WLAN_PDEV_OP_OBSS_LONGSLOT      0x00008000
   /* enable/disable min rssi cli block */
#define WLAN_PDEV_OP_MIN_RSSI_ENABLE    0x00010000


struct osif_pdev_priv;

/**
 * struct wlan_objmgr_pdev_nif  - pdev object nif structure
 * @pdev_fw_caps:       radio specific FW capabilities
 * @pdev_feature_caps:  radio specific feature capabilities
 * @pdev_ospriv:        OS specific pointer
 * @macaddr[]:          MAC address
 * @notified_ap_vdev:   ap vdev
 */
struct wlan_objmgr_pdev_nif {
	uint32_t pdev_fw_caps;
	uint32_t pdev_feature_caps;
	struct pdev_osif_priv *pdev_ospriv;
	uint8_t macaddr[QDF_MAC_ADDR_SIZE];
	uint8_t notified_ap_vdev;
};

/**
 * struct wlan_objmgr_pdev_mlme - pdev object mlme structure
 * @pdev_op_flags:    PDEV operation flags, can be used to know the
 *                    operation status (deletion progress, etc)
 */
struct wlan_objmgr_pdev_mlme {
	uint32_t pdev_op_flags;
};

/**
 * struct wlan_objmgr_pdev_objmgr - pdev object object manager structure
 * @wlan_pdev_id:      PDEV id
 * @wlan_vdev_count:   VDEVs count
 * @max_vdev_count:    Max no. of VDEVs supported by this PDEV
 * @print_cnt:         Count to throttle Logical delete prints
 * @wlan_vdev_list:    List maintains the VDEVs created on this PDEV
 * @wlan_peer_count:   Peer count
 * @max_peer_count:    Max Peer count
 * @temp_peer_count:   Temporary peer count
 * @wlan_psoc:         back pointer to PSOC, its attached to
 * @ref_cnt:           Ref count
 * @ref_id_dbg:        Array to track Ref count
 */
struct wlan_objmgr_pdev_objmgr {
	uint8_t wlan_pdev_id;
	uint8_t wlan_vdev_count;
	uint8_t max_vdev_count;
	uint8_t print_cnt;
	qdf_list_t wlan_vdev_list;
	uint16_t wlan_peer_count;
	uint16_t max_peer_count;
	uint16_t temp_peer_count;
	struct wlan_objmgr_psoc *wlan_psoc;
	qdf_atomic_t ref_cnt;
	qdf_atomic_t ref_id_dbg[WLAN_REF_ID_MAX];
};

/**
 * struct wlan_objmgr_pdev - PDEV common object
 * @current_chan_list: Active/current Channel list of the radio
 * @pdev_nif:          pdev nif structure
 * @pdev_objmgr:       pdev object manager structure
 * @pdev_mlme:         pdev MLME structure
 * @pdev_comp_priv_obj[]:   component's private object array
 * @obj_status[]:      object status of each component object
 * @obj_state:         object state
 * @tgt_if_handle:     Target interface handle
 * @dp_handle:         DP module handle
 * @pdev_lock:         lock to protect object
*/
struct wlan_objmgr_pdev {
	struct wlan_chan_list *current_chan_list;
	struct wlan_objmgr_pdev_nif  pdev_nif;
	struct wlan_objmgr_pdev_objmgr pdev_objmgr;
	struct wlan_objmgr_pdev_mlme   pdev_mlme;
	void *pdev_comp_priv_obj[WLAN_UMAC_MAX_COMPONENTS];
	QDF_STATUS obj_status[WLAN_UMAC_MAX_COMPONENTS];
	WLAN_OBJ_STATE obj_state;
	void *tgt_if_handle;
	void *dp_handle;
	qdf_spinlock_t pdev_lock;
};

/**
 ** APIs to Create/Delete Global object APIs
 */
/**
 * wlan_objmgr_pdev_obj_create() - pdev create
 * @psoc: PSOC object
 * @scn: os private object
 *
 * Creates PDEV object, intializes with default values
 * Invokes the registered notifiers to create component object
 *
 * Return: Handle to struct wlan_objmgr_psoc on successful creation,
 *         NULL on Failure (on Mem alloc failure and Component objects
 *         Failure)
 */
struct wlan_objmgr_pdev *wlan_objmgr_pdev_obj_create(
	struct wlan_objmgr_psoc *psoc, struct pdev_osif_priv *osif_priv);

/**
 * wlan_objmgr_pdev_obj_delete() - pdev delete
 * @psoc: PDEV object
 *
 * Logically deletes PDEV object,
 * Once all the references are released, object manager invokes the registered
 * notifiers to destroy component objects
 *
 * Return: SUCCESS/FAILURE
 */
QDF_STATUS wlan_objmgr_pdev_obj_delete(struct wlan_objmgr_pdev *pdev);

/**
 ** APIs to attach/detach component objects
 */
/**
 * wlan_objmgr_pdev_component_obj_attach() - pdev comp object attach
 * @psoc: PDEV object
 * @id: Component id
 * @comp_priv_obj: component's private object pointer
 * @status: Component's private object creation status
 *
 * API to be used for attaching component object with PDEV common object
 *
 * Return: SUCCESS on successful storing of component's object in common object
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS wlan_objmgr_pdev_component_obj_attach(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj,
		QDF_STATUS status);

/**
 * wlan_objmgr_pdev_component_obj_detach() - pdev comp object detach
 * @psoc: PDEV object
 * @id: Component id
 * @comp_priv_obj: component's private object pointer
 *
 * API to be used for detaching component object with PDEV common object
 *
 * Return: SUCCESS on successful removal of component's object from common
 *         object
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS wlan_objmgr_pdev_component_obj_detach(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj);

/**
 ** APIs to operations on pdev objects
 */

typedef void (*wlan_objmgr_pdev_op_handler)(struct wlan_objmgr_pdev *pdev,
					void *object,
					void *arg);

/**
 * wlan_objmgr_pdev_iterate_obj_list() - operate on all objects of pdev
 * @pdev: PDEV object
 * @obj_type: VDEV_OP/PEER_OP
 * @handler: the handler will be called for each object of requested type
 *           the handler should be implemented to perform required operation
 * @arg: agruments passed by caller
 * @lock_free_op: its obsolete
 * @dbg_id: id of the caller
 *
 * API to be used for performing the operations on all VDEV/PEER objects
 * of pdev
 *
 * Return: SUCCESS/FAILURE
 */
QDF_STATUS wlan_objmgr_pdev_iterate_obj_list(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_objmgr_obj_type obj_type,
		wlan_objmgr_pdev_op_handler handler,
		void *arg, uint8_t lock_free_op,
		wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_trigger_pdev_comp_priv_object_creation() - create
 * comp object of pdev
 * @pdev: PDEV object
 * @id: Component id
 *
 * API to create component private object in run time, this would be
 * used for features which gets enabled in run time
 *
 * Return: SUCCESS on successful creation
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS wlan_objmgr_trigger_pdev_comp_priv_object_creation(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_umac_comp_id id);

/**
 * wlan_objmgr_trigger_pdev_comp_priv_object_deletion() - destroy
 * comp object of pdev
 * @pdev: PDEV object
 * @id: Component id
 *
 * API to destroy component private object in run time, this would
 * be used for features which gets disabled in run time
 *
 * Return: SUCCESS on successful deletion
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS wlan_objmgr_trigger_pdev_comp_priv_object_deletion(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_umac_comp_id id);

/**
 * wlan_objmgr_get_vdev_by_id_from_pdev() - find vdev using id from pdev
 * @pdev: PDEV object
 * @vdev_id: vdev id
 * @dbg_id: id of the caller
 *
 * API to find vdev object pointer by vdev id from pdev's vdev list
 *
 * This API increments the ref count of the vdev object internally, the
 * caller has to invoke the wlan_objmgr_vdev_release_ref() to decrement
 * ref count
 *
 * Return: vdev pointer
 *         NULL on FAILURE
 */
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_pdev(
			struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_get_vdev_by_id_from_pdev_no_state() - find vdev using id from
 *                                                      pdev
 * @pdev: PDEV object
 * @vdev_id: vdev id
 * @dbg_id: id of the caller
 *
 * API to find vdev object pointer by vdev id from pdev's vdev list
 *
 * This API increments the ref count of the vdev object internally, the
 * caller has to invoke the wlan_objmgr_vdev_release_ref() to decrement
 * ref count
 *
 * Return: vdev pointer
 *         NULL on FAILURE
 */
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_pdev_no_state(
			struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_get_vdev_by_macaddr_from_pdev() - find vdev using macaddr
 * @pdev: PDEV object
 * @macaddr: MAC address
 * @dbg_id: id of the caller
 *
 * API to find vdev object pointer by vdev mac addr from pdev's vdev list
 *
 * This API increments the ref count of the vdev object internally, the
 * caller has to invoke the wlan_objmgr_vdev_release_ref() to decrement
 * ref count
 *
 * Return: vdev pointer
 *         NULL on FAILURE
 */
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_pdev(
		struct wlan_objmgr_pdev *pdev, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_get_vdev_by_macaddr_from_pdev_no_state() - find vdev using
 *                                                           macaddr
 * @pdev: PDEV object
 * @macaddr: MAC address
 * @dbg_id: id of the caller
 *
 * API to find vdev object pointer by vdev mac addr from pdev's vdev list
 *
 * This API increments the ref count of the vdev object internally, the
 * caller has to invoke the wlan_objmgr_vdev_release_ref() to decrement
 * ref count
 *
 * Return: vdev pointer
 *         NULL on FAILURE
 */
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_pdev_no_state(
		struct wlan_objmgr_pdev *pdev, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_pdev_get_comp_private_obj() - get pdev component private object
 * @pdev: PDEV object
 * @id: Component id
 *
 * API to get component private object
 *
 * Return: void *ptr on SUCCESS
 *         NULL on Failure
 */
void *wlan_objmgr_pdev_get_comp_private_obj(
		struct wlan_objmgr_pdev *pdev,
		enum wlan_umac_comp_id id);

/**
 * wlan_pdev_obj_lock() - Acquire PDEV spinlock
 * @pdev: PDEV object
 *
 * API to acquire PDEV lock
 * Parent lock should not be taken in child lock context
 * but child lock can be taken in parent lock context
 * (for ex: psoc lock can't be invoked in pdev/vdev/peer lock context)
 *
 * Return: void
 */
static inline void wlan_pdev_obj_lock(struct wlan_objmgr_pdev *pdev)
{
	qdf_spin_lock_bh(&pdev->pdev_lock);
}

/**
 * wlan_pdev_obj_unlock() - Release PDEV spinlock
 * @pdev: PDEV object
 *
 * API to Release PDEV lock
 *
 * Return: void
 */
static inline void wlan_pdev_obj_unlock(struct wlan_objmgr_pdev *pdev)
{
	qdf_spin_unlock_bh(&pdev->pdev_lock);
}

/**
 * wlan_pdev_get_psoc() - get psoc
 * @pdev: PDEV object
 *
 * API to get the psoc object from PDEV
 *
 * Return:
 * @psoc: PSOC object
 */
static inline struct wlan_objmgr_psoc *wlan_pdev_get_psoc(
			struct wlan_objmgr_pdev *pdev)
{
	return pdev->pdev_objmgr.wlan_psoc;
}

/**
 * wlan_pdev_set_psoc() - set psoc
 * @pdev: PDEV object
 * @psoc: PSOC object
 *
 * API to set the psoc object from PDEV
 *
 * Return: void
 */
static inline void wlan_pdev_set_psoc(struct wlan_objmgr_pdev *pdev,
				struct wlan_objmgr_psoc *psoc)
{
	pdev->pdev_objmgr.wlan_psoc = psoc;
}

/**
 * wlan_pdev_nif_fw_cap_set() - set fw caps
 * @pdev: PDEV object
 * @cap: capability flag to be set
 *
 * API to set fw caps in pdev
 *
 * Return: void
 */
static inline void wlan_pdev_nif_fw_cap_set(struct wlan_objmgr_pdev *pdev,
				uint32_t cap)
{
	pdev->pdev_nif.pdev_fw_caps |= cap;
}

/**
 * wlan_pdev_nif_fw_cap_clear() - clear fw cap
 * @pdev: PDEV object
 * @cap: capability flag to be cleared
 *
 * API to clear fw caps in pdev
 *
 * Return: void
 */
static inline void wlan_pdev_nif_fw_cap_clear(struct wlan_objmgr_pdev *pdev,
				uint32_t cap)
{
	pdev->pdev_nif.pdev_fw_caps &= ~cap;
}

/**
 * wlan_pdev_nif_fw_cap_get() - get fw caps
 * @pdev: PDEV object
 * @cap: capability flag to be checked
 *
 * API to know, whether particular fw caps flag is set in pdev
 *
 * Return: 1 (for set) or 0 (for not set)
 */
static inline uint8_t wlan_pdev_nif_fw_cap_get(struct wlan_objmgr_pdev *pdev,
				uint32_t cap)
{
	return (pdev->pdev_nif.pdev_fw_caps & cap) ? 1 : 0;
}

/**
 * wlan_pdev_nif_feat_cap_set() - set feature caps
 * @pdev: PDEV object
 * @cap: capability flag to be set
 *
 * API to set feat caps in pdev
 *
 * Return: void
 */
static inline void wlan_pdev_nif_feat_cap_set(struct wlan_objmgr_pdev *pdev,
				uint32_t cap)
{
	pdev->pdev_nif.pdev_feature_caps |= cap;
}

/**
 * wlan_pdev_nif_feat_cap_clear() - clear feature caps
 * @pdev: PDEV object
 * @cap: capability flag to be cleared
 *
 * API to clear feat caps in pdev
 *
 * Return: void
 */
static inline void wlan_pdev_nif_feat_cap_clear(struct wlan_objmgr_pdev *pdev,
				uint32_t cap)
{
	pdev->pdev_nif.pdev_feature_caps &= ~cap;
}

/**
 * wlan_pdev_nif_feat_cap_get() - get feature caps
 * @pdev: PDEV object
 * @cap: capability flag to be checked
 *
 * API to know, whether particular feat caps flag is set in pdev
 *
 * Return: 1 (for set) or 0 (for not set)
 */
static inline uint8_t wlan_pdev_nif_feat_cap_get(struct wlan_objmgr_pdev *pdev,
				uint32_t cap)
{
	return (pdev->pdev_nif.pdev_feature_caps & cap) ? 1 : 0;
}

/**
 * wlan_pdev_get_hw_macaddr() - get hw macaddr
 * @pdev: PDEV object
 *
 * API to get HW MAC address form PDEV
 *
 * Caller need to acquire lock with wlan_pdev_obj_lock()
 *
 * Return: @macaddr -MAC address
 */
static inline uint8_t *wlan_pdev_get_hw_macaddr(struct wlan_objmgr_pdev *pdev)
{
	if (!pdev)
		return NULL;

	/* This API is invoked with lock acquired, do not add log prints */
	return pdev->pdev_nif.macaddr;
}

/**
 * wlan_pdev_set_hw_macaddr() - set hw macaddr
 * @pdev: PDEV object
 * @macaddr: MAC address
 *
 * API to set HW MAC address form PDEV
 *
 * Caller need to acquire lock with wlan_pdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_pdev_set_hw_macaddr(struct wlan_objmgr_pdev *pdev,
			uint8_t *macaddr)
{
	/* This API is invoked with lock acquired, do not add log prints */
	WLAN_ADDR_COPY(pdev->pdev_nif.macaddr, macaddr);
}

/**
 * wlan_pdev_get_ospriv() - get os priv pointer
 * @pdev: PDEV object
 *
 * API to get OS private pointer from PDEV
 *
 * Return: ospriv - private pointer
 */
static inline struct pdev_osif_priv *wlan_pdev_get_ospriv(
				struct wlan_objmgr_pdev *pdev)
{
	return pdev->pdev_nif.pdev_ospriv;
}

/**
 * wlan_pdev_reset_ospriv() - reset os priv pointer
 * @pdev: PDEV object
 *
 * API to reset OS private pointer in PDEV
 *
 * Return: void
 */
static inline void wlan_pdev_reset_ospriv(struct wlan_objmgr_pdev *pdev)
{
	pdev->pdev_nif.pdev_ospriv = NULL;
}

/**
 * wlan_pdev_set_max_vdev_count() - set pdev max vdev count
 * @pdev: PDEV object
 * @vdev count: Max vdev count
 *
 * API to set Max vdev count
 *
 * Return: void
 */
static inline void wlan_pdev_set_max_vdev_count(struct wlan_objmgr_pdev *pdev,
					   uint8_t max_vdev_count)
{
	pdev->pdev_objmgr.max_vdev_count = max_vdev_count;
}

/**
 * wlan_pdev_get_max_vdev_count() - get pdev max vdev count
 * @pdev: PDEV object
 *
 * API to set Max vdev count
 *
 * Return: @vdev count: Max vdev count
 */
static inline uint8_t wlan_pdev_get_max_vdev_count(
					struct wlan_objmgr_pdev *pdev)
{
	return pdev->pdev_objmgr.max_vdev_count;
}

/**
 * DOC: Examples to use PDEV ref count APIs
 *
 * In all the scenarios, the pair of API should be followed
 * otherwise it lead to memory leak
 *
 *  scenario 1:
 *
 *     wlan_objmgr_pdev_obj_create()
 *     ----
 *     wlan_objmgr_pdev_obj_delete()
 *
 *  scenario 2:
 *
 *     wlan_objmgr_pdev_get_ref()
 *     ----
 *     the operations which are done on
 *     pdev object
 *     ----
 *     wlan_objmgr_pdev_release_ref()
 *
 *  scenario 3:
 *
 *     wlan_objmgr_get_pdev_by_id[_no_state]()
 *     ----
 *     the operations which are done on
 *     pdev object
 *     ----
 *     wlan_objmgr_pdev_release_ref()
 *
 *  scenario 4:
 *
 *     wlan_objmgr_get_pdev_by_macaddr[_no_state]()
 *     ----
 *     the operations which are done on
 *     pdev object
 *     ----
 *     wlan_objmgr_pdev_release_ref()
 */

/**
 * wlan_objmgr_pdev_get_ref() - increment ref count
 * @pdev: PDEV object
 * @id:   Object Manager ref debug id
 *
 * API to increment ref count of pdev
 *
 * Return: void
 */
void wlan_objmgr_pdev_get_ref(struct wlan_objmgr_pdev *pdev,
					wlan_objmgr_ref_dbgid id);

/**
 * wlan_objmgr_pdev_try_get_ref() - increment ref count, if allowed
 * @pdev: PDEV object
 * @id:   Object Manager ref debug id
 *
 * API to increment ref count of pdev after checking valid object state
 *
 * Return: void
 */
QDF_STATUS wlan_objmgr_pdev_try_get_ref(struct wlan_objmgr_pdev *pdev,
						wlan_objmgr_ref_dbgid id);

/**
 * wlan_objmgr_pdev_release_ref() - decrement ref count
 * @pdev: PDEV object
 * @id:   Object Manager ref debug id
 *
 * API to decrement ref count of pdev, if ref count is 1, it initiates the
 * PDEV deletion
 *
 * Return: void
 */
void wlan_objmgr_pdev_release_ref(struct wlan_objmgr_pdev *pdev,
						wlan_objmgr_ref_dbgid id);

/**
 * wlan_objmgr_pdev_get_first_vdev() - Get first vdev of pdev
 * @pdev: PDEV object
 * @dbg_id:   Object Manager ref debug id
 *
 * API to get reference to first vdev of pdev.
 *
 * Return: reference to first vdev
 */
struct wlan_objmgr_vdev *wlan_objmgr_pdev_get_first_vdev(
		struct wlan_objmgr_pdev *pdev,
		wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_pdev_get_pdev_id() - get pdev id
 * @pdev: PDEV object
 *
 * API to get pdev id from pdev object
 *
 * Return: @pdev id
 */
static inline
uint8_t wlan_objmgr_pdev_get_pdev_id(struct wlan_objmgr_pdev *pdev)
{
	return pdev->pdev_objmgr.wlan_pdev_id;
}

/**
 * wlan_pdev_set_tgt_if_handle(): API to set target if handle in pdev object
 * @pdev: Pdev pointer
 * @tgt_if_handle: target interface handle
 *
 * API to set target interface handle in pdev object
 *
 * Caller needs to acquire lock with wlan_pdev_obj_lock()
 *
 * Return: None
 */
static inline void wlan_pdev_set_tgt_if_handle(struct wlan_objmgr_pdev *pdev,
			void *tgt_if_handle)
{
	/* This API is invoked with lock acquired, do not add log prints */
	if (pdev == NULL)
		return;

	pdev->tgt_if_handle = tgt_if_handle;
}

/**
 * wlan_pdev_get_tgt_if_handle(): API to get target interface handle
 * @pdev: Pdev pointer
 *
 * API to get target interface handle from pdev object
 *
 * Return: target interface handle
 */
static inline void *wlan_pdev_get_tgt_if_handle(struct wlan_objmgr_pdev *pdev)
{
	if (pdev == NULL)
		return NULL;

	return pdev->tgt_if_handle;
}

/**
 * wlan_pdev_set_max_peer_count() - set max peer count
 * @vdev: PDEV object
 * @count: Max peer count
 *
 * API to set max peer count of PDEV
 *
 * Return: void
 */
static inline void wlan_pdev_set_max_peer_count(struct wlan_objmgr_pdev *pdev,
						uint16_t count)
{
	pdev->pdev_objmgr.max_peer_count = count;
}

/**
 * wlan_pdev_get_max_peer_count() - get max peer count
 * @pdev: PDEV object
 *
 * API to get max peer count of PDEV
 *
 * Return: max peer count
 */
static inline uint16_t wlan_pdev_get_max_peer_count(
						struct wlan_objmgr_pdev *pdev)
{
	return pdev->pdev_objmgr.max_peer_count;
}

/**
 * wlan_pdev_get_peer_count() - get pdev peer count
 * @pdev: PDEV object
 *
 * API to get peer count from PDEV
 *
 * Return: peer_count - pdev's peer count
 */
static inline uint16_t wlan_pdev_get_peer_count(struct wlan_objmgr_pdev *pdev)
{
	return pdev->pdev_objmgr.wlan_peer_count;
}

/**
 * wlan_pdev_get_temp_peer_count() - get pdev temporary peer count
 * @pdev: PDEV object
 *
 * API to get temporary peer count from PDEV
 *
 * Return: temp_peer_count - pdev's temporary peer count
 */
static inline uint16_t wlan_pdev_get_temp_peer_count(struct wlan_objmgr_pdev *pdev)
{
	return pdev->pdev_objmgr.temp_peer_count;
}


/**
 * wlan_pdev_incr_peer_count() - increment pdev peer count
 * @pdev: PDEV object
 *
 * API to increment peer count of PDEV by 1
 *
 * Return: void
 */
static inline void wlan_pdev_incr_peer_count(struct wlan_objmgr_pdev *pdev)
{
	pdev->pdev_objmgr.wlan_peer_count++;
}

/**
 * wlan_pdev_decr_peer_count() - decrement pdev peer count
 * @pdev: PDEV object
 *
 * API to decrement peer count of PDEV by 1
 *
 * Return: void
 */
static inline void wlan_pdev_decr_peer_count(struct wlan_objmgr_pdev *pdev)
{
	pdev->pdev_objmgr.wlan_peer_count--;
}

/**
 * wlan_pdev_incr_temp_peer_count() - increment temporary pdev peer count
 * @pdev: PDEV object
 *
 * API to increment temporary  peer count of PDEV by 1
 *
 * Return: void
 */
static inline void wlan_pdev_incr_temp_peer_count(struct wlan_objmgr_pdev *pdev)
{
	pdev->pdev_objmgr.temp_peer_count++;
}

/**
 * wlan_pdev_decr_temp_peer_count() - decrement pdev temporary peer count
 * @pdev: PDEV object
 *
 * API to decrement temporary peer count of PDEV by 1
 *
 * Return: void
 */
static inline void wlan_pdev_decr_temp_peer_count(struct wlan_objmgr_pdev *pdev)
{
	pdev->pdev_objmgr.temp_peer_count--;
}

/**
 * wlan_pdev_get_vdev_count() - get PDEV vdev count
 * @pdev: PDEV object
 *
 * API to get vdev count from PDEV
 *
 * Return: vdev_count - pdev's vdev count
 */
static inline uint8_t wlan_pdev_get_vdev_count(struct wlan_objmgr_pdev *pdev)
{
	return pdev->pdev_objmgr.wlan_vdev_count;
}

/**
 * wlan_pdev_set_dp_handle() - set dp handle
 * @pdev: pdev object pointer
 * @dp_handle: Data path module handle
 *
 * Return: void
 */
static inline void wlan_pdev_set_dp_handle(struct wlan_objmgr_pdev *pdev,
		void *dp_handle)
{
	if (qdf_unlikely(!pdev)) {
		QDF_BUG(0);
		return;
	}

	pdev->dp_handle = dp_handle;
}

/**
 * wlan_pdev_get_dp_handle() - get dp handle
 * @pdev: pdev object pointer
 *
 * Return: dp handle
 */
static inline void *wlan_pdev_get_dp_handle(struct wlan_objmgr_pdev *pdev)
{
	if (qdf_unlikely(!pdev)) {
		QDF_BUG(0);
		return NULL;
	}

	return pdev->dp_handle;
}

#endif /* _WLAN_OBJMGR_PDEV_H_*/
