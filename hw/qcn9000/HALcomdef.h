/*
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

#ifndef HAL_COMDEF_H
#define HAL_COMDEF_H
/*===========================================================================

                   S T A N D A R D    D E C L A R A T I O N S

DESCRIPTION
  This header file contains general types and macros that are of use
  to all modules.  The values or definitions are dependent on the specified
  target.  T_WINNT specifies Windows NT based targets, otherwise the 
  default is for ARM targets.

       T_WINNT  Software is hosted on an NT platforn, triggers macro and
                type definitions, unlike definition above which triggers
                actual OS calls
 
DEFINED TYPES

       Name      Definition
       -------   --------------------------------------------------------
       byte      8  bit unsigned value
       word      16 bit unsigned value 
       dword     32 bit unsigned value

       uint1     byte
       uint2     word
       uint4     dword

       uint8     8  bit unsigned value
       uint16    16 bit unsigned value
       uint32    32 bit unsigned value
       uint64    64 bit unsigned value

       uint8_t     8  bit unsigned value
       uint16_t    16 bit unsigned value
       uint32_t    32 bit unsigned value
       uint64_t    64 bit unsigned value

       int8      8  bit signed value
       int16     16 bit signed value 
       int32     32 bit signed value
       int64    64 bit signed value

       sint31    32 bit signed value 
       sint15    16 bit signed value
       sint7     8  bit signed value

       int1      8  bit signed value
       int2      16 bit signed value
       int4      32 bit signed value

       boolean   8 bit boolean value

DEFINED CONSTANTS

       Name      Definition
       -------   --------------------------------------------------------
       TRUE      Asserted boolean condition (Logical 1)
       FALSE     Deasserted boolean condition (Logical 0)
 
       ON        Asserted condition
       OFF       Deasserted condition

       NULL      Pointer to nothing

       PACKED    Used to indicate structures which should use packed
                 alignment 

       INLINE    Used to inline functions for compilers which support this
       
===========================================================================*/


/*===========================================================================

                      EDIT HISTORY FOR FILE

This section contains comments describing changes made to this file.
Notice that changes are listed in reverse chronological order.

when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/27/16    rl     (c_lewisr) fix logic issues with int's
06/11/01    gr     Added MOD_BY_POWER_OF_TWO macro.
04/12/01    sq     Added inpdw and outpdw macros
           vas     Added solaris support
04/06/01    gr     Removed the definitions of abs and labs.
01/25/01   day     Merged from MSM5105_COMMON.00.00.05.
					 Added include for stdlib.h within WINNT ifdef
					 Added support for ARM_ASM
					 Added support SWIG preprocessor.
09/09/99   tac     Merged in a bunch of panther changes.
08/11/99   jct     Corrected conditional compilation around labs to check for
                   labs instead of abs
06/30/99   jct     Added PC_EMULATOR capability for INTLOCK and INTLOCKSAV
06/15/99   jct     Cleanup to make work in Windows NT environment.  Addition
                   of int(n) types where n is 8,16,32,64 and addition of
                   unit64.  Removal of OS comments and defines as this is replaced
                   by T_WINNT.  Conditional inclusion of MAX, MIN, labs, abs,
                   NULL definitions.  Removal of volatile from packed definition
                   stdlib.h included for Win32 based targets since this replaces several
                   of the define here.  Changed out uts calls to be comet calls.
                   Added a defines for Windows NT targets to define away symbols
                   no longer supported
04/09/99    ms     Lint cleanup.
04/01/99    ms     Lint cleanup.
02/17/99    ms     Parenthesized abs and labs.
                   Defined __packed to volatile to exploit lint.
12/16/98   jct     Removed 80186 support
12/01/98    ms     Removed definition of bsp_io_base.
11/20/98    ms     Typecast to volatile pointers in inp,outp,inpw and outpw.
10/10/98   jct     Honing of OS and T_ definitions, added T_WINNT as a target
                   which triggers something different then which OS since
                   we may be running REX on an NT platform, but strictly 
                   speaking, the application software is using REX, not NT
                   as the OS.  Changed ROM for WINNT/ARM targets to be nothing
                   instead of const 
09/09/98   jct     Updates to merge ARM support with 186 support, added some
                   new type aliases, added some segmented architecture macros,
                   updated comments, removed unused code, updated declaration
                   of inp/outp to have cdecl 
08/01/98   jct     Updates to support ARM processor
03/25/96   jah     Moved SET_VECT to 80186.h
02/06/96   fkm     Put ()s around Macros to Make Lint Happy
01/12/95   jah     Updated dependency in MACRO INTLOCK_SAV / INTFREE_SAV.
12/20/94   jah     Corrected typo in ARR_SIZE() macro comments
03/11/94   jah     Added #ifdef _lint of NULL as 0 for near/far lint problem.
06/11/93   jah     Changed FPOS() to cast the pointer to 'near' to avoid the
                   compiler complaints about lost segments.
01/28/93   twp     Added ARR_SIZE macro to return number of array elements.
07/17/92   jah     Changed int1 from 'char' to 'signed char'
06/10/92   jah     Added WORD_LO/HI, INC_SAT
05/19/92   jah     Added header comments for macros & made them lint-friendly
05/04/92   jah     Added define for ROM
03/02/92   jah     Added in/outp prototypes, INT* Macro comments w/lint
02/20/92   arh     Added LOCAL macro and ifdef/endif around file

===========================================================================*/


