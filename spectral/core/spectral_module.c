/*
 * Copyright (c) 2011,2017-2019 The Linux Foundation. All rights reserved.
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

#include<linux/module.h>
#include <wlan_spectral_utils_api.h>
#include <qdf_types.h>
#include<wlan_global_lmac_if_api.h>
#include "spectral_defs_i.h"
#include <dispatcher_init_deinit.h>

MODULE_LICENSE("Dual BSD/GPL");

/**
 * spectral_init_module() - Initialize Spectral module
 *
 * Return: None
 */

#ifndef QCA_SINGLE_WIFI_3_0
static int __init spectral_init_module(void)
#else
int spectral_init_module(void)
#endif
{
	spectral_info("qca_spectral module loaded");
	wlan_spectral_init();
	/* register spectral rxops */
	wlan_lmac_if_sptrl_set_rx_ops_register_cb
	    (wlan_lmac_if_sptrl_register_rx_ops);
	/* register spectral pdev open handler */
	dispatcher_register_spectral_pdev_open_handler(
		spectral_pdev_open);

	return 0;
}

/**
 * spectral_exit_module() - De-initialize and exit Spectral module
 *
 * Return: None
 */
#ifndef QCA_SINGLE_WIFI_3_0
static void __exit spectral_exit_module(void)
#else
void spectral_exit_module(void)
#endif
{
	wlan_spectral_deinit();
	spectral_info("qca_spectral module unloaded");
}

#ifndef QCA_SINGLE_WIFI_3_0
module_init(spectral_init_module);
module_exit(spectral_exit_module);
#endif
