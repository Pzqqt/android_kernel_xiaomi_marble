
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

 
 
 
 
 
 
 


#ifndef _ACK_REPORT_H_
#define _ACK_REPORT_H_
#if !defined(__ASSEMBLER__)
#endif

#define NUM_OF_DWORDS_ACK_REPORT 1


struct ack_report {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             uint32_t selfgen_response_reason                                 :  4,  
                      ax_trigger_type                                         :  4,  
                      sr_ppdu                                                 :  1,  
                      reserved                                                :  7,  
                      frame_control                                           : 16;  
#else
             uint32_t frame_control                                           : 16,  
                      reserved                                                :  7,  
                      sr_ppdu                                                 :  1,  
                      ax_trigger_type                                         :  4,  
                      selfgen_response_reason                                 :  4;  
#endif
};


 

#define ACK_REPORT_SELFGEN_RESPONSE_REASON_OFFSET                                   0x00000000
#define ACK_REPORT_SELFGEN_RESPONSE_REASON_LSB                                      0
#define ACK_REPORT_SELFGEN_RESPONSE_REASON_MSB                                      3
#define ACK_REPORT_SELFGEN_RESPONSE_REASON_MASK                                     0x0000000f


 

#define ACK_REPORT_AX_TRIGGER_TYPE_OFFSET                                           0x00000000
#define ACK_REPORT_AX_TRIGGER_TYPE_LSB                                              4
#define ACK_REPORT_AX_TRIGGER_TYPE_MSB                                              7
#define ACK_REPORT_AX_TRIGGER_TYPE_MASK                                             0x000000f0


 

#define ACK_REPORT_SR_PPDU_OFFSET                                                   0x00000000
#define ACK_REPORT_SR_PPDU_LSB                                                      8
#define ACK_REPORT_SR_PPDU_MSB                                                      8
#define ACK_REPORT_SR_PPDU_MASK                                                     0x00000100


 

#define ACK_REPORT_RESERVED_OFFSET                                                  0x00000000
#define ACK_REPORT_RESERVED_LSB                                                     9
#define ACK_REPORT_RESERVED_MSB                                                     15
#define ACK_REPORT_RESERVED_MASK                                                    0x0000fe00


 

#define ACK_REPORT_FRAME_CONTROL_OFFSET                                             0x00000000
#define ACK_REPORT_FRAME_CONTROL_LSB                                                16
#define ACK_REPORT_FRAME_CONTROL_MSB                                                31
#define ACK_REPORT_FRAME_CONTROL_MASK                                               0xffff0000



#endif    
