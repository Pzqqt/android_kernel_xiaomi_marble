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
 * @file cdp_txrx_misc.h
 * @brief Define the host data path miscelleneous API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_MISC_H_
#define _CDP_TXRX_MISC_H_

/**
 * @enum ol_tx_spec
 * @brief indicate what non-standard transmission actions to apply
 * @details
 *  Indicate one or more of the following:
 *    - The tx frame already has a complete 802.11 header.
 *      Thus, skip 802.3/native-WiFi to 802.11 header encapsulation and
 *      A-MSDU aggregation.
 *    - The tx frame should not be aggregated (A-MPDU or A-MSDU)
 *    - The tx frame is already encrypted - don't attempt encryption.
 *    - The tx frame is a segment of a TCP jumbo frame.
 *    - This tx frame should not be unmapped and freed by the txrx layer
 *      after transmission, but instead given to a registered tx completion
 *      callback.
 *  More than one of these specification can apply, though typically
 *  only a single specification is applied to a tx frame.
 *  A compound specification can be created, as a bit-OR of these
 *  specifications.
 */
enum ol_tx_spec {
	OL_TX_SPEC_STD = 0x0,   /* do regular processing */
	OL_TX_SPEC_RAW = 0x1,   /* skip encap + A-MSDU aggr */
	OL_TX_SPEC_NO_AGGR = 0x2,       /* skip encap + all aggr */
	OL_TX_SPEC_NO_ENCRYPT = 0x4,    /* skip encap + encrypt */
	OL_TX_SPEC_TSO = 0x8,   /* TCP segmented */
	OL_TX_SPEC_NWIFI_NO_ENCRYPT = 0x10,     /* skip encrypt for nwifi */
	OL_TX_SPEC_NO_FREE = 0x20,      /* give to cb rather than free */
};

/**
 * ol_tx_non_std() - Allow the control-path SW to send data frames
 *
 * @data_vdev - which vdev should transmit the tx data frames
 * @tx_spec - what non-standard handling to apply to the tx data frames
 * @msdu_list - NULL-terminated list of tx MSDUs
 *
 * Generally, all tx data frames come from the OS shim into the txrx layer.
 * However, there are rare cases such as TDLS messaging where the UMAC
 * control-path SW creates tx data frames.
 *  This UMAC SW can call this function to provide the tx data frames to
 *  the txrx layer.
 *  The UMAC SW can request a callback for these data frames after their
 *  transmission completes, by using the ol_txrx_data_tx_cb_set function
 *  to register a tx completion callback, and by specifying
 *  ol_tx_spec_no_free as the tx_spec arg when giving the frames to
 *  ol_tx_non_std.
 *  The MSDUs need to have the appropriate L2 header type (802.3 vs. 802.11),
 *  as specified by ol_cfg_frame_type().
 *
 *  Return: null - success, skb - failure
 */
qdf_nbuf_t ol_tx_non_std(ol_txrx_vdev_handle vdev,
			 enum ol_tx_spec tx_spec, qdf_nbuf_t msdu_list);

/**
 * ol_txrx_update_ibss_vdev_heart_beat_timer_of_vdev() - Update ibss vdev heart
 * beat timer
 * @vdev: vdev handle
 * @timer_value_sec: new heart beat timer value
 *
 * Return: Old timer value set in vdev.
 */
uint16_t ol_txrx_set_ibss_vdev_heart_beat_timer(ol_txrx_vdev_handle vdev,
						uint16_t timer_value_sec);
#endif /* _CDP_TXRX_MISC_H_ */















