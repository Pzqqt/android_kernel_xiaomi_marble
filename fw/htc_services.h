/*
 * Copyright (c) 2012, 2014-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef __HTC_SERVICES_H__
#define __HTC_SERVICES_H__

/* Current service IDs */

typedef enum {
    RSVD_SERVICE_GROUP       = 0,
    WMI_SERVICE_GROUP        = 1,
    NMI_SERVICE_GROUP        = 2,
    HTT_SERVICE_GROUP        = 3,
    CFG_NV_SERVICE_GROUP     = 4,
    WDI_IPA_SERVICE_GROUP    = 5,
    PACKET_LOG_SERVICE_GROUP = 6,

    HTC_TEST_GROUP           = 254,
    HTC_SERVICE_GROUP_LAST   = 255
}HTC_SERVICE_GROUP_IDS;

#define MAKE_SERVICE_ID(group,index) \
            (int)(((int)group << 8) | (int)(index))

/* NOTE: service ID of 0x0000 is reserved and should never be used */
#define HTC_CTRL_RSVD_SVC MAKE_SERVICE_ID(RSVD_SERVICE_GROUP,1)
/* WMI_CONTROL_SVC: WMI service for MAC 0 */
#define WMI_CONTROL_SVC        MAKE_SERVICE_ID(WMI_SERVICE_GROUP,0)
#define WMI_DATA_BE_SVC        MAKE_SERVICE_ID(WMI_SERVICE_GROUP,1)
#define WMI_DATA_BK_SVC        MAKE_SERVICE_ID(WMI_SERVICE_GROUP,2)
#define WMI_DATA_VI_SVC        MAKE_SERVICE_ID(WMI_SERVICE_GROUP,3)
#define WMI_DATA_VO_SVC        MAKE_SERVICE_ID(WMI_SERVICE_GROUP,4)
/* WMI_CONTROL_SVC_WMAC1,2: WMI service for MACs 1 and 2 (where applicable) */
#define WMI_CONTROL_SVC_WMAC1  MAKE_SERVICE_ID(WMI_SERVICE_GROUP,5)
#define WMI_CONTROL_SVC_WMAC2  MAKE_SERVICE_ID(WMI_SERVICE_GROUP,6)
#define WMI_MAX_SERVICES  7

#define NMI_CONTROL_SVC   MAKE_SERVICE_ID(NMI_SERVICE_GROUP,0)
#define NMI_DATA_SVC      MAKE_SERVICE_ID(NMI_SERVICE_GROUP,1)

/* HTT_DATA_MSG_SVC
 * This service is the regular (mandatory) HTT service
 * used for transferring data-path messages betwween host and target.
 */
#define HTT_DATA_MSG_SVC  MAKE_SERVICE_ID(HTT_SERVICE_GROUP,0)

/* HTT_DATA2_MSG_SVG
 * This is an optional service, used for downloading short tx frames
 * over a communication channel parallel to the regular HTT comm channel.
 * This can only be used for protocols where frame reordering is permitted (TCP).
 * This is helpful for minimizing latency and maximizing efficiency of TCP acks.
 * This service is also for sending Rx In-order indication packets over copy engine 9 for
 * for dual-band systems with multi-core hosts.
 */
#define HTT_DATA2_MSG_SVC MAKE_SERVICE_ID(HTT_SERVICE_GROUP,1)

/* This service is only for sending Rx In-order indication packets over copy engine 10
 * for dual-band systems with multi-core hosts, in which rx indications from one pdev/MAC
 * can be routed to one host processor core in parallel with rx indications from another
 * pdev/MAC being routed to another host core over another copy engine, such that these
 * rx indications can be processed in parallel.
 */
#define HTT_DATA3_MSG_SVC MAKE_SERVICE_ID(HTT_SERVICE_GROUP,2)

/* raw stream service (i.e. flash, tcmd, calibration apps) */
#define HTC_RAW_STREAMS_SVC MAKE_SERVICE_ID(HTC_TEST_GROUP,0)

#define CFG_NV_SVC  MAKE_SERVICE_ID(CFG_NV_SERVICE_GROUP,0)

#define WDI_IPA_TX_SVC MAKE_SERVICE_ID(WDI_IPA_SERVICE_GROUP,0)

#define PACKET_LOG_SVC MAKE_SERVICE_ID(PACKET_LOG_SERVICE_GROUP,0)

/*
 * Directions for interconnect pipe configuration.
 * These definitions may be used during configuration and are shared
 * between Host and Target.
 *
 * Pipe Directions are relative to the Host, so PIPEDIR_IN means
 * "coming IN over air through Target to Host" as with a WiFi Rx operation.
 * Conversely, PIPEDIR_OUT means "going OUT from Host through Target over air"
 * as with a WiFi Tx operation. This is somewhat awkward for the "middle-man"
 * Target since things that are "PIPEDIR_OUT" are coming IN to the Target
 * over the interconnect.
 */
typedef A_UINT32 PIPEDIR;
#define PIPEDIR_NONE      0
#define PIPEDIR_IN        1  /* Target-->Host, WiFi Rx direction */
#define PIPEDIR_OUT       2  /* Host->Target, WiFi Tx direction */
#define PIPEDIR_INOUT     3  /* bidirectional, target to target */
#define PIPEDIR_INOUT_T2T PIPEDIR_INOUT
#define PIPEDIR_INOUT_H2H 4  /* bidirectional, host to host */
#define PIPEDIR_MATCH(d1, d2) (((PIPEDIR)(d1) & (PIPEDIR)(d2)) != 0)
#define INVALID_PIPE_NO (-1)

/* Establish a mapping between a service/direction and a pipe. */
struct service_to_pipe {
    A_UINT32 service_id;
    A_UINT32 pipedir;
    A_UINT32 pipenum;
};


#endif /*HTC_SERVICES_H_*/
