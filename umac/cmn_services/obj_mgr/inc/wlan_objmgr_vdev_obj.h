/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
 * DOC: Define the vdev data structure of UMAC
 */

#ifndef _WLAN_OBJMGR_VDEV_OBJ_H_
#define _WLAN_OBJMGR_VDEV_OBJ_H_

#include "qdf_atomic.h"
#include "qdf_list.h"
#include "qdf_lock.h"
#include "qdf_types.h"
#include "wlan_cmn.h"
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_psoc_obj.h"

	/* CONF: privacy enabled */
#define WLAN_VDEV_F_PRIVACY              0x00000001
	/* CONF: 11g w/o 11b sta's */
#define WLAN_VDEV_F_PUREG                0x00000002
	/* CONF: des_bssid is set */
#define WLAN_VDEV_F_DESBSSID             0x00000004
	/* CONF: bg scan enabled */
#define WLAN_VDEV_F_BGSCAN               0x00000008
	/* CONF: sw tx retry enabled */
#define WLAN_VDEV_F_SWRETRY              0x00000010
	/* STATUS: update beacon tim */
#define WLAN_VDEV_F_TIMUPDATE            0x00000020
	/* CONF: WPA enabled */
#define WLAN_VDEV_F_WPA1                 0x00000040
	/* CONF: WPA2 enabled */
#define WLAN_VDEV_F_WPA2                 0x00000080
	/* CONF: WPA/WPA2 enabled */
#define WLAN_VDEV_F_WPA                  0x000000c0
	/* CONF: drop unencrypted */
#define WLAN_VDEV_F_DROPUNENC            0x00000100
	/* CONF: TKIP countermeasures */
#define WLAN_VDEV_F_COUNTERM             0x00000200
	/* CONF: hide SSID in beacon */  /*TODO PDEV/PSOC */
#define WLAN_VDEV_F_HIDESSID             0x00000400
	/* CONF: disable internal bridge */ /*TODO PDEV/PSOC */
#define WLAN_VDEV_F_NOBRIDGE             0x00000800
	/* STATUS: update beacon wme */
#define WLAN_VDEV_F_WMEUPDATE            0x00001000
	/* CONF: 4 addr allowed */
#define WLAN_VDEV_F_WDS                  0x00002000
	/* CONF: enable U-APSD */
#define WLAN_VDEV_F_UAPSD                0x00004000
	/* STATUS: sleeping */
#define WLAN_VDEV_F_SLEEP                0x00008000
	/* drop uapsd EOSP frames for test */
#define WLAN_VDEV_F_EOSPDROP             0x00010000
	/* drop uapsd EOSP frames for test */
#define WLAN_VDEV_F_AMPDU                0x00020000
	/* STATE: beacon APP IE updated */
#define WLAN_VDEV_F_APPIE_UPDATE         0x00040000
	/* CONF: WDS auto Detect/DELBA */
#define WLAN_VDEV_F_WDS_AUTODETECT       0x00080000
	/* 11b only without 11g stations */
#define WLAN_VDEV_F_PUREB                0x00100000
	/* disable HT rates */
#define WLAN_VDEV_F_HTRATES              0x00200000
	/* Extender AP */
#define WLAN_VDEV_F_AP                   0x00400000
	/* CONF: deliver rx frames with 802.11 header */
#define WLAN_VDEV_F_DELIVER_80211        0x00800000
	/* CONF: os sends down tx frames with 802.11 header */
#define WLAN_VDEV_F_SEND_80211           0x01000000
	/* CONF: statically configured WDS */
#define WLAN_VDEV_F_WDS_STATIC           0x02000000
	/* CONF: pure 11n mode */
#define WLAN_VDEV_F_PURE11N              0x04000000
	/* CONF: pure 11ac mode */
#define WLAN_VDEV_F_PURE11AC             0x08000000
	/* Basic Rates Update */
#define WLAN_VDEV_F_BR_UPDATE            0x10000000
	/* CONF: restrict bw ont top of per 11ac/n */
#define WLAN_VDEV_F_STRICT_BW            0x20000000
	/* Wi-Fi SON mode (with APS) */
#define WLAN_VDEV_F_SON                  0x40000000
	/* Wi-Fi SON mode (with APS) */
#define WLAN_VDEV_F_MBO                  0x80000000

/* Feature extension flags */
		/* CONF: MSFT safe mode         */
#define WLAN_VDEV_FEXT_SAFEMODE             0x00000001
		/* if the vap can sleep*/
#define WLAN_VDEV_FEXT_CANSLEEP             0x00000002
		/* use sw bmiss timer */
#define WLAN_VDEV_FEXT_SWBMISS              0x00000004
		/* enable beacon copy */
#define WLAN_VDEV_FEXT_COPY_BEACON          0x00000008
#define WLAN_VDEV_FEXT_WAPI                 0x00000010
		/* 802.11h enabled */
#define WLAN_VDEV_FEXT_DOTH                 0x00000020
	/* if the vap has wds independance set */
#define WLAN_VDEV_FEXT_VAPIND               0x00000040
	/* QBSS load IE enabled */
#define WLAN_VDEV_FEXT_BSSLOAD              0x00000080
	/* Short Guard Interval Enable:1 Disable:0 */
#define WLAN_VDEV_FEXT_SGI                  0x00000100
	/* Short Guard Interval Enable:1 Disable:0 for VHT fixed rates */
#define WLAN_VDEV_FEXT_DATASGI              0x00000200
	/* LDPC Enable Rx:1 TX: 2 ; Disable:0 */
