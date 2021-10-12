/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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
  * DOC: Define the pSoc data structure of UMAC
  *      Public APIs to perform operations on Global objects
  */
#ifndef _WLAN_OBJMGR_PSOC_OBJ_H_
#define _WLAN_OBJMGR_PSOC_OBJ_H_

#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_debug.h"

#define REG_DMN_CH144        0x0001
#define REG_DMN_ENTREPRISE   0x0002


/* fw_caps */
	/* CAPABILITY: WEP available */
#define WLAN_SOC_C_WEP                  0x00000001
	/* CAPABILITY: TKIP available */
#define WLAN_SOC_C_TKIP                 0x00000002
	/* CAPABILITY: AES OCB avail */
#define WLAN_SOC_C_AES                  0x00000004
	/* CAPABILITY: AES CCM avail */
#define WLAN_SOC_C_AES_CCM              0x00000008
	/* CAPABILITY: 11n HT available */
#define WLAN_SOC_C_HT                   0x00000010
	/* CAPABILITY: CKIP available */
#define WLAN_SOC_C_CKIP                 0x00000020
	/* CAPABILITY: ATH FF avail */
#define WLAN_SOC_C_FF                   0x00000040
	/* CAPABILITY: ATH Turbo avail*/
#define WLAN_SOC_C_TURBOP               0x00000080
	/* CAPABILITY: IBSS available */
#define WLAN_SOC_C_IBSS                 0x00000100
	/* CAPABILITY: Power mgmt */
#define WLAN_SOC_C_PMGT                 0x00000200
	/* CAPABILITY: HOSTAP avail */
#define WLAN_SOC_C_HOSTAP               0x00000400
	/* CAPABILITY: Old Adhoc Demo */
#define WLAN_SOC_C_AHDEMO               0x00000800
	/* CAPABILITY: tx power mgmt */
#define WLAN_SOC_C_TXPMGT               0x00001000
	/* CAPABILITY: short slottime */
#define WLAN_SOC_C_SHSLOT               0x00002000
	/* CAPABILITY: short preamble */
#define WLAN_SOC_C_SHPREAMBLE           0x00004000
	/* CAPABILITY: monitor mode */
#define WLAN_SOC_C_MONITOR              0x00008000
	/* CAPABILITY: TKIP MIC avail */
#define WLAN_SOC_C_TKIPMIC              0x00010000
	/* CAPABILITY: ATH WAPI avail */
#define WLAN_SOC_C_WAPI                 0x00020000
	/* CONF: WDS auto Detect/DELBA */
#define WLAN_SOC_C_WDS_AUTODETECT       0x00040000
	/* CAPABILITY: WPA1 avail */
#define WLAN_SOC_C_WPA1                 0x00080000
	/* CAPABILITY: WPA2 avail */
#define WLAN_SOC_C_WPA2                 0x00100000
	/* CAPABILITY: WPA1+WPA2 avail*/
#define WLAN_SOC_C_WPA                  0x00180000
	/* CAPABILITY: frame bursting */
#define WLAN_SOC_C_BURST                0x00200000
	/* CAPABILITY: WME avail */
#define WLAN_SOC_C_WME                  0x00400000
	/* CAPABILITY: 4-addr support */
#define WLAN_SOC_C_WDS                  0x00800000
	/* CAPABILITY: TKIP MIC for QoS frame */
#define WLAN_SOC_C_WME_TKIPMIC          0x01000000
	/* CAPABILITY: bg scanning */
#define WLAN_SOC_C_BGSCAN               0x02000000
	/* CAPABILITY: UAPSD */
#define WLAN_SOC_C_UAPSD                0x04000000
	/* CAPABILITY: enabled 11.h */
#define WLAN_SOC_C_DOTH                 0x08000000

/* XXX protection/barker? */
	/* CAPABILITY: crypto alg's */
#define WLAN_SOC_C_CRYPTO         0x0000002f

/* fw_caps_ext */
	/* CAPABILITY: fast channel change */
#define WLAN_SOC_CEXT_FASTCC           0x00000001
	/* CAPABILITY: P2P */
#define WLAN_SOC_CEXT_P2P              0x00000002
	/* CAPABILITY: Multi-Channel Operations */
#define WLAN_SOC_CEXT_MULTICHAN        0x00000004
	/* CAPABILITY: the device supports perf and power offload */
#define WLAN_SOC_CEXT_PERF_PWR_OFLD    0x00000008
	/* CAPABILITY: the device supports 11ac */
#define WLAN_SOC_CEXT_11AC             0x00000010
	/* CAPABILITY: the device support acs channel hopping */
#define WLAN_SOC_CEXT_ACS_CHAN_HOP     0x00000020
	/* CAPABILITY: the device support STA DFS */
#define WLAN_SOC_CEXT_STADFS           0x00000040
	/* NSS offload capability */
#define WLAN_SOC_CEXT_NSS_OFFLOAD      0x00000080
	/* SW cal support capability */
#define WLAN_SOC_CEXT_SW_CAL           0x00000100
	/* Hybrid mode */
#define WLAN_SOC_CEXT_HYBRID_MODE      0x00000200
	/* TT support */
#define WLAN_SOC_CEXT_TT_SUPPORT       0x00000400
	/* WMI MGMT REF */
#define WLAN_SOC_CEXT_WMI_MGMT_REF     0x00000800
	/* Wideband scan */
#define WLAN_SOC_CEXT_WIDEBAND_SCAN    0x00001000
	/* TWT Requester capable */
#define WLAN_SOC_CEXT_TWT_REQUESTER    0x00002000
	/* TWT Responder capable */
#define WLAN_SOC_CEXT_TWT_RESPONDER    0x00004000
	/* HW DB2DBM CAPABLE */
#define WLAN_SOC_CEXT_HW_DB2DBM        0x00008000
	/* OBSS Narrow Bandwidth RU Tolerance */
#define WLAN_SOC_CEXT_OBSS_NBW_RU      0x00010000
	/* MBSS IE support */
#define WLAN_SOC_CEXT_MBSS_IE          0x00020000
	/* RXOLE Flow Search Support */
#define WLAN_SOC_CEXT_RX_FSE_SUPPORT   0x00040000
	/* Dynamic HW Mode Switch Support */
#define WLAN_SOC_CEXT_DYNAMIC_HW_MODE  0x00080000
	/* Restricted 80+80 MHz support */
#define WLAN_SOC_RESTRICTED_80P80_SUPPORT 0x00100000
	/* Indicates Firmware supports sending NSS ratio info to host */
#define WLAN_SOC_NSS_RATIO_TO_HOST_SUPPORT 0x00200000
	/* EMA AP Support */
#define WLAN_SOC_CEXT_EMA_AP           0x00400000
	/* MBSS PARAM IN START REQ Support */
#define WLAN_SOC_CEXT_MBSS_PARAM_IN_START   0x00800000
/* Per channel scan config flags support */
#define WLAN_SOC_CEXT_SCAN_PER_CH_CONFIG    0x01000000
	/* CAPABILITY: csa offload in case of AP */
#define WLAN_SOC_CEXT_CSA_TX_OFFLOAD      0x02000000
	/* AP initiator mode supported in staggered beacon mode */
#define WLAN_SOC_RTT_AP_INITIATOR_STAGGERED_MODE_SUPPORTED 0x04000000
	/* AP initiator mode supported in burst beacon mode */
#define WLAN_SOC_RTT_AP_INITIATOR_BURSTED_MODE_SUPPORTED 0x08000000
	/* ext cc event supported by fw */
#define WLAN_SOC_EXT_EVENT_SUPPORTED      0x010000000
	/* check 29th bit for p2p + p2p conc support by fw */
#define WLAN_SOC_EXT_P2P_P2P_CONC_SUPPORT 0x20000000

