/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: Declare various api/struct which shall be used
 * by packet capture component for wmi cmd (tx path) and
 * event (rx) handling.
 */

#ifndef _TARGET_IF_PKT_CAPTURE_H_
#define _TARGET_IF_PKT_CAPTURE_H_

#include <qdf_types.h>
#include "wlan_pkt_capture_public_structs.h"

/**
 * target_if_set_packet_capture_mode() - set packet capture mode
 * @psoc: pointer to psoc object
 * @vdev_id: vdev id
 * @mode: mode to set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS target_if_set_packet_capture_mode(struct wlan_objmgr_psoc *psoc,
					     uint8_t vdev_id,
					     enum pkt_capture_mode mode);
#endif /* _TARGET_IF_PKT_CAPTURE_H_ */
