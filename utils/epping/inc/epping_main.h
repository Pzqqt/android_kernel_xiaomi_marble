/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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

#ifndef EPPING_MAIN_H
#define EPPING_MAIN_H
/**===========================================================================

   \file  epping_main.h

   \brief Linux epping head file

   ==========================================================================*/

/*---------------------------------------------------------------------------
   Include files
   -------------------------------------------------------------------------*/
#include <qdf_lock.h>
#include <qdf_types.h>

/* epping_main signatures */
#ifdef WLAN_FEATURE_EPPING
int epping_open(void);
void epping_close(void);
void epping_disable(void);
int epping_enable(struct device *parent_dev);
#else
static inline int epping_open(void)
{
	return QDF_STATUS_E_INVAL;
}

static inline int epping_enable(struct device *parent_dev)
{
	return QDF_STATUS_E_INVAL;
}

static inline void epping_close(void) {}
static inline void epping_disable(void) {}
#endif
#endif /* end #ifndef EPPING_MAIN_H */
