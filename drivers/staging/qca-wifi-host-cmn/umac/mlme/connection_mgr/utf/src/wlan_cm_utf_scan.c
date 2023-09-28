/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

/*
 * DOC: Implements scan functinality for CM UTF
 */

#ifdef FEATURE_CM_UTF_ENABLE
#include <wlan_cm_utf.h>
#include <qdf_str.h>

struct wlan_cm_utf_raw_bcn {
	uint32_t channel_number;
	int32_t rssi;
	uint8_t band;
	uint8_t bssid[QDF_MAC_ADDR_SIZE];
	uint8_t ssid[WLAN_SSID_MAX_LEN];
};

static void wlan_cm_utf_scan_db_update(struct wlan_objmgr_vdev *vdev,
				       void *buffer)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_cm_utf_raw_bcn *event = NULL;
	struct mgmt_rx_event_params *rx_param = NULL;
	struct wlan_objmgr_psoc *psoc;
	char *buff = (char *)buffer;
	char *token;
	uint32_t frame_len = 0;
	qdf_nbuf_t buf;
	struct ie_ssid ssid;
	struct ie_header *ie;
	struct wlan_frame_hdr *hdr;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("Pdev is Null");
		return;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		mlme_err("Psoc is Null");
		return;
	}

	while ((token = qdf_str_sep(&buff, "\n")) != NULL) {
		mlme_err("%s", token);
		event = qdf_mem_malloc(sizeof(struct wlan_cm_utf_raw_bcn));
		if (!event) {
			mlme_err("Failed to allocate event memory");
			return;
		}
		if (sscanf(token,
			   "%2x:%2x:%2x:%2x:%2x:%2x ,%u ,%u ,%u ,%s",
			   (unsigned int *)&event->bssid[0],
			   (unsigned int *)&event->bssid[1],
			   (unsigned int *)&event->bssid[2],
			   (unsigned int *)&event->bssid[3],
			   (unsigned int *)&event->bssid[4],
			   (unsigned int *)&event->bssid[5],
			   &event->channel_number,
			   (unsigned int *)&event->band,
			   &event->rssi,
			   event->ssid) != 10) {
			goto free_buf;
		}
		ssid.ssid_id = 0;	//Element id for ssid
		ssid.ssid_len = strlen(event->ssid);
		qdf_mem_copy(ssid.ssid, event->ssid, strlen(event->ssid));

		rx_param = qdf_mem_malloc(sizeof(struct mgmt_rx_event_params));
		if (!rx_param) {
			mlme_err("Failed to allocate memory");
			goto free_buf;
		}

		qdf_mem_zero(rx_param, sizeof(struct mgmt_rx_event_params));
		rx_param->snr = event->rssi;
		rx_param->channel = event->channel_number;
		rx_param->chan_freq = wlan_reg_chan_band_to_freq(
						pdev,
						event->channel_number,
						BIT(event->band));

		rx_param->pdev_id = 0;
		frame_len = sizeof(struct wlan_frame_hdr) +
			    sizeof(struct wlan_bcn_frame) -
			    sizeof(struct ie_header) +
			    sizeof(struct ie_header) + ssid.ssid_len;

		buf = qdf_nbuf_alloc(NULL, frame_len, 0, 0, FALSE);
		if (!buf) {
			mlme_err("Failed to allocate buffer");
			goto free_buf;
		}

		qdf_nbuf_set_pktlen(buf, frame_len);
		qdf_mem_zero((uint8_t *)qdf_nbuf_data(buf), frame_len);

		hdr = (struct wlan_frame_hdr *)qdf_nbuf_data(buf);
		qdf_mem_copy(hdr->i_addr3, event->bssid, QDF_MAC_ADDR_SIZE);
		qdf_mem_copy(hdr->i_addr2, event->bssid, QDF_MAC_ADDR_SIZE);
		ie = (struct ie_header *)(((uint8_t *)qdf_nbuf_data(buf))
				+ sizeof(struct wlan_frame_hdr)
				+ offsetof(struct wlan_bcn_frame, ie));

		qdf_mem_copy(ie, &ssid, sizeof(struct ie_ssid));
		tgt_scan_bcn_probe_rx_callback(psoc, NULL, buf,
					       rx_param, MGMT_BEACON);
free_buf:
		if (event) {
			qdf_mem_free(event);
			event = NULL;
		}
		if (rx_param) {
			qdf_mem_free(rx_param);
			rx_param = NULL;
		}
	}
}

int wlan_cm_utf_scan_db_update_show(qdf_debugfs_file_t m, void *v)
{
	return 0;
}

ssize_t wlan_cm_utf_scan_db_update_write(struct file *file,
					 const char __user *buf,
					 size_t count, loff_t *ppos)
{
	struct wlan_cm_utf *cm_utf =
			((struct seq_file *)file->private_data)->private;
	char *locbuf;

	if ((!buf) || (count <= 0))
		return -EFAULT;

	locbuf = (char *)qdf_mem_malloc(count);

	if (!locbuf)
		return -EFAULT;

	qdf_mem_zero(locbuf, count);

	if (copy_from_user(locbuf, buf, count))
		return -EFAULT;

	wlan_cm_utf_scan_db_update(cm_utf->vdev, locbuf);
	qdf_mem_free(locbuf);
	return count;
}
#endif
