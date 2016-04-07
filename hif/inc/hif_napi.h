/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
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

#ifndef __HIF_NAPI_H__
#define __HIF_NAPI_H__

/**
 * DOC: hif_napi.h
 *
 * Interface to HIF implemented functions of NAPI.
 * These are used by hdd_napi.
 */


/* CLD headers */
#include <hif.h> /* struct hif_opaque_softc; */

/**
 * common stuff
 * The declarations until #ifdef FEATURE_NAPI below
 * are valid whether or not FEATURE_NAPI has been
 * defined.
 */

/* the following triggers napi_enable/disable as required */
enum qca_napi_event {
	NAPI_EVT_INVALID,
	NAPI_EVT_INI_FILE,
	NAPI_EVT_CMD_STATE /* ioctl enable/disable commands */
};

/**
 * Macros to map ids -returned by ...create()- to pipes and vice versa
 */
#define NAPI_ID2PIPE(i) ((i)-1)
#define NAPI_PIPE2ID(p) ((p)+1)


#ifdef FEATURE_NAPI

/**
 * NAPI HIF API
 *
 * the declarations below only apply to the case
 * where FEATURE_NAPI is defined
 */

int hif_napi_create(struct hif_opaque_softc   *hif,
		    int (*poll)(struct napi_struct *, int),
		    int                budget,
		    int                scale);
int hif_napi_destroy(struct hif_opaque_softc  *hif,
		     uint8_t           id,
		     int               force);

struct qca_napi_data *hif_napi_get_all(struct hif_opaque_softc   *hif);

int hif_napi_event(struct hif_opaque_softc     *hif,
		   enum  qca_napi_event event,
		   void                *data);

/* called from the ISR within hif, so, ce is known */
int hif_napi_enabled(struct hif_opaque_softc *hif, int ce);

/* called from hdd (napi_poll), using napi id as a selector */
void hif_napi_enable_irq(struct hif_opaque_softc *hif, int id);

/* called by ce_tasklet.c::ce_dispatch_interrupt*/
int hif_napi_schedule(struct hif_opaque_softc *scn, int ce_id);

/* called by hdd_napi, which is called by kernel */
int hif_napi_poll(struct hif_opaque_softc *hif_ctx,
			struct napi_struct *napi, int budget);

#ifdef FEATURE_NAPI_DEBUG
#define NAPI_DEBUG(fmt, ...)			\
	qdf_print("wlan: NAPI: %s:%d "fmt, __func__, __LINE__, ##__VA_ARGS__);
#else
#define NAPI_DEBUG(fmt, ...) /* NO-OP */
#endif /* FEATURE NAPI_DEBUG */

#else /* ! defined(FEATURE_NAPI) */

/**
 * Stub API
 *
 * The declarations in this section are valid only
 * when FEATURE_NAPI has *not* been defined.
 */

#define NAPI_DEBUG(fmt, ...) /* NO-OP */

static inline int hif_napi_create(struct hif_opaque_softc   *hif,
				  uint8_t            pipe_id,
				  int (*poll)(struct napi_struct *, int),
				  int                budget,
				  int                scale)
{ return -EPERM; }

static inline int hif_napi_destroy(struct hif_opaque_softc  *hif,
				   uint8_t           id,
				   int               force)
{ return -EPERM; }

static inline struct qca_napi_data *hif_napi_get_all(
				struct hif_opaque_softc *hif)
{ return NULL; }

static inline int hif_napi_event(struct hif_opaque_softc     *hif,
				 enum  qca_napi_event event,
				 void                *data)
{ return -EPERM; }

/* called from the ISR within hif, so, ce is known */
static inline int hif_napi_enabled(struct hif_opaque_softc *hif, int ce)
{ return 0; }

/* called from hdd (napi_poll), using napi id as a selector */
static inline void hif_napi_enable_irq(struct hif_opaque_softc *hif, int id)
{ return; }

static inline int hif_napi_schedule(struct hif_opaque_softc *hif, int ce_id)
{ return 0; }

static inline int hif_napi_poll(struct napi_struct *napi, int budget)
{ return -EPERM; }

#endif /* FEATURE_NAPI */

#endif /* __HIF_NAPI_H__ */
