/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains centralized cfg definitions of Spectral component
 */
#ifndef __CONFIG_SPECTRAL_H
#define __CONFIG_SPECTRAL_H

/*
 * <ini>
 * spectral_disable - disable spectral feature
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to disable spectral feature.
 *
 * Related: None
 *
 * Supported Feature: Spectral
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_SPECTRAL_DISABLE \
	CFG_INI_BOOL("spectral_disable", false, \
			"Spectral disable")

/*
 * <ini>
 * poison_spectral_bufs - enable poisoning of spectral buffers
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable the poisoning of spectral buffers.
 *
 * Related: None
 *
 * Supported Feature: Spectral
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_SPECTRAL_POISON_BUFS \
	CFG_INI_BOOL("poison_spectral_bufs", false, \
			"Enable spectral bufs poison at init")

#define CFG_SPECTRAL_ALL \
	CFG(CFG_SPECTRAL_DISABLE) \
	CFG(CFG_SPECTRAL_POISON_BUFS)

#endif
