/*
 * Copyright (c) 2011, 2016-2017 The Linux Foundation. All rights reserved.
 * Copyright (c) 2010, Atheros Communications Inc.
 * All Rights Reserved.
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
 * DOC: This file has dfs param copy functions.
 */

#ifndef _DFS_IOCTL_PRIVATE_H_
#define _DFS_IOCTL_PRIVATE_H_


static inline void
wlan_dfs_dfsparam_to_ioctlparam(struct wlan_dfs_phyerr_param *src,
		struct dfs_ioctl_params *dst)
{
	dst->dfs_firpwr = src->pe_firpwr;
	dst->dfs_rrssi = src->pe_rrssi;
	dst->dfs_height = src->pe_height;
	dst->dfs_prssi = src->pe_prssi;
	dst->dfs_inband = src->pe_inband;
	dst->dfs_relpwr = src->pe_relpwr;
	dst->dfs_relstep = src->pe_relstep;
	dst->dfs_maxlen = src->pe_maxlen;
}

#endif  /* _DFS_IOCTL_PRIVATE_H_ */
