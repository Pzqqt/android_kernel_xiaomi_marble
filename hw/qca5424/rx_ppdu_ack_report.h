
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 
 
 
 
 
 
 


#ifndef _RX_PPDU_ACK_REPORT_H_
#define _RX_PPDU_ACK_REPORT_H_
#if !defined(__ASSEMBLER__)
#endif

#include "ack_report.h"
#define NUM_OF_DWORDS_RX_PPDU_ACK_REPORT 2

#define NUM_OF_QWORDS_RX_PPDU_ACK_REPORT 1


struct rx_ppdu_ack_report {
#ifndef WIFI_BIT_ORDER_BIG_ENDIAN
             struct   ack_report                                                ack_report_details;
             uint32_t tlv64_padding                                           : 32;  
#else
             struct   ack_report                                                ack_report_details;
             uint32_t tlv64_padding                                           : 32;  
#endif
};


 


 

#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_SELFGEN_RESPONSE_REASON_OFFSET        0x0000000000000000
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_SELFGEN_RESPONSE_REASON_LSB           0
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_SELFGEN_RESPONSE_REASON_MSB           3
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_SELFGEN_RESPONSE_REASON_MASK          0x000000000000000f


 

#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_AX_TRIGGER_TYPE_OFFSET                0x0000000000000000
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_AX_TRIGGER_TYPE_LSB                   4
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_AX_TRIGGER_TYPE_MSB                   7
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_AX_TRIGGER_TYPE_MASK                  0x00000000000000f0


 

#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_SR_PPDU_OFFSET                        0x0000000000000000
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_SR_PPDU_LSB                           8
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_SR_PPDU_MSB                           8
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_SR_PPDU_MASK                          0x0000000000000100


 

#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_RESERVED_OFFSET                       0x0000000000000000
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_RESERVED_LSB                          9
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_RESERVED_MSB                          15
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_RESERVED_MASK                         0x000000000000fe00


 

#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_FRAME_CONTROL_OFFSET                  0x0000000000000000
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_FRAME_CONTROL_LSB                     16
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_FRAME_CONTROL_MSB                     31
#define RX_PPDU_ACK_REPORT_ACK_REPORT_DETAILS_FRAME_CONTROL_MASK                    0x00000000ffff0000


 

#define RX_PPDU_ACK_REPORT_TLV64_PADDING_OFFSET                                     0x0000000000000000
#define RX_PPDU_ACK_REPORT_TLV64_PADDING_LSB                                        32
#define RX_PPDU_ACK_REPORT_TLV64_PADDING_MSB                                        63
#define RX_PPDU_ACK_REPORT_TLV64_PADDING_MASK                                       0xffffffff00000000



#endif    
