/*
 * Copyright (c) 2011-2016,2018 The Linux Foundation. All rights reserved.
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
 * This file sir_types.h contains the common types
 *
 * Author:    V. K. Kandarpa
 * Date:      04/12/2002
 */

#ifndef __SIR_TYPES_H
#define __SIR_TYPES_H

#include <qdf_status.h>
#include <qdf_types.h>

/**
 * typedef mac_handle_t - MAC Handle
 *
 * Handle to the MAC.  The MAC handle is returned to the HDD from the
 * UMAC on Open.  The MAC handle is an input to all UMAC function
 * calls and represents an opaque handle to the UMAC instance that is
 * tied to the HDD instance
 *
 * The UMAC must be able to derive it's internal instance structure
 * pointer through this handle.
 */
/*
 * NOTE WELL: struct opaque_mac_handle is not defined anywhere. This
 * reference is used to help ensure that a mac_handle_t is never used
 * where a different handle type is expected
 */
struct opaque_mac_handle;
typedef struct opaque_mac_handle *mac_handle_t;

/* retain legacy name until all instances have been replaced */
typedef mac_handle_t tHalHandle;

/**
 * typedef hdd_handle_t - HDD Handle
 *
 * Handle to the HDD.  The HDD handle is given to the UMAC from the
 * HDD on Open.  The HDD handle is an input to all HDD/PAL function
 * calls and represents an opaque handle to the HDD instance that is
 * tied to the UMAC instance
 *
 * The HDD must be able to derive it's internal instance structure
 * pointer through this handle.
 */
/*
 * NOTE WELL: struct opaque_hdd_handle is not defined anywhere. This
 * reference is used to help ensure that a hdd_handle_t is never used
 * where a different handle type is expected
 */
struct opaque_hdd_handle;
typedef struct opaque_hdd_handle *hdd_handle_t;

#define HAL_NUM_ASSOC_STA           32
#define HAL_NUM_STA                 41

#define STACFG_MAX_TC               8

#endif /* __SIR_TYPES_H */
