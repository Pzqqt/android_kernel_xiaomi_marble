/*
 * Copyright (c) 2014-2016, 2018 The Linux Foundation. All rights reserved.
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
 *
 * Name:  nan_api.h
 *
 * Description: NAN FSM defines.
 *
 */

#ifndef __NAN_API_H__
#define __NAN_API_H__

#include "qdf_types.h"
#include "sir_types.h"

typedef void (*nan_callback)(hdd_handle_t hdd_handle, tSirNanEvent *event);

#ifdef WLAN_FEATURE_NAN
typedef struct sNanRequestReq {
	uint16_t request_data_len;
	const uint8_t *request_data;
} tNanRequestReq, *tpNanRequestReq;

/**
 * sme_nan_register_callback() - Register NAN Callback function
 * @mac_handle: Opaque handle to the MAC context
 * @callback: Callback function to be registered
 *
 * Function to be called by HDD to register the NAN Response callback
 * function with the sme layer.
 *
 * Return: void
 */
void sme_nan_register_callback(mac_handle_t mac_handle, nan_callback callback);

/**
 * sme_nan_deregister_callback() - NAN De-register cb function
 * @mac_handle: Opaque handle to the MAC context
 *
 * Function to be called by HDD to deregister the NAN Response
 * callback function with the sme layer.
 *
 * Return: void
 */
void sme_nan_deregister_callback(mac_handle_t mac_handle);

/**
 * sme_nan_request() - Process a NAN request
 * @input: Nan Request structure ptr
 *
 * This function gets called when HDD receives NAN vendor command
 * from userspace
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_nan_request(tpNanRequestReq input);

/**
 * sme_nan_event() - Process a NAN event
 * @mac_handle: Opaque handle to the MAC context
 * @event: NAN event received from firmware
 *
 * This callback function will be called when SME received eWNI_SME_NAN_EVENT
 * event from WMA
 *
 * Return: void
 */
void sme_nan_event(mac_handle_t mac_handle, tSirNanEvent *event);

#else /* WLAN_FEATURE_NAN */

static inline void sme_nan_register_callback(mac_handle_t mac_handle,
					     nan_callback callback)
{
}

static inline void sme_nan_deregister_callback(mac_handle_t mac_handle)
{
}

#endif /* WLAN_FEATURE_NAN */

#endif /* __NAN_API_H__ */
