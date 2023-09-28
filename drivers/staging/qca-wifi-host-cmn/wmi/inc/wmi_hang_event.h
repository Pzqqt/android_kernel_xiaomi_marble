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
#ifndef WMI_HANG_EVENT_H
#define WMI_HANG_EVENT_H

#include <wmi_unified_priv.h>
#ifdef WLAN_HANG_EVENT

/**
 * wmi_hang_event_notifier_register() - wmi hang event notifier register
 * @wmi_hdl: WMI Handle
 *
 * This function registers wmi layer notifier for the hang event notifier chain.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_hang_event_notifier_register(struct wmi_unified *wmi_hdl);

/**
 * wmi_hang_event_notifier_unregister() - wmi hang event notifier unregister
 * @wmi_hdl: WMI Handle
 *
 * This function unregisters wmi layer notifier for the hang event notifier
 * chain.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wmi_hang_event_notifier_unregister(void);
#else
static inline
QDF_STATUS wmi_hang_event_notifier_register(struct wmi_unified *wmi_hdl)
{
	return 0;
}

static inline QDF_STATUS wmi_hang_event_notifier_unregister(void)
{
	return 0;
}
#endif
#endif
