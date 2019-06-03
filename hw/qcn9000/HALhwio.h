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

#ifndef HAL_HWIO_H
#define HAL_HWIO_H

/*===========================================================================
 *
 * HWIO REGISTER INPUT/OUTPUT HEADER FILE
 *
 * DESCRIPTION
 * This interface is a replacement for that represented by
 * msm<ASIC>reg.h, msm<ASIC>io.h and msm<ASIC>redefs.h. For further background
 * and documentation on this interface, please see word docs and ppt docs at
 *
 * This file, together with msmhwioreg.h, provides a portable interface
 * from the perspective of chip address file coupling to DMSS register
 * I/O software. The interface defined by msmhwio.h and msmhwioreg.h
 * encapsulates register name, size, type and access method.
 *
 * msmhwioreg.h is an auto-generated file that may be subsequently edited
 * by hand to handle arbitrary register and bit field name changes, and
 * register type and size changes. When hand editing, one does not directly
 * hand edit msmhwioreg.h or msm<ASIC>reg.h except to cut/paste registers
 * or bitmasks from msm<ASIC>reg.h to msm<ASIC>reg_port.h or from msmhwioreg.h
 * to msmhwioreg_port.h. One edits an msmreg.pl input file, possibly named
 * something like msmreg_port.dat, to add patterns which msmreg.pl matches
 * during chip address file parsing. If a pattern from input the port.dat input
 * file is matched, that register or bitmask is not ouput to the register
 * header files, but is output to the *_port.h versions of the register header
 * files. This mechanism allows hand edit of register header file without loss
 * of hand edits on each successive re-parse of chip address file. See msmreg.pl
 * header comments for more information.
 *
 * msmhwioreg.h also handles register access method changes by allowing users
 * to change a registers I/O functions on a per register basis. By default,
 * when auto-generated, all register I/O functions are selected from the
 * inline functions listed below.
 *
 * (Note that currently inline functions are not used. This is because
 * rex_int_lock/rex_int_free are not inline and there the masked IO functions
 * would not inline since the compiler will not inline a function that calls
 * a function. The task of figuring out how to inline rex_int_lock has been
 * deferred. So, for the time being, the functions described below are
 * implemented as #define marcos.)
 *
 * To customize register I/O for a single register, one follows the porting
 * mechanism mentioned above and changes the input/output function for the
 * desired register.
 *
 * Direct, un-encapsulated, use of register addresses or register header file
 * components is discouraged. To that end, the interface described here fully
 * encapsulates all the CAF components including register address, bit field mask,
 * and bit field shift values in addition to register input and output.
 *
 * INLINE byte in_byte(dword addr)
 * INLINE byte in_byte_masked(dword addr, dword mask)
 * INLINE void out_byte(dword addr, byte val)
 * INLINE void out_byte_masked(dword io, dword mask, byte val)
 *
 * INLINE word in_word(dword addr)
 * INLINE word in_word_masked(dword addr, dword mask)
 * INLINE void out_word(dword addr, word val)
 * INLINE void out_word_masked(dword io, dword mask, word val)
 *
 * INLINE dword in_dword(dword addr)
 * INLINE dword in_dword_masked(dword addr, dword mask)
 * INLINE void out_dword(dword addr, dword val)
 * INLINE void out_dword_masked(dword io, dword mask, dword val)
 *
 * Initially, then there is a one-to-one correspondance between the inline
 * functions above and the macro interface below with the inline functions
 * intended as implementation detail of the macro interface. Register access
 * method changes of arbitrary complexity are handled by writing the
 * appropriate new I/O function and replacing the existing one for the
 * register under consideration.
 *
 * The interface described below takes chip address file (CAF) symbols as input.
 * Wherever hwiosym, hsio_regsym, or hwio_fldsym appears below, that is
 * a CAF register or bit field name.
 *
 *
 * #define HWIO_IN(hwiosym)
 *    Perform input on register hwiosym. Replaces MSM_IN, MSM_INH and MSM_INW.
 *
 * #define HWIO_INM(hwiosym, m)
 *    Perform masked input on register hwiosym applying mask m.
 *    Replaces MSM_INM, MSM_INHM, and MSM_INWM.
 *
 * #define HWIO_INF(hwio_regsym, hwio_fldsym)
 *    Read the value from the given field in the given register.
 *
 * #define HWIO_OUT(hwiosym, val)
 *    Write input val on register hwiosym. Replaces MSM_OUT, MSM_OUTH, and
 *    MSM_OUTW.
 *
 * #define HWIO_OUTM(hwiosym, mask, val)
 *    Write input val on register hwiosym. Input mask is applied to shadow
 *    memory and val such that only bits in mask are updated on register hwiosym
 *    and shadow memory. Replaces MSM_OUTM, MSM_OUTHM, and MSM_OUTWM.
 *
 * #define HWIO_OUTF(hwio_regsym, hwio_fldsym, val)
 *    Set the given field in the given register to the given value.
 *
 * #define HWIO_ADDR(hwiosym)
 *    Get the register address of hwiosym. Replaces the unencapulsated use of
 *    of numeric literal HEX constants for register address.
 *
 * #define HWIO_RMSK(hwiosym)
 *    Get the mask describing valid bits in register hwiosym. Replaces direct,
 *    unencapsulated use of mask symbol from register header file.
 *
 * #define HWIO_RSHFT(hwiosym)
 *    Get the shift value of the least significant bit in register hwiosym.a
 *    Replaces SHIFT_FROM_MASK.
 *
 * #define HWIO_FMSK(hwio_regsym, hwio_fldsym)
 *    Get the bit field mask for bit field hwio_fldsym in register hwio_regsym.
 *    Replaces the direct, unencapsulated use of bit field masks from register
 *    header file.
 *
 * #define HWIO_SHFT(hwio_regsym, hwio_fldsym)
 *    Get the shift value of the least significant bit of bit field hwio_fldsym
 *    in register hwio_regsym. Replaces SHIFT_FROM_MASK.
 *
 *===========================================================================*/

