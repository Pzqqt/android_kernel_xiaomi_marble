/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
 /**
 * @file cdp_txrx_ctrl.h
 * @brief Define the host data path control API functions
 * called by the host control SW and the OS interface module
 */

#ifndef _CDP_TXRX_CTRL_H_
#define _CDP_TXRX_CTRL_H_
/* TODO: adf need to be replaced with qdf */
/*
 * Cleanups --  Might need cleanup
 */
#if !QCA_OL_TX_PDEV_LOCK && QCA_NSS_PLATFORM || \
	(defined QCA_PARTNER_PLATFORM && QCA_PARTNER_SUPPORT_FAST_TX)
#define VAP_TX_SPIN_LOCK(_x) spin_lock(_x)
#define VAP_TX_SPIN_UNLOCK(_x) spin_unlock(_x)
#else /* QCA_OL_TX_PDEV_LOCK */
#define VAP_TX_SPIN_LOCK(_x)
#define VAP_TX_SPIN_UNLOCK(_x)
#endif /* QCA_OL_TX_PDEV_LOCK */

#if QCA_OL_TX_PDEV_LOCK
void ol_ll_pdev_tx_lock(void *);
void ol_ll_pdev_tx_unlock(void *);
#define OL_TX_LOCK(_x)  ol_ll_pdev_tx_lock(_x)
#define OL_TX_UNLOCK(_x) ol_ll_pdev_tx_unlock(_x)

#define OL_TX_PDEV_LOCK(_x)  qdf_spin_lock_bh(_x)
#define OL_TX_PDEV_UNLOCK(_x) qdf_spin_unlock_bh(_x)
#else
#define OL_TX_PDEV_LOCK(_x)
#define OL_TX_PDEV_UNLOCK(_x)

#define OL_TX_LOCK(_x)
#define OL_TX_UNLOCK(_x)
#endif /* QCA_OL_TX_PDEV_LOCK */

#if !QCA_OL_TX_PDEV_LOCK
#define OL_TX_FLOW_CTRL_LOCK(_x)  qdf_spin_lock_bh(_x)
#define OL_TX_FLOW_CTRL_UNLOCK(_x) qdf_spin_unlock_bh(_x)

#define OL_TX_DESC_LOCK(_x)  qdf_spin_lock_bh(_x)
#define OL_TX_DESC_UNLOCK(_x) qdf_spin_unlock_bh(_x)

#define OSIF_VAP_TX_LOCK(_x)  spin_lock(&((_x)->tx_lock))
#define OSIF_VAP_TX_UNLOCK(_x)  spin_unlock(&((_x)->tx_lock))

#define OL_TX_PEER_LOCK(_x, _id) qdf_spin_lock_bh(&((_x)->peer_lock[_id]))
#define OL_TX_PEER_UNLOCK(_x, _id) qdf_spin_unlock_bh(&((_x)->peer_lock[_id]))

#define OL_TX_PEER_UPDATE_LOCK(_x, _id) \
	qdf_spin_lock_bh(&((_x)->peer_lock[_id]))
#define OL_TX_PEER_UPDATE_UNLOCK(_x, _id) \
	qdf_spin_unlock_bh(&((_x)->peer_lock[_id]))

#else
#define OSIF_VAP_TX_LOCK(_x)  ol_ll_pdev_tx_lock((_x)->iv_txrx_handle)
#define OSIF_VAP_TX_UNLOCK(_x) ol_ll_pdev_tx_unlock((_x)->iv_txrx_handle)

#define OL_TX_FLOW_CTRL_LOCK(_x)
#define OL_TX_FLOW_CTRL_UNLOCK(_x)

#define OL_TX_DESC_LOCK(_x)
#define OL_TX_DESC_UNLOCK(_x)

#define OL_TX_PEER_LOCK(_x, _id)
#define OL_TX_PEER_UNLOCK(_x, _id)

#define OL_TX_PEER_UPDATE_LOCK(_x, _id) qdf_spin_lock_bh(&((_x)->tx_lock))
#define OL_TX_PEER_UPDATE_UNLOCK(_x, _id) qdf_spin_unlock_bh(&((_x)->tx_lock))

#endif /* !QCA_OL_TX_PDEV_LOCK */


extern int ol_txrx_is_target_ar900b(ol_txrx_vdev_handle vdev);
#define OL_TXRX_IS_TARGET_AR900B(vdev)  ol_txrx_is_target_ar900b(vdev)


/* WIN */
int
ol_txrx_mempools_attach(ol_pdev_handle ctrl_pdev);
int
ol_txrx_set_filter_neighbour_peers(
	ol_txrx_pdev_handle pdev,
	u_int32_t val);
