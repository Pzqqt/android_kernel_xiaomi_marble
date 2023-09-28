/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2008 Atheros Communications, Inc.
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
 * DOC: The structurs and functions in this file are used only within DFS
 * component.
 */

#ifndef _DFS_INTERNAL_H_
#define _DFS_INTERNAL_H_

#include <qdf_timer.h>
#include "dfs.h"

/**
 * enum DFS_DOMAIN - DFS domain
 * @DFS_UNINIT_DOMAIN: Uninitialized domain
 * @DFS_FCC_DOMAIN: FCC domain
 * @DFS_ETSI_DOMAIN: ETSI domain
 * @DFS_MKK4_DOMAIN: MKK domain
 * @DFS_CN_DOMAIN: China domain
 * @DFS_KR_DOMAIN: Korea domain
 * @DFS_MKKN_DOMAIN: MKKN domain
 * @DFS_UNDEF_DOMAIN: Undefined domain
 */
enum DFS_DOMAIN {
	DFS_UNINIT_DOMAIN = 0,
	DFS_FCC_DOMAIN    = 1,
	DFS_ETSI_DOMAIN   = 2,
	DFS_MKK4_DOMAIN   = 3,
	DFS_CN_DOMAIN     = 4,
	DFS_KR_DOMAIN     = 5,
	DFS_MKKN_DOMAIN   = 6,
	DFS_UNDEF_DOMAIN
};

/* CAPABILITY: the device support STA DFS */
#define WLAN_CEXT_STADFS 0x00000040

/**
 * dfs_chan2freq() - Convert channel to frequency value.
 * @chan: Pointer to dfs_channel structure.
 *
 * Return: Channel frequency.
 */
uint16_t dfs_chan2freq(struct dfs_channel *chan);

#endif /*  _DFS_INTERNAL_H_ */