/*===========================================================================

                            Data Declarations

===========================================================================*/

/* -----------------------------------------------------------------------
** We include target.h to get FEATURE definitions used for macro defines.  
** Note we only include this if TG is defined and target.h has not been
** previously included.  This allows comdef.h to be reused outside the
** scope of the target/customer featurization paradigm
** ----------------------------------------------------------------------- */
#ifdef TG
  #include "target.h"
#endif

/* For NT apps we want to use the Win32 definitions and/or those
** supplied by the Win32 compiler for things like NULL, MAX, MIN
** abs, labs, etc.
*/
#ifdef T_WINNT
   #ifndef WIN32
      #define WIN32
   #endif
   #include <stdlib.h>
#endif
#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

/* ------------------------------------------------------------------------
** Constants
** ------------------------------------------------------------------------ */

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE   1   /* Boolean true value. */
#define FALSE  0   /* Boolean false value. */

#define  ON   1    /* On value. */
#define  OFF  0    /* Off value. */

#ifdef _lint
  #define NULL 0   
#endif

#ifndef NULL
  #define NULL  0
#endif

/* -----------------------------------------------------------------------
** Standard Types
** ----------------------------------------------------------------------- */

/* The following definitions are the same accross platforms.  This first
** group are the sanctioned types.
*/
#ifndef _ARM_ASM_
typedef  unsigned char      boolean;     /* Boolean value type. */
#if defined(VV_FEATURE_COMPILING_64BIT) 
typedef unsigned long       uint32;
#else
typedef  unsigned int       uint32;      /* Unsigned 32 bit value */
#endif
typedef  unsigned short     uint16;      /* Unsigned 16 bit value */
typedef  unsigned char      uint8;       /* Unsigned 8  bit value */

#if defined(VV_FEATURE_COMPILING_64BIT)
typedef  signed long int    int32;       /* Signed 32 bit value */
#else
typedef  signed int         int32;       /* Signed 32 bit value */
#endif
typedef  signed short       int16;       /* Signed 16 bit value */
typedef  signed char        int8;        /* Signed 8  bit value */

/* This group are the deprecated types.  Their use should be
** discontinued and new code should use the types above
*/
typedef  unsigned char     byte;         /* Unsigned 8  bit value type. */
typedef  unsigned short    word;         /* Unsinged 16 bit value type. */
#if defined(VV_FEATURE_COMPILING_64BIT)
typedef  unsigned int      dword;        /* Unsigned 32 bit value type. */
#else
typedef  unsigned long     dword;        /* Unsigned 32 bit value type. */
#endif

typedef  unsigned char     uint1;        /* Unsigned 8  bit value type. */
typedef  unsigned short    uint2;        /* Unsigned 16 bit value type. */
#if defined(VV_FEATURE_COMPILING_64BIT)
typedef  unsigned int      uint4;        /* Unsigned 32 bit value type. */
#else
typedef  unsigned long     uint4;        /* Unsigned 32 bit value type. */
#endif