#define WLAN_VDEV_FEXT_LDPC_TX              0x00000400
#define WLAN_VDEV_FEXT_LDPC_RX              0x00000800
#define WLAN_VDEV_FEXT_LDPC                 0x00000c00
	/* wme enabled */
#define WLAN_VDEV_FEXT_WME                  0x00001000
	/* WNM Capabilities */
#define WLAN_VDEV_FEXT_WNM                  0x00002000
	/* RRM Capabilities */
#define WLAN_VDEV_FEXT_RRM                  0x00004000
	/* WNM Proxy ARP Capabilities */
#define WLAN_VDEV_FEXT_PROXYARP             0x00008000
	/* 256 QAM support in 2.4GHz mode Enable:1 Disable:0 */
#define WLAN_VDEV_FEXT_256QAM               0x00010000
	/* 2.4NG 256 QAM Interop mode Enable:1 Disable:0 */
#define WLAN_VDEV_FEXT_256QAM_INTEROP       0x00020000
	/* static mimo ps enabled */
#define WLAN_VDEV_FEXT_STATIC_MIMOPS        0x00040000
	/* dynamic mimo ps enabled */
#define WLAN_VDEV_FEXT_DYN_MIMOPS           0x00080000
	/* Country IE enabled */
#define WLAN_VDEV_FEXT_CNTRY_IE             0x00100000
	/*does not want to trigger multi channel operation
	instead follow master vaps channel (for AP/GO Vaps) */
#define WLAN_VDEV_FEXT_NO_MULCHAN           0x00200000
	/*non-beaconing AP VAP*/
#define WLAN_VDEV_FEXT_NON_BEACON           0x00400000
/* SPL repeater enabled for SON*/
#define WLAN_VDEV_FEXT_SON_SPL_RPT          0x00800000
/* SON IE update in MGMT frame */
#define WLAN_VDEV_FEXT_SON_INFO_UPDATE      0x01000000

/* VDEV OP flags  */
  /* if the vap destroyed by user */
#define WLAN_VDEV_OP_DELETE_PROGRESS        0x00000001
 /* set to enable sta-fws fweature */
#define WLAN_VDEV_OP_STAFWD                 0x00000002
   /* Off-channel support enabled */
#define WLAN_VDEV_OP_OFFCHAN                0x00000004
  /* if the vap has erp update set */
#define WLAN_VDEV_OP_ERPUPDATE              0x00000008
  /* this vap needs scheduler for off channel operation */
#define WLAN_VDEV_OP_NEEDS_SCHED            0x00000010
  /*STA in forced sleep set PS bit for all outgoing frames */
#define WLAN_VDEV_OP_FORCED_SLEEP           0x00000020
  /* update bssload IE in beacon */
#define WLAN_VDEV_OP_BSSLOAD_UPDATE         0x00000040
  /* Hotspot 2.0 DGAF Disable bit */
#define WLAN_VDEV_OP_DGAF_DISABLE           0x00000080
  /* STA SmartNet enabled */
#define WLAN_VDEV_OP_SMARTNET_EN            0x00000100
  /* SoftAP to reject resuming in DFS channels */
#define WLAN_VDEV_OP_REJ_DFS_CHAN           0x00000200
  /* Trigger mlme response */
#define WLAN_VDEV_OP_TRIGGER_MLME_RESP      0x00000400
  /* test flag for MFP */
#define WLAN_VDEV_OP_MFP_TEST               0x00000800
  /* flag to indicate using default ratemask */
#define WLAN_VDEV_OP_DEF_RATEMASK           0x00001000
/*For wakeup AP VAP when wds-sta connect to the AP only use when
	export (UMAC_REPEATER_DELAYED_BRINGUP || DBDC_REPEATER_SUPPORT)=1*/
#define WLAN_VDEV_OP_KEYFLAG                0x00002000
  /* if performe the iwlist scanning */
#define WLAN_VDEV_OP_LIST_SCANNING          0x00004000
   /*Set when VAP down*/
#define WLAN_VDEV_OP_IS_DOWN                0x00008000
  /* if vap may require acs when another vap is brought down */
#define WLAN_VDEV_OP_NEEDS_UP_ACS           0x00010000
  /* Block data traffic tx for this vap */
#define WLAN_VDEV_OP_BLOCK_TX_TRAFFIC       0x00020000
  /* for mbo functionality */
#define WLAN_VDEV_OP_MBO                    0x00040000

 /* CAPABILITY: IBSS available */
#define WLAN_VDEV_C_IBSS                    0x00000001
/* CAPABILITY: HOSTAP avail */
#define WLAN_VDEV_C_HOSTAP               0x00000002
   /* CAPABILITY: Old Adhoc Demo */
#define WLAN_VDEV_C_AHDEMO               0x00000004
  /* CAPABILITY: sw tx retry */
#define WLAN_VDEV_C_SWRETRY              0x00000008
  /* CAPABILITY: monitor mode */
#define WLAN_VDEV_C_MONITOR              0x00000010
  /* CAPABILITY: TKIP MIC avail */
#define WLAN_VDEV_C_TKIPMIC              0x00000020
  /* CAPABILITY: 4-addr support */
#define WLAN_VDEV_C_WDS                  0x00000040
  /* CAPABILITY: TKIP MIC for QoS frame */
#define WLAN_VDEV_C_WME_TKIPMIC          0x00000080
  /* CAPABILITY: bg scanning */
#define WLAN_VDEV_C_BGSCAN               0x00000100
  /* CAPABILITY: Restrict offchannel */
#define WLAN_VDEV_C_RESTRICT_OFFCHAN     0x00000200

/* Invalid VDEV identifier */
#define WLAN_INVALID_VDEV_ID 255

