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
static int DATARMNET83d48307c2(struct DATARMNET31caee06e2*DATARMNETd44a80d414,
struct DATARMNETadd6ff90d0*DATARMNETc3ffad4b30){u64 DATARMNET0ee619d1b8;u16 
DATARMNETfc02a09052;if(!DATARMNETd44a80d414->DATARMNET30d367f886)return 
DATARMNET7d8a4bb6b1;DATARMNETfc02a09052=(DATARMNETc3ffad4b30->
DATARMNET798fafaf36->gso_size)?:DATARMNETc3ffad4b30->DATARMNET5671455f38;if(
DATARMNETfc02a09052!=DATARMNETd44a80d414->DATARMNET632219f39d){
DATARMNET5d24865423(DATARMNETaee7f80b20);return DATARMNET6b149bc29f;}
DATARMNET0ee619d1b8=DATARMNET3d487d950f(DATARMNETfbbc8e2e06);if(
DATARMNETc3ffad4b30->DATARMNET5671455f38+DATARMNETd44a80d414->
DATARMNET5f92dbf405>=DATARMNET0ee619d1b8){DATARMNET5d24865423(
DATARMNETb3aa109946);return DATARMNET6b149bc29f;}DATARMNETc3ffad4b30->
DATARMNET1e7926dc4c=false;return DATARMNET7d8a4bb6b1;}bool DATARMNET9dd18589e3(
struct DATARMNET31caee06e2*DATARMNETd44a80d414,struct DATARMNETadd6ff90d0*
DATARMNETc3ffad4b30,bool DATARMNETc72284bb2d){int DATARMNET094475d445;if(
DATARMNETc72284bb2d){DATARMNET5d24865423(DATARMNET76bca11273);
DATARMNET3bdaf91675(DATARMNETd44a80d414);DATARMNETf9d626d72b(DATARMNETc3ffad4b30
);return true;}DATARMNET094475d445=DATARMNET83d48307c2(DATARMNETd44a80d414,
DATARMNETc3ffad4b30);if(DATARMNET094475d445==DATARMNET7d8a4bb6b1){
DATARMNETfc01092f09(DATARMNETd44a80d414,DATARMNETc3ffad4b30);}else if(
DATARMNET094475d445==DATARMNET6b149bc29f){DATARMNET3bdaf91675(
DATARMNETd44a80d414);DATARMNETfc01092f09(DATARMNETd44a80d414,DATARMNETc3ffad4b30
);}return true;}