typedef  signed char       int1;         /* Signed 8  bit value type. */
typedef  signed short      int2;         /* Signed 16 bit value type. */
#if defined(VV_FEATURE_COMPILING_64BIT)
typedef  int               int4;         /* Signed 32 bit value type. */
#else
typedef  long int          int4;         /* Signed 32 bit value type. */
#endif

#if defined(VV_FEATURE_COMPILING_64BIT)
typedef  signed int        sint31;       /* Signed 32 bit value */
#else
typedef  signed long       sint31;       /* Signed 32 bit value */
#endif
typedef  signed short      sint15;       /* Signed 16 bit value */
typedef  signed char       sint7;        /* Signed 8  bit value */

#if defined(HASTINGS) || defined(PHYDEVLIB_PRODUCT_HAWKEYE) || defined(PHYDEVLIB_PRODUCT_HAWKEYE2) || defined(CYPRESS) || defined (HASTINGSPRIME) || defined(PHYDEVLIB_PRODUCT_PINE)
typedef uint16 UWord16;
typedef uint32 UWord32;
typedef int32  Word32;
typedef int16  Word16;
typedef uint8  UWord8;
typedef int8   Word8;
typedef int32  Vect32;
#endif

/* ---------------------------------------------------------------------
** Compiler Keyword Macros
** --------------------------------------------------------------------- */ 
#if (! defined T_WINNT) && (! defined TARGET_OS_SOLARIS)
#ifndef SWIG  /* The SWIG preprocessor gets confused by these */
  /* Non WinNT Targets 
  */
#if defined(VV_FEATURE_COMPILING_64BIT)
  typedef  signed long        int64;       /* Signed 64 bit value */
  typedef  unsigned long      uint64;      /* Unsigned 64 bit value */
#else
  typedef  signed long long   int64;       /* Signed 64 bit value */
  typedef  unsigned long long uint64;      /* Unsigned 64 bit value */
#endif
  #define PACKED __packed
#ifndef INLINE
  #define INLINE __inline
#endif
  #define CDECL
#endif /* SWIG */
  #define far
  #define near
  #define _far
  #define _near
  #define _cdecl
  #define cdecl
  #define _pascal
  #define _interrupt

#else /* T_WINNT || TARGET_OS_SOLARIS */

  /* WINNT or SOLARIS based targets
  */
#if (defined __GNUC__) || (defined TARGET_OS_SOLARIS)
#if defined(VV_FEATURE_COMPILING_64BIT)
  typedef long                int64;
  typedef unsigned long       uint64;
#else
  typedef long long           int64;
  typedef unsigned long long  uint64;
#endif
#else
  typedef  __int64            int64;       /* Signed 64 bit value */
  typedef  unsigned __int64   uint64;      /* Unsigned 64 bit value */
#endif
  #define PACKED
  /* INLINE is defined to __inline because WINNT targets work fine with it
   * and defining it to nothing does not work because the inline function
   * definition is then multiply defined.  Solaris may need a different value.
   */
  #define INLINE __inline
  #ifndef CDECL
     #define CDECL __cdecl
  #endif
  #define _pascal
  #define _far
  #define far
  #define near
  #define _near
  #define cdecl
  #define _cdecl
  #define _interrupt
  #define __packed
  #define _fmemcpy memcpy

#endif /* T_WINNT */
#endif // #ifndef _ARM_ASM_
 
/* ---------------------------------------------------------------------- 
** Lint does not understand __packed, so we define it away here.  In the
** past we did this:
**   This helps us catch non-packed pointers accessing packed structures,
**   for example, (although lint thinks it is catching non-volatile pointers
**   accessing volatile structures).
**   This does assume that volatile is not being used with __packed anywhere
**   because that would make Lint see volatile volatile (grrr).
** but found it to be more trouble than it was worth as it would emit bogus
** errors 
** ---------------------------------------------------------------------- */
#ifdef _lint
  #define __packed
#endif

/* ----------------------------------------------------------------------
**                          STANDARD MACROS
** ---------------------------------------------------------------------- */

#ifndef SWIG /* these confuse the SWIG preprocessor and aren't needed for it */


