/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
/**
 * DOC: wifi_pos_oem_interface.h
 * This file defines the interface between host driver and userspace layer.
 */
#ifndef _WIFI_POS_OEM_INTERFACE_H_
#define _WIFI_POS_OEM_INTERFACE_H_

/* Include files */
#include "qdf_types.h"
#include "qdf_status.h"
#include "wlan_objmgr_cmn.h"

#define TARGET_OEM_CAPABILITY_REQ       0x01
#define TARGET_OEM_CAPABILITY_RSP       0x02
#define TARGET_OEM_MEASUREMENT_REQ      0x03
#define TARGET_OEM_MEASUREMENT_RSP      0x04
#define TARGET_OEM_ERROR_REPORT_RSP     0x05
#define TARGET_OEM_NAN_MEAS_REQ         0x06
#define TARGET_OEM_NAN_MEAS_RSP         0x07
#define TARGET_OEM_NAN_PEER_INFO        0x08
#define TARGET_OEM_CONFIGURE_LCR        0x09
#define TARGET_OEM_CONFIGURE_LCI        0x0A
#define TARGET_OEM_CONFIGURE_WRU        0x80
#define TARGET_OEM_CONFIGURE_FTMRR      0x81

#define WIFI_POS_FLAG_DFS               10
#define WIFI_POS_SET_DFS(info)          (info |=  (1 << WIFI_POS_FLAG_DFS))

struct wifi_pos_field {
	uint32_t id;
	uint32_t offset;
	uint32_t length;
};

struct wifi_pos_field_info {
	uint32_t count;
	struct wifi_pos_field fields[1];
};

/**
 * enum WMIRTT_FIELD_ID - identifies which field is being specified
 * @WMIRTT_FIELD_ID_oem_data_sub_type: oem data req sub type
 * @WMIRTT_FIELD_ID_channel_mhz: channel mhz info
 * @WMIRTT_FIELD_ID_pdev: pdev info
 */
enum WMIRTT_FIELD_ID {
	WMIRTT_FIELD_ID_oem_data_sub_type,
	WMIRTT_FIELD_ID_channel_mhz,
	WMIRTT_FIELD_ID_pdev,
};

#endif
