/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2007-2008 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * DOC: This file has ETSI Pre-CAC DFS APIs.
 */

#ifndef _DFS_ETSI_PRECAC_H_
#define _DFS_ETSI_PRECAC_H_

#include "dfs.h"

#define VHT160_IEEE_FREQ_DIFF 16

/**
 * struct dfs_etsi_precac_entry - PreCAC entry for ETSI domain
 * @pe_list:           ETSI PreCAC entry.
 * @ieee:              channel number
 * @etsi_caclst_ticks  start tick, OS speicfic.
 * @dfs:               Pointer to wlan_dfs structure.
 */
struct dfs_etsi_precac_entry {
	TAILQ_ENTRY(dfs_etsi_precac_entry) pe_list;
	uint16_t          ieee;
	unsigned long     etsi_caclst_ticks;
	struct wlan_dfs   *dfs;
};

/**
 * dfs_print_etsi_precaclists() - Print etsi precac list.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
void dfs_print_etsi_precaclists(struct wlan_dfs *dfs);
#else
static inline void dfs_print_etsi_precaclists(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_reset_etsi_precac_lists() - Resets the ETSI precac lists.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
void dfs_reset_etsi_precac_lists(struct wlan_dfs *dfs);
#else
static inline void dfs_reset_etsi_precac_lists(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_reset_etsiprecaclists()- Clears and initializes etsi_precac_required_list
 *                                etsi_precac_done_list.
 *
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
void dfs_reset_etsiprecaclists(struct wlan_dfs *dfs);
#else
static inline void dfs_reset_etsiprecaclists(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_add_to_etsi_precac_required_list()- Add channel to ETSI PreCAC Required
 * list.
 *
 * @dfs: Pointer to wlan_dfs structure.
 * @chan: Pointer to channel to be added to ETSI PreCAC Required List.
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
void dfs_add_to_etsi_precac_required_list(struct wlan_dfs *dfs, uint8_t *chan);
#else
static inline void dfs_add_to_etsi_precac_required_list(struct wlan_dfs *dfs,
							uint8_t *chan)
{
}
#endif

/**
 * dfs_deinit_etsi_precac_list() - Clears the etsi precac list.
 * @dfs: Pointer to wlan_dfs dtructure.
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
void dfs_deinit_etsi_precac_list(struct wlan_dfs *dfs);
#else
static inline void dfs_deinit_etsi_precac_list(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_etsi_precac_attach() - Initialize ETSI precac variables.
 * @dfs: Pointer to DFS structure.
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
void dfs_etsi_precac_attach(struct wlan_dfs *dfs);
#else
static inline void dfs_etsi_precac_attach(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_etsi_precac_detach() - Free etsi_precac memory.
 * @dfs: Pointer to wlan_dfs dtructure.
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
void dfs_etsi_precac_detach(struct wlan_dfs *dfs);
#else
static inline void dfs_etsi_precac_detach(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_init_etsiprecac_list() - Init ETSI precac list.
 * @dfs: Pointer to wlan_dfs dtructure.
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
void dfs_init_etsi_precac_list(struct wlan_dfs *dfs);
#else
static inline void dfs_init_etsi_precac_list(struct wlan_dfs *dfs)
{
}
#endif

/**
 * dfs_is_subchan_in_etsi_precac_done_list() - Is HT20 sub channel
 *                                             in etsi precac done list.
 * @dfs: Pointer to wlan_dfs structure.
 * @channel: HT20 sub channel
 *
 * Return: If subchannel present in precac done list return 1.
 *         Otherwise return 0
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
int dfs_is_subchan_in_etsi_precac_done_list(struct wlan_dfs *dfs,
					    uint8_t channel);
#else
static inline int dfs_is_subchan_in_etsi_precac_done_list(struct wlan_dfs *dfs)
{
	return 0;
}
#endif
/**
 * dfs_is_etsi_precac_done() - Is precac done.
 * @dfs: Pointer to wlan_dfs structure.
 *
 * Return: If precac already done in channel, return 1. Otherwise return 0.
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
bool dfs_is_etsi_precac_done(struct wlan_dfs *dfs);
#else
static inline bool dfs_is_etsi_precac_done(struct wlan_dfs *dfs)
{
	return false;
}
#endif

/**
 * dfs_mark_etsi_precac_dfs() - Mark the precac channel as radar for ETSI.
 * @dfs: Pointer to wlan_dfs structure.
 * @channels: List of HT20 primary channels
 * @num_channels: Number of HT20 primary channels
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
void dfs_mark_etsi_precac_dfs(struct wlan_dfs *dfs,
			      uint8_t *channels, uint8_t num_channels);
#else
static inline void dfs_mark_etsi_precac_dfs(struct wlan_dfs *dfs,
					    uint8_t *channels,
					    uint8_t num_channels)
{
}
#endif

/**
 * dfs_add_chan_to_etsi_done_list() - Add subchannel to ETSI CAC done list,
 *                                       if present in ETSI CAC required list
 * @dfs: Pointer to wlan_dfs structure.
 * @channel: HT20 primary channel
 *
 * Return: If channel added to ETSI CAC done list, return 1. Otherwise return 0.
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
int dfs_add_chan_to_etsi_done_list(struct wlan_dfs *dfs, uint8_t channel);
#else
static inline int dfs_add_chan_to_etsi_done_list(struct wlan_dfs *dfs,
						 uint8_t channel)
{
	return 0;
}
#endif

/**
 * dfs_add_to_etsi_precac_done_list() - Add channel to ETSI CAC done list
 * @curchan: Pointer to dfs_channel structure.
 *
 */
#if defined(QCA_SUPPORT_ETSI_PRECAC_DFS)
void dfs_add_to_etsi_precac_done_list(struct wlan_dfs *dfs);
#else
static inline void dfs_add_to_etsi_precac_done_list(struct wlan_dfs *dfs)
{
}
#endif

#endif /* _DFS_ETSI_PRECAC_H_ */