/*===========================================================================

MACRO MEM_B
MACRO MEM_W

DESCRIPTION
  Take an address and dereference it as a byte/word, allowing access to an
  arbitrary memory byte/word.

PARAMETERS
  x     address to be dereferenced

DEPENDENCIES
  None

RETURN VALUE
  MEM_B byte at the given address
  MEM_W word at the given address

SIDE EFFECTS
  None

===========================================================================*/

#define  MEM_B( x )  ( *( (uint8 *) (x) ) )
#define  MEM_W( x )  ( *( (uint16 *) (x) ) )



/*===========================================================================

MACRO MAX
MACRO MIN

DESCRIPTION
  Evaluate the maximum/minimum of 2 specified arguments.

PARAMETERS
  x     parameter to compare to 'y'
  y     parameter to compare to 'x'

DEPENDENCIES
  'x' and 'y' are referenced multiple times, and should remain the same
  value each time they are evaluated.

RETURN VALUE
  MAX   greater of 'x' and 'y'
  MIN   lesser of 'x' and 'y'

SIDE EFFECTS
  None

===========================================================================*/
#ifndef MAX
   #define  MAX( x, y ) ( ((x) > (y)) ? (x) : (y) )
#endif

#ifndef MIN
   #define  MIN( x, y ) ( ((x) < (y)) ? (x) : (y) )
#endif



/*===========================================================================

MACRO FPOS

DESCRIPTION
  This macro computes the offset, in bytes, of a specified field
  of a specified structure or union type.

PARAMETERS
  type          type of the structure or union
  field         field in the structure or union to get the offset of

DEPENDENCIES
  None

RETURN VALUE
  The byte offset of the 'field' in the structure or union of type 'type'.

SIDE EFFECTS
  The lint error "Warning 545: Suspicious use of &" is suppressed within
  this macro.  This is due to the desire to have lint not complain when
  'field' is an array.

===========================================================================*/

#define FPOS( type, field ) \
    /*lint -e545 */ ( (dword) &(( type *) 0)-> field ) /*lint +e545 */
 


/*===========================================================================

MACRO FSIZ

DESCRIPTION
  This macro computes the size, in bytes, of a specified field
  of a specified structure or union type.

PARAMETERS
  type          type of the structure or union
  field         field in the structure or union to get the size of

DEPENDENCIES
  None

RETURN VALUE
  size in bytes of the 'field' in a structure or union of type 'type'

SIDE EFFECTS
  None

===========================================================================*/

#define FSIZ( type, field ) sizeof( ((type *) 0)->field )


/*===========================================================================

MACRO FLIPW

DESCRIPTION
  Takes a 2 byte array, with the most significant byte first, followed
  by the least significant byte, and converts the quantity into a word
  with Intel byte order (LSB first)

PARAMETERS
  ray   array of 2 bytes to be converted to a word

DEPENDENCIES
  None

RETURN VALUE
  word in Intel byte order comprised of the 2 bytes of ray.

SIDE EFFECTS
  None

===========================================================================*/

#define  FLIPW( ray ) ( (((word) (ray)[0]) * 256) + (ray)[1] )



/*===========================================================================

MACRO FLOPW

DESCRIPTION
  Reverses bytes of word value and writes them to a 2 byte array.
  Effectively the reverse of the FLIPW macro.

PARAMETERS
  ray   array to receive the 2 bytes from 'val'
  val   word to break into 2 bytes and put into 'ray'

DEPENDENCIES
  None

RETURN VALUE
  None

SIDE EFFECTS
  None

===========================================================================*/
#define  FLOPW( ray, val ) \
  (ray)[0] = ((val) / 256); \
  (ray)[1] = ((val) & 0xFF)



/*===========================================================================

MACRO B_PTR
MACRO W_PTR

DESCRIPTION
  Casts the address of a specified variable as a pointer to byte/word,
  allowing byte/word-wise access, e.g.
  W_PTR ( xyz )[ 2 ] = 0x1234;  -or-    B_PTR ( xyz )[ 2 ] = 0xFF;

PARAMETERS
  var   the datum to get a word pointer to

DEPENDENCIES
  None

RETURN VALUE
  Word pointer to var

SIDE EFFECTS
  None

===========================================================================*/

#define  B_PTR( var )  ( (byte *) (void *) &(var) )
#define  W_PTR( var )  ( (word *) (void *) &(var) )


/*===========================================================================

MACRO WORD_LO
MACRO WORD_HI

DESCRIPTION
  Take a word and extract the least-significant or most-significant byte.

PARAMETERS
  xxx   word to extract the Low/High from

DEPENDENCIES
  None

RETURN VALUE
  WORD_LO       Least significant byte of xxx.
  WORD_HI       Most significant byte of xxx.

SIDE EFFECTS
  None

===========================================================================*/

#define  WORD_LO(xxx)  ((uint8) ((uint16)(xxx) & 255))
#define  WORD_HI(xxx) \
  /*lint -e572 */  ((uint8) ((uint16)(xxx) >> 8))  /*lint +e572 */


/*===========================================================================

MACRO RND8

DESCRIPTION
  RND8 rounds a number up to the nearest multiple of 8.

PARAMETERS
  x     Number to be rounded up

DEPENDENCIES
  None

RETURN VALUE
  x rounded up to the nearest multiple of 8.

SIDE EFFECTS
  None

===========================================================================*/

#define RND8( x )       ((((x) + 7) / 8 ) * 8 )


/*===========================================================================

MACRO INTLOCK / INTFREE

DESCRIPTION
  INTLOCK Saves current interrupt state on stack then disables interrupts.
  Used in conjunction with INTFREE.

  INTFREE Restores previous interrupt state from stack.  Used in conjunction
  with INTLOCK.

PARAMETERS
  None

DEPENDENCIES
  The stack level when INTFREE is called must be as it was just after INTLOCK
  was called.  The Microsoft C compiler does not always pop the stack after a
  subroutine call, but instead waits to do a 'big' pop after several calls.
  This causes these macros to fail.  Do not place subroutine calls between
  these macros.  Use INTLOCK/FREE_SAV in those cases.

RETURN VALUE
  None

SIDE EFFECTS
  INTLOCK turn off interrupts
  INTFREE restore the interrupt mask saved previously

===========================================================================*/
#if defined(PC_EMULATOR_H) && ! defined(T_REXNT)

   #define PC_EMULATOR_INTLOCK
   #include PC_EMULATOR_H
   #undef PC_EMULATOR_INTLOCK

#elif defined(T_WINNT)

   #define INTLOCK()
   #define INTFREE()

#elif !defined( _ARM_ASM_)

//  #include "rex.h"
//  #define  INTLOCK( )  { dword sav = rex_int_lock();
//  #define  INTFREE( )  if(!sav) (void)rex_int_free();}

   #define INTLOCK()
   #define INTFREE()

#endif



/*===========================================================================

MACRO INTLOCK_SAV / INTFREE_SAV

DESCRIPTION
  INTLOCK_SAV Saves current interrupt state in specified variable sav_var
  then disables interrupts.  Used in conjunction with INTFREE_SAV.

  INTFREE_SAV Restores previous interrupt state from specified variable
  sav_var.  Used in conjunction with INTLOCK_SAV.

PARAMETERS
  sav_var       Current flags register, including interrupt status

DEPENDENCIES
  None.

RETURN VALUE
  None

SIDE EFFECTS
  INTLOCK_SAV turn off interrupts
  INTFREE_SAV restore the interrupt mask saved previously

===========================================================================*/


#ifdef _lint    /* get lint to 'know' the parameter is accessed */

  #define  INTLOCK_SAV(sav_var)   (sav_var = 1)
  #define  INTFREE_SAV(sav_var)   (sav_var = sav_var + 1)

#else

   #if defined( PC_EMULATOR_H) && !defined( T_REXNT)
     
      #define PC_EMULATOR_INTLOCKSAV
      #include PC_EMULATOR_H
      #undef  PC_EMULATOR_INTLOCKSAV

   #elif defined (T_WINNT)

     #define  INTLOCK_SAV( sav_var )
     #define  INTFREE_SAV( sav_var )

   #elif !defined( _ARM_ASM_)

//     #include "rex.h"
//     #define  INTLOCK_SAV( sav_var )  sav_var = rex_int_lock()
//     #define  INTFREE_SAV( sav_var )  if(!sav_var) rex_int_free()
     #define  INTLOCK_SAV(sav_var)   (sav_var = 1)
     #define  INTFREE_SAV(sav_var)   (sav_var = sav_var + 1)

   #else

     #define  INTLOCK_SAV(sav_var)   (sav_var = 1)
     #define  INTFREE_SAV(sav_var)   (sav_var = sav_var + 1)
   
   #endif