/* feature_flags */
	/* CONF: ATH FF enabled */
#define WLAN_SOC_F_FF                   0x00000001
	/* CONF: ATH Turbo enabled*/
#define WLAN_SOC_F_TURBOP               0x00000002
	/* STATUS: promiscuous mode */
#define WLAN_SOC_F_PROMISC              0x00000004
	/* STATUS: all multicast mode */
#define WLAN_SOC_F_ALLMULTI             0x00000008
/* NB: this is intentionally setup to be IEEE80211_CAPINFO_PRIVACY */
	/* STATUS: start IBSS */
#define WLAN_SOC_F_SIBSS                0x00000010
/* NB: this is intentionally setup to be IEEE80211_CAPINFO_SHORT_SLOTTIME */
	/* CONF: Power mgmt enable */
#define WLAN_SOC_F_PMGTON               0x00000020
	/* CONF: IBSS creation enable */
#define WLAN_SOC_F_IBSSON               0x00000040
	/* force chanswitch */
#define WLAN_SOC_F_CHANSWITCH           0x00000080

/* ic_flags_ext and/or iv_flags_ext */
	/* CONF: enable country IE */
#define WLAN_SOC_F_COUNTRYIE           0x00000100
	/* STATE: enable full bgscan completion */
#define WLAN_SOC_F_BGSCAN              0x00000200
	/* CONF: enable U-APSD */
#define WLAN_SOC_F_UAPSD               0x00000400
	/* STATUS: sleeping */
#define WLAN_SOC_F_SLEEP               0x00000800
	/* Enable marking of dfs interfernce */
#define WLAN_SOC_F_MARKDFS             0x00001000
	/* enable or disable s/w ccmp encrypt decrypt support */
#define WLAN_SOC_F_CCMPSW_ENCDEC       0x00002000
	/* STATE: hibernating */
#define WLAN_SOC_F_HIBERNATION         0x00004000
	/* CONF: desired country has been set */
#define WLAN_SOC_F_DESCOUNTRY          0x00008000
	/* CONF: enable power capability or contraint IE */
#define WLAN_SOC_F_PWRCNSTRIE          0x00010000
	/* STATUS: 11D in used */
#define WLAN_SOC_F_DOT11D              0x00020000
	/* Beacon offload */
#define WLAN_SOC_F_BCN_OFFLOAD         0x00040000
	/* LTEU support */
#define WLAN_SOC_F_LTEU_SUPPORT        0x00100000
	/* BT coext support */
#define WLAN_SOC_F_BTCOEX_SUPPORT      0x00200000
	/* HOST 80211 enable*/
#define WLAN_SOC_F_HOST_80211_ENABLE   0x00400000
	/* Spectral disable from INI */
#define WLAN_SOC_F_SPECTRAL_INI_DISABLE    0x00800000
	/* FTM testmode enable */
#define WLAN_SOC_F_TESTMODE_ENABLE     0x01000000
	/* Dynamic HW mode swithch enable */
#define WLAN_SOC_F_DYNAMIC_HW_MODE     0x02000000
	/* Broadcast TWT support enable */
#define WLAN_SOC_F_BCAST_TWT           0x04000000
       /* WDS Extended support */
#define WLAN_SOC_F_WDS_EXTENDED        0x08000000
/* Peer create response */
#define WLAN_SOC_F_PEER_CREATE_RESP    0x10000000
/* Strict channel mode */
#define WLAN_SOC_F_STRICT_CHANNEL      0x20000000
/* MGMT Rx REO feature capability */
#define WLAN_SOC_F_MGMT_RX_REO_CAPABLE  0x40000000

/* PSOC op flags */

	/* Invalid VHT cap */
#define WLAN_SOC_OP_VHT_INVALID_CAP    0x00000001

/* enum wlan_nss_ratio - NSS ratio received from FW during service ready ext
 *                       event.
 * WLAN_NSS_RATIO_1BY2_NSS : Max nss of 160MHz is equals to half of the max nss
 *                           of 80MHz
 * WLAN_NSS_RATIO_3BY4_NSS : Max nss of 160MHz is equals to 3/4 of the max nss
 *                           of 80MHz
 * WLAN_NSS_RATIO_1_NSS    : Max nss of 160MHz is equals to the max nss of 80MHz
 * WLAN_NSS_RATIO_2_NSS    : Max nss of 160MHz is equals to two times the max
 *                           nss of 80MHz
 * Values of this enum should be in sync with WMI_NSS_RATIO_INFO value provided
 * in wmi_unified.h.
 */
enum wlan_nss_ratio {
	WLAN_NSS_RATIO_1BY2_NSS = 0x0,
	WLAN_NSS_RATIO_3BY4_NSS = 0x1,
	WLAN_NSS_RATIO_1_NSS = 0x2,
	WLAN_NSS_RATIO_2_NSS = 0x3,
};

/**
 * struct wlan_objmgr_psoc_regulatory -  Regulatory sub structure of PSOC
 * @country_code:  Country code
 * @reg_dmn:       Regulatory Domain
 * @reg_flags:     Regulatory flags
 */
struct wlan_objmgr_psoc_regulatory {
	uint16_t country_code;
	uint16_t reg_dmn;
	uint16_t reg_flags;
};

/**
 * struct wlan_objmgr_psoc_user_config -  user configurations to
 * be used by common modules
 * @is_11d_support_enabled: Enable/disable 11d feature
 * @is_11h_support_enabled: Enable/disable 11h feature
 * @dot11_mode: Phy mode
 * @skip_dfs_chnl_in_p2p_search: Skip Dfs Channel in case of P2P
 *                             Search
 * @band_capability: Preferred band (0:Both,  1:2G only,  2:5G only)
 */
struct wlan_objmgr_psoc_user_config {
	bool is_11d_support_enabled;
	bool is_11h_support_enabled;
	uint8_t dot11_mode;
	uint8_t band_capability;
};

/**
 * struct wlan_objmgr_psoc_nif - HDD/OSIF specific sub structure of PSOC
 * @phy_version:     phy version, read in device probe
 * @phy_type:        OL/DA type
 * @soc_fw_caps:     FW capabilities
 * @soc_fw_ext_caps: FW ext capabilities
 * @soc_feature_caps:Feature capabilities
 * @soc_op_flags:    Flags to set/reset during operation
 * @soc_hw_macaddr[]:HW MAC address
 * @user_config:     user config from OS layer
 */
struct wlan_objmgr_psoc_nif {
	uint32_t phy_version;
	WLAN_DEV_TYPE phy_type;
	uint32_t soc_fw_caps;
	uint32_t soc_fw_ext_caps;
	uint32_t soc_feature_caps;
	uint32_t soc_op_flags;
	uint8_t soc_hw_macaddr[QDF_MAC_ADDR_SIZE];
	struct wlan_objmgr_psoc_user_config user_config;
};

/**
 * struct wlan_objmgr_psoc_objmgr - psoc object manager sub structure
 * @psoc_id:              The PSOC's numeric Id
 * @wlan_pdev_count:      PDEV count
 * @wlan_pdev_id_map:     PDEV id map, to allocate free ids
 * @wlan_vdev_count:      VDEV count
 * @max_vdev_count:       Max no. of VDEVs supported by this PSOC
 * @print_cnt:            Count to throttle Logical delete prints
 * @wlan_peer_count:      PEER count
 * @max_peer_count:       Max no. of peers supported by this PSOC
 * @temp_peer_count:      Temporary peer count
 * @wlan_pdev_list[]:     PDEV list
 * @wlan_vdev_list[]:     VDEV list
 * @wlan_vdev_id_map[]:   VDEV id map, to allocate free ids
 * @peer_list:            Peer list
 * @ref_cnt:              Ref count
 * @ref_id_dbg:           Array to track Ref count
 * @qdf_dev:              QDF Device
 */
