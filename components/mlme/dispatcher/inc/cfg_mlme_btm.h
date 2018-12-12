/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains MLME BTM related CFG/INI Items.
 */

#ifndef CFG_MLME_BTM_H_
#define CFG_MLME_BTM_H_

/*
 * <ini>
 * prefer_btm_query - Prefer btm query over 11k neighbor report
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable the STA to send BTM query instead of
 * 11k neighbor report.
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_PREFER_BTM_QUERY CFG_INI_BOOL( \
			"prefer_btm_query", \
			1, \
			"prefer btm query over 11k neighbor report")

/*
 * <ini>
 * btm_offload_config - Configure BTM
 * @Min: 0x00000000
 * @Max: 0xFFFFFFFF
 * @Default: 0x00000001
 *
 * This ini is used to configure BTM
 *
 * Bit 0: Enable/Disable the BTM offload. Set this to 1 will
 * enable and 0 will disable BTM offload.
 *
 * BIT 2, 1: Action on non matching candidate with cache. If a BTM request
 * is received from AP then the candidate AP's may/may-not be present in
 * the firmware scan cache . Based on below config firmware will decide
 * whether to forward BTM frame to host or consume with firmware and proceed
 * with Roaming to candidate AP.
 * 00 scan and consume
 * 01 no scan and forward to host
 * 10, 11 reserved
 *
 * BIT 5, 4, 3: Roaming handoff decisions on multiple candidates match
 * 000 match if exact BSSIDs are found
 * 001 match if at least one top priority BSSID only
 * 010, 011, 100, 101, 110, 111 reserved
 *
 * BIT 6: Set this to 1 will send BTM query frame and 0 not sent.
 *
 * BIT 7-31: Reserved
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BTM_ENABLE CFG_INI_UINT( \
	"btm_offload_config", \
	0x00000000, \
	0xffffffff, \
	0x00000001, \
	CFG_VALUE_OR_DEFAULT, \
	"configure btm offload")

/*
 * <ini>
 * btm_solicited_timeout - timeout value for waiting BTM request
 * @Min: 1
 * @Max: 10000
 * @Default: 100
 *
 * This ini is used to configure timeout value for waiting BTM request.
 * Unit: millionsecond
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BTM_SOLICITED_TIMEOUT CFG_INI_UINT( \
	"btm_solicited_timeout", \
	1, \
	10000, \
	100, \
	CFG_VALUE_OR_DEFAULT, \
	"configure timeout value for waiting BTM request")

/*
 * <ini>
 * btm_max_attempt_cnt - Maximum attempt for sending BTM query to ESS
 * @Min: 1
 * @Max: 0xFFFFFFFF
 * @Default: 3
 *
 * This ini is used to configure maximum attempt for sending BTM query to ESS.
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BTM_MAX_ATTEMPT_CNT CFG_INI_UINT( \
	"btm_max_attempt_cnt", \
	1, \
	0xFFFFFFFF, \
	3, \
	CFG_VALUE_OR_DEFAULT, \
	"configure maximum attempt for sending BTM query to ESS")

/*
 * <ini>
 * btm_sticky_time - Stick time after roaming to new AP by BTM
 * @Min: 0
 * @Max: 0x0000FFFF
 * @Default: 300
 *
 * This ini is used to configure Stick time after roaming to new AP by BTM.
 * Unit: seconds
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_BTM_STICKY_TIME CFG_INI_UINT( \
	"btm_sticky_time", \
	0, \
	0x0000FFFF, \
	300, \
	CFG_VALUE_OR_DEFAULT, \
	"configure Stick time after roaming to new AP by BTM")

#define CFG_BTM_ALL \
	CFG(CFG_PREFER_BTM_QUERY) \
	CFG(CFG_BTM_ENABLE) \
	CFG(CFG_BTM_SOLICITED_TIMEOUT) \
	CFG(CFG_BTM_MAX_ATTEMPT_CNT) \
	CFG(CFG_BTM_STICKY_TIME)

#endif /* CFG_MLME_BTM_H_ */
