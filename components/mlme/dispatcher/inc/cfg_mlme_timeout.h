/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains centralized definitions of converged configuration.
 */

#ifndef __CFG_MLME_TIMEOUT_H
#define __CFG_MLME_TIMEOUT_H

/*
 * <ini>
 * join_failure_timeout - Join failure timeout value
 * @Min: 0
 * @Max: 65535
 * @Default: 3000
 *
 * This cfg is used to configure the join failure timeout.
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_JOIN_FAILURE_TIMEOUT CFG_INI_UINT( \
		"join_failure_timeout", \
		0, \
		65535, \
		3000, \
		CFG_VALUE_OR_DEFAULT, \
		"Join failure timeout")

#define CFG_TIMEOUT_ALL \
	CFG(CFG_JOIN_FAILURE_TIMEOUT)

#endif /* __CFG_MLME_TIMEOUT_H */
