/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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

#ifndef _HTC_CREDIT_HISTORY_H_
#define _HTC_CREDIT_HISTORY_H_

#include "htc_internal.h"

#ifdef FEATURE_HTC_CREDIT_HISTORY

/**
 * htc_credit_history_init(): Init helper function to initialize HTC credit
 *                            history buffers and variable.
 * Return: None
 */

void htc_credit_history_init(void);
void htc_credit_record(enum htc_credit_exchange_type type, uint32_t tx_credit,
			uint32_t htc_tx_queue_depth);

#else /* FEATURE_HTC_CREDIT_HISTORY */

static inline
void htc_credit_history_init(void)
{
}

static inline
void htc_credit_record(enum htc_credit_exchange_type type, uint32_t tx_credit,
			uint32_t htc_tx_queue_depth)
{ }
#endif /* FEATURE_HTC_CREDIT_HISTORY */
#endif /* _HTC_CREDIT_HISTORY_H_ */
