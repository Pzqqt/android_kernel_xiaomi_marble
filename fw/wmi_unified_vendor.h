/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

/** @file
 *  This File provides dummy defs for the vendor-specific message content structs defined in wmi_unified.h.
 *  Vendor may replace this file wmi_unified_vendor.h with there own vendor defs header file containing the real vendor defs.
 **/
#ifndef _WMI_UNIFIED_VENDOR_H_
#define _WMI_UNIFIED_VENDOR_H_

typedef union
{
    A_UINT32 pdev_sample1_cmd;
    A_UINT32 pdev_sample2_cmd;
} wmi_pdev_vendor_cmd_val;

typedef union
{
    A_UINT32 pdev_sample1_event;
    A_UINT32 pdev_sample2_event;
} wmi_pdev_vendor_event_val;

typedef union
{
    A_UINT32 vdev_sample1_cmd;
    A_UINT32 vdev_sample2_cmd;
} wmi_vdev_vendor_cmd_val;

typedef union
{
    A_UINT32 vdev_sample1_event;
    A_UINT32 vdev_sample2_event;
} wmi_vdev_vendor_event_val;

typedef union
{
    A_UINT32 peer_sample1_cmd;
    A_UINT32 peer_sample2_cmd;
} wmi_peer_vendor_cmd_val;

typedef union
{
    A_UINT32 peer_sample1_event;
    A_UINT32 peer_sample2_event;
} wmi_peer_vendor_event_val;

#endif /* _WMI_UNIFIED_VENDOR_H_ */
