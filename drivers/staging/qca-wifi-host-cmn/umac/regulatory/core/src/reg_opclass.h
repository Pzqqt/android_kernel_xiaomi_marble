/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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

#ifdef CONFIG_AFC_SUPPORT
#include <wlan_reg_afc.h>
#endif

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
					    uint16_t ch_width,
					    uint16_t behav_limit);

/**
 * reg_get_band_cap_from_op_class() - Return band capability bitmap
 * @country: Pointer to Country code.
 * @num_of_opclass: Number of Operating class.
 * @opclass: Pointer to opclass.
 *
 * Return supported band bitmap based on the input operating class list
 * provided.
 *
 * Return: Return supported band capability
 */
uint8_t reg_get_band_cap_from_op_class(const uint8_t *country,
				       uint8_t num_of_opclass,
				       const uint8_t *opclass);

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

/**
 * reg_get_opclass_details() - Get details about the current opclass table.
 * @pdev: Pointer to pdev.
 * @reg_ap_cap: Pointer to reg_ap_cap.
 * @n_opclasses: Pointer to number of opclasses.
 * @max_supp_op_class: Maximum number of operating classes supported.
 * @global_tbl_lookup: Whether to lookup global op class table.
 *
 * Return: QDF_STATUS_SUCCESS if success, else return QDF_STATUS_FAILURE.
 */
QDF_STATUS reg_get_opclass_details(struct wlan_objmgr_pdev *pdev,
				   struct regdmn_ap_cap_opclass_t *reg_ap_cap,
				   uint8_t *n_opclasses,
				   uint8_t max_supp_op_class,
				   bool global_tbl_lookup);

/**
 * reg_is_5ghz_op_class() - Check if the input opclass is a 5GHz opclass.
 * @country: Country code.
 * @op_class: Operating class.
 *
 * Return: Return true if input the opclass is a 5GHz opclass,
 * else return false.
 */
bool reg_is_5ghz_op_class(const uint8_t *country, uint8_t op_class);

/**
 * reg_is_2ghz_op_class() - Check if the input opclass is a 2.4GHz opclass.
 * @country: Country code.
 * @op_class: Operating class.
 *
 * Return: Return true if input the opclass is a 2.4GHz opclass,
 * else return false.
 */
