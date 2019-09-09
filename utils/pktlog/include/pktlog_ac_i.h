/*
 * Copyright (c) 2012-2019 The Linux Foundation. All rights reserved.
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

#ifndef _PKTLOG_AC_I_
#define _PKTLOG_AC_I_

#ifndef REMOVE_PKT_LOG

#include <ol_txrx_internal.h>
#include <pktlog_ac.h>


#define PKTLOG_DEFAULT_BUFSIZE          (10 * 1024 * 1024) /* 10MB */
#define PKTLOG_DEFAULT_SACK_THR         3
#define PKTLOG_DEFAULT_TAIL_LENGTH      100
#define PKTLOG_DEFAULT_THRUPUT_THRESH   (64 * 1024)
#define PKTLOG_DEFAULT_PER_THRESH       30
#define PKTLOG_DEFAULT_PHYERR_THRESH    300
#define PKTLOG_DEFAULT_TRIGGER_INTERVAL 500

/* Max Pktlog buffer size received from fw/hw */
#define MAX_PKTLOG_RECV_BUF_SIZE        2048

struct ath_pktlog_arg {
	struct ath_pktlog_info *pl_info;
	uint32_t flags;
	uint16_t missed_cnt;
#ifdef HELIUMPLUS
	uint8_t log_type;
	uint8_t macId;
#else
	uint16_t log_type;
#endif
	size_t log_size;
	uint16_t timestamp;
#ifdef PKTLOG_HAS_SPECIFIC_DATA
	uint32_t type_specific_data;
#endif
	char *buf;
};

void pktlog_getbuf_intsafe(struct ath_pktlog_arg *plarg);
char *pktlog_getbuf(struct pktlog_dev_t *pl_dev,
		    struct ath_pktlog_info *pl_info,
		    size_t log_size, struct ath_pktlog_hdr *pl_hdr);

A_STATUS process_tx_info(struct cdp_pdev *pdev, void *data);
A_STATUS process_rx_info(void *pdev, void *data);
A_STATUS process_rx_info_remote(void *pdev, void *data);
A_STATUS process_rate_find(void *pdev, void *data);
A_STATUS process_rate_update(void *pdev, void *data);
A_STATUS process_sw_event(void *pdev, void *data);
int process_pktlog_lite(void *context, void *log_data, uint16_t log_type);
int process_rx_desc_remote(void *pdev, void *data);
A_STATUS process_offload_pktlog(struct cdp_pdev *pdev, void *data);
#endif /* REMOVE_PKT_LOG */
#endif
