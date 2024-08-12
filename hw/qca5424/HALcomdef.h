
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
#ifndef HAL_COMDEF_H
#define HAL_COMDEF_H
 


 
#ifndef _ARM_ASM_

 
#ifdef __cplusplus
extern "C" {
#endif

#include "com_dtypes.h"

 

 
#ifndef _BOOL32_DEFINED
typedef  unsigned long int  bool32;
#define _BOOL32_DEFINED
#endif

 
#define HAL_ENUM_32BITS(x) HAL_##x##_FORCE32BITS = 0x7FFFFFFF

 

   

  #define inp(port)         (*((volatile byte *) (port)))
  #define inpw(port)        (*((volatile word *) (port)))
  #define inpdw(port)       (*((volatile dword *)(port)))

  #define outp(port, val)   (*((volatile byte *) (port)) = ((byte) (val)))
  #define outpw(port, val)  (*((volatile word *) (port)) = ((word) (val)))
  #define outpdw(port, val) (*((volatile dword *) (port)) = ((dword) (val)))

#ifdef __cplusplus
}
#endif

#endif  

#endif  

