/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_dfs_utils_api.h>
#include <wlan_reg_services_api.h>
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
#include "../dfs_process_radar_found_ind.h"
#include "../dfs_partial_offload_radar.h"
#endif

void dfs_set_update_nol_flag(struct wlan_dfs *dfs, bool val)
{
	dfs->update_nol = val;
}

bool dfs_get_update_nol_flag(struct wlan_dfs *dfs)
{
	return dfs->update_nol;
}

/**
 * dfs_nol_timeout() - NOL timeout function.
 *
 * Clears the WLAN_CHAN_DFS_RADAR_FOUND flag for the NOL timeout channel.
 */
/* Unused function */
#ifdef CONFIG_CHAN_FREQ_API
static os_timer_func(dfs_nol_timeout)
{
	struct dfs_channel *c = NULL, lc;
	unsigned long oldest, now;
	struct wlan_dfs *dfs = NULL;
	int i;
	int nchans = 0;

	c = &lc;

	OS_GET_TIMER_ARG(dfs, struct wlan_dfs *);
	dfs_mlme_get_dfs_ch_nchans(dfs->dfs_pdev_obj, &nchans);

	now = oldest = qdf_system_ticks();
	for (i = 0; i < nchans; i++) {
		dfs_mlme_get_dfs_channels_for_freq
			(dfs->dfs_pdev_obj,
			 &c->dfs_ch_freq,
			 &c->dfs_ch_flags,
			 &c->dfs_ch_flagext,
			 &c->dfs_ch_ieee,
			 &c->dfs_ch_vhtop_ch_freq_seg1,
			 &c->dfs_ch_vhtop_ch_freq_seg2,
			 &c->dfs_ch_mhz_freq_seg1,
			 &c->dfs_ch_mhz_freq_seg2,
			 i);
		if (WLAN_IS_CHAN_RADAR(c)) {
			if (qdf_system_time_after_eq(now,
						     dfs->dfs_nol_event[i] +
						     dfs_get_nol_timeout(dfs))) {
				c->dfs_ch_flagext &= ~WLAN_CHAN_DFS_RADAR_FOUND;
				if (c->dfs_ch_flags & WLAN_CHAN_DFS_RADAR) {
					/*
					 * NB: do this here so we get only one
					 * msg instead of one for every channel
					 * table entry.
					 */
					dfs_debug(dfs, WLAN_DEBUG_DFS,
						  "radar on channel %u (%u MHz) cleared after timeout",
						  c->dfs_ch_ieee,
						  c->dfs_ch_freq);
				}
			} else if (dfs->dfs_nol_event[i] < oldest) {
				oldest = dfs->dfs_nol_event[i];
			}
		}
	}
	if (oldest != now) {
		/* Arrange to process next channel up for a status change. */
		qdf_timer_mod(&dfs->dfs_nol_timer,
			      dfs_get_nol_timeout(dfs) -
			      qdf_system_ticks_to_msecs(qdf_system_ticks()));
	}
}
#else
#ifdef CONFIG_CHAN_NUM_API
static os_timer_func(dfs_nol_timeout)
{
	struct dfs_channel *c = NULL, lc;
	unsigned long oldest, now;
	struct wlan_dfs *dfs = NULL;
	int i;
	int nchans = 0;

	c = &lc;

	OS_GET_TIMER_ARG(dfs, struct wlan_dfs *);
	dfs_mlme_get_dfs_ch_nchans(dfs->dfs_pdev_obj, &nchans);

	now = oldest = qdf_system_ticks();
	for (i = 0; i < nchans; i++) {
		dfs_mlme_get_dfs_ch_channels(dfs->dfs_pdev_obj,
				&(c->dfs_ch_freq),
				&(c->dfs_ch_flags),
				&(c->dfs_ch_flagext),
				&(c->dfs_ch_ieee),
				&(c->dfs_ch_vhtop_ch_freq_seg1),
				&(c->dfs_ch_vhtop_ch_freq_seg2),
				i);
		if (WLAN_IS_CHAN_RADAR(c)) {
			if (qdf_system_time_after_eq(now,
						dfs->dfs_nol_event[i] +
						dfs_get_nol_timeout(dfs))) {
				c->dfs_ch_flagext &=
					~WLAN_CHAN_DFS_RADAR_FOUND;
				if (c->dfs_ch_flags &
						WLAN_CHAN_DFS_RADAR) {
					/*
					 * NB: do this here so we get only one
					 * msg instead of one for every channel
					 * table entry.
					 */
					dfs_debug(dfs, WLAN_DEBUG_DFS,
						"radar on channel %u (%u MHz) cleared after timeout",

						c->dfs_ch_ieee,
						c->dfs_ch_freq);
				}
			} else if (dfs->dfs_nol_event[i] < oldest)
				oldest = dfs->dfs_nol_event[i];
		}
	}
	if (oldest != now) {
		/* Arrange to process next channel up for a status change. */
		qdf_timer_mod(&dfs->dfs_nol_timer,
				dfs_get_nol_timeout(dfs) -
				qdf_system_ticks_to_msecs(qdf_system_ticks()));
	}
}
#endif
#endif

