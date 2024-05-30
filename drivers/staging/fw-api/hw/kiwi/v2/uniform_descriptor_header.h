
/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

 

#ifndef _UNIFORM_DESCRIPTOR_HEADER_H_
#define _UNIFORM_DESCRIPTOR_HEADER_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_UNIFORM_DESCRIPTOR_HEADER 1

struct uniform_descriptor_header {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t owner                                                   :  4,  
                      buffer_type                                             :  4,  
                      reserved_0a                                             : 24;  
#else
             uint32_t reserved_0a                                             : 24,  
                      buffer_type                                             :  4,  
                      owner                                                   :  4;  
#endif
};

#define UNIFORM_DESCRIPTOR_HEADER_OWNER_OFFSET                                      0x00000000
#define UNIFORM_DESCRIPTOR_HEADER_OWNER_LSB                                         0
#define UNIFORM_DESCRIPTOR_HEADER_OWNER_MSB                                         3
#define UNIFORM_DESCRIPTOR_HEADER_OWNER_MASK                                        0x0000000f

#define UNIFORM_DESCRIPTOR_HEADER_BUFFER_TYPE_OFFSET                                0x00000000
#define UNIFORM_DESCRIPTOR_HEADER_BUFFER_TYPE_LSB                                   4
#define UNIFORM_DESCRIPTOR_HEADER_BUFFER_TYPE_MSB                                   7
#define UNIFORM_DESCRIPTOR_HEADER_BUFFER_TYPE_MASK                                  0x000000f0

#define UNIFORM_DESCRIPTOR_HEADER_RESERVED_0A_OFFSET                                0x00000000
#define UNIFORM_DESCRIPTOR_HEADER_RESERVED_0A_LSB                                   8
#define UNIFORM_DESCRIPTOR_HEADER_RESERVED_0A_MSB                                   31
#define UNIFORM_DESCRIPTOR_HEADER_RESERVED_0A_MASK                                  0xffffff00

#endif