/*===========================================================================
 *
 * EDIT HISTORY FOR FILE
 *
 * This section contains comments describing changes made to this file.
 * Notice that changes are listed in reverse chronological order.
 *
 * when       who     what, where, why
 * --------   ---     ----------------------------------------------------------
 * 02/01/08   gfr     Wrap passed in mask with parenthesis.
 * 12/06/07   gfr     More depreciated MSM macros.
 * 11/19/07   gfr     Added MSM_R macro.
 * 11/19/07   gfr     Removed obselete hwio_interrupts_locked_by_user.
 * 10/26/07   gfr     Fix MSM_SET macro.
 * 10/10/07   gfr     Added mappings for MSM_R_MASKED and some others.
 * 09/21/07   gfr     Added mapping for MSM_W_MASKED_i.
 * 09/21/07   gfr     Merged in changes for WinCE and 7k.
 * 04/24/07   gfr     Support for HWIO_INTLOCK/INTFREE to remove lint warnings.
 * 04/09/07   gfr     Cleanup, compile support for HWIO_INTLOCK/HWIO_INTFREE.
 * 09/19/06   eav     Added HWIO_PHYS, HWIO_PHYSI, HWIO_PHYSI2
 * 08/28/06   gfr     Added HWIO_RMSKI, HWIO_SHDW, HWIO_SHDWI
 * 06/23/06   gfr     Added HWIO_INF, HWIO_INFI, HWIO_OUTF and HWIO_OUTFI
 * 06/16/05   eav     Changed  __msmhwio_addri to call HWIO_##hwiosym##_ADDRI
 * 10/10/02   aks     Map MSM_OUTSH() used by the MSM6200, to the equivalent
 *                    macro in the MSM6300.
 * 4/29/02    cr      initial revision
 *===========================================================================*/


/*===========================================================================
 *
 *                         INCLUDE FILES
 *
 *===========================================================================*/

#ifdef FEATURE_WINCE_OAL
#include <windows.h>
#include <nkintr.h>
#endif

#ifndef _ARM_ASM_
#include "HALcomdef.h"
#endif



/*===========================================================================
 *
 *                         EXTERNAL DECLARATIONS
 *
 *===========================================================================*/
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _ARM_ASM_
extern uint32 rex_int_lock(void);
extern uint32 rex_int_free(void);

uint32 pti_inpdw(uint32 * port);
void pti_outpdw(uint32 * port, uint32 val);
#endif

#ifdef __cplusplus
}
#endif


/* -----------------------------------------------------------------------
** Macros
** ----------------------------------------------------------------------- */

/** 
  @addtogroup macros
  @{ 
*/ 

/**
 * Map a base name to the pointer to access the base.
 *
 * This macro maps a base name to the pointer to access the base.
 * This is generally just used internally.
 *
 */
#define HWIO_BASE_PTR(base) base##_BASE_PTR


/**
 * Declare a HWIO base pointer.
 *
 * This macro will declare a HWIO base pointer data structure.  The pointer
 * will always be declared as a weak symbol so multiple declarations will
 * resolve correctly to the same data at link-time.
 */
#ifdef __ARMCC_VERSION
  #define DECLARE_HWIO_BASE_PTR(base) __weak uint8 *HWIO_BASE_PTR(base)
#else
  #define DECLARE_HWIO_BASE_PTR(base) uint8 *HWIO_BASE_PTR(base)
#endif

/**
  @}
*/


/** 
  @addtogroup hwio_macros
  @{ 
*/ 

/**
 * @name Address Macros
 *
 * Macros for getting register addresses.
 * These macros are used for retrieving the address of a register.
 * HWIO_ADDR* will return the directly accessible address (virtual or physical based
 * on environment), HWIO_PHYS* will always return the physical address.
 * The offset from the base region can be retrieved using HWIO_OFFS*.
 * The "X" extension is used for explicit addressing where the base address of
 * the module in question is provided as an argument to the macro.
 *
 * @{
 */
#define HWIO_ADDR(hwiosym)                               __msmhwio_addr(hwiosym)
#define HWIO_ADDRI(hwiosym, index)                       __msmhwio_addri(hwiosym, index)
#define HWIO_ADDRI2(hwiosym, index1, index2)             __msmhwio_addri2(hwiosym, index1, index2)
#define HWIO_ADDRI3(hwiosym, index1, index2, index3)     __msmhwio_addri3(hwiosym, index1, index2, index3)

#define HWIO_ADDRX(base, hwiosym)                           __msmhwio_addrx(base, hwiosym)
#define HWIO_ADDRXI(base, hwiosym, index)                   __msmhwio_addrxi(base, hwiosym, index)
#define HWIO_ADDRXI2(base, hwiosym, index1, index2)         __msmhwio_addrxi2(base, hwiosym, index1, index2)
#define HWIO_ADDRXI3(base, hwiosym, index1, index2, index3) __msmhwio_addrxi3(base, hwiosym, index1, index2, index3)


#define HWIO_PHYS(hwiosym)                               __msmhwio_phys(hwiosym)
#define HWIO_PHYSI(hwiosym, index)                       __msmhwio_physi(hwiosym, index)
#define HWIO_PHYSI2(hwiosym, index1, index2)             __msmhwio_physi2(hwiosym, index1, index2)
#define HWIO_PHYSI3(hwiosym, index1, index2, index3)     __msmhwio_physi3(hwiosym, index1, index2, index3)

#define HWIO_PHYSX(base, hwiosym)                           __msmhwio_physx(base, hwiosym)
#define HWIO_PHYSXI(base, hwiosym, index)                   __msmhwio_physxi(base, hwiosym, index)
#define HWIO_PHYSXI2(base, hwiosym, index1, index2)         __msmhwio_physxi2(base, hwiosym, index1, index2)
#define HWIO_PHYSXI3(base, hwiosym, index1, index2, index3) __msmhwio_physxi3(base, hwiosym, index1, index2, index3)

#define HWIO_OFFS(hwiosym)                               __msmhwio_offs(hwiosym)
#define HWIO_OFFSI(hwiosym, index)                       __msmhwio_offsi(hwiosym, index)
#define HWIO_OFFSI2(hwiosym, index1, index2)             __msmhwio_offsi2(hwiosym, index1, index2)
#define HWIO_OFFSI3(hwiosym, index1, index2, index3)     __msmhwio_offsi3(hwiosym, index1, index2, index3)

/** @} */


/*===========================================================================
 *
 *                         MACRO DECLARATIONS
 *
 *===========================================================================*/

#define HWIO_POR(io)                                  HWIO_##io##_POR

/**
 * @name Input Macros
 *
 * These macros are used for reading from a named hardware register.  Register
 * arrays ("indexed") use the macros with the "I" suffix.  The "M" suffix
 * indicates that the input will be masked with the supplied mask.  The HWIO_INF*
 * macros take a field name and will do the appropriate masking and shifting
 * to return just the value of that field.
 * The "X" extension is used for explicit addressing where the base address of
 * the module in question is provided as an argument to the macro.
 *
 * Generally you want to use either HWIO_IN or HWIO_INF (with required indexing).
 *
 * @{
 */

#define HWIO_IN(hwiosym)                                         __msmhwio_in(hwiosym)
#define HWIO_INI(hwiosym, index)                                 __msmhwio_ini(hwiosym, index)
#define HWIO_INI2(hwiosym, index1, index2)                       __msmhwio_ini2(hwiosym, index1, index2)
#define HWIO_INI3(hwiosym, index1, index2, index3)               __msmhwio_ini3(hwiosym, index1, index2, index3)

