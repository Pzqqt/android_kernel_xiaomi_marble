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

#ifndef _DP_HTT_H_
#define _DP_HTT_H_

#include <qdf_types.h>
#include <qdf_lock.h>
#include <qdf_nbuf.h>
#include <htc_api.h>

#define HTT_TX_MUTEX_TYPE qdf_spinlock_t

#define HTT_TX_MUTEX_INIT(_mutex)				\
	qdf_spinlock_create(_mutex)

#define HTT_TX_MUTEX_ACQUIRE(_mutex)			\
	qdf_spin_lock_bh(_mutex)

#define HTT_TX_MUTEX_RELEASE(_mutex)			\
	qdf_spin_unlock_bh(_mutex)

#define HTT_TX_MUTEX_DESTROY(_mutex)			\
	qdf_spinlock_destroy(_mutex)

#define DP_HTT_MAX_SEND_QUEUE_DEPTH 64

#ifndef HTT_MAC_ADDR_LEN
#define HTT_MAC_ADDR_LEN 6
#endif

struct dp_htt_htc_pkt {
	void *soc_ctxt;
	qdf_dma_addr_t nbuf_paddr;
	HTC_PACKET htc_pkt;
};

struct dp_htt_htc_pkt_union {
	union {
		struct dp_htt_htc_pkt pkt;
		struct dp_htt_htc_pkt_union *next;
	} u;
};

struct htt_soc {
	void *osif_soc;
	void *dp_soc;
	void *hal_soc;
	HTC_HANDLE htc_soc;
	qdf_device_t osdev;
	HTC_ENDPOINT_ID htc_endpoint;
	struct dp_htt_htc_pkt_union *htt_htc_pkt_freelist;
	struct {
		u_int8_t major;
		u_int8_t minor;
	} tgt_ver;
	struct {
		u_int8_t major;
		u_int8_t minor;
	} wifi_ip_ver;

	struct {
		int htc_err_cnt;
	} stats;

	HTT_TX_MUTEX_TYPE htt_tx_mutex;
};

void *
htt_soc_attach(void *txrx_soc, void *osif_soc, HTC_HANDLE htc_soc,
	void *hal_soc, qdf_device_t osdev);

void htt_soc_detach(void *soc);

int htt_srng_setup(void *htt_soc, int pdev_id, void *hal_srng,
	int hal_ring_type);

int htt_soc_attach_target(void *htt_soc);

#endif /* _DP_HTT_H_ */
