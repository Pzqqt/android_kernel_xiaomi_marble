/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

#include <qdf_types.h>
#include <qdf_trace.h>
#include <qdf_threads.h>
#include <dispatcher_init_deinit.h>
#include <scheduler_api.h>
#include <wlan_mgmt_txrx_utils_api.h>

/**
 * DOC: This file provides various init/deinit trigger point for new
 * components.
 */

/* All new components needs to replace their dummy init/deinit
 * psoc_open, psco_close, psoc_enable and psoc_disable APIs once
 * thier actual handlers are ready
 */

static QDF_STATUS scm_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS scm_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS p2p_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS p2p_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}


static QDF_STATUS tdls_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS tdls_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS scm_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS scm_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS p2p_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS p2p_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS tdls_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS tdls_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS scm_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS scm_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}


static QDF_STATUS p2p_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS p2p_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}


static QDF_STATUS tdls_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}


static QDF_STATUS tdls_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dispatcher_init(void)
{
	if (QDF_STATUS_SUCCESS != wlan_objmgr_global_obj_init())
		goto out;

	if (QDF_STATUS_SUCCESS != wlan_mgmt_txrx_init())
		goto mgmt_txrx_init_fail;

	if (QDF_STATUS_SUCCESS != scm_init())
		goto scm_init_fail;

	if (QDF_STATUS_SUCCESS != p2p_init())
		goto p2p_init_fail;

	if (QDF_STATUS_SUCCESS != tdls_init())
		goto tdls_init_fail;

	if (QDF_STATUS_SUCCESS != scheduler_init())
		goto scheduler_init_fail;

	return QDF_STATUS_SUCCESS;

scheduler_init_fail:
	tdls_deinit();
tdls_init_fail:
	p2p_deinit();
p2p_init_fail:
	scm_deinit();
scm_init_fail:
	wlan_mgmt_txrx_deinit();
mgmt_txrx_init_fail:
	wlan_objmgr_global_obj_deinit();

out:
	return QDF_STATUS_E_FAILURE;
}
EXPORT_SYMBOL(dispatcher_init);

QDF_STATUS dispatcher_deinit(void)
{
	QDF_BUG(QDF_STATUS_SUCCESS == scheduler_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == tdls_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == p2p_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == scm_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == wlan_mgmt_txrx_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == wlan_objmgr_global_obj_deinit());

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(dispatcher_deinit);

QDF_STATUS dispatcher_psoc_open(struct wlan_objmgr_psoc *psoc)
{
	if (QDF_STATUS_SUCCESS != scm_psoc_open(psoc))
		goto out;

	if (QDF_STATUS_SUCCESS != p2p_psoc_open(psoc))
		goto p2p_psoc_open_fail;

	if (QDF_STATUS_SUCCESS != tdls_psoc_open(psoc))
		goto tdls_psoc_open_fail;

	return QDF_STATUS_SUCCESS;

tdls_psoc_open_fail:
	p2p_psoc_close(psoc);
p2p_psoc_open_fail:
	scm_psoc_close(psoc);

out:
	return QDF_STATUS_E_FAILURE;
}
EXPORT_SYMBOL(dispatcher_psoc_open);

QDF_STATUS dispatcher_psoc_close(struct wlan_objmgr_psoc *psoc)
{
	QDF_BUG(QDF_STATUS_SUCCESS == tdls_psoc_close(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == p2p_psoc_close(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == scm_psoc_close(psoc));

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(dispatcher_psoc_close);

QDF_STATUS dispatcher_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	if (QDF_STATUS_SUCCESS != scm_psoc_enable(psoc))
		goto out;

	if (QDF_STATUS_SUCCESS != p2p_psoc_enable(psoc))
		goto p2p_psoc_enable_fail;

	if (QDF_STATUS_SUCCESS != tdls_psoc_enable(psoc))
		goto tdls_psoc_enable_fail;

	return QDF_STATUS_SUCCESS;

tdls_psoc_enable_fail:
	p2p_psoc_disable(psoc);
p2p_psoc_enable_fail:
	scm_psoc_disable(psoc);

out:
	return QDF_STATUS_E_FAILURE;
}
EXPORT_SYMBOL(dispatcher_psoc_enable);

QDF_STATUS dispatcher_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	QDF_BUG(QDF_STATUS_SUCCESS == tdls_psoc_disable(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == p2p_psoc_disable(psoc));

	QDF_BUG(QDF_STATUS_SUCCESS == scm_psoc_disable(psoc));

	return QDF_STATUS_SUCCESS;
}
EXPORT_SYMBOL(dispatcher_psoc_disable);