/**
 * dfs_nol_elem_free_work_cb -  Free NOL element
 *
 * Free the NOL element memory
 */
static void dfs_nol_elem_free_work_cb(void *context)
{
	struct wlan_dfs *dfs = (struct wlan_dfs *)context;
	struct dfs_nolelem *nol_head;

	while (true) {
		WLAN_DFSNOL_LOCK(dfs);

		nol_head = TAILQ_FIRST(&dfs->dfs_nol_free_list);
		if (nol_head) {
			TAILQ_REMOVE(&dfs->dfs_nol_free_list, nol_head,
				     nolelem_list);
			WLAN_DFSNOL_UNLOCK(dfs);

			qdf_timer_free(&nol_head->nol_timer);
			qdf_mem_free(nol_head);
		} else {
			WLAN_DFSNOL_UNLOCK(dfs);
			break;
		}
	}
}

void dfs_nol_timer_init(struct wlan_dfs *dfs)
{
	qdf_timer_init(NULL,
			&(dfs->dfs_nol_timer),
			dfs_nol_timeout,
			(void *)(dfs),
			QDF_TIMER_TYPE_WAKE_APPS);
}

void dfs_nol_attach(struct wlan_dfs *dfs)
{
	dfs->wlan_dfs_nol_timeout = DFS_NOL_TIMEOUT_S;
	dfs_nol_timer_init(dfs);
	qdf_create_work(NULL, &dfs->dfs_nol_elem_free_work,
			dfs_nol_elem_free_work_cb, dfs);
	TAILQ_INIT(&dfs->dfs_nol_free_list);
	dfs->dfs_use_nol = 1;
	WLAN_DFSNOL_LOCK_CREATE(dfs);
}

void dfs_nol_detach(struct wlan_dfs *dfs)
{
	dfs_nol_timer_cleanup(dfs);
	qdf_flush_work(&dfs->dfs_nol_elem_free_work);
	qdf_destroy_work(NULL, &dfs->dfs_nol_elem_free_work);
	WLAN_DFSNOL_LOCK_DESTROY(dfs);
}

