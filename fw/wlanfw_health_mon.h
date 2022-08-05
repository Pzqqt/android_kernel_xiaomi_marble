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

#ifndef __WLANFW_HEALTH_MON_H__
#define __WLANFW_HEALTH_MON_H__

#include <a_types.h> /* A_UINT32 */

/* WLAN Health monitor data structure shared by host and FW */

/*
 * Version 1 of the upload metric structs (i.e. wlanfw_health_mon_metric_upload_t)
 * provides a fixed allocation of 8 bytes of scratch space after each metric,
 * to optionally hold metric-specific data, e.g. from intermediate calculations when computing the score_pct.
 */
#define WLANFW_HEALTH_MON_EXTRA_WORDS32 2

/* word0:
 * Extract bitfields from the A_UINT32 "word" containing them.
 * The target produces the data in little-endian order.
 * If the host uses big-endian order, it needs to account for the endianness
 * difference when reading the data.
 * Definition of bitfields within 32-bit word:
 *     bits 7:0   = module ID (M7...M0)
 *     bits 14:8  = module local ID (m6...m0)
 *     bits 31:15 = instance ID (I16...I0)
 *                                bits
 * |31                           15|14              8|7                 0|
 * +-------------------------------+-----------------+-------------------+
 * |         instance ID           | metric local ID |     module ID     |
 * |I16            ...           I0|m6     ...     m0|M7      ...      M0|
 *
 * Layout in memory:
 *                    bits
 *        |7  |6                        0|
 *        +---+--------------------------+
 * byte 0 |M7  M6  M5  M4  M3  M2  M1  M0|
 *        +---+--------------------------+
 * byte 1 |I0 |m6  m5  m4  m3  m2  m1  m0|
 *        +---+--------------------------+
 * byte 2 |I8  I7  I6  I5  I4  I3  I2  I1|
 *        +------------------------------+
 * byte 3 |I16 I15 I14 I13 I12 I11 I10 I9|
 */
#ifdef LITTLE_ENDIAN
  /* The little-endian version of the macro to extract the module id and metric id */
  #define WLANFW_HEALTH_MON_MODULE_ID_GET(word32) \
      (((word32) & 0x000000ff) >> 0)
  #define WLANFW_HEALTH_MON_METRIC_LOCAL_ID_GET(word32) \
      (((word32) & 0x00007f00) >> 8)
  #define WLANFW_HEALTH_MON_INSTANCE_ID_GET(word32) \
      (((word32) & 0xffff8000) >> 15)
#else
  /*
   * When read into a big-endian 32-bit word:
   *                                bits
   * |31          24|23|22          16|15               8|7               0|
   * +--------------+--+--------------+------------------+-----------------+
   * |M7          M0|I0|m6          m0|I8              I1|I16            I9|
   *
   */
  /*
   * big-endian macro def to extract module ID = M7:M0 = byte3
   */
  #define WLANFW_HEALTH_MON_MODULE_ID_GET(word32) \
      (((word32) >> 24) && 0xff)
  /*
   * big-endian macro def to extract metric local ID = m6:m0 = byte2 & 0x7f
   */
  #define WLANFW_HEALTH_MON_METRIC_LOCAL_ID_GET(word32) \
      (((word32) >> 16) && 0x7f)
  /*
   * big-endian macro def to extract instance ID = I16:I0 =
   *     (I16:I9 << 9) | (I8:I1 << 1) | I0
   *     (byte0 << 9) | (byte1 << 1) | (byte2 >> 7)
   */
  #define WLANFW_HEALTH_MON_INSTANCE_ID_GET(word32) \
      (((((word32) >> 0)  & 0xff) << 9) | /* I16:I9 */ \
       ((((word32) >> 8)  & 0xff) << 1) | /* I8:I1 */  \
       ((((word32) >> 23) & 0x01) << 0))  /* I0 */
#endif

/* word1:
 * Extract bitfields from the A_UINT32 "word" containing them.
 * The target produces the data in little-endian order.
 * If the host uses big-endian order, it needs to account for the endianness
 * difference when reading the data.
 * Definition of bitfields within 32-bit word:
 *     bits 7:0   = score_pct (S7...S0)
 *     bits 15:8  = alarm_threshold (A7...A0)
 *     bit  16    = old_alarm_state (P)
 *     bits 20:17 = num_extra_bytes (N3..N0)
 *     bit  21    = valid_data_flag (V)
 *     bits 31:22 = reserved (R9...R0)
 *                                bits
 * |31              22|21|20     17|16|15              8|7              0|
 * +------------------+--+---------+--+-----------------+----------------+
 * |     reserved     |V |num bytes|P | alarm threshold |    score_pct   |
 * |R9              R0|V |N3     N0|P |A7             A0|S7            S0|
 *
 * Layout in memory:
 *                    bits
 *        |7     6| 5 |4             2|1 0|
 *        +-------+---+---------------+---+
 * byte 0 |S7  S6  S5  S4  S3  S2  S1  S0 |
 *        +-------------------------------+
 * byte 1 |A7  A6  A5  A4  A3  A2  A1  A0 |
 *        +-------+---+---------------+---+
 * byte 2 |R1  R0 | V |N3  N2  N1  N0 | P |
 *        +-------------------------------+
 * byte 3 |R9  R8  R7  R6  R5  R4  R3  R2 |
 *        +-------------------------------+
 */