/**
 * enum wlan_vdev_state - VDEV state
 * @WLAN_VDEV_S_INIT:    Default state, IDLE state
 * @WLAN_VDEV_S_SCAN:    SCAN state
 * @WLAN_VDEV_S_JOIN:    Join state
 * @WLAN_VDEV_S_DFS_WAIT:CAC period
 * @WLAN_VDEV_S_RUN:     RUN state
 * @WLAN_VDEV_S_STOP:    STOP state
 * @WLAN_VDEV_S_RESET:   RESET state, STOP+INIT+JOIN
 * @WLAN_VDEV_S_MAX:     MAX state
 */
enum wlan_vdev_state {
	WLAN_VDEV_S_INIT     = 0,
	WLAN_VDEV_S_SCAN     = 1,
	WLAN_VDEV_S_JOIN     = 2,
	WLAN_VDEV_S_DFS_WAIT = 3,
	WLAN_VDEV_S_RUN      = 4,
	WLAN_VDEV_S_STOP     = 5,
	WLAN_VDEV_S_RESET    = 6,
	WLAN_VDEV_S_MAX,
};

/**
 * struct wlan_vdev_create_params - Create params, HDD/OSIF passes this
 *				    structure While creating VDEV
 * @opmode:      Opmode of VDEV
 * @macaddr[]:   MAC address
 * @flags:       create flags
 * @osifp:       OS structure
 * @mataddr[]:   MAT address
 */
struct wlan_vdev_create_params {
	enum tQDF_ADAPTER_MODE opmode;
	uint8_t macaddr[WLAN_MACADDR_LEN];
	uint32_t flags;
	struct vdev_osif_priv *osifp;
	uint8_t mataddr[WLAN_MACADDR_LEN];
};

/**
 * struct wlan_channel - channel structure
 */
struct wlan_channel {

};

/**
 * struct wlan_objmgr_vdev_mlme - VDEV MLME specific sub structure
 * @vdev_opmode:        Opmode of VDEV
 * @ssid[]:             SSID
 * @ssid_len:           SSID length
 * @bss_chan:           BSS channel
 * @des_chan:           Desired channel, for STA Desired may not be used
 * @nss:                Num. Spatial streams
 * @tx_chainmask:       Tx Chainmask
 * @rx_chainmask:       Rx Chainmask
 * @macaddr[]:          VDEV self MAC address
 * @vdev_caps:          VDEV capabilities
 * @vdev_feat_caps:     VDEV feature caps
 * @vdev_feat_ext_caps: VDEV Extended feature caps
 * @max_rate:           MAX rate
 * @tx_mgmt_rate:       TX Mgmt. Rate
 * @tx_power:           Tx power
 * @mlme_state:         VDEV state
 * @vdev_op_flags:      Operation flags
 * @mataddr[]:          MAT address
 */
struct wlan_objmgr_vdev_mlme {
	enum tQDF_ADAPTER_MODE vdev_opmode;
	char ssid[WLAN_SSID_MAX_LEN+1];
	uint8_t ssid_len;
	struct wlan_channel  *bss_chan;   /* Define wlan_channel */
	struct wlan_channel  *des_chan;  /*TODO ??? */
	uint8_t nss;
	uint8_t tx_chainmask;
	uint8_t rx_chainmask;
	uint8_t  macaddr[WLAN_MACADDR_LEN];
	uint32_t vdev_caps;
	uint32_t vdev_feat_caps;
	uint32_t vdev_feat_ext_caps;
	uint32_t max_rate;
	uint32_t tx_mgmt_rate;
	uint8_t  tx_power;
	enum wlan_vdev_state mlme_state;
	uint32_t vdev_op_flags;
	uint8_t  mataddr[WLAN_MACADDR_LEN];
};

/**
 *  struct wlan_objmgr_vdev_nif - VDEV HDD specific sub structure
 *  @osdev:  OS specific pointer
 */
struct wlan_objmgr_vdev_nif {
	struct vdev_osif_priv *osdev;
};

/**
 *  struct wlan_objmgr_vdev_objmgr - vdev object manager sub structure
 *  @vdev_id:           VDEV id
 *  @self_peer:         Self PEER
 *  @bss_peer:          BSS PEER
 *  @wlan_peer_list:    PEER list
 *  @wlan_pdev:         PDEV pointer
 *  @wlan_peer_count:   Peer count
 *  @max_peer_count:    Max Peer count
 *  @c_flags:           creation specific flags
 *  @ref_cnt:           Ref count
 *  @ref_id_dbg:        Array to track Ref count
 */
struct wlan_objmgr_vdev_objmgr {
	uint8_t vdev_id;
	struct wlan_objmgr_peer *self_peer;
	struct wlan_objmgr_peer *bss_peer;
	qdf_list_t wlan_peer_list;
	struct wlan_objmgr_pdev *wlan_pdev;
	uint16_t wlan_peer_count;
	uint16_t max_peer_count;
	uint32_t c_flags;
	qdf_atomic_t ref_cnt;
	qdf_atomic_t ref_id_dbg[WLAN_REF_ID_MAX];
};

/**
 * struct wlan_objmgr_vdev - VDEV common object
 * @vdev_node:      qdf list of pdev's vdev list
 * @vdev_mlme:      VDEV MLME substructure
 * @vdev_objmgr:    VDEV Object Mgr substructure
 * @vdev_nif:       VDEV HDD substructure
 * @vdev_comp_priv_obj[]:Component's private objects list
 * @obj_status[]:   Component object status
 * @obj_state:      VDEV object state
 * @vdev_lock:      VDEV lock
 */
