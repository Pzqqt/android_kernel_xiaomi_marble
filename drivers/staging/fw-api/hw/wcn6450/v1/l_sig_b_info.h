
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



#ifndef _L_SIG_B_INFO_H_
#define _L_SIG_B_INFO_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_L_SIG_B_INFO 1

struct l_sig_b_info {
             uint32_t rate                            :  4,
                      length                          : 12,
                      reserved                        : 16;
};

#define L_SIG_B_INFO_0_RATE_OFFSET                                   0x00000000
#define L_SIG_B_INFO_0_RATE_LSB                                      0
#define L_SIG_B_INFO_0_RATE_MASK                                     0x0000000f

#define L_SIG_B_INFO_0_LENGTH_OFFSET                                 0x00000000
#define L_SIG_B_INFO_0_LENGTH_LSB                                    4
#define L_SIG_B_INFO_0_LENGTH_MASK                                   0x0000fff0

#define L_SIG_B_INFO_0_RESERVED_OFFSET                               0x00000000
#define L_SIG_B_INFO_0_RESERVED_LSB                                  16
#define L_SIG_B_INFO_0_RESERVED_MASK                                 0xffff0000

#endif