#define HWIO_INM(hwiosym, mask)                                  __msmhwio_inm(hwiosym, mask)
#define HWIO_INMI(hwiosym, index, mask)                          __msmhwio_inmi(hwiosym, index, mask)
#define HWIO_INMI2(hwiosym, index1, index2, mask)                __msmhwio_inmi2(hwiosym, index1, index2, mask)
#define HWIO_INMI3(hwiosym, index1, index2, index3, mask)        __msmhwio_inmi3(hwiosym, index1, index2, index3, mask)


#define HWIO_INF(io, field)                                      (HWIO_INM(io, HWIO_FMSK(io, field)) >> HWIO_SHFT(io, field))
#define HWIO_INFI(io, index, field)                              (HWIO_INMI(io, index, HWIO_FMSK(io, field)) >> HWIO_SHFT(io, field))
#define HWIO_INFI2(io, index1, index2, field)                    (HWIO_INMI2(io, index1, index2, HWIO_FMSK(io, field)) >> HWIO_SHFT(io, field))
#define HWIO_INFI3(io, index1, index2, index3, field)            (HWIO_INMI3(io, index1, index2, index3, HWIO_FMSK(io, field)) >> HWIO_SHFT(io, field))

#define HWIO_INX(base, hwiosym)                                  __msmhwio_inx(base, hwiosym)
#define HWIO_INXI(base, hwiosym, index)                          __msmhwio_inxi(base, hwiosym, index)
#define HWIO_INXI2(base, hwiosym, index1, index2)                __msmhwio_inxi2(base, hwiosym, index1, index2)
#define HWIO_INXI3(base, hwiosym, index1, index2, index3)        __msmhwio_inxi3(base, hwiosym, index1, index2, index3)

#define HWIO_INXM(base, hwiosym, mask)                           __msmhwio_inxm(base, hwiosym, mask)
#define HWIO_INXMI(base, hwiosym, index, mask)                   __msmhwio_inxmi(base, hwiosym, index, mask)
#define HWIO_INXMI2(base, hwiosym, index1, index2, mask)         __msmhwio_inxmi2(base, hwiosym, index1, index2, mask)
#define HWIO_INXMI3(base, hwiosym, index1, index2, index3, mask) __msmhwio_inxmi3(base, hwiosym, index1, index2, index3, mask)

#define HWIO_INXF(base, io, field)                               (HWIO_INXM(base, io, HWIO_FMSK(io, field)) >> HWIO_SHFT(io, field))
#define HWIO_INXFI(base, io, index, field)                       (HWIO_INXMI(base, io, index, HWIO_FMSK(io, field)) >> HWIO_SHFT(io, field))
#define HWIO_INXFI2(base, io, index1, index2, field)             (HWIO_INXMI2(base, io, index1, index2, HWIO_FMSK(io, field)) >> HWIO_SHFT(io, field))
#define HWIO_INXFI3(base, io, index1, index2, index3, field)     (HWIO_INXMI3(base, io, index1, index2, index3, HWIO_FMSK(io, field)) >> HWIO_SHFT(io, field))
/** @} */



/**
 * @name Output Macros
 *
 * These macros are used for writing to a named hardware register.  Register
 * arrays ("indexed") use the macros with the "I" suffix.  The "M" suffix
 * indicates that the output will be masked with the supplied mask (meaning these
 * macros do a read first, mask in the supplied data, then write it back).
 * The "X" extension is used for explicit addressing where the base address of
 * the module in question is provided as an argument to the macro.
 * The HWIO_OUTF* macros take a field name and will do the appropriate masking
 * and shifting to output just the value of that field.
 * HWIO_OUTV* registers take a named value instead of a numeric value and
 * do the same masking/shifting as HWIO_OUTF.
 *
 * Generally you want to use either HWIO_OUT or HWIO_OUTF (with required indexing).
 *
 * @{
 */

#define HWIO_OUT(hwiosym, val)                                   __msmhwio_out(hwiosym, val)
#define HWIO_OUTI(hwiosym, index, val)                           __msmhwio_outi(hwiosym, index, val)
#define HWIO_OUTI2(hwiosym, index1, index2, val)                 __msmhwio_outi2(hwiosym, index1, index2, val)
#define HWIO_OUTI3(hwiosym, index1, index2, index3, val)         __msmhwio_outi3(hwiosym, index1, index2, index3, val)

#define HWIO_OUTM(hwiosym, mask, val)                            __msmhwio_outm(hwiosym, mask, val)
#define HWIO_OUTMI(hwiosym, index, mask, val)                    __msmhwio_outmi(hwiosym, index, mask, val)
#define HWIO_OUTMI2(hwiosym, index1, index2, mask, val)          __msmhwio_outmi2(hwiosym, index1, index2, mask, val)
#define HWIO_OUTMI3(hwiosym, index1, index2, index3, mask, val)  __msmhwio_outmi3(hwiosym, index1, index2, index3, mask, val)

#define HWIO_OUTF(io, field, val)                                HWIO_OUTM(io, HWIO_FMSK(io, field), (uint32)(val) << HWIO_SHFT(io, field))
#define HWIO_OUTFI(io, index, field, val)                        HWIO_OUTMI(io, index, HWIO_FMSK(io, field), (uint32)(val) << HWIO_SHFT(io, field))
#define HWIO_OUTFI2(io, index1, index2, field, val)              HWIO_OUTMI2(io, index1, index2, HWIO_FMSK(io, field), (uint32)(val) << HWIO_SHFT(io, field))
#define HWIO_OUTFI3(io, index1, index2, index3, field, val)      HWIO_OUTMI3(io, index1, index2, index3, HWIO_FMSK(io, field), (uint32)(val) << HWIO_SHFT(io, field))


//----------------------------------------------------------------------------------------------
//<DVICP> added for Waverider
#define HWIO_OUTF2(io, field2, field1, val2, val1)   HWIO_OUTM(io, (HWIO_FMSK(io, field2)|HWIO_FMSK(io, field1)), \
                                                     ( (val2<<HWIO_SHFT(io, field2))|(val1<<HWIO_SHFT(io, field1))) )
#define HWIO_OUTF3(io, field3, field2, field1, val3, val2, val1)   HWIO_OUTM(io, (HWIO_FMSK(io, field3)|HWIO_FMSK(io, field2)|HWIO_FMSK(io, field1)), \
                                                     ( (val3<<HWIO_SHFT(io, field3))|(val2<<HWIO_SHFT(io, field2))|(val1<<HWIO_SHFT(io, field1))) )
#define HWIO_OUTF4(io, field4, field3, field2, field1, val4, val3, val2, val1)   HWIO_OUTM(io, (HWIO_FMSK(io, field4) |HWIO_FMSK(io, field3)|HWIO_FMSK(io, field2)|HWIO_FMSK(io, field1)), \
                                                     ( (val4<<HWIO_SHFT(io, field4))|(val3<<HWIO_SHFT(io, field3))|(val2<<HWIO_SHFT(io, field2))|(val1<<HWIO_SHFT(io, field1))) )
