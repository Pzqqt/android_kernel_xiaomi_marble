/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef REMOVE_PKT_LOG
#include "ol_txrx_types.h"
#include "ol_htt_tx_api.h"
#include "ol_tx_desc.h"
#include "qdf_mem.h"
#include "htt.h"
#include "htt_internal.h"
#include "pktlog_ac_i.h"
#include "wma_api.h"
#include "wlan_logging_sock_svc.h"

#ifdef PKTLOG_HAS_SPECIFIC_DATA
void
pktlog_hdr_set_specific_data(struct ath_pktlog_hdr *log_hdr,
			     uint32_t type_specific_data)
{
	log_hdr->type_specific_data = type_specific_data;
}

uint32_t
pktlog_hdr_get_specific_data(struct ath_pktlog_hdr *log_hdr)
{
	return log_hdr->type_specific_data;
}

void
pktlog_arg_set_specific_data(struct ath_pktlog_arg *plarg,
			     uint32_t type_specific_data)
{
	plarg->type_specific_data = type_specific_data;
}

uint32_t
pktlog_arg_get_specific_data(struct ath_pktlog_arg *plarg)
{
	return plarg->type_specific_data;
}
#endif /* PKTLOG_HAS_SPECIFIC_DATA */

void pktlog_getbuf_intsafe(struct ath_pktlog_arg *plarg)
{
	struct ath_pktlog_buf *log_buf;
	int32_t buf_size;
	struct ath_pktlog_hdr *log_hdr;
	int32_t cur_wr_offset;
	char *log_ptr;
	struct ath_pktlog_info *pl_info;
	uint16_t log_type;
	size_t log_size;
	uint32_t flags;
#ifdef HELIUMPLUS
	uint8_t mac_id;
#endif

	if (!plarg) {
		qdf_nofl_info("Invalid parg in %s", __func__);
		return;
	}

	pl_info = plarg->pl_info;
#ifdef HELIUMPLUS
	mac_id = plarg->macId;
	log_type = plarg->log_type;
#else
	log_type = plarg->log_type;
#endif
	log_size = plarg->log_size;
	log_buf = pl_info->buf;
	flags = plarg->flags;

	if (!log_buf) {
		qdf_nofl_info("Invalid log_buf in %s", __func__);
		return;
	}


	buf_size = pl_info->buf_size;
	cur_wr_offset = log_buf->wr_offset;
	/* Move read offset to the next entry if there is a buffer overlap */
	if (log_buf->rd_offset >= 0) {
		if ((cur_wr_offset <= log_buf->rd_offset)
		    && (cur_wr_offset + sizeof(struct ath_pktlog_hdr)) >
		    log_buf->rd_offset) {
			PKTLOG_MOV_RD_IDX(log_buf->rd_offset, log_buf,
					  buf_size);
		}
	} else {
		log_buf->rd_offset = cur_wr_offset;
	}

	log_hdr = (struct ath_pktlog_hdr *)(log_buf->log_data + cur_wr_offset);

	log_hdr->flags = flags;
#ifdef HELIUMPLUS
	log_hdr->macId = mac_id;
	log_hdr->log_type = log_type;
#else
	log_hdr->log_type = log_type;
#endif
	log_hdr->size = (uint16_t) log_size;
	log_hdr->missed_cnt = plarg->missed_cnt;
	log_hdr->timestamp = plarg->timestamp;
	pktlog_hdr_set_specific_data(log_hdr,
				     pktlog_arg_get_specific_data(plarg));
	cur_wr_offset += sizeof(*log_hdr);

	if ((buf_size - cur_wr_offset) < log_size) {
		while ((cur_wr_offset <= log_buf->rd_offset)
		       && (log_buf->rd_offset < buf_size)) {
			PKTLOG_MOV_RD_IDX(log_buf->rd_offset, log_buf,
					  buf_size);
		}
		cur_wr_offset = 0;
	}

	while ((cur_wr_offset <= log_buf->rd_offset)
	       && (cur_wr_offset + log_size) > log_buf->rd_offset) {
		PKTLOG_MOV_RD_IDX(log_buf->rd_offset, log_buf, buf_size);
	}

	log_ptr = &(log_buf->log_data[cur_wr_offset]);
	cur_wr_offset += log_hdr->size;

	log_buf->wr_offset = ((buf_size - cur_wr_offset) >=
			      sizeof(struct ath_pktlog_hdr)) ? cur_wr_offset :
			     0;

	plarg->buf = log_ptr;
}

char *pktlog_getbuf(struct pktlog_dev_t *pl_dev,
		    struct ath_pktlog_info *pl_info,
		    size_t log_size, struct ath_pktlog_hdr *pl_hdr)
{
	struct ath_pktlog_arg plarg = { 0, };
	uint8_t flags = 0;

	plarg.pl_info = pl_info;
#ifdef HELIUMPLUS
	plarg.macId = pl_hdr->macId;
	plarg.log_type = pl_hdr->log_type;
#else
	plarg.log_type = pl_hdr->log_type;
#endif
	plarg.log_size = log_size;
	plarg.flags = pl_hdr->flags;
	plarg.missed_cnt = pl_hdr->missed_cnt;
	plarg.timestamp = pl_hdr->timestamp;
	pktlog_arg_set_specific_data(&plarg,
				     pktlog_hdr_get_specific_data(pl_hdr));

	if (flags & PHFLAGS_INTERRUPT_CONTEXT) {
		/*
		 * We are already in interrupt context, no need to make it
		 * intsafe. call the function directly.
		 */
		pktlog_getbuf_intsafe(&plarg);
	} else {
		PKTLOG_LOCK(pl_info);
		pktlog_getbuf_intsafe(&plarg);
		PKTLOG_UNLOCK(pl_info);
	}

	return plarg.buf;
}
#endif /*REMOVE_PKT_LOG */
