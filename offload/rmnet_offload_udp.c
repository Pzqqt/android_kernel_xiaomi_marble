/* Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "rmnet_descriptor.h"
#include "rmnet_offload_main.h"
#include "rmnet_offload_engine.h"
#include "rmnet_offload_stats.h"
#include "rmnet_offload_knob.h"
static int DATARMNETdf8e0dc3a0(struct DATARMNETd7c9631acd*DATARMNETaa568481cf,
struct DATARMNETd812bcdbb5*DATARMNET5fe4c722a8){u64 DATARMNET71c7d18d88;u16 
DATARMNET95acece3fc;if(!DATARMNETaa568481cf->DATARMNET1db11fa85e)return 
DATARMNET0413b43080;DATARMNET95acece3fc=(DATARMNET5fe4c722a8->
DATARMNET719f68fb88->gso_size)?:DATARMNET5fe4c722a8->DATARMNET1ef22e4c76;if(
DATARMNET95acece3fc!=DATARMNETaa568481cf->DATARMNET1978d5d8de){
DATARMNETa00cda79d0(DATARMNETbc56977b7e);return DATARMNETb0bd5db24d;}
DATARMNET71c7d18d88=DATARMNETf1d1b8287f(DATARMNET9c85bb95a3);if(
DATARMNET5fe4c722a8->DATARMNET1ef22e4c76+DATARMNETaa568481cf->
DATARMNETcf28ae376b>=DATARMNET71c7d18d88){DATARMNETa00cda79d0(
DATARMNETc9b8ef90d1);return DATARMNETb0bd5db24d;}DATARMNET5fe4c722a8->
DATARMNETf1b6b0a6cc=false;return DATARMNET0413b43080;}bool DATARMNET8dc47eb7af(
struct DATARMNETd7c9631acd*DATARMNETaa568481cf,struct DATARMNETd812bcdbb5*
DATARMNET5fe4c722a8,bool DATARMNETd87669e323){int DATARMNETb14e52a504;if(
DATARMNETd87669e323){DATARMNETa00cda79d0(DATARMNETc34a778ea2);
DATARMNETa3055c21f2(DATARMNETaa568481cf);DATARMNET19d190f2bd(DATARMNET5fe4c722a8
);return true;}DATARMNETb14e52a504=DATARMNETdf8e0dc3a0(DATARMNETaa568481cf,
DATARMNET5fe4c722a8);if(DATARMNETb14e52a504==DATARMNET0413b43080){
DATARMNET33aa5df9ef(DATARMNETaa568481cf,DATARMNET5fe4c722a8);}else if(
DATARMNETb14e52a504==DATARMNETb0bd5db24d){DATARMNETa3055c21f2(
DATARMNETaa568481cf);DATARMNET33aa5df9ef(DATARMNETaa568481cf,DATARMNET5fe4c722a8
);}return true;}
