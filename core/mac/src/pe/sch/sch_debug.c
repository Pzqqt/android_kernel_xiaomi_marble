/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/*
 *
 * This file sch_debug.cc contains some debug functions.
 *
 * Author:      Sandesh Goel
 * Date:        02/25/02
 * History:-
 * Date            Modified by    Modification Information
 * --------------------------------------------------------------------
 *
 */

#include "qdf_trace.h"
#include "sch_debug.h"
#define LOG_SIZE 256

void sch_log(tpAniSirGlobal pMac, uint32_t loglevel, const char *pString, ...)
{

	QDF_TRACE_LEVEL qdf_debug_level;
	char logBuffer[LOG_SIZE];
	va_list marker;

	/* getting proper Debug level */
	qdf_debug_level = get_vos_debug_level(loglevel);

	/* extracting arguments from pstring */
	va_start(marker, pString);
	vsnprintf(logBuffer, LOG_SIZE, pString, marker);
	QDF_TRACE(QDF_MODULE_ID_PE, qdf_debug_level, "%s", logBuffer);
	va_end(marker);
}

/* -------------------------------------------------------------------- */
