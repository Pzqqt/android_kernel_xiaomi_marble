/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

#ifndef TARGETDEFS_H_
#define TARGETDEFS_H_

#include <a_osapi.h>
#include <athdefs.h>
#include <a_types.h>
#include "target_reg_init.h"

extern struct targetdef_s *AR6002_TARGETdef;
extern struct targetdef_s *AR6003_TARGETdef;
extern struct targetdef_s *AR6004_TARGETdef;
extern struct targetdef_s *AR9888_TARGETdef;
extern struct targetdef_s *AR9888V2_TARGETdef;
extern struct targetdef_s *AR6320_TARGETdef;
extern struct targetdef_s *AR900B_TARGETdef;
extern struct targetdef_s *QCA9984_TARGETdef;
extern struct targetdef_s *QCA9888_TARGETdef;
#ifdef ATH_AHB
extern struct targetdef_s *IPQ4019_TARGETdef;
#endif

extern struct ce_reg_def *AR6002_CE_TARGETdef;
extern struct ce_reg_def *AR6003_CE_TARGETdef;
extern struct ce_reg_def *AR6004_CE_TARGETdef;
extern struct ce_reg_def *AR9888_CE_TARGETdef;
extern struct ce_reg_def *AR9888V2_CE_TARGETdef;
extern struct ce_reg_def *AR6320_CE_TARGETdef;
extern struct ce_reg_def *AR900B_CE_TARGETdef;
extern struct ce_reg_def *QCA9984_CE_TARGETdef;
extern struct ce_reg_def *QCA9888_CE_TARGETdef;
#ifdef ATH_AHB
extern struct ce_reg_def *IPQ4019_CE_TARGETdef;
#endif


#endif
