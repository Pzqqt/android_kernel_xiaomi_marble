/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

/*
 * Structure and macros in this file are copied from rtt_oem_interface.h FW
 * header file.
 */

#ifndef _WMI_UNIFIED_RTT_H_
#define _WMI_UNIFIED_RTT_H_
/* For backward compatibility reason, CIVIC_INFO_MAX_LENGTH shouldn't be changed
 * from value 64.
 */
#define CIVIC_INFO_MAX_LENGTH   64

/* TLV Helper macro to set the TLV Header given the pointer to the TLV buffer */
#define WMIRTT_TLV_SET_HDR(tlv_buf, tag, len) \
	((((A_UINT32 *)(tlv_buf))[0]) = ((tag << 16) | (len & 0x0000FFFF)))

#define WMIRTT_TLV_TAG_STRUC_wmi_rtt_oem_req_head 32
#define WMIRTT_TLV_TAG_STRUC_wmi_rtt_oem_lcr_cfg_head 46
#define WMIRTT_TLV_TAG_STRUC_wmi_rtt_oem_lci_cfg_head 47

/* Set/get RTT request ID*/
#define WMI_RTT_REQ_ID_S 0
#define WMI_RTT_REQ_ID (0xffff << WMI_RTT_REQ_ID_S)
#define WMI_RTT_REQ_ID_GET(x) WMI_F_MS(x, WMI_RTT_REQ_ID)
#define WMI_RTT_REQ_ID_SET(x, z) WMI_F_RMW(x, z, WMI_RTT_REQ_ID)

/* Set/get latitude */
#define WMI_RTT_LCI_LAT_UNC_S 0
#define WMI_RTT_LCI_LAT_UNC (0xff << WMI_RTT_LCI_LAT_UNC_S)
#define WMI_RTT_LCI_LAT_UNC_GET(x) WMI_F_MS(x, WMI_RTT_LCI_LAT_UNC)
#define WMI_RTT_LCI_LAT_UNC_SET(x, z) WMI_F_RMW(x, z, WMI_RTT_LCI_LAT_UNC)

/* Set/get longitude */
#define WMI_RTT_LCI_LON_UNC_S 8
#define WMI_RTT_LCI_LON_UNC (0xff << WMI_RTT_LCI_LON_UNC_S)
#define WMI_RTT_LCI_LON_UNC_GET(x) WMI_F_MS(x, WMI_RTT_LCI_LON_UNC)
#define WMI_RTT_LCI_LON_UNC_SET(x, z) WMI_F_RMW(x, z, WMI_RTT_LCI_LON_UNC)

/* Set/get altitude */
#define WMI_RTT_LCI_ALT_UNC_S 16
#define WMI_RTT_LCI_ALT_UNC (0xff << WMI_RTT_LCI_ALT_UNC_S)
#define WMI_RTT_LCI_ALT_UNC_GET(x) WMI_F_MS(x, WMI_RTT_LCI_ALT_UNC)
#define WMI_RTT_LCI_ALT_UNC_SET(x, z) WMI_F_RMW(x, z, WMI_RTT_LCI_ALT_UNC)

/* Set/get motion pattern */
#define WMI_RTT_LCI_Z_MOTION_PAT_S 24
#define WMI_RTT_LCI_Z_MOTION_PAT (0xff << WMI_RTT_LCI_Z_MOTION_PAT_S)
#define WMI_RTT_LCI_Z_MOTION_PAT_GET(x) WMI_F_MS(x, WMI_RTT_LCI_Z_MOTION_PAT)
#define WMI_RTT_LCI_Z_MOTION_PAT_SET(x, z) \
	WMI_F_RMW(x, z, WMI_RTT_LCI_Z_MOTION_PAT)

/* Set/get hight above floor */
#define WMI_RTT_LCI_Z_HEIGHT_ABV_FLR_S 0
#define WMI_RTT_LCI_Z_HEIGHT_ABV_FLR (0xffff << WMI_RTT_LCI_Z_HEIGHT_ABV_FLR_S)
#define WMI_RTT_LCI_Z_HEIGHT_ABV_FLR_GET(x) \
	WMI_F_MS(x, WMI_RTT_LCI_Z_HEIGHT_ABV_FLR)
#define WMI_RTT_LCI_Z_HEIGHT_ABV_FLR_SET(x, z) \
	WMI_F_RMW(x, z, WMI_RTT_LCI_Z_HEIGHT_ABV_FLR)

