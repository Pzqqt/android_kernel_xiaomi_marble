/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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


#ifndef _MON_INGRESS_RING_H_
#define _MON_INGRESS_RING_H_

#include "buffer_addr_info.h"
#define NUM_OF_DWORDS_MON_INGRESS_RING 4

struct mon_ingress_ring {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   buffer_addr_info                                          buffer_addr_info_details;
             uint32_t buffer_virt_addr_31_0                                   : 32;
             uint32_t buffer_virt_addr_63_32                                  : 32;
#else
             struct   buffer_addr_info                                          buffer_addr_info_details;
             uint32_t buffer_virt_addr_31_0                                   : 32;
             uint32_t buffer_virt_addr_63_32                                  : 32;
#endif
};

#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_OFFSET           0x00000000
#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_LSB              0
#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MSB              31
#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_31_0_MASK             0xffffffff

#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_OFFSET          0x00000004
#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_LSB             0
#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MSB             7
#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_BUFFER_ADDR_39_32_MASK            0x000000ff

#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_OFFSET      0x00000004
#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_LSB         8
#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MSB         11
#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_RETURN_BUFFER_MANAGER_MASK        0x00000f00

#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_OFFSET           0x00000004
#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_LSB              12
#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MSB              31
#define MON_INGRESS_RING_BUFFER_ADDR_INFO_DETAILS_SW_BUFFER_COOKIE_MASK             0xfffff000

#define MON_INGRESS_RING_BUFFER_VIRT_ADDR_31_0_OFFSET                               0x00000008
#define MON_INGRESS_RING_BUFFER_VIRT_ADDR_31_0_LSB                                  0
#define MON_INGRESS_RING_BUFFER_VIRT_ADDR_31_0_MSB                                  31
#define MON_INGRESS_RING_BUFFER_VIRT_ADDR_31_0_MASK                                 0xffffffff

#define MON_INGRESS_RING_BUFFER_VIRT_ADDR_63_32_OFFSET                              0x0000000c
#define MON_INGRESS_RING_BUFFER_VIRT_ADDR_63_32_LSB                                 0
#define MON_INGRESS_RING_BUFFER_VIRT_ADDR_63_32_MSB                                 31
#define MON_INGRESS_RING_BUFFER_VIRT_ADDR_63_32_MASK                                0xffffffff

#endif
