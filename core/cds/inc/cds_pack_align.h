/*
 * Copyright (c) 2014 The Linux Foundation. All rights reserved.
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

#if !defined( __CDS_PACK_ALIGN_H )
#define __CDS_PACK_ALIGN_H

/**=========================================================================

   \file  cds_pack_align.h

   \brief Connectivity driver services (CDS) pack and align primitives

   Definitions for platform independent means of packing and aligning
   data structures

   ========================================================================*/

/*

   Place the macro CDS_PACK_START above a structure declaration to pack. We
   are not going to allow modifying the pack size because pack size cannot be
   specified in AMSS and GNU. Place the macro CDS_PACK_END below a structure
   declaration to stop the pack. This requirement is necessitated by Windows
   which need pragma based prolog and epilog.

   Pack-size > 1-byte is not supported since gcc and arm do not support that.

   Here are some examples

   1. Pack-size 1-byte foo_t across all platforms

   CDS_PACK_START
   typedef CDS_PACK_PRE struct foo_s { ... } CDS_PACK_POST foo_t;
   CDS_PACK_END

   2. 2-byte alignment for foo_t across all platforms

   typedef CDS_ALIGN_PRE(2) struct foo_s { ... } CDS_ALIGN_POST(2) foo_t;

   3. Pack-size 1-byte and 2-byte alignment for foo_t across all platforms

   CDS_PACK_START
   typedef CDS_PACK_PRE CDS_ALIGN_PRE(2) struct foo_s { ... } CDS_ALIGN_POST(2) CDS_PACK_POST foo_t;
   CDS_PACK_END

 */

#if defined __GNUC__

#define CDS_PACK_START
#define CDS_PACK_END

#define CDS_PACK_PRE
#define CDS_PACK_POST  __attribute__((__packed__))

#define CDS_ALIGN_PRE(__value)
#define CDS_ALIGN_POST(__value)  __attribute__((__aligned__(__value)))

#elif defined __arm

#define CDS_PACK_START
#define CDS_PACK_END

#define CDS_PACK_PRE  __packed
#define CDS_PACK_POST

#define CDS_ALIGN_PRE(__value)  __align(__value)
#define CDS_ALIGN_POST(__value)

#elif defined _MSC_VER

#define CDS_PACK_START  __pragma(pack(push,1))
#define CDS_PACK_END  __pragma(pack(pop))

#define CDS_PACK_PRE
#define CDS_PACK_POST

#define CDS_ALIGN_PRE(__value)  __declspec(align(__value))
#define CDS_ALIGN_POST(__value)

#else

#error Unsupported compiler!!!

#endif

#endif /* __CDS_PACK_ALIGN_H */
