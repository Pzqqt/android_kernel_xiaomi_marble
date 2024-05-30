/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

//////////////////////////////////////////////////////////////////////////////
// RCS File        : -USE CVS LOG-
// Revision        : -USE CVS LOG-
// Last Check In   : -USE CVS LOG-
//////////////////////////////////////////////////////////////////////////////
// Description     : Top C Struct file
//
//////////////////////////////////////////////////////////////////////////////

#ifndef WFSS_PMM_BASE_STUCT_HEADER_INCLUDED
   #ifdef _LSB_TO_MSB_REGS
      #ifdef _MSB_TO_LSB_REGS
         #error You can not define both _LSB_TO_MSB_REGS and _MSB_TO_LSB_REGS!
      #endif

      #define WFSS_PMM_BASE_STUCT_HEADER_INCLUDED
      #include "wfss_pmm_base_struct_ltm.h"
   #endif

   #ifdef _MSB_TO_LSB_REGS
      #define WFSS_PMM_BASE_STUCT_HEADER_INCLUDED
      #include "wfss_pmm_base_struct_mtl.h"
   #endif

   #ifndef WFSS_PMM_BASE_STUCT_HEADER_INCLUDED
      #error You have to define _LSB_TO_MSB_REGS or _MSB_TO_LSB_REGS
   #endif

#endif

