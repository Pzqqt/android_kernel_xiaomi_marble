/*
 * Copyright (c) 2012-2015 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/*
 *				dfs_phyerr.h
 *
 * OVERVIEW:
 *
 * Source code borrowed from QCA_MAIN DFS module
 *
 * DEPENDENCIES:
 *
 * Are listed for each API below.
 */


#ifndef __DFS_PHYERR_H__
#define __DFS_PHYERR_H__

extern int dfs_process_phyerr_bb_tlv(struct ath_dfs *dfs, void *buf,
				     uint16_t datalen, uint8_t rssi,
				     uint8_t ext_rssi, uint32_t rs_tstamp,
				     uint64_t fulltsf, struct dfs_phy_err *e,
				     bool enable_log);

#endif /* __DFS_PHYERR_H__ */
