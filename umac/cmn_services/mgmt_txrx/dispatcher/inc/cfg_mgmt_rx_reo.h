/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 *  DOC: cfg_mgmt_rx_reo.h
 *  This file contains cfg definitions of mgmt rx reo sub-component
 */

#ifndef __CFG_MGMT_RX_REO_H
#define __CFG_MGMT_RX_REO_H

#ifdef WLAN_MGMT_RX_REO_SUPPORT

/*
 * <ini>
 * mgmt_rx_reo_enable - Enable MGMT Rx REO feature
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable MGMT Rx REO feature
 *
 * Related: None
 *
 * Supported Feature: MGMT Rx REO
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_MGMT_RX_REO_ENABLE \
	CFG_INI_BOOL("mgmt_rx_reo_enable", false, \
			"Enable MGMT Rx REO feature")

#define CFG_MGMT_RX_REO_ALL \
	CFG(CFG_MGMT_RX_REO_ENABLE)
#else

#define CFG_MGMT_RX_REO_ALL

#endif /* WLAN_MGMT_RX_REO_SUPPORT */
#endif /* __CFG_MGMT_RX_REO_H */
