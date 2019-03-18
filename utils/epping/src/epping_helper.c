/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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

/*========================================================================

   \file  epping_main.c

   \brief WLAN End Point Ping test tool implementation

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <cds_api.h>
#include <cds_sched.h>
#include <linux/etherdevice.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <wni_api.h>
#include <wlan_ptt_sock_svc.h>
#include <linux/wireless.h>
#include <net/cfg80211.h>
#include <linux/rtnetlink.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/ctype.h>
#include "epping_main.h"
#include "epping_internal.h"

int epping_cookie_init(epping_context_t *pEpping_ctx)
{
	uint32_t i, j;

	pEpping_ctx->cookie_list = NULL;
	pEpping_ctx->cookie_count = 0;
	for (i = 0; i < MAX_COOKIE_SLOTS_NUM; i++) {
		pEpping_ctx->s_cookie_mem[i] =
			qdf_mem_malloc(sizeof(struct epping_cookie) *
				       MAX_COOKIE_SLOT_SIZE);
		if (!pEpping_ctx->s_cookie_mem[i])
			goto error;
	}
	qdf_spinlock_create(&pEpping_ctx->cookie_lock);

	for (i = 0; i < MAX_COOKIE_SLOTS_NUM; i++) {
		struct epping_cookie *cookie_mem = pEpping_ctx->s_cookie_mem[i];
		for (j = 0; j < MAX_COOKIE_SLOT_SIZE; j++) {
			epping_free_cookie(pEpping_ctx, &cookie_mem[j]);
		}
	}
	return 0;
error:
	for (i = 0; i < MAX_COOKIE_SLOTS_NUM; i++) {
		if (pEpping_ctx->s_cookie_mem[i]) {
			qdf_mem_free(pEpping_ctx->s_cookie_mem[i]);
			pEpping_ctx->s_cookie_mem[i] = NULL;
		}
	}
	return -ENOMEM;
}

/* cleanup cookie queue */
void epping_cookie_cleanup(epping_context_t *pEpping_ctx)
{
	int i;
	qdf_spin_lock_bh(&pEpping_ctx->cookie_lock);
	pEpping_ctx->cookie_list = NULL;
	pEpping_ctx->cookie_count = 0;
	qdf_spin_unlock_bh(&pEpping_ctx->cookie_lock);
	for (i = 0; i < MAX_COOKIE_SLOTS_NUM; i++) {
		if (pEpping_ctx->s_cookie_mem[i]) {
			qdf_mem_free(pEpping_ctx->s_cookie_mem[i]);
			pEpping_ctx->s_cookie_mem[i] = NULL;
		}
	}
}

void epping_free_cookie(epping_context_t *pEpping_ctx,
			struct epping_cookie *cookie)
{
	qdf_spin_lock_bh(&pEpping_ctx->cookie_lock);
	cookie->next = pEpping_ctx->cookie_list;
	pEpping_ctx->cookie_list = cookie;
	pEpping_ctx->cookie_count++;
	qdf_spin_unlock_bh(&pEpping_ctx->cookie_lock);
}

struct epping_cookie *epping_alloc_cookie(epping_context_t *pEpping_ctx)
{
	struct epping_cookie *cookie;

	qdf_spin_lock_bh(&pEpping_ctx->cookie_lock);
	cookie = pEpping_ctx->cookie_list;
	if (cookie) {
		pEpping_ctx->cookie_list = cookie->next;
		pEpping_ctx->cookie_count--;
	}
	qdf_spin_unlock_bh(&pEpping_ctx->cookie_lock);
	return cookie;
}

void epping_get_dummy_mac_addr(tSirMacAddr macAddr)
{
	macAddr[0] = 69;        /* E */
	macAddr[1] = 80;        /* P */
	macAddr[2] = 80;        /* P */
	macAddr[3] = 73;        /* I */
	macAddr[4] = 78;        /* N */
	macAddr[5] = 71;        /* G */
}

void epping_hex_dump(void *data, int buf_len, const char *str)
{
	char *buf = (char *)data;
	int i;

	printk("%s: E, %s\n", __func__, str);
	for (i = 0; (i + 7) < buf_len; i += 8) {
		printk("%02x %02x %02x %02x %02x %02x %02x %02x\n",
		       buf[i],
		       buf[i + 1],
		       buf[i + 2],
		       buf[i + 3],
		       buf[i + 4], buf[i + 5], buf[i + 6], buf[i + 7]);
	}

	/* Dump the bytes in the last line */
	for (; i < buf_len; i++) {
		printk("%02x ", buf[i]);
	}
	printk("\n%s: X %s\n", __func__, str);
}

void *epping_get_qdf_ctx(void)
{
	qdf_device_t *qdf_ctx;

	qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	return qdf_ctx;
}

void epping_log_packet(epping_adapter_t *adapter,
		       EPPING_HEADER *eppingHdr, int ret, const char *str)
{
	if (eppingHdr->Cmd_h & EPPING_LOG_MASK) {
		EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
			   "%s: cmd = %d, seqNo = %u, flag = 0x%x, ret = %d, "
			   "txCount = %lu, txDrop =  %lu, txBytes = %lu,"
			   "rxCount = %lu, rxDrop = %lu, rxBytes = %lu\n",
			   str, eppingHdr->Cmd_h, eppingHdr->SeqNo,
			   eppingHdr->CmdFlags_h, ret,
			   adapter->stats.tx_packets,
			   adapter->stats.tx_dropped,
			   adapter->stats.tx_bytes,
			   adapter->stats.rx_packets,
			   adapter->stats.rx_dropped,
			   adapter->stats.rx_bytes);
	}
}

void epping_log_stats(epping_adapter_t *adapter, const char *str)
{
	EPPING_LOG(QDF_TRACE_LEVEL_FATAL,
		   "%s: txCount = %lu, txDrop = %lu, tx_bytes = %lu, "
		   "rxCount = %lu, rxDrop = %lu, rx_bytes = %lu, tx_acks = %u\n",
		   str,
		   adapter->stats.tx_packets,
		   adapter->stats.tx_dropped,
		   adapter->stats.tx_bytes,
		   adapter->stats.rx_packets,
		   adapter->stats.rx_dropped,
		   adapter->stats.rx_bytes,
		   adapter->pEpping_ctx->total_tx_acks);
}

void epping_set_kperf_flag(epping_adapter_t *adapter,
			   HTC_ENDPOINT_ID eid, uint8_t kperf_flag)
{
	adapter->pEpping_ctx->kperf_num_rx_recv[eid] = 0;
	adapter->pEpping_ctx->kperf_num_tx_acks[eid] = 0;
}
