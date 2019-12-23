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
 * reg_dmn_get_opclass_from_freq_width() - Get operating class from frequency
 * @country: Country code.
 * @freq: Channel center frequency.
 * @ch_width: Channel width.
 * @behav_limit: Behaviour limit.
 *
 * Return: Error code.
 */
uint8_t reg_dmn_get_opclass_from_freq_width(uint8_t *country,
					    qdf_freq_t freq,
					    uint8_t ch_width,
					    uint16_t behav_limit);

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

#ifdef CONFIG_CHAN_FREQ_API

/**
 * reg_freq_width_to_chan_op_class() - convert frequency to oper class,
 *                                     channel
 * @pdev: pdev pointer
 * @freq: channel frequency in mhz
 * @chan_width: channel width
 * @global_tbl_lookup: whether to lookup global op class tbl
 * @behav_limit: behavior limit
 * @op_class: operating class
 * @chan_num: channel number
 *
 * Return: Void.
 */
void reg_freq_width_to_chan_op_class(struct wlan_objmgr_pdev *pdev,
				     qdf_freq_t freq,
				     uint16_t chan_width,
				     bool global_tbl_lookup,
				     uint16_t behav_limit,
				     uint8_t *op_class,
				     uint8_t *chan_num);

/**
 * reg_freq_to_chan_op_class() - convert frequency to oper class,
 *                                   channel
 * @pdev: pdev pointer
 * @freq: channel frequency in mhz
 * @global_tbl_lookup: whether to lookup global op class tbl
 * @behav_limit: behavior limit
 * @op_class: operating class
 * @chan_num: channel number
 *
 * Return: Void.
 */
void reg_freq_to_chan_op_class(struct wlan_objmgr_pdev *pdev,
			       qdf_freq_t freq,
			       bool global_tbl_lookup,
			       uint16_t behav_limit,
			       uint8_t *op_class,
			       uint8_t *chan_num);
#endif

/**
 * reg_get_op_class_width() - get oper class width
 *
 * @pdev: pdev pointer
 * @global_tbl_lookup: whether to lookup global op class tbl
 * @op_class: operating class
 * Return: uint16
 */
uint16_t reg_get_op_class_width(struct wlan_objmgr_pdev *pdev,
				uint8_t op_class,
				bool global_tbl_lookup);

/**
 * reg_chan_opclass_to_freq() - Convert channel number and opclass to frequency
 * @chan: IEEE Channel Number.
 * @op_class: Opclass.
 * @global_tbl_lookup: Global table lookup.
 *
 * Return: Channel center frequency else return 0.
 */
uint16_t reg_chan_opclass_to_freq(uint8_t chan,
				  uint8_t op_class,
				  bool global_tbl_lookup);
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

static inline
uint8_t reg_dmn_get_opclass_from_freq_width(uint8_t *country,
					    qdf_freq_t freq,
					    uint8_t ch_width,
					    uint16_t behav_limit)
{
	return 0;
}

static inline void reg_dmn_print_channels_in_opclass(uint8_t *country,
						     uint8_t op_class)
{
}

#ifdef CONFIG_CHAN_FREQ_API

static inline void
reg_freq_width_to_chan_op_class(struct wlan_objmgr_pdev *pdev,
				qdf_freq_t freq,
				uint16_t chan_width,
				bool global_tbl_lookup,
				uint16_t behav_limit,
				uint8_t *op_class,
				uint8_t *chan_num)
{
}

static inline void
reg_freq_to_chan_op_class(struct wlan_objmgr_pdev *pdev,
			  qdf_freq_t freq,
			  bool global_tbl_lookup,
			  uint16_t behav_limit,
			  uint8_t *op_class,
			  uint8_t *chan_num)
{
}

#endif

static inline uint16_t reg_get_op_class_width(struct wlan_objmgr_pdev *pdev,
					      uint8_t op_class,
					      bool global_tbl_lookup)
{
	return 0;
}

static inline uint16_t
reg_chan_opclass_to_freq(uint8_t chan,
			 uint8_t op_class,
			 bool global_tbl_lookup)
{
	return 0;
}

#endif
#endif
