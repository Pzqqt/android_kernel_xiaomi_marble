/*
 * Copyright (c) 2012-2014, 2016-2018, 2020 The Linux Foundation.
 * All rights reserved.
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
 *  The file is used to define structures that are shared between
 *  kernel space and user space pktlog application.
 */

#ifndef _PKTLOG_AC_API_
#define _PKTLOG_AC_API_
#ifndef REMOVE_PKT_LOG

/**
 * @typedef ol_pktlog_dev_handle
 * @brief opaque handle for pktlog device object
 */
struct ol_pktlog_dev_t;

/**
 * @typedef hif_opaque_softc_handle
 * @brief opaque handle for hif_opaque_softc
 */
struct hif_opaque_softc;
typedef struct hif_opaque_softc *hif_opaque_softc_handle;

enum pktlog_callback_regtype {
	PKTLOG_DEFAULT_CALLBACK_REGISTRATION,
	PKTLOG_LITE_CALLBACK_REGISTRATION
};

/**
 * @typedef net_device_handle
 * @brief opaque handle linux phy device object
 */
struct net_device;
typedef struct net_device *net_device_handle;

struct pktlog_dev_t;

void pktlog_sethandle(struct pktlog_dev_t **pl_handle,
		     hif_opaque_softc_handle scn);
void pktlog_set_pdev_id(struct pktlog_dev_t *pl_dev, uint8_t pdev_id);

void *get_txrx_context(void);

struct pktlog_dev_t *get_pktlog_handle(void);
void pktlog_set_callback_regtype(enum pktlog_callback_regtype callback_type);

/* Packet log state information */
#ifndef _PKTLOG_INFO
#define _PKTLOG_INFO

/**
 * enum ath_pktlog_state - pktlog status
 * @PKTLOG_OPR_IN_PROGRESS : pktlog command in progress
 * @PKTLOG_OPR_IN_PROGRESS_READ_START: pktlog read is issued
 * @PKTLOG_OPR_IN_PROGRESS_READ_START_PKTLOG_DISABLED:
 *			as part of pktlog read, pktlog is disabled
 * @PKTLOG_OPR_IN_PROGRESS_READ_COMPLETE:
 *		as part of read, till pktlog read is complete
 * @PKTLOG_OPR_IN_PROGRESS_CLEARBUFF_COMPLETE:
 *		as part of read, pktlog clear buffer is done
 * @PKTLOG_OPR_NOT_IN_PROGRESS: no pktlog command in progress
 */
enum ath_pktlog_state {
	PKTLOG_OPR_IN_PROGRESS = 0,
	PKTLOG_OPR_IN_PROGRESS_READ_START,
	PKTLOG_OPR_IN_PROGRESS_READ_START_PKTLOG_DISABLED,
	PKTLOG_OPR_IN_PROGRESS_READ_COMPLETE,
	PKTLOG_OPR_IN_PROGRESS_CLEARBUFF_COMPLETE,
	PKTLOG_OPR_NOT_IN_PROGRESS
};

struct ath_pktlog_info {
	struct ath_pktlog_buf *buf;
	uint32_t log_state;
	uint32_t saved_state;
	uint32_t options;
	/* Initial saved state: It will save the log state in pktlog
	 * open and used in pktlog release after
	 * pktlog read is complete.
	 */
	uint32_t init_saved_state;
	enum ath_pktlog_state curr_pkt_state;

	/* Size of buffer in bytes */
	int32_t buf_size;
	qdf_spinlock_t log_lock;
	struct mutex pktlog_mutex;

	/* Threshold of TCP SACK packets for triggered stop */
	int sack_thr;

	/* # of tail packets to log after triggered stop */
	int tail_length;

	/* throuput threshold in bytes for triggered stop */
	uint32_t thruput_thresh;

	/* (aggregated or single) packet size in bytes */
	uint32_t pktlen;

	/* a temporary variable for counting TX throughput only */
	/* PER threshold for triggered stop, 10 for 10%, range [1, 99] */
	uint32_t per_thresh;

	/* Phyerr threshold for triggered stop */
	uint32_t phyerr_thresh;

	/* time period for counting trigger parameters, in milisecond */
	uint32_t trigger_interval;
	uint32_t start_time_thruput;
	uint32_t start_time_per;
};
#endif /* _PKTLOG_INFO */
#else                           /* REMOVE_PKT_LOG */
typedef void *pktlog_dev_handle;
#define pktlog_set_pdev_id(pl_dev, pdev_id)	\
	do {					\
		(void)pl_dev;			\
		(void)pdev_id;			\
	} while (0)

#define pktlog_sethandle(pl_handle, scn)	\
	do {				\
		(void)pl_handle;	\
		(void)scn;		\
	} while (0)

#define ol_pl_set_name(dev)	\
	do {				\
		(void)scn;		\
		(void)dev;		\
	} while (0)

#endif /* REMOVE_PKT_LOG */
#endif /* _PKTLOG_AC_API_ */