/* Set/get hight uncertainty value */
#define WMI_RTT_LCI_Z_HEIGHT_UNC_S 16
#define WMI_RTT_LCI_Z_HEIGHT_UNC (0xff << WMI_RTT_LCI_Z_HEIGHT_UNC_S)
#define WMI_RTT_LCI_Z_HEIGHT_UNC_GET(x) WMI_F_MS(x, WMI_RTT_LCI_Z_HEIGHT_UNC)
#define WMI_RTT_LCI_Z_HEIGHT_UNC_SET(x, z) \
	WMI_F_RMW(x, z, WMI_RTT_LCI_Z_HEIGHT_UNC)

/* Set/get civic info */
#define WMI_RTT_LOC_CIVIC_LENGTH_S 0
#define WMI_RTT_LOC_CIVIC_LENGTH (0xff << WMI_RTT_LOC_CIVIC_LENGTH_S)
#define WMI_RTT_LOC_CIVIC_LENGTH_GET(x) WMI_F_MS(x, WMI_RTT_LOC_CIVIC_LENGTH)
#define WMI_RTT_LOC_CIVIC_LENGTH_SET(x, z) \
	WMI_F_RMW(x, z, WMI_RTT_LOC_CIVIC_LENGTH)

/**
 * WMIRTT_OEM_MSG_SUBTYPE - RTT message subtype definitions
 * @RTT_MSG_SUBTYPE_INVALID: Invalid message subtype
 * @RTT_MSG_SUBTYPE_CAPABILITY_REQ: Capability request
 * @RTT_MSG_SUBTYPE_CAPABILITY_RSP: Capability response
 * @RTT_MSG_SUBTYPE_MEASUREMENT_REQ: Measurement request
 * @RTT_MSG_SUBTYPE_MEASUREMENT_RSP: Measurement response
 * @RTT_MSG_SUBTYPE_ERROR_REPORT_RSP: Error report response
 * @RTT_MSG_SUBTYPE_CONFIGURE_LCR: Configure LCR
 * @RTT_MSG_SUBTYPE_CONFIGURE_LCI: Configure LCI
 * @RTT_MSG_SUBTYPE_CLEANUP_REQ: Cleanup request
 * @RTT_MSG_SUBTYPE_CLEANUP_RSP: Cleanup response
 * @RTT_MSG_SUBTYPE_GET_CHANNEL_INFO_REQ: Request for channel info
 * @RTT_MSG_SUBTYPE_GET_CHANNEL_INFO_RSP: RESPONSE for channel info
 * @RTT_MSG_SUBTYPE_CFG_RESPONDER_MODE_REQ: Request to enable/disable responder
 *                                          mode
 * @RTT_MSG_SUBTYPE_CFG_RESPONDER_MODE_RSP: Response for enable mode
 * @RTT_MSG_SUBTYPE_CANCEL_MEASUREMENT_REQ: Request to cancel measurement
 *                                          request
 * @RTT_MSG_SUBTYPE_CANCEL_MEASUREMENT_RSP: Response for cancel measurement
 *                                          request
 * @RTT_MSG_SUBTYPE_CFG_RESPONDER_MEASUREMENT_REQ: Request to enable or disable
 *                                                 responder measurement
 * @RTT_MSG_SUBTYPE_CFG_RESPONDER_MEASUREMENT_RSP: Response to responder
 *                                           measurement enable/disable request
 * @RTT_MSG_SUBTYPE_RESPONDER_MEASUREMENT_RSP: Responder measurement report
 */
enum WMIRTT_OEM_MSG_SUBTYPE {
	RTT_MSG_SUBTYPE_INVALID                  = 0x00,
	RTT_MSG_SUBTYPE_CAPABILITY_REQ           = 0x01,
	RTT_MSG_SUBTYPE_CAPABILITY_RSP           = 0x02,
	RTT_MSG_SUBTYPE_MEASUREMENT_REQ          = 0x03,
	RTT_MSG_SUBTYPE_MEASUREMENT_RSP          = 0x04,
	RTT_MSG_SUBTYPE_ERROR_REPORT_RSP         = 0x05,
	RTT_MSG_SUBTYPE_CONFIGURE_LCR            = 0x06,
	RTT_MSG_SUBTYPE_CONFIGURE_LCI            = 0x07,
	RTT_MSG_SUBTYPE_CLEANUP_REQ              = 0x08,
	RTT_MSG_SUBTYPE_CLEANUP_RSP              = 0x09,
	RTT_MSG_SUBTYPE_GET_CHANNEL_INFO_REQ     = 0x10,
	RTT_MSG_SUBTYPE_GET_CHANNEL_INFO_RSP     = 0x11,
	RTT_MSG_SUBTYPE_CFG_RESPONDER_MODE_REQ   = 0x12,
	RTT_MSG_SUBTYPE_CFG_RESPONDER_MODE_RSP   = 0x13,
	RTT_MSG_SUBTYPE_CANCEL_MEASUREMENT_REQ   = 0x14,
	RTT_MSG_SUBTYPE_CANCEL_MEASUREMENT_RSP   = 0x15,
	RTT_MSG_SUBTYPE_CFG_RESPONDER_MEASUREMENT_REQ = 0x16,
	RTT_MSG_SUBTYPE_CFG_RESPONDER_MEASUREMENT_RSP = 0x17,
	RTT_MSG_SUBTYPE_RESPONDER_MEASUREMENT_RSP = 0x18,
};