#define HWIO_OUTF5(io, field5, field4, field3, field2, field1, val5, val4, val3, val2, val1)   HWIO_OUTM(io, (HWIO_FMSK(io, field5)|HWIO_FMSK(io, field4)|HWIO_FMSK(io, field3)|HWIO_FMSK(io, field2)|HWIO_FMSK(io, field1)), \
                                                     ( (val5<<HWIO_SHFT(io, field5))|(val4<<HWIO_SHFT(io, field4))|(val3<<HWIO_SHFT(io, field3))|(val2<<HWIO_SHFT(io, field2))|(val1<<HWIO_SHFT(io, field1))) )
#define HWIO_OUTF6(io, field6, field5, field4, field3, field2, field1, val6, val5, val4, val3, val2, val1)   HWIO_OUTM(io, (HWIO_FMSK(io, field6)|HWIO_FMSK(io, field5)|HWIO_FMSK(io, field4)|HWIO_FMSK(io, field3)|HWIO_FMSK(io, field2)|HWIO_FMSK(io, field1)), \
                                                     ( (val6<<HWIO_SHFT(io, field6))|(val5<<HWIO_SHFT(io, field5))|(val4<<HWIO_SHFT(io, field4))|(val3<<HWIO_SHFT(io, field3))|(val2<<HWIO_SHFT(io, field2))|(val1<<HWIO_SHFT(io, field1))) )

#define HWIO_OUTF2I(io, idx, field2, field1, val2, val1)  HWIO_OUTMI(io, idx, (HWIO_FMSK(io, field2)|HWIO_FMSK(io, field1)), \
                                                          ( (val2<<HWIO_SHFT(io, field2))|(val1<<HWIO_SHFT(io, field1))) )
#define HWIO_OUTF3I(io, idx, field3, field2, field1, val3, val2, val1)  HWIO_OUTMI(io, idx, (HWIO_FMSK(io, field3)|HWIO_FMSK(io, field2)|HWIO_FMSK(io, field1)), \
                                                          ( (val3<<HWIO_SHFT(io, field3))|(val2<<HWIO_SHFT(io, field2))|(val1<<HWIO_SHFT(io, field1))) )
#define HWIO_OUTF4I(io, idx, field4, field3, field2, field1, val4, val3, val2, val1)  HWIO_OUTMI(io, idx, (HWIO_FMSK(io, field4)|HWIO_FMSK(io, field3)|HWIO_FMSK(io, field2)|HWIO_FMSK(io, field1)), \
                                                          ( (val4<<HWIO_SHFT(io, field4))|(val3<<HWIO_SHFT(io, field3))|(val2<<HWIO_SHFT(io, field2))|(val1<<HWIO_SHFT(io, field1))) )
#define HWIO_OUTF5I(io, idx, field5, field4, field3, field2, field1, val5, val4, val3, val2, val1)  HWIO_OUTMI(io, idx, (HWIO_FMSK(io, field5)|HWIO_FMSK(io, field4)|HWIO_FMSK(io, field3)|HWIO_FMSK(io, field2)|HWIO_FMSK(io, field1)), \
                                                          ( (val5<<HWIO_SHFT(io, field5))|(val4<<HWIO_SHFT(io, field4))|(val3<<HWIO_SHFT(io, field3))|(val2<<HWIO_SHFT(io, field2))|(val1<<HWIO_SHFT(io, field1))) )
#define HWIO_OUTF6I(io, idx, field6, field5, field4, field3, field2, field1, val6, val5, val4, val3, val2, val1)  HWIO_OUTMI(io, idx, (HWIO_FMSK(io, field6)|HWIO_FMSK(io, field5)|HWIO_FMSK(io, field4)|HWIO_FMSK(io, field3)|HWIO_FMSK(io, field2)|HWIO_FMSK(io, field1)), \
                                                          ( (val6<<HWIO_SHFT(io, field6))|(val5<<HWIO_SHFT(io, field5))|(val4<<HWIO_SHFT(io, field4))|(val3<<HWIO_SHFT(io, field3))|(val2<<HWIO_SHFT(io, field2))|(val1<<HWIO_SHFT(io, field1))) )
// ----------------------------------------------------------------------------------------------


#define HWIO_OUTV(io, field, val)                                HWIO_OUTM(io, HWIO_FMSK(io, field), (uint32)(HWIO_VAL(io, field, val)) << HWIO_SHFT(io, field))
#define HWIO_OUTVI(io, index, field, val)                        HWIO_OUTMI(io, index, HWIO_FMSK(io, field), (uint32)(HWIO_VAL(io, field, val)) << HWIO_SHFT(io, field))
#define HWIO_OUTVI2(io, index1, index2, field, val)              HWIO_OUTMI2(io, index1, index2, HWIO_FMSK(io, field), (uint32)(HWIO_VAL(io, field, val)) << HWIO_SHFT(io, field))
#define HWIO_OUTVI3(io, index1, index2, index3, field, val)      HWIO_OUTMI3(io, index1, index2, index3, HWIO_FMSK(io, field), (uint32)(HWIO_VAL(io, field, val)) << HWIO_SHFT(io, field))

#define HWIO_OUTX(base, hwiosym, val)                                   __msmhwio_outx(base, hwiosym, val)
#define HWIO_OUTXI(base, hwiosym, index, val)                           __msmhwio_outxi(base, hwiosym, index, val)
#define HWIO_OUTXI2(base, hwiosym, index1, index2, val)                 __msmhwio_outxi2(base, hwiosym, index1, index2, val)
#define HWIO_OUTXI3(base, hwiosym, index1, index2, index3, val)         __msmhwio_outxi3(base, hwiosym, index1, index2, index3, val)

#define HWIO_OUTXM(base, hwiosym, mask, val)                            __msmhwio_outxm(base, hwiosym, mask, val)
#define HWIO_OUTXM2(base, hwiosym, mask1, mask2, val1, val2)  __msmhwio_outxm2(base, hwiosym, mask1, mask2, val1, val2)
#define HWIO_OUTXM3(base, hwiosym, mask1, mask2, mask3, val1, val2, val3) __msmhwio_outxm3(base, hwiosym, mask1, mask2, mask3, val1, val2, val3)
#define HWIO_OUTXM4(base, hwiosym, mask1, mask2, mask3, mask4, val1, val2, val3, val4) __msmhwio_outxm4(base, hwiosym, mask1, mask2, mask3, mask4, val1, val2, val3, val4)
#define HWIO_OUTXMI(base, hwiosym, index, mask, val)                    __msmhwio_outxmi(base, hwiosym, index, mask, val)
#define HWIO_OUTXMI2(base, hwiosym, index1, index2, mask, val)          __msmhwio_outxmi2(base, hwiosym, index1, index2, mask, val)
#define HWIO_OUTXMI3(base, hwiosym, index1, index2, index3, mask, val)  __msmhwio_outxmi3(base, hwiosym, index1, index2, index3, mask, val)

