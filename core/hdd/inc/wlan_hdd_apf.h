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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: wlan_hdd_apf.h
 *
 * Android Packet Filter related API's and definitions
 */

#ifndef __WLAN_HDD_APF_H
#define __WLAN_HDD_APF_H

#ifdef FEATURE_WLAN_APF

#include <net/cfg80211.h>
#include "sir_api.h"
#include "wlan_hdd_main.h"
#include "wmi_unified_param.h"

#define APF_CONTEXT_MAGIC 0x4575354

#define MAX_APF_MEMORY_LEN	4096

/* APF commands wait times in msec */
#define WLAN_WAIT_TIME_APF_GET_CAPS     1000
#define WLAN_WAIT_TIME_APF_READ_MEM     10000

/*
 * struct hdd_apf_context - hdd Context for apf
 * @magic: magic number
 * @qdf_apf_event: Completion variable for APF get operations
 * @capability_response: capabilities response received from fw
 * @apf_enabled: True: APF Interpreter enabled, False: Disabled
 * @cmd_in_progress: Flag that indicates an APF command is in progress
 * @buf: Buffer to accumulate read memory chunks
 * @buf_len: Length of the read memory requested
 * @offset: APF work memory offset to fetch from
 * @lock: APF Context lock
 */
struct hdd_apf_context {
	unsigned int magic;
	qdf_event_t qdf_apf_event;
	struct sir_apf_get_offload capability_response;
	bool apf_enabled;
	bool cmd_in_progress;
	uint8_t *buf;
	uint32_t buf_len;
	uint32_t offset;
	qdf_spinlock_t lock;
};

/**
 * wlan_hdd_cfg80211_apf_offload() - SSR Wrapper to APF Offload
 * @wiphy:    wiphy structure pointer
 * @wdev:     Wireless device structure pointer
 * @data:     Pointer to the data received
 * @data_len: Length of @data
 *
 * Return: 0 on success; errno on failure
 */

int wlan_hdd_cfg80211_apf_offload(struct wiphy *wiphy,
				  struct wireless_dev *wdev,
				  const void *data, int data_len);

/**
 * hdd_apf_context_init - APF Context initialization operations
 *
 * Return: None
 */
void hdd_apf_context_init(void);

/**
 * hdd_apf_context_destroy - APF Context de-init operations
 *
 * Return: None
 */
void hdd_apf_context_destroy(void);

/**
 * hdd_get_apf_capabilities_cb() - Callback function to get APF capabilities
 * @hdd_context: hdd_context
 * @apf_get_offload: struct for get offload
 *
 * This function receives the response/data from the lower layer and
 * checks to see if the thread is still waiting then post the results to
 * upper layer, if the request has timed out then ignore.
 *
 * Return: None
 */
void hdd_get_apf_capabilities_cb(void *hdd_context,
				 struct sir_apf_get_offload *data);
#else /* FEATURE_WLAN_APF */

static inline void hdd_apf_context_init(void)
{
}

static inline void hdd_apf_context_destroy(void)
{
}

#endif /* FEATURE_WLAN_APF */

#endif /* WLAN_HDD_APF_H */
