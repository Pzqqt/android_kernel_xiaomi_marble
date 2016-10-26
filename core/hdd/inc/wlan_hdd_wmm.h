/*
 * Copyright (c) 2011-2012,2016 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_HDD_WMM_H
#define _WLAN_HDD_WMM_H

/**
 * DOC: HDD WMM
 *
 * This module (wlan_hdd_wmm.h interface + wlan_hdd_wmm.c implementation)
 * houses all the logic for WMM in HDD.
 *
 * On the control path, it has the logic to setup QoS, modify QoS and delete
 * QoS (QoS here refers to a TSPEC). The setup QoS comes in two flavors: an
 * explicit application invoked and an internal HDD invoked.  The implicit QoS
 * is for applications that do NOT call the custom QCT WLAN OIDs for QoS but
 * which DO mark their traffic for priortization. It also has logic to start,
 * update and stop the U-APSD trigger frame generation. It also has logic to
 * read WMM related config parameters from the registry.
 *
 * On the data path, it has the logic to figure out the WMM AC of an egress
 * packet and when to signal TL to serve a particular AC queue. It also has the
 * logic to retrieve a packet based on WMM priority in response to a fetch from
 * TL.
 *
 * The remaining functions are utility functions for information hiding.
 */

/* Include files */
#include <linux/workqueue.h>
#include <linux/list.h>
#include <wlan_hdd_main.h>
#include <wlan_hdd_wext.h>
#include <sme_qos_api.h>

/*Maximum number of ACs */
#define WLAN_MAX_AC                         4


/* Preprocessor Definitions and Constants */

/* #define HDD_WMM_DEBUG 1 */

#define HDD_WMM_CTX_MAGIC 0x574d4d58    /* "WMMX" */

#define HDD_WMM_HANDLE_IMPLICIT 0xFFFFFFFF

#define HDD_WLAN_INVALID_STA_ID 0xFF

/* Type Declarations */

/**
 * enum hdd_wmm_classification: types of classification supported
 */
typedef enum hdd_wmm_classification {
	HDD_WMM_CLASSIFICATION_DSCP = 0,
	HDD_WMM_CLASSIFICATION_802_1Q = 1
} hdd_wmm_classification_t;

/**
 * enum hdd_wmm_user_mode - WMM modes of operation
 *
 * @HDD_WMM_USER_MODE_AUTO: STA can associate with any AP, & HDD looks at
 *	the SME notification after association to find out if associated
 *	with QAP and acts accordingly
 * @HDD_WMM_USER_MODE_QBSS_ONLY - SME will add the extra logic to make sure
 *	STA associates with a QAP only
 * @HDD_WMM_USER_MODE_NO_QOS - Join any AP, but uapsd is disabled
 */
typedef enum hdd_wmm_user_mode {
	HDD_WMM_USER_MODE_AUTO = 0,
	HDD_WMM_USER_MODE_QBSS_ONLY = 1,
	HDD_WMM_USER_MODE_NO_QOS = 2,
} hdd_wmm_user_mode_t;

/* UAPSD Mask bits */
/* (Bit0:VO; Bit1:VI; Bit2:BK; Bit3:BE all other bits are ignored) */
#define HDD_AC_VO 0x1
#define HDD_AC_VI 0x2
#define HDD_AC_BK 0x4
#define HDD_AC_BE 0x8

/**
 * struct hdd_wmm_qos_context - HDD WMM QoS Context
 *
 * This structure holds the context for a single flow which has either
 * been confgured explicitly from userspace or implicitly via the
 * Implicit QoS feature.
 *
 * @node: list node which can be used to put the context into a list
 *	of contexts
 * @handle: identifer which uniquely identifies this context to userspace
 * @qosFlowID: identifier which uniquely identifies this flow to SME
 * @pAdapter: adapter upon which this flow was configured
 * @acType: access category for this flow
 * @lastStatus: the status of the last operation performed on this flow by SME
 * @wmmAcSetupImplicitQos: work structure used for deferring implicit QoS work
 *	from softirq context to thread context
 * @magic: magic number used to verify that this is a valid context when
 *	referenced anonymously
 */
typedef struct hdd_wmm_qos_context {
	struct list_head node;
	uint32_t handle;
	uint32_t qosFlowId;
	hdd_adapter_t *pAdapter;
	sme_ac_enum_type acType;
	hdd_wlan_wmm_status_e lastStatus;
	struct work_struct wmmAcSetupImplicitQos;
	uint32_t magic;
	bool is_inactivity_timer_running;
} hdd_wmm_qos_context_t;

