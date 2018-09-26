/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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

#ifndef __CFG_MLME_DFS_H
#define __CFG_MLME_DFS_H

/*
 * <ini>
 * gEnableDFSMasterCap - Enable DFS master capability
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable/disable the DFS master capability.
 * Disabling it will cause driver to not advertise the spectrum
 * management capability
 * Related: NA.
 *
 * upported Feature: DFS
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_ENABLE_DFS_MASTER_CAPABILITY CFG_INI_BOOL( \
			"gEnableDFSMasterCap", \
			0, \
			"DFS master mode capability")

#define CFG_DFS_ALL \
	CFG(CFG_ENABLE_DFS_MASTER_CAPABILITY)

#endif /* __CFG_MLME_DFS_H */
