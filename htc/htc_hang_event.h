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
#ifndef HTC_HANG_EVENT_H
#define HTC_HANG_EVENT_H

#include "htc_internal.h"

#ifdef WLAN_HANG_EVENT
/**
 * htc_hang_event_notifier_register() - HTC hang event notifier register
 * @target: Target specific htc hangle
 *
 * This function registers htc layer notifier for the hang event notifier chain.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS htc_hang_event_notifier_register(HTC_TARGET *target);

/**
 * htc_hang_event_notifier_unregister() - htc hang event notifier unregister
 *
 * This function unregisters htc layer notifier for the hang event notifier
 * chain.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS htc_hang_event_notifier_unregister(void);
#else
static inline QDF_STATUS htc_hang_event_notifier_register(HTC_TARGET *target)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS htc_hang_event_notifier_unregister(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif
