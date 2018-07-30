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
 * DOC: This file contains centralized definitions of sap erp protection related
 * converged configurations.
 */

#ifndef __CFG_MLME_SAP_PROTECTION_H
#define __CFG_MLME_SAP_PROTECTION_H

#define CFG_PROTECTION_ENABLED CFG_UINT( \
		"protection_enabled", \
		0, \
		65535, \
		65535, \
		CFG_VALUE_OR_DEFAULT, \
		"sap protection enabled")

#define CFG_FORCE_POLICY_PROTECTION CFG_UINT( \
		"protection_force_policy", \
		0, \
		5, \
		5, \
		CFG_VALUE_OR_DEFAULT, \
		"force policy protection")

/*
 * <ini>
 * gignore_peer_ht_opmode
 *
 * @min 0
 * @max 1
 * @default 1
 *
 * Enabling gignore_peer_ht_opmode will enable 11g
 * protection only when there is a 11g AP in vicinity.
 *
 * Related: None
 *
 * Supported Feature: SAP Protection
 * </ini>
 */
#define CFG_IGNORE_PEER_HT_MODE CFG_INI_BOOL( \
		"gignore_peer_ht_opmode", \
		0, \
		"ignore the peer ht mode")

#define CFG_SAP_PROTECTION_ALL \
	CFG(CFG_PROTECTION_ENABLED) \
	CFG(CFG_FORCE_POLICY_PROTECTION) \
	CFG(CFG_IGNORE_PEER_HT_MODE)

#endif /* __CFG_MLME_SAP_PROTECTION_H */
