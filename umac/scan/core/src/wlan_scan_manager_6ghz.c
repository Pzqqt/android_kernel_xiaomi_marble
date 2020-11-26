/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: contains 6ghz scan manager functionality
 */

#include "wlan_scan_main.h"
#include "wlan_utility.h"
#include <wlan_reg_services_api.h>
#include "wlan_scan_manager.h"

/* Beacon/probe weightage multiplier */
#define BCN_PROBE_WEIGHTAGE 5

/* maximum number of 6ghz hints can be sent per scan request */
#define MAX_HINTS_PER_SCAN_REQ 15

/* Saved profile weightage multiplier */
#define SAVED_PROFILE_WEIGHTAGE 10

#ifdef FEATURE_6G_SCAN_CHAN_SORT_ALGO

/**
 * scm_sort_6ghz_channel_list() - Sort the 6ghz channels based on weightage
 * @vdev: vdev on which scan request is issued
 * @chan_list: channel info of the scan request
 *
 * Calculate weightage of each channel based on beacon weightage and saved
 * profile weightage. Sort the channels based on this weight in descending order
 * to scan the most preferred channels first compared other 6ghz channels.
 *
 * Return: None
 */
static void
scm_sort_6ghz_channel_list(struct wlan_objmgr_vdev *vdev,
			   struct chan_list *chan_list)
{
	uint8_t i, j = 0, max, tmp_list_count;
	struct meta_rnr_channel *channel;
	struct chan_info temp_list[MAX_6GHZ_CHANNEL];
	struct rnr_chan_weight *rnr_chan_info, *temp;
	uint32_t weight;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		scm_err("Psoc is NULL");
		return;
	}

	for (i = 0; i < chan_list->num_chan; i++)
		if (WLAN_REG_IS_6GHZ_CHAN_FREQ(chan_list->chan[i].freq))
			temp_list[j++].freq = chan_list->chan[i].freq;

	tmp_list_count = j;
	scm_debug("Total 6ghz channels %d", tmp_list_count);

	/* No Need to sort if the 6ghz channels are less than or one */
	if (tmp_list_count <= 1)
		return;

	rnr_chan_info = qdf_mem_malloc(sizeof(*rnr_chan_info) * tmp_list_count);
	if (!rnr_chan_info)
		return;

	/* compute the weightage */
	for (i = 0, j = 0; i < tmp_list_count; i++) {
		channel = scm_get_chan_meta(psoc, temp_list[i].freq);
		if (!channel)
			continue;
		weight = channel->bss_beacon_probe_count * BCN_PROBE_WEIGHTAGE +
			 channel->saved_profile_count * SAVED_PROFILE_WEIGHTAGE;
		rnr_chan_info[j].weight = weight;
		rnr_chan_info[j].chan_freq = temp_list[i].freq;
		j++;
		/*
		 * Log the info only if weight or bss_beacon_probe_count are
		 * non-zero to avoid excessive logging.
		 */
		if (weight || channel->bss_beacon_probe_count)
			scm_debug("Freq %d weight %d bcn_cnt %d",
				  temp_list[i].freq, weight,
				  channel->bss_beacon_probe_count);
	}

	/* Sort the channel using selection sort - descending order */
	for (i = 0; i < tmp_list_count - 1; i++) {
		max = i;
		for (j = i + 1; j < tmp_list_count; j++) {
			if (rnr_chan_info[j].weight >
			    rnr_chan_info[max].weight)
				max = j;
		}
		if (max != i) {
			qdf_mem_copy(&temp, &rnr_chan_info[max],
				     sizeof(*rnr_chan_info));
			qdf_mem_copy(&rnr_chan_info[max], &rnr_chan_info[i],
				     sizeof(*rnr_chan_info));
			qdf_mem_copy(&rnr_chan_info[i], &temp,
				     sizeof(*rnr_chan_info));
		}
	}

	/* update the 6g list based on the weightage */
	for (i = 0, j = 0; (i < NUM_CHANNELS && j < tmp_list_count); i++)
		if (wlan_reg_is_6ghz_chan_freq(chan_list->chan[i].freq))
			chan_list->chan[i].freq = rnr_chan_info[j++].chan_freq;

	qdf_mem_free(rnr_chan_info);
}

static void scm_update_rnr_info(struct wlan_objmgr_psoc *psoc,
				struct scan_start_request *req)
{
	uint8_t i, num_bssid = 0, num_ssid = 0;
	uint8_t total_count = MAX_HINTS_PER_SCAN_REQ;
	uint32_t freq;
	struct meta_rnr_channel *chan;
	qdf_list_node_t *cur_node, *next_node = NULL;
	struct scan_rnr_node *rnr_node;
	struct chan_list *chan_list;
	QDF_STATUS status;

	if (!req)
		return;