void dfs_nol_timer_detach(struct wlan_dfs *dfs)
{
	qdf_timer_free(&dfs->dfs_nol_timer);
}

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

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS_NOL,
		"remove channel=%d/%d MHz from NOL",
		 delfreq, delchwidth);
	prev_next = &(dfs->dfs_nol);
	nol = dfs->dfs_nol;
	while (nol) {
		if (nol->nol_freq == delfreq &&
			nol->nol_chwidth == delchwidth) {
			*prev_next = nol->nol_next;
			dfs_debug(dfs, WLAN_DEBUG_DFS_NOL,
				"removing channel %d/%dMHz from NOL tstamp=%d",
				 nol->nol_freq,
				nol->nol_chwidth,
				(qdf_system_ticks_to_msecs
				 (qdf_system_ticks()) / 1000));
			TAILQ_INSERT_TAIL(&dfs->dfs_nol_free_list,
						nol, nolelem_list);
			nol = *prev_next;

			/* Update the NOL counter. */
			dfs->dfs_nol_count--;

			/* Be paranoid! */
			if (dfs->dfs_nol_count < 0) {
				dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS, "dfs_nol_count < 0; eek!");
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
#ifdef CONFIG_CHAN_FREQ_API
static os_timer_func(dfs_remove_from_nol)
{
	struct dfs_nolelem *nol_arg;
	struct wlan_dfs *dfs;
	uint16_t delfreq;
	uint16_t delchwidth;
	uint8_t chan;

	OS_GET_TIMER_ARG(nol_arg, struct dfs_nolelem *);

	dfs = nol_arg->nol_dfs;
	delfreq = nol_arg->nol_freq;
	delchwidth = nol_arg->nol_chwidth;

	/* Delete the given NOL entry. */
	DFS_NOL_DELETE_CHAN_LOCKED(dfs, delfreq, delchwidth);

	/* Update the wireless stack with the new NOL. */
	dfs_nol_update(dfs);

	dfs_mlme_nol_timeout_notification(dfs->dfs_pdev_obj);
	chan = utils_dfs_freq_to_chan(delfreq);
	utils_dfs_deliver_event(dfs->dfs_pdev_obj, delfreq,
				WLAN_EV_NOL_FINISHED);
	dfs_debug(dfs, WLAN_DEBUG_DFS_NOL,
		  "remove channel %d from nol", chan);
	utils_dfs_unmark_precac_nol_for_freq(dfs->dfs_pdev_obj, delfreq);
	utils_dfs_reg_update_nol_chan_for_freq(dfs->dfs_pdev_obj,
					     &delfreq, 1, DFS_NOL_RESET);
	utils_dfs_save_nol(dfs->dfs_pdev_obj);
}
#else
#ifdef CONFIG_CHAN_NUM_API
static os_timer_func(dfs_remove_from_nol)
{
	struct dfs_nolelem *nol_arg;
	struct wlan_dfs *dfs;
	uint16_t delfreq;
	uint16_t delchwidth;
	uint8_t chan;

	OS_GET_TIMER_ARG(nol_arg, struct dfs_nolelem *);

	dfs = nol_arg->nol_dfs;
	delfreq = nol_arg->nol_freq;
	delchwidth = nol_arg->nol_chwidth;

	/* Delete the given NOL entry. */
	DFS_NOL_DELETE_CHAN_LOCKED(dfs, delfreq, delchwidth);

	/* Update the wireless stack with the new NOL. */
	dfs_nol_update(dfs);

	dfs_mlme_nol_timeout_notification(dfs->dfs_pdev_obj);
	chan = utils_dfs_freq_to_chan(delfreq);
	utils_dfs_deliver_event(dfs->dfs_pdev_obj, delfreq,
				WLAN_EV_NOL_FINISHED);
	dfs_debug(dfs, WLAN_DEBUG_DFS_NOL,
		    "remove channel %d from nol", chan);
	utils_dfs_unmark_precac_nol(dfs->dfs_pdev_obj, chan);
	utils_dfs_reg_update_nol_ch(dfs->dfs_pdev_obj,
				    &chan, 1, DFS_NOL_RESET);
	utils_dfs_save_nol(dfs->dfs_pdev_obj);
}
#endif
#endif

void dfs_print_nol(struct wlan_dfs *dfs)
{
	struct dfs_nolelem *nol;
	int i = 0;
	uint32_t diff_ms, remaining_sec;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	nol = dfs->dfs_nol;
	dfs_debug(dfs, WLAN_DEBUG_DFS_NOL, "NOL");
	while (nol) {
		diff_ms = qdf_system_ticks_to_msecs(qdf_system_ticks() -
				nol->nol_start_ticks);
		diff_ms = (nol->nol_timeout_ms - diff_ms);
		remaining_sec = diff_ms / 1000; /* Convert to seconds */
		dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"nol:%d channel=%d MHz width=%d MHz time left=%u seconds nol starttick=%llu",
			i++, nol->nol_freq,
			nol->nol_chwidth,
			remaining_sec,
			(uint64_t)nol->nol_start_ticks);
		nol = nol->nol_next;
	}
}

void dfs_print_nolhistory(struct wlan_dfs *dfs)
{
	struct dfs_channel *chan_list;
	int i, j;
	int nchans;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	nchans = dfs_get_num_chans();

	chan_list = qdf_mem_malloc(nchans * sizeof(*chan_list));
	if (!chan_list)
		return;

	utils_dfs_get_nol_history_chan_list(dfs->dfs_pdev_obj,
					    (void *)chan_list, &nchans);
	if (!nchans) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "zero chans");
		qdf_mem_free(chan_list);
		return;
	}

	for (i = 0, j = 0; i < nchans; i++, j++)
		dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS,
			 "nolhistory = %d channel = %d MHz",
			 j, chan_list[i].dfs_ch_freq);

	qdf_mem_free(chan_list);
}

