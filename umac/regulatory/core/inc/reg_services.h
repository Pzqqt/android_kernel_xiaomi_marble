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
 * DOC: reg_services.h
 * This file provides prototypes of the regulatory component
 * service functions
 */

#include "qdf_types.h"
#include "qdf_trace.h"
#include "wlan_reg_services_api.h"

QDF_STATUS reg_get_channel_list_with_power(struct regulatory_channel *ch_list);
void reg_read_default_country(uint8_t *country);
enum channel_state reg_get_channel_state(uint8_t ch);
enum channel_state reg_get_5g_bonded_channel_state(uint8_t ch, uint8_t bw);
enum channel_state reg_get_2g_bonded_channel_state(uint8_t ch, uint8_t bw);
void reg_set_channel_params(uint8_t ch, struct ch_params *ch_params);
void reg_get_dfs_region(enum dfs_region *dfs_reg);
bool reg_is_dfs_ch(uint8_t ch);
