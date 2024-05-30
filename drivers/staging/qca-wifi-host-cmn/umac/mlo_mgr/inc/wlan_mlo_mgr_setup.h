/* Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: contains MLO manager public file containing setup/teardown functionality
 */

#ifdef WLAN_MLO_MULTI_CHIP
/**
 * mlo_setup_update_total_socs() - API to update total socs for mlo
 * @tot_socs: Total socs
 *
 * Return: None.
 */
void mlo_setup_update_total_socs(uint8_t tot_socs);

/**
 * mlo_setup_update_num_links() - API to update num links in soc for mlo
 * @soc_id: soc object of SoC corresponding to num_link
 * @num_links: Number of links in that soc
 *
 * Return: None.
 */
void mlo_setup_update_num_links(struct wlan_objmgr_psoc *psoc,
				uint8_t num_links);

/**
 * mlo_setup_update_soc_ready() - API to notify when FW init done
 * @psoc: soc object of SoC ready
 *
 * Return: None.
 */
void mlo_setup_update_soc_ready(struct wlan_objmgr_psoc *psoc);

/**
 * mlo_setup_link_ready() - API to notify link ready
 * @pdev: Pointer to pdev object
 *
 * Return: None.
 */
void mlo_setup_link_ready(struct wlan_objmgr_pdev *pdev);

/**
 * mlo_link_setup_complete() - API to notify setup complete
 * @pdev: Pointer to pdev object
 *
 * Return: None.
 */
void mlo_link_setup_complete(struct wlan_objmgr_pdev *pdev);

/**
 * mlo_link_teardown_complete() - API to notify teardown complete
 * @pdev: Pointer to pdev object
 *
 * Return: None.
 */
void mlo_link_teardown_complete(struct wlan_objmgr_pdev *pdev);
#endif
