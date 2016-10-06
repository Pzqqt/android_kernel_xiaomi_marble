/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
#include <dispatcher_init_deinit.h>
#ifdef NAPIER_CODE
#include <scheduler_api.h>
#endif

/**
 * DOC: This file provides various init/deinit trigger point for new
 * components.
 */

/* All new components needs to replace their dummy init/deinit
 * psoc_open, psco_close, psoc_enable and psoc_disable APIs once
 * thier actual handlers are ready
 */

static QDF_STATUS obj_manager_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS obj_manager_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}

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

static QDF_STATUS scm_psoc_open(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS scm_psoc_close(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS p2p_psoc_open(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS p2p_psoc_close(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS tdls_psoc_open(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS tdls_psoc_close(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS scm_psoc_enable(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS scm_psoc_disable(void)
{
	return QDF_STATUS_SUCCESS;
}


static QDF_STATUS p2p_psoc_enable(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS p2p_psoc_disable(void)
{
	return QDF_STATUS_SUCCESS;
}


static QDF_STATUS tdls_psoc_enable(void)
{
	return QDF_STATUS_SUCCESS;
}


static QDF_STATUS tdls_psoc_disable(void)
{
	return QDF_STATUS_SUCCESS;
}

#ifndef NAPIER_CODE
static QDF_STATUS scheduler_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS scheduler_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif

QDF_STATUS dispatcher_init(void)
{
	if (QDF_STATUS_SUCCESS != obj_manager_init())
		goto out;

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
	obj_manager_deinit();

out:
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS dispatcher_deinit(void)
{
	QDF_BUG(QDF_STATUS_SUCCESS == scheduler_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == tdls_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == p2p_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == scm_deinit());

	QDF_BUG(QDF_STATUS_SUCCESS == obj_manager_deinit());

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dispatcher_psoc_open(void)
{
	if (QDF_STATUS_SUCCESS != scm_psoc_open())
		goto out;

	if (QDF_STATUS_SUCCESS != p2p_psoc_open())
		goto p2p_psoc_open_fail;

	if (QDF_STATUS_SUCCESS != tdls_psoc_open())
		goto tdls_psoc_open_fail;

	return QDF_STATUS_SUCCESS;

tdls_psoc_open_fail:
	p2p_psoc_close();
p2p_psoc_open_fail:
	scm_psoc_close();

out:
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS dispatcher_psoc_close(void)
{
	QDF_BUG(QDF_STATUS_SUCCESS == tdls_psoc_close());

	QDF_BUG(QDF_STATUS_SUCCESS == p2p_psoc_close());

	QDF_BUG(QDF_STATUS_SUCCESS == scm_psoc_close());

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dispatcher_psoc_enable(void)
{
	if (QDF_STATUS_SUCCESS != scm_psoc_enable())
		goto out;

	if (QDF_STATUS_SUCCESS != p2p_psoc_enable())
		goto p2p_psoc_enable_fail;

	if (QDF_STATUS_SUCCESS != tdls_psoc_enable())
		goto tdls_psoc_enable_fail;

	return QDF_STATUS_SUCCESS;

tdls_psoc_enable_fail:
	p2p_psoc_disable();
p2p_psoc_enable_fail:
	scm_psoc_disable();

out:
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS dispatcher_psoc_disable(void)
{
	QDF_BUG(QDF_STATUS_SUCCESS == tdls_psoc_disable());

	QDF_BUG(QDF_STATUS_SUCCESS == p2p_psoc_disable());

	QDF_BUG(QDF_STATUS_SUCCESS == scm_psoc_disable());

	return QDF_STATUS_SUCCESS;
}
