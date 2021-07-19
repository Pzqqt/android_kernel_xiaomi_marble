/*
 * Copyright (c) 2013-2016,2018-2021 The Linux Foundation. All rights reserved.
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

#ifndef HOSTDEFS_H_
#define HOSTDEFS_H_

#include <a_osapi.h>
#include <athdefs.h>
#include <a_types.h>
#include "host_reg_init.h"

extern struct hostdef_s *AR6002_HOSTdef;
extern struct hostdef_s *AR6003_HOSTdef;
extern struct hostdef_s *AR6004_HOSTdef;
extern struct hostdef_s *AR9888_HOSTdef;
extern struct hostdef_s *AR9888V2_HOSTdef;
extern struct hostdef_s *AR6320_HOSTdef;
extern struct hostdef_s *AR900B_HOSTdef;
extern struct hostdef_s *QCA9984_HOSTdef;
extern struct hostdef_s *QCA9888_HOSTdef;
extern struct hostdef_s *QCA6290_HOSTdef;
extern struct hostdef_s *QCA6390_HOSTdef;
extern struct hostdef_s *QCA6490_HOSTdef;
extern struct hostdef_s *QCA6750_HOSTdef;
extern struct hostdef_s *WCN7850_HOSTdef;

#ifdef ATH_AHB
extern struct hostdef_s *IPQ4019_HOSTdef;
#endif
extern struct hostdef_s *QCA8074_HOSTdef;
extern struct hostdef_s *QCA8074V2_HOSTDEF;
extern struct hostdef_s *QCA6018_HOSTDEF;
extern struct hostdef_s *QCA5018_HOSTDEF;
extern struct hostdef_s *QCN9000_HOSTDEF;
extern struct hostdef_s *QCN6122_HOSTDEF;
extern struct hostdef_s *QCN9224_HOSTDEF;
extern struct hostdef_s *QCA9574_HOSTDEF;
#endif
