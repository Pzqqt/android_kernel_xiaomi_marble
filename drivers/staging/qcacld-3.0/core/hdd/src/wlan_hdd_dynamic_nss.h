/*
 * Copyright (c) 2014, 2017 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_HDD_DYNAMIC_NSS_H
#define _WLAN_HDD_DYNAMIC_NSS_H
/**
 * DOC: wlan_hdd_dynamic_nss.h
 *
 * This module describes dynamic Nss strategy for power saving.
 */

/*
 * Include Files
 */
#include "qdf_mc_timer.h"
#include "wlan_hdd_main.h"

/*
 * Preprocessor Definitions and Constants
 */


/*
 * Type Declarations
 */
struct dynamic_nss_context {
	char	intf_name[IFNAMSIZ];
	struct dynamic_nss_context *next;
	qdf_mc_timer_t tput_stats_timer; //start the timer when sta get ip addr, and stop it when disconnected
	unsigned long last_txrx_bytes;
	unsigned long last_time_in_millisec;
	unsigned long consct_low_tput_cnt; //consecutive low throughput cnt
	unsigned long consct_high_tput_cnt;
	unsigned long consct_low_rssi_cnt;
	uint32_t tput_th;
	int8_t rssi_th;
	bool is_dnss_2x2;
};

void wlan_hdd_dynamic_nss_list_node_add(struct dynamic_nss_context *node);
void wlan_hdd_dynamic_nss_list_node_del(struct dynamic_nss_context *node);
struct dynamic_nss_context *wlan_hdd_get_dynamic_nss(struct net_device *dev);
void wlan_hdd_config_enable_dynamic_nss(bool enable);
void wlan_hdd_config_set_bt_active(bool active);
void wlan_hdd_set_bt_connected_mode(bool connected);
void wlan_hdd_init_dynamic_nss_ctx(struct hdd_adapter *adapter, struct dynamic_nss_context *dynamic_nss);
int wlan_hdd_set_nss_and_antenna_mode(struct hdd_adapter *adapter, int nss, int mode);
void wlan_hdd_start_dynamic_nss(struct hdd_adapter *adapter);
void wlan_hdd_stop_dynamic_nss(struct hdd_adapter *adapter);
void wlan_hdd_check_if_nss_switch(void *data);
void wlan_hdd_tput_stats_timer_deinit(struct hdd_context *hdd_ctx);
#endif /* #ifndef _WLAN_HDD_DYNAMIC_NSS_H */

