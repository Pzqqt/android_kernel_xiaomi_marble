/*
 * Copyright (c) 2011-2019 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_MODULE_IDS_H_
#define _WLAN_MODULE_IDS_H_

/* Wlan module ids , global across all the modules */
typedef enum {
  WLAN_MODULE_ID_MIN = 0,
  WLAN_MODULE_INF = WLAN_MODULE_ID_MIN, /* 0x0 */
  WLAN_MODULE_WMI,                      /* 0x1 */
  WLAN_MODULE_STA_PWRSAVE,              /* 0x2 */
  WLAN_MODULE_WHAL,                     /* 0x3 */
  WLAN_MODULE_COEX,                     /* 0x4 */
  WLAN_MODULE_ROAM,                     /* 0x5 */
  WLAN_MODULE_RESMGR_CHAN_MANAGER,      /* 0x6 */
  WLAN_MODULE_RESMGR,                   /* 0x7 */
  WLAN_MODULE_VDEV_MGR,                 /* 0x8 */
  WLAN_MODULE_SCAN,                     /* 0x9 */
  WLAN_MODULE_RATECTRL,                 /* 0xa */
  WLAN_MODULE_AP_PWRSAVE,               /* 0xb */
  WLAN_MODULE_BLOCKACK,                 /* 0xc */
  WLAN_MODULE_MGMT_TXRX,                /* 0xd */
  WLAN_MODULE_DATA_TXRX,                /* 0xe */
  WLAN_MODULE_HTT,                      /* 0xf */
  WLAN_MODULE_HOST,                     /* 0x10 */
  WLAN_MODULE_BEACON,                   /* 0x11 */
  WLAN_MODULE_OFFLOAD,                  /* 0x12 */
  WLAN_MODULE_WAL,                      /* 0x13 */
  WAL_MODULE_DE,                        /* 0x14 */
  WLAN_MODULE_PCIELP,                   /* 0x15 */
  WLAN_MODULE_RTT,                      /* 0x16 */
  WLAN_MODULE_RESOURCE,                 /* 0x17 */
  WLAN_MODULE_DCS,                      /* 0x18 */
  WLAN_MODULE_CACHEMGR,                 /* 0x19 */
  WLAN_MODULE_ANI,                      /* 0x1a */
  WLAN_MODULE_P2P,                      /* 0x1b */
  WLAN_MODULE_CSA,                      /* 0x1c */
  WLAN_MODULE_NLO,                      /* 0x1d */
  WLAN_MODULE_CHATTER,                  /* 0x1e */
  WLAN_MODULE_WOW,                      /* 0x1f */
  WLAN_MODULE_WAL_VDEV,                 /* 0x20 */
  WLAN_MODULE_WAL_PDEV,                 /* 0x21 */
  WLAN_MODULE_TEST,                     /* 0x22 */
  WLAN_MODULE_STA_SMPS,                 /* 0x23 */
  WLAN_MODULE_SWBMISS,                  /* 0x24 */
  WLAN_MODULE_WMMAC,                    /* 0x25 */
  WLAN_MODULE_TDLS,                     /* 0x26 */
  WLAN_MODULE_HB,                       /* 0x27 */
  WLAN_MODULE_TXBF,                     /* 0x28 */
  WLAN_MODULE_BATCH_SCAN,               /* 0x29 */
  WLAN_MODULE_THERMAL_MGR,              /* 0x2a */
  WLAN_MODULE_PHYERR_DFS,               /* 0x2b */
  WLAN_MODULE_RMC,                      /* 0x2c */
  WLAN_MODULE_STATS,                    /* 0x2d */
  WLAN_MODULE_NAN,                      /* 0x2e */
  WLAN_MODULE_IBSS_PWRSAVE,             /* 0x2f */
  WLAN_MODULE_HIF_UART,                 /* 0x30 */
  WLAN_MODULE_LPI,                      /* 0x31 */
  WLAN_MODULE_EXTSCAN,                  /* 0x32 */
  WLAN_MODULE_UNIT_TEST,                /* 0x33 */
  WLAN_MODULE_MLME,                     /* 0x34 */
  WLAN_MODULE_SUPPL,                    /* 0x35 */
  WLAN_MODULE_ERE,                      /* 0x36 */
  WLAN_MODULE_OCB,                      /* 0x37 */
  WLAN_MODULE_RSSI_MONITOR,             /* 0x38 */
  WLAN_MODULE_WPM,                      /* 0x39 */
  WLAN_MODULE_CSS,                      /* 0x3a */
  WLAN_MODULE_PPS,                      /* 0x3b */
  WLAN_MODULE_SCAN_CH_PREDICT,          /* 0x3c */
  WLAN_MODULE_MAWC,                     /* 0x3d */
  WLAN_MODULE_CMC_QMIC,                 /* 0x3e */
  WLAN_MODULE_EGAP,                     /* 0x3f */
  WLAN_MODULE_NAN20,                    /* 0x40 */
  WLAN_MODULE_QBOOST,                   /* 0x41 */
  WLAN_MODULE_P2P_LISTEN_OFFLOAD,       /* 0x42 */
  WLAN_MODULE_HALPHY,                   /* 0x43 */
  WAL_MODULE_ENQ,                       /* 0x44 */
  WLAN_MODULE_GNSS,                     /* 0x45 */
  WLAN_MODULE_WAL_MEM,                  /* 0x46 */
  WLAN_MODULE_SCHED_ALGO,               /* 0x47 */
  WLAN_MODULE_TX,                       /* 0x48 */
  WLAN_MODULE_RX,                       /* 0x49 */
  WLAN_MODULE_WLM,                      /* 0x4a */
  WLAN_MODULE_RU_ALLOCATOR,             /* 0x4b */
  WLAN_MODULE_11K_OFFLOAD,              /* 0x4c */
  WLAN_MODULE_STA_TWT,                  /* 0x4d */
  WLAN_MODULE_AP_TWT,                   /* 0x4e */
  WLAN_MODULE_UL_OFDMA,                 /* 0x4f */
  WLAN_MODULE_HPCS_PULSE,               /* 0x50 */
  WLAN_MODULE_DTF,                      /* 0x51 */ /* Deterministic Test Framework */

  WLAN_MODULE_ID_MAX,
  WLAN_MODULE_ID_INVALID = WLAN_MODULE_ID_MAX,
} WLAN_MODULE_ID;


#endif /* _WLAN_MODULE_IDS_H_ */