bool reg_is_2ghz_op_class(const uint8_t *country, uint8_t op_class);

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
 * reg_freq_width_to_chan_op_class_auto() - convert frequency to operating
 * class,channel after fixing up the global_tbl_lookup and behav_limit
 * for 6G frequencies.
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
void reg_freq_width_to_chan_op_class_auto(struct wlan_objmgr_pdev *pdev,
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

/**
 * reg_is_freq_in_country_opclass() - check for frequency in (tbl, oper class)
 *
 * @pdev: pdev pointer
 * @country: country from country IE
 * @op_class: operating class
 * @chan_freq: channel frequency in mhz
 *
 * Return: bool
 */
bool reg_is_freq_in_country_opclass(struct wlan_objmgr_pdev *pdev,
				    const uint8_t country[3],
				    uint8_t op_class,
				    qdf_freq_t chan_freq);
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

#ifdef HOST_OPCLASS_EXT
/**
 * reg_country_chan_opclass_to_freq() - Convert channel number to frequency
 * based on country code and op class
 * @pdev: pdev object.
 * @country: country code.
 * @chan: IEEE Channel Number.
 * @op_class: Opclass.
 * @strict: flag to find channel from matched operating class code.
 *
 * Look up (channel, operating class) pair in country operating class tables
 * and return the channel frequency.
 * If not found and "strict" flag is false, try to get frequency (Mhz) by
 * channel number only.
 *
 * Return: Channel center frequency else return 0.
 */
qdf_freq_t reg_country_chan_opclass_to_freq(struct wlan_objmgr_pdev *pdev,
					    const uint8_t country[3],
					    uint8_t chan, uint8_t op_class,
					    bool strict);
#endif

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

/**
 * reg_chan_opclass_to_freq_auto() - Convert channel number and opclass to
 * frequency after fixing global_tbl_lookup
 * @chan: IEEE Channel Number.
 * @op_class: Opclass.
 * @global_tbl_lookup: Global table lookup.
 *
 * Return: Channel center frequency else return 0.
 */
qdf_freq_t reg_chan_opclass_to_freq_auto(uint8_t chan, uint8_t op_class,
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
					    uint16_t ch_width,
					    uint16_t behav_limit)
{
	return 0;
}

static inline
uint8_t reg_get_band_cap_from_op_class(const uint8_t *country,
				       uint8_t num_of_opclass,
				       const uint8_t *opclass)
{
	return 0;
}

static inline void reg_dmn_print_channels_in_opclass(uint8_t *country,
						     uint8_t op_class)
{
}

static inline
QDF_STATUS reg_get_opclass_details(struct wlan_objmgr_pdev *pdev,
				   struct regdmn_ap_cap_opclass_t *reg_ap_cap,
				   uint8_t *n_opclasses,
				   uint8_t max_supp_op_class,
				   bool global_tbl_lookup)
{
	return QDF_STATUS_E_FAILURE;
}

static inline
bool reg_is_5ghz_op_class(const uint8_t *country, uint8_t op_class)
{
	return false;
}

static inline
bool reg_is_2ghz_op_class(const uint8_t *country, uint8_t op_class)
{
	return false;
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
reg_freq_width_to_chan_op_class_auto(struct wlan_objmgr_pdev *pdev,
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

static inline bool
reg_is_freq_in_country_opclass(struct wlan_objmgr_pdev *pdev,
			       const uint8_t country[3],
			       uint8_t op_class,
			       uint16_t chan_freq)
{
	return false;
}

#endif

static inline uint16_t reg_get_op_class_width(struct wlan_objmgr_pdev *pdev,
					      uint8_t op_class,
					      bool global_tbl_lookup)
{
	return 0;
}

#ifdef HOST_OPCLASS_EXT
static inline
qdf_freq_t reg_country_chan_opclass_to_freq(struct wlan_objmgr_pdev *pdev,
					    const uint8_t country[3],
					    uint8_t chan, uint8_t op_class,
					    bool strict)
{
	return 0;
}
#endif

static inline uint16_t
reg_chan_opclass_to_freq(uint8_t chan,
			 uint8_t op_class,
			 bool global_tbl_lookup)
{
	return 0;
}

static inline qdf_freq_t
reg_chan_opclass_to_freq_auto(uint8_t chan, uint8_t op_class,
			      bool global_tbl_lookup)
{
	return 0;
}
#endif

/**
 * reg_dmn_get_chanwidth_from_opclass_auto()- Get channel width for the
 * given channel and opclass. If not found then search it in the global
 * op class.
 * @country - Country
 * @channel - Channel for which channel spacing is required
 * @opclass - Opclass to search from.
 *
 * Return: valid channel spacing if found. If not found then
 * return 0.
 */
uint16_t reg_dmn_get_chanwidth_from_opclass_auto(uint8_t *country,
						 uint8_t channel,
						 uint8_t opclass);

#ifdef CONFIG_AFC_SUPPORT

/**
 * reg_dmn_get_6g_opclasses_and_channels()- Get the following from the
 * operating class table for 6Ghz band: number of operating classes, list of
 * opclasses, list channel sizes, list of channel lists.
 * @p_frange_lst: Pointer to frequencey range list (AFC)
 * @pdev: Pointer to pdev.
 * @num_opclasses: Pointer to number of operating classes. This is the number
 * of elements in the list array arguments
 * @opclas_lst: Pointer to pointer to memory of list of opclasses
 * @chansize_lst: Pointer to pointer to memory of list of channel sizes
 * @channel_lists: Array of pointers to pointer to memory of list of channels
 *
 * Return: QDF_STATUS
 * NOTE:- All memory allocations done by this function should be freed by the
 *        caller. The caller may use the function
 *        'reg_dmn_free_6g_opclasses_and_channels' to free the allocations.
 */

QDF_STATUS reg_dmn_get_6g_opclasses_and_channels(struct wlan_objmgr_pdev *pdev,
						 struct wlan_afc_frange_list *p_frange_lst,
						 uint8_t *num_opclasses,
						 uint8_t **opclass_lst,
						 uint8_t **chansize_lst,
						 uint8_t **channel_lists[]);

/**
 * reg_dmn_free_6g_opclasses_and_channels()- Free the memory allocated by
 * the pointers and arrays indicated by the arguments.
 * @pdev: Pointer to pdev.
 * @num_opclasses: Number of operating classes. This is the number of
 * elements in the 'channel_lists' array.
 * @opclas_lst: Pointer to memory of list of opclasses
 * @chansize_lst: Pointer to memory of list of channel sizes
 * @channel_lists: Array of pointers to memory of list of channels
 *
 * Return: void
 */
void reg_dmn_free_6g_opclasses_and_channels(struct wlan_objmgr_pdev *pdev,
					    uint8_t num_opclasses,
					    uint8_t *opclass_lst,
					    uint8_t *chansize_lst,
					    uint8_t *channel_lists[]);
#endif
#endif