#ifdef LITTLE_ENDIAN
  #define WLANFW_HEALTH_MON_SCORE_PCT_GET(word32) \
      (((word32) & 0x000000ff) >> 0)
  #define WLANFW_HEALTH_MON_SCORE_ALARM_THRESHOLD_GET(word32) \
      (((word32) & 0x0000ff00) >> 8)
  #define WLANFW_HEALTH_MON_SCORE_OLD_ALARM_STATE_GET(word32) \
      (((word32) & 0x00010000) >> 16)
  #define WLANFW_HEALTH_MON_SCORE_NUM_EXTRA_BYTES_GET(word32) \
      (((word32) & 0x001e0000) >> 17)
  #define WLANFW_HEALTH_MON_SCORE_VALID_DATA_FLAG_GET(word32) \
      (((word32) & 0x00200000) >> 21)
#else
  /*
   * When read into a big-endian 32-bit word:
   *                                bits
   * |31            24|23          16|15 14|13|12    9|8 |7               0|
   * +----------------+--------------+-------------------+-----------------+
   * |S7            S0|A7          A0|R1 R0|V |N3   N0|P |R9             R2|
   */
  /*
   * big-endian macro def to extract score_pct = S7:S0 = byte3
   */
  #define WLANFW_HEALTH_MON_SCORE_PCT_GET(word32) \
      (((word32) >> 24) && 0xff)
  /*
   * big-endian macro def to extract alarm_threshold = A7:A0 = byte2
   */
  #define WLANFW_HEALTH_MON_SCORE_ALARM_THRESHOLD_GET(word32) \
      (((word32) >> 16) && 0xff)
  /*
   * big-endian macro def to extract old_alarm_state = P = byte1 & 0x1
   */
  #define WLANFW_HEALTH_MON_SCORE_OLD_ALARM_STATE_GET(word32) \
      (((word32) >> 8) && 0x01)
  /*
   * big-endian macro def to extract num_extra_bytes = N3:N0 = (byte1>>1) & 0xf
   */
  #define WLANFW_HEALTH_MON_SCORE_NUM_EXTRA_BYTES_GET(word32) \
      (((word32) >> 9) && 0x0f)
  /*
   * big-endian macro def to extract valid_data_flag = V = (byte1 >> 5) & 0x1
   */
  #define WLANFW_HEALTH_MON_SCORE_VALID_DATA_FLAG_GET(word32) \
      (((word32) >> 13) && 0x01)
#endif


typedef struct _wlanfw_health_mon_metric_upload {
    union {
        A_UINT32 metric_id;         /* Metric ID
                                     * Unique ID assigned to every metric registered
                                     */
        struct {
            A_UINT32 module_id:8,       /* module_id:
                                     * Unique ID assigned to the FW module that owns the metric.
                                     * Refer to WLAN_MODULE_ID enum.
                                     */
                metric_local_id:7, /* metric_local_id:
                                     * Unique ID (within the module) assigned to this type of metric by the module that owns the metric.
                                     */
                instance_id:17;    /* instance_id:
                                     * The ID of the owner of this particular instance of the metric.
                                     * E.g. for a per-pdev metric, this is the pdev_id, for a per-vdev metric this is the vdev_id, etc.
                                     */
        };
    };

    union {
        A_UINT32 word1;

        struct {
            A_UINT32 score_pct:8, /* range: 0 (bad) - 100 (perfect) */
            alarm_threshold:8, /* alarm_threshold:
                                * If the metric's score falls below this threshold, the characteristic measured by the metric is a concern.
                                * If the score is above this threshold the characteristic in question is behaving normally.
                                * Any metrics that are purely for measurement (i.e. not for fault detection) should have alarm_threshold = 0.
                                */
            old_alarm_state:1,  /* old_alarm_state:
                                * This flag indicates whether a score_pct below the alarm_threshold is new (old_alarm_state = 0) or ongoing (old_alarm_state = 1).
                                */
            num_extra_bytes:4,  /* num_extra_words32:
                                 * How many valid extra 4-byte words of metric-specific context follow this struct.
                                 */
            valid_data_flag:1,  /* valid_data_flag:
                                * Indication of whether this metric is in-use and containing valid data, or unallocated and invalid.
                                * Since the target may upload all metric objects, both those that are in use and those that are available but unused, the host must check this flag to see whether the metric object contains valid data.
                                */
            reserved:10;
        };
    };

    /* Extra scratch space for metric specific context data, e.g. the raw data used to compute the score_pct. */
    A_UINT32 extra_data[WLANFW_HEALTH_MON_EXTRA_WORDS32];
} wlanfw_health_mon_metric_upload_t;

typedef enum {
    WLANFW_HEALTH_MON_UPLOAD_FMT_INVALID = 0,

    /* V1: metrics use the wlanfw_health_mon_metric_upload_t format */
    WLANFW_HEALTH_MON_UPLOAD_FMT_V1 = 1,
} wlanfw_health_mon_upload_fmt;

typedef struct _wlanfw_health_mon_upload_ring {
    wlanfw_health_mon_upload_fmt version_number;    /* version_number:
                                                     * Specifies the format of the uploaded records.
                                                     * 0 - unused
                                                     * 1 - the upload records use the wlanfw_health_mon_metric_upload_t format.
                                                     *     (WLANFW_HEALTH_MON_UPLOAD_FMT_V1)
                                                     * All other values are reserved.
                                                     */
    /* specifies how large each element within the upload ring is */
    A_UINT32 ring_element_bytes;
    /* specifies how many spaces the ring contains */
    A_UINT32 num_ring_elements;
    /* specifies which ring element was last written by the target */
    A_UINT32 write_index;
} wlanfw_health_mon_upload_ring_t;

typedef struct _wlanfw_health_mon_upload_ring_elem_t {
    A_UINT32 timestamp;
    /* how much of the head portion of the ring element contains valid data */
    A_UINT32 num_valid_bytes;
} wlanfw_health_mon_upload_ring_elem_t;


#endif /* __WLANFW_HEALTH_MON_H__*/
