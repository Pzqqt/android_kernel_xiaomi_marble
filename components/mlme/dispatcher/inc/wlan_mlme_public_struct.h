/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: contains mlme structure definations
 */

#ifndef _WLAN_MLME_STRUCT_H_
#define _WLAN_MLME_STRUCT_H_

#include <wlan_cmn.h>

/**
 * struct mlme_ht_capabilities_info - HT Capabilities Info
 * @lsigTXOPProtection: L-SIG TXOP Protection Mechanism support
 * @stbcControlFrame: STBC Control frame support
 * @psmp: PSMP Support
 * @dsssCckMode40MHz: To indicate use of DSSS/CCK in 40Mhz
 * @maximalAMSDUsize: Maximum AMSDU Size - 0:3839 octes, 1:7935 octets
 * @delayedBA: Support of Delayed Block Ack
 * @rxSTBC: Rx STBC Support - 0:Not Supported, 1: 1SS, 2: 1,2SS, 3: 1,2,3SS
 * @txSTBC: Tx STBC Support
 * @shortGI40MHz: Short GI Support for HT40
 * @shortGI20MHz: Short GI support for HT20
 * @greenField: Support for HT Greenfield PPDUs
 * @mimoPowerSave: SM Power Save Mode - 0:Static, 1:Dynamic, 3:Disabled, 2:Res
 * @supportedChannelWidthSet: Supported Channel Width - 0:20Mhz, 1:20Mhz & 40Mhz
 * @advCodingCap: Rx LDPC support
 */
#ifndef ANI_LITTLE_BIT_ENDIAN
struct mlme_ht_capabilities_info {
	uint16_t lsigTXOPProtection:1;
	uint16_t stbcControlFrame:1;
	uint16_t psmp:1;
	uint16_t dsssCckMode40MHz:1;
	uint16_t maximalAMSDUsize:1;
	uint16_t delayedBA:1;
	uint16_t rxSTBC:2;
	uint16_t txSTBC:1;
	uint16_t shortGI40MHz:1;
	uint16_t shortGI20MHz:1;
	uint16_t greenField:1;
	uint16_t mimoPowerSave:2;
	uint16_t supportedChannelWidthSet:1;
	uint16_t advCodingCap:1;
} qdf_packed;
#else
struct mlme_ht_capabilities_info {
	uint16_t advCodingCap:1;
	uint16_t supportedChannelWidthSet:1;
	uint16_t mimoPowerSave:2;
	uint16_t greenField:1;
	uint16_t shortGI20MHz:1;
	uint16_t shortGI40MHz:1;
	uint16_t txSTBC:1;
	uint16_t rxSTBC:2;
	uint16_t delayedBA:1;
	uint16_t maximalAMSDUsize:1;
	uint16_t dsssCckMode40MHz:1;
	uint16_t psmp:1;
	uint16_t stbcControlFrame:1;
	uint16_t lsigTXOPProtection:1;
} qdf_packed;
#endif

/**
 * struct wlan_mlme_ht_caps - HT Capabilities related config items
 * @ht_cap_info: HT capabilities Info Structure
 */
struct wlan_mlme_ht_caps {
	struct mlme_ht_capabilities_info ht_cap_info;
};

struct wlan_mlme_vht_caps {
	/* VHT related configs */
};

/**
 * struct wlan_mlme_cfg - MLME config items
 * @ht_cfg: HT related CFG Items
 * @vht_cfg: VHT related CFG Items
 */
struct wlan_mlme_cfg {
	struct wlan_mlme_ht_caps ht_caps;
	struct wlan_mlme_vht_caps vht_caps;
};

#endif