/**
 * @brief set the safemode of the device
 * @details
 *  This flag is used to bypass the encrypt and decrypt processes when send and
 *  receive packets. It works like open AUTH mode, HW will treate all packets
 *  as non-encrypt frames because no key installed. For rx fragmented frames,
 *  it bypasses all the rx defragmentaion.
 *
 * @param vdev - the data virtual device object
 * @param val - the safemode state
 * @return - void
 */

void
ol_txrx_set_safemode(
	ol_txrx_vdev_handle vdev,
	u_int32_t val);
/**
 * @brief configure the drop unencrypted frame flag
 * @details
 *  Rx related. When set this flag, all the unencrypted frames
 *  received over a secure connection will be discarded
 *
 * @param vdev - the data virtual device object
 * @param val - flag
 * @return - void
 */
void
ol_txrx_set_drop_unenc(
	ol_txrx_vdev_handle vdev,
	u_int32_t val);


/**
 * @brief set the Tx encapsulation type of the VDEV
 * @details
 *  This will be used to populate the HTT desc packet type field during Tx
 *
 * @param vdev - the data virtual device object
 * @param val - the Tx encap type
 * @return - void
 */
void
ol_txrx_set_tx_encap_type(
	ol_txrx_vdev_handle vdev,
	enum htt_pkt_type val);

/**
 * @brief set the Rx decapsulation type of the VDEV
 * @details
 *  This will be used to configure into firmware and hardware which format to
 *  decap all Rx packets into, for all peers under the VDEV.
 *
 * @param vdev - the data virtual device object
 * @param val - the Rx decap mode
 * @return - void
 */
void
ol_txrx_set_vdev_rx_decap_type(
	ol_txrx_vdev_handle vdev,
	enum htt_pkt_type val);

/**
 * @brief get the Rx decapsulation type of the VDEV
 *
 * @param vdev - the data virtual device object
 * @return - the Rx decap type
 */
enum htt_pkt_type
ol_txrx_get_vdev_rx_decap_type(ol_txrx_vdev_handle vdev);

/* Is this similar to ol_txrx_peer_state_update() in MCL */
/**
 * @brief Update the authorize peer object at association time
 * @details
 *  For the host-based implementation of rate-control, it
 *  updates the peer/node-related parameters within rate-control
 *  context of the peer at association.
 *
 * @param peer - pointer to the node's object
 * @authorize - either to authorize or unauthorize peer
 *
 * @return none
 */
void
ol_txrx_peer_authorize(struct ol_txrx_peer_t *peer, u_int32_t authorize);

bool
ol_txrx_set_inact_params(ol_txrx_pdev_handle pdev,
			u_int16_t inact_check_interval,
			u_int16_t inact_normal,
			u_int16_t inact_overload);
bool
ol_txrx_start_inact_timer(
	ol_txrx_pdev_handle pdev,
	bool enable);

/**
 * @brief Set the overload status of the radio
 * @details
 *   Set the overload status of the radio, updating the inactivity
 *   threshold and inactivity count for each node.
 *
 * @param pdev - the data physical device object
 * @param overload - whether the radio is overloaded or not
 */
void
ol_txrx_set_overload(
	ol_txrx_pdev_handle pdev,
	bool overload);
/**
 * @brief Check the inactivity status of the peer/node
 *
 * @param peer - pointer to the node's object
 * @return true if the node is inactive; otherwise return false
 */
bool
ol_txrx_peer_is_inact(ol_txrx_peer_handle peer);

/**
 * @brief Mark inactivity status of the peer/node
 * @details
 *   If it becomes active, reset inactivity count to reload value;
 *   if the inactivity status changed, notify umac band steering.
 *
 * @param peer - pointer to the node's object
 * @param inactive - whether the node is inactive or not
 */
void
ol_txrx_mark_peer_inact(
	ol_txrx_peer_handle peer,
	bool inactive);


/* Should be ol_txrx_ctrl_api.h */
void ol_txrx_set_mesh_mode(ol_txrx_vdev_handle vdev, u_int32_t val);

void ol_tx_flush_buffers(struct ol_txrx_vdev_t *vdev);

