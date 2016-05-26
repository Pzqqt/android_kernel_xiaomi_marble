/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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
 * DOC: sme_nan_datapath.h
 *
 * SME NAN Data path API specification
 */

#ifndef __SME_NAN_DATAPATH_H
#define __SME_NAN_DATAPATH_H

#include "ani_global.h"

/* NAN initiator request handler */
QDF_STATUS sme_ndp_initiator_req_handler(uint32_t session_id,
					struct ndp_initiator_req *req_params);

/* NAN responder request handler */
QDF_STATUS sme_ndp_responder_req_handler(uint32_t session_id,
					struct ndp_responder_req *req_params);

/* NAN indication response handler */
QDF_STATUS sme_ndp_end_req_handler(uint32_t session_id,
					struct ndp_end_req *req_params);

/* NAN schedule update request handler */
QDF_STATUS sme_ndp_sched_req_handler(uint32_t session_id,
				struct ndp_schedule_update_req *req_params);

/* Function to handle NDP messages from lower layers */
void sme_ndp_message_processor(tpAniSirGlobal mac_ctx, uint16_t msg_type,
				void *msg);
#endif /* __SME_NAN_DATAPATH_H */