void dfs_get_nol(struct wlan_dfs *dfs,
		struct dfsreq_nolelem *dfs_nol,
		int *nchan)
{
	struct dfs_nolelem *nol;

	*nchan = 0;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	nol = dfs->dfs_nol;
	while (nol) {
		dfs_nol[*nchan].nol_freq = nol->nol_freq;
		dfs_nol[*nchan].nol_chwidth = nol->nol_chwidth;
		dfs_nol[*nchan].nol_start_ticks = nol->nol_start_ticks;
		dfs_nol[*nchan].nol_timeout_ms = nol->nol_timeout_ms;
		++(*nchan);
		nol = nol->nol_next;
	}
}

#ifdef CONFIG_CHAN_FREQ_API
void dfs_set_nol(struct wlan_dfs *dfs,
		 struct dfsreq_nolelem *dfs_nol,
		 int nchan)
{
#define TIME_IN_MS 1000
	uint32_t nol_time_lft_ms;
	struct dfs_channel chan;
	int i;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	for (i = 0; i < nchan; i++) {
		nol_time_lft_ms =
			qdf_system_ticks_to_msecs(qdf_system_ticks() -
				dfs_nol[i].nol_start_ticks);

		if (nol_time_lft_ms < dfs_nol[i].nol_timeout_ms) {
			chan.dfs_ch_freq = dfs_nol[i].nol_freq;
			chan.dfs_ch_flags = 0;
			chan.dfs_ch_flagext = 0;
			nol_time_lft_ms =
				(dfs_nol[i].nol_timeout_ms - nol_time_lft_ms);

			DFS_NOL_ADD_CHAN_LOCKED(dfs, chan.dfs_ch_freq,
						(nol_time_lft_ms / TIME_IN_MS));
			utils_dfs_reg_update_nol_chan_for_freq(dfs->dfs_pdev_obj,
							     &chan.dfs_ch_freq,
							     1, DFS_NOL_SET);
		}
	}
#undef TIME_IN_MS
	dfs_nol_update(dfs);
}
#else
#ifdef CONFIG_CHAN_NUM_API
void dfs_set_nol(struct wlan_dfs *dfs,
		struct dfsreq_nolelem *dfs_nol,
		int nchan)
{
#define TIME_IN_MS 1000
	uint32_t nol_time_left_ms;
	struct dfs_channel chan;
	int i;
	uint8_t chan_num;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	for (i = 0; i < nchan; i++) {
		nol_time_left_ms =
			qdf_system_ticks_to_msecs(qdf_system_ticks() -
				dfs_nol[i].nol_start_ticks);

		if (nol_time_left_ms < dfs_nol[i].nol_timeout_ms) {
			chan.dfs_ch_freq = dfs_nol[i].nol_freq;
			chan.dfs_ch_flags = 0;
			chan.dfs_ch_flagext = 0;
			nol_time_left_ms =
				(dfs_nol[i].nol_timeout_ms - nol_time_left_ms);

			DFS_NOL_ADD_CHAN_LOCKED(dfs, chan.dfs_ch_freq,
					(nol_time_left_ms / TIME_IN_MS));
			chan_num = utils_dfs_freq_to_chan(chan.dfs_ch_freq);
			utils_dfs_reg_update_nol_ch(dfs->dfs_pdev_obj,
						&chan_num, 1, DFS_NOL_SET);
		}
	}
#undef TIME_IN_MS
	dfs_nol_update(dfs);
}
#endif
#endif

