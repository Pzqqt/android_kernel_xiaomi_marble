/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

/**
 * DOC: Declare public API related to the ftm timesync called by north bound
 * HDD/OSIF/LIM
 */

#ifndef _FTM_TIME_SYNC_UCFG_API_H_
#define _FTM_TIME_SYNC_UCFG_API_H_

#include <qdf_status.h>
#include <qdf_types.h>
//#include "ftm_time_sync_public_struct.h"
#include "ftm_time_sync_objmgr.h"

#ifdef FEATURE_WLAN_TIME_SYNC_FTM

/**
 * ucfg_ftm_timesync_init() - FTM time sync component initialization.
 *
 * This function initializes the ftm time sync component and registers
 * the handlers which are invoked on vdev creation.
 *
 * Return: For successful registration - QDF_STATUS_SUCCESS,
 *	   else QDF_STATUS error codes.
 */
QDF_STATUS ucfg_ftm_timesync_init(void);

/**
 * ucfg_ftm_timesync_deinit() - FTM time sync component deinit.
 *
 * This function deinits ftm time sync component.
 *
 * Return: None
 */
void ucfg_ftm_timesync_deinit(void);

#else

static inline
QDF_STATUS ucfg_ftm_timesync_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void ucfg_ftm_timesync_deinit(void)
{
}

#endif /* FEATURE_WLAN_TIME_SYNC_FTM */
#endif /* _FTM_TIME_SYNC_UCFG_API_H_ */
