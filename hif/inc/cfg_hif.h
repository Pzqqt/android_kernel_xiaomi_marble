/**
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#ifndef _CFG_HIF_H_
#define _CFG_HIF_H_

/* Min/Max/default CE status srng timer threshold */
#define WLAN_CFG_CE_STATUS_RING_TIMER_THRESH_MIN 0
#define WLAN_CFG_CE_STATUS_RING_TIMER_THRESH_MAX 4096
#define WLAN_CFG_CE_STATUS_RING_TIMER_THRESH_DEFAULT 4096

/* Min/Max/default CE status srng batch count threshold */
#define WLAN_CFG_CE_STATUS_RING_BATCH_COUNT_THRESH_MIN 0
#define WLAN_CFG_CE_STATUS_RING_BATCH_COUNT_THRESH_MAX 512
#define WLAN_CFG_CE_STATUS_RING_BATCH_COUNT_THRESH_DEFAULT 1

#ifdef WLAN_CE_INTERRUPT_THRESHOLD_CONFIG
/**
 * <ini>
 * ce_status_ring_timer_thresh - ce status srng timer threshold
 * @Min: 0
 * @Max: 4096
 * @Default: 0
 *
 * This ini specifies the timer threshold for CE status srng to
 * indicate the interrupt to be fired whenever the timer threshold
 * runs out.
 *
 * Supported Feature: interrupt threshold for CE status srng
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_CE_STATUS_RING_TIMER_THRESHOLD \
	CFG_INI_UINT("ce_status_ring_timer_threshold", \
		     WLAN_CFG_CE_STATUS_RING_TIMER_THRESH_MIN, \
		     WLAN_CFG_CE_STATUS_RING_TIMER_THRESH_MAX, \
		     WLAN_CFG_CE_STATUS_RING_TIMER_THRESH_DEFAULT, \
		     CFG_VALUE_OR_DEFAULT, \
		     "CE Status ring timer threshold")

/**
 * <ini>
 * ce_status_ring_batch_count_thresh - ce status srng batch count threshold
 * @Min: 0
 * @Max: 512
 * @Default: 1
 *
 * This ini specifies the batch count threshold for CE status srng to
 * indicate the interrupt to be fired for a given number of packets in
 * the ring.
 *
 * Supported Feature: interrupt threshold for CE status srng
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_CE_STATUS_RING_BATCH_COUNT_THRESHOLD \
	CFG_INI_UINT("ce_status_ring_batch_count_threshold", \
		     WLAN_CFG_CE_STATUS_RING_BATCH_COUNT_THRESH_MIN, \
		     WLAN_CFG_CE_STATUS_RING_BATCH_COUNT_THRESH_MAX, \
		     WLAN_CFG_CE_STATUS_RING_BATCH_COUNT_THRESH_DEFAULT, \
		     CFG_VALUE_OR_DEFAULT, \
		     "CE Status ring batch count threshold")

#define CFG_HIF \
	CFG(CFG_CE_STATUS_RING_TIMER_THRESHOLD) \
	CFG(CFG_CE_STATUS_RING_BATCH_COUNT_THRESHOLD)
#else
#define CFG_HIF
#endif /* WLAN_CE_INTERRUPT_THRESHOLD_CONFIG */
#endif /* _CFG_HIF_H_ */