void dfs_nol_addchan(struct wlan_dfs *dfs,
		uint16_t freq,
		uint32_t dfs_nol_timeout)
{
#define TIME_IN_MS 1000
#define TIME_IN_US (TIME_IN_MS * 1000)
	struct dfs_nolelem *nol, *elem, *prev;
	/* For now, assume all events are 20MHz wide. */
	int ch_width = 20;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}
	nol = dfs->dfs_nol;
	prev = dfs->dfs_nol;
	elem = NULL;
	while (nol) {
		if ((nol->nol_freq == freq) &&
				(nol->nol_chwidth == ch_width)) {
			nol->nol_start_ticks = qdf_system_ticks();
			nol->nol_timeout_ms = dfs_nol_timeout * TIME_IN_MS;

			dfs_debug(dfs, WLAN_DEBUG_DFS_NOL,
				"Update OS Ticks for NOL %d MHz / %d MHz",
				 nol->nol_freq, nol->nol_chwidth);

			qdf_timer_stop(&nol->nol_timer);
			qdf_timer_mod(&nol->nol_timer,
					dfs_nol_timeout * TIME_IN_MS);
			return;
		}
		prev = nol;
		nol = nol->nol_next;
	}

	/* Add a new element to the NOL. */
	elem = (struct dfs_nolelem *)qdf_mem_malloc(sizeof(struct dfs_nolelem));
	if (!elem)
		goto bad;

	qdf_mem_zero(elem, sizeof(*elem));
	elem->nol_dfs = dfs;
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

	qdf_timer_init(NULL,
		       &elem->nol_timer, dfs_remove_from_nol,
		       elem, QDF_TIMER_TYPE_WAKE_APPS);

	qdf_timer_mod(&elem->nol_timer, dfs_nol_timeout * TIME_IN_MS);

	/* Update the NOL counter. */
	dfs->dfs_nol_count++;

	dfs_debug(dfs, WLAN_DEBUG_DFS_NOL,
		"new NOL channel %d MHz / %d MHz",
		 elem->nol_freq, elem->nol_chwidth);
	return;

bad:
	dfs_debug(dfs, WLAN_DEBUG_DFS_NOL | WLAN_DEBUG_DFS,
		"failed to allocate memory for nol entry");

#undef TIME_IN_MS
#undef TIME_IN_US
}

void dfs_get_nol_chfreq_and_chwidth(struct dfsreq_nolelem *dfs_nol,
		uint32_t *nol_chfreq,
		uint32_t *nol_chwidth,
		int index)
{
	if (!dfs_nol)
		return;

	*nol_chfreq = dfs_nol[index].nol_freq;
	*nol_chwidth = dfs_nol[index].nol_chwidth;
}

void dfs_nol_update(struct wlan_dfs *dfs)
{
	struct dfsreq_nolelem *dfs_nol;
	int nlen;

	if (!dfs->dfs_nol_count) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_NOL, "dfs_nol_count is zero");
		dfs_mlme_clist_update(dfs->dfs_pdev_obj, NULL, 0);
		return;
	}

	/*
	 * Allocate enough entries to store the NOL. At least on Linux
	 * (don't ask why), if you allocate a 0 entry array, the
	 * returned pointer is 0x10.  Make sure you're aware of this
	 * when you start debugging.
	 */
	dfs_nol = (struct dfsreq_nolelem *)qdf_mem_malloc(
		sizeof(struct dfsreq_nolelem) * dfs->dfs_nol_count);

	/*
	 * XXX TODO: if this fails, just schedule a task to retry
	 * updating the NOL at a later stage.  That way the NOL
	 * update _DOES_ happen - hopefully the failure was just
	 * temporary.
	 */
	if (!dfs_nol)
		return;

	DFS_GET_NOL_LOCKED(dfs, dfs_nol, &nlen);

	/* Be suitably paranoid for now. */
	if (nlen != dfs->dfs_nol_count)
		dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS, "nlen (%d) != dfs->dfs_nol_count (%d)!",
			 nlen, dfs->dfs_nol_count);

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

