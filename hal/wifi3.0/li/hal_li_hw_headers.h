/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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

#ifndef _HAL_LI_HW_INTERNAL_H_
#define _HAL_LI_HW_INTERNAL_H_

#include "hal_hw_headers.h"

#include "reo_reg_seq_hwioreg.h"
#include "mac_tcl_reg_seq_hwioreg.h"
#include "wbm_reg_seq_hwioreg.h"
#ifdef QCA_WIFI_QCA6490
#include "wfss_ce_channel_dst_reg_seq_hwioreg.h"
#include "wfss_ce_channel_src_reg_seq_hwioreg.h"
#else
#include "wfss_ce_reg_seq_hwioreg.h"
#endif /* QCA_WIFI_QCA6490 */
#include <reo_descriptor_threshold_reached_status.h>
#include <reo_flush_queue.h>
#include <reo_flush_timeout_list_status.h>
#include <reo_unblock_cache.h>
#include <reo_flush_cache.h>
#include <reo_flush_queue_status.h>
#include <reo_get_queue_stats.h>
#include <reo_unblock_cache_status.h>
#include <reo_flush_cache_status.h>
#include <reo_flush_timeout_list.h>
#include <reo_get_queue_stats_status.h>
#include <reo_update_rx_reo_queue.h>
#include <reo_update_rx_reo_queue_status.h>
#include <tlv_tag_def.h>


#endif /* _HAL_LI_HW_INTERNAL_H_ */
