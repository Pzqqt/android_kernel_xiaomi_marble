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

/**
 * DOC: qdf_ipa.c
 *
 * This source file contains linux specific definitions for QDF IPA APIs
 */

/* Include Files */
#include <qdf_ipa.h>

static uint8_t __qdf_to_ipa_wlan_event(int qdf_ipa_event)
{
	uint8_t ipa_event;

	switch (qdf_ipa_event) {
	case QDF_IPA_CLIENT_CONNECT:
		ipa_event = WLAN_CLIENT_CONNECT;
		break;
	case QDF_IPA_CLIENT_DISCONNECT:
		ipa_event = WLAN_CLIENT_DISCONNECT;
		break;
	case QDF_IPA_AP_CONNECT:
		ipa_event = WLAN_AP_CONNECT;
		break;
	case QDF_IPA_AP_DISCONNECT:
		ipa_event = WLAN_AP_DISCONNECT;
		break;
	case QDF_IPA_STA_CONNECT:
		ipa_event = WLAN_STA_CONNECT;
		break;
	case QDF_IPA_STA_DISCONNECT:
		ipa_event = WLAN_STA_DISCONNECT;
		break;
	case QDF_IPA_CLIENT_CONNECT_EX:
		ipa_event = WLAN_CLIENT_CONNECT_EX;
		break;
	case QDF_SWITCH_TO_SCC:
		ipa_event = WLAN_SWITCH_TO_SCC;
		break;
	case QDF_SWITCH_TO_MCC:
		ipa_event = WLAN_SWITCH_TO_MCC;
		break;
	case QDF_WDI_ENABLE:
		ipa_event = WLAN_WDI_ENABLE;
		break;
	case QDF_WDI_DISABLE:
		ipa_event = WLAN_WDI_DISABLE;
		break;
	case QDF_FWR_SSR_BEFORE_SHUTDOWN:
		ipa_event = WLAN_FWR_SSR_BEFORE_SHUTDOWN;
		break;
	case QDF_IPA_WLAN_EVENT_MAX:
	default:
		ipa_event = IPA_WLAN_EVENT_MAX;
		break;
	}

	return ipa_event;
}

void __qdf_ipa_set_meta_msg_type(__qdf_ipa_msg_meta_t *meta, int type)
{
	meta->msg_type = __qdf_to_ipa_wlan_event(type);
}
