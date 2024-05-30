
/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_handle.h
 * @brief Holds the forward structure declarations for handles
 * passed from the upper layers
 */

#ifndef CDP_TXRX_HANDLE_H
#define CDP_TXRX_HANDLE_H

struct cdp_cfg;
struct cdp_pdev;
struct cdp_vdev;
struct cdp_peer;
struct cdp_raw_ast;
struct cdp_soc;

/**
 * cdp_ctrl_objmgr_psoc - opaque handle for UMAC psoc object
 */
struct cdp_ctrl_objmgr_psoc;

/**
 * cdp_ctrl_objmgr_pdev - opaque handle for UMAC pdev object
 */
struct cdp_ctrl_objmgr_pdev;

/**
 * cdp_ctrl_objmgr_vdev - opaque handle for UMAC vdev object
 */
struct cdp_ctrl_objmgr_vdev;

/**
 * cdp_ctrl_objmgr_peer - opaque handle for UMAC peer object
 */
struct cdp_ctrl_objmgr_peer;

/**
 * cdp_cal_client - opaque handle for cal client object
 */
struct cdp_cal_client;

/**
 * cdp_ext_vdev - opaque handle for extended vdev data path handle
 */
struct cdp_ext_vdev;
#endif