void dfs_nol_free_list(struct wlan_dfs *dfs)
{
	struct dfs_nolelem *nol = dfs->dfs_nol, *prev;

	while (nol) {
		prev = nol;
		nol = nol->nol_next;
		qdf_mem_free(prev);
		/* Update the NOL counter. */
		dfs->dfs_nol_count--;

		if (dfs->dfs_nol_count < 0) {
			dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs_nol_count < 0");
			ASSERT(0);
		}
	}

	dfs->dfs_nol = NULL;
}

#ifdef CONFIG_CHAN_FREQ_API
void dfs_nol_timer_cleanup(struct wlan_dfs *dfs)
{
	struct dfs_nolelem *nol;
	uint16_t nol_freq;

	while (true) {
		WLAN_DFSNOL_LOCK(dfs);

		nol = dfs->dfs_nol;
		if (nol) {
			dfs->dfs_nol = nol->nol_next;
			dfs->dfs_nol_count--;
			nol_freq = nol->nol_freq;
			WLAN_DFSNOL_UNLOCK(dfs);
			utils_dfs_reg_update_nol_chan_for_freq(
					dfs->dfs_pdev_obj,
					&nol_freq,
					1,
					DFS_NOL_RESET);

			qdf_timer_free(&nol->nol_timer);
			qdf_mem_free(nol);
		} else {
			WLAN_DFSNOL_UNLOCK(dfs);
			break;
		}
	}
}
#else
#ifdef CONFIG_CHAN_NUM_API
void dfs_nol_timer_cleanup(struct wlan_dfs *dfs)
{
	struct dfs_nolelem *nol;
	uint8_t nol_chan;

	while (true) {
		WLAN_DFSNOL_LOCK(dfs);

		nol = dfs->dfs_nol;
		if (nol) {
			dfs->dfs_nol = nol->nol_next;
			dfs->dfs_nol_count--;
			nol_chan = utils_dfs_freq_to_chan(nol->nol_freq);
			WLAN_DFSNOL_UNLOCK(dfs);
			utils_dfs_reg_update_nol_ch(dfs->dfs_pdev_obj,
						    &nol_chan,
						    1,
						    DFS_NOL_RESET);

			qdf_timer_free(&nol->nol_timer);
			qdf_mem_free(nol);
		} else {
			WLAN_DFSNOL_UNLOCK(dfs);
			break;
		}
	}
}
#endif
#endif

void dfs_nol_workqueue_cleanup(struct wlan_dfs *dfs)
{
	qdf_flush_work(&dfs->dfs_nol_elem_free_work);
}

int dfs_get_use_nol(struct wlan_dfs *dfs)
{
	return dfs->dfs_use_nol;
}

int dfs_get_nol_timeout(struct wlan_dfs *dfs)
{
	return dfs->wlan_dfs_nol_timeout;
}

void dfs_getnol(struct wlan_dfs *dfs, void *dfs_nolinfo)
{
	struct dfsreq_nolinfo *nolinfo = (struct dfsreq_nolinfo *)dfs_nolinfo;

	DFS_GET_NOL_LOCKED(dfs, nolinfo->dfs_nol, &(nolinfo->dfs_ch_nchans));
}

