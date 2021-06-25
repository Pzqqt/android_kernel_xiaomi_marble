/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 * struct wlan_afc_host_response - Structure for AFC Host response to FW
 *
 * @status:       Flag to indicate validity of data. To be updated by TZ
 * @time_to_live: Period(in seconds) the data is valid for
 * @length:       Length of the response message
 * @afc_resp:     Response message from the AFC server for queried parameters
 */
struct wlan_afc_host_resp {
	int32_t status;
	int32_t time_to_live;
	int32_t length;
	uint8_t afc_resp[0];
} qdf_packed;
#endif
