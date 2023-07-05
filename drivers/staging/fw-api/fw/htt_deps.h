/*
 * Copyright (c) 2011-2017 The Linux Foundation. All rights reserved.
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

/**
 * @file htt_deps.h
 *
 * @details list other header files that contain the defs for data types,
 *      constants, and compiler pragmas used in the HTT header files
 */

#ifndef _HTT_DEPS_H_
#define _HTT_DEPS_H_

#include <a_types.h>    /* A_UINT32 */

#ifndef QCA_WIFI_WIFISTATS_EXTBUILD
#include <a_osapi.h>    /* PREPACK, POSTPACK */
#endif

#ifdef ATHR_WIN_NWF
#pragma warning(disable:4214) /* bit field types other than int */
#endif

#include "wlan_defs.h"

#endif /* _HTT_DEPS_H_ */