struct wlan_objmgr_psoc_objmgr {
	uint8_t psoc_id;
	uint8_t wlan_pdev_count;
	uint8_t wlan_pdev_id_map;
	uint8_t wlan_vdev_count;
	uint8_t max_vdev_count;
	uint8_t print_cnt;
	uint16_t wlan_peer_count;
	uint16_t max_peer_count;
	uint16_t temp_peer_count;
	struct wlan_objmgr_pdev *wlan_pdev_list[WLAN_UMAC_MAX_PDEVS];
	struct wlan_objmgr_vdev *wlan_vdev_list[WLAN_UMAC_PSOC_MAX_VDEVS];
	qdf_bitmap(wlan_vdev_id_map, WLAN_UMAC_PSOC_MAX_VDEVS);
	struct wlan_peer_list peer_list;
	qdf_atomic_t ref_cnt;
	qdf_atomic_t ref_id_dbg[WLAN_REF_ID_MAX];
	qdf_device_t qdf_dev;
};

/**
 * struct wlan_soc_southbound_cb - Southbound callbacks
 * @tx_ops: contains southbound tx callbacks
 * @rx_ops: contains southbound rx callbacks
 */
struct wlan_soc_southbound_cb {
	struct wlan_lmac_if_tx_ops *tx_ops;
	struct wlan_lmac_if_rx_ops *rx_ops;
};

/**
 * struct wlan_concurrency_info - structure for concurrency info
 *
 */
struct wlan_concurrency_info {
};

/**
 * struct wlan_soc_timer - structure for soc timer
 *
 */
struct wlan_soc_timer {
};

/**
 * struct wlan_objmgr_psoc - PSOC common object
 * @soc_reg:               regulatory sub structure
 * @soc_nif:               nif sub strucutre
 * @soc_objmgr:            object manager sub structure
 * @soc_cb:                south bound callbacks
 * @soc_timer:             soc timer for inactivity
 * @soc_concurrency:       concurrency info
 * @soc_comp_priv_obj[]:   component private object pointers
 * @obj_status[]:          component object status
 * @obj_state:             object state
 * @tgt_if_handle:         target interface handle
 * @dp_handle:             DP module handle
 * @psoc_lock:             psoc lock
 */
struct wlan_objmgr_psoc {
	struct wlan_objmgr_psoc_regulatory soc_reg;
	struct wlan_objmgr_psoc_nif  soc_nif;
	struct wlan_objmgr_psoc_objmgr soc_objmgr;
	struct wlan_soc_southbound_cb soc_cb;
	struct wlan_soc_timer soc_timer;
	struct wlan_concurrency_info soc_concurrency; /*TODO */
	void *soc_comp_priv_obj[WLAN_UMAC_MAX_COMPONENTS];
	QDF_STATUS obj_status[WLAN_UMAC_MAX_COMPONENTS];
	WLAN_OBJ_STATE obj_state;
	struct target_psoc_info *tgt_if_handle;
	void *dp_handle;
	qdf_spinlock_t psoc_lock;
};

/**
 * struct wlan_psoc_host_hal_reg_capabilities_ext: Below are Reg caps per PHY.
 *                       Please note PHY ID starts with 0.
 * @phy_id: phy id starts with 0.
 * @eeprom_reg_domain: regdomain value specified in EEPROM
 * @eeprom_reg_domain_ext: regdomain
 * @regcap1: CAP1 capabilities bit map, see REGDMN_CAP1_ defines
 * @regcap2: REGDMN EEPROM CAP, see REGDMN_EEPROM_EEREGCAP_ defines
 * @wireless_modes: REGDMN MODE, see REGDMN_MODE_ enum
 * @low_2ghz_chan: 2G channel low
 * @high_2ghz_chan: 2G channel High
 * @low_5ghz_chan: 5G channel low
 * @high_5ghz_chan: 5G channel High
 */
struct wlan_psoc_host_hal_reg_capabilities_ext {
	uint32_t phy_id;
	uint32_t eeprom_reg_domain;
	uint32_t eeprom_reg_domain_ext;
	uint32_t regcap1;
	uint32_t regcap2;
	uint64_t wireless_modes;
	uint32_t low_2ghz_chan;
	uint32_t high_2ghz_chan;
	uint32_t low_5ghz_chan;
	uint32_t high_5ghz_chan;
};

/**
 * struct wlan_psoc_host_hal_reg_capabilities_ext2 - HAL reg capabilities
 * from service ready ext2 event.
 * @phy_id: phy id starts with 0
 * @wireless_modes_ext: REGDMN MODE, see REGDMN_MODE_ enum
 */
struct wlan_psoc_host_hal_reg_capabilities_ext2 {
	uint32_t phy_id;
	uint64_t wireless_modes_ext;
};

/**
 ** APIs to Create/Delete Global object APIs
 */
/**
 * wlan_objmgr_psoc_obj_create() - psoc object create
 * @phy_version: device id (from probe)
 * @dev_type: Offload/DA
 *
 * Creates PSOC object, intializes with default values
 * Invokes the registered notifiers to create component object
 *
 * Return: Handle to struct wlan_objmgr_psoc on successful creation,
 *         NULL on Failure (on Mem alloc failure and Component objects
 *         Failure)
 */
struct wlan_objmgr_psoc *wlan_objmgr_psoc_obj_create(uint32_t phy_version,
				WLAN_DEV_TYPE dev_type);

/**
 * wlan_objmgr_psoc_obj_delete() - psoc object delete
 * @psoc: PSOC object
 *
 * Logically deletes PSOC object,
 * Once all the references are released, object manager invokes the registered
 * notifiers to destroy component objects
 *
 * Return: SUCCESS/FAILURE
 */
QDF_STATUS wlan_objmgr_psoc_obj_delete(struct wlan_objmgr_psoc *psoc);

/**
 ** APIs to attach/detach component objects
 */

/**
 * wlan_objmgr_psoc_component_obj_attach() - psoc comp object attach
 * @psoc: PSOC object
 * @id: Component id
 * @comp_priv_obj: component's private object pointer
 * @status: Component's private object creation status
 *
 * API to be used for attaching component object with PSOC common object
 *
 * Return: SUCCESS on successful storing of component's object in common object
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS wlan_objmgr_psoc_component_obj_attach(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj,
		QDF_STATUS status);

/**
 * wlan_objmgr_psoc_component_obj_detach() - psoc comp object detach
 * @psoc: PSOC object
 * @id: Component id
 * @comp_priv_obj: component's private object pointer
 *
 * API to be used for detaching component object with PSOC common object
 *
 * Return: SUCCESS on successful removal of component's object from common
 *         object
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS wlan_objmgr_psoc_component_obj_detach(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id id,
		void *comp_priv_obj);

/**
 ** APIs to operations on psoc objects
 */
typedef void (*wlan_objmgr_op_handler)(struct wlan_objmgr_psoc *psoc,
					void *object,
					void *arg);

/**
 * wlan_objmgr_iterate_obj_list() - iterate through all psoc objects
 *                                  (CREATED state)
 * @psoc: PSOC object
 * @obj_type: PDEV_OP/VDEV_OP/PEER_OP
 * @handler: the handler will be called for each object of requested type
 *            the handler should be implemented to perform required operation
 * @arg:     agruments passed by caller
 * @lock_free_op: its obsolete
 * @dbg_id: id of the caller
 *
 * API to be used for performing the operations on all PDEV/VDEV/PEER objects
 * of psoc
 *
 * Return: SUCCESS/FAILURE
 */
