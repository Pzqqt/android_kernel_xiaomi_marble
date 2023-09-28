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

#include<linux/module.h>
#include <wlan_iot_sim_utils_api.h>
#include <qdf_types.h>
#include<wlan_global_lmac_if_api.h>
#include "iot_sim_defs_i.h"
#include <dispatcher_init_deinit.h>

MODULE_LICENSE("Dual BSD/GPL");

/**
 * iot_sim_init_module() - Initialize IOT Simulation module
 *
 * Return: None
 */

static int __init iot_sim_init_module(void)
{
	wlan_iot_sim_init();
	iot_sim_info("qca_iot_sim module loaded");
	wlan_lmac_if_iot_sim_set_rx_ops_register_cb(
					wlan_lmac_if_iot_sim_register_rx_ops);
	return 0;
}

/**
 * iot_sim_exit_module() - De-initialize and exit IOT Simulation module
 *
 * Return: None
 */
static void __exit iot_sim_exit_module(void)
{
	wlan_iot_sim_deinit();
	iot_sim_info("qca_iot_sim module unloaded");
}

module_init(iot_sim_init_module);
module_exit(iot_sim_exit_module);
