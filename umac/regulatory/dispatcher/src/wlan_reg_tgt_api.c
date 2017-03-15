/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

 /**
 * @file wlan_req_tgt_api.c
 * @brief contains regulatory target interface definations
 */

#include <qdf_status.h>
#include <qdf_types.h>
#include <wlan_reg_tgt_api.h>
#include "../../core/src/reg_services.h"

/**
 * tgt_process_master_chan_list() - process master channel list
 * @reg_info: regulatory info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_reg_process_master_chan_list(struct cur_regulatory_info
					    *reg_info)
{
	return reg_process_master_chan_list(reg_info);
}