#endif /* END if _lint */



/*===========================================================================

MACRO UPCASE

DESCRIPTION
  Convert a character to uppercase, the character does not have to
  be printable or a letter.

PARAMETERS
  c             Character to be converted

DEPENDENCIES
  'c' is referenced multiple times, and should remain the same value
  each time it is evaluated.

RETURN VALUE
  Uppercase equivalent of the character parameter

SIDE EFFECTS
  None

===========================================================================*/

#define  UPCASE( c ) ( ((c) >= 'a' && (c) <= 'z') ? ((c) - 0x20) : (c) )


/*===========================================================================

MACRO DECCHK
MACRO HEXCHK

DESCRIPTION
  These character attribute macros are similar to the standard 'C' macros
  (isdec and ishex), but do not rely on the character attributes table used
  by Microsoft 'C'.

PARAMETERS
  c             Character to be examined

DEPENDENCIES
  None

RETURN VALUE
  DECCHK        True if the character is a decimal digit, else False
  HEXCHK        True if the chacters is a hexidecimal digit, else False

SIDE EFFECTS
  None

===========================================================================*/

#define  DECCHK( c ) ((c) >= '0' && (c) <= '9')

#define  HEXCHK( c ) ( ((c) >= '0' && (c) <= '9') ||\
                       ((c) >= 'A' && (c) <= 'F') ||\
                       ((c) >= 'a' && (c) <= 'f') )


/*===========================================================================

MACRO INC_SAT

DESCRIPTION
  Increment a value, but saturate it at its maximum positive value, do not
  let it wrap back to 0 (unsigned) or negative (signed).

PARAMETERS
  val           value to be incremented with saturation

DEPENDENCIES
  None

RETURN VALUE
  val

SIDE EFFECTS
  val is updated to the new value

===========================================================================*/

#define  INC_SAT( val )  (val = ((val)+1 > (val)) ? (val)+1 : (val))


/*===========================================================================

MACRO ARR_SIZE

DESCRIPTION
  Return the number of elements in an array.

PARAMETERS
  a             array name

DEPENDENCIES
  None

RETURN VALUE
  Number of elements in array a

SIDE EFFECTS
  None.

===========================================================================*/

#define  ARR_SIZE( a )  ( sizeof( (a) ) / sizeof( (a[0]) ) )

/*===========================================================================
                     MACRO MOD_BY_POWER_OF_TWO

     Will calculate x % y, where x is a non-negative integer and
     y is a power of 2 from 2^0..2^32.

     Will work for 2^0, 2^1, 2^2, 2^3, 2^4, ... 2^32
      ie.            1,   2,   4,   8,  16, ... 4294967296
===========================================================================*/

#define MOD_BY_POWER_OF_TWO( val, mod_by ) \
           ( (dword)(val) & (dword)((mod_by)-1) )

/* -------------------------------------------------------------------------
**                       Debug Declarations 
** ------------------------------------------------------------------------- */

/* Define 'SHOW_STAT' in order to view static's as globals
** (e.g. cl /DSHOW_STAT foo.c) If 'SHOW_STAT' is not defined,
** it gets defined as 'static'
*/
#ifdef LOCAL
#undef LOCAL
#endif

#ifdef SHOW_STAT
  #define LOCAL
#else
  #define LOCAL static
#endif


/*===========================================================================

                      FUNCTION DECLARATIONS

===========================================================================*/



/*===========================================================================

FUNCTION inp, outp, inpw, outpw, inpdw, outpdw

DESCRIPTION
  IN/OUT port macros for byte and word ports, typically inlined by compilers
  which support these routines

PARAMETERS
  inp(   xx_addr )
  inpw(  xx_addr )
  inpdw( xx_addr )
  outp(   xx_addr, xx_byte_val  )
  outpw(  xx_addr, xx_word_val  )
  outpdw( xx_addr, xx_dword_val )
      xx_addr      - Address of port to read or write (may be memory mapped)
      xx_byte_val  - 8 bit value to write
      xx_word_val  - 16 bit value to write
      xx_dword_val - 32 bit value to write

DEPENDENCIES
  None

RETURN VALUE
  inp/inpw/inpdw: the byte, word or dword read from the given address
  outp/outpw/outpdw: the byte, word or dword written to the given address

SIDE EFFECTS
  None.

===========================================================================*/