#ifdef CONFIG_CHAN_FREQ_API
void dfs_clear_nolhistory(struct wlan_dfs *dfs)
{
	struct dfs_channel *chan_list;
	int nchans = 0;
	bool sta_opmode;

	if (!dfs->dfs_is_stadfs_enabled)
		return;

	sta_opmode = dfs_mlme_is_opmode_sta(dfs->dfs_pdev_obj);
	if (!sta_opmode)
		return;

	nchans = dfs_get_num_chans();

	chan_list = qdf_mem_malloc(nchans * sizeof(*chan_list));
	if (!chan_list)
		return;

	utils_dfs_get_nol_history_chan_list(dfs->dfs_pdev_obj,
					    (void *)chan_list, &nchans);
	if (!nchans) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "zero chans");
		qdf_mem_free(chan_list);
		return;
	}

	utils_dfs_reg_update_nol_history_chan_for_freq(dfs->dfs_pdev_obj,
						     (void *)chan_list, nchans,
						     DFS_NOL_HISTORY_RESET);

	qdf_mem_free(chan_list);
}
#else
#ifdef CONFIG_CHAN_NUM_API
void dfs_clear_nolhistory(struct wlan_dfs *dfs)
{
	struct dfs_channel *chan_list;
	int nchans = 0;
	bool sta_opmode;

	if (!dfs->dfs_is_stadfs_enabled)
		return;

	sta_opmode = dfs_mlme_is_opmode_sta(dfs->dfs_pdev_obj);
	if (!sta_opmode)
		return;

	nchans = dfs_get_num_chans();

	chan_list = qdf_mem_malloc(nchans * sizeof(*chan_list));
	if (!chan_list)
		return;

	utils_dfs_get_nol_history_chan_list(dfs->dfs_pdev_obj,
					    (void *)chan_list, &nchans);
	if (!nchans) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "zero chans");
		qdf_mem_free(chan_list);
		return;
	}

	utils_dfs_reg_update_nol_history_ch(dfs->dfs_pdev_obj,
					    (void *)chan_list, nchans,
					    DFS_NOL_HISTORY_RESET);

	qdf_mem_free(chan_list);
}
#endif
#endif

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST) && \
	defined(CONFIG_CHAN_FREQ_API)
void dfs_remove_spoof_channel_from_nol(struct wlan_dfs *dfs)
{
	struct dfs_nolelem *nol;
	uint16_t freq_list[NUM_CHANNELS_160MHZ];
	int i, nchans = 0;

	nchans = dfs_get_bonding_channels_for_freq(dfs,
						   &dfs->dfs_radar_found_chan,
						   SEG_ID_PRIMARY,
						   DETECTOR_ID_0,
						   freq_list);

	WLAN_DFSNOL_LOCK(dfs);
	for (i = 0; i < nchans && i < NUM_CHANNELS_160MHZ; i++) {
		nol = dfs->dfs_nol;
		while (nol) {
			if (nol->nol_freq == freq_list[i]) {
				OS_SET_TIMER(&nol->nol_timer, 0);
				break;
			}
			nol = nol->nol_next;
		}
	}
	WLAN_DFSNOL_UNLOCK(dfs);

	utils_dfs_reg_update_nol_chan_for_freq(dfs->dfs_pdev_obj,
					     freq_list, nchans, DFS_NOL_RESET);
}
#else
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST) && \
	defined(CONFIG_CHAN_NUM_API)
void dfs_remove_spoof_channel_from_nol(struct wlan_dfs *dfs)
{
	struct dfs_nolelem *nol;
	uint8_t channels[NUM_CHANNELS_160MHZ];
	int i, nchans = 0;

	nchans = dfs_get_bonding_channels(dfs,
					  &dfs->dfs_radar_found_chan,
					  SEG_ID_PRIMARY,
					  DETECTOR_ID_0,
					  channels);

	WLAN_DFSNOL_LOCK(dfs);
	for (i = 0; i < nchans && i < NUM_CHANNELS_160MHZ; i++) {
		nol = dfs->dfs_nol;
		while (nol) {
			if (nol->nol_freq == (uint16_t)utils_dfs_chan_to_freq(
				    channels[i])) {
				OS_SET_TIMER(&nol->nol_timer, 0);
				break;
			}
			nol = nol->nol_next;
		}
	}
	WLAN_DFSNOL_UNLOCK(dfs);

	utils_dfs_reg_update_nol_ch(dfs->dfs_pdev_obj,
				    channels, nchans, DFS_NOL_RESET);
}
#endif
#endif

