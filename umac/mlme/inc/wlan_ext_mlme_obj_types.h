/*
 * Copyright (c) 2019,2021 The Linux Foundation. All rights reserved.
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

/**
 * DOC: wlan_ext_mlme_obj_types.h
 *
 * This header file maintain structure definitions for mlme ptr specific to ic
 */

#ifndef __WLAN_EXT_MLME_OBJ_TYPE_H__
#define __WLAN_EXT_MLME_OBJ_TYPE_H__

/**
 * typedef mlme_pdev_ext_t - Opaque definition of pdev mlme pointer
 */
struct ieee80211com;
typedef struct ieee80211com mlme_pdev_ext_t;

/**
 * typedef mlme_vdev_ext_t - Opaque definition of vdev mlme pointer
 */
struct ieee80211vap;
typedef struct ieee80211vap mlme_vdev_ext_t;

/**
 * typedef mlme_psoc_ext_t - Opaque definition of psoc mlme pointer
 */
struct soc_ext_ptr;
typedef struct soc_ext_ptr mlme_psoc_ext_t;

/**
 * typedef cm_ext_t - Opaque Definition of connection manager ext pointer
 */
struct cm_ext_obj;
typedef struct cm_ext_obj cm_ext_t;
#endif /* __WLAN_EXT_MLME_OBJ_TYPE_H__ */