#define HWIO_OUTXF(base, io, field, val)                                HWIO_OUTXM(base, io, HWIO_FMSK(io, field), (uint32)(val) << HWIO_SHFT(io, field))
#define HWIO_OUTX2F(base, io, field1, field2, val1, val2)                                HWIO_OUTXM2(base, io, HWIO_FMSK(io, field1),  HWIO_FMSK(io, field2), (uint32)(val1) << HWIO_SHFT(io, field1), (uint32)(val2) << HWIO_SHFT(io, field2))
#define HWIO_OUTX3F(base, io, field1, field2, field3, val1, val2, val3)                                HWIO_OUTXM3(base, io, HWIO_FMSK(io, field1),  HWIO_FMSK(io, field2),  HWIO_FMSK(io, field3),(uint32)(val1) << HWIO_SHFT(io, field1), (uint32)(val2) << HWIO_SHFT(io, field2), (uint32)(val3) << HWIO_SHFT(io, field3) )
#define HWIO_OUTX4F(base, io, field1, field2, field3, field4, val1, val2, val3, val4)                                HWIO_OUTXM4(base, io, HWIO_FMSK(io, field1),  HWIO_FMSK(io, field2),  HWIO_FMSK(io, field3),  HWIO_FMSK(io, field4), (uint32)(val1) << HWIO_SHFT(io, field1) , (uint32)(val2) << HWIO_SHFT(io, field2), (uint32)(val3) << HWIO_SHFT(io, field3), (uint32)(val4) << HWIO_SHFT(io, field4) )
#define HWIO_OUTXFI(base, io, index, field, val)                        HWIO_OUTXMI(base, io, index, HWIO_FMSK(io, field), (uint32)(val) << HWIO_SHFT(io, field))
#define HWIO_OUTXFI2(base, io, index1, index2, field, val)              HWIO_OUTXMI2(base, io, index1, index2, HWIO_FMSK(io, field), (uint32)(val) << HWIO_SHFT(io, field))
#define HWIO_OUTXFI3(base, io, index1, index2, index3, field, val)      HWIO_OUTXMI3(base, io, index1, index2, index3, HWIO_FMSK(io, field), (uint32)(val) << HWIO_SHFT(io, field))

#define HWIO_OUTXV(base, io, field, val)                                HWIO_OUTXM(base, io, HWIO_FMSK(io, field), (uint32)(HWIO_VAL(io, field, val)) << HWIO_SHFT(io, field))
#define HWIO_OUTXVI(base, io, index, field, val)                        HWIO_OUTXMI(base, io, index, HWIO_FMSK(io, field), (uint32)(HWIO_VAL(io, field, val)) << HWIO_SHFT(io, field))
#define HWIO_OUTXVI2(base, io, index1, index2, field, val)              HWIO_OUTXMI2(base, io, index1, index2, HWIO_FMSK(io, field), (uint32)(HWIO_VAL(io, field, val)) << HWIO_SHFT(io, field))
#define HWIO_OUTXVI3(base, io, index1, index2, index3, field, val)      HWIO_OUTXMI3(base, io, index1, index2, index3, HWIO_FMSK(io, field), (uint32)(HWIO_VAL(io, field, val)) << HWIO_SHFT(io, field))
/** @} */



/**
 * @name Shift and Mask Macros
 *
 * Macros for getting shift and mask values for fields and registers.
 *  HWIO_RMSK: The mask value for accessing an entire register.  For example:
 *             @code
 *             HWIO_RMSK(REG) -> 0xFFFFFFFF
 *             @endcode
 *  HWIO_RSHFT: The right-shift value for an entire register (rarely necessary).\n
 *  HWIO_SHFT: The right-shift value for accessing a field in a register.  For example:
 *             @code
 *             HWIO_SHFT(REG, FLD) -> 8
 *             @endcode
 *  HWIO_FMSK: The mask value for accessing a field in a register.  For example:
 *             @code
 *             HWIO_FMSK(REG, FLD) -> 0xFF00
 *             @endcode
 *  HWIO_VAL:  The value for a field in a register.  For example:
 *             @code
 *             HWIO_VAL(REG, FLD, ON) -> 0x1
 *             @endcode
 *  HWIO_FVAL: This macro takes a numerical value and will shift and mask it into
 *             the given field position.  For example:
 *             @code
 *             HWIO_FVAL(REG, FLD, 0x1) -> 0x100
 *             @endcode
 *  HWIO_FVALV: This macro takes a logical (named) value and will shift and mask it
 *              into the given field position.  For example:
 *              @code
 *              HWIO_FVALV(REG, FLD, ON) -> 0x100
 *              @endcode
 *
 * @{
 */
#define HWIO_RMSK(hwiosym)                               __msmhwio_rmsk(hwiosym)
#define HWIO_RMSKI(hwiosym, index)                       __msmhwio_rmski(hwiosym, index)
#define HWIO_RSHFT(hwiosym)                              __msmhwio_rshft(hwiosym)
#define HWIO_SHFT(hwio_regsym, hwio_fldsym)              __msmhwio_shft(hwio_regsym, hwio_fldsym)
#define HWIO_FMSK(hwio_regsym, hwio_fldsym)              __msmhwio_fmsk(hwio_regsym, hwio_fldsym)
#define HWIO_VAL(io, field, val)                         __msmhwio_val(io, field, val)
#define HWIO_FVAL(io, field, val)                        (((uint32)(val) << HWIO_SHFT(io, field)) & HWIO_FMSK(io, field))
#define HWIO_FVALV(io, field, val)                       (((uint32)(HWIO_VAL(io, field, val)) << HWIO_SHFT(io, field)) & HWIO_FMSK(io, field))
/** @} */


/**
 * @name Shadow Register Macros
 *
 * These macros are used for directly reading the value stored in a 
 * shadow register.
 * Shadow registers are defined for write-only registers.  Generally these
 * macros should not be necessary as HWIO_OUTM* macros will automatically use
 * the shadow values internally.
 *
 * @{
 */
#define HWIO_SHDW(hwiosym)                               __msmhwio_shdw(hwiosym)
#define HWIO_SHDWI(hwiosym, index)                       __msmhwio_shdwi(hwiosym, index)
/** @} */



/** 
  @}
*/ /* end_group */







