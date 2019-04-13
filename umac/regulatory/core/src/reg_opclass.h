/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: reg_opclass.h
 * This file provides prototypes of the regulatory opclass functions
 */

#ifndef __REG_OPCLASS_H__
#define __REG_OPCLASS_H__

#ifdef HOST_OPCLASS
/**
 * reg_dmn_get_chanwidth_from_opclass() - Get channel width from opclass.
 * @country: Country code
 * @channel: Channel number
 * @opclass: Operating class
 *
 * Return: Channel width
 */
uint16_t reg_dmn_get_chanwidth_from_opclass(uint8_t *country, uint8_t channel,
					    uint8_t opclass);

/**
 * reg_dmn_get_opclass_from_channel() - Get operating class from channel.
 * @country: Country code.
 * @channel: Channel number.
 * @offset: Operating class offset.
 *
 * Return: Error code.
 */
uint16_t reg_dmn_get_opclass_from_channel(uint8_t *country, uint8_t channel,
					  uint8_t offset);
/**
 * reg_dmn_get_opclass_from_channe() - Print channels in op class.
 * @country: Country code.
 * @opclass: opclass.
 *
 * Return: Void.
 */
void reg_dmn_print_channels_in_opclass(uint8_t *country, uint8_t op_class);

/**
 * reg_dmn_set_curr_opclasses() - Set current operating class
 * @num_classes: Number of classes
 * @class: Pointer to operating class.
 *
 * Return: Error code.
 */
uint16_t reg_dmn_set_curr_opclasses(uint8_t num_classes, uint8_t *class);

/**
 * reg_dmn_get_curr_opclasses() - Get current supported operating classes.
 * @num_classes: Number of classes.
 * @class: Pointer to operating class.
 *
 * Return: Error code.
 */
uint16_t reg_dmn_get_curr_opclasses(uint8_t *num_classes, uint8_t *class);

#else

static inline uint16_t reg_dmn_get_chanwidth_from_opclass(
		uint8_t *country, uint8_t channel, uint8_t opclass)
{
	return 0;
}

static inline uint16_t reg_dmn_set_curr_opclasses(
		uint8_t num_classes, uint8_t *class)
{
	return 0;
}

static inline uint16_t reg_dmn_get_curr_opclasses(
		uint8_t *num_classes, uint8_t *class)
{
	return 0;
}

static inline uint16_t reg_dmn_get_opclass_from_channel(
		uint8_t *country, uint8_t channel, uint8_t offset)
{
	return 0;
}

static inline void reg_dmn_print_channels_in_opclass(uint8_t *country,
						     uint8_t op_class)
{
}

#endif
#endif
