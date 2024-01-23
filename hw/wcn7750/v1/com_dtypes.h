/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#ifndef COM_DTYPES_H
#define COM_DTYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef T_WINNT
   #ifndef WIN32
      #define WIN32
   #endif
   #include <stdlib.h>
#endif

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE   1
#define FALSE  0

#define  ON   1
#define  OFF  0

#ifndef NULL
  #define NULL  0
#endif

#ifndef _ARM_ASM_
#ifndef _BOOLEAN_DEFINED

typedef  unsigned char      boolean;
#define _BOOLEAN_DEFINED
#endif

#if defined(DALSTDDEF_H)
#define _BOOLEAN_DEFINED
#define _UINT32_DEFINED
#define _UINT16_DEFINED
#define _UINT8_DEFINED
#define _INT32_DEFINED
#define _INT16_DEFINED
#define _INT8_DEFINED
#define _UINT64_DEFINED
#define _INT64_DEFINED
#define _BYTE_DEFINED
#endif

#ifndef _UINT32_DEFINED

typedef  unsigned int  uint32;
#define _UINT32_DEFINED
#endif

#ifndef _UINT16_DEFINED

typedef  unsigned short     uint16;
#define _UINT16_DEFINED
#endif

#ifndef _UINT8_DEFINED

typedef  unsigned char      uint8;
#define _UINT8_DEFINED
#endif

#ifndef _INT32_DEFINED

typedef  signed int    int32;
#define _INT32_DEFINED
#endif

#ifndef _INT16_DEFINED

typedef  signed short       int16;
#define _INT16_DEFINED
#endif

#ifndef _INT8_DEFINED

typedef  signed char        int8;
#define _INT8_DEFINED
#endif

#ifndef _BYTE_DEFINED

typedef  unsigned char      byte;
#define  _BYTE_DEFINED
#endif

typedef  unsigned short     word;

typedef  unsigned long      dword;

typedef  unsigned char      uint1;

typedef  unsigned short     uint2;

typedef  unsigned long      uint4;

typedef  signed char        int1;

typedef  signed short       int2;

typedef  long int           int4;

typedef  signed long        sint31;

typedef  signed short       sint15;

typedef  signed char        sint7;

typedef uint16 UWord16 ;
typedef uint32 UWord32 ;
typedef int32  Word32 ;
typedef int16  Word16 ;
typedef uint8  UWord8 ;
typedef int8   Word8 ;
typedef int32  Vect32 ;

#if (! defined T_WINNT) && (! defined __GNUC__)

  #ifndef _INT64_DEFINED

    typedef long long     int64;
    #define _INT64_DEFINED
  #endif
  #ifndef _UINT64_DEFINED

    typedef  unsigned long long  uint64;
    #define _UINT64_DEFINED
  #endif
#else

  #if (defined __GNUC__)
    #ifndef _INT64_DEFINED
      typedef long long           int64;
      #define _INT64_DEFINED
    #endif
    #ifndef _UINT64_DEFINED
      typedef unsigned long long  uint64;
      #define _UINT64_DEFINED
    #endif
  #else
    typedef  __int64              int64;
    #ifndef _UINT64_DEFINED
      typedef  unsigned __int64   uint64;
      #define _UINT64_DEFINED
    #endif
  #endif
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif
