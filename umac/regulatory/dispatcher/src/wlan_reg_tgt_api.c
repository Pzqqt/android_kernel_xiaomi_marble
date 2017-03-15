/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * @file wlan_req_tgt_api.c
 * @brief contains regulatory target interface definations
 */

#include "wlan_reg_tgt_api.h"
/**
 * tgt_reg_country_code_update_handler() - tgt country code update handler
 * @handle: pointer to the tgt handle
 * @event: pointer to the event msg
 * @len: event msg length
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_reg_country_code_update_handler(void *handle, uint8_t *event,
	uint32_t len)
{
	/*
	 * Convert the tlv/non tlv data to struct coutry_code_update
	 * and Post msg to target_if queue
	 */
	return 0;
}

/**
 * tgt_reg_country_code_update_handler() - tgt country code update handler
 * @handle: pointer to the tgt handle
 * @event: pointer to the event msg
 * @len: event msg length
 *
 * Return: QDF_STATUS
 */
QDF_STATUS tgt_reg_ch_list_update_handler(void *handle, uint8_t *event,
	uint32_t len)
{
	/*
	 * Convert the tlv/non tlv data to struct ch_list_update
	 * and Post msg to target_if queue
	 */
	return 0;
}