#define __msmhwio_in(hwiosym)                                   HWIO_##hwiosym##_IN
#define __msmhwio_ini(hwiosym, index)                           HWIO_##hwiosym##_INI(index)
#define __msmhwio_ini2(hwiosym, index1, index2)                 HWIO_##hwiosym##_INI2(index1, index2)
#define __msmhwio_ini3(hwiosym, index1, index2, index3)         HWIO_##hwiosym##_INI3(index1, index2, index3)
#define __msmhwio_inm(hwiosym, mask)                            HWIO_##hwiosym##_INM(mask)
#define __msmhwio_inmi(hwiosym, index, mask)                    HWIO_##hwiosym##_INMI(index, mask)
#define __msmhwio_inmi2(hwiosym, index1, index2, mask)          HWIO_##hwiosym##_INMI2(index1, index2, mask)
#define __msmhwio_inmi3(hwiosym, index1, index2, index3, mask)  HWIO_##hwiosym##_INMI3(index1, index2, index3, mask)
#define __msmhwio_out(hwiosym, val)                             HWIO_##hwiosym##_OUT(val)
#define __msmhwio_outi(hwiosym, index, val)                     HWIO_##hwiosym##_OUTI(index,val)
#define __msmhwio_outi2(hwiosym, index1, index2, val)           HWIO_##hwiosym##_OUTI2(index1, index2, val)
#define __msmhwio_outi3(hwiosym, index1, index2, index3, val)   HWIO_##hwiosym##_OUTI2(index1, index2, index3, val)
#define __msmhwio_outm(hwiosym, mask, val)                      HWIO_##hwiosym##_OUTM(mask, val)
#define __msmhwio_outmi(hwiosym, index, mask, val)              HWIO_##hwiosym##_OUTMI(index, mask, val)
#define __msmhwio_outmi2(hwiosym, idx1, idx2, mask, val)        HWIO_##hwiosym##_OUTMI2(idx1, idx2, mask, val)
#define __msmhwio_outmi3(hwiosym, idx1, idx2, idx3, mask, val)  HWIO_##hwiosym##_OUTMI3(idx1, idx2, idx3, mask, val)

#define __msmhwio_addr(hwiosym)                                 HWIO_##hwiosym##_ADDR
#define __msmhwio_addri(hwiosym, index)                         HWIO_##hwiosym##_ADDR(index)
#define __msmhwio_addri2(hwiosym, idx1, idx2)                   HWIO_##hwiosym##_ADDR(idx1, idx2)
#define __msmhwio_addri3(hwiosym, idx1, idx2, idx3)             HWIO_##hwiosym##_ADDR(idx1, idx2, idx3)
#define __msmhwio_phys(hwiosym)                                 HWIO_##hwiosym##_PHYS
#define __msmhwio_physi(hwiosym, index)                         HWIO_##hwiosym##_PHYS(index)
#define __msmhwio_physi2(hwiosym, idx1, idx2)                   HWIO_##hwiosym##_PHYS(idx1, idx2)
#define __msmhwio_physi3(hwiosym, idx1, idx2, idx3)             HWIO_##hwiosym##_PHYS(idx1, idx2, idx3)
#define __msmhwio_offs(hwiosym)                                 HWIO_##hwiosym##_OFFS 
#define __msmhwio_offsi(hwiosym, index)                         HWIO_##hwiosym##_OFFS(index)
#define __msmhwio_offsi2(hwiosym, idx1, idx2)                   HWIO_##hwiosym##_OFFS(idx1, idx2)
#define __msmhwio_offsi3(hwiosym, idx1, idx2, idx3)             HWIO_##hwiosym##_OFFS(idx1, idx2, idx3)
#define __msmhwio_rmsk(hwiosym)                                 HWIO_##hwiosym##_RMSK
#define __msmhwio_rmski(hwiosym, index)                         HWIO_##hwiosym##_RMSK(index)
#define __msmhwio_fmsk(hwiosym, hwiofldsym)                     HWIO_##hwiosym##_##hwiofldsym##_BMSK
#define __msmhwio_rshft(hwiosym)                                HWIO_##hwiosym##_SHFT
#define __msmhwio_shft(hwiosym, hwiofldsym)                     HWIO_##hwiosym##_##hwiofldsym##_SHFT
#define __msmhwio_shdw(hwiosym)                                 HWIO_##hwiosym##_shadow
#define __msmhwio_shdwi(hwiosym, index)                         HWIO_##hwiosym##_SHDW(index)
#define __msmhwio_val(hwiosym, hwiofld, hwioval)                HWIO_##hwiosym##_##hwiofld##_##hwioval##_FVAL

#define __msmhwio_inx(base, hwiosym)                                  HWIO_##hwiosym##_IN(base)
#define __msmhwio_inxi(base, hwiosym, index)                          HWIO_##hwiosym##_INI(base, index)
#define __msmhwio_inxi2(base, hwiosym, index1, index2)                HWIO_##hwiosym##_INI2(base, index1, index2)
#define __msmhwio_inxi3(base, hwiosym, index1, index2, index3)        HWIO_##hwiosym##_INI3(base, index1, index2, index3)
#define __msmhwio_inxm(base, hwiosym, mask)                           HWIO_##hwiosym##_INM(base, mask)
#define __msmhwio_inxmi(base, hwiosym, index, mask)                   HWIO_##hwiosym##_INMI(base, index, mask)
#define __msmhwio_inxmi2(base, hwiosym, index1, index2, mask)         HWIO_##hwiosym##_INMI2(base, index1, index2, mask)
#define __msmhwio_inxmi3(base, hwiosym, index1, index2, index3, mask) HWIO_##hwiosym##_INMI3(base, index1, index2, index3, mask)
#define __msmhwio_outx(base, hwiosym, val)                            HWIO_##hwiosym##_OUT(base, val)
#define __msmhwio_outxi(base, hwiosym, index, val)                    HWIO_##hwiosym##_OUTI(base, index,val)
#define __msmhwio_outxi2(base, hwiosym, index1, index2, val)          HWIO_##hwiosym##_OUTI2(base, index1, index2, val)
#define __msmhwio_outxi3(base, hwiosym, index1, index2, index3, val)  HWIO_##hwiosym##_OUTI3(base, index1, index2, index3, val)
#define __msmhwio_outxm(base, hwiosym, mask, val)                     HWIO_##hwiosym##_OUTM(base, mask, val)
#define __msmhwio_outxm2(base, hwiosym, mask1, mask2, val1, val2)  {	\
                                                                                HWIO_##hwiosym##_OUTM(base, mask1, val1); \
                                                                                HWIO_##hwiosym##_OUTM(base, mask2, val2); \
                                                                               }
#define __msmhwio_outxm3(base, hwiosym, mask1, mask2, mask3,  val1, val2, val3) { \
                                                                                HWIO_##hwiosym##_OUTM(base, mask1, val1); \
                                                                                HWIO_##hwiosym##_OUTM(base, mask2, val2); \
                                                                                HWIO_##hwiosym##_OUTM(base, mask3, val3); \
                                                                               }  