QDF_STATUS wlan_objmgr_iterate_obj_list(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_objmgr_obj_type obj_type,
		wlan_objmgr_op_handler handler,
		void *arg, uint8_t lock_free_op,
		wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_iterate_obj_list_all() - iterate through all psoc objects
 * @psoc: PSOC object
 * @obj_type: PDEV_OP/VDEV_OP/PEER_OP
 * @handler: the handler will be called for each object of requested type
 *            the handler should be implemented to perform required operation
 * @arg:     agruments passed by caller
 * @lock_free_op: its obsolete
 * @dbg_id: id of the caller
 *
 * API to be used for performing the operations on all PDEV/VDEV/PEER objects
 * of psoc
 *
 * Return: SUCCESS/FAILURE
 */
QDF_STATUS wlan_objmgr_iterate_obj_list_all(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_objmgr_obj_type obj_type,
		wlan_objmgr_op_handler handler,
		void *arg, uint8_t lock_free_op,
		wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_free_all_objects_per_psoc() - free all psoc objects
 * @psoc: PSOC object
 *
 * API to be used free all the objects(pdev/vdev/peer) of psoc
 *
 * Return: SUCCESS/FAILURE
 */
QDF_STATUS wlan_objmgr_free_all_objects_per_psoc(
		struct wlan_objmgr_psoc *psoc);

/**
 * wlan_objmgr_trigger_psoc_comp_priv_object_creation() - create
 * psoc comp object
 * @psoc: PSOC object
 * @id: Component id
 *
 * API to create component private object in run time, this would
 * be used for features which gets enabled in run time
 *
 * Return: SUCCESS on successful creation
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS wlan_objmgr_trigger_psoc_comp_priv_object_creation(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id id);

/**
 * wlan_objmgr_trigger_psoc_comp_priv_object_deletion() - destroy
 * psoc comp object
 * @psoc: PSOC object
 * @id: Component id
 *
 * API to destroy component private object in run time, this would
 * be used for features which gets disabled in run time
 *
 * Return: SUCCESS on successful deletion
 *         On FAILURE (appropriate failure codes are returned)
 */
QDF_STATUS wlan_objmgr_trigger_psoc_comp_priv_object_deletion(
		struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id id);

/**
 * wlan_objmgr_get_peer_by_mac() - find peer from psoc's peer list
 * @psoc: PSOC object
 * @macaddr: MAC address
 * @dbg_id: id of the caller
 *
 * API to find peer object pointer by MAC addr
 *
 * This API increments the ref count of the peer object internally, the
 * caller has to invoke the wlan_objmgr_peer_release_ref() to decrement
 * ref count
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac_debug(
		struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id, const char *func, int line);

#define wlan_objmgr_get_peer_by_mac(psoc, macaddr, dbgid) \
		wlan_objmgr_get_peer_by_mac_debug(psoc, macaddr, dbgid, \
		__func__, __LINE__)
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac(
		struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_get_peer() - find peer from psoc's peer list
 * @psoc: PSOC object
 * @pdev_id: Pdev id
 * @macaddr: MAC address
 * @dbg_id: id of the caller
 *
 * API to find peer object pointer by MAC addr and pdev id
 *
 * This API increments the ref count of the peer object internally, the
 * caller has to invoke the wlan_objmgr_peer_release_ref() to decrement
 * ref count
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line);

#define wlan_objmgr_get_peer(psoc, pdev_id, macaddr, dbgid) \
		wlan_objmgr_get_peer_debug(psoc, pdev_id, macaddr, dbgid, \
		__func__, __LINE__)
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_get_peer_nolock() - find peer from psoc's peer list (lock free)
 * @psoc: PSOC object
 * @pdev_id: Pdev id
 * @macaddr: MAC address
 * @dbg_id: id of the caller
 *
 * API to find peer object pointer by MAC addr
 *
 * This API increments the ref count of the peer object internally, the
 * caller has to invoke the wlan_objmgr_peer_release_ref() to decrement
 * ref count
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_nolock_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line);

#define wlan_objmgr_get_peer_nolock(psoc, pdev_id, macaddr, dbgid) \
		wlan_objmgr_get_peer_nolock_debug(psoc, pdev_id, macaddr, \
		dbgid, __func__, __LINE__)
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer_nolock(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_get_peer_logically_deleted() - find peer
 * from psoc's peer list
 * @psoc: PSOC object
 * @macaddr: MAC address
 * @dbg_id: id of the caller
 *
 * API to find peer object pointer of logically deleted peer
 *
 * This API increments the ref count of the peer object internally, the
 * caller has to invoke the wlan_objmgr_peer_release_ref() to decrement
 * ref count
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_logically_deleted_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line);

#define wlan_objmgr_get_peer_logically_deleted(psoc, macaddr, dbgid) \
		wlan_objmgr_get_peer_logically_deleted_debug(psoc, macaddr, \
		dbgid, __func__, __LINE__)
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer_logically_deleted(
			struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_get_peer_no_state() - find peer from psoc's peer list
 * @psoc: PSOC object
 * @pdev_id: Pdev id
 * @macaddr: MAC address
 * @dbg_id: id of the caller
 *
 * API to find peer object pointer by MAC addr and pdev id,
 * ignores the state check
 *
 * This API increments the ref count of the peer object internally, the
 * caller has to invoke the wlan_objmgr_peer_release_ref() to decrement
 * ref count
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_no_state_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line);

#define wlan_objmgr_get_peer_no_state(psoc, pdev_id, macaddr, dbgid) \
		wlan_objmgr_get_peer_no_state_debug(psoc, pdev_id, macaddr, \
		dbgid, __func__, __LINE__)
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer_no_state(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_populate_logically_deleted_peerlist_by_mac_n_vdev() - get peer from
 *                                                psoc peer list using
 *                                                mac and vdev
 *                                                self mac
 * @psoc: PSOC object
 * @pdev_id: Pdev id
 * @bssid: BSSID address
 * @macaddr: MAC address
 * @dbg_id: id of the caller
 *
 * API to find peer object pointer by MAC addr, vdev self mac
 * address and pdev id for a node that is logically in deleted state
 *
 * This API increments the ref count of the peer object internally, the
 * caller has to invoke the wlan_objmgr_peer_release_ref() to decrement
 * ref count
 *
 * Return: List of peer pointers
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
qdf_list_t *wlan_objmgr_populate_logically_deleted_peerlist_by_mac_n_vdev_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *bssid, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line);

#define wlan_objmgr_populate_logically_deleted_peerlist_by_mac_n_vdev( \
	psoc, pdev_id, bssid, macaddr, dbgid) \
		wlan_objmgr_populate_logically_deleted_peerlist_by_mac_n_vdev_debug( \
		psoc, pdev_id, bssid, macaddr, dbgid, __func__, __LINE__)
#else
qdf_list_t *wlan_objmgr_populate_logically_deleted_peerlist_by_mac_n_vdev(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *bssid, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_get_peer_by_mac_n_vdev() - find peer from psoc's peer list
 *                                          using mac address and bssid
 * @psoc: PSOC object
 * @pdev_id: Pdev id
 * @bssid: MAC address of AP its associated
 * @macaddr: MAC address
 * @dbg_id: id of the caller
 *
 * API to find peer object pointer by MAC addr and vdev self mac address
 * and pdev id
 *
 * This API increments the ref count of the peer object internally, the
 * caller has to invoke the wlan_objmgr_peer_release_ref() to decrement
 * ref count
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac_n_vdev_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *bssid, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line);

#define wlan_objmgr_get_peer_by_mac_n_vdev(psoc, pdevid, bssid, macaddr, \
	dbgid) \
		wlan_objmgr_get_peer_by_mac_n_vdev_debug(psoc, pdevid, \
		bssid, macaddr, dbgid, __func__, __LINE__)
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac_n_vdev(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *bssid, uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_get_peer_by_mac_n_vdev_no_state() - find peer from psoc's peer
 *                                          list using mac address and bssid
 * @psoc: PSOC object
 * @pdev_id: Pdev id
 * @bssid: MAC address of AP its associated
 * @macaddr: MAC address
 * @dbg_id: id of the caller
 *
 * API to find peer object pointer by MAC addr, vdev self mac address,
 * and pdev id ,ignores the state
 *
 * This API increments the ref count of the peer object internally, the
 * caller has to invoke the wlan_objmgr_peer_release_ref() to decrement
 * ref count
 *
 * Return: peer pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac_n_vdev_no_state_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *bssid,  uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line);

#define wlan_objmgr_get_peer_by_mac_n_vdev_no_state(psoc, pdevid, bssid, \
	macaddr, dbgid) \
		wlan_objmgr_get_peer_by_mac_n_vdev_no_state_debug(psoc, \
		pdevid, bssid, macaddr, dbgid, __func__, __LINE__)
#else
struct wlan_objmgr_peer *wlan_objmgr_get_peer_by_mac_n_vdev_no_state(
			struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
			uint8_t *bssid,  uint8_t *macaddr,
			wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_get_pdev_by_id() - retrieve pdev by id
 * @psoc: PSOC object
 * @id: pdev id
 * @dbg_id: id of the caller
 *
 * API to find pdev object pointer by pdev id
 *
 * This API increments the ref count of the pdev object internally, the
 * caller has to invoke the wlan_objmgr_pdev_release_ref() to decrement
 * ref count
 *
 * Return: pdev pointer
 *         NULL on FAILURE
 */
struct wlan_objmgr_pdev *wlan_objmgr_get_pdev_by_id(
		struct wlan_objmgr_psoc *psoc, uint8_t id,
		wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_get_pdev_by_id_no_state() - retrieve pdev by id
 * @psoc: PSOC object
 * @id: pdev id
 * @dbg_id: id of the caller
 *
 * API to find pdev object pointer by pdev id, Ignores the state check
 *
 * This API increments the ref count of the pdev object internally, the
 * caller has to invoke the wlan_objmgr_pdev_release_ref() to decrement
 * ref count
 *
 * Return: pdev pointer
 *         NULL on FAILURE
 */
struct wlan_objmgr_pdev *wlan_objmgr_get_pdev_by_id_no_state(
			struct wlan_objmgr_psoc *psoc, uint8_t id,
			wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_get_pdev_by_macaddr() - retrieve pdev by macaddr
 * @psoc: PSOC object
 * @macaddr: MAC address
 * @dbg_id: id of the caller
 *
 * API to find pdev object pointer by pdev macaddr
 *
 * This API increments the ref count of the pdev object internally, the
 * caller has to invoke the wlan_objmgr_pdev_release_ref() to decrement
 * ref count
 *
 * Return: pdev pointer
 *         NULL on FAILURE
 */
struct wlan_objmgr_pdev *wlan_objmgr_get_pdev_by_macaddr(
		struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_get_pdev_by_macaddr_no_state() - retrieve pdev by macaddr
 * @psoc: PSOC object
 * @macaddr: MAC address
 * @dbg_id: id of the caller
 *
 * API to find pdev object pointer by pdev macaddr, ignores the state check
 *
 * This API increments the ref count of the pdev object internally, the
 * caller has to invoke the wlan_objmgr_pdev_release_ref() to decrement
 * ref count
 *
 * Return: pdev pointer
 *         NULL on FAILURE
 */
struct wlan_objmgr_pdev *wlan_objmgr_get_pdev_by_macaddr_no_state(
		struct wlan_objmgr_psoc *psoc, uint8_t *macaddr,
		wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_get_vdev_by_opmode_from_psoc() - retrieve vdev by opmode
 * @psoc: PSOC object
 * @opmode: vdev operating mode
 * @dbg_id: id of the caller
 *
 * API to find vdev object pointer by vdev operating mode from psoc
 *
 * This API increments the ref count of the vdev object internally, the
 * caller has to invoke the wlan_objmgr_vdev_release_ref() to decrement
 * ref count
 *
 * Return: vdev pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_opmode_from_psoc_debug(
			struct wlan_objmgr_psoc *psoc,
			enum QDF_OPMODE opmode,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line);

#define wlan_objmgr_get_vdev_by_opmode_from_psoc(psoc, opmode, dbgid) \
		wlan_objmgr_get_vdev_by_opmode_from_psoc_debug(psoc, opmode, \
		dbgid, __func__, __LINE__)
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_opmode_from_psoc(
			struct wlan_objmgr_psoc *psoc,
			enum QDF_OPMODE opmode,
			wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_get_vdev_by_id_from_psoc() - retrieve vdev by id
 * @psoc: PSOC object
 * @id: vdev id
 * @dbg_id: id of the caller
 *
 * API to find vdev object pointer by vdev id from psoc
 *
 * This API increments the ref count of the vdev object internally, the
 * caller has to invoke the wlan_objmgr_vdev_release_ref() to decrement
 * ref count
 *
 * Return: vdev pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line);

#define wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id, dbgid) \
		wlan_objmgr_get_vdev_by_id_from_psoc_debug(psoc, vdev_id, \
		dbgid, __func__, __LINE__)
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_get_vdev_by_id_from_psoc_no_state() - retrieve vdev by id
 * @psoc: PSOC object
 * @id: vdev id
 * @dbg_id: id of the caller
 *
 * API to find vdev object pointer by vdev id from psoc, ignores the
 * state check
 *
 * This API increments the ref count of the vdev object internally, the
 * caller has to invoke the wlan_objmgr_vdev_release_ref() to decrement
 * ref count
 *
 * Return: vdev pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc_no_state_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line);

#define wlan_objmgr_get_vdev_by_id_from_psoc_no_state(psoc, vdev_id, dbgid) \
		wlan_objmgr_get_vdev_by_id_from_psoc_no_state_debug(psoc, \
		vdev_id, dbgid, __func__, __LINE__)
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc_no_state(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_get_vdev_by_id_from_psoc_not_log_del() - retrieve vdev by id
 * @psoc: PSOC object
 * @id: vdev id
 * @dbg_id: id of the caller
 *
 * API to find vdev object pointer by vdev id from psoc, ignores the
 * state check
 *
 * This API increments the ref count of the vdev object internally, the
 * caller has to invoke the wlan_objmgr_vdev_release_ref() to decrement
 * ref count
 *
 * Return: vdev pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc_not_log_del_debug(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id,
			const char *func, int line);

#define wlan_objmgr_get_vdev_by_id_from_psoc_not_log_del(psoc, vdev_id, dbgid) \
		wlan_objmgr_get_vdev_by_id_from_psoc_not_log_del_debug(psoc, \
		vdev_id, dbgid, __func__, __LINE__)
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_id_from_psoc_not_log_del(
			struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_get_vdev_by_macaddr_from_psoc() - retrieve vdev by macaddr
 * @psoc: PSOC object
 * @pdev_id: Pdev id
 * @macaddr: macaddr
 * @dbg_id: id of the caller
 *
 * API to find vdev object pointer by vdev macaddr from pdev
 *
 * This API increments the ref count of the vdev object internally, the
 * caller has to invoke the wlan_objmgr_vdev_release_ref() to decrement
 * ref count
 *
 * Return: vdev pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_psoc_debug(
		struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
		uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id,
		const char *func, int line);

#define wlan_objmgr_get_vdev_by_macaddr_from_psoc(psoc, pdev_id, macaddr, \
	dbgid) \
		wlan_objmgr_get_vdev_by_macaddr_from_psoc_debug(psoc, pdev_id, \
		macaddr, dbgid, __func__, __LINE__)
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_psoc(
		struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
		uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_objmgr_get_vdev_by_macaddr_from_psoc_no_state() - retrieve vdev by
 *                                                           macaddr
 * @psoc: PSOC object
 * @pdev_id: Pdev id
 * @macaddr: macaddr
 * @dbg_id: id of the caller
 *
 * API to find vdev object pointer by vdev macaddr from psoc, ignores the state
 * check
 *
 * This API increments the ref count of the vdev object internally, the
 * caller has to invoke the wlan_objmgr_vdev_release_ref() to decrement
 * ref count
 *
 * Return: vdev pointer
 *         NULL on FAILURE
 */
#ifdef WLAN_OBJMGR_REF_ID_TRACE
struct wlan_objmgr_vdev
	*wlan_objmgr_get_vdev_by_macaddr_from_psoc_no_state_debug(
		struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
		uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id,
		const char *func, int line);

#define wlan_objmgr_get_vdev_by_macaddr_from_psoc_no_state(psoc, pdev_id, \
	macaddr, dbgid) \
		wlan_objmgr_get_vdev_by_macaddr_from_psoc_no_state_debug(psoc, \
		pdev_id, macaddr, dbgid, __func__, __LINE__)
#else
struct wlan_objmgr_vdev *wlan_objmgr_get_vdev_by_macaddr_from_psoc_no_state(
		struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
		uint8_t *macaddr, wlan_objmgr_ref_dbgid dbg_id);
#endif

/**
 * wlan_psoc_obj_lock() - Acquire PSOC spinlock
 * @psoc: PSOC object
 *
 * API to acquire PSOC lock
 * Parent lock should not be taken in child lock context
 * but child lock can be taken in parent lock context
 * (for ex: psoc lock can't be invoked in pdev/vdev/peer lock context)
 *
 * Return: void
 */
static inline void wlan_psoc_obj_lock(struct wlan_objmgr_psoc *psoc)
{
	qdf_spin_lock_bh(&psoc->psoc_lock);
}

/**
 * wlan_psoc_obj_unlock() - Release PSOC spinlock
 * @psoc: PSOC object
 *
 * API to Release PSOC lock
 *
 * Return: void
 */
static inline void wlan_psoc_obj_unlock(struct wlan_objmgr_psoc *psoc)
{
	qdf_spin_unlock_bh(&psoc->psoc_lock);
}

/**
 * wlan_psoc_set_nif_phy_version() - set nif phy version
 * @psoc: PSOC object
 * @phy_ver: phy version
 *
 * API to set nif phy version in psoc
 *
 * Return: void
 */
static inline void wlan_psoc_set_nif_phy_version(struct wlan_objmgr_psoc *psoc,
			uint32_t phy_ver)
{
	psoc->soc_nif.phy_version = phy_ver;
}

/**
 * wlan_psoc_get_nif_phy_version() - get nif phy version
 * @psoc: PSOC object
 *
 * API to set nif phy version in psoc
 *
 * Return: @phy_ver: phy version
 */
static inline uint32_t wlan_psoc_get_nif_phy_version(
			struct wlan_objmgr_psoc *psoc)
{
	if (!psoc)
		return (uint32_t)-1;

	return psoc->soc_nif.phy_version;
}

/**
 * wlan_psoc_set_dev_type() - set dev type
 * @psoc: PSOC object
 * @phy_type: phy type (OL/DA)
 *
 * API to set dev type in psoc
 *
 * Return: void
 */
static inline void wlan_psoc_set_dev_type(struct wlan_objmgr_psoc *psoc,
				WLAN_DEV_TYPE phy_type)
{
	psoc->soc_nif.phy_type = phy_type;
}

/**
 * wlan_objmgr_psoc_get_dev_type - get dev type
 * @psoc: PSOC object
 *
 * API to get dev type in psoc
 *
 * Return: phy type (OL/DA)
 */
static inline WLAN_DEV_TYPE wlan_objmgr_psoc_get_dev_type(
				struct wlan_objmgr_psoc *psoc)
{
	if (!psoc)
		return (uint32_t)-1;

	return psoc->soc_nif.phy_type;
}

/**
 * wlan_psoc_nif_fw_cap_set() - set fw caps
 * @psoc: PSOC object
 * @cap: capability flag to be set
 *
 * API to set fw caps in psoc
 *
 * Return: void
 */
static inline void wlan_psoc_nif_fw_cap_set(struct wlan_objmgr_psoc *psoc,
					uint32_t cap)
{
	psoc->soc_nif.soc_fw_caps |= cap;
}

/**
 * wlan_psoc_nif_fw_cap_clear() - clear fw caps
 * @psoc: PSOC object
 * @cap: capability flag to be cleared
 *
 * API to clear fw caps in psoc
 *
 * Return: void
 */
static inline void wlan_psoc_nif_fw_cap_clear(struct wlan_objmgr_psoc *psoc,
				uint32_t cap)
{
	psoc->soc_nif.soc_fw_caps &= ~cap;
}

/**
 * wlan_psoc_nif_fw_cap_get() - get fw caps
 * @psoc: PSOC object
 * @cap: capability flag to be checked
 *
 * API to know, whether particular fw caps flag is set in psoc
 *
 * Return: 1 (for set) or 0 (for not set)
 */
static inline uint8_t wlan_psoc_nif_fw_cap_get(struct wlan_objmgr_psoc *psoc,
				uint32_t cap)
{
	return (psoc->soc_nif.soc_fw_caps & cap) ? 1 : 0;
}

/**
 * wlan_psoc_nif_fw_ext_cap_set() - set fw ext caps
 * @psoc: PSOC object
 * @ext_cap: capability flag to be set
 *
 * API to set fw ext caps in psoc
 *
 * Return: void
 */
static inline void wlan_psoc_nif_fw_ext_cap_set(struct wlan_objmgr_psoc *psoc,
				uint32_t ext_cap)
{
	psoc->soc_nif.soc_fw_ext_caps |= ext_cap;
}

/**
 * wlan_psoc_nif_fw_ext_cap_clear() - clear fw ext caps
 * @psoc: PSOC object
 * @ext_cap: capability flag to be cleared
 *
 * API to clear fw ext caps in psoc
 *
 * Return: void
 */
static inline void wlan_psoc_nif_fw_ext_cap_clear(struct wlan_objmgr_psoc *psoc,
				uint32_t ext_cap)
{
	psoc->soc_nif.soc_fw_ext_caps &= ~ext_cap;
}

/**
 * wlan_psoc_nif_fw_ext_cap_get() - get fw caps
 * @psoc: PSOC object
 * @ext_cap: capability flag to be checked
 *
 * API to know, whether particular fw caps flag is set in psoc
 *
 * Return: 1 (for set) or 0 (for not set)
 */
static inline uint8_t wlan_psoc_nif_fw_ext_cap_get(
		struct wlan_objmgr_psoc *psoc, uint32_t ext_cap)
{
	return (psoc->soc_nif.soc_fw_ext_caps & ext_cap) ? 1 : 0;
}

/**
 * wlan_psoc_nif_feat_cap_set() - set feature caps
 * @psoc: PSOC object
 * @cap: feature flag to be set
 *
 * API to set feature caps in psoc
 *
 * Return: void
 */
static inline void wlan_psoc_nif_feat_cap_set(struct wlan_objmgr_psoc *psoc,
				uint32_t feat_cap)
{
	psoc->soc_nif.soc_feature_caps |= feat_cap;
}

/**
 * wlan_psoc_nif_feat_cap_clear() - clear feature caps
 * @psoc: PSOC object
 * @cap: feature flag to be cleared
 *
 * API to clear feature caps in psoc
 *
 * Return: void
 */
static inline void wlan_psoc_nif_feat_cap_clear(struct wlan_objmgr_psoc *psoc,
				uint32_t feat_cap)
{
	psoc->soc_nif.soc_feature_caps &= ~feat_cap;
}

/**
 * wlan_psoc_nif_feat_cap_get() - get feature caps
 * @psoc: PSOC object
 * @cap: feature flag to be checked
 *
 * API to know, whether particular feature cap flag is set in psoc
 *
 * Return: 1 (for set) or 0 (for not set)
 */
static inline uint8_t wlan_psoc_nif_feat_cap_get(struct wlan_objmgr_psoc *psoc,
							uint32_t feat_cap)
{
	return (psoc->soc_nif.soc_feature_caps & feat_cap) ? 1 : 0;
}

/**
 * wlan_psoc_nif_op_flag_get() - get op flags
 * @psoc: PSOC object
 * @flag: op flag to be checked
 *
 * API to know, whether particular op flag is set in psoc
 *
 * Return: 1 (for set) or 0 (for not set)
 */
static inline uint8_t wlan_psoc_nif_op_flag_get(struct wlan_objmgr_psoc *psoc,
						uint32_t flag)
{
	return (psoc->soc_nif.soc_op_flags & flag) ? 1 : 0;
}

/**
 * wlan_psoc_nif_op_flag_set() - set op flag
 * @psoc: PSOC object
 * @flag: op flag to be set
 *
 * API to set op flag in psoc
 *
 * Return: void
 */
static inline void wlan_psoc_nif_op_flag_set(struct wlan_objmgr_psoc *psoc,
						uint32_t flag)
{
	psoc->soc_nif.soc_op_flags |= flag;
}

/**
 * wlan_psoc_nif_op_flag_clear() - clear op flag
 * @psoc: PSOC object
 * @flag: op flag to be cleared
 *
 * API to clear op flag in psoc
 *
 * Return: void
 */
static inline void wlan_psoc_nif_op_flag_clear(struct wlan_objmgr_psoc *psoc,
						uint32_t flag)
{
	psoc->soc_nif.soc_op_flags &= ~flag;
}

/**
 * wlan_psoc_set_hw_macaddr() - set hw mac addr
 * @psoc: PSOC object
 * @macaddr: hw macaddr
 *
 * API to set hw macaddr of psoc
 *
 * Caller need to acquire lock with wlan_psoc_obj_lock()
 *
 * Return: void
 */
static inline void wlan_psoc_set_hw_macaddr(struct wlan_objmgr_psoc *psoc,
					uint8_t *macaddr)
{
	/* This API is invoked with lock acquired, do not add log prints */
	if (psoc)
		WLAN_ADDR_COPY(psoc->soc_nif.soc_hw_macaddr, macaddr);
}

/**
 * wlan_psoc_get_hw_macaddr() - get hw macaddr
 * @psoc: PSOC object
 *
 * API to set hw macaddr of psoc
 *
 * Return: hw macaddr
 */
static inline uint8_t *wlan_psoc_get_hw_macaddr(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc)
		return NULL;

	return psoc->soc_nif.soc_hw_macaddr;
}

/**
 * wlan_objmgr_psoc_get_comp_private_obj() - API to retrieve component object
 * @psoc: Psoc pointer
 * @id: component id
 *
 * This API is used to get the component private object pointer tied to the
 * corresponding psoc object
 *
 * Return: Component private object
 */
void *wlan_objmgr_psoc_get_comp_private_obj(struct wlan_objmgr_psoc *psoc,
					enum wlan_umac_comp_id id);
/**
 * wlan_psoc_get_pdev_count() - get pdev count for psoc
 * @psoc: PSOC object
 *
 * API to get number of pdev's attached to the psoc
 *
 * Return: number of pdev's
 */
static inline uint8_t wlan_psoc_get_pdev_count(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc)
		return 0;

	return psoc->soc_objmgr.wlan_pdev_count;
}

/**
 * wlan_psoc_set_lmac_if_txops() - API to set tx ops handle in psoc object
 * @psoc: Psoc pointer
 * @tx_ops: tx callbacks handle
 *
 * API to set tx callbacks handle in psoc object
 *
 * Return: None
 */
static inline
void wlan_psoc_set_lmac_if_txops(struct wlan_objmgr_psoc *psoc,
			  struct wlan_lmac_if_tx_ops *tx_ops)
{
	if (!psoc)
		return;

	psoc->soc_cb.tx_ops = tx_ops;
}

/**
 * wlan_psoc_get_lmac_if_txops() - API to get tx ops handle
 * @psoc: Psoc pointer
 *
 * API to get tx callbacks handle from psoc object
 *
 * Return: tx callbacks handle
 */
static inline
struct wlan_lmac_if_tx_ops *wlan_psoc_get_lmac_if_txops(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc)
		return NULL;

	return psoc->soc_cb.tx_ops;
}

/**
 * wlan_psoc_set_lmac_if_rxops() - API to set rx ops handle in psoc object
 * @psoc: Psoc pointer
 * @tgt_if_handle: rx callbacks handle
 *
 * API to set rx callbacks handle in psoc object
 *
 * Return: None
 */
static inline
void wlan_psoc_set_lmac_if_rxops(struct wlan_objmgr_psoc *psoc, struct
		wlan_lmac_if_rx_ops *rx_ops)
{
	if (!psoc)
		return;

	psoc->soc_cb.rx_ops = rx_ops;
}

/**
 * wlan_psoc_get_lmac_if_rxops() - API to get rx ops handle
 * @psoc: Psoc pointer
 *
 * API to get rx callbacks handle from psoc object
 *
 * Return: rx callbacks handle
 */
static inline
struct wlan_lmac_if_rx_ops *wlan_psoc_get_lmac_if_rxops(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc)
		return NULL;

	return psoc->soc_cb.rx_ops;
}

/**
 * wlan_psoc_set_tgt_if_handle() - API to set target if handle in psoc object
 * @psoc: Psoc pointer
 * @tgt_if_handle: target interface handle
 *
 * API to set target interface handle in psoc object
 *
 * Return: None
 */
static inline
void wlan_psoc_set_tgt_if_handle(struct wlan_objmgr_psoc *psoc,
				 struct target_psoc_info *tgt_if_handle)
{
	if (!psoc)
		return;

	psoc->tgt_if_handle = tgt_if_handle;
}

/**
 * wlan_psoc_get_tgt_if_handle() - API to get target interface handle
 * @psoc: Psoc pointer
 *
 * API to get target interface handle from psoc object
 *
 * Return: target interface handle
 */
static inline
struct target_psoc_info *wlan_psoc_get_tgt_if_handle(
				struct wlan_objmgr_psoc *psoc)
{
	if (!psoc)
		return NULL;

	return psoc->tgt_if_handle;
}

/**
 * wlan_psoc_get_qdf_dev() - API to get qdf device
 * @psoc: Psoc pointer
 *
 * API to get qdf device from psoc object
 *
 * Return: qdf_device_t
 */
static inline qdf_device_t wlan_psoc_get_qdf_dev(
			struct wlan_objmgr_psoc *psoc)
{
	if (!psoc)
		return NULL;

	return psoc->soc_objmgr.qdf_dev;
}

/**
 * wlan_psoc_set_qdf_dev() - API to get qdf device
 * @psoc: Psoc pointer
 * dev: qdf device
 *
 * API to set qdf device from psoc object
 *
 * Return: None
 */
static inline void wlan_psoc_set_qdf_dev(
			struct wlan_objmgr_psoc *psoc,
			qdf_device_t dev)
{
	if (!psoc)
		return;

	psoc->soc_objmgr.qdf_dev = dev;
}

/**
 * wlan_psoc_set_max_vdev_count() - set psoc max vdev count
 * @psoc: PSOC object
 * @vdev count: Max vdev count
 *
 * API to set Max vdev count
 *
 * Return: void
 */
static inline void wlan_psoc_set_max_vdev_count(struct wlan_objmgr_psoc *psoc,
						uint8_t max_vdev_count)
{
	if (max_vdev_count > WLAN_UMAC_PSOC_MAX_VDEVS)
		QDF_BUG(0);

	psoc->soc_objmgr.max_vdev_count = max_vdev_count;
}

/**
 * wlan_psoc_get_max_vdev_count() - get psoc max vdev count
 * @psoc: PSOC object
 *
 * API to set Max vdev count
 *
 * Return: @vdev count: Max vdev count
 */
static inline uint8_t wlan_psoc_get_max_vdev_count(
					struct wlan_objmgr_psoc *psoc)
{
	return psoc->soc_objmgr.max_vdev_count;
}

/**
 * wlan_psoc_set_max_peer_count() - set psoc max peer count
 * @psoc: PSOC object
 * @peer count: Max peer count
 *
 * API to set Max peer count
 *
 * Return: void
 */
static inline void wlan_psoc_set_max_peer_count(struct wlan_objmgr_psoc *psoc,
						uint16_t max_peer_count)
{
	if (max_peer_count > WLAN_UMAC_PSOC_MAX_PEERS)
		QDF_BUG(0);

	psoc->soc_objmgr.max_peer_count = max_peer_count;
}

/**
 * wlan_psoc_get_max_peer_count() - get psoc max peer count
 * @psoc: PSOC object
 *
 * API to set Max peer count
 *
 * Return: @peer count: Max peer count
 */
static inline uint16_t wlan_psoc_get_max_peer_count(
					struct wlan_objmgr_psoc *psoc)
{
	return psoc->soc_objmgr.max_peer_count;
}

/**
 * wlan_psoc_get_peer_count() - get psoc peer count
 * @psoc: PSOC object
 *
 * API to get peer count
 *
 * Return: @peer count: peer count
 */
static inline uint16_t wlan_psoc_get_peer_count(
					struct wlan_objmgr_psoc *psoc)
{
	return psoc->soc_objmgr.wlan_peer_count;
}


/**
 * DOC: Examples to use PSOC ref count APIs
 *
 * In all the scenarios, the pair of API should be followed
 * other it lead to memory leak
 *
 *  scenario 1:
 *
 *     wlan_objmgr_psoc_obj_create()
 *     ----
 *     wlan_objmgr_psoc_obj_delete()
 *
 *  scenario 2:
 *
 *     wlan_objmgr_psoc_get_ref()
 *     ----
 *     the operations which are done on
 *     psoc object
 *     ----
 *     wlan_objmgr_psoc_release_ref()
 */

/**
 * wlan_objmgr_psoc_get_ref() - increment ref count
 * @psoc: PSOC object
 * @id:   Object Manager ref debug id
 *
 * API to increment ref count of psoc
 *
 * Return: void
 */
void wlan_objmgr_psoc_get_ref(struct wlan_objmgr_psoc *psoc,
					wlan_objmgr_ref_dbgid id);

/**
 * wlan_objmgr_psoc_try_get_ref() - increment ref count, if allowed
 * @psoc: PSOC object
 * @id:   Object Manager ref debug id
 *
 * API to increment ref count after checking valid object state
 *
 * Return: void
 */
QDF_STATUS wlan_objmgr_psoc_try_get_ref(struct wlan_objmgr_psoc *psoc,
						wlan_objmgr_ref_dbgid id);

/**
 * wlan_objmgr_psoc_release_ref() - decrement ref count
 * @psoc: PSOC object
 * @id:   Object Manager ref debug id
 *
 * API to decrement ref count of psoc, if ref count is 1, it initiates the
 * PSOC deletion
 *
 * Return: void
 */
void wlan_objmgr_psoc_release_ref(struct wlan_objmgr_psoc *psoc,
						wlan_objmgr_ref_dbgid id);

/**
 * wlan_objmgr_print_ref_all_objects_per_psoc() - print all psoc objects'
 *                                                ref counts
 * @psoc: PSOC object
 *
 * API to be used for printing all the objects(pdev/vdev/peer) ref counts
 *
 * Return: SUCCESS/FAILURE
 */
QDF_STATUS wlan_objmgr_print_ref_all_objects_per_psoc(
		struct wlan_objmgr_psoc *psoc);

/**
* wlan_objmgr_psoc_set_user_config () - populate user config
* data in psoc
* @psoc: psoc object pointer
* @user_config_data: pointer to user config data filled up by os
*                  dependent component
* it is intended to set all elements by OSIF/HDD and it not
* intended to modify a single element
* Return: QDF status
*/
QDF_STATUS wlan_objmgr_psoc_set_user_config(struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_psoc_user_config *user_config_data);

/**
 * wlan_objmgr_psoc_check_for_pdev_leaks() - Assert no pdevs attached to @psoc
 * @psoc: The psoc to check
 *
 * Return: No. of psoc leaks
 */
uint32_t wlan_objmgr_psoc_check_for_pdev_leaks(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_objmgr_psoc_check_for_vdev_leaks() - Assert no vdevs attached to @psoc
 * @psoc: The psoc to check
 *
 * Return: No. of vdev leaks
 */
uint32_t wlan_objmgr_psoc_check_for_vdev_leaks(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_objmgr_psoc_check_for_peer_leaks() - Assert no peers attached to @psoc
 * @psoc: The psoc to check
 *
 * Return: No. of peer leaks
 */
uint32_t wlan_objmgr_psoc_check_for_peer_leaks(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_objmgr_psoc_check_for_leaks() - Assert on leak
 * @psoc: The psoc to check
 *
 * Return: None
 */
void wlan_objmgr_psoc_check_for_leaks(struct wlan_objmgr_psoc *psoc);

/**
* wlan_objmgr_psoc_get_band_capability () - get user config
* data for band capability
* @psoc: psoc object pointer
*
* Return: band_capability
*/
static inline uint8_t wlan_objmgr_psoc_get_band_capability(
		struct wlan_objmgr_psoc *psoc)
{
	if (!psoc)
		return 0;

	return psoc->soc_nif.user_config.band_capability;
}

/**
 * wlan_psoc_set_dp_handle() - set dp handle
 * @psoc: psoc object pointer
 * @dp_handle: Data path module handle
 *
 * Return: void
 */
static inline void wlan_psoc_set_dp_handle(struct wlan_objmgr_psoc *psoc,
		void *dp_handle)
{
	if (qdf_unlikely(!psoc)) {
		QDF_BUG(0);
		return;
	}

	psoc->dp_handle = dp_handle;
}

/**
 * wlan_psoc_get_dp_handle() - get dp handle
 * @psoc: psoc object pointer
 *
 * Return: dp handle
 */
static inline void *wlan_psoc_get_dp_handle(struct wlan_objmgr_psoc *psoc)
{
	if (qdf_unlikely(!psoc)) {
		QDF_BUG(0);
		return NULL;
	}

	return psoc->dp_handle;
}

struct wlan_logically_del_peer {
	qdf_list_node_t list;
	struct wlan_objmgr_peer *peer;
};

/**
 * wlan_psoc_get_id() - get psoc id
 * @psoc: PSOC object
 *
 * API to get psoc id
 *
 * Return: @psoc_id: psoc id
 */
static inline uint8_t wlan_psoc_get_id(
			struct wlan_objmgr_psoc *psoc)
{
	if (!psoc)
		return (uint8_t)-1;

	return psoc->soc_objmgr.psoc_id;
}

/**
 * wlan_print_psoc_info() - print psoc members
 * @psoc: psoc object pointer
 *
 * Return: void
 */
#ifdef WLAN_OBJMGR_DEBUG
void wlan_print_psoc_info(struct wlan_objmgr_psoc *psoc);
#else
static inline void wlan_print_psoc_info(struct wlan_objmgr_psoc *psoc) {}
#endif

#endif /* _WLAN_OBJMGR_PSOC_OBJ_H_*/
