/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC : osif_twt_req.h
 *
 */

#ifndef _OSIF_TWT_REQ_H_
#define _OSIF_TWT_REQ_H_

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_twt_public_structs.h>

/**
 * osif_twt_requestor_enable() - TWT requestor enable api
 * @psoc: psoc handle
 * @req: TWT enable request
 *
 * Return: 0 on success, errno on failure
 */
int osif_twt_requestor_enable(struct wlan_objmgr_psoc *psoc,
			      struct twt_enable_param *req);

/**
 * osif_twt_responder_enable() - TWT responder enable api
 * @psoc: psoc handle
 * @req: TWT enable request
 *
 * Return: 0 on success, errno on failure
 */
int osif_twt_responder_enable(struct wlan_objmgr_psoc *psoc,
			      struct twt_enable_param *req);

/**
 * osif_twt_requestor_disable() - TWT requestor disable api
 * @psoc: psoc handle
 * @req: TWT disable request
 *
 * Return: 0 on success, errno on failure
 */
int osif_twt_requestor_disable(struct wlan_objmgr_psoc *psoc,
			       struct twt_disable_param *req);

/**
 * osif_twt_responder_disable() - TWT responder disable api
 * @psoc: psoc handle
 * @req: TWT disable request
 *
 * Return: 0 on success, errno on failure
 */
int osif_twt_responder_disable(struct wlan_objmgr_psoc *psoc,
			       struct twt_disable_param *req);
#else
static inline
int osif_twt_requestor_enable(struct wlan_objmgr_psoc *psoc,
			      struct twt_enable_param *req)
{
	return 0;
}

static inline
int osif_twt_responder_enable(struct wlan_objmgr_psoc *psoc,
			      struct twt_enable_param *req)
{
	return 0;
}

static inline
int osif_twt_requestor_disable(struct wlan_objmgr_psoc *psoc,
			       struct twt_disable_param *req)
{
	return 0;
}

static inline
int osif_twt_responder_disable(struct wlan_objmgr_psoc *psoc,
			       struct twt_disable_param *req)
{
	return 0;
}
#endif
#endif /* _OSIF_TWT_REQ_H_ */

