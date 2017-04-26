/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
 * Copyright (c) 2002-2010, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: This file contains NOL related functionality, NOL being the non
 * occupancy list. After radar has been detected in a particular channel,
 * the channel cannot be used for a period of 30 minutes which is called
 * the non occupancy. The NOL is basically a list of all the channels that
 * radar has been detected on. Each channel has a 30 minute timer associated
 * with it. This file contains the functionality to add a channel to the NOL,
 * the NOL timer  function and the functionality to remove a channel from the
 * NOL when its time is up.
 */

#include "../dfs.h"
#include "../dfs_channel.h"
#include "../dfs_ioctl_private.h"
#include "../dfs_internal.h"
#include <qdf_time.h>
#include <wlan_dfs_mlme_api.h>
#include <wlan_dfs_utils_api.h>
#include <wlan_reg_services_api.h>
#include <dfs_ioctl.h>

/* for loop to traverse the channel list. */

/**
 * dfs_nol_delete() - Delete the given frequency/chwidth from the NOL.
 * @dfs: Pointer to wlan_dfs structure.
 * @delfreq: Freq to delete.
 * @delchwidth: Channel width to delete.
 */
static void dfs_nol_delete(struct wlan_dfs *dfs,
		uint16_t delfreq,
		uint16_t delchwidth)
{
	struct dfs_nolelem *nol, **prev_next;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
			"%s: sc_dfs is NULL\n", __func__);
		return;
	}

	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL,
		"%s: remove channel=%d/%d MHz from NOL\n",
		__func__, delfreq, delchwidth);
	prev_next = &(dfs->dfs_nol);
	nol = dfs->dfs_nol;
	while (nol != NULL) {
		if (nol->nol_freq == delfreq &&
			nol->nol_chwidth == delchwidth) {
			*prev_next = nol->nol_next;
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL,
				"%s removing channel %d/%dMHz from NOL tstamp=%d\n",
				__func__, nol->nol_freq,
				nol->nol_chwidth,
				(qdf_system_ticks_to_msecs
				 (qdf_system_ticks()) / 1000));
			qdf_timer_stop(&nol->nol_timer);
			qdf_mem_free(nol);
			nol = NULL;
			nol = *prev_next;

			/* Update the NOL counter. */
			dfs->dfs_nol_count--;

			/* Be paranoid! */
			if (dfs->dfs_nol_count < 0) {
				DFS_PRINTK(
					"%s: dfs_nol_count < 0; eek!\n",
					__func__);
				dfs->dfs_nol_count = 0;
			}

		} else {
			prev_next = &(nol->nol_next);
			nol = nol->nol_next;
		}
	}
}

/**
 * dfs_remove_from_nol() - Remove the freq from NOL list.
 *
 * When NOL times out, this function removes the channel from NOL list.
 */
static os_timer_func(dfs_remove_from_nol)
{
	struct dfs_nol_timer_arg *nol_arg;
	struct wlan_dfs *dfs;
	uint16_t delfreq;
	uint16_t delchwidth;
	uint8_t chan;

	OS_GET_TIMER_ARG(nol_arg, struct dfs_nol_timer_arg *);

	dfs = nol_arg->dfs;
	delfreq = nol_arg->delfreq;
	delchwidth = nol_arg->delchwidth;

	/* Delete the given NOL entry. */
	dfs_nol_delete(dfs, delfreq, delchwidth);

	/* Update the wireless stack with the new NOL. */
	dfs_nol_update(dfs);

	dfs_mlme_nol_timeout_notification(dfs->dfs_pdev_obj);
	chan = utils_dfs_freq_to_chan(delfreq);
	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL,
		    "%s: remove channel %d from nol\n", __func__, chan);
	utils_dfs_reg_update_nol_ch(dfs->dfs_pdev_obj,
			(uint8_t *)&chan, 1, DFS_NOL_RESET);
	dfs_save_nol(dfs->dfs_pdev_obj);
	qdf_mem_free(nol_arg);
}

void dfs_print_nol(struct wlan_dfs *dfs)
{
	struct dfs_nolelem *nol;
	int i = 0;
	uint32_t diff_ms, remaining_sec;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL,
			"%s: sc_dfs is NULL\n", __func__);
		return;
	}

	nol = dfs->dfs_nol;
	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL, "%s: NOL\n", __func__);
	while (nol != NULL) {
		diff_ms = qdf_system_ticks_to_msecs(qdf_system_ticks() -
				nol->nol_start_ticks);
		diff_ms = (nol->nol_timeout_ms - diff_ms);
		remaining_sec = diff_ms / 1000; /* Convert to seconds */
		DFS_PRINTK(
			"nol:%d channel=%d MHz width=%d MHz time left=%u seconds nol starttick=%llu\n",
			i++, nol->nol_freq,
			nol->nol_chwidth,
			remaining_sec,
			(uint64_t)nol->nol_start_ticks);
		nol = nol->nol_next;
	}
}

