/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC : osif_twt_internal.h
 *
 * WLAN Host Device Driver file for TWT (Target Wake Time) support.
 *
 */

#ifndef _OSIF_TWT_INTERNAL_H_
#define _OSIF_TWT_INTERNAL_H_

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
/**
 * struct twt_conc_context: TWT concurrency args
 * @psoc: pointer to psoc
 */
struct twt_conc_context {
	struct wlan_objmgr_psoc *psoc;
};

/**
 * osif_twt_send_requestor_enable_cmd() - Send TWT requestor enable command to
 * target
 * @psoc: pointer to global psoc structure
 * @pdev_id: pdev id
 *
 * Return: errno
 */
int osif_twt_send_requestor_enable_cmd(struct wlan_objmgr_psoc *psoc,
				       uint8_t pdev_id);

/**
 * osif_twt_send_responder_enable_cmd() - Send TWT responder enable command to
 * target
 * @psoc: pointer to global psoc structure
 * @pdev_id: pdev id
 *
 * Return: errno
 */
int osif_twt_send_responder_enable_cmd(struct wlan_objmgr_psoc *psoc,
				       uint8_t pdev_id);

/**
 * osif_twt_send_requestor_disable_cmd() - Send TWT requestor disable command
 * to target
 * @psoc: pointer to global psoc structure
 * @pdev_id: pdev id
 * @reason: disable reason code
 *
 * Return: errno
 */
int osif_twt_send_requestor_disable_cmd(struct wlan_objmgr_psoc *psoc,
					uint8_t pdev_id, uint32_t reason);

/**
 * osif_twt_send_responder_disable_cmd() - Send TWT responder disable command
 * to target
 * @psoc: pointer to global psoc structure
 * @pdev_id: pdev id
 * @reason: disable reason code
 *
 * Return: errno
 */
int osif_twt_send_responder_disable_cmd(struct wlan_objmgr_psoc *psoc,
					uint8_t pdev_id, uint32_t reason);

/**
 * osif_twt_concurrency_update_handler() - Handle TWT concurrency scenario
 * @psoc: pointer to global psoc structure
 * @pdev: pointer to pdev
 *
 * Return: none
 */
void osif_twt_concurrency_update_handler(struct wlan_objmgr_psoc *psoc,
					 struct wlan_objmgr_pdev *pdev);

/**
 * osif_twt_teardown_in_ps_disable() - Send TWT teardown if power save
 * mode is disabled
 *
 * @psoc: pointer to global psoc structure
 * @mac_addr: Peer MAC Address
 * @vdev_id: vdev_id
 */
void osif_twt_teardown_in_ps_disable(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr *mac_addr,
				     uint8_t vdev_id);
#else
static inline
int osif_twt_send_requestor_disable_cmd(struct wlan_objmgr_psoc *psoc,
					uint8_t pdev_id, uint32_t reason)
{
	return 0;
}

static inline
int osif_twt_send_requestor_enable_cmd(struct wlan_objmgr_psoc *psoc,
				       uint8_t pdev_id)
{
	return 0;
}

static inline
int osif_twt_send_responder_enable_cmd(struct wlan_objmgr_psoc *psoc,
				       uint8_t pdev_id)
{
	return 0;
}

static inline
void osif_twt_concurrency_update_handler(struct wlan_objmgr_psoc *psoc,
					 struct wlan_objmgr_pdev *pdev)
{
}
static inline
void osif_twt_teardown_in_ps_disable(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr *mac_addr,
				     uint8_t vdev_id)
{
}
#endif
#endif /* _OSIF_TWT_INTERNAL_H_ */

