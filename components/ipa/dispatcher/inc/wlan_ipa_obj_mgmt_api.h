/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: Declare public API related to the wlan ipa called by north bound
 */

#ifndef _WLAN_IPA_OBJ_MGMT_H_
#define _WLAN_IPA_OBJ_MGMT_H_

#include "wlan_ipa_public_struct.h"
#include "wlan_objmgr_pdev_obj.h"

#ifdef IPA_OFFLOAD

/**
 * ipa_init() - IPA module initialization
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS ipa_init(void);

/**
 * ipa_deinit() - IPA module deinitialization
 *
 * Return: QDF_STATUS_SUCCESS on success
 */
QDF_STATUS ipa_deinit(void);
#else

static inline QDF_STATUS ipa_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS ipa_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* IPA_OFFLOAD */

#endif /* _WLAN_IPA_OBJ_MGMT_H_ */
