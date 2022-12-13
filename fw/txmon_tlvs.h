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

#ifndef _TXMON_TLVS_H_
#define _TXMON_TLVS_H_

#define TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ1_M   0x0000ffff
#define TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ1_S   0

#define TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ2_M   0xffff0000
#define TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ2_S   16

#define TXMON_FW2SW_MON_FES_SETUP_PHY_MODE_M            0x000000ff
#define TXMON_FW2SW_MON_FES_SETUP_PHY_MODE_S            0

#define TXMON_FW2SW_MON_FES_SETUP_MHZ_M                 0x00ffff00
#define TXMON_FW2SW_MON_FES_SETUP_MHZ_S                 8

#define TXMON_FW2SW_MON_FES_SETUP_SCHEDULE_ID_M         0xffffffff
#define TXMON_FW2SW_MON_FES_SETUP_SCHEDULE_ID_S         0

#define TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ1_GET(_var) \
    (((_var) & TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ1_M) >> \
     TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ1_S)

#define TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ1_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ1, _val); \
        ((_var) |= ((_val) << TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ1_S)); \
    } while (0)

#define TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ2_GET(_var) \
    (((_var) & TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ2_M) >> \
     TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ2_S)

#define TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ2_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ2, _val); \
        ((_var) |= ((_val) << TXMON_FW2SW_MON_FES_SETUP_BAND_CENTER_FREQ2_S)); \
    } while (0)

#define TXMON_FW2SW_MON_FES_SETUP_PHY_MODE_GET(_var) \
    (((_var) & TXMON_FW2SW_MON_FES_SETUP_PHY_MODE_M) >> \
     TXMON_FW2SW_MON_FES_SETUP_PHY_MODE_S)

#define TXMON_FW2SW_MON_FES_SETUP_PHY_MODE_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(TXMON_FW2SW_MON_FES_SETUP_PHY_MODE, _val); \
        ((_var) |= ((_val) << TXMON_FW2SW_MON_FES_SETUP_PHY_MODE_S)); \
    } while (0)

#define TXMON_FW2SW_MON_FES_SETUP_MHZ_GET(_var) \
    (((_var) & TXMON_FW2SW_MON_FES_SETUP_MHZ_M) >> \
     TXMON_FW2SW_MON_FES_SETUP_MHZ_S)

#define TXMON_FW2SW_MON_FES_SETUP_MHZ_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(TXMON_FW2SW_MON_FES_SETUP_MHZ, _val); \
        ((_var) |= ((_val) << TXMON_FW2SW_MON_FES_SETUP_MHZ_S)); \
    } while (0)

#define TXMON_FW2SW_MON_FES_SETUP_SCHEDULE_ID_GET(_var) \
    (((_var) & TXMON_FW2SW_MON_FES_SETUP_SCHEDULE_ID_M) >> \
     TXMON_FW2SW_MON_FES_SETUP_SCHEDULE_ID_S)

#define TXMON_FW2SW_MON_FES_SETUP_SCHEDULE_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(TXMON_FW2SW_MON_FES_SETUP_SCHEDULE_ID, _val); \
        ((_var) |= ((_val) << TXMON_FW2SW_MON_FES_SETUP_SCHEDULE_ID_S)); \
    } while (0)

enum txmon_fw2sw_user_id {
    TXMON_FW2SW_TYPE_FES_SETUP      = 0, /* Placed after  FES_SETUP */
    TXMON_FW2SW_TYPE_FES_SETUP_USER = 1, /* Placed before FES_SETUP_COMPLETE */
    TXMON_FW2SW_TYPE_FES_SETUP_EXT  = 2, /* Placed after  FES_SETUP_COMPLETE */
    TXMON_FW2SW_TYPE_MAX            = 4
};

typedef struct txmon_fw2sw_fes_setup {
    A_UINT32 band_center_freq1  : 16,
             band_center_freq2  : 16;
    A_UINT32 phy_mode : 8,              /* this field is filled with WLAN_PHY_MODE enum value */
             mhz      : 16,
             reserved : 8;
    A_UINT32 schedule_id;
} txmon_fw2sw_fes_setup_t;

typedef struct txmon_fw2sw_fes_setup_ext {
    A_UINT32 reserved;
} txmon_fw2sw_fes_setup_ext_t;

#define TXMON_FW2SW_MON_FES_SETUP_USER_USER_ID_M        0x0000003f
#define TXMON_FW2SW_MON_FES_SETUP_USER_USER_ID_S        0

#define TXMON_FW2SW_MON_FES_SETUP_USER_USER_ID_GET(_var) \
    (((_var) & TXMON_FW2SW_MON_FES_SETUP_USER_USER_ID_M) >> \
     TXMON_FW2SW_MON_FES_SETUP_USER_USER_ID_S)

#define TXMON_FW2SW_MON_FES_SETUP_USER_USER_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(TXMON_FW2SW_MON_FES_SETUP_USER_USER_ID, _val); \
        ((_var) |= ((_val) << TXMON_FW2SW_MON_FES_SETUP_USER_USER_ID_S)); \
    } while (0)


typedef struct txmon_fw2sw_fes_setup_user {
    A_UINT32 user_id  : 6,
             reserved : 24;
} txmon_fw2sw_fes_setup_user_t;

struct txmon_fw2sw_mon_fes_setup_tlv {
    struct tlv_usr_32_hdr tag; /* tlv_usrid => TXMON_FW2SW_TYPE_FES_SETUP */
    struct txmon_fw2sw_fes_setup setup;
};

struct txmon_fw2sw_mon_fes_setup_ext_tlv {
    struct tlv_usr_32_hdr tag; /* tlv_usrid => TXMON_FW2SW_TYPE_FES_SETUP_EXT*/
    struct txmon_fw2sw_fes_setup_ext setup_ext;
};

struct txmon_fw2sw_mon_fes_setup_user_tlv {
    struct tlv_usr_32_hdr tag; /* tlv_usrid => TXMON_FW2SW_TYPE_FES_SETUP_USER */
    struct txmon_fw2sw_fes_setup_user user_setup;
};

#endif /* _TXMON_TLVS_H_ */
