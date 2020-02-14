/*
 * Copyright (c) 2014-2020 The Linux Foundation. All rights reserved.
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

/**
 * DOC: reg_lte.c
 * This file contains the LTE feature APIs.
 */

#include <qdf_status.h>
#include <qdf_types.h>
#include <wlan_cmn.h>
#include "reg_services_public_struct.h"
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include "reg_services_common.h"
#include "reg_priv_objs.h"
#include "reg_build_chan_list.h"
#include "reg_callbacks.h"
#include "reg_lte.h"

#ifdef LTE_COEX
/**
 * reg_process_ch_avoid_freq() - Update unsafe frequencies in psoc_priv_obj
 * @psoc: pointer to psoc object
 * @pdev: pointer to pdev object
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS reg_process_ch_avoid_freq(struct wlan_objmgr_psoc *psoc,
					    struct wlan_objmgr_pdev *pdev)
{
	enum channel_enum ch_loop;
	enum channel_enum start_ch_idx;
	enum channel_enum end_ch_idx;
	uint8_t start_channel;
	uint8_t end_channel;
	uint32_t i;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct ch_avoid_freq_type *range;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < psoc_priv_obj->avoid_freq_list.ch_avoid_range_cnt;
		i++) {
		if (psoc_priv_obj->unsafe_chan_list.chan_cnt >= NUM_CHANNELS) {
			reg_warn("LTE Coex unsafe channel list full");
			break;
		}

		start_ch_idx = INVALID_CHANNEL;
		end_ch_idx = INVALID_CHANNEL;
		range = &psoc_priv_obj->avoid_freq_list.avoid_freq_range[i];

		start_channel = reg_freq_to_chan(pdev, range->start_freq);
		end_channel = reg_freq_to_chan(pdev, range->end_freq);
		reg_debug("start: freq %d, ch %d, end: freq %d, ch %d",
			  range->start_freq, start_channel, range->end_freq,
			  end_channel);

		/* do not process frequency bands that are not mapped to
		 * predefined channels
		 */
		if (start_channel == 0 || end_channel == 0)
			continue;

		for (ch_loop = 0; ch_loop < NUM_CHANNELS;
			ch_loop++) {
			if (REG_CH_TO_FREQ(ch_loop) >= range->start_freq) {
				start_ch_idx = ch_loop;
				break;
			}
		}
		for (ch_loop = 0; ch_loop < NUM_CHANNELS;
			ch_loop++) {
			if (REG_CH_TO_FREQ(ch_loop) >= range->end_freq) {
				end_ch_idx = ch_loop;
				if (REG_CH_TO_FREQ(ch_loop) > range->end_freq)
					end_ch_idx--;
				break;
			}
		}

		if (start_ch_idx == INVALID_CHANNEL ||
		    end_ch_idx == INVALID_CHANNEL)
			continue;

		for (ch_loop = start_ch_idx; ch_loop <= end_ch_idx;
			ch_loop++) {
			psoc_priv_obj->unsafe_chan_list.chan_freq_list[
				psoc_priv_obj->unsafe_chan_list.chan_cnt++] =
				REG_CH_TO_FREQ(ch_loop);
			if (psoc_priv_obj->unsafe_chan_list.chan_cnt >=
				NUM_CHANNELS) {
				reg_warn("LTECoex unsafe ch list full");
				break;
			}
		}
	}

	if (!psoc_priv_obj->unsafe_chan_list.chan_cnt)
		return QDF_STATUS_SUCCESS;

	for (ch_loop = 0; ch_loop < psoc_priv_obj->unsafe_chan_list.chan_cnt;
		ch_loop++) {
		if (ch_loop >= NUM_CHANNELS)
			break;
		reg_debug("Unsafe freq %d",
			  psoc_priv_obj->unsafe_chan_list.chan_freq_list[ch_loop]);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_update_unsafe_ch() - Updates unsafe channels in current channel list
 * @psoc: Pointer to psoc structure
 * @object: Pointer to pdev structure
 * @arg: List of arguments
 *
 * Return: None
 */
static void reg_update_unsafe_ch(struct wlan_objmgr_psoc *psoc,
				 void *object, void *arg)
{
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)object;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	QDF_STATUS status;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	if (psoc_priv_obj->ch_avoid_ind) {
		status = reg_process_ch_avoid_freq(psoc, pdev);
		if (QDF_IS_STATUS_ERROR(status))
			psoc_priv_obj->ch_avoid_ind = false;
	}

	reg_compute_pdev_current_chan_list(pdev_priv_obj);
	status = reg_send_scheduler_msg_nb(psoc, pdev);

	if (QDF_IS_STATUS_ERROR(status))
		reg_err("channel change msg schedule failed");
}

QDF_STATUS reg_process_ch_avoid_event(struct wlan_objmgr_psoc *psoc,
				      struct ch_avoid_ind_type *ch_avoid_event)
{
	uint32_t i;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	QDF_STATUS status;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	if (CH_AVOID_RULE_DO_NOT_RESTART ==
	    psoc_priv_obj->restart_beaconing) {
		reg_debug("skipping all LTE Coex unsafe channel range");
		return QDF_STATUS_SUCCESS;
	}
	/* Make unsafe channel list */
	reg_debug("band count %d", ch_avoid_event->ch_avoid_range_cnt);

	/* generate vendor specific event */
	qdf_mem_zero(&psoc_priv_obj->avoid_freq_list,
		     sizeof(struct ch_avoid_ind_type));
	qdf_mem_zero(&psoc_priv_obj->unsafe_chan_list,
		     sizeof(struct unsafe_ch_list));

	for (i = 0; i < ch_avoid_event->ch_avoid_range_cnt; i++) {
		if ((CH_AVOID_RULE_RESTART_24G_ONLY ==
				psoc_priv_obj->restart_beaconing) &&
			REG_IS_5GHZ_FREQ(ch_avoid_event->
				avoid_freq_range[i].start_freq)) {
			reg_debug(
				  "skipping 5Ghz LTE Coex unsafe channel range");
			continue;
		}
		psoc_priv_obj->avoid_freq_list.avoid_freq_range[i].start_freq =
			ch_avoid_event->avoid_freq_range[i].start_freq;
		psoc_priv_obj->avoid_freq_list.avoid_freq_range[i].end_freq =
			ch_avoid_event->avoid_freq_range[i].end_freq;
	}
	psoc_priv_obj->avoid_freq_list.ch_avoid_range_cnt =
		ch_avoid_event->ch_avoid_range_cnt;

	psoc_priv_obj->ch_avoid_ind = true;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_NB_ID);

	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}

	status = wlan_objmgr_iterate_obj_list(
			psoc, WLAN_PDEV_OP, reg_update_unsafe_ch, NULL, 1,
			WLAN_REGULATORY_NB_ID);

	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);

	return status;
}
#endif
