
/* Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

 
 
 
 
 
 
 


#ifndef _MON_BUFFER_ADDR_H_
#define _MON_BUFFER_ADDR_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_MON_BUFFER_ADDR 4

#define NUM_OF_QWORDS_MON_BUFFER_ADDR 2


struct mon_buffer_addr {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t buffer_virt_addr_31_0                                   : 32;  
             uint32_t buffer_virt_addr_63_32                                  : 32;  
             uint32_t dma_length                                              : 12,  
                      reserved_2a                                             :  4,  
                      msdu_continuation                                       :  1,  
                      truncated                                               :  1,  
                      reserved_2b                                             : 14;  
             uint32_t tlv64_padding                                           : 32;  
#else
             uint32_t buffer_virt_addr_31_0                                   : 32;  
             uint32_t buffer_virt_addr_63_32                                  : 32;  
             uint32_t reserved_2b                                             : 14,  
                      truncated                                               :  1,  
                      msdu_continuation                                       :  1,  
                      reserved_2a                                             :  4,  
                      dma_length                                              : 12;  
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 

#define MON_BUFFER_ADDR_BUFFER_VIRT_ADDR_31_0_OFFSET                                0x0000000000000000
#define MON_BUFFER_ADDR_BUFFER_VIRT_ADDR_31_0_LSB                                   0
#define MON_BUFFER_ADDR_BUFFER_VIRT_ADDR_31_0_MSB                                   31
#define MON_BUFFER_ADDR_BUFFER_VIRT_ADDR_31_0_MASK                                  0x00000000ffffffff


 

#define MON_BUFFER_ADDR_BUFFER_VIRT_ADDR_63_32_OFFSET                               0x0000000000000000
#define MON_BUFFER_ADDR_BUFFER_VIRT_ADDR_63_32_LSB                                  32
#define MON_BUFFER_ADDR_BUFFER_VIRT_ADDR_63_32_MSB                                  63
#define MON_BUFFER_ADDR_BUFFER_VIRT_ADDR_63_32_MASK                                 0xffffffff00000000


 

#define MON_BUFFER_ADDR_DMA_LENGTH_OFFSET                                           0x0000000000000008
#define MON_BUFFER_ADDR_DMA_LENGTH_LSB                                              0
#define MON_BUFFER_ADDR_DMA_LENGTH_MSB                                              11
#define MON_BUFFER_ADDR_DMA_LENGTH_MASK                                             0x0000000000000fff


 

#define MON_BUFFER_ADDR_RESERVED_2A_OFFSET                                          0x0000000000000008
#define MON_BUFFER_ADDR_RESERVED_2A_LSB                                             12
#define MON_BUFFER_ADDR_RESERVED_2A_MSB                                             15
#define MON_BUFFER_ADDR_RESERVED_2A_MASK                                            0x000000000000f000


 

#define MON_BUFFER_ADDR_MSDU_CONTINUATION_OFFSET                                    0x0000000000000008
#define MON_BUFFER_ADDR_MSDU_CONTINUATION_LSB                                       16
#define MON_BUFFER_ADDR_MSDU_CONTINUATION_MSB                                       16
#define MON_BUFFER_ADDR_MSDU_CONTINUATION_MASK                                      0x0000000000010000


 

#define MON_BUFFER_ADDR_TRUNCATED_OFFSET                                            0x0000000000000008
#define MON_BUFFER_ADDR_TRUNCATED_LSB                                               17
#define MON_BUFFER_ADDR_TRUNCATED_MSB                                               17
#define MON_BUFFER_ADDR_TRUNCATED_MASK                                              0x0000000000020000


 

#define MON_BUFFER_ADDR_RESERVED_2B_OFFSET                                          0x0000000000000008
#define MON_BUFFER_ADDR_RESERVED_2B_LSB                                             18
#define MON_BUFFER_ADDR_RESERVED_2B_MSB                                             31
#define MON_BUFFER_ADDR_RESERVED_2B_MASK                                            0x00000000fffc0000


 

#define MON_BUFFER_ADDR_TLV64_PADDING_OFFSET                                        0x0000000000000008
#define MON_BUFFER_ADDR_TLV64_PADDING_LSB                                           32
#define MON_BUFFER_ADDR_TLV64_PADDING_MSB                                           63
#define MON_BUFFER_ADDR_TLV64_PADDING_MASK                                          0xffffffff00000000



#endif    