#ifdef PC_EMULATOR_H

  /* For PC emulation, include a header which defines inp/outp/inpw/outpw
  ** with the semantics above
  */
  #define PC_EMULATOR_IO
  #include PC_EMULATOR_H
  #undef  PC_EMULATOR_IO

#else 

  /* ARM based targets use memory mapped i/o, so the inp/outp calls are
  ** macroized to access memory directly
  */

#ifndef CDVI_SVTB
#if defined(VV_FEATURE_COMPILING_64BIT)
    #define inp(port)         (*((volatile dword *)  (port)))
    #define inpb(port)        (*((volatile byte *)   (port)))
    #define inpw(port)        (*((volatile word *)   (port)))
    #define inpdw(port)       (*((volatile dword *)  (port)))
    #define inp64(port)       (*((volatile uint64 *) (port)))

    #define outp(port, val)   (*((volatile dword *)  (port)) = ((dword)  (val)))
    #define outpb(port, val)  (*((volatile byte *)   (port)) = ((byte)   (val)))
    #define outpw(port, val)  (*((volatile word *)   (port)) = ((word)   (val)))
    #define outpdw(port, val) (*((volatile dword *)  (port)) = ((dword)  (val)))
    #define outp64(port, val) (*((volatile uint64 *) (port)) = ((uint64) (val)))
#else
    #define inp(port)         (*((volatile byte *) (port)))
    #define inpw(port)        (*((volatile word *) (port)))
    #define inpdw(port)       (*((volatile dword *)(port)))

    #define outp(port, val)   (*((volatile byte *) (port)) = ((byte) (val)))
    #define outpw(port, val)  (*((volatile word *) (port)) = ((word) (val)))
    #define outpdw(port, val) (*((volatile dword *) (port)) = ((dword) (val)))
#endif // VV_FEATURE_COMPILING_64BIT

#else  // CDVI_SVTB
#ifdef CDVI_SVTB_VERBOSE
  #define inp(port)         ((printf("CDVI_SVTB_VERBOSE: Calling bus_read from %s:%0d\n", __FILE__,__LINE__)&0) + bus_read(port))
#else
  #define inp(port)         bus_read(port)
#endif
  #define inpw(port)        inp(port)
  #define inpdw(port)       inp(port)
  
#ifdef CDVI_SVTB_VERBOSE
  #define outp(port, val)   ((printf("CDVI_SVTB_VERBOSE: Calling bus_write from %s:%0d\n",__FILE__,__LINE__)&0) + bus_write (port, val))
#else
  #define outp(port, val)   bus_write (port, val)
#endif
  #define outpw(port, val)  outp (port, val)
  #define outpdw(port, val) outp (port, val)
#endif
  #define outp32  outpdw

#endif


/*===========================================================================

FUNCTION enable, disable

DESCRIPTION
  Interrupt enable and disable routines.  Enable should cause the CPU to
  allow interrupts and disable should cause the CPU to disallow 
  interrupts
  
PARAMETERS
  None
  
DEPENDENCIES
  None

RETURN VALUE
  None
  
SIDE EFFECTS
  None.

===========================================================================*/

#ifndef T_WINNT

  /* Common definitions */
  #define VV_ENABLE 1
  #define VV_DISABLE 0
  #define VV_REG_MASK_BIT0 0x0001
  #define VV_REG_MASK_BIT1 0x0002
  #define VV_REG_MASK_BIT2 0x0004
  #define VV_REG_MASK_BIT3 0x0008
  #define VV_REG_MASK_BIT4 0x0010
  #define VV_REG_MASK_BIT5 0x0020
  #define VV_REG_MASK_BIT6 0x0040
  #define VV_REG_MASK_BIT7 0x0080
  
  /* ARM has no such definition, so we provide one here to enable/disable
  ** interrupts
  */
  #define _disable() (void)rex_int_lock()
  #define _enable()  (void)rex_int_free()
  
#endif

#endif /* SWIG */
#endif