#define __msmhwio_outxm4(base, hwiosym, mask1, mask2, mask3, mask4, val1, val2, val3, val4) { \
                                                                                HWIO_##hwiosym##_OUTM(base, mask1, val1); \
                                                                                HWIO_##hwiosym##_OUTM(base, mask2, val2); \
                                                                                HWIO_##hwiosym##_OUTM(base, mask3, val3); \
                                                                                HWIO_##hwiosym##_OUTM(base, mask4, val4); \
                                                                               } 
#define __msmhwio_outxmi(base, hwiosym, index, mask, val)             HWIO_##hwiosym##_OUTMI(base, index, mask, val)
#define __msmhwio_outxmi2(base, hwiosym, idx1, idx2, mask, val)       HWIO_##hwiosym##_OUTMI2(base, idx1, idx2, mask, val)
#define __msmhwio_outxmi3(base, hwiosym, idx1, idx2, idx3, mask, val) HWIO_##hwiosym##_OUTMI3(base, idx1, idx2, idx3, mask, val)
#define __msmhwio_addrx(base, hwiosym)                                HWIO_##hwiosym##_ADDR(base)
#define __msmhwio_addrxi(base, hwiosym, index)                        HWIO_##hwiosym##_ADDR(base, index)
#define __msmhwio_addrxi2(base, hwiosym, idx1, idx2)                  HWIO_##hwiosym##_ADDR(base, idx1, idx2)
#define __msmhwio_addrxi3(base, hwiosym, idx1, idx2, idx3)            HWIO_##hwiosym##_ADDR(base, idx1, idx2, idx3)
#define __msmhwio_physx(base, hwiosym)                                HWIO_##hwiosym##_PHYS(base)
#define __msmhwio_physxi(base, hwiosym, index)                        HWIO_##hwiosym##_PHYS(base, index)
#define __msmhwio_physxi2(base, hwiosym, idx1, idx2)                  HWIO_##hwiosym##_PHYS(base, idx1, idx2)
#define __msmhwio_physxi3(base, hwiosym, idx1, idx2, idx3)            HWIO_##hwiosym##_PHYS(base, idx1, idx2, idx3)





/*
 * MSM_x (depreciated)
 *
 * These macros should be replaced with the relevant HWIO equivalent.
 */
#define MSM_OUT(io, val)                HWIO_OUT(io, val)
#define MSM_IN(io)                      HWIO_IN(io)
#define MSM_OUTM(io, mask, val)         HWIO_OUTM(io, mask, val)
#define MSM_INM(io, mask)               HWIO_INM(io, mask)
#define MSM_INF(io, field)              HWIO_INF(io, field)
#define MSM_FIELD(field, val)           (((val) << field##_SHFT) & (field##_BMSK))
#define MSM_OUTSH(io, mask, val)        HWIO_OUTM(io, mask, val)
#define MSM_FOUTSH(io, field, val)      HWIO_OUTM(io, mask, val)
#define MSM_SET(io, field)              HWIO_OUTM(io, HWIO_FMSK(io, field), HWIO_FMSK(io, field))
#define MSM_SET_BIT(io, bit)            HWIO_OUTM(io, (1<<bit), (1<<bit))
#define MSM_SET_i(io, field, index)     HWIO_OUTMI(io, index, HWIO_FMSK(io,field), HWIO_FMSK(io,field))
#define MSM_SET_ij(io, field, index1, index2) HWIO_OUTMI2(io, index1, index2, HWIO_FMSK(io,field), HWIO_FMSK(io,field))
#define MSM_CLEAR(io, field)            HWIO_OUTM(io, HWIO_FMSK(io, field), 0)
#define MSM_CLEAR_BIT(io, bit)          HWIO_OUTM(io, (1<<bit), 0)
#define MSM_CLEAR_i(io, field, n)       HWIO_OUTMI(io, n, HWIO_FMSK(io,field), 0)
#define MSM_CLEAR_ij(io, field, m, n)   HWIO_OUTMI2(io, m, n, HWIO_FMSK(io,field), 0)
#define MSM_SET_MASK(io, mask)          HWIO_OUTM(io, mask, mask)
#define MSM_CLEAR_MASK(io, mask)        HWIO_OUTM(io, mask, 0)
#define MSM_PULSE(io, field)            HWIO_OUTF(io, field, 1); HWIO_OUTF(io, field, 0)
#define MSM_GET(io, field)              HWIO_INF(io, field)
#define MSM_W(io, val)                  HWIO_OUT(io, val)
#define MSM_W_i(io, val, index)         HWIO_OUTI(io, index, val)
#define MSM_W_ij(io, val, index1, index2) HWIO_OUTI2(io, index1, index2, val )
#define MSM_W_MASKED(io, field, val)    HWIO_OUTF(io, field, val)
#define MSM_W_MASKED_i(io, field, val, index)            HWIO_OUTFI(io, index, field, val)
#define MSM_W_MASKED_ij(io, field, val, index1, index2)  HWIO_OUTFI2(io, index1, index2, field, val)
#define MSM_R(io)                       HWIO_IN(io)
#define MSM_R_MASKED(base, field)       HWIO_INF(base, field)
#define MSM_R_MASKED_i(base, field, n)  HWIO_INFI(base, n, field)
#define MSM_R_MASKED_ij(base, field, index1, index2)  HWIO_INFI2(base, index1, index2, field)
#define MSM_GET_BIT(io, bit)            (HWIO_INM(io, (1<<bit) ) >> bit)
#define MSM_NOT_USED( i )               if(i) {}


/*
 * MSM_LOCK / MSM_UNLOCK (depreciated)
 *
 * These macros are intended to be used if a client will be issuing a
 * series of HWIO writes to avoid the multiple locking/freeing of interrupts
 * that will otherwise occur.
 * They should be replaced with HWIO_LOCK / HWIO_UNLOCK
 */
#define MSM_LOCK_REQUIRED                 \
  /*lint -save -e548 else expected*/      \
  MSM_LOCK_required = MSM_LOCK_required;  \
  /*lint -restore */

#define MSM_LOCK( )                   \
  {                                   \
    boolean MSM_LOCK_required = TRUE; \
    uint32 msm_sav;                   \
    INTLOCK_SAV(msm_sav)

#define MSM_UNLOCK( )      \
    MSM_LOCK_REQUIRED      \
    INTFREE_SAV(msm_sav);  \
  }

#define MSM_UNLOCK_AND_LOCK( ) \
    MSM_LOCK_REQUIRED          \
    INTFREE_SAV( msm_sav ),    \
    INTLOCK_SAV( msm_sav )


/*
 * INPxx / OUTPxx (depreciated)
 *
 */