struct wlan_objmgr_vdev {
	qdf_list_node_t vdev_node;
	struct wlan_objmgr_vdev_mlme vdev_mlme;
	struct wlan_objmgr_vdev_objmgr vdev_objmgr;
	struct wlan_objmgr_vdev_nif vdev_nif;
	void *vdev_comp_priv_obj[WLAN_UMAC_MAX_COMPONENTS];
	QDF_STATUS obj_status[WLAN_UMAC_MAX_COMPONENTS];
	WLAN_OBJ_STATE obj_state;
	qdf_spinlock_t vdev_lock;
};

/**
 ** APIs to Create/Delete Global object APIs
 */
/**
 * wlan_objmgr_vdev_obj_create() - vdev object create
 * @pdev: PDEV object on which this vdev gets created
 * @params: VDEV create params from HDD
 *
 * Creates vdev object, intializes with default values
 * Attaches to psoc and pdev objects
 * Invokes the registered notifiers to create component object
 *
 * Return: Handle to struct wlan_objmgr_vdev on successful creation,
 *         NULL on Failure (on Mem alloc failure and Component objects
 *         Failure)
 */
struct wlan_objmgr_vdev *wlan_objmgr_vdev_obj_create(
			struct wlan_objmgr_pdev *pdev,
			struct wlan_vdev_create_params *params);

/**
 * wlan_objmgr_vdev_obj_delete() - vdev object delete
 * @vdev: vdev object
 *
 * Logically deletes VDEV object,
 * Once all the references are released, object manager invokes the registered
 * notifiers to destroy component objects
 *
 * Return: SUCCESS/FAILURE
 */
QDF_STATUS wlan_objmgr_vdev_obj_delete(struct wlan_objmgr_vdev *vdev);

/**
 ** APIs to attach/detach component objects
 */
/**
 * wlan_objmgr_vdev_component_obj_attach() - vdev comp object attach
 * @vdev: VDEV object
 * @id: Component id
 * @comp_priv_obj: component's private object pointer
 * @status: Component's private object creation status
 *
 * API to be used for attaching component object with VDEV common object
 *
 * Return: SUCCESS on successful storing of component's object in common object
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS wlan_objmgr_vdev_component_obj_attach(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj,
		QDF_STATUS status);

/**
 * wlan_objmgr_vdev_component_obj_detach() - vdev comp object detach
 * @vdev: VDEV object
 * @id: Component id
 * @comp_priv_obj: component's private object pointer
 *
 * API to be used for detaching component object with VDEV common object
 *
 * Return: SUCCESS on successful removal of component's object from common
 *         object
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS wlan_objmgr_vdev_component_obj_detach(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj);
/*
 ** APIs to operations on vdev objects
*/

typedef void (*wlan_objmgr_vdev_op_handler)(struct wlan_objmgr_vdev *vdev,
					void *object,
					void *arg);

/**
 * wlan_objmgr_iterate_peerobj_list() - iterate vdev's peer list
 * @vdev: vdev object
 * @handler: the handler will be called for each object of requested type
 *            the handler should be implemented to perform required operation
 * @arg:     agruments passed by caller
 * @dbg_id: id of the caller
 *
 * API to be used for performing the operations on all PEER objects
 * of vdev
 *
 * Return: SUCCESS/FAILURE
 */
