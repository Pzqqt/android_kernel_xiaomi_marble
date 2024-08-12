/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

/**
 * DOC: Define the data structure for AFC implementation
 */

#ifndef __WLAN_REG_AFC_H
#define __WLAN_REG_AFC_H

/* All the structures in this header will be packed and will follow network
 * byte order
 */

/**
 * struct wlan_afc_host_req_fixed_params - Structure to send the list of AFC
 *                                          requests to AFC app to query the
 *                                          AFC server.
 *
 * @req_id:        Unique request ID from FW to be used as AFC request ID
 *                 to server.
 * @version_minor  Lower 16 bits for the AFC request version.
 * @version_major  Higher 16 bits for the AFC request version.
 * @req_length:    Length of entire AFC request message.
 * @min_des_power: Minimum desired power(in dbm) for queried spectrum.
 */
struct wlan_afc_host_req_fixed_params {
	uint64_t req_id;
	uint16_t version_minor;
	uint16_t version_major;
	uint16_t req_length;
	int16_t  min_des_power;
} qdf_packed;

/**
 * struct wlan_afc_freq_range_obj - Structure for frequency range query.
 *
 * @lowfreq:  Lower limit(in MHz) for frequency range query.
 * @highfreq: Higher limit(in MHz) for frequency range query.
 */
struct wlan_afc_freq_range_obj {
	uint16_t lowfreq;
	uint16_t highfreq;
} qdf_packed;

/**
 * struct wlan_afc_frange_list - Structure to send freq range list to AFC app.
 *
 * @num_ranges: Number of queried frequency ranges.
 * @range_objs: List of queried frequency ranges.
 */
struct wlan_afc_frange_list {
	uint32_t num_ranges;
	struct wlan_afc_freq_range_obj range_objs[0];
} qdf_packed;

/**
 * struct wlan_afc_opclass_obj - Structure for opclass/channel query.
 *
 * @opclass_num_cfis: Number of channels to be required for given opclass.
 * @opclass:          Operating class to be queried.
 * @cfis:             List of Channels to be queried for given Global opclass.
 */
struct wlan_afc_opclass_obj {
	uint8_t opclass_num_cfis;
	uint8_t opclass;
	uint8_t cfis[0];
} qdf_packed;

/**
 * struct wlan_afc_num_opclasses - Structure for opclass list
 *
 * @num_opclasses: Number of opclass to be queried.
 */
struct wlan_afc_num_opclasses {
	uint8_t num_opclasses;
} qdf_packed;

/**
 * The following is the layout of the AFC host request
 * It is not a C structure as some of the structures are not of fixed size.
 *
 * struct wlan_afc_host_partial_request {
 *      <fixed-size>    struct wlan_afc_host_req_fixed_params fixed_parms;
 *      <variable-size> struct wlan_afc_freq_list freq_lst;
 *      <fixed-size>    struct wlan_afc_num_opclasses opclss_list_size;
 *      <variable-size> struct wlan_afc_opclass_obj obj[0];
 *      <variable-size> struct wlan_afc_opclass_obj obj[1];
 *      ....
 *      <variable-size>  struct wlan_afc_opclass_obj obj[opclass_list_size-1];
 * };
 *
 * struct wlan_afc_host_partial_request - Structure to send AFC request info
 *
 * @fixed_params: AFC request fixed params (req_id, length, min_des_power)
 */
struct wlan_afc_host_partial_request {
	struct wlan_afc_host_req_fixed_params fixed_params;
	/* Other structures to follow. See the layout in the comment above */
} qdf_packed;

