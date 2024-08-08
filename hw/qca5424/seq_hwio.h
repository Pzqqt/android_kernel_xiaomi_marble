
/*
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: ISC
 */

 

 
 

#ifndef __SEQ_H__
#define __SEQ_H__

#include "HALhwio.h"



 
#define SEQ_INH(base, regtype, reg) \
        SEQ_##regtype##_INH(base, reg)

 
#define SEQ_INMH(base, regtype, reg, mask) \
        SEQ_##regtype##_INMH(base, reg, mask)


 
#define SEQ_INFH(base, regtype, reg, fld) \
        (SEQ_##regtype##_INMH(base, reg, HWIO_FMSK(regtype, fld)) >> HWIO_SHFT(regtype, fld))


 
#define SEQ_OUTH(base, regtype, reg, val) \
        SEQ_##regtype##_OUTH(base, reg, val)

 
#define SEQ_OUTMH(base, regtype, reg, mask, val) \
        SEQ_##regtype##_OUTMH(base, reg, mask, val)


 
#define SEQ_OUTFH(base, regtype, reg, fld, val) \
        SEQ_##regtype##_OUTMH(base, reg, HWIO_FMSK(regtype, fld), val << HWIO_SHFT(regtype, fld))


 

 

typedef enum {
    SEC,
    MS,
    US,
    NS
} SEQ_TimeUnit;

extern void seq_wait(uint32 time_value, SEQ_TimeUnit time_unit);


 
extern uint32 seq_poll(uint32 reg_offset, uint32 expect_value, uint32 value_mask, uint32 value_shift, uint32 max_poll_cnt);

#endif  



