	chan_list = &req->scan_req.chan_list;
	for (i = 0; i < chan_list->num_chan; i++) {
		freq = chan_list->chan[i].freq;

		chan = scm_get_chan_meta(psoc, freq);
		if (!chan || qdf_list_empty(&chan->rnr_list))
			continue;

		qdf_list_peek_front(&chan->rnr_list, &cur_node);
		while (cur_node && total_count) {
			rnr_node = qdf_container_of(cur_node,
						    struct scan_rnr_node,
						    node);
			if (!qdf_is_macaddr_zero(&rnr_node->entry.bssid) &&
			    req->scan_req.num_hint_bssid <
			    WLAN_SCAN_MAX_HINT_BSSID) {
				qdf_mem_copy(&req->scan_req.hint_bssid[
							num_bssid++].bssid,
					     &rnr_node->entry.bssid,
					     QDF_MAC_ADDR_SIZE);
				req->scan_req.num_hint_bssid++;
				total_count--;
			} else if (rnr_node->entry.short_ssid &&
				   req->scan_req.num_hint_s_ssid <
				   WLAN_SCAN_MAX_HINT_S_SSID) {
				req->scan_req.hint_s_ssid[
					num_ssid++].short_ssid =
						rnr_node->entry.short_ssid;
				req->scan_req.num_hint_s_ssid++;
				total_count--;
			}
			status = qdf_list_peek_next(&chan->rnr_list, cur_node,
						    &next_node);
			if (QDF_IS_STATUS_ERROR(status))
				break;
			cur_node = next_node;
			next_node = NULL;
		}
	}
}

/**
 * scm_add_rnr_info() - Add the cached RNR info to scan request
 * @vdev: vdev on which scan request is issued
 * @req: Scan start request
 *
 * Fetch the cached RNR info from scan db and update it to the scan request to
 * include RNR channels in the scan request.
 *
 * Return: None
 */
static void scm_add_rnr_info(struct wlan_objmgr_pdev *pdev,
			     struct scan_start_request *req)
{
	struct wlan_objmgr_psoc *psoc;
	struct channel_list_db *rnr_db;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return;
	rnr_db = scm_get_rnr_channel_db(psoc);
	if (!rnr_db)
		return;

	rnr_db->scan_count++;
	if (rnr_db->scan_count >= RNR_UPDATE_SCAN_CNT_THRESHOLD) {
		rnr_db->scan_count = 0;
		scm_rnr_db_flush(psoc);
		scm_update_rnr_from_scan_cache(pdev);
	}

	scm_update_rnr_info(psoc, req);
}
#else
static void
scm_sort_6ghz_channel_list(struct wlan_objmgr_vdev *vdev,
			   struct chan_list *chan_list)
{
}

static void scm_add_rnr_info(struct wlan_objmgr_pdev *pdev,
			     struct scan_start_request *req)
{
}
#endif

static inline bool
scm_is_full_scan_by_userspace(struct chan_list *chan_list)
{
	return (chan_list->num_chan >= FULL_SCAN_CH_COUNT_MIN_BY_USERSPACE);
}

static void
scm_copy_valid_channels(struct wlan_objmgr_psoc *psoc,
			enum scan_mode_6ghz scan_mode,
			struct chan_list *chan_list,
			uint8_t *num_scan_ch)
{
	uint8_t i, num_ch = *num_scan_ch;
	qdf_freq_t freq;

	switch (scan_mode) {
	case SCAN_MODE_6G_NO_CHANNEL:
		/* Don't add any 6g channels */
		for (i = 0; i < chan_list->num_chan; i++)
			if (!wlan_reg_is_6ghz_chan_freq(
					chan_list->chan[i].freq))
				chan_list->chan[num_ch++] =
					chan_list->chan[i];
		break;
	case SCAN_MODE_6G_PSC_CHANNEL:
	case SCAN_MODE_6G_PSC_DUTY_CYCLE:
		/*
		 * Don't add non-PSC 6g channels if firmware doesn't
		 * support RNR_ONLY scan flag/feature.
		 */
		if (!scm_is_6ghz_scan_optimization_supported(psoc)) {
			for (i = 0; i < chan_list->num_chan; i++) {
				freq = chan_list->chan[i].freq;
				if (!wlan_reg_is_6ghz_chan_freq(freq) ||
				    (wlan_reg_is_6ghz_chan_freq(freq) &&
				     wlan_reg_is_6ghz_psc_chan_freq(freq)))
					chan_list->chan[num_ch++] =
						chan_list->chan[i];
			}
			break;
		}
		/*
		 * Consider the complete channel list if firmware supports
		 * RNR_ONLY scan flag/feature.
		 */
		/* fallthrough */
	default:
		/*
		 * Allow all 2g/5g/6g channels. Below are also covered in this
		 * 1. SCAN_MODE_6G_ALL_CHANNEL: Copy all channels and RNR flag
		 *    won't be set for any channel.
		 * 2. SCAN_MODE_6G_PSC_CHANNEL: Copy all channels and RNR flag
		 *    will be set for non-PSC.
		 * 3. SCAN_MODE_6G_PSC_DUTY_CYCLE: Copy all channels and RNR
		 *    flag will be set for non-PSC for all scans and RNR flag
		 *    will be set for PSC channels only for duty cycle scan.
		 */
		num_ch = chan_list->num_chan;
	}

	*num_scan_ch = num_ch;
}

