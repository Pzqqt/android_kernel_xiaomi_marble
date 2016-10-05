/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

#ifndef WLAN_HDD_FTM_H
#define WLAN_HDD_FTM_H

/**
 * DOC: wlan_hdd_ftm.h
 *
 * WLAN Host Device Driver Factory Test Mode header file
 */

#include "qdf_status.h"
#include "cds_mq.h"
#include "cds_api.h"
#include "msg.h"
#include "qdf_types.h"
#include <wlan_ptt_sock_svc.h>

enum wlan_hdd_ftm_state {
	WLAN_FTM_INITIALIZED,
	WLAN_FTM_STOPPED,
	WLAN_FTM_STARTED,
};

/**
 * struct wlan_hdd_ftm_status - FTM status
 * @ftm_state: The current state of the FTM process
 */
struct wlan_hdd_ftm_status {
	enum wlan_hdd_ftm_state ftm_state;
};

int hdd_update_cds_config_ftm(hdd_context_t *hdd_ctx);
void hdd_ftm_mc_process_msg(void *message);
#if  defined(QCA_WIFI_FTM)
QDF_STATUS wlan_hdd_ftm_testmode_cmd(void *data, int len);
int wlan_hdd_qcmbr_unified_ioctl(hdd_adapter_t *adapter, struct ifreq *ifr);
#endif

#endif
