
/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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



#ifndef _PHYRX_COMMON_USER_INFO_H_
#define _PHYRX_COMMON_USER_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_PHYRX_COMMON_USER_INFO 1

struct phyrx_common_user_info {
             uint32_t receive_duration                : 16,
                      reserved_0a                     : 16;
};

#define PHYRX_COMMON_USER_INFO_0_RECEIVE_DURATION_OFFSET             0x00000000
#define PHYRX_COMMON_USER_INFO_0_RECEIVE_DURATION_LSB                0
#define PHYRX_COMMON_USER_INFO_0_RECEIVE_DURATION_MASK               0x0000ffff

#define PHYRX_COMMON_USER_INFO_0_RESERVED_0A_OFFSET                  0x00000000
#define PHYRX_COMMON_USER_INFO_0_RESERVED_0A_LSB                     16
#define PHYRX_COMMON_USER_INFO_0_RESERVED_0A_MASK                    0xffff0000

#endif
