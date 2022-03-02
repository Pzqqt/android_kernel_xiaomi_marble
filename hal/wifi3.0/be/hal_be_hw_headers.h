/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _HAL_BE_HW_INTERNAL_H_
#define _HAL_BE_HW_INTERNAL_H_

#include "wcss_seq_hwioreg_umac.h"
#include "phyrx_location.h"
#include "receive_rssi_info.h"
#include "buffer_addr_info.h"

#include "wbm2sw_completion_ring_tx.h"
#include "wbm2sw_completion_ring_rx.h"

#if defined(QCA_WIFI_KIWI)
#include "msmhwioreg.h"
#include "phyrx_common_user_info.h"
#endif
#include <reo_descriptor_threshold_reached_status.h>
#include <reo_flush_queue.h>

#define HAL_DESC_64_SET_FIELD(_desc, _word, _fld, _value) do { \
	((uint64_t *)(_desc))[(_word ## _ ## _fld ## _OFFSET) >> 3] &= \
		~(_word ## _ ## _fld ## _MASK); \
	((uint64_t *)(_desc))[(_word ## _ ## _fld ## _OFFSET) >> 3] |= \
		(((uint64_t)(_value)) << _word ## _ ## _fld ## _LSB); \
} while (0)

#endif /* _HAL_BE_HW_INTERNAL_H_ */