void dfs_print_nolhistory(struct wlan_dfs *dfs)
{
	struct dfs_ieee80211_channel *c, lc;
	int i, j = 0;
	int nchans = 0;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL,
			"%s: sc_dfs is NULL\n", __func__);
		return;
	}

	c = &lc;

	dfs_mlme_get_ic_nchans(dfs->dfs_pdev_obj, &nchans);
	for (i = 0; i < nchans; i++) {
		dfs_mlme_get_ic_channels(dfs->dfs_pdev_obj,
				&(c->ic_freq),
				&(c->ic_flags),
				&(c->ic_flagext),
				&(c->ic_ieee),
				&(c->ic_vhtop_ch_freq_seg1),
				&(c->ic_vhtop_ch_freq_seg2),
				i);
		if (IEEE80211_IS_CHAN_HISTORY_RADAR(c)) {
			DFS_PRINTK(
				"nolhistory:%d channel=%d MHz Flags=%X\n",
				j, c->ic_freq, c->ic_flags);
			j++;
		}
	}
}

void dfs_get_nol(struct wlan_dfs *dfs,
		struct dfsreq_nolelem *dfs_nol,
		int *nchan)
{
	struct dfs_nolelem *nol;

	*nchan = 0;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL,
			"%s: sc_dfs is NULL\n", __func__);
		return;
	}

	nol = dfs->dfs_nol;
	while (nol != NULL) {
		dfs_nol[*nchan].nol_freq = nol->nol_freq;
		dfs_nol[*nchan].nol_chwidth = nol->nol_chwidth;
		dfs_nol[*nchan].nol_start_ticks = nol->nol_start_ticks;
		dfs_nol[*nchan].nol_timeout_ms = nol->nol_timeout_ms;
		++(*nchan);
		nol = nol->nol_next;
	}
}

void dfs_set_nol(struct wlan_dfs *dfs,
		struct dfsreq_nolelem *dfs_nol,
		int nchan)
{
#define TIME_IN_MS 1000
	uint32_t nol_time_left_ms;
	struct dfs_ieee80211_channel chan;
	int i;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL,
			"%s: sc_dfs is NULL\n", __func__);
		return;
	}

	for (i = 0; i < nchan; i++) {
		nol_time_left_ms =
			qdf_system_ticks_to_msecs(qdf_system_ticks() -
				dfs_nol[i].nol_start_ticks);
		if (nol_time_left_ms < dfs_nol[i].nol_timeout_ms) {
			chan.ic_freq = dfs_nol[i].nol_freq;
			chan.ic_flags = 0;
			chan.ic_flagext = 0;
			nol_time_left_ms =
				(dfs_nol[i].nol_timeout_ms - nol_time_left_ms);
			dfs_nol_addchan(dfs, chan.ic_freq,
					(nol_time_left_ms / TIME_IN_MS));
		}
	}
#undef TIME_IN_MS
	dfs_nol_update(dfs);
}

void dfs_nol_addchan(struct wlan_dfs *dfs,
		uint16_t freq,
		uint32_t dfs_nol_timeout)
{
#define TIME_IN_MS 1000
#define TIME_IN_US (TIME_IN_MS * 1000)
	struct dfs_nolelem *nol, *elem, *prev;
	struct dfs_nol_timer_arg *dfs_nol_arg;
	/* For now, assume all events are 20MHz wide. */
	int ch_width = 20;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL,
			"%s: dfs is NULL\n", __func__);
		return;
	}
	nol = dfs->dfs_nol;
	prev = dfs->dfs_nol;
	elem = NULL;
	while (nol != NULL) {
		if ((nol->nol_freq == freq) &&
				(nol->nol_chwidth == ch_width)) {
			nol->nol_start_ticks = qdf_system_ticks();
			nol->nol_timeout_ms = dfs_nol_timeout * TIME_IN_MS;

			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL,
				"%s: Update OS Ticks for NOL %d MHz / %d MHz\n",
				__func__, nol->nol_freq, nol->nol_chwidth);

			qdf_timer_stop(&nol->nol_timer);
			OS_SET_TIMER(&nol->nol_timer,
					dfs_nol_timeout * TIME_IN_MS);
			return;
		}
		prev = nol;
		nol = nol->nol_next;
	}

	/* Add a new element to the NOL. */
	elem = (struct dfs_nolelem *)qdf_mem_malloc(sizeof(struct dfs_nolelem));
	if (elem == NULL)
		goto bad;

	dfs_nol_arg = (struct dfs_nol_timer_arg *)qdf_mem_malloc(
			sizeof(struct dfs_nol_timer_arg));
	if (dfs_nol_arg == NULL) {
		qdf_mem_free(elem);
		goto bad;
	}
	elem->nol_freq = freq;
	elem->nol_chwidth = ch_width;
	elem->nol_start_ticks = qdf_system_ticks();
	elem->nol_timeout_ms = dfs_nol_timeout*TIME_IN_MS;
	elem->nol_next = NULL;
	if (prev) {
		prev->nol_next = elem;
	} else {
		/* This is the first element in the NOL. */
		dfs->dfs_nol = elem;
	}
	dfs_nol_arg->dfs = dfs;
	dfs_nol_arg->delfreq = elem->nol_freq;
	dfs_nol_arg->delchwidth = elem->nol_chwidth;

	qdf_timer_init(NULL,
			&elem->nol_timer, dfs_remove_from_nol,
			dfs_nol_arg, QDF_TIMER_TYPE_WAKE_APPS);
	OS_SET_TIMER(&elem->nol_timer, dfs_nol_timeout * TIME_IN_MS);

	/* Update the NOL counter. */
	dfs->dfs_nol_count++;

	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL,
		"%s: new NOL channel %d MHz / %d MHz\n",
		__func__, elem->nol_freq, elem->nol_chwidth);
	return;