/**
 * The following is the layout of the AFC response.
 *
 * struct wlan_afc_host_resp {
 *     header;
 *     status;
 *     time_to_live;
 *     length;
 *     resp_format;
 *     afc_resp {
 *          struct wlan_afc_bin_resp_data fixed_params;
 *          struct wlan_afc_resp_freq_psd_info obj[0];
 *          ....
 *          struct wlan_afc_resp_freq_psd_info obj[num_frequency_obj - 1];
 *          struct wlan_afc_resp_opclass_info opclass[0];
 *          {
 *              struct wlan_afc_resp_eirp_info eirp[0];
 *              ....
 *              struct wlan_afc_resp_eirp_info eirp[num_channels - 1];
 *          }
 *          .
 *          .
 *          struct wlan_afc_resp_opclass_info opclass[num_channel_obj - 1];
 *          {
 *              struct wlan_afc_resp_eirp_info eirp[0];
 *              ....
 *              struct wlan_afc_resp_eirp_info eirp[num_channels - 1];
 *          }
 *     }
 * }
 *
 * struct wlan_afc_host_resp - Structure for AFC Host response to FW
 *
 * @header:       Header for compatibility.
 *                Valid value: 0
 * @status:       Flag to indicate validity of data. To be updated by TZ
 *                1:  Success
 *                -1: Failure
 * @time_to_live: Period(in seconds) the data is valid for
 * @length:       Length of the response message
 * @resp_format:  AFC response format.
 *                0: JSON format
 *                1: Binary data format
 * @afc_resp:     Response message from the AFC server for queried parameters
 */
struct wlan_afc_host_resp {
	uint32_t header;
	int32_t  status;
	uint32_t time_to_live;
	uint32_t length;
	uint32_t resp_format;
	uint8_t afc_resp[];
} qdf_packed;

/**
 * struct wlan_afc_resp_opclass_info - Structure to populate operating class
 *                                     and channel information from AFC
 *                                     response.
 *
 * @opclass:        Operating class
 * @num_channels:   Number of channels received in AFC response
 */
struct wlan_afc_resp_opclass_info {
	uint32_t opclass;
	uint32_t num_channels;
} qdf_packed;

/**
 * struct wlan_afc_eirp_info - Structure to update EIRP values for channels
 *
 * @channel_cfi:  Channel center frequency index
 * @max_eirp_pwr: Maximum permissible EIRP(in dBm) for the Channel
 */
struct wlan_afc_resp_eirp_info {
	uint32_t channel_cfi;
	uint32_t max_eirp_pwr;
} qdf_packed;

/**
 * struct wlan_afc_freq_info - Structure to update PSD values for queried
 *                             frequency ranges
 *
 * @freq_info: Frequency range in MHz:- bits 15:0  = u16 start_freq,
 *                                      bits 31:16 = u16 end_freq
 * @max_psd:   Maximum PSD in dbm/MHz
 */
struct wlan_afc_resp_freq_psd_info {
	uint32_t freq_info;
	uint32_t max_psd;
} qdf_packed;

/**
 * struct wlan_afc_bin_resp_data - Structure to populate AFC binary response
 *
 * @local_err_code:     Internal error code between AFC app and FW
 *                      0 - Success
 *                      1 - General failure
 * @version:            Internal version between AFC app and FW
 *                      Current version: 1
 * @afc_wfa_version:    AFC spec version info. Bits 15:0  - Minor version
 *                                             Bits 31:16 - Major version
 * @request_id:         AFC unique request ID
 * @avail_exp_time_d:   Availability expiry date in UTC.
 *                      Date format: bits 7:0   - DD (Day 1-31)
 *                                   bits 15:8  - MM (Month 1-12)
 *                                   bits 31:16 - YYYY (Year)
 * @avail_exp_time_t:   Availability expiry time in UTC.
 *                      Time format: bits 7:0   - SS (Seconds 0-59)
 *                                   bits 15:8  - MM (Minutes 0-59)
 *                                   bits 23:16 - HH (Hours 0-23)
 *                                   bits 31:24 - Reserved
 * @afc_serv_resp_code: AFC server response code. The AFC server response codes
 *                      are defined in the WiFi Spec doc for AFC as follows:
 *                      0: Success.
 *                      100 - 199: General errors related to protocol.
 *                      300 - 399: Error events specific to message exchange
 *                                 for the available Spectrum Inquiry.
 * @num_frequency_obj:  Number of frequency objects
 * @num_channel_obj:    Number of channel objects
 * @shortdesc:          Short description corresponding to resp_code field
 */
struct wlan_afc_bin_resp_data {
	uint32_t local_err_code;
	uint32_t version;
	uint32_t afc_wfa_version;
	uint32_t request_id;
	uint32_t avail_exp_time_d;
	uint32_t avail_exp_time_t;
	uint32_t afc_serv_resp_code;
	uint32_t num_frequency_obj;
	uint32_t num_channel_obj;
	uint8_t  shortdesc[64];
	uint32_t reserved[2];
} qdf_packed;
#endif