/**
 * struct hdd_wmm_ac_status - WMM related per-AC state & status info
 * @wmmAcAccessRequired - does the AP require access to this AC?
 * @wmmAcAccessNeeded - does the worker thread need to acquire access to
 *	this AC?
 * @wmmAcAccessPending - is implicit QoS negotiation currently taking place?
 * @wmmAcAccessFailed - has implicit QoS negotiation already failed?
 * @wmmAcAccessGranted - has implicit QoS negotiation already succeeded?
 * @wmmAcAccessAllowed - is access to this AC allowed, either because we
 *	are not doing WMM, we are not doing implicit QoS, implict QoS has
 *	completed, or explicit QoS has completed?
 * @wmmAcTspecValid - is the wmmAcTspecInfo valid?
 * @wmmAcUapsdInfoValid - are the wmmAcUapsd* fields valid?
 * @wmmAcTspecInfo - current (possibly aggregate) Tspec for this AC
 * @wmmAcIsUapsdEnabled - is UAPSD enabled on this AC?
 * @wmmAcUapsdServiceInterval - service interval for this AC
 * @wmmAcUapsdSuspensionInterval - suspension interval for this AC
 * @wmmAcUapsdDirection - direction for this AC
 * @wmmInactivityTime - inactivity time for this AC
 * @wmmPrevTrafficCnt - TX counter used for inactivity detection
 * @wmmInactivityTimer - timer used for inactivity detection
 */
typedef struct hdd_wmm_ac_status {
	bool wmmAcAccessRequired;
	bool wmmAcAccessNeeded;
	bool wmmAcAccessPending;
	bool wmmAcAccessFailed;
	bool wmmAcAccessGranted;
	bool wmmAcAccessAllowed;
	bool wmmAcTspecValid;
	bool wmmAcUapsdInfoValid;
	sme_QosWmmTspecInfo wmmAcTspecInfo;
	bool wmmAcIsUapsdEnabled;
	uint32_t wmmAcUapsdServiceInterval;
	uint32_t wmmAcUapsdSuspensionInterval;
	sme_QosWmmDirType wmmAcUapsdDirection;

#ifdef FEATURE_WLAN_ESE
	uint32_t wmmInactivityTime;
	uint32_t wmmPrevTrafficCnt;
	qdf_mc_timer_t wmmInactivityTimer;
#endif

} hdd_wmm_ac_status_t;

/**
 * struct hdd_wmm_status - WMM status maintained per-adapter
 * @wmmContextList - list of WMM contexts active on the adapter
 * @wmmLock - mutex used for exclusive access to this adapter's WMM status
 * @wmmACStatus - per-AC WMM status
 * @wmmQap - is this connected to a QoS-enabled AP?
 * @wmmQosConnection - is this a QoS connection?
 */
typedef struct hdd_wmm_status {
	struct list_head wmmContextList;
	struct mutex wmmLock;
	hdd_wmm_ac_status_t wmmAcStatus[WLAN_MAX_AC];
	bool wmmQap;
	bool wmmQosConnection;
} hdd_wmm_status_t;

extern const uint8_t hdd_qdisc_ac_to_tl_ac[];
extern const uint8_t hdd_wmm_up_to_ac_map[];
extern const uint8_t hdd_linux_up_to_ac_map[];

#define WLAN_HDD_MAX_DSCP 0x3f

/**
 * hdd_wmmps_helper() - Function to set uapsd psb dynamically
 *
 * @pAdapter: [in] pointer to adapter structure
 * @ptr: [in] pointer to command buffer
 *
 * Return: Zero on success, appropriate error on failure.
 */
int hdd_wmmps_helper(hdd_adapter_t *pAdapter, uint8_t *ptr);

/**
 * hdd_wmm_init() - initialize the WMM DSCP configuation
 * @pAdapter : [in]  pointer to Adapter context
 *
 * This function will initialize the WMM DSCP configuation of an
 * adapter to an initial state.  The configuration can later be
 * overwritten via application APIs or via QoS Map sent OTA.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_wmm_init(hdd_adapter_t *pAdapter);

/**
 * hdd_wmm_adapter_init() - initialize the WMM configuration of an adapter
 * @pAdapter: [in]  pointer to Adapter context
 *
 * This function will initialize the WMM configuation and status of an
 * adapter to an initial state.  The configuration can later be
 * overwritten via application APIs
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_wmm_adapter_init(hdd_adapter_t *pAdapter);

/**
 * hdd_wmm_close() - WMM close function
 * @pAdapter: [in]  pointer to adapter context
 *
 * Function which will perform any necessary work to to clean up the
 * WMM functionality prior to the kernel module unload.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_wmm_adapter_close(hdd_adapter_t *pAdapter);

/**
 * hdd_wmm_select_queue() - Function which will classify the packet
 *       according to linux qdisc expectation.
 *
 * @dev: [in] pointer to net_device structure
 * @skb: [in] pointer to os packet
 *
 * Return: Qdisc queue index
 */
uint16_t hdd_wmm_select_queue(struct net_device *dev, struct sk_buff *skb);

