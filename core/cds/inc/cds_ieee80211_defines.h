/*
 * Copyright (c) 2011, 2014-2015, 2018-2019 The Linux Foundation. All rights reserved.
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

#ifndef CDS_IEEE80211_DEFINES_H_
#define CDS_IEEE80211_DEFINES_H_

#include "cds_ieee80211_common.h"

/*
 * Public defines for Atheros Upper MAC Layer
 */

#ifndef EXTERNAL_USE_ONLY

#define IEEE80211_LSIG_LEN  3
#define IEEE80211_HTSIG_LEN 6
#define IEEE80211_SB_LEN    2

typedef struct ieee80211_rx_status {
	int rs_numchains;
	int rs_flags;
#define IEEE80211_RX_FCS_ERROR      0x01
#define IEEE80211_RX_MIC_ERROR      0x02
#define IEEE80211_RX_DECRYPT_ERROR  0x04
/* holes in flags here between, ATH_RX_XXXX to IEEE80211_RX_XXX */
#define IEEE80211_RX_KEYMISS        0x200
	int rs_rssi;            /* RSSI (noise floor ajusted) */
	int rs_abs_rssi;        /* absolute RSSI */
	int rs_datarate;        /* data rate received */
	int rs_rateieee;
	int rs_ratephy;

#define IEEE80211_MAX_ANTENNA       3   /* Keep the same as ATH_MAX_ANTENNA */
	uint8_t rs_rssictl[IEEE80211_MAX_ANTENNA];      /* RSSI (noise floor ajusted) */
	uint8_t rs_rssiextn[IEEE80211_MAX_ANTENNA];     /* RSSI (noise floor ajusted) */
	uint8_t rs_isvalidrssi; /* rs_rssi is valid or not */

	enum ieee80211_phymode rs_phymode;
	int rs_freq;

	union {
		uint8_t data[8];
		uint64_t tsf;
	} rs_tstamp;

	/*
	 * Detail channel structure of recv frame.
	 * It could be NULL if not available
	 */
	struct ieee80211_channel *rs_full_chan;

	uint8_t rs_isaggr;
	uint8_t rs_isapsd;
	int16_t rs_noisefloor;
	uint16_t rs_channel;
#ifdef ATH_SUPPORT_TxBF
	uint32_t rs_rpttstamp;  /* txbf report time stamp */
#endif

	/* The following counts are meant to assist in stats calculation.
	   These variables are incremented only in specific situations, and
	   should not be relied upon for any purpose other than the original
	   stats related purpose they have been introduced for. */

	uint16_t rs_cryptodecapcount;   /* Crypto bytes decapped/demic'ed. */
	uint8_t rs_padspace;    /* No. of padding bytes present after header
	                           in wbuf. */
	uint8_t rs_qosdecapcount;       /* QoS/HTC bytes decapped. */

	/* End of stats calculation related counts. */

	uint8_t rs_lsig[IEEE80211_LSIG_LEN];
	uint8_t rs_htsig[IEEE80211_HTSIG_LEN];
	uint8_t rs_servicebytes[IEEE80211_SB_LEN];

} ieee80211_recv_status;
#endif /* EXTERNAL_USE_ONLY */

#endif
