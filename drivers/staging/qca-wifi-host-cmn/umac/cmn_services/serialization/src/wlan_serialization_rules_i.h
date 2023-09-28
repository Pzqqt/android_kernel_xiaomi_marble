/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_serialization_rules_i.h
 * This file defines the prototypes for the rules related data
 * pertinent to the serialization component.
 */
#ifndef __WLAN_SERIALIZATION_RULES_I_H
#define __WLAN_SERIALIZATION_RULES_I_H

#include <qdf_types.h>
#include <wlan_serialization_api.h>

/**
 * wlan_serialization_apply_scan_rules() - apply scan rules callback
 * @info: rules info structure
 * @comp_id: component Identifier
 *
 * This callback is registered with object manager during initialization and
 * when serialization request is called by component, this callback handler
 * applies rules depending on component.
 * There will be many apply rules callback handlers in future
 *
 * Return: boolean
 */
bool
wlan_serialization_apply_scan_rules(
		union wlan_serialization_rules_info *info, uint8_t comp_id);
#endif
