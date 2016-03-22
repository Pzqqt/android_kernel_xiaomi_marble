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


qdf_nbuf_t
ol_tx_non_std(ol_txrx_vdev_handle vdev,
		 enum ol_tx_spec tx_spec, qdf_nbuf_t msdu_list);

#endif /* _CDP_TXRX_MISC_H_ */