/*
** Enumeration of PDEV Configuration parameter
*/
typedef enum _ol_ath_param_t {
	OL_ATH_PARAM_TXCHAINMASK               = 1,
	OL_ATH_PARAM_RXCHAINMASK               = 2,
	OL_ATH_PARAM_AMPDU                     = 6,
	OL_ATH_PARAM_AMPDU_LIMIT               = 7,
	OL_ATH_PARAM_AMPDU_SUBFRAMES           = 8,
	OL_ATH_PARAM_TXPOWER_LIMIT2G           = 12,
	OL_ATH_PARAM_TXPOWER_LIMIT5G           = 13,
	OL_ATH_PARAM_LDPC                      = 32,
	OL_ATH_PARAM_VOW_EXT_STATS             = 45,
	OL_ATH_PARAM_DYN_TX_CHAINMASK          = 73,
	OL_ATH_PARAM_BURST_ENABLE              = 77,
	OL_ATH_PARAM_BURST_DUR                 = 78,
	OL_ATH_PARAM_BCN_BURST                 = 80,
	OL_ATH_PARAM_DCS                       = 82,
#if UMAC_SUPPORT_PERIODIC_PERFSTATS
	OL_ATH_PARAM_PRDPERFSTAT_THRPUT_ENAB   = 83,
	OL_ATH_PARAM_PRDPERFSTAT_THRPUT_WIN    = 84,
	OL_ATH_PARAM_PRDPERFSTAT_THRPUT        = 85,
	OL_ATH_PARAM_PRDPERFSTAT_PER_ENAB      = 86,
	OL_ATH_PARAM_PRDPERFSTAT_PER_WIN       = 87,
	OL_ATH_PARAM_PRDPERFSTAT_PER           = 88,
#endif
	/* UMAC_SUPPORT_PERIODIC_PERFSTATS */
	OL_ATH_PARAM_TOTAL_PER                 = 89,
	 /*set manual rate for rts frame */
	OL_ATH_PARAM_RTS_CTS_RATE              = 92,
	 /** co channel interference threshold level */
	OL_ATH_PARAM_DCS_COCH_THR              = 93,
	 /** transmit error threshold */
	OL_ATH_PARAM_DCS_TXERR_THR             = 94,
	 /** phy error threshold */
	OL_ATH_PARAM_DCS_PHYERR_THR            = 95,
	/*  The IOCTL number is 114, it is made 114, inorder to make the IOCTL
	    number same as Direct-attach IOCTL.
	    Please, don't change number. This IOCTL gets the Interface code path
	    it should be either DIRECT-ATTACH or OFF-LOAD.
	*/
	OL_ATH_PARAM_GET_IF_ID                 = 114,
	/*Enable Acs back Ground Channel selection Scan timer in AP mode*/
	OL_ATH_PARAM_ACS_ENABLE_BK_SCANTIMEREN = 118,
	 /* ACS scan timer value in Seconds */
	OL_ATH_PARAM_ACS_SCANTIME              = 119,
	 /*Negligence Delta RSSI between two channel */
	OL_ATH_PARAM_ACS_RSSIVAR               = 120,
	 /*Negligence Delta Channel load between two channel*/
	OL_ATH_PARAM_ACS_CHLOADVAR             = 121,
	  /* Enable Limited OBSS check */
	OL_ATH_PARAM_ACS_LIMITEDOBSS           = 122,
	/* Acs control flag for Scan timer */
	OL_ATH_PARAM_ACS_CTRLFLAG              = 123,
	 /* Acs Run time Debug level*/
	OL_ATH_PARAM_ACS_DEBUGTRACE            = 124,
	OL_ATH_PARAM_SET_FW_HANG_ID            = 137,
	 /* Radio type 1:11ac 0:11abgn */
	OL_ATH_PARAM_RADIO_TYPE                = 138,
	OL_ATH_PARAM_IGMPMLD_OVERRIDE, /* IGMP/MLD packet override */
	OL_ATH_PARAM_IGMPMLD_TID, /* IGMP/MLD packet TID no */
	OL_ATH_PARAM_ARPDHCP_AC_OVERRIDE,
	OL_ATH_PARAM_NON_AGG_SW_RETRY_TH,
	OL_ATH_PARAM_AGG_SW_RETRY_TH,
	/* Dont change this number it as per sync with DA
	     Blocking certian channel from ic channel list */
	OL_ATH_PARAM_DISABLE_DFS   = 144,
	OL_ATH_PARAM_ENABLE_AMSDU  = 145,
	OL_ATH_PARAM_ENABLE_AMPDU  = 146,
	OL_ATH_PARAM_STA_KICKOUT_TH,
	OL_ATH_PARAM_WLAN_PROF_ENABLE,
	OL_ATH_PARAM_LTR_ENABLE,
	OL_ATH_PARAM_LTR_AC_LATENCY_BE = 150,
	OL_ATH_PARAM_LTR_AC_LATENCY_BK,
	OL_ATH_PARAM_LTR_AC_LATENCY_VI,
	OL_ATH_PARAM_LTR_AC_LATENCY_VO,
	OL_ATH_PARAM_LTR_AC_LATENCY_TIMEOUT,
	OL_ATH_PARAM_LTR_TX_ACTIVITY_TIMEOUT = 155,
	OL_ATH_PARAM_LTR_SLEEP_OVERRIDE,
	OL_ATH_PARAM_LTR_RX_OVERRIDE,
	OL_ATH_PARAM_L1SS_ENABLE,
	OL_ATH_PARAM_DSLEEP_ENABLE,
	/** radar error threshold */
	OL_ATH_PARAM_DCS_RADAR_ERR_THR = 160,
	/** Tx channel utilization due to AP's tx and rx */
	OL_ATH_PARAM_DCS_USERMAX_CU_THR,
	 /** interference detection threshold */
	OL_ATH_PARAM_DCS_INTR_DETECT_THR,
	/** sampling window, default 10secs */
	OL_ATH_PARAM_DCS_SAMPLE_WINDOW,
	/** debug logs enable/disable */
	OL_ATH_PARAM_DCS_DEBUG,
	OL_ATH_PARAM_ANI_ENABLE = 165,
	OL_ATH_PARAM_ANI_POLL_PERIOD,
	OL_ATH_PARAM_ANI_LISTEN_PERIOD,
	OL_ATH_PARAM_ANI_OFDM_LEVEL,
	OL_ATH_PARAM_ANI_CCK_LEVEL,
	OL_ATH_PARAM_DSCP_TID_MAP = 170,
	OL_ATH_PARAM_TXPOWER_SCALE,
	/** Phy error penalty */
	OL_ATH_PARAM_DCS_PHYERR_PENALTY,
#if ATH_SUPPORT_DSCP_OVERRIDE
	 /** set/get TID for sending HMMC packets */
	OL_ATH_PARAM_HMMC_DSCP_TID_MAP,
	/** set/get DSCP mapping override */
	OL_ATH_PARAM_DSCP_OVERRIDE,
	 /** set/get HMMC-DSCP mapping override */
	OL_ATH_PARAM_HMMC_DSCP_OVERRIDE = 175,
#endif
#if ATH_RX_LOOPLIMIT_TIMER
	OL_ATH_PARAM_LOOPLIMIT_NUM,
#endif
	OL_ATH_PARAM_ANTENNA_GAIN_2G,
	OL_ATH_PARAM_ANTENNA_GAIN_5G,
	OL_ATH_PARAM_RX_FILTER,
#if ATH_SUPPORT_HYFI_ENHANCEMENTS
	OL_ATH_PARAM_BUFF_THRESH = 180,
	OL_ATH_PARAM_BLK_REPORT_FLOOD,
	OL_ATH_PARAM_DROP_STA_QUERY,
#endif
	OL_ATH_PARAM_QBOOST,
	OL_ATH_PARAM_SIFS_FRMTYPE,
	OL_ATH_PARAM_SIFS_UAPSD = 185,
	OL_ATH_PARAM_FW_RECOVERY_ID,
	OL_ATH_PARAM_RESET_OL_STATS,
	OL_ATH_PARAM_AGGR_BURST,
	/* Number of deauth sent in consecutive rx_peer_invalid */
	OL_ATH_PARAM_DEAUTH_COUNT,
	OL_ATH_PARAM_BLOCK_INTERBSS = 190,
	 /* Firmware reset control for Bmiss / timeout / reset */
	OL_ATH_PARAM_FW_DISABLE_RESET,
	OL_ATH_PARAM_MSDU_TTL,
	OL_ATH_PARAM_PPDU_DURATION,
	OL_ATH_PARAM_SET_TXBF_SND_PERIOD,
	OL_ATH_PARAM_ALLOW_PROMISC = 195,
	OL_ATH_PARAM_BURST_MODE,
	OL_ATH_PARAM_DYN_GROUPING,
	OL_ATH_PARAM_DPD_ENABLE,
	OL_ATH_PARAM_DBGLOG_RATELIM,
	 /* firmware should intimate us about ps state change for node  */
	OL_ATH_PARAM_PS_STATE_CHANGE = 200,
	OL_ATH_PARAM_MCAST_BCAST_ECHO,
	 /* OBSS RSSI threshold for 20/40 coexistance */
	OL_ATH_PARAM_OBSS_RSSI_THRESHOLD,
	 /* Link/node RX RSSI threshold  for 20/40 coexistance */
	OL_ATH_PARAM_OBSS_RX_RSSI_THRESHOLD,
#if ATH_CHANNEL_BLOCKING
	OL_ATH_PARAM_ACS_BLOCK_MODE = 205,
#endif
	OL_ATH_PARAM_ACS_TX_POWER_OPTION,
	/* Default Antenna Polarization MSB 8 bits (24:31) specifying
	   enable/disable ; LSB 24 bits (0:23) antenna mask value */
	OL_ATH_PARAM_ANT_POLARIZATION,
	/* rate limit mute type error prints */
	OL_ATH_PARAM_PRINT_RATE_LIMIT,
	OL_ATH_PARAM_PDEV_RESET,   /* Reset FW PDEV*/
	/*Do not crash host when target assert happened*/
	OL_ATH_PARAM_FW_DUMP_NO_HOST_CRASH = 210,
	/*Consider OBSS non-erp to change to long slot*/
	OL_ATH_PARAM_CONSIDER_OBSS_NON_ERP_LONG_SLOT = 211,
#if PEER_FLOW_CONTROL
	OL_ATH_PARAM_STATS_FC,
	OL_ATH_PARAM_QFLUSHINTERVAL,
	OL_ATH_PARAM_TOTAL_Q_SIZE,
	OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE0,
	OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE1,
	OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE2,
	OL_ATH_PARAM_TOTAL_Q_SIZE_RANGE3,
	OL_ATH_PARAM_MIN_THRESHOLD,
	OL_ATH_PARAM_MAX_Q_LIMIT,
	OL_ATH_PARAM_MIN_Q_LIMIT,
	OL_ATH_PARAM_CONG_CTRL_TIMER_INTV,
	OL_ATH_PARAM_STATS_TIMER_INTV,
	OL_ATH_PARAM_ROTTING_TIMER_INTV,
	OL_ATH_PARAM_LATENCY_PROFILE,
	OL_ATH_PARAM_HOSTQ_DUMP,
	OL_ATH_PARAM_TIDQ_MAP,
#endif
	OL_ATH_PARAM_DBG_ARP_SRC_ADDR, /* ARP DEBUG source address*/
	OL_ATH_PARAM_DBG_ARP_DST_ADDR, /* ARP DEBUG destination address*/
	OL_ATH_PARAM_ARP_DBG_CONF,   /* ARP debug configuration */
	OL_ATH_PARAM_DISABLE_STA_VAP_AMSDU, /* Disable AMSDU for station vap */
#if ATH_SUPPORT_DFS && ATH_SUPPORT_STA_DFS
	OL_ATH_PARAM_STADFS_ENABLE = 300,    /* STA DFS is enabled or not  */
#endif
#if QCA_AIRTIME_FAIRNESS
	OL_ATH_PARAM_ATF_STRICT_SCHED = 301,
	OL_ATH_PARAM_ATF_GROUP_POLICY = 302,
#endif
#if DBDC_REPEATER_SUPPORT
	OL_ATH_PARAM_PRIMARY_RADIO,
	OL_ATH_PARAM_DBDC_ENABLE,
#endif
	OL_ATH_PARAM_TXPOWER_DBSCALE,
	OL_ATH_PARAM_CTL_POWER_SCALE,
#if QCA_AIRTIME_FAIRNESS
	OL_ATH_PARAM_ATF_OBSS_SCHED = 307,
	OL_ATH_PARAM_ATF_OBSS_SCALE = 308,
#endif
	OL_ATH_PARAM_PHY_OFDM_ERR = 309,
	OL_ATH_PARAM_PHY_CCK_ERR = 310,
	OL_ATH_PARAM_FCS_ERR = 311,
	OL_ATH_PARAM_CHAN_UTIL = 312,
#if DBDC_REPEATER_SUPPORT
	OL_ATH_PARAM_CLIENT_MCAST,
#endif
	OL_ATH_PARAM_EMIWAR_80P80 = 314,
	OL_ATH_PARAM_BATCHMODE = 315,
	OL_ATH_PARAM_PACK_AGGR_DELAY = 316,
#if UMAC_SUPPORT_ACFG
	OL_ATH_PARAM_DIAG_ENABLE = 317,
#endif
#if ATH_SUPPORT_VAP_QOS
	OL_ATH_PARAM_VAP_QOS = 318,
#endif
	OL_ATH_PARAM_CHAN_STATS_TH = 319,
	 /* Passive scan is enabled or disabled  */
	OL_ATH_PARAM_PASSIVE_SCAN_ENABLE = 320,
	OL_ATH_MIN_RSSI_ENABLE = 321,
	OL_ATH_MIN_RSSI = 322,
} ol_ath_param_t;

/*
** Enumeration of PDEV Configuration parameter
*/

typedef enum _ol_hal_param_t {
	OL_HAL_CONFIG_DMA_BEACON_RESPONSE_TIME         = 0
} ol_hal_param_t;

#endif