void dfs_init_tmp_psoc_nol(struct wlan_dfs *dfs, uint8_t num_radios)
{
	struct dfs_soc_priv_obj *dfs_soc_obj = dfs->dfs_soc_obj;

	if (WLAN_UMAC_MAX_PDEVS < num_radios) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"num_radios (%u) exceeds limit", num_radios);
		return;
	}

	/* Allocate the temporary psoc NOL copy structure for the number
	 * of radios provided.
	 */
	dfs_soc_obj->dfs_psoc_nolinfo =
		qdf_mem_malloc(sizeof(struct dfsreq_nolinfo) * num_radios);
}

void dfs_deinit_tmp_psoc_nol(struct wlan_dfs *dfs)
{
	struct dfs_soc_priv_obj *dfs_soc_obj = dfs->dfs_soc_obj;

	if (!dfs_soc_obj->dfs_psoc_nolinfo)
		return;

	qdf_mem_free(dfs_soc_obj->dfs_psoc_nolinfo);
	dfs_soc_obj->dfs_psoc_nolinfo = NULL;
}

void dfs_save_dfs_nol_in_psoc(struct wlan_dfs *dfs,
			      uint8_t pdev_id,
			      uint16_t low_5ghz_freq,
			      uint16_t high_5ghz_freq)
{
	struct dfs_soc_priv_obj *dfs_soc_obj = dfs->dfs_soc_obj;
	struct dfsreq_nolinfo tmp_nolinfo, *nolinfo;
	uint32_t i, num_chans = 0;
	uint16_t tmp_freq;

	if (!dfs->dfs_nol_count)
		return;

	if (!dfs_soc_obj->dfs_psoc_nolinfo)
		return;

	nolinfo = &dfs_soc_obj->dfs_psoc_nolinfo[pdev_id];
	/* Fetch the NOL entries for the DFS object. */
	dfs_getnol(dfs, &tmp_nolinfo);

	/* nolinfo might already have some data. Do not overwrite it */
	num_chans = nolinfo->dfs_ch_nchans;
	for (i = 0; i < tmp_nolinfo.dfs_ch_nchans; i++) {
		tmp_freq = tmp_nolinfo.dfs_nol[i].nol_freq;

		/* Add to nolinfo only if within the pdev's frequency range. */
		if ((low_5ghz_freq < tmp_freq) && (high_5ghz_freq > tmp_freq)) {
			/* Figure out the completed duration of each NOL. */
			uint32_t nol_completed_ms =
				qdf_system_ticks_to_msecs(qdf_system_ticks() -
				tmp_nolinfo.dfs_nol[i].nol_start_ticks);

			nolinfo->dfs_nol[num_chans] = tmp_nolinfo.dfs_nol[i];
			/* Remember the remaining NOL time in the timeout
			 * variable.
			 */
			nolinfo->dfs_nol[num_chans++].nol_timeout_ms -=
				nol_completed_ms;
		}
	}

	nolinfo->dfs_ch_nchans = num_chans;
}

void dfs_reinit_nol_from_psoc_copy(struct wlan_dfs *dfs, uint8_t pdev_id)
{
	struct dfs_soc_priv_obj *dfs_soc_obj = dfs->dfs_soc_obj;
	struct dfsreq_nolinfo *nol;
	uint8_t i;

	if (!dfs_soc_obj->dfs_psoc_nolinfo)
		return;

	if (!dfs_soc_obj->dfs_psoc_nolinfo[pdev_id].dfs_ch_nchans)
		return;

	nol = &dfs_soc_obj->dfs_psoc_nolinfo[pdev_id];

	/* The NOL timeout value in each entry points to the remaining time
	 * of the NOL. This is to indicate that the NOL entries are paused
	 * and are not left to continue.
	 * While adding these NOL, update the start ticks to current time
	 * to avoid losing entries which might have timed out during
	 * the pause and resume mechanism.
	 */
	for (i = 0; i < nol->dfs_ch_nchans; i++)
		nol->dfs_nol[i].nol_start_ticks = qdf_system_ticks();
	dfs_set_nol(dfs, nol->dfs_nol, nol->dfs_ch_nchans);
}