QDF_STATUS wlan_objmgr_iterate_peerobj_list(
		struct wlan_objmgr_vdev *vdev,
		wlan_objmgr_vdev_op_handler handler,
		void *arg, wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_trigger_vdev_comp_priv_object_creation() - vdev
 * comp object creation
 * @vdev: VDEV object
 * @id: Component id
 *
 * API to create component private object in run time, this would
 * be used for features which gets enabled in run time
 *
 * Return: SUCCESS on successful creation
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS wlan_objmgr_trigger_vdev_comp_priv_object_creation(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id);

/**
 * wlan_objmgr_trigger_vdev_comp_priv_object_deletion() - vdev
 * comp object deletion
 * @vdev: VDEV object
 * @id: Component id
 *
 * API to destroy component private object in run time, this would
 * be used for features which gets disabled in run time
 *
 * Return: SUCCESS on successful deletion
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS wlan_objmgr_trigger_vdev_comp_priv_object_deletion(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id);

/**
 * wlan_objmgr_vdev_get_comp_private_obj() - get vdev component private object
 * @vdev: VDEV object
 * @id: Component id
 *
 * API to get component private object
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void *ptr on SUCCESS
 *         NULL on Failure
 */
void *wlan_objmgr_vdev_get_comp_private_obj(
		struct wlan_objmgr_vdev *vdev,
		enum wlan_umac_comp_id id);

/* Util APIs */

/**
 * wlan_vdev_get_pdev() - get pdev
 * @vdev: VDEV object
 *
 * API to get pdev object pointer from vdev
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: pdev object pointer
 */
static inline struct wlan_objmgr_pdev *wlan_vdev_get_pdev(
				struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_objmgr.wlan_pdev;
}

/**
 * wlan_pdev_vdev_list_peek_head() - get first vdev from pdev list
 * @peer_list: qdf_list_t
 *
 * API to get the head vdev of given peer (of pdev's vdev list)
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @peer: head peer
 */
static inline struct wlan_objmgr_vdev *wlan_pdev_vdev_list_peek_head(
					qdf_list_t *vdev_list)
{
	struct wlan_objmgr_vdev *vdev;
	qdf_list_node_t *vdev_node = NULL;

	/* This API is invoked with lock acquired, do not add log prints */
	if (qdf_list_peek_front(vdev_list, &vdev_node) != QDF_STATUS_SUCCESS)
		return NULL;

	vdev = qdf_container_of(vdev_node, struct wlan_objmgr_vdev, vdev_node);
	return vdev;
}

/**
 * wlan_vdev_get_next_vdev_of_pdev() - get next vdev
 * @vdev: VDEV object
 *
 * API to get next vdev object pointer of vdev
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @vdev_next: VDEV object
 */
static inline struct wlan_objmgr_vdev *wlan_vdev_get_next_vdev_of_pdev(
					qdf_list_t *vdev_list,
					struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_vdev *vdev_next;
	qdf_list_node_t *node = &vdev->vdev_node;
	qdf_list_node_t *next_node = NULL;

	/* This API is invoked with lock acquired, do not add log prints */
	if (node == NULL)
		return NULL;

	if (qdf_list_peek_next(vdev_list, node, &next_node) !=
						QDF_STATUS_SUCCESS)
		return NULL;

	vdev_next = qdf_container_of(next_node, struct wlan_objmgr_vdev,
				vdev_node);
	return vdev_next;
}

/**
 * wlan_vdev_set_pdev() - set pdev
 * @vdev: VDEV object
 * @pdev: PDEV object
 *
 * API to get pdev object pointer from vdev
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_set_pdev(struct wlan_objmgr_vdev *vdev,
					struct wlan_objmgr_pdev *pdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_objmgr.wlan_pdev = pdev;
}

/**
 * wlan_vdev_get_psoc() - get psoc
 * @vdev: VDEV object
 *
 * API to get pdev object pointer from vdev
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: psoc object pointer
 */
static inline struct wlan_objmgr_psoc *wlan_vdev_get_psoc(
				struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_objmgr_psoc *psoc = NULL;

	/* This API is invoked with lock acquired, do not add log prints */
	pdev = wlan_vdev_get_pdev(vdev);
	if (pdev == NULL)
		return NULL;

	psoc = wlan_pdev_get_psoc(pdev);

	return psoc;
}

/**
 * wlan_vdev_mlme_set_opmode() - set vdev opmode
 * @vdev: VDEV object
 * @mode: VDEV op mode
 *
 * API to set opmode in vdev object
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_set_opmode(struct wlan_objmgr_vdev *vdev,
				enum tQDF_ADAPTER_MODE mode)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.vdev_opmode = mode;
}

/**
 * wlan_vdev_mlme_get_opmode() - get vdev opmode
 * @vdev: VDEV object
 *
 * API to set opmode of vdev object
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @mode: VDEV op mode
 */
static inline enum tQDF_ADAPTER_MODE wlan_vdev_mlme_get_opmode(
					struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_mlme.vdev_opmode;
}

/**
 * wlan_vdev_mlme_set_macaddr() - set vdev macaddr
 * @vdev: VDEV object
 * @macaddr: MAC address
 *
 * API to set macaddr in vdev object
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_set_macaddr(struct wlan_objmgr_vdev *vdev,
					 uint8_t *macaddr)
{
	/* This API is invoked with lock acquired, do not add log prints */
	WLAN_ADDR_COPY(vdev->vdev_mlme.macaddr, macaddr);
}

/**
 * wlan_vdev_mlme_get_macaddr() - get vdev macaddr
 * @vdev: VDEV object
 *
 * API to get MAC address from vdev object
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @macaddr: MAC address
 */
static inline uint8_t *wlan_vdev_mlme_get_macaddr(struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_mlme.macaddr;
}

/**
 * wlan_vdev_mlme_set_mataddr() - set vdev mataddr
 * @vdev: VDEV object
 * @mataddr: MAT address
 *
 * API to set mataddr in vdev object
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_set_mataddr(struct wlan_objmgr_vdev *vdev,
					uint8_t *mataddr)
{
	/* This API is invoked with lock acquired, do not add log prints */
	WLAN_ADDR_COPY(vdev->vdev_mlme.mataddr, mataddr);
}

/**
 * wlan_vdev_mlme_get_mataddr() - get mataddr
 * @vdev: VDEV object
 *
 * API to get MAT address from vdev object
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @mataddr: MAT address
 */
static inline uint8_t *wlan_vdev_mlme_get_mataddr(struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_mlme.mataddr;
}

/**
 * wlan_vdev_get_id() - get vdev id
 * @vdev: VDEV object
 *
 * API to get vdev id
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @id: vdev id
 */
static inline uint8_t wlan_vdev_get_id(struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_objmgr.vdev_id;
}

/**
 * wlan_vdev_get_hw_macaddr() - get hw macaddr
 * @vdev: VDEV object
 *
 * API to retrieve the HW MAC address from PDEV
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @macaddr: HW MAC address
 */
static inline uint8_t *wlan_vdev_get_hw_macaddr(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);

	/* This API is invoked with lock acquired, do not add log prints */
	if (pdev != NULL)
		return wlan_pdev_get_hw_macaddr(pdev);
	else
		return NULL;
}

/**
 * wlan_vdev_mlme_set_ssid() - set ssid
 * @vdev: VDEV object
 * @ssid: SSID (input)
 * @ssid_len: Length of SSID
 *
 * API to set the SSID of VDEV
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: SUCCESS, if update is done
 *          FAILURE, if ssid length is > max ssid len
 */
