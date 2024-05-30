/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
/*
===========================================================================
*/
/**
  @file HALhwio.h 
  
  Public interface include file for accessing the HWIO HAL definitions.
  
  The HALhwio.h file is the public API interface to the HW I/O (HWIO)
  register access definitions.
*/

/*=========================================================================
      Include Files
==========================================================================*/


/*
 * Common types.
 */
#include "HALcomdef.h"



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


/** @cond */

/*
 * Map to final symbols.  This remapping is done to allow register 
 * redefinitions.  If we just define HWIO_IN(xreg) as HWIO_##xreg##_IN
 * then remappings like "#define xreg xregnew" do not work as expected.
 */
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
#define __msmhwio_outxi3(base, hwiosym, index1, index2, index3, val)  HWIO_##hwiosym##_OUTI2(base, index1, index2, index3, val)
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
 * HWIO_INTLOCK
 *
 * Macro used by autogenerated code for mutual exclusion around
 * read-mask-write operations.  This is not supported in HAL
 * code but can be overridden by non-HAL code.
 */
#define HWIO_INTLOCK()
#define HWIO_INTFREE()


/*
 * Input/output port macros for memory mapped IO.
 */
#define __inp(port)         (*((volatile uint8 *) (port)))
#define __inpw(port)        (*((volatile uint16 *) (port)))
#define __inpdw(port)       (*((volatile uint32 *) (port)))
#define __outp(port, val)   (*((volatile uint8 *) (port)) = ((uint8) (val)))
#define __outpw(port, val)  (*((volatile uint16 *) (port)) = ((uint16) (val)))
#define __outpdw(port, val) (*((volatile uint32 *) (port)) = ((uint32) (val)))


#ifdef HAL_HWIO_EXTERNAL

/*
 * Replace macros with externally supplied functions.
 */
#undef  __inp
#undef  __inpw
#undef  __inpdw
#undef  __outp
#undef  __outpw
#undef  __outpdw

#define  __inp(port)          __inp_extern(port)         
#define  __inpw(port)         __inpw_extern(port)
#define  __inpdw(port)        __inpdw_extern(port)
#define  __outp(port, val)    __outp_extern(port, val)
#define  __outpw(port, val)   __outpw_extern(port, val)
#define  __outpdw(port, val)  __outpdw_extern(port, val)

extern uint8   __inp_extern      ( uint32 nAddr );
extern uint16  __inpw_extern     ( uint32 nAddr );
extern uint32  __inpdw_extern    ( uint32 nAddr );
extern void    __outp_extern     ( uint32 nAddr, uint8  nData );
extern void    __outpw_extern    ( uint32 nAddr, uint16 nData );
extern void    __outpdw_extern   ( uint32 nAddr, uint32 nData );

#endif /* HAL_HWIO_EXTERNAL */


/*
 * Base 8-bit byte accessing macros.
 */
#define in_byte(addr)               (__inp(addr))
#define in_byte_masked(addr, mask)  (__inp(addr) & (mask)) 
#define out_byte(addr, val)         __outp(addr,val)
#define out_byte_masked(io, mask, val, shadow)  \
  HWIO_INTLOCK();    \
  out_byte( io, shadow); \
  shadow = (shadow & (uint16)(~(mask))) | ((uint16)((val) & (mask))); \
  HWIO_INTFREE()
#define out_byte_masked_ns(io, mask, val, current_reg_content)  \
  out_byte( io, ((current_reg_content & (uint16)(~(mask))) | \
                ((uint16)((val) & (mask)))) )


/*
 * Base 16-bit word accessing macros.
 */
#define in_word(addr)              (__inpw(addr))
#define in_word_masked(addr, mask) (__inpw(addr) & (mask))
#define out_word(addr, val)        __outpw(addr,val)
#define out_word_masked(io, mask, val, shadow)  \
  HWIO_INTLOCK( ); \
  shadow = (shadow & (uint16)(~(mask))) |  ((uint16)((val) & (mask))); \
  out_word( io, shadow); \
  HWIO_INTFREE( )
#define out_word_masked_ns(io, mask, val, current_reg_content)  \
  out_word( io, ((current_reg_content & (uint16)(~(mask))) | \
                ((uint16)((val) & (mask)))) )


/*
 * Base 32-bit double-word accessing macros.
 */
#define in_dword(addr)              (__inpdw(addr))
#define in_dword_masked(addr, mask) (__inpdw(addr) & (mask))
#define out_dword(addr, val)        __outpdw(addr,val)
#define out_dword_masked(io, mask, val, shadow)  \
   HWIO_INTLOCK(); \
   shadow = (shadow & (uint32)(~(mask))) | ((uint32)((val) & (mask))); \
   out_dword( io, shadow); \
   HWIO_INTFREE()
#define out_dword_masked_ns(io, mask, val, current_reg_content) \
  out_dword( io, ((current_reg_content & (uint32)(~(mask))) | \
                 ((uint32)((val) & (mask)))) )

/** @endcond */

#endif /* HAL_HWIO_H */