#define INP32(addr)          in_dword(addr)
#define INP32M(addr, mask)   in_dword_masked(addr,mask)
#define OUTP32(addr, val)    out_dword(addr, val)
#define OUTP32M(addr, mask, val)                      \
   HWIO_INTLOCK();                                    \
   out_dword_masked_ns(addr, mask, val, INP32(addr)); \
   HWIO_INTFREE()


/*
 * HWIO_LOCK / HWIO_UNLOCK
 *
 * These macros are intended to be used if a client will be issuing a
 * series of HWIO writes to avoid the multiple locking/freeing of interrupts
 * that will otherwise occur.
 */
#define HWIO_LOCK()    \
  {                    \
    uint16 hwio_sav;   \
    INTLOCK_SAV(hwio_sav)

#define HWIO_UNLOCK()       \
    INTFREE_SAV(hwio_sav);  \
  }


#if defined(FEATURE_WINCE_BOOTLOADER)

  #define HWIO_INTLOCK()

  #define HWIO_INTFREE()

#elif defined(FEATURE_WINCE_OAL)

  #define HWIO_INTLOCK() \
   { \
     uint32 ints_already_enabled = INTERRUPTS_ENABLE(FALSE)

  #define HWIO_INTFREE() \
     if (ints_already_enabled) INTERRUPTS_ENABLE(TRUE); \
   }

#else

  #define HWIO_INTLOCK()     \
   {                         \
     uint32 intlock_sav;     \
     INTLOCK_SAV(intlock_sav)

  #define HWIO_INTFREE()       \
     INTFREE_SAV(intlock_sav); \
   }

#endif

#if defined (PTI_PRINTF_EN) || defined(PTI_MEMORY_EN) || defined(PTI_STM_EN)

  #define __inp(port)         (*((volatile uint8 *) (port)))
  #define __inpw(port)        (*((volatile uint16 *) (port)))
//  #define __inpdw(port)       (*((volatile uint32 *) (port)))
  #define __inpdw(port)       pti_inpdw((uint32 *) (port))

  #define __outp(port, val)   (*((volatile uint8 *) (port)) = ((uint8) (val)))
  #define __outpw(port, val)  (*((volatile uint16 *) (port)) = ((uint16) (val)))
//  #define __outpdw(port, val) (*((volatile uint32 *) (port)) = ((uint32) (val)))
  #define __outpdw(port, val) pti_outpdw((uint32 *) (port), (val))
#elif defined(__FWV) || defined(__KERNEL__) || defined(__SHARED_CODE) || defined(QURT_UFW_BUILD) || defined (_WIN)
extern uint32_t registerRead(unsigned long addr);
extern void registerWrite(unsigned long addr, uint32_t value);
#define __inp(port)       registerRead(port)
#define __inpw(port)       registerRead(port)
#define __inpdw(port)       registerRead(port)
#define __outp(port, val)  registerWrite(port, val)
#define __outpw(port, val)  registerWrite(port, val)
#define __outpdw(port, val)  registerWrite(port, val)
#else
#ifdef CDVI_SVTB_VERBOSE
  #define __inp(port)         ((printf("CDVI_SVTB_VERBOSE: Calling bus_read from %s:%0d\n", __FILE__,__LINE__)&0) + bus_read(port))
#else
  #define __inp(port)         bus_read(port)
#endif
  #define __inpw(port)        __inp(port)
  #define __inpdw(port)       __inp(port)
  
#ifdef CDVI_SVTB_VERBOSE
  #define __outp(port, val)   ((printf("CDVI_SVTB_VERBOSE: Calling bus_write from %s:%0d\n",__FILE__,__LINE__)&0) + bus_write (port, val))
#else
  #define __outp(port, val)   bus_write (port, val)
#endif
  #define __outpw(port, val)  __outp (port, val)
  #define __outpdw(port, val) __outp (port, val)
#endif


#define in_byte(addr)               (__inp(addr))
#define in_byte_masked(addr, mask)  (__inp(addr) & (mask))
#define out_byte(addr, val)         __outp(addr,val)
#define out_byte_masked(io, mask, val, shadow)  \
  HWIO_INTLOCK();    \
  (void) out_byte( io, shadow); \
  shadow = (shadow & (uint16)(~(mask))) | ((uint16)((val) & (mask))); \
  HWIO_INTFREE()
#define out_byte_masked_ns(io, mask, val, current_reg_content)  \
  (void) out_byte( io, ((current_reg_content & (uint16)(~(mask))) | \
                       ((uint16)((val) & (mask)))) )

#define in_word(addr)              (__inpw(addr))
#define in_word_masked(addr, mask) (__inpw(addr) & (mask))
#define out_word(addr, val)        __outpw(addr,val)
#define out_word_masked(io, mask, val, shadow)  \
  HWIO_INTLOCK( ); \
  shadow = (shadow & (uint16)(~(mask))) |  ((uint16)((val) & (mask))); \
  (void) out_word( io, shadow); \
  HWIO_INTFREE( )
#define out_word_masked_ns(io, mask, val, current_reg_content)  \
  (void) out_word( io, ((current_reg_content & (uint16)(~(mask))) | \
                       ((uint16)((val) & (mask)))) )

#define in_dword(addr)              (__inpdw(addr))
#define in_dword_masked(addr, mask) (__inpdw(addr) & (mask))
#define out_dword(addr, val)        __outpdw(addr,val)
#define out_dword_masked(io, mask, val, shadow)  \
   HWIO_INTLOCK( ); \
   shadow = (shadow & (uint32)(~(mask))) | ((uint32)((val) & (mask))); \
   (void) out_dword( io, shadow); \
   HWIO_INTFREE( )
#define out_dword_masked_ns(io, mask, val, current_reg_content) \
  (void) out_dword( io, ((current_reg_content & (uint32)(~(mask))) | ((uint32)((val) & (mask)))) )

/*
 * Base 64-bit quad-word accessing macros.
 */
#define in_qword(addr)              (__inpqw(addr))
#define in_qword_masked(addr, mask) (__inpqw(addr) & (mask))
#define out_qword(addr, val)        __outpqw(addr,val)
#define out_qword_masked(io, mask, val, shadow)  \
   HWIO_INTLOCK(); \
   shadow = (shadow & (uint64)(~(mask))) | ((uint64)((val) & (mask))); \
   out_qword( io, shadow); \
   HWIO_INTFREE()
#define out_qword_masked_ns(io, mask, val, current_reg_content) \
  out_qword( io, ((current_reg_content & (uint64)(~(mask))) | \
                 ((uint64)((val) & (mask)))) )

#define OUTP64(addr, val)    	out_qword(addr, val)
#define __outpqw(port, val) 	(*((volatile uint64 *) (port)) = ((uint64) (val)))

#define INP64(addr)    			in_qword(addr)
#define __inpqw(port)       	(*((volatile uint64 *) (port)))

#endif


