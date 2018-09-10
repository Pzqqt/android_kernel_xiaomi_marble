/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains centralized definitions of converged configuration.
 */

#ifndef __CFG_MLME_FEATURE_FLAG_H
#define __CFG_MLME_FEATURE_FLAG_H

#define CFG_ACCEPT_SHORT_SLOT_ASSOC_ONLY CFG_BOOL( \
		"accept_short_slot_assoc", \
		0, \
		"Accept short slot assoc only")

#define CFG_HCF_ENABLED CFG_BOOL( \
		"enable_hcf", \
		0, \
		"HCF enabled")

#define CFG_RSN_ENABLED CFG_BOOL( \
		"enable_rsn", \
		0, \
		"RSN enabled")

#define CFG_11G_SHORT_PREAMBLE_ENABLED CFG_BOOL( \
		"enable_short_preamble_11g", \
		0, \
		"Short Preamble Enable")

#define CFG_11G_SHORT_SLOT_TIME_ENABLED CFG_BOOL( \
		"enable_short_slot_time_11g", \
		1, \
		"Short Slot time enable")

#define CFG_CHANNEL_BONDING_MODE CFG_UINT( \
		"channel_bonding_mode", \
		0, \
		10, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"channel bonding mode")

#define CFG_BLOCK_ACK_ENABLED CFG_UINT( \
		"enable_block_ack", \
		0, \
		3, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"enable block Ack")
/*
 * <ini>
 * gEnableAMPDUPS - Enable the AMPDUPS
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to set default AMPDUPS
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: Internal/External
 *
 * </ini>
 */

#define CFG_ENABLE_AMPDUPS CFG_INI_BOOL( \
				"gEnableAMPDUPS", \
				0, \
				"Enable AMPDU")

#define CFG_FEATURE_FLAG_ALL \
	CFG(CFG_ACCEPT_SHORT_SLOT_ASSOC_ONLY) \
	CFG(CFG_HCF_ENABLED) \
	CFG(CFG_RSN_ENABLED) \
	CFG(CFG_11G_SHORT_PREAMBLE_ENABLED) \
	CFG(CFG_11G_SHORT_SLOT_TIME_ENABLED) \
	CFG(CFG_CHANNEL_BONDING_MODE) \
	CFG(CFG_BLOCK_ACK_ENABLED) \
	CFG(CFG_ENABLE_AMPDUPS)

#endif /* __CFG_MLME_FEATURE_FLAG_H */