/**
 * wmi_rtt_oem_req_head - RTT OEM request head structure
 * @tlv_header: TLV tag and len; tag equals
 *              WMIRTT_TLV_TAG_STRUC_wmi_rtt_oem_req_head
 * @sub_type: WMIRTT_OEM_MSG_SUBTYPE
 * @req_id: Unique request ID for this RTT oem req;
 *          bit 15:0       Request ID
 *          bit 16:        sps enable  0- disable  1--enable
 *          bit 31:17      reserved
 * @pdev_id: pdev_id for identifying the MAC.
 */
struct wmi_rtt_oem_req_head {
	A_UINT32 tlv_header;
	A_UINT32 sub_type;
	A_UINT32 req_id;
	A_UINT32 pdev_id;
};

/**
 * wmi_rtt_oem_lci_cfg_head - LCI config structure
 * @tlv_header: TLV tag and len; tag equals
 *              WMIRTT_TLV_TAG_STRUC_wmi_rtt_oem_lci_cfg_head
 * @latitude: LS 34 bits - latitude in degrees * 2^25 , 2's complement; Lower
 *            32 bits comes first followed by higher 32 bytes
 * @longitude: LS 34 bits - latitude in degrees * 2^25 , 2's complement; Lower
 *             32 bits comes first followed by higher 32 bytes
 * @altitude: LS 30bits - Altitude in units of 1/256 m
 * @lci_cfg_param_info: Uncertainities & motion pattern cfg
 *                      bits 7:0 - Latitude_uncertainity as defined in Section
                                   2.3.2 of IETF RFC 6225
 *                      bits 15:8 - Longitude_uncertainity as defined in Section
 *                                  2.3.2 of IETF RFC 6225
 *                      bits 23:16 - Altitude_uncertainity as defined in Section
 *                                   2.4.5 of IETF RFC 6225
 *                      bits 31:24 - motion_pattern for use with z subelement
 *                                   cfg as per wmi_rtt_z_subelem_motion_pattern
 * @floor: in units 1/16th of floor # if known.
 *         value is 80000000 if unknown
 * @floor_param_info: height_above_floor & uncertainity
 *                    bits 15:0 - Height above floor in units of 1/64 m
 *                    bits 23:16 - Height uncertainity as defined in 802.11REVmc
 *                                 D4.0 Z subelem format.
 *                                 value 0 means unknown, values 1-18 are valid
 *                                 and 19 and above are reserved.
 *                    bits 31:24 - reserved
 * @usage_rules:
 *   bit  0     - usage_rules: retransmittion allowed: 0-No 1-Yes
 *   bit  1     - usage_rules: retention expires relative present: 0-No 1-Yes
 *   bit  2     - usage_rules: STA Location policy for Additional neighbor info:
 *                             0-No 1-Yes
 *   bits 7:3   - usage_rules: reserved
 *   bits 23:8  - usage_rules: retention expires relative, if present, as per
 *                             IETF RFC 4119
 *   bits 31:24 - reserved
 */
struct wmi_rtt_oem_lci_cfg_head {
	A_UINT32 tlv_header;
	A_UINT64 latitude;
	A_UINT64 longitude;
	A_UINT32 altitude;
	A_UINT32 lci_cfg_param_info;
	A_UINT32 floor;
	A_UINT32 floor_param_info;
	A_UINT32 usage_rules;
};

/**
 * wmi_rtt_oem_lcr_cfg_head - LCR config structure
 * @tlv_header: TLV tag and len; tag equals
 *              WMIRTT_TLV_TAG_STRUC_wmi_rtt_oem_lcr_cfg_head
 * @loc_civic_params:
 *     bit 7:0 - len in bytes. civic_info to be used in reference to this.
 *     bit 31:8 - reserved
 * @civic_info: Civic info including country_code to be copied in FTM frame.
 *              256 bytes max. Based on len, FW will copy byte-wise into
 *              local buffers and transfer OTA. This is packed as a 4 bytes
 *              aligned buffer at this interface for transfer to FW though.
 */
struct wmi_rtt_oem_lcr_cfg_head {
	A_UINT32 tlv_header;
	A_UINT32 loc_civic_params;
	A_UINT32 civic_info[CIVIC_INFO_MAX_LENGTH];
};
#endif
