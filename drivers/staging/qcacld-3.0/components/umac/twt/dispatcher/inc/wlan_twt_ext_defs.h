/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_twt_ext_defs.h
 *
 * This file provide definition for structure/enums/defines related to
 * twt component
 */

#ifndef __WLAN_TWT_EXT_DEFS_H__
#define __WLAN_TWT_EXT_DEFS_H__

/*
 * struct twt_mc_cfg_params - All twt related cfg items
 * @enable_twt: global twt configuration
 * @twt_responder: twt responder enable/disable
 * @twt_requestor: twt requestor enable/disable
 * @twt_congestion_timeout: congestion timeout value
 * @bcast_requestor_enabled: bcast requestor enable/disable
 * @bcast_responder_enabled: bcast responder enable/disable
 * @enable_twt_24ghz: Enable/disable host TWT when STA is connected in
 * 2.4Ghz
 * @flex_twt_sched: flex twt scheduling enable/disable
 * @req_flag: requestor flag enable/disable
 * @res_flag: responder flag enable/disable
 */
struct twt_mc_cfg_params {
	bool enable_twt;
	bool twt_responder;
	bool twt_requestor;
	uint32_t twt_congestion_timeout;
	bool bcast_requestor_enabled;
	bool bcast_responder_enabled;
	bool enable_twt_24ghz;
	bool flex_twt_sched;
	bool req_flag;
	bool res_flag;
};

#endif /* __WLAN_TWT_EXT_DEFS_H__ */