/**
 * hdd_hostapd_select_queue() - Function which will classify the packet
 *       according to linux qdisc expectation.
 *
 * @dev: [in] pointer to net_device structure
 * @skb: [in] pointer to os packet
 *
 * Return: Qdisc queue index
 */
uint16_t hdd_hostapd_select_queue(struct net_device *dev, struct sk_buff *skb
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0))
				  , void *accel_priv
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
				  , select_queue_fallback_t fallback
#endif
);

/**
 * hdd_wmm_acquire_access_required() - Function which will determine
 * acquire admittance for a WMM AC is required or not based on psb configuration
 * done in framework
 *
 * @pAdapter: [in] pointer to adapter structure
 * @acType: [in] WMM AC type of OS packet
 *
 * Return: void
 */
void hdd_wmm_acquire_access_required(hdd_adapter_t *pAdapter,
				     sme_ac_enum_type acType);

/**
 * hdd_wmm_acquire_access() - Function which will attempt to acquire
 * admittance for a WMM AC
 *
 * @pAdapter: [in]  pointer to adapter context
 * @acType: [in]  WMM AC type of OS packet
 * @pGranted: [out] pointer to bool flag when indicates if access
 *	      has been granted or not
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_wmm_acquire_access(hdd_adapter_t *pAdapter,
				  sme_ac_enum_type acType, bool *pGranted);

/**
 * hdd_wmm_assoc() - Function which will handle the housekeeping
 * required by WMM when association takes place
 *
 * @pAdapter: [in]  pointer to adapter context
 * @pRoamInfo: [in]  pointer to roam information
 * @eBssType: [in]  type of BSS
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_wmm_assoc(hdd_adapter_t *pAdapter,
			 tCsrRoamInfo *pRoamInfo, eCsrRoamBssType eBssType);

/**
 * hdd_wmm_connect() - Function which will handle the housekeeping
 * required by WMM when a connection is established
 *
 * @pAdapter : [in]  pointer to adapter context
 * @pRoamInfo: [in]  pointer to roam information
 * @eBssType : [in]  type of BSS
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_wmm_connect(hdd_adapter_t *pAdapter,
			   tCsrRoamInfo *pRoamInfo, eCsrRoamBssType eBssType);

/**
 * hdd_wmm_get_uapsd_mask() - Function which will calculate the
 * initial value of the UAPSD mask based upon the device configuration
 *
 * @pAdapter  : [in]  pointer to adapter context
 * @pUapsdMask: [out] pointer to where the UAPSD Mask is to be stored
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_wmm_get_uapsd_mask(hdd_adapter_t *pAdapter,
				  uint8_t *pUapsdMask);

/**
 * hdd_wmm_is_active() - Function which will determine if WMM is
 * active on the current connection
 *
 * @pAdapter: [in]  pointer to adapter context
 *
 * Return: true if WMM is enabled, false if WMM is not enabled
 */
bool hdd_wmm_is_active(hdd_adapter_t *pAdapter);

/**
 * hdd_wmm_addts() - Function which will add a traffic spec at the
 * request of an application
 *
 * @pAdapter  : [in]  pointer to adapter context
 * @handle    : [in]  handle to uniquely identify a TS
 * @pTspec    : [in]  pointer to the traffic spec
 *
 * Return: HDD_WLAN_WMM_STATUS_*
 */
hdd_wlan_wmm_status_e hdd_wmm_addts(hdd_adapter_t *pAdapter,
				    uint32_t handle,
				    sme_QosWmmTspecInfo *pTspec);

/**
 * hdd_wmm_delts() - Function which will delete a traffic spec at the
 * request of an application
 *
 * @pAdapter: [in]  pointer to adapter context
 * @handle: [in]  handle to uniquely identify a TS
 *
 * Return: HDD_WLAN_WMM_STATUS_*
 */
hdd_wlan_wmm_status_e hdd_wmm_delts(hdd_adapter_t *pAdapter, uint32_t handle);

/**
 * hdd_wmm_checkts() - Function which will return the status of a traffic
 * spec at the request of an application
 *
 * @pAdapter: [in]  pointer to adapter context
 * @handle: [in]  handle to uniquely identify a TS
 *
 * Return: HDD_WLAN_WMM_STATUS_*
 */
hdd_wlan_wmm_status_e hdd_wmm_checkts(hdd_adapter_t *pAdapter,
				      uint32_t handle);
/**
 * hdd_wmm_adapter_clear() - Function which will clear the WMM status
 * for all the ACs
 *
 * @pAdapter: [in]  pointer to Adapter context
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS hdd_wmm_adapter_clear(hdd_adapter_t *pAdapter);

void wlan_hdd_process_peer_unauthorised_pause(hdd_adapter_t *adapter);
#endif /* #ifndef _WLAN_HDD_WMM_H */