static inline QDF_STATUS wlan_vdev_mlme_set_ssid(
				struct wlan_objmgr_vdev *vdev,
				const uint8_t *ssid, uint8_t ssid_len)
{
	/* This API is invoked with lock acquired, do not add log prints */
	if (ssid_len <= WLAN_SSID_MAX_LEN) {
		qdf_mem_copy(vdev->vdev_mlme.ssid, ssid, ssid_len);
		vdev->vdev_mlme.ssid_len = ssid_len;
	} else {
		vdev->vdev_mlme.ssid_len = 0;
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_vdev_mlme_get_ssid() - get ssid
 * @vdev: VDEV object
 * @ssid: SSID
 * @ssid_len: Length of SSID
 *
 * API to get the SSID of VDEV, it updates the SSID and its length
 * in @ssid, @ssid_len respectively
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: SUCCESS, if update is done
 *          FAILURE, if ssid length is > max ssid len
 */
static inline QDF_STATUS wlan_vdev_mlme_get_ssid(
				struct wlan_objmgr_vdev *vdev,
				 uint8_t *ssid, uint8_t *ssid_len)
{
	/* This API is invoked with lock acquired, do not add log prints */
	if (vdev->vdev_mlme.ssid_len > 0) {
		*ssid_len = vdev->vdev_mlme.ssid_len;
		qdf_mem_copy(ssid, vdev->vdev_mlme.ssid, *ssid_len);
	} else {
		*ssid_len = 0;
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_vdev_obj_lock() - Acquire VDEV spinlock
 * @vdev: VDEV object
 *
 * API to acquire VDEV lock
 * Parent lock should not be taken in child lock context
 * but child lock can be taken in parent lock context
 * (for ex: psoc lock can't be invoked in pdev/vdev/peer lock context)
 *
 * Return: void
 */
static inline void wlan_vdev_obj_lock(struct wlan_objmgr_vdev *vdev)
{
	qdf_spin_lock_bh(&vdev->vdev_lock);
}

/**
 * wlan_vdev_obj_unlock() - Release VDEV spinlock
 * @vdev: VDEV object
 *
 * API to Release VDEV lock
 *
 * Return: void
 */
static inline void wlan_vdev_obj_unlock(struct wlan_objmgr_vdev *vdev)
{
	qdf_spin_unlock_bh(&vdev->vdev_lock);
}

/**
 * wlan_vdev_mlme_set_bss_chan() - set bss chan
 * @vdev: VDEV object
 * @bss_chan: Channel
 *
 * API to set the BSS channel
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_set_bss_chan(struct wlan_objmgr_vdev *vdev,
			struct wlan_channel *bss_chan)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.bss_chan = bss_chan;
}

/**
 * wlan_vdev_mlme_get_bss_chan() - get bss chan
 * @vdev: VDEV object
 *
 * API to get the BSS channel
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @bss_chan: Channel
 */
static inline struct wlan_channel *wlan_vdev_mlme_get_bss_chan(
				struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_mlme.bss_chan;
}

/**
 * wlan_vdev_mlme_set_des_chan() - set desired chan
 * @vdev: VDEV object
 * @des_chan: Channel configured by user
 *
 * API to set the desired channel
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_set_des_chan(struct wlan_objmgr_vdev *vdev,
			struct wlan_channel *des_chan)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.des_chan = des_chan;
}

/**
 * wlan_vdev_mlme_get_des_chan() - get desired chan
 * @vdev: VDEV object
 *
 * API to get the desired channel
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @des_chan: Channel configured by user
 */
static inline struct wlan_channel *wlan_vdev_mlme_get_des_chan(
				struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_mlme.des_chan;
}

/**
 * wlan_vdev_mlme_set_nss() - set NSS
 * @vdev: VDEV object
 * @nss: nss configured by user
 *
 * API to set the Number of Spatial streams
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_set_nss(struct wlan_objmgr_vdev *vdev,
			uint8_t nss)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.nss = nss;
}

/**
 * wlan_vdev_mlme_get_nss() - get NSS
 * @vdev: VDEV object
 *
 * API to get the Number of Spatial Streams
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @nss: nss value
 */
static inline uint8_t wlan_vdev_mlme_get_nss(
				struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_mlme.nss;
}

/**
 * wlan_vdev_mlme_set_txchainmask() - set Tx chainmask
 * @vdev: VDEV object
 * @chainmask : chainmask either configured by user or max supported
 *
 * API to set the Tx chainmask
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_set_txchainmask(struct wlan_objmgr_vdev *vdev,
			uint8_t chainmask)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.tx_chainmask = chainmask;
}

/**
 * wlan_vdev_mlme_get_txchainmask() - get Tx chainmask
 * @vdev: VDEV object
 *
 * API to get the Tx chainmask
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @chainmask : Tx chainmask either configured by user or max supported
 */
static inline uint8_t wlan_vdev_mlme_get_txchainmask(
				struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_mlme.tx_chainmask;
}

/**
 * wlan_vdev_mlme_set_rxchainmask() - set Rx chainmask
 * @vdev: VDEV object
 * @chainmask : Rx chainmask either configured by user or max supported
 *
 * API to set the Rx chainmask
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_set_rxchainmask(struct wlan_objmgr_vdev *vdev,
			uint8_t chainmask)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.rx_chainmask = chainmask;
}

/**
 * wlan_vdev_mlme_get_rxchainmask() - get Rx chainmask
 * @vdev: VDEV object
 *
 * API to get the Rx chainmask
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @chainmask : Rx chainmask either configured by user or max supported
 */
static inline uint8_t wlan_vdev_mlme_get_rxchainmask(
				struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_mlme.rx_chainmask;
}

/**
 * wlan_vdev_mlme_set_txpower() - set tx power
 * @vdev: VDEV object
 * @txpow: tx power either configured by used or max allowed
 *
 * API to set the tx power
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_set_txpower(struct wlan_objmgr_vdev *vdev,
			uint8_t txpow)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.tx_power = txpow;
}

/**
 * wlan_vdev_mlme_get_txpower() - get tx power
 * @vdev: VDEV object
 *
 * API to get the tx power
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @txpow: tx power either configured by used or max allowed
 */
static inline uint8_t wlan_vdev_mlme_get_txpower(
				struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_mlme.tx_power;
}

/**
 * wlan_vdev_mlme_set_maxrate() - set max rate
 * @vdev: VDEV object
 * @maxrate: configured by used or based on configured mode
 *
 * API to set the max rate the vdev supports
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_set_maxrate(struct wlan_objmgr_vdev *vdev,
			uint32_t maxrate)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.max_rate = maxrate;
}

/**
 * wlan_vdev_mlme_get_maxrate() - get max rate
 * @vdev: VDEV object
 *
 * API to get the max rate the vdev supports
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @maxrate: configured by used or based on configured mode
 */
static inline uint32_t wlan_vdev_mlme_get_maxrate(
				struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_mlme.max_rate;
}

/**
 * wlan_vdev_mlme_set_txmgmtrate() - set txmgmtrate
 * @vdev: VDEV object
 * @txmgmtrate: Tx Mgmt rate
 *
 * API to set Mgmt Tx rate
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_set_txmgmtrate(struct wlan_objmgr_vdev *vdev,
			uint32_t txmgmtrate)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.tx_mgmt_rate = txmgmtrate;
}

/**
 * wlan_vdev_mlme_get_txmgmtrate() - get txmgmtrate
 * @vdev: VDEV object
 *
 * API to get Mgmt Tx rate
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @txmgmtrate: Tx Mgmt rate
 */
static inline uint32_t wlan_vdev_mlme_get_txmgmtrate(
				struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_mlme.tx_mgmt_rate;
}

/**
 * wlan_vdev_mlme_feat_cap_set() - set feature caps
 * @vdev: VDEV object
 * @cap: capabilities to be set
 *
 * API to set MLME feature capabilities
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_feat_cap_set(struct wlan_objmgr_vdev *vdev,
				uint32_t cap)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.vdev_feat_caps |= cap;
}

/**
 * wlan_vdev_mlme_feat_cap_clear() - clear feature caps
 * @vdev: VDEV object
 * @cap: capabilities to be cleared
 *
 * API to clear MLME feature capabilities
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_feat_cap_clear(struct wlan_objmgr_vdev *vdev,
				uint32_t cap)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.vdev_feat_caps &= ~cap;
}

/**
 * wlan_vdev_mlme_feat_cap_get() - get feature caps
 * @vdev: VDEV object
 * @cap: capabilities to be checked
 *
 * API to know MLME feature capability is set or not
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: 1 -- if capabilities set
 *         0 -- if capabilities clear
 */
static inline uint8_t wlan_vdev_mlme_feat_cap_get(struct wlan_objmgr_vdev *vdev,
				uint32_t cap)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return (vdev->vdev_mlme.vdev_feat_caps & cap) ? 1 : 0;
}

/**
 * wlan_vdev_mlme_feat_ext_cap_set() - set ext feature caps
 * @vdev: VDEV object
 * @cap: capabilities to be set
 *
 * API to set the MLME extensive feature capabilities
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_feat_ext_cap_set(
				struct wlan_objmgr_vdev *vdev,
				uint32_t cap)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.vdev_feat_ext_caps |= cap;
}

/**
 * wlan_vdev_mlme_feat_ext_cap_clear() - clear ext feature caps
 * @vdev: VDEV object
 * @cap: capabilities to be cleared
 *
 * API to clear the MLME extensive feature capabilities
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_feat_ext_cap_clear(
				struct wlan_objmgr_vdev *vdev,
				uint32_t cap)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.vdev_feat_ext_caps &= ~cap;
}

/**
 * wlan_vdev_mlme_feat_ext_cap_get() - get feature ext caps
 * @vdev: VDEV object
 * @cap: capabilities to be checked
 *
 * API to know MLME ext feature capability is set or not
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: 1 -- if capabilities set
 *         0 -- if capabilities clear
 */
static inline uint8_t wlan_vdev_mlme_feat_ext_cap_get(
				struct wlan_objmgr_vdev *vdev,
				uint32_t cap)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return (vdev->vdev_mlme.vdev_feat_ext_caps & cap) ? 1 : 0;
}

/**
 * wlan_vdev_mlme_cap_set() - mlme caps set
 * @vdev: VDEV object
 * @cap: capabilities to be set
 *
 * API to set the MLME capabilities
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_cap_set(struct wlan_objmgr_vdev *vdev,
				uint32_t cap)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.vdev_caps |= cap;
}

/**
 * wlan_vdev_mlme_cap_clear() -  mlme caps clear
 * @vdev: VDEV object
 * @cap: capabilities to be cleared
 *
 * API to clear the MLME capabilities
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_cap_clear(struct wlan_objmgr_vdev *vdev,
				uint32_t cap)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_mlme.vdev_caps &= ~cap;
}

/**
 * wlan_vdev_mlme_cap_get() - get mlme caps
 * @vdev: VDEV object
 * @cap: capabilities to be checked
 *
 * API to know MLME capability is set or not
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: 1 -- if capabilities set
 *         0 -- if capabilities clear
 */
static inline uint8_t wlan_vdev_mlme_cap_get(struct wlan_objmgr_vdev *vdev,
				uint32_t cap)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return (vdev->vdev_mlme.vdev_caps & cap) ? 1 : 0;
}

/**
 * wlan_vdev_mlme_get_state() - get mlme state
 * @vdev: VDEV object
 *
 * API to get MLME state
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: state of MLME
 */
static inline enum wlan_vdev_state wlan_vdev_mlme_get_state(
				struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_mlme.mlme_state;
}

/**
 * wlan_vdev_mlme_set_state() - set mlme state
 * @vdev: VDEV object
 * @state: MLME state
 *
 * API to set MLME state
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_mlme_set_state(struct wlan_objmgr_vdev *vdev,
					enum wlan_vdev_state state)
{
	/* This API is invoked with lock acquired, do not add log prints */
	if (state < WLAN_VDEV_S_MAX)
		vdev->vdev_mlme.mlme_state = state;
}

/**
 * wlan_vdev_set_selfpeer() - set self peer
 * @vdev: VDEV object
 * @peer: peer pointer
 *
 * API to set the self peer of VDEV
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_set_selfpeer(struct wlan_objmgr_vdev *vdev,
						struct wlan_objmgr_peer *peer)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_objmgr.self_peer = peer;
}

/**
 * wlan_vdev_get_selfpeer() - get self peer
 * @vdev: VDEV object
 *
 * API to get the self peer of VDEV
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @peer: peer pointer
 */
static inline struct wlan_objmgr_peer *wlan_vdev_get_selfpeer(
					struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_objmgr.self_peer;
}

/**
 * wlan_vdev_set_bsspeer() - set bss peer
 * @vdev: VDEV object
 * @peer: BSS peer pointer
 *
 * API to set the BSS peer of VDEV
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_set_bsspeer(struct wlan_objmgr_vdev *vdev,
						 struct wlan_objmgr_peer *peer)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_objmgr.bss_peer = peer;
}

/**
 * wlan_vdev_get_bsspeer() - get bss peer
 * @vdev: VDEV object
 *
 * API to get the BSS peer of VDEV
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return:
 * @peer: BSS peer pointer
 */
static inline struct wlan_objmgr_peer *wlan_vdev_get_bsspeer(
					struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_objmgr.bss_peer;
}

/**
 * wlan_vdev_get_ospriv() - get os priv pointer
 * @vdev: VDEV object
 *
 * API to get OS private pointer from VDEV
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: ospriv - private pointer
 */
static inline struct vdev_osif_priv *wlan_vdev_get_ospriv(
	struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_nif.osdev;
}

/**
 * wlan_vdev_reset_ospriv() - reset os priv pointer
 * @vdev: VDEV object
 *
 * API to reset OS private pointer in VDEV
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_reset_ospriv(struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_nif.osdev = NULL;
}

/**
 * wlan_vdev_get_peer_count() - get vdev peer count
 * @vdev: VDEV object
 *
 * API to get OS private pointer from VDEV
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: peer_count - vdev's peer count
 */
static inline uint16_t wlan_vdev_get_peer_count(struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_objmgr.wlan_peer_count;
}

/**
 * DOC: Examples to use VDEV ref count APIs
 *
 * In all the scenarios, the pair of API should be followed
 * other it lead to memory leak
 *
 *  scenario 1:
 *
 *     wlan_objmgr_vdev_obj_create()
 *     ----
 *     wlan_objmgr_vdev_obj_delete()
 *
 *  scenario 2:
 *
 *     wlan_objmgr_vdev_get_ref()
 *     ----
 *     the operations which are done on
 *     vdev object
 *     ----
 *     wlan_objmgr_vdev_release_ref()
 *
 *  scenario 3:
 *
 *     API to retrieve vdev (xxx_get_vdev_xxx())
 *     ----
 *     the operations which are done on
 *     vdev object
 *     ----
 *     wlan_objmgr_vdev_release_ref()
 */

/**
 * wlan_objmgr_vdev_get_ref() - increment ref count
 * @vdev: VDEV object
 * @id:   Object Manager ref debug id
 *
 * API to increment ref count of vdev
 *
 * Return: void
 */
void wlan_objmgr_vdev_get_ref(struct wlan_objmgr_vdev *vdev,
				wlan_objmgr_ref_dbgid id);

/**
 * wlan_objmgr_vdev_try_get_ref() - increment ref count, if allowed
 * @vdev: VDEV object
 * @id:   Object Manager ref debug id
 *
 * API to increment ref count of vdev after checking valid object state
 *
 * Return: void
 */
QDF_STATUS wlan_objmgr_vdev_try_get_ref(struct wlan_objmgr_vdev *vdev,
						wlan_objmgr_ref_dbgid id);

/**
 * wlan_objmgr_vdev_release_ref() - decrement ref count
 * @vdev: VDEV object
 * @id:   Object Manager ref debug id
 *
 * API to decrement ref count of vdev, if ref count is 1, it initiates the
 * VDEV deletion
 *
 * Return: void
 */
void wlan_objmgr_vdev_release_ref(struct wlan_objmgr_vdev *vdev,
						wlan_objmgr_ref_dbgid id);

/**
 * wlan_vdev_set_max_peer_count() - set max peer count
 * @vdev: VDEV object
 * @count: Max peer count
 *
 * API to set max peer count of VDEV
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: void
 */
static inline void wlan_vdev_set_max_peer_count(struct wlan_objmgr_vdev *vdev,
						uint16_t count)
{
	/* This API is invoked with lock acquired, do not add log prints */
	vdev->vdev_objmgr.max_peer_count = count;
}

/**
 * wlan_vdev_get_max_peer_count() - get max peer count
 * @vdev: VDEV object
 *
 * API to get max peer count of VDEV
 *
 * Caller need to acquire lock with wlan_vdev_obj_lock()
 *
 * Return: max peer count
 */
static inline uint16_t wlan_vdev_get_max_peer_count(
						struct wlan_objmgr_vdev *vdev)
{
	/* This API is invoked with lock acquired, do not add log prints */
	return vdev->vdev_objmgr.max_peer_count;
}
#endif /* _WLAN_OBJMGR_VDEV_OBJ_H_*/