static inline void
scm_set_rnr_flag_non_psc_6g_ch(struct chan_info *chan, uint8_t num_chan)
{
	uint8_t i;

	for (i = 0; i < num_chan; i++)
		if (wlan_reg_is_6ghz_chan_freq(chan[i].freq) &&
		    !wlan_reg_is_6ghz_psc_chan_freq(chan[i].freq))
			chan[i].flags = FLAG_SCAN_ONLY_IF_RNR_FOUND;
}

static inline void
scm_set_rnr_flag_all_6g_ch(struct chan_info *chan, uint8_t num_chan)
{
	uint8_t i;

	for (i = 0; i < num_chan; i++)
		if (wlan_reg_is_6ghz_chan_freq(chan[i].freq))
			chan[i].flags = FLAG_SCAN_ONLY_IF_RNR_FOUND;
}

static bool scm_is_duty_cycle_scan(struct wlan_scan_obj *scan_obj)
{
	bool duty_cycle = false;

	scan_obj->duty_cycle_cnt_6ghz++;
	if (scan_obj->duty_cycle_cnt_6ghz ==
		scan_obj->scan_def.duty_cycle_6ghz) {
		duty_cycle = true;
		scan_obj->duty_cycle_cnt_6ghz = 0;
	}

	return duty_cycle;
}

inline bool
scm_is_6ghz_scan_optimization_supported(struct wlan_objmgr_psoc *psoc)
{
	return wlan_psoc_nif_fw_ext_cap_get(psoc,
					    WLAN_SOC_CEXT_SCAN_PER_CH_CONFIG);
}

void
scm_update_6ghz_channel_list(struct scan_start_request *req,
			     struct wlan_scan_obj *scan_obj)
{
	struct wlan_objmgr_vdev *vdev = req->vdev;
	struct wlan_objmgr_pdev *pdev;
	struct chan_list *chan_list = &req->scan_req.chan_list;
	enum scan_mode_6ghz scan_mode;
	uint8_t num_scan_ch = 0;
	enum QDF_OPMODE op_mode;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return;

	/* Dont update the channel list for not STA mode */
	op_mode = wlan_vdev_mlme_get_opmode(req->vdev);
	if (op_mode == QDF_SAP_MODE ||
	    op_mode == QDF_P2P_DEVICE_MODE ||
	    op_mode == QDF_P2P_CLIENT_MODE ||
	    op_mode == QDF_P2P_GO_MODE)
		return;

	scan_mode = scan_obj->scan_def.scan_mode_6g;
	scm_debug("6g scan mode %d", scan_mode);

	/*
	 * Host has learned RNR info/channels from previous scan. Add them to
	 * the scan request and don't set RNR_ONLY flag to scan them without
	 * optimization.
	 */
	if (scan_mode != SCAN_MODE_6G_NO_CHANNEL &&
	    scm_is_full_scan_by_userspace(chan_list))
		scm_add_rnr_info(pdev, req);

	/* copy all the channels given by userspace */
	scm_copy_valid_channels(wlan_pdev_get_psoc(pdev), scan_mode, chan_list,
				&num_scan_ch);

	/* No more optimizations are needed in the below cases */
	if (!scm_is_full_scan_by_userspace(chan_list) ||
	    !scm_is_6ghz_scan_optimization_supported(
				wlan_pdev_get_psoc(pdev)))
		goto end;

	switch (scan_mode) {
	case SCAN_MODE_6G_RNR_ONLY:
		/*
		 * When the ini is set to SCAN_MODE_6G_RNR_ONLY,
		 * always set RNR flag for all(PSC and non-PSC) channels.
		 */
		scm_set_rnr_flag_all_6g_ch(&chan_list->chan[0], num_scan_ch);
		break;
	case SCAN_MODE_6G_PSC_CHANNEL:
		/*
		 * When the ini is set to SCAN_MODE_6G_PSC_CHANNEL,
		 * always set RNR flag for non-PSC channels.
		 */
		scm_set_rnr_flag_non_psc_6g_ch(&chan_list->chan[0],
					       num_scan_ch);
		break;
	case SCAN_MODE_6G_PSC_DUTY_CYCLE:
	case SCAN_MODE_6G_ALL_DUTY_CYCLE:
		if (!scm_is_duty_cycle_scan(scan_obj))
			scm_set_rnr_flag_all_6g_ch(&chan_list->chan[0],
						   num_scan_ch);
		else if (scan_mode == SCAN_MODE_6G_PSC_DUTY_CYCLE)
			scm_set_rnr_flag_non_psc_6g_ch(&chan_list->chan[0],
						       num_scan_ch);
		break;
	default:
		/*
		 * Don't set the RNR flag for SCAN_MODE_6G_NO_CHANNEL/
		 * SCAN_MODE_6G_RNR_ONLY
		 */
		break;
	}

end:
	chan_list->num_chan = num_scan_ch;

	scm_sort_6ghz_channel_list(req->vdev, &req->scan_req.chan_list);
}