bad:
	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS_NOL | WLAN_DEBUG_DFS,
		"%s: failed to allocate memory for nol entry\n",
		__func__);

#undef TIME_IN_MS
#undef TIME_IN_US
}

void dfs_get_nol_chfreq_and_chwidth(struct dfs_nol_chan_entry *nollist,
		uint32_t *nol_chfreq,
		uint32_t *nol_chwidth,
		int index)
{
	if (nollist == NULL)
		return;

	*nol_chfreq = nollist[index].nol_chfreq;
	*nol_chwidth = nollist[index].nol_chwidth;
}

void dfs_nol_update(struct wlan_dfs *dfs)
{
	struct dfs_nol_chan_entry *dfs_nol;
	struct dfs_nolelem *nol;
	int nlen;

	/*
	 * Allocate enough entries to store the NOL. At least on Linux
	 * (don't ask why), if you allocate a 0 entry array, the
	 * returned pointer is 0x10.  Make sure you're aware of this
	 * when you start debugging.
	 */
	dfs_nol = (struct dfs_nol_chan_entry *)qdf_mem_malloc(
		sizeof(struct dfs_nol_chan_entry) * dfs->dfs_nol_count);

	if (dfs_nol == NULL) {
		/*
		 * XXX TODO: if this fails, just schedule a task to retry
		 * updating the NOL at a later stage.  That way the NOL
		 * update _DOES_ happen - hopefully the failure was just
		 * temporary.
		 */
		DFS_PRINTK(
			"%s: failed to allocate NOL update memory!\n",
			__func__);
		return;
	}

	/* Populate the nol array. */
	nlen = 0;

	nol = dfs->dfs_nol;
	while (nol != NULL && nlen < dfs->dfs_nol_count) {
		dfs_nol[nlen].nol_chfreq = nol->nol_freq;
		dfs_nol[nlen].nol_chwidth = nol->nol_chwidth;
		dfs_nol[nlen].nol_start_ticks = nol->nol_start_ticks;
		dfs_nol[nlen].nol_timeout_ms = nol->nol_timeout_ms;
		nlen++;
		nol = nol->nol_next;
	}

	/* Be suitably paranoid for now. */
	if (nlen != dfs->dfs_nol_count)
		DFS_PRINTK("%s: nlen (%d) != dfs->dfs_nol_count (%d)!\n",
			__func__, nlen, dfs->dfs_nol_count);

	/*
	 * Call the driver layer to have it recalculate the NOL flags
	 * for each driver/umac channel. If the list is empty, pass
	 * NULL instead of dfs_nol. The operating system may have some
	 * special representation for "malloc a 0 byte memory region"
	 * - for example, Linux 2.6.38-13 (ubuntu) returns 0x10 rather
	 * than a valid allocation (and is likely not NULL so the
	 * pointer doesn't match NULL checks in any later code.
	 */
	dfs_mlme_clist_update(dfs->dfs_pdev_obj,
			(nlen > 0) ? dfs_nol : NULL,
			nlen);

	qdf_mem_free(dfs_nol);
}

void dfs_nol_timer_cleanup(struct wlan_dfs *dfs)
{
	struct dfs_nolelem *nol = dfs->dfs_nol, *prev;

	while (nol) {
		qdf_timer_stop(&nol->nol_timer);
		nol = nol->nol_next;
	}

	/* Free NOL elem, don't mix this while loop with above loop. */
	nol = dfs->dfs_nol;
	while (nol) {
		prev = nol;
		nol = nol->nol_next;
		qdf_mem_free(prev);
		/* Update the NOL counter. */
		dfs->dfs_nol_count--;

		if (dfs->dfs_nol_count < 0) {
			DFS_PRINTK("%s: dfs_nol_count < 0\n", __func__);
			ASSERT(0);
		}
	}
	dfs->dfs_nol = NULL;
	dfs_nol_update(dfs);
	dfs_mlme_set_no_chans_available(dfs->dfs_pdev_obj, 0);
}

int dfs_get_rn_use_nol(struct wlan_dfs *dfs)
{
	return dfs->dfs_rinfo.rn_use_nol;
}

int dfs_get_nol_timeout(struct wlan_dfs *dfs)
{
	return dfs->wlan_dfs_nol_timeout;
}
